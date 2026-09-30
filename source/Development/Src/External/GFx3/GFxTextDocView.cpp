// GFx 3.3 text layout - the line buffer, the line cursor, the paragraph formatter and the document
// view. See GFxTextDocView.h for the coordinate system and the one documented deviation.
#include "GFxTextDocView.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

// GTypes.h's GArray has a destructor and no copy constructor, so copying one by value shallow-copies
// its buffer and both copies free it. Every place this file needs a copy goes through this instead.
// (Found by the harness: a by-value GArray copy in the word-wrap path corrupted the heap.)
template<class T>
static void GFxTextCopyArray(GArray<T>* dst, const GArray<T>& src)
{
    dst->Clear();
    dst->Reserve(src.GetSize());
    for (unsigned int i = 0; i < src.GetSize(); ++i)
        dst->PushBack(src[i]);
}

static int GFxRoundToInt(float v)
{
    // Retail rounds half away from zero everywhere in the formatter: `v <= 0 ? v - 0.5 : v + 0.5`
    // then truncates (0xa9a940 does it six times).
    return (int)(v <= 0.0f ? v - 0.5f : v + 0.5f);
}

// =============================================================================================
// GFxTextLineBuffer
// =============================================================================================

void GFxTextLineBuffer::GlyphEntry::SetFontSize(float px)
{
    // DISHONORED(port): 0xa43e40 - below 256 px a size with a non-zero sixteenth is stored as
    // `(int)(px * 16)` with the fraction flag set, otherwise as the whole number of pixels. The
    // quantum is what makes two text fields at 11.5 px share a glyph-cache entry.
    if (px < 256.0f)
    {
        const int q = (int)(16.0f * px);
        if ((q & 0x0F) != 0)
        {
            SizeQuantum = (unsigned short)(q & 0x0FFF);
            SizeIsFraction = true;
            return;
        }
    }
    SizeQuantum = (unsigned short)((int)px & 0x0FFF);
    SizeIsFraction = false;
}

float GFxTextLineBuffer::GlyphEntry::GetFontSize() const
{
    // DISHONORED(port): 0x9bdc40
    if (!SizeIsFraction)
        return (float)SizeQuantum;
    return (float)((double)SizeQuantum * 0.0625);
}

GFxTextLineBuffer::Line::Line()
    : TextPos(0), NumGlyphs(0), OffsetX(0), OffsetY(0), Width(0), Height(0), Leading(0),
      BaselineOffset(0.0f)
{
}

bool GFxTextLineBuffer::Line::HasNewLine() const
{
    // DISHONORED(port): 0xa44280 - the last glyph of the line carries the newline flag.
    if (!Glyphs.GetSize())
        return false;
    return (Glyphs[Glyphs.GetSize() - 1].Flags & GlyphEntry::EF_Newline) != 0;
}

GFxTextLineBuffer::GFxTextLineBuffer()
    : VScrollOffset(0), HScrollOffset(0)
{
    // DISHONORED(port): 0xa45aa0
}

GFxTextLineBuffer::~GFxTextLineBuffer()
{
    // DISHONORED(port): 0xa48260
    Clear();
}

void GFxTextLineBuffer::Clear()
{
    for (unsigned int i = 0; i < Lines.GetSize(); ++i)
        delete Lines[i];
    Lines.Clear();
}

GFxTextLineBuffer::Line* GFxTextLineBuffer::InsertNewLine()
{
    // DISHONORED(port): 0xa45b80 / 0xa99060
    Line* l = new Line;
    Lines.PushBack(l);
    return l;
}

void GFxTextLineBuffer::RemoveLines(unsigned int at, unsigned int count)
{
    // DISHONORED(port): 0xa48180
    if (at >= Lines.GetSize())
        return;
    if (at + count > Lines.GetSize())
        count = Lines.GetSize() - at;
    for (unsigned int i = 0; i < count; ++i)
        delete Lines[at + i];
    for (unsigned int i = at; i + count < Lines.GetSize(); ++i)
        Lines[i] = Lines[i + count];
    Lines.Resize(Lines.GetSize() - count);
}

int GFxTextLineBuffer::GetMinLineHeight() const
{
    // DISHONORED(port): 0xa44680
    int m = 0;
    for (unsigned int i = 0; i < Lines.GetSize(); ++i)
    {
        const int h = Lines[i]->GetHeight();
        if (i == 0 || h < m)
            m = h;
    }
    return m;
}

unsigned int GFxTextLineBuffer::CalcLineSize(unsigned int glyphs, unsigned int formats,
                                            bool longLine)
{
    // DISHONORED(port): 0xa442c0
    return (((longLine ? 38u : 26u) + 8u * glyphs + 7u) & 0xFFFFFFFCu) + 4u * formats;
}

// =============================================================================================
// GFxLineCursor
// =============================================================================================

GFxLineCursor::GFxLineCursor()
{
    // DISHONORED(port): 0xa9a470
    Reset();
}

void GFxLineCursor::Reset()
{
    // DISHONORED(port): 0xa98eb0
    MaxAscent = 0.0f;
    MaxDescent = 0.0f;
    MaxLeading = 0.0f;
}

void GFxLineCursor::TrackFontParams(GFxFontResource* font, float scale)
{
    // DISHONORED(port): 0xa99640 - the 960 and 64 defaults are retail's literals and they add up to
    // the 1024-unit EM, so a font with no layout block still produces a line of the right height.
    if (!font)
        return;
    float ascent = font->GetAscent();
    float descent = font->GetDescent();
    if (ascent == 0.0f)
        ascent = 960.0f;
    if (descent == 0.0f)
        descent = 64.0f;

    const float a = ascent * scale;
    if (a > MaxAscent)
        MaxAscent = a;
    const float d = descent * scale;
    if (d > MaxDescent)
        MaxDescent = d;
    const float l = font->GetLeading() * scale;
    if (l > MaxLeading)
        MaxLeading = l;
}

// =============================================================================================
// GFxParagraphFormatter
// =============================================================================================

GFxParagraphFormatter::GFxParagraphFormatter(GFxTextDocView* view)
    : pView(view), pParaFormat(0), pLine(0), LineWidth(0), LeftOffset(0), RightMargin(0),
      AvailWidth(0), CurY(0), MaxWidth(0), TextPos(0), WrapGlyph(~0u), WrapWidth(0),
      WrapTextPos(0), GlyphsWithoutShape(0), bLastLine(false)
{
    // DISHONORED(port): 0xa9d080
    AvailWidth = (int)(view->ViewRect.Right - view->ViewRect.Left);
}

GFxParagraphFormatter::~GFxParagraphFormatter()
{
    // DISHONORED(port): 0xa9d160
}

float GFxParagraphFormatter::GetActualFontSize(const GFxTextFormat* fmt) const
{
    // DISHONORED(port): 0xa99740 - the format's size in pixels, times the font handle's own scale
    // when it is not 1, times the view's font scale factor when the view asks for one.
    if (!fmt)
        return 12.0f;
    float size = (float)fmt->GetFontSizeTwips() / 20.0f;
    GFxFontHandle* h = fmt->GetFontHandle();
    if (h && h->GetScale() != 1.0f)
        size = h->GetScale() * size;
    if (pView->FontScaleFactor != 1.0f && pView->FontScaleFactor != 0.0f)
        size = pView->FontScaleFactor * size;
    return size;
}

void GFxParagraphFormatter::InitParagraph(const GFxTextParagraph& para)
{
    // DISHONORED(port): 0xa9c5b0 - the margins and the indent come out of the paragraph format in
    // *pixels* and are multiplied by 20 here, which is the asymmetry GFxText.h documents.
    pParaFormat = para.GetFormat();
    if (!pParaFormat)
        pParaFormat = &pView->Text.GetDefaultParagraphFormat();

    AvailWidth = (int)(pView->ViewRect.Right - pView->ViewRect.Left);
    LeftOffset = 20 * (int)(pParaFormat->GetLeftMargin() + pParaFormat->GetBlockIndent());
    RightMargin = 20 * (int)pParaFormat->GetRightMargin();
    Cursor.Reset();
    LineWidth = 0;
    WrapGlyph = ~0u;
    WrapWidth = 0;
    bLastLine = false;
    TextPos = para.StartIndex;
    WrapTextPos = TextPos;
    beginLine();
}

void GFxParagraphFormatter::beginLine()
{
    pLine = pView->Lines.InsertNewLine();
    pLine->TextPos = TextPos;
    LineWidth = 0;
    WrapGlyph = ~0u;
    Cursor.Reset();
}

unsigned int GFxParagraphFormatter::formatIndexFor(GFxFontResource* font, GColor color,
                                                   float sizePx, bool underline)
{
    // Retail writes one GFxFormatDataEntry per style change of the line and stores its index in each
    // glyph (GFxTextLineBuffer::Line::GetFormatData 0xa44250). Same shape here.
    const unsigned int n = pLine->Formats.GetSize();
    if (n)
    {
        const GFxTextLineBuffer::FormatEntry& last = pLine->Formats[n - 1];
        if (last.pFont == font && last.Color == color && last.FontSize == sizePx &&
            last.Underline == underline)
            return n - 1;
    }
    GFxTextLineBuffer::FormatEntry e;
    e.pFont = font;
    e.Color = color;
    e.FontSize = sizePx;
    e.GlyphStart = pLine->Glyphs.GetSize();
    e.Underline = underline;
    pLine->Formats.PushBack(e);
    return pLine->Formats.GetSize() - 1;
}

void GFxParagraphFormatter::pushGlyph(const GFxTextLineBuffer::GlyphEntry& g)
{
    pLine->Glyphs.PushBack(g);
    pLine->NumGlyphs = pLine->Glyphs.GetSize();
    LineWidth += g.Advance;
}

bool GFxParagraphFormatter::CheckWordWrap()
{
    // DISHONORED(port): 0xa9c1a0 - wrap when the accumulated width plus the offsets exceeds the
    // available width less the right margin, then back the line up to the recorded break opportunity
    // and finalise. Returns true when a line was closed.
    if ((pView->Flags & GFxTextDocView::VF_WordWrap) == 0)
        return false;
    if ((float)(LeftOffset + LineWidth) <= (float)(AvailWidth - RightMargin))
        return false;
    if (pLine->Glyphs.GetSize() <= 1)
        return false;

    if (WrapGlyph != ~0u && WrapGlyph + 1 < pLine->Glyphs.GetSize())
    {
        // Move everything after the break onto the next line, exactly as retail's
        // GlyphInserter::ResetTo (0xa457b0) plus the cursor restore does.
        GArray<GFxTextLineBuffer::GlyphEntry> carry;
        for (unsigned int i = WrapGlyph + 1; i < pLine->Glyphs.GetSize(); ++i)
            carry.PushBack(pLine->Glyphs[i]);
        GArray<GFxTextLineBuffer::FormatEntry> formats;
        GFxTextCopyArray(&formats, pLine->Formats);

        pLine->Glyphs.Resize(WrapGlyph + 1);
        pLine->NumGlyphs = pLine->Glyphs.GetSize();
        LineWidth = WrapWidth;

        const unsigned int nextPos = WrapTextPos;
        FinalizeLine();
        TextPos = nextPos;
        beginLine();
        GFxTextCopyArray(&pLine->Formats, formats);
        for (unsigned int i = 0; i < carry.GetSize(); ++i)
            pushGlyph(carry[i]);
        return true;
    }

    // No break opportunity on the line: retail still closes it, which is what makes an unbreakable
    // run overflow by one glyph rather than for ever.
    GFxTextLineBuffer::GlyphEntry last = pLine->Glyphs[pLine->Glyphs.GetSize() - 1];
    pLine->Glyphs.Resize(pLine->Glyphs.GetSize() - 1);
    pLine->NumGlyphs = pLine->Glyphs.GetSize();
    LineWidth -= last.Advance;
    GArray<GFxTextLineBuffer::FormatEntry> formats;
    GFxTextCopyArray(&formats, pLine->Formats);
    FinalizeLine();
    beginLine();
    GFxTextCopyArray(&pLine->Formats, formats);
    pushGlyph(last);
    return true;
}

void GFxParagraphFormatter::FinalizeLine()
{
    // DISHONORED(port): 0xa9a940. The order is retail's: clamp the width, take the leading from the
    // paragraph format when it has one (times 20 - it is in pixels) and from the font otherwise,
    // height is round(ascent + descent), then the alignment decides the line's x offset.
    if (!pLine)
        return;

    if (LineWidth < 0)
        LineWidth = 0;

    float leading = Cursor.MaxLeading;
    if (pParaFormat && pParaFormat->HasLeading())
        leading = (float)(20 * (int)pParaFormat->GetLeading());

    const int height = GFxRoundToInt(Cursor.MaxAscent + Cursor.MaxDescent);
    const int leadingTwips = GFxRoundToInt(leading);

    pLine->Width = LineWidth;
    pLine->Height = height;
    pLine->Leading = leadingTwips;
    pLine->SetBaseLineOffset(Cursor.MaxAscent);
    pLine->OffsetY = CurY;
    pLine->NumGlyphs = pLine->Glyphs.GetSize();

    if (pParaFormat && pParaFormat->IsRightAlignment())
    {
        const int x = AvailWidth - RightMargin - LineWidth;
        pLine->OffsetX = x < 0 ? 0 : x;
    }
    else if (pParaFormat && pParaFormat->IsCenterAlignment())
    {
        const float avail = (float)(AvailWidth - RightMargin);
        const float x = avail * 0.5f - (float)(LineWidth / 2);
        const int off = RightMargin + GFxRoundToInt(x) - RightMargin;
        pLine->OffsetX = off < 0 ? 0 : off;
    }
    else if (pParaFormat && pParaFormat->IsJustifyAlignment())
    {
        // Retail distributes the slack over the whitespace glyphs of the line and then reports the
        // line as full width. Only a line that is not the paragraph's last is justified, which is
        // what the `!IsLastLine` test in the retail body checks.
        unsigned int spaces = 0;
        for (unsigned int i = 0; i < pLine->Glyphs.GetSize(); ++i)
            if (pLine->Glyphs[i].Flags & GFxTextLineBuffer::GlyphEntry::EF_Whitespace)
                ++spaces;
        const int slack = AvailWidth - RightMargin - LeftOffset - LineWidth;
        if (spaces && slack > 0 && !bLastLine && !pLine->HasNewLine())
        {
            const int per = slack / (int)spaces;
            for (unsigned int i = 0; i < pLine->Glyphs.GetSize(); ++i)
            {
                if (pLine->Glyphs[i].Flags & GFxTextLineBuffer::GlyphEntry::EF_Whitespace)
                    pLine->Glyphs[i].SetAdvance(pLine->Glyphs[i].Advance + per);
            }
            LineWidth += per * (int)spaces;
            pLine->Width = LineWidth;
        }
        pLine->OffsetX = LeftOffset;
    }
    else
    {
        pLine->OffsetX = LeftOffset;
    }

    const int right = pLine->OffsetX + LineWidth;
    if (right > MaxWidth)
        MaxWidth = right;
    CurY += GFxRoundToInt((float)(height + leadingTwips));

    pLine = 0;
}

void GFxParagraphFormatter::Format(const GFxTextParagraph& para)
{
    // DISHONORED(port): 0xa9d310 (4,226 bytes). The glyph scale, the kerning, the letter spacing, the
    // tab handling and the bullet size are all retail's; the HTML image substitution, the custom
    // word-wrap callback, the IME composition string and the editor kit's caret are not (see
    // agentCB.md's table of remaining functions).
    InitParagraph(para);

    const GFxTextFormat& defFmt = pView->Text.GetDefaultTextFormat();
    const unsigned int len = para.GetLength();

    GFxFontResource* font = 0;
    const GFxTextFormat* curFmt = 0;
    float fontSize = 0.0f;
    float glyphScale = 0.0f;
    int letterSpacing = 0;
    bool kerning = false;
    unsigned int formatIndex = 0;
    int prevGlyph = -1;

    for (unsigned int i = 0; i < len; ++i)
    {
        const GFxTextFormat* fmt = para.GetTextFormatPtr(i);
        if (!fmt)
            fmt = &defFmt;
        if (fmt != curFmt)
        {
            curFmt = fmt;
            font = pView->FindFont(fmt);
            fontSize = GetActualFontSize(fmt);
            // The one conversion that matters: 1024 glyph units are one EM, an EM is fontSize
            // pixels, and a pixel is 20 twips.
            glyphScale = fontSize * 20.0f * 0.0009765625f;
            letterSpacing = (int)(fmt->GetLetterSpacing() * 20.0f);
            kerning = fmt->IsKerning();
            formatIndex = formatIndexFor(font, fmt->GetColor(), fontSize, fmt->IsUnderline());
            prevGlyph = -1;
        }
        if (!pLine)
            beginLine();
        else if (pLine->Formats.GetSize() == 0)
            formatIndex = formatIndexFor(font, fmt->GetColor(), fontSize, fmt->IsUnderline());

        Cursor.TrackFontParams(font, glyphScale);

        const wchar_t ch = para.GetChar(i);

        GFxTextLineBuffer::GlyphEntry g;
        g.GlyphIndex = ~0u;
        g.Advance = 0;
        g.FormatIndex = formatIndex;
        g.Flags = 0;
        g.SetFontSize(fontSize);

        if (ch == L'\n' || ch == L'\r')
        {
            g.Flags |= GFxTextLineBuffer::GlyphEntry::EF_Newline;
            pushGlyph(g);
            ++TextPos;
            bLastLine = (i + 1 >= len);
            FinalizeLine();
            bLastLine = false;
            if (i + 1 < len)
                beginLine();
            prevGlyph = -1;
            continue;
        }

        int glyphIndex = font ? font->GetGlyphIndex((unsigned short)ch) : -1;
        if (glyphIndex < 0)
        {
            ++GlyphsWithoutShape;
            glyphIndex = -1;
        }
        g.GlyphIndex = glyphIndex < 0 ? ~0u : (unsigned int)glyphIndex;

        float advance;
        if (ch == L'\t')
        {
            // 0xa9d310's tab arithmetic: the default tab is eight times a nominal
            // (fontSize + fontSize + 8) / 8 quantum, in pixels, converted to twips.
            const float quantum = (fontSize + fontSize + 8.0f) * 0.125f;
            advance = quantum * 8.0f * 20.0f;
            g.Flags |= GFxTextLineBuffer::GlyphEntry::EF_Tab;
            g.Flags |= GFxTextLineBuffer::GlyphEntry::EF_Whitespace;
        }
        else
        {
            advance = font ? font->GetAdvance(g.GlyphIndex) * glyphScale : 0.0f;
            if (kerning && prevGlyph >= 0 && font && glyphIndex >= 0)
                advance += font->GetKerningAdjustment((unsigned int)prevGlyph,
                                                     (unsigned int)glyphIndex) * glyphScale;
            advance += (float)letterSpacing;
        }
        g.SetAdvance(GFxRoundToInt(advance));

        if (ch == L' ' || ch == L'\t')
            g.Flags |= GFxTextLineBuffer::GlyphEntry::EF_Whitespace;
        if (curFmt->IsUnderline())
            g.Flags |= GFxTextLineBuffer::GlyphEntry::EF_Underline;

        // A break opportunity is recorded *after* a space, which is what makes the trailing space of
        // a wrapped line stay on it.
        if (ch == L' ' || ch == L'\t' || ch == L'-')
        {
            WrapGlyph = pLine->Glyphs.GetSize();
            WrapWidth = LineWidth + g.Advance;
            WrapTextPos = TextPos + 1;
        }

        pushGlyph(g);
        ++TextPos;
        prevGlyph = glyphIndex;
        ++pView->GlyphsLaidOut;

        CheckWordWrap();
    }

    if (pLine)
    {
        bLastLine = true;
        FinalizeLine();
        bLastLine = false;
    }
}

// =============================================================================================
// GFxTextDocView
// =============================================================================================

GFxTextDocView::GFxTextDocView(GFxTextAllocator* alloc, GFxFontManager* fonts)
    : pAllocator(alloc), pFontManager(fonts), Text(alloc),
      TextRect(0.0f, 0.0f, 0.0f, 0.0f), ViewRect(0.0f, 0.0f, 0.0f, 0.0f),
      Flags(VF_NeedsFormat), VAlignment(VAlign_None), FontScaleFactor(1.0f),
      TextWidthTwips(0), TextHeightTwips(0), GlyphsLaidOut(0), GlyphsWithoutShape(0)
{
    // DISHONORED(port): 0xaa06b0
}

GFxTextDocView::~GFxTextDocView()
{
    // DISHONORED(port): 0xa9c4e0
}

void GFxTextDocView::SetViewRect(const GRect<float>& rect)
{
    // DISHONORED(port): 0xa9a1e0 - the 40-twip gutter on each side, which is SWF's text-field inset.
    TextRect = rect;
    ViewRect.Left = TextRect.Left + 40.0f;
    ViewRect.Top = TextRect.Top + 40.0f;
    ViewRect.Right = TextRect.Right - 40.0f;
    ViewRect.Bottom = TextRect.Bottom - 40.0f;
    Flags |= VF_NeedsFormat;
}

void GFxTextDocView::SetText(const wchar_t* s, unsigned int len)
{
    // DISHONORED(port): 0xa991c0
    Text.SetText(s, len);
    Flags |= VF_NeedsFormat;
}

void GFxTextDocView::SetText(const char* s)
{
    // DISHONORED(port): 0xa99190
    Text.SetText(s);
    Flags |= VF_NeedsFormat;
}

void GFxTextDocView::SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len)
{
    // DISHONORED(port): 0xa991f0
    Text.SetTextFormat(f, at, len);
    Flags |= VF_NeedsFormat;
}

void GFxTextDocView::SetParagraphFormat(const GFxTextParagraphFormat& f, unsigned int at,
                                        unsigned int len)
{
    // DISHONORED(port): 0xa99220
    Text.SetParagraphFormat(f, at, len);
    Flags |= VF_NeedsFormat;
}

void GFxTextDocView::SetDefaultTextAndParaFormat(const GFxTextFormat& f,
                                                 const GFxTextParagraphFormat& p)
{
    // DISHONORED(port): 0xa99920
    Text.SetDefaultTextFormat(f);
    Text.SetDefaultParagraphFormat(p);
    Flags |= VF_NeedsFormat;
}

// DISHONORED(bringup, agent EX): every distinct (requested font list -> resolved font, by which
// route) pair a run produces, once each. "Which font does this field actually get, and from where"
// is the direct reading of a wrong-font report, and it is otherwise only answerable by a bisect.
// "FALLBACK" is the interesting line: it means the field asked for a font nothing could supply and
// got font 0 instead.
static void GFxTextDocViewNoteFontResolution(const char* requested, GFxFontResource* resolved,
                                             const char* route)
{
    static char Seen[64][160];
    static unsigned int SeenCount = 0;
    char line[160];
    const char* got = (resolved && resolved->GetName()) ? resolved->GetName() : "<none>";
    _snprintf(line, sizeof(line), "'%s' -> '%s' (%s)", requested ? requested : "", got, route);
    line[sizeof(line) - 1] = 0;
    for (unsigned int i = 0; i < SeenCount; ++i)
        if (strcmp(Seen[i], line) == 0)
            return;
    if (SeenCount < 64)
    {
        strncpy(Seen[SeenCount], line, sizeof(Seen[0]) - 1);
        Seen[SeenCount][sizeof(Seen[0]) - 1] = 0;
        ++SeenCount;
    }
    GFxLogf("DISHONORED(bringup): GFx font resolved: %s", line);
}

GFxFontResource* GFxTextDocView::FindFont(const GFxTextFormat* fmt)
{
    // DISHONORED(port): 0xa9ccf0. Retail's order is: the format's already-resolved handle, then the
    // comma-separated font list against the movie's own fonts, then the font library, then the font
    // provider, then the font map, then the empty font. The handle and the list are here; the
    // provider and the map are not (agentCB.md, "Remaining").
    if (!fmt)
        return 0;
    GFxFontHandle* h = fmt->GetFontHandle();
    if (h && h->GetFontResource())
    {
        GFxTextDocViewNoteFontResolution(fmt->GetFontList(), h->GetFontResource(), "handle");
        return h->GetFontResource();
    }
    if (!pFontManager)
    {
        GFxTextDocViewNoteFontResolution(fmt->GetFontList(), 0, "no font manager");
        return 0;
    }

    const char* list = fmt->GetFontList();
    const unsigned int style = fmt->GetFontStyleFlags();
    char name[128];
    unsigned int at = 0;
    while (list[at])
    {
        unsigned int n = 0;
        while (list[at] && list[at] != ',' && n < sizeof(name) - 1)
            name[n++] = list[at++];
        while (n && (name[n - 1] == ' ' || name[n - 1] == '\t'))
            --n;
        name[n] = 0;
        if (n)
        {
            GFxFontResource* r = pFontManager->FindFontResource(name, style);
            if (r)
            {
                GFxTextDocViewNoteFontResolution(fmt->GetFontList(), r, "font manager");
                return r;
            }
        }
        if (list[at] == ',')
            ++at;
        while (list[at] == ' ')
            ++at;
    }
    // Retail falls back to the *first* registered font rather than to nothing, because a null font
    // would make the line have no height at all.
    if (pFontManager->GetFontCount())
    {
        GFxFontResource* r = pFontManager->GetFontByIndex(0);
        GFxTextDocViewNoteFontResolution(fmt->GetFontList(), r, "FALLBACK");
        return r;
    }
    GFxTextDocViewNoteFontResolution(fmt->GetFontList(), 0, "NONE");
    return 0;
}

bool GFxTextDocView::ContainsNonLeftAlignment() const
{
    // DISHONORED(port): 0xa999e0 - what SetViewRect tests before it can take the cheap path.
    for (unsigned int i = 0; i < Text.GetParagraphCount(); ++i)
    {
        const GFxTextParagraphFormat* f = Text.GetParagraph(i)->GetFormat();
        if (f && !f->IsLeftAlignment())
            return true;
    }
    return !Text.GetDefaultParagraphFormat().IsLeftAlignment();
}

void GFxTextDocView::Format()
{
    // DISHONORED(port): 0xa9e3a0 (3,615 bytes). The retail body additionally rescales the whole line
    // buffer to fit when the field is a shrink-to-fit one (GFxTextLineBuffer::Scale 0xa47d40, with the
    // 120.0 minimum-line-height clamp), handles the vertical alignment and drives the scroll
    // clamping; the paragraph loop, the auto-size grow and the width/height accumulation are here.
    Lines.Clear();
    GlyphsLaidOut = 0;
    GlyphsWithoutShape = 0;

    GFxParagraphFormatter formatter(this);
    for (unsigned int i = 0; i < Text.GetParagraphCount(); ++i)
        formatter.Format(*Text.GetParagraph(i));

    GlyphsWithoutShape = formatter.GetGlyphsWithoutShape();
    TextWidthTwips = formatter.GetMaxWidth();
    TextHeightTwips = formatter.GetTotalHeight();

    // Auto-size: grow the text rect so the content fits, with the 40-twip gutter re-applied. Retail's
    // grow adds 80 twips (two gutters) in each direction, which is the `+ 80.0` of 0xa9e3a0.
    if (Flags & VF_AutoSizeX)
    {
        const float want = (float)TextWidthTwips + 80.0f;
        if (want > TextRect.Right - TextRect.Left)
        {
            GRect<float> r = TextRect;
            r.Right = r.Left + want;
            SetViewRect(r);
        }
    }
    if (Flags & VF_AutoSizeY)
    {
        const float want = (float)TextHeightTwips + 80.0f;
        if (want > TextRect.Bottom - TextRect.Top)
        {
            GRect<float> r = TextRect;
            r.Bottom = r.Top + want;
            SetViewRect(r);
        }
    }

    // Vertical alignment moves every line, which is what VAlign_Center/Bottom mean for a field whose
    // content is shorter than its box.
    if (VAlignment != VAlign_None && VAlignment != VAlign_Top)
    {
        const int box = (int)(ViewRect.Bottom - ViewRect.Top);
        int shift = box - TextHeightTwips;
        if (shift > 0)
        {
            if (VAlignment == VAlign_Center)
                shift /= 2;
            for (unsigned int i = 0; i < Lines.GetLineCount(); ++i)
                Lines.GetLine(i)->OffsetY += shift;
        }
    }

    Flags &= ~(unsigned int)VF_NeedsFormat;
}

GRect<float>& GFxTextDocView::GetViewRect()
{
    if (Flags & VF_NeedsFormat)
        Format();
    return ViewRect;
}

float GFxTextDocView::GetTextWidth()
{
    // DISHONORED(port): 0xa9f200 - formats first when the view is dirty, which is why every AS2
    // textWidth read is correct without the field having been drawn.
    if (Flags & VF_NeedsFormat)
        Format();
    return (float)TextWidthTwips;
}

float GFxTextDocView::GetTextHeight()
{
    // DISHONORED(port): 0xa9f240
    if (Flags & VF_NeedsFormat)
        Format();
    return (float)TextHeightTwips;
}

unsigned int GFxTextDocView::GetLinesCount()
{
    // DISHONORED(port): 0xa9f280
    if (Flags & VF_NeedsFormat)
        Format();
    return Lines.GetLineCount();
}

bool GFxTextDocView::GetLineMetrics(unsigned int line, LineMetrics* out)
{
    // DISHONORED(port): 0xa9fc60 - the field order is retail's: width, height, ascent, descent,
    // offset x, leading.
    if (!out)
        return false;
    if (Flags & VF_NeedsFormat)
        Format();
    GFxTextLineBuffer::Line* l = Lines.GetLine(line);
    if (!l)
        return false;
    out->Width = l->Width;
    out->Height = l->Height;
    out->Ascent = (int)l->BaselineOffset;
    out->Descent = (int)l->GetDescent();
    out->OffsetX = l->OffsetX;
    out->Leading = l->Leading;
    return true;
}
