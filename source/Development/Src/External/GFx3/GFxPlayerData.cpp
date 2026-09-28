// Scaleform GFx 3.3.89 - GFxStream, the tag walk, the timeline and character model, and
// GFxMovieDataDef / GFxMovieDefImpl. Package BC.
//
// The tag set this walk handles is not a guess either: retail registers one loader per code through
// GFxLoaderImpl::GetTagLoader (2012 0x9badf0), and the 43 free functions named GFx_*Loader in
// libgfx are exactly that table. The ones that matter for a timeline to run are
// GFx_PlaceObjectLoader (0xa37910), GFx_PlaceObject2Loader (0xa379b0), GFx_PlaceObject3Loader
// (0xa37ac0), GFx_RemoveObjectLoader (0xa37b90), GFx_RemoveObject2Loader (0xa37c00),
// GFx_DoActionLoader (0x9e43a0), GFx_DoInitActionLoader (0x9e4420), GFx_SpriteLoader (0xa35a00),
// GFx_ExportLoader (0xa35bf0), GFx_ImportLoader (0xa385f0), GFx_FrameLabelLoader (0xa351b0) and
// GFx_SetBackgroundColorLoader (0xa36cd0). Those twelve are ported here. The remaining 31 are the
// shape, font, text, button, image and filter loaders: every one of them defines a character, so
// each is recorded as a GFxPlaceholderDef with its tag code and body size, which keeps the character
// dictionary complete and the display list honest while the geometry is still missing.
// DISHONORED(port): see GFxAS2.h.
#include "GFxCharacterDefs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ---------------------------------------------------------------------------------------------
// GFxStream

GFxStream::GFxStream(const unsigned char* data, unsigned int size)
    : Data(data), Size(size), Pos(0), TagEnd(size), BitBuf(0), BitPos(0) {}

unsigned char GFxStream::ReadU8()
{
    Align();
    return Pos < Size ? Data[Pos++] : 0;
}

unsigned short GFxStream::ReadU16()
{
    Align();
    if (Pos + 2 > Size) { Pos = Size; return 0; }
    unsigned short v = (unsigned short)(Data[Pos] | ((unsigned int)Data[Pos + 1] << 8));
    Pos += 2;
    return v;
}

unsigned int GFxStream::ReadU32()
{
    Align();
    if (Pos + 4 > Size) { Pos = Size; return 0; }
    unsigned int v = (unsigned int)Data[Pos] | ((unsigned int)Data[Pos + 1] << 8)
                   | ((unsigned int)Data[Pos + 2] << 16) | ((unsigned int)Data[Pos + 3] << 24);
    Pos += 4;
    return v;
}

float GFxStream::ReadFloat()
{
    unsigned int bits = ReadU32();
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

double GFxStream::ReadDouble()
{
    unsigned int lo = ReadU32();
    unsigned int hi = ReadU32();
    unsigned int parts[2] = { lo, hi };
    double d;
    memcpy(&d, parts, 8);
    return d;
}

float GFxStream::ReadFixed88()
{
    short v = ReadS16();
    return (float)v / 256.0f;
}

unsigned int GFxStream::ReadUBits(unsigned int bits)
{
    unsigned int value = 0;
    while (bits > 0)
    {
        if (BitPos == 0)
        {
            BitBuf = Pos < Size ? Data[Pos++] : 0;
            BitPos = 8;
        }
        unsigned int take = bits < BitPos ? bits : BitPos;
        unsigned int shift = BitPos - take;
        unsigned int mask = (1u << take) - 1u;
        value = (value << take) | ((BitBuf >> shift) & mask);
        BitPos -= take;
        bits -= take;
    }
    return value;
}

int GFxStream::ReadSBits(unsigned int bits)
{
    if (bits == 0)
        return 0;
    unsigned int raw = ReadUBits(bits);
    if (raw & (1u << (bits - 1)))
        raw |= ~((1u << bits) - 1u);
    return (int)raw;
}

void GFxStream::ReadString(char* out, unsigned int outSize)
{
    Align();
    unsigned int n = 0;
    while (Pos < Size && Data[Pos] != 0)
    {
        if (n + 1 < outSize)
            out[n++] = (char)Data[Pos];
        ++Pos;
    }
    if (Pos < Size) ++Pos;
    out[n < outSize ? n : outSize - 1] = 0;
}

void GFxStream::ReadStringWithLength(char* out, unsigned int outSize)
{
    Align();
    unsigned int len = ReadU8();
    unsigned int n = 0;
    for (unsigned int i = 0; i < len && Pos < Size; ++i, ++Pos)
        if (n + 1 < outSize)
            out[n++] = (char)Data[Pos];
    out[n < outSize ? n : outSize - 1] = 0;
}

void GFxStream::ReadRect(int* outLRTB)
{
    Align();
    unsigned int nbits = ReadUBits(5);
    outLRTB[0] = ReadSBits(nbits);
    outLRTB[1] = ReadSBits(nbits);
    outLRTB[2] = ReadSBits(nbits);
    outLRTB[3] = ReadSBits(nbits);
    Align();
}

void GFxStream::ReadMatrix(GMatrix2D* out)
{
    Align();
    out->SetIdentity();
    if (ReadUBits(1))
    {
        unsigned int nbits = ReadUBits(5);
        out->M_[0][0] = (float)ReadSBits(nbits) / 65536.0f;
        out->M_[1][1] = (float)ReadSBits(nbits) / 65536.0f;
    }
    if (ReadUBits(1))
    {
        unsigned int nbits = ReadUBits(5);
        out->M_[1][0] = (float)ReadSBits(nbits) / 65536.0f;
        out->M_[0][1] = (float)ReadSBits(nbits) / 65536.0f;
    }
    unsigned int tbits = ReadUBits(5);
    out->M_[0][2] = (float)ReadSBits(tbits);
    out->M_[1][2] = (float)ReadSBits(tbits);
    Align();
}

void GFxStream::ReadCxformRgb(GRenderer::Cxform* out)
{
    Align();
    for (int i = 0; i < 4; ++i) { out->M_[i][0] = 1.f; out->M_[i][1] = 0.f; }
    unsigned int hasAdd = ReadUBits(1);
    unsigned int hasMult = ReadUBits(1);
    unsigned int nbits = ReadUBits(4);
    if (hasMult)
        for (int i = 0; i < 3; ++i)
            out->M_[i][0] = (float)ReadSBits(nbits) / 256.0f;
    if (hasAdd)
        for (int i = 0; i < 3; ++i)
            out->M_[i][1] = (float)ReadSBits(nbits);
    Align();
}

void GFxStream::ReadCxformRgba(GRenderer::Cxform* out)
{
    Align();
    for (int i = 0; i < 4; ++i) { out->M_[i][0] = 1.f; out->M_[i][1] = 0.f; }
    unsigned int hasAdd = ReadUBits(1);
    unsigned int hasMult = ReadUBits(1);
    unsigned int nbits = ReadUBits(4);
    if (hasMult)
        for (int i = 0; i < 4; ++i)
            out->M_[i][0] = (float)ReadSBits(nbits) / 256.0f;
    if (hasAdd)
        for (int i = 0; i < 4; ++i)
            out->M_[i][1] = (float)ReadSBits(nbits);
    Align();
}

void GFxStream::ReadRgb(GColor* out)
{
    unsigned char r = ReadU8(), g = ReadU8(), b = ReadU8();
    *out = GColor(r, g, b, 255);
}

void GFxStream::ReadRgba(GColor* out)
{
    unsigned char r = ReadU8(), g = ReadU8(), b = ReadU8(), a = ReadU8();
    *out = GColor(r, g, b, a);
}

bool GFxStream::OpenTag(unsigned int* outCode, unsigned int* outEndPos)
{
    Align();
    if (Pos + 2 > Size)
        return false;
    unsigned int header = ReadU16();
    unsigned int code = header >> 6;
    unsigned int len = header & 0x3F;
    if (len == 0x3F)
        len = ReadU32();
    TagEnd = Pos + len;
    if (TagEnd > Size)
        TagEnd = Size;
    *outCode = code;
    *outEndPos = TagEnd;
    return true;
}

// ---------------------------------------------------------------------------------------------
// GFxCharPosInfo and the execute tags

GFxCharPosInfo::GFxCharPosInfo()
    : PlaceFlags(0), Depth(0), CharacterId(0), Ratio(0.f), ClipDepth(0), BlendMode(0)
{
    Name[0] = 0;
    for (int i = 0; i < 4; ++i) { ColorTransform.M_[i][0] = 1.f; ColorTransform.M_[i][1] = 0.f; }
}

// DISHONORED(bringup, agent DK): the filter census. One line per distinct (kind, instance name).
static unsigned int GFxDK_FilterCounts[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
void GFxDK_NoteFilter(unsigned char kind, const char* name)
{
    static const char* Kinds[8] = { "DropShadow", "Blur", "Glow", "Bevel", "GradientGlow",
                                    "Convolution", "ColorMatrix", "GradientBevel" };
    if (kind < 8)
        ++GFxDK_FilterCounts[kind];
    GFxLogf("DISHONORED(bringup): PlaceObject3 filter: %s on '%s'",
            kind < 8 ? Kinds[kind] : "unknown", (name && name[0]) ? name : "<unnamed>");
}

void GFxPlaceObject2Tag::Read(GFxStream* s, unsigned int tagCode)
{
    // The flag bits and their order are GFxPlaceObject2::UnpackBase's (2012 0xa0bab0).
    bIsPlaceObject3 = (tagCode == 70);
    Pos.PlaceFlags = s->ReadU8();
    unsigned int flags3 = bIsPlaceObject3 ? s->ReadU8() : 0;
    Pos.Depth = s->ReadU16();
    if (bIsPlaceObject3)
    {
        if (flags3 & 0x08) { char cls[128]; s->ReadString(cls, sizeof(cls)); }
    }
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasCharacter)
        Pos.CharacterId = s->ReadU16();
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasMatrix)
        s->ReadMatrix(&Pos.Matrix);
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasCxform)
        s->ReadCxformRgba(&Pos.ColorTransform);
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasRatio)
        Pos.Ratio = (float)s->ReadU16() / 65535.0f;
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasName)
        s->ReadString(Pos.Name, sizeof(Pos.Name));
    if (Pos.PlaceFlags & GFxCharPosInfo::Place_HasClipDepth)
        Pos.ClipDepth = s->ReadU16();
    if (bIsPlaceObject3)
    {
        if (flags3 & 0x01)
        {
            // DISHONORED(port): GFx_LoadFilters (2012 0xa93b60) - a u8 count and that many records.
            // The records are stepped over by their fixed sizes, as GFxButtonRecord::Read does, and
            // counted per kind so the census can say what the cook asks for.
            const unsigned int count = s->ReadU8();
            for (unsigned int i = 0; i < count; ++i)
            {
                const unsigned char kind = s->ReadU8();
                GFxDK_NoteFilter(kind, Pos.HasName() ? Pos.Name : "");
                switch (kind)
                {
                case 0: s->Skip(23); break;                            // drop shadow
                case 1: s->Skip(9);  break;                            // blur
                case 2: s->Skip(15); break;                            // glow
                case 3: s->Skip(27); break;                            // bevel
                case 4: case 7: { const unsigned int n = s->ReadU8(); s->Skip(5u * n + 19u); break; }
                case 5: { const unsigned int mx = s->ReadU8(); const unsigned int my = s->ReadU8();
                          s->Skip(8u + 4u * mx * my + 1u); break; }
                case 6: s->Skip(80); break;                            // colour matrix
                default: i = count; break;
                }
            }
        }
        if (flags3 & 0x02) Pos.BlendMode = s->ReadU8();
        if (flags3 & 0x04) s->ReadU8();
    }
    // Clip actions are the `on(press)` handlers; they need the button event model, which is not this
    // wave. The tag body is closed by length, so skipping them costs nothing downstream.
}

void GFxPlaceObject2Tag::Execute(GFxSprite* sprite)                   // via 2012 0xa0d440
{
    if (sprite == 0)
        return;
    // ExecuteBase's three-way split: a place with a character id creates, a move without one moves,
    // and a place with both the move flag and a character id replaces.
    const bool hasChar = Pos.HasCharacter();
    const bool isMove = Pos.IsMove();
    if (hasChar && !isMove)
        sprite->AddDisplayObject(Pos);
    else if (!hasChar && isMove)
        sprite->MoveDisplayObject(Pos);
    else if (hasChar && isMove)
        sprite->ReplaceDisplayObject(Pos);
}

void GFxRemoveObject2Tag::Read(GFxStream* s, unsigned int tagCode)
{
    if (tagCode == GFxTag_RemoveObject2)
    {
        Depth = s->ReadU16();
        bHasCharacterId = false;
    }
    else
    {
        CharacterId = s->ReadU16();
        Depth = s->ReadU16();
        bHasCharacterId = true;
    }
}

void GFxRemoveObject2Tag::Execute(GFxSprite* sprite)                  // 2012 0xa01d90
{
    if (sprite == 0)
        return;
    GFxResourceId id;
    id.Id = bHasCharacterId ? CharacterId : GFxResourceId::InvalidId;
    sprite->RemoveDisplayObject(Depth, id);
}

void GFxSetBackgroundColorTag::Read(GFxStream* s)
{
    s->ReadRgb(&Color);
}

void GFxSetBackgroundColorTag::Execute(GFxSprite* sprite)
{
    if (sprite && sprite->GetMovieRoot())
        sprite->GetMovieRoot()->SetBackgroundColor(Color);
}

void GASDoActionTag::Read(GFxStream* s, unsigned int endPos)           // 2012 0x9e18d0
{
    unsigned int start = s->Tell();
    if (endPos > start)
        Buffer.SetBytes(s->GetPtr(start), endPos - start);
}

void GASDoActionTag::Execute(GFxSprite* sprite)                        // 2012 0x9e4240
{
    ExecuteWithPriority(sprite, GFxAP_Frame);
}

void GASDoActionTag::ExecuteWithPriority(GFxSprite* sprite, GFxActionPriority prio) // 0x9e42b0
{
    if (sprite == 0 || sprite->GetMovieRoot() == 0)
        return;
    sprite->GetMovieRoot()->PushActionBuffer(&Buffer, sprite, prio);
}

void GASDoInitActionTag::Read(GFxStream* s, unsigned int endPos)        // via 2012 0x9e4420
{
    SpriteId = s->ReadU16();
    unsigned int start = s->Tell();
    if (endPos > start)
        Buffer.SetBytes(s->GetPtr(start), endPos - start);
}

void GASDoInitActionTag::Execute(GFxSprite* sprite)                    // 2012 0x9e4330
{
    // An init action runs once, before the frame's own actions, at Init priority. That ordering is
    // what makes a __Packages class visible to the frame-1 code that instantiates it.
    if (sprite == 0 || sprite->GetMovieRoot() == 0)
        return;
    sprite->GetMovieRoot()->PushActionBuffer(&Buffer, sprite, GFxAP_Init);
}

// ---------------------------------------------------------------------------------------------
// GFxTagList / GFxTimelineDef

GFxTagList::~GFxTagList()
{
    for (unsigned int i = 0; i < Size; ++i)
        delete Tags[i];
    free(Tags);
}

void GFxTagList::Add(GASExecuteTag* tag)
{
    if (Size >= Capacity)
    {
        Capacity = Capacity ? Capacity * 2 : 8;
        Tags = (GASExecuteTag**)realloc(Tags, Capacity * sizeof(GASExecuteTag*));
    }
    Tags[Size++] = tag;
}

GFxTimelineDef::GFxTimelineDef()
    : Frames(0), InitActions(0), FrameCapacity(0), FrameCount(0), Labels(0), LabelCount(0),
      LabelCapacity(0) {}

GFxTimelineDef::~GFxTimelineDef()
{
    delete[] Frames;
    delete[] InitActions;
    free(Labels);
}

void GFxTimelineDef::BeginFrames(unsigned int count)
{
    unsigned int cap = count ? count : 1;
    // A GFX header can under-report the frame count when the last frame carries no ShowFrame, so the
    // array is grown on demand by AddTagToFrame as well.
    Frames = new GFxTagList[cap];
    InitActions = new GFxTagList[cap];
    FrameCapacity = cap;
    FrameCount = count;
}

void GFxTimelineDef::GrowFrames(unsigned int need)
{
    // A declared frame count is a lower bound, not a promise: a movie or a sprite can carry tags past
    // its header count, and dropping them would silently lose a frame's whole playlist.
    if (need <= FrameCapacity)
        return;
    unsigned int cap = FrameCapacity ? FrameCapacity * 2 : 8;
    if (cap < need) cap = need;
    GFxTagList* nextFrames = new GFxTagList[cap];
    GFxTagList* nextInits = new GFxTagList[cap];
    for (unsigned int i = 0; i < FrameCapacity; ++i)
    {
        nextFrames[i].TakeFrom(Frames[i]);
        nextInits[i].TakeFrom(InitActions[i]);
    }
    delete[] Frames;
    delete[] InitActions;
    Frames = nextFrames;
    InitActions = nextInits;
    FrameCapacity = cap;
}

void GFxTimelineDef::AddTagToFrame(unsigned int frame, GASExecuteTag* tag)
{
    GrowFrames(frame + 1);
    Frames[frame].Add(tag);
    if (frame + 1 > FrameCount)
        FrameCount = frame + 1;
}

void GFxTimelineDef::AddInitActionToFrame(unsigned int frame, GASExecuteTag* tag)
{
    GrowFrames(frame + 1);
    InitActions[frame].Add(tag);
}

const GFxTagList* GFxTimelineDef::GetPlaylist(unsigned int frame) const
{
    return frame < FrameCapacity ? &Frames[frame] : 0;
}

const GFxTagList* GFxTimelineDef::GetInitActionList(unsigned int frame) const
{
    return frame < FrameCapacity ? &InitActions[frame] : 0;
}

void GFxTimelineDef::AddFrameLabel(const char* name, unsigned int frame)  // 2012 0xa003f0
{
    if (LabelCount >= LabelCapacity)
    {
        LabelCapacity = LabelCapacity ? LabelCapacity * 2 : 8;
        Labels = (FrameLabel*)realloc(Labels, LabelCapacity * sizeof(FrameLabel));
    }
    strncpy(Labels[LabelCount].Name, name, sizeof(Labels[0].Name) - 1);
    Labels[LabelCount].Name[sizeof(Labels[0].Name) - 1] = 0;
    Labels[LabelCount].Frame = frame;
    ++LabelCount;
}

bool GFxTimelineDef::GetLabeledFrame(const char* label, unsigned int* outFrame) const  // 0x9fe2d0
{
    for (unsigned int i = 0; i < LabelCount; ++i)
        if (_stricmp(Labels[i].Name, label) == 0)
        {
            *outFrame = Labels[i].Frame;
            return true;
        }
    return false;
}

// ---------------------------------------------------------------------------------------------
// Character definitions

unsigned int GFxPlaceholderDef::GetResourceTypeCode() const
{
    // RT_None is the point: every tag that defines a real character now has a loader, so a
    // placeholder means one thing only - an imported symbol whose source movie has not been bound -
    // and the resource type has to say so, because that is what BindImports repoints.
    return GFxResource::RT_None;
}

const char* GFxPlaceholderDef::GetDefTypeName() const
{
    return TagCode == 71 || TagCode == 57 ? "UnboundImport" : "Placeholder";
}

GFxCharacter* GFxPlaceholderDef::CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                         GFxMovieDefImpl* defImpl)
{
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

unsigned int GFxSpriteDef::GetResourceTypeCode() const
{
    return GFxResource::RT_SpriteDef;
}

GFxCharacter* GFxSpriteDef::CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                    GFxMovieDefImpl* defImpl)  // 2012 0xa00390
{
    return new GFxSprite(this, this, defImpl, parent, id, parent ? parent->GetMovieRoot() : 0);
}

const char* GFxGenericCharacter::GetCharacterTypeName() const
{
    return pDef ? pDef->GetDefTypeName() : "Character";
}

// The object type follows the definition, but only inside the character span [2,5] that
// GFxValue::ObjectInterface::GetMember (2012 0x9b0450) routes to ToASCharacter: a generic character
// is a display object, so a script-object type here would send every member access down the wrong
// branch. Retail's GFxGenericCharacter does not override this at all and takes GFxASCharacter's
// value; the one case worth distinguishing is a button, because the object type is how the button's
// event model will find it when package CB's focus and mouse path lands.
GASObjectType GFxGenericCharacter::GetObjectType() const
{
    return pDef != 0 && pDef->GetResourceTypeCode() == GFxResource::RT_ButtonDef
               ? Object_Button : Object_EditText;
}

// ---------------------------------------------------------------------------------------------
// GFxMovieDataDef

GFxMovieDataDef::GFxMovieDataDef()
    : Dict(0), DictSize(0), DictCapacity(0), Exports(0), ExportSize(0), ExportCapacity(0),
      Imports(0), ImportSize(0), ImportCapacity(0), SWFFlags(0),
      Version(0), FrameRate(0.f), WidthPixels(0.f), HeightPixels(0.f)
{
    memset(FrameRectTwips, 0, sizeof(FrameRectTwips));
    memset(&Stats, 0, sizeof(Stats));
}

GFxMovieDataDef::~GFxMovieDataDef()
{
    // An imported definition belongs to the movie that exported it, so a bound slot is skipped here;
    // that is the same ownership rule as retail's resource handles.
    for (unsigned int i = 0; i < DictSize; ++i)
    {
        bool bImported = false;
        for (unsigned int k = 0; k < ImportSize && !bImported; ++k)
            bImported = Imports[k].bBound && Imports[k].Id == Dict[i].Id;
        if (!bImported)
            delete Dict[i].pDef;
    }
    free(Dict);
    free(Exports);
    free(Imports);
}

unsigned int GFxMovieDataDef::GetResourceTypeCode() const
{
    return GFxResource::RT_MovieDataDef;
}

GRect<float> GFxMovieDataDef::GetFrameRect() const
{
    return GRect<float>((float)FrameRectTwips[0] * GFxTwipsToPixels,
                        (float)FrameRectTwips[2] * GFxTwipsToPixels,
                        (float)FrameRectTwips[1] * GFxTwipsToPixels,
                        (float)FrameRectTwips[3] * GFxTwipsToPixels);
}

void GFxMovieDataDef::AddCharacter(unsigned int id, GFxCharacterDef* def)
{
    if (DictSize >= DictCapacity)
    {
        DictCapacity = DictCapacity ? DictCapacity * 2 : 64;
        Dict = (DictEntry*)realloc(Dict, DictCapacity * sizeof(DictEntry));
    }
    def->Id.Id = id;
    Dict[DictSize].Id = id;
    Dict[DictSize].pDef = def;
    ++DictSize;
    ++Stats.Characters;
}

GFxCharacterDef* GFxMovieDataDef::GetCharacterDefById(unsigned int id) const
{
    for (unsigned int i = 0; i < DictSize; ++i)
        if (Dict[i].Id == id)
            return Dict[i].pDef;
    return 0;
}

void GFxMovieDataDef::AddExport(const char* name, unsigned int id)
{
    if (ExportSize >= ExportCapacity)
    {
        ExportCapacity = ExportCapacity ? ExportCapacity * 2 : 64;
        Exports = (ExportEntry*)realloc(Exports, ExportCapacity * sizeof(ExportEntry));
    }
    strncpy(Exports[ExportSize].Name, name, sizeof(Exports[0].Name) - 1);
    Exports[ExportSize].Name[sizeof(Exports[0].Name) - 1] = 0;
    Exports[ExportSize].Id = id;
    ++ExportSize;
    ++Stats.Exports;
}

// The reverse of GetExportedId: what Object.registerClass is keyed by. Retail reads it out of the
// character def itself (`NameOfExportedResource` in the decompile of GFxSprite::AddDisplayObject,
// 2012 0x9fee10); the export table is the same mapping read the other way round.
const char* GFxMovieDataDef::GetExportedName(unsigned int id) const
{
    for (unsigned int i = 0; i < ExportSize; ++i)
        if (Exports[i].Id == id)
            return Exports[i].Name;
    // An IMPORTED character is exported by the movie it came from, and retail reads the name off the
    // definition itself, so the import symbol is the same string Object.registerClass was keyed by.
    for (unsigned int i = 0; i < ImportSize; ++i)
        if (Imports[i].Id == id)
            return Imports[i].Symbol;
    return 0;
}

GFxMovieDataDef* GFxMovieDataDef::FindImportSource(const char* url) const
{
    if (url == 0)
        return 0;
    for (unsigned int i = 0; i < ImportSize; ++i)
        if (Imports[i].pSource != 0 && strcmp(Imports[i].Url, url) == 0)
            return Imports[i].pSource;
    return 0;
}

// 2012 0x9f46c0's body, one level down: every frame's init-action list of the imported movie, run
// against the sprite that imported it.
void GFxMovieDataDef::ExecuteInitActionsOn(GFxSprite* sprite)
{
    for (unsigned int frame = 0; frame < GetFrameCount(); ++frame)
    {
        const GFxTagList* list = GetInitActionList(frame);
        if (list == 0)
            continue;
        for (unsigned int i = 0; i < list->GetSize(); ++i)
            (*list)[i]->Execute(sprite);
    }
}

GASImportInitActionsTag::GASImportInitActionsTag(GFxMovieDataDef* owner, const char* url)
    : pOwner(owner)
{
    Url[0] = 0;
    if (url != 0)
    {
        unsigned int n = 0;
        while (url[n] != 0 && n + 1 < sizeof(Url)) { Url[n] = url[n]; ++n; }
        Url[n] = 0;
    }
}

void GASImportInitActionsTag::Execute(GFxSprite* sprite)
{
    if (pOwner == 0 || sprite == 0)
        return;
    GFxMovieDataDef* source = pOwner->FindImportSource(Url);
    if (source != 0 && source != pOwner)
        source->ExecuteInitActionsOn(sprite);
}

int GFxMovieDataDef::GetExportedId(const char* name) const
{
    for (unsigned int i = 0; i < ExportSize; ++i)
        if (strcmp(Exports[i].Name, name) == 0)
            return (int)Exports[i].Id;
    // An imported symbol is in this movie's library under the name it was imported by, which is what
    // attachMovie is given.
    for (unsigned int i = 0; i < ImportSize; ++i)
        if (Imports[i].bBound && strcmp(Imports[i].Symbol, name) == 0)
            return (int)Imports[i].Id;
    return -1;
}

GFxCharacterDef* GFxMovieDataDef::GetExportedCharacter(const char* name) const
{
    int id = GetExportedId(name);
    return id < 0 ? 0 : GetCharacterDefById((unsigned int)id);
}

GFxCharacterDef* GFxMovieDataDef::FindExportedCharacter(const char* name, unsigned int depth) const
{
    GFxCharacterDef* def = GetExportedCharacter(name);
    if (def != 0 || depth >= 8)
        return def;
    for (unsigned int i = 0; i < ImportSize; ++i)
    {
        GFxMovieDataDef* source = Imports[i].pSource;
        if (source == 0 || source == this)
            continue;
        bool seen = false;
        for (unsigned int j = 0; j < i && !seen; ++j)
            seen = Imports[j].pSource == source;
        if (seen)
            continue;
        def = source->FindExportedCharacter(name, depth + 1);
        if (def != 0)
            return def;
    }
    return 0;
}

GFxCharacter* GFxMovieDataDef::CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                       GFxMovieDefImpl* defImpl)
{
    return new GFxSprite(this, this, defImpl, parent, id, parent ? parent->GetMovieRoot() : 0);
}

void GFxMovieDataDef::NoteSkipped(unsigned int code)
{
    ++Stats.TagsSkipped;
    for (unsigned int i = 0; i < Stats.SkippedCodeCount; ++i)
        if (Stats.SkippedCodes[i] == code)
        {
            ++Stats.SkippedCounts[i];
            return;
        }
    if (Stats.SkippedCodeCount < 96)
    {
        Stats.SkippedCodes[Stats.SkippedCodeCount] = code;
        Stats.SkippedCounts[Stats.SkippedCodeCount] = 1;
        ++Stats.SkippedCodeCount;
    }
}

void GFxMovieDataDef::NoteDefined(GFxCharacterDef* def)
{
    switch (def->GetResourceTypeCode())
    {
    case GFxResource::RT_ShapeDef:
        if (strcmp(def->GetDefTypeName(), "MorphShape") == 0) ++Stats.MorphShapes;
        else                                                  ++Stats.Shapes;
        break;
    case GFxResource::RT_EditTextDef: ++Stats.EditTexts;   break;
    case GFxResource::RT_TextDef:     ++Stats.StaticTexts; break;
    case GFxResource::RT_ButtonDef:   ++Stats.Buttons;     break;
    case GFxResource::RT_Font:        ++Stats.Fonts;       break;
    case GFxResource::RT_Image:       ++Stats.Images;      break;
    case GFxResource::RT_SpriteDef:   ++Stats.Sprites;     break;
    default:                          ++Stats.Placeholders; break;
    }
}

void GFxMovieDataDef::SetSourceUrl(const char* url)
{
    SourceUrl[0] = 0;
    if (url == 0)
        return;
    unsigned int n = 0;
    while (url[n] && n + 1 < sizeof(SourceUrl))
    {
        SourceUrl[n] = url[n];
        ++n;
    }
    SourceUrl[n] = 0;
}

void GFxMovieDataDef::AddImport(const char* url, const char* symbol, unsigned int id)
{
    if (ImportSize >= ImportCapacity)
    {
        ImportCapacity = ImportCapacity ? ImportCapacity * 2 : 16;
        Imports = (ImportEntry*)realloc(Imports, ImportCapacity * sizeof(ImportEntry));
    }
    ImportEntry& e = Imports[ImportSize++];
    strncpy(e.Url, url, sizeof(e.Url) - 1);          e.Url[sizeof(e.Url) - 1] = 0;
    strncpy(e.Symbol, symbol, sizeof(e.Symbol) - 1); e.Symbol[sizeof(e.Symbol) - 1] = 0;
    e.Id = id;
    e.bBound = false;
    e.pSource = 0;
    ++Stats.Imports;
    // The dictionary slot is taken now and repointed at bind time, which is what retail's
    // GFxMovieDataDef::LoadTaskData::AddNewResourceHandle does inside GFx_ImportLoader: a handle with
    // no resource behind it yet. Until BindImports runs the slot answers as a placeholder, so a
    // PlaceObject naming it still produces a character rather than nothing.
    AddCharacter(id, new GFxPlaceholderDef(71, 0));
}

void GFxMovieDataDef::NoteActionBytes(unsigned int bytes, bool bInit)
{
    if (bInit) ++Stats.DoInitActions;
    else       { ++Stats.DoActions; Stats.ActionBytes += bytes; }
}

void GFxMovieDataDef::ReadSpriteTags(GFxStream* s, GFxSpriteDef* sprite, unsigned int endPos)
{
    ReadTags(s, sprite, endPos, 1);
}

// The URL a GFX payload imports from is a relative authoring path and what it has to resolve to is
// the cooked movie that exports the symbol. Retail splits the URL in GFxURLBuilder
// (GFxLoadStates::BuildURL 2012 0xa225c0) and hands the result to the state bag's file opener;
// ImportResolver is that seam and nothing more.
unsigned int GFxMovieDataDef::BindImports(ImportResolver* resolver)
{
    if (resolver == 0)
        return 0;
    unsigned int bound = 0;
    for (unsigned int i = 0; i < ImportSize; ++i)
    {
        ImportEntry& e = Imports[i];
        if (e.bBound)
            continue;
        GFxMovieDataDef* source = resolver->ResolveImportMovie(e.Url);
        if (source == 0)
            continue;
        GFxCharacterDef* def = source->GetExportedCharacter(e.Symbol);
        if (def == 0)
            continue;
        // The imported definition stays owned by the movie that exported it - retail holds a
        // GFxResourceHandle into the other movie's library, not a copy - so the slot is repointed and
        // the placeholder that stood in for it is destroyed.
        for (unsigned int d = 0; d < DictSize; ++d)
        {
            if (Dict[d].Id != e.Id)
                continue;
            GFxCharacterDef* old = Dict[d].pDef;
            Dict[d].pDef = def;
            if (old != 0 && old != def && old->GetResourceTypeCode() == GFxResource::RT_None)
            {
                --Stats.Placeholders;
                delete old;
            }
            break;
        }
        e.bBound = true;
        e.pSource = source;
        ++bound;
        ++Stats.ImportsBound;
        NoteDefined(def);
    }
    return bound;
}

bool GFxMovieDataDef::ReadTags(GFxStream* s, GFxTimelineDef* timeline, unsigned int endPos,
                               unsigned int depth)
{
    // One dispatch through the retail loader table and no tag-specific code here at all, which is
    // what GFxMovieDataDef::LoadTaskData::Read (2012 0xa21d10) is: open the tag, look the loader up
    // through GFxLoaderImpl::GetTagLoader (0x9badf0), call it, close the tag by length.
    (void)depth;
    GFxLoadProcess process(this, s, timeline);
    while (s->Tell() < endPos)
    {
        unsigned int code = 0, tagEnd = 0;
        const unsigned int tagStart = s->Tell();
        if (!s->OpenTag(&code, &tagEnd))
            break;
        ++Stats.Tags;

        if (code == GFxTag_End)
            return true;

        GFxTagInfo info;
        info.TagType = code;
        info.TagOffset = tagStart;
        info.TagDataOffset = s->Tell();
        info.TagLength = tagEnd > info.TagDataOffset ? tagEnd - info.TagDataOffset : 0;

        GFxTagLoaderFn loader = GFxGetTagLoader(code);
        if (loader != 0)
        {
            const unsigned int dictBefore = DictSize;
            loader(&process, info);
            ++Stats.TagsHandled;
            for (unsigned int i = dictBefore; i < DictSize; ++i)
                NoteDefined(Dict[i].pDef);
        }
        else
        {
            // No row in either retail table. GFxStream::CloseTag skips the body by length, which is
            // the same thing retail does when GetTagLoader answers null.
            ++Stats.Unhandled;
            NoteSkipped(code);
        }
        s->CloseTag();
    }
    return true;
}

bool GFxMovieDataDef::Read(const unsigned char* data, unsigned int size)
{
    if (!GFxGfxParseFile(data, size, FileInfo))
        return false;

    Version = FileInfo.Version;
    FrameRate = FileInfo.FrameRate;
    WidthPixels = FileInfo.FrameWidthPixels;
    HeightPixels = FileInfo.FrameHeightPixels;
    for (int i = 0; i < 4; ++i)
        FrameRectTwips[i] = FileInfo.FrameRectTwips[i];

    BeginFrames(FileInfo.FrameCount ? FileInfo.FrameCount : 1);

    GFxStream s(data, size);
    s.SetPosition(FileInfo.FirstTagOffset);
    ReadTags(&s, this, size, 0);
    return true;
}

// ---------------------------------------------------------------------------------------------
// GFxMovieDefImpl

GFxMovieDefImpl::GFxMovieDefImpl(GFxMovieDataDef* dataDef)
    : pDataDef(dataDef), pParentBag(0), bOwnsDataDef(true)
{
    for (int i = 0; i < MaxStates; ++i)
        States[i] = 0;
    FileUrl[0] = 0;
}

GFxMovieDefImpl::~GFxMovieDefImpl()
{
    // DISHONORED(port): a definition handed out of GFxLoaderImpl's cache is shared between every movie
    // that imports it, so only a definition this impl created is deleted with it.
    if (bOwnsDataDef)
        delete pDataDef;
    for (int i = 0; i < MaxStates; ++i)
    {
        if (States[i])
            States[i]->Release();
    }
}

void GFxMovieDefImpl::SetFileURL(const char* url)
{
    FileUrl[0] = 0;
    if (url)
    {
        unsigned int n = 0;
        while (url[n] && n + 1 < sizeof(FileUrl))
        {
            FileUrl[n] = url[n];
            ++n;
        }
        FileUrl[n] = 0;
    }
    if (pDataDef)
        pDataDef->SetSourceUrl(url);
}

unsigned int GFxMovieDefImpl::GetVersion() const { return pDataDef->GetVersion(); }
unsigned int GFxMovieDefImpl::GetLoadingFrame() const { return pDataDef->GetFrameCount(); }
float GFxMovieDefImpl::GetWidth() const { return pDataDef->GetWidth(); }
float GFxMovieDefImpl::GetHeight() const { return pDataDef->GetHeight(); }
unsigned int GFxMovieDefImpl::GetFrameCount() const { return pDataDef->GetFrameCount(); }
float GFxMovieDefImpl::GetFrameRate() const { return pDataDef->GetFrameRate(); }
GRect<float> GFxMovieDefImpl::GetFrameRect() const { return pDataDef->GetFrameRect(); }
unsigned int GFxMovieDefImpl::GetSWFFlags() const { return pDataDef->GetSWFFlags(); }
const char* GFxMovieDefImpl::GetFileURL() const
{
    return FileUrl[0] ? FileUrl : pDataDef->GetFileInfo().ExporterInfo.SWFName;
}
GFxResource* GFxMovieDefImpl::GetMovieDataResource() const { return pDataDef; }
unsigned int GFxMovieDefImpl::GetResourceTypeCode() const { return GFxResource::RT_MovieDef; }

GFxResource* GFxMovieDefImpl::GetResource(const char* name) const
{
    return pDataDef->GetExportedCharacter(name);
}

GFxMovieView* GFxMovieDefImpl::CreateInstance(const GFxMovieDef::MemoryParams& params, bool bd)
{                                                                     // 2012 0xa1b960
    (void)params;
    // DISHONORED(bringup): retail's bInitFirstFrame runs frame 1 inside CreateInstance, which is what
    // makes GetVariable work before the first Advance (UGFxMoviePlayer::PostStart relies on it). It is
    // not done here: the movie has no viewport yet at this point - FGFxEngine::StartScene sets it
    // immediately after - and the engine's own Tick advances it on the next frame either way. The
    // consequence is that a PostStart which reads an AS2 variable sees it one frame later.
    (void)bd;
    return new GFxMovieRoot(this);
}

GFxMovieView* GFxMovieDefImpl::CreateInstance(GFxMovieDef::MemoryContext* ctx, bool bd)
{                                                                     // 2012 0xa1ba00
    (void)ctx; (void)bd;
    return new GFxMovieRoot(this);
}

void GFxMovieDefImpl::SetState(GFxState::StateType t, GFxState* s)
{
    if ((unsigned int)t >= MaxStates)
        return;
    if (s) s->AddRef();
    if (States[t]) States[t]->Release();
    States[t] = s;
}

GFxState* GFxMovieDefImpl::GetStateAddRef(GFxState::StateType t) const
{
    // DISHONORED(port): the chain. A state the definition does not carry is the loader's, which is
    // what retail's GFxLoadStates parent link does.
    if ((unsigned int)t >= MaxStates || States[t] == 0)
        return pParentBag ? pParentBag->GetStateAddRef(t) : 0;
    States[t]->AddRef();
    return States[t];
}

void GFxMovieDefImpl::GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                      unsigned int count) const
{
    for (unsigned int i = 0; i < count; ++i)
        out[i] = GetStateAddRef(types[i]);
}
