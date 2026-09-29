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
// GASObjectCollector - the teardown half of retail's GASRefCountCollector.

void GASObjectCollector::Add(GASObject* obj)
{
    obj->pCollectorNext = pHead;
    obj->pCollector = this;
    pHead = obj;
    ++Count;
    ++Created;
}

void GASObjectCollector::FreeAll()
{
    // Two passes, because an object's destructor releases members that point at other objects in
    // this same list: first drop every reference so no destructor can reach a freed sibling, then
    // free. The first pass has to clear __proto__ as well as the member hash, because an AS2 class
    // and its prototype always point at each other.
    for (GASObject* o = pHead; o; o = o->pCollectorNext)
        o->PrepareForCollection();
    GASObject* o = pHead;
    while (o)
    {
        GASObject* next = o->pCollectorNext;
        o->pCollector = 0;
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
    : pCollectorNext(0), pCollector(0), Table(0), TableSize(0), MemberCount(0), pProto(0),
      pResolveHandler(0), RefCount(0), bDestroying(false)
{
    if (sc && sc->pContext)
        sc->pContext->GetCollector()->Add(this);
}

GASObject::GASObject(GASStringContext* sc, GASObject* proto)           // 2012 0x9dc7b0
    : pCollectorNext(0), pCollector(0), Table(0), TableSize(0), MemberCount(0), pProto(proto),
      pResolveHandler(0), RefCount(0), bDestroying(false)
{
    if (pProto) pProto->AddRef();
    if (sc && sc->pContext)
        sc->pContext->GetCollector()->Add(this);
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

void GASObject::PrepareForCollection()
{
    ReleaseAllMembers();
    if (pProto) { pProto->Release(); pProto = 0; }
    if (pResolveHandler) { pResolveHandler->Release(); pResolveHandler = 0; }
}

void GASObject::OnZeroRef()
{
    // Nothing: every object is owned by the movie's GASObjectCollector and freed at teardown.
    // AS2 object graphs are full of cycles (a class's prototype names its constructor and the
    // constructor names its prototype, which is what every one of the 669 __Packages registrations
    // builds), so a plain refcount cannot free them and retail uses a real mark-and-sweep. This is
    // the honest bringup answer: correct lifetime within a movie, no leak across movies, and no
    // collection during a frame. resources/docs/agents/agentBC.md names it as remaining work.
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

void GASArrayObject::PrepareForCollection()
{
    Resize(0);
    GASObject::PrepareForCollection();
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
}

void GASFunctionObject::PrepareForCollection()
{
    pDeclTarget = 0;
    pOwnerProto = 0;
    pBuffer = 0;
    GASObject::PrepareForCollection();
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

void GASSuperObject::PrepareForCollection()
{
    pRealThis = 0;
    if (pConstructor) { pConstructor->Release(); pConstructor = 0; }
    GASObject::PrepareForCollection();
}

bool GASSuperObject::GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val)
{
    // `super.foo()` must find foo on the *parent* prototype and then run it with the real `this`,
    // which the interpreter arranges by keeping pRealThis here; the lookup itself is the ordinary
    // prototype-chain walk starting one level up.
    GASObject* p = Get__proto__();
    return p ? p->GetMemberRaw(sc, name, val) : false;
}
