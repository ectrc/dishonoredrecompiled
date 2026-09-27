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
// DISHONORED(port): the bodies of the renderer seam. Declarations, slot numbers and 2013 rvas are
// in gfxuirenderer.h. Every one of GRenderer's 54 slots, GTexture's 12 + 5 and GRenderTarget's
// 7 + 3 is defined here, so the seam instantiates, links and can be handed to the runtime.
//
// The bodies are DISHONORED(bringup): they record the call in the seam census and return a neutral
// value. That is deliberate and it is what package BB's acceptance asks for - the drawing itself is
// 221 functions of RHI work against Dishonored's own shader set, which is package BD's material and
// a later wave's. What this file buys now is that nothing on the GFx side can fail for want of an
// implementation, and that a run can report exactly which slots the runtime reached.
#include "gfxuirendererimpl.h"

#include <string.h>
#include <stdio.h>

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

void FGFxSeamNote(const char* Slot)
{
    ++GSeamTotalCalls;
    for (unsigned int i = 0; i < GSeamSlotCount; ++i)
    {
        if (GSeamSlots[i].Name == Slot || strcmp(GSeamSlots[i].Name, Slot) == 0)
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
        if (strcmp(GSeamSlots[i].Name, Slot) == 0)
            return GSeamSlots[i].Calls;
    return 0;
}

unsigned int FGFxSeamSlotsTouched() { return GSeamSlotCount; }
unsigned int FGFxSeamTotalCalls() { return GSeamTotalCalls; }

void FGFxSeamReset()
{
    GSeamSlotCount = 0;
    GSeamTotalCalls = 0;
}

unsigned int FGFxSeamCensus(char* Out, unsigned int Capacity)
{
    if (Out == 0 || Capacity == 0) return 0;
    int n = _snprintf(Out, Capacity - 1,
                      "DISHONORED(bringup): GFx seam census: %u slots touched, %u calls",
                      GSeamSlotCount, GSeamTotalCalls);
    if (n < 0) n = 0;
    unsigned int used = (unsigned int)n;
    for (unsigned int i = 0; i < GSeamSlotCount && used + 1 < Capacity; ++i)
    {
        int k = _snprintf(Out + used, Capacity - 1 - used, ", %s %u",
                          GSeamSlots[i].Name, GSeamSlots[i].Calls);
        if (k < 0) break;
        used += (unsigned int)k;
    }
    Out[used < Capacity ? used : Capacity - 1] = 0;
    return used;
}

// ---------------------------------------------------------------------------------------------
// FGFxRendererImpl::ConvertFromUI - 2013 0x572c30, byte-identical to 2012 0x5b7710. The only real
// port in this file: it is pure logic over two PDB layouts.
// ---------------------------------------------------------------------------------------------
void FGFxRendererImpl::ConvertFromUI(const GRenderer::FillTexture& In, FFillTextureInfo& Out)
{
    Out.Texture = 0;                        // set by the caller from In.pTexture's engine texture
    Out.TextureMatrix = In.TextureMatrix;
    Out.WrapMode = In.WrapMode;
    Out.SampleMode = In.SampleMode;
    Out.bUseMips = 0;
    GFXUI_SEAM_TRACE("FGFxRendererImpl::ConvertFromUI");
}

// ---------------------------------------------------------------------------------------------
// FGFxTexture
// ---------------------------------------------------------------------------------------------
FGFxTexture::FGFxTexture(GRenderer* InRenderer)
    : Renderer(InRenderer), Texture(0), Texture2D(0), RenderTarget(0)
{
    GFXUI_SEAM_TRACE("FGFxTexture::FGFxTexture");
}

FGFxTexture::~FGFxTexture() { GFXUI_SEAM_TRACE("FGFxTexture::~FGFxTexture"); }

bool FGFxTexture::InitTexture(GImageBase* Image, unsigned int Usage)
{
    (void)Image; (void)Usage;
    GFXUI_SEAM_TRACE("FGFxTexture::InitTexture(GImageBase*)");
    return false;
}

bool FGFxTexture::InitDynamicTexture(int Width, int Height, GImageBase::ImageFormat Format,
                                     int Mipmaps, unsigned int Usage)
{
    (void)Width; (void)Height; (void)Format; (void)Mipmaps; (void)Usage;
    GFXUI_SEAM_TRACE("FGFxTexture::InitDynamicTexture");
    return false;
}

void FGFxTexture::Update(int Level, int NumRects, const GTexture::UpdateRect* Rects,
                         const GImageBase* Image)
{
    (void)Level; (void)NumRects; (void)Rects; (void)Image;
    GFXUI_SEAM_TRACE("FGFxTexture::Update");
}

int FGFxTexture::Map(int Level, int NumRects, GTexture::MapRect* Maps, int Flags)
{
    (void)Level; (void)NumRects; (void)Maps; (void)Flags;
    GFXUI_SEAM_TRACE("FGFxTexture::Map");
    return 0;
}

bool FGFxTexture::Unmap(int Level, int NumRects, GTexture::MapRect* Maps, int Flags)
{
    (void)Level; (void)NumRects; (void)Maps; (void)Flags;
    GFXUI_SEAM_TRACE("FGFxTexture::Unmap");
    return false;
}

GRenderer* FGFxTexture::GetRenderer() const { return Renderer; }

bool FGFxTexture::IsDataValid() const
{
    // 2012 0x5bf620: retail returns whether the engine texture is resident.
    return Texture != 0;
}

void* FGFxTexture::GetUserData() const { return 0; }
void FGFxTexture::SetUserData(void* Data) { (void)Data; }

void FGFxTexture::AddChangeHandler(GTexture::ChangeHandler* Handler)
{
    (void)Handler;
    GFXUI_SEAM_TRACE("FGFxTexture::AddChangeHandler");
}

void FGFxTexture::RemoveChangeHandler(GTexture::ChangeHandler* Handler)
{
    (void)Handler;
    GFXUI_SEAM_TRACE("FGFxTexture::RemoveChangeHandler");
}

bool FGFxTexture::InitTexture(UTexture* InTexture, bool bAsRenderTarget)
{
    // 2013 0x580810: the path the stripped-bitmap substitution takes - the tag-1009 export name is
    // resolved to a package Texture2D and handed here (agentBB.md).
    (void)bAsRenderTarget;
    Texture = InTexture;
    GFXUI_SEAM_TRACE("FGFxTexture::InitTexture(UTexture*)");
    return InTexture != 0;
}

bool FGFxTexture::InitTextureFromFile(const char* FileName)
{
    (void)FileName;
    GFXUI_SEAM_TRACE("FGFxTexture::InitTextureFromFile");
    return false;
}

int FGFxTexture::IsYUVTexture() const { return 0; }

void FGFxTexture::Bind(int Stage, FGFxPixelShaderInterface& Shader,
                       GRenderer::BitmapWrapMode WrapMode,
                       GRenderer::BitmapSampleMode SampleMode, bool bUseMips) const
{
    (void)Stage; (void)Shader; (void)WrapMode; (void)SampleMode; (void)bUseMips;
    GFXUI_SEAM_TRACE("FGFxTexture::Bind");
}

void FGFxTexture::InternalTermGCState() { GFXUI_SEAM_TRACE("FGFxTexture::InternalTermGCState"); }

// ---------------------------------------------------------------------------------------------
// FGFxRenderTarget
// ---------------------------------------------------------------------------------------------
FGFxRenderTarget::FGFxRenderTarget(GRenderer* InRenderer)
    : Renderer(InRenderer), Resource(0), TargetWidth(0), TargetHeight(0), IsTemp(false)
{
    GFXUI_SEAM_TRACE("FGFxRenderTarget::FGFxRenderTarget");
}

FGFxRenderTarget::~FGFxRenderTarget() { GFXUI_SEAM_TRACE("FGFxRenderTarget::~FGFxRenderTarget"); }

bool FGFxRenderTarget::InitRenderTarget(GTexture* Color, GTexture* DepthStencil, GTexture* Resolve)
{
    (void)Color; (void)DepthStencil; (void)Resolve;
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget(GTexture*)");
    return false;
}

GRenderer* FGFxRenderTarget::GetRenderer() const { return Renderer; }
void* FGFxRenderTarget::GetUserData() const { return 0; }
void FGFxRenderTarget::SetUserData(void* Data) { (void)Data; }

void FGFxRenderTarget::AddChangeHandler(GTexture::ChangeHandler* Handler) { (void)Handler; }
void FGFxRenderTarget::RemoveChangeHandler(GTexture::ChangeHandler* Handler) { (void)Handler; }

bool FGFxRenderTarget::InitRenderTarget(const FGFxRenderTargetResource& Native)
{
    (void)Native;
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget(Native)");
    return false;
}

bool FGFxRenderTarget::InitRenderTarget_RenderThread(GTexture* Color,
                                                     FGFxRenderResources* Stencil,
                                                     unsigned int Width, unsigned int Height)
{
    (void)Color; (void)Stencil;
    TargetWidth = Width;
    TargetHeight = Height;
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget_RenderThread(GTexture*)");
    return false;
}

bool FGFxRenderTarget::InitRenderTarget_RenderThread(const FGFxRenderTargetResource& Native)
{
    (void)Native;
    GFXUI_SEAM_TRACE("FGFxRenderTarget::InitRenderTarget_RenderThread(Native)");
    return false;
}

bool FGFxRenderTarget::AdjustBounds(float* Width, float* Height)
{
    // 2012 0x5b7cd0: clamp a requested temp-target size to the target this render target holds.
    if (Width == 0 || Height == 0) return false;
    if (TargetWidth == 0 || TargetHeight == 0) return false;
    if (*Width > (float)TargetWidth) *Width = (float)TargetWidth;
    if (*Height > (float)TargetHeight) *Height = (float)TargetHeight;
    return true;
}

// ---------------------------------------------------------------------------------------------
// FGFxRenderer - all 54 GRenderer slots.
// ---------------------------------------------------------------------------------------------
FGFxRenderer::FGFxRenderer()
    : Viewport(0), RenderTarget(0), RenderMode(0), InverseGamma(1.0f),
      UVPMatricesChanged(0), Is3DEnabled(0), CurRenderTarget(0), CurRenderTargetSet(0),
      BlendMode(GRenderer::Blend_None), bAlphaComposite(0), MaxTempRTSize(0), StencilCounter(0)
{
    memset(&RenderStats, 0, sizeof(RenderStats));
    GFXUI_SEAM_TRACE("FGFxRenderer::FGFxRenderer");
}

FGFxRenderer::~FGFxRenderer() { GFXUI_SEAM_TRACE("FGFxRenderer::~FGFxRenderer"); }

void FGFxRenderer::ScopedEventCallback(const char* Name)
{
    (void)Name;
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

bool FGFxRenderer::GetRenderCaps(GRenderer::RenderCaps* Caps)
{
    // 2013 0x572e00. The values retail reports for the D3D9 RHI: every vertex and index format the
    // player uses, every blend mode, and a 2048 maximum texture size. Reported for real because the
    // runtime branches on them before it ever calls a drawing slot.
    GFXUI_SEAM_TRACE("FGFxRenderer::GetRenderCaps");
    if (Caps == 0) return false;
    Caps->CapBits = GRenderer::Cap_Index16 | GRenderer::Cap_FillGouraud
                  | GRenderer::Cap_FillGouraudTex | GRenderer::Cap_CxformAdd
                  | GRenderer::Cap_NestedMasks | GRenderer::Cap_RenderTargets;
    Caps->VertexFormats = GRenderer::Vertex_XY16i | GRenderer::Vertex_XY32f
                        | GRenderer::Vertex_XY16iC32 | GRenderer::Vertex_XY16iCF32;
    Caps->BlendModes = 0xFFFFFFFFu;
    Caps->MaxTextureSize = 2048;
    return true;
}

FGFxTexture* FGFxRenderer::CreateTexture()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateTexture");
    return new FGFxTexture(this);
}

FGFxTexture* FGFxRenderer::CreateTextureYUV()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateTextureYUV");
    return 0;
}

void FGFxRenderer::BeginFrame() { GFXUI_SEAM_TRACE("FGFxRenderer::BeginFrame"); }
void FGFxRenderer::EndFrame() { GFXUI_SEAM_TRACE("FGFxRenderer::EndFrame"); }

FGFxRenderTarget* FGFxRenderer::CreateRenderTarget()
{
    GFXUI_SEAM_TRACE("FGFxRenderer::CreateRenderTarget");
    return new FGFxRenderTarget(this);
}

void FGFxRenderer::SetDisplayRenderTarget(GRenderTarget* Target, bool bSetState)
{
    (void)bSetState;
    CurRenderTarget = (FGFxRenderTarget*)Target;
    CurRenderTargetSet = Target != 0;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetDisplayRenderTarget");
}

void FGFxRenderer::PushRenderTarget(const GRect<float>& FrameRect, GRenderTarget* Target)
{
    (void)FrameRect; (void)Target;
    GFXUI_SEAM_TRACE("FGFxRenderer::PushRenderTarget");
}

void FGFxRenderer::PopRenderTarget() { GFXUI_SEAM_TRACE("FGFxRenderer::PopRenderTarget"); }

FGFxTexture* FGFxRenderer::PushTempRenderTarget(const GRect<float>& FrameRect, unsigned int Width,
                                                unsigned int Height, bool bWantStencil)
{
    (void)FrameRect; (void)Width; (void)Height; (void)bWantStencil;
    GFXUI_SEAM_TRACE("FGFxRenderer::PushTempRenderTarget");
    return 0;
}

void FGFxRenderer::ReleaseTempRenderTargets(unsigned int KeepArea)
{
    (void)KeepArea;
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseTempRenderTargets");
}

void FGFxRenderer::BeginDisplay(GColor BackgroundColor, const GViewport& InViewport,
                                float x0, float x1, float y0, float y1)
{
    // 2013 0x59dd90. The viewport matrix is the one piece of BeginDisplay that is pure arithmetic
    // and that the player reads back through SetMatrix, so it is computed for real.
    (void)BackgroundColor;
    GFXUI_SEAM_TRACE("FGFxRenderer::BeginDisplay");
    const float dx = (x1 - x0) != 0.0f ? (x1 - x0) : 1.0f;
    const float dy = (y1 - y0) != 0.0f ? (y1 - y0) : 1.0f;
    ViewportMatrix.SetIdentity();
    ViewportMatrix.M_[0][0] = 2.0f / dx;
    ViewportMatrix.M_[1][1] = -2.0f / dy;
    ViewportMatrix.M_[0][2] = -1.0f - ViewportMatrix.M_[0][0] * x0;
    ViewportMatrix.M_[1][2] = 1.0f - ViewportMatrix.M_[1][1] * y0;
    CurrentMatrix = ViewportMatrix;
    RenderMode = InViewport.Flags;
}

void FGFxRenderer::EndDisplay() { GFXUI_SEAM_TRACE("FGFxRenderer::EndDisplay"); }

void FGFxRenderer::SetMatrix(const GMatrix2D& Matrix)
{
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

void FGFxRenderer::PushBlendMode(GRenderer::BlendType Mode)
{
    BlendMode = Mode;
    GFXUI_SEAM_TRACE("FGFxRenderer::PushBlendMode");
}

void FGFxRenderer::PopBlendMode() { GFXUI_SEAM_TRACE("FGFxRenderer::PopBlendMode"); }

bool FGFxRenderer::PushUserData(GRenderer::UserData* Data)
{
    (void)Data;
    GFXUI_SEAM_TRACE("FGFxRenderer::PushUserData");
    return false;
}

void FGFxRenderer::PopUserData() { GFXUI_SEAM_TRACE("FGFxRenderer::PopUserData"); }

void FGFxRenderer::SetPerspective3D(const GMatrix3D& Persp)
{
    ProjMatrix = Persp;
    UVPMatricesChanged = 1;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetPerspective3D");
}

void FGFxRenderer::SetView3D(const GMatrix3D& View)
{
    ViewMatrix = View;
    UVPMatricesChanged = 1;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetView3D");
}

void FGFxRenderer::SetWorld3D(const GMatrix3D* World)
{
    if (World) { WorldMatrix = *World; Is3DEnabled = 1; }
    else { WorldMatrix.SetIdentity(); Is3DEnabled = 0; }
    UVPMatricesChanged = 1;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetWorld3D");
}

void FGFxRenderer::MakeViewAndPersp3D(const GRect<float>& FrameRect, GMatrix3D& View,
                                      GMatrix3D& Persp, float FovY, bool bInvertY)
{
    (void)FrameRect; (void)FovY; (void)bInvertY;
    View.SetIdentity();
    Persp.SetIdentity();
    GFXUI_SEAM_TRACE("FGFxRenderer::MakeViewAndPersp3D");
}

void FGFxRenderer::SetStereoParams(GRenderer::StereoParams Params)
{
    S3DParams = Params;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetStereoParams");
}

void FGFxRenderer::SetStereoDisplay(GRenderer::StereoDisplay Display, bool bSet)
{
    (void)bSet;
    S3DDisplay = Display;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetStereoDisplay");
}

void FGFxRenderer::SetVertexData(const void* Vertices, int NumVertices,
                                 GRenderer::VertexFormat Format, GRenderer::CacheProvider* Cache)
{
    (void)Vertices; (void)NumVertices; (void)Format; (void)Cache;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetVertexData");
}

void FGFxRenderer::SetIndexData(const void* Indices, int NumIndices,
                                GRenderer::IndexFormat Format, GRenderer::CacheProvider* Cache)
{
    (void)Indices; (void)NumIndices; (void)Format; (void)Cache;
    GFXUI_SEAM_TRACE("FGFxRenderer::SetIndexData");
}

void FGFxRenderer::ReleaseCachedData(GRenderer::CachedData* Data, GRenderer::CachedDataType Type)
{
    (void)Data; (void)Type;
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseCachedData");
}

void FGFxRenderer::DrawIndexedTriList(int BaseVertexIndex, int MinVertexIndex, int NumVertices,
                                      int StartIndex, int TriangleCount)
{
    (void)BaseVertexIndex; (void)MinVertexIndex; (void)NumVertices; (void)StartIndex;
    RenderStats.Triangles += (unsigned int)(TriangleCount > 0 ? TriangleCount : 0);
    ++RenderStats.Primitives;
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawIndexedTriList");
}

void FGFxRenderer::DrawLineStrip(int BaseVertexIndex, int LineCount)
{
    (void)BaseVertexIndex;
    RenderStats.Lines += (unsigned int)(LineCount > 0 ? LineCount : 0);
    ++RenderStats.Primitives;
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawLineStrip");
}

void FGFxRenderer::LineStyleDisable() { GFXUI_SEAM_TRACE("FGFxRenderer::LineStyleDisable"); }

void FGFxRenderer::LineStyleColor(GColor Color)
{
    (void)Color;
    GFXUI_SEAM_TRACE("FGFxRenderer::LineStyleColor");
}

void FGFxRenderer::FillStyleDisable() { GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleDisable"); }

void FGFxRenderer::FillStyleColor(GColor Color)
{
    (void)Color;
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleColor");
}

void FGFxRenderer::FillStyleBitmap(const GRenderer::FillTexture* Fill)
{
    FGFxRendererImpl::FFillTextureInfo Info;
    if (Fill) FGFxRendererImpl::ConvertFromUI(*Fill, Info);
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleBitmap");
}

void FGFxRenderer::FillStyleGouraud(GRenderer::GouraudFillType Type,
                                    const GRenderer::FillTexture* T0,
                                    const GRenderer::FillTexture* T1,
                                    const GRenderer::FillTexture* T2)
{
    (void)Type; (void)T0; (void)T1; (void)T2;
    GFXUI_SEAM_TRACE("FGFxRenderer::FillStyleGouraud");
}

void FGFxRenderer::DrawBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize, int StartIndex,
                               int Count, const GTexture* InTexture, const GMatrix2D& Matrix,
                               GRenderer::CacheProvider* Cache)
{
    (void)Bitmaps; (void)ListSize; (void)StartIndex; (void)InTexture; (void)Matrix; (void)Cache;
    RenderStats.Primitives += (unsigned int)(Count > 0 ? Count : 0);
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawBitmaps");
}

void FGFxRenderer::DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* Bitmaps, int ListSize,
                                            int StartIndex, int Count, const GTexture* InTexture,
                                            const GMatrix2D& Matrix,
                                            const GRenderer::DistanceFieldParams& Params,
                                            GRenderer::CacheProvider* Cache)
{
    (void)Bitmaps; (void)ListSize; (void)StartIndex; (void)InTexture; (void)Matrix; (void)Params;
    (void)Cache;
    RenderStats.Primitives += (unsigned int)(Count > 0 ? Count : 0);
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawDistanceFieldBitmaps");
}

void FGFxRenderer::BeginSubmitMask(GRenderer::SubmitMaskMode Mode)
{
    (void)Mode;
    ++RenderStats.Masks;
    GFXUI_SEAM_TRACE("FGFxRenderer::BeginSubmitMask");
}

void FGFxRenderer::EndSubmitMask() { GFXUI_SEAM_TRACE("FGFxRenderer::EndSubmitMask"); }
void FGFxRenderer::DisableMask() { GFXUI_SEAM_TRACE("FGFxRenderer::DisableMask"); }

unsigned int FGFxRenderer::CheckFilterSupport(const GRenderer::BlurFilterParams& Params)
{
    // 2012 0x5b7eb0: retail reports which filter passes the RHI can do. Nothing is supported until
    // the filter shaders are ported (package BD), and the runtime falls back to no filter.
    (void)Params;
    GFXUI_SEAM_TRACE("FGFxRenderer::CheckFilterSupport");
    return 0;
}

void FGFxRenderer::DrawBlurRect(GTexture* Source, const GRect<float>& Dest,
                                const GRect<float>& Src, const GRenderer::BlurFilterParams& Params,
                                bool bOnStack)
{
    (void)Source; (void)Dest; (void)Src; (void)Params; (void)bOnStack;
    ++RenderStats.Filters;
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawBlurRect");
}

void FGFxRenderer::DrawColorMatrixRect(GTexture* Source, const GRect<float>& Dest,
                                       const GRect<float>& Src, const float* Matrix, bool bOnStack)
{
    (void)Source; (void)Dest; (void)Src; (void)Matrix; (void)bOnStack;
    ++RenderStats.Filters;
    GFXUI_SEAM_TRACE("FGFxRenderer::DrawColorMatrixRect");
}

void FGFxRenderer::GetRenderStats(GRenderer::Stats* Stats, bool bReset)
{
    // 2012 0x5b7c90.
    if (Stats) *Stats = RenderStats;
    if (bReset) memset(&RenderStats, 0, sizeof(RenderStats));
    GFXUI_SEAM_TRACE("FGFxRenderer::GetRenderStats");
}

void FGFxRenderer::GetStats(GStatBag* Bag, bool bReset)
{
    (void)Bag; (void)bReset;
    GFXUI_SEAM_TRACE("FGFxRenderer::GetStats");
}

void FGFxRenderer::ReleaseResources()
{
    CurRenderTarget = 0;
    CurRenderTargetSet = 0;
    GFXUI_SEAM_TRACE("FGFxRenderer::ReleaseResources");
}

bool FGFxRenderer::AddEventHandler(GRendererEventHandler* Handler)
{
    (void)Handler;
    GFXUI_SEAM_TRACE("FGFxRenderer::AddEventHandler");
    return false;
}

void FGFxRenderer::RemoveEventHandler(GRendererEventHandler* Handler)
{
    (void)Handler;
    GFXUI_SEAM_TRACE("FGFxRenderer::RemoveEventHandler");
}
