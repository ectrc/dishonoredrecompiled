// Scaleform GFx 3.3.89 - the tag loaders and the two loader tables. Package CD, retail object
// GFxTagLoaders.obj.
//
// Section 1 is the table, and it is a dump rather than a reconstruction: GFxLoaderImpl::GetTagLoader
// (2012 0x9badf0) indexes GFx_SWF_TagLoaderTable for codes 0..0x54 and a second table for 1000..1009,
// and both were read out of the database by address. Every row below carries the retail function it
// came from. The rows that are null in retail are null here, which is what makes "unhandled" an
// answer rather than a gap: tags 0 (End), 24 (Protect) and 1000 (ExporterInfo) have a loader that
// does nothing - 0xa26ea0, a shared `retn` - and every code with no row is a tag GFx itself ignores.
//
// Section 2 is the loaders. Each one is named after its retail function and carries its 2012 rva; the
// bodies read exactly the fields the retail body reads, in the retail order, including the two places
// where GFx departs from the published SWF layout (ImportAssets2's count before its reserved word,
// and the line-style count's unconditional u16 promotion).
// DISHONORED(port): 2013 rvas beside each function; see agentCD.md.
#include "GFxCharacterDefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------
// GFxLoadProcess

void GFxLoadProcess::ReadRgbaTag(GColor* out, unsigned int tagType)    // 2012 0xa22460
{
    GFxReadRgbaTag(pStream, out, tagType);
}

void GFxLoadProcess::AddCharacter(unsigned int id, GFxCharacterDef* def)
{
    pDataDef->AddCharacter(id, def);
}

GFxCharacterDef* GFxLoadProcess::GetCharacterDef(unsigned int id) const
{
    return pDataDef->GetCharacterDefById(id);
}

void GFxLoadProcess::AddExecuteTag(GASExecuteTag* tag)                 // 2012 0x9e2810
{
    pTimeline->AddTagToFrame(Frame, tag);
}

void GFxLoadProcess::AddInitActionTag(GASExecuteTag* tag)
{
    pTimeline->AddInitActionToFrame(Frame, tag);
}

// ---------------------------------------------------------------------------------------------
// Section 2: the loaders

// 2012 0xa26ea0 - the shared do-nothing loader tags 0, 24 and 1000 point at. Retail really does
// register it rather than leaving those rows null, which is why a `default: unhandled` count of zero
// is reachable at all.
static void GFx_NullLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{
    (void)p; (void)info;
}

static void GFx_DefineShapeLoader(GFxLoadProcess* p, const GFxTagInfo& info)   // 2012 0xa36850
{
    const unsigned int id = p->ReadU16();
    GFxShapeCharacterDef* def = new GFxShapeCharacterDef(info.TagType);
    def->Read(p->GetStream(), info.TagType, info.GetEndPos());
    p->AddCharacter(id, def);
}

static void GFx_DefineShapeMorphLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa35710
{
    const unsigned int id = p->ReadU16();
    GFxMorphCharacterDef* def = new GFxMorphCharacterDef(info.TagType);
    def->Read(p->GetStream(), info.TagType, info.GetEndPos());
    p->AddCharacter(id, def);
}

static void GFx_DefineEditTextLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa272a0
{
    const unsigned int id = p->ReadU16();
    GFxEditTextCharacterDef* def = new GFxEditTextCharacterDef;
    def->Read(p->GetStream(), info.TagType);
    p->AddCharacter(id, def);
}

static void GFx_DefineTextLoader(GFxLoadProcess* p, const GFxTagInfo& info)     // 2012 0xa8d500
{
    const unsigned int id = p->ReadU16();
    GFxStaticTextCharacterDef* def = new GFxStaticTextCharacterDef;
    def->Read(p->GetStream(), info.TagType, info.GetEndPos());
    p->AddCharacter(id, def);
}

static void GFx_ButtonCharacterLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa35ac0
{
    const unsigned int id = p->ReadU16();
    GFxButtonCharacterDef* def = new GFxButtonCharacterDef;
    def->Read(p->GetStream(), info.TagType, info.GetEndPos());
    p->AddCharacter(id, def);
}

static void GFx_DefineFontLoader(GFxLoadProcess* p, const GFxTagInfo& info)     // 2012 0xa357d0
{
    const unsigned int id = p->ReadU16();
    GFxFontCharacterDef* def = new GFxFontCharacterDef(info.TagType);
    def->Read(p->GetStream(), info.TagType, info.GetEndPos());
    p->AddCharacter(id, def);
}

static void GFx_DefineFontInfoLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa35960
{
    // DefineFontInfo names a font that is already in the dictionary; it must not replace it, which
    // the previous walk did by treating every id-bearing tag as a definition.
    const unsigned int id = p->ReadU16();
    GFxCharacterDef* def = p->GetCharacterDef(id);
    if (def != 0 && def->GetResourceTypeCode() == GFxResource::RT_Font)
        ((GFxFontCharacterDef*)def)->ReadFontInfo(p->GetStream(), info.TagType, info.GetEndPos());
}

// 2012 0xa35fc0 / 0xa36210 / 0xa364d0. The three external-image loaders differ only in which fields
// precede the two names: the plain form has none, the gradient form carries a gradient id, and the
// "2" form carries a u32 bitmap format and the target size. Agent BB established that every bitmap
// in this cook is stripped to one of these, so the reference is the whole payload.
static void GFx_DefineExternalImageLoaderCommon(GFxLoadProcess* p, const GFxTagInfo& info,
                                                bool bWideId)
{
    // The only difference between the plain loader (0xa35fc0) and the "2" loader (0xa36210) is the
    // width of the first field: the plain form reads four u16s, the "2" form reads a u32 and then
    // three u16s. That u32 is a GFxResourceId and the retail body masks it with 0x9FFFF, which is
    // GFxResourceId's index mask plus the DynFontImage type bits - so an id can be 0x90001 as well
    // as 1, and the mask is what makes both land in the same dictionary.
    //
    // Getting this wrong is silent: reading a u16 id out of the "2" form yields the *format* word,
    // which is 13 for every image in the cook, so all 55 of the menu's images landed in one slot and
    // every bitmap fill resolved to nothing. The harness reports bitmap fills that resolve to an
    // image definition for exactly this reason.
    GFxImageCharacterDef* def = new GFxImageCharacterDef(info.TagType);
    const unsigned int id = bWideId ? (p->ReadU32() & 0x9FFFFu) : (unsigned int)p->ReadU16();
    def->Format       = p->ReadU16();
    def->TargetWidth  = p->ReadU16();
    def->TargetHeight = p->ReadU16();
    p->GetStream()->ReadStringWithLength(def->ExportName, sizeof(def->ExportName));
    p->GetStream()->ReadStringWithLength(def->FileName, sizeof(def->FileName));
    p->AddCharacter(id, def);
}

static void GFx_DefineExternalImageLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa35fc0
    GFx_DefineExternalImageLoaderCommon(p, info, false);
}

static void GFx_DefineExternalImageLoader2(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa36210
    GFx_DefineExternalImageLoaderCommon(p, info, true);
}

static void GFx_DefineExternalGradientImageLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa364d0
    GFx_DefineExternalImageLoaderCommon(p, info, false);
}

static void GFx_DefineSubImageLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa369d0
{
    // Six u16s and nothing else: the sub-image id, the atlas it comes from, and the rectangle in
    // pixels that GFxSubImageResource's constructor (0xa22750) takes as a GRect<int>.
    GFxImageCharacterDef* def = new GFxImageCharacterDef(info.TagType);
    const unsigned int id = p->ReadU16();
    def->bIsSubImage = true;
    def->BaseImageId = p->ReadU16();
    const int x0 = (int)p->ReadU16();
    const int y0 = (int)p->ReadU16();
    const int x1 = (int)p->ReadU16();
    const int y1 = (int)p->ReadU16();
    def->SubRect = GRect<int>(x0, y0, x1, y1);
    def->TargetWidth  = (unsigned int)(x1 - x0);
    def->TargetHeight = (unsigned int)(y1 - y0);
    p->AddCharacter(id, def);
}

// 2012 0xa35420 / 0xa35560 / 0xa37080 / 0xa37260. No asset in the retail cook has an embedded bitmap
// tag - the count is zero in all 22 payloads - so these record the reference and the format and
// decode nothing. That is a stub and it is marked as one, but it is a stub for a code path the game
// never takes.
static void GFx_DefineBitsLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{
    GFxImageCharacterDef* def = new GFxImageCharacterDef(info.TagType);
    const unsigned int id = p->ReadU16();
    def->Format = info.TagType;
    // DISHONORED(port): the JPEG and zlib decoders of GFx_DefineBitsJpeg*Loader and
    // GFx_DefineBitsLossless2Loader are not ported; no payload in the cook reaches them.
    p->AddCharacter(id, def);
}

static void GFx_JpegTablesLoader(GFxLoadProcess* p, const GFxTagInfo& info)     // 2012 0xa353b0
{
    (void)p; (void)info;
}

static void GFx_Scale9GridLoader(GFxLoadProcess* p, const GFxTagInfo& info)     // 2012 0xa36680
{
    (void)info;
    const unsigned int id = p->ReadU16();
    int r[4];
    p->GetStream()->ReadRect(r);
    const GRect<float> grid((float)r[0], (float)r[2], (float)r[1], (float)r[3]);
    // Retail rejects a degenerate grid with a warning before it ever looks the character up, and the
    // warning prints the width in pixels, which is where the /20 comes from.
    if (grid.Right <= grid.Left || grid.Bottom <= grid.Top)
        return;
    GFxCharacterDef* def = p->GetCharacterDef(id);
    if (def == 0)
        return;
    // The retail body tests the resource type's class byte for 0x84 (sprite) or 0x81 (button); a
    // shape carries its grid through the sprite that contains it, which is why a shape is not a case.
    if (def->GetResourceTypeCode() == GFxResource::RT_SpriteDef)
        ((GFxSpriteDef*)def)->SetScale9Grid(grid);
    else if (def->GetResourceTypeCode() == GFxResource::RT_ButtonDef)
        ((GFxButtonCharacterDef*)def)->SetScale9Grid(grid);
}

static void GFx_CSMTextSettings(GFxLoadProcess* p, const GFxTagInfo& info)      // 2012 0xa26ca0
{
    (void)info;
    const unsigned int id = p->ReadU16();
    p->GetStream()->ReadUBits(3);                                      // UseFlashType + GridFit
    p->GetStream()->Skip(4);                                           // thickness
    p->GetStream()->Skip(4);                                           // sharpness
    GFxCharacterDef* def = p->GetCharacterDef(id);
    if (def == 0)
        return;
    if (def->GetResourceTypeCode() == GFxResource::RT_EditTextDef)
        ((GFxEditTextCharacterDef*)def)->Flags |= GFxEditTextCharacterDef::Flag_UseFlashType;
    else if (def->GetResourceTypeCode() == GFxResource::RT_TextDef)
        ((GFxStaticTextCharacterDef*)def)->bUseFlashType = true;
}

static void GFx_SpriteLoader(GFxLoadProcess* p, const GFxTagInfo& info)         // 2012 0xa35a00
{
    const unsigned int id = p->ReadU16();
    const unsigned int frames = p->ReadU16();
    GFxSpriteDef* sprite = new GFxSpriteDef(p->GetDataDef());
    sprite->BeginFrames(frames);
    p->AddCharacter(id, sprite);
    p->GetDataDef()->ReadSpriteTags(p->GetStream(), sprite, info.GetEndPos());
}

static void GFx_ExportLoader(GFxLoadProcess* p, const GFxTagInfo& info)         // 2012 0xa35bf0
{
    const unsigned int count = p->ReadU16();
    for (unsigned int i = 0; i < count && p->Tell() < info.GetEndPos(); ++i)
    {
        const unsigned int id = p->ReadU16();
        char name[160];
        p->GetStream()->ReadString(name, sizeof(name));
        p->GetDataDef()->AddExport(name, id);
    }
}

static void GFx_ImportLoader(GFxLoadProcess* p, const GFxTagInfo& info)         // 2012 0xa385f0
{
    // The retail order is the URL, then the symbol count as a u16, then - for ImportAssets2 only -
    // one more u16. The published SWF layout puts two reserved bytes before the count; GFx reads
    // them after it. Both readings consume the same four bytes and every ImportAssets2 tag in this
    // cook has 01 00 01 00 there, so they agree on this data; retail's order is what is ported.
    char url[192];
    p->GetStream()->ReadString(url, sizeof(url));
    const unsigned int count = p->ReadU16();
    if (info.TagType == 71)
        p->ReadU16();
    for (unsigned int i = 0; i < count && p->Tell() < info.GetEndPos(); ++i)
    {
        const unsigned int id = p->ReadU16();
        char symbol[160];
        p->GetStream()->ReadString(symbol, sizeof(symbol));
        p->GetDataDef()->AddImport(url, symbol, id);
    }
    // The init-action entry retail leaves behind for this tag (GFxInitImportActions): when the
    // importing sprite reaches this frame, the imported movie's own init actions run on it.
    p->AddInitActionTag(new GASImportInitActionsTag(p->GetDataDef(), url));
}

static void GFx_FrameLabelLoader(GFxLoadProcess* p, const GFxTagInfo& info)     // 2012 0xa351b0
{
    (void)info;
    char label[96];
    p->GetStream()->ReadString(label, sizeof(label));
    p->GetTimeline()->AddFrameLabel(label, p->Frame);
}

static void GFx_SetBackgroundColorLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa36cd0
    (void)info;
    GFxSetBackgroundColorTag* tag = new GFxSetBackgroundColorTag;
    tag->Read(p->GetStream());
    p->AddExecuteTag(tag);
}

static void GFx_PlaceObjectLoader(GFxLoadProcess* p, const GFxTagInfo& info)    // 2012 0xa37910
{
    // PlaceObject (tag 4): id, depth, matrix, and a colour transform only if the tag has bytes left,
    // which is how GFxPlaceObject::Unpack (0xa019d0) decides.
    GFxPlaceObject2Tag* tag = new GFxPlaceObject2Tag;
    tag->Pos.CharacterId = p->ReadU16();
    tag->Pos.Depth = p->ReadU16();
    p->GetStream()->ReadMatrix(&tag->Pos.Matrix);
    tag->Pos.PlaceFlags = GFxCharPosInfo::Place_HasCharacter | GFxCharPosInfo::Place_HasMatrix;
    if (p->Tell() < info.GetEndPos())
    {
        p->GetStream()->ReadCxformRgb(&tag->Pos.ColorTransform);
        tag->Pos.PlaceFlags |= GFxCharPosInfo::Place_HasCxform;
    }
    p->AddExecuteTag(tag);
}

static void GFx_PlaceObject2Loader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                       // 2012 0xa379b0 and 0xa37ac0 for tag 70
    GFxPlaceObject2Tag* tag = new GFxPlaceObject2Tag;
    tag->Read(p->GetStream(), info.TagType);
    p->AddExecuteTag(tag);
}

static void GFx_RemoveObjectLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                       // 2012 0xa37b90 and 0xa37c00 for tag 28
    GFxRemoveObject2Tag* tag = new GFxRemoveObject2Tag;
    tag->Read(p->GetStream(), info.TagType);
    p->AddExecuteTag(tag);
}

static void GFx_DoActionLoader(GFxLoadProcess* p, const GFxTagInfo& info)       // 2012 0x9e43a0
{
    GASDoActionTag* tag = new GASDoActionTag;
    p->GetDataDef()->NoteActionBytes(info.GetEndPos() - p->Tell(), false);
    tag->Read(p->GetStream(), info.GetEndPos());
    p->AddExecuteTag(tag);
}

static void GFx_DoInitActionLoader(GFxLoadProcess* p, const GFxTagInfo& info)   // 2012 0x9e4420
{
    GASDoInitActionTag* tag = new GASDoInitActionTag;
    p->GetDataDef()->NoteActionBytes(info.GetEndPos() - p->Tell(), true);
    tag->Read(p->GetStream(), info.GetEndPos());
    p->AddInitActionTag(tag);
}

static void GFx_ShowFrameLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{
    // Retail has no loader row for ShowFrame: GFxLoadProcess::CommitFrameTags (0xa23640) is called by
    // the tag walk itself when the code is 1. The row exists here so the walk stays a pure dispatch.
    (void)info;
    ++p->Frame;
}

static void GFx_FileAttributesLoader(GFxLoadProcess* p, const GFxTagInfo& info) // 2012 0xa352f0
{
    (void)info;
    p->GetDataDef()->SetSWFFlags(p->ReadU32());
}

static void GFx_MetadataLoader(GFxLoadProcess* p, const GFxTagInfo& info)       // 2012 0xa35d90
{
    (void)p; (void)info;
}

static void GFx_DebugIDLoader(GFxLoadProcess* p, const GFxTagInfo& info)        // 2012 0xa35240
{
    (void)p; (void)info;
}

static void GFx_SetTabIndexLoader(GFxLoadProcess* p, const GFxTagInfo& info)    // 2012 0xa35b80
{
    (void)info;
    p->ReadU16();                                                      // depth
    p->ReadU16();                                                      // tab index
}

static void GFx_DefineGradientMapLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa36660
    (void)p; (void)info;
}

static void GFx_FontTextureInfoLoader(GFxLoadProcess* p, const GFxTagInfo& info)
{                                                                      // 2012 0xa37d10
    // GFx's font-texture tag. Agent BB proved no payload in the cook carries one - both game fonts
    // are outlines - so this is recorded and skipped rather than parsed into a texture atlas.
    (void)p; (void)info;
}

// ---------------------------------------------------------------------------------------------
// Section 1: the tables, read out of GFxLoaderImpl::GetTagLoader (2012 0x9badf0)

GFxTagLoaderFn GFxGetTagLoader(unsigned int tagCode)
{
    switch (tagCode)
    {
    case 0:    return GFx_NullLoader;                 // 0xa26ea0  End
    case 1:    return GFx_ShowFrameLoader;            // the walk's own, see above
    case 2:    return GFx_DefineShapeLoader;          // 0xa36850
    case 4:    return GFx_PlaceObjectLoader;          // 0xa37910
    case 5:    return GFx_RemoveObjectLoader;         // 0xa37b90
    case 6:    return GFx_DefineBitsLoader;           // 0xa35420  DefineBitsJpeg
    case 7:    return GFx_ButtonCharacterLoader;      // 0xa35ac0
    case 8:    return GFx_JpegTablesLoader;           // 0xa353b0
    case 9:    return GFx_SetBackgroundColorLoader;   // 0xa36cd0
    case 10:   return GFx_DefineFontLoader;           // 0xa357d0
    case 11:   return GFx_DefineTextLoader;           // 0xa8d500
    case 12:   return GFx_DoActionLoader;             // 0x9e43a0
    case 13:   return GFx_DefineFontInfoLoader;       // 0xa35960
    case 20:   return GFx_DefineBitsLoader;           // 0xa37260  DefineBitsLossless
    case 21:   return GFx_DefineBitsLoader;           // 0xa35560  DefineBitsJpeg2
    case 22:   return GFx_DefineShapeLoader;          // 0xa36850  DefineShape2
    case 24:   return GFx_NullLoader;                 // 0xa26ea0  Protect
    case 26:   return GFx_PlaceObject2Loader;         // 0xa379b0
    case 28:   return GFx_RemoveObjectLoader;         // 0xa37c00
    case 32:   return GFx_DefineShapeLoader;          // 0xa36850  DefineShape3
    case 33:   return GFx_DefineTextLoader;           // 0xa8d500  DefineText2
    case 34:   return GFx_ButtonCharacterLoader;      // 0xa35ac0  DefineButton2
    case 35:   return GFx_DefineBitsLoader;           // 0xa37080  DefineBitsJpeg3
    case 36:   return GFx_DefineBitsLoader;           // 0xa37260  DefineBitsLossless2
    case 37:   return GFx_DefineEditTextLoader;       // 0xa272a0
    case 39:   return GFx_SpriteLoader;               // 0xa35a00
    case 43:   return GFx_FrameLabelLoader;           // 0xa351b0
    case 46:   return GFx_DefineShapeMorphLoader;     // 0xa35710
    case 48:   return GFx_DefineFontLoader;           // 0xa357d0  DefineFont2
    case 56:   return GFx_ExportLoader;               // 0xa35bf0
    case 57:   return GFx_ImportLoader;               // 0xa385f0
    case 59:   return GFx_DoInitActionLoader;         // 0x9e4420
    case 62:   return GFx_DefineFontInfoLoader;       // 0xa35960  DefineFontInfo2
    case 63:   return GFx_DebugIDLoader;              // 0xa35240
    case 66:   return GFx_SetTabIndexLoader;          // 0xa35b80
    case 69:   return GFx_FileAttributesLoader;       // 0xa352f0
    case 70:   return GFx_PlaceObject2Loader;         // 0xa37ac0  PlaceObject3
    case 71:   return GFx_ImportLoader;               // 0xa385f0  ImportAssets2
    case 74:   return GFx_CSMTextSettings;            // 0xa26ca0
    case 75:   return GFx_DefineFontLoader;           // 0xa357d0  DefineFont3
    case 77:   return GFx_MetadataLoader;             // 0xa35d90
    case 78:   return GFx_Scale9GridLoader;           // 0xa36680
    case 83:   return GFx_DefineShapeLoader;          // 0xa36850  DefineShape4
    case 84:   return GFx_DefineShapeMorphLoader;     // 0xa35710  DefineMorphShape2

    // The second table: GFx's own codes 1000..1009. 1006 and 1007 are null in retail.
    case 1000: return GFx_NullLoader;                 // 0xa26ea0  ExporterInfo, read from the header
    case 1001: return GFx_DefineExternalImageLoader;  // 0xa35fc0
    case 1002: return GFx_FontTextureInfoLoader;      // 0xa37d10
    case 1003: return GFx_DefineExternalGradientImageLoader; // 0xa364d0
    case 1004: return GFx_DefineGradientMapLoader;    // 0xa36660
    case 1005: return GFx_DefineFontLoader;           // 0xa357d0  the compacted font
    case 1008: return GFx_DefineSubImageLoader;       // 0xa369d0
    case 1009: return GFx_DefineExternalImageLoader2; // 0xa36210
    default:   return 0;
    }
}
