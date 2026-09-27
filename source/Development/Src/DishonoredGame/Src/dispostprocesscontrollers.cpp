/*=============================================================================
	dispostprocesscontrollers.cpp: the post-process node controllers.

	DISHONORED(port): a UArkPpNodeController is asked three things by the graph it sits in: whether its node draws at
	all (IsShown, UArkPpNode::IsShownInConfig), what material and uber-parameter override the node should use this
	frame (Update, called from FArkPpNodeMaterialProxy's constructor) and to advance its own timers (Tick). The timers
	are what 'is my effect running' means, and every controller at rest answers its node hidden.

	Not ported, and why:
	* UDisBlindedPpController::Update (2013 rva 0x7ea620, 2012 0x84ba30) builds its material instance at runtime and
	  drives it from a 1D Perlin noise over a global clock (SmoothIntNoise 2012 0x8493f0, PerlineNoise1D 0x84b980).
	  The node the content gives it (TEST_PPG_Blinded_INST in Test_PPG) is m_bShowInGame 0, so nothing reaches it in
	  game; the base's NULL return leaves the node's own material in place.
	* UDisDarkVisionMeshRenderPpController::Render is a mesh pass of its own -
	  (2013 rva 0x7fe690) TSoulPartMeshDrawingPolicy<FSoulPartMeshPolicy> with its own shader types - not a post-process
	  pass, and no node in the shipped chain has this controller.
	* Nothing in this tree calls UArkPpNodeController::Tick. In retail the call sits behind
	  ADishonoredPlayerController::ModifyPostProcessSettings (2013 rva 0x6adeb0), which reaches the graph's nodes by
	  name and drives the water, health, dark-vision and possession effects; it is unported (agent DE hand-over 1).
	  Until it is, m_CurrentTime, m_EyeLidTime and m_PowerTime stay 0 and both IsShown bodies answer FALSE, which is
	  exactly the rest state they answer in retail.
=============================================================================*/

#include "DishonoredGame.h"
#include "arkpp.h"

// The four classes are registered in DishonoredGameRegistrants.cpp; this unit only carries their bodies.

/**
 * DISHONORED(port): 2013 rva 0x7eaba0 (2012 0x84bfe0) - the opacity controller's fade. m_bIsOn fades m_CurrentTime up over
 * m_FadinDuration and anything else fades it down over m_FadoutDuration; both ends clamp.
 *
 * DISHONORED(retail): the rise is gated on m_bIsOn alone (bit 1 of the bitfield word), while IsShown also accepts
 * m_bDoNotDisablePp (bit 0) - so a controller with m_bDoNotDisablePp set and m_bIsOn clear keeps its node drawn while
 * the fade runs *down*, which is what that flag's name says.
 */
UBOOL UDisOpacityParameterPpController::Tick(FLOAT DeltaTime,enum ELevelTick TickType)
{
	if (m_bIsOn)
	{
		if (m_CurrentTime < 1.0f)
		{
			const FLOAT Next = m_CurrentTime + DeltaTime / m_FadinDuration;
			m_CurrentTime = (Next >= 1.0f) ? 1.0f : Next;
		}
	}
	else if (m_CurrentTime > 0.0f)
	{
		const FLOAT Next = m_CurrentTime - DeltaTime / m_FadoutDuration;
		m_CurrentTime = (Next <= 0.0f) ? 0.0f : Next;
	}
	return TRUE;
}

/** DISHONORED(port): 2013 rva 0x7e7ca0 (2012 0x849540) - the node draws while the effect is on or still fading out. */
UBOOL UDisOpacityParameterPpController::IsShown(const UArkPpNode* Node)
{
	return m_bDoNotDisablePp || m_bIsOn || m_CurrentTime > 0.0f;
}

/**
 * DISHONORED(port): the runtime material every controller's Update starts with (the first half of 2013 0x7eac10,
 * 0x7ee630 and 0x7ea620, byte for byte the same in all three): a UMaterialInstance parent is duplicated into the
 * transient package, anything else becomes a fresh UMaterialInstanceConstant with the node's material as its parent.
 */
static UMaterialInstance* ArkPpMakeRuntimeMaterial(UMaterialInterface* Parent)
{
	UMaterialInstance* AsInstance = Cast<UMaterialInstance>(Parent);
	if (AsInstance)
	{
		return Cast<UMaterialInstance>(UObject::StaticDuplicateObject(AsInstance,AsInstance,UObject::GetTransientPackage(),TEXT("None")));
	}
	UMaterialInstanceConstant* Constant = ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(),UObject::GetTransientPackage());
	Constant->SetParent(Parent);
	return Constant;
}

/**
 * DISHONORED(port): 2013 rva 0x7eac10 (2012 0x84c050) - the opacity controller pushes its fade into one named vector parameter of its
 * own copy of the node's material, in all four channels, and overrides no uber parameter.
 */
UMaterialInterface* UDisOpacityParameterPpController::Update(const UArkPpNodeMaterial* Node,UBOOL* bOutOverrideUber,FLOAT* OutWeight,FArkUberPpParameters* InOutParams)
{
	if (!m_RuntimeMaterial)
	{
		m_RuntimeMaterial = ArkPpMakeRuntimeMaterial(Node->m_Material);
	}
	if (m_RuntimeMaterial)
	{
		m_RuntimeMaterial->SetVectorParameterValue(m_ParameterName,FLinearColor(m_CurrentTime,m_CurrentTime,m_CurrentTime,m_CurrentTime));
	}
	return m_RuntimeMaterial;
}

/**
 * DISHONORED(port): 2013 rva 0x7e7b00 (2012 0x8493c0) - the blind effect's only clock, a file-scope accumulator its Update samples
 * the noise with. Tick runs whether or not the effect is on, exactly as retail has it.
 */
FLOAT GBlindedT = 0.0f;

UBOOL UDisBlindedPpController::Tick(FLOAT DeltaTime,enum ELevelTick TickType)
{
	GBlindedT += DeltaTime;
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x7eaab0 (2012 0x84be90) - dark vision runs in two stages and the eyelid is the first. While the power is
 * on, m_EyeLidTime rises over m_EyeLidClosingDuration until the lid is shut, and only then does m_PowerTime rise over
 * m_PowerFadeInDuration; off, m_PowerTime falls over m_PowerFadeOutDuration first and the lid opens over
 * m_EyeLidOpeningDuration after it reaches zero. Both ends clamp to the stage's own target.
 */
UBOOL UDisDarkVisionPpController::Tick(FLOAT DeltaTime,enum ELevelTick TickType)
{
	if (m_bIsActive || m_bDebugIsActive)
	{
		if (m_EyeLidTime >= 1.0f)
		{
			const FLOAT Next = m_PowerTime + DeltaTime / m_LevelParameters.m_PowerFadeInDuration;
			m_PowerTime = (Next < 1.0f) ? Next : 1.0f;
		}
		else
		{
			const FLOAT Next = m_EyeLidTime + DeltaTime / m_LevelParameters.m_EyeLidClosingDuration;
			m_EyeLidTime = (Next < 1.0f) ? Next : 1.0f;
		}
	}
	else if (m_PowerTime > 0.0f)
	{
		const FLOAT Next = m_PowerTime - DeltaTime / m_LevelParameters.m_PowerFadeOutDuration;
		m_PowerTime = (Next > 0.0f) ? Next : 0.0f;
	}
	else
	{
		const FLOAT Next = m_EyeLidTime - DeltaTime / m_LevelParameters.m_EyeLidOpeningDuration;
		m_EyeLidTime = (Next > 0.0f) ? Next : 0.0f;
	}
	return TRUE;
}

/** DISHONORED(port): 2013 rva 0x7e7c30 (2012 0x8494e0) - shown while the power is on or either stage is unfinished. */
UBOOL UDisDarkVisionPpController::IsShown(const UArkPpNode* Node)
{
	return m_bIsActive || m_bDebugIsActive || m_EyeLidTime > 0.0f || m_PowerTime > 0.0f;
}

/**
 * DISHONORED(port): 2013 rva 0x7ee630 (2012 0x84ffb0) - which of the two materials the node draws this frame, and the only controller
 * that pushes an uber override. Before the lid is shut it is the eyelid material at m_EyeLidTime and no override;
 * after it, the power material at m_PowerTime, plus the static uber adjustment at full weight and the dynamic one at
 * m_PowerTime, which is how dark vision desaturates and lifts the world as it fades in.
 *
 * DISHONORED(retail): both branches set the same two parameters, `Alpha` and `EyeLidColor`, and both take
 * EyeLidColor from m_ClosedLidColor - the power material gets the closed-lid colour too.
 */
UMaterialInterface* UDisDarkVisionPpController::Update(const UArkPpNodeMaterial* Node,UBOOL* bOutOverrideUber,FLOAT* OutWeight,FArkUberPpParameters* InOutParams)
{
	if (!m_RuntimeEyeLidMaterial)
	{
		m_RuntimeEyeLidMaterial = ArkPpMakeRuntimeMaterial(m_GlobalParameters.m_EyeLidMaterial);
	}
	if (!m_RuntimePowerMaterial)
	{
		m_RuntimePowerMaterial = ArkPpMakeRuntimeMaterial(m_GlobalParameters.m_PowerMaterial);
	}

	static FName AlphaName = FName(TEXT("Alpha"));
	static FName EyeLidColorName = FName(TEXT("EyeLidColor"));

	if (m_EyeLidTime >= 1.0f)
	{
		if (m_RuntimePowerMaterial)
		{
			m_RuntimePowerMaterial->SetVectorParameterValue(AlphaName,FLinearColor(m_PowerTime,m_PowerTime,m_PowerTime,m_PowerTime));
			m_RuntimePowerMaterial->SetVectorParameterValue(EyeLidColorName,FLinearColor(m_GlobalParameters.m_ClosedLidColor));
		}
		*bOutOverrideUber = TRUE;
		*OutWeight = 1.0f;
		ArkUberPpApplyTo(m_GlobalParameters.m_StaticUberAdjustement,*InOutParams,1.0f,FALSE);
		ArkUberPpApplyTo(m_GlobalParameters.m_DynamicUberAdjustement,*InOutParams,m_PowerTime,FALSE);
		return m_RuntimePowerMaterial;
	}

	if (m_RuntimeEyeLidMaterial)
	{
		m_RuntimeEyeLidMaterial->SetVectorParameterValue(AlphaName,FLinearColor(m_EyeLidTime,m_EyeLidTime,m_EyeLidTime,m_EyeLidTime));
		m_RuntimeEyeLidMaterial->SetVectorParameterValue(EyeLidColorName,FLinearColor(m_GlobalParameters.m_ClosedLidColor));
	}
	*bOutOverrideUber = FALSE;
	return m_RuntimeEyeLidMaterial;
}

/*---------------------------------------------------------------------------
	The generated stub's PDB inventory of this compiland, kept as the reference list of what is here and what is not:

// DishonoredGame/src/dispostprocesscontrollers.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x8493a0  public: static void __cdecl UDisBlindedPpController::InitializePrivateStaticClassUDisBlindedPpController(void)
//   0x8493c0  public: virtual unsigned int __thiscall UDisBlindedPpController::Tick(float, enum ELevelTick)
//   0x8493f0  float __cdecl SmoothIntNoise(int)
//   0x8494c0  public: static void __cdecl UDisDarkVisionPpController::InitializePrivateStaticClassUDisDarkVisionPpController(void)
//   0x8494e0  public: virtual unsigned int __thiscall UDisDarkVisionPpController::IsShown(class UArkPpNode const &)
//   0x849520  public: static void __cdecl UDisOpacityParameterPpController::InitializePrivateStaticClassUDisOpacityParameterPpController(void)
//   0x849540  public: virtual unsigned int __thiscall UDisOpacityParameterPpController::IsShown(class UArkPpNode const &)
//   0x849570  public: static void __cdecl UDisDarkVisionMeshRenderPpController::InitializePrivateStaticClassUDisDarkVisionMeshRenderPpController(void)
//   0x84b8f0  public: static class UClass * __cdecl UDisBlindedPpController::GetPrivateStaticClassUDisBlindedPpController(wchar_t const *)
//   0x84b980  float __cdecl PerlineNoise1D(float)
//   0x84ba30  public: virtual class UMaterialInterface * __thiscall UDisBlindedPpController::Update(class UArkPpNodeMaterial const &, unsigned int &, float &, struct FArkUberPpParameters &)
//   0x84be90  public: virtual unsigned int __thiscall UDisDarkVisionPpController::Tick(float, enum ELevelTick)
//   0x84bf50  public: static class UClass * __cdecl UDisOpacityParameterPpController::GetPrivateStaticClassUDisOpacityParameterPpController(wchar_t const *)
//   0x84bfe0  public: virtual unsigned int __thiscall UDisOpacityParameterPpController::Tick(float, enum ELevelTick)
//   0x84c050  public: virtual class UMaterialInterface * __thiscall UDisOpacityParameterPpController::Update(class UArkPpNodeMaterial const &, unsigned int &, float &, struct FArkUberPpParameters &)
//   0x84c150  public: static class UClass * __cdecl UDisDarkVisionMeshRenderPpController::GetPrivateStaticClassUDisDarkVisionMeshRenderPpController(wchar_t const *)
//   0x84ff80  public: static class UClass * __cdecl UDisBlindedPpController::StaticClassNoInline(void)
//   0x84ffb0  public: virtual class UMaterialInterface * __thiscall UDisDarkVisionPpController::Update(class UArkPpNodeMaterial const &, unsigned int &, float &, struct FArkUberPpParameters &)
//   0x850280  public: static class UClass * __cdecl UDisOpacityParameterPpController::StaticClassNoInline(void)
//   0x8502b0  public: static class UClass * __cdecl UDisDarkVisionMeshRenderPpController::StaticClassNoInline(void)
//   0x855620  public: static class UClass * __cdecl UDisDarkVisionPpController::GetPrivateStaticClassUDisDarkVisionPpController(wchar_t const *)
//   0x8573d0  public: static class UClass * __cdecl UDisDarkVisionPpController::StaticClassNoInline(void)
//   0x858870  public: virtual unsigned int __thiscall TDepthOnlyPixelShader::Serialize(class FArchive &)
//   0x858ab0  public: static class FShader * __cdecl TSoulPartMeshVertexShader<class FSoulPartMeshPolicy>::ConstructSerializedInstance(void)
//   0x85d3a0  public: __thiscall TSoulPartMeshDrawingPolicy<class FSoulPartMeshPolicy>::TSoulPartMeshDrawingPolicy<class FSoulPartMeshPolicy>(class FVertexFactory const *, class FMaterialRenderProxy const *)
//   0x85d410  public: void __thiscall TSoulPartMeshDrawingPolicy<class FSoulPartMeshPolicy>::SetMeshRenderState(class FSceneView const &, class FPrimitiveSceneInfo const *, struct FMeshElement const &, unsigned int, struct FMeshDrawingPolicy::ElementDataType const &)const
//   0x85d510  public: void __thiscall TSoulPartMeshDrawingPolicy<class FSoulPartMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x85d640  public: static unsigned int __cdecl TSoulPartMeshDrawingPolicyFactory<class FSoulPartMeshPolicy>::DrawDynamicMesh(class FSceneView const &, unsigned int, struct FMeshElement const &, unsigned int, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x85f480  public: virtual unsigned int __thiscall UDisDarkVisionMeshRenderPpController::Render(enum EPpNodeRenderStage, class FScene const *, class FViewInfo &)
//   0xbaaba0  _dynamic_initializer_for__TSoulPartMeshVertexShader_FSoulPartMeshPolicy_::StaticType__
//   0xbaabe0  _dynamic_initializer_for__TSoulPartMeshPixelShader_FSoulPartMeshPolicy_::StaticType__
---------------------------------------------------------------------------*/
