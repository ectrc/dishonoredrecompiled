// GFxUI/src/gfxuishaders.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x5d7360  class FGFxPixelShaderInterface * __cdecl GetUIPixelShaderInterface2_RenderThread(enum EGFxPixelShaderType)
//   0xba1900  _dynamic_initializer_for__FGFxFilterPixelShader_1_::StaticType__
//   0xba1940  _dynamic_initializer_for__FGFxFilterPixelShader_2_::StaticType__
//   0xba1980  _dynamic_initializer_for__FGFxFilterPixelShader_3_::StaticType__
//   0xba19c0  _dynamic_initializer_for__FGFxFilterPixelShader_4_::StaticType__
//   0xba1a00  _dynamic_initializer_for__FGFxFilterPixelShader_5_::StaticType__
//   0xba1a40  _dynamic_initializer_for__FGFxFilterPixelShader_6_::StaticType__
//   0xba1a80  _dynamic_initializer_for__FGFxFilterPixelShader_7_::StaticType__
//   0xba1ac0  _dynamic_initializer_for__FGFxFilterPixelShader_8_::StaticType__
//   0xba1b00  _dynamic_initializer_for__FGFxFilterPixelShader_9_::StaticType__
//   0xba1b40  _dynamic_initializer_for__FGFxFilterPixelShader_10_::StaticType__
//   0xba1b80  _dynamic_initializer_for__FGFxFilterPixelShader_11_::StaticType__
//   0xba1bc0  _dynamic_initializer_for__FGFxFilterPixelShader_12_::StaticType__
//   0xba1c00  _dynamic_initializer_for__FGFxFilterPixelShader_13_::StaticType__
//   0xba1c40  _dynamic_initializer_for__FGFxFilterPixelShader_14_::StaticType__
//   0xba1c80  _dynamic_initializer_for__FGFxFilterPixelShader_15_::StaticType__
//   0xba1cc0  _dynamic_initializer_for__FGFxFilterPixelShader_16_::StaticType__
//   0xba1d00  _dynamic_initializer_for__FGFxFilterPixelShader_17_::StaticType__
//   0xba1d40  _dynamic_initializer_for__FGFxFilterPixelShader_18_::StaticType__
//   0xba1d80  _dynamic_initializer_for__FGFxFilterPixelShader_19_::StaticType__
//   0xba1dc0  _dynamic_initializer_for__FGFxFilterPixelShader_20_::StaticType__
//   0xba1e00  _dynamic_initializer_for__FGFxFilterPixelShader_22_::StaticType__
//   0xba1e40  _dynamic_initializer_for__FGFxFilterPixelShader_24_::StaticType__
//   0xba1e80  _dynamic_initializer_for__FGFxFilterPixelShader_25_::StaticType__
//   0xba1ec0  _dynamic_initializer_for__FGFxFilterPixelShader_27_::StaticType__
//   0xba1f00  _dynamic_initializer_for__FGFxFilterPixelShader_28_::StaticType__
//   0xba1f40  _dynamic_initializer_for__FGFxFilterPixelShader_29_::StaticType__

#include "GFxUI.h"
#include "EnginePrivate.h"
#include "ShaderManager.h"
#include "GlobalShader.h"

/*-----------------------------------------------------------------------------
	DISHONORED(port): the 50 GFx render shader types of the cooked global shader cache.

	Arkane draws Scaleform through the engine's own RHI, not through a Scaleform renderer: gfxuirenderer.cpp and
	gfxuishaders.cpp declare three shader templates - a vertex shader, a pixel shader and a filter pixel shader - and
	instantiate them once per GFx blend/texture/filter combination. The cooked cache holds 7 + 17 + 26 of them
	(2013 rva 0xb85120 .. 0xb85d60, sources GFxVertexShader / GFxPixelShader / GFxFilterPixelShader, 786 / 1).

	Only the types are declared here, with the parameter layout of their cooked records: the GFx runtime itself is a
	separate multi-wave project (packages BB / BC / BE, gfx_decision.md). Without the declarations every one of those
	50 records counts as an undeclared type and its shader is dropped on load.
-----------------------------------------------------------------------------*/

/** DISHONORED(layout): the GFx vertex shader kinds, numbered as the retail type initializers order them (0xb85be0 ff.). */
enum EGFxVertexShaderType
{
	GFx_VS_Strip					= 1,
	GFx_VS_Glyph					= 2,
	GFx_VS_XY16iC32					= 3,
	GFx_VS_XY16iCF32				= 4,
	GFx_VS_XY16iCF32_NoTex			= 5,
	GFx_VS_XY16iCF32_NoTexNoAlpha	= 6,
	GFx_VS_XY16iCF32_T2				= 7,
};

/** DISHONORED(layout): the GFx pixel shader kinds (0xb857a0 ff.; the distance-field text shader is 50). */
enum EGFxPixelShaderType
{
	GFx_PS_SolidColor						= 30,
	GFx_PS_CxformTexture					= 31,
	GFx_PS_CxformTextureMultiply			= 32,
	GFx_PS_TextTexture						= 33,
	GFx_PS_TextTextureColor					= 34,
	GFx_PS_TextTextureColorMultiply			= 35,
	GFx_PS_TextTextureSRGB					= 36,
	GFx_PS_TextTextureSRGBMultiply			= 37,
	GFx_PS_CxformGouraud					= 38,
	GFx_PS_CxformGouraudNoAddAlpha			= 39,
	GFx_PS_CxformGouraudTexture				= 40,
	GFx_PS_Cxform2Texture					= 41,
	GFx_PS_CxformGouraudMultiply			= 42,
	GFx_PS_CxformGouraudMultiplyNoAddAlpha	= 43,
	GFx_PS_CxformGouraudMultiplyTexture		= 44,
	GFx_PS_CxformMultiply2Texture			= 45,
	GFx_PS_TextTextureDFA					= 50,
};

/** DISHONORED(layout): the GFx filter kinds (0xb85120 ff.; 21, 23 and 26 have no cooked shader and no initializer). */
enum EGFxFilterShaderType
{
	FS2_FBox2InnerShadow						= 1,
	FS2_FBox2InnerShadowHighlight				= 2,
	FS2_FBox2InnerShadowMul						= 3,
	FS2_FBox2InnerShadowMulHighlight			= 4,
	FS2_FBox2InnerShadowKnockout				= 5,
	FS2_FBox2InnerShadowHighlightKnockout		= 6,
	FS2_FBox2InnerShadowMulKnockout				= 7,
	FS2_FBox2InnerShadowMulHighlightKnockout	= 8,
	FS2_FBox2Shadow								= 9,
	FS2_FBox2ShadowHighlight					= 10,
	FS2_FBox2ShadowMul							= 11,
	FS2_FBox2ShadowMulHighlight					= 12,
	FS2_FBox2ShadowKnockout						= 13,
	FS2_FBox2ShadowHighlightKnockout			= 14,
	FS2_FBox2ShadowMulKnockout					= 15,
	FS2_FBox2ShadowMulHighlightKnockout			= 16,
	FS2_FBox2Shadowonly							= 17,
	FS2_FBox2ShadowonlyHighlight				= 18,
	FS2_FBox2ShadowonlyMul						= 19,
	FS2_FBox2ShadowonlyMulHighlight				= 20,
	FS2_FBox2Blur								= 22,
	FS2_FBox2BlurMul							= 24,
	FS2_FBox1Blur								= 25,
	FS2_FBox1BlurMul							= 27,
	FS2_FCMatrix								= 28,
	FS2_FCMatrixMul								= 29,
};

/**
 * DISHONORED(layout): three parameters - the 2D transform and two texture matrices (2013 rva 0x57cf00 Serialize,
 * 0x57ce70 SetParameterTransform). Nine history words in every cooked FGFxVertexShader record.
 */
template<EGFxVertexShaderType ShaderType>
class FGFxVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGFxVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	FGFxVertexShader() {}

	FGFxVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		TransformParameter.Bind(Initializer.ParameterMap,TEXT("Transform"),TRUE);
		TextureMatrixParams[0].Bind(Initializer.ParameterMap,TEXT("TextureMatrix"),TRUE);
		TextureMatrixParams[1].Bind(Initializer.ParameterMap,TEXT("TextureMatrix2"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TransformParameter;
		Ar << TextureMatrixParams[0];
		Ar << TextureMatrixParams[1];
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter TransformParameter;
	FShaderParameter TextureMatrixParams[2];
};

/**
 * DISHONORED(layout): four textures and twelve constants (2013 rva 0x57cc80 Serialize): the colour transform, then
 * the distance-field text constants (width, shadow width/offset/enable/colour, glow size/enable/colour).
 */
template<EGFxPixelShaderType ShaderType>
class FGFxPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGFxPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	FGFxPixelShader() {}

	FGFxPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		for (INT TextureIndex = 0; TextureIndex < 4; TextureIndex++)
		{
			TextureParams[TextureIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Texture%u"),TextureIndex),TRUE);
		}
		ConstantColorParameter.Bind(Initializer.ParameterMap,TEXT("ConstantColor"),TRUE);
		ColorScaleParameter.Bind(Initializer.ParameterMap,TEXT("ColorScale"),TRUE);
		ColorBiasParameter.Bind(Initializer.ParameterMap,TEXT("ColorBias"),TRUE);
		InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"),TRUE);
		DFWidthParameter.Bind(Initializer.ParameterMap,TEXT("DFWidth"),TRUE);
		DFShadowWidthParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowWidth"),TRUE);
		DFShadowOffsetParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowOffset"),TRUE);
		DFShadowEnableParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowEnable"),TRUE);
		DFShadowColorParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowColor"),TRUE);
		DFGlowSizeParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowSize"),TRUE);
		DFGlowEnableParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowEnable"),TRUE);
		DFGlowColorParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowColor"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		for (INT TextureIndex = 0; TextureIndex < 4; TextureIndex++)
		{
			Ar << TextureParams[TextureIndex];
		}
		Ar << ConstantColorParameter;
		Ar << ColorScaleParameter;
		Ar << ColorBiasParameter;
		Ar << InverseGammaParameter;
		Ar << DFWidthParameter;
		Ar << DFShadowWidthParameter;
		Ar << DFShadowOffsetParameter;
		Ar << DFShadowEnableParameter;
		Ar << DFShadowColorParameter;
		Ar << DFGlowSizeParameter;
		Ar << DFGlowEnableParameter;
		Ar << DFGlowColorParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter TextureParams[4];
	FShaderParameter ConstantColorParameter;
	FShaderParameter ColorScaleParameter;
	FShaderParameter ColorBiasParameter;
	FShaderParameter InverseGammaParameter;
	FShaderParameter DFWidthParameter;
	FShaderParameter DFShadowWidthParameter;
	FShaderParameter DFShadowOffsetParameter;
	FShaderParameter DFShadowEnableParameter;
	FShaderParameter DFShadowColorParameter;
	FShaderParameter DFGlowSizeParameter;
	FShaderParameter DFGlowEnableParameter;
	FShaderParameter DFGlowColorParameter;
};

/**
 * DISHONORED(layout): twelve parameters (2013 rva 0x57c920 Serialize), written interleaved: texture and texture
 * scale for each of the two sources, then colour transform and shadow colour for each, then the inverse gamma, the
 * colour matrix, the blur size and the shadow offset. The highlight parameter is bound but never serialized.
 */
template<EGFxFilterShaderType FilterType>
class FGFxFilterPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGFxFilterPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	FGFxFilterPixelShader() {}

	FGFxFilterPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
		{
			TextureParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Texture%u"),SourceIndex),TRUE);
			TexScaleParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("TexScale%u"),SourceIndex),TRUE);
			CxformParameters[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Cxform%u"),SourceIndex),TRUE);
			ShadowColorParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("ShadowColor%u"),SourceIndex),TRUE);
		}
		InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"),TRUE);
		ColorMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ColorMatrix"),TRUE);
		BlurSizeParameter.Bind(Initializer.ParameterMap,TEXT("BlurSize"),TRUE);
		ShadowOffsetParameter.Bind(Initializer.ParameterMap,TEXT("ShadowOffset"),TRUE);
		HighlightParameter.Bind(Initializer.ParameterMap,TEXT("Highlight"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
		{
			Ar << TextureParams[SourceIndex];
			Ar << TexScaleParams[SourceIndex];
		}
		for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
		{
			Ar << CxformParameters[SourceIndex];
			Ar << ShadowColorParams[SourceIndex];
		}
		Ar << InverseGammaParameter;
		Ar << ColorMatrixParameter;
		Ar << BlurSizeParameter;
		Ar << ShadowOffsetParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter TextureParams[2];
	FShaderParameter TexScaleParams[2];
	FShaderParameter InverseGammaParameter;
	FShaderParameter CxformParameters[2];
	FShaderParameter ColorMatrixParameter;
	FShaderParameter BlurSizeParameter;
	FShaderParameter ShadowColorParams[2];
	FShaderParameter ShadowOffsetParameter;
	FShaderParameter HighlightParameter;
};

// DISHONORED(retail): the cooked type name is the C++ class name with the enumerator spelled out, which is what
// IMPLEMENT_SHADER_TYPE writes, so these need no explicit name. Source file and entry point per 0xb85120 ff.
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_Strip>,TEXT("GFxVertexShader"),TEXT("MainStrip"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_Glyph>,TEXT("GFxVertexShader"),TEXT("MainGlyph"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_XY16iC32>,TEXT("GFxVertexShader"),TEXT("MainStripXY16iC32"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_XY16iCF32>,TEXT("GFxVertexShader"),TEXT("MainStripXY16iCF32"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_XY16iCF32_NoTex>,TEXT("GFxVertexShader"),TEXT("MainStripXY16iCF32_NoTex"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_XY16iCF32_NoTexNoAlpha>,TEXT("GFxVertexShader"),TEXT("MainStripXY16iC32_NoTexNoAlpha"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxVertexShader<GFx_VS_XY16iCF32_T2>,TEXT("GFxVertexShader"),TEXT("MainStripXY16iCF32_T2"),SF_Vertex,786,1);

IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_SolidColor>,TEXT("GFxPixelShader"),TEXT("Main_SolidColor"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformTexture>,TEXT("GFxPixelShader"),TEXT("Main_CxformTexture"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformTextureMultiply>,TEXT("GFxPixelShader"),TEXT("Main_CxformTextureMultiply"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTexture>,TEXT("GFxPixelShader"),TEXT("Main_Glyph"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTextureColor>,TEXT("GFxPixelShader"),TEXT("Main_GlyphColor"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTextureColorMultiply>,TEXT("GFxPixelShader"),TEXT("Main_GlyphColorMultiply"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTextureSRGB>,TEXT("GFxPixelShader"),TEXT("Main_GlyphSRGB"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTextureSRGBMultiply>,TEXT("GFxPixelShader"),TEXT("Main_GlyphSRGBMultiply"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_TextTextureDFA>,TEXT("GFxPixelShader"),TEXT("Main_GlyphDFA"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraud>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraud"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraudNoAddAlpha>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraud_NoAddAlpha"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraudTexture>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraudTexture"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_Cxform2Texture>,TEXT("GFxPixelShader"),TEXT("Main_Cxform2Texture"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraudMultiply>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraudMultiply"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraudMultiplyNoAddAlpha>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraudMultiply_NoAddAlpha"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformGouraudMultiplyTexture>,TEXT("GFxPixelShader"),TEXT("Main_CxformGouraudMultiplyTexture"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxPixelShader<GFx_PS_CxformMultiply2Texture>,TEXT("GFxPixelShader"),TEXT("Main_CxformMultiply2Texture"),SF_Pixel,786,1);

IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadow>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadow"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowMul>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowMul"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowMulHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowMulHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowHighlightKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowHighlightKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowMulKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowMulKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2InnerShadowMulHighlightKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2InnerShadowMulHighlightKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2Shadow>,TEXT("GFxFilterPixelShader"),TEXT("FBox2Shadow"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowMul>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowMul"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowMulHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowMulHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowHighlightKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowHighlightKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowMulKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowMulKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowMulHighlightKnockout>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowMulHighlightKnockout"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2Shadowonly>,TEXT("GFxFilterPixelShader"),TEXT("FBox2Shadowonly"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowonlyHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowonlyHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowonlyMul>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowonlyMul"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2ShadowonlyMulHighlight>,TEXT("GFxFilterPixelShader"),TEXT("FBox2ShadowonlyMulHighlight"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2Blur>,TEXT("GFxFilterPixelShader"),TEXT("FBox2Blur"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox2BlurMul>,TEXT("GFxFilterPixelShader"),TEXT("FBox2BlurMul"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox1Blur>,TEXT("GFxFilterPixelShader"),TEXT("FBox1Blur"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FBox1BlurMul>,TEXT("GFxFilterPixelShader"),TEXT("FBox1BlurMul"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FCMatrix>,TEXT("GFxFilterPixelShader"),TEXT("FCMatrix"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE(template<>,FGFxFilterPixelShader<FS2_FCMatrixMul>,TEXT("GFxFilterPixelShader"),TEXT("FCMatrixMul"),SF_Pixel,786,1);

/**
 * DISHONORED(bringup): GFxUI is a static library and nothing in it references this unit, so the linker would drop it
 * together with the 50 registrations above. FogRendering.cpp takes this function's address (behind
 * DISHONORED_WITH_GFXUI_SHADERS) to force the link; drop it when the GFx renderer seam lands (package BB) and the
 * shaders are actually bound.
 */
void DishonoredLinkGFxShaderTypes()
{
}
