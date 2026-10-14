// DishonoredGame/src/dishonoredplayercontroller_use.cpp
// DISHONORED(port): agent GD (PHASE19 GD). The crosshair and the player's use path on ADishonoredPlayerController, in retail's
// call order: the camera's UpdateViewTarget calls OnCameraUpdate per pass (2013 rva 0x6ba860 / 2012 0x6fa840), pass 1 calls
// UpdateCrosshair (0x6b7b80 / 0x6f7910), which asks GetPlayerCrosshairTarget (0x6a5ff0 / 0x6e1e00) for a combat target and
// FindInteractableActor (0x6b70f0 / 0x6f73f0) for the usable in front of the camera, asks that actor's
// IDisInteractableInterface::GetCrosshairStatus, and hands the result to SetCrosshairActor (0x6a6280 / 0x6e2090) - which is what
// UDisUseState_WaitForInput::TickState reads when the Use button is down. Both databases read by hand; the 2012 names are
// the PDB's, the 2013 bodies are unnamed (sub_AB70F0 is FindInteractableActor, sub_ABA860 OnCameraUpdate).
//
// DISHONORED(bringup), each named: ADishonoredPawn::OnCameraUpdate (2012 0x7aa7a0, the arm placement; agent FZ left it), the
// pass-0 TickCameraTracking and the pass-1 UpdateBlindedEffect, the UDisItemContext_ProjectileAttack aim-assist direction
// (item contexts are unported), FindCombatTarget_Dir (2012 0x6d6b20 on the controller, 0x6fea40 on the pawn - the melee
// target probe; GetPlayerCrosshairTarget answers "nothing" until it is ported), ADishonoredPawn::CanUseObjectTypes (the
// pawn virtual whose trace mask gates the probe), DisLineProbe_Interact / DisLineProbeMulti_Interact (2012 0x83d530 /
// 0x83d590, dishonoredutilities_math.cpp: the masked probe family is a skeleton, so FindInteractableActor runs the engine's
// swept line check with the narrow extent and skips the broad multi-probe fallback), the HUD's crosshair update
// (SetCrosshairActor's DisGetGFxHUD() call, 2013 sub_BAF540) and the interface slot +32 that names the actor to highlight.

#include "DishonoredGame.h"
#include "gddiag.h"

IDisInteractableInterface* DisGDInteractableOf( AActor* _pActor )
{
	if( !_pActor )
	{
		return NULL;
	}
	// Retail: UObject::GetInterfaceAddress( UDisInteractableInterface::StaticClass() ), which adds the cooked class's
	// Interfaces(i).PointerOffset - retail's offset of the interface subobject. The native classes this tree implements the
	// interface on are reached by the C++ cast, which is right for THIS tree's layout; InterfaceCast is the fallback for any
	// other implementer, and the two answers are compared once under -gddiag.
	IDisInteractableInterface* Native = NULL;
	if( ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( _pActor ) )
	{
		Native = NPC;
	}
	else if( ADisPickup_Base* Pickup = Cast<ADisPickup_Base>( _pActor ) )
	{
		Native = Pickup;
	}
	IDisInteractableInterface* Interface = InterfaceCast<IDisInteractableInterface>( _pActor );
	if( Native && DisGDDiagEnabled() )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( TEXT("DISHONORED(bringup): gddiag %s (%s): InterfaceCast<IDisInteractableInterface> %p, the C++ cast %p (%s)"), *_pActor->GetName(), *_pActor->GetClass()->GetName(),
				Interface, Native, Interface == Native ? TEXT("same") : TEXT("DIFFERENT - the cooked PointerOffset is not this tree's layout") );
		}
	}
	return Native ? Native : Interface;
}

static const UDisTweaks_PlayerInput* DisGDPlayerInputTweaks( ADishonoredPlayerController* _pPC )
{
	UDishonoredPlayerInput* Input = _pPC ? Cast<UDishonoredPlayerInput>( _pPC->PlayerInput ) : NULL;
	const UDisTweaks_PlayerInput* Tweaks = Input ? Input->m_pTweaks : NULL;
	if( !Tweaks )
	{
		// retail's own fallback: the class default object
		Tweaks = (UDisTweaks_PlayerInput*)UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject();
	}
	return Tweaks;
}

// OnCameraUpdate itself (2013 rva 0x6ba860 / 2012 0x6fa840) is agent GF's, in dishonoredplayercontroller.cpp; its pass 1 calls UpdateCrosshair.

// DISHONORED(port): 2013 rva 0x6b7b80 (2012 0x6f7910, 418 B), transcribed. A live combat target under the crosshair makes the
// actor the crosshair's and the status CHS_OVER_ENEMY; the interactable in front of the camera is the crosshair actor
// when there is no combat target, and when there is one the player can interact only if both are the same actor; an
// interactable the player can interact with answers its own status.
void ADishonoredPlayerController::UpdateCrosshair( const FDishonoredViewTarget& _VT )
{
	ADishonoredPawn* pPawn = Cast<ADishonoredPawn>( Pawn );
	if( !pPawn )
	{
		return;
	}
	AActor* HitActor = NULL;
	eCrossHairStatus NewCrosshairStatus = CHS_OVER_NEUTRAL;
	UBOOL bCanInteractWithNewCrosshairActor = TRUE;

	FVector CamDir = _VT.m_VT.POV.Rotation.Vector();
	FImpactInfo CombatResult;
	appMemzero( &CombatResult, sizeof( CombatResult ) );
	GetPlayerCrosshairTarget( CombatResult, 6, &CamDir );
	ADishonoredPawn* CombatTarget = Cast<ADishonoredPawn>( CombatResult.HitActor );
	if( CombatTarget && !CombatTarget->m_bIsOfficiallyDead )
	{
		HitActor = CombatResult.HitActor;
		NewCrosshairStatus = CHS_OVER_ENEMY;
	}

	AActor* InteractableActor = FindInteractableActor( _VT );
	if( HitActor )
	{
		if( InteractableActor != HitActor )
		{
			bCanInteractWithNewCrosshairActor = FALSE;
		}
	}
	else
	{
		HitActor = InteractableActor;
		bCanInteractWithNewCrosshairActor = InteractableActor != NULL;
	}
	if( HitActor )
	{
		IDisInteractableInterface* Interactable = DisGDInteractableOf( HitActor );
		if( Interactable && bCanInteractWithNewCrosshairActor )
		{
			const eCrossHairStatus Status = Interactable->GetCrosshairStatus( pPawn );
			if( Status != CHS_OVER_NEUTRAL )
			{
				NewCrosshairStatus = Status;
			}
		}
	}
	SetCrosshairActor( HitActor, NewCrosshairStatus, bCanInteractWithNewCrosshairActor );
}

// DISHONORED(port): 2013 rva 0x6a6280 (2012 0x6e2090, 450 B). Only a change is work: the old actor (and the old highlight actor)
// lose the crosshair focus when the player could interact with them, the three fields are written, and the new actor gains it.
void ADishonoredPlayerController::SetCrosshairActor( AActor* _pActor, eCrossHairStatus _Status, UBOOL _bCanInteract )
{
	const UBOOL bHadCanInteract = m_bCanInteractWithCrosshairActor ? TRUE : FALSE;
	if( m_pCrosshairActor == _pActor && m_CrosshairStatus == (BYTE)_Status && bHadCanInteract == ( _bCanInteract ? TRUE : FALSE ) )
	{
		return;
	}
	if( m_pCrosshairActor && bHadCanInteract )
	{
		if( IDisInteractableInterface* Old = DisGDInteractableOf( m_pCrosshairActor ) )
		{
			Old->LoseCrosshairFocus();
		}
	}
	if( m_pCrosshairHighlightActor && m_pCrosshairHighlightActor != m_pCrosshairActor && bHadCanInteract )
	{
		if( IDisInteractableInterface* OldHighlight = DisGDInteractableOf( m_pCrosshairHighlightActor ) )
		{
			OldHighlight->LoseCrosshairFocus();
		}
	}
	if( DisGDDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): gddiag crosshair %s (%s) status %d canInteract %d (was %s status %d canInteract %d)"),
			_pActor ? *_pActor->GetName() : TEXT("none"), _pActor ? *_pActor->GetClass()->GetName() : TEXT("-"), (INT)_Status, (INT)_bCanInteract,
			m_pCrosshairActor ? *m_pCrosshairActor->GetName() : TEXT("none"), (INT)m_CrosshairStatus, (INT)bHadCanInteract );
	}
	m_CrosshairStatus = (BYTE)_Status;
	m_bCanInteractWithCrosshairActor = _bCanInteract ? 1 : 0;
	m_pCrosshairActor = _pActor;
	m_pCrosshairHighlightActor = NULL;
	if( _bCanInteract && _pActor )
	{
		if( IDisInteractableInterface* New = DisGDInteractableOf( _pActor ) )
		{
			New->GainCrosshairFocus();
		}
	}
}

// DISHONORED(port): 2013 rva 0x6a5ff0 (2012 0x6e1e00, 220 B): the melee-target probe from the camera along the override direction
// (or the camera's), trace mask 0x2207F, 128 uu, every DisTrace type but the excluded ones.
// DISHONORED(bringup): FindCombatTarget_Dir (2012 0x6d6b20 / 0x6fea40) is unported, so the answer is an empty impact.
void ADishonoredPlayerController::GetPlayerCrosshairTarget( FImpactInfo& _rImpactInfoOut, INT _ExcludeDisTraceMask, const FVector* _pOverrideDir ) const
{
	appMemzero( &_rImpactInfoOut, sizeof( _rImpactInfoOut ) );
}

// DISHONORED(port): 2013 rva 0x6b70f0 (2012 0x6f73f0, 1,302 B): from the view target's eye, m_fUsableObjectDist along its rotation,
// a narrow probe (extent m_fUsableObjectExtentNarrow) for an interactable the player can interact with within its tweaks'
// m_fMaxDistance; failing that a broad multi-probe (m_fUsableObjectExtentBroad) whose candidates are re-traced one by one.
// DISHONORED(bringup): the probe family is unported (file banner); the narrow probe is UWorld::SingleLineCheck with the same
// extent against everything that collides, the broad fallback is left out.
AActor* ADishonoredPlayerController::FindInteractableActor( const FDishonoredViewTarget& _VT )
{
	ADishonoredPawn* pPawn = Cast<ADishonoredPawn>( Pawn );
	if( !pPawn || !GWorld )
	{
		return NULL;
	}
	const UDisTweaks_PlayerInput* Tweaks = DisGDPlayerInputTweaks( this );
	const FLOAT fUsableObjectDist = Tweaks ? Tweaks->m_fUsableObjectDist : 0.f;
	const FLOAT fExtent = Tweaks ? Tweaks->m_fUsableObjectExtentNarrow : 0.f;
	if( fUsableObjectDist <= 0.f )
	{
		return NULL;
	}
	const FVector Start = _VT.m_VT.POV.Location;
	const FVector End = Start + _VT.m_VT.POV.Rotation.Vector() * fUsableObjectDist;
	FCheckResult Hit( 1.f );
	// TRACE_Blocking: the interact functor's probe does not stop at a trigger volume the player stands in (GD2 measured the
	// crosshair on DisTrigger_16's brush instead of the captain 120 uu ahead without it).
	GWorld->SingleLineCheck( Hit, pPawn, End, Start, TRACE_World | TRACE_Pawns | TRACE_Others | TRACE_Movers | TRACE_Blocking | TRACE_ComplexCollision, FVector( fExtent, fExtent, fExtent ) );
	if( !Hit.Actor )
	{
		return NULL;
	}
	IDisInteractableInterface* Interactable = DisGDInteractableOf( Hit.Actor );
	if( !Interactable )
	{
		return NULL;
	}
	FCanInteractParams Params( EC_EventParm );
	Params.m_pInteractor = pPawn;
	Params.m_pWithComponent = Hit.Component;
	Params.m_DisTraceFlags = 16;
	return CanInteractWithInteractable( *Interactable, Params, Start, Hit.Location ) ? Hit.Actor : NULL;
}

// DISHONORED(port): 2013 rva 0x6a2dc0 (2012 0x6d6cc0, 128 B), transcribed: the interactable's own CanInteract, then the hit within
// its tweaks' m_fMaxDistance of the eye.
UBOOL ADishonoredPlayerController::CanInteractWithInteractable( const IDisInteractableInterface& _rInteractable, const FCanInteractParams& _rInteractParams, const FVector& _rStartLoc, const FVector& _rHitLoc ) const
{
	if( !_rInteractable.CanInteract( _rInteractParams ) )
	{
		return FALSE;
	}
	const FLOAT fMaxDistance = _rInteractable.GetInteractableTweaks().m_fMaxDistance;
	return ( fMaxDistance * fMaxDistance ) > ( _rHitLoc - _rStartLoc ).SizeSquared();
}
