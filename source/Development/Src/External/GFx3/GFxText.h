// GFx 3.3 styled text: the character and paragraph formats, the paragraph with its format runs, and
// the styled-text document a text field holds.
//
// Everything here is in **twips** (1/20 px) unless the name says otherwise, which is retail's own
// convention and is measured at each setter:
//   GFxTextFormat::SetFontSize      (0xa70dc0) stores `(int)(size * 20)` in a u16, clamped at 3276.8
//   GFxTextFormat::SetLetterSpacing (0xa70da0) stores `(int)(spacing * 20)` in an s16
//   GFxTextParagraphFormat's margins/indent/leading are in **pixels**, not twips:
//     GFxEditTextCharacter::GetInitialFormats (0xa27860) divides the DefineEditText fields by 20 on
//     the way in and GFxParagraphFormatter::FinalizeLine (0xa9a940) multiplies the leading by 20 on
//     the way out. That asymmetry is retail's and it is reproduced.
//
// The alignment enum is pinned case by case out of the predicates rather than guessed, because the
// SWF order and the GFx order differ and getting it wrong silently centres left-aligned text:
//   GFxTextParagraphFormat::SetAlignment  0xa242b0 writes `(align << 9)` into bits 9..10 and sets
//                                         bit 0 ("alignment present")
//   IsLeftAlignment   0xaae080  bits 9..10 == 0
//   IsRightAlignment  0xa990a0  bits 9..10 == 1   (0x200)
//   IsCenterAlignment 0xa990d0  bits 9..10 == 3   (0x600)
//   FinalizeLine      0xa9a940  treats 2 (0x400) as justify
// so AlignType is Left = 0, Right = 1, Justify = 2, Center = 3 - and the DefineEditText Align byte,
// which is SWF's order (0 left, 1 right, 2 center, 3 justify), is remapped in GetInitialFormats.
//
// Ported functions: GFxTextFormat 33, GFxTextParagraphFormat 24, GFxTextParagraph 17,
// GFxStyledText 41, GFxTextAllocator 7. What is *not* here is named in agentCB.md: the HTML parser
// (GFxStyledText::ParseHtmlImpl 0xaa7630, 6,268 bytes), the CSS style manager and the IME classes.
#ifndef INC_GFXTEXT_H
#define INC_GFXTEXT_H

#include "GFx3.h"
#include "GFxFont.h"

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

class GFxTextAllocator;

// ---------------------------------------------------------------------------------------------
// GFxTextFormat: the character format. Every field has a "present" bit, because a format is built by
// Merge (0xa924a0) and Intersection (0xa928f0) out of the default, the field's own and any <font>
// tag's, and only the present fields of the more specific one win.
class GFxTextFormat
{
public:
    enum PresentFlags
    {
        PF_LetterSpacing = 0x0002,
        PF_FontSize      = 0x0008,
        PF_Bold          = 0x0010,
        PF_Italic        = 0x0020,
        PF_Underline     = 0x0040,
        PF_Kerning       = 0x0080,
        PF_Url           = 0x0100,
        PF_Alpha         = 0x0400,
        PF_FontHandle    = 0x0800,
        PF_Color         = 0x0001,
        PF_FontList      = 0x2000
    };
    enum StyleFlags
    {
        SF_Bold      = 0x01,
        SF_Italic    = 0x02,
        SF_Underline = 0x04,
        SF_Kerning   = 0x08
    };

    GFxTextFormat();                                          // 0xa07980
    GFxTextFormat(const GFxTextFormat& o);                    // 0xa91f80
    ~GFxTextFormat();                                         // 0xa079e0
    GFxTextFormat& operator=(const GFxTextFormat& o);         // 0xa27c30
    bool operator==(const GFxTextFormat& o) const;            // 0xa90e50

    void InitByDefaultValues();                               // 0xa92ce0

    void SetFontList(const char* list);                       // 0xa92190
    void SetFontName(const char* name) { SetFontList(name); } // 0xa92380
    const char* GetFontList() const { return FontList; }

    void SetFontSize(float px);                               // 0xa70dc0
    float GetFontSize() const { return (float)FontSizeTwips / 20.0f; }
    unsigned short GetFontSizeTwips() const { return FontSizeTwips; }

    void SetLetterSpacing(float px);                          // 0xa70da0
    float GetLetterSpacing() const { return (float)LetterSpacingTwips / 20.0f; }

    void SetBold(bool v);                                     // 0xa92400
    void SetItalic(bool v);                                   // 0xa92450
    void SetUnderline(bool v);                                // 0xa90be0
    void SetKerning(bool v);                                  // 0xa90c00
    bool IsBold() const { return (Style & SF_Bold) != 0; }
    bool IsItalic() const { return (Style & SF_Italic) != 0; }
    bool IsUnderline() const { return (Style & SF_Underline) != 0; }
    bool IsKerning() const { return (Style & SF_Kerning) != 0; }

    void SetColor(GColor c) { Color = c; Present |= PF_Color; }
    GColor GetColor() const { return Color; }
    void SetAlpha(unsigned char a);                           // 0xa70cc0

    void SetFontHandle(GFxFontHandle* h);                      // 0xa923b0
    void ClearFontHandle();                                    // 0xa92090
    GFxFontHandle* GetFontHandle() const { return pFontHandle.GetPtr(); }

    void SetUrl(const char* url);                              // 0xa90b20
    void ClearUrl();                                           // 0xa70cf0
    bool IsUrlSet() const { return (Present & PF_Url) != 0 && Url[0] != 0; }  // 0xa2c950
    const char* GetUrl() const { return Url; }

    bool IsFontSame(const GFxTextFormat& o) const;             // 0xa91070
    GFxTextFormat Merge(const GFxTextFormat& o) const;         // 0xa924a0
    GFxTextFormat Intersection(const GFxTextFormat& o) const;  // 0xa928f0
    unsigned int Hash() const;                                 // 0xa91220

    unsigned int GetFontStyleFlags() const;   // the FF_Bold/FF_Italic pair for the font search

private:
    char   FontList[128];
    char   Url[192];
    GColor Color;
    unsigned short FontSizeTwips;
    short  LetterSpacingTwips;
    unsigned short Present;
    unsigned char  Style;
    GPtr<GFxFontHandle> pFontHandle;
};

// ---------------------------------------------------------------------------------------------
// GFxTextParagraphFormat: the block format. Margins, indent and leading are in pixels.
class GFxTextParagraphFormat
{
public:
    enum AlignType
    {
        Align_Left    = 0,
        Align_Right   = 1,
        Align_Justify = 2,
        Align_Center  = 3
    };
    enum PresentFlags
    {
        PF_Alignment  = 0x0001,
        PF_Indent     = 0x0004,
        PF_Leading    = 0x0008,
        PF_LeftMargin = 0x0010,
        PF_RightMargin= 0x0020,
        PF_TabStops   = 0x0040,
        PF_Bullet     = 0x0080,
        PF_BlockIndent= 0x0002,
        PF_AlignMask  = 0x0600,
        PF_AlignShift = 9,
        PF_BulletOn   = 0x8000
    };

    GFxTextParagraphFormat();                                  // 0xa260b0
    void InitByDefaultValues();                                // 0xa91350

    void SetAlignment(AlignType a);                            // 0xa242b0
    AlignType GetAlignment() const
        { return (AlignType)((Present & PF_AlignMask) >> PF_AlignShift); }
    bool IsLeftAlignment() const;                              // 0xaae080
    bool IsRightAlignment() const;                             // 0xa990a0
    bool IsCenterAlignment() const;                            // 0xa990d0
    bool IsJustifyAlignment() const;

    void SetIndent(short px)      { Indent = px; Present |= PF_Indent; }
    void SetBlockIndent(short px) { BlockIndent = px; Present |= PF_BlockIndent; }
    void SetLeading(short px)     { Leading = px; Present |= PF_Leading; }
    void SetLeftMargin(unsigned short px)  { LeftMargin = px; Present |= PF_LeftMargin; }
    void SetRightMargin(unsigned short px) { RightMargin = px; Present |= PF_RightMargin; }
    void SetBullet(bool v);                                    // 0xa70d10
    bool IsBullet() const;                                     // 0xa70d40

    short GetIndent() const { return Indent; }
    short GetBlockIndent() const { return BlockIndent; }
    short GetLeading() const { return (Present & PF_Leading) ? Leading : 0; }
    bool  HasLeading() const { return (Present & PF_Leading) != 0; }
    unsigned short GetLeftMargin() const { return LeftMargin; }
    unsigned short GetRightMargin() const { return RightMargin; }

    bool operator==(const GFxTextParagraphFormat& o) const;    // 0xa90f70
    GFxTextParagraphFormat Merge(const GFxTextParagraphFormat& o) const;        // 0xa91a10
    GFxTextParagraphFormat Intersection(const GFxTextParagraphFormat& o) const; // 0xa91b80
    unsigned int Hash() const;                                 // 0xa914b0

private:
    short Indent;
    short BlockIndent;
    short Leading;
    unsigned short LeftMargin;
    unsigned short RightMargin;
    unsigned short Present;
};

// ---------------------------------------------------------------------------------------------
// GFxTextAllocator: retail interns formats so a paragraph's format runs are pointer comparisons.
// AllocateTextFormat (0xa93870) and AllocateParagraphFormat (0xa93960) look the value up in a hash
// keyed by GFxTextFormat::HashFunctor (0xa91220) and hand back a shared, refcounted copy;
// FlushTextFormatCache (0xa935f0) drops the entries nothing references any more.
class GFxTextAllocator
{
public:
    GFxTextAllocator();                                        // 0xa0dd70
    ~GFxTextAllocator();

    const GFxTextFormat*          AllocateTextFormat(const GFxTextFormat& f);
    const GFxTextParagraphFormat* AllocateParagraphFormat(const GFxTextParagraphFormat& f);
    unsigned int GetTextFormatCacheSize() const { return TextFormats.GetSize(); }
    unsigned int GetParagraphFormatCacheSize() const { return ParagraphFormats.GetSize(); }

private:
    GArray<GFxTextFormat*>          TextFormats;
    GArray<GFxTextParagraphFormat*> ParagraphFormats;
};

// ---------------------------------------------------------------------------------------------
// GFxTextParagraph: one line of source text (up to and including its newline) with a paragraph
// format and a run-length list of character formats.
class GFxTextParagraph
{
public:
    // The run-length entry of retail's GRangeDataArray<GPtr<GFxTextFormat> >.
    struct FormatRun
    {
        unsigned int Index;
        unsigned int Length;
        const GFxTextFormat* pFormat;
    };

    GFxTextParagraph(GFxTextAllocator* alloc);                 // 0xaa4650
    ~GFxTextParagraph();

    void Clear();                                              // 0xaa49a0
    unsigned int GetLength() const { return Text.GetSize(); }   // 0xaa09d0
    const wchar_t* GetText() const { return Text.GetSize() ? &Text[0] : L""; }
    wchar_t GetChar(unsigned int i) const { return i < Text.GetSize() ? Text[i] : 0; }
    bool HasNewLine() const;                                   // 0xaa0990

    void SetText(const wchar_t* s, unsigned int len);           // 0xaa0f40
    void InsertString(const wchar_t* s, unsigned int at, unsigned int len,
                      const GFxTextFormat* fmt);               // 0xaa55d0
    void Remove(unsigned int at, unsigned int len);             // 0xaa51a0

    void SetFormat(const GFxTextParagraphFormat& f);           // 0xaa14e0
    const GFxTextParagraphFormat* GetFormat() const { return pFormat; }

    void SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len);  // 0xaa5400
    const GFxTextFormat* GetTextFormatPtr(unsigned int at) const;                   // 0xaa3540

    unsigned int GetFormatRunCount() const { return Runs.GetSize(); }
    const FormatRun& GetFormatRun(unsigned int i) const { return Runs[i]; }

    unsigned int StartIndex;      // the paragraph's first character index in the document

private:
    void normalizeRuns();

    GFxTextAllocator*             pAllocator;
    GArray<wchar_t>               Text;
    GArray<FormatRun>             Runs;
    const GFxTextParagraphFormat* pFormat;
};

// ---------------------------------------------------------------------------------------------
// GFxStyledText: the document. A list of paragraphs plus the two default formats.
class GFxStyledText
{
public:
    enum NewLinePolicy
    {
        NLP_PreserveAll     = 0,
        NLP_ReplaceCRLFWithLF = 1,
        NLP_CollapseCRLF    = 2
    };

    GFxStyledText(GFxTextAllocator* alloc);                    // 0xaa6ee0
    ~GFxStyledText();                                          // 0xaa0650

    void Clear();                                              // 0xaa6d90
    unsigned int GetLength() const;                            // 0xaa1720
    void GetText(GArray<wchar_t>* out) const;                  // 0xaa17e0

    void SetText(const char* s);                               // 0xaa6fe0
    void SetText(const wchar_t* s, unsigned int len);          // 0xaa7010
    unsigned int AppendString(const char* s, unsigned int len, NewLinePolicy p);      // 0xaa6fc0
    unsigned int AppendString(const wchar_t* s, unsigned int len, NewLinePolicy p,
                              const GFxTextFormat* fmt,
                              const GFxTextParagraphFormat* pfmt);                   // 0xaa6190

    GFxTextParagraph* AppendNewParagraph(const GFxTextParagraphFormat* pfmt);         // 0xaa5af0
    GFxTextParagraph* GetLastParagraph();                                             // 0xaa1560
    unsigned int GetParagraphCount() const { return Paragraphs.GetSize(); }
    GFxTextParagraph* GetParagraph(unsigned int i) const { return Paragraphs[i]; }

    void SetDefaultTextFormat(const GFxTextFormat& f);                                // 0xaa47b0
    void SetDefaultParagraphFormat(const GFxTextParagraphFormat& f);                  // 0xaa4890
    const GFxTextFormat& GetDefaultTextFormat() const { return DefaultTextFormat; }
    const GFxTextParagraphFormat& GetDefaultParagraphFormat() const
        { return DefaultParagraphFormat; }

    void SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len);    // 0xaa5730
    void SetParagraphFormat(const GFxTextParagraphFormat& f, unsigned int at,
                            unsigned int len);                                        // 0xaa1670

    GFxTextAllocator* GetAllocator() const { return pAllocator; }                     // 0xa9b480

private:
    void reindex();

    GFxTextAllocator*          pAllocator;
    GArray<GFxTextParagraph*>  Paragraphs;
    GFxTextFormat              DefaultTextFormat;
    GFxTextParagraphFormat     DefaultParagraphFormat;
};

// Widen a UTF-8 / Latin-1 byte string. Retail goes through GFxWStringBuffer and the UTF-8 decoder of
// GUTF8Util; this decodes the same two cases (a plain byte and a 2/3-byte UTF-8 sequence), which is
// all the cook's content uses.
unsigned int GFxTextUtf8ToWide(const char* s, unsigned int bytes, GArray<wchar_t>* out);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // INC_GFXTEXT_H
