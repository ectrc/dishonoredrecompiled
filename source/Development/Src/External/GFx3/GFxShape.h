// GFx 3.3 shape geometry: the SWF shape record, the flattened compound shape the rasteriser and the
// tessellator both consume, and the fill/line styles.
//
// This unit exists for the glyph rasteriser. Agent BB established (agentBB.md 3.4) that the game's
// fonts are DefineFont3 glyph *outlines* with no font-texture tag anywhere in the cook, so the path
// from a cooked movie to a visible character of text runs
//
//   DefineFont3 tag -> GFxFontData -> per-glyph GFxConstShapeNoStyles (a SWF SHAPE record)
//     -> GFxShapeBase::MakeCompoundShape -> GCompoundShape (quadratics flattened to line segments)
//     -> GRasterizer::AddShapeScaled -> coverage scanlines -> GImage -> the glyph atlas.
//
// Retail's classes, with the functions this unit ports named at their 2012 rvas:
//   GFxShapeBase              30 fns; MakeCompoundShapeImpl<GFxSwfPathData> 0xa40490
//   GFxConstShapeNoStyles     12 fns; Read 0xa42ab0 / 0xa43de0
//   GFx_ReadFillStyles / GFx_ReadLineStyles  0xa429c0 / 0xa41270, the no-style-owner arm only
//   GFxSwfPathData::PathsIterator  8 fns; PathsIterator 0xa3d110, ReadNext 0xa3a7f0,
//                             AddForTessellation 0xa3fb50
//   GFxSwfPathData::EdgesIterator  5 fns; GetEdge 0xa3b380, GetPlainEdge 0xa3b4f0
//   GCompoundShape            14 fns; BeginPath 0xa5a010 / 0xa5a0a0, AddCurve 0xa5a410,
//                             ClosePath 0xa5a120, flattenQuadraticCurve 0xa5a1a0,
//                             ScaleAndTranslate 0xa59830, PerceiveBounds 0xa59750,
//                             SetCurveTolerance 0xa59730, Clear 0xa59c90
//
// DEVIATION, stated once and deliberately (agentBC.md 0: the 2012 PDB carries no layout for any
// runtime-internal GFx type, so no offset here is reproducible and none is asserted): retail's
// GFxConstShapeNoStyles keeps the *raw* SWF shape bytes and decodes them lazily through
// GFxSwfPathData::PathsIterator on every traversal, with the path and shape counts appended to the
// blob (GFxSwfPathData::GetShapeAndPathCounts 0xa3b5c0 reads them off the end). This decodes the
// same records once, at load, into GFxShapePath/Edge. The decoded *semantics* are retail's, read out
// of ReadNext and GetEdge; only the moment of decoding differs, and nothing here is ABI-visible.
#ifndef INC_GFXSHAPE_H
#define INC_GFXSHAPE_H

#include "GFx3.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

class GFxStream;
class GCompoundShape;

// ---------------------------------------------------------------------------------------------
// One edge of a path. Retail's GFxSwfPathData::EdgesIterator::Edge is the same four coordinates
// plus the curve flag GetEdge (0xa3b380) writes: a straight edge leaves the control point equal to
// the anchor and clears the flag.
struct GFxShapeEdge
{
    float Cx, Cy;   // quadratic control point, in the shape's own coordinate units
    float Ax, Ay;   // anchor (the edge's end point)
    bool  Curve;
};

// One path: a contour with a fill style on each side and an optional line style. Style indices are
// 1-based in the SWF record and are stored here already decremented, which is what
// AddForTessellation (0xa3fb50) passes to GCompoundShape::BeginPath: `*(this+5) - 1` etc, so -1
// means "no style".
struct GFxShapePath
{
    int   LeftStyle;
    int   RightStyle;
    int   LineStyle;
    float MoveX, MoveY;
    unsigned int EdgeStart;
    unsigned int EdgeCount;
};

// ---------------------------------------------------------------------------------------------
// GFxShapeBase: the refcounted geometry base. Retail's is 44 bytes with the cached bound in the
// first four floats and a flag byte at +36 whose bit 1 (0x02) means "DefineFont3 coordinates", i.e.
// the glyph EM square is 1024 * 20 units and everything must be multiplied by 0.05 before use. That
// is exactly what GFxSwfPathData::PathsIterator's constructor (0xa3d110) does:
//     if (shape->Flags & 2) { Scale = 0.05f; ... } else Scale = 1.0f;
// and AddForTessellation then scales the move-to and every edge by it. It is the single most
// important constant in the font path and it is measured, not assumed.
class GFxShapeBase : public GRefCountBase<GFxShapeBase, 44>
{
public:
    enum Flags
    {
        SF_ShapeHasStyles   = 0x01,
        SF_TwentyTimesScale = 0x02,   // DefineFont3: coordinates are 20x the 1024-EM units
        SF_ShapeIsClosed    = 0x08
    };

    GFxShapeBase();
    virtual ~GFxShapeBase();

    virtual void MakeCompoundShape(GCompoundShape* out, float tolerance) const;   // 0xa40490
    virtual void ComputeBound(GRect<float>* out) const;                           // 0xa3f320
    virtual void GetShapeAndPathCounts(unsigned int* shapes, unsigned int* paths) const;
    virtual const char* GetShapeTypeName() const { return "ShapeBase"; }

    GRect<float> GetRectBoundsLocal() const;                                      // 0xa3cf10

    float GetCoordScale() const { return (Flags & SF_TwentyTimesScale) ? 0.05f : 1.0f; }

    unsigned int GetPathCount() const { return Paths.GetSize(); }
    const GFxShapePath& GetPath(unsigned int i) const { return Paths[i]; }
    const GFxShapeEdge& GetEdge(unsigned int i) const { return Edges[i]; }

    GArray<GFxShapePath> Paths;
    GArray<GFxShapeEdge> Edges;
    unsigned int ShapeCount;      // how many StateNewStyles groups the record carried
    // How many of those groups declared style arrays this no-style shape cannot own. Retail logs
    // "GFx_ReadFillStyles, trying to read %d fillstyles into no-style shape" (0xa429c0) and reads no
    // further; counting it here is what lets the harness show the number is zero over the whole cook
    // rather than assuming it.
    unsigned int StyleRecordsRefused;
    unsigned char Flags;

protected:
    mutable GRect<float> CachedBound;
    mutable bool         BoundValid;
};

// ---------------------------------------------------------------------------------------------
// GFxConstShapeNoStyles: the shape a glyph is. "NoStyles" because a DefineFont glyph's SHAPE record
// carries no style arrays at all - one implicit fill, fillBits = lineBits = 1 - which is why the
// font reader (GFxFontData::Read 0xa587d0) constructs this class and not GFxShapeCharacterDef.
class GFxConstShapeNoStyles : public GFxShapeBase
{
public:
    GFxConstShapeNoStyles() {}

    virtual const char* GetShapeTypeName() const { return "ConstShapeNoStyles"; }

    // 0xa42ab0 (the 7-argument form; 0xa43de0 forwards to it with both style arrays null).
    // `tagType` selects the record dialect exactly as retail passes it: DefineShape (2) for a
    // DefineFont glyph, DefineShape2 (22) for DefineFont2, and the DefineFont3 code (75) itself for
    // a DefineFont3 glyph, which is what sets SF_TwentyTimesScale.
    bool Read(GFxStream* s, unsigned int tagType, unsigned int endPos);
};

// ---------------------------------------------------------------------------------------------
// GCompoundShape: the flattened form. Every quadratic is subdivided to line segments against a
// curve tolerance, so what comes out is pure polygons - which is all GRasterizer and the
// tessellator can take.
// The deepest subdivision GCompoundShape::flattenQuadraticCurve has reached since the counter was
// last reset, so the depth cap can be shown to be a safety net rather than a truncation. Retail
// has no cap at all (0xa5a1a0 recurses until the collinearity test stops it).
unsigned int GFxShapeGetMaxFlattenDepth();
void         GFxShapeResetMaxFlattenDepth();

class GCompoundShape
{
public:
    // One flattened contour. Retail's SPath is 24 bytes: owner, vertex count, vertex start, left,
    // right, line (GCompoundShape::BeginPath 0xa5a010 writes exactly those six words).
    struct SPath
    {
        unsigned int VertexStart;
        unsigned int VertexCount;
        int LeftStyle;
        int RightStyle;
        int LineStyle;
    };

    GCompoundShape();

    void Clear();                                                  // 0xa59c90
    void SetCurveTolerance(float t);                               // 0xa59730
    float GetCurveTolerance() const { return CurveTolerance; }

    void BeginPath(int left, int right, int line);                 // 0xa5a010
    void BeginPath(int left, int right, int line, float x, float y); // 0xa5a0a0
    void AddVertex(float x, float y);
    void AddCurve(float cx, float cy, float ax, float ay);         // 0xa5a410
    void ClosePath();                                              // 0xa5a120

    void ScaleAndTranslate(float sx, float sy, float tx, float ty);  // 0xa59830
    void PerceiveBounds(float* l, float* t, float* r, float* b) const; // 0xa59750

    unsigned int GetPathCount() const { return Paths.GetSize(); }
    const SPath& GetPath(unsigned int i) const { return Paths[i]; }
    const GPoint<float>& GetVertex(unsigned int i) const { return Vertices[i]; }
    unsigned int GetVertexCount() const { return Vertices.GetSize(); }
    int GetMinStyle() const { return MinStyle; }
    int GetMaxStyle() const { return MaxStyle; }

private:
    void flattenQuadraticCurve(float x0, float y0, float cx, float cy, float ax, float ay,
                               int depth);                          // 0xa5a1a0

    GArray<GPoint<float> > Vertices;
    GArray<SPath>          Paths;
    float CurveTolerance;
    float ToleranceSq;      // (CurveTolerance * 0.25f)^2 - SetCurveTolerance 0xa59730
    int   MinStyle;
    int   MaxStyle;
    int   Current;          // index into Paths, -1 when no path is open
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXSHAPE_H
