// GFx 3.3 text fields - the DefineEditText definition and the character that lays it out.
#include "GFxTextField.h"

#include <math.h>
#include "GFxAS2Runtime.h"
#include "GFxDisplay.h"

#include <string.h>
#include <stdio.h>

// =============================================================================================
// The process-wide font registry and the tag-loader hook
// =============================================================================================

static GFxFontManager* GFxTextFontManagerInstance = 0;

GFxFontManager* GFxTextGetFontManager()
{
    if (!GFxTextFontManagerInstance)
        GFxTextFontManagerInstance = new GFxFontManager;
    return GFxTextFontManagerInstance;
}

void GFxTextResetFontManager()
{
    delete GFxTextFontManagerInstance;
    GFxTextFontManagerInstance = 0;
}

// =============================================================================================
// GFxTextFieldDesc - the DefineEditText record
// =============================================================================================

GFxTextFieldDesc::GFxTextFieldDesc()
    : TextRectTwips(0, 0, 0, 0), FontId(0), FontHeightTwips(240.0f), TextColor(0xFF000000u),
      MaxLength(0), LeftMarginTwips(0.0f), RightMarginTwips(0.0f), IndentTwips(0.0f),
      LeadingTwips(0.0f), Flags(0), Align(0)
{
    // DISHONORED(port): 2013 0xa1d900, GFxEditTextCharacterDef's constructor. (Was cited as
    // 2012 0xa27200, which is not a function start in retail.)
    VariableName[0] = 0;
    InitialText[0] = 0;
    FontName[0] = 0;
}

void GFxTextFieldDesc::InitEmptyTextDef()
{
    // DISHONORED(port): 0xa26720 - what an AS2 createTextField() produces.
    Flags = ETF_Selectable;
    FontHeightTwips = 240.0f;
    TextColor = GColor(0xFF000000u);
}

bool GFxTextFieldReadDesc(GFxStream* s, unsigned int tagType, unsigned int tagEnd,
                          GFxTextFieldDesc* out)
{
    GFxTextFieldDesc& d = *out;
    // DISHONORED(port): 0xa26190 - the DefineEditText record, in retail's exact read order. The two
    // inverted bits (NoSelect -> Selectable, UseOutlines -> UseDeviceFont) are retail's inversions.
    (void)tagType;
    int rect[4];
    s->ReadRect(rect);
    // GFxStream::ReadRect's order is left, right, top, bottom.
    d.TextRectTwips = GRect<int>(rect[0], rect[2], rect[1], rect[3]);
    s->Align();

    const bool hasText = s->ReadUBits(1) != 0;
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_WordWrap;
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_Multiline;
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_Password;
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_ReadOnly;
    const bool hasColor  = s->ReadUBits(1) != 0;
    const bool hasMaxLen = s->ReadUBits(1) != 0;
    const bool hasFont   = s->ReadUBits(1) != 0;
    s->ReadUBits(1);                              // HasFontClass, read and dropped
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_AutoSize;
    const bool hasLayout = s->ReadUBits(1) != 0;
    if (!s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_Selectable;   // NoSelect, inverted
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_Border;
    s->ReadUBits(1);                              // WasStatic, read and dropped
    if (s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_Html;
    if (!s->ReadUBits(1)) d.Flags |= GFxTextFieldDesc::ETF_UseDeviceFont; // UseOutlines, inverted
    s->Align();

    if (hasFont)
    {
        d.FontId = s->ReadU16();
        d.FontHeightTwips = (float)s->ReadU16();
    }
    if (hasColor)
        s->ReadRgba(&d.TextColor);
    if (hasMaxLen)
        d.MaxLength = s->ReadU16();
    if (hasLayout)
    {
        d.Flags |= GFxTextFieldDesc::ETF_HasLayout;
        d.Align = s->ReadU8();
        d.LeftMarginTwips = (float)s->ReadU16();
        d.RightMarginTwips = (float)s->ReadU16();
        d.IndentTwips = (float)s->ReadS16();
        d.LeadingTwips = (float)s->ReadS16();
    }
    s->ReadString(d.VariableName, sizeof(d.VariableName));
    if (hasText)
        s->ReadString(d.InitialText, sizeof(d.InitialText));
    return s->Tell() <= tagEnd;
}

GFxEditTextCharacter* GFxTextFieldCreateFromTag(GFxStream* s, unsigned int tagType,
                                                unsigned int tagEnd, GFxASCharacter* parent,
                                                GFxResourceId id, GFxMovieRoot* root)
{
    // DISHONORED(port): GFx_DefineEditTextLoader 0xa272a0 followed by
    // GFxEditTextCharacterDef::CreateCharacterInstance 0xa32df0.
    GFxTextFieldDesc desc;
    if (!GFxTextFieldReadDesc(s, tagType, tagEnd, &desc))
        return 0;
    return new GFxEditTextCharacter(desc, parent, id, root);
}

// =============================================================================================
// GFxEditTextCharacter
// =============================================================================================

GFxEditTextCharacter::GFxEditTextCharacter(const GFxTextFieldDesc& desc, GFxASCharacter* parent,
                                           GFxResourceId id, GFxMovieRoot* root)
    : GFxASCharacter(parent, id, root), Desc(desc), Doc(&Allocator, GFxTextGetFontManager()),
      bDirty(true)
{
    const GFxTextFieldDesc* def = &Desc;
    // DISHONORED(port): 0xa2c470 (1,016 bytes) - build the document view, set the text rect from the
    // definition's bounds, copy the definition's flags across, apply the initial formats and then the
    // initial text.
    TextValue[0] = 0;

    GRect<float> rect((float)def->TextRectTwips.Left, (float)def->TextRectTwips.Top,
                      (float)def->TextRectTwips.Right, (float)def->TextRectTwips.Bottom);
    Doc.SetViewRect(rect);

    unsigned int flags = 0;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_WordWrap))      flags |= GFxTextDocView::VF_WordWrap;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_Multiline))     flags |= GFxTextDocView::VF_Multiline;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_Password))      flags |= GFxTextDocView::VF_Password;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_ReadOnly))      flags |= GFxTextDocView::VF_ReadOnly;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_Selectable))    flags |= GFxTextDocView::VF_Selectable;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_Border))        flags |= GFxTextDocView::VF_Border;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_Html))          flags |= GFxTextDocView::VF_Html;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_UseDeviceFont)) flags |= GFxTextDocView::VF_UseDeviceFont;
    if (Desc.HasFlag(GFxTextFieldDesc::ETF_AutoSize))
    {
        // UpdateAutosizeSettings (0xa2d170): a single-line auto-size field grows in x, a word-wrapped
        // or multiline one in y.
        if (Desc.HasFlag(GFxTextFieldDesc::ETF_WordWrap) ||
            Desc.HasFlag(GFxTextFieldDesc::ETF_Multiline))
            flags |= GFxTextDocView::VF_AutoSizeY;
        else
            flags |= GFxTextDocView::VF_AutoSizeX;
    }
    Doc.SetFlags(flags);

    SetInitialFormatsAsDefault();

    if (Desc.InitialText[0])
        SetTextValue(Desc.InitialText,
                     Desc.HasFlag(GFxTextFieldDesc::ETF_Html), false);
}

GFxEditTextCharacter::~GFxEditTextCharacter()
{
    // DISHONORED(port): 0xa32980
}

// -gfxuinohtmlface, parsed in GFxUI/Src/gfxuiengine.cpp. See GFxTextField.h.
bool GFxTextNoHtmlFace = false;

// DISHONORED(bringup): every DefineEditText field's font, once per distinct outcome, plus a running
// census of the routes. This is the path the menu's text actually takes - GFxTextDocView::FindFont
// only ever sees a format that already has a handle, which is why a trace there measured no
// fallbacks on screens that are visibly wrong.
static unsigned int GFxFieldFontTotal = 0;
static unsigned int GFxFieldFontByFace = 0;
static unsigned int GFxFieldFontById = 0;
static unsigned int GFxFieldFontFallback = 0;
static unsigned int GFxFieldFontNone = 0;

static void GFxEditTextNoteFontResolution(unsigned int fontId, const char* wanted,
                                          GFxFontResource* got, const char* route, const char* text)
{
    ++GFxFieldFontTotal;
    if (strcmp(route, "html face") == 0)            ++GFxFieldFontByFace;
    else if (strcmp(route, "by name") == 0)         ++GFxFieldFontById;
    else if (strncmp(route, "FALLBACK", 8) == 0)    ++GFxFieldFontFallback;
    else                                            ++GFxFieldFontNone;

    static char Seen[128][224];
    static unsigned int SeenCount = 0;
    char sample[40];
    unsigned int n = 0;
    for (; text && text[n] && n < sizeof(sample) - 1; ++n)
        sample[n] = (text[n] >= 32 && text[n] < 127) ? text[n] : '.';
    sample[n] = 0;
    char line[224];
    _snprintf(line, sizeof(line), "id %u '%s' -> '%s' (%s) text '%s'", fontId,
              wanted ? wanted : "", (got && got->GetName()) ? got->GetName() : "<none>", route, sample);
    line[sizeof(line) - 1] = 0;
    for (unsigned int i = 0; i < SeenCount; ++i)
        if (strcmp(Seen[i], line) == 0)
            return;
    if (SeenCount < 128)
    {
        strncpy(Seen[SeenCount], line, sizeof(Seen[0]) - 1);
        Seen[SeenCount][sizeof(Seen[0]) - 1] = 0;
        ++SeenCount;
    }
    GFxLogf("DISHONORED(bringup): GFx field font: %s", line);
    GFxLogf("DISHONORED(bringup): text field font census: %u fields, %u by html face, %u by font id, "
            "%u fell back to font 0, %u with no font",
            GFxFieldFontTotal, GFxFieldFontByFace, GFxFieldFontById, GFxFieldFontFallback,
            GFxFieldFontNone);
}

// The first `<font face="...">` of an HTML string, which is what a cooked Dishonored text field
// carries. Retail applies it in GFxStyledText::ParseHtmlImpl (2013 rva 0xa9d870, 6,268 bytes; the
// 2012 rva the rest of this file cites for it is 0xaa7630); that parser is not ported
// (SetTextValue strips the markup), so the one attribute that decides which of the two menu fonts a
// field gets is read here instead. Returns false when the string has no <font face>.
static bool GFxHtmlFirstFontFace(const char* html, char* out, unsigned int outSize)
{
    if (!html || !out || outSize == 0)
        return false;
    out[0] = 0;
    for (unsigned int i = 0; html[i]; ++i)
    {
        if (html[i] != '<')
            continue;
        unsigned int t = i + 1;
        while (html[t] == ' ')
            ++t;
        if (!((html[t] == 'f' || html[t] == 'F') && (html[t + 1] == 'o' || html[t + 1] == 'O') &&
              (html[t + 2] == 'n' || html[t + 2] == 'N') && (html[t + 3] == 't' || html[t + 3] == 'T')))
            continue;
        // inside this tag, up to '>', find face=" ... "
        for (unsigned int a = t + 4; html[a] && html[a] != '>'; ++a)
        {
            if (!((html[a] == 'f' || html[a] == 'F') && (html[a + 1] == 'a' || html[a + 1] == 'A') &&
                  (html[a + 2] == 'c' || html[a + 2] == 'C') && (html[a + 3] == 'e' || html[a + 3] == 'E')))
                continue;
            unsigned int v = a + 4;
            while (html[v] == ' ')
                ++v;
            if (html[v] != '=')
                continue;
            ++v;
            while (html[v] == ' ')
                ++v;
            const char quote = (html[v] == '"' || html[v] == '\'') ? html[v] : 0;
            if (quote)
                ++v;
            unsigned int n = 0;
            while (html[v] && n + 1 < outSize &&
                   (quote ? html[v] != quote : (html[v] != ' ' && html[v] != '>')))
                out[n++] = html[v++];
            out[n] = 0;
            return n != 0;
        }
    }
    return false;
}

void GFxEditTextCharacter::GetInitialFormats(GFxTextFormat* fmt, GFxTextParagraphFormat* pfmt)
{
    // DISHONORED(port): 2012 rva 0xa27860, 2013 rva 0xa1df60 - the default values first, then the
    // definition's font, size, colour, alignment and layout on top.
    fmt->InitByDefaultValues();
    pfmt->InitByDefaultValues();

    GFxFontManager* fonts = GFxTextGetFontManager();
    GFxFontResource* res = 0;
    const char* route = "no font id";

    // The HTML face comes first, because it is what retail's parser applies to the run and it is the
    // only source that is right for every field: the DefineEditText font id resolves through the
    // movie's export table, which answers nothing for some ids in the cook and an unrelated symbol
    // for others (measured: id 220 and id 10 give no name at all, id 4 gives
    // "m_nGame_bkgdMenu -nopack"). Every one of those fields then took the font-0 fallback, which is
    // ChaletComprime-CologneEighty - and that is exactly the "wrong font" on the header of every
    // screen whose header asks for $TitleFont.
    char face[96];
    if (fonts && !GFxTextNoHtmlFace && Desc.HasFlag(GFxTextFieldDesc::ETF_Html) &&
        GFxHtmlFirstFontFace(Desc.InitialText, face, sizeof(face)))
    {
        res = fonts->FindFontResource(face, 0);
        route = res ? "html face" : "html face not registered";
    }

    if (res == 0 && Desc.FontId && fonts)
    {
        // DISHONORED(port): 2012 0xa27860 / 2013 0xa1df60 resolves the font id against the movie's
        // resource binding. The name the definition's movie gives that id is carried in the
        // descriptor; the manager matches it against both a DefineFont's own face name and the
        // export symbol an ImportAssets2 bound it under, which is the pair retail's GFxFontLib
        // entry holds.
        if (Desc.FontName[0] != 0)
        {
            res = fonts->FindFontResource(Desc.FontName, 0);
            route = res ? "by name" : "name not registered";
        }
        else
        {
            route = "no font name for the id";
        }
        if (res == 0 && fonts->GetFontCount())
        {
            res = fonts->GetFontByIndex(0);
            route = (Desc.FontName[0] != 0) ? "FALLBACK after name miss" : "FALLBACK, no name";
        }
    }
    GFxEditTextNoteFontResolution(Desc.FontId, Desc.FontName, res, route, Desc.InitialText);
    if (res)
    {
        fmt->SetFontName(res->GetName());
        if (!Desc.HasFlag(GFxTextFieldDesc::ETF_UseDeviceFont))
        {
            const unsigned int ff = res->GetFontFlags();
            fmt->SetBold((ff & GFxFont::FF_Bold) != 0);
            fmt->SetItalic((ff & GFxFont::FF_Italic) != 0);
            if ((ff & GFxFont::FF_DeviceFont) == 0)
            {
                GPtr<GFxFontHandle> h = new GFxFontHandle(fonts, res, res->GetName());
                fmt->SetFontHandle(h.GetPtr());
            }
        }
    }

    // The definition's FontHeight is already in twips, so it goes straight into the format's own twip
    // field rather than through SetFontSize's multiply.
    fmt->SetFontSize(Desc.FontHeightTwips / 20.0f);
    fmt->SetColor(Desc.TextColor);

    // The SWF Align byte remapped to AlignType, which is the mapping GFxText.h documents.
    switch (Desc.Align)
    {
    case 0:  pfmt->SetAlignment(GFxTextParagraphFormat::Align_Left);    break;
    case 1:  pfmt->SetAlignment(GFxTextParagraphFormat::Align_Right);   break;
    case 2:  pfmt->SetAlignment(GFxTextParagraphFormat::Align_Center);  break;
    case 3:  pfmt->SetAlignment(GFxTextParagraphFormat::Align_Justify); break;
    default: break;
    }

    if (Desc.HasFlag(GFxTextFieldDesc::ETF_HasLayout))
    {
        // Retail divides each of the four by 20 on the way in: the paragraph format's margins,
        // indent and leading are in pixels.
        pfmt->SetLeftMargin((unsigned short)(int)(Desc.LeftMarginTwips / 20.0f));
        pfmt->SetRightMargin((unsigned short)(int)(Desc.RightMarginTwips / 20.0f));
        pfmt->SetIndent((short)(int)(Desc.IndentTwips / 20.0f));
        pfmt->SetLeading((short)(int)(Desc.LeadingTwips / 20.0f));
    }
}

void GFxEditTextCharacter::SetInitialFormatsAsDefault()
{
    // DISHONORED(port): 0xa28460
    GFxTextFormat fmt;
    GFxTextParagraphFormat pfmt;
    GetInitialFormats(&fmt, &pfmt);
    Doc.SetDefaultTextAndParaFormat(fmt, pfmt);
    bDirty = true;
}

void GFxEditTextCharacter::SetTextValue(const char* s, bool html, bool notifyVariable)
{
    // DISHONORED(port): 2012 rva 0xa32e50, 2013 rva 0xa293e0 (1,529 bytes). The HTML branch runs
    // GFxStyledText::ParseHtmlImpl (2012 0xaa7630 / 2013 0xa9d870), which is not ported - the markup
    // is stripped instead and the report says so.
    (void)notifyVariable;
    if (!s)
        s = "";
    unsigned int i = 0;
    if (html)
    {
        // The stand-in for the HTML parser: drop the tags, keep the text, decode the four entities
        // the cook uses. Documented as a stub at the site.
        bool inTag = false;
        for (unsigned int j = 0; s[j] && i < sizeof(TextValue) - 1; ++j)
        {
            const char c = s[j];
            if (c == '<') { inTag = true; continue; }
            if (c == '>') { inTag = false; continue; }
            if (inTag)
                continue;
            if (c == '&')
            {
                if (strncmp(s + j, "&amp;", 5) == 0)      { TextValue[i++] = '&'; j += 4; continue; }
                if (strncmp(s + j, "&lt;", 4) == 0)       { TextValue[i++] = '<'; j += 3; continue; }
                if (strncmp(s + j, "&gt;", 4) == 0)       { TextValue[i++] = '>'; j += 3; continue; }
                if (strncmp(s + j, "&quot;", 6) == 0)     { TextValue[i++] = '"'; j += 5; continue; }
                if (strncmp(s + j, "&nbsp;", 6) == 0)     { TextValue[i++] = ' '; j += 5; continue; }
            }
            TextValue[i++] = c;
        }
    }
    else
    {
        for (; s[i] && i < sizeof(TextValue) - 1; ++i)
            TextValue[i] = s[i];
    }
    TextValue[i] = 0;

    // The markup is gone, but the `<font face>` in it still decides the font, so it is applied to the
    // default format here as well as at load: a field whose text is replaced from UnrealScript gets a
    // fresh HTML string and retail's parser would re-read the face from it.
    if (html)
    {
        char face[96];
        GFxFontManager* fonts = GFxTextGetFontManager();
        if (fonts && !GFxTextNoHtmlFace && GFxHtmlFirstFontFace(s, face, sizeof(face)))
        {
            GFxFontResource* res = fonts->FindFontResource(face, 0);
            const GFxTextFormat& current = Doc.GetDefaultTextFormat();
            if (res && res->GetName() &&
                (current.GetFontList() == 0 || strcmp(current.GetFontList(), res->GetName()) != 0))
            {
                GFxTextFormat fmt = current;
                fmt.SetFontName(res->GetName());
                if (!Desc.HasFlag(GFxTextFieldDesc::ETF_UseDeviceFont) &&
                    (res->GetFontFlags() & GFxFont::FF_DeviceFont) == 0)
                {
                    GPtr<GFxFontHandle> h = new GFxFontHandle(fonts, res, res->GetName());
                    fmt.SetFontHandle(h.GetPtr());
                }
                // the paragraph format is copied out first: the setter merges its argument into
                // the member, and passing the member itself would alias it
                const GFxTextParagraphFormat pfmt = Doc.GetDefaultParagraphFormat();
                Doc.SetDefaultTextAndParaFormat(fmt, pfmt);
            }
        }
    }

    Doc.SetText(TextValue);
    bDirty = true;
}

void GFxEditTextCharacter::AdvanceFrame(bool bAdvance, float framePos)
{
    // DISHONORED(port): 0xa2e250 -> CheckAdvanceStatus 0xa2d0b0. A text field's frame advance is a
    // reformat when it is dirty plus the caret blink, which needs the editor kit and is not here.
    (void)bAdvance;
    (void)framePos;
    if (bDirty || (Doc.GetFlags() & GFxTextDocView::VF_NeedsFormat) != 0)
    {
        Doc.Format();
        bDirty = false;
    }
}

void GFxEditTextCharacter::OnEventLoad()
{
    // DISHONORED(port): 0xa30cd0 - the field formats once on load, which is what makes textWidth
    // correct before the first advance.
    AdvanceFrame(false, 0.0f);
}

void GFxEditTextCharacter::ProduceGlyphs(GFxGlyphRasterCache* cache, GlyphOutput* out)
{
    // DISHONORED(port): the traversal of GFxTextLineBuffer::Display (0xa45bf0), with the GRenderer
    // submission left to package CC. The per-glyph transform retail builds is
    //   translate(line.OffsetX + penX, line.OffsetY + line.BaselineOffset) * scale(size / 1024)
    // in twips; the pen advances by each entry's advance.
    if (out)
    {
        out->Lines = 0;
        out->Glyphs = 0;
        out->Rasterized = 0;
        out->Blank = 0;
        out->Missing = 0;
        out->CoveredPixels = 0;
    }
    if (bDirty)
        AdvanceFrame(false, 0.0f);

    GFxTextLineBuffer& lines = Doc.GetLineBuffer();
    for (unsigned int li = 0; li < lines.GetLineCount(); ++li)
    {
        GFxTextLineBuffer::Line* line = lines.GetLine(li);
        if (!line)
            continue;
        if (out)
            ++out->Lines;
        for (unsigned int gi = 0; gi < line->Glyphs.GetSize(); ++gi)
        {
            const GFxTextLineBuffer::GlyphEntry& g = line->Glyphs[gi];
            if (g.Flags & GFxTextLineBuffer::GlyphEntry::EF_Newline)
                continue;
            if (out)
                ++out->Glyphs;
            if (g.GlyphIndex == ~0u)
            {
                if (out)
                    ++out->Missing;
                continue;
            }
            GFxFontResource* font = 0;
            if (g.FormatIndex < line->Formats.GetSize())
                font = line->Formats[g.FormatIndex].pFont;
            if (!font || !cache)
                continue;

            GFxGlyphParam p;
            p.pFont = font;
            p.GlyphIndex = g.GlyphIndex;
            const float sizePx = g.GetFontSize();
            p.FontSize = (unsigned char)(sizePx < 1.0f ? 1.0f : (sizePx > 255.0f ? 255.0f : sizePx));
            const GFxGlyphNode* node = cache->GetGlyph(p);
            if (!node)
                continue;
            if (node->Width && node->Height)
            {
                if (out)
                {
                    ++out->Rasterized;
                    out->CoveredPixels += node->Width * node->Height;
                }
            }
            else if (out)
            {
                ++out->Blank;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------
// The AS2 member surface. Retail's GetMember is 4,401 bytes and SetMember 6,232; this is the subset
// the cook's own content reads and writes, which the harness reports by name.

// ---------------------------------------------------------------------------------------------
// GFxTextFilter

// DISHONORED(port): 2013 0xa24420. The defaults are a 45-degree shadow four pixels away, which is
// what Flash's own filter panel starts at; nothing draws until LoadFilterDesc has put a colour in.
GFxTextFilter::GFxTextFilter()
    : BlurX(0), BlurY(0), BlurStrength(16), ShadowFlags(0x80), ShadowBlurX(64), ShadowBlurY(64),
      ShadowStrength(16), ShadowAlpha(255), GlowSize(0), ShadowAngle(450), ShadowDistance(80),
      ShadowOffsetX(57), ShadowOffsetY(57), ShadowColor(0u), GlowColor(0u)
{
}

// DISHONORED(port): 2013 0xa24480.
unsigned char GFxTextFilter::FloatToFixed44(float v)
{
    const unsigned int q = (unsigned int)(long long)(v * 16.0f + 0.5f);
    return q >= 0xFF ? (unsigned char)0xFF : (unsigned char)q;
}

// DISHONORED(port): 2013 0xa22f30. Angle in tenths of a degree, distance in twips, offset in twips.
void GFxTextFilter::UpdateShadowOffset()
{
    const float radians = (float)((double)ShadowAngle * 3.141592741012573 / 1800.0);
    const float distance = (float)ShadowDistance;
    ShadowOffsetX = (short)(int)((float)cos(radians) * distance);
    ShadowOffsetY = (short)(int)((float)sin(radians) * distance);
}

// DISHONORED(port): 2013 0xa89910, branch for branch. A Blur record fills the blur fields; a
// DropShadow or a Glow fills the shadow block, but only the *first* one does - once a colour and a
// distance are set, a following Glow contributes its colour and its size and nothing else, which is
// how retail keeps three filters on one field from fighting.
void GFxTextFilter::LoadFilterDesc(const GFxFilterDesc& desc)
{
    const unsigned int kind = desc.GetFilterType();
    if (kind == GFxFilterDesc::FT_Blur)
    {
        BlurX = FloatToFixed44(desc.Params.BlurX);
        BlurY = FloatToFixed44(desc.Params.BlurY);
        BlurStrength = FloatToFixed44(desc.Params.Strength);
        return;
    }
    if (kind != GFxFilterDesc::FT_DropShadow && kind != GFxFilterDesc::FT_Glow)
        return;
    if (ShadowColor.Raw != 0 && ShadowDistance != 0)
    {
        if (kind == GFxFilterDesc::FT_Glow)
        {
            GlowColor = desc.Params.Color;
            GlowSize = ShadowBlurY < ShadowBlurX ? ShadowBlurX : ShadowBlurY;
        }
        return;
    }
    ShadowFlags = (unsigned char)(desc.Filter & 0xF0);
    ShadowBlurX = FloatToFixed44(desc.Params.BlurX);
    ShadowBlurY = FloatToFixed44(desc.Params.BlurY);
    ShadowStrength = FloatToFixed44(desc.Params.Strength);
    ShadowAlpha = desc.Params.Color.Channels.Alpha;
    ShadowAngle = desc.Angle;
    ShadowDistance = desc.Distance;
    ShadowOffsetX = 0;
    ShadowOffsetY = 0;
    ShadowColor = desc.Params.Color;
    UpdateShadowOffset();
}

// DISHONORED(port): 2013 0xa275b0.
void GFxEditTextCharacter::SetFilters(const GFxFilterDesc* filters, unsigned int count)
{
    GFxTextFilter built;
    for (unsigned int i = 0; i < count; ++i)
    {
        built.LoadFilterDesc(filters[i]);
        // The pass count is what the composite costs: a blurred glyph is rasterised once per pass.
        unsigned int passes = filters[i].Params.Passes;
        if (passes == 0)
            passes = 1;
        GFxDL_NoteFilterApplied(filters[i], passes, GetName().ToCStr(), TextValue);
    }
    Filter = built;
    SetDirtyFlag();
}

GRect<float> GFxEditTextCharacter::GetBoundsTwips(const GMatrix2D& m) const
{
    // DISHONORED(port): 0xa2ed60 - the view rect through the matrix, and nothing else. The
    // definition's own rect never enters into it, which is the whole difference for a field that
    // was auto-sized or had its text replaced.
    const GRect<float> r = const_cast<GFxEditTextCharacter*>(this)->Doc.GetViewRect();
    if (r.Right <= r.Left && r.Bottom <= r.Top)
        return GRect<float>(0.f, 0.f, 0.f, 0.f);
    const float xs[4] = { r.Left, r.Right, r.Left, r.Right };
    const float ys[4] = { r.Top, r.Top, r.Bottom, r.Bottom };
    GRect<float> out(0.f, 0.f, 0.f, 0.f);
    for (int i = 0; i < 4; ++i)
    {
        float x = xs[i], y = ys[i];
        m.Transform(&x, &y);
        if (i == 0)
        {
            out.Left = out.Right = x;
            out.Top = out.Bottom = y;
        }
        else
        {
            if (x < out.Left) out.Left = x;
            if (x > out.Right) out.Right = x;
            if (y < out.Top) out.Top = y;
            if (y > out.Bottom) out.Bottom = y;
        }
    }
    return out;
}

bool GFxEditTextCharacter::GetMember(GASEnvironment* env, const GASString& name, GASValue* val)
{
    // DISHONORED(port): 0xa2f9f0, the properties named below only.
    const char* n = name.ToCStr();
    if (strcmp(n, "text") == 0 || strcmp(n, "htmlText") == 0)
    {
        if (pMovieRoot)
            val->SetString(pMovieRoot->CreateString(TextValue));
        else
            val->SetUndefined();
        return true;
    }
    if (strcmp(n, "length") == 0)
    {
        val->SetInt((int)Doc.GetStyledText().GetLength());
        return true;
    }
    if (strcmp(n, "textWidth") == 0)
    {
        val->SetNumber((double)(Doc.GetTextWidth() / 20.0f));
        return true;
    }
    if (strcmp(n, "textHeight") == 0)
    {
        val->SetNumber((double)(Doc.GetTextHeight() / 20.0f));
        return true;
    }
    if (strcmp(n, "numLines") == 0)
    {
        val->SetInt((int)Doc.GetLinesCount());
        return true;
    }
    if (strcmp(n, "wordWrap") == 0)
    {
        val->SetBool((Doc.GetFlags() & GFxTextDocView::VF_WordWrap) != 0);
        return true;
    }
    if (strcmp(n, "multiline") == 0)
    {
        val->SetBool((Doc.GetFlags() & GFxTextDocView::VF_Multiline) != 0);
        return true;
    }
    if (strcmp(n, "selectable") == 0)
    {
        val->SetBool((Doc.GetFlags() & GFxTextDocView::VF_Selectable) != 0);
        return true;
    }
    if (strcmp(n, "html") == 0)
    {
        val->SetBool((Doc.GetFlags() & GFxTextDocView::VF_Html) != 0);
        return true;
    }
    if (strcmp(n, "embedFonts") == 0)
    {
        val->SetBool((Doc.GetFlags() & GFxTextDocView::VF_UseDeviceFont) == 0);
        return true;
    }
    if (strcmp(n, "textColor") == 0)
    {
        val->SetInt((int)(Doc.GetDefaultTextFormat().GetColor().GetRaw() & 0x00FFFFFF));
        return true;
    }
    if (strcmp(n, "autoSize") == 0)
    {
        const unsigned int f = Doc.GetFlags();
        const char* s = "none";
        if (f & (GFxTextDocView::VF_AutoSizeX | GFxTextDocView::VF_AutoSizeY))
            s = "left";
        if (pMovieRoot)
            val->SetString(pMovieRoot->CreateString(s));
        return true;
    }
    // DISHONORED(port, agent EA): the AS2 TextField prototype - getTextFormat and whatever else
    // this cook asks a field for. Retail resolves it through GASTextFieldObject's own prototype
    // chain (GASTextFieldProto 2013 0xa21120); the field has no AS2 object of its own here, so the
    // prototype is consulted directly, in the same place the chain would have been.
    {
        GASObject* proto = GFxDrawingTextFieldProto();
        if (proto != 0 && pMovieRoot != 0
            && proto->GetMemberRaw(pMovieRoot->GetASContext()->GetSC(), name, val))
        {
            return true;
        }
    }
    return GFxASCharacter::GetMember(env, name, val);
}

bool GFxEditTextCharacter::SetMember(GASEnvironment* env, const GASString& name,
                                     const GASValue& val, const GASPropFlags& flags)
{
    // DISHONORED(port): 0xa33450, the properties named below only.
    const char* n = name.ToCStr();
    if (strcmp(n, "text") == 0 || strcmp(n, "htmlText") == 0)
    {
        GASValue tmp(val);
        GASString s = tmp.ToString(env);
        SetTextValue(s.ToCStr(), strcmp(n, "htmlText") == 0, true);
        return true;
    }
    if (strcmp(n, "wordWrap") == 0)
    {
        GASValue tmp(val);
        if (tmp.ToBool(env))
            Doc.SetWordWrap();
        else
            Doc.ClearWordWrap();
        bDirty = true;
        return true;
    }
    if (strcmp(n, "multiline") == 0)
    {
        GASValue tmp(val);
        Doc.SetMultiline(tmp.ToBool(env));
        bDirty = true;
        return true;
    }
    if (strcmp(n, "autoSize") == 0)
    {
        GASValue tmp(val);
        GASString s = tmp.ToString(env);
        const char* v = s.ToCStr();
        unsigned int f = Doc.GetFlags() &
            ~(unsigned int)(GFxTextDocView::VF_AutoSizeX | GFxTextDocView::VF_AutoSizeY);
        if (strcmp(v, "none") != 0 && strcmp(v, "false") != 0)
        {
            if (f & (GFxTextDocView::VF_WordWrap | GFxTextDocView::VF_Multiline))
                f |= GFxTextDocView::VF_AutoSizeY;
            else
                f |= GFxTextDocView::VF_AutoSizeX;
        }
        Doc.SetFlags(f);
        bDirty = true;
        return true;
    }
    if (strcmp(n, "textColor") == 0)
    {
        GASValue tmp(val);
        GFxTextFormat fmt;
        const unsigned int rgb = (unsigned int)tmp.ToInt32(env) & 0x00FFFFFFu;
        fmt.SetColor(GColor(0xFF000000u | rgb));
        // DISHONORED(port): the colour is merged into the document's own default, not substituted
        // for it - the same rule 0xaa5400 applies to a run. Replacing it left the default with no
        // font name and no size.
        const GFxTextFormat merged = Doc.GetStyledText().GetDefaultTextFormat().Merge(fmt);
        Doc.SetDefaultTextAndParaFormat(merged, Doc.GetStyledText().GetDefaultParagraphFormat());
        Doc.SetTextFormat(fmt, 0, Doc.GetStyledText().GetLength());
        bDirty = true;
        return true;
    }
    if (strcmp(n, "embedFonts") == 0)
    {
        GASValue tmp(val);
        unsigned int f = Doc.GetFlags();
        if (tmp.ToBool(env))
            f &= ~(unsigned int)GFxTextDocView::VF_UseDeviceFont;
        else
            f |= GFxTextDocView::VF_UseDeviceFont;
        Doc.SetFlags(f);
        bDirty = true;
        return true;
    }
    return GFxASCharacter::SetMember(env, name, val, flags);
}
