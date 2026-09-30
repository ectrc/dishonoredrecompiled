// The display half: see GFxDisplay.h for the retail functions each body is written from and for the
// one large deviation (the tessellator).
//
// Coordinate convention, stated once because every transform here depends on it. A character's matrix
// maps its own units to its parent's; the root's matrix maps twips to pixels, and GFxMovieRoot::Display
// (0xa07aa0) is what installs it from the viewport. Everything below therefore works in twips and the
// renderer is handed one GMatrix2D per character that ends in the twips-to-viewport scale, which is
// exactly what FGFxRenderer::SetMatrix expects (agentCC.md 3: the row-vector form).
#include "GFxDisplay.h"
#include "GFxGlyphCache.h"
#include "GFxTextField.h"
#include "GFxTextDocView.h"
#include "GFxFont.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ---------------------------------------------------------------------------------------------
// The two composition operators.

bool GFxDisplayNoShapes = false;
bool GFxDisplayNoText = false;
bool GFxDisplayNoImages = false;
bool GFxDisplayNoBeginDisplay = false;
// DISHONORED(bringup): style groups skipped because their image fill had no texture.
unsigned int GFxDisplayUntexturedFills = 0;
unsigned int GFxDisplayDrawTrace = 0;
unsigned int GFxDisplayEmptyMasks = 0;
static char GFxDisplayLastFill[96] = "";
static void GFxStrCopy(char* dst, unsigned int cap, const char* src)
{
    unsigned int i = 0;
    for (; src != 0 && src[i] != 0 && i + 1 < cap; ++i) dst[i] = src[i];
    dst[i] = 0;
}
bool GFxDisplayFitFill = false;
bool GFxDisplayNoTextShadow = false;

void GFxDisplayMatrixAppend(GMatrix2D* out, const GMatrix2D& a, const GMatrix2D& b)
{
    // GMatrix2D::Append: out = a * b, i.e. b applied first. GFx stores two rows of three.
    GMatrix2D r;
    r.M_[0][0] = a.M_[0][0] * b.M_[0][0] + a.M_[0][1] * b.M_[1][0];
    r.M_[0][1] = a.M_[0][0] * b.M_[0][1] + a.M_[0][1] * b.M_[1][1];
    r.M_[0][2] = a.M_[0][0] * b.M_[0][2] + a.M_[0][1] * b.M_[1][2] + a.M_[0][2];
    r.M_[1][0] = a.M_[1][0] * b.M_[0][0] + a.M_[1][1] * b.M_[1][0];
    r.M_[1][1] = a.M_[1][0] * b.M_[0][1] + a.M_[1][1] * b.M_[1][1];
    r.M_[1][2] = a.M_[1][0] * b.M_[0][2] + a.M_[1][1] * b.M_[1][2] + a.M_[1][2];
    *out = r;
}

bool GFxDisplayMatrixInvert(GMatrix2D* out, const GMatrix2D& m)
{
    const float det = m.M_[0][0] * m.M_[1][1] - m.M_[0][1] * m.M_[1][0];
    if (det > -1e-12f && det < 1e-12f)
    {
        out->SetIdentity();
        return false;
    }
    const float inv = 1.0f / det;
    GMatrix2D r;
    r.M_[0][0] =  m.M_[1][1] * inv;
    r.M_[0][1] = -m.M_[0][1] * inv;
    r.M_[0][2] = (m.M_[0][1] * m.M_[1][2] - m.M_[1][1] * m.M_[0][2]) * inv;
    r.M_[1][0] = -m.M_[1][0] * inv;
    r.M_[1][1] =  m.M_[0][0] * inv;
    r.M_[1][2] = (m.M_[1][0] * m.M_[0][2] - m.M_[0][0] * m.M_[1][2]) * inv;
    *out = r;
    return true;
}

GMatrix2D GFxCharacterWorldMatrix(const GFxCharacter* ch)
{
    GMatrix2D world;
    world.SetIdentity();
    for (const GFxCharacter* p = ch; p != 0; p = p->GetParent())
    {
        GMatrix2D composed;
        GFxDisplayMatrixAppend(&composed, p->GetMatrix(), world);
        world = composed;
    }
    return world;
}

void GFxDisplayCxformConcat(GRenderer::Cxform* out, const GRenderer::Cxform& outer,
                            const GRenderer::Cxform& inner)
{
    // GRenderer::Cxform::Concatenate. The layout is channel-major with the multiply in column 0 and
    // the add in column 1 (commit 42e9cbe; agentCC.md 2). Applying the inner transform first and then
    // the outer gives mul = outer.mul * inner.mul and add = outer.add + outer.mul * inner.add.
    for (int c = 0; c < 4; ++c)
    {
        out->M_[c][1] = outer.M_[c][1] + outer.M_[c][0] * inner.M_[c][1];
        out->M_[c][0] = outer.M_[c][0] * inner.M_[c][0];
    }
}

static void GFxDisplayCxformIdentity(GRenderer::Cxform* cx)
{
    for (int c = 0; c < 4; ++c)
    {
        cx->M_[c][0] = 1.0f;
        cx->M_[c][1] = 0.0f;
    }
}

static GUByte GFxDisplayClampByte(float v)
{
    if (v <= 0.0f)
        return 0;
    if (v >= 255.0f)
        return 255;
    return (GUByte)(v + 0.5f);
}

GColor GFxDisplayApplyCxform(const GRenderer::Cxform& cx, GColor c)
{
    // The renderer applies the colour transform itself for a solid fill (FGFxRenderStyle::Apply sets
    // the cxform shader parameters), so this is only used where a colour has to be folded into a
    // vertex or a bitmap descriptor - the glyph colour and the Gouraud path.
    return GColor(GFxDisplayClampByte(c.GetRed() * cx.M_[0][0] + cx.M_[0][1]),
                  GFxDisplayClampByte(c.GetGreen() * cx.M_[1][0] + cx.M_[1][1]),
                  GFxDisplayClampByte(c.GetBlue() * cx.M_[2][0] + cx.M_[2][1]),
                  GFxDisplayClampByte(c.GetAlpha() * cx.M_[3][0] + cx.M_[3][1]));
}

// ---------------------------------------------------------------------------------------------
// GFxShapeMesh: flatten, split by style, triangulate.

GFxShapeMesh::GFxShapeMesh()
    : Groups(0), GroupCount(0), GroupCapacity(0), Vertices(0), VertexCount(0), VertexCapacity(0),
      bBuilt(false)
{
}

GFxShapeMesh::~GFxShapeMesh()
{
    free(Groups);
    free(Vertices);
}

void GFxShapeMesh::reserveVertices(unsigned int extra)
{
    if (VertexCount * 2 + extra * 2 <= VertexCapacity)
        return;
    unsigned int cap = VertexCapacity ? VertexCapacity : 1024;
    while (VertexCount * 2 + extra * 2 > cap)
        cap *= 2;
    short* grown = (short*)realloc(Vertices, cap * sizeof(short));
    if (grown == 0)
        return;
    Vertices = grown;
    VertexCapacity = cap;
}

// The twips coordinate, clamped into int16. A shape whose bounds exceed +-32767 twips (+-1638 px) in its
// own space loses the excess; retail has the same limit because VertexXY16i is the mesh vertex.
static short GFxDisplayToTwip16(float v)
{
    if (v <= -32767.0f)
        return -32767;
    if (v >= 32767.0f)
        return 32767;
    return (short)(v >= 0.0f ? (v + 0.5f) : (v - 0.5f));
}

namespace
{
    unsigned short* GLinearIndices = 0;
    unsigned int    GLinearIndexCount = 0;
}

const unsigned short* GFxDisplayGetLinearIndices(unsigned int count)
{
    if (count > GLinearIndexCount)
    {
        unsigned int cap = GLinearIndexCount ? GLinearIndexCount : 1024;
        while (cap < count)
            cap *= 2;
        unsigned short* grown = (unsigned short*)realloc(GLinearIndices, cap * sizeof(unsigned short));
        if (grown == 0)
            return 0;
        GLinearIndices = grown;
        for (unsigned int i = GLinearIndexCount; i < cap; ++i)
            GLinearIndices[i] = (unsigned short)i;
        GLinearIndexCount = cap;
    }
    return GLinearIndices;
}

void GFxShapeMesh::addTriangle(int style, int lineStyle, const float* a, const float* b,
                               const float* c)
{
    if (Vertices == 0 && VertexCapacity == 0)
        reserveVertices(3);
    reserveVertices(3);
    if (VertexCount * 2 + 6 > VertexCapacity)
        return;

    // A run of triangles with the same style is one group, which is one DrawIndexedTriList.
    if (GroupCount == 0 || Groups[GroupCount - 1].Style != style
        || Groups[GroupCount - 1].LineStyle != lineStyle)
    {
        if (GroupCount == GroupCapacity)
        {
            const unsigned int cap = GroupCapacity ? GroupCapacity * 2 : 16;
            Group* grown = (Group*)realloc(Groups, cap * sizeof(Group));
            if (grown == 0)
                return;
            Groups = grown;
            GroupCapacity = cap;
        }
        Group& g = Groups[GroupCount++];
        g.Style = style;
        g.LineStyle = lineStyle;
        g.VertexStart = VertexCount;
        g.VertexCount = 0;
    }
    short* v = Vertices + VertexCount * 2;
    v[0] = GFxDisplayToTwip16(a[0]); v[1] = GFxDisplayToTwip16(a[1]);
    v[2] = GFxDisplayToTwip16(b[0]); v[3] = GFxDisplayToTwip16(b[1]);
    v[4] = GFxDisplayToTwip16(c[0]); v[5] = GFxDisplayToTwip16(c[1]);
    VertexCount += 3;
    Groups[GroupCount - 1].VertexCount += 3;
}

namespace
{
    // One flattened directed segment of one fill's boundary.
    struct MeshSeg
    {
        float X0, Y0, X1, Y1;
        int   Dir;         // +1 when the segment runs down the page, -1 up; 0 when horizontal
    };

    struct MeshSegList
    {
        MeshSeg*     Items;
        unsigned int Count;
        unsigned int Capacity;

        MeshSegList() : Items(0), Count(0), Capacity(0) {}
        ~MeshSegList() { free(Items); }
        void Add(float x0, float y0, float x1, float y1)
        {
            if (y0 == y1)
                return;
            if (Count == Capacity)
            {
                const unsigned int cap = Capacity ? Capacity * 2 : 256;
                MeshSeg* grown = (MeshSeg*)realloc(Items, cap * sizeof(MeshSeg));
                if (grown == 0)
                    return;
                Items = grown;
                Capacity = cap;
            }
            MeshSeg& s = Items[Count++];
            s.X0 = x0; s.Y0 = y0; s.X1 = x1; s.Y1 = y1;
            s.Dir = (y1 > y0) ? 1 : -1;
        }
        void Clear() { Count = 0; }
    };

    // The quadratic flattening GCompoundShape::flattenQuadraticCurve (0xa5a1a0) does, with the same
    // collinearity test: subdivide while the control point is further than the tolerance from the
    // chord. The tolerance is in the shape's own units.
    void FlattenQuad(MeshSegList& out, float x0, float y0, float cx, float cy, float x1, float y1,
                     float tolSq, int depth)
    {
        const float mx = (x0 + 2.0f * cx + x1) * 0.25f;
        const float my = (y0 + 2.0f * cy + y1) * 0.25f;
        const float dx = mx - (x0 + x1) * 0.5f;
        const float dy = my - (y0 + y1) * 0.5f;
        if (depth >= 12 || dx * dx + dy * dy <= tolSq)
        {
            out.Add(x0, y0, x1, y1);
            return;
        }
        const float ax = (x0 + cx) * 0.5f, ay = (y0 + cy) * 0.5f;
        const float bx = (cx + x1) * 0.5f, by = (cy + y1) * 0.5f;
        FlattenQuad(out, x0, y0, ax, ay, mx, my, tolSq, depth + 1);
        FlattenQuad(out, mx, my, bx, by, x1, y1, tolSq, depth + 1);
    }

    struct FloatList
    {
        float*       Items;
        unsigned int Count;
        unsigned int Capacity;
        FloatList() : Items(0), Count(0), Capacity(0) {}
        ~FloatList() { free(Items); }
        void Add(float v)
        {
            if (Count == Capacity)
            {
                const unsigned int cap = Capacity ? Capacity * 2 : 256;
                float* grown = (float*)realloc(Items, cap * sizeof(float));
                if (grown == 0)
                    return;
                Items = grown;
                Capacity = cap;
            }
            Items[Count++] = v;
        }
        void Clear() { Count = 0; }
    };

    int FloatCompare(const void* a, const void* b)
    {
        const float fa = *(const float*)a;
        const float fb = *(const float*)b;
        return fa < fb ? -1 : (fa > fb ? 1 : 0);
    }

    struct Crossing { float X; int Dir; };

    int CrossingCompare(const void* a, const void* b)
    {
        const Crossing* ca = (const Crossing*)a;
        const Crossing* cb = (const Crossing*)b;
        return ca->X < cb->X ? -1 : (ca->X > cb->X ? 1 : 0);
    }
}

// The trapezoidal decomposition. Bands are the sorted unique Y values of the segment endpoints plus
// the Y values at which two segments cross, so that within a band the crossing order of the segments
// is constant and one quad per inside span is exact. The inside test is the non-zero winding rule,
// which is SWF's (a path's LeftStyle is the fill on the side the winding enters).
static void GFxMeshTessellateStyle(GFxShapeMesh* mesh, MeshSegList& segs, int style, int lineStyle,
                                   void (GFxShapeMesh::*addTri)(int, int, const float*, const float*,
                                                                const float*))
{
    if (segs.Count == 0)
        return;

    FloatList bands;
    for (unsigned int i = 0; i < segs.Count; ++i)
    {
        bands.Add(segs.Items[i].Y0);
        bands.Add(segs.Items[i].Y1);
    }
    // Crossings. O(n^2), capped: above the cap the decomposition stays a band decomposition and a
    // crossing inside a band costs a sliver of a pixel, which is why the cap is safe rather than a
    // silent wrong answer.
    if (segs.Count <= 512)
    {
        for (unsigned int i = 0; i < segs.Count; ++i)
        {
            const MeshSeg& a = segs.Items[i];
            for (unsigned int j = i + 1; j < segs.Count; ++j)
            {
                const MeshSeg& b = segs.Items[j];
                const float ax = a.X1 - a.X0, ay = a.Y1 - a.Y0;
                const float bx = b.X1 - b.X0, by = b.Y1 - b.Y0;
                const float den = ax * by - ay * bx;
                if (den == 0.0f)
                    continue;
                const float t = ((b.X0 - a.X0) * by - (b.Y0 - a.Y0) * bx) / den;
                const float u = ((b.X0 - a.X0) * ay - (b.Y0 - a.Y0) * ax) / den;
                if (t > 0.0f && t < 1.0f && u > 0.0f && u < 1.0f)
                    bands.Add(a.Y0 + ay * t);
            }
        }
    }
    if (bands.Count < 2)
        return;
    qsort(bands.Items, bands.Count, sizeof(float), FloatCompare);

    Crossing top[256];
    Crossing bottom[256];
    for (unsigned int bi = 0; bi + 1 < bands.Count; ++bi)
    {
        const float yTop = bands.Items[bi];
        const float yBot = bands.Items[bi + 1];
        if (yBot - yTop < 0.0001f)
            continue;
        const float yMid = (yTop + yBot) * 0.5f;

        unsigned int n = 0;
        for (unsigned int i = 0; i < segs.Count && n < 256; ++i)
        {
            const MeshSeg& s = segs.Items[i];
            const float lo = s.Y0 < s.Y1 ? s.Y0 : s.Y1;
            const float hi = s.Y0 < s.Y1 ? s.Y1 : s.Y0;
            if (yMid <= lo || yMid >= hi)
                continue;
            const float f = (s.Y1 - s.Y0);
            top[n].X = s.X0 + (s.X1 - s.X0) * ((yTop - s.Y0) / f);
            top[n].Dir = s.Dir;
            bottom[n].X = s.X0 + (s.X1 - s.X0) * ((yBot - s.Y0) / f);
            bottom[n].Dir = s.Dir;
            ++n;
        }
        if (n < 2)
            continue;

        // Sort by the band's midpoint x so the top and bottom pairs stay matched.
        for (unsigned int i = 1; i < n; ++i)
        {
            const float key = (top[i].X + bottom[i].X);
            Crossing t = top[i], b = bottom[i];
            unsigned int j = i;
            while (j > 0 && (top[j - 1].X + bottom[j - 1].X) > key)
            {
                top[j] = top[j - 1];
                bottom[j] = bottom[j - 1];
                --j;
            }
            top[j] = t;
            bottom[j] = b;
        }

        int winding = 0;
        for (unsigned int i = 0; i + 1 < n; ++i)
        {
            winding += top[i].Dir;
            if (winding == 0)
                continue;
            const float tl = top[i].X, tr = top[i + 1].X;
            const float bl = bottom[i].X, br = bottom[i + 1].X;
            if (tr - tl < 0.0001f && br - bl < 0.0001f)
                continue;
            float a[2] = { tl, yTop };
            float b[2] = { tr, yTop };
            float c[2] = { br, yBot };
            float d[2] = { bl, yBot };
            (mesh->*addTri)(style, lineStyle, a, b, c);
            (mesh->*addTri)(style, lineStyle, a, c, d);
        }
    }
}

void GFxShapeMesh::Build(const GFxShapeRecord& record, const GFxShapeCharacterDef* def)
{
    // DISHONORED(written): the deviation of GFxDisplay.h - retail's GFxShapeBase::TessellateImpl
    // (0xa3fe20) over GTessellator. The flattening tolerance is GFxRenderConfig's MaxCurvePixelError
    // (the engine sets 2.0 px, FGFxEngine::FGFxEngine 2013 0x5a2370) expressed in twips.
    bBuilt = true;
    GroupCount = 0;
    VertexCount = 0;
    if (record.GetPathCount() == 0)
        return;

    const float coordScale = record.bTwentyTimesScale ? 0.05f : 1.0f;
    const float tol = 2.0f * 20.0f * (record.bTwentyTimesScale ? 20.0f : 1.0f);
    const float tolSq = tol * tol;

    // How many fill styles the record refers to. A glyph outline has no style array at all and one
    // implicit fill, which is what GFx_ReadFillStyles (0xa429c0) refuses to read into.
    unsigned int maxFill = def ? def->GetFillStyleCount() : 0;
    unsigned int maxLine = def ? def->GetLineStyleCount() : 0;
    for (unsigned int p = 0; p < record.GetPathCount(); ++p)
    {
        const GFxShapePathCD* path = record.GetPath(p);
        if (path == 0)
            continue;
        if (path->Fill0 > maxFill) maxFill = path->Fill0;
        if (path->Fill1 > maxFill) maxFill = path->Fill1;
        if (path->Line > maxLine) maxLine = path->Line;
    }

    MeshSegList segs;
    for (unsigned int style = 1; style <= maxFill; ++style)
    {
        segs.Clear();
        for (unsigned int p = 0; p < record.GetPathCount(); ++p)
        {
            const GFxShapePathCD* path = record.GetPath(p);
            if (path == 0 || path->EdgeCount == 0)
                continue;
            const bool left = path->Fill0 == style;
            const bool right = path->Fill1 == style;
            if (!left && !right)
                continue;
            // A path contributes its own direction when the style is on its left and the reverse
            // direction when it is on its right, which is what makes the winding close.
            float x = path->StartX * coordScale;
            float y = path->StartY * coordScale;
            for (unsigned int e = 0; e < path->EdgeCount; ++e)
            {
                const GFxShapeEdgeCD& edge = path->Edges[e];
                const float ax = edge.Ax * coordScale;
                const float ay = edge.Ay * coordScale;
                if (edge.bCurve)
                {
                    const float cx = edge.Cx * coordScale;
                    const float cy = edge.Cy * coordScale;
                    if (left)
                        FlattenQuad(segs, x, y, cx, cy, ax, ay, tolSq, 0);
                    if (right)
                        FlattenQuad(segs, ax, ay, cx, cy, x, y, tolSq, 0);
                }
                else
                {
                    if (left)
                        segs.Add(x, y, ax, ay);
                    if (right)
                        segs.Add(ax, ay, x, y);
                }
                x = ax;
                y = ay;
            }
        }
        GFxMeshTessellateStyle(this, segs, (int)style - 1, -1, &GFxShapeMesh::addTriangle);
    }

    // The implicit fill of a style-less record - a glyph, or a shape whose paths name fill 1 with no
    // style array. Emitted as style -1 with line style -1, which the display body paints with the
    // character's own colour.
    if (maxFill == 0)
    {
        segs.Clear();
        for (unsigned int p = 0; p < record.GetPathCount(); ++p)
        {
            const GFxShapePathCD* path = record.GetPath(p);
            if (path == 0 || path->EdgeCount == 0)
                continue;
            float x = path->StartX * coordScale;
            float y = path->StartY * coordScale;
            for (unsigned int e = 0; e < path->EdgeCount; ++e)
            {
                const GFxShapeEdgeCD& edge = path->Edges[e];
                const float ax = edge.Ax * coordScale;
                const float ay = edge.Ay * coordScale;
                if (edge.bCurve)
                    FlattenQuad(segs, x, y, edge.Cx * coordScale, edge.Cy * coordScale, ax, ay, tolSq, 0);
                else
                    segs.Add(x, y, ax, ay);
                x = ax;
                y = ay;
            }
        }
        GFxMeshTessellateStyle(this, segs, -1, -2, &GFxShapeMesh::addTriangle);
    }

    // Strokes. DEVIATION, stated at the site: retail strokes through GStrokerAA::Tessellate
    // (0xacaba0) and caches the result in a GFxCachedStroke (Display 0xab36e0) with SWF's join, cap
    // and miter model. This emits one quad per segment at the style's width, which is the stroke's
    // area but not its joins.
    for (unsigned int style = 1; style <= maxLine; ++style)
    {
        const GFxLineStyle* ls = def ? def->GetLineStyle(style - 1) : 0;
        const float halfWidth = ls ? (ls->Width * 0.5f) : 10.0f;
        for (unsigned int p = 0; p < record.GetPathCount(); ++p)
        {
            const GFxShapePathCD* path = record.GetPath(p);
            if (path == 0 || path->EdgeCount == 0 || path->Line != style)
                continue;
            float x = path->StartX * coordScale;
            float y = path->StartY * coordScale;
            for (unsigned int e = 0; e < path->EdgeCount; ++e)
            {
                const GFxShapeEdgeCD& edge = path->Edges[e];
                const float ax = edge.Ax * coordScale;
                const float ay = edge.Ay * coordScale;
                const float dx = ax - x, dy = ay - y;
                const float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.0001f)
                {
                    const float nx = -dy / len * halfWidth;
                    const float ny = dx / len * halfWidth;
                    float a[2] = { x + nx, y + ny };
                    float b[2] = { ax + nx, ay + ny };
                    float c[2] = { ax - nx, ay - ny };
                    float d[2] = { x - nx, y - ny };
                    addTriangle(-1, (int)style - 1, a, b, c);
                    addTriangle(-1, (int)style - 1, a, c, d);
                }
                x = ax;
                y = ay;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------
// The mesh cache, one entry per shape definition.

namespace
{
    struct MeshCacheEntry { GFxShapeCharacterDef* pDef; GFxShapeMesh* pMesh; };
    MeshCacheEntry* GMeshCache = 0;
    unsigned int    GMeshCacheCount = 0;
    unsigned int    GMeshCacheCapacity = 0;
}

GFxShapeMesh* GFxDisplayGetShapeMesh(GFxShapeCharacterDef* def)
{
    if (def == 0)
        return 0;
    for (unsigned int i = 0; i < GMeshCacheCount; ++i)
    {
        if (GMeshCache[i].pDef == def)
            return GMeshCache[i].pMesh;
    }
    if (GMeshCacheCount == GMeshCacheCapacity)
    {
        const unsigned int cap = GMeshCacheCapacity ? GMeshCacheCapacity * 2 : 128;
        MeshCacheEntry* grown = (MeshCacheEntry*)realloc(GMeshCache, cap * sizeof(MeshCacheEntry));
        if (grown == 0)
            return 0;
        GMeshCache = grown;
        GMeshCacheCapacity = cap;
    }
    GFxShapeMesh* mesh = new GFxShapeMesh;
    mesh->Build(def->Shape, def);
    GMeshCache[GMeshCacheCount].pDef = def;
    GMeshCache[GMeshCacheCount].pMesh = mesh;
    ++GMeshCacheCount;
    return mesh;
}

void GFxDisplayInvalidateShapeMesh(GFxShapeCharacterDef* def)
{
    for (unsigned int i = 0; i < GMeshCacheCount; ++i)
    {
        if (GMeshCache[i].pDef == def)
        {
            delete GMeshCache[i].pMesh;
            GMeshCache[i] = GMeshCache[GMeshCacheCount - 1];
            --GMeshCacheCount;
            return;
        }
    }
}

void GFxDisplayReleaseShapeMeshes()
{
    for (unsigned int i = 0; i < GMeshCacheCount; ++i)
        delete GMeshCache[i].pMesh;
    free(GMeshCache);
    GMeshCache = 0;
    GMeshCacheCount = 0;
    GMeshCacheCapacity = 0;
}

// ---------------------------------------------------------------------------------------------
// The glyph cache and its atlas texture.

namespace
{
    GFxGlyphRasterCache* GGlyphCache = 0;
    GTexture*            GGlyphTexture = 0;
    unsigned int         GGlyphTextureGeneration = 0;
}

GFxGlyphRasterCache* GFxDisplayGetGlyphCache()
{
    if (GGlyphCache == 0)
    {
        GGlyphCache = new GFxGlyphRasterCache;
        // The configuration FGFxEngine::InitGFxLoaderCommon writes into GFxFontCacheManager's
        // TextureConfig, read out of 2013 0x590ba0: one 1024x1024 texture, 48-pixel slots, 2-pixel
        // padding.
        GGlyphCache->Init(1024, 1024, 1, 48, 2);
    }
    return GGlyphCache;
}

void GFxDisplayResetGlyphCache()
{
    delete GGlyphCache;
    GGlyphCache = 0;
    GFxDisplayReleaseGlyphTexture();
}

void GFxDisplayReleaseGlyphTexture()
{
    if (GGlyphTexture)
    {
        GGlyphTexture->Release();
        GGlyphTexture = 0;
    }
    GGlyphTextureGeneration = 0;
}

GTexture* GFxDisplayGetGlyphTexture(GRenderer* renderer)
{
    // Agent CC's hand-over, applied: CreateTexture then InitDynamicTexture at Image_A_8, which makes
    // the engine resource PF_G8 and therefore selects GFx_PS_TextTexture - the alpha-only glyph
    // shader - by itself. The whole atlas is re-uploaded whenever the cache has rasterised more
    // glyphs than the last upload saw; retail's GFxGlyphRasterCache::UpdateTextures (0xa4db30)
    // uploads only the dirty rectangle, which is the difference and is named in agentDC.md.
    GFxGlyphRasterCache* cache = GFxDisplayGetGlyphCache();
    if (renderer == 0 || cache->GetTextureCount() == 0)
        return 0;
    GImage* atlas = cache->GetTexture(0);
    if (atlas == 0 || atlas->pData == 0)
        return 0;

    if (GGlyphTexture == 0)
    {
        GGlyphTexture = renderer->CreateTexture();
        if (GGlyphTexture == 0)
            return 0;
        if (!GGlyphTexture->InitDynamicTexture((int)atlas->Width, (int)atlas->Height,
                                               GImageBase::Image_A_8, 1, GTexture::Usage_Update))
        {
            GGlyphTexture->Release();
            GGlyphTexture = 0;
            return 0;
        }
        GGlyphTextureGeneration = 0;
    }
    const unsigned int generation = cache->GetRasterizedCount();
    if (generation != GGlyphTextureGeneration)
    {
        GTexture::UpdateRect rect;
        rect.dest.x = 0;
        rect.dest.y = 0;
        rect.src.Left = 0;
        rect.src.Top = 0;
        rect.src.Right = (int)atlas->Width;
        rect.src.Bottom = (int)atlas->Height;
        GGlyphTexture->Update(0, 1, &rect, atlas);
        GGlyphTextureGeneration = generation;
    }
    return GGlyphTexture;
}

// ---------------------------------------------------------------------------------------------
// GFxDisplayContext

GFxDisplayContext::GFxDisplayContext()
    : pRenderer(0), pRoot(0), pDefImpl(0), pDataDef(0), pGlyphCache(0), pGlyphTexture(0), MaskLevel(0),
      MaskDrawDepth(0)
{
    Matrix.SetIdentity();
    GFxDisplayCxformIdentity(&Cx);
    memset(&Stats, 0, sizeof(Stats));
}

void GFxDisplayContext::PreDisplay(const GFxCharacter* ch, GMatrix2D* savedMatrix,
                                   GRenderer::Cxform* savedCx)
{
    // DISHONORED(port): 2012 0xa5f5d0.
    *savedMatrix = Matrix;
    *savedCx = Cx;
    GFxDisplayMatrixAppend(&Matrix, *savedMatrix, ch->GetMatrix());
    GFxDisplayCxformConcat(&Cx, *savedCx, ch->GetCxform());
}

void GFxDisplayContext::PostDisplay(const GMatrix2D& savedMatrix, const GRenderer::Cxform& savedCx)
{
    // DISHONORED(port): 2012 0xa5f800.
    Matrix = savedMatrix;
    Cx = savedCx;
}

void GFxDisplayContext::ApplyToRenderer()
{
    if (pRenderer)
    {
        pRenderer->SetMatrix(Matrix);
        pRenderer->SetCxform(Cx);
    }
}

// ---------------------------------------------------------------------------------------------
// The display-list walk. GFxDisplayList::Display (2012 0x9d56c0): depth-ascending, with an entry
// whose ClipDepth is non-zero acting as the stencil mask for every entry up to that depth.

void GFxDisplayList::Display(GFxDisplayContext& ctx)
{
    unsigned int maskEndDepth = 0;
    bool bMaskActive = false;

    for (unsigned int i = 0; i < Size; ++i)
    {
        GFxCharacter* ch = Entries[i].pChar;
        if (ch == 0)
            continue;

        if (bMaskActive && (unsigned int)ch->GetDepth() > maskEndDepth)
        {
            if (ctx.pRenderer)
                ctx.pRenderer->DisableMask();
            bMaskActive = false;
            if (ctx.MaskLevel)
                --ctx.MaskLevel;
        }

        // Retail's own condition, 0x9cbf20: `if (!ClipDepth || ctx->MaskDrawDepth) draw as content;
        // else PushAndDrawMask`. A clip that carries a ClipDepth while a mask is being drawn is part of
        // THAT mask's shape, not the start of a nested one.
        if (ch->GetClipDepth() > 0 && !bMaskActive && ctx.MaskDrawDepth == 0)
        {
            // 2012 0xa5f510 GFxDisplayContext::PushAndDrawMask: clear the stencil, draw the mask
            // shape with colour writes off, then switch the test to "equal to the counter".
            if (ctx.pRenderer)
            {
                ctx.pRenderer->BeginSubmitMask(ctx.MaskLevel ? GRenderer::Mask_Increment
                                                             : GRenderer::Mask_Clear);
                ++ctx.MaskDrawDepth;
                const unsigned int trisBefore = ctx.Stats.Triangles;
                ch->Display(ctx);
                const unsigned int trisDrawn = ctx.Stats.Triangles - trisBefore;
                --ctx.MaskDrawDepth;
                ctx.pRenderer->EndSubmitMask();
                // DISHONORED(bringup): a mask that submitted NO triangle leaves the stencil empty, and
                // everything up to its clip depth is then rejected by EndSubmitMask's `== counter` test.
                // That is not a wrong pixel, it is the whole movie missing, so it is counted and named.
                if (trisDrawn == 0)
                {
                    ++GFxDisplayEmptyMasks;
                    if (GFxDisplayEmptyMasks <= 8)
                    {
                        GFxLogf("DISHONORED(bringup): GFx mask submitted NO geometry: depth %d clipDepth "
                                "%d visible %d alpha %.2f*x+%.0f - everything it masks is now rejected",
                                ch->GetDepth(), ch->GetClipDepth(), ch->GetVisible() ? 1 : 0,
                                ctx.Cx.M_[3][0], ctx.Cx.M_[3][1]);
                    }
                }
            }
            ++ctx.MaskLevel;
            ++ctx.Stats.Masks;
            maskEndDepth = (unsigned int)ch->GetClipDepth();
            bMaskActive = true;
            continue;
        }

        ch->Display(ctx);
    }

    if (bMaskActive)
    {
        if (ctx.pRenderer)
            ctx.pRenderer->DisableMask();
        if (ctx.MaskLevel)
            --ctx.MaskLevel;
    }
}

// ---------------------------------------------------------------------------------------------
// GFxCharacter and its three concrete display bodies.

void GFxCharacter::Display(GFxDisplayContext& ctx)
{
    (void)ctx;
}

// DISHONORED(bringup): a subtree whose composed colour transform multiplies alpha by zero and adds
// nothing is fully transparent, and it is not submitted. Retail draws it and the pixel shader's
// Cxform makes every pixel transparent; in this tree the Cxform reaches FGFxFillStyle and the cooked
// GFx_PS_CxformTexture shader does not honour it, so a clip at _alpha 0 was painted at full opacity.
// The main menu keeps two whole SCREENS on its display list at _alpha 0 - the imported load-game and
// options screens - so the whole interface was drawn under them.
static bool GFxDisplayCxformIsTransparent(const GRenderer::Cxform& cx)
{
    return cx.M_[3][0] <= 0.0f && cx.M_[3][1] <= 0.5f;
}

void GFxSprite::Display(GFxDisplayContext& ctx)
{
    // DISHONORED(port): 2012 0x9faeb0 (2013 0x9f1780). Invisible clips and clips with a zero alpha
    // multiply are skipped before the transform is composed, which is what retail's early-out does.
    ++ctx.Stats.Characters;
    if (!GetVisible())
    {
        ++ctx.Stats.Invisible;
        return;
    }
    ++ctx.Stats.Sprites;

    GMatrix2D savedMatrix;
    GRenderer::Cxform savedCx;
    ctx.PreDisplay(this, &savedMatrix, &savedCx);
    // The alpha early-out is this reconstruction's, not retail's, and it must not reach a mask: the
    // mask pass has colour writes off, so what the shape's alpha would have produced is irrelevant,
    // while NOT submitting it leaves the stencil empty and hides everything the mask covers. A mask
    // layer being invisible is Flash's own convention, so this is the common case and not a corner.
    if (ctx.MaskDrawDepth == 0 && GFxDisplayCxformIsTransparent(ctx.Cx))
    {
        ++ctx.Stats.Invisible;
        ctx.PostDisplay(savedMatrix, savedCx);
        return;
    }
    GFxMovieDataDef* savedDataDef = ctx.pDataDef;
    if (GetOwnDataDef() != 0)
        ctx.pDataDef = GetOwnDataDef();
    // DISHONORED(port, agent EA): 2013 GFxSprite::Display draws the clip's own drawing context - what
    // AS2's moveTo/lineTo/beginFill wrote - BELOW its display list, which is Flash's rule: a shape
    // drawn from script sits under every child the timeline placed.
    if (pDrawing != 0)
        pDrawing->Display(ctx, this);
    DisplayList.Display(ctx);
    ctx.pDataDef = savedDataDef;
    ctx.PostDisplay(savedMatrix, savedCx);
}

void GFxGenericCharacter::Display(GFxDisplayContext& ctx)
{
    // 2012 0x9cfdd0 forwards to the definition, which is where the geometry is.
    ++ctx.Stats.Characters;
    if (!GetVisible())
    {
        ++ctx.Stats.Invisible;
        return;
    }
    if (pDef == 0)
    {
        ++ctx.Stats.NoGeometry;
        return;
    }
    GMatrix2D savedMatrix;
    GRenderer::Cxform savedCx;
    ctx.PreDisplay(this, &savedMatrix, &savedCx);
    pDef->Display(ctx, this);
    ctx.PostDisplay(savedMatrix, savedCx);
}

// ---------------------------------------------------------------------------------------------
// The definition-side bodies.

void GFxCharacterDef::Display(GFxDisplayContext& ctx, GFxCharacter* ch)
{
    (void)ctx;
    (void)ch;
}

// The fill style, applied to the renderer. 2012 0xa909a0 GFxFillStyle::Apply, with
// GetFillTexture (0xa90950) for the bitmap and gradient arms.
static bool GFxDisplayApplyFill(GFxDisplayContext& ctx, const GFxFillStyle* fill,
                                GFxMovieDataDef* dataDef, GColor fallback,
                                const GRect<int>* bounds)
{
    GRenderer* r = ctx.pRenderer;
    if (r == 0)
        return false;
    if (fill == 0)
    {
        r->FillStyleColor(fallback);
        return true;
    }
    if (fill->IsImage())
    {
        GTexture* tex = 0;
        // DISHONORED(written, agent EA): a run-time fill from AS2's beginBitmapFill carries the
        // definition itself (GFxFillStyle::pDirectImage), because its BitmapData was resolved out of
        // an imported movie's export table and has no id in this dictionary.
        GFxCharacterDef* def = GFxDisplayNoImages ? 0 : (GFxCharacterDef*)fill->pDirectImage;
        if (def == 0)
        {
            def = (dataDef && !GFxDisplayNoImages)
                      ? dataDef->GetCharacterDefById(fill->ImageId) : 0;
        }
        if (def && def->GetResourceTypeCode() == GFxResource::RT_Image)
            tex = ((GFxImageCharacterDef*)def)->GetTexture(ctx.pRenderer, dataDef);
        if (tex)
        {
            GRenderer::FillTexture ft;
            ft.pTexture = tex;
            // The style's matrix maps the shape's twips into the image's own space at TWENTY units per
            // image pixel, which is SWF's bitmap-fill convention; GRenderer::FillTexture::TextureMatrix
            // maps a vertex position to 0..1 (agent CC's probe builds exactly that shape and the
            // checkerboard it produced is what confirmed it). So the style matrix is divided by
            // 20 * the texture's pixel size. Without the division the quad samples a fraction of one
            // texel and paints flat - which is what the first drawn frame did, on all 51 of them.
            // MEASURED, not assumed. The style's matrix maps the IMAGE's pixels onto the shape's
            // twips, and it already carries the twips factor: the menu's first bitmap fill is
            // [20 0 0 / 0 20 0] over bounds 0..10240 twips with a 512x512 image, i.e. 20 twips per
            // image pixel written into the matrix itself, and the second is [20 0 -7967 / 0 20 -548]
            // over bounds -7967..2273 x -548..412 with a 512x48 image, which lands exactly on 0..512
            // and 0..48. So the position-to-UV matrix the renderer wants (agent CC's probe proves that
            // convention) is the INVERSE divided by the image size in pixels - with no second factor of
            // 20. Both of the other two readings produce a quad that samples inside one texel and
            // paints flat, which is what the first three drawn frames did on all 51 of them.
            unsigned int imgW = 0, imgH = 0;
            ((GFxImageCharacterDef*)def)->GetImageSize(ctx.pRenderer, dataDef, &imgW, &imgH);
            // DISHONORED(bringup): the numbers the fill-matrix convention has to be derived from, for
            // the first few bitmap fills of a run.
            static unsigned int dumped = 0;
            if (dumped < 24 && bounds != 0)
            {
                ++dumped;
                GFxLogf("DISHONORED(bringup): GFx fill %u: matrix [%g %g %g / %g %g %g] bounds "
                        "[%d %d %d %d] image %ux%u '%s'%s",
                        dumped, fill->Matrix.M_[0][0], fill->Matrix.M_[0][1], fill->Matrix.M_[0][2],
                        fill->Matrix.M_[1][0], fill->Matrix.M_[1][1], fill->Matrix.M_[1][2],
                        bounds->Left, bounds->Top, bounds->Right, bounds->Bottom, imgW, imgH,
                        ((GFxImageCharacterDef*)def)->GetResolveName(),
                        ((GFxImageCharacterDef*)def)->bIsSubImage ? " (sub)" : "");
            }
            if (GFxDisplayFitFill && bounds != 0
                && bounds->Right > bounds->Left && bounds->Bottom > bounds->Top)
            {
                const float w = (float)(bounds->Right - bounds->Left);
                const float h = (float)(bounds->Bottom - bounds->Top);
                ft.TextureMatrix.SetIdentity();
                ft.TextureMatrix.M_[0][0] = 1.0f / w;
                ft.TextureMatrix.M_[0][2] = -(float)bounds->Left / w;
                ft.TextureMatrix.M_[1][1] = 1.0f / h;
                ft.TextureMatrix.M_[1][2] = -(float)bounds->Top / h;
            }
            else
            {
                GMatrix2D inverse;
                GFxDisplayMatrixInvert(&inverse, fill->Matrix);
                // The inverse lands in the image's own pixel space; BuildPixelToUVMatrix is what turns
                // that into the resolved texture's UVs, and for an atlas sub-image it is the only
                // thing that carries the rectangle's origin.
                GMatrix2D toUV;
                if (!((GFxImageCharacterDef*)def)->BuildPixelToUVMatrix(ctx.pRenderer, dataDef, &toUV))
                    toUV.SetIdentity();
                GFxDisplayMatrixAppend(&ft.TextureMatrix, toUV, inverse);
            }
            ft.WrapMode = (fill->Type == GFxFill_TiledImage || fill->Type == GFxFill_TiledSmoothImage)
                              ? GRenderer::Wrap_Repeat : GRenderer::Wrap_Clamp;
            ft.SampleMode = (fill->Type == GFxFill_TiledSmoothImage
                             || fill->Type == GFxFill_ClippedSmoothImage
                             || fill->Type == GFxFill_TiledImage
                             || fill->Type == GFxFill_ClippedImage)
                                ? GRenderer::Sample_Linear : GRenderer::Sample_Point;
            r->FillStyleBitmap(&ft);
            if (GFxDisplayDrawTrace != 0)
            {
                GFxStrCopy(GFxDisplayLastFill, sizeof(GFxDisplayLastFill),
                           ((GFxImageCharacterDef*)def)->GetResolveName());
            }
            ++ctx.Stats.Images;
            return true;
        }
        // DISHONORED(bringup): an image fill the renderer cannot texture draws NOTHING. The null
        // bitmap id (0xFFFF) is 23 of the menu's 61 bitmap fills and it is what the vignette is made
        // of; painting those with the style's flat colour laid seven opaque rectangles over the whole
        // interface. Retail has no such case, because a fill it cannot resolve fails the bind.
        // DISHONORED(bringup, agent EA): which picture. A counter says a fill drew nothing; a name
        // says whether the image is absent from the cook, unresolvable through the loader, or a
        // sub-image of an atlas whose base failed. One line per distinct id.
        {
            static unsigned int Reported[32];
            static unsigned int ReportedCount = 0;
            bool seen = false;
            for (unsigned int i = 0; i < ReportedCount && !seen; ++i)
                seen = Reported[i] == fill->ImageId;
            if (!seen && ReportedCount < 32)
            {
                Reported[ReportedCount++] = fill->ImageId;
                GFxLogf("DISHONORED(bringup): GFx fill NOT TEXTURED: image id %u '%s' "
                        "(export '%s' file '%s') type 0x%x def %s bounds [%d %d %d %d]",
                        fill->ImageId,
                        def ? ((GFxImageCharacterDef*)def)->GetResolveName() : "<no def>",
                        def ? ((GFxImageCharacterDef*)def)->ExportName : "",
                        def ? ((GFxImageCharacterDef*)def)->FileName : "",
                        (unsigned int)fill->Type, def ? def->GetDefTypeName() : "<none>",
                        bounds ? bounds->Left : 0, bounds ? bounds->Top : 0,
                        bounds ? bounds->Right : 0, bounds ? bounds->Bottom : 0);
            }
        }
        r->FillStyleDisable();
        ++GFxDisplayUntexturedFills;
        return false;
    }
    if (fill->IsGradient())
    {
        // DISHONORED(bringup): the gradient texture GFxFillStyle::GetGradientFillTexture (0xa90600)
        // builds through GFxGradientParams is not generated here; the middle gradient stop's colour
        // stands in, which keeps the shape's area and its rough colour. Named in agentDC.md.
        const unsigned int mid = fill->GradientCount ? fill->GradientCount / 2 : 0;
        r->FillStyleColor(fill->GradientCount ? fill->Gradient[mid].Color : fill->Color);
        return true;
    }
    r->FillStyleColor(fill->Color);
    if (GFxDisplayDrawTrace != 0) GFxStrCopy(GFxDisplayLastFill, sizeof(GFxDisplayLastFill), "solid");
    return true;
}

// DISHONORED(bringup): -gfxuidrawtrace logs every style group a frame submits, with the screen
// rectangle it covers and the fill it was given. "Which draw is that rectangle" is otherwise a
// question only a bisect can answer, and it took six runs the first time.

// One mesh group, submitted. XY16i in twips with an ascending index list, which is the only geometry
// shape the renderer's vertex declarations accept (see GetVertices in GFxDisplay.h).
static void GFxDisplaySubmitGroup(GFxDisplayContext& ctx, const GFxShapeMesh* mesh,
                                  const GFxShapeMesh::Group& group)
{
    GRenderer* r = ctx.pRenderer;
    if (r == 0 || group.VertexCount < 3)
        return;
    if (GFxDisplayDrawTrace != 0)
    {
        const short* v = mesh->GetVertices() + group.VertexStart * 2;
        float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
        for (unsigned int i = 0; i < group.VertexCount; ++i)
        {
            float px = (float)v[i * 2], py = (float)v[i * 2 + 1];
            ctx.Matrix.Transform(&px, &py);
            px *= 0.05f; py *= 0.05f;
            if (px < x0) x0 = px;
            if (py < y0) y0 = py;
            if (px > x1) x1 = px;
            if (py > y1) y1 = py;
        }
        GFxLogf("DISHONORED(bringup): draw %u tris, box (%.0f,%.0f)-(%.0f,%.0f), alpha %.2f*x+%.0f, "
                "fill %s", group.VertexCount / 3, x0 * 20.f, y0 * 20.f, x1 * 20.f, y1 * 20.f,
                ctx.Cx.M_[3][0], ctx.Cx.M_[3][1], GFxDisplayLastFill);
        --GFxDisplayDrawTrace;
    }
    const unsigned short* indices = GFxDisplayGetLinearIndices(group.VertexCount);
    if (indices == 0)
        return;
    r->SetVertexData(mesh->GetVertices() + group.VertexStart * 2, (int)group.VertexCount,
                     GRenderer::Vertex_XY16i, 0);
    r->SetIndexData(indices, (int)group.VertexCount, GRenderer::Index_16, 0);
    r->DrawIndexedTriList(0, 0, (int)group.VertexCount, 0, (int)(group.VertexCount / 3));
    ++ctx.Stats.TriListDraws;
    ctx.Stats.Triangles += group.VertexCount / 3;
}

void GFxShapeCharacterDef::Display(GFxDisplayContext& ctx, GFxCharacter* ch)
{
    // DISHONORED(port): 2012 0xa3cf40 -> 0xa3bb80 -> GFxMeshSet::Display 0xab4a60. The mesh is built
    // once per definition (the cache above) and the current matrix is what places it.
    (void)ch;
    ++ctx.Stats.Shapes;
    if (GFxDisplayNoShapes)
        return;
    GFxShapeMesh* mesh = GFxDisplayGetShapeMesh(this);
    if (mesh == 0 || mesh->GetGroupCount() == 0)
    {
        ++ctx.Stats.NoGeometry;
        return;
    }
    ctx.ApplyToRenderer();
    GFxMovieDataDef* dataDef = ctx.pDataDef ? ctx.pDataDef
                                            : (ctx.pDefImpl ? ctx.pDefImpl->GetDataDef() : 0);
    for (unsigned int g = 0; g < mesh->GetGroupCount(); ++g)
    {
        const GFxShapeMesh::Group& group = mesh->GetGroup(g);
        if (group.LineStyle >= 0)
        {
            const GFxLineStyle* ls = GetLineStyle((unsigned int)group.LineStyle);
            ctx.pRenderer->FillStyleColor(ls ? ls->Color : GColor(255, 255, 255, 255));
        }
        else if (group.Style >= 0)
        {
            // A style group whose fill has no texture is NOT submitted: the renderer keeps the last
            // fill it was given, so drawing it anyway paints the previous shape's bitmap over this
            // one's area - which is how the vignette came to be painted with the DISHONORED logo.
            if (!GFxDisplayApplyFill(ctx, GetFillStyle((unsigned int)group.Style), dataDef,
                                     GColor(255, 255, 255, 255), &Bounds))
            {
                continue;
            }
        }
        else
        {
            ctx.pRenderer->FillStyleColor(GColor(255, 255, 255, 255));
        }
        GFxDisplaySubmitGroup(ctx, mesh, group);
    }
    ctx.pRenderer->FillStyleDisable();
}

void GFxImageCharacterDef::Display(GFxDisplayContext& ctx, GFxCharacter* ch)
{
    // A bitmap placed directly on the timeline rather than through a shape's fill. gfxexport emits
    // both shapes and direct placements, so both paths exist.
    (void)ch;
    GFxMovieDataDef* dataDef = ctx.pDataDef ? ctx.pDataDef
                                            : (ctx.pDefImpl ? ctx.pDefImpl->GetDataDef() : 0);
    GTexture* tex = GFxDisplayNoImages ? 0 : GetTexture(ctx.pRenderer, dataDef);
    if (tex == 0 || ctx.pRenderer == 0)
    {
        ++ctx.Stats.NoGeometry;
        return;
    }
    ++ctx.Stats.Images;

    const float w = (float)(TargetWidth ? TargetWidth : 1) * GFxPixelsToTwips;
    const float h = (float)(TargetHeight ? TargetHeight : 1) * GFxPixelsToTwips;
    GRenderer::FillTexture ft;
    ft.pTexture = tex;
    // The quad below is this reference's own pixel size in twips, so the UV map is the twips-to-pixels
    // divide composed with the pixel-to-UV map - which for an atlas sub-image is the whole of the
    // difference between drawing this rectangle and drawing the sheet's top-left corner.
    GMatrix2D twipsToPixels;
    twipsToPixels.SetIdentity();
    twipsToPixels.M_[0][0] = 1.0f / GFxPixelsToTwips;
    twipsToPixels.M_[1][1] = 1.0f / GFxPixelsToTwips;
    GMatrix2D pixelsToUV;
    if (!BuildPixelToUVMatrix(ctx.pRenderer, dataDef, &pixelsToUV))
    {
        pixelsToUV.SetIdentity();
        pixelsToUV.M_[0][0] = TargetWidth ? 1.0f / (float)TargetWidth : 1.0f;
        pixelsToUV.M_[1][1] = TargetHeight ? 1.0f / (float)TargetHeight : 1.0f;
    }
    GFxDisplayMatrixAppend(&ft.TextureMatrix, pixelsToUV, twipsToPixels);
    ft.WrapMode = GRenderer::Wrap_Clamp;
    ft.SampleMode = GRenderer::Sample_Linear;

    short verts[12];
    const short sw = GFxDisplayToTwip16(w);
    const short sh = GFxDisplayToTwip16(h);
    verts[0] = 0;  verts[1] = 0;
    verts[2] = sw; verts[3] = 0;
    verts[4] = sw; verts[5] = sh;
    verts[6] = 0;  verts[7] = 0;
    verts[8] = sw; verts[9] = sh;
    verts[10] = 0; verts[11] = sh;

    const unsigned short* indices = GFxDisplayGetLinearIndices(6);
    if (indices == 0)
        return;
    ctx.ApplyToRenderer();
    ctx.pRenderer->FillStyleBitmap(&ft);
    ctx.pRenderer->SetVertexData(verts, 6, GRenderer::Vertex_XY16i, 0);
    ctx.pRenderer->SetIndexData(indices, 6, GRenderer::Index_16, 0);
    ctx.pRenderer->DrawIndexedTriList(0, 0, 6, 0, 2);
    ctx.pRenderer->FillStyleDisable();
    ++ctx.Stats.TriListDraws;
    ctx.Stats.Triangles += 2;
}

void GFxButtonCharacterDef::Display(GFxDisplayContext& ctx, GFxCharacter* ch)
{
    // DISHONORED(bringup): the button's up state, drawn directly. Retail instantiates the state's
    // records as children of a GFxButtonCharacter (38 functions, plus GFx_GenerateMouseButtonEvents
    // 0xa66a90) and the state machine picks which record set to show; without the mouse path there is
    // one state to show and it is the up state, which is what the menu's buttons look like at rest.
    (void)ch;
    ++ctx.Stats.Buttons;
    GFxMovieDataDef* dataDef = ctx.pDataDef ? ctx.pDataDef
                                            : (ctx.pDefImpl ? ctx.pDefImpl->GetDataDef() : 0);
    if (dataDef == 0)
        return;
    for (unsigned int i = 0; i < RecordCount; ++i)
    {
        const GFxButtonRecord& rec = Records[i];
        if ((rec.StateFlags & GFxButtonRecord::State_Up) == 0)
            continue;
        GFxCharacterDef* def = dataDef->GetCharacterDefById(rec.CharacterId);
        if (def == 0 || def == this)
            continue;
        const GMatrix2D savedMatrix = ctx.Matrix;
        const GRenderer::Cxform savedCx = ctx.Cx;
        GFxDisplayMatrixAppend(&ctx.Matrix, savedMatrix, rec.Matrix);
        GFxDisplayCxformConcat(&ctx.Cx, savedCx, rec.ColorTransform);
        def->Display(ctx, 0);
        ctx.Matrix = savedMatrix;
        ctx.Cx = savedCx;
    }
}

// ---------------------------------------------------------------------------------------------
// The text field. 2012 0xa2ed90 -> GFxTextDocView::Display 0xaa04f0 -> GFxTextLineBuffer::Display
// 0xa45bf0. Agent CB's ProduceGlyphs is the same traversal with the submission removed; this is the
// submission, in the shape CB's hand-over specified: one DrawBitmaps per atlas and per transform,
// Coords in the space the matrix maps, TextureCoords normalised.

void GFxEditTextCharacter::Display(GFxDisplayContext& ctx)
{
    ++ctx.Stats.Characters;
    if (!GetVisible() || ctx.pRenderer == 0)
    {
        if (!GetVisible())
            ++ctx.Stats.Invisible;
        return;
    }
    ++ctx.Stats.TextFields;
    if (TextValue[0] == 0 || strcmp(TextValue, "Text") == 0)
        ++ctx.Stats.TextFieldsUnbound;
    if (GFxDisplayNoText)
        return;

    GMatrix2D savedMatrix;
    GRenderer::Cxform savedCx;
    ctx.PreDisplay(this, &savedMatrix, &savedCx);

    if (IsDirty())
        AdvanceFrame(false, 0.0f);

    GFxGlyphRasterCache* cache = ctx.pGlyphCache ? ctx.pGlyphCache : GFxDisplayGetGlyphCache();
    GFxTextLineBuffer& lines = GetDocView().GetLineBuffer();
    const GRect<float> viewRect = GetDocView().GetViewRect();

    // DISHONORED(port): the field's filter list, folded into one GFxTextFilter by SetFilters
    // (0xa275b0). A shadow is a whole extra batch of the same glyphs, rasterised blurred into their
    // own atlas slots, drawn at the filter's twips offset in the filter's colour and drawn *first*.
    const GFxTextFilter& filter = GetTextFilter();
    const bool bShadow = filter.HasShadow() && !GFxDisplayNoTextShadow;

    // Rasterise every glyph of the field first, so the atlas is complete before it is uploaded: the
    // upload is one call per frame, not one per line.
    GRenderer::BitmapDesc descs[192];
    unsigned int descCount = 0;

    // pass 0 fills the atlas, pass 1 submits the shadow, pass 2 the text. The atlas is uploaded once,
    // between pass 0 and pass 1.
    for (unsigned int pass = 0; pass < 3; ++pass)
    {
        const bool bShadowPass = (pass == 1);
        if (bShadowPass && !bShadow)
            continue;
        if (pass == 1 || (pass == 2 && !bShadow))
        {
            ctx.pGlyphTexture = GFxDisplayGetGlyphTexture(ctx.pRenderer);
            if (ctx.pGlyphTexture == 0)
                break;
            ctx.pRenderer->SetMatrix(ctx.Matrix);
            ctx.pRenderer->SetCxform(ctx.Cx);
        }
        if (pass == 2 && descCount)
        {
            // The shadow batch is closed before the first glyph of the text, so the two batches
            // cannot interleave.
            ctx.pRenderer->DrawBitmaps(descs, (int)descCount, 0, (int)descCount, ctx.pGlyphTexture,
                                       ctx.Matrix, 0);
            ++ctx.Stats.GlyphDraws;
            descCount = 0;
        }

        GImage* atlas = cache->GetTextureCount() ? cache->GetTexture(0) : 0;
        const float atlasW = atlas ? (float)atlas->Width : 1.0f;
        const float atlasH = atlas ? (float)atlas->Height : 1.0f;

        for (unsigned int li = 0; li < lines.GetLineCount(); ++li)
        {
            GFxTextLineBuffer::Line* line = lines.GetLine(li);
            if (line == 0)
                continue;
            float pen = 0.0f;
            for (unsigned int gi = 0; gi < line->Glyphs.GetSize(); ++gi)
            {
                const GFxTextLineBuffer::GlyphEntry& g = line->Glyphs[gi];
                if (g.Flags & GFxTextLineBuffer::GlyphEntry::EF_Newline)
                    continue;
                const float advance = (float)g.Advance;
                if (g.GlyphIndex == ~0u)
                {
                    pen += advance;
                    continue;
                }
                GFxFontResource* font = 0;
                GColor colour(255, 255, 255, 255);
                if (g.FormatIndex < line->Formats.GetSize())
                {
                    font = line->Formats[g.FormatIndex].pFont;
                    colour = line->Formats[g.FormatIndex].Color;
                }
                if (font == 0)
                {
                    pen += advance;
                    continue;
                }

                GFxGlyphParam p;
                p.pFont = font;
                p.GlyphIndex = g.GlyphIndex;
                const float sizePx = g.GetFontSize();
                p.FontSize = (unsigned char)(sizePx < 1.0f ? 1.0f
                                             : (sizePx > 255.0f ? 255.0f : sizePx));
                // pass 0 has to rasterise both variants, or the shadow's slot is missing from the
                // atlas the frame it is first asked for.
                const unsigned int variants = (pass == 0 && bShadow) ? 2u : 1u;
                const GFxGlyphNode* node = 0;
                for (unsigned int v = 0; v < variants; ++v)
                {
                    const bool bShadowVariant = bShadowPass || (pass == 0 && v == 1);
                    GFxGlyphParam q = p;
                    if (bShadowVariant)
                    {
                        q.Flags |= (unsigned char)GFxGlyphParam::GPF_Shadow;
                        q.BlurX = filter.ShadowBlurX;
                        q.BlurY = filter.ShadowBlurY;
                        q.Strength = filter.ShadowStrength;
                    }
                    else
                    {
                        q.BlurX = filter.BlurX;
                        q.BlurY = filter.BlurY;
                        q.Strength = filter.BlurStrength;
                    }
                    if (q.BlurX != 0 || q.BlurY != 0)
                        q.Flags |= (unsigned char)GFxGlyphParam::GPF_Blur;
                    node = cache->GetGlyph(q);
                }
                if (node == 0 || node->Width == 0 || node->Height == 0)
                {
                    pen += advance;
                    continue;
                }
                if (pass == 0)
                {
                    pen += advance;
                    continue;
                }

                // The pen is at (line.OffsetX + pen, line.OffsetY + baseline) in twips, relative to
                // the view rect; the node's origin is in pixels relative to the pen with y upwards.
                const float penX = viewRect.Left + (float)line->OffsetX + pen;
                const float penY = viewRect.Top + (float)line->OffsetY + line->BaselineOffset;
                float left = penX + node->OriginX * GFxPixelsToTwips;
                float top = penY - node->OriginY * GFxPixelsToTwips;
                if (bShadowPass)
                {
                    // GFxTextFilter::UpdateShadowOffset (0xa22f30) keeps the offset in twips, which
                    // is the space these coordinates are in.
                    left += (float)filter.ShadowOffsetX;
                    top += (float)filter.ShadowOffsetY;
                    colour = GColor(filter.ShadowColor.Channels.Red,
                                    filter.ShadowColor.Channels.Green,
                                    filter.ShadowColor.Channels.Blue, filter.ShadowAlpha);
                }
                descs[descCount].Coords.Left = left;
                descs[descCount].Coords.Top = top;
                descs[descCount].Coords.Right = left + node->Width * GFxPixelsToTwips;
                descs[descCount].Coords.Bottom = top + node->Height * GFxPixelsToTwips;
                descs[descCount].TextureCoords.Left = (float)node->X / atlasW;
                descs[descCount].TextureCoords.Top = (float)node->Y / atlasH;
                descs[descCount].TextureCoords.Right = (float)(node->X + node->Width) / atlasW;
                descs[descCount].TextureCoords.Bottom = (float)(node->Y + node->Height) / atlasH;
                descs[descCount].Color = colour;
                ++descCount;
                if (bShadowPass)
                    ++ctx.Stats.ShadowGlyphs;
                else
                    ++ctx.Stats.Glyphs;
                pen += advance;

                if (descCount == 192)
                {
                    ctx.pRenderer->DrawBitmaps(descs, (int)descCount, 0, (int)descCount,
                                               ctx.pGlyphTexture, ctx.Matrix, 0);
                    ++ctx.Stats.GlyphDraws;
                    descCount = 0;
                }
            }
        }
    }

    if (descCount && ctx.pGlyphTexture)
    {
        ctx.pRenderer->DrawBitmaps(descs, (int)descCount, 0, (int)descCount, ctx.pGlyphTexture,
                                   ctx.Matrix, 0);
        ++ctx.Stats.GlyphDraws;
    }

    ctx.PostDisplay(savedMatrix, savedCx);
}

// ---------------------------------------------------------------------------------------------
// GFxMovieRoot::Display. 2012 0xa07aa0 (2013 0x9fe550).

void GFxMovieRoot::Display()
{
    // DISHONORED(port): retail's body sets up the display context from the state bag's render config,
    // installs the viewport matrix, walks the levels and then draws the focus rect
    // (DisplayFocusRect 0xa066f0) and the topmost level characters (0xa02ec0).
    bDirty = false;
    if (pLevel0 == 0)
        return;

    GRenderer* renderer = 0;
    GFxState* s = GetStateAddRef(GFxState::State_RenderConfig);
    if (s)
    {
        renderer = ((GFxRenderConfig*)s)->pRenderer.GetPtr();
        s->Release();
    }
    if (renderer == 0)
        return;

    GFxDisplayContext ctx;
    ctx.pRenderer = renderer;
    ctx.pRoot = this;
    ctx.pDefImpl = pDefImpl;
    ctx.pGlyphCache = GFxDisplayGetGlyphCache();

    // The root matrix: twips to the movie's own pixel frame. The renderer's BeginDisplay has already
    // set the viewport matrix that maps that frame to the viewport (FGFxRenderer::BeginDisplay, 2012
    // 0x59dd90), so all this contributes is the twips scale.
    ctx.Matrix.SetIdentity();
    ctx.Matrix.M_[0][0] = GFxTwipsToPixels;
    ctx.Matrix.M_[1][1] = GFxTwipsToPixels;
    GFxDisplayCxformIdentity(&ctx.Cx);

    // The visible frame rect, in the movie's own pixels: what BeginDisplay's x0..y1 are. Retail keeps
    // it as four floats on the movie root (this+36..39 in the decompile of 0xa07aa0) and recomputes it
    // in SetViewport from the scale mode. SM_ShowAll keeps the aspect ratio and shows *more* than the
    // frame on the long axis, which is what the four numbers below are.
    // DISHONORED(port, agent DG): the same rectangle the mouse is mapped back through, so the two
    // halves cannot disagree about where the movie is on screen. GFxInput.cpp.
    float x0, y0, x1, y1;
    GetVisibleFrameRectPixels(&x0, &y0, &x1, &y1);

    GColor background = BackgroundColor;
    background.SetAlpha((GUByte)(BackgroundAlpha * 255.0f + 0.5f));

    // DISHONORED(bringup, agent EX): the stage-to-viewport mapping, logged whenever it changes, so
    // "the movie is magnified" is a measurement rather than an inference. It was measured correct -
    // stage 1280x720, SM_ShowAll, viewport 1280x720, visible 0,0..1280,720, root matrix identity -
    // which is what ruled the stage out as the cause of wave 16's blur.
    {
        static char GFxEXMapLast[512] = {0};
        char line[512];
        const GMatrix2D& rootMatrix = pLevel0->GetMatrix();
        _snprintf(line, sizeof(line),
                  "movie %gx%g scaleMode %d align %d viewport buf %dx%d rect %d,%d %dx%d "
                  "visible %g,%g..%g,%g root [%g %g %g / %g %g %g]",
                  pDefImpl ? pDefImpl->GetWidth() : -1.f, pDefImpl ? pDefImpl->GetHeight() : -1.f,
                  (int)ScaleMode, (int)Alignment,
                  Viewport.BufferWidth, Viewport.BufferHeight, Viewport.Left, Viewport.Top,
                  Viewport.Width, Viewport.Height, x0, y0, x1, y1,
                  rootMatrix.M_[0][0], rootMatrix.M_[0][1], rootMatrix.M_[0][2],
                  rootMatrix.M_[1][0], rootMatrix.M_[1][1], rootMatrix.M_[1][2]);
        if (strcmp(line, GFxEXMapLast) != 0)
        {
            GFxStrCopy(GFxEXMapLast, sizeof(GFxEXMapLast), line);
            GFxLogf("DISHONORED(bringup): GFx stage map: %s", line);
        }
    }

    if (!GFxDisplayNoBeginDisplay)
        renderer->BeginDisplay(background, Viewport, x0, x1, y0, y1);
    ctx.pRenderer->SetCxform(ctx.Cx);
    pLevel0->Display(ctx);
    if (!GFxDisplayNoBeginDisplay)
        renderer->EndDisplay();

    LastDisplayStats = ctx.Stats;
}

void GFxMovieRoot::DisplayPrePass()
{
    // 2012 0xa08090. The pre-pass exists for the filter render targets
    // (GFxDisplayContext::DisplayFilterPrePass 0xa605e0); nothing in this cook keeps a live filter
    // list after the strip, so there is nothing to pre-render.
}
