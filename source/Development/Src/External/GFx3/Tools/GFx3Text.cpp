// Agent CB's acceptance harness for the text engine and the glyph rasteriser. Like agent BC's
// GFx3Run it drives the runtime from a cooked asset and nothing else: no engine, no renderer, no
// game, no loader table.
//
//   GFx3Text --fonts <fontlib.gfx> [--verbose]
//       read every DefineFont/2/3 out of a payload and report each font's glyph coverage: how many
//       of its glyphs have outlines, how many contours and vertices they flatten to, the code table,
//       the kerning table and the layout metrics.
//
//   GFx3Text --raster <fontlib.gfx> [--string S] [--size N] [--dump <dir>]
//       rasterise a string in every font of the payload and report the per-glyph bitmap size,
//       coverage and metrics. --dump writes one binary PGM per glyph plus a combined strip, so the
//       glyphs can be looked at rather than believed.
//
//   GFx3Text --run <asset.gfx> [--fontlib <fontlib.gfx>] [--size N] [--verbose]
//       read every DefineEditText out of a cooked movie, build the text field it defines, lay it out
//       and ask the glyph cache for every glyph of every line. Reports how many fields resolved a
//       font, how many laid out, and how many produced glyph output.
//
//   GFx3Text --table
//       the implemented-against-remaining table of the text-and-fonts function group, with each
//       class's retail function count from resources/docs/symbols/functions.csv.
//
// Payloads come out of the cooked *_SF.upk packages with build/agentBB/extract_gfx.py.
// DISHONORED(written): resources/docs/agents/agentCB.md.
#include "GFxTextField.h"
#include "GFxGlyphCache.h"
#include "GFxRasterizer.h"
#include "GFxShape.h"
#include "GFxCharacterDefs.h"
#include "GFxFont.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace
{

int LoadFile(const char* path, unsigned char** outData, unsigned int* outSize)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        printf("  cannot open %s\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    const long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0)
    {
        fclose(f);
        return 0;
    }
    unsigned char* buf = (unsigned char*)malloc((unsigned int)n);
    if (!buf)
    {
        fclose(f);
        return 0;
    }
    const size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    if (got != (size_t)n)
    {
        free(buf);
        return 0;
    }
    *outData = buf;
    *outSize = (unsigned int)n;
    return 1;
}

const char* BaseName(const char* path)
{
    const char* p = path;
    for (const char* q = path; *q; ++q)
        if (*q == '/' || *q == '\\')
            p = q + 1;
    return p;
}

void WritePgm(const char* path, const GImage& img)
{
    FILE* f = fopen(path, "wb");
    if (!f)
        return;
    fprintf(f, "P5\n%u %u\n255\n", (unsigned int)img.Width, (unsigned int)img.Height);
    for (unsigned int y = 0; y < img.Height; ++y)
        fwrite(img.pData + y * img.Pitch, 1, img.Width, f);
    fclose(f);
}

// -------------------------------------------------------------------------------------------
// --fonts

int CmdFonts(const char* path, bool verbose)
{
    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(path, &data, &size))
        return 2;

    printf("  payload        %s  %u bytes\n", BaseName(path), size);

    GFxTextResetFontManager();
    GFxFontManager* mgr = GFxTextGetFontManager();
    char err[256];
    const unsigned int loaded = GFxFontLoadFromPayload(mgr, data, size, err, sizeof(err));
    if (!loaded)
        printf("  fonts          0   %s\n", err[0] ? err : "no DefineFont tag in this payload");
    else
        printf("  fonts          %u   tag consumption: %s\n", loaded,
               err[0] ? err : "every DefineFont read landed exactly on its tag's last byte");

    GRasterizer raster;
    GCompoundShape compound;

    for (unsigned int i = 0; i < mgr->GetFontCount(); ++i)
    {
        GFxFontResource* res = mgr->GetFontByIndex(i);
        GFxFont* font = res->GetFont();
        printf("\n  font %u\n", i);
        printf("    face         '%s'\n", res->GetName());
        printf("    export       '%s'\n", res->GetExportName());
        printf("    flags        0x%04x%s%s%s%s%s\n", font->GetFontFlags(),
               font->IsBold() ? " bold" : "", font->IsItalic() ? " italic" : "",
               font->HasLayout() ? " hasLayout" : "",
               (font->GetFontFlags() & GFxFont::FF_WideCodes) ? " wideCodes" : "",
               font->AreGlyphShapesStripped() ? " SHAPES-STRIPPED" : "");
        printf("    glyphs       %u\n", font->GetGlyphShapeCount());
        printf("    metrics      ascent %.1f  descent %.1f  leading %.1f  (1024-unit EM)\n",
               font->GetAscent(), font->GetDescent(), font->GetLeading());
        printf("    code table   %u entries\n",
               ((GFxFontData*)font)->GetCodeTableSize());
        printf("    kerning      %u pairs read of %u declared%s\n",
               ((GFxFontData*)font)->GetKerningPairCount(),
               ((GFxFontData*)font)->GetKerningPairsDeclared(),
               ((GFxFontData*)font)->GetKerningPairCount() ==
                   ((GFxFontData*)font)->GetKerningPairsDeclared() ? "" : "   TRUNCATED");
        printf("    read ended   byte %u of the payload\n",
               ((GFxFontData*)font)->GetReadEndPos());
        printf("    hint tops    upper %u  lower %u\n", res->GetUpperCaseTop(),
               res->GetLowerCaseTop());

        GFxShapeResetMaxFlattenDepth();
        unsigned int withOutline = 0, blank = 0, contours = 0, vertices = 0, edges = 0, curves = 0;
        float advSum = 0.0f;
        unsigned int advCount = 0;
        for (unsigned int g = 0; g < font->GetGlyphShapeCount(); ++g)
        {
            GPtr<GFxShapeBase> shape = font->GetGlyphShape(g, 0);
            if (!shape)
                continue;
            if (shape->GetPathCount())
            {
                ++withOutline;
                contours += shape->GetPathCount();
                for (unsigned int p = 0; p < shape->GetPathCount(); ++p)
                {
                    const GFxShapePath& path = shape->GetPath(p);
                    edges += path.EdgeCount;
                    for (unsigned int e = 0; e < path.EdgeCount; ++e)
                        if (shape->GetEdge(path.EdgeStart + e).Curve)
                            ++curves;
                }
                shape->MakeCompoundShape(&compound, 10.0f);
                vertices += compound.GetVertexCount();
            }
            else
            {
                ++blank;
            }
            const float a = font->GetAdvance(g);
            if (a > 0.0f)
            {
                advSum += a;
                ++advCount;
            }
        }
        printf("    coverage     %u of %u glyphs have an outline (%.1f %%), %u blank\n",
               withOutline, font->GetGlyphShapeCount(),
               font->GetGlyphShapeCount() ? 100.0 * withOutline / font->GetGlyphShapeCount() : 0.0,
               blank);
        printf("    outlines     %u contours, %u edges of which %u quadratic\n",
               contours, edges, curves);
        printf("    flattened    %u vertices at tolerance 10.0, deepest subdivision %u of a 24 cap\n",
               vertices, GFxShapeGetMaxFlattenDepth());
        printf("    advance      mean %.1f over %u glyphs (%.3f EM)\n",
               advCount ? advSum / (float)advCount : 0.0f, advCount,
               advCount ? advSum / (float)advCount / 1024.0f : 0.0f);

        if (verbose)
        {
            printf("    the ASCII range, code -> glyph:\n     ");
            unsigned int printed = 0;
            for (unsigned int c = 32; c < 127; ++c)
            {
                const int gi = font->GetGlyphIndex((unsigned short)c);
                if (gi < 0)
                    continue;
                printf(" %c=%d", (char)c, gi);
                if (++printed % 12 == 0)
                    printf("\n     ");
            }
            printf("\n");
        }
    }
    free(data);
    return 0;
}

// -------------------------------------------------------------------------------------------
// --raster

int CmdRaster(const char* path, const char* text, float sizePx, const char* dumpDir)
{
    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(path, &data, &size))
        return 2;

    GFxTextResetFontManager();
    GFxFontManager* mgr = GFxTextGetFontManager();
    char err[256];
    GFxFontLoadFromPayload(mgr, data, size, err, sizeof(err));
    if (!mgr->GetFontCount())
    {
        printf("  no font in %s (%s)\n", BaseName(path), err);
        free(data);
        return 2;
    }

    printf("  payload        %s\n", BaseName(path));
    printf("  string         \"%s\"  (%u chars)\n", text, (unsigned int)strlen(text));
    printf("  size           %.1f px  -> glyph scale %.6f px per 1024-EM unit\n",
           sizePx, sizePx / 1024.0f);

    GRasterizer raster;
    GCompoundShape compound;
    unsigned int grandTotalPixels = 0, grandRasterized = 0, grandMissing = 0, grandBlank = 0;

    for (unsigned int fi = 0; fi < mgr->GetFontCount(); ++fi)
    {
        GFxFontResource* res = mgr->GetFontByIndex(fi);
        printf("\n  font %u '%s' (export '%s')\n", fi, res->GetName(), res->GetExportName());
        printf("    %-6s %-5s %-9s %-9s %-16s %-9s %s\n",
               "char", "glyph", "bitmap", "covered", "origin px", "advance", "contours/verts");

        float penX = 0.0f;
        unsigned int fontPixels = 0, fontRasterized = 0, fontMissing = 0, fontBlank = 0;
        // The strip the glyphs are blitted into, so a dump can be looked at as one image.
        const unsigned int stripW = (unsigned int)(sizePx * (float)strlen(text) * 1.2f) + 8;
        const unsigned int stripH = (unsigned int)(sizePx * 2.0f) + 8;
        GImage strip;
        strip.Format = GImageBase::Image_None;
        strip.Width = strip.Height = strip.Pitch = 0;
        strip.pData = 0;
        strip.DataSize = 0;
        strip.MipMapCount = 0;
        GFxImageInit(&strip, stripW, stripH, GImageBase::Image_A_8);
        const float baseline = sizePx * 1.2f;

        for (unsigned int i = 0; text[i]; ++i)
        {
            const unsigned char ch = (unsigned char)text[i];
            const int gi = res->GetGlyphIndex((unsigned short)ch);
            if (gi < 0)
            {
                printf("    '%c'    %-5s %-9s %-9s %-16s %-9s %s\n", ch, "-", "-", "-", "-", "-",
                       "no glyph for this code");
                ++fontMissing;
                continue;
            }

            GImage img;
            img.Format = GImageBase::Image_None;
            img.Width = img.Height = img.Pitch = 0;
            img.pData = 0;
            img.DataSize = 0;
            img.MipMapCount = 0;
            GFxGlyphRasterMetrics m;
            const bool ok = GFxGlyphRasterize(res, (unsigned int)gi, sizePx, 1, &img, &m,
                                              &raster, &compound);
            if (!ok)
            {
                printf("    '%c'    %-5d %s\n", ch, gi, "rasterise failed");
                continue;
            }
            printf("    '%c'    %-5d %3dx%-5d %-9u %7.2f,%-7.2f %-9.2f %u/%u\n",
                   ch, gi, m.Width, m.Height, m.CoveredPixels, m.OriginX, m.OriginY,
                   m.AdvancePx, m.Contours, m.Vertices);

            if (img.pData && m.CoveredPixels)
            {
                ++fontRasterized;
                fontPixels += m.CoveredPixels;
                // Blit into the strip at the pen, with the origin applied.
                const int dx = (int)(penX + m.OriginX) + 2;
                const int dy = (int)(baseline - m.OriginY) + 2;
                for (int y = 0; y < m.Height; ++y)
                {
                    const int ty = dy + y;
                    if (ty < 0 || (unsigned int)ty >= strip.Height)
                        continue;
                    for (int x = 0; x < m.Width; ++x)
                    {
                        const int tx = dx + x;
                        if (tx < 0 || (unsigned int)tx >= strip.Width)
                            continue;
                        unsigned char* d = strip.pData + (unsigned int)ty * strip.Pitch + tx;
                        const unsigned char s = img.pData[(unsigned int)y * img.Pitch + x];
                        if (s > *d)
                            *d = s;
                    }
                }
                if (dumpDir)
                {
                    char name[512];
                    sprintf(name, "%s/f%u_%03d_%02x.pgm", dumpDir, fi, gi, ch);
                    WritePgm(name, img);
                }
            }
            else
            {
                ++fontBlank;
            }
            penX += m.AdvancePx;
            GFxImageFree(&img);
        }

        printf("    total        %u glyphs rasterised, %u blank, %u with no glyph; "
               "%u covered pixels; pen advanced %.2f px\n",
               fontRasterized, fontBlank, fontMissing, fontPixels, penX);
        if (dumpDir)
        {
            char name[512];
            sprintf(name, "%s/strip_f%u.pgm", dumpDir, fi);
            WritePgm(name, strip);
            printf("    dumped       %s and %u per-glyph PGMs\n", name, fontRasterized);
        }
        GFxImageFree(&strip);

        grandRasterized += fontRasterized;
        grandTotalPixels += fontPixels;
        grandMissing += fontMissing;
        grandBlank += fontBlank;
    }

    printf("\n  all fonts      %u rasterised, %u blank, %u missing, %u covered pixels\n",
           grandRasterized, grandBlank, grandMissing, grandTotalPixels);
    free(data);
    return grandRasterized ? 0 : 1;
}

// -------------------------------------------------------------------------------------------
// --run

struct FieldReport
{
    unsigned int Id;
    unsigned int Lines;
    unsigned int Glyphs;
    unsigned int Rasterized;
    unsigned int Blank;
    unsigned int Missing;
    unsigned int Pixels;
    int  WidthTwips, HeightTwips;
    bool HasFont;
    bool HasText;
    char Text[64];
};

int CmdRun(const char* assetPath, const char* fontlibPath, float defaultSize, bool verbose)
{
    unsigned char* fontData = 0;
    unsigned int fontSize = 0;
    GFxTextResetFontManager();
    GFxFontManager* mgr = GFxTextGetFontManager();
    if (fontlibPath)
    {
        if (!LoadFile(fontlibPath, &fontData, &fontSize))
            return 2;
        char err[256];
        const unsigned int n = GFxFontLoadFromPayload(mgr, fontData, fontSize, err, sizeof(err));
        printf("  fontlib        %s: %u fonts, %u resources\n", BaseName(fontlibPath), n,
               mgr->GetFontCount());
    }
    else
    {
        printf("  fontlib        none given: every field will fall back to no font\n");
    }

    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(assetPath, &data, &size))
        return 2;

    GFxGfxFileInfo info;
    if (!GFxGfxParseFile(data, size, info))
    {
        printf("  parse failed   %s\n", info.Error);
        free(data);
        return 2;
    }
    printf("  asset          %s  GFX v%u  %.0f x %.0f px  %u frames  %u tags\n",
           BaseName(assetPath), info.Version, info.FrameWidthPixels, info.FrameHeightPixels,
           info.FrameCount, info.TagCount);

    GFxGlyphRasterCache cache;
    cache.Init(1024, 1024, 4, 64, 1);

    GArray<FieldReport> fields;
    unsigned int editTextTags = 0;

    GFxStream s(data, size);
    s.SetPosition(info.FirstTagOffset);
    while (s.Tell() < size)
    {
        unsigned int code = 0, tagEnd = 0;
        if (!s.OpenTag(&code, &tagEnd))
            break;
        if (code == GFxTag_End)
            break;
        if (code == GFxTag_DefineEditText)
        {
            ++editTextTags;
            const unsigned int id = s.ReadU16();
            GFxTextFieldDesc desc;
            if (GFxTextFieldReadDesc(&s, code, tagEnd, &desc))
            {
                GFxResourceId rid;
                rid.Id = id;
                GFxEditTextCharacter* field = new GFxEditTextCharacter(desc, 0, rid, 0);
                // If the definition carries no initial text there is nothing to lay out, so give it
                // the field's own variable name, which is what the content will assign into.
                if (!desc.InitialText[0] && desc.VariableName[0])
                    field->SetTextValue(desc.VariableName, false, false);
                field->OnEventLoad();

                GFxEditTextCharacter::GlyphOutput out;
                field->ProduceGlyphs(&cache, &out);

                FieldReport r;
                r.Id = id;
                r.Lines = out.Lines;
                r.Glyphs = out.Glyphs;
                r.Rasterized = out.Rasterized;
                r.Blank = out.Blank;
                r.Missing = out.Missing;
                r.Pixels = out.CoveredPixels;
                r.WidthTwips = (int)field->GetDocView().GetTextWidth();
                r.HeightTwips = (int)field->GetDocView().GetTextHeight();
                r.HasFont = mgr->GetFontCount() != 0;
                r.HasText = field->GetTextValue()[0] != 0;
                unsigned int k = 0;
                for (; field->GetTextValue()[k] && k < sizeof(r.Text) - 1; ++k)
                    r.Text[k] = field->GetTextValue()[k];
                r.Text[k] = 0;
                fields.PushBack(r);
                field->Release();
            }
        }
        s.CloseTag();
        (void)defaultSize;
    }

    unsigned int resolved = 0, laidOut = 0, withGlyphs = 0, totalGlyphs = 0, totalRaster = 0;
    unsigned int totalMissing = 0, totalPixels = 0, totalLines = 0;
    for (unsigned int i = 0; i < fields.GetSize(); ++i)
    {
        const FieldReport& r = fields[i];
        if (r.HasFont)
            ++resolved;
        if (r.Lines)
            ++laidOut;
        if (r.Rasterized)
            ++withGlyphs;
        totalLines += r.Lines;
        totalGlyphs += r.Glyphs;
        totalRaster += r.Rasterized;
        totalMissing += r.Missing;
        totalPixels += r.Pixels;
    }

    printf("\n  -- text fields --\n");
    printf("  DefineEditText tags          %u\n", editTextTags);
    printf("  fields built                 %u\n", fields.GetSize());
    printf("  fields that resolved a font  %u\n", resolved);
    printf("  fields that laid out         %u  (%u lines in all)\n", laidOut, totalLines);
    printf("  fields with glyph output     %u\n", withGlyphs);
    printf("  glyph entries laid out       %u\n", totalGlyphs);
    printf("  glyphs rasterised            %u\n", totalRaster);
    printf("  glyphs with no outline       %u\n", totalMissing);
    printf("  atlas                        %u textures, %u cached glyphs, %u rasterised, "
           "%u blank\n",
           cache.GetTextureCount(), cache.GetGlyphCount(), cache.GetRasterizedCount(),
           cache.GetEmptyCount());
    printf("  covered pixels               %u\n", totalPixels);

    if (verbose)
    {
        printf("\n  %-6s %-6s %-7s %-7s %-7s %-9s %-9s %s\n",
               "id", "lines", "glyphs", "raster", "missing", "w twips", "h twips", "text");
        for (unsigned int i = 0; i < fields.GetSize(); ++i)
        {
            const FieldReport& r = fields[i];
            printf("  %-6u %-6u %-7u %-7u %-7u %-9d %-9d '%s'\n", r.Id, r.Lines, r.Glyphs,
                   r.Rasterized, r.Missing, r.WidthTwips, r.HeightTwips, r.Text);
        }
    }

    free(data);
    if (fontData)
        free(fontData);
    return 0;
}

// -------------------------------------------------------------------------------------------
// --layout: the formatter on its own. Lay a string out into a box of a given width, in each of the
// four alignments, and print the per-line metrics GFxTextDocView::GetLineMetrics (0xa9fc60) reports.

int CmdLayout(const char* path, const char* text, float sizePx, float boxWidthPx,
              float boxHeightPx, bool wordWrap)
{
    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(path, &data, &size))
        return 2;
    GFxTextResetFontManager();
    GFxFontManager* mgr = GFxTextGetFontManager();
    char err[256];
    GFxFontLoadFromPayload(mgr, data, size, err, sizeof(err));
    if (!mgr->GetFontCount())
    {
        printf("  no font in %s\n", BaseName(path));
        free(data);
        return 2;
    }

    printf("  box            %.0f x %.0f px = %.0f x %.0f twips, less the 40-twip gutter a side\n",
           boxWidthPx, boxHeightPx, boxWidthPx * 20.0f, boxHeightPx * 20.0f);
    printf("  size           %.1f px   word wrap %s\n", sizePx, wordWrap ? "on" : "off");
    printf("  string         '%s'\n", text);

    static const char* names[4] = { "left", "right", "centre", "justify" };
    GFxGlyphRasterCache cache;
    cache.Init(1024, 1024, 4, 64, 1);

    int rc = 0;
    for (int a = 0; a < 4; ++a)
    {
        GFxTextFieldDesc desc;
        desc.TextRectTwips = GRect<int>(0, 0, (int)(boxWidthPx * 20.0f),
                                        (int)(boxHeightPx * 20.0f));
        desc.FontHeightTwips = sizePx * 20.0f;
        desc.Flags = GFxTextFieldDesc::ETF_Multiline | GFxTextFieldDesc::ETF_HasLayout |
                     GFxTextFieldDesc::ETF_Selectable;
        if (wordWrap)
            desc.Flags |= GFxTextFieldDesc::ETF_WordWrap;
        // The raw SWF Align byte: 0 left, 1 right, 2 centre, 3 justify.
        desc.Align = (unsigned int)a;

        GFxResourceId rid;
        rid.Id = 1000u + (unsigned int)a;
        GFxEditTextCharacter* field = new GFxEditTextCharacter(desc, 0, rid, 0);
        field->SetTextValue(text, false, false);
        field->OnEventLoad();

        GFxEditTextCharacter::GlyphOutput out;
        field->ProduceGlyphs(&cache, &out);
        GFxTextDocView& doc = field->GetDocView();

        printf("\n  align %-8s lines %u   text %d x %d twips = %.1f x %.1f px   "
               "%u glyphs, %u rasterised\n",
               names[a], doc.GetLinesCount(), (int)doc.GetTextWidth(), (int)doc.GetTextHeight(),
               doc.GetTextWidth() / 20.0f, doc.GetTextHeight() / 20.0f, out.Glyphs,
               out.Rasterized);
        printf("    %-5s %-8s %-8s %-8s %-8s %-8s %-8s %s\n",
               "line", "offX", "offY", "width", "height", "ascent", "leading", "text");
        for (unsigned int l = 0; l < doc.GetLinesCount(); ++l)
        {
            GFxTextDocView::LineMetrics m;
            if (!doc.GetLineMetrics(l, &m))
                continue;
            GFxTextLineBuffer::Line* line = doc.GetLineBuffer().GetLine(l);
            char buf[160];
            unsigned int k = 0;
            const unsigned int start = line->TextPos;
            const unsigned int len = (unsigned int)strlen(text);
            for (unsigned int c = 0; c < line->Glyphs.GetSize() && k < sizeof(buf) - 1; ++c)
            {
                const unsigned int at = start + c;
                if (at < len && text[at] != '\n')
                    buf[k++] = text[at];
            }
            buf[k] = 0;
            printf("    %-5u %-8d %-8d %-8d %-8d %-8d %-8d '%s'\n", l, m.OffsetX,
                   line->OffsetY, m.Width, m.Height, m.Ascent, m.Leading, buf);
        }
        if (!doc.GetLinesCount())
            rc = 1;
        field->Release();
    }
    printf("\n  atlas          %u textures, %u cached glyphs, %u rasterised\n",
           cache.GetTextureCount(), cache.GetGlyphCount(), cache.GetRasterizedCount());
    free(data);
    return rc;
}

// -------------------------------------------------------------------------------------------
// --defs: the twelve-line adapter, end to end through package CD's loader.
//
// --run builds a text field straight from a DefineEditText body, which proves the field but not the
// wiring. This goes the whole way instead: package CD's tag loaders build the movie's dictionary, and
// every edit-text definition in it is asked for a character instance through
// GFxEditTextCharacterDef::CreateCharacterInstance (retail 0xa32df0) - the adapter. Before the adapter
// that slot returned a GFxGenericCharacter; the test is that every one now comes back an EditText that
// lays out and produces glyphs.

int CmdDefs(const char* assetPath, const char* fontlibPath, bool verbose)
{
    GFxTextResetFontManager();
    GFxFontManager* mgr = GFxTextGetFontManager();
    unsigned char* fontData = 0;
    unsigned int fontSize = 0;
    if (fontlibPath)
    {
        if (!LoadFile(fontlibPath, &fontData, &fontSize))
            return 2;
        char err[256];
        const unsigned int n = GFxFontLoadFromPayload(mgr, fontData, fontSize, err, sizeof(err));
        printf("  fontlib        %s: %u fonts\n", BaseName(fontlibPath), n);
    }

    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(assetPath, &data, &size))
        return 2;

    GFxMovieDataDef* def = new GFxMovieDataDef;
    if (!def->Read(data, size))
    {
        printf("  read failed    %s\n", BaseName(assetPath));
        delete def;
        free(data);
        return 2;
    }
    printf("  asset          %s  %u dictionary entries\n", BaseName(assetPath), def->GetDictSize());

    unsigned int editTextDefs = 0, instances = 0, asEditText = 0, asGeneric = 0;
    unsigned int laidOut = 0, withGlyphs = 0, totalGlyphs = 0, totalRaster = 0;
    GFxGlyphRasterCache cache;
    cache.Init(1024, 1024, 4, 64, 1);

    for (unsigned int i = 0; i < def->GetDictSize(); ++i)
    {
        GFxCharacterDef* cdef = def->GetDictDef(i);
        if (!cdef)
            continue;
        if (cdef->GetResourceTypeCode() != GFxResource::RT_EditTextDef)
            continue;
        ++editTextDefs;

        GFxCharacter* ch = cdef->CreateCharacterInstance(0, cdef->Id, 0);
        if (!ch)
            continue;
        ++instances;
        const char* type = ch->GetCharacterTypeName();
        if (strcmp(type, "EditText") == 0)
        {
            ++asEditText;
            GFxEditTextCharacter* field = (GFxEditTextCharacter*)ch;
            field->OnEventLoad();
            GFxEditTextCharacter::GlyphOutput out;
            field->ProduceGlyphs(&cache, &out);
            if (out.Lines)
                ++laidOut;
            if (out.Rasterized)
                ++withGlyphs;
            totalGlyphs += out.Glyphs;
            totalRaster += out.Rasterized;
            if (verbose)
                printf("    id %-5u %-9s lines %u  glyphs %u  raster %u  '%s'\n",
                       cdef->Id.Id, type, out.Lines, out.Glyphs, out.Rasterized,
                       field->GetTextValue());
        }
        else
        {
            ++asGeneric;
            if (verbose)
                printf("    id %-5u %-9s  NOT A TEXT FIELD\n", cdef->Id.Id, type);
        }
        ch->Release();
    }

    printf("  edit-text definitions in the dictionary   %u\n", editTextDefs);
    printf("  character instances created               %u\n", instances);
    printf("  came back an EditText (the adapter)       %u\n", asEditText);
    printf("  came back something else                  %u\n", asGeneric);
    printf("  of those, laid out                        %u\n", laidOut);
    printf("  of those, produced glyph output           %u\n", withGlyphs);
    printf("  glyph entries %u, rasterised %u\n", totalGlyphs, totalRaster);

    delete def;
    free(data);
    if (fontData)
        free(fontData);
    // Two payloads in the cook carry no edit-text definition at all (HUDFX, GammaImage); that is
    // nothing to prove rather than a failure, so it is a pass.
    return (asEditText == editTextDefs && asGeneric == 0) ? 0 : 1;
}

// -------------------------------------------------------------------------------------------
// --xcheck: the two record walks against each other.
//
// This package's GFxConstShapeNoStyles::Read and package CD's GFxShapeRecord::Read are the same retail
// function (0xa42ab0) decompiled twice, on two different paths: mine reads glyph outlines out of a
// DefineFont tag, CD's reads DefineShape/2/3/4 with their style arrays. Neither can replace the other
// - CD's own GFxFontCharacterDef stores my class and my rasteriser consumes it - so rather than assert
// they agree, this runs both over the identical byte range of every glyph in a payload and compares
// the path and edge counts glyph by glyph. It also reports how many glyph records asked a no-style
// shape to own style arrays, which is the arm the two deleted skip helpers used to guess at.

int CmdXCheck(const char* path)
{
    unsigned char* data = 0;
    unsigned int size = 0;
    if (!LoadFile(path, &data, &size))
        return 2;

    GFxGfxFileInfo info;
    if (!GFxGfxParseFile(data, size, info))
    {
        printf("  parse failed   %s\n", info.Error);
        free(data);
        return 2;
    }
    printf("  payload        %s  %u bytes\n", BaseName(path), size);

    unsigned int fonts = 0, glyphs = 0, agree = 0, differ = 0, refused = 0, noRange = 0;
    unsigned int minePaths = 0, mineEdges = 0, cdPaths = 0, cdEdges = 0;

    GFxStream s(data, size);
    s.SetPosition(info.FirstTagOffset);
    while (s.Tell() < size)
    {
        unsigned int code = 0, tagEnd = 0;
        if (!s.OpenTag(&code, &tagEnd))
            break;
        if (code == GFxTag_End)
            break;
        if (code == GFxTag_DefineFont || code == GFxTag_DefineFont2 || code == GFxTag_DefineFont3)
        {
            ++fonts;
            s.ReadU16();                            // character id
            GPtr<GFxFontData> font = new GFxFontData;
            if (font->Read(&s, code, tagEnd))
            {
                const unsigned int shapeTag = (code == GFxTag_DefineFont)  ? GFxTag_DefineShape
                                            : (code == GFxTag_DefineFont2) ? GFxTag_DefineShape2
                                                                           : code;
                for (unsigned int g = 0; g < font->GetGlyphShapeCount(); ++g)
                {
                    GPtr<GFxShapeBase> mine = font->GetGlyphShape(g, 0);
                    if (!mine)
                        continue;
                    ++glyphs;
                    unsigned int mp = mine->GetPathCount(), me = 0;
                    for (unsigned int p = 0; p < mp; ++p)
                        me += mine->GetPath(p).EdgeCount;
                    minePaths += mp;
                    mineEdges += me;
                    refused += mine->StyleRecordsRefused;

                    unsigned int gs = 0, ge = 0;
                    if (!font->GetGlyphRange(g, &gs, &ge))
                    {
                        ++noRange;
                        continue;
                    }
                    GFxStream s2(data, size);
                    s2.SetPosition(gs);
                    GFxShapeRecord rec;
                    rec.Read(&s2, shapeTag, ge, 0);
                    const unsigned int cp = rec.GetPathCount();
                    const unsigned int ce = rec.GetEdgeCount();
                    cdPaths += cp;
                    cdEdges += ce;
                    if (cp == mp && ce == me)
                    {
                        ++agree;
                    }
                    else
                    {
                        if (differ < 8)
                            printf("    glyph %-4u mine %u paths / %u edges,  CD %u paths / %u edges\n",
                                   g, mp, me, cp, ce);
                        ++differ;
                    }
                }
            }
        }
        s.CloseTag();
    }

    printf("  fonts          %u,  glyphs compared %u\n", fonts, glyphs);
    printf("  mine           %u paths, %u edges\n", minePaths, mineEdges);
    printf("  CD's walk      %u paths, %u edges\n", cdPaths, cdEdges);
    printf("  agree          %u   differ %u   no range %u\n", agree, differ, noRange);
    printf("  style arrays a no-style shape had to refuse: %u\n", refused);
    free(data);
    return differ ? 1 : 0;
}

// -------------------------------------------------------------------------------------------
// --table

struct Row { const char* Name; int RetailFns; int Done; const char* Note; };

// The retail function counts are from resources/docs/symbols/functions.csv, module libgfx, grouped by
// the demangled owning class - build/agentCB/surf2.py prints the same table. `Done` is the number of
// those functions this package implements for real; a stub with a documented body does not count.
const Row Rows[] =
{
    // geometry, the input to the rasteriser
    { "GFxShapeBase",              30, 8,  "MakeCompoundShape, ComputeBound, the bound cache; the tessellator and Display are CC's" },
    { "GFxConstShapeNoStyles",     12, 4,  "Read (the SWF SHAPE record walk), MakeCompoundShape, GetShapeAndPathCounts" },
    { "GFxSwfPathData iterators",  13, 0,  "retail decodes the raw bytes lazily; we decode once at load (GFxShape.h, DEVIATION)" },
    { "GFxPathPacker/Allocator",   15, 0,  "the packed GFxPathData form; the const form is what fonts use" },
    { "GCompoundShape",            14, 11, "BeginPath x2, AddCurve, ClosePath, flattenQuadraticCurve, ScaleAndTranslate, PerceiveBounds, SetCurveTolerance, Clear, AddVertex" },
    { "GTessellator",              60, 0,  "triangulation: the renderer's, package CC" },
    // the rasteriser
    { "GRasterizer",               13, 13, "all of it: MoveTo/LineTo/ClosePolygon/line/horLine/SortCells/SweepScanline/SetGamma/AddShape/AddShapeScaled/Clear/ctor/cellXLess" },
    { "GFxGlyphRasterCache",       19, 8,  "CalcGlyphParam, Init, Clear, GetGlyph, rasterizeAndPack's raster core, filterScanline, snapFontSizeToRamp, releaseAllTextures" },
    { "GFxGlyphSlotQueue",         21, 3,  "the shelf allocation; the LRU extrusion and slot merging are not ported" },
    { "GFxGlyphFitter",             9, 0,  "auto-hinting" },
    { "GFxFontGlyphPacker",         9, 0,  "the offline atlas baker: ExportFlags is 0 in every cooked asset, so it is unreachable" },
    { "GFxTextureGlyph(Data)",     10, 0,  "pre-baked font textures; there is no 1002/1005 tag anywhere in the cook" },
    // fonts
    { "GFxFontData",               15, 11, "Read (DefineFont/2/3), ReadCodeTable, GetGlyphShape/Bounds/Advance/Width/Height, GetKerningAdjustment, GetGlyphIndex, GetCharValue, HasVectorOrRasterGlyphs" },
    { "GFxFontData::ReadFontInfo",  1, 0,  "tags 13 and 62; no asset in the cook carries one" },
    { "GFxFontDataCompactedSwf/Gfx",29, 0, "GFx's own compacted glyph stream; the cook is plain DefineFont3" },
    { "GFxFontCompactor",          22, 0,  "the writer for the above" },
    { "GFxFontResource",           13, 8,  "ctor, GetResourceTypeCode, GetLowerCaseTop, GetUpperCaseTop, calcLowerUpperTop, calcTopBound, the forwarders" },
    { "GFxFontHandle",              4, 3,  "ctor, operator==, dtor" },
    { "GFxFontManager",            12, 6,  "ctor, AddFont, FindFontResource, CreateFontHandle, GetEmptyFont, dtor" },
    { "GFxFontLib",                 5, 1,  "the registration half of AddFontsFrom" },
    { "GFxFontMap",                 4, 0,  "the name substitution table" },
    { "GFxFontCacheManager(Impl)", 23, 0,  "the batch package: it submits through GRenderer, package CC" },
    { "GFxTextureFont",             9, 0,  "unreachable, see GFxTextureGlyph" },
    // the text model
    { "GFxTextFormat",             33, 24, "InitByDefaultValues, every setter, Merge, Intersection, operator==, Hash, IsFontSame" },
    { "GFxTextParagraphFormat",    24, 17, "InitByDefaultValues, SetAlignment, the three alignment predicates, the margins, Merge, Intersection, operator==, Hash, SetBullet, IsBullet" },
    { "GFxTextAllocator",           7, 3,  "AllocateTextFormat, AllocateParagraphFormat (interning); the two cache flushes are not ported" },
    { "GFxTextParagraph",          17, 12, "SetText, InsertString, Remove, SetFormat, SetTextFormat, GetTextFormatPtr, HasNewLine, GetLength, Clear" },
    { "GFxStyledText",             41, 16, "AppendString x2, SetText x2, Clear, GetLength, GetText, AppendNewParagraph, the default formats, SetTextFormat, SetParagraphFormat" },
    { "GFxStyledText::ParseHtml*",  4, 0,  "6,268 bytes of HTML parser; the tags are stripped instead and the site says so" },
    { "GFxTextStyleManager",       10, 0,  "the CSS style sheet" },
    // layout
    { "GFxTextLineBuffer",         21, 9,  "the line store, InsertNewLine, RemoveLines, GetMinLineHeight, CalcLineSize, the Line and GlyphEntry accessors; Display and DrawUnderline are CC's" },
    { "GFxLineCursor",              8, 3,  "ctor, Reset, TrackFontParams" },
    { "GFxParagraphFormatter",      9, 6,  "Format, InitParagraph, FinalizeLine, CheckWordWrap, GetActualFontSize, ctor; the custom word-wrap callback is not ported" },
    { "GFxTextDocView",            97, 31, "Format, SetViewRect, the text and format setters, FindFont, GetTextWidth/Height, GetLinesCount, GetLineMetrics, the flag setters, ContainsNonLeftAlignment" },
    { "GFxTextDocView filters",    22, 0,  "the shadow and blur properties: they are the glyph cache's filter path, which is not ported" },
    { "GFxTextEditorKit",          31, 0,  "the caret, the selection and the key handling: needs the input path" },
    { "GFxTextHighlighter",        14, 0,  "selection highlighting" },
    { "GFxTextCompositionString",  11, 0,  "IME" },
    { "GFxTextClipboard/KeyMap",   11, 0,  "editing" },
    // the field
    { "GFxEditTextCharacter",     125, 14, "ctor, GetInitialFormats, SetInitialFormatsAsDefault, SetTextValue, SetText, GetTextValue, AdvanceFrame, OnEventLoad, GetObjectType, GetMember and SetMember for 13 properties, ProduceGlyphs, SetDirtyFlag" },
    { "GFxEditTextCharacterDef",    7, 3,  "the DefineEditText record: ported here as GFxTextFieldReadDesc and by package CD as its own def class" },
    { "GFxEditTextCharacter AS2",  26, 0,  "the GASFnCall methods: replaceSel, getTextFormat, getLineMetrics, the clipboard, the image substitution" },
    { "GFxStaticTextCharacter",    15, 0,  "DefineText: no asset in the cook carries one (CD measured this)" },
    { "GASTextFieldObject/Proto",  16, 0,  "the AS2 TextField class object: needs the class library, package BC's area" },
    { "GASTextFormatObject/Proto", 13, 0,  "the AS2 TextFormat class" },
    { "GASTextSnapshot*",          19, 0,  "TextSnapshot" },
    { "GASStyleSheet*",            23, 0,  "StyleSheet" }
};

void CmdTable()
{
    int totalRetail = 0, totalDone = 0;
    printf("  %-30s %8s %8s %6s  %s\n", "class / group", "retail", "ported", "%", "note");
    for (unsigned int i = 0; i < sizeof(Rows) / sizeof(Rows[0]); ++i)
    {
        const Row& r = Rows[i];
        totalRetail += r.RetailFns;
        totalDone += r.Done;
        printf("  %-30s %8d %8d %5.0f%%  %s\n", r.Name, r.RetailFns, r.Done,
               r.RetailFns ? 100.0 * r.Done / r.RetailFns : 0.0, r.Note);
    }
    printf("  %-30s %8d %8d %5.1f%%\n", "TOTAL", totalRetail, totalDone,
           totalRetail ? 100.0 * totalDone / totalRetail : 0.0);
}

} // namespace

int main(int argc, char** argv)
{
    const char* fonts = 0;
    const char* raster = 0;
    const char* run = 0;
    const char* layout = 0;
    const char* xcheck = 0;
    const char* defs = 0;
    float boxW = 300.0f, boxH = 100.0f;
    bool noWrap = false;
    const char* fontlib = 0;
    const char* dumpDir = 0;
    const char* text = "The Outsider's Mark";
    float sizePx = 24.0f;
    bool verbose = false;
    bool table = false;

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--fonts") && i + 1 < argc)        fonts = argv[++i];
        else if (!strcmp(argv[i], "--raster") && i + 1 < argc)  raster = argv[++i];
        else if (!strcmp(argv[i], "--run") && i + 1 < argc)     run = argv[++i];
        else if (!strcmp(argv[i], "--layout") && i + 1 < argc)  layout = argv[++i];
        else if (!strcmp(argv[i], "--xcheck") && i + 1 < argc)  xcheck = argv[++i];
        else if (!strcmp(argv[i], "--defs") && i + 1 < argc)    defs = argv[++i];
        else if (!strcmp(argv[i], "--box") && i + 2 < argc)
        {
            boxW = (float)atof(argv[++i]);
            boxH = (float)atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "--nowrap"))                  noWrap = true;
        else if (!strcmp(argv[i], "--fontlib") && i + 1 < argc) fontlib = argv[++i];
        else if (!strcmp(argv[i], "--dump") && i + 1 < argc)    dumpDir = argv[++i];
        else if (!strcmp(argv[i], "--string") && i + 1 < argc)  text = argv[++i];
        else if (!strcmp(argv[i], "--size") && i + 1 < argc)    sizePx = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "--verbose"))                 verbose = true;
        else if (!strcmp(argv[i], "--table"))                   table = true;
        else
        {
            printf("GFx3Text: unknown argument '%s'\n", argv[i]);
            return 2;
        }
    }

    if (!fonts && !raster && !run && !layout && !xcheck && !defs && !table)
    {
        printf("GFx3Text - agent CB's text engine and glyph rasteriser harness\n"
               "  --fonts <payload.gfx> [--verbose]\n"
               "  --raster <payload.gfx> [--string S] [--size N] [--dump <dir>]\n"
               "  --run <asset.gfx> [--fontlib <payload.gfx>] [--verbose]\n"
               "  --table\n");
        return 2;
    }

    int rc = 0;
    if (fonts)
    {
        printf("== fonts ==\n");
        rc |= CmdFonts(fonts, verbose);
    }
    if (raster)
    {
        printf("\n== raster ==\n");
        rc |= CmdRaster(raster, text, sizePx, dumpDir);
    }
    if (run)
    {
        printf("\n== run ==\n");
        rc |= CmdRun(run, fontlib, sizePx, verbose);
    }
    if (layout)
    {
        printf("\n== layout ==\n");
        rc |= CmdLayout(layout, text, sizePx, boxW, boxH, !noWrap);
    }
    if (defs)
    {
        printf("\n== defs ==\n");
        rc |= CmdDefs(defs, fontlib, verbose);
    }
    if (xcheck)
    {
        printf("\n== xcheck ==\n");
        rc |= CmdXCheck(xcheck);
    }
    if (table)
    {
        printf("\n== implemented against remaining ==\n");
        CmdTable();
    }
    GFxTextResetFontManager();
    return rc;
}
