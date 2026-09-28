// GFx 3.3 glyph cache: the parameters a rasterised glyph is keyed by, the rasterise-and-pack step
// that turns a glyph outline into coverage pixels in a texture atlas, and the cache in front of it.
//
// This is the second half of what makes text visible. The measured constants of the raster step, all
// out of GFxGlyphRasterCache::rasterizeAndPack (2012 0xa4f5d0, 3,058 bytes):
//   * the curve tolerance handed to GFxShapeBase::MakeCompoundShape is **10.0**;
//   * the glyph scale is `sizeIn16thsOfAPixel * 1/16384`, i.e. `1/(1024 * 16)` - the literal in the
//     body is 0.00006103515625 - which is the 1024-EM unit expressed in pixels;
//   * the shape is submitted with fill style **-1**, the "every path that has a style on that side"
//     mode of GRasterizer::AddShapeScaled (0xab6ed0);
//   * the rasteriser's gamma is set to **0.4** for a knocked-out or blurred glyph and 1.0 otherwise.
//
// GFxGlyphParam's field layout is read out of CalcGlyphParam (0xa4bc60), which writes a u8 font size
// at +6, a flag byte at +7, a u8 blur x at +8 and blur y at +9 (both in 1/16 px - the body multiplies
// by 0.0625 to read them back) and a u8 outline at +10, and clamps both blurs at 15.75 px.
//
// What is ported and what is not is stated once: GFxGlyphRasterCache 19 functions, of which
// CalcGlyphParam, Init, GetGlyph, rasterizeAndPack's raster core and filterScanline are here;
// stackBlur (0xa4d5b0), recursiveBlur (0xa4da90), strengthenImage (0xa4bbd0), knockOut (0xa4c230),
// makeKnockOutCopy (0xa4c160) and UpdateTextures (0xa4db30) are not - the first five are the drop
// shadow and glow filters, the last needs a GRenderer, which is package CC's. GFxGlyphSlotQueue's 21
// functions are reduced to the shelf allocation (allocateNewSlot 0xa4efe0, findSpaceInSlots
// 0xa4efa0, packGlyph 0xa4eeb0) without the LRU extrusion and merging, so this cache fills and then
// refuses rather than evicting. GFxGlyphFitter's 9 functions (auto-hinting) are not ported.
#ifndef INC_GFXGLYPHCACHE_H
#define INC_GFXGLYPHCACHE_H

#include "GFx3.h"
#include "GFxFont.h"
#include "GFxRasterizer.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

// ---------------------------------------------------------------------------------------------
struct GFxGlyphParam
{
    enum ParamFlags
    {
        // Read out of CalcGlyphParam (0xa4bc60), which tests `(flags & 0x81) == 0` before it snaps
        // the blur down and clears 0x02 when the glyph had to be shrunk to fit a slot, and out of
        // rasterizeAndPack, which tests 0x20 for the knock-out path and `flags & 0x1E` for "has any
        // filter at all".
        GPF_FauxBold    = 0x01,
        GPF_AutoFit     = 0x02,
        GPF_Blur        = 0x04,
        GPF_Shadow      = 0x08,
        GPF_Strengthen  = 0x10,
        GPF_KnockOut    = 0x20,
        GPF_FineBlur    = 0x80
    };

    GFxFontResource* pFont;
    unsigned int     GlyphIndex;
    unsigned char    FontSize;    // whole pixels, snapped to the size ramp
    unsigned char    Flags;
    unsigned char    BlurX;       // 1/16 px
    unsigned char    BlurY;       // 1/16 px
    unsigned char    Outline;
    // DISHONORED(layout): retail keeps the strength in the byte after BlurY and reads it as
    // `param[10] != 16` before calling strengthenImage (0xa45a70's tail); this reconstruction has
    // Outline in that byte, so the strength is appended instead. The struct is a cache key and
    // nothing serialises it, so the order costs nothing - but it is not retail's.
    unsigned char    Strength;    // 1/16, so 16 is 1.0 and "leave the coverage alone"

    GFxGlyphParam()
        : pFont(0), GlyphIndex(0), FontSize(0), Flags(0), BlurX(0), BlurY(0), Outline(0),
          Strength(16) {}

    bool operator==(const GFxGlyphParam& o) const;                 // 0x9bdbe0
};

// ---------------------------------------------------------------------------------------------
// One rasterised glyph in the atlas.
struct GFxGlyphNode
{
    GFxGlyphParam Param;
    unsigned int  TextureIndex;
    // The glyph's pixels inside the atlas.
    unsigned int  X, Y, Width, Height;
    // Where the bitmap sits relative to the pen position, in pixels: the pen is at (0, baseline), the
    // bitmap's top left at (OriginX, -OriginY).
    float OriginX, OriginY;
    float AdvancePx;

    GFxGlyphNode()
        : TextureIndex(0), X(0), Y(0), Width(0), Height(0), OriginX(0.0f), OriginY(0.0f),
          AdvancePx(0.0f) {}
};

// ---------------------------------------------------------------------------------------------
class GFxGlyphRasterCache
{
public:
    GFxGlyphRasterCache();                                          // 0xa4f290
    ~GFxGlyphRasterCache();                                         // 0xa4e890

    // 0xa4e490. `textureWidth`/`textureHeight` are rounded up to a power of two exactly as retail
    // does (the `for (i = 0; v10; v10 >>= 1) ++i; 1 << i` loops), `maxTextures` is clamped to 32 and
    // `slotHeight` is the band granularity.
    void Init(unsigned int textureWidth, unsigned int textureHeight, unsigned int maxTextures,
              unsigned int slotHeight, unsigned int padding);
    void Clear();                                                   // 0xa4e460

    // 0xa4bc60 - snap the requested size to the ramp and scale the blur down until the glyph fits a
    // slot, writing the glyph scale (in 1/16 px) and a 8.8 fixed shrink factor.
    void CalcGlyphParam(float scaleLimit, unsigned int fontSize, float blurScale, float outlineScale,
                        const GFxGlyphParam& in, GFxGlyphParam* out,
                        unsigned short* outSize16, unsigned short* outShrink) const;
    unsigned int SnapFontSizeToRamp(unsigned int size) const;       // 0xa4be40's caller, 0xa4bdd0

    // 0xa501d0 -> 0xa4f5d0. Returns the cached node, rasterising and packing it on a miss. Null when
    // the atlas is full or the glyph has no outline.
    const GFxGlyphNode* GetGlyph(const GFxGlyphParam& param);

    unsigned int GetTextureCount() const { return Textures.GetSize(); }
    GImage* GetTexture(unsigned int i) const { return i < Textures.GetSize() ? Textures[i] : 0; }
    unsigned int GetGlyphCount() const { return Glyphs.GetSize(); }
    unsigned int GetRasterizedCount() const { return Rasterized; }
    unsigned int GetMissCount() const { return Misses; }
    // DISHONORED(bringup): Misses is the CACHE-miss count - one per distinct glyph, by
    // construction, so it can never be zero while any text is drawn. What says whether a
    // glyph reached the atlas is Rasterized + Empty against it; Failed is the difference.
    unsigned int GetEmptyCount() const { return Empty; }
    unsigned int GetFailedCount() const { return Failed; }

private:
    const GFxGlyphNode* rasterizeAndPack(const GFxGlyphParam& param);  // 0xa4f5d0
    bool allocateSlot(unsigned int w, unsigned int h, unsigned int* outTex,
                      unsigned int* outX, unsigned int* outY);         // 0xa4efe0 / 0xa4efa0
    void filterScanline(const unsigned char* src, unsigned char* dst, unsigned int n) const;

    struct Band { unsigned int Texture, Y, Height, NextX; };

    GArray<GImage*>       Textures;
    GArray<Band>          Bands;
    GArray<GFxGlyphNode*> Glyphs;
    GRasterizer           Raster;
    GCompoundShape        Compound;

    unsigned int TextureWidth, TextureHeight, MaxTextures, SlotHeight, Padding;
    unsigned int Rasterized, Misses, Empty, Failed;
};

// ---------------------------------------------------------------------------------------------
// Rasterise one glyph on its own, at a given pixel size, into an 8-bit alpha image, and report its
// metrics. This is rasterizeAndPack's raster core with the atlas taken out; the harness dumps its
// output, and GFxGlyphRasterCache::rasterizeAndPack calls it.
struct GFxGlyphRasterMetrics
{
    int   Width, Height;      // pixels
    float OriginX, OriginY;   // pixels: the bitmap's offset from the pen, y up
    float AdvancePx;
    unsigned int Contours;    // paths in the flattened shape
    unsigned int Vertices;
    unsigned int CoveredPixels;
    unsigned int MaxCoverage;
};

bool GFxGlyphRasterize(GFxFontResource* font, unsigned int glyphIndex, float fontSizePx,
                       unsigned int padding, GImage* out, GFxGlyphRasterMetrics* metrics,
                       GRasterizer* raster, GCompoundShape* compound);

// DISHONORED(port): 2013 0xa43a50 - Anti-Grain Geometry's stack_blur_gray8 over an 8-bit alpha image,
// with AGG's multiply/shift tables as they are stored in the retail image. Two separable passes, rows
// then columns; a radius of zero on an axis skips that axis.
void GFxGlyphStackBlur(GImage* img, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                       unsigned int radiusX, unsigned int radiusY);
// DISHONORED(port): 2013 0xa420c0 - `bias + (int)((p - bias) * strength + 0.5)`, clamped to a byte.
void GFxGlyphStrengthen(GImage* img, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                        float strength, int bias);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXGLYPHCACHE_H
