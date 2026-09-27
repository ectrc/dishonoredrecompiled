// DishonoredGame/src/dishonorednpcpawn_body.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x7abb70  public: static void __cdecl UDisTweaks_ComponentMeshOffset::InitializePrivateStaticClassUDisTweaks_ComponentMeshOffset(void)
//   0x7abb90  public: enum eDisNPCBodyStance __thiscall ADishonoredNPCPawn::GetCurrentManifestedBodyStance(void)const
//   0x7abba0  public: void __thiscall ADishonoredNPCPawn::SetSpineBendingTarget(class FVector const &)
//   0x7abbd0  public: void __thiscall ADishonoredNPCPawn::ClearSpineBendingTarget(void)
//   0x7abbe0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsCorpseDetectable(void)const
//   0x7abc10  public: virtual class IDisRelationshipInterface * __thiscall ADishonoredNPCPawn::GetRelationshipInterface(void)
//   0x7abc30  public: void __thiscall ADishonoredNPCPawn::SetCurrentMurderer(class ADishonoredPawn *)
//   0x7abc40  public: class UStaticMeshComponent * __thiscall ADishonoredNPCPawn::GetAccessoryStaticMeshComponent(enum eDisAccessoryType)const
//   0x7adcc0  class FArchive & __cdecl operator<<(class FArchive &, struct FDisMaterialReplacement &)
//   0x7adde0  protected: class FVector __thiscall ADishonoredNPCPawn::ComputeTurnAngle(class UDisTweaks_NPCPawn_Body const * const, float &)const
//   0x7ae310  public: void __thiscall ADishonoredNPCPawn::SetBodyIntention(enum EDisBodyIntentionPriority, enum eDisNPCBodyStance, class UClass * const, class UClass * const)
//   0x7ae420  public: enum eDisNPCBodyStance __thiscall ADishonoredNPCPawn::GetBodyStance(void)const
//   0x7ae440  public: class UClass * __thiscall ADishonoredNPCPawn::GetDesiredPrimaryItem(void)const
//   0x7ae460  public: class UClass * __thiscall ADishonoredNPCPawn::GetDesiredSecondaryItem(void)const
//   0x7ae480  public: void __thiscall ADishonoredNPCPawn::ClearBodyIntention(enum EDisBodyIntentionPriority)
//   0x7ae4a0  public: void __thiscall ADishonoredNPCPawn::GoToLimpState(class ADishonoredPawn *)
//   0x7ae500  public: virtual void __thiscall ADishonoredNPCPawn::ModifyMovementExtents(class FVector &, class FVector &, class FVector &, class FVector &, unsigned int &)const
//   0x7ae530  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsCorpseKnockedOut(void)const
//   0x7ae560  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanDeathBeWitnessed(void)const
//   0x7ae590  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAttnTargetBeingMurdered(class IDisAttentionTargetInterface * *)const
//   0x7ae5c0  private: virtual void __thiscall ADishonoredNPCPawn::OnExitStealthVolume(class ADisStealthVolume *)
//   0x7ae5f0  private: class UStaticMeshComponent * __thiscall ADishonoredNPCPawn::CreateAccessoryStaticMeshComponent(enum eDisAccessoryType)
//   0x7b29b0  public: void __thiscall ADishonoredNPCPawn::RestoreAccessories(int const *, int const *)
//   0x7b2a70  protected: unsigned int __thiscall ADishonoredNPCPawn::IsFootPlacementEnabled(void)const
//   0x7b2b40  protected: unsigned int __thiscall ADishonoredNPCPawn::IsActionSpineBendingEnabled(void)const
//   0x7b2bf0  protected: void __thiscall ADishonoredNPCPawn::Tick_NPCFootPlacement(float)
//   0x7b2cc0  public: virtual void __thiscall ADishonoredNPCPawn::NPCTouchedDeepWater(void)
//   0x7b2e20  public: unsigned int __thiscall ADishonoredNPCPawn::AllowSurfacesDisableFootPlacement(void)const
//   0x7b2ec0  public: virtual class UClass * __thiscall ADishonoredNPCPawn::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x7b6ec0  public: void __thiscall FDisMaterialsOverride::Set(class UMaterialInterface *, class UMaterialInterface *)
//   0x7b7050  protected: unsigned int __thiscall ADishonoredNPCPawn::IsSlopeOffsettingEnabled(void)const
//   0x7b7120  protected: void __thiscall ADishonoredNPCPawn::Tick_SpineBending_Apply(float)
//   0x7bbf50  public: void __thiscall FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh(class USkeletalMeshComponent *, struct FDisMaterialsOverride &)const
//   0x7bc110  public: static void __cdecl FDisMeshMaterialVariationList::RestoreMaterialVariationToMesh(class USkeletalMeshComponent *, struct FDisMaterialsOverride const &)
//   0x7bc180  public: virtual void __thiscall ADishonoredNPCPawn::PostBeginPlay_Body(void)
//   0x7bc480  protected: void __thiscall ADishonoredNPCPawn::Tick_SpineBending(float)
//   0x7be180  public: virtual void __thiscall ADishonoredNPCPawn::Tick_Body(float, enum ELevelTick)
//   0x7be670  public: void __thiscall ADishonoredNPCPawn::RestoreAppearance(int, struct FDisMaterialsOverride const &, struct FDisMaterialsOverride const &, int const *, int const *)
//   0x7bf2c0  public: static class UClass * __cdecl UDisTweaks_ComponentMeshOffset::GetPrivateStaticClassUDisTweaks_ComponentMeshOffset(wchar_t const *)
//   0x7c11d0  public: static class UClass * __cdecl UDisTweaks_ComponentMeshOffset::StaticClassNoInline(void)
//   0x7c1200  public: virtual unsigned int __thiscall ADishonoredNPCPawn::OnTeleport_Native(class USeqAct_Teleport *)
//   0x7c42c0  protected: virtual class UClass * __thiscall ADishonoredNPCPawn::ChooseFootfallContactType(void)const
//   0x7c4320  protected: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAutoFootfallEnabled(void)const
//   0x7c94a0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IgnoreBlockingBy(class AActor const *)const

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF) ----

// DISHONORED(written): the generated exec wrapper of ADishonoredNPCPawn's own script declaration of
// TakeFallingDamage_Native. The class has no C++ override - the 2012 PDB has no
// ADishonoredNPCPawn::TakeFallingDamage_Native symbol - so the exec dispatches through the virtual to
// ADishonoredPawn::TakeFallingDamage_Native (2013 rva 0x74dcc0), which is why ICF folded this wrapper onto
// ADishonoredPawn::execTakeFallingDamage_Native (2013 rva 0x5ec5f0).
void ADishonoredNPCPawn::execTakeFallingDamage_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FVector, HitNormal);
	P_GET_ACTOR(FloorActor);
	P_FINISH;
	*(INT*)Result = TakeFallingDamage_Native( HitNormal, FloorActor );
}
