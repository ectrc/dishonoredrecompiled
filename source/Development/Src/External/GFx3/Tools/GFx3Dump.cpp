// Agent BB's acceptance harness for the GFx 3.3 foundation. Two jobs, both of which the package's
// accept line asks for and neither of which needs the engine:
//
//   GFx3Dump --parse <file.gfx> [...]   load a cooked movie payload through our own container
//                                       parser and report its header, its exported symbol table,
//                                       its frame count and its tag histogram
//   GFx3Dump --slots                    instantiate the renderer seam and call every one of
//                                       GRenderer's 54 slots, GTexture's 12 + 5 and
//                                       GRenderTarget's 7 + 3 through a base-class pointer, then
//                                       print the seam census. If a slot were missing the build
//                                       would not link; if a slot were in the wrong place the
//                                       census name would not match the call.
//
// The payloads come out of the cooked *_SF.upk packages with build/agentBB/extract_gfx.py, which
// reuses resources/tools/pdb/read_package_classes.py; SwfMovie::RawData is the payload byte for
// byte. resources/docs/agents/agentBB.md has the measurements this tool reproduces.
// DISHONORED(written).
#include "GFx3.h"
#include "gfxuirendererimpl.h"
#include "gfxuifile.h"
#include "gfxuiimageinfo.h"
#include "gfxuiallocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace
{

const char* TagName(unsigned int Code)
{
    switch (Code)
    {
    case GFxTag_End: return "End";
    case GFxTag_ShowFrame: return "ShowFrame";
    case GFxTag_DefineShape: return "DefineShape";
    case GFxTag_DefineBits: return "DefineBits";
    case GFxTag_JPEGTables: return "JPEGTables";
    case GFxTag_SetBackgroundColor: return "SetBackgroundColor";
    case GFxTag_DefineFont: return "DefineFont";
    case GFxTag_DoAction: return "DoAction";
    case GFxTag_DefineBitsLossless: return "DefineBitsLossless";
    case GFxTag_DefineBitsJPEG2: return "DefineBitsJPEG2";
    case GFxTag_DefineShape2: return "DefineShape2";
    case GFxTag_PlaceObject2: return "PlaceObject2";
    case GFxTag_RemoveObject2: return "RemoveObject2";
    case GFxTag_DefineShape3: return "DefineShape3";
    case GFxTag_DefineButton2: return "DefineButton2";
    case GFxTag_DefineBitsJPEG3: return "DefineBitsJPEG3";
    case GFxTag_DefineBitsLossless2: return "DefineBitsLossless2";
    case GFxTag_DefineEditText: return "DefineEditText";
    case GFxTag_DefineSprite: return "DefineSprite";
    case GFxTag_FrameLabel: return "FrameLabel";
    case GFxTag_DefineMorphShape: return "DefineMorphShape";
    case GFxTag_DefineFont2: return "DefineFont2";
    case GFxTag_ExportAssets: return "ExportAssets";
    case GFxTag_ImportAssets: return "ImportAssets";
    case GFxTag_DoInitAction: return "DoInitAction";
    case GFxTag_FileAttributes: return "FileAttributes";
    case GFxTag_ImportAssets2: return "ImportAssets2";
    case GFxTag_CSMTextSettings: return "CSMTextSettings";
    case GFxTag_DefineFont3: return "DefineFont3";
    case GFxTag_SymbolClass: return "SymbolClass";
    case GFxTag_Metadata: return "Metadata";
    case GFxTag_DefineScalingGrid: return "DefineScalingGrid";
    case GFxTag_DoABC: return "DoABC";
    case GFxTag_DefineShape4: return "DefineShape4";
    case GFxTag_DefineBitsJPEG4: return "DefineBitsJPEG4";
    case GFxTag_GFxExporterInfo: return "GFx_ExporterInfo";
    case GFxTag_GFxDefineSubImage: return "GFx_DefineSubImage";
    case GFxTag_GFxDefineExternalImage2: return "GFx_DefineExternalImage2";
    default: return "?";
    }
}

int ParseOne(const char* Path, bool bVerbose)
{
    FILE* f = fopen(Path, "rb");
    if (f == 0)
    {
        printf("FAIL %s: cannot open\n", Path);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc(size > 0 ? (size_t)size : 1);
    size_t got = fread(data, 1, (size_t)size, f);
    fclose(f);

    // Through the seam's own GFile: the runtime never sees a path, it sees a memory file over the
    // package's bytes, which is exactly what the retail FGFxFile layout says (gfxuifile.h).
    FGFxFile File(Path, data, (int)got);
    if (!File.IsValid())
    {
        printf("FAIL %s: FGFxFile invalid (error %d)\n", Path, File.GetErrorCode());
        free(data);
        return 1;
    }
    unsigned char* buffer = (unsigned char*)malloc(got ? got : 1);
    int read = File.Read(buffer, (int)got);

    // ~140 KB; heap-allocated rather than put on the stack (GFxGfxFile.h).
    GFxGfxFileInfo* pinfo = new GFxGfxFileInfo();
    GFxGfxFileInfo& info = *pinfo;
    bool ok = GFxGfxParseFile(buffer, (unsigned int)read, info);
    if (!ok)
    {
        printf("FAIL %s: %s\n", Path, info.Error);
        free(buffer);
        free(data);
        return 1;
    }

    printf("%s\n", Path);
    printf("  signature      %s  version %u  %s\n", info.Signature, (unsigned)info.Version,
           info.IsGfxExport ? "gfxexport GFX" : (info.IsFlashSwf ? "Flash SWF" : "?"));
    printf("  declared len   %u   payload %u   %s\n", info.DeclaredLength, info.PayloadLength,
           info.DeclaredLength == info.PayloadLength ? "(equal: uncompressed)" : "(MISMATCH)");
    printf("  frame rect     twips [%d %d %d %d] = %.0f x %.0f px\n",
           info.FrameRectTwips[0], info.FrameRectTwips[1], info.FrameRectTwips[2],
           info.FrameRectTwips[3], info.FrameWidthPixels, info.FrameHeightPixels);
    printf("  frame rate     %.1f fps   frames %u\n", info.FrameRate, info.FrameCount);
    printf("  first tag at   %u   tags %u   distinct codes %u   non-standard %u\n",
           info.FirstTagOffset, info.TagCount, info.TagCodeCount, info.NonStandardTagCount);
    printf("  embedded bitmap tags %u   glyph tags %u   consumed exactly %s%s\n",
           info.EmbeddedBitmapTagCount, info.GlyphTagCount,
           info.ConsumedExactly ? "yes" : "no", info.Truncated ? "   TRUNCATED" : "");
    if (info.HasExporterInfo)
    {
        printf("  exporter       version %u  flags 0x%08x  fileFormat %u  prefix \"%s\"  swf \"%s\"\n",
               (unsigned)info.ExporterInfo.Version, info.ExporterInfo.ExportFlags,
               (unsigned)info.ExporterInfo.FileFormatType, info.ExporterInfo.Prefix,
               info.ExporterInfo.SWFName);
    }
    else
    {
        printf("  exporter       NONE (not gfxexport output)\n");
    }
    printf("  exports %u   imports %u   external images %u   sub-images %u\n",
           info.ExportCount, info.ImportCount, info.ImageCount, info.SubImageCount);

    if (bVerbose)
    {
        printf("  tag histogram:\n");
        for (unsigned int i = 0; i < info.TagCodeCount; ++i)
        {
            printf("    %5u %-26s %5u%s\n", info.TagCodes[i].Code, TagName(info.TagCodes[i].Code),
                   info.TagCodes[i].Count,
                   info.TagCodes[i].Code > 91 ? "   <- Scaleform, not SWF" : "");
        }
        printf("  exported symbols:\n");
        for (unsigned int i = 0; i < info.ExportCount; ++i)
            printf("    %5u  %s\n", (unsigned)info.Exports[i].CharacterId, info.Exports[i].Name);
        for (unsigned int i = 0; i < info.ImportCount; ++i)
            printf("  import %5u  %-24s from %s\n", (unsigned)info.Imports[i].CharacterId,
                   info.Imports[i].Name, info.Imports[i].Url);
        for (unsigned int i = 0; i < info.ImageCount; ++i)
            printf("  image  idx %3u flags %u fmt %u %ux%u  export \"%s\" file \"%s\"\n",
                   (unsigned)info.Images[i].ImageIndex, (unsigned)info.Images[i].Flags,
                   (unsigned)info.Images[i].FileFormatType, (unsigned)info.Images[i].SrcWidth,
                   (unsigned)info.Images[i].SrcHeight, info.Images[i].ExportName,
                   info.Images[i].FileName);
    }

    delete pinfo;
    free(buffer);
    free(data);
    return 0;
}

// Call every slot of the three interfaces through a base-class pointer. The point is not what the
// bodies do (they log and return) but that the seam is complete: a missing override would be a link
// error and a misplaced one would show up as the wrong census name.
int ExerciseSlots()
{
    FGFxSeamReset();
    FGFxRenderer* Impl = new FGFxRenderer();
    GRenderer* R = Impl;

    GRenderer::RenderCaps Caps;
    GViewport VP(1280, 720, 0, 0, 1280, 720);
    GMatrix2D M2;
    GMatrix3D M3;
    GRenderer::Cxform Cx;
    GRenderer::Stats Stats;
    GRenderer::BitmapDesc Desc;
    GRenderer::FillTexture Fill;
    GRenderer::BlurFilterParams Blur;
    GRenderer::DistanceFieldParams Df;
    GRect<float> Rect(0.f, 0.f, 1280.f, 720.f);
    float ColorMatrix[20] = { 0 };

    R->ScopedEventCallback("probe");                           // 1
    R->SaveCurrentRenderTargetContents();                      // 2
    R->RestoreCurrentRenderTargetContents();                   // 3
    R->GetRenderCaps(&Caps);                                   // 4
    GTexture* T = R->CreateTexture();                          // 5
    R->CreateTextureYUV();                                     // 6
    R->BeginFrame();                                           // 7
    R->EndFrame();                                             // 8
    GRenderTarget* RT = R->CreateRenderTarget();               // 9
    R->SetDisplayRenderTarget(RT, true);                       // 10
    R->PushRenderTarget(Rect, RT);                             // 11
    R->PopRenderTarget();                                      // 12
    R->PushTempRenderTarget(Rect, 256, 256, false);            // 13
    R->ReleaseTempRenderTargets(0);                            // 14
    R->BeginDisplay(GColor(0u), VP, 0.f, 1280.f, 0.f, 720.f);   // 15
    R->EndDisplay();                                           // 16
    R->SetMatrix(M2);                                          // 17
    R->SetUserMatrix(M2);                                      // 18
    R->SetCxform(Cx);                                          // 19
    R->PushBlendMode(GRenderer::Blend_Normal);                  // 20
    R->PopBlendMode();                                         // 21
    R->PushUserData(0);                                        // 22
    R->PopUserData();                                          // 23
    R->SetPerspective3D(M3);                                   // 24
    R->SetView3D(M3);                                          // 25
    R->SetWorld3D(&M3);                                        // 26
    {
        GMatrix3D View, Persp;
        R->MakeViewAndPersp3D(Rect, View, Persp, 60.f, false);   // 27
    }
    R->SetStereoParams(GRenderer::StereoParams());             // 28
    R->SetStereoDisplay(GRenderer::StereoCenter, true);      // 29
    R->SetVertexData(0, 0, GRenderer::Vertex_XY16i, 0);         // 30
    R->SetIndexData(0, 0, GRenderer::Index_16, 0);              // 31
    R->ReleaseCachedData(0, GRenderer::Cached_Vertex);           // 32
    R->DrawIndexedTriList(0, 0, 3, 0, 1);                       // 33
    R->DrawLineStrip(0, 1);                                     // 34
    R->LineStyleDisable();                                      // 35
    R->LineStyleColor(GColor(0u));                              // 36
    R->FillStyleDisable();                                      // 37
    R->FillStyleColor(GColor(0u));                              // 38
    R->FillStyleBitmap(&Fill);                                  // 39
    R->FillStyleGouraud(GRenderer::GFill_1Texture, &Fill, 0, 0); // 40
    R->DrawBitmaps(&Desc, 1, 0, 1, T, M2, 0);                   // 41
    R->DrawDistanceFieldBitmaps(&Desc, 1, 0, 1, T, M2, Df, 0);   // 42
    R->BeginSubmitMask(GRenderer::Mask_Clear);                   // 43
    R->EndSubmitMask();                                         // 44
    R->DisableMask();                                           // 45
    R->CheckFilterSupport(Blur);                                // 46
    R->DrawBlurRect(T, Rect, Rect, Blur, false);                // 47
    R->DrawColorMatrixRect(T, Rect, Rect, ColorMatrix, false);   // 48
    R->GetRenderStats(&Stats, true);                            // 49
    R->GetStats(0, true);                                       // 50
    R->ReleaseResources();                                      // 51
    R->AddEventHandler(0);                                      // 52
    R->RemoveEventHandler(0);                                   // 53

    // GTexture: 12 base slots plus FGFxTexture's 5.
    GTexture::UpdateRect UR;
    GTexture::MapRect MR;
    T->InitTexture(0, 0);
    T->InitDynamicTexture(4, 4, GImageBase::Image_ARGB_8888, 1, 0);
    T->Update(0, 1, &UR, 0);
    T->Map(0, 1, &MR, 0);
    T->Unmap(0, 1, &MR, 0);
    T->GetRenderer();
    T->IsDataValid();
    T->GetUserData();
    T->SetUserData(0);
    T->AddChangeHandler(0);
    T->RemoveChangeHandler(0);
    FGFxTexture* FT = (FGFxTexture*)T;
    FT->InitTexture((UTexture*)0, false);
    FT->InitTextureFromFile("probe");
    FT->IsYUVTexture();
    FT->InternalTermGCState();

    // GRenderTarget: 7 base slots plus FGFxRenderTarget's 3.
    RT->InitRenderTarget(T, 0, 0);
    RT->GetRenderer();
    RT->GetUserData();
    RT->SetUserData(0);
    RT->AddChangeHandler(0);
    RT->RemoveChangeHandler(0);
    FGFxRenderTarget* FRT = (FGFxRenderTarget*)RT;
    FRT->InitRenderTarget_RenderThread(T, 0, 64, 64);
    float w = 1000.f, h = 1000.f;
    FRT->AdjustBounds(&w, &h);

    // The file opener (4 slots) and the image seam (1 + 1 + its own).
    FGFxFileOpener Opener;
    GFxFileOpener* O = &Opener;
    O->OpenFile("probe.gfx", GFileConstants::Open_Read, 0);
    O->GetFileModifyTime("probe.gfx");
    O->OpenFileEx("probe.gfx", 0, GFileConstants::Open_Read, 0);

    FGFxImageLoader Loader;
    ((GFxImageLoader*)&Loader)->LoadImageW("probe.tga");
    FGFxImageCreator Creator;
    GFxImageCreateInfo CI;
    memset(&CI, 0, sizeof(CI));
    ((GFxImageCreator*)&Creator)->CreateImage(CI);

    FGFxAllocator Alloc;
    GSysAllocPaged* A = &Alloc;
    GSysAllocPaged::Info AInfo;
    A->GetInfo(&AInfo);
    void* p = A->Alloc(64, 16);
    A->Free(p, 64, 16);
    A->GetFootprint();
    A->GetUsedSpace();

    char Census[4096];
    FGFxSeamCensus(Census, sizeof(Census));
    printf("%s\n", Census);
    printf("DISHONORED(bringup): GFx renderer seam: %u distinct slots recorded, %u calls\n",
           FGFxSeamSlotsTouched(), FGFxSeamTotalCalls());
    printf("sizeof: FGFxRenderer %u  FGFxTexture %u  FGFxRenderTarget %u  FGFxFile %u"
           "  FGFxImageInfo %u  FGFxAllocator %u\n",
           (unsigned)sizeof(FGFxRenderer), (unsigned)sizeof(FGFxTexture),
           (unsigned)sizeof(FGFxRenderTarget), (unsigned)sizeof(FGFxFile),
           (unsigned)sizeof(FGFxImageInfo), (unsigned)sizeof(FGFxAllocator));

    delete RT;
    delete T;
    delete Impl;
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    bool bVerbose = false;
    bool bSlots = false;
    int first = 1;
    for (; first < argc; ++first)
    {
        if (strcmp(argv[first], "--verbose") == 0) bVerbose = true;
        else if (strcmp(argv[first], "--slots") == 0) bSlots = true;
        else if (strcmp(argv[first], "--parse") == 0) continue;
        else break;
    }
    int rc = 0;
    if (bSlots) rc |= ExerciseSlots();
    for (int i = first; i < argc; ++i) rc |= ParseOne(argv[i], bVerbose);
    if (!bSlots && first >= argc)
    {
        printf("usage: GFx3Dump [--slots] [--verbose] [--parse] <file.gfx> ...\n");
        return 2;
    }
    return rc;
}
