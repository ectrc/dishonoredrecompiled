// Scaleform GFx 3.3.89 - the input half. See GFxInput.h for the chain and for why the AS2 Key
// broadcaster, not the button model, is what the Dishonored menus are driven by. Package DG.
#include "GFxInput.h"
#include "GFxPlayer.h"
#include "GFxAS2Object.h"
#include "GFxAS2Runtime.h"

#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------------------------------------------
// GFxKeyboardState
// ---------------------------------------------------------------------------------------------

GFxKeyboardState::GFxKeyboardState()                                  // 2012 0xa5f280
    : pListener(0), KeyboardIndex(0)
{
    memset(Keymap, 0, sizeof(Keymap));
    memset(Toggled, 0, sizeof(Toggled));
}

bool GFxKeyboardState::IsKeyDown(int code) const                      // 2012 0xa5f040
{
    if ((unsigned int)code > KeyCount)
        return false;
    return (Keymap[code >> 3] & (1 << (code & 7))) != 0;
}

bool GFxKeyboardState::IsKeyToggled(int code) const                   // 2012 0xa5f080
{
    // Retail toggles only the three lock keys, and it packs them into three bytes by the same
    // code >> 3 / code & 7 arithmetic with the CapsLock base subtracted: 20 (Caps), 144 (Num),
    // 145 (Scroll). The three bits land in [0], [2] and [2] of a three-byte map.
    if (code == 20)
        return (Toggled[0] & 1) != 0;
    if (code == 144)
        return (Toggled[1] & 1) != 0;
    if (code == 145)
        return (Toggled[2] & 1) != 0;
    return false;
}

void GFxKeyboardState::SetKeyToggled(int code, bool toggled)          // 2012 0xa5f0c0
{
    unsigned char* slot = 0;
    if (code == 20)       slot = &Toggled[0];
    else if (code == 144) slot = &Toggled[1];
    else if (code == 145) slot = &Toggled[2];
    if (slot == 0)
        return;
    *slot = toggled ? 1 : 0;
}

void GFxKeyboardState::SetKeyDown(int code, unsigned char ascii, GFxSpecialKeysState special)
{                                                                     // 2012 0xa5f100
    (void)ascii; (void)special;
    if ((unsigned int)code > KeyCount)
        return;
    Keymap[code >> 3] |= (unsigned char)(1 << (code & 7));
    // DISHONORED(bringup): retail also pushes the key onto GFxKeyboardState::KeyQueue (2012
    // 0xa5ef30), which exists so that a movie opened after the key went down sees the pending
    // events. Nothing in this runtime reads that queue - the listener path is synchronous - so it
    // is not reconstructed, and the omission is stated here rather than hidden.
}

void GFxKeyboardState::SetKeyUp(int code, unsigned char ascii, GFxSpecialKeysState special)
{                                                                     // 2012 0xa5f150
    (void)ascii; (void)special;
    if ((unsigned int)code > KeyCount)
        return;
    Keymap[code >> 3] &= (unsigned char)~(1 << (code & 7));
}

void GFxKeyboardState::ResetState()                                   // 2012 0xa5efc0
{
    memset(Keymap, 0, sizeof(Keymap));
    memset(Toggled, 0, sizeof(Toggled));
}

void GFxKeyboardState::NotifyListeners(GASStringContext* sc, short code, unsigned char ascii,
                                       unsigned int wcharCode, GFxEvent::EventType type) const
{                                                                     // 2012 0xa5f1a0
    if (pListener == 0)
        return;
    if (type == GFxEvent::KeyDown)
        pListener->OnKeyDown(sc, code, ascii, wcharCode, KeyboardIndex);
    else if (type == GFxEvent::KeyUp)
        pListener->OnKeyUp(sc, code, ascii, wcharCode, KeyboardIndex);
}

void GFxKeyboardState::UpdateListeners(short code, unsigned char ascii, unsigned int wcharCode)
{                                                                     // 2012 0xa5f200
    if (pListener)
        pListener->Update(code, ascii, wcharCode, KeyboardIndex);
}

// ---------------------------------------------------------------------------------------------
// GFxInputEventsQueue
// ---------------------------------------------------------------------------------------------

GFxInputEventsQueue::GFxInputEventsQueue()
    : Entries(0), Count(0), Capacity(0), Head(0)
{
}

GFxInputEventsQueue::~GFxInputEventsQueue()
{
    delete[] Entries;
}

GFxInputEventsQueue::QueueEntry* GFxInputEventsQueue::AddEmptyQueueEntry()  // 2012 0xa01db0
{
    if (Head > 0 && Head == Count)
    {
        Head = Count = 0;
    }
    if (Count >= Capacity)
    {
        unsigned int cap = Capacity ? Capacity * 2 : 16;
        QueueEntry* next = new QueueEntry[cap];
        for (unsigned int i = 0; i < Count; ++i)
            next[i] = Entries[i];
        delete[] Entries;
        Entries = next;
        Capacity = cap;
    }
    QueueEntry* e = &Entries[Count++];
    memset(e, 0, sizeof(*e));
    return e;
}

void GFxInputEventsQueue::AddMouseMove(unsigned int index, const GPoint<float>& p)
{                                                                     // 2012 0xa01e10
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Mouse;
    e->MouseData.x = p.x;
    e->MouseData.y = p.y;
    e->MouseData.MouseIndex = index;
}

void GFxInputEventsQueue::AddMouseButtonEvent(unsigned int index, const GPoint<float>& p,
                                              unsigned int buttons, unsigned int changedMask)
{                                                                     // 2012 0xa01e50
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Mouse;
    e->MouseData.x = p.x;
    e->MouseData.y = p.y;
    e->MouseData.Buttons = buttons;
    e->MouseData.ChangedButtons = changedMask;
    e->MouseData.MouseIndex = index;
}

void GFxInputEventsQueue::AddMouseWheel(unsigned int index, const GPoint<float>& p, int delta)
{                                                                     // 2012 0xa01eb0
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Mouse;
    e->MouseData.x = p.x;
    e->MouseData.y = p.y;
    e->MouseData.ScrollDelta = delta;
    e->MouseData.MouseIndex = index;
}

void GFxInputEventsQueue::AddKeyDown(short code, unsigned char ascii, GFxSpecialKeysState special,
                                     unsigned char keyboardIndex)     // 2012 0xa02000
{
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Key;
    e->KeyData.Code = code;
    e->KeyData.AsciiCode = ascii;
    e->KeyData.SpecialKeysState = special;
    e->KeyData.KeyboardIndex = keyboardIndex;
    e->KeyData.bKeyDown = true;
}

void GFxInputEventsQueue::AddKeyUp(short code, unsigned char ascii, GFxSpecialKeysState special,
                                   unsigned char keyboardIndex)       // 2012 0xa02040
{
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Key;
    e->KeyData.Code = code;
    e->KeyData.AsciiCode = ascii;
    e->KeyData.SpecialKeysState = special;
    e->KeyData.KeyboardIndex = keyboardIndex;
    e->KeyData.bKeyDown = false;
}

void GFxInputEventsQueue::AddCharTyped(unsigned int wcharCode, unsigned char keyboardIndex)
{                                                                     // 2012 0xa02080
    QueueEntry* e = AddEmptyQueueEntry();
    e->EntryKind = QueueEntry::Char;
    e->KeyData.WcharCode = wcharCode;
    e->KeyData.KeyboardIndex = keyboardIndex;
}

const GFxInputEventsQueue::QueueEntry* GFxInputEventsQueue::GetEntry()  // 2012 0xa01f10
{
    if (Head >= Count)
        return 0;
    return &Entries[Head++];
}

// ---------------------------------------------------------------------------------------------
// The AS2 side: AsBroadcaster, Key, Mouse.
// ---------------------------------------------------------------------------------------------

static GFxInputCensus GInputCensus;

GFxInputCensus& GFxInputGetCensus() { return GInputCensus; }
void GFxInputResetCensus() { memset(&GInputCensus, 0, sizeof(GInputCensus)); }

namespace
{

const char* const kListeners = "_listeners";

GASArrayObject* GetListenerArray(GASEnvironment* env, GASObjectInterface* obj)
{
    if (obj == 0)
        return 0;
    GASValue v;
    if (!obj->GetConstMemberRaw(env->GetSC(), kListeners, &v))
        return 0;
    GASObject* o = v.ToObject(env);
    if (o == 0 || o->GetObjectType() != Object_Array)
        return 0;
    return (GASArrayObject*)o;
}

// 2012 0xa6eb60. The instance half of AsBroadcaster.initialize: a fresh `_listeners` array on the
// object. Retail's Initialize (0xa6f2b0) is this plus InitializeProto (0xa6eac0), which copies the
// three methods off AsBroadcaster's own prototype.
void BroadcasterInitializeInstance(GASGlobalContext* gc, GASObjectInterface* obj)
{
    if (obj == 0)
        return;
    GASArrayObject* arr = new GASArrayObject(gc->GetSC(), gc->GetPrototype(GASGlobalContext::Proto_Array));
    GASValue v;
    v.SetAsObject(arr);
    obj->SetConstMemberRaw(gc->GetSC(), kListeners, v,
                           GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

// 2012 0xa6ebf0. Refuses a duplicate, which is what keeps a clip that re-registers on every frame
// from being called N times.
bool BroadcasterAddListener(GASEnvironment* env, GASObjectInterface* self,
                            GASObjectInterface* listener)
{
    if (self == 0 || listener == 0)
        return false;
    GASArrayObject* arr = GetListenerArray(env, self);
    if (arr == 0)
        return true;
    for (unsigned int i = 0; i < arr->GetSize(); ++i)
    {
        const GASValue* e = arr->GetElementPtr(i);
        if (e && e->ToObjectInterface(env) == listener)
            return false;
    }
    GASValue v;
    GASObject* o = listener->ToASObject();
    GFxASCharacter* ch = listener->ToASCharacter();
    if (ch)
        v.SetAsCharacter(ch);
    else if (o)
        v.SetAsObject(o);
    else
        return false;
    arr->PushBack(v);
    ++GInputCensus.ListenersAdded;
    return true;
}

// 2012 0xa6ecf0
bool BroadcasterRemoveListener(GASEnvironment* env, GASObjectInterface* self,
                               GASObjectInterface* listener)
{
    if (self == 0 || listener == 0)
        return false;
    GASArrayObject* arr = GetListenerArray(env, self);
    if (arr == 0)
        return false;
    for (unsigned int i = 0; i < arr->GetSize(); ++i)
    {
        const GASValue* e = arr->GetElementPtr(i);
        if (e && e->ToObjectInterface(env) == listener)
        {
            arr->RemoveElements(i, 1);
            return true;
        }
    }
    return false;
}

// 2012 0xa6ee70 (BroadcastMessageWithCallback) with retail's own LocalInvokeCallback (0xa70a50),
// which is what the plain BroadcastMessage at 0xa6f2e0 installs: invoke the named method on each
// listener with the argument window the caller already pushed.
//
// The snapshot matters and is retail's: a listener that removes itself from inside its own handler
// must not shorten the walk under it.
bool BroadcasterBroadcast(GASEnvironment* env, GASObjectInterface* self, const GASString& msg,
                          const GASValue* args, unsigned int nargs)
{
    if (self == 0)
        return false;
    GASArrayObject* arr = GetListenerArray(env, self);
    if (arr == 0)
        return false;
    const unsigned int count = arr->GetSize();
    if (count == 0)
        return true;
    GASValue* snapshot = new GASValue[count];
    for (unsigned int i = 0; i < count; ++i)
    {
        const GASValue* e = arr->GetElementPtr(i);
        if (e)
            snapshot[i] = *e;
    }
    for (unsigned int i = 0; i < count; ++i)
    {
        GASObjectInterface* listener = snapshot[i].ToObjectInterface(env);
        if (listener == 0)
            continue;
        GASValue fnVal;
        if (!listener->GetMember(env, msg, &fnVal))
            continue;
        GASFunctionObject* fn = fnVal.GetFunction();
        if (fn == 0)
            continue;
        for (unsigned int a = 0; a < nargs; ++a)
            env->Push(args[nargs - 1 - a]);
        GASValue result;
        GASFnCall call(&result, listener, env, (int)nargs, env->GetTopIndex(), &msg);
        fn->Invoke(call);
        env->Drop((int)nargs);
        ++GInputCensus.KeyListenerCalls;
    }
    delete[] snapshot;
    return true;
}

void BroadcasterAddListenerFn(const GASFnCall& fn)                    // 2012 0xa6f170
{
    bool ok = fn.GetNumArgs() >= 1
        && BroadcasterAddListener(fn.pEnv, fn.pThis, fn.Arg(0).ToObjectInterface(fn.pEnv));
    fn.pResult->SetBool(ok);
}

void BroadcasterRemoveListenerFn(const GASFnCall& fn)                 // 2012 0xa6f1c0
{
    bool ok = fn.GetNumArgs() >= 1
        && BroadcasterRemoveListener(fn.pEnv, fn.pThis, fn.Arg(0).ToObjectInterface(fn.pEnv));
    fn.pResult->SetBool(ok);
}

void BroadcasterBroadcastMessageFn(const GASFnCall& fn)               // 2012 0xa6f330
{
    if (fn.GetNumArgs() < 1)
    {
        fn.pResult->SetBool(false);
        return;
    }
    GASString msg = fn.Arg(0).ToString(fn.pEnv);
    const int extra = fn.GetNumArgs() - 1;
    GASValue* args = extra > 0 ? new GASValue[extra] : 0;
    for (int i = 0; i < extra; ++i)
        args[i] = fn.Arg(i + 1);
    bool ok = BroadcasterBroadcast(fn.pEnv, fn.pThis, msg, args, (unsigned int)(extra > 0 ? extra : 0));
    delete[] args;
    fn.pResult->SetBool(ok);
}

void BroadcasterInitializeFn(const GASFnCall& fn)                     // 2012 0xa6f3e0 / 0xa6f2b0
{
    if (fn.GetNumArgs() < 1)
        return;
    GASObjectInterface* target = fn.Arg(0).ToObjectInterface(fn.pEnv);
    if (target == 0)
        return;
    GASGlobalContext* gc = fn.pEnv->GetGC();
    BroadcasterInitializeInstance(gc, target);
    // InitializeProto: the three methods on the object itself, which is what makes
    // `AsBroadcaster.initialize(o); o.addListener(x)` work on a plain object.
    GASValue v;
    v.SetAsFunction(gc->NewCFunction(BroadcasterAddListenerFn));
    target->SetConstMemberRaw(gc->GetSC(), "addListener", v,
                              GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    v.SetAsFunction(gc->NewCFunction(BroadcasterRemoveListenerFn));
    target->SetConstMemberRaw(gc->GetSC(), "removeListener", v,
                              GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    v.SetAsFunction(gc->NewCFunction(BroadcasterBroadcastMessageFn));
    target->SetConstMemberRaw(gc->GetSC(), "broadcastMessage", v,
                              GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

} // namespace

// ---------------------------------------------------------------------------------------------
// The Key object. Retail's is GASKeyCtorFunction (2012 0xa8a3a0), a constructor function that is
// also a GFxKeyboardState::IListener; the four static methods come off GASNameFunction's table and
// the eighteen key constants are set on it by name. This is the same object with GASObject as the
// base rather than GASFunctionObject: nothing in the cook calls `new Key()`, and `Key` is only ever
// read as a namespace (`Key.addListener`, `Key.getCode()`), which is what 2,180 strings of the menu
// asset show. Stated as deviation 1 in agentDG.md.
// ---------------------------------------------------------------------------------------------

class GASKeyObject : public GASObject, public GFxKeyboardState::IListener
{
public:
    enum { MaxKeyboards = 4 };

    GASKeyObject(GASStringContext* sc, GASObject* proto, GFxMovieRoot* root)
        : GASObject(sc, proto), pMovieRoot(root)
    {
        memset(LastCode, 0, sizeof(LastCode));
        memset(LastAscii, 0, sizeof(LastAscii));
        memset(LastWchar, 0, sizeof(LastWchar));
    }

    // 2012 0xa8a0e0 - the "what did the last key do" triple, one per keyboard index.
    virtual void Update(int code, unsigned char ascii, unsigned int wcharCode,
                        unsigned char keyboardIndex)
    {
        if (keyboardIndex >= MaxKeyboards)
            return;
        LastCode[keyboardIndex] = code;
        LastAscii[keyboardIndex] = ascii;
        LastWchar[keyboardIndex] = wcharCode;
    }

    virtual void OnKeyDown(GASStringContext* sc, int code, unsigned char ascii,
                           unsigned int wcharCode, unsigned char keyboardIndex) // 2012 0xa8a9b0
    {
        NotifyListeners(sc, code, ascii, wcharCode, keyboardIndex, true);
    }

    virtual void OnKeyUp(GASStringContext* sc, int code, unsigned char ascii,
                         unsigned int wcharCode, unsigned char keyboardIndex)   // 2012 0xa8a9e0
    {
        NotifyListeners(sc, code, ascii, wcharCode, keyboardIndex, false);
    }

    // 2012 0xa8a890. Record the triple first - a listener's onKeyDown calls Key.getCode() and must
    // see this key, not the previous one - then broadcast.
    void NotifyListeners(GASStringContext* sc, int code, unsigned char ascii,
                         unsigned int wcharCode, unsigned char keyboardIndex, bool bDown);

    int          LastCode[MaxKeyboards];
    unsigned char LastAscii[MaxKeyboards];
    unsigned int LastWchar[MaxKeyboards];
    GFxMovieRoot* pMovieRoot;
};

namespace
{

GASKeyObject* KeyObjectOf(const GASFnCall& fn)
{
    GASObjectInterface* self = fn.pThis;
    return self ? (GASKeyObject*)self->ToASObject() : 0;
}

unsigned int KeyboardIndexArg(const GASFnCall& fn, int argIndex)
{
    if (fn.GetNumArgs() > argIndex)
    {
        int v = fn.Arg(argIndex).ToInt32(fn.pEnv);
        if (v >= 0 && v < GASKeyObject::MaxKeyboards)
            return (unsigned int)v;
    }
    return 0;
}

void KeyGetCode(const GASFnCall& fn)                                  // 2012 0xa8a170
{
    GASKeyObject* key = KeyObjectOf(fn);
    fn.pResult->SetInt(key ? key->LastCode[KeyboardIndexArg(fn, 0)] : 0);
}

void KeyGetAscii(const GASFnCall& fn)                                 // 2012 0xa8a110
{
    GASKeyObject* key = KeyObjectOf(fn);
    fn.pResult->SetInt(key ? (int)key->LastAscii[KeyboardIndexArg(fn, 0)] : 0);
}

void KeyIsDown(const GASFnCall& fn);
void KeyIsToggled(const GASFnCall& fn);

// Mouse: show/hide are counters on the movie root, and the class is a broadcaster like Key.
void MouseShow(const GASFnCall& fn)                                   // 2012 0xa6f6b0
{
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    if (root)
        root->SetMouseCursorCount(root->GetMouseCursorCount() + 1);
    fn.pResult->SetInt(root ? (int)root->GetMouseCursorCount() : 0);
}

void MouseHide(const GASFnCall& fn)                                   // 2012 0xa6f740
{
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    if (root && root->GetMouseCursorCount() > 0)
        root->SetMouseCursorCount(root->GetMouseCursorCount() - 1);
    fn.pResult->SetInt(root ? (int)root->GetMouseCursorCount() : 0);
}

} // namespace
// ------ DG input tail ------

namespace
{

void KeyIsDown(const GASFnCall& fn)                                   // 2012 0xa8a1d0
{
    if (fn.GetNumArgs() < 1)
    {
        fn.pEnv->LogScriptError("Key.isDown needs one argument (the key code)");
        fn.pResult->SetBool(false);
        return;
    }
    const int code = fn.Arg(0).ToInt32(fn.pEnv);
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    GFxKeyboardState* kb = root ? root->GetKeyboardState(KeyboardIndexArg(fn, 1)) : 0;
    fn.pResult->SetBool(kb != 0 && kb->IsKeyDown(code));
}

void KeyIsToggled(const GASFnCall& fn)                                // 2012 0xa8a290
{
    if (fn.GetNumArgs() < 1)
    {
        fn.pEnv->LogScriptError("Key.isToggled needs one argument (the key code)");
        fn.pResult->SetBool(false);
        return;
    }
    const int code = fn.Arg(0).ToInt32(fn.pEnv);
    GFxMovieRoot* root = fn.pEnv->GetMovieRoot();
    GFxKeyboardState* kb = root ? root->GetKeyboardState(KeyboardIndexArg(fn, 1)) : 0;
    fn.pResult->SetBool(kb != 0 && kb->IsKeyToggled(code));
}

void AddFn(GASObject* obj, GASGlobalContext* gc, const char* name, GASCFunctionPtr f)
{
    GASValue v;
    v.SetAsFunction(gc->NewCFunction(f));
    obj->SetConstMemberRaw(gc->GetSC(), name, v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

void AddConst(GASObject* obj, GASGlobalContext* gc, const char* name, int value)
{
    GASValue v;
    v.SetInt(value);
    obj->SetConstMemberRaw(gc->GetSC(), name, v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
}

} // namespace

// 2012 0xa8a890. The triple is written before the broadcast because a listener's onKeyDown calls
// Key.getCode() and has to see the key that woke it, which is the ordering in the retail body too
// (the three stores precede GFxEventId::GetFunctionName).
void GASKeyObject::NotifyListeners(GASStringContext* sc, int code, unsigned char ascii,
                                   unsigned int wcharCode, unsigned char keyboardIndex, bool bDown)
{
    (void)sc;
    Update(code, ascii, wcharCode, keyboardIndex);
    if (pMovieRoot == 0)
        return;
    GASEnvironment* env = pMovieRoot->GetASEnvironment();
    // Retail pushes the keyboard index as the single argument when the SWF version is 7 or later
    // (`*(_BYTE *)(*((_DWORD *)v13 + 30) + 684) == 1` in the decompile is that version gate); it is
    // what a CLIK listener reads as `onKeyDown(controllerIdx)`.
    GASValue arg;
    arg.SetInt((int)keyboardIndex);
    GASString msg = env->CreateString(bDown ? "onKeyDown" : "onKeyUp");
    GFxInputBroadcast(env, this, msg, &arg, 1);
}

// ---------------------------------------------------------------------------------------------
// Installation. Called from GASGlobalContext::InitStandardLibrary.
// ---------------------------------------------------------------------------------------------

void GFxInputInstall(GASGlobalContext* gc, GASObject* global)
{
    GASStringContext* sc = gc->GetSC();
    GASObject* objProto = gc->GetPrototype(GASGlobalContext::Proto_Object);

    // AsBroadcaster itself, which the content calls as `AsBroadcaster.initialize(this)`.
    {
        GASObject* bc = new GASObject(sc, objProto);
        AddFn(bc, gc, "initialize", BroadcasterInitializeFn);
        AddFn(bc, gc, "addListener", BroadcasterAddListenerFn);
        AddFn(bc, gc, "removeListener", BroadcasterRemoveListenerFn);
        AddFn(bc, gc, "broadcastMessage", BroadcasterBroadcastMessageFn);
        GASValue v;
        v.SetAsObject(bc);
        global->SetConstMemberRaw(sc, "AsBroadcaster", v,
                                  GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    // Key. 2012 0xa8a3a0: AsBroadcaster.initialize, the eighteen constants, the four statics, then
    // GFxMovieRoot::SetKeyboardListener (0xa01730) - in that order.
    {
        GASKeyObject* key = new GASKeyObject(sc, objProto, gc->GetMovieRoot());
        BroadcasterInitializeInstance(gc, key);
        AddFn(key, gc, "addListener", BroadcasterAddListenerFn);
        AddFn(key, gc, "removeListener", BroadcasterRemoveListenerFn);
        AddFn(key, gc, "broadcastMessage", BroadcasterBroadcastMessageFn);
        AddConst(key, gc, "BACKSPACE", 8);
        AddConst(key, gc, "CAPSLOCK", 20);
        AddConst(key, gc, "CONTROL", 17);
        AddConst(key, gc, "DELETEKEY", 46);
        AddConst(key, gc, "DOWN", 40);
        AddConst(key, gc, "END", 35);
        AddConst(key, gc, "ENTER", 13);
        AddConst(key, gc, "ESCAPE", 27);
        AddConst(key, gc, "HOME", 36);
        AddConst(key, gc, "INSERT", 45);
        AddConst(key, gc, "LEFT", 37);
        AddConst(key, gc, "PGDN", 34);
        AddConst(key, gc, "PGUP", 33);
        AddConst(key, gc, "RIGHT", 39);
        AddConst(key, gc, "SHIFT", 16);
        AddConst(key, gc, "SPACE", 32);
        AddConst(key, gc, "TAB", 9);
        AddConst(key, gc, "UP", 38);
        AddFn(key, gc, "getAscii", KeyGetAscii);
        AddFn(key, gc, "getCode", KeyGetCode);
        AddFn(key, gc, "isDown", KeyIsDown);
        AddFn(key, gc, "isToggled", KeyIsToggled);
        GASValue v;
        v.SetAsObject(key);
        global->SetConstMemberRaw(sc, "Key", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        if (gc->GetMovieRoot())
            gc->GetMovieRoot()->SetKeyboardListener(key);
    }

    // Stage: the visible frame in the movie's own pixels, which is what content that lays itself
    // out over the whole screen reads. A broadcaster too (onResize), and scaleMode / align are the
    // movie root's own view settings under their AS2 names.
    {
        GASObject* stage = new GASObject(sc, objProto);
        BroadcasterInitializeInstance(gc, stage);
        AddFn(stage, gc, "addListener", BroadcasterAddListenerFn);
        AddFn(stage, gc, "removeListener", BroadcasterRemoveListenerFn);
        AddFn(stage, gc, "broadcastMessage", BroadcasterBroadcastMessageFn);
        GFxMovieRoot* root = gc->GetMovieRoot();
        float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
        if (root)
            root->GetVisibleFrameRectPixels(&x0, &y0, &x1, &y1);
        AddConst(stage, gc, "width", (int)(x1 - x0));
        AddConst(stage, gc, "height", (int)(y1 - y0));
        GASValue sv;
        sv.SetString(gc->GetSC()->CreateString("showAll"));
        stage->SetConstMemberRaw(sc, "scaleMode", sv, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        sv.SetString(gc->GetSC()->CreateString(""));
        stage->SetConstMemberRaw(sc, "align", sv, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
        GASValue v;
        v.SetAsObject(stage);
        global->SetConstMemberRaw(sc, "Stage", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }

    // Mouse. 2012 0xa70460: the same broadcaster plus show/hide. The four mouse notifications
    // (onMouseDown/Up/Move/Wheel, 0xa6f890..0xa6fac0) are broadcast from ProcessMouse.
    {
        GASObject* mouse = new GASObject(sc, objProto);
        BroadcasterInitializeInstance(gc, mouse);
        AddFn(mouse, gc, "addListener", BroadcasterAddListenerFn);
        AddFn(mouse, gc, "removeListener", BroadcasterRemoveListenerFn);
        AddFn(mouse, gc, "broadcastMessage", BroadcasterBroadcastMessageFn);
        AddFn(mouse, gc, "show", MouseShow);
        AddFn(mouse, gc, "hide", MouseHide);
        GASValue v;
        v.SetAsObject(mouse);
        global->SetConstMemberRaw(sc, "Mouse", v, GASPropFlags(GASPropFlags::PropFlag_DontEnum));
    }
}

void GFxInputShutdown(GASGlobalContext* gc)
{
    if (gc && gc->GetMovieRoot())
        gc->GetMovieRoot()->SetKeyboardListener(0);
}

void GFxInputBroadcast(GASEnvironment* env, GASObjectInterface* self, const GASString& msg,
                       const GASValue* args, unsigned int nargs)
{
    BroadcasterBroadcast(env, self, msg, args, nargs);
}

// The Mouse class's own broadcast, used by ProcessMouse. Kept here so the listener-array walk is
// the one function.
static void GFxInputBroadcastMouse(GASEnvironment* env, const char* message, const GASValue* args,
                                   unsigned int nargs)
{
    if (env == 0 || env->GetGC() == 0)
        return;
    GASObject* global = env->GetGC()->GetGlobalObject();
    if (global == 0)
        return;
    GASValue mouseVal;
    if (!global->GetConstMemberRaw(env->GetSC(), "Mouse", &mouseVal))
        return;
    GASObject* mouse = mouseVal.ToObject(env);
    if (mouse == 0)
        return;
    GASString msg = env->CreateString(message);
    BroadcasterBroadcast(env, mouse, msg, args, nargs);
}

// ---------------------------------------------------------------------------------------------
// GFxMovieRoot's input half.
// ---------------------------------------------------------------------------------------------

// The movie's own pixel rectangle that the viewport shows, which is the one number both halves of
// the player need: BeginDisplay is given it (2012 0xa07aa0) and a viewport-space mouse position is
// mapped back through it. SM_ShowAll keeps the aspect ratio and shows MORE than the frame on the
// long axis rather than letterboxing, which is what makes this not simply the frame rect.
void GFxMovieRoot::GetVisibleFrameRectPixels(float* px0, float* py0, float* px1, float* py1) const
{
    const float movieW = pDefImpl ? pDefImpl->GetWidth() : 0.0f;
    const float movieH = pDefImpl ? pDefImpl->GetHeight() : 0.0f;
    float x0 = 0.0f, y0 = 0.0f, x1 = movieW, y1 = movieH;
    if (movieW > 0.0f && movieH > 0.0f && Viewport.Width > 0 && Viewport.Height > 0)
    {
        const float vw = (float)Viewport.Width;
        const float vh = (float)Viewport.Height;
        if (ScaleMode == GFxMovieView::SM_NoScale)
        {
            x1 = vw;
            y1 = vh;
        }
        else if (ScaleMode == GFxMovieView::SM_ShowAll || ScaleMode == GFxMovieView::SM_NoBorder)
        {
            const float sx = vw / movieW;
            const float sy = vh / movieH;
            const float scale = (ScaleMode == GFxMovieView::SM_ShowAll)
                                    ? (sx < sy ? sx : sy)
                                    : (sx > sy ? sx : sy);
            const float visW = vw / scale;
            const float visH = vh / scale;
            x0 = (movieW - visW) * 0.5f;
            y0 = (movieH - visH) * 0.5f;
            if (Alignment == GFxMovieView::Align_TopLeft || Alignment == GFxMovieView::Align_TopCenter
                || Alignment == GFxMovieView::Align_TopRight)
                y0 = 0.0f;
            if (Alignment == GFxMovieView::Align_BottomLeft
                || Alignment == GFxMovieView::Align_BottomCenter
                || Alignment == GFxMovieView::Align_BottomRight)
                y0 = movieH - visH;
            if (Alignment == GFxMovieView::Align_TopLeft || Alignment == GFxMovieView::Align_CenterLeft
                || Alignment == GFxMovieView::Align_BottomLeft)
                x0 = 0.0f;
            if (Alignment == GFxMovieView::Align_TopRight
                || Alignment == GFxMovieView::Align_CenterRight
                || Alignment == GFxMovieView::Align_BottomRight)
                x0 = movieW - visW;
            x1 = x0 + visW;
            y1 = y0 + visH;
        }
    }
    if (px0) *px0 = x0;
    if (py0) *py0 = y0;
    if (px1) *px1 = x1;
    if (py1) *py1 = y1;
}

// A viewport-space position in twips: the inverse of the viewport matrix BeginDisplay installs,
// then the 20 twips a pixel. Retail composes the same two steps inline in HandleEvent
// (`this+30 * x + this+32`, then `* 20.0`).
GPoint<float> GFxMovieRoot::ViewportToTwips(float x, float y) const
{
    float x0, y0, x1, y1;
    GetVisibleFrameRectPixels(&x0, &y0, &x1, &y1);
    GPoint<float> p;
    const float vw = Viewport.Width > 0 ? (float)Viewport.Width : 1.0f;
    const float vh = Viewport.Height > 0 ? (float)Viewport.Height : 1.0f;
    p.x = (x0 + (x - (float)Viewport.Left) * (x1 - x0) / vw) * 20.0f;
    p.y = (y0 + (y - (float)Viewport.Top) * (y1 - y0) / vh) * 20.0f;
    return p;
}

GFxKeyboardState* GFxMovieRoot::GetKeyboardState(unsigned int index)  // 2012 0x9cd8b0
{
    return index < MaxKeyboards ? &KeyboardStates[index] : 0;
}

void GFxMovieRoot::SetKeyboardListener(GFxKeyboardState::IListener* l)  // 2012 0xa01730
{
    for (unsigned int i = 0; i < MaxKeyboards; ++i)
    {
        KeyboardStates[i].SetKeyboardIndex((unsigned char)i);
        KeyboardStates[i].SetListener(l);
    }
}

// DISHONORED(port): 2012 0xa042d0. Every arm queues and answers HE_Completed (Handled |
// NoDefaultAction, the 3 the decompile returns); only an event this movie has no use for falls
// through to HE_NotHandled, which is what lets the game act on the key instead.
unsigned int GFxMovieRoot::HandleEvent(const GFxEvent& e)
{
    GFxInputCensus& census = GFxInputGetCensus();
    switch (e.Type)
    {
    case GFxEvent::KeyDown:
    case GFxEvent::KeyUp:
    {
        const GFxKeyEvent& k = (const GFxKeyEvent&)e;
        GFxKeyboardState* kb = GetKeyboardState(k.KeyboardIndex);
        if (kb != 0 && k.SpecialKeysState.States != 0)
        {
            // Retail mirrors the three lock keys out of the special-keys byte on every key event
            // (the SetKeyToggled(144/20/145) triple at the head of the body), because the platform
            // reports them as modifiers rather than as key events of their own.
            kb->SetKeyToggled(144, (k.SpecialKeysState.States & GFxSpecialKeysState::Key_NumToggled) != 0);
            kb->SetKeyToggled(20, (k.SpecialKeysState.States & GFxSpecialKeysState::Key_CapsToggled) != 0);
            kb->SetKeyToggled(145, (k.SpecialKeysState.States & GFxSpecialKeysState::Key_ScrollToggled) != 0);
        }
        if (e.Type == GFxEvent::KeyDown)
        {
            if (kb)
                kb->SetKeyDown(k.KeyCode, k.AsciiCode, k.SpecialKeysState);
            InputQueue.AddKeyDown((short)k.KeyCode, k.AsciiCode, k.SpecialKeysState,
                                  k.KeyboardIndex);
            ++census.KeyDowns;
            // A printable key carries its character with it, which is retail's
            // "WcharCode >= 32 && != 127 -> AddCharTyped" arm.
            if (k.WcharCode >= 32 && k.WcharCode != 127)
            {
                InputQueue.AddCharTyped(k.WcharCode, k.KeyboardIndex);
                ++census.CharsTyped;
            }
        }
        else
        {
            if (kb)
                kb->SetKeyUp(k.KeyCode, k.AsciiCode, k.SpecialKeysState);
            InputQueue.AddKeyUp((short)k.KeyCode, k.AsciiCode, k.SpecialKeysState,
                                k.KeyboardIndex);
            ++census.KeyUps;
        }
        bDirty = true;
        ++census.EventsHandled;
        return GFxMovieView::HE_Completed;
    }
    case GFxEvent::CharEvent:
    {
        const GFxCharEvent& c = (const GFxCharEvent&)e;
        InputQueue.AddCharTyped(c.WcharCode, c.KeyboardIndex);
        ++census.CharsTyped;
        ++census.EventsHandled;
        bDirty = true;
        return GFxMovieView::HE_Completed;
    }
    case GFxEvent::MouseMove:
    case GFxEvent::MouseDown:
    case GFxEvent::MouseUp:
    case GFxEvent::MouseWheel:
    {
        const GFxMouseEvent& m = (const GFxMouseEvent&)e;
        if (m.MouseIndex >= MaxMice)
        {
            ++census.EventsNotHandled;
            return GFxMovieView::HE_NotHandled;
        }
        // The viewport-to-twips map is the one the display half inverts: the viewport matrix scales
        // and offsets into the movie's own pixels, and twips are 20 per pixel.
        GPoint<float> p = ViewportToTwips(m.x, m.y);
        if (e.Type == GFxEvent::MouseMove)
            InputQueue.AddMouseMove(m.MouseIndex, p);
        else if (e.Type == GFxEvent::MouseWheel)
            InputQueue.AddMouseWheel(m.MouseIndex, p, (int)m.ScrollDelta);
        else
            InputQueue.AddMouseButtonEvent(m.MouseIndex, p, 1u << m.Button,
                                           e.Type == GFxEvent::MouseUp ? 0x80u : 0u);
        ++census.MouseEvents;
        ++census.EventsHandled;
        bDirty = true;
        return GFxMovieView::HE_Completed;
    }
    case GFxEvent::SetFocus:
        bMovieFocused = true;
        ++census.EventsHandled;
        return GFxMovieView::HE_Handled;
    case GFxEvent::KillFocus:
        bMovieFocused = false;
        // Every key that was down is released with the focus, which is retail's ResetState here.
        for (unsigned int i = 0; i < MaxKeyboards; ++i)
            KeyboardStates[i].ResetState();
        ++census.EventsNotHandled;
        return GFxMovieView::HE_NotHandled;
    default:
        break;
    }
    ++census.EventsNotHandled;
    return GFxMovieView::HE_NotHandled;
}

// DISHONORED(port): 2012 0xa0cfa0. The level walk first (a clip's own onKeyDown / onKeyUp), then
// the Key broadcaster, then the focus key.
void GFxMovieRoot::ProcessKeyboard(const GFxInputEventsQueue::QueueEntry& entry)
{
    const GFxInputEventsQueue::KeyEntry& k = entry.KeyData;
    if (entry.EntryKind == GFxInputEventsQueue::QueueEntry::Char)
    {
        // Retail hands a typed character to the focused character's OnCharEvent (vtable slot 38 in
        // the decompile). Text input belongs to agent CB's edit-text package, not this one, and no
        // menu in the cook focuses an editable field, so the event is counted and dropped.
        return;
    }
    if (k.Code == 0)
        return;

    // DISHONORED(bringup): retail walks the movie levels first and calls vtable slot 164
    // (GFxCharacter::OnKeyEvent) on each, which exists to run a DefineButton2 record's `on(keyPress)`
    // actions and to let a focused button take Enter. The retail main menu asset holds ONE
    // DefineButton2 in 477 tags and none of its records carries a keyPress condition, so that walk
    // delivers nothing here; it goes with GFxButtonCharacter, which agentDG.md hands over.

    GFxKeyboardState* kb = GetKeyboardState(k.KeyboardIndex);
    if (kb != 0)
    {
        kb->NotifyListeners(pGC->GetSC(), k.Code, k.AsciiCode, k.WcharCode,
                            k.bKeyDown ? GFxEvent::KeyDown : GFxEvent::KeyUp);
    }
    // DISHONORED(bringup): GFxMovieRoot::ProcessFocusKey (2012 0xa0a230, 4,941 bytes) walks the tab
    // order and moves the focus on Tab and the arrow keys. The Dishonored menus move their own
    // selection from their own onKeyDown handlers - measured: the menu asset's strings carry `Key`,
    // `addListener` and `getCode`, and it holds one DefineButton2 in 477 tags - so the focus model
    // is not what makes them operable and it is not reconstructed here. agentDG.md deviation 3.
}

// DISHONORED(port, agent EH): 2013 0xa077d0 (2012 0xa10d80). The queue drain is the first half; the
// second half is the one this tree did not have, and it is what makes the mouse answer a display list
// that changed under a pointer that did not move.
//
// Retail's body, after the drain:
//
//     if ( (this[9316] & 0x80) != 0 && (processed & allMice) != allMice )
//         for ( i = 0; i < MouseCursorCount; ++i )
//             if ( !(processed & (1 << i)) && (state[i].Flags & 0x10) )
//             {
//                 state[i].PrevButtons = state[i].CurButtons;
//                 top = GetTopMostEntity((state[i].X, state[i].Y), i, false, 0);  // 0x9fabf0
//                 state[i].SetTopmostEntity(top);                                  // 0x9fc460
//                 GFxMovieRoot::CheckMouseCursorType(this, i, top);
//                 GFx_GenerateMouseButtonEvents(i, &state[i], ...);                // 0xa5ad10
//             }
//     this[9316] &= ~0x80;
//
// and bit 0x80 is set once per GFxMovieRoot::Advance (0xa088d8), after this function has run. So the
// hit test is re-asked once a frame from the STORED pointer position, and rollOver / rollOut follow
// the content rather than the hand.
//
// Measured, same driver, same schedule, two binaries (build/agentEH/pk2_before_log.txt against
// pk2_after_log.txt): the pointer is parked on QUIT GAME while the start screen is still up and is
// never touched again, and the menu bar then animates in under it. Without this arm the interface
// asks what is under the pointer 45 times, ALL of them before the bar exists, resolves 0 targets and
// dispatches 0 rollOver. With it: 9006 hit tests, 7399 resolved, and the four rollOver retail
// dispatches as the four entries sweep under the cursor.
//
// What it does NOT fix, so that nobody reads it as the cure: an entry the pointer is ALREADY resting
// on cannot take the selection back after the keyboard moved the selection off it, because
// topmost == active and retail's last block is `if (!CurButtons && topmost != active)`. That is
// retail's behaviour too - agentEH.md 6.
void GFxMovieRoot::ProcessInput()
{
    if (pGC == 0)
        return;
    unsigned int processedMice = 0;
    while (const GFxInputEventsQueue::QueueEntry* e = InputQueue.GetEntry())
    {
        if (e->EntryKind == GFxInputEventsQueue::QueueEntry::Mouse)
            ProcessMouse(*e, &processedMice);
        else
            ProcessKeyboard(*e);
        // A handler can queue more work (a clip that moves its selection and then calls gotoAndStop
        // on the new one), and the action queue has to settle between two input events or the second
        // runs against a half-built display list.
        DrainActionSessions();
    }
    InputQueue.Clear();

    const unsigned int mice = MouseCursorCount < (unsigned int)MaxMice ? MouseCursorCount
                                                                       : (unsigned int)MaxMice;
    const unsigned int allMice = mice != 0 ? ((1u << mice) - 1u) : 0u;
    if (bMouseStateDirty && (processedMice & allMice) != allMice)
    {
        for (unsigned int i = 0; i < mice; ++i)
        {
            if ((processedMice & (1u << i)) != 0)
                continue;
            GFxMouseState* state = GetMouseStateStruct(i);
            if (state == 0 || !state->IsUpdated())
                continue;
            state->CarryButtons();
            GPoint<float> pt(state->GetX(), state->GetY());
            GFxASCharacter* topmost = GetTopMostEntity(pt, i, false, 0);
            if (topmost != 0)
                topmost->AddRef();
            state->SetTopmostEntity(topmost);
            // Retail's GFxMovieRoot::CheckMouseCursorType goes here. It is the hand cursor, and
            // Dishonored's cursor is a movie clip the global movie attaches (agentDQ.md deviation 1
            // and hand-over 3), so it is left out rather than faked.
            GFx_GenerateMouseButtonEvents((unsigned char)i, state, 1);
            if (topmost != 0)
                topmost->Release();
            DrainActionSessions();
        }
    }
    bMouseStateDirty = false;
}

// DISHONORED(port): 2013 0xa05330 (2012 0xa0e900). The whole body now, in retail's order: fold the
// entry into the per-mouse state, resolve the topmost entity under the pointer, remember it, broadcast
// the Mouse class's notifications, and let GFx_GenerateMouseButtonEvents (2013 0xa5ad10, 2012
// 0xa66a90 - the address the briefs carry) turn the change into rollOver / rollOut / press / release /
// dragOver / dragOut. Until the middle three existed a delivered click had no notion of what it was
// over, which is agentDG.md deviation 4 and agentDM.md hand-over 2.
void GFxMovieRoot::ProcessMouse(const GFxInputEventsQueue::QueueEntry& entry,
                               unsigned int* processedMice)
{
    const GFxInputEventsQueue::MouseEntry& m = entry.MouseData;
    // Retail's first line, `*a4 |= 1 << entry[16]`: this mouse has had its events generated from a
    // real queue entry this pass, so ProcessInput's per-frame arm must not generate them again.
    if (processedMice != 0)
        *processedMice |= 1u << m.MouseIndex;
    if (m.MouseIndex < MaxMice)
    {
        MouseX[m.MouseIndex] = m.x;
        MouseY[m.MouseIndex] = m.y;
        if (m.ChangedButtons == 0)
            MouseButtons[m.MouseIndex] |= m.Buttons;
        else
            MouseButtons[m.MouseIndex] &= ~m.Buttons;
    }
    GFxMouseState* state = GetMouseStateStruct(m.MouseIndex);
    GFxASCharacter* topmost = 0;
    if (GFxMouseTrace)
    {
        printf("DISHONORED(bringup): ProcessMouse idx %u at (%.1f,%.1f) px buttons %u changed %u "
               "wheel %d state %p level0 %p\n",
               m.MouseIndex, m.x * 0.05f, m.y * 0.05f, m.Buttons, m.ChangedButtons, m.ScrollDelta,
               (void*)state, (void*)GetLevel0());
    }
    if (state != 0)
    {
        state->UpdateState(entry);
        GPoint<float> pt(m.x, m.y);
        topmost = GetTopMostEntity(pt, m.MouseIndex, false, 0);
        if (topmost != 0)
            topmost->AddRef();
        state->SetTopmostEntity(topmost);
    }
    GASValue arg;
    arg.SetInt((int)m.MouseIndex);
    if (m.ScrollDelta != 0)
    {
        GASValue args[2];
        args[0].SetInt(m.ScrollDelta);
        args[1].SetInt((int)m.MouseIndex);
        GFxInputBroadcastMouse(&Env, "onMouseWheel", args, 2);
    }
    else if (m.Buttons != 0)
    {
        GFxInputBroadcastMouse(&Env, m.ChangedButtons != 0 ? "onMouseUp" : "onMouseDown", &arg, 1);
    }
    else
    {
        GFxInputBroadcastMouse(&Env, "onMouseMove", &arg, 1);
    }
    if (state != 0)
    {
        // Retail's last line. `buttonCount` is 1 unless the global context's extended-clip-event flag
        // is set, which this cook does not set (ProcessMouse's own
        // `*(pGC + 684) - 1 != 0 ? 1 : 16`).
        GFx_GenerateMouseButtonEvents((unsigned char)m.MouseIndex, state, 1);
    }
    if (topmost != 0)
        topmost->Release();
}

void GFxMovieRoot::NotifyMouseState(float x, float y, unsigned int buttons, unsigned int index)
{                                                                     // 2012 0xa04750
    if (index >= MaxMice)
        return;
    const GPoint<float> p = ViewportToTwips(x, y);
    const float px = p.x;
    const float py = p.y;
    if (px != MouseX[index] || py != MouseY[index])
        InputQueue.AddMouseMove(index, p);
    const unsigned int changed = MouseButtons[index] ^ buttons;
    if (changed != 0)
    {
        if ((changed & buttons) != 0)
            InputQueue.AddMouseButtonEvent(index, p, changed & buttons, 0);
        if ((changed & ~buttons) != 0)
            InputQueue.AddMouseButtonEvent(index, p, changed & ~buttons, 0x80);
    }
    MouseX[index] = px;
    MouseY[index] = py;
    MouseButtons[index] = buttons;
    bDirty = true;
}

void GFxMovieRoot::GetMouseState(unsigned int i, float* x, float* y, unsigned int* buttons)
{                                                                     // 2012 0x9cd8d0
    if (i >= MaxMice)
        i = 0;
    if (x) *x = MouseX[i];
    if (y) *y = MouseY[i];
    if (buttons) *buttons = MouseButtons[i];
}
