// Scaleform GFx 3.3.89 - the AS2 class library subset. Package BC.
//
// Scope, and why this subset: the content in the cook is 669 DoInitAction tags of __Packages.* class
// registrations (agentBB.md 3.5), and the main menu's own asset carries 45 of them. What that code
// needs to run is not the whole AS2 library - it is Object with registerClass and addProperty,
// Function with call and apply, Array, String, Number, Boolean, Math, Error, MovieClip's timeline
// methods and the free functions ASSetPropFlags, trace, parseInt/parseFloat, isNaN, setInterval.
// Those are what this file installs. Everything else retail has - Date (68 functions in GASDate.obj),
// Key, Mouse, Selection, TextField, TextFormat, Stage, Color, Sound, XML, LoadVars, BitmapData, the
// filter classes, the CLIK helpers - is named in resources/docs/agents/agentBC.md as remaining, with
// its retail function count, rather than half-written here.
//
// Every class is installed the way retail's GASGlobalContext::AddBuiltinClassRegistry does it: a
// constructor function on _global, a prototype object on the constructor, and each method on the
// prototype with PropFlag_DontEnum set so `for..in` over an instance does not see the library.
// DISHONORED(port): see GFxAS2.h.
#include "GFxAS2Runtime.h"
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

unsigned int GFxAS2GetTimerMs()
{
    // A movie asks for a monotonic millisecond count; the CRT clock is enough and keeps this file
    // free of any platform header.
    return (unsigned int)((double)clock() * 1000.0 / (double)CLOCKS_PER_SEC);
}

namespace
{

GASGlobalContext* ContextOf(const GASFnCall& fn) { return fn.pEnv->GetGC(); }

void AddMethod(GASObject* obj, GASGlobalContext* gc, const char* name, GASCFunctionPtr f)
{
    GASValue v;
    v.SetAsFunction(gc->NewCFunction(f));
    obj->SetConstMemberRaw(gc->GetSC(), name, v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

void AddNumber(GASObject* obj, GASGlobalContext* gc, const char* name, double d)
{
    GASValue v;
    v.SetNumber(d);
    obj->SetConstMemberRaw(gc->GetSC(), name, v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

GASArrayObject* ArrayOf(const GASFnCall& fn)
{
    GASObjectInterface* oi = fn.pThis;
    if (oi == 0)
        return 0;
    GASObject* o = oi->ToASObject();
    if (o && o->GetObjectType() == Object_Array)
        return (GASArrayObject*)o;
    return 0;
}

// --- Object ---------------------------------------------------------------------------------

void ObjectCtor(const GASFnCall& fn)
{
    if (fn.pResult) fn.pResult->SetUndefined();
}

void ObjectToString(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetString(fn.pEnv->CreateString("[object Object]"));
}

void ObjectValueOf(const GASFnCall& fn)
{
    if (fn.pResult && fn.pThis)
    {
        GASObject* o = fn.pThis->ToASObject();
        if (o) fn.pResult->SetAsObject(o);
        else fn.pResult->SetUndefined();
    }
}

void ObjectHasOwnProperty(const GASFnCall& fn)
{
    bool has = false;
    if (fn.GetNumArgs() >= 1 && fn.pThis)
        has = fn.pThis->HasMember(fn.pEnv->GetSC(), fn.Arg(0).ToString(fn.pEnv), false);
    if (fn.pResult) fn.pResult->SetBool(has);
}

void ObjectIsPropertyEnumerable(const GASFnCall& fn)
{
    GASMember m;
    bool ok = false;
    if (fn.GetNumArgs() >= 1 && fn.pThis
        && fn.pThis->FindMember(fn.pEnv->GetSC(), fn.Arg(0).ToString(fn.pEnv), &m))
        ok = !m.Flags.GetDontEnum();
    if (fn.pResult) fn.pResult->SetBool(ok);
}

void ObjectIsPrototypeOf(const GASFnCall& fn)
{
    bool ok = false;
    if (fn.GetNumArgs() >= 1 && fn.pThis)
    {
        GASObjectInterface* other = fn.Arg(0).ToObjectInterface(fn.pEnv);
        GASObject* self = fn.pThis->ToASObject();
        if (other && self)
            ok = other->InstanceOf(fn.pEnv, self, true);
    }
    if (fn.pResult) fn.pResult->SetBool(ok);
}

// Object.prototype.addProperty(name, getter, setter): this is how every CLIK class in the cook
// exposes a property, so a PROPERTY member is the common case rather than an exotic one.
void ObjectAddProperty(const GASFnCall& fn)
{
    if (fn.GetNumArgs() < 3 || fn.pThis == 0)
    {
        if (fn.pResult) fn.pResult->SetBool(false);
        return;
    }
    GASString name = fn.Arg(0).ToString(fn.pEnv);
    GASValue prop;
    prop.SetProperty(fn.Arg(1).GetFunction(), fn.Arg(2).GetFunction());
    fn.pThis->SetMemberRaw(fn.pEnv->GetSC(), name, prop,
                           GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    if (fn.pResult) fn.pResult->SetBool(true);
}

// Object.registerClass(symbolName, ctor): the bridge between an exported library symbol and an AS2
// class. Every __Packages registration in the cook ends with one of these, and it is what makes an
// AttachMovie of that symbol construct the class rather than a bare movie clip.
void ObjectRegisterClass(const GASFnCall& fn)
{
    if (fn.GetNumArgs() < 2)
    {
        if (fn.pResult) fn.pResult->SetBool(false);
        return;
    }
    GASString symbol = fn.Arg(0).ToString(fn.pEnv);
    GASFunctionObject* ctor = fn.Arg(1).GetFunction();
    ContextOf(fn)->RegisterClass(symbol, ctor);
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    if (root)
        ++root->GetCensus().ClassesRegistered;
    if (fn.pResult) fn.pResult->SetBool(true);
}

// --- Function -------------------------------------------------------------------------------

void FunctionCall(const GASFnCall& fn)
{
    GASFunctionObject* self = fn.pThis ? fn.pThis->ToFunction() : 0;
    if (self == 0)
        return;
    GASObjectInterface* newThis = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToObjectInterface(fn.pEnv) : 0;
    int nargs = fn.GetNumArgs() > 0 ? fn.GetNumArgs() - 1 : 0;
    // The arguments are already contiguous on the stack; skipping the first one is a shift of the
    // window rather than a copy, which is what retail does too.
    GASFnCall inner(fn.pResult, newThis, fn.pEnv, nargs, fn.FirstArgBottomIndex - 1);
    self->Invoke(inner);
}

void FunctionApply(const GASFnCall& fn)
{
    GASFunctionObject* self = fn.pThis ? fn.pThis->ToFunction() : 0;
    if (self == 0)
        return;
    GASObjectInterface* newThis = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToObjectInterface(fn.pEnv) : 0;
    GASArrayObject* args = 0;
    if (fn.GetNumArgs() >= 2)
    {
        GASObject* o = fn.Arg(1).ToObject(fn.pEnv);
        if (o && o->GetObjectType() == Object_Array)
            args = (GASArrayObject*)o;
    }
    int n = args ? (int)args->GetSize() : 0;
    for (int i = n - 1; i >= 0; --i)
        fn.pEnv->Push(*args->GetElementPtr((unsigned int)i));
    GASFnCall inner(fn.pResult, newThis, fn.pEnv, n, fn.pEnv->GetTopIndex());
    self->Invoke(inner);
    fn.pEnv->Drop(n);
}

// --- Array ----------------------------------------------------------------------------------

void ArrayCtor(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0)
    {
        // Called as a function rather than with `new`: AS2 returns a new array.
        arr = ContextOf(fn)->NewArray();
        if (fn.pResult) fn.pResult->SetAsObject(arr);
    }
    if (fn.GetNumArgs() == 1 && fn.Arg(0).IsNumber())
        arr->Resize((unsigned int)fn.Arg(0).GetNumber());
    else
        for (int i = 0; i < fn.GetNumArgs(); ++i)
            arr->PushBack(fn.Arg(i));
}

void ArrayPush(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    for (int i = 0; i < fn.GetNumArgs(); ++i)
        arr->PushBack(fn.Arg(i));
    if (fn.pResult) fn.pResult->SetInt((int)arr->GetSize());
}

void ArrayPop(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0 || arr->GetSize() == 0) return;
    if (fn.pResult) *fn.pResult = *arr->GetElementPtr(arr->GetSize() - 1);
    arr->Resize(arr->GetSize() - 1);
}

void ArrayShift(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0 || arr->GetSize() == 0) return;
    if (fn.pResult) *fn.pResult = *arr->GetElementPtr(0);
    arr->RemoveElements(0, 1);
}

void ArrayUnshift(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    int n = fn.GetNumArgs();
    unsigned int old = arr->GetSize();
    arr->Resize(old + (unsigned int)n);
    for (int i = (int)old - 1; i >= 0; --i)
        arr->SetElement((unsigned int)(i + n), *arr->GetElementPtr((unsigned int)i));
    for (int i = 0; i < n; ++i)
        arr->SetElement((unsigned int)i, fn.Arg(i));
    if (fn.pResult) fn.pResult->SetInt((int)arr->GetSize());
}

void ArraySplice(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0 || fn.GetNumArgs() < 1) return;
    int start = fn.Arg(0).ToInt32(fn.pEnv);
    int size = (int)arr->GetSize();
    if (start < 0) start += size;
    if (start < 0) start = 0;
    if (start > size) start = size;
    int count = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : size - start;
    if (count < 0) count = 0;
    if (start + count > size) count = size - start;

    GASArrayObject* removed = ContextOf(fn)->NewArray();
    for (int i = 0; i < count; ++i)
        removed->PushBack(*arr->GetElementPtr((unsigned int)(start + i)));
    arr->RemoveElements((unsigned int)start, count);
    for (int i = fn.GetNumArgs() - 1; i >= 2; --i)
    {
        arr->Resize(arr->GetSize() + 1);
        for (int j = (int)arr->GetSize() - 1; j > start; --j)
            arr->SetElement((unsigned int)j, *arr->GetElementPtr((unsigned int)(j - 1)));
        arr->SetElement((unsigned int)start, fn.Arg(i));
    }
    if (fn.pResult) fn.pResult->SetAsObject(removed);
}

void ArraySlice(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    int size = (int)arr->GetSize();
    int start = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    int end = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : size;
    if (start < 0) start += size;
    if (end < 0) end += size;
    if (start < 0) start = 0;
    if (end > size) end = size;
    GASArrayObject* out = ContextOf(fn)->NewArray();
    for (int i = start; i < end; ++i)
        out->PushBack(*arr->GetElementPtr((unsigned int)i));
    if (fn.pResult) fn.pResult->SetAsObject(out);
}

void ArrayConcat(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    GASArrayObject* out = ContextOf(fn)->NewArray();
    if (arr)
        for (unsigned int i = 0; i < arr->GetSize(); ++i)
            out->PushBack(*arr->GetElementPtr(i));
    for (int a = 0; a < fn.GetNumArgs(); ++a)
    {
        GASObject* o = fn.Arg(a).ToObject(fn.pEnv);
        if (o && o->GetObjectType() == Object_Array)
        {
            GASArrayObject* other = (GASArrayObject*)o;
            for (unsigned int i = 0; i < other->GetSize(); ++i)
                out->PushBack(*other->GetElementPtr(i));
        }
        else
        {
            out->PushBack(fn.Arg(a));
        }
    }
    if (fn.pResult) fn.pResult->SetAsObject(out);
}

void ArrayJoin(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    GASString sep = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToString(fn.pEnv)
                                         : fn.pEnv->CreateString(",");
    unsigned int total = 1;
    for (unsigned int i = 0; i < arr->GetSize(); ++i)
        total += arr->GetElementPtr(i)->ToString(fn.pEnv).GetSize() + sep.GetSize();
    char* buf = (char*)malloc(total + 1);
    buf[0] = 0;
    for (unsigned int i = 0; i < arr->GetSize(); ++i)
    {
        if (i) strcat(buf, sep.ToCStr());
        strcat(buf, arr->GetElementPtr(i)->ToString(fn.pEnv).ToCStr());
    }
    if (fn.pResult) fn.pResult->SetString(fn.pEnv->CreateString(buf));
    free(buf);
}

void ArrayReverse(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    unsigned int n = arr->GetSize();
    for (unsigned int i = 0; i < n / 2; ++i)
    {
        GASValue a = *arr->GetElementPtr(i);
        GASValue b = *arr->GetElementPtr(n - 1 - i);
        arr->SetElement(i, b);
        arr->SetElement(n - 1 - i, a);
    }
    if (fn.pResult) fn.pResult->SetAsObject(arr);
}

void ArrayIndexOf(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    int found = -1;
    if (arr && fn.GetNumArgs() >= 1)
        for (unsigned int i = 0; i < arr->GetSize(); ++i)
            if (arr->GetElementPtr(i)->IsStrictEqual(fn.pEnv, fn.Arg(0)))
            {
                found = (int)i;
                break;
            }
    if (fn.pResult) fn.pResult->SetInt(found);
}

void ArraySort(const GASFnCall& fn)
{
    GASArrayObject* arr = ArrayOf(fn);
    if (arr == 0) return;
    GASFunctionObject* cmp = fn.GetNumArgs() >= 1 ? fn.Arg(0).GetFunction() : 0;
    // An insertion sort: the arrays a UI sorts are small, and a comparator that re-enters the
    // interpreter must not be called from inside a qsort callback that holds the environment stack.
    unsigned int n = arr->GetSize();
    for (unsigned int i = 1; i < n; ++i)
    {
        GASValue key = *arr->GetElementPtr(i);
        unsigned int j = i;
        while (j > 0)
        {
            GASValue prev = *arr->GetElementPtr(j - 1);
            int order;
            if (cmp)
            {
                GASValue r;
                fn.pEnv->Push(key);
                fn.pEnv->Push(prev);
                GASFnCall c(&r, 0, fn.pEnv, 2, fn.pEnv->GetTopIndex());
                cmp->Invoke(c);
                fn.pEnv->Drop(2);
                order = r.ToInt32(fn.pEnv);
            }
            else
            {
                order = strcmp(key.ToString(fn.pEnv).ToCStr(), prev.ToString(fn.pEnv).ToCStr());
            }
            if (order >= 0)
                break;
            arr->SetElement(j, prev);
            --j;
        }
        arr->SetElement(j, key);
    }
    if (fn.pResult) fn.pResult->SetAsObject(arr);
}

void ArrayToString(const GASFnCall& fn) { ArrayJoin(fn); }

// --- String ---------------------------------------------------------------------------------

GASString ThisString(const GASFnCall& fn)
{
    if (fn.pThis)
    {
        GASValue v = fn.pThis->GetValue();
        if (v.IsString())
            return v.GetString();
    }
    return fn.pEnv->CreateString("");
}

void StringCtor(const GASFnCall& fn)
{
    if (fn.pResult)
    {
        if (fn.GetNumArgs() >= 1) fn.pResult->SetString(fn.Arg(0).ToString(fn.pEnv));
        else fn.pResult->SetString(fn.pEnv->CreateString(""));
    }
}

void StringFromCharCode(const GASFnCall& fn)
{
    char buf[64];
    int n = fn.GetNumArgs() < 63 ? fn.GetNumArgs() : 63;
    for (int i = 0; i < n; ++i)
        buf[i] = (char)fn.Arg(i).ToInt32(fn.pEnv);
    buf[n] = 0;
    if (fn.pResult) fn.pResult->SetString(fn.pEnv->CreateString(buf, (unsigned int)n));
}

void StringToString(const GASFnCall& fn)
{
    if (fn.pResult) fn.pResult->SetString(ThisString(fn));
}

void StringValueOf(const GASFnCall& fn) { StringToString(fn); }

void StringCharAt(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    int i = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    if (fn.pResult == 0) return;
    if (i < 0 || (unsigned int)i >= s.GetSize())
    {
        fn.pResult->SetString(fn.pEnv->CreateString(""));
        return;
    }
    fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(s.ToCStr() + i, 1));
}

void StringCharCodeAt(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    int i = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    if (fn.pResult == 0) return;
    if (i < 0 || (unsigned int)i >= s.GetSize()) fn.pResult->SetNumber(GFxAS2NaN());
    else fn.pResult->SetInt((int)(unsigned char)s.ToCStr()[i]);
}

void StringIndexOf(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    if (fn.pResult == 0) return;
    if (fn.GetNumArgs() < 1) { fn.pResult->SetInt(-1); return; }
    GASString needle = fn.Arg(0).ToString(fn.pEnv);
    int from = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : 0;
    if (from < 0) from = 0;
    if ((unsigned int)from > s.GetSize()) { fn.pResult->SetInt(-1); return; }
    const char* hit = strstr(s.ToCStr() + from, needle.ToCStr());
    fn.pResult->SetInt(hit ? (int)(hit - s.ToCStr()) : -1);
}

void StringLastIndexOf(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    if (fn.pResult == 0) return;
    if (fn.GetNumArgs() < 1) { fn.pResult->SetInt(-1); return; }
    GASString needle = fn.Arg(0).ToString(fn.pEnv);
    int found = -1;
    const char* p = s.ToCStr();
    for (const char* hit = strstr(p, needle.ToCStr()); hit; hit = strstr(hit + 1, needle.ToCStr()))
        found = (int)(hit - p);
    fn.pResult->SetInt(found);
}

void StringSubstring(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    int len = (int)s.GetSize();
    int a = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    int b = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : len;
    if (a < 0) a = 0;
    if (a > len) a = len;
    if (b < 0) b = 0;
    if (b > len) b = len;
    if (a > b) { int t = a; a = b; b = t; }
    if (fn.pResult)
        fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(s.ToCStr() + a, (unsigned int)(b - a)));
}

void StringSubstr(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    int len = (int)s.GetSize();
    int a = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    if (a < 0) a += len;
    if (a < 0) a = 0;
    if (a > len) a = len;
    int n = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : len - a;
    if (n < 0) n = 0;
    if (a + n > len) n = len - a;
    if (fn.pResult)
        fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(s.ToCStr() + a, (unsigned int)n));
}

void StringSlice(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    int len = (int)s.GetSize();
    int a = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToInt32(fn.pEnv) : 0;
    int b = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : len;
    if (a < 0) a += len;
    if (b < 0) b += len;
    if (a < 0) a = 0;
    if (a > len) a = len;
    if (b < 0) b = 0;
    if (b > len) b = len;
    if (b < a) b = a;
    if (fn.pResult)
        fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(s.ToCStr() + a, (unsigned int)(b - a)));
}

void StringSplit(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    GASArrayObject* out = ContextOf(fn)->NewArray();
    GASString sep = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToString(fn.pEnv) : fn.pEnv->CreateString("");
    GASValue v;
    if (sep.GetSize() == 0)
    {
        for (unsigned int i = 0; i < s.GetSize(); ++i)
        {
            v.SetString(fn.pEnv->GetSC()->CreateString(s.ToCStr() + i, 1));
            out->PushBack(v);
        }
    }
    else
    {
        const char* p = s.ToCStr();
        for (;;)
        {
            const char* hit = strstr(p, sep.ToCStr());
            if (hit == 0)
            {
                v.SetString(fn.pEnv->CreateString(p));
                out->PushBack(v);
                break;
            }
            v.SetString(fn.pEnv->GetSC()->CreateString(p, (unsigned int)(hit - p)));
            out->PushBack(v);
            p = hit + sep.GetSize();
        }
    }
    if (fn.pResult) fn.pResult->SetAsObject(out);
}

void StringToUpperCase(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    char stack[256];
    char* buf = s.GetSize() < sizeof(stack) ? stack : (char*)malloc(s.GetSize() + 1);
    for (unsigned int i = 0; i < s.GetSize(); ++i)
    {
        char c = s.ToCStr()[i];
        buf[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
    }
    buf[s.GetSize()] = 0;
    if (fn.pResult) fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(buf, s.GetSize()));
    if (buf != stack) free(buf);
}

void StringToLowerCase(const GASFnCall& fn)
{
    GASString s = ThisString(fn);
    char stack[256];
    char* buf = s.GetSize() < sizeof(stack) ? stack : (char*)malloc(s.GetSize() + 1);
    for (unsigned int i = 0; i < s.GetSize(); ++i)
    {
        char c = s.ToCStr()[i];
        buf[i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }
    buf[s.GetSize()] = 0;
    if (fn.pResult) fn.pResult->SetString(fn.pEnv->GetSC()->CreateString(buf, s.GetSize()));
    if (buf != stack) free(buf);
}

void NumberToString(const GASFnCall& fn)
{
    if (fn.pResult == 0) return;
    GASValue v = fn.pThis ? fn.pThis->GetValue() : GASValue();
    fn.pResult->SetString(v.ToString(fn.pEnv));
}

void NumberValueOf(const GASFnCall& fn)
{
    if (fn.pResult == 0) return;
    GASValue v = fn.pThis ? fn.pThis->GetValue() : GASValue();
    fn.pResult->SetNumber(v.ToNumber(fn.pEnv));
}

// --- Number / Boolean -----------------------------------------------------------------------

void NumberCtor(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetNumber(fn.GetNumArgs() >= 1 ? fn.Arg(0).ToNumber(fn.pEnv) : 0.0);
}

void BooleanCtor(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetBool(fn.GetNumArgs() >= 1 ? fn.Arg(0).ToBool(fn.pEnv) : false);
}

// --- Math -----------------------------------------------------------------------------------

double Arg0(const GASFnCall& fn) { return fn.GetNumArgs() >= 1 ? fn.Arg(0).ToNumber(fn.pEnv) : GFxAS2NaN(); }
double Arg1(const GASFnCall& fn) { return fn.GetNumArgs() >= 2 ? fn.Arg(1).ToNumber(fn.pEnv) : GFxAS2NaN(); }

void MathAbs(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(fabs(Arg0(fn))); }
void MathCeil(const GASFnCall& fn)  { if (fn.pResult) fn.pResult->SetNumber(ceil(Arg0(fn))); }
void MathFloor(const GASFnCall& fn) { if (fn.pResult) fn.pResult->SetNumber(floor(Arg0(fn))); }
void MathRound(const GASFnCall& fn) { if (fn.pResult) fn.pResult->SetNumber(floor(Arg0(fn) + 0.5)); }
void MathSqrt(const GASFnCall& fn)  { if (fn.pResult) fn.pResult->SetNumber(sqrt(Arg0(fn))); }
void MathSin(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(sin(Arg0(fn))); }
void MathCos(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(cos(Arg0(fn))); }
void MathTan(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(tan(Arg0(fn))); }
void MathAsin(const GASFnCall& fn)  { if (fn.pResult) fn.pResult->SetNumber(asin(Arg0(fn))); }
void MathAcos(const GASFnCall& fn)  { if (fn.pResult) fn.pResult->SetNumber(acos(Arg0(fn))); }
void MathAtan(const GASFnCall& fn)  { if (fn.pResult) fn.pResult->SetNumber(atan(Arg0(fn))); }
void MathExp(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(exp(Arg0(fn))); }
void MathLog(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(log(Arg0(fn))); }
void MathAtan2(const GASFnCall& fn) { if (fn.pResult) fn.pResult->SetNumber(atan2(Arg0(fn), Arg1(fn))); }
void MathPow(const GASFnCall& fn)   { if (fn.pResult) fn.pResult->SetNumber(pow(Arg0(fn), Arg1(fn))); }
void MathRandom(const GASFnCall& fn){ if (fn.pResult) fn.pResult->SetNumber((double)rand() / ((double)RAND_MAX + 1.0)); }

void MathMin(const GASFnCall& fn)
{
    double best = GFxAS2Infinity();
    for (int i = 0; i < fn.GetNumArgs(); ++i)
    {
        double d = fn.Arg(i).ToNumber(fn.pEnv);
        if (GFxAS2IsNaN(d)) { best = d; break; }
        if (d < best) best = d;
    }
    if (fn.pResult) fn.pResult->SetNumber(best);
}

void MathMax(const GASFnCall& fn)
{
    double best = -GFxAS2Infinity();
    for (int i = 0; i < fn.GetNumArgs(); ++i)
    {
        double d = fn.Arg(i).ToNumber(fn.pEnv);
        if (GFxAS2IsNaN(d)) { best = d; break; }
        if (d > best) best = d;
    }
    if (fn.pResult) fn.pResult->SetNumber(best);
}

// --- MovieClip ------------------------------------------------------------------------------

GFxSprite* ThisSprite(const GASFnCall& fn)
{
    return fn.pThis ? fn.pThis->ToSprite() : 0;
}

void MCPlay(const GASFnCall& fn) { GFxSprite* s = ThisSprite(fn); if (s) s->SetPlaying(true); }
void MCStop(const GASFnCall& fn) { GFxSprite* s = ThisSprite(fn); if (s) s->SetPlaying(false); }

void MCGotoAndPlay(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s == 0 || fn.GetNumArgs() < 1) return;
    if (fn.Arg(0).IsString())
        s->GotoLabeledFrame(fn.Arg(0).GetString().ToCStr(), 0);
    else
        s->GotoFrame((unsigned int)fn.Arg(0).ToInt32(fn.pEnv) - 1);
    s->SetPlaying(true);
}

void MCGotoAndStop(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s == 0 || fn.GetNumArgs() < 1) return;
    if (fn.Arg(0).IsString())
        s->GotoLabeledFrame(fn.Arg(0).GetString().ToCStr(), 0);
    else
        s->GotoFrame((unsigned int)fn.Arg(0).ToInt32(fn.pEnv) - 1);
    s->SetPlaying(false);
}

void MCNextFrame(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s) s->GotoFrame(s->GetCurrentFrame() + 1);
}

void MCPrevFrame(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s && s->GetCurrentFrame() > 0) s->GotoFrame(s->GetCurrentFrame() - 1);
}

void MCCreateEmptyMovieClip(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s == 0 || fn.GetNumArgs() < 2) return;
    GFxSprite* child = s->CreateEmptyMovieClip(fn.Arg(0).ToString(fn.pEnv),
                                               fn.Arg(1).ToInt32(fn.pEnv));
    if (fn.pResult) fn.pResult->SetAsCharacter(child);
}

void MCAttachMovie(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s == 0 || fn.GetNumArgs() < 3) return;
    GFxSprite* child = s->AttachMovie(fn.Arg(0).ToString(fn.pEnv), fn.Arg(1).ToString(fn.pEnv),
                                      fn.Arg(2).ToInt32(fn.pEnv));
    if (child && fn.GetNumArgs() >= 4)
    {
        // The init object's members are copied onto the new clip before its first frame runs, which
        // is how the CLIK widgets are configured.
        GASObject* init = fn.Arg(3).ToObject(fn.pEnv);
        if (init)
        {
            struct Copier : public GASObjectInterface::MemberVisitor
            {
                GFxSprite* pTo;
                GASEnvironment* pEnv;
                virtual void Visit(const GASString& name, const GASValue& val, unsigned char f)
                {
                    pTo->SetMember(pEnv, name, val, GASPropFlags());
                }
            } copier;
            copier.pTo = child;
            copier.pEnv = fn.pEnv;
            init->VisitMembers(fn.pEnv->GetSC(), &copier, 0, init);
        }
    }
    if (fn.pResult) fn.pResult->SetAsCharacter(child);
}

void MCRemoveMovieClip(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (s == 0 || s->GetParent() == 0) return;
    GFxSprite* parent = s->GetParent()->ToSprite();
    if (parent) parent->RemoveDisplayObject(s->GetDepth(), s->GetId());
}

void MCSwapDepths(const GASFnCall& fn)
{
    // Needs the depth-swap path of GFxDisplayList (2012 0x9d5c10), which is not in this wave.
    if (fn.pResult) fn.pResult->SetUndefined();
}

void MCGetNextHighestDepth(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (fn.pResult)
        fn.pResult->SetInt(s ? s->GetDisplayList().GetLargestDepthInUse() + 1 : 0);
}

void MCGetDepth(const GASFnCall& fn)
{
    GFxSprite* s = ThisSprite(fn);
    if (fn.pResult) fn.pResult->SetInt(s ? s->GetDepth() : 0);
}

void MCHitTest(const GASFnCall& fn)
{
    // Hit testing needs shape geometry, which arrives with the tessellator.
    if (fn.pResult) fn.pResult->SetBool(false);
}

// --- free functions -------------------------------------------------------------------------

void GlobalTrace(const GASFnCall& fn)
{
    GASString s = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToString(fn.pEnv) : fn.pEnv->CreateString("");
    printf("DISHONORED(bringup): AS2 trace: %s\n", s.ToCStr());
}

// ASSetPropFlags(object, names, setFlags, clearFlags). `names` null means every member. The class
// registrations use it on every class they install, which is why it is not optional.
void GlobalASSetPropFlags(const GASFnCall& fn)
{
    if (fn.GetNumArgs() < 3)
        return;
    GASObjectInterface* oi = fn.Arg(0).ToObjectInterface(fn.pEnv);
    if (oi == 0)
        return;
    unsigned char setFlags = (unsigned char)fn.Arg(2).ToInt32(fn.pEnv);
    unsigned char clearFlags = fn.GetNumArgs() >= 4
                                   ? (unsigned char)fn.Arg(3).ToInt32(fn.pEnv) : 0;

    struct Setter : public GASObjectInterface::MemberVisitor
    {
        GASObjectInterface* pObj;
        GASStringContext*   pSC;
        unsigned char       Set;
        unsigned char       Clear;
        virtual void Visit(const GASString& name, const GASValue& val, unsigned char flags)
        {
            unsigned char f = (unsigned char)((flags & ~Clear) | Set);
            pObj->SetMemberFlags(pSC, name, f);
        }
    };

    if (fn.Arg(1).IsNull() || fn.Arg(1).IsUndefined())
    {
        Setter s;
        s.pObj = oi;
        s.pSC = fn.pEnv->GetSC();
        s.Set = setFlags;
        s.Clear = clearFlags;
        oi->VisitMembers(fn.pEnv->GetSC(), &s, GASObjectInterface::VisitMember_DontEnum, oi);
        return;
    }

    // A comma-separated name list, which is the other form the content uses.
    GASString list = fn.Arg(1).ToString(fn.pEnv);
    const char* p = list.ToCStr();
    char name[128];
    while (*p)
    {
        unsigned int n = 0;
        while (*p && *p != ',' && n < sizeof(name) - 1)
            name[n++] = *p++;
        name[n] = 0;
        if (*p == ',') ++p;
        if (n == 0) continue;
        GASMember m;
        GASString ns = fn.pEnv->CreateString(name, n);
        unsigned char cur = 0;
        if (oi->FindMember(fn.pEnv->GetSC(), ns, &m))
            cur = m.Flags.Flags;
        oi->SetMemberFlags(fn.pEnv->GetSC(), ns, (unsigned char)((cur & ~clearFlags) | setFlags));
    }
}

void GlobalParseInt(const GASFnCall& fn)
{
    if (fn.pResult == 0) return;
    if (fn.GetNumArgs() < 1) { fn.pResult->SetNumber(GFxAS2NaN()); return; }
    GASString s = fn.Arg(0).ToString(fn.pEnv);
    int radix = fn.GetNumArgs() >= 2 ? fn.Arg(1).ToInt32(fn.pEnv) : 0;
    char* end = 0;
    long v = strtol(s.ToCStr(), &end, radix);
    if (end == s.ToCStr()) fn.pResult->SetNumber(GFxAS2NaN());
    else fn.pResult->SetNumber((double)v);
}

void GlobalParseFloat(const GASFnCall& fn)
{
    if (fn.pResult == 0) return;
    if (fn.GetNumArgs() < 1) { fn.pResult->SetNumber(GFxAS2NaN()); return; }
    GASString s = fn.Arg(0).ToString(fn.pEnv);
    char* end = 0;
    double v = strtod(s.ToCStr(), &end);
    if (end == s.ToCStr()) fn.pResult->SetNumber(GFxAS2NaN());
    else fn.pResult->SetNumber(v);
}

void GlobalIsNaN(const GASFnCall& fn)
{
    if (fn.pResult)
        fn.pResult->SetBool(fn.GetNumArgs() >= 1 ? GFxAS2IsNaN(fn.Arg(0).ToNumber(fn.pEnv)) : true);
}

void GlobalIsFinite(const GASFnCall& fn)
{
    if (fn.pResult == 0) return;
    double d = fn.GetNumArgs() >= 1 ? fn.Arg(0).ToNumber(fn.pEnv) : GFxAS2NaN();
    fn.pResult->SetBool(!GFxAS2IsNaN(d) && d != GFxAS2Infinity() && d != -GFxAS2Infinity());
}

void GlobalGetTimer(const GASFnCall& fn)
{
    if (fn.pResult) fn.pResult->SetNumber((double)GFxAS2GetTimerMs());
}

// setInterval / setTimeout need the movie root's timer list, which arrives with the input and
// advance path. They return a handle so the content's bookkeeping stays consistent.
void GlobalSetInterval(const GASFnCall& fn)
{
    static int nextHandle = 1;
    if (fn.pResult) fn.pResult->SetInt(nextHandle++);
}

void GlobalClearInterval(const GASFnCall& fn)
{
    if (fn.pResult) fn.pResult->SetUndefined();
}

void ErrorCtor(const GASFnCall& fn)
{
    if (fn.pThis && fn.GetNumArgs() >= 1)
        fn.pThis->SetConstMemberRaw(fn.pEnv->GetSC(), "message", fn.Arg(0));
}

} // namespace

// ---------------------------------------------------------------------------------------------

void GASGlobalContext::InitStandardLibrary()
{
    pGlobal = new GASObject(&SC);

    // Object and Function first, because every other prototype chains to Object.prototype and every
    // constructor is a Function. Retail's GASGlobalContext does the same ordering.
    Prototypes[Proto_Object] = new GASObject(&SC, 0);
    Prototypes[Proto_Function] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_Array] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_String] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_Number] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_Boolean] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_MovieClip] = new GASObject(&SC, Prototypes[Proto_Object]);
    Prototypes[Proto_Error] = new GASObject(&SC, Prototypes[Proto_Object]);

    AddMethod(Prototypes[Proto_Object], this, "toString", ObjectToString);
    AddMethod(Prototypes[Proto_Object], this, "valueOf", ObjectValueOf);
    AddMethod(Prototypes[Proto_Object], this, "hasOwnProperty", ObjectHasOwnProperty);
    AddMethod(Prototypes[Proto_Object], this, "isPropertyEnumerable", ObjectIsPropertyEnumerable);
    AddMethod(Prototypes[Proto_Object], this, "isPrototypeOf", ObjectIsPrototypeOf);
    AddMethod(Prototypes[Proto_Object], this, "addProperty", ObjectAddProperty);

    AddMethod(Prototypes[Proto_Function], this, "call", FunctionCall);
    AddMethod(Prototypes[Proto_Function], this, "apply", FunctionApply);

    AddMethod(Prototypes[Proto_Array], this, "push", ArrayPush);
    AddMethod(Prototypes[Proto_Array], this, "pop", ArrayPop);
    AddMethod(Prototypes[Proto_Array], this, "shift", ArrayShift);
    AddMethod(Prototypes[Proto_Array], this, "unshift", ArrayUnshift);
    AddMethod(Prototypes[Proto_Array], this, "splice", ArraySplice);
    AddMethod(Prototypes[Proto_Array], this, "slice", ArraySlice);
    AddMethod(Prototypes[Proto_Array], this, "concat", ArrayConcat);
    AddMethod(Prototypes[Proto_Array], this, "join", ArrayJoin);
    AddMethod(Prototypes[Proto_Array], this, "reverse", ArrayReverse);
    AddMethod(Prototypes[Proto_Array], this, "indexOf", ArrayIndexOf);
    AddMethod(Prototypes[Proto_Array], this, "sort", ArraySort);
    AddMethod(Prototypes[Proto_Array], this, "toString", ArrayToString);

    AddMethod(Prototypes[Proto_String], this, "toString", StringToString);
    AddMethod(Prototypes[Proto_String], this, "valueOf", StringValueOf);
    AddMethod(Prototypes[Proto_String], this, "charAt", StringCharAt);
    AddMethod(Prototypes[Proto_String], this, "charCodeAt", StringCharCodeAt);
    AddMethod(Prototypes[Proto_String], this, "indexOf", StringIndexOf);
    AddMethod(Prototypes[Proto_String], this, "lastIndexOf", StringLastIndexOf);
    AddMethod(Prototypes[Proto_String], this, "substring", StringSubstring);
    AddMethod(Prototypes[Proto_String], this, "substr", StringSubstr);
    AddMethod(Prototypes[Proto_String], this, "slice", StringSlice);
    AddMethod(Prototypes[Proto_String], this, "split", StringSplit);
    AddMethod(Prototypes[Proto_String], this, "toUpperCase", StringToUpperCase);
    AddMethod(Prototypes[Proto_String], this, "toLowerCase", StringToLowerCase);

    AddMethod(Prototypes[Proto_Number], this, "toString", NumberToString);
    AddMethod(Prototypes[Proto_Number], this, "valueOf", NumberValueOf);
    AddMethod(Prototypes[Proto_Boolean], this, "toString", NumberToString);
    AddMethod(Prototypes[Proto_Boolean], this, "valueOf", NumberValueOf);

    AddMethod(Prototypes[Proto_MovieClip], this, "play", MCPlay);
    AddMethod(Prototypes[Proto_MovieClip], this, "stop", MCStop);
    AddMethod(Prototypes[Proto_MovieClip], this, "gotoAndPlay", MCGotoAndPlay);
    AddMethod(Prototypes[Proto_MovieClip], this, "gotoAndStop", MCGotoAndStop);
    AddMethod(Prototypes[Proto_MovieClip], this, "nextFrame", MCNextFrame);
    AddMethod(Prototypes[Proto_MovieClip], this, "prevFrame", MCPrevFrame);
    AddMethod(Prototypes[Proto_MovieClip], this, "createEmptyMovieClip", MCCreateEmptyMovieClip);
    AddMethod(Prototypes[Proto_MovieClip], this, "attachMovie", MCAttachMovie);
    AddMethod(Prototypes[Proto_MovieClip], this, "removeMovieClip", MCRemoveMovieClip);
    AddMethod(Prototypes[Proto_MovieClip], this, "swapDepths", MCSwapDepths);
    AddMethod(Prototypes[Proto_MovieClip], this, "getDepth", MCGetDepth);
    AddMethod(Prototypes[Proto_MovieClip], this, "getNextHighestDepth", MCGetNextHighestDepth);
    AddMethod(Prototypes[Proto_MovieClip], this, "hitTest", MCHitTest);

    // The constructors, each with its prototype and each on _global.
    struct ClassInstall { const char* Name; ProtoId Id; GASCFunctionPtr Ctor; };
    static const ClassInstall installs[] =
    {
        { "Object",    Proto_Object,    ObjectCtor },
        { "Function",  Proto_Function,  ObjectCtor },
        { "Array",     Proto_Array,     ArrayCtor },
        { "String",    Proto_String,    StringCtor },
        { "Number",    Proto_Number,    NumberCtor },
        { "Boolean",   Proto_Boolean,   BooleanCtor },
        { "MovieClip", Proto_MovieClip, ObjectCtor },
        { "Error",     Proto_Error,     ErrorCtor }
    };
    for (int i = 0; i < (int)(sizeof(installs) / sizeof(installs[0])); ++i)
    {
        GASFunctionObject* ctor = NewCFunction(installs[i].Ctor);
        GASValue protoVal;
        protoVal.SetAsObject(Prototypes[installs[i].Id]);
        ctor->SetConstMemberRaw(&SC, "prototype", protoVal,
                                GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASValue ctorVal;
        ctorVal.SetAsFunction(ctor);
        Prototypes[installs[i].Id]->SetMemberRaw(&SC, Builtins[GASbuiltin_constructorUS], ctorVal,
                                                 GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        pGlobal->SetConstMemberRaw(&SC, installs[i].Name, ctorVal,
                                   GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        Constructors[installs[i].Id] = ctor;
    }

    // The statics that hang off a constructor rather than a prototype.
    {
        GASValue objectCtor;
        pGlobal->GetConstMemberRaw(&SC, "Object", &objectCtor);
        GASFunctionObject* oc = objectCtor.GetFunction();
        if (oc)
            AddMethod(oc, this, "registerClass", ObjectRegisterClass);

        GASValue stringCtor;
        pGlobal->GetConstMemberRaw(&SC, "String", &stringCtor);
        GASFunctionObject* strc = stringCtor.GetFunction();
        if (strc)
            AddMethod(strc, this, "fromCharCode", StringFromCharCode);

        GASValue numberCtor;
        pGlobal->GetConstMemberRaw(&SC, "Number", &numberCtor);
        GASFunctionObject* nc = numberCtor.GetFunction();
        if (nc)
        {
            AddNumber(nc, this, "MAX_VALUE", 1.7976931348623157e308);
            AddNumber(nc, this, "MIN_VALUE", 5e-324);
            AddNumber(nc, this, "NaN", GFxAS2NaN());
            AddNumber(nc, this, "POSITIVE_INFINITY", GFxAS2Infinity());
            AddNumber(nc, this, "NEGATIVE_INFINITY", -GFxAS2Infinity());
        }
    }

    // Math is an object, not a class.
    {
        GASObject* math = new GASObject(&SC, Prototypes[Proto_Object]);
        AddMethod(math, this, "abs", MathAbs);
        AddMethod(math, this, "ceil", MathCeil);
        AddMethod(math, this, "floor", MathFloor);
        AddMethod(math, this, "round", MathRound);
        AddMethod(math, this, "sqrt", MathSqrt);
        AddMethod(math, this, "sin", MathSin);
        AddMethod(math, this, "cos", MathCos);
        AddMethod(math, this, "tan", MathTan);
        AddMethod(math, this, "asin", MathAsin);
        AddMethod(math, this, "acos", MathAcos);
        AddMethod(math, this, "atan", MathAtan);
        AddMethod(math, this, "atan2", MathAtan2);
        AddMethod(math, this, "exp", MathExp);
        AddMethod(math, this, "log", MathLog);
        AddMethod(math, this, "pow", MathPow);
        AddMethod(math, this, "min", MathMin);
        AddMethod(math, this, "max", MathMax);
        AddMethod(math, this, "random", MathRandom);
        AddNumber(math, this, "PI", 3.14159265358979323846);
        AddNumber(math, this, "E", 2.71828182845904523536);
        AddNumber(math, this, "LN2", 0.693147180559945309417);
        AddNumber(math, this, "LN10", 2.30258509299404568402);
        AddNumber(math, this, "LOG2E", 1.44269504088896340736);
        AddNumber(math, this, "LOG10E", 0.434294481903251827651);
        AddNumber(math, this, "SQRT2", 1.41421356237309504880);
        AddNumber(math, this, "SQRT1_2", 0.707106781186547524401);
        GASValue v;
        v.SetAsObject(math);
        pGlobal->SetConstMemberRaw(&SC, "Math", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    AddMethod(pGlobal, this, "trace", GlobalTrace);
    AddMethod(pGlobal, this, "ASSetPropFlags", GlobalASSetPropFlags);
    AddMethod(pGlobal, this, "parseInt", GlobalParseInt);
    AddMethod(pGlobal, this, "parseFloat", GlobalParseFloat);
    AddMethod(pGlobal, this, "isNaN", GlobalIsNaN);
    AddMethod(pGlobal, this, "isFinite", GlobalIsFinite);
    AddMethod(pGlobal, this, "getTimer", GlobalGetTimer);
    AddMethod(pGlobal, this, "setInterval", GlobalSetInterval);
    AddMethod(pGlobal, this, "setTimeout", GlobalSetInterval);
    AddMethod(pGlobal, this, "clearInterval", GlobalClearInterval);
    AddMethod(pGlobal, this, "clearTimeout", GlobalClearInterval);

    GASValue nanVal;
    nanVal.SetNumber(GFxAS2NaN());
    pGlobal->SetConstMemberRaw(&SC, "NaN", nanVal, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    GASValue infVal;
    infVal.SetNumber(GFxAS2Infinity());
    pGlobal->SetConstMemberRaw(&SC, "Infinity", infVal,
                               GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    GASValue globalVal;
    globalVal.SetAsObject(pGlobal);
    pGlobal->SetConstMemberRaw(&SC, "_global", globalVal,
                               GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}
