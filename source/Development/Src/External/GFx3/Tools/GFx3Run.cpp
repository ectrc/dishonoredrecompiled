// Agent BC's acceptance harness for the AS2 machine. It drives the runtime from a cooked asset and
// nothing else: no engine, no renderer, no game.
//
//   GFx3Run --run <file.gfx> [--frames N] [--verbose]
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
#include "GFxPlayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int RunMovie(const char* path, unsigned int frames, bool verbose)
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
    printf("  exports        %u   imports %u\n", rs.Exports, rs.Imports);
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

    printf("  root clip      _level0, %u frames, def '%s'\n",
           root->GetLevel0()->GetFrameCount(), dataDef->GetDefTypeName());

    for (unsigned int f = 0; f < frames; ++f)
        root->Advance(1.0f / (info.FrameRate > 0.f ? info.FrameRate : 30.f), 0);

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
    if (argc < 2)
    {
        printf("GFx3Run --run <file.gfx> [--frames N] [--verbose] | --opcodes | --classes\n");
        return 2;
    }
    unsigned int frames = 1;
    bool verbose = false;
    const char* path = 0;
    bool wantOpcodes = false, wantClasses = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--run") == 0 && i + 1 < argc) path = argv[++i];
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) frames = (unsigned int)atoi(argv[++i]);
        else if (strcmp(argv[i], "--verbose") == 0) verbose = true;
        else if (strcmp(argv[i], "--opcodes") == 0) wantOpcodes = true;
        else if (strcmp(argv[i], "--classes") == 0) wantClasses = true;
        else if (argv[i][0] != '-' && path == 0) path = argv[i];
    }

    int rc = 0;
    if (path)
        rc = RunMovie(path, frames ? frames : 1, verbose);
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
