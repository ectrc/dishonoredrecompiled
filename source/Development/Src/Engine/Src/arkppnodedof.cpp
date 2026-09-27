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
#include "arkcommonvertexdeclaration.h"
#include "arkpp.h"

/**
 * DISHONORED(port): the shader types of Arkane's depth-of-field node (UArkPpNodeDof / FArkPpNodeDofProxy), which is
 * also where the colour treatment happens: the uber pixel shader applies focus, the linear-to-gamma ramp and the film
 * grain, and the LUT blender builds the colour-balance lookup out of FArkUberPpParameters. 2013 rva 0xb82e50 ff.,
 * source ArkPpDof, entry points Downsample_VS/PS, Uber_VS/PS, LUTBlender_VS/PS.
 */

/*-----------------------------------------------------------------------------
	The colour lookup table (2012 PDB `anonymous namespace'::FArkDofRamp<16>, 28 bytes;
	InitDynamicRHI 2013 rva 0x514bc0, ReleaseDynamicRHI 0x514cf0)
-----------------------------------------------------------------------------*/

namespace
{
	/**
	 * DISHONORED(port): 2013 rva 0x514bc0. The "3D" colour lookup table the uber pass samples is a LutSize cube
	 * unwrapped along X into a plain (LutSize*LutSize) x LutSize A8R8G8B8 2D render target, so nothing here needs a
	 * volume texture. TexCreate flags 0x12 are TexCreate_NoTiling | TexCreate_ResolveTargetable; the surface is
	 * created with no flags at all, which is how a resolve of it lands in mRamp3D.
	 */
	template<UINT LutSize>
	class FArkDofRamp : public FRenderResource
	{
	public:
		virtual void InitDynamicRHI()
		{
			mRamp3D = RHICreateTexture2D(LutSize * LutSize,LutSize,PF_A8R8G8B8,1,TexCreate_NoTiling|TexCreate_ResolveTargetable,NULL);
			mRamp3DSurface = RHICreateTargetableSurface(LutSize * LutSize,LutSize,PF_A8R8G8B8,mRamp3D,0,TEXT("ArkDofRamp"));
		}

		/** DISHONORED(port): 2013 rva 0x514cf0. */
		virtual void ReleaseDynamicRHI()
		{
			mRamp3D.SafeRelease();
			mRamp3DSurface.SafeRelease();
		}

		static UINT GetSizeX() { return LutSize * LutSize; }
		static UINT GetSizeY() { return LutSize; }

		FTexture2DRHIRef mRamp3D;
		FSurfaceRHIRef mRamp3DSurface;
	};

	/** DISHONORED(port): 2012 dynamic initialiser 0xb9f5b0 - one ramp for the whole renderer, not one per node. */
	TGlobalResource<FArkDofRamp<16> > GDofRamp;
}

/** DISHONORED(layout): 2012 PDB FArkPpDofDownsampleParameters (16 bytes). */
struct FArkPpDofDownsampleParameters
{
	FSamplerStateRHIParamRef mSamplerState;
	FTextureRHIParamRef mTexture;
	UINT mSizeX;
	UINT mSizeY;
};

/**
 * DISHONORED(layout): 2012 PDB FArkPpDofUberParameters (320 bytes: mSceneColor, mSceneDepth, mLowResColor and
 * mLinearToGammaRamp as FTexture @0/64/128/192, mSizeX/mSizeY @256, the three focus floats @264, the film grain @276,
 * the view rectangle @288 and the view @304). Only the four textures and the rectangle are cleared by retail's
 * constructor (2013 rva 0x50efb0); everything else is written by Blend.
 */
struct FArkPpDofUberParameters
{
	FArkPpDofUberParameters()
		: mSizeX(0)
		, mSizeY(0)
		, m_FocusDistance(0.0f)
		, m_InFocusRadius(1.0f)
		, m_FarBlurAmount(0.0f)
		, m_FilmGrain(0.0f)
		, m_Viewport(0.0f,0.0f,0.0f,0.0f)
		, m_View(NULL)
	{}

	FTexture mSceneColor;
	FTexture mSceneDepth;
	FTexture mLowResColor;
	FTexture mLinearToGammaRamp;
	UINT mSizeX;
	UINT mSizeY;
	FLOAT m_FocusDistance;
	FLOAT m_InFocusRadius;
	FLOAT m_FarBlurAmount;
	FLOAT m_FilmGrain;
	/** retail's FBox2D is Arkane's own (one FVector4, MinX MinY MaxX MaxY), not the reference engine's 20-byte one */
	FVector4 m_Viewport;
	const FSceneView* m_View;
};

/** DISHONORED(layout): one parameter, (1/w, 1/h, 36, 42) (2013 rva 0x50eab0 SetParameters). */
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

	/** DISHONORED(port): 2013 rva 0x50eab0 - the source's texel size; the last two words are retail's own constants. */
	void SetParameters(const FArkPpDofDownsampleParameters& iParams)
	{
		const FVector4 Resolutions(1.0f / (FLOAT)iParams.mSizeX,1.0f / (FLOAT)iParams.mSizeY,36.0f,42.0f);
		SetVertexShaderValues<FVector4>(GetVertexShader(),mResolutionParams,&Resolutions,1);
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

	/** DISHONORED(port): retail binds this one inline in Downsample (2013 rva 0x522210), there is no SetParameters. */
	void SetSourceTexture(const FTexture& Source)
	{
		SetTextureParameterDirectly(GetPixelShader(),mSrcColorParam,&Source);
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

	/**
	 * DISHONORED(port): 2013 rva 0x50eb50. The film-grain noise offset is a pair of indices into a 128x128 grid, and
	 * retail draws each of them again until it differs from the previous frame's, so the grain never stands still.
	 * The two function statics are retail's own (`SetParameters'::`3'::idxu / idxv).
	 */
	void SetParameters(const FArkPpDofUberParameters& iParams)
	{
		const FLOAT SizeX = (FLOAT)iParams.mSizeX;
		const FLOAT SizeY = (FLOAT)iParams.mSizeY;
		const FVector4 Resolutions(1.0f / SizeX,1.0f / SizeY,36.0f,42.0f);
		SetVertexShaderValues<FVector4>(GetVertexShader(),mResolutionParams,&Resolutions,1);

		const FVector4 ViewportScaleBias(
			(iParams.m_Viewport.Z - iParams.m_Viewport.X) / SizeX,
			(iParams.m_Viewport.W - iParams.m_Viewport.Y) / SizeY,
			iParams.m_Viewport.X / SizeX,
			iParams.m_Viewport.Y / SizeY);
		SetVertexShaderValues<FVector4>(GetVertexShader(),mViewportScaleBias,&ViewportScaleBias,1);

		static INT idxu = 0;
		static INT idxv = 0;
		INT NextU;
		do
		{
			NextU = appRand() & 0x7F;
		}
		while (NextU == idxu);
		idxu = NextU;
		INT NextV;
		do
		{
			NextV = appRand() & 0x7F;
		}
		while (NextV == idxv);
		idxv = NextV;

		const FVector4 NoiseParams(SizeX,SizeY,(FLOAT)idxu,(FLOAT)idxv);
		SetVertexShaderValues<FVector4>(GetVertexShader(),mNoiseParams,&NoiseParams,1);
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
 * DISHONORED(retail): the declaration order here is the serialisation order, which is what the cooked record needs;
 * retail's own member order (2012 PDB) puts mFocusParams after mSceneDepthCalcParameter and serialises it fourth all
 * the same, so the two orders differ in retail too.
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

	/**
	 * DISHONORED(port): 2013 rva 0x514d40 (the three instantiations are COMDAT-folded onto one body). The focus word
	 * carries 1/m_InFocusRadius, not the radius, and the grain pair is the amplitude scaled to +2/255 and -1/255 so
	 * the shader can add a signed dither. The depth reconstruction constant comes from the RHI, as everywhere else.
	 */
	void SetParameters(const FArkPpDofUberParameters& iParams)
	{
		const FPixelShaderRHIParamRef PixelShader = GetPixelShader();
		SetTextureParameterDirectly(PixelShader,mSrcColorParam,&iParams.mSceneColor);
		SetTextureParameterDirectly(PixelShader,mSrcDepthParam,&iParams.mSceneDepth);
		SetTextureParameterDirectly(PixelShader,mLowColorParam,&iParams.mLowResColor);
		SetTextureParameterDirectly(PixelShader,mLinearToGammaParam,&iParams.mLinearToGammaRamp);

		const FVector4 FocusParams(iParams.m_FocusDistance,1.0f / iParams.m_InFocusRadius,iParams.m_FarBlurAmount,1.0f);
		SetPixelShaderValues<FVector4>(PixelShader,mFocusParams,&FocusParams,1);

		RHISetViewPixelParameters(iParams.m_View,PixelShader,&mSceneDepthCalcParameter,NULL,NULL);

		const FVector4 FilmGrainParams(iParams.m_FilmGrain * (2.0f / 255.0f),iParams.m_FilmGrain * (-1.0f / 255.0f),0.0f,0.0f);
		SetPixelShaderValues<FVector4>(PixelShader,mFilmGrainParams,&FilmGrainParams,1);
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

	/**
	 * DISHONORED(port): 2013 rva 0x50ed00 - this is the colour treatment of the game. The three tone words each carry
	 * one FVector of cyan-red / magenta-green / yellow-blue offsets, and the shadow one carries the whole grade's
	 * opacity in its W; LUTParams is (2^exposure, gamma adjustment over the display gamma, pre-desaturation,
	 * post-desaturation); the overlay is the view's own colour; and the brightness/contrast pair turns the contrast
	 * dial into tan((c+1)*pi/4), which is 1 at the neutral setting.
	 */
	void SetParameters(const FArkUberPpParameters& iParams,const FLinearColor& iOverlay)
	{
		const FPixelShaderRHIParamRef PixelShader = GetPixelShader();
		const FArkPpColorBalanceParameters& CB = iParams.m_CBParameters;

		FVector4 CBs[3];
		CBs[0] = FVector4(CB.m_CrMgYbShadTones.X,CB.m_CrMgYbShadTones.Y,CB.m_CrMgYbShadTones.Z,CB.m_Opacity);
		CBs[1] = FVector4(CB.m_CrMgYbMidTones.X,CB.m_CrMgYbMidTones.Y,CB.m_CrMgYbMidTones.Z,0.0f);
		CBs[2] = FVector4(CB.m_CrMgYbHighTones.X,CB.m_CrMgYbHighTones.Y,CB.m_CrMgYbHighTones.Z,0.0f);
		for (INT ToneIndex = 0; ToneIndex < 3; ToneIndex++)
		{
			SetPixelShaderValues<FVector4>(PixelShader,mCBParams[ToneIndex],&CBs[ToneIndex],1);
		}

		// retail's fallback is 1/2.2 exactly (FLOAT_0_45454544), which is the display gamma the client defaults to
		FLOAT InvDisplayGamma = 1.0f / 2.2f;
		if (GEngine && GEngine->Client)
		{
			InvDisplayGamma = 1.0f / GEngine->Client->DisplayGamma;
		}
		const FVector4 LUTParams(
			appPow(2.0f,iParams.m_HDRParameters.m_Exposure),
			iParams.m_HDRParameters.m_GammaAdjustment * InvDisplayGamma,
			CB.m_PreDesaturation,
			CB.m_PostDesaturation);
		SetPixelShaderValues<FVector4>(PixelShader,mLUTParams,&LUTParams,1);

		SetPixelShaderValues<FLinearColor>(PixelShader,mOverlay,&iOverlay,1);

		const FVector4 BrightnessContrast(
			iParams.m_HDRParameters.m_GimpBrightness,
			appTan((iParams.m_HDRParameters.m_GimpContrast + 1.0f) * ((FLOAT)PI / 4.0f)),
			0.0f,
			0.0f);
		SetPixelShaderValues<FVector4>(PixelShader,mBrightnessContrast,&BrightnessContrast,1);
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

/*-----------------------------------------------------------------------------
	FArkUberPpParameters, the script struct the whole graph grades with
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x2a2980 - the colour balance group's neutral state for the fields the content did not
 * override.
 */
void ArkPpColorBalanceSetDefaultOnNoOverride(FArkPpColorBalanceParameters& Params)
{
	if (!Params.m_bOverrideCrMgYbShadTones)
	{
		Params.m_CrMgYbShadTones = FVector(0.0f,0.0f,0.0f);
	}
	if (!Params.m_bOverrideCrMgYbMidTones)
	{
		Params.m_CrMgYbMidTones = FVector(0.0f,0.0f,0.0f);
	}
	if (!Params.m_bOverrideCrMgYbHighTones)
	{
		Params.m_CrMgYbHighTones = FVector(0.0f,0.0f,0.0f);
	}
	if (!Params.m_bOverrideOpacity)
	{
		Params.m_Opacity = 1.0f;
	}
	if (!Params.m_bOverridePreDesaturation)
	{
		Params.m_PreDesaturation = 0.0f;
	}
	if (!Params.m_bOverridePostDesaturation)
	{
		Params.m_PostDesaturation = 0.0f;
	}
}

/** DISHONORED(port): 2013 rva 0x2a2a30 - the same, plus the six override bits cleared. */
void ArkPpColorBalanceForceDefault(FArkPpColorBalanceParameters& Params)
{
	Params.m_bOverrideCrMgYbShadTones = 0;
	Params.m_bOverrideCrMgYbMidTones = 0;
	Params.m_bOverrideCrMgYbHighTones = 0;
	Params.m_bOverrideOpacity = 0;
	Params.m_bOverridePreDesaturation = 0;
	Params.m_bOverridePostDesaturation = 0;
	Params.m_CrMgYbShadTones = FVector(0.0f,0.0f,0.0f);
	Params.m_CrMgYbMidTones = FVector(0.0f,0.0f,0.0f);
	Params.m_CrMgYbHighTones = FVector(0.0f,0.0f,0.0f);
	Params.m_Opacity = 1.0f;
	Params.m_PreDesaturation = 0.0f;
	Params.m_PostDesaturation = 0.0f;
}

/**
 * DISHONORED(port): 2013 rva 0x2a2d20 - what a node's own uber parameters mean before any override is applied. A
 * group the node does not override is forced to the neutral state whatever its own fields say, which is why the
 * depth-of-field node of a chain that grades nothing still bakes an identity LUT.
 *
 * DISHONORED(retail): the m_InFocusRadius line is retail's, bug included. Every other field resets when its override
 * bit is *clear*; this one resets when the bit is *set* (`if ((bits & 2) != 0) m_InFocusRadius = 500`), so a node that
 * overrides the radius has it overwritten with 500 and a node that does not keeps whatever its archetype held. Ported
 * as retail has it: the focus radius of a graded chain comes from the override stack, not from the node.
 */
void ArkUberPpSetDefaultOnNoOverride(FArkUberPpParameters& Params)
{
	if (!Params.m_bOverrideDOFParameters)
	{
		Params.m_DOFParameters.m_bOverrideFocusDistance = 0;
		Params.m_DOFParameters.m_bOverrideInFocusRadius = 0;
		Params.m_DOFParameters.m_bOverrideFarBlurAmount = 0;
		Params.m_DOFParameters.m_FocusDistance = 10000.0f;
		Params.m_DOFParameters.m_InFocusRadius = 500.0f;
		Params.m_DOFParameters.m_FarBlurAmount = 0.0f;
	}
	else
	{
		if (!Params.m_DOFParameters.m_bOverrideFocusDistance)
		{
			Params.m_DOFParameters.m_FocusDistance = 10000.0f;
		}
		if (Params.m_DOFParameters.m_bOverrideInFocusRadius)
		{
			Params.m_DOFParameters.m_InFocusRadius = 500.0f;
		}
		if (!Params.m_DOFParameters.m_bOverrideFarBlurAmount)
		{
			Params.m_DOFParameters.m_FarBlurAmount = 0.0f;
		}
	}

	if (Params.m_bOverrideCBParameters)
	{
		ArkPpColorBalanceSetDefaultOnNoOverride(Params.m_CBParameters);
	}
	else
	{
		ArkPpColorBalanceForceDefault(Params.m_CBParameters);
	}

	if (Params.m_bOverrideHDRParameters)
	{
		if (!Params.m_HDRParameters.m_bOverrideExposure)
		{
			Params.m_HDRParameters.m_Exposure = 0.0f;
		}
		if (!Params.m_HDRParameters.m_bOverrideGammaAdjustment)
		{
			Params.m_HDRParameters.m_GammaAdjustment = 1.0f;
		}
		if (!Params.m_HDRParameters.m_bOverrideFilmGrainNoise)
		{
			Params.m_HDRParameters.m_FilmGrainNoise = 0.0f;
		}
		if (!Params.m_HDRParameters.m_bOverrideGimpBrightness)
		{
			Params.m_HDRParameters.m_GimpBrightness = 0.0f;
		}
		if (!Params.m_HDRParameters.m_bOverrideGimpContrast)
		{
			Params.m_HDRParameters.m_GimpContrast = 0.0f;
		}
	}
	else
	{
		// retail clears only the first three bits of the group here, not the two gimp ones
		Params.m_HDRParameters.m_bOverrideExposure = 0;
		Params.m_HDRParameters.m_bOverrideGammaAdjustment = 0;
		Params.m_HDRParameters.m_bOverrideFilmGrainNoise = 0;
		Params.m_HDRParameters.m_Exposure = 0.0f;
		Params.m_HDRParameters.m_GammaAdjustment = 1.0f;
		Params.m_HDRParameters.m_FilmGrainNoise = 0.0f;
		Params.m_HDRParameters.m_GimpBrightness = 0.0f;
		Params.m_HDRParameters.m_GimpContrast = 0.0f;
	}
}

/** DISHONORED(port): 2013 rva 0x2aca00 - each overridden field lerps towards this one's by iAlpha. */
void ArkPpColorBalanceApplyTo(const FArkPpColorBalanceParameters& Params,FArkPpColorBalanceParameters& oResult,FLOAT iAlpha)
{
	if (Params.m_bOverrideCrMgYbShadTones)
	{
		oResult.m_bOverrideCrMgYbShadTones = 1;
		oResult.m_CrMgYbShadTones += (Params.m_CrMgYbShadTones - oResult.m_CrMgYbShadTones) * iAlpha;
	}
	if (Params.m_bOverrideCrMgYbMidTones)
	{
		oResult.m_bOverrideCrMgYbMidTones = 1;
		oResult.m_CrMgYbMidTones += (Params.m_CrMgYbMidTones - oResult.m_CrMgYbMidTones) * iAlpha;
	}
	if (Params.m_bOverrideCrMgYbHighTones)
	{
		oResult.m_bOverrideCrMgYbHighTones = 1;
		oResult.m_CrMgYbHighTones += (Params.m_CrMgYbHighTones - oResult.m_CrMgYbHighTones) * iAlpha;
	}
	if (Params.m_bOverrideOpacity)
	{
		oResult.m_bOverrideOpacity = 1;
		oResult.m_Opacity += (Params.m_Opacity - oResult.m_Opacity) * iAlpha;
	}
	if (Params.m_bOverridePreDesaturation)
	{
		oResult.m_bOverridePreDesaturation = 1;
		oResult.m_PreDesaturation += (Params.m_PreDesaturation - oResult.m_PreDesaturation) * iAlpha;
	}
	if (Params.m_bOverridePostDesaturation)
	{
		oResult.m_bOverridePostDesaturation = 1;
		oResult.m_PostDesaturation += (Params.m_PostDesaturation - oResult.m_PostDesaturation) * iAlpha;
	}
}

/**
 * DISHONORED(port): 2013 rva 0x2acbc0. bDOFOnlyBlendAmount takes the focus distance and radius whole instead of
 * lerping them, so a camera that snaps its focus does not drag the plane across the world; the far blur amount is
 * always lerped, which is what makes a depth-of-field transition fade in.
 */
void ArkUberPpApplyTo(const FArkUberPpParameters& Params,FArkUberPpParameters& oResult,FLOAT iAlpha,UBOOL bDOFOnlyBlendAmount)
{
	if (Params.m_bOverrideDOFParameters)
	{
		if (Params.m_DOFParameters.m_bOverrideFocusDistance)
		{
			FLOAT Value = Params.m_DOFParameters.m_FocusDistance;
			if (!bDOFOnlyBlendAmount)
			{
				Value = oResult.m_DOFParameters.m_FocusDistance + (Value - oResult.m_DOFParameters.m_FocusDistance) * iAlpha;
			}
			oResult.m_DOFParameters.m_bOverrideFocusDistance = 1;
			oResult.m_DOFParameters.m_FocusDistance = Value;
		}
		if (Params.m_DOFParameters.m_bOverrideInFocusRadius)
		{
			FLOAT Value = Params.m_DOFParameters.m_InFocusRadius;
			if (!bDOFOnlyBlendAmount)
			{
				Value = oResult.m_DOFParameters.m_InFocusRadius + (Value - oResult.m_DOFParameters.m_InFocusRadius) * iAlpha;
			}
			oResult.m_DOFParameters.m_bOverrideInFocusRadius = 1;
			oResult.m_DOFParameters.m_InFocusRadius = Value;
		}
		if (Params.m_DOFParameters.m_bOverrideFarBlurAmount)
		{
			oResult.m_DOFParameters.m_bOverrideFarBlurAmount = 1;
			oResult.m_DOFParameters.m_FarBlurAmount +=
				(Params.m_DOFParameters.m_FarBlurAmount - oResult.m_DOFParameters.m_FarBlurAmount) * iAlpha;
		}
		oResult.m_bOverrideDOFParameters = 1;
	}

	if (Params.m_bOverrideCBParameters)
	{
		ArkPpColorBalanceApplyTo(Params.m_CBParameters,oResult.m_CBParameters,iAlpha);
		oResult.m_bOverrideCBParameters = 1;
	}

	if (Params.m_bOverrideHDRParameters)
	{
		if (Params.m_HDRParameters.m_bOverrideExposure)
		{
			oResult.m_HDRParameters.m_bOverrideExposure = 1;
			oResult.m_HDRParameters.m_Exposure += (Params.m_HDRParameters.m_Exposure - oResult.m_HDRParameters.m_Exposure) * iAlpha;
		}
		if (Params.m_HDRParameters.m_bOverrideGammaAdjustment)
		{
			oResult.m_HDRParameters.m_bOverrideGammaAdjustment = 1;
			oResult.m_HDRParameters.m_GammaAdjustment +=
				(Params.m_HDRParameters.m_GammaAdjustment - oResult.m_HDRParameters.m_GammaAdjustment) * iAlpha;
		}
		if (Params.m_HDRParameters.m_bOverrideFilmGrainNoise)
		{
			oResult.m_HDRParameters.m_bOverrideFilmGrainNoise = 1;
			oResult.m_HDRParameters.m_FilmGrainNoise +=
				(Params.m_HDRParameters.m_FilmGrainNoise - oResult.m_HDRParameters.m_FilmGrainNoise) * iAlpha;
		}
		if (Params.m_HDRParameters.m_bOverrideGimpBrightness)
		{
			oResult.m_HDRParameters.m_bOverrideGimpBrightness = 1;
			oResult.m_HDRParameters.m_GimpBrightness +=
				(Params.m_HDRParameters.m_GimpBrightness - oResult.m_HDRParameters.m_GimpBrightness) * iAlpha;
		}
		if (Params.m_HDRParameters.m_bOverrideGimpContrast)
		{
			oResult.m_HDRParameters.m_bOverrideGimpContrast = 1;
			oResult.m_HDRParameters.m_GimpContrast +=
				(Params.m_HDRParameters.m_GimpContrast - oResult.m_HDRParameters.m_GimpContrast) * iAlpha;
		}
		oResult.m_bOverrideHDRParameters = 1;
	}
}

/*-----------------------------------------------------------------------------
	UArkPpNodeDof / FArkPpNodeDofProxy (2012 PDB 116 bytes; ctor 2013 rva 0x50e7e0, Render 0x523170,
	Downsample 0x522210, LutCreation 0x522750, Blend 0x522990)
-----------------------------------------------------------------------------*/

/** DISHONORED(bringup): -arkppdofdbg reports the resolved uber parameters and the first blend passes. */
static UBOOL ArkPpDofDbg()
{
	static UBOOL bDbg = ParseParam(appCmdLine(),TEXT("arkppdofdbg"));
	return bDbg;
}

/**
 * DISHONORED(port): the depth-of-field node's proxy. The interesting half of the constructor is retail's: the node's
 * own uber parameters are defaulted where they override nothing and then every push on the config's override stack is
 * applied to them, which is how a camera, a volume or a controller reaches the depth-of-field pass.
 */
class FArkPpNodeDofProxy : public FArkPpNodeProxy
{
public:
	/** DISHONORED(port): 2013 rva 0x50e7e0. */
	FArkPpNodeDofProxy(FArkPpCreateProxyConfig& Config,UArkPpNodeDof* InNode)
	{
		m_InProxy = InNode->m_SurfaceTarget ? InNode->m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
		m_UberPpParams = InNode->m_Parameters;
		ArkUberPpSetDefaultOnNoOverride(m_UberPpParams);
		const INT NumOverrides = Config.m_UberOverrides.Num();
		if (NumOverrides > 0)
		{
			// DISHONORED(retail): retail applies push 0 first and then walks the rest from the newest down to 1, so
			// the oldest push is the base and the newest wins; with the one push FViewInfo makes, the order is moot.
			ArkUberPpApplyTo(Config.m_UberOverrides(0).m_UberParams,m_UberPpParams,Config.m_UberOverrides(0).m_Weight,FALSE);
			for (INT OverrideIndex = NumOverrides - 1; OverrideIndex > 0; OverrideIndex--)
			{
				ArkUberPpApplyTo(Config.m_UberOverrides(OverrideIndex).m_UberParams,m_UberPpParams,
					Config.m_UberOverrides(OverrideIndex).m_Weight,FALSE);
			}
		}
		if (m_UberPpParams.m_DOFParameters.m_FarBlurAmount < 0.01f)
		{
			m_UberPpParams.m_DOFParameters.m_FarBlurAmount = 0.0f;
		}

		// DISHONORED(bringup): -arkppdoftestgrade pushes one non-neutral grade through the real ApplyTo so that the
		// whole colour path - override blend, LUT bake, uber lookup - can be measured on content whose own grade is
		// neutral. It stands in for what retail's ULocalPlayer::UpdatePostProcessSettings (2013 rva 0x2b08b0) delivers
		// and this tree does not (see the report); drop it when that function is ported.
		static UBOOL bTestGrade = ParseParam(appCmdLine(),TEXT("arkppdoftestgrade"));
		if (bTestGrade)
		{
			FArkUberPpParameters TestGrade;
			appMemzero(&TestGrade,sizeof(TestGrade));
			TestGrade.m_bOverrideCBParameters = 1;
			TestGrade.m_CBParameters.m_bOverrideCrMgYbHighTones = 1;
			TestGrade.m_CBParameters.m_CrMgYbHighTones = FVector(0.15f,-0.05f,-0.20f);
			TestGrade.m_CBParameters.m_bOverrideCrMgYbShadTones = 1;
			TestGrade.m_CBParameters.m_CrMgYbShadTones = FVector(-0.10f,0.0f,0.15f);
			TestGrade.m_CBParameters.m_bOverrideOpacity = 1;
			TestGrade.m_CBParameters.m_Opacity = 1.0f;
			TestGrade.m_CBParameters.m_bOverridePostDesaturation = 1;
			TestGrade.m_CBParameters.m_PostDesaturation = 0.5f;
			TestGrade.m_bOverrideHDRParameters = 1;
			TestGrade.m_HDRParameters.m_bOverrideExposure = 1;
			TestGrade.m_HDRParameters.m_Exposure = -1.0f;
			TestGrade.m_HDRParameters.m_bOverrideGimpContrast = 1;
			TestGrade.m_HDRParameters.m_GimpContrast = 0.25f;
			ArkUberPpApplyTo(TestGrade,m_UberPpParams,1.0f,FALSE);
		}
		// DISHONORED(retail): m_LinearToGammaRsc (2012 PDB @12) is cleared here and written nowhere in the whole exe,
		// and UArkPpNodeDof::m_LinearToGammaRamp (@108) is never read either: the ramp the uber pass samples is always
		// the one GDofRamp bakes. The member is kept for the layout's sake and stays NULL, as it does in retail.
		if (ArkPpDofDbg())
		{
			static UBOOL bReported = FALSE;
			if (!bReported)
			{
				bReported = TRUE;
				const FArkPpColorBalanceParameters& CB = m_UberPpParams.m_CBParameters;
				debugf(TEXT("DISHONORED(bringup): FArkPp dof uber parameters: %i overrides, groups dof %i cb %i hdr %i"),
					NumOverrides,(INT)m_UberPpParams.m_bOverrideDOFParameters,
					(INT)m_UberPpParams.m_bOverrideCBParameters,(INT)m_UberPpParams.m_bOverrideHDRParameters);
				debugf(TEXT("DISHONORED(bringup):   focus %.1f radius %.1f far blur %.3f | shad (%.3f %.3f %.3f) mid (%.3f %.3f %.3f) high (%.3f %.3f %.3f)"),
					m_UberPpParams.m_DOFParameters.m_FocusDistance,m_UberPpParams.m_DOFParameters.m_InFocusRadius,
					m_UberPpParams.m_DOFParameters.m_FarBlurAmount,
					CB.m_CrMgYbShadTones.X,CB.m_CrMgYbShadTones.Y,CB.m_CrMgYbShadTones.Z,
					CB.m_CrMgYbMidTones.X,CB.m_CrMgYbMidTones.Y,CB.m_CrMgYbMidTones.Z,
					CB.m_CrMgYbHighTones.X,CB.m_CrMgYbHighTones.Y,CB.m_CrMgYbHighTones.Z);
				debugf(TEXT("DISHONORED(bringup):   opacity %.3f pre desat %.3f post desat %.3f | exposure %.3f gamma %.3f grain %.3f brightness %.3f contrast %.3f"),
					CB.m_Opacity,CB.m_PreDesaturation,CB.m_PostDesaturation,
					m_UberPpParams.m_HDRParameters.m_Exposure,m_UberPpParams.m_HDRParameters.m_GammaAdjustment,
					m_UberPpParams.m_HDRParameters.m_FilmGrainNoise,m_UberPpParams.m_HDRParameters.m_GimpBrightness,
					m_UberPpParams.m_HDRParameters.m_GimpContrast);
			}
		}
	}

	UBOOL Downsample(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config);
	UBOOL LutCreation(const FLinearColor& iOverlay);
	UBOOL Blend(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config);

	/**
	 * DISHONORED(port): 2013 rva 0x523170 - render the input with m_bForceToDestination cleared, set the opaque blend
	 * and the solid two-sided rasterizer every pass of this node draws with, then downsample (only when there is a
	 * far blur), bake the lookup table and blend. The bUseLDRSceneColor bit at the end is what keeps
	 * FSceneRenderer::FinishRenderViewTarget from copying the un-post-processed scene over this node's output.
	 */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		SCOPED_DRAW_EVENT(EventDof)(DEC_SCENE_ITEMS,TEXT("ArkPpNodeDof"));
		if (m_bDone)
		{
			return FALSE;
		}
		m_bDone = TRUE;

		// DISHONORED(bringup): -noarkppdof leaves the node's passes out and lets the input reach the destination with
		// the config unchanged, which is the pass-through this package replaces; it is the switch of the pair.
		static UBOOL bNoDof = ParseParam(appCmdLine(),TEXT("noarkppdof"));
		if (bNoDof)
		{
			GDisCensusArkPpSkipped++;
			return m_InProxy ? m_InProxy->Render(Scene,View,Config) : FALSE;
		}

		// retail calls this with no null check; a depth-of-field node with no surface target would take it down
		if (m_InProxy)
		{
			m_InProxy->Render(Scene,View,FArkPpRenderConfig(FALSE));
		}
		GDisCensusArkPpNodes++;

		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		{
			SCOPED_DRAW_EVENT(EventUber)(DEC_SCENE_ITEMS,TEXT("D.O.F.us"));
			if (m_UberPpParams.m_DOFParameters.m_FarBlurAmount > 0.0f)
			{
				Downsample(Scene,View,Config);
			}
			LutCreation(View.OverlayColor);
			Blend(Scene,View,Config);
		}

		View.bUseLDRSceneColor |= 1;
		return TRUE;
	}

	/** DISHONORED(port): everything is the input's (the bodies are folded with the blur proxy's, 2013 rvas 0x50b2f0 / 0x50e4b0). */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetSurface(View) : FSurfaceRHIRef(); }
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetTexture(View) : FTexture2DRHIRef(); }
	virtual UINT GetSurfaceSizeX() { return m_InProxy ? m_InProxy->GetSurfaceSizeX() : 0; }
	virtual UINT GetSurfaceSizeY() { return m_InProxy ? m_InProxy->GetSurfaceSizeY() : 0; }

private:
	/** 2012 PDB @12: cleared by the constructor and written nowhere. See the note there. */
	FTextureRHIRef m_LinearToGammaRsc;
	TRefCountPtr<FArkPpNodeProxy> m_InProxy;
	FArkUberPpParameters m_UberPpParams;
};

/**
 * DISHONORED(port): 2013 rva 0x522210 - two full-screen triangles, the scene colour into the half-size depth-of-field
 * target and that into the filter buffer, then one gaussian blur of the filter buffer. The vertex shader is handed the
 * *node's* surface size both times, not the size of the surface it is reading, which is retail's own choice.
 *
 * DISHONORED(retail): the second pass' destination and the blur's buffer are the engine's FilterColor target, not the
 * m_DofQuarterRT that the node's name would suggest - the quarter target belongs to the common-target node (EPpCt 2).
 * Blend reads mLowResColor back out of FilterColor, so the "low resolution colour" is really a blurred full-size
 * buffer fed from a half-size downsample.
 *
 * DISHONORED(bringup): retail's GaussianBlurFilterBuffer takes an absolute kernel radius; the reference engine's,
 * which is what this tree has, scales the radius by ViewSizeX/1280 and takes a sample-mask pair. The node's own width
 * is passed as the view width and the mask is the no-clamping pair. The pass only runs when the far blur amount is
 * above zero, which the content's own chain leaves at zero.
 */
UBOOL FArkPpNodeDofProxy::Downsample(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
{
	FSurfaceRHIRef iSurfaces[2];
	iSurfaces[0] = GSceneRenderTargets.GetRenderTargetSurface(ArkDofHalf);
	iSurfaces[1] = GSceneRenderTargets.GetFilterColorSurface(SRTI_FilterColor0);

	FTexture2DRHIRef iTextures[2];
	iTextures[0] = GSceneRenderTargets.GetSceneColorTexture();
	iTextures[1] = GSceneRenderTargets.GetRenderTargetTexture(ArkDofHalf);

	TShaderMapRef<FArkPpDofDownsamplePS> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
	TShaderMapRef<FArkPpDofDownsampleVS> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*PixelShader || !*VertexShader)
	{
		return FALSE;
	}

	for (UINT PassIndex = 0; PassIndex < 2; PassIndex++)
	{
		const FSurfaceRHIRef& iSurface = iSurfaces[PassIndex];
		RHISetRenderTarget(iSurface,FSurfaceRHIRef());
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetColorWriteMask(CW_RGBA);

		static FGlobalBoundShaderState PpDofDownsampleBS;
		SetGlobalBoundShaderState(PpDofDownsampleBS,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
			*VertexShader,*PixelShader,sizeof(FVector2D));

		FTexture SourceTexture;
		SourceTexture.TextureRHI = iTextures[PassIndex];
		SourceTexture.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

		FArkPpDofDownsampleParameters Params;
		Params.mSamplerState = SourceTexture.SamplerStateRHI;
		Params.mTexture = SourceTexture.TextureRHI;
		Params.mSizeX = GetSurfaceSizeX();
		Params.mSizeY = GetSurfaceSizeY();

		RHIReduceTextureCachePenalty((*PixelShader)->GetPixelShader());
		(*PixelShader)->SetSourceTexture(SourceTexture);
		(*VertexShader)->SetParameters(Params);

		RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
		RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
		GDisCensusArkPpDraws++;
	}

	GaussianBlurFilterBuffer(View,(FLOAT)GetSurfaceSizeX(),GetSurfaceSizeX(),GetSurfaceSizeY(),7.0f,1.0f,
		SRTI_FilterColor0,FVector2D(-1.0f,-1.0f),FVector2D(2.0f,2.0f));
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x522750 - the colour treatment is baked here. One triangle over the whole 256x16
 * A8R8G8B8 ramp with the LUT blender pair: the pixel shader turns the unwrapped cube coordinate into a linear colour,
 * applies the colour balance, the exposure, the gamma, the two desaturations, the overlay and the brightness/contrast
 * pair, and writes the graded colour back, so the uber pass only has to look a colour up. There is no viewport call
 * because setting a render target already covers all of it, and no blend state because Render set the opaque one for
 * every pass of this node.
 */
UBOOL FArkPpNodeDofProxy::LutCreation(const FLinearColor& iOverlay)
{
	SCOPED_DRAW_EVENT(EventLut)(DEC_SCENE_ITEMS,TEXT("L.U.T.her"));

	RHISetRenderTarget(GDofRamp.mRamp3DSurface,FSurfaceRHIRef());

	TShaderMapRef<FArkPpDofLutBlenderVS> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	TShaderMapRef<FArkPpDofLutBlenderPS> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader || !*PixelShader)
	{
		return FALSE;
	}

	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetColorWriteMask(CW_RGBA);

	static FGlobalBoundShaderState PpDofLutBlenderBS;
	SetGlobalBoundShaderState(PpDofLutBlenderBS,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
		*VertexShader,*PixelShader,sizeof(FVector2D));

	// DISHONORED(bringup): -arkppdoflutblack clears the ramp instead of baking it, so the pair says how much of the
	// frame the uber pass really takes through this lookup table.
	static UBOOL bBlackLut = ParseParam(appCmdLine(),TEXT("arkppdoflutblack"));
	if (bBlackLut)
	{
		RHIClear(TRUE,FLinearColor(0.0f,0.0f,0.0f,0.0f),FALSE,0.0f,FALSE,0);
	}
	else
	{
		(*PixelShader)->SetParameters(m_UberPpParams,iOverlay);
		RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	}
	// retail resolves this one with bKeepOriginalSurface FALSE, unlike every other pass of the node
	RHICopyToResolveTarget(GDofRamp.mRamp3DSurface,FALSE,FResolveParams());
	GDisCensusArkPpDraws++;

	// DISHONORED(bringup): -arkppdoflut writes the baked ramp out once so the colour ramp can be inspected.
	static UBOOL bDumpLut = ParseParam(appCmdLine(),TEXT("arkppdoflut"));
	if (bDumpLut)
	{
		static UBOOL bDumped = FALSE;
		if (!bDumped)
		{
			bDumped = TRUE;
			const UINT LutSizeX = FArkDofRamp<16>::GetSizeX();
			const UINT LutSizeY = FArkDofRamp<16>::GetSizeY();
			TArray<BYTE> RawData;
			RawData.Init(LutSizeX * LutSizeY * sizeof(FColor));
			FReadSurfaceDataFlags ReadFlags;
			RHIReadSurfaceData(GDofRamp.mRamp3DSurface,0,0,LutSizeX - 1,LutSizeY - 1,RawData,ReadFlags);
			TArray<FColor> Bitmap;
			Bitmap.Init(LutSizeX * LutSizeY);
			appMemcpy(&Bitmap(0),&RawData(0),Min<INT>(RawData.Num(),Bitmap.Num() * sizeof(FColor)));
			const FString LutDir = appGameDir() * TEXT("Logs");
			GFileManager->MakeDirectory(*LutDir,TRUE);
			const FString LutName = LutDir * TEXT("ArkDofLut");
			appCreateBitmap(*LutName,LutSizeX,LutSizeY,&Bitmap(0),GFileManager);
			debugf(TEXT("DISHONORED(bringup): FArkPp dof LUT dumped, %ix%i, first %i,%i,%i last %i,%i,%i -> %s"),
				LutSizeX,LutSizeY,
				(INT)Bitmap(0).R,(INT)Bitmap(0).G,(INT)Bitmap(0).B,
				(INT)Bitmap(Bitmap.Num() - 1).R,(INT)Bitmap(Bitmap.Num() - 1).G,(INT)Bitmap(Bitmap.Num() - 1).B,
				*LutName);
		}
	}
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x522990 - the uber pass. It draws one triangle over the destination with
 * TArkPpDofUberPS<1,1> when there is a far blur and <0,1> when there is not, sampling the scene colour, the blurred
 * low-resolution colour and the baked ramp, and writes the graded, focused, grained image.
 *
 * DISHONORED(retail): mSceneColor and mSceneDepth are both bound to the scene colour texture - two separate reads of
 * GSceneRenderTargets.RenderTargets[SceneColor].Texture, checked in the disassembly, not one read the decompiler
 * folded. Depth on this renderer lives in the scene colour's alpha, which is also why the pass takes MinZ_MaxZRatio
 * from the RHI; the only difference between the two bindings is the filter, point for the colour and bilinear for the
 * depth.
 */
UBOOL FArkPpNodeDofProxy::Blend(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
{
	FSurfaceRHIRef iSurface;
	if (!Config.m_bForceToDestination || GSystemSettings.NeedsUpscale())
	{
		iSurface = GetSurface(View);
	}
	else
	{
		iSurface = GSceneRenderTargets.GetBackBuffer();
	}
	if (!iSurface)
	{
		return TRUE;
	}

	RHISetRenderTarget(iSurface,FSurfaceRHIRef());
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetColorWriteMask(CW_RGBA);
	RHISetViewport(
		View.RenderTargetX,
		View.RenderTargetY,
		0.0f,
		View.RenderTargetX + View.RenderTargetSizeX,
		View.RenderTargetY + View.RenderTargetSizeY,
		1.0f);

	// retail builds this texture and never reads it; kept so the sequence of RHI calls matches
	FTexture InputTexture;
	InputTexture.TextureRHI = GetTexture(View);
	InputTexture.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

	FArkPpDofUberParameters Params;
	Params.mSceneColor.SamplerStateRHI = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	Params.mSceneColor.TextureRHI = GSceneRenderTargets.GetSceneColorTexture();
	Params.mSceneDepth.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	Params.mSceneDepth.TextureRHI = GSceneRenderTargets.GetSceneColorTexture();
	Params.mLowResColor.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	Params.mLowResColor.TextureRHI = GSceneRenderTargets.GetFilterColorTexture(SRTI_FilterColor0);
	Params.mLinearToGammaRamp.SamplerStateRHI = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	Params.mLinearToGammaRamp.TextureRHI = GDofRamp.mRamp3D;

	Params.m_FilmGrain = m_UberPpParams.m_HDRParameters.m_FilmGrainNoise;
	Params.m_FocusDistance = m_UberPpParams.m_DOFParameters.m_FocusDistance;
	Params.m_InFocusRadius = m_UberPpParams.m_DOFParameters.m_InFocusRadius;
	Params.m_FarBlurAmount = m_UberPpParams.m_DOFParameters.m_FarBlurAmount;
	Params.mSizeX = GetSurfaceSizeX();
	Params.mSizeY = GetSurfaceSizeY();
	Params.m_Viewport = FVector4(
		(FLOAT)View.RenderTargetX,
		(FLOAT)View.RenderTargetY,
		(FLOAT)(View.RenderTargetX + View.RenderTargetSizeX),
		(FLOAT)(View.RenderTargetY + View.RenderTargetSizeY));
	Params.m_View = &View;

	TShaderMapRef<FArkPpDofUberVS> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader)
	{
		return FALSE;
	}

	if (m_UberPpParams.m_DOFParameters.m_FarBlurAmount <= 0.0f)
	{
		TShaderMapRef<FArkPpDofUber_01PSType> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
		if (!*PixelShader)
		{
			return FALSE;
		}
		static FGlobalBoundShaderState PpDofUber01BS;
		SetGlobalBoundShaderState(PpDofUber01BS,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
			*VertexShader,*PixelShader,sizeof(FVector2D));
		(*PixelShader)->SetParameters(Params);
	}
	else
	{
		TShaderMapRef<FArkPpDofUber_11PSType> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
		if (!*PixelShader)
		{
			return FALSE;
		}
		static FGlobalBoundShaderState PpDofUber11BS;
		SetGlobalBoundShaderState(PpDofUber11BS,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
			*VertexShader,*PixelShader,sizeof(FVector2D));
		(*PixelShader)->SetParameters(Params);
	}

	(*VertexShader)->SetParameters(Params);
	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	GDisCensusArkPpDraws++;

	if (ArkPpDofDbg())
	{
		static INT NumReported = 0;
		if (NumReported < 2)
		{
			NumReported++;
			debugf(TEXT("DISHONORED(bringup): FArkPp dof blend: surface %ix%i, destination %s, back buffer %s, viewport %i,%i %ix%i, far blur %.3f"),
				Params.mSizeX,Params.mSizeY,
				Config.m_bForceToDestination ? TEXT("yes") : TEXT("no"),
				(iSurface == GSceneRenderTargets.GetBackBuffer()) ? TEXT("same") : TEXT("other"),
				View.RenderTargetX,View.RenderTargetY,View.RenderTargetSizeX,View.RenderTargetSizeY,
				m_UberPpParams.m_DOFParameters.m_FarBlurAmount);
		}
	}
	return TRUE;
}

/** DISHONORED(port): 2013 rva 0x5245b0. */
UBOOL UArkPpNodeDof::IsValid(FArkPpIsValidData& Cache)
{
	UBOOL* Memo = Cache.mIsValidCache.Find(this);
	if (Memo)
	{
		return *Memo;
	}
	const UBOOL bValid = m_SurfaceTarget && m_SurfaceTarget->IsValid(Cache);
	Cache.mIsValidCache.Set(this,bValid);
	return bValid;
}

/** DISHONORED(port): 2013 rva 0x524640 - no show gate on this node in retail. */
FArkPpNodeProxy* UArkPpNodeDof::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeDofProxy(Config,this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpDofShaderTypes()
{
}
