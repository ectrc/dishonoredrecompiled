// Scaleform GFx 3.3.89 - GASString, GASStringManager and GASValue. Package BC.
// DISHONORED(port): the rva on each function is the 2012 body it was read from; see GFxAS2.h.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ---------------------------------------------------------------------------------------------
// GASStringNode / GASStringManager

double GFxAS2NaN()
{
    // A quiet NaN built from bits, because /fp:precise plus 0.0/0.0 is a divide-by-zero at compile
    // time in MSVC and the CRT's NAN macro is C99.
    static const unsigned int bits[2] = { 0x00000000u, 0x7FF80000u };
    return *(const double*)bits;
}

double GFxAS2Infinity()
{
    static const unsigned int bits[2] = { 0x00000000u, 0x7FF00000u };
    return *(const double*)bits;
}

bool GFxAS2IsNaN(double d)                                            // 2012 0x9acb10
{
    return d != d;
}

void GFxAS2NumberToString(double d, char* out, unsigned int outSize)
{
    // AS2 prints an integral double without a decimal point and a fractional one with up to 15
    // significant digits, which is what GASNumberUtil::ToString does with its `precision` argument.
    if (GFxAS2IsNaN(d)) { strncpy(out, "NaN", outSize); out[outSize - 1] = 0; return; }
    if (d == GFxAS2Infinity()) { strncpy(out, "Infinity", outSize); out[outSize - 1] = 0; return; }
    if (d == -GFxAS2Infinity()) { strncpy(out, "-Infinity", outSize); out[outSize - 1] = 0; return; }
    if (d == 0.0) { strncpy(out, "0", outSize); out[outSize - 1] = 0; return; }
    if (d == floor(d) && d > -1e15 && d < 1e15)
    {
        sprintf(out, "%.0f", d);
        return;
    }
    sprintf(out, "%.15g", d);
}

void GASStringNode::Release()                                         // 2012 0x9acaf0 / 0x9c9d20
{
    // Interned nodes outlive their references: the manager owns them and frees the table in one go.
    // Retail does return a node to a free list at zero (GASStringManager::FreeStringNode 0x9c98a0);
    // holding them for the movie's lifetime costs a movie-sized table and removes a whole class of
    // use-after-free during bringup, which is the right trade while the machine is being built.
    if (RefCount > 0)
        --RefCount;
}

unsigned int GASStringManager::HashOf(const char* str, unsigned int len)
{
    // The 2.1 hash GFx uses for GASString (GASStringHashFunctor); any stable hash gives identical
    // behaviour because the value never leaves the process.
    unsigned int h = 2166136261u;
    for (unsigned int i = 0; i < len; ++i)
    {
        h ^= (unsigned char)str[i];
        h *= 16777619u;
    }
    return h;
}

GASStringManager::GASStringManager()                                  // 2012 0x9ca580
    : Table(0), TableSize(0), Count(0)
{
    TableSize = 1024;
    Table = (GASStringNode**)calloc(TableSize, sizeof(GASStringNode*));
}

// The chain pointer lives immediately after the node so GASStringNode keeps the four words the
// decompiles read at 0, 4, 8 and 12.
static GASStringNode** ChainNextOf(GASStringNode* n)
{
    return (GASStringNode**)((char*)n + sizeof(GASStringNode));
}

GASStringManager::~GASStringManager()                                 // 2012 0x9ca630
{
    // DISHONORED(bringup): a node something still holds is LEAKED rather than freed. The action
    // buffers of a movie definition keep their constant pool as GASStrings and the loader outlives
    // the movie root, so those references are destroyed after this manager is gone; freeing the node
    // here makes that a read of freed memory (GASStringNode::Release, which is where it faulted).
    // The next execution of such a buffer rebuilds its dictionary from the live context, so a leaked
    // node is never read again - it is a movie-sized leak per close, against a use-after-free.
    for (unsigned int i = 0; i < TableSize; ++i)
    {
        GASStringNode* n = Table[i];
        while (n)
        {
            GASStringNode* next = *ChainNextOf(n);
            if (n->RefCount == 0)
            {
                free((void*)n->pData);
                free(n);
            }
            n = next;
        }
    }
    free(Table);
}

GASStringNode* GASStringManager::Find(const char* str, unsigned int len, unsigned int hash) const
{
    GASStringNode* n = Table[hash & (TableSize - 1)];
    while (n)
    {
        if (n->HashCode == hash && n->Size == len && memcmp(n->pData, str, len) == 0)
            return n;
        n = *ChainNextOf(n);
    }
    return 0;
}

void GASStringManager::Rehash()
{
    unsigned int newSize = TableSize * 2;
    GASStringNode** newTable = (GASStringNode**)calloc(newSize, sizeof(GASStringNode*));
    for (unsigned int i = 0; i < TableSize; ++i)
    {
        GASStringNode* n = Table[i];
        while (n)
        {
            GASStringNode* next = *ChainNextOf(n);
            unsigned int slot = n->HashCode & (newSize - 1);
            *ChainNextOf(n) = newTable[slot];
            newTable[slot] = n;
            n = next;
        }
    }
    free(Table);
    Table = newTable;
    TableSize = newSize;
}

GASStringNode* GASStringManager::CreateStringNode(const char* str, unsigned int len) // 2012 0x9c9e50
{
    if (str == 0) { str = ""; len = 0; }
    unsigned int hash = HashOf(str, len);
    GASStringNode* found = Find(str, len, hash);
    if (found)
        return found;

    if (Count * 2 >= TableSize)
        Rehash();

    GASStringNode* n = (GASStringNode*)calloc(1, sizeof(GASStringNode) + sizeof(GASStringNode*));
    char* data = (char*)malloc(len + 1);
    memcpy(data, str, len);
    data[len] = 0;
    n->pData = data;
    n->pLower = 0;
    n->RefCount = 0;
    n->HashCode = hash;
    n->Size = len;

    unsigned int slot = hash & (TableSize - 1);
    *ChainNextOf(n) = Table[slot];
    Table[slot] = n;
    ++Count;
    return n;
}

GASStringNode* GASStringManager::CreateStringNode(const char* str)     // 2012 0x9c9f30
{
    return CreateStringNode(str, str ? (unsigned int)strlen(str) : 0);
}

void GASStringManager::FreeStringNode(GASStringNode* node)             // 2012 0x9c98a0
{
    (void)node;
}

void GASStringManager::ResolveLowercase(GASStringNode* node)           // 2012 0x9ca4b0
{
    if (node == 0 || node->pLower != 0)
        return;
    char stack[256];
    char* buf = node->Size < sizeof(stack) ? stack : (char*)malloc(node->Size + 1);
    for (unsigned int i = 0; i < node->Size; ++i)
    {
        char c = node->pData[i];
        buf[i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }
    buf[node->Size] = 0;
    node->pLower = CreateStringNode(buf, node->Size);
    if (buf != stack)
        free(buf);
}

bool GASString::operator==(const char* s) const                        // 2012 0x9f3660
{
    if (pNode == 0)
        return s == 0 || s[0] == 0;
    return strcmp(pNode->pData, s ? s : "") == 0;
}

bool GASString::operator<(const GASString& o) const                    // 2012 0x9ca870
{
    return strcmp(ToCStr(), o.ToCStr()) < 0;
}

bool GASString::operator>(const GASString& o) const                    // 2012 0x9ca8c0
{
    return strcmp(ToCStr(), o.ToCStr()) > 0;
}

bool GASString::EqualsNoCase(const GASString& o) const                 // 2012 0x9deb60
{
    const char* a = ToCStr();
    const char* b = o.ToCStr();
    while (*a && *b)
    {
        char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a - 'A' + 'a') : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b - 'A' + 'a') : *b;
        if (ca != cb)
            return false;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

const GASString& GASStringContext::GetBuiltin(GASBuiltinString which) const
{
    return pContext->GetBuiltin(which);
}

// ---------------------------------------------------------------------------------------------
// GASValue

void GASValue::DropRefs()                                             // 2012 0x9caf50
{
    // Only STRING and above hold a reference; every retail setter guards DropRefs with `type >= 5`,
    // which is exactly this test (GASValue::SetString 0x9acbe0, SetNumber 0x9acc00, SetInt 0xa65810).
    switch (T)
    {
    case STRING:
        if (V.pStringNode) V.pStringNode->Release();
        break;
    case OBJECT:
        if (V.pObject) V.pObject->Release();
        break;
    case CHARACTER:
        if (V.pCharHandle) V.pCharHandle->Release();
        break;
    case FUNCTION:
    case RESOLVE_HANDLER:
        if (V.pFunction) V.pFunction->Release();
        break;
    case PROPERTY:
        if (V.Prop.pGetter) V.Prop.pGetter->Release();
        if (V.Prop.pSetter) V.Prop.pSetter->Release();
        break;
    default:
        break;
    }
    V.NValue = 0.0;
    T = UNDEFINED;
}

void GASValue::Assign(const GASValue& src)
{
    if (this == &src)
        return;
    // Take the new reference before dropping the old one: self-assignment through an alias
    // (a member being written from itself) is normal in AS2.
    GASValue tmp;
    tmp.T = src.T;
    tmp.V = src.V;
    switch (src.T)
    {
    case STRING:          if (tmp.V.pStringNode) tmp.V.pStringNode->AddRef(); break;
    case OBJECT:          if (tmp.V.pObject) tmp.V.pObject->AddRef(); break;
    case CHARACTER:       if (tmp.V.pCharHandle) tmp.V.pCharHandle->AddRef(); break;
    case FUNCTION:
    case RESOLVE_HANDLER: if (tmp.V.pFunction) tmp.V.pFunction->AddRef(); break;
    case PROPERTY:
        if (tmp.V.Prop.pGetter) tmp.V.Prop.pGetter->AddRef();
        if (tmp.V.Prop.pSetter) tmp.V.Prop.pSetter->AddRef();
        break;
    default: break;
    }
    DropRefs();
    T = tmp.T;
    V = tmp.V;
    tmp.T = UNDEFINED;
    tmp.V.NValue = 0.0;
}

void GASValue::SetUndefined() { DropRefs(); T = UNDEFINED; }           // 2012 0x9acbc0
void GASValue::SetUnset() { DropRefs(); T = UNSET; }
void GASValue::SetNull() { DropRefs(); T = NULLTYPE; }                 // 2012 0x9acbd0
void GASValue::SetBool(bool b) { DropRefs(); T = BOOLEAN; V.BValue = b; }   // 2012 0x9acc20
void GASValue::SetNumber(double d) { DropRefs(); T = NUMBER; V.NValue = d; } // 2012 0x9acc00
void GASValue::SetInt(int i) { DropRefs(); T = INT; V.IValue = i; }    // 2012 0xa65810

void GASValue::SetString(const GASString& s)                          // 2012 0x9acbe0
{
    GASStringNode* n = s.pNode;
    if (n) n->AddRef();
    DropRefs();
    T = STRING;
    V.pStringNode = n;
}

void GASValue::SetAsObject(GASObject* o)                              // 2012 0x9cbd30
{
    // The retail body forwards to SetAsFunction when the object reports GetObjectType() == 23,
    // which the measured GASFunctionObject::GetObjectType (2012 0x9af140) says is a function.
    if (o && o->GetObjectType() == Object_Function)
    {
        SetAsFunction(o->ToFunction());
        return;
    }
    if (T == OBJECT && V.pObject == o)
        return;
    if (o) o->AddRef();
    DropRefs();
    T = OBJECT;
    V.pObject = o;
}

void GASValue::SetAsCharacter(GFxASCharacter* c)                      // 2012 0x9cb7e0
{
    GFxCharacterHandle* h = 0;
    if (c)
    {
        h = c->GetCharacterHandle();
        if (h == 0)
            h = c->CreateCharacterHandle();
    }
    if (T == CHARACTER && V.pCharHandle == h)
        return;
    if (h) h->AddRef();
    DropRefs();
    T = CHARACTER;
    V.pCharHandle = h;
}

void GASValue::SetAsFunction(GASFunctionObject* f)                     // 2012 0x9cb830
{
    if (T == FUNCTION && V.pFunction == f)
        return;
    if (f) f->AddRef();
    DropRefs();
    T = FUNCTION;
    V.pFunction = f;
}

void GASValue::SetProperty(GASFunctionObject* getter, GASFunctionObject* setter)
{
    if (getter) getter->AddRef();
    if (setter) setter->AddRef();
    DropRefs();
    T = PROPERTY;
    V.Prop.pGetter = getter;
    V.Prop.pSetter = setter;
}

GFxASCharacter* GASValue::GetCharacter() const
{
    if (T != CHARACTER || V.pCharHandle == 0)
        return 0;
    return V.pCharHandle->pCharacter;
}

bool GASValue::TypesMatch(const GASValue& o) const                     // 2012 0x9deb90
{
    if (T == o.T)
        return true;
    return (T == NUMBER || T == INT) && (o.T == NUMBER || o.T == INT);
}

bool GASValue::ToBool(GASEnvironment* env) const                        // 2012 0x9cac70
{
    switch (T)
    {
    case UNDEFINED:
    case UNSET:
    case NULLTYPE:
        return false;
    case BOOLEAN:
        return V.BValue;
    case NUMBER:
        return V.NValue != 0.0 && V.NValue == V.NValue;
    case INT:
        return V.IValue != 0;
    case STRING:
    {
        // AS2 version 6 and earlier coerce a string to a number first; 7 and later use
        // non-emptiness. The cook is version 8 and 10 (agentBB.md 4), so the non-empty rule is the
        // live one, and the older branch is kept because the fontlib movies are version 8 too.
        const char* s = GetStringCStr();
        if (env && env->GetGC()->GetSWFVersion() <= 6)
            return atof(s) != 0.0;
        return s[0] != 0;
    }
    case OBJECT:
    case CHARACTER:
    case FUNCTION:
    case RESOLVE_HANDLER:
        return true;
    default:
        return false;
    }
}

double GASValue::ToNumber(GASEnvironment* env) const                   // 2012 0x9cc2a0
{
    switch (T)
    {
    case NUMBER:  return V.NValue;
    case INT:     return (double)V.IValue;
    case BOOLEAN: return V.BValue ? 1.0 : 0.0;
    case STRING:
    {
        const char* s = GetStringCStr();
        while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
        if (*s == 0)
            return 0.0;
        char* end = 0;
        double d;
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            d = (double)strtol(s, &end, 16);
        else
            d = strtod(s, &end);
        if (end == s)
            return GFxAS2NaN();
        while (*end == ' ' || *end == '\t') ++end;
        return *end == 0 ? d : GFxAS2NaN();
    }
    case NULLTYPE:
        return 0.0;
    case OBJECT:
    case CHARACTER:
    {
        GASValue prim;
        if (ToPrimitiveInto(env, &prim))
            return prim.ToNumber(env);
        return GFxAS2NaN();
    }
    default:
        return GFxAS2NaN();
    }
}

int GASValue::ToInt32(GASEnvironment* env) const                       // 2012 0x9cc500
{
    double d = ToNumber(env);
    if (d != d || d == GFxAS2Infinity() || d == -GFxAS2Infinity())
        return 0;
    return (int)(long long)d;
}

unsigned int GASValue::ToUInt32(GASEnvironment* env) const             // 2012 0x9cc5f0
{
    return (unsigned int)ToInt32(env);
}

GASString GASValue::ToString(GASEnvironment* env) const                // 2012 0x9cbe50
{
    GASStringContext* sc = env->GetSC();
    if (env->ToStringDepth >= 255)
        return sc->CreateString("[object Object]");
    switch (T)
    {
    case UNDEFINED:
    case UNSET:
        return sc->GetBuiltin(GASbuiltin_undefined);
    case NULLTYPE:
        return sc->GetBuiltin(GASbuiltin_null);
    case BOOLEAN:
        return sc->GetBuiltin(V.BValue ? GASbuiltin_true : GASbuiltin_false);
    case NUMBER:
    {
        char buf[64];
        GFxAS2NumberToString(V.NValue, buf, sizeof(buf));
        return sc->CreateString(buf);
    }
    case INT:
    {
        char buf[32];
        sprintf(buf, "%d", V.IValue);
        return sc->CreateString(buf);
    }
    case STRING:
        return GetString();
    case OBJECT:
    case CHARACTER:
    {
        // The retail body calls the object's own toString() when it has one, and falls back to
        // [type Object] / the character's target path otherwise. It also guards against recursion
        // with a depth counter that gives up at 255 (0x9cbe50: `if (v18 >= 0xFFu)`).
        GASValue prim;
        if (ToPrimitiveInto(env, &prim) && !prim.IsObjectOrCharacter())
            return prim.ToString(env);
        if (T == CHARACTER)
        {
            GFxASCharacter* ch = GetCharacter();
            if (ch)
                return ch->GetTargetPath(sc);
        }
        return sc->CreateString("[object Object]");
    }
    case FUNCTION:
    case RESOLVE_HANDLER:
        return sc->CreateString("[type Function]");
    default:
        return sc->GetBuiltin(GASbuiltin_undefined);
    }
}

GASString GASValue::Typeof(GASEnvironment* env) const
{
    // The inner switch of GASActionBuffer::Execute's ActionTypeOf case (0x44) is on exactly these
    // value types: 0/0xA, 1, 2, 5, 6, 7, 8.
    GASStringContext* sc = env->GetSC();
    switch (T)
    {
    case UNDEFINED:
    case UNSET:     return sc->GetBuiltin(GASbuiltin_undefined);
    case NULLTYPE:  return sc->CreateString("null");
    case BOOLEAN:   return sc->GetBuiltin(GASbuiltin_boolean);
    case NUMBER:
    case INT:       return sc->GetBuiltin(GASbuiltin_number);
    case STRING:    return sc->GetBuiltin(GASbuiltin_string);
    case OBJECT:    return sc->GetBuiltin(GASbuiltin_object);
    case CHARACTER: return sc->GetBuiltin(GASbuiltin_movieclip);
    case FUNCTION:
    case RESOLVE_HANDLER: return sc->GetBuiltin(GASbuiltin_function);
    default:        return sc->GetBuiltin(GASbuiltin_undefined);
    }
}

GASObjectInterface* GASValue::ToObjectInterface(GASEnvironment* env) const  // 2012 0x9cbc60
{
    switch (T)
    {
    case OBJECT:    return V.pObject;
    case FUNCTION:
    case RESOLVE_HANDLER: return V.pFunction;
    case CHARACTER: return GetCharacter();
    case STRING:
    case NUMBER:
    case INT:
    case BOOLEAN:
        // Box it, so a method found on the prototype still sees the primitive as `this`.
        return env ? env->PrimitiveToTempObject(*this) : 0;
    default:        return 0;
    }
}

GASObject* GASValue::ToObject(GASEnvironment* env) const                // 2012 0x9cbb10
{
    GASObjectInterface* oi = ToObjectInterface(env);
    return oi ? oi->ToASObject() : 0;
}

void GASValue::GetPropertyValue(GASEnvironment* env, GASObjectInterface* self, GASValue* out) const
{
    GASFunctionObject* getter = GetPropertyGetter();
    if (getter == 0)
    {
        out->SetUndefined();
        return;
    }
    GASValue result;
    GASFnCall call(&result, self, env, 0, env->GetTopIndex());
    getter->Invoke(call);
    *out = result;
}

void GASValue::SetPropertyValue(GASEnvironment* env, GASObjectInterface* self, const GASValue& v)
{
    GASFunctionObject* setter = GetPropertySetter();
    if (setter == 0)
        return;
    GASValue result;
    env->Push(v);
    GASFnCall call(&result, self, env, 1, env->GetTopIndex());
    setter->Invoke(call);
    env->Drop(1);
}

int GASValue::PrototypeForPrimitive() const
{
    switch (T)
    {
    case STRING:  return (int)GASGlobalContext::Proto_String;
    case NUMBER:
    case INT:     return (int)GASGlobalContext::Proto_Number;
    case BOOLEAN: return (int)GASGlobalContext::Proto_Boolean;
    default:      return (int)GASGlobalContext::Proto_Object;
    }
}

bool GASValue::ToPrimitiveInto(GASEnvironment* env, GASValue* out) const   // 2012 0x9cc880
{
    GASObjectInterface* oi = ToObjectInterface(env);
    if (oi == 0 || env == 0)
    {
        *out = *this;
        return !IsObjectOrCharacter();
    }
    // The retail guard, same limit: an object whose valueOf or toString returns an object would
    // otherwise recurse for ever, and one asset in the cook (UI_Shop) does exactly that.
    if (env->ToStringDepth >= 255)
    {
        *out = *this;
        return false;
    }
    ++env->ToStringDepth;
    struct DepthGuard { GASEnvironment* e; ~DepthGuard() { --e->ToStringDepth; } } guard = { env };
    // valueOf first, then toString: the order GASValue::ToPrimitive takes with Hint_Number, which is
    // the hint every arithmetic caller passes.
    static const GASBuiltinString order[2] = { GASbuiltin_valueOf, GASbuiltin_toString };
    for (int i = 0; i < 2; ++i)
    {
        GASValue fnVal;
        if (!oi->GetMember(env, env->GetBuiltin(order[i]), &fnVal))
            continue;
        GASFunctionObject* fn = fnVal.GetFunction();
        if (fn == 0)
            continue;
        GASValue result;
        GASFnCall call(&result, oi, env, 0, env->GetTopIndex());
        fn->Invoke(call);
        if (!result.IsObjectOrCharacter())
        {
            *out = result;
            return true;
        }
    }
    *out = *this;
    return false;
}

// The arithmetic. Every one of these is its own retail function and the rva says which; the shape is
// the same in all of them - coerce both sides, compute, store.
void GASValue::Add(GASEnvironment* env, const GASValue& r)            // 2012 0x9ccc70
{
    // ActionAdd2 (0x47) is the one operator that is string concatenation when either side is a
    // string after primitive conversion, and numeric otherwise.
    GASValue lp, rp;
    const GASValue* l = this;
    const GASValue* rr = &r;
    if (IsObjectOrCharacter() && ToPrimitiveInto(env, &lp)) l = &lp;
    if (r.IsObjectOrCharacter() && r.ToPrimitiveInto(env, &rp)) rr = &rp;

    if (l->IsString() || rr->IsString())
    {
        GASString a = l->ToString(env);
        GASString b = rr->ToString(env);
        unsigned int n = a.GetSize() + b.GetSize();
        char stack[512];
        char* buf = n < sizeof(stack) ? stack : (char*)malloc(n + 1);
        memcpy(buf, a.ToCStr(), a.GetSize());
        memcpy(buf + a.GetSize(), b.ToCStr(), b.GetSize());
        buf[n] = 0;
        GASString sum = env->GetSC()->CreateString(buf, n);
        if (buf != stack) free(buf);
        SetString(sum);
        return;
    }
    SetNumber(l->ToNumber(env) + rr->ToNumber(env));
}

void GASValue::Sub(GASEnvironment* env, const GASValue& r)            // 2012 0x9dec00
{
    SetNumber(ToNumber(env) - r.ToNumber(env));
}

void GASValue::Mul(GASEnvironment* env, const GASValue& r)            // 2012 0x9dec50
{
    SetNumber(ToNumber(env) * r.ToNumber(env));
}

void GASValue::Div(GASEnvironment* env, const GASValue& r)            // 2012 0x9cc770
{
    double d = r.ToNumber(env);
    double n = ToNumber(env);
    if (d == 0.0)
    {
        // AS2 divides by zero to +-Infinity, and 0/0 to NaN; the retail body has the same three-way
        // split rather than raising.
        if (n == 0.0 || GFxAS2IsNaN(n)) SetNumber(GFxAS2NaN());
        else SetNumber(n > 0.0 ? GFxAS2Infinity() : -GFxAS2Infinity());
        return;
    }
    SetNumber(n / d);
}

void GASValue::Mod(GASEnvironment* env, const GASValue& r)
{
    double d = r.ToNumber(env);
    double n = ToNumber(env);
    if (d == 0.0 || GFxAS2IsNaN(n) || GFxAS2IsNaN(d)) { SetNumber(GFxAS2NaN()); return; }
    SetNumber(fmod(n, d));
}

void GASValue::And(GASEnvironment* env, const GASValue& r)             // 2012 0x9deca0
{ SetInt(ToInt32(env) & r.ToInt32(env)); }
void GASValue::Or(GASEnvironment* env, const GASValue& r)              // 2012 0x9dece0
{ SetInt(ToInt32(env) | r.ToInt32(env)); }
void GASValue::Xor(GASEnvironment* env, const GASValue& r)             // 2012 0x9ded20
{ SetInt(ToInt32(env) ^ r.ToInt32(env)); }
void GASValue::Shl(GASEnvironment* env, const GASValue& r)             // 2012 0x9ded60
{ SetInt(ToInt32(env) << (r.ToInt32(env) & 31)); }
void GASValue::Asr(GASEnvironment* env, const GASValue& r)             // 2012 0x9deda0
{ SetInt(ToInt32(env) >> (r.ToInt32(env) & 31)); }
void GASValue::Lsr(GASEnvironment* env, const GASValue& r)             // 2012 0x9dede0
{ SetNumber((double)(ToUInt32(env) >> (r.ToInt32(env) & 31))); }

bool GASValue::IsEqual(GASEnvironment* env, const GASValue& r) const   // 2012 0x9ccf40
{
    // ActionEquals2 (0x49): abstract equality. undefined == null, number/string coerce, objects
    // compare by identity.
    ValueType lt = T, rt = r.T;
    if (lt == UNSET) lt = UNDEFINED;
    if (rt == UNSET) rt = UNDEFINED;
    if ((lt == UNDEFINED || lt == NULLTYPE) && (rt == UNDEFINED || rt == NULLTYPE))
        return true;
    if (lt == UNDEFINED || lt == NULLTYPE || rt == UNDEFINED || rt == NULLTYPE)
        return false;

    if (lt == STRING && rt == STRING)
        return V.pStringNode == r.V.pStringNode || strcmp(GetStringCStr(), r.GetStringCStr()) == 0;
    if (IsObjectOrCharacter() && r.IsObjectOrCharacter())
        return GetRawPointer() == r.GetRawPointer();
    if (IsFunction() && r.IsFunction())
        return V.pFunction == r.V.pFunction;
    if (IsObjectOrCharacter() || r.IsObjectOrCharacter())
    {
        GASValue lp, rp;
        const GASValue* l = this;
        const GASValue* rr = &r;
        if (IsObjectOrCharacter() && ToPrimitiveInto(env, &lp)) l = &lp;
        if (r.IsObjectOrCharacter() && r.ToPrimitiveInto(env, &rp)) rr = &rp;
        if (l == this && rr == &r)
            return GetRawPointer() == r.GetRawPointer();
        return l->IsEqual(env, *rr);
    }
    if (lt == BOOLEAN || rt == BOOLEAN || IsNumber() || r.IsNumber())
    {
        double a = ToNumber(env), b = r.ToNumber(env);
        if (GFxAS2IsNaN(a) || GFxAS2IsNaN(b))
            return false;
        return a == b;
    }
    return strcmp(GetStringCStr(), r.GetStringCStr()) == 0;
}

bool GASValue::IsStrictEqual(GASEnvironment* env, const GASValue& r) const
{
    // ActionStrictEquals (0x66).
    if (!TypesMatch(r))
        return false;
    switch (T)
    {
    case UNDEFINED:
    case UNSET:
    case NULLTYPE: return true;
    case BOOLEAN:  return V.BValue == r.V.BValue;
    case NUMBER:
    case INT:
    {
        double a = GetNumber(), b = r.GetNumber();
        if (GFxAS2IsNaN(a) || GFxAS2IsNaN(b)) return false;
        return a == b;
    }
    case STRING:   return V.pStringNode == r.V.pStringNode
                       || strcmp(GetStringCStr(), r.GetStringCStr()) == 0;
    default:       return GetRawPointer() == r.GetRawPointer();
    }
}

int GASValue::Compare(GASEnvironment* env, const GASValue& r) const     // 2012 0x9cd420
{
    // -1 less, 0 equal, 1 greater, 2 "not comparable" (a NaN on either side), which is the fourth
    // result ActionLess2/ActionGreater need in order to push false for both directions.
    GASValue lp, rp;
    const GASValue* l = this;
    const GASValue* rr = &r;
    if (IsObjectOrCharacter() && ToPrimitiveInto(env, &lp)) l = &lp;
    if (r.IsObjectOrCharacter() && r.ToPrimitiveInto(env, &rp)) rr = &rp;

    if (l->IsString() && rr->IsString())
    {
        int c = strcmp(l->GetStringCStr(), rr->GetStringCStr());
        return c < 0 ? -1 : (c > 0 ? 1 : 0);
    }
    double a = l->ToNumber(env), b = rr->ToNumber(env);
    if (GFxAS2IsNaN(a) || GFxAS2IsNaN(b))
        return 2;
    return a < b ? -1 : (a > b ? 1 : 0);
}
