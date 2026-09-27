#pragma once
// GFxUI/inc/gfxuirenderer.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x5b97b0  public: __thiscall FGFxRenderer::RTState::RTState(class FGFxRenderTarget *, class GMatrix2D const &, class GMatrix3D const &, class GMatrix3D const &, class GMatrix3D const &, unsigned int, struct FGFxViewportUserParams const &, int, class TDynamicRHIResource<4> *)
//   0x5bf620  public: virtual bool __thiscall FGFxTexture::IsDataValid(void)const
//   0x5c9070  public: virtual bool __thiscall FGFxTexture::InitTextureFromFile(char const *)
//   0x5ce5b0  public: __thiscall FGFxRenderTarget::FGFxRenderTarget(class GRenderer *)
//   0x5ce660  public: virtual bool __thiscall FGFxRenderTarget::InitRenderTarget(struct FGFxRenderTargetResource::NativeRenderTarget const &)
//   0x5ce6f0  public: virtual bool __thiscall FGFxRenderTarget::InitRenderTarget_RenderThread(struct FGFxRenderTargetResource::NativeRenderTarget const &)

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the GFx 3.3 renderer seam and its drawing half. Agent BB declared and verified
// all 54 GRenderer / 12+5 GTexture / 7+3 GRenderTarget slots; package CC (PHASE9.md) filled the
// bodies that submit geometry, so this is the renderer Arkane shipped: Scaleform is drawn through
// UE3's own RHI with the GFx shader families of gfxuishaders.cpp, never through a Scaleform
// renderer back end.
//
// Every layout below is the 2012 Shipping PDB's (resources/tools/pdb/dia_types.py --udt-re "^FGFx",
// build/agentBB/fgfx_types.json), every body is the retail decompile named at its declaration
// (build/agentCC/dec, headless), and the 2013 rvas come from
// resources/docs/symbols/match_2012_2013.csv.
//
// Key 2013 rvas of the retail bodies these declarations correspond to:
//   FGFxRenderer::FGFxRenderer            0x5a1f30   (2012 0x5e1bf0)
//   FGFxRenderer::GetRenderCaps           0x572e00   (2012 0x5b78e0)
//   FGFxRenderer::CreateTexture           0x5800d0   (2012 0x5c47d0)
//   FGFxRenderer::CreateRenderTarget      0x58f120   (2012 0x5cf8d0)
//   FGFxRenderer::BeginDisplay            0x59dd90   (2012 0x5df080)
//   FGFxRenderer::EndDisplay              0x593c40   (2012 0x5d3e40)
//   FGFxRenderer::SetMatrix               0x58ac30   (2012 0x5cca20)
//   FGFxRenderer::SetVertexData           0x599f90   (2012 0x5d9ca0)
//   FGFxRenderer::SetIndexData            0x599fc0   (2012 0x5d9cd0)
//   FGFxRenderer::DrawIndexedTriList      0x5a1620   (2012 0x5e2160)
//   FGFxRenderer::DrawBitmaps             0x5a18c0   (2012 0x5e2400)
//   FGFxRenderer::ReleaseResources        0x59db10   (2012 0x5dec80)
//   FGFxTexture::InitTexture(GImageBase*) 0x598780   (2012 0x5d8300)
//   FGFxTexture::InitTexture(UTexture*)   0x580810   (2012 0x5c4f10)
//   FGFxTexture::Bind                     0x5775b0   (2012 0x5bbed0)
//   FGFxRenderTarget::FGFxRenderTarget    0x58d240   (2012 0x5ce5b0)
//   FGFxRenderTarget::InitRenderTarget(GTexture*,GTexture*,GTexture*) 0x577890 (2012 0x5bc1b0)
//   FGFxRenderTarget::InitRenderTarget(NativeRenderTarget&)           0x58d2f0 (2012 0x5ce660)
//
// MSVC lays a run of consecutive virtual overloads out in REVERSE declaration order (agentAL.md 4,
// re-proved for this toolchain in build/agentBB/vt/vtprobe.cod), so FGFxRenderTarget's
// InitRenderTarget_RenderThread pair below is declared slot 9 first, slot 8 second.
//
// This header needs the engine, which agent BB's seam deliberately did not: the drawing half holds
// RHI references, an FViewport, an FRenderTarget and UTexture2D pointers, exactly as the 892-byte
// retail FGFxRenderer does. The cost is agent BB's no-engine compile check (build/agentBB_seam.cmd),
// which no longer applies to gfxuirenderer.{h,cpp}, gfxuiimageinfo.{h,cpp} or gfxuifile.cpp.
#include "GFxUI.h"

// DISHONORED(bringup): two enumerations end up declared twice in one translation unit once both the
// generated module header and the reconstructed GFx 3.3 headers are in - GFxRenderTextureMode and
// GFxTimingMode, identical enumerators from two generators (GFxUIEngineShims.h from
// resources/tools/symbols/gen_classes_header.py --sdk, GFx3Enums.h from the 2012 PDB). Nothing in
// External/GFx3 uses either name, so GFx3's copies are renamed for the duration of the include
// rather than either generated header being hand-edited. Retire it when one of the two generators
// stops emitting them.
// And one class name, for the same reason and with one more consequence: GFxUI/Inc/gfxui_gfx3.h
// declares its own GRefCountImplCore with AddRef/Release defined out of line in
// gfxuigfx3absent.cpp, while External/GFx3/GTypes.h declares the PDB's with both inline. Any
// unit that uses the GFx3 refcount chain therefore collides with that one at link time (two
// LNK2005, measured). The two are the same 8 bytes and the same behaviour - two reconstructions
// of one retail class - so GFx3's copy is renamed here, which keeps both worlds linkable. The
// real fix is agent BE's own: DISHONORED_GFXUI_GFX3_RUNTIME=1 drops gfxuigfx3absent.cpp and the
// duplicate with it, and that switch belongs to the commit that wires the runtime up.
#define GRefCountImplCore GFx3RefCountImplCore
#define GFxRenderTextureMode GFx3RenderTextureMode
#define GFxTimingMode GFx3TimingMode
#define RTM_Opaque GFx3RTM_Opaque
#define RTM_Alpha GFx3RTM_Alpha
#define RTM_AlphaComposite GFx3RTM_AlphaComposite
#define RTM_MAX GFx3RTM_MAX
#define TM_Game GFx3TM_Game
#define TM_Real GFx3TM_Real
#define TM_MAX GFx3TM_MAX
#include "GFx3.h"
#undef GRefCountImplCore
#undef GFxRenderTextureMode
#undef GFxTimingMode
#undef RTM_Opaque
#undef RTM_Alpha
#undef RTM_AlphaComposite
#undef RTM_MAX
#undef TM_Game
#undef TM_Real
#undef TM_MAX

class FGFxRenderResources;
class FGFxRenderTargetResource;
class FGFxPixelShaderInterface;
class FGFxVertexShaderInterface;
class FGFxRenderer;
class FGFxRenderTarget;
class FGFxTexture;
class FSceneDepthTargetProxy;
struct FGFxBoundShaderState;

// FGFxRendererImpl is a namespace in retail, not a class: every one of its PDB entries demangles as
// a free `void __cdecl FGFxRendererImpl::f(...)`, never as `public: static`. That is what lets the
// element stores be named here by forward declaration and defined in gfxuirendererimpl.h, after
// FGFxRenderer - which they need, because their InitElements links itself into the renderer's list.
namespace FGFxRendererImpl
{
    struct FGFxRenderElementStoreBase;
    struct FGFxVertexStore;
    struct FGFxIndexStore;
    struct FGFxBitmapDescStore;
}

// ---------------------------------------------------------------------------------------------
// The seam census. Every slot records itself here, so a run can report exactly which of the
// 54 + 12 + 7 + 4 + 19 slots the runtime reached and how much geometry each drawing slot
// submitted, in the "DISHONORED(bringup): <thing> census: <N> ..." shape
// resources/tools/run_regression.py expects.
// ---------------------------------------------------------------------------------------------
enum { GFXUI_SEAM_MAX_SLOTS = 192 };

void         FGFxSeamNote(const char* Slot);
unsigned int FGFxSeamCalls(const char* Slot);
unsigned int FGFxSeamSlotsTouched();
unsigned int FGFxSeamTotalCalls();
unsigned int FGFxSeamCensus(char* Out, unsigned int Capacity);
void         FGFxSeamReset();

#define GFXUI_SEAM_TRACE(name) FGFxSeamNote(name)

// The draw census: what actually reached the RHI, per pass kind.
struct FGFxDrawCensus
{
    unsigned int Draws;             // RHI draw calls issued
    unsigned int TriListDraws;      // DrawIndexedTriList_RenderThread
    unsigned int LineDraws;         // DrawLineStrip_RenderThread
    unsigned int BitmapDraws;       // DrawBitmaps_RenderThread (glyph and bitmap batches)
    unsigned int BackgroundDraws;   // BeginDisplay's background quad
    unsigned int FilterDraws;       // DrawBlurRect / DrawColorMatrixRect
    unsigned int MaskPasses;        // BeginSubmitMask
    unsigned int Triangles;
    unsigned int Lines;
    unsigned int Glyphs;
    unsigned int BoundShaderStates; // distinct bound shader states created (cache misses)
};
extern FGFxDrawCensus GGFxDrawCensus;

// ---------------------------------------------------------------------------------------------
// FGFxViewportUserParams - PDB sizeof 64, two 32-byte axes. SetUIViewport clips the pixel range
// against the buffer and derives the viewport matrix from the clipped range (2012 0x5c4820).
// ---------------------------------------------------------------------------------------------
struct FGFxViewportAxisInfo
{
    FLOAT PixelStart;      // @0
    FLOAT PixelEnd;        // @4
    FLOAT PixelClipStart;  // @8
    FLOAT PixelClipEnd;    // @12
    FLOAT PixelClipLength; // @16
    INT   ViewStart;       // @20
    INT   ViewLength;      // @24
    INT   ViewLengthMax;   // @28

    void Clip();
};

struct FGFxViewportUserParams
{
    FGFxViewportAxisInfo xAxis; // @0
    FGFxViewportAxisInfo yAxis; // @32
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderResources - the shared depth/stencil surface a temp render target borrows.
// PDB sizeof 40: GRefCountBase<FGFxRenderResources,2> @0, FRenderResource @8, SizeX @28,
// SizeY @32, DepthSurface @36.
// ---------------------------------------------------------------------------------------------
class FGFxRenderResources : public GRefCountBase<FGFxRenderResources, 2>, public FRenderResource
{
public:
    INT            SizeX;        // @28
    INT            SizeY;        // @32
    FSurfaceRHIRef DepthSurface; // @36

    FGFxRenderResources() : SizeX(0), SizeY(0) {}
    virtual ~FGFxRenderResources();                                           // 2012 0x5bab20

    virtual void InitDynamicRHI();                                            // 2012 0x5c4430
    virtual void ReleaseDynamicRHI();                                         // 2012 0x5c44f0
    virtual FString GetFriendlyName() const { return TEXT("FGFxRenderResources"); }
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderTargetResource - PDB sizeof 56, base FRenderResource (20). Either it wraps the
// engine's own FRenderTarget (Owner set: the viewport the UI draws over) or it owns a colour
// texture plus its targetable surface (Owner null: a GFx temp render target).
// ---------------------------------------------------------------------------------------------
class FGFxRenderTargetResource : public FRenderResource
{
public:
    // What FGFxRenderTarget::InitRenderTarget(NativeRenderTarget&) is handed. PDB sizeof 8.
    struct NativeRenderTarget
    {
        FRenderTarget*          Owner;      // @0
        FSceneDepthTargetProxy* OwnerDepth; // @4

        NativeRenderTarget() : Owner(NULL), OwnerDepth(NULL) {}
        NativeRenderTarget(FRenderTarget* InOwner, FSceneDepthTargetProxy* InOwnerDepth = NULL)
            : Owner(InOwner), OwnerDepth(InOwnerDepth) {}
    };

    FRenderTarget*          Owner;        // @20
    FSceneDepthTargetProxy* OwnerDepth;   // @24
    INT                     SizeX;        // @28
    INT                     SizeY;        // @32
    FLOAT                   InverseGamma; // @36
    FTexture2DRHIRef        ColorTexture; // @40
    FSurfaceRHIRef          ColorBuffer;  // @44
    FSurfaceRHIRef          DepthBuffer;  // @48
    UINT                    StatSize;     // @52

    FGFxRenderTargetResource()
        : Owner(NULL), OwnerDepth(NULL), SizeX(0), SizeY(0), InverseGamma(1.f), StatSize(0) {}
    virtual ~FGFxRenderTargetResource();                                      // 2012 0x5bc0e0

    virtual void InitDynamicRHI();                                            // 2012 0x5c4f50
    virtual void ReleaseDynamicRHI();                                         // 2012 0x5c5110
    virtual FString GetFriendlyName() const { return TEXT("FGFxRenderTargetResource"); }
};

// ---------------------------------------------------------------------------------------------
// FGFxTexture - GTexture (12 slots) plus 5 of its own. PDB sizeof 24, members at @8..@20.
// ---------------------------------------------------------------------------------------------
class FGFxTexture : public GTexture
{
public:
    GRenderer*        Renderer;      // @8
    UTexture*         Texture;       // @12
    UTexture2D*       Texture2D;     // @16
    FGFxRenderTarget* RenderTarget;  // @20

    FGFxTexture(GRenderer* InRenderer);

    // GTexture
    virtual ~FGFxTexture();                                                              // vt[0]  2012 0x5d4220
    virtual bool InitTexture(GImageBase* Image, unsigned int Usage);                      // vt[1]  2012 0x5d8300
    virtual bool InitDynamicTexture(int Width, int Height, GImageBase::ImageFormat Format,
                                    int Mipmaps, unsigned int Usage);                    // vt[2]  2012 0x5d8590
    virtual void Update(int Level, int NumRects, const GTexture::UpdateRect* Rects,
                        const GImageBase* Image);                                        // vt[3]  2012 0x5d42b0
    virtual int  Map(int Level, int NumRects, GTexture::MapRect* Maps, int Flags);        // vt[4]  2012 0x5bbff0
    virtual bool Unmap(int Level, int NumRects, GTexture::MapRect* Maps, int Flags);      // vt[5]  2012 0x5bc0a0
    virtual GRenderer* GetRenderer() const;                                              // vt[6]
    virtual bool IsDataValid() const;                                                    // vt[7]  2012 0x5bf620
    virtual void* GetUserData() const;                                                   // vt[8]
    virtual void SetUserData(void* Data);                                                // vt[9]
    virtual void AddChangeHandler(GTexture::ChangeHandler* Handler);                      // vt[10]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* Handler);                   // vt[11]

    // FGFxTexture's own
    virtual bool InitTexture(UTexture* InTexture, bool bAsRenderTarget);                 // vt[12] 2012 0x5c4f10
    virtual bool InitTextureFromFile(const char* FileName);                              // vt[13] 2012 0x5c9070
    virtual int  IsYUVTexture() const;                                                   // vt[14]
    virtual void Bind(int Stage, FGFxPixelShaderInterface& Shader,
                      GRenderer::BitmapWrapMode WrapMode,
                      GRenderer::BitmapSampleMode SampleMode, bool bUseMips) const;      // vt[15] 2012 0x5bbed0
    virtual void InternalTermGCState();                                                  // vt[16] 2012 0x5cff60

protected:
    void LoadMipLevel(INT Level, UINT DestPitch, const BYTE* Src, UINT Width, UINT Height,
                      INT BytesPerPixel, UINT SrcPitch);                                 // 2012 0x5bbcf0
public:
    void Update_RenderThread(INT Level, INT NumRects, const FUpdateTextureRegion2D* Regions,
                             const GImageBase* Image);                                   // 2012 0x5bbd70
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderTarget - GRenderTarget (7 slots) plus 3 of its own. PDB sizeof 36.
// ---------------------------------------------------------------------------------------------
class FGFxRenderTarget : public GRenderTarget
{
public:
    GRenderer*                Renderer;      // @8
    GPtr<FGFxTexture>         Texture;       // @12
    GPtr<FGFxRenderResources> StencilBuffer; // @16
    FGFxRenderTargetResource* Resource;      // @20 (a reference in retail; the ctor makes it)
    unsigned int              TargetWidth;   // @24
    unsigned int              TargetHeight;  // @28
    bool                      IsTemp;        // @32

    FGFxRenderTarget(GRenderer* InRenderer);                                  // 2013 0x58d240

    // GRenderTarget
    virtual ~FGFxRenderTarget();                                                          // vt[0]  2012 0x5cffd0
    virtual bool InitRenderTarget(GTexture* Color, GTexture* DepthStencil,
                                  GTexture* Resolve);                                     // vt[1]  2012 0x5bc1b0
    virtual GRenderer* GetRenderer() const;                                               // vt[2]
    virtual void* GetUserData() const;                                                    // vt[3]
    virtual void SetUserData(void* Data);                                                 // vt[4]
    virtual void AddChangeHandler(GTexture::ChangeHandler* Handler);                       // vt[5]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* Handler);                    // vt[6]

    // FGFxRenderTarget's own. Slots 8 and 9 are a consecutive same-name run, so they are declared
    // highest first: slot 9 before slot 8.
    virtual bool InitRenderTarget(const FGFxRenderTargetResource::NativeRenderTarget& Native);  // vt[7] 2012 0x5ce660
    virtual bool InitRenderTarget_RenderThread(GTexture* Color, FGFxRenderResources* Stencil,
                                               unsigned int Width, unsigned int Height);       // vt[9] 2012 0x5d0140
    virtual bool InitRenderTarget_RenderThread(const FGFxRenderTargetResource::NativeRenderTarget& Native); // vt[8] 2012 0x5ce6f0

    bool AdjustBounds(float* Width, float* Height);   // 2012 0x5b7cd0, non-virtual
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderer - all 54 GRenderer slots plus the render-thread half. PDB sizeof 892; every member
// below is at its PDB offset and the list is complete.
// ---------------------------------------------------------------------------------------------
class FGFxRenderer : public GRenderer
{
public:
    // The render-thread copy of a GRenderer::FillTexture. PDB FGFxRenderer::FFillTextureInfo,
    // sizeof 40.
    class FFillTextureInfo
    {
    public:
        FTexture*                   Texture;       // @0
        GMatrix2D                   TextureMatrix; // @4
        GRenderer::BitmapWrapMode   WrapMode;      // @28
        GRenderer::BitmapSampleMode SampleMode;    // @32
        unsigned int                bUseMips;      // @36

        FFillTextureInfo()
            : Texture(NULL), WrapMode(GRenderer::Wrap_Clamp),
              SampleMode(GRenderer::Sample_Linear), bUseMips(0) {}
    };

    enum EGFxRenderStyleMode
    {
        GFx_SM_Disabled = 0,
        GFx_SM_Color    = 1,
        GFx_SM_Bitmap   = 2,
        GFx_SM_Gouraud  = 3
    };

    // FGFxRenderer::FGFxRenderStyle, PDB sizeof 44: the line style, and the base of the fill style.
    // GetEnumeratedBoundShaderState_RenderThread and Apply_RenderThread take void* in the PDB,
    // which is what keeps this header free of the shader enumerations; they are an
    // FGFxEnumeratedBoundShaderState / FGFxBoundShaderState and an FGFxRenderStyleContext.
    class FGFxRenderStyle
    {
    public:
        EGFxRenderStyleMode StyleMode;     // @4
        GColor              Color;         // @8
        GRenderer::Cxform   CxColorMatrix; // @12

        FGFxRenderStyle();
        virtual ~FGFxRenderStyle() {}

        virtual void GetEnumeratedBoundShaderState_RenderThread(void* const OutState,
                                                                void const* const Context); // 2012 0x5b77b0
        virtual void Apply_RenderThread(FGFxRenderer* Renderer, void const* const BoundState,
                                        void* const Context);                               // 2012 0x5c46d0
        virtual void EndDisplay_RenderThread();                                             // 2012 0x5bb150

        void Disable();                                                                     // 2012 0x5bb240
        void SetStyleColor_RenderThread(GColor InColor);
    };

    // FGFxRenderer::FGFxFillStyle, PDB sizeof 128: the render style plus the Gouraud mode and two
    // texture infos.
    class FGFxFillStyle : public FGFxRenderStyle
    {
    public:
        GRenderer::GouraudFillType GouraudFillMode; // @44
        FFillTextureInfo           TexInfo;         // @48
        FFillTextureInfo           TexInfo2;        // @88

        FGFxFillStyle() : GouraudFillMode(GRenderer::GFill_Color) {}

        virtual void GetEnumeratedBoundShaderState_RenderThread(void* const OutState,
                                                                void const* const Context); // 2012 0x5b77e0
        virtual void Apply_RenderThread(FGFxRenderer* Renderer, void const* const BoundState,
                                        void* const Context);                               // 2012 0x5cc820
        virtual void EndDisplay_RenderThread();                                             // 2012 0x5bb160

        void SetStyleBitmap_RenderThread(const FFillTextureInfo& InTexInfo,
                                         const GRenderer::Cxform& Cx);                      // 2012 0x5bb350
        void SetStyleGouraud_RenderThread(GRenderer::GouraudFillType Type,
                                          const FFillTextureInfo* T0, const FFillTextureInfo* T1,
                                          const FFillTextureInfo* T2,
                                          const GRenderer::Cxform& Cx);                     // 2012 0x5bb3b0

    private:
        static void StaticApplyTexture_RenderThread(FGFxRenderer* Renderer,
                                                    void const* const BoundState,
                                                    void const* const Context,
                                                    const FFillTextureInfo& TexInfo, INT i); // 2012 0x5c4720
        static void StaticApplyTextureMatrix_RenderThread(void const* const BoundState,
                                                          void const* const Context,
                                                          const FFillTextureInfo& TexInfo,
                                                          INT i);                            // 2012 0x5bb480
    };

    // FGFxRenderer::FGFxLineStyle, PDB sizeof 44: a render style with no additions.
    class FGFxLineStyle : public FGFxRenderStyle
    {
    };

    // FGFxRenderer::RTState, PDB sizeof 296: what PushRenderTarget saves and PopRenderTarget
    // restores. 2012 ctor 0x5b97b0.
    class RTState
    {
    public:
        FGFxRenderTarget*      pRT;           // @0
        GMatrix2D              ViewMatrix;    // @4
        GMatrix3D              ViewMatrix3D;  // @28
        GMatrix3D              PerspMatrix3D; // @92
        GMatrix3D              WorldMatrix3D; // @156
        FGFxViewportUserParams ViewRect;      // @220
        INT                    RenderMode;    // @284
        unsigned int           Is3DEnabled;   // @288
        FStencilStateRHIRef    StencilState;  // @292

        RTState() : pRT(NULL), RenderMode(0), Is3DEnabled(0)
        {
            appMemzero(&ViewRect, sizeof(ViewRect));
        }
        RTState(FGFxRenderTarget* InRT, const GMatrix2D& InView, const GMatrix3D& InView3D,
                const GMatrix3D& InPersp3D, const GMatrix3D& InWorld3D, unsigned int InIs3DEnabled,
                const FGFxViewportUserParams& InViewRect, INT InRenderMode,
                FStencilStateRHIParamRef InStencilState);
    };

    // FGFxRenderer::FMiscRenderStateInitParams, PDB sizeof 28: what BeginDisplay hands the render
    // thread so InitUIBlendStackAndMiscRenderState_RenderingThread can reset the state block.
    struct FMiscRenderStateInitParams
    {
        FGFxRenderer*                 Renderer;        // @0
        GRenderer::BlendType*         BlendMode;       // @4
        TArray<GRenderer::BlendType>* BlendModeStack;  // @8
        unsigned long*                StencilCounter;  // @12
        FLOAT*                        InverseGamma;    // @16
        unsigned int*                 bAlphaComposite; // @20
        unsigned long                 ViewFlags;       // @24
    };

    GRenderer::Stats                              RenderStats;           // @44
    FViewport*                                    Viewport;              // @64
    FRenderTarget*                                RenderTarget;          // @68
    unsigned int                                  RenderMode;            // @72
    FGFxRendererImpl::FGFxVertexStore*            VertexStore;           // @76
    FGFxRendererImpl::FGFxIndexStore*             IndexStore;            // @80
    GMatrix2D                                     UserMatrix;            // @84
    GMatrix2D                                     CurrentMatrix;         // @108
    GMatrix2D                                     ViewportMatrix;        // @132
    GRenderer::Cxform                             CurrentCxform;         // @156
    FGFxViewportUserParams                        ViewRect;              // @188
    FLOAT                                         InverseGamma;          // @252
    FStencilStateRHIRef                           CurStencilState;       // @256
    GMatrix3D                                     ViewMatrix;            // @260
    GMatrix3D                                     ProjMatrix;            // @324
    GMatrix3D                                     WorldMatrix;           // @388
    GMatrix3D                                     UVPMatrix;             // @452
    unsigned int                                  UVPMatricesChanged;    // @516
    unsigned int                                  Is3DEnabled;           // @520
    FGFxRenderTarget*                             CurRenderTarget;       // @524
    unsigned int                                  CurRenderTargetSet;    // @528
    GArray<RTState, 2, GArrayConstPolicy<0, 4, 1> > RenderTargetStack;   // @532
    GArray<GPtr<FGFxRenderTarget>, 2>             TempRenderTargets;     // @544
    GArray<GPtr<FGFxRenderResources>, 2>          TempStencilBuffers;    // @556
    GRendererNode                                 ElementStoreList;      // @568
    GLock                                         ElementAccessLock;     // @576
    GRenderer::BlendType                          BlendMode;             // @600
    TArray<GRenderer::BlendType>                  BlendModeStack;        // @604
    unsigned int                                  bAlphaComposite;       // @616
    unsigned int                                  MaxTempRTSize;         // @620
    TMap<DWORD, FBoundShaderStateRHIRef>          BoundShaderStateCache; // @624
    FSamplerStateRHIRef                           SamplerStates[8];      // @684
    FGFxFillStyle                                 FillStyle;             // @716
    FGFxLineStyle                                 LineStyle;             // @844
    unsigned long                                 StencilCounter;        // @888

    FGFxRenderer();                                                            // 2013 0x5a1f30
    virtual ~FGFxRenderer();                                                              // vt[0]  2012 0x5decf0

    virtual void ScopedEventCallback(const char* Name);                                   // vt[1]  GRenderer's own in retail
    virtual void SaveCurrentRenderTargetContents();                                       // vt[2]  GRenderer's own in retail
    virtual void RestoreCurrentRenderTargetContents();                                    // vt[3]  GRenderer's own in retail
    virtual bool GetRenderCaps(GRenderer::RenderCaps* Caps);                              // vt[4]  2013 0x572e00
    virtual FGFxTexture* CreateTexture();                                                 // vt[5]  2013 0x5800d0
    virtual FGFxTexture* CreateTextureYUV();                                              // vt[6]
    virtual void BeginFrame();                                                            // vt[7]  GRenderer's own in retail
    virtual void EndFrame();                                                              // vt[8]  GRenderer's own in retail
    virtual FGFxRenderTarget* CreateRenderTarget();                                       // vt[9]  2013 0x58f120
    virtual void SetDisplayRenderTarget(GRenderTarget* Target, bool bSetState);            // vt[10] 2012 0x5d3b10
    virtual void PushRenderTarget(const GRect<float>& FrameRect, GRenderTarget* Target);   // vt[11] 2012 0x5d9270
    virtual void PopRenderTarget();                                                       // vt[12] 2012 0x5d94a0
    virtual FGFxTexture* PushTempRenderTarget(const GRect<float>& FrameRect, unsigned int Width,
                                              unsigned int Height, bool bWantStencil);    // vt[13] 2012 0x5dee70
    virtual void ReleaseTempRenderTargets(unsigned int KeepArea);                          // vt[14] 2012 0x5e2030
    virtual void BeginDisplay(GColor BackgroundColor, const GViewport& InViewport,
                              float x0, float x1, float y0, float y1);                    // vt[15] 2013 0x59dd90
    virtual void EndDisplay();                                                             // vt[16] 2013 0x593c40
    virtual void SetMatrix(const GMatrix2D& Matrix);                                       // vt[17] 2013 0x58ac30
    virtual void SetUserMatrix(const GMatrix2D& Matrix);                                   // vt[18] 2012 0x5cca40
    virtual void SetCxform(const GRenderer::Cxform& Cx);                                   // vt[19] 2012 0x5c4b60
    virtual void PushBlendMode(GRenderer::BlendType Mode);                                 // vt[20] 2012 0x5cfa50
    virtual void PopBlendMode();                                                           // vt[21] 2012 0x5d3f60
    virtual bool PushUserData(GRenderer::UserData* Data);                                  // vt[22] GRenderer's own in retail
    virtual void PopUserData();                                                            // vt[23] GRenderer's own in retail
    virtual void SetPerspective3D(const GMatrix3D& Persp);                                 // vt[24] 2012 0x5bb690
    virtual void SetView3D(const GMatrix3D& View);                                         // vt[25] 2012 0x5bb7d0
    virtual void SetWorld3D(const GMatrix3D* World);                                       // vt[26] 2012 0x5bb910
    virtual void MakeViewAndPersp3D(const GRect<float>& FrameRect, GMatrix3D& View,
                                    GMatrix3D& Persp, float FovY, bool bInvertY);          // vt[27] GRenderer's own in retail
    virtual void SetStereoParams(GRenderer::StereoParams Params);                          // vt[28] GRenderer's own in retail
    virtual void SetStereoDisplay(GRenderer::StereoDisplay Display, bool bSet);            // vt[29] GRenderer's own in retail
    virtual void SetVertexData(const void* Vertices, int NumVertices,
                               GRenderer::VertexFormat Format,
                               GRenderer::CacheProvider* Cache);                           // vt[30] 2013 0x599f90
    virtual void SetIndexData(const void* Indices, int NumIndices,
                              GRenderer::IndexFormat Format,
                              GRenderer::CacheProvider* Cache);                            // vt[31] 2013 0x599fc0
    virtual void ReleaseCachedData(GRenderer::CachedData* Data,
                                   GRenderer::CachedDataType Type);                        // vt[32] 2012 0x5c4b80
    virtual void DrawIndexedTriList(int BaseVertexIndex, int MinVertexIndex, int NumVertices,
                                    int StartIndex, int TriangleCount);                    // vt[33] 2013 0x5a1620
    virtual void DrawLineStrip(int BaseVertexIndex, int LineCount);                         // vt[34] 2012 0x5e22c0
    virtual void LineStyleDisable();                                                        // vt[35] 2012 0x5bbce0
    virtual void LineStyleColor(GColor Color);                                              // vt[36] 2012 0x5ccf30
    virtual void FillStyleDisable();                                                        // vt[37] 2012 0x5bbcd0
    virtual void FillStyleColor(GColor Color);                                              // vt[38] 2012 0x5ccc60
    virtual void FillStyleBitmap(const GRenderer::FillTexture* Fill);                       // vt[39] 2012 0x5ccac0
    virtual void FillStyleGouraud(GRenderer::GouraudFillType Type,
                                  const GRenderer::FillTexture* T0,
                                  const GRenderer::FillTexture* T1,
                                  const GRenderer::FillTexture* T2);                       // vt[40] 2012 0x5ccdb0
    virtual void DrawBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize, int StartIndex,
                             int Count, const GTexture* InTexture, const GMatrix2D& Matrix,
                             GRenderer::CacheProvider* Cache);                             // vt[41] 2013 0x5a18c0
    virtual void DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize,
                                          int StartIndex, int Count, const GTexture* InTexture,
                                          const GMatrix2D& Matrix,
                                          const GRenderer::DistanceFieldParams& Params,
                                          GRenderer::CacheProvider* Cache);                // vt[42] 2012 0x5e2670
    virtual void BeginSubmitMask(GRenderer::SubmitMaskMode Mode);                           // vt[43] 2012 0x5dce70
    virtual void EndSubmitMask();                                                           // vt[44] 2012 0x5cfe30
    virtual void DisableMask();                                                             // vt[45] 2012 0x5d4100
    virtual unsigned int CheckFilterSupport(const GRenderer::BlurFilterParams& Params);     // vt[46] 2012 0x5b7eb0
    virtual void DrawBlurRect(GTexture* Source, const GRect<float>& Dest, const GRect<float>& Src,
                              const GRenderer::BlurFilterParams& Params, bool bOnStack);    // vt[47] 2012 0x5df5d0
    virtual void DrawColorMatrixRect(GTexture* Source, const GRect<float>& Dest,
                                     const GRect<float>& Src, const float* Matrix,
                                     bool bOnStack);                                        // vt[48] 2012 0x5df8b0
    virtual void GetRenderStats(GRenderer::Stats* Stats, bool bReset);                      // vt[49] 2012 0x5b7c90
    virtual void GetStats(GStatBag* Bag, bool bReset);                                       // vt[50]
    virtual void ReleaseResources();                                                         // vt[51] 2013 0x59db10
    virtual bool AddEventHandler(GRendererEventHandler* Handler);                            // vt[52] GRenderer's own in retail
    virtual void RemoveEventHandler(GRendererEventHandler* Handler);                         // vt[53] GRenderer's own in retail

    // The render-thread half. Everything above that touches the RHI goes through one of these,
    // enqueued with ENQUEUE_RENDER_COMMAND exactly as retail's generated commands do.
    void SetUIViewport(FGFxViewportUserParams& InViewportParams);                            // 2012 0x5c4820
    void InitUIBlendStackAndMiscRenderState_RenderingThread(FMiscRenderStateInitParams& Params); // 2012 0x5d7e10
    void EndDisplay_RenderThread();                                                          // 2012 0x5cc9c0
    void ApplyUITransform_RenderThread(const GMatrix2D& InViewportMatrix,
                                       const GMatrix2D& TransformMatrix,
                                       FGFxVertexShaderInterface& VertexShader);             // 2012 0x5baf50
    void DrawIndexedTriList_RenderThread(int BaseVertexIndex, int MinVertexIndex, int NumVertices,
                                         int StartIndex, int TriangleCount);                 // 2012 0x5db740
    void DrawLineStrip_RenderThread(int BaseVertexIndex, int LineCount);                     // 2012 0x5db940
    void DrawBitmaps_RenderThread(FGFxRendererImpl::FGFxBitmapDescStore* Store,
                                  int StartIndex, int Count,
                                  const FGFxTexture* Texture, const GMatrix2D& Matrix,
                                  const GRenderer::DistanceFieldParams* Params);             // 2012 0x5dbb30
    void DrawBlurRect_RenderThread(GTexture* Source, const GRect<float>& Dest,
                                   const GRect<float>& Src,
                                   const GRenderer::BlurFilterParams& Params);               // 2012 0x5dc220
    void DrawColorMatrixRect_RenderThread(GTexture* Source, const GRect<float>& Dest,
                                          const GRect<float>& Src, const float* Matrix);     // 2012 0x5dbf50
    void BeginSubmitMask_RenderThread(GRenderer::SubmitMaskMode Mode);                        // 2012 0x5d8170
    void EndSubmitMask_RenderThread();                                                        // 2012 0x5c4df0
    void DisableMask_RenderThread();                                                          // 2012 0x5cd090
    void PushRenderTarget_RenderThread(const GRect<float>& FrameRect, GRenderTarget* Target);  // 2012 0x5d5580
    void PopRenderTarget_RenderThread();                                                       // 2012 0x5d58e0
    FGFxTexture* PushTempRenderTarget_RenderThread(FGFxTexture* StubTexture,
                                                   const GRect<float>& FrameRect,
                                                   unsigned int Width, unsigned int Height,
                                                   bool bWantStencil);                         // 2012 0x5d95c0
    void ReleaseTempRenderTargets_RenderThread(unsigned int KeepArea);                          // 2012 0x5db250
    FSamplerStateRHIRef GetSamplerState(GRenderer::BitmapSampleMode SampleMode,
                                        GRenderer::BitmapWrapMode WrapMode,
                                        unsigned int bUseMips);                                // 2012 0x5bb540
    void CheckRenderTarget_RenderThread();                                                     // 2012 0x5d7c80
};
