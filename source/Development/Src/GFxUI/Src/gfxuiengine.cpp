// GFxUI/src/gfxuiengine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (62):
//   0x5b6b50  public: static void __cdecl UGFxEngine::InitializePrivateStaticClassUGFxEngine(void)
//   0x5b6b70  public: void __thiscall UGFxEngine::Release(void)
//   0x5b6ba0  IsFilesystemPath
//   0x5b6be0  public: virtual __thiscall FGFxEngineBase::~FGFxEngineBase(void)
//   0x5b6c40  _FGFxEngine::_FGFxEngine_::_2_::ReleaseRenderTargetsCommand::DescribeCommand
//   0x5b6c50  _FGFxEngine::RenderUI_::_8_::FSetGFxRenderTargets::DescribeCommand
//   0x5b6c60  _FGFxEngine::RenderUI_::_31_::FFinishGFxRender::Execute
//   0x5b6ca0  _FGFxEngine::RenderUI_::_31_::FFinishGFxRender::DescribeCommand
//   0x5b6cb0  _FGFxEngine::RenderTextures_::_10_::FSetGFxRenderTargets::DescribeCommand
//   0x5b6cc0  _FGFxEngine::RenderTextures_::_22_::FGFxResolveRenderTargets::DescribeCommand
//   0x5b9940  public: __thiscall FGFxEngineBase::FGFxEngineBase(void)
//   0x5b99d0  _FGFxEngine::RenderTextures_::_22_::FGFxResolveRenderTargets::Execute
//   0x5bf7d0  public: void __thiscall FGFxEngine::SetRenderViewport(class FViewport *)
//   0x5bfa30  public: void __thiscall FGFxEngine::Tick(float)
//   0x5bfb90  public: class FGFxMovie * __thiscall FGFxEngine::GetTopmostMovie(void)const
//   0x5bfbe0  public: class FGFxMovie * __thiscall FGFxEngine::GetOpenMovie(int)const
//   0x5bfc40  private: void __thiscall FGFxEngine::SetMovieSize(class FGFxMovie *)
//   0x5bfe30  public: int __thiscall FGFxEngine::GetLocalPlayerIndexFromControllerID(int, unsigned int)
//   0x5bfec0  public: class FGFxMovie * __thiscall FGFxEngine::GetFocusedMovieFromControllerID(int)
//   0x5c9160  public: void __thiscall FGFxEngine::InsertMovieIntoList(class FGFxMovie *, class TArray<class FGFxMovie *, class FDefaultAllocator> *)
//   0x5c9240  public: static class FFilename __cdecl FGFxEngine::CollapseRelativePath(class FFilename const &)
//   0x5c9610  public: static unsigned int __cdecl FGFxEngine::GetPackagePath(char const *, class FFilename &)
//   0x5c9870  private: virtual void __thiscall FGFxURLBuilder::BuildURL(class GString *, struct GFxURLBuilder::LocationInfo const &)
//   0x5c9be0  public: virtual class GImageInfoBase * __thiscall FGFxImageCreator::CreateImage(class GFxImageCreateInfo const &)
//   0x5c9e90  public: virtual class GImageInfoBase * __thiscall FGFxImageLoader::LoadImageW(char const *)
//   0x5ca230  _FGFxEngine::_FGFxEngine_::_2_::ReleaseRenderTargetsCommand::Execute
//   0x5ca2a0  _FGFxEngine::RenderUI_::_8_::FSetGFxRenderTargets::Execute
//   0x5ca3d0  _FGFxEngine::RenderTextures_::_10_::FSetGFxRenderTargets::Execute
//   0x5ca600  public: void __thiscall FGFxEngine::ReevaluateSizes(void)
//   0x5ca660  public: void __thiscall FGFxEngine::ReevaluateFocus(void)
//   0x5ce820  public: void __thiscall FGFxEngine::InsertMovie(class FGFxMovie *, unsigned char)
//   0x5ce890  public: void __thiscall FGFxEngine::CloseScene(class FGFxMovie *, unsigned int)
//   0x5ce9c0  public: void __thiscall FGFxEngine::DeleteQueuedMovies(unsigned int)
//   0x5ceb00  public: void __thiscall FGFxEngine::RenderUI(unsigned int, int)
//   0x5cee50  public: void __thiscall FGFxEngine::RenderTextures(void)
//   0x5cf0d0  public: void __thiscall FGFxEngine::StartScene(class FGFxMovie *, class UTextureRenderTarget2D *, unsigned int, unsigned int)
//   0x5cf2a0  public: void __thiscall FGFxEngine::AddPlayerState(void)
//   0x5d1570  public: static class UClass * __cdecl UGFxEngine::GetPrivateStaticClassUGFxEngine(wchar_t const *)
//   0x5d1600  private: static void __cdecl FGFxEngine::InitGFxLoaderCommon(class GFxLoader &)
//   0x5d17f0  public: void __thiscall FGFxEngine::CloseAllMovies(int)
//   0x5d18d0  public: void __thiscall FGFxEngine::CloseAllTextureMovies(void)
//   0x5d1950  public: void __thiscall FGFxEngine::CloseTopmostScene(void)
//   0x5d1a30  private: unsigned int __thiscall FGFxEngine::InputKey(int, class FGFxMovie *, class FName, enum EInputEvent)
//   0x5d1e40  public: unsigned int __thiscall FGFxEngine::InputKey(int, class FName, enum EInputEvent)
//   0x5d1f90  private: unsigned int __thiscall FGFxEngine::IsKeyCaptured(class FName)
//   0x5d20a0  public: unsigned int __thiscall FGFxEngine::InputChar(int, wchar_t)
//   0x5d4980  public: static class UClass * __cdecl UGFxEngine::StaticClassNoInline(void)
//   0x5d49b0  public: void __thiscall FGFxEngine::NotifyGameSessionEnded(void)
//   0x5d4b40  public: unsigned int __thiscall FGFxEngine::InputAxis(int, class FName, float, float, unsigned int)
//   0x5da330  public: void __thiscall FGFxEngine::FlushPlayerInput(class TSet<int, struct DefaultKeyFuncs<int, 0>, class FDefaultSetAllocator> *)
//   0x5dda60  public: virtual class GFile * __thiscall FGFxFileOpener::OpenFile(char const *, int, int)
//   0x5ddc80  public: virtual __int64 __thiscall FGFxFileOpener::GetFileModifyTime(char const *)
//   0x5ddd90  public: virtual __thiscall FGFxEngine::~FGFxEngine(void)
//   0x5de0a0  public: class GFxMovieDef * __thiscall FGFxEngine::LoadMovieDef(wchar_t const *, struct GFxMovieInfo &)
//   0x5de3e0  public: class FGFxMovie * __thiscall FGFxEngine::LoadMovie(wchar_t const *, unsigned int)
//   0x5dfdd0  public: void __thiscall FGFxEngine::InitKeyMap(void)
//   0x5e1780  public: void __thiscall FGFxEngine::UpdateKeyEmulation(class FName, class FName, class FName, class FName, class FName)
//   0x5e2900  private: __thiscall FGFxEngine::FGFxEngine(void)
//   0x5e3190  public: virtual void __thiscall FGFxFSCommandHandler::Callback(class GFxMovieView *, char const *, char const *)
//   0x5e3450  public: static class FGFxEngine * __cdecl FGFxEngine::GetEngine(void)
//   0x5e4880  public: virtual void __thiscall FGFxExternalInterface::Callback(class GFxMovieView *, char const *, class GFxValue const *, unsigned int)
//   0xba18f0  dynamic_initializer_for((long long, ()[llocator]))
