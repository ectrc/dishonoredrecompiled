// Engine/src/arkppnodematerial.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x54f9d0  public: virtual class FString __thiscall UArkPpNodeMaterial::InputName(unsigned int)const
//   0x54fb40  public: virtual unsigned int __thiscall UArkPpNodeMaterial::LinkInput(unsigned int, class UArkPpNode *)
//   0x54fbc0  public: virtual unsigned int __thiscall UArkPpNodeMaterial::UnlinkInput(class UArkPpNode *)
//   0x54fc60  public: virtual unsigned int __thiscall UArkPpNodeMaterial::UnlinkInput(unsigned int)
//   0x559670  public: __thiscall FArkPpNodeMaterialProxy::FArkPpNodeMaterialProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeMaterial *)
//   0x559990  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::GetSurfaceSizeX(void)
//   0x5599a0  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::GetSurfaceSizeY(void)
//   0x5599b0  public: virtual class TDynamicRHIResourceReference<12> const __thiscall FArkPpNodeMaterialProxy::GetSurface(class FViewInfo const &)
//   0x5599e0  public: virtual class TDynamicRHIResourceReference<14> const __thiscall FArkPpNodeMaterialProxy::GetTexture(class FViewInfo const &)
//   0x55a570  public: class TDynamicRHIResourceReference<9> __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::CreateBoundShaderState(unsigned long)
//   0x55a640  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::SetMeshRenderState(class FSceneView const &, class FPrimitiveSceneInfo const *, struct FMeshElement const &, unsigned int, struct FMeshDrawingPolicy::ElementDataType const &)const
//   0x55a760  public: void __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::SetParameters(class FVertexFactory const *, class FMaterialRenderProxy const *, class FSceneView const *, struct FVector2D, class FTexture const * const)
//   0x55b010  public: __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>(class FVertexFactory const *, class FMaterialRenderProxy const *, struct FPpMaterialContextType, unsigned int)
//   0x55b6b0  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x55b840  public: virtual __thiscall FArkPpNodeMaterialProxy::~FArkPpNodeMaterialProxy(void)
//   0x55d070  public: __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 1>>::TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 1>>(void)
//   0x55d0f0  public: virtual unsigned int __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::Serialize(class FArchive &)
//   0x55d360  public: static unsigned int __cdecl TPpMaterialDrawingPolicyFactory<class FPpMaterialMeshPolicy>::DrawDynamicMesh(class FSceneView const &, struct FPpMaterialContextType, struct FMeshElement const &, unsigned int, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x55e2f0  public: static class FShader * __cdecl TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::ConstructSerializedInstance(void)
//   0x55f3f0  public: static void __cdecl UArkPpNodeMaterial::InitializePrivateStaticClassUArkPpNodeMaterial(void)
//   0x55f410  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x564720  public: static class UClass * __cdecl UArkPpNodeMaterial::GetPrivateStaticClassUArkPpNodeMaterial(wchar_t const *)
//   0x564bf0  public: static class UClass * __cdecl UArkPpNodeMaterial::StaticClassNoInline(void)
//   0x5657a0  public: virtual unsigned int __thiscall UArkPpNodeMaterial::IsValid(struct FArkPpIsValidData &)
//   0x5658c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeMaterial::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f8d0  _dynamic_initializer_for__TPpMaterialVertexShader_FPpMaterialMeshPolicy_::StaticType__
//   0xb9f910  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_0___::StaticType__
//   0xb9f950  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_1___::StaticType__

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/**
 * DISHONORED(written): the shader types of Arkane's post-process material node (UArkPpNodeMaterial, 2013 rva 0x5599b0 ff.),
 * which draws a material over a post-process surface with up to eight bound node textures. Only the types are declared: the
 * cooked material shader maps reference them (renderer.md 2), and without a declaration every such map counts an undeclared
 * type. The FArkPp node graph that drives the pass is wave-5 work (renderer.md 5); nothing selects these shaders yet.
 */
class FPpMaterialMeshPolicy {};
class FPpMaterialLinearSpaceMeshPolicy {};
class FPpMaterialGammaSpaceMeshPolicy {};

/** The number of post-process node textures a pp-material pixel shader can sample (2012 PDB: FShaderResourceParameter[8]). */
#define ARK_PP_MATERIAL_NUM_TEXTURES 8

/**
 * DISHONORED(layout): TPpMaterialVertexShader<FPpMaterialMeshPolicy> is 196 bytes (2012 PDB): FShader,
 * FVertexFactoryParameterRef @108, FMaterialVertexShaderParameters @136 (2012 ctor rva 0x55cfd0).
 */
template<typename MeshPolicyType>
class TPpMaterialVertexShader : public FMeshMaterialVertexShader
{
	DECLARE_SHADER_TYPE(TPpMaterialVertexShader,MeshMaterial);
public:
	TPpMaterialVertexShader() {}

	TPpMaterialVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FMeshMaterialVertexShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
	}

	// DISHONORED(bringup): retail's gating is not recoverable (no shader compiler in the shipping exes); the type accepts
	// every material and compiles nothing, so only the cooked shaders ever exist.
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

	virtual UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& UniformExpressionSet) const
	{
		return MaterialParameters.IsUniformExpressionSetValid(UniformExpressionSet);
	}

private:
	FMaterialVertexShaderParameters MaterialParameters;
};

/**
 * DISHONORED(layout): TPpMaterialPixelShader is 368 bytes (2012 PDB TPpMaterialPixelShader<ArkPpAddGammaValue<Policy,0|1> >):
 * FShader, FMaterialPixelShaderParameters @108, FShaderResourceParameter m_ArkPpTextureSampleParameter[8] @300,
 * m_ScreenPositionScaleBias @348, m_SurfaceResolution @354, m_ScreenResolution @360.
 * DISHONORED(port): 2013 rva 0x51caa0 Serialize: the material parameters, the eight texture samplers, then the three vectors.
 * The registered names are TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy> / <FPpMaterialGammaSpaceMeshPolicy>
 * (the cooked type names), which is why the gamma variant is a policy here and not the retail template's integer argument.
 */
template<typename MeshPolicyType>
class TPpMaterialPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(TPpMaterialPixelShader,MeshMaterial);
public:
	TPpMaterialPixelShader() {}

	TPpMaterialPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
		for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
		{
			// DISHONORED(bringup): the retail parameter names are not in the shipping exes (no Bind); loaded from the caches only
			m_ArkPpTextureSampleParameter[TextureIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("ArkPpTexture_%u"),TextureIndex),TRUE);
		}
		m_ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
		m_SurfaceResolutionParameter.Bind(Initializer.ParameterMap,TEXT("SurfaceResolution"),TRUE);
		m_ScreenResolutionParameter.Bind(Initializer.ParameterMap,TEXT("ScreenResolution"),TRUE);
	}

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
		{
			Ar << m_ArkPpTextureSampleParameter[TextureIndex];
		}
		Ar << m_ScreenPositionScaleBiasParameter;
		Ar << m_SurfaceResolutionParameter;
		Ar << m_ScreenResolutionParameter;
		return bShaderHasOutdatedParameters;
	}

	virtual UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& UniformExpressionSet) const
	{
		return MaterialParameters.IsUniformExpressionSetValid(UniformExpressionSet);
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderResourceParameter m_ArkPpTextureSampleParameter[ARK_PP_MATERIAL_NUM_TEXTURES];
	FShaderParameter m_ScreenPositionScaleBiasParameter;
	FShaderParameter m_SurfaceResolutionParameter;
	FShaderParameter m_ScreenResolutionParameter;
};

// DISHONORED(retail): 2013 rva 0xb83150 ("ArkPpMaterialVertexShader", 786 / 23), 0xb83190 / 0xb831d0
// ("ArkPpMaterialPixelShader", 801 / 23 - the highest material gate in the exe).
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialVertexShader<FPpMaterialMeshPolicy>,TEXT("ArkPpMaterialVertexShader"),TEXT("Main"),SF_Vertex,786,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy>,TEXT("ArkPpMaterialPixelShader"),TEXT("Main"),SF_Pixel,801,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialPixelShader<FPpMaterialGammaSpaceMeshPolicy>,TEXT("ArkPpMaterialPixelShader"),TEXT("Main"),SF_Pixel,801,23);

/**
 * DISHONORED(bringup): see DishonoredLinkArkPartMeshShaderTypes - the static library drops an unreferenced object file, and
 * with it these shader type registrations. Drop this once the FArkPp material node pass is ported and called.
 */
void DishonoredLinkArkPpMaterialShaderTypes()
{
}
