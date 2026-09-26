// Engine/src/arkbloompartsrendering.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (32):
//   0x54b650  public: virtual unsigned int __thiscall FBloomComposePixelShader::Serialize(class FArchive &)
//   0x54b690  public: static class FShader * __cdecl FShadowProjectionVertexShader::ConstructSerializedInstance(void)
//   0x54b700  public: virtual unsigned int __thiscall VisualizeTexturePixelShader::Serialize(class FArchive &)
//   0x54b740  public: virtual __thiscall bloom::FArkGaussianVertexDeclaration::~FArkGaussianVertexDeclaration(void)
//   0x54b9e0  public: virtual unsigned int __thiscall TBloomBlurPixelShader<5>::Serialize(class FArchive &)
//   0x54dca0  public: __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>(class FVertexFactory const *, class FMaterialRenderProxy const *, unsigned int, unsigned int)
//   0x54dd20  public: void __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::SetMeshRenderState(class FSceneView const &, class FPrimitiveSceneInfo const *, struct FMeshElement const &, unsigned int, struct FMeshDrawingPolicy::ElementDataType const &)const
//   0x54e510  public: static class FShader * __cdecl FBloomComposePixelShader::ConstructSerializedInstance(void)
//   0x54e5a0  public: void __thiscall FBloomDownSamplePixelShader::SetParameters(class FViewInfo const &, class FTexture const *)
//   0x54e6c0  public: void __thiscall FArkBloomPartPrimSet::AddScenePrimitive(class FPrimitiveSceneInfo *, class FViewInfo const &)
//   0x54e6d0  public: virtual void __thiscall bloom::FArkGaussianVertexDeclaration::InitRHI(void)
//   0x54ed80  public: static class FShader * __cdecl FSplashPixelShader::ConstructSerializedInstance(void)
//   0x55cfd0  public: __thiscall TPpMaterialVertexShader<class FPpMaterialMeshPolicy>::TPpMaterialVertexShader<class FPpMaterialMeshPolicy>(void)
//   0x55d050  public: virtual unsigned int __thiscall TBasePassVertexShader<class FNoLightMapPolicy>::IsUniformExpressionSetValid(class FUniformExpressionSet const &)const
//   0x55d1b0  public: virtual unsigned int __thiscall FShadowDepthPixelShader::IsUniformExpressionSetValid(class FUniformExpressionSet const &)const
//   0x55e280  public: static class FShader * __cdecl TDistortionMeshPixelShader<class FDistortMeshAccumulatePolicy>::ConstructSerializedInstance(void)
//   0x560890  public: void __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x5609f0  public: class TDynamicRHIResourceReference<9> __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::CreateBoundShaderState(unsigned long)
//   0x560b00  void __cdecl bloom::GaussianBlur(class TDynamicRHIResourceReference<12> const &, class TDynamicRHIResourceReference<14> const &, class TDynamicRHIResourceReference<12> const &, class TDynamicRHIResourceReference<14> const &, unsigned int, unsigned int, struct bloom::URECT<unsigned int>, unsigned int, float)
//   0x564980  public: static unsigned int __cdecl TBloomPartMeshDrawingPolicyFactory<class FBloomPartMeshPolicy>::DrawStaticMesh(class FSceneView const *, unsigned int, class FStaticMesh const &, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x564ad0  public: static unsigned int __cdecl TBloomPartMeshDrawingPolicyFactory<class FBloomPartMeshPolicy>::DrawDynamicMesh(class FSceneView const &, unsigned int, struct FMeshElement const &, unsigned int, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x565e80  public: unsigned int __thiscall FArkBloomPartPrimSet::DrawBloomPrims(class FViewInfo const *, unsigned int, unsigned int)
//   0x566120  private: unsigned int __thiscall FSceneRenderer::RenderBloomParts(unsigned int)
//   0xb9ee50  _dynamic_initializer_for__FBloomComposePixelShader::StaticType__
//   0xb9ee90  _dynamic_initializer_for__FBloomComposeVertexShader::StaticType__
//   0xb9eed0  _dynamic_initializer_for__FBloomDownSampleVertexShader::StaticType__
//   0xb9ef10  _dynamic_initializer_for__FBloomDownSamplePixelShader::StaticType__
//   0xb9ef50  _dynamic_initializer_for__TBloomBlurVertexShader_5_::StaticType__
//   0xb9ef90  _dynamic_initializer_for__TBloomBlurPixelShader_5_::StaticType__
//   0xb9efd0  _dynamic_initializer_for__TBloomPartMeshVertexShader_FBloomPartMeshPolicy_::StaticType__
//   0xb9f010  _dynamic_initializer_for__TBloomPartMeshPixelShader_FBloomPartMeshPolicy_::StaticType__
//   0xb9f050  bloom::_dynamic_initializer_for__GArkGaussianVertexDeclaration__

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/**
 * DISHONORED(written): Arkane's bloom-part and soul-part mesh passes draw the primitives of FArkBloomPartPrimSet /
 * FViewInfo::m_VisibleSoulPrimitives with the material's own shaders (2013 rva 0x566120 FSceneRenderer::RenderBloomParts).
 * Only the shader types are declared here: the cooked material shader maps of Startup.upk / the level packages reference them
 * (renderer.md 2), so without a declaration every map that uses them counts an undeclared type. Their passes stay unported
 * (wave 5, with the FArkPp graph); nothing selects these shaders at runtime yet.
 */
class FBloomPartMeshPolicy {};
class FSoulPartMeshPolicy {};

/**
 * DISHONORED(layout): TBloomPartMeshVertexShader<FBloomPartMeshPolicy> / TSoulPartMeshVertexShader<FSoulPartMeshPolicy> are
 * 196 bytes (2012 PDB): FShader, FVertexFactoryParameterRef @108, FMaterialVertexShaderParameters @136 - the
 * FMeshMaterialVertexShader shape without any extra parameter.
 */
template<typename MeshPolicyType>
class TArkPartMeshVertexShader : public FMeshMaterialVertexShader
{
public:
	TArkPartMeshVertexShader() {}

	TArkPartMeshVertexShader(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer):
		FMeshMaterialVertexShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
	}

	// DISHONORED(bringup): retail's gating is not recoverable from the shipping exes (no shader compiler); the cooked cache is
	// the only source of these shaders, so the type accepts every material and compiles nothing.
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
 * DISHONORED(layout): TBloomPartMeshPixelShader<FBloomPartMeshPolicy> / TSoulPartMeshPixelShader<FSoulPartMeshPolicy> are
 * 300 bytes (2012 PDB): FShader plus FMaterialPixelShaderParameters @108 and nothing else.
 */
template<typename MeshPolicyType>
class TArkPartMeshPixelShader : public FShader
{
public:
	TArkPartMeshPixelShader() {}

	TArkPartMeshPixelShader(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
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
		return bShaderHasOutdatedParameters;
	}

	virtual UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& UniformExpressionSet) const
	{
		return MaterialParameters.IsUniformExpressionSetValid(UniformExpressionSet);
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
};

/** The cooked type names are the template-ids below, so the shader classes must be named exactly like this. */
template<typename MeshPolicyType>
class TBloomPartMeshVertexShader : public TArkPartMeshVertexShader<MeshPolicyType>
{
	DECLARE_SHADER_TYPE(TBloomPartMeshVertexShader,MeshMaterial);
public:
	TBloomPartMeshVertexShader() {}
	TBloomPartMeshVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		TArkPartMeshVertexShader<MeshPolicyType>(Initializer) {}
};

template<typename MeshPolicyType>
class TBloomPartMeshPixelShader : public TArkPartMeshPixelShader<MeshPolicyType>
{
	DECLARE_SHADER_TYPE(TBloomPartMeshPixelShader,MeshMaterial);
public:
	TBloomPartMeshPixelShader() {}
	TBloomPartMeshPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		TArkPartMeshPixelShader<MeshPolicyType>(Initializer) {}
};

template<typename MeshPolicyType>
class TSoulPartMeshVertexShader : public TArkPartMeshVertexShader<MeshPolicyType>
{
	DECLARE_SHADER_TYPE(TSoulPartMeshVertexShader,MeshMaterial);
public:
	TSoulPartMeshVertexShader() {}
	TSoulPartMeshVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		TArkPartMeshVertexShader<MeshPolicyType>(Initializer) {}
};

template<typename MeshPolicyType>
class TSoulPartMeshPixelShader : public TArkPartMeshPixelShader<MeshPolicyType>
{
	DECLARE_SHADER_TYPE(TSoulPartMeshPixelShader,MeshMaterial);
public:
	TSoulPartMeshPixelShader() {}
	TSoulPartMeshPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		TArkPartMeshPixelShader<MeshPolicyType>(Initializer) {}
};

// DISHONORED(retail): 2013 rva 0xb82850 / 0xb82890 (bloom part, "ArkBloomPartVertexShader" / "ArkBloomPartPixelShader",
// 786 / 23 and 798 / 23) and 0xb8efe0 / 0xb8f020 (soul part, "ArkSoulPartVertexShader" / "ArkSoulPartPixelShader",
// 786 / 24 and 798 / 24; retail declares the soul-part pair in DishonoredGame/Src/dispostprocesscontrollers.cpp, which is
// not ported yet - move them there with that unit).
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBloomPartMeshVertexShader<FBloomPartMeshPolicy>,TEXT("ArkBloomPartVertexShader"),TEXT("Main"),SF_Vertex,786,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBloomPartMeshPixelShader<FBloomPartMeshPolicy>,TEXT("ArkBloomPartPixelShader"),TEXT("Main"),SF_Pixel,798,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TSoulPartMeshVertexShader<FSoulPartMeshPolicy>,TEXT("ArkSoulPartVertexShader"),TEXT("Main"),SF_Vertex,786,24);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TSoulPartMeshPixelShader<FSoulPartMeshPolicy>,TEXT("ArkSoulPartPixelShader"),TEXT("Main"),SF_Pixel,798,24);

/**
 * DISHONORED(bringup): Engine is a static library, so an object file that nothing references is never pulled into the exe and
 * its shader type initializers never run (the cooked maps then report these types as undeclared). BasePassRendering.cpp takes
 * the address of this function to force the link; drop it once the bloom-part / soul-part passes are ported and called.
 */
void DishonoredLinkArkPartMeshShaderTypes()
{
}
