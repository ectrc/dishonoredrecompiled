// GFxUI/src/gfxuirenderer.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (221):
//   0x5b7710  void __cdecl FGFxRendererImpl::ConvertFromUI(struct GRenderer::FillTexture const &, class FGFxRenderer::FFillTextureInfo &)
//   0x5b7790  _FGFxRenderer::FGFxRenderStyle::Disable_::_2_::DisableUIStyleCommand::Execute
//   0x5b77a0  _FGFxRenderer::FGFxRenderStyle::Disable_::_2_::DisableUIStyleCommand::DescribeCommand
//   0x5b77b0  public: virtual void __thiscall FGFxRenderer::FGFxRenderStyle::GetEnumeratedBoundShaderState_RenderThread(void * const, void const * const)
//   0x5b77e0  public: virtual void __thiscall FGFxRenderer::FGFxFillStyle::GetEnumeratedBoundShaderState_RenderThread(void * const, void const * const)
//   0x5b78e0  public: virtual bool __thiscall FGFxRenderer::GetRenderCaps(struct GRenderer::RenderCaps *)
//   0x5b7930  _FGFxRenderer::SetPerspective3D_::_2_::SetPerspective3DCommand::Execute
//   0x5b7960  _FGFxRenderer::SetPerspective3D_::_2_::SetPerspective3DCommand::DescribeCommand
//   0x5b7970  _FGFxRenderer::SetView3D_::_2_::SetView3DCommand::Execute
//   0x5b79a0  _FGFxRenderer::SetView3D_::_2_::SetView3DCommand::DescribeCommand
//   0x5b79b0  _FGFxRenderer::SetWorld3D_::_5_::SetWorld3DCommand::Execute
//   0x5b79e0  _FGFxRenderer::SetWorld3D_::_5_::SetWorld3DCommand::DescribeCommand
//   0x5b79f0  _FGFxRenderer::SetWorld3D_::_19_::SetNullWorld3DCommand::Execute
//   0x5b7a00  _FGFxRenderer::SetWorld3D_::_19_::SetNullWorld3DCommand::DescribeCommand
//   0x5b7a10  _FGFxRenderer::SetDisplayRenderTarget_::_2_::SetRenderTarget::DescribeCommand
//   0x5b7a20  _FGFxRenderer::PushRenderTarget_::_4_::PushRenderTargetCommand::DescribeCommand
//   0x5b7a30  _FGFxRenderer::PopRenderTarget_::_2_::PushRenderTargetCommand::DescribeCommand
//   0x5b7a40  _FGFxRenderer::PushTempRenderTarget_::_2_::PushRenderTargetCommand::DescribeCommand
//   0x5b7a50  _FGFxRenderer::ReleaseTempRenderTargets_::_2_::ReleaseTempRenderTargetCommand::DescribeCommand
//   0x5b7a60  _FGFxRenderer::SetUIViewport_::_2_::SetUIViewportCommand::Execute
//   0x5b7af0  _FGFxRenderer::SetUIViewport_::_2_::SetUIViewportCommand::DescribeCommand
//   0x5b7b00  _FGFxRenderer::BeginDisplay_::_2_::InitUIBlendStackAndMiscRenderStateCommand::DescribeCommand
//   0x5b7b10  _FGFxRenderer::BeginDisplay_::_18_::DrawUIBackgroundColorCommand::DescribeCommand
//   0x5b7b20  _FGFxRenderer::EndDisplay_::_2_::EndDisplayCommand::DescribeCommand
//   0x5b7b30  _FGFxRenderer::PushBlendMode_::_2_::PushUIBlendModeCommand::DescribeCommand
//   0x5b7b40  _FGFxRenderer::PopBlendMode_::_2_::PopUIBlendModeCommand::DescribeCommand
//   0x5b7b50  _FGFxRenderer::DrawIndexedTriList_::_2_::DrawUIIndexedTriListCommand::DescribeCommand
//   0x5b7b60  _FGFxRenderer::DrawLineStrip_::_2_::DrawLineStripCommand::DescribeCommand
//   0x5b7b70  _FGFxRenderer::DrawBitmaps_::_17_::DrawBitmapsCommand::DescribeCommand
//   0x5b7b80  _FGFxRenderer::DrawDistanceFieldBitmaps_::_17_::DrawBitmapsCommand::DescribeCommand
//   0x5b7b90  _FGFxRenderer::FillStyleBitmap_::_2_::FillStyleBitmapCommand::DescribeCommand
//   0x5b7ba0  _FGFxRenderer::FillStyleColor_::_2_::FillStyleColorCommand::DescribeCommand
//   0x5b7bb0  _FGFxRenderer::FillStyleGouraud_::_2_::FillStyleGouraudCommand::FillStyleGouraudCommand
//   0x5b7c80  _FGFxRenderer::FillStyleGouraud_::_2_::FillStyleGouraudCommand::DescribeCommand
//   0x5b7c90  public: virtual void __thiscall FGFxRenderer::GetRenderStats(class GRenderer::Stats *, bool)
//   0x5b7cc0  _FGFxRenderer::LineStyleColor_::_2_::LineStyleColorCommand::DescribeCommand
//   0x5b7cd0  public: bool __thiscall FGFxRenderTarget::AdjustBounds(float *, float *)
//   0x5b7eb0  public: virtual unsigned int __thiscall FGFxRenderer::CheckFilterSupport(struct GRenderer::BlurFilterParams const &)
//   0x5b7ee0  _FGFxRenderer::DrawBlurRect_::_2_::DrawBlurRectCommand::DescribeCommand
//   0x5b7ef0  _FGFxRenderer::DrawColorMatrixRect_::_2_::DrawColorMatrixRectCommand::DescribeCommand
//   0x5b7f00  _FGFxRenderer::BeginSubmitMask_::_2_::BeginSubmitMaskCommand::DescribeCommand
//   0x5b7f10  _FGFxRenderer::EndSubmitMask_::_2_::EndSubmitMaskCommand::DescribeCommand
//   0x5b7f20  _FGFxRenderer::DisableMask_::_2_::DisableMaskCommand::DescribeCommand
//   0x5b7f40  FGFxRendererImpl::SoftwareResample
//   0x5b8060  FGFxRendererImpl::LoadTexture
//   0x5b8190  FGFxRendererImpl::ScaleformPerformGCCheck
//   0x5b81a0  _FGFxTexture::Update_::_4_::UpdateCommand::DescribeCommand
//   0x5b81b0  _FGFxRenderTarget::_FGFxRenderTarget_::_7_::InitRenderTargetCommand::DescribeCommand
//   0x5b81c0  _FGFxRenderTarget::InitRenderTarget_::_2_::InitRenderTargetCommand::Execute
//   0x5b81f0  _FGFxRenderTarget::InitRenderTarget_::_2_::InitRenderTargetCommand::DescribeCommand
//   0x5b82e0  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<struct GRenderer::BitmapDesc *>::InitElementsCopy(struct GRenderer::BitmapDesc *, unsigned int, void const *)
//   0x5b8d00  _FGFxRendererImpl::SetUITransformMatrix_GMatrix2D__::_2_::SetUITransformMatrixCommand::Execute
//   0x5b8d30  _FGFxRendererImpl::SetUITransformMatrix_GMatrix2D__::_2_::SetUITransformMatrixCommand::DescribeCommand
//   0x5b8d40  _FGFxRendererImpl::SetUITransformMatrix_GRenderer::Cxform__::_2_::SetUITransformMatrixCommand::Execute
//   0x5b8d60  _FGFxRendererImpl::SetUITransformMatrix_GRenderer::Cxform__::_2_::SetUITransformMatrixCommand::DescribeCommand
//   0x5b8d70  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxVertexStore__::_5_::DestroyUIElementStoreCommand::DescribeCommand
//   0x5b8d80  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxVertexStore__::_31_::TransferNewUIRenderElementStoreCommand::DescribeCommand
//   0x5b8d90  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxIndexStore__::_5_::DestroyUIElementStoreCommand::DescribeCommand
//   0x5b8da0  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxIndexStore__::_31_::TransferNewUIRenderElementStoreCommand::DescribeCommand
//   0x5b8db0  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<enum GRenderer::VertexFormat>::InitElementsCopy(enum GRenderer::VertexFormat, unsigned int, void const *)
//   0x5b8e40  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<enum GRenderer::IndexFormat>::InitElementsCopy(enum GRenderer::IndexFormat, unsigned int, void const *)
//   0x5bab20  public: virtual __thiscall FGFxRenderResources::~FGFxRenderResources(void)
//   0x5babe0  class TDynamicRHIResourceReference<6> __cdecl FGFxRendererImpl::GetUIVertexDecl_RenderThread(enum EGFxVertexDeclarationType, unsigned long *)
//   0x5badc0  void __cdecl FGFxRendererImpl::ConvertFromUI(class GColor, struct FLinearColor &)
//   0x5bae50  void __cdecl FGFxRendererImpl::ApplyUIColor_RenderThread(class FGFxRenderer *, class GColor, enum GRenderer::BlendType, class FGFxPixelShaderInterface &)
//   0x5baf50  public: void __thiscall FGFxRenderer::ApplyUITransform_RenderThread(class GMatrix2D const &, class GMatrix2D const &, class FGFxVertexShaderInterface &)
//   0x5bb150  public: virtual void __thiscall FGFxRenderer::FGFxRenderStyle::EndDisplay_RenderThread(void)
//   0x5bb160  public: virtual void __thiscall FGFxRenderer::FGFxFillStyle::EndDisplay_RenderThread(void)
//   0x5bb240  public: void __thiscall FGFxRenderer::FGFxRenderStyle::Disable(void)
//   0x5bb350  public: void __thiscall FGFxRenderer::FGFxFillStyle::SetStyleBitmap_RenderThread(class FGFxRenderer::FFillTextureInfo const &, class GRenderer::Cxform const &)
//   0x5bb3b0  public: void __thiscall FGFxRenderer::FGFxFillStyle::SetStyleGouraud_RenderThread(enum GRenderer::GouraudFillType, class FGFxRenderer::FFillTextureInfo const * const, class FGFxRenderer::FFillTextureInfo const * const, class FGFxRenderer::FFillTextureInfo const * const, class GRenderer::Cxform const &)
//   0x5bb480  private: static void __cdecl FGFxRenderer::FGFxFillStyle::StaticApplyTextureMatrix_RenderThread(void const * const, void const * const, class FGFxRenderer::FFillTextureInfo const &, int)
//   0x5bb540  public: class TDynamicRHIResourceReference<1> __thiscall FGFxRenderer::GetSamplerState(enum GRenderer::BitmapSampleMode, enum GRenderer::BitmapWrapMode, unsigned int)
//   0x5bb690  public: virtual void __thiscall FGFxRenderer::SetPerspective3D(class GMatrix3D const &)
//   0x5bb7d0  public: virtual void __thiscall FGFxRenderer::SetView3D(class GMatrix3D const &)
//   0x5bb910  public: virtual void __thiscall FGFxRenderer::SetWorld3D(class GMatrix3D const *)
//   0x5bbb60  _FGFxRenderer::SetDisplayRenderTarget_::_2_::SetRenderTarget::Execute
//   0x5bbc80  _FGFxRenderer::SetUIViewport_::_2_::SetUIViewportCommand::SetUIViewportCommand
//   0x5bbcd0  public: virtual void __thiscall FGFxRenderer::FillStyleDisable(void)
//   0x5bbce0  public: virtual void __thiscall FGFxRenderer::LineStyleDisable(void)
//   0x5bbcf0  protected: void __thiscall FGFxTexture::LoadMipLevel(int, int, unsigned char const *, unsigned int, unsigned int, int, unsigned int)
//   0x5bbd70  protected: void __thiscall FGFxTexture::Update_RenderThread(int, int, struct FUpdateTextureRegion2D const *, class GImageBase const *)
//   0x5bbe80  _FGFxTexture::Update_::_4_::UpdateCommand::Execute
//   0x5bbed0  public: virtual void __thiscall FGFxTexture::Bind(int, class FGFxPixelShaderInterface &, enum GRenderer::BitmapWrapMode, enum GRenderer::BitmapSampleMode, bool)const
//   0x5bbff0  public: virtual int __thiscall FGFxTexture::Map(int, int, struct GTexture::MapRect *, int)
//   0x5bc0a0  public: virtual bool __thiscall FGFxTexture::Unmap(int, int, struct GTexture::MapRect *, int)
//   0x5bc0e0  public: virtual __thiscall FGFxRenderTargetResource::~FGFxRenderTargetResource(void)
//   0x5bc1b0  public: virtual bool __thiscall FGFxRenderTarget::InitRenderTarget(class GTexture *, class GTexture *, class GTexture *)
//   0x5bd9e0  void __cdecl FGFxRendererImpl::SetUITransformMatrix<class GRenderer::Cxform>(class GRenderer::Cxform const &, class GRenderer::Cxform &)
//   0x5c4430  public: virtual void __thiscall FGFxRenderResources::InitDynamicRHI(void)
//   0x5c44f0  public: virtual void __thiscall FGFxRenderResources::ReleaseDynamicRHI(void)
//   0x5c4510  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStoreBase::Release_MainThread(class GLock *)
//   0x5c4630  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStoreBase::Release_RenderThread(class GLock *)
//   0x5c46d0  public: virtual void __thiscall FGFxRenderer::FGFxRenderStyle::Apply_RenderThread(class FGFxRenderer *, void const * const, void * const)
//   0x5c4720  private: static void __cdecl FGFxRenderer::FGFxFillStyle::StaticApplyTexture_RenderThread(class FGFxRenderer *, void const * const, void const * const, class FGFxRenderer::FFillTextureInfo const &, int)
//   0x5c47d0  public: virtual class FGFxTexture * __thiscall FGFxRenderer::CreateTexture(void)
//   0x5c4820  public: void __thiscall FGFxRenderer::SetUIViewport(struct FGFxViewportUserParams &)
//   0x5c4b60  public: virtual void __thiscall FGFxRenderer::SetCxform(class GRenderer::Cxform const &)
//   0x5c4b80  public: virtual void __thiscall FGFxRenderer::ReleaseCachedData(class GRenderer::CachedData *, enum GRenderer::CachedDataType)
//   0x5c4ba0  _FGFxRenderer::DrawBitmaps_::_17_::DrawBitmapsCommand::DrawBitmapsCommand
//   0x5c4cd0  _FGFxRenderer::FillStyleBitmap_::_2_::FillStyleBitmapCommand::FillStyleBitmapCommand
//   0x5c4d20  _FGFxRenderer::FillStyleBitmap_::_2_::FillStyleBitmapCommand::Execute
//   0x5c4d40  _FGFxRenderer::FillStyleColor_::_2_::FillStyleColorCommand::Execute
//   0x5c4d80  _FGFxRenderer::FillStyleGouraud_::_2_::FillStyleGouraudCommand::Execute
//   0x5c4db0  _FGFxRenderer::LineStyleColor_::_2_::LineStyleColorCommand::Execute
//   0x5c4df0  public: void __thiscall FGFxRenderer::EndSubmitMask_RenderThread(void)
//   0x5c4f10  public: virtual bool __thiscall FGFxTexture::InitTexture(class UTexture *, bool)
//   0x5c4f50  public: virtual void __thiscall FGFxRenderTargetResource::InitDynamicRHI(void)
//   0x5c5110  public: virtual void __thiscall FGFxRenderTargetResource::ReleaseDynamicRHI(void)
//   0x5c6750  void __cdecl FGFxRendererImpl::SetUITransformMatrix<class GMatrix2D>(class GMatrix2D const &, class GMatrix2D &)
//   0x5c68e0  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxIndexStore__::_5_::DestroyUIElementStoreCommand::Execute
//   0x5c6910  _FGFxRendererImpl::SetUIRenderElementStore_FGFxRendererImpl::FGFxIndexStore__::_31_::TransferNewUIRenderElementStoreCommand::Execute
//   0x5cc020  void __cdecl FGFxRendererImpl::ApplyUIBlendMode_RenderThread(unsigned int, enum GRenderer::BlendType, unsigned int)
//   0x5cc820  public: virtual void __thiscall FGFxRenderer::FGFxFillStyle::Apply_RenderThread(class FGFxRenderer *, void const * const, void * const)
//   0x5cc8e0  _FGFxRenderer::BeginDisplay_::_17_::FDrawUIBackGroundColorParams::FDrawUIBackGroundColorParams
//   0x5cc9c0  public: void __thiscall FGFxRenderer::EndDisplay_RenderThread(void)
//   0x5cca20  public: virtual void __thiscall FGFxRenderer::SetMatrix(class GMatrix2D const &)
//   0x5cca40  public: virtual void __thiscall FGFxRenderer::SetUserMatrix(class GMatrix2D const &)
//   0x5cca60  _FGFxRenderer::PushBlendMode_::_2_::PushUIBlendModeCommand::Execute
//   0x5ccaa0  _FGFxRenderer::DrawDistanceFieldBitmaps_::_17_::DrawBitmapsCommand::DrawBitmapsCommand
//   0x5ccac0  public: virtual void __thiscall FGFxRenderer::FillStyleBitmap(struct GRenderer::FillTexture const *)
//   0x5ccc60  public: virtual void __thiscall FGFxRenderer::FillStyleColor(class GColor)
//   0x5ccdb0  public: virtual void __thiscall FGFxRenderer::FillStyleGouraud(enum GRenderer::GouraudFillType, struct GRenderer::FillTexture const *, struct GRenderer::FillTexture const *, struct GRenderer::FillTexture const *)
//   0x5ccf30  public: virtual void __thiscall FGFxRenderer::LineStyleColor(class GColor)
//   0x5cd080  _FGFxRenderer::EndSubmitMask_::_2_::EndSubmitMaskCommand::Execute
//   0x5cd090  public: void __thiscall FGFxRenderer::DisableMask_RenderThread(void)
//   0x5cf8d0  public: virtual class FGFxRenderTarget * __thiscall FGFxRenderer::CreateRenderTarget(void)
//   0x5cf9e0  _FGFxRenderer::PushRenderTarget_::_4_::PushRenderTargetCommand::PushRenderTargetCommand
//   0x5cfa40  _FGFxRenderer::EndDisplay_::_2_::EndDisplayCommand::Execute
//   0x5cfa50  public: virtual void __thiscall FGFxRenderer::PushBlendMode(enum GRenderer::BlendType)
//   0x5cfbd0  _FGFxRenderer::PopBlendMode_::_2_::PopUIBlendModeCommand::Execute
//   0x5cfe30  public: virtual void __thiscall FGFxRenderer::EndSubmitMask(void)
//   0x5cff50  _FGFxRenderer::DisableMask_::_2_::DisableMaskCommand::Execute
//   0x5cff60  protected: virtual void __thiscall FGFxTexture::InternalTermGCState(void)
//   0x5cffd0  public: virtual __thiscall FGFxRenderTarget::~FGFxRenderTarget(void)
//   0x5d0140  public: virtual bool __thiscall FGFxRenderTarget::InitRenderTarget_RenderThread(class GTexture *, class FGFxRenderResources *, unsigned int, unsigned int)
//   0x5d3b10  public: virtual void __thiscall FGFxRenderer::SetDisplayRenderTarget(class GRenderTarget *, bool)
//   0x5d3d90  _FGFxRenderer::BeginDisplay_::_18_::DrawUIBackgroundColorCommand::DrawUIBackgroundColorCommand
//   0x5d3e40  public: virtual void __thiscall FGFxRenderer::EndDisplay(void)
//   0x5d3f60  public: virtual void __thiscall FGFxRenderer::PopBlendMode(void)
//   0x5d4100  public: virtual void __thiscall FGFxRenderer::DisableMask(void)
//   0x5d4220  public: virtual __thiscall FGFxTexture::~FGFxTexture(void)
//   0x5d42b0  public: virtual void __thiscall FGFxTexture::Update(int, int, struct GTexture::UpdateRect const *, class GImageBase const *)
//   0x5d4540  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<struct GRenderer::BitmapDesc *>::InitElements(class FGFxRenderer *, class GRenderer::CachedData *, struct GRenderer::BitmapDesc *, unsigned int, void *)
//   0x5d47f0  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<enum GRenderer::VertexFormat>::InitElements(class FGFxRenderer *, class GRenderer::CachedData *, enum GRenderer::VertexFormat, unsigned int, void *)
//   0x5d4880  public: void __thiscall FGFxRendererImpl::FGFxRenderElementStore<enum GRenderer::IndexFormat>::InitElements(class FGFxRenderer *, class GRenderer::CachedData *, enum GRenderer::IndexFormat, unsigned int, void *)
//   0x5d5580  public: void __thiscall FGFxRenderer::PushRenderTarget_RenderThread(class GRect<float> const &, class GRenderTarget *)
//   0x5d58e0  public: void __thiscall FGFxRenderer::PopRenderTarget_RenderThread(void)
//   0x5d77f0  class FGFxPixelShaderInterface * __cdecl GetUIPixelShaderInterface_RenderThread(enum EGFxPixelShaderType)
//   0x5d7b00  class FGFxVertexShaderInterface * __cdecl GetUIVertexShaderInterface_RenderThread(enum EGFxVertexShaderType)
//   0x5d7c50  _FGFxRenderer::PushRenderTarget_::_4_::PushRenderTargetCommand::Execute
//   0x5d7c70  _FGFxRenderer::PopRenderTarget_::_2_::PushRenderTargetCommand::Execute
//   0x5d7c80  private: void __thiscall FGFxRenderer::CheckRenderTarget_RenderThread(void)
//   0x5d7e10  private: void __thiscall FGFxRenderer::InitUIBlendStackAndMiscRenderState_RenderingThread(struct FGFxRenderer::FMiscRenderStateInitParams &)
//   0x5d80a0  _FGFxRenderer::BeginDisplay_::_2_::InitUIBlendStackAndMiscRenderStateCommand::Execute
//   0x5d8170  public: void __thiscall FGFxRenderer::BeginSubmitMask_RenderThread(enum GRenderer::SubmitMaskMode)
//   0x5d8300  public: virtual bool __thiscall FGFxTexture::InitTexture(class GImageBase *, unsigned int)
//   0x5d8590  public: virtual bool __thiscall FGFxTexture::InitDynamicTexture(int, int, enum GImageBase::ImageFormat, int, unsigned int)
//   0x5d8a10  void __cdecl FGFxRendererImpl::SetUIRenderElementStore<struct FGFxRendererImpl::FGFxVertexStore>(class FGFxRenderer *, enum GRenderer::CachedDataType, void const *, int, enum GRenderer::VertexFormat, class GRenderer::CacheProvider * const, struct FGFxRendererImpl::FGFxVertexStore * &)
//   0x5d8d60  void __cdecl FGFxRendererImpl::SetUIRenderElementStore<struct FGFxRendererImpl::FGFxIndexStore>(class FGFxRenderer *, enum GRenderer::CachedDataType, void const *, int, enum GRenderer::IndexFormat, class GRenderer::CacheProvider * const, struct FGFxRendererImpl::FGFxIndexStore * &)
//   0x5d9270  public: virtual void __thiscall FGFxRenderer::PushRenderTarget(class GRect<float> const &, class GRenderTarget *)
//   0x5d94a0  public: virtual void __thiscall FGFxRenderer::PopRenderTarget(void)
//   0x5d95c0  public: class FGFxTexture * __thiscall FGFxRenderer::PushTempRenderTarget_RenderThread(class FGFxTexture *, class GRect<float> const &, unsigned int, unsigned int, bool)
//   0x5d9ca0  public: virtual void __thiscall FGFxRenderer::SetVertexData(void const *, int, enum GRenderer::VertexFormat, class GRenderer::CacheProvider *)
//   0x5d9cd0  public: virtual void __thiscall FGFxRenderer::SetIndexData(void const *, int, enum GRenderer::IndexFormat, class GRenderer::CacheProvider *)
//   0x5d9d00  _FGFxRenderer::BeginSubmitMask_::_2_::BeginSubmitMaskCommand::Execute
//   0x5daef0  void __cdecl FGFxRendererImpl::GetUIBoundShaderState_RenderThread(struct FGFxBoundShaderState &, class TMap<unsigned long, class TDynamicRHIResourceReference<9>, class FDefaultSetAllocator> &, struct FGFxEnumeratedBoundShaderState const &)
//   0x5db0d0  void __cdecl FGFxRendererImpl::DrawUIBackgroundColor_RenderThread(class FGFxRenderer *, class GColor, class TArray<struct FGFxVertex_XY16i, class FDefaultAllocator> const &, struct FGFxTransformHandles const &, class TMap<unsigned long, class TDynamicRHIResourceReference<9>, class FDefaultSetAllocator> &)
//   0x5db220  _FGFxRenderer::PushTempRenderTarget_::_2_::PushRenderTargetCommand::Execute
//   0x5db250  public: void __thiscall FGFxRenderer::ReleaseTempRenderTargets_RenderThread(unsigned int)
//   0x5db710  _FGFxRenderer::BeginDisplay_::_18_::DrawUIBackgroundColorCommand::Execute
//   0x5db740  public: void __thiscall FGFxRenderer::DrawIndexedTriList_RenderThread(int, int, int, int, int)
//   0x5db940  public: void __thiscall FGFxRenderer::DrawLineStrip_RenderThread(int, int)
//   0x5dbb30  public: void __thiscall FGFxRenderer::DrawBitmaps_RenderThread(struct FGFxRendererImpl::FGFxBitmapDescStore *, int, int, class FGFxTexture const *, class GMatrix2D const &, struct GRenderer::DistanceFieldParams const *)
//   0x5dbf50  public: void __thiscall FGFxRenderer::DrawColorMatrixRect_RenderThread(class GTexture *, class GRect<float> const &, class GRect<float> const &, float const *)
//   0x5dc220  public: void __thiscall FGFxRenderer::DrawBlurRect_RenderThread(class GTexture *, class GRect<float> const &, class GRect<float> const &, struct GRenderer::BlurFilterParams const &)
//   0x5dce20  _FGFxRenderer::DrawBlurRect_::_2_::DrawBlurRectCommand::Execute
//   0x5dce50  _FGFxRenderer::DrawColorMatrixRect_::_2_::DrawColorMatrixRectCommand::Execute
//   0x5dce70  public: virtual void __thiscall FGFxRenderer::BeginSubmitMask(enum GRenderer::SubmitMaskMode)
//   0x5dec80  public: virtual void __thiscall FGFxRenderer::ReleaseResources(void)
//   0x5decf0  public: virtual __thiscall FGFxRenderer::~FGFxRenderer(void)
//   0x5dee70  public: virtual class FGFxTexture * __thiscall FGFxRenderer::PushTempRenderTarget(class GRect<float> const &, unsigned int, unsigned int, bool)
//   0x5df060  _FGFxRenderer::ReleaseTempRenderTargets_::_2_::ReleaseTempRenderTargetCommand::Execute
//   0x5df080  public: virtual void __thiscall FGFxRenderer::BeginDisplay(class GColor, class GViewport const &, float, float, float, float)
//   0x5df520  _FGFxRenderer::DrawIndexedTriList_::_2_::DrawUIIndexedTriListCommand::Execute
//   0x5df550  _FGFxRenderer::DrawLineStrip_::_2_::DrawLineStripCommand::Execute
//   0x5df570  _FGFxRenderer::DrawBitmaps_::_17_::DrawBitmapsCommand::Execute
//   0x5df5a0  _FGFxRenderer::DrawDistanceFieldBitmaps_::_17_::DrawBitmapsCommand::Execute
//   0x5df5d0  public: virtual void __thiscall FGFxRenderer::DrawBlurRect(class GTexture *, class GRect<float> const &, class GRect<float> const &, struct GRenderer::BlurFilterParams const &, bool)
//   0x5df8b0  public: virtual void __thiscall FGFxRenderer::DrawColorMatrixRect(class GTexture *, class GRect<float> const &, class GRect<float> const &, float const *, bool)
//   0x5e1bf0  public: __thiscall FGFxRenderer::FGFxRenderer(void)
//   0x5e2030  public: virtual void __thiscall FGFxRenderer::ReleaseTempRenderTargets(unsigned int)
//   0x5e2160  public: virtual void __thiscall FGFxRenderer::DrawIndexedTriList(int, int, int, int, int)
//   0x5e22c0  public: virtual void __thiscall FGFxRenderer::DrawLineStrip(int, int)
//   0x5e2400  public: virtual void __thiscall FGFxRenderer::DrawBitmaps(struct GRenderer::BitmapDesc *, int, int, int, class GTexture const *, class GMatrix2D const &, class GRenderer::CacheProvider *)
//   0x5e2670  public: virtual void __thiscall FGFxRenderer::DrawDistanceFieldBitmaps(struct GRenderer::BitmapDesc *, int, int, int, class GTexture const *, class GMatrix2D const &, struct GRenderer::DistanceFieldParams const &, class GRenderer::CacheProvider *)
//   0xba1f80  _dynamic_initializer_for__FGFxPixelShader_30_::StaticType__
//   0xba1fc0  _dynamic_initializer_for__FGFxPixelShader_31_::StaticType__
//   0xba2000  _dynamic_initializer_for__FGFxPixelShader_32_::StaticType__
//   0xba2040  _dynamic_initializer_for__FGFxPixelShader_33_::StaticType__
//   ... 21 more, see resources/docs/symbols/functions.csv
// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the bodies of the renderer seam, drawing half included. Declarations, slot
// numbers and 2013 rvas are in gfxuirenderer.h and gfxuirendererimpl.h; every function below names
// the 2012 rva of the retail body it was written from (build/agentCC/dec, build/agentCC/dec2,
// headless decompiles of resources/docs/idb/shipping2012_agentCC.i64).
//
// The shape is retail's, and it is worth stating because it is not the shape a Scaleform renderer
// back end has: every GRenderer slot runs on the game thread, keeps the state GFx asked for, and
// enqueues a render command; the _RenderThread half of each pair is what touches the RHI. Retail's
// commands are ENQUEUE_UNIQUE_RENDER_COMMAND expansions (the decompiles show the ring-buffer
// allocation inline), so the commands here are declared as FRenderCommand classes and enqueued with
// ENQUEUE_RENDER_COMMAND, which is the same code with the same single-threaded fallback.
//
// Geometry goes to the RHI as user-pointer draws - RHIDrawIndexedPrimitiveUP and RHIDrawPrimitiveUP
// straight out of the element store, under the element lock - exactly as retail does; there is no
// vertex buffer anywhere in the retail renderer's PDB list.
#include "gfxuirendererimpl.h"

IMPLEMENT_CLASS(UGFxUpdatableTexture);
IMPLEMENT_CLASS(UGFxMappableTexture);

// ---------------------------------------------------------------------------------------------
// The seam census.
// ---------------------------------------------------------------------------------------------
namespace
{
struct FGFxSeamEntry
{
    const char*  Name;
    unsigned int Calls;
};

FGFxSeamEntry GSeamSlots[GFXUI_SEAM_MAX_SLOTS];
unsigned int  GSeamSlotCount = 0;
unsigned int  GSeamTotalCalls = 0;
}

FGFxDrawCensus GGFxDrawCensus = { 0 };

void FGFxSeamNote(const char* Slot)
{
    ++GSeamTotalCalls;
    for (unsigned int i = 0; i < GSeamSlotCount; ++i)
    {
        // DISHONORED(bringup): no rva - the seam census has no retail counterpart. The name may not be
        // written yet: this counter is touched from the game thread and from the
        // render thread, and FGFxSeamReset rewinds the count without clearing the slots.
        if (GSeamSlots[i].Name == NULL)
        {
            continue;
        }
        if (GSeamSlots[i].Name == Slot || appStrcmpANSI(GSeamSlots[i].Name, Slot) == 0)
        {
            ++GSeamSlots[i].Calls;
            return;
        }
    }
    if (GSeamSlotCount < GFXUI_SEAM_MAX_SLOTS)
    {
        GSeamSlots[GSeamSlotCount].Name = Slot;
        GSeamSlots[GSeamSlotCount].Calls = 1;
        ++GSeamSlotCount;
    }
}

unsigned int FGFxSeamCalls(const char* Slot)
{
    for (unsigned int i = 0; i < GSeamSlotCount; ++i)
        if (appStrcmpANSI(GSeamSlots[i].Name, Slot) == 0)
            return GSeamSlots[i].Calls;
    return 0;
}

unsigned int FGFxSeamSlotsTouched() { return GSeamSlotCount; }
unsigned int FGFxSeamTotalCalls() { return GSeamTotalCalls; }

void FGFxSeamReset()
{
    GSeamSlotCount = 0;
    GSeamTotalCalls = 0;
    appMemzero(&GGFxDrawCensus, sizeof(GGFxDrawCensus));
}

unsigned int FGFxSeamCensus(char* Out, unsigned int Capacity)
{
    if (Out == NULL || Capacity == 0) return 0;
    INT n = appSprintfANSI(Out,
                           "DISHONORED(bringup): GFx seam census: %u slots touched, %u calls, "
                           "%u draws (%u trilist, %u line, %u bitmap, %u background, %u filter), "
                           "%u triangles, %u lines, %u glyphs, %u masks, %u bound shader states",
                           GSeamSlotCount, GSeamTotalCalls, GGFxDrawCensus.Draws,
                           GGFxDrawCensus.TriListDraws, GGFxDrawCensus.LineDraws,
                           GGFxDrawCensus.BitmapDraws, GGFxDrawCensus.BackgroundDraws,
                           GGFxDrawCensus.FilterDraws, GGFxDrawCensus.Triangles,
                           GGFxDrawCensus.Lines, GGFxDrawCensus.Glyphs, GGFxDrawCensus.MaskPasses,
                           GGFxDrawCensus.BoundShaderStates);
    if (n < 0) n = 0;
    unsigned int Used = (unsigned int)n;
    for (unsigned int i = 0; i < GSeamSlotCount && Used + 1 < Capacity; ++i)
    {
        const INT k = appSprintfANSI(Out + Used, ", %s %u", GSeamSlots[i].Name, GSeamSlots[i].Calls);
        if (k < 0) break;
        Used += (unsigned int)k;
    }
    Out[Used < Capacity ? Used : Capacity - 1] = 0;
    return Used;
}

// ---------------------------------------------------------------------------------------------
// The shader interface for a kind. 2012 GetUIPixelShaderInterface_RenderThread 0x5d77f0,
// GetUIVertexShaderInterface_RenderThread 0x5d7b00; the filter half is
// GetUIPixelShaderInterface2_RenderThread (2012 0x5d7360, gfxuishaders.cpp in retail, here too).
//
// DISHONORED(bringup): four of retail's 51 pixel-shader kinds have no cooked shader in this game's
// cache and therefore no declared type to instantiate - GFx_PS_TextTextureYUV{,Multiply,A,AMultiply}
// (46..49), which are the video path - and so do the three gaps in the filter enumeration (21, 23,
// 26). They answer NULL, which is what FGFxTexture::IsYUVTexture returning 0 keeps the renderer
// from asking for.
// ---------------------------------------------------------------------------------------------
#define GFXUI_PS_CASE(Kind) \
    case Kind: { TShaderMapRef<FGFxPixelShader<Kind> > Shader(GetGlobalShaderMap(GRHIShaderPlatform)); \
                 return Shader->GetShaderInterface(); }
#define GFXUI_VS_CASE(Kind) \
    case Kind: { TShaderMapRef<FGFxVertexShader<Kind> > Shader(GetGlobalShaderMap(GRHIShaderPlatform)); \
                 return Shader->GetShaderInterface(); }

FGFxPixelShaderInterface* GetUIPixelShaderInterface_RenderThread(EGFxPixelShaderType Type)
{
    switch (Type)
    {
        GFXUI_PS_CASE(GFx_PS_SolidColor)
        GFXUI_PS_CASE(GFx_PS_CxformTexture)
        GFXUI_PS_CASE(GFx_PS_CxformTextureMultiply)
        GFXUI_PS_CASE(GFx_PS_TextTexture)
        GFXUI_PS_CASE(GFx_PS_TextTextureColor)
        GFXUI_PS_CASE(GFx_PS_TextTextureColorMultiply)
        GFXUI_PS_CASE(GFx_PS_TextTextureSRGB)
        GFXUI_PS_CASE(GFx_PS_TextTextureSRGBMultiply)
        GFXUI_PS_CASE(GFx_PS_CxformGouraud)
        GFXUI_PS_CASE(GFx_PS_CxformGouraudNoAddAlpha)
        GFXUI_PS_CASE(GFx_PS_CxformGouraudTexture)
        GFXUI_PS_CASE(GFx_PS_Cxform2Texture)
        GFXUI_PS_CASE(GFx_PS_CxformGouraudMultiply)
        GFXUI_PS_CASE(GFx_PS_CxformGouraudMultiplyNoAddAlpha)
        GFXUI_PS_CASE(GFx_PS_CxformGouraudMultiplyTexture)
        GFXUI_PS_CASE(GFx_PS_CxformMultiply2Texture)
        GFXUI_PS_CASE(GFx_PS_TextTextureDFA)
    default:
        if (Type > FS2_None && Type < GFx_PS_SolidColor)
        {
            return GetUIPixelShaderInterface2_RenderThread(Type);
        }
        return NULL;
    }
}

// GetUIPixelShaderInterface2_RenderThread (2012 0x5d7360) is gfxuishaders.cpp's, next to the 26
// filter registrations it instantiates - which is also what pulls that unit into the link.

FGFxVertexShaderInterface* GetUIVertexShaderInterface_RenderThread(EGFxVertexShaderType Type)
{
    switch (Type)
    {
        GFXUI_VS_CASE(GFx_VS_Strip)
        GFXUI_VS_CASE(GFx_VS_Glyph)
        GFXUI_VS_CASE(GFx_VS_XY16iC32)
        GFXUI_VS_CASE(GFx_VS_XY16iCF32)
        GFXUI_VS_CASE(GFx_VS_XY16iCF32_NoTex)
        GFXUI_VS_CASE(GFx_VS_XY16iCF32_NoTexNoAlpha)
        GFXUI_VS_CASE(GFx_VS_XY16iCF32_T2)
    default:
        return NULL;
    }
}

#undef GFXUI_PS_CASE
#undef GFXUI_VS_CASE

// ---------------------------------------------------------------------------------------------
// namespace FGFxRendererImpl
// ---------------------------------------------------------------------------------------------
namespace FGFxRendererImpl
{

UINT GetElementSize(GRenderer::VertexFormat Format)
{
    switch (Format)
    {
    case GRenderer::Vertex_XY16i:     return 4;
    case GRenderer::Vertex_XY32f:     return 8;
    case GRenderer::Vertex_XY16iC32:  return 8;
    case GRenderer::Vertex_XY16iCF32: return 12;
    default:                          return 0;
    }
}

UINT GetElementSize(GRenderer::IndexFormat Format)
{
    switch (Format)
    {
    case GRenderer::Index_16: return 2;
    case GRenderer::Index_32: return 4;
    default:                  return 0;
    }
}

UINT GetElementSize(GRenderer::BitmapDesc* /*Format*/)
{
    return sizeof(GRenderer::BitmapDesc);
}

UINT GetImageBytesPerPixel(GImageBase::ImageFormat Format)
{
    switch (Format)
    {
    case GImageBase::Image_ARGB_8888: return 4;
    case GImageBase::Image_RGB_888:   return 3;
    case GImageBase::Image_L_8:
    case GImageBase::Image_A_8:       return 1;
    default:                          return 1;
    }
}

UBOOL IsImageDataCompressed(GImageBase::ImageFormat Format)
{
    return Format >= GImageBase::Image_DXT1 && Format <= GImageBase::Image_DXT5;
}

UINT GetImageMipLevelSize(GImageBase::ImageFormat Format, UINT Width, UINT Height)
{
    if (IsImageDataCompressed(Format))
    {
        const UINT BlockBytes = (Format == GImageBase::Image_DXT1) ? 8 : 16;
        return ((Width + 3) / 4) * ((Height + 3) / 4) * BlockBytes;
    }
    return Width * Height * GetImageBytesPerPixel(Format);
}

const BYTE* GetImageMipLevelData(const GImageBase& Image, UINT Level, UINT& OutWidth,
                                 UINT& OutHeight, UINT& OutPitch)
{
    UINT Width = Image.Width;
    UINT Height = Image.Height;
    UINT Offset = 0;
    for (UINT i = 0; i < Level; ++i)
    {
        Offset += GetImageMipLevelSize(Image.Format, Width, Height);
        Width = Max<UINT>(1, Width >> 1);
        Height = Max<UINT>(1, Height >> 1);
    }
    if (Image.pData == NULL || (Image.DataSize != 0 && Offset >= Image.DataSize))
    {
        return NULL;
    }
    OutWidth = Width;
    OutHeight = Height;
    OutPitch = IsImageDataCompressed(Image.Format)
        ? GetImageMipLevelSize(Image.Format, Width, 4) / 4
        : Width * GetImageBytesPerPixel(Image.Format);
    return Image.pData + Offset;
}

// 2012 0x5b7710 -> 2013 0x572c30. The engine texture is resolved from the GFx texture here, which
// is the one thing the retail body does that its name does not say.
void ConvertFromUI(const GRenderer::FillTexture& In, FGFxRenderer::FFillTextureInfo& Out)
{
    Out.Texture = NULL;
    if (In.pTexture)
    {
        const FGFxTexture* GFxTexture = (const FGFxTexture*)In.pTexture;
        if (GFxTexture->Texture && GFxTexture->Texture->Resource)
        {
            Out.Texture = GFxTexture->Texture->Resource;
            Out.bUseMips = (GFxTexture->Texture->LODGroup != TEXTUREGROUP_UI) ? 1 : 0;
        }
    }
    Out.TextureMatrix = In.TextureMatrix;
    Out.WrapMode = In.WrapMode;
    Out.SampleMode = In.SampleMode;
    GFXUI_SEAM_TRACE("FGFxRendererImpl::ConvertFromUI");
}

// 2012 0x5badc0.
void ConvertFromUI(GColor InUIColor, FLinearColor& OutColor)
{
    OutColor.R = InUIColor.Channels.Red ? InUIColor.Channels.Red / 255.f : 0.f;
    OutColor.G = InUIColor.Channels.Green ? InUIColor.Channels.Green / 255.f : 0.f;
    OutColor.B = InUIColor.Channels.Blue ? InUIColor.Channels.Blue / 255.f : 0.f;
    OutColor.A = InUIColor.Channels.Alpha ? InUIColor.Channels.Alpha / 255.f : 0.f;
}

// 2012 0x5bae50. Multiply and Darken need the colour pushed towards white by the inverse of the
// alpha, because those two blend modes take the source colour unmodulated; then the whole thing is
// raised to the inverse gamma, which is the renderer's, not the shader's.
void ApplyUIColor_RenderThread(FGFxRenderer* Renderer, GColor Color,
                               GRenderer::BlendType BlendMode,
                               FGFxPixelShaderInterface& PixelShader)
{
    FLinearColor NativeColor;
    ConvertFromUI(Color, NativeColor);
    if (BlendMode == GRenderer::Blend_Multiply || BlendMode == GRenderer::Blend_Darken)
    {
        NativeColor.R = (NativeColor.R - 1.f) * NativeColor.A + 1.f;
        NativeColor.G = (NativeColor.G - 1.f) * NativeColor.A + 1.f;
        NativeColor.B = (NativeColor.B - 1.f) * NativeColor.A + 1.f;
    }
    const FLOAT InverseGamma = Renderer->InverseGamma;
    const FLinearColor GammaColor(appPow(NativeColor.R, InverseGamma),
                                  appPow(NativeColor.G, InverseGamma),
                                  appPow(NativeColor.B, InverseGamma),
                                  NativeColor.A);
    PixelShader.SetParameterConstantColor(PixelShader.GetNativeShader()->GetPixelShader(),
                                          GammaColor);
}

// 2012 0x5cc020. Three tables, transcribed term for term: the source-alpha-composited one, the
// destination-alpha-composited one, and the plain one.
void ApplyUIBlendMode_RenderThread(UINT bAlphaComposite, GRenderer::BlendType NewMode,
                                   UINT bSourceAc)
{
    if (bSourceAc)
    {
        switch (NewMode)
        {
        case GRenderer::Blend_Multiply:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_DestAlpha,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Lighten:
            RHISetBlendState(TStaticBlendState<BO_Max,BF_One,BF_One,BO_Max,BF_One,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Darken:
            RHISetBlendState(TStaticBlendState<BO_Min,BF_One,BF_One,BO_Min,BF_One,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Add:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Subtract:
            RHISetBlendState(TStaticBlendState<BO_ReverseSubtract,BF_One,BF_One,BO_ReverseSubtract,BF_Zero,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Alpha:
        case GRenderer::Blend_Erase:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_Zero,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_None:
        case GRenderer::Blend_Normal:
        case GRenderer::Blend_Layer:
        case GRenderer::Blend_Screen:
        case GRenderer::Blend_Difference:
        case GRenderer::Blend_Invert:
        case GRenderer::Blend_Overlay:
        case GRenderer::Blend_HardLight:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_InverseSourceAlpha,BO_Add,BF_One,BF_InverseSourceAlpha>::GetRHI());
            break;
        default:
            return;
        }
    }
    else if (bAlphaComposite)
    {
        switch (NewMode)
        {
        case GRenderer::Blend_Multiply:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_DestAlpha,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Lighten:
            RHISetBlendState(TStaticBlendState<BO_Max,BF_SourceAlpha,BF_One,BO_Max,BF_One,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Darken:
            RHISetBlendState(TStaticBlendState<BO_Min,BF_SourceAlpha,BF_One,BO_Min,BF_One,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Add:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Subtract:
            RHISetBlendState(TStaticBlendState<BO_ReverseSubtract,BF_SourceAlpha,BF_One,BO_ReverseSubtract,BF_Zero,BF_One>::GetRHI());
            break;
        case GRenderer::Blend_Alpha:
        case GRenderer::Blend_Erase:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_Zero,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_None:
        case GRenderer::Blend_Normal:
        case GRenderer::Blend_Layer:
        case GRenderer::Blend_Screen:
        case GRenderer::Blend_Difference:
        case GRenderer::Blend_Invert:
        case GRenderer::Blend_Overlay:
        case GRenderer::Blend_HardLight:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_One,BF_InverseSourceAlpha>::GetRHI());
            break;
        default:
            return;
        }
    }
    else
    {
        switch (NewMode)
        {
        case GRenderer::Blend_Multiply:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Lighten:
            RHISetBlendState(TStaticBlendState<BO_Max,BF_SourceAlpha,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Darken:
            RHISetBlendState(TStaticBlendState<BO_Min,BF_SourceAlpha,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Add:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Subtract:
            RHISetBlendState(TStaticBlendState<BO_ReverseSubtract,BF_SourceAlpha,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_Alpha:
        case GRenderer::Blend_Erase:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_Zero,BF_One,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        case GRenderer::Blend_None:
        case GRenderer::Blend_Normal:
        case GRenderer::Blend_Layer:
        case GRenderer::Blend_Screen:
        case GRenderer::Blend_Difference:
        case GRenderer::Blend_Invert:
        case GRenderer::Blend_Overlay:
        case GRenderer::Blend_HardLight:
            RHISetBlendState(TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_One,BF_Zero>::GetRHI());
            break;
        default:
            return;
        }
    }
}

// 2012 0x5babe0. Vertex element types 9 / 2 / 7 are VET_Short2 / VET_Float2 / VET_Color and usages
// 0 / 1 / 7 are VEU_Position / VEU_TextureCoordinate / VEU_Color, which is what the retail
// initialisers hold.
FVertexDeclarationRHIRef GetUIVertexDecl_RenderThread(EGFxVertexDeclarationType DeclType,
                                                      DWORD* OutStrides)
{
    FVertexDeclarationElementList Elements;
    switch (DeclType)
    {
    case GFx_VD_Strip:
        Elements.AddItem(FVertexElement(0,0,VET_Short2,VEU_Position,0));
        *OutStrides = 4;
        break;
    case GFx_VD_Glyph:
        Elements.AddItem(FVertexElement(0,0,VET_Float2,VEU_Position,0));
        Elements.AddItem(FVertexElement(0,8,VET_Float2,VEU_TextureCoordinate,0));
        Elements.AddItem(FVertexElement(0,16,VET_Color,VEU_Color,0));
        *OutStrides = 20;
        break;
    case GFx_VD_XY16iC32:
        Elements.AddItem(FVertexElement(0,0,VET_Short2,VEU_Position,0));
        Elements.AddItem(FVertexElement(0,4,VET_Color,VEU_Color,0));
        *OutStrides = 8;
        break;
    case GFx_VD_XY16iCF32:
        Elements.AddItem(FVertexElement(0,0,VET_Short2,VEU_Position,0));
        Elements.AddItem(FVertexElement(0,4,VET_Color,VEU_Color,0));
        Elements.AddItem(FVertexElement(0,8,VET_Color,VEU_Color,1));
        *OutStrides = 12;
        break;
    default:
        break;
    }
    return RHICreateVertexDeclaration(Elements);
}

// 2012 0x5daef0. The cache key is the three kinds packed into one DWORD, which is retail's
// PixelShaderType | VertexShaderType << 8 | VertexDeclarationType << 16.
void GetUIBoundShaderState_RenderThread(FGFxBoundShaderState& OutBoundShaderState,
                                        TMap<DWORD,FBoundShaderStateRHIRef>& Cache,
                                        const FGFxEnumeratedBoundShaderState& Enumerated)
{
    OutBoundShaderState.PixelShaderInterface =
        GetUIPixelShaderInterface_RenderThread(Enumerated.PixelShaderType);
    OutBoundShaderState.VertexShaderInterface =
        GetUIVertexShaderInterface_RenderThread(Enumerated.VertexShaderType);
    if (OutBoundShaderState.PixelShaderInterface == NULL ||
        OutBoundShaderState.VertexShaderInterface == NULL)
    {
        OutBoundShaderState.NativeBoundShaderState.SafeRelease();
        return;
    }

    const DWORD Key = (DWORD)Enumerated.PixelShaderType
                    | ((DWORD)Enumerated.VertexShaderType << 8)
                    | ((DWORD)Enumerated.VertexDeclarationType << 16);
    FBoundShaderStateRHIRef* Found = Cache.Find(Key);
    if (Found != NULL)
    {
        OutBoundShaderState.NativeBoundShaderState = *Found;
        return;
    }

    DWORD Strides[MaxVertexElementCount];
    appMemzero(Strides, sizeof(Strides));
    FVertexDeclarationRHIRef VertexDeclaration =
        GetUIVertexDecl_RenderThread(Enumerated.VertexDeclarationType, Strides);
    OutBoundShaderState.NativeBoundShaderState = RHICreateBoundShaderState(
        VertexDeclaration, Strides,
        OutBoundShaderState.VertexShaderInterface->GetNativeShader()->GetVertexShader(),
        OutBoundShaderState.PixelShaderInterface->GetNativeShader()->GetPixelShader(),
        EGST_None);
    Cache.Set(Key, OutBoundShaderState.NativeBoundShaderState);
    ++GGFxDrawCensus.BoundShaderStates;
}

// 2012 0x5db0d0. BeginDisplay's background: a four-vertex XY16i triangle strip through the solid
// colour shader, drawn with the identity transform the caller has just set.
void DrawUIBackgroundColor_RenderThread(FGFxRenderer* Renderer, GColor BackgroundColor,
                                        const TArray<FGFxVertex_XY16i>& BackgroundQuad,
                                        const GMatrix2D& ViewportMatrix,
                                        const GMatrix2D& CurrentMatrix,
                                        TMap<DWORD,FBoundShaderStateRHIRef>& Cache)
{
    FGFxEnumeratedBoundShaderState Enumerated;
    Enumerated.PixelShaderType = GFx_PS_SolidColor;
    Enumerated.VertexShaderType = GFx_VS_Strip;
    Enumerated.VertexDeclarationType = GFx_VD_Strip;

    FGFxBoundShaderState BoundShaderState;
    GetUIBoundShaderState_RenderThread(BoundShaderState, Cache, Enumerated);
    if (!IsValidRef(BoundShaderState.NativeBoundShaderState) || BackgroundQuad.Num() < 4)
    {
        return;
    }

    FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                          Renderer->CurRenderTarget->Resource->InverseGamma);
    ApplyUIColor_RenderThread(Renderer, BackgroundColor, GRenderer::Blend_None, *PixelShader);
    Renderer->ApplyUITransform_RenderThread(ViewportMatrix, CurrentMatrix,
                                            *BoundShaderState.VertexShaderInterface);
    RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);
    RHIDrawPrimitiveUP(PT_TriangleStrip, 2, &BackgroundQuad(0), sizeof(FGFxVertex_XY16i));
    ++GGFxDrawCensus.Draws;
    ++GGFxDrawCensus.BackgroundDraws;
    GGFxDrawCensus.Triangles += 2;
}

// 2012 0x5b8060: the untiled copy the texture loader uses. DestBpp 4 means an ARGB destination fed
// either from ARGB (SrcBpp 4) or RGB (SrcBpp 3); DestBpp 1 is the A8 path.
void LoadTexture(UINT DestPitch, INT DestBpp, BYTE* Dest, const BYTE* Src, UINT Width,
                 UINT Height, UINT SrcPitch)
{
    if (Dest == NULL || Src == NULL)
    {
        return;
    }
    if (DestBpp == 1)
    {
        for (UINT y = 0; y < Height; ++y)
        {
            appMemcpy(Dest + y * DestPitch, Src + y * SrcPitch, Width);
        }
        return;
    }
    const UINT SrcBpp = (SrcPitch >= Width * 4) ? 4 : 3;
    for (UINT y = 0; y < Height; ++y)
    {
        const BYTE* SrcRow = Src + y * SrcPitch;
        BYTE* DestRow = Dest + y * DestPitch;
        for (UINT x = 0; x < Width; ++x)
        {
            // GFx image data is RGBA in memory order; the engine's PF_A8R8G8B8 is BGRA.
            DestRow[x * 4 + 0] = SrcRow[x * SrcBpp + 2];
            DestRow[x * 4 + 1] = SrcRow[x * SrcBpp + 1];
            DestRow[x * 4 + 2] = SrcRow[x * SrcBpp + 0];
            DestRow[x * 4 + 3] = (SrcBpp == 4) ? SrcRow[x * SrcBpp + 3] : 255;
        }
    }
}

// 2012 0x5b7f40: a point resample into a fresh GFx-heap buffer, used when the image's size is not
// the power-of-two size the texture was created at.
BYTE* SoftwareResample(UINT DestWidth, GImageBase::ImageFormat Format, UINT SrcWidth,
                       UINT SrcHeight, UINT SrcPitch, const BYTE* Src, UINT DestHeight)
{
    const UINT Bpp = (Format == GImageBase::Image_A_8) ? 1 : 4;
    const UINT SrcBpp = (Format == GImageBase::Image_RGB_888) ? 3 : Bpp;
    BYTE* Out = (BYTE*)appMalloc(DestWidth * DestHeight * Bpp);
    if (Out == NULL || Src == NULL || SrcWidth == 0 || SrcHeight == 0)
    {
        return Out;
    }
    for (UINT y = 0; y < DestHeight; ++y)
    {
        const UINT sy = Min<UINT>(y * SrcHeight / DestHeight, SrcHeight - 1);
        const BYTE* SrcRow = Src + sy * SrcPitch;
        BYTE* DestRow = Out + y * DestWidth * Bpp;
        for (UINT x = 0; x < DestWidth; ++x)
        {
            const UINT sx = Min<UINT>(x * SrcWidth / DestWidth, SrcWidth - 1);
            for (UINT c = 0; c < Bpp; ++c)
            {
                DestRow[x * Bpp + c] = (c < SrcBpp) ? SrcRow[sx * SrcBpp + c] : 255;
            }
        }
    }
    return Out;
}

// 2012 0x5c4510 / 0x5c4630.
void FGFxRenderElementStoreBase::Release_MainThread(GLock* ElementsLock)
{
    if (RefCount.Value > 1)
    {
        // The render thread still holds this store and its elements point into the caller's
        // buffer, which is about to go away: take a copy first.
        GLock::Locker Guard(ElementsLock);
        if (!AllocatedElements)
        {
            void* NewElements = appMalloc(ElementSize * NumElements);
            appMemcpy(NewElements, Elements, ElementSize * NumElements);
            Elements = NewElements;
            AllocatedElements = true;
        }
    }
    if (pNext)
    {
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
        pPrev = NULL;
        pNext = NULL;
    }
    if (--RefCount == 0)
    {
        {
            GLock::Locker Guard(ElementsLock);
            if (AllocatedElements)
            {
                appFree(Elements);
                AllocatedElements = false;
                Elements = NULL;
            }
        }
        delete this;
    }
}

void FGFxRenderElementStoreBase::Release_RenderThread(GLock* ElementsLock)
{
    if (--RefCount == 0)
    {
        {
            GLock::Locker Guard(ElementsLock);
            if (AllocatedElements)
            {
                appFree(Elements);
                AllocatedElements = false;
                Elements = NULL;
            }
        }
        delete this;
    }
}

// 2012 0x5d47f0 / 0x5d4880 / 0x5d4540: a cached store points at the caller's buffer and links
// itself into the renderer's list, so ReleaseResources can disown it.
template<class FormatType>
void FGFxRenderElementStore<FormatType>::InitElements(FGFxRenderer* Renderer,
                                                      GRenderer::CachedData* InCachedData,
                                                      FormatType InFormat, UINT InNumElements,
                                                      void* InElements)
{
    Format = InFormat;
    ElementSize = GetElementSize(InFormat);
    pPrev = &Renderer->ElementStoreList;
    pNext = Renderer->ElementStoreList.pNext;
    Renderer->ElementStoreList.pNext->pPrev = this;
    Renderer->ElementStoreList.pNext = this;
    ++RefCount;
    CachedData = InCachedData;
    NumElements = InNumElements;
    AllocatedElements = false;
    Elements = InElements;
}

// 2012 0x5b8db0 / 0x5b8e40 / 0x5b82e0: an uncached store owns a copy.
template<class FormatType>
void FGFxRenderElementStore<FormatType>::InitElementsCopy(FormatType InFormat, UINT InNumElements,
                                                          const void* InElements)
{
    Format = InFormat;
    ElementSize = GetElementSize(InFormat);
    pPrev = NULL;
    pNext = NULL;
    NumElements = InNumElements;
    AllocatedElements = true;
    Elements = appMalloc(NumElements * ElementSize);
    appMemcpy(Elements, InElements, NumElements * ElementSize);
}

// 2012 0x5d8a10 / 0x5d8d60.
template<class StoreType, class FormatType>
void SetUIRenderElementStore(FGFxRenderer* Renderer, GRenderer::CachedDataType BuffType,
                             const void* Elements, INT NumElements, FormatType Format,
                             GRenderer::CacheProvider* const Cache, StoreType*& OwnedStore)
{
    StoreType* NewStore = NULL;
    if (Elements != NULL && NumElements > 0)
    {
        if (Cache != NULL && Cache->pData != NULL)
        {
            GRenderer::CachedData* pData = Cache->pData;
            if (pData->pRenderer == Renderer && pData->hData != NULL)
            {
                NewStore = (StoreType*)pData->hData;
                ++NewStore->RefCount;
            }
            else
            {
                if (pData->pRenderer != Renderer)
                {
                    if (pData->pRenderer)
                    {
                        pData->pRenderer->ReleaseCachedData(pData, BuffType);
                    }
                    pData->pRenderer = Renderer;
                    pData->hData = NULL;
                }
                Cache->DiscardSharedData = false;
                NewStore = new StoreType;
                NewStore->InitElements(Renderer, pData, Format, NumElements, (void*)Elements);
                pData->hData = NewStore;
            }
        }
        else
        {
            NewStore = new StoreType;
            NewStore->InitElementsCopy(Format, NumElements, Elements);
        }
    }

    // Both branches below are retail's two commands: transfer the new store, or destroy the old one.
    struct FTransferNewUIRenderElementStoreCommand : public FRenderCommand
    {
        GLock*      Lock;
        StoreType*  NewStore;
        StoreType** OwnedStorePtr;
        FTransferNewUIRenderElementStoreCommand(GLock* InLock, StoreType* InNewStore,
                                                StoreType** InOwnedStorePtr)
            : Lock(InLock), NewStore(InNewStore), OwnedStorePtr(InOwnedStorePtr) {}
        virtual UINT Execute()
        {
            if (*OwnedStorePtr)
            {
                (*OwnedStorePtr)->Release_RenderThread(Lock);
                *OwnedStorePtr = NULL;
            }
            *OwnedStorePtr = NewStore;
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand()
        {
            return TEXT("FTransferNewUIRenderElementStoreCommand");
        }
    };
    struct FDestroyUIElementStoreCommand : public FRenderCommand
    {
        GLock*      Lock;
        StoreType** OwnedStorePtr;
        FDestroyUIElementStoreCommand(GLock* InLock, StoreType** InOwnedStorePtr)
            : Lock(InLock), OwnedStorePtr(InOwnedStorePtr) {}
        virtual UINT Execute()
        {
            if (*OwnedStorePtr)
            {
                (*OwnedStorePtr)->Release_RenderThread(Lock);
                *OwnedStorePtr = NULL;
            }
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FDestroyUIElementStoreCommand"); }
    };

    if (NewStore != NULL)
    {
        ENQUEUE_RENDER_COMMAND(FTransferNewUIRenderElementStoreCommand,
                               (&Renderer->ElementAccessLock,NewStore,&OwnedStore));
    }
    else
    {
        ENQUEUE_RENDER_COMMAND(FDestroyUIElementStoreCommand,
                               (&Renderer->ElementAccessLock,&OwnedStore));
    }
}

// The two instantiations retail has, so the definitions above are emitted here rather than in
// every caller.
template void SetUIRenderElementStore<FGFxVertexStore,GRenderer::VertexFormat>(
    FGFxRenderer*, GRenderer::CachedDataType, const void*, INT, GRenderer::VertexFormat,
    GRenderer::CacheProvider* const, FGFxVertexStore*&);
template void SetUIRenderElementStore<FGFxIndexStore,GRenderer::IndexFormat>(
    FGFxRenderer*, GRenderer::CachedDataType, const void*, INT, GRenderer::IndexFormat,
    GRenderer::CacheProvider* const, FGFxIndexStore*&);

} // namespace FGFxRendererImpl

using namespace FGFxRendererImpl;

// DISHONORED(bringup): a GFx render target or stencil buffer is created and destroyed on whichever
// thread happens to drop the last reference - the render thread for a temp target released by
// ReleaseTempRenderTargets_RenderThread, the game thread for the renderer's own teardown - and
// FRenderResource's entry points are each assertive about which thread they are on
// (RenderResource.cpp: InitResource/ReleaseResource want the rendering thread, BeginReleaseResource's
// enqueue wants the game thread). This picks the right one.
static void FGFxReleaseResourceAnyThread(FRenderResource* Resource)
{
    if (Resource == NULL)
    {
        return;
    }
    if (IsInRenderingThread())
    {
        Resource->ReleaseResource();
    }
    else
    {
        BeginReleaseResource(Resource);
        FlushRenderingCommands();
    }
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderResources - the shared depth/stencil surface. 2012 0x5c4430 / 0x5c44f0 / 0x5bab20.
// ---------------------------------------------------------------------------------------------
FGFxRenderResources::~FGFxRenderResources()
{
    FGFxReleaseResourceAnyThread(this);
}

void FGFxRenderResources::InitDynamicRHI()
{
    DepthSurface = RHICreateTargetableSurface(SizeX, SizeY, PF_DepthStencil, FTexture2DRHIRef(),
                                              TargetSurfCreate_Dedicated, TEXT("GFxDepth"));
    GFXUI_SEAM_TRACE("FGFxRenderResources::InitDynamicRHI");
}

void FGFxRenderResources::ReleaseDynamicRHI()
{
    DepthSurface.SafeRelease();
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderTargetResource. 2012 0x5c4f50 / 0x5c5110 / 0x5bc0e0.
// ---------------------------------------------------------------------------------------------
FGFxRenderTargetResource::~FGFxRenderTargetResource()
{
    FGFxReleaseResourceAnyThread(this);
}

void FGFxRenderTargetResource::InitDynamicRHI()
{
    if (Owner != NULL)
    {
        // The engine owns the surface: this is the viewport (or scene target) the UI draws over.
        ColorBuffer = Owner->GetRenderTargetSurface();
    }
    else
    {
        ColorTexture = RHICreateTexture2D(SizeX, SizeY, PF_A8R8G8B8, 1,
                                         TexCreate_ResolveTargetable, NULL);
        ColorBuffer = RHICreateTargetableSurface(SizeX, SizeY, PF_A8R8G8B8, ColorTexture,
                                                 TargetSurfCreate_None, TEXT("GFxTempColor"));
        StatSize = 4 * SizeX * SizeY;
    }
    // DISHONORED(bringup): retail takes the depth surface from the scene's own depth proxy here
    // (OwnerDepth->GetDepthTargetSurface()). FSceneDepthTargetProxy lives in Engine/Src, which is
    // not on this module's include path, and nothing in this tree hands the seam an OwnerDepth yet -
    // the UI pass that would is the engine-side call site this package does not add. When it lands,
    // this is the one line to fill in.
    if (OwnerDepth != NULL)
    {
        debugf(NAME_Warning, TEXT("DISHONORED(bringup): FGFxRenderTargetResource: OwnerDepth set ")
               TEXT("but the scene depth proxy is not reachable from GFxUI; no depth surface bound"));
    }
    GFXUI_SEAM_TRACE("FGFxRenderTargetResource::InitDynamicRHI");
}

void FGFxRenderTargetResource::ReleaseDynamicRHI()
{
    ColorTexture.SafeRelease();
    ColorBuffer.SafeRelease();
    DepthBuffer.SafeRelease();
    StatSize = 0;
}

// ---------------------------------------------------------------------------------------------
// FGFxUpdatableTexture and the two UTexture2D subclasses that produce it.
// 2012 0x5d5cf0 / 0x5d5e60 / 0x5d5f50 / 0x5d5cb0 / 0x5d5b90.
// ---------------------------------------------------------------------------------------------
void FGFxUpdatableTexture::InitRHI()
{
    Texture2DRHI = RHICreateTexture2D(Width, Height, TexFormat, Levels, Flags, NULL);
    TextureRHI = Texture2DRHI;

    FSamplerStateInitializerRHI SamplerStateInitializer(Levels > 1 ? SF_Trilinear : SF_Bilinear,
                                                        AM_Clamp, AM_Clamp, AM_Clamp);
    SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);
    GFXUI_SEAM_TRACE("FGFxUpdatableTexture::InitRHI");
}

void FGFxUpdatableTexture::ReleaseRHI()
{
    TextureRHI.SafeRelease();
    SamplerStateRHI.SafeRelease();
    Texture2DRHI.SafeRelease();
}

FTextureResource* UGFxUpdatableTexture::CreateResource()
{
    return new FGFxUpdatableTexture(SizeX, SizeY, RequestedMips, (EPixelFormat)Format,
                                    TexCreate_Dynamic);
}

FTextureResource* UGFxMappableTexture::CreateResource()
{
    // 0x50 = TexCreate_Dynamic | TexCreate_NoTiling: the mappable texture is locked directly.
    return new FGFxUpdatableTexture(SizeX, SizeY, RequestedMips, (EPixelFormat)Format,
                                    TexCreate_Dynamic | TexCreate_NoTiling);
}

UINT UGFxMappableTexture::UpdateStreamingStatus(UBOOL /*bWaitForMipFading*/)
{
    // 2012 0x5d5b90: a mappable texture is always fully resident, so the streamer never touches it.
    ResidentMips = RequestedMips;
    return 0;
}

// ---------------------------------------------------------------------------------------------
// FGFxTexture
// ---------------------------------------------------------------------------------------------
FGFxTexture::FGFxTexture(GRenderer* InRenderer)
    : Renderer(InRenderer), Texture(NULL), Texture2D(NULL), RenderTarget(NULL)
{
    // GTexture's RefCount is a GAtomicInt, which default-constructs to 0, and retail's CreateTexture
    // (2012 0x5c47d0) sets it to 1 right after the vtable store. Without that the first GPtr that
    // takes and drops a reference frees the object under its creator, the heap hands the block
    // straight back out, and the next virtual call on it jumps to whatever overwrote the vptr -
    // measured, with the vptr reading as a heap address and the members as UTF-16 text.
    RefCount.Value = 1;
    GFXUI_SEAM_TRACE("FGFxTexture::FGFxTexture");
}

FGFxTexture::~FGFxTexture()
{
    InternalTermGCState();
    GFXUI_SEAM_TRACE("FGFxTexture::~FGFxTexture");
}

// DISHONORED(port): retail roots the engine texture through UGFxEngine::AddGCReferenceFor
// (2012 0x5c0000) on the GGFxGCManager singleton, which gfxuiengine.cpp owns and this build does not
// compile; AddToRoot has the same effect for the renderer's purposes and the stat accounting is
// retail's. Drop this pair when the engine object lands.
static void FGFxAddGCReferenceFor(UObject* Object)
{
    if (Object != NULL && !Object->HasAnyFlags(RF_RootSet))
    {
        Object->AddToRoot();
    }
}

static void FGFxRemoveGCReferenceFor(UObject* Object)
{
    if (Object != NULL && Object->HasAnyFlags(RF_RootSet))
    {
        Object->RemoveFromRoot();
    }
}

// 2012 0x5d8300 -> 2013 0x598780. A GFx image becomes a transient UTexture2D: the size is rounded
// up to a power of two unless the data is DXT (which is already block-sized), the mip chain is
// copied in through the bulk data, and a mismatch is point-resampled.
bool FGFxTexture::InitTexture(GImageBase* Image, unsigned int /*Usage*/)
{
    InternalTermGCState();
    if (Image == NULL)
    {
        return true;
    }

    const GImageBase::ImageFormat Format = Image->Format;
    UINT BytesPerPixel = 0;
    UINT DestBpp = 1;
    EPixelFormat TexFormat = PF_A8R8G8B8;
    switch (Format)
    {
    case GImageBase::Image_ARGB_8888: BytesPerPixel = 4; DestBpp = 4; TexFormat = PF_A8R8G8B8; break;
    case GImageBase::Image_RGB_888:   BytesPerPixel = 3; DestBpp = 4; TexFormat = PF_A8R8G8B8; break;
    case GImageBase::Image_A_8:       BytesPerPixel = 1; TexFormat = PF_G8;   break;
    case GImageBase::Image_DXT1:      BytesPerPixel = 1; TexFormat = PF_DXT1; break;
    case GImageBase::Image_DXT3:      BytesPerPixel = 1; TexFormat = PF_DXT3; break;
    case GImageBase::Image_DXT5:      BytesPerPixel = 1; TexFormat = PF_DXT5; break;
    default: break;
    }

    UINT Width = 1, Height = 1;
    if (Format >= GImageBase::Image_DXT1 && Format <= GImageBase::Image_DXT5)
    {
        Width = Image->Width;
        Height = Image->Height;
    }
    else
    {
        for (Width = 1; Width < (UINT)Image->Width; Width *= 2) {}
        for (Height = 1; Height < (UINT)Image->Height; Height *= 2) {}
    }

    Texture2D = ConstructObject<UTexture2D>(UTexture2D::StaticClass(),
                                            UObject::GetTransientPackage(), NAME_None,
                                            RF_Transient);
    Texture = Texture2D;
    FGFxAddGCReferenceFor(Texture);
    Texture2D->NeverStream = TRUE;
    Texture2D->SRGB = FALSE;
    Texture2D->Init(Width, Height, TexFormat);

    if (Width == (UINT)Image->Width && Height == (UINT)Image->Height)
    {
        if (Image->MipMapCount > 1 ||
            (Format >= GImageBase::Image_DXT1 && Format <= GImageBase::Image_DXT5))
        {
            for (UINT Level = 0; Level < (UINT)Image->MipMapCount; ++Level)
            {
                UINT MipW = 0, MipH = 0, MipPitch = 0;
                const BYTE* Data = FGFxRendererImpl::GetImageMipLevelData(*Image, Level, MipW, MipH, MipPitch);
                if (Data == NULL)
                {
                    break;
                }
                if (FGFxRendererImpl::IsImageDataCompressed(Image->Format))
                {
                    if (Level < (UINT)Texture2D->Mips.Num())
                    {
                        FTexture2DMipMap& Mip = Texture2D->Mips(Level);
                        BYTE* Dest = (BYTE*)Mip.Data.Lock(LOCK_READ_WRITE);
                        appMemcpy(Dest, Data, FGFxRendererImpl::GetImageMipLevelSize(Format, MipW, MipH));
                        Mip.Data.Unlock();
                    }
                }
                else
                {
                    LoadMipLevel(Level, DestBpp * Width, Data, MipW, MipH, BytesPerPixel, MipPitch);
                }
            }
        }
        else
        {
            LoadMipLevel(0, DestBpp * Width, Image->pData, Width, Height, BytesPerPixel,
                         Image->Pitch);
        }
    }
    else
    {
        BYTE* Resampled = FGFxRendererImpl::SoftwareResample(Width, Format, Image->Width,
                                                             Image->Height, Image->Pitch,
                                                             Image->pData, Height);
        LoadMipLevel(0, DestBpp * Width, Resampled, Width, Height, BytesPerPixel,
                     BytesPerPixel * Width);
        appFree(Resampled);
    }

    Texture2D->UpdateResource();
    GFXUI_SEAM_TRACE("FGFxTexture::InitTexture(GImageBase*)");
    return true;
}

// 2012 0x5d8590. GTexture::Usage_Map (0x20) asks for a mappable texture, Usage_Update (0x10) for an
// updatable one; anything else is a plain UTexture2D.
bool FGFxTexture::InitDynamicTexture(int Width, int Height, GImageBase::ImageFormat Format,
                                     int Mipmaps, unsigned int Usage)
{
    InternalTermGCState();
    const EPixelFormat TexFormat = (Format == GImageBase::Image_A_8) ? PF_G8 : PF_A8R8G8B8;
    if (Usage & GTexture::Usage_Map)
    {
        Texture2D = ConstructObject<UGFxMappableTexture>(UGFxMappableTexture::StaticClass(),
                                                         UObject::GetTransientPackage(), NAME_None,
                                                         RF_Transient);
    }
    else if (Usage & GTexture::Usage_Update)
    {
        Texture2D = ConstructObject<UGFxUpdatableTexture>(UGFxUpdatableTexture::StaticClass(),
                                                          UObject::GetTransientPackage(), NAME_None,
                                                          RF_Transient);
    }
    else
    {
        Texture2D = ConstructObject<UTexture2D>(UTexture2D::StaticClass(),
                                                UObject::GetTransientPackage(), NAME_None,
                                                RF_Transient);
    }
    Texture = Texture2D;
    RenderTarget = NULL;
    FGFxAddGCReferenceFor(Texture);

    Texture2D->NeverStream = TRUE;
    Texture2D->RequestedMips = Mipmaps + 1;
    Texture2D->SRGB = FALSE;
    Texture2D->Init(Width, Height, TexFormat);
    Texture->UpdateResource();
    GFXUI_SEAM_TRACE("FGFxTexture::InitDynamicTexture");
    return true;
}

// 2012 0x5bbcf0.
void FGFxTexture::LoadMipLevel(INT Level, UINT DestPitch, const BYTE* Src, UINT Width, UINT Height,
                               INT BytesPerPixel, UINT SrcPitch)
{
    if (Texture2D == NULL || Level < 0 || Level >= Texture2D->Mips.Num())
    {
        return;
    }
    FTexture2DMipMap& Mip = Texture2D->Mips(Level);
    BYTE* Dest = (BYTE*)Mip.Data.Lock(LOCK_READ_WRITE);
    FGFxRendererImpl::LoadTexture(DestPitch, BytesPerPixel == 1 ? 1 : 4, Dest, Src, Width, Height,
                                  SrcPitch);
    Mip.Data.Unlock();
}

// 2012 0x5d42b0 and 0x5bbd70: the main thread turns the GFx rectangles into engine ones and the
// render thread pushes them into the RHI texture, falling back to a lock and a manual copy where
// RHIUpdateTexture2D is not implemented.
void FGFxTexture::Update(int Level, int NumRects, const GTexture::UpdateRect* Rects,
                         const GImageBase* Image)
{
    GFXUI_SEAM_TRACE("FGFxTexture::Update");
    if (Texture == NULL || Rects == NULL || Image == NULL || NumRects <= 0)
    {
        return;
    }

    TArray<FUpdateTextureRegion2D> Regions;
    Regions.Empty(NumRects);
    for (INT i = 0; i < NumRects; ++i)
    {
        Regions.AddItem(FUpdateTextureRegion2D(Rects[i].dest.x, Rects[i].dest.y,
                                               Rects[i].src.Left, Rects[i].src.Top,
                                               Rects[i].src.Right - Rects[i].src.Left,
                                               Rects[i].src.Bottom - Rects[i].src.Top));
    }

    struct FGFxTextureUpdateCommand : public FRenderCommand
    {
        FGFxTexture*                   Texture;
        INT                            Level;
        TArray<FUpdateTextureRegion2D> Regions;
        const GImageBase*              Image;
        FGFxTextureUpdateCommand(FGFxTexture* InTexture, INT InLevel,
                                 const TArray<FUpdateTextureRegion2D>& InRegions,
                                 const GImageBase* InImage)
            : Texture(InTexture), Level(InLevel), Regions(InRegions), Image(InImage) {}
        virtual UINT Execute()
        {
            Texture->Update_RenderThread(Level, Regions.Num(), &Regions(0), Image);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxTextureUpdateCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxTextureUpdateCommand,(this,Level,Regions,Image));
}

void FGFxTexture::Update_RenderThread(INT Level, INT NumRects,
                                      const FUpdateTextureRegion2D* Regions,
                                      const GImageBase* Image)
{
    if (Texture == NULL || Texture->Resource == NULL || Image == NULL)
    {
        return;
    }
    FGFxUpdatableTexture* Resource = (FGFxUpdatableTexture*)Texture->Resource;
    const UINT SrcBpp = FGFxRendererImpl::GetImageBytesPerPixel(Image->Format);
    const INT DestBpp = (SrcBpp == 1) ? 1 : 4;

    if (!RHIUpdateTexture2D(Resource->Texture2DRHI, Level, NumRects, Regions, Image->Pitch, SrcBpp,
                            Image->pData))
    {
        UINT DestPitch = 0;
        BYTE* Dest = (BYTE*)RHILockTexture2D(Resource->Texture2DRHI, Level, TRUE, DestPitch, FALSE);
        for (INT i = 0; i < NumRects; ++i)
        {
            const FUpdateTextureRegion2D& Region = Regions[i];
            FGFxRendererImpl::LoadTexture(DestPitch, DestBpp,
                                          Dest + DestPitch * Region.DestY + DestBpp * Region.DestX,
                                          Image->pData + SrcBpp * Region.SrcX
                                              + Image->Pitch * Region.SrcY,
                                          Region.Width, Region.Height, Image->Pitch);
        }
        RHIUnlockTexture2D(Resource->Texture2DRHI, Level, FALSE);
    }
}

// 2012 0x5bbff0 / 0x5bc0a0: a mappable texture hands GFx the RHI lock directly.
int FGFxTexture::Map(int Level, int NumRects, GTexture::MapRect* Maps, int /*Flags*/)
{
    GFXUI_SEAM_TRACE("FGFxTexture::Map");
    if (Texture == NULL || Texture->Resource == NULL || Maps == NULL || NumRects <= 0)
    {
        return 0;
    }
    FGFxUpdatableTexture* Resource = (FGFxUpdatableTexture*)Texture->Resource;
    UINT Pitch = 0;
    BYTE* Data = (BYTE*)RHILockTexture2D(Resource->Texture2DRHI, Level, TRUE, Pitch, FALSE);
    if (Data == NULL)
    {
        return 0;
    }
    UINT MipW = Texture->Resource->GetSizeX();
    UINT MipH = Texture->Resource->GetSizeY();
    for (INT i = 0; i < Level; ++i)
    {
        MipW = Max<UINT>(1, MipW >> 1);
        MipH = Max<UINT>(1, MipH >> 1);
    }
    Maps->width = MipW;
    Maps->height = MipH;
    Maps->pData = Data;
    Maps->pitch = Pitch;
    return 1;
}

bool FGFxTexture::Unmap(int Level, int NumRects, GTexture::MapRect* /*Maps*/, int /*Flags*/)
{
    GFXUI_SEAM_TRACE("FGFxTexture::Unmap");
    if (NumRects >= 1 && Texture != NULL && Texture->Resource != NULL)
    {
        FGFxUpdatableTexture* Resource = (FGFxUpdatableTexture*)Texture->Resource;
        RHIUnlockTexture2D(Resource->Texture2DRHI, Level, FALSE);
    }
    return true;
}

GRenderer* FGFxTexture::GetRenderer() const { return Renderer; }

bool FGFxTexture::IsDataValid() const
{
    // 2012 0x5bf620.
    return Texture2D != NULL || RenderTarget != NULL;
}

void* FGFxTexture::GetUserData() const { return NULL; }
void FGFxTexture::SetUserData(void* /*Data*/) {}
void FGFxTexture::AddChangeHandler(GTexture::ChangeHandler* /*Handler*/) {}
void FGFxTexture::RemoveChangeHandler(GTexture::ChangeHandler* /*Handler*/) {}

// 2012 0x5c4f10 -> 2013 0x580810: the path a stripped cooked bitmap takes - the tag-1009 export name
// is resolved to a package Texture2D and handed here (agentBB.md 3.3).
bool FGFxTexture::InitTexture(UTexture* InTexture, bool /*bAsRenderTarget*/)
{
    InternalTermGCState();
    Texture = InTexture;
    Texture2D = Cast<UTexture2D>(InTexture);
    FGFxAddGCReferenceFor(InTexture);
    GFXUI_SEAM_TRACE("FGFxTexture::InitTexture(UTexture*)");
    return true;
}

bool FGFxTexture::InitTextureFromFile(const char* /*FileName*/)
{
    // 2012 0x5c9070: retail's is the editor path (it reads a file through the image loader); the
    // cook has no loose image files, so there is nothing for it to open.
    GFXUI_SEAM_TRACE("FGFxTexture::InitTextureFromFile");
    return false;
}

int FGFxTexture::IsYUVTexture() const { return 0; }

// 2012 0x5bbed0 -> 2013 0x5775b0.
void FGFxTexture::Bind(int Stage, FGFxPixelShaderInterface& Shader,
                       GRenderer::BitmapWrapMode WrapMode,
                       GRenderer::BitmapSampleMode SampleMode, bool /*bUseMips*/) const
{
    GFXUI_SEAM_TRACE("FGFxTexture::Bind");
    FGFxRenderer* GFxRenderer = (FGFxRenderer*)Renderer;
    if (Texture != NULL)
    {
        if (Texture->Resource == NULL)
        {
            return;
        }
        const FSamplerStateRHIRef SamplerState =
            GFxRenderer->GetSamplerState(SampleMode, WrapMode,
                                         Texture->LODGroup != TEXTUREGROUP_UI ? 1 : 0);
        Shader.SetParameterTextureRHI(Shader.GetNativeShader()->GetPixelShader(), SamplerState,
                                      Texture->Resource->TextureRHI, Stage);
    }
    else if (RenderTarget != NULL)
    {
        const FSamplerStateRHIRef SamplerState =
            GFxRenderer->GetSamplerState(SampleMode, WrapMode, 0);
        Shader.SetParameterTextureRHI(Shader.GetNativeShader()->GetPixelShader(), SamplerState,
                                      RenderTarget->Resource->ColorTexture, Stage);
    }
}

// 2012 0x5cff60.
void FGFxTexture::InternalTermGCState()
{
    if (Texture != NULL)
    {
        FGFxRemoveGCReferenceFor(Texture);
    }
    if (RenderTarget != NULL)
    {
        RenderTarget->Texture = (FGFxTexture*)NULL;
    }
    Texture2D = NULL;
    Texture = NULL;
    RenderTarget = NULL;
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderTarget
// ---------------------------------------------------------------------------------------------
FGFxRenderTarget::FGFxRenderTarget(GRenderer* InRenderer)
    : Renderer(InRenderer), Resource(new FGFxRenderTargetResource), TargetWidth(0),
      TargetHeight(0), IsTemp(false)
{
    // As FGFxTexture above, and for the same reason: retail's constructor (2012 0x5ce5b0) writes
    // RefCount.Value = 1 between its two vtable stores.
    RefCount.Value = 1;
    GFXUI_SEAM_TRACE("FGFxRenderTarget::FGFxRenderTarget");
}

FGFxRenderTarget::~FGFxRenderTarget()
{
    // 2012 0x5cffd0.
    Texture = (FGFxTexture*)NULL;
    StencilBuffer = (FGFxRenderResources*)NULL;
    if (Resource != NULL)
    {
        FGFxReleaseResourceAnyThread(Resource);
        delete Resource;
        Resource = NULL;
    }
    GFXUI_SEAM_TRACE("FGFxRenderTarget::~FGFxRenderTarget");
}

// 2012 0x5bc1b0 -> 2013 0x577890: point this render target at the texture GFx just made a target of.
bool FGFxRenderTarget::InitRenderTarget(GTexture* Color, GTexture* /*DepthStencil*/,
                                        GTexture* /*Resolve*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget(GTexture*)");
    FGFxTexture* Target = (FGFxTexture*)Color;
    if (Target == NULL || Target->RenderTarget == NULL || Target->RenderTarget->Resource == NULL)
    {
        return false;
    }
    FGFxReleaseResourceAnyThread(Resource);

    const UINT Width = Target->RenderTarget->Resource->SizeX;
    const UINT Height = Target->RenderTarget->Resource->SizeY;
    struct FGFxInitRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderTarget* RT;
        FGFxTexture*      Texture;
        UINT              Width;
        UINT              Height;
        FGFxInitRenderTargetCommand(FGFxRenderTarget* InRT, FGFxTexture* InTexture, UINT InWidth,
                                    UINT InHeight)
            : RT(InRT), Texture(InTexture), Width(InWidth), Height(InHeight) {}
        virtual UINT Execute()
        {
            RT->InitRenderTarget_RenderThread(Texture, NULL, Width, Height);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxInitRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxInitRenderTargetCommand,(this,Target,Width,Height));
    return true;
}

GRenderer* FGFxRenderTarget::GetRenderer() const { return Renderer; }
void* FGFxRenderTarget::GetUserData() const { return NULL; }
void FGFxRenderTarget::SetUserData(void* /*Data*/) {}
void FGFxRenderTarget::AddChangeHandler(GTexture::ChangeHandler* /*Handler*/) {}
void FGFxRenderTarget::RemoveChangeHandler(GTexture::ChangeHandler* /*Handler*/) {}

// 2012 0x5ce660 -> 2013 0x58d2f0: wrap one of the engine's own render targets.
bool FGFxRenderTarget::InitRenderTarget(const FGFxRenderTargetResource::NativeRenderTarget& Native)
{
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget(Native)");
    Texture = (FGFxTexture*)NULL;
    FGFxReleaseResourceAnyThread(Resource);
    Resource->Owner = Native.Owner;
    Resource->OwnerDepth = Native.OwnerDepth;
    if (Native.Owner != NULL)
    {
        Resource->SizeX = Native.Owner->GetSizeX();
        Resource->SizeY = Native.Owner->GetSizeY();
        BeginInitResource(Resource);
    }
    TargetWidth = Resource->SizeX;
    TargetHeight = Resource->SizeY;
    return true;
}

// 2012 0x5d0140: the render-thread half of the texture form.
bool FGFxRenderTarget::InitRenderTarget_RenderThread(GTexture* Color,
                                                     FGFxRenderResources* Stencil,
                                                     unsigned int Width, unsigned int Height)
{
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget_RenderThread(GTexture*)");
    Resource->ReleaseResource();
    Texture = (FGFxTexture*)Color;
    StencilBuffer = Stencil;
    Resource->Owner = NULL;
    Resource->OwnerDepth = NULL;
    Resource->SizeX = Width;
    Resource->SizeY = Height;
    TargetWidth = Width;
    TargetHeight = Height;
    Resource->InitResource();
    return true;
}

// 2012 0x5ce6f0: the render-thread half of the native form.
bool FGFxRenderTarget::InitRenderTarget_RenderThread(
    const FGFxRenderTargetResource::NativeRenderTarget& Native)
{
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget_RenderThread(Native)");
    Resource->ReleaseResource();
    Texture = (FGFxTexture*)NULL;
    Resource->Owner = Native.Owner;
    Resource->OwnerDepth = Native.OwnerDepth;
    if (Native.Owner != NULL)
    {
        Resource->SizeX = Native.Owner->GetSizeX();
        Resource->SizeY = Native.Owner->GetSizeY();
        Resource->InitResource();
    }
    TargetWidth = Resource->SizeX;
    TargetHeight = Resource->SizeY;
    return true;
}

bool FGFxRenderTarget::AdjustBounds(float* Width, float* Height)
{
    // 2012 0x5b7cd0: clamp a requested temp-target size to the target this render target holds.
    if (Width == NULL || Height == NULL) return false;
    if (TargetWidth == 0 || TargetHeight == 0) return false;
    if (*Width > (float)TargetWidth) *Width = (float)TargetWidth;
    if (*Height > (float)TargetHeight) *Height = (float)TargetHeight;
    return true;
}

// ---------------------------------------------------------------------------------------------
// The styles. 2012 0x5b77b0 / 0x5b77e0 / 0x5c46d0 / 0x5cc820 / 0x5bb150 / 0x5bb160 / 0x5bb240 /
// 0x5bb350 / 0x5bb3b0 / 0x5c4720 / 0x5bb480.
// ---------------------------------------------------------------------------------------------
FGFxRenderer::FGFxRenderStyle::FGFxRenderStyle()
    : StyleMode(GFx_SM_Disabled)
{
    Color.Raw = 0;
    FGFxCxformSetIdentity(CxColorMatrix);
}

void FGFxRenderer::FGFxRenderStyle::GetEnumeratedBoundShaderState_RenderThread(
    void* const OutState, void const* const Context)
{
    FGFxEnumeratedBoundShaderState& State = *(FGFxEnumeratedBoundShaderState*)OutState;
    const FGFxRenderStyleContext& StyleContext = *(const FGFxRenderStyleContext*)Context;
    if (StyleContext.VertexFmt == GRenderer::Vertex_XY16i)
    {
        State.VertexShaderType = GFx_VS_Strip;
        State.VertexDeclarationType = GFx_VD_Strip;
    }
    State.PixelShaderType = GFx_PS_SolidColor;
}

void FGFxRenderer::FGFxRenderStyle::Apply_RenderThread(FGFxRenderer* Renderer,
                                                       void const* const BoundState,
                                                       void* const Context)
{
    const FGFxBoundShaderState& State = *(const FGFxBoundShaderState*)BoundState;
    const FGFxRenderStyleContext& StyleContext = *(const FGFxRenderStyleContext*)Context;
    FGFxPixelShaderInterface* PixelShader = State.PixelShaderInterface;
    FGFxRendererImpl::ApplyUIColor_RenderThread(Renderer, Color, StyleContext.BlendFmt, *PixelShader);
    PixelShader->SetParametersColorScaleAndColorBias(
        PixelShader->GetNativeShader()->GetPixelShader(), CxColorMatrix);
}

void FGFxRenderer::FGFxRenderStyle::EndDisplay_RenderThread()
{
    StyleMode = GFx_SM_Disabled;
    Color.Raw = 0;
    FGFxCxformSetIdentity(CxColorMatrix);
}

void FGFxRenderer::FGFxRenderStyle::Disable()
{
    struct FDisableUIStyleCommand : public FRenderCommand
    {
        EGFxRenderStyleMode* StyleModePtr;
        FDisableUIStyleCommand(EGFxRenderStyleMode* InStyleModePtr) : StyleModePtr(InStyleModePtr) {}
        virtual UINT Execute() { *StyleModePtr = GFx_SM_Disabled; return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FDisableUIStyleCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FDisableUIStyleCommand,(&StyleMode));
}

void FGFxRenderer::FGFxRenderStyle::SetStyleColor_RenderThread(GColor InColor)
{
    Color = InColor;
    StyleMode = GFx_SM_Color;
}

// 2012 0x5b77e0: which of the 51 pixel shaders and 8 vertex shaders this fill needs. The +4 at the
// end is the Multiply/Darken variant, which is always four kinds along in the enumeration.
void FGFxRenderer::FGFxFillStyle::GetEnumeratedBoundShaderState_RenderThread(
    void* const OutState, void const* const Context)
{
    FGFxEnumeratedBoundShaderState& State = *(FGFxEnumeratedBoundShaderState*)OutState;
    const FGFxRenderStyleContext& StyleContext = *(const FGFxRenderStyleContext*)Context;
    if (StyleContext.VertexFmt == GRenderer::Vertex_XY16i)
    {
        State.VertexShaderType = GFx_VS_Strip;
        State.VertexDeclarationType = GFx_VD_Strip;
    }
    State.PixelShaderType = GFx_PS_SolidColor;

    if (StyleMode == GFx_SM_Bitmap)
    {
        State.PixelShaderType = (StyleContext.BlendFmt == GRenderer::Blend_Multiply ||
                                 StyleContext.BlendFmt == GRenderer::Blend_Darken)
                              ? GFx_PS_CxformTextureMultiply : GFx_PS_CxformTexture;
    }
    else if (StyleMode == GFx_SM_Gouraud)
    {
        if (StyleContext.VertexFmt == GRenderer::Vertex_XY16iC32)
        {
            State.VertexShaderType = GFx_VS_XY16iC32;
            State.VertexDeclarationType = GFx_VD_XY16iC32;
        }
        else if (StyleContext.VertexFmt == GRenderer::Vertex_XY16iCF32)
        {
            State.VertexDeclarationType = GFx_VD_XY16iCF32;
            if (GouraudFillMode == GRenderer::GFill_Color)
            {
                State.VertexShaderType = GFx_VS_XY16iCF32_NoTex;
            }
            else
            {
                State.VertexShaderType = (GouraudFillMode != GRenderer::GFill_2Texture)
                                       ? GFx_VS_XY16iCF32 : GFx_VS_XY16iCF32_T2;
            }
        }
        State.PixelShaderType = GFx_PS_SolidColor;
        if (TexInfo.Texture != NULL)
        {
            State.PixelShaderType = (GouraudFillMode == GRenderer::GFill_1TextureColor ||
                                     GouraudFillMode == GRenderer::GFill_1Texture)
                                  ? GFx_PS_CxformGouraudTexture : GFx_PS_Cxform2Texture;
        }
        else if (StyleContext.VertexFmt == GRenderer::Vertex_XY16iC32)
        {
            State.VertexShaderType = GFx_VS_XY16iCF32_NoTexNoAlpha;
            State.PixelShaderType = GFx_PS_CxformGouraudNoAddAlpha;
        }
        else
        {
            State.PixelShaderType = GFx_PS_CxformGouraud;
        }
        if (StyleContext.BlendFmt == GRenderer::Blend_Multiply ||
            StyleContext.BlendFmt == GRenderer::Blend_Darken)
        {
            State.PixelShaderType = (EGFxPixelShaderType)(State.PixelShaderType + 4);
        }
    }
}

// 2012 0x5cc820.
void FGFxRenderer::FGFxFillStyle::Apply_RenderThread(FGFxRenderer* Renderer,
                                                     void const* const BoundState,
                                                     void* const Context)
{
    const FGFxBoundShaderState& State = *(const FGFxBoundShaderState*)BoundState;
    const FGFxRenderStyleContext& StyleContext = *(const FGFxRenderStyleContext*)Context;
    FGFxPixelShaderInterface* PixelShader = State.PixelShaderInterface;
    FGFxRendererImpl::ApplyUIColor_RenderThread(Renderer, Color, StyleContext.BlendFmt, *PixelShader);
    PixelShader->SetParametersColorScaleAndColorBias(
        PixelShader->GetNativeShader()->GetPixelShader(), CxColorMatrix);

    if (StyleMode != GFx_SM_Color && TexInfo.Texture != NULL)
    {
        StaticApplyTextureMatrix_RenderThread(BoundState, Context, TexInfo, 0);
        StaticApplyTexture_RenderThread(Renderer, BoundState, Context, TexInfo, 0);
        if ((TexInfo2.Texture != NULL && GouraudFillMode == GRenderer::GFill_2TextureColor) ||
            GouraudFillMode == GRenderer::GFill_2Texture)
        {
            StaticApplyTextureMatrix_RenderThread(BoundState, Context, TexInfo2, 1);
            StaticApplyTexture_RenderThread(Renderer, BoundState, Context, TexInfo2, 1);
        }
    }
}

void FGFxRenderer::FGFxFillStyle::EndDisplay_RenderThread()
{
    // 2012 0x5bb160.
    StyleMode = GFx_SM_Disabled;
    Color.Raw = 0;
    FGFxCxformSetIdentity(CxColorMatrix);
    GouraudFillMode = GRenderer::GFill_Color;
    TexInfo = FFillTextureInfo();
    TexInfo2 = FFillTextureInfo();
}

void FGFxRenderer::FGFxFillStyle::SetStyleBitmap_RenderThread(const FFillTextureInfo& InTexInfo,
                                                              const GRenderer::Cxform& Cx)
{
    TexInfo = InTexInfo;
    CxColorMatrix = Cx;
    StyleMode = GFx_SM_Bitmap;
}

void FGFxRenderer::FGFxFillStyle::SetStyleGouraud_RenderThread(GRenderer::GouraudFillType Type,
                                                               const FFillTextureInfo* T0,
                                                               const FFillTextureInfo* T1,
                                                               const FFillTextureInfo* T2,
                                                               const GRenderer::Cxform& Cx)
{
    // 2012 0x5bb3b0.
    const FFillTextureInfo* First = T0 ? T0 : T1;
    if (First != NULL)
    {
        SetStyleBitmap_RenderThread(*First, Cx);
        if (T1 != NULL)
        {
            TexInfo2 = *T1;
        }
        else
        {
            TexInfo2.Texture = NULL;
        }
    }
    else if (T2 != NULL)
    {
        TexInfo2.Texture = NULL;
    }
    else
    {
        CxColorMatrix = Cx;
        TexInfo.Texture = NULL;
        TexInfo2.Texture = NULL;
    }
    GouraudFillMode = Type;
    StyleMode = GFx_SM_Gouraud;
}

void FGFxRenderer::FGFxFillStyle::StaticApplyTexture_RenderThread(FGFxRenderer* Renderer,
                                                                  void const* const BoundState,
                                                                  void const* const /*Context*/,
                                                                  const FFillTextureInfo& TexInfo,
                                                                  INT i)
{
    // 2012 0x5c4720.
    if (TexInfo.Texture == NULL)
    {
        return;
    }
    const FGFxBoundShaderState& State = *(const FGFxBoundShaderState*)BoundState;
    const FSamplerStateRHIRef SamplerState =
        Renderer->GetSamplerState(TexInfo.SampleMode, TexInfo.WrapMode, TexInfo.bUseMips);
    FGFxPixelShaderInterface* PixelShader = State.PixelShaderInterface;
    PixelShader->SetParameterTextureRHI(PixelShader->GetNativeShader()->GetPixelShader(),
                                        SamplerState, TexInfo.Texture->TextureRHI, i);
}

void FGFxRenderer::FGFxFillStyle::StaticApplyTextureMatrix_RenderThread(
    void const* const BoundState, void const* const /*Context*/, const FFillTextureInfo& TexInfo,
    INT i)
{
    // 2012 0x5bb480.
    if (TexInfo.Texture == NULL)
    {
        return;
    }
    const FGFxBoundShaderState& State = *(const FGFxBoundShaderState*)BoundState;
    const FMatrix NativeTextureMatrix = FGFxMatrix2DToNative(TexInfo.TextureMatrix);
    FGFxVertexShaderInterface* VertexShader = State.VertexShaderInterface;
    VertexShader->SetParameterTextureMatrix(VertexShader->GetNativeShader()->GetVertexShader(),
                                            NativeTextureMatrix, i);
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderer::RTState - 2012 0x5b97b0.
// ---------------------------------------------------------------------------------------------
FGFxRenderer::RTState::RTState(FGFxRenderTarget* InRT, const GMatrix2D& InView,
                               const GMatrix3D& InView3D, const GMatrix3D& InPersp3D,
                               const GMatrix3D& InWorld3D, unsigned int InIs3DEnabled,
                               const FGFxViewportUserParams& InViewRect, INT InRenderMode,
                               FStencilStateRHIParamRef InStencilState)
    : pRT(InRT), ViewMatrix(InView), ViewMatrix3D(InView3D), PerspMatrix3D(InPersp3D),
      WorldMatrix3D(InWorld3D), ViewRect(InViewRect), RenderMode(InRenderMode),
      Is3DEnabled(InIs3DEnabled), StencilState(InStencilState)
{
}

// ---------------------------------------------------------------------------------------------
// FGFxViewportAxisInfo::Clip - the arithmetic inside SetUIViewport (2012 0x5c4820): the pixel range
// is clipped to the part of the buffer the viewport actually covers.
// ---------------------------------------------------------------------------------------------
void FGFxViewportAxisInfo::Clip()
{
    PixelClipStart = PixelStart;
    PixelClipEnd = PixelEnd;
    PixelClipLength = PixelEnd - PixelStart;
    if (PixelClipLength >= 1.f)
    {
        PixelClipLength = 1.f;
    }
    const FLOAT Length = PixelClipLength;
    if (ViewStart < 0)
    {
        PixelClipStart = PixelStart + (-ViewStart * Length) / (FLOAT)ViewLength;
        ViewStart = 0;
    }
    if (ViewLength + ViewStart > ViewLengthMax)
    {
        const INT Visible = ViewLengthMax - ViewStart;
        PixelClipEnd = PixelStart + (Visible * Length) / (FLOAT)ViewLength;
        ViewLength = Visible;
    }
    PixelClipLength = PixelClipEnd - PixelClipStart;
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderer - all 54 GRenderer slots.
// ---------------------------------------------------------------------------------------------
FGFxRenderer::FGFxRenderer()
    : Viewport(NULL), RenderTarget(NULL), RenderMode(0), VertexStore(NULL), IndexStore(NULL),
      InverseGamma(0.f), UVPMatricesChanged(0), Is3DEnabled(0), CurRenderTarget(NULL),
      CurRenderTargetSet(0), BlendMode(GRenderer::Blend_None), bAlphaComposite(0),
      MaxTempRTSize(1024), StencilCounter(0)
{
    // 2012 0x5e1bf0 -> 2013 0x5a1f30.
    appMemzero(&RenderStats, sizeof(RenderStats));
    UserMatrix.SetIdentity();
    CurrentMatrix.SetIdentity();
    ViewportMatrix.SetIdentity();
    FGFxCxformSetIdentity(CurrentCxform);
    appMemzero(&ViewRect, sizeof(ViewRect));
    ViewMatrix.SetIdentity();
    ProjMatrix.SetIdentity();
    WorldMatrix.SetIdentity();
    UVPMatrix.SetIdentity();
    ElementStoreList.pPrev = &ElementStoreList;
    ElementStoreList.pNext = &ElementStoreList;
    BlendModeStack.Empty(16);
    GFXUI_SEAM_TRACE("FGFxRenderer::FGFxRenderer");
}

FGFxRenderer::~FGFxRenderer()
{
    // 2012 0x5decf0.
    ReleaseResources();
    for (INT i = 0; i < 8; ++i)
    {
        SamplerStates[i].SafeRelease();
    }
    BoundShaderStateCache.Empty();
    CurStencilState.SafeRelease();
    GFXUI_SEAM_TRACE("FGFxRenderer::~FGFxRenderer");
}

void FGFxRenderer::ScopedEventCallback(const char* /*Name*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::ScopedEventCallback");
}

void FGFxRenderer::SaveCurrentRenderTargetContents()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SaveCurrentRenderTargetContents");
}

void FGFxRenderer::RestoreCurrentRenderTargetContents()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::RestoreCurrentRenderTargetContents");
}

// 2012 0x5b78e0 -> 2013 0x572e00. The literals are retail's: 0x1004 is Cap_Index16 | Cap_CxformAdd,
// 0x1304 adds the two Gouraud fills when the RHI can feed a VET_Color vertex element, 0xC02020 adds
// render targets, nested masks and both filter families, and the blend-mode mask is 875.
bool FGFxRenderer::GetRenderCaps(GRenderer::RenderCaps* Caps)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::GetRenderCaps");
    if (Caps == NULL) return false;
    Caps->CapBits = 4100;
    Caps->BlendModes = 875;
    Caps->VertexFormats = 3;
    if (GVertexElementTypeSupport.IsSupported(VET_Color))
    {
        Caps->VertexFormats = 27;
        Caps->CapBits = 4868;
    }
    Caps->CapBits |= 0xC02020u;
    Caps->MaxTextureSize = 1 << GMaxTextureMipCount;
    return true;
}

FGFxTexture* FGFxRenderer::CreateTexture()
{
    // 2012 0x5c47d0 -> 2013 0x5800d0.
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateTexture");
    return new FGFxTexture(this);
}

FGFxTexture* FGFxRenderer::CreateTextureYUV()
{
    // DISHONORED(bringup): the YUV texture is the video path, whose four pixel-shader kinds
    // (GFx_PS_TextTextureYUV*, 46..49) have no cooked shader in this game's cache. Returning NULL is
    // what makes GFxVideo fall back, and nothing in the cook asks for one.
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateTextureYUV");
    return NULL;
}

void FGFxRenderer::BeginFrame() { GFXUI_SEAM_TRACE("FGFxRenderer::BeginFrame"); }
void FGFxRenderer::EndFrame() { GFXUI_SEAM_TRACE("FGFxRenderer::EndFrame"); }

FGFxRenderTarget* FGFxRenderer::CreateRenderTarget()
{
    // 2012 0x5cf8d0 -> 2013 0x58f120.
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateRenderTarget");
    return new FGFxRenderTarget(this);
}

// 2012 0x5d3b10 and its command's Execute at 0x5bbb60.
void FGFxRenderer::SetDisplayRenderTarget(GRenderTarget* Target, bool bSetState)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetDisplayRenderTarget");
    struct FGFxSetRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderer*     Renderer;
        FGFxRenderTarget* RT;
        UBOOL             bSetState;
        FGFxSetRenderTargetCommand(FGFxRenderer* InRenderer, FGFxRenderTarget* InRT,
                                   UBOOL InbSetState)
            : Renderer(InRenderer), RT(InRT), bSetState(InbSetState) {}
        virtual UINT Execute()
        {
            Renderer->CurRenderTarget = RT;
            if (bSetState && RT != NULL && RT->Resource != NULL)
            {
                RHISetRenderTarget(RT->Resource->ColorBuffer, RT->Resource->DepthBuffer);
            }
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxSetRenderTargetCommand,(this,(FGFxRenderTarget*)Target,bSetState));
    CurRenderTargetSet = (Target != NULL) ? 1 : 0;
}

// 2012 0x5d9270 / 0x5d94a0: both halves are the render thread's, so the slots only enqueue.
void FGFxRenderer::PushRenderTarget(const GRect<float>& FrameRect, GRenderTarget* Target)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PushRenderTarget");
    struct FGFxPushRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderer*     Renderer;
        GRect<float>      FrameRect;
        FGFxRenderTarget* RT;
        FGFxPushRenderTargetCommand(FGFxRenderer* InRenderer, const GRect<float>& InFrameRect,
                                    FGFxRenderTarget* InRT)
            : Renderer(InRenderer), FrameRect(InFrameRect), RT(InRT) {}
        virtual UINT Execute()
        {
            Renderer->PushRenderTarget_RenderThread(FrameRect, RT);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxPushRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxPushRenderTargetCommand,(this,FrameRect,(FGFxRenderTarget*)Target));
}

void FGFxRenderer::PopRenderTarget()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PopRenderTarget");
    struct FGFxPopRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxPopRenderTargetCommand(FGFxRenderer* InRenderer) : Renderer(InRenderer) {}
        virtual UINT Execute() { Renderer->PopRenderTarget_RenderThread(); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxPopRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxPopRenderTargetCommand,(this));
}

// 2012 0x5dee70: the main thread makes the stub texture the runtime will read back from, and the
// render thread fills it in and pushes it.
FGFxTexture* FGFxRenderer::PushTempRenderTarget(const GRect<float>& FrameRect, unsigned int Width,
                                                unsigned int Height, bool bWantStencil)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PushTempRenderTarget");
    FGFxTexture* StubTexture = CreateTexture();
    struct FGFxPushTempRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxTexture*  StubTexture;
        GRect<float>  FrameRect;
        UINT          Width;
        UINT          Height;
        UBOOL         bWantStencil;
        FGFxPushTempRenderTargetCommand(FGFxRenderer* InRenderer, FGFxTexture* InStubTexture,
                                        const GRect<float>& InFrameRect, UINT InWidth,
                                        UINT InHeight, UBOOL InbWantStencil)
            : Renderer(InRenderer), StubTexture(InStubTexture), FrameRect(InFrameRect),
              Width(InWidth), Height(InHeight), bWantStencil(InbWantStencil) {}
        virtual UINT Execute()
        {
            Renderer->PushTempRenderTarget_RenderThread(StubTexture, FrameRect, Width, Height,
                                                        bWantStencil != 0);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxPushTempRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxPushTempRenderTargetCommand,
                           (this,StubTexture,FrameRect,Width,Height,bWantStencil));
    return StubTexture;
}

void FGFxRenderer::ReleaseTempRenderTargets(unsigned int KeepArea)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseTempRenderTargets");
    struct FGFxReleaseTempRenderTargetCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        UINT          KeepArea;
        FGFxReleaseTempRenderTargetCommand(FGFxRenderer* InRenderer, UINT InKeepArea)
            : Renderer(InRenderer), KeepArea(InKeepArea) {}
        virtual UINT Execute()
        {
            Renderer->ReleaseTempRenderTargets_RenderThread(KeepArea);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxReleaseTempRenderTargetCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxReleaseTempRenderTargetCommand,(this,KeepArea));
}

// 2012 0x5df080 -> 2013 0x59dd90.
void FGFxRenderer::BeginDisplay(GColor BackgroundColor, const GViewport& InViewport,
                                float x0, float x1, float y0, float y1)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::BeginDisplay");

    FGFxViewportUserParams ViewportParams;
    appMemzero(&ViewportParams, sizeof(ViewportParams));
    ViewportParams.xAxis.PixelStart = x0;
    ViewportParams.xAxis.PixelEnd = x1;
    ViewportParams.xAxis.ViewStart = InViewport.Left;
    ViewportParams.xAxis.ViewLength = InViewport.Width;
    ViewportParams.xAxis.ViewLengthMax = InViewport.BufferWidth;
    ViewportParams.yAxis.PixelStart = y0;
    ViewportParams.yAxis.PixelEnd = y1;
    ViewportParams.yAxis.ViewStart = InViewport.Top;
    ViewportParams.yAxis.ViewLength = InViewport.Height;
    ViewportParams.yAxis.ViewLengthMax = InViewport.BufferHeight;
    RenderMode = (InViewport.Flags >> 1) & 1;
    SetUIViewport(ViewportParams);

    FMiscRenderStateInitParams Params;
    Params.Renderer = this;
    Params.BlendMode = &BlendMode;
    Params.BlendModeStack = &BlendModeStack;
    Params.StencilCounter = &StencilCounter;
    Params.InverseGamma = &InverseGamma;
    Params.bAlphaComposite = &bAlphaComposite;
    Params.ViewFlags = InViewport.Flags;

    struct FGFxInitUIBlendStackCommand : public FRenderCommand
    {
        FMiscRenderStateInitParams Params;
        FGFxInitUIBlendStackCommand(const FMiscRenderStateInitParams& InParams) : Params(InParams) {}
        virtual UINT Execute()
        {
            Params.Renderer->InitUIBlendStackAndMiscRenderState_RenderingThread(Params);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxInitUIBlendStackCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxInitUIBlendStackCommand,(Params));

    if (BackgroundColor.Channels.Alpha)
    {
        SetMatrix(GMatrix2D());

        TArray<FGFxVertex_XY16i> BgQuad;
        BgQuad.Add(4);
        BgQuad(0).X = (SWORD)x0; BgQuad(0).Y = (SWORD)y0;
        BgQuad(1).X = (SWORD)x1; BgQuad(1).Y = (SWORD)y0;
        BgQuad(2).X = (SWORD)x0; BgQuad(2).Y = (SWORD)y1;
        BgQuad(3).X = (SWORD)x1; BgQuad(3).Y = (SWORD)y1;

        struct FGFxDrawUIBackgroundColorCommand : public FRenderCommand
        {
            FGFxRenderer*            Renderer;
            GColor                   BackgroundColor;
            TArray<FGFxVertex_XY16i> BgQuad;
            FGFxDrawUIBackgroundColorCommand(FGFxRenderer* InRenderer, GColor InBackgroundColor,
                                             const TArray<FGFxVertex_XY16i>& InBgQuad)
                : Renderer(InRenderer), BackgroundColor(InBackgroundColor), BgQuad(InBgQuad) {}
            virtual UINT Execute()
            {
                FGFxRendererImpl::DrawUIBackgroundColor_RenderThread(
                    Renderer, BackgroundColor, BgQuad, Renderer->ViewportMatrix,
                    Renderer->CurrentMatrix, Renderer->BoundShaderStateCache);
                return sizeof(*this);
            }
            virtual const TCHAR* DescribeCommand()
            {
                return TEXT("FGFxDrawUIBackgroundColorCommand");
            }
        };
        ENQUEUE_RENDER_COMMAND(FGFxDrawUIBackgroundColorCommand,(this,BackgroundColor,BgQuad));
    }
}

// 2012 0x5c4820: the viewport matrix is the clipped pixel range mapped onto clip space, with the
// half-pixel offset the RHI needs.
void FGFxRenderer::SetUIViewport(FGFxViewportUserParams& InViewportParams)
{
    InViewportParams.xAxis.Clip();
    InViewportParams.yAxis.Clip();

    GMatrix2D NewViewportMatrix;
    NewViewportMatrix.SetIdentity();
    NewViewportMatrix.M_[0][0] = 2.f / InViewportParams.xAxis.PixelClipLength;
    NewViewportMatrix.M_[1][1] = -2.f / InViewportParams.yAxis.PixelClipLength;
    const FLOAT XOffset = InViewportParams.xAxis.ViewLength > 0
        ? (GPixelCenterOffset * 2.f) / (FLOAT)InViewportParams.xAxis.ViewLength : 0.f;
    const FLOAT YOffset = InViewportParams.yAxis.ViewLength > 0
        ? (GPixelCenterOffset * 2.f) / (FLOAT)InViewportParams.yAxis.ViewLength : 0.f;
    NewViewportMatrix.M_[0][2] = -1.f - NewViewportMatrix.M_[0][0] * InViewportParams.xAxis.PixelClipStart
                               - XOffset;
    NewViewportMatrix.M_[1][2] = 1.f - NewViewportMatrix.M_[1][1] * InViewportParams.yAxis.PixelClipStart
                               + YOffset;

    struct FGFxSetUIViewportCommand : public FRenderCommand
    {
        GMatrix2D              NewViewMatrix;
        FGFxViewportUserParams ViewportParams;
        FGFxRenderer*          Renderer;
        FGFxSetUIViewportCommand(const GMatrix2D& InNewViewMatrix,
                                 const FGFxViewportUserParams& InViewportParams,
                                 FGFxRenderer* InRenderer)
            : NewViewMatrix(InNewViewMatrix), ViewportParams(InViewportParams), Renderer(InRenderer) {}
        virtual UINT Execute()
        {
            // 2012 0x5b7a60, term for term: the clipped viewport, the RHI viewport it implies, the
            // new viewport matrix, and the user matrix appended to it. It does NOT touch
            // CurrentMatrix - BeginDisplay sets that to the identity right after - and it does not
            // re-bind the render target. Getting the first of those wrong is visible: the
            // background quad is then transformed by the viewport matrix twice and lands off
            // screen, which is what the first dumped image showed.
            Renderer->ViewRect = ViewportParams;
            RHISetViewport(ViewportParams.xAxis.ViewStart, ViewportParams.yAxis.ViewStart, 0.f,
                           ViewportParams.xAxis.ViewStart + ViewportParams.xAxis.ViewLength,
                           ViewportParams.yAxis.ViewStart + ViewportParams.yAxis.ViewLength, 0.f);
            Renderer->ViewportMatrix = NewViewMatrix;
            FGFxMatrix2DAppend(Renderer->ViewportMatrix, Renderer->UserMatrix);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetUIViewportCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxSetUIViewportCommand,(NewViewportMatrix,InViewportParams,this));
}

// 2012 0x5d7e10: one command resets the whole render state block the UI needs - blend stack, depth
// off, no culling, no stencil - and computes the inverse gamma from the view flags.
void FGFxRenderer::InitUIBlendStackAndMiscRenderState_RenderingThread(
    FMiscRenderStateInitParams& Params)
{
    CheckRenderTarget_RenderThread();

    Params.BlendModeStack->Empty(16);
    *Params.BlendMode = GRenderer::Blend_None;
    *Params.bAlphaComposite = (Params.ViewFlags >> 1) & 1;
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(*Params.bAlphaComposite, *Params.BlendMode, 0);

    RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
    RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
    // DISHONORED(bringup, agent DC): the cached stencil state is SET here, not released. This line
    // read `CurStencilState.SafeRelease()`, and the consequence is a crash rather than a wrong pixel:
    // CheckRenderTarget_RenderThread (2012 0x5d7c80, and this file's copy of it) re-asserts the state
    // with `RHISetStencilState(CurStencilState)` on every render-target change, and
    // FD3D9DynamicRHI::SetStencilState dereferences its argument with no null check (D3D9Commands.cpp,
    // which is the reference engine's own body, so retail's does too). The first frame the game drew a
    // movie through the real path took the null: "Rendering thread exception" at
    // FD3D9DynamicRHI::SetStencilState+0x18, resolved against the link map. Retail cannot have a null
    // there either, so the cached state must track the one just pushed - which is what this does.
    RHISetStencilState(TStaticStencilState<>::GetRHI());
    CurStencilState = TStaticStencilState<>::GetRHI();

    *Params.StencilCounter = 0;

    // DISHONORED(layout): the two gamma bits the movie player sets in GViewport::Flags above the
    // PDB's four - 0x1000 asks for no gamma correction at all and 0x2000 for the display gamma
    // itself rather than its inverse.
    INT GammaMode = (Params.ViewFlags & 0x1000) ? 0 : ((Params.ViewFlags & 0x2000) ? -1 : 1);
    FLOAT DisplayGamma;
    if (RenderTarget != NULL)
    {
        DisplayGamma = RenderTarget->GetDisplayGamma();
    }
    else
    {
        --GammaMode;
        DisplayGamma = (GEngine && GEngine->Client) ? GEngine->Client->DisplayGamma : 2.2f;
    }
    if (GammaMode < 0)
    {
        *Params.InverseGamma = DisplayGamma > 0.f ? DisplayGamma : 2.2f;
    }
    else if (GammaMode == 0)
    {
        *Params.InverseGamma = 1.f;
    }
    else
    {
        *Params.InverseGamma = DisplayGamma > 0.f ? 1.f / DisplayGamma : 0.45454544f;
    }

    TShaderMapRef<FGFxPixelShader<GFx_PS_TextTexture> > Shader(GetGlobalShaderMap(GRHIShaderPlatform));
    FGFxPixelShaderInterface* PixelShader = Shader->GetShaderInterface();
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                          *Params.InverseGamma);
    if (CurRenderTarget != NULL && CurRenderTarget->Resource != NULL)
    {
        CurRenderTarget->Resource->InverseGamma = *Params.InverseGamma;
    }
}

// 2012 0x5d3e40 -> 2013 0x593c40 and 0x5cc9c0.
void FGFxRenderer::EndDisplay()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::EndDisplay");
    struct FGFxEndDisplayCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxEndDisplayCommand(FGFxRenderer* InRenderer) : Renderer(InRenderer) {}
        virtual UINT Execute() { Renderer->EndDisplay_RenderThread(); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxEndDisplayCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxEndDisplayCommand,(this));
}

void FGFxRenderer::EndDisplay_RenderThread()
{
    if (VertexStore != NULL)
    {
        VertexStore->Release_RenderThread(&ElementAccessLock);
        VertexStore = NULL;
    }
    if (IndexStore != NULL)
    {
        IndexStore->Release_RenderThread(&ElementAccessLock);
        IndexStore = NULL;
    }
    LineStyle.EndDisplay_RenderThread();
    FillStyle.EndDisplay_RenderThread();
}

void FGFxRenderer::SetMatrix(const GMatrix2D& Matrix)
{
    // 2012 0x5cca20 -> 2013 0x58ac30: a store, which is why it is not a render command.
    CurrentMatrix = Matrix;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetMatrix");
}

void FGFxRenderer::SetUserMatrix(const GMatrix2D& Matrix)
{
    UserMatrix = Matrix;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetUserMatrix");
}

void FGFxRenderer::SetCxform(const GRenderer::Cxform& Cx)
{
    CurrentCxform = Cx;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetCxform");
}

// 2012 0x5cfa50 / 0x5d3f60 and their commands at 0x5cca60 / 0x5cfbd0.
void FGFxRenderer::PushBlendMode(GRenderer::BlendType Mode)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PushBlendMode");
    struct FGFxPushUIBlendModeCommand : public FRenderCommand
    {
        FGFxRenderer*        Renderer;
        GRenderer::BlendType Mode;
        FGFxPushUIBlendModeCommand(FGFxRenderer* InRenderer, GRenderer::BlendType InMode)
            : Renderer(InRenderer), Mode(InMode) {}
        virtual UINT Execute()
        {
            Renderer->BlendModeStack.AddItem(Renderer->BlendMode);
            if (Mode > GRenderer::Blend_Layer)
            {
                Renderer->BlendMode = Mode;
                FGFxRendererImpl::ApplyUIBlendMode_RenderThread(Renderer->bAlphaComposite,
                                                                Renderer->BlendMode, 0);
            }
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxPushUIBlendModeCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxPushUIBlendModeCommand,(this,Mode));
}

void FGFxRenderer::PopBlendMode()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PopBlendMode");
    struct FGFxPopUIBlendModeCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxPopUIBlendModeCommand(FGFxRenderer* InRenderer) : Renderer(InRenderer) {}
        virtual UINT Execute()
        {
            if (Renderer->BlendModeStack.Num() > 0)
            {
                const GRenderer::BlendType Mode =
                    Renderer->BlendModeStack(Renderer->BlendModeStack.Num() - 1);
                Renderer->BlendModeStack.Remove(Renderer->BlendModeStack.Num() - 1);
                if (Mode != Renderer->BlendMode)
                {
                    Renderer->BlendMode = Mode;
                    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(Renderer->bAlphaComposite,
                                                                    Renderer->BlendMode, 0);
                }
            }
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxPopUIBlendModeCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxPopUIBlendModeCommand,(this));
}

bool FGFxRenderer::PushUserData(GRenderer::UserData* /*Data*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::PushUserData");
    return false;
}

void FGFxRenderer::PopUserData() { GFXUI_SEAM_TRACE("FGFxRenderer::PopUserData"); }

// 2012 0x5bb690 / 0x5bb7d0 / 0x5bb910: the 3D matrices are render-thread state, so each slot is a
// command that writes it and marks the combined matrix stale.
void FGFxRenderer::SetPerspective3D(const GMatrix3D& Persp)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetPerspective3D");
    struct FGFxSetPerspective3DCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        GMatrix3D     Persp;
        FGFxSetPerspective3DCommand(FGFxRenderer* InRenderer, const GMatrix3D& InPersp)
            : Renderer(InRenderer), Persp(InPersp) {}
        virtual UINT Execute()
        {
            Renderer->ProjMatrix = Persp;
            Renderer->UVPMatricesChanged = 1;
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetPerspective3DCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxSetPerspective3DCommand,(this,Persp));
}

void FGFxRenderer::SetView3D(const GMatrix3D& View)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetView3D");
    struct FGFxSetView3DCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        GMatrix3D     View;
        FGFxSetView3DCommand(FGFxRenderer* InRenderer, const GMatrix3D& InView)
            : Renderer(InRenderer), View(InView) {}
        virtual UINT Execute()
        {
            Renderer->ViewMatrix = View;
            Renderer->UVPMatricesChanged = 1;
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetView3DCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxSetView3DCommand,(this,View));
}

void FGFxRenderer::SetWorld3D(const GMatrix3D* World)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetWorld3D");
    struct FGFxSetWorld3DCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        GMatrix3D     World;
        UBOOL         bEnabled;
        FGFxSetWorld3DCommand(FGFxRenderer* InRenderer, const GMatrix3D& InWorld, UBOOL InbEnabled)
            : Renderer(InRenderer), World(InWorld), bEnabled(InbEnabled) {}
        virtual UINT Execute()
        {
            Renderer->WorldMatrix = World;
            Renderer->Is3DEnabled = bEnabled ? 1 : 0;
            Renderer->UVPMatricesChanged = 1;
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxSetWorld3DCommand"); }
    };
    GMatrix3D Identity;
    ENQUEUE_RENDER_COMMAND(FGFxSetWorld3DCommand,(this,World ? *World : Identity,World != NULL));
}

// DISHONORED(bringup): GRenderer's own body, which lives in libgfx and is not in this tree's
// decompiles. Only the 3D display path calls it, and GFxCharacter::SetMatrix3D returns false today
// (agentBC.md 6.9), so nothing reaches it.
void FGFxRenderer::MakeViewAndPersp3D(const GRect<float>& /*FrameRect*/, GMatrix3D& View,
                                      GMatrix3D& Persp, float /*FovY*/, bool /*bInvertY*/)
{
    View.SetIdentity();
    Persp.SetIdentity();
    GFXUI_SEAM_TRACE("FGFxRenderer::MakeViewAndPersp3D");
}

void FGFxRenderer::SetStereoParams(GRenderer::StereoParams Params)
{
    S3DParams = Params;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetStereoParams");
}

void FGFxRenderer::SetStereoDisplay(GRenderer::StereoDisplay Display, bool /*bSet*/)
{
    S3DDisplay = Display;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetStereoDisplay");
}

// 2012 0x5d9ca0 / 0x5d9cd0 -> 2013 0x599f90 / 0x599fc0.
void FGFxRenderer::SetVertexData(const void* Vertices, int NumVertices,
                                 GRenderer::VertexFormat Format, GRenderer::CacheProvider* Cache)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetVertexData");
    FGFxRendererImpl::SetUIRenderElementStore<FGFxRendererImpl::FGFxVertexStore,
                                             GRenderer::VertexFormat>(
        this, GRenderer::Cached_Vertex, Vertices, NumVertices, Format, Cache, VertexStore);
}

void FGFxRenderer::SetIndexData(const void* Indices, int NumIndices,
                                GRenderer::IndexFormat Format, GRenderer::CacheProvider* Cache)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::SetIndexData");
    FGFxRendererImpl::SetUIRenderElementStore<FGFxRendererImpl::FGFxIndexStore,
                                             GRenderer::IndexFormat>(
        this, GRenderer::Cached_Index, Indices, NumIndices, Format, Cache, IndexStore);
}

void FGFxRenderer::ReleaseCachedData(GRenderer::CachedData* Data, GRenderer::CachedDataType /*Type*/)
{
    // 2012 0x5c4b80.
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseCachedData");
    if (Data != NULL && Data->hData != NULL)
    {
        ((FGFxRendererImpl::FGFxRenderElementStoreBase*)Data->hData)
            ->Release_MainThread(&ElementAccessLock);
    }
}

// 2012 0x5e2160 -> 2013 0x5a1620.
void FGFxRenderer::DrawIndexedTriList(int BaseVertexIndex, int MinVertexIndex, int NumVertices,
                                      int StartIndex, int TriangleCount)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawIndexedTriList");
    struct FGFxDrawUIIndexedTriListCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        INT BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, TriangleCount;
        FGFxDrawUIIndexedTriListCommand(FGFxRenderer* InRenderer, INT InBaseVertexIndex,
                                        INT InMinVertexIndex, INT InNumVertices, INT InStartIndex,
                                        INT InTriangleCount)
            : Renderer(InRenderer), BaseVertexIndex(InBaseVertexIndex),
              MinVertexIndex(InMinVertexIndex), NumVertices(InNumVertices),
              StartIndex(InStartIndex), TriangleCount(InTriangleCount) {}
        virtual UINT Execute()
        {
            Renderer->DrawIndexedTriList_RenderThread(BaseVertexIndex, MinVertexIndex, NumVertices,
                                                      StartIndex, TriangleCount);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDrawUIIndexedTriListCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDrawUIIndexedTriListCommand,
                           (this,BaseVertexIndex,MinVertexIndex,NumVertices,StartIndex,TriangleCount));
    RenderStats.Triangles += TriangleCount;
}

// 2012 0x5db740. The draw is a user-pointer draw straight out of the two stores, under the element
// lock; NumVertices is the store's count minus the base, not the caller's.
void FGFxRenderer::DrawIndexedTriList_RenderThread(int BaseVertexIndex, int MinVertexIndex,
                                                   int /*NumVertices*/, int StartIndex,
                                                   int TriangleCount)
{
    if (FillStyle.StyleMode == GFx_SM_Disabled || VertexStore == NULL || IndexStore == NULL)
    {
        return;
    }
    CheckRenderTarget_RenderThread();

    FGFxRendererImpl::FGFxVertexStore* LocalVertexStore = VertexStore;
    FGFxRendererImpl::FGFxIndexStore* LocalIndexStore = IndexStore;

    FGFxEnumeratedBoundShaderState Enumerated;
    Enumerated.PixelShaderType = GFx_PS_SolidColor;
    Enumerated.VertexShaderType = GFx_VS_None;
    Enumerated.VertexDeclarationType = GFx_VD_None;
    FGFxRenderStyleContext StyleContext;
    StyleContext.VertexFmt = LocalVertexStore->Format;
    StyleContext.BlendFmt = BlendMode;
    FillStyle.GetEnumeratedBoundShaderState_RenderThread(&Enumerated, &StyleContext);

    FGFxBoundShaderState BoundShaderState;
    FGFxRendererImpl::GetUIBoundShaderState_RenderThread(BoundShaderState, BoundShaderStateCache,
                                                        Enumerated);
    if (!IsValidRef(BoundShaderState.NativeBoundShaderState))
    {
        // DISHONORED(bringup, agent DC): name the combination that failed. Silence here is what made
        // 103 of a frame's 134 submitted draws vanish with nothing in the log.
        static UBOOL bWarned[GFx_PS_Count] = { FALSE };
        if (Enumerated.PixelShaderType < GFx_PS_Count && !bWarned[Enumerated.PixelShaderType])
        {
            bWarned[Enumerated.PixelShaderType] = TRUE;
            debugf(NAME_Warning, TEXT("DISHONORED(bringup): GFx trilist draw dropped: no bound shader ")
                   TEXT("state for pixel %d vertex %d decl %d (fill mode %d, vertex fmt %d)"),
                   (INT)Enumerated.PixelShaderType, (INT)Enumerated.VertexShaderType,
                   (INT)Enumerated.VertexDeclarationType, (INT)FillStyle.StyleMode,
                   (INT)StyleContext.VertexFmt);
        }
        return;
    }

    ApplyUITransform_RenderThread(ViewportMatrix, CurrentMatrix,
                                  *BoundShaderState.VertexShaderInterface);
    FillStyle.Apply_RenderThread(this, &BoundShaderState, &StyleContext);
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(bAlphaComposite, BlendMode, 0);

    FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                          CurRenderTarget->Resource->InverseGamma);
    RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);

    {
        GLock::Locker Guard(&ElementAccessLock);
        const BYTE* IndexData = (const BYTE*)LocalIndexStore->Elements
                              + StartIndex * LocalIndexStore->ElementSize;
        const BYTE* VertexData = (const BYTE*)LocalVertexStore->Elements
                               + BaseVertexIndex * LocalVertexStore->ElementSize;
        RHIDrawIndexedPrimitiveUP(PT_TriangleList, MinVertexIndex,
                                  LocalVertexStore->NumElements - BaseVertexIndex, TriangleCount,
                                  IndexData, LocalIndexStore->ElementSize, VertexData,
                                  LocalVertexStore->ElementSize);
    }
    ++GGFxDrawCensus.Draws;
    ++GGFxDrawCensus.TriListDraws;
    GGFxDrawCensus.Triangles += TriangleCount;
}

// 2012 0x5e22c0 / 0x5db940: the strip is expanded into a line list of vertex pairs, 384 vertices at
// a time, because the RHI has no line-strip primitive with a base vertex.
void FGFxRenderer::DrawLineStrip(int BaseVertexIndex, int LineCount)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawLineStrip");
    struct FGFxDrawLineStripCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        INT BaseVertexIndex, LineCount;
        FGFxDrawLineStripCommand(FGFxRenderer* InRenderer, INT InBaseVertexIndex, INT InLineCount)
            : Renderer(InRenderer), BaseVertexIndex(InBaseVertexIndex), LineCount(InLineCount) {}
        virtual UINT Execute()
        {
            Renderer->DrawLineStrip_RenderThread(BaseVertexIndex, LineCount);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDrawLineStripCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDrawLineStripCommand,(this,BaseVertexIndex,LineCount));
    RenderStats.Lines += LineCount;
}

void FGFxRenderer::DrawLineStrip_RenderThread(int BaseVertexIndex, int LineCount)
{
    if (VertexStore == NULL || LineCount <= 0)
    {
        return;
    }
    CheckRenderTarget_RenderThread();

    FGFxRendererImpl::FGFxVertexStore* LocalVertexStore = VertexStore;
    FGFxEnumeratedBoundShaderState Enumerated;
    Enumerated.PixelShaderType = GFx_PS_SolidColor;
    Enumerated.VertexShaderType = GFx_VS_None;
    Enumerated.VertexDeclarationType = GFx_VD_None;
    FGFxRenderStyleContext StyleContext;
    StyleContext.VertexFmt = LocalVertexStore->Format;
    StyleContext.BlendFmt = BlendMode;
    LineStyle.GetEnumeratedBoundShaderState_RenderThread(&Enumerated, &StyleContext);

    FGFxBoundShaderState BoundShaderState;
    FGFxRendererImpl::GetUIBoundShaderState_RenderThread(BoundShaderState, BoundShaderStateCache,
                                                        Enumerated);
    if (!IsValidRef(BoundShaderState.NativeBoundShaderState))
    {
        return;
    }

    FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                          CurRenderTarget->Resource->InverseGamma);
    ApplyUITransform_RenderThread(ViewportMatrix, CurrentMatrix,
                                  *BoundShaderState.VertexShaderInterface);
    LineStyle.Apply_RenderThread(this, &BoundShaderState, &StyleContext);
    RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);

    {
        GLock::Locker Guard(&ElementAccessLock);
        const UINT Stride = LocalVertexStore->ElementSize;
        const INT MaxVertices = Min<INT>(2 * LineCount, 384);
        BYTE* Batch = (BYTE*)appMalloc(MaxVertices * Stride);
        const BYTE* Source = (const BYTE*)LocalVertexStore->Elements + BaseVertexIndex * Stride;
        INT Line = 0;
        while (Line < LineCount)
        {
            INT NumVertices = 0;
            while (NumVertices < MaxVertices && Line < LineCount)
            {
                appMemcpy(Batch + NumVertices * Stride, Source + Line * Stride, Stride);
                appMemcpy(Batch + (NumVertices + 1) * Stride, Source + (Line + 1) * Stride, Stride);
                NumVertices += 2;
                ++Line;
            }
            RHIDrawPrimitiveUP(PT_LineList, NumVertices / 2, Batch, Stride);
            ++GGFxDrawCensus.Draws;
            ++GGFxDrawCensus.LineDraws;
            GGFxDrawCensus.Lines += NumVertices / 2;
        }
        appFree(Batch);
    }
}

void FGFxRenderer::LineStyleDisable()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::LineStyleDisable");
    LineStyle.Disable();
}

// 2012 0x5ccf30 / 0x5ccc60: the colour is put through the current colour transform on the way in,
// which is why the style keeps a colour and not a Cxform for the solid case.
void FGFxRenderer::LineStyleColor(GColor Color)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::LineStyleColor");
    struct FGFxLineStyleColorCommand : public FRenderCommand
    {
        FGFxRenderStyle* Style;
        GColor           Color;
        FGFxLineStyleColorCommand(FGFxRenderStyle* InStyle, GColor InColor)
            : Style(InStyle), Color(InColor) {}
        virtual UINT Execute() { Style->SetStyleColor_RenderThread(Color); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxLineStyleColorCommand"); }
    };
    const GColor Transformed = FGFxCxformTransform(CurrentCxform, Color);
    ENQUEUE_RENDER_COMMAND(FGFxLineStyleColorCommand,((FGFxRenderStyle*)&LineStyle,Transformed));
}

void FGFxRenderer::FillStyleDisable()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleDisable");
    FillStyle.Disable();
}

void FGFxRenderer::FillStyleColor(GColor Color)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleColor");
    struct FGFxFillStyleColorCommand : public FRenderCommand
    {
        FGFxRenderStyle* Style;
        GColor           Color;
        FGFxFillStyleColorCommand(FGFxRenderStyle* InStyle, GColor InColor)
            : Style(InStyle), Color(InColor) {}
        virtual UINT Execute() { Style->SetStyleColor_RenderThread(Color); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxFillStyleColorCommand"); }
    };
    const GColor Transformed = FGFxCxformTransform(CurrentCxform, Color);
    ENQUEUE_RENDER_COMMAND(FGFxFillStyleColorCommand,((FGFxRenderStyle*)&FillStyle,Transformed));
}

// 2012 0x5ccac0: the engine texture and the texture matrix are resolved on the main thread, because
// the GFx texture is a main-thread object.
void FGFxRenderer::FillStyleBitmap(const GRenderer::FillTexture* Fill)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleBitmap");
    FFillTextureInfo NewTexInfo;
    if (Fill != NULL)
    {
        FGFxRendererImpl::ConvertFromUI(*Fill, NewTexInfo);
    }
    struct FGFxFillStyleBitmapCommand : public FRenderCommand
    {
        FGFxFillStyle*    Style;
        FFillTextureInfo  NewTexInfoRT;
        GRenderer::Cxform Cx;
        FGFxFillStyleBitmapCommand(FGFxFillStyle* InStyle, const FFillTextureInfo& InTexInfo,
                                   const GRenderer::Cxform& InCx)
            : Style(InStyle), NewTexInfoRT(InTexInfo), Cx(InCx) {}
        virtual UINT Execute()
        {
            Style->SetStyleBitmap_RenderThread(NewTexInfoRT, Cx);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxFillStyleBitmapCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxFillStyleBitmapCommand,(&FillStyle,NewTexInfo,CurrentCxform));
}

// 2012 0x5ccdb0.
void FGFxRenderer::FillStyleGouraud(GRenderer::GouraudFillType Type,
                                    const GRenderer::FillTexture* T0,
                                    const GRenderer::FillTexture* T1,
                                    const GRenderer::FillTexture* T2)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleGouraud");
    FFillTextureInfo Info[3];
    UBOOL bHas[3];
    const GRenderer::FillTexture* Fills[3] = { T0, T1, T2 };
    for (INT i = 0; i < 3; ++i)
    {
        bHas[i] = (Fills[i] != NULL);
        if (bHas[i])
        {
            FGFxRendererImpl::ConvertFromUI(*Fills[i], Info[i]);
        }
    }
    struct FGFxFillStyleGouraudCommand : public FRenderCommand
    {
        FGFxFillStyle*             Style;
        GRenderer::GouraudFillType Type;
        FFillTextureInfo           Info[3];
        UBOOL                      bHas[3];
        GRenderer::Cxform          Cx;
        FGFxFillStyleGouraudCommand(FGFxFillStyle* InStyle, GRenderer::GouraudFillType InType,
                                    const FFillTextureInfo* InInfo, const UBOOL* InbHas,
                                    const GRenderer::Cxform& InCx)
            : Style(InStyle), Type(InType), Cx(InCx)
        {
            for (INT i = 0; i < 3; ++i) { Info[i] = InInfo[i]; bHas[i] = InbHas[i]; }
        }
        virtual UINT Execute()
        {
            Style->SetStyleGouraud_RenderThread(Type, bHas[0] ? &Info[0] : NULL,
                                                bHas[1] ? &Info[1] : NULL,
                                                bHas[2] ? &Info[2] : NULL, Cx);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxFillStyleGouraudCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxFillStyleGouraudCommand,(&FillStyle,Type,Info,bHas,CurrentCxform));
}

// 2012 0x5e2400 -> 2013 0x5a18c0, and 0x5e2670 for the distance-field form: the descriptor list
// becomes an element store and the render thread turns each descriptor into six glyph vertices.
static void FGFxDrawBitmapsInternal(FGFxRenderer* Renderer, GRenderer::BitmapDesc* Bitmaps,
                                    INT ListSize, INT StartIndex, INT Count,
                                    const GTexture* InTexture, const GMatrix2D& Matrix,
                                    GRenderer::CacheProvider* Cache,
                                    const GRenderer::DistanceFieldParams* Params)
{
    if (Bitmaps == NULL || InTexture == NULL || Count == 0)
    {
        return;
    }
    FGFxRendererImpl::FGFxBitmapDescStore* Store = NULL;
    INT FirstIndex = StartIndex;
    if (Cache != NULL && Cache->pData != NULL)
    {
        GRenderer::CachedData* pData = Cache->pData;
        if (pData->pRenderer == Renderer && pData->hData != NULL)
        {
            Store = (FGFxRendererImpl::FGFxBitmapDescStore*)pData->hData;
            ++Store->RefCount;
        }
        else
        {
            if (pData->pRenderer != Renderer)
            {
                if (pData->pRenderer)
                {
                    pData->pRenderer->ReleaseCachedData(pData, GRenderer::Cached_BitmapList);
                }
                pData->pRenderer = Renderer;
                pData->hData = NULL;
            }
            Cache->DiscardSharedData = false;
            Store = new FGFxRendererImpl::FGFxBitmapDescStore;
            Store->InitElements(Renderer, pData, (GRenderer::BitmapDesc*)NULL, ListSize, Bitmaps);
            pData->hData = Store;
        }
    }
    else
    {
        Store = new FGFxRendererImpl::FGFxBitmapDescStore;
        Store->InitElementsCopy((GRenderer::BitmapDesc*)NULL, Count, &Bitmaps[StartIndex]);
        FirstIndex = 0;
    }

    FGFxTexture* Texture = (FGFxTexture*)InTexture;
    Texture->AddRef();

    struct FGFxDrawBitmapsCommand : public FRenderCommand
    {
        FGFxRenderer*                        Renderer;
        FGFxRendererImpl::FGFxBitmapDescStore* Store;
        INT                                  StartIndex;
        INT                                  Count;
        FGFxTexture*                         Texture;
        GMatrix2D                            FontMatrix;
        GRenderer::DistanceFieldParams       Params;
        UBOOL                                bHasParams;
        FGFxDrawBitmapsCommand(FGFxRenderer* InRenderer,
                               FGFxRendererImpl::FGFxBitmapDescStore* InStore, INT InStartIndex,
                               INT InCount, FGFxTexture* InTexture, const GMatrix2D& InFontMatrix,
                               const GRenderer::DistanceFieldParams* InParams)
            : Renderer(InRenderer), Store(InStore), StartIndex(InStartIndex), Count(InCount),
              Texture(InTexture), FontMatrix(InFontMatrix), bHasParams(InParams != NULL)
        {
            if (InParams) { Params = *InParams; }
            else { appMemzero(&Params, sizeof(Params)); }
        }
        virtual UINT Execute()
        {
            Renderer->DrawBitmaps_RenderThread(Store, StartIndex, Count, Texture, FontMatrix,
                                               bHasParams ? &Params : NULL);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDrawBitmapsCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDrawBitmapsCommand,
                           (Renderer,Store,FirstIndex,Count,Texture,Matrix,Params));
    Renderer->RenderStats.Triangles += 2 * Count;
}

void FGFxRenderer::DrawBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize, int StartIndex,
                               int Count, const GTexture* InTexture, const GMatrix2D& Matrix,
                               GRenderer::CacheProvider* Cache)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawBitmaps");
    FGFxDrawBitmapsInternal(this, Bitmaps, ListSize, StartIndex, Count, InTexture, Matrix, Cache,
                            NULL);
}

void FGFxRenderer::DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize,
                                            int StartIndex, int Count, const GTexture* InTexture,
                                            const GMatrix2D& Matrix,
                                            const GRenderer::DistanceFieldParams& Params,
                                            GRenderer::CacheProvider* Cache)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawDistanceFieldBitmaps");
    FGFxDrawBitmapsInternal(this, Bitmaps, ListSize, StartIndex, Count, InTexture, Matrix, Cache,
                            &Params);
}

// 2012 0x5dbb30. Six vertices per descriptor, 192 descriptors a batch, straight into one
// DrawPrimitiveUP - which is what makes a page of text one draw call.
void FGFxRenderer::DrawBitmaps_RenderThread(FGFxRendererImpl::FGFxBitmapDescStore* Store,
                                            int StartIndex, int Count, const FGFxTexture* Texture,
                                            const GMatrix2D& Matrix,
                                            const GRenderer::DistanceFieldParams* Params)
{
    enum { MaxBatchGlyphs = 192, VerticesPerGlyph = 6 };

    CheckRenderTarget_RenderThread();
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(bAlphaComposite, BlendMode, 0);

    FGFxEnumeratedBoundShaderState Enumerated;
    const UBOOL bMultiply = (BlendMode == GRenderer::Blend_Multiply ||
                             BlendMode == GRenderer::Blend_Darken);
    if (Params != NULL)
    {
        Enumerated.PixelShaderType = GFx_PS_TextTextureDFA;
    }
    else if (Texture->Texture != NULL && Texture->Texture->Resource != NULL &&
             Texture->Texture->Resource->bGreyScaleFormat)
    {
        Enumerated.PixelShaderType = GFx_PS_TextTexture;
    }
    else if (Texture->Texture != NULL && !Texture->Texture->SRGB)
    {
        Enumerated.PixelShaderType = bMultiply ? GFx_PS_TextTextureColorMultiply
                                               : GFx_PS_TextTextureColor;
    }
    else
    {
        Enumerated.PixelShaderType = bMultiply ? GFx_PS_TextTextureSRGBMultiply
                                               : GFx_PS_TextTextureSRGB;
    }
    Enumerated.VertexShaderType = GFx_VS_Glyph;
    Enumerated.VertexDeclarationType = GFx_VD_Glyph;

    FGFxBoundShaderState BoundShaderState;
    FGFxRendererImpl::GetUIBoundShaderState_RenderThread(BoundShaderState, BoundShaderStateCache,
                                                        Enumerated);
    if (IsValidRef(BoundShaderState.NativeBoundShaderState))
    {
        FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
        PixelShader->SetParametersColorScaleAndColorBias(
            PixelShader->GetNativeShader()->GetPixelShader(), CurrentCxform);
        PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                             CurRenderTarget->Resource->InverseGamma);
        if (Params != NULL)
        {
            PixelShader->SetDistanceFieldParams(PixelShader->GetNativeShader()->GetPixelShader(),
                                                Texture->Texture2D, *Params);
        }
        Texture->Bind(0, *PixelShader, GRenderer::Wrap_Clamp, GRenderer::Sample_Linear, true);
        ApplyUITransform_RenderThread(ViewportMatrix, Matrix,
                                      *BoundShaderState.VertexShaderInterface);
        RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);

        GLock::Locker Guard(&ElementAccessLock);
        FGFxVertex_Glyph Batch[MaxBatchGlyphs * VerticesPerGlyph];
        const GRenderer::BitmapDesc* Descs = (const GRenderer::BitmapDesc*)Store->Elements
                                           + StartIndex;
        INT Glyph = 0;
        while (Glyph < Count)
        {
            INT NumVertices = 0;
            while (NumVertices < MaxBatchGlyphs * VerticesPerGlyph && Glyph < Count)
            {
                const GRenderer::BitmapDesc& Desc = Descs[Glyph];
                FGFxVertex_Glyph* V = Batch + NumVertices;
                V[0].X = Desc.Coords.Left;  V[0].Y = Desc.Coords.Top;
                V[0].U = Desc.TextureCoords.Left; V[0].V = Desc.TextureCoords.Top;
                V[1].X = Desc.Coords.Right; V[1].Y = Desc.Coords.Top;
                V[1].U = Desc.TextureCoords.Right; V[1].V = Desc.TextureCoords.Top;
                V[2].X = Desc.Coords.Left;  V[2].Y = Desc.Coords.Bottom;
                V[2].U = Desc.TextureCoords.Left; V[2].V = Desc.TextureCoords.Bottom;
                V[3].X = Desc.Coords.Left;  V[3].Y = Desc.Coords.Bottom;
                V[3].U = Desc.TextureCoords.Left; V[3].V = Desc.TextureCoords.Bottom;
                V[4].X = Desc.Coords.Right; V[4].Y = Desc.Coords.Top;
                V[4].U = Desc.TextureCoords.Right; V[4].V = Desc.TextureCoords.Top;
                V[5].X = Desc.Coords.Right; V[5].Y = Desc.Coords.Bottom;
                V[5].U = Desc.TextureCoords.Right; V[5].V = Desc.TextureCoords.Bottom;
                for (INT v = 0; v < VerticesPerGlyph; ++v)
                {
                    V[v].Color = Desc.Color;
                }
                NumVertices += VerticesPerGlyph;
                ++Glyph;
            }
            RHIDrawPrimitiveUP(PT_TriangleList, NumVertices / 3, Batch, sizeof(FGFxVertex_Glyph));
            ++GGFxDrawCensus.Draws;
            ++GGFxDrawCensus.BitmapDraws;
            GGFxDrawCensus.Triangles += NumVertices / 3;
            GGFxDrawCensus.Glyphs += NumVertices / VerticesPerGlyph;
        }
    }

    Store->Release_RenderThread(&ElementAccessLock);
    ((FGFxTexture*)Texture)->Release();
}

// 2012 0x5dce70 / 0x5cfe30 / 0x5d4100 and their render-thread halves 0x5d8170 / 0x5c4df0 /
// 0x5cd090: masks are stencil, and a nested mask is a stencil increment.
void FGFxRenderer::BeginSubmitMask(GRenderer::SubmitMaskMode Mode)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::BeginSubmitMask");
    struct FGFxBeginSubmitMaskCommand : public FRenderCommand
    {
        FGFxRenderer*               Renderer;
        GRenderer::SubmitMaskMode   Mode;
        FGFxBeginSubmitMaskCommand(FGFxRenderer* InRenderer, GRenderer::SubmitMaskMode InMode)
            : Renderer(InRenderer), Mode(InMode) {}
        virtual UINT Execute()
        {
            Renderer->BeginSubmitMask_RenderThread(Mode);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxBeginSubmitMaskCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxBeginSubmitMaskCommand,(this,Mode));
    ++RenderStats.Masks;
}

void FGFxRenderer::BeginSubmitMask_RenderThread(GRenderer::SubmitMaskMode Mode)
{
    CheckRenderTarget_RenderThread();
    RHISetColorWriteEnable(FALSE);

    FStencilStateInitializerRHI Initializer;
    Initializer.bEnableFrontFaceStencil = TRUE;
    Initializer.FrontFaceStencilTest = CF_Always;
    Initializer.FrontFaceStencilFailStencilOp = SO_Keep;
    Initializer.FrontFaceDepthFailStencilOp = SO_Keep;
    Initializer.FrontFacePassStencilOp = SO_Keep;
    Initializer.bEnableBackFaceStencil = FALSE;
    Initializer.BackFaceStencilTest = CF_Always;
    Initializer.BackFaceStencilFailStencilOp = SO_Keep;
    Initializer.BackFaceDepthFailStencilOp = SO_Keep;
    Initializer.BackFacePassStencilOp = SO_Keep;
    Initializer.StencilReadMask = 0xFFFFFFFF;
    Initializer.StencilWriteMask = 0xFFFFFFFF;
    Initializer.StencilRef = 0;

    if (Mode == GRenderer::Mask_Increment)
    {
        Initializer.StencilRef = StencilCounter;
        Initializer.FrontFaceStencilTest = CF_Equal;
        Initializer.FrontFacePassStencilOp = SO_Increment;
        StencilCounter = Initializer.StencilRef + 1;
    }
    else if (Mode == GRenderer::Mask_Decrement)
    {
        Initializer.StencilRef = StencilCounter;
        Initializer.FrontFaceStencilTest = CF_Equal;
        Initializer.FrontFacePassStencilOp = SO_Decrement;
        StencilCounter = Initializer.StencilRef - 1;
    }
    else
    {
        RHIClear(FALSE, FLinearColor::Black, FALSE, 0.f, TRUE, 0);
        Initializer.FrontFacePassStencilOp = SO_Replace;
        Initializer.StencilRef = 1;
        StencilCounter = 1;
    }

    CurStencilState = RHICreateStencilState(Initializer);
    RHISetStencilState(CurStencilState);
    ++GGFxDrawCensus.MaskPasses;
}

void FGFxRenderer::EndSubmitMask()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::EndSubmitMask");
    struct FGFxEndSubmitMaskCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxEndSubmitMaskCommand(FGFxRenderer* InRenderer) : Renderer(InRenderer) {}
        virtual UINT Execute() { Renderer->EndSubmitMask_RenderThread(); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxEndSubmitMaskCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxEndSubmitMaskCommand,(this));
}

void FGFxRenderer::EndSubmitMask_RenderThread()
{
    RHISetColorWriteEnable(TRUE);
    FStencilStateInitializerRHI Initializer;
    Initializer.bEnableFrontFaceStencil = TRUE;
    Initializer.FrontFaceStencilTest = CF_Equal;
    Initializer.FrontFaceStencilFailStencilOp = SO_Keep;
    Initializer.FrontFaceDepthFailStencilOp = SO_Keep;
    Initializer.FrontFacePassStencilOp = SO_Keep;
    Initializer.bEnableBackFaceStencil = FALSE;
    Initializer.BackFaceStencilTest = CF_Always;
    Initializer.BackFaceStencilFailStencilOp = SO_Keep;
    Initializer.BackFaceDepthFailStencilOp = SO_Keep;
    Initializer.BackFacePassStencilOp = SO_Keep;
    Initializer.StencilReadMask = 0xFFFFFFFF;
    Initializer.StencilWriteMask = 0xFFFFFFFF;
    Initializer.StencilRef = StencilCounter;

    CurStencilState = RHICreateStencilState(Initializer);
    RHISetStencilState(CurStencilState);
}

void FGFxRenderer::DisableMask()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DisableMask");
    struct FGFxDisableMaskCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxDisableMaskCommand(FGFxRenderer* InRenderer) : Renderer(InRenderer) {}
        virtual UINT Execute() { Renderer->DisableMask_RenderThread(); return sizeof(*this); }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDisableMaskCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDisableMaskCommand,(this));
}

void FGFxRenderer::DisableMask_RenderThread()
{
    RHISetColorWriteEnable(TRUE);
    StencilCounter = 0;
    CurStencilState = TStaticStencilState<>::GetRHI();
    RHISetStencilState(CurStencilState);
}

// 2012 0x5b7eb0: one pass unless the runtime asked for more than one or the blur is bigger than a
// 64-pixel kernel, in which case the runtime splits it and calls back once per pass.
unsigned int FGFxRenderer::CheckFilterSupport(const GRenderer::BlurFilterParams& Params)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::CheckFilterSupport");
    if (Params.Passes > 1 || Params.BlurX * Params.BlurY > 64.f)
    {
        return GRenderer::FilterSupport_Ok | GRenderer::FilterSupport_Multipass;
    }
    return GRenderer::FilterSupport_Ok;
}

void FGFxRenderer::DrawBlurRect(GTexture* Source, const GRect<float>& Dest,
                                const GRect<float>& Src, const GRenderer::BlurFilterParams& Params,
                                bool /*bOnStack*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawBlurRect");
    struct FGFxDrawBlurRectCommand : public FRenderCommand
    {
        FGFxRenderer*                  Renderer;
        FGFxTexture*                   Source;
        GRect<float>                   Dest;
        GRect<float>                   Src;
        GRenderer::BlurFilterParams    Params;
        FGFxDrawBlurRectCommand(FGFxRenderer* InRenderer, FGFxTexture* InSource,
                                const GRect<float>& InDest, const GRect<float>& InSrc,
                                const GRenderer::BlurFilterParams& InParams)
            : Renderer(InRenderer), Source(InSource), Dest(InDest), Src(InSrc), Params(InParams) {}
        virtual UINT Execute()
        {
            Renderer->DrawBlurRect_RenderThread(Source, Dest, Src, Params);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDrawBlurRectCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDrawBlurRectCommand,(this,(FGFxTexture*)Source,Dest,Src,Params));
    ++RenderStats.Filters;
}

void FGFxRenderer::DrawColorMatrixRect(GTexture* Source, const GRect<float>& Dest,
                                       const GRect<float>& Src, const float* Matrix,
                                       bool /*bOnStack*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawColorMatrixRect");
    struct FGFxDrawColorMatrixRectCommand : public FRenderCommand
    {
        FGFxRenderer* Renderer;
        FGFxTexture*  Source;
        GRect<float>  Dest;
        GRect<float>  Src;
        FLOAT         Matrix[20];
        FGFxDrawColorMatrixRectCommand(FGFxRenderer* InRenderer, FGFxTexture* InSource,
                                       const GRect<float>& InDest, const GRect<float>& InSrc,
                                       const FLOAT* InMatrix)
            : Renderer(InRenderer), Source(InSource), Dest(InDest), Src(InSrc)
        {
            appMemcpy(Matrix, InMatrix, sizeof(Matrix));
        }
        virtual UINT Execute()
        {
            Renderer->DrawColorMatrixRect_RenderThread(Source, Dest, Src, Matrix);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxDrawColorMatrixRectCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxDrawColorMatrixRectCommand,
                           (this,(FGFxTexture*)Source,Dest,Src,Matrix));
    ++RenderStats.Filters;
}

// The one quad both filter draws share: the destination rectangle with the source rectangle's
// texture coordinates, scaled by the source render target's size and clamped by AdjustBounds.
static UBOOL FGFxBuildFilterQuad(FGFxTexture* Source, const GRect<float>& Dest,
                                 const GRect<float>& Src, FGFxVertex_Glyph* OutVerts)
{
    if (Source == NULL || Source->RenderTarget == NULL || Source->RenderTarget->Resource == NULL)
    {
        return FALSE;
    }
    FGFxRenderTarget* RT = Source->RenderTarget;
    FLOAT TexScaleX = 1.f / (FLOAT)RT->Resource->SizeX;
    FLOAT TexScaleY = 1.f / (FLOAT)RT->Resource->SizeY;
    FLOAT Width = Src.Right - Src.Left;
    FLOAT Height = Src.Bottom - Src.Top;
    if (RT->AdjustBounds(&Width, &Height) && (Src.Right - Src.Left) != 0.f &&
        (Src.Bottom - Src.Top) != 0.f)
    {
        TexScaleX *= Width / (Src.Right - Src.Left);
        TexScaleY *= Height / (Src.Bottom - Src.Top);
    }
    const FLOAT U0 = Src.Left * TexScaleX;
    const FLOAT U1 = Src.Right * TexScaleX;
    const FLOAT V0 = Src.Top * TexScaleY;
    const FLOAT V1 = Src.Bottom * TexScaleY;
    OutVerts[0].X = Dest.Left;  OutVerts[0].Y = Dest.Top;    OutVerts[0].U = U0; OutVerts[0].V = V0;
    OutVerts[1].X = Dest.Right; OutVerts[1].Y = Dest.Top;    OutVerts[1].U = U1; OutVerts[1].V = V0;
    OutVerts[2].X = Dest.Left;  OutVerts[2].Y = Dest.Bottom;  OutVerts[2].U = U0; OutVerts[2].V = V1;
    OutVerts[3].X = Dest.Right; OutVerts[3].Y = Dest.Bottom;  OutVerts[3].U = U1; OutVerts[3].V = V1;
    for (INT i = 0; i < 4; ++i)
    {
        OutVerts[i].Color.Raw = 0;
    }
    return TRUE;
}

// 2012 0x5dbf50.
void FGFxRenderer::DrawColorMatrixRect_RenderThread(GTexture* Source, const GRect<float>& Dest,
                                                    const GRect<float>& Src, const float* Matrix)
{
    CheckRenderTarget_RenderThread();

    FGFxEnumeratedBoundShaderState Enumerated;
    Enumerated.PixelShaderType = (BlendMode == GRenderer::Blend_Multiply ||
                                  BlendMode == GRenderer::Blend_Darken)
                               ? FS2_FCMatrixMul : FS2_FCMatrix;
    Enumerated.VertexShaderType = GFx_VS_Glyph;
    Enumerated.VertexDeclarationType = GFx_VD_Glyph;

    FGFxBoundShaderState BoundShaderState;
    FGFxRendererImpl::GetUIBoundShaderState_RenderThread(BoundShaderState, BoundShaderStateCache,
                                                        Enumerated);
    if (!IsValidRef(BoundShaderState.NativeBoundShaderState))
    {
        return;
    }
    FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
    ApplyUITransform_RenderThread(ViewportMatrix, CurrentMatrix,
                                  *BoundShaderState.VertexShaderInterface);
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(1, BlendMode, 1);
    PixelShader->SetParameterColorMatrix(PixelShader->GetNativeShader()->GetPixelShader(), Matrix);
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(),
                                         CurRenderTarget->Resource->InverseGamma);
    ((FGFxTexture*)Source)->Bind(0, *PixelShader, GRenderer::Wrap_Clamp, GRenderer::Sample_Linear,
                                 false);
    RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);

    FGFxVertex_Glyph Verts[4];
    if (FGFxBuildFilterQuad((FGFxTexture*)Source, Dest, Src, Verts))
    {
        RHIDrawPrimitiveUP(PT_TriangleStrip, 2, Verts, sizeof(FGFxVertex_Glyph));
        ++GGFxDrawCensus.Draws;
        ++GGFxDrawCensus.FilterDraws;
        GGFxDrawCensus.Triangles += 2;
    }
}

// 2012 0x5dc220. The shader selection is retail's, bit for bit: a blur is FBox2Blur (+2 for the
// multiply variants), a shadow is FBox2Shadow or FBox2InnerShadow (+4 knockout, +1 highlight, +2
// multiply), and Filter_HideObject makes it a shadow-only kind.
//
// DISHONORED(bringup): retail also runs up to three of these passes into temp render targets when
// one pass cannot hold the kernel - the loop over pass[3] in the decompile - and this port draws the
// single pass the parameters name. CheckFilterSupport already reports FilterSupport_Multipass for
// the big kernels, which is the runtime's cue to split the filter itself, so the multi-pass case
// arrives here as several single-pass calls. What is not reproduced is retail's internal ping-pong
// for the shadow composite.
void FGFxRenderer::DrawBlurRect_RenderThread(GTexture* Source, const GRect<float>& Dest,
                                             const GRect<float>& Src,
                                             const GRenderer::BlurFilterParams& Params)
{
    FGFxTexture* SourceTexture = (FGFxTexture*)Source;
    CheckRenderTarget_RenderThread();
    if (SourceTexture == NULL || SourceTexture->RenderTarget == NULL)
    {
        return;
    }

    const UBOOL bMultiply = (BlendMode == GRenderer::Blend_Multiply ||
                             BlendMode == GRenderer::Blend_Darken);
    INT Kind = FS2_FBox2Blur + (bMultiply ? 2 : 0);
    if (Params.Mode & GRenderer::Filter_Shadow)
    {
        if (Params.Mode & GRenderer::Filter_HideObject)
        {
            Kind = FS2_FBox2Shadowonly;
            if (Params.Mode & GRenderer::Filter_Highlight)
            {
                Kind = FS2_FBox2ShadowonlyHighlight;
            }
        }
        else
        {
            Kind = (Params.Mode & GRenderer::Filter_Inner) ? FS2_FBox2InnerShadow : FS2_FBox2Shadow;
            if (Params.Mode & GRenderer::Filter_Knockout)
            {
                Kind += 4;
            }
            if (Params.Mode & GRenderer::Filter_Highlight)
            {
                Kind += 1;
            }
        }
        if (bMultiply)
        {
            Kind += 2;
        }
    }

    FGFxEnumeratedBoundShaderState Enumerated;
    Enumerated.PixelShaderType = (EGFxPixelShaderType)Kind;
    Enumerated.VertexShaderType = GFx_VS_Glyph;
    Enumerated.VertexDeclarationType = GFx_VD_Glyph;

    FGFxBoundShaderState BoundShaderState;
    FGFxRendererImpl::GetUIBoundShaderState_RenderThread(BoundShaderState, BoundShaderStateCache,
                                                        Enumerated);
    if (!IsValidRef(BoundShaderState.NativeBoundShaderState))
    {
        return;
    }
    FGFxPixelShaderInterface* PixelShader = BoundShaderState.PixelShaderInterface;
    ApplyUITransform_RenderThread(ViewportMatrix, CurrentMatrix,
                                  *BoundShaderState.VertexShaderInterface);
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(1, BlendMode, 1);

    FGFxRenderTarget* SourceRT = SourceTexture->RenderTarget;
    const FLOAT TexScaleX = 1.f / (FLOAT)SourceRT->Resource->SizeX;
    const FLOAT TexScaleY = 1.f / (FLOAT)SourceRT->Resource->SizeY;
    FPixelShaderRHIParamRef NativePixelShader = PixelShader->GetNativeShader()->GetPixelShader();
    PixelShader->SetParameterTexScale(NativePixelShader, 0, TexScaleX, TexScaleY);
    PixelShader->SetParameterTexScale(NativePixelShader, 1, TexScaleX, TexScaleY);
    PixelShader->SetParameterFilterSize4(NativePixelShader, Params.BlurX * TexScaleX,
                                         Params.BlurY * TexScaleY, Params.Strength, 1.f);
    PixelShader->SetParameterShadowColor(NativePixelShader, 0, Params.Color);
    PixelShader->SetParameterShadowColor(NativePixelShader, 1, Params.Color2);
    PixelShader->SetParameterShadowOffset(NativePixelShader, Params.Offset.x * TexScaleX,
                                          Params.Offset.y * TexScaleY);
    PixelShader->SetParametersCxformAc(NativePixelShader, Params.cxform);
    PixelShader->SetParameterInverseGamma(NativePixelShader,
                                          CurRenderTarget->Resource->InverseGamma);
    SourceTexture->Bind(0, *PixelShader, GRenderer::Wrap_Clamp, GRenderer::Sample_Linear, false);
    RHISetBoundShaderState(BoundShaderState.NativeBoundShaderState);

    FGFxVertex_Glyph Verts[4];
    if (FGFxBuildFilterQuad(SourceTexture, Dest, Src, Verts))
    {
        RHIDrawPrimitiveUP(PT_TriangleStrip, 2, Verts, sizeof(FGFxVertex_Glyph));
        ++GGFxDrawCensus.Draws;
        ++GGFxDrawCensus.FilterDraws;
        GGFxDrawCensus.Triangles += 2;
    }
}

void FGFxRenderer::GetRenderStats(GRenderer::Stats* Stats, bool bReset)
{
    // 2012 0x5b7c90.
    if (Stats) *Stats = RenderStats;
    if (bReset) appMemzero(&RenderStats, sizeof(RenderStats));
    GFXUI_SEAM_TRACE("FGFxRenderer::GetRenderStats");
}

void FGFxRenderer::GetStats(GStatBag* /*Bag*/, bool /*bReset*/)
{
    // The Shipping build compiles the stat bodies out (agentBB.md: GStatBag is sizeof 1), so there
    // is nothing for this to fill.
    GFXUI_SEAM_TRACE("FGFxRenderer::GetStats");
}

// 2012 0x5dec80 -> 2013 0x59db10.
void FGFxRenderer::ReleaseResources()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseResources");
    while (ElementStoreList.pNext != &ElementStoreList)
    {
        FGFxRendererImpl::FGFxRenderElementStoreBase* Store =
            (FGFxRendererImpl::FGFxRenderElementStoreBase*)ElementStoreList.pNext;
        if (Store->CachedData != NULL)
        {
            Store->CachedData->pRenderer = NULL;
            Store->CachedData->hData = NULL;
        }
        Store->Release_MainThread(&ElementAccessLock);
    }
    if (IsInGameThread())
    {
        ReleaseTempRenderTargets(0);
        FlushRenderingCommands();
    }
    else
    {
        ReleaseTempRenderTargets_RenderThread(0);
    }
    CurRenderTarget = NULL;
    CurRenderTargetSet = 0;
}

bool FGFxRenderer::AddEventHandler(GRendererEventHandler* /*Handler*/)
{
    // GRenderer's own body keeps a list of handlers for device loss; nothing in this tree
    // registers one, and the D3D9 RHI recreates its resources through FRenderResource instead.
    GFXUI_SEAM_TRACE("FGFxRenderer::AddEventHandler");
    return false;
}

void FGFxRenderer::RemoveEventHandler(GRendererEventHandler* /*Handler*/)
{
    GFXUI_SEAM_TRACE("FGFxRenderer::RemoveEventHandler");
}

// ---------------------------------------------------------------------------------------------
// The render-thread state helpers.
// ---------------------------------------------------------------------------------------------

// 2012 0x5d7c80: the render target has to be re-bound before the first draw of a display pass,
// because everything else in the frame has been drawing somewhere else.
void FGFxRenderer::CheckRenderTarget_RenderThread()
{
    if (CurRenderTargetSet || CurRenderTarget == NULL || CurRenderTarget->Resource == NULL)
    {
        return;
    }

    FGFxRenderTargetResource* Resource = CurRenderTarget->Resource;
    const FSurfaceRHIRef& DepthSurface = CurRenderTarget->StencilBuffer
        ? CurRenderTarget->StencilBuffer->DepthSurface : Resource->DepthBuffer;
    RHISetRenderTarget(Resource->ColorBuffer, DepthSurface);
    // DISHONORED(bringup, agent DC): the cached stencil state has to be valid before it is pushed.
    // FD3D9DynamicRHI::SetStencilState dereferences its argument with no null check (the reference
    // engine's own body, so retail's too), and the very first CheckRenderTarget of a frame runs from
    // INSIDE InitUIBlendStackAndMiscRenderState_RenderingThread - before that function has a chance to
    // set anything - so a freshly constructed renderer took the null and the render thread died at
    // SetStencilState+0x18. Retail cannot reach it either; what keeps it out is not in the decompile of
    // 0x5d7c80 or 0x5d7e10, so the guard is here and it is stated as such.
    if (!IsValidRef(CurStencilState))
    {
        CurStencilState = TStaticStencilState<>::GetRHI();
    }
    RHISetStencilState(CurStencilState);

    if (CurRenderTarget->Texture)
    {
        // A temp target starts clean.
        FGFxRenderTargetResource* TargetResource =
            CurRenderTarget->Texture->RenderTarget->Resource;
        RHISetViewport(0, 0, 0.f, TargetResource->SizeX, TargetResource->SizeY, 1.f);
        RHIClear(TRUE, FLinearColor(0.f, 0.f, 0.f, 0.f), FALSE, 0.f,
                 CurRenderTarget->StencilBuffer ? TRUE : FALSE, 0);
    }
    RHISetViewport(ViewRect.xAxis.ViewStart, ViewRect.yAxis.ViewStart, 0.f,
                   ViewRect.xAxis.ViewStart + ViewRect.xAxis.ViewLength,
                   ViewRect.yAxis.ViewStart + ViewRect.yAxis.ViewLength, 1.f);
    FGFxRendererImpl::ApplyUIBlendMode_RenderThread(bAlphaComposite, BlendMode, 0);

    TShaderMapRef<FGFxPixelShader<GFx_PS_TextTexture> > Shader(GetGlobalShaderMap(GRHIShaderPlatform));
    FGFxPixelShaderInterface* PixelShader = Shader->GetShaderInterface();
    PixelShader->SetParameterInverseGamma(PixelShader->GetNativeShader()->GetPixelShader(), 1.f);

    CurRenderTargetSet = 1;
}

// 2012 0x5baf50: the 2D path multiplies the viewport matrix by the object's transform and hands the
// result to the vertex shader as a 4x4; the 3D path composes world, view and perspective first.
void FGFxRenderer::ApplyUITransform_RenderThread(const GMatrix2D& InViewportMatrix,
                                                 const GMatrix2D& TransformMatrix,
                                                 FGFxVertexShaderInterface& VertexShader)
{
    if (!Is3DEnabled)
    {
        GMatrix2D Combined = InViewportMatrix;
        FGFxMatrix2DPrepend(Combined, TransformMatrix);
        const FMatrix NativeMatrix = FGFxMatrix2DToNative(Combined);
        VertexShader.SetParameterTransform(VertexShader.GetNativeShader()->GetVertexShader(),
                                          NativeMatrix);
        return;
    }

    // DISHONORED(bringup): the 3D branch composes the object matrix with the world matrix and then
    // with the cached view-projection, rebuilding the latter when UVPMatricesChanged is set. Nothing
    // reaches it yet (GFxCharacter::SetMatrix3D returns false, agentBC.md 6.9), and retail's
    // GRenderer::Adjust3DMatrixForRT - the render-target correction inside it - is a libgfx body
    // this tree does not have.
    if (UVPMatricesChanged)
    {
        UVPMatricesChanged = 0;
        UVPMatrix = ProjMatrix;
    }
    const FMatrix NativeMatrix = FGFxMatrix2DToNative(TransformMatrix);
    VertexShader.SetParameterTransform(VertexShader.GetNativeShader()->GetVertexShader(),
                                      NativeMatrix);
}

// 2012 0x5bb540: eight sampler states, indexed by sample mode, wrap mode and whether mips are used.
FSamplerStateRHIRef FGFxRenderer::GetSamplerState(GRenderer::BitmapSampleMode SampleMode,
                                                  GRenderer::BitmapWrapMode WrapMode,
                                                  unsigned int bUseMips)
{
    const INT Index = (INT)SampleMode | (2 * (INT)WrapMode) | (bUseMips ? 4 : 0);
    if (!IsValidRef(SamplerStates[Index]))
    {
        FSamplerStateInitializerRHI SamplerInitializer(
            SampleMode != GRenderer::Sample_Linear ? SF_Point : SF_Trilinear,
            WrapMode == GRenderer::Wrap_Clamp ? AM_Clamp : AM_Wrap,
            WrapMode == GRenderer::Wrap_Clamp ? AM_Clamp : AM_Wrap,
            WrapMode == GRenderer::Wrap_Clamp ? AM_Clamp : AM_Wrap,
            bUseMips ? MIPBIAS_None : MIPBIAS_HigherResolution_13);
        SamplerStates[Index] = RHICreateSamplerState(SamplerInitializer);
    }
    return SamplerStates[Index];
}

// 2012 0x5d5580: push the whole view state and start drawing into the given target.
void FGFxRenderer::PushRenderTarget_RenderThread(const GRect<float>& FrameRect,
                                                 GRenderTarget* Target)
{
    FGFxRenderTarget* RT = (FGFxRenderTarget*)Target;
    if (RT == NULL || RT->Texture.pObject == NULL || RT->Texture->RenderTarget == NULL)
    {
        return;
    }
    RT->Texture->AddRef();

    RenderTargetStack.PushBack(RTState(CurRenderTarget, ViewportMatrix, ViewMatrix, ProjMatrix,
                                      WorldMatrix, Is3DEnabled, ViewRect, bAlphaComposite,
                                      CurStencilState));
    CurRenderTarget = RT;
    CurStencilState = TStaticStencilState<>::GetRHI();

    FLOAT Width = FrameRect.Right - FrameRect.Left;
    FLOAT Height = FrameRect.Bottom - FrameRect.Top;
    if (Width < 1.f) Width = 1.f;
    if (Height < 1.f) Height = 1.f;

    appMemzero(&ViewRect, sizeof(ViewRect));
    ViewRect.xAxis.PixelStart = FrameRect.Left;
    ViewRect.xAxis.PixelClipStart = FrameRect.Left;
    ViewRect.xAxis.PixelEnd = FrameRect.Right;
    ViewRect.xAxis.PixelClipLength = Width;
    ViewRect.xAxis.ViewStart = 0;
    ViewRect.xAxis.ViewLength = CurRenderTarget->TargetWidth;
    ViewRect.xAxis.ViewLengthMax = CurRenderTarget->Texture->RenderTarget->Resource->SizeX;
    ViewRect.yAxis.PixelStart = FrameRect.Top;
    ViewRect.yAxis.PixelClipStart = FrameRect.Top;
    ViewRect.yAxis.PixelEnd = FrameRect.Bottom;
    ViewRect.yAxis.PixelClipLength = Height;
    ViewRect.yAxis.ViewStart = 0;
    ViewRect.yAxis.ViewLength = CurRenderTarget->TargetHeight;
    ViewRect.yAxis.ViewLengthMax = CurRenderTarget->Texture->RenderTarget->Resource->SizeY;

    ViewportMatrix.SetIdentity();
    ViewportMatrix.M_[0][0] = 2.f / ViewRect.xAxis.PixelClipLength;
    ViewportMatrix.M_[1][1] = -2.f / ViewRect.yAxis.PixelClipLength;
    const FLOAT XOffset = ViewRect.xAxis.ViewLength > 0
        ? (GPixelCenterOffset * 2.f) / (FLOAT)ViewRect.xAxis.ViewLength : 0.f;
    const FLOAT YOffset = ViewRect.yAxis.ViewLength > 0
        ? (GPixelCenterOffset * 2.f) / (FLOAT)ViewRect.yAxis.ViewLength : 0.f;
    ViewportMatrix.M_[0][2] = -1.f - ViewRect.xAxis.PixelClipStart * ViewportMatrix.M_[0][0] - XOffset;
    ViewportMatrix.M_[1][2] = 1.f - ViewRect.yAxis.PixelClipStart * ViewportMatrix.M_[1][1] + YOffset;

    GMatrix3D View, Persp;
    View.SetIdentity();
    Persp.SetIdentity();
    MakeViewAndPersp3D(FrameRect, View, Persp, 1.f, false);
    ViewMatrix = View;
    ProjMatrix = Persp;
    Is3DEnabled = 0;
    UVPMatricesChanged = 1;
    bAlphaComposite = 1;
    CurRenderTargetSet = 0;
}

// 2012 0x5d58e0: resolve what was drawn, then restore the state the push saved.
void FGFxRenderer::PopRenderTarget_RenderThread()
{
    if (CurRenderTarget != NULL && CurRenderTarget->Resource != NULL)
    {
        RHICopyToResolveTarget(CurRenderTarget->Resource->ColorBuffer, TRUE, FResolveParams());
        if (CurRenderTarget->IsTemp)
        {
            CurRenderTarget->StencilBuffer = (FGFxRenderResources*)NULL;
        }
        if (CurRenderTarget->Texture.pObject != NULL)
        {
            CurRenderTarget->Texture->Release();
        }
    }
    if (RenderTargetStack.GetSize() == 0)
    {
        return;
    }
    // A copy, not a reference: Resize() below destroys the element, and agent CB found that the
    // reconstructed GArray has a destructor and no copy constructor, so nothing here may alias it.
    const RTState State = RenderTargetStack[RenderTargetStack.GetSize() - 1];
    CurRenderTarget = State.pRT;
    ViewportMatrix = State.ViewMatrix;
    ViewRect = State.ViewRect;
    bAlphaComposite = (State.RenderMode != 0) ? 1 : 0;
    CurStencilState = State.StencilState;
    ViewMatrix = State.ViewMatrix3D;
    ProjMatrix = State.PerspMatrix3D;
    WorldMatrix = State.WorldMatrix3D;
    Is3DEnabled = State.Is3DEnabled;
    CurRenderTargetSet = 0;
    UVPMatricesChanged = 1;
    RenderTargetStack.Resize(RenderTargetStack.GetSize() - 1);
}

// 2012 0x5d95c0: reuse a temp target of at least the requested size, or make one, then push it.
// The size ladder retail walks is the power-of-two one below, clamped to MaxTempRTSize.
FGFxTexture* FGFxRenderer::PushTempRenderTarget_RenderThread(FGFxTexture* StubTexture,
                                                             const GRect<float>& FrameRect,
                                                             unsigned int Width,
                                                             unsigned int Height,
                                                             bool bWantStencil)
{
    static const UINT TempRTSizes[] = { 128, 256, 512, 1024, 2048, 4096 };

    UINT RequestedWidth = Width;
    UINT RequestedHeight = Height;
    if (RequestedWidth > MaxTempRTSize || RequestedHeight > MaxTempRTSize)
    {
        if (RequestedWidth <= RequestedHeight)
        {
            RequestedWidth = (UINT)appCeil((FLOAT)(RequestedWidth * MaxTempRTSize)
                                           / (FLOAT)RequestedHeight);
            RequestedHeight = MaxTempRTSize;
        }
        else
        {
            RequestedHeight = (UINT)appCeil((FLOAT)(RequestedHeight * MaxTempRTSize)
                                            / (FLOAT)RequestedWidth);
            RequestedWidth = MaxTempRTSize;
        }
    }

    UINT AllocWidth = 128, AllocHeight = 128;
    for (INT i = 0; i < ARRAY_COUNT(TempRTSizes) && AllocWidth < RequestedWidth; ++i)
    {
        AllocWidth = TempRTSizes[i];
    }
    for (INT i = 0; i < ARRAY_COUNT(TempRTSizes) && AllocHeight < RequestedHeight; ++i)
    {
        AllocHeight = TempRTSizes[i];
    }

    FGFxRenderTarget* RT = NULL;
    UBOOL bNeedsInit = FALSE;
    for (UINT i = 0; i < TempRenderTargets.GetSize(); ++i)
    {
        FGFxRenderTarget* Candidate = TempRenderTargets[i].pObject;
        if (Candidate->Texture.pObject == NULL || Candidate->Texture->RefCount.Value > 1)
        {
            continue;
        }
        FGFxRenderTargetResource* Resource = Candidate->Texture->RenderTarget
            ? Candidate->Texture->RenderTarget->Resource : NULL;
        if (Resource != NULL && (UINT)Resource->SizeX >= RequestedWidth &&
            (UINT)Resource->SizeY >= RequestedHeight)
        {
            RT = Candidate;
            AllocWidth = Resource->SizeX;
            AllocHeight = Resource->SizeY;
            break;
        }
    }
    if (RT == NULL)
    {
        RT = CreateRenderTarget();
        RT->IsTemp = true;
        RT->Texture = StubTexture ? StubTexture : CreateTexture();
        TempRenderTargets.PushBack(GPtr<FGFxRenderTarget>(RT));
        RT->Release();
        bNeedsInit = TRUE;
    }

    FGFxRenderResources* DepthStencil = NULL;
    if (bWantStencil)
    {
        for (UINT i = 0; i < TempStencilBuffers.GetSize(); ++i)
        {
            FGFxRenderResources* Candidate = TempStencilBuffers[i].pObject;
            if (Candidate->GetRefCount() <= 1 && (UINT)Candidate->SizeX >= RequestedWidth &&
                (UINT)Candidate->SizeY >= RequestedHeight)
            {
                DepthStencil = Candidate;
                break;
            }
        }
        if (DepthStencil == NULL)
        {
            DepthStencil = new FGFxRenderResources;
            DepthStencil->SizeX = AllocWidth;
            DepthStencil->SizeY = AllocHeight;
            DepthStencil->InitResource();
            TempStencilBuffers.PushBack(GPtr<FGFxRenderResources>(DepthStencil));
            DepthStencil->Release();
            bNeedsInit = TRUE;
        }
    }

    if (bNeedsInit)
    {
        {
        }
        RT->InitRenderTarget_RenderThread(RT->Texture.pObject, DepthStencil, AllocWidth,
                                         AllocHeight);
        RT->Resource->InverseGamma = 1.f;
        RT->Texture->RenderTarget = RT;
    }
    else
    {
        RT->StencilBuffer = DepthStencil;
    }

    if (StubTexture != NULL && RT->Texture.pObject != StubTexture)
    {
        if (RT->Texture.pObject != NULL)
        {
            RT->Texture->RenderTarget = NULL;
        }
        RT->Texture = StubTexture;
        StubTexture->RenderTarget = RT;
    }

    RT->TargetWidth = RequestedWidth;
    RT->TargetHeight = RequestedHeight;
    PushRenderTarget_RenderThread(FrameRect, RT);
    return RT->Texture.pObject;
}

// 2012 0x5db250: drop every temp target and stencil buffer nothing holds and that is bigger than
// the area the caller asked to keep.
void FGFxRenderer::ReleaseTempRenderTargets_RenderThread(unsigned int KeepArea)
{
    GArray<GPtr<FGFxRenderTarget>, 2> NewTempRenderTargets;
    for (UINT i = 0; i < TempRenderTargets.GetSize(); ++i)
    {
        FGFxRenderTarget* RT = TempRenderTargets[i].pObject;
        FGFxRenderTargetResource* Resource =
            (RT->Texture.pObject && RT->Texture->RenderTarget) ? RT->Texture->RenderTarget->Resource
                                                               : NULL;
        const UBOOL bInUse = RT->Texture.pObject && RT->Texture->RefCount.Value > 1;
        const UBOOL bSmallEnough = Resource != NULL &&
            (UINT)(Resource->SizeX * Resource->SizeY) <= KeepArea;
        if (bInUse || bSmallEnough)
        {
            NewTempRenderTargets.PushBack(TempRenderTargets[i]);
        }
    }
    TempRenderTargets.Clear();
    for (UINT i = 0; i < NewTempRenderTargets.GetSize(); ++i)
    {
        TempRenderTargets.PushBack(NewTempRenderTargets[i]);
    }

    GArray<GPtr<FGFxRenderResources>, 2> NewTempStencilBuffers;
    for (UINT i = 0; i < TempStencilBuffers.GetSize(); ++i)
    {
        FGFxRenderResources* Buffer = TempStencilBuffers[i].pObject;
        if (Buffer->GetRefCount() > 1 || (UINT)(Buffer->SizeX * Buffer->SizeY) <= KeepArea)
        {
            NewTempStencilBuffers.PushBack(TempStencilBuffers[i]);
        }
        else
        {
            Buffer->ReleaseResource();
        }
    }
    TempStencilBuffers.Clear();
    for (UINT i = 0; i < NewTempStencilBuffers.GetSize(); ++i)
    {
        TempStencilBuffers.PushBack(NewTempStencilBuffers[i]);
    }
}

#if DISHONORED_GFXUI_GFX3_RUNTIME
#include "GFxPlayer.h"
#endif

// ---------------------------------------------------------------------------------------------
// DISHONORED(bringup): the acceptance probe of package CC. Two switches, both one-shot, both
// dumping a bitmap rather than a log line:
//
//   -gfxdrawprobe             draw known geometry through the 54-slot interface into a GFx temp
//                             render target and dump it, then print the draw census
//   -gfxmovieprobe=Pkg.Movie  open one cooked movie, advance its first frame, and render that frame
//                             to the same kind of target through BeginDisplay / Display / EndDisplay
//
// Every call below goes through a GRenderer* rather than an FGFxRenderer*, so what is exercised is
// the vtable the runtime sees, not the concrete class - the same rule agent BB's --slots used.
// Drop the pair when the movie player drives the renderer for real.
// ---------------------------------------------------------------------------------------------


namespace
{

// Read the render target back and write it out. The readback has to happen on the render thread,
// which is what UnEngine.cpp's VisualizeTexture path does too.
void FGFxProbeDumpTarget(FGFxRenderTarget* RT, const FString& Name, INT ReadX = 0, INT ReadY = 0)
{
    if (RT == NULL || RT->Resource == NULL || !IsValidRef(RT->Resource->ColorBuffer))
    {
        debugf(NAME_Warning, TEXT("DISHONORED(bringup): GFx probe: no colour surface to dump"));
        return;
    }
    // A temp render target is rounded up to the next size on retail's ladder, so the frame that was
    // drawn is usually a sub-rectangle of it: read that, not the padding.
    const INT SizeX = (ReadX > 0) ? Min<INT>(ReadX, RT->Resource->SizeX) : RT->Resource->SizeX;
    const INT SizeY = (ReadY > 0) ? Min<INT>(ReadY, RT->Resource->SizeY) : RT->Resource->SizeY;
    TArray<BYTE> Bytes;

    struct FGFxProbeReadbackCommand : public FRenderCommand
    {
        FSurfaceRHIRef  Surface;
        INT             SizeX;
        INT             SizeY;
        TArray<BYTE>*   Out;
        FGFxProbeReadbackCommand(FSurfaceRHIParamRef InSurface, INT InSizeX, INT InSizeY,
                                 TArray<BYTE>* InOut)
            : Surface(InSurface), SizeX(InSizeX), SizeY(InSizeY), Out(InOut) {}
        virtual UINT Execute()
        {
            FReadSurfaceDataFlags ReadDataFlags;
            RHIReadSurfaceData(Surface, 0, 0, SizeX - 1, SizeY - 1, *Out, ReadDataFlags);
            return sizeof(*this);
        }
        virtual const TCHAR* DescribeCommand() { return TEXT("FGFxProbeReadbackCommand"); }
    };
    ENQUEUE_RENDER_COMMAND(FGFxProbeReadbackCommand,(RT->Resource->ColorBuffer,SizeX,SizeY,&Bytes));
    FlushRenderingCommands();

    if (Bytes.Num() < SizeX * SizeY * (INT)sizeof(FColor))
    {
        debugf(NAME_Warning,
               TEXT("DISHONORED(bringup): GFx probe: readback gave %d bytes, wanted %d"),
               Bytes.Num(), SizeX * SizeY * (INT)sizeof(FColor));
        return;
    }

    // Count what is not the background, so the log says whether the draws changed pixels even if
    // nobody looks at the image.
    const FColor* Pixels = (const FColor*)&Bytes(0);
    INT NonBackground = 0;
    const FColor Background = Pixels[0];
    for (INT i = 0; i < SizeX * SizeY; ++i)
    {
        if (Pixels[i].DWColor() != Background.DWColor())
        {
            ++NonBackground;
        }
    }

    GFileManager->MakeDirectory(*appScreenShotDir(), TRUE);
    const FString FileName = appScreenShotDir() * Name;
    appCreateBitmap(*FileName, SizeX, SizeY, (FColor*)&Bytes(0), GFileManager);
    debugf(TEXT("DISHONORED(bringup): GFx probe: wrote %s (%dx%d), %d of %d pixels differ from the ")
           TEXT("corner pixel 0x%08x"),
           *FileName, SizeX, SizeY, NonBackground, SizeX * SizeY, Background.DWColor());
}

// A small checkerboard in a transient UTexture2D: the cook feeds the bitmap path exactly this way
// (a Texture2D export named by tag 1009), so the probe uses the same entry point.
UTexture2D* FGFxProbeMakeCheckerTexture()
{
    const INT Size = 16;
    UTexture2D* Texture = ConstructObject<UTexture2D>(UTexture2D::StaticClass(),
                                                      UObject::GetTransientPackage(), NAME_None,
                                                      RF_Transient);
    Texture->NeverStream = TRUE;
    Texture->SRGB = FALSE;
    Texture->Init(Size, Size, PF_A8R8G8B8);
    FColor* Dest = (FColor*)Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
    for (INT y = 0; y < Size; ++y)
    {
        for (INT x = 0; x < Size; ++x)
        {
            const UBOOL bWhite = ((x / 4) + (y / 4)) & 1;
            Dest[y * Size + x] = bWhite ? FColor(255, 255, 255, 255) : FColor(255, 200, 40, 128);
        }
    }
    Texture->Mips(0).Data.Unlock();
    Texture->UpdateResource();
    return Texture;
}

// The movie's background colour: SetBackgroundColor (tag 9, three bytes of RGB) in the tag stream
// agentBB.md 3 documents. GFxMovieRoot keeps the parsed value private, and the container parser does
// not record it, so the probe reads the one tag it needs itself.
GColor FGFxProbeMovieBackground(const BYTE* Data, UINT Size, UINT FirstTagOffset)
{
    GColor Background(0, 0, 0, 255);
    UINT Offset = FirstTagOffset;
    while (Offset + 2 <= Size)
    {
        const UINT Header = Data[Offset] | ((UINT)Data[Offset + 1] << 8);
        Offset += 2;
        const UINT Code = Header >> 6;
        UINT Length = Header & 0x3F;
        if (Length == 0x3F)
        {
            if (Offset + 4 > Size)
            {
                break;
            }
            Length = Data[Offset] | ((UINT)Data[Offset + 1] << 8) | ((UINT)Data[Offset + 2] << 16)
                   | ((UINT)Data[Offset + 3] << 24);
            Offset += 4;
        }
        if (Code == 0 || Offset + Length > Size)
        {
            break;
        }
        if (Code == 9 && Length >= 3)
        {
            Background = GColor(Data[Offset], Data[Offset + 1], Data[Offset + 2], 255);
        }
        Offset += Length;
    }
    return Background;
}

// The texture matrix that maps a pixel rectangle onto the whole texture, which is the form GFx
// hands the renderer for a bitmap fill.
GMatrix2D FGFxProbeFillMatrix(FLOAT X0, FLOAT Y0, FLOAT X1, FLOAT Y1)
{
    GMatrix2D M;
    M.SetIdentity();
    M.M_[0][0] = 1.f / (X1 - X0);
    M.M_[0][2] = -X0 / (X1 - X0);
    M.M_[1][1] = 1.f / (Y1 - Y0);
    M.M_[1][2] = -Y0 / (Y1 - Y0);
    return M;
}

struct FGFxProbeQuad
{
    FGFxVertex_XY16i Vertices[4];
    FGFxProbeQuad(INT X0, INT Y0, INT X1, INT Y1)
    {
        Vertices[0].X = (SWORD)X0; Vertices[0].Y = (SWORD)Y0;
        Vertices[1].X = (SWORD)X1; Vertices[1].Y = (SWORD)Y0;
        Vertices[2].X = (SWORD)X0; Vertices[2].Y = (SWORD)Y1;
        Vertices[3].X = (SWORD)X1; Vertices[3].Y = (SWORD)Y1;
    }
};

const WORD GGFxProbeQuadIndices[6] = { 0, 1, 2, 2, 1, 3 };

} // anonymous namespace

// DISHONORED(bringup): the probe flushes and logs after every stage, so a fault on the render thread
// names the stage that enqueued the command rather than an address.
static void FGFxProbeStage(const TCHAR* Stage)
{
    FlushRenderingCommands();
    debugf(TEXT("DISHONORED(bringup): GFx probe stage: %s"), Stage);
}

// The geometry the probe draws, in the order it draws it. Everything is in the 256x256 pixel space
// BeginDisplay establishes, so the picture is predictable:
//   a dark blue background from BeginDisplay's own background quad
//   a red quad top left            (solid fill, XY16i, indexed tri list)
//   a gouraud strip top right      (XY16iC32, vertex colours)
//   a checkerboard quad bottom left(bitmap fill, the texture path and the texture matrix)
//   a green quad bottom right, clipped to the left half of itself by a stencil mask
//   a white line strip across the middle
//   a row of four glyph quads      (DrawBitmaps, the batched glyph path)
void DishonoredGFxDrawProbe(FViewport* Viewport)
{
    const INT W = 256;
    const INT H = 256;

    FGFxRenderer* Impl = new FGFxRenderer;
    Impl->Viewport = Viewport;
    Impl->RenderTarget = Viewport;
    GRenderer* Renderer = Impl;

    GRenderer::RenderCaps Caps;
    appMemzero(&Caps, sizeof(Caps));
    Renderer->GetRenderCaps(&Caps);
    debugf(TEXT("DISHONORED(bringup): GFx probe: caps 0x%08x vertex formats 0x%x blend modes 0x%x ")
           TEXT("max texture %d"),
           Caps.CapBits, Caps.VertexFormats, Caps.BlendModes, Caps.MaxTextureSize);

    UTexture2D* CheckerTexture = FGFxProbeMakeCheckerTexture();
    FlushRenderingCommands();
    GTexture* GfxTexture = Renderer->CreateTexture();
    ((FGFxTexture*)GfxTexture)->InitTexture(CheckerTexture, false);
    FGFxProbeStage(TEXT("texture"));

    GRect<float> FrameRect;
    FrameRect.Left = 0.f; FrameRect.Top = 0.f;
    FrameRect.Right = (FLOAT)W; FrameRect.Bottom = (FLOAT)H;
    GTexture* TargetTexture = Renderer->PushTempRenderTarget(FrameRect, W, H, true);
    FGFxProbeStage(TEXT("temp render target"));
    FGFxRenderTarget* TargetRT = ((FGFxTexture*)TargetTexture)->RenderTarget;

    GViewport ViewportDesc(W, H, 0, 0, W, H);
    Renderer->BeginDisplay(GColor(32, 40, 56, 255), ViewportDesc, 0.f, (FLOAT)W, 0.f, (FLOAT)H);
    FGFxProbeStage(TEXT("begin display"));

    GMatrix2D Identity;
    Identity.SetIdentity();
    Renderer->SetMatrix(Identity);
    GRenderer::Cxform Cx;
    FGFxCxformSetIdentity(Cx);
    Renderer->SetCxform(Cx);

    // 1. a solid red quad, top left.
    {
        FGFxProbeQuad Quad(16, 16, 112, 112);
        Renderer->FillStyleColor(GColor(220, 48, 48, 255));
        Renderer->SetVertexData(Quad.Vertices, 4, GRenderer::Vertex_XY16i, NULL);
        Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
        Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        FGFxProbeStage(TEXT("solid quad"));
    }

    // 2. a Gouraud quad, top right: four XY16iC32 vertices with their own colours.
    {
        GRenderer::VertexXY16iC32 Verts[4];
        Verts[0].x = 144; Verts[0].y = 16;  Verts[0].Color = 0xFF0000FF;
        Verts[1].x = 240; Verts[1].y = 16;  Verts[1].Color = 0xFF00FF00;
        Verts[2].x = 144; Verts[2].y = 112; Verts[2].Color = 0xFFFF0000;
        Verts[3].x = 240; Verts[3].y = 112; Verts[3].Color = 0xFFFFFFFF;
        Renderer->FillStyleGouraud(GRenderer::GFill_Color, NULL, NULL, NULL);
        Renderer->SetVertexData(Verts, 4, GRenderer::Vertex_XY16iC32, NULL);
        Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
        Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        FGFxProbeStage(TEXT("gouraud quad"));
    }

    // 3. the checkerboard through the bitmap fill, bottom left.
    {
        GRenderer::FillTexture Fill;
        Fill.pTexture = GfxTexture;
        Fill.TextureMatrix = FGFxProbeFillMatrix(16.f, 144.f, 112.f, 240.f);
        Fill.WrapMode = GRenderer::Wrap_Clamp;
        Fill.SampleMode = GRenderer::Sample_Linear;
        FGFxProbeQuad Quad(16, 144, 112, 240);
        Renderer->FillStyleBitmap(&Fill);
        Renderer->SetVertexData(Quad.Vertices, 4, GRenderer::Vertex_XY16i, NULL);
        Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
        Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        FGFxProbeStage(TEXT("bitmap quad"));
    }

    // 4. a mask, then a green quad that only shows where the mask was drawn.
    {
        Renderer->BeginSubmitMask(GRenderer::Mask_Clear);
        FGFxProbeQuad MaskQuad(144, 144, 192, 240);
        Renderer->FillStyleColor(GColor(255, 255, 255, 255));
        Renderer->SetVertexData(MaskQuad.Vertices, 4, GRenderer::Vertex_XY16i, NULL);
        Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
        Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        Renderer->EndSubmitMask();

        FGFxProbeQuad Quad(144, 144, 240, 240);
        Renderer->FillStyleColor(GColor(48, 200, 96, 255));
        Renderer->SetVertexData(Quad.Vertices, 4, GRenderer::Vertex_XY16i, NULL);
        Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
        Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        Renderer->DisableMask();
        FGFxProbeStage(TEXT("mask"));
    }

    // 5. a line strip across the middle.
    {
        FGFxVertex_XY16i Line[5];
        for (INT i = 0; i < 5; ++i)
        {
            Line[i].X = (SWORD)(16 + i * 56);
            Line[i].Y = (SWORD)(128 + ((i & 1) ? 8 : -8));
        }
        Renderer->LineStyleColor(GColor(255, 255, 255, 255));
        Renderer->SetVertexData(Line, 5, GRenderer::Vertex_XY16i, NULL);
        Renderer->DrawLineStrip(0, 4);
        Renderer->LineStyleDisable();
        FGFxProbeStage(TEXT("line strip"));
    }

    // 6. four glyph quads through the batched bitmap path.
    {
        GRenderer::BitmapDesc Descs[4];
        for (INT i = 0; i < 4; ++i)
        {
            Descs[i].Coords.Left = 16.f + i * 24.f;
            Descs[i].Coords.Top = 244.f;
            Descs[i].Coords.Right = Descs[i].Coords.Left + 20.f;
            Descs[i].Coords.Bottom = 252.f;
            Descs[i].TextureCoords.Left = 0.f;
            Descs[i].TextureCoords.Top = 0.f;
            Descs[i].TextureCoords.Right = 1.f;
            Descs[i].TextureCoords.Bottom = 1.f;
            Descs[i].Color = GColor(255, 255, 255, 255);
        }
        Renderer->DrawBitmaps(Descs, 4, 0, 4, GfxTexture, Identity, NULL);
        FGFxProbeStage(TEXT("glyph batch"));
    }

    Renderer->EndDisplay();
    FGFxProbeStage(TEXT("end display"));
    Renderer->PopRenderTarget();
    FGFxProbeStage(TEXT("pop render target"));

    GRenderer::Stats Stats;
    appMemzero(&Stats, sizeof(Stats));
    Renderer->GetRenderStats(&Stats, false);
    debugf(TEXT("DISHONORED(bringup): GFx probe: stats %u triangles, %u lines, %u primitives, ")
           TEXT("%u masks, %u filters"),
           Stats.Triangles, Stats.Lines, Stats.Primitives, Stats.Masks, Stats.Filters);

    FGFxProbeDumpTarget(TargetRT, TEXT("agentCC_geometry"));

    ANSICHAR Census[4096];
    FGFxSeamCensus(Census, ARRAY_COUNT(Census));
    debugf(TEXT("%s"), ANSI_TO_TCHAR(Census));

    GfxTexture->Release();
    Renderer->ReleaseResources();
    FlushRenderingCommands();
    Renderer->Release();
}

// The movie probe: one cooked payload, its first frame, through the same renderer into the same kind
// of target. What the movie contributes to the image today is its background over its own frame
// rect, because the shape, text and image character definitions are the tag loaders package CD is
// porting: this prints the display-list count and the read statistics so the report can say exactly
// how much of the frame is there.
void DishonoredGFxMovieProbe(FViewport* Viewport, const FString& MovieName)
{
    // The cooked movies live inside packages whose file name is not their object path -
    // Dishonored_MainMenu.upk holds UI_MainMenu.MainMenu (agentBB.md 3) - so a path that does not
    // resolve falls back to a search of what the map has already loaded, by full path then by name.
    USwfMovie* MovieAsset = LoadObject<USwfMovie>(NULL, *MovieName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
    if (MovieAsset == NULL)
    {
        // The switch may name a package file rather than an object: Dishonored_MainMenu.upk holds
        // UI_MainMenu.MainMenu, and the cooked UI packages are not loaded until the menu opens one.
        UObject::LoadPackage(NULL, *MovieName, LOAD_NoWarn | LOAD_Quiet);
        for (TObjectIterator<USwfMovie> It; It; ++It)
        {
            if (It->GetPathName() == MovieName || It->GetName() == MovieName ||
                It->GetOutermost()->GetName() == MovieName)
            {
                MovieAsset = *It;
                break;
            }
        }
    }
    if (MovieAsset == NULL)
    {
        // Last resort: whatever cooked movie the map did load, so the probe still renders a frame.
        for (TObjectIterator<USwfMovie> It; It; ++It)
        {
            if (It->RawData.Num() > 0)
            {
                MovieAsset = *It;
                debugf(TEXT("DISHONORED(bringup): GFx movie probe: '%s' not found, using %s"),
                       *MovieName, *MovieAsset->GetPathName());
                break;
            }
        }
    }
    if (MovieAsset == NULL || MovieAsset->RawData.Num() == 0)
    {
        INT Loaded = 0;
        for (TObjectIterator<USwfMovie> It; It; ++It)
        {
            if (It->RawData.Num() > 0)
            {
                debugf(TEXT("DISHONORED(bringup): GFx movie probe: loaded movie %s (%d bytes)"),
                       *It->GetPathName(), It->RawData.Num());
                ++Loaded;
            }
        }
        debugf(NAME_Warning, TEXT("DISHONORED(bringup): GFx movie probe: '%s' did not load; %d ")
               TEXT("cooked movies are loaded"), *MovieName, Loaded);
        return;
    }
    debugf(TEXT("DISHONORED(bringup): GFx movie probe: %s, %d bytes of cooked payload"),
           *MovieAsset->GetPathName(), MovieAsset->RawData.Num());

    // The container, through agent BB's parser: the header, the frame rect, the symbol table and the
    // external-image table. The movie's characters are read by agent BC's GFxMovieDataDef, which this
    // unit cannot instantiate yet - see the note at the end of this function.
    GFxGfxFileInfo* Info = new GFxGfxFileInfo;
    if (!GFxGfxParseFile(&MovieAsset->RawData(0), MovieAsset->RawData.Num(), *Info))
    {
        debugf(NAME_Warning, TEXT("DISHONORED(bringup): GFx movie probe: parse failed: %s"),
               ANSI_TO_TCHAR(Info->Error));
        delete Info;
        return;
    }

    const INT W = Max<INT>(1, (INT)Info->FrameWidthPixels);
    const INT H = Max<INT>(1, (INT)Info->FrameHeightPixels);
    const GColor Background = FGFxProbeMovieBackground(&MovieAsset->RawData(0),
                                                      MovieAsset->RawData.Num(),
                                                      Info->FirstTagOffset);

    FGFxRenderer* Impl = new FGFxRenderer;
    Impl->Viewport = Viewport;
    Impl->RenderTarget = Viewport;
    // The renderer's default temp-target cap is retail's 1024 (FGFxRenderer's constructor); a 720p
    // movie frame needs the next size up, and the movie player is what sets this in retail.
    Impl->MaxTempRTSize = 2048;
    GRenderer* Renderer = Impl;

    GRect<float> FrameRect;
    FrameRect.Left = 0.f; FrameRect.Top = 0.f;
    FrameRect.Right = (FLOAT)W; FrameRect.Bottom = (FLOAT)H;
    GTexture* TargetTexture = Renderer->PushTempRenderTarget(FrameRect, W, H, true);
    FlushRenderingCommands();
    FGFxRenderTarget* TargetRT = ((FGFxTexture*)TargetTexture)->RenderTarget;

    // The movie's first frame, through the display sequence the runtime uses: the viewport the movie
    // declares, its own background colour, then whatever the frame's characters submit.
    GViewport ViewportDesc(W, H, 0, 0, W, H);
    Renderer->BeginDisplay(Background, ViewportDesc, 0.f, (FLOAT)W, 0.f, (FLOAT)H);

#if DISHONORED_GFXUI_GFX3_RUNTIME
    // The call site the frame's characters come through, ready for the commit that turns the runtime
    // on. It is compiled out today for one reason and it is a link error, not a design one: pulling
    // gfx3.lib's GFxPlayerRoot.cpp into this module collides with agent BE's stand-in bodies in
    // gfxuigfx3absent.cpp (28 LNK2005, measured), and DISHONORED_GFXUI_GFX3_RUNTIME=1 is what drops
    // that file. Everything below is agent BC's API as GFx3Run drives it.
    GFxMovieDataDef* DataDef = new GFxMovieDataDef;
    if (DataDef->Read(&MovieAsset->RawData(0), MovieAsset->RawData.Num()))
    {
        GFxMovieDefImpl* DefImpl = new GFxMovieDefImpl(DataDef);
        GFxMovieDef::MemoryParams MemParams;
        GFxMovieView* View = DefImpl->CreateInstance(MemParams, false);
        GFxMovieRoot* Root = (GFxMovieRoot*)View;
        Root->SetViewport(ViewportDesc);
        Root->Advance(1.f / (Info->FrameRate > 0.f ? Info->FrameRate : 30.f), 0);
        Root->Display();
        debugf(TEXT("DISHONORED(bringup): GFx movie probe: frame %u of %u, display list %u"),
               Root->GetCurrentFrame() + 1, Root->GetLevel0()->GetFrameCount(),
               Root->GetLevel0()->GetDisplayList().GetCount());
        delete View;
        delete DefImpl;
    }
    delete DataDef;
#endif

    // And the one piece of the frame's real content the renderer can reach without the character
    // definitions: the movie's own cooked bitmaps. Each tag-1009 entry names a Texture2D export in
    // the same package (agentBB.md 3.3); they are laid out here left to right, each scaled into a
    // cell, as a check of the texture path against real cooked data - NOT as the frame's layout,
    // which needs the shape and sub-image characters package CD is porting.
    UINT ImagesDrawn = 0;
    if (Info->ImageCount > 0)
    {
        const FString PackageName = MovieAsset->GetOutermost()->GetName();
        GMatrix2D Identity;
        Identity.SetIdentity();
        Renderer->SetMatrix(Identity);
        FLOAT PenX = 8.f;
        FLOAT PenY = 8.f;
        FLOAT RowHeight = 0.f;
        for (UINT i = 0; i < Info->ImageCount && ImagesDrawn < 64; ++i)
        {
            // The Texture2D export name, measured from the cook: tag 1009's ExportName is empty
            // for all but the handful of images the artist named, and the rest are the TGA file
            // name without its extension (Global_I2.tga -> Global_I2). A named one can carry the
            // exporter's own suffix after a space ("gl_msgBox_bkgd -nopack"), which is not part of
            // the object name. Recorded in agentCC.md for the image loader.
            FString ExportName = ANSI_TO_TCHAR(Info->Images[i].ExportName);
            if (ExportName.Len() == 0)
            {
                ExportName = ANSI_TO_TCHAR(Info->Images[i].FileName);
                const INT Slash = Max<INT>(ExportName.InStr(TEXT("\\"), TRUE),
                                           ExportName.InStr(TEXT("/"), TRUE));
                if (Slash >= 0)
                {
                    ExportName = ExportName.Mid(Slash + 1);
                }
                const INT Dot = ExportName.InStr(TEXT("."), TRUE);
                if (Dot >= 0)
                {
                    ExportName = ExportName.Left(Dot);
                }
            }
            const INT Space = ExportName.InStr(TEXT(" "));
            if (Space >= 0)
            {
                ExportName = ExportName.Left(Space);
            }
            if (ExportName.Len() == 0)
            {
                continue;
            }
            UTexture2D* Texture = LoadObject<UTexture2D>(
                NULL, *(PackageName + TEXT(".") + ExportName), NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
            if (Texture == NULL)
            {
                continue;
            }
            // Scaled into a cell rather than drawn at native size: a 1024-pixel atlas would
            // otherwise fill the frame by itself. The aspect ratio is kept.
            const FLOAT CellW = (FLOAT)W / 8.f - 8.f;
            const FLOAT CellH = (FLOAT)H / 6.f - 8.f;
            const FLOAT Scale = Min<FLOAT>(Min<FLOAT>(CellW / (FLOAT)Texture->SizeX,
                                                      CellH / (FLOAT)Texture->SizeY), 1.f);
            const FLOAT Width = Max<FLOAT>(4.f, Texture->SizeX * Scale);
            const FLOAT Height = Max<FLOAT>(4.f, Texture->SizeY * Scale);
            if (PenX + Width > (FLOAT)W)
            {
                PenX = 8.f;
                PenY += RowHeight + 8.f;
                RowHeight = 0.f;
            }
            if (PenY + Height > (FLOAT)H)
            {
                continue;
            }
            GTexture* GfxTexture = Renderer->CreateTexture();
            ((FGFxTexture*)GfxTexture)->InitTexture(Texture, false);

            GRenderer::FillTexture Fill;
            Fill.pTexture = GfxTexture;
            Fill.TextureMatrix = FGFxProbeFillMatrix(PenX, PenY, PenX + Width, PenY + Height);
            Fill.WrapMode = GRenderer::Wrap_Clamp;
            Fill.SampleMode = GRenderer::Sample_Linear;
            FGFxProbeQuad Quad((INT)PenX, (INT)PenY, (INT)(PenX + Width), (INT)(PenY + Height));
            Renderer->FillStyleBitmap(&Fill);
            Renderer->SetVertexData(Quad.Vertices, 4, GRenderer::Vertex_XY16i, NULL);
            Renderer->SetIndexData(GGFxProbeQuadIndices, 6, GRenderer::Index_16, NULL);
            Renderer->DrawIndexedTriList(0, 0, 4, 0, 2);
            GfxTexture->Release();

            PenX += Width + 8.f;
            RowHeight = Max<FLOAT>(RowHeight, Height);
            ++ImagesDrawn;
        }
    }

    Renderer->EndDisplay();
    Renderer->PopRenderTarget();
    FlushRenderingCommands();

    debugf(TEXT("DISHONORED(bringup): GFx movie probe: %s %c%c%c v%u, %.0fx%.0f px, %.1f fps, ")
           TEXT("%u frames, first tag at %u, %u tags, background 0x%08x, %u exports, %u imports, ")
           TEXT("%u external images (%u drawn), %u sub-images"),
           *MovieAsset->GetPathName(), Info->Signature[0], Info->Signature[1],
           Info->Signature[2],
           (UINT)Info->Version, Info->FrameWidthPixels, Info->FrameHeightPixels, Info->FrameRate,
           Info->FrameCount, Info->FirstTagOffset, Info->TagCount, Background.Raw,
           Info->ExportCount, Info->ImportCount, Info->ImageCount, ImagesDrawn,
           Info->SubImageCount);

    // DISHONORED(bringup): what this frame is missing, measured rather than guessed. Instantiating
    // agent BC's player here (GFxMovieDataDef -> GFxMovieDefImpl -> GFxMovieRoot::Advance/Display)
    // pulls gfx3.lib's GFxPlayerRoot.cpp into the link, and every GFxValue::ObjectInterface method in
    // it collides with agent BE's stand-in bodies in gfxuigfx3absent.cpp: 28 LNK2005, measured. That
    // switch - DISHONORED_GFXUI_GFX3_RUNTIME - has to flip in the same commit that lets this module
    // call the runtime; agentCC.md records it for the coordinator. Until then the frame is its
    // background and its own bitmaps, and GFxMovieRoot::Display draws nothing anyway (agentBC.md 6.9)
    // because the shape, text and image character definitions are package CD's tag loaders.
    debugf(TEXT("DISHONORED(bringup): GFx movie probe: the frame's characters are not drawn: ")
           TEXT("GFxMovieRoot::Display is empty until the tag loaders land (package CD), and this ")
           TEXT("unit cannot link the player while gfxuigfx3absent.cpp is compiled"));

    FGFxProbeDumpTarget(TargetRT, TEXT("agentCC_movieframe"), W, H);

    ANSICHAR Census[4096];
    FGFxSeamCensus(Census, ARRAY_COUNT(Census));
    debugf(TEXT("%s"), ANSI_TO_TCHAR(Census));

    Renderer->ReleaseResources();
    FlushRenderingCommands();
    Renderer->Release();
    delete Info;
}

// The one entry point the engine calls, once, on the frame after the RHI and the viewport are up.
void DishonoredGFxRenderProbe(FViewport* Viewport)
{
    static UBOOL bDone = FALSE;
    if (bDone || Viewport == NULL || GUsingNullRHI)
    {
        return;
    }

    // DISHONORED(bringup): a function-local static, not a file-scope one. A file-scope
    // ParseParam(appCmdLine(), ...) in a static library runs its dynamic initialiser before the
    // entry point, while the command line is still empty, so the switch is always FALSE - agent CA
    // found three dead switches that way. The reference engine's own idiom is this one
    // (UnAnimPlay.cpp:583).
    static const UBOOL bDrawProbe = ParseParam(appCmdLine(), TEXT("gfxdrawprobe"));
    static FString MovieName;
    static const UBOOL bMovieProbe = Parse(appCmdLine(), TEXT("gfxmovieprobe="), MovieName);
    if (!bDrawProbe && !bMovieProbe)
    {
        return;
    }
    bDone = TRUE;

    if (bDrawProbe)
    {
        FGFxSeamReset();
        DishonoredGFxDrawProbe(Viewport);
    }
    if (bMovieProbe)
    {
        FGFxSeamReset();
        DishonoredGFxMovieProbe(Viewport, MovieName);
    }
}
