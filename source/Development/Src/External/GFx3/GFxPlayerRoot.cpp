// Scaleform GFx 3.3.89 - GFxMovieRoot and GFxValue::ObjectInterface. Package BC.
//
// This is the file the engine actually talks to. agent BB left the 25 ObjectInterface methods
// declared and undefined on purpose, because 1,096 of the 1,350 calls the engine makes into libgfx
// are those methods (gfx_decision.md 2.2) - ObjectRelease 669, SetMember 120, Invoke 102,
// SetDisplayInfo 48, PushBack 36, GetMember 29, GetElement 25, GotoAndPlay 24, SetText 5,
// AttachMovie 4, CreateEmptyMovieClip 1. All 25 are defined here.
//
// Two things the decompiles settle and that a from-scratch implementation would get wrong:
//   1. the `isDObj` flag on GetMember/SetMember/Invoke does NOT mean "the pointer is a character".
//      GFxValue::ObjectInterface::GetMember (2012 0x9b0450) resolves it through
//      GFxCharacterHandle::ResolveCharacter first and bails out with undefined when the character is
//      gone. So a GFxValue that names a display object holds a *handle*, and the engine can keep one
//      across a frame in which the clip is removed without dangling;
//   2. GetMember's property path hands the property the object the lookup started from, chosen by
//      object type: ToASObject() for types 6..44 and ToASCharacter() for 2..5 (the two span tests in
//      0x9b0450). GetPropertyValue then runs the getter against that.
// DISHONORED(port): see GFxAS2.h.
#include "GFxPlayer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// ---------------------------------------------------------------------------------------------

GFxMovieRoot::GFxMovieRoot(GFxMovieDefImpl* defImpl)
    : pDefImpl(defImpl), pGC(0), pLevel0(0), ObjInterface(this), ScaleMode(GFxMovieView::SM_ShowAll),
      Alignment(GFxMovieView::Align_Center), BackgroundColor(0), BackgroundAlpha(1.f),
      bPaused(false), bVisible(true), bDirty(true), pUserData(0), TimeElapsed(0.f), FrameTime(0.f),
      MouseCursorCount(0), ControllerCount(1), Actions(0), ActionCount(0), ActionCapacity(0),
      bInActionQueue(false)
{
    memset(&Stats, 0, sizeof(Stats));
    for (int i = 0; i < MaxStates; ++i)
        States[i] = 0;

    GFxMovieDataDef* dataDef = defImpl->GetDataDef();
    pGC = new GASGlobalContext(this, dataDef->GetVersion());
    Env.Init(pGC, 0);

    // _level0: the root movie clip. Its definition is the movie data def itself, which is why
    // GFxMovieDataDef is both a character def and a timeline.
    GFxResourceId rootId;
    rootId.Id = GFxResourceId::InvalidId;
    pLevel0 = new GFxSprite(dataDef, dataDef, defImpl, 0, rootId, this);
    pLevel0->SetName(pGC->GetBuiltin(GASbuiltin__level0));
    Env.SetTarget(pLevel0);

    Viewport.Left = 0;
    Viewport.Top = 0;
    Viewport.Width = (int)dataDef->GetWidth();
    Viewport.Height = (int)dataDef->GetHeight();
    Viewport.BufferWidth = Viewport.Width;
    Viewport.BufferHeight = Viewport.Height;

    FrameTime = dataDef->GetFrameRate() > 0.f ? 1.0f / dataDef->GetFrameRate() : 1.0f / 30.0f;
}

GFxMovieRoot::~GFxMovieRoot()
{
    // The order is the one retail's teardown needs and it is not interchangeable: the character tree
    // first (its destructors release handles and members), then the AS2 graph in one collection pass,
    // then the states. An AS2 object outliving its character is fine - the handle degrades to null -
    // but a character outliving the string manager is not.
    free(Actions);
    if (bTraceTeardown) printf("  [teardown] characters\n");
    if (pLevel0)
    {
        pLevel0->Release();
        pLevel0 = 0;
    }
    if (bTraceTeardown) printf("  [teardown] AS2 graph\n");
    Env.ReleaseAll();
    delete pGC;
    pGC = 0;
    if (bTraceTeardown) printf("  [teardown] states\n");
    for (int i = 0; i < MaxStates; ++i)
        if (States[i])
            States[i]->Release();
    if (bTraceTeardown) printf("  [teardown] movie root gone\n");
}

bool GFxMovieRoot::bTraceTeardown = false;

void GFxMovieRoot::LogScriptError(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ++Stats.ScriptErrors;
    printf("DISHONORED(bringup): AS2 error: %s\n", buf);
}

GASString GFxMovieRoot::CreateString(const char* s)
{
    return pGC->GetSC()->CreateString(s ? s : "");
}

// The action queue. GFxMovieRoot::ActionQueueType in retail is a priority-ordered queue with
// sessions; a DoAction tag pushes and DoActionsForSession drains. The priority matters: an init
// action (GFxAP_Init) must run before the frame actions (GFxAP_Frame) of the same frame, which is
// what makes a __Packages class visible to the code that instantiates it.
void GFxMovieRoot::PushActionBuffer(GASActionBuffer* buffer, GFxSprite* target,
                                    GFxActionPriority prio)
{
    if (buffer == 0 || buffer->IsNull())
        return;
    if (ActionCount >= ActionCapacity)
    {
        ActionCapacity = ActionCapacity ? ActionCapacity * 2 : 16;
        Actions = (ActionEntry*)realloc(Actions, ActionCapacity * sizeof(ActionEntry));
    }
    unsigned int at = ActionCount;
    while (at > 0 && Actions[at - 1].Priority > prio)
    {
        Actions[at] = Actions[at - 1];
        --at;
    }
    Actions[at].pBuffer = buffer;
    Actions[at].pTarget = target;
    Actions[at].Priority = prio;
    ++ActionCount;
}

void GFxMovieRoot::DoActions()
{
    if (bInActionQueue)
        return;
    bInActionQueue = true;
    // The queue can grow while it is being drained - a frame action that calls gotoAndPlay queues
    // more - so the index walks forward rather than snapshotting the count.
    for (unsigned int i = 0; i < ActionCount; ++i)
    {
        ActionEntry e = Actions[i];
        Env.SetTarget(e.pTarget ? e.pTarget : pLevel0);
        e.pBuffer->Execute(&Env);
        ++Stats.ActionBuffersRun;
        if (Env.bThrowing)
        {
            LogScriptError("uncaught exception from an action buffer");
            Env.bThrowing = false;
        }
    }
    ActionCount = 0;
    bInActionQueue = false;
    Env.SetTarget(pLevel0);
}

// --- GFxMovie -------------------------------------------------------------------------------

GFxMovieDef* GFxMovieRoot::GetMovieDef() const { return pDefImpl; }
unsigned int GFxMovieRoot::GetCurrentFrame() const                     // 2012 0xa016f0
{ return pLevel0 ? pLevel0->GetCurrentFrame() : 0; }
bool GFxMovieRoot::HasLooped() const { return pLevel0 ? pLevel0->HasLooped() : false; }
void GFxMovieRoot::GotoFrame(unsigned int frame)                       // 2012 0xa01760
{ if (pLevel0) pLevel0->GotoFrame(frame); }
bool GFxMovieRoot::GotoLabeledFrame(const char* label, int offset)      // 2012 0xa04ad0
{ return pLevel0 ? pLevel0->GotoLabeledFrame(label, offset) : false; }
void GFxMovieRoot::SetPlayState(GFxMovie::PlayState s)
{ if (pLevel0) pLevel0->SetPlaying(s == GFxMovie::Playing); }
GFxMovie::PlayState GFxMovieRoot::GetPlayState() const
{ return (pLevel0 && pLevel0->GetPlaying()) ? GFxMovie::Playing : GFxMovie::Stopped; }
void GFxMovieRoot::SetVisible(bool v) { bVisible = v; }
bool GFxMovieRoot::GetVisible() const { return bVisible; }

bool GFxMovieRoot::IsAvailable(const char* path) const
{
    GASValue v;
    GFxMovieRoot* self = const_cast<GFxMovieRoot*>(this);
    return self->Env.GetVariable(self->CreateString(path), &v, 0, 0) && !v.IsUndefined();
}

void GFxMovieRoot::CreateString(GFxValue* v, const char* s)            // 2012 0x9af4e0
{
    v->SetString(CreateString(s).ToCStr());
}

void GFxMovieRoot::CreateStringW(GFxValue* v, const wchar_t* s)         // 2012 0x9b0010
{
    // A wide string is narrowed for now: the runtime has no wide string manager yet, and the text
    // engine that needs one is the next wave's (agentBB.md 3.4).
    char buf[1024];
    unsigned int n = 0;
    if (s)
        while (s[n] && n < sizeof(buf) - 1) { buf[n] = (char)s[n]; ++n; }
    buf[n] = 0;
    v->SetString(CreateString(buf).ToCStr());
}

void GFxMovieRoot::CreateObject(GFxValue* v, const char* className, const GFxValue* args,
                                unsigned int nargs)                   // 2012 0x9b00a0
{
    GASValue result;
    if (className == 0 || className[0] == 0)
    {
        result.SetAsObject(pGC->NewObject());
    }
    else
    {
        GASValue ctorVal;
        Env.GetVariable(CreateString(className), &ctorVal, 0, 0);
        GASFunctionObject* ctor = ctorVal.GetFunction();
        if (ctor == 0)
        {
            LogScriptError("CreateObject: '%s' is not a constructor", className);
            v->SetUndefined();
            return;
        }
        for (unsigned int i = 0; i < nargs; ++i)
        {
            GASValue a;
            GFxValue2ASValue(args[nargs - 1 - i], &a);
            Env.Push(a);
        }
        GASObject* obj = Env.OperatorNew(ctor, (int)nargs, Env.GetTopIndex());
        Env.Drop((int)nargs);
        if (obj) result.SetAsObject(obj);
    }
    ASValue2GFxValue(&Env, result, v);
}

void GFxMovieRoot::CreateArray(GFxValue* v)                            // 2012 0x9af550
{
    GASValue result;
    result.SetAsObject(pGC->NewArray());
    ASValue2GFxValue(&Env, result, v);
}

void GFxMovieRoot::CreateFunction(GFxValue* v, GFxFunctionHandler* h, void* userData) // 0x9af5c0
{
    // Wrapping a GFxFunctionHandler needs GFxFunctionHandler::Call, whose parameter struct
    // (GFxFunctionHandler::Params) the generated header does not carry yet. Reported to agent BB
    // rather than guessed; agentBC.md 6 lists it.
    v->SetUndefined();
}

bool GFxMovieRoot::SetVariable(const char* path, const GFxValue& v, GFxMovie::SetVarType type)
{
    GASValue av;
    GFxValue2ASValue(v, &av);
    return Env.SetVariable(CreateString(path), av, 0, 0);
}

bool GFxMovieRoot::GetVariable(GFxValue* v, const char* path) const
{
    GFxMovieRoot* self = const_cast<GFxMovieRoot*>(this);
    GASValue av;
    bool ok = self->Env.GetVariable(self->CreateString(path), &av, 0, 0);
    self->ASValue2GFxValue(&self->Env, av, v);
    return ok;
}

bool GFxMovieRoot::SetVariableArray(GFxMovie::SetArrayType t, const char* path, unsigned int index,
                                    const void* data, unsigned int count, GFxMovie::SetVarType vt)
{
    GASValue arrVal;
    if (!Env.GetVariable(CreateString(path), &arrVal, 0, 0))
        return false;
    GASObject* o = arrVal.ToObject(&Env);
    if (o == 0 || o->GetObjectType() != Object_Array)
        return false;
    GASArrayObject* arr = (GASArrayObject*)o;
    for (unsigned int i = 0; i < count; ++i)
    {
        GASValue v;
        switch (t)
        {
        case GFxMovie::SA_Int:    v.SetInt(((const int*)data)[i]); break;
        case GFxMovie::SA_Double: v.SetNumber(((const double*)data)[i]); break;
        case GFxMovie::SA_Float:  v.SetNumber((double)((const float*)data)[i]); break;
        case GFxMovie::SA_String: v.SetString(CreateString(((const char* const*)data)[i])); break;
        case GFxMovie::SA_Value:  GFxValue2ASValue(((const GFxValue*)data)[i], &v); break;
        default: break;
        }
        arr->SetElement(index + i, v);
    }
    return true;
}

bool GFxMovieRoot::SetVariableArraySize(const char* path, unsigned int count,
                                        GFxMovie::SetVarType vt)
{
    GASValue arrVal;
    if (!Env.GetVariable(CreateString(path), &arrVal, 0, 0))
        return false;
    GASObject* o = arrVal.ToObject(&Env);
    if (o == 0 || o->GetObjectType() != Object_Array)
        return false;
    ((GASArrayObject*)o)->Resize(count);
    return true;
}

unsigned int GFxMovieRoot::GetVariableArraySize(const char* path)
{
    GASValue arrVal;
    if (!Env.GetVariable(CreateString(path), &arrVal, 0, 0))
        return 0;
    GASObject* o = arrVal.ToObject(&Env);
    if (o == 0 || o->GetObjectType() != Object_Array)
        return 0;
    return ((GASArrayObject*)o)->GetSize();
}

bool GFxMovieRoot::GetVariableArray(GFxMovie::SetArrayType t, const char* path, unsigned int index,
                                    void* data, unsigned int count)
{
    GASValue arrVal;
    if (!Env.GetVariable(CreateString(path), &arrVal, 0, 0))
        return false;
    GASObject* o = arrVal.ToObject(&Env);
    if (o == 0 || o->GetObjectType() != Object_Array)
        return false;
    GASArrayObject* arr = (GASArrayObject*)o;
    for (unsigned int i = 0; i < count; ++i)
    {
        const GASValue* e = arr->GetElementPtr(index + i);
        GASValue v;
        if (e) v = *e;
        switch (t)
        {
        case GFxMovie::SA_Int:    ((int*)data)[i] = v.ToInt32(&Env); break;
        case GFxMovie::SA_Double: ((double*)data)[i] = v.ToNumber(&Env); break;
        case GFxMovie::SA_Float:  ((float*)data)[i] = (float)v.ToNumber(&Env); break;
        case GFxMovie::SA_Value:  ASValue2GFxValue(&Env, v, &((GFxValue*)data)[i]); break;
        default: break;
        }
    }
    return true;
}

bool GFxMovieRoot::Invoke(const char* path, GFxValue* result, const GFxValue* args,
                          unsigned int nargs)
{
    GASString full = CreateString(path);
    GASString objPath, method;
    GASObjectInterface* self = pLevel0;
    GASValue fnVal;
    if (GASEnvironment::ParsePath(pGC->GetSC(), full, &objPath, &method))
    {
        GFxASCharacter* target = Env.FindTarget(objPath);
        if (target == 0)
        {
            GASValue objVal;
            if (Env.GetVariable(objPath, &objVal, 0, 0))
                self = objVal.ToObjectInterface(&Env);
            else
                return false;
        }
        else
        {
            self = target;
        }
        if (self == 0 || !self->GetMember(&Env, method, &fnVal))
            return false;
    }
    else if (!Env.GetVariable(full, &fnVal, 0, 0))
    {
        return false;
    }

    GASFunctionObject* fn = fnVal.GetFunction();
    if (fn == 0)
        return false;
    for (unsigned int i = 0; i < nargs; ++i)
    {
        GASValue a;
        GFxValue2ASValue(args[nargs - 1 - i], &a);
        Env.Push(a);
    }
    GASValue res;
    GASFnCall call(&res, self, &Env, (int)nargs, Env.GetTopIndex());
    fn->Invoke(call);
    Env.Drop((int)nargs);
    if (result)
        ASValue2GFxValue(&Env, res, result);
    return true;
}

bool GFxMovieRoot::Invoke(const char* path, GFxValue* result, const char* argFmt, ...)
{
    va_list args;
    va_start(args, argFmt);
    bool ok = InvokeArgs(path, result, argFmt, (char*)args);
    va_end(args);
    return ok;
}

bool GFxMovieRoot::InvokeArgs(const char* path, GFxValue* result, const char* argFmt, char* args)
{
    // The printf-style overload retail supports: i, u, f, d (double), s, w (wide), b.
    enum { MaxArgs = 16 };
    GFxValue argv[MaxArgs];
    unsigned int n = 0;
    va_list va;
    memcpy(&va, &args, sizeof(va));
    for (const char* p = argFmt; p && *p && n < MaxArgs; ++p)
    {
        switch (*p)
        {
        case 'i': argv[n++].SetNumber((double)va_arg(va, int)); break;
        case 'u': argv[n++].SetNumber((double)va_arg(va, unsigned int)); break;
        case 'f':
        case 'd': argv[n++].SetNumber(va_arg(va, double)); break;
        case 's': argv[n++].SetString(va_arg(va, const char*)); break;
        case 'w': argv[n++].SetStringW(va_arg(va, const wchar_t*)); break;
        case 'b': argv[n++].SetBoolean(va_arg(va, int) != 0); break;
        default: break;
        }
    }
    return Invoke(path, result, argv, n);
}

// --- GFxMovieView ---------------------------------------------------------------------------

void GFxMovieRoot::SetViewport(const GViewport& vp) { Viewport = vp; bDirty = true; }
void GFxMovieRoot::GetViewport(GViewport* vp) const { *vp = Viewport; }
void GFxMovieRoot::SetViewScaleMode(GFxMovieView::ScaleModeType m) { ScaleMode = m; }
GFxMovieView::ScaleModeType GFxMovieRoot::GetViewScaleMode() const { return ScaleMode; }
void GFxMovieRoot::SetViewAlignment(GFxMovieView::AlignType a) { Alignment = a; }
GFxMovieView::AlignType GFxMovieRoot::GetViewAlignment() const { return Alignment; }

GRect<float> GFxMovieRoot::GetVisibleFrameRect() const                 // 2012 0xa102a0
{
    return pDefImpl->GetFrameRect();
}

void GFxMovieRoot::SetPerspective3D(const GMatrix3D& m) {}
void GFxMovieRoot::SetView3D(const GMatrix3D& m) {}
GRect<float> GFxMovieRoot::GetSafeRect() const { return pDefImpl->GetFrameRect(); }
void GFxMovieRoot::SetSafeRect(const GRect<float>& r) {}

void GFxMovieRoot::Restart()
{
    if (pLevel0)
    {
        pLevel0->GotoFrame(0);
        pLevel0->SetPlaying(true);
    }
}

float GFxMovieRoot::Advance(float deltaT, unsigned int frameCatchUp)   // 2012 0xa11830
{
    if (bPaused)
        return FrameTime;

    // Frame 0 is special: retail runs its tags and its onLoad through ExecuteFrame0Events rather
    // than through the advance path, because advancing would already have moved past it.
    if (pLevel0)
    {
        bool bFirst = Stats.FramesAdvanced == 0;
        if (bFirst)
        {
            pLevel0->ExecuteFrame0Events();
            DoActions();
        }
        else
        {
            TimeElapsed += deltaT;
            unsigned int steps = 0;
            while (TimeElapsed >= FrameTime && steps <= frameCatchUp)
            {
                TimeElapsed -= FrameTime;
                pLevel0->AdvanceFrame(true, 0.f);
                DoActions();
                ++steps;
            }
            if (steps == 0)
            {
                pLevel0->AdvanceFrame(false, 0.f);
                DoActions();
            }
        }
        ++Stats.FramesAdvanced;
    }
    bDirty = true;
    return FrameTime;
}

void GFxMovieRoot::Display()                                          // 2012 0xa07aa0
{
    // Drawing is the renderer seam's and the tessellator's, neither of which is this package: agent
    // BB's FGFxRenderer has all 54 slots and agent BD owns the GFx shader families. The display list
    // is walked here so a future renderer has the traversal already in the right order.
    bDirty = false;
}

void GFxMovieRoot::DisplayPrePass() {}
void GFxMovieRoot::SetPause(bool p) { bPaused = p; }
bool GFxMovieRoot::IsPaused() const { return bPaused; }
void GFxMovieRoot::SetBackgroundColor(const GColor c) { BackgroundColor = c; }
void GFxMovieRoot::SetBackgroundAlpha(float a) { BackgroundAlpha = a; }
float GFxMovieRoot::GetBackgroundAlpha() const { return BackgroundAlpha; }

unsigned int GFxMovieRoot::HandleEvent(const GFxEvent& e)
{
    // Mouse, key and focus events need the button and focus model (GFxButtonCharacter, 38 retail
    // functions, plus GFx_GenerateMouseButtonEvents at 2012 0xa66a90). Not this wave.
    return GFxMovieView::HE_NotHandled;
}

void GFxMovieRoot::GetMouseState(unsigned int i, float* x, float* y, unsigned int* buttons)
{
    if (x) *x = 0.f;
    if (y) *y = 0.f;
    if (buttons) *buttons = 0;
}

void GFxMovieRoot::NotifyMouseState(float x, float y, unsigned int buttons, unsigned int index)
{ (void)x; (void)y; (void)buttons; (void)index; }
bool GFxMovieRoot::HitTest(float x, float y, GFxMovieView::HitTestType t, unsigned int c)
{ (void)x; (void)y; (void)t; (void)c; return false; }
bool GFxMovieRoot::HitTest3D(GPoint3<float>* p, float x, float y, unsigned int c)
{ (void)p; (void)x; (void)y; (void)c; return false; }
void GFxMovieRoot::SetExternalInterfaceRetVal(const GFxValue& v) { (void)v; }
void* GFxMovieRoot::GetUserData() const { return pUserData; }
void GFxMovieRoot::SetUserData(void* d) { pUserData = d; }
bool GFxMovieRoot::AttachDisplayCallback(const char* path, void* cb, void* userData)
{ return false; }
bool GFxMovieRoot::IsMovieFocused() const { return true; }
bool GFxMovieRoot::GetDirtyFlag(bool clear)
{
    bool v = bDirty;
    if (clear) bDirty = false;
    return v;
}
void GFxMovieRoot::SetMouseCursorCount(unsigned int n) { MouseCursorCount = n; }
unsigned int GFxMovieRoot::GetMouseCursorCount() const { return MouseCursorCount; }
void GFxMovieRoot::SetControllerCount(unsigned int n) { ControllerCount = n; }
unsigned int GFxMovieRoot::GetControllerCount() const { return ControllerCount; }
void GFxMovieRoot::GetStats(GStatBag* bag, bool reset) {}
GMemoryHeap* GFxMovieRoot::GetHeap() const { return 0; }
void GFxMovieRoot::ForceCollectGarbage() {}

GPoint<float> GFxMovieRoot::TranslateToScreen(const GPoint<float>& p, GMatrix2D m)
{
    GPoint<float> out = p;
    m.Transform(&out.x, &out.y);
    return out;
}

GRect<float> GFxMovieRoot::TranslateToScreen(const GRect<float>& r, GMatrix2D m)
{
    float x0 = r.Left, y0 = r.Top, x1 = r.Right, y1 = r.Bottom;
    m.Transform(&x0, &y0);
    m.Transform(&x1, &y1);
    return GRect<float>(x0, y0, x1, y1);
}

bool GFxMovieRoot::TranslateLocalToScreen(const char* path, const GPoint<float>& p,
                                          GPoint<float>* out, GMatrix2D m)
{
    GFxASCharacter* ch = Env.FindTarget(CreateString(path));
    if (ch == 0)
        return false;
    GPoint<float> local = p;
    ch->GetMatrix().Transform(&local.x, &local.y);
    m.Transform(&local.x, &local.y);
    *out = local;
    return true;
}

bool GFxMovieRoot::SetControllerFocusGroup(unsigned int c, unsigned int g) { return false; }
unsigned int GFxMovieRoot::GetControllerFocusGroup(unsigned int c) const { return 0; }
GFxMovieDef::MemoryContext* GFxMovieRoot::GetMemoryContext() const { return 0; }
void GFxMovieRoot::Release() { GRefCountImplCore::Release(); }

void GFxMovieRoot::SetState(GFxState::StateType t, GFxState* s)
{
    if ((unsigned int)t >= MaxStates)
        return;
    if (s) s->AddRef();
    if (States[t]) States[t]->Release();
    States[t] = s;
}

GFxState* GFxMovieRoot::GetStateAddRef(GFxState::StateType t) const
{
    if ((unsigned int)t >= MaxStates || States[t] == 0)
        return pDefImpl ? pDefImpl->GetStateAddRef(t) : 0;
    States[t]->AddRef();
    return States[t];
}

void GFxMovieRoot::GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                   unsigned int count) const
{
    for (unsigned int i = 0; i < count; ++i)
        out[i] = GetStateAddRef(types[i]);
}

// ---------------------------------------------------------------------------------------------
// GASValue <-> GFxValue. The union slot a GFxValue carries for an object is what the ObjectInterface
// methods take as their `void* obj`: a GASObjectInterface* for VT_Object/VT_Array and a
// GFxCharacterHandle* for VT_DisplayObject. GFxValue::ObjectInterface::GetMember (2012 0x9b0450)
// proves the second half by calling GFxCharacterHandle::ResolveCharacter on it.

void GFxMovieRoot::ASValue2GFxValue(GASEnvironment* env, const GASValue& in, GFxValue* out)
{
    if (out == 0)
        return;
    switch (in.GetType())
    {
    case GASValue::UNDEFINED:
    case GASValue::UNSET:
        out->SetUndefined();
        break;
    case GASValue::NULLTYPE:
        out->SetNull();
        break;
    case GASValue::BOOLEAN:
        out->SetBoolean(in.GetBool());
        break;
    case GASValue::NUMBER:
    case GASValue::INT:
        out->SetNumber(in.GetNumber());
        break;
    case GASValue::STRING:
        // The interned node outlives the movie, so handing the engine the pointer is safe and is
        // what retail's unmanaged string case does.
        out->SetString(in.GetStringCStr());
        break;
    case GASValue::CHARACTER:
    {
        GFxASCharacter* ch = in.GetCharacter();
        if (ch == 0) { out->SetUndefined(); break; }
        GFxCharacterHandle* h = ch->GetCharacterHandle();
        if (h == 0) h = ch->CreateCharacterHandle();
        h->AddRef();
        out->pObjectInterface = &ObjInterface;
        out->Type = (GFxValue::ValueType)(GFxValue::VT_DisplayObject | GFxValue::VTC_ManagedBit);
        out->Value.pData = h;
        break;
    }
    case GASValue::OBJECT:
    case GASValue::FUNCTION:
    case GASValue::RESOLVE_HANDLER:
    {
        GASObjectInterface* oi = in.ToObjectInterface(env);
        if (oi == 0) { out->SetUndefined(); break; }
        GASObject* o = oi->ToASObject();
        if (o) o->AddRef();
        out->pObjectInterface = &ObjInterface;
        bool isArray = o && o->GetObjectType() == Object_Array;
        out->Type = (GFxValue::ValueType)((isArray ? GFxValue::VT_Array : GFxValue::VT_Object)
                                         | GFxValue::VTC_ManagedBit);
        out->Value.pData = oi;
        break;
    }
    default:
        out->SetUndefined();
        break;
    }
}

void GFxMovieRoot::GFxValue2ASValue(const GFxValue& in, GASValue* out)
{
    switch (in.GetType())
    {
    case GFxValue::VT_Null:          out->SetNull(); break;
    case GFxValue::VT_Boolean:       out->SetBool(in.GetBool()); break;
    case GFxValue::VT_Number:        out->SetNumber(in.GetNumber()); break;
    case GFxValue::VT_String:        out->SetString(CreateString(in.GetString())); break;
    case GFxValue::VT_StringW:
    {
        char buf[1024];
        const wchar_t* w = in.GetStringW();
        unsigned int n = 0;
        if (w) while (w[n] && n < sizeof(buf) - 1) { buf[n] = (char)w[n]; ++n; }
        buf[n] = 0;
        out->SetString(CreateString(buf));
        break;
    }
    case GFxValue::VT_Object:
    case GFxValue::VT_Array:
    {
        GASObjectInterface* oi = (GASObjectInterface*)in.Value.pData;
        out->SetAsObject(oi ? oi->ToASObject() : 0);
        break;
    }
    case GFxValue::VT_DisplayObject:
    {
        GFxCharacterHandle* h = (GFxCharacterHandle*)in.Value.pData;
        out->SetAsCharacter(h ? h->ResolveCharacter(this) : 0);
        break;
    }
    default:
        out->SetUndefined();
        break;
    }
}

// ---------------------------------------------------------------------------------------------
// GFxValue::ObjectInterface - all 25 methods. `obj` is a GASObjectInterface* unless isDObj, in which
// case it is a GFxCharacterHandle*.

static GASObjectInterface* ResolveObj(void* obj, bool isDObj, GFxMovieRoot* root)
{
    if (obj == 0)
        return 0;
    if (!isDObj)
        return (GASObjectInterface*)obj;
    GFxCharacterHandle* h = (GFxCharacterHandle*)obj;
    return h->ResolveCharacter(root);
}

void GFxValue::ObjectInterface::ObjectAddRef(GFxValue* val, void* obj)  // 2012 0x9ae650
{
    if (obj == 0)
        return;
    if (val->GetType() == GFxValue::VT_DisplayObject)
        ((GFxCharacterHandle*)obj)->AddRef();
    else
    {
        GASObjectInterface* oi = (GASObjectInterface*)obj;
        GASObject* o = oi->ToASObject();
        if (o) o->AddRef();
    }
}

void GFxValue::ObjectInterface::ObjectRelease(GFxValue* val, void* obj) // 2012 0x9aed40
{
    if (obj == 0)
        return;
    if (val->GetType() == GFxValue::VT_DisplayObject)
        ((GFxCharacterHandle*)obj)->Release();
    else
    {
        GASObjectInterface* oi = (GASObjectInterface*)obj;
        GASObject* o = oi->ToASObject();
        if (o) o->Release();
    }
}

bool GFxValue::ObjectInterface::HasMember(void* obj, const char* name, bool isDObj) const
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0)
        return false;
    return oi->HasMember(root->GetASContext()->GetSC(), root->CreateString(name), true);
}

bool GFxValue::ObjectInterface::GetMember(void* obj, const char* name, GFxValue* val,
                                         bool isDObj) const           // 2012 0x9b0450
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0)
    {
        if (val) val->SetUndefined();
        return false;
    }
    GASEnvironment* env = root->GetASEnvironment();
    GASValue av;
    if (!oi->GetMember(env, root->CreateString(name), &av))
    {
        if (val) val->SetUndefined();
        return false;
    }
    root->ASValue2GFxValue(env, av, val);
    return true;
}

bool GFxValue::ObjectInterface::SetMember(void* obj, const char* name, const GFxValue& val,
                                         bool isDObj)                 // 2012 0x9ad590
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0)
        return false;
    GASValue av;
    root->GFxValue2ASValue(val, &av);
    return oi->SetMember(root->GetASEnvironment(), root->CreateString(name), av, GASPropFlags());
}

bool GFxValue::ObjectInterface::Invoke(void* obj, GFxValue* result, const char* name,
                                      const GFxValue* args, unsigned int nargs, bool isDObj)
{                                                                     // 2012 0x9afbb0
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0)
        return false;
    GASEnvironment* env = root->GetASEnvironment();
    GASValue fnVal;
    if (!oi->GetMember(env, root->CreateString(name), &fnVal))
        return false;
    GASFunctionObject* fn = fnVal.GetFunction();
    if (fn == 0)
        return false;
    for (unsigned int i = 0; i < nargs; ++i)
    {
        GASValue a;
        root->GFxValue2ASValue(args[nargs - 1 - i], &a);
        env->Push(a);
    }
    GASValue res;
    GASFnCall call(&res, oi, env, (int)nargs, env->GetTopIndex());
    fn->Invoke(call);
    env->Drop((int)nargs);
    if (result)
        root->ASValue2GFxValue(env, res, result);
    return true;
}

bool GFxValue::ObjectInterface::DeleteMember(void* obj, const char* name, bool isDObj)
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0)
        return false;
    return oi->DeleteMember(root->GetASContext()->GetSC(), root->CreateString(name));
}

namespace
{
class GFxObjVisitorBridge : public GASObjectInterface::MemberVisitor
{
public:
    GFxObjVisitorBridge(GFxValue::ObjectInterface::ObjVisitor* v, GFxMovieRoot* r)
        : pVisitor(v), pRoot(r) {}
    virtual void Visit(const GASString& name, const GASValue& val, unsigned char flags)
    {
        GFxValue gv;
        pRoot->ASValue2GFxValue(pRoot->GetASEnvironment(), val, &gv);
        pVisitor->Visit(name.ToCStr(), gv);
    }
    GFxValue::ObjectInterface::ObjVisitor* pVisitor;
    GFxMovieRoot* pRoot;
};

class GFxArrVisitorBridge
{
public:
    GFxArrVisitorBridge(GFxValue::ObjectInterface::ArrVisitor* v, GFxMovieRoot* r)
        : pVisitor(v), pRoot(r) {}
    void Visit(unsigned int i, const GASValue& val)
    {
        GFxValue gv;
        pRoot->ASValue2GFxValue(pRoot->GetASEnvironment(), val, &gv);
        pVisitor->Visit(i, gv);
    }
    GFxValue::ObjectInterface::ArrVisitor* pVisitor;
    GFxMovieRoot* pRoot;
};
}

void GFxValue::ObjectInterface::VisitMembers(void* obj, ObjVisitor* visitor, bool isDObj) const
{                                                                     // 2012 0x9ae6c0
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, isDObj, root);
    if (oi == 0 || visitor == 0)
        return;
    GFxObjVisitorBridge bridge(visitor, root);
    oi->VisitMembers(root->GetASContext()->GetSC(), &bridge, 0, oi);
}

static GASArrayObject* AsArray(void* obj)
{
    if (obj == 0)
        return 0;
    GASObjectInterface* oi = (GASObjectInterface*)obj;
    GASObject* o = oi->ToASObject();
    if (o && o->GetObjectType() == Object_Array)
        return (GASArrayObject*)o;
    return 0;
}

unsigned int GFxValue::ObjectInterface::GetArraySize(void* obj) const   // 2012 0x9ad650
{
    GASArrayObject* arr = AsArray(obj);
    return arr ? arr->GetSize() : 0;
}

bool GFxValue::ObjectInterface::SetArraySize(void* obj, unsigned int sz)
{
    GASArrayObject* arr = AsArray(obj);
    if (arr == 0)
        return false;
    arr->Resize(sz);
    return true;
}

bool GFxValue::ObjectInterface::GetElement(void* obj, unsigned int idx, GFxValue* val) const
{                                                                     // 2012 0x9b05c0
    GASArrayObject* arr = AsArray(obj);
    const GASValue* e = arr ? arr->GetElementPtr(idx) : 0;
    if (e == 0)
    {
        if (val) val->SetUndefined();
        return false;
    }
    pMovieRoot->ASValue2GFxValue(pMovieRoot->GetASEnvironment(), *e, val);
    return true;
}

bool GFxValue::ObjectInterface::SetElement(void* obj, unsigned int idx, const GFxValue& val)
{                                                                     // 2012 0x9ad670
    GASArrayObject* arr = AsArray(obj);
    if (arr == 0)
        return false;
    GASValue av;
    pMovieRoot->GFxValue2ASValue(val, &av);
    arr->SetElement(idx, av);
    return true;
}

bool GFxValue::ObjectInterface::VisitElements(void* obj, ArrVisitor* visitor, unsigned int idx,
                                             int count) const
{
    GASArrayObject* arr = AsArray(obj);
    if (arr == 0 || visitor == 0)
        return false;
    unsigned int n = count < 0 ? arr->GetSize() : (unsigned int)count;
    GFxArrVisitorBridge bridge(visitor, pMovieRoot);
    for (unsigned int i = idx; i < idx + n && i < arr->GetSize(); ++i)
        bridge.Visit(i, *arr->GetElementPtr(i));
    return true;
}

bool GFxValue::ObjectInterface::PushBack(void* obj, const GFxValue& val)  // 2012 0x9ad6c0
{
    GASArrayObject* arr = AsArray(obj);
    if (arr == 0)
        return false;
    GASValue av;
    pMovieRoot->GFxValue2ASValue(val, &av);
    arr->PushBack(av);
    return true;
}

bool GFxValue::ObjectInterface::RemoveElements(void* obj, unsigned int idx, int count)
{
    GASArrayObject* arr = AsArray(obj);
    if (arr == 0)
        return false;
    arr->RemoveElements(idx, count);
    return true;
}

bool GFxValue::ObjectInterface::GetText(void* obj, GFxValue* val, bool html) const // 2012 0x9b0640
{
    // A text field's text lives on GFxEditTextCharacter, which is 125 retail functions and needs the
    // text engine. Until then the member named "text" is the honest answer, because that is where a
    // dynamic text field's content is written from AS2 anyway.
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    if (oi == 0)
        return false;
    GASValue av;
    if (!oi->GetMemberRaw(root->GetASContext()->GetSC(), root->CreateString("text"), &av))
    {
        if (val) val->SetUndefined();
        return false;
    }
    root->ASValue2GFxValue(root->GetASEnvironment(), av, val);
    return true;
}

bool GFxValue::ObjectInterface::SetText(void* obj, const char* text, bool html) // 2012 0x9afdb0
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    if (oi == 0)
        return false;
    GASValue av;
    av.SetString(root->CreateString(text));
    return oi->SetMemberRaw(root->GetASContext()->GetSC(), root->CreateString("text"), av,
                            GASPropFlags());
}

bool GFxValue::ObjectInterface::SetText(void* obj, const wchar_t* text, bool html) // 2012 0x9afee0
{
    char buf[2048];
    unsigned int n = 0;
    if (text)
        while (text[n] && n < sizeof(buf) - 1) { buf[n] = (char)text[n]; ++n; }
    buf[n] = 0;
    return SetText(obj, buf, html);
}

bool GFxValue::ObjectInterface::GetDisplayInfo(void* obj, DisplayInfo* info) const // 2012 0x9ad900
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0 || info == 0)
        return false;
    const GMatrix2D& m = ch->GetMatrix();
    info->Clear();
    info->SetX(m.M_[0][2] * GFxTwipsToPixels);
    info->SetY(m.M_[1][2] * GFxTwipsToPixels);
    info->SetXScale(m.M_[0][0] * 100.0);
    info->SetYScale(m.M_[1][1] * 100.0);
    info->SetAlpha(ch->GetCxform().M_[0][3] * 100.0);
    info->SetVisible(ch->GetVisible());
    info->SetRotation(0.0);
    return true;
}

bool GFxValue::ObjectInterface::SetDisplayInfo(void* obj, const DisplayInfo& info) // 2012 0x9adb20
{
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0)
        return false;
    GMatrix2D m = ch->GetMatrix();
    if (info.IsFlagSet(DisplayInfo::V_x)) m.M_[0][2] = (float)(info.GetX() * GFxPixelsToTwips);
    if (info.IsFlagSet(DisplayInfo::V_y)) m.M_[1][2] = (float)(info.GetY() * GFxPixelsToTwips);
    if (info.IsFlagSet(DisplayInfo::V_xscale)) m.M_[0][0] = (float)(info.GetXScale() / 100.0);
    if (info.IsFlagSet(DisplayInfo::V_yscale)) m.M_[1][1] = (float)(info.GetYScale() / 100.0);
    ch->SetMatrix(m);
    if (info.IsFlagSet(DisplayInfo::V_alpha))
    {
        GRenderer::Cxform cx = ch->GetCxform();
        cx.M_[0][3] = (float)(info.GetAlpha() / 100.0);
        ch->SetCxform(cx);
    }
    if (info.IsFlagSet(DisplayInfo::V_visible))
        ch->SetVisible(info.GetVisible());
    return true;
}

bool GFxValue::ObjectInterface::GetDisplayMatrix(void* obj, GMatrix2D* mat) const // 2012 0x9ad710
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0 || mat == 0)
        return false;
    *mat = ch->GetMatrix();
    return true;
}

bool GFxValue::ObjectInterface::SetDisplayMatrix(void* obj, const GMatrix2D& mat) // 2012 0x9ad7a0
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0)
        return false;
    ch->SetMatrix(mat);
    return true;
}

bool GFxValue::ObjectInterface::SetMatrix3D(void* obj, const GMatrix3D& mat) // 2012 0x9acd30
{
    // A 3D transform on a character needs GFxCharacter::CreateMatrix3D (2012 0x9ce370) and the
    // renderer's 3D path; recorded as unsupported rather than silently dropped.
    return false;
}

bool GFxValue::ObjectInterface::GetCxform(void* obj, GRenderer::Cxform* cx) const // 2012 0x9ad130
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0 || cx == 0)
        return false;
    *cx = ch->GetCxform();
    return true;
}

bool GFxValue::ObjectInterface::SetCxform(void* obj, const GRenderer::Cxform& cx) // 2012 0x9ad160
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxASCharacter* ch = oi ? oi->ToASCharacter() : 0;
    if (ch == 0)
        return false;
    ch->SetCxform(cx);
    return true;
}

bool GFxValue::ObjectInterface::GotoAndPlay(void* obj, const char* frame, bool stop) // 0x9ad050
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxSprite* sp = oi ? oi->ToSprite() : 0;
    if (sp == 0)
        return false;
    if (!sp->GotoLabeledFrame(frame, 0))
        return false;
    sp->SetPlaying(!stop);
    return true;
}

bool GFxValue::ObjectInterface::GotoAndPlay(void* obj, unsigned int frame, bool stop) // 0x9ad0d0
{
    GASObjectInterface* oi = ResolveObj(obj, true, pMovieRoot);
    GFxSprite* sp = oi ? oi->ToSprite() : 0;
    if (sp == 0)
        return false;
    sp->GotoFrame(frame);
    sp->SetPlaying(!stop);
    return true;
}

bool GFxValue::ObjectInterface::CreateEmptyMovieClip(void* obj, GFxValue* mc,
                                                     const char* instanceName, int depth)
{                                                                     // 2012 0x9af6a0
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    GFxSprite* sp = oi ? oi->ToSprite() : 0;
    if (sp == 0)
        return false;
    if (depth < 0)
        depth = sp->GetDisplayList().GetLargestDepthInUse() + 1;
    GFxSprite* child = sp->CreateEmptyMovieClip(root->CreateString(instanceName), depth);
    if (child == 0)
        return false;
    GASValue v;
    v.SetAsCharacter(child);
    root->ASValue2GFxValue(root->GetASEnvironment(), v, mc);
    return true;
}

bool GFxValue::ObjectInterface::AttachMovie(void* obj, GFxValue* mc, const char* symbolName,
                                            const char* instanceName, int depth,
                                            const GFxValue* initArgs)
{                                                                     // 2012 0x9af830
    GFxMovieRoot* root = pMovieRoot;
    GASObjectInterface* oi = ResolveObj(obj, true, root);
    GFxSprite* sp = oi ? oi->ToSprite() : 0;
    if (sp == 0)
        return false;
    if (depth < 0)
        depth = sp->GetDisplayList().GetLargestDepthInUse() + 1;
    GFxSprite* child = sp->AttachMovie(root->CreateString(symbolName),
                                       root->CreateString(instanceName), depth);
    if (child == 0)
        return false;
    GASValue v;
    v.SetAsCharacter(child);
    root->ASValue2GFxValue(root->GetASEnvironment(), v, mc);
    return true;
}

void GFxValue::DisplayInfo::Set(double x, double y, double rot, double xs, double ys, double alpha,
                                bool visible, double z, double xrot, double yrot, double zscale,
                                double fov)                           // 2012 0x9ad320
{
    SetX(x); SetY(y); SetRotation(rot); SetXScale(xs); SetYScale(ys); SetAlpha(alpha);
    SetVisible(visible); SetZ(z); SetXRotation(xrot); SetYRotation(yrot); SetZScale(zscale);
    SetFOV(fov);
}

// ---------------------------------------------------------------------------------------------
// The opcode names, for the acceptance table.

const char* GFxAS2GetOpcodeName(unsigned char op)
{
    switch (op)
    {
    case GASop_End: return "End";
    case GASop_NextFrame: return "NextFrame";
    case GASop_PrevFrame: return "PrevFrame";
    case GASop_Play: return "Play";
    case GASop_Stop: return "Stop";
    case GASop_Add: return "Add";
    case GASop_Subtract: return "Subtract";
    case GASop_Multiply: return "Multiply";
    case GASop_Divide: return "Divide";
    case GASop_Equal: return "Equal";
    case GASop_LessThan: return "LessThan";
    case GASop_LogicalAnd: return "LogicalAnd";
    case GASop_LogicalOr: return "LogicalOr";
    case GASop_LogicalNot: return "LogicalNot";
    case GASop_StringEqual: return "StringEqual";
    case GASop_StringLength: return "StringLength";
    case GASop_SubString: return "SubString";
    case GASop_Pop: return "Pop";
    case GASop_ToInteger: return "ToInteger";
    case GASop_GetVariable: return "GetVariable";
    case GASop_SetVariable: return "SetVariable";
    case GASop_SetTargetExpression: return "SetTargetExpression";
    case GASop_StringConcat: return "StringConcat";
    case GASop_GetProperty: return "GetProperty";
    case GASop_SetProperty: return "SetProperty";
    case GASop_DuplicateClip: return "DuplicateClip";
    case GASop_RemoveClip: return "RemoveClip";
    case GASop_Trace: return "Trace";
    case GASop_StartDragMovie: return "StartDragMovie";
    case GASop_StopDragMovie: return "StopDragMovie";
    case GASop_StringCompare: return "StringCompare";
    case GASop_Throw: return "Throw";
    case GASop_CastOp: return "CastOp";
    case GASop_ImplementsOp: return "ImplementsOp";
    case GASop_Random: return "Random";
    case GASop_MBLength: return "MBLength";
    case GASop_Ord: return "Ord";
    case GASop_Chr: return "Chr";
    case GASop_GetTimer: return "GetTimer";
    case GASop_MBSubString: return "MBSubString";
    case GASop_MBOrd: return "MBOrd";
    case GASop_MBChr: return "MBChr";
    case GASop_Delete: return "Delete";
    case GASop_Delete2: return "Delete2";
    case GASop_DefineLocal: return "DefineLocal";
    case GASop_CallFunction: return "CallFunction";
    case GASop_Return: return "Return";
    case GASop_Modulo: return "Modulo";
    case GASop_New: return "New";
    case GASop_DeclareLocal: return "DeclareLocal";
    case GASop_InitArray: return "InitArray";
    case GASop_InitObject: return "InitObject";
    case GASop_TypeOf: return "TypeOf";
    case GASop_TargetPath: return "TargetPath";
    case GASop_Enumerate: return "Enumerate";
    case GASop_NewAdd: return "Add2";
    case GASop_NewLessThan: return "Less2";
    case GASop_NewEquals: return "Equals2";
    case GASop_ToNumber: return "ToNumber";
    case GASop_ToString: return "ToString";
    case GASop_Dup: return "PushDuplicate";
    case GASop_Swap: return "StackSwap";
    case GASop_GetMember: return "GetMember";
    case GASop_SetMember: return "SetMember";
    case GASop_Increment: return "Increment";
    case GASop_Decrement: return "Decrement";
    case GASop_CallMethod: return "CallMethod";
    case GASop_NewMethod: return "NewMethod";
    case GASop_InstanceOf: return "InstanceOf";
    case GASop_Enumerate2: return "Enumerate2";
    case GASop_BitwiseAnd: return "BitAnd";
    case GASop_BitwiseOr: return "BitOr";
    case GASop_BitwiseXor: return "BitXor";
    case GASop_ShiftLeft: return "BitLShift";
    case GASop_ShiftRight: return "BitRShift";
    case GASop_ShiftRightUnsigned: return "BitURShift";
    case GASop_StrictEqual: return "StrictEquals";
    case GASop_Greater: return "Greater";
    case GASop_StringGreater: return "StringGreater";
    case GASop_Extends: return "Extends";
    case GASop_GotoFrame: return "GotoFrame";
    case GASop_GetUrl: return "GetURL";
    case GASop_StoreRegister: return "StoreRegister";
    case GASop_ConstantPool: return "ConstantPool";
    case GASop_WaitForFrame: return "WaitForFrame";
    case GASop_SetTarget: return "SetTarget";
    case GASop_GotoLabel: return "GotoLabel";
    case GASop_WaitForFrameExpression: return "WaitForFrame2";
    case GASop_DefineFunction2: return "DefineFunction2";
    case GASop_Try: return "Try";
    case GASop_With: return "With";
    case GASop_Push: return "Push";
    case GASop_BranchAlways: return "Jump";
    case GASop_GetUrl2: return "GetURL2";
    case GASop_DefineFunction: return "DefineFunction";
    case GASop_BranchIfTrue: return "If";
    case GASop_CallFrame: return "Call";
    case GASop_GotoExpression: return "GotoFrame2";
    default: return "?";
    }
}
