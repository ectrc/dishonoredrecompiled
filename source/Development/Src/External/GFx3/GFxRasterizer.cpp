// GRasterizer - the anti-aliased scanline rasteriser. See GFxRasterizer.h for the measured
// constants; every function below carries its 2012 rva.
#include "GFxRasterizer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// The sub-pixel grid, measured out of MoveTo (0xab6280) and SweepScanline (0xab62b0).
enum
{
    GFxPolyBaseShift = 8,
    GFxPolyBaseSize  = 1 << GFxPolyBaseShift,
    GFxPolyBaseMask  = GFxPolyBaseSize - 1,
    GFxAAShift       = 8,
    GFxAANum         = 1 << GFxAAShift,
    GFxAAMask        = GFxAANum - 1,
    GFxAA2Num        = GFxAANum * 2,
    GFxAA2Mask       = GFxAA2Num - 1
};

// ---------------------------------------------------------------------------------------------
// GImage helpers over GFx3Gen.h's PDB layout.

static unsigned int GFxImageBytesPerPixel(GImageBase::ImageFormat f)
{
    switch (f)
    {
    case GImageBase::Image_ARGB_8888: return 4;
    case GImageBase::Image_RGB_888:   return 3;
    case GImageBase::Image_L_8:
    case GImageBase::Image_A_8:
    case GImageBase::Image_P_8:       return 1;
    default:                          return 0;
    }
}

bool GFxImageInit(GImage* img, unsigned int width, unsigned int height,
                  GImageBase::ImageFormat format)
{
    const unsigned int bpp = GFxImageBytesPerPixel(format);
    if (!img || !bpp || !width || !height)
        return false;
    GFxImageFree(img);
    img->Format = format;
    img->Width = width;
    img->Height = height;
    img->Pitch = width * bpp;
    img->DataSize = img->Pitch * height;
    img->MipMapCount = 1;
    img->pData = (unsigned char*)malloc(img->DataSize);
    if (!img->pData)
    {
        img->DataSize = 0;
        return false;
    }
    memset(img->pData, 0, img->DataSize);
    return true;
}

void GFxImageFree(GImage* img)
{
    if (img && img->pData)
    {
        free(img->pData);
        img->pData = 0;
        img->DataSize = 0;
    }
}

void GFxImageClear(GImage* img, unsigned char value)
{
    if (img && img->pData)
        memset(img->pData, value, img->DataSize);
}

// ---------------------------------------------------------------------------------------------

GRasterizer::GRasterizer()
{
    // DISHONORED(port): 0xab66e0 - gamma 1.0 with no table, the bound inverted and the cell empty.
    Rule = FillNonZero;
    Gamma = 1.0f;
    Clear();
}

GRasterizer::~GRasterizer()
{
}

void GRasterizer::Clear()
{
    // DISHONORED(port): 0xab6580 - note MaxX/MaxY come back as -0x7FFFFFFF, not INT_MIN, exactly as
    // the constructor sets them.
    Cells.Clear();
    SortedCells.Clear();
    SortedY.Clear();
    CurCell.X = 0x7FFFFFFF;
    CurCell.Y = 0x7FFFFFFF;
    CurCell.Cover = 0;
    CurCell.Area = 0;
    MinX = 0x7FFFFFFF;
    MinY = 0x7FFFFFFF;
    MaxX = -0x7FFFFFFF;
    MaxY = -0x7FFFFFFF;
    StartX = StartY = 0;
    CurX = CurY = 0;
}

void GRasterizer::SetGamma(float gamma)
{
    // DISHONORED(port): 0xab65c0
    Gamma = gamma;
    if (gamma == 1.0f)
    {
        GammaTable.Clear();
        return;
    }
    GammaTable.Resize(256);
    for (int i = 0; i < 256; ++i)
        GammaTable[(unsigned int)i] = (int)(pow((double)i / 255.0, (double)gamma) * 255.0 + 0.5);
}

void GRasterizer::addCurrCell()
{
    if (CurCell.Area | CurCell.Cover)
        Cells.PushBack(CurCell);
}

void GRasterizer::setCurrCell(int x, int y)
{
    // The fused compare of the two words is what the decompile shows as
    // `if ((this[16] - ey) | (this[15] - ex))`.
    if ((CurCell.X - x) | (CurCell.Y - y))
    {
        addCurrCell();
        CurCell.X = x;
        CurCell.Y = y;
        CurCell.Cover = 0;
        CurCell.Area = 0;
    }
}

void GRasterizer::horLine(int ey, int x1, int fy1, int x2, int fy2)
{
    // DISHONORED(port): 0xab6750
    int ex1 = x1 >> GFxPolyBaseShift;
    int ex2 = x2 >> GFxPolyBaseShift;
    const int fx1 = x1 & GFxPolyBaseMask;
    const int fx2 = x2 & GFxPolyBaseMask;

    if (fy1 == fy2)
    {
        setCurrCell(ex2, ey);
        return;
    }

    const int dyTotal = fy2 - fy1;
    if (ex1 == ex2)
    {
        CurCell.Cover += dyTotal;
        CurCell.Area += (fx1 + fx2) * dyTotal;
        return;
    }

    int p = (GFxPolyBaseSize - fx1) * dyTotal;
    int first = GFxPolyBaseSize;
    int incr = 1;
    int dx = x2 - x1;
    if (dx < 0)
    {
        p = fx1 * dyTotal;
        first = 0;
        incr = -1;
        dx = -dx;
    }

    int delta = p / dx;
    int mod = p % dx;
    if (mod < 0)
    {
        --delta;
        mod += dx;
    }

    CurCell.Cover += delta;
    CurCell.Area += (fx1 + first) * delta;

    ex1 += incr;
    setCurrCell(ex1, ey);
    int y1 = fy1 + delta;

    if (ex1 != ex2)
    {
        p = GFxPolyBaseSize * (fy2 - fy1 + delta - y1);
        int lift = p / dx;
        int rem = p % dx;
        if (rem < 0)
        {
            --lift;
            rem += dx;
        }
        mod -= dx;
        while (ex1 != ex2)
        {
            delta = lift;
            mod += rem;
            if (mod >= 0)
            {
                mod -= dx;
                ++delta;
            }
            CurCell.Cover += delta;
            CurCell.Area += GFxPolyBaseSize * delta;
            y1 += delta;
            ex1 += incr;
            setCurrCell(ex1, ey);
        }
    }

    delta = fy2 - y1;
    CurCell.Cover += delta;
    CurCell.Area += (fx2 + GFxPolyBaseSize - first) * delta;
}

void GRasterizer::line(int x1, int y1, int x2, int y2)
{
    // DISHONORED(port): 0xab6960
    const int dx = x2 - x1;
    int dy = y2 - y1;
    const int ex1 = x1 >> GFxPolyBaseShift;
    const int ex2 = x2 >> GFxPolyBaseShift;
    int ey1 = y1 >> GFxPolyBaseShift;
    const int ey2 = y2 >> GFxPolyBaseShift;
    const int fy1 = y1 & GFxPolyBaseMask;
    const int fy2 = y2 & GFxPolyBaseMask;

    if (ex1 < MinX) MinX = ex1;
    if (ex1 > MaxX) MaxX = ex1;
    if (ey1 < MinY) MinY = ey1;
    if (ey1 > MaxY) MaxY = ey1;
    if (ex2 < MinX) MinX = ex2;
    if (ex2 > MaxX) MaxX = ex2;
    if (ey2 < MinY) MinY = ey2;
    if (ey2 > MaxY) MaxY = ey2;

    setCurrCell(ex1, ey1);

    if (ey1 == ey2)
    {
        horLine(ey1, x1, fy1, x2, fy2);
        return;
    }

    int incr = 1;
    int first = GFxPolyBaseSize;

    if (dx == 0)
    {
        // The vertical special case: two_fx is twice the sub-pixel x, so the area contribution of a
        // full sub-pixel step is two_fx * delta.
        const int ex = x1 >> GFxPolyBaseShift;
        const int twoFx = 2 * (x1 - (ex << GFxPolyBaseShift));
        if (dy < 0)
        {
            first = 0;
            incr = -1;
        }
        int delta = first - fy1;
        CurCell.Cover += delta;
        CurCell.Area += twoFx * delta;

        ey1 += incr;
        setCurrCell(ex, ey1);

        delta = first + first - GFxPolyBaseSize;
        const int area = twoFx * delta;
        while (ey1 != ey2)
        {
            CurCell.Cover = delta;
            CurCell.Area = area;
            ey1 += incr;
            setCurrCell(ex, ey1);
        }
        delta = fy2 - GFxPolyBaseSize + first;
        CurCell.Cover += delta;
        CurCell.Area += twoFx * delta;
        return;
    }

    int p = (GFxPolyBaseSize - fy1) * dx;
    if (dy < 0)
    {
        p = fy1 * dx;
        first = 0;
        incr = -1;
        dy = -dy;
    }

    int delta = p / dy;
    int mod = p % dy;
    if (mod < 0)
    {
        --delta;
        mod += dy;
    }

    int xFrom = x1 + delta;
    horLine(ey1, x1, fy1, xFrom, first);

    ey1 += incr;
    setCurrCell(xFrom >> GFxPolyBaseShift, ey1);

    if (ey1 != ey2)
    {
        p = GFxPolyBaseSize * dx;
        int lift = p / dy;
        int rem = p % dy;
        if (rem < 0)
        {
            --lift;
            rem += dy;
        }
        mod -= dy;
        while (ey1 != ey2)
        {
            delta = lift;
            mod += rem;
            if (mod >= 0)
            {
                mod -= dy;
                ++delta;
            }
            const int xTo = xFrom + delta;
            horLine(ey1, xFrom, GFxPolyBaseSize - first, xTo, first);
            xFrom = xTo;
            ey1 += incr;
            setCurrCell(xFrom >> GFxPolyBaseShift, ey1);
        }
    }
    horLine(ey1, xFrom, GFxPolyBaseSize - first, x2, fy2);
}

void GRasterizer::MoveTo(float x, float y)
{
    // DISHONORED(port): 0xab6280
    CurX = StartX = (int)(x * 256.0f);
    CurY = StartY = (int)(256.0f * y);
}

void GRasterizer::LineTo(float x, float y)
{
    // DISHONORED(port): 0xab6e50
    const int nx = (int)(x * 256.0f);
    const int ny = (int)(256.0f * y);
    line(CurX, CurY, nx, ny);
    CurX = nx;
    CurY = ny;
}

void GRasterizer::ClosePolygon()
{
    // DISHONORED(port): 0xab6ea0
    line(CurX, CurY, StartX, StartY);
    CurX = StartX;
    CurY = StartY;
}

void GRasterizer::AddShape(const GCompoundShape& shape, float scale, int fillStyle)
{
    // DISHONORED(port): 0xab7190
    AddShapeScaled(shape, scale, scale, 0.0f, 0.0f, fillStyle);
}

void GRasterizer::AddShapeScaled(const GCompoundShape& shape, float sx, float sy,
                                 float tx, float ty, int fillStyle)
{
    // DISHONORED(port): 0xab6ed0. A path is emitted *forwards* when its left style matches and
    // *backwards* when its right style matches, so a contour that fills on both sides cancels and is
    // skipped outright - which is the first test in the retail body.
    for (unsigned int pi = 0; pi < shape.GetPathCount(); ++pi)
    {
        const GCompoundShape::SPath& path = shape.GetPath(pi);
        if (path.LeftStyle == path.RightStyle)
            continue;
        if (path.VertexCount == 0)
            continue;

        const bool forward = (fillStyle < 0) ? (path.LeftStyle >= 0) : (path.LeftStyle == fillStyle);
        const bool backward = (fillStyle < 0) ? (path.RightStyle >= 0) : (path.RightStyle == fillStyle);

        if (forward)
        {
            const GPoint<float>& v0 = shape.GetVertex(path.VertexStart);
            MoveTo(v0.x * sx + tx, v0.y * sy + ty);
            for (unsigned int i = 1; i < path.VertexCount; ++i)
            {
                const GPoint<float>& v = shape.GetVertex(path.VertexStart + i);
                LineTo(v.x * sx + tx, v.y * sy + ty);
            }
        }
        if (backward)
        {
            const GPoint<float>& vn = shape.GetVertex(path.VertexStart + path.VertexCount - 1);
            MoveTo(vn.x * sx + tx, vn.y * sy + ty);
            for (unsigned int i = path.VertexCount; i > 1; --i)
            {
                const GPoint<float>& v = shape.GetVertex(path.VertexStart + i - 2);
                LineTo(v.x * sx + tx, v.y * sy + ty);
            }
        }
    }
}

static int GFxCellXLess(const void* a, const void* b)
{
    // DISHONORED(port): 0xab6260 - the comparator is on X only; the bucketing by Y is the caller's.
    const GRasterizer::Cell* ca = *(const GRasterizer::Cell* const*)a;
    const GRasterizer::Cell* cb = *(const GRasterizer::Cell* const*)b;
    return ca->X - cb->X;
}

bool GRasterizer::SortCells()
{
    // DISHONORED(port): 0xab6ce0 - flush the open cell, bucket every cell by scanline, sort each
    // bucket by x. The two parallel arrays the sweep reads are the pointer array and the
    // (start, count) table per scanline.
    addCurrCell();
    CurCell.X = 0x7FFFFFFF;
    CurCell.Y = 0x7FFFFFFF;
    CurCell.Cover = 0;
    CurCell.Area = 0;

    SortedCells.Clear();
    SortedY.Clear();
    if (Cells.GetSize() == 0 || MinY > MaxY)
        return false;

    const unsigned int lines = (unsigned int)(MaxY - MinY + 1);
    SortedY.Resize(lines);
    for (unsigned int i = 0; i < lines; ++i)
    {
        SortedY[i].Start = 0;
        SortedY[i].Num = 0;
    }
    for (unsigned int i = 0; i < Cells.GetSize(); ++i)
        ++SortedY[(unsigned int)(Cells[i].Y - MinY)].Num;

    unsigned int start = 0;
    for (unsigned int i = 0; i < lines; ++i)
    {
        SortedY[i].Start = start;
        start += SortedY[i].Num;
        SortedY[i].Num = 0;
    }

    SortedCells.Resize(Cells.GetSize());
    for (unsigned int i = 0; i < Cells.GetSize(); ++i)
    {
        ScanlineRange& r = SortedY[(unsigned int)(Cells[i].Y - MinY)];
        SortedCells[r.Start + r.Num] = &Cells[i];
        ++r.Num;
    }
    for (unsigned int i = 0; i < lines; ++i)
    {
        if (SortedY[i].Num > 1)
            qsort(&SortedCells[SortedY[i].Start], SortedY[i].Num, sizeof(Cell*), GFxCellXLess);
    }
    return true;
}

void GRasterizer::SweepScanline(unsigned int y, unsigned char* dst, unsigned int pixBytes) const
{
    // DISHONORED(port): 0xab62b0
    if (y >= SortedY.GetSize())
        return;
    const ScanlineRange& range = SortedY[y];
    unsigned int num = range.Num;
    if (!num)
        return;

    const Cell* const* cells = &SortedCells[range.Start];
    int cover = 0;

    while (num)
    {
        const Cell* cell = *cells;
        int x = cell->X;
        int area = cell->Area;
        cover += cell->Cover;
        ++cells;
        --num;

        // Merge every cell at the same x.
        while (num && (*cells)->X == x)
        {
            area += (*cells)->Area;
            cover += (*cells)->Cover;
            ++cells;
            --num;
        }

        if (area)
        {
            int alpha = ((cover << (GFxPolyBaseShift + 1)) - area) >> (GFxPolyBaseShift * 2 + 1 - GFxAAShift);
            if (alpha < 0)
                alpha = -alpha;
            if (Rule == FillEvenOdd)
            {
                alpha &= GFxAA2Mask;
                if (alpha > GFxAANum)
                    alpha = GFxAA2Num - alpha;
            }
            if (alpha > GFxAAMask)
                alpha = GFxAAMask;
            if (GammaTable.GetSize())
                alpha = GammaTable[(unsigned int)alpha];
            if (pixBytes)
                memset(dst + pixBytes * (unsigned int)(x - MinX), (unsigned char)alpha, pixBytes);
            ++x;
        }

        if (!num)
            break;

        const int nextX = (*cells)->X;
        if (nextX > x)
        {
            int alpha = (cover << (GFxPolyBaseShift + 1)) >> (GFxPolyBaseShift * 2 + 1 - GFxAAShift);
            if (alpha < 0)
                alpha = -alpha;
            if (Rule == FillEvenOdd)
            {
                alpha &= GFxAA2Mask;
                if (alpha > GFxAANum)
                    alpha = GFxAA2Num - alpha;
            }
            if (alpha > GFxAAMask)
                alpha = GFxAAMask;
            if (GammaTable.GetSize())
                alpha = GammaTable[(unsigned int)alpha];
            if (alpha)
                memset(dst + pixBytes * (unsigned int)(x - MinX), (unsigned char)alpha,
                       pixBytes * (unsigned int)(nextX - x));
        }
    }
}

bool GRasterizer::RasterizeToAlpha(GImage* out, int width, int height)
{
    // Factored out of GFxGlyphRasterCache::rasterizeAndPack (0xa4f5d0) so a glyph can be dumped
    // without an atlas. The sweep writes from MinX, which is why each row is offset by MinX and
    // clipped to the requested width.
    if (!SortCells())
    {
        if (width > 0 && height > 0)
            return GFxImageInit(out, (unsigned int)width, (unsigned int)height, GImageBase::Image_A_8);
        return false;
    }
    if (width <= 0)
        width = MaxX - MinX + 1;
    if (height <= 0)
        height = MaxY - MinY + 1;
    if (!GFxImageInit(out, (unsigned int)width, (unsigned int)height, GImageBase::Image_A_8))
        return false;

    const unsigned int span = (unsigned int)(MaxX - MinX + 2);
    unsigned char* row = (unsigned char*)malloc(span);
    if (!row)
        return false;

    for (unsigned int line = 0; line < SortedY.GetSize(); ++line)
    {
        const int y = MinY + (int)line;
        if (y < 0 || y >= height)
            continue;
        memset(row, 0, span);
        SweepScanline(line, row, 1);
        unsigned char* dst = out->pData + (unsigned int)y * out->Pitch;
        for (int x = 0; x < width; ++x)
        {
            const int sx = x - MinX;
            if (sx >= 0 && (unsigned int)sx < span)
                dst[x] = row[sx];
        }
    }
    free(row);
    return true;
}
