// DishonoredGame/src/disattentioninfo_complex.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (45):
//   0x78dab0  public: virtual unsigned int __thiscall UDisAttentionInfo_Complex::IsAttentionDecreasing(void)const
//   0x78dae0  public: virtual void __thiscall UDisAttentionInfo_Complex::GainLOS_Derived(void)
//   0x78daf0  public: virtual void __thiscall UDisAttentionInfo_Complex::LoseLOS_Derived(void)
//   0x78db00  public: virtual void __thiscall UDisAttentionInfo_Complex::BeTouched_Derived(void)
//   0x78db10  public: virtual void __thiscall UDisAttentionInfo_Complex::EnterHideout_Derived(void)
//   0x78db20  public: virtual void __thiscall UDisAttentionInfo_Complex::ExitHideout_Derived(void)
//   0x78db30  public: virtual void __thiscall UDisAttentionInfo_Complex::GainPsychicAttention_Derived(void)
//   0x78db40  public: virtual void __thiscall UDisAttentionInfo_Complex::LosePsychicAttention_Derived(void)
//   0x78db50  public: virtual void __thiscall UDisAttentionInfo_Complex::SetMinAttentionLevel_Derived(enum EDisMinAttentionLevelType, enum EDisAttentionLevel)
//   0x78db70  public: virtual enum EDisAttentionLevel __thiscall UDisAttentionInfo_Complex::GetMinAttentionLevel_Derived(enum EDisMinAttentionLevelType)const
//   0x78db90  public: virtual void __thiscall UDisAttentionInfo_Complex::OnDifficultyChange_AttnInfo_Derived(void)
//   0x78dbb0  public: virtual unsigned int __thiscall UDisAttentionInfo_Complex::HasAnyAttention_Derived(void)const
//   0x78dbe0  public: virtual float __thiscall UDisAttentionInfo_Complex::GetNormalizedAttnValue_Derived(void)const
//   0x78dc40  public: virtual enum EDisAttentionLevel __thiscall UDisAttentionInfo_Complex::GetAttnLevel_Derived(void)const
//   0x78dc50  public: virtual struct FDisAttentionChangeReason __thiscall UDisAttentionInfo_Complex::GetLastChangeReason_Derived(void)const
//   0x78dc80  private: void __thiscall UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Touch(struct FDisTickAttentionParams const &, float)
//   0x78dcf0  private: virtual unsigned int __thiscall UDisAttentionInfo_Complex::HasDirectAttention(void)const
//   0x78dd10  public: virtual unsigned int __thiscall UDisAttentionInfo_Complex::HasPsychicAttention(void)const
//   0x790010  public: virtual void __thiscall UDisAttentionInfo_Complex::IncreaseAttention_Derived(struct FDisAttentionIncrease const &, struct FDisAttentionChangeReason const &, struct FDisAttentionIncreaseInfo const &, unsigned int &)
//   0x7900c0  private: void __thiscall UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Sight_MakePendingChange(struct FDisTickAttentionParams const &, float)
//   0x790230  private: void __thiscall UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Psychic(struct FDisTickAttentionParams const &)
//   0x7902c0  private: void __thiscall UDisAttentionInfo_Complex::UpdateAttentionProxy_Player(struct FDisTickAttentionParams const &, float)
//   0x790430  private: enum EDisAttentionLevel __thiscall UDisAttentionInfo_Complex::GetAttentionLevelFromAttentionValue(struct FDisTickAttentionParams const &, unsigned int)
//   0x790510  private: enum EDisAttentionLevel __thiscall UDisAttentionInfo_Complex::GetMinAttentionLevel(unsigned int, enum EDisAttentionLevel)const
//   0x793300  public: virtual void __thiscall UDisAttentionInfo_Complex::OnOtherActorTerminated_AttnInfo_Derived(class AActor const &)
//   0x793410  private: void __thiscall UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Sight_UpdateSightedFlag(struct FDisTickAttentionParams const &)
//   0x7934f0  private: void __thiscall UDisAttentionInfo_Complex::SetAccumulatorParams(struct FDisTickAttentionParams const &, class UDishonoredAIBrain const * const)
//   0x793590  private: unsigned int __thiscall UDisAttentionInfo_Complex::ShouldClampAttentionToInvestigate(struct FDisTickAttentionParams const &, struct FDisAttentionChangeReason const &, enum EDisAttentionLevel, enum EDisAttentionLevel, unsigned int &)
//   0x793680  public: virtual unsigned int __thiscall UDisAttentionInfo_Complex::ShouldKeepAlarmRinging_Derived(void)const
//   0x793710  public: virtual float __thiscall UDisAttentionInfo_Complex::GetTimeLeftInInvestigate_Derived(void)const
//   0x7937b0  private: virtual void __thiscall UDisAttentionInfo_Complex::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x793830  private: virtual void __thiscall UDisAttentionInfo_Complex::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x798b30  private: void __thiscall UDisAttentionInfo_Complex::UpdateAttentionProxy(struct FDisTickAttentionParams const &, struct TMemStackArray<struct FDisAppliedAttentionChangeInfo> const &, float)
//   0x798d20  private: void __thiscall UDisAttentionInfo_Complex::UpdateAttentionInfo(struct FDisTickAttentionParams const &, struct FDisAppliedAttentionChangeInfo &, struct TMemStackArray<struct DisAttention::FDisAttentionChangeMessage> &)
//   0x79b180  private: void __thiscall UDisAttentionInfo_Complex::ApplyAttentionDrain(struct FDisTickAttentionParams const &, float, unsigned int, struct FDisAppliedAttentionChangeInfo &, struct TMemStackArray<struct DisAttention::FDisAttentionChangeMessage> &)
//   0x79ce20  public: static void __cdecl UDisAttentionInfo_Complex::InitializePrivateStaticClassUDisAttentionInfo_Complex(void)
//   0x79ce40  private: unsigned int __thiscall UDisAttentionInfo_Complex::ApplyAttentionIncreases(struct FDisTickAttentionParams const &, float, struct TMemStackArray<struct FDisAppliedAttentionChangeInfo> &, struct FDisAttentionChangeReason &, struct TMemStackArray<struct DisAttention::FDisAttentionChangeMessage> &)
//   0x7a0860  public: virtual void __thiscall UDisAttentionInfo_Complex::SetAsTopAttnTarget_Derived(class UDishonoredAIBrain *, struct FDisAttentionProxy const &)
//   0x7a2090  public: static class UClass * __cdecl UDisAttentionInfo_Complex::GetPrivateStaticClassUDisAttentionInfo_Complex(wchar_t const *)
//   0x7a2120  void __cdecl DisAttentionInfo_Complex::SetupTickParamsFromTarget(class IDisAttentionTargetInterface * const, class UDisTweaks_PawnAttention const * const, class UDisAIBrainProcessAttention const * const, class UDishonoredAIBrain const * const, struct FDisTickAttentionParams &, struct FDisTickAttentionParams_PawnAttentionTweaks &, struct FDisTickAttentionParams_AttentionTargetTweaks &)
//   0x7a37f0  public: static class UClass * __cdecl UDisAttentionInfo_Complex::StaticClassNoInline(void)
//   0x7a7430  private: void __thiscall UDisAttentionInfo_Complex::InitAttnInfo_Common(class UDisAIBrainProcessAttention const *, class IDisAttentionTargetInterface const *)
//   0x7a74d0  public: virtual void __thiscall UDisAttentionInfo_Complex::TickAttnInfo_Derived(float, class IDisAttentionTargetInterface *, class UDisAIBrainProcessAttention const *, class UDishonoredAIBrain const *, unsigned int, struct TMemStackArray<struct FDisAppliedAttentionChangeInfo> &, struct TMemStackArray<struct DisAttention::FDisAttentionChangeMessage> &)
//   0x7a7770  public: virtual void __thiscall UDisAttentionInfo_Complex::OnAttnTargetStateChange(class UDisAIBrainProcessAttention const *, class IDisAttentionTargetInterface const *)
//   0x7a7b50  public: virtual void __thiscall UDisAttentionInfo_Complex::InitAttnInfo_Derived(class UDisAIBrainProcessAttention const *, class IDisAttentionTargetInterface const *)
