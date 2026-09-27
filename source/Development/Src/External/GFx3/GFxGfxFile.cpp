// Scaleform GFx 3.3.89 - the cooked container parser. See GFxGfxFile.h for the format and for the
// measurements it was written against (build/agentBB/movies.json, all 22 retail payloads).
// DISHONORED(written): resources/docs/agents/agentBB.md.
#include "GFxGfxFile.h"

#include <string.h>

namespace
{

class Reader
{
public:
    const GUByte* D;
    unsigned int  N;
    unsigned int  O;
    unsigned int  BitPos;
    bool          Bad;

    Reader(const void* d, unsigned int n) : D((const GUByte*)d), N(n), O(0), BitPos(0), Bad(false) {}

    bool Left(unsigned int n) const { return O + n <= N; }

    GUByte U8()
    {
        if (!Left(1)) { Bad = true; return 0; }
        return D[O++];
    }
    unsigned short U16()
    {
        if (!Left(2)) { Bad = true; return 0; }
        unsigned short v = (unsigned short)(D[O] | (D[O + 1] << 8));
        O += 2;
        return v;
    }
    unsigned int U32()
    {
        if (!Left(4)) { Bad = true; return 0; }
        unsigned int v = (unsigned int)D[O] | ((unsigned int)D[O + 1] << 8)
                       | ((unsigned int)D[O + 2] << 16) | ((unsigned int)D[O + 3] << 24);
        O += 4;
        return v;
    }
    // Byte-align after a bit field: the SWF RECT is 5 + 4*nbits bits, so the reader is mid-byte
    // when it ends and the frame rate that follows starts at the next byte boundary.
    void Align() { if (BitPos) { BitPos = 0; ++O; } }

    unsigned int Bits(unsigned int n)
    {
        unsigned int v = 0;
        for (unsigned int i = 0; i < n; ++i)
        {
            if (O >= N) { Bad = true; return v; }
            unsigned int bit = (D[O] >> (7 - BitPos)) & 1;
            v = (v << 1) | bit;
            if (++BitPos == 8) { BitPos = 0; ++O; }
        }
        return v;
    }
    int SBits(unsigned int n)
    {
        if (n == 0) return 0;
        unsigned int v = Bits(n);
        if (v & (1u << (n - 1))) return (int)(v | ~((1u << n) - 1));
        return (int)v;
    }

    // A NUL-terminated string, which is how ExportAssets, ImportAssets2 and the exporter tags that
    // are not length-prefixed spell a name.
    void CStr(char* out, unsigned int cap)
    {
        unsigned int i = 0;
        for (;;)
        {
            GUByte c = U8();
            if (Bad || c == 0) break;
            if (i + 1 < cap) out[i++] = (char)c;
        }
        out[i < cap ? i : cap - 1] = 0;
    }
    // A u8 length-prefixed string, which is how tag 1000 and tag 1009 spell theirs.
    void PStr(char* out, unsigned int cap)
    {
        unsigned int n = U8();
        unsigned int i = 0;
        for (unsigned int k = 0; k < n; ++k)
        {
            GUByte c = U8();
            if (Bad) break;
            if (i + 1 < cap) out[i++] = (char)c;
        }
        out[i < cap ? i : cap - 1] = 0;
    }
};

bool IsStandardSwfTag(unsigned int code)
{
    return code <= 91;
}

bool IsEmbeddedBitmapTag(unsigned int code)
{
    return code == GFxTag_DefineBits || code == GFxTag_DefineBitsLossless
        || code == GFxTag_DefineBitsJPEG2 || code == GFxTag_DefineBitsJPEG3
        || code == GFxTag_DefineBitsLossless2 || code == GFxTag_DefineBitsJPEG4
        || code == GFxTag_JPEGTables;
}

bool IsGlyphTag(unsigned int code)
{
    return code == GFxTag_DefineFont || code == GFxTag_DefineFont2 || code == GFxTag_DefineFont3;
}

void BumpTagCode(GFxGfxFileInfo& info, unsigned int code)
{
    for (unsigned int i = 0; i < info.TagCodeCount; ++i)
    {
        if (info.TagCodes[i].Code == code)
        {
            ++info.TagCodes[i].Count;
            return;
        }
    }
    if (info.TagCodeCount < GFxGfxFileInfo::MaxTagCodes)
    {
        info.TagCodes[info.TagCodeCount].Code = code;
        info.TagCodes[info.TagCodeCount].Count = 1;
        ++info.TagCodeCount;
    }
}

} // namespace

void GFxGfxFileInfo::Reset()
{
    memset(this, 0, sizeof(*this));
}

unsigned int GFxGfxFileInfo::CountOfTag(unsigned int code) const
{
    for (unsigned int i = 0; i < TagCodeCount; ++i)
        if (TagCodes[i].Code == code)
            return TagCodes[i].Count;
    return 0;
}

bool GFxGfxParseFile(const void* data, unsigned int size, GFxGfxFileInfo& info)
{
    info.Reset();
    info.PayloadLength = size;
    if (data == 0 || size < 21)
    {
        strncpy(info.Error, "payload shorter than a GFX/SWF header (21 bytes)", sizeof(info.Error) - 1);
        return false;
    }

    Reader r(data, size);
    info.Signature[0] = (char)r.U8();
    info.Signature[1] = (char)r.U8();
    info.Signature[2] = (char)r.U8();
    info.Signature[3] = 0;
    info.Version = r.U8();
    info.DeclaredLength = r.U32();

    const char* sig = info.Signature;
    info.IsGfxExport = (sig[0] == 'G' && sig[1] == 'F' && sig[2] == 'X')
                    || (sig[0] == 'C' && sig[1] == 'F' && sig[2] == 'X');
    info.IsFlashSwf = (sig[1] == 'W' && sig[2] == 'S')
                   && (sig[0] == 'F' || sig[0] == 'C' || sig[0] == 'Z');
    info.IsCompressed = (sig[0] == 'C' && sig[1] == 'F' && sig[2] == 'X')
                     || (sig[0] == 'C' && sig[1] == 'W' && sig[2] == 'S')
                     || (sig[0] == 'Z' && sig[1] == 'W' && sig[2] == 'S');
    if (!info.IsGfxExport && !info.IsFlashSwf)
    {
        strncpy(info.Error, "signature is neither GFX/CFX nor FWS/CWS/ZWS", sizeof(info.Error) - 1);
        return false;
    }
    if (info.IsCompressed)
    {
        // Nothing in the retail cook is compressed (the u32 at +4 always equals the payload size),
        // so the header is all we can report without a zlib pass over the tail.
        strncpy(info.Error, "compressed container: the tag stream needs inflating first",
                sizeof(info.Error) - 1);
        return true;
    }

    r.Align();
    unsigned int nbits = r.Bits(5);
    info.FrameRectTwips[0] = r.SBits(nbits);   // left
    info.FrameRectTwips[1] = r.SBits(nbits);   // right
    info.FrameRectTwips[2] = r.SBits(nbits);   // top
    info.FrameRectTwips[3] = r.SBits(nbits);   // bottom
    r.Align();
    info.FrameWidthPixels = (info.FrameRectTwips[1] - info.FrameRectTwips[0]) / 20.0f;
    info.FrameHeightPixels = (info.FrameRectTwips[3] - info.FrameRectTwips[2]) / 20.0f;

    unsigned short rate = r.U16();
    info.FrameRate = rate / 256.0f;
    info.FrameCount = r.U16();
    info.FirstTagOffset = r.O;

    if (r.Bad)
    {
        strncpy(info.Error, "header truncated", sizeof(info.Error) - 1);
        return false;
    }

    // The tag stream.
    for (;;)
    {
        if (!r.Left(2))
        {
            info.Truncated = true;
            break;
        }
        unsigned int tagStart = r.O;
        unsigned short th = r.U16();
        unsigned int code = th >> 6;
        unsigned int len = th & 0x3F;
        if (len == 0x3F) len = r.U32();
        if (r.Bad || !r.Left(len))
        {
            info.Truncated = true;
            break;
        }
        unsigned int bodyStart = r.O;
        ++info.TagCount;
        BumpTagCode(info, code);
        if (!IsStandardSwfTag(code)) ++info.NonStandardTagCount;
        if (IsEmbeddedBitmapTag(code)) ++info.EmbeddedBitmapTagCount;
        if (IsGlyphTag(code)) ++info.GlyphTagCount;

        switch (code)
        {
        case GFxTag_ExportAssets:
        {
            unsigned int n = r.U16();
            for (unsigned int i = 0; i < n && !r.Bad; ++i)
            {
                unsigned short id = r.U16();
                char name[160];
                r.CStr(name, sizeof(name));
                if (info.ExportCount < GFxGfxFileInfo::MaxExports)
                {
                    GFxGfxExportedSymbol& e = info.Exports[info.ExportCount++];
                    e.CharacterId = id;
                    strncpy(e.Name, name, sizeof(e.Name) - 1);
                    e.Name[sizeof(e.Name) - 1] = 0;
                }
            }
            break;
        }
        case GFxTag_ImportAssets2:
        {
            char url[192];
            r.CStr(url, sizeof(url));
            r.U8();   // reserved, 1 in SWF 8+
            r.U8();   // reserved, 0
            unsigned int n = r.U16();
            for (unsigned int i = 0; i < n && !r.Bad; ++i)
            {
                unsigned short id = r.U16();
                char name[160];
                r.CStr(name, sizeof(name));
                if (info.ImportCount < GFxGfxFileInfo::MaxImports)
                {
                    GFxGfxImportedSymbol& im = info.Imports[info.ImportCount++];
                    im.CharacterId = id;
                    strncpy(im.Url, url, sizeof(im.Url) - 1);
                    im.Url[sizeof(im.Url) - 1] = 0;
                    strncpy(im.Name, name, sizeof(im.Name) - 1);
                    im.Name[sizeof(im.Name) - 1] = 0;
                }
            }
            break;
        }
        case GFxTag_GFxExporterInfo:
        {
            info.HasExporterInfo = true;
            GFxGfxExporterInfo& x = info.ExporterInfo;
            x.Version = r.U16();
            x.ExportFlags = r.U32();
            x.FileFormatType = r.U16();
            r.PStr(x.Prefix, sizeof(x.Prefix));
            r.PStr(x.SWFName, sizeof(x.SWFName));
            break;
        }
        case GFxTag_GFxDefineExternalImage2:
        {
            unsigned short idx = r.U16();
            unsigned short flags = r.U16();
            unsigned short fmt = r.U16();
            unsigned short w = r.U16();
            unsigned short h = r.U16();
            char exportName[128];
            char fileName[128];
            r.PStr(exportName, sizeof(exportName));
            r.PStr(fileName, sizeof(fileName));
            if (info.ImageCount < GFxGfxFileInfo::MaxImages)
            {
                GFxGfxExternalImage& im = info.Images[info.ImageCount++];
                im.ImageIndex = idx;
                im.Flags = flags;
                im.FileFormatType = fmt;
                im.SrcWidth = w;
                im.SrcHeight = h;
                strncpy(im.ExportName, exportName, sizeof(im.ExportName) - 1);
                im.ExportName[sizeof(im.ExportName) - 1] = 0;
                strncpy(im.FileName, fileName, sizeof(im.FileName) - 1);
                im.FileName[sizeof(im.FileName) - 1] = 0;
            }
            break;
        }
        case GFxTag_GFxDefineSubImage:
        {
            if (info.SubImageCount < GFxGfxFileInfo::MaxSubImages)
            {
                GFxGfxSubImage& s = info.SubImages[info.SubImageCount++];
                s.CharacterId = r.U16();
                s.ImageIndex = r.U16();
                s.X0 = r.U16();
                s.Y0 = r.U16();
                s.X1 = r.U16();
                s.Y1 = r.U16();
            }
            break;
        }
        default:
            break;
        }

        // Every tag body has an explicit length, so a mis-parse inside one cannot derail the walk.
        r.O = bodyStart + len;
        r.BitPos = 0;
        (void)tagStart;
        if (code == GFxTag_End)
        {
            info.ConsumedExactly = (r.O == size);
            break;
        }
    }
    return true;
}
