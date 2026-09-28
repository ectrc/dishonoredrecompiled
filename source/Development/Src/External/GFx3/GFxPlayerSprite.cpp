// Scaleform GFx 3.3.89 - GFxCharacter, GFxASCharacter, GFxDisplayList and GFxSprite. Package BC.
//
// The frame machine is the port of five retail bodies, and the order they impose is not negotiable:
//   GFxSprite::AdvanceFrame (2012 0x9f93f0)  increments the frame, then IncrementFrameAndCheckForLoop,
//     and only if the frame number actually changed does it run ExecuteInitActionFrameTags, then the
//     EnterFrame event, then ExecuteFrameTags. When it did not change it fires EnterFrame alone.
//   GFxSprite::ExecuteFrameTags (0x9f60a0)   init actions first, then the playlist, each tag through
//     ExecuteWithPriority(this, GFxAP_Frame).
//   GFxSprite::ExecuteInitActionFrameTags (0x9f4640)  guarded by a per-frame byte so the init actions
//     of a frame run exactly once for the life of the instance.
//   GFxSprite::IncrementFrameAndCheckForLoop (0x9f4500)  wraps to 0 past the end and, when the
//     timeline has more than one frame, marks every display-list entry for removal so the next frame's
//     PlaceObject tags rebuild the list.
//   GFxSprite::CallFrameActions (0x9f63b0)   opens an action-queue session, runs only the tags that
//     answer IsActionTag(), and drains the session.
// DISHONORED(port): see GFxAS2.h.
#include "GFxPlayer.h"
#include "GFxCharacterDefs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <set>

// ---------------------------------------------------------------------------------------------
// GFxCharacterHandle

GFxCharacterHandle::GFxCharacterHandle(const GASString& name, GFxASCharacter* parent,
                                      GFxASCharacter* ch)
    : RefCount(1), Name(name), pParent(parent), pCharacter(ch) {}

// ---------------------------------------------------------------------------------------------
// GFxCharacter

GFxCharacter::GFxCharacter(GFxASCharacter* parent, GFxResourceId id)
    : RefCount(1), pParent(parent), Depth(0), ClipDepth(0), Ratio(0.f), bVisible(true)
{
    Id = id;
    for (int i = 0; i < 4; ++i) { ColorTransform.M_[i][0] = 1.f; ColorTransform.M_[i][1] = 0.f; }
}

GFxCharacter::~GFxCharacter() {}

// ---------------------------------------------------------------------------------------------
// GFxDisplayList

GFxDisplayList::GFxDisplayList() : Entries(0), Size(0), Capacity(0) {}

GFxDisplayList::~GFxDisplayList()
{
    Clear();
    free(Entries);
}

bool GFxSprite::bCheckDisplayList = false;
static std::set<const void*>* GFxDK_LiveChars = 0;

bool GFxDK_LiveCharsOn()
{
    return GFxSprite::bCheckDisplayList;
}

void GFxDK_LiveCharAdd(const GFxASCharacter* c)
{
    if (GFxDK_LiveChars == 0)
        GFxDK_LiveChars = new std::set<const void*>();
    GFxDK_LiveChars->insert((const void*)c);
}

void GFxDK_LiveCharRemove(const GFxASCharacter* c)
{
    if (GFxDK_LiveChars != 0)
        GFxDK_LiveChars->erase((const void*)c);
}

bool GFxDK_LiveCharIs(const GFxCharacter* c)
{
    return GFxDK_LiveChars != 0 && GFxDK_LiveChars->find((const void*)c) != GFxDK_LiveChars->end();
}

void GFxDisplayList::InsertAt(unsigned int index, GFxCharacter* ch)
{
    if (Size >= Capacity)
    {
        Capacity = Capacity ? Capacity * 2 : 8;
        Entries = (DisplayEntry*)realloc(Entries, Capacity * sizeof(DisplayEntry));
    }
    for (unsigned int i = Size; i > index; --i)
        Entries[i] = Entries[i - 1];
    Entries[index].pChar = ch;
    Entries[index].bMarkedForRemove = false;
    ++Size;
}

void GFxDisplayList::RemoveAt(unsigned int index)
{
    if (index >= Size)
        return;
    // DISHONORED(bringup): detach before releasing. The destructor destroys this character's own
    // children and each of those runs script, which reads display lists by name.
    GFxCharacter* ch = Entries[index].pChar;
    for (unsigned int i = index; i + 1 < Size; ++i)
        Entries[i] = Entries[i + 1];
    --Size;
    if (ch)
    {
        if (GFxDK_LiveCharsOn())
            GFxLogf("DISHONORED(bringup): dlcheck: RemoveAt releases %p (index %u, size now %u)", (void*)ch, index, Size);
        ch->Release();
    }
}

int GFxDisplayList::FindDisplayIndex(int depth) const                 // 2012 0x9d5a40
{
    // The list is kept sorted by depth, so this is the insertion point of `depth`.
    unsigned int lo = 0, hi = Size;
    while (lo < hi)
    {
        unsigned int mid = (lo + hi) / 2;
        if (Entries[mid].pChar->GetDepth() < depth)
            lo = mid + 1;
        else
            hi = mid;
    }
    return (int)lo;
}

GFxCharacter* GFxDisplayList::GetCharacterAtDepth(int depth, bool* outMarked) const // 0x9d5a80
{
    int i = FindDisplayIndex(depth);
    if (i < (int)Size && Entries[i].pChar->GetDepth() == depth)
    {
        if (outMarked) *outMarked = Entries[i].bMarkedForRemove;
        return Entries[i].pChar;
    }
    if (outMarked) *outMarked = false;
    return 0;
}

// DISHONORED(bringup, agent DK): -gfxuidlcheck. GFxASCharacter registers itself here and unregisters
// in its destructor, so a display list entry that outlives its character can be named instead of
// crashing on the next read of its GASString.
bool GFxDK_LiveCharsOn();
void GFxDK_LiveCharAdd(const GFxASCharacter* c);
void GFxDK_LiveCharRemove(const GFxASCharacter* c);
bool GFxDK_LiveCharIs(const GFxCharacter* c);

GFxCharacter* GFxDisplayList::GetCharacterByName(GASStringContext* sc, const GASString& name) const
{                                                                     // 2012 0x9d5450
    for (unsigned int i = 0; i < Size; ++i)
    {
        GFxCharacter* c = Entries[i].pChar;
        if (GFxDK_LiveCharsOn() && c != 0 && !GFxDK_LiveCharIs(c))
        {
            GFxLogf("DISHONORED(bringup): dlcheck: entry %u of %u in a display list is a DESTROYED character %p (looking for '%s')",
                    i, Size, (void*)c, name.ToCStr());
            continue;
        }
        if (c && c->IsASCharacter())
        {
            const GASString& n = c->ToASCharacterDef()->GetName();
            if (n == name || (sc && sc->IsCaseInsensitive() && n.EqualsNoCase(name)))
                return c;
        }
    }
    return 0;
}

int GFxDisplayList::GetLargestDepthInUse() const                      // 2012 0x9d5430
{
    return Size ? Entries[Size - 1].pChar->GetDepth() : 0;
}

// DISHONORED(port): 2013 0x9cc8f0 (2012 0x9d60f0). A place onto an occupied depth replaces the
// sitting entry. This list never releases the character it is handed and never returns without
// taking it: retail's body does one AddRef, one InsertAt and one Release, so the caller's reference
// always ends up owned by the list. The branch that used to sit here released `ch` and returned when
// the sitting entry was marked for removal and carried the same character id - a revive - and every
// one of the three callers goes on to use `ch` afterwards, so a revived place called virtuals on
// freed memory. That is the crash that killed the main menu about two seconds in (a wild call at
// GFxSprite::AddDisplayObject, `ch->IsASCharacter()`). The revive itself is retail's, but it belongs
// where retail has it: GFxSprite::AddDisplayObject reads the depth before it creates anything.
void GFxDisplayList::AddDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch)
{
    int i = FindDisplayIndex(pos.Depth);
    if (i < (int)Size && Entries[i].pChar->GetDepth() == pos.Depth)
    {
        // DISHONORED(bringup): store before releasing, as in RemoveAt.
        GFxCharacter* old = Entries[i].pChar;
        Entries[i].pChar = ch;
        Entries[i].bMarkedForRemove = false;
        if (old)
        {
            if (GFxDK_LiveCharsOn())
                GFxLogf("DISHONORED(bringup): dlcheck: AddDisplayObject replaces %p with %p at depth %d", (void*)old, (void*)ch, pos.Depth);
            old->Release();
        }
    }
    else
    {
        InsertAt((unsigned int)i, ch);
    }
    ch->SetDepth(pos.Depth);
    if (pos.HasMatrix()) ch->SetMatrix(pos.Matrix);
    if (pos.HasCxform()) ch->SetCxform(pos.ColorTransform);
    if (pos.FilterCount) ch->SetFilters(pos.pFilters, pos.FilterCount);
    ch->SetClipDepth(pos.ClipDepth);
    ch->SetRatio(pos.Ratio);
}

void GFxDisplayList::MoveDisplayObject(const GFxCharPosInfo& pos)      // 2012 0x9d6250
{
    bool marked = false;
    GFxCharacter* ch = GetCharacterAtDepth(pos.Depth, &marked);
    if (ch == 0)
        return;
    if (marked)
    {
        int i = FindDisplayIndex(pos.Depth);
        if (i < (int)Size) Entries[i].bMarkedForRemove = false;
    }
    if (pos.HasMatrix()) ch->SetMatrix(pos.Matrix);
    if (pos.HasCxform()) ch->SetCxform(pos.ColorTransform);
    if (pos.FilterCount) ch->SetFilters(pos.pFilters, pos.FilterCount);
    if (pos.PlaceFlags & GFxCharPosInfo::Place_HasRatio) ch->SetRatio(pos.Ratio);
    if (pos.PlaceFlags & GFxCharPosInfo::Place_HasClipDepth) ch->SetClipDepth(pos.ClipDepth);
}

void GFxDisplayList::ReplaceDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch) // 0x9d6350
{
    int i = FindDisplayIndex(pos.Depth);
    if (i < (int)Size && Entries[i].pChar->GetDepth() == pos.Depth)
    {
        // DISHONORED(bringup): store before releasing, as in RemoveAt.
        GFxCharacter* old = Entries[i].pChar;
        Entries[i].pChar = ch;
        Entries[i].bMarkedForRemove = false;
        if (old)
        {
            if (GFxDK_LiveCharsOn())
                GFxLogf("DISHONORED(bringup): dlcheck: ReplaceDisplayObject replaces %p with %p at depth %d", (void*)old, (void*)ch, pos.Depth);
            old->Release();
        }
    }
    else
    {
        InsertAt((unsigned int)i, ch);
    }
    ch->SetDepth(pos.Depth);
    if (pos.HasMatrix()) ch->SetMatrix(pos.Matrix);
    if (pos.HasCxform()) ch->SetCxform(pos.ColorTransform);
    if (pos.FilterCount) ch->SetFilters(pos.pFilters, pos.FilterCount);
}

void GFxDisplayList::RemoveDisplayObject(int depth, GFxResourceId id)   // 2012 0x9d5fd0
{
    int i = FindDisplayIndex(depth);
    if (i >= (int)Size || Entries[i].pChar->GetDepth() != depth)
        return;
    if (id.Id != GFxResourceId::InvalidId && Entries[i].pChar->GetId().Id != id.Id)
        return;
    // DISHONORED(bringup): as UnloadMarkedObjects - the handler may have removed this entry already.
    GFxCharacter* ch = Entries[i].pChar;
    ch->AddRef();
    ch->OnEventUnload();
    if ((unsigned int)i < Size && Entries[i].pChar == ch)
        RemoveAt((unsigned int)i);
    ch->Release();
}

void GFxDisplayList::MarkAllEntriesForRemoval(unsigned int fromIndex)   // 2012 0x9d5670
{
    for (unsigned int i = fromIndex; i < Size; ++i)
        Entries[i].bMarkedForRemove = true;
}

// DISHONORED(bringup): OnEventUnload runs the clip's own handler and that handler can remove further
// entries, so the index is re-clamped every step and the character is kept alive across the call.
void GFxDisplayList::UnloadMarkedObjects()                            // 2012 0x9d60a0
{
    unsigned int i = Size;
    while (i > 0)
    {
        if (i > Size)
            i = Size;
        else
            --i;
        if (i >= Size || !Entries[i].bMarkedForRemove)
            continue;
        GFxCharacter* ch = Entries[i].pChar;
        if (ch == 0)
            continue;
        ch->AddRef();
        ch->OnEventUnload();
        if (i < Size && Entries[i].pChar == ch)
            RemoveAt(i);
        ch->Release();
    }
}

void GFxDisplayList::UnloadAll()                                      // 2012 0x9d6070
{
    unsigned int i = Size;
    while (i > 0)
    {
        if (i > Size)
            i = Size;
        else
            --i;
        if (i >= Size)
            continue;
        GFxCharacter* ch = Entries[i].pChar;
        if (ch == 0)
            continue;
        ch->AddRef();
        ch->OnEventUnload();
        if (i < Size && Entries[i].pChar == ch)
            RemoveAt(i);
        ch->Release();
    }
}

void GFxDisplayList::Clear()                                          // 2012 0x9d5e30
{
    for (unsigned int i = Size; i > 0; --i)
        RemoveAt(i - 1);
}

// ---------------------------------------------------------------------------------------------
// GFxASCharacter

GFxASCharacter::GFxASCharacter(GFxASCharacter* parent, GFxResourceId id, GFxMovieRoot* root)
    : GFxCharacter(parent, id), pMovieRoot(root), pHandle(0), pASObject(0), pProto(0),
      pGeomData(0)
{
    if (GFxDK_LiveCharsOn())
        GFxDK_LiveCharAdd(this);
}

GFxASCharacter::~GFxASCharacter()
{
    if (GFxDK_LiveCharsOn())
    {
        int listedAt = -1, listedOf = 0;
        GFxSprite* parentSprite = GetParent() ? GetParent()->ToSprite() : 0;
        if (parentSprite != 0)
        {
            const GFxDisplayList& dl = parentSprite->GetDisplayList();
            listedOf = (int)dl.GetCount();
            for (int k = 0; k < listedOf; ++k)
                if (dl.GetAt((unsigned int)k) == (const GFxCharacter*)this)
                    listedAt = k;
        }
        GFxLogf("DISHONORED(bringup): dlcheck: destroying %p name '%s' depth %d parent %p, parent list %d of %d%s",
                (void*)this, Name.ToCStr(), GetDepth(), (void*)GetParent(), listedAt, listedOf,
                listedAt >= 0 ? " STILL LISTED" : "");
        GFxDK_LiveCharRemove(this);
    }
    if (pHandle)
    {
        pHandle->ChangeCharacter(0);
        pHandle->Release();
        pHandle = 0;
    }
    delete pGeomData;
    pGeomData = 0;
}

GASObject* GFxASCharacter::EnsureASObject()
{
    if (pASObject == 0 && pMovieRoot)
    {
        GASGlobalContext* gc = pMovieRoot->GetASContext();
        pASObject = new GASObject(gc->GetSC(), pProto);
    }
    return pASObject;
}

GFxCharacterHandle* GFxASCharacter::CreateCharacterHandle()            // 2012 0x9d1a40
{
    if (pHandle == 0)
        pHandle = new GFxCharacterHandle(Name, pParent, this);
    return pHandle;
}

void GFxASCharacter::SetName(const GASString& n)
{
    Name = n;
    if (pHandle)
        pHandle->Name = n;
}

GASString GFxASCharacter::GetTargetPath(GASStringContext* sc) const
{
    // "_level0.a.b", built by walking up to the root. GFxASCharacter::GetAbsolutePath in retail.
    char buf[512];
    const GFxASCharacter* chain[32];
    unsigned int n = 0;
    for (const GFxASCharacter* c = this; c && n < 32; c = c->GetParent())
        chain[n++] = c;
    buf[0] = 0;
    strcat(buf, "_level0");
    for (unsigned int i = n; i > 1; --i)
    {
        const GFxASCharacter* c = chain[i - 2];
        strcat(buf, ".");
        strcat(buf, c->GetName().ToCStr());
    }
    return sc->CreateString(buf);
}

// m * child, in GMatrix2D's own 2x3 shape. There is no Prepend on this type and one composition is
// all the bounds walk needs.
static GMatrix2D GFxComposeMatrix(const GMatrix2D& m, const GMatrix2D& child)
{
    GMatrix2D out;
    out.M_[0][0] = m.M_[0][0] * child.M_[0][0] + m.M_[0][1] * child.M_[1][0];
    out.M_[0][1] = m.M_[0][0] * child.M_[0][1] + m.M_[0][1] * child.M_[1][1];
    out.M_[0][2] = m.M_[0][0] * child.M_[0][2] + m.M_[0][1] * child.M_[1][2] + m.M_[0][2];
    out.M_[1][0] = m.M_[1][0] * child.M_[0][0] + m.M_[1][1] * child.M_[1][0];
    out.M_[1][1] = m.M_[1][0] * child.M_[0][1] + m.M_[1][1] * child.M_[1][1];
    out.M_[1][2] = m.M_[1][0] * child.M_[0][2] + m.M_[1][1] * child.M_[1][2] + m.M_[1][2];
    return out;
}

// DISHONORED(port, agent DG): 2012 0x9d2ab0. The character's extent through `m`. This is what a
// clip's _width and _height read, and a clip that answers `undefined` for them makes the content's
// own layout loops non-terminating - see the report.
GRect<float> GFxCharacter::GetBoundsTwips(const GMatrix2D& m) const
{
    const GRect<int> local = GFxCharacterDefGetBoundsTwips(GetCharacterDef());
    if (local.Right <= local.Left && local.Bottom <= local.Top)
        return GRect<float>(0.f, 0.f, 0.f, 0.f);
    // All four corners, because a rotation turns the axis-aligned box into a quad and the bounds are
    // the box of that quad.
    float xs[4] = { (float)local.Left, (float)local.Right, (float)local.Left, (float)local.Right };
    float ys[4] = { (float)local.Top, (float)local.Top, (float)local.Bottom, (float)local.Bottom };
    GRect<float> out(0.f, 0.f, 0.f, 0.f);
    for (int i = 0; i < 4; ++i)
    {
        float x = xs[i], y = ys[i];
        m.Transform(&x, &y);
        if (i == 0)
        {
            out.Left = out.Right = x;
            out.Top = out.Bottom = y;
        }
        else
        {
            if (x < out.Left) out.Left = x;
            if (x > out.Right) out.Right = x;
            if (y < out.Top) out.Top = y;
            if (y > out.Bottom) out.Bottom = y;
        }
    }
    return out;
}

// A sprite has no geometry of its own: its extent is the union of its display list, each child
// through its own matrix composed with `m`.
GRect<float> GFxSprite::GetBoundsTwips(const GMatrix2D& m) const
{
    GRect<float> out(0.f, 0.f, 0.f, 0.f);
    bool bAny = false;
    for (unsigned int i = 0; i < DisplayList.GetCount(); ++i)
    {
        GFxCharacter* ch = DisplayList.GetAt(i);
        if (ch == 0)
            continue;
        const GMatrix2D child = GFxComposeMatrix(m, ch->GetMatrix());
        const GRect<float> r = ch->GetBoundsTwips(child);
        if (r.Right <= r.Left && r.Bottom <= r.Top)
            continue;
        if (!bAny)
        {
            out = r;
            bAny = true;
        }
        else
        {
            if (r.Left < out.Left) out.Left = r.Left;
            if (r.Top < out.Top) out.Top = r.Top;
            if (r.Right > out.Right) out.Right = r.Right;
            if (r.Bottom > out.Bottom) out.Bottom = r.Bottom;
        }
    }
    return out;
}

void GFxASCharacter::Set__proto__(GASStringContext* sc, GASObject* proto)
{
    pProto = proto;
    if (pASObject)
        pASObject->Set__proto__(sc, proto);
}

bool GFxASCharacter::GetStandardMember(GASBuiltinString which, GASValue* out) const
{
    switch (which)
    {
    case GASbuiltin__x:       out->SetNumber(Matrix.M_[0][2] * GFxTwipsToPixels); return true;
    case GASbuiltin__y:       out->SetNumber(Matrix.M_[1][2] * GFxTwipsToPixels); return true;
    case GASbuiltin__alpha:   out->SetNumber(ColorTransform.M_[3][0] * 100.0); return true;
    case GASbuiltin__visible: out->SetBool(bVisible); return true;
    case GASbuiltin__name:    out->SetString(Name); return true;
    default:                  return false;
    }
}

// DISHONORED(port): 2012 GFxASCharacter_MatrixScaleAndRotate2x2 0x9cdf10. The 2x2 is rotated by
// `rot` and then each column scaled; a factor at or below 1e-4 leaves the matrix alone, which is
// retail's guard against a zero-size clip losing its orientation.
static void GFxMatrixScaleAndRotate2x2(GMatrix2D* m, float sx, float sy, float rot)
{
    const float c = cosf(rot), s = sinf(rot);
    const float a = m->M_[0][0], b = m->M_[0][1], d = m->M_[1][0], e = m->M_[1][1];
    if (fabsf(sx) <= 0.0001f || fabsf(sy) <= 0.0001f)
        return;
    m->M_[0][0] = (a * c - d * s) * sx;
    m->M_[0][1] = (b * c - e * s) * sy;
    m->M_[1][0] = (a * s + d * c) * sx;
    m->M_[1][1] = (s * b + c * e) * sy;
}

// DISHONORED(port): 0x9ceb10. The stored GeomData when there is one, otherwise the one the current
// matrix implies - which is what a character that has never been written answers.
void GFxASCharacter::GetGeomData(GeomDataType* out) const
{
    if (pGeomData)
    {
        *out = *pGeomData;
        return;
    }
    out->X = (int)Matrix.M_[0][2];
    out->Y = (int)Matrix.M_[1][2];
    out->XScale = Matrix.GetXScale() * 100.0;
    out->YScale = Matrix.GetYScale() * 100.0;
    out->Rotation = Matrix.GetRotation() * 180.0 / 3.14159265358979323846;
    out->Matrix = Matrix;
}

void GFxASCharacter::SetGeomData(const GeomDataType& d)               // 0x9cf3b0
{
    if (pGeomData == 0)
        pGeomData = new GeomDataType;
    *pGeomData = d;
}

void GFxASCharacter::EnsureGeomDataCreated()                         // 0x9cf410
{
    if (pGeomData == 0)
    {
        GeomDataType d;
        GetGeomData(&d);
        SetGeomData(d);
    }
}

// The working matrix every geometry setter measures against: GeomData's own 2x2, with the
// character's CURRENT translation. 0x9d3350 builds it five times, once per case.
static GMatrix2D GFxCharacterGeomMatrix(const GFxASCharacter::GeomDataType& g, const GMatrix2D& live)
{
    GMatrix2D m = g.Matrix;
    m.M_[0][2] = live.M_[0][2];
    m.M_[1][2] = live.M_[1][2];
    return m;
}

bool GFxASCharacter::SetStandardMember(GASBuiltinString which, const GASValue& v)
{
    switch (which)
    {
    case GASbuiltin__x:
    {
        // DISHONORED(port): 0x9d3350 case 0 - the translation is quantised to whole twips and the
        // same value is kept in GeomData, which is what `_x` reads back.
        const double want = v.GetNumber();
        if (v.IsUndefined() || want != want) return true;
        EnsureGeomDataCreated();
        pGeomData->X = (int)floor(want * GFxPixelsToTwips);
        Matrix.M_[0][2] = (float)pGeomData->X;
        return true;
    }
    case GASbuiltin__y:
    {
        const double want = v.GetNumber();
        if (v.IsUndefined() || want != want) return true;
        EnsureGeomDataCreated();
        pGeomData->Y = (int)floor(want * GFxPixelsToTwips);
        Matrix.M_[1][2] = (float)pGeomData->Y;
        return true;
    }
    case GASbuiltin__alpha:   ColorTransform.M_[3][0] = (float)(v.GetNumber() / 100.0); return true;
    case GASbuiltin__visible: bVisible = v.GetBool(); return true;
    case GASbuiltin__width:
    case GASbuiltin__height:
    {
        // DISHONORED(port): 0x9d3350 cases 8 and 9. The extent is measured through GeomData's own
        // 2x2 with its rotation taken back out, so the answer is the size along the clip's OWN axes;
        // the factor goes into GeomData's scale and the matrix is rebuilt from GeomData.
        const double want = v.GetNumber();
        if (v.IsUndefined() || want != want)
            return true;
        EnsureGeomDataCreated();
        GeomDataType& g = *pGeomData;
        GMatrix2D m = GFxCharacterGeomMatrix(g, Matrix);
        const float deltaRot = (float)(g.Rotation * 3.14159265358979323846 / 180.0
                                       - m.GetRotation());
        GMatrix2D unrotated = m;
        GFxMatrixScaleAndRotate2x2(&unrotated, 1.f, 1.f, deltaRot);
        const GRect<float> b = GetBoundsTwips(unrotated);
        const bool bWidth = (which == GASbuiltin__width);
        const float extent = bWidth ? (b.Right - b.Left) : (b.Bottom - b.Top);
        double scale = 0.0;
        if (fabsf(extent) > 0.000001f)
            scale = want * GFxPixelsToTwips / (double)extent;
        const double axisScale = bWidth ? m.GetXScale() : m.GetYScale();
        double stored = scale * axisScale * 100.0;
        double divisor = axisScale;
        if (axisScale == 0.0)
        {
            stored = 0.0;
            divisor = 1.0;
        }
        if (bWidth) g.XScale = stored; else g.YScale = stored;
        const float rot = (float)(g.Rotation * 3.14159265358979323846 / 180.0 - m.GetRotation());
        const float sx = bWidth ? (float)fabs(stored / (100.0 * divisor))
                                : (float)fabs(g.XScale / (m.GetXScale() * 100.0));
        const float sy = bWidth ? (float)fabs(g.YScale / (m.GetYScale() * 100.0))
                                : (float)fabs(stored / (divisor * 100.0));
        GFxMatrixScaleAndRotate2x2(&m, sx, sy, rot);
        g.XScale = fabs(g.XScale);
        g.YScale = fabs(g.YScale);
        if (m.IsValid())
            Matrix = m;
        return true;
    }
    case GASbuiltin__xscale:
    case GASbuiltin__yscale:
    {
        // DISHONORED(port): 0x9d3350 cases 2 and 3. The percentage is stored and the matrix rebuilt
        // from GeomData, so the two axes and the rotation stay independent of each other.
        const double want = v.GetNumber();
        if (v.IsUndefined() || want != want)
            return true;
        EnsureGeomDataCreated();
        GeomDataType& g = *pGeomData;
        GMatrix2D m = GFxCharacterGeomMatrix(g, Matrix);
        const bool bX = (which == GASbuiltin__xscale);
        const double axisScale = bX ? m.GetXScale() : m.GetYScale();
        double stored = want;
        double divisor = axisScale;
        if (axisScale == 0.0 || want > 1.0e16)
        {
            stored = 0.0;
            divisor = 1.0;
        }
        if (bX) g.XScale = want; else g.YScale = want;
        const float rot = (float)(g.Rotation * 3.14159265358979323846 / 180.0 - m.GetRotation());
        const float sx = bX ? (float)(stored / (100.0 * divisor))
                            : (float)(g.XScale / (m.GetXScale() * 100.0));
        const float sy = bX ? (float)(g.YScale / (m.GetYScale() * 100.0))
                            : (float)(stored / (divisor * 100.0));
        GFxMatrixScaleAndRotate2x2(&m, sx, sy, rot);
        if (m.IsValid())
            Matrix = m;
        return true;
    }
    case GASbuiltin__rotation:
    {
        // DISHONORED(port): 0x9d3350 case 10. The angle is wrapped into (-180, 180], stored, and the
        // matrix rebuilt from GeomData with both axis scales kept.
        double want = v.GetNumber();
        if (v.IsUndefined() || want != want)
            return true;
        EnsureGeomDataCreated();
        GeomDataType& g = *pGeomData;
        want = fmod(want, 360.0);
        if (want > 180.0) want -= 360.0;
        else if (want < -180.0) want += 360.0;
        g.Rotation = want;
        GMatrix2D m = GFxCharacterGeomMatrix(g, Matrix);
        const float rot = (float)(want * 3.14159265358979323846 / 180.0 - m.GetRotation());
        const float sy = (float)(g.YScale / (m.GetYScale() * 100.0));
        const float sx = (float)(g.XScale / (m.GetXScale() * 100.0));
        GFxMatrixScaleAndRotate2x2(&m, sx, sy, rot);
        if (m.IsValid())
            Matrix = m;
        return true;
    }
    default:                  return false;
    }
}

bool GFxASCharacter::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{
    // The display properties are checked by name before the member store, because a timeline can
    // never shadow _x or _parent. Then the member store, then the prototype chain.
    static const GASBuiltinString standard[] =
    {
        GASbuiltin__x, GASbuiltin__y, GASbuiltin__alpha, GASbuiltin__visible, GASbuiltin__name
    };
    for (int i = 0; i < 5; ++i)
        if (name == sc->GetBuiltin(standard[i]) && GetStandardMember(standard[i], val))
            return true;

    // _width / _height / _rotation are not in the builtin string table, and `clip._width` in source
    // compiles to a GetMember rather than to ActionGetProperty, so the name has to be answered here
    // too. GFxAS2GetDisplayProperty holds the one implementation.
    {
        const char* n = name.ToCStr();
        if (n != 0 && n[0] == '_')
        {
            int propIndex = -1;
            if (strcmp(n, "_width") == 0)         propIndex = 8;
            else if (strcmp(n, "_height") == 0)   propIndex = 9;
            else if (strcmp(n, "_rotation") == 0) propIndex = 10;
            else if (strcmp(n, "_xscale") == 0)   propIndex = 2;
            else if (strcmp(n, "_yscale") == 0)   propIndex = 3;
            if (propIndex >= 0)
            {
                GFxAS2GetDisplayProperty(const_cast<GFxASCharacter*>(this), propIndex, val);
                return true;
            }
        }
    }
    if (name == sc->GetBuiltin(GASbuiltin__parent))
    {
        val->SetAsCharacter(pParent);
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin__root))
    {
        val->SetAsCharacter(pMovieRoot ? pMovieRoot->GetLevel0() : 0);
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin__target))
    {
        val->SetString(GetTargetPath(sc));
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin_this))
    {
        val->SetAsCharacter(this);
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin__global) && pMovieRoot)
    {
        val->SetAsObject(pMovieRoot->GetASContext()->GetGlobalObject());
        return true;
    }
    if (pASObject && pASObject->GetMemberRaw(sc, name, val))
        return true;
    // DISHONORED(port): a named child of the display list is a member of its parent - that is how
    // `_btnContainer_mc["btn" + i]` reaches the buttons the menu bar built, and how any AS2 code
    // reaches a clip the timeline placed by name. GFxSprite::GetMember (2012 0x9fb540) resolves it
    // through GFxDisplayList::GetCharacterByName (0x9d5450), which is already here.
    {
        GFxSprite* selfSprite = const_cast<GFxASCharacter*>(this)->ToSprite();
        if (selfSprite != 0)
        {
            GFxCharacter* child = selfSprite->GetDisplayList().GetCharacterByName(sc, name);
            if (child != 0 && child->IsASCharacter())
            {
                val->SetAsCharacter(child->ToASCharacterDef());
                return true;
            }
        }
    }
    if (pProto && pProto->GetMemberRaw(sc, name, val))
        return true;
    // The built-in prototype for this kind of character, last. Retail answers the MovieClip
    // built-ins from the character itself (GFxSprite::GetMember, 2012 0x9fb540, resolves them
    // through GetStandardMemberConstant and its own switch before it looks at __proto__), so a
    // registered class can replace __proto__ with anything and the clip still has attachMovie. This
    // tree keeps them on a prototype object instead, so the equivalent is to consult it here -
    // without it, Object.registerClass takes attachMovie away from the clip it just classed.
    if (pMovieRoot != 0 && ToSprite() != 0)
    {
        GASObject* builtin = pMovieRoot->GetASContext()->GetPrototype(GASGlobalContext::Proto_MovieClip);
        if (builtin != 0 && builtin != pProto && builtin->GetMemberRaw(sc, name, val))
            return true;
    }
    return false;
}

bool GFxASCharacter::SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                                  const GASPropFlags& flags)
{
    static const GASBuiltinString standard[] =
    {
        GASbuiltin__x, GASbuiltin__y, GASbuiltin__alpha, GASbuiltin__visible,
        GASbuiltin__width, GASbuiltin__height, GASbuiltin__xscale, GASbuiltin__yscale,
        GASbuiltin__rotation
    };
    for (int i = 0; i < 9; ++i)
        if (name == sc->GetBuiltin(standard[i]))
            return SetStandardMember(standard[i], val);
    if (name == sc->GetBuiltin(GASbuiltin_proto))
    {
        Set__proto__(sc, val.GetObject());
        return true;
    }
    GASObject* obj = EnsureASObject();
    return obj ? obj->SetMemberRaw(sc, name, val, flags) : false;
}

bool GFxASCharacter::GetMember(GASEnvironment* env, const GASString& name, GASValue* val)
{
    GASStringContext* sc = env->GetSC();
    if (!GetMemberRaw(sc, name, val))
        return false;
    if (val->IsProperty())
    {
        GASValue prop = *val;
        prop.GetPropertyValue(env, this, val);
    }
    return true;
}

bool GFxASCharacter::SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                               const GASPropFlags& flags)
{
    GASStringContext* sc = env->GetSC();
    GASValue existing;
    if (pProto && pProto->GetMemberRaw(sc, name, &existing) && existing.IsProperty())
    {
        existing.SetPropertyValue(env, this, val);
        return true;
    }
    return SetMemberRaw(sc, name, val, flags);
}

bool GFxASCharacter::FindMember(GASStringContext* sc, const GASString& name, GASMember* member)
{
    return pASObject ? pASObject->FindMember(sc, name, member) : false;
}

bool GFxASCharacter::DeleteMember(GASStringContext* sc, const GASString& name)
{
    return pASObject ? pASObject->DeleteMember(sc, name) : false;
}

bool GFxASCharacter::SetMemberFlags(GASStringContext* sc, const GASString& name, unsigned char f)
{
    return pASObject ? pASObject->SetMemberFlags(sc, name, f) : false;
}

bool GFxASCharacter::HasMember(GASStringContext* sc, const GASString& name, bool inherited)
{
    GASValue v;
    return GetMemberRaw(sc, name, &v);
}

void GFxASCharacter::VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                                  const GASObjectInterface* instance) const
{
    if (pASObject)
        pASObject->VisitMembers(sc, visitor, flags, instance ? instance : this);
}

bool GFxASCharacter::InstanceOf(GASEnvironment* env, const GASObject* proto, bool inherited) const
{
    const GASObject* p = pProto;
    while (p)
    {
        if (p == proto)
            return true;
        if (!inherited)
            return false;
        p = p->Get__proto__();
    }
    return false;
}

void GFxASCharacter::ExecuteEvent(GASBuiltinString eventName)          // via 2012 0x9d22e0
{
    if (pMovieRoot == 0)
        return;
    GASEnvironment* env = pMovieRoot->GetASEnvironment();
    GASValue handler;
    if (!GetMemberRaw(env->GetSC(), env->GetBuiltin(eventName), &handler))
        return;
    GASFunctionObject* fn = handler.GetFunction();
    if (fn == 0)
        return;
    GASValue result;
    GASFnCall call(&result, this, env, 0, env->GetTopIndex());
    fn->Invoke(call);
}

// ActionGetProperty / ActionSetProperty, by index.
void GFxAS2GetDisplayProperty(GFxASCharacter* ch, int index, GASValue* out)
{
    GFxSprite* sp = ch->ToSprite();
    switch (index)
    {
    case 0:  ch->GetStandardMember(GASbuiltin__x, out); break;
    case 1:  ch->GetStandardMember(GASbuiltin__y, out); break;
    case 2:  out->SetNumber(ch->GetMatrix().M_[0][0] * 100.0); break;
    case 3:  out->SetNumber(ch->GetMatrix().M_[1][1] * 100.0); break;
    case 4:  out->SetInt(sp ? (int)sp->GetCurrentFrame() + 1 : 0); break;
    case 5:  out->SetInt(sp ? (int)sp->GetFrameCount() : 0); break;
    case 6:  ch->GetStandardMember(GASbuiltin__alpha, out); break;
    case 7:  ch->GetStandardMember(GASbuiltin__visible, out); break;
    case 8:
    case 9:
    {
        // _width and _height: the clip's bounds in its PARENT's space, i.e. through its own matrix,
        // in pixels. 2012 GFxASCharacter::GetStandardMember's Standard__width / __height arms.
        const GRect<float> r = ch->GetBoundsTwips(ch->GetMatrix());
        out->SetNumber((double)((index == 8 ? (r.Right - r.Left) : (r.Bottom - r.Top))
                                * GFxTwipsToPixels));
        break;
    }
    case 10:
    {
        // _rotation, in degrees, out of the composed matrix.
        const GMatrix2D& mx = ch->GetMatrix();
        out->SetNumber(atan2((double)mx.M_[1][0], (double)mx.M_[0][0]) * 57.29577951308232);
        break;
    }
    case 11: out->SetString(ch->GetTargetPath(ch->GetMovieRoot()->GetASContext()->GetSC())); break;
    case 12: out->SetInt(sp ? (int)sp->GetFrameCount() : 0); break;
    case 13: ch->GetStandardMember(GASbuiltin__name, out); break;
    default: out->SetUndefined(); break;
    }
}

void GFxAS2SetDisplayProperty(GFxASCharacter* ch, int index, const GASValue& v, GASEnvironment* env)
{
    switch (index)
    {
    case 0:  ch->SetStandardMember(GASbuiltin__x, v); break;
    case 1:  ch->SetStandardMember(GASbuiltin__y, v); break;
    case 2:  ch->SetStandardMember(GASbuiltin__xscale, v); break;
    case 3:  ch->SetStandardMember(GASbuiltin__yscale, v); break;
    case 6:  ch->SetStandardMember(GASbuiltin__alpha, v); break;
    case 7:  ch->SetStandardMember(GASbuiltin__visible, v); break;
    case 8:  ch->SetStandardMember(GASbuiltin__width, v); break;
    case 9:  ch->SetStandardMember(GASbuiltin__height, v); break;
    case 10: ch->SetStandardMember(GASbuiltin__rotation, v); break;
    case 13: ch->SetName(v.ToString(env)); break;
    default: break;
    }
}

// ---------------------------------------------------------------------------------------------
// GFxSprite

GFxSprite::GFxSprite(GFxTimelineDef* def, GFxCharacterDef* charDef, GFxMovieDefImpl* defImpl,
                     GFxASCharacter* parent, GFxResourceId id, GFxMovieRoot* root)
    : GFxASCharacter(parent, id, root), pTimelineDef(def), pCharDef(charDef), pDefImpl(defImpl),
      CurrentFrame(0), InitActionsExecuted(0), InitActionsSize(0), bPlaying(true),
      bHasLooped(false), bFrame0Executed(false)
{
    unsigned int frames = def ? def->GetFrameCount() : 1;
    InitActionsSize = frames ? frames : 1;
    InitActionsExecuted = (unsigned char*)calloc(InitActionsSize, 1);
    if (root)
    {
        pProto = root->GetASContext()->GetPrototype(GASGlobalContext::Proto_MovieClip);
        ++root->GetCensus().SpritesCreated;
    }
}

GFxSprite::~GFxSprite()
{
    free(InitActionsExecuted);
}

void GFxSprite::ExecuteInitActionFrameTags(unsigned int frame)         // 2012 0x9f4640
{
    if (frame >= InitActionsSize || InitActionsExecuted[frame])
        return;
    const GFxTagList* list = pTimelineDef ? pTimelineDef->GetInitActionList(frame) : 0;
    if (list == 0 || list->GetSize() == 0)
    {
        InitActionsExecuted[frame] = 1;
        return;
    }
    for (unsigned int i = 0; i < list->GetSize(); ++i)
        (*list)[i]->Execute(this);
    InitActionsExecuted[frame] = 1;
}

void GFxSprite::ExecuteFrameTags(unsigned int frame, bool bWithActions)  // 2012 0x9f60a0
{
    if (pTimelineDef == 0 || frame >= pTimelineDef->GetFrameCount())
        return;
    ExecuteInitActionFrameTags(frame);
    const GFxTagList* list = pTimelineDef->GetPlaylist(frame);
    if (list == 0)
        return;
    for (unsigned int i = 0; i < list->GetSize(); ++i)
    {
        // bWithActions is false for the frames a goto passes THROUGH, and that is retail's own
        // distinction rather than a shortcut. GFxSprite::GotoFrame (2012 0xa012a0) does not call
        // ExecuteFrameTags on the intervening frames at all: it builds a GFxTimelineSnapshot of them
        // (GFxSprite::MakeSnapshot) and replays it through ExecuteSnapshot(this, snapshot, 4), and a
        // snapshot is display state - the place and remove records - with no action buffers in it.
        // Only the target frame gets the full ExecuteFrameTags. Replaying the actions of every
        // skipped frame instead is what made the imported CLIK platform-switch clips re-queue the
        // very DoAction that called gotoAndStop, 148,000 times before a guard caught it.
        if (!bWithActions && (*list)[i]->IsActionTag())
            continue;
        (*list)[i]->ExecuteWithPriority(this, GFxAP_Frame);
    }
}

void GFxSprite::ExecuteFrame0Events()                                 // 2012 0x9f8ac0
{
    if (bFrame0Executed)
        return;
    bFrame0Executed = true;
    ExecuteFrameTags(0);
    ExecuteEvent(GASbuiltin_onLoad);
}

void GFxSprite::IncrementFrameAndCheckForLoop()                       // 2012 0x9f4500
{
    ++CurrentFrame;
    unsigned int total = GetFrameCount();
    if (CurrentFrame >= total)
    {
        bHasLooped = true;
        CurrentFrame = 0;
        if (total <= 1)
        {
            // A single-frame timeline does not rebuild its display list; retail calls the
            // stop-at-end path (vtable +328) instead.
            bPlaying = false;
        }
        else
        {
            DisplayList.MarkAllEntriesForRemoval(0);
        }
    }
}

void GFxSprite::AdvanceFrame(bool bAdvance, float framePos)            // 2012 0x9f93f0
{
    if (!bAdvance)
    {
        ExecuteEvent(GASbuiltin_onEnterFrame);
    }
    else if (!bPlaying)
    {
        ExecuteEvent(GASbuiltin_onEnterFrame);
    }
    else
    {
        unsigned int before = CurrentFrame;
        IncrementFrameAndCheckForLoop();
        if (CurrentFrame != before)
        {
            ExecuteInitActionFrameTags(CurrentFrame);
            ExecuteEvent(GASbuiltin_onEnterFrame);
            ExecuteFrameTags(CurrentFrame);
        }
        else
        {
            ExecuteEvent(GASbuiltin_onEnterFrame);
        }
        if (CurrentFrame == 0)
            DisplayList.UnloadMarkedObjects();
    }

    // Children advance after their parent, in depth order, which is the order retail's
    // GFxDisplayList::AdvanceFrame walks. DISHONORED(bringup): over a refcounted snapshot, because
    // a child's onEnterFrame may remove a sibling and the entry owns the only reference.
    const unsigned int childCount = DisplayList.GetCount();
    if (childCount != 0)
    {
        GFxCharacter** children = (GFxCharacter**)malloc(childCount * sizeof(GFxCharacter*));
        for (unsigned int i = 0; i < childCount; ++i)
        {
            children[i] = DisplayList.GetAt(i);
            if (children[i])
                children[i]->AddRef();
        }
        for (unsigned int i = 0; i < childCount; ++i)
            children[i]->AdvanceFrame(bAdvance, framePos);
        for (unsigned int i = 0; i < childCount; ++i)
            children[i]->Release();
        free(children);
    }
}

void GFxSprite::GotoFrame(unsigned int frame)                         // 2012 0xa012a0
{
    unsigned int total = GetFrameCount();
    if (total == 0)
        return;
    if (frame >= total)
        frame = total - 1;
    if (frame == CurrentFrame)
        return;
    // CurrentFrame is assigned BEFORE the target frame's tags run, and that order is not cosmetic:
    // retail's body (2012 0xa012a0) writes the frame number (`this[27].__vftable = v8`) and only then
    // calls ExecuteFrameTags(v8), on both the forward and the rewind path. Executing first and
    // assigning afterwards is an infinite loop, because a frame whose own DoAction calls
    // gotoAndPlay on that same frame - which is what a two-frame CLIK component's idle loop is -
    // still sees the old CurrentFrame, fails the `frame == CurrentFrame` early out and re-enters.
    // That is the hang that binding the imports exposed: the loop is in the imported components.
    const unsigned int from = CurrentFrame;
    CurrentFrame = frame;
    // A backward goto replays the timeline from 0, because the display list of frame N is the sum of
    // frames 0..N. Retail marks the list for removal, replays, and sweeps, which is why a rewind does
    // not accumulate children.
    if (frame < from)
    {
        DisplayList.MarkAllEntriesForRemoval(0);
        for (unsigned int f = 0; f < frame; ++f)
            ExecuteFrameTags(f, false);
        ExecuteFrameTags(frame, true);
        DisplayList.UnloadMarkedObjects();
    }
    else
    {
        for (unsigned int f = from + 1; f < frame; ++f)
            ExecuteFrameTags(f, false);
        ExecuteFrameTags(frame, true);
    }
}

bool GFxSprite::GotoLabeledFrame(const char* label, int offset)         // 2012 0x9f6130
{
    unsigned int frame = 0;
    if (pTimelineDef == 0 || !pTimelineDef->GetLabeledFrame(label, &frame))
    {
        if (pMovieRoot)
        {
            // DISHONORED(bringup): naming the clip is the difference between "a label is missing"
            // and "which asset is missing it"; the platform-switch clips of the shared library all
            // ask for the same three names.
            const GASString path = GetTargetPath(pMovieRoot->GetASContext()->GetSC());
            pMovieRoot->LogScriptError("GotoLabeledFrame: no frame named '%s' on %s (%u labels)",
                                       label, path.ToCStr(),
                                       pTimelineDef ? pTimelineDef->GetLabelCount() : 0);
        }
        return false;
    }
    int target = (int)frame + offset;
    if (target < 0) target = 0;
    GotoFrame((unsigned int)target);
    return true;
}

void GFxSprite::CallFrameActions(unsigned int frame)                   // 2012 0x9f63b0
{
    if (pTimelineDef == 0 || frame >= pTimelineDef->GetFrameCount())
    {
        if (pMovieRoot)
            pMovieRoot->LogScriptError("CallFrame('%u') - unknown frame", frame);
        return;
    }
    const GFxTagList* list = pTimelineDef->GetPlaylist(frame);
    if (list == 0)
        return;
    for (unsigned int i = 0; i < list->GetSize(); ++i)
        if ((*list)[i]->IsActionTag())
            (*list)[i]->Execute(this);
    if (pMovieRoot)
        pMovieRoot->DoActions();
}

void GFxSprite::ExecuteBuffer(GASActionBuffer* buffer)                 // 2012 0x9f39f0
{
    if (pMovieRoot)
        pMovieRoot->PushActionBuffer(buffer, this, GFxAP_Frame);
}

GFxMovieDataDef* GFxSprite::GetOwnDataDef() const
{
    // A GFxSpriteDef records the movie data def it was parsed out of (pMovieDef), which is what makes
    // an imported clip's timeline resolve against its exporter's dictionary rather than the importing
    // movie's. The root clip's timeline def is the movie data def itself, so the fall-back is the
    // bound def impl's.
    GFxSpriteDef* spriteDef = pCharDef != 0
        && pCharDef->GetResourceTypeCode() == GFxResource::RT_SpriteDef
            ? (GFxSpriteDef*)pCharDef : 0;
    if (spriteDef != 0 && spriteDef->pMovieDef != 0)
        return spriteDef->pMovieDef;
    return pDefImpl ? pDefImpl->GetDataDef() : 0;
}

// DISHONORED(port, agent DG): 2012 0x9fee10's class binding, shared with AttachMovie. Retail queues
// it as an action-queue entry at priority 1 and the frame-0 events at priority 3, so the constructor
// runs before the clip's own first-frame actions; calling it here, before ExecuteFrame0Events, is
// that ordering without the queue.
bool GFxSprite::bTraceClassBinding = false;
bool GFxSprite::bBindRegisteredClasses = true;

void GFxSprite::BindRegisteredClass(GFxSprite* child, const GASString& symbol)
{
    GFxMovieRoot* root = child ? child->GetMovieRoot() : 0;
    if (root == 0 || symbol.GetSize() == 0 || !bBindRegisteredClasses)
        return;
    GASGlobalContext* gc = root->GetASContext();
    GASFunctionObject* ctor = gc->FindRegisteredClass(symbol);
    if (ctor == 0)
    {
        if (bTraceClassBinding)
            GFxLogf("DISHONORED(bringup): class binding: '%s' has no registered class",
                    symbol.ToCStr());
        return;
    }
    GASValue protoVal;
    if (ctor->GetMemberRaw(gc->GetSC(), gc->GetBuiltin(GASbuiltin_prototype), &protoVal))
    {
        GASObject* proto = protoVal.GetObject();
        if (bTraceClassBinding)
        {
            const GASObject* mcProto = gc->GetPrototype(GASGlobalContext::Proto_MovieClip);
            int depth = 0;
            bool bReachesMovieClip = false;
            for (const GASObject* p = proto; p != 0 && depth < 32; ++depth)
            {
                if (p == mcProto)
                {
                    bReachesMovieClip = true;
                    break;
                }
                p = p->Get__proto__();
            }
            GFxLogf("DISHONORED(bringup): class binding: '%s' -> class, chain depth %d, %s",
                    symbol.ToCStr(), depth,
                    bReachesMovieClip ? "reaches MovieClip.prototype"
                                      : "DOES NOT reach MovieClip.prototype");
        }
        child->Set__proto__(gc->GetSC(), proto);
    }
    GASEnvironment* env = root->GetASEnvironment();
    GASValue result;
    GASFnCall call(&result, child, env, 0, env->GetTopIndex());
    ctor->Invoke(call);
}

// DISHONORED(port): 2013 0x9f5830 (2012 0x9fee10).
GFxCharacter* GFxSprite::AddDisplayObject(const GFxCharPosInfo& pos)
{
    GFxMovieDataDef* dataDef = GetOwnDataDef();
    GFxCharacterDef* def = dataDef ? dataDef->GetCharacterDefById(pos.CharacterId) : 0;
    if (def == 0)
    {
        if (pMovieRoot)
            pMovieRoot->LogScriptError("PlaceObject: character %u is not in the dictionary",
                                       pos.CharacterId);
        return 0;
    }
    // Retail reads the depth before it creates anything: when the character already sitting there is
    // this very character under this very name, the tag is a move of that instance and nothing is
    // created at all - the function returns 0 after GFxSprite::MoveDisplayObject (2013 0x9eb080).
    // That is what a timeline loop does on every frame it rebuilds, and creating a second instance
    // there was what left the first one to be released underneath its own caller.
    {
        GFxCharacter* sitting = DisplayList.GetCharacterAtDepth(pos.Depth, 0);
        if (sitting != 0 && sitting->GetId().Id == pos.CharacterId)
        {
            bool bSameName = true;
            if (pos.HasName())
            {
                bSameName = sitting->IsASCharacter()
                    && sitting->ToASCharacterDef()->GetName() == pos.Name;
            }
            if (bSameName)
            {
                MoveDisplayObject(pos);
                return 0;
            }
        }
    }
    if (bTraceClassBinding)
    {
        GFxLogf("DISHONORED(bringup): place: char %u depth %d on %s, dataDef %p%s, def %p",
                pos.CharacterId, pos.Depth,
                pMovieRoot ? GetTargetPath(pMovieRoot->GetASContext()->GetSC()).ToCStr() : "?",
                (void*)dataDef,
                (pDefImpl != 0 && dataDef == pDefImpl->GetDataDef()) ? " (root)" : " (IMPORTED)",
                (void*)def);
    }
    GFxResourceId id;
    id.Id = pos.CharacterId;
    GFxCharacter* ch = def->CreateCharacterInstance(this, id, pDefImpl);
    if (ch == 0)
        return 0;
    if (ch->IsASCharacter())
    {
        GFxASCharacter* asch = ch->ToASCharacterDef();
        GASStringContext* sc = pMovieRoot->GetASContext()->GetSC();
        if (pos.HasName())
        {
            asch->SetName(sc->CreateString(pos.Name));
        }
        else
        {
            // An unnamed clip still needs a name so that a target path can address it; retail's
            // GFxMovieRoot::CreateNewInstanceName (2012 0xa08b50) produces "instanceN".
            char buf[32];
            sprintf(buf, "instance%d", pos.Depth);
            asch->SetName(sc->CreateString(buf));
        }
        if (pos.HasName() && pMovieRoot)
        {
            GASValue v;
            v.SetAsCharacter(asch);
            SetMemberRaw(sc, asch->GetName(), v, GASPropFlags());
        }
    }
    DisplayList.AddDisplayObject(pos, ch);
    if (pMovieRoot)
        ++pMovieRoot->GetCensus().DisplayObjectsPlaced;

    GFxSprite* childSprite = ch->IsASCharacter() ? ch->ToASCharacterDef()->ToSprite() : 0;
    if (childSprite)
    {
        // Retail looks the symbol up on the character definition itself; the export table is the
        // same mapping. Without this every timeline-placed clip in a Dishonored menu stays a bare
        // movie clip, so the movie player's PostStart finds no Open on the screen it wants to show
        // and every screen of the asset is left visible at once - which is what agent DC's
        // screenshot was, and what its report read as "the content's response to Open".
        if (pMovieRoot != 0 && dataDef != 0)
            pMovieRoot->QueueClassBinding(childSprite, dataDef->GetExportedName(pos.CharacterId));
        childSprite->ExecuteFrame0Events();
    }
    else
    {
        ch->OnEventLoad();
    }
    return ch;
}

void GFxSprite::MoveDisplayObject(const GFxCharPosInfo& pos)           // 2012 0x9f48a0
{
    DisplayList.MoveDisplayObject(pos);
    if (pMovieRoot)
        ++pMovieRoot->GetCensus().DisplayObjectsMoved;
}

void GFxSprite::ReplaceDisplayObject(const GFxCharPosInfo& pos)        // 2012 0x9f6300
{
    GFxMovieDataDef* dataDef = GetOwnDataDef();
    GFxCharacterDef* def = dataDef ? dataDef->GetCharacterDefById(pos.CharacterId) : 0;
    if (def == 0)
        return;
    GFxResourceId id;
    id.Id = pos.CharacterId;
    GFxCharacter* ch = def->CreateCharacterInstance(this, id, pDefImpl);
    if (ch == 0)
        return;
    DisplayList.ReplaceDisplayObject(pos, ch);
    GFxSprite* childSprite = ch->IsASCharacter() ? ch->ToASCharacterDef()->ToSprite() : 0;
    if (childSprite)
        childSprite->ExecuteFrame0Events();
}

void GFxSprite::RemoveDisplayObject(int depth, GFxResourceId id)        // 2012 0x9f4970
{
    DisplayList.RemoveDisplayObject(depth, id);
    if (pMovieRoot)
        ++pMovieRoot->GetCensus().DisplayObjectsRemoved;
}

GFxSprite* GFxSprite::CreateEmptyMovieClip(const GASString& name, int depth)
{
    // An empty clip is a one-frame sprite with no tags; retail keeps a shared empty definition for
    // it (GFxSpriteDef::InitEmptyClipDef, 2012 0x9fa3d0).
    static GFxSpriteDef* emptyDef = 0;
    if (emptyDef == 0)
    {
        emptyDef = new GFxSpriteDef(0);
        emptyDef->BeginFrames(1);
    }
    GFxResourceId id;
    id.Id = GFxResourceId::InvalidId;
    GFxSprite* child = new GFxSprite(emptyDef, emptyDef, pDefImpl, this, id, pMovieRoot);
    child->SetName(name);
    GFxCharPosInfo pos;
    pos.Depth = depth;
    DisplayList.AddDisplayObject(pos, child);
    if (pMovieRoot)
    {
        ++pMovieRoot->GetCensus().DisplayObjectsPlaced;
        GASValue v;
        v.SetAsCharacter(child);
        SetMemberRaw(pMovieRoot->GetASContext()->GetSC(), name, v, GASPropFlags());
    }
    return child;
}

GFxSprite* GFxSprite::AttachMovie(const GASString& symbolName, const GASString& instanceName,
                                  int depth)
{
    GFxMovieDataDef* dataDef = GetOwnDataDef();
    // DISHONORED(port): 2012 GFxMovieDefImpl::GetExportedResource 0xa1ede0, reached through
    // GFxMovieRoot::FindExportedResource 0xa03180: this movie's own exports first, then every movie
    // it imports, recursively, with the caller skipped so a cycle terminates.
    GFxCharacterDef* def = dataDef ? dataDef->FindExportedCharacter(symbolName.ToCStr()) : 0;
    if (def == 0)
    {
        if (pMovieRoot)
            pMovieRoot->LogScriptError("attachMovie: no exported symbol '%s' in '%s'",
                                       symbolName.ToCStr(),
                                       dataDef ? dataDef->GetSourceUrl() : "no data def");
        return 0;
    }
    GFxResourceId id = def->Id;
    GFxCharacter* ch = def->CreateCharacterInstance(this, id, pDefImpl);
    GFxSprite* child = (ch && ch->IsASCharacter()) ? ch->ToASCharacterDef()->ToSprite() : 0;
    if (child == 0)
    {
        if (ch) ch->Release();
        return 0;
    }
    child->SetName(instanceName);

    GFxCharPosInfo pos;
    pos.Depth = depth;
    DisplayList.AddDisplayObject(pos, child);
    if (pMovieRoot)
    {
        ++pMovieRoot->GetCensus().DisplayObjectsPlaced;
        GASValue v;
        v.SetAsCharacter(child);
        SetMemberRaw(pMovieRoot->GetASContext()->GetSC(), instanceName, v, GASPropFlags());
    }
    // DISHONORED(port): the clip's own first frame runs BEFORE its class constructor, because the
    // constructor reads the children that frame places. `_common.GenericMenu`'s does exactly that -
    // `new SelectionHandler(this, "btn", n, {elementContainer: this._btnContainer_mc})` - and with
    // the constructor first the container is undefined, so every later getSelectedElement answers
    // undefined and the whole menu bar stays at alpha 0. The timeline path already has this order:
    // AddDisplayObject places the children and queues the binding for the next drain.
    child->ExecuteFrame0Events();
    // Object.registerClass binds an AS2 class to a library symbol; if one is registered for this
    // symbol the clip is constructed as that class, which is what makes every CLIK widget in the
    // cook behave like its script class rather than like a bare movie clip.
    BindRegisteredClass(child, symbolName);
    return child;
}

bool GFxSprite::GetStandardMember(GASBuiltinString which, GASValue* out) const
{
    switch (which)
    {
    case GASbuiltin__currentframe: out->SetInt((int)CurrentFrame + 1); return true;
    case GASbuiltin__totalframes:  out->SetInt((int)GetFrameCount()); return true;
    default:                       return GFxASCharacter::GetStandardMember(which, out);
    }
}

bool GFxSprite::SetStandardMember(GASBuiltinString which, const GASValue& v)
{
    return GFxASCharacter::SetStandardMember(which, v);
}

bool GFxSprite::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{
    if (name == sc->GetBuiltin(GASbuiltin__currentframe))
    {
        val->SetInt((int)CurrentFrame + 1);
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin__totalframes))
    {
        val->SetInt((int)GetFrameCount());
        return true;
    }
    if (GFxASCharacter::GetMemberRaw(sc, name, val))
        return true;
    // A named child of the display list is addressable as a member of its parent even when the
    // PlaceObject did not install it, which is what a timeline instance name means.
    GFxCharacter* ch = DisplayList.GetCharacterByName(sc, name);
    if (ch && ch->IsASCharacter())
    {
        val->SetAsCharacter(ch->ToASCharacterDef());
        return true;
    }
    return false;
}
