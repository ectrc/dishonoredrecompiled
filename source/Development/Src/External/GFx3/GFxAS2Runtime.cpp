// Scaleform GFx 3.3.89 - GASEnvironment and GASGlobalContext. Package BC.
//
// GASEnvironment::GetVariableRaw (2012 0x9e9270) and SetVariable (0x9ea060) are the two bodies this
// file is built from, and between them they define AS2 name resolution, which is four lookups in a
// fixed order and nothing else:
//   1. the local frames of the innermost function, innermost first (FindLocal 0x9e6be0);
//   2. the with-stack, innermost first (the GArrayLH_POD<GASWithStackEntry> every one of those
//      signatures carries as its penultimate parameter);
//   3. the current target character, i.e. the movie clip the code is running on;
//   4. _global and then each loaded level (CheckGlobalAndLevels 0x9e0100).
// A dotted or slashed name is split first (ParsePath 0x9e0390, IsPath 0x9df680) and the path part is
// resolved to a character (FindTarget 0x9e0e70) before the leaf name is looked up on it.
// DISHONORED(port): see GFxAS2.h.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// ---------------------------------------------------------------------------------------------
// GASLocalFrame

GASLocalFrame::~GASLocalFrame()
{
    delete[] Locals;
}

void GASLocalFrame::Clear()
{
    for (unsigned int i = 0; i < Size; ++i)
    {
        Locals[i].Value.SetUndefined();
        Locals[i].Name = GASString();
    }
    Size = 0;
}

void GASLocalFrame::ReleaseAll()
{
    // Everything, not just [0, Size): a frame that was deeper earlier in the run still holds the
    // values of that depth in its unused tail, and those are references into the AS2 graph.
    for (unsigned int i = 0; i < Capacity; ++i)
    {
        Locals[i].Value.SetUndefined();
        Locals[i].Name = GASString();
    }
    Size = 0;
}

GASValue* GASLocalFrame::Find(const GASString& name)
{
    for (unsigned int i = Size; i > 0; --i)
        if (Locals[i - 1].Name == name)
            return &Locals[i - 1].Value;
    return 0;
}

void GASLocalFrame::Add(const GASString& name, const GASValue& v)
{
    if (Size >= Capacity)
    {
        unsigned int cap = Capacity ? Capacity * 2 : 8;
        Local* next = new Local[cap];
        for (unsigned int i = 0; i < Size; ++i)
        {
            next[i].Name = Locals[i].Name;
            next[i].Value = Locals[i].Value;
        }
        delete[] Locals;
        Locals = next;
        Capacity = cap;
    }
    Locals[Size].Name = name;
    Locals[Size].Value = v;
    ++Size;
}

// ---------------------------------------------------------------------------------------------
// GASEnvironment

GASEnvironment::GASEnvironment()                                      // 2012 0x9fe6c0
    : bThrowing(false), ToStringDepth(0), pGC(0), pTarget(0), bInvalidTarget(false), Stack(0), StackSize(0), StackCapacity(0),
      Frames(0), FrameCount(0), FrameCapacity(0), Registers(0), RegisterCount(0)
{
    StackCapacity = 256;
    Stack = new GASValue[StackCapacity];
    FrameCapacity = 64;
    Frames = new GASLocalFrame[FrameCapacity];
}

GASEnvironment::~GASEnvironment()                                     // 2012 0x9fe750
{
    delete[] Stack;
    delete[] Frames;
    delete[] Registers;
}

void GASEnvironment::Init(GASGlobalContext* gc, GFxASCharacter* target)
{
    pGC = gc;
    pTarget = target;
    SC = *gc->GetSC();
}

void GASEnvironment::ReleaseAll()
{
    Drop((int)StackSize);
    for (unsigned int i = 0; i < StackCapacity; ++i)
        Stack[i].SetUndefined();
    for (unsigned int i = 0; i < FrameCapacity; ++i)
        Frames[i].ReleaseAll();
    FrameCount = 0;
    delete[] Registers;
    Registers = 0;
    RegisterCount = 0;
    for (int i = 0; i < 4; ++i)
        GlobalRegisters[i].SetUndefined();
    ThrowValue.SetUndefined();
    Dummy.SetUndefined();
    ToStringDepth = 0;
    bThrowing = false;
    pTarget = 0;
}

GFxMovieRoot* GASEnvironment::GetMovieRoot() const                    // 2012 0x9df670
{
    return pGC ? pGC->GetMovieRoot() : 0;
}

void GASEnvironment::Push(const GASValue& v)
{
    if (StackSize >= StackCapacity)
    {
        // The value stack is bounded for the same reason the local-frame array is: a runaway
        // recursion in content must produce a reported error, not a bad_alloc out of the machine.
        enum { MaxStack = 1 << 18 };
        if (StackCapacity >= MaxStack)
        {
            LogScriptError("AS2 value stack overflow: more than %u values pushed (%u local frames)",
                           (unsigned int)MaxStack, FrameCount);
            return;
        }
        unsigned int cap = StackCapacity * 2;
        GASValue* next = new GASValue[cap];
        for (unsigned int i = 0; i < StackSize; ++i)
            next[i] = Stack[i];
        delete[] Stack;
        Stack = next;
        StackCapacity = cap;
    }
    Stack[StackSize++] = v;
}

void GASEnvironment::Drop(int n)
{
    while (n-- > 0 && StackSize > 0)
        Stack[--StackSize].SetUndefined();
}

GASValue& GASEnvironment::Top(int off)
{
    // Dummy is a member rather than a function static on purpose: it is a real GASValue that content
    // can write into when the stack underflows, so it has to be dropped with the environment. A
    // function-local static would still hold that reference at process exit, after the AS2 graph it
    // points into has been collected.
    if (StackSize == 0 || (unsigned int)off >= StackSize)
        return Dummy;
    return Stack[StackSize - 1 - off];
}

const GASValue& GASEnvironment::TopConst(int off) const
{
    if (StackSize == 0 || (unsigned int)off >= StackSize)
        return Dummy;
    return Stack[StackSize - 1 - off];
}

GASValue GASEnvironment::Pop()
{
    if (StackSize == 0)
        return GASValue();
    GASValue v = Stack[StackSize - 1];
    Stack[--StackSize].SetUndefined();
    return v;
}

GASValue& GASEnvironment::Bottom(int index)
{
    if (index < 0 || (unsigned int)index >= StackSize)
        return Dummy;
    return Stack[index];
}

bool GASEnvironment::CreateNewLocalFrame()                            // 2012 0x9e7880
{
    if (FrameCount >= FrameCapacity)
    {
        // A runaway recursion in content would otherwise grow without bound; retail caps the
        // activation depth too (GASEnvironment's frame array is a GArray with a limit and the error
        // is "Stack overflow" from LogScriptError).
        LogScriptError("Stack overflow: more than %u nested function calls", FrameCapacity);
        return false;
    }
    Frames[FrameCount].Clear();
    ++FrameCount;
    return true;
}

void GASEnvironment::PopLocalFrame()
{
    if (FrameCount > 0)
    {
        Frames[FrameCount - 1].Clear();
        --FrameCount;
    }
}

GASLocalFrame* GASEnvironment::GetTopLocalFrame()                     // 2012 0x9e0370
{
    return FrameCount ? &Frames[FrameCount - 1] : 0;
}

GASValue* GASEnvironment::FindLocal(const GASString& name)            // 2012 0x9e6be0
{
    if (FrameCount == 0)
        return 0;
    return Frames[FrameCount - 1].Find(name);
}

void GASEnvironment::AddLocal(const GASString& name, const GASValue& v) // 2012 0x9e8910
{
    if (FrameCount == 0)
        return;
    Frames[FrameCount - 1].Add(name, v);
}

void GASEnvironment::DeclareLocal(const GASString& name)              // 2012 0x9e8970
{
    if (FrameCount == 0)
        return;
    if (Frames[FrameCount - 1].Find(name) == 0)
        Frames[FrameCount - 1].Add(name, GASValue());
}

bool GASEnvironment::SetLocal(const GASString& name, const GASValue& v) // 2012 0x9e9150
{
    GASValue* slot = FindLocal(name);
    if (slot == 0)
        return false;
    *slot = v;
    return true;
}

void GASEnvironment::AddLocalRegisters(unsigned int count)            // 2012 0x9f1de0
{
    unsigned int total = RegisterCount + count;
    GASValue* next = new GASValue[total ? total : 1];
    for (unsigned int i = 0; i < RegisterCount; ++i)
        next[i] = Registers[i];
    delete[] Registers;
    Registers = next;
    RegisterCount = total;
}

void GASEnvironment::DropLocalRegisters(unsigned int count)           // 2012 0x9f1e20
{
    RegisterCount = count <= RegisterCount ? RegisterCount - count : 0;
}

GASValue* GASEnvironment::LocalRegisterPtr(unsigned int i)            // 2012 0x9e0e30
{
    // DISHONORED(port): 2012 0x9e0e30. The DefineFunction2 window is the *tail* of the register
    // array and it is indexed BACKWARDS from the end - register 0 is the last slot, register 1 the
    // one before it - which is what keeps a nested call's block clear of its caller's. Indexing
    // from the front instead gave every callee the same slots as its caller.
    if (i < RegisterCount)
        return &Registers[RegisterCount - i - 1];
    // Retail logs "Invalid local register %d" here; the four v1 global registers are this tree's
    // equivalent of the caller-side choice real GFx makes at each StoreRegister site.
    return GlobalRegisterPtr(i);
}

GASValue* GASEnvironment::GlobalRegisterPtr(unsigned int i)
{
    if (i < 4)
        return &GlobalRegisters[i];
    return &Dummy;
}

bool GASEnvironment::IsPath(const GASString& s)                       // 2012 0x9df680
{
    for (const char* p = s.ToCStr(); *p; ++p)
        if (*p == '.' || *p == '/' || *p == ':')
            return true;
    return false;
}

bool GASEnvironment::ParsePath(GASStringContext* sc, const GASString& path, GASString* outPath,
                              GASString* outVar)
{                                                                     // 2012 0x9e0390
    const char* s = path.ToCStr();
    int len = (int)path.GetSize();
    int split = -1;
    for (int i = len - 1; i >= 0; --i)
        if (s[i] == '.' || s[i] == '/' || s[i] == ':')
        {
            split = i;
            break;
        }
    if (split < 0)
        return false;
    // "a/b:c" and "a.b.c" both split at the last separator; ":" is the slash-syntax variable marker
    // and is not part of either side.
    *outPath = sc->CreateString(s, (unsigned int)split);
    *outVar = sc->CreateString(s + split + 1);
    return !outVar->IsEmpty();
}

GFxASCharacter* GASEnvironment::FindTarget(const GASString& path) const  // 2012 0x9e0e70
{
    GFxASCharacter* cur = pTarget;
    const char* s = path.ToCStr();
    if (s[0] == 0)
        return cur;

    GFxMovieRoot* root = GetMovieRoot();
    if (s[0] == '/')
    {
        cur = root ? root->GetLevel0() : 0;
        ++s;
    }

    char name[128];
    while (*s && cur)
    {
        unsigned int n = 0;
        while (*s && *s != '.' && *s != '/' && n < sizeof(name) - 1)
            name[n++] = *s++;
        name[n] = 0;
        if (*s) ++s;
        if (n == 0)
            continue;
        if (strcmp(name, "..") == 0 || strcmp(name, "_parent") == 0)
        {
            cur = cur->GetParent();
            continue;
        }
        if (strcmp(name, ".") == 0 || strcmp(name, "this") == 0)
            continue;
        if (strcmp(name, "_root") == 0)
        {
            cur = root ? root->GetLevel0() : 0;
            continue;
        }
        GASValue v;
        GASStringContext* sc = const_cast<GASStringContext*>(&SC);
        if (cur->GetMemberRaw(sc, sc->CreateString(name), &v) && v.IsCharacter())
        {
            cur = v.GetCharacter();
            continue;
        }
        GFxSprite* sp = cur->ToSprite();
        if (sp)
        {
            GFxCharacter* ch = sp->GetDisplayList().GetCharacterByName(sc, sc->CreateString(name));
            if (ch && ch->IsASCharacter())
            {
                cur = ch->ToASCharacterDef();
                continue;
            }
        }
        return 0;
    }
    return cur;
}

GFxASCharacter* GASEnvironment::FindTargetByValue(const GASValue& v)   // 2012 0x9e4c50
{
    if (v.IsCharacter())
        return v.GetCharacter();
    if (v.IsString())
        return FindTarget(v.GetString());
    return pTarget;
}

bool GASEnvironment::GetMember(GASObjectInterface* obj, const GASString& name, GASValue* out)
{                                                                     // 2012 0x9e5a10
    if (obj == 0)
    {
        out->SetUndefined();
        return false;
    }
    return obj->GetMember(this, name, out);
}

bool GASEnvironment::GetVariableRaw(const GASString& name, GASValue* out,
                                   const GASWithStackEntry* withStack, unsigned int withCount) const
{                                                                     // 2012 0x9e9270
    GASEnvironment* self = const_cast<GASEnvironment*>(this);

    if (GASValue* local = self->FindLocal(name))
    {
        *out = *local;
        return true;
    }
    for (unsigned int i = withCount; i > 0; --i)
    {
        GASObject* o = withStack[i - 1].pObject;
        if (o && o->GetMember(self, name, out))
            return true;
    }
    if (pTarget && pTarget->GetMember(self, name, out))
        return true;
    // CheckGlobalAndLevels (0x9e0100): _global, then the loaded levels. Only level 0 exists until
    // loadMovie is ported.
    GASObject* global = pGC->GetGlobalObject();
    if (global && global->GetMember(self, name, out))
        return true;
    if (name == GetBuiltin(GASbuiltin__global))
    {
        out->SetAsObject(global);
        return true;
    }
    if (name == GetBuiltin(GASbuiltin__root) || name == GetBuiltin(GASbuiltin__level0))
    {
        GFxMovieRoot* root = GetMovieRoot();
        out->SetAsCharacter(root ? root->GetLevel0() : 0);
        return true;
    }
    if (name == GetBuiltin(GASbuiltin_this))
    {
        out->SetAsCharacter(pTarget);
        return true;
    }
    out->SetUndefined();
    return false;
}

bool GASEnvironment::GetVariable(const GASString& path, GASValue* out,
                                const GASWithStackEntry* withStack, unsigned int withCount) const
{                                                                     // 2012 0x9e9d60
    if (IsPath(path))
    {
        GASEnvironment* self = const_cast<GASEnvironment*>(this);
        GASString pathPart, varPart;
        if (ParsePath(self->GetSC(), path, &pathPart, &varPart))
        {
            GFxASCharacter* target = FindTarget(pathPart);
            if (target != 0)
            {
                if (varPart.IsEmpty())
                {
                    out->SetAsCharacter(target);
                    return true;
                }
                return target->GetMember(self, varPart, out);
            }
            // DISHONORED(port): 2012 0x9ea060/0x9e9d60 resolve the prefix to a VALUE and ask it for
            // an object interface; a prefix that names an ordinary object is as good as one that
            // names a clip.
            if (!varPart.IsEmpty())
            {
                GASValue container;
                if (GetVariable(pathPart, &container, withStack, withCount))
                {
                    GASObjectInterface* oi = container.ToObjectInterface(self);
                    if (oi != 0)
                        return oi->GetMember(self, varPart, out);
                }
            }
            out->SetUndefined();
            return false;
        }
    }
    return GetVariableRaw(path, out, withStack, withCount);
}

bool GASEnvironment::SetVariableRaw(const GASString& name, const GASValue& v,
                                    const GASWithStackEntry* withStack, unsigned int withCount)
{                                                                     // 2012 0x9e7790
    if (SetLocal(name, v))
        return true;
    for (unsigned int i = withCount; i > 0; --i)
    {
        GASObject* o = withStack[i - 1].pObject;
        if (o && o->HasMember(GetSC(), name, false))
            return o->SetMember(this, name, v, GASPropFlags());
    }
    if (pTarget)
        return pTarget->SetMember(this, name, v, GASPropFlags());
    GASObject* global = pGC->GetGlobalObject();
    return global ? global->SetMember(this, name, v, GASPropFlags()) : false;
}

bool GASEnvironment::SetVariable(const GASString& path, const GASValue& v,
                                 const GASWithStackEntry* withStack, unsigned int withCount)
{                                                                     // 2012 0x9ea060
    if (IsPath(path))
    {
        GASString pathPart, varPart;
        if (ParsePath(GetSC(), path, &pathPart, &varPart))
        {
            GFxASCharacter* target = FindTarget(pathPart);
            if (target != 0)
                return target->SetMember(this, varPart, v, GASPropFlags());
            // DISHONORED(port): 2012 0x9ea060, as above - the prefix may name an ordinary object.
            GASValue container;
            if (GetVariable(pathPart, &container, withStack, withCount))
            {
                GASObjectInterface* oi = container.ToObjectInterface(this);
                if (oi != 0)
                    return oi->SetMember(this, varPart, v, GASPropFlags());
            }
            return false;
        }
    }
    return SetVariableRaw(path, v, withStack, withCount);
}

GASObject* GASEnvironment::OperatorNew(GASFunctionObject* ctor, int nargs, int firstArgBottom)
{                                                                     // 2012 0x9e89c0
    if (ctor == 0)
        return 0;
    // `new F(...)`: an object whose __proto__ is F.prototype, run F with it as `this`, and keep
    // __constructor__ so instanceof and super work.
    GASValue protoVal;
    GASObject* proto = 0;
    if (ctor->GetMemberRaw(GetSC(), GetBuiltin(GASbuiltin_prototype), &protoVal))
        proto = protoVal.ToObject(this);
    if (proto == 0)
        proto = pGC->GetPrototype(GASGlobalContext::Proto_Object);

    // DISHONORED(port): the object is made by the constructor the PROTOTYPE names, not always by
    // the one the opcode named. `MyClass.prototype.__constructor__` is the class MyClass extends, so
    // a subclass of Array is allocated as an array and only then run through its own constructor.
    GASFunctionObject* maker = ctor;
    if (proto != 0)
    {
        GASFunctionObject* protoCtor = proto->Get__constructor__(GetSC());
        if (protoCtor != 0)
            maker = protoCtor;
    }
    GASObject* obj = maker->CreateNewObject(GetSC(), proto);
    if (obj == 0)
        return 0;
    // DISHONORED(bringup): the object is held for the whole of its own construction. GASObject
    // starts at RefCount 0 and Release() frees at zero, so the first temporary GASValue inside the
    // constructor - which every `this.x = y` makes, because ActionSetMember pops the receiver into
    // one - would take the count to 1 and back to 0 and destroy the object under its own
    // constructor. The caller releases this reference once it owns the value.
    obj->AddRef();
    GASValue ctorVal;
    ctorVal.SetAsFunction(ctor);
    obj->SetMemberRaw(GetSC(), GetBuiltin(GASbuiltin_constructorUS), ctorVal, GASPropFlags(
                          GASPropFlags::PropFlag_DontEnum | GASPropFlags::PropFlag_DontDelete));

    GASValue result;
    GASFnCall call(&result, obj, this, nargs, firstArgBottom);
    ctor->Invoke(call);
    // DISHONORED(port): 2012 0x9e89c0 returns the object it MADE and never looks at the call's
    // result - the GASFnCall's result slot is a local it destroys. AS2's `new` is not JavaScript's:
    // a constructor cannot replace the instance. Honouring the result was an invention of this tree
    // and it fired 25 times in one run of the main menu, because the result slot is not cleared
    // between calls - so `_menu_mc.sel` was a different object from the one SelectionHandler's
    // constructor had run on, its element id and container were never written, and the menu bar's
    // six buttons never faded in.
    return obj;
}

GASObject* GASEnvironment::PrimitiveToTempObject(const GASValue& v)   // 2012 0x9e91a0
{
    GASGlobalContext::ProtoId id = (GASGlobalContext::ProtoId)v.PrototypeForPrimitive();
    GASObjectType type = Object_Object;
    if (id == GASGlobalContext::Proto_String) type = Object_String;
    else if (id == GASGlobalContext::Proto_Number) type = Object_Number;
    else if (id == GASGlobalContext::Proto_Boolean) type = Object_Boolean;
    return new GASPrimitiveObject(GetSC(), pGC->GetPrototype(id), v, type);
}

void GASEnvironment::LogScriptError(const char* fmt, ...) const       // 2012 0x9e4900
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    GFxMovieRoot* root = GetMovieRoot();
    if (root)
        root->LogScriptError("%s", buf);
    else
        printf("DISHONORED(bringup): AS2 error: %s\n", buf);
}

void GASEnvironment::LogScriptWarning(const char* fmt, ...) const     // 2012 0x9e4a30
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf("DISHONORED(bringup): AS2 warning: %s\n", buf);
}

// ---------------------------------------------------------------------------------------------
// GASGlobalContext

static const char* GFxAS2BuiltinText[GASbuiltin_COUNT] =
{
    "", "undefined", "null", "true", "false", "NaN", "Infinity", "-Infinity", "0",
    "__proto__", "__constructor__", "__resolve", "prototype", "constructor", "toString", "valueOf",
    "length",
    "this", "super", "_global", "_root", "_parent", "_level0", "arguments", "callee", "caller",
    "apply", "call",
    "Object", "Array", "String", "Number", "Boolean", "Function", "Math", "MovieClip", "Error",
    "object", "movieclip", "function", "string", "number", "boolean",
    "onLoad", "onEnterFrame", "onUnload",
    "_x", "_y", "_visible", "_alpha", "_name", "_target", "_currentframe", "_totalframes",
    "_width", "_height", "_xscale", "_yscale", "_rotation"
};

GASGlobalContext::GASGlobalContext(GFxMovieRoot* root, unsigned int swfVersion)
    : pMovieRoot(root), pGlobal(0), bGFxExtensions(false), Classes(0), ClassCount(0),
      ClassCapacity(0)
{
    SC.pStrings = &Strings;
    SC.pContext = this;
    SC.Version = swfVersion;
    for (int i = 0; i < Proto_COUNT; ++i)
    {
        Prototypes[i] = 0;
        Constructors[i] = 0;
    }
    InitBuiltinStrings();
    InitStandardLibrary();
}

GASGlobalContext::~GASGlobalContext()
{
    free(Classes);
    // Every GASObject, prototype and constructor is owned by Collector, so one call frees the graph.
    Collector.FreeAll();
}

void GASGlobalContext::InitBuiltinStrings()
{
    for (int i = 0; i < GASbuiltin_COUNT; ++i)
        Builtins[i] = Strings.CreateString(GFxAS2BuiltinText[i]);
}

GASObject* GASGlobalContext::NewObject()
{
    return new GASObject(&SC, Prototypes[Proto_Object]);
}

GASArrayObject* GASGlobalContext::NewArray()
{
    return new GASArrayObject(&SC, Prototypes[Proto_Array]);
}

GASFunctionObject* GASGlobalContext::NewCFunction(GASCFunctionPtr fn)
{
    GASFunctionObject* f = new GASFunctionObject(&SC, Prototypes[Proto_Function]);
    f->pCFunction = fn;
    return f;
}

GASFunctionObject* GASGlobalContext::NewFunction()
{
    return new GASFunctionObject(&SC, Prototypes[Proto_Function]);
}

void GASGlobalContext::RegisterClass(const GASString& symbol, GASFunctionObject* ctor)
{
    for (unsigned int i = 0; i < ClassCount; ++i)
        if (Classes[i].Symbol == symbol)
        {
            Classes[i].pCtor = ctor;
            return;
        }
    if (ClassCount >= ClassCapacity)
    {
        unsigned int cap = ClassCapacity ? ClassCapacity * 2 : 32;
        ClassEntry* next = (ClassEntry*)calloc(cap, sizeof(ClassEntry));
        for (unsigned int i = 0; i < ClassCount; ++i)
        {
            next[i].Symbol = Classes[i].Symbol;
            next[i].pCtor = Classes[i].pCtor;
        }
        free(Classes);
        Classes = next;
        ClassCapacity = cap;
    }
    Classes[ClassCount].Symbol = symbol;
    Classes[ClassCount].pCtor = ctor;
    ++ClassCount;
}

GASFunctionObject* GASGlobalContext::FindRegisteredClass(const GASString& symbol) const
{
    for (unsigned int i = 0; i < ClassCount; ++i)
        if (Classes[i].Symbol == symbol)
            return Classes[i].pCtor;
    return 0;
}
