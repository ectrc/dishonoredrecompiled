// DishonoredGame/src/arkcomponentlocomotionrootmove.cpp
// DISHONORED(port): agent GK. The turn-in-place / start / stop animation system of FArkComponentLocomotion - retail's
// Engine/Src/arkcomponentlocomotionrootmove.cpp, 15 functions, all transcribed from the 2012 database and checked against the
// 2013 one (the 2013 bodies are the same control flow on a layout 8 bytes longer; the 2012 layout is the one this tree keeps).
// It drives UArkAnimNodeLocomotion's specific animations (GC's port): a turn when the body has to swing more than 15 degrees
// before or during a path, a start animation when the path ahead is straight for the start's distance, and a stop
// animation chosen by which foot is forward and how far the mark is - the "landing" agent GD's two stand-ins in
// arkcomponentlocomotiondynamic.cpp imitated.
//
// | function | 2013 rva | 2012 rva (size) |
// | UpdateTurn | 0x54b9d0 | 0x58c520 (1,260) |
// | UpdateStartMove | 0x546f60 | 0x587c40 (916) |
// | UpdateStopMove | 0x547300 | 0x587fe0 (1,515) |
// | RequestTurn | 0x54af40 | 0x58ba90 (1,451) |
// | EvaluateStaticTurn | 0x54b4f0 | 0x58c040 (651) |
// | EvaluateStartPathTurn | 0x54b780 | 0x58c2d0 (241) |
// | EvaluatePathTurn | 0x54b880 | 0x58c3d0 (330) |
// | IsSpeedModeCanDoTurn | 0x54ae10 | 0x58b960 (299) |
// | ResetTurn / ResetStartMove / ResetStopMove | 0x545a10 / 0x545ae0 / 0x545b70 | 0x5868d0 / 0x5869a0 / 0x586a30 |
// | SetRootState | 0x5459a0 | 0x586860 (100) |
// | IsReadyToTurn | 0x53e7a0 | 0x57fad0 (113) |
// | IsTurnNotTooCloseFromPathEnd | 0x542b20 | 0x583af0 (143) |
// | IsLocoFaceToRequestActive | 0x53e750 | 0x57fa80 (70) |
//
// FArkComponentFaceTo is created by SetConfig for every NPC (IK4) and read as retail reads it (agent IO). UArkAvoidable
// (m_pAvoidable may be NULL - retail's GetAffectorCount / GetForce are the members m_AffectorCount / m_Force, read through the
// two helpers below).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"
#include "arkcomponentfaceto.h"

static INT DisLocoAffectorCount( const UArkAvoidable* _pAvoidable )
{
	return _pAvoidable ? _pAvoidable->m_AffectorCount : 0;
}

static FVector DisLocoAvoidanceForce( const UArkAvoidable* _pAvoidable )
{
	return _pAvoidable ? _pAvoidable->m_Force : FVector( 0.f, 0.f, 0.f );
}

// The shortest signed way from one unreal-unit yaw to another; retail inlines it everywhere in these fifteen functions.
static INT DisLocoYawDelta( INT _From, INT _To )
{
	return (INT)(SWORD)(WORD)( _To - _From );
}

static FLOAT DisLocoYawToDegrees( INT _Yaw )
{
	return (FLOAT)_Yaw * 0.0054931641f;
}

/*-----------------------------------------------------------------------------
	the state
-----------------------------------------------------------------------------*/

void FArkComponentLocomotion::SetRootState( EArkCpntLocoRootState _RootState )
{
	const UBOOL bWasPathOwned = ( m_RootState == CPNT_LOCO_ROOT_STATE_NONE || m_RootState == CPNT_LOCO_ROOT_STATE_TURN_END );
	const UBOOL bIsPathOwned = ( _RootState == CPNT_LOCO_ROOT_STATE_NONE || _RootState == CPNT_LOCO_ROOT_STATE_TURN_END );
	m_RootState = _RootState;
	if( bWasPathOwned != bIsPathOwned )
	{
		m_bIsArrived = ComputeIsArrivedFlagAndSq2DDistToPathEnd( m_fSq2DDistToPathEnd );
	}
}

void FArkComponentLocomotion::ResetTurn()
{
	if( m_RootState < CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT || m_RootState > CPNT_LOCO_ROOT_STATE_TURN_END )
	{
		return;
	}
	if( m_pAnimNodeLoco && m_pAnimNodeLoco->IsSpecificAnimRequested() && m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() == ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN )
	{
		m_pAnimNodeLoco->AbortSpecificAnimRequest( FALSE );
	}
	SetRootState( CPNT_LOCO_ROOT_STATE_NONE );
	if( m_pPawnOwner && m_pPawnOwner->Mesh )
	{
		m_pPawnOwner->Mesh->RootRotationScale = 1.f;
		m_pPawnOwner->Mesh->AdditionalRootRotation = FRotator( 0, 0, 0 );
	}
}

void FArkComponentLocomotion::ResetStartMove()
{
	if( m_RootState < CPNT_LOCO_ROOT_STATE_START_WAIT_ANIM || m_RootState > CPNT_LOCO_ROOT_STATE_START_END )
	{
		return;
	}
	if( m_pAnimNodeLoco && m_pAnimNodeLoco->IsSpecificAnimRequested() && m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() == ANIM_NODE_LOCO_SPECIFIC_ANIM_START )
	{
		m_pAnimNodeLoco->AbortSpecificAnimRequest( FALSE );
	}
	SetRootState( CPNT_LOCO_ROOT_STATE_NONE );
}

void FArkComponentLocomotion::ResetStopMove()
{
	if( m_RootState < CPNT_LOCO_ROOT_STATE_STOP_APPROACH || m_RootState > CPNT_LOCO_ROOT_STATE_STOP_END )
	{
		return;
	}
	if( m_pAnimNodeLoco && m_pAnimNodeLoco->IsSpecificAnimRequested() && m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() == ANIM_NODE_LOCO_SPECIFIC_ANIM_STOP )
	{
		m_pAnimNodeLoco->AbortSpecificAnimRequest( FALSE );
	}
	SetRootState( CPNT_LOCO_ROOT_STATE_NONE );
}

/*-----------------------------------------------------------------------------
	the tests
-----------------------------------------------------------------------------*/

UBOOL FArkComponentLocomotion::IsReadyToTurn() const
{
	return !m_bInertiaDisabled && m_RootState == CPNT_LOCO_ROOT_STATE_NONE
		&& m_bCanManageAnim && m_pAnimNodeLoco && !m_pAnimNodeLoco->IsSpecificAnimPlaying()
		&& ( !m_pAnimNodeLoco->IsSpecificAnimRequested() || m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() == ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN );
}

UBOOL FArkComponentLocomotion::IsTurnNotTooCloseFromPathEnd( INT _TurnPathPointIdx ) const
{
	const FVector& TurnPoint = m_PathPoints( _TurnPathPointIdx ).m_Point;
	const FLOAT fDX = TurnPoint.X - m_LastPathPoint.X;
	const FLOAT fDY = TurnPoint.Y - m_LastPathPoint.Y;
	return ( fDY * fDY + fDX * fDX ) > ( m_pConfig->m_fMaxDistToPathEndToTurn * m_pConfig->m_fMaxDistToPathEndToTurn );
}

// DISHONORED(port, agent IO): retail's body (2012 0x57fa80): no face-to request, or the active one is this component's, or it is
// at most the locomotion's priority. The stand-in answered "no face-to component", which since FaceTo is on by default (IK4)
// was FALSE for every NPC and switched off the static, start-path and path turns.
UBOOL FArkComponentLocomotion::IsLocoFaceToRequestActive() const
{
	return m_pFaceToCpnt->GetRequestsCount() == 0 || m_pFaceToCpnt->GetActiveRequestID() == m_FaceToRequestID
		|| m_pFaceToCpnt->GetActiveRequestPriority() <= m_pConfig->m_FaceToPriority;
}

UBOOL FArkComponentLocomotion::IsSpeedModeCanDoTurn( INT _SpeedModeIdx, INT _YawDif, INT& _TurnAnimToPlayIdx )
{
	const FArkCpntLocoSpeedModeProp& SpeedMode = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( _SpeedModeIdx ) );
	const INT TurnAnimCount = Min( SpeedMode.m_TurnAnimProps.Num(), 8 );
	if( TurnAnimCount == 0 )
	{
		return FALSE;
	}
	// The turn animations come in pairs (left, right) for 180, 135, 90 and 45 degrees; the pair whose angle is within the
	// threshold of the swing wins, a two-animation set accepting anything within 45 degrees of a half turn.
	const INT AbsYawDif = Abs( _YawDif );
	const INT PairCount = TurnAnimCount >> 1;
	const INT Threshold = ( TurnAnimCount == 2 ) ? 0x2000 : 4096;
	INT PairIdx = 0;
	for( ; PairIdx < PairCount; ++PairIdx )
	{
		if( Abs( 0x8000 - AbsYawDif - PairIdx * 0x2000 ) < Threshold )
		{
			break;
		}
	}
	if( PairIdx == PairCount )
	{
		return FALSE;
	}
	_TurnAnimToPlayIdx = 2 * PairIdx + ( ( _YawDif > 0 ) ? 1 : 0 );
	return TRUE;
}

/*-----------------------------------------------------------------------------
	the turn
-----------------------------------------------------------------------------*/

UBOOL FArkComponentLocomotion::RequestTurn( INT _StartYaw, INT _EndYaw, FLOAT _fTurnSpeed, const FVector& _TurnLocation )
{
	if( !m_pAnimNodeLoco || !m_pConfig )
	{
		return FALSE;
	}
	const FAnimNodeLocoModifierSlot* pMostRelevantSlot = m_pAnimNodeLoco->GetMostRelevantModifierSlot();
	const INT ModifierIdx = pMostRelevantSlot ? pMostRelevantSlot->m_Modifier : m_CurModifierIdx;

	INT SpeedModeIdx0 = 0;
	INT SpeedModeIdx1 = 0;
	FLOAT fSpeedMode0Weight = 0.f;
	m_pConfig->GetSpeedModeFromForwardSpeed( _fTurnSpeed, ModifierIdx, SpeedModeIdx0, SpeedModeIdx1, fSpeedMode0Weight );

	const FArkCpntLocoSpeedModeProp& SpeedMode0 = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( SpeedModeIdx0 ) );
	const UBOOL bSpeedMode0IsStatic = Abs( SpeedMode0.m_MoveAnimProps( 0 ).m_fAnimSpeed ) < 0.0001f;
	const INT YawDif = DisLocoYawDelta( _StartYaw, _EndYaw );

	INT TurnAnimIndex = INDEX_NONE;
	INT BestSpeedModeIdx;
	if( fSpeedMode0Weight < 0.5f || ( bSpeedMode0IsStatic && Abs( _fTurnSpeed ) >= 0.1f ) || !IsSpeedModeCanDoTurn( SpeedModeIdx0, YawDif, TurnAnimIndex ) )
	{
		if( SpeedModeIdx1 == INDEX_NONE || ( fSpeedMode0Weight > 0.5f && ( !bSpeedMode0IsStatic || Abs( _fTurnSpeed ) < 0.1f ) )
			|| !IsSpeedModeCanDoTurn( SpeedModeIdx1, YawDif, TurnAnimIndex ) )
		{
			return FALSE;
		}
		BestSpeedModeIdx = SpeedModeIdx1;
	}
	else
	{
		BestSpeedModeIdx = SpeedModeIdx0;
	}
	if( BestSpeedModeIdx == INDEX_NONE )
	{
		return FALSE;
	}
	const FArkCpntLocoSpeedModeProp& SpeedMode = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( BestSpeedModeIdx ) );

	UBOOL bInMovement = FALSE;
	if( m_bHasRequestWithPathComputed )
	{
		FLOAT fScratch = 0.f;
		ComputeIsArrivedFlagAndSq2DDistToPathEnd( fScratch );
		bInMovement = !m_bIsArrived;
	}
	INT MovingTurnAnimIndex = TurnAnimIndex;
	if( SpeedMode.m_TurnAnimProps.Num() > 8 )
	{
		MovingTurnAnimIndex = TurnAnimIndex + 8;
	}
	if( bInMovement )
	{
		const FLOAT fStraightDist = Max( SpeedMode.m_TurnAnimProps( MovingTurnAnimIndex ).m_fStraightDist, 0.1f );
		const FLOAT fDX = m_CurPawnLocation.X - m_PathPoint.X;
		const FLOAT fDY = m_CurPawnLocation.Y - m_PathPoint.Y;
		if( ( fDY * fDY + fDX * fDX ) <= ( fStraightDist * fStraightDist ) )
		{
			bInMovement = FALSE;
		}
		else
		{
			TurnAnimIndex = MovingTurnAnimIndex;
		}
	}
	if( !m_pAnimNodeLoco->SkelComponent || !m_pAnimNodeLoco->SkelComponent->FindAnimSequence( SpeedMode.m_TurnAnimProps( TurnAnimIndex ).m_AnimName ) )
	{
		return FALSE;
	}

	m_TurnProps.m_Location = _TurnLocation;
	m_TurnProps.m_StartYaw = (INT)(WORD)( _StartYaw + 0x7FFF ) - 0x7FFF;
	m_TurnProps.m_EndYaw = (INT)(WORD)( _EndYaw + 0x7FFF ) - 0x7FFF;
	m_TurnProps.m_ModifierIdx = ModifierIdx;
	m_TurnProps.m_SpeedModeIdx = BestSpeedModeIdx;
	m_TurnProps.m_TurnAnimIdx = TurnAnimIndex;
	m_TurnProps.m_bPlayNow = bSpeedMode0IsStatic && fSpeedMode0Weight >= 0.5f;
	m_TurnProps.m_bInMovement = bInMovement;
	UBOOL bFastBlend = TRUE;
	if( bInMovement && ( BestSpeedModeIdx % m_pConfig->m_SpeedNames.Num() ) == 0 )
	{
		m_TurnProps.m_fTurnStartSpeed = 0.f;
		bFastBlend = ( TurnAnimIndex >= 8 );
	}
	else
	{
		m_TurnProps.m_fTurnStartSpeed = m_pConfig->GetForwardSpeedOfSpeedMode( BestSpeedModeIdx );
	}

	if( m_TurnProps.m_bPlayNow )
	{
		m_pAnimNodeLoco->RequestTurnAnim( m_TurnProps.m_SpeedModeIdx, m_TurnProps.m_TurnAnimIdx, TRUE, m_TurnProps.m_bInMovement, bFastBlend );
		SetRootState( CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM );
	}
	else
	{
		m_fBeforeTurnDuration = m_pAnimNodeLoco->GetDurationBeforePlayingTurnAnim( m_TurnProps.m_SpeedModeIdx, m_TurnProps.m_TurnAnimIdx );
		const FLOAT fDX = m_TurnProps.m_Location.X - m_pPawnOwner->Location.X;
		const FLOAT fDY = m_TurnProps.m_Location.Y - m_pPawnOwner->Location.Y;
		m_fBeforeTurnSquaredDist = fDY * fDY + fDX * fDX;
		SetRootState( CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT );
	}
	return TRUE;
}

// A turn on the spot: the pawn is slower than m_fMaxSpeedForStaticTurn and either has no path (it turns towards the face-to
// target) or its path's first stretch leaves at more than 90 degrees from the way it is moving.
void FArkComponentLocomotion::EvaluateStaticTurn()
{
	if( !IsReadyToTurn() || m_pConfig->m_fMaxSpeedForStaticTurn <= m_fRealScaledSpeed )
	{
		return;
	}
	const UBOOL bAvoidanceForce = DisLocoAffectorCount( m_pAvoidable ) > 0 && !DisLocoAvoidanceForce( m_pAvoidable ).IsZero();
	const UBOOL bNoPathToFollow = !m_ReqMgr.HasRequest() || m_CurReqWorkData.m_bDisabled || IsArrived();

	// DISHONORED(port, agent IO): retail turns towards the face-to component's target yaw when there is no path to follow, or
	// when the path's first stretch is within 90 degrees of it and the active face-to request is not the locomotion's.
	INT TargetYaw = m_pFaceToCpnt->GetTargetYaw();
	if( bNoPathToFollow )
	{
		if( bAvoidanceForce )
		{
			return;
		}
	}
	else
	{
		if( !m_bHasRequestWithPathComputed || m_TargetMoveYaw == -200000 )
		{
			return;
		}
		if( bAvoidanceForce )
		{
			const FVector MoveDir = FRotator( 0, m_TargetMoveYaw, 0 ).Vector();
			if( ( DisLocoAvoidanceForce( m_pAvoidable ) | MoveDir ) <= 0.f )
			{
				return;
			}
		}
		const FLOAT fMaxDistToPathEndToTurn = m_pConfig->m_fMaxDistToPathEndToTurn;
		if( GetSq2DDistToPathEnd() <= fMaxDistToPathEndToTurn * fMaxDistToPathEndToTurn )
		{
			return;
		}
		const INT FirstPathYaw = ComputePathYawFromDist( m_pConfig->m_fMaxDistToFirstPathTurn );
		if( Abs( DisLocoYawDelta( m_TargetMoveYaw, FirstPathYaw ) ) >= 0x4000 )
		{
			return;
		}
		if( IsLocoFaceToRequestActive() )
		{
			TargetYaw = m_TargetMoveYaw;
		}
		else if( Abs( DisLocoYawDelta( TargetYaw, FirstPathYaw ) ) >= 0x4000 )
		{
			return;
		}
	}

	const INT Delta = DisLocoYawDelta( m_pPawnOwner->Rotation.Yaw, TargetYaw );
	if( Abs( DisLocoYawToDegrees( Delta ) ) > 15.f )
	{
		RequestTurn( m_pPawnOwner->Rotation.Yaw, TargetYaw, 0.f, m_pPawnOwner->Location );
	}
}

// A turn at the start of a new path while already moving.
void FArkComponentLocomotion::EvaluateStartPathTurn()
{
	if( !m_bStartingNewPath || !IsReadyToTurn() || m_fRealScaledSpeed < m_pConfig->m_fMaxSpeedForStaticTurn
		|| !IsLocoFaceToRequestActive() || DisLocoAffectorCount( m_pAvoidable ) > 0 )
	{
		return;
	}
	const INT Delta = DisLocoYawDelta( m_pPawnOwner->Rotation.Yaw, m_CurPathYaw );
	if( Abs( DisLocoYawToDegrees( Delta ) ) > 15.f && IsTurnNotTooCloseFromPathEnd( 0 ) )
	{
		RequestTurn( m_pPawnOwner->Rotation.Yaw, m_CurPathYaw, m_fRealScaledSpeed, m_pPawnOwner->Location );
	}
}

// A turn at a corner of the path, requested while the face-to request this component owns is snapped on the path (agent IO:
// UpdateFaceTo now owns m_FaceToRequestID, and retail's IsActiveRequestSnapped test is back).
void FArkComponentLocomotion::EvaluatePathTurn()
{
	if( !m_bHasRequestWithPathComputed || m_FaceToRequestID == INDEX_NONE || !IsReadyToTurn()
		|| DisLocoAffectorCount( m_pAvoidable ) > 0 || !IsLocoFaceToRequestActive() || !m_pFaceToCpnt->IsActiveRequestSnapped() || m_fRealScaledSpeed <= 0.f )
	{
		return;
	}
	const INT Delta = DisLocoYawDelta( m_CurPathYaw, m_NextPathYaw );
	if( Abs( DisLocoYawToDegrees( Delta ) ) > 15.f && IsTurnNotTooCloseFromPathEnd( m_CurPathPointIdx ) )
	{
		RequestTurn( m_CurPathYaw, m_NextPathYaw, m_fRealScaledSpeed, m_PathPoints( m_CurPathPointIdx ).m_Point );
	}
}

void FArkComponentLocomotion::UpdateTurn( FLOAT _fTimeStep )
{
	if( !m_pAnimNodeLoco || !m_pConfig || !m_pPawnOwner )
	{
		return;
	}
	EvaluateStaticTurn();
	EvaluateStartPathTurn();
	EvaluatePathTurn();

	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT:
		if( m_bInertiaDisabled
			|| ( m_pAnimNodeLoco->IsSpecificAnimRequestedOrPlaying() && m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() != ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN )
			|| ( m_CurModifierIdx != m_TurnProps.m_ModifierIdx && !RequestTurn( m_TurnProps.m_StartYaw, m_TurnProps.m_EndYaw, m_TurnProps.m_fTurnStartSpeed, m_TurnProps.m_Location ) ) )
		{
			ResetTurn();
		}
		if( m_RootState == CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT )
		{
			const FArkCpntLocoSpeedModeProp& SpeedMode = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( m_TurnProps.m_SpeedModeIdx ) );
			const FLOAT fStraightDist = SpeedMode.m_TurnAnimProps( m_TurnProps.m_TurnAnimIdx ).m_fStraightDist;
			const FLOAT fDX = m_TurnProps.m_Location.X - m_pPawnOwner->Location.X;
			const FLOAT fDY = m_TurnProps.m_Location.Y - m_pPawnOwner->Location.Y;
			const FLOAT fSqDist = fDX * fDX + fDY * fDY;
			// The good foot is waited for while the pawn still closes on the turn point; once it has stopped closing (or is
			// within the turn's straight distance, or has not moved), the animation is requested as soon as the node's
			// duration-before-turn no longer shrinks.
			const UBOOL bNotClosingAnyMore = fSqDist > m_fBeforeTurnSquaredDist || ( fStraightDist * fStraightDist ) >= fSqDist
				|| ( fSqDist > ( m_fBeforeTurnSquaredDist - 1.f ) && ( m_fBeforeTurnSquaredDist + 1.f ) > fSqDist );
			const FLOAT fDurationBeforeTurn = m_pAnimNodeLoco->GetDurationBeforePlayingTurnAnim( m_TurnProps.m_SpeedModeIdx, m_TurnProps.m_TurnAnimIdx );
			const UBOOL bFootIsGood = fDurationBeforeTurn > m_fBeforeTurnDuration || _fTimeStep >= fDurationBeforeTurn;
			if( m_bStuckedInAWall || ( bNotClosingAnyMore && bFootIsGood ) )
			{
				m_pAnimNodeLoco->RequestTurnAnim( m_TurnProps.m_SpeedModeIdx, m_TurnProps.m_TurnAnimIdx, TRUE, m_TurnProps.m_bInMovement, TRUE );
				SetRootState( CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM );
			}
			else
			{
				m_fBeforeTurnDuration = fDurationBeforeTurn;
				m_fBeforeTurnSquaredDist = fSqDist;
			}
		}
		break;

	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM:
		if( m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_TURN_BEGIN );
			// The turn animation's root rotation is scaled from its authored angle (180, 135, 90, 45 degrees by pair) to the
			// swing actually wanted, and the mesh's root motion drives the actor's rotation while it plays.
			const FLOAT fRealAngle = Abs( DisLocoYawToDegrees( DisLocoYawDelta( m_TurnProps.m_StartYaw, m_TurnProps.m_EndYaw ) ) );
			INT TurnAnimIdx = m_TurnProps.m_TurnAnimIdx;
			if( TurnAnimIdx >= 8 )
			{
				TurnAnimIdx -= 8;
			}
			if( m_pPawnOwner->Mesh )
			{
				m_pPawnOwner->Mesh->RootRotationScale = fRealAngle / (FLOAT)( 45 * ( 4 - ( TurnAnimIdx >> 1 ) ) );
				m_pPawnOwner->Mesh->RootMotionMode = RMM_Accel;
				m_pPawnOwner->Mesh->RootMotionRotationMode = RMRM_RotateActor;
			}
		}
		else if( !m_pAnimNodeLoco->IsSpecificAnimRequested() || m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() != ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN )
		{
			ResetTurn();
		}
		break;

	case CPNT_LOCO_ROOT_STATE_TURN_BEGIN:
		if( m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() || !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_TURN );
		}
		break;

	case CPNT_LOCO_ROOT_STATE_TURN:
		if( !m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() )
		{
			if( m_TurnProps.m_bInMovement && m_pConfig->m_fVelocityToStopMoving > m_fRealScaledSpeed )
			{
				m_fRealScaledSpeed = m_pConfig->m_fVelocityToStartToMove;
			}
			SetRootState( CPNT_LOCO_ROOT_STATE_TURN_END );
		}
		break;

	case CPNT_LOCO_ROOT_STATE_TURN_END:
		if( !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			ResetTurn();
		}
		break;

	default:
		break;
	}
}

/*-----------------------------------------------------------------------------
	the start
-----------------------------------------------------------------------------*/

void FArkComponentLocomotion::UpdateStartMove( FLOAT _fTimeStep )
{
	if( !m_pAnimNodeLoco || !m_pConfig || !m_pPawnOwner )
	{
		return;
	}
	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_NONE:
		if( !m_bInertiaDisabled && m_bHasRequestWithPathComputed && m_pConfig->m_fVelocityToStopMoving > m_fRealScaledSpeed
			&& m_bCanManageAnim && !m_pAnimNodeLoco->IsSpecificAnimRequestedOrPlaying() && DisLocoAffectorCount( m_pAvoidable ) == 0 )
		{
			const FAnimNodeLocoModifierSlot* pMostRelevantSlot = m_pAnimNodeLoco->GetMostRelevantModifierSlot();
			const INT ModifierIdx = pMostRelevantSlot ? pMostRelevantSlot->m_Modifier : m_CurModifierIdx;
			if( ModifierIdx == m_CurModifierIdx )
			{
				const FArkCpntLocoSpeedModeProp& SpeedMode = GetSpeedMode( 0, ModifierIdx );
				const INT Delta = Abs( DisLocoYawDelta( m_pPawnOwner->Rotation.Yaw, m_CurPathYaw ) );
				if( SpeedMode.m_StartAnimProps.Num() > 0 && Delta < 4096 )
				{
					const FVector ToPathPoint = m_PathPoint - m_CurPawnLocation;
					const FLOAT fSq2D = ToPathPoint.Y * ToPathPoint.Y + ToPathPoint.X * ToPathPoint.X;
					const FLOAT fStraightDist = SpeedMode.m_StartAnimProps( 0 ).m_fStraightDist;
					if( fSq2D > ( fStraightDist * fStraightDist ) && ( m_CurPathDir | ToPathPoint ) > 0.f && !IsArrived() )
					{
						m_pAnimNodeLoco->RequestStartAnim( ModifierIdx * m_pConfig->m_SpeedNames.Num() );
						SetRootState( CPNT_LOCO_ROOT_STATE_START_WAIT_ANIM );
					}
				}
			}
		}
		break;

	case CPNT_LOCO_ROOT_STATE_START_WAIT_ANIM:
		if( m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_START_BEGIN );
		}
		else if( !m_pAnimNodeLoco->IsSpecificAnimRequested() || m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() != ANIM_NODE_LOCO_SPECIFIC_ANIM_START )
		{
			ResetStartMove();
		}
		break;

	case CPNT_LOCO_ROOT_STATE_START_BEGIN:
		if( m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() || !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_START );
		}
		break;

	case CPNT_LOCO_ROOT_STATE_START:
		if( !m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_START_END );
		}
		break;

	case CPNT_LOCO_ROOT_STATE_START_END:
		if( !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			ResetStartMove();
		}
		break;

	default:
		break;
	}
}

/*-----------------------------------------------------------------------------
	the stop
-----------------------------------------------------------------------------*/

void FArkComponentLocomotion::UpdateStopMove( FLOAT _fTimeStep )
{
	if( !m_pAnimNodeLoco || !m_pConfig || !m_pPawnOwner )
	{
		return;
	}
	// m_bHandleStop: the pawn is, or is about to be, landed by a stop animation rather than by the path's braking rule.
	// m_bHaveStopAnim has no writer in either database (the 2012 constructor and every function of the component were
	// searched), so the first clause never raises it; the stop states hold it.
	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_NONE:
	case CPNT_LOCO_ROOT_STATE_TURN_END:
	case CPNT_LOCO_ROOT_STATE_STOP_APPROACH:
		m_bHandleStop = FALSE;
		if( !m_bInertiaDisabled && m_bHasRequestWithPathComputed && m_bCanManageAnim && m_bHaveStopAnim
			&& m_ReqMgr.HasRequest() && !m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_bAccurateStop && !IsArrived() )
		{
			m_bHandleStop = Abs( DisLocoYawDelta( m_pPawnOwner->Rotation.Yaw, m_CurPathYaw ) ) < 4096;
		}
		break;
	case CPNT_LOCO_ROOT_STATE_STOP_WAIT_ANIM:
	case CPNT_LOCO_ROOT_STATE_STOP_BEGIN:
	case CPNT_LOCO_ROOT_STATE_STOP:
	case CPNT_LOCO_ROOT_STATE_STOP_END:
		m_bHandleStop = TRUE;
		break;
	default:
		m_bHandleStop = FALSE;
		break;
	}

	FLOAT fPrevProgressAntedate = 0.f;
	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_NONE:
	{
		const FLOAT fUnaccuracyRadius = m_pConfig->m_fUnaccuracyRadius;
		if( !m_bHandleStop || ( fUnaccuracyRadius * fUnaccuracyRadius ) <= GetSq2DDistToPathEnd() )
		{
			return;
		}
		fPrevProgressAntedate = 0.03f;
		SetRootState( CPNT_LOCO_ROOT_STATE_STOP_APPROACH );
	}
	// falls through: the approach is evaluated on the frame it is entered
	case CPNT_LOCO_ROOT_STATE_STOP_APPROACH:
	{
		const FLOAT fUnaccuracyRadius = m_pConfig->m_fUnaccuracyRadius;
		if( !m_bHandleStop || GetSq2DDistToPathEnd() > ( fUnaccuracyRadius * fUnaccuracyRadius ) || m_pAnimNodeLoco->IsSpecificAnimRequestedOrPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_NONE );
			return;
		}
		FLOAT fPrevProgress = 0.f;
		FLOAT fCurProgress = 0.f;
		m_pAnimNodeLoco->GetMovementAnimPrevAndCurProgress( fPrevProgress, fCurProgress );
		const INT SpeedModeIdx = m_CurModifierIdx * m_pConfig->m_SpeedNames.Num();
		const FLOAT fToPathEndDist = appSqrt( GetSq2DDistToPathEnd() );
		const FArkCpntLocoSpeedModeProp& StaticSpeedMode = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( SpeedModeIdx ) );
		const INT FeetCount = StaticSpeedMode.m_StopAnimProps.Num();
		// One stop animation per foot: the one whose move-animation phase the walk cycle has just passed (prev <= phase <=
		// cur, with the wrap at 1 handled, and the previous progress antedated by 0.03 s on the frame the approach begins).
		for( INT FootIdx = 0; FootIdx < FeetCount; ++FootIdx )
		{
			const FArkCpntLocoStopAnimProp& StopAnim = StaticSpeedMode.m_StopAnimProps( FootIdx );
			const FLOAT fMoveAnimIn = StopAnim.m_fMoveAnimIn;
			const FLOAT fPrevAntedated = appFmod( fPrevProgress + 1.f - fPrevProgressAntedate, 1.f );
			UBOOL bPhasePassed;
			if( fPrevAntedated <= fCurProgress )
			{
				bPhasePassed = fMoveAnimIn >= fPrevAntedated && fCurProgress >= fMoveAnimIn;
			}
			else
			{
				bPhasePassed = fMoveAnimIn >= fPrevAntedated || fCurProgress >= fMoveAnimIn;
			}
			if( !bPhasePassed )
			{
				continue;
			}
			// The stop is requested when one more step of the walk would overshoot the mark by more than stopping now
			// undershoots it: |dist - stopStraight - stepLength| > |dist - stopStraight|, the step being the walk
			// animation's length times its foot speed over the feet count.
			const FLOAT fStopStraightDist = StopAnim.m_fStraightDist;
			const FArkCpntLocoSpeedModeProp& WalkSpeedMode = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( SpeedModeIdx + 1 ) );
			const FArkCpntLocoMoveAnimProp& WalkAnim = WalkSpeedMode.m_MoveAnimProps( 4 / ( 8 / WalkSpeedMode.m_MoveAnimProps.Num() ) );
			const UAnimSequence* pWalkSequence = m_pPawnOwner->Mesh ? m_pPawnOwner->Mesh->FindAnimSequence( WalkAnim.m_AnimName ) : NULL;
			if( pWalkSequence )
			{
				const FLOAT fStepLength = pWalkSequence->SequenceLength * WalkAnim.m_fAnimSpeed / (FLOAT)FeetCount;
				if( Abs( fToPathEndDist - fStopStraightDist - fStepLength ) > Abs( fToPathEndDist - fStopStraightDist ) )
				{
					m_pAnimNodeLoco->RequestStopAnim( SpeedModeIdx, FootIdx );
					SetRootState( CPNT_LOCO_ROOT_STATE_STOP_WAIT_ANIM );
				}
			}
			return;
		}
		return;
	}

	case CPNT_LOCO_ROOT_STATE_STOP_WAIT_ANIM:
		if( m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_STOP_BEGIN );
		}
		else if( !m_pAnimNodeLoco->IsSpecificAnimRequested() || m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() != ANIM_NODE_LOCO_SPECIFIC_ANIM_STOP )
		{
			ResetStopMove();
		}
		return;

	case CPNT_LOCO_ROOT_STATE_STOP_BEGIN:
		if( m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() || !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_STOP );
		}
		return;

	case CPNT_LOCO_ROOT_STATE_STOP:
		if( !m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() )
		{
			SetRootState( CPNT_LOCO_ROOT_STATE_STOP_END );
		}
		return;

	case CPNT_LOCO_ROOT_STATE_STOP_END:
		if( !m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		{
			ResetStopMove();
			m_fCurMoveSpeed = 0.f;
		}
		return;

	default:
		return;
	}
}
