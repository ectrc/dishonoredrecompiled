// DishonoredGame/src/interptracklocomotion_game.cpp
// DISHONORED(port): agent GA (PHASE18 package GA). The half of UInterpTrackInstLocomotion (an Engine class,
// Engine/Inc/EngineInterpolationClasses.h) that drives FArkComponentLocomotion, which this tree declares in
// DishonoredGame (Inc/arkcomponentlocomotion.h, agent DN's reason). Same arrangement as agent FK's interpgroupplayer.cpp:
// the Engine emits the class's vtable and the linker takes these virtual bodies out of DishonoredGameModule.lib.
// Retail's units: Engine/Src/interptracklocomotion.cpp (UpdateTrackInst 2012 0x542630 / 2013 0x503c30) and
// interptrackinstlocomotion.cpp (TermTrackInst 0x53bb90 / 0x4fca10, CallbackLocoEvent 0x53bc00, StopCurrentLocoRequest
// 0x53ce50 / 0x4fdce0, UpdateStopObsoleteLocoRequest 0x53f760 / 0x5003e0, ComputeLocoProp 0x5423d0 / 0x5037e0);
// GetGameSpecificLocomotionModifier is DishonoredGame's (2012 0x7ab590 / 2013 0x74b030, dishonorednpcpawn.cpp region).
// The request-property defaults are FArkCpntLocoGoToActorProp::InitAllByDefaultExcept (2012 0x5803d0 / 2013 0x53f1c0)
// and the Overwrite* setters (2013 0x53f160 EndLocationThreshold, 0x53e700 StopBeforeEndDist, 0x53f190 SpeedMultiplier),
// written inline on agent DN's struct.

#include "DishonoredGame.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "arkcomponentlocomotion.h"
#include "disdesirestructs.h"

// 2012 0x7ab590 (27 B), 2013 0x74b030: the only key modifier with a locomotion modifier of its own is the sword, 5
INT GetGameSpecificLocomotionModifier( BYTE _SoireeModifier )
{
	return ( _SoireeModifier == EKLM_Sword ) ? 5 : 0;
}

/** -gadiag (or -fkdiag): every request the track starts, updates or stops is logged. */
static UBOOL DisGADiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-gadiag") ) != NULL || appStrfind( appCmdLine(), TEXT("-fkdiag") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

// Retail hands the component a member-function pointer (UObject::*)(INT, EArkCpntLocoEvent); agent DN's callback type
// is a free function taking the owner, so this is the one line between the two.
static void DisLocoTrackCallbackThunk( UObject* _pCallbackOwner, const INT _RequestID, const BYTE _Event )
{
	UInterpTrackInstLocomotion* Inst = Cast<UInterpTrackInstLocomotion>( _pCallbackOwner );
	if( Inst )
	{
		Inst->CallbackLocoEvent( _RequestID, _Event );
	}
}

// 2012 0x53bc00 (52 B): a reached destination retires the goal key; the request's stop is deferred to the next update
void UInterpTrackInstLocomotion::CallbackLocoEvent( INT _iRequestID, BYTE _LocoEvent )
{
	if( DisGADiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): gadiag locotrack %s: request %d event %d (goal key %d)"), *GetPathName(), _iRequestID, (INT)_LocoEvent, m_CurrentGoalKeyIndex );
	}
	if( _LocoEvent == CPNT_LOCO_EVENT_DESTINATION_REACHED )
	{
		m_iPendingStopLocoRequestId = m_iCurrentLocoRequestId;
		m_LastReachedGoalKeyIndex = m_CurrentGoalKeyIndex;
		m_CurrentGoalKeyIndex = INDEX_NONE;
		m_iCurrentLocoRequestId = INDEX_NONE;
	}
}

// 2012 0x53ce50 (36 B), 2013 0x4fdce0
void UInterpTrackInstLocomotion::StopCurrentLocoRequest( FArkComponentLocomotion* const _pLocoComp, INT iKeyIndex )
{
	_pLocoComp->StopLoco( m_iCurrentLocoRequestId );
	m_iCurrentLocoRequestId = INDEX_NONE;
}

// 2012 0x53f760 (157 B), 2013 0x5003e0: a goal key that asked to stop its request at its end does so once the
// position is past that end
void UInterpTrackInstLocomotion::UpdateStopObsoleteLocoRequest( FArkComponentLocomotion* const _pLocoComp, FLOAT _fNewPosition )
{
	if( m_iCurrentLocoRequestId == INDEX_NONE )
	{
		return;
	}
	const UInterpTrackLocomotion* LocoTrack = CastChecked<UInterpTrackLocomotion>( Track );
	if( !LocoTrack->LocoKeys.IsValidIndex( m_CurrentGoalKeyIndex ) )
	{
		return;
	}
	const FLocomotionControlTrackKey& Key = LocoTrack->LocoKeys(m_CurrentGoalKeyIndex);
	if( Key.Properties && Key.Properties->m_bStopLocoRequestAtEnd && _fNewPosition > Key.StartTime + Key.KeyLength )
	{
		_pLocoComp->StopLoco( m_iCurrentLocoRequestId );
		m_iCurrentLocoRequestId = INDEX_NONE;
		m_CurrentGoalKeyIndex = INDEX_NONE;
	}
}

// 2012 0x5423d0 (259 B), 2013 0x5037e0: the request a key describes. The defaults are InitAllByDefaultExcept's (the
// config's last speed index, a 20-unit end threshold, accurate stop), then the key overrides the max speed, the end
// threshold (0), the stop distance (at least 150 units when the target is the player), the accurate-stop flag (the
// key's, or any end speed), the end speed and the speed multiplier (the track's float curve at this position, at least
// 0.01).
void UInterpTrackInstLocomotion::ComputeLocoProp( FArkCpntLocoGoToActorProp& _LocoProp, FArkComponentLocomotion* _pLocoComp, const AActor* const _pTarget, const UInterpTrackLocomotionKeyProperties* const _pKeyProperties, FLOAT _fNewPosition )
{
	const FLOAT MinStopDistance = ( _pKeyProperties->m_TargetName == UInterpTrackLocomotion::PlayerTargetName() ) ? 150.f : 0.f;
	const FLOAT StopDistance = Max<FLOAT>( _pKeyProperties->m_fTargetStopDistance, MinStopDistance );
	const UBOOL bAccurateStop = _pKeyProperties->m_bAcurateStop || _pKeyProperties->m_iEndSpeedIdxNumber > 0;
	const UInterpTrackLocomotion* LocoTrack = CastChecked<UInterpTrackLocomotion>( Track );
	const FLOAT SpeedMultiplier = Max<FLOAT>( LocoTrack->FloatTrack.Eval( _fNewPosition, 1.f ), 0.01f );

	const UArkComponentLocomotionConfig* Config = _pLocoComp->GetConfig();
	_LocoProp.m_CommonProps.m_pCallbackOwner = this;
	_LocoProp.m_CommonProps.m_LocoEventCallback = &DisLocoTrackCallbackThunk;
	_LocoProp.m_CommonProps.m_MaxSpeedIdx = Config ? Config->m_SpeedNames.Num() - 1 : 0;
	_LocoProp.m_CommonProps.m_EndSpeedIdx = 0;
	_LocoProp.m_CommonProps.m_fEndLocationThreshold = 20.f;
	_LocoProp.m_CommonProps.m_fMaxFunnelRadiusMultiplier = 1.f;
	_LocoProp.m_CommonProps.m_fSpeedMultiplier = 1.f;
	_LocoProp.m_CommonProps.m_fStopBeforeEndDist = 0.f;
	_LocoProp.m_CommonProps.m_bAccurateStop = TRUE;
	_LocoProp.m_CommonProps.m_bSpeedIsLookAtDependent = FALSE;
	_LocoProp.m_pLocoCpnt = _pLocoComp;
	_LocoProp.m_bInitialized = TRUE;
	_LocoProp.m_ActorProps.m_pActor = _pTarget;
	_LocoProp.m_ActorProps.m_fFollowAngle = 0.f;
	_LocoProp.m_ActorProps.m_fFollowDist = 0.f;
	_LocoProp.m_ActorProps.m_bFollow = FALSE;

	_LocoProp.m_CommonProps.m_MaxSpeedIdx = _pKeyProperties->m_iSpeedIdxNumber;
	_LocoProp.m_CommonProps.m_fEndLocationThreshold = 0.f;
	_LocoProp.m_CommonProps.m_fStopBeforeEndDist = StopDistance;
	_LocoProp.m_CommonProps.m_bAccurateStop = bAccurateStop;
	_LocoProp.m_CommonProps.m_EndSpeedIdx = _pKeyProperties->m_iEndSpeedIdxNumber;
	_LocoProp.m_CommonProps.m_fSpeedMultiplier = SpeedMultiplier;
}

// 2012 0x53bb90 (109 B), 2013 0x4fca10: both outstanding requests stopped on a started component, the bookkeeping reset
void UInterpTrackInstLocomotion::TermTrackInst( UInterpTrack* InTrack )
{
	APawn* Pawn = GetGroupPawn();
	FArkComponentLocomotion* Loco = Pawn ? FArkComponentLocomotion::GetLocomotionComponent( Pawn ) : NULL;
	if( Loco && Loco->m_bStarted )
	{
		if( m_iCurrentLocoRequestId != INDEX_NONE )
		{
			Loco->StopLoco( m_iCurrentLocoRequestId );
		}
		if( m_iPendingStopLocoRequestId != INDEX_NONE )
		{
			Loco->StopLoco( m_iPendingStopLocoRequestId );
		}
	}
	m_CurrentGoalKeyIndex = INDEX_NONE;
	m_LastReachedGoalKeyIndex = INDEX_NONE;
	m_iCurrentLocoRequestId = INDEX_NONE;
	m_iPendingStopLocoRequestId = INDEX_NONE;
}

// 2012 0x542630 (1,144 B), 2013 0x503c30 (1,144 B). Retail's order: nothing without a group pawn; a jump is handed to
// UpdateTrackInst_Jump first and ends the update when it took it; nothing without a started locomotion component; the
// obsolete-request test and the deferred stop; then the newest key whose start the position has just crossed is taken
// as the goal - its target actor found through the key's group (or the player): a target that is the pawn itself is
// ignored, a missing one is a "bad target" warning, and a real one starts a go-to-actor request (or, with a zero speed
// index, is marked reached at once) and applies the key's modifier; then the current goal's request is updated with the
// target's position every frame (or retired, with a warning, when its group lost its actor); last the position.
void UInterpTrackInstLocomotion::UpdateTrackInst( FLOAT NewPosition, UBOOL bJump )
{
	APawn* Pawn = GetGroupPawn();
	if( !Pawn )
	{
		return;
	}
	USeqAct_Interp* Interp = GetMatinee();
	const UInterpTrackLocomotion* LocoTrack = CastChecked<UInterpTrackLocomotion>( Track );
	if( bJump && UpdateTrackInst_Jump( NewPosition ) )
	{
		return;
	}
	FArkComponentLocomotion* Loco = FArkComponentLocomotion::GetLocomotionComponent( Pawn );
	if( !Loco || !Loco->m_bStarted )
	{
		return;
	}
	UpdateStopObsoleteLocoRequest( Loco, NewPosition );
	if( m_iPendingStopLocoRequestId != INDEX_NONE )
	{
		Loco->StopLoco( m_iPendingStopLocoRequestId );
		m_iPendingStopLocoRequestId = INDEX_NONE;
	}

	APlayerController* LocalPC = Pawn->GetALocalPlayerController();
	for( INT KeyIndex = LocoTrack->LocoKeys.Num() - 1; KeyIndex >= 0; KeyIndex-- )
	{
		const FLocomotionControlTrackKey& Key = LocoTrack->LocoKeys(KeyIndex);
		if( !( ( Key.StartTime > m_LastPosition || ( Key.StartTime == 0.f && NewPosition == 0.f ) ) && NewPosition >= Key.StartTime ) )
		{
			continue;
		}
		UInterpTrackLocomotionKeyProperties* Props = Key.Properties;
		if( !Props )
		{
			break;
		}
		AActor* Target = Props->FindKeyTargetActor( Interp, LocalPC );
		if( Target != Pawn )
		{
			if( Target )
			{
				if( ( m_CurrentGoalKeyIndex != KeyIndex && m_LastReachedGoalKeyIndex != KeyIndex ) || ( m_CurrentGoalKeyIndex != INDEX_NONE && m_iCurrentLocoRequestId == INDEX_NONE ) )
				{
					const UBOOL bHadRequest = ( m_iCurrentLocoRequestId != INDEX_NONE );
					m_CurrentGoalKeyIndex = KeyIndex;
					if( bHadRequest )
					{
						StopCurrentLocoRequest( Loco, m_CurrentGoalKeyIndex );
					}
					if( Props->m_iSpeedIdxNumber != 0 )
					{
						FArkCpntLocoGoToActorProp LocoProp;
						ComputeLocoProp( LocoProp, Loco, Target, Props, NewPosition );
						m_iCurrentLocoRequestId = Loco->StartLocoToActor( this, GetFName(), UInterpTrackLocomotion::s_iInterpTrackLocomotionPriority, LocoProp );
						if( DisGADiagEnabled() )
						{
							debugf( TEXT("DISHONORED(bringup): gadiag locotrack %s group '%s' pos %.2f: key %d -> %s walks to %s (speed idx %d, end idx %d, stop %.0f, mult %.2f, wait %d): request %d"),
								Interp ? *Interp->GetName() : TEXT("?"), *Cast<UInterpGroupInst>( GetOuter() )->Group->GroupName.ToString(), NewPosition, KeyIndex, *Pawn->GetName(), *Target->GetName(),
								Props->m_iSpeedIdxNumber, Props->m_iEndSpeedIdxNumber, LocoProp.m_CommonProps.m_fStopBeforeEndDist, LocoProp.m_CommonProps.m_fSpeedMultiplier, (INT)Props->WaitForGoalReached, m_iCurrentLocoRequestId );
						}
					}
					else
					{
						m_LastReachedGoalKeyIndex = m_CurrentGoalKeyIndex;
						m_CurrentGoalKeyIndex = INDEX_NONE;
					}
					Loco->SetModifier( GetGameSpecificLocomotionModifier( Props->m_Modifier ) );
				}
			}
			else
			{
				debugf( NAME_Warning, TEXT("Soiree %s Group %s : Locomotion bad target '%s'"),
					Interp ? *Interp->ObjComment : TEXT("?"), *Cast<UInterpGroupInst>( GetOuter() )->Group->GroupName.ToString(), *Props->m_TargetName.ToString() );
			}
		}
		break;
	}

	if( m_CurrentGoalKeyIndex != INDEX_NONE && LocoTrack->LocoKeys.IsValidIndex( m_CurrentGoalKeyIndex ) )
	{
		UInterpTrackLocomotionKeyProperties* Props = LocoTrack->LocoKeys(m_CurrentGoalKeyIndex).Properties;
		AActor* Target = Props ? Props->FindKeyTargetActor( Interp, LocalPC ) : NULL;
		if( Target )
		{
			FArkCpntLocoGoToActorProp LocoProp;
			ComputeLocoProp( LocoProp, Loco, Target, Props, NewPosition );
			Loco->UpdateLocoToActor( m_iCurrentLocoRequestId, LocoProp );
		}
		else if( m_LastReachedGoalKeyIndex != m_CurrentGoalKeyIndex )
		{
			UInterpGroupInst* TargetInst = ( Interp && Props ) ? Interp->FindFirstGroupInst( Props->m_Target ) : NULL;
			debugf( NAME_Warning, TEXT("%s : No target connected to group %s"), Interp ? *Interp->GetName() : TEXT("?"),
				( TargetInst && TargetInst->Group ) ? *TargetInst->Group->GroupName.ToString() : TEXT("?") );
			m_LastReachedGoalKeyIndex = m_CurrentGoalKeyIndex;
			m_CurrentGoalKeyIndex = INDEX_NONE;
		}
	}
	m_LastPosition = NewPosition;
}
