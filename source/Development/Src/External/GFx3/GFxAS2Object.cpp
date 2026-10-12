// Scaleform GFx 3.3.89 - GASObject, GASArrayObject, GASFunctionObject, GASSuperObject. Package BC.
//
// The member store is a chained hash of GASString -> GASMember, which is the type the retail
// decompiles spell out in full: GASObject::GetMemberRaw (2012 0x9dc890) and SetMemberRaw (0x9de1c0)
// both operate on GHashSetBase<GHashNode<GASString,GASMember,GASStringHashFunctor>, ...>. Those two
// bodies also fix three behaviours that are easy to get wrong and that the class registrations in
// the cook depend on:
//   1. the lookup walks __proto__ and stops at the first owner that has the name;
//   2. __proto__ and __resolve are not members - they are intercepted by name before the hash
//      is ever consulted, on both the read and the write side;
//   3. when the found member is a PROPERTY (type 9/10) the *owner* that was reached is the object the
//      getter runs against, not the object the lookup started from.
// DISHONORED(port): see GFxAS2.h.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------------------------------------------
// GASRefCountCollector - GRefCountCollector<323>, the AS2 cycle collector. The algorithm and its
// policy words are read out of 2013 0xda78b0 (Collect), 0xda7b50 (Release), 0xda77c0
// (GRefCountBaseGC::CollectGarbage), 0xdd81a0 (AdvanceFrame), 0xdd82e0 / 0xdd8330 (ForceCollect /
// ForceEmergencyCollect), 0xdd6130 (SetParams), 0xddb610 (the constructor) and the five
// GASObject::ForEachChild_GC<...Functor> instantiations 0xdd19d0 / 0xdd1af0 / 0xdd1c00 / 0xdd2100
// / 0xdd2200, whose functors are inlined there and transcribed in VisitChild.

GASRefCountCollector::GASRefCountCollector()                           // 2013 0xddb610
    : pHead(0), Count(0), Created(0), FreedByCount(0), FreedInCycle(0), Collections(0),
      Roots(0), RootCount(0), RootCapacity(0), pKillList(0), bCollecting(false),
      FrameCnt(0), MaxRootCount(1000), PeakRootCount(0), LastRootCount(0), LastCollectedRoots(0),
      LastPeakRootCount(0), TotalFramesCount(0), LastCollectionFrameNum(0),
      PresetMaxRootCount(1000), MaxFramesBetweenCollections(0)
{
}

GASRefCountCollector::~GASRefCountCollector()
{
    FreeAll();
    free(Roots);
}

void GASRefCountCollector::SetParams(unsigned int framesBetweenCollections, unsigned int maxRootCount)
{                                                                     // 2013 0xdd6130
    MaxFramesBetweenCollections = framesBetweenCollections != ~0u ? framesBetweenCollections : 0;
    if (maxRootCount == ~0u)
        maxRootCount = 1000;
    PresetMaxRootCount = maxRootCount;
    MaxRootCount = maxRootCount;
}

void GASRefCountCollector::Register(GASObject* obj)
{
    obj->pCollector = this;
    obj->pCollectorPrev = 0;
    obj->pCollectorNext = pHead;
    if (pHead)
        pHead->pCollectorPrev = obj;
    pHead = obj;
    ++Count;
    ++Created;
    // DISHONORED(port): retail's GASObject constructor (2013 0xdd2ea0) starts the count at 1 for
    // the GPtr its creator holds; this tree's creators hold raw pointers and start it at 0, so the
    // newborn is a kill-list candidate until something stores it - see the class comment.
    AddToKillList(obj, false);
}

void GASRefCountCollector::AddRoot(GASObject* obj)                     // 0xda7b50's buffering arm
{
    if (RootCount >= RootCapacity)
    {
        unsigned int cap = RootCapacity ? RootCapacity * 2 : 1024;
        GASObject** next = (GASObject**)realloc(Roots, cap * sizeof(GASObject*));
        if (next == 0)
            return;
        Roots = next;
        RootCapacity = cap;
    }
    obj->bBuffered = true;
    obj->RootIndex = (int)RootCount;
    Roots[RootCount++] = obj;
}

void GASRefCountCollector::AddToKillList(GASObject* obj, bool bCycleGarbage)
{
    if (bCycleGarbage)
        obj->bCycleGarbage = true;
    if (obj->bInKillList)
        return;
    obj->bInKillList = true;
    obj->pKillNext = pKillList;
    pKillList = obj;
}

// 0xda78b0's kill loop (Finalize_GC, then the heap's Free), with this tree's two readings of the
// entry: a cycle member is freed outright - its edges were subtracted by MarkInCycle and
// Finalize_GC does not release them again - and a zero-count candidate is freed only if its count
// is still zero, after its children are released the way 0xda7b50 releases them (op 0) on the way
// to zero. A candidate whose count came back is left alone.
unsigned int GASRefCountCollector::ExecuteKillList()
{
    unsigned int freed = 0;
    while (pKillList)
    {
        GASObject* o = pKillList;
        pKillList = o->pKillNext;
        o->pKillNext = 0;
        o->bInKillList = false;
        if (o->bCycleGarbage)
        {
            ++FreedInCycle;
            Free(o);
            ++freed;
        }
        else if (o->RefCount == 0)
        {
            o->ExecuteForEachChild_GC(Op_Release);
            ++FreedByCount;
            Free(o);
            ++freed;
        }
    }
    return freed;
}

void GASRefCountCollector::Free(GASObject* obj)
{
    if (obj->bBuffered && obj->RootIndex >= 0 && (unsigned int)obj->RootIndex < RootCount
        && Roots[obj->RootIndex] == obj)
        Roots[obj->RootIndex] = 0;
    obj->bBuffered = false;
    obj->RootIndex = -1;
    if (obj->pCollectorPrev)
        obj->pCollectorPrev->pCollectorNext = obj->pCollectorNext;
    else
        pHead = obj->pCollectorNext;
    if (obj->pCollectorNext)
        obj->pCollectorNext->pCollectorPrev = obj->pCollectorPrev;
    obj->pCollectorNext = obj->pCollectorPrev = 0;
    obj->pCollector = 0;
    --Count;
    obj->Finalize_GC();
    delete obj;
}

// The functors of GRefCountBaseGC<323>, as inlined into GASObject::ForEachChild_GC<F> and
// GASValue::ForEachChild_GC<F> (2013 0xdc1700 MarkInCycle, 0xdc1690 ScanInUse, 0xdc1780 Scan,
// 0xdc18f0 CollectGarbage -> 0xda77c0, 0xdc18c0 Release).
void GASRefCountCollector::VisitChild(GASObject* child, int op)
{
    if (child == 0)
        return;
    switch (op)
    {
    case Op_Release:
        child->Release();
        break;
    case Op_MarkInCycle:
        if (child->RefCount)
            --child->RefCount;
        if (child->Color != GASObject::GC_Gray)
        {
            child->Color = GASObject::GC_Gray;
            child->ExecuteForEachChild_GC(Op_MarkInCycle);
        }
        break;
    case Op_ScanInUse:
        ++child->RefCount;
        if (child->Color != GASObject::GC_Black)
        {
            child->Color = GASObject::GC_Black;
            child->ExecuteForEachChild_GC(Op_ScanInUse);
        }
        break;
    case Op_Scan:
        if (child->Color == GASObject::GC_Gray)
        {
            if (child->RefCount)
            {
                child->Color = GASObject::GC_Black;
                child->ExecuteForEachChild_GC(Op_ScanInUse);
            }
            else
            {
                child->Color = GASObject::GC_White;
                child->ExecuteForEachChild_GC(Op_Scan);
            }
        }
        break;
    case Op_CollectGarbage:                                           // 2013 0xda77c0
        if (child->Color == GASObject::GC_White && !child->bBuffered)
        {
            child->Color = GASObject::GC_Black;
            child->ExecuteForEachChild_GC(Op_CollectGarbage);
            if (!child->bCycleGarbage && child->pCollector)
                child->pCollector->AddToKillList(child, true);
        }
        break;
    default:
        break;
    }
}

bool GASRefCountCollector::Collect(Stats* stats)                      // 2013 0xda78b0
{
    if (bCollecting)
    {
        if (stats) { stats->RootsProcessed = 0; stats->ObjectsFreed = 0; }
        return false;
    }
    bCollecting = true;
    ++Collections;
    unsigned int total = 0, freed = 0;
    for (;;)
    {
        // This tree's: the zero-count candidates go first, so that no object the root walk meets is
        // still holding edges on behalf of a count that has already reached zero.
        freed += ExecuteKillList();
        unsigned int n = RootCount;
        if (n == 0)
            break;
        total += n;
        // MarkRoots: a purple root goes gray and its subgraph is trial-deleted; a root that was
        // re-coloured since it was buffered just leaves the buffer.
        unsigned int w = 0;
        for (unsigned int i = 0; i < n; ++i)
        {
            GASObject* o = Roots[i];
            if (o == 0)
                continue;
            if (o->Color == GASObject::GC_Purple)
            {
                o->Color = GASObject::GC_Gray;
                o->ExecuteForEachChild_GC(Op_MarkInCycle);
            }
            else
            {
                o->bBuffered = false;
                o->RootIndex = -1;
            }
            Roots[w++] = o;
        }
        RootCount = w;
        // ScanRoots.
        for (unsigned int j = 0; j < w; ++j)
        {
            GASObject* o = Roots[j];
            if (o == 0 || o->Color != GASObject::GC_Gray)
                continue;
            if (o->RefCount)
            {
                o->Color = GASObject::GC_Black;
                o->ExecuteForEachChild_GC(Op_ScanInUse);
            }
            else
            {
                o->Color = GASObject::GC_White;
                o->ExecuteForEachChild_GC(Op_Scan);
            }
        }
        // CollectRoots.
        for (unsigned int k = 0; k < w; ++k)
        {
            GASObject* o = Roots[k];
            if (o == 0)
                continue;
            o->bBuffered = false;
            o->RootIndex = -1;
            if (o->Color == GASObject::GC_White)
            {
                o->Color = GASObject::GC_Black;
                o->ExecuteForEachChild_GC(Op_CollectGarbage);
                if (!o->bCycleGarbage)
                    AddToKillList(o, true);
            }
        }
        RootCount = 0;
        freed += ExecuteKillList();
    }
    bCollecting = false;
    if (stats)
    {
        stats->RootsProcessed = total;
        stats->ObjectsFreed = freed < total ? freed : total;
    }
    return true;
}

// 2013 0xdd81a0. frameCnt and lastCollectionFrame are the caller's two words (GFxMovieRoot+24 /
// +28), because retail shares one collector between the movies of a memory context and each movie
// keeps its own view of the frame count; here there is one movie per collector and the dance is
// kept as written.
void GASRefCountCollector::AdvanceFrame(unsigned int* frameCnt, unsigned int* lastCollectionFrame)
{
    // This tree's: the frees retail does inside Release, batched to the tail of the advance.
    ExecuteKillList();
    if (*lastCollectionFrame != LastCollectionFrameNum)
    {
        *lastCollectionFrame = LastCollectionFrameNum;
        *frameCnt = 1;
        return;
    }
    if (*frameCnt < FrameCnt)
    {
        ++*frameCnt;
        return;
    }
    ++TotalFramesCount;
    ++FrameCnt;
    unsigned int roots = RootCount;
    if (roots > PeakRootCount)
        PeakRootCount = roots;
    if ((PresetMaxRootCount != 0 && roots > MaxRootCount)
        || (MaxFramesBetweenCollections != 0 && FrameCnt >= MaxFramesBetweenCollections
            && roots > PresetMaxRootCount))
    {
        Stats st = { 0, 0 };
        Collect(&st);
        if (st.ObjectsFreed > PresetMaxRootCount)
        {
            PeakRootCount = roots;
            MaxRootCount = PresetMaxRootCount;
        }
        unsigned int remaining = roots - st.ObjectsFreed;
        if (remaining < MaxRootCount)
            remaining = MaxRootCount;
        MaxRootCount = remaining;
        unsigned int seventy = (unsigned int)((double)remaining * 0.7);
        if (PeakRootCount < seventy)
            MaxRootCount = seventy;
        LastCollectionFrameNum = TotalFramesCount;
        FrameCnt = 0;
        LastPeakRootCount = PeakRootCount;
        LastCollectedRoots = st.ObjectsFreed;
    }
    LastRootCount = roots;
    *frameCnt = FrameCnt;
    *lastCollectionFrame = LastCollectionFrameNum;
}

void GASRefCountCollector::ForceCollect()                             // 2013 0xdd82e0
{
    unsigned int roots = RootCount;
    Stats st = { 0, 0 };
    Collect(&st);
    FrameCnt = 0;
    LastRootCount = roots;
    if (roots > PeakRootCount)
        PeakRootCount = roots;
}

void GASRefCountCollector::ForceEmergencyCollect()                    // 2013 0xdd8330
{
    ForceCollect();
    if (!bCollecting && RootCount == 0)
    {
        free(Roots);
        Roots = 0;
        RootCapacity = 0;
    }
    PeakRootCount = 0;
    MaxRootCount = PresetMaxRootCount;
}

void GASRefCountCollector::FreeAll()
{
    pKillList = 0;
    RootCount = 0;
    // Two passes, as before this collector existed: first every object lets go of what it names,
    // then every object is deleted, so no destructor reaches a freed sibling. Finalize_GC detaches
    // without releasing, which is what a list that is being freed whole needs.
    for (GASObject* o = pHead; o; o = o->pCollectorNext)
    {
        o->bInKillList = false;
        o->bBuffered = false;
        o->Finalize_GC();
    }
    GASObject* o = pHead;
    while (o)
    {
        GASObject* next = o->pCollectorNext;
        o->pCollector = 0;
        o->pCollectorNext = o->pCollectorPrev = 0;
        delete o;
        o = next;
    }
    pHead = 0;
    Count = 0;
}

// ---------------------------------------------------------------------------------------------
// GASObjectInterface

GASFunctionObject* GASObjectInterface::Get__constructor__(GASStringContext* sc) // 2012 0x9af1e0
{
    GASValue v;
    if (GetMemberRaw(sc, sc->GetBuiltin(GASbuiltin_constructorUS), &v))
        return v.GetFunction();
    return 0;
}

bool GASObjectInterface::GetConstMemberRaw(GASStringContext* sc, const char* name, GASValue* val)
{                                                                     // 2012 0x9ae5e0
    return GetMemberRaw(sc, sc->CreateConstString(name), val);
}

bool GASObjectInterface::SetConstMemberRaw(GASStringContext* sc, const char* name,
                                           const GASValue& val, const GASPropFlags& flags)
{                                                                     // 2012 0x9d6dd0
    return SetMemberRaw(sc, sc->CreateConstString(name), val, flags);
}

bool GASObjectInterface::SetConstMemberRaw(GASStringContext* sc, const char* name,
                                           const GASValue& val)
{                                                                     // 2012 0x9e0880
    return SetMemberRaw(sc, sc->CreateConstString(name), val, GASPropFlags());
}

GASObjectInterface* GASObjectInterface::FindOwner(GASStringContext* sc, const GASString& name)
{                                                                     // 2012 0x9dac70
    GASObjectInterface* oi = this;
    while (oi)
    {
        GASObject* obj = oi->ToASObject();
        GASMember m;
        if (oi->FindMember(sc, name, &m))
            return oi;
        if (obj == 0)
            break;
        oi = obj->Get__proto__();
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------
// GASObject

GASObject::GASObject(GASStringContext* sc)                            // 2012 0x9dc740
    : pCollector(0), pCollectorNext(0), pCollectorPrev(0), pKillNext(0), RefCount(0), RootIndex(-1),
      Color(GC_Black), bBuffered(false), bInKillList(false), bCycleGarbage(false),
      Table(0), TableSize(0), MemberCount(0), pProto(0), pResolveHandler(0), bDestroying(false)
{
    if (sc && sc->pContext)
        sc->pContext->GetCollector()->Register(this);
}

GASObject::GASObject(GASStringContext* sc, GASObject* proto)           // 2012 0x9dc7b0
    : pCollector(0), pCollectorNext(0), pCollectorPrev(0), pKillNext(0), RefCount(0), RootIndex(-1),
      Color(GC_Black), bBuffered(false), bInKillList(false), bCycleGarbage(false),
      Table(0), TableSize(0), MemberCount(0), pProto(proto), pResolveHandler(0), bDestroying(false)
{
    if (pProto) pProto->AddRef();
    if (sc && sc->pContext)
        sc->pContext->GetCollector()->Register(this);
}

// GRefCountBaseGC<323>::Release, 2013 0xda7b50 / 2012 0x9aeb20. A count already at zero is left
// alone (the body's first test); a count that reaches zero releases the children and frees; a
// count that stays above zero makes the object a purple root candidate, buffered once. The free at
// zero is deferred to the kill list here - GASRefCountCollector's class comment says why.
void GASObject::Release()
{
    if (RefCount == 0)
        return;
    --RefCount;
    if (RefCount == 0)
    {
        if (pCollector)
            pCollector->AddToKillList(this, false);
        return;
    }
    if (Color != GC_Purple)
    {
        Color = GC_Purple;
        if (!bBuffered && pCollector)
            pCollector->AddRoot(this);
    }
}

GASObject::~GASObject()                                               // 2012 0x9dd6c0
{
    bDestroying = true;
    ReleaseAllMembers();
    if (pProto) { pProto->Release(); pProto = 0; }
    if (pResolveHandler) { pResolveHandler->Release(); pResolveHandler = 0; }
}

void GASObject::ReleaseAllMembers()
{
    for (unsigned int i = 0; i < TableSize; ++i)
    {
        MemberNode* n = Table[i];
        while (n)
        {
            MemberNode* next = n->pNext;
            delete n;
            n = next;
        }
        Table[i] = 0;
    }
    free(Table);
    Table = 0;
    TableSize = 0;
    MemberCount = 0;
}

// 2013 0xdd19d0 (and its four siblings): the member hash's values, the __resolve function ref at
// object+32 (GASFunctionRefBase::ForEachChild_GC), the watchpoints this tree has none of, then
// __proto__ (object+24) last - in that order.
void GASObject::ExecuteForEachChild_GC(int op) const
{
    for (unsigned int i = 0; i < TableSize; ++i)
        for (MemberNode* n = Table[i]; n; n = n->pNext)
            n->Member.Value.ForEachChild_GC(op);
    GASRefCountCollector::VisitChild(pResolveHandler, op);
    GASRefCountCollector::VisitChild(pProto, op);
}

// 2013 0xdd3970 / 2012 0x9dd210: the member hash is destroyed through its _GC node entry type,
// which drops the values without releasing them; the names (strings) are released as always.
void GASObject::Finalize_GC()
{
    for (unsigned int i = 0; i < TableSize; ++i)
        for (MemberNode* n = Table[i]; n; n = n->pNext)
            n->Member.Value.Finalize_GC();
    ReleaseAllMembers();
    pProto = 0;
    pResolveHandler = 0;
}

void GASObject::Rehash()
{
    unsigned int newSize = TableSize ? TableSize * 2 : 8;
    MemberNode** newTable = (MemberNode**)calloc(newSize, sizeof(MemberNode*));
    for (unsigned int i = 0; i < TableSize; ++i)
    {
        MemberNode* n = Table[i];
        while (n)
        {
            MemberNode* next = n->pNext;
            unsigned int slot = n->Name.GetHash() & (newSize - 1);
            n->pNext = newTable[slot];
            newTable[slot] = n;
            n = next;
        }
    }
    free(Table);
    Table = newTable;
    TableSize = newSize;
}

GASObject::MemberNode* GASObject::FindNode(GASStringContext* sc, const GASString& name) const
{
    if (TableSize == 0)
        return 0;
    MemberNode* n = Table[name.GetHash() & (TableSize - 1)];
    while (n)
    {
        if (n->Name == name)
            return n;
        n = n->pNext;
    }
    // Version 6 and earlier look members up case-insensitively; GASObject::GetMemberRaw takes that
    // branch on `*((_BYTE *)a2 + 4) <= 6u`, i.e. on the string context's SWF version.
    if (sc && sc->IsCaseInsensitive())
    {
        for (unsigned int i = 0; i < TableSize; ++i)
            for (MemberNode* m = Table[i]; m; m = m->pNext)
                if (m->Name.EqualsNoCase(name))
                    return m;
    }
    return 0;
}

GASObject::MemberNode* GASObject::AddNode(const GASString& name)
{
    if (MemberCount * 2 >= TableSize)
        Rehash();
    MemberNode* n = new MemberNode;
    n->Name = name;
    unsigned int slot = name.GetHash() & (TableSize - 1);
    n->pNext = Table[slot];
    Table[slot] = n;
    ++MemberCount;
    return n;
}

void GASObject::Set__proto__(GASStringContext* sc, GASObject* proto)   // 2012 0x9aef50
{
    if (proto == pProto)
        return;
    if (proto) proto->AddRef();
    if (pProto) pProto->Release();
    pProto = proto;
}

const void* GASObject::DishonoredFindMemberByText(const char* name, unsigned int* outHash) const
{
    for (unsigned int i = 0; i < TableSize; ++i)
        for (MemberNode* n = Table[i]; n; n = n->pNext)
            if (strcmp(n->Name.ToCStr(), name) == 0)
            {
                if (outHash) *outHash = n->Name.GetHash();
                return (const void*)n->Name.pNode;
            }
    return 0;
}

bool GASObject::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{                                                                     // 2012 0x9dc890
    const GASString& protoName = sc->GetBuiltin(GASbuiltin_proto);
    const GASString& resolveName = sc->GetBuiltin(GASbuiltin_resolve);

    GASObject* obj = this;
    bool resolveSet = false;
    // A step limit on the prototype walk: AS2 content can build a __proto__ cycle (one asset in the
    // cook does), and retail's collector-backed walk tolerates it where a plain loop would not.
    unsigned int steps = 0;
    while (obj && ++steps < 256)
    {
        if (name == protoName)
        {
            if (obj->pProto) val->SetAsObject(obj->pProto);
            else val->SetUndefined();
            return true;
        }
        if (name == resolveName)
        {
            if (obj->pResolveHandler) val->SetAsFunction(obj->pResolveHandler);
            else val->SetUndefined();
            return true;
        }
        MemberNode* n = obj->FindNode(sc, name);
        if (n)
        {
            // A PROPERTY found on a *prototype* is evaluated against the object the search started
            // from, which is why the retail body hands the owner to the caller and the caller
            // (GFxValue::ObjectInterface::GetMember 0x9b0450, GASEnvironment::GetMember 0x9e5a10)
            // passes `this` as the property's self.
            *val = n->Member.Value;
            return true;
        }
        if (!resolveSet && obj->pResolveHandler)
        {
            val->SetProperty(obj->pResolveHandler, 0);
            resolveSet = true;
        }
        obj = obj->pProto;
    }
    return false;
}

bool GASObject::SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                             const GASPropFlags& flags)
{                                                                     // 2012 0x9de1c0
    if (name == sc->GetBuiltin(GASbuiltin_proto))
    {
        if (!val.IsUnset())
            Set__proto__(sc, val.ToObject(0));
        return true;
    }
    if (name == sc->GetBuiltin(GASbuiltin_resolve))
    {
        if (!val.IsUnset())
        {
            GASFunctionObject* f = val.GetFunction();
            if (f) f->AddRef();
            if (pResolveHandler) pResolveHandler->Release();
            pResolveHandler = f;
        }
        return true;
    }
    // DISHONORED(bringup): which object a member was written to, and which string manager interned
    // the name it was written under. Two managers writing one object is what made `tween__start`
    // present by text and absent by identity on MovieClip.prototype (agentEG.md 2).
    if (GFxAS2MemberWriteDiag > 0 && GFxAS2WatchMatches(name.ToCStr()))
    {
        --GFxAS2MemberWriteDiag;
        GFxLogf("DISHONORED(bringup): MEMBERSET '%s' node %p hash %08x on object %p strings %p",
                name.ToCStr(), (void*)name.pNode, name.GetHash(), (void*)this,
                sc ? (void*)sc->pStrings : 0);
    }
    MemberNode* n = FindNode(sc, name);
    if (n)
    {
        if (n->Member.Flags.GetReadOnly())
            return true;
        n->Member.Value = val;
        return true;
    }
    n = AddNode(name);
    n->Member.Value = val;
    n->Member.Flags = flags;
    return true;
}

bool GASObject::GetMember(GASEnvironment* env, const GASString& name, GASValue* val) // 0xa26fa0
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

bool GASObject::SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                          const GASPropFlags& flags)
{                                                                     // 2012 0x9dde20
    GASStringContext* sc = env->GetSC();
    // A setter anywhere on the prototype chain wins over creating an own member, which is how the
    // CLIK widgets in the cook expose their properties.
    GASObject* obj = this;
    while (obj)
    {
        MemberNode* n = obj->FindNode(sc, name);
        if (n && n->Member.Value.IsProperty())
        {
            GASValue prop = n->Member.Value;
            prop.SetPropertyValue(env, this, val);
            return true;
        }
        if (n)
            break;
        obj = obj->pProto;
    }
    return SetMemberRaw(sc, name, val, flags);
}

bool GASObject::FindMember(GASStringContext* sc, const GASString& name, GASMember* member)
{
    MemberNode* n = FindNode(sc, name);
    if (n == 0)
        return false;
    if (member) *member = n->Member;
    return true;
}

bool GASObject::HasMember(GASStringContext* sc, const GASString& name, bool inherited)
{                                                                     // 2012 0x9dcb30
    GASObject* obj = this;
    while (obj)
    {
        if (obj->FindNode(sc, name))
            return true;
        if (!inherited)
            return false;
        obj = obj->pProto;
    }
    return false;
}

bool GASObject::DeleteMember(GASStringContext* sc, const GASString& name)  // 2012 0x9dd280
{
    if (TableSize == 0)
        return false;
    unsigned int slot = name.GetHash() & (TableSize - 1);
    MemberNode** pp = &Table[slot];
    while (*pp)
    {
        if ((*pp)->Name == name)
        {
            if ((*pp)->Member.Flags.GetDontDelete())
                return false;
            MemberNode* dead = *pp;
            *pp = dead->pNext;
            delete dead;
            --MemberCount;
            return true;
        }
        pp = &(*pp)->pNext;
    }
    return false;
}

bool GASObject::SetMemberFlags(GASStringContext* sc, const GASString& name, unsigned char flags)
{                                                                     // 2012 0x9de450
    MemberNode* n = FindNode(sc, name);
    if (n == 0)
        return false;
    n->Member.Flags = GASPropFlags(flags);
    return true;
}

void GASObject::VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                             const GASObjectInterface* instance) const
{                                                                     // 2012 0x9db7e0
    for (unsigned int i = 0; i < TableSize; ++i)
        for (MemberNode* n = Table[i]; n; n = n->pNext)
        {
            if (n->Member.Flags.GetDontEnum() && (flags & VisitMember_DontEnum) == 0)
                continue;
            visitor->Visit(n->Name, n->Member.Value, n->Member.Flags.Flags);
        }
    if ((flags & VisitMember_Prototype) != 0 && pProto)
        pProto->VisitMembers(sc, visitor, flags, instance ? instance : this);
}

bool GASObject::InstanceOf(GASEnvironment* env, const GASObject* proto, bool inherited) const
{                                                                     // 2012 0x9dacb0
    const GASObject* p = pProto;
    while (p)
    {
        if (p == proto)
            return true;
        if (!inherited)
            return false;
        p = p->pProto;
    }
    return false;
}

bool GASObject::DoesImplement(GASEnvironment* env, const GASObject* iface) const // 2012 0x9af130
{
    // ActionImplementsOp (0x2C) records the interfaces a class claims in a hidden member list; a
    // class that has not been through that opcode implements nothing, which is the base answer.
    return InstanceOf(env, iface, true);
}

// ---------------------------------------------------------------------------------------------
// GASArrayObject

GASArrayObject::GASArrayObject(GASStringContext* sc)
    : GASObject(sc), Elements(0), Size(0), Capacity(0) {}

GASArrayObject::GASArrayObject(GASStringContext* sc, GASObject* proto)
    : GASObject(sc, proto), Elements(0), Size(0), Capacity(0) {}

GASArrayObject::~GASArrayObject()
{
    delete[] Elements;
}

void GASArrayObject::ExecuteForEachChild_GC(int op) const             // 2013 0xdcd650
{
    for (unsigned int i = 0; i < Size; ++i)
        Elements[i].ForEachChild_GC(op);
    GASObject::ExecuteForEachChild_GC(op);
}

void GASArrayObject::Finalize_GC()                                    // 2013 0xdce510
{
    for (unsigned int i = 0; i < Size; ++i)
        Elements[i].Finalize_GC();
    Resize(0);
    GASObject::Finalize_GC();
}

int GASArrayObject::ParseIndex(const GASString& name)
{
    const char* s = name.ToCStr();
    if (s[0] == 0)
        return -1;
    int v = 0;
    for (const char* p = s; *p; ++p)
    {
        if (*p < '0' || *p > '9')
            return -1;
        v = v * 10 + (*p - '0');
        if (v > 0x00FFFFFF)
            return -1;
    }
    return v;
}

void GASArrayObject::Resize(unsigned int n)
{
    // An AS2 array index that came out of an undefined value is 0x80000000 or worse; growing to it
    // is a bad_alloc with nothing to say. The bound is generous against any real content (the
    // largest array in the cook's menus is the save-slot list) and it reports rather than throws.
    enum { MaxElements = 1 << 20 };
    if (n > MaxElements)
    {
        GFxLogf("DISHONORED(bringup): AS2 array refused a length of %u (bound %u)",
                n, (unsigned int)MaxElements);
        return;
    }
    if (n > Capacity)
    {
        unsigned int cap = Capacity ? Capacity * 2 : 8;
        if (cap < n) cap = n;
        GASValue* next = new GASValue[cap];
        for (unsigned int i = 0; i < Size; ++i)
            next[i] = Elements[i];
        delete[] Elements;
        Elements = next;
        Capacity = cap;
    }
    if (n < Size)
        for (unsigned int i = n; i < Size; ++i)
            Elements[i].SetUndefined();
    Size = n;
}

void GASArrayObject::SetElement(unsigned int i, const GASValue& v)
{
    if (i >= Size)
        Resize(i + 1);
    Elements[i] = v;
}

void GASArrayObject::PushBack(const GASValue& v)
{
    Resize(Size + 1);
    Elements[Size - 1] = v;
}

void GASArrayObject::RemoveElements(unsigned int index, int count)
{
    if (index >= Size)
        return;
    unsigned int n = (count < 0) ? (Size - index) : (unsigned int)count;
    if (index + n > Size)
        n = Size - index;
    for (unsigned int i = index; i + n < Size; ++i)
        Elements[i] = Elements[i + n];
    Resize(Size - n);
}

bool GASArrayObject::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{
    if (name == sc->GetBuiltin(GASbuiltin_length))
    {
        val->SetInt((int)Size);
        return true;
    }
    int idx = ParseIndex(name);
    if (idx >= 0)
    {
        if ((unsigned int)idx < Size)
        {
            *val = Elements[idx];
            return true;
        }
        val->SetUndefined();
        return false;
    }
    return GASObject::GetMemberRaw(sc, name, val);
}

bool GASArrayObject::SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                                  const GASPropFlags& flags)
{
    if (name == sc->GetBuiltin(GASbuiltin_length))
    {
        Resize((unsigned int)val.GetNumber());
        return true;
    }
    int idx = ParseIndex(name);
    if (idx >= 0)
    {
        SetElement((unsigned int)idx, val);
        return true;
    }
    return GASObject::SetMemberRaw(sc, name, val, flags);
}

bool GASArrayObject::HasMember(GASStringContext* sc, const GASString& name, bool inherited)
{
    if (name == sc->GetBuiltin(GASbuiltin_length))
        return true;
    int idx = ParseIndex(name);
    if (idx >= 0)
        return (unsigned int)idx < Size;
    return GASObject::HasMember(sc, name, inherited);
}

void GASArrayObject::VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                                  const GASObjectInterface* instance) const
{
    // for..in over an array enumerates the indices, which is what the CLIK list code relies on.
    char buf[16];
    for (unsigned int i = 0; i < Size; ++i)
    {
        sprintf(buf, "%u", i);
        visitor->Visit(sc->CreateString(buf), Elements[i], 0);
    }
    GASObject::VisitMembers(sc, visitor, flags, instance);
}

// ---------------------------------------------------------------------------------------------
// GASFnCall / GASFunctionObject

const GASValue& GASFnCall::Arg(int n) const
{
    return pEnv->Bottom(FirstArgBottomIndex - n);
}

GASFunctionObject::GASFunctionObject(GASStringContext* sc, GASObject* proto)
    : GASObject(sc, proto), pCFunction(0), pNewObjectFunc(0), pBuffer(0), StartPC(0), Length(0),
      Version(0), RegisterCount(0), Flags(0), Args(0), NumArgs(0), pDeclTarget(0), pOwnerProto(0) {}

GASObject* GASFunctionObject::CreateNewObject(GASStringContext* sc, GASObject* proto) const
{
    return pNewObjectFunc ? pNewObjectFunc(sc, proto) : new GASObject(sc, proto);
}

GASFunctionObject::~GASFunctionObject()
{
    delete[] Args;
    if (pDeclTarget)
        pDeclTarget->Release();
}

// 2013 0xde9290: the declaring clip's handle is not collectable and is released as always
// (GASAsFunctionObject+56); the local frame retail's closures capture does not exist here.
void GASFunctionObject::Finalize_GC()
{
    if (pDeclTarget)
        pDeclTarget->Release();
    pDeclTarget = 0;
    pOwnerProto = 0;
    pBuffer = 0;
    GASObject::Finalize_GC();
}

void GASPrimitiveObject::ExecuteForEachChild_GC(int op) const
{
    Primitive.ForEachChild_GC(op);
    GASObject::ExecuteForEachChild_GC(op);
}

void GASPrimitiveObject::Finalize_GC()
{
    Primitive.Finalize_GC();
    GASObject::Finalize_GC();
}

// 2012 0x9f28f0's tail: the declaring clip's handle, made on demand, with a reference.
void GASFunctionObject::SetDeclTarget(GFxASCharacter* target)
{
    GFxCharacterHandle* h = target ? target->CreateCharacterHandle() : 0;
    if (h)
        h->AddRef();
    if (pDeclTarget)
        pDeclTarget->Release();
    pDeclTarget = h;
}

GFxASCharacter* GASFunctionObject::ResolveDeclTarget(GFxMovieRoot* root) const
{
    return pDeclTarget ? pDeclTarget->ResolveCharacter(root) : 0;
}

void GASFunctionObject::Invoke(const GASFnCall& fn)
{
    if (pCFunction)
    {
        pCFunction(fn);
        return;
    }
    if (pBuffer == 0)
        return;
    GFxAS2InvokeScriptFunction(this, fn);
}

// ---------------------------------------------------------------------------------------------
// GASSuperObject

GASSuperObject::GASSuperObject(GASStringContext* sc, GASObject* proto, GASObjectInterface* self,
                               GASFunctionObject* ctor)
    : GASObject(sc, proto), pRealThis(self), pConstructor(ctor)
{
    if (pConstructor) pConstructor->AddRef();
}

void GASSuperObject::ExecuteForEachChild_GC(int op) const             // 2013 0xdd8e40
{
    GASRefCountCollector::VisitChild(pConstructor, op);
    GASObject::ExecuteForEachChild_GC(op);
}

void GASSuperObject::Finalize_GC()                                    // 2013 0xdd8bb0
{
    pRealThis = 0;
    pConstructor = 0;
    GASObject::Finalize_GC();
}

bool GASSuperObject::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{
    // `super.foo()` must find foo on the *parent* prototype and then run it with the real `this`,
    // which the interpreter arranges by keeping pRealThis here; the lookup itself is the ordinary
    // prototype-chain walk starting one level up.
    GASObject* p = Get__proto__();
    return p ? p->GetMemberRaw(sc, name, val) : false;
}
