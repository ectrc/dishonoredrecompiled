// DishonoredGame/src/dishonoredplayerpawn_mantle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x6fb830  public: unsigned int __thiscall ADishonoredPlayerPawn::IsMantleAllowed(void)const
//   0x706f00  private: void __thiscall ADishonoredPlayerPawn::DoMantle_StepUp(float, class FVector const &, int)
//   0x707170  private: unsigned int __thiscall ADishonoredPlayerPawn::IsMantleRequested(void)const
//   0x7071a0  private: unsigned int __thiscall ADishonoredPlayerPawn::IsPressingForward(void)const
//   0x70d380  private: void __thiscall ADishonoredPlayerPawn::DoMantle_Anim(float, class FVector const &, int, unsigned int, unsigned int, unsigned int)
//   0x70ff10  public: unsigned int __thiscall ADishonoredPlayerPawn::PerformMantle(unsigned int, unsigned int)
//   0x7129d0  private: void __thiscall ADishonoredPlayerPawn::Tick_Mantle(float, enum ELevelTick)
//   0x712b10  private: unsigned int __thiscall ADishonoredPlayerPawn::DoJump_Mantle(void)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): the jump's half of the mantle (DoJump_Mantle, IsMantleRequested, IsPressingForward).
	agent IG (PHASE23 IG): the mantle itself. Every tick (Tick_Begin's first phase) Tick_Mantle allows the mantle on the ground and
	in water, keeps the edge finder running except in a fall the jump did not allow, and mantles from a fall while the jump button is
	held (in water on the jump button, or with the tweak's automatic water mantle on forward). PerformMantle takes the finder's last
	spot: an edge no higher than m_LowMantleMaxEdgeHeight (m_bLowMantleUseStepUp, not crouch-forced) is a step-up - three sweeps to
	the start, up the edge height, forward m_fMantleForwardMoveAmount, with the camera and arm blends and a short no-run-off-ledges
	window; a higher one is the animated mantle (UStatePlayerMasterMantle through the master FSM's full-body action).
	2012 / 2013 rvas: PerformMantle 0x70ff10 / 0x6d78d0 (283 / 324 B), Tick_Mantle 0x7129d0 / 0x6dbd00 (319 / 365 B, 2013 inlines
	SetMantleCheckDisabled), DoMantle_StepUp 0x706f00 / 0x6d35e0, DoMantle_Anim 0x70d380 / 0x6cd820, IsMantleAllowed 0x6fb830 (10 B),
	DoJump_Mantle 0x712b10 / 0x6dbe70 (83 B both), IsMantleRequested 0x707170 / 0x6c8000, IsPressingForward 0x7071a0 / 0x6c8030.
	agent IM (PHASE23 IM): the master action base plays the mantle's animation now (UStatePlayerMasterAction::DoAction,
	ADishonoredPlayerPawn::SetAnimState), so PerformMantle requests the animated mantle as retail does; agent IG's refusal is gone.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "igdiag.h"
#include "imdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_PlayerPawnMantle()
{
}

// 2013 0x6dbe70: with an edge finder the mantle is allowed and, while the jump button is held, attempted; without one it is not
UBOOL ADishonoredPlayerPawn::DoJump_Mantle()
{
	if( m_pMantleEdgeFinder == NULL )
	{
		m_bMantleAllowed = FALSE;
		return FALSE;
	}
	m_bMantleAllowed = TRUE;
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->m_bJumpButton && PerformMantle( TRUE, FALSE );
}

// 2013 0x6c8000 (37 B)
UBOOL ADishonoredPlayerPawn::IsMantleRequested() const
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->m_bJumpButton;
}

// 2013 0x6c8030 (55 B)
UBOOL ADishonoredPlayerPawn::IsPressingForward() const
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->PlayerInput != NULL && PC->PlayerInput->RawJoyUp > 0.f;
}

// 2012 0x6fb830 (10 B)
UBOOL ADishonoredPlayerPawn::IsMantleAllowed() const
{
	return m_bMantleAllowed;
}

// 2013 0x6d78d0 (324 B)
UBOOL ADishonoredPlayerPawn::PerformMantle( UBOOL bCancelable, UBOOL bBlinkToMantle )
{
	UDishonoredMantleEdgeFinderComponent* Finder = bBlinkToMantle ? m_pMantleEdgeFinder_Blink : m_pMantleEdgeFinder;
	if( Finder == NULL || !Finder->HasValidMantleSpot() )
	{
		DisIGNotePerformMantle( this, Finder, TEXT("no valid spot") );
		return FALSE;
	}
	const FDisMantleSpot Spot = Finder->GetMantleSpot();
	UDisTweaks_PlayerPawn_Mantle* Tweaks = Finder->GetMantleTweaks();
	if( Tweaks->m_bLowMantleUseStepUp && Tweaks->m_LowMantleMaxEdgeHeight >= Spot.m_fEdgeHeight && !Spot.m_bForcesCrouch )
	{
		DisIGNotePerformMantle( this, Finder, TEXT("step-up") );
		DoMantle_StepUp( Spot.m_fEdgeHeight, Spot.m_StartLocation, Spot.m_StartYaw );
		return TRUE;
	}
	if( IsHoldingMovableObject() || IsCarryingCorpse() )
	{
		DisIGNotePerformMantle( this, Finder, TEXT("refused: holding an object or carrying a corpse") );
		return FALSE;
	}
	DisIGNotePerformMantle( this, Finder, TEXT("animated mantle") );
	DisIMNoteMantleRequest( this, Spot.m_fEdgeHeight );
	DoMantle_Anim( Spot.m_fEdgeHeight, Spot.m_StartLocation, Spot.m_StartYaw, Spot.m_bRequiresImpactAnim, Spot.m_bForcesCrouch, bCancelable );
	return TRUE;
}

/**
 * 2013 0x6d35e0 (625 B). The camera's step-up influence remembers where the view was and blends out from full weight over
 * m_fLowMantleStepUpBlendTime; the pawn is swept to the spot's start, up the edge height and forward m_fMantleForwardMoveAmount; a
 * power jump in progress loses its power; the arms' step-up blend node plays; running off ledges is refused for
 * m_fLowMantleStepUpDisallowFallTime.
 */
void ADishonoredPlayerPawn::DoMantle_StepUp( FLOAT MantleHeight, const FVector& MantleStartLocation, INT MantleStartYaw )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pMantleEdgeFinder->GetMantleTweaks();
	UDisCamera_StepUpMantleOffset* StepUpOffset = NULL;
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( Camera )
	{
		StepUpOffset = Camera->m_pStepUpMantleOffset_Influence;
		if( StepUpOffset )
		{
			StepUpOffset->m_StepUpStartPos = ADishonoredPlayerController::s_pInstance->PlayerCamera->ViewTarget.POV.Location;
		}
	}
	const FVector StartLocation = Location;
	const FVector OffsetToMantleStart = MantleStartLocation - Location;
	const FVector MantleUpwardOffset( MantleHeight * 0.f, MantleHeight * 0.f, MantleHeight );
	const FVector MantleForwardOffset = FRotator( 0, MantleStartYaw, 0 ).Vector() * Tweaks->m_fMantleForwardMoveAmount;
	FCheckResult Hit;
	GWorld->MoveActor( this, OffsetToMantleStart, Rotation, 0, Hit );
	GWorld->MoveActor( this, MantleUpwardOffset, Rotation, 0, Hit );
	GWorld->MoveActor( this, MantleForwardOffset, Rotation, 0, Hit );
	if( StepUpOffset )
	{
		StepUpOffset->SetWeight( 1.f, 0.f );
		StepUpOffset->SetWeight( 0.f, Tweaks->m_fLowMantleStepUpBlendTime );
	}
	UStatePlayerMasterJump* Jump = Cast<UStatePlayerMasterJump>( m_pPlayerMasterFSM->GetCurrentlyActiveState() );
	if( Jump )
	{
		Jump->DisablePowerJump();
	}
	if( Mesh )
	{
		UDisAnimNodeBlendByStepUpMantle* StepUpNode = Cast<UDisAnimNodeBlendByStepUpMantle>( Mesh->FindAnimNode( Tweaks->m_LowMantleStepUpAnimNodeName ) );
		if( StepUpNode )
		{
			StepUpNode->OnStepUpMantle();
		}
	}
	m_fMantleDisallowFallTimer = Tweaks->m_fLowMantleStepUpDisallowFallTime;
	DisIGNoteStepUp( this, StartLocation, MantleStartLocation, MantleHeight, StepUpOffset != NULL );
}

// 2013 0x6cd820 (140 B): the full-body mantle action with the spot, through the master FSM; the mantle is used up
void ADishonoredPlayerPawn::DoMantle_Anim( FLOAT MantleHeight, const FVector& MantleStartLocation, INT MantleStartYaw, UBOOL bImpactAnimRequired, UBOOL bCrouchRequired, UBOOL bCancelable )
{
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( GetTweaks_Derived() );
	if( Tweaks == NULL )
	{
		Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
	}
	UDisTweaks_PlayerPawn_NavigationActions* Actions = ((UDisTweaks_PlayerPawn_Actions*)Tweaks->m_pActionTweaks)->m_pNavigationActionTweaks;
	FStatePlayerMasterMantle_Param MantleParam( Actions->m_Action_Mantle, MantleHeight, MantleStartLocation, MantleStartYaw, bImpactAnimRequired, bCrouchRequired, bCancelable );
	RequestAction( eDisPlayerActionUsage_Fullbody, MantleParam, NULL, FALSE );
	m_bMantleAllowed = FALSE;
}

/**
 * 2013 0x6dbd00 (365 B). On the ground or in water the mantle becomes allowed; the finder is disabled only in a fall the jump did
 * not allow; falling with the mantle allowed and the jump held mantles, in water the jump (or, with m_bAutomaticallyMantleInWater,
 * forward or the jump) mantles. Then the step-up's no-run-off-ledges window counts down (m_bCanRunOffLedges, APawn +640 bit 22).
 */
void ADishonoredPlayerPawn::Tick_Mantle( FLOAT DeltaTime, ELevelTick TickType )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pMantleEdgeFinder->GetMantleTweaks();
	const UBOOL bAutomaticallyMantleInWater = Tweaks->m_bAutomaticallyMantleInWater;
	const UBOOL bFalling = Physics == PHYS_Falling;
	const UBOOL bSwimming = Physics == PHYS_Swimming;
	if( Physics == PHYS_Walking || Physics == PHYS_Swimming )
	{
		m_bMantleAllowed = TRUE;
	}
	m_pMantleEdgeFinder->SetMantleCheckDisabled( bFalling && !m_bMantleAllowed );
	UBOOL bMantle = FALSE;
	if( bFalling )
	{
		bMantle = m_bMantleAllowed && IsMantleRequested();
	}
	else if( bSwimming )
	{
		bMantle = bAutomaticallyMantleInWater ? ( IsPressingForward() || IsMantleRequested() ) : IsMantleRequested();
	}
	if( bMantle )
	{
		PerformMantle( TRUE, FALSE );
	}
	m_bCanRunOffLedges = TRUE;
	m_fMantleDisallowFallTimer -= DeltaTime;
	if( m_fMantleDisallowFallTimer > 0.f )
	{
		m_bCanRunOffLedges = FALSE;
	}
	DisIGReportMantle( this, DeltaTime );
}
