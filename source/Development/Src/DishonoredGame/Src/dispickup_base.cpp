// DishonoredGame/src/dispickup_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x66a830  public: virtual void __thiscall ADisPickup_Base::PostLoad(void)
//   0x66a870  protected: virtual void __thiscall ADisPickup_Base::TakeDamage_Impl(int, class AController * const, class FVector const &, class FVector const &, class UClass * const, struct FTraceHitInfo const &, class AActor * const)
//   0x66a8e0  public: virtual void __thiscall ADisPickup_Base::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x66a910  public: virtual void __thiscall ADisPickup_Base::DisableSoulRendering(void)
//   0x66a930  public: virtual class UPrimitiveComponent * __thiscall ADisPickup_Base::GetMovablePrimitiveComponent(void)const
//   0x66a940  public: virtual struct FBoxSphereBounds __thiscall ADisPickup_Base::GetMovableBodyBounds(void)const
//   0x66a990  protected: virtual void __thiscall ADisPickup_Base::HideHighlight(void)
//   0x66a9b0  protected: virtual unsigned int __thiscall ADisPickup_Base::WantsTick_Derived(void)const
//   0x66c7b0  public: virtual void __thiscall ADisPickup_Base::OnRigidBodyStatusChange(void)
//   0x66c820  public: virtual unsigned int __thiscall ADisPickup_Base::CanSplash(void)
//   0x66c840  public: void __thiscall ADisPickup_Base::ConsumePickup(void)
//   0x66c870  public: virtual void __thiscall ADisPickup_Base::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66c8d0  public: virtual unsigned int __thiscall ADisPickup_Base::WasJustThrownBy(class ADishonoredPawn const *)const
//   0x66c900  public: virtual unsigned int __thiscall ADisPickup_Base::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x66c9a0  protected: virtual void __thiscall ADisPickup_Base::ShowHighlight(class UMaterialInterface *)
//   0x66ca70  private: void __thiscall ADisPickup_Base::AdjustDetachedPosition(void)
//   0x6705b0  public: virtual unsigned int __thiscall ADisPickup_Base::IgnoreBlockingBy(class AActor const *)const
//   0x6705f0  public: void __thiscall ADisPickup_Base::Detach(void)
//   0x670680  public: virtual void __thiscall ADisPickup_Base::ClearComponents(void)
//   0x6706b0  public: virtual void __thiscall ADisPickup_Base::BaseChange(void)
//   0x676490  public: virtual class AActor * __thiscall ADisPickup_Base::GetMovableActor(void)
//   0x678bc0  public: static class UClass * __cdecl ADisPickup_Base::GetPrivateStaticClassADisPickup_Base(wchar_t const *)
//   0x67a360  public: static void __cdecl ADisPickup_Base::InitializePrivateStaticClassADisPickup_Base(void)
//   0x67b0a0  public: static class UClass * __cdecl ADisPickup_Base::StaticClassNoInline(void)
//   0x680050  protected: virtual void __thiscall ADisPickup_Base::ApplyTweakChanges_Derived(void)
//   0x6800d0  public: void __thiscall ADisPickup_Base::StartPickupTravel(class ADishonoredPawn *)
//   0x680260  protected: virtual void __thiscall ADisPickup_Base::OnActorTerminated(void)
//   0x6802c0  private: unsigned int __thiscall ADisPickup_Base::IsUseBlockedByPossession(class ADishonoredPlayerPawn const &)const
//   0x680320  protected: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisPickup_Base::GetInteractableTweaks_Derived(void)const
//   0x680350  protected: virtual unsigned int __thiscall ADisPickup_Base::AttemptCannotUseInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x6803c0  public: virtual void __thiscall ADisPickup_Base::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x680490  public: virtual void __thiscall ADisPickup_Base::PostGameLoad(enum ESaveLoadLocation)
//   0x6804b0  public: virtual unsigned int __thiscall ADisPickup_Base::HasSoul(int)const
//   0x680500  private: class ADishonoredPlayerPawn * __thiscall ADisPickup_Base::GetPlayerPawnFromUser(class ADishonoredPawn *)const
//   0x680580  public: virtual enum EMovableWeightClass __thiscall ADisPickup_Base::GetMovableWeightClass(void)const
//   0x6805c0  public: void __thiscall ADisPickup_Base::Attach(class AActor *, class USkeletalMeshComponent *)
//   0x680650  public: virtual void __thiscall ADisPickup_Base::Steal(class ADishonoredPlayerPawn *)
//   0x6806b0  public: virtual unsigned int __thiscall ADisPickup_Base::Tick(float, enum ELevelTick)
//   0x680980  public: virtual void __thiscall ADisPickup_Base::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x680a00  public: virtual void __thiscall ADisPickup_Base::PostBeginPlay(void)
//   0x682c50  protected: virtual enum eCrossHairStatus __thiscall ADisPickup_Base::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x682ca0  protected: virtual unsigned int __thiscall ADisPickup_Base::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)

// ---- agent AU ports (PHASE7 AU): the pickup collection path ----
//
// Every "2013 rva" below is retail2013_agentAU.i64 (decompiles in build/agentAU_decomp/r13); the "2012" rva in
// brackets is shipping2012_agentAU.i64, which carries the symbol names and is the readable version of the same
// function (build/agentAU_decomp/s12). Member offsets were resolved against resources/docs/types/retail_sdk_layout.json.
//
// What is NOT ported, and why (each is outside this package or has no ported callee):
//  * the Heart's marker list bookkeeping of PostBeginPlay (UDisGadget_Heart, UDisGFxMoviePlayerHUD::AddHeartMarker);
//  * ADishonoredPlayerPawn::IncrementStat (ePlayerStat_ItemsCollected / ePlayerStat_ItemsStolen), 2013 rva 0x6b9560,
//    still a comment-only unit;
//  * the possession branch of GetPlayerPawnFromUser / IsUseBlockedByPossession: ADishonoredPawn::DisIsPossessed,
//    GetPossessingPlayerPawn and IsPossessing are not ported, so a pickup is never possession-blocked yet;
//  * the PhysX body resync tail of AdjustDetachedPosition (URB_BodyInstance::GetUnrealWorldTM / NxActor).

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(retail): the seventh bit of Arkane's FDisPrimTraceMask has no name in Engine/Inc/UnLevel.h (which is not
// this package's file): 0x40000000 is m_bTraceForMove_NonPawn, the move-trace flag for a non-pawn mover.
#define DIS_TRACE_MOVE_NONPAWN 0x40000000

/*-----------------------------------------------------------------------------
	ADisPickup_Base
-----------------------------------------------------------------------------*/

// DISHONORED(written): the GetTweaks_Derived-or-class-default shape every ADisPickup_Base body opens with
UDisTweaks_PickupBase* ADisPickup_Base::GetPickupTweaks() const
{
	UDisTweaksBase* Tweaks = const_cast<ADisPickup_Base*>( this )->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaksBase*)UDisTweaks_PickupBase::StaticClass()->GetDefaultObject();
	}
	return (UDisTweaks_PickupBase*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x62b4c0 (2012 0x680a00): the max-draw-distance propagation, the Heart's target list,
// the idle particle system (not when a save is about to restore this object's state) and the first tick.
// DISHONORED(bringup): retail also refreshes the Heart gadget's marker list here (UDisGadget_Heart, HUD slot for
// AddHeartMarker); only m_HeartTargets is filled.
void ADisPickup_Base::PostBeginPlay()
{
	AActor::PostBeginPlay();
	PropagateMaxDrawDistance();

	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	if( ADishonoredPlayerPawn::s_pInstance && Tweaks->m_bTrackedByHeart )
	{
		ADishonoredPlayerPawn::s_pInstance->m_HeartTargets.AddItem( this );
	}

	if( !DisIsObjectStateGoingToBeRestored( this ) )
	{
		m_pParticleSystem = DishonoredSpawnEmitter( Tweaks->m_pParticleSystem, Location, Rotation, this, TRUE );
	}

	if( LightEnvironment )
	{
		LightEnvironment->bDynamic = TRUE;
	}
	SetTickIsDisabled( FALSE );
}

// DISHONORED(written): 2013 rva 0x619a60 (2012 0x66a830, same bytes)
void ADisPickup_Base::PostLoad()
{
	AActor::PostLoad();
	ApplyTweakChanges();
	if( LightEnvironment )
	{
		LightEnvironment->SetEnabled( TRUE );
	}
}

// DISHONORED(written): 2013 rva 0x62aac0 (2012 0x680050): the tweaks' static mesh, through SetStaticMesh while the
// component is attached and by direct assignment before that; m_bNoPhysics clears the actor's Physics.
void ADisPickup_Base::ApplyTweakChanges_Derived()
{
	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	if( StaticMeshComponent )
	{
		if( GIsGame && StaticMeshComponent->IsAttached() )
		{
			StaticMeshComponent->SetStaticMesh( Tweaks->m_pPickupStaticMesh, FALSE );
		}
		else
		{
			StaticMeshComponent->StaticMesh = Tweaks->m_pPickupStaticMesh;
		}
	}
	if( Tweaks->m_bNoPhysics )
	{
		Physics = PHYS_None;
	}
}

// DISHONORED(written): 2013 rva 0x62ad90 (2012 0x680320)
const UDisTweaks_InteractableInterface* ADisPickup_Base::GetInteractableTweaks_Derived() const
{
	return GetPickupTweaks()->m_pInteractableTweaks;
}

// DISHONORED(written): 2013 rva 0x619be0 (2012 0x66a9b0): the pickup ticks while its proximity sound has not started,
// while it is travelling to the player, while it is pending destruction, while it is based on something, while its
// rigid body is awake, and whenever bend time is dilating the world.
UBOOL ADisPickup_Base::WantsTick_Derived() const
{
	if( !m_bProximitySoundStarted || m_pTravellingTowardPC || m_bPendingDestructionAfterOneFullTickCycle
		|| Base || !CollisionComponent || CollisionComponent->RigidBodyIsAwake() )
	{
		return TRUE;
	}
	// DISHONORED(bringup): retail asks AGameInfo::GetBendTimeDilation (vtable +976), which is not ported; without bend
	// time the dilation is exactly 1 and the test is FALSE.
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x61d6e0 (2012 0x66c7b0): a rigid-body sleep/wake re-evaluates the tick, and a pickup
// whose proximity sound is playing stops it once the body is asleep outside bend time.
void ADisPickup_Base::OnRigidBodyStatusChange()
{
	const UBOOL bWantsTick = WantsTick_Derived();
	if( LightEnvironment )
	{
		LightEnvironment->bDynamic = bWantsTick ? TRUE : FALSE;
	}
	SetTickIsDisabled( !bWantsTick );

	if( m_bProximitySoundStarted && !DisIsBendTimeFrozen() )
	{
		if( !CollisionComponent || !CollisionComponent->RigidBodyIsAwake() )
		{
			m_bProximitySoundStarted = FALSE;
		}
	}
}

// DISHONORED(written): 2013 rva 0x61d820 (2012 0x66c900): a consumed pickup, and a pickup asked about by a non-pawn
// movement trace, is invisible to traces; a pickup based on an NPC only answers touch-overlap and visibility sweeps;
// a held pickup answers neither the melee/vision gameplay traces nor visibility.
UBOOL ADisPickup_Base::ShouldTrace( UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags )
{
	if( m_bPendingDestructionAfterOneFullTickCycle || ( TraceFlags & DIS_TRACE_MOVE_NONPAWN ) )
	{
		return FALSE;
	}
	if( Base && Base->IsA( ADishonoredNPCPawn::StaticClass() )
		&& ( TraceFlags & ( TRACE_DisTouchOverlap | TRACE_Visible ) ) == 0 )
	{
		return FALSE;
	}
	// DISHONORED(retail): 0x8C401000 = 0x80000000 | TRACE_DisGameplay_VisionLOS | TRACE_DisGameplay_Melee | 0x00400000
	// | TRACE_Visible; the two unnamed bits have no reader in Engine/Inc/UnLevel.h yet.
	if( ( TraceFlags & 0x8C401000 ) != 0 && m_pMovableComponent && m_pMovableComponent->m_pHeldBy )
	{
		return FALSE;
	}
	return FDisPhysicsUtil::PhysObjectShouldTraceCommon( this, TraceFlags )
		&& AKActor::ShouldTrace( Primitive, SourceActor, TraceFlags );
}

// DISHONORED(written): 2013 rva 0x62afe0 (2012 0x680500).
// DISHONORED(bringup): retail tests APawn's m_ActorTypeFlags (BYTE @266, value 36 = the player pawn), which nothing in
// this build writes (agentAS.md follow-up 3), so the reflected Cast is used; and the possessed-pawn branch needs
// ADishonoredPawn::DisIsPossessed / GetPossessingPlayerPawn, which are not ported.
ADishonoredPlayerPawn* ADisPickup_Base::GetPlayerPawnFromUser( ADishonoredPawn* Pawn ) const
{
	return Cast<ADishonoredPlayerPawn>( Pawn );
}

// DISHONORED(written): 2013 rva 0x62ad30 (2012 0x6802c0).
// DISHONORED(bringup): ADishonoredPawn::IsPossessing is not ported, so the second half of the test is FALSE and a
// pickup is never possession-blocked yet.
UBOOL ADisPickup_Base::IsUseBlockedByPossession( const ADishonoredPlayerPawn* Player ) const
{
	return GetPickupTweaks()->m_bNotUsableWhilePossessing && FALSE;
}

// DISHONORED(written): 2013 rva 0x631ba0 (2012 0x682c50)
eCrossHairStatus ADisPickup_Base::GetCrosshairStatus( ADishonoredPawn* Pawn ) const
{
	ADishonoredPlayerPawn* PlayerPawn = GetPlayerPawnFromUser( Pawn );
	if( PlayerPawn && !CanBePickedUp( PlayerPawn ) )
	{
		return CHS_OVER_CANNOT_USE;
	}
	return CHS_OVER_PICKUP;
}

// DISHONORED(written): 2013 rva 0x631bf0 (2012 0x682ca0): the interact itself. A pending-destruction pickup refuses;
// a possession-blocked one prints the interactable tweaks' rejection text; a non-player user grabs it as a movable
// object instead; otherwise every UDisSeqEvent_PickupPickedUp on the actor is checked (and each one that does NOT want
// the pickup destroyed keeps it in the world), DoInteract_Impl consumes it, and the fly-to-hand starts.
UBOOL ADisPickup_Base::AttemptInteract_Derived( ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed )
{
	bOutCanBeWitnessed = FALSE;
	if( m_bPendingDestructionAfterOneFullTickCycle )
	{
		return FALSE;
	}

	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	if( IsUseBlockedByPossession( ADishonoredPlayerPawn::s_pInstance ) )
	{
		UDisTweaks_Interactable_PickupBase* InteractableTweaks = Tweaks->m_pInteractableTweaks;
		if( InteractableTweaks )
		{
			FString RejectedMessage = InteractableTweaks->m_NotUsableWhilePossessingInteractText;
			if( RejectedMessage.Len() > 0 )
			{
				FormatText( RejectedMessage );
				DisAddUseMessage( RejectedMessage );
			}
		}
		return FALSE;
	}

	ADishonoredPlayerPawn* PlayerPawn = GetPlayerPawnFromUser( Pawn );
	if( !PlayerPawn )
	{
		if( !Tweaks->m_bNoPhysics )
		{
			// DISHONORED(bringup): ADishonoredPawn::GrabMovableObject (retail vtable +1520) is not ported, so an NPC
			// user cannot pick the object up as a movable yet; retail returns TRUE here.
			bOutCanBeWitnessed = TRUE;
			return TRUE;
		}
		return FALSE;
	}

	if( Tweaks->m_pPickupSoundEvent )
	{
		PostAkEvent( Tweaks->m_pPickupSoundEvent );
	}

	UBOOL bDestroyPickup = TRUE;
	for( INT Idx = 0; Idx < GeneratedEvents.Num(); Idx++ )
	{
		UDisSeqEvent_PickupPickedUp* Event = Cast<UDisSeqEvent_PickupPickedUp>( GeneratedEvents(Idx) );
		if( Event )
		{
			bDestroyPickup = bDestroyPickup && Event->m_bDestroyPickup;
			Event->CheckActivate( this, Pawn, FALSE, NULL, FALSE );
		}
	}

	const UBOOL bTravel = DoInteract_Impl( PlayerPawn ) && !Tweaks->m_bDontDestroyOnPickup;
	if( bDestroyPickup && bTravel )
	{
		StartPickupTravel( Pawn );
		// DISHONORED(bringup): ADishonoredPlayerPawn::IncrementStat( ePlayerStat_ItemsCollected, 1.f, Tweaks, NULL,
		// NULL ) (2013 rva 0x6b9560) is not ported.
	}

	bOutCanBeWitnessed = TRUE;
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x62adc0 (2012 0x680350)
UBOOL ADisPickup_Base::AttemptCannotUseInteract_Derived( ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed )
{
	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	if( Tweaks->m_pCannotPickupSoundEvent )
	{
		PostAkEvent( Tweaks->m_pCannotPickupSoundEvent );
	}
	bOutCanBeWitnessed = FALSE;
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x62ab40 (2012 0x6800d0): the fly-to-hand. The travel is driven from Tick by
// m_fPickupTravelTime counting down from the tweaks' m_fTravelToUserTime, and the pickup stops colliding at once.
void ADisPickup_Base::StartPickupTravel( ADishonoredPawn* Pawn )
{
	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();

	m_pTravellingTowardPC = Pawn ? Cast<ADishonoredPlayerController>( Pawn->Controller ) : NULL;
	m_fPickupTravelTime = Tweaks->m_fTravelToUserTime;
	m_fMaxTravelTime = Tweaks->m_fTravelToUserTime;
	m_PickupStartPos = Location;

	const FBox ComponentBounds = GetComponentsBoundingBox();
	m_OffsetToBoundsCenter = ComponentBounds.GetCenter() - Location;

	if( Pawn && Pawn->m_bOutOfBendTime && DisIsBendTimeOn() )
	{
		DisPullFromBendTime( this, NULL, FALSE, FALSE );
	}

	if( LightEnvironment )
	{
		LightEnvironment->bDynamic = TRUE;
	}
	SetTickIsDisabled( FALSE );
	setPhysics( PHYS_None );
	SetCollisionType( COLLIDE_NoCollision );
}

// DISHONORED(written): 2013 rva 0x61d770 (2012 0x66c840): the pickup is hidden and marked for destruction on the next
// full tick cycle; Tick then calls DisDestroyActorNextTick.
void ADisPickup_Base::ConsumePickup()
{
	if( LightEnvironment )
	{
		LightEnvironment->bDynamic = TRUE;
	}
	SetTickIsDisabled( FALSE );
	SetHidden( TRUE );
	m_bPendingDestructionAfterOneFullTickCycle = TRUE;
	debugf( TEXT("DISHONORED(bringup): pickup %s (%s) consumed"), *GetName(), *GetClass()->GetName() );
}

// DISHONORED(written): 2013 rva 0x62b130 (2012 0x6806b0): destruction, the proximity sound and the tick re-evaluation,
// the fly-to-hand interpolation toward the player camera (quadratic ease, the tweaks' m_TravelCameraOffset raising the
// target), and the deferred reattach of a moving pickup's components.
UBOOL ADisPickup_Base::Tick( FLOAT DeltaTime, ELevelTick TickType )
{
	const UBOOL bResult = AActor::Tick( DeltaTime, TickType );

	if( m_bPendingDestructionAfterOneFullTickCycle )
	{
		DisDestroyActorNextTick( *this );
	}
	else
	{
		if( !m_bProximitySoundStarted && ( !GIsEditor || GIsGame ) && !bDeleteMe )
		{
			UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
			PostAkEvent( Tweaks->m_pProximitySoundStart );
			m_bProximitySoundStarted = TRUE;
			const UBOOL bWantsTick = WantsTick_Derived();
			if( LightEnvironment )
			{
				LightEnvironment->bDynamic = bWantsTick ? TRUE : FALSE;
			}
			SetTickIsDisabled( !bWantsTick );
		}

		if( m_pTravellingTowardPC )
		{
			m_fPickupTravelTime -= DeltaTime;
			if( m_fPickupTravelTime <= 0.f )
			{
				ConsumePickup();
			}
			else if( m_pTravellingTowardPC->PlayerCamera )
			{
				UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
				const FLOAT Alpha = Clamp<FLOAT>( m_fMaxTravelTime > 0.f ? m_fPickupTravelTime / m_fMaxTravelTime : 0.f, 0.f, 1.f );
				const FLOAT Weight = ( 1.f - Alpha ) * ( 1.f - Alpha );
				FVector Target = m_pTravellingTowardPC->PlayerCamera->CameraCache.POV.Location;
				Target.Z += Tweaks->m_TravelCameraOffset;
				Target -= m_OffsetToBoundsCenter;
				const FVector Pos = m_PickupStartPos + ( Target - m_PickupStartPos ) * Weight;
				GWorld->FarMoveActor( this, Pos, FALSE, FALSE, FALSE );
			}
		}
	}

	if( Base || Abs( Velocity.X ) >= 0.0001f || Abs( Velocity.Y ) >= 0.0001f || Abs( Velocity.Z ) >= 0.0001f )
	{
		for( INT Idx = 0; Idx < AllComponents.Num(); Idx++ )
		{
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>( AllComponents(Idx) );
			// DISHONORED(retail): the component flag retail tests is UActorComponent @76 bit 0x10
			// (bIsPrimitiveComponent), and the field it invalidates is UPrimitiveComponent::VisibilityId @304: a pickup
			// that moves drops its precomputed-visibility id and reattaches.
			if( Primitive && Primitive->VisibilityId != INDEX_NONE )
			{
				Primitive->VisibilityId = INDEX_NONE;
				Primitive->BeginDeferredReattach();
			}
		}
	}
	return bResult;
}

// DISHONORED(written): 2013 rva 0x62b440 (2012 0x680980): a m_bNoPhysics pickup ignores every physics change
void ADisPickup_Base::setPhysics( BYTE NewPhysics, AActor* NewFloor, FVector NewFloorV )
{
	if( !GetPickupTweaks()->m_bNoPhysics )
	{
		AActor::setPhysics( NewPhysics, NewFloor, NewFloorV );
	}
}

// DISHONORED(written): 2013 rva 0x62b0a0 (2012 0x6805c0): attach to the socket the tweaks name and become visible
void ADisPickup_Base::Attach( AActor* Other, USkeletalMeshComponent* SkelComp )
{
	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	SetBase( Other, FVector(0,0,1), 1, SkelComp, Tweaks->m_SocketName );
	SetHidden( FALSE );
}

// DISHONORED(written): 2013 rva 0x6226c0 (2012 0x6705f0): clear the pawn's stealable slot, then unbase
void ADisPickup_Base::Detach()
{
	ADishonoredNPCPawn* BasePawn = Cast<ADishonoredNPCPawn>( Base );
	if( BasePawn && m_bIsAttachedAsStealable && BasePawn->m_pStealable == this )
	{
		BasePawn->m_pStealable = NULL;
		AdjustDetachedPosition();
		m_bIsAttachedAsStealable = FALSE;
	}
	SetBase( NULL, FVector(0,0,1), 1, NULL, NAME_None );
}

// DISHONORED(written): 2013 rva 0x61dbb0 (2012 0x6706b0): the native BaseChange event. Based: no encroachment check,
// no physics, and a pawn base takes this pickup as its stealable; unbased: rigid body again and the detached position
// is nudged out of the world.
void ADisPickup_Base::BaseChange()
{
	if( Base )
	{
		bNoEncroachCheck = TRUE;
		setPhysics( PHYS_None, Base );
		ADishonoredNPCPawn* BasePawn = Cast<ADishonoredNPCPawn>( Base );
		if( BasePawn )
		{
			BasePawn->m_pStealable = this;
			m_bIsAttachedAsStealable = TRUE;
		}
	}
	else
	{
		setPhysics( PHYS_RigidBody );
		bNoEncroachCheck = FALSE;
		AdjustDetachedPosition();
		m_bIsAttachedAsStealable = FALSE;
	}
}

// DISHONORED(written): 2013 rva 0x5f93a0 (2012 0x63f350): the generated script wrapper of BaseChange
void ADisPickup_Base::execBaseChange( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	BaseChange();
}

// DISHONORED(written): 2013 rva 0x61d990 (2012 0x66ca70): a pickup that was attached as a stealable is pushed out of
// world geometry when it comes off the pawn.
// DISHONORED(bringup): retail then resyncs the PhysX body to the new transform (URB_BodyInstance::GetUnrealWorldTM,
// U2NTransform, NxActor::setGlobalPose); that tail is not ported, so the rigid body catches up on its own next step.
void ADisPickup_Base::AdjustDetachedPosition()
{
	if( !m_bIsAttachedAsStealable )
	{
		return;
	}
	const FBox ComponentBounds = GetComponentsBoundingBox();
	const FVector Extent( ( ComponentBounds.Max.X - ComponentBounds.Min.X ) * 0.5f + 2.f,
	                      ( ComponentBounds.Max.Y - ComponentBounds.Min.Y ) * 0.5f + 2.f,
	                      ( ComponentBounds.Max.Z - ComponentBounds.Min.Z ) * 0.5f + 2.f );
	FCheckResult Hit( 1.f );
	if( !GWorld->EncroachingWorldGeometry( Hit, Location, Extent, TRUE ) )
	{
		return;
	}
	FVector Spot = Location;
	Spot.Z += Extent.Z;
	if( GWorld->FindSpot( Extent, Spot, FALSE, this ) )
	{
		GWorld->FarMoveActor( this, Spot, FALSE, FALSE, FALSE );
	}
}

// DISHONORED(written): 2013 rva 0x622750 (2012 0x670680, same bytes)
void ADisPickup_Base::ClearComponents()
{
	AActor::ClearComponents();
	if( GIsGame && StaticMeshComponent && Base )
	{
		Detach();
	}
}

// DISHONORED(written): 2013 rva 0x62acd0 (2012 0x680260)
void ADisPickup_Base::OnActorTerminated()
{
	UDisTweaks_PickupBase* Tweaks = GetPickupTweaks();
	PostAkEvent( Tweaks->m_pProximitySoundStop );
	if( m_pParticleSystem )
	{
		m_pParticleSystem->DeactivateSystem();
	}
	Detach();
}

// DISHONORED(written): 2013 rva 0x61d6a0 (2012 0x6705b0)
UBOOL ADisPickup_Base::IgnoreBlockingBy( const AActor* Other ) const
{
	if( AActor::IgnoreBlockingBy( Other ) )
	{
		return TRUE;
	}
	// DISHONORED(bringup): UDisMovableComponent::HandleIgnoreBlockingBy is not ported; the holder itself is the case
	// that matters and is kept.
	return m_pMovableComponent && m_pMovableComponent->m_pHeldBy == Other;
}

// DISHONORED(written): 2012 rva 0x66c820 (no distinct 2013 symbol: identical-COMDAT folded)
UBOOL ADisPickup_Base::CanSplash()
{
	return m_pMovableComponent == NULL || m_pMovableComponent->m_pHeldBy == NULL;
}

// DISHONORED(written): 2013 rva 0x62af90 (2012 0x6804b0): the Heart sees a key item
UBOOL ADisPickup_Base::HasSoul( INT PowerLevel ) const
{
	return m_bIsKeyItemOverride || GetPickupTweaks()->m_bIsKeyItem;
}

// DISHONORED(written): 2013 rva 0x62b060 (2012 0x680580)
BYTE ADisPickup_Base::GetMovableWeightClass() const
{
	return GetPickupTweaks()->m_MovableParams.m_WeightClass;
}

// DISHONORED(written): 2013 rva 0x619a90 (2012 0x66a870, same bytes): damage records the instigator pawn and pulls the
// pickup out of bend time when the damage came from something that is itself out of bend time.
void ADisPickup_Base::TakeDamage_Impl( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const FTraceHitInfo& HitInfo, AActor* const DamageCauser )
{
	Instigator = DisGetPawnInstigator( InstigatedBy );
	const UBOOL bCauserOutOfBendTime = ( DamageCauser && DamageCauser->m_bOutOfBendTime )
		|| ( InstigatedBy && InstigatedBy->m_bOutOfBendTime );
	if( bCauserOutOfBendTime && DisIsBendTimeOn() )
	{
		DisPullFromBendTime( this, Instigator ? (AActor*)Instigator : DamageCauser, TRUE, FALSE );
	}
}

// DISHONORED(written): 2013 rva 0x63f8d0 (2012 0x680650, same bytes): a steal is an interact that also scores the
// ePlayerStat_ItemsStolen stat (IncrementStat is not ported, see the file banner).
void ADisPickup_Base::Steal( ADishonoredPlayerPawn* PlayerPawn )
{
	AttemptInteract( PlayerPawn );
}
