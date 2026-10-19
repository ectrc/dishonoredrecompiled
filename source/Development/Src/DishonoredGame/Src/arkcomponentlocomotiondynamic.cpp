// DishonoredGame/src/arkcomponentlocomotiondynamic.cpp
// DISHONORED(port): agent DN. The dynamics: how fast, which way, and then the translation itself. Retail's unit is
// Engine/Src/arkcomponentlocomotiondynamic.cpp (42 functions, 25,685 bytes, the largest of the six).
//
// What is faithful here and what is bring-up, stated once rather than per function:
//  * faithful ports (transcribed from the decompile, rva at each site): UpdateMoveSpeed, ComputeTargetMoveSpeed's
//    braking-distance rule, ComputeIsArrivedFlagAndSq2DDistToPathEnd, IsThereAnotherForceThanPath, IsArrived,
//    AddSteeringForce, ApplySteering's inertia blend, MovePawn's branch between the moderate and the full move,
//    ModerateMovePawn, HandleArrival's event bookkeeping, ComputeMoveYaw, UpdateSharedProperties.
//  * bring-up reconstructions, each named with its retail rva: ComputeDynamic (retail 3,026 bytes - the call ORDER below
//    is retail's, taken from the decompile, and the parts it calls that need the unported animation/avoidance systems are
//    left out), ComputeTargetMoveYaw and UpdateMoveYaw (retail 1,790 + 1,739 bytes of multi-directional speed blending
//    against the anim node's strafe set), FullMovePawn (retail 4,707 bytes of hand-rolled swept movement; this uses
//    UWorld::MoveActor, which is the engine's own swept move and gives step-up, wall sliding and touch notifications).
//  * not ported at all, named with rvas where they would be called: ComputeDynamic's avoidance half
//    (DetectPreAvoidanceCollision 0x543f70, HandlePushPeriod 0x548b90, ComputePriorityOfNPCsFromSharedProps 0x5491b0)
//    and the follow mode (arkcomponentlocomotionfollow.cpp: UpdateFollow 0x544da0, ComputeFollowLocation 0x5448a0,
//    InitFollowProperties 0x544780). They drive FArkComponentAvoidance, which is not ported.
//  * the turn-in-place / start / stop animation system (arkcomponentlocomotionrootmove.cpp, 15 functions) is ported by
//    agent GK and called from ComputeDynamic and ComputeTargetMoveSpeed where retail calls it.
// The consequence, stated plainly: the NPC walks the path at the right speed and faces where it is going, but it does not
// give way to other NPCs.
// Agent IB (PHASE23 IB): the velocity source is retail's - ComputeDynamic writes the pawn's Velocity/Acceleration from the
// move it decided (2012 0x58ca10 / 2013 VA 0x94bec0 tail) and FullMovePawn moves Velocity x its own time step, then clamps the
// measured velocity to the asked speed (2012 0x580ae0 / 2013 VA 0x93fb00). HEAD moved m_CurMove (a distance for the dynamics'
// time step) in the next frame's physWalking and re-derived Velocity from it, so any frame-time change between the
// manager's PreAsyncWork tick and physWalking showed as a speed dip (measured: Worker B's sampled speed down to 72 u/s).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"
#include "arkcomponentfaceto.h"
#include "ibdiag.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "disdesirestructs.h"

/*-----------------------------------------------------------------------------
	small faithful ports
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x53dfe0 (2012 0x57f130): forces accumulate within a frame; ApplySteering consumes them.
void FArkComponentLocomotion::AddSteeringForce( const FVector& _Force )
{
	if( !m_bSteeringDisabled )
	{
		m_CurSteeringForce += _Force;
	}
}

// DISHONORED(port): 2013 rva 0x53ed30 (2012 0x57ff70): root motion means the animation owns the translation, so the
// dynamics must not also move the pawn.
// DISHONORED(port, agent GK): retail's body (2013 rva 0x53ed30 / 2012 0x57ff70, 60 B) - "the animation owns the pawn": the
// component is disabled, the pawn is being deleted, the root state is STOP_END, or the locomotion node plays only a specific
// animation. Agent DN's version read the mesh's RootMotionMode, which is RMM_Velocity on every NPC.
UBOOL FArkComponentLocomotion::IsRootMotion() const
{
	return m_bDisabled || ( m_pPawnOwner && m_pPawnOwner->bDeleteMe ) || m_RootState == CPNT_LOCO_ROOT_STATE_STOP_END
		|| ( m_bCanManageAnim && m_pAnimNodeLoco && m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() );
}

// DISHONORED(port, agent IK): 2013 VA 0x93e1f0 (2012 0x57f340, 35 B): no yaw while the move speed is under 0.1, else the
// current move yaw (HEAD answered the path's yaw, so a pawn without a path never turned towards anything).
INT FArkComponentLocomotion::ComputeMoveYaw() const
{
	return ( Abs( m_fCurMoveSpeed ) < 0.1f ) ? -200000 : m_CurMoveYaw;
}

// DISHONORED(port): 2013 rva 0x541a00 (2012 0x5829d0): is anything other than the path asking the pawn to move? Steering,
// a push period or root motion all count, and each of them means ClampMove must not simply clamp to the path.
// DISHONORED(port, agent GK): retail's body (2013 rva 0x541a00 / 2012 0x5829d0, 249 B): a steering force, or - unless this
// component's shared-props row says to ignore avoidance - an avoidance force of 0.05 or more. Agent DN's version answered
// IsRootMotion() as a third clause, which is TRUE for every NPC whose mesh runs RMM_Velocity (GA: `rmm 1/1` on all of them),
// so the push-speed clamp in ComputeDynamic held every lone pawn's move to one acceleration step above its real speed - the
// "moves zeroed by the push clamp" crawl GD2 measured and papered over with its two landing stand-ins.
UBOOL FArkComponentLocomotion::IsThereAnotherForceThanPath() const
{
	const UBOOL bSteering = Abs( m_CurSteeringForce.X ) >= 0.0001f || Abs( m_CurSteeringForce.Y ) >= 0.0001f || Abs( m_CurSteeringForce.Z ) >= 0.0001f;
	UBOOL bAvoidance = FALSE;
	if( m_LocoCpntIdx >= 0 && m_LocoCpntIdx < ms_LocoCpntSharedProps.Num() && !ms_LocoCpntSharedProps( m_LocoCpntIdx ).m_bMustIgnoreAvoidance && m_pAvoidable )
	{
		const FVector& Force = m_pAvoidable->m_Force;
		bAvoidance = Abs( Force.X ) >= 0.05f || Abs( Force.Y ) >= 0.05f || Abs( Force.Z ) >= 0.05f;
	}
	return bSteering || bAvoidance;
}

// DISHONORED(port): 2013 rva 0x544390 (2012 0x585330, 249 bytes), agent GD. Retail's rule, read off both decompiles: with no
// computed path the pawn counts as arrived (the dynamics then have nothing to do); otherwise the squared 2D distance is
// measured from m_CurPawnLocation - the position the pawn WILL have once this sub-step's path move is applied, which
// ComputeDynamic keeps up to date - to the last path point. An accurate-stop request, or any request on a pawn with no stop
// animation, has arrived when that distance is under 0.1 uu (0.01 squared); a request that leaves the stop to the animation
// has arrived once the pawn is inside the config's unaccuracy radius and slower than m_fVelocityToStopMoving, and only in
// the NONE / state-5 root states. Agent DN's version measured from the pawn's current location against the request's end
// threshold with the collision radius as the fallback - that is what left NPCPawn_25 "arrived" 37 uu short (agentGD.md).
UBOOL FArkComponentLocomotion::ComputeIsArrivedFlagAndSq2DDistToPathEnd( FLOAT& _rSq2DDist ) const
{
	if( !m_bHasRequestWithPathComputed || !m_ReqMgr.HasRequest() )
	{
		_rSq2DDist = 0.f;
		return TRUE;
	}
	const FVector ToEnd = m_LastPathPoint - m_CurPawnLocation;
	_rSq2DDist = ToEnd.X * ToEnd.X + ToEnd.Y * ToEnd.Y;

	if( m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_bAccurateStop || !m_bHaveStopAnim )
	{
		return _rSq2DDist < 0.01f;
	}
	if( ( m_RootState == CPNT_LOCO_ROOT_STATE_NONE || m_RootState == 5 ) && m_pConfig )
	{
		return ( m_pConfig->m_fUnaccuracyRadius * m_pConfig->m_fUnaccuracyRadius ) > _rSq2DDist
			&& m_pConfig->m_fVelocityToStopMoving > Abs( m_fRealScaledSpeed );
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x546260 (2012 0x586f60): the component's row in the static shared table, which is what the
// crowd rule would read.
void FArkComponentLocomotion::UpdateSharedProperties()
{
	if( m_LocoCpntIdx < 0 || m_LocoCpntIdx >= ms_LocoCpntSharedProps.Num() )
	{
		return;
	}
	FLocoCpntSharedProps& rRow = ms_LocoCpntSharedProps( m_LocoCpntIdx );
	rRow.m_pLocoCpnt = this;
	rRow.m_FinalLocation = m_LastPathPoint;
	rRow.m_bCloseToFinalLocation = m_bIsArrived;
	rRow.m_bMustIgnoreAvoidance = m_bInPushPeriod;
}

/*-----------------------------------------------------------------------------
	speed
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x548050 (2012 0x588e00, 957 bytes). Retail's rule, and the reason an NPC arrives smoothly
// rather than stopping dead: the braking distance from the request's max speed down to its end speed is computed from the
// current deceleration, and while the remaining distance exceeds it the pawn holds max speed; inside it the target speed
// is interpolated linearly between end and max by the fraction of the braking distance left, with the acceleration
// adapted so the interpolation is actually achievable. A turn of more than 90 degrees clamps the target to the walk speed.
FLOAT FArkComponentLocomotion::ComputeTargetMoveSpeed( FLOAT _fTimeStep, FLOAT& _rAdaptedAccel ) const
{
	_rAdaptedAccel = m_fCurDeceleration * m_fCurFollowAccelMultiplier;
	if( !m_bHasRequestWithPathComputed || !m_ReqMgr.HasRequest() )
	{
		// a pawn walking back onto the navigation mesh does it at walking pace
		return ( m_bHasRequestWithPathComputed || !m_bTryToReturnOnNavMesh ) ? 0.f : GetForwardSpeedOfSpeedMode( 1, m_CurModifierIdx );
	}
	// DISHONORED(port, agent GK): retail's switch on the root state - a turn waiting for its animation holds the turn's
	// start speed, a stop approaching its animation holds the walk speed, the states where the animation owns the pawn
	// answer 0, and only NONE / TURN_END / START_END run the braking rule below.
	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_NONE:
	case CPNT_LOCO_ROOT_STATE_TURN_END:
	case CPNT_LOCO_ROOT_STATE_START_END:
		break;
	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT:
	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM:
	case CPNT_LOCO_ROOT_STATE_TURN_BEGIN:
		return m_TurnProps.m_fTurnStartSpeed;
	case CPNT_LOCO_ROOT_STATE_STOP_APPROACH:
	case CPNT_LOCO_ROOT_STATE_STOP_WAIT_ANIM:
	case CPNT_LOCO_ROOT_STATE_STOP_BEGIN:
		return GetForwardSpeedOfSpeedMode( 1, m_CurModifierIdx );
	default:
		return 0.f;
	}
	if( m_bIsArrived )
	{
		return 0.f;
	}

	const FArkCpntLocoRequestCommonProps& rCommon = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps;
	INT EndSpeedIdx = rCommon.m_EndSpeedIdx;
	if( m_bHandleStop && EndSpeedIdx < 1 )
	{
		EndSpeedIdx = 1;
	}
	const FLOAT fSpeedMultiplier = rCommon.m_fSpeedMultiplier;
	const FLOAT fMaxSpeed  = Max( GetForwardSpeedOfSpeedMode( rCommon.m_MaxSpeedIdx, m_CurModifierIdx ) * fSpeedMultiplier, 0.f );
	const FLOAT fEndSpeed  = Max( GetForwardSpeedOfSpeedMode( EndSpeedIdx, m_CurModifierIdx ) * fSpeedMultiplier, 0.f );
	const FLOAT fWalkSpeed = Max( GetForwardSpeedOfSpeedMode( 1, m_CurModifierIdx ) * fSpeedMultiplier, 0.f );

	FLOAT fBrakingDist = 0.f;
	if( fMaxSpeed != fEndSpeed && m_fCurDeceleration != 0.f )
	{
		fBrakingDist = Abs( ( fMaxSpeed + ( fEndSpeed - fMaxSpeed ) * 0.5f ) * ( ( fEndSpeed - fMaxSpeed ) / m_fCurDeceleration ) );
	}

	FLOAT fDistLeft = appSqrt( m_fSq2DDistToPathEnd );
	if( m_bHandleStop && m_pConfig )
	{
		fDistLeft = Max( fDistLeft - m_pConfig->m_fUnaccuracyRadius, 0.f );
	}

	FLOAT fTargetSpeed;
	if( fBrakingDist < fDistLeft )
	{
		fTargetSpeed = ( m_fMaxMoveRotSpeedMultiplier != 0.f ) ? ( fMaxSpeed / m_fMaxMoveRotSpeedMultiplier ) : fMaxSpeed;
		_rAdaptedAccel = ( fTargetSpeed <= m_fCurMoveSpeed )
			? ( m_fMaxMoveRotSpeedMultiplier * m_fCurFollowAccelMultiplier ) * _rAdaptedAccel
			: m_fCurAcceleration * m_fCurFollowAccelMultiplier;
	}
	else if( Abs( fBrakingDist ) >= 0.1f )
	{
		fTargetSpeed = ( fDistLeft / fBrakingDist ) * ( fMaxSpeed - fEndSpeed ) + fEndSpeed;
		if( fTargetSpeed > m_fCurMoveSpeed )
		{
			_rAdaptedAccel = m_fCurAcceleration * m_fCurFollowAccelMultiplier;
		}
		else if( m_fCurMoveSpeed != fTargetSpeed && fDistLeft != 0.f )
		{
			const FLOAT fMean = m_fCurMoveSpeed + ( fTargetSpeed - m_fCurMoveSpeed ) * 0.5f;
			if( fMean != 0.f )
			{
				_rAdaptedAccel = Abs( ( fTargetSpeed - m_fCurMoveSpeed ) / ( fDistLeft / fMean ) );
			}
		}
	}
	else
	{
		fTargetSpeed = fEndSpeed;
	}

	// A sharp turn is taken at walking pace: retail measures the shortest signed yaw delta and compares its degrees
	// against 90 (the 0.0054931641 in the decompile is 360/65536).
	const INT YawDelta = Abs( (INT)(SWORD)( m_TargetMoveYaw - m_CurMoveYaw ) );
	if( ( (FLOAT)YawDelta * 0.0054931641f ) > 90.f && fTargetSpeed >= fWalkSpeed )
	{
		fTargetSpeed = fWalkSpeed;
	}
	return fTargetSpeed;
}

// DISHONORED(port): 2013 rva 0x53e220 (2012 0x57f370). Faithful: inertia off answers the target at once, a difference
// under 0.1 answers the current speed, and otherwise one acceleration step is taken towards the target without
// overshooting it.
FLOAT FArkComponentLocomotion::UpdateMoveSpeed( FLOAT _fTimeStep ) const
{
	if( m_bInertiaDisabled )
	{
		return m_fTargetMoveSpeed;
	}
	if( Abs( m_fCurMoveSpeed - m_fTargetMoveSpeed ) < 0.1f )
	{
		return m_fCurMoveSpeed;
	}
	const FLOAT fAccelStep = m_fCurAdaptedAccel * _fTimeStep;
	const FLOAT fSpeedDif = m_fTargetMoveSpeed - m_fCurMoveSpeed;
	if( fAccelStep >= Abs( fSpeedDif ) )
	{
		return m_fTargetMoveSpeed;
	}
	return ( fSpeedDif <= 0.f ) ? ( m_fCurMoveSpeed - fAccelStep ) : ( m_fCurMoveSpeed + fAccelStep );
}

/*-----------------------------------------------------------------------------
	yaw
-----------------------------------------------------------------------------*/

// DISHONORED(port, agent IK): 2013 VA 0x947950 (2012 0x588700, 1,766 / 1,790 B). By root state: NONE and the START states aim
// at the current path point while a path is computed and the pawn has not arrived, else at m_NearestLocationOnNavMesh while
// the pawn walks back onto the mesh, else keep the current yaw; the turn states answer the turn's start / end yaw.
// DISHONORED(bringup): the avoidance half of the path branch is not ported - beyond m_fAvoidanceUnstuckMinDist of the path
// end and with a move-hit normal or one avoidance affector, retail swings the yaw by +-m_fAvoidanceAngle on the side
// UNavigationHandle::PointReachableZeroExtent / DetectPreAvoidanceCollision pick (decompile build/agentIK/decomp2012/588700_*).
INT FArkComponentLocomotion::ComputeTargetMoveYaw() const
{
	switch( m_RootState )
	{
	case CPNT_LOCO_ROOT_STATE_NONE:
	case CPNT_LOCO_ROOT_STATE_START_WAIT_ANIM:
	case CPNT_LOCO_ROOT_STATE_START_BEGIN:
	case CPNT_LOCO_ROOT_STATE_START:
	case CPNT_LOCO_ROOT_STATE_START_END:
		if( m_bHasRequestWithPathComputed && !m_bIsArrived )
		{
			return appTrunc( appAtan2( m_PathPoint.Y - m_CurPawnLocation.Y, m_PathPoint.X - m_CurPawnLocation.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
		}
		if( m_bTryToReturnOnNavMesh )
		{
			return appTrunc( appAtan2( m_NearestLocationOnNavMesh.Y - m_CurPawnLocation.Y, m_NearestLocationOnNavMesh.X - m_CurPawnLocation.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
		}
		return m_CurMoveYaw;
	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT:
	case CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM:
	case CPNT_LOCO_ROOT_STATE_TURN_BEGIN:
		return m_TurnProps.m_StartYaw;
	case CPNT_LOCO_ROOT_STATE_TURN:
	case CPNT_LOCO_ROOT_STATE_TURN_END:
		return m_TurnProps.m_EndYaw;
	default:
		return m_CurMoveYaw;
	}
}

// DISHONORED(port): 2013 rva 0x541330 (2012 0x582300, 1,739 bytes).
// DISHONORED(bringup): retail accelerates the rotation speed towards the speed mode's maximum and reports back the
// rotation speed, that maximum and the multi-directional speed limit for the resulting angle (the three out parameters).
// This turns towards the target at the speed mode's maximum rotation speed without the acceleration curve, and reports
// the same three values so the callers above are unchanged.
INT FArkComponentLocomotion::UpdateMoveYaw( FLOAT _fTimeStep, FLOAT& _rCurRotSpeed, FLOAT& _rMaxRotSpeed, FLOAT& _rMaxMultiDirSpeedForAngle ) const
{
	FLOAT fMaxRotationSpeed = 90.f;
	if( m_pConfig && m_ReqMgr.HasRequest() )
	{
		const INT SpeedIdx = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_MaxSpeedIdx;
		if( SpeedIdx >= 0 && SpeedIdx < m_pConfig->m_SpeedNames.Num() )
		{
			fMaxRotationSpeed = GetSpeedMode( SpeedIdx, m_CurModifierIdx ).m_fRotationMaxSpeed;
		}
	}
	if( fMaxRotationSpeed <= 0.f )
	{
		fMaxRotationSpeed = 90.f;
	}
	_rMaxRotSpeed = fMaxRotationSpeed;
	_rCurRotSpeed = fMaxRotationSpeed;
	_rMaxMultiDirSpeedForAngle = 0.f;

	if( m_CurMoveYaw == -200000 || m_TargetMoveYaw == -200000 )
	{
		return m_TargetMoveYaw;
	}
	// 65536 units of yaw is 360 degrees, so degrees per second becomes units per second by * 65536/360.
	const INT MaxStep = Max( 1, appTrunc( fMaxRotationSpeed * _fTimeStep * ( 65536.f / 360.f ) ) );
	const INT Delta = (INT)(SWORD)( m_TargetMoveYaw - m_CurMoveYaw );
	if( Abs( Delta ) <= MaxStep )
	{
		return m_TargetMoveYaw;
	}
	return ( m_CurMoveYaw + ( Delta > 0 ? MaxStep : -MaxStep ) ) & 0xFFFF;
}

/*-----------------------------------------------------------------------------
	the move vector
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x548410 (2012 0x5891c0, 913 bytes). The frame's path move is the move yaw's direction times
// the current speed times the time step; within m_fSecurityRadiusAroundPathPoint of the current corner retail blends that
// direction towards the direction to the corner itself, weighted by how close the pawn is, which is what stops an NPC
// cutting a corner it is about to turn at.
FVector FArkComponentLocomotion::ComputePathMove( FLOAT _fTimeStep )
{
	if( m_CurMoveYaw == -200000 )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	FRotator MoveRotator( 0, m_CurMoveYaw, 0 );
	FVector MoveDir = MoveRotator.Vector();
	MoveDir.Z = 0.f;
	MoveDir = MoveDir.SafeNormal();

	if( m_bHasRequestWithPathComputed && !m_bPathAtEnd && !m_bHandleStop && !IsArrived()
		&& m_pConfig && m_CurPathPointIdx >= 0 && m_CurPathPointIdx < m_PathPoints.Num() )
	{
		FLOAT fSecurityRadius = m_pConfig->m_fSecurityRadiusAroundPathPoint;
		FVector PawnToPathPoint = m_PathPoints( m_CurPathPointIdx ).m_Point - m_CurPawnLocation;
		const FLOAT fSq2D = PawnToPathPoint.X * PawnToPathPoint.X + PawnToPathPoint.Y * PawnToPathPoint.Y;
		if( ( m_fDistBetweenPathPoints * m_fDistBetweenPathPoints ) > fSq2D && fSecurityRadius > m_fDistBetweenPathPoints )
		{
			fSecurityRadius = m_fDistBetweenPathPoints;
		}
		if( fSecurityRadius > 0.f && ( fSecurityRadius * fSecurityRadius ) > fSq2D )
		{
			const FLOAT fInfluence = appSqrt( fSq2D ) / fSecurityRadius;
			PawnToPathPoint.Z = 0.f;
			const FVector ToCorner = PawnToPathPoint.SafeNormal();
			MoveDir = ( MoveDir * fInfluence + ToCorner * ( 1.f - fInfluence ) ).SafeNormal();
		}
	}
	return MoveDir * ( m_fCurMoveSpeed * _fTimeStep );
}

// DISHONORED(port): 2013 rva 0x54a630 (2012 0x58b110, 1,044 bytes). The steering force of this frame is blended into the
// path move with the previous frame's force, so an avoidance push decays rather than snapping; the result is clamped so
// steering can never make the pawn faster than the path would.
FVector FArkComponentLocomotion::ApplySteering( FLOAT _fTimeStep, const FVector& _PathMove )
{
	if( m_bSteeringDisabled || m_CurSteeringForce.IsZero() )
	{
		m_PreviousSteeringForce = FVector( 0.f, 0.f, 0.f );
		m_PreviousPathForce = _PathMove;
		m_CurSteeringForce = FVector( 0.f, 0.f, 0.f );
		return _PathMove;
	}
	FVector Steering = m_CurSteeringForce * ( m_fSteeringMultiplier * _fTimeStep );
	Steering.Z = 0.f;
	const FLOAT fPathSize = _PathMove.Size2D();
	FVector Move = _PathMove + ( Steering + m_PreviousSteeringForce ) * 0.5f;
	Move.Z = _PathMove.Z;
	const FLOAT fMoveSize = Move.Size2D();
	if( fPathSize > 0.f && fMoveSize > fPathSize )
	{
		Move *= ( fPathSize / fMoveSize );
	}
	m_PreviousSteeringForce = Steering;
	m_PreviousPathForce = _PathMove;
	m_CurSteeringForce = FVector( 0.f, 0.f, 0.f );
	return Move;
}

// DISHONORED(port): 2013 rva 0x544490 (2012 0x585430, 752 bytes). The move is clamped so the pawn cannot overshoot the
// last path point in one frame, and the maximum upward step it is allowed this frame is reported back.
FVector FArkComponentLocomotion::ClampMove( FLOAT _fTimeStep, const FVector& _Move, FLOAT& _rMaxMoveUpDist ) const
{
	_rMaxMoveUpDist = m_pPawnOwner ? m_pPawnOwner->MaxStepHeight : 0.f;
	if( !m_bHasRequestWithPathComputed || m_PathPoints.Num() < 2 )
	{
		return _Move;
	}
	FVector ToEnd = m_LastPathPoint - m_CurPawnLocation;
	ToEnd.Z = 0.f;
	const FLOAT fDistToEnd = ToEnd.Size();
	const FLOAT fMoveSize = _Move.Size2D();
	if( fDistToEnd > 0.f && fMoveSize > fDistToEnd )
	{
		return _Move * ( fDistToEnd / fMoveSize );
	}
	return _Move;
}

// DISHONORED(port): 2013 rva 0x5487b0 (2012 0x589560, 977 bytes), agent GD, transcribed whole. Two jobs. First the stop on the
// mark: when the pawn is on the last segment (m_bPathAtEnd), is not yet arrived and the stop is not the animation's, a
// sub-step whose path move would carry it past the last path point is clamped to land exactly on it, the speed becomes
// the request's end speed and the arrival flag is recomputed - this is how an accurate-stop pawn ends 0.0 uu from its
// mark rather than oscillating around it. Second the events: with a computed path, an arrived (or just-clamped) pawn
// tells its asker THRESHOLD_REACHED then DESTINATION_REACHED, each once; a pawn that is not arrived re-arms the
// destination event and re-arms the threshold event once it is outside the request's threshold again, so a target that
// walks away can be reached a second time. Both fire only while the path is not dirty. Agent DN's version set the arrived
// flag here from the pawn's current location and never re-armed either event.
void FArkComponentLocomotion::HandleArrival( FLOAT _fTimeStep )
{
	FLOAT fScratch = 0.f;
	ComputeIsArrivedFlagAndSq2DDistToPathEnd( fScratch );
	const UBOOL bArrived = m_bIsArrived;
	UBOOL bClampedOnEnd = FALSE;

	if( !bArrived && !m_bHandleStop && m_bPathAtEnd && m_pPawnOwner && m_ReqMgr.HasRequest() )
	{
		const FVector ToEnd = m_LastPathPoint - m_pPawnOwner->Location;
		const FLOAT fSqToEnd = ToEnd.X * ToEnd.X + ToEnd.Y * ToEnd.Y;
		if( ( m_CurPathMove.X * m_CurPathMove.X + m_CurPathMove.Y * m_CurPathMove.Y ) > fSqToEnd )
		{
			const FArkCpntLocoRequestCommonProps& rCommon = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps;
			// DISHONORED(bringup): UArkComponentLocomotionConfig::ComputeSpeedFromSpeedModeAndAngle( EndSpeedIdx, modifier,
			// FArkComponentFaceTo::GetAngle() ) - with no face-to component the angle is 0 and the answer is the forward speed.
			m_fCurMoveSpeed = GetForwardSpeedOfSpeedMode( rCommon.m_EndSpeedIdx, m_CurModifierIdx );
			m_CurPathMove.X = ToEnd.X;
			m_CurPathMove.Y = ToEnd.Y;
			m_CurPawnLocation = m_pPawnOwner->Location + m_CurPathMove;
			m_bIsArrived = ComputeIsArrivedFlagAndSq2DDistToPathEnd( m_fSq2DDistToPathEnd );
			bClampedOnEnd = TRUE;
		}
	}

	if( !m_bHasRequestWithPathComputed || !m_ReqMgr.HasRequest() )
	{
		return;
	}
	const FArkRequestManager<FLocoRequestData>::FRequest& rRequest = m_ReqMgr.GetFirstRequest();
	const FArkCpntLocoRequestCommonProps& rCommon = rRequest.m_Data.m_CommonProps;
	if( bArrived || bClampedOnEnd )
	{
		if( rCommon.m_LocoEventCallback && !m_bPathIsDirty && !m_CurReqWorkData.m_bThresholdAlreadyReached )
		{
			m_CurReqWorkData.m_bThresholdAlreadyReached = TRUE;
			rCommon.m_LocoEventCallback( rCommon.m_pCallbackOwner, rRequest.m_ID, CPNT_LOCO_EVENT_THRESHOLD_REACHED );
		}
		if( rCommon.m_LocoEventCallback && !m_bPathIsDirty && !m_CurReqWorkData.m_bDestinationAlreadyReached )
		{
			m_CurReqWorkData.m_bDestinationAlreadyReached = TRUE;
			GDisLocoArrivals++;
			rCommon.m_LocoEventCallback( rCommon.m_pCallbackOwner, rRequest.m_ID, CPNT_LOCO_EVENT_DESTINATION_REACHED );
		}
		return;
	}
	m_CurReqWorkData.m_bDestinationAlreadyReached = FALSE;
	ComputeIsArrivedFlagAndSq2DDistToPathEnd( fScratch );
	if( ( rCommon.m_fEndLocationThreshold * rCommon.m_fEndLocationThreshold ) < m_fSq2DDistToPathEnd )
	{
		m_CurReqWorkData.m_bThresholdAlreadyReached = FALSE;
	}
	else if( rCommon.m_LocoEventCallback && !m_bPathIsDirty && !m_CurReqWorkData.m_bThresholdAlreadyReached )
	{
		m_CurReqWorkData.m_bThresholdAlreadyReached = TRUE;
		rCommon.m_LocoEventCallback( rCommon.m_pCallbackOwner, rRequest.m_ID, CPNT_LOCO_EVENT_THRESHOLD_REACHED );
	}
}

/*-----------------------------------------------------------------------------
	ComputeDynamic
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54bec0 (2012 0x58ca10, 3,026 bytes), agent GD: the frame's structure is now retail's, read off
// both decompiles. Retail first picks the multi-directional speed limit and the look-at multiplier, then takes one of two
// roads. Disabled, or a pawn being deleted, or the STOP_END root state, or an anim node playing a specific anim: the
// "animation owns the pawn" road - m_CurPawnLocation is the pawn's location, the arrival flag is recomputed,
// UpdatePathProperties / UpdateTurn / UpdateStartMove / UpdateStopMove run, every speed and yaw is reset and the move is
// the mesh's root-motion delta. Otherwise the full road: the real speed from the pawn's velocity, the stuck-against-a-wall
// test (velocity below m_fVelocityToStopMoving while the previous frame's was above m_fVelocityToStartToMove), UpdateFollow,
// and then the time step is walked in sub-steps of at most 0.04 s - each one UpdatePathProperties, the move yaw, the three
// root-motion updates, the target yaw, UpdateMoveYaw, ComputeTargetMoveSpeed, UpdateMoveSpeed, ComputePathMove accumulated
// into m_CurPathMove, m_CurPawnLocation = Location + m_CurPathMove, the arrival flag, HandleArrival - so a 0.3 s hitch
// frame is eight 0.04 s steps and the arrival clamp still lands on the mark. After the loop ApplySteering, the speed
// clamp when something other than the path pushes, ClampMove, then the pawn's Velocity / Acceleration from the move,
// m_PreviousVelocity, the steering force rolled over and ComputeMostRelevantSpeedIdx (the caller's). Agent DN's
// single-step version computed the arrival flag inside HandleArrival from the pawn's current location, which is the premise
// agentFV.md §1 carried ("ComputeDynamic exits on IsDisabled() || STOP_END first" - it did not: nothing in this tree
// enters STOP_END and IsDisabled() was never true for the go-to pawn; the pawn stood "arrived" inside DN's radius).
// DISHONORED(bringup): UpdateFollow and the avoidance force are unported (the file banner names them);
// ComputeLookatSpeedMultiplier needs the look-at component and stays 1. UpdateTurn / UpdateStartMove / UpdateStopMove are agent GK's
// (arkcomponentlocomotionrootmove.cpp) and run where retail runs them.
void FArkComponentLocomotion::ComputeDynamic( FLOAT _fTimeStep )
{
	if( !m_pPawnOwner )
	{
		return;
	}
	DISLOCO_MARK( dyn_enter )
	const FVector RootMotionRead = m_pPawnOwner->Mesh ? m_pPawnOwner->Mesh->RootMotionDelta.GetTranslation() : FVector( 0.f, 0.f, 0.f );
	// retail clears the stuck flag at the top of every frame (2012 0x58ca10+0x6b); the test below re-raises it
	m_bStuckedInAWall = FALSE;
	// DISHONORED(port, agent GK): retail's first road is taken when the component is disabled, the pawn is being deleted, the
	// root state is STOP_END, TURN or START, or the anim node plays only a specific animation (the turn / start / stop
	// animation owns the pawn): the three root-move machines still run there, on the pawn's own location.
	const UBOOL bAnimationOwnsThePawn = m_RootState == CPNT_LOCO_ROOT_STATE_STOP_END || m_RootState == CPNT_LOCO_ROOT_STATE_TURN
		|| m_RootState == CPNT_LOCO_ROOT_STATE_START || ( m_bCanManageAnim && m_pAnimNodeLoco && m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() );
	if( IsDisabled() || bAnimationOwnsThePawn )
	{
		if( !IsDisabled() )
		{
			m_CurPawnLocation = m_pPawnOwner->Location;
			m_bIsArrived = ComputeIsArrivedFlagAndSq2DDistToPathEnd( m_fSq2DDistToPathEnd );
			UpdatePathProperties();
			UpdateTurn( _fTimeStep );
			UpdateStartMove( _fTimeStep );
			UpdateStopMove( _fTimeStep );
			m_fRealSpeed = m_pPawnOwner->Velocity.Size();
			m_fRealScaledSpeed = m_fRealSpeed / GetFinalSpeedMultiplier();
			m_fCurMoveSpeed = m_fRealScaledSpeed;
		}
		else
		{
			m_fRealSpeed = 0.f;
			m_fRealScaledSpeed = 0.f;
			m_fCurMoveSpeed = 0.f;
		}
		m_fTargetMoveSpeed = 0.f;
		m_fCurAdaptedAccel = m_fCurDeceleration;
		m_fCurMoveRotationSpeed = 0.f;
		m_CurMoveYaw = -200000;
		m_TargetMoveYaw = -200000;
		m_CurPathMove = FVector( 0.f, 0.f, 0.f );
		m_PreviousPathForce = FVector( 0.f, 0.f, 0.f );
		m_CurMove = FVector( 0.f, 0.f, 0.f );
		if( m_pPawnOwner->Mesh )
		{
			m_CurMove = m_pPawnOwner->Mesh->RootMotionDelta.GetTranslation();
			m_CurMove.Z = 0.f;
		}
		m_fMaxMultiDirSpeedForAngle = 0.f;
	}
	else
	{
		m_fRealSpeed = m_pPawnOwner->Velocity.Size();
		m_fRealScaledSpeed = m_fRealSpeed / GetFinalSpeedMultiplier();
		if( !m_PreviousVelocity.IsZero() && m_pConfig )
		{
			const FLOAT fPrevSq2D = m_PreviousVelocity.X * m_PreviousVelocity.X + m_PreviousVelocity.Y * m_PreviousVelocity.Y;
			if( m_pConfig->m_fVelocityToStopMoving > m_fRealSpeed && fPrevSq2D > ( m_pConfig->m_fVelocityToStartToMove * m_pConfig->m_fVelocityToStartToMove ) )
			{
				m_bStuckedInAWall = TRUE;
				m_CurMoveYaw = -200000;
				IBDiagCheckStuck();
			}
		}
		m_CurPathMove = FVector( 0.f, 0.f, 0.f );
		m_CurPawnLocation = m_pPawnOwner->Location;
		m_bIsArrived = ComputeIsArrivedFlagAndSq2DDistToPathEnd( m_fSq2DDistToPathEnd );

		FLOAT fTimeLeft = _fTimeStep;
		do
		{
			const FLOAT fStep = Min( 0.04f, fTimeLeft );
			UpdatePathProperties();
			m_CurMoveYaw = ComputeMoveYaw();
			// retail refreshes the target yaw before the root-move machines only on a new path or a stale polyline, then
			// again after them (2012 0x58cb8d..0x58ceea)
			if( m_bStartingNewPath || !m_bBuiltPathUpToDate )
			{
				m_TargetMoveYaw = ComputeTargetMoveYaw();
			}
			UpdateTurn( fStep );
			UpdateStartMove( _fTimeStep );
			UpdateStopMove( _fTimeStep );
			m_TargetMoveYaw = ComputeTargetMoveYaw();
			m_CurMoveYaw = UpdateMoveYaw( fStep, m_fCurMoveRotationSpeed, m_fMaxMoveRotationSpeed, m_fMaxMultiDirSpeedForAngle );
			m_fTargetMoveSpeed = ComputeTargetMoveSpeed( fStep, m_fCurAdaptedAccel );
			m_fCurMoveSpeed = UpdateMoveSpeed( fStep );
			m_CurPathMove += ComputePathMove( fStep );
			m_CurPawnLocation = m_pPawnOwner->Location + m_CurPathMove;
			m_bIsArrived = ComputeIsArrivedFlagAndSq2DDistToPathEnd( m_fSq2DDistToPathEnd );
			HandleArrival( fStep );
			fTimeLeft -= 0.04f;
		}
		while( fTimeLeft > 0.00001f );

		m_CurMove = ApplySteering( _fTimeStep, m_CurPathMove );
		if( IsThereAnotherForceThanPath() && m_pConfig && _fTimeStep > 0.f )
		{
			// Something besides the path is pushing: the move's speed is held to what the config lets a pawn start or stop
			// with and to one acceleration step above the real speed, so a push never launches the pawn.
			const FLOAT fSpeed = m_CurMove.Size2D() / _fTimeStep;
			FLOAT fMaxSpeed = fSpeed;
			if( Abs( m_fRealScaledSpeed ) < 0.1f )
			{
				fMaxSpeed = ( m_pConfig->m_fVelocityToStartToMove <= fSpeed ) ? m_pConfig->m_fVelocityToStopMoving : 0.f;
			}
			else if( m_pConfig->m_fVelocityToStopMoving > fSpeed )
			{
				fMaxSpeed = 0.f;
			}
			if( Abs( fMaxSpeed ) >= 0.1f && ( m_fCurAdaptedAccel * _fTimeStep + m_fRealScaledSpeed ) < fMaxSpeed )
			{
				fMaxSpeed = m_fCurAdaptedAccel * _fTimeStep + m_fRealScaledSpeed;
			}
			FVector Dir = m_CurMove;
			Dir.Z = 0.f;
			Dir = Dir.SafeNormal();
			m_CurMove = Dir * ( fMaxSpeed * _fTimeStep );
		}
		FLOAT fMaxMoveUpDist = 0.f;
		m_CurMove = ClampMove( _fTimeStep, m_CurMove, fMaxMoveUpDist );
	}

	if( m_pPawnOwner->Mesh )
	{
		m_pPawnOwner->Mesh->RootMotionDelta.SetTranslation( FVector( 0.f, 0.f, 0.f ) );
	}
	// DISHONORED(port, agent IB): the pawn's Velocity is the dynamics' answer, 2012 0x58ca10 / 2013 0x94bec0 (VA) tail: unless the
	// pawn falls or is a rigid body, or root motion owns it with the mesh in RMM_Ignore, Velocity = m_CurMove / dt x the final speed
	// multiplier (on a sloped floor: |m_CurMove.XY| / dt x multiplier along m_CurMove projected onto the floor plane) and
	// Acceleration = Velocity / dt. FullMovePawn then moves Velocity x ITS time step, so the move does not depend on the dynamics
	// and the move running in different frames (the manager's PreAsyncWork tick runs after the actors, one physWalking ahead).
	const BYTE Physics = m_pPawnOwner->Physics;
	if( _fTimeStep > 0.f && Physics != PHYS_RigidBody && Physics != PHYS_Falling
		&& !( IsRootMotion() && m_pPawnOwner->Mesh && m_pPawnOwner->Mesh->RootMotionMode == RMM_Ignore ) )
	{
		const FLOAT fInvTimeStep = 1.f / _fTimeStep;
		const FVector& Floor = m_pPawnOwner->Floor;
		if( Floor.IsNearlyZero( 0.0001f ) || Floor.Z >= 0.99f )
		{
			m_pPawnOwner->Velocity = m_CurMove * fInvTimeStep * GetFinalSpeedMultiplier();
		}
		else
		{
			const FLOAT fSpeed = GetFinalSpeedMultiplier() * ( m_CurMove.Size2D() * fInvTimeStep );
			FVector Dir = m_CurMove;
			if( Abs( Floor.Z ) >= 0.0001f )
			{
				Dir.Z = -( Dir.Y * Floor.Y + Dir.X * Floor.X ) / Floor.Z;
			}
			m_pPawnOwner->Velocity = Dir.SafeNormal() * fSpeed;
		}
		m_pPawnOwner->Acceleration = m_pPawnOwner->Velocity * fInvTimeStep;
	}
	m_PreviousVelocity = m_pPawnOwner->Velocity;
	UpdateSharedProperties();
	IKDiagDynamic( bAnimationOwnsThePawn, RootMotionRead );
}

/*-----------------------------------------------------------------------------
	the translation
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x543d00 (2012 0x584ca0). Faithful: below the time-step floor nothing happens; the moderate
// move is taken only when the pawn is on a valid base, is not on a start poly-less navigation handle, is not being
// previewed and the moderate move itself accepts the frame - otherwise the full move runs.
void FArkComponentLocomotion::MovePawn( FLOAT _fTimeStep, UBOOL _bPreview )
{
	if( _fTimeStep <= 0.0001f || !m_pPawnOwner )
	{
		return;
	}
	GDisLocoMovePawnCalls++;
	IBDiagCount( IBDIAG_MOVE );

	UBOOL bOnValidBase = FALSE;
	if( m_pPawnOwner->Base )
	{
		UPrimitiveComponent* pBaseCollision = m_pPawnOwner->Base->CollisionComponent;
		bOnValidBase = ( pBaseCollision == NULL ) || pBaseCollision->BlockActors;
	}

	UNavigationHandle* pNavHandle = m_pPawnOwner->GetNavigationHandle();
	const UBOOL bHasStartPoly = pNavHandle && pNavHandle->m_pStartPoly;
	if( m_bInertiaDisabled || !bOnValidBase || !bHasStartPoly || _bPreview || !ModerateMovePawn( _fTimeStep ) )
	{
		FullMovePawn( _fTimeStep, bOnValidBase, _bPreview );
	}
}

// DISHONORED(port): 2013 rva 0x540d80 (2012 0x581d50, 1,447 bytes). The cheap move, used when the pawn is standing on
// something solid and the navigation handle already knows which poly it is on: the frame's velocity is applied directly
// and the Z is taken from the plane of that poly rather than from a trace, so a walking NPC costs no collision query.
// Retail refuses the frame (returns FALSE, so the full move runs instead) when the destination is not reachable.
// DISHONORED(bringup): retail also raises touch and bump events from a MultiPointCheck it does anyway
// (SendTouchAndBumpEvents / UnTouchActors, 2013 rvas 0x53e0d0 / 0x53fa60) and projects the Z onto the poly's plane using
// FNavMeshPolyBase's normal; this projects onto the poly's centre plane, which is the same thing for the flat polys the
// mesh is mostly made of, and it leaves the touch events to the full move.
UBOOL FArkComponentLocomotion::ModerateMovePawn( FLOAT _fTimeStep )
{
	// DISHONORED(bringup): declined, always, so every frame takes FullMovePawn. Retail's cheap path writes the pawn's
	// Location directly and takes the Z from the start poly's plane, which is only correct while the pawn is provably
	// inside that poly - and the thing that keeps it provable is the multi-point check and the touch bookkeeping this
	// package leaves to UWorld::MoveActor. Declining costs one swept move per NPC per frame (26 on L_Tower_P) and is the
	// difference between "cheap and possibly through a wall" and "correct"; the counters say what it costs.
	return FALSE;
}

// DISHONORED(port, agent IK3): 2012 0x580ae0 (4,707 B) / 2013 VA 0x93fb00 (4,729 B), whole. Retail moves the pawn with its own
// traces and writes the location, it never calls UWorld::MoveActor:
//   1. the rotation is (0, Normalize( Rotation.Yaw + face-to m_CurBodyYawDelta ), 0); the move is Velocity x dt (m_CurMove when
//      that is not zero while the move speed is and no animation owns the pawn), flattened;
//   2. moving or without a base, the cylinder (XY x 1.001, x 0.5 with inertia disabled), raised by MaxStepHeight, is pushed out
//      of what it overlaps (FindValidLocation) and the touch / bump events of that check are sent; an NPC-only collision in
//      it drops the NPC-only trace flag (0x10000000) from the move's traces;
//   3. moving: the raised cylinder is swept along the move; a hit on something steeper than the pawn's ledge floor Z slides
//      along it (the move's part along the hit's 2D normal removed, scaled by the time left) with a second sweep, and the hit
//      normal of a non-pawn becomes m_MoveHitNormal; not moving with a base: nothing but the rotation;
//   4. the floor: a sweep from the moved, raised cylinder down 3 x MaxStepHeight. No floor: the move is kept flat only when a
//      fall is allowed, and the pawn falls. A floor steeper than the ledge floor Z: one slide down its slope (2 x MaxStepHeight)
//      onto a walkable floor, or the same as no floor. A walkable floor ends the move on it - unless it is a skeletal mesh whose
//      actor is not bCanStepUpOn (AActor +288 bit 0x20000), which cancels the move;
//   5. the location and rotation are written, the floor actor becomes the base (or refreshes Floor), the components update,
//      the pawn leaves what it no longer touches, the zone and the push body follow, and Velocity is the realised move / dt
//      clamped to the speed asked;
//   6. the root-motion translation is consumed; a pawn whose move failed while it was moving or without a base falls when
//      nothing walkable is within 1.99 x MaxStepHeight below.
// DISHONORED(bringup): UWorld::MoveAttachedActors (2012 0x25f970, Arkane's 2,558 B) is the location write plus the attached
// actors' move; the attached actors follow the location only (FarMoveActor, as the reference's MoveActor would), not the
// rotation. The pawn flag cleared at the end (2012: pawn +296 bit 0x1000) is not identified and not cleared.
void FArkComponentLocomotion::FullMovePawn( FLOAT _fTimeStep, UBOOL _bOnValidBase, UBOOL _bPreview )
{
	if( !m_pPawnOwner || !m_pPawnOwner->CollisionComponent )
	{
		return;
	}
	ADishonoredNPCPawn* pPawn = m_pPawnOwner;
	FRotator NewRotation = pPawn->Rotation;
	if( m_pFaceToCpnt )
	{
		NewRotation = FRotator( 0, FRotator::NormalizeAxis( pPawn->Rotation.Yaw + m_pFaceToCpnt->GetCurBodyYawDelta() ), 0 );
	}
	else if( m_CurMoveYaw != -200000 )
	{
		// DISHONORED(bringup): without FArkComponentFaceTo (IC's -icfaceto is off by default) the pawn faces its move yaw.
		NewRotation = FRotator( 0, m_CurMoveYaw, 0 );
	}
	const UBOOL bRotationChanged = pPawn->Rotation.Yaw != NewRotation.Yaw || pPawn->Rotation.Pitch != 0 || pPawn->Rotation.Roll != 0;

	const FLOAT fSpeedBeforeMove = pPawn->Velocity.Size();
	FVector VelocityMove = pPawn->Velocity * _fTimeStep;
	if( !VelocityMove.IsZero() && m_fCurMoveSpeed == 0.f && !IsRootMotion() )
	{
		VelocityMove = m_CurMove;
	}
	FVector Delta( VelocityMove.X, VelocityMove.Y, 0.f );
	const UBOOL bMoving = Delta.X != 0.f || Delta.Y != 0.f;
	m_MoveHitNormal = FVector( 0.f, 0.f, 0.f );
	const UBOOL bCheckFall = bMoving || !_bOnValidBase;

	const FLOAT fXYScale = m_bInertiaDisabled ? 0.5f : 1.001f;
	const FVector BoundsExtent = pPawn->CollisionComponent->Bounds.BoxExtent;
	const FVector Extent( BoundsExtent.X * fXYScale, BoundsExtent.Y * fXYScale, BoundsExtent.Z );
	const FLOAT fStep = pPawn->MaxStepHeight;
	const FLOAT fLedgeFloorZ = pPawn->WalkableFloorZ;
	FVector Origin = pPawn->CollisionComponent->Bounds.Origin;

	FMemMark Mark( GMainThreadMemStack );
	FCheckResult* pValidHits = NULL;
	FVector ValidDelta( 0.f, 0.f, 0.f );
	UBOOL bLocationWasUnsafe = FALSE;
	UBOOL bHitNPCOnlyCollision = FALSE;
	if( bCheckFall )
	{
		const FVector RaisedOrigin = Origin + FVector( 0.f, 0.f, fStep );
		FVector ValidOrigin = RaisedOrigin;
		pValidHits = FindValidLocation( GMainThreadMemStack, ValidOrigin, 0x1020209F, Extent, bLocationWasUnsafe, bHitNPCOnlyCollision );
		if( bLocationWasUnsafe )
		{
			ValidDelta = ValidOrigin - RaisedOrigin;
			pPawn->Location += ValidDelta;
			Origin += ValidDelta;
		}
		SendTouchAndBumpEvents( pValidHits, _bPreview );
	}
	const DWORD TraceFlags = bHitNPCOnlyCollision ? 0x30DF : 0x100030DF;

	FCheckResult HoriHit( 1.f );
	if( bMoving )
	{
		const FVector HoriStart = Origin + FVector( 0.f, 0.f, fStep );
		if( !GWorld->SingleLineCheck( HoriHit, pPawn, HoriStart + Delta, HoriStart, TraceFlags, Extent ) )
		{
			FVector HitNormal2D( HoriHit.Normal.X, HoriHit.Normal.Y, 0.f );
			HitNormal2D = HitNormal2D.SafeNormal();
			FVector Slide( 0.f, 0.f, 0.f );
			UBOOL bSlide = FALSE;
			if( fLedgeFloorZ > HoriHit.Normal.Z )
			{
				Slide = ( Delta - HitNormal2D * ( HitNormal2D | Delta ) ) * ( 1.f - HoriHit.Time );
				bSlide = ( Slide | Delta ) > 0.f;
			}
			Delta = HoriHit.Location - HoriStart;
			if( bSlide )
			{
				FCheckResult SlideHit( 1.f );
				if( !GWorld->SingleLineCheck( SlideHit, pPawn, HoriStart + Delta + Slide, HoriStart + Delta, TraceFlags, Extent ) )
				{
					Delta = SlideHit.Location - HoriStart;
				}
				else
				{
					Delta += Slide;
				}
			}
			if( !Cast<APawn>( HoriHit.Actor ) )
			{
				m_MoveHitNormal = HoriHit.Normal;
			}
		}
	}

	UBOOL bMoved = TRUE;
	UBOOL bFalls = FALSE;
	const TCHAR* FloorOutcome = TEXT("not checked");
	const FCheckResult* pFloorHit = NULL;
	FCheckResult VertHit( 1.f );
	FCheckResult SlopeHit( 1.f );
	if( !bMoving && _bOnValidBase && !_bPreview )
	{
		bMoved = FALSE;
		Delta = FVector( 0.f, 0.f, 0.f );
	}
	else
	{
		const UBOOL bFallAllowed = m_bFallAllowedByExternal || m_bFallAllowedByCode;
		const FVector VertStart = Origin + Delta + FVector( 0.f, 0.f, fStep );
		const FVector VertEnd = VertStart - FVector( 0.f, 0.f, fStep * 3.f );
		UBOOL bNoFloor = GWorld->SingleLineCheck( VertHit, pPawn, VertEnd, VertStart, TraceFlags, Extent );
		FloorOutcome = TEXT("walkable floor");
		if( !bNoFloor )
		{
			if( fLedgeFloorZ >= VertHit.Normal.Z )
			{
				const FVector Side = VertHit.Normal ^ FVector( 0.f, 0.f, -1.f );
				const FVector Downhill = Side ^ VertHit.Normal;
				const FVector SlopeEnd = VertHit.Location + Downhill * ( ( 1.f - VertHit.Time ) * ( fStep * 2.f ) );
				if( !GWorld->SingleLineCheck( SlopeHit, pPawn, SlopeEnd, VertHit.Location, TraceFlags, Extent ) && fLedgeFloorZ <= SlopeHit.Normal.Z )
				{
					Delta = SlopeHit.Location - Origin;
					pFloorHit = &SlopeHit;
					if( !Cast<APawn>( HoriHit.Actor ) )
					{
						m_MoveHitNormal = VertHit.Normal;
					}
				}
				else
				{
					bNoFloor = TRUE;
				}
				FloorOutcome = bNoFloor ? TEXT("steep floor, no slope landing") : TEXT("steep floor, slid down the slope");
			}
			else if( ( VertHit.Actor && VertHit.Actor->bCanStepUpOn ) || !Cast<USkeletalMeshComponent>( VertHit.Component ) )
			{
				Delta = VertHit.Location - Origin;
				pFloorHit = &VertHit;
			}
			else
			{
				Delta = FVector( 0.f, 0.f, 0.f );
				bMoved = FALSE;
				FloorOutcome = TEXT("skeletal-mesh floor of an actor that is not bCanStepUpOn: move cancelled");
			}
		}
		else
		{
			FloorOutcome = TEXT("no floor within 3 x MaxStepHeight");
		}
		if( bNoFloor )
		{
			bMoved = bFallAllowed;
			bFalls = bFallAllowed;
			Delta.Z = 0.f;
			if( !bFallAllowed )
			{
				Delta = FVector( 0.f, 0.f, 0.f );
			}
		}
	}

	if( !bMoved && !bRotationChanged )
	{
		if( bLocationWasUnsafe && !_bPreview )
		{
			pPawn->ForceUpdateComponents( TRUE, TRUE );
		}
		pPawn->Velocity = FVector( 0.f, 0.f, 0.f );
	}
	else
	{
		if( !bMoved )
		{
			Delta = FVector( 0.f, 0.f, 0.f );
		}
		const FVector StartLocation = pPawn->Location;
		pPawn->Location += Delta;
		pPawn->Rotation = NewRotation;
		if( !_bPreview )
		{
			for( INT AttachedIdx = 0; AttachedIdx < pPawn->Attached.Num(); ++AttachedIdx )
			{
				AActor* pAttached = pPawn->Attached( AttachedIdx );
				if( pAttached && !pAttached->bHardAttach )
				{
					GWorld->FarMoveActor( pAttached, pAttached->Location + Delta, FALSE, TRUE, TRUE );
				}
			}
			if( pFloorHit )
			{
				if( pFloorHit->Actor == pPawn->Base || !pPawn->IsBlockedBy( pFloorHit->Actor, pFloorHit->Component ) )
				{
					pPawn->Floor = pFloorHit->Normal;
				}
				else
				{
					pPawn->SetBase( pFloorHit->Actor, pFloorHit->Normal, TRUE, NULL, NAME_None );
					if( !_bOnValidBase && pPawn->Base )
					{
						pPawn->RelativeLocation = pPawn->Location - pPawn->Base->Location;
					}
				}
			}
			pPawn->ForceUpdateComponents( GWorld->InTick && !GWorld->bPostTickComponentUpdate, TRUE );
			if( bMoved )
			{
				UnTouchActors( pValidHits, FALSE );
			}
			if( !m_bInertiaDisabled )
			{
				pPawn->SetZone( FALSE, FALSE );
				pPawn->UpdatePushBody();
			}
			pPawn->Velocity = ClampLength( ( ValidDelta + Delta ) * ( 1.f / _fTimeStep ), fSpeedBeforeMove );
		}
		const FVector Actual = pPawn->Location - StartLocation;
		GDisLocoDistanceMoved += Actual.Size2D();
		IKDiagMove( VelocityMove, HoriHit, Actual );
	}
	IKDiagFloor( FloorOutcome, VertHit, Delta );

	if( pPawn->Mesh )
	{
		pPawn->Mesh->RootMotionDelta.SetTranslation( FVector( 0.f, 0.f, 0.f ) );
	}
	m_CurPawnLocation = pPawn->Location;
	if( !bFalls && !bMoved && bCheckFall && !_bPreview )
	{
		const FVector GroundStart = pPawn->CollisionComponent->Bounds.Origin;
		FCheckResult FallHit( 1.f );
		if( GWorld->SingleLineCheck( FallHit, pPawn, GroundStart - FVector( 0.f, 0.f, fStep * 1.99f ), GroundStart, TraceFlags, Extent )
			|| fLedgeFloorZ > FallHit.Normal.Z )
		{
			bFalls = TRUE;
		}
	}
	if( bFalls )
	{
		GDisLocoFalls++;
		pPawn->setPhysics( PHYS_Falling, NULL, FVector( 0.f, 0.f, 1.f ) );
	}
}

// DISHONORED(port, agent IK3): 2012 0x57f220 (275 B). Every hit actor that is neither the pawn nor something the pawn is based
// on (or based on the pawn) is touched when it does not block, and bumped both ways - or, for world geometry the pawn's
// 0x2030DF trace would hit, NotifyBumpLevel.
void FArkComponentLocomotion::SendTouchAndBumpEvents( const FCheckResult* const _pHits, UBOOL _bPreview )
{
	if( _bPreview )
	{
		return;
	}
	for( const FCheckResult* pHit = _pHits; pHit; pHit = pHit->GetNext() )
	{
		AActor* pActor = pHit->Actor;
		if( !pActor || pActor == m_pPawnOwner || m_pPawnOwner->IsBasedOn( pActor ) || pActor->IsBasedOn( m_pPawnOwner ) )
		{
			continue;
		}
		if( !m_pPawnOwner->IsBlockedBy( pActor, pHit->Component ) )
		{
			m_pPawnOwner->BeginTouch( pActor, pHit->Component, pHit->Location, pHit->Normal, pHit->SourceComponent );
		}
		if( !pActor->bWorldGeometry )
		{
			pActor->NotifyBump( m_pPawnOwner, m_pPawnOwner->CollisionComponent, pHit->Normal );
			m_pPawnOwner->NotifyBump( pActor, pHit->Component, pHit->Normal );
		}
		else if( pActor->ShouldTrace( pHit->Component, m_pPawnOwner, 0x2030DF ) )
		{
			m_pPawnOwner->NotifyBumpLevel( pHit->Location, pHit->Normal );
		}
	}
}

// DISHONORED(port, agent IK3): 2012 0x580a40 (155 B): the pawn stops touching every actor the check did not hit.
void FArkComponentLocomotion::UnTouchActors( const FCheckResult* const _pHits, UBOOL _bPreview )
{
	if( _bPreview )
	{
		return;
	}
	INT TouchIdx = 0;
	while( TouchIdx < m_pPawnOwner->Touching.Num() )
	{
		AActor* pTouching = m_pPawnOwner->Touching( TouchIdx );
		UBOOL bStillHit = pTouching == NULL;
		for( const FCheckResult* pHit = _pHits; pHit && !bStillHit; pHit = pHit->GetNext() )
		{
			bStillHit = pHit->Actor == pTouching;
		}
		if( bStillHit )
		{
			++TouchIdx;
		}
		else
		{
			m_pPawnOwner->EndTouch( pTouching, FALSE );
		}
	}
}

// DISHONORED(port, agent IK2): 2012 0x57ffb0 (745 B). Up to four rounds (one with inertia disabled) of a multi-point check of
// the extent at _Location: among the overlapped actors that are neither the pawn nor based on it and that ShouldTrace the
// pawn's flags | 0x1040 (a cylinder only when its own zero-length check at _Location really hits, unless inertia is
// disabled), an NPC-only collision (ShouldTrace 0x10000000) is reported, any other one's pushed-out location is a candidate
// and the nearest candidate's XY becomes _Location; a candidate equal to _Location ends the search as unsafe.
FCheckResult* FArkComponentLocomotion::FindValidLocation( FMemStack& _Mem, FVector& _Location, DWORD _TraceFlags, const FVector& _Extent, UBOOL& _bLocationWasUnsafe, UBOOL& _bHitNPCOnlyCollision ) const
{
	INT MaxIteration = m_bInertiaDisabled ? 1 : 4;
	FCheckResult* pHitResult = NULL;
	UBOOL bCollidedSomething = TRUE;
	while( bCollidedSomething )
	{
		bCollidedSomething = FALSE;
		FCheckResult* pHits = GWorld->MultiPointCheck( _Mem, _Location, _Extent, _TraceFlags );
		if( pHits )
		{
			if( pHitResult )
			{
				pHitResult->Next = pHits;
			}
			else
			{
				pHitResult = pHits;
			}
			FCheckResult BestHit( 1.f );
			FLOAT fBestSquaredDist = BIG_NUMBER;
			UBOOL bCollided = FALSE;
			for( FCheckResult* pHit = pHits; pHit; pHit = pHit->GetNext() )
			{
				AActor* pActor = pHit->Actor;
				if( !pActor || pActor == m_pPawnOwner )
				{
					continue;
				}
				UBOOL bBasedOnPawn = FALSE;
				for( AActor* pBase = pActor; pBase; pBase = pBase->Base )
				{
					if( pBase == m_pPawnOwner )
					{
						bBasedOnPawn = TRUE;
						break;
					}
				}
				if( bBasedOnPawn || !pActor->ShouldTrace( pHit->Component, m_pPawnOwner, _TraceFlags | 0x1040 ) )
				{
					continue;
				}
				if( !m_bInertiaDisabled && pHit->Component && pHit->Component->IsA( UCylinderComponent::StaticClass() ) )
				{
					FCheckResult CylinderHit( 1.f );
					if( pHit->Component->LineCheck( CylinderHit, _Location, _Location, _Extent, _TraceFlags ) )
					{
						continue;
					}
				}
				if( pActor->ShouldTrace( pHit->Component, m_pPawnOwner, 0x10000000 ) )
				{
					_bHitNPCOnlyCollision = TRUE;
				}
				else if( !pHit->Location.Equals( _Location, 0.0001f ) )
				{
					const FLOAT fSquaredDist = ( pHit->Location - _Location ).SizeSquared();
					if( fBestSquaredDist > fSquaredDist )
					{
						BestHit = *pHit;
						fBestSquaredDist = fSquaredDist;
						bCollided = TRUE;
					}
				}
			}
			if( bCollided )
			{
				_bLocationWasUnsafe = TRUE;
				if( BestHit.Location.X != _Location.X || BestHit.Location.Y != _Location.Y )
				{
					--MaxIteration;
					_Location.X = BestHit.Location.X;
					_Location.Y = BestHit.Location.Y;
					bCollidedSomething = TRUE;
				}
			}
		}
		if( MaxIteration <= 0 )
		{
			break;
		}
	}
	return pHitResult;
}

