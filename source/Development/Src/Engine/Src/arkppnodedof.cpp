// Engine/src/arkppnodedof.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (36):
//   0x549e20  public: virtual unsigned int __thiscall UArkPpNodeDof::LinkInput(unsigned int, class UArkPpNode *)
//   0x549e40  public: virtual unsigned int __thiscall UArkPpNodeDof::UnlinkInput(class UArkPpNode *)
//   0x549e70  public: virtual unsigned int __thiscall UArkPpNodeDof::UnlinkInput(unsigned int)
//   0x54b4d0  public: __thiscall TArkPpDofUberPS<0, 1>::TArkPpDofUberPS<0, 1>(void)
//   0x54b520  public: virtual unsigned int __thiscall TArkPpDofUberPS<1, 0>::Serialize(class FArchive &)
//   0x54ba50  public: virtual unsigned int __thiscall FArkPpDofLutBlenderPS::Serialize(class FArchive &)
//   0x54d9b0  public: virtual __thiscall `anonymous namespace'::FArkDofRamp<16>::~FArkDofRamp<16>(void)
//   0x54da40  public: static class FShader * __cdecl TArkPpDofUberPS<1, 0>::ConstructSerializedInstance(void)
//   0x54ed50  public: virtual class FString __thiscall UArkPpNodeDof::InputName(unsigned int)const
//   0x54ee70  public: static class FShader * __cdecl FArkPpDofLutBlenderPS::ConstructSerializedInstance(void)
//   0x54ef00  public: __thiscall FArkPpNodeDofProxy::FArkPpNodeDofProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeDof *)
//   0x54f140  public: virtual __thiscall FArkPpNodeDofProxy::~FArkPpNodeDofProxy(void)
//   0x54f1f0  public: void __thiscall FArkPpDofDownsampleVS::SetParameters(struct FArkPpDofDownsampleParameters const &)
//   0x54f290  public: void __thiscall FArkPpDofUberVS::SetParameters(struct FArkPpDofUberParameters const &)
//   0x54f440  public: void __thiscall FArkPpDofLutBlenderPS::SetParameters(struct FArkUberPpParameters const &, struct FLinearColor const &)
//   0x555220  public: virtual void __thiscall `anonymous namespace'::FArkDofRamp<16>::InitDynamicRHI(void)
//   0x555350  public: virtual void __thiscall `anonymous namespace'::FArkDofRamp<16>::ReleaseDynamicRHI(void)
//   0x5553a0  public: void __thiscall TArkPpDofUberPS<0, 1>::SetParameters(struct FArkPpDofUberParameters const &)
//   0x55d740  public: static class UClass * __cdecl UArkPpNodeDof::GetPrivateStaticClassUArkPpNodeDof(wchar_t const *)
//   0x55f3b0  public: static void __cdecl UArkPpNodeDof::InitializePrivateStaticClassUArkPpNodeDof(void)
//   0x563020  public: static class UClass * __cdecl UArkPpNodeDof::StaticClassNoInline(void)
//   0x563050  public: unsigned int __thiscall FArkPpNodeDofProxy::Downsample(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x563560  public: unsigned int __thiscall FArkPpNodeDofProxy::LutCreation(struct FLinearColor const &)
//   0x5637a0  public: unsigned int __thiscall FArkPpNodeDofProxy::Blend(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x563f40  public: virtual unsigned int __thiscall FArkPpNodeDofProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5654b0  public: virtual unsigned int __thiscall UArkPpNodeDof::IsValid(struct FArkPpIsValidData &)
//   0x565540  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeDof::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f5b0  _anonymous_namespace_::_dynamic_initializer_for__GDofRamp__
//   0xb9f5d0  _dynamic_initializer_for__FArkPpDofDownsamplePS::StaticType__
//   0xb9f610  _dynamic_initializer_for__FArkPpDofDownsampleVS::StaticType__
//   0xb9f650  _dynamic_initializer_for__TArkPpDofUberPS_1_1_::StaticType__
//   0xb9f690  _dynamic_initializer_for__TArkPpDofUberPS_1_0_::StaticType__
//   0xb9f6d0  _dynamic_initializer_for__TArkPpDofUberPS_0_1_::StaticType__
//   0xb9f710  _dynamic_initializer_for__FArkPpDofUberVS::StaticType__
//   0xb9f750  _dynamic_initializer_for__FArkPpDofLutBlenderPS::StaticType__
//   0xb9f790  _dynamic_initializer_for__FArkPpDofLutBlenderVS::StaticType__

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/**
 * DISHONORED(port): the shader types of Arkane's depth-of-field node (UArkPpNodeDof / FArkPpNodeDofProxy), which is
 * also where the colour treatment happens: the uber pixel shader applies focus, the linear-to-gamma ramp and the film
 * grain, and the LUT blender builds the colour-balance lookup out of FArkUberPpParameters. 2013 rva 0xb82e50 ff.,
 * source ArkPpDof, entry points Downsample_VS/PS, Uber_VS/PS, LUTBlender_VS/PS.
 */

/** DISHONORED(layout): one parameter, (1/w, 1/h, 42, 36) (2013 rva 0x50eab0 SetParameters). */
class FArkPpDofDownsampleVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FArkPpDofDownsampleVS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FArkPpDofDownsampleVS() {}

	FArkPpDofDownsampleVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mResolutionParams.Bind(Initializer.ParameterMap,TEXT("ResolutionParams"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mResolutionParams;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter mResolutionParams;
};

/** DISHONORED(layout): one parameter, the source colour. */
class FArkPpDofDownsamplePS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FArkPpDofDownsamplePS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FArkPpDofDownsamplePS() {}

	FArkPpDofDownsamplePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mSrcColorParam.Bind(Initializer.ParameterMap,TEXT("SrcColor"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSrcColorParam;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter mSrcColorParam;
};

/** DISHONORED(layout): three parameters (2013 rva 0x50eb50 SetParameters), gate 787 / 1 (0xb82f90). */
class FArkPpDofUberVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FArkPpDofUberVS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FArkPpDofUberVS() {}

	FArkPpDofUberVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mResolutionParams.Bind(Initializer.ParameterMap,TEXT("ResolutionParams"),TRUE);
		mViewportScaleBias.Bind(Initializer.ParameterMap,TEXT("ViewportScaleBias"),TRUE);
		mNoiseParams.Bind(Initializer.ParameterMap,TEXT("NoiseParams"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mResolutionParams;
		Ar << mViewportScaleBias;
		Ar << mNoiseParams;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter mResolutionParams;
	FShaderParameter mViewportScaleBias;
	FShaderParameter mNoiseParams;
};

/**
 * DISHONORED(layout): seven parameters (2013 rva 0x50b520 Serialize): source colour, source depth, the low-resolution
 * blurred colour, the focus constants, the linear-to-gamma ramp texture, the scene depth reconstruction constant and
 * the film grain constants. The two template arguments are the cooked name's suffix (_01, _10, _11): whether the node
 * blurs the near field and whether it blurs the far field. Gate 787 / 1 (0xb82ed0 ff.).
 */
template<UINT bNearBlur,UINT bFarBlur>
class TArkPpDofUberPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TArkPpDofUberPS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TArkPpDofUberPS() {}

	TArkPpDofUberPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mSrcColorParam.Bind(Initializer.ParameterMap,TEXT("SrcColor"),TRUE);
		mSrcDepthParam.Bind(Initializer.ParameterMap,TEXT("SrcDepth"),TRUE);
		mLowColorParam.Bind(Initializer.ParameterMap,TEXT("LowColor"),TRUE);
		mFocusParams.Bind(Initializer.ParameterMap,TEXT("FocusParams"),TRUE);
		mLinearToGammaParam.Bind(Initializer.ParameterMap,TEXT("LinearToGamma"),TRUE);
		mSceneDepthCalcParameter.Bind(Initializer.ParameterMap,TEXT("MinZ_MaxZRatio"),TRUE);
		mFilmGrainParams.Bind(Initializer.ParameterMap,TEXT("FilmGrainParams"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSrcColorParam;
		Ar << mSrcDepthParam;
		Ar << mLowColorParam;
		Ar << mFocusParams;
		Ar << mLinearToGammaParam;
		Ar << mSceneDepthCalcParameter;
		Ar << mFilmGrainParams;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter mSrcColorParam;
	FShaderResourceParameter mSrcDepthParam;
	FShaderResourceParameter mLowColorParam;
	FShaderParameter mFocusParams;
	FShaderResourceParameter mLinearToGammaParam;
	FShaderParameter mSceneDepthCalcParameter;
	FShaderParameter mFilmGrainParams;
};

/** DISHONORED(layout): no parameters at all (the cooked record has 0 parameter words). */
class FArkPpDofLutBlenderVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FArkPpDofLutBlenderVS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FArkPpDofLutBlenderVS() {}

	FArkPpDofLutBlenderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}
};

/**
 * DISHONORED(layout): six parameters (2013 rva 0x50b1c0 Serialize, 0x50ed00 SetParameters): the source lookup
 * texture, the three colour-balance constants (shadow, mid and high tones, from FArkPpColorBalanceParameters), the
 * overlay opacity and the brightness/contrast pair. Gate 789 / 1 (0xb82fd0), the highest of the FArkPp family.
 */
class FArkPpDofLutBlenderPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FArkPpDofLutBlenderPS,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	FArkPpDofLutBlenderPS() {}

	FArkPpDofLutBlenderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mLUTParams.Bind(Initializer.ParameterMap,TEXT("LUTParams"),TRUE);
		mCBParams[0].Bind(Initializer.ParameterMap,TEXT("CBShadowTones"),TRUE);
		mCBParams[1].Bind(Initializer.ParameterMap,TEXT("CBMidTones"),TRUE);
		mCBParams[2].Bind(Initializer.ParameterMap,TEXT("CBHighTones"),TRUE);
		mOverlay.Bind(Initializer.ParameterMap,TEXT("Overlay"),TRUE);
		mBrightnessContrast.Bind(Initializer.ParameterMap,TEXT("BrightnessContrast"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mLUTParams;
		Ar << mCBParams[0];
		Ar << mCBParams[1];
		Ar << mCBParams[2];
		Ar << mOverlay;
		Ar << mBrightnessContrast;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter mLUTParams;
	FShaderParameter mCBParams[3];
	FShaderParameter mOverlay;
	FShaderParameter mBrightnessContrast;
};

// DISHONORED(retail): 2013 rva 0xb82e50 .. 0xb83010.
IMPLEMENT_SHADER_TYPE(,FArkPpDofDownsamplePS,TEXT("ArkPpDof"),TEXT("Downsample_PS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(,FArkPpDofDownsampleVS,TEXT("ArkPpDof"),TEXT("Downsample_VS"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(,FArkPpDofUberVS,TEXT("ArkPpDof"),TEXT("Uber_VS"),SF_Vertex,787,1);
IMPLEMENT_SHADER_TYPE(,FArkPpDofLutBlenderPS,TEXT("ArkPpDof"),TEXT("LUTBlender_PS"),SF_Pixel,789,1);
IMPLEMENT_SHADER_TYPE(,FArkPpDofLutBlenderVS,TEXT("ArkPpDof"),TEXT("LUTBlender_VS"),SF_Vertex,786,1);

typedef TArkPpDofUberPS<1,1> FArkPpDofUber_11PSType;
typedef TArkPpDofUberPS<1,0> FArkPpDofUber_10PSType;
typedef TArkPpDofUberPS<0,1> FArkPpDofUber_01PSType;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpDofUber_11PSType,TEXT("FArkPpDofUber_11PS"),TEXT("ArkPpDof"),TEXT("Uber_PS"),SF_Pixel,787,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpDofUber_10PSType,TEXT("FArkPpDofUber_10PS"),TEXT("ArkPpDof"),TEXT("Uber_PS"),SF_Pixel,787,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpDofUber_01PSType,TEXT("FArkPpDofUber_01PS"),TEXT("ArkPpDof"),TEXT("Uber_PS"),SF_Pixel,787,1);

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpDofShaderTypes()
{
}
