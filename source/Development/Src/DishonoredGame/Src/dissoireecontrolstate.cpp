/*=============================================================================
	dissoireecontrolstate.cpp: agent FZ (PHASE18 FZ) - FDisSoireeControlState, the forced perspective a cutscene's
	player group puts the player in. Shared by UStatePlayerMasterSoiree (a scene started from the walk state) and
	UStatePlayerMasterInDialog (a scene started while already in a dialog).

	Retail: EnterSoireeMode 2012 0x69a4b0 (525 B) / 2013 0x666ff0 (591 B, matched as callee 0.674 and read in both
	databases - the 2013 body is ported: it adds the SOIREE_LeaveCameraToDialogTree branch that keeps the camera and the
	input with the dialog tree, the equipped items' context notification, and passes bWaitToBlendOneFrame = FALSE to the
	influence weights), TickSoireeMode 0x692b80 (22 B, unnamed in 2013), ExitSoireeMode 0x695a70 (414 B) / 2013 0x6628f0
	(388 B: the re-entry flag moved from bit 3 to bit 4 because bit 3 became SOIREE_HasAnimControlTrack, the bump-smoother and
	physical-react weights are restored through one helper, and the anim state is only invalidated when the scene had an
	anim-control track).

	What the state does, in retail's order, and what of it exists here:
	  - the camera's core influence group blends to UDishonoredCamera_AnimDriven over the blend-in time: the view becomes the
	    player's camera bone (the full-body mesh the matinee animates), with the player's own look clamped to whatever
	    UDisInterpTrackAllowPlayerLook allows (no key: no look at all) - ported, dishonoredplayercamera.cpp and
	    dishonoredcamera_animdriven.cpp;
	  - both hands' arm-follow influences are weighted to 1 (the arms stop following the camera), the bump smoother and the
	    physical react to 0 - ported as influence weights; the arm placement itself is ADishonoredPlayerPawn::OnCameraUpdate_
	    Derived (2012 0x71e3b0), which is not ported;
	  - the soiree input mask level loses bits 1 and 15 (0x8002), the save lock is pushed, sneaking stops, bWantsToCrouch
	    is cleared - ported (StopSneaking partially);
	  - with SOIREE_DisablePlayerCollision the pawn's four collision bits are saved and collision switched off; with that or
	    SOIREE_HasMoveTrack the pawn goes to PHYS_Interpolating so the matinee can carry it - ported;
	  - every active power is cancelled and a carried corpse is dropped (the upper FSM's state exits) - the power components
	    and the upper FSM have no bodies here; logged once.
=============================================================================*/
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "stateplayermasterparams.h"
#include "fzsoireediag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_FDisSoireeControlState()
{
}

void FDisSoireeControlState::EnterSoireeMode( USeqAct_Interp* Matinee, ADishonoredPlayerPawn* PlayerPawn, FLOAT BlendInTime, INT Flags )
{
	if( !PlayerPawn )
	{
		return;
	}
	if( m_bControlledBySoiree )
	{
		m_Flags |= SOIREE_Reentered;
		ExitSoireeMode( PlayerPawn, 0.f, TRUE );
	}
	m_Flags = Flags;

	if( !( Flags & SOIREE_LeaveCameraToDialogTree ) )
	{
		PlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_AnimDriven::StaticClass(), BlendInTime, FALSE );
		ADishonoredPlayerCamera* Camera = PlayerPawn->GetDishonoredCamera();
		if( Camera )
		{
			if( Camera->GetCamera_BumpSmoother() )
			{
				Camera->GetCamera_BumpSmoother()->SetWeight( 0.f, BlendInTime, FALSE );
			}
			if( Camera->GetCamera_PhysicalReact() )
			{
				Camera->GetCamera_PhysicalReact()->SetWeight( 0.f, BlendInTime, FALSE );
			}
		}
		PlayerPawn->EnableControllerInput( 0x8002, FALSE, DisInputMaskLevel_Soiree );
	}
	PlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, TRUE, BlendInTime, FALSE );
	PlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, TRUE, BlendInTime, FALSE );
	DisPushDisableSave( DDST_Soiree );
	PlayerPawn->StopSneaking( FALSE );
	PlayerPawn->bWantsToCrouch = FALSE;

	// 2013: every equipped item's contexts hear that a soiree started (UDisItemContext vtable slot 74 with TRUE, 2013
	// 0x816d90 over GetContextsForSlot). The item contexts are not ported.
	FZDIAG_ONCE( TEXT("EnterSoireeMode: the equipped items' context notification (2013 0x816d90) is not ported") );

	if( Flags & SOIREE_DisablePlayerCollision )
	{
		m_bOldCollide_Actors = PlayerPawn->bCollideActors;
		m_bOldCollide_BlockActors = PlayerPawn->bBlockActors;
		m_bOldCollide_IgnoreEncroach = PlayerPawn->bIgnoreEncroachers;
		m_bOldCollide_World = PlayerPawn->bCollideWorld;
		PlayerPawn->SetCollision( FALSE, FALSE, TRUE );
		PlayerPawn->bCollideWorld = FALSE;
	}
	if( Flags & ( SOIREE_DisablePlayerCollision | SOIREE_HasMoveTrack ) )
	{
		PlayerPawn->setPhysics( PHYS_Interpolating );
	}

	PlayerPawn->CancelAllActivePowers();
	if( PlayerPawn->IsCarryingCorpse() )
	{
		// retail asks the upper FSM's logical state to exit (RequestStateExit_Derived); the upper FSM is not initialised here
		FZDIAG_ONCE( TEXT("EnterSoireeMode: the carried-corpse exit through the upper FSM is not ported") );
	}

	m_bControlledBySoiree = TRUE;
	m_pMatinee = Matinee;
	FZDIAG( TEXT("soiree ENTER %s on %s: flags 0x%x blendIn %.2f (collision %s, physics %d, body mode %d)"),
		Matinee ? *Matinee->GetName() : TEXT("none"), *PlayerPawn->GetName(), Flags, BlendInTime,
		( Flags & SOIREE_DisablePlayerCollision ) ? TEXT("off") : TEXT("kept"), (INT)PlayerPawn->Physics, (INT)PlayerPawn->m_BodyMode );
}

// DISHONORED(port): 2012 0x692b80: while controlled, sneaking is stopped every tick
void FDisSoireeControlState::TickSoireeMode( ADishonoredPlayerPawn* PlayerPawn )
{
	if( m_bControlledBySoiree && PlayerPawn )
	{
		PlayerPawn->StopSneaking( FALSE );
	}
}

void FDisSoireeControlState::ExitSoireeMode( ADishonoredPlayerPawn* PlayerPawn, FLOAT BlendOutTime, UBOOL bInterrupted )
{
	if( !m_bControlledBySoiree || !PlayerPawn )
	{
		return;
	}
	if( !( m_Flags & SOIREE_Reentered ) )
	{
		const UBOOL bWaitOneFrame = BlendOutTime > 0.f;
		PlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_PlayerControl::StaticClass(), BlendOutTime, bWaitOneFrame );
		// agent IA: 2013 0x6628f0 re-enables the arm follow of each hand whose actual stance is not eDisPlayerStance_NotReady
		// (GetActualPlayerStance 0x6a20f0, DisableArmFollow 0x6a1cf0 with the same wait flag); diffed for FOV writes: there are none
		if( PlayerPawn->GetActualPlayerStance( EDisEquipUsage_Primary ) != eDisPlayerStance_NotReady )
		{
			PlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, FALSE, BlendOutTime, bWaitOneFrame );
		}
		if( PlayerPawn->GetActualPlayerStance( EDisEquipUsage_Secondary ) != eDisPlayerStance_NotReady )
		{
			PlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, FALSE, BlendOutTime, bWaitOneFrame );
		}
		ADishonoredPlayerCamera* Camera = PlayerPawn->GetDishonoredCamera();
		if( Camera )
		{
			if( Camera->GetCamera_BumpSmoother() )
			{
				Camera->GetCamera_BumpSmoother()->SetWeight( Camera->GetCamera_BumpSmoother()->m_fDefaultWeight, 0.f, FALSE );
			}
			if( Camera->GetCamera_PhysicalReact() )
			{
				Camera->GetCamera_PhysicalReact()->SetWeight( Camera->GetCamera_PhysicalReact()->m_fDefaultWeight, BlendOutTime, BlendOutTime > 0.f );
			}
		}
	}
	PlayerPawn->EnableControllerInput( 0x8002, TRUE, DisInputMaskLevel_Soiree );
	DisPopDisableSave( DDST_Soiree, 0.f );
	if( m_Flags & SOIREE_HasAnimControlTrack )
	{
		// retail: UDisAnimStateComponent::InvalidateCurAnimState( m_pAnimStateComp, FALSE ) (2012 0x6bd8a0), unported
		FZDIAG_ONCE( TEXT("ExitSoireeMode: UDisAnimStateComponent::InvalidateCurAnimState is not ported") );
	}
	if( m_Flags & SOIREE_DisablePlayerCollision )
	{
		PlayerPawn->SetCollision( m_bOldCollide_Actors, m_bOldCollide_BlockActors, m_bOldCollide_IgnoreEncroach );
		PlayerPawn->bCollideWorld = m_bOldCollide_World;
	}
	if( m_Flags & ( SOIREE_DisablePlayerCollision | SOIREE_HasMoveTrack ) )
	{
		PlayerPawn->setPhysics( PHYS_Walking );
	}
	m_bControlledBySoiree = FALSE;
	DisPCamNoteSoireeExit();
	FZDIAG( TEXT("soiree EXIT %s on %s: blendOut %.2f interrupted %d (physics %d, body mode %d)"),
		m_pMatinee ? *m_pMatinee->GetName() : TEXT("none"), *PlayerPawn->GetName(), BlendOutTime, bInterrupted, (INT)PlayerPawn->Physics, (INT)PlayerPawn->m_BodyMode );
	if( bInterrupted && m_pMatinee )
	{
		m_pMatinee->DeActivated();
	}
	m_pMatinee = NULL;
}
