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
#include "GFxPlayer.h"

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
    for (int i = 0; i < 4; ++i) { out->M_[0][i] = 1.f; out->M_[1][i] = 0.f; }
    unsigned int hasAdd = ReadUBits(1);
    unsigned int hasMult = ReadUBits(1);
    unsigned int nbits = ReadUBits(4);
    if (hasMult)
        for (int i = 0; i < 3; ++i)
            out->M_[0][i] = (float)ReadSBits(nbits) / 256.0f;
    if (hasAdd)
        for (int i = 0; i < 3; ++i)
            out->M_[1][i] = (float)ReadSBits(nbits);
    Align();
}

void GFxStream::ReadCxformRgba(GRenderer::Cxform* out)
{
    Align();
    for (int i = 0; i < 4; ++i) { out->M_[0][i] = 1.f; out->M_[1][i] = 0.f; }
    unsigned int hasAdd = ReadUBits(1);
    unsigned int hasMult = ReadUBits(1);
    unsigned int nbits = ReadUBits(4);
    if (hasMult)
        for (int i = 0; i < 4; ++i)
            out->M_[0][i] = (float)ReadSBits(nbits) / 256.0f;
    if (hasAdd)
        for (int i = 0; i < 4; ++i)
            out->M_[1][i] = (float)ReadSBits(nbits);
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
    for (int i = 0; i < 4; ++i) { ColorTransform.M_[0][i] = 1.f; ColorTransform.M_[1][i] = 0.f; }
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
        if (flags3 & 0x01) { /* filter list */ }
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
    switch (TagCode)
    {
    case GFxTag_DefineShape:
    case GFxTag_DefineShape2:
    case GFxTag_DefineShape3:
    case GFxTag_DefineShape4:      return GFxResource::RT_ShapeDef;
    case GFxTag_DefineEditText:    return GFxResource::RT_EditTextDef;
    case GFxTag_DefineButton2:     return GFxResource::RT_ButtonDef;
    case GFxTag_DefineFont:
    case GFxTag_DefineFont2:
    case GFxTag_DefineFont3:       return GFxResource::RT_Font;
    default:                       return GFxResource::RT_None;
    }
}

const char* GFxPlaceholderDef::GetDefTypeName() const
{
    switch (TagCode)
    {
    case GFxTag_DefineShape:       return "Shape";
    case GFxTag_DefineShape2:      return "Shape2";
    case GFxTag_DefineShape3:      return "Shape3";
    case GFxTag_DefineShape4:      return "Shape4";
    case GFxTag_DefineEditText:    return "EditText";
    case GFxTag_DefineButton2:     return "Button2";
    case GFxTag_DefineFont:        return "Font";
    case GFxTag_DefineFont2:       return "Font2";
    case GFxTag_DefineFont3:       return "Font3";
    case GFxTag_DefineMorphShape:  return "MorphShape";
    case 11:                       return "Text";
    case 33:                       return "Text2";
    default:                       return "Character";
    }
}

GFxCharacter* GFxPlaceholderDef::CreateCharacterInstance(GFxASCharacter* parent, GFxResourceId id,
                                                         GFxMovieDefImpl* defImpl)
{
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

// ---------------------------------------------------------------------------------------------
// GFxMovieDataDef

GFxMovieDataDef::GFxMovieDataDef()
    : Dict(0), DictSize(0), DictCapacity(0), Exports(0), ExportSize(0), ExportCapacity(0),
      Version(0), FrameRate(0.f), WidthPixels(0.f), HeightPixels(0.f)
{
    memset(FrameRectTwips, 0, sizeof(FrameRectTwips));
    memset(&Stats, 0, sizeof(Stats));
}

GFxMovieDataDef::~GFxMovieDataDef()
{
    for (unsigned int i = 0; i < DictSize; ++i)
        delete Dict[i].pDef;
    free(Dict);
    free(Exports);
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

int GFxMovieDataDef::GetExportedId(const char* name) const
{
    for (unsigned int i = 0; i < ExportSize; ++i)
        if (strcmp(Exports[i].Name, name) == 0)
            return (int)Exports[i].Id;
    return -1;
}

GFxCharacterDef* GFxMovieDataDef::GetExportedCharacter(const char* name) const
{
    int id = GetExportedId(name);
    return id < 0 ? 0 : GetCharacterDefById((unsigned int)id);
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

static bool GFxIsCharacterDefTag(unsigned int code)
{
    // Every tag that begins with a u16 character id, i.e. everything that defines a dictionary
    // entry. The list is the intersection of the SWF spec with the 43 GFx_*Loader functions.
    switch (code)
    {
    case GFxTag_DefineShape:
    case GFxTag_DefineBits:
    case GFxTag_DefineFont:            // 10
    case 11:                           // DefineText
    case 13:                           // DefineFontInfo has an id but defines no character
    case GFxTag_DefineBitsLossless:
    case GFxTag_DefineBitsJPEG2:
    case GFxTag_DefineShape2:
    case 32:
    case 33:                           // DefineText2
    case GFxTag_DefineButton2:
    case GFxTag_DefineBitsJPEG3:
    case GFxTag_DefineBitsLossless2:
    case GFxTag_DefineEditText:
    case GFxTag_DefineMorphShape:
    case GFxTag_DefineFont2:
    case GFxTag_DefineFont3:
    case GFxTag_DefineShape4:
    case GFxTag_DefineBitsJPEG4:
    case 84:                           // DefineMorphShape2
        return true;
    default:
        return false;
    }
}

bool GFxMovieDataDef::ReadTags(GFxStream* s, GFxTimelineDef* timeline, unsigned int endPos,
                               unsigned int depth)
{
    unsigned int frame = 0;
    while (s->Tell() < endPos)
    {
        unsigned int code = 0, tagEnd = 0;
        if (!s->OpenTag(&code, &tagEnd))
            break;
        ++Stats.Tags;
        bool handled = true;

        switch (code)
        {
        case GFxTag_End:
            return true;

        case GFxTag_ShowFrame:
            ++frame;
            break;

        case GFxTag_SetBackgroundColor:
        {
            GFxSetBackgroundColorTag* tag = new GFxSetBackgroundColorTag;
            tag->Read(s);
            timeline->AddTagToFrame(frame, tag);
            break;
        }

        case 4:                        // PlaceObject
        case GFxTag_PlaceObject2:
        case 70:                       // PlaceObject3
        {
            GFxPlaceObject2Tag* tag = new GFxPlaceObject2Tag;
            if (code == 4)
            {
                // PlaceObject (2012 0xa37910 / Unpack 0xa019d0): character id, depth, matrix and an
                // optional colour transform detected by the remaining tag length.
                tag->Pos.CharacterId = s->ReadU16();
                tag->Pos.Depth = s->ReadU16();
                s->ReadMatrix(&tag->Pos.Matrix);
                tag->Pos.PlaceFlags = GFxCharPosInfo::Place_HasCharacter
                                    | GFxCharPosInfo::Place_HasMatrix;
                if (s->Tell() < tagEnd)
                {
                    s->ReadCxformRgb(&tag->Pos.ColorTransform);
                    tag->Pos.PlaceFlags |= GFxCharPosInfo::Place_HasCxform;
                }
            }
            else
            {
                tag->Read(s, code);
            }
            timeline->AddTagToFrame(frame, tag);
            break;
        }

        case 5:                        // RemoveObject
        case GFxTag_RemoveObject2:
        {
            GFxRemoveObject2Tag* tag = new GFxRemoveObject2Tag;
            tag->Read(s, code);
            timeline->AddTagToFrame(frame, tag);
            break;
        }

        case GFxTag_DoAction:
        {
            GASDoActionTag* tag = new GASDoActionTag;
            tag->Read(s, tagEnd);
            timeline->AddTagToFrame(frame, tag);
            ++Stats.DoActions;
            Stats.ActionBytes += tagEnd - s->Tell();
            break;
        }

        case GFxTag_DoInitAction:
        {
            GASDoInitActionTag* tag = new GASDoInitActionTag;
            tag->Read(s, tagEnd);
            timeline->AddInitActionToFrame(frame, tag);
            ++Stats.DoInitActions;
            break;
        }

        case GFxTag_FrameLabel:
        {
            char label[96];
            s->ReadString(label, sizeof(label));
            timeline->AddFrameLabel(label, frame);
            break;
        }

        case GFxTag_DefineSprite:      // 2012 0xa35a00 -> GFxSpriteDef::Read 0x9fa170
        {
            unsigned int id = s->ReadU16();
            unsigned int frames = s->ReadU16();
            GFxSpriteDef* sprite = new GFxSpriteDef(this);
            sprite->BeginFrames(frames);
            AddCharacter(id, sprite);
            ++Stats.Sprites;
            ReadTags(s, sprite, tagEnd, depth + 1);
            break;
        }

        case GFxTag_ExportAssets:      // 2012 0xa35bf0
        {
            unsigned int count = s->ReadU16();
            for (unsigned int i = 0; i < count && s->Tell() < tagEnd; ++i)
            {
                unsigned int id = s->ReadU16();
                char name[160];
                s->ReadString(name, sizeof(name));
                AddExport(name, id);
            }
            break;
        }

        case GFxTag_ImportAssets:      // 2012 0xa385f0
        case GFxTag_ImportAssets2:
        {
            char url[192];
            s->ReadString(url, sizeof(url));
            if (code == GFxTag_ImportAssets2)
            {
                s->ReadU8();
                s->ReadU8();
            }
            unsigned int count = s->ReadU16();
            for (unsigned int i = 0; i < count && s->Tell() < tagEnd; ++i)
            {
                unsigned int id = s->ReadU16();
                char name[160];
                s->ReadString(name, sizeof(name));
                // An imported symbol is a placeholder until the fontlib movie is bound, which needs
                // the loader's file opener. It keeps its dictionary slot so a PlaceObject naming it
                // still produces a character rather than nothing.
                AddCharacter(id, new GFxPlaceholderDef(code, 0));
                ++Stats.Imports;
                ++Stats.Placeholders;
            }
            break;
        }

        default:
            if (GFxIsCharacterDefTag(code))
            {
                unsigned int id = s->ReadU16();
                AddCharacter(id, new GFxPlaceholderDef(code, tagEnd - s->Tell()));
                ++Stats.Placeholders;
                handled = false;
                NoteSkipped(code);
            }
            else
            {
                handled = false;
                NoteSkipped(code);
            }
            break;
        }

        if (handled)
            ++Stats.TagsHandled;
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

GFxMovieDefImpl::GFxMovieDefImpl(GFxMovieDataDef* dataDef) : pDataDef(dataDef)
{
    for (int i = 0; i < MaxStates; ++i)
        States[i] = 0;
}

GFxMovieDefImpl::~GFxMovieDefImpl()
{
    delete pDataDef;
}

unsigned int GFxMovieDefImpl::GetVersion() const { return pDataDef->GetVersion(); }
unsigned int GFxMovieDefImpl::GetLoadingFrame() const { return pDataDef->GetFrameCount(); }
float GFxMovieDefImpl::GetWidth() const { return pDataDef->GetWidth(); }
float GFxMovieDefImpl::GetHeight() const { return pDataDef->GetHeight(); }
unsigned int GFxMovieDefImpl::GetFrameCount() const { return pDataDef->GetFrameCount(); }
float GFxMovieDefImpl::GetFrameRate() const { return pDataDef->GetFrameRate(); }
GRect<float> GFxMovieDefImpl::GetFrameRect() const { return pDataDef->GetFrameRect(); }
unsigned int GFxMovieDefImpl::GetSWFFlags() const { return 0; }
const char* GFxMovieDefImpl::GetFileURL() const { return pDataDef->GetFileInfo().ExporterInfo.SWFName; }
GFxResource* GFxMovieDefImpl::GetMovieDataResource() const { return pDataDef; }
unsigned int GFxMovieDefImpl::GetResourceTypeCode() const { return GFxResource::RT_MovieDef; }

GFxResource* GFxMovieDefImpl::GetResource(const char* name) const
{
    return pDataDef->GetExportedCharacter(name);
}

GFxMovieView* GFxMovieDefImpl::CreateInstance(const GFxMovieDef::MemoryParams& params, bool bd)
{                                                                     // 2012 0xa1b960
    return new GFxMovieRoot(this);
}

GFxMovieView* GFxMovieDefImpl::CreateInstance(GFxMovieDef::MemoryContext* ctx, bool bd)
{                                                                     // 2012 0xa1ba00
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
    if ((unsigned int)t >= MaxStates || States[t] == 0)
        return 0;
    States[t]->AddRef();
    return States[t];
}

void GFxMovieDefImpl::GetStatesAddRef(GFxState** out, const GFxState::StateType* types,
                                      unsigned int count) const
{
    for (unsigned int i = 0; i < count; ++i)
        out[i] = GetStateAddRef(types[i]);
}
