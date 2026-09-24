// DishonoredGame/src/discomponentanimplayer.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x8af2b0  public: static void __cdecl UDisAnimDefinitions::InitializePrivateStaticClassUDisAnimDefinitions(void)
//   0x8af2d0  public: virtual unsigned long __thiscall FDisComponentAnimPlayer::GetMemoryFootprint(void)const
//   0x8af2e0  public: unsigned int __thiscall FDisComponentAnimPlayer::HACK_PlayFullBodyAnimByName(class FName &, unsigned int)
//   0x8af330  public: void __thiscall FDisComponentAnimPlayer::ForceSyncWithOtherNode(class UAnimNodeSequence *, enum eBodyFilter)
//   0x8af350  public: int __thiscall FDisComponentAnimPlayer::GetPlayingAnimOnChannel(enum eBodyFilter)const
//   0x8af370  public: struct FAnimSlotUsage const & __thiscall FDisComponentAnimPlayer::GetSlotUsage(enum eBodyFilter)const
//   0x8af390  protected: void __thiscall FDisComponentAnimPlayer::SendNotification(class UDishonoredNativeState *, enum eBodyFilter, enum EAnimPlayerNotification)
//   0x8b1880  public: __thiscall FDisComponentAnimPlayer::FDisComponentAnimPlayer(void)
//   0x8b9070  public: struct FDisAnimationChain * __thiscall UDisAnimDefinitions::FindAnimationChain(unsigned char)
//   0x8b90d0  public: virtual void __thiscall FDisComponentAnimPlayer::Starting(void)
//   0x8b92d0  protected: void __thiscall FDisComponentAnimPlayer::PlayAnimOnItem(unsigned char, class FName const &, unsigned int, float)
//   0x8b9360  protected: unsigned int __thiscall FDisComponentAnimPlayer::PlayAnimOnPlayer(enum eBodyFilter)
//   0x8b9490  public: void __thiscall FDisComponentAnimPlayer::StopAnimOnItem(unsigned char)
//   0x8b94f0  protected: void __thiscall FDisComponentAnimPlayer::StopAnimInternal(enum eBodyFilter, unsigned int)
//   0x8b96d0  public: int __thiscall FDisComponentAnimPlayer::GetNumStep(unsigned char, int)const
//   0x8bc950  public: void __thiscall FDisComponentAnimPlayer::StopAllAnims(void)
//   0x8bc970  public: void __thiscall FDisComponentAnimPlayer::StopAnim(enum eBodyFilter)
//   0x8bc990  protected: class FName __thiscall FDisComponentAnimPlayer::GetAnimationName(struct FDisAnimationChain *, int, int, unsigned int, int)
//   0x8bcd50  public: unsigned int __thiscall FDisComponentAnimPlayer::IsValidAnimChain(unsigned char, int, int)
//   0x8bd230  public: float __thiscall FDisComponentAnimPlayer::GetAnimDuration(unsigned char, int, int)
//   0x8c1430  public: virtual void __thiscall FDisComponentAnimPlayer::Stopping(void)
//   0x8c1560  protected: unsigned int __thiscall FDisComponentAnimPlayer::PlayAnim(struct FDisAnimationChain *, int, enum eBodyFilter)
//   0x8c19b0  protected: void __thiscall FDisComponentAnimPlayer::UpdateAnimation(enum eBodyFilter, float)
//   0x8c1b50  public: void __thiscall FDisComponentAnimPlayer::AdvanceToNextStep(enum eBodyFilter, unsigned int)
//   0x8c4cf0  public: virtual void __thiscall FDisComponentAnimPlayer::PreAsyncWorkTick(float)
//   0x8c4d40  public: unsigned int __thiscall FDisComponentAnimPlayer::PlayAnimChain(unsigned char, class UDishonoredNativeState *, int, int, enum eBodyFilter, float)
//   0x8d8570  public: static class UClass * __cdecl UDisAnimDefinitions::GetPrivateStaticClassUDisAnimDefinitions(wchar_t const *)
//   0x8d8a00  public: static class UClass * __cdecl UDisAnimDefinitions::StaticClassNoInline(void)
