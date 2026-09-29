// Scaleform GFx 3.3.89 - the mouse's half of the player: which character is under the pointer, and
// the seven button events that follow from it changing. Package DQ.
//
// Why this exists. Agent DG measured that the main menu is navigated from the AS2 `Key` broadcaster
// and not from button characters, and that is true - of the KEYBOARD. The mouse is the other half and
// it needs the thing DG's deviation 4 and DM's hand-over 2 both named: a notion of which display
// object the pointer is over. Measured here, the asset settles what that object is. The menu's single
// DefineButton2 (character 100, `btn` inside every MainMenuButton) carries two records:
//
//     record 0  states 0x07 [up|over|down]  char 98   identity matrix
//     record 1  states 0x08 [hitTest]       char 99   identity matrix
//
// and `_common.SelectionHandler`'s element loop assigns `elem.btn.onRollOver` and `elem.btn.onRelease`.
// So every menu entry's hit area IS that one button's hitTest record, and its handlers live on the
// button instance. A tree in which a DefineButton2 instantiates as a GFxGenericCharacter - which
// answers its definition's point test, and a button definition has none - cannot see it.
//
// DISHONORED(port), the retail chain:
//   GFxMovieRoot::ProcessMouse                 2013 0xa05330
//   GFxMovieRoot::GetTopMostEntity             2013 0x9fabf0
//   GFxSprite::GetTopMostMouseEntity           2013 0x9f0e80
//   GFxSprite::PointTestLocal                  2013 0x9f2320
//   GFxSprite::CalcDisplayListHitTestMaskArray 2013 0x9f0d70
//   GFxSprite::ActsAsButton                    2013 0x9ecc70 / GASMovieClipObject:: 0x9ec260
//   GFxSprite::HasEventHandler                 2013 0x9eb2e0
//   GFxGenericCharacter::PointTestLocal        2013 0x9c4980
//   GFxGenericCharacter::GetTopMostMouseEntity 2013 0x9c55a0
//   GFxButtonCharacter::PointTestLocal         2013 0xa5c310
//   GFxButtonCharacter::GetTopMostMouseEntity  2013 0xa5c490
//   GFxEditTextCharacter::PointTestLocal       2013 0xa23d60
//   GFxEditTextCharacter::GetTopMostMouseEntity 2013 0xa23990
//   GFxShapeCharacterDef::DefPointTestLocal    2013 0xa3a3a0 -> DefPointTestLocalImpl 0xa39fc0
//   GFxShapeBase::PointInShape                 2013 0xa38890
//   GMath2D::CheckCurveIntersection            2013 0xa32490 / CheckMonoCurveIntersection 0xa30920
//   GFxASCharacter::ExecuteEvent(GFxEventId)   2013 0x9c8b30
//   GFxEventId::GetFunctionName                2013 0x9d6030 / 0x9d5e80
//   GFxMouseState::*                           2013 0x9fb950, 0x9fc460, 0x9fc510, 0xa25e10, 0xa5acb0
//   GFx_GenerateMouseButtonEvents              2013 0xa5ad10
#include "GFxPlayer.h"
#include "GFxCharacterDefs.h"
#include "GFxTextField.h"
#include "GFxAS2Object.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// The census the engine's -dismouse line reports. It is the only way to tell "the click was
// delivered" from "the click found a target" from "the target had a handler", and those three were
// indistinguishable before this package.
// How deep the walk is, for -dismouse's indentation only.
int GFxMouseTraceDepth = 0;
GFxMouseCensus GFxMouseCensusData;
GFxMouseCensus& GFxMouseGetCensus() { return GFxMouseCensusData; }
void GFxMouseResetCensus() { memset(&GFxMouseCensusData, 0, sizeof(GFxMouseCensusData)); }
bool GFxMouseTrace = false;

// ---------------------------------------------------------------------------------------------
// GFxEventId::GetFunctionName. 2013 0x9d6030 is one line: the builtin name at
// GetFunctionNameBuiltinType (0x9d5e80), which is log2(Id) into a 0x23-entry table. The table was
// read out of the retail image (dword_13B3F38) and the seven button events 0x400..0x10000 map to a
// contiguous run of builtin indices 91..97, in SWF ClipActionRecord bit order - which is what settles
// rollOver = 0x2000, rollOut = 0x4000, press = 0x400, release = 0x800, releaseOutside = 0x1000.
const char* GFxEventId::GetFunctionName() const
{
    switch (Id)
    {
    case Event_Load:             return "onLoad";
    case Event_EnterFrame:       return "onEnterFrame";
    case Event_Unload:           return "onUnload";
    case Event_MouseMove:        return "onMouseMove";
    case Event_MouseDown:        return "onMouseDown";
    case Event_MouseUp:          return "onMouseUp";
    case Event_KeyDown:          return "onKeyDown";
    case Event_KeyUp:            return "onKeyUp";
    case Event_Data:             return "onData";
    case Event_Initialize:       return "onInitialize";
    case Event_Press:
    case Event_Press_1:          return "onPress";
    case Event_Release:
    case Event_Release_1:        return "onRelease";
    case Event_ReleaseOutside:
    case Event_ReleaseOutside_1: return "onReleaseOutside";
    case Event_RollOver:         return "onRollOver";
    case Event_RollOut:          return "onRollOut";
    case Event_DragOver:
    case Event_DragOver_1:       return "onDragOver";
    case Event_DragOut:
    case Event_DragOut_1:        return "onDragOut";
    case Event_Construct:        return "onConstruct";
    default:                     return 0;
    }
}

// ---------------------------------------------------------------------------------------------
// GFxCharacter: the weak proxy and the two hit-test virtuals' bases.

GFxWeakProxy* GFxCharacter::CreateWeakProxy()
{
    if (pWeakProxy == 0)
        pWeakProxy = new GFxWeakProxy(this);
    pWeakProxy->AddRef();
    return pWeakProxy;
}

bool GFxCharacter::PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const
{
    (void)pt; (void)hitTestMask;
    return false;
}

GFxASCharacter* GFxCharacter::GetTopMostMouseEntity(const GPoint<float>& pt,
                                                    const TopMostParams& params)
{
    (void)pt; (void)params;
    return 0;
}

bool GFxCharacterDef::DefPointTestLocal(const GPoint<float>& pt, bool testShape,
                                        const GFxCharacter* inst) const
{
    (void)pt; (void)testShape; (void)inst;
    return false;
}

// ---------------------------------------------------------------------------------------------
// The geometry. 2013 GFxShapeBase::PointInShape 0xa38890.
//
// One horizontal-ray crossing test per edge, with the parity kept per sub-shape and a path taken into
// account only when exactly one of its two sides carries a fill - `(fill0 == 0) != (fill1 == 0)`,
// which is the decompile's `v18 != v19`. The sign test is retail's own and it is worth writing out:
// with (xlo, ylo) the lower-y endpoint and (xhi, yhi) the higher,
//     d = (px - xhi) * (yhi - ylo) - (xhi - xlo) * (py - yhi)
// and d > 0 is exactly px > x(py) on the edge, so the ray is cast to the right.
//
// The stroke half of retail's body - which builds a GFxRenderGenStroker, generates the stroked outline
// of each line-styled path and tests THAT with GCompoundShape::PointInShape (0xa50410) - is not
// reproduced: it only matters for a shape whose hit area is its outline rather than its fill, and the
// menu's hit shape (char 99) is a filled rectangle. Named as deviation 4.

static bool GFxHT_CrossesEdge(float x0, float y0, float x1, float y1, float px, float py)
{
    float xlo = x0, ylo = y0, xhi = x1, yhi = y1;
    if (y1 < y0)
    {
        xlo = x1; ylo = y1;
        xhi = x0; yhi = y0;
    }
    if (ylo <= py && yhi > py)
    {
        const float d = (px - xhi) * (yhi - ylo) - (xhi - xlo) * (py - yhi);
        if (d > 0.f)
            return true;
    }
    return false;
}

// 2013 GMath2D::CheckMonoCurveIntersection 0xa30920: a quadratic whose y is monotonic. The three
// early answers are the cheap corner tests; the root of the y equation then gives the parameter at
// which the curve crosses the scan line and the x there decides.
static bool GFxHT_MonoCurveCrosses(float x0, float y0, float cx, float cy, float x1, float y1,
                                   float px, float py)
{
    if (y0 > py)
        return false;
    if (y1 <= py)
        return false;
    const bool s0 = ((px - cx) * (cy - y0) - (cx - x0) * (py - cy)) > 0.f;
    const bool s1 = ((y1 - cy) * (px - x1) - (x1 - cx) * (py - y1)) > 0.f;
    const float s2 = (y1 - y0) * (px - x1) - (x1 - x0) * (py - y1);
    if (s1 && s2 > 0.f && s0)
        return true;
    if (!s1 && s2 <= 0.f && !s0)
        return false;
    const float a = y0 - (cy + cy) + y1;
    float t = -1.f;
    if (a == 0.f)
    {
        const float b = y1 - y0;
        if (b != 0.f)
            t = (py - y0) / b;
    }
    else
    {
        const float disc = py * y1 + cy * cy - (y1 - py) * y0 - (py + py) * cy;
        t = (disc <= 0.f) ? ((y0 - cy) / a) : ((y0 + sqrtf(disc) - cy) / a);
    }
    const float omt = 1.f - t;
    const float x = omt * omt * x0 + 2.f * omt * t * cx + t * t * x1;
    return px > x;
}

// 2013 GMath2D::CheckCurveIntersection 0xa32490: split a non-monotonic quadratic at its y extremum
// and answer the parity of the two monotonic halves.
static bool GFxHT_CurveCrosses(float x0, float y0, float cx, float cy, float x1, float y1,
                               float px, float py)
{
    if (y0 <= cy && y1 >= cy)
        return GFxHT_MonoCurveCrosses(x0, y0, cx, cy, x1, y1, px, py);
    const float den = cy + cy - y0 - y1;
    const float t = (den == 0.f) ? -1.f : ((cy - y0) / den);
    // SubdivideQuadCurve at t: de Casteljau.
    const float ax = x0 + (cx - x0) * t,  ay = y0 + (cy - y0) * t;
    const float bx = cx + (x1 - cx) * t,  by = cy + (y1 - cy) * t;
    const float mx = ax + (bx - ax) * t,  my = ay + (by - ay) * t;
    float h0[6] = { x0, y0, ax, ay, mx, my };
    float h1[6] = { mx, my, bx, by, x1, y1 };
    if (h0[5] < h0[1]) { float tx = h0[0], ty = h0[1]; h0[0] = h0[4]; h0[1] = h0[5]; h0[4] = tx; h0[5] = ty; }
    if (h1[5] < h1[1]) { float tx = h1[0], ty = h1[1]; h1[0] = h1[4]; h1[1] = h1[5]; h1[4] = tx; h1[5] = ty; }
    const bool a = GFxHT_MonoCurveCrosses(h1[0], h1[1], h1[2], h1[3], h1[4], h1[5], px, py);
    const bool b = GFxHT_MonoCurveCrosses(h0[0], h0[1], h0[2], h0[3], h0[4], h0[5], px, py);
    return a != b;
}

bool GFxShapeCharacterDef::DefPointTestLocal(const GPoint<float>& pt, bool testShape,
                                             const GFxCharacter* inst) const
{
    // 2013 GFxShapeBase::DefPointTestLocalImpl 0xa39fc0: the bounds first, then the winding walk,
    // with the one-entry per-character cache in between.
    const GRect<int>& b = Bounds;
    if (!((float)b.Right >= pt.x && (float)b.Left <= pt.x
          && (float)b.Bottom >= pt.y && (float)b.Top <= pt.y))
        return false;
    if (!testShape)
        return true;
    if (inst != 0 && inst->CheckLastHitResult(pt.x, pt.y))
        return inst->GetLastHitResult();

    const float scale = Shape.bTwentyTimesScale ? 0.05f : 1.0f;
    bool inside = false;
    bool result = false;
    for (unsigned int i = 0; i < Shape.GetPathCount(); ++i)
    {
        const GFxShapePathCD* path = Shape.GetPath(i);
        if (path == 0)
            continue;
        // A new sub-shape starts where a path declares no styles at all; retail's iterator reports
        // that as the group boundary and resets the parity, answering TRUE if it was set.
        if (path->Fill0 == 0 && path->Fill1 == 0 && path->Line == 0)
        {
            if (inside)
            {
                result = true;
                break;
            }
            continue;
        }
        if ((path->Fill0 == 0) == (path->Fill1 == 0))
            continue;
        float x = (float)path->StartX * scale;
        float y = (float)path->StartY * scale;
        for (unsigned int e = 0; e < path->EdgeCount; ++e)
        {
            const GFxShapeEdgeCD& ed = path->Edges[e];
            const float ax = (float)ed.Ax * scale, ay = (float)ed.Ay * scale;
            if (ed.bCurve)
            {
                const float cx = (float)ed.Cx * scale, cy = (float)ed.Cy * scale;
                if (GFxHT_CurveCrosses(x, y, cx, cy, ax, ay, pt.x, pt.y))
                    inside = !inside;
            }
            else if (GFxHT_CrossesEdge(x, y, ax, ay, pt.x, pt.y))
            {
                inside = !inside;
            }
            x = ax;
            y = ay;
        }
    }
    if (inside)
        result = true;
    if (inst != 0)
        inst->SetLastHitResult(pt.x, pt.y, result);
    ++GFxMouseCensusData.ShapeTests;
    return result;
}

// ---------------------------------------------------------------------------------------------
// GFxGenericCharacter. 2013 PointTestLocal 0x9c4980 is one line - ask the definition - and
// GetTopMostMouseEntity 0x9c55a0 asks the definition and then walks UP to the nearest ancestor sprite
// that acts as a button, which is how a shape inside a clip makes the CLIP the mouse entity.
bool GFxGenericCharacter::PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const
{
    if (pDef == 0)
        return false;
    return pDef->DefPointTestLocal(pt, (hitTestMask & HitTest_TestShape) != 0, this);
}

GFxASCharacter* GFxGenericCharacter::GetTopMostMouseEntity(const GPoint<float>& pt,
                                                           const TopMostParams& params)
{
    if (!GetVisible())
        return 0;
    GPoint<float> local;
    Matrix.TransformByInverse(&local, pt);
    if (GetClipDepth() != 0)
        return 0;
    if (pDef == 0 || !pDef->DefPointTestLocal(local, true, this))
        return 0;
    GFxASCharacter* up = pParent;
    while (up != 0 && up->ToSprite() != 0)
    {
        if (params.bTestAll || up->ToSprite()->ActsAsButton())
        {
            if (params.pIgnoreMC != up)
                return up;
        }
        up = up->pParent;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------
// GFxButtonCharacter. Both bodies walk the definition's records and use only those that belong to the
// hitTest state, transforming the point by each record's own matrix. 2013 0xa5c310 / 0xa5c490.

static bool GFxHT_ButtonHit(const GFxCharacter* self, GFxCharacterDef* def,
                           const GPoint<float>& local, unsigned char hitTestMask)
{
    GFxButtonCharacterDef* btn = (def != 0 && def->GetResourceTypeCode() == GFxResource::RT_ButtonDef)
                                     ? (GFxButtonCharacterDef*)def : 0;
    if (btn == 0)
        return false;
    GFxMovieDataDef* dataDef = 0;
    const GFxASCharacter* as = self->pParent;
    if (as != 0)
    {
        GFxSprite* sp = ((GFxASCharacter*)as)->ToSprite();
        if (sp != 0)
            dataDef = sp->GetOwnDataDef();
    }
    if (dataDef == 0)
        return false;
    for (unsigned int i = 0; i < btn->RecordCount; ++i)
    {
        const GFxButtonRecord& rec = btn->Records[i];
        if ((rec.StateFlags & GFxButtonRecord::State_HitTest) == 0)
            continue;
        GFxCharacterDef* sub = dataDef->GetCharacterDefById(rec.CharacterId);
        if (sub == 0 || sub == def)
            continue;
        GPoint<float> inner;
        rec.Matrix.TransformByInverse(&inner, local);
        if (sub->DefPointTestLocal(inner, (hitTestMask & GFxCharacter::HitTest_TestShape) != 0, self))
            return true;
    }
    return false;
}

bool GFxButtonCharacter::PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const
{
    if ((hitTestMask & HitTest_SkipInvisible) != 0 && !GetVisible())
        return false;
    return GFxHT_ButtonHit(this, pDef, pt, hitTestMask);
}

GFxASCharacter* GFxButtonCharacter::GetTopMostMouseEntity(const GPoint<float>& pt,
                                                          const TopMostParams& params)
{
    if (!GetVisible() || params.pIgnoreMC == this)
        return 0;
    GPoint<float> local;
    Matrix.TransformByInverse(&local, pt);
    const bool hit = GFxHT_ButtonHit(this, pDef, local, HitTest_TestShape);
    if (GFxMouseTrace)
    {
        printf("DISHONORED(bringup): hit %*sbutton '%s' depth %d pt (%.0f,%.0f) -> %d\n",
               GFxMouseTraceDepth * 2, "", Name.ToCStr(), GetDepth(),
               local.x * 0.05f, local.y * 0.05f, hit ? 1 : 0);
    }
    if (!hit)
        return 0;
    ++GFxMouseCensusData.ButtonHits;
    return this;
}

// ---------------------------------------------------------------------------------------------
// GFxEditTextCharacter. 2013 PointTestLocal 0xa23d60 is the formatted view rectangle, and
// GetTopMostMouseEntity 0xa23990 answers the field itself only when it is selectable or has a mouse
// handler; otherwise it walks up like a generic character does. A label field is neither, which is why
// the label of a menu entry does not steal the entry's rollover.
bool GFxEditTextCharacter::PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const
{
    if ((hitTestMask & HitTest_SkipInvisible) != 0 && !GetVisible())
        return false;
    GMatrix2D identity;
    const GRect<float> r = GetBoundsTwips(identity);
    return r.Left <= pt.x && r.Right >= pt.x && r.Top <= pt.y && r.Bottom >= pt.y;
}

GFxASCharacter* GFxEditTextCharacter::GetTopMostMouseEntity(const GPoint<float>& pt,
                                                            const TopMostParams& params)
{
    if (!GetVisible())
        return 0;
    GPoint<float> local;
    Matrix.TransformByInverse(&local, pt);
    if (!PointTestLocal(local, HitTest_TestShape))
        return 0;
    GFxASCharacter* up = pParent;
    while (up != 0 && up->ToSprite() != 0)
    {
        if (params.bTestAll || up->ToSprite()->ActsAsButton())
        {
            if (params.pIgnoreMC != up)
                return up;
        }
        up = up->pParent;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------
// GFxSprite::ActsAsButton. 2013 0x9ecc70 -> GASMovieClipObject::ActsAsButton 0x9ec260: the clip, then
// every object up its prototype chain, is asked whether it carries any of the seven button handlers.
// Retail keeps a per-object bitmask of which handler names have ever been assigned; this walks the
// member store by name instead, which gives the same answer for the same reason.

static const char* const GFxHT_ButtonHandlerNames[] =
{
    "onPress", "onRelease", "onReleaseOutside", "onRollOver", "onRollOut",
    "onDragOver", "onDragOut"
};

static bool GFxHT_ObjectHasButtonHandler(GASStringContext* sc, GASObject* obj)
{
    int guard = 0;
    while (obj != 0 && guard++ < 16)
    {
        for (int i = 0; i < 7; ++i)
        {
            GASValue v;
            if (obj->GetConstMemberRaw(sc, GFxHT_ButtonHandlerNames[i], &v) && !v.IsUndefined())
                return true;
        }
        obj = obj->Get__proto__();
    }
    return false;
}

bool GFxSprite::ActsAsButton() const
{
    if (GetClipDepth() != 0)
        return false;
    GFxMovieRoot* root = GetMovieRoot();
    GASStringContext* sc = (root != 0 && root->GetASContext() != 0)
                               ? root->GetASContext()->GetSC() : 0;
    if (sc == 0)
        return false;
    if (pASObject != 0 && GFxHT_ObjectHasButtonHandler(sc, pASObject))
        return true;
    return GFxHT_ObjectHasButtonHandler(sc, pProto);
}

bool GFxSprite::HasEventHandler(const GFxEventId& id) const
{
    const char* name = id.GetFunctionName();
    GFxMovieRoot* root = GetMovieRoot();
    if (name == 0 || root == 0 || root->GetASContext() == 0)
        return false;
    GASStringContext* sc = root->GetASContext()->GetSC();
    GASValue v;
    if (pASObject != 0 && pASObject->GetConstMemberRaw(sc, name, &v) && !v.IsUndefined())
        return true;
    GASObject* p = pProto;
    int guard = 0;
    while (p != 0 && guard++ < 16)
    {
        if (p->GetConstMemberRaw(sc, name, &v) && !v.IsUndefined())
            return true;
        p = p->Get__proto__();
    }
    return false;
}

// 2013 0x9f0d70. One byte per entry; an entry with a non-zero ClipDepth decides the byte for itself
// and for every following entry whose depth is still within that clip depth.
void GFxSprite::CalcDisplayListHitTestMaskArray(unsigned char* out, unsigned int outCount,
                                                const GPoint<float>& pt, bool testShape) const
{
    GFxDisplayList& list = ((GFxSprite*)this)->GetDisplayList();
    const unsigned int n = list.GetCount() < outCount ? list.GetCount() : outCount;
    for (unsigned int i = 0; i < n; ++i)
        out[i] = 1;
    for (unsigned int i = 0; i < n; ++i)
    {
        GFxCharacter* mask = list.GetAt(i);
        if (mask == 0 || mask->GetClipDepth() == 0)
            continue;
        GPoint<float> inner;
        mask->GetMatrix().TransformByInverse(&inner, pt);
        const unsigned char hit =
            mask->PointTestLocal(inner, testShape ? HitTest_TestShape : 0) ? 1u : 0u;
        out[i] = hit;
        unsigned int j = i + 1;
        for (; j < n; ++j)
        {
            GFxCharacter* ch = list.GetAt(j);
            if (ch != 0 && ch->GetDepth() > mask->GetClipDepth())
                break;
            out[j] = hit;
        }
        i = j - 1;
    }
}

// ---------------------------------------------------------------------------------------------
// GFxSprite::PointTestLocal. 2013 0x9f2320: the bounds, the sprite's own mask, then the display list
// top-down, and finally the drawing context (which this tree has none of).
bool GFxSprite::PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const
{
    if (GetClipDepth() != 0)
        return false;
    if ((hitTestMask & HitTest_SkipInvisible) != 0 && !GetVisible())
        return false;
    GFxDisplayList& list = ((GFxSprite*)this)->GetDisplayList();
    const unsigned int n = list.GetCount();
    if (n == 0)
        return false;
    unsigned char maskArray[256];
    const unsigned int masked = n < 256 ? n : 256;
    CalcDisplayListHitTestMaskArray(maskArray, masked, pt, (hitTestMask & HitTest_TestShape) != 0);
    for (int i = (int)n - 1; i >= 0; --i)
    {
        GFxCharacter* ch = list.GetAt((unsigned int)i);
        if (ch == 0 || ch->GetClipDepth() != 0)
            continue;
        if ((unsigned int)i < masked && maskArray[i] == 0)
            continue;
        if ((hitTestMask & HitTest_SkipInvisible) != 0 && !ch->GetVisible())
            continue;
        GPoint<float> inner;
        ch->GetMatrix().TransformByInverse(&inner, pt);
        if (ch->PointTestLocal(inner, hitTestMask))
            return true;
    }
    return false;
}

// GFxSprite::GetTopMostMouseEntity. 2013 0x9f0e80.
GFxASCharacter* GFxSprite::GetTopMostMouseEntity(const GPoint<float>& pt,
                                                 const TopMostParams& params)
{
    if (GFxMouseTrace)
    {
        printf("DISHONORED(bringup): hit %*ssprite '%s' depth %d clip %d vis %d pt (%.0f,%.0f)\n",
               GFxMouseTraceDepth * 2, "", Name.ToCStr(), GetDepth(), GetClipDepth(),
               GetVisible() ? 1 : 0, pt.x * 0.05f, pt.y * 0.05f);
    }
    if (GetClipDepth() != 0 || !GetVisible() || params.pIgnoreMC == this)
        return 0;
    GPoint<float> local;
    Matrix.TransformByInverse(&local, pt);

    GFxDisplayList& list = GetDisplayList();
    const unsigned int n = list.GetCount();
    unsigned char maskArray[256];
    const unsigned int masked = n < 256 ? n : 256;
    if (n != 0)
        CalcDisplayListHitTestMaskArray(maskArray, masked, local, true);

    const bool selfIsButton = ActsAsButton();
    ++GFxMouseTraceDepth;
    for (int i = (int)n - 1; i >= 0; --i)
    {
        GFxCharacter* ch = list.GetAt((unsigned int)i);
        if (ch == 0 || ch->GetClipDepth() != 0)
            continue;
        if ((unsigned int)i < masked && maskArray[i] == 0)
            continue;
        GFxASCharacter* hit = ch->GetTopMostMouseEntity(local, params);
        if (GFxMouseTrace && hit != 0)
        {
            printf("DISHONORED(bringup): hit %*s-> child %d of '%s' answered '%s' (selfIsButton %d)\n",
                   GFxMouseTraceDepth * 2, "", i, Name.ToCStr(), hit->Name.ToCStr(),
                   selfIsButton ? 1 : 0);
        }
        if (hit != 0 && params.bTestAll)
        {
            --GFxMouseTraceDepth;
            return hit;
        }
        if (selfIsButton)
        {
            // Retail: a child hit makes THIS sprite the entity, because the sprite IS the button. The
            // child answering non-null is the whole condition - a plain child resolves the walk by
            // returning the nearest ancestor that acts as a button, which is this sprite.
            if (hit != 0)
            {
                ++GFxMouseCensusData.SpriteHits;
                --GFxMouseTraceDepth;
                return this;
            }
            continue;
        }
        if (hit != 0 && hit != this)
        {
            --GFxMouseTraceDepth;
            if (hit->pParent != 0 && hit->pParent->GetVisible())
                return hit;
            return 0;
        }
    }
    --GFxMouseTraceDepth;
    return 0;
}

// ---------------------------------------------------------------------------------------------
// GFxMovieRoot::GetTopMostEntity. 2013 0x9fabf0.
GFxASCharacter* GFxMovieRoot::GetTopMostEntity(const GPoint<float>& pt, unsigned int mouseIndex,
                                               bool testAll, const GFxASCharacter* ignore)
{
    ++GFxMouseCensusData.HitTests;
    if (pLevel0 == 0)
        return 0;
    GFxCharacter::TopMostParams params(this, ignore, mouseIndex, testAll);
    extern int GFxMouseTraceDepth;
    GFxMouseTraceDepth = 0;
    GFxASCharacter* hit = pLevel0->GetTopMostMouseEntity(pt, params);
    if (hit != 0)
        ++GFxMouseCensusData.TargetsResolved;
    if (GFxMouseTrace)
    {
        GASStringContext* sc = (pGC != 0) ? pGC->GetSC() : 0;
        printf("DISHONORED(bringup): mouse hit test at (%.1f,%.1f) px -> %s\n",
               pt.x * 0.05f, pt.y * 0.05f,
               (hit != 0 && sc != 0) ? hit->GetTargetPath(sc).ToCStr() : "NONE");
    }
    return hit;
}

// ---------------------------------------------------------------------------------------------
// GFxASCharacter::ExecuteEvent(const GFxEventId&). 2013 0x9c8b30, reduced to the arm this cook uses:
// the handler is a member of the character (or of its registered class's prototype) and retail invokes
// it with no arguments while the global context's extended-clip-event flag is clear, which is the
// case here.
bool GFxASCharacter::ExecuteEvent(const GFxEventId& id)
{
    const char* name = id.GetFunctionName();
    if (name == 0 || pMovieRoot == 0)
        return false;
    GASEnvironment* env = pMovieRoot->GetASEnvironment();
    if (env == 0)
        return false;
    GASValue handler;
    if (!GetMemberRaw(env->GetSC(), env->CreateString(name), &handler))
        return false;
    if (handler.GetType() == GASValue::PROPERTY)
    {
        GASValue resolved;
        handler.GetPropertyValue(env, this, &resolved);
        handler = resolved;
    }
    GASFunctionObject* fn = handler.GetFunction();
    if (fn == 0)
        return false;
    if (GFxMouseTrace)
    {
        printf("DISHONORED(bringup): mouse event %s -> %s\n", name,
               GetTargetPath(env->GetSC()).ToCStr());
    }
    ++GFxMouseCensusData.HandlersInvoked;
    GASValue result;
    GASFnCall call(&result, this, env, 0, env->GetTopIndex());
    fn->Invoke(call);
    return true;
}

// ---------------------------------------------------------------------------------------------
// GFxMouseState. 2013 0x9fb950 / 0x9fc460 / 0x9fc510 / 0xa25e10 / 0xa5acb0.

GFxMouseState::GFxMouseState()
    : pTopmost(0), pPrevTopmost(0), pActive(0), CurButtons(0), PrevButtons(0),
      X(0.f), Y(0.f), ScrollDelta(0), Flags(Flag_TopmostNull | Flag_PrevNull) {}

GFxMouseState::~GFxMouseState()
{
    if (pTopmost) pTopmost->Release();
    if (pPrevTopmost) pPrevTopmost->Release();
    if (pActive) pActive->Release();
}

GFxASCharacter* GFxMouseState::Resolve(GFxWeakProxy** slot)
{
    GFxWeakProxy* p = *slot;
    if (p == 0)
        return 0;
    if (p->pObject != 0)
        return (GFxASCharacter*)p->pObject;
    // Retail drops the slot the moment the proxy says its object is gone, which is what makes a clip
    // removed between two mouse events read as "no entity" rather than as a stale one.
    p->Release();
    *slot = 0;
    return 0;
}

void GFxMouseState::Assign(GFxWeakProxy** slot, GFxASCharacter* ch, unsigned char* flags,
                           unsigned char nullBit)
{
    GFxWeakProxy* proxy = (ch != 0) ? ch->CreateWeakProxy() : 0;
    if (*slot != 0)
        (*slot)->Release();
    *slot = proxy;
    if (ch == 0)
        *flags = (unsigned char)(*flags | nullBit);
    else
        *flags = (unsigned char)(*flags & ~nullBit);
}

void GFxMouseState::UpdateState(const GFxInputEventsQueue::QueueEntry& e)
{
    const GFxInputEventsQueue::MouseEntry& m = e.MouseData;
    Flags = (unsigned char)(Flags | Flag_Updated);
    PrevButtons = CurButtons;
    if (m.Buttons != 0)
    {
        // The queue marks a release with the 0x80 bit in ChangedButtons (GFxMovieRoot::HandleEvent);
        // retail reads the same distinction out of the entry's flag byte.
        if ((m.ChangedButtons & 0x80u) != 0)
            CurButtons = CurButtons & ~m.Buttons;
        else
            CurButtons = CurButtons | m.Buttons;
    }
    if ((int)m.x == (int)X && (int)m.y == (int)Y)
        Flags = (unsigned char)(Flags & ~Flag_PosMoved);
    else
        Flags = (unsigned char)(Flags | Flag_PosMoved);
    X = m.x;
    Y = m.y;
    ScrollDelta = m.ScrollDelta;
}

void GFxMouseState::SetTopmostEntity(GFxASCharacter* ch)
{
    // prev = cur, cur = ch, and the two "deliberately empty" bits move with them.
    if (pTopmost != 0)
        pTopmost->AddRef();
    if (pPrevTopmost != 0)
        pPrevTopmost->Release();
    pPrevTopmost = pTopmost;
    Flags = (unsigned char)((Flags & ~Flag_PrevNull) | ((Flags & Flag_TopmostNull) << 1));
    Assign(&pTopmost, ch, &Flags, (unsigned char)Flag_TopmostNull);
}

bool GFxMouseState::IsTopmostEntityChanged() const
{
    GFxMouseState* self = (GFxMouseState*)this;
    GFxASCharacter* cur = Resolve(&self->pTopmost);
    GFxASCharacter* prev = Resolve(&self->pPrevTopmost);
    if (cur == prev
        && (cur != 0 || (Flags & Flag_TopmostNull) != 0)
        && (prev != 0 || (Flags & Flag_PrevNull) != 0))
        return false;
    return true;
}

GFxASCharacter* GFxMouseState::GetTopmostEntity() const
{
    return Resolve(&((GFxMouseState*)this)->pTopmost);
}

GFxASCharacter* GFxMouseState::GetActiveEntity() const
{
    return Resolve(&((GFxMouseState*)this)->pActive);
}

void GFxMouseState::SetActiveEntity(GFxASCharacter* ch)
{
    Assign(&pActive, ch, &Flags, 0);
}

// ---------------------------------------------------------------------------------------------
// GFx_GenerateMouseButtonEvents. 2013 0xa5ad10 (2012 0xa66a90, which is the address the briefs carry).
//
// The active entity is the one the press landed on and the topmost is the one the pointer is over now.
// Retail's body is one loop over the changed button bits and then one final rollOver / rollOut pair:
//
//   a bit that went DOWN            -> Press on the active entity, and the pointer counts as inside
//   a bit that went UP, inside      -> Release
//   a bit that went UP, outside     -> ReleaseOutside
//   a bit still down, entity same   -> DragOver, ++RollOverCnt
//   a bit still down, entity moved  -> DragOut,  --RollOverCnt
//   no bit down and the entity moved-> RollOut on the old (--cnt), RollOver on the new (++cnt)
//
// The rollOver count is the character's own +169 byte and travels in the event id, which is how the
// content can tell a re-entry from a first entry.
void GFx_GenerateMouseButtonEvents(unsigned char controllerIdx, GFxMouseState* state,
                                   unsigned int buttonCount)
{
    if (state == 0)
        return;
    // Both entities are held for the length of the body, as retail holds them: every ExecuteEvent
    // below runs content that can remove the clip it is called on.
    GFxASCharacter* active = state->GetActiveEntity();
    GFxASCharacter* topmost = state->GetTopmostEntity();
    if (active != 0) active->AddRef();
    if (topmost != 0) topmost->AddRef();
    const unsigned int changed = state->GetChangedButtons();
    const unsigned int cur = state->GetButtons();
    const unsigned int prev = state->GetPrevButtons();
    bool inside = state->IsInside();
    bool releasedOutside = false;

    for (unsigned int i = 0; i < buttonCount; ++i)
    {
        const unsigned int bit = 1u << i;
        if ((changed & bit) != 0)
        {
            if ((bit & prev) != 0 && (bit & cur) == 0 && active != 0)
            {
                if (inside)
                {
                    GFxEventId id(GFxEventId::Event_Release);
                    id.AsciiCode = (unsigned char)i;
                    id.KeyboardIndex = controllerIdx;
                    active->ExecuteEvent(id);
                    ++GFxMouseCensusData.Releases;
                }
                else
                {
                    GFxEventId id(GFxEventId::Event_ReleaseOutside);
                    id.AsciiCode = (unsigned char)i;
                    id.KeyboardIndex = controllerIdx;
                    active->ExecuteEvent(id);
                    releasedOutside = true;
                }
            }
            if ((bit & cur) != 0)
            {
                if (active != 0)
                {
                    GFxEventId id(GFxEventId::Event_Press);
                    id.AsciiCode = (unsigned char)i;
                    id.KeyboardIndex = controllerIdx;
                    active->ExecuteEvent(id);
                    ++GFxMouseCensusData.Presses;
                }
                inside = true;
            }
        }
        else if ((prev & bit) != 0)
        {
            if (inside)
            {
                if (topmost != active && active != 0)
                {
                    const unsigned char cnt = active->RollOverCnt;
                    active->RollOverCnt = (unsigned char)(cnt ? cnt - 1 : 0xFF);
                    GFxEventId id(GFxEventId::Event_DragOut);
                    id.AsciiCode = (unsigned char)i;
                    id.KeyboardIndex = controllerIdx;
                    id.RollOverCnt = active->RollOverCnt;
                    active->ExecuteEvent(id);
                    inside = false;
                }
            }
            else if (topmost == active && active != 0)
            {
                const unsigned char cnt = active->RollOverCnt;
                active->RollOverCnt = (unsigned char)(cnt + 1);
                GFxEventId id(GFxEventId::Event_DragOver);
                id.AsciiCode = (unsigned char)i;
                id.KeyboardIndex = controllerIdx;
                id.RollOverCnt = cnt;
                active->ExecuteEvent(id);
                inside = true;
            }
        }
    }

    if (cur == 0 && topmost != active)
    {
        if (!releasedOutside && active != 0)
        {
            const unsigned char cnt = active->RollOverCnt;
            active->RollOverCnt = (unsigned char)(cnt ? cnt - 1 : 0xFF);
            GFxEventId id(GFxEventId::Event_RollOut);
            id.KeyboardIndex = controllerIdx;
            id.RollOverCnt = active->RollOverCnt;
            active->ExecuteEvent(id);
            ++GFxMouseCensusData.RollOuts;
        }
        if (active != 0)
            active->Release();
        active = topmost;
        if (active != 0)
        {
            active->AddRef();
            const unsigned char cnt = active->RollOverCnt;
            active->RollOverCnt = (unsigned char)(cnt + 1);
            GFxEventId id(GFxEventId::Event_RollOver);
            id.KeyboardIndex = controllerIdx;
            id.RollOverCnt = cnt;
            active->ExecuteEvent(id);
            ++GFxMouseCensusData.RollOvers;
        }
        inside = true;
    }

    state->SetInside(inside);
    state->SetActiveEntity(active);
    if (topmost != 0) topmost->Release();
    if (active != 0) active->Release();
}

// ---------------------------------------------------------------------------------------------
// GFxButtonCharacter::GetBoundsTwips. The union of the records' bounds through each record's matrix,
// then through `m`. Without it a button answers an empty rectangle and every `btn._width = w` the
// content writes is lost, because the geometry setters derive the scale from the bounds.
GRect<float> GFxButtonCharacter::GetBoundsTwips(const GMatrix2D& m) const
{
    GFxButtonCharacterDef* btn = (pDef != 0 && pDef->GetResourceTypeCode() == GFxResource::RT_ButtonDef)
                                     ? (GFxButtonCharacterDef*)pDef : 0;
    GFxMovieDataDef* dataDef = 0;
    if (pParent != 0)
    {
        GFxSprite* sp = pParent->ToSprite();
        if (sp != 0)
            dataDef = sp->GetOwnDataDef();
    }
    if (btn == 0 || dataDef == 0)
        return GRect<float>(0.f, 0.f, 0.f, 0.f);
    GRect<float> out(0.f, 0.f, 0.f, 0.f);
    bool any = false;
    for (unsigned int i = 0; i < btn->RecordCount; ++i)
    {
        const GFxButtonRecord& rec = btn->Records[i];
        GFxCharacterDef* sub = dataDef->GetCharacterDefById(rec.CharacterId);
        if (sub == 0 || sub == pDef)
            continue;
        const GRect<int> b = GFxCharacterDefGetBoundsTwips(sub);
        if (b.Right <= b.Left && b.Bottom <= b.Top)
            continue;
        const float xs[4] = { (float)b.Left, (float)b.Right, (float)b.Left, (float)b.Right };
        const float ys[4] = { (float)b.Top, (float)b.Top, (float)b.Bottom, (float)b.Bottom };
        for (int c = 0; c < 4; ++c)
        {
            float x = xs[c], y = ys[c];
            rec.Matrix.Transform(&x, &y);
            m.Transform(&x, &y);
            if (!any)
            {
                out.Left = out.Right = x;
                out.Top = out.Bottom = y;
                any = true;
            }
            else
            {
                if (x < out.Left) out.Left = x;
                if (x > out.Right) out.Right = x;
                if (y < out.Top) out.Top = y;
                if (y > out.Bottom) out.Bottom = y;
            }
        }
    }
    return out;
}

// MovieClip.hitTest(x, y) / hitTest(target). 2012 GFxValue::ObjectInterface's AS2 entry; the point
// form takes STAGE coordinates in pixels, which is why the content passes `_root._xmouse`.
// `_common.SelectionHandler::RightClick` is the caller in this cook.
bool GFxHitTestClipAtStagePoint(GFxSprite* sprite, float stageX, float stageY, bool testShape)
{
    if (sprite == 0)
        return false;
    // Stage pixels -> twips, then down through every ancestor's matrix to the clip's own space.
    GMatrix2D world;
    world.SetIdentity();
    GFxASCharacter* chain[32];
    int n = 0;
    for (GFxASCharacter* p = sprite; p != 0 && n < 32; p = p->pParent)
        chain[n++] = p;
    for (int i = n - 1; i >= 0; --i)
        world.Prepend(chain[i]->GetMatrix());
    GPoint<float> pt(stageX * 20.f, stageY * 20.f);
    GPoint<float> local;
    world.TransformByInverse(&local, pt);
    return sprite->PointTestLocal(local, testShape ? GFxCharacter::HitTest_TestShape : 0);
}
