#pragma once
/*===========================================================================
    enginearkppclasses.h - the node classes of Arkane's post-process graph, the retail Engine package's.

    DISHONORED(port): retail's post-processing is this graph, not the reference UPostProcessEffect chain: a
    UPostProcessChain holds m_GraphRoot / m_AllNodes (EngineClasses.h) and the view turns the graph into a tree of
    FArkPpNodeProxy objects (arkpp.h) that draw one full-screen pass per node. The material node is where Dishonored's
    colour treatment lives.

    These twelve classes and the six structs nested in them are `package_2013 = Engine` classes that lived in
    DishonoredGame's generated shim header (native_class_sizes.csv); this is the same move agent BD made for the DisFog
    classes and agent AI for the Arkane Engine classes, and it is needed because the proxies are Engine's. Layouts are
    copied verbatim from the generated shim blocks (retail SDK + 2012 PDB) and asserted below; the virtuals and their
    inline bodies come from the decompiles named at each one.

    Base UArkPpNode defaults, read off the vtable (vtables.csv slots 72..80, COMDAT-folded bodies): NumInputs 0,
    InputName "", GetInput NULL, LinkInput TRUE, UnlinkInput FALSE, OutputIsSurface FALSE, IsValid FALSE,
    CreateSceneProxy NULL.
===========================================================================*/
#ifndef NAMES_ONLY
#ifndef INCLUDED_ENGINE_ARKPP_CLASSES
#define INCLUDED_ENGINE_ARKPP_CLASSES 1

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

// the DECLARE_CLASS family of the script classes, like every generated *Classes.h of the module
#define ENABLE_DECLARECLASS_MACRO 1
#include "UnObjBas.h"
#undef ENABLE_DECLARECLASS_MACRO

struct FArkPpCreateProxyConfig;
struct FArkPpIsValidData;
class FArkPpNodeProxy;
class FScene;
class FViewInfo;
// DISHONORED(layout): the four script enums of these classes, moved verbatim out of DishonoredGame's generated shim
// header with the retail SDK's own names and values (they are Engine.ArkPpNode* enums, so they belong here now, and
// the generator drops an enum the tree declares). EPpNodeCommonTarget's names are the content's and not all of them
// read as what they are: the values are what FArkPpNodeCommonTargetProxy::GetSurface / GetSurfaceSizeX select
// (2013 rva 0x50b840 / 0x509fe0) - AttenuationBuffer and SceneColorLdr are the LDR scene colour at full size, FogMask
// the fog mask at full size, SceneColor the quarter-size and LowResParticles the half-size depth-of-field target.
enum EPpNodeAAType
{
    EPpAa_None              =0,
    EPpAa_Mlaa              =1,
    EPpAa_Fxaa              =2,
    EPpAa_MAX               =3,
};

enum EPpNodeBlurType
{
    EPpBt_Box               =0,
    EPpBt_Motion            =1,
    EPpBt_Radial            =2,
    EPpBt_MAX               =3,
};

enum EPpNodeCommonTarget
{
    EPpCt_AttenuationBuffer =0,
    EPpCt_FogMask           =1,
    EPpRs_SceneColor        =2,
    EPpRs_SceneColorLdr     =3,
    EPpRs_LowResParticles   =4,
    EPpNodeCommonTarget_MAX =5,
};

enum EPpNodeRenderStage
{
    EPpRs_BeforeAll         =0,
    EPpRs_BeforeDraw        =1,
    EPpRs_AfterDraw         =2,
    EPpRs_AfterAll          =3,
    EPpRs_MAX               =4,
};


// Engine.ArkPpNodeMaterial.AnInput: retail SDK size 20 (2012 PDB 20)
struct FAnInput
{
	class UArkPpNode* m_Node;
	FStringNoInit m_Name;
	BYTE m_FilterType;
	BYTE m_TileU;
	BYTE m_TileV;

	/** Constructors */
	FAnInput() {}
	FAnInput(EEventParm)
	{
		appMemzero(this, sizeof(FAnInput));
	}
};

// Engine.ArkPpNodeBlur.BoxBlurConfig: retail SDK size 4 (2012 PDB 4)
struct FBoxBlurConfig
{
	FLOAT m_KernelSize;

	/** Constructors */
	FBoxBlurConfig() {}
	FBoxBlurConfig(EEventParm)
	{
		appMemzero(this, sizeof(FBoxBlurConfig));
	}
};

// Engine.ArkPpNodeBlur.MotionBlurConfig: retail SDK size 16 (2012 PDB 16)
struct FMotionBlurConfig
{
	FLOAT m_LengthStrength;
	FLOAT m_InitialOffsetStrength;
	INT m_PassCount;
	BITFIELD m_FullOffsetInVectorField:1;

	/** Constructors */
	FMotionBlurConfig() {}
	FMotionBlurConfig(EEventParm)
	{
		appMemzero(this, sizeof(FMotionBlurConfig));
	}
};

// Engine.ArkPpNodeBlur.RadialBlurConfig: retail SDK size 12 (2012 PDB 12)
struct FRadialBlurConfig
{
	FLOAT m_Strength;
	FVector2D m_Center;

	/** Constructors */
	FRadialBlurConfig() {}
	FRadialBlurConfig(EEventParm)
	{
		appMemzero(this, sizeof(FRadialBlurConfig));
	}
};

// Engine.ArkPpNodeAA.FxAaConfig: retail SDK size 16 (2012 PDB 16)
struct FFxAaConfig
{
	FVector m_LuminanceEquation;
	BYTE m_Luma;

	/** Constructors */
	FFxAaConfig() {}
	FFxAaConfig(EEventParm)
	{
		appMemzero(this, sizeof(FFxAaConfig));
	}
};

// Engine.ArkPpNodeAA.MlAaConfig: retail SDK size 16 (2012 PDB 16)
struct FMlAaConfig
{
	FVector m_LuminanceEquation;
	FLOAT m_EdgeDetectionThresold;

	/** Constructors */
	FMlAaConfig() {}
	FMlAaConfig(EEventParm)
	{
		appMemzero(this, sizeof(FMlAaConfig));
	}
};

// Engine.ArkPpNodeController: retail sizeof 56, reflected span 56..56 (2012 PDB sizeof 56). The subclasses are
// DishonoredGame's (dispostprocesscontrollers.cpp); a node with a controller asks it for its material and its uber
// parameters and lets it draw at the four render stages.
class UArkPpNodeController : public UObject
{
public:
	//## BEGIN PROPS ArkPpNodeController
	//## END PROPS ArkPpNodeController

	/** DISHONORED(port): vtable slots 72..76 (2012 PDB UArkPpNodeController_vtbl @288..304); every body is a
	    DishonoredGame subclass's, so the base does nothing. */
	virtual class UMaterialInterface* Update(const class UArkPpNodeMaterial* Node,UBOOL* bOutOverrideUber,FLOAT* OutWeight,struct FArkUberPpParameters* InOutParams) { return NULL; }
	virtual void Update(const class UArkPpNode* Node,struct FArkUberPpParameters* InOutParams) {}
	/**
	 * DISHONORED(port): retail's base answers TRUE. Slot 74 of UArkPpNodeController's vtable is 2013 rva 0x5ea9d0
	 * (2012 0x66a860),
	 * whose whole body is `mov eax, 1; retn 4` (identical-code-folded with UObject::IsRefSaveable, which is why the
	 * name in the vtable dump is that one). A controller only hides its node when it overrides this and says so, and
	 * exactly two of the four subclasses do: UDisOpacityParameterPpController::IsShown is
	 * `m_bDoNotDisablePp || m_bIsOn || m_CurrentTime > 0` (2013 rva 0x7e7ca0) and UDisDarkVisionPpController::IsShown
	 * is `m_bIsActive || m_bDebugIsActive || m_EyeLidTime > 0 || m_PowerTime > 0` (0x7e7c30). Both are ported now
	 * (dispostprocesscontrollers.cpp), which is what lets the base stop standing in for them: agent CE had to answer
	 * FALSE here because nothing overrode it and the eyelid material's default is a closed eye over the whole frame.
	 */
	virtual UBOOL IsShown(const class UArkPpNode* Node) { return TRUE; }
	virtual UBOOL Tick(FLOAT DeltaTime,enum ELevelTick TickType) { return FALSE; }
	virtual UBOOL Render(EPpNodeRenderStage Stage,const FScene* Scene,FViewInfo* View) { return FALSE; }

	DECLARE_ABSTRACT_CLASS(UArkPpNodeController,UObject,0,Engine)
};

// Engine.ArkPpNode: retail sizeof 104, reflected span 56..104 (2012 PDB sizeof 104)
class UArkPpNode : public UObject
{
public:
	//## BEGIN PROPS ArkPpNode
	BITFIELD m_bShowInEditor:1;
	BITFIELD m_bShowInGame:1;
	FName EffectName;
	class UArkPpNodeController* m_Controller;
	INT NodePosY;
	INT NodePosX;
	INT DrawWidth;
	INT DrawHeight;
	INT OutDrawY;
	TArrayNoInit<INT> InDrawY;
	//## END PROPS ArkPpNode

	/** DISHONORED(port): the graph interface, vtable slots 72..80 (2012 PDB UArkPpNode_vtbl @288..320). */
	virtual UINT NumInputs() const { return 0; }
	virtual FString InputName(UINT Idx) const { return FString(TEXT("")); }
	virtual UArkPpNode* GetInput(UINT Idx) { return NULL; }
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node) { return TRUE; }
	virtual UBOOL UnlinkInput(UArkPpNode* Node) { return FALSE; }
	virtual UBOOL UnlinkInput(UINT Idx) { return FALSE; }
	virtual UBOOL OutputIsSurface() { return FALSE; }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache) { return FALSE; }
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config) { return NULL; }

	/** Whether this node draws in this view at all: m_bShowInEditor in the editor, m_bShowInGame in game, and a
	    controller may hide it as well (2013 rva 0x5249c0 (2012 0x5658c0), 0x5656c0). */
	UBOOL IsShownInConfig(const FArkPpCreateProxyConfig& Config);

	DECLARE_ABSTRACT_CLASS(UArkPpNode,UObject,0,Engine)
};

// Engine.ArkPpNodeSceneColor: retail sizeof 112, reflected span 104..112 (2012 PDB sizeof 112)
class UArkPpNodeSceneColor : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeSceneColor
	class UArkPpNode* m_Out;
	BITFIELD m_LowRange:1;
	//## END PROPS ArkPpNodeSceneColor

	virtual UBOOL OutputIsSurface() { return TRUE; }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache) { return TRUE; }
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeSceneColor,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeTarget: retail sizeof 108, reflected span 104..108 (2012 PDB sizeof 108)
class UArkPpNodeTarget : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeTarget
	class UTextureRenderTarget2D* m_Surface;
	//## END PROPS ArkPpNodeTarget

	virtual UBOOL OutputIsSurface() { return TRUE; }
	/** DISHONORED(port): 2013 rva 0x5097e0 (2012 0x54a070). */
	virtual UBOOL IsValid(FArkPpIsValidData& Cache) { return m_Surface != NULL; }
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeTarget,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeCommonTarget: retail sizeof 128, reflected span 104..128 (2012 PDB sizeof 128)
class UArkPpNodeCommonTarget : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeCommonTarget
	BYTE m_Target;
	BITFIELD m_bClear:1;
	FLinearColor m_ClearColor;
	//## END PROPS ArkPpNodeCommonTarget

	virtual UBOOL OutputIsSurface() { return TRUE; }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache) { return TRUE; }
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeCommonTarget,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeSwitch: retail sizeof 128, reflected span 104..128 (2012 PDB sizeof 128). The switch has no proxy of
// its own: it creates the chosen input's (2013 rva 0x509970 (2012 0x54a200)).
class UArkPpNodeSwitch : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeSwitch
	class UArkPpNode* m_TRUE;
	class UArkPpNode* m_FALSE;
	BITFIELD m_Selection:1;
	BITFIELD m_bFromPostProcessChain:1;
	FStringNoInit m_Switch;
	//## END PROPS ArkPpNodeSwitch

	virtual UINT NumInputs() const { return 2; }
	/** DISHONORED(port): 2013 rva 0x50f880 (2012 0x54ff70). */
	virtual FString InputName(UINT Idx) const { return FString(Idx ? TEXT("FALSE") : TEXT("TRUE")); }
	/** DISHONORED(port): 2013 rva 0x51b070 (2012 0x559510). */
	virtual UArkPpNode* GetInput(UINT Idx)
	{
		if (Idx == 0)
		{
			return m_TRUE;
		}
		if (Idx == 1)
		{
			return m_FALSE;
		}
		return NULL;
	}
	/** DISHONORED(port): 2013 rva 0x5098c0 (2012 0x54a150). */
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node)
	{
		if (Idx == 0)
		{
			m_TRUE = Node;
			return TRUE;
		}
		if (Idx == 1)
		{
			m_FALSE = Node;
			return TRUE;
		}
		return FALSE;
	}
	/** DISHONORED(port): 2013 rva 0x5098f0 (2012 0x54a180) / 0x54a1c0. */
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		if (Node == m_TRUE)
		{
			UnlinkInput((UINT)0);
		}
		if (Node == m_FALSE)
		{
			UnlinkInput((UINT)1);
		}
		return TRUE;
	}
	virtual UBOOL UnlinkInput(UINT Idx)
	{
		if (Idx == 0 && m_TRUE)
		{
			m_TRUE = NULL;
			return TRUE;
		}
		if (Idx == 1 && m_FALSE)
		{
			m_FALSE = NULL;
			return TRUE;
		}
		return FALSE;
	}
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeSwitch,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeMaterial: retail sizeof 228, reflected span 104..228 (2012 PDB sizeof 228). The node that carries
// Dishonored's colour treatment: it draws m_Material over m_SurfaceTarget's surface with up to eight of the graph's
// other nodes bound as textures.
class UArkPpNodeMaterial : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeMaterial
	class UArkPpNode* m_SurfaceTarget;
	TArrayNoInit<FAnInput> m_Inputs;
	class UMaterialInterface* m_Material;
	BITFIELD m_bPreserveAlphaChannel:1;
	BITFIELD m_bOverrideUberPp:1;
	FArkUberPpParameters m_UberParameters;
	FLOAT m_UberParametersWeight;
	//## END PROPS ArkPpNodeMaterial

	/** DISHONORED(port): 2013 rva 0x51df40 (2012 0x55f260) - input 0 is the surface, then one per m_Inputs entry. */
	virtual UINT NumInputs() const { return m_Inputs.Num() + 1; }
	/** DISHONORED(port): 2013 rva 0x50f2e0 (2012 0x54f9d0) - the surface has no name, an input its m_Name. */
	virtual FString InputName(UINT Idx) const
	{
		if (Idx >= NumInputs())
		{
			return FString(TEXT("Error"));
		}
		return Idx ? FString(m_Inputs(Idx - 1).m_Name) : FString(TEXT(""));
	}
	/** DISHONORED(port): 2013 rva 0x51df50 (2012 0x55f270). */
	virtual UArkPpNode* GetInput(UINT Idx)
	{
		if (Idx == 0)
		{
			return m_SurfaceTarget;
		}
		return m_Inputs(Idx - 1).m_Node;
	}
	/** DISHONORED(port): 2013 rva 0x50f450 (2012 0x54fb40). */
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node)
	{
		if (Idx >= NumInputs())
		{
			return FALSE;
		}
		if (Idx == 0)
		{
			m_SurfaceTarget = Node;
		}
		else
		{
			m_Inputs(Idx - 1).m_Node = Node;
		}
		return TRUE;
	}
	/** DISHONORED(port): 2013 rva 0x50f4d0 (2012 0x54fbc0) / 0x54fc60. */
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		UBOOL bUnlinked = FALSE;
		if (m_SurfaceTarget == Node)
		{
			bUnlinked = UnlinkInput((UINT)0);
		}
		for (INT InputIndex = 0; InputIndex < m_Inputs.Num(); InputIndex++)
		{
			if (m_Inputs(InputIndex).m_Node == Node)
			{
				bUnlinked |= UnlinkInput((UINT)(InputIndex + 1));
			}
		}
		return bUnlinked;
	}
	virtual UBOOL UnlinkInput(UINT Idx)
	{
		if (Idx >= NumInputs())
		{
			return FALSE;
		}
		if (Idx == 0)
		{
			if (!m_SurfaceTarget)
			{
				return TRUE;
			}
			m_SurfaceTarget = NULL;
		}
		else
		{
			m_Inputs(Idx - 1).m_Node = NULL;
		}
		return TRUE;
	}
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeMaterial,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeBlur: retail sizeof 256, reflected span 104..256 (2012 PDB sizeof 256)
class UArkPpNodeBlur : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeBlur
	BYTE m_Type;
	FBoxBlurConfig m_BoxBlurConfig;
	FMotionBlurConfig m_MotionBlurConfig;
	FRadialBlurConfig m_RadialConfig;
	BITFIELD m_bOverrideUberPp:1;
	FArkUberPpParameters m_UberParameters;
	FLOAT m_UberParametersWeight;
	class UArkPpNode* m_Input;
	class UArkPpNode* m_Output;
	class UArkPpNode* m_VectorField;
	//## END PROPS ArkPpNodeBlur

	/** DISHONORED(port): 2013 rva 0x509530 (2012 0x549dd0) - the vector field is an input of motion blur only. */
	virtual UINT NumInputs() const { return (m_Type == EPpBt_Motion) ? 3 : 2; }
	virtual FString InputName(UINT Idx) const { return FString(Idx == 0 ? TEXT("In") : (Idx == 1 ? TEXT("Out") : TEXT("Vectors"))); }
	/** DISHONORED(port): 2013 rva 0x509540 (2012 0x549de0) / 0x549d00. */
	virtual UArkPpNode* GetInput(UINT Idx)
	{
		switch (Idx)
		{
		case 0: return m_Input;
		case 1: return m_Output;
		case 2: return m_VectorField;
		}
		return NULL;
	}
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node)
	{
		switch (Idx)
		{
		case 0: m_Input = Node; return TRUE;
		case 1: m_Output = Node; return TRUE;
		case 2: m_VectorField = Node; return TRUE;
		}
		return FALSE;
	}
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		UBOOL bUnlinked = FALSE;
		for (UINT Idx = 0; Idx < NumInputs(); Idx++)
		{
			if (GetInput(Idx) == Node)
			{
				bUnlinked |= UnlinkInput(Idx);
			}
		}
		return bUnlinked;
	}
	virtual UBOOL UnlinkInput(UINT Idx) { return LinkInput(Idx,NULL); }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeBlur,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeDof: retail sizeof 208, reflected span 104..208 (2012 PDB sizeof 208)
class UArkPpNodeDof : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeDof
	class UArkPpNode* m_SurfaceTarget;
	class UTexture2D* m_LinearToGammaRamp;
	FArkUberPpParameters m_Parameters;
	//## END PROPS ArkPpNodeDof

	virtual UINT NumInputs() const { return 1; }
	virtual FString InputName(UINT Idx) const { return FString(TEXT("In")); }
	/** DISHONORED(port): 2013 rva 0x51cfa0 (2012 0x559330) / 0x549e20 / 0x549e40. */
	virtual UArkPpNode* GetInput(UINT Idx) { return m_SurfaceTarget; }
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node) { m_SurfaceTarget = Node; return TRUE; }
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		if (m_SurfaceTarget == Node)
		{
			m_SurfaceTarget = NULL;
			return TRUE;
		}
		return FALSE;
	}
	virtual UBOOL UnlinkInput(UINT Idx) { m_SurfaceTarget = NULL; return TRUE; }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeDof,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeKuwa: retail sizeof 120, reflected span 104..120 (2012 PDB sizeof 120)
class UArkPpNodeKuwa : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeKuwa
	INT m_Type;
	FLOAT m_Strength;
	class UArkPpNode* m_SurfaceTarget;
	class UArkPpNode* m_SrcColor;
	//## END PROPS ArkPpNodeKuwa

	virtual UINT NumInputs() const { return 2; }
	virtual FString InputName(UINT Idx) const { return FString(Idx ? TEXT("In") : TEXT("Out")); }
	/** DISHONORED(port): 2013 rva 0x51cfb0 (2012 0x5593d0) / 0x549e80. */
	virtual UArkPpNode* GetInput(UINT Idx) { return Idx ? m_SrcColor : m_SurfaceTarget; }
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node)
	{
		if (Idx)
		{
			m_SrcColor = Node;
		}
		else
		{
			m_SurfaceTarget = Node;
		}
		return TRUE;
	}
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		UBOOL bUnlinked = FALSE;
		if (m_SurfaceTarget == Node)
		{
			m_SurfaceTarget = NULL;
			bUnlinked = TRUE;
		}
		if (m_SrcColor == Node)
		{
			m_SrcColor = NULL;
			bUnlinked = TRUE;
		}
		return bUnlinked;
	}
	virtual UBOOL UnlinkInput(UINT Idx) { return LinkInput(Idx,NULL); }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeKuwa,UArkPpNode,0,Engine)
};

// Engine.ArkPpNodeAA: retail sizeof 144, reflected span 104..144 (2012 PDB sizeof 144)
class UArkPpNodeAA : public UArkPpNode
{
public:
	//## BEGIN PROPS ArkPpNodeAA
	BYTE m_Type;
	FFxAaConfig m_FxAaConfig;
	FMlAaConfig m_MlAaConfig;
	class UArkPpNode* m_SurfaceTarget;
	//## END PROPS ArkPpNodeAA

	virtual UINT NumInputs() const { return 1; }
	virtual FString InputName(UINT Idx) const { return FString(TEXT("In")); }
	/** DISHONORED(port): 2013 rva 0x51cf60 (2012 0x559170) / 0x549c90. */
	virtual UArkPpNode* GetInput(UINT Idx) { return m_SurfaceTarget; }
	virtual UBOOL LinkInput(UINT Idx,UArkPpNode* Node) { m_SurfaceTarget = Node; return TRUE; }
	virtual UBOOL UnlinkInput(UArkPpNode* Node)
	{
		if (m_SurfaceTarget == Node)
		{
			m_SurfaceTarget = NULL;
			return TRUE;
		}
		return FALSE;
	}
	virtual UBOOL UnlinkInput(UINT Idx) { m_SurfaceTarget = NULL; return TRUE; }
	virtual UBOOL IsValid(FArkPpIsValidData& Cache);
	virtual FArkPpNodeProxy* CreateSceneProxy(FArkPpCreateProxyConfig& Config);

	DECLARE_CLASS(UArkPpNodeAA,UArkPpNode,0,Engine)
};

// Engine.ArkPpSettings: retail sizeof 56, reflected span 56..56 (2012 PDB sizeof 56)
class UArkPpSettings : public UObject
{
public:
	//## BEGIN PROPS ArkPpSettings
	//## END PROPS ArkPpSettings

	DECLARE_CLASS(UArkPpSettings,UObject,0,Engine)
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

static_assert(sizeof(FAnInput) == 20, "FAnInput: retail SDK size 20");
static_assert(sizeof(FBoxBlurConfig) == 4, "FBoxBlurConfig: retail SDK size 4");
static_assert(sizeof(FMotionBlurConfig) == 16, "FMotionBlurConfig: retail SDK size 16");
static_assert(sizeof(FRadialBlurConfig) == 12, "FRadialBlurConfig: retail SDK size 12");
static_assert(sizeof(FFxAaConfig) == 16, "FFxAaConfig: retail SDK size 16");
static_assert(sizeof(FMlAaConfig) == 16, "FMlAaConfig: retail SDK size 16");
static_assert(sizeof(UArkPpNodeController) == 56, "UArkPpNodeController: retail sizeof 56");
static_assert(sizeof(UArkPpNode) == 104, "UArkPpNode: retail sizeof 104");
static_assert(sizeof(UArkPpNodeSceneColor) == 112, "UArkPpNodeSceneColor: retail sizeof 112");
static_assert(sizeof(UArkPpNodeTarget) == 108, "UArkPpNodeTarget: retail sizeof 108");
static_assert(sizeof(UArkPpNodeCommonTarget) == 128, "UArkPpNodeCommonTarget: retail sizeof 128");
static_assert(sizeof(UArkPpNodeSwitch) == 128, "UArkPpNodeSwitch: retail sizeof 128");
static_assert(sizeof(UArkPpNodeMaterial) == 228, "UArkPpNodeMaterial: retail sizeof 228");
static_assert(sizeof(UArkPpNodeBlur) == 256, "UArkPpNodeBlur: retail sizeof 256");
static_assert(sizeof(UArkPpNodeDof) == 208, "UArkPpNodeDof: retail sizeof 208");
static_assert(sizeof(UArkPpNodeKuwa) == 120, "UArkPpNodeKuwa: retail sizeof 120");
static_assert(sizeof(UArkPpNodeAA) == 144, "UArkPpNodeAA: retail sizeof 144");
static_assert(sizeof(UArkPpSettings) == 56, "UArkPpSettings: retail sizeof 56");
static_assert(STRUCT_OFFSET(UArkPpNode, EffectName) == 60, "UArkPpNode::EffectName: retail offset 60");
static_assert(STRUCT_OFFSET(UArkPpNode, m_Controller) == 68, "UArkPpNode::m_Controller: retail offset 68");
static_assert(STRUCT_OFFSET(UArkPpNode, InDrawY) == 92, "UArkPpNode::InDrawY: retail offset 92");
static_assert(STRUCT_OFFSET(UArkPpNodeMaterial, m_SurfaceTarget) == 104, "UArkPpNodeMaterial::m_SurfaceTarget: retail offset 104");
static_assert(STRUCT_OFFSET(UArkPpNodeMaterial, m_Inputs) == 108, "UArkPpNodeMaterial::m_Inputs: retail offset 108");
static_assert(STRUCT_OFFSET(UArkPpNodeMaterial, m_Material) == 120, "UArkPpNodeMaterial::m_Material: retail offset 120");
static_assert(STRUCT_OFFSET(UArkPpNodeMaterial, m_UberParameters) == 128, "UArkPpNodeMaterial::m_UberParameters: retail offset 128");
static_assert(STRUCT_OFFSET(UArkPpNodeMaterial, m_UberParametersWeight) == 224, "UArkPpNodeMaterial::m_UberParametersWeight: retail offset 224");
static_assert(STRUCT_OFFSET(UArkPpNodeSwitch, m_Switch) == 116, "UArkPpNodeSwitch::m_Switch: retail offset 116");
static_assert(STRUCT_OFFSET(UArkPpNodeCommonTarget, m_ClearColor) == 112, "UArkPpNodeCommonTarget::m_ClearColor: retail offset 112");
static_assert(STRUCT_OFFSET(UArkPpNodeBlur, m_UberParameters) == 144, "UArkPpNodeBlur::m_UberParameters: retail offset 144");
static_assert(STRUCT_OFFSET(UArkPpNodeBlur, m_VectorField) == 252, "UArkPpNodeBlur::m_VectorField: retail offset 252");
static_assert(STRUCT_OFFSET(UArkPpNodeDof, m_Parameters) == 112, "UArkPpNodeDof::m_Parameters: retail offset 112");
static_assert(STRUCT_OFFSET(UArkPpNodeKuwa, m_SrcColor) == 116, "UArkPpNodeKuwa::m_SrcColor: retail offset 116");
static_assert(STRUCT_OFFSET(UArkPpNodeAA, m_SurfaceTarget) == 140, "UArkPpNodeAA::m_SurfaceTarget: retail offset 140");

#endif // !INCLUDED_ENGINE_ARKPP_CLASSES
#endif // !NAMES_ONLY

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE_ARKPP \
	UArkPpNodeController::StaticClass(); \
	UArkPpNode::StaticClass(); \
	UArkPpNodeSceneColor::StaticClass(); \
	UArkPpNodeTarget::StaticClass(); \
	UArkPpNodeCommonTarget::StaticClass(); \
	UArkPpNodeSwitch::StaticClass(); \
	UArkPpNodeMaterial::StaticClass(); \
	UArkPpNodeBlur::StaticClass(); \
	UArkPpNodeDof::StaticClass(); \
	UArkPpNodeKuwa::StaticClass(); \
	UArkPpNodeAA::StaticClass(); \
	UArkPpSettings::StaticClass();
