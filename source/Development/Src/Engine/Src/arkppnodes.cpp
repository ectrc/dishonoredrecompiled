/*=============================================================================
	arkppnodes.cpp: the backbone of Arkane's post-process node graph.

	DISHONORED(port): the node classes of enginearkppclasses.h and the proxies of the four nodes that only stand for a
	surface: the scene colour, a render-target asset, one of the engine's own targets, and the switch (which has no
	proxy of its own). 2012 source file arkppnodes.cpp, 50 functions; the rvas are in the comments.

	The graph is built in FViewInfo::FViewInfo (2013 rva 0x46b3a0 (2012 0x493e40)) and rendered from
	FSceneRenderer::RenderPostProcessEffects (2013 rva 0x448990 (2012 0x46bef0)); one pass per node, each drawing into the surface its
	target input names, and the node that holds FArkPpRenderConfig::m_bForceToDestination draws into the back buffer.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "arkpp.h"

IMPLEMENT_CLASS(UArkPpNodeController);
IMPLEMENT_CLASS(UArkPpNode);
IMPLEMENT_CLASS(UArkPpNodeSceneColor);
IMPLEMENT_CLASS(UArkPpNodeTarget);
IMPLEMENT_CLASS(UArkPpNodeCommonTarget);
IMPLEMENT_CLASS(UArkPpNodeSwitch);
IMPLEMENT_CLASS(UArkPpNodeMaterial);
IMPLEMENT_CLASS(UArkPpNodeBlur);
IMPLEMENT_CLASS(UArkPpNodeDof);
IMPLEMENT_CLASS(UArkPpNodeKuwa);
IMPLEMENT_CLASS(UArkPpNodeAA);
IMPLEMENT_CLASS(UArkPpSettings);


/**
 * DISHONORED(port): the node-cache lookup every CreateSceneProxy starts with (2013 rva 0x524ae0 (2012 0x5659e0) and friends): a node
 * several nodes take as input gets one proxy, and a node that answered NULL once stays NULL.
 */
UBOOL ArkPpFindCachedProxy(FArkPpCreateProxyConfig& Config,UArkPpNode* Node,FArkPpNodeProxy*& OutProxy)
{
	FArkPpNodeProxy** Cached = Config.mNodeCache.Find(Node);
	if (Cached)
	{
		OutProxy = *Cached;
		return TRUE;
	}
	return FALSE;
}

/**
 * DISHONORED(port): the viewport every node pass sets before it draws (2012 rvas 0x54bed0, 0x54c180, 0x55f410 and the
 * filter nodes): the view's render-target rectangle scaled from the scene buffer to this node's surface, so a node with
 * a half or quarter size target covers the same part of the image.
 */
void ArkPpSetNodeViewport(const FViewInfo& View,UINT SizeX,UINT SizeY)
{
	const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
	const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
	RHISetViewport(
		SizeX * View.RenderTargetX / BufferSizeX,
		SizeY * View.RenderTargetY / BufferSizeY,
		0.0f,
		SizeX * (View.RenderTargetX + View.RenderTargetSizeX) / BufferSizeX,
		SizeY * (View.RenderTargetY + View.RenderTargetSizeY) / BufferSizeY,
		1.0f);
}

/** DISHONORED(port): 2013 rva 0x5249c0 (2012 0x5658c0) / 0x5656c0 - the bit pair at UArkPpNode @56 plus the controller's veto. */
UBOOL UArkPpNode::IsShownInConfig(const FArkPpCreateProxyConfig& Config)
{
	const UBOOL bShown = Config.mbOnlyInEditor ? m_bShowInEditor : m_bShowInGame;
	if (!bShown)
	{
		return FALSE;
	}
	if (!m_Controller)
	{
		return TRUE;
	}
	// DISHONORED(bringup): the two controllers that decide this answer for themselves are ported now
	// (dispostprocesscontrollers.cpp) and the base answers retail's TRUE, so this is the content's own answer.
	// -arkppcontrollersshown is agent CE's probe and forces every controller-driven node to draw; it is bring-up only
	// and stays until the controllers' timers are driven, which needs ADishonoredPlayerController::
	// ModifyPostProcessSettings (2013 rva 0x6adeb0) - nothing in this tree calls UArkPpNodeController::Tick yet.
	static UBOOL bControllersShown = ParseParam(appCmdLine(),TEXT("arkppcontrollersshown"));
	return bControllersShown || m_Controller->IsShown(this);
}

/*-----------------------------------------------------------------------------
	FArkPpNodeSceneColorProxy (2012 PDB 16 bytes: FArkPpNodeProxy + m_UseLR)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the scene colour as a graph node. It draws nothing: it is the surface the world was rendered into,
 * either the HDR scene colour or - when the node says so - the LDR one (2012 rvas 0x54bd60 GetSurface, 0x54bdb0
 * GetTexture, 0x54a010 GetSurfaceSizeX).
 */
class FArkPpNodeSceneColorProxy : public FArkPpNodeProxy
{
public:
	FArkPpNodeSceneColorProxy(UArkPpNodeSceneColor* InNode)
		: m_UseLR(InNode->m_LowRange)
	{}

	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config) { return FALSE; }

	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View)
	{
		return m_UseLR ? GSceneRenderTargets.GetSceneColorLDRSurface() : GSceneRenderTargets.GetSceneColorSurface();
	}

	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View)
	{
		return m_UseLR ? GSceneRenderTargets.GetSceneColorLDRTexture() : GSceneRenderTargets.GetSceneColorTexture();
	}

	virtual UINT GetSurfaceSizeX() { return GSceneRenderTargets.GetBufferSizeX(); }
	virtual UINT GetSurfaceSizeY() { return GSceneRenderTargets.GetBufferSizeY(); }

private:
	UINT m_UseLR;
};

/** DISHONORED(port): 2013 rva 0x524ae0 (2012 0x5659e0). */
FArkPpNodeProxy* UArkPpNodeSceneColor::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeSceneColorProxy(this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/*-----------------------------------------------------------------------------
	FArkPpNodeTargetProxy (2012 PDB 20 bytes: FArkPpNodeProxy + m_RT2D + m_Node)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): a UTextureRenderTarget2D asset as a graph node. Its Render only runs when the node has a
 * controller: the controller draws into the target and the proxy resolves it (2013 rva 0x50b6d0 (2012 0x54bed0)).
 */
class FArkPpNodeTargetProxy : public FArkPpNodeProxy
{
public:
	FArkPpNodeTargetProxy(UArkPpNodeTarget* InNode)
		: m_Node(InNode)
	{}

	/** DISHONORED(port): 2013 rva 0x50b6d0 (2012 0x54bed0). */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (!m_Node->m_Controller)
		{
			return FALSE;
		}
		const FSurfaceRHIRef Surface = GetSurface(View);
		RHISetRenderTarget(Surface,GSceneRenderTargets.GetSceneDepthSurface());
		const UINT SizeX = GetSurfaceSizeX();
		const UINT SizeY = GetSurfaceSizeY();
		ArkPpSetNodeViewport(View,SizeX,SizeY);
		const UBOOL bDirty = m_Node->m_Controller->Render(EPpRs_BeforeAll,Scene,&View);
		if (bDirty)
		{
			RHICopyToResolveTarget(Surface,TRUE,FResolveParams());
		}
		return bDirty;
	}

	/** DISHONORED(port): 2013 rva 0x50b600 (2012 0x54be00) / 0x54fe00 - NULL until the asset has a render-target resource. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View)
	{
		FTextureRenderTargetResource* Resource = m_Node->m_Surface ? m_Node->m_Surface->GetRenderTargetResource() : NULL;
		return Resource ? Resource->GetRenderTargetSurface() : FSurfaceRHIRef();
	}

	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View)
	{
		FTextureRenderTargetResource* Resource = m_Node->m_Surface ? m_Node->m_Surface->GetRenderTargetResource() : NULL;
		FTextureRenderTarget2DResource* Resource2D = Resource ? Resource->GetTextureRenderTarget2DResource() : NULL;
		return Resource2D ? Resource2D->GetTextureRHI() : FTexture2DRHIRef();
	}

	/** DISHONORED(port): 2013 rva 0x5097a0 (2012 0x54a030) / 0x54a050. */
	virtual UINT GetSurfaceSizeX()
	{
		FTextureRenderTargetResource* Resource = m_Node->m_Surface ? m_Node->m_Surface->GetRenderTargetResource() : NULL;
		return Resource ? Resource->GetSizeX() : 0;
	}

	virtual UINT GetSurfaceSizeY()
	{
		FTextureRenderTargetResource* Resource = m_Node->m_Surface ? m_Node->m_Surface->GetRenderTargetResource() : NULL;
		return Resource ? Resource->GetSizeY() : 0;
	}

private:
	UArkPpNodeTarget* m_Node;
};

/** DISHONORED(port): 2013 rva 0x524b70 (2012 0x565a70). */
FArkPpNodeProxy* UArkPpNodeTarget::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeTargetProxy(this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/*-----------------------------------------------------------------------------
	FArkPpNodeCommonTargetProxy (2012 PDB 40 bytes)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): one of the engine's own surfaces as a graph node, optionally cleared before use (2012 rvas
 * 0x54a080 ctor, 0x54c040 GetSurface, 0x54c0e0 GetTexture, 0x54a0d0 GetSurfaceSizeX, 0x54c180 Render).
 */
class FArkPpNodeCommonTargetProxy : public FArkPpNodeProxy
{
public:
	FArkPpNodeCommonTargetProxy(UArkPpNodeCommonTarget* InNode)
		: m_Target(InNode->m_Target)
		, m_Controller(InNode->m_Controller)
		, m_bClear(InNode->m_bClear)
		, m_ClearColor(InNode->m_ClearColor)
	{}

	/** DISHONORED(port): 2013 rva 0x50b980 (2012 0x54c180). */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (!m_Controller && !m_bClear)
		{
			return FALSE;
		}
		const FSurfaceRHIRef Surface = GetSurface(View);
		RHISetRenderTarget(Surface,GSceneRenderTargets.GetSceneDepthSurface());
		ArkPpSetNodeViewport(View,GetSurfaceSizeX(),GetSurfaceSizeY());
		UBOOL bDirty = FALSE;
		if (m_bClear)
		{
			RHIClear(TRUE,m_ClearColor,FALSE,0.0f,FALSE,0);
			bDirty = TRUE;
		}
		if (m_Controller)
		{
			bDirty |= m_Controller->Render(EPpRs_BeforeAll,Scene,&View);
		}
		if (bDirty)
		{
			RHICopyToResolveTarget(Surface,TRUE,FResolveParams());
		}
		return bDirty;
	}

	/** DISHONORED(port): 2013 rva 0x50b840 (2012 0x54c040). */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View)
	{
		switch (m_Target)
		{
		case EPpRs_SceneColorLdr:
		case EPpCt_AttenuationBuffer:
			return GSceneRenderTargets.GetSceneColorLDRSurface();
		case EPpCt_FogMask:
			return GSceneRenderTargets.GetRenderTargetSurface(ArkFogMask);
		case EPpRs_SceneColor:
			return GSceneRenderTargets.GetRenderTargetSurface(ArkDofQuarter);
		case EPpRs_LowResParticles:
			return GSceneRenderTargets.GetRenderTargetSurface(ArkDofHalf);
		}
		return FSurfaceRHIRef();
	}

	/** DISHONORED(port): 2013 rva 0x50b8e0 (2012 0x54c0e0). */
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View)
	{
		switch (m_Target)
		{
		case EPpRs_SceneColorLdr:
		case EPpCt_AttenuationBuffer:
			return GSceneRenderTargets.GetSceneColorLDRTexture();
		case EPpCt_FogMask:
			return GSceneRenderTargets.GetRenderTargetTexture(ArkFogMask);
		case EPpRs_SceneColor:
			return GSceneRenderTargets.GetRenderTargetTexture(ArkDofQuarter);
		case EPpRs_LowResParticles:
			return GSceneRenderTargets.GetRenderTargetTexture(ArkDofHalf);
		}
		return FTexture2DRHIRef();
	}

	/** DISHONORED(port): 2013 rva 0x509840 (2012 0x54a0d0) / 0x54a110 - the depth-of-field pair is a quarter and a half of the buffer. */
	virtual UINT GetSurfaceSizeX()
	{
		switch (m_Target)
		{
		case EPpRs_SceneColorLdr:
		case EPpCt_AttenuationBuffer:
		case EPpCt_FogMask:
			return GSceneRenderTargets.GetBufferSizeX();
		case EPpRs_SceneColor:
			return GSceneRenderTargets.GetBufferSizeX() >> 2;
		case EPpRs_LowResParticles:
			return GSceneRenderTargets.GetBufferSizeX() >> 1;
		}
		return 0;
	}

	virtual UINT GetSurfaceSizeY()
	{
		switch (m_Target)
		{
		case EPpRs_SceneColorLdr:
		case EPpCt_AttenuationBuffer:
		case EPpCt_FogMask:
			return GSceneRenderTargets.GetBufferSizeY();
		case EPpRs_SceneColor:
			return GSceneRenderTargets.GetBufferSizeY() >> 2;
		case EPpRs_LowResParticles:
			return GSceneRenderTargets.GetBufferSizeY() >> 1;
		}
		return 0;
	}

private:
	BYTE m_Target;
	UArkPpNodeController* m_Controller;
	UINT m_bClear;
	FLinearColor m_ClearColor;
};

/** DISHONORED(port): 2013 rva 0x524c00 (2012 0x565b00). */
FArkPpNodeProxy* UArkPpNodeCommonTarget::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeCommonTargetProxy(this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/*-----------------------------------------------------------------------------
	UArkPpNodeSwitch: no proxy of its own
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x524c90 (2012 0x565b90) - both branches have to exist and be valid, whichever is selected. */
UBOOL UArkPpNodeSwitch::IsValid(FArkPpIsValidData& Cache)
{
	UBOOL* Memo = Cache.mIsValidCache.Find(this);
	if (Memo)
	{
		return *Memo;
	}
	const UBOOL bValid = m_TRUE && m_FALSE && m_TRUE->IsValid(Cache) && m_FALSE->IsValid(Cache);
	Cache.mIsValidCache.Set(this,bValid);
	return bValid;
}

/** DISHONORED(port): 2013 rva 0x509970 (2012 0x54a200). */
FArkPpNodeProxy* UArkPpNodeSwitch::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	UArkPpNode* Selected = m_Selection ? m_TRUE : m_FALSE;
	return Selected ? Selected->CreateSceneProxy(Config) : NULL;
}
