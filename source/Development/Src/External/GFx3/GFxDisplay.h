// GFx 3.3 the display half: the display-list traversal, the shape mesh, and the glyph submission.
//
// This is the loop every other GFx package stopped at. Agent BC left GFxMovieRoot::Display empty
// (agentBC.md 6.9), agent CB left GFxEditTextCharacter::ProduceGlyphs as the traversal with the
// submission removed (agentCB.md 9), and agent CC built the whole 54-slot GRenderer and said the
// characters were not its package (agentCC.md 5). What was missing between them is here: a
// GFxDisplayContext, a walk of the display list that composes the transform and the colour transform
// and handles the stencil masks, a triangle mesh per shape definition, and the DrawBitmaps call per
// text field.
//
// Retail's classes and the functions each body is written from:
//   GFxDisplayContext            Init 0xa5f3c0, ctor 0xa5f9b0, PreDisplay 0xa5f5d0, PostDisplay
//                                0xa5f800, PushAndDrawMask 0xa5f510, PopMask 0xa5f5a0
//   GFxMovieRoot::Display        0xa07aa0 (2013 0x9fe550)
//   GFxDisplayList::Display      0x9d56c0
//   GFxSprite::Display           0x9faeb0 (2013 0x9f1780)
//   GFxShapeBase::Display        0xa3cf40 / 0xa3bb80
//   GFxMesh::Display             0xab2b60, GFxMeshSet::Display 0xab4a60
//   GFxEditTextCharacter::Display 0xa2ed90, GFxTextLineBuffer::Display 0xa45bf0
//   GFxFillStyle::Apply          0xa909a0, GetFillTexture 0xa90950, GetImageInfo 0xa8fda0
//
// DEVIATION, the one large one, stated here and at the site. Retail tessellates a shape with
// GTessellator (0xabdbb0 .. 0xac5930, about sixty functions: a sweep-line monotone decomposition with
// intersection events, coherent-curve triangulation and GFxEdgeAAGenerator's edge-antialiasing pass)
// and caches the result in a GFxMeshSet keyed by scale. This unit tessellates with a trapezoidal
// decomposition of its own - the bands are the union of the vertex Y values and the edge-edge
// intersection Y values, and each band's inside spans come from the non-zero winding rule, which is
// SWF's own fill rule - and caches one mesh per shape definition in shape-local twips. The observable
// geometry is the same closed region with the same fill styles; what is not reproduced is retail's
// triangle *count* and ordering, its edge antialiasing, and its per-scale retessellation. Strokes are
// quads per segment with round-ish joins rather than GStrokerAA's join and cap model
// (GFxCachedStroke::Display 0xab36e0). Both are named in agentDC.md as the remaining 1:1 work.
#ifndef INC_GFX3_GFXDISPLAY_H
#define INC_GFX3_GFXDISPLAY_H

#include "GFxPlayer.h"
#include "GFxCharacterDefs.h"

class GFxGlyphRasterCache;

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

// ---------------------------------------------------------------------------------------------
// One tessellated shape definition: triangles grouped by the fill style that paints them, in the
// shape's own twips. Retail's GFxMeshSet with one GFxMesh per style.
class GFxShapeMesh
{
public:
    struct Group
    {
        int          Style;          // index into the definition's fill styles, -1 for a stroke
        int          LineStyle;      // index into the line styles when Style is -1
        unsigned int VertexStart;    // into Vertices, in pairs of floats
        unsigned int VertexCount;
    };

    GFxShapeMesh();
    ~GFxShapeMesh();

    // 0xa3fe20's job: flatten, split by style, triangulate. `record` is CD's decoded SHAPE record and
    // `def` its style arrays; a glyph outline has no styles and paints with one implicit solid fill.
    void Build(const GFxShapeRecord& record, const GFxShapeCharacterDef* def);

    unsigned int GetGroupCount() const { return GroupCount; }
    const Group& GetGroup(unsigned int i) const { return Groups[i]; }
    // Two shorts per vertex, in twips. DISHONORED(layout): the renderer has no vertex declaration for
    // GRenderer::Vertex_XY32f (EGFxVertexDeclarationType in gfxuirendererimpl.h has None, Strip, Glyph,
    // XY16iC32 and XY16iCF32 and nothing else), because retail's meshes are 16-bit integer twips and a
    // 1280x720 frame is 25600x14400 twips - inside int16. So the mesh is built in exactly that.
    const short* GetVertices() const { return Vertices; }
    unsigned int GetVertexCount() const { return VertexCount; }
    unsigned int GetTriangleCount() const { return VertexCount / 3; }
    bool IsBuilt() const { return bBuilt; }

private:
    void addTriangle(int style, int lineStyle, const float* a, const float* b, const float* c);
    void reserveVertices(unsigned int extra);

    Group*       Groups;
    unsigned int GroupCount, GroupCapacity;
    short*       Vertices;
    unsigned int VertexCount, VertexCapacity;
    bool         bBuilt;
};

// The ascending index list every mesh group is drawn with: DrawIndexedTriList needs indices and a mesh
// group is already in triangle order, so 0,1,2,... of the right length is all it wants. One shared array,
// grown on demand, because the renderer copies it into its own element store.
const unsigned short* GFxDisplayGetLinearIndices(unsigned int count);

// DISHONORED(bringup, agent DC): three switches the host sets, to take one kind of submission out of
// the walk. They are how the render-thread exception of the first drawn frame was bisected, and they are
// worth keeping: a UI that draws wrong is bisected the same way every time.
extern bool GFxDisplayNoShapes;
extern bool GFxDisplayNoText;
extern bool GFxDisplayNoImages;
extern bool GFxDisplayNoBeginDisplay;
extern unsigned int GFxDisplayUntexturedFills;
extern unsigned int GFxDisplayDrawTrace;
// -gfxuifitfill: ignore the style's own matrix and stretch the texture across the shape's bounds. A
// diagnostic for the fill-matrix convention, and the fallback when a style carries no usable matrix.
extern bool GFxDisplayFitFill;

// The per-definition mesh cache. Retail's is the GFxMeshCacheManager state
// (GFxMeshCache, GFxRenderGen); this is one mesh per definition, built on first display and kept for
// the definition's life, which is what makes the second frame free.
GFxShapeMesh* GFxDisplayGetShapeMesh(GFxShapeCharacterDef* def);
void          GFxDisplayReleaseShapeMeshes();

// ---------------------------------------------------------------------------------------------
// GFxDisplayContext: everything a character needs to draw itself. Retail's carries the state bag, the
// resource binding and the filter stack too; the filters are agent CC's DrawBlurRect, which nothing in
// the menu reaches (no DefineFilters tag survives the cook with a live filter list).
class GFxDisplayContext
{
public:
    GFxDisplayContext();

    GRenderer*           pRenderer;
    GFxMovieRoot*        pRoot;
    GFxMovieDefImpl*     pDefImpl;
    // DISHONORED(port): the dictionary a SHAPE's fill ids index, which is the movie the shape's
    // definition was parsed out of and NOT the root's. GFxSprite::GetOwnDataDef keeps the same
    // distinction for PlaceObject (GFxPlayer.h); a fill resolved against the wrong movie either
    // finds nothing or finds a different picture, and the main menu imports five whole screens.
    class GFxMovieDataDef* pDataDef;
    GFxGlyphRasterCache* pGlyphCache;
    GTexture*            pGlyphTexture;      // the atlas, uploaded on demand; owned by the cache glue
    GMatrix2D            Matrix;
    GRenderer::Cxform    Cx;
    unsigned int         MaskLevel;
    GFxDisplayStats      Stats;

    // 0xa5f5d0 / 0xa5f800: push and pop one character's transform and colour transform.
    void PreDisplay(const GFxCharacter* ch, GMatrix2D* savedMatrix, GRenderer::Cxform* savedCx);
    void PostDisplay(const GMatrix2D& savedMatrix, const GRenderer::Cxform& savedCx);

    void ApplyToRenderer();
};

// GMatrix2D and Cxform composition, the two operations the walk is built on. GFx spells them
// GMatrix2D::Prepend/Append and Cxform::Concatenate.
void GFxDisplayMatrixAppend(GMatrix2D* out, const GMatrix2D& outer, const GMatrix2D& inner);
void GFxDisplayCxformConcat(GRenderer::Cxform* out, const GRenderer::Cxform& outer,
                            const GRenderer::Cxform& inner);
// GMatrix2D::Invert. A SWF fill matrix maps the *bitmap's* space (20 units per image pixel) onto the
// shape's twips, so the texture matrix the renderer wants - position to 0..1 - is its inverse scaled by
// the image size.
bool GFxDisplayMatrixInvert(GMatrix2D* out, const GMatrix2D& m);
GColor GFxDisplayApplyCxform(const GRenderer::Cxform& cx, GColor c);

// The process-wide glyph cache the text fields rasterise into, and its atlas upload. Retail hangs the
// cache off the GFxFontCacheManager state of the loader's state bag (GFxFontCacheManagerImpl, and
// UpdateTextures 0xa4db30 is the upload); this is one cache for the process, created with the
// configuration FGFxEngine::InitGFxLoaderCommon sets (1024x1024, one texture, 48-px slots, 2-px
// padding - measured out of 2013 0x590ba0). Stated as a deviation in agentDC.md, with the reason: the
// cache has to outlive any one movie because every UI movie shares the two fontlib fonts.
GFxGlyphRasterCache* GFxDisplayGetGlyphCache();
void                 GFxDisplayResetGlyphCache();
// Create or refresh the alpha-only atlas texture through the renderer. Agent CC's hand-over: a
// GTexture from CreateTexture(), InitDynamicTexture(w, h, Image_A_8, 1, Usage_Update), then Update
// with the dirty rectangle, which selects GFx_PS_TextTexture by itself because the resource is PF_G8.
GTexture* GFxDisplayGetGlyphTexture(GRenderer* renderer);
void      GFxDisplayReleaseGlyphTexture();

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFX3_GFXDISPLAY_H
