// DishonoredGame/src/dishonorednpccontroller.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (38):
//   0x7ab020  public: static void __cdecl ADishonoredNPCController::InitializePrivateStaticClassADishonoredNPCController(void)
//   0x7ab040  public: virtual void __thiscall ADishonoredNPCController::OnAIKismetDoBehavior(class UDisSeqAct_AIDoBehavior_Base *)
//   0x7ab050  public: virtual void __thiscall ADishonoredNPCController::OnAISetBrainFlags(class UDisSeqAct_AISetBrainFlags *)
//   0x7ab070  public: virtual void __thiscall ADishonoredNPCController::OnAIGetBrainFlags(class UDisSeqAct_AIGetBrainFlagValue *)
//   0x7ab090  public: virtual void __thiscall ADishonoredNPCController::OnAISetSenses(class UDisSeqAct_AISetSenses *)
//   0x7ab0f0  public: virtual void __thiscall ADishonoredNPCController::ClearComponents(void)
//   0x7ab130  public: virtual void __thiscall ADishonoredNPCController::OnOtherActorTerminated(class AActor const &)
//   0x7ab140  public: void __thiscall ADishonoredNPCController::SetGoToActionStatus(enum AIGoToActorOutputEnum, class UDisSeqAct_AIGoToActor *)
//   0x7ab180  public: void __thiscall ADishonoredNPCController::SetShootActionStatus(enum AIShootOutputEnum)
//   0x7ab1b0  public: virtual void __thiscall ADishonoredNPCController::UnPossess(void)
//   0x7ab370  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsSaveable(enum ESaveLoadLocation)const
//   0x7ad0b0  public: virtual void __thiscall ADishonoredNPCController::PostBeginPlay(void)
//   0x7ad0d0  public: virtual void __thiscall ADishonoredNPCController::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7ad130  public: virtual class FVector __thiscall ADishonoredNPCController::GetMoveTargetLocation(void)const
//   0x7ad180  public: virtual unsigned int __thiscall ADishonoredNPCController::IsDead(void)const
//   0x7ad1a0  public: class FDisComponentVision * __thiscall ADishonoredNPCController::GetVisionComponent(void)const
//   0x7b05a0  public: virtual void __thiscall ADishonoredNPCController::OnAISetSuspicionLevel(class UDisSeqAct_AISetSuspicionLevel *)
//   0x7b0630  public: virtual void __thiscall ADishonoredNPCController::OnAIClearAttention(class UDisSeqAct_AIClearAttention *)
//   0x7b0740  public: virtual void __thiscall ADishonoredNPCController::OnAIPsychicAttention(class UDisSeqAct_AIPsychicAttention *)
//   0x7b0840  public: virtual void __thiscall ADishonoredNPCController::OnAIProtectNeutralsOverride(class UDisSeqAct_AIProtectNeutralsOverride *)
//   0x7b0890  public: virtual unsigned long __thiscall ADishonoredNPCController::SeePawn(class APawn *, unsigned int)
//   0x7b08c0  public: void __thiscall ADishonoredNPCController::StartSeeingVisibleThing(class AActor *)
//   0x7b0970  public: void __thiscall ADishonoredNPCController::StopSeeingVisibleThing(class AActor *)
//   0x7bd0d0  public: virtual void __thiscall ADishonoredNPCController::OnAIAmbush(class UDisSeqAct_AIAmbush *)
//   0x7bd250  public: virtual void __thiscall ADishonoredNPCController::OnAIDoSearch(class UDisSeqAct_AIDoSearch *)
//   0x7bd370  public: virtual void __thiscall ADishonoredNPCController::OnAIGoToActor(class UDisSeqAct_AIGoToActor *)
//   0x7bd560  public: virtual void __thiscall ADishonoredNPCController::OnAIGuard(class UDisSeqAct_AIGuard *)
//   0x7bd6b0  public: virtual void __thiscall ADishonoredNPCController::OnAISetPatrol(class UDisSeqAct_AISetPatrol *)
//   0x7bd7e0  public: virtual void __thiscall ADishonoredNPCController::OnAIShoot(class UDisSeqAct_AIShoot *)
//   0x7bd9b0  public: virtual void __thiscall ADishonoredNPCController::OnAIStartDistraction(class UDisSeqAct_AIStartDistraction *)
//   0x7bda70  public: virtual void __thiscall ADishonoredNPCController::OnAIRingAlarm(class UDisSeqAct_AIRingAlarm *)
//   0x7be930  public: static class UClass * __cdecl ADishonoredNPCController::GetPrivateStaticClassADishonoredNPCController(wchar_t const *)
//   0x7bed50  public: static class UClass * __cdecl ADishonoredNPCController::StaticClassNoInline(void)
//   0x7c8f40  public: void __thiscall ADishonoredNPCController::InitNPC(class UDisTweaks_AIBrain * const, enum EDisAISuspicionLevel)
//   0x7c9030  public: virtual unsigned int __thiscall ADishonoredNPCController::Tick(float, enum ELevelTick)
//   0x7c90b0  public: virtual unsigned int __thiscall ADishonoredNPCController::OnNotifyBump(class AActor *, class UPrimitiveComponent *, class FVector const &)
//   0x7c91c0  public: virtual void __thiscall ADishonoredNPCController::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0xbaaa80  _dynamic_initializer_for__ADishonoredNPCController::s_DisAIBrainDefaultName__
