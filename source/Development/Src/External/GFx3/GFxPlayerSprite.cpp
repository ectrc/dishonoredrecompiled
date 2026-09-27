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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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
    if (Entries[index].pChar)
        Entries[index].pChar->Release();
    for (unsigned int i = index; i + 1 < Size; ++i)
        Entries[i] = Entries[i + 1];
    --Size;
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

GFxCharacter* GFxDisplayList::GetCharacterByName(GASStringContext* sc, const GASString& name) const
{                                                                     // 2012 0x9d5450
    for (unsigned int i = 0; i < Size; ++i)
    {
        GFxCharacter* c = Entries[i].pChar;
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

void GFxDisplayList::AddDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch) // 0x9d60f0
{
    int i = FindDisplayIndex(pos.Depth);
    if (i < (int)Size && Entries[i].pChar->GetDepth() == pos.Depth)
    {
        // A place onto an occupied depth replaces, unless the sitting entry was only marked for
        // removal by the loop rebuild, in which case it is revived rather than replaced.
        if (Entries[i].bMarkedForRemove)
        {
            Entries[i].bMarkedForRemove = false;
            if (Entries[i].pChar->GetId().Id == ch->GetId().Id)
            {
                ch->Release();
                return;
            }
        }
        Entries[i].pChar->Release();
        Entries[i].pChar = ch;
        Entries[i].bMarkedForRemove = false;
    }
    else
    {
        InsertAt((unsigned int)i, ch);
    }
    ch->SetDepth(pos.Depth);
    if (pos.HasMatrix()) ch->SetMatrix(pos.Matrix);
    if (pos.HasCxform()) ch->SetCxform(pos.ColorTransform);
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
    if (pos.PlaceFlags & GFxCharPosInfo::Place_HasRatio) ch->SetRatio(pos.Ratio);
    if (pos.PlaceFlags & GFxCharPosInfo::Place_HasClipDepth) ch->SetClipDepth(pos.ClipDepth);
}

void GFxDisplayList::ReplaceDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch) // 0x9d6350
{
    int i = FindDisplayIndex(pos.Depth);
    if (i < (int)Size && Entries[i].pChar->GetDepth() == pos.Depth)
    {
        Entries[i].pChar->Release();
        Entries[i].pChar = ch;
        Entries[i].bMarkedForRemove = false;
    }
    else
    {
        InsertAt((unsigned int)i, ch);
    }
    ch->SetDepth(pos.Depth);
    if (pos.HasMatrix()) ch->SetMatrix(pos.Matrix);
    if (pos.HasCxform()) ch->SetCxform(pos.ColorTransform);
}

void GFxDisplayList::RemoveDisplayObject(int depth, GFxResourceId id)   // 2012 0x9d5fd0
{
    int i = FindDisplayIndex(depth);
    if (i >= (int)Size || Entries[i].pChar->GetDepth() != depth)
        return;
    if (id.Id != GFxResourceId::InvalidId && Entries[i].pChar->GetId().Id != id.Id)
        return;
    Entries[i].pChar->OnEventUnload();
    RemoveAt((unsigned int)i);
}

void GFxDisplayList::MarkAllEntriesForRemoval(unsigned int fromIndex)   // 2012 0x9d5670
{
    for (unsigned int i = fromIndex; i < Size; ++i)
        Entries[i].bMarkedForRemove = true;
}

void GFxDisplayList::UnloadMarkedObjects()                            // 2012 0x9d60a0
{
    for (unsigned int i = Size; i > 0; --i)
        if (Entries[i - 1].bMarkedForRemove)
        {
            Entries[i - 1].pChar->OnEventUnload();
            RemoveAt(i - 1);
        }
}

void GFxDisplayList::UnloadAll()                                      // 2012 0x9d6070
{
    for (unsigned int i = Size; i > 0; --i)
    {
        Entries[i - 1].pChar->OnEventUnload();
        RemoveAt(i - 1);
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
    : GFxCharacter(parent, id), pMovieRoot(root), pHandle(0), pASObject(0), pProto(0) {}

GFxASCharacter::~GFxASCharacter()
{
    if (pHandle)
    {
        pHandle->ChangeCharacter(0);
        pHandle->Release();
        pHandle = 0;
    }
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

bool GFxASCharacter::SetStandardMember(GASBuiltinString which, const GASValue& v)
{
    switch (which)
    {
    case GASbuiltin__x:       Matrix.M_[0][2] = (float)(v.GetNumber() * GFxPixelsToTwips); return true;
    case GASbuiltin__y:       Matrix.M_[1][2] = (float)(v.GetNumber() * GFxPixelsToTwips); return true;
    case GASbuiltin__alpha:   ColorTransform.M_[3][0] = (float)(v.GetNumber() / 100.0); return true;
    case GASbuiltin__visible: bVisible = v.GetBool(); return true;
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
    if (pProto && pProto->GetMemberRaw(sc, name, val))
        return true;
    return false;
}

bool GFxASCharacter::SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                                  const GASPropFlags& flags)
{
    static const GASBuiltinString standard[] =
    {
        GASbuiltin__x, GASbuiltin__y, GASbuiltin__alpha, GASbuiltin__visible
    };
    for (int i = 0; i < 4; ++i)
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
    case 6:  ch->SetStandardMember(GASbuiltin__alpha, v); break;
    case 7:  ch->SetStandardMember(GASbuiltin__visible, v); break;
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
    // GFxDisplayList::AdvanceFrame walks.
    for (unsigned int i = 0; i < DisplayList.GetCount(); ++i)
        DisplayList.GetAt(i)->AdvanceFrame(bAdvance, framePos);
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
            pMovieRoot->LogScriptError("GotoLabeledFrame: no frame named '%s'", label);
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

GFxCharacter* GFxSprite::AddDisplayObject(const GFxCharPosInfo& pos)   // 2012 0x9fee10
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
        childSprite->ExecuteFrame0Events();
    else
        ch->OnEventLoad();
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
    GFxCharacterDef* def = dataDef ? dataDef->GetExportedCharacter(symbolName.ToCStr()) : 0;
    if (def == 0)
    {
        if (pMovieRoot)
            pMovieRoot->LogScriptError("attachMovie: no exported symbol '%s'", symbolName.ToCStr());
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

    // Object.registerClass binds an AS2 class to a library symbol; if one is registered for this
    // symbol the clip is constructed as that class, which is what makes every CLIK widget in the
    // cook behave like its script class rather than like a bare movie clip.
    if (pMovieRoot)
    {
        GASGlobalContext* gc = pMovieRoot->GetASContext();
        GASFunctionObject* ctor = gc->FindRegisteredClass(symbolName);
        if (ctor)
        {
            GASValue protoVal;
            if (ctor->GetMemberRaw(gc->GetSC(), gc->GetBuiltin(GASbuiltin_prototype), &protoVal))
                child->Set__proto__(gc->GetSC(), protoVal.GetObject());
            GASEnvironment* env = pMovieRoot->GetASEnvironment();
            GASValue result;
            GASFnCall call(&result, child, env, 0, env->GetTopIndex());
            ctor->Invoke(call);
        }
    }

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
    child->ExecuteFrame0Events();
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
