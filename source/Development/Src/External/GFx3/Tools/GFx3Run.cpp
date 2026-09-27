// Agent BC's acceptance harness for the AS2 machine. It drives the runtime from a cooked asset and
// nothing else: no engine, no renderer, no game.
//
//   GFx3Run --run <file.gfx> [--frames N] [--verbose] [--imports <dir>] [--platform PC]
//       parse the payload, instantiate its root movie clip, advance N frames (1 by default) and
//       report the characters created, the display-list operations, the action buffers executed, the
//       opcodes executed by code, and the AS2 classes the content registered.
//   GFx3Run --opcodes
//       the implemented-against-remaining opcode table, checked against the 98 case labels of
//       GASActionBuffer::Execute (2012 0x9ea900).
//   GFx3Run --classes
//       the implemented-against-remaining class-library table, with each remaining class's retail
//       function count.
//
// Payloads come out of the cooked *_SF.upk packages with build/agentBB/extract_gfx.py.
// DISHONORED(written): resources/docs/agents/agentBC.md.
#include "GFxCharacterDefs.h"
#include "GFxInput.h"
#include "GFxAS2Runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <new>
#include <intrin.h>

#ifdef _WIN32
// The whole GFx3 reconstruction compiles with /Zp4 (retail's packing) and the Windows headers
// static_assert against a non-default packing; nothing of theirs is shared with the runtime here.
#define WINDOWS_IGNORE_PACKING_MISMATCH
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Agent DG: where the machine died, as module-relative addresses. No debugger and no PDB needed;
// build/agentDG/map.py resolves them against GFx3Run.map.
// A throw out of the machine is caught in main(), so the unhandled filter never sees it and the
// stack is gone by then. A first-chance handler prints it at the throw site instead.
static LONG WINAPI GFxRunFirstChance(EXCEPTION_POINTERS* info)
{
    if (info->ExceptionRecord->ExceptionCode != 0xe06d7363)
        return EXCEPTION_CONTINUE_SEARCH;
    static bool bReported = false;
    if (bReported)
        return EXCEPTION_CONTINUE_SEARCH;
    bReported = true;
    void* frames[40];
    const USHORT n = CaptureStackBackTrace(0, 40, frames, 0);
    const char* base = (const char*)GetModuleHandleA(0);
    printf("\n== THROW at first chance (module base %p)\n", base);
    for (USHORT i = 0; i < n; ++i)
    {
        const char* f = (const char*)frames[i];
        if (f > base && f - base < 0x2000000)
            printf("   frame %2u  rva 0x%tx\n", i, f - base);
    }
    fflush(stdout);
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI GFxRunCrashFilter(EXCEPTION_POINTERS* info)
{
    void* frames[32];
    const USHORT n = CaptureStackBackTrace(0, 32, frames, 0);
    const char* base = (const char*)GetModuleHandleA(0);
    printf("\n== CRASH: code 0x%08lx at %p (module base %p)\n",
           info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress, base);
    printf("   rva 0x%tx\n", (const char*)info->ExceptionRecord->ExceptionAddress - base);
    for (USHORT i = 0; i < n; ++i)
        printf("   frame %2u  rva 0x%tx\n", i, (const char*)frames[i] - base);
    fflush(stdout);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef _WIN32
// Agent DG: an allocation this large is always a defect in the machine, never content. Printing
// the size and the return address turns "bad allocation" into a site.
void* operator new(size_t n)
{
    static size_t total = 0;
    static size_t nextReport = 128u << 20;
    total += n;
    if (total > nextReport)
    {
        nextReport = total + (128u << 20);
        const char* b = (const char*)GetModuleHandleA(0);
        const char* r = (const char*)_ReturnAddress();
        printf("\n== ALLOCATED %zu MB, last block %zu bytes from rva 0x%tx\n",
               total >> 20, n, r - b);
        fflush(stdout);
    }
    if (n > (64u << 20))
    {
        const char* base = (const char*)GetModuleHandleA(0);
        const char* ret = (const char*)_ReturnAddress();
        printf("\n== HUGE ALLOCATION %zu bytes from rva 0x%tx\n", n, ret - base);
        fflush(stdout);
    }
    void* p = malloc(n ? n : 1);
    if (p == 0)
        throw std::bad_alloc();
    return p;
}
void operator delete(void* p) noexcept { free(p); }
void operator delete(void* p, size_t) noexcept { free(p); }
void* operator new[](size_t n) { return operator new(n); }
void operator delete[](void* p) noexcept { free(p); }
void operator delete[](void* p, size_t) noexcept { free(p); }
#endif

namespace
{

// Which opcodes this implementation executes for real. An opcode listed here has a case in
// GASActionBuffer::Execute that does the work; the ones retail has and this does not are the
// complement, and the table prints both.
const unsigned char ImplementedOpcodes[] =
{
    GASop_End, GASop_NextFrame, GASop_PrevFrame, GASop_Play, GASop_Stop,
    GASop_Add, GASop_Subtract, GASop_Multiply, GASop_Divide, GASop_Equal, GASop_LessThan,
    GASop_LogicalAnd, GASop_LogicalOr, GASop_LogicalNot, GASop_StringEqual, GASop_StringLength,
    GASop_SubString, GASop_Pop, GASop_ToInteger, GASop_GetVariable, GASop_SetVariable,
    GASop_SetTargetExpression, GASop_StringConcat, GASop_GetProperty, GASop_SetProperty,
    GASop_DuplicateClip, GASop_RemoveClip, GASop_Trace, GASop_StringCompare, GASop_Throw,
    GASop_CastOp, GASop_ImplementsOp, GASop_Random, GASop_MBLength, GASop_Ord, GASop_Chr,
    GASop_GetTimer, GASop_MBSubString, GASop_MBOrd, GASop_MBChr, GASop_Delete, GASop_Delete2,
    GASop_DefineLocal, GASop_CallFunction, GASop_Return, GASop_Modulo, GASop_New,
    GASop_DeclareLocal, GASop_InitArray, GASop_InitObject, GASop_TypeOf, GASop_TargetPath,
    GASop_Enumerate, GASop_NewAdd, GASop_NewLessThan, GASop_NewEquals, GASop_ToNumber,
    GASop_ToString, GASop_Dup, GASop_Swap, GASop_GetMember, GASop_SetMember, GASop_Increment,
    GASop_Decrement, GASop_CallMethod, GASop_NewMethod, GASop_InstanceOf, GASop_Enumerate2,
    GASop_BitwiseAnd, GASop_BitwiseOr, GASop_BitwiseXor, GASop_ShiftLeft, GASop_ShiftRight,
    GASop_ShiftRightUnsigned, GASop_StrictEqual, GASop_Greater, GASop_StringGreater,
    GASop_Extends,
    GASop_GotoFrame, GASop_StoreRegister, GASop_ConstantPool, GASop_WaitForFrame, GASop_SetTarget,
    GASop_GotoLabel, GASop_WaitForFrameExpression, GASop_DefineFunction2, GASop_Try, GASop_With,
    GASop_Push, GASop_BranchAlways, GASop_DefineFunction, GASop_BranchIfTrue, GASop_CallFrame,
    GASop_GotoExpression
};

bool IsImplemented(unsigned char op)
{
    for (unsigned int i = 0; i < sizeof(ImplementedOpcodes); ++i)
        if (ImplementedOpcodes[i] == op)
            return true;
    return false;
}

struct ClassRow { const char* Name; int RetailFns; bool Done; const char* Note; };

// The retail function counts are from resources/docs/symbols/functions.csv, module libgfx, grouped by
// the demangled class name (build/agentBC/surface.py prints the same table).
const ClassRow Classes[] =
{
    { "Object",          32, true,  "prototype + registerClass + addProperty" },
    { "Function",         8, true,  "call, apply" },
    { "Array",          129, true,  "GASArrayObject 30 + GASArrayProto 99" },
    { "String",          35, true,  "GASString 18 + GASStringProto 17" },
    { "Number",          14, true,  "GASNumberUtil" },
    { "Boolean",          7, true,  "" },
    { "Math",            20, true,  "GASMathCtorFunction" },
    { "MovieClip",       16, true,  "GASMovieClipObject 11 + GASMovieClipProto 3 + ctor 2" },
    { "Error",            4, true,  "" },
    { "_global freefns", 12, true,  "ASSetPropFlags, trace, parseInt/Float, isNaN, setInterval" },
    { "Date",           109, false, "GASDate 68 + GASDateProto 41" },
    { "Key",             12, false, "GASKeyObject; needs the input path" },
    { "Mouse",            9, false, "needs the input path" },
    { "Selection",       19, false, "GASSelectionCtorFunction; needs the text engine" },
    { "TextField",      125, false, "GFxEditTextCharacter; needs the text engine" },
    { "TextFormat",      33, false, "GFxTextFormat" },
    { "StyleSheet",      14, false, "GASStyleSheetObject" },
    { "Stage",           34, false, "GASStageCtorFunction + GASStageProto" },
    { "Color",            9, false, "" },
    { "Sound",           18, false, "audio is out of scope (PLAN.md Phase 10)" },
    { "XML / XMLNode",   47, false, "" },
    { "LoadVars",        26, false, "needs the loader queue" },
    { "MovieClipLoader", 16, false, "needs the loader queue" },
    { "BitmapData",      22, false, "needs the renderer" },
    { "Matrix/Point/Rectangle/Transform", 55, false, "GASMatrixProto 16 + GASRectangleProto 17 + ..." },
    { "the filter classes", 48, false, "Drop shadow, glow, bevel, colour matrix, blur" },
    { "SharedObject",    11, false, "" },
    { "NetConnection / NetStream", 14, false, "video, out of scope" }
};

int LoadFile(const char* path, unsigned char** outData, unsigned int* outSize);

// The import resolver. A GFX payload's ImportAssets URL is a relative authoring path -
// '..\DisFonts\gfxfontlib.swf' or '../common_assets/lib.swf' - and what it has to resolve to is the
// cooked movie that exports the symbol. In the engine that mapping is a package lookup behind
// FGFxFileOpener (agent BB's seam); in this harness the payloads are files named
// <package>.<movie>.gfx, so the URL's basename minus its extension is matched against the movie half
// of the file name. Both are the same function - take an authoring URL, produce a movie - which is
// why GFxMovieDataDef::ImportResolver is the seam rather than a file path.
class DirImportResolver : public GFxMovieDataDef::ImportResolver
{
public:
    DirImportResolver(const char* dir) : Dir(dir), Count(0) {}
    ~DirImportResolver()
    {
        // The imported movies outlive the movie that imported them, because an imported definition
        // stays owned by its exporter; the resolver is that owner and it is destroyed last.
        for (unsigned int i = 0; i < Count; ++i)
        {
            delete Cache[i].pDef;
            free(Cache[i].pData);
        }
    }

    virtual GFxMovieDataDef* ResolveImportMovie(const char* url)
    {
        char stem[160];
        BaseStem(url, stem, sizeof(stem));
        for (unsigned int i = 0; i < Count; ++i)
            if (_stricmp(Cache[i].Stem, stem) == 0)
                return Cache[i].pDef;
        if (Count >= MaxCache)
            return 0;

        char path[512];
        if (!FindPayload(stem, path, sizeof(path)))
            return 0;
        unsigned char* data = 0;
        unsigned int size = 0;
        if (LoadFile(path, &data, &size) != 0)
            return 0;
        GFxMovieDataDef* def = new GFxMovieDataDef;
        if (!def->Read(data, size))
        {
            delete def;
            free(data);
            return 0;
        }
        Entry& e = Cache[Count++];
        strncpy(e.Stem, stem, sizeof(e.Stem) - 1);
        e.Stem[sizeof(e.Stem) - 1] = 0;
        e.pDef = def;
        e.pData = data;
        // An imported movie has imports of its own - every UI movie imports the font library through
        // lib.swf - so the bind is transitive, exactly as GFxLoadStates::CloneForImport (2012
        // 0xa240b0) makes it in retail.
        def->BindImports(this);
        return def;
    }

    unsigned int GetMovieCount() const { return Count; }
    const char*  GetMovieStem(unsigned int i) const { return Cache[i].Stem; }

private:
    enum { MaxCache = 16 };
    struct Entry { char Stem[160]; GFxMovieDataDef* pDef; unsigned char* pData; };

    static void BaseStem(const char* url, char* out, unsigned int outSize)
    {
        const char* base = url;
        for (const char* c = url; *c; ++c)
            if (*c == '/' || *c == '\\')
                base = c + 1;
        unsigned int n = 0;
        for (const char* c = base; *c && n + 1 < outSize; ++c)
        {
            if (*c == '.')
                break;
            out[n++] = *c;
        }
        out[n] = 0;
    }

    // <package>.<movie>.gfx, matched on the movie half. Two packages export lib.swf's symbols
    // (DishonoredGame.lib and Startup.lib) and they are byte-identical payloads, so the first match
    // wins and the report says which one was taken.
    bool FindPayload(const char* stem, char* out, unsigned int outSize) const
    {
        static const char* const Packages[] =
        {
            "DishonoredGame", "Startup", "DisFonts_SF", "Dishonored_MainMenu",
            "UI_HUD_SF", "UI_PauseMenu_SF", "UI_Shop_SF", "UI_Journal_SF",
            "UI_PowerWheel_SF", "UI_MissionStats_SF", "UI_Gamma_SF"
        };
        for (unsigned int i = 0; i < sizeof(Packages) / sizeof(Packages[0]); ++i)
        {
            _snprintf(out, outSize, "%s/%s.%s.gfx", Dir, Packages[i], stem);
            out[outSize - 1] = 0;
            FILE* f = fopen(out, "rb");
            if (f != 0)
            {
                fclose(f);
                return true;
            }
        }
        return false;
    }

    const char*  Dir;
    Entry        Cache[MaxCache];
    unsigned int Count;
};

int LoadFile(const char* path, unsigned char** outData, unsigned int* outSize)
{
    FILE* f = fopen(path, "rb");
    if (f == 0)
    {
        printf("cannot open %s\n", path);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc((size_t)size);
    size_t read = fread(data, 1, (size_t)size, f);
    fclose(f);
    if (read != (size_t)size)
    {
        printf("short read on %s\n", path);
        free(data);
        return 1;
    }
    *outData = data;
    *outSize = (unsigned int)size;
    return 0;
}

// The names are copied into plain buffers rather than kept as GASStrings: an interned node belongs to
// the movie's string manager, so holding one past the movie's release is a use-after-free. Retail has
// the same rule and it is worth saying out loud, because it is the mistake a consumer of this runtime
// will make first.
class ClassLister : public GASObjectInterface::MemberVisitor
{
public:
    ClassLister() : Count(0) {}
    virtual void Visit(const GASString& name, const GASValue& val, unsigned char flags)
    {
        if (Count < 512)
        {
            strncpy(Names[Count], name.ToCStr(), sizeof(Names[0]) - 1);
            Names[Count][sizeof(Names[0]) - 1] = 0;
        }
        ++Count;
    }
    char         Names[512][80];
    unsigned int Count;
};

void PrintOpcodeTable()
{
    const unsigned char* retail = GFxAS2GetRetailOpcodes();
    unsigned int n = GFxAS2GetRetailOpcodeCount();
    unsigned int done = 0;
    printf("retail opcodes (case labels of GASActionBuffer::Execute, 2012 0x9ea900): %u\n", n);
    printf("%-6s %-24s %s\n", "code", "name", "state");
    for (unsigned int i = 0; i < n; ++i)
    {
        bool impl = IsImplemented(retail[i]);
        if (impl) ++done;
        printf("0x%02X   %-24s %s\n", retail[i], GFxAS2GetOpcodeName(retail[i]),
               impl ? "implemented" : "REMAINING");
    }
    printf("\nimplemented %u of %u (%.1f %%); remaining %u\n", done, n,
           100.0 * (double)done / (double)n, n - done);
}

void PrintClassTable()
{
    int doneFns = 0, totalFns = 0, doneCount = 0;
    const int rows = (int)(sizeof(Classes) / sizeof(Classes[0]));
    printf("%-34s %-8s %-12s %s\n", "class", "retailFns", "state", "note");
    for (int i = 0; i < rows; ++i)
    {
        totalFns += Classes[i].RetailFns;
        if (Classes[i].Done) { doneFns += Classes[i].RetailFns; ++doneCount; }
        printf("%-34s %-8d %-12s %s\n", Classes[i].Name, Classes[i].RetailFns,
               Classes[i].Done ? "implemented" : "REMAINING", Classes[i].Note);
    }
    printf("\nimplemented %d of %d classes, %d of %d retail functions worth (%.1f %%)\n",
           doneCount, rows, doneFns, totalFns, 100.0 * (double)doneFns / (double)totalFns);
}

// Agent DG: one scripted step, "<frame>:<what>". `--invoke 2:_root.startScreen_mc.Open` calls that
// path after the second advance; `--key 5:40` delivers a key-down/key-up pair for key code 40 after
// the fifth. Between them they are the whole menu flow - PostStart's Open, a key press, the
// selection move - driven without the game.
struct GFxRunStep
{
    unsigned int Frame;
    const char*  Text;
    bool         bKey;
};

int RunMovie(const char* path, unsigned int frames, bool verbose,
             DirImportResolver* imports, const char* platform,
             const GFxRunStep* steps, unsigned int stepCount)
{
    unsigned char* data = 0;
    unsigned int size = 0;
    if (LoadFile(path, &data, &size) != 0)
        return 1;

    GFxMovieDataDef* dataDef = new GFxMovieDataDef;
    if (!dataDef->Read(data, size))
    {
        printf("parse failed: %s\n", dataDef->GetFileInfo().Error);
        free(data);
        delete dataDef;
        return 1;
    }

    // Import binding, before anything instantiates a character: retail does it in
    // GFxMovieBindProcess, between the data def's parse and the movie def's creation, which is why
    // GFxMovieDefImpl::CreateInstance can rely on every dictionary slot being real.
    unsigned int importsBound = 0;
    if (imports != 0)
        importsBound = dataDef->BindImports(imports);

    const GFxGfxFileInfo& info = dataDef->GetFileInfo();
    const GFxMovieDataDef::ReadStats& rs = dataDef->GetReadStats();

    printf("== %s\n", path);
    printf("  header         %c%c%c v%u  %.0f x %.0f px  %.1f fps  %u frames\n",
           info.Signature[0], info.Signature[1], info.Signature[2], (unsigned int)info.Version,
           info.FrameWidthPixels, info.FrameHeightPixels, info.FrameRate, info.FrameCount);
    printf("  tags           %u total, %u handled, %u skipped by length\n",
           rs.Tags, rs.TagsHandled, rs.TagsSkipped);
    printf("  dictionary     %u characters (%u sprites, %u placeholders)\n",
           rs.Characters, rs.Sprites, rs.Placeholders);
    printf("  definitions    %u shape, %u morph, %u edittext, %u text, %u button, %u font, %u image\n",
           rs.Shapes, rs.MorphShapes, rs.EditTexts, rs.StaticTexts, rs.Buttons, rs.Fonts, rs.Images);
    printf("  exports        %u   imports %u (%u bound, %u this pass)   unhandled tags %u\n",
           rs.Exports, rs.Imports, rs.ImportsBound, importsBound, rs.Unhandled);

    // The shape geometry, which is the point of the loaders: counting definitions proves only that a
    // dictionary slot was filled, while the path and edge totals prove the records were decoded. A
    // loader that consumed no bytes would show definitions and zero paths.
    {
        unsigned int paths = 0, edges = 0, fills = 0, lines = 0, curves = 0, glyphs = 0;
        unsigned int newStyleShapes = 0;
        unsigned int fields = 0, withFont = 0, buttons = 0, buttonRecs = 0, images = 0, subImages = 0;
        for (unsigned int i = 0; i < dataDef->GetDictSize(); ++i)
        {
            GFxCharacterDef* def = dataDef->GetDictDef(i);
            if (def == 0)
                continue;
            switch (def->GetResourceTypeCode())
            {
            case GFxResource::RT_ShapeDef:
            {
                if (strcmp(def->GetDefTypeName(), "MorphShape") == 0)
                    break;
                GFxShapeCharacterDef* sh = (GFxShapeCharacterDef*)def;
                if (sh->Shape.ShapeCount > 1)
                    ++newStyleShapes;
                paths += sh->GetPathCount();
                edges += sh->GetEdgeCount();
                fills += sh->GetFillStyleCount();
                lines += sh->GetLineStyleCount();
                for (unsigned int p = 0; p < sh->GetPathCount(); ++p)
                {
                    const GFxShapePathCD* pp = sh->Shape.GetPath(p);
                    for (unsigned int e = 0; e < pp->EdgeCount; ++e)
                        if (pp->Edges[e].bCurve)
                            ++curves;
                }
                break;
            }
            case GFxResource::RT_EditTextDef:
                ++fields;
                if (((GFxEditTextCharacterDef*)def)->FontId != GFxResourceId::InvalidId)
                    ++withFont;
                break;
            case GFxResource::RT_ButtonDef:
                ++buttons;
                buttonRecs += ((GFxButtonCharacterDef*)def)->RecordCount;
                break;
            case GFxResource::RT_Font:
                glyphs += ((GFxFontCharacterDef*)def)->GetGlyphCount();
                break;
            case GFxResource::RT_Image:
                ++images;
                if (((GFxImageCharacterDef*)def)->bIsSubImage)
                    ++subImages;
                break;
            default:
                break;
            }
        }
        // The cross-check that the fill records were decoded and not merely consumed: every bitmap
        // fill names a character id, and in a well-formed payload that id is an image definition in
        // the same dictionary. A misaligned fill reader produces ids that resolve to nothing.
        unsigned int bmpFills = 0, bmpResolved = 0, bmpNull = 0, gradFills = 0;
        unsigned int badFills = 0;
        for (unsigned int i = 0; i < dataDef->GetDictSize(); ++i)
        {
            GFxCharacterDef* def = dataDef->GetDictDef(i);
            if (def == 0 || def->GetResourceTypeCode() != GFxResource::RT_ShapeDef ||
                strcmp(def->GetDefTypeName(), "Shape") != 0)
                continue;
            GFxShapeCharacterDef* sh = (GFxShapeCharacterDef*)def;
            for (unsigned int k = 0; k < sh->GetFillStyleCount(); ++k)
            {
                const GFxFillStyle* f = sh->GetFillStyle(k);
                // A type retail does not branch on means the style array was misaligned; it is the
                // cheapest direct symptom there is, so it is counted rather than inferred from the
                // bitmap ids downstream.
                if (f->Type != GFxFill_Solid && (f->Type & 0x10) == 0 && (f->Type & 0x40) == 0)
                    ++badFills;
                if (f->IsGradient())
                    ++gradFills;
                if (!f->IsImage())
                    continue;
                // 0xFFFF is the SWF null character id: a bitmap fill with no bitmap, which the
                // exporter leaves behind when the image was stripped. It is not a failed lookup.
                if (f->ImageId == 0xFFFFu)
                {
                    ++bmpNull;
                    continue;
                }
                ++bmpFills;
                GFxCharacterDef* img = dataDef->GetCharacterDefById(f->ImageId);
                if (img != 0 && img->GetResourceTypeCode() == GFxResource::RT_Image)
                    ++bmpResolved;
                else if (verbose)
                    printf("    bitmap fill id %-6u -> %s\n", f->ImageId,
                           img != 0 ? img->GetDefTypeName() : "nothing in the dictionary");
            }
        }
        printf("  shape geometry %u paths, %u edges (%u quadratic), %u fill styles, %u line styles,"
               " %u shapes with a NewStyles record\n",
               paths, edges, curves, fills, lines, newStyleShapes);
        printf("  fill styles    %u bitmap (%u resolve to an image def, %u with the null id),"
               " %u gradient, %u with an unknown type\n",
               bmpFills, bmpResolved, bmpNull, gradFills, badFills);
        printf("  other defs     %u text fields (%u with a font), %u buttons with %u state records,"
               " %u images (%u sub), %u glyphs\n",
               fields, withFont, buttons, buttonRecs, images, subImages, glyphs);
    }
    if (verbose)
        for (unsigned int i = 0; i < dataDef->GetImportCount(); ++i)
        {
            const GFxMovieDataDef::ImportEntry& e = dataDef->GetImport(i);
            printf("    import %-5s id %-5u %-28s from %s\n", e.bBound ? "bound" : "UNRES",
                   e.Id, e.Symbol, e.Url);
        }
    printf("  actions        %u DoAction, %u DoInitAction, %u bytes of bytecode\n",
           rs.DoActions, rs.DoInitActions, rs.ActionBytes);
    if (verbose && rs.SkippedCodeCount)
    {
        printf("  skipped codes  ");
        for (unsigned int i = 0; i < rs.SkippedCodeCount; ++i)
            printf("%u x%u ", rs.SkippedCodes[i], rs.SkippedCounts[i]);
        printf("\n");
    }

    GASActionBuffer::ResetCounters();

    GFxMovieDefImpl* defImpl = new GFxMovieDefImpl(dataDef);
    GFxMovieDef::MemoryParams params;
    GFxMovieView* view = defImpl->CreateInstance(params, false);
    GFxMovieRoot* root = (GFxMovieRoot*)view;
    GFxMovieRoot::bTraceTeardown = true;
    GFxSprite::bTraceClassBinding = verbose;

    // The one thing the harness has to stand in for the engine on. The CLIK components in the shared
    // lib movie switch their button-glyph frames with `this.gotoAndStop(_global.PlatformName)`, and
    // the engine sets that variable before the first advance; with it undefined the clip asks for a
    // frame label that does not exist on every pass, which is a content-level ping-pong rather than a
    // runtime defect. Setting it is what the engine's own movie player does, and the difference it
    // makes is measured in agentCD.md rather than assumed.
    if (platform != 0)
    {
        GFxValue pv;
        pv.SetString(platform);
        root->SetVariable("_global.PlatformName", pv, GFxMovie::SV_Normal);
    }

    printf("  root clip      _level0, %u frames, def '%s'\n",
           root->GetLevel0()->GetFrameCount(), dataDef->GetDefTypeName());

    GFxInputResetCensus();
    for (unsigned int f = 0; f < frames; ++f)
    {
        root->Advance(1.0f / (info.FrameRate > 0.f ? info.FrameRate : 30.f), 0);
        for (unsigned int s = 0; s < stepCount; ++s)
        {
            if (steps[s].Frame != f + 1)
                continue;
            if (steps[s].bKey)
            {
                const int code = atoi(steps[s].Text);
                GFxKeyEvent down;
                memset(&down, 0, sizeof(down));
                down.Type = GFxEvent::KeyDown;
                down.KeyCode = (GFxKey::Code)code;
                const unsigned int rd = root->HandleEvent(down);
                GFxKeyEvent up = down;
                up.Type = GFxEvent::KeyUp;
                const unsigned int ru = root->HandleEvent(up);
                printf("  [frame %u] key %d -> HandleEvent %s / %s\n", f + 1, code,
                       (rd & GFxMovieView::HE_Handled) ? "HE_Handled" : "HE_NotHandled",
                       (ru & GFxMovieView::HE_Handled) ? "HE_Handled" : "HE_NotHandled");
            }
            else
            {
                GFxValue result;
                const bool ok = root->Invoke(steps[s].Text, &result, (const GFxValue*)0, 0);
                printf("  [frame %u] invoke %s -> %s\n", f + 1, steps[s].Text,
                       ok ? "ok" : "NOT FOUND");
            }
        }
        if (verbose)
            printf("  [frame %u] sprites %u placed %u removed %u buffers %u objects %u\n",
                   f + 1, root->GetCensus().SpritesCreated, root->GetCensus().DisplayObjectsPlaced,
                   root->GetCensus().DisplayObjectsRemoved, root->GetCensus().ActionBuffersRun,
                   root->GetASContext()->GetCollector()->GetCreatedCount());
    }

    const GFxMovieRoot::Census& c = root->GetCensus();
    printf("\n  -- after %u advance(s) --\n", frames);
    printf("  frames advanced        %u   current frame %u of %u\n",
           c.FramesAdvanced, root->GetCurrentFrame() + 1, root->GetLevel0()->GetFrameCount());
    printf("  sprites created        %u\n", c.SpritesCreated);
    printf("  display objects        %u placed, %u moved, %u removed; list holds %u\n",
           c.DisplayObjectsPlaced, c.DisplayObjectsMoved, c.DisplayObjectsRemoved,
           root->GetLevel0()->GetDisplayList().GetCount());
    printf("  action buffers run     %u   queued and drained\n", c.ActionBuffersRun);
    printf("  AS2 objects created    %u (live %u)\n",
           root->GetASContext()->GetCollector()->GetCreatedCount(),
           root->GetASContext()->GetCollector()->GetLiveCount());
    printf("  interned strings       %u\n",
           root->GetASContext()->GetStringManager()->GetNodeCount());
    printf("  classes registered     %u via Object.registerClass\n", c.ClassesRegistered);
    {
        const GFxInputCensus& ic = GFxInputGetCensus();
        printf("  input                  %u handled / %u not handled, %u key downs, %u key ups, "
               "%u chars, %u mouse, %u listeners added, %u listener calls\n",
               ic.EventsHandled, ic.EventsNotHandled, ic.KeyDowns, ic.KeyUps, ic.CharsTyped,
               ic.MouseEvents, ic.KeyListenerCalls);
    }
    printf("  script errors          %u\n", c.ScriptErrors);
    printf("  opcodes executed       %u, of which %u had no implementation\n",
           GASActionBuffer::OpsExecuted, GASActionBuffer::OpsUnimplemented);

    // Which opcodes the content actually used, and whether each one was implemented. This is the
    // measurement that matters: coverage against what the asset needs, not against the whole set.
    unsigned int usedCodes = 0, usedImplemented = 0;
    printf("\n  opcodes this asset executed:\n");
    for (unsigned int op = 0; op < 256; ++op)
    {
        if (GASActionBuffer::OpCounts[op] == 0)
            continue;
        ++usedCodes;
        bool impl = IsImplemented((unsigned char)op);
        if (impl) ++usedImplemented;
        printf("    0x%02X %-22s %8u %s\n", op, GFxAS2GetOpcodeName((unsigned char)op),
               GASActionBuffer::OpCounts[op], impl ? "" : "  <-- NOT IMPLEMENTED");
    }
    printf("  %u distinct opcodes used, %u of them implemented\n", usedCodes, usedImplemented);

    // The display list, by depth: the objects frame 1 created.
    printf("\n  _level0 display list:\n");
    GFxDisplayList& dl = root->GetLevel0()->GetDisplayList();
    for (unsigned int i = 0; i < dl.GetCount(); ++i)
    {
        GFxCharacter* ch = dl.GetAt(i);
        const char* name = ch->IsASCharacter() ? ch->ToASCharacterDef()->GetName().ToCStr() : "";
        printf("    depth %6d  %-12s id %-6lu %s\n", ch->GetDepth(),
               ch->GetCharacterTypeName(), ch->GetId().Id, name);
    }

    // What the content installed on _global: the __Packages classes are all here.
    ClassLister lister;
    root->GetASContext()->GetGlobalObject()->VisitMembers(
        root->GetASContext()->GetSC(), &lister, GASObjectInterface::VisitMember_DontEnum, 0);
    printf("\n  _global holds %u members", lister.Count);
    if (verbose)
    {
        printf(":");
        for (unsigned int i = 0; i < lister.Count && i < 512; ++i)
            printf("%s%s", (i % 6) == 0 ? "\n    " : " ", lister.Names[i]);
    }
    printf("\n");

    // Proving the engine-facing path works on the movie that was just run: set a member, read it
    // back and invoke a method through GFxValue::ObjectInterface, which is where 81 % of the
    // engine's calls into libgfx go.
    // The braces are not decoration: a managed GFxValue holds a reference through the movie's own
    // ObjectInterface, so every one of them has to be destroyed before the movie view is released.
    // Retail is identical - GFxValue::ObjectRelease (2012 0x9aed40) dereferences pMovieRoot - and this
    // is the ownership rule the GFxUI natives have to follow too.
    printf("\n  GFxValue::ObjectInterface round trip:\n");
    {
        GFxValue rootVal;
        if (root->GetVariable(&rootVal, "_root"))
        {
            GFxValue num;
            num.SetNumber(1234.5);
            bool setOk = rootVal.SetMember("agentBC_probe", num);
            GFxValue back;
            bool getOk = rootVal.GetMember("agentBC_probe", &back);
            printf("    _root is %s; SetMember %s; GetMember %s -> %.1f\n",
                   rootVal.IsDisplayObject() ? "a display object" : "not a display object",
                   setOk ? "ok" : "failed", getOk ? "ok" : "failed",
                   back.IsNumber() ? back.GetNumber() : -1.0);

            // Invoke through the same interface, on a method the content itself defined.
            GFxValue menu;
            if (rootVal.GetMember("mainMenu_mc", &menu) && menu.IsDisplayObject())
            {
                GFxValue res;
                bool invoked = menu.Invoke("toString", &res);
                printf("    _root.mainMenu_mc resolved; Invoke(toString) %s\n",
                       invoked ? "ok" : "no such method");
            }
        }
        else
        {
            printf("    _root did not resolve\n");
        }
        GFxValue arr;
        root->CreateArray(&arr);
        GFxValue e0;
        e0.SetNumber(7.0);
        arr.PushBack(e0);
        printf("    CreateArray + PushBack -> size %u\n", arr.GetArraySize());
    }

    printf("\n  teardown: releasing the movie view\n");
    view->Release();
    printf("  teardown: view released\n");
    free(data);
    printf("  teardown: done\n");
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    // Unbuffered, because a crash in the machine must not swallow the report that says how far it got.
    setvbuf(stdout, 0, _IONBF, 0);
#ifdef _WIN32
    SetUnhandledExceptionFilter(GFxRunCrashFilter);
    AddVectoredExceptionHandler(1, GFxRunFirstChance);
#endif
    if (argc < 2)
    {
        printf("GFx3Run --run <file.gfx> [--frames N] [--verbose] [--imports <dir>] [--platform PC]"
               " [--invoke <frame>:<path>] [--key <frame>:<code>] | --opcodes | --classes\n");
        return 2;
    }
    unsigned int frames = 1;
    bool verbose = false;
    const char* path = 0;
    bool wantOpcodes = false, wantClasses = false;
    const char* importDir = 0;
    const char* platform = 0;
    GFxRunStep steps[32];
    unsigned int stepCount = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--run") == 0 && i + 1 < argc) path = argv[++i];
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) frames = (unsigned int)atoi(argv[++i]);
        else if (strcmp(argv[i], "--verbose") == 0) verbose = true;
        else if (strcmp(argv[i], "--imports") == 0 && i + 1 < argc) importDir = argv[++i];
        else if (strcmp(argv[i], "--platform") == 0 && i + 1 < argc) platform = argv[++i];
        else if (strcmp(argv[i], "--opcodes") == 0) wantOpcodes = true;
        else if (strcmp(argv[i], "--classes") == 0) wantClasses = true;
        else if (strcmp(argv[i], "--optrace") == 0 && i + 1 < argc)
            GFxAS2OpTraceFrom = (unsigned int)atoi(argv[++i]);
        else if ((strcmp(argv[i], "--invoke") == 0 || strcmp(argv[i], "--key") == 0)
                 && i + 1 < argc && stepCount < 32)
        {
            const bool bKey = strcmp(argv[i], "--key") == 0;
            const char* spec = argv[++i];
            const char* colon = strchr(spec, ':');
            steps[stepCount].Frame = colon ? (unsigned int)atoi(spec) : 1;
            steps[stepCount].Text = colon ? colon + 1 : spec;
            steps[stepCount].bKey = bKey;
            ++stepCount;
        }
        else if (argv[i][0] != '-' && path == 0) path = argv[i];
    }

    int rc = 0;
    if (path)
    {
        // The resolver owns the movies it loads and every one of them outlives the movie that
        // imported their symbols, so it is destroyed after RunMovie returns and not before.
        DirImportResolver resolver(importDir ? importDir : ".");
        // A C++ exception out of the machine (0xe06d7363) says nothing through the crash filter, so
        // it is caught here where what() is still readable. Agent DG.
        try
        {
            rc = RunMovie(path, frames ? frames : 1, verbose, importDir ? &resolver : 0, platform,
                          steps, stepCount);
        }
        catch (const std::exception& e)
        {
            printf("\n== EXCEPTION out of the machine: %s\n", e.what());
            rc = 3;
        }
        catch (...)
        {
            printf("\n== EXCEPTION out of the machine: not a std::exception\n");
            rc = 3;
        }
    }
    if (wantOpcodes)
    {
        printf("\n");
        PrintOpcodeTable();
    }
    if (wantClasses)
    {
        printf("\n");
        PrintClassTable();
    }
    return rc;
}
