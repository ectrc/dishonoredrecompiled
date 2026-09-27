// Scaleform GFx 3.3.89 - the cooked container format, parsed for real.
//
// DISHONORED(written): the answer to the one assumption resources/docs/gfx_decision.md rested on
// and did not check (its section 4, row 3). Measured over all 22 SwfMovie payloads in the retail
// cook (build/agentBB/movies.json, extracted by build/agentBB/extract_gfx.py):
//
//   * every payload begins with the ASCII bytes "GFX" - gfxexport output, never Flash "FWS"/"CWS"
//     and never the zlib-compressed Scaleform "CFX". The u32 at +4 is always exactly the payload
//     length, which is what "uncompressed" means here;
//   * the first tag is always at offset 21 and is always tag 1000, GFx_ExporterInfo, whose
//     ExportFlags are 0 in every file (so EXF_GlyphTexturesExported and EXF_GlyphsStripped are
//     clear) and whose FileFormatType is 13 = GFxFileConstants::File_TGA;
//   * there is not one DefineBits* tag in the whole game. Bitmaps are stripped into UE3 Texture2D
//     objects in the same package and named back by tag 1009, GFx_DefineExternalImage2, one per
//     bitmap: the 1009 count equals the package's Texture2D export count exactly in every
//     single-movie package. Where bPackTextures is on, tag 1008 GFx_DefineSubImage gives each
//     character's rectangle inside the atlas;
//   * fonts are NOT pre-packed to textures, which is where gfx_decision.md guessed wrong: the
//     DisFonts packages carry real DefineFont3 (tag 75) glyph outlines - gfxfontlib exports
//     $NormalFont (ChaletComprime-CologneEighty, 188 glyphs) and $TitleFont (Emerge BF, 188
//     glyphs) - and every UI movie imports them with ImportAssets2 (tag 71). No 1002/1005
//     font-texture tag exists anywhere in the cook;
//   * the content is AS2: 669 DoInitAction (tag 59) carrying __Packages.* class registrations, zero
//     DoABC and zero SymbolClass.
//
// The header is the SWF header with a different signature, so the reader below is a SWF reader:
//   +0  char[3]  "GFX"
//   +3  u8       version (8 or 10 - the authoring Flash version)
//   +4  u32      file length (== the payload length for GFX)
//   +8  RECT     5-bit nbits then 4 signed nbits fields, the frame rect in twips
//   ..  u16      frame rate, 8.8 fixed
//   ..  u16      frame count
//   ..           the tag stream: u16 (code << 6) | len, len == 0x3F meaning a following u32 length
#ifndef INC_GFX3_GFXGFXFILE_H
#define INC_GFX3_GFXGFXFILE_H

#include "GTypes.h"

#pragma pack(push, 8)

// The tag codes this parser names. 0..91 are SWF's; 1000+ are Scaleform's own, and their presence
// is the decisive proof that the asset is gfxexport output.
enum GFxTagType
{
    GFxTag_End                  = 0,
    GFxTag_ShowFrame            = 1,
    GFxTag_DefineShape          = 2,
    GFxTag_DefineBits           = 6,
    GFxTag_JPEGTables           = 8,
    GFxTag_SetBackgroundColor   = 9,
    GFxTag_DefineFont           = 10,
    GFxTag_DoAction             = 12,
    GFxTag_DefineBitsLossless   = 20,
    GFxTag_DefineBitsJPEG2      = 21,
    GFxTag_DefineShape2         = 22,
    GFxTag_PlaceObject2         = 26,
    GFxTag_RemoveObject2        = 28,
    GFxTag_DefineShape3         = 32,
    GFxTag_DefineButton2        = 34,
    GFxTag_DefineBitsJPEG3      = 35,
    GFxTag_DefineBitsLossless2  = 36,
    GFxTag_DefineEditText       = 37,
    GFxTag_DefineSprite         = 39,
    GFxTag_FrameLabel           = 43,
    GFxTag_DefineMorphShape     = 46,
    GFxTag_DefineFont2          = 48,
    GFxTag_ExportAssets         = 56,
    GFxTag_ImportAssets         = 57,
    GFxTag_DoInitAction         = 59,
    GFxTag_FileAttributes       = 69,
    GFxTag_ImportAssets2        = 71,
    GFxTag_CSMTextSettings      = 74,
    GFxTag_DefineFont3          = 75,
    GFxTag_SymbolClass          = 76,
    GFxTag_Metadata             = 77,
    GFxTag_DefineScalingGrid    = 78,
    GFxTag_DoABC                = 82,
    GFxTag_DefineShape4         = 83,
    GFxTag_DefineBitsJPEG4      = 90,

    GFxTag_GFxExporterInfo        = 1000,
    GFxTag_GFxDefineSubImage      = 1008,
    GFxTag_GFxDefineExternalImage2 = 1009
};

// GFx_ExporterInfo (tag 1000), always the first tag.
class GFxGfxExporterInfo
{
public:
    unsigned short Version;          // 878 (0x36E) in every retail asset
    unsigned int   ExportFlags;      // 0 in every retail asset
    unsigned short FileFormatType;   // 13 = GFxFileConstants::File_TGA
    char           Prefix[64];       // length-prefixed, empty in every retail asset
    char           SWFName[128];     // length-prefixed, the movie's own name

    GFxGfxExporterInfo() : Version(0), ExportFlags(0), FileFormatType(0)
    {
        Prefix[0] = 0;
        SWFName[0] = 0;
    }
};

// GFx_DefineExternalImage2 (tag 1009): one per bitmap stripped into an engine texture.
class GFxGfxExternalImage
{
public:
    unsigned short ImageIndex;
    unsigned short Flags;            // 9 for the atlas entries of a bPackTextures movie, else 0
    unsigned short FileFormatType;   // 13 = File_TGA
    unsigned short SrcWidth;         // the source TGA size; the cooked Texture2D is this rounded
    unsigned short SrcHeight;        // up to a multiple of 4 (TextureRescale FlashTextureScale_Mult4)
    char           ExportName[128];
    char           FileName[128];
};

// GFx_DefineSubImage (tag 1008): a character's rectangle inside a packed atlas. Always 12 bytes.
class GFxGfxSubImage
{
public:
    unsigned short CharacterId;
    unsigned short ImageIndex;
    unsigned short X0, Y0, X1, Y1;
};

// ExportAssets (tag 56) entry: the movie's symbol table, which is what the player binds
// AttachMovie() and the AS2 __Packages.* class registrations against.
class GFxGfxExportedSymbol
{
public:
    unsigned short CharacterId;
    char           Name[160];
};

// ImportAssets2 (tag 71) entry. The URLs inside a GFX file still say ".swf" and mix '\' and '/'
// separators, sometimes within one file, because gfxexport preserves the authoring path.
class GFxGfxImportedSymbol
{
public:
    unsigned short CharacterId;
    char           Url[192];
    char           Name[160];
};

class GFxGfxTagCount
{
public:
    unsigned int Code;
    unsigned int Count;
};

// The result of parsing one payload: the header, the symbol tables and the tag histogram.
// Fixed-capacity on purpose - the parser allocates nothing - which makes this about 140 KB, so
// heap-allocate it rather than putting it on a worker thread's stack. The caps are sized from the
// retail cook with room to spare: the largest movie has 116 exports (UI_HUD), 55 external images
// (MainMenu), 184 sub-images (UI_HUD) and 10 imports (MainMenu).
class GFxGfxFileInfo
{
public:
    enum { MaxExports = 256, MaxImports = 64, MaxImages = 256, MaxSubImages = 512, MaxTagCodes = 96 };

    // header
    char           Signature[4];
    GUByte         Version;
    unsigned int   DeclaredLength;
    unsigned int   PayloadLength;
    int            FrameRectTwips[4];      // left, right, top, bottom, as the RECT is ordered
    float          FrameWidthPixels;
    float          FrameHeightPixels;
    float          FrameRate;
    unsigned int   FrameCount;
    unsigned int   FirstTagOffset;

    bool           IsGfxExport;            // signature "GFX"
    bool           IsCompressed;           // signature "CFX" or "CWS"/"ZWS"
    bool           IsFlashSwf;             // signature "FWS"/"CWS"/"ZWS"

    // tags
    unsigned int   TagCount;
    GFxGfxTagCount TagCodes[MaxTagCodes];
    unsigned int   TagCodeCount;
    unsigned int   NonStandardTagCount;
    unsigned int   EmbeddedBitmapTagCount; // DefineBits* - 0 in every retail asset
    unsigned int   GlyphTagCount;          // DefineFont* - non-zero only in the DisFonts movies

    bool               HasExporterInfo;
    GFxGfxExporterInfo ExporterInfo;

    unsigned int          ExportCount;
    GFxGfxExportedSymbol  Exports[MaxExports];
    unsigned int          ImportCount;
    GFxGfxImportedSymbol  Imports[MaxImports];
    unsigned int          ImageCount;
    GFxGfxExternalImage   Images[MaxImages];
    unsigned int          SubImageCount;
    GFxGfxSubImage        SubImages[MaxSubImages];

    bool           Truncated;              // the tag walk ran off the end
    bool           ConsumedExactly;        // the End tag landed on the last byte
    char           Error[128];

    GFxGfxFileInfo() { Reset(); }
    void Reset();

    unsigned int CountOfTag(unsigned int code) const;
};

// Parse `size` bytes of a cooked SwfMovie::RawData payload. Returns false with Info.Error set when
// the buffer is not a GFX/SWF container at all; a truncated tag stream returns true with
// Info.Truncated set, because the header and everything up to the break are still usable.
bool GFxGfxParseFile(const void* data, unsigned int size, GFxGfxFileInfo& info);

#pragma pack(pop)
#endif // INC_GFX3_GFXGFXFILE_H
