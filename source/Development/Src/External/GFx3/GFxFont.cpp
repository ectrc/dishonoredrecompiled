// GFx 3.3 fonts - the DefineFont/2/3 reader, the font resource, the handle, the manager.
// See GFxFont.h for the unit system, which is the thing to get right in this file.
#include "GFxFont.h"
#include "GFxPlayer.h"

#include <string.h>
#include <stdio.h>

// =============================================================================================
// GFxFontData
// =============================================================================================

GFxFontData::GFxFontData()
    : ReadEndPos(0), KerningDeclared(0)
{
    // DISHONORED(port): 0xa581f0
}

GFxFontData::~GFxFontData()
{
    // DISHONORED(port): 0xa58340
}

void GFxFontData::SetName(const char* name)
{
    if (!name)
    {
        Name[0] = 0;
        return;
    }
    unsigned int i = 0;
    for (; name[i] && i < sizeof(Name) - 1; ++i)
        Name[i] = name[i];
    Name[i] = 0;
}

bool GFxFontData::Read(GFxStream* s, unsigned int tagType, unsigned int tagEnd)
{
    // DISHONORED(port): 0xa587d0 (3,924 bytes). The retail body is two branches on the tag type:
    // DefineFont (10) has nothing but an offset table and glyph shapes, DefineFont2 (48) and
    // DefineFont3 (75) have the flag byte, the language code, the name, the glyph count, an optional
    // wide offset table, the code table and the optional layout block.
    Glyphs.Clear();
    Advances.Clear();
    Kerning.Clear();
    CodeTable.Clear();
    Flags = 0;
    Ascent = Descent = Leading = 0.0f;

    if (tagType == GFxTag_DefineFont)
    {
        // The plain DefineFont. The first u16 of the offset table is itself the byte offset of the
        // first glyph, so dividing it by two gives the glyph count - retail's `v130 = *v12 >> 1`.
        const unsigned int base = s->Tell();
        GArray<unsigned int> offsets;
        unsigned int first = s->ReadU16();
        offsets.PushBack(first);
        const unsigned int count = first >> 1;
        bool valid = true;
        for (unsigned int i = 1; i < count; ++i)
        {
            const unsigned int off = s->ReadU16();
            if (!off)
            {
                valid = false;
                break;
            }
            offsets.PushBack(off);
        }
        Glyphs.Resize(count);
        for (unsigned int i = 0; i < count; ++i)
            Glyphs[i] = 0;
        if (!valid)
        {
            Flags |= FF_GlyphShapesStripped;
            ReadEndPos = s->Tell();
            buildLookups();
            return true;
        }
        for (unsigned int i = 0; i < count && i < offsets.GetSize(); ++i)
        {
            const unsigned int at = base + offsets[i];
            const unsigned int end = (i + 1 < offsets.GetSize()) ? (base + offsets[i + 1]) : tagEnd;
            s->SetPosition(at);
            GPtr<GFxConstShapeNoStyles> shape = new GFxConstShapeNoStyles;
            shape->Read(s, GFxTag_DefineShape, end);
            Glyphs[i] = shape.GetPtr();
        }
        ReadEndPos = s->Tell();
        buildLookups();
        return true;
    }

    if (tagType != GFxTag_DefineFont2 && tagType != GFxTag_DefineFont3)
        return false;

    // The flag bits, read in the order the retail body reads them and mapped to the flag word at
    // exactly the bits it sets (GFxFont::FontFlags in GFxFont.h names each one).
    const bool hasLayout   = s->ReadUBits(1) != 0;
    const bool shiftJIS    = s->ReadUBits(1) != 0;
    const bool smallText   = s->ReadUBits(1) != 0;
    const bool ansi        = s->ReadUBits(1) != 0;
    const bool wideOffsets = s->ReadUBits(1) != 0;
    const bool wideCodes   = s->ReadUBits(1) != 0;
    const bool italic      = s->ReadUBits(1) != 0;
    const bool bold        = s->ReadUBits(1) != 0;

    if (hasLayout) Flags |= FF_HasLayout;
    Flags &= ~(unsigned int)FF_CodePageMask;
    if (shiftJIS)      Flags |= FF_CodePageShiftJIS;
    else if (ansi)     Flags |= FF_CodePageANSI;
    if (smallText) Flags |= FF_SmallText;
    if (wideCodes) Flags |= FF_WideCodes;
    if (italic)    Flags |= FF_Italic;
    if (bold)      Flags |= FF_Bold;

    s->Align();
    s->ReadU8();                                   // LanguageCode, read and dropped
    s->ReadStringWithLength(Name, sizeof(Name));

    const unsigned int glyphCount = s->ReadU16();
    const unsigned int base = s->Tell();            // offsets are relative to here

    GArray<unsigned int> offsets;
    bool valid = true;
    if (glyphCount > 0)
    {
        const unsigned int first = wideOffsets ? s->ReadU32() : s->ReadU16();
        if (first == 0)
        {
            // A zero first offset means gfxexport stripped the outlines out of the tag.
            valid = false;
        }
        else
        {
            offsets.PushBack(first);
        }
    }
    unsigned int codeTableOffset = 0;
    if (valid)
    {
        for (unsigned int i = 1; i < glyphCount; ++i)
            offsets.PushBack(wideOffsets ? s->ReadU32() : s->ReadU16());
        codeTableOffset = wideOffsets ? s->ReadU32() : s->ReadU16();
    }

    Glyphs.Resize(glyphCount);
    for (unsigned int i = 0; i < glyphCount; ++i)
        Glyphs[i] = 0;
    if (hasLayout)
    {
        Advances.Resize(glyphCount);
        for (unsigned int i = 0; i < glyphCount; ++i)
        {
            Advances[i].Advance = 0.0f;
            Advances[i].Left = Advances[i].Top = 0;
            Advances[i].Width = Advances[i].Height = 0;
        }
    }

    if (!valid)
    {
        Flags |= FF_GlyphShapesStripped;
        ReadEndPos = s->Tell();
        buildLookups();
        return true;
    }

    // The glyph shape tag dialect: DefineShape2 for a DefineFont2 glyph, the DefineFont3 code itself
    // for a DefineFont3 glyph (which is what turns on the 20x coordinate scale), DefineShape for a
    // DefineFont glyph. Retail: `shapeTag = 22; if (tagType != 48) shapeTag = tagType;`.
    const unsigned int shapeTag = (tagType == GFxTag_DefineFont2) ? GFxTag_DefineShape2 : tagType;

    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        const unsigned int at = base + offsets[i];
        const unsigned int next = (i + 1 < glyphCount) ? (base + offsets[i + 1])
                                                       : (base + codeTableOffset);
        s->SetPosition(at);
        GPtr<GFxConstShapeNoStyles> shape = new GFxConstShapeNoStyles;
        shape->Read(s, shapeTag, next);
        Glyphs[i] = shape.GetPtr();

        if (hasLayout)
        {
            // The per-glyph bound, cached in twips of glyph units, exactly as the retail body does:
            // an inverted bound (which a blank glyph produces) is stored as four zeroes.
            GRect<float> b = shape->GetRectBoundsLocal();
            if (b.Left > b.Right || b.Top > b.Bottom)
            {
                Advances[i].Left = Advances[i].Top = 0;
                Advances[i].Width = Advances[i].Height = 0;
            }
            else
            {
                Advances[i].Left = (short)(b.Left * 20.0f);
                Advances[i].Top = (short)(b.Top * 20.0f);
                Advances[i].Width = (unsigned short)((b.Right - b.Left) * 20.0f);
                Advances[i].Height = (unsigned short)((b.Bottom - b.Top) * 20.0f);
            }
        }
    }

    if (base + codeTableOffset >= tagEnd)
    {
        ReadEndPos = s->Tell();
        buildLookups();
        return true;
    }
    s->SetPosition(base + codeTableOffset);
    ReadCodeTable(s, glyphCount);

    if (!hasLayout)
    {
        ReadEndPos = s->Tell();
        buildLookups();
        return true;
    }

    // The layout block. The 0.05 is the DefineFont3 20x factor and nothing else: retail is
    // `scale = (tagType == 75) ? 0.05f : 1.0f`.
    const float scale = (tagType == GFxTag_DefineFont3) ? 0.050000001f : 1.0f;
    Ascent = (float)s->ReadS16() * scale;
    Descent = (float)s->ReadS16() * scale;
    Leading = (float)s->ReadS16() * scale;

    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        // Retail reads the advance *unsigned* (`*(unsigned __int16*)`) although the SWF spec calls
        // the field SI16. Reproduced as-is: no glyph in the cook has a negative advance, so the two
        // readings agree on this content, and the faithful one is retail's.
        const unsigned int adv = s->ReadU16();
        Advances[i].Advance = (float)adv * scale;
    }
    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        int rect[4];
        s->ReadRect(rect);          // the per-glyph bound again, discarded: the shape's is used
    }

    const unsigned int kernCount = s->ReadU16();
    KerningDeclared = kernCount;
    for (unsigned int i = 0; i < kernCount; ++i)
    {
        if (s->Tell() >= tagEnd)
            break;                  // retail logs "kerning table ... longer than tagLength" here
        KerningPair kp;
        if (wideCodes)
        {
            kp.Left = s->ReadU16();
            kp.Right = s->ReadU16();
        }
        else
        {
            kp.Left = s->ReadU8();
            kp.Right = s->ReadU8();
        }
        kp.Adjustment = (float)s->ReadS16() * scale;
        Kerning.PushBack(kp);
    }
    ReadEndPos = s->Tell();
    buildLookups();
    return true;
}

void GFxFontData::ReadCodeTable(GFxStream* s, unsigned int glyphCount)
{
    // DISHONORED(port): 0xa583c0 - one code per glyph, u16 when FF_WideCodes and u8 otherwise, into a
    // code -> glyph-index hash. Note the *key* is the character code and the *value* the index, so a
    // font may map several codes to one glyph.
    const bool wide = (Flags & FF_WideCodes) != 0;
    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        CodeEntry e;
        e.Code = wide ? s->ReadU16() : (unsigned short)s->ReadU8();
        e.GlyphIndex = (unsigned short)i;
        CodeTable.PushBack(e);
    }
}

GFxShapeBase* GFxFontData::GetGlyphShape(unsigned int glyphIndex, unsigned int hintedSize)
{
    // DISHONORED(port): 0xa52bd0 - adds a reference. The hinted size is what the *compacted* font
    // data uses to pick a hinted outline; a plain GFxFontData ignores it, as retail's does.
    (void)hintedSize;
    if (glyphIndex >= Glyphs.GetSize())
        return 0;
    GFxShapeBase* p = Glyphs[glyphIndex].GetPtr();
    if (p)
        p->AddRef();
    return p;
}

GRect<float>& GFxFontData::GetGlyphBounds(unsigned int glyphIndex, GRect<float>* out) const
{
    // DISHONORED(port): 0x9c54d0. Three cases, in retail's order:
    //   * glyphIndex == ~0u: the "no glyph" box, 0,0 to advance x height;
    //   * a cached AdvanceEntry: the four twip fields divided by 20, with a zero width falling back
    //     to the advance;
    //   * no layout: compute the shape's bound on the spot.
    if (glyphIndex == ~0u)
    {
        out->Left = 0.0f;
        out->Top = 0.0f;
        out->Right = GetAdvance(~0u);
        out->Bottom = GetGlyphHeight(~0u);
        return *out;
    }
    if (glyphIndex < Advances.GetSize())
    {
        const AdvanceEntry& e = Advances[glyphIndex];
        float w = (float)e.Width / 20.0f;
        if (w == 0.0f)
            w = e.Advance;
        const float h = (float)e.Height / 20.0f;
        out->Left = (float)e.Left / 20.0f;
        out->Top = (float)e.Top / 20.0f;
        out->Right = out->Left + w;
        out->Bottom = out->Top + h;
        return *out;
    }
    out->Left = out->Top = out->Right = out->Bottom = 0.0f;
    if (glyphIndex < Glyphs.GetSize() && Glyphs[glyphIndex].GetPtr())
    {
        GRect<float> b;
        Glyphs[glyphIndex]->ComputeBound(&b);
        if (b.Left <= b.Right && b.Top <= b.Bottom)
            *out = b;
    }
    return *out;
}

float GFxFontData::GetAdvance(unsigned int glyphIndex) const
{
    // DISHONORED(port): 0xa52c00 - 512 is half an EM and is what a font with no layout answers for
    // every glyph, with a warn-once in retail.
    if (glyphIndex == ~0u)
        return 512.0f;
    if (Advances.GetSize() == 0)
        return 512.0f;
    if (glyphIndex >= Advances.GetSize())
        return 0.0f;
    return Advances[glyphIndex].Advance;
}

float GFxFontData::GetGlyphWidth(unsigned int glyphIndex) const
{
    // DISHONORED(port): 0xa52c40's shape (GFxTextureFont's) applied to the advance table: the cached
    // width in twips of glyph units, divided by 20.
    if (glyphIndex == ~0u)
        return 512.0f;
    if (glyphIndex >= Advances.GetSize())
        return 0.0f;
    return (float)Advances[glyphIndex].Width / 20.0f;
}

float GFxFontData::GetGlyphHeight(unsigned int glyphIndex) const
{
    // DISHONORED(port): 0xa52ca0 - 1024 is the whole EM.
    if (glyphIndex == ~0u)
        return 1024.0f;
    if (Advances.GetSize() == 0)
        return 1024.0f;
    if (glyphIndex >= Advances.GetSize())
        return 0.0f;
    return (float)Advances[glyphIndex].Height / 20.0f;
}

// The 32-bit key retail hashes: the pair packed into one word, which is what
// GFxFontData::GetKerningAdjustment (0x9c5b00) builds before it calls findIndexAlt.
static unsigned int GFxKernKey(unsigned int left, unsigned int right)
{
    return ((unsigned int)(unsigned short)left) | (((unsigned int)(unsigned short)right) << 16);
}

static unsigned int GFxKernHash(unsigned int key)
{
    // The 65599 multiplier with the 5381 seed over the four bytes of the key, which is the hash the
    // retail kerning insert computes inline at the end of GFxFontData::Read (0xa587d0).
    unsigned int h = 5381;
    for (int i = 0; i < 4; ++i)
        h = ((key >> (8 * i)) & 0xFFu) + 65599u * h;
    return h;
}

void GFxFontData::buildLookups()
{
    // Open addressing, a power-of-two table at least twice the entry count, -1 for empty.
    unsigned int cap = 16;
    while (cap < (Kerning.GetSize() + 1) * 2)
        cap <<= 1;
    KernHash.Resize(cap);
    for (unsigned int i = 0; i < cap; ++i)
        KernHash[i] = -1;
    for (unsigned int i = 0; i < Kerning.GetSize(); ++i)
    {
        unsigned int at = GFxKernHash(GFxKernKey(Kerning[i].Left, Kerning[i].Right)) & (cap - 1);
        while (KernHash[at] != -1)
            at = (at + 1) & (cap - 1);
        KernHash[at] = (int)i;
    }

    cap = 16;
    while (cap < (CodeTable.GetSize() + 1) * 2)
        cap <<= 1;
    CodeHash.Resize(cap);
    for (unsigned int i = 0; i < cap; ++i)
        CodeHash[i] = -1;
    for (unsigned int i = 0; i < CodeTable.GetSize(); ++i)
    {
        unsigned int at = GFxKernHash(CodeTable[i].Code) & (cap - 1);
        while (CodeHash[at] != -1)
            at = (at + 1) & (cap - 1);
        CodeHash[at] = (int)i;
    }
}

float GFxFontData::GetKerningAdjustment(unsigned int left, unsigned int right) const
{
    // DISHONORED(port): 0x9c5b00 - the hash lookup on the packed (left, right) pair. It has to be a
    // hash and not a scan: $TitleFont carries 5,000 pairs and $NormalFont 2,163 (measured by the
    // harness), and this runs once per character pair of every line laid out.
    if (!KernHash.GetSize() || !Kerning.GetSize())
        return 0.0f;
    const unsigned int cap = KernHash.GetSize();
    const unsigned int key = GFxKernKey(left, right);
    unsigned int at = GFxKernHash(key) & (cap - 1);
    for (unsigned int probes = 0; probes < cap; ++probes)
    {
        const int idx = KernHash[at];
        if (idx < 0)
            return 0.0f;
        const KerningPair& kp = Kerning[(unsigned int)idx];
        if (GFxKernKey(kp.Left, kp.Right) == key)
            return kp.Adjustment;
        at = (at + 1) & (cap - 1);
    }
    return 0.0f;
}

int GFxFontData::GetGlyphIndex(unsigned short code) const
{
    // DISHONORED(port): 0xa542d0 / 0xa58100's shape - the code table lookup, -1 when the font has no
    // glyph for the character. Retail's is a GHashSet<u16,u16> and so is this, for the same reason as
    // the kerning table: once per character laid out.
    if (!CodeHash.GetSize())
        return -1;
    const unsigned int cap = CodeHash.GetSize();
    unsigned int at = GFxKernHash(code) & (cap - 1);
    for (unsigned int probes = 0; probes < cap; ++probes)
    {
        const int idx = CodeHash[at];
        if (idx < 0)
            return -1;
        if (CodeTable[(unsigned int)idx].Code == code)
            return (int)CodeTable[(unsigned int)idx].GlyphIndex;
        at = (at + 1) & (cap - 1);
    }
    return -1;
}

int GFxFontData::GetCharValue(unsigned int glyphIndex) const
{
    // DISHONORED(port): 0xa542e0 - the reverse map, used by TextSnapshot and by GetCharRanges.
    for (unsigned int i = 0; i < CodeTable.GetSize(); ++i)
    {
        if (CodeTable[i].GlyphIndex == (unsigned short)glyphIndex)
            return (int)CodeTable[i].Code;
    }
    return -1;
}

bool GFxFontData::HasVectorOrRasterGlyphs() const
{
    // DISHONORED(port): 0xa58270 - false only when the outlines were stripped out of the tag.
    return (Flags & FF_GlyphShapesStripped) == 0;
}

// =============================================================================================
// GFxFontResource
// =============================================================================================

GFxFontResource::GFxFontResource(GFxFont* font)
    : pFont(font), LowerCaseTop(0), UpperCaseTop(0)
{
    // DISHONORED(port): 0xa54510 - the two hinting tops start at 0, which is the "not calculated
    // yet" sentinel GetLowerCaseTop tests for.
    pLib = 0;
    ExportName[0] = 0;
}

void GFxFontResource::SetExportName(const char* name)
{
    ExportName[0] = 0;
    if (!name)
        return;
    unsigned int i = 0;
    for (; name[i] && i < sizeof(ExportName) - 1; ++i)
        ExportName[i] = name[i];
    ExportName[i] = 0;
}

GFxFontResource::~GFxFontResource()
{
    // DISHONORED(port): 0xa52d00
}

GRect<float>& GFxFontResource::GetGlyphBounds(unsigned int glyphIndex, GRect<float>* out) const
{
    if (pFont)
        return pFont->GetGlyphBounds(glyphIndex, out);
    out->Left = out->Top = out->Right = out->Bottom = 0.0f;
    return *out;
}

unsigned short GFxFontResource::calcTopBound(unsigned short charCode)
{
    // DISHONORED(port): 0xa54680 - the *negated* top of the glyph's bound, i.e. its height above the
    // baseline in glyph units.
    if (!pFont)
        return 0;
    const int idx = pFont->GetGlyphIndex(charCode);
    if (idx == -1)
        return 0;
    GRect<float> r(0.0f, 0.0f, 0.0f, 0.0f);
    pFont->GetGlyphBounds((unsigned int)idx, &r);
    return (unsigned short)(int)(-r.Top);
}

void GFxFontResource::calcLowerUpperTop()
{
    // DISHONORED(port): 0xa54700 - the auto-hinting reference heights, taken from the first glyph of
    // "HEFTUVWXZ" the font has and the first of "zxvwy". Both character sets are retail's literals.
    static const char* upper = "HEFTUVWXZ";
    static const char* lower = "zxvwy";
    if (!pFont || LowerCaseTop || UpperCaseTop)
        return;

    unsigned short up = 0;
    for (const char* p = upper; *p; ++p)
    {
        up = calcTopBound((unsigned short)(unsigned char)*p);
        if (up)
            break;
    }
    unsigned short lo = 0;
    if (up)
    {
        for (const char* p = lower; *p; ++p)
        {
            lo = calcTopBound((unsigned short)(unsigned char)*p);
            if (lo)
                break;
        }
    }
    if (up && lo)
    {
        UpperCaseTop = up;
        LowerCaseTop = lo;
    }
    else
    {
        // Retail warns "No hinting chars (any of 'HEFTUVWXZ' and 'zxvwy'). Auto-Hinting is disabled"
        // and stores 0xFFFF in both, which GetLowerCaseTop then reports as 0.
        UpperCaseTop = 0xFFFF;
        LowerCaseTop = 0xFFFF;
    }
}

unsigned short GFxFontResource::GetLowerCaseTop()
{
    // DISHONORED(port): 0xa4ba50
    if (!LowerCaseTop)
        calcLowerUpperTop();
    return (short)LowerCaseTop > 0 ? LowerCaseTop : 0;
}

unsigned short GFxFontResource::GetUpperCaseTop()
{
    // DISHONORED(port): 0xa4ba80
    if (!UpperCaseTop)
        calcLowerUpperTop();
    return (short)UpperCaseTop > 0 ? UpperCaseTop : 0;
}

// =============================================================================================
// GFxFontHandle
// =============================================================================================

GFxFontHandle::GFxFontHandle(GFxFontManager* mgr, GFxFontResource* res, const char* name)
    : pManager(mgr), pFont(res), Scale(1.0f)
{
    // DISHONORED(port): 0x9f4190
    Name[0] = 0;
    if (name)
    {
        unsigned int i = 0;
        for (; name[i] && i < sizeof(Name) - 1; ++i)
            Name[i] = name[i];
        Name[i] = 0;
    }
}

bool GFxFontHandle::operator==(const GFxFontHandle& o) const
{
    // DISHONORED(port): 0xa90d60 - the resource decides identity; the name is compared only when
    // both handles are unresolved.
    if (pFont != o.pFont)
        return false;
    if (pFont)
        return true;
    return strcmp(Name, o.Name) == 0;
}

// =============================================================================================
// GFxFontManager
// =============================================================================================

GFxFontManager::GFxFontManager()
{
    // DISHONORED(port): 0xa52490 / commonInit 0xa516a0
}

GFxFontManager::~GFxFontManager()
{
    // DISHONORED(port): 0xa517c0
}

void GFxFontManager::AddFont(GFxFontResource* res)
{
    // The registration half of GFxFontLib::AddFontsFrom (0x9c4dc0).
    if (!res)
        return;
    Fonts.PushBack(GPtr<GFxFontResource>(res));
}

// Retail's font-name matching is case-insensitive and ignores a leading '$', because fontlib exports
// its symbols as `$NormalFont` / `$TitleFont` while a text field's font name is the *face* name
// ("ChaletComprime-CologneEighty"). FindOrCreateHandle (0xa51850) tries the export name first and
// then the face name, which is what the two probes below reproduce.
static bool GFxFontNameEqual(const char* a, const char* b)
{
    if (!a || !b)
        return false;
    if (*a == '$') ++a;
    if (*b == '$') ++b;
    for (;;)
    {
        char ca = *a++, cb = *b++;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb)
            return false;
        if (!ca)
            return true;
    }
}

GFxFontResource* GFxFontManager::FindFontResource(const char* name, unsigned int flags) const
{
    // DISHONORED(port): the search core of FindOrCreateHandle (0xa51850), reduced to the two sources
    // this tree has: the registered resources, matched on style first and then without style.
    const unsigned int want = flags & (GFxFont::FF_Bold | GFxFont::FF_Italic);
    for (unsigned int pass = 0; pass < 2; ++pass)
    {
        for (unsigned int i = 0; i < Fonts.GetSize(); ++i)
        {
            GFxFontResource* r = Fonts[i].GetPtr();
            if (!r)
                continue;
            if (!GFxFontNameEqual(r->GetName(), name) &&
                !GFxFontNameEqual(r->GetExportName(), name))
                continue;
            if (pass == 0)
            {
                const unsigned int have = r->GetFontFlags() & (GFxFont::FF_Bold | GFxFont::FF_Italic);
                if (have != want)
                    continue;
            }
            return r;
        }
    }
    return 0;
}

GFxFontHandle* GFxFontManager::CreateFontHandle(const char* name, unsigned int flags)
{
    // DISHONORED(port): 0xa52520 -> 0xa52240 -> 0xa51850. A handle is returned with one reference,
    // which the caller (a GFxTextFormat) owns.
    GFxFontResource* res = FindFontResource(name, flags);
    if (!res)
        return 0;
    return new GFxFontHandle(this, res, name);
}

GFxFontHandle* GFxFontManager::GetEmptyFont()
{
    // DISHONORED(port): 0xa508a0 - the one handle with no resource, so the formatter always has
    // something to hold.
    if (!pEmptyFont)
        pEmptyFont = new GFxFontHandle(this, 0, "");
    return pEmptyFont.GetPtr();
}

// =============================================================================================
// Loading the fonts out of a cooked payload
// =============================================================================================

unsigned int GFxFontLoadFromPayload(GFxFontManager* mgr, const unsigned char* data,
                                    unsigned int size, char* errBuf, unsigned int errBufSize)
{
    // The retail route into this is GFx_DefineFontLoader (2012 0xa357d0) off the tag-loader table,
    // reached from GFxMovieDataDef's tag walk, plus GFx_ImportLoader (0xa385f0) to bind a UI movie's
    // ImportAssets2 reference to the fontlib movie's exports. Neither is in this tree (package CD),
    // so this walks the payload itself: it is the same tag loop with only the font tags handled.
    if (errBuf && errBufSize)
        errBuf[0] = 0;
    if (!mgr || !data || size < 21)
    {
        if (errBuf && errBufSize)
            _snprintf(errBuf, errBufSize - 1, "payload too small (%u bytes)", size);
        return 0;
    }

    GFxGfxFileInfo info;
    if (!GFxGfxParseFile(data, size, info))
    {
        if (errBuf && errBufSize)
            _snprintf(errBuf, errBufSize - 1, "%s", info.Error);
        return 0;
    }

    GFxStream s(data, size);
    s.SetPosition(info.FirstTagOffset);
    unsigned int loaded = 0;

    while (s.Tell() < size)
    {
        unsigned int code = 0, tagEnd = 0;
        if (!s.OpenTag(&code, &tagEnd))
            break;
        if (code == GFxTag_End)
            break;

        if (code == GFxTag_DefineFont || code == GFxTag_DefineFont2 || code == GFxTag_DefineFont3)
        {
            const unsigned int id = s.ReadU16();
            GPtr<GFxFontData> font = new GFxFontData;
            if (font->Read(&s, code, tagEnd))
            {
                // A correctness check the harness prints: a DefineFont read has to land exactly on
                // the tag's last byte, which is the same acceptance agent BB used for the container
                // parser. Anything else means a field was mis-sized.
                if (font->GetReadEndPos() != tagEnd && errBuf && errBufSize)
                {
                    _snprintf(errBuf, errBufSize - 1,
                              "font id %u: read stopped at %u, tag ends at %u (delta %d)",
                              id, font->GetReadEndPos(), tagEnd,
                              (int)font->GetReadEndPos() - (int)tagEnd);
                }
                // The export name wins over the DefineFont2/3 face name when the movie exports this
                // character: it is what a UI movie's ImportAssets2 asks for by name.
                if (font->GetName()[0] == 0)
                    font->SetName("Unnamed");
                GPtr<GFxFontResource> res = new GFxFontResource(font.GetPtr());
                for (unsigned int e = 0; e < info.ExportCount; ++e)
                {
                    if (info.Exports[e].CharacterId == (unsigned short)id)
                    {
                        res->SetExportName(info.Exports[e].Name);
                        break;
                    }
                }
                mgr->AddFont(res.GetPtr());
                ++loaded;
            }
        }
        s.CloseTag();
    }
    return loaded;
}
