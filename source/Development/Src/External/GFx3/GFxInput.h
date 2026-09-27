// Scaleform GFx 3.3.89 - the input half of the player: the per-keyboard key state, the input event
// queue, the movie root's event entry points, and the AS2 classes an interface listens with
// (AsBroadcaster, Key, Mouse). Package DG.
//
// Why this and not the button model: agent DC's hand-over named GFxButtonCharacter (38 functions) and
// GFx_GenerateMouseButtonEvents as what stood between the keys and the menu. Measured, the retail
// main menu asset holds exactly ONE DefineButton2 tag in 477 tags, and its own strings carry
// `Key`, `addListener`, `getCode`, `isDown`, `onKeyDown` and `onKeyUp` - the menu is driven by the AS2
// Key broadcaster, not by button characters. That is what this unit ports (agentDG.md 2).
//
// DISHONORED(port): the retail chain, read out of the decompiles rather than assumed -
//   GFxMovieRoot::HandleEvent      2012 0xa042d0  queue the event, update the keyboard state, HE_Handled
//   GFxMovieRoot::ProcessInput     2012 0xa10d80  drained once per Advance, before the frame's tags
//   GFxMovieRoot::ProcessKeyboard  2012 0xa0cfa0  the level walk, then GFxKeyboardState::NotifyListeners
//   GFxKeyboardState::*            2012 0xa5ef30..0xa5f280
//   GFxInputEventsQueue::*         2012 0xa01db0..0xa02080
//   GASKeyCtorFunction::*          2012 0xa8a0e0..0xa8a9e0
//   GASAsBroadcaster::*            2012 0xa6eac0..0xa6f2e0
#ifndef INC_GFX3_GFXINPUT_H
#define INC_GFX3_GFXINPUT_H

#include "GFx3.h"
#include "GFxAS2.h"

#pragma pack(push, 4)

class GASStringContext;
class GASEnvironment;
class GASObject;
class GASObjectInterface;
class GASGlobalContext;
class GFxASCharacter;
class GFxMovieRoot;

// GFxKeyboardState. Retail keeps one per keyboard index (four of them, inline in GFxMovieRoot at
// +2500 with a 1660-byte stride, which is what every decompile's `1660 * index + 2500` is), and the
// bit array is 0xE5 keys wide - SetKeyDown's own bound (2012 0xa5f100).
class GFxKeyboardState
{
public:
    enum { KeyCount = 0xE5, KeymapBytes = 29, ToggledBytes = 3 };

    // 2012: the three virtuals GASKeyCtorFunction implements. Retail's IListener also carries the
    // GFxKeyboardState::IListener::~IListener slot, which is slot 0 here too.
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void OnKeyDown(GASStringContext* sc, int code, unsigned char ascii,
                               unsigned int wcharCode, unsigned char keyboardIndex) = 0;
        virtual void OnKeyUp(GASStringContext* sc, int code, unsigned char ascii,
                             unsigned int wcharCode, unsigned char keyboardIndex) = 0;
        virtual void Update(int code, unsigned char ascii, unsigned int wcharCode,
                            unsigned char keyboardIndex) = 0;
    };

    GFxKeyboardState();                                               // 2012 0xa5f280

    bool IsKeyDown(int code) const;                                   // 2012 0xa5f040
    bool IsKeyToggled(int code) const;                                // 2012 0xa5f080
    void SetKeyToggled(int code, bool toggled);                       // 2012 0xa5f0c0
    void SetKeyDown(int code, unsigned char ascii, GFxSpecialKeysState special);   // 2012 0xa5f100
    void SetKeyUp(int code, unsigned char ascii, GFxSpecialKeysState special);     // 2012 0xa5f150
    void ResetState();                                                // 2012 0xa5efc0
    void NotifyListeners(GASStringContext* sc, short code, unsigned char ascii,
                         unsigned int wcharCode, GFxEvent::EventType type) const;  // 2012 0xa5f1a0
    void UpdateListeners(short code, unsigned char ascii, unsigned int wcharCode); // 2012 0xa5f200
    void SetListener(IListener* l) { pListener = l; }                 // 2012 0xa5f230
    void SetKeyboardIndex(unsigned char i) { KeyboardIndex = i; }

    IListener*    pListener;
    unsigned char Keymap[KeymapBytes];
    unsigned char Toggled[ToggledBytes];
    unsigned char KeyboardIndex;
};

// GFxInputEventsQueue. Retail's QueueEntry is a tagged union of a key entry and a mouse entry, which
// is how ProcessInput tells the two apart (the decompile branches on the entry's first dword before
// it calls ProcessKeyboard or ProcessMouse). The queue itself is retail's ring; this is a flat
// growable array drained to empty in the same place, which has the same observable order.
class GFxInputEventsQueue
{
public:
    struct KeyEntry
    {
        unsigned int        WcharCode;
        short               Code;
        unsigned char       AsciiCode;
        unsigned char       KeyboardIndex;
        GFxSpecialKeysState SpecialKeysState;
        bool                bKeyDown;
    };
    struct MouseEntry
    {
        float        x;
        float        y;
        unsigned int Buttons;
        unsigned int ChangedButtons;
        int          ScrollDelta;
        unsigned int MouseIndex;
    };
    struct QueueEntry
    {
        enum Kind { Key = 0, Mouse = 1, Char = 2 };
        Kind       EntryKind;
        KeyEntry   KeyData;
        MouseEntry MouseData;
    };

    GFxInputEventsQueue();
    ~GFxInputEventsQueue();

    QueueEntry* AddEmptyQueueEntry();                                 // 2012 0xa01db0
    void AddMouseMove(unsigned int index, const GPoint<float>& p);    // 2012 0xa01e10
    void AddMouseButtonEvent(unsigned int index, const GPoint<float>& p, unsigned int buttons,
                             unsigned int changedMask);               // 2012 0xa01e50
    void AddMouseWheel(unsigned int index, const GPoint<float>& p, int delta);     // 2012 0xa01eb0
    void AddKeyDown(short code, unsigned char ascii, GFxSpecialKeysState special,
                    unsigned char keyboardIndex);                     // 2012 0xa02000
    void AddKeyUp(short code, unsigned char ascii, GFxSpecialKeysState special,
                  unsigned char keyboardIndex);                       // 2012 0xa02040
    void AddCharTyped(unsigned int wcharCode, unsigned char keyboardIndex);        // 2012 0xa02080

    const QueueEntry* GetEntry();                                     // 2012 0xa01f10
    bool IsEmpty() const { return Head >= Count; }
    void Clear() { Head = Count = 0; }

private:
    QueueEntry*  Entries;
    unsigned int Count;
    unsigned int Capacity;
    unsigned int Head;
};

// The input half of GFxMovieRoot lives in GFxInput.cpp rather than on the class, so that the one
// file carries the whole chain. These are what GFxPlayerRoot.cpp's members forward to.
void GFxInputInstall(GASGlobalContext* gc, GASObject* global);
void GFxInputShutdown(GASGlobalContext* gc);
// AsBroadcaster's listener walk, reachable so that the Key object and the Mouse object share it.
void GFxInputBroadcast(GASEnvironment* env, GASObjectInterface* self, const GASString& msg,
                       const GASValue* args, unsigned int nargs);

// The Key object's own state, reachable so that Key.isDown can read the movie's keyboard state and
// so that the harness can report what a key event did. One per global context.
struct GFxInputCensus
{
    unsigned int EventsHandled;
    unsigned int EventsNotHandled;
    unsigned int KeyDowns;
    unsigned int KeyUps;
    unsigned int CharsTyped;
    unsigned int MouseEvents;
    unsigned int KeyListenerCalls;
    unsigned int ListenersAdded;
    unsigned int ClipKeyHandlers;
};
GFxInputCensus& GFxInputGetCensus();
void GFxInputResetCensus();

#pragma pack(pop)
#endif // INC_GFX3_GFXINPUT_H
