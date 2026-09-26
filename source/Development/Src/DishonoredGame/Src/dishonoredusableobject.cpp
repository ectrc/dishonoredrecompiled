// DishonoredGame/src/dishonoredusableobject.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (95):
//   0x66b210  public: static void __cdecl UDisSeqEvent_Lock::InitializePrivateStaticClassUDisSeqEvent_Lock(void)
//   0x66b230  public: unsigned int __thiscall ADishonoredUsableObject::GetBoneOrSocketInfo(class FName const &, class FVector * const, class FRotator * const)const
//   0x66b340  private: void __thiscall ADishonoredUsableObject::OnAnyPawnDestroyed(class FArkGameEvent const &)
//   0x66b360  public: unsigned int __thiscall ADishonoredUsableObject::IsEnabled(void)const
//   0x66b370  public: virtual void __thiscall ADishonoredUsableObject::HideHighlight(void)
//   0x66b3b0  public: void __thiscall ADishonoredUsableObject::HandleTransitioningTapToUse(float)
//   0x66b420  public: virtual void __thiscall ADishonoredUsableObject::InitRBPhys(void)
//   0x66b460  public: class ADishonoredNPCPawn * __thiscall UDisNPCDistractionComponent::GetPawnGoingToDistraction(void)const
//   0x66b470  public: unsigned int __thiscall ADishonoredUsableObject::BeingUsedByPawn(class ADishonoredPawn const *)const
//   0x66b4a0  private: void __thiscall ADishonoredUsableObject::SetRemoteActorAnimPositionByTime(float)
//   0x66b4e0  public: virtual void __thiscall ADishonoredUsableObject::DisableSoulRendering(void)
//   0x66b520  public: virtual void __thiscall ADishonoredUsableObject::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x66b530  public: virtual void __thiscall ADishonoredUsableObject::UnTouch(class AActor *)
//   0x66b540  public: class UStaticMeshComponent * __thiscall ADishonoredUsableObject::GetStaticMeshComponent(void)
//   0x66d8a0  int __cdecl DisUsableObjectHelperFunctions::GetAbstractItemQuantity(class ADishonoredPawn const *, class UDisAbstractItem *)
//   0x66d8f0  class FString const * __cdecl DisUsableObjectHelperFunctions::HasKey(class ADishonoredPawn const *, class TArray<class FString, class FDefaultAllocator> const &)
//   0x66d9c0  void __cdecl DisUsableObjectHelperFunctions::UseKey(class ADishonoredPawn *, class FString const &)
//   0x66da10  public: virtual void __thiscall ADishonoredUsableObject::ShowHighlight(class UMaterialInterface *)
//   0x66db90  public: virtual void __thiscall ADishonoredUsableObject::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66dca0  private: void __thiscall ADishonoredUsableObject::SetHeldAnimPositionByTime(float, unsigned int)
//   0x671590  public: virtual void __thiscall UDisTweaks_UsableObject::Serialize(class FArchive &)
//   0x671660  private: void __thiscall ADishonoredUsableObject::SetRemoteActorAnimName(class FName const &)
//   0x674870  protected: virtual void __thiscall UDisTweaks_UsableObject::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x6748e0  public: virtual void __thiscall ADishonoredUsableObject::PostScriptDestroyed(void)
//   0x674930  public: virtual void __thiscall ADishonoredUsableObject::BeginDestroy(void)
//   0x674980  public: virtual void __thiscall ADishonoredUsableObject::MarkComponentsAsPendingKill(unsigned int)
//   0x6790d0  public: static class UClass * __cdecl ADishonoredUsableObject::GetPrivateStaticClassADishonoredUsableObject(wchar_t const *)
//   0x679160  public: static class UClass * __cdecl ADishonoredDynamicUsableObject::GetPrivateStaticClassADishonoredDynamicUsableObject(wchar_t const *)
//   0x67a460  public: static void __cdecl ADishonoredUsableObject::InitializePrivateStaticClassADishonoredUsableObject(void)
//   0x67b5e0  public: static class UClass * __cdecl ADishonoredUsableObject::StaticClassNoInline(void)
//   0x67b610  public: static class UClass * __cdecl UDisTweaks_UsableObject::GetPrivateStaticClassUDisTweaks_UsableObject(wchar_t const *)
//   0x67b6a0  public: static void __cdecl ADishonoredDynamicUsableObject::InitializePrivateStaticClassADishonoredDynamicUsableObject(void)
//   0x67bf90  public: static class UClass * __cdecl UDisUsableObjectBreakSteps::GetPrivateStaticClassUDisUsableObjectBreakSteps(wchar_t const *)
//   0x67c020  public: static void __cdecl UDisTweaks_UsableObject::InitializePrivateStaticClassUDisTweaks_UsableObject(void)
//   0x67c040  public: static class UClass * __cdecl ADishonoredDynamicUsableObject::StaticClassNoInline(void)
//   0x67ca70  public: static class UClass * __cdecl UDisTweaks_UsableObject::StaticClassNoInline(void)
//   0x67caa0  public: static class UClass * __cdecl UDisSeqEvent_Lock::GetPrivateStaticClassUDisSeqEvent_Lock(wchar_t const *)
//   0x6812e0  public: static void __cdecl UDisUsableObjectBreakSteps::InitializePrivateStaticClassUDisUsableObjectBreakSteps(void)
//   0x681300  public: static class UClass * __cdecl UDisSeqEvent_Lock::StaticClassNoInline(void)
//   0x681330  public: virtual unsigned int __thiscall ADishonoredUsableObject::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x6834e0  public: static class UClass * __cdecl UDisUsableObjectBreakSteps::StaticClassNoInline(void)
//   0x683510  private: virtual void __thiscall ADishonoredUsableObject::ApplyTweakChanges_Derived(void)
//   0x6835a0  public: void __thiscall ADishonoredUsableObject::SetEnabled(unsigned int)
//   0x683650  public: unsigned int __thiscall ADishonoredUsableObject::IsUsable(void)const
//   0x683720  public: virtual enum eCrossHairStatus __thiscall ADishonoredUsableObject::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x6837a0  public: virtual unsigned int __thiscall ADishonoredUsableObject::CanInteract(struct FCanInteractParams const &)const
//   0x6838d0  public: virtual int __thiscall ADishonoredUsableObject::InteractDTraceFlag(void)const
//   0x683910  public: virtual class FString const & __thiscall ADishonoredUsableObject::GetCrosshairFocusText(void)const
//   0x683980  public: virtual void __thiscall ADishonoredUsableObject::FormatText(class FString &)const
//   0x683b90  public: virtual class UDisTweaks_InteractableInterface const * __thiscall ADishonoredUsableObject::GetInteractableTweaks_Derived(void)const
//   0x683bc0  public: virtual void __thiscall ADishonoredUsableObject::Lock(void)
//   0x683c60  public: virtual void __thiscall ADishonoredUsableObject::Unlock(void)
//   0x683d50  public: virtual unsigned int __thiscall ADishonoredUsableObject::AttemptCannotUseInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x683fe0  public: int __thiscall ADishonoredUsableObject::GetNumKismetOutputs(void)const
//   0x684020  public: class FName __thiscall ADishonoredUsableObject::GetKismetOutputName(int)const
//   0x6840d0  public: virtual void __thiscall ADishonoredUsableObject::OnToggle_Native(class USeqAct_Toggle *)
//   0x684180  public: class UAnimNodeSequence * __thiscall ADishonoredUsableObject::GetSequenceNode(void)const
//   0x684200  protected: virtual unsigned int __thiscall ADishonoredUsableObject::ShouldBeMovable(void)const
//   0x684240  protected: virtual void __thiscall ADishonoredUsableObject::OnUseLockedObject(void)
//   0x684340  public: int __thiscall ADishonoredUsableObject::GetNumStages(void)const
//   0x684380  public: unsigned int __thiscall ADishonoredUsableObject::CanTriggerFromNotifies(void)const
//   0x6843e0  public: void __thiscall ADishonoredUsableObject::SetActiveStage(int)
//   0x684440  public: struct FUsableObjectStage const & __thiscall ADishonoredUsableObject::GetActiveStage(class UDisTweaks_UsableObject const *)const
//   0x6844a0  private: void __thiscall ADishonoredUsableObject::SetInitialProgress(void)
//   0x6845f0  public: virtual unsigned int __thiscall ADishonoredUsableObject::HasSoul(int)const
//   0x684650  public: virtual void __thiscall ADishonoredUsableObject::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x688b70  protected: virtual unsigned int __thiscall UDisTweaks_UsableObject::FixupDefaults_Derived(void)
//   0x688c40  public: virtual void __thiscall ADishonoredUsableObject::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x688d80  private: void __thiscall ADishonoredUsableObject::SetRemoteActorAnimFromStage(void)
//   0x688df0  private: int __thiscall ADishonoredUsableObject::GetCurrentKismetOutput(void)const
//   0x688e70  private: void __thiscall ADishonoredUsableObject::CommonInit(unsigned int)
//   0x6890f0  public: virtual void __thiscall ADishonoredUsableObject::PostEditImport(void)
//   0x689110  public: virtual void __thiscall ADishonoredUsableObject::PostLoad(void)
//   0x689160  public: void __thiscall ADishonoredUsableObject::ResetTransitionAnimation(void)
//   0x689280  private: void __thiscall ADishonoredUsableObject::HoldToUseStartForward(void)
//   0x689330  private: void __thiscall ADishonoredUsableObject::HoldToUseStopForward(void)
//   0x6893d0  private: void __thiscall ADishonoredUsableObject::HoldToUseCompleteForward(void)
//   0x689480  private: void __thiscall ADishonoredUsableObject::HoldToUseStartReverse(void)
//   0x6894f0  private: void __thiscall ADishonoredUsableObject::HoldToUseStopReverse(void)
//   0x689560  private: void __thiscall ADishonoredUsableObject::HoldToUseCompleteReverse(void)
//   0x68d010  public: void __thiscall ADishonoredUsableObject::HandleTransitionStart(float)
//   0x68d2c0  public: void __thiscall ADishonoredUsableObject::HandleTransitioningHoldToUse(float)
//   0x68d420  public: void __thiscall ADishonoredUsableObject::TriggerActivate(unsigned int)
//   0x68d5d0  public: virtual void __thiscall ADishonoredUsableObject::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x68d8c0  public: virtual void __thiscall ADishonoredUsableObject::PostBeginPlay(void)
//   0x68d910  public: virtual void __thiscall ADishonoredUsableObject::Spawned(void)
//   0x68d930  public: virtual void __thiscall ADishonoredUsableObject::OnBroken(int, int, class FVector const &, class FVector const &, class AActor *, class UClass const *)
//   0x68d9f0  private: void __thiscall ADishonoredUsableObject::HandleUseStopped(void)
//   0x68dbf0  public: virtual void __thiscall ADishonoredUsableObject::LoseCrosshairFocus(void)
//   0x68dc10  public: virtual void __thiscall ADishonoredUsableObject::OnUseRelease(void)
//   0x68fc60  public: void __thiscall ADishonoredUsableObject::UseObject(class ADishonoredPawn *)
//   0x68fe00  public: void __thiscall ADishonoredUsableObject::HandleTransitioning(float)
//   0x68fe70  public: void __thiscall ADishonoredUsableObject::HandleTransitionComplete(float)
//   0x690590  public: virtual unsigned int __thiscall ADishonoredUsableObject::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x6905c0  public: virtual unsigned int __thiscall ADishonoredUsableObject::Tick(float, enum ELevelTick)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x64e170 (2012 0x671590): licensee versions below 29 fold the deprecated
// m_bUsableWhileCarryingCorpse into m_bUsableWhileCarryingSomething, fallback skip included
void UDisTweaks_UsableObject::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( Ar.LicenseeVer() < 29 )
	{
		m_bUsableWhileCarryingSomething = m_bUsableWhileCarryingCorpse;
		if( FindFallbackSkip( TEXT("m_bUsableWhileCarryingCorpse") ) != INDEX_NONE )
		{
			SkipFallback( FName( TEXT("m_bUsableWhileCarryingSomething") ) );
		}
	}
}
