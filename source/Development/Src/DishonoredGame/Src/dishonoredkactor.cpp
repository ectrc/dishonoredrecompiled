// DishonoredGame/src/dishonoredkactor.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21):
//   0x669850  public: static void __cdecl ADishonoredKActor::InitializePrivateStaticClassADishonoredKActor(void)
//   0x669870  public: virtual void __thiscall ADishonoredKActor::DestroyIfPlayerCantSeeMe(void)
//   0x669890  public: virtual unsigned int __thiscall ADishonoredKActor::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x6698c0  public: static void __cdecl ADishonoredKActor::CollideContactSystemEnd(struct FDisPhysicsContactInfo &)
//   0x669910  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredKActor::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x669930  public: void __thiscall ADishonoredKActor::SetPlayerDeservesBlame(unsigned int)
//   0x669950  public: unsigned int __thiscall ADishonoredKActor::DoesPlayerDeserveBlame(void)const
//   0x66bad0  public: virtual void __thiscall ADishonoredKActor::physRigidBody(float)
//   0x66bb20  public: virtual void __thiscall ADishonoredKActor::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x66bc10  public: virtual class ADishonoredPawn * __thiscall ADishonoredKAsset::GetNoiseMakerPawn_Derived(void)const
//   0x66bc20  protected: virtual class UDisTweaksBase * __thiscall ADishonoredKActor::GetTweaks_Derived(void)
//   0x66bc40  public: virtual void __thiscall ADishonoredKActor::OnRigidBodyStatusChange(void)
//   0x66f270  public: static void __cdecl ADishonoredKActor::FindRelativeVelocity(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &, unsigned int, class FVector &, float &, float &, unsigned int &)
//   0x66f620  public: virtual void __thiscall ADishonoredKActor::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x66f690  public: virtual class UClass * __thiscall ADishonoredKActor::GetContactTypeOverride(void)const
//   0x66f6c0  public: void __thiscall ADishonoredKActor::PropagateMaxDrawDistance(void)
//   0x6729a0  public: static void __cdecl ADishonoredKActor::CollideContactSystem(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &, struct FDisPhysicsContactInfo &, class UClass *)
//   0x672c80  public: virtual void __thiscall ADishonoredKActor::PostBeginPlay(void)
//   0x673fb0  public: virtual void __thiscall ADishonoredKActor::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x677ef0  public: static class UClass * __cdecl ADishonoredKActor::GetPrivateStaticClassADishonoredKActor(wchar_t const *)
//   0x67a1c0  public: static class UClass * __cdecl ADishonoredKActor::StaticClassNoInline(void)

// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x6210b0 (2012 0x66f6c0): the tweaks' max draw distance is pushed onto every primitive
// of the actor, each inside a reattach context so the scene proxy is rebuilt with it.
void ADishonoredKActor::PropagateMaxDrawDistance()
{
	UDisTweaksBase* Tweaks = GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaksBase*)UDisTweaks_KActor::StaticClass()->GetDefaultObject();
	}
	const FLOAT MaxDrawDist = ((UDisTweaks_KActor*)Tweaks)->m_fMaxDrawDistance;
	if( MaxDrawDist <= 0.f )
	{
		return;
	}
	for( INT Idx = Components.Num() - 1; Idx >= 0; Idx-- )
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>( Components(Idx) );
		if( Primitive )
		{
			FComponentReattachContext ReattachContext( Primitive );
			Primitive->CachedMaxDrawDistance = MaxDrawDist;
		}
	}
}

// DISHONORED(written): 2013 rva 0x621010 (2012 0x66f620): a hit whose causer is the player pawn makes the player to
// blame for whatever this object goes on to do (the AI's murder / destruction accounting reads the flag), and the damage
// itself goes to the class's own TakeDamage_Impl (retail vtable +940).
void ADishonoredKActor::TakeDamage_Native( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const FTraceHitInfo& HitInfo, AActor* const DamageCauser )
{
	m_bPlayerDeservesBlame = m_bPlayerDeservesBlame || ( Cast<ADishonoredPlayerPawn>( DamageCauser ) != NULL );
	TakeDamage_Impl( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
}

void ADishonoredKActor::execTakeDamage_Native( FFrame& Stack, RESULT_DECL )
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

// DISHONORED(written): 2013 rva 0x6185f0 (2012 0x669870, same bytes): the Kismet "clean this up when nobody is looking"
// native. DISHONORED(bringup): retail's tail is DisDestroyActor( *this, FALSE, TRUE ) in dishonoredutilities.cpp, whose
// deferred-destroy half is not ported; the engine's own Destroy() is used, which is what that helper reduces to for an
// actor with no save state.
void ADishonoredKActor::DestroyIfPlayerCantSeeMe()
{
	if( !PlayerCanSeeMe() )
	{
		GWorld->DestroyActor( this );
	}
}

void ADishonoredKActor::execDestroyIfPlayerCantSeeMe( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	DestroyIfPlayerCantSeeMe();
}
