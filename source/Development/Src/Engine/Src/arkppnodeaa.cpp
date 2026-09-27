// Engine/src/arkppnodeaa.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (58):
//   0x549c90  public: virtual unsigned int __thiscall UArkPpNodeAA::LinkInput(unsigned int, class UArkPpNode *)
//   0x549cb0  public: virtual unsigned int __thiscall UArkPpNodeAA::UnlinkInput(class UArkPpNode *)
//   0x549ce0  public: virtual unsigned int __thiscall UArkPpNodeAA::UnlinkInput(unsigned int)
//   0x54b270  public: __thiscall TFXAAPixelShader<0, 1, 1>::TFXAAPixelShader<0, 1, 1>(void)
//   0x54b2e0  public: virtual unsigned int __thiscall TFXAAPixelShader<2, 1, 0>::Serialize(class FArchive &)
//   0x54b390  public: virtual unsigned int __thiscall TMLAAEdgeDetectionPixelShader<0>::Serialize(class FArchive &)
//   0x54b3e0  public: virtual unsigned int __thiscall TMLAABlendPixelShader<0>::Serialize(class FArchive &)
//   0x54b440  public: virtual unsigned int __thiscall TMLAABlendPixelShader<1>::Serialize(class FArchive &)
//   0x54b800  public: __thiscall FArkPpNodeAAProxy::FArkPpNodeAAProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeAA *)
//   0x54b8d0  public: virtual class TDynamicRHIResourceReference<12> const __thiscall FArkPpNodeAAProxy::GetSurface(class FViewInfo const &)
//   0x54b900  public: virtual class TDynamicRHIResourceReference<14> const __thiscall FArkPpNodeAAProxy::GetTexture(class FViewInfo const &)
//   0x54b930  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::GetSurfaceSizeX(void)
//   0x54b940  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::GetSurfaceSizeY(void)
//   0x54b950  public: virtual __thiscall FArkPpNodeAAProxy::~FArkPpNodeAAProxy(void)
//   0x54d720  public: static class FShader * __cdecl TFXAAPixelShader<1, 1, 0>::ConstructSerializedInstance(void)
//   0x54d790  public: static class FShader * __cdecl TMLAAEdgeDetectionPixelShader<0>::ConstructSerializedInstance(void)
//   0x54d810  public: static class FShader * __cdecl TMLAAEdgeDetectionPixelShader<1>::ConstructSerializedInstance(void)
//   0x54d890  public: static class FShader * __cdecl TMLAABlendPixelShader<0>::ConstructSerializedInstance(void)
//   0x54d920  public: static class FShader * __cdecl TMLAABlendPixelShader<1>::ConstructSerializedInstance(void)
//   0x54e7b0  public: void __thiscall FFXAAVertexShader::SetParameters(struct FArkPpFxAaParameters const &)
//   0x54e900  public: void __thiscall FMLAAVertexShader::SetParameters(struct FArkPpMlaaParameters const &)
//   0x54e9e0  public: virtual class FString __thiscall UArkPpNodeAA::InputName(unsigned int)const
//   0x555760  public: void __thiscall TMLAAEdgeDetectionPixelShader<0>::SetParameters(struct FArkPpMlaaParameters &)
//   0x5558b0  public: void __thiscall TMLAAEdgeDetectionPixelShader<1>::SetParameters(struct FArkPpMlaaParameters &)
//   0x5587a0  public: void __thiscall TFXAAPixelShader<0, 1, 0>::SetParameters(struct FArkPpFxAaParameters const &)
//   0x558ba0  public: void __thiscall TMLAABlendPixelShader<0>::SetParameters(struct FArkPpMlaaParameters const &)
//   0x558dc0  public: void __thiscall TMLAABlendPixelShader<1>::SetParameters(struct FArkPpMlaaParameters const &)
//   0x55d620  public: static class UClass * __cdecl UArkPpNodeAA::GetPrivateStaticClassUArkPpNodeAA(wchar_t const *)
//   0x55f370  public: static void __cdecl UArkPpNodeAA::InitializePrivateStaticClassUArkPpNodeAA(void)
//   0x560170  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<0, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560250  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<2, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560330  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<1, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560410  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAAEdgeDetectionPixelShader<1>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5604f0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAAEdgeDetectionPixelShader<0>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5605d0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAABlendPixelShader<1>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5606b0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAABlendPixelShader<0>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x561080  public: static class UClass * __cdecl UArkPpNodeAA::StaticClassNoInline(void)
//   0x5610b0  public: unsigned int __thiscall FArkPpNodeAAProxy::RenderFxaa(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x561570  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaEdgeDetectingPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x561990  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaComputeEdgeLengthPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x561ee0  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaBlendColorPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x5623f0  public: unsigned int __thiscall FArkPpNodeAAProxy::RenderMlaa(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5625d0  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x565120  public: virtual unsigned int __thiscall UArkPpNodeAA::IsValid(struct FArkPpIsValidData &)
//   0x5651c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeAA::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f070  _dynamic_initializer_for__FFXAAVertexShader::StaticType__
//   0xb9f0b0  _dynamic_initializer_for__TFXAAPixelShader_0_1_0_::StaticType__
//   0xb9f0f0  _dynamic_initializer_for__TFXAAPixelShader_2_1_0_::StaticType__
//   0xb9f130  _dynamic_initializer_for__TFXAAPixelShader_1_1_0_::StaticType__
//   0xb9f170  _dynamic_initializer_for__TFXAAPixelShader_0_1_1_::StaticType__
//   0xb9f1b0  _dynamic_initializer_for__TFXAAPixelShader_2_1_1_::StaticType__
//   0xb9f1f0  _dynamic_initializer_for__TFXAAPixelShader_1_1_1_::StaticType__
//   0xb9f230  _dynamic_initializer_for__TMLAAEdgeDetectionPixelShader_0_::StaticType__
//   0xb9f270  _dynamic_initializer_for__TMLAAEdgeDetectionPixelShader_1_::StaticType__
//   0xb9f2b0  _dynamic_initializer_for__TMLAABlendPixelShader_0_::StaticType__
//   0xb9f2f0  _dynamic_initializer_for__TMLAABlendPixelShader_1_::StaticType__
//   0xb9f330  _dynamic_initializer_for__FMLAAVertexShader::StaticType__
//   0xb9f370  _dynamic_initializer_for__FMLAAComputeLineLengthPixelShader::StaticType__

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/**
 * DISHONORED(port): the shader types of Arkane's anti-aliasing node (UArkPpNodeAA / FArkPpNodeAAProxy), which runs
 * either FXAA (RenderFxaa, 2013 rva 0x5210b0) or MLAA in three passes (edge detection, line length, blend colour;
 * 0x521570 / 0x521990 / 0x521ee0). 2013 rva 0xb828f0 ff., sources FXAAShader and MLAAShader.
 *
 * FMLAAVertexShader, FFXAAVertexShader and FMLAAComputeLineLengthPixelShader are Arkane's too, but this tree declares
 * them in PostProcessAA.cpp (agent AH gave them the retail layout there) and they already load; only the types with
 * no declaration at all are here.
 */

/**
 * DISHONORED(layout): eleven parameters (2013 rva 0x50b2e0 Serialize): three scene colour textures (the plain one and
 * two exposure-biased copies), the luminance equation, the inverse display gamma and six FXAA tuning constants.
 * The template arguments are the luma source (0 compute, 1 green channel, 2 alpha), the quality preset and whether
 * the pass reads scene colour rather than the node's own surface - which is the cooked name's _ForSceneColor suffix.
 */
template<UINT LumaSource,UINT Quality,UINT bForSceneColor>
class TFXAAPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TFXAAPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TFXAAPixelShader() {}

	TFXAAPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		m_SceneColorTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"),TRUE);
		m_SceneColorExpNegOneTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorExpNegOneTexture"),TRUE);
		m_SceneColorExpNegTwoTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorExpNegTwoTexture"),TRUE);
		m_LuminanceEquationParameter.Bind(Initializer.ParameterMap,TEXT("LuminanceEquation"),TRUE);
		m_InverseDisplayGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseDisplayGamma"),TRUE);
		m_ConsoleRcpFrameOptParameter.Bind(Initializer.ParameterMap,TEXT("fxaaConsoleRcpFrameOpt"),TRUE);
		m_ConsoleRcpFrameOpt2Parameter.Bind(Initializer.ParameterMap,TEXT("fxaaConsoleRcpFrameOpt2"),TRUE);
		m_Console360RcpFrameOpt2Parameter.Bind(Initializer.ParameterMap,TEXT("fxaaConsole360RcpFrameOpt2"),TRUE);
		m_QualityParamsParameter.Bind(Initializer.ParameterMap,TEXT("fxaaQualityParams"),TRUE);
		m_ConsoleParamsParameter.Bind(Initializer.ParameterMap,TEXT("fxaaConsoleParams"),TRUE);
		m_Console360ConstDirParameter.Bind(Initializer.ParameterMap,TEXT("fxaaConsole360ConstDir"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << m_SceneColorTextureParameter;
		Ar << m_SceneColorExpNegOneTextureParameter;
		Ar << m_SceneColorExpNegTwoTextureParameter;
		Ar << m_LuminanceEquationParameter;
		Ar << m_InverseDisplayGammaParameter;
		Ar << m_ConsoleRcpFrameOptParameter;
		Ar << m_ConsoleRcpFrameOpt2Parameter;
		Ar << m_Console360RcpFrameOpt2Parameter;
		Ar << m_QualityParamsParameter;
		Ar << m_ConsoleParamsParameter;
		Ar << m_Console360ConstDirParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter m_SceneColorTextureParameter;
	FShaderResourceParameter m_SceneColorExpNegOneTextureParameter;
	FShaderResourceParameter m_SceneColorExpNegTwoTextureParameter;
	FShaderParameter m_LuminanceEquationParameter;
	FShaderParameter m_InverseDisplayGammaParameter;
	FShaderParameter m_ConsoleRcpFrameOptParameter;
	FShaderParameter m_ConsoleRcpFrameOpt2Parameter;
	FShaderParameter m_Console360RcpFrameOpt2Parameter;
	FShaderParameter m_QualityParamsParameter;
	FShaderParameter m_ConsoleParamsParameter;
	FShaderParameter m_Console360ConstDirParameter;
};

/**
 * DISHONORED(layout): the MLAA edge pass (2013 rva 0x50b390 Serialize, 0x5155e0 / 0x515760 SetParameters). The
 * template argument picks linear (0) or sRGB (1) source colour, and only the linear one carries the inverse display
 * gamma: the cooked FMLAAEdgeDetection_Linear_PixelShader has 4 parameters, the sRGB one 3.
 */
template<UINT bSRGB>
class TMLAAEdgeDetectionPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TMLAAEdgeDetectionPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TMLAAEdgeDetectionPixelShader() {}

	TMLAAEdgeDetectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mSceneTextureParameters.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"),TRUE);
		mRTSizeParameter.Bind(Initializer.ParameterMap,TEXT("RTSize"),TRUE);
		mLuminanceEquationParameter.Bind(Initializer.ParameterMap,TEXT("LuminanceEquation"),TRUE);
		if (bSRGB == 0)
		{
			mInverseDisplayGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseDisplayGamma"),TRUE);
		}
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSceneTextureParameters;
		Ar << mRTSizeParameter;
		Ar << mLuminanceEquationParameter;
		if (bSRGB == 0)
		{
			Ar << mInverseDisplayGammaParameter;
		}
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter mSceneTextureParameters;
	FShaderParameter mRTSizeParameter;
	FShaderParameter mLuminanceEquationParameter;
	FShaderParameter mInverseDisplayGammaParameter;
};

/**
 * DISHONORED(layout): the MLAA blend pass (2013 rva 0x50b3e0 / 0x50b440 Serialize): the source colour, the edge count
 * texture the second pass wrote, the target size and the luminance equation, plus the inverse display gamma for the
 * linear variant only (5 parameters against 4).
 */
template<UINT bSRGB>
class TMLAABlendPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TMLAABlendPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TMLAABlendPixelShader() {}

	TMLAABlendPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		mSceneTextureParameters.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"),TRUE);
		mEdgeCountTextureParameter.Bind(Initializer.ParameterMap,TEXT("EdgeCountTexture"),TRUE);
		mRTSizeParameter.Bind(Initializer.ParameterMap,TEXT("RTSize"),TRUE);
		mLuminanceEquationParameter.Bind(Initializer.ParameterMap,TEXT("LuminanceEquation"),TRUE);
		if (bSRGB == 0)
		{
			mInverseDisplayGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseDisplayGamma"),TRUE);
		}
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSceneTextureParameters;
		Ar << mEdgeCountTextureParameter;
		Ar << mRTSizeParameter;
		Ar << mLuminanceEquationParameter;
		if (bSRGB == 0)
		{
			Ar << mInverseDisplayGammaParameter;
		}
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter mSceneTextureParameters;
	FShaderResourceParameter mEdgeCountTextureParameter;
	FShaderParameter mRTSizeParameter;
	FShaderParameter mLuminanceEquationParameter;
	FShaderParameter mInverseDisplayGammaParameter;
};

// DISHONORED(retail): 2013 rva 0xb82930 .. 0xb82b70. The three _ForSceneColor variants have no cooked shader in the
// PC cache - retail registers them all the same, so they are declared here for the same reason.
typedef TFXAAPixelShader<0,1,0> FFXAAPixelShader_ComputeLuma_SRGBColorType;
typedef TFXAAPixelShader<2,1,0> FFXAAPixelShader_LumaInAlpha_SRGBColorType;
typedef TFXAAPixelShader<1,1,0> FFXAAPixelShader_LumaAsGreen_SRGBColorType;
typedef TFXAAPixelShader<0,1,1> FFXAAPixelShader_ComputeLuma_SRGBColor_ForSceneColorType;
typedef TFXAAPixelShader<2,1,1> FFXAAPixelShader_LumaInAlpha_SRGBColor_ForSceneColorType;
typedef TFXAAPixelShader<1,1,1> FFXAAPixelShader_LumaAsGreen_SRGBColor_ForSceneColorType;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_ComputeLuma_SRGBColorType,TEXT("FFXAAPixelShader_ComputeLuma_SRGBColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_LumaInAlpha_SRGBColorType,TEXT("FFXAAPixelShader_LumaInAlpha_SRGBColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_LumaAsGreen_SRGBColorType,TEXT("FFXAAPixelShader_LumaAsGreen_SRGBColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_ComputeLuma_SRGBColor_ForSceneColorType,TEXT("FFXAAPixelShader_ComputeLuma_SRGBColor_ForSceneColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_LumaInAlpha_SRGBColor_ForSceneColorType,TEXT("FFXAAPixelShader_LumaInAlpha_SRGBColor_ForSceneColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FFXAAPixelShader_LumaAsGreen_SRGBColor_ForSceneColorType,TEXT("FFXAAPixelShader_LumaAsGreen_SRGBColor_ForSceneColor"),TEXT("FXAAShader"),TEXT("FXAA_PixelMain"),SF_Pixel,786,24);

typedef TMLAAEdgeDetectionPixelShader<0> FMLAAEdgeDetection_Linear_PixelShaderType;
typedef TMLAAEdgeDetectionPixelShader<1> FMLAAEdgeDetection_SRGB_PixelShaderType;
typedef TMLAABlendPixelShader<0> FMLAABlend_Linear_PixelShaderType;
typedef TMLAABlendPixelShader<1> FMLAABlend_SRGB_PixelShaderType;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FMLAAEdgeDetection_Linear_PixelShaderType,TEXT("FMLAAEdgeDetection_Linear_PixelShader"),TEXT("MLAAShader"),TEXT("MLAA_SeperatingLines_PS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FMLAAEdgeDetection_SRGB_PixelShaderType,TEXT("FMLAAEdgeDetection_SRGB_PixelShader"),TEXT("MLAAShader"),TEXT("MLAA_SeperatingLines_PS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FMLAABlend_Linear_PixelShaderType,TEXT("FMLAABlend_Linear_PixelShader"),TEXT("MLAAShader"),TEXT("MLAA_BlendColor_PS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FMLAABlend_SRGB_PixelShaderType,TEXT("FMLAABlend_SRGB_PixelShader"),TEXT("MLAAShader"),TEXT("MLAA_BlendColor_PS"),SF_Pixel,786,1);

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpAAShaderTypes()
{
}
