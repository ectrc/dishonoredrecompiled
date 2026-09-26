// DishonoredGame/src/disdefencetower.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (41):
//   0x6310c0  public: static void __cdecl ADisDefenceTower::InitializePrivateStaticClassADisDefenceTower(void)
//   0x6310e0  public: static void __cdecl UDisTweaks_DefenceTower::InitializePrivateStaticClassUDisTweaks_DefenceTower(void)
//   0x631100  public: static void __cdecl UDisSeqEvent_DefenceTower::InitializePrivateStaticClassUDisSeqEvent_DefenceTower(void)
//   0x631120  public: virtual void __thiscall ADisDefenceTower::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x6311a0  public: virtual void __thiscall ADisWatchTower::PostLoad(void)
//   0x6311c0  public: unsigned int __thiscall ADisDefenceTower::IsActivated(void)const
//   0x6311d0  private: void __thiscall ADisDefenceTower::SetActivationMaterialParam(float)const
//   0x631230  public: virtual unsigned int __thiscall ADisDefenceTower::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x631260  public: virtual void __thiscall ADisDefenceTower::DisableSoulRendering(void)
//   0x631280  public: unsigned int __thiscall ADisDefenceTower::ComputeShape(class TArray<class FVector, class FDefaultAllocator> &)const
//   0x6316a0  public: virtual void __thiscall ADisDoor::Modify(unsigned int)
//   0x641470  public: virtual void __thiscall ADisDefenceTower::PostConstructed(void)
//   0x641480  public: virtual class FBox __thiscall ADisDefenceTower::GetObjectNavigationBounds(void)
//   0x646bb0  public: virtual void __thiscall UDisTweaks_DefenceTower::Serialize(class FArchive &)
//   0x64a110  public: virtual void __thiscall ADisDefenceTower::CreateEdgesForPathObject(class APylon *)
//   0x652300  public: static class UClass * __cdecl ADisDefenceTower::GetPrivateStaticClassADisDefenceTower(wchar_t const *)
//   0x652390  public: virtual void __thiscall ADisDefenceTower::DisGetMeshSplittingPolys(class TArray<struct UNavigationMeshBase::FMeshSplitingShape, class FDefaultAllocator> &)
//   0x653c40  public: static class UClass * __cdecl ADisDefenceTower::StaticClassNoInline(void)
//   0x6553f0  public: static class UClass * __cdecl UDisTweaks_DefenceTower::GetPrivateStaticClassUDisTweaks_DefenceTower(wchar_t const *)
//   0x656a40  public: static class UClass * __cdecl UDisTweaks_DefenceTower::StaticClassNoInline(void)
//   0x6580d0  public: static class UClass * __cdecl UDisSeqEvent_DefenceTower::GetPrivateStaticClassUDisSeqEvent_DefenceTower(wchar_t const *)
//   0x65b8b0  public: static class UClass * __cdecl UDisSeqEvent_DefenceTower::StaticClassNoInline(void)
//   0x65b8e0  protected: virtual void __thiscall ADisDefenceTower::ApplyTweakChanges_Derived(void)
//   0x65b9f0  public: virtual void __thiscall ADisDefenceTower::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x65bad0  public: virtual void __thiscall ADisDefenceTower::PostBeginPlay(void)
//   0x65bca0  public: virtual void __thiscall ADisDefenceTower::PreBeginPlay(void)
//   0x65bcf0  private: void __thiscall ADisDefenceTower::KillRat(class ADisGameCrowdAgentSkeletalRat *)
//   0x65be30  private: unsigned int __thiscall ADisDefenceTower::IsObjHostileToTower(class UDisTweaks_DefenceTower const *, class UObject const *)const
//   0x65bf30  public: unsigned int __thiscall ADisDefenceTower::HasWhaleOil(void)const
//   0x65bf90  private: unsigned int __thiscall ADisDefenceTower::HasLOSToTarget(class AActor const *)const
//   0x65c0b0  public: virtual unsigned int __thiscall ADisDefenceTower::HasSoul(int)const
//   0x65c110  public: virtual void __thiscall ADisDefenceTower::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x65c180  public: unsigned int __thiscall ADisDefenceTower::IsPassageAllowedFor(class ADishonoredNPCPawn *)const
//   0x65c2e0  public: virtual int __thiscall ADisDefenceTower::CostFor(struct FNavMeshPathParams const &, class FVector const &, class FVector &, struct FNavMeshPathObjectEdge *, struct FNavMeshPolyBase *)
//   0x65e510  private: class ADishonoredPawn * __thiscall ADisDefenceTower::GetPawnTarget(void)const
//   0x65e6a0  private: class ADisGameCrowdAgentSkeletalRat * __thiscall ADisDefenceTower::GetRatTarget(void)const
//   0x65e980  public: void __thiscall ADisDefenceTower::Deactivate(unsigned int)
//   0x65eb40  public: void __thiscall ADisDefenceTower::SwitchPolarity(void)
//   0x65ec40  public: virtual void __thiscall ADisDefenceTower::OnBatteryPlugged(void)
//   0x65ed60  public: virtual void __thiscall ADisDefenceTower::OnBatteryUnplugged(void)
//   0x65f6a0  public: virtual unsigned int __thiscall ADisDefenceTower::Tick(float, enum ELevelTick)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x6208a0 (2012 0x646bb0, same bytes): tweak versions up to 3 loaded in the game (not a class
// default, not a commandlet) get their guard faction appended to the friendly factions
void UDisTweaks_DefenceTower::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( m_VersionNum <= 3 && Ar.IsLoading() && !HasAnyFlags( RF_ClassDefaultObject ) && !GIsUCC )
	{
		m_FriendlyFactions.AddItem( m_pGuardFaction );
	}
}
