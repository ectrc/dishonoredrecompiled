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
#include "SceneRenderTargets.h"
#include "arkcommonvertexdeclaration.h"
#include "arkpp.h"
#include "arkppnodeaa.h"

/**
 * DISHONORED(port): the shader types and the passes of Arkane's anti-aliasing node (UArkPpNodeAA /
 * FArkPpNodeAAProxy), which runs either MLAA in three passes (edge detection, line length, blend colour;
 * 2013 rva 0x520880 / 0x520ca0 / 0x5211f0, driver 0x521720) or FXAA in one (RenderFxaa, 0x5203c0).
 * 2013 rva 0xb828f0 ff., sources FXAAShader and MLAAShader.
 *
 * DISHONORED(retail, agent EE): the four pass addresses this comment used to carry (0x5210b0, 0x521570, 0x521990,
 * 0x521ee0) were the 2012 build's with one nibble changed, and 0x521990 happens to be retail's RenderMotionBlur.
 * The retail addresses above are resolved against retail2013_named.i64 and the 2012/2013 match table.
 *
 * FMLAAVertexShader, FFXAAVertexShader and FMLAAComputeLineLengthPixelShader are Arkane's too and retail declares
 * them here; this tree inherited them in PostProcessAA.cpp, so they now live in arkppnodeaa.h, which both units
 * include, and their SetParameters bodies are below - which is where retail's are.
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

	/**
	 * DISHONORED(port): 2013 rva 0x518480 (2012 0x5587a0). The six FXAA tuning constants are literals in retail, not
	 * content: quality (0.75, 0.166, 0.0833), console (8, 0.125, 0.05, 0) and the console direction (1, -1, 0.25,
	 * -0.25); the three frame-option vectors are +-0.5, +-2 and 8 / -4 texels of the *scene colour buffer*, not of the
	 * node's surface. The two exposure-biased scene colour samplers of the layout are never set: they are the Xbox 360
	 * path.
	 */
	void SetParameters(const FArkPpFxAaParameters& iParams)
	{
		const FPixelShaderRHIParamRef Shader = GetPixelShader();

		SetPixelShaderValue(Shader,m_LuminanceEquationParameter,
			FVector4(iParams.mLumEquation.X,iParams.mLumEquation.Y,iParams.mLumEquation.Z,0.0f));

		const FRenderTarget* RenderTarget = iParams.mView->Family->RenderTarget;
		SetPixelShaderValue(Shader,m_InverseDisplayGammaParameter,1.0f / RenderTarget->GetDisplayGamma());

		if (iParams.mbUseSceneColorLdr)
		{
			// DISHONORED(retail): retail reads RenderTargets[15] here, which is its own LDR scene colour slot. Every
			// call site of every pass sets mbUseSceneColorLdr FALSE, so this branch never runs in the shipped game.
			SetTextureParameterDirectly(Shader,m_SceneColorTextureParameter,
				TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetSceneColorLDRTexture());
		}
		else
		{
			SetTextureParameter(Shader,m_SceneColorTextureParameter,&iParams.mSceneColor);
		}

		const FLOAT OOBufferX = 1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeX();
		const FLOAT OOBufferY = 1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeY();
		SetPixelShaderValue(Shader,m_ConsoleRcpFrameOptParameter,
			FVector4(-0.5f * OOBufferX,-0.5f * OOBufferY,0.5f * OOBufferX,0.5f * OOBufferY));
		SetPixelShaderValue(Shader,m_ConsoleRcpFrameOpt2Parameter,
			FVector4(-2.0f * OOBufferX,-2.0f * OOBufferY,2.0f * OOBufferX,2.0f * OOBufferY));
		SetPixelShaderValue(Shader,m_Console360RcpFrameOpt2Parameter,
			FVector4(8.0f * OOBufferX,8.0f * OOBufferY,-4.0f * OOBufferX,-4.0f * OOBufferY));
		SetPixelShaderValue(Shader,m_QualityParamsParameter,FVector4(0.75f,0.166f,0.0833f,0.0f));
		SetPixelShaderValue(Shader,m_ConsoleParamsParameter,FVector4(8.0f,0.125f,0.05f,0.0f));
		SetPixelShaderValue(Shader,m_Console360ConstDirParameter,FVector4(1.0f,-1.0f,0.25f,-0.25f));
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

	/**
	 * DISHONORED(port): 2012 rva 0x555760 (linear) / 0x5558b0 (sRGB); the match table has no 2013 address for either,
	 * their retail bodies are identical bar the display gamma. RTSize is the *scene colour buffer*'s reciprocal size,
	 * and the edge threshold rides in the luminance equation's W as its reciprocal.
	 */
	void SetParameters(const FArkPpMlaaParameters& iParams)
	{
		const FPixelShaderRHIParamRef Shader = GetPixelShader();
		SetPixelShaderValue(Shader,mRTSizeParameter,
			FVector4(1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeY(),0.0f,0.0f));
		SetPixelShaderValue(Shader,mLuminanceEquationParameter,
			FVector4(iParams.mLumEquation.X,iParams.mLumEquation.Y,iParams.mLumEquation.Z,1.0f / iParams.mEdgeThresold));
		if (bSRGB == 0)
		{
			const FRenderTarget* RenderTarget = iParams.mView->Family->RenderTarget;
			SetPixelShaderValue(Shader,mInverseDisplayGammaParameter,1.0f / RenderTarget->GetDisplayGamma());
		}
		SetTextureParameter(Shader,mSceneTextureParameters,&iParams.mSceneColor);
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

	/**
	 * DISHONORED(port): 2012 rva 0x558ba0 (linear) / 0x558dc0 (sRGB). The same three constants as the edge pass plus
	 * the edge count target the second pass wrote, point-sampled and clamped.
	 */
	void SetParameters(const FArkPpMlaaParameters& iParams)
	{
		const FPixelShaderRHIParamRef Shader = GetPixelShader();
		SetPixelShaderValue(Shader,mRTSizeParameter,
			FVector4(1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeY(),0.0f,0.0f));
		SetPixelShaderValue(Shader,mLuminanceEquationParameter,
			FVector4(iParams.mLumEquation.X,iParams.mLumEquation.Y,iParams.mLumEquation.Z,1.0f / iParams.mEdgeThresold));
		if (bSRGB == 0)
		{
			const FRenderTarget* RenderTarget = iParams.mView->Family->RenderTarget;
			SetPixelShaderValue(Shader,mInverseDisplayGammaParameter,1.0f / RenderTarget->GetDisplayGamma());
		}
		SetTextureParameterDirectly(Shader,mEdgeCountTextureParameter,
			TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			GSceneRenderTargets.GetRenderTargetTexture(MLAAEdgeCount));
		SetTextureParameter(Shader,mSceneTextureParameters,&iParams.mSceneColor);
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

/*-----------------------------------------------------------------------------
	UArkPpNodeAA / FArkPpNodeAAProxy (2012 PDB 52 bytes; ctor 0x54b800, Render 0x5625d0,
	RenderFxaa 0x5610b0, RenderMlaa 0x5623f0 with its three passes 0x561570 / 0x561990 / 0x561ee0)
-----------------------------------------------------------------------------*/

/** DISHONORED(bringup): one line per shader type whose cooked record is missing, so a pass that cannot run says so. */
static void ReportMissingArkPpAAShader(const TCHAR* Name)
{
	static UBOOL bReported = FALSE;
	if (!bReported)
	{
		bReported = TRUE;
		warnf(TEXT("DISHONORED(bringup): dishonored aa: no cooked shader for %s, the pass is skipped"),Name);
	}
}

/** DISHONORED(bringup): the antialiasing node's share of the post-process census (arkppnodes.cpp holds the rest). */
INT GDisCensusArkPpAADraws = 0;

/** DISHONORED(bringup): -noarkppaa leaves both antialiasing passes out; the switch of this package's pair. */
static UBOOL DishonoredNoArkPpAA()
{
	// a file-scope static ParseParam in a static library runs before WinMain sets GCmdLine (agent CA): read on first use
	static UBOOL bNo = ParseParam(appCmdLine(),TEXT("noarkppaa"));
	return bNo;
}

/** DISHONORED(bringup): -arkppaadbg reports the resolved type and configuration once and the first passes. */
static UBOOL DishonoredArkPpAADbg()
{
	static UBOOL bDbg = ParseParam(appCmdLine(),TEXT("arkppaadbg"));
	return bDbg;
}

/** DISHONORED(bringup): -arkppaafxaa forces the FXAA branch, -arkppaamlaa the MLAA one, whatever the node asks for. */
static UBOOL DishonoredForceFxaa()
{
	static UBOOL bForce = ParseParam(appCmdLine(),TEXT("arkppaafxaa"));
	return bForce;
}

static UBOOL DishonoredForceMlaa()
{
	static UBOOL bForce = ParseParam(appCmdLine(),TEXT("arkppaamlaa"));
	return bForce;
}

/**
 * DISHONORED(port): 2013 rva 0x51fe10 ff. - FArkPpNodeAAProxy::FlushShader<PS> / FlushMlaaEdgeShader<PS>
 * (2012 0x560170 / 0x560250 / 0x560330 and 0x560410 / 0x5604f0 / 0x5605d0 / 0x5606b0): look the pixel shader up, set
 * the bound shader state against the common float2 declaration, then hand the pass argument to the pixel shader. Each
 * instantiation owns its own static bound shader state, exactly as retail's function-local statics do.
 */
template<typename PixelShaderType>
static UBOOL FlushFxAaShader(TShaderMapRef<FFXAAVertexShader>& iVS,const FArkPpFxAaParameters& iParams,const TCHAR* Name)
{
	TShaderMapRef<PixelShaderType> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*PixelShader)
	{
		ReportMissingArkPpAAShader(Name);
		return FALSE;
	}
	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(BoundShaderState,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
		*iVS,*PixelShader,sizeof(FVector2D));
	(*PixelShader)->SetParameters(iParams);
	return TRUE;
}

template<typename PixelShaderType>
static UBOOL FlushMlaaShader(TShaderMapRef<FMLAAVertexShader>& iVS,const FArkPpMlaaParameters& iParams,const TCHAR* Name)
{
	TShaderMapRef<PixelShaderType> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*PixelShader)
	{
		ReportMissingArkPpAAShader(Name);
		return FALSE;
	}
	static FGlobalBoundShaderState BoundShaderState;
	SetGlobalBoundShaderState(BoundShaderState,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
		*iVS,*PixelShader,sizeof(FVector2D));
	(*PixelShader)->SetParameters(iParams);
	return TRUE;
}

/**
 * DISHONORED(port): the antialiasing node's proxy: the graph shape, the configuration, the surface delegation and both
 * antialiasing passes are retail's (ctor 2013 rva 0x50afe0, Render 0x5218d0, RenderFxaa 0x5203c0, RenderMlaa 0x521720
 * and its three passes 0x520880 / 0x520ca0 / 0x5211f0).
 */
class FArkPpNodeAAProxy : public FArkPpNodeProxy
{
public:
	/** DISHONORED(port): 2013 rva 0x50afe0 (2012 0x54b800). */
	FArkPpNodeAAProxy(FArkPpCreateProxyConfig& Config,UArkPpNodeAA* InNode)
		: m_Type(InNode->m_Type)
		, m_FxAaConfig(InNode->m_FxAaConfig)
		, m_MlAaConfig(InNode->m_MlAaConfig)
	{
		m_InProxy = InNode->m_SurfaceTarget ? InNode->m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
	}

	/**
	 * DISHONORED(port): 2013 rva 0x5218d0 (2012 0x5625d0) - retail renders the input with m_bForceToDestination cleared
	 * and then draws MLAA (m_Type 1) or FXAA (m_Type 2); any other type only renders the input, with the bit *kept*,
	 * so the input owns the destination instead.
	 */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (m_bDone)
		{
			return FALSE;
		}
		m_bDone = TRUE;
		if (!m_InProxy)
		{
			// retail dereferences m_InProxy with no check; an AA node with no surface target would take it down
			GDisCensusArkPpSkipped++;
			return FALSE;
		}

		BYTE Type = m_Type;
		if (DishonoredForceFxaa())
		{
			Type = EPpAa_Fxaa;
		}
		else if (DishonoredForceMlaa())
		{
			Type = EPpAa_Mlaa;
		}
		if (DishonoredNoArkPpAA())
		{
			GDisCensusArkPpSkipped++;
			return m_InProxy->Render(Scene,View,Config);
		}
		if (Type != EPpAa_Mlaa && Type != EPpAa_Fxaa)
		{
			GDisCensusArkPpSkipped++;
			return m_InProxy->Render(Scene,View,Config);
		}

		if (DishonoredArkPpAADbg())
		{
			static UBOOL bReported = FALSE;
			if (!bReported)
			{
				bReported = TRUE;
				debugf(TEXT("DISHONORED(bringup): dishonored aa node: type %u (%s), luma source %u, lum equation (%.3f %.3f %.3f), mlaa threshold %.4f, iType_AntiAlias %u, destination %s"),
					(UINT)Type,Type == EPpAa_Mlaa ? TEXT("MLAA") : TEXT("FXAA"),(UINT)m_FxAaConfig.m_Luma,
					m_FxAaConfig.m_LuminanceEquation.X,m_FxAaConfig.m_LuminanceEquation.Y,m_FxAaConfig.m_LuminanceEquation.Z,
					m_MlAaConfig.m_EdgeDetectionThresold,(UINT)GSystemSettings.iType_AntiAlias,
					Config.m_bForceToDestination ? TEXT("back buffer") : TEXT("own surface"));
				const FTexture2DRHIRef InputTexture = m_InProxy->GetTexture(View);
				debugf(TEXT("DISHONORED(bringup): dishonored aa input: %ux%u, texture is %s, surface is %s"),
					m_InProxy->GetSurfaceSizeX(),m_InProxy->GetSurfaceSizeY(),
					InputTexture == GSceneRenderTargets.GetSceneColorTexture() ? TEXT("SceneColor")
						: (InputTexture == GSceneRenderTargets.GetSceneColorLDRTexture() ? TEXT("SceneColorLDR") : TEXT("some other target")),
					m_InProxy->GetSurface(View) == GSceneRenderTargets.GetSceneColorSurface() ? TEXT("SceneColor")
						: (m_InProxy->GetSurface(View) == GSceneRenderTargets.GetSceneColorLDRSurface() ? TEXT("SceneColorLDR") : TEXT("some other target")));
			}
		}

		m_InProxy->Render(Scene,View,FArkPpRenderConfig(FALSE));
		if (Type == EPpAa_Mlaa)
		{
			RenderMlaa(Scene,View,Config);
		}
		else
		{
			RenderFxaa(Scene,View,Config);
		}
		return TRUE;
	}

	UBOOL RenderFxaa(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config);
	UBOOL RenderMlaa(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config);
	void RenderMlaaEdgeDetectingPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace);
	void RenderMlaaComputeEdgeLengthPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace);
	void RenderMlaaBlendColorPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace);

	/** DISHONORED(port): 2013 rva 0x50b0b0 (2012 0x54b8d0) / 0x54b900 / 0x54b930 / 0x54b940 - everything is the input's. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetSurface(View) : FSurfaceRHIRef(); }
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetTexture(View) : FTexture2DRHIRef(); }
	virtual UINT GetSurfaceSizeX() { return m_InProxy ? m_InProxy->GetSurfaceSizeX() : 0; }
	virtual UINT GetSurfaceSizeY() { return m_InProxy ? m_InProxy->GetSurfaceSizeY() : 0; }

private:
	/** The surface the node draws into: the back buffer when this node ends the graph and the view is not upscaled. */
	const FSurfaceRHIRef Destination(const FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (!Config.m_bForceToDestination || GSystemSettings.NeedsUpscale())
		{
			return GetSurface(View);
		}
		return GSceneRenderTargets.GetBackBuffer();
	}

	/** The pass argument both shaders read: the node input's texture and size and the view's rectangle inside it. */
	void FillCommon(FViewInfo& View,UINT& OutSizeX,UINT& OutSizeY,FVector4& OutViewport,FTexture& OutSource,ESamplerFilter Filter)
	{
		OutSource.TextureRHI = m_InProxy->GetTexture(View);
		OutSource.SamplerStateRHI = Filter == SF_Point
			? TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI()
			: TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
		OutSizeX = m_InProxy->GetSurfaceSizeX();
		OutSizeY = m_InProxy->GetSurfaceSizeY();

		const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
		const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
		const UINT MinX = OutSizeX * View.RenderTargetX / BufferSizeX;
		const UINT MinY = OutSizeY * View.RenderTargetY / BufferSizeY;
		const UINT MaxX = OutSizeX * (View.RenderTargetX + View.RenderTargetSizeX) / BufferSizeX;
		const UINT MaxY = OutSizeY * (View.RenderTargetY + View.RenderTargetSizeY) / BufferSizeY;
		RHISetViewport(MinX,MinY,0.0f,MaxX,MaxY,1.0f);
		OutViewport = FVector4((FLOAT)MinX,(FLOAT)MinY,(FLOAT)MaxX,(FLOAT)MaxY);
	}

	TRefCountPtr<FArkPpNodeProxy> m_InProxy;
	BYTE m_Type;
	FFxAaConfig m_FxAaConfig;
	FMlAaConfig m_MlAaConfig;
};

/**
 * DISHONORED(port): 2013 rva 0x5203c0 (2012 0x5610b0). One full-screen triangle of the node's input through the FXAA
 * pixel shader the configuration's luma source picks: 0 computes the luminance, 1 reads the green channel, 2 the alpha.
 * The quality preset is always 1 and the _ForSceneColor variants are never chosen, which is why they have no cooked
 * shader. The colour write mask is set after the shaders and before the draw, as retail does.
 */
UBOOL FArkPpNodeAAProxy::RenderFxaa(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
{
	SCOPED_DRAW_EVENT(EventFxaa)(DEC_SCENE_ITEMS,TEXT("FxAa"));

	const FSurfaceRHIRef iSurface = Destination(View,Config);
	if (!IsValidRef(iSurface))
	{
		return TRUE;
	}
	RHISetRenderTarget(iSurface,FSurfaceRHIRef());

	FArkPpFxAaParameters Params;
	Params.mLumEquation = m_FxAaConfig.m_LuminanceEquation;
	Params.mView = &View;
	Params.mbUseSceneColorLdr = FALSE;
	FillCommon(View,Params.mSizeX,Params.mSizeY,Params.m_Viewport,Params.mSceneColor,SF_Bilinear);

	TShaderMapRef<FFXAAVertexShader> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader)
	{
		ReportMissingArkPpAAShader(TEXT("FFXAAVertexShader"));
		return FALSE;
	}
	(*VertexShader)->SetParameters(Params);

	// DISHONORED(retail): retail looks each pixel shader up with a bare TShaderMapRef, which appErrorfs on a missing
	// record; a post-process type with no cooked shader would take the render thread down, so the pass reports and skips
	switch (m_FxAaConfig.m_Luma)
	{
	case 2: // the luma is in scene colour's alpha
		if (!FlushFxAaShader<FFXAAPixelShader_LumaInAlpha_SRGBColorType>(VertexShader,Params,TEXT("FFXAAPixelShader_LumaInAlpha_SRGBColor")))
		{
			return FALSE;
		}
		break;
	case 1: // the luma is the green channel
		if (!FlushFxAaShader<FFXAAPixelShader_LumaAsGreen_SRGBColorType>(VertexShader,Params,TEXT("FFXAAPixelShader_LumaAsGreen_SRGBColor")))
		{
			return FALSE;
		}
		break;
	default:
		if (!FlushFxAaShader<FFXAAPixelShader_ComputeLuma_SRGBColorType>(VertexShader,Params,TEXT("FFXAAPixelShader_ComputeLuma_SRGBColor")))
		{
			return FALSE;
		}
		break;
	}

	RHISetColorWriteMask(CW_RGBA);
	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	GDisCensusArkPpDraws++;
	GDisCensusArkPpAADraws++;
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x521720 (2012 0x5623f0). MLAA falls back to FXAA when the system settings forbid it,
 * then sets the three states once for the whole technique and runs the three passes with the view's LDR flag as the
 * source colour space.
 */
UBOOL FArkPpNodeAAProxy::RenderMlaa(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
{
	// DISHONORED(retail): the 2012 build opens with `if (!GSystemSettings.RenderThreadSettings.bAllowMLAA) return
	// RenderFxaa(...)`. Retail 2013 has no such test (0x521720 goes straight to the three passes) because 2013 dropped
	// bAllowMLAA from FSystemSettingsData for iType_AntiAlias @128, which picks the node's m_Type instead. Ported as
	// retail 2013 has it, which also keeps the pass off RenderThreadSettings - a structure nothing in this tree writes.
	SCOPED_DRAW_EVENT(EventMlaa)(DEC_SCENE_ITEMS,TEXT("MLAA"));

	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());

	// DISHONORED(bringup): -arkppaaclearmasks clears both MLAA targets first. Retail clears neither; this is the pair
	// that says whether unwritten target memory is what a run-to-run difference in the blended frame comes from.
	static UBOOL bClearMasks = ParseParam(appCmdLine(),TEXT("arkppaaclearmasks"));
	if (bClearMasks)
	{
		RHISetRenderTarget(GSceneRenderTargets.GetRenderTargetSurface(MLAAEdgeMask),FSurfaceRHIRef());
		RHIClear(TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
		RHISetRenderTarget(GSceneRenderTargets.GetRenderTargetSurface(MLAAEdgeCount),FSurfaceRHIRef());
		RHIClear(TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
	}

	const UBOOL bInSRGBSpace = View.bUseLDRSceneColor != 0;
	RenderMlaaEdgeDetectingPass(Scene,View,Config,bInSRGBSpace);
	RenderMlaaComputeEdgeLengthPass(Scene,View,Config,bInSRGBSpace);
	RenderMlaaBlendColorPass(Scene,View,Config,bInSRGBSpace);

	RHISetColorWriteMask(CW_RGBA);
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x520880 (2012 0x561570). The separating lines go into the full-size MLAAEdgeMask target,
 * red channel only, from a point-sampled read of the node's input.
 */
void FArkPpNodeAAProxy::RenderMlaaEdgeDetectingPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace)
{
	SCOPED_DRAW_EVENT(EventEdge)(DEC_SCENE_ITEMS,TEXT("EdgeDetectingPass"));

	const FSurfaceRHIRef& iSurface = GSceneRenderTargets.GetRenderTargetSurface(MLAAEdgeMask);
	if (!IsValidRef(iSurface))
	{
		return;
	}
	RHISetRenderTarget(iSurface,FSurfaceRHIRef());

	FArkPpMlaaParameters Params;
	Params.mLumEquation = m_MlAaConfig.m_LuminanceEquation;
	Params.mEdgeThresold = m_MlAaConfig.m_EdgeDetectionThresold;
	Params.mView = &View;
	Params.mbUseSceneColorLdr = FALSE;
	FillCommon(View,Params.mSizeX,Params.mSizeY,Params.m_Viewport,Params.mSceneColor,SF_Point);

	TShaderMapRef<FMLAAVertexShader> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader)
	{
		ReportMissingArkPpAAShader(TEXT("FMLAAVertexShader"));
		return;
	}
	if (bInSRGBSpace)
	{
		if (!FlushMlaaShader<FMLAAEdgeDetection_SRGB_PixelShaderType>(VertexShader,Params,TEXT("FMLAAEdgeDetection_SRGB_PixelShader")))
		{
			return;
		}
	}
	else
	{
		if (!FlushMlaaShader<FMLAAEdgeDetection_Linear_PixelShaderType>(VertexShader,Params,TEXT("FMLAAEdgeDetection_Linear_PixelShader")))
		{
			return;
		}
	}
	(*VertexShader)->SetParameters(Params);

	RHISetColorWriteMask(CW_RED);
	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	GDisCensusArkPpDraws++;
	GDisCensusArkPpAADraws++;
}

/**
 * DISHONORED(port): 2013 rva 0x520ca0 (2012 0x520ca0's 2012 original 0x561990). The line lengths go into MLAAEdgeCount,
 * red and green, from the edge mask the first pass wrote. This pass sets its pixel shader's two parameters itself
 * rather than through a SetParameters of its own, which is how retail has it.
 */
void FArkPpNodeAAProxy::RenderMlaaComputeEdgeLengthPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace)
{
	SCOPED_DRAW_EVENT(EventLength)(DEC_SCENE_ITEMS,TEXT("EdgeLengthPass"));

	const FSurfaceRHIRef& iSurface = GSceneRenderTargets.GetRenderTargetSurface(MLAAEdgeCount);
	if (!IsValidRef(iSurface))
	{
		return;
	}
	RHISetRenderTarget(iSurface,FSurfaceRHIRef());

	FArkPpMlaaParameters Params;
	Params.mLumEquation = m_MlAaConfig.m_LuminanceEquation;
	Params.mEdgeThresold = m_MlAaConfig.m_EdgeDetectionThresold;
	Params.mView = &View;
	Params.mbUseSceneColorLdr = FALSE;
	FillCommon(View,Params.mSizeX,Params.mSizeY,Params.m_Viewport,Params.mSceneColor,SF_Point);

	TShaderMapRef<FMLAAVertexShader> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	TShaderMapRef<FMLAAComputeLineLengthPixelShader> PixelShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader || !*PixelShader)
	{
		ReportMissingArkPpAAShader(TEXT("FMLAAComputeLineLengthPixelShader"));
		return;
	}
	static FGlobalBoundShaderState MlaaLengthBS;
	SetGlobalBoundShaderState(MlaaLengthBS,ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),
		*VertexShader,*PixelShader,sizeof(FVector2D));

	SetPixelShaderValue((*PixelShader)->GetPixelShader(),(*PixelShader)->MLAAParameter,
		FVector4(1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeX(),1.0f / (FLOAT)GSceneRenderTargets.GetBufferSizeY(),0.0f,0.0f));
	SetTextureParameterDirectly((*PixelShader)->GetPixelShader(),(*PixelShader)->EdgeMaskTextureParameter,
		TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
		GSceneRenderTargets.GetRenderTargetTexture(MLAAEdgeMask));
	(*VertexShader)->SetParameters(Params);

	RHISetColorWriteMask(CW_RED | CW_GREEN);
	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	GDisCensusArkPpDraws++;
	GDisCensusArkPpAADraws++;
}

/**
 * DISHONORED(port): 2013 rva 0x5211f0 (2012 0x561ee0). The blend writes the antialiased colour into the node's
 * destination - the back buffer when the node ends the graph - RGB only, so scene colour's alpha (which is depth on
 * this renderer) survives.
 */
void FArkPpNodeAAProxy::RenderMlaaBlendColorPass(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config,UBOOL bInSRGBSpace)
{
	SCOPED_DRAW_EVENT(EventBlend)(DEC_SCENE_ITEMS,TEXT("BlendColorPass"));

	const FSurfaceRHIRef iSurface = Destination(View,Config);
	if (!IsValidRef(iSurface))
	{
		return;
	}
	RHISetRenderTarget(iSurface,FSurfaceRHIRef());

	FArkPpMlaaParameters Params;
	Params.mLumEquation = m_MlAaConfig.m_LuminanceEquation;
	Params.mEdgeThresold = m_MlAaConfig.m_EdgeDetectionThresold;
	Params.mView = &View;
	Params.mbUseSceneColorLdr = FALSE;
	FillCommon(View,Params.mSizeX,Params.mSizeY,Params.m_Viewport,Params.mSceneColor,SF_Point);

	TShaderMapRef<FMLAAVertexShader> VertexShader(GetGlobalShaderMap(GRHIShaderPlatform));
	if (!*VertexShader)
	{
		ReportMissingArkPpAAShader(TEXT("FMLAAVertexShader"));
		return;
	}
	if (bInSRGBSpace)
	{
		if (!FlushMlaaShader<FMLAABlend_SRGB_PixelShaderType>(VertexShader,Params,TEXT("FMLAABlend_SRGB_PixelShader")))
		{
			return;
		}
	}
	else
	{
		if (!FlushMlaaShader<FMLAABlend_Linear_PixelShaderType>(VertexShader,Params,TEXT("FMLAABlend_Linear_PixelShader")))
		{
			return;
		}
	}
	RHISetColorWriteMask(CW_RGB);
	(*VertexShader)->SetParameters(Params);

	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	GDisCensusArkPpDraws++;
	GDisCensusArkPpAADraws++;
}

/** DISHONORED(port): 2013 rva 0x50e090 (2012 0x54e7b0) - the reciprocal source size and the view rectangle in it. */
void FFXAAVertexShader::SetParameters(const FArkPpFxAaParameters& iParams)
{
	const FVertexShaderRHIParamRef Shader = GetVertexShader();
	const FLOAT OOSizeX = 1.0f / (FLOAT)iParams.mSizeX;
	const FLOAT OOSizeY = 1.0f / (FLOAT)iParams.mSizeY;
	SetVertexShaderValue(Shader,fxaaQualityRcpFrameParameter,FVector2D(OOSizeX,OOSizeY));
	SetVertexShaderValue(Shader,TexCoordScaleBiasParameter,FVector4(
		(iParams.m_Viewport.Z - iParams.m_Viewport.X) * OOSizeX,
		(iParams.m_Viewport.W - iParams.m_Viewport.Y) * OOSizeY,
		iParams.m_Viewport.X * OOSizeX,
		iParams.m_Viewport.Y * OOSizeY));
}

/** DISHONORED(port): 2013 rva 0x50e170 (2012 0x54e900) - the same pair as the FXAA vertex shader's. */
void FMLAAVertexShader::SetParameters(const FArkPpMlaaParameters& iParams)
{
	const FVertexShaderRHIParamRef Shader = GetVertexShader();
	const FLOAT OOSizeX = 1.0f / (FLOAT)iParams.mSizeX;
	const FLOAT OOSizeY = 1.0f / (FLOAT)iParams.mSizeY;
	SetVertexShaderValue(Shader,InvTextureSizeParameter,FVector2D(OOSizeX,OOSizeY));
	SetVertexShaderValue(Shader,TexCoordScaleBiasParameter,FVector4(
		(iParams.m_Viewport.Z - iParams.m_Viewport.X) * OOSizeX,
		(iParams.m_Viewport.W - iParams.m_Viewport.Y) * OOSizeY,
		iParams.m_Viewport.X * OOSizeX,
		iParams.m_Viewport.Y * OOSizeY));
}

/** DISHONORED(port): 2013 rva 0x524220 (2012 0x565120). */
UBOOL UArkPpNodeAA::IsValid(FArkPpIsValidData& Cache)
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

/** DISHONORED(port): 2013 rva 0x5242c0 (2012 0x5651c0) - an AA node with no type is its input. */
FArkPpNodeProxy* UArkPpNodeAA::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	if (m_Type == EPpAa_None)
	{
		return m_SurfaceTarget ? m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
	}
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeAAProxy(Config,this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpAAShaderTypes()
{
}
