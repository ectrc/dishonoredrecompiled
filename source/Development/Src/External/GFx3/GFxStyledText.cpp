// GFx 3.3 styled text - the paragraph with its format runs, and the document a text field holds.
#include "GFxText.h"

#include <string.h>

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

unsigned int GFxTextUtf8ToWide(const char* s, unsigned int bytes, GArray<wchar_t>* out)
{
    // Retail routes this through GFxWStringBuffer and GUTF8Util::DecodeNextChar. The cook's own
    // strings are ASCII plus the odd Latin-1 byte, and the localised packages are UTF-8, so the two
    // and three byte forms are decoded and anything longer is replaced, which is what the SDK's
    // decoder does with an ill-formed sequence.
    out->Clear();
    if (!s)
        return 0;
    unsigned int i = 0;
    while (i < bytes && s[i])
    {
        const unsigned char c = (unsigned char)s[i];
        if (c < 0x80)
        {
            out->PushBack((wchar_t)c);
            ++i;
        }
        else if ((c & 0xE0) == 0xC0 && i + 1 < bytes)
        {
            out->PushBack((wchar_t)(((c & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F)));
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0 && i + 2 < bytes)
        {
            out->PushBack((wchar_t)(((c & 0x0F) << 12) | (((unsigned char)s[i + 1] & 0x3F) << 6) |
                                    ((unsigned char)s[i + 2] & 0x3F)));
            i += 3;
        }
        else
        {
            out->PushBack((wchar_t)c);
            ++i;
        }
    }
    return out->GetSize();
}

unsigned int GFxTextWideToUtf8(const wchar_t* s, unsigned int len, char* out, unsigned int outBytes)
{
    // The encoder half of retail's GString::operator=(const wchar_t*) (2013 rva 0x9b5390): one byte
    // below 0x80, two below 0x800, three otherwise, and a surrogate pair folded into one four-byte
    // sequence. A sequence that does not fit stops the loop whole, so the result is never a partial
    // character.
    if (!out || outBytes == 0)
        return 0;
    unsigned int n = 0;
    for (unsigned int i = 0; i < len && s && s[i]; ++i)
    {
        unsigned int c = (unsigned int)(unsigned short)s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len)
        {
            const unsigned int lo = (unsigned int)(unsigned short)s[i + 1];
            if (lo >= 0xDC00 && lo <= 0xDFFF)
            {
                c = 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
                ++i;
            }
        }
        const unsigned int need = c < 0x80 ? 1u : (c < 0x800 ? 2u : (c < 0x10000 ? 3u : 4u));
        if (n + need + 1 > outBytes)
            break;
        switch (need)
        {
        case 1:
            out[n++] = (char)c;
            break;
        case 2:
            out[n++] = (char)(0xC0 | (c >> 6));
            out[n++] = (char)(0x80 | (c & 0x3F));
            break;
        case 3:
            out[n++] = (char)(0xE0 | (c >> 12));
            out[n++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[n++] = (char)(0x80 | (c & 0x3F));
            break;
        default:
            out[n++] = (char)(0xF0 | (c >> 18));
            out[n++] = (char)(0x80 | ((c >> 12) & 0x3F));
            out[n++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[n++] = (char)(0x80 | (c & 0x3F));
            break;
        }
    }
    out[n] = 0;
    return n;
}

// =============================================================================================
// GFxTextParagraph
// =============================================================================================

GFxTextParagraph::GFxTextParagraph(GFxTextAllocator* alloc)
    : StartIndex(0), pAllocator(alloc), pFormat(0)
{
    // DISHONORED(port): 0xaa4650
}

GFxTextParagraph::~GFxTextParagraph()
{
}

void GFxTextParagraph::Clear()
{
    // DISHONORED(port): 0xaa49a0
    Text.Clear();
    Runs.Clear();
}

bool GFxTextParagraph::HasNewLine() const
{
    // DISHONORED(port): 0xaa0990 - a paragraph owns its terminating newline, which is what makes the
    // line buffer able to answer "does this line end in a break".
    const unsigned int n = Text.GetSize();
    if (!n)
        return false;
    const wchar_t c = Text[n - 1];
    return c == L'\n' || c == L'\r';
}

void GFxTextParagraph::SetText(const wchar_t* s, unsigned int len)
{
    // DISHONORED(port): 0xaa0f40 -> TextBuffer::SetString 0xaa0ec0
    Text.Clear();
    for (unsigned int i = 0; i < len; ++i)
        Text.PushBack(s[i]);
    Runs.Clear();
}

void GFxTextParagraph::InsertString(const wchar_t* s, unsigned int at, unsigned int len,
                                    const GFxTextFormat* fmt)
{
    // DISHONORED(port): 0xaa55d0 -> TextBuffer::CreatePosition 0xaa0910
    if (at > Text.GetSize())
        at = Text.GetSize();
    const unsigned int oldSize = Text.GetSize();
    Text.Resize(oldSize + len);
    for (unsigned int i = oldSize; i > at; --i)
        Text[i - 1 + len] = Text[i - 1];
    for (unsigned int i = 0; i < len; ++i)
        Text[at + i] = s[i];

    // Shift every run past the insertion point and give the inserted range its own run.
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        if (Runs[i].Index >= at)
            Runs[i].Index += len;
        else if (Runs[i].Index + Runs[i].Length > at)
            Runs[i].Length += len;
    }
    if (fmt && len)
    {
        FormatRun r;
        r.Index = at;
        r.Length = len;
        r.pFormat = fmt;
        Runs.PushBack(r);
        normalizeRuns();
    }
}

void GFxTextParagraph::Remove(unsigned int at, unsigned int len)
{
    // DISHONORED(port): 0xaa51a0
    if (at >= Text.GetSize())
        return;
    if (at + len > Text.GetSize())
        len = Text.GetSize() - at;
    for (unsigned int i = at; i + len < Text.GetSize(); ++i)
        Text[i] = Text[i + len];
    Text.Resize(Text.GetSize() - len);

    GArray<FormatRun> kept;
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        FormatRun r = Runs[i];
        const unsigned int end = r.Index + r.Length;
        if (end <= at)
        {
            kept.PushBack(r);
        }
        else if (r.Index >= at + len)
        {
            r.Index -= len;
            kept.PushBack(r);
        }
        else
        {
            const unsigned int lo = r.Index < at ? at - r.Index : 0;
            const unsigned int hi = end > at + len ? end - (at + len) : 0;
            if (lo + hi)
            {
                r.Index = r.Index < at ? r.Index : at;
                r.Length = lo + hi;
                kept.PushBack(r);
            }
        }
    }
    GFxTextCopyArray(&Runs, kept);
}

void GFxTextParagraph::SetFormat(const GFxTextParagraphFormat& f)
{
    // DISHONORED(port): 0xaa14e0
    pFormat = pAllocator ? pAllocator->AllocateParagraphFormat(f) : 0;
}

void GFxTextParagraph::SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len)
{
    // DISHONORED(port): 0xaa5400 - each run the range covers gets `existing.Merge(f)`, which is
    // what the decompile does (GFxTextFormat::Merge feeding GFxTextAllocator::AllocateTextFormat),
    // so only the fields `f` marks present override: a colour-only format leaves the font and the
    // size where they were. Overlapping runs are split at the range's ends.
    if (!pAllocator || !len)
        return;
    if (at >= Text.GetSize())
        return;
    if (at + len > Text.GetSize())
        len = Text.GetSize() - at;

    const unsigned int lo = at, hi = at + len;
    unsigned int covered = 0;
    GArray<FormatRun> next;
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        const FormatRun& r = Runs[i];
        const unsigned int end = r.Index + r.Length;
        if (end <= lo || r.Index >= hi)
        {
            next.PushBack(r);
            continue;
        }
        if (r.Index < lo)
        {
            FormatRun head = r;
            head.Length = lo - r.Index;
            next.PushBack(head);
        }
        FormatRun mid;
        mid.Index = r.Index > lo ? r.Index : lo;
        mid.Length = (end < hi ? end : hi) - mid.Index;
        const GFxTextFormat base = r.pFormat ? *r.pFormat : GFxTextFormat();
        mid.pFormat = pAllocator->AllocateTextFormat(base.Merge(f));
        next.PushBack(mid);
        covered += mid.Length;
        if (end > hi)
        {
            FormatRun tail = r;
            tail.Index = hi;
            tail.Length = end - hi;
            next.PushBack(tail);
        }
    }
    if (covered == 0)
    {
        // Nothing carried a format over that range yet, so the format itself is what it gets.
        FormatRun mid;
        mid.Index = lo;
        mid.Length = len;
        mid.pFormat = pAllocator->AllocateTextFormat(f);
        next.PushBack(mid);
    }
    GFxTextCopyArray(&Runs, next);
    normalizeRuns();
}

void GFxTextParagraph::normalizeRuns()
{
    // Sort by index and merge neighbours that share a format pointer. Retail's GRangeDataArray keeps
    // the invariant by construction; this restores it after a batch edit.
    for (unsigned int i = 1; i < Runs.GetSize(); ++i)
    {
        FormatRun key = Runs[i];
        unsigned int j = i;
        while (j > 0 && Runs[j - 1].Index > key.Index)
        {
            Runs[j] = Runs[j - 1];
            --j;
        }
        Runs[j] = key;
    }
    GArray<FormatRun> merged;
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        if (!Runs[i].Length)
            continue;
        if (merged.GetSize())
        {
            FormatRun& last = merged[merged.GetSize() - 1];
            if (last.pFormat == Runs[i].pFormat && last.Index + last.Length == Runs[i].Index)
            {
                last.Length += Runs[i].Length;
                continue;
            }
        }
        merged.PushBack(Runs[i]);
    }
    GFxTextCopyArray(&Runs, merged);
}

const GFxTextFormat* GFxTextParagraph::GetTextFormatPtr(unsigned int at) const
{
    // DISHONORED(port): 0xaa3540
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        if (at >= Runs[i].Index && at < Runs[i].Index + Runs[i].Length)
            return Runs[i].pFormat;
    }
    return 0;
}

// =============================================================================================
// GFxStyledText
// =============================================================================================

GFxStyledText::GFxStyledText(GFxTextAllocator* alloc)
    : pAllocator(alloc), ImageTagsDropped(0)
{
    // DISHONORED(port): 0xaa6ee0 - both default formats start at their default values, which is what
    // makes a text field with no format at all still lay out.
    DefaultTextFormat.InitByDefaultValues();
    DefaultParagraphFormat.InitByDefaultValues();
}

GFxStyledText::~GFxStyledText()
{
    // DISHONORED(port): 0xaa0650
    Clear();
}

void GFxStyledText::Clear()
{
    // DISHONORED(port): 0xaa6d90
    for (unsigned int i = 0; i < Paragraphs.GetSize(); ++i)
        delete Paragraphs[i];
    Paragraphs.Clear();
}

unsigned int GFxStyledText::GetLength() const
{
    // DISHONORED(port): 0xaa1720
    unsigned int n = 0;
    for (unsigned int i = 0; i < Paragraphs.GetSize(); ++i)
        n += Paragraphs[i]->GetLength();
    return n;
}

void GFxStyledText::GetText(GArray<wchar_t>* out) const
{
    // DISHONORED(port): 0xaa17e0
    out->Clear();
    for (unsigned int i = 0; i < Paragraphs.GetSize(); ++i)
    {
        const GFxTextParagraph* p = Paragraphs[i];
        for (unsigned int j = 0; j < p->GetLength(); ++j)
            out->PushBack(p->GetChar(j));
    }
    out->PushBack(0);
}

GFxTextParagraph* GFxStyledText::AppendNewParagraph(const GFxTextParagraphFormat* pfmt)
{
    // DISHONORED(port): 0xaa5af0
    GFxTextParagraph* p = new GFxTextParagraph(pAllocator);
    p->SetFormat(pfmt ? *pfmt : DefaultParagraphFormat);
    Paragraphs.PushBack(p);
    reindex();
    return p;
}

GFxTextParagraph* GFxStyledText::GetLastParagraph()
{
    // DISHONORED(port): 0xaa1560
    if (!Paragraphs.GetSize())
        return 0;
    return Paragraphs[Paragraphs.GetSize() - 1];
}

void GFxStyledText::reindex()
{
    unsigned int at = 0;
    for (unsigned int i = 0; i < Paragraphs.GetSize(); ++i)
    {
        Paragraphs[i]->StartIndex = at;
        at += Paragraphs[i]->GetLength();
    }
}

unsigned int GFxStyledText::AppendString(const wchar_t* s, unsigned int len, NewLinePolicy policy,
                                         const GFxTextFormat* fmt,
                                         const GFxTextParagraphFormat* pfmt)
{
    // DISHONORED(port): 0xaa6190 - split on the newline and open a paragraph per line. A CR LF pair
    // counts as one break; the newline itself stays at the end of the paragraph that owns it, which
    // is what GFxTextParagraph::HasNewLine (0xaa0990) reads back.
    if (!s || !len)
        return 0;
    unsigned int written = 0;
    unsigned int i = 0;
    while (i < len)
    {
        unsigned int runEnd = i;
        while (runEnd < len && s[runEnd] != L'\n' && s[runEnd] != L'\r')
            ++runEnd;

        unsigned int breakLen = 0;
        if (runEnd < len)
        {
            if (s[runEnd] == L'\r' && runEnd + 1 < len && s[runEnd + 1] == L'\n')
                breakLen = (policy == NLP_PreserveAll) ? 2u : 1u;
            else
                breakLen = 1;
        }

        GFxTextParagraph* p = GetLastParagraph();
        if (!p)
            p = AppendNewParagraph(pfmt);

        const unsigned int at = p->GetLength();
        const GFxTextFormat* interned = pAllocator
            ? pAllocator->AllocateTextFormat(fmt ? *fmt : DefaultTextFormat) : 0;
        if (runEnd > i)
        {
            p->InsertString(s + i, at, runEnd - i, interned);
            written += runEnd - i;
        }
        if (breakLen)
        {
            static const wchar_t nl[1] = { L'\n' };
            p->InsertString(nl, p->GetLength(), 1, interned);
            ++written;
            AppendNewParagraph(pfmt);
            i = runEnd + ((s[runEnd] == L'\r' && runEnd + 1 < len && s[runEnd + 1] == L'\n') ? 2u : 1u);
        }
        else
        {
            i = runEnd;
        }
    }
    reindex();
    return written;
}

unsigned int GFxStyledText::AppendString(const char* s, unsigned int len, NewLinePolicy policy)
{
    // DISHONORED(port): 0xaa6fc0 / 0xaa5f00
    GArray<wchar_t> wide;
    GFxTextUtf8ToWide(s, len, &wide);
    if (!wide.GetSize())
        return 0;
    return AppendString(&wide[0], wide.GetSize(), policy, 0, 0);
}

void GFxStyledText::SetText(const wchar_t* s, unsigned int len)
{
    // DISHONORED(port): 0xaa7010
    Clear();
    AppendNewParagraph(0);
    if (s && len)
        AppendString(s, len, NLP_PreserveAll, 0, 0);
}

void GFxStyledText::SetText(const char* s)
{
    // DISHONORED(port): 0xaa6fe0
    GArray<wchar_t> wide;
    GFxTextUtf8ToWide(s, s ? (unsigned int)strlen(s) : 0u, &wide);
    SetText(wide.GetSize() ? &wide[0] : L"", wide.GetSize());
}

void GFxStyledText::SetDefaultTextFormat(const GFxTextFormat& f)
{
    // DISHONORED(port): 0xaa47b0 - the default is merged into, not replaced by, the argument, so a
    // partial format (a <font> tag's, or TextField.setNewTextFormat's) keeps the rest.
    DefaultTextFormat = DefaultTextFormat.Merge(f);
}

void GFxStyledText::SetDefaultParagraphFormat(const GFxTextParagraphFormat& f)
{
    // DISHONORED(port): 0xaa4890
    DefaultParagraphFormat = DefaultParagraphFormat.Merge(f);
}

void GFxStyledText::SetTextFormat(const GFxTextFormat& f, unsigned int at, unsigned int len)
{
    // DISHONORED(port): 0xaa5730 - the range is in *document* coordinates, so it is split across the
    // paragraphs it spans.
    for (unsigned int i = 0; i < Paragraphs.GetSize() && len; ++i)
    {
        GFxTextParagraph* p = Paragraphs[i];
        const unsigned int pStart = p->StartIndex;
        const unsigned int pLen = p->GetLength();
        if (at >= pStart + pLen)
            continue;
        const unsigned int lo = at > pStart ? at - pStart : 0;
        unsigned int n = pLen - lo;
        if (n > len)
            n = len;
        p->SetTextFormat(f, lo, n);
        at += n;
        len -= n;
    }
}

void GFxStyledText::SetParagraphFormat(const GFxTextParagraphFormat& f, unsigned int at,
                                       unsigned int len)
{
    // DISHONORED(port): 0xaa1670 - a paragraph format applies to whole paragraphs, so every paragraph
    // the range touches gets it.
    for (unsigned int i = 0; i < Paragraphs.GetSize(); ++i)
    {
        GFxTextParagraph* p = Paragraphs[i];
        const unsigned int pStart = p->StartIndex;
        const unsigned int pEnd = pStart + p->GetLength();
        if (pEnd <= at || pStart >= at + len)
            continue;
        p->SetFormat(p->GetFormat() ? p->GetFormat()->Merge(f) : f);
    }
}

// =============================================================================================
// The HTML parser
// =============================================================================================
//
// DISHONORED(port, agent FG): GFxStyledText::ParseHtmlImpl<wchar_t> (2013 rva 0xa9d870,
// 0x187c = 6,268 bytes; 2012 rva 0xaa7630), tokenised by GFxSGMLParser<wchar_t>::GetNext
// (2013 rva 0xa988b0), which answers a token kind rather than a tag name - the tag names
// themselves are matched inline in the parser and appear in the image as neither wide nor narrow
// literals.
//
// Three deviations, each because the thing it needs is not in this tree:
//   * retail takes a GArrayDH<HTMLImageTagInfo>* out-parameter and a GFxTextStyleManager*. Neither
//     the image-tag substitution nor the CSS style manager is ported, so <img> is counted and
//     dropped and the style manager has no say;
//   * retail puts the <font face> value straight into the run's font *list* and lets
//     GFxTextDocView::FindFont resolve it, whose font-map route (GFxFontMap::GetFontMapping) is what
//     turns "$TitleFont" into a real family. That route is unported, so the face is resolved here
//     against the font manager the field passes in, exactly as GFxEditTextCharacter::
//     GetInitialFormats already does for the definition's own text (agent FB);
//   * <sbr> and <tab> are not handled. The tag set here is the one a census of the six cooked
//     movies and every DishonoredGame.<lang> file actually contains - font
//     (face/size/color/letterSpacing/kerning), p (align), br, b, i, u, a (href), li, textformat
//     (indent/blockindent/leftmargin/rightmargin/leading) and img.

namespace
{

// Plain fixed buffers on purpose: GArray copy-constructs its elements when it grows and GArray has
// no copy constructor, so an array of anything holding a GArray double-frees on the first reserve.
// That hazard is already recorded at the top of this file and it is what a first cut of this parser
// hit - the build died with STATUS_HEAP_CORRUPTION before it reached the menu.
const unsigned int GFxHtmlMaxAttrs = 16;
const unsigned int GFxHtmlMaxName  = 40;
const unsigned int GFxHtmlMaxValue = 256;

struct GFxHtmlAttr
{
    wchar_t      Name[GFxHtmlMaxName];
    wchar_t      Value[GFxHtmlMaxValue];
    unsigned int NameLen;
    unsigned int ValueLen;
};

bool GFxHtmlIsSpace(wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

wchar_t GFxHtmlLower(wchar_t c)
{
    return (c >= L'A' && c <= L'Z') ? (wchar_t)(c - L'A' + L'a') : c;
}

bool GFxHtmlNameIs(const wchar_t* name, unsigned int len, const char* lit)
{
    unsigned int i = 0;
    for (; lit[i]; ++i)
    {
        if (i >= len || GFxHtmlLower(name[i]) != (wchar_t)lit[i])
            return false;
    }
    return i == len;
}

bool GFxHtmlNameIs(const GArray<wchar_t>& name, const char* lit)
{
    return GFxHtmlNameIs(name.GetSize() ? &name[0] : L"", name.GetSize(), lit);
}

// &amp; &lt; &gt; &quot; &apos; &nbsp; &#NN; &#xNN; - the set GFxSGMLCharIter::DecodeEscapedChar
// (2013 rva 0xa971d0) answers for. An unterminated or unknown entity stays literal text, which is what
// that function leaves behind when it runs out of characters.
bool GFxHtmlDecodeEntity(const wchar_t* s, unsigned int len, unsigned int at, wchar_t* out,
                         unsigned int* consumed)
{
    unsigned int end = at + 1;
    while (end < len && end < at + 12 && s[end] != L';' && s[end] != L'&' && !GFxHtmlIsSpace(s[end]))
        ++end;
    if (end >= len || s[end] != L';')
        return false;
    const unsigned int n = end - at - 1;
    if (n == 0)
        return false;
    const wchar_t* body = s + at + 1;
    if (body[0] == L'#')
    {
        unsigned int value = 0;
        if (n >= 2 && (body[1] == L'x' || body[1] == L'X'))
        {
            for (unsigned int i = 2; i < n; ++i)
            {
                const wchar_t c = GFxHtmlLower(body[i]);
                if (c >= L'0' && c <= L'9')      value = value * 16 + (unsigned int)(c - L'0');
                else if (c >= L'a' && c <= L'f') value = value * 16 + (unsigned int)(c - L'a' + 10);
                else return false;
            }
        }
        else
        {
            for (unsigned int i = 1; i < n; ++i)
            {
                if (body[i] < L'0' || body[i] > L'9')
                    return false;
                value = value * 10 + (unsigned int)(body[i] - L'0');
            }
        }
        *out = (wchar_t)value;
        *consumed = end - at + 1;
        return true;
    }
    static const struct { const char* Name; wchar_t Char; } Named[] = {
        { "amp", L'&' }, { "lt", L'<' }, { "gt", L'>' }, { "quot", L'"' },
        { "apos", L'\'' }, { "nbsp", L' ' }
    };
    for (unsigned int e = 0; e < sizeof(Named) / sizeof(Named[0]); ++e)
    {
        unsigned int i = 0;
        for (; Named[e].Name[i]; ++i)
        {
            if (i >= n || GFxHtmlLower(body[i]) != (wchar_t)Named[e].Name[i])
                break;
        }
        if (Named[e].Name[i] == 0 && i == n)
        {
            *out = Named[e].Char;
            *consumed = end - at + 1;
            return true;
        }
    }
    return false;
}

const GFxHtmlAttr* GFxHtmlFindAttr(const GFxHtmlAttr* attrs, unsigned int count, const char* name)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        if (GFxHtmlNameIs(attrs[i].Name, attrs[i].NameLen, name))
            return &attrs[i];
    }
    return 0;
}

void GFxHtmlAttrToUtf8(const GFxHtmlAttr* a, char* out, unsigned int outSize)
{
    out[0] = 0;
    if (!a || !a->ValueLen)
        return;
    GFxTextWideToUtf8(a->Value, a->ValueLen, out, outSize);
}

bool GFxHtmlAttrToFloat(const GFxHtmlAttr* a, float* out)
{
    if (!a || !a->ValueLen)
        return false;
    const wchar_t* v = a->Value;
    const unsigned int n = a->ValueLen;
    float sign = 1.0f, whole = 0.0f, frac = 0.0f, scale = 0.1f;
    unsigned int i = 0;
    while (i < n && GFxHtmlIsSpace(v[i]))
        ++i;
    if (i < n && (v[i] == L'+' || v[i] == L'-'))
        sign = v[i++] == L'-' ? -1.0f : 1.0f;
    bool any = false;
    for (; i < n && v[i] >= L'0' && v[i] <= L'9'; ++i)
    {
        whole = whole * 10.0f + (float)(v[i] - L'0');
        any = true;
    }
    if (i < n && v[i] == L'.')
    {
        for (++i; i < n && v[i] >= L'0' && v[i] <= L'9'; ++i, scale *= 0.1f)
        {
            frac += (float)(v[i] - L'0') * scale;
            any = true;
        }
    }
    if (!any)
        return false;
    *out = sign * (whole + frac);
    return true;
}

// `#RRGGBB`, `0xRRGGBB` and a bare decimal are the three spellings Flash accepts and the cook writes
// the first. The alpha stays where the inherited format had it: <font color> carries RGB only.
bool GFxHtmlAttrToColor(const GFxHtmlAttr* a, unsigned int* rgb)
{
    if (!a || !a->ValueLen)
        return false;
    const wchar_t* v = a->Value;
    const unsigned int n = a->ValueLen;
    unsigned int i = 0;
    while (i < n && GFxHtmlIsSpace(v[i]))
        ++i;
    bool hex = false;
    if (i < n && v[i] == L'#')
    {
        hex = true;
        ++i;
    }
    else if (i + 1 < n && v[i] == L'0' && GFxHtmlLower(v[i + 1]) == L'x')
    {
        hex = true;
        i += 2;
    }
    unsigned int value = 0;
    bool any = false;
    for (; i < n; ++i)
    {
        const wchar_t c = GFxHtmlLower(v[i]);
        unsigned int digit;
        if (c >= L'0' && c <= L'9')             digit = (unsigned int)(c - L'0');
        else if (hex && c >= L'a' && c <= L'f') digit = (unsigned int)(c - L'a' + 10);
        else break;
        value = value * (hex ? 16u : 10u) + digit;
        any = true;
    }
    if (!any)
        return false;
    *rgb = value & 0xFFFFFFu;
    return true;
}

bool GFxHtmlAttrIsTrue(const GFxHtmlAttr* a)
{
    // Flash reads kerning="1" / "true"; anything else is off.
    return a && a->ValueLen && (a->Value[0] == L'1' || GFxHtmlLower(a->Value[0]) == L't');
}

struct GFxHtmlOpenTag
{
    GArray<wchar_t>        Name;
    GFxTextFormat          Format;
    GFxTextParagraphFormat ParagraphFormat;
};

} // namespace

bool GFxStyledText::ParseHtml(const wchar_t* text, unsigned int len, const GFxTextFormat* defFmt,
                              const GFxTextParagraphFormat* defPFmt, GFxFontManager* fonts)
{
    Clear();
    ImageTagsDropped = 0;
    if (defFmt)
        DefaultTextFormat = *defFmt;
    if (defPFmt)
        DefaultParagraphFormat = *defPFmt;

    GArray<GFxHtmlOpenTag*> stack;
    GFxTextFormat          fmt = DefaultTextFormat;
    GFxTextParagraphFormat pfmt = DefaultParagraphFormat;
    AppendNewParagraph(&pfmt);

    GArray<wchar_t> run;
    unsigned int i = 0;
    bool ok = true;

    while (i < len && text[i])
    {
        if (text[i] != L'<')
        {
            wchar_t decoded;
            unsigned int eaten;
            if (text[i] == L'&' && GFxHtmlDecodeEntity(text, len, i, &decoded, &eaten))
            {
                run.PushBack(decoded);
                i += eaten;
            }
            else
            {
                run.PushBack(text[i]);
                ++i;
            }
            continue;
        }

        if (run.GetSize())
        {
            AppendString(&run[0], run.GetSize(), NLP_PreserveAll, &fmt, &pfmt);
            run.Clear();
        }

        // <!-- comment --> and <!doctype ...> are skipped whole, as SkipComment (2013 rva 0xa97bf0) does.
        if (i + 1 < len && text[i + 1] == L'!')
        {
            unsigned int skip = i + 2;
            while (skip < len && text[skip] != L'>')
                ++skip;
            i = skip < len ? skip + 1 : len;
            continue;
        }

        const bool closing = (i + 1 < len && text[i + 1] == L'/');
        unsigned int at = i + 1 + (closing ? 1u : 0u);
        GArray<wchar_t> name;
        while (at < len && !GFxHtmlIsSpace(text[at]) && text[at] != L'>' && text[at] != L'/')
            name.PushBack(text[at++]);

        GFxHtmlAttr attrs[GFxHtmlMaxAttrs];
        unsigned int attrCount = 0;
        bool selfClosing = false;
        while (at < len && text[at] != L'>')
        {
            if (GFxHtmlIsSpace(text[at]))
            {
                ++at;
                continue;
            }
            if (text[at] == L'/')
            {
                selfClosing = true;
                ++at;
                continue;
            }
            GFxHtmlAttr& attr = attrs[attrCount < GFxHtmlMaxAttrs ? attrCount : 0];
            attr.NameLen = 0;
            attr.ValueLen = 0;
            while (at < len && !GFxHtmlIsSpace(text[at]) && text[at] != L'=' && text[at] != L'>')
            {
                if (attr.NameLen < GFxHtmlMaxName)
                    attr.Name[attr.NameLen++] = text[at];
                ++at;
            }
            while (at < len && GFxHtmlIsSpace(text[at]))
                ++at;
            if (at < len && text[at] == L'=')
            {
                ++at;
                while (at < len && GFxHtmlIsSpace(text[at]))
                    ++at;
                const wchar_t quote = (at < len && (text[at] == L'"' || text[at] == 0x27))
                                      ? text[at] : (wchar_t)0;
                if (quote)
                    ++at;
                while (at < len)
                {
                    if (quote ? (text[at] == quote)
                              : (GFxHtmlIsSpace(text[at]) || text[at] == L'>'))
                        break;
                    wchar_t decoded;
                    unsigned int eaten;
                    if (text[at] == L'&' && GFxHtmlDecodeEntity(text, len, at, &decoded, &eaten))
                        at += eaten;
                    else
                        decoded = text[at++];
                    if (attr.ValueLen < GFxHtmlMaxValue)
                        attr.Value[attr.ValueLen++] = decoded;
                }
                if (quote && at < len)
                    ++at;
            }
            if (attr.NameLen && attrCount < GFxHtmlMaxAttrs)
                ++attrCount;
        }
        if (at >= len)
            ok = false;
        i = at < len ? at + 1 : len;

        if (closing)
        {
            // Pop to the matching open tag. An unbalanced close is ignored rather than discarding
            // the document, which is what retail's stack walk leaves behind.
            int found = -1;
            for (int s = (int)stack.GetSize() - 1; s >= 0; --s)
            {
                bool same = stack[(unsigned int)s]->Name.GetSize() == name.GetSize();
                for (unsigned int c = 0; same && c < name.GetSize(); ++c)
                    same = GFxHtmlLower(stack[(unsigned int)s]->Name[c]) == GFxHtmlLower(name[c]);
                if (same)
                {
                    found = s;
                    break;
                }
            }
            if (found >= 0)
            {
                fmt = stack[(unsigned int)found]->Format;
                pfmt = stack[(unsigned int)found]->ParagraphFormat;
                for (unsigned int s = (unsigned int)found; s < stack.GetSize(); ++s)
                    delete stack[s];
                stack.Resize((unsigned int)found);
            }
            else
            {
                ok = false;
            }
            continue;
        }

        if (GFxHtmlNameIs(name, "br"))
        {
            static const wchar_t nl[1] = { L'\n' };
            AppendString(nl, 1, NLP_PreserveAll, &fmt, &pfmt);
            continue;
        }
        if (GFxHtmlNameIs(name, "img"))
        {
            ++ImageTagsDropped;
            continue;
        }

        GFxHtmlOpenTag* open = new GFxHtmlOpenTag;
        GFxTextCopyArray(&open->Name, name);
        open->Format = fmt;
        open->ParagraphFormat = pfmt;

        if (GFxHtmlNameIs(name, "b"))
        {
            fmt.SetBold(true);
        }
        else if (GFxHtmlNameIs(name, "i"))
        {
            fmt.SetItalic(true);
        }
        else if (GFxHtmlNameIs(name, "u"))
        {
            fmt.SetUnderline(true);
        }
        else if (GFxHtmlNameIs(name, "a"))
        {
            char url[192];
            GFxHtmlAttrToUtf8(GFxHtmlFindAttr(attrs, attrCount, "href"), url, sizeof(url));
            if (url[0])
                fmt.SetUrl(url);
        }
        else if (GFxHtmlNameIs(name, "font"))
        {
            char face[128];
            GFxHtmlAttrToUtf8(GFxHtmlFindAttr(attrs, attrCount, "face"), face, sizeof(face));
            if (face[0])
            {
                GFxFontResource* res = fonts ? fonts->FindFontResource(face, 0) : 0;
                if (res && res->GetName())
                {
                    fmt.SetFontName(res->GetName());
                    if ((res->GetFontFlags() & GFxFont::FF_DeviceFont) == 0)
                    {
                        GPtr<GFxFontHandle> handle = new GFxFontHandle(fonts, res, res->GetName());
                        fmt.SetFontHandle(handle.GetPtr());
                    }
                }
                else
                {
                    fmt.SetFontName(face);
                    fmt.ClearFontHandle();
                }
            }
            float size;
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "size"), &size) && size > 0.0f)
                fmt.SetFontSize(size);
            unsigned int rgb;
            if (GFxHtmlAttrToColor(GFxHtmlFindAttr(attrs, attrCount, "color"), &rgb))
            {
                const GColor was = fmt.GetColor();
                fmt.SetColor(GColor((GUByte)((rgb >> 16) & 0xFF), (GUByte)((rgb >> 8) & 0xFF),
                                    (GUByte)(rgb & 0xFF), was.Channels.Alpha));
            }
            float spacing;
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "letterspacing"), &spacing))
                fmt.SetLetterSpacing(spacing);
            const GFxHtmlAttr* kerning = GFxHtmlFindAttr(attrs, attrCount, "kerning");
            if (kerning)
                fmt.SetKerning(GFxHtmlAttrIsTrue(kerning));
        }
        else if (GFxHtmlNameIs(name, "p") || GFxHtmlNameIs(name, "li"))
        {
            const GFxHtmlAttr* align = GFxHtmlFindAttr(attrs, attrCount, "align");
            if (align)
            {
                if (GFxHtmlNameIs(align->Value, align->ValueLen, "right"))
                    pfmt.SetAlignment(GFxTextParagraphFormat::Align_Right);
                else if (GFxHtmlNameIs(align->Value, align->ValueLen, "center"))
                    pfmt.SetAlignment(GFxTextParagraphFormat::Align_Center);
                else if (GFxHtmlNameIs(align->Value, align->ValueLen, "justify"))
                    pfmt.SetAlignment(GFxTextParagraphFormat::Align_Justify);
                else
                    pfmt.SetAlignment(GFxTextParagraphFormat::Align_Left);
            }
            if (GFxHtmlNameIs(name, "li"))
                pfmt.SetBullet(true);
            // A second block opens a new paragraph; the first one takes the format of the paragraph
            // AppendNewParagraph already made, so a lone <p align> does not leave a blank line.
            GFxTextParagraph* last = GetLastParagraph();
            if (last && last->GetLength())
            {
                static const wchar_t nl[1] = { L'\n' };
                AppendString(nl, 1, NLP_PreserveAll, &fmt, &pfmt);
                last = GetLastParagraph();
            }
            if (last)
                last->SetFormat(pfmt);
        }
        else if (GFxHtmlNameIs(name, "textformat"))
        {
            float v;
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "indent"), &v))
                pfmt.SetIndent((short)(int)v);
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "blockindent"), &v))
                pfmt.SetBlockIndent((short)(int)v);
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "leftmargin"), &v))
                pfmt.SetLeftMargin((unsigned short)(int)v);
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "rightmargin"), &v))
                pfmt.SetRightMargin((unsigned short)(int)v);
            if (GFxHtmlAttrToFloat(GFxHtmlFindAttr(attrs, attrCount, "leading"), &v))
                pfmt.SetLeading((short)(int)v);
            GFxTextParagraph* last = GetLastParagraph();
            if (last)
                last->SetFormat(pfmt);
        }
        else
        {
            ok = false;
        }

        if (selfClosing)
        {
            fmt = open->Format;
            pfmt = open->ParagraphFormat;
            delete open;
        }
        else
        {
            stack.PushBack(open);
        }
    }

    if (run.GetSize())
        AppendString(&run[0], run.GetSize(), NLP_PreserveAll, &fmt, &pfmt);
    for (unsigned int s = 0; s < stack.GetSize(); ++s)
        delete stack[s];
    return ok;
}
