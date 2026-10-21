/*=============================================================================
	stateplayermasterindialog.cpp: UStatePlayerMasterInDialog, the player master FSM's dialog state.
	agent FZ (PHASE18 FZ): the soiree hand-over the player MAT_* contract reaches when the master FSM is already in a dialog:
	StartSoireeControl 2012 0x6afa80 (2013 0x6696e0), StopSoireeControl 0x6afab0 (2013 0x663e20), SetSoireeBlendOutTime 0x6afae0
	(2013 0x6609c0, inline in the CppText).
	agent JE (PHASE24 JE): the state itself - ADishonoredPlayerPawn::StartDialog requests it for a conversation that blocks the
	player, EndDialog asks it to exit. On top of UStatePlayerMasterChoice_Base's enter (input, save lock, the dialog HUD set with
	the letterbox) it stops sneaking and crouching, puts the hands away (empty hands, reequip remembered) and drops a held movable;
	ticking keeps the timed look-at constraint and the soiree control; leaving ends a soiree control it still holds, reequips
	unless the next state is the store, stops the running conversation and the view track. The view tracks are the look-at
	nodes' (UDisConv_PlayerLookAtActor / _PlayerLookAtSpeaker). 2013 bodies where they differ.
	DISHONORED(bringup), named and not called (OnEnterState): the 2013 carried-corpse drop when the conversation does not allow
	carrying a body (the carried corpse / UDisCarriable path), and the deactivation of every usable active power but Possess
	(UDishonoredActivePowerComponent vtable +352 / +368, not declared in this tree).
	agent JF (PHASE24 JF): the choice half - the choice node hands itself over (SetActivePlayerChoice: every master input off, the
	choice up, the bend time paused), the list is its static choices then the optional ones whose story flag holds, and a 2013 test
	that the dialog's view track has finished blending in before the node is handed over.
=============================================================================*/
#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "jediag.h"
#include "jfdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_UStatePlayerMasterInDialog()
{
}

void UStatePlayerMasterInDialog::StartSoireeControl( USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags )
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.EnterSoireeMode( Matinee, m_pPlayerPawn, BlendInTime, Flags );
	}
}

void UStatePlayerMasterInDialog::StopSoireeControl( UBOOL bInterrupted )
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.ExitSoireeMode( m_pPlayerPawn, m_fSoireeBlendOutTime, bInterrupted );
	}
}

// 2013 0x6715a0 (71 B)
FStatePlayerMasterInDialog_Param::FStatePlayerMasterInDialog_Param( FDisDialogRunningInstance* DialogRunningInst, UBOOL bAllowPlayerToCarryBody )
	: FStatePlayerMasterBase_Param( UStatePlayerMasterInDialog::StaticClass() )
	, m_pDialogRunningInst( DialogRunningInst )
	, m_bAllowPlayerToCarryBody( bAllowPlayerToCarryBody )
{
}

// 2013 0x6693a0 (102 B)
void FStatePlayerMasterInDialog_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FStatePlayerMasterBase_Param::OnPending( PendingState, ManagedObject );
	UStatePlayerMasterInDialog* InDialog = (UStatePlayerMasterInDialog*)PendingState;
	InDialog->StopTrackingForDialog();
	InDialog->m_pDialogRunningInst = (FPointer)m_pDialogRunningInst;
	InDialog->m_bAllowPlayerToCarryBody = m_bAllowPlayerToCarryBody ? 1 : 0;
	InDialog->m_ViewTrackID = -1;
	InDialog->m_fSoireeBlendOutTime = 0.4f;
}

// 2013 0x673720 (709 B; 2012 0x6b8b20, 521 B)
void UStatePlayerMasterInDialog::OnEnterState( UDishonoredNativeState* LastState )
{
	UStatePlayerMasterChoice_Base::OnEnterState( LastState );
	m_pPlayerPawn->StopSneaking( FALSE );
	m_pPlayerPawn->bWantsToCrouch = FALSE;
	if( m_pPlayerPawn->IsCarryingCorpse() )
	{
		m_pPlayerPawn->m_pInventory->SetReequipItem( EDisEquipUsage_Primary, UDishonoredItemEmpty::StaticClass() );
		if( !m_bAllowPlayerToCarryBody )
		{
			static UBOOL bNoted = FALSE;
			if( !bNoted )
			{
				bNoted = TRUE;
				debugf( TEXT("DISHONORED(bringup): UStatePlayerMasterInDialog::OnEnterState: the carried corpse is not dropped (the 2013 carry path is not ported)") );
			}
		}
	}
	if( m_pPlayerPawn->HasNothingEquipped() )
	{
		m_pPlayerPawn->m_pInventory->SetReequipItem( EDisEquipUsage_Primary, UDishonoredItemEmpty::StaticClass() );
		m_pPlayerPawn->m_pInventory->SetReequipItem( EDisEquipUsage_Secondary, UDishonoredItemEmpty::StaticClass() );
	}
	else
	{
		m_pPlayerPawn->EquipItemByType_KeepTrying( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Primary, ADishonoredPawn::EEquipItemFlags_TempFastUnequip );
		m_pPlayerPawn->EquipItemByType_KeepTrying( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Secondary, ADishonoredPawn::EEquipItemFlags_TempFastUnequip );
	}
	if( m_pPlayerPawn->m_pInventory && m_pPlayerPawn->IsHoldingMovableObject() )
	{
		m_pPlayerPawn->DropMovableObject( FALSE );
	}
	JEDIAG( "dialog state entered from %s: running instance %s, carry body %d, letterboxing %d", LastState ? *LastState->GetClass()->GetName() : TEXT("none"),
		m_pDialogRunningInst ? TEXT("set") : TEXT("none"), (INT)m_bAllowPlayerToCarryBody, (INT)m_bUseLetterboxing );
}

// 2013 0x6739f0 (240 B; 2012 0x6b8d30)
void UStatePlayerMasterInDialog::OnExitState( UDishonoredNativeState* NextState )
{
	UStatePlayerMasterChoice_Base::OnExitState( NextState );
	if( NextState )
	{
		if( m_SoireeControlState.m_bControlledBySoiree )
		{
			m_SoireeControlState.ExitSoireeMode( m_pPlayerPawn, m_fSoireeBlendOutTime, TRUE );
		}
		if( !NextState->IsA( UStatePlayerMasterInStore::StaticClass() ) )
		{
			if( UClass* Primary = m_pPlayerPawn->m_pInventory->GetReequipItem( EDisEquipUsage_Primary, FALSE ) )
			{
				m_pPlayerPawn->EquipItemByType_KeepTrying( Primary, EDisEquipUsage_Primary, ADishonoredPawn::EEquipItemFlags_None );
			}
			if( UClass* Secondary = m_pPlayerPawn->m_pInventory->GetReequipItem( EDisEquipUsage_Secondary, FALSE ) )
			{
				m_pPlayerPawn->EquipItemByType_KeepTrying( Secondary, EDisEquipUsage_Secondary, ADishonoredPawn::EEquipItemFlags_None );
			}
		}
	}
	if( m_pDialogRunningInst )
	{
		( (FDisDialogRunningInstance*)m_pDialogRunningInst )->StopInstance( DisConvoEndReason_PlayerExit );
		m_pDialogRunningInst = NULL;
	}
	m_pActivePlayerChoice = NULL;
	StopTrackingForDialog();
	JEDIAG( "dialog state left for %s", NextState ? *NextState->GetClass()->GetName() : TEXT("none") );
}

// 2013 0x679280 (148 B; 2012 0x6c94c0): the base's tick, a timed look-at constraint counted down, and the soiree control.
void UStatePlayerMasterInDialog::TickState( FLOAT DeltaSeconds )
{
	UStatePlayerMasterChoice_Base::TickState( DeltaSeconds );
	if( m_bUseConstraints && m_LookAtConstraints.m_bUseDuration )
	{
		m_LookAtConstraints.m_fDuration -= DeltaSeconds;
		if( m_LookAtConstraints.m_fDuration <= 0.f )
		{
			StopTrackingForDialog();
		}
	}
	m_SoireeControlState.TickSoireeMode( m_pPlayerPawn );
}

// 2012 0x6b8ea0 (23 B, unmatched in 2013): the instance is no longer this state's to stop, and the bend time is released.
void UStatePlayerMasterInDialog::RequestStateExit_Derived()
{
	UStatePlayerMasterChoice_Base::RequestStateExit_Derived();
	m_pDialogRunningInst = NULL;
	UnpauseBendTime();
}

// 2013 0x660970 / 0x660990 (2012 0x6afa30 / 0x6afa50): the node playing now decides
void UStatePlayerMasterInDialog::PlayerRequestSkip_Derived()
{
	FDisDialogRunningInstance* Instance = (FDisDialogRunningInstance*)m_pDialogRunningInst;
	if( Instance && Instance->m_pCurPlayingNode )
	{
		Instance->m_pCurPlayingNode->InterruptTickingNode( Instance, DisConvoEndReason_PlayerSkipWhole );
	}
}

UBOOL UStatePlayerMasterInDialog::IsSkippable_Derived() const
{
	FDisDialogRunningInstance* Instance = (FDisDialogRunningInstance*)m_pDialogRunningInst;
	if( Instance && Instance->m_pCurPlayingNode )
	{
		return Instance->m_pCurPlayingNode->IsTickingNodeInterruptible( Instance, DisConvoEndReason_PlayerSkipWhole );
	}
	return FALSE;
}

// 2013 0x663d90 (44 B)
void UStatePlayerMasterInDialog::StopTrackingForDialog()
{
	ADishonoredPlayerController* PC = (ADishonoredPlayerController*)m_pPlayerPawn->Controller;
	if( PC )
	{
		PC->StopTrackingTarget( m_ViewTrackID );
		m_bUseConstraints = FALSE;
		m_ViewTrackID = -1;
	}
}

// 2013 0x663cc0 (132 B, 2012 0x6b40d0 the same): the field of view that frames a sphere of that radius at that point, twice the
// half-angle in degrees; 0 (no override) when the controller has no camera or the point is the eye.
FLOAT UStatePlayerMasterInDialog::CalcZoomFOVBasedOnRadius( const FVector& _rTargetPos, FLOAT _fTargetRadius ) const
{
	APlayerController* PC = Cast<APlayerController>( m_pPlayerPawn->Controller );
	if( !PC || !PC->PlayerCamera )
	{
		return 0.f;
	}
	const FVector ViewLoc = m_pPlayerPawn->GetPawnViewLocation();
	const FLOAT Distance = ( _rTargetPos - ViewLoc ).Size();
	if( Distance == 0.f )
	{
		return 0.f;
	}
	const FLOAT HalfAngle = appAtan2( _fTargetRadius / Distance, 1.f ) * ( 180.f / PI );
	return HalfAngle + HalfAngle;
}

/**
 * 2013 0x669480 (280 B; 2012 0x6b8ec0): the previous track stopped, then the pawn's region tracked in pitch and yaw, held (flags
 * 0x43); constraints limit the player's push (0x47) and may zoom on the region (0x147), and 0x200 caches the bone relative to the
 * controller when the NPC's animation is to be ignored. The constraints are kept for the timed release in TickState.
 * DISHONORED(bringup): the zoom frames ADishonoredPawn::GetBBox_ForRegion (2012 0x7a1250, the hit-region table, not ported); the
 * pawn's collision bounds stand in.
 */
void UStatePlayerMasterInDialog::TrackPawnForDialog( ADishonoredPawn* _pPawn, FLOAT _fBlendTime, BYTE _FocusHitRegion, const FDisConvLookAtConstraints* _pConstraints )
{
	StopTrackingForDialog();
	ADishonoredPlayerController* PC = (ADishonoredPlayerController*)m_pPlayerPawn->Controller;
	INT Flags = 0x43;
	FDisPlayerViewTracking_ExtraParams ExtraParams( EC_EventParm );
	if( _pConstraints )
	{
		ExtraParams.m_fMaxPlayerControlledDelta_Pitch = _pConstraints->m_fMaxPitchDelta;
		ExtraParams.m_fMaxPlayerControlledDelta_Yaw = _pConstraints->m_fMaxYawDelta;
		Flags = 0x47;
		if( _pConstraints->m_bUseZoom && _pPawn->CollisionComponent )
		{
			Flags = 0x147;
			ExtraParams.m_fZoomFOVOverride = CalcZoomFOVBasedOnRadius( _pPawn->CollisionComponent->Bounds.Origin, _pConstraints->m_fZoomPercentOnScreen * _pPawn->CollisionComponent->Bounds.SphereRadius );
		}
		if( _pConstraints->m_bIgnoreNPCAnimation )
		{
			Flags |= 0x200;
		}
	}
	const INT TrackID = PC ? PC->StartTrackingPawn_ByRegion( _pPawn, _FocusHitRegion, eDisRegionFocusType_Conversation, _fBlendTime, Flags, &ExtraParams ) : -1;
	JEDIAG( "dialog look-at pawn %s blend %.2f flags 0x%x -> track %d", _pPawn ? *_pPawn->GetName() : TEXT("none"), _fBlendTime, Flags, TrackID );
	if( TrackID != -1 )
	{
		m_ViewTrackID = TrackID;
		if( _pConstraints )
		{
			m_LookAtConstraints = *_pConstraints;
			m_bUseConstraints = TRUE;
		}
		else
		{
			m_bUseConstraints = FALSE;
		}
	}
}

// 2013 0x6695a0 (311 B; 2012 0x6b8ff0): the same for an actor, tracked at its collision bounds (no bone, no offset), and the zoom
// frames those bounds.
void UStatePlayerMasterInDialog::TrackActorForDialog( AActor* _pActor, FLOAT _fBlendTime, const FDisConvLookAtConstraints* _pConstraints )
{
	StopTrackingForDialog();
	ADishonoredPlayerController* PC = (ADishonoredPlayerController*)m_pPlayerPawn->Controller;
	INT Flags = 0x43;
	FDisPlayerViewTracking_ExtraParams ExtraParams( EC_EventParm );
	if( _pConstraints )
	{
		ExtraParams.m_fMaxPlayerControlledDelta_Pitch = _pConstraints->m_fMaxPitchDelta;
		ExtraParams.m_fMaxPlayerControlledDelta_Yaw = _pConstraints->m_fMaxYawDelta;
		Flags = 0x47;
		if( _pConstraints->m_bUseZoom && _pActor->CollisionComponent )
		{
			Flags = 0x147;
			ExtraParams.m_fZoomFOVOverride = CalcZoomFOVBasedOnRadius( _pActor->CollisionComponent->Bounds.Origin, _pActor->CollisionComponent->Bounds.SphereRadius * _pConstraints->m_fZoomPercentOnScreen );
		}
	}
	const INT TrackID = PC ? PC->StartTrackingActor( _pActor, NULL, NAME_None, FVector( 0.f, 0.f, 0.f ), _fBlendTime, Flags, &ExtraParams ) : -1;
	JEDIAG( "dialog look-at actor %s blend %.2f flags 0x%x -> track %d", _pActor ? *_pActor->GetName() : TEXT("none"), _fBlendTime, Flags, TrackID );
	if( TrackID != -1 )
	{
		m_ViewTrackID = TrackID;
		if( _pConstraints )
		{
			m_LookAtConstraints = *_pConstraints;
			m_bUseConstraints = TRUE;
		}
		else
		{
			m_bUseConstraints = FALSE;
		}
	}
}

// 2013 0x669410 (102 B; 2012 0x6b8e30): the node is the state's choice, the list starts at its first entry, every master input is
// taken away and the bend time is paused (the list shows once it has stopped the player, TickState)
void UStatePlayerMasterInDialog::SetActivePlayerChoice( const UDisConv_PlayerChoice* _pPlayerChoice )
{
	m_pActivePlayerChoice = const_cast<UDisConv_PlayerChoice*>( _pPlayerChoice );
	m_iCurChoice = 0;
	m_DialogState = 1;
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, FALSE, DisInputMaskLevel_MasterAction );
	PauseBendTime();
	JFDIAG( "dialog state: active player choice %s, bend time paused", _pPlayerChoice ? *_pPlayerChoice->GetName() : TEXT("none") );
}

// 2013 0x6608c0 (162 B; 2012 0x6af980): the static choices and the optional ones whose story flag holds on the player
INT UStatePlayerMasterInDialog::GetNumChoices()
{
	const INT NumStatic = m_pActivePlayerChoice->GetNumChoices();
	INT NumOptional = 0;
	const INT NumOptionalChoices = m_pActivePlayerChoice->GetNumOptionalChoices();
	for( INT OptionalIndex = 0; OptionalIndex < NumOptionalChoices; OptionalIndex++ )
	{
		const FDisPlayerChoice_Optional& Optional = m_pActivePlayerChoice->GetOptionalChoice( OptionalIndex );
		const FGuid& Flag = Optional.m_StoryFlag;
		if( Optional.m_pStoryFlagSet && ( Flag.A | Flag.B | Flag.C | Flag.D ) && m_pPlayerPawn->CheckStoryFlag( Optional.m_pStoryFlagSet, Flag ) == !Optional.m_bStoryFlagMustBeFalse )
		{
			NumOptional++;
		}
	}
	return NumOptional + NumStatic;
}

// 2013 0x663dc0 (96 B; 2012 0x6b4190): a static choice's text, or the shown optional choice at that index
void UStatePlayerMasterInDialog::GetChoiceInfo( INT _iIndex, FDisPlayerChoiceInfo& _rChoiceInfo )
{
	if( m_pActivePlayerChoice->IsValidChoice( _iIndex ) )
	{
		_rChoiceInfo.m_ChoiceText = m_pActivePlayerChoice->GetChoice( _iIndex ).GetChoiceText();
		return;
	}
	INT OutputIndex = _iIndex;
	_rChoiceInfo.m_ChoiceText = m_pActivePlayerChoice->GetOptionalChoiceForIndex( _iIndex, OutputIndex )->GetChoiceText();
}

// 2013 0x663d50 (55 B, unnamed, new in 2013): the dialog's view track is still blending in (or still zooming)
UBOOL UStatePlayerMasterInDialog::IsDialogViewTrackBlending() const
{
	ADishonoredPlayerController* PC = (ADishonoredPlayerController*)m_pPlayerPawn->Controller;
	if( !PC )
	{
		return FALSE;
	}
	UBOOL bBlending = FALSE;
	if( !PC->IsTrackingTarget( m_ViewTrackID, &bBlending ) )
	{
		return FALSE;
	}
	return bBlending;
}
