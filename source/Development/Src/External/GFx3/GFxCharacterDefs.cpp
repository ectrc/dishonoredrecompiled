// Scaleform GFx 3.3.89 - the concrete character definitions the tag loaders produce. Package CD,
// retail objects GFxShape.obj (the with-styles prefix), GFxStyles.obj (the style records),
// GFxTextField.obj (the edit-text definition), GFxButton.obj and GFxFontResource.obj.
//
// The record walk (GFxConstShapeNoStyles::Read, 2012 0xa42ab0) is GFxShapeRecord::Read below, and the
// with-styles prefix retail keeps in GFxConstShapeWithStyles::Read (0xa43610) - the bounds and the two
// style arrays - is GFxShapeCharacterDef::Read.
//
// Every read below is the retail body term for term, including the four places where GFx departs
// from the published SWF layout and each of which is called out at the site:
//   * the fill-style count promotes to u16 on 255 only when the tag type is above 2 (0xa429c0),
//     while the line-style count promotes unconditionally (0xa41270);
//   * a focal gradient's focal point is read after the gradient records, not before (0xa90290);
//   * a DefineShape4 line style's miter limit is flag 0x20, not 0x0800 (0xa907d0);
//   * an edit text's variable name is read before its initial text, and three of its fifteen flag
//     bits are stored inverted (0xa26190).
// DISHONORED(port): 2013 rvas beside each function; see agentCD.md.
#include "GFxCharacterDefs.h"

#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------
// GFxGradientRecord / GFxFillStyle / GFxLineStyle

void GFxReadRgbaTag(GFxStream* s, GColor* out, unsigned int tagType)   // 2012 0xa22460
{
    if (tagType > 22)
    {
        s->ReadRgba(out);
    }
    else
    {
        s->ReadRgb(out);
        out->SetAlpha(255);
    }
}

void GFxGradientRecord::Read(GFxStream* s, unsigned int tagType)       // 2012 0xa8dfc0
{
    Ratio = s->ReadU8();
    GFxReadRgbaTag(s, &Color, tagType);
}

void GFxFillStyle::Read(GFxStream* s, unsigned int tagType)            // 2012 0xa90290
{
    Type = s->ReadU8();
    Flags = 0;

    if (Type == GFxFill_Solid)
    {
        GFxReadRgbaTag(s, &Color, tagType);
        if (Color.GetAlpha() != 0xFF)
            Flags |= Flag_HasAlpha;
        return;
    }

    if ((Type & 0x10) != 0)
    {
        s->ReadMatrix(&Matrix);
        const unsigned char info = s->ReadU8();
        if ((info & 0x10) != 0)
            Flags |= Flag_Interpolation;
        GradientCount = (unsigned int)(info & 0x0F);
        for (unsigned int i = 0; i < GradientCount; ++i)
        {
            GFxGradientRecord r;
            r.Read(s, tagType);
            if (r.Color.GetAlpha() != 0xFF)
                Flags |= Flag_HasAlpha;
            if (i < MaxGradientRecords)
                Gradient[i] = r;
        }
        // The retail body tests the type for exactly 19 rather than masking, and it does so *after*
        // the records, which is what makes 0x13 the focal gradient and 0x12 the radial.
        if (Type == GFxFill_FocalGradient)
            FocalPoint = (float)s->ReadS16() / 256.0f;
        return;
    }

    if ((Type & 0x40) != 0)
    {
        ImageId = s->ReadU16();
        s->ReadMatrix(&Matrix);
        Flags |= Flag_HasAlpha;
    }
    // An unknown fill type leaves the stream where it is; the record walk's end check catches it,
    // which is what the retail body relies on too.
}

void GFxLineStyle::Read(GFxStream* s, unsigned int tagType)            // 2012 0xa907d0
{
    Width = s->ReadU16();
    Flags = 0;
    if (tagType == GFxTag_DefineShape4)
    {
        Flags = s->ReadU16();
        if ((Flags & Flag2_HasMiterLimit) != 0)
            MiterLimit = (float)s->ReadU16() / 256.0f;
    }
    if ((Flags & Flag2_HasFill) != 0)
    {
        delete pFill;
        pFill = new GFxFillStyle;
        pFill->Read(s, tagType);
        // A stroke with a fill has no colour of its own; retail takes the fill's solid colour, or
        // the first gradient stop when the fill is a gradient, so the renderer always has one.
        if (!pFill->IsGradient() && !pFill->IsImage())
            Color = pFill->Color;
        else if (pFill->IsGradient() && pFill->GradientCount != 0)
            Color = pFill->Gradient[0].Color;
    }
    else
    {
        GFxReadRgbaTag(s, &Color, tagType);
    }
}

// ---------------------------------------------------------------------------------------------
// GFxShapeRecord: the SWF shape-record walk of GFxConstShapeNoStyles::Read (2012 0xa42ab0).
//
// The grammar below is that body term for term. What retail does with it is different and is the one
// deviation this package declares: retail copies the raw bit stream into a packed block out of a
// GFxPathAllocator and only *scans* it, keeping the path count and the shape count in a
// variable-length header (GFxSwfPathData::GetShapeAndPathCounts 0xa3b5c0 reads them back off the end)
// so that GFxSwfPathData::PathsIterator can decode it again on every traversal. This decodes once.
// Both counts come out the same, which is what the harness reports.

GFxShapePathCD::~GFxShapePathCD()
{
    free(Edges);
}

void GFxShapePathCD::AddEdge(const GFxShapeEdgeCD& e)
{
    if (EdgeCount >= EdgeCapacity)
    {
        EdgeCapacity = EdgeCapacity ? EdgeCapacity * 2 : 8;
        Edges = (GFxShapeEdgeCD*)realloc(Edges, EdgeCapacity * sizeof(GFxShapeEdgeCD));
    }
    Edges[EdgeCount++] = e;
}

GFxShapeRecord::~GFxShapeRecord()
{
    for (unsigned int i = 0; i < PathCount; ++i)
        delete Paths[i];
    free(Paths);
}

void GFxShapeRecord::AddPath(GFxShapePathCD* p)
{
    if (p->EdgeCount == 0)
    {
        delete p;
        return;
    }
    if (PathCount >= PathCapacity)
    {
        PathCapacity = PathCapacity ? PathCapacity * 2 : 8;
        Paths = (GFxShapePathCD**)realloc(Paths, PathCapacity * sizeof(GFxShapePathCD*));
    }
    Paths[PathCount++] = p;
}

unsigned int GFxShapeRecord::GetEdgeCount() const
{
    unsigned int n = 0;
    for (unsigned int i = 0; i < PathCount; ++i)
        n += Paths[i]->EdgeCount;
    return n;
}

GRect<int> GFxShapeRecord::ComputeBound() const                        // 2012 0xa3eea0
{
    GRect<int> b(0, 0, 0, 0);
    bool first = true;
    for (unsigned int i = 0; i < PathCount; ++i)
    {
        const GFxShapePathCD* p = Paths[i];
        int x = p->StartX, y = p->StartY;
        for (unsigned int e = 0; e <= p->EdgeCount; ++e)
        {
            if (first) { b = GRect<int>(x, y, x, y); first = false; }
            if (x < b.Left)   b.Left = x;
            if (x > b.Right)  b.Right = x;
            if (y < b.Top)    b.Top = y;
            if (y > b.Bottom) b.Bottom = y;
            if (e < p->EdgeCount)
            {
                // The control point is inside the convex hull of the three points, so including the
                // anchors alone is the same bound retail computes for a quadratic.
                x = p->Edges[e].Ax;
                y = p->Edges[e].Ay;
            }
        }
    }
    return b;
}

void GFxShapeRecord::Read(GFxStream* s, unsigned int tagType, unsigned int endPos,
                          GFxShapeCharacterDef* styleOwner)            // 2012 0xa42ab0
{
    // A DefineFont3 glyph's coordinates are 20x the 1024-unit EM square, which the traversal has to
    // undo; retail keeps it as a flag bit (0x02 at +36) that GFxSwfPathData::PathsIterator turns into
    // a 0.05 scale factor. It is the single most important constant in the font path.
    bTwentyTimesScale = (tagType == GFxTag_DefineFont3);

    s->Align();
    unsigned int numFillBits = s->ReadUBits(4);
    unsigned int numLineBits = s->ReadUBits(4);
    unsigned int fillBase = 0, lineBase = 0;
    ShapeCount = 1;

    GFxShapePathCD* path = 0;
    int x = 0, y = 0;
    unsigned int fill0 = 0, fill1 = 0, line = 0;

    for (;;)
    {
        if (s->Tell() > endPos)
        {
            bCorrupt = true;
            break;
        }
        if (s->ReadUBits(1))
        {
            // An edge record. Both forms carry their own bit width, which is why one shape can
            // interleave 2-bit and 15-bit deltas.
            if (path == 0)
            {
                path = new GFxShapePathCD;
                path->Fill0 = fill0; path->Fill1 = fill1; path->Line = line;
                path->StartX = x;    path->StartY = y;
            }
            GFxShapeEdgeCD e;
            if (s->ReadUBits(1))
            {
                // Straight. A general line reads both deltas; a vertical line reads only dy and a
                // horizontal line only dx. The retail body reaches dy by falling through the general
                // case, which is why the vertical branch has no dx read of its own.
                const unsigned int nbits = s->ReadUBits(4) + 2;
                int dx = 0, dy = 0;
                if (s->ReadUBits(1))
                {
                    dx = s->ReadSBits(nbits);
                    dy = s->ReadSBits(nbits);
                }
                else if (s->ReadUBits(1))
                {
                    dy = s->ReadSBits(nbits);
                }
                else
                {
                    dx = s->ReadSBits(nbits);
                }
                x += dx; y += dy;
                e.bCurve = false;
                e.Cx = x; e.Cy = y;
                e.Ax = x; e.Ay = y;
            }
            else
            {
                const unsigned int nbits = s->ReadUBits(4) + 2;
                const int cdx = s->ReadSBits(nbits);
                const int cdy = s->ReadSBits(nbits);
                const int adx = s->ReadSBits(nbits);
                const int ady = s->ReadSBits(nbits);
                e.bCurve = true;
                e.Cx = x + cdx;    e.Cy = y + cdy;
                e.Ax = e.Cx + adx; e.Ay = e.Cy + ady;
                x = e.Ax; y = e.Ay;
            }
            path->AddEdge(e);
            continue;
        }

        const unsigned int flags = s->ReadUBits(5);
        if (flags == 0)
            break;

        // Every one of the five selection bits closes the open run first, and that flush - not the
        // MoveTo alone - is what makes the path count come out at the number retail writes into the
        // compacted block's header.
        if ((flags & 0x01) != 0)
        {
            const unsigned int nbits = s->ReadUBits(5);
            x = s->ReadSBits(nbits);
            y = s->ReadSBits(nbits);
        }
        if ((flags & 0x02) != 0 && numFillBits > 0)
        {
            const unsigned int v = s->ReadUBits(numFillBits);
            fill0 = v ? v + fillBase : 0;
        }
        if ((flags & 0x04) != 0 && numFillBits > 0)
        {
            const unsigned int v = s->ReadUBits(numFillBits);
            fill1 = v ? v + fillBase : 0;
        }
        if ((flags & 0x08) != 0 && numLineBits > 0)
        {
            const unsigned int v = s->ReadUBits(numLineBits);
            line = v ? v + lineBase : 0;
        }
        if ((flags & 0x10) != 0)
        {
            // StateNewStyles: fresh style arrays appended to the existing ones, then fresh bit
            // widths. Retail appends rather than replaces - the array readers resize from the current
            // size - and accumulates the index bases by the counts it walked past, so the indices the
            // following records use stay valid. A glyph record never carries one.
            // The index base becomes the size the array had *before* the append, because the records
            // after a NewStyles are 1-based into the new group while the array holds every group.
            if (styleOwner != 0)
            {
                fillBase = styleOwner->FillCount;
                lineBase = styleOwner->LineCount;
                styleOwner->ReadFillStyles(s, tagType);
                styleOwner->ReadLineStyles(s, tagType);
            }
            fill0 = fill1 = line = 0;
            s->Align();
            numFillBits = s->ReadUBits(4);
            numLineBits = s->ReadUBits(4);
            ++ShapeCount;
        }

        if (path != 0)
        {
            AddPath(path);
            path = 0;
        }
    }

    if (path != 0)
        AddPath(path);
    s->Align();
}

// ---------------------------------------------------------------------------------------------
// GFxShapeCharacterDef

GFxShapeCharacterDef::GFxShapeCharacterDef(unsigned int tagCode)
    : TagCode(tagCode), bUsesNonScalingStrokes(false), bUsesScalingStrokes(false),
      bUsesFillWinding(false), bHasScale9Grid(false),
      Fills(0), FillCount(0), FillCapacity(0), LineArray(0), LineCount(0), LineCapacity(0)
{
}

GFxShapeCharacterDef::~GFxShapeCharacterDef()
{
    free(Fills);
    for (unsigned int i = 0; i < LineCount; ++i)
        delete LineArray[i];
    free(LineArray);
}

void GFxShapeCharacterDef::AddFill(const GFxFillStyle& f)
{
    if (FillCount >= FillCapacity)
    {
        FillCapacity = FillCapacity ? FillCapacity * 2 : 8;
        Fills = (GFxFillStyle*)realloc(Fills, FillCapacity * sizeof(GFxFillStyle));
    }
    Fills[FillCount++] = f;
}

void GFxShapeCharacterDef::AddLine(GFxLineStyle* l)
{
    if (LineCount >= LineCapacity)
    {
        LineCapacity = LineCapacity ? LineCapacity * 2 : 8;
        LineArray = (GFxLineStyle**)realloc(LineArray, LineCapacity * sizeof(GFxLineStyle*));
    }
    LineArray[LineCount++] = l;
}

void GFxShapeCharacterDef::ReadFillStyles(GFxStream* s, unsigned int tagType)  // 2012 0xa429c0
{
    unsigned int count = s->ReadU8();
    if (tagType > 2 && count == 255)
        count = s->ReadU16();
    for (unsigned int i = 0; i < count; ++i)
    {
        GFxFillStyle f;
        f.Read(s, tagType);
        AddFill(f);
    }
}

void GFxShapeCharacterDef::ReadLineStyles(GFxStream* s, unsigned int tagType)  // 2012 0xa41270
{
    // Unlike the fill count, the line count promotes to u16 on 255 for every tag type: the retail
    // body has no tag test here at all, and that asymmetry is real rather than a transcription slip.
    unsigned int count = s->ReadU8();
    if (count == 255)
        count = s->ReadU16();
    for (unsigned int i = 0; i < count; ++i)
    {
        GFxLineStyle* l = new GFxLineStyle;
        l->Read(s, tagType);
        AddLine(l);
    }
}

void GFxShapeCharacterDef::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{                                                                      // 2012 0xa43610
    TagCode = tagType;
    int r[4];
    s->ReadRect(r);
    Bounds = GRect<int>(r[0], r[2], r[1], r[3]);
    // DefineShape4 carries a second rect - the bound including stroke widths - and a flags byte.
    if (tagType == GFxTag_DefineShape4)
    {
        s->ReadRect(r);
        EdgeBounds = GRect<int>(r[0], r[2], r[1], r[3]);
        const unsigned char f = s->ReadU8();
        bUsesFillWinding       = (f & 0x04) != 0;
        bUsesNonScalingStrokes = (f & 0x02) != 0;
        bUsesScalingStrokes    = (f & 0x01) != 0;
    }
    else
    {
        EdgeBounds = Bounds;
    }
    ReadFillStyles(s, tagType);
    ReadLineStyles(s, tagType);
    Shape.Read(s, tagType, endPos, this);
    s->Align();
}

GFxCharacter* GFxShapeCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                            GFxResourceId id,
                                                            GFxMovieDefImpl* defImpl)
{
    // Retail's GFxGenericCharacter (ctor 2012 0x9cfd90, the same three-argument shape as ours) is
    // what a shape definition instantiates: a display object with a definition and no timeline.
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxMorphCharacterDef (2012 Read 0xab1df0, ReadMorphFillStyle 0xab1370)

GFxMorphCharacterDef::GFxMorphCharacterDef(unsigned int tagCode)
    : TagCode(tagCode),
      pStart(new GFxShapeCharacterDef(tagCode == 84 ? GFxTag_DefineShape4 : 32u)),
      pEnd(new GFxShapeCharacterDef(tagCode == 84 ? GFxTag_DefineShape4 : 32u))
{
}

GFxMorphCharacterDef::~GFxMorphCharacterDef()
{
    delete pStart;
    delete pEnd;
}

void GFxMorphCharacterDef::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    // Two bounds rects (four for tag 84), a u32 offset to the second record, the interpolated style
    // pairs, then the start record and the end record. The pairs are what stand in for a style array,
    // which is why both records are read with the no-styles walk.
    int r[4];
    s->ReadRect(r); pStart->Bounds = GRect<int>(r[0], r[2], r[1], r[3]);
    s->ReadRect(r); pEnd->Bounds   = GRect<int>(r[0], r[2], r[1], r[3]);
    if (tagType == 84)
    {
        s->ReadRect(r); pStart->EdgeBounds = GRect<int>(r[0], r[2], r[1], r[3]);
        s->ReadRect(r); pEnd->EdgeBounds   = GRect<int>(r[0], r[2], r[1], r[3]);
        const unsigned char f = s->ReadU8();
        pStart->bUsesNonScalingStrokes = pEnd->bUsesNonScalingStrokes = (f & 0x02) != 0;
        pStart->bUsesScalingStrokes    = pEnd->bUsesScalingStrokes    = (f & 0x01) != 0;
    }
    const unsigned int offsetToEnd = s->ReadU32();
    const unsigned int endRecordPos = s->Tell() + offsetToEnd;

    // ReadMorphFillStyle reads one fill twice, once per key, and the pairs always use the
    // DefineShape3 dialect (RGBA everywhere) whatever the morph tag's own code is.
    unsigned int fillCount = s->ReadU8();
    if (fillCount == 255) fillCount = s->ReadU16();
    for (unsigned int i = 0; i < fillCount; ++i)
    {
        GFxFillStyle a, b;
        a.Read(s, 32);
        b.Read(s, 32);
        pStart->AddFill(a);
        pEnd->AddFill(b);
    }
    unsigned int lineCount = s->ReadU8();
    if (lineCount == 255) lineCount = s->ReadU16();
    for (unsigned int i = 0; i < lineCount; ++i)
    {
        s->ReadU16();                            // start width
        s->ReadU16();                            // end width
        if (tagType == 84)
        {
            const unsigned short flags = s->ReadU16();
            if ((flags & GFxLineStyle::Flag2_HasMiterLimit) != 0)
                s->ReadU16();
            if ((flags & GFxLineStyle::Flag2_HasFill) != 0)
            {
                GFxFillStyle f0, f1;
                f0.Read(s, GFxTag_DefineShape4);
                f1.Read(s, GFxTag_DefineShape4);
            }
            else
            {
                GColor c;
                GFxReadRgbaTag(s, &c, 32);
                GFxReadRgbaTag(s, &c, 32);
            }
        }
        else
        {
            GColor c;
            GFxReadRgbaTag(s, &c, 32);
            GFxReadRgbaTag(s, &c, 32);
        }
    }

    pStart->Shape.Read(s, tagType == 84 ? GFxTag_DefineShape4 : 32u, endRecordPos, 0);
    s->SetPosition(endRecordPos);
    pEnd->Shape.Read(s, tagType == 84 ? GFxTag_DefineShape4 : 32u, endPos, 0);
}

GFxCharacter* GFxMorphCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                            GFxResourceId id,
                                                            GFxMovieDefImpl* defImpl)
{
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxEditTextCharacterDef (2012 ctor 0xa27200, Read 0xa26190, InitEmptyTextDef 0xa26720)

GFxEditTextCharacterDef::GFxEditTextCharacterDef()
    : FontId(GFxResourceId::InvalidId), FontHeight(0.f), TextColor(GColor(0u, 0u, 0u, 255u)),
      MaxLength(0), Alignment(Align_Left), LeftMargin(0.f), RightMargin(0.f), Indent(0.f),
      Leading(0.f), Flags(0)
{
    VariableName[0] = 0;
    InitialText[0] = 0;
}

void GFxEditTextCharacterDef::InitEmptyTextDef()                       // 2012 0xa26720
{
    Flags = (unsigned short)(Flag_WordWrap | Flag_Multiline | Flag_Selectable | Flag_UseDeviceFont);
    TextRect = GRect<int>(0, 0, 0, 0);
    FontHeight = 12.f * 20.f;
}

void GFxEditTextCharacterDef::Read(GFxStream* s, unsigned int tagType)  // 2012 0xa26190
{
    (void)tagType;
    int r[4];
    s->ReadRect(r);
    TextRect = GRect<int>(r[0], r[2], r[1], r[3]);

    // Fifteen single bits in the order the retail body reads them. Three are stored inverted -
    // NoSelect and UseOutlines are written as their opposites - and two, HasFontClass and WasStatic,
    // are read and dropped, which is why they appear here as bare reads.
    const bool hasText = s->ReadUBits(1) != 0;
    if (s->ReadUBits(1)) Flags |= Flag_WordWrap;  else Flags &= (unsigned short)~Flag_WordWrap;
    if (s->ReadUBits(1)) Flags |= Flag_Multiline; else Flags &= (unsigned short)~Flag_Multiline;
    if (s->ReadUBits(1)) Flags |= Flag_Password;  else Flags &= (unsigned short)~Flag_Password;
    if (s->ReadUBits(1)) Flags |= Flag_ReadOnly;  else Flags &= (unsigned short)~Flag_ReadOnly;
    const bool hasColor     = s->ReadUBits(1) != 0;
    const bool hasMaxLength = s->ReadUBits(1) != 0;
    const bool hasFont      = s->ReadUBits(1) != 0;
    s->ReadUBits(1);                                                   // HasFontClass, dropped
    if (s->ReadUBits(1)) Flags |= Flag_AutoSize;  else Flags &= (unsigned short)~Flag_AutoSize;
    const bool hasLayout    = s->ReadUBits(1) != 0;
    if (s->ReadUBits(1)) Flags &= (unsigned short)~Flag_Selectable;
    else                 Flags |= Flag_Selectable;                     // the stream bit is NoSelect
    if (s->ReadUBits(1)) Flags |= Flag_Border;    else Flags &= (unsigned short)~Flag_Border;
    s->ReadUBits(1);                                                   // WasStatic, dropped
    if (s->ReadUBits(1)) Flags |= Flag_Html;      else Flags &= (unsigned short)~Flag_Html;
    if (s->ReadUBits(1)) Flags &= (unsigned short)~Flag_UseDeviceFont;
    else                 Flags |= Flag_UseDeviceFont;                  // the bit is UseOutlines

    if (hasFont)
    {
        FontId = s->ReadU16();
        FontHeight = (float)s->ReadU16();
    }
    if (hasColor)
        s->ReadRgba(&TextColor);
    if (hasMaxLength)
        MaxLength = s->ReadU16();
    if (hasLayout)
    {
        Flags |= Flag_HasLayout;
        Alignment   = s->ReadU8();
        LeftMargin  = (float)s->ReadU16();
        RightMargin = (float)s->ReadU16();
        Indent      = (float)s->ReadS16();
        Leading     = (float)s->ReadS16();
    }
    // The variable name comes first and unconditionally; the initial text follows only on HasText.
    s->ReadString(VariableName, sizeof(VariableName));
    if (hasText)
        s->ReadString(InitialText, sizeof(InitialText));
}

GFxCharacter* GFxEditTextCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                              GFxResourceId id,
                                                              GFxMovieDefImpl* defImpl)
{
    // Retail's slot (2012 0xa32df0) builds a GFxEditTextCharacter (0xa2c470), the 294-function text
    // field whose layout engine and glyph cache are package CB's. Until that lands the instance is a
    // display object with the definition attached, which is what makes the field's _x/_y/_visible and
    // its member store work; the text itself lives in a member named `text`, which is where AS2
    // writes it anyway.
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxStaticTextCharacterDef (2012 0xa8c130)

void GFxStaticTextCharacterDef::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    int r[4];
    s->ReadRect(r);
    Bounds = GRect<int>(r[0], r[2], r[1], r[3]);
    s->ReadMatrix(&Matrix);
    const unsigned int glyphBits = s->ReadU8();
    const unsigned int advanceBits = s->ReadU8();

    for (;;)
    {
        if (s->Tell() >= endPos)
            break;
        const unsigned char first = s->ReadU8();
        if (first == 0)
            break;
        if ((first & 0x80) != 0)
        {
            ++RecordCount;
            if ((first & 0x08) != 0) s->ReadU16();                     // font id
            if ((first & 0x04) != 0)                                   // colour
            {
                GColor c;
                GFxReadRgbaTag(s, &c, tagType);
            }
            if ((first & 0x01) != 0) s->ReadS16();                     // x offset
            if ((first & 0x02) != 0) s->ReadS16();                     // y offset
            if ((first & 0x08) != 0) s->ReadU16();                     // text height
        }
        else
        {
            const unsigned int count = first & 0x7F;
            GlyphCount += count;
            for (unsigned int i = 0; i < count; ++i)
            {
                s->ReadUBits(glyphBits);
                s->ReadSBits(advanceBits);
            }
            s->Align();
        }
    }
}

GFxCharacter* GFxStaticTextCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                                GFxResourceId id,
                                                                GFxMovieDefImpl* defImpl)
{
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxButtonCharacterDef (2012 0xa6a0c0) and GFxButtonRecord (0xa69bb0)

bool GFxButtonRecord::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    if (s->Tell() >= endPos)
        return false;
    StateFlags = s->ReadU8();
    if (StateFlags == 0)
        return false;
    CharacterId = s->ReadU16();
    Depth = (int)s->ReadU16();
    s->ReadMatrix(&Matrix);
    if (tagType == 34)
        s->ReadCxformRgba(&ColorTransform);
    if ((StateFlags & Has_FilterList) != 0)
    {
        // GFx_LoadFilters (2012 0xa93b60): a u8 count and then that many filter records. The five
        // filter classes are 48 retail functions and none of them is in this package, so the records
        // are stepped over by their fixed sizes, which is what keeps the record stream aligned.
        const unsigned int count = s->ReadU8();
        for (unsigned int i = 0; i < count && s->Tell() < endPos; ++i)
        {
            const unsigned char kind = s->ReadU8();
            switch (kind)
            {
            case 0: s->Skip(23); break;                                // drop shadow
            case 1: s->Skip(9);  break;                                // blur
            case 2: s->Skip(15); break;                                // glow
            case 3: s->Skip(27); break;                                // bevel
            case 4: case 7: { const unsigned int n = s->ReadU8(); s->Skip(5u * n + 19u); break; }
            case 5: { const unsigned int mx = s->ReadU8(); const unsigned int my = s->ReadU8();
                      s->Skip(8u + 4u * mx * my + 1u); break; }        // convolution
            case 6: s->Skip(80); break;                                // colour matrix
            default: s->SetPosition(endPos); break;
            }
        }
    }
    if ((StateFlags & Has_BlendMode) != 0)
        BlendMode = s->ReadU8();
    return true;
}

void GFxButtonCharacterDef::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    if (tagType == 34)
    {
        bTrackAsMenu = (s->ReadU8() & 0x01) != 0;
        s->ReadU16();                                                  // the action offset
    }
    while (RecordCount < MaxRecords)
    {
        if (!Records[RecordCount].Read(s, tagType, endPos))
            break;
        ++RecordCount;
    }
    // DefineButton2 ends with a chain of length-prefixed condition-action blocks, DefineButton with
    // one unterminated action buffer. Neither is executed here - the button's event model needs the
    // mouse path, which is not in this package - but both are counted so the report can say so.
    if (tagType == 34)
    {
        while (s->Tell() + 2 <= endPos)
        {
            const unsigned int here = s->Tell();
            const unsigned int next = s->ReadU16();
            ++CondActionCount;
            if (next == 0)
                break;
            s->SetPosition(here + next);
        }
    }
    else if (s->Tell() < endPos)
    {
        ++CondActionCount;
    }
}

GFxCharacter* GFxButtonCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                             GFxResourceId id,
                                                             GFxMovieDefImpl* defImpl)
{
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxImageCharacterDef

GFxCharacter* GFxImageCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                            GFxResourceId id,
                                                            GFxMovieDefImpl* defImpl)
{
    // An image id placed straight onto a timeline is wrapped by GFxMovieRoot::CreateImageMovieDef
    // (2012 0xa02190) in retail; until the renderer's texture path exists the instance is a plain
    // display object carrying the reference.
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}

// ---------------------------------------------------------------------------------------------
// GFxFontCharacterDef (2012 0xa357d0 -> GFxFontData::Read 0xa587d0)
//
// HAND-OVER to package CB: this is the loader's product and a thin one. CB's GFxFont.h already
// declares GFxFontData with the same Read(stream, tagType, tagEnd), and when that unit compiles the
// tag-10/48/75/1005 row of GFxGetTagLoader should construct a GFxFontData and this class should
// become a one-member wrapper around it. The reason it exists at all is that the dictionary is
// GFxCharacterDef-based while a font is a resource, so something has to adapt the two, and the 12
// DefineFont3 tags in the cook have to leave real entries behind for the fontlib movies to bind.

GFxFontCharacterDef::GFxFontCharacterDef(unsigned int tagCode)
    : TagCode(tagCode), Flags(0), GlyphCount(0), Ascent(0.f), Descent(0.f), Leading(0.f),
      KerningPairCount(0), Glyphs(0), CodeTable(0), Advances(0), GlyphBounds(0)
{
    Name[0] = 0;
}

GFxFontCharacterDef::~GFxFontCharacterDef()
{
    for (unsigned int i = 0; i < GlyphCount; ++i)
        delete Glyphs[i];
    free(Glyphs);
    free(CodeTable);
    free(Advances);
    free(GlyphBounds);
}

void GFxFontCharacterDef::Alloc(unsigned int count)
{
    const unsigned int n = count ? count : 1;
    GlyphCount = count;
    Glyphs      = (GFxShapeRecord**)calloc(n, sizeof(GFxShapeRecord*));
    CodeTable   = (unsigned short*)calloc(n, sizeof(unsigned short));
    Advances    = (float*)calloc(n, sizeof(float));
    GlyphBounds = (GRect<int>*)calloc(n, sizeof(GRect<int>));
}

int GFxFontCharacterDef::GetGlyphIndexForCode(unsigned int code) const
{
    for (unsigned int i = 0; i < GlyphCount; ++i)
        if (CodeTable[i] == (unsigned short)code)
            return (int)i;
    return -1;
}

void GFxFontCharacterDef::Read(GFxStream* s, unsigned int tagType, unsigned int endPos)
{
    const unsigned int tagStart = s->Tell();

    if (tagType == 10)
    {
        // DefineFont: an offset table and the glyph shapes, nothing else. The glyph count falls out
        // of the first offset, which is the retail body's own trick and the only way to know it.
        const unsigned int first = s->ReadU16();
        const unsigned int count = first / 2;
        Alloc(count);
        if (count == 0)
            return;
        unsigned int* offsets = (unsigned int*)malloc(count * sizeof(unsigned int));
        offsets[0] = first;
        for (unsigned int i = 1; i < count; ++i)
            offsets[i] = s->ReadU16();
        for (unsigned int i = 0; i < count; ++i)
        {
            s->SetPosition(tagStart + offsets[i]);
            Glyphs[i] = new GFxShapeRecord;
            Glyphs[i]->Read(s, tagType, endPos, 0);
            GlyphBounds[i] = Glyphs[i]->ComputeBound();
        }
        free(offsets);
        return;
    }

    // DefineFont2 (48), DefineFont3 (75) and GFx's compacted font (1005) share one layout.
    const unsigned char f1 = s->ReadU8();
    if ((f1 & 0x01) != 0) Flags |= Flag_Bold;
    if ((f1 & 0x02) != 0) Flags |= Flag_Italic;
    if ((f1 & 0x04) != 0) Flags |= Flag_WideCodes;
    if ((f1 & 0x08) != 0) Flags |= Flag_WideOffsets;
    if ((f1 & 0x10) != 0) Flags |= Flag_Ansi;
    if ((f1 & 0x20) != 0) Flags |= Flag_SmallText;
    if ((f1 & 0x40) != 0) Flags |= Flag_ShiftJis;
    if ((f1 & 0x80) != 0) Flags |= Flag_HasLayout;
    s->ReadU8();                                                       // language code
    s->ReadStringWithLength(Name, sizeof(Name));

    const unsigned int count = s->ReadU16();
    Alloc(count);
    if (count == 0)
        return;

    const bool wide = (Flags & Flag_WideOffsets) != 0;
    const unsigned int tableStart = s->Tell();
    unsigned int* offsets = (unsigned int*)malloc(count * sizeof(unsigned int));
    for (unsigned int i = 0; i < count; ++i)
        offsets[i] = wide ? s->ReadU32() : s->ReadU16();
    const unsigned int codeTableOffset = wide ? s->ReadU32() : s->ReadU16();

    for (unsigned int i = 0; i < count; ++i)
    {
        const unsigned int glyphEnd = tableStart +
            (i + 1 < count ? offsets[i + 1] : codeTableOffset);
        s->SetPosition(tableStart + offsets[i]);
        Glyphs[i] = new GFxShapeRecord;
        Glyphs[i]->Read(s, tagType, glyphEnd < endPos ? glyphEnd : endPos, 0);
        GlyphBounds[i] = Glyphs[i]->ComputeBound();
    }

    s->SetPosition(tableStart + codeTableOffset);
    for (unsigned int i = 0; i < count; ++i)
        CodeTable[i] = (unsigned short)((Flags & Flag_WideCodes) != 0 ? s->ReadU16() : s->ReadU8());

    if ((Flags & Flag_HasLayout) != 0)
    {
        Ascent  = (float)s->ReadS16();
        Descent = (float)s->ReadS16();
        Leading = (float)s->ReadS16();
        for (unsigned int i = 0; i < count; ++i)
            Advances[i] = (float)s->ReadS16();
        for (unsigned int i = 0; i < count; ++i)
        {
            int r[4];
            s->ReadRect(r);
            GlyphBounds[i] = GRect<int>(r[0], r[2], r[1], r[3]);
        }
        KerningPairCount = s->ReadU16();
        for (unsigned int i = 0; i < KerningPairCount && s->Tell() < endPos; ++i)
        {
            if ((Flags & Flag_WideCodes) != 0) { s->ReadU16(); s->ReadU16(); }
            else                               { s->ReadU8();  s->ReadU8();  }
            s->ReadS16();
        }
    }
    free(offsets);
}

void GFxFontCharacterDef::ReadFontInfo(GFxStream* s, unsigned int tagType, unsigned int endPos)
{                                                                      // 2012 0xa35960
    // DefineFontInfo / DefineFontInfo2 name a font that is already in the dictionary and give it a
    // code table it did not have. It must not replace the definition, which is the bug the previous
    // walk had by treating every id-bearing tag as one.
    s->ReadStringWithLength(Name, sizeof(Name));
    const unsigned char f = s->ReadU8();
    if ((f & 0x01) != 0) Flags |= Flag_WideCodes;
    if ((f & 0x02) != 0) Flags |= Flag_Bold;
    if ((f & 0x04) != 0) Flags |= Flag_Italic;
    if ((f & 0x10) != 0) Flags |= Flag_Ansi;
    if ((f & 0x20) != 0) Flags |= Flag_ShiftJis;
    if (tagType == 62)
        s->ReadU8();                                                   // language code
    for (unsigned int i = 0; i < GlyphCount && s->Tell() < endPos; ++i)
        CodeTable[i] = (unsigned short)((Flags & Flag_WideCodes) != 0 ? s->ReadU16() : s->ReadU8());
}

GFxCharacter* GFxFontCharacterDef::CreateCharacterInstance(GFxASCharacter* parent,
                                                           GFxResourceId id,
                                                           GFxMovieDefImpl* defImpl)
{
    // A font is a resource, never a display object: retail's font definition has no character slot at
    // all, so a PlaceObject naming a font id is a corrupt file. The instance exists only so the
    // dictionary answer is uniform.
    (void)defImpl;
    return new GFxGenericCharacter(this, parent, id, parent ? parent->GetMovieRoot() : 0);
}
