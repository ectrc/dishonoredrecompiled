// DishonoredGame/src/diswatchtower.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (60):
//   0x66a450  public: static void __cdecl ADisWatchTower::InitializePrivateStaticClassADisWatchTower(void)
//   0x66a470  public: static void __cdecl UDisTweaks_WatchTower::InitializePrivateStaticClassUDisTweaks_WatchTower(void)
//   0x66a490  public: static void __cdecl UDisSeqEvent_WatchTower::InitializePrivateStaticClassUDisSeqEvent_WatchTower(void)
//   0x66a4b0  public: virtual void __thiscall ADisWatchTower::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x66a4d0  public: virtual void __thiscall ADisWatchTower::Serialize(class FArchive &)
//   0x66a500  public: virtual struct FDisRelationshipOverrideInfo * __thiscall ADisWatchTower::GetRelationshipOverrideInfo(void)
//   0x66a510  public: virtual unsigned int __thiscall ADisWatchTower::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x66a540  public: unsigned int __thiscall ADisWatchTower::IsActivated(void)const
//   0x66a550  public: void __thiscall ADisWatchTower::SetExploreTracked(class AActor *)
//   0x66a570  public: virtual void __thiscall ADisWatchTower::OnSetDisposition(class UDisSeqAct_SetDisposition *)
//   0x66a580  public: virtual void __thiscall ADisWatchTower::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x66a650  private: virtual void __thiscall ADisWatchTower::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x66a6e0  private: virtual void __thiscall ADisWatchTower::DisableSoulRendering(void)
//   0x66c490  protected: virtual void __thiscall ADisWatchTower::SetTweaks_Derived(class UDisTweaksBase *)
//   0x66c4a0  public: virtual void __thiscall ADisWatchTower::DoKismetAttachment(class AActor *, struct FAttachmentInfos)
//   0x6701f0  private: class AActor * __thiscall ADisWatchTower::GetMostSuspiciousSeenActor(enum EDisWatchTowerActorSuspicion &)const
//   0x670340  private: void __thiscall ADisWatchTower::GetBestTargetPosition(class AActor const *, class FVector &)
//   0x670390  public: virtual void __thiscall ADisWatchTower::OnWatchTowerShootAtTarget(class UDisSeqAct_WatchTowerShootAtTarget *)
//   0x670400  public: virtual void __thiscall ADisWatchTower::HandleSquadNameChange(class FName const &, class FName const &)
//   0x670490  public: virtual class ADishonoredPawn * __thiscall ADisWatchTower::GetNoiseMakerPawn_Derived(void)const
//   0x674090  public: virtual void __thiscall ADisWatchTower::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x674160  public: void __thiscall ADisWatchTower::VisionStatusChanged(unsigned int, class AActor *)
//   0x6741f0  protected: virtual void __thiscall ADisWatchTower::OnOtherActorTerminated(class AActor const &)
//   0x674730  private: virtual void __thiscall ADisWatchTower::OnActorTerminated(void)
//   0x67ad40  public: static class UClass * __cdecl UDisTweaks_WatchTower::GetPrivateStaticClassUDisTweaks_WatchTower(wchar_t const *)
//   0x67bd10  public: static class UClass * __cdecl UDisTweaks_WatchTower::StaticClassNoInline(void)
//   0x67c580  public: static class UClass * __cdecl UDisSeqEvent_WatchTower::GetPrivateStaticClassUDisSeqEvent_WatchTower(wchar_t const *)
//   0x67e600  public: static class UClass * __cdecl UDisSeqEvent_WatchTower::StaticClassNoInline(void)
//   0x67e630  protected: virtual void __thiscall ADisWatchTower::ApplyTweakChanges_Derived(void)
//   0x67e7c0  public: virtual class FVector __thiscall ADisWatchTower::GetEarLocation(void)const
//   0x67e820  public: virtual class UDisTweaks_Faction * __thiscall ADisWatchTower::GetFactionTweak(void)const
//   0x67e860  private: void __thiscall ADisWatchTower::GetLightPivot(class FVector &)const
//   0x67e8e0  private: void __thiscall ADisWatchTower::AimLightTowardsTarget(float, class FVector)
//   0x67ee90  private: unsigned int __thiscall ADisWatchTower::GetArrowSocketPosRot(class FVector &, class FRotator &)
//   0x67eef0  private: void __thiscall ADisWatchTower::FireArrow(class AActor const *)
//   0x67f0b0  private: void __thiscall ADisWatchTower::CheckForAllyAwareOfPlayer(void)
//   0x67f380  private: void __thiscall ADisWatchTower::UpdateLight(float)
//   0x67f5a0  private: void __thiscall ADisWatchTower::UpdateGun(float)
//   0x67f7b0  private: void __thiscall ADisWatchTower::UpdateBatteryDrain(float)
//   0x67f840  public: virtual unsigned int __thiscall ADisWatchTower::IsBlinding(struct FDishonoredViewTarget const &)const
//   0x67fbb0  public: class UDisTweaks_Vision const * __thiscall ADisWatchTower::GetVisionTweaks(void)const
//   0x67fc50  private: void __thiscall ADisWatchTower::CreateLightParticleSystem(class UParticleSystem *)
//   0x67fd50  private: void __thiscall ADisWatchTower::CreateAlertLightParticleSystem(class UParticleSystem *)
//   0x6821c0  public: static class UClass * __cdecl ADisWatchTower::GetPrivateStaticClassADisWatchTower(wchar_t const *)
//   0x682250  private: void __thiscall ADisWatchTower::EnterState(enum EDisWatchTowerState)
//   0x682500  public: void __thiscall ADisWatchTower::Deactivate(unsigned int)
//   0x6825f0  private: void __thiscall ADisWatchTower::NoteCorpse(class AActor *)
//   0x6826b0  private: void __thiscall ADisWatchTower::SetPolarityReversed(unsigned int)
//   0x682840  private: void __thiscall ADisWatchTower::Investigate(class FVector)
//   0x685d90  public: static class UClass * __cdecl ADisWatchTower::StaticClassNoInline(void)
//   0x685dc0  public: virtual void __thiscall ADisWatchTower::HandleNoiseHeard(struct FDisAINoiseInfo const &)
//   0x685f20  public: virtual void __thiscall ADisWatchTower::OnBatteryPlugged(void)
//   0x685f60  public: virtual void __thiscall ADisWatchTower::OnBatteryUnplugged(void)
//   0x685fa0  public: virtual void __thiscall ADisWatchTower::OnUsableUsed(class ADishonoredUsableObject *, int)
//   0x685fe0  public: virtual void __thiscall ADisWatchTower::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6860c0  public: virtual void __thiscall ADisWatchTower::PostBeginPlay(void)
//   0x686ba0  private: void __thiscall ADisWatchTower::TickInGame(float)
//   0x687100  public: virtual void __thiscall ADisWatchTower::TakeDamage_Native(int &, class AController *, class FVector, class FVector &, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x68ae90  public: virtual unsigned int __thiscall ADisWatchTower::Tick(float, enum ELevelTick)
//   0x68ffb0  public: void __thiscall ADisWatchTower::SwitchPolarity(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x6195c0 (2012 0x66a4d0, same bytes; diswatchtower.cpp:107)
void ADisWatchTower::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	DisSerializeRelationshipOverrideInfo( m_PersonalRelationships, Ar );
}
