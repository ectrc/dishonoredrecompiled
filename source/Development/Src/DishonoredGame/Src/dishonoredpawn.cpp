// DishonoredGame/src/dishonoredpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (67):
//   0x78e1b0  public: static void __cdecl ADishonoredPawn::InitializePrivateStaticClassADishonoredPawn(void)
//   0x78e1d0  public: virtual void __thiscall ADishonoredPawn::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x78e1f0  public: virtual void __thiscall ADishonoredPawn::Serialize(class FArchive &)
//   0x78e220  public: virtual void __thiscall ADishonoredPawn::PreBeginPlay(void)
//   0x78e270  public: virtual void __thiscall ADishonoredPawn::PossessedBy(class AController *)
//   0x78e2b0  public: virtual class USkeletalMeshComponent * __thiscall ADishonoredPawn::GetMeshByIndex(int)const
//   0x78e2d0  public: virtual void __thiscall ADishonoredPawn::PlayDying_Native(class AController *, class UClass *, class FVector)
//   0x78e2e0  public: void __thiscall ADishonoredPawn::BecomeOfficiallyDead(void)
//   0x78e300  public: unsigned int __thiscall ADishonoredPawn::IsDead(void)const
//   0x78e310  public: virtual unsigned int __thiscall ADishonoredPawn::ArkIsDeadOrDestroyed(void)const
//   0x78e320  public: unsigned int __thiscall ADishonoredPawn::ArePowersInhibited(void)const
//   0x78e350  public: virtual class UObject * __thiscall ADishonoredPawn::GetUObjectInterfaceDisLookAtInterface(void)
//   0x78e360  public: virtual class ADishonoredPawn * __thiscall ADishonoredPawn::GetNoiseMakerPawn_Derived(void)const
//   0x78e3a0  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredPawn::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x78e3c0  public: virtual class FVector __thiscall ADishonoredPawn::GetEarLocation(void)const
//   0x78e3f0  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredPawn::GetListenerAudioCell(void)const
//   0x78e410  public: virtual void __thiscall ADishonoredPawn::OnSetDisposition(class UDisSeqAct_SetDisposition *)
//   0x78e420  protected: virtual class UDisTweaksBase * __thiscall ADishonoredPawn::GetTweaks_Derived(void)
//   0x78e430  public: class FDisComponentRetargeting * __thiscall ADishonoredPawn::GetComponentRetargeting(void)const
//   0x78e440  public: void __thiscall ADishonoredPawn::ConsumeRat(void)
//   0x78e490  public: void __thiscall ADishonoredPawn::DeferredDestroy(float)
//   0x78e4b0  public: virtual void __thiscall ADishonoredPawn::StartRootMotionMode(enum ERootMotionMode, enum ERootMotionRotationMode)
//   0x78e4c0  public: virtual void __thiscall ADishonoredPawn::StopRootMotionMode(void)
//   0x78e4d0  protected: virtual unsigned int __thiscall ADishonoredPawn::MAT_RootMotionEnabled(void)const
//   0x78e500  protected: void __thiscall ADishonoredPawn::MAT_StopRootMotion(void)
//   0x790c10  public: virtual unsigned int __thiscall ADishonoredPawn::Tick(float, enum ELevelTick)
//   0x790d40  public: void __thiscall ADishonoredPawn::SetIsVisible(unsigned int)
//   0x790dc0  public: virtual void __thiscall ADishonoredPawn::Landed_Native(class FVector, class AActor *)
//   0x790fc0  public: virtual void __thiscall ADishonoredPawn::SelectPower(int, enum ADishonoredPawn::EEquipItemFlags)
//   0x790ff0  protected: virtual void __thiscall ADishonoredPawn::SetTweaks_Derived(class UDisTweaksBase *)
//   0x791000  public: void __thiscall ADishonoredPawn::DrinkWater(void)
//   0x791070  public: virtual void __thiscall ADishonoredPawn::MAT_SetAnimPosition(class FName, int, class FName, float, unsigned int, unsigned int, unsigned int)
//   0x7910e0  protected: virtual void __thiscall ADishonoredPawn::MAT_SetRootMotionMode(enum EMatRootMotionMode)
//   0x791130  protected: virtual void __thiscall ADishonoredPawn::MAT_DisableMeshTranslation(unsigned int)
//   0x791170  protected: void __thiscall ADishonoredPawn::MAT_EnableMeshTranslation(void)
//   0x791190  protected: virtual void __thiscall ADishonoredPawn::MAT_SetMeshTranslationMode(enum EMatMeshTranslationMode)
//   0x793f50  protected: virtual void __thiscall ADishonoredPawn::PreBeginPlay_NativeComponents(void)
//   0x793ff0  protected: void __thiscall ADishonoredPawn::PostBeginPlay_Attachments(void)
//   0x794090  public: virtual unsigned int __thiscall ADishonoredPawn::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x794110  public: virtual void __thiscall ADishonoredPawn::NotifyBump(class AActor *, class UPrimitiveComponent *, class FVector const &)
//   0x7941c0  public: virtual void __thiscall ADishonoredPawn::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x794280  public: virtual unsigned int __thiscall ADishonoredPawn::AllowBump(class ADishonoredNPCPawn *)const
//   0x794370  public: virtual int __thiscall ADishonoredPawn::GetCurrentPower(void)const
//   0x7943a0  protected: virtual void __thiscall ADishonoredPawn::SwitchToPower(int, enum ADishonoredPawn::EEquipItemFlags)
//   0x794440  public: virtual class FVector __thiscall ADishonoredPawn::GetLookAtPoint(void)const
//   0x794540  public: virtual unsigned int __thiscall ADishonoredPawn::IsVulnerableToInstaDeath_Derived(enum EDisInstaDeathType)const
//   0x7945b0  public: virtual enum eDisPawnHitReactionType __thiscall ADishonoredPawn::IsVulnerableToHitReact_Derived(enum eDisPawnHitReactionType)const
//   0x794600  public: class FName __thiscall ADishonoredPawn::GetAnchorBoneName(void)const
//   0x794690  public: virtual unsigned short __thiscall ADishonoredPawn::GetAnchorBoneIndex(class USkeletalMeshComponent *)const
//   0x7946c0  public: virtual void __thiscall ADishonoredPawn::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x794780  public: unsigned int __thiscall ADishonoredPawn::IsBlinking(void)const
//   0x7995f0  public: virtual void __thiscall ADishonoredPawn::PostBeginPlay(void)
//   0x799760  public: virtual void __thiscall ADishonoredPawn::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7999d0  public: virtual void __thiscall ADishonoredPawn::AddAssociatedActor(class AActor *)
//   0x7999f0  public: virtual unsigned int __thiscall ADishonoredPawn::CanSplash(void)
//   0x799aa0  public: void __thiscall ADishonoredPawn::ForceDestroyAssociatedActors(void)
//   0x79b360  public: virtual void __thiscall ADishonoredPawn::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x79d300  public: virtual void __thiscall ADishonoredPawn::ClearComponents(void)
//   0x79d360  private: void __thiscall ADishonoredPawn::ShutPawnDown(void)
//   0x79ed70  private: virtual void __thiscall ADishonoredPawn::MAT_OnDestroyPreviewPawn(void)
//   0x79ed80  private: virtual void __thiscall ADishonoredPawn::OnActorTerminated(void)
//   0x79eda0  public: void __thiscall ADishonoredPawn::PreGameSave(void)
//   0x7a2d60  protected: virtual void __thiscall ADishonoredPawn::ApplyTweakChanges_Derived(void)
//   0x7a5b10  public: static class UClass * __cdecl ADishonoredPawn::GetPrivateStaticClassADishonoredPawn(wchar_t const *)
//   0x7a5d10  public: static class UClass * __cdecl ADishonoredPawn::StaticClassNoInline(void)
//   0x7a5fd0  protected: virtual void __thiscall ADishonoredPawn::HandleBendtimeTouch(class AActor *)
//   0x7a6190  protected: virtual void __thiscall ADishonoredPawn::OnOtherActorTerminated(class AActor const &)

#include "DishonoredGame.h"

// DISHONORED(written): exec 2013 rva 0x5ecc30 -> vtable +1380 = 2013 rva 0x74d580 (2013 only; not overridden in retail): the
// action is taken as a UDisSeqAct_DLC05_SetStoryGroup without a test, as retail does
void ADishonoredPawn::execOnDLC05SetStoryGroup( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, _pAction);
	P_FINISH;
	m_pCurStoryGroupTweak = ((UDisSeqAct_DLC05_SetStoryGroup*)_pAction)->m_pStoryGroup;
}

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5ec2c0; the retail ADishonoredPawn vtable slot +1312 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void ADishonoredPawn::execDisplayDebug_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AHUD, _pHUD);
	P_GET_FLOAT_REF(_rfOut_YL);
	P_GET_FLOAT_REF(_rfOut_YPos);
	P_FINISH;
}

// ---- end of trivial natives ----
