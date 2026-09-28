// GFx 3.3 glyph cache - rasterise a glyph outline and pack it into a texture atlas.
// See GFxGlyphCache.h for the measured constants and for the list of what is and is not ported.
#include "GFxGlyphCache.h"

#include <math.h>
#include <string.h>

// DISHONORED(port): the tables retail's stackBlur indexes by radius, read out of the retail image at
// VAs 0x11FDFD8 (word) and 0x11FE1D8 (byte). mul[r] / 2^shr[r] is 1/(r+1)^2 to within a rounding step,
// which is the stack blur's normalisation.
static const unsigned short GFxStackBlurMul[32] = {
    512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512,
    454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512 };
static const unsigned char GFxStackBlurShr[32] = {
    9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19 };

// One axis of the stack blur, over a strided run of bytes. AGG's own loop: a ring of 2r+1 samples
// whose front and back halves are summed separately, so each output costs one add and one subtract.
static void GFxStackBlurAxis(unsigned char* base, unsigned int count, unsigned int stride,
                             unsigned int radius, unsigned char* stack)
{
    const unsigned int div = 2 * radius + 1;
    const unsigned int mul = GFxStackBlurMul[radius];
    const unsigned int shr = GFxStackBlurShr[radius];
    unsigned int sum = 0, sumIn = 0, sumOut = 0;

    unsigned char pix = base[0];
    for (unsigned int i = 0; i <= radius; ++i)
    {
        stack[i] = pix;
        sum += (unsigned int)pix * (i + 1);
        sumOut += pix;
    }
    for (unsigned int i = 1; i <= radius; ++i)
    {
        const unsigned int at = i < count ? i : count - 1;
        const unsigned char p = base[at * stride];
        stack[i + radius] = p;
        sum += (unsigned int)p * (radius + 1 - i);
        sumIn += p;
    }

    unsigned int sp = radius;
    unsigned int xp = radius < count - 1 ? radius : count - 1;
    for (unsigned int i = 0; i < count; ++i)
    {
        base[i * stride] = (unsigned char)((sum * mul) >> shr);
        sum -= sumOut;
        unsigned int stackStart = sp + div - radius;
        if (stackStart >= div)
            stackStart -= div;
        sumOut -= stack[stackStart];
        if (xp < count - 1)
            ++xp;
        const unsigned char p = base[xp * stride];
        stack[stackStart] = p;
        sumIn += p;
        sum += sumIn;
        if (++sp >= div)
            sp = 0;
        sumOut += stack[sp];
        sumIn -= stack[sp];
    }
}

void GFxGlyphStackBlur(GImage* img, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                       unsigned int radiusX, unsigned int radiusY)
{
    if (img == 0 || img->pData == 0 || w == 0 || h == 0)
        return;
    if (radiusX > 15) radiusX = 15;
    if (radiusY > 15) radiusY = 15;
    unsigned char stack[64];
    if (radiusX != 0 && w > 1)
    {
        for (unsigned int row = 0; row < h; ++row)
            GFxStackBlurAxis(img->pData + (y + row) * img->Pitch + x, w, 1, radiusX, stack);
    }
    if (radiusY != 0 && h > 1)
    {
        for (unsigned int col = 0; col < w; ++col)
            GFxStackBlurAxis(img->pData + y * img->Pitch + x + col, h, img->Pitch, radiusY, stack);
    }
}

void GFxGlyphStrengthen(GImage* img, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                        float strength, int bias)
{
    // DISHONORED(port): 0xa420c0. A strength of exactly 1 is retail's early-out.
    if (img == 0 || img->pData == 0 || strength == 1.f)
        return;
    for (unsigned int row = 0; row < h; ++row)
    {
        unsigned char* p = img->pData + (y + row) * img->Pitch + x;
        for (unsigned int i = 0; i < w; ++i)
        {
            int v = bias + (int)((float)((double)((int)p[i] - bias) * strength + 0.5));
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            p[i] = (unsigned char)v;
        }
    }
}

bool GFxGlyphParam::operator==(const GFxGlyphParam& o) const
{
    // DISHONORED(port): 0x9bdbe0
    return pFont == o.pFont && GlyphIndex == o.GlyphIndex && FontSize == o.FontSize &&
           Flags == o.Flags && BlurX == o.BlurX && BlurY == o.BlurY && Outline == o.Outline &&
           Strength == o.Strength;
}

// =============================================================================================
// The raster core
// =============================================================================================

bool GFxGlyphRasterize(GFxFontResource* font, unsigned int glyphIndex, float fontSizePx,
                       unsigned int padding, GImage* out, GFxGlyphRasterMetrics* metrics,
                       GRasterizer* raster, GCompoundShape* compound)
{
    // DISHONORED(port): the raster core of GFxGlyphRasterCache::rasterizeAndPack (0xa4f5d0).
    if (metrics)
    {
        metrics->Width = metrics->Height = 0;
        metrics->OriginX = metrics->OriginY = 0.0f;
        metrics->AdvancePx = 0.0f;
        metrics->Contours = metrics->Vertices = 0;
        metrics->CoveredPixels = 0;
        metrics->MaxCoverage = 0;
    }
    if (!font || !out || !raster || !compound)
        return false;

    GPtr<GFxShapeBase> shape = font->GetGlyphShape(glyphIndex, 0);
    if (!shape)
        return false;

    // px per glyph unit. Retail carries the size as 1/16 of a pixel and multiplies by
    // 1/16384 = 1/(1024 * 16); the same number is fontSizePx / 1024.
    const float scale = fontSizePx * (1.0f / 1024.0f);

    // The curve tolerance is retail's literal 10.0. It is in *glyph* units, which is why a 1024-EM
    // outline flattens to a handful of segments per curve rather than hundreds.
    shape->MakeCompoundShape(compound, 10.0f);

    float bl, bt, br, bb;
    compound->PerceiveBounds(&bl, &bt, &br, &bb);
    if (bl > br || bt > bb)
    {
        // A blank glyph (space): no outline at all. Retail packs a zero-size slot for it, so the
        // layout still advances and nothing is drawn.
        if (metrics)
            metrics->AdvancePx = font->GetAdvance(glyphIndex) * scale;
        return true;
    }

    const float pad = (float)padding;
    const int w = (int)ceilf(br * scale - bl * scale) + 2 * (int)padding + 1;
    const int h = (int)ceilf(bb * scale - bt * scale) + 2 * (int)padding + 1;
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096)
        return false;

    const float tx = -bl * scale + pad;
    const float ty = -bt * scale + pad;

    raster->Clear();
    raster->SetGamma(1.0f);
    // Fill style -1 is retail's: a glyph's SHAPE record has one implicit fill and the rasteriser is
    // asked for "every path with a style on either side".
    raster->AddShapeScaled(*compound, scale, scale, tx, ty, -1);

    if (!GFxImageInit(out, (unsigned int)w, (unsigned int)h, GImageBase::Image_A_8))
        return false;
    if (!raster->SortCells())
        return true;

    const int minX = raster->GetMinX();
    const int minY = raster->GetMinY();
    const unsigned int span = (unsigned int)(raster->GetMaxX() - minX + 2);
    unsigned char* row = (unsigned char*)malloc(span);
    if (!row)
        return false;

    unsigned int covered = 0, maxCov = 0;
    for (unsigned int line = 0; line < raster->GetScanlineCount(); ++line)
    {
        const int y = minY + (int)line;
        if (y < 0 || y >= h)
            continue;
        memset(row, 0, span);
        raster->SweepScanline(line, row, 1);
        unsigned char* dst = out->pData + (unsigned int)y * out->Pitch;
        for (unsigned int i = 0; i < span; ++i)
        {
            const int x = minX + (int)i;
            if (x < 0 || x >= w)
                continue;
            if (row[i])
            {
                dst[x] = row[i];
                ++covered;
                if (row[i] > maxCov)
                    maxCov = row[i];
            }
        }
    }
    free(row);

    if (metrics)
    {
        metrics->Width = w;
        metrics->Height = h;
        // The bitmap's top left relative to the pen, y measured upwards - so a glyph whose outline
        // top is -700 glyph units sits (700 * scale) pixels above the baseline.
        metrics->OriginX = bl * scale - pad;
        metrics->OriginY = -(bt * scale - pad);
        metrics->AdvancePx = font->GetAdvance(glyphIndex) * scale;
        metrics->Contours = compound->GetPathCount();
        metrics->Vertices = compound->GetVertexCount();
        metrics->CoveredPixels = covered;
        metrics->MaxCoverage = maxCov;
    }
    return true;
}

// =============================================================================================
// GFxGlyphRasterCache
// =============================================================================================

GFxGlyphRasterCache::GFxGlyphRasterCache()
    : TextureWidth(0), TextureHeight(0), MaxTextures(0), SlotHeight(0), Padding(0),
      Rasterized(0), Misses(0), Empty(0), Failed(0)
{
    // DISHONORED(port): 0xa4f290
    Init(1024, 1024, 4, 48, 1);
}

GFxGlyphRasterCache::~GFxGlyphRasterCache()
{
    // DISHONORED(port): 0xa4e890
    Clear();
}

void GFxGlyphRasterCache::Init(unsigned int textureWidth, unsigned int textureHeight,
                               unsigned int maxTextures, unsigned int slotHeight,
                               unsigned int padding)
{
    // DISHONORED(port): 0xa4e490 - both texture dimensions are rounded up to a power of two (the
    // `--v; while (v) { v >>= 1; ++i; } 1 << i` pair, with a floor of 64) and the texture count is
    // clamped at 32.
    Clear();
    unsigned int wv = textureWidth >= 64 ? textureWidth - 1 : 63;
    unsigned int hv = textureHeight >= 64 ? textureHeight - 1 : 63;
    unsigned int wi = 0, hi = 0;
    for (; wv; wv >>= 1) ++wi;
    for (; hv; hv >>= 1) ++hi;
    TextureWidth = 1u << wi;
    TextureHeight = 1u << hi;
    MaxTextures = maxTextures > 32 ? 32u : maxTextures;
    SlotHeight = slotHeight ? slotHeight : 48u;
    Padding = padding;
}

void GFxGlyphRasterCache::Clear()
{
    // DISHONORED(port): 0xa4e460 -> releaseAllTextures 0xa4d550
    for (unsigned int i = 0; i < Textures.GetSize(); ++i)
    {
        GFxImageFree(Textures[i]);
        delete Textures[i];
    }
    Textures.Clear();
    Bands.Clear();
    for (unsigned int i = 0; i < Glyphs.GetSize(); ++i)
        delete Glyphs[i];
    Glyphs.Clear();
    Rasterized = Misses = Empty = 0;
}

unsigned int GFxGlyphRasterCache::SnapFontSizeToRamp(unsigned int size) const
{
    // DISHONORED(port): snapFontSizeToRamp 0xa4bdd0 / 0x9bdd40 - the size ramp is what keeps a text
    // field animating its scale from filling the atlas with near-identical glyphs. Retail's ramp is a
    // table in the font-cache manager's state; the shape of it (exact below 16, then coarser) is
    // reproduced and the exact table is named in agentCB.md as not measured.
    if (size <= 16)
        return size;
    if (size <= 32)
        return (size + 1) & ~1u;
    if (size <= 64)
        return (size + 3) & ~3u;
    return (size + 7) & ~7u;
}

void GFxGlyphRasterCache::CalcGlyphParam(float scaleLimit, unsigned int fontSize, float blurScale,
                                         float outlineScale, const GFxGlyphParam& in,
                                         GFxGlyphParam* out, unsigned short* outSize16,
                                         unsigned short* outShrink) const
{
    // DISHONORED(port): 0xa4bc60, term for term.
    out->pFont = in.pFont;
    out->GlyphIndex = in.GlyphIndex;
    out->FontSize = (unsigned char)fontSize;
    out->Flags = in.Flags;
    out->Outline = in.Outline;
    out->Strength = in.Strength;

    *outSize16 = (unsigned short)(16 * fontSize);
    const float sizeF = (float)fontSize;
    float ratio = sizeF / scaleLimit;
    if (ratio < 1.0f)
        ratio = 1.0f;

    float bx = (float)in.BlurX * 0.0625f * blurScale * ratio;
    float by = (float)in.BlurY * 0.0625f * blurScale * ratio;
    const float extra = sizeF * 0.0009765625f * outlineScale + by + by;

    // The slot budget: the texture's slot height less two paddings less two.
    const float budget = (float)(SlotHeight - 2 * Padding - 2);
    *outShrink = 256;
    if (budget < extra)
    {
        const float k = budget / extra;
        *outSize16 = (unsigned short)(int)((double)*outSize16 * k);
        *outShrink = (unsigned short)(int)(256.0f * k + 0.5f);
        out->Flags &= ~(unsigned char)GFxGlyphParam::GPF_AutoFit;
        bx = bx * k;
        by = by * k;
    }
    if (bx > 15.75f) bx = 15.75f;
    if (by > 15.75f) by = 15.75f;
    out->BlurX = (unsigned char)(int)(bx * 16.0f + 0.5f);
    const unsigned char byq = (unsigned char)(int)(16.0f * by + 0.5f);
    out->BlurY = byq;

    if ((in.Flags & (GFxGlyphParam::GPF_FauxBold | GFxGlyphParam::GPF_FineBlur)) == 0)
    {
        // The whole-pixel blur snap: when neither faux bold nor the fine-blur bit is set, the blur is
        // rounded to whole pixels and the glyph shrunk by the same ratio, so the blurred glyph still
        // fits the slot it was measured for.
        float q = 0.0625f * (float)byq;
        unsigned int whole = (unsigned int)(q + 0.5f);
        if (!whole)
            whole = byq != 0 ? 1u : 0u;
        if ((float)whole < by && by != 0.0f)
        {
            *outSize16 = (unsigned short)(int)((double)(whole * *outSize16) / by);
            *outShrink = (unsigned short)(int)((double)(whole * *outShrink) / by);
        }
    }
}

bool GFxGlyphRasterCache::allocateSlot(unsigned int w, unsigned int h, unsigned int* outTex,
                                       unsigned int* outX, unsigned int* outY)
{
    // DISHONORED(port): the shelf half of GFxGlyphSlotQueue (findSpaceInSlots 0xa4efa0,
    // allocateNewSlot 0xa4efe0, packGlyph 0xa4eeb0). The LRU extrusion (extrudeOldSlot 0xa4f100) and
    // the neighbour merging (mergeSlotWithNeighbor 0xa4e1d0) are not ported: this cache fills and
    // then refuses, which is honest and which the harness reports as a miss.
    if (w > TextureWidth || h > TextureHeight)
        return false;

    for (unsigned int i = 0; i < Bands.GetSize(); ++i)
    {
        Band& b = Bands[i];
        if (h <= b.Height && b.NextX + w <= TextureWidth)
        {
            *outTex = b.Texture;
            *outX = b.NextX;
            *outY = b.Y;
            b.NextX += w;
            return true;
        }
    }

    // A new band on the last texture, or a new texture.
    const unsigned int bandHeight = h > SlotHeight ? h : SlotHeight;
    unsigned int tex = Textures.GetSize() ? Textures.GetSize() - 1 : 0;
    unsigned int y = 0;
    for (unsigned int i = 0; i < Bands.GetSize(); ++i)
    {
        if (Bands[i].Texture == tex && Bands[i].Y + Bands[i].Height > y)
            y = Bands[i].Y + Bands[i].Height;
    }
    if (!Textures.GetSize() || y + bandHeight > TextureHeight)
    {
        if (Textures.GetSize() >= MaxTextures)
            return false;
        GImage* img = new GImage;
        img->Format = GImageBase::Image_None;
        img->Width = img->Height = img->Pitch = 0;
        img->pData = 0;
        img->DataSize = 0;
        img->MipMapCount = 0;
        if (!GFxImageInit(img, TextureWidth, TextureHeight, GImageBase::Image_A_8))
        {
            delete img;
            return false;
        }
        Textures.PushBack(img);
        tex = Textures.GetSize() - 1;
        y = 0;
    }
    Band nb;
    nb.Texture = tex;
    nb.Y = y;
    nb.Height = bandHeight;
    nb.NextX = w;
    Bands.PushBack(nb);
    *outTex = tex;
    *outX = 0;
    *outY = y;
    return true;
}

void GFxGlyphRasterCache::filterScanline(const unsigned char* src, unsigned char* dst,
                                         unsigned int n) const
{
    // DISHONORED(port): 0xa4bb60 - the 1-2-1 horizontal box the sub-pixel path runs over each
    // scanline before it is packed.
    for (unsigned int i = 0; i < n; ++i)
    {
        const unsigned int a = i ? src[i - 1] : 0u;
        const unsigned int b = src[i];
        const unsigned int c = (i + 1 < n) ? src[i + 1] : 0u;
        dst[i] = (unsigned char)((a + 2 * b + c) >> 2);
    }
}

const GFxGlyphNode* GFxGlyphRasterCache::GetGlyph(const GFxGlyphParam& param)
{
    // DISHONORED(port): 0xa501d0 - the cache probe, then rasterizeAndPack on a miss.
    for (unsigned int i = 0; i < Glyphs.GetSize(); ++i)
    {
        if (Glyphs[i]->Param == param)
            return Glyphs[i];
    }
    ++Misses;
    return rasterizeAndPack(param);
}

const GFxGlyphNode* GFxGlyphRasterCache::rasterizeAndPack(const GFxGlyphParam& param)
{
    // DISHONORED(port): 0xa4f5d0 - rasterise into a scratch image, allocate a slot, blit.
    if (!param.pFont)
        return 0;

    GImage img;
    img.Format = GImageBase::Image_None;
    img.Width = img.Height = img.Pitch = 0;
    img.pData = 0;
    img.DataSize = 0;
    img.MipMapCount = 0;

    // DISHONORED(port): 0xa45a70's box growth. The radius is the blur in whole pixels, rounded, and a
    // blur that rounds to zero but is not zero still gets one pixel - `if (!v72) v72 = v9 != 0`.
    unsigned int radiusX = (unsigned int)((float)param.BlurX * 0.0625f + 0.5f);
    unsigned int radiusY = (unsigned int)((float)param.BlurY * 0.0625f + 0.5f);
    if (radiusX == 0 && param.BlurX != 0) radiusX = 1;
    if (radiusY == 0 && param.BlurY != 0) radiusY = 1;
    // Retail grows the box by radiusX horizontally and radiusY vertically; this reconstruction's
    // raster core takes one padding for both axes, so it takes the larger. Every filter in this cook
    // has BlurX == BlurY, so the two are the same number here.
    const unsigned int extra = radiusX > radiusY ? radiusX : radiusY;

    GFxGlyphRasterMetrics m;
    const bool ok = GFxGlyphRasterize(param.pFont, param.GlyphIndex, (float)param.FontSize,
                                      Padding + extra, &img, &m, &Raster, &Compound);
    if (!ok)
    {
        ++Failed;
        GFxImageFree(&img);
        return 0;
    }

    GFxGlyphNode* node = new GFxGlyphNode;
    node->Param = param;
    node->AdvancePx = m.AdvancePx;
    node->OriginX = m.OriginX;
    node->OriginY = m.OriginY;

    if (!img.pData || m.Width <= 0 || m.Height <= 0)
    {
        // A blank glyph still gets a node, with a zero-size rectangle, so the cache does not
        // rasterise a space over and over.
        ++Empty;
        Glyphs.PushBack(node);
        GFxImageFree(&img);
        return node;
    }

    // DISHONORED(port): the filter block of 0xa45a70, in retail's order - blur, then strengthen. The
    // knock-out pair (makeKnockOutCopy 0xa42650 / knockOut 0xa42720) is not ported: no filter in this
    // cook sets the Knockout bit, and a knock-out with nothing to knock out is a no-op.
    if (radiusX != 0 || radiusY != 0)
    {
        GFxGlyphStackBlur(&img, 0, 0, (unsigned int)m.Width, (unsigned int)m.Height, radiusX,
                          radiusY);
    }
    if (param.Strength != 16)
    {
        const float strength = (float)param.Strength * 0.0625f;
        const int bias = (strength > 1.f && (radiusX != 0 || radiusY != 0)) ? 2 : 0;
        GFxGlyphStrengthen(&img, 0, 0, (unsigned int)m.Width, (unsigned int)m.Height, strength,
                           bias);
    }

    unsigned int tex = 0, x = 0, y = 0;
    if (!allocateSlot((unsigned int)m.Width, (unsigned int)m.Height, &tex, &x, &y))
    {
        ++Failed;
        GFxImageFree(&img);
        delete node;
        return 0;
    }
    node->TextureIndex = tex;
    node->X = x;
    node->Y = y;
    node->Width = (unsigned int)m.Width;
    node->Height = (unsigned int)m.Height;

    GImage* dstTex = Textures[tex];
    for (unsigned int row = 0; row < node->Height; ++row)
    {
        const unsigned char* src = img.pData + row * img.Pitch;
        unsigned char* dst = dstTex->pData + (y + row) * dstTex->Pitch + x;
        memcpy(dst, src, node->Width);
    }
    GFxImageFree(&img);

    ++Rasterized;
    Glyphs.PushBack(node);
    return node;
}
