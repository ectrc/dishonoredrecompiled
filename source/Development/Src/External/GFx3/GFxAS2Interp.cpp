// Scaleform GFx 3.3.89 - GASActionBuffer and the AS2 bytecode interpreter. Package BC.
//
// This file is a port of GASActionBuffer::Execute (2012 0x9ea900, 13,931 bytes; the decompile is
// build/agentBC/dec/GASActionBuffer_Execute_9ea900.c). Its structure is retail's: read one opcode,
// take the short path when the high bit is clear and the length-prefixed path when it is set, and
// dispatch in one switch. Where retail factors a case out into GASExecutionContext - the nine
// *OpCode methods, 2012 0x9e58f0 / 0x9e5ef0 / 0x9e6120 / 0x9e6290 / 0x9e64a0 / 0x9e66a0 / 0x9e8060 /
// 0x9e8490 / 0x9ea610 / 0x9ea7a0 - so does this, as a static function named after it.
//
// The counters at the bottom are not decoration: the package's acceptance is a table of implemented
// against remaining opcodes, and OpCounts is where that table comes from - measured on a real run of
// a real cooked asset rather than read off this source.
// DISHONORED(port): see GFxAS2.h.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned int GASActionBuffer::OpsExecuted = 0;

// The budget and its counter. The counter is reset by the outermost Execute, so a deep but finite
// call tree spends one budget rather than one per frame.
unsigned int GFxAS2OpBudget = 1000000;
unsigned int GFxAS2OpsThisBuffer = 0;
unsigned int GFxAS2OpTraceFrom = 0;
unsigned int GFxAS2OpTraceCount = 200;
// DISHONORED(bringup): a trace window pinned to a BUFFER and a pc range rather than to the
// opcode counter, which shifts between runs because it is reset per advance.
int GFxAS2OpTraceLen = 0;
int GFxAS2OpTraceLo = 0;
int GFxAS2OpTraceHi = 0;
char GFxAS2WatchMember[64] = "";

// DISHONORED(bringup): a trailing '*' makes the watch a prefix, which is the only way to see a
// member whose name the content builds - `_elementContainer["btn" + idx]` is one.
bool GFxAS2WatchMatches(const char* name)
{
    if (GFxAS2WatchMember[0] == 0 || name == 0)
        return false;
    size_t n = strlen(GFxAS2WatchMember);
    if (n > 0 && GFxAS2WatchMember[n - 1] == '*')
        return strncmp(name, GFxAS2WatchMember, n - 1) == 0;
    return strcmp(name, GFxAS2WatchMember) == 0;
}
int  GFxAS2WatchCount = 0;
unsigned int GASActionBuffer::OpsUnimplemented = 0;
unsigned int GASActionBuffer::OpCounts[256] = { 0 };

void GASActionBuffer::ResetCounters()
{
    OpsExecuted = 0;
    OpsUnimplemented = 0;
    memset(OpCounts, 0, sizeof(OpCounts));
}

// The 98 opcodes retail has a case for, so a run can report coverage against the real set rather
// than against a guess. Transcribed from the two switch statements of 0x9ea900.
static const unsigned char GFxAS2RetailOpcodes[] =
{
    0x00, 0x04, 0x05, 0x06, 0x07, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18, 0x1C, 0x1D,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x81, 0x83, 0x87, 0x88, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x94, 0x96, 0x99, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F
};

unsigned int GFxAS2GetRetailOpcodeCount()
{
    return (unsigned int)sizeof(GFxAS2RetailOpcodes);
}

const unsigned char* GFxAS2GetRetailOpcodes()
{
    return GFxAS2RetailOpcodes;
}

// ---------------------------------------------------------------------------------------------
// GASActionBuffer

GASActionBuffer::GASActionBuffer()                                    // 2012 0x9e19a0
    : Bytes(0), Length(0), Dict(0), DictCount(0) {}

GASActionBuffer::~GASActionBuffer()
{
    free(Bytes);
    delete[] Dict;
}

void GASActionBuffer::SetBytes(const unsigned char* bytes, unsigned int length)
{
    free(Bytes);
    Bytes = 0;
    Length = length;
    if (length)
    {
        Bytes = (unsigned char*)malloc(length);
        memcpy(Bytes, bytes, length);
    }
}

void GASActionBuffer::ProcessDeclDict(GASStringContext* sc, unsigned int start, unsigned int end)
{                                                                     // 2012 0x9e4590
    // ActionConstantPool's body is u16 count then that many NUL-terminated strings. Every string
    // goes through the interner, which is what makes the later identity comparisons in
    // GASObject::GetMemberRaw work at all.
    if (start + 3 > Length)
        return;
    unsigned int pc = start + 3;
    if (pc + 2 > Length)
        return;
    unsigned int count = Bytes[pc] | ((unsigned int)Bytes[pc + 1] << 8);
    pc += 2;
    delete[] Dict;
    Dict = count ? new GASString[count] : 0;
    DictCount = 0;
    for (unsigned int i = 0; i < count && pc < end && pc < Length; ++i)
    {
        const char* s = (const char*)(Bytes + pc);
        unsigned int len = 0;
        while (pc + len < Length && Bytes[pc + len] != 0)
            ++len;
        Dict[i] = sc->CreateString(s, len);
        ++DictCount;
        pc += len + 1;
    }
}

const GASString& GASActionBuffer::GetConstant(unsigned int i) const
{
    static const GASString empty;
    return i < DictCount ? Dict[i] : empty;
}

void GASActionBuffer::Execute(GASEnvironment* env)                     // 2012 0x9f0a60
{
    Execute(env, 0, (int)Length, 0, Exec_Normal);
}

// ---------------------------------------------------------------------------------------------
// The factored-out cases, named after the GASExecutionContext methods they come from.

static void GFxAS2ExtendsOpCode(GASEnvironment* env)                  // 2012 0x9e64a0
{
    // ActionExtends (0x69). DISHONORED(port): 2012 0x9e64a0. The stack is [subclass][superclass]
    // with the SUPERCLASS on top - the decompile reads Top(0) for the prototype it copies and the
    // constructor it records, and Top(1) for the function whose prototype it replaces. AS2
    // inheritance is: sub.prototype = { __proto__ : super.prototype, __constructor__ : super }.
    // This is the opcode every one of the 669 __Packages registrations in the cook ends up in.
    GASValue superVal = env->Pop();
    GASValue subVal = env->Pop();
    GASFunctionObject* sub = subVal.GetFunction();
    GASFunctionObject* super = superVal.GetFunction();
    if (sub == 0 || super == 0)
    {
        env->LogScriptError("ActionExtends: operand is not a function");
        return;
    }
    GASStringContext* sc = env->GetSC();
    GASValue superProtoVal;
    GASObject* superProto = 0;
    if (super->GetMemberRaw(sc, env->GetBuiltin(GASbuiltin_prototype), &superProtoVal))
        superProto = superProtoVal.ToObject(env);

    GASObject* newProto = new GASObject(sc, superProto);
    GASValue ctorVal;
    ctorVal.SetAsFunction(super);
    newProto->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_constructorUS), ctorVal,
                           GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    GASValue protoVal;
    protoVal.SetAsObject(newProto);
    sub->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_prototype), protoVal, GASPropFlags());
}

static void GFxAS2ImplementsOpCode(GASEnvironment* env)               // 2012 0x9e6290
{
    // ActionImplementsOp (0x2C): [interfaceN..interface1][count][constructor]. The interface list is
    // recorded on the constructor so `instanceof` against an interface can answer later.
    GASValue ctorVal = env->Pop();
    int count = env->Pop().ToInt32(env);
    GASFunctionObject* ctor = ctorVal.GetFunction();
    GASArrayObject* list = env->GetGC()->NewArray();
    for (int i = 0; i < count; ++i)
        list->PushBack(env->Pop());
    if (ctor)
    {
        GASValue v;
        v.SetAsObject(list);
        ctor->SetConstMemberRaw(env->GetSC(), "__interfaces__", v,
                                GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }
}

static void GFxAS2CastObjectOpCode(GASEnvironment* env)               // 2012 0x9e6120
{
    // ActionCastOp (0x2B): [constructor][object] -> object if it is an instance, else null.
    GASValue objVal = env->Pop();
    GASValue ctorVal = env->Pop();
    GASFunctionObject* ctor = ctorVal.GetFunction();
    GASObject* proto = 0;
    if (ctor)
    {
        GASValue p;
        if (ctor->GetMemberRaw(env->GetSC(), env->GetBuiltin(GASbuiltin_prototype), &p))
            proto = p.ToObject(env);
    }
    GASObjectInterface* oi = objVal.ToObjectInterface(env);
    if (oi && proto && oi->InstanceOf(env, proto, true))
        env->Push(objVal);
    else
    {
        GASValue nul;
        nul.SetNull();
        env->Push(nul);
    }
}

static void GFxAS2InstanceOfOpCode(GASEnvironment* env)               // 2012 0x9e66a0
{
    GASValue ctorVal = env->Pop();
    GASValue objVal = env->Pop();
    GASFunctionObject* ctor = ctorVal.GetFunction();
    GASObject* proto = 0;
    if (ctor)
    {
        GASValue p;
        if (ctor->GetMemberRaw(env->GetSC(), env->GetBuiltin(GASbuiltin_prototype), &p))
            proto = p.ToObject(env);
    }
    GASObjectInterface* oi = objVal.ToObjectInterface(env);
    env->Push(GASValue(oi != 0 && proto != 0 && oi->InstanceOf(env, proto, true)));
}

// ActionEnumerate / ActionEnumerate2 (0x46 / 0x55), 2012 0x9ea7a0. Pushes null then every
// enumerable member name, which is what `for (var k in o)` consumes.
class GFxAS2EnumVisitor : public GASObjectInterface::MemberVisitor
{
public:
    GFxAS2EnumVisitor(GASEnvironment* e) : pEnv(e), Count(0) {}
    virtual void Visit(const GASString& name, const GASValue& val, unsigned char flags)
    {
        GASValue v;
        v.SetString(name);
        pEnv->Push(v);
        ++Count;
    }
    GASEnvironment* pEnv;
    unsigned int    Count;
};

static void GFxAS2EnumerateOpCode(GASEnvironment* env, bool byValue)   // 2012 0x9ea7a0
{
    GASValue src = env->Pop();
    GASValue nul;
    nul.SetNull();
    env->Push(nul);
    GASObjectInterface* oi = byValue ? src.ToObjectInterface(env)
                                     : env->FindTarget(src.GetString());
    if (!byValue && oi == 0)
        oi = src.ToObjectInterface(env);
    if (oi == 0)
        return;
    GFxAS2EnumVisitor v(env);
    oi->VisitMembers(env->GetSC(), &v, GASObjectInterface::VisitMember_Prototype, oi);
}

static void GFxAS2SetTargetOpCode(GASEnvironment* env)                // 2012 0x9ea610
{
    GASValue v = env->Pop();
    GFxASCharacter* t = env->FindTargetByValue(v);
    if (t)
        env->SetTarget(t);
}

// DefineFunction (0x9B) and DefineFunction2 (0x8E). Retail's Function1OpCode (2012 0x9e8060) and
// Function2OpCode (0x9e8490); the payload layouts below are theirs.
static const unsigned char* GFxAS2ReadString(const unsigned char* p, GASStringContext* sc,
                                             GASString* out)
{
    const char* s = (const char*)p;
    unsigned int n = (unsigned int)strlen(s);
    *out = sc->CreateString(s, n);
    return p + n + 1;
}

static unsigned int GFxAS2ReadU16(const unsigned char* p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static int GFxAS2Function1OpCode(GASActionBuffer* buf, GASEnvironment* env, int pc, int nextPC,
                                 GFxASCharacter* target)
{                                                                     // 2012 0x9e8060
    GASStringContext* sc = env->GetSC();
    const unsigned char* p = buf->GetBufferPtr(pc + 3);
    GASString name;
    p = GFxAS2ReadString(p, sc, &name);
    unsigned int nparams = GFxAS2ReadU16(p);
    p += 2;

    GASFunctionObject* fn = env->GetGC()->NewFunction();
    fn->Version = 1;
    fn->NumArgs = nparams;
    fn->Args = nparams ? new GASFunctionObject::ArgSpec[nparams] : 0;
    for (unsigned int i = 0; i < nparams; ++i)
    {
        fn->Args[i].Register = 0;
        p = GFxAS2ReadString(p, sc, &fn->Args[i].Name);
    }
    unsigned int codeSize = GFxAS2ReadU16(p);
    p += 2;
    fn->pBuffer = buf;
    fn->StartPC = nextPC;
    fn->Length = (int)codeSize;
    fn->pDeclTarget = target;

    // Every function gets its own prototype object with a __constructor__ back-pointer, which is
    // what makes `new f()` and `f.prototype.x = ...` work on a plain DefineFunction.
    GASObject* proto = new GASObject(sc, env->GetGC()->GetPrototype(GASGlobalContext::Proto_Object));
    GASValue ctorVal;
    ctorVal.SetAsFunction(fn);
    proto->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_constructorUS), ctorVal,
                        GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    GASValue protoVal;
    protoVal.SetAsObject(proto);
    fn->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_prototype), protoVal, GASPropFlags());

    GASValue fnVal;
    fnVal.SetAsFunction(fn);
    if (name.IsEmpty())
        env->Push(fnVal);
    else
        env->SetVariableRaw(name, fnVal, 0, 0);

    return nextPC + (int)codeSize;
}

static int GFxAS2Function2OpCode(GASActionBuffer* buf, GASEnvironment* env, int pc, int nextPC,
                                 GFxASCharacter* target)
{                                                                     // 2012 0x9e8490
    GASStringContext* sc = env->GetSC();
    const unsigned char* p = buf->GetBufferPtr(pc + 3);
    GASString name;
    p = GFxAS2ReadString(p, sc, &name);
    unsigned int nparams = GFxAS2ReadU16(p);
    p += 2;
    unsigned char regCount = *p++;
    unsigned int flags = GFxAS2ReadU16(p);
    p += 2;

    GASFunctionObject* fn = env->GetGC()->NewFunction();
    fn->Version = 2;
    fn->RegisterCount = regCount;
    fn->Flags = (unsigned short)flags;
    fn->NumArgs = nparams;
    fn->Args = nparams ? new GASFunctionObject::ArgSpec[nparams] : 0;
    for (unsigned int i = 0; i < nparams; ++i)
    {
        fn->Args[i].Register = *p++;
        p = GFxAS2ReadString(p, sc, &fn->Args[i].Name);
    }
    unsigned int codeSize = GFxAS2ReadU16(p);
    p += 2;
    fn->pBuffer = buf;
    fn->StartPC = nextPC;
    fn->Length = (int)codeSize;
    fn->pDeclTarget = target;

    GASObject* proto = new GASObject(sc, env->GetGC()->GetPrototype(GASGlobalContext::Proto_Object));
    GASValue ctorVal;
    ctorVal.SetAsFunction(fn);
    proto->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_constructorUS), ctorVal,
                        GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    GASValue protoVal;
    protoVal.SetAsObject(proto);
    fn->SetMemberRaw(sc, env->GetBuiltin(GASbuiltin_prototype), protoVal, GASPropFlags());

    GASValue fnVal;
    fnVal.SetAsFunction(fn);
    if (name.IsEmpty())
        env->Push(fnVal);
    else
        env->SetVariableRaw(name, fnVal, 0, 0);

    return nextPC + (int)codeSize;
}

void GFxAS2InvokeScriptFunction(GASFunctionObject* fn, const GASFnCall& call)
{
    GASEnvironment* env = call.pEnv;
    GASStringContext* sc = env->GetSC();
    GFxASCharacter* savedTarget = env->GetTarget();
    if (call.pFuncName != 0 && GFxAS2WatchMatches(call.pFuncName->ToCStr()) && GFxAS2WatchCount > 0)
    {
        --GFxAS2WatchCount;
        GFxLogf("DISHONORED(bringup): watch INVOKE '%s' buffer %p pc %d len %d args %d depth %d op %u",
                call.pFuncName->ToCStr(), (void*)fn->pBuffer, fn->StartPC, fn->Length, call.NArgs,
                env->GetLocalFrameDepth(), GFxAS2OpsThisBuffer);
    }
    if (fn->pDeclTarget)
        env->SetTarget(fn->pDeclTarget);

    if (!env->CreateNewLocalFrame())
    {
        // Out of activation records: returning here rather than running the body is what actually
        // stops a runaway recursion, which logging alone does not.
        if (call.pResult) call.pResult->SetUndefined();
        env->SetTarget(savedTarget);
        return;
    }

    // `arguments` is an array with `callee`, which the CLIK code uses for currying.
    GASArrayObject* argsArray = env->GetGC()->NewArray();
    for (int i = 0; i < call.NArgs; ++i)
        argsArray->PushBack(call.Arg(i));
    GASValue calleeVal;
    calleeVal.SetAsFunction(fn);
    argsArray->SetConstMemberRaw(sc, "callee", calleeVal);

    // DISHONORED(port): 2012 0x9f2b40. A `super(...)` call arrives with the GASSuperObject as its
    // `this`; the body must see the instance, so it is unwrapped here - and only here, because the
    // super object itself is what the next `super` is derived from, below.
    GASObjectInterface* pThisIface = call.pThis;
    GASObjectInterface* pRealThis = pThisIface;
    if (pThisIface != 0 && pThisIface->GetObjectType() == Object_Super)
        pRealThis = ((GASSuperObject*)pThisIface->ToASObject())->pRealThis;

    GASValue thisVal;
    if (pRealThis)
    {
        GFxASCharacter* ch = pRealThis->ToASCharacter();
        if (ch) thisVal.SetAsCharacter(ch);
        else thisVal.SetAsObject(pRealThis->ToASObject());
    }

    // `super`: the prototype one level above the prototype that DECLARES the running function, with
    // the real `this` kept so that super.method() runs against the instance.
    //
    // Deriving it from `this.__proto__.__proto__` is the defect agent BC measured as the shop asset's
    // stack overflow (agentBC.md 6.5): when a method declared on a base prototype is invoked with an
    // instance of a subclass two levels down, `this.__proto__.__proto__` is the base prototype
    // itself, so `super.method` resolves to the very function that is running and the call recurses
    // until the 64-activation guard fires. Every one of the 669 __Packages registrations in the cook
    // builds a chain deep enough for that to happen.
    //
    // Retail's answer is InvokeContext::Setup (2012 0x9f2b40): it starts at `this.__proto__` and,
    // when the call carried a method name, walks the prototype chain with
    // GASObjectInterface::FindOwner (0x9dac70) to find the object that actually owns that name -
    // `v57 = FindOwner(v54 + 16, sc, name)` - and builds the GASSuperObject from *that* prototype's
    // __proto__ and *that* prototype's __constructor__. That is what is ported here.
    GASValue superVal;
    if (pThisIface)
    {
        // The chain is walked from the value the call carried - the super object when this is a
        // `super(...)` - and never from the unwrapped instance; that is what keeps a three-deep
        // class hierarchy moving one level per call instead of standing on the derived prototype.
        GASObject* self = pThisIface->ToASObject();
        GASObject* proto = self ? self->Get__proto__() : 0;
        if (proto == 0 && pThisIface->ToASCharacter())
            proto = pThisIface->ToASCharacter()->pProto;
        GASObject* declaring = proto;
        if (proto != 0 && call.pFuncName != 0)
        {
            GASObjectInterface* owner = proto->FindOwner(sc, *call.pFuncName);
            if (owner != 0 && owner->ToASObject() != 0)
                declaring = owner->ToASObject();
        }
        GASObject* superProto = declaring ? declaring->Get__proto__() : 0;
        if (superProto)
        {
            GASFunctionObject* superCtor = declaring->Get__constructor__(sc);
            if (superCtor == 0)
                superCtor = superProto->Get__constructor__(sc);
            GASSuperObject* so = new GASSuperObject(sc, superProto, pRealThis, superCtor);
            superVal.SetAsObject(so);
        }
    }

    if (fn->Version == 2)
    {
        env->AddLocalRegisters(fn->RegisterCount);
        // The preload bits of DefineFunction2, in the order the spec and the retail body assign
        // registers: this, arguments, super, _root, _parent, _global.
        unsigned int reg = 1;
        if ((fn->Flags & GASFunctionObject::PreloadThis) != 0)
            *env->LocalRegisterPtr(reg++) = thisVal;
        else if ((fn->Flags & GASFunctionObject::SuppressThis) == 0)
            env->AddLocal(env->GetBuiltin(GASbuiltin_this), thisVal);

        if ((fn->Flags & GASFunctionObject::PreloadArgs) != 0)
        {
            GASValue v;
            v.SetAsObject(argsArray);
            *env->LocalRegisterPtr(reg++) = v;
        }
        else if ((fn->Flags & GASFunctionObject::SuppressArgs) == 0)
        {
            GASValue v;
            v.SetAsObject(argsArray);
            env->AddLocal(env->GetBuiltin(GASbuiltin_arguments), v);
        }

        if ((fn->Flags & GASFunctionObject::PreloadSuper) != 0)
            *env->LocalRegisterPtr(reg++) = superVal;
        else if ((fn->Flags & GASFunctionObject::SuppressSuper) == 0)
            env->AddLocal(env->GetBuiltin(GASbuiltin_super), superVal);

        if ((fn->Flags & GASFunctionObject::PreloadRoot) != 0)
        {
            GASValue v;
            GFxMovieRoot* root = env->GetMovieRoot();
            v.SetAsCharacter(root ? root->GetLevel0() : 0);
            *env->LocalRegisterPtr(reg++) = v;
        }
        if ((fn->Flags & GASFunctionObject::PreloadParent) != 0)
        {
            GASValue v;
            v.SetAsCharacter(env->GetTarget() ? env->GetTarget()->GetParent() : 0);
            *env->LocalRegisterPtr(reg++) = v;
        }
        if ((fn->Flags & GASFunctionObject::PreloadGlobal) != 0)
        {
            GASValue v;
            v.SetAsObject(env->GetGC()->GetGlobalObject());
            *env->LocalRegisterPtr(reg++) = v;
        }

        for (unsigned int i = 0; i < fn->NumArgs; ++i)
        {
            GASValue a;
            if ((int)i < call.NArgs)
                a = call.Arg((int)i);
            if (fn->Args[i].Register != 0)
                *env->LocalRegisterPtr(fn->Args[i].Register) = a;
            else
                env->AddLocal(fn->Args[i].Name, a);
        }
    }
    else
    {
        env->AddLocal(env->GetBuiltin(GASbuiltin_this), thisVal);
        GASValue argsVal;
        argsVal.SetAsObject(argsArray);
        env->AddLocal(env->GetBuiltin(GASbuiltin_arguments), argsVal);
        env->AddLocal(env->GetBuiltin(GASbuiltin_super), superVal);
        for (unsigned int i = 0; i < fn->NumArgs; ++i)
        {
            GASValue a;
            if ((int)i < call.NArgs)
                a = call.Arg((int)i);
            env->AddLocal(fn->Args[i].Name, a);
        }
    }

    fn->pBuffer->Execute(env, fn->StartPC, fn->Length, call.pResult,
                         fn->Version == 2 ? GASActionBuffer::Exec_Function2
                                          : GASActionBuffer::Exec_Function);

    if (fn->Version == 2)
        env->DropLocalRegisters(fn->RegisterCount);
    env->PopLocalFrame();
    env->SetTarget(savedTarget);
}

// ---------------------------------------------------------------------------------------------
// Invoking whatever is on the stack, which four opcodes need (0x3D, 0x40, 0x52, 0x53).

// DISHONORED(bringup): where the interpreter is, so a script error can say which buffer raised it.
// Written once per opcode and read only when an error is logged, so the innermost Execute that is
// running is always the one named.
static int GFxAS2ErrPC = -1;
static int GFxAS2ErrLen = 0;

// DISHONORED(bringup): the same resolution the four invoking opcodes share - a constructor or method
// can arrive as a function value or as an object whose ToFunction answers one (GASSuperObject, and
// any GASObject a class was stored in).
static GASFunctionObject* GFxAS2ResolveFunction(const GASValue& v)
{
    GASFunctionObject* fn = v.GetFunction();
    if (fn == 0 && v.IsObject() && v.GetObject() != 0)
        fn = v.GetObject()->ToFunction();
    return fn;
}

static void GFxAS2CallFunctionValue(GASEnvironment* env, const GASValue& fnVal,
                                    GASObjectInterface* self, int nargs, GASValue* result,
                                    const GASString& name)
{
    GASFunctionObject* fn = fnVal.GetFunction();
    if (fn == 0 && fnVal.IsObject())
        fn = fnVal.GetObject()->ToFunction();
    if (fn == 0)
    {
        // Naming the receiver is what makes this error actionable rather than a count: a method
        // missing on a Sprite is a class-library gap, a method missing on a Shape or an EditText is
        // the content asking a non-clip for a MovieClip method, which retail also refuses.
        GFxASCharacter* recvChar = self ? self->ToASCharacter() : 0;
        // DISHONORED(bringup): the receiver's own target path, not just its type. "attachMovie on
        // Sprite" does not say which clip asked, and which clip it was is the answer to every one of
        // the seven script errors the menu's census reports (agentDG.md 2).
        GASString path = recvChar != 0 ? recvChar->GetTargetPath(env->GetSC()) : GASString();
        env->LogScriptError("call of a value that is not a function: '%s' at pc %d of a %d-byte "
                            "buffer, on %s %s", name.ToCStr(), GFxAS2ErrPC, GFxAS2ErrLen,
                            recvChar != 0 ? recvChar->GetCharacterTypeName()
                                          : (self != 0 ? "an object" : "undefined"),
                            recvChar != 0 ? path.ToCStr() : "");
        result->SetUndefined();
        return;
    }
    // The name is handed on, because `super` inside the callee is resolved against the prototype
    // that declares it; retail passes the same string down as Invoke's third parameter.
    GASFnCall call(result, self, env, nargs, env->GetTopIndex(), &name);
    fn->Invoke(call);
}

// ---------------------------------------------------------------------------------------------

void GASActionBuffer::Execute(GASEnvironment* env, int startPC, int execBytes, GASValue* retval,
                              ExecuteType execType)
{                                                                     // 2012 0x9ea900
    if (Bytes == 0 || Length == 0)
        return;


    GASStringContext* sc = env->GetSC();
    const int stopPC = startPC + execBytes > (int)Length ? (int)Length : startPC + execBytes;
    int pc = startPC;

    enum { MaxWith = 8 };
    GASWithStackEntry WithStack[MaxWith];
    unsigned int WithCount = 0;

    GFxASCharacter* originalTarget = env->GetTarget();
    GFxMovieRoot* movieRoot = env->GetMovieRoot();

    while (pc < stopPC)
    {
        while (WithCount > 0 && pc >= WithStack[WithCount - 1].BlockEndPC)
            --WithCount;

        const unsigned char op = Bytes[pc];
        int nextPC;
        unsigned int bodyLen = 0;
        if ((op & 0x80) != 0)
        {
            if (pc + 3 > stopPC)
                break;
            bodyLen = (unsigned int)Bytes[pc + 1] | ((unsigned int)Bytes[pc + 2] << 8);
            nextPC = pc + 3 + (int)bodyLen;
        }
        else
        {
            nextPC = pc + 1;
        }

        ++OpsExecuted;
        ++OpCounts[op];
        ++GFxAS2OpsThisBuffer;
        if ((GFxAS2OpTraceFrom != 0 && GFxAS2OpsThisBuffer >= GFxAS2OpTraceFrom
             && GFxAS2OpsThisBuffer < GFxAS2OpTraceFrom + GFxAS2OpTraceCount)
            || (GFxAS2OpTraceLen != 0 && (int)Length == GFxAS2OpTraceLen
                && pc >= GFxAS2OpTraceLo && pc <= GFxAS2OpTraceHi))
        {
            GFxLogf("DISHONORED(bringup): AS2 op %u: pc %4d %-16s stack %2u top '%s'",
                    GFxAS2OpsThisBuffer, pc, GFxAS2GetOpcodeName(op), env->GetStackSize(),
                    env->GetStackSize() > 0 ? env->Top(0).ToString(env).ToCStr() : "");
        }
        if (GFxAS2OpsThisBuffer > GFxAS2OpBudget)
        {
            // Reported once per advance, not once per buffer: the first buffer to run out of
            // budget names the frame and the rest of the frame is abandoned quietly.
            if (GFxAS2OpsThisBuffer == GFxAS2OpBudget + 1)
            {
                GFxASCharacter* t = env->GetTarget();
                env->LogScriptError("this frame's action buffers exceeded %u opcodes and were "
                                    "abandoned (a loop whose exit depends on a library method that "
                                    "is not ported); running on %s at pc %d of %d, %d local frames",
                                    GFxAS2OpBudget,
                                    t ? t->GetTargetPath(env->GetSC()).ToCStr() : "no target",
                                    pc, (int)Length, env->GetLocalFrameDepth());
            }
            break;
        }

        GFxAS2ErrPC = pc;
        GFxAS2ErrLen = (int)Length;
        GFxASCharacter* target = env->GetTarget();
        GFxSprite* sprite = target ? target->ToSprite() : 0;

        switch (op)
        {
        case GASop_End:
            pc = stopPC;
            continue;

        // --- timeline ---
        case GASop_NextFrame:
            if (sprite) sprite->GotoFrame(sprite->GetCurrentFrame() + 1);
            break;
        case GASop_PrevFrame:
            if (sprite && sprite->GetCurrentFrame() > 0)
                sprite->GotoFrame(sprite->GetCurrentFrame() - 1);
            break;
        case GASop_Play:
            if (sprite) sprite->SetPlaying(true);
            break;
        case GASop_Stop:
            if (sprite) sprite->SetPlaying(false);
            break;

        // --- SWF 4 arithmetic (still emitted by the cook's older movies) ---
        case GASop_Add:
        {
            GASValue r = env->Pop();
            env->Top().SetNumber(env->Top().ToNumber(env) + r.ToNumber(env));
            break;
        }
        case GASop_Subtract:
        {
            GASValue r = env->Pop();
            env->Top().Sub(env, r);
            break;
        }
        case GASop_Multiply:
        {
            GASValue r = env->Pop();
            env->Top().Mul(env, r);
            break;
        }
        case GASop_Divide:
        {
            GASValue r = env->Pop();
            env->Top().Div(env, r);
            break;
        }
        case GASop_Modulo:
        {
            GASValue r = env->Pop();
            env->Top().Mod(env, r);
            break;
        }
        case GASop_Equal:
        {
            GASValue r = env->Pop();
            bool eq = env->Top().ToNumber(env) == r.ToNumber(env);
            env->Top().SetBool(eq);
            break;
        }
        case GASop_LessThan:
        {
            GASValue r = env->Pop();
            bool lt = env->Top().ToNumber(env) < r.ToNumber(env);
            env->Top().SetBool(lt);
            break;
        }
        case GASop_LogicalAnd:
        {
            GASValue r = env->Pop();
            bool v = env->Top().ToBool(env) && r.ToBool(env);
            env->Top().SetBool(v);
            break;
        }
        case GASop_LogicalOr:
        {
            GASValue r = env->Pop();
            bool v = env->Top().ToBool(env) || r.ToBool(env);
            env->Top().SetBool(v);
            break;
        }
        case GASop_LogicalNot:
            env->Top().SetBool(!env->Top().ToBool(env));
            break;
        case GASop_StringEqual:
        {
            GASValue r = env->Pop();
            bool eq = strcmp(env->Top().ToString(env).ToCStr(), r.ToString(env).ToCStr()) == 0;
            env->Top().SetBool(eq);
            break;
        }
        case GASop_StringLength:
            env->Top().SetInt((int)env->Top().ToString(env).GetSize());
            break;
        case GASop_SubString:
        case GASop_MBSubString:
        {
            int count = env->Pop().ToInt32(env);
            int base = env->Pop().ToInt32(env) - 1;
            GASString s = env->Top().ToString(env);
            if (base < 0) base = 0;
            int len = (int)s.GetSize();
            if (base > len) base = len;
            if (count < 0 || base + count > len) count = len - base;
            env->Top().SetString(sc->CreateString(s.ToCStr() + base, (unsigned int)count));
            break;
        }
        case GASop_Pop:
            env->Drop(1);
            break;
        case GASop_ToInteger:
            env->Top().SetInt(env->Top().ToInt32(env));
            break;

        // --- variables ---
        case GASop_GetVariable:
        {
            GASString name = env->Top().ToString(env);
            GASValue v;
            env->GetVariable(name, &v, WithStack, WithCount);
            env->Top() = v;
            break;
        }
        case GASop_SetVariable:
        {
            GASValue v = env->Pop();
            GASString name = env->Pop().ToString(env);
            env->SetVariable(name, v, WithStack, WithCount);
            break;
        }
        case GASop_SetTargetExpression:
            GFxAS2SetTargetOpCode(env);
            break;
        case GASop_StringConcat:
        {
            GASValue r = env->Pop();
            GASString a = env->Top().ToString(env);
            GASString b = r.ToString(env);
            unsigned int n = a.GetSize() + b.GetSize();
            char stack[512];
            char* buf = n < sizeof(stack) ? stack : (char*)malloc(n + 1);
            memcpy(buf, a.ToCStr(), a.GetSize());
            memcpy(buf + a.GetSize(), b.ToCStr(), b.GetSize());
            buf[n] = 0;
            env->Top().SetString(sc->CreateString(buf, n));
            if (buf != stack) free(buf);
            break;
        }
        case GASop_GetProperty:
        {
            int index = env->Pop().ToInt32(env);
            GASString path = env->Top().ToString(env);
            GFxASCharacter* t = env->FindTarget(path);
            GASValue out;
            if (t) GFxAS2GetDisplayProperty(t, index, &out);
            env->Top() = out;
            break;
        }
        case GASop_SetProperty:
        {
            GASValue v = env->Pop();
            int index = env->Pop().ToInt32(env);
            GASString path = env->Pop().ToString(env);
            GFxASCharacter* t = env->FindTarget(path);
            if (t) GFxAS2SetDisplayProperty(t, index, v, env);
            break;
        }
        case GASop_DuplicateClip:
        {
            int depth = env->Pop().ToInt32(env) - 16384;
            GASString newName = env->Pop().ToString(env);
            GASString path = env->Pop().ToString(env);
            GFxASCharacter* src = env->FindTarget(path);
            if (src && sprite)
                sprite->CreateEmptyMovieClip(newName, depth);
            break;
        }
        case GASop_RemoveClip:
        {
            GASString path = env->Pop().ToString(env);
            GFxASCharacter* t = env->FindTarget(path);
            if (t && t->GetParent())
            {
                GFxSprite* ps = t->GetParent()->ToSprite();
                if (ps) ps->RemoveDisplayObject(t->GetDepth(), t->GetId());
            }
            break;
        }
        case GASop_Trace:
        {
            GASString s = env->Pop().ToString(env);
            // DISHONORED(bringup): through the log hook, not stdout - the game has no console, and
            // the content's own trace() is the only narration of what its classes do.
            GFxLogf("DISHONORED(bringup): AS2 trace: %s", s.ToCStr());
            break;
        }
        case GASop_StartDragMovie:
        case GASop_StopDragMovie:
            // Mouse dragging needs the input path, which is not this package. The operands are
            // consumed so the stack stays balanced, which is what matters for the rest of the run.
            if (op == GASop_StartDragMovie)
            {
                GASValue lockCenter = env->Pop();
                if (lockCenter.ToBool(env)) env->Drop(4);
                env->Drop(2);
            }
            ++OpsUnimplemented;
            break;
        case GASop_StringCompare:
        {
            GASValue r = env->Pop();
            bool lt = strcmp(env->Top().ToString(env).ToCStr(), r.ToString(env).ToCStr()) < 0;
            env->Top().SetBool(lt);
            break;
        }
        case GASop_Throw:
            env->ThrowValue = env->Pop();
            env->bThrowing = true;
            pc = stopPC;
            continue;
        case GASop_CastOp:
            GFxAS2CastObjectOpCode(env);
            break;
        case GASop_ImplementsOp:
            GFxAS2ImplementsOpCode(env);
            break;

        // --- SWF 5 built-ins that are opcodes rather than library calls ---
        case GASop_Random:
        {
            int max = env->Top().ToInt32(env);
            env->Top().SetInt(max > 0 ? (rand() % max) : 0);
            break;
        }
        case GASop_MBLength:
            env->Top().SetInt((int)env->Top().ToString(env).GetSize());
            break;
        case GASop_Ord:
        case GASop_MBOrd:
            env->Top().SetInt((int)(unsigned char)env->Top().ToString(env).ToCStr()[0]);
            break;
        case GASop_Chr:
        case GASop_MBChr:
        {
            char buf[2] = { (char)env->Top().ToInt32(env), 0 };
            env->Top().SetString(sc->CreateString(buf, 1));
            break;
        }
        case GASop_GetTimer:
            env->Push(GASValue((double)GFxAS2GetTimerMs()));
            break;

        case GASop_Delete:
        {
            GASString name = env->Pop().ToString(env);
            GASValue objVal = env->Pop();
            GASObjectInterface* oi = objVal.ToObjectInterface(env);
            env->Push(GASValue(oi != 0 && oi->DeleteMember(sc, name)));
            break;
        }
        case GASop_Delete2:
        {
            GASString name = env->Pop().ToString(env);
            bool done = false;
            if (target)
                done = target->DeleteMember(sc, name);
            if (!done)
            {
                GASObject* g = env->GetGC()->GetGlobalObject();
                if (g) done = g->DeleteMember(sc, name);
            }
            env->Push(GASValue(done));
            break;
        }
        case GASop_DefineLocal:
        {
            GASValue v = env->Pop();
            GASString name = env->Pop().ToString(env);
            if (env->GetLocalFrameDepth() > 0)
            {
                if (!env->SetLocal(name, v))
                    env->AddLocal(name, v);
            }
            else
            {
                env->SetVariableRaw(name, v, WithStack, WithCount);
            }
            break;
        }
        case GASop_DeclareLocal:
        {
            GASString name = env->Pop().ToString(env);
            if (env->GetLocalFrameDepth() > 0)
                env->DeclareLocal(name);
            break;
        }
        case GASop_CallFunction:
        {
            GASString name = env->Pop().ToString(env);
            int nargs = env->Pop().ToInt32(env);
            GASValue fnVal;
            env->GetVariable(name, &fnVal, WithStack, WithCount);
            GASValue result;
            GASObjectInterface* self = target;
            GFxAS2CallFunctionValue(env, fnVal, self, nargs, &result, name);
            env->Drop(nargs);
            env->Push(result);
            break;
        }
        case GASop_Return:
            if (retval)
                *retval = env->Pop();
            else
                env->Drop(1);
            pc = stopPC;
            continue;
        case GASop_New:
        {
            GASString name = env->Pop().ToString(env);
            int nargs = env->Pop().ToInt32(env);
            GASValue ctorVal;
            env->GetVariable(name, &ctorVal, WithStack, WithCount);
            GASFunctionObject* ctor = GFxAS2ResolveFunction(ctorVal);
            if (ctor == 0)
                env->LogScriptError("new: '%s' is not a constructor, at pc %d of a %d-byte buffer",
                                    name.ToCStr(), GFxAS2ErrPC, GFxAS2ErrLen);
            GASObject* obj = env->OperatorNew(ctor, nargs, env->GetTopIndex());
            env->Drop(nargs);
            GASValue out;
            // OperatorNew returns one reference of its own; the value now owns the object.
            if (obj) { out.SetAsObject(obj); obj->Release(); }
            env->Push(out);
            break;
        }
        case GASop_NewMethod:
        {
            // DISHONORED(port): opcode 0x53, the same blank-name rule as 0x52 above.
            GASValue nameVal = env->Pop();
            const bool bNameIsBlank = nameVal.IsUndefined() || nameVal.IsNull()
                || (nameVal.IsString() && nameVal.GetString().IsEmpty());
            GASString name = bNameIsBlank ? sc->CreateConstString("") : nameVal.ToString(env);
            GASValue objVal = env->Pop();
            int nargs = env->Pop().ToInt32(env);
            GASObjectInterface* oi = objVal.ToObjectInterface(env);
            GASValue ctorVal;
            if (name.IsEmpty())
                ctorVal = objVal;
            else if (oi)
                oi->GetMember(env, name, &ctorVal);
            GASFunctionObject* ctor = GFxAS2ResolveFunction(ctorVal);
            if (ctor == 0)
            {
                GFxASCharacter* recvChar = oi ? oi->ToASCharacter() : 0;
                GASString path = recvChar != 0 ? recvChar->GetTargetPath(env->GetSC()) : GASString();
                env->LogScriptError("new: '%s' is not a constructor on %s, at pc %d of a %d-byte "
                                    "buffer", name.ToCStr(),
                                    path.IsEmpty() ? "an object" : path.ToCStr(),
                                    GFxAS2ErrPC, GFxAS2ErrLen);
            }
            GASObject* obj = env->OperatorNew(ctor, nargs, env->GetTopIndex());
            env->Drop(nargs);
            GASValue out;
            if (obj) { out.SetAsObject(obj); obj->Release(); }
            env->Push(out);
            break;
        }
        case GASop_InitArray:
        {
            int n = env->Pop().ToInt32(env);
            GASArrayObject* arr = env->GetGC()->NewArray();
            arr->Resize(n > 0 ? (unsigned int)n : 0);
            for (int i = 0; i < n; ++i)
                arr->SetElement((unsigned int)i, env->Pop());
            GASValue v;
            v.SetAsObject(arr);
            env->Push(v);
            break;
        }
        case GASop_InitObject:
        {
            int n = env->Pop().ToInt32(env);
            GASObject* obj = env->GetGC()->NewObject();
            for (int i = 0; i < n; ++i)
            {
                GASValue v = env->Pop();
                GASString name = env->Pop().ToString(env);
                obj->SetMemberRaw(sc, name, v, GASPropFlags());
            }
            GASValue out;
            out.SetAsObject(obj);
            env->Push(out);
            break;
        }
        case GASop_TypeOf:
            env->Top().SetString(env->Top().Typeof(env));
            break;
        case GASop_TargetPath:
        {
            GFxASCharacter* ch = env->Top().GetCharacter();
            if (ch) env->Top().SetString(ch->GetTargetPath(sc));
            else env->Top().SetUndefined();
            break;
        }
        case GASop_Enumerate:
            GFxAS2EnumerateOpCode(env, false);
            break;
        case GASop_Enumerate2:
            GFxAS2EnumerateOpCode(env, true);
            break;
        case GASop_NewAdd:
        {
            GASValue r = env->Pop();
            env->Top().Add(env, r);
            break;
        }
        case GASop_NewLessThan:
        {
            GASValue r = env->Pop();
            int c = env->Top().Compare(env, r);
            if (c == 2) env->Top().SetUndefined();
            else env->Top().SetBool(c < 0);
            break;
        }
        case GASop_Greater:
        {
            GASValue r = env->Pop();
            int c = env->Top().Compare(env, r);
            if (c == 2) env->Top().SetUndefined();
            else env->Top().SetBool(c > 0);
            break;
        }
        case GASop_StringGreater:
        {
            GASValue r = env->Pop();
            bool gt = strcmp(env->Top().ToString(env).ToCStr(), r.ToString(env).ToCStr()) > 0;
            env->Top().SetBool(gt);
            break;
        }
        case GASop_NewEquals:
        {
            GASValue r = env->Pop();
            bool eq = env->Top().IsEqual(env, r);
            env->Top().SetBool(eq);
            break;
        }
        case GASop_StrictEqual:
        {
            GASValue r = env->Pop();
            bool eq = env->Top().IsStrictEqual(env, r);
            env->Top().SetBool(eq);
            break;
        }
        case GASop_ToNumber:
            env->Top().SetNumber(env->Top().ToNumber(env));
            break;
        case GASop_ToString:
            env->Top().SetString(env->Top().ToString(env));
            break;
        case GASop_Dup:
        {
            GASValue v = env->Top();
            env->Push(v);
            break;
        }
        case GASop_Swap:
        {
            GASValue a = env->Pop();
            GASValue b = env->Pop();
            env->Push(a);
            env->Push(b);
            break;
        }
        case GASop_GetMember:
        {
            GASString name = env->Pop().ToString(env);
            GASValue objVal = env->Top();
            GASValue out;
            GASObjectInterface* oi = objVal.ToObjectInterface(env);
            if (oi)
            {
                oi->GetMember(env, name, &out);
                // A PROPERTY is a getter/setter pair, never a value: whatever answered the lookup,
                // the stack gets the getter's result. Without this the comparison operators see the
                // pair itself and every `member == undefined` test against a property is false.
                if (out.IsProperty())
                {
                    const GASValue prop = out;
                    prop.GetPropertyValue(env, oi, &out);
                }
            }
            else if (objVal.IsString() && name == env->GetBuiltin(GASbuiltin_length))
                out.SetInt((int)objVal.GetString().GetSize());
            // DISHONORED(bringup): -gfxuiwatch=<member> logs every read of one member name with the
            // receiver and the answer. "Why is this property undefined" is otherwise a bisect.
            if (GFxAS2WatchMatches(name.ToCStr()) && GFxAS2WatchCount > 0)
            {
                --GFxAS2WatchCount;
                GFxASCharacter* rc = oi ? oi->ToASCharacter() : 0;
                GFxLogf("DISHONORED(bringup): watch '%s' on %p %s -> type %d '%s' (pc %d of %d)",
                        name.ToCStr(), (void*)oi,
                        rc ? rc->GetTargetPath(sc).ToCStr() : (oi ? "an object" : "undefined"),
                        (int)out.GetType(), out.ToString(env).ToCStr(), pc, (int)Length);
            }
            env->Top() = out;
            break;
        }
        case GASop_SetMember:
        {
            GASValue v = env->Pop();
            GASString name = env->Pop().ToString(env);
            GASValue objVal = env->Pop();
            GASObjectInterface* oi = objVal.ToObjectInterface(env);
            if (GFxAS2WatchMatches(name.ToCStr()) && GFxAS2WatchCount > 0)
            {
                --GFxAS2WatchCount;
                GFxASCharacter* rc = oi ? oi->ToASCharacter() : 0;
                GFxLogf("DISHONORED(bringup): watch SET '%s' on %p %s = type %d '%s' (pc %d of %d)",
                        name.ToCStr(), (void*)oi,
                        rc ? rc->GetTargetPath(sc).ToCStr() : (oi ? "an object" : "undefined"),
                        (int)v.GetType(), v.ToString(env).ToCStr(), pc, (int)Length);
            }
            if (oi)
                oi->SetMember(env, name, v, GASPropFlags());
            break;
        }
        case GASop_Increment:
            env->Top().SetNumber(env->Top().ToNumber(env) + 1.0);
            break;
        case GASop_Decrement:
            env->Top().SetNumber(env->Top().ToNumber(env) - 1.0);
            break;
        case GASop_CallMethod:
        {
            // DISHONORED(port): 2012 GASActionBuffer::Execute 0x995ee0, opcode 0x52. The name is
            // popped as a VALUE and only then coerced: an *undefined* name (not the string
            // "undefined") means the object on the stack is itself the function to invoke. That is
            // exactly how the AS2 compiler emits `super(...)` - push argc, push register 2, push
            // undefined, CallMethod - so every class constructor in the cook stops on its first
            // statement without it.
            GASValue nameVal = env->Pop();
            const bool bNameIsBlank = nameVal.IsUndefined() || nameVal.IsNull()
                || (nameVal.IsString() && nameVal.GetString().IsEmpty());
            GASString name = bNameIsBlank ? sc->CreateConstString("") : nameVal.ToString(env);
            GASValue objVal = env->Pop();
            int nargs = env->Pop().ToInt32(env);
            GASObjectInterface* oi = objVal.ToObjectInterface(env);
            if (GFxAS2WatchMatches(name.ToCStr()) && GFxAS2WatchCount > 0)
            {
                --GFxAS2WatchCount;
                GFxLogf("DISHONORED(bringup): watch CALL '%s' receiver type %d, interface %p "
                        "(pc %d of %d)", name.ToCStr(), (int)objVal.GetType(), (void*)oi, pc,
                        (int)Length);
            }
            GASValue fnVal;
            GASObjectInterface* self = oi;
            if (name.IsEmpty())
            {
                fnVal = objVal;
                // The receiver stays exactly what the stack carried: for `super(...)` that is the
                // super object, which the invocation unwraps for `this` and keeps for `super`.
                if (oi == 0)
                    self = target;
            }
            else if (oi)
            {
                oi->GetMember(env, name, &fnVal);
            }
            GASValue result;
            GFxAS2CallFunctionValue(env, fnVal, self, nargs, &result, name);
            if (GFxAS2WatchMatches(name.ToCStr()) && GFxAS2WatchCount > 0)
            {
                --GFxAS2WatchCount;
                GFxLogf("DISHONORED(bringup): watch RET '%s' -> type %d (pc %d of %d)",
                        name.ToCStr(), (int)result.GetType(), pc, (int)Length);
            }
            env->Drop(nargs);
            env->Push(result);
            break;
        }
        case GASop_InstanceOf:
            GFxAS2InstanceOfOpCode(env);
            break;
        case GASop_BitwiseAnd: { GASValue r = env->Pop(); env->Top().And(env, r); break; }
        case GASop_BitwiseOr:  { GASValue r = env->Pop(); env->Top().Or(env, r); break; }
        case GASop_BitwiseXor: { GASValue r = env->Pop(); env->Top().Xor(env, r); break; }
        case GASop_ShiftLeft:  { GASValue r = env->Pop(); env->Top().Shl(env, r); break; }
        case GASop_ShiftRight: { GASValue r = env->Pop(); env->Top().Asr(env, r); break; }
        case GASop_ShiftRightUnsigned: { GASValue r = env->Pop(); env->Top().Lsr(env, r); break; }
        case GASop_Extends:
            GFxAS2ExtendsOpCode(env);
            break;

        // --- length-prefixed ---
        case GASop_GotoFrame:
        {
            unsigned int frame = (unsigned int)GFxAS2ReadU16(Bytes + pc + 3);
            if (sprite) sprite->GotoFrame(frame);
            break;
        }
        case GASop_GotoLabel:
            if (sprite) sprite->GotoLabeledFrame((const char*)(Bytes + pc + 3), 0);
            break;
        case GASop_GotoExpression:
        {
            unsigned char flags = Bytes[pc + 3];
            GASValue frameVal = env->Pop();
            bool play = (flags & 1) != 0;
            if (sprite)
            {
                if (frameVal.IsString() && !frameVal.GetString().IsEmpty()
                    && (frameVal.GetString().ToCStr()[0] < '0'
                        || frameVal.GetString().ToCStr()[0] > '9'))
                    sprite->GotoLabeledFrame(frameVal.GetString().ToCStr(), 0);
                else
                    sprite->GotoFrame((unsigned int)frameVal.ToInt32(env));
                sprite->SetPlaying(play);
            }
            break;
        }
        case GASop_SetTarget:
        {
            const char* name = (const char*)(Bytes + pc + 3);
            if (name[0] == 0)
                env->SetTarget(originalTarget);
            else
            {
                GFxASCharacter* t = env->FindTarget(sc->CreateString(name));
                if (t) env->SetTarget(t);
                else env->SetInvalidTarget(originalTarget);
            }
            break;
        }
        case GASop_StoreRegister:
        {
            unsigned int reg = Bytes[pc + 3];
            *env->LocalRegisterPtr(reg) = env->Top();
            break;
        }
        case GASop_ConstantPool:
            ProcessDeclDict(sc, (unsigned int)pc, (unsigned int)nextPC);
            break;
        case GASop_WaitForFrame:
        case GASop_WaitForFrameExpression:
            // Streaming: retail's WaitForFrameOpCode (2012 0x9e58f0) skips the guarded actions when
            // the frame is not loaded yet. A cooked GFX payload is never partially loaded, so the
            // frame is always available and the guarded block always runs, which is what falling
            // through to the next opcode does.
            if (op == GASop_WaitForFrameExpression)
                env->Drop(1);
            break;
        case GASop_With:
        {
            int blockLen = (int)GFxAS2ReadU16(Bytes + pc + 3);
            GASValue objVal = env->Pop();
            GASObject* obj = objVal.ToObject(env);
            if (obj && WithCount < MaxWith)
            {
                WithStack[WithCount] = GASWithStackEntry(obj, nextPC + blockLen);
                ++WithCount;
            }
            break;
        }
        case GASop_Push:
        {
            int p = pc + 3;
            const int end = nextPC;
            while (p < end)
            {
                unsigned char type = Bytes[p++];
                GASValue v;
                switch (type)
                {
                case GASpush_String:
                {
                    const char* s = (const char*)(Bytes + p);
                    unsigned int n = (unsigned int)strlen(s);
                    v.SetString(sc->CreateString(s, n));
                    p += (int)n + 1;
                    break;
                }
                case GASpush_Float:
                {
                    float f;
                    memcpy(&f, Bytes + p, 4);
                    v.SetNumber((double)f);
                    p += 4;
                    break;
                }
                case GASpush_Null:
                    v.SetNull();
                    break;
                case GASpush_Undefined:
                    v.SetUndefined();
                    break;
                case GASpush_Register:
                    v = *env->LocalRegisterPtr(Bytes[p]);
                    p += 1;
                    break;
                case GASpush_Boolean:
                    v.SetBool(Bytes[p] != 0);
                    p += 1;
                    break;
                case GASpush_Double:
                {
                    // SWF stores a pushed double as two 32-bit halves, high word first.
                    unsigned char tmp[8];
                    memcpy(tmp, Bytes + p + 4, 4);
                    memcpy(tmp + 4, Bytes + p, 4);
                    double d;
                    memcpy(&d, tmp, 8);
                    v.SetNumber(d);
                    p += 8;
                    break;
                }
                case GASpush_Integer:
                {
                    int i;
                    memcpy(&i, Bytes + p, 4);
                    v.SetInt(i);
                    p += 4;
                    break;
                }
                case GASpush_Constant8:
                    v.SetString(GetConstant(Bytes[p]));
                    p += 1;
                    break;
                case GASpush_Constant16:
                    v.SetString(GetConstant(GFxAS2ReadU16(Bytes + p)));
                    p += 2;
                    break;
                default:
                    ++OpsUnimplemented;
                    p = end;
                    break;
                }
                env->Push(v);
            }
            break;
        }
        case GASop_BranchAlways:
        {
            int off = (short)GFxAS2ReadU16(Bytes + pc + 3);
            pc = nextPC + off;
            continue;
        }
        case GASop_BranchIfTrue:
        {
            int off = (short)GFxAS2ReadU16(Bytes + pc + 3);
            bool cond = env->Pop().ToBool(env);
            pc = cond ? nextPC + off : nextPC;
            continue;
        }
        case GASop_CallFrame:
            if (sprite)
            {
                GASValue frameVal = env->Pop();
                unsigned int frame = 0;
                if (frameVal.IsString())
                {
                    if (!sprite->pTimelineDef
                        || !sprite->pTimelineDef->GetLabeledFrame(frameVal.GetString().ToCStr(),
                                                                  &frame))
                        break;
                }
                else
                {
                    frame = (unsigned int)frameVal.ToInt32(env);
                }
                sprite->CallFrameActions(frame);
            }
            break;
        case GASop_GetUrl:
        case GASop_GetUrl2:
        {
            // GetURL is how AS2 reaches the host: "FSCommand:" and "print:" prefixes go to the
            // FSCommand handler and everything else to the loader. The retail body (case 131 of
            // 0x9ea900) splits on exactly that strncmp against "FSCommand:". The handler side is
            // agent BE's (FGFxFSCommandHandler::Callback, 2013 0x586450).
            // DISHONORED(port): the split is now made and the handler called. Dishonored's menus use
            // it for every screen change (TransitionHandler fires getURL("FSCommand:ToXxx")).
            GASString url, target2;
            if (op == GASop_GetUrl)
            {
                const char* u = (const char*)(Bytes + pc + 3);
                url = sc->CreateString(u);
                const char* w = u + strlen(u) + 1;
                target2 = sc->CreateString(w);
            }
            else
            {
                target2 = env->Pop().ToString(env);
                url = env->Pop().ToString(env);
            }
            const char* s = url.ToCStr();
            if (s != 0 && (strncmp(s, "FSCommand:", 10) == 0 || strncmp(s, "fscommand:", 10) == 0))
            {
                GFxMovieRoot* root = env->GetMovieRoot();
                if (root)
                    root->CallFSCommand(s + 10, target2.ToCStr());
            }
            break;
        }
        case GASop_DefineFunction:
            pc = GFxAS2Function1OpCode(this, env, pc, nextPC, target);
            continue;
        case GASop_DefineFunction2:
            pc = GFxAS2Function2OpCode(this, env, pc, nextPC, target);
            continue;
        case GASop_Try:
        {
            // ActionTry's body is: flags u8, tryLen u16, catchLen u16, finallyLen u16, then either a
            // catch variable name or a catch register, then the three blocks back to back.
            unsigned char flags = Bytes[pc + 3];
            unsigned int tryLen = GFxAS2ReadU16(Bytes + pc + 4);
            unsigned int catchLen = GFxAS2ReadU16(Bytes + pc + 6);
            unsigned int finallyLen = GFxAS2ReadU16(Bytes + pc + 8);
            const bool bCatchInRegister = (flags & 0x04) != 0;
            const bool bHasCatch = (flags & 0x01) != 0;
            unsigned char catchRegister = 0;
            GASString catchName;
            int p = pc + 10;
            if (bCatchInRegister)
                catchRegister = Bytes[p++];
            else
            {
                const char* s = (const char*)(Bytes + p);
                catchName = sc->CreateString(s);
                p += (int)strlen(s) + 1;
            }
            const int tryStart = p;
            Execute(env, tryStart, (int)tryLen, retval, execType);
            if (env->bThrowing && bHasCatch)
            {
                GASValue thrown = env->ThrowValue;
                env->bThrowing = false;
                if (bCatchInRegister)
                    *env->LocalRegisterPtr(catchRegister) = thrown;
                else if (env->GetLocalFrameDepth() > 0)
                    env->AddLocal(catchName, thrown);
                else
                    env->SetVariableRaw(catchName, thrown, WithStack, WithCount);
                Execute(env, tryStart + (int)tryLen, (int)catchLen, retval, execType);
            }
            if (finallyLen)
                Execute(env, tryStart + (int)tryLen + (int)catchLen, (int)finallyLen, retval,
                        execType);
            pc = tryStart + (int)tryLen + (int)catchLen + (int)finallyLen;
            if (env->bThrowing)
                return;
            continue;
        }

        default:
            // Retail has no case for 0x08 ToggleQuality or 0x09 StopSounds either; anything else here
            // is a byte the machine does not know and the count is what the acceptance table reports.
            if (op != 0x08 && op != 0x09)
                ++OpsUnimplemented;
            break;
        }

        if (env->bThrowing)
            return;
        pc = nextPC;
    }

    env->SetTarget(originalTarget);
    (void)movieRoot;
}
