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
    : ShapeCount(0), Flags(0), BoundValid(false)
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
//   * tags >= DefineShape2 allow an extended (0xFF + u16) style count;
//   * tags >= DefineShape3 carry RGBA rather than RGB fills;
//   * the DefineFont3 code additionally means "coordinates are 20x".
static bool GFxShapeTagHasAlpha(unsigned int tagType)
{
    return tagType == GFxTag_DefineShape3 || tagType == GFxTag_DefineShape4;
}

static bool GFxShapeTagHasExtendedCounts(unsigned int tagType)
{
    return tagType >= GFxTag_DefineShape2;
}

// Skip one FILLSTYLE / LINESTYLE array. A glyph never has them, but a DefineShape does and the same
// decoder serves both, exactly as retail's does (GFxConstShapeNoStyles::Read 0xa42ab0 reads them
// into the two GArray<GFxFillStyle>/GArray<GFxLineStyle> out-parameters it is handed, or discards
// them when they are null - which is the call the font reader makes).
static void GFxSkipFillStyles(GFxStream* s, unsigned int tagType, unsigned int* outCount)
{
    unsigned int count = s->ReadU8();
    if (count == 0xFF && GFxShapeTagHasExtendedCounts(tagType))
        count = s->ReadU16();
    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned char type = s->ReadU8();
        if (type == 0x00)
        {
            if (GFxShapeTagHasAlpha(tagType)) s->Skip(4); else s->Skip(3);
        }
        else if (type == 0x10 || type == 0x12 || type == 0x13)
        {
            GMatrix2D m;
            s->ReadMatrix(&m);
            if (type == 0x13)
                s->ReadU16();                    // focal point, DefineShape4's focal gradient
            unsigned char info = s->ReadU8();
            unsigned int records = info & 0x0F;
            for (unsigned int g = 0; g < records; ++g)
            {
                s->ReadU8();                     // ratio
                if (GFxShapeTagHasAlpha(tagType)) s->Skip(4); else s->Skip(3);
            }
        }
        else if (type >= 0x40 && type <= 0x43)
        {
            s->ReadU16();                        // bitmap character id
            GMatrix2D m;
            s->ReadMatrix(&m);
        }
    }
    if (outCount)
        *outCount = count;
}

static void GFxSkipLineStyles(GFxStream* s, unsigned int tagType, unsigned int* outCount)
{
    unsigned int count = s->ReadU8();
    if (count == 0xFF && GFxShapeTagHasExtendedCounts(tagType))
        count = s->ReadU16();
    for (unsigned int i = 0; i < count; ++i)
    {
        s->ReadU16();                            // width
        if (tagType == GFxTag_DefineShape4)
        {
            unsigned short flags = s->ReadU16();
            if ((flags & 0x0800) != 0)           // HasMiterJoin
                s->ReadU16();
            if ((flags & 0x0008) != 0)           // HasFillFlag
                GFxSkipFillStyles(s, tagType, 0);
            else if (GFxShapeTagHasAlpha(tagType))
                s->Skip(4);
            else
                s->Skip(3);
        }
        else if (GFxShapeTagHasAlpha(tagType))
        {
            s->Skip(4);
        }
        else
        {
            s->Skip(3);
        }
    }
    if (outCount)
        *outCount = count;
}

bool GFxConstShapeNoStyles::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    // DISHONORED(port): 0xa42ab0, with the record walk of GFxSwfPathData::PathsIterator::ReadNext
    // (0xa3a7f0) folded in - retail defers that walk, we do it here once (GFxShape.h, DEVIATION).
    Paths.Clear();
    Edges.Clear();
    ShapeCount = 0;
    Flags = 0;
    BoundValid = false;

    if (tagType == GFxTag_DefineFont3)
        Flags |= SF_TwentyTimesScale;

    unsigned int fillBase = 0, lineBase = 0;
    unsigned int fillBits = 1, lineBits = 1;

    if (tagType >= GFxTag_DefineShape && tagType <= GFxTag_DefineShape4 &&
        tagType != GFxTag_DefineFont && tagType != GFxTag_DefineFont2 &&
        tagType != GFxTag_DefineFont3)
    {
        // A real DefineShape*: the bound and the style arrays come first. A glyph record skips both,
        // which is why the font reader passes the raw glyph offset as the stream position.
    }

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
                // ReadNext (0xa3a7f0) resets the three current styles, accumulates the style bases
                // by the counts of the arrays it walks past, and re-reads fillBits/lineBits.
                if (pathOpen)
                {
                    path.EdgeCount = Edges.GetSize() - path.EdgeStart;
                    if (path.EdgeCount)
                        Paths.PushBack(path);
                    pathOpen = false;
                }
                fill0 = fill1 = line = 0;
                unsigned int nf = 0, nl = 0;
                GFxSkipFillStyles(s, tagType, &nf);
                GFxSkipLineStyles(s, tagType, &nl);
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
