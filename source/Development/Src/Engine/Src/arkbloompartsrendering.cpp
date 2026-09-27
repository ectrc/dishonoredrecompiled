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

	// DISHONORED(port): 2013 rva 0x51fc10 / 0x50d6c0 set these through the vertex factory reference and the material
	// parameters and nothing else; the part shaders have no parameter of their own.
	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		VertexFactoryParameters.Set(this,VertexFactory,*View);
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy,*MaterialRenderProxy->GetMaterial(),View->Family->CurrentWorldTime,View->Family->CurrentRealTime,View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FPrimitiveSceneInfo* PrimitiveSceneInfo,const FMeshBatch& Mesh,INT BatchElementIndex,const FSceneView& View)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,BatchElementIndex,View);
		MaterialParameters.SetMesh(this,PrimitiveSceneInfo,Mesh,BatchElementIndex,View);
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

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy,*MaterialRenderProxy->GetMaterial(),View->Family->CurrentWorldTime,View->Family->CurrentRealTime,View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FPrimitiveSceneInfo* PrimitiveSceneInfo,const FMeshBatch& Mesh,INT BatchElementIndex,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,PrimitiveSceneInfo,Mesh,BatchElementIndex,View,bBackFace);
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

/*-----------------------------------------------------------------------------
	DISHONORED(port): the global shaders of Arkane's bloom (2013 rva 0xb826d0 .. 0xb82810, source
	ArkBloomPartVertexShader / ArkBloomPartPixelShader, entry points MainDownSample, MainBlur, MainCompose, 786 / 23).
	FSceneRenderer::RenderBloomParts (2013 rva 0x5251a0) draws the bloom-part primitives of FArkBloomPartPrimSet into
	the quarter-size bloom target with the mesh shaders above, downsamples, blurs it five taps at a time between
	m_BloomRT and m_BloomRT2, and composes the result back over scene colour.
-----------------------------------------------------------------------------*/

/** DISHONORED(layout): no parameters (the cooked record has 0 parameter words). */
class FBloomComposeVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FBloomComposeVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FBloomComposeVertexShader() {}

	FBloomComposeVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}
};

/** DISHONORED(layout): the five scene texture parameters and the bloom texture (2013 rva 0x50b650 Serialize). */
class FBloomComposePixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FBloomComposePixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FBloomComposePixelShader() {}

	FBloomComposePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mSceneTextureParameters.Bind(Initializer.ParameterMap);
		mBloomTexture.Bind(Initializer.ParameterMap,TEXT("BloomTexture"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSceneTextureParameters;
		Ar << mBloomTexture;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x50e520 - scene colour and the blurred bloom target. */
	void SetParameters(const FSceneView* View,const FTexture2DRHIRef& BloomTexture)
	{
		mSceneTextureParameters.Set(View,this,SF_Point);
		SetTextureParameterDirectly(GetPixelShader(),mBloomTexture,TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),BloomTexture);
	}

private:
	FSceneTextureShaderParameters mSceneTextureParameters;
	FShaderResourceParameter mBloomTexture;
};

/** DISHONORED(layout): no parameters. */
class FBloomDownSampleVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FBloomDownSampleVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FBloomDownSampleVertexShader() {}

	FBloomDownSampleVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}
};

/**
 * DISHONORED(layout): two parameters, the source texture and the bloom tint (2013 rva 0x50e5a0 SetParameters, which
 * takes the tint and threshold from FSceneView::m_ArkPpConfig->m_PpBloomParameters: tint * scale in RGB and
 * -threshold * scale in alpha). This is where a colour-scale volume's bloom settings reach the image.
 */
class FBloomDownSamplePixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FBloomDownSamplePixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FBloomDownSamplePixelShader() {}

	FBloomDownSamplePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mBloomTexture.Bind(Initializer.ParameterMap,TEXT("BloomTexture"),TRUE);
		mBloomTint.Bind(Initializer.ParameterMap,TEXT("BloomTint"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mBloomTexture;
		Ar << mBloomTint;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x50e5a0. */
	void SetParameters(const FSceneView* View,const FTexture2DRHIRef& SourceTexture)
	{
		SetTextureParameterDirectly(GetPixelShader(),mBloomTexture,TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),SourceTexture);

		FLinearColor BloomTint(1.0f,1.0f,1.0f,0.0f);
		if (View->m_ArkPpConfig)
		{
			const FArkPpBloomParameters& Bloom = View->m_ArkPpConfig->m_PpBloomParameters;
			BloomTint = FLinearColor(Bloom.m_Tint.R * Bloom.m_Scale,Bloom.m_Tint.G * Bloom.m_Scale,Bloom.m_Tint.B * Bloom.m_Scale,-(Bloom.m_Threshold * Bloom.m_Scale));
		}
		SetPixelShaderValue(GetPixelShader(),mBloomTint,BloomTint);
	}

private:
	FShaderResourceParameter mBloomTexture;
	FShaderParameter mBloomTint;
};

/**
 * DISHONORED(layout): one parameter at offset 108, and bloom::GaussianBlur (2013 rva 0x51fe50) writes one 16-byte value
 * into it per pass: (1,0,0,0) for the horizontal pass and (0,1,0,0) for the vertical one. It is a direction, not a
 * NumSamples-long array of offsets - the taps come from the source texel size the vertex carries in zw, and the weights
 * are baked into MainBlur. The bound name cannot be recovered (a cooked shader's parameters are serialized, never
 * bound by name, and the retail exe has no compiled-initializer constructor), so the reference name is kept.
 */
template<UINT NumSamples>
class TBloomBlurVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TBloomBlurVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TBloomBlurVertexShader() {}

	TBloomBlurVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		SampleOffsetsParameter.Bind(Initializer.ParameterMap,TEXT("SampleOffsets"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SampleOffsetsParameter;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x51ff1f - SetVertexShaderValue of one FVector4. */
	void SetParameters(const FVector4& Direction)
	{
		SetVertexShaderValue(GetVertexShader(),SampleOffsetsParameter,Direction);
	}

private:
	FShaderParameter SampleOffsetsParameter;
};

/** DISHONORED(layout): one parameter, the source texture (2013 rva 0x50b9e0 Serialize). */
template<UINT NumSamples>
class TBloomBlurPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TBloomBlurPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TBloomBlurPixelShader() {}

	TBloomBlurPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		BlurTextureParameter.Bind(Initializer.ParameterMap,TEXT("BlurTexture"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << BlurTextureParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(const FTexture2DRHIRef& SourceTexture)
	{
		SetTextureParameterDirectly(GetPixelShader(),BlurTextureParameter,TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),SourceTexture);
	}

private:
	FShaderResourceParameter BlurTextureParameter;
};

// DISHONORED(retail): 2013 rva 0xb826d0 .. 0xb82810.
IMPLEMENT_SHADER_TYPE(,FBloomComposePixelShader,TEXT("ArkBloomPartPixelShader"),TEXT("MainCompose"),SF_Pixel,786,23);
IMPLEMENT_SHADER_TYPE(,FBloomComposeVertexShader,TEXT("ArkBloomPartVertexShader"),TEXT("MainCompose"),SF_Vertex,786,23);
IMPLEMENT_SHADER_TYPE(,FBloomDownSampleVertexShader,TEXT("ArkBloomPartVertexShader"),TEXT("MainDownSample"),SF_Vertex,786,23);
IMPLEMENT_SHADER_TYPE(,FBloomDownSamplePixelShader,TEXT("ArkBloomPartPixelShader"),TEXT("MainDownSample"),SF_Pixel,786,23);
typedef TBloomBlurVertexShader<5> TBloomBlurVertexShader5Type;
typedef TBloomBlurPixelShader<5> TBloomBlurPixelShader5Type;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,TBloomBlurVertexShader5Type,TEXT("TBloomBlurVertexShader<5>"),TEXT("ArkBloomPartVertexShader"),TEXT("MainBlur"),SF_Vertex,786,23);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,TBloomBlurPixelShader5Type,TEXT("TBloomBlurPixelShader<5>"),TEXT("ArkBloomPartPixelShader"),TEXT("MainBlur"),SF_Pixel,786,23);

/*-----------------------------------------------------------------------------
	DISHONORED(bringup): the census counters of this pass and its one switch.
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(bringup): -nobloomparts leaves the pass out, which is the switch the before-and-after pair of this package
 * differs by. Read on first use: a file-scope static initialiser in a static library runs before WinMain fills GCmdLine
 * and would always be FALSE (agent CA, commit c403e2f).
 */
static UBOOL DishonoredNoBloomParts()
{
	static INT Cached = -1;
	if (Cached < 0)
	{
		Cached = ParseParam(appCmdLine(),TEXT("nobloomparts")) ? 1 : 0;
	}
	return Cached != 0;
}

/**
 * DISHONORED(bringup): -bloomcompose composes the bloom here even when the fog pass would have done it, so the compose
 * half of this port can be measured on a map that has fog.
 */
static UBOOL DishonoredBloomCompose()
{
	static INT Cached = -1;
	if (Cached < 0)
	{
		Cached = ParseParam(appCmdLine(),TEXT("bloomcompose")) ? 1 : 0;
	}
	return Cached != 0;
}

/**
 * DISHONORED(bringup): -bloomclearblack clears the fog mask to black instead of retail's (128,128,0,0), which quantizes
 * to opaque yellow. It separates what the pass's own primitives contribute from what the cleared background does.
 */
static UBOOL DishonoredBloomClearBlack()
{
	static INT Cached = -1;
	if (Cached < 0)
	{
		Cached = ParseParam(appCmdLine(),TEXT("bloomclearblack")) ? 1 : 0;
	}
	return Cached != 0;
}

/*-----------------------------------------------------------------------------
	DISHONORED(port): the vertex declaration and the geometry every pass of this file draws.
-----------------------------------------------------------------------------*/

namespace bloom
{

/**
 * DISHONORED(layout): 2012 rva 0x54e6d0 InitRHI fills exactly one FVertexElement - stream 0, offset 0, VET_Float4,
 * VEU_Position, usage index 0 - and sets NumElements to 1. xy is the clip-space position and zw the source texel size,
 * which is how the blur shader finds its neighbours without a sample-offset array.
 */
class FArkGaussianVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FArkGaussianVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float4,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** DISHONORED(retail): 2012 rva 0xb9f050 - bloom::GArkGaussianVertexDeclaration. */
TGlobalResource<FArkGaussianVertexDeclaration> GArkGaussianVertexDeclaration;

/** The vertex of that declaration. */
struct FArkBloomVertex
{
	FLOAT X;
	FLOAT Y;
	FLOAT InvSizeX;
	FLOAT InvSizeY;
};

/**
 * DISHONORED(retail): every pass here draws one triangle that covers the whole target, (-1,-1) (3,-1) (-1,3), with
 * 1/SourceSizeX and 1/SourceSizeY in zw of all three vertices (RenderBloomParts 2013 rva 0x5251a0 and GaussianBlur
 * 0x51fe50 build the same three).
 */
static void BuildFullTargetTriangle(FArkBloomVertex OutVertices[3],UINT SourceSizeX,UINT SourceSizeY)
{
	const FLOAT InvSizeX = 1.0f / (FLOAT)SourceSizeX;
	const FLOAT InvSizeY = 1.0f / (FLOAT)SourceSizeY;

	OutVertices[0].X = -1.0f;	OutVertices[0].Y = -1.0f;
	OutVertices[1].X =  3.0f;	OutVertices[1].Y = -1.0f;
	OutVertices[2].X = -1.0f;	OutVertices[2].Y =  3.0f;

	for (INT VertexIndex = 0; VertexIndex < 3; VertexIndex++)
	{
		OutVertices[VertexIndex].InvSizeX = InvSizeX;
		OutVertices[VertexIndex].InvSizeY = InvSizeY;
	}
}

/**
 * DISHONORED(port): 2013 rva 0x51fe50 (2012 0x560b00). Two passes, horizontal then vertical, between two targets of the
 * same size; the only per-pass parameter is the direction.
 *
 * DISHONORED(retail): iKernelSpread and iScale are dead in the retail body - the one call site passes 5 and 1.0f
 * (2013 rva 0x5259a6) and neither is read, because the tap count is the shader type's template argument and the weights
 * are baked into MainBlur. They are kept because the exported symbol has them.
 */
void GaussianBlur(
	const FSurfaceRHIRef& iSurface,
	const FTexture2DRHIRef& iTexture,
	const FSurfaceRHIRef& iSurface2,
	const FTexture2DRHIRef& iTexture2,
	UINT iTextureWidth,
	UINT iTextureHeight,
	URECT<UINT> iRect,
	UINT iKernelSpread,
	FLOAT iScale
	)
{
	SCOPED_DRAW_EVENT(EventArkGaussian)(DEC_SCENE_ITEMS,TEXT("ArkGaussian"));

	TShaderMapRef<TBloomBlurVertexShader<5> > VertexShader(GetGlobalShaderMap());
	TShaderMapRef<TBloomBlurPixelShader<5> > PixelShader(GetGlobalShaderMap());

	FArkBloomVertex Vertices[3];
	BuildFullTargetTriangle(Vertices,iTextureWidth,iTextureHeight);

	for (UINT PassIndex = 0; PassIndex < 2; PassIndex++)
	{
		const FSurfaceRHIRef& DestSurface = (PassIndex == 1) ? iSurface : iSurface2;
		const FTexture2DRHIRef& SourceTexture = (PassIndex == 1) ? iTexture2 : iTexture;

		RHISetRenderTarget(DestSurface,FSurfaceRHIRef());
		RHISetViewport(0,0,0.0f,iTextureWidth,iTextureHeight,1.0f);
		RHISetScissorRect(TRUE,iRect.mLeft,iRect.mTop,iRect.mRight,iRect.mBottom);

		static FGlobalBoundShaderState BoundShaderState;
		SetGlobalBoundShaderState(
			BoundShaderState,
			GArkGaussianVertexDeclaration.VertexDeclarationRHI,
			*VertexShader,
			*PixelShader,
			sizeof(FArkBloomVertex)
			);

		VertexShader->SetParameters(FVector4((PassIndex == 1) ? 0.0f : 1.0f,(PassIndex == 1) ? 1.0f : 0.0f,0.0f,0.0f));
		PixelShader->SetParameters(SourceTexture);

		GDisCensusBloomPartDraws++;
		RHIDrawPrimitiveUP(PT_TriangleList,1,Vertices,sizeof(FArkBloomVertex));

		RHISetScissorRect(FALSE,0,0,0,0);
		RHICopyToResolveTarget(DestSurface,FALSE,FResolveParams(FResolveRect(iRect.mLeft,iRect.mTop,iRect.mRight,iRect.mBottom)));
	}
}

}	// namespace bloom

/*-----------------------------------------------------------------------------
	DISHONORED(port): TBloomPartMeshDrawingPolicy - the material's own bloom-part shaders over its own geometry.
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x50d640 (ctor), 0x51fc10 (DrawShared), 0x51fd70 (CreateBoundShaderState),
 * 0x50d6c0 (SetMeshRenderState). The retail policy is the distortion policy with the two bloom-part shaders in place of
 * the distortion pair: it keeps that template's bInitializeOffsets member (stored, never read), the shader-complexity
 * override and the same rasterizer computation. The mesh interface here is the tree's FMeshBatch one, not retail's
 * FMeshElement one.
 */
template<class MeshPolicyType>
class TBloomPartMeshDrawingPolicy : public FMeshDrawingPolicy
{
public:

	TBloomPartMeshDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		const FMaterial& InMaterialResource,
		UBOOL bInInitializeOffsets,
		UBOOL bInOverrideWithShaderComplexity
		)
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy,InMaterialResource,bInOverrideWithShaderComplexity)
	,	bInitializeOffsets(bInInitializeOffsets)
	{
		// DISHONORED(bringup): retail looks both shaders up with FMaterial::GetShader, which appErrorf's when the
		// material's cooked map has none. A bloom-part material cooked without the two types would take the game down
		// mid-frame, so they are looked up without the assert and the factory reports and skips the element (the same
		// deviation agent CE documented for the post-process material types).
		FVertexFactoryType* VertexFactoryType = InVertexFactory->GetType();
		VertexShader = (TBloomPartMeshVertexShader<MeshPolicyType>*)FindShader(InMaterialResource,&TBloomPartMeshVertexShader<MeshPolicyType>::StaticType,VertexFactoryType);
		BloomPartPixelShader = (TBloomPartMeshPixelShader<MeshPolicyType>*)FindShader(InMaterialResource,&TBloomPartMeshPixelShader<MeshPolicyType>::StaticType,VertexFactoryType);
	}

	/** The shader of that type in the material's cooked map, or NULL. */
	static FShader* FindShader(const FMaterial& MaterialResource,FShaderType* ShaderType,FVertexFactoryType* VertexFactoryType)
	{
		const FMaterialShaderMap* ShaderMap = MaterialResource.GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderMap = ShaderMap ? ShaderMap->GetMeshShaderMap(VertexFactoryType) : NULL;
		return MeshShaderMap ? MeshShaderMap->GetShader(ShaderType) : NULL;
	}

	UBOOL HasShaders() const
	{
		return VertexShader != NULL && BloomPartPixelShader != NULL;
	}

	UBOOL Matches(const TBloomPartMeshDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) &&
			VertexShader == Other.VertexShader &&
			bInitializeOffsets == Other.bInitializeOffsets &&
			BloomPartPixelShader == Other.BloomPartPixelShader;
	}

	/** DISHONORED(port): 2013 rva 0x51fc10 - the shared state, then this pass's own depth state and colour mask. */
	void DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		VertexShader->SetParameters(VertexFactory,MaterialRenderProxy,View);

#if !FINAL_RELEASE
		if (bOverrideWithShaderComplexity)
		{
			TShaderMapRef<FShaderComplexityAccumulatePixelShader> ShaderComplexityPixelShader(GetGlobalShaderMap());
			ShaderComplexityPixelShader->SetParameters(0,BloomPartPixelShader->GetNumInstructions());
		}
		else
#endif
		{
			BloomPartPixelShader->SetParameters(VertexFactory,MaterialRenderProxy,View);
		}

		FMeshDrawingPolicy::DrawShared(View);
		RHISetBoundShaderState(BoundShaderState);

		// DISHONORED(retail): the pass tests depth against the scene depth buffer the fog-mask target is bound with and
		// writes none, so a bloom part behind geometry does not bloom.
		RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
		RHISetColorWriteMask(CW_RGBA);
	}

	/** DISHONORED(port): 2013 rva 0x51fd70. */
	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0)
	{
		FVertexDeclarationRHIRef VertexDeclaration;
		DWORD StreamStrides[MaxVertexElementCount];

		FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration,StreamStrides);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}

		FPixelShaderRHIParamRef PixelShaderRHIRef = BloomPartPixelShader->GetPixelShader();

#if !FINAL_RELEASE
		if (bOverrideWithShaderComplexity)
		{
			TShaderMapRef<FShaderComplexityAccumulatePixelShader> ShaderComplexityAccumulatePixelShader(GetGlobalShaderMap());
			PixelShaderRHIRef = ShaderComplexityAccumulatePixelShader->GetPixelShader();
		}
#endif

		return RHICreateBoundShaderState(VertexDeclaration,StreamStrides,VertexShader->GetVertexShader(),PixelShaderRHIRef,EGST_None);
	}

	/** DISHONORED(port): 2013 rva 0x50d6c0. */
	void SetMeshRenderState(
		const FSceneView& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		EmitMeshDrawEvents(PrimitiveSceneInfo,Mesh);

		VertexShader->SetMesh(PrimitiveSceneInfo,Mesh,BatchElementIndex,View);

#if !FINAL_RELEASE
		if (!bOverrideWithShaderComplexity)
#endif
		{
			BloomPartPixelShader->SetMesh(PrimitiveSceneInfo,Mesh,BatchElementIndex,View,bBackFace);
		}

		const FRasterizerStateInitializerRHI Initializer = {
			(Mesh.bWireframe || IsWireframe()) ? FM_Wireframe : FM_Solid,
			IsTwoSided() ? CM_None : (XOR( XOR(View.bReverseCulling,bBackFace), Mesh.ReverseCulling) ? CM_CCW : CM_CW),
			Mesh.DepthBias,
			Mesh.SlopeScaleDepthBias,
			TRUE
		};
		RHISetRasterizerStateImmediate(Initializer);
	}

private:
	TBloomPartMeshVertexShader<MeshPolicyType>* VertexShader;
	/** DISHONORED(retail): the retail policy stores this and never reads it (the distortion template's member). */
	UBOOL bInitializeOffsets;
	TBloomPartMeshPixelShader<MeshPolicyType>* BloomPartPixelShader;
};

/** DISHONORED(bringup): one line per material whose cooked map is missing the pass's shaders, not one per draw. */
static void ReportMissingBloomPartShaders(const FMaterialRenderProxy* MaterialRenderProxy)
{
	static UBOOL bReported = FALSE;
	if (!bReported)
	{
		bReported = TRUE;
		warnf(TEXT("DISHONORED(bringup): bloom parts: %s has no cooked ArkBloomPart shader for its vertex factory; the pass skips it"),
			MaterialRenderProxy && MaterialRenderProxy->GetMaterial() ? *MaterialRenderProxy->GetMaterial()->GetFriendlyName() : TEXT("(no material)"));
	}
}

/**
 * DISHONORED(port): 2013 rva 0x523b60 (DrawStaticMesh) / 0x523c60 (DrawDynamicMesh). What decides whether an element
 * belongs in the pass is the material's own bloom-part flag, which retail reads through FMaterial vtable slot 12,
 * FMaterialResource::HasBloomPartForCompilation (2013 rva 0x128320 - UMaterial::BloomColor has an expression).
 */
template<class MeshPolicyType>
class TBloomPartMeshDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	typedef UBOOL ContextType;

	static UBOOL DrawDynamicMesh(
		const FSceneView& View,
		ContextType bInitializeOffsets,
		const FMeshBatch& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		)
	{
		if (!Mesh.MaterialRenderProxy || bBackFace)
		{
			return FALSE;
		}
		const FMaterial* Material = Mesh.MaterialRenderProxy->GetMaterial();
		if (!Material->HasBloomPartForCompilation())
		{
			return FALSE;
		}

		TBloomPartMeshDrawingPolicy<MeshPolicyType> DrawingPolicy(
			Mesh.VertexFactory,
			Mesh.MaterialRenderProxy,
			*Material,
			bInitializeOffsets,
			(View.Family->ShowFlags & SHOW_ShaderComplexity) != 0
			);
		if (!DrawingPolicy.HasShaders())
		{
			ReportMissingBloomPartShaders(Mesh.MaterialRenderProxy);
			return FALSE;
		}

		DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		for (INT BatchElementIndex = 0; BatchElementIndex < Mesh.Elements.Num(); BatchElementIndex++)
		{
			DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,BatchElementIndex,bBackFace,typename TBloomPartMeshDrawingPolicy<MeshPolicyType>::ElementDataType());
			GDisCensusBloomPartDraws++;
			DrawingPolicy.DrawMesh(Mesh,BatchElementIndex);
		}
		return TRUE;
	}

	static UBOOL DrawStaticMesh(
		const FSceneView* View,
		ContextType bInitializeOffsets,
		const FStaticMesh& StaticMesh,
		UBOOL bBackFace,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		)
	{
		if (!StaticMesh.MaterialRenderProxy)
		{
			return FALSE;
		}
		const FMaterial* Material = StaticMesh.MaterialRenderProxy->GetMaterial();
		if (!Material->HasBloomPartForCompilation())
		{
			return FALSE;
		}

		TBloomPartMeshDrawingPolicy<MeshPolicyType> DrawingPolicy(
			StaticMesh.VertexFactory,
			StaticMesh.MaterialRenderProxy,
			*Material,
			bInitializeOffsets,
			(View->Family->ShowFlags & SHOW_ShaderComplexity) != 0
			);
		if (!DrawingPolicy.HasShaders())
		{
			ReportMissingBloomPartShaders(StaticMesh.MaterialRenderProxy);
			return FALSE;
		}

		DrawingPolicy.DrawShared(View,DrawingPolicy.CreateBoundShaderState());
		for (INT BatchElementIndex = 0; BatchElementIndex < StaticMesh.Elements.Num(); BatchElementIndex++)
		{
			DrawingPolicy.SetMeshRenderState(*View,PrimitiveSceneInfo,StaticMesh,BatchElementIndex,bBackFace,typename TBloomPartMeshDrawingPolicy<MeshPolicyType>::ElementDataType());
			GDisCensusBloomPartDraws++;
			DrawingPolicy.DrawMesh(StaticMesh,BatchElementIndex);
		}
		return TRUE;
	}

	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
	{
		return MaterialRenderProxy && !MaterialRenderProxy->GetMaterial()->HasBloomPartForCompilation();
	}
};

/*-----------------------------------------------------------------------------
	FArkBloomPartPrimSet
-----------------------------------------------------------------------------*/

void FArkBloomPartPrimSet::AddScenePrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo,const FViewInfo& ViewInfo)
{
	Prims.AddItem(PrimitiveSceneInfo);
}

/**
 * DISHONORED(port): 2013 rva 0x524f00. The static half has no material filter of its own - unlike the distortion set,
 * which also requires IsTranslucent - because the factory's bloom-part check is the filter.
 */
UBOOL FArkBloomPartPrimSet::DrawBloomPrims(const FViewInfo* ViewInfo,UINT DPGIndex,UBOOL bInitializeOffsets)
{
	UBOOL bDirty = DrawViewElements<TBloomPartMeshDrawingPolicyFactory<FBloomPartMeshPolicy> >(
		*ViewInfo,
		bInitializeOffsets,
		DPGIndex,
		TRUE	// bloom parts are drawn pre fog
		);

	if (Prims.Num())
	{
		TDynamicPrimitiveDrawer<TBloomPartMeshDrawingPolicyFactory<FBloomPartMeshPolicy> > Drawer(
			ViewInfo,
			DPGIndex,
			bInitializeOffsets,
			TRUE
			);

		for (INT PrimIdx = 0; PrimIdx < Prims.Num(); PrimIdx++)
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = Prims(PrimIdx);
			const FPrimitiveViewRelevance& ViewRelevance = ViewInfo->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			if (ViewRelevance.bBloomPartRelevance)
			{
				if (ViewRelevance.bDynamicRelevance)
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,ViewInfo,DPGIndex);
				}
				if (ViewRelevance.bStaticRelevance)
				{
					for (INT StaticMeshIdx = 0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++)
					{
						FStaticMesh& StaticMesh = PrimitiveSceneInfo->StaticMeshes(StaticMeshIdx);
						if (ViewInfo->StaticMeshVisibilityMap(StaticMesh.Id))
						{
							bDirty |= TBloomPartMeshDrawingPolicyFactory<FBloomPartMeshPolicy>::DrawStaticMesh(
								ViewInfo,
								bInitializeOffsets,
								StaticMesh,
								FALSE,
								PrimitiveSceneInfo,
								StaticMesh.HitProxyId
								);
						}
					}
				}
			}
		}

		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}

/*-----------------------------------------------------------------------------
	FSceneRenderer::RenderBloomParts
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5251a0 (2012 0x566120). Arkane's bloom is not the reference engine's: it is a pass of its
 * own over the primitives whose material has a bloom part, and it runs at SDPG_World between the soft-masked base pass
 * and the fog pass (RenderDPGEnd, 2013 rva 0x464290).
 *
 *   1. resolve scene colour, bind the full-size fog-mask target with the scene depth buffer, clear it and draw the bloom
 *      parts of BloomPartPrimSet[SDPG_World] into it with each material's own bloom-part shaders
 *   2. downsample that target into the quarter-size bloom target, scaling by the view's bloom tint and subtracting its
 *      threshold, then blur it five taps at a time between the two quarter-size targets
 *   3. compose the result back over scene colour - unless the fog pass will do it, which is what m_BloomNeedBlit says
 *
 * @return TRUE if scene colour was written
 */
UBOOL FSceneRenderer::RenderBloomParts(UINT DPGIndex)
{
	m_BloomNeedBlit = FALSE;

	// DISHONORED(retail): the pass exists at SDPG_World only.
	if (DPGIndex != SDPG_World)
	{
		return FALSE;
	}

	GDisCensusBloomPartPrims = 0;
	GDisCensusBloomPartDraws = 0;
	if (Views.Num())
	{
		for (UINT SetIndex = 0; SetIndex < SDPG_MAX_SceneRender; SetIndex++)
		{
			GDisCensusBloomPartSetPrims[SetIndex] = Views(0).BloomPartPrimSet[SetIndex].NumPrims();
		}
		GDisCensusBloomPartPrims = Views(0).BloomPartPrimSet[DPGIndex].NumPrims();
	}

	UBOOL bBloomRelevant = FALSE;
	for (INT ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		const FViewInfo& CheckView = Views(ViewIndex);
		if (CheckView.m_ArkPpConfig
			&& CheckView.m_ArkPpConfig->m_PpBloomParameters.m_bEnable
			&& (CheckView.BloomPartPrimSet[DPGIndex].NumPrims() > 0
				|| (CheckView.bHasBloomPartViewMeshElements & (1 << DPGIndex))))
		{
			bBloomRelevant = TRUE;
			break;
		}
	}
	if (!bBloomRelevant || DishonoredNoBloomParts())
	{
		return FALSE;
	}

	// DISHONORED(retail): set before anything is drawn, and cleared again only when this pass composes the result itself.
	m_BloomNeedBlit = TRUE;

	SCOPED_DRAW_EVENT(EventBloomParts)(DEC_SCENE_ITEMS,TEXT("BloomParts"));

	// DISHONORED(retail): retail takes Views(0) for the viewport and the scissor rects from here on, whichever view the
	// loop above stopped at.
	FViewInfo& View = Views(0);

	UBOOL bDirty = FALSE;
	{
		SCOPED_DRAW_EVENT(EventDrawBloomPrims)(DEC_SCENE_ITEMS,TEXT("Draw blooming primitives"));

		GSceneRenderTargets.ResolveSceneColor(FResolveRect());
		GSceneRenderTargets.BeginRenderingFogMask();

		RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
		RHISetViewParameters(View);

		// DISHONORED(retail): the clear colour is (128,128,0,0) (2013 rva 0x5253d7, __real@43000000 twice), far outside
		// the 0..1 an A8R8G8B8 target holds, so it clamps to opaque yellow-white. It is retail's value; what keeps a
		// cleared pixel from blooming is the threshold the downsample subtracts, not the clear.
		RHIClear(TRUE,DishonoredBloomClearBlack() ? FLinearColor(0.0f,0.0f,0.0f,0.0f) : FLinearColor(128.0f,128.0f,0.0f,0.0f),FALSE,0.0f,TRUE,0);
		RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());

		bDirty = View.BloomPartPrimSet[DPGIndex].DrawBloomPrims(&View,DPGIndex,FALSE);

		GSceneRenderTargets.FinishRenderingFogMask();
	}

	if (bDirty)
	{
		SCOPED_DRAW_EVENT(EventBloomReduction)(DEC_SCENE_ITEMS,TEXT("Bloom reduction and blur"));

		const UINT QuarterSizeX = GSceneRenderTargets.GetBufferSizeX() >> 2;
		const UINT QuarterSizeY = GSceneRenderTargets.GetBufferSizeY() >> 2;

		bloom::FArkBloomVertex Vertices[3];
		bloom::BuildFullTargetTriangle(Vertices,QuarterSizeX,QuarterSizeY);

		TShaderMapRef<FBloomDownSampleVertexShader> DownSampleVertexShader(GetGlobalShaderMap());
		TShaderMapRef<FBloomDownSamplePixelShader> DownSamplePixelShader(GetGlobalShaderMap());

		GSceneRenderTargets.BeginRenderingBloom();
		RHISetViewport(0,0,0.0f,QuarterSizeX,QuarterSizeY,1.0f);
		RHISetScissorRect(TRUE,View.RenderTargetX >> 2,View.RenderTargetY >> 2,(View.RenderTargetX + View.RenderTargetSizeX) >> 2,(View.RenderTargetY + View.RenderTargetSizeY) >> 2);

		{
			// DISHONORED(retail): retail builds an FTexture around the fog-mask texture with a bilinear clamp sampler and
			// hands that to SetParameters (2013 rva 0x50de60); agent BD's shader takes the texture and binds the same
			// sampler itself.
			RHIReduceTextureCachePenalty(DownSamplePixelShader->GetPixelShader());
			DownSamplePixelShader->SetParameters(&View,GSceneRenderTargets.GetFogMaskTexture());

			static FGlobalBoundShaderState DownSampleBoundShaderState;
			SetGlobalBoundShaderState(
				DownSampleBoundShaderState,
				bloom::GArkGaussianVertexDeclaration.VertexDeclarationRHI,
				*DownSampleVertexShader,
				*DownSamplePixelShader,
				sizeof(bloom::FArkBloomVertex)
				);

			RHISetBlendState(TStaticBlendState<>::GetRHI());
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
			RHISetColorWriteEnable(TRUE);
			RHISetStencilState(TStaticStencilState<>::GetRHI());
			RHISetColorWriteMask(CW_RGBA);

			GDisCensusBloomPartDraws++;
			RHIDrawPrimitiveUP(PT_TriangleList,1,Vertices,sizeof(bloom::FArkBloomVertex));
		}

		GSceneRenderTargets.FinishRenderingBloom();

		bloom::URECT<UINT> BlurRect;
		BlurRect.mLeft = View.RenderTargetX >> 2;
		BlurRect.mTop = View.RenderTargetY >> 2;
		BlurRect.mRight = (View.RenderTargetX + View.RenderTargetSizeX) >> 2;
		BlurRect.mBottom = (View.RenderTargetY + View.RenderTargetSizeY) >> 2;
		bloom::GaussianBlur(
			GSceneRenderTargets.GetBloomPartsSurface(),
			GSceneRenderTargets.GetBloomPartsTexture(),
			GSceneRenderTargets.GetBloomPartsSurface2(),
			GSceneRenderTargets.GetBloomPartsTexture2(),
			QuarterSizeX,
			QuarterSizeY,
			BlurRect,
			5,
			1.0f
			);
	}

	// DISHONORED(retail): with fog or rain in the scene the fog pass composes the bloom target itself (FogRendering.cpp
	// reads m_BloomNeedBlit and binds GetBloomPartsTexture), so this pass leaves the bit set and does not compose.
	// DISHONORED(bringup): retail's condition is `DisFogs.Num() || m_Rains.Num()`; FScene has no m_Rains (@10160, the
	// rain scene info) in this tree, so only the fog half is tested. A level with rain and no fog would compose here and
	// once more in the rain pass when that pass exists.
	if (!bDirty
		|| (!DishonoredBloomCompose() && (ViewFamily.ShowFlags & SHOW_Fog) && Scene->DisFogs.Num()))
	{
		return FALSE;
	}
	m_BloomNeedBlit = FALSE;

	{
		SCOPED_DRAW_EVENT(EventBloomCompose)(DEC_SCENE_ITEMS,TEXT("Bloom compose"));

		bloom::FArkBloomVertex Vertices[3];
		bloom::BuildFullTargetTriangle(Vertices,GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY());

		TShaderMapRef<FBloomComposeVertexShader> ComposeVertexShader(GetGlobalShaderMap());
		TShaderMapRef<FBloomComposePixelShader> ComposePixelShader(GetGlobalShaderMap());

		GSceneRenderTargets.BeginRenderingSceneColor();
		RHISetViewport(0,0,0.0f,GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY(),1.0f);
		RHISetScissorRect(TRUE,View.RenderTargetX,View.RenderTargetY,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY);

		ComposePixelShader->SetParameters(&View,GSceneRenderTargets.GetBloomPartsTexture());

		static FGlobalBoundShaderState ComposeBoundShaderState;
		SetGlobalBoundShaderState(
			ComposeBoundShaderState,
			bloom::GArkGaussianVertexDeclaration.VertexDeclarationRHI,
			*ComposeVertexShader,
			*ComposePixelShader,
			sizeof(bloom::FArkBloomVertex)
			);

		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetColorWriteEnable(TRUE);
		RHISetStencilState(TStaticStencilState<>::GetRHI());
		// DISHONORED(retail): the compose keeps the alpha channel of scene colour (CW_RGB); the downsample does not.
		RHISetColorWriteMask(CW_RGB);

		GDisCensusBloomPartDraws++;
		RHIDrawPrimitiveUP(PT_TriangleList,1,Vertices,sizeof(bloom::FArkBloomVertex));

		RHISetScissorRect(FALSE,0,0,0,0);
		GSceneRenderTargets.FinishRenderingSceneColor(TRUE);
	}

	return TRUE;
}
