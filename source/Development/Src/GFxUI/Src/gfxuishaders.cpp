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
/*-----------------------------------------------------------------------------
	DISHONORED(port): the 50 GFx render shader types of the cooked global shader cache, and the filter
	half of the shader-interface lookup.

	Arkane draws Scaleform through the engine's own RHI, not through a Scaleform renderer: the three
	shader templates - a vertex shader, a pixel shader and a filter pixel shader - are declared in
	GFxUI/Inc/gfxuirendererimpl.h, which is the file the PDB attributes their bodies to, and are
	instantiated once per GFx blend/texture/filter combination. The cooked cache holds 7 + 17 + 26 of
	them (2013 rva 0xb85120 .. 0xb85d60, sources GFxVertexShader / GFxPixelShader /
	GFxFilterPixelShader, 786 / 1).

	Package BD declared the types so the cooked records load; package CC moved the templates into
	gfxuirendererimpl.h with their parameter setters and added the lookup below, which is the retail
	arrangement: GetUIPixelShaderInterface2_RenderThread (2012 0x5d7360) lives in this unit, and the
	renderer's own GetUIPixelShaderInterface_RenderThread (2012 0x5d77f0) delegates the filter kinds
	to it - which is also what pulls this translation unit, and with it all 50 registrations, into the
	link.
-----------------------------------------------------------------------------*/
#include "GFxUI.h"
#include "EnginePrivate.h"
#include "ShaderManager.h"
#include "GlobalShader.h"
#include "gfxuirendererimpl.h"

// DISHONORED(retail): the cooked type name is the C++ class name with the enumerator spelled out,
// which is what IMPLEMENT_SHADER_TYPE writes, so these need no explicit name. Source file and entry
// point per 0xb85120 ff.
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

// 2012 0x5d7360: the filter half of the pixel-shader enumeration. FS2_* 21, 23 and 26 are gaps in
// retail's own enumeration and have no cooked shader, so they answer NULL.
#define GFXUI_FS_CASE(Kind)     case Kind: { TShaderMapRef<FGFxFilterPixelShader<Kind> > Shader(GetGlobalShaderMap(GRHIShaderPlatform));                  return Shader->GetShaderInterface(); }

FGFxPixelShaderInterface* GetUIPixelShaderInterface2_RenderThread(EGFxPixelShaderType Type)
{
	switch (Type)
	{
		GFXUI_FS_CASE(FS2_FBox2InnerShadow)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowHighlight)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowMul)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowMulHighlight)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowKnockout)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowHighlightKnockout)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowMulKnockout)
		GFXUI_FS_CASE(FS2_FBox2InnerShadowMulHighlightKnockout)
		GFXUI_FS_CASE(FS2_FBox2Shadow)
		GFXUI_FS_CASE(FS2_FBox2ShadowHighlight)
		GFXUI_FS_CASE(FS2_FBox2ShadowMul)
		GFXUI_FS_CASE(FS2_FBox2ShadowMulHighlight)
		GFXUI_FS_CASE(FS2_FBox2ShadowKnockout)
		GFXUI_FS_CASE(FS2_FBox2ShadowHighlightKnockout)
		GFXUI_FS_CASE(FS2_FBox2ShadowMulKnockout)
		GFXUI_FS_CASE(FS2_FBox2ShadowMulHighlightKnockout)
		GFXUI_FS_CASE(FS2_FBox2Shadowonly)
		GFXUI_FS_CASE(FS2_FBox2ShadowonlyHighlight)
		GFXUI_FS_CASE(FS2_FBox2ShadowonlyMul)
		GFXUI_FS_CASE(FS2_FBox2ShadowonlyMulHighlight)
		GFXUI_FS_CASE(FS2_FBox2Blur)
		GFXUI_FS_CASE(FS2_FBox2BlurMul)
		GFXUI_FS_CASE(FS2_FBox1Blur)
		GFXUI_FS_CASE(FS2_FBox1BlurMul)
		GFXUI_FS_CASE(FS2_FCMatrix)
		GFXUI_FS_CASE(FS2_FCMatrixMul)
	default:
		return NULL;
	}
}

#undef GFXUI_FS_CASE

/**
 * DISHONORED(bringup): GFxUI is a static library and FogRendering.cpp takes this function's address
 * (behind DISHONORED_WITH_GFXUI_SHADERS) to force the link. It is no longer the only thing that
 * does - gfxuirenderer.cpp calls GetUIPixelShaderInterface2_RenderThread above - so this can go
 * together with agent BD's link anchor.
 */
void DishonoredLinkGFxShaderTypes()
{
}
