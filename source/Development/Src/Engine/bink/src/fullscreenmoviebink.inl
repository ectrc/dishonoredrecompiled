#pragma once
// Engine/bink/src/fullscreenmoviebink.inl
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (69):
//   0x1009b0  protected: __thiscall FFullScreenMovieBink::FFullScreenMovieBink(unsigned int)
//   0x100f50  public: virtual void __thiscall FFullScreenMovieBink::GameThreadPlayMovie(enum EMovieMode, wchar_t const *, int, int, int)
//   0xb8b9e0  _dynamic_initializer_for__FBinkMovieRenderClient::BinkShaders__
//   0xd9320  public: virtual unsigned int __thiscall FFullScreenMovieBink::IsTickable(void)const
//   0xd9360  _FFullScreenMovieBink::InputKey_::_17_::QueueBinkMovieTogglePauseCommand::DescribeCommand
//   0xd9370  _FFullScreenMovieBink::InputKey_::_40_::SkipBinkMovieCommand::DescribeCommand
//   0xd9380  public: virtual void __thiscall FFullScreenMovieBink::CloseRequested(class FViewport *)
//   0xd93c0  _FFullScreenMovieBink::GameThreadPlayMovie_::_14_::InitBinkRender::Execute
//   0xd93e0  _FFullScreenMovieBink::GameThreadPlayMovie_::_14_::InitBinkRender::DescribeCommand
//   0xd93f0  _FFullScreenMovieBink::GameThreadPlayMovie_::_43_::PlayGearsMovieCommand::DescribeCommand
//   0xd9400  _FFullScreenMovieBink::GameThreadStopMovie_::_8_::StopGearsMovieCommand::DescribeCommand
//   0xd9410  _FFullScreenMovieBink::GameThreadStopMovie_::_28_::DeleteBinkRenderCommand::Execute
//   0xd9430  _FFullScreenMovieBink::GameThreadStopMovie_::_28_::DeleteBinkRenderCommand::DescribeCommand
//   0xd9440  public: virtual void __thiscall FFullScreenMovieBink::GameThreadWaitForMovie(void)
//   0xd9540  _FFullScreenMovieBink::GameThreadToggleInputProcessing_::_5_::ToggleMovieInput::Execute
//   0xd9560  _FFullScreenMovieBink::GameThreadToggleInputProcessing_::_5_::ToggleMovieInput::DescribeCommand
//   0xd9570  _FFullScreenMovieBink::GameThreadSetMovieHidden_::_2_::ToggleMovieInput::Execute
//   0xd9590  _FFullScreenMovieBink::GameThreadSetMovieHidden_::_2_::ToggleMovieInput::DescribeCommand
//   0xd95a0  public: virtual void __thiscall FFullScreenMovieBink::GameThreadRequestDelayedStopMovie(void)
//   0xd95b0  public: virtual void __thiscall FFullScreenMovieBink::Mute(unsigned int)
//   0xd95d0  _FFullScreenMovieBink::GameThreadRemoveAllOverlays_::_2_::RemoveAllOverlaysCommand::DescribeCommand
//   0xd95e0  _FFullScreenMovieBink::GameThreadAddOverlay_::_5_::RemoveAllOverlaysCommand::DescribeCommand
//   0xd95f0  public: void __thiscall FBinkMovieAudio::SetAudioChannels(struct BINK *)
//   0xd97a0  public: void __thiscall FBinkMovieRenderClient::BeginUpdateTextures(struct BINK *)
//   0xd9800  public: void __thiscall FBinkMovieRenderClient::BeginRenderFrame(void)
//   0xd9890  public: void __thiscall FBinkMovieRenderClient::EndRenderFrame(void)
//   0xe0160  public: virtual void __thiscall FFullScreenMovieBink::InitAudio(void)
//   0xe0170  _FFullScreenMovieBink::InputKey_::_17_::QueueBinkMovieTogglePauseCommand::Execute
//   0xe0190  public: virtual void __thiscall FFullScreenMovieBink::GameThreadToggleInputProcessing(unsigned int)
//   0xe02d0  public: virtual void __thiscall FFullScreenMovieBink::GameThreadSetMovieHidden(unsigned int)
//   0xe03d0  public: void __thiscall FBinkMovieAudio::UpdateVolumeLevels(void)
//   0xe0460  public: __thiscall FBinkMovieRenderClient::FBinkMovieRenderClient(class FFullScreenMovieBink *)
//   0xe04a0  public: void __thiscall FBinkMovieRenderClient::MovieCleanupRendering(void)
//   0xe5370  public: virtual class FString __thiscall FFullScreenMovieBink::GameThreadGetLastMovieName(void)
//   0xe53c0  public: virtual void __thiscall FFullScreenMovieBink::GameThreadInitiateStartupSequence(void)
//   0xe5480  private: void __thiscall FFullScreenMovieBink::FreeStartupMovieMemory(void)
//   0xe5500  public: __thiscall FFullScreenMovieBink::FStartupMovie::FStartupMovie(class FString const &, unsigned int)
//   0xe5590  public: __thiscall FFullScreenMovieBink::FStartupMovie::~FStartupMovie(void)
//   0xe56c0  public: void __thiscall FBinkMovieAudio::SetSoundTracks(wchar_t const *)
//   0xeaeb0  GetFilenameWithoutBinkExtension
//   0xeafe0  public: unsigned int __thiscall FFullScreenMovieBink::LocateMovieInDLC(class UDownloadableContentManager *, wchar_t const *)
//   0xeb100  public: unsigned int __thiscall FFullScreenMovieBink::LocateMovie(class UDownloadableContentManager *, wchar_t const *)
//   0xeb4a0  _FFullScreenMovieBink::GameThreadPlayMovie_::_43_::PlayGearsMovieCommand::PlayGearsMovieCommand
//   0xeb530  public: virtual unsigned int __thiscall FFullScreenMovieBink::GameThreadIsMovieFinished(wchar_t const *)
//   0xeb640  public: virtual unsigned int __thiscall FFullScreenMovieBink::GameThreadIsMoviePlaying(wchar_t const *)
//   0xeb750  private: unsigned int __thiscall FFullScreenMovieBink::OpenStreamedMovie(wchar_t const *)
//   0xeb890  private: unsigned int __thiscall FFullScreenMovieBink::OpenPreloadedMovie(wchar_t const *, void *)
//   0xebad0  private: void __thiscall FFullScreenMovieBink::BinkDecodeFrame(struct BINK *)
//   0xebbe0  _FFullScreenMovieBink::GameThreadAddOverlay_::_5_::RemoveAllOverlaysCommand::RemoveAllOverlaysCommand
//   0xebc70  _FFullScreenMovieBink::GameThreadAddOverlay_::_5_::RemoveAllOverlaysCommand::Execute
//   0xf1200  public: virtual void __thiscall FFullScreenMovieBink::GameThreadAddOverlay(class UFont *, class FString const &, float, float, float, float, unsigned int, unsigned int, float)
//   0xf5c40  public: virtual __thiscall FFullScreenMovieBink::~FFullScreenMovieBink(void)
//   0xf5e00  private: void __thiscall FFullScreenMovieBink::StopCurrentAndPlayNext(unsigned int)
//   0xf5ef0  _FFullScreenMovieBink::GameThreadRemoveAllOverlays_::_2_::RemoveAllOverlaysCommand::Execute
//   0xf5f00  public: void __thiscall FBinkMovieRenderClient::FInternalBinkTextures::Init(struct BINK *)
//   0xf7e00  private: unsigned int __thiscall FFullScreenMovieBink::StopMovie(unsigned int)
//   0xf7e80  private: void __thiscall FFullScreenMovieBink::SkipMovie(void)
//   0xf7ee0  public: virtual void __thiscall FFullScreenMovieBink::GameThreadRemoveAllOverlays(void)
//   0xf8010  public: void __thiscall FBinkMovieRenderClient::DisplayWrappedString(class FCanvas *, wchar_t const *, unsigned int, unsigned int, class UFont *, struct FIntRect &, struct FLinearColor)
//   0xf82a0  public: void __thiscall FBinkMovieRenderClient::RenderFrame(struct BINK *, wchar_t const *, wchar_t const *, unsigned int, class TArray<struct FFullScreenMovieOverlay, class FDefaultAllocator> const &, unsigned int)
//   0xfa7a0  _FFullScreenMovieBink::InputKey_::_40_::SkipBinkMovieCommand::Execute
//   0xfa7b0  _FFullScreenMovieBink::GameThreadStopMovie_::_8_::StopGearsMovieCommand::Execute
//   0xfa7d0  private: unsigned int __thiscall FFullScreenMovieBink::PlayMovie(enum EMovieMode, wchar_t const *, wchar_t const *, void *, unsigned long, int, int, int, int)
//   0xfb080  private: unsigned int __thiscall FFullScreenMovieBink::PumpMovie(void)
//   0xfb530  private: unsigned int __thiscall FFullScreenMovieBink::ProcessNextStartupSequence(void)
//   0xfd0a0  public: virtual void __thiscall FFullScreenMovieBink::Tick(float)
//   0xfd130  public: virtual unsigned int __thiscall FFullScreenMovieBink::InputKey(class FViewport *, int, class FName, enum EInputEvent, float, unsigned int)
//   0xfd440  _FFullScreenMovieBink::GameThreadPlayMovie_::_43_::PlayGearsMovieCommand::Execute
//   0xfd4e0  public: virtual void __thiscall FFullScreenMovieBink::GameThreadStopMovie(float, unsigned int, unsigned int)
