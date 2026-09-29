// GFx 3.3 text layout: the line buffer of laid-out glyphs, the line cursor, the paragraph formatter
// and the document view a text field drives.
//
// This is where a string becomes positioned glyphs. The coordinate system is twips throughout, and
// the one conversion that matters is the glyph scale, which GFxParagraphFormatter::Format computes
// as `fontSizePx * 20.0f * (1/1024)` (0xa9d310: the literal 0.0009765625 is 1/1024) - so a glyph's
// 1024-EM advance times that scale is its advance in twips.
//
// The text rect and the view rect differ by exactly **40 twips (2 px) on each side**:
// GFxTextDocView::SetViewRect (0xa9a1e0) writes `ViewRect = TextRect inset by 40`. That is SWF's
// text-field gutter and it is why a 100x20 px field lays out into 96x16 px of usable space.
//
// Ported functions: GFxTextLineBuffer 21 (the ones layout needs), GFxLineCursor 8,
// GFxParagraphFormatter 9, GFxTextDocView 97 declared with the layout half implemented. Retail's
// GFxTextLineBuffer::Display (0xa45bf0, 8,522 bytes) and GFxTextDocView::Display (0xaa04f0) are the
// *drawing* half and are not here: they submit through GRenderer, which is package CC's, and the
// interface between us is GFxTextLineBufferIterate below.
//
// DEVIATION, stated once: retail's Line and GlyphEntry are hand-packed variable-length allocations -
// GFxTextLineBuffer::CalcLineSize (0xa442c0) is `((base + 8*glyphs + 7) & ~3) + 4*formats` with base
// 26 for a short line and 38 for a long one, GlyphEntry packs its index, advance and font size into
// 8 bytes with sign and fraction flags (SetFontSize 0xa43e40, SetAdvance 0xa44420), and Line stores
// its fields at two different sets of offsets depending on a high bit of its first word
// (GetHeight 0xa43f40, GetDescent 0xa98ff0). None of that is ABI-visible and the 2012 PDB carries no
// layout for it (agentBC.md 0), so the fields are plain here. The *semantics* are retail's: the
// 1/16 px font-size quantum, the signed advance, `descent = height - baselineOffset` and
// `leading = max(leading, 0)` are all reproduced.
#ifndef INC_GFXTEXTDOCVIEW_H
#define INC_GFXTEXTDOCVIEW_H

#include "GFx3.h"
#include "GFxText.h"
#include "GFxFont.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

class GFxTextDocView;

// ---------------------------------------------------------------------------------------------
class GFxTextLineBuffer
{
public:
    // One laid-out glyph. `Advance` is in twips and may be negative (retail carries the sign in a
    // flag bit); `FontSize` is in pixels and is quantised to 1/16 px below 256 px, which is exactly
    // what GlyphEntry::SetFontSize (0xa43e40) and GetFontSize (0x9bdc40) do.
    struct GlyphEntry
    {
        unsigned int  GlyphIndex;      // ~0u for a character the font has no glyph for
        int           Advance;
        unsigned int  FormatIndex;     // index into the line's Formats array
        unsigned char Flags;

        enum EntryFlags
        {
            EF_Newline    = 0x01,
            EF_Whitespace = 0x02,      // FinalizeLine's justify pass only widens these
            EF_Tab        = 0x04,
            EF_Underline  = 0x08
        };

        void  SetFontSize(float px);                             // 0xa43e40
        float GetFontSize() const;                               // 0x9bdc40
        void  SetAdvance(int twips) { Advance = twips; }          // 0xa44420

        unsigned short SizeQuantum;    // the 12-bit packed size
        bool           SizeIsFraction; // the 0x10 flag of the packed word
    };

    // One run of a line that shares a font, a colour and a size: retail's GFxFormatDataEntry.
    struct FormatEntry
    {
        GFxFontResource* pFont;
        GColor           Color;
        float            FontSize;     // pixels
        unsigned int     GlyphStart;
        bool             Underline;
    };

    struct Line
    {
        Line();

        unsigned int TextPos;          // index of the line's first character in the document
        unsigned int NumGlyphs;
        int OffsetX, OffsetY;          // twips, relative to the view rect's top left
        int Width, Height;             // twips
        int Leading;                   // twips, may be negative
        float BaselineOffset;          // twips: the ascent

        GArray<GlyphEntry>  Glyphs;
        GArray<FormatEntry> Formats;

        int   GetHeight() const { return Height; }                        // 0xa43f40
        float GetDescent() const { return (float)Height - BaselineOffset; }// 0xa98ff0
        int   GetNonNegLeading() const { return Leading > 0 ? Leading : 0; } // 0xa482b0
        bool  HasNewLine() const;                                          // 0xa44280
        void  SetBaseLineOffset(float v) { BaselineOffset = v; }            // 0xa43ee0
    };

    GFxTextLineBuffer();                                          // 0xa45aa0
    ~GFxTextLineBuffer();                                         // 0xa48260

    void Clear();
    Line* InsertNewLine();                                        // 0xa45b80 / 0xa99060
    unsigned int GetLineCount() const { return Lines.GetSize(); }
    Line* GetLine(unsigned int i) const                           // 0xa444b0
        { return i < Lines.GetSize() ? Lines[i] : 0; }
    void RemoveLines(unsigned int at, unsigned int count);         // 0xa48180
    int  GetMinLineHeight() const;                                 // 0xa44680

    // The number of bytes retail would allocate for a line of this shape. Kept because it is the one
    // function of the packed representation that is observable: GFxParagraphFormatter::InitParagraph
    // (0xa9c5b0) switches to a heap line above 0x400 bytes.
    static unsigned int CalcLineSize(unsigned int glyphs, unsigned int formats, bool longLine);

    unsigned int VScrollOffset;   // in lines
    unsigned int HScrollOffset;   // in twips

private:
    GArray<Line*> Lines;
};

// ---------------------------------------------------------------------------------------------
// GFxLineCursor: the running maxima of the line being built. TrackFontParams (0xa99640) is where the
// 960 / 64 ascent and descent defaults come from, and 960 + 64 = 1024 is the EM.
class GFxLineCursor
{
public:
    GFxLineCursor();                                              // 0xa9a470
    void Reset();                                                 // 0xa98eb0
    void TrackFontParams(GFxFontResource* font, float scale);      // 0xa99640

    float MaxAscent;    // twips
    float MaxDescent;   // twips
    float MaxLeading;   // twips
};

// ---------------------------------------------------------------------------------------------
// GFxParagraphFormatter: lays one paragraph out into as many lines as word wrap needs.
class GFxParagraphFormatter
{
public:
    GFxParagraphFormatter(GFxTextDocView* view);                   // 0xa9d080
    ~GFxParagraphFormatter();                                      // 0xa9d160

    void Format(const GFxTextParagraph& para);                     // 0xa9d310
    void InitParagraph(const GFxTextParagraph& para);              // 0xa9c5b0
    bool CheckWordWrap();                                          // 0xa9c1a0
    void FinalizeLine();                                           // 0xa9a940
    float GetActualFontSize(const GFxTextFormat* fmt) const;       // 0xa99740

    int GetMaxWidth() const { return MaxWidth; }
    int GetTotalHeight() const { return CurY; }
    unsigned int GetGlyphsWithoutShape() const { return GlyphsWithoutShape; }

private:
    void beginLine();
    void pushGlyph(const GFxTextLineBuffer::GlyphEntry& g);
    unsigned int formatIndexFor(GFxFontResource* font, GColor color, float sizePx, bool underline);

    GFxTextDocView* pView;
    const GFxTextParagraphFormat* pParaFormat;

    GFxTextLineBuffer::Line* pLine;
    GFxLineCursor Cursor;

    int LineWidth;          // twips accumulated on the current line
    int LeftOffset;         // twips: indent + block indent + left margin
    int RightMargin;        // twips
    int AvailWidth;         // twips: the view rect's width
    int CurY;               // twips
    int MaxWidth;           // twips
    unsigned int TextPos;   // document index of the current line's first character
    unsigned int WrapGlyph; // the last break opportunity on the current line, ~0u for none
    int WrapWidth;
    unsigned int WrapTextPos;
    unsigned int GlyphsWithoutShape;
    // FinalizeLine justifies only a line that is *not* the paragraph's last, which is the
    // `!IsLastLine` test of the retail body (0xa9a940); the flag is set by Format's closing call.
    bool bLastLine;
};

// ---------------------------------------------------------------------------------------------
class GFxTextDocView
{
public:
    enum ViewFlags
    {
        VF_WordWrap      = 0x0001,
        VF_Multiline     = 0x0002,
        VF_Password      = 0x0004,
        VF_ReadOnly      = 0x0008,
        VF_AutoSizeX     = 0x0010,
        VF_AutoSizeY     = 0x0020,
        VF_Selectable    = 0x0040,
        VF_Border        = 0x0080,
        VF_Html          = 0x0100,
        VF_UseDeviceFont = 0x0200,
        VF_NeedsFormat   = 0x0400
    };
    enum ViewVAlignment
    {
        VAlign_None   = 0,
        VAlign_Top    = 1,
        VAlign_Center = 2,
        VAlign_Bottom = 3
    };
    struct LineMetrics                 // 0xa9fc60's out-parameter, in this order
    {
        int Width, Height, Ascent, Descent, OffsetX, Leading;
    };
    struct FindFontInfo
    {
        const char*  FontList;
        unsigned int StyleFlags;
    };

    GFxTextDocView(GFxTextAllocator* alloc, GFxFontManager* fonts);  // 0xaa06b0
    ~GFxTextDocView();                                               // 0xa9c4e0

    GFxStyledText& GetStyledText() { return Text; }
    const GFxStyledText& GetStyledText() const { return Text; }
    GFxTextLineBuffer& GetLineBuffer() { return Lines; }
    GFxFontManager* GetFontManager() const { return pFontManager; }

    void SetText(const char* s);                                     // 0xa99190
    void SetText(const wchar_t* s, unsigned int len);                // 0xa991c0
    void SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len); // 0xa991f0
    void SetParagraphFormat(const GFxTextParagraphFormat& f, unsigned int at,
                            unsigned int len);                       // 0xa99220
    void SetDefaultTextAndParaFormat(const GFxTextFormat& f,
                                     const GFxTextParagraphFormat& p);// 0xa99920

    // `rect` is in twips and is the *text* rect; the view rect is it inset by 40 twips a side.
    void SetViewRect(const GRect<float>& rect);                      // 0xa9a1e0
    // DISHONORED(port): 2013 0xa95270 formats the document when the view is dirty before it
    // answers, exactly as GetTextWidth and GetTextHeight do, so an auto-sized field reports its
    // grown rect on the first read after the flag is set. (Was cited as 2012 0xa9f2c0.)
    GRect<float>& GetViewRect();                                     // 0xa95270
    const GRect<float>& GetTextRect() const { return TextRect; }

    void SetWordWrap()   { Flags |= VF_WordWrap;  Flags |= VF_NeedsFormat; }   // 0xa98f40
    void ClearWordWrap() { Flags &= ~(unsigned int)VF_WordWrap; Flags |= VF_NeedsFormat; } // 0xa98f60
    void SetAutoSizeX()  { Flags |= VF_AutoSizeX; Flags |= VF_NeedsFormat; }   // 0xa98f00
    void SetAutoSizeY()  { Flags |= VF_AutoSizeY; Flags |= VF_NeedsFormat; }   // 0xa98f20
    void SetMultiline(bool v)
        { if (v) Flags |= VF_Multiline; else Flags &= ~(unsigned int)VF_Multiline;
          Flags |= VF_NeedsFormat; }
    void SetVAlignment(ViewVAlignment a) { VAlignment = a; Flags |= VF_NeedsFormat; } // 0xa2cba0
    void SetFontScaleFactor(float f) { FontScaleFactor = f; Flags |= VF_NeedsFormat; } // 0xa997f0
    float GetFontScaleFactor() const { return FontScaleFactor; }     // 0xa2d010
    unsigned int GetFlags() const { return Flags; }
    void SetFlags(unsigned int f) { Flags = f | VF_NeedsFormat; }

    bool ForceReformat() { Flags |= VF_NeedsFormat; return true; }   // 0xa9f2a0
    void Format();                                                   // 0xa9e3a0

    float GetTextWidth();                                            // 0xa9f200
    float GetTextHeight();                                           // 0xa9f240
    unsigned int GetLinesCount();                                    // 0xa9f280
    bool GetLineMetrics(unsigned int line, LineMetrics* out);         // 0xa9fc60
    bool ContainsNonLeftAlignment() const;                           // 0xa999e0

    // 0xa9ccf0 - resolve a format's font list against the manager, falling back to the empty font so
    // layout never has a null font.
    GFxFontResource* FindFont(const GFxTextFormat* fmt);

    const GFxTextFormat& GetDefaultTextFormat() const
        { return Text.GetDefaultTextFormat(); }

    // Measured by Format() and what the harness reports.
    unsigned int GetGlyphsLaidOut() const { return GlyphsLaidOut; }
    unsigned int GetGlyphsWithoutShape() const { return GlyphsWithoutShape; }

private:
    friend class GFxParagraphFormatter;

    GFxTextAllocator* pAllocator;
    GFxFontManager*   pFontManager;
    GFxStyledText     Text;
    GFxTextLineBuffer Lines;

    GRect<float> TextRect;      // twips
    GRect<float> ViewRect;      // twips, TextRect inset by 40 a side
    unsigned int Flags;
    ViewVAlignment VAlignment;
    float FontScaleFactor;

    int TextWidthTwips;
    int TextHeightTwips;
    unsigned int GlyphsLaidOut;
    unsigned int GlyphsWithoutShape;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXTEXTDOCVIEW_H
