// Scaleform GFx 3.3.89 - the player: the tag and character model, the display list, the sprite
// timeline, the movie data definition and the movie root. Package BC.
//
// The shape of this file is dictated by five decompiles, and the dispatch it describes is the reason
// a frame runs at all:
//   GFxSprite::AdvanceFrame        2012 0x9f93f0  increment, loop-check, EnterFrame, frame tags
//   GFxSprite::ExecuteFrameTags    2012 0x9f60a0  GetInitActionList then GetPlaylist then, per tag,
//                                                 ExecuteWithPriority(this, 4)
//   GFxSprite::ExecuteInitActionFrameTags 0x9f4640  once per frame, guarded by a per-frame flag
//   GFxSprite::CallFrameActions    2012 0x9f63b0  opens an action-queue session, runs the tags that
//                                                 answer IsActionTag(), then DoActionsForSession
//   GFxSprite::IncrementFrameAndCheckForLoop 0x9f4500  wraps to 0 and marks the display list
// GFxCharacterDef's virtual slots 10/11/12 are GetFrameCount / GetPlaylist / GetInitActionList:
// ExecuteFrameTags reads exactly those three through the def pointer at sprite+184.
//
// DISHONORED(port): see GFxAS2.h for why there are no offset assertions in this file.
#ifndef INC_GFX3_GFXPLAYER_H
#define INC_GFX3_GFXPLAYER_H

#include "GFxAS2Runtime.h"
#include "GFxInput.h"
#include "GFxGfxFile.h"

#pragma pack(push, 8)

class GFxCharacter;
class GFxASCharacter;
class GFxSprite;
class GFxMovieDataDef;
class GFxMovieDefImpl;
class GFxMovieRoot;
class GFxSpriteDef;
class GFxLoadProcess;
struct GFxTagInfo;
// DISHONORED(port): 2012 0xa5f9b0. The display half lives in GFxDisplay.{h,cpp}; everything here
// needs is the forward declaration and the census the walk fills.
class GFxDisplayContext;

// What one Display() pass submitted. Retail counts into GRenderer::Stats and GFxRenderStats; this is
// the per-movie half of the same thing and it is what the engine's census line reports.
struct GFxDisplayStats
{
    unsigned int Characters;      // display objects visited
    unsigned int Sprites;
    unsigned int Shapes;
    unsigned int TextFields;
    unsigned int Buttons;
    unsigned int Images;
    unsigned int TriListDraws;
    unsigned int Triangles;
    unsigned int GlyphDraws;
    unsigned int Glyphs;
    unsigned int Masks;
    unsigned int Invisible;
    unsigned int NoGeometry;
};

// One twip is 1/20 of a pixel; every coordinate in a SWF/GFX tag stream is in twips.
const float GFxTwipsToPixels = 0.05f;
const float GFxPixelsToTwips = 20.0f;

// ActionGetProperty / ActionSetProperty (0x22 / 0x23) address the display properties by index rather
// than by name. The order is the SWF one, which retail's property table follows: 0 _x, 1 _y,
// 2 _xscale, 3 _yscale, 4 _currentframe, 5 _totalframes, 6 _alpha, 7 _visible, 8 _width, 9 _height,
// 10 _rotation, 11 _target, 12 _framesloaded, 13 _name, 14 _droptarget, 15 _url, 16 _highquality,
// 17 _focusrect, 18 _soundbuftime, 19 _quality, 20 _xmouse, 21 _ymouse.
void GFxAS2GetDisplayProperty(GFxASCharacter* ch, int index, GASValue* out);
void GFxAS2SetDisplayProperty(GFxASCharacter* ch, int index, const GASValue& v, GASEnvironment* env);

// ---------------------------------------------------------------------------------------------
// GFxStream: the tag-stream reader. Retail's has 29 functions in GFxStream.obj; the ones the tag
// loaders call are OpenTag/CloseTag (2012 0xa1a710 / 0xa19de0), the fixed-width reads, the bit
// reads and ReadMatrix/ReadCxform/ReadString.
class GFxStream
{
public:
    GFxStream(const unsigned char* data, unsigned int size);

    unsigned int Tell() const { return Pos; }
    void         SetPosition(unsigned int p) { Pos = p; BitPos = 0; BitBuf = 0; }
    unsigned int GetSize() const { return Size; }
    bool         IsAtEnd() const { return Pos >= Size; }

    void          Align() { BitPos = 0; BitBuf = 0; }
    unsigned char ReadU8();
    signed char   ReadS8() { return (signed char)ReadU8(); }
    unsigned short ReadU16();
    short          ReadS16() { return (short)ReadU16(); }
    unsigned int  ReadU32();
    float         ReadFloat();
    double        ReadDouble();
    float         ReadFixed88();
    unsigned int  ReadUBits(unsigned int bits);
    int           ReadSBits(unsigned int bits);
    void          ReadString(char* out, unsigned int outSize);
    void          ReadStringWithLength(char* out, unsigned int outSize);
    void          ReadRect(int* outLRTB);
    void          ReadMatrix(GMatrix2D* out);
    void          ReadCxformRgb(GRenderer::Cxform* out);
    void          ReadCxformRgba(GRenderer::Cxform* out);
    void          ReadRgb(GColor* out);
    void          ReadRgba(GColor* out);
    void          Skip(unsigned int n) { Align(); Pos += n; }

    // OpenTag reads the (code << 6) | length word, and the long form when length == 0x3F.
    bool          OpenTag(unsigned int* outCode, unsigned int* outEndPos);
    void          CloseTag() { Align(); Pos = TagEnd; }
    unsigned int  GetTagEndPosition() const { return TagEnd; }

    const unsigned char* GetPtr(unsigned int at) const { return Data + at; }

private:
    const unsigned char* Data;
    unsigned int Size;
    unsigned int Pos;
    unsigned int TagEnd;
    unsigned char BitBuf;
    unsigned int  BitPos;
};

// ---------------------------------------------------------------------------------------------
// The execute tags. GASExecuteTag's slot 1 is Execute(GFxSprite*), slot 2
// ExecuteWithPriority(GFxSprite*, GFxActionPriority::Priority) (2012 0x9dee80, which forwards to
// Execute) and slot 4 IsActionTag() (2012 0xa89630, `return 0` in the base and 1 in GASDoAction).
enum GFxActionPriority
{
    GFxAP_Highest = 0, GFxAP_Init = 1, GFxAP_User = 2, GFxAP_Frame = 4, GFxAP_Lowest = 5
};

class GASExecuteTag
{
public:
    virtual ~GASExecuteTag() {}
    virtual void Execute(GFxSprite* sprite) = 0;
    virtual void ExecuteWithPriority(GFxSprite* sprite, GFxActionPriority prio)
                     { (void)prio; Execute(sprite); }
    virtual bool IsActionTag() const { return false; }
    virtual const char* GetTagName() const = 0;
};

// The unpacked PlaceObject2/3 record. The flag bits are read out of GFxPlaceObject2::UnpackBase
// (2012 0xa0bab0): bit 0 Move, 1 HasCharacter, 2 HasMatrix, 3 HasCxform, 4 HasRatio, 5 HasName,
// 6 HasClipDepth, 7 HasClipActions - and the 0x80 case is why that body starts the field walk at
// +5 instead of +1, because the event-handler block is restructured in front of the data.
class GFxCharPosInfo
{
public:
    enum Flags
    {
        Place_Move          = 0x01,
        Place_HasCharacter  = 0x02,
        Place_HasMatrix     = 0x04,
        Place_HasCxform     = 0x08,
        Place_HasRatio      = 0x10,
        Place_HasName       = 0x20,
        Place_HasClipDepth  = 0x40,
        Place_HasClipActions = 0x80
    };

    unsigned int      PlaceFlags;
    int               Depth;
    unsigned int      CharacterId;
    GMatrix2D         Matrix;
    GRenderer::Cxform ColorTransform;
    float             Ratio;
    int               ClipDepth;
    char              Name[128];
    unsigned int      BlendMode;

    GFxCharPosInfo();

    bool HasCharacter() const { return (PlaceFlags & Place_HasCharacter) != 0; }
    bool HasMatrix() const { return (PlaceFlags & Place_HasMatrix) != 0; }
    bool HasCxform() const { return (PlaceFlags & Place_HasCxform) != 0; }
    bool HasName() const { return (PlaceFlags & Place_HasName) != 0; }
    bool IsMove() const { return (PlaceFlags & Place_Move) != 0; }
};

// GFxPlaceObject2 / GFxPlaceObject3. Retail keeps the tag bytes packed and unpacks lazily
// (GFx_PlaceObject2Loader 0xa379b0 copies the raw bytes and only Unpack 0xa0d430 reads them); this
// unpacks at load, which is the same behaviour with a different memory profile.
class GFxPlaceObject2Tag : public GASExecuteTag
{
public:
    GFxCharPosInfo Pos;
    bool           bIsPlaceObject3;

    GFxPlaceObject2Tag() : bIsPlaceObject3(false) {}
    void Read(GFxStream* s, unsigned int tagCode);
    virtual void Execute(GFxSprite* sprite);                          // via 2012 0xa0d440
    virtual const char* GetTagName() const { return bIsPlaceObject3 ? "PlaceObject3" : "PlaceObject2"; }
};

class GFxRemoveObject2Tag : public GASExecuteTag
{
public:
    int          Depth;
    unsigned int CharacterId;
    bool         bHasCharacterId;

    GFxRemoveObject2Tag() : Depth(0), CharacterId(0), bHasCharacterId(false) {}
    void Read(GFxStream* s, unsigned int tagCode);
    virtual void Execute(GFxSprite* sprite);                          // 2012 0xa01d90
    virtual const char* GetTagName() const { return "RemoveObject2"; }
};

class GFxSetBackgroundColorTag : public GASExecuteTag
{
public:
    GColor Color;
    void Read(GFxStream* s);                                          // via 2012 0xa36cd0
    virtual void Execute(GFxSprite* sprite);
    virtual const char* GetTagName() const { return "SetBackgroundColor"; }
};

// GASDoAction / GASDoInitAction. IsActionTag() is true only for the first, which is what
// GFxSprite::CallFrameActions (0x9f63b0) filters on, and Execute pushes the buffer onto the movie
// root's action queue rather than running it inline - that queue is the AS2 execution order.
class GASDoActionTag : public GASExecuteTag
{
public:
    GASActionBuffer Buffer;

    void Read(GFxStream* s, unsigned int endPos);                     // 2012 0x9e18d0
    virtual void Execute(GFxSprite* sprite);                          // 2012 0x9e4240
    virtual void ExecuteWithPriority(GFxSprite* sprite, GFxActionPriority prio); // 0x9e42b0
    virtual bool IsActionTag() const { return true; }
    virtual const char* GetTagName() const { return "DoAction"; }
};

class GASDoInitActionTag : public GASExecuteTag
{
public:
    GASActionBuffer Buffer;
    unsigned int    SpriteId;

    GASDoInitActionTag() : SpriteId(0) {}
    void Read(GFxStream* s, unsigned int endPos);                     // via 2012 0x9e4420
    virtual void Execute(GFxSprite* sprite);                          // 2012 0x9e4330
    virtual const char* GetTagName() const { return "DoInitAction"; }
};

// ---------------------------------------------------------------------------------------------
class GFxTagList
{
public:
    GFxTagList() : Tags(0), Size(0), Capacity(0) {}
    ~GFxTagList();

    void Add(GASExecuteTag* tag);
    unsigned int GetSize() const { return Size; }
    GASExecuteTag* operator[](unsigned int i) const { return Tags[i]; }
    void ReleaseOwnership() { Tags = 0; Size = 0; Capacity = 0; }
    void TakeFrom(GFxTagList& other)
    {
        Tags = other.Tags; Size = other.Size; Capacity = other.Capacity;
        other.ReleaseOwnership();
    }

private:
    GASExecuteTag** Tags;
    unsigned int    Size;
    unsigned int    Capacity;
};

// GFxTimelineDef::Frame is a tag list; a timeline is a vector of them plus the init-action lists
// and the frame labels. GFxMovieDataDef and GFxSpriteDef are both timelines, which is why
// GFxSprite can drive either through the same three virtual slots.
class GFxTimelineDef
{
public:
    GFxTimelineDef();
    virtual ~GFxTimelineDef();

    virtual unsigned int GetFrameCount() const { return FrameCount; }             // slot 10
    virtual const GFxTagList* GetPlaylist(unsigned int frame) const;              // slot 11
    virtual const GFxTagList* GetInitActionList(unsigned int frame) const;        // slot 12
    virtual bool GetLabeledFrame(const char* label, unsigned int* outFrame) const;// 2012 0x9fe2d0

    void BeginFrames(unsigned int count);
    void AddTagToFrame(unsigned int frame, GASExecuteTag* tag);
    void AddInitActionToFrame(unsigned int frame, GASExecuteTag* tag);
    void AddFrameLabel(const char* name, unsigned int frame);                     // 2012 0xa003f0
    void CommitFrameCount(unsigned int count) { FrameCount = count; }
    void GrowFrames(unsigned int need);

protected:
    struct FrameLabel { char Name[96]; unsigned int Frame; };

    GFxTagList*  Frames;
    GFxTagList*  InitActions;
    unsigned int FrameCapacity;
    unsigned int FrameCount;
    FrameLabel*  Labels;
    unsigned int LabelCount;
    unsigned int LabelCapacity;
};

// A character definition: the immutable, shared description an instance is created from.
// GetResourceTypeCode returns GFxResource::RT_SpriteDef and friends, which is the one virtual of
// GFxResource this hierarchy overrides for real.
class GFxCharacterDef : public GFxResource
{
public:
    GFxCharacterDef() { Id.Id = GFxResourceId::InvalidId; }
    virtual ~GFxCharacterDef() {}

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl) = 0;
    virtual const char* GetDefTypeName() const = 0;

    // DISHONORED(port): 2012 0xa3d7d0 - the definition-side draw. A definition with no geometry (a
    // placeholder, a font, a morph shape) keeps the base body, which is what retail's base does too.
    virtual void Display(GFxDisplayContext& ctx, GFxCharacter* ch);

    GFxResourceId Id;
};

// Every non-sprite definition in the cook - DefineShape/2/3/4, DefineEditText, DefineText,
// DefineButton2, DefineMorphShape - is recorded as one of these. It is a real character with a real
// depth and a real transform that simply has no geometry attached yet, which is exactly what is
// needed to get a frame to run and to report what a frame created: the shape tessellator and the
// glyph rasteriser are the next wave's (agentBB.md 3.4 - the fonts are outlines, not textures).
class GFxPlaceholderDef : public GFxCharacterDef
{
public:
    GFxPlaceholderDef(unsigned int tagCode, unsigned int bodyBytes)
        : TagCode(tagCode), BodyBytes(bodyBytes) {}

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const;
    virtual const char* GetDefTypeName() const;

    unsigned int TagCode;
    unsigned int BodyBytes;
};

// DefineSprite (tag 39): a nested timeline. GFxSpriteDef::Read (2012 0x9fa170) reads the frame
// count then recurses into the same tag loop the root uses.
class GFxSpriteDef : public GFxCharacterDef, public GFxTimelineDef
{
public:
    GFxSpriteDef(GFxMovieDataDef* movie) : pMovieDef(movie), bHasScale9Grid(false) {}

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);  // 2012 0xa00390
    virtual unsigned int GetResourceTypeCode() const;
    virtual const char* GetDefTypeName() const { return "SpriteDef"; }

    // GFx_Scale9GridLoader (2012 0xa36680) applies the grid to a sprite or a button definition;
    // GFxButtonCharacterDef::SetScale9Grid is its sibling at 0xa35150.
    void SetScale9Grid(const GRect<float>& r) { Scale9Grid = r; bHasScale9Grid = true; }

    GFxMovieDataDef* pMovieDef;
    GRect<float>     Scale9Grid;
    bool             bHasScale9Grid;
};

// ---------------------------------------------------------------------------------------------
// GFxCharacterHandle: a weak name-plus-pointer handle to a character. GASValue::SetAsCharacter
// (2012 0x9cb7e0) never stores a GFxASCharacter* - it stores the handle, refcounted - and
// GFxValue::ObjectInterface::GetMember (0x9b0450) resolves it again on every access through
// GFxCharacterHandle::ResolveCharacter. That indirection is what makes a GFxValue referring to a
// removed movie clip degrade to undefined instead of dangling, so it is ported rather than shortcut.
class GFxCharacterHandle
{
public:
    GFxCharacterHandle(const GASString& name, GFxASCharacter* parent, GFxASCharacter* ch);

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount <= 0) delete this; }

    GFxASCharacter* ResolveCharacter(GFxMovieRoot* root) const { (void)root; return pCharacter; }
    void ChangeCharacter(GFxASCharacter* ch) { pCharacter = ch; }
    const GASString& GetName() const { return Name; }

    int             RefCount;
    GASString       Name;
    GFxASCharacter* pParent;
    GFxASCharacter* pCharacter;
};

// GFxCharacter: the display-list citizen. Matrix, colour transform, depth, ratio, parent, def.
class GFxCharacter
{
public:
    GFxCharacter(GFxASCharacter* parent, GFxResourceId id);
    virtual ~GFxCharacter();

    void AddRef() { ++RefCount; }
    void Release() { if (--RefCount <= 0) delete this; }

    virtual GFxCharacterDef*  GetCharacterDef() const { return 0; }
    virtual GFxASCharacter*   ToASCharacterDef() { return 0; }
    virtual bool              IsASCharacter() const { return false; }
    virtual const char*       GetCharacterTypeName() const = 0;
    /** the character's bounds in twips, through `m`. A definition with no geometry answers an empty
        rectangle; a sprite unions its display list. 2012 0x9d2ab0 / GFxSprite's own override. */
    virtual GRect<float>      GetBoundsTwips(const GMatrix2D& m) const;
    virtual void              OnEventLoad() {}
    virtual void              OnEventUnload() {}
    virtual void              AdvanceFrame(bool bAdvance, float framePos)
                                  { (void)bAdvance; (void)framePos; }
    // DISHONORED(port): 2012 0x9cfdd0 / 0x9faeb0 / 0xa2ed90. The base draws nothing, which is what a
    // character with no definition is. Bodies in GFxDisplay.cpp.
    virtual void              Display(GFxDisplayContext& ctx);

    const GMatrix2D& GetMatrix() const { return Matrix; }
    void SetMatrix(const GMatrix2D& m) { Matrix = m; }
    const GRenderer::Cxform& GetCxform() const { return ColorTransform; }
    void SetCxform(const GRenderer::Cxform& c) { ColorTransform = c; }
    int  GetDepth() const { return Depth; }
    void SetDepth(int d) { Depth = d; }
    float GetRatio() const { return Ratio; }
    void SetRatio(float r) { Ratio = r; }
    int  GetClipDepth() const { return ClipDepth; }
    void SetClipDepth(int d) { ClipDepth = d; }
    GFxResourceId GetId() const { return Id; }
    GFxASCharacter* GetParent() const { return pParent; }
    bool GetVisible() const { return bVisible; }
    void SetVisible(bool v) { bVisible = v; }

    int               RefCount;
    GFxResourceId     Id;
    GFxASCharacter*   pParent;
    GMatrix2D         Matrix;
    GRenderer::Cxform ColorTransform;
    int               Depth;
    int               ClipDepth;
    float             Ratio;
    bool              bVisible;
};

// GFxDisplayList. Depth-sorted, and the four mutators are the four retail functions: AddDisplayObject
// (2012 0x9d60f0), MoveDisplayObject (0x9d6250), ReplaceDisplayObject (0x9d6350) and
// RemoveDisplayObject (0x9d5fd0), plus the mark-and-sweep pair MarkAllEntriesForRemoval (0x9d5670)
// and UnloadMarkedObjects (0x9d60a0) that IncrementFrameAndCheckForLoop uses to rebuild a looping
// timeline's list.
class GFxDisplayList
{
public:
    struct DisplayEntry
    {
        GFxCharacter* pChar;
        bool          bMarkedForRemove;
    };

    GFxDisplayList();
    ~GFxDisplayList();

    void AddDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch);
    void MoveDisplayObject(const GFxCharPosInfo& pos);
    void ReplaceDisplayObject(const GFxCharPosInfo& pos, GFxCharacter* ch);
    void RemoveDisplayObject(int depth, GFxResourceId id);
    void MarkAllEntriesForRemoval(unsigned int fromIndex);
    void UnloadMarkedObjects();
    void UnloadAll();
    void Clear();

    GFxCharacter* GetCharacterAtDepth(int depth, bool* outMarked) const;   // 2012 0x9d5a80
    GFxCharacter* GetCharacterByName(GASStringContext* sc, const GASString& name) const; // 0x9d5450
    int           GetLargestDepthInUse() const;                           // 2012 0x9d5430
    int           FindDisplayIndex(int depth) const;                      // 2012 0x9d5a40

    unsigned int  GetCount() const { return Size; }
    GFxCharacter* GetAt(unsigned int i) const { return Entries[i].pChar; }

    // DISHONORED(port): 2012 0x9d56c0. Depth-ascending, with an entry whose ClipDepth is non-zero
    // acting as the stencil mask for every entry up to that depth. Body in GFxDisplay.cpp.
    void Display(GFxDisplayContext& ctx);

private:
    DisplayEntry* Entries;
    unsigned int  Size;
    unsigned int  Capacity;

    void InsertAt(unsigned int index, GFxCharacter* ch);
    void RemoveAt(unsigned int index);
};

// GFxASCharacter: a character that ActionScript can see. Retail derives it from both GFxCharacter
// and GASObjectInterface, with the interface subobject at +120 - that is the `this - 120` in
// GASObjectInterface::ToASCharacter (2012 0x9da8d0) and the `+ 120` in every GFxValue path.
class GFxASCharacter : public GFxCharacter, public GASObjectInterface
{
public:
    GFxASCharacter(GFxASCharacter* parent, GFxResourceId id, GFxMovieRoot* root);
    virtual ~GFxASCharacter();

    virtual bool IsASCharacter() const { return true; }
    virtual GFxASCharacter* ToASCharacterDef() { return this; }
    virtual GFxASCharacter* ToASCharacter() { return this; }
    virtual GASObjectType GetObjectType() const { return Object_EditText; }

    // The GASObjectInterface contract, implemented against the member hash plus the display-object
    // properties (_x, _y, _alpha, _visible, _name, _target, _currentframe, _totalframes, _parent,
    // _root, _global). GetStandardMember / SetStandardMember are the property half.
    virtual bool SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                           const GASPropFlags& flags);
    virtual bool GetMember(GASEnvironment* env, const GASString& name, GASValue* val);
    virtual bool FindMember(GASStringContext* sc, const GASString& name, GASMember* member);
    virtual bool DeleteMember(GASStringContext* sc, const GASString& name);
    virtual bool SetMemberFlags(GASStringContext* sc, const GASString& name, unsigned char flags);
    virtual void VisitMembers(GASStringContext* sc, MemberVisitor* visitor, unsigned int flags,
                              const GASObjectInterface* instance) const;
    virtual bool HasMember(GASStringContext* sc, const GASString& name, bool inherited);
    virtual bool SetMemberRaw(GASStringContext* sc, const GASString& name, const GASValue& val,
                              const GASPropFlags& flags);
    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val);
    virtual void Set__proto__(GASStringContext* sc, GASObject* proto);
    virtual bool InstanceOf(GASEnvironment* env, const GASObject* proto, bool inherited) const;

    virtual bool GetStandardMember(GASBuiltinString which, GASValue* out) const;
    virtual bool SetStandardMember(GASBuiltinString which, const GASValue& v);

    // The AS-visible name, the handle GASValue stores, and the script object that carries whatever
    // the content assigned onto this clip.
    GFxCharacterHandle* CreateCharacterHandle();                      // 2012 0x9d1a40
    GFxCharacterHandle* GetCharacterHandle() const { return pHandle; }
    const GASString&    GetName() const { return Name; }
    void                SetName(const GASString& n);
    GASString           GetTargetPath(GASStringContext* sc) const;
    GFxMovieRoot*       GetMovieRoot() const { return pMovieRoot; }

    void ExecuteEvent(GASBuiltinString eventName);                    // via 2012 0x9d22e0

    GFxMovieRoot*       pMovieRoot;
    GASString           Name;
    GFxCharacterHandle* pHandle;
    GASObject*          pASObject;      // the member store; created lazily
    GASObject*          pProto;

protected:
    GASObject* EnsureASObject();
};

// A non-sprite character instance: the placeholder an unported definition produces.
class GFxGenericCharacter : public GFxASCharacter
{
public:
    GFxGenericCharacter(GFxCharacterDef* def, GFxASCharacter* parent, GFxResourceId id,
                        GFxMovieRoot* root)
        : GFxASCharacter(parent, id, root), pDef(def) {}

    virtual GFxCharacterDef* GetCharacterDef() const { return pDef; }
    virtual const char* GetCharacterTypeName() const;
    virtual GASObjectType GetObjectType() const;
    virtual void Display(GFxDisplayContext& ctx);                     // 2012 0x9cfdd0

    GFxCharacterDef* pDef;
};

// GFxSprite: a movie clip. The timeline state is the four members every decompile touches -
// CurrentFrame (sprite+216 in retail), the play state, the per-frame init-action flags and the
// display list (sprite+196).
class GFxSprite : public GFxASCharacter
{
public:
    GFxSprite(GFxTimelineDef* def, GFxCharacterDef* charDef, GFxMovieDefImpl* defImpl,
              GFxASCharacter* parent, GFxResourceId id, GFxMovieRoot* root);
    virtual ~GFxSprite();

    virtual GFxCharacterDef* GetCharacterDef() const { return pCharDef; }
    virtual GRect<float>     GetBoundsTwips(const GMatrix2D& m) const;
    virtual const char* GetCharacterTypeName() const { return "Sprite"; }
    virtual GASObjectType GetObjectType() const { return Object_Sprite; }
    virtual GFxSprite* ToSprite() { return this; }

    virtual void AdvanceFrame(bool bAdvance, float framePos);          // 2012 0x9f93f0
    virtual void Display(GFxDisplayContext& ctx);                      // 2012 0x9faeb0
    void IncrementFrameAndCheckForLoop();                             // 2012 0x9f4500
    void ExecuteFrameTags(unsigned int frame, bool bWithActions = true); // 2012 0x9f60a0
    void ExecuteInitActionFrameTags(unsigned int frame);               // 2012 0x9f4640
    void ExecuteFrame0Events();                                        // 2012 0x9f8ac0
    void CallFrameActions(unsigned int frame);                         // 2012 0x9f63b0
    void GotoFrame(unsigned int frame);                                // 2012 0xa012a0
    bool GotoLabeledFrame(const char* label, int offset);              // 2012 0x9f6130
    void ExecuteBuffer(GASActionBuffer* buffer);                       // 2012 0x9f39f0

    // The four display-list mutators a PlaceObject/RemoveObject tag lands in.
    GFxCharacter* AddDisplayObject(const GFxCharPosInfo& pos);         // 2012 0x9fee10
    // Object.registerClass's half of instantiation: the clip is constructed as the class registered
    // for its library symbol, which is what gives a Dishonored screen its Open / Close / SetMenu.
    static void BindRegisteredClass(GFxSprite* child, const GASString& symbol);
    /** narrates every class binding: which symbol, which class, and whether the class's prototype
        chain still reaches MovieClip.prototype (if it does not, the clip loses attachMovie and its
        kin, which is a real content-versus-runtime disagreement rather than a missing method). */
    static bool bTraceClassBinding;
    /** off only for a comparison run; retail always binds (agentDG.md 4) */
    static bool bBindRegisteredClasses;
    void          MoveDisplayObject(const GFxCharPosInfo& pos);        // 2012 0x9f48a0
    void          ReplaceDisplayObject(const GFxCharPosInfo& pos);     // 2012 0x9f6300
    void          RemoveDisplayObject(int depth, GFxResourceId id);    // 2012 0x9f4970

    GFxSprite* CreateEmptyMovieClip(const GASString& name, int depth);
    GFxSprite* AttachMovie(const GASString& symbolName, const GASString& instanceName, int depth);

    unsigned int GetCurrentFrame() const { return CurrentFrame; }
    unsigned int GetFrameCount() const { return pTimelineDef ? pTimelineDef->GetFrameCount() : 1; }
    bool         GetPlaying() const { return bPlaying; }
    void         SetPlaying(bool p) { bPlaying = p; }
    bool         HasLooped() const { return bHasLooped; }

    GFxDisplayList&  GetDisplayList() { return DisplayList; }
    GFxMovieDefImpl* GetDefImpl() const { return pDefImpl; }

    virtual bool GetStandardMember(GASBuiltinString which, GASValue* out) const;
    virtual bool SetStandardMember(GASBuiltinString which, const GASValue& v);
    virtual bool GetMemberRaw(GASStringContext* sc, const GASString& name, GASValue* val);

    // GetOwnDataDef is the dictionary a PlaceObject inside this sprite's own timeline resolves
    // against, and it is NOT the root movie's. An imported symbol is a sprite whose timeline was
    // authored in another file, so its child ids index that file's dictionary; retail keeps the
    // distinction in GFxSpriteDef::pMovieDef and binds an import as a resource handle into the
    // exporting movie's library, which is the same thing. Without it every child of an imported
    // clip resolves to nothing, which is exactly what binding the imports first exposed.
    GFxMovieDataDef* GetOwnDataDef() const;

    GFxTimelineDef*  pTimelineDef;
    GFxCharacterDef* pCharDef;
    GFxMovieDefImpl* pDefImpl;

private:
    GFxDisplayList DisplayList;
    unsigned int   CurrentFrame;
    unsigned char* InitActionsExecuted;
    unsigned int   InitActionsSize;
    bool           bPlaying;
    bool           bHasLooped;
    bool           bFrame0Executed;
};

// ---------------------------------------------------------------------------------------------
// GFxMovieDataDef: one parsed GFX payload. It is both the root timeline and the character
// dictionary, which is why retail's GFxMovieDataDef is a GFxTimelineDef too.
class GFxMovieDataDef : public GFxCharacterDef, public GFxTimelineDef
{
public:
    GFxMovieDataDef();
    virtual ~GFxMovieDataDef();

    // The tag walk. One pass, the loaders of resources/docs/symbols the retail table registers
    // (GFxLoaderImpl::GetTagLoader 2012 0x9badf0 and the 43 GFx_*Loader free functions), and the
    // tags with no loader are counted and skipped by length exactly as GFxStream::CloseTag does.
    bool Read(const unsigned char* data, unsigned int size);

    virtual GFxCharacter* CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                  GFxMovieDefImpl* defImpl);
    virtual unsigned int GetResourceTypeCode() const;
    virtual const char* GetDefTypeName() const { return "MovieDataDef" ; }

    void             AddCharacter(unsigned int id, GFxCharacterDef* def);
    GFxCharacterDef* GetCharacterDefById(unsigned int id) const;
    void             AddExport(const char* name, unsigned int id);
    GFxCharacterDef* GetExportedCharacter(const char* name) const;
    int              GetExportedId(const char* name) const;
    const char*      GetExportedName(unsigned int id) const;

    // The loaders call back into these three: GFx_SpriteLoader recurses into the nested timeline,
    // GFx_ImportLoader records an unresolved symbol, and the two action loaders count bytes.
    void ReadSpriteTags(GFxStream* s, GFxSpriteDef* sprite, unsigned int endPos);
    void AddImport(const char* url, const char* symbol, unsigned int id);
    void NoteActionBytes(unsigned int bytes, bool bInit);
    void SetSWFFlags(unsigned int flags) { SWFFlags = flags; }
    unsigned int GetSWFFlags() const { return SWFFlags; }

    // Import binding. GFx_ImportLoader (2012 0xa385f0) only records the URL and the symbol names and
    // leaves a handle in the dictionary; the resolution happens later, in GFxMovieBindProcess, which
    // clones the load states for the imported file (GFxLoadStates::CloneForImport 0xa240b0), opens it
    // through the state bag's file opener (GFxLoadStates::OpenFile 0xa22520) and replaces each
    // handle with the exported definition of that movie. BindImports is that step; the resolver is
    // the seam the file opener sits behind, which is FGFxFileOpener in the engine and a path map in
    // the harness.
    class ImportResolver
    {
    public:
        virtual ~ImportResolver() {}
        virtual GFxMovieDataDef* ResolveImportMovie(const char* url) = 0;
    };
    unsigned int BindImports(ImportResolver* resolver);

    struct ImportEntry
    {
        char         Url[192];
        char         Symbol[160];
        unsigned int Id;
        bool         bBound;
    };
    unsigned int       GetImportCount() const { return ImportSize; }
    const ImportEntry& GetImport(unsigned int i) const { return Imports[i]; }
    unsigned int       GetDictSize() const { return DictSize; }
    GFxCharacterDef*   GetDictDef(unsigned int i) const { return Dict[i].pDef; }

    const GFxGfxFileInfo& GetFileInfo() const { return FileInfo; }

    // The url this payload was opened from. Every relative path inside it - an import, an external
    // image - resolves against it, which is what GFxLoadStates carries in retail.
    void        SetSourceUrl(const char* url);
    const char* GetSourceUrl() const { return SourceUrl; }

    unsigned int GetVersion() const { return Version; }
    float        GetFrameRate() const { return FrameRate; }
    float        GetWidth() const { return WidthPixels; }
    float        GetHeight() const { return HeightPixels; }
    GRect<float> GetFrameRect() const;

    // Counters the harness prints; every one of them is a real measurement of the parse.
    struct ReadStats
    {
        unsigned int Tags;
        unsigned int TagsHandled;
        unsigned int TagsSkipped;
        unsigned int Characters;
        unsigned int Sprites;
        unsigned int Placeholders;
        unsigned int Exports;
        unsigned int Imports;
        unsigned int ImportsBound;
        unsigned int Shapes;
        unsigned int MorphShapes;
        unsigned int EditTexts;
        unsigned int StaticTexts;
        unsigned int Buttons;
        unsigned int Fonts;
        unsigned int Images;
        unsigned int Unhandled;
        unsigned int DoActions;
        unsigned int DoInitActions;
        unsigned int ActionBytes;
        unsigned int SkippedCodes[96];
        unsigned int SkippedCounts[96];
        unsigned int SkippedCodeCount;
    };
    const ReadStats& GetReadStats() const { return Stats; }

private:
    struct DictEntry { unsigned int Id; GFxCharacterDef* pDef; };
    struct ExportEntry { char Name[160]; unsigned int Id; };

    bool ReadTags(GFxStream* s, GFxTimelineDef* timeline, unsigned int endPos, unsigned int depth);
    void NoteSkipped(unsigned int code);
    void NoteDefined(GFxCharacterDef* def);

    DictEntry*   Dict;
    unsigned int DictSize;
    unsigned int DictCapacity;
    ExportEntry* Exports;
    unsigned int ExportSize;
    unsigned int ExportCapacity;
    ImportEntry* Imports;
    unsigned int ImportSize;
    unsigned int ImportCapacity;
    unsigned int SWFFlags;

    unsigned int Version;
    float        FrameRate;
    float        WidthPixels;
    float        HeightPixels;
    int          FrameRectTwips[4];

    GFxGfxFileInfo FileInfo;
    ReadStats      Stats;
    char           SourceUrl[256];
};

// GFxMovieDefImpl: the bound movie definition, i.e. the GFxMovieDef the engine holds. Its job in
// this wave is to answer the 24 GFxMovieDef virtuals from the data def and to create the root
// GFxMovieRoot (2012 0xa1ba00 / 0xa1b960).
class GFxMovieDefImpl : public GFxMovieDef
{
public:
    GFxMovieDefImpl(GFxMovieDataDef* dataDef);
    virtual ~GFxMovieDefImpl();

    virtual unsigned int GetVersion() const;
    virtual unsigned int GetLoadingFrame() const;
    virtual float GetWidth() const;
    virtual float GetHeight() const;
    virtual unsigned int GetFrameCount() const;
    virtual float GetFrameRate() const;
    virtual GRect<float> GetFrameRect() const;
    virtual unsigned int GetSWFFlags() const;
    virtual const char* GetFileURL() const;
    virtual void WaitForLoadFinish(bool a0) const { (void)a0; }
    virtual void WaitForFrame(unsigned int a0) const { (void)a0; }
    virtual unsigned int GetFileAttributesW() const { return 0; }
    virtual unsigned int GetMetadata(char* a0, unsigned int a1) const
                                  { (void)a0; (void)a1; return 0; }
    virtual GMemoryHeap* GetLoadDataHeap() const { return 0; }
    virtual GMemoryHeap* GetBindDataHeap() const { return 0; }
    virtual GMemoryHeap* GetImageHeap() const { return 0; }
    virtual GFxResource* GetMovieDataResource() const;
    virtual const GFxExporterInfo* GetExporterInfo() const { return 0; }
    virtual GFxMovieDef::MemoryContext* CreateMemoryContext(const char* a0,
                                                            const GFxMovieDef::MemoryParams& a1,
                                                            bool a2)
                                  { (void)a0; (void)a1; (void)a2; return 0; }
    virtual GFxMovieView* CreateInstance(const GFxMovieDef::MemoryParams& a0, bool a1);
    virtual GFxMovieView* CreateInstance(GFxMovieDef::MemoryContext* a0, bool a1);
    virtual void VisitImportedMovies(GFxMovieDef::ImportVisitor* a0) { (void)a0; }
    virtual void VisitResources(GFxMovieDef::ResourceVisitor* a0, unsigned int a1)
                                  { (void)a0; (void)a1; }
    virtual GFxResource* GetResource(const char* a0) const;
    virtual unsigned int GetResourceTypeCode() const;

    // Our own real state bag, so the base-class bringup bodies of GFx3RuntimeStubs.cpp are not what
    // answers a GetStateAddRef on a movie definition.
    virtual GFxStateBag* GetStateBagImpl() const { return (GFxStateBag*)this; }
    virtual void SetState(GFxState::StateType t, GFxState* s);
    virtual GFxState* GetStateAddRef(GFxState::StateType t) const;
    virtual void GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                 unsigned int count) const;

    GFxMovieDataDef* GetDataDef() const { return pDataDef; }

    // DISHONORED(port): the state-bag chain. Retail's definition falls through to the GFxLoadStates
    // clone it was bound with, whose parent is the loader (GFxLoadStates::CloneForImport 0xa240b0), so
    // a movie instance reaches the render config, the log and the image loader the engine set on the
    // loader. Without this link GetStateAddRef answers null and Display has no renderer.
    void SetStateBagParent(GFxStateBag* parent) { pParentBag = parent; }
    // The url the loader opened, which GetFileURL must answer with because it is what every relative
    // path in the movie - an import, an external image - is resolved against.
    void SetFileURL(const char* url);
    // A definition shared out of the loader's cache must not be deleted with the def impl.
    void SetOwnsDataDef(bool b) { bOwnsDataDef = b; }

private:
    enum { MaxStates = 40 };
    GFxMovieDataDef* pDataDef;
    GFxState*        States[MaxStates];
    GFxStateBag*     pParentBag;
    char             FileUrl[256];
    bool             bOwnsDataDef;
};

// GFxMovieRoot: the GFxMovieView the engine talks to, and the owner of everything AS2. Retail's is
// 161 functions in GFxPlayerImpl.obj. Of the 73 GFxMovieView slots this wave implements the ones a
// frame needs for real and answers the rest from state; every body says which.
class GFxMovieRoot : public GFxMovieView
{
public:
    GFxMovieRoot(GFxMovieDefImpl* defImpl);
    virtual ~GFxMovieRoot();

    // --- GFxMovie (24) ---
    virtual GFxMovieDef* GetMovieDef() const;
    virtual unsigned int GetCurrentFrame() const;                     // 2012 0xa016f0
    virtual bool HasLooped() const;
    virtual void GotoFrame(unsigned int frame);                       // 2012 0xa01760
    virtual bool GotoLabeledFrame(const char* label, int offset);      // 2012 0xa04ad0
    virtual void SetPlayState(GFxMovie::PlayState state);
    virtual GFxMovie::PlayState GetPlayState() const;
    virtual void SetVisible(bool v);
    virtual bool GetVisible() const;
    virtual bool IsAvailable(const char* path) const;
    virtual void CreateString(GFxValue* v, const char* s);            // 2012 0x9af4e0
    virtual void CreateStringW(GFxValue* v, const wchar_t* s);         // 2012 0x9b0010
    virtual void CreateObject(GFxValue* v, const char* className, const GFxValue* args,
                              unsigned int nargs);                    // 2012 0x9b00a0
    virtual void CreateArray(GFxValue* v);                            // 2012 0x9af550
    virtual void CreateFunction(GFxValue* v, GFxFunctionHandler* h, void* userData); // 0x9af5c0
    virtual bool SetVariable(const char* path, const GFxValue& v, GFxMovie::SetVarType type);
    virtual bool GetVariable(GFxValue* v, const char* path) const;
    virtual bool SetVariableArray(GFxMovie::SetArrayType t, const char* path, unsigned int index,
                                  const void* data, unsigned int count, GFxMovie::SetVarType vt);
    virtual bool SetVariableArraySize(const char* path, unsigned int count,
                                      GFxMovie::SetVarType vt);
    virtual unsigned int GetVariableArraySize(const char* path);
    virtual bool GetVariableArray(GFxMovie::SetArrayType t, const char* path, unsigned int index,
                                  void* data, unsigned int count);
    virtual bool Invoke(const char* path, GFxValue* result, const GFxValue* args,
                        unsigned int nargs);
    virtual bool Invoke(const char* path, GFxValue* result, const char* argFmt, ...);
    virtual bool InvokeArgs(const char* path, GFxValue* result, const char* argFmt, char* args);

    // --- GFxMovieView (45) ---
    virtual void SetViewport(const GViewport& vp);
    virtual void GetViewport(GViewport* vp) const;
    virtual void SetViewScaleMode(GFxMovieView::ScaleModeType m);
    virtual GFxMovieView::ScaleModeType GetViewScaleMode() const;
    virtual void SetViewAlignment(GFxMovieView::AlignType a);
    virtual GFxMovieView::AlignType GetViewAlignment() const;
    virtual GRect<float> GetVisibleFrameRect() const;                 // 2012 0xa102a0
    virtual void SetPerspective3D(const GMatrix3D& m);
    virtual void SetView3D(const GMatrix3D& m);
    virtual GRect<float> GetSafeRect() const;
    virtual void SetSafeRect(const GRect<float>& r);
    virtual void Restart();
    virtual float Advance(float deltaT, unsigned int frameCatchUp);   // 2012 0xa11830
    virtual void Display();                                           // 2012 0xa07aa0
    virtual void DisplayPrePass();                                    // 2012 0xa08090
    virtual void SetPause(bool p);
    virtual bool IsPaused() const;
    virtual void SetBackgroundColor(const GColor c);
    virtual void SetBackgroundAlpha(float a);
    virtual float GetBackgroundAlpha() const;
    virtual unsigned int HandleEvent(const GFxEvent& e);
    virtual void GetMouseState(unsigned int i, float* x, float* y, unsigned int* buttons);
    virtual void NotifyMouseState(float x, float y, unsigned int buttons, unsigned int index);
    virtual bool HitTest(float x, float y, GFxMovieView::HitTestType t, unsigned int ctrlIdx);
    virtual bool HitTest3D(GPoint3<float>* p, float x, float y, unsigned int ctrlIdx);
    virtual void SetExternalInterfaceRetVal(const GFxValue& v);
    virtual void* GetUserData() const;
    virtual void SetUserData(void* d);
    virtual bool AttachDisplayCallback(const char* path, void* cb, void* userData);
    virtual bool IsMovieFocused() const;
    virtual bool GetDirtyFlag(bool clear);
    virtual void SetMouseCursorCount(unsigned int n);
    virtual unsigned int GetMouseCursorCount() const;
    virtual void SetControllerCount(unsigned int n);
    virtual unsigned int GetControllerCount() const;
    virtual void GetStats(GStatBag* bag, bool reset);
    virtual GMemoryHeap* GetHeap() const;
    virtual void ForceCollectGarbage();
    virtual GPoint<float> TranslateToScreen(const GPoint<float>& p, GMatrix2D m);
    virtual GRect<float> TranslateToScreen(const GRect<float>& r, GMatrix2D m);
    virtual bool TranslateLocalToScreen(const char* path, const GPoint<float>& p,
                                        GPoint<float>* out, GMatrix2D m);
    virtual bool SetControllerFocusGroup(unsigned int ctrlIdx, unsigned int group);
    virtual unsigned int GetControllerFocusGroup(unsigned int ctrlIdx) const;
    virtual GFxMovieDef::MemoryContext* GetMemoryContext() const;
    virtual void Release();

    // --- GFxStateBag (4), with real storage rather than the bringup bodies ---
    virtual GFxStateBag* GetStateBagImpl() const { return (GFxStateBag*)this; }
    virtual void SetState(GFxState::StateType t, GFxState* s);
    virtual GFxState* GetStateAddRef(GFxState::StateType t) const;
    virtual void GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                 unsigned int count) const;

    // --- the runtime's own surface ---
    GASGlobalContext* GetASContext() const { return pGC; }
    GASEnvironment*   GetASEnvironment() { return &Env; }
    GFxSprite*        GetLevel0() const { return pLevel0; }
    GFxSprite*        GetMainContainer() const { return pLevel0; }
    GFxMovieDefImpl*  GetMovieDefImpl() const { return pDefImpl; }

    // The action queue. A DoAction tag does not run inline: GASDoAction::Execute pushes onto the
    // session GFxSprite::CallFrameActions opened, and DoActionsForSession drains it. That ordering
    // is why a class registered in frame 1 is visible to frame 1's own timeline actions.
    void PushActionBuffer(GASActionBuffer* buffer, GFxSprite* target, GFxActionPriority prio);
    // The other kind of queue entry retail has: construct this clip as the AS2 class registered for
    // its library symbol. It is queued rather than run inline because the frame's own init actions
    // are what register the class (2012 0x9fee10's InsertEntry(queue, 1)).
    void QueueClassBinding(GFxSprite* target, const char* exportSymbol);
    void DrainActionSessions();
    void DoActions();
    unsigned int GetQueuedActionCount() const { return ActionCount; }

    GASString CreateString(const char* s);
    void      ASValue2GFxValue(GASEnvironment* env, const GASValue& in, GFxValue* out);
    void      GFxValue2ASValue(const GFxValue& in, GASValue* out);
    GFxValue::ObjectInterface* GetObjectInterface() { return &ObjInterface; }

    // Everything the harness reports. The counters are incremented where the events happen, not
    // reconstructed afterwards.
    struct Census
    {
        unsigned int SpritesCreated;
        unsigned int GenericCharactersCreated;
        unsigned int DisplayObjectsPlaced;
        unsigned int DisplayObjectsMoved;
        unsigned int DisplayObjectsRemoved;
        unsigned int ActionBuffersRun;
        unsigned int FramesAdvanced;
        unsigned int ClassesRegistered;
        unsigned int ScriptErrors;
    };
    Census& GetCensus() { return Stats; }

    // What the last Display() pass submitted; the engine's census line reports it.
    const GFxDisplayStats& GetDisplayStats() const { return LastDisplayStats; }
    GFxDisplayStats LastDisplayStats;

    void LogScriptError(const char* fmt, ...);

    // --- the input half (GFxInput.cpp) ---
    enum { MaxKeyboards = 4, MaxMice = 4 };
    GFxKeyboardState* GetKeyboardState(unsigned int index);           // 2012 0x9cd8b0
    void SetKeyboardListener(GFxKeyboardState::IListener* l);         // 2012 0xa01730
    void ProcessInput();                                              // 2012 0xa10d80
    void ProcessKeyboard(const GFxInputEventsQueue::QueueEntry& e);   // 2012 0xa0cfa0
    void ProcessMouse(const GFxInputEventsQueue::QueueEntry& e);      // 2012 0xa0e900
    // The movie's own pixel rectangle that maps onto the viewport: what BeginDisplay is given and
    // what a viewport-space mouse position is mapped back through. Retail keeps it as four floats on
    // the movie root (this+36..39 in the decompile of 0xa07aa0) and recomputes it in SetViewport.
    void GetVisibleFrameRectPixels(float* x0, float* y0, float* x1, float* y1) const;
    GPoint<float> ViewportToTwips(float x, float y) const;

    // Set by the harness to narrate the three teardown steps; off in any other build.
    static bool bTraceTeardown;
    // DISHONORED(bringup, agent DC): narrates the constructor, which is where the first in-game
    // instantiation of the machine failed. Off unless the host sets it.
    static bool bTraceConstruction;

private:
    struct ActionEntry
    {
        GASActionBuffer*  pBuffer;        // null for a class binding
        GFxSprite*        pTarget;
        GFxActionPriority Priority;
        unsigned int      Session;
        const char*       BindSymbol;     // into the data def's export table, which outlives the queue
    };

    enum { MaxStates = 40 };

    GFxMovieDefImpl*          pDefImpl;
    GASGlobalContext*         pGC;
    GASEnvironment            Env;
    GFxSprite*                pLevel0;
    GFxValue::ObjectInterface ObjInterface;
    GViewport                 Viewport;
    GFxMovieView::ScaleModeType ScaleMode;
    GFxMovieView::AlignType   Alignment;
    GColor                    BackgroundColor;
    float                     BackgroundAlpha;
    bool                      bPaused;
    bool                      bVisible;
    bool                      bDirty;
    void*                     pUserData;
    float                     TimeElapsed;
    float                     FrameTime;
    unsigned int              MouseCursorCount;
    unsigned int              ControllerCount;
    bool                      bMovieFocused;
    GFxKeyboardState          KeyboardStates[MaxKeyboards];
    GFxInputEventsQueue       InputQueue;
    float                     MouseX[MaxMice];
    float                     MouseY[MaxMice];
    unsigned int              MouseButtons[MaxMice];
    GFxState*                 States[MaxStates];

    ActionEntry* Actions;
    unsigned int ActionCount;
    unsigned int SessionFill;            // the session a push joins; see GFxMovieRoot::DoActions
    unsigned int ActionCapacity;
    bool         bInActionQueue;

    Census Stats;
};

#pragma pack(pop)
#endif // INC_GFX3_GFXPLAYER_H
