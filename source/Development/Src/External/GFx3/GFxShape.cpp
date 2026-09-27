// GFx 3.3 shape geometry - the SWF shape-record decoder and the compound shape.
// See GFxShape.h for the evidence trail and the one documented deviation.
#include "GFxShape.h"
#include "GFxPlayer.h"

#include <math.h>
#include <float.h>

static unsigned int GFxShapeMaxFlattenDepth = 0;

unsigned int GFxShapeGetMaxFlattenDepth() { return GFxShapeMaxFlattenDepth; }
void GFxShapeResetMaxFlattenDepth() { GFxShapeMaxFlattenDepth = 0; }

// =============================================================================================
// GFxShapeBase
// =============================================================================================

GFxShapeBase::GFxShapeBase()
    : ShapeCount(0), StyleRecordsRefused(0), Flags(0), BoundValid(false)
{
    // DISHONORED(port): 0xa41370 - the base constructor zeroes the cached bound and the flags.
}

GFxShapeBase::~GFxShapeBase()
{
    // DISHONORED(port): 0xa3baf0
}

void GFxShapeBase::GetShapeAndPathCounts(unsigned int* shapes, unsigned int* paths) const
{
    // DISHONORED(port): 0xa3b660 / 0xa3c930 - both write through only the non-null pointers.
    if (shapes)
        *shapes = ShapeCount;
    if (paths)
        *paths = Paths.GetSize();
}

void GFxShapeBase::ComputeBound(GRect<float>* out) const
{
    // DISHONORED(port): 0xa3f320 (ComputeBoundImpl<GFxSwfPathData>) - walk every path's move-to and
    // every edge's control and anchor point. Retail bounds the *control* points too rather than the
    // true curve extent, which over-estimates by design; that is reproduced, because the glyph
    // atlas's slot sizes depend on it.
    float l = FLT_MAX, t = FLT_MAX, r = -FLT_MAX, b = -FLT_MAX;
    const float scale = GetCoordScale();
    for (unsigned int p = 0; p < Paths.GetSize(); ++p)
    {
        const GFxShapePath& path = Paths[p];
        float x = path.MoveX * scale;
        float y = path.MoveY * scale;
        if (x < l) l = x;
        if (x > r) r = x;
        if (y < t) t = y;
        if (y > b) b = y;
        for (unsigned int e = 0; e < path.EdgeCount; ++e)
        {
            const GFxShapeEdge& edge = Edges[path.EdgeStart + e];
            const float cx = edge.Cx * scale, cy = edge.Cy * scale;
            const float ax = edge.Ax * scale, ay = edge.Ay * scale;
            if (edge.Curve)
            {
                if (cx < l) l = cx;
                if (cx > r) r = cx;
                if (cy < t) t = cy;
                if (cy > b) b = cy;
            }
            if (ax < l) l = ax;
            if (ax > r) r = ax;
            if (ay < t) t = ay;
            if (ay > b) b = ay;
        }
    }
    if (l > r)
    {
        l = t = r = b = 0.0f;
    }
    out->Left = l;
    out->Top = t;
    out->Right = r;
    out->Bottom = b;
}

GRect<float> GFxShapeBase::GetRectBoundsLocal() const
{
    // DISHONORED(port): 0xa3cf10 - caches, because a glyph's bound is asked for once per raster and
    // once per fit.
    if (!BoundValid)
    {
        ComputeBound(&CachedBound);
        BoundValid = true;
    }
    return CachedBound;
}

void GFxShapeBase::MakeCompoundShape(GCompoundShape* out, float tolerance) const
{
    // DISHONORED(port): 0xa40490 - Clear, SetCurveTolerance, copy the "closed" flag, then walk the
    // paths iterator calling AddForTessellation (0xa3fb50) until it reports the end.
    out->Clear();
    out->SetCurveTolerance(tolerance);

    const float scale = GetCoordScale();
    for (unsigned int p = 0; p < Paths.GetSize(); ++p)
    {
        const GFxShapePath& path = Paths[p];
        // AddForTessellation scales the move-to by the DefineFont3 factor when the iterator's
        // high flag bit is set (0xa3fb50: `if (*(char*)(this+112) < 0) { x *= Scale; y *= Scale; }`).
        out->BeginPath(path.LeftStyle, path.RightStyle, path.LineStyle,
                       path.MoveX * scale, path.MoveY * scale);
        for (unsigned int e = 0; e < path.EdgeCount; ++e)
        {
            const GFxShapeEdge& edge = Edges[path.EdgeStart + e];
            if (edge.Curve)
                out->AddCurve(edge.Cx * scale, edge.Cy * scale, edge.Ax * scale, edge.Ay * scale);
            else
                out->AddVertex(edge.Ax * scale, edge.Ay * scale);
        }
        out->ClosePath();
    }
}

// =============================================================================================
// GFxConstShapeNoStyles::Read - the SWF SHAPE / SHAPEWITHSTYLE record
// =============================================================================================

// A DefineFont glyph's record has no style arrays; DefineShape and friends put them in front. The
// dialect differences retail encodes in the tag type it passes down (GFxFontData::Read 0xa587d0
// passes DefineShape for tag 10, DefineShape2 for tag 48 and the DefineFont3 code itself for 75):
//   * tags above DefineShape allow an extended (0xFF + u16) fill-style count;
//   * `GFxLoadProcess::ReadRgbaTag` (0xa22460) reads RGBA when `tagType > 22` and RGB otherwise, so
//     DefineShape and DefineShape2 fills are three bytes and DefineShape3/4 fills are four;
//   * the DefineFont3 code additionally means "coordinates are 20x", which is
//     `if (a3 == 75) Flags |= 2` in the retail record walk (0xa42ab0).
//
// ADDENDUM (agent CD's hand-over 2, re-verified against retail): what a *no-style* shape does with a
// StateNewStyles record is not to walk the style arrays' wire format. Retail's record walk calls two
// helpers, `GFx_ReadFillStyles` (2012 0xa429c0) and `GFx_ReadLineStyles` (0xa41270) - the names come
// from their own error strings - and each of them:
//
//   * reads the count (a u8, promoted to a u16 when it is 0xFF: for fill styles only when
//     `tagType > 2`, for line styles **unconditionally** - the two thresholds differ and both are in
//     the decompiles);
//   * then, **when there is no style owner, logs an error and reads nothing more**:
//     "Error: GFx_ReadFillStyles, trying to read %d fillstyles into no-style shape";
//   * and only with an owner reads each style through GFxFillStyle::Read (0xa90290) /
//     GFxLineStyle::Read (0xa907d0), which package CD owns and which are the tree's only
//     implementation of those two functions.
//
// The record walk then re-reads fillBits/lineBits from the position the helpers report. So that is
// what happens here, and the two style-array walkers this file used to carry are gone: they were an
// invention rather than a port, and they carried two genuine wire-format errors with them (the focal
// gradient's focal point is read *after* the gradient records, not before, and DefineShape4's
// miter-limit flag is 0x20, not 0x0800). agentCB.md's addendum has the detail.
static bool GFxShapeTagHasExtendedFillCount(unsigned int tagType)
{
    // GFx_ReadFillStyles 0xa429c0: `if (a3 > 2 && count == 255) count = ReadU16()`.
    return tagType > GFxTag_DefineShape;
}

// DISHONORED(port): 0xa429c0 / 0xa41270 - GFx_ReadFillStyles and GFx_ReadLineStyles against a shape
// with no style owner: consume the two counts and report them, which is all those bodies do in that
// case before they log "trying to read %d fillstyles into no-style shape" and return. The with-owner
// arm, which reads each style through GFxFillStyle::Read (0xa90290) / GFxLineStyle::Read (0xa907d0),
// is package CD's GFxShapeCharacterDef and is deliberately not duplicated here.
static void GFxReadStyleCountsNoOwner(GFxStream* s, unsigned int tagType,
                                      unsigned int* outFills, unsigned int* outLines)
{
    unsigned int fills = s->ReadU8();
    if (fills == 0xFF && GFxShapeTagHasExtendedFillCount(tagType))
        fills = s->ReadU16();
    unsigned int lines = s->ReadU8();
    if (lines == 0xFF)                       // unconditional in 0xa41270, unlike the fill count
        lines = s->ReadU16();
    if (outFills)
        *outFills = fills;
    if (outLines)
        *outLines = lines;
}

bool GFxConstShapeNoStyles::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    // DISHONORED(port): 0xa42ab0, with the record walk of GFxSwfPathData::PathsIterator::ReadNext
    // (0xa3a7f0) folded in - retail defers that walk, we do it here once (GFxShape.h, DEVIATION).
    Paths.Clear();
    Edges.Clear();
    ShapeCount = 0;
    StyleRecordsRefused = 0;
    Flags = 0;
    BoundValid = false;

    if (tagType == GFxTag_DefineFont3)
        Flags |= SF_TwentyTimesScale;

    unsigned int fillBase = 0, lineBase = 0;
    unsigned int fillBits = 1, lineBits = 1;

    // The caller positions the stream at the record itself. A real DefineShape* carries its bound and
    // its two style arrays in front of it, and reading those is GFxConstShapeWithStyles::Read
    // (0xa43610) - package CD's GFxShapeCharacterDef - not this function's; the font reader
    // (0xa587d0) positions us at the raw glyph offset, where the record starts immediately.
    //
    // The SHAPE record itself: fillBits(4), lineBits(4), then records until a zero non-edge flag.
    s->Align();
    fillBits = s->ReadUBits(4);
    lineBits = s->ReadUBits(4);

    int fill0 = 0, fill1 = 0, line = 0;
    int penX = 0, penY = 0;
    bool pathOpen = false;
    GFxShapePath path;
    path.LeftStyle = path.RightStyle = path.LineStyle = -1;
    path.MoveX = path.MoveY = 0.0f;
    path.EdgeStart = path.EdgeCount = 0;

    const unsigned int guard = endPos;
    ++ShapeCount;

    for (;;)
    {
        if (s->Tell() >= guard)
            break;

        unsigned int typeFlag = s->ReadUBits(1);
        if (typeFlag == 0)
        {
            unsigned int flags = s->ReadUBits(5);
            if (flags == 0)
                break;                              // end of the shape record

            if ((flags & 0x01) != 0)                // StateMoveTo
            {
                unsigned int moveBits = s->ReadUBits(5);
                penX = s->ReadSBits(moveBits);
                penY = s->ReadSBits(moveBits);
            }
            if ((flags & 0x02) != 0 && fillBits)    // StateFillStyle0
            {
                unsigned int v = s->ReadUBits(fillBits);
                fill0 = v ? (int)(v + fillBase) : 0;
            }
            if ((flags & 0x04) != 0 && fillBits)    // StateFillStyle1
            {
                unsigned int v = s->ReadUBits(fillBits);
                fill1 = v ? (int)(v + fillBase) : 0;
            }
            if ((flags & 0x08) != 0 && lineBits)    // StateLineStyle
            {
                unsigned int v = s->ReadUBits(lineBits);
                line = v ? (int)(v + lineBase) : 0;
            }
            if ((flags & 0x10) != 0)                // StateNewStyles
            {
                // ReadNext (0xa3a7f0) resets the three current styles, accumulates the style bases by
                // the counts the two style helpers report, and re-reads fillBits/lineBits. This class
                // is the *no-style* shape, so the helpers behave as 0xa429c0 / 0xa41270 do with a null
                // style owner: the counts are consumed, nothing else is, and a non-zero count is the
                // error retail logs. A shape that really has styles is package CD's
                // GFxShapeCharacterDef, whose GFxShapeRecord hands the arrays to a style owner.
                if (pathOpen)
                {
                    path.EdgeCount = Edges.GetSize() - path.EdgeStart;
                    if (path.EdgeCount)
                        Paths.PushBack(path);
                    pathOpen = false;
                }
                fill0 = fill1 = line = 0;
                unsigned int nf = 0, nl = 0;
                GFxReadStyleCountsNoOwner(s, tagType, &nf, &nl);
                if (nf || nl)
                {
                    // Retail logs and gives up on the styles here; the record walk still continues,
                    // so the flag is recorded and the caller can report it rather than the stream
                    // silently drifting.
                    ++StyleRecordsRefused;
                }
                fillBase += nf;
                lineBase += nl;
                s->Align();
                fillBits = s->ReadUBits(4);
                lineBits = s->ReadUBits(4);
                ++ShapeCount;
            }

            // Every style or position change starts a new path, because a compound shape's path is
            // exactly a run of edges with one (left, right, line) triple.
            if (pathOpen)
            {
                path.EdgeCount = Edges.GetSize() - path.EdgeStart;
                if (path.EdgeCount)
                    Paths.PushBack(path);
            }
            path.LeftStyle = fill1 - 1;
            path.RightStyle = fill0 - 1;
            path.LineStyle = line - 1;
            path.MoveX = (float)penX;
            path.MoveY = (float)penY;
            path.EdgeStart = Edges.GetSize();
            path.EdgeCount = 0;
            pathOpen = true;
        }
        else
        {
            unsigned int straight = s->ReadUBits(1);
            GFxShapeEdge edge;
            if (straight)
            {
                unsigned int numBits = s->ReadUBits(4) + 2;
                unsigned int general = s->ReadUBits(1);
                int dx = 0, dy = 0;
                if (general)
                {
                    dx = s->ReadSBits(numBits);
                    dy = s->ReadSBits(numBits);
                }
                else if (s->ReadUBits(1))
                {
                    dy = s->ReadSBits(numBits);
                }
                else
                {
                    dx = s->ReadSBits(numBits);
                }
                penX += dx;
                penY += dy;
                edge.Cx = (float)penX;
                edge.Cy = (float)penY;
                edge.Ax = (float)penX;
                edge.Ay = (float)penY;
                edge.Curve = false;
            }
            else
            {
                unsigned int numBits = s->ReadUBits(4) + 2;
                int cdx = s->ReadSBits(numBits);
                int cdy = s->ReadSBits(numBits);
                int adx = s->ReadSBits(numBits);
                int ady = s->ReadSBits(numBits);
                const int cx = penX + cdx;
                const int cy = penY + cdy;
                penX = cx + adx;
                penY = cy + ady;
                edge.Cx = (float)cx;
                edge.Cy = (float)cy;
                edge.Ax = (float)penX;
                edge.Ay = (float)penY;
                edge.Curve = true;
            }
            if (!pathOpen)
            {
                path.LeftStyle = fill1 - 1;
                path.RightStyle = fill0 - 1;
                path.LineStyle = line - 1;
                path.MoveX = (float)penX;
                path.MoveY = (float)penY;
                path.EdgeStart = Edges.GetSize();
                pathOpen = true;
            }
            Edges.PushBack(edge);
        }
    }

    if (pathOpen)
    {
        path.EdgeCount = Edges.GetSize() - path.EdgeStart;
        if (path.EdgeCount)
            Paths.PushBack(path);
    }
    s->Align();
    return true;
}

// =============================================================================================
// GCompoundShape
// =============================================================================================

GCompoundShape::GCompoundShape()
    : CurveTolerance(1.0f), ToleranceSq(0.0625f), MinStyle(0x7FFFFFFF), MaxStyle(-0x7FFFFFFF),
      Current(-1)
{
    SetCurveTolerance(1.0f);
}

void GCompoundShape::Clear()
{
    // DISHONORED(port): 0xa59c90
    Vertices.Clear();
    Paths.Clear();
    Current = -1;
    MinStyle = 0x7FFFFFFF;
    MaxStyle = -0x7FFFFFFF;
}

void GCompoundShape::SetCurveTolerance(float t)
{
    // DISHONORED(port): 0xa59730 - the flattener's threshold is (t/4)^2, not t.
    CurveTolerance = t;
    const float q = t * 0.25f;
    ToleranceSq = q * q;
}

void GCompoundShape::BeginPath(int left, int right, int line)
{
    // DISHONORED(port): 0xa5a010
    SPath p;
    p.VertexStart = Vertices.GetSize();
    p.VertexCount = 0;
    p.LeftStyle = left;
    p.RightStyle = right;
    p.LineStyle = line;
    Paths.PushBack(p);
    Current = (int)Paths.GetSize() - 1;
    if (left >= 0)
    {
        if (left < MinStyle) MinStyle = left;
        if (left > MaxStyle) MaxStyle = left;
    }
    if (right >= 0)
    {
        if (right < MinStyle) MinStyle = right;
        if (right > MaxStyle) MaxStyle = right;
    }
}

void GCompoundShape::BeginPath(int left, int right, int line, float x, float y)
{
    // DISHONORED(port): 0xa5a0a0 - BeginPath then the first vertex.
    BeginPath(left, right, line);
    AddVertex(x, y);
}

void GCompoundShape::AddVertex(float x, float y)
{
    if (Current < 0)
        BeginPath(-1, -1, -1);
    Vertices.PushBack(GPoint<float>(x, y));
    ++Paths[(unsigned int)Current].VertexCount;
}

void GCompoundShape::AddCurve(float cx, float cy, float ax, float ay)
{
    // DISHONORED(port): 0xa5a410 - a quadratic whose three points are collinear within 1e-4 of the
    // cross product degenerates to its anchor and is not subdivided at all.
    if (Current < 0 || Paths[(unsigned int)Current].VertexCount == 0)
    {
        AddVertex(ax, ay);
        return;
    }
    const GPoint<float>& last = Vertices[Vertices.GetSize() - 1];
    const float cross = (ax - cx) * (cy - last.y) - (cx - last.x) * (ay - cy);
    if (fabsf(cross) >= 0.00009999999747378752f)
        flattenQuadraticCurve(last.x, last.y, cx, cy, ax, ay, 0);
    else
        AddVertex(ax, ay);
}

void GCompoundShape::flattenQuadraticCurve(float x0, float y0, float cx, float cy,
                                           float ax, float ay, int depth)
{
    // DISHONORED(port): 0xa5a1a0. Retail is a true recursion with no depth limit, guarded only by
    // the collinearity test; the limit here is a safety net, and GFxShapeGetMaxFlattenDepth reports
    // how close any real glyph came to it so the claim is measured rather than asserted. Measured
    // over all 12 fonts of the six DisFonts payloads at tolerance 10.0, the deepest subdivision any
    // glyph reaches is 4 (ChaletComprime 2, Emerge BF 3, Goudy Stout 4, Benguiat 4, PragmaticaCondC
    // 3), so the cap of 24 is never approached and truncates nothing.
    if ((unsigned int)depth > GFxShapeMaxFlattenDepth)
        GFxShapeMaxFlattenDepth = (unsigned int)depth;
    const float mid1x = (x0 + cx) * 0.5f;
    const float mid1y = (y0 + cy) * 0.5f;
    const float mid2x = (cx + ax) * 0.5f;
    const float mid2y = (cy + ay) * 0.5f;
    const float dx = ax - x0;
    const float dy = ay - y0;
    const float cross = (cx - ax) * dy - (cy - ay) * dx;
    const float acr = fabsf(cross);

    if (depth < 24 && acr > 0.00009999999747378752f &&
        acr * acr > (dx * dx + dy * dy) * ToleranceSq)
    {
        const float mx = (mid1x + mid2x) * 0.5f;
        const float my = (mid1y + mid2y) * 0.5f;
        flattenQuadraticCurve(x0, y0, mid1x, mid1y, mx, my, depth + 1);
        flattenQuadraticCurve(mx, my, mid2x, mid2y, ax, ay, depth + 1);
        return;
    }
    AddVertex(ax, ay);
}

void GCompoundShape::ClosePath()
{
    // DISHONORED(port): 0xa5a120 - a path with more than one vertex gets a copy of its first vertex
    // appended, which is what makes GRasterizer's implicit close a no-op and what keeps the
    // tessellator's edge list closed.
    if (Current < 0)
        return;
    SPath& p = Paths[(unsigned int)Current];
    if (p.VertexCount > 1)
    {
        const GPoint<float> first = Vertices[p.VertexStart];
        Vertices.PushBack(first);
        ++p.VertexCount;
    }
}

void GCompoundShape::ScaleAndTranslate(float sx, float sy, float tx, float ty)
{
    // DISHONORED(port): 0xa59830
    for (unsigned int i = 0; i < Vertices.GetSize(); ++i)
    {
        Vertices[i].x = Vertices[i].x * sx + tx;
        Vertices[i].y = Vertices[i].y * sy + ty;
    }
}

void GCompoundShape::PerceiveBounds(float* l, float* t, float* r, float* b) const
{
    // DISHONORED(port): 0xa59750 - note retail's initial values are 1,1,0,0, i.e. an *inverted*
    // rectangle when there are no vertices, which callers test for.
    *l = 1.0f; *t = 1.0f; *r = 0.0f; *b = 0.0f;
    if (Vertices.GetSize() == 0)
        return;
    *l = *r = Vertices[0].x;
    *t = *b = Vertices[0].y;
    for (unsigned int i = 1; i < Vertices.GetSize(); ++i)
    {
        const GPoint<float>& v = Vertices[i];
        if (*l > v.x) *l = v.x;
        if (*t > v.y) *t = v.y;
        if (*r < v.x) *r = v.x;
        if (*b < v.y) *b = v.y;
    }
}
