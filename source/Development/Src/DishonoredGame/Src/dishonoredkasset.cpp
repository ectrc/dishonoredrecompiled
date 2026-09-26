// DishonoredGame/src/dishonoredkasset.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (16):
//   0x669960  public: static void __cdecl ADishonoredKAsset::InitializePrivateStaticClassADishonoredKAsset(void)
//   0x669980  public: virtual void __thiscall ADishonoredKAsset::physRigidBody(float)
//   0x6699d0  public: virtual unsigned int __thiscall ADishonoredKAsset::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x669a00  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredKAsset::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x66a860  public: virtual unsigned int __thiscall UObject::IsRefSaveable(enum ESaveLoadLocation)const
//   0x66bc70  public: virtual void __thiscall ADishonoredKAsset::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x66bd60  public: virtual unsigned int __thiscall ADishonoredKAsset::Tick(float, enum ELevelTick)
//   0x66f7e0  public: virtual void __thiscall ADishonoredKAsset::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x66f840  public: void __thiscall ADishonoredKAsset::PropagateMaxDrawDistance(void)
//   0x66f960  public: virtual void __thiscall ADishonoredKAsset::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66f9e0  public: virtual void __thiscall ADishonoredKAsset::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x66fa70  public: virtual class UClass * __thiscall ADishonoredKAsset::GetContactTypeOverride(void)const
//   0x672c90  public: virtual void __thiscall ADishonoredKAsset::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x672d40  public: virtual void __thiscall ADishonoredKAsset::PostBeginPlay(void)
//   0x677f80  public: static class UClass * __cdecl ADishonoredKAsset::GetPrivateStaticClassADishonoredKAsset(wchar_t const *)
//   0x67a1f0  public: static class UClass * __cdecl ADishonoredKAsset::StaticClassNoInline(void)
// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x6211d0 (2012 0x66f7e0): as ADishonoredKActor's, except that the blame flag is
// assigned rather than accumulated, and the damage goes to this class's TakeDamage_Impl (retail vtable +932).
void ADishonoredKAsset::TakeDamage_Native( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const FTraceHitInfo& HitInfo, AActor* const DamageCauser )
{
	m_bPlayerDeservesBlame = ( Cast<ADishonoredPlayerPawn>( DamageCauser ) != NULL );
	TakeDamage_Impl( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
}

void ADishonoredKAsset::execTakeDamage_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(Damage);
	P_GET_OBJECT(AController,InstigatedBy);
	P_GET_STRUCT(FVector,HitLocation);
	P_GET_STRUCT(FVector,Momentum);
	P_GET_OBJECT(UClass,DamageType);
	P_GET_STRUCT_OPTX(FTraceHitInfo,HitInfo,FTraceHitInfo(EC_EventParm));
	P_GET_OBJECT_OPTX(AActor,DamageCauser,NULL);
	P_FINISH;
	TakeDamage_Native( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
}
