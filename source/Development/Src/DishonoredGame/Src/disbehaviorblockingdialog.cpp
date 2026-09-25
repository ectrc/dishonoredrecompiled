// DishonoredGame/src/disbehaviorblockingdialog.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (37):
//   0x722830  public: static void __cdecl UDisTweaks_AIBehavior_BlockingDialog::InitializePrivateStaticClassUDisTweaks_AIBehavior_BlockingDialog(void)
//   0x722850  public: virtual void __thiscall UDisBehaviorBlockingDialog::InitBehavior(class UDishonoredAIBrain * const)
//   0x722860  public: virtual void __thiscall UDisBehaviorBlockingDialog::OnBehaviorStop(unsigned int)
//   0x7228c0  public: virtual void __thiscall UDisBehaviorBlockingDialog::OnBehaviorPause(unsigned int)
//   0x7228d0  public: virtual unsigned int __thiscall UDisBehaviorBlockingDialog::IsBehaviorFinished(void)const
//   0x722920  public: virtual unsigned char const * __thiscall UDisBehaviorBlockingDialog::BuildFilterStimMask(void)const
//   0x722a00  public: virtual unsigned char const * __thiscall UDisBehaviorBlockingDialog::BuildEvaluateStimMask(void)const
//   0x722a50  protected: void __thiscall UDisBehaviorBlockingDialog::SetupFromBlockingDialogRequest(struct FAIStimStruct_BlockingDialogRequest const &)
//   0x722a70  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterBlockingDialogEnd(struct FAIStimStruct_BlockingDialogEnd const &)
//   0x722aa0  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x722ab0  public: unsigned int __thiscall UDisBehaviorBlockingDialog::IsReadyToSpeak(void)const
//   0x722b00  protected: void __thiscall UDisBehaviorBlockingDialog::OnSoireeStarting(void)
//   0x723d60  public: virtual class TDynamicRHIResourceReference<12> const & __thiscall FRenderTarget::GetRenderTargetSurface(void)const
//   0x724bf0  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterReactions(struct FAIStimStruct const &)
//   0x724c40  private: virtual unsigned int __thiscall UDisBehaviorBlockingDialog::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x727ee0  public: virtual unsigned char const * __thiscall UDisBehaviorBlockingDialog::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x727f50  public: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterSoiree(struct FAIStimStruct_Soiree const &)
//   0x72b490  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorBlockingDialog::GetSetupFromStimDelegate(enum EAIStimID)
//   0x730190  protected: void __thiscall UDisBehaviorBlockingDialog::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x7301e0  protected: virtual void __thiscall UDisBehaviorBlockingDialog::BeginDestroy(void)
//   0x731ee0  public: virtual void __thiscall UDisBehaviorBlockingDialog::OnBehaviorStart(void)
//   0x731f30  public: virtual void __thiscall UDisBehaviorBlockingDialog::OnBehaviorResume(void)
//   0x7320c0  public: virtual void __thiscall UDisBehaviorBlockingDialog::TickBehavior(float)
//   0x7360a0  public: static class UClass * __cdecl UDisBehaviorBlockingDialog::GetPrivateStaticClassUDisBehaviorBlockingDialog(wchar_t const *)
//   0x738940  public: static void __cdecl UDisBehaviorBlockingDialog::InitializePrivateStaticClassUDisBehaviorBlockingDialog(void)
//   0x739a80  public: static class UClass * __cdecl UDisBehaviorBlockingDialog::StaticClassNoInline(void)
//   0x739ab0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_BlockingDialog::GetPrivateStaticClassUDisTweaks_AIBehavior_BlockingDialog(wchar_t const *)
//   0x73ac20  public: static class UClass * __cdecl UDisTweaks_AIBehavior_BlockingDialog::StaticClassNoInline(void)
//   0x73c870  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::EvaluateBlockingDialogRequest(struct FAIStimStruct_BlockingDialogRequest const &)const
//   0x73c8c0  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterBlockAtHighPriority(struct FAIStimStruct const &)
//   0x73c920  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterFinishAtLowPriority(struct FAIStimStruct const &)
//   0x73c970  protected: unsigned int __thiscall UDisBehaviorBlockingDialog::FilterBlockingDialogRequest(struct FAIStimStruct_BlockingDialogRequest const &)
//   0x73c9c0  public: virtual enum eDisDialogAvailability __thiscall UDisBehaviorBlockingDialog::GetDialogAvailability(enum eDisConvPriority, int)const
//   0x73ca20  public: virtual unsigned int __thiscall UDisBehaviorBlockingDialog::CanBlockSoiree(class FGuid, enum ESoireeAIPriority)const
//   0x73ca50  public: virtual unsigned int __thiscall UDisBehaviorBlockingDialog::CanBeSurprised(void)const
//   0x742660  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorBlockingDialog::GetFilterStimDelegate(enum EAIStimID)
//   0x7427a0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorBlockingDialog::GetEvaluateStimDelegate(enum EAIStimID)const
