// GFx 3.3 text fields: the DefineEditText character definition and the GFxEditTextCharacter instance
// that owns a document view, lays it out and produces glyphs.
//
// GFxEditTextCharacterDef::Read (2012 0xa26190, 1,424 bytes) is the SWF DefineEditText record and
// every one of its bits is mapped at the site it is read, in retail's order. The definition's flag
// word is read out of that body one bit at a time:
//   0x0001 WordWrap   0x0002 Multiline  0x0004 Password  0x0008 ReadOnly   0x0010 AutoSize
//   0x0020 Selectable (the NoSelect bit *inverted*)      0x0040 Border
//   0x0080 Html       0x0100 UseDeviceFont (the UseOutlines bit inverted)  0x0200 HasLayout
// and the SWF Align byte is remapped to GFxTextParagraphFormat::AlignType in
// GFxEditTextCharacter::GetInitialFormats (0xa27860): SWF 0/1/2/3 (left, right, centre, justify) go
// to 0x001 / 0x201 / 0x601 / 0x401, i.e. AlignType Left, Right, Center, Justify.
//
// Of GFxEditTextCharacter's 125 retail functions this ports the construction, the initial formats,
// the text setters, the layout drive and the AS2 member surface the cook actually uses. The table of
// what remains is in agentCB.md; the big absences are the editor kit (GFxTextEditorKit, 31 fns), the
// IME composition string (GFxTextCompositionString, 11), the URL zones and the HTML image
// substitution, and GetMember/SetMember's full 4,401 + 6,232 bytes of property surface.
#ifndef INC_GFXTEXTFIELD_H
#define INC_GFXTEXTFIELD_H

#include "GFx3.h"
#include "GFxPlayer.h"
#include "GFxText.h"
#include "GFxTextDocView.h"
#include "GFxGlyphCache.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

// ---------------------------------------------------------------------------------------------
// GFxTextFieldDesc: everything a text field needs out of its DefineEditText tag.
//
// OWNERSHIP NOTE, and it is deliberate. Package CD owns the tag loaders and the character
// definitions, and its GFxCharacterDefs.h declares GFxEditTextCharacterDef with exactly these
// fields. So this package does *not* declare a competing definition class: it declares the POD a
// field is constructed from, plus its own port of the DefineEditText record for the harness to use,
// and CD's GFxEditTextCharacterDef::CreateCharacterInstance (2012 0xa32df0) fills one of these in
// twelve lines. The adapter is spelled out in agentCB.md's hand-over to CD.
struct GFxTextFieldDesc
{
    enum DefFlags
    {
        // The same bits, at the same values, as CD's GFxEditTextCharacterDef::Flags: both are read
        // out of the word the retail body writes at +80 (0xa26190).
        ETF_WordWrap      = 0x0001,
        ETF_Multiline     = 0x0002,
        ETF_Password      = 0x0004,
        ETF_ReadOnly      = 0x0008,
        ETF_AutoSize      = 0x0010,
        ETF_Selectable    = 0x0020,   // the stream bit is NoSelect and is stored inverted
        ETF_Border        = 0x0040,
        ETF_Html          = 0x0080,
        ETF_UseDeviceFont = 0x0100,   // the stream bit is UseOutlines and is stored inverted
        ETF_HasLayout     = 0x0200,
        ETF_UseFlashType  = 0x0400
    };

    GFxTextFieldDesc();
    void InitEmptyTextDef();                                          // 0xa26720
    bool HasFlag(unsigned int f) const { return (Flags & f) != 0; }

    GRect<int> TextRectTwips;     // left, top, right, bottom
    unsigned int FontId;
    // DISHONORED(port): the name the font id resolves to in the movie that owns the field - the
    // DefineFont's own name when it is local, the ImportAssets2 symbol when it is imported. Empty
    // when the movie could not be consulted, and the field then falls back as it did before.
    char           FontName[96];
    float FontHeightTwips;
    GColor TextColor;
    unsigned int MaxLength;
    float LeftMarginTwips;
    float RightMarginTwips;
    float IndentTwips;
    float LeadingTwips;
    unsigned int Flags;
    unsigned int Align;           // the raw SWF byte: 0 left, 1 right, 2 centre, 3 justify
    char VariableName[128];
    char InitialText[512];
};

// This package's port of the DefineEditText record, 2012 0xa26190. CD ports the same record into its
// own definition class; both are the same decompile and the duplication is one function, which is
// cheaper than either package depending on the other's header while both are in flight.
bool GFxTextFieldReadDesc(GFxStream* s, unsigned int tagType, unsigned int tagEnd,
                          GFxTextFieldDesc* out);

// -gfxuinohtmlface: resolve a field's font from the DefineEditText font id only, as the tree did
// before the HTML face was read. It exists so the before and the after of that change can be
// measured with one binary; the census line reports which route every field took.
extern bool GFxTextNoHtmlFace;


// ---------------------------------------------------------------------------------------------
// GFxTextFilter: a whole SWF filter list folded into the handful of numbers a text field needs.
//
// DISHONORED(layout): 36 bytes including the eight-byte refcounted base retail derives it from
// (GRefCountBaseNTS); the fields below are the 28 bytes after it and every offset is read out of the
// ctor (2013 0xa24420), LoadFilterDesc (0xa89910), UpdateShadowOffset (0xa22f30) and
// GFxTextFieldParam::LoadFromTextFilter (0xa809c0):
//   +8  BlurX           fixed 4.4, so 16 is one pixel
//   +9  BlurY           fixed 4.4
//   +10 BlurStrength    fixed 4.4, default 16
//   +11 ShadowFlags     the filter descriptor's own high nibble; default 0x80
//   +12 ShadowBlurX     fixed 4.4, default 64
//   +13 ShadowBlurY     fixed 4.4, default 64
//   +14 ShadowStrength  fixed 4.4, default 16
//   +15 ShadowAlpha     the shadow colour's alpha byte, default 255
//   +16 GlowSize        max(ShadowBlurX, ShadowBlurY) when a glow followed a shadow; default 0
//   +18 ShadowAngle     tenths of a degree, default 450
//   +20 ShadowDistance  twips, default 80
//   +22 ShadowOffsetX   twips, default 57
//   +24 ShadowOffsetY   twips, default 57
//   +28 ShadowColor     default 0
//   +32 GlowColor       default 0
// Retail's refcounted base is left out here: this tree holds one of these by value on the character,
// because nothing else ever shares it.
struct GFxTextFilter
{
    unsigned char  BlurX;
    unsigned char  BlurY;
    unsigned char  BlurStrength;
    unsigned char  ShadowFlags;
    unsigned char  ShadowBlurX;
    unsigned char  ShadowBlurY;
    unsigned char  ShadowStrength;
    unsigned char  ShadowAlpha;
    unsigned char  GlowSize;
    short          ShadowAngle;
    short          ShadowDistance;
    short          ShadowOffsetX;
    short          ShadowOffsetY;
    GColor         ShadowColor;
    GColor         GlowColor;

    GFxTextFilter();                                              // 2013 0xa24420
    static unsigned char FloatToFixed44(float v);                 // 2013 0xa24480
    void UpdateShadowOffset();                                    // 2013 0xa22f30
    void LoadFilterDesc(const GFxFilterDesc& desc);               // 2013 0xa89910

    // Retail's own predicate, read out of GFxTextFieldParam::LoadFromTextFilter (0xa809c0): the
    // shadow block is copied only when bit 0 of ShadowFlags is clear, and a shadow with no colour at
    // all draws nothing.
    bool HasShadow() const
    {
        return (ShadowFlags & 0x01) == 0 && ShadowAlpha != 0 && ShadowColor.Raw != 0;
    }
    bool HasBlur() const { return BlurX != 0 || BlurY != 0; }
};

// ---------------------------------------------------------------------------------------------
class GFxEditTextCharacter : public GFxASCharacter
{
public:
    GFxEditTextCharacter(const GFxTextFieldDesc& desc, GFxASCharacter* parent, GFxResourceId id,
                         GFxMovieRoot* root);                        // 0xa2c470
    virtual ~GFxEditTextCharacter();                                 // 0xa32980

    virtual const char* GetCharacterTypeName() const { return "EditText"; }
    virtual GRect<float> GetBoundsTwips(const GMatrix2D& m) const;   // 0xa2ed60
    virtual GASObjectType GetObjectType() const { return Object_TextField; } // 0xa2cfe0 -> 13
    virtual void AdvanceFrame(bool bAdvance, float framePos);         // 0xa2e250
    // DISHONORED(port): 0xa2ed90 -> GFxTextDocView::Display 0xaa04f0 -> GFxTextLineBuffer::Display
    // 0xa45bf0. This is ProduceGlyphs' traversal with the DrawBitmaps submission put back.
    virtual void Display(GFxDisplayContext& ctx);
    virtual void OnEventLoad();                                      // 0xa30cd0
    // DISHONORED(port): 2013 0xa275b0. Every descriptor of the list is folded into one GFxTextFilter,
    // which is what makes the text's drop shadow a second glyph batch rather than a blur of the
    // subtree.
    virtual void SetFilters(const GFxFilterDesc* filters, unsigned int count);
    // DISHONORED(port): 2013 0xa23d60 / 0xa23990. A text field's hit area is its formatted view
    // rectangle, and a field that is neither selectable nor carrying a mouse handler passes the
    // question up to the nearest ancestor clip that acts as a button - which is what keeps a menu
    // entry's label from stealing the entry's own rollover.
    virtual bool PointTestLocal(const GPoint<float>& pt, unsigned char hitTestMask) const;
    virtual GFxASCharacter* GetTopMostMouseEntity(const GPoint<float>& pt,
                                                 const TopMostParams& params);

    const GFxTextFilter& GetTextFilter() const { return Filter; }

    virtual bool GetMember(GASEnvironment* env, const GASString& name, GASValue* val);   // 0xa2f9f0
    virtual bool SetMember(GASEnvironment* env, const GASString& name, const GASValue& val,
                           const GASPropFlags& flags);                                  // 0xa33450

    void GetInitialFormats(GFxTextFormat* fmt, GFxTextParagraphFormat* pfmt);            // 0xa27860
    void SetInitialFormatsAsDefault();                                                   // 0xa28460
    void SetTextValue(const char* s, bool html, bool notifyVariable);                    // 0xa32e50
    void SetText(const char* s, bool html) { SetTextValue(s, html, true); }              // 0xa2ce10
    const char* GetTextValue() const { return TextValue; }                               // 0xa2cf40

    GFxTextDocView& GetDocView() { return Doc; }
    const GFxTextFieldDesc& GetDesc() const { return Desc; }

    // Ask the glyph cache for every laid-out glyph of every line and report how many produced pixels.
    // This is the measurement half of retail's GFxTextLineBuffer::Display (0xa45bf0) with the
    // GRenderer submission - package CC's half - left out; the interface between us is exactly this
    // walk, so CC can replace the body of the loop with a mesh submission.
    struct GlyphOutput
    {
        unsigned int Lines;
        unsigned int Glyphs;          // laid-out glyph entries
        unsigned int Rasterized;      // entries the cache produced pixels for
        unsigned int Blank;           // entries with no outline (spaces)
        unsigned int Missing;         // entries the font has no glyph for
        unsigned int CoveredPixels;
    };
    void ProduceGlyphs(GFxGlyphRasterCache* cache, GlyphOutput* out);

    bool IsDirty() const { return bDirty; }
    void SetDirtyFlag() { bDirty = true; }                           // 0xa24790

private:
    GFxTextFieldDesc         Desc;
    GFxTextAllocator         Allocator;
    GFxTextDocView           Doc;
    char  TextValue[512];
    bool  bDirty;
    GFxTextFilter            Filter;
};

// ---------------------------------------------------------------------------------------------
// The font registry every text field resolves against.
//
// DEVIATION, documented: retail hangs a GFxFontManager off each GFxMovieDefImpl
// (GFxFontManager::GFxFontManager 0xa52490 takes the def impl) and a UI movie reaches the fontlib's
// fonts through import binding (GFx_ImportLoader 2012 0xa385f0), which this tree does not have yet -
// package CD is adding it, and `ImportAssets2` symbols are placeholders today (agentBC.md 6.2). Until
// then the registry is process-wide, which is a close approximation of what gfxfontlib *is*: one
// shared movie whose two exported fonts every UI movie imports. When CD's import binding lands this
// should become per-movie-def and the accessor can stay as the fallback.
GFxFontManager* GFxTextGetFontManager();
void            GFxTextResetFontManager();

// Create a text field from a raw DefineEditText body. This is what a tag loader does
// (GFx_DefineEditTextLoader 2012 0xa272a0 -> GFxEditTextCharacterDef::Read -> the def's
// CreateCharacterInstance 0xa32df0), factored so the harness can do it without a loader table.
GFxEditTextCharacter* GFxTextFieldCreateFromTag(GFxStream* s, unsigned int tagType,
                                                unsigned int tagEnd, GFxASCharacter* parent,
                                                GFxResourceId id, GFxMovieRoot* root);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXTEXTFIELD_H
