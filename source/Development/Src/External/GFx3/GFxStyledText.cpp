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
    // DISHONORED(port): 0xaa5400 - the range gets the interned format; overlapping runs are split.
    if (!pAllocator || !len)
        return;
    if (at >= Text.GetSize())
        return;
    if (at + len > Text.GetSize())
        len = Text.GetSize() - at;

    const GFxTextFormat* interned = pAllocator->AllocateTextFormat(f);
    GArray<FormatRun> next;
    for (unsigned int i = 0; i < Runs.GetSize(); ++i)
    {
        const FormatRun& r = Runs[i];
        const unsigned int end = r.Index + r.Length;
        if (end <= at || r.Index >= at + len)
        {
            next.PushBack(r);
            continue;
        }
        if (r.Index < at)
        {
            FormatRun head = r;
            head.Length = at - r.Index;
            next.PushBack(head);
        }
        if (end > at + len)
        {
            FormatRun tail = r;
            tail.Index = at + len;
            tail.Length = end - (at + len);
            next.PushBack(tail);
        }
    }
    FormatRun mid;
    mid.Index = at;
    mid.Length = len;
    mid.pFormat = interned;
    next.PushBack(mid);
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
    : pAllocator(alloc)
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
