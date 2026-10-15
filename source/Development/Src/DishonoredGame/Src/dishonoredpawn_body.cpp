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

// ---- agent DI ports (PHASE10 DI): the body pass and the bone lookup the appearance path needs ----

// DISHONORED(port): 2013 rva 0x76a130 (2012 0x7a4b70). The base body pass: the two eye skel controls out of the body
// tweaks, and the cached-position pair every prediction query reads back.
// DISHONORED(bringup): two parts of retail's body need unported subsystems and are named rather than skipped in
// silence: ADishonoredPawn::SetupHitRegions (2013 rva 0x769f30; the per-region hit table, so a headshot
// still counts as a body shot - the same gap agent CG recorded for GetBoneRegionInfo), and the m_Foot_Caches build-out,
// which needs FDisFootCache::Init (2013 rva 0x750fe0) and the foot-placement component.
void ADishonoredPawn::PostBeginPlay_Body()
{
	UDisTweaks_Pawn_Body* BodyTweaks = GetPawnTweaks()->m_pBodyTweaks;
	if( Mesh && BodyTweaks )
	{
		if( BodyTweaks->m_EyeControl_Left != NAME_None )
		{
			m_pEyeControl_Left = Cast<USkelControlSingleBone>( Mesh->FindSkelControl( BodyTweaks->m_EyeControl_Left ) );
		}
		if( BodyTweaks->m_EyeControl_Right != NAME_None )
		{
			m_pEyeControl_Right = Cast<USkelControlSingleBone>( Mesh->FindSkelControl( BodyTweaks->m_EyeControl_Right ) );
		}
	}
	m_fPredictionTimer = 0.f;
	m_fPredictionConfidence = 0.f;
	// retail reads UWorld::GetBendTimeSeconds(); this tree keeps the value on AWorldInfo (BendTimeSeconds @920)
	AWorldInfo* Info = GWorld ? GWorld->GetWorldInfo() : NULL;
	m_CurrentCachedPosition.m_CachedPositionTimeStamp = Info ? Info->BendTimeSeconds : 0.f;
	m_CurrentCachedPosition.m_CachedPosition = CollisionComponent ? CollisionComponent->Bounds.Origin : Location;
	m_CurrentCachedPosition.m_CachedVelocity = FVector(0.f,0.f,0.f);
	m_LastCachedPosition = m_CurrentCachedPosition;
}

// DISHONORED(port): 2013 rva 0x757730 (2012 0x7956a0): a bone of the pawn's body mesh by name, as a world position
// and/or a rotation. Both outputs are optional and a mesh that has no such bone leaves them untouched.
// DISHONORED(port): agent FZ (PHASE18 FZ) - the rotation branch as retail has it: the bone's component-space matrix
// (USkeletalMeshComponent::GetBoneMatrixLocal 2012 0x33afa0: SpaceBases(Bone) as a matrix, identity out of range),
// PerformAngleOffsetFixup( TRUE ) - which also takes it to world space - then the origin and the rotator off that one
// matrix. Without the fixup the player's camera bone answers a 90-degree pitch (agent FZ measured 16358 units on the
// boat ride), which is the bone's axis convention the body tweaks' m_fBone_AngleOffset_* undo. The position-only branch
// reads the world bone matrix directly, as retail does.
void ADishonoredPawn::GetBone_ByName( FName _BoneName, FVector* _pPos, FRotator* _pRotator ) const
{
	if( ( !_pPos && !_pRotator ) || !Mesh )
	{
		return;
	}
	const INT BoneIndex = Mesh->MatchRefBone( _BoneName );
	if( BoneIndex == INDEX_NONE )
	{
		return;
	}
	if( _pRotator )
	{
		FMatrix BoneMatrix = FMatrix::Identity;
		if( BoneIndex < Mesh->SpaceBases.Num() )
		{
			BoneMatrix = Mesh->SpaceBases(BoneIndex).ToMatrix();
		}
		PerformAngleOffsetFixup( BoneMatrix, TRUE );
		if( _pPos )
		{
			*_pPos = BoneMatrix.GetOrigin();
		}
		*_pRotator = BoneMatrix.Rotator();
		_pRotator->MakeShortestRoute();
	}
	else if( _pPos )
	{
		*_pPos = Mesh->GetBoneMatrix( BoneIndex ).GetOrigin();
	}
}

/**
 * DISHONORED(port): agent FZ, 2012 0x795080 (2013 0x757110). With no angle offset in the body tweaks an object-space
 * matrix is just taken to world space (times the mesh's LocalToWorld) and a world-space one is left alone. With an
 * offset: a world-space matrix is first taken to object space (times the inverse of LocalToWorld), the rotation part is
 * multiplied by the tweaks' m_AngleOffsetMatrix with the origin held aside, and the result goes to world space.
 * The body tweaks fall back to UDisTweaks_Pawn's class default, as every retail tweak read does.
 */
void ADishonoredPawn::PerformAngleOffsetFixup( FMatrix& InOut, UBOOL bObjectSpace ) const
{
	const UDisTweaks_Pawn* Tweaks = GetPawnTweaks();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_Pawn*)UDisTweaks_Pawn::StaticClass()->GetDefaultObject();
	}
	const UDisTweaks_Pawn_Body* Body = Tweaks ? Tweaks->m_pBodyTweaks : NULL;
	const FMatrix& LocalToWorld = Mesh ? Mesh->LocalToWorld : FMatrix::Identity;
	if( !Body || ( Body->m_fBone_AngleOffset_Pitch == 0.f && Body->m_fBone_AngleOffset_Yaw == 0.f && Body->m_fBone_AngleOffset_Roll == 0.f ) )
	{
		if( bObjectSpace )
		{
			InOut = InOut * LocalToWorld;
		}
		return;
	}
	if( !bObjectSpace )
	{
		InOut = InOut * LocalToWorld.InverseSafe();
	}
	const FVector OldOrigin = InOut.GetOrigin();
	InOut.SetOrigin( FVector::ZeroVector );
	InOut = Body->m_AngleOffsetMatrix * InOut;
	InOut.SetOrigin( OldOrigin );
	InOut = InOut * LocalToWorld;
}

// ---- agent FV (PHASE18 FV): the body pass ----

// DISHONORED(port): 2013 rva 0x7510b0 (2012 0x7959c0, 31 B): the cached-position pass runs unless the retargeting component
// says the pawn is being retargeted (FDisComponentRetargeting::m_bIsRetargeting; agent HC ported the component).
void ADishonoredPawn::Tick_Body( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	if( !IsRetargeting() )
	{
		Tick_Body_CachedPositions( _fDeltaTime );
	}
}

// DISHONORED(port): 2013 rva 0x74d850 (2012 0x791470, 390 B): the position-prediction pair. Every m_fPredictionInterval
// seconds the current cache becomes the last; the current is refreshed every frame with the collision origin and the
// velocity; the prediction confidence grows while the velocity keeps its direction and drops to zero when it turns back.
void ADishonoredPawn::Tick_Body_CachedPositions( FLOAT _fDeltaTime )
{
	m_fPredictionTimer -= _fDeltaTime;
	if( m_fPredictionTimer < 0.f )
	{
		m_fPredictionTimer = m_fPredictionInterval;
		m_LastCachedPosition = m_CurrentCachedPosition;
	}
	// retail reads UWorld::GetBendTimeSeconds(); this tree keeps the value on AWorldInfo (BendTimeSeconds @920)
	AWorldInfo* Info = GWorld ? GWorld->GetWorldInfo() : NULL;
	m_CurrentCachedPosition.m_CachedPositionTimeStamp = Info ? Info->BendTimeSeconds : 0.f;
	m_CurrentCachedPosition.m_CachedPosition = CollisionComponent ? CollisionComponent->Bounds.Origin : Location;
	m_CurrentCachedPosition.m_CachedVelocity = Velocity;

	FVector CurrentDirection = m_CurrentCachedPosition.m_CachedVelocity;
	CurrentDirection.Normalize( 1.e-8f );
	FVector LastDirection = m_LastCachedPosition.m_CachedVelocity;
	LastDirection.Normalize( 1.e-8f );
	const FLOAT Dot = CurrentDirection | LastDirection;
	FLOAT Confidence = 0.f;
	if( Dot > 0.f )
	{
		Confidence = Min( m_fPredictionConfidence + _fDeltaTime / m_fPredictionInterval, 1.f ) * Dot;
	}
	m_fPredictionConfidence = Confidence;
}

// DISHONORED(port): 2013 rva 0x7689a0 (2012 0x7a1560): GetFocusBone_ByRegion( DHR_TORSO, eDisRegionFocusType_MeleeAttack ),
// the torso's melee focus bone out of the hit-region table.
// DISHONORED(bringup): the hit-region table (SetupHitRegions 2013 0x769f30, GetFocusBone_ByRegion 2012 0x7a0bd0) is not
// ported (agent DI's note on PostBeginPlay_Body), so the reference point is the pawn's location. It is read by the
// action spine bending alone, which nothing requests yet (SetSpineBendingTarget has no caller in this tree).
FVector ADishonoredPawn::GetSpineBendRefPoint() const
{
	return Location;
}

// ---- agent GF (PHASE19 GF): the component tick-end phase ----

// DISHONORED(port): 2012 0x799d10 (71 B; 2013 unnamed, reached through the vtable): after the base, a component that is not slaved to a
// parent animation component runs the tick-end hook chain. Retail also skips it under a per-component flag this tree does not name
// (the dword after m_pSoulMaterial) and then syncs the NPC's physics sensor (ADishonoredNPCPawn::SyncPhysicsSensor, unported).
void UDisSkeletalMeshComponent::TickEnd( FLOAT DeltaTime )
{
	Super::TickEnd( DeltaTime );
	if( !ParentAnimComponent )
	{
		UpdateChildForTickEnd();
	}
}

// DISHONORED(port): 2012 0x794e50 / 2013 0x750f20 (179 B, same bytes): the derived hook, then every attached player skeletal component's
void UDisSkeletalMeshComponent::UpdateChildForTickEnd()
{
	UpdateChildForTickEnd_Derived();
	for( INT Index = 0; Index < Attachments.Num(); Index++ )
	{
		UDishonoredPlayerSkeletalComponent* Child = Cast<UDishonoredPlayerSkeletalComponent>( Attachments(Index).Component );
		if( Child )
		{
			Child->UpdateChildForTickEnd();
		}
	}
}
