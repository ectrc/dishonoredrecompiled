// GRasterizer: GFx 3.3's anti-aliased scanline polygon rasteriser, and the image helpers the glyph
// cache rasterises into.
//
// This is the piece that makes text visible at all. Retail's GRasterizer is 13 functions in
// GFxFontCompactor.obj's neighbourhood, all ported here:
//
//   GRasterizer::GRasterizer      0xab66e0     MoveTo          0xab6280
//   Clear                         0xab6580     LineTo          0xab6e50
//   ClosePolygon                  0xab6ea0     line            0xab6960
//   horLine                       0xab6750     SortCells       0xab6ce0
//   SweepScanline                 0xab62b0     SetGamma        0xab65c0
//   AddShape                      0xab7190     AddShapeScaled  0xab6ed0
//   cellXLess                     0xab6260
//
// It is Anti-Grain Geometry's `rasterizer_scanline_aa` with poly_base_shift = 8 and aa_shift = 8,
// and every one of those constants is measured rather than assumed:
//   * MoveTo/LineTo multiply by 256.0 before truncating (0xab6280, 0xab6e50), so a coordinate is
//     carried in 1/256 of a pixel: poly_base_shift = 8, poly_base_size = 256, poly_base_mask = 255;
//   * SweepScanline computes `((cover << 9) - area) >> 9` (0xab62b0), i.e. the area normalisation is
//     poly_base_shift*2 + 1 - aa_shift = 9, so aa_shift = 8 and a coverage value is 0..255;
//   * the even-odd fill rule is `filling_rule == 1`, and it folds coverage with
//     `c &= 0x1FF; if (c > 256) c = 512 - c;` (0xab62b0);
//   * SetGamma builds a 256-entry table of pow(i/255, gamma)*255 + 0.5 and drops the table entirely
//     when gamma == 1.0 (0xab65c0).
#ifndef INC_GFXRASTERIZER_H
#define INC_GFXRASTERIZER_H

#include "GFx3.h"
#include "GFxShape.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

// ---------------------------------------------------------------------------------------------
// GImage is declared in GFx3Gen.h at its PDB layout and carries no allocator, because in retail the
// allocation is GMemoryHeap's. These three are our own helpers over that layout.
bool  GFxImageInit(GImage* img, unsigned int width, unsigned int height,
                   GImageBase::ImageFormat format);
void  GFxImageFree(GImage* img);
void  GFxImageClear(GImage* img, unsigned char value);

class GRasterizer
{
public:
    enum FillingRule
    {
        FillNonZero = 0,
        FillEvenOdd = 1
    };

    // A cell is one pixel's accumulated winding contribution: `Cover` is the signed sub-pixel height
    // the edges crossed inside it and `Area` twice the sub-pixel area to the left of them.
    struct Cell
    {
        int X, Y;
        int Cover, Area;
    };

    GRasterizer();                                        // 0xab66e0
    ~GRasterizer();

    void Clear();                                         // 0xab6580
    void SetFillingRule(FillingRule r) { Rule = r; }
    void SetGamma(float gamma);                           // 0xab65c0

    void MoveTo(float x, float y);                        // 0xab6280
    void LineTo(float x, float y);                        // 0xab6e50
    void ClosePolygon();                                  // 0xab6ea0

    // Push every contour of a flattened shape whose fill style matches. AddShape is
    // AddShapeScaled with one scale and no translation (0xab7190). A negative `fillStyle` means
    // "every path that has a style on that side", which is what a glyph - one implicit fill - uses.
    void AddShape(const GCompoundShape& shape, float scale, int fillStyle);            // 0xab7190
    void AddShapeScaled(const GCompoundShape& shape, float sx, float sy,
                        float tx, float ty, int fillStyle);                            // 0xab6ed0

    bool SortCells();                                     // 0xab6ce0
    // Writes `pixBytes` bytes of coverage per covered pixel into `dst`, which is indexed from
    // GetMinX(). 0xab62b0.
    void SweepScanline(unsigned int y, unsigned char* dst, unsigned int pixBytes) const;

    int GetMinX() const { return MinX; }
    int GetMinY() const { return MinY; }
    int GetMaxX() const { return MaxX; }
    int GetMaxY() const { return MaxY; }
    unsigned int GetScanlineCount() const { return SortedY.GetSize(); }
    unsigned int GetCellCount() const { return Cells.GetSize(); }

    // Rasterise straight into an 8-bit alpha image of the shape's own bound. Retail does this inside
    // GFxGlyphRasterCache::rasterizeAndPack (0xa4f5d0); it is factored out here so the harness can
    // dump a glyph bitmap without a texture atlas.
    bool RasterizeToAlpha(GImage* out, int width, int height);

private:
    void line(int x1, int y1, int x2, int y2);             // 0xab6960
    void horLine(int ey, int x1, int fy1, int x2, int fy2);// 0xab6750
    void setCurrCell(int x, int y);
    void addCurrCell();

    FillingRule  Rule;
    float        Gamma;
    GArray<int>  GammaTable;
    GArray<Cell> Cells;
    GArray<Cell*> SortedCells;
    struct ScanlineRange { unsigned int Start, Num; };
    GArray<ScanlineRange> SortedY;

    Cell CurCell;
    int  MinX, MinY, MaxX, MaxY;
    int  StartX, StartY;       // the current polygon's first point, in 1/256 px
    int  CurX, CurY;           // the pen, in 1/256 px
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXRASTERIZER_H
