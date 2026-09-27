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
// DISHONORED(port): the GFx 3.3 renderer seam. This is the half of the UI that is OURS whichever
// runtime drives it: GRenderer (54 vtable slots), GTexture (12) and GRenderTarget (7) are
// interfaces libgfx calls into, and none of them is implemented anywhere in this tree today.
// resources/docs/gfx_decision.md 2.3 measured the shape; the layouts and the slot numbers below
// come out of the 2012 Shipping PDB through resources/tools/pdb/dia_types.py, and the 2013 rvas
// come from resources/docs/symbols/match_2012_2013.csv.
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
// State of the port, stated plainly: every slot is declared and defined, so the seam builds, links
// and can be handed to the runtime; the bodies are DISHONORED(bringup) - they record the call in
// the seam census and return a neutral value. Nothing draws yet, which is what package BB's
// acceptance asks for. The 892-byte retail FGFxRenderer layout (35 members, every one listed in
// build/agentBB/fgfx_types.json with its offset) is deliberately NOT reproduced: FGFxRenderer is a
// runtime object that nothing serializes, and most of its members are RHI resource references whose
// port belongs with the real drawing code.
//
// MSVC lays a run of consecutive virtual overloads out in REVERSE declaration order (agentAL.md 4,
// re-proved for this toolchain in build/agentBB/vt/vtprobe.cod), so FGFxRenderTarget's
// InitRenderTarget_RenderThread pair below is declared slot 9 first, slot 8 second.
#include "GFx3.h"

// Engine types the seam holds by pointer. Forward declarations only: the seam compiles both inside
// the GFxUI module and on its own (build/agentBB_seam.cmd), which is how "every slot is present"
// is checked without a full engine build.
class FViewport;
class FRenderTarget;
class FTexture;
class UTexture;
class UTexture2D;
class FGFxRenderResources;
class FGFxRenderTargetResource;
class FGFxPixelShaderInterface;
class FGFxRenderer;
class FGFxRenderTarget;
class FGFxTexture;

// ---------------------------------------------------------------------------------------------
// The seam census. Every bringup body records itself here, so a run can report exactly which of
// the 54 + 12 + 7 + 4 + 19 slots the runtime reached, in the
// "DISHONORED(bringup): <thing> census: <N> ..." shape resources/tools/run_regression.py expects.
// ---------------------------------------------------------------------------------------------
enum { GFXUI_SEAM_MAX_SLOTS = 192 };

void         FGFxSeamNote(const char* Slot);
unsigned int FGFxSeamCalls(const char* Slot);
unsigned int FGFxSeamSlotsTouched();
unsigned int FGFxSeamTotalCalls();
unsigned int FGFxSeamCensus(char* Out, unsigned int Capacity);
void         FGFxSeamReset();

#define GFXUI_SEAM_TRACE(name) FGFxSeamNote(name)

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
    virtual ~FGFxTexture();                                                              // vt[0]
    virtual bool InitTexture(GImageBase* Image, unsigned int Usage);                      // vt[1]
    virtual bool InitDynamicTexture(int Width, int Height, GImageBase::ImageFormat Format,
                                    int Mipmaps, unsigned int Usage);                    // vt[2]
    virtual void Update(int Level, int NumRects, const GTexture::UpdateRect* Rects,
                        const GImageBase* Image);                                        // vt[3]
    virtual int  Map(int Level, int NumRects, GTexture::MapRect* Maps, int Flags);        // vt[4]
    virtual bool Unmap(int Level, int NumRects, GTexture::MapRect* Maps, int Flags);      // vt[5]
    virtual GRenderer* GetRenderer() const;                                              // vt[6]
    virtual bool IsDataValid() const;                                                    // vt[7]
    virtual void* GetUserData() const;                                                   // vt[8]
    virtual void SetUserData(void* Data);                                                // vt[9]
    virtual void AddChangeHandler(GTexture::ChangeHandler* Handler);                      // vt[10]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* Handler);                   // vt[11]

    // FGFxTexture's own
    virtual bool InitTexture(UTexture* InTexture, bool bAsRenderTarget);                 // vt[12]
    virtual bool InitTextureFromFile(const char* FileName);                              // vt[13]
    virtual int  IsYUVTexture() const;                                                   // vt[14]
    virtual void Bind(int Stage, FGFxPixelShaderInterface& Shader,
                      GRenderer::BitmapWrapMode WrapMode,
                      GRenderer::BitmapSampleMode SampleMode, bool bUseMips) const;      // vt[15]
    virtual void InternalTermGCState();                                                  // vt[16]
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderTarget - GRenderTarget (7 slots) plus 3 of its own. PDB sizeof 36.
// ---------------------------------------------------------------------------------------------
class FGFxRenderTarget : public GRenderTarget
{
public:
    GRenderer*                Renderer;      // @8
    GPtr<FGFxTexture>         Texture;       // @12
    // Retail holds a GPtr<FGFxRenderResources> here; the raw pointer is the same 4 bytes at the
    // same offset and does not need the render-resource type to be complete in this header.
    FGFxRenderResources*      StencilBuffer; // @16
    FGFxRenderTargetResource* Resource;      // @20 (a reference in retail)
    unsigned int              TargetWidth;   // @24
    unsigned int              TargetHeight;  // @28
    bool                      IsTemp;        // @32

    FGFxRenderTarget(GRenderer* InRenderer);                                  // 2013 0x58d240

    // GRenderTarget
    virtual ~FGFxRenderTarget();                                                          // vt[0]
    virtual bool InitRenderTarget(GTexture* Color, GTexture* DepthStencil,
                                  GTexture* Resolve);                                     // vt[1]
    virtual GRenderer* GetRenderer() const;                                               // vt[2]
    virtual void* GetUserData() const;                                                    // vt[3]
    virtual void SetUserData(void* Data);                                                 // vt[4]
    virtual void AddChangeHandler(GTexture::ChangeHandler* Handler);                       // vt[5]
    virtual void RemoveChangeHandler(GTexture::ChangeHandler* Handler);                    // vt[6]

    // FGFxRenderTarget's own. Slots 8 and 9 are a consecutive same-name run, so they are declared
    // highest first: slot 9 before slot 8.
    virtual bool InitRenderTarget(const FGFxRenderTargetResource& Native);                 // vt[7]
    virtual bool InitRenderTarget_RenderThread(GTexture* Color, FGFxRenderResources* Stencil,
                                               unsigned int Width, unsigned int Height);   // vt[9]
    virtual bool InitRenderTarget_RenderThread(const FGFxRenderTargetResource& Native);     // vt[8]

    bool AdjustBounds(float* Width, float* Height);   // 2012 0x5b7cd0, non-virtual
};

// ---------------------------------------------------------------------------------------------
// FGFxRenderer - all 54 GRenderer slots. Retail overrides 43 of them and leaves 11 to GRenderer's
// own bodies; since there is no libgfx to link against, those 11 are overridden here too and each
// one says so.
// ---------------------------------------------------------------------------------------------
class FGFxRenderer : public GRenderer
{
public:
    GRenderer::Stats     RenderStats;        // @44
    FViewport*           Viewport;           // @64
    FRenderTarget*       RenderTarget;       // @68
    unsigned int         RenderMode;         // @72
    GMatrix2D            UserMatrix;         // @84
    GMatrix2D            CurrentMatrix;      // @108
    GMatrix2D            ViewportMatrix;     // @132
    GRenderer::Cxform    CurrentCxform;      // @156
    float                InverseGamma;       // @252
    GMatrix3D            ViewMatrix;         // @260
    GMatrix3D            ProjMatrix;         // @324
    GMatrix3D            WorldMatrix;        // @388
    GMatrix3D            UVPMatrix;          // @452
    unsigned int         UVPMatricesChanged; // @516
    unsigned int         Is3DEnabled;        // @520
    FGFxRenderTarget*    CurRenderTarget;    // @524
    unsigned int         CurRenderTargetSet; // @528
    GRenderer::BlendType BlendMode;          // @600
    unsigned int         bAlphaComposite;    // @616
    unsigned int         MaxTempRTSize;      // @620
    unsigned long        StencilCounter;     // @888

    FGFxRenderer();                                                            // 2013 0x5a1f30
    virtual ~FGFxRenderer();                                                              // vt[0]

    virtual void ScopedEventCallback(const char* Name);                                   // vt[1]  GRenderer's own in retail
    virtual void SaveCurrentRenderTargetContents();                                       // vt[2]  GRenderer's own in retail
    virtual void RestoreCurrentRenderTargetContents();                                    // vt[3]  GRenderer's own in retail
    virtual bool GetRenderCaps(GRenderer::RenderCaps* Caps);                              // vt[4]  2013 0x572e00
    virtual FGFxTexture* CreateTexture();                                                 // vt[5]  2013 0x5800d0
    virtual FGFxTexture* CreateTextureYUV();                                              // vt[6]
    virtual void BeginFrame();                                                            // vt[7]  GRenderer's own in retail
    virtual void EndFrame();                                                              // vt[8]  GRenderer's own in retail
    virtual FGFxRenderTarget* CreateRenderTarget();                                       // vt[9]  2013 0x58f120
    virtual void SetDisplayRenderTarget(GRenderTarget* Target, bool bSetState);            // vt[10]
    virtual void PushRenderTarget(const GRect<float>& FrameRect, GRenderTarget* Target);   // vt[11]
    virtual void PopRenderTarget();                                                       // vt[12]
    virtual FGFxTexture* PushTempRenderTarget(const GRect<float>& FrameRect, unsigned int Width,
                                              unsigned int Height, bool bWantStencil);    // vt[13]
    virtual void ReleaseTempRenderTargets(unsigned int KeepArea);                          // vt[14]
    virtual void BeginDisplay(GColor BackgroundColor, const GViewport& InViewport,
                              float x0, float x1, float y0, float y1);                    // vt[15] 2013 0x59dd90
    virtual void EndDisplay();                                                             // vt[16] 2013 0x593c40
    virtual void SetMatrix(const GMatrix2D& Matrix);                                       // vt[17] 2013 0x58ac30
    virtual void SetUserMatrix(const GMatrix2D& Matrix);                                   // vt[18]
    virtual void SetCxform(const GRenderer::Cxform& Cx);                                   // vt[19]
    virtual void PushBlendMode(GRenderer::BlendType Mode);                                 // vt[20]
    virtual void PopBlendMode();                                                           // vt[21]
    virtual bool PushUserData(GRenderer::UserData* Data);                                  // vt[22] GRenderer's own in retail
    virtual void PopUserData();                                                            // vt[23] GRenderer's own in retail
    virtual void SetPerspective3D(const GMatrix3D& Persp);                                 // vt[24]
    virtual void SetView3D(const GMatrix3D& View);                                         // vt[25]
    virtual void SetWorld3D(const GMatrix3D* World);                                       // vt[26]
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
                                   GRenderer::CachedDataType Type);                        // vt[32]
    virtual void DrawIndexedTriList(int BaseVertexIndex, int MinVertexIndex, int NumVertices,
                                    int StartIndex, int TriangleCount);                    // vt[33] 2013 0x5a1620
    virtual void DrawLineStrip(int BaseVertexIndex, int LineCount);                         // vt[34]
    virtual void LineStyleDisable();                                                        // vt[35]
    virtual void LineStyleColor(GColor Color);                                              // vt[36]
    virtual void FillStyleDisable();                                                        // vt[37]
    virtual void FillStyleColor(GColor Color);                                              // vt[38]
    virtual void FillStyleBitmap(const GRenderer::FillTexture* Fill);                       // vt[39]
    virtual void FillStyleGouraud(GRenderer::GouraudFillType Type,
                                  const GRenderer::FillTexture* T0,
                                  const GRenderer::FillTexture* T1,
                                  const GRenderer::FillTexture* T2);                       // vt[40]
    virtual void DrawBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize, int StartIndex,
                             int Count, const GTexture* InTexture, const GMatrix2D& Matrix,
                             GRenderer::CacheProvider* Cache);                             // vt[41] 2013 0x5a18c0
    virtual void DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize,
                                          int StartIndex, int Count, const GTexture* InTexture,
                                          const GMatrix2D& Matrix,
                                          const GRenderer::DistanceFieldParams& Params,
                                          GRenderer::CacheProvider* Cache);                // vt[42]
    virtual void BeginSubmitMask(GRenderer::SubmitMaskMode Mode);                           // vt[43]
    virtual void EndSubmitMask();                                                           // vt[44]
    virtual void DisableMask();                                                             // vt[45]
    virtual unsigned int CheckFilterSupport(const GRenderer::BlurFilterParams& Params);     // vt[46]
    virtual void DrawBlurRect(GTexture* Source, const GRect<float>& Dest, const GRect<float>& Src,
                              const GRenderer::BlurFilterParams& Params, bool bOnStack);    // vt[47]
    virtual void DrawColorMatrixRect(GTexture* Source, const GRect<float>& Dest,
                                     const GRect<float>& Src, const float* Matrix,
                                     bool bOnStack);                                        // vt[48]
    virtual void GetRenderStats(GRenderer::Stats* Stats, bool bReset);                      // vt[49]
    virtual void GetStats(GStatBag* Bag, bool bReset);                                      // vt[50]
    virtual void ReleaseResources();                                                        // vt[51] 2013 0x59db10
    virtual bool AddEventHandler(GRendererEventHandler* Handler);                           // vt[52] GRenderer's own in retail
    virtual void RemoveEventHandler(GRendererEventHandler* Handler);                        // vt[53] GRenderer's own in retail
};
