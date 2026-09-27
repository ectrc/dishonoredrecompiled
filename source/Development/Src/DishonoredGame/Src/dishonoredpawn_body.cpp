// DishonoredGame/src/dishonoredpawn_body.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (49):
//   0x78e900  public: static void __cdecl UDisSkeletalMeshComponent::InitializePrivateStaticClassUDisSkeletalMeshComponent(void)
//   0x78e920  public: void __thiscall ADishonoredPawn::LockMeshTranslation(unsigned int, enum ADishonoredPawn::eDisMeshOffsetDisableFlag)
//   0x78e960  public: void __thiscall ADishonoredPawn::SetMeshTranslation(class FVector const &, float)
//   0x78e9c0  public: void __thiscall ADishonoredPawn::ImpulsePawn(class FVector const &)
//   0x78ea60  public: virtual class FVector __thiscall ADishonoredPawn::GetCameraPos(void)const
//   0x78ea90  public: class FVector __thiscall ADishonoredPawn::GetPredictedLocation(float)const
//   0x7911f0  protected: virtual void __thiscall UDisTweaks_Pawn_Body::ApplyFallbackChain_Derived(void)
//   0x791280  public: unsigned int __thiscall UDisSkeletalMeshComponent::DisLegLineCheck(class FVector const &, class FVector const &, class FVector &, class FVector &, class FVector const &, struct FCheckResult &)
//   0x791380  public: virtual void __thiscall ADishonoredPawn::SetBase(class AActor *, class FVector, int, class USkeletalMeshComponent *, class FName)
//   0x791470  protected: void __thiscall ADishonoredPawn::Tick_Body_CachedPositions(float)
//   0x791600  public: virtual unsigned int __thiscall ADishonoredPawn::ResolveAttachedMoveEncroachment(class AActor *, struct FCheckResult const &)
//   0x791870  public: virtual class UClass * __thiscall ADishonoredPawn::GetContactTypeOverride(void)const
//   0x7918e0  public: virtual int __thiscall ADishonoredPawn::TakeFallingDamage_Native(class FVector, class AActor *)
//   0x794d20  public: void __thiscall FDisRegionCached::Set(struct FDisRegionInfo const *, class USkeletalMeshComponent *)
//   0x794dd0  public: virtual unsigned int __thiscall UDisSkeletalMeshComponent::LegLineCheck(class FVector const &, class FVector const &, class FVector &, class FVector &, class FVector const &)
//   0x794e50  public: void __thiscall UDisSkeletalMeshComponent::UpdateChildForTickEnd(void)
//   0x794f10  public: void __thiscall FDisFootCache::Init(struct FDisFootInfo const &, class USkeletalMeshComponent const *)
//   0x794fd0  public: void __thiscall ADishonoredPawn::ApplyTweakChanges_Body(void)
//   0x795080  public: void __thiscall ADishonoredPawn::PerformAngleOffsetFixup(class FMatrix &, unsigned int)const
//   0x795630  public: void __thiscall ADishonoredPawn::GetBone_ByName(class FName, class FMatrix *)const
//   0x7956a0  public: void __thiscall ADishonoredPawn::GetBone_ByName(class FName, class FVector *, class FRotator *)const
//   0x7957d0  protected: void __thiscall ADishonoredPawn::GetFocusBoneHelper(class FName const &, class FVector const &, class FVector *, class FRotator *)const
//   0x7959c0  public: virtual void __thiscall ADishonoredPawn::Tick_Body(float, enum ELevelTick)
//   0x7959e0  protected: virtual unsigned int __thiscall ADishonoredPawn::IsAutoFootfallEnabled(void)const
//   0x795a30  protected: virtual class UClass * __thiscall ADishonoredPawn::ChooseFootfallContactType(void)const
//   0x795a80  public: void __thiscall ADishonoredPawn::DoFootLock_Notify(unsigned int, int)
//   0x795b80  public: unsigned int __thiscall FDisFootCache::UpdateFootfallCache(struct FDisFootInfo const &, class USkeletalMeshComponent *, float, class FVector *)
//   0x795f70  public: virtual void __thiscall ADishonoredPawn::CrushedBy_Native(class APawn *)
//   0x799d10  public: virtual void __thiscall UDisSkeletalMeshComponent::TickEnd(float)
//   0x799d60  public: virtual unsigned int __thiscall ADishonoredPawn::OnTeleport_Native(class USeqAct_Teleport *)
//   0x79d690  public: void __thiscall UDisSkeletalMeshComponent::SetupUsedMaterials(void)
//   0x79d7c0  public: void __thiscall ADishonoredPawn::DoFootfallDamage(class UClass *, class FVector const &, float, int)
//   0x79ee50  public: enum eDisHitRegion __thiscall ADishonoredPawn::GetBoneRegion(class FName)const
//   0x79eea0  public: struct FDisRegionInfo const * __thiscall ADishonoredPawn::GetBoneRegionInfo(class FName)const
//   0x79ef60  public: struct FDisRegionInfo const * __thiscall ADishonoredPawn::GetHitRegionInfo(enum eDisHitRegion)const
//   0x79efa0  public: unsigned int __thiscall ADishonoredPawn::IsBoneInRegion(class FName, enum eDisHitRegion)const
//   0x79efd0  public: void __thiscall ADishonoredPawn::DoFootfallContact(int, class FVector const *)
//   0x79f560  public: void __thiscall ADishonoredPawn::DoFootfallContact_Notify(int)
//   0x79f640  public: virtual class UClass * __thiscall ADishonoredPawn::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x7a0bd0  public: void __thiscall ADishonoredPawn::GetFocusBone_ByRegion(enum eDisHitRegion, enum eDisRegionFocusType, class FVector *, class FRotator *)const
//   0x7a0c90  public: void __thiscall ADishonoredPawn::GetNearestFocusBoneToCam(enum eDisHitRegion, enum eDisRegionFocusType, class FVector const &, class FVector const &, class FVector *, class FRotator *)const
//   0x7a1250  public: unsigned int __thiscall ADishonoredPawn::GetBBox_ForRegion(enum eDisHitRegion, struct FBoxSphereBounds &)const
//   0x7a13e0  protected: virtual void __thiscall ADishonoredPawn::OnMeshPrecompose(void)
//   0x7a1560  public: class FVector __thiscall ADishonoredPawn::GetSpineBendRefPoint(void)const
//   0x7a4150  public: static class UClass * __cdecl UDisSkeletalMeshComponent::GetPrivateStaticClassUDisSkeletalMeshComponent(wchar_t const *)
//   0x7a4940  public: static class UClass * __cdecl UDisSkeletalMeshComponent::StaticClassNoInline(void)
//   0x7a4970  public: void __thiscall ADishonoredPawn::SetupHitRegions(void)
//   0x7a4b70  public: virtual void __thiscall ADishonoredPawn::PostBeginPlay_Body(void)
//   0x7a6360  public: virtual void __thiscall UDisSkeletalMeshComponent::PreComposeSkeleton(void)

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): the fall-damage native ----

// DISHONORED(written): 2013 rva 0x74dcc0 (2012 0x7918e0). The fall speed is -Velocity.Z; the two thresholds are the
// attributes Attribute_MaxSpeedBeforeFallingDamage and Attribute_MaxSpeedBeforeFallingDeath, sorted so the smaller is
// the no-damage floor and the larger the lethal ceiling; the damage is HealthMax (an INT, @840) scaled by where the fall
// speed sits between them. Below the floor the native returns 0 without touching anything.
// This is the division agent AU warned about: with the attributes unset both thresholds are 0, every landing divides by
// zero, the alpha clamps to 1 and the pawn takes HealthMax damage. The whole point of porting the attributes system
// first is that these two reads now answer the cooked values.
// DISHONORED(port): the tail is the contact system - DisGetPhysicalMaterial (trace flags 0x20DF) picks the surface the
// pawn landed on, UDisContactType_BreakingBones_Player (m_ActorTypeFlags @266 == 36) or UDisContactType_BreakingBones is
// crossed with it, and UDishonoredContactSystem::ApplyContact plays the result. None of UDisContactType_*,
// FDisContactContext, DisGetPhysicalMaterial, DisConvertCheckResultToImpactInfo or DisGetContactSystem is ported (agent
// AU's root 4), so the damage is returned without the impact effect. The damage itself - the observable half - is exact.
INT ADishonoredPawn::TakeFallingDamage_Native( FVector HitNormal, AActor* FloorActor )
{
	// The attribute names are the member names of UDisTweaks_Pawn_Attributes with m_ replaced by Attribute_; the two
	// strings are in the retail exe at 0xcd25e0 and 0xcd2658 and retail reads them through the DNAME_Attribute_* globals.
	static const FName NAME_MaxSpeedBeforeFallingDamage( TEXT("Attribute_MaxSpeedBeforeFallingDamage") );
	static const FName NAME_MaxSpeedBeforeFallingDeath( TEXT("Attribute_MaxSpeedBeforeFallingDeath") );
	const FLOAT FallSpeed = -Velocity.Z;
	const FLOAT FallDamageSpeed = GetAttributeValue( NAME_MaxSpeedBeforeFallingDamage );
	const FLOAT FallDeathSpeed = GetAttributeValue( NAME_MaxSpeedBeforeFallingDeath );
	const FLOAT FallMin = Min<FLOAT>( FallDamageSpeed, FallDeathSpeed );
	const FLOAT FallMax = Max<FLOAT>( FallDamageSpeed, FallDeathSpeed );
	if( FallSpeed <= FallMin )
	{
		if( UDisAttributes::IsCensusEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): disattrib fall %s: speed %.1f, no damage below %.1f (death at %.1f)"),
				*GetName(), FallSpeed, FallMin, FallMax );
		}
		return 0;
	}
	if( FallMax <= FallMin )
	{
		// DISHONORED(bringup): retail divides by (FallMax - FallMin) unguarded, because its two thresholds always come from
		// the cooked pawn attribute tweaks and always differ. Both being equal means the attribute set is missing or
		// unauthored, and retail's arithmetic would then divide by zero, clamp the alpha to 1 and take HealthMax damage -
		// i.e. kill the pawn on every landing. That is exactly why these two natives were left stubbed before this
		// package; the degenerate case answers "no damage" and says so once.
		static UBOOL bNamed = FALSE;
		if( !bNamed )
		{
			bNamed = TRUE;
			debugf( TEXT("DISHONORED(bringup): %s has no fall-damage thresholds (Attribute_MaxSpeedBeforeFallingDamage and _Death both %.1f): no fall damage"),
				*GetName(), FallMin );
		}
		return 0;
	}
	const FLOAT Alpha = Clamp<FLOAT>( ( FallSpeed - FallMin ) / ( FallMax - FallMin ), 0.f, 1.f );
	const INT DamageToTake = appTrunc( (FLOAT)HealthMax * Alpha );
	if( UDisAttributes::IsCensusEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): disattrib fall %s: speed %.1f in [%.1f .. %.1f] alpha %.3f -> damage %i of health %i/%i"),
			*GetName(), FallSpeed, FallMin, FallMax, Alpha, DamageToTake, Health, HealthMax );
	}
	return DamageToTake;
}

// DISHONORED(written): 2013 rva 0x5ec5f0 (2012 0x633170), the generated exec wrapper: a vector, an actor, then the
// virtual. ADishonoredNPCPawn's and ADishonoredPlayerPawn's copies are the same code and were folded by ICF.
void ADishonoredPawn::execTakeFallingDamage_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FVector, HitNormal);
	P_GET_ACTOR(FloorActor);
	P_FINISH;
	*(INT*)Result = TakeFallingDamage_Native( HitNormal, FloorActor );
}
