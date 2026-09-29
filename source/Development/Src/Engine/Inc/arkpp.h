#pragma once
/*===========================================================================
    arkpp.h - the core of Arkane's post-process node graph.

    DISHONORED(port): retail's post-processing is not the reference chain of UPostProcessEffect proxies but a graph of
    UArkPpNode objects (enginearkppclasses.h) that the view turns into a tree of FArkPpNodeProxy render proxies. The
    graph is built in FViewInfo::FViewInfo (2013 rva 0x493e40) and rendered from
    FSceneRenderer::RenderPostProcessEffects(SDPG_PostProcess) (2013 rva 0x448990).

    Layouts are the 2012 PDB: FArkPpRenderConfig 4, FArkPpNodeProxy 12 (FRefCountedObject + m_bDone),
    FArkPpIsValidData and FArkPpCreateProxyConfig hold one TMap keyed on the node (retail gives both a 128-element
    inline set allocator, which is a speed choice and not observable).
===========================================================================*/
#ifndef _INC_ARKPP
#define _INC_ARKPP

class UArkPpNode;
class FArkPpNodeProxy;
class FScene;
class FViewInfo;

/**
 * DISHONORED(port): 2012 PDB FArkPpRenderConfig (4 bytes, one bit). m_bForceToDestination says that this node is the
 * last of the graph, so it draws into the destination surface (the back buffer) instead of its own target; every node
 * clears the bit before it renders its inputs.
 */
struct FArkPpRenderConfig
{
	BITFIELD m_bForceToDestination : 1;

	FArkPpRenderConfig(UBOOL bInForceToDestination = FALSE)
		: m_bForceToDestination(bInForceToDestination ? 1 : 0)
	{}
};

/**
 * DISHONORED(port): 2012 PDB FArkPpIsValidData (2360 bytes) - the memo of UArkPpNode::IsValid, so a node that several
 * nodes take as input is validated once. The value is the node's validity.
 */
struct FArkPpIsValidData
{
	TMap<UArkPpNode*,UBOOL> mIsValidCache;
};

/**
 * DISHONORED(port): 2012 PDB FArkPpNodeProxy (12 bytes; vtable ~FArkPpNodeProxy, Release, Render, GetSurface,
 * GetTexture, GetSurfaceSizeX, GetSurfaceSizeY). One render-thread proxy per graph node; the tree is rebuilt with the
 * FViewInfo of every frame, which is why m_bDone (a node with several consumers renders once) needs no reset.
 */
class FArkPpNodeProxy : public FRefCountedObject
{
public:
	FArkPpNodeProxy()
		: m_bDone(FALSE)
	{}

	/** Renders this node and everything it depends on; returns TRUE if anything was drawn. */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config) = 0;
	/** The surface this node draws into. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) = 0;
	/** The texture a consumer samples this node's output from. */
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) = 0;
	virtual UINT GetSurfaceSizeX() = 0;
	virtual UINT GetSurfaceSizeY() = 0;

	/** Set by Render, so a node several nodes take as input draws once per frame (2012 PDB @8). */
	UINT m_bDone;
};

/**
 * DISHONORED(port): 2012 PDB FArkPpCreateProxyConfig (2376 bytes; ctor 2013 rva 0x4936e0, PushUberOverride 0x471080).
 * Carried down the graph while the proxies are created: mbOnlyInEditor is the view's SHOW_Editor, so an editor-only
 * node is skipped in game; mbRedirectToBackBuffer starts TRUE and the first material node that sees it takes it, which
 * is how exactly one node owns the destination; mNodeCache gives a shared node one proxy; m_UberOverrides is the stack
 * of uber-parameter pushes a node or its controller made, which the depth-of-field node reads.
 */
struct FArkPpCreateProxyConfig
{
	/** 2012 PDB FArkPpCreateProxyConfig::UberOverride (100 bytes). */
	struct UberOverride
	{
		FArkUberPpParameters m_UberParams;
		FLOAT m_Weight;
	};

	BITFIELD mbOnlyInEditor : 1;
	BITFIELD mbRedirectToBackBuffer : 1;
	TMap<UArkPpNode*,FArkPpNodeProxy*> mNodeCache;
	TArray<UberOverride> m_UberOverrides;

	/** DISHONORED(port): 2013 rva 0x4936e0 - mbOnlyInEditor FALSE, mbRedirectToBackBuffer TRUE. */
	FArkPpCreateProxyConfig()
		: mbOnlyInEditor(0)
		, mbRedirectToBackBuffer(1)
	{}

	/** DISHONORED(port): 2013 rva 0x471080. */
	void PushUberOverride(const FArkUberPpParameters& iParams,FLOAT iW)
	{
		UberOverride& Override = m_UberOverrides(m_UberOverrides.Add());
		Override.m_UberParams = iParams;
		Override.m_Weight = iW;
	}
};

/**
 * DISHONORED(port): the four methods of the uber-parameter script structs of EngineClasses.h (2013 rvas 0x2a2980,
 * 0x2a2a30, 0x2a2d20, 0x2aca00, 0x2acbc0), which every FArkPp filter node resolves its parameters with. They are
 * free functions rather than members because EngineClasses.h is generated and its Arkane structs have no CppText
 * hook yet; arkppnodedof.cpp holds the bodies.
 */
extern void ArkPpColorBalanceSetDefaultOnNoOverride(FArkPpColorBalanceParameters& Params);
extern void ArkPpColorBalanceForceDefault(FArkPpColorBalanceParameters& Params);
extern void ArkPpColorBalanceApplyTo(const FArkPpColorBalanceParameters& Params,FArkPpColorBalanceParameters& oResult,FLOAT iAlpha);
extern void ArkUberPpSetDefaultOnNoOverride(FArkUberPpParameters& Params);
extern void ArkUberPpApplyTo(const FArkUberPpParameters& Params,FArkUberPpParameters& oResult,FLOAT iAlpha,UBOOL bDOFOnlyBlendAmount);

/**
 * DISHONORED(port): the two the settings path needs on top of those (2013 rvas 0x2a2e00, 0x2adb50). The
 * bodies are in UnPlayer.cpp beside ULocalPlayer::UpdatePostProcessSettings, their only caller.
 */
extern void ArkUberPpForceDefault(FArkUberPpParameters& Params);
extern void ArkPpConfigApplyTo(const FArkPpConfig& Config,FArkPpConfig& oResult,FLOAT iAlpha,UBOOL bDOFOnlyBlendAmount);

/** DISHONORED(port): arkppnodes.cpp - the node-cache lookup and the viewport every node pass sets. */
extern UBOOL ArkPpFindCachedProxy(FArkPpCreateProxyConfig& Config,UArkPpNode* Node,FArkPpNodeProxy*& OutProxy);
extern void ArkPpSetNodeViewport(const FViewInfo& View,UINT SizeX,UINT SizeY);

/** DISHONORED(bringup): the per-node counters of the post-process census (arkppnodes.cpp). */
extern INT GDisCensusArkPpNodes;
extern INT GDisCensusArkPpDraws;
extern INT GDisCensusArkPpMaterialDraws;
extern INT GDisCensusArkPpSkipped;

/** DISHONORED(bringup): the filter nodes' own draws (arkppnode{aa,blur,kuwa}.cpp), so the census names each pass. */
extern INT GDisCensusArkPpAADraws;
extern INT GDisCensusArkPpBlurDraws;
extern INT GDisCensusArkPpKuwaDraws;
extern INT GDisCensusArkPpDofDraws;

#endif // _INC_ARKPP
