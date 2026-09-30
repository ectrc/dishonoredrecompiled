// Scaleform GFx 3.3.89 - the AS2 execution environment, the global context and the bytecode
// interpreter. Package BC.
//
// GASActionBuffer::Execute (2012 0x9ea900, 13,931 bytes, decompiled to
// build/agentBC/dec/GASActionBuffer_Execute_9ea900.c) is the machine. Its two switch statements are
// the definitive answer to "what is the opcode set", and the enum below is transcribed from them
// case label by case label: 80 single-byte opcodes in the first switch and 18 length-prefixed ones
// in the second, 98 in all. Nothing was taken from an AVM1 reference; the reference agrees, which is
// the cross-check rather than the source.
//
// The opcodes retail does NOT have a case for, and therefore neither do we: 0x08 ToggleQuality and
// 0x09 StopSounds (they fall through the default and are no-ops), and 0x16, 0x1A, 0x1B, 0x1E, 0x1F,
// 0x2D-0x2F, 0x38, 0x39, 0x56-0x5F, which are not opcodes at all.
#ifndef INC_GFX3_GFXAS2RUNTIME_H
#define INC_GFX3_GFXAS2RUNTIME_H

#include "GFxAS2Object.h"

#pragma pack(push, 8)

class GFxMovieRoot;
class GFxSprite;
class GASGlobalContext;

// Agent DG: the per-buffer opcode budget (GFxAS2Interp.cpp).
extern unsigned int GFxAS2OpBudget;
extern unsigned int GFxAS2OpsThisBuffer;
extern unsigned int GFxAS2OpTraceFrom;
extern unsigned int GFxAS2OpTraceCount;
extern int GFxAS2OpTraceLen;
extern int GFxAS2OpTraceLo;
extern int GFxAS2OpTraceHi;
extern char GFxAS2WatchMember[64];
bool GFxAS2WatchMatches(const char* name);
extern int  GFxAS2WatchCount;
// Agent EG: how many more unresolved method calls to report in full, with the receiver's whole
// resolution chain, and how many more member writes to report with the string manager that interned
// the name. Both are -gfxuitweendiag; the second is filtered by GFxAS2WatchMatches.
extern int GFxAS2NotAFunctionDiag;
extern int GFxAS2MemberWriteDiag;

// Runs a DefineFunction/DefineFunction2 body: binds `this`, the arguments, the preload registers and
// the local frame, then re-enters GASActionBuffer::Execute at the function's own pc. Retail does this
// inside GASAsFunctionObject::Invoke; it is a free function here so GFxAS2Object.cpp does not have to
// know the interpreter.
void GFxAS2InvokeScriptFunction(GASFunctionObject* fn, const GASFnCall& call);

// ActionGetTimer (0x34): milliseconds since the runtime started.
unsigned int GFxAS2GetTimerMs();

// The 98 opcodes GASActionBuffer::Execute (2012 0x9ea900) has a case for, so a run can report its
// coverage against the measured set. GFxAS2Interp.cpp holds the table.
unsigned int         GFxAS2GetRetailOpcodeCount();
const unsigned char* GFxAS2GetRetailOpcodes();
const char*          GFxAS2GetOpcodeName(unsigned char op);

enum GASAction
{
    // --- single byte, no payload (GASActionBuffer::Execute's first switch) ---
    GASop_End                    = 0x00,
    GASop_NextFrame              = 0x04,
    GASop_PrevFrame              = 0x05,
    GASop_Play                   = 0x06,
    GASop_Stop                   = 0x07,
    GASop_Add                    = 0x0A,
    GASop_Subtract               = 0x0B,
    GASop_Multiply               = 0x0C,
    GASop_Divide                 = 0x0D,
    GASop_Equal                  = 0x0E,
    GASop_LessThan               = 0x0F,
    GASop_LogicalAnd             = 0x10,
    GASop_LogicalOr              = 0x11,
    GASop_LogicalNot             = 0x12,
    GASop_StringEqual            = 0x13,
    GASop_StringLength           = 0x14,
    GASop_SubString              = 0x15,
    GASop_Pop                    = 0x17,
    GASop_ToInteger              = 0x18,
    GASop_GetVariable            = 0x1C,
    GASop_SetVariable            = 0x1D,
    GASop_SetTargetExpression    = 0x20,
    GASop_StringConcat           = 0x21,
    GASop_GetProperty            = 0x22,
    GASop_SetProperty            = 0x23,
    GASop_DuplicateClip          = 0x24,
    GASop_RemoveClip             = 0x25,
    GASop_Trace                  = 0x26,
    GASop_StartDragMovie         = 0x27,
    GASop_StopDragMovie          = 0x28,
    GASop_StringCompare          = 0x29,
    GASop_Throw                  = 0x2A,
    GASop_CastOp                 = 0x2B,
    GASop_ImplementsOp           = 0x2C,
    GASop_Random                 = 0x30,
    GASop_MBLength               = 0x31,
    GASop_Ord                    = 0x32,
    GASop_Chr                    = 0x33,
    GASop_GetTimer               = 0x34,
    GASop_MBSubString            = 0x35,
    GASop_MBOrd                  = 0x36,
    GASop_MBChr                  = 0x37,
    GASop_Delete                 = 0x3A,
    GASop_Delete2                = 0x3B,
    GASop_DefineLocal            = 0x3C,
    GASop_CallFunction           = 0x3D,
    GASop_Return                 = 0x3E,
    GASop_Modulo                 = 0x3F,
    GASop_New                    = 0x40,
    GASop_DeclareLocal           = 0x41,
    GASop_InitArray              = 0x42,
    GASop_InitObject             = 0x43,
    GASop_TypeOf                 = 0x44,
    GASop_TargetPath             = 0x45,
    GASop_Enumerate              = 0x46,
    GASop_NewAdd                 = 0x47,
    GASop_NewLessThan            = 0x48,
    GASop_NewEquals              = 0x49,
    GASop_ToNumber               = 0x4A,
    GASop_ToString               = 0x4B,
    GASop_Dup                    = 0x4C,
    GASop_Swap                   = 0x4D,
    GASop_GetMember              = 0x4E,
    GASop_SetMember              = 0x4F,
    GASop_Increment              = 0x50,
    GASop_Decrement              = 0x51,
    GASop_CallMethod             = 0x52,
    GASop_NewMethod              = 0x53,
    GASop_InstanceOf             = 0x54,
    GASop_Enumerate2             = 0x55,
    GASop_BitwiseAnd             = 0x60,
    GASop_BitwiseOr              = 0x61,
    GASop_BitwiseXor             = 0x62,
    GASop_ShiftLeft              = 0x63,
    GASop_ShiftRight             = 0x64,
    GASop_ShiftRightUnsigned     = 0x65,
    GASop_StrictEqual            = 0x66,
    GASop_Greater                = 0x67,
    GASop_StringGreater          = 0x68,
    GASop_Extends                = 0x69,

    // --- length-prefixed (the second switch) ---
    GASop_GotoFrame              = 0x81,
    GASop_GetUrl                 = 0x83,
    GASop_StoreRegister          = 0x87,
    GASop_ConstantPool           = 0x88,
    GASop_WaitForFrame           = 0x8A,
    GASop_SetTarget              = 0x8B,
    GASop_GotoLabel              = 0x8C,
    GASop_WaitForFrameExpression = 0x8D,
    GASop_DefineFunction2        = 0x8E,
    GASop_Try                    = 0x8F,
    GASop_With                   = 0x94,
    GASop_Push                   = 0x96,
    GASop_BranchAlways           = 0x99,
    GASop_GetUrl2                = 0x9A,
    GASop_DefineFunction         = 0x9B,
    GASop_BranchIfTrue           = 0x9D,
    GASop_CallFrame              = 0x9E,
    GASop_GotoExpression         = 0x9F
};

// The ActionPush payload types, from the inner switch at the end of GASActionBuffer::Execute
// (cases 0,1,2,3,4,5,6,7,8,9 of the switch on the type byte).
enum GASPushType
{
    GASpush_String      = 0,
    GASpush_Float       = 1,
    GASpush_Null        = 2,
    GASpush_Undefined   = 3,
    GASpush_Register    = 4,
    GASpush_Boolean     = 5,
    GASpush_Double      = 6,
    GASpush_Integer     = 7,
    GASpush_Constant8   = 8,
    GASpush_Constant16  = 9
};

// GASActionBuffer wraps one DoAction/DoInitAction/DefineFunction body plus the constant pool that
// ActionConstantPool (0x88) fills. ProcessDeclDict (2013 0x9dad40) is the pool reader.
//
// DISHONORED(port, 2013 0x9d8140 / 0x9da9f0 / 0x9daae0): retail splits this in two. The bytecode is
// a refcounted GASActionBufferData owned by the tag in the movie's shared GFxMovieDataDef; the
// constant pool lives in a GASActionBuffer that GASDoAction::Execute builds fresh, out of the
// executing movie root's own GASStringContext, for every execution. Interned GASStrings belong to
// one GASStringManager and compare by node pointer, so a pool shared by two movie roots hands the
// second one names the first one interned and every member lookup out of that buffer misses. This
// tree keeps the bytecode and the pool in one object, so the pool is kept once PER STRING MANAGER
// instead, with retail's once-only guard (this+28, -1 until the first ActionConstantPool) per pool.
class GASActionBuffer
{
public:
    GASActionBuffer();                                                // 2012 0x9e19a0
    ~GASActionBuffer();

    void SetBytes(const unsigned char* bytes, unsigned int length);

    const unsigned char* GetBufferPtr(int pc) const { return Bytes + pc; }
    int  GetLength() const { return (int)Length; }
    bool IsNull() const { return Length == 0; }

    void ProcessDeclDict(GASStringContext* sc, unsigned int start, unsigned int end); // 0x9dad40
    const GASString& GetConstant(GASStringContext* sc, unsigned int i) const;
    unsigned int     GetConstantCount(GASStringContext* sc) const;

    // The interpreter. `retval` is written by ActionReturn; `execType` distinguishes a normal
    // buffer from a function body, which is the last parameter of the retail signature
    // (GASActionBuffer::Execute(GASEnvironment*, int, int, GASValue*, ..., ExecuteType)).
    enum ExecuteType { Exec_Normal = 0, Exec_Function = 1, Exec_Function2 = 2 };

    void Execute(GASEnvironment* env);                                // 2012 0x9f0a60
    void Execute(GASEnvironment* env, int startPC, int execBytes, GASValue* retval,
                 ExecuteType execType);                               // 2012 0x9ea900

    // Counted for the harness: how many opcodes this buffer executed and how many of them the
    // machine has no case for yet.
    static unsigned int OpsExecuted;
    static unsigned int OpsUnimplemented;
    static unsigned int OpCounts[256];
    static void ResetCounters();

private:
    // One constant pool per string manager, named by that manager's serial. ProcessedAt is retail's
    // this+28: the pc of the ActionConstantPool that filled this pool, -1 while it is empty.
    struct DeclDict
    {
        unsigned int Serial;
        int          ProcessedAt;
        GASString*   Strings;
        unsigned int Count;
    };

    DeclDict* FindDict(GASStringContext* sc) const;
    DeclDict* OpenDict(GASStringContext* sc);

    unsigned char* Bytes;
    unsigned int   Length;
    DeclDict*      Dicts;
    unsigned int   DictCount;
};

// A `with` scope. Retail's GASWithStackEntry is exactly (object, end pc) - the two-argument
// constructor at 2012 0x9e0d40 says so.
class GASWithStackEntry
{
public:
    GASObject* pObject;
    int        BlockEndPC;

    GASWithStackEntry() : pObject(0), BlockEndPC(0) {}
    GASWithStackEntry(GASObject* obj, int endPC) : pObject(obj), BlockEndPC(endPC) {}
};

// One activation record. `super` and `arguments` live here because DefineFunction2's preload bits
// put them in registers, and locals are a flat name/value list because that is what
// GASEnvironment::FindLocal (2012 0x9e6be0) walks.
class GASLocalFrame
{
public:
    struct Local { GASString Name; GASValue Value; };

    GASLocalFrame() : Locals(0), Size(0), Capacity(0) {}
    ~GASLocalFrame();

    void       Clear();
    void       ReleaseAll();
    GASValue*  Find(const GASString& name);
    void       Add(const GASString& name, const GASValue& v);

    Local*       Locals;
    unsigned int Size;
    unsigned int Capacity;
};

// GASEnvironment. The members are the ones every decompiled body touches: the value stack
// (GASPagedStack<GASValue,32> at +8 in retail), the local-frame stack, the four local registers and
// the DefineFunction2 register window, the with-stack, the target character and the global context.
class GASEnvironment
{
public:
    GASEnvironment();                                                 // 2012 0x9fe6c0
    ~GASEnvironment();                                                // 2012 0x9fe750

    void Init(GASGlobalContext* gc, GFxASCharacter* target);

    // Drops every value the environment is still holding. The movie root calls this before it
    // collects the AS2 graph, because the value stack and the local frames are full of references
    // into it and GASValue::DropRefs would otherwise touch objects the collector has already freed.
    void ReleaseAll();

    GASGlobalContext* GetGC() const { return pGC; }
    GASStringContext* GetSC() { return &SC; }
    GFxMovieRoot*     GetMovieRoot() const;                           // 2012 0x9df670
    GFxASCharacter*   GetTarget() const { return pTarget; }
    void              SetTarget(GFxASCharacter* t) { pTarget = t; }
    void              SetInvalidTarget(GFxASCharacter* orig) { pTarget = orig; bInvalidTarget = true; }

    GASString CreateString(const char* s) { return SC.CreateString(s); }
    GASString CreateString(const char* s, unsigned int n) { return SC.CreateString(s, n); }
    const GASString& GetBuiltin(GASBuiltinString b) const { return SC.GetBuiltin(b); }

    // --- the value stack ---
    void      Push(const GASValue& v);
    void      Drop(int n);
    GASValue& Top(int off = 0);
    const GASValue& TopConst(int off = 0) const;
    GASValue  Pop();
    int       GetTopIndex() const { return (int)StackSize - 1; }
    GASValue& Bottom(int index);
    unsigned int GetStackSize() const { return StackSize; }

    // --- local frames ---
    bool CreateNewLocalFrame();                                       // 2012 0x9e7880
    void PopLocalFrame();
    GASLocalFrame* GetTopLocalFrame();                                // 2012 0x9e0370
    GASValue* FindLocal(const GASString& name);                       // 2012 0x9e6be0
    void AddLocal(const GASString& name, const GASValue& v);          // 2012 0x9e8910
    void DeclareLocal(const GASString& name);                         // 2012 0x9e8970
    bool SetLocal(const GASString& name, const GASValue& v);          // 2012 0x9e9150
    int  GetLocalFrameDepth() const { return (int)FrameCount; }

    // --- DefineFunction2 registers ---
    void      AddLocalRegisters(unsigned int count);                  // 2012 0x9f1de0
    void      DropLocalRegisters(unsigned int count);                 // 2012 0x9f1e20
    GASValue* LocalRegisterPtr(unsigned int i);                       // 2012 0x9e0e30
    GASValue* GlobalRegisterPtr(unsigned int i);

    // --- variables and paths ---
    bool GetVariable(const GASString& path, GASValue* out,
                     const GASWithStackEntry* withStack, unsigned int withCount) const; // 0x9e9d60
    bool SetVariable(const GASString& path, const GASValue& v,
                     const GASWithStackEntry* withStack, unsigned int withCount);       // 0x9ea060
    bool GetVariableRaw(const GASString& name, GASValue* out,
                        const GASWithStackEntry* withStack, unsigned int withCount) const; // 0x9e9270
    bool SetVariableRaw(const GASString& name, const GASValue& v,
                        const GASWithStackEntry* withStack, unsigned int withCount);       // 0x9e7790
    GFxASCharacter* FindTarget(const GASString& path) const;          // 2012 0x9e0e70
    GFxASCharacter* FindTargetByValue(const GASValue& v);             // 2012 0x9e4c50
    static bool ParsePath(GASStringContext* sc, const GASString& path, GASString* outPath,
                          GASString* outVar);                         // 2012 0x9e0390
    static bool IsPath(const GASString& s);                           // 2012 0x9df680

    bool GetMember(GASObjectInterface* obj, const GASString& name, GASValue* out); // 0x9e5a10
    GASObject* OperatorNew(GASFunctionObject* ctor, int nargs, int firstArgBottom); // 0x9e89c0
    GASObject* PrimitiveToTempObject(const GASValue& v);              // 2012 0x9e91a0

    void LogScriptError(const char* fmt, ...) const;                  // 2012 0x9e4900
    void LogScriptWarning(const char* fmt, ...) const;                // 2012 0x9e4a30

    // ActionThrow (0x2A) / ActionTry (0x8F). Retail unwinds through GASEnvironment::CheckExceptions
    // (2012 0x9ee1b0) and a stack of TryDescr records; this carries the in-flight exception on the
    // environment and lets each Execute frame test for it, which handles a try/catch inside one
    // buffer and propagates out of a function call. What it does not yet do is resume a `finally`
    // after a return, which agentBC.md lists as remaining.
    bool     bThrowing;
    GASValue ThrowValue;

    // The value Top() / Bottom() hand back on an underflow. A member and not a function static, so
    // that a reference content wrote into it dies with the movie rather than at process exit.
    GASValue Dummy;

    // Retail's own recursion guard: GASValue::ToStringImpl (2012 0x9cbe50) keeps a 16-bit counter on
    // the environment and returns a fixed string once it passes 255, because an object whose
    // toString() returns itself would otherwise recurse for ever. The same counter guards
    // ToPrimitive, which is the other half of the same loop.
    unsigned int ToStringDepth;

    GASStringContext SC;

private:
    GASGlobalContext* pGC;
    GFxASCharacter*   pTarget;
    bool              bInvalidTarget;

    GASValue*     Stack;
    unsigned int  StackSize;
    unsigned int  StackCapacity;

    GASLocalFrame* Frames;
    unsigned int   FrameCount;
    unsigned int   FrameCapacity;

    GASValue*     Registers;        // the DefineFunction2 window, grown by AddLocalRegisters
    unsigned int  RegisterCount;
    GASValue      GlobalRegisters[4];
};

// GASGlobalContext owns everything that is one-per-movie: the string manager, the builtin string
// table, _global, the prototypes of the class library and the class registry that
// Object.registerClass fills. Retail's is 55 functions in GFxAction.obj.
class GASGlobalContext
{
public:
    GASGlobalContext(GFxMovieRoot* root, unsigned int swfVersion);
    ~GASGlobalContext();

    GASStringManager* GetStringManager() { return &Strings; }
    GASStringContext* GetSC() { return &SC; }
    GFxMovieRoot*     GetMovieRoot() const { return pMovieRoot; }
    GASObjectCollector* GetCollector() { return &Collector; }

    const GASString& GetBuiltin(GASBuiltinString which) const { return Builtins[which]; }

    GASObject* GetGlobalObject() const { return pGlobal; }

    /** DISHONORED(port): retail's GASGlobalContext+684, written by `_global.gfxExtensions = <bool>`
        (GASGlobalObject::SetMember 2013 0x9d72d0, which writes 1 for true and 2 for false and
        publishes `_global.gfxVersion` = "3.3.89" while it is on). It is the gate on Scaleform's own
        additions to the AS2 surface - the six 3D display properties, `noInvisibleAdvance` and
        `continueAnimation` - so a movie that never sets it behaves exactly like Flash. */
    bool AreGFxExtensionsEnabled() const { return bGFxExtensions; }
    void SetGFxExtensionsEnabled(bool bEnabled) { bGFxExtensions = bEnabled; }

    // The class library. Each entry is the prototype object every instance of that class chains to.
    enum ProtoId
    {
        Proto_Object = 0, Proto_Function, Proto_Array, Proto_String, Proto_Number, Proto_Boolean,
        Proto_MovieClip, Proto_Error, Proto_COUNT
    };
    GASObject* GetPrototype(ProtoId id) const { return Prototypes[id]; }
    GASFunctionObject* GetConstructor(ProtoId id) const { return Constructors[id]; }

    // Object.registerClass's table: symbol name -> constructor. The main menu's __Packages code is
    // 45 registrations on its own (agentBB.md 4).
    void RegisterClass(const GASString& symbol, GASFunctionObject* ctor);
    GASFunctionObject* FindRegisteredClass(const GASString& symbol) const;
    unsigned int GetRegisteredClassCount() const { return ClassCount; }

    GASObject*         NewObject();
    GASArrayObject*    NewArray();
    GASFunctionObject* NewCFunction(GASCFunctionPtr fn);
    GASFunctionObject* NewFunction();

    // Installs Object, Function, Array, String, Number, Boolean, Math, MovieClip, Error and the
    // free functions (ASSetPropFlags, trace, Number, String, Boolean, parseInt, parseFloat,
    // isNaN, setInterval/clearInterval as recognised no-ops). GFxAS2Lib.cpp.
    void InitStandardLibrary();

    unsigned int GetSWFVersion() const { return SC.Version; }

private:
    GFxMovieRoot*      pMovieRoot;
    GASStringManager   Strings;
    GASStringContext   SC;
    GASObjectCollector Collector;
    GASString          Builtins[GASbuiltin_COUNT];
    GASObject*         pGlobal;
    bool               bGFxExtensions;
    GASObject*         Prototypes[Proto_COUNT];
    GASFunctionObject* Constructors[Proto_COUNT];

    struct ClassEntry { GASString Symbol; GASFunctionObject* pCtor; };
    ClassEntry*  Classes;
    unsigned int ClassCount;
    unsigned int ClassCapacity;

    void InitBuiltinStrings();
};

#pragma pack(pop)
#endif // INC_GFX3_GFXAS2RUNTIME_H
