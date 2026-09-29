// Scaleform GFx 3.3.89 - the ActionScript built-in surface the Dishonored New Game flow calls and
// that packages BC..DQ did not install: the MovieClip drawing API, flash.geom.Matrix,
// flash.display.BitmapData with its loadBitmap, and TextField.getTextFormat with
// TextFormat.getTextExtent. Package EA.
//
// MEASURED, on one keyboard-driven run of the user's own path (main menu -> NEW GAME -> a difficulty
// -> brightness), the AS2 errors this file answers, by receiver:
//
//   54  loadBitmap        on undefined   flash.display.BitmapData does not exist
//    8  lineTo            on Sprite      _common.HelpBar's strip and _common.ItemsList's mouse area
//    2  moveTo            on Sprite
//    2  endFill           on Sprite
//    2  beginBitmapFill   on Sprite
//    2  new 'Matrix' is not a constructor              flash.geom.Matrix does not exist
//    1  getTextFormat     on EditText    _common.TitleBar measures its own label
//    1  getTextExtent     on undefined   the TextFormat the line above could not return
//
// The retail bodies, all resolved against retail2013_named.i64 by this package:
//
//   the MovieClip built-in table                         data 0xdfb018..0xdfb160, name/function pairs
//   GFxSprite drawing entry points   clear 0x9ec060  beginFill 0x9ede80  beginGradientFill 0x9ee7f0
//                                    beginBitmapFill 0x9f2d80  lineGradientStyle 0x9ee850
//                                    endFill 0x9ee8b0  lineStyle 0x9ee900  moveTo 0x9eecd0
//                                    lineTo 0x9ec0c0  curveTo 0x9ec150  attachBitmap 0x9f66d0
//   GFxSprite::MoveTo 0x9ede30  LineTo 0x9ebe00  CurveTo 0x9ebea0  AcquirePath 0x9ebf60
//                     SetLineStyle 0x9edce0
//   GFxDrawingContext                ctor 0xa83950  MoveTo 0xa83620  LineTo 0xa83650
//                                    CurveTo 0xa83680  Clear 0xa836c0  SetNoLine 0xa83600
//                                    SameLineStyle 0xa83710  NoLine 0xa837d0  SetNonZeroFill 0xa837f0
//                                    AddPath 0xa83810  SetLineStyle 0xa839c0  SetNewFill 0xa83ab0
//                                    SetFill 0xa83b80  SetBitmapFill 0xa83be0  AcquirePath 0xa83c40
//                                    ComputeBound 0xa83cd0  Display 0xa83cf0
//                                    DefPointTestLocal 0xa83e10
//   GASBitmapData                    ctor 0xa7ab30  commonInit 0xa7a8a0  SetImage 0xa7ac30
//                                    GetMember 0xa7a990  SetMember 0xa7a760  GetObjectType 0xa7a7f0
//                                    LoadBitmapA(GASFnCall) 0xa7b640  LoadBitmapA 0xa7b560
//                                    Register 0xa7b710  GlobalCtor 0xa7ae00
//   GFx_LoadBitmap<GASString>        0xa7b240   GFx_LoadBitmap<GString> 0xa7aef0
//   GASTextFormatProto::GetTextExtent 0xa680b0  GASTextFormatObject::SetTextFormat 0xa678f0
//   GFxEditTextCharacter::GetTextFormat 0xa1edf0
//
// DEVIATION, stated once and deliberately. Retail's GFxDrawingContext packs its path into a
// GFxPathPacker over a GFxShapeWithStyles - the lazily decoded blob form that agentCD.md already
// records this tree as not reproducing (GFxShapeRecord decodes at load instead). So the drawing
// context here builds a run-time GFxShapeCharacterDef, the same class the tag loader builds from a
// DefineShape record, and the whole existing path - the trapezoid tessellator, the fill styles, the
// mask/stencil pass, GFxShapeCharacterDef::Display and its DefPointTestLocal - draws and hit-tests it
// unchanged. What retail's packer and this differ in is the moment of decoding, which is agent CD's
// existing deviation and not a new one; the semantics of every entry point below are retail's, read
// out of the bodies named above.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"
#include "GFxDisplay.h"
#include "GFxTextField.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

static void GFxDrawStrCopy(char* dst, unsigned int cap, const char* src)
{
    if (cap == 0)
        return;
    if (src == 0)
    {
        dst[0] = 0;
        return;
    }
    unsigned int i = 0;
    for (; i + 1 < cap && src[i]; ++i)
        dst[i] = src[i];
    dst[i] = 0;
}

// ---------------------------------------------------------------------------------------------
// GFxDrawingContext

GFxDrawingContext::GFxDrawingContext()                                 // 2013 0xa83950
    : pShape(0), pOpenPath(0), Fill0(0), Fill1(0), Line(0),
      StartX(0), StartY(0), PenX(0), PenY(0), bNewShape(false)
{
    Reset();
}

GFxDrawingContext::~GFxDrawingContext()
{
    delete pOpenPath;
    if (pShape)
    {
        GFxDisplayInvalidateShapeMesh(pShape);
        delete pShape;
    }
}

void GFxDrawingContext::Reset()
{
    delete pOpenPath;
    pOpenPath = 0;
    if (pShape)
    {
        GFxDisplayInvalidateShapeMesh(pShape);
        delete pShape;
    }
    // Tag code 32 is DefineShape3, the dialect whose fills carry an alpha channel - and the drawing
    // API's colours always do, because beginFill's second argument is a percentage.
    pShape = new GFxShapeCharacterDef(32);
    Fill0 = Fill1 = Line = 0;
    StartX = StartY = PenX = PenY = 0;
    bNewShape = false;
}

// 2013 0xa836c0. Retail throws the whole GFxShapeWithStyles away and builds a fresh one; the mesh
// cached against the old definition has to go with it, or the next Display draws what was cleared.
void GFxDrawingContext::Clear()
{
    Reset();
}

// 2013 0xa83c40: close the open contour and hand it to the shape.
bool GFxDrawingContext::AcquirePath(bool newShape)
{
    bool had = false;
    if (pOpenPath != 0)
    {
        if (pOpenPath->EdgeCount != 0)
        {
            pShape->Shape.AddPath(pOpenPath);           // takes ownership
            GFxDisplayInvalidateShapeMesh(pShape);
            had = true;
        }
        else
        {
            delete pOpenPath;
        }
        pOpenPath = 0;
    }
    bNewShape = newShape;
    return had;
}

void GFxDrawingContext::OpenPath()
{
    if (pOpenPath != 0)
        return;
    pOpenPath = new GFxShapePathCD();
    pOpenPath->Fill0 = Fill0;
    pOpenPath->Fill1 = Fill1;
    pOpenPath->Line = Line;
    pOpenPath->StartX = PenX;
    pOpenPath->StartY = PenY;
}

// 2013 0xa83620, through GFxSprite::MoveTo (0x9ede30): the arguments are PIXELS and the path is in
// twips. A move ends the contour that was open, which is what SetMoveTo does.
void GFxDrawingContext::MoveTo(float x, float y)
{
    AcquirePath(bNewShape);
    PenX = (int)(x * GFxPixelsToTwips);
    PenY = (int)(y * GFxPixelsToTwips);
    StartX = PenX;
    StartY = PenY;
}

// 2013 0xa83650
void GFxDrawingContext::LineTo(float x, float y)
{
    OpenPath();
    GFxShapeEdgeCD e;
    e.Ax = (int)(x * GFxPixelsToTwips);
    e.Ay = (int)(y * GFxPixelsToTwips);
    e.Cx = e.Ax;
    e.Cy = e.Ay;
    e.bCurve = false;
    pOpenPath->AddEdge(e);
    PenX = e.Ax;
    PenY = e.Ay;
}

// 2013 0xa83680
void GFxDrawingContext::CurveTo(float cx, float cy, float ax, float ay)
{
    OpenPath();
    GFxShapeEdgeCD e;
    e.Cx = (int)(cx * GFxPixelsToTwips);
    e.Cy = (int)(cy * GFxPixelsToTwips);
    e.Ax = (int)(ax * GFxPixelsToTwips);
    e.Ay = (int)(ay * GFxPixelsToTwips);
    e.bCurve = true;
    pOpenPath->AddEdge(e);
    PenX = e.Ax;
    PenY = e.Ay;
}

// 2013 0xa83b80. A new style is appended and becomes the current fill; the index a path stores is
// one-based, which is the SWF convention GFxShapePathCD already carries (0 means "no style").
void GFxDrawingContext::SetFill(GColor c)
{
    AcquirePath(bNewShape);
    GFxFillStyle f;
    f.Type = GFxFill_Solid;
    f.Color = c;
    pShape->AddFill(f);
    Fill0 = pShape->GetFillStyleCount();
    Fill1 = 0;
}

// 2013 0xa83be0 -> SetNewFill 0xa83ab0 -> GFxFillStyle::SetImageFill. The type is retail's: repeat
// picks the tiled group, smoothing the smooth one.
void GFxDrawingContext::SetBitmapFill(GFxImageCharacterDef* image, const GMatrix2D& m,
                                      bool repeat, bool smooth)
{
    AcquirePath(bNewShape);
    GFxFillStyle f;
    f.Type = (unsigned char)(repeat ? (smooth ? GFxFill_TiledSmoothImage : GFxFill_TiledImage)
                                    : (smooth ? GFxFill_ClippedSmoothImage : GFxFill_ClippedImage));
    f.pDirectImage = image;
    f.Matrix = m;
    pShape->AddFill(f);
    Fill0 = pShape->GetFillStyleCount();
    Fill1 = 0;
}

// The no-argument arm of beginFill and the whole of endFill (2013 0x9ede80 and 0x9ee8b0 both write 0
// into the context's two fill slots).
void GFxDrawingContext::SetNoFill()
{
    AcquirePath(bNewShape);
    Fill0 = 0;
    Fill1 = 0;
}

// 2013 0xa839c0, through GFxSprite::SetLineStyle (0x9edce0). The width is in pixels, the style in
// twips.
void GFxDrawingContext::SetLineStyle(float width, GColor c)
{
    AcquirePath(bNewShape);
    GFxLineStyle* l = new GFxLineStyle();
    float twips = width * GFxPixelsToTwips;
    if (twips < 0.f)
        twips = 0.f;
    if (twips > 65535.f)
        twips = 65535.f;
    l->Width = (unsigned short)twips;
    l->Color = c;
    pShape->AddLine(l);
    Line = pShape->GetLineStyleCount();
}

// 2013 0xa83600
void GFxDrawingContext::SetNoLine()
{
    AcquirePath(bNewShape);
    Line = 0;
}

bool GFxDrawingContext::IsEmpty() const
{
    return pShape == 0 || (pShape->Shape.GetPathCount() == 0 && pOpenPath == 0);
}

// 2013 0xa83cd0. The bound the geometry setters derive _width and _height from.
GRect<int> GFxDrawingContext::ComputeBound()
{
    AcquirePath(bNewShape);
    GRect<int> empty;
    empty.Left = empty.Top = empty.Right = empty.Bottom = 0;
    if (pShape == 0 || pShape->Shape.GetPathCount() == 0)
        return empty;
    pShape->Bounds = pShape->Shape.ComputeBound();
    return pShape->Bounds;
}

// 2013 0xa83cf0: close the path, then draw the shape with the transform GFxSprite::Display has
// already pushed for this character.
void GFxDrawingContext::Display(GFxDisplayContext& ctx, GFxCharacter* ch)
{
    AcquirePath(bNewShape);
    if (pShape == 0 || pShape->Shape.GetPathCount() == 0)
        return;
    pShape->Bounds = pShape->Shape.ComputeBound();
    pShape->Display(ctx, ch);
}

// 2013 0xa83e10
bool GFxDrawingContext::PointTestLocal(const GPoint<float>& pt, bool testShape,
                                       const GFxCharacter* inst)
{
    AcquirePath(bNewShape);
    if (pShape == 0 || pShape->Shape.GetPathCount() == 0)
        return false;
    return pShape->DefPointTestLocal(pt, testShape, inst);
}

// ---------------------------------------------------------------------------------------------
// The AS2 entry points. Each mirrors the retail wrapper named above: the receiver is resolved the
// same way (the `this` of the call, or the environment's own target when the call had none), the
// arguments are read in the same order, and a call with too few arguments is a no-op.

namespace
{

// The member a BitmapData keeps its image in. Retail's GASBitmapData is a class of its own with a
// GFxImageResource at +36 and a GetMember override for width and height (0xa7a990); this is a plain
// object with the same three script-visible answers, as Key, Mouse, Stage, ExternalInterface and
// fscommand already are in this tree (agentDG.md deviation 1, agentDJ.md deviation 7).
const char* const BitmapDataImageMember = "__gfxImageDef";
// The member a TextFormat keeps the field it was taken from in, so getTextExtent measures with that
// field's own font and size - which is what retail's format carries as a font handle.
const char* const TextFormatFieldMember = "__gfxTextField";

GFxSprite* DrawTarget(const GASFnCall& fn)
{
    // 2013 0x9ec060's first block: when `this` is an object it must be a sprite (object type 2);
    // with no `this` the environment's own target is used.
    if (fn.pThis != 0)
    {
        GFxASCharacter* ch = fn.pThis->ToASCharacter();
        if (ch)
            return ch->ToSprite();
        return 0;
    }
    if (fn.pEnv == 0)
        return 0;
    GFxASCharacter* t = fn.pEnv->GetTarget();
    return t ? t->ToSprite() : 0;
}

// beginFill and lineStyle build their colour the same way (2013 0x9ede80, 0x9ee900): the RGB
// argument with the alpha byte replaced by the percentage argument scaled to 0..255 and clamped.
GColor ArgColor(const GASFnCall& fn, int rgbArg, int alphaArg)
{
    unsigned int rgb = 0;
    if (fn.GetNumArgs() > rgbArg)
        rgb = fn.Arg(rgbArg).ToUInt32(fn.pEnv);
    unsigned int alpha = 255;
    if (fn.GetNumArgs() > alphaArg)
    {
        double a = fn.Arg(alphaArg).ToNumber(fn.pEnv) * 255.0 / 100.0;
        if (!(a > 0.0))
            a = 0.0;
        if (a >= 255.0)
            a = 255.0;
        alpha = (unsigned int)a;
    }
    GColor c;
    c.SetRaw((rgb & 0x00FFFFFFu) | (alpha << 24));
    return c;
}

double ObjNumber(GASObject* o, GASStringContext* sc, const char* name, double def)
{
    if (o == 0)
        return def;
    GASValue v;
    if (o->GetMemberRaw(sc, sc->CreateString(name), &v) && !v.IsUndefined())
        return v.ToNumber(0);
    return def;
}

void ObjSetNumber(GASObject* o, GASStringContext* sc, const char* name, double d)
{
    GASValue v;
    v.SetNumber(d);
    o->SetMemberRaw(sc, sc->CreateString(name), v, GASPropFlags());
}

GASObject* ArgObject(const GASFnCall& fn, int index)
{
    if (fn.GetNumArgs() <= index)
        return 0;
    const GASValue& v = fn.Arg(index);
    return v.IsObject() ? v.GetObject() : 0;
}

GFxImageCharacterDef* BitmapImageOf(GASObject* o, GASStringContext* sc)
{
    if (o == 0)
        return 0;
    GASValue v;
    if (!o->GetMemberRaw(sc, sc->CreateString(BitmapDataImageMember), &v) || v.IsUndefined())
        return 0;
    return (GFxImageCharacterDef*)(size_t)(unsigned int)v.ToInt32(0);
}

void MCClear(const GASFnCall& fn)                                      // 2013 0x9ec060
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0)
        return;
    if (s->HasDrawing())
        s->GetDrawing()->Clear();
}

void MCBeginFill(const GASFnCall& fn)                                  // 2013 0x9ede80
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0)
        return;
    if (fn.GetNumArgs() <= 0)
    {
        s->GetDrawing()->SetNoFill();
        return;
    }
    s->GetDrawing()->SetFill(ArgColor(fn, 0, 1));
}

void MCBeginBitmapFill(const GASFnCall& fn)                            // 2013 0x9f2d80
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0 || fn.GetNumArgs() < 1 || fn.pEnv == 0)
        return;
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    GFxImageCharacterDef* image = BitmapImageOf(ArgObject(fn, 0), sc);
    if (image == 0)
    {
        s->GetDrawing()->SetNoFill();
        return;
    }
    // The AS2 matrix maps the image's PIXELS onto the clip's pixels; a SWF fill matrix maps the
    // image's own space onto the shape's TWIPS and already carries the twips factor, which is the
    // convention GFxDisplayApplyFill derives its texture matrix against (agentDC.md 5.4, measured
    // against two real fills). Multiplying the whole matrix by 20 is exactly that change of units.
    GMatrix2D m;
    m.SetIdentity();
    GASObject* mo = ArgObject(fn, 1);
    if (mo)
    {
        m.M_[0][0] = (float)ObjNumber(mo, sc, "a", 1.0);
        m.M_[1][0] = (float)ObjNumber(mo, sc, "b", 0.0);
        m.M_[0][1] = (float)ObjNumber(mo, sc, "c", 0.0);
        m.M_[1][1] = (float)ObjNumber(mo, sc, "d", 1.0);
        m.M_[0][2] = (float)ObjNumber(mo, sc, "tx", 0.0);
        m.M_[1][2] = (float)ObjNumber(mo, sc, "ty", 0.0);
    }
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 3; ++c)
            m.M_[r][c] *= GFxPixelsToTwips;
    const bool repeat = fn.GetNumArgs() > 2 ? fn.Arg(2).ToBool(fn.pEnv) : true;
    const bool smooth = fn.GetNumArgs() > 3 ? fn.Arg(3).ToBool(fn.pEnv) : false;
    s->GetDrawing()->SetBitmapFill(image, m, repeat, smooth);
}

void MCEndFill(const GASFnCall& fn)                                    // 2013 0x9ee8b0
{
    GFxSprite* s = DrawTarget(fn);
    if (s)
        s->GetDrawing()->SetNoFill();
}

void MCLineStyle(const GASFnCall& fn)                                  // 2013 0x9ee900
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0)
        return;
    if (fn.GetNumArgs() <= 0)
    {
        s->GetDrawing()->SetNoLine();
        return;
    }
    s->GetDrawing()->SetLineStyle((float)fn.Arg(0).ToNumber(fn.pEnv), ArgColor(fn, 1, 2));
}

void MCMoveTo(const GASFnCall& fn)                                     // 2013 0x9eecd0
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0 || fn.GetNumArgs() < 2)
        return;
    s->GetDrawing()->MoveTo((float)fn.Arg(0).ToNumber(fn.pEnv),
                            (float)fn.Arg(1).ToNumber(fn.pEnv));
}

void MCLineTo(const GASFnCall& fn)                                     // 2013 0x9ec0c0
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0 || fn.GetNumArgs() < 2)
        return;
    s->GetDrawing()->LineTo((float)fn.Arg(0).ToNumber(fn.pEnv),
                            (float)fn.Arg(1).ToNumber(fn.pEnv));
}

void MCCurveTo(const GASFnCall& fn)                                    // 2013 0x9ec150
{
    GFxSprite* s = DrawTarget(fn);
    if (s == 0 || fn.GetNumArgs() < 4)
        return;
    s->GetDrawing()->CurveTo((float)fn.Arg(0).ToNumber(fn.pEnv),
                             (float)fn.Arg(1).ToNumber(fn.pEnv),
                             (float)fn.Arg(2).ToNumber(fn.pEnv),
                             (float)fn.Arg(3).ToNumber(fn.pEnv));
}

// 2013 0xa7b640 -> GFx_LoadBitmap<GASString> 0xa7b240. Retail resolves the name against the movie's
// export table through GFxMovieRoot::FindExportedResource and accepts it only when the resource is
// an image (`(type & 0xFF00) == 0x100`, GFxResource::RT_Image); the `img://` and `imgps://` prefixes
// go to the external image loader instead, and this cook uses neither - every one of the 27 names
// _common.EmbedImg asks for is an ExportAssets symbol of the shared library.
void BitmapDataLoadBitmap(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetUndefined();
    if (fn.GetNumArgs() < 1 || fn.pEnv == 0)
        return;
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    if (root == 0)
        return;
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    const GASString name = fn.Arg(0).ToString(fn.pEnv);
    GFxMovieDefImpl* impl = root->GetMovieDefImpl();
    GFxMovieDataDef* dataDef = impl ? impl->GetDataDef() : 0;
    GFxCharacterDef* def = dataDef ? dataDef->FindExportedCharacter(name.ToCStr()) : 0;
    if (def == 0 || def->GetResourceTypeCode() != GFxResource::RT_Image)
    {
        root->LogScriptError("BitmapData.loadBitmap: no exported image '%s'", name.ToCStr());
        return;
    }
    GFxImageCharacterDef* image = (GFxImageCharacterDef*)def;
    GASObject* o = new GASObject(sc, fn.pEnv->GetGC()->GetPrototype(GASGlobalContext::Proto_Object));
    unsigned int w = image->TargetWidth, h = image->TargetHeight;
    ObjSetNumber(o, sc, "width", (double)w);
    ObjSetNumber(o, sc, "height", (double)h);
    GASValue v;
    v.SetInt((int)(unsigned int)(size_t)image);
    o->SetMemberRaw(sc, sc->CreateString(BitmapDataImageMember), v,
                    GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    if (fn.pResult)
        fn.pResult->SetAsObject(o);
}

// flash.geom.Matrix. The six constructor arguments are Flash's (a, b, c, d, tx, ty) and default to
// the identity.
void MatrixCtor(const GASFnCall& fn)
{
    GASObject* o = fn.pThis ? fn.pThis->ToASObject() : 0;
    if (o == 0 || fn.pEnv == 0)
        return;
    static const char* const names[6] = { "a", "b", "c", "d", "tx", "ty" };
    static const double identity[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    for (int i = 0; i < 6; ++i)
        ObjSetNumber(o, sc, names[i],
                     fn.GetNumArgs() > i ? fn.Arg(i).ToNumber(fn.pEnv) : identity[i]);
}

void MatrixIdentity(const GASFnCall& fn)
{
    GASObject* o = fn.pThis ? fn.pThis->ToASObject() : 0;
    if (o == 0 || fn.pEnv == 0)
        return;
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    ObjSetNumber(o, sc, "a", 1.0);  ObjSetNumber(o, sc, "b", 0.0);
    ObjSetNumber(o, sc, "c", 0.0);  ObjSetNumber(o, sc, "d", 1.0);
    ObjSetNumber(o, sc, "tx", 0.0); ObjSetNumber(o, sc, "ty", 0.0);
}

void MatrixTranslate(const GASFnCall& fn)
{
    GASObject* o = fn.pThis ? fn.pThis->ToASObject() : 0;
    if (o == 0 || fn.pEnv == 0 || fn.GetNumArgs() < 2)
        return;
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    ObjSetNumber(o, sc, "tx", ObjNumber(o, sc, "tx", 0.0) + fn.Arg(0).ToNumber(fn.pEnv));
    ObjSetNumber(o, sc, "ty", ObjNumber(o, sc, "ty", 0.0) + fn.Arg(1).ToNumber(fn.pEnv));
}

void MatrixScale(const GASFnCall& fn)
{
    GASObject* o = fn.pThis ? fn.pThis->ToASObject() : 0;
    if (o == 0 || fn.pEnv == 0 || fn.GetNumArgs() < 2)
        return;
    GASStringContext* sc = fn.pEnv->GetGC()->GetSC();
    const double sx = fn.Arg(0).ToNumber(fn.pEnv);
    const double sy = fn.Arg(1).ToNumber(fn.pEnv);
    ObjSetNumber(o, sc, "a",  ObjNumber(o, sc, "a", 1.0) * sx);
    ObjSetNumber(o, sc, "b",  ObjNumber(o, sc, "b", 0.0) * sx);
    ObjSetNumber(o, sc, "c",  ObjNumber(o, sc, "c", 0.0) * sy);
    ObjSetNumber(o, sc, "d",  ObjNumber(o, sc, "d", 1.0) * sy);
    ObjSetNumber(o, sc, "tx", ObjNumber(o, sc, "tx", 0.0) * sx);
    ObjSetNumber(o, sc, "ty", ObjNumber(o, sc, "ty", 0.0) * sy);
}

void EmptyCtor(const GASFnCall& fn)
{
    (void)fn;
}

// TextFormat.getTextExtent(text [, width]) - 2013 0xa680b0. Retail lays the string out with the
// format's own font and returns width, height, textFieldWidth, textFieldHeight, ascent and descent.
// This measures with the field the format was taken from, which is the same text engine, the same
// font and the same size - and it is the field whose extent the two callers are asking about.
void TextFormatGetTextExtent(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetUndefined();
    if (fn.GetNumArgs() < 1 || fn.pEnv == 0)
        return;
    GASGlobalContext* gc = fn.pEnv->GetGC();
    GASStringContext* sc = gc->GetSC();
    GASObject* self = fn.pThis ? fn.pThis->ToASObject() : 0;
    const GASString text = fn.Arg(0).ToString(fn.pEnv);

    float width = 0.f, height = 0.f;
    GASValue fieldVal;
    if (self && self->GetMemberRaw(sc, sc->CreateString(TextFormatFieldMember), &fieldVal)
        && fieldVal.IsCharacter())
    {
        GFxASCharacter* ch = fieldVal.GetCharacter();
        GFxEditTextCharacter* field =
            (ch && ch->GetObjectType() == Object_TextField) ? (GFxEditTextCharacter*)ch : 0;
        if (field)
        {
            // Measure by formatting the string in the field's own document and putting the field's
            // own text back, which is what makes the measurement carry the field's font, size and
            // paragraph format. The restore is the same call the content makes a line later.
            char saved[512];
            GFxDrawStrCopy(saved, sizeof(saved), field->GetTextValue());
            field->SetTextValue(text.ToCStr(), false, false);
            width = field->GetDocView().GetTextWidth() * GFxTwipsToPixels;
            height = field->GetDocView().GetTextHeight() * GFxTwipsToPixels;
            field->SetTextValue(saved, false, false);
        }
    }

    GASObject* out = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
    ObjSetNumber(out, sc, "width", (double)width);
    ObjSetNumber(out, sc, "height", (double)height);
    ObjSetNumber(out, sc, "textFieldWidth", (double)width + 4.0);
    ObjSetNumber(out, sc, "textFieldHeight", (double)height + 4.0);
    ObjSetNumber(out, sc, "ascent", (double)height);
    ObjSetNumber(out, sc, "descent", 0.0);
    if (fn.pResult)
        fn.pResult->SetAsObject(out);
}

GASObject* GTextFormatProto = 0;
GASObject* GTextFieldProto = 0;

// 2013 0xa1edf0. TextField.getTextFormat() answers a TextFormat for the field's current format.
void TextFieldGetTextFormat(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetUndefined();
    if (fn.pEnv == 0 || fn.pThis == 0)
        return;
    GFxASCharacter* ch = fn.pThis->ToASCharacter();
    GFxEditTextCharacter* field =
            (ch && ch->GetObjectType() == Object_TextField) ? (GFxEditTextCharacter*)ch : 0;
    if (field == 0)
        return;
    GASGlobalContext* gc = fn.pEnv->GetGC();
    GASStringContext* sc = gc->GetSC();
    GASObject* proto = GTextFormatProto ? GTextFormatProto
                                        : gc->GetPrototype(GASGlobalContext::Proto_Object);
    GASObject* fmt = new GASObject(sc, proto);
    GASValue v;
    v.SetAsCharacter(ch);
    fmt->SetMemberRaw(sc, sc->CreateString(TextFormatFieldMember), v,
                      GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    if (fn.pResult)
        fn.pResult->SetAsObject(fmt);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Installation. Retail registers BitmapData and TextFormat through
// GASGlobalContext::AddBuiltinClassRegistry (0xa7b710 and 0xa68dc0); the drawing methods are not a
// class at all - they are entries of the MovieClip built-in table at 0xdfb018, which this tree
// answers from MovieClip.prototype. agentDG.md deviation 2 records that the prototype is consulted
// LAST here; every content class in this cook extends MovieClip, so the chain still reaches it.
void GFxDrawingInstall(GASGlobalContext* gc, GASObject* global, GASObject* movieClipProto)
{
    GASStringContext* sc = gc->GetSC();

    struct Entry { const char* Name; GASCFunctionPtr Fn; };
    static const Entry mcMethods[] =
    {
        { "clear",           MCClear },
        { "beginFill",       MCBeginFill },
        { "beginBitmapFill", MCBeginBitmapFill },
        { "endFill",         MCEndFill },
        { "lineStyle",       MCLineStyle },
        { "moveTo",          MCMoveTo },
        { "lineTo",          MCLineTo },
        { "curveTo",         MCCurveTo }
    };
    for (int i = 0; i < (int)(sizeof(mcMethods) / sizeof(mcMethods[0])); ++i)
    {
        GASValue v;
        v.SetAsFunction(gc->NewCFunction(mcMethods[i].Fn));
        movieClipProto->SetConstMemberRaw(sc, mcMethods[i].Name, v,
                                          GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    // flash.display.BitmapData and flash.geom.Matrix hang off the `flash` object agent DJ built for
    // ExternalInterface, which is already on _global by the time this runs.
    GASValue flashVal;
    global->GetConstMemberRaw(sc, "flash", &flashVal);
    GASObject* flash = flashVal.IsObject() ? flashVal.GetObject() : 0;

    {
        GASObject* bitmapProto = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
        GASFunctionObject* ctor = gc->NewCFunction(EmptyCtor);
        GASValue v;
        v.SetAsObject(bitmapProto);
        ctor->SetConstMemberRaw(sc, "prototype", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        v.SetAsFunction(gc->NewCFunction(BitmapDataLoadBitmap));
        ctor->SetConstMemberRaw(sc, "loadBitmap", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASValue ctorVal;
        ctorVal.SetAsFunction(ctor);
        if (flash)
        {
            GASObject* display = new GASObject(sc,
                                               gc->GetPrototype(GASGlobalContext::Proto_Object));
            display->SetConstMemberRaw(sc, "BitmapData", ctorVal,
                                       GASPropFlags(GASPropFlags::PropFlag_DontEnum));
            v.SetAsObject(display);
            flash->SetConstMemberRaw(sc, "display", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        }
        // Unqualified, which is what `import flash.display.*` compiles to.
        global->SetConstMemberRaw(sc, "BitmapData", ctorVal,
                                  GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    {
        GASObject* matrixProto = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
        GASValue v;
        v.SetAsFunction(gc->NewCFunction(MatrixIdentity));
        matrixProto->SetConstMemberRaw(sc, "identity", v,
                                       GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        v.SetAsFunction(gc->NewCFunction(MatrixTranslate));
        matrixProto->SetConstMemberRaw(sc, "translate", v,
                                       GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        v.SetAsFunction(gc->NewCFunction(MatrixScale));
        matrixProto->SetConstMemberRaw(sc, "scale", v,
                                       GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASFunctionObject* ctor = gc->NewCFunction(MatrixCtor);
        v.SetAsObject(matrixProto);
        ctor->SetConstMemberRaw(sc, "prototype", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASValue ctorVal;
        ctorVal.SetAsFunction(ctor);
        if (flash)
        {
            GASObject* geom = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
            geom->SetConstMemberRaw(sc, "Matrix", ctorVal,
                                    GASPropFlags(GASPropFlags::PropFlag_DontEnum));
            v.SetAsObject(geom);
            flash->SetConstMemberRaw(sc, "geom", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        }
        global->SetConstMemberRaw(sc, "Matrix", ctorVal,
                                  GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    // TextFormat, for the object getTextFormat answers with. getTextExtent is the only method the
    // cook calls on it; retail's other GASTextFormatProto entries are the format's own properties,
    // which are ordinary members here.
    {
        GASObject* proto = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
        GASValue v;
        v.SetAsFunction(gc->NewCFunction(TextFormatGetTextExtent));
        proto->SetConstMemberRaw(sc, "getTextExtent", v,
                                 GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASFunctionObject* ctor = gc->NewCFunction(EmptyCtor);
        v.SetAsObject(proto);
        ctor->SetConstMemberRaw(sc, "prototype", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASValue ctorVal;
        ctorVal.SetAsFunction(ctor);
        global->SetConstMemberRaw(sc, "TextFormat", ctorVal,
                                  GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GTextFormatProto = proto;
    }

    // The TextField prototype, which is what an EditText character's member lookup falls through to
    // (GFxEditTextCharacter::GetMember). Retail's is GASTextFieldProto, 2013 0xa21120.
    {
        GASObject* proto = new GASObject(sc, gc->GetPrototype(GASGlobalContext::Proto_Object));
        GASValue v;
        v.SetAsFunction(gc->NewCFunction(TextFieldGetTextFormat));
        proto->SetConstMemberRaw(sc, "getTextFormat", v,
                                 GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GTextFieldProto = proto;
    }
}

GASObject* GFxDrawingTextFieldProto()
{
    return GTextFieldProto;
}
