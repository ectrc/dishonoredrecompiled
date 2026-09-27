// GFx 3.3 text and paragraph formats, and the format allocator that interns them.
// See GFxText.h for the units and for how the alignment enum was pinned.
#include "GFxText.h"

#include <string.h>

// =============================================================================================
// GFxTextFormat
// =============================================================================================

static void GFxCopyString(char* dst, unsigned int cap, const char* src)
{
    if (!src)
    {
        dst[0] = 0;
        return;
    }
    unsigned int i = 0;
    for (; src[i] && i < cap - 1; ++i)
        dst[i] = src[i];
    dst[i] = 0;
}

GFxTextFormat::GFxTextFormat()
    : Color(0xFF000000), FontSizeTwips(0), LetterSpacingTwips(0), Present(0), Style(0)
{
    // DISHONORED(port): 0xa07980
    FontList[0] = 0;
    Url[0] = 0;
}

GFxTextFormat::GFxTextFormat(const GFxTextFormat& o)
{
    // DISHONORED(port): 0xa91f80
    *this = o;
}

GFxTextFormat::~GFxTextFormat()
{
    // DISHONORED(port): 0xa079e0
}

GFxTextFormat& GFxTextFormat::operator=(const GFxTextFormat& o)
{
    // DISHONORED(port): 0xa27c30
    if (this == &o)
        return *this;
    memcpy(FontList, o.FontList, sizeof(FontList));
    memcpy(Url, o.Url, sizeof(Url));
    Color = o.Color;
    FontSizeTwips = o.FontSizeTwips;
    LetterSpacingTwips = o.LetterSpacingTwips;
    Present = o.Present;
    Style = o.Style;
    pFontHandle = o.pFontHandle;
    return *this;
}

void GFxTextFormat::InitByDefaultValues()
{
    // DISHONORED(port): 0xa92ce0 - "Times New Roman" at 12 points, opaque black, no letter spacing.
    // The colour write is `Color &= 0x00FFFFFF; ... Color |= 0xFF000000`, i.e. black with alpha 255.
    GFxCopyString(FontList, sizeof(FontList), "Times New Roman");
    Present |= PF_FontList;
    SetFontSize(12.0f);
    ClearFontHandle();
    Color = GColor(0xFF000000u);
    LetterSpacingTwips = 0;
    Style = 0;
    Present |= PF_Color | PF_LetterSpacing | PF_Bold | PF_Italic | PF_Underline | PF_Kerning;
    Url[0] = 0;
    Present &= ~(unsigned int)PF_Url;
}

void GFxTextFormat::SetFontList(const char* list)
{
    // DISHONORED(port): 0xa92190 - changing the font list invalidates a resolved handle.
    GFxCopyString(FontList, sizeof(FontList), list);
    Present |= PF_FontList;
    ClearFontHandle();
}

void GFxTextFormat::SetFontSize(float px)
{
    // DISHONORED(port): 0xa70dc0 - 3276.8 px is the u16 twip ceiling and retail stores 0xFFFF there.
    Present |= PF_FontSize;
    if (px >= 3276.8f)
        FontSizeTwips = 0xFFFF;
    else
        FontSizeTwips = (unsigned short)(int)(px * 20.0f);
}

void GFxTextFormat::SetLetterSpacing(float px)
{
    // DISHONORED(port): 0xa70da0
    Present |= PF_LetterSpacing;
    LetterSpacingTwips = (short)(int)(px * 20.0f);
}

void GFxTextFormat::SetBold(bool v)
{
    // DISHONORED(port): 0xa92400 - a style change invalidates the handle only when the style
    // actually differs, because resolving a font handle is the expensive part.
    if ((Present & PF_FontHandle) != 0 && (((Style & SF_Bold) != 0) != v))
        ClearFontHandle();
    if (v) Style |= SF_Bold; else Style &= ~(unsigned char)SF_Bold;
    Present |= PF_Bold;
}

void GFxTextFormat::SetItalic(bool v)
{
    // DISHONORED(port): 0xa92450
    if ((Present & PF_FontHandle) != 0 && (((Style & SF_Italic) != 0) != v))
        ClearFontHandle();
    if (v) Style |= SF_Italic; else Style &= ~(unsigned char)SF_Italic;
    Present |= PF_Italic;
}

void GFxTextFormat::SetUnderline(bool v)
{
    // DISHONORED(port): 0xa90be0
    if (v) Style |= SF_Underline; else Style &= ~(unsigned char)SF_Underline;
    Present |= PF_Underline;
}

void GFxTextFormat::SetKerning(bool v)
{
    // DISHONORED(port): 0xa90c00
    if (v) Style |= SF_Kerning; else Style &= ~(unsigned char)SF_Kerning;
    Present |= PF_Kerning;
}

void GFxTextFormat::SetAlpha(unsigned char a)
{
    // DISHONORED(port): 0xa70cc0 - alpha is the top byte of the colour word.
    Color.SetRaw((Color.GetRaw() & 0x00FFFFFFu) | ((unsigned long)a << 24));
    Present |= PF_Alpha;
}

void GFxTextFormat::SetFontHandle(GFxFontHandle* h)
{
    // DISHONORED(port): 0xa923b0
    pFontHandle = h;
    if (h)
        Present |= PF_FontHandle;
    else
        Present &= ~(unsigned int)PF_FontHandle;
}

void GFxTextFormat::ClearFontHandle()
{
    // DISHONORED(port): 0xa92090
    pFontHandle = 0;
    Present &= ~(unsigned int)PF_FontHandle;
}

void GFxTextFormat::SetUrl(const char* url)
{
    // DISHONORED(port): 0xa90b20
    GFxCopyString(Url, sizeof(Url), url);
    Present |= PF_Url;
}

void GFxTextFormat::ClearUrl()
{
    // DISHONORED(port): 0xa70cf0
    Url[0] = 0;
    Present &= ~(unsigned int)PF_Url;
}

bool GFxTextFormat::IsFontSame(const GFxTextFormat& o) const
{
    // DISHONORED(port): 0xa91070 - a resolved handle decides, the name only when neither has one.
    if (pFontHandle.GetPtr() && o.pFontHandle.GetPtr())
        return pFontHandle.GetPtr() == o.pFontHandle.GetPtr();
    if (strcmp(FontList, o.FontList) != 0)
        return false;
    return (Style & (SF_Bold | SF_Italic)) == (o.Style & (SF_Bold | SF_Italic));
}

bool GFxTextFormat::operator==(const GFxTextFormat& o) const
{
    // DISHONORED(port): 0xa90e50
    if (Present != o.Present || Style != o.Style)
        return false;
    if (FontSizeTwips != o.FontSizeTwips || LetterSpacingTwips != o.LetterSpacingTwips)
        return false;
    if (Color != o.Color)
        return false;
    if (strcmp(FontList, o.FontList) != 0)
        return false;
    if (strcmp(Url, o.Url) != 0)
        return false;
    return pFontHandle.GetPtr() == o.pFontHandle.GetPtr();
}

GFxTextFormat GFxTextFormat::Merge(const GFxTextFormat& o) const
{
    // DISHONORED(port): 0xa924a0 - `o`'s present fields override ours, field by field. 1,089 bytes in
    // retail because every field carries its own present bit and its own invalidation rule.
    GFxTextFormat r = *this;
    if (o.Present & PF_FontList)
    {
        memcpy(r.FontList, o.FontList, sizeof(r.FontList));
        r.pFontHandle = o.pFontHandle;
        r.Present |= PF_FontList;
        if (o.Present & PF_FontHandle) r.Present |= PF_FontHandle;
        else                           r.Present &= ~(unsigned int)PF_FontHandle;
    }
    else if (o.Present & PF_FontHandle)
    {
        r.pFontHandle = o.pFontHandle;
        r.Present |= PF_FontHandle;
    }
    if (o.Present & PF_FontSize)      { r.FontSizeTwips = o.FontSizeTwips; r.Present |= PF_FontSize; }
    if (o.Present & PF_LetterSpacing) { r.LetterSpacingTwips = o.LetterSpacingTwips; r.Present |= PF_LetterSpacing; }
    if (o.Present & PF_Color)         { r.Color.SetRaw((r.Color.GetRaw() & 0xFF000000u) | (o.Color.GetRaw() & 0x00FFFFFFu)); r.Present |= PF_Color; }
    if (o.Present & PF_Alpha)         { r.Color.SetRaw((r.Color.GetRaw() & 0x00FFFFFFu) | (o.Color.GetRaw() & 0xFF000000u)); r.Present |= PF_Alpha; }
    if (o.Present & PF_Bold)      { r.Style = (unsigned char)((r.Style & ~SF_Bold) | (o.Style & SF_Bold)); r.Present |= PF_Bold; }
    if (o.Present & PF_Italic)    { r.Style = (unsigned char)((r.Style & ~SF_Italic) | (o.Style & SF_Italic)); r.Present |= PF_Italic; }
    if (o.Present & PF_Underline) { r.Style = (unsigned char)((r.Style & ~SF_Underline) | (o.Style & SF_Underline)); r.Present |= PF_Underline; }
    if (o.Present & PF_Kerning)   { r.Style = (unsigned char)((r.Style & ~SF_Kerning) | (o.Style & SF_Kerning)); r.Present |= PF_Kerning; }
    if (o.Present & PF_Url)       { memcpy(r.Url, o.Url, sizeof(r.Url)); r.Present |= PF_Url; }
    return r;
}

GFxTextFormat GFxTextFormat::Intersection(const GFxTextFormat& o) const
{
    // DISHONORED(port): 0xa928f0 - keep only what both agree on. This is what TextField.getTextFormat
    // over a range returns.
    GFxTextFormat r;
    r.Present = 0;
    if ((Present & o.Present & PF_FontList) && strcmp(FontList, o.FontList) == 0)
    {
        memcpy(r.FontList, FontList, sizeof(r.FontList));
        r.Present |= PF_FontList;
    }
    if ((Present & o.Present & PF_FontSize) && FontSizeTwips == o.FontSizeTwips)
    {
        r.FontSizeTwips = FontSizeTwips;
        r.Present |= PF_FontSize;
    }
    if ((Present & o.Present & PF_LetterSpacing) && LetterSpacingTwips == o.LetterSpacingTwips)
    {
        r.LetterSpacingTwips = LetterSpacingTwips;
        r.Present |= PF_LetterSpacing;
    }
    if ((Present & o.Present & PF_Color) &&
        (Color.GetRaw() & 0x00FFFFFFu) == (o.Color.GetRaw() & 0x00FFFFFFu))
    {
        r.Color.SetRaw((r.Color.GetRaw() & 0xFF000000u) | (Color.GetRaw() & 0x00FFFFFFu));
        r.Present |= PF_Color;
    }
    if ((Present & o.Present & PF_Alpha) &&
        (Color.GetRaw() & 0xFF000000u) == (o.Color.GetRaw() & 0xFF000000u))
    {
        r.Color.SetRaw((r.Color.GetRaw() & 0x00FFFFFFu) | (Color.GetRaw() & 0xFF000000u));
        r.Present |= PF_Alpha;
    }
    static const unsigned short styleBits[4] = { PF_Bold, PF_Italic, PF_Underline, PF_Kerning };
    static const unsigned char  styleVals[4] = { SF_Bold, SF_Italic, SF_Underline, SF_Kerning };
    for (int i = 0; i < 4; ++i)
    {
        if ((Present & o.Present & styleBits[i]) &&
            (Style & styleVals[i]) == (o.Style & styleVals[i]))
        {
            r.Style = (unsigned char)(r.Style | (Style & styleVals[i]));
            r.Present |= styleBits[i];
        }
    }
    if ((Present & o.Present & PF_Url) && strcmp(Url, o.Url) == 0)
    {
        memcpy(r.Url, Url, sizeof(r.Url));
        r.Present |= PF_Url;
    }
    return r;
}

unsigned int GFxTextFormat::Hash() const
{
    // DISHONORED(port): 0xa91220 (GFxTextFormat::HashFunctor::operator()) - the same djb2-with-65599
    // string hash the rest of the runtime uses, fed the fields in declaration order.
    unsigned int h = 5381;
    for (const char* p = FontList; *p; ++p)
        h = (unsigned int)(unsigned char)*p + 65599u * h;
    for (const char* p = Url; *p; ++p)
        h = (unsigned int)(unsigned char)*p + 65599u * h;
    h = (unsigned int)Color.GetRaw() + 65599u * h;
    h = FontSizeTwips + 65599u * h;
    h = (unsigned int)(unsigned short)LetterSpacingTwips + 65599u * h;
    h = Present + 65599u * h;
    h = Style + 65599u * h;
    return h;
}

unsigned int GFxTextFormat::GetFontStyleFlags() const
{
    unsigned int f = 0;
    if (Style & SF_Bold)   f |= GFxFont::FF_Bold;
    if (Style & SF_Italic) f |= GFxFont::FF_Italic;
    return f;
}

// =============================================================================================
// GFxTextParagraphFormat
// =============================================================================================

GFxTextParagraphFormat::GFxTextParagraphFormat()
    : Indent(0), BlockIndent(0), Leading(0), LeftMargin(0), RightMargin(0), Present(0)
{
    // DISHONORED(port): 0xa260b0
}

void GFxTextParagraphFormat::InitByDefaultValues()
{
    // DISHONORED(port): 0xa91350 - everything zero, alignment left and present (`Present = ... | 1`),
    // tab stops released.
    Indent = BlockIndent = Leading = 0;
    LeftMargin = RightMargin = 0;
    Present = PF_Alignment;
}

void GFxTextParagraphFormat::SetAlignment(AlignType a)
{
    // DISHONORED(port): 0xa242b0 - `Present = (Present & ~0x600) | (a << 9) | 1`.
    Present = (unsigned short)((Present & ~(unsigned int)PF_AlignMask) |
                               (((unsigned int)a << PF_AlignShift) & PF_AlignMask) | PF_Alignment);
}

bool GFxTextParagraphFormat::IsLeftAlignment() const
{
    // DISHONORED(port): 0xaae080
    return (Present & PF_Alignment) != 0 && (Present & PF_AlignMask) == 0;
}

bool GFxTextParagraphFormat::IsRightAlignment() const
{
    // DISHONORED(port): 0xa990a0
    return (Present & PF_Alignment) != 0 && (Present & PF_AlignMask) == 0x200;
}

bool GFxTextParagraphFormat::IsCenterAlignment() const
{
    // DISHONORED(port): 0xa990d0
    return (Present & PF_Alignment) != 0 && (Present & PF_AlignMask) == 0x600;
}

bool GFxTextParagraphFormat::IsJustifyAlignment() const
{
    // FinalizeLine (0xa9a940) distributes the slack when bits 9..10 are 2.
    return (Present & PF_Alignment) != 0 && (Present & PF_AlignMask) == 0x400;
}

void GFxTextParagraphFormat::SetBullet(bool v)
{
    // DISHONORED(port): 0xa70d10 - two bits: "bullet field present" and the value itself.
    if (v) Present |= PF_BulletOn; else Present &= ~(unsigned int)PF_BulletOn;
    Present |= PF_Bullet;
}

bool GFxTextParagraphFormat::IsBullet() const
{
    // DISHONORED(port): 0xa70d40
    return (Present & PF_Bullet) != 0 && (Present & PF_BulletOn) != 0;
}

bool GFxTextParagraphFormat::operator==(const GFxTextParagraphFormat& o) const
{
    // DISHONORED(port): 0xa90f70
    return Indent == o.Indent && BlockIndent == o.BlockIndent && Leading == o.Leading &&
           LeftMargin == o.LeftMargin && RightMargin == o.RightMargin && Present == o.Present;
}

GFxTextParagraphFormat GFxTextParagraphFormat::Merge(const GFxTextParagraphFormat& o) const
{
    // DISHONORED(port): 0xa91a10
    GFxTextParagraphFormat r = *this;
    if (o.Present & PF_Alignment)
    {
        r.Present = (unsigned short)((r.Present & ~(unsigned int)PF_AlignMask) |
                                     (o.Present & PF_AlignMask) | PF_Alignment);
    }
    if (o.Present & PF_Indent)      { r.Indent = o.Indent; r.Present |= PF_Indent; }
    if (o.Present & PF_BlockIndent) { r.BlockIndent = o.BlockIndent; r.Present |= PF_BlockIndent; }
    if (o.Present & PF_Leading)     { r.Leading = o.Leading; r.Present |= PF_Leading; }
    if (o.Present & PF_LeftMargin)  { r.LeftMargin = o.LeftMargin; r.Present |= PF_LeftMargin; }
    if (o.Present & PF_RightMargin) { r.RightMargin = o.RightMargin; r.Present |= PF_RightMargin; }
    if (o.Present & PF_Bullet)
    {
        r.Present = (unsigned short)((r.Present & ~(unsigned int)PF_BulletOn) |
                                     (o.Present & PF_BulletOn) | PF_Bullet);
    }
    return r;
}

GFxTextParagraphFormat GFxTextParagraphFormat::Intersection(const GFxTextParagraphFormat& o) const
{
    // DISHONORED(port): 0xa91b80
    GFxTextParagraphFormat r;
    if ((Present & o.Present & PF_Alignment) && (Present & PF_AlignMask) == (o.Present & PF_AlignMask))
        r.Present = (unsigned short)((Present & PF_AlignMask) | PF_Alignment);
    if ((Present & o.Present & PF_Indent) && Indent == o.Indent)
        { r.Indent = Indent; r.Present |= PF_Indent; }
    if ((Present & o.Present & PF_BlockIndent) && BlockIndent == o.BlockIndent)
        { r.BlockIndent = BlockIndent; r.Present |= PF_BlockIndent; }
    if ((Present & o.Present & PF_Leading) && Leading == o.Leading)
        { r.Leading = Leading; r.Present |= PF_Leading; }
    if ((Present & o.Present & PF_LeftMargin) && LeftMargin == o.LeftMargin)
        { r.LeftMargin = LeftMargin; r.Present |= PF_LeftMargin; }
    if ((Present & o.Present & PF_RightMargin) && RightMargin == o.RightMargin)
        { r.RightMargin = RightMargin; r.Present |= PF_RightMargin; }
    if ((Present & o.Present & PF_Bullet) && (Present & PF_BulletOn) == (o.Present & PF_BulletOn))
        r.Present = (unsigned short)(r.Present | (Present & PF_BulletOn) | PF_Bullet);
    return r;
}

unsigned int GFxTextParagraphFormat::Hash() const
{
    // DISHONORED(port): 0xa914b0
    unsigned int h = 5381;
    h = (unsigned int)(unsigned short)Indent + 65599u * h;
    h = (unsigned int)(unsigned short)BlockIndent + 65599u * h;
    h = (unsigned int)(unsigned short)Leading + 65599u * h;
    h = LeftMargin + 65599u * h;
    h = RightMargin + 65599u * h;
    h = Present + 65599u * h;
    return h;
}

// =============================================================================================
// GFxTextAllocator
// =============================================================================================

GFxTextAllocator::GFxTextAllocator()
{
    // DISHONORED(port): 0xa0dd70
}

GFxTextAllocator::~GFxTextAllocator()
{
    for (unsigned int i = 0; i < TextFormats.GetSize(); ++i)
        delete TextFormats[i];
    for (unsigned int i = 0; i < ParagraphFormats.GetSize(); ++i)
        delete ParagraphFormats[i];
}

const GFxTextFormat* GFxTextAllocator::AllocateTextFormat(const GFxTextFormat& f)
{
    // DISHONORED(port): 0xa93870 - a hash lookup on GFxTextFormat::HashFunctor in retail. The hash is
    // computed here too and compared before the full operator==, so the common case is one integer
    // compare per cached entry, which is what makes the format runs pointer-comparable.
    const unsigned int h = f.Hash();
    for (unsigned int i = 0; i < TextFormats.GetSize(); ++i)
    {
        if (TextFormats[i]->Hash() == h && *TextFormats[i] == f)
            return TextFormats[i];
    }
    GFxTextFormat* p = new GFxTextFormat(f);
    TextFormats.PushBack(p);
    return p;
}

const GFxTextParagraphFormat* GFxTextAllocator::AllocateParagraphFormat(
    const GFxTextParagraphFormat& f)
{
    // DISHONORED(port): 0xa93960
    const unsigned int h = f.Hash();
    for (unsigned int i = 0; i < ParagraphFormats.GetSize(); ++i)
    {
        if (ParagraphFormats[i]->Hash() == h && *ParagraphFormats[i] == f)
            return ParagraphFormats[i];
    }
    GFxTextParagraphFormat* p = new GFxTextParagraphFormat(f);
    ParagraphFormats.PushBack(p);
    return p;
}
