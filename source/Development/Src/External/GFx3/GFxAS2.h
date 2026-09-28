// Scaleform GFx 3.3.89 - the ActionScript 2 machine: the value model, the object model and the
// bytecode interpreter. Package BC of resources/docs/PHASE8.md.
//
// HOW THIS DIFFERS FROM THE REST OF THE DIRECTORY, and it matters: agent BB's GFx3Gen.h is
// generated from the 2012 Shipping PDB with every member at its recorded offset, because the PDB
// carries the layout of every type the engine's own translation units name. The runtime's internal
// types are NOT in that PDB: dia_types.py on GASObject, GASValue, GASEnvironment, GASActionBuffer,
// GFxSprite, GFxMovieRoot and GFxASCharacter all come back sizeof 0, i.e. a forward declaration
// with no members (build/agentBC/gas_types.json: one UDT, GASStringContext, size 0). libgfx was
// linked in as a library whose type stream the game's PDB does not carry.
//
// What the symbols DO give, and what this file is therefore built from:
//   * every libgfx function's fully demangled MSVC signature - class, method, every parameter type
//     and constness - for all 5,635 of them (resources/docs/symbols/functions.csv, module=libgfx;
//     build/agentBC/surface.py reads them out). That fixes the API of every class below.
//   * resources/docs/symbols/vtables.csv for the dispatch shape.
//   * headless Hex-Rays decompiles of the load-bearing bodies (build/agentBC/dec, dec2), which fix
//     the semantics and, where a decompile reads a constant, the constant. GASValue::ValueType is
//     pinned enumerator by enumerator that way - see the enum's comment.
// So: behaviour is ported, member offsets are not reproduced and are not reproducible. Nothing here
// is ABI-visible to retail (we replace libgfx rather than call into it), so that costs nothing; it
// does mean this file carries no GFX3_ASSERT_OFFSET, and GFx3Layout.cpp is untouched by it.
//
// DISHONORED(port): 2013 rvas in resources/docs/agents/agentBC_status.csv; the rva in a comment
// below is the 2012 address of the function that body was read from (93 % of libgfx is
// byte-identical between the two builds, gfx_decision.md 1).
#ifndef INC_GFX3_GFXAS2_H
#define INC_GFX3_GFXAS2_H

#include "GFx3.h"

#pragma pack(push, 8)

class GASObject;
class GASObjectInterface;
class GASArrayObject;
class GASFunctionObject;
class GASEnvironment;
class GASGlobalContext;
class GASActionBuffer;
class GASFnCall;
class GFxASCharacter;
class GFxSprite;
class GFxMovieRoot;
class GFxCharacterHandle;

// The three number helpers retail keeps in GASNumberUtil (IsNaN 2012 0x9acb10,
// IsPOSITIVE_INFINITY 0x9acb50, ToString / IntToString called from GASValue::ToStringImpl).
double GFxAS2NaN();
double GFxAS2Infinity();
bool   GFxAS2IsNaN(double d);
void   GFxAS2NumberToString(double d, char* out, unsigned int outSize);

// ---------------------------------------------------------------------------------------------
// Interned strings. GASStringNode's first four words are read straight out of the decompiles:
// GASObject::GetMemberRaw (2012 0x9dc890) uses node+0 as the character data (CallFrameActions
// 0x9f63b0 does **((const char ***)handle + 3)), node+4 as the lowercase-resolved node
// (GASStringNode::ResolveLowercase_Impl 0x9ca4b0 fills it and the case-insensitive lookup keys on
// node[1]->HashCode), node+8 as the reference count (GASValue::SetString 0x9acbe0 does
// ++*(node+8)) and node+12 as the cached hash code (*(node+12) & table->Mask).
struct GASStringNode
{
    const char*    pData;
    GASStringNode* pLower;
    int            RefCount;
    unsigned int   HashCode;
    unsigned int   Size;

    void AddRef() { ++RefCount; }
    void Release();                                                   // 2012 0x9acaf0 / 0x9c9d20
};

class GASStringManager;

class GASString
{
public:
    GASStringNode* pNode;

    GASString() : pNode(0) {}
    explicit GASString(GASStringNode* node) : pNode(node) { if (pNode) pNode->AddRef(); }
    GASString(const GASString& src) : pNode(src.pNode) { if (pNode) pNode->AddRef(); }
    ~GASString() { if (pNode) pNode->Release(); }

    GASString& operator=(const GASString& src)
    {
        if (src.pNode) src.pNode->AddRef();
        if (pNode) pNode->Release();
        pNode = src.pNode;
        return *this;
    }

    bool IsEmpty() const { return pNode == 0 || pNode->Size == 0; }
    const char*  ToCStr() const { return pNode ? pNode->pData : ""; }
    unsigned int GetSize() const { return pNode ? pNode->Size : 0; }
    unsigned int GetLength() const { return GetSize(); }              // 2012 0x9c9580
    unsigned int GetHash() const { return pNode ? pNode->HashCode : 0; }

    // Interning makes identity the fast path and it is the one retail takes first
    // (GASObject::GetMemberRaw compares node pointers before it ever hashes).
    bool operator==(const GASString& o) const { return pNode == o.pNode; }
    bool operator!=(const GASString& o) const { return pNode != o.pNode; }
    bool operator==(const char* s) const;                             // 2012 0x9f3660
    bool operator<(const GASString& o) const;                         // 2012 0x9ca870
    bool operator>(const GASString& o) const;                         // 2012 0x9ca8c0

    bool EqualsNoCase(const GASString& o) const;                      // 2012 0x9deb60
};

// GASStringManager interns every string of one movie. Retail's is a hash set of nodes plus a
// text-buffer pool (AllocateStringNodes 0x9c9730, AllocTextBuffer 0x9c9840); this is the hash set
// and one allocation per node, which is the same contract without the pool.
class GASStringManager
{
public:
    GASStringManager();                                               // 2012 0x9ca580
    ~GASStringManager();                                              // 2012 0x9ca630

    GASStringNode* CreateStringNode(const char* str);                 // 2012 0x9c9f30
    GASStringNode* CreateStringNode(const char* str, unsigned int len);// 2012 0x9c9e50
    GASString      CreateString(const char* str) { return GASString(CreateStringNode(str)); }
    GASString      CreateString(const char* str, unsigned int len)
                       { return GASString(CreateStringNode(str, len)); }
    GASString      CreateEmptyString() { return CreateString(""); }

    void FreeStringNode(GASStringNode* node);                         // 2012 0x9c98a0
    void ResolveLowercase(GASStringNode* node);                       // 2012 0x9ca4b0

    static unsigned int HashOf(const char* str, unsigned int len);

    unsigned int GetNodeCount() const { return Count; }

private:
    GASStringNode** Table;
    unsigned int    TableSize;
    unsigned int    Count;

    void Rehash();
    GASStringNode* Find(const char* str, unsigned int len, unsigned int hash) const;
};

// The builtin strings the interpreter and the class library compare against by identity. Retail
// keeps them in GASStringBuiltinManager and GASGlobalContext reaches them through
// GASStringContext; GASObject::GetMemberRaw reads __proto__ at context+320 and __resolve at
// context+336, which is this table by another name.
enum GASBuiltinString
{
    GASbuiltin_empty = 0,
    GASbuiltin_undefined,
    GASbuiltin_null,
    GASbuiltin_true,
    GASbuiltin_false,
    GASbuiltin_NaN,
    GASbuiltin_Infinity,
    GASbuiltin_minusInfinity,
    GASbuiltin_zero,
    GASbuiltin_proto,             // __proto__
    GASbuiltin_constructorUS,     // __constructor__, an ordinary member
    GASbuiltin_resolve,           // __resolve, the object's own resolve handler
    GASbuiltin_prototype,
    GASbuiltin_constructor,
    GASbuiltin_toString,
    GASbuiltin_valueOf,
    GASbuiltin_length,
    GASbuiltin_this,
    GASbuiltin_super,
    GASbuiltin__global,
    GASbuiltin__root,
    GASbuiltin__parent,
    GASbuiltin__level0,
    GASbuiltin_arguments,
    GASbuiltin_callee,
    GASbuiltin_caller,
    GASbuiltin_apply,
    GASbuiltin_call,
    GASbuiltin_Object,
    GASbuiltin_Array,
    GASbuiltin_String,
    GASbuiltin_Number,
    GASbuiltin_Boolean,
    GASbuiltin_Function,
    GASbuiltin_Math,
    GASbuiltin_MovieClip,
    GASbuiltin_Error,
    GASbuiltin_object,
    GASbuiltin_movieclip,
    GASbuiltin_function,
    GASbuiltin_string,
    GASbuiltin_number,
    GASbuiltin_boolean,
    GASbuiltin_onLoad,
    GASbuiltin_onEnterFrame,
    GASbuiltin_onUnload,
    GASbuiltin__x,
    GASbuiltin__y,
    GASbuiltin__visible,
    GASbuiltin__alpha,
    GASbuiltin__name,
    GASbuiltin__target,
    GASbuiltin__currentframe,
    GASbuiltin__totalframes,
    GASbuiltin__width,
    GASbuiltin__height,
    GASbuiltin__xscale,
    GASbuiltin__yscale,
    GASbuiltin__rotation,
    GASbuiltin_COUNT
};

class GASStringContext
{
public:
    GASStringManager* pStrings;
    GASGlobalContext* pContext;
    unsigned int      Version;        // the SWF version; the case-insensitive path is <= 6

    GASStringContext() : pStrings(0), pContext(0), Version(10) {}

    GASString CreateString(const char* s) { return pStrings->CreateString(s); }
    GASString CreateString(const char* s, unsigned int n) { return pStrings->CreateString(s, n); }
    GASString CreateConstString(const char* s) { return pStrings->CreateString(s); }
    const GASString& GetBuiltin(GASBuiltinString which) const;
    bool IsCaseInsensitive() const { return Version <= 6; }
};

// ---------------------------------------------------------------------------------------------
// GASValue. Every enumerator below is read out of a decompiled retail body rather than guessed,
// which is the trap agent BB's section 2.4a was caught by:
//   SetUndefined 0x9acbc0 writes 0, SetNull 0x9acbd0 writes 1, SetBool 0x9acc20 writes 2,
//   SetNumber 0x9acc00 writes 3, SetInt 0xa65810 writes 4, SetString 0x9acbe0 writes 5,
//   SetAsObject 0x9cbd30 writes 6, SetAsCharacter 0x9cb7e0 writes 7, SetAsFunction 0x9cb830
//   writes 8; GetMember 0x9b0450 treats 9 as a property and calls GetPropertyValue;
//   IsUndefined 0x9ca960 is (t == 0 || t == 10) and IsFunction 0x9ca940 is (t == 8 || t == 11),
//   which is what fixes UNSET at 10 and RESOLVE_HANDLER at 11.
// Two more facts from the same bodies: IsPrimitive 0x9ca9a0 is types 1..5, and every setter only
// calls DropRefs when the old type is >= 5, i.e. only STRING and up hold a reference.
class GASValue
{
public:
    enum ValueType
    {
        UNDEFINED       = 0,
        NULLTYPE        = 1,
        BOOLEAN         = 2,
        NUMBER          = 3,
        INT             = 4,
        STRING          = 5,
        OBJECT          = 6,
        CHARACTER       = 7,
        FUNCTION        = 8,
        PROPERTY        = 9,
        UNSET           = 10,
        RESOLVE_HANDLER = 11
    };

    GASValue() : T(UNDEFINED) { V.NValue = 0.0; }
    GASValue(const GASValue& src) : T(UNDEFINED) { V.NValue = 0.0; Assign(src); }
    explicit GASValue(bool b) : T(BOOLEAN) { V.NValue = 0.0; V.BValue = b; }
    explicit GASValue(double d) : T(NUMBER) { V.NValue = d; }
    explicit GASValue(int i) : T(INT) { V.NValue = 0.0; V.IValue = i; }
    explicit GASValue(const GASString& s) : T(UNDEFINED) { V.NValue = 0.0; SetString(s); }
    explicit GASValue(GASObject* o) : T(UNDEFINED) { V.NValue = 0.0; SetAsObject(o); }
    ~GASValue() { DropRefs(); }

    GASValue& operator=(const GASValue& src) { Assign(src); return *this; }

    ValueType GetType() const { return T; }

    bool IsUndefined() const { return T == UNDEFINED || T == UNSET; }  // 2012 0x9ca960
    bool IsUnset() const { return T == UNSET; }
    bool IsNull() const { return T == NULLTYPE; }
    bool IsBool() const { return T == BOOLEAN; }
    bool IsNumber() const { return T == NUMBER || T == INT; }          // 2012 0x9ca980
    bool IsString() const { return T == STRING; }
    bool IsObject() const { return T == OBJECT; }
    bool IsCharacter() const { return T == CHARACTER; }
    bool IsFunction() const { return T == FUNCTION || T == RESOLVE_HANDLER; } // 2012 0x9ca940
    bool IsProperty() const { return T == PROPERTY; }
    bool IsPrimitive() const { return T >= NULLTYPE && T <= STRING; }  // 2012 0x9ca9a0
    bool IsObjectOrCharacter() const { return T == OBJECT || T == CHARACTER; }
    bool TypesMatch(const GASValue& o) const;                         // 2012 0x9deb90

    void SetUndefined();
    void SetUnset();
    void SetNull();
    void SetBool(bool b);
    void SetNumber(double d);
    void SetInt(int i);
    void SetString(const GASString& s);
    void SetAsObject(GASObject* o);                                   // 2012 0x9cbd30
    void SetAsCharacter(GFxASCharacter* c);                           // 2012 0x9cb7e0
    void SetAsFunction(GASFunctionObject* f);                         // 2012 0x9cb830
    void SetProperty(GASFunctionObject* getter, GASFunctionObject* setter);

    bool               GetBool() const { return V.BValue; }
    double             GetNumber() const { return T == INT ? (double)V.IValue : V.NValue; }
    int                GetInt() const { return T == INT ? V.IValue : (int)V.NValue; }
    GASString          GetString() const { return GASString(V.pStringNode); }
    GASObject*         GetObject() const { return T == OBJECT ? V.pObject : 0; }
    GFxASCharacter*    GetCharacter() const;
    GASFunctionObject* GetFunction() const { return IsFunction() ? V.pFunction : 0; }
    GASFunctionObject* GetPropertyGetter() const { return T == PROPERTY ? V.Prop.pGetter : 0; }
    GASFunctionObject* GetPropertySetter() const { return T == PROPERTY ? V.Prop.pSetter : 0; }
    const char*        GetStringCStr() const
                           { return V.pStringNode ? V.pStringNode->pData : ""; }
    const void*        GetRawPointer() const { return (const void*)V.pObject; }

    // The conversions. Every one of these is a real retail function and the rva says which.
    bool                ToBool(GASEnvironment* env) const;            // 2012 0x9cac70
    double              ToNumber(GASEnvironment* env) const;          // 2012 0x9cc2a0
    int                 ToInt32(GASEnvironment* env) const;           // 2012 0x9cc500
    unsigned int        ToUInt32(GASEnvironment* env) const;          // 2012 0x9cc5f0
    GASString           ToString(GASEnvironment* env) const;          // 2012 0x9cbe50
    GASObjectInterface* ToObjectInterface(GASEnvironment* env) const; // 2012 0x9cbc60
    GASObject*          ToObject(GASEnvironment* env) const;          // 2012 0x9cbb10
    GASString           Typeof(GASEnvironment* env) const;

    // The operators the interpreter dispatches to, each its own retail function.
    void Add(GASEnvironment* env, const GASValue& r);                 // 2012 0x9ccc70
    void Sub(GASEnvironment* env, const GASValue& r);                 // 2012 0x9dec00
    void Mul(GASEnvironment* env, const GASValue& r);                 // 2012 0x9dec50
    void Div(GASEnvironment* env, const GASValue& r);                 // 2012 0x9cc770
    void Mod(GASEnvironment* env, const GASValue& r);
    void And(GASEnvironment* env, const GASValue& r);                 // 2012 0x9deca0
    void Or(GASEnvironment* env, const GASValue& r);                  // 2012 0x9dece0
    void Xor(GASEnvironment* env, const GASValue& r);                 // 2012 0x9ded20
    void Shl(GASEnvironment* env, const GASValue& r);                 // 2012 0x9ded60
    void Asr(GASEnvironment* env, const GASValue& r);                 // 2012 0x9deda0
    void Lsr(GASEnvironment* env, const GASValue& r);                 // 2012 0x9dede0
    bool IsEqual(GASEnvironment* env, const GASValue& r) const;       // 2012 0x9ccf40
    bool IsStrictEqual(GASEnvironment* env, const GASValue& r) const;
    int  Compare(GASEnvironment* env, const GASValue& r) const;       // 2012 0x9cd420

    // PROPERTY members are not values: reading one calls its getter, writing one calls its setter.
    // GFxValue::ObjectInterface::GetMember (0x9b0450) does exactly this after the raw lookup.
    void GetPropertyValue(GASEnvironment* env, GASObjectInterface* self, GASValue* out) const;
    void SetPropertyValue(GASEnvironment* env, GASObjectInterface* self, const GASValue& v);

    void DropRefs();                                                  // 2012 0x9caf50

    // GASValue::ToPrimitive (2012 0x9cc880): valueOf() then toString(), which is what every
    // arithmetic operator on an object goes through.
    bool ToPrimitiveInto(GASEnvironment* env, GASValue* out) const;

    // Which class-library prototype a primitive borrows its members from, so "abc".length and
    // (5).toString() work without boxing.
    int PrototypeForPrimitive() const;

private:
    void Assign(const GASValue& src);

    ValueType T;
    union ValueUnion
    {
        double              NValue;
        int                 IValue;
        bool                BValue;
        GASStringNode*      pStringNode;
        GASObject*          pObject;
        GFxCharacterHandle* pCharHandle;
        GASFunctionObject*  pFunction;
        struct { GASFunctionObject* pGetter; GASFunctionObject* pSetter; } Prop;
    } V;
};

// GASPropFlags: the ASSetPropFlags bits. 1 = don't enumerate, 2 = don't delete, 4 = read only -
// the values ASSetPropFlags takes, which is the interface the __Packages class registrations use on
// every class they install (669 DoInitAction tags in the cook, agentBB.md 3.5).
class GASPropFlags
{
public:
    enum { PropFlag_DontEnum = 1, PropFlag_DontDelete = 2, PropFlag_ReadOnly = 4 };
    unsigned char Flags;

    GASPropFlags() : Flags(0) {}
    explicit GASPropFlags(unsigned char f) : Flags(f) {}
    bool GetDontEnum() const { return (Flags & PropFlag_DontEnum) != 0; }
    bool GetDontDelete() const { return (Flags & PropFlag_DontDelete) != 0; }
    bool GetReadOnly() const { return (Flags & PropFlag_ReadOnly) != 0; }
};

class GASMember
{
public:
    GASValue     Value;
    GASPropFlags Flags;

    GASMember() {}
    GASMember(const GASValue& v, const GASPropFlags& f) : Value(v), Flags(f) {}
};

#pragma pack(pop)
#endif // INC_GFX3_GFXAS2_H
