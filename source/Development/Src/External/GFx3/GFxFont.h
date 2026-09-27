// GFx 3.3 fonts: the abstract font, the DefineFont/2/3 reader, the font resource, the handle and the
// font manager.
//
// The units matter more than anything else in this file and they are measured, not assumed:
//
//   * a glyph's outline coordinates and its metrics live on an **EM square of 1024** units.
//     GFxFontData::GetAdvance (0xa52c00) returns 512.0 for the "no glyph" index and
//     GetGlyphHeight (0xa52ca0) returns 1024.0, and GFxLineCursor::TrackFontParams (0xa99640)
//     substitutes ascent 960 / descent 64 when a font declares none - 960 + 64 = 1024;
//   * DefineFont3 stores those coordinates at **20x**, so GFxSwfPathData::PathsIterator's
//     constructor (0xa3d110) multiplies by 0.05 and GFxFontData::Read (0xa587d0) scales every
//     layout metric by the same 0.05 when the tag is 75 (and by 1.0 for tags 10 and 48);
//   * the layout engine works in **twips** (1/20 px), so the glyph-to-layout scale is
//     `fontSizeInPixels * 20 / 1024` - which is literally what GFxParagraphFormatter::Format
//     computes (0xa9d310: `fontSize * 20.0 * 0.0009765625`);
//   * a per-glyph AdvanceEntry is 12 bytes - `float Advance; s16 Left; s16 Top; u16 Width;
//     u16 Height` - with the four bound fields in **twips of glyph units**, i.e. 20x the 1024-EM
//     values GetGlyphBounds hands back (0x9c54d0 divides each of them by 20).
//
// Ported functions, all with their 2012 rvas at the definition:
//   GFxFontData      Read 0xa587d0, ReadCodeTable 0xa583c0, GetGlyphShape 0xa52bd0,
//                    GetGlyphBounds 0x9c54d0, GetAdvance 0xa52c00, GetGlyphHeight 0xa52ca0,
//                    GetKerningAdjustment 0x9c5b00, HasVectorOrRasterGlyphs 0xa58270
//   GFxFontResource  ctor 0xa54510, GetLowerCaseTop 0xa4ba50, GetUpperCaseTop 0xa4ba80,
//                    calcLowerUpperTop 0xa54700, calcTopBound 0xa54680, GetResourceTypeCode 0xa52d80
//   GFxFontHandle    ctor 0x9f4190, operator== 0xa90d60
//   GFxFontManager   CreateFontHandle 0xa52520, CreateFontHandleFromName 0xa52240,
//                    FindOrCreateHandle 0xa51850, GetEmptyFont 0xa508a0, CleanCache 0xa50d50
//   GFxFontLib       AddFontsFrom 0x9c4dc0, FindFont 0x9c47f0
//
// No offset here is asserted: the 2012 PDB carries no layout for any of these types (agentBC.md 0).
#ifndef INC_GFXFONT_H
#define INC_GFXFONT_H

#include "GFx3.h"
#include "GFxShape.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

class GFxStream;
class GFxMovieDataDef;
class GFxFontResource;
class GFxFontManager;

// ---------------------------------------------------------------------------------------------
// GFxFont: the abstract font. Retail's own GFxFont has only two non-inline functions
// (GetCharRanges 0x9c56a0 and the deleting destructor) because the rest is pure virtual and the
// implementations are GFxFontData, GFxFontDataCompactedSwf/Gfx and GFxTextureFont.
class GFxFont : public GRefCountBase<GFxFont, 2>
{
public:
    // Measured out of GFxFontData::Read (0xa587d0), one bit at a time, at the site where the tag's
    // flag sets it. FF_DeviceFont at 0x40 is the one that is *inferred* rather than read from a
    // writer: GFxEditTextCharacter::GetInitialFormats (0xa27860) skips creating a font handle when
    // `(fontFlags & 0x40) != 0`, which is what a device (system) font means, but no body in the cook
    // sets it.
    enum FontFlags
    {
        FF_Italic              = 0x0001,   // DefineFont2/3 FontFlagsItalic
        FF_Bold                = 0x0002,   // DefineFont2/3 FontFlagsBold
        FF_DeviceFont          = 0x0040,   // inferred, see above
        FF_CodePageANSI        = 0x0100,
        FF_CodePageShiftJIS    = 0x0200,
        FF_CodePageMask        = 0x0300,
        FF_GlyphShapesStripped = 0x1000,   // the offset table was empty: glyphs were exported out
        FF_HasLayout           = 0x2000,   // DefineFont2/3 FontFlagsHasLayout
        FF_WideCodes           = 0x4000,   // DefineFont2/3 FontFlagsWideCodes
        FF_SmallText           = 0x8000    // DefineFont2/3 FontFlagsSmallText
    };

    GFxFont() : Flags(0), Ascent(0.0f), Descent(0.0f), Leading(0.0f) { Name[0] = 0; }
    virtual ~GFxFont() {}

    virtual GFxShapeBase* GetGlyphShape(unsigned int glyphIndex, unsigned int hintedSize) = 0;
    virtual GRect<float>& GetGlyphBounds(unsigned int glyphIndex, GRect<float>* out) const = 0;
    virtual float GetAdvance(unsigned int glyphIndex) const = 0;
    virtual float GetGlyphWidth(unsigned int glyphIndex) const = 0;
    virtual float GetGlyphHeight(unsigned int glyphIndex) const = 0;
    virtual float GetKerningAdjustment(unsigned int left, unsigned int right) const = 0;
    virtual int   GetGlyphIndex(unsigned short code) const = 0;
    virtual int   GetCharValue(unsigned int glyphIndex) const = 0;
    virtual unsigned int GetGlyphShapeCount() const = 0;
    virtual bool  HasVectorOrRasterGlyphs() const = 0;
    virtual bool  IsHintedVectorGlyph(unsigned int glyphIndex, unsigned int size) const
    {
        // 0x9c5cd0 - `return false` in every implementation in this build.
        (void)glyphIndex; (void)size;
        return false;
    }

    const char* GetName() const { return Name; }
    unsigned int GetFontFlags() const { return Flags; }
    bool IsBold() const { return (Flags & FF_Bold) != 0; }
    bool IsItalic() const { return (Flags & FF_Italic) != 0; }
    bool HasLayout() const { return (Flags & FF_HasLayout) != 0; }
    bool AreGlyphShapesStripped() const { return (Flags & FF_GlyphShapesStripped) != 0; }

    // In 1024-EM glyph units. TrackFontParams (0xa99640) substitutes 960 / 64 when these are zero.
    float GetAscent() const { return Ascent; }
    float GetDescent() const { return Descent; }
    float GetLeading() const { return Leading; }

protected:
    char  Name[128];
    unsigned int Flags;
    float Ascent, Descent, Leading;
};

// ---------------------------------------------------------------------------------------------
// GFxFontData: a font read straight out of a DefineFont, DefineFont2 or DefineFont3 tag, keeping its
// glyphs as GFxConstShapeNoStyles outlines. This is what every font in the Dishonored cook is:
// `$NormalFont` (ChaletComprime-CologneEighty, 188 glyphs) and `$TitleFont` (Emerge BF, 188 glyphs),
// both DefineFont3, with no GFx font-texture tag (1002/1005) anywhere (agentBB.md 3.4).
class GFxFontData : public GFxFont
{
public:
    // 12 bytes, exactly retail's: GetAdvance (0xa52c00) reads the float at +0, GetGlyphBounds
    // (0x9c54d0) the four 16-bit fields at +4, +6, +8 and +10, all four in twips of glyph units.
    struct AdvanceEntry
    {
        float          Advance;
        short          Left;
        short          Top;
        unsigned short Width;
        unsigned short Height;
    };

    struct KerningPair
    {
        unsigned short Left;
        unsigned short Right;
        float          Adjustment;
    };

    GFxFontData();                                                      // 0xa581f0 / 0xa58290
    virtual ~GFxFontData();                                             // 0xa58340

    // The tag reader. `tagType` is 10, 48 or 75; `tagEnd` is the stream position one past the tag's
    // last byte. 0xa587d0.
    bool Read(GFxStream* s, unsigned int tagType, unsigned int tagEnd);
    void ReadCodeTable(GFxStream* s, unsigned int glyphCount);           // 0xa583c0

    virtual GFxShapeBase* GetGlyphShape(unsigned int glyphIndex, unsigned int hintedSize);
    virtual GRect<float>& GetGlyphBounds(unsigned int glyphIndex, GRect<float>* out) const;
    virtual float GetAdvance(unsigned int glyphIndex) const;
    virtual float GetGlyphWidth(unsigned int glyphIndex) const;
    virtual float GetGlyphHeight(unsigned int glyphIndex) const;
    virtual float GetKerningAdjustment(unsigned int left, unsigned int right) const;
    virtual int   GetGlyphIndex(unsigned short code) const;
    virtual int   GetCharValue(unsigned int glyphIndex) const;
    virtual unsigned int GetGlyphShapeCount() const { return Glyphs.GetSize(); }
    virtual bool  HasVectorOrRasterGlyphs() const;                       // 0xa58270

    unsigned int GetKerningPairCount() const { return Kerning.GetSize(); }
    // What the tag's u16 kerning count said, against what was actually read: they differ only when
    // the table ran past the tag, which is the case retail logs as "Corrupted file ... kerning table
    // of the font '%s' is longer than tagLength".
    unsigned int GetKerningPairsDeclared() const { return KerningDeclared; }
    unsigned int GetCodeTableSize() const { return CodeTable.GetSize(); }

    // Where Read stopped, so a caller can check it landed on the tag's last byte. The harness prints
    // it for every DefineFont tag in the cook.
    unsigned int GetReadEndPos() const { return ReadEndPos; }

    void SetName(const char* name);

private:
    struct CodeEntry { unsigned short Code; unsigned short GlyphIndex; };

    GArray<GPtr<GFxShapeBase> > Glyphs;
    GArray<AdvanceEntry>        Advances;
    GArray<KerningPair>         Kerning;
    GArray<CodeEntry>           CodeTable;
    // The two lookups retail keeps as hash sets (a GHashSet<KerningPair,float> at +60 and a
    // GHashSet<u16,u16> for the code table). Both matter: $TitleFont carries 5,000 kerning pairs and
    // 188 code entries, so a linear scan would cost up to 5,000 compares per character pair of every
    // line laid out. Open addressing over a power-of-two table, built once at the end of Read.
    GArray<int> KernHash;
    GArray<int> CodeHash;
    unsigned int ReadEndPos;
    unsigned int KerningDeclared;

    void buildLookups();
};

// ---------------------------------------------------------------------------------------------
// GFxFontResource: the font as the resource library sees it - a GFxFont plus the two auto-hinting
// reference heights the formatter and the underline drawing ask for.
class GFxFontResource : public GFxResource
{
public:
    GFxFontResource(GFxFont* font);                                      // 0xa54510
    virtual ~GFxFontResource();                                          // 0xa52d00

    virtual unsigned int GetResourceTypeCode() const { return RT_Font; } // 0xa52d80

    GFxFont* GetFont() const { return pFont; }
    const char* GetName() const { return pFont ? pFont->GetName() : ""; }

    // The name the *movie* exports this font under. fontlib exports `$NormalFont` and `$TitleFont`
    // while the DefineFont3 face names are "ChaletComprime-CologneEighty" and "Emerge BF", and a UI
    // movie's ImportAssets2 asks for the export name; retail's FindOrCreateHandle (0xa51850) tries
    // both, so both are kept here.
    const char* GetExportName() const { return ExportName; }
    void SetExportName(const char* name);

    // Both in 1024-EM glyph units and both cached. 0xa4ba50 / 0xa4ba80 / 0xa54700 / 0xa54680.
    unsigned short GetLowerCaseTop();
    unsigned short GetUpperCaseTop();

    // The font interface, forwarded, so the formatter and the glyph cache never hold a GFxFont*.
    GFxShapeBase* GetGlyphShape(unsigned int glyphIndex, unsigned int hintedSize)
        { return pFont ? pFont->GetGlyphShape(glyphIndex, hintedSize) : 0; }
    float GetAdvance(unsigned int glyphIndex) const
        { return pFont ? pFont->GetAdvance(glyphIndex) : 512.0f; }
    float GetGlyphHeight(unsigned int glyphIndex) const
        { return pFont ? pFont->GetGlyphHeight(glyphIndex) : 1024.0f; }
    float GetKerningAdjustment(unsigned int l, unsigned int r) const
        { return pFont ? pFont->GetKerningAdjustment(l, r) : 0.0f; }
    int GetGlyphIndex(unsigned short code) const
        { return pFont ? pFont->GetGlyphIndex(code) : -1; }
    GRect<float>& GetGlyphBounds(unsigned int glyphIndex, GRect<float>* out) const;
    float GetAscent() const { return pFont ? pFont->GetAscent() : 0.0f; }
    float GetDescent() const { return pFont ? pFont->GetDescent() : 0.0f; }
    float GetLeading() const { return pFont ? pFont->GetLeading() : 0.0f; }
    unsigned int GetFontFlags() const { return pFont ? pFont->GetFontFlags() : 0; }
    unsigned int GetGlyphShapeCount() const { return pFont ? pFont->GetGlyphShapeCount() : 0; }

private:
    void calcLowerUpperTop();                                            // 0xa54700
    unsigned short calcTopBound(unsigned short charCode);                // 0xa54680

    GPtr<GFxFont>  pFont;
    char           ExportName[160];
    unsigned short LowerCaseTop;
    unsigned short UpperCaseTop;
};

// ---------------------------------------------------------------------------------------------
// GFxFontHandle: the (manager, resource, name, flags) tuple a GFxTextFormat holds. Retail's is 36
// bytes and refcounted by hand rather than through GRefCountBase, which is why the formatter's
// decompiles are full of `if ((*(DWORD*)h)-- == 1) { ~GFxFontHandle(h); Free(h); }`.
class GFxFontHandle : public GRefCountBase<GFxFontHandle, 326>
{
public:
    GFxFontHandle(GFxFontManager* mgr, GFxFontResource* res, const char* name);  // 0x9f4190

    bool operator==(const GFxFontHandle& o) const;                       // 0xa90d60

    GFxFontResource* GetFontResource() const { return pFont; }
    const char* GetName() const { return Name; }
    float GetScale() const { return Scale; }

    GFxFontManager*  pManager;
    GFxFontResource* pFont;       // borrowed: the manager's cache owns it
    char  Name[128];
    float Scale;                  // GetActualFontSize (0xa99740) multiplies by this; 1.0 normally
};

// ---------------------------------------------------------------------------------------------
// GFxFontManager: name -> resource, with the handles it has already made cached. Retail's
// FindOrCreateHandle (0xa51850) is 2,537 bytes of a search that walks the movie's own exports, then
// the font library, then the font provider, then the font map; that whole chain is not here (the
// font *library* is, the provider and the map are not: section "Remaining" of agentCB.md).
class GFxFontManager
{
public:
    GFxFontManager();                                                    // 0xa52490
    ~GFxFontManager();                                                   // 0xa517c0

    // Register a font read out of a movie. This is what GFxFontLib::AddFontsFrom (0x9c4dc0) does for
    // every exported $-prefixed font symbol of a fontlib movie.
    void AddFont(GFxFontResource* res);

    // 0xa52520 / 0xa52240 / 0xa51850. `flags` is the FF_Bold / FF_Italic pair the format asks for;
    // retail's search prefers an exact style match and falls back to the plain face, which is what
    // the two-pass loop below reproduces.
    GFxFontHandle* CreateFontHandle(const char* name, unsigned int flags);
    GFxFontHandle* GetEmptyFont();                                       // 0xa508a0

    GFxFontResource* FindFontResource(const char* name, unsigned int flags) const;

    unsigned int GetFontCount() const { return Fonts.GetSize(); }
    GFxFontResource* GetFontByIndex(unsigned int i) const { return Fonts[i]; }

private:
    GArray<GPtr<GFxFontResource> > Fonts;
    GPtr<GFxFontHandle>            pEmptyFont;
};

// ---------------------------------------------------------------------------------------------
// Reading the fonts out of a cooked movie. Retail reaches DefineFont3 through the tag-loader table
// (GFx_DefineFontLoader 2012 0xa357d0) and the fontlib through import binding (GFx_ImportLoader
// 0xa385f0), neither of which is in this tree yet - the tag walk of GFxPlayerData.cpp records every
// font tag as a GFxPlaceholderDef and the imports as placeholders too (package CD's work).
//
// GFxFontLoadFromPayload walks a payload's tag stream itself, reads every DefineFont/2/3 into a
// GFxFontData, matches it to the ExportAssets name of its character id, and registers it with the
// manager. That is what makes the harness independent of the loader work, and it is the entry point
// CD's GFx_DefineFontLoader should call once it exists.
unsigned int GFxFontLoadFromPayload(GFxFontManager* mgr, const unsigned char* data,
                                    unsigned int size, char* errBuf, unsigned int errBufSize);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXFONT_H
