// Scaleform GFx 3.3.89 - the ActionScript 2 object model. Package BC.
//
// The method set and the virtual-ness of GASObjectInterface below are NOT invented: they are the 21
// rows of resources/docs/symbols/vtables.csv for the classes `GASObjectInterface` and
// `GASObject{for GASObjectInterface}`, which IDA exported from the retail image with no PDB
// involved. That table is the reason this interface has both a `SetMember` and a `SetMemberRaw`
// (slots 3 and 10) and both a `GetMember` and a `GetMemberRaw` (4 and 11): the Raw pair does the
// hash-and-prototype-chain lookup, the plain pair adds the property (getter/setter) and watchpoint
// behaviour on top. Every engine call that reaches an AS2 object goes through the plain pair.
//
// Slot order is not reproduced here (no ABI interop with retail - see GFxAS2.h's header comment),
// only the set and the signatures.
#ifndef INC_GFX3_GFXAS2OBJECT_H
#define INC_GFX3_GFXAS2OBJECT_H

#include "GFxAS2.h"

#pragma pack(push, 8)

class GASRefCountCollector;

// GASObjectInterface::ObjectType. The nine values marked `measured` are read out of the 6-byte
// retail bodies that return them (build/agentBC/dec3); the two spans are read out of
// GFxValue::ObjectInterface::GetMember (2012 0x9b0450), which routes a property owner to
// ToASCharacter() when the type is in [2,5] and to ToASObject() when it is in [6,44]. The values
// inside the spans that no body in the cook reaches are interpolated and marked.
enum GASObjectType
{
    Object_Unknown          = 0,
    Object_Invalid          = 1,
    Object_Sprite           = 2,    // span [2,5] = the character types (0x9b0450)
    Object_Button           = 3,    // interpolated
    Object_EditText         = 4,    // interpolated
    Object_Video            = 5,    // interpolated
    Object_Object           = 6,    // span [6,44] = the script objects (0x9b0450)
    Object_Array            = 7,    // interpolated
    Object_Number           = 8,    // interpolated
    Object_Boolean          = 9,    // interpolated
    Object_String           = 10,   // interpolated
    Object_MovieClipObject  = 11,   // interpolated
    Object_Stage            = 12,   // interpolated
    Object_TextField        = 13,   // measured, GASTextFieldObject 2012 0xa26c80
    Object_Matrix           = 15,   // measured, the `== 15` guard in GASTransformObject::SetMember
                                    // 2013 0xa70a20 before GASMatrixObject::GetMatrix
    Object_ColorTransform   = 18,   // measured, the `== 18` guard in GASColorTransformCtorFunction
                                    // ::GlobalCtor 2013 0xa77c30 and in SetMember 0xa70a20
    Object_Transform        = 20,   // measured, the `== 20` guard in GASTransformCtorFunction
                                    // ::GlobalCtor 2013 0xa70c90
    Object_Key              = 22,   // measured, GASKeyObject 2012 0xa8a350
    Object_Function         = 23,   // measured, GASFunctionObject 2012 0x9af140
    Object_MovieClipLoader  = 25,   // measured, 2012 0x9c8770
    Object_BitmapData       = 26,   // measured, 2012 0xa84950
    Object_LoadVars         = 27,   // measured, 2012 0xa6a9f0
    Object_TextFormat       = 30,   // measured, 2012 0xa71950
    Object_StyleSheet       = 31,   // measured, 2012 0xaadf20
    Object_Date             = 35,   // measured, 2012 0xa76b80
    Object_Super            = 44    // the top of the ToASObject span
};

class GASObjectInterface
{
public:
    struct MemberVisitor
    {
        virtual ~MemberVisitor() {}
        virtual void Visit(const GASString& name, const GASValue& val, unsigned char flags) = 0;
    };
    enum VisitFlags { VisitMember_Prototype = 1, VisitMember_DontEnum = 2 };

    GASObjectInterface() {}                                           // 2012 0x9dbc70
    virtual ~GASObjectInterface() {}                                  // 2012 0x9dbc90

    virtual GASObjectType GetObjectType() const = 0;                  // vtables.csv slot 2

    virtual bool SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                           const GASPropFlags& flags) = 0;            // slot 3
    virtual bool GetMember(GASEnvironment* env, const GASString& name, GASValue* val) = 0; // slot 4
    virtual bool FindMember(GASStringContext* sc, const GASString& name,
                            GASMember* member) = 0;                   // slot 5
    virtual bool DeleteMember(GASStringContext* sc, const GASString& name) = 0;  // slot 6
    virtual bool SetMemberFlags(GASStringContext* sc, const GASString& name,
                                unsigned char flags) = 0;             // slot 7
    virtual void VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                              const GASObjectInterface* instance) const = 0;     // slot 8
    virtual bool HasMember(GASStringContext* sc, const GASString& name, bool inherited) = 0; // 9
    virtual bool SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                              const GASPropFlags& flags) = 0;         // slot 10
    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val) = 0; // 11

    virtual GASFunctionObject* ToFunction() { return 0; }             // slot 12, 2012 0x9dbcf0
    virtual void Set__proto__(GASStringContext* sc, GASObject* proto) = 0;        // slot 13
    virtual GASFunctionObject* Get__constructor__(GASStringContext* sc);          // slot 14

    // The three downcasts, all of which are `return 0` in the base and a fixed subobject
    // adjustment in the derived: GASObjectInterface::ToASObject (2012 0x9da8f0) returns
    // `this - 16` in GASObject, ToASCharacter (0x9da8d0) and ToSprite (0x9da910) return
    // `this - 120` in GFxASCharacter. That is where the +16 and +120 in every decompile of a
    // GFxValue path come from.
    virtual GASObject*      ToASObject() { return 0; }                // slot 15
    virtual GFxASCharacter* ToASCharacter() { return 0; }             // slot 16
    virtual GFxSprite*      ToSprite() { return 0; }                  // slot 17

    virtual bool InstanceOf(GASEnvironment* env, const GASObject* proto,
                            bool inherited) const = 0;                // slot 18
    virtual bool DoesImplement(GASEnvironment* env, const GASObject* iface) const
                           { (void)env; (void)iface; return false; }
    virtual GASValue GetValue() const { return GASValue(); }          // 2012 0x9da930

    // Non-virtual helpers retail carries on the interface itself.
    bool GetConstMemberRaw(GASStringContext* sc, const char* name,
                           GASValue* val);                            // 2012 0x9ae5e0
    bool SetConstMemberRaw(GASStringContext* sc, const char* name, const GASValue& val,
                           const GASPropFlags& flags);                // 2012 0x9d6dd0
    bool SetConstMemberRaw(GASStringContext* sc, const char* name,
                           const GASValue& val);                      // 2012 0x9e0880
    GASObjectInterface* FindOwner(GASStringContext* sc, const GASString& name); // 2012 0x9dac70
};

// GASObject. The member store is a hash of GASString -> GASMember, which is literally what the
// retail decompile names: GHashSetBase<GHashNode<GASString,GASMember,GASStringHashFunctor>, ...>
// inside GASObject::GetMemberRaw / SetMemberRaw. The prototype is one pointer
// (`v4[6]` in 0x9dc890) and the resolve handler one function ref (`v4[8]`).
class GASObject : public GASObjectInterface
{
public:
    GASObject(GASStringContext* sc);                                  // 2012 0x9dc740
    GASObject(GASStringContext* sc, GASObject* proto);                // 2012 0x9dc7b0
    virtual ~GASObject();                                             // 2012 0x9dd6c0

    // GRefCountBaseGC<323>::AddRef / Release (2013 0xda7b50, 2012 0x9aeb20): the collectable
    // reference count, with the Bacon-Rajan colouring the collector works on. See
    // GASRefCountCollector below for what Release does at zero and at non-zero.
    void AddRef() { ++RefCount; }
    void Release();
    unsigned int GetRefCount() const { return RefCount; }

    virtual GASObjectType GetObjectType() const { return Object_Object; }

    virtual bool SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                           const GASPropFlags& flags);                // 2012 0x9dde20
    virtual bool GetMember(GASEnvironment* env, const GASString& name, GASValue* val); // 0xa26fa0
    virtual bool FindMember(GASStringContext* sc, const GASString& name, GASMember* member);
    virtual bool DeleteMember(GASStringContext* sc, const GASString& name); // 2012 0x9dd280
    virtual bool SetMemberFlags(GASStringContext* sc, const GASString& name, unsigned char flags);
    virtual void VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                              const GASObjectInterface* instance) const;  // 2012 0x9db7e0
    virtual bool HasMember(GASStringContext* sc, const GASString& name, bool inherited);
    virtual bool SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                              const GASPropFlags& flags);             // 2012 0x9de1c0
    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val); // 0x9dc890
    virtual void Set__proto__(GASStringContext* sc, GASObject* proto); // 2012 0x9aef50
    virtual GASObject* ToASObject() { return this; }
    virtual bool InstanceOf(GASEnvironment* env, const GASObject* proto, bool inherited) const;
    virtual bool DoesImplement(GASEnvironment* env, const GASObject* iface) const; // 2012 0x9af130

    GASObject* Get__proto__() const { return pProto; }

    unsigned int GetMemberCount() const { return MemberCount; }
    void         ReleaseAllMembers();

    // DISHONORED(bringup): the member store read two ways, which is the whole of agent EA's
    // hand-over 3. FindNode is an interned-pointer compare; this asks the same question by text, so
    // a name that is present under a second node for the same characters says so instead of
    // reading as absent.
    bool DishonoredHasMemberByIdentity(GASStringContext* sc, const GASString& name) const
        { return FindNode(sc, name) != 0; }
    const void* DishonoredFindMemberByText(const char* name, unsigned int* outHash) const;

    // The two GRefCountBaseGC<323> virtuals, retail's slots 0 and 1 of the collectable's own vtable
    // (GASObject::ExecuteForEachChild_GC 2013 0xdd24d0, GASObject::Finalize_GC 2013 0xdd3970 /
    // 2012 0x9dd210). ExecuteForEachChild_GC hands every collectable this object references -
    // the member values, __proto__, __resolve - to GASRefCountCollector::VisitChild under the
    // operation; Finalize_GC drops those references WITHOUT releasing them, which is what the
    // collector needs when it frees a whole cycle whose counts it has already accounted for
    // (retail's ~GHashSetBase<..., GHashsetNodeEntry_GC<...>> in Finalize_GC does exactly that).
    // Strings and character handles are not collectable and are still released by the destructor.
    enum GCColor { GC_Black = 0, GC_Gray = 1, GC_White = 2, GC_Purple = 3 };
    virtual void ExecuteForEachChild_GC(int op) const;
    virtual void Finalize_GC();

    // Retail's GRefCountBaseGC<323> header (object+4 pRCC, +8 RefCount in the low 28 bits with the
    // colour at bits 28..30 and the buffered bit at 31, +12 RootIndex; the +4 word doubles as the
    // kill-list link once the object is on it). Spelled out as fields here.
    GASRefCountCollector* pCollector;
    GASObject*            pCollectorNext;    // the all-objects list the census and teardown walk
    GASObject*            pCollectorPrev;
    GASObject*            pKillNext;
    unsigned int          RefCount;
    int                   RootIndex;
    unsigned char         Color;
    bool                  bBuffered;         // in the collector's root buffer
    bool                  bInKillList;
    bool                  bCycleGarbage;     // on the kill list as part of a dead cycle: free without releasing

protected:
    struct MemberNode
    {
        GASString   Name;
        GASMember   Member;
        MemberNode* pNext;
    };

    MemberNode**       Table;
    unsigned int       TableSize;
    unsigned int       MemberCount;
    GASObject*         pProto;
    GASFunctionObject* pResolveHandler;
    bool               bDestroying;

    MemberNode* FindNode(GASStringContext* sc, const GASString& name) const;
    MemberNode* AddNode(const GASString& name);
    void        Rehash();
};

// GASArrayObject. Retail's is 30 functions in GASArrayObject.obj plus 99 in the prototype; the
// element store is a flat GArray of GASValue and `length` is a real member rather than a hash
// entry, which is what GFxValue::ObjectInterface::GetArraySize (2012 0x9ad650) reads.
class GASArrayObject : public GASObject
{
public:
    GASArrayObject(GASStringContext* sc);
    GASArrayObject(GASStringContext* sc, GASObject* proto);
    virtual ~GASArrayObject();

    virtual GASObjectType GetObjectType() const { return Object_Array; }

    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val);
    virtual bool SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                              const GASPropFlags& flags);
    virtual bool HasMember(GASStringContext* sc, const GASString& name, bool inherited);
    virtual void VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                              const GASObjectInterface* instance) const;
    virtual void ExecuteForEachChild_GC(int op) const;               // 2013 0xdcd650
    virtual void Finalize_GC();                                       // 2013 0xdce510

    unsigned int GetSize() const { return Size; }
    void         Resize(unsigned int n);
    const GASValue* GetElementPtr(unsigned int i) const { return i < Size ? &Elements[i] : 0; }
    void         SetElement(unsigned int i, const GASValue& v);
    void         PushBack(const GASValue& v);
    void         RemoveElements(unsigned int index, int count);

    // -1 when the name is not a decimal index, which is how AS2 tells arr[3] from arr.foo.
    static int   ParseIndex(const GASString& name);

private:
    GASValue*    Elements;
    unsigned int Size;
    unsigned int Capacity;
};

// A call into an AS2 function, C++ side or script side. Retail's GASFnCall carries exactly this:
// the result slot, `this`, the environment and the argument window into the environment stack
// (GASFnCall::GASFnCall in GFxValueImpl.obj).
class GASFnCall
{
public:
    GASValue*           pResult;
    GASObjectInterface* pThis;
    GASEnvironment*     pEnv;
    int                 NArgs;
    int                 FirstArgBottomIndex;
    // The name the call was made under, which is what `super` needs and nothing else does. Retail
    // carries it the same way: GASObjectInterface::Invoke's third parameter is `char const* name`
    // and InvokeContext keeps it at +16, where InvokeContext::Setup (2012 0x9f2b40) reads it to find
    // the prototype that declares the function. Null when the callee was reached without a name -
    // a function value off the stack - in which case retail falls back to `this.__proto__` too.
    const GASString*    pFuncName;

    GASFnCall(GASValue* result, GASObjectInterface* self, GASEnvironment* env, int nargs,
              int firstArg, const GASString* name = 0)
        : pResult(result), pThis(self), pEnv(env), NArgs(nargs), FirstArgBottomIndex(firstArg),
          pFuncName(name) {}

    const GASValue& Arg(int n) const;
    int  GetNumArgs() const { return NArgs; }
};

typedef void (*GASCFunctionPtr)(const GASFnCall& fn);

// DISHONORED(port): 2012 GASEnvironment::OperatorNew 0x9e89c0 makes the instance of `new F(...)` by
// calling F's own CreateNewObject rather than by making a plain object, and logs
// "%s::CreateNewObject returned NULL during creation of %s class instance." when it answers null.
// This is what gives a built-in class its own storage: Array's makes a GASArrayObject, so `push`
// and `length` reach the element array, and String's makes a boxed primitive.
typedef GASObject* (*GASNewObjectPtr)(GASStringContext* sc, GASObject* proto);

// GASFunctionObject covers both kinds of callable: a C++ function (the class library) and a
// DefineFunction/DefineFunction2 body (the content). GetObjectType() is the measured 23.
class GASFunctionObject : public GASObject
{
public:
    struct ArgSpec
    {
        int       Register;
        GASString Name;
    };

    GASFunctionObject(GASStringContext* sc, GASObject* proto);
    virtual ~GASFunctionObject();

    virtual GASObjectType GetObjectType() const { return Object_Function; }
    virtual GASFunctionObject* ToFunction() { return this; }
    virtual void Finalize_GC();                                       // 2013 0xde9290 (GASAsFunctionObject)

    void Invoke(const GASFnCall& fn);

    // 2012 0x9e89c0 calls this through the vtable slot at +60. Null makes a plain GASObject, which
    // is what a content class's constructor gets.
    GASObject* CreateNewObject(GASStringContext* sc, GASObject* proto) const;

    // The C++ form.
    GASCFunctionPtr pCFunction;
    GASNewObjectPtr pNewObjectFunc;

    // The script form: a slice of an action buffer plus its declaration environment.
    GASActionBuffer* pBuffer;
    int              StartPC;
    int              Length;
    unsigned char    Version;          // 1 = DefineFunction, 2 = DefineFunction2
    unsigned char    RegisterCount;
    unsigned short   Flags;            // the DefineFunction2 preload/suppress bits
    ArgSpec*         Args;
    unsigned int     NumArgs;
    /** DISHONORED(port, agent FQ): the timeline the declaration was made on, held the way retail
        holds it - as a GFxCharacterHandle (GASAsFunctionObject+56, taken with a reference in the
        constructor 2012 0x9f28f0 / 2013 0x9e90b0, size 0x101) and resolved at every invocation
        (GetEnvironment 2012 0x9f1ab0 / 2013 0x9e82d0, 0x6b: GFxCharacterHandle::ResolveCharacter).
        It was a raw GFxASCharacter* here, which is why agent FJ could not write retail's unloaded
        test over it: the handle's pCharacter is zeroed by the clip's unload (and by its
        destructor), so resolving it never touches freed memory. */
    GFxCharacterHandle* pDeclTarget;
    void            SetDeclTarget(GFxASCharacter* target);
    GFxASCharacter* ResolveDeclTarget(GFxMovieRoot* root) const;
    GASObject*       pOwnerProto;      // the prototype the function was installed on, for `super`

    enum FunctionFlags
    {
        SuppressThis    = 0x02, PreloadThis      = 0x01,
        SuppressArgs    = 0x08, PreloadArgs      = 0x04,
        SuppressSuper   = 0x20, PreloadSuper     = 0x10,
        PreloadRoot     = 0x40, PreloadParent    = 0x80,
        PreloadGlobal   = 0x100
    };
};

// A primitive boxed as an object, so that "abc".charAt(1) and (5).toString() find their methods on
// String.prototype / Number.prototype with the primitive still reachable as `this`. Retail does
// exactly this and names it so: GASEnvironment::PrimitiveToTempObject (2012 0x9e4d50 and 0x9e91a0).
class GASPrimitiveObject : public GASObject
{
public:
    GASPrimitiveObject(GASStringContext* sc, GASObject* proto, const GASValue& prim,
                       GASObjectType type)
        : GASObject(sc, proto), Primitive(prim), Type(type) {}

    virtual GASObjectType GetObjectType() const { return Type; }
    virtual GASValue GetValue() const { return Primitive; }
    virtual void ExecuteForEachChild_GC(int op) const;
    virtual void Finalize_GC();

    GASValue      Primitive;
    GASObjectType Type;
};

// GASSuperObject: `super` inside a method. Retail has 18 functions for it; what the interpreter
// needs is a thing whose GetMemberRaw looks in the *parent* prototype and whose invocation calls
// the parent constructor with the current `this`.
class GASSuperObject : public GASObject
{
public:
    GASSuperObject(GASStringContext* sc, GASObject* proto, GASObjectInterface* self,
                   GASFunctionObject* ctor);

    virtual GASObjectType GetObjectType() const { return Object_Super; }
    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val);
    virtual GASFunctionObject* ToFunction() { return pConstructor; }
    virtual void ExecuteForEachChild_GC(int op) const;               // 2013 0xdd8e40
    virtual void Finalize_GC();                                       // 2013 0xdd8bb0

    GASObjectInterface* pRealThis;
    GASFunctionObject*  pConstructor;
};

// GASRefCountCollector: retail's AS2 garbage collector, GRefCountCollector<323> under the name the
// GASGlobalContext hands out (GASGlobalContext::GetGC 2013 0xdd6800). It is Bacon & Rajan's
// synchronous cycle collection over the reference counts: a Release that leaves a count above
// zero colours the object purple and buffers it as a root (0xda7b50), and Collect (2013 0xda78b0 /
// 2012 0x9ae880) runs MarkInCycle (trial deletion: every edge out of a purple root's subgraph is
// subtracted and the subgraph goes gray), Scan (a gray object with a count left is live - ScanInUse
// restores its subgraph's counts and paints it black - and one with none is white) and
// CollectGarbage (the white ones go to the kill list), then frees the kill list through
// Finalize_GC. The frame-rate policy is GASRefCountCollector::AdvanceFrame (2013 0xdd81a0 / 2012
// 0x9e1a00): collect when the root buffer has outgrown an adaptive bound that SetParams (0xdd6130)
// seeds at 1000, called once at the tail of GFxMovieRoot::Advance (0xe08280, at 0xe088f0).
//
// Two things here are this tree's and are said so in the bodies: (1) a zero count does not free
// the object inside Release the way retail's does - it goes on the kill list and is freed when the
// list next runs, at the tail of the advance, with its count re-read first, so a C++ frame of this
// reconstruction that still names the object through a raw pointer finds it rather than freed
// memory; (2) every new object is put on that same list at birth, because retail's creator holds a
// GPtr that this tree's `new GASObject(...)` sites do not, so an object nothing ever stored dies
// at the end of the frame instead of at the movie's close.
class GASRefCountCollector
{
public:
    // GRefCountBaseGC<323>::OperationGC, the argument of ExecuteForEachChild_GC: the values retail's
    // Collect and functors pass (0xda78b0 passes 1, 2/4 and 3; Release passes 0).
    enum OperationGC
    {
        Op_Release        = 0,
        Op_MarkInCycle    = 1,
        Op_ScanInUse      = 2,
        Op_CollectGarbage = 3,
        Op_Scan           = 4
    };

    // GRefCountCollector<323>::Stats, the two words Collect fills: roots walked, objects freed.
    struct Stats
    {
        unsigned int RootsProcessed;
        unsigned int ObjectsFreed;
    };

    GASRefCountCollector();                                           // 2013 0xddb610
    ~GASRefCountCollector();

    void SetParams(unsigned int framesBetweenCollections, unsigned int maxRootCount); // 2013 0xdd6130
    void AdvanceFrame(unsigned int* frameCnt, unsigned int* lastCollectionFrame);    // 2013 0xdd81a0
    void ForceCollect();                                              // 2013 0xdd82e0
    void ForceEmergencyCollect();                                     // 2013 0xdd8330
    bool Collect(Stats* stats);                                       // 2013 0xda78b0

    // Teardown: frees every object still registered, counts or no counts. Retail reaches the same
    // end through ~GASGlobalContext releasing its last holders and ForceCollect in ~GFxMovieRoot
    // (0xe09150 at +0x2fc); this tree frees the list outright because its holders are not all counted.
    void FreeAll();

    // The object side: registration at construction, the root buffer, the kill list.
    void Register(GASObject* obj);
    void AddRoot(GASObject* obj);
    void AddToKillList(GASObject* obj, bool bCycleGarbage);
    unsigned int ExecuteKillList();
    static void VisitChild(GASObject* child, int op);

    unsigned int GetLiveCount() const { return Count; }
    unsigned int GetCreatedCount() const { return Created; }
    unsigned int GetFreedByCountCount() const { return FreedByCount; }
    unsigned int GetFreedInCycleCount() const { return FreedInCycle; }
    unsigned int GetCollectionCount() const { return Collections; }
    unsigned int GetRootCount() const { return RootCount; }
    unsigned int GetMaxRootCount() const { return MaxRootCount; }

private:
    void Free(GASObject* obj);

    GASObject*   pHead;
    unsigned int Count;
    unsigned int Created;
    unsigned int FreedByCount;
    unsigned int FreedInCycle;
    unsigned int Collections;

    GASObject**  Roots;                 // retail: GArrayPagedBase<GRefCountBaseGC<323>*,10,5> at +20
    unsigned int RootCount;
    unsigned int RootCapacity;
    GASObject*   pKillList;             // retail +28
    bool         bCollecting;           // retail +32 bit 0

    // The AdvanceFrame policy words, in the order retail's constructor zeroes them (+36..+72).
    unsigned int FrameCnt;              // +36
    unsigned int MaxRootCount;          // +40, 1000
    unsigned int PeakRootCount;         // +44
    unsigned int LastRootCount;         // +48
    unsigned int LastCollectedRoots;    // +52
    unsigned int LastPeakRootCount;     // +56
    unsigned int TotalFramesCount;      // +60
    unsigned int LastCollectionFrameNum;// +64
    unsigned int PresetMaxRootCount;    // +68, 1000
    unsigned int MaxFramesBetweenCollections; // +72, 0
};

#pragma pack(pop)
#endif // INC_GFX3_GFXAS2OBJECT_H
