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

// DISHONORED(port): 2013 rva 0x53fb00 (2012 0x580ae0, 4,707 bytes) - the largest function in the component. Retail rolls
// its own swept move: a MultiPointCheck of the pawn's cylinder along the move, the touch and bump events that fall out of
// it, a step-up attempt when the move is blocked by something shorter than MaxStepHeight, a wall slide when it is not, a
// fall when the ground has gone and there is no navigation mesh below, and the teleport back onto the mesh when the pawn
// has been off it for too long.
// DISHONORED(bringup): this uses UWorld::MoveActor, which is UE3's own swept move and already does the step-up, the wall
// slide, the touch and bump notifications and the attached-actor update. What is lost against retail: the pawn will not
// teleport back onto the navigation mesh when it leaves it (CanBeTeleportedToLocationWithoutBeingSeen 0x53e310,
// FindValidLocation 0x53ed70), the fall decision is UE3's rather than Arkane's, and the hit normal the dynamics read for
// the "stuck in a wall" flag comes from MoveActor's hit rather than from the multi-point check.
// DISHONORED(bringup, agent IK): correction, read off 2012 0x580ae0: UWorld::MoveActor has no step-up - retail sweeps the
// horizontal move from MaxStepHeight above the cylinder (0x30DF / 0x100030DF), drops onto the floor within 3 x MaxStepHeight,
// refuses a floor that is a USkeletalMeshComponent unless bit 0x20000 at +288 of the hit's actor is set (decompile labels unresolved), and un-sticks a
// penetrating start with FindValidLocation; none of that is here (agentIK.md section 3).
void FArkComponentLocomotion::FullMovePawn( FLOAT _fTimeStep, UBOOL _bOnValidBase, UBOOL _bPreview )
{
	if( _bPreview )
	{
		return;
	}
	// DISHONORED(port, agent IB): retail's move is the dynamics' Velocity x this frame's time step (2012 0x580ae0 / 2013 0x93fb00
	// head), m_CurMove only when that velocity is not zero while the dynamics' move speed is and no animation owns the pawn.
	const FLOAT fSpeedBeforeMove = m_pPawnOwner->Velocity.Size();
	FVector Move = m_pPawnOwner->Velocity * _fTimeStep;
	if( !Move.IsZero() && m_fCurMoveSpeed == 0.f && !IsRootMotion() )
	{
		Move = m_CurMove;
	}
	Move.Z = 0.f;
	// DISHONORED(port, agent IK2): the body yaw, 2012 0x580ae0 head: with the face-to component the new rotation is
	// (0, Normalize( Rotation.Yaw + m_CurBodyYawDelta ), 0) and a rotation change alone runs the move.
	// DISHONORED(bringup): without FArkComponentFaceTo (IC's -icfaceto is off by default) the pawn faces its move yaw, HEAD's
	// stand-in.
	FRotator NewRotation = m_pPawnOwner->Rotation;
	if( m_pFaceToCpnt )
	{
		NewRotation = FRotator( 0, FRotator::NormalizeAxis( m_pPawnOwner->Rotation.Yaw + m_pFaceToCpnt->GetCurBodyYawDelta() ), 0 );
	}
	else if( m_CurMoveYaw != -200000 )
	{
		NewRotation.Yaw = m_CurMoveYaw;
	}
	const UBOOL bRotationChanged = m_pFaceToCpnt && ( m_pPawnOwner->Rotation.Yaw != NewRotation.Yaw || m_pPawnOwner->Rotation.Pitch != 0 || m_pPawnOwner->Rotation.Roll != 0 );
	if( Move.IsNearlyZero() && !bRotationChanged )
	{
		m_pPawnOwner->Velocity = FVector( 0.f, 0.f, 0.f );
		m_MoveHitNormal = FVector( 0.f, 0.f, 0.f );
		// DISHONORED(port, agent FV): retail's FullMovePawn runs its ground check even for a pawn that is not moving when the pawn
		// has no valid base (`bFallAllowed || !_bHaveBase`, 2012 0x580ae0) - that is what bases a spawned pawn on the floor it
		// stands on, and the boat's guard on the boat. With a base and no move retail does nothing more, and neither does this.
		if( !_bOnValidBase )
		{
			UpdateFloorAndBase( FALSE );
		}
		if( m_pPawnOwner->Mesh != NULL )
		{
			m_pPawnOwner->Mesh->RootMotionDelta.SetTranslation( FVector( 0.f, 0.f, 0.f ) );
		}
		return;
	}

	// DISHONORED(port, agent IK2): retail's un-sticking step, 2012 0x580ae0 (`bFallAllowed || !_bHaveBase`): the cylinder (XY x 1.001,
	// x 0.5 with inertia disabled), raised by MaxStepHeight, is checked where it stands and pushed out of whatever it overlaps
	// before the move; the pawn's location takes the XY correction.
	// DISHONORED(bringup): SendTouchAndBumpEvents (2012 0x57f220) on the check's hits is not ported.
	if( !Move.IsNearlyZero() || !_bOnValidBase )
	{
		const FBoxSphereBounds& Bounds = m_pPawnOwner->CollisionComponent->Bounds;
		const FLOAT fXYScale = m_bInertiaDisabled ? 0.5f : 1.001f;
		const FVector Extent( Bounds.BoxExtent.X * fXYScale, Bounds.BoxExtent.Y * fXYScale, Bounds.BoxExtent.Z );
		const FVector CheckOrigin = Bounds.Origin + FVector( 0.f, 0.f, m_pPawnOwner->MaxStepHeight );
		FVector ValidOrigin = CheckOrigin;
		UBOOL bLocationWasUnsafe = FALSE;
		UBOOL bHitNPCOnlyCollision = FALSE;
		FMemMark Mark( GMainThreadMemStack );
		FindValidLocation( GMainThreadMemStack, ValidOrigin, 0x1020209F, Extent, bLocationWasUnsafe, bHitNPCOnlyCollision );
		if( bLocationWasUnsafe && ValidOrigin != CheckOrigin )
		{
			GWorld->FarMoveActor( m_pPawnOwner, m_pPawnOwner->Location + ( ValidOrigin - CheckOrigin ), FALSE, TRUE, FALSE );
		}
	}

	const FVector StartLocation = m_pPawnOwner->Location;
	FCheckResult Hit( 1.f );
	DISLOCO_TRACE( move_enter, "move %s from %s", *Move.ToString(), *StartLocation.ToString() )
	GWorld->MoveActor( m_pPawnOwner, Move, NewRotation, 0, Hit );
	DISLOCO_MARK( move_done )

	// DISHONORED(port, agent IK2): retail keeps only the hit normal (when the hit is not a pawn) - the stuck flag is the dynamics'
	// velocity test and the scheduler repaths on it (2012 0x58a4d0). HEAD also raised the stuck flag here, burnt the request's
	// "pathfinding failed" bit and sent CPNT_LOCO_EVENT_HIT_OBSTACLE, none of which retail's FullMovePawn does.
	m_MoveHitNormal = ( Hit.Time < 1.f && !Cast<APawn>( Hit.Actor ) ) ? Hit.Normal : FVector( 0.f, 0.f, 0.f );

	const FVector Actual = m_pPawnOwner->Location - StartLocation;
	IKDiagMove( Move, Hit, Actual );
	GDisLocoDistanceMoved += Actual.Size2D();
	// DISHONORED(port, agent IB): retail clamps the measured velocity to the speed the move was asked for (2012 0x580ae0 +0x5f0,
	// ClampLength) and leaves Acceleration as the dynamics wrote it.
	m_pPawnOwner->Velocity = ClampLength( ( _fTimeStep > 0.f ) ? ( Actual / _fTimeStep ) : FVector( 0.f, 0.f, 0.f ), fSpeedBeforeMove );
	m_CurPawnLocation = m_pPawnOwner->Location;
	IBDiagCheckStuck();
	UpdateFloorAndBase( _bOnValidBase );
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

// DISHONORED(port): agent FV (PHASE18 FV). The ground hit of retail's FullMovePawn (2012 0x580ae0, after MoveAttachedActors):
// the pawn's cylinder is swept from MaxStepHeight above its location to twice MaxStepHeight below it, with the trace flags
// retail passes its vertical checks (0x1020209F: pawns, movers, level, volumes, others, level geometry, stop at any hit,
// complex collision). What the sweep hits is the floor: an actor the pawn is already based on, or one that would not
// block it, only refreshes Floor; a blocking actor becomes the Base (APawn::SetBase with the hit normal, notifying it), and
// a pawn that had no base keeps its offset from the new one in RelativeLocation. This is how a pawn standing on a moving
// InterpActor is carried by it - the swept move UWorld::MoveActor this tree uses for the walk never sets a base.
void FArkComponentLocomotion::UpdateFloorAndBase( UBOOL _bHadBase )
{
	if( !m_pPawnOwner || !m_pPawnOwner->CollisionComponent || !GWorld )
	{
		return;
	}
	const FLOAT Step = m_pPawnOwner->MaxStepHeight;
	FVector Extent = m_pPawnOwner->CollisionComponent->Bounds.BoxExtent;
	Extent.X *= 1.001f;
	Extent.Y *= 1.001f;
	const FVector Start = m_pPawnOwner->Location + FVector( 0.f, 0.f, Step );
	const FVector End = m_pPawnOwner->Location - FVector( 0.f, 0.f, 2.f * Step );
	FCheckResult Hit( 1.f );
	if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, 0x1020209F, Extent ) )
	{
		// DISHONORED(port, agent GA): retail's tail (2012 0x580ae0 +0x8a0): a pawn that is not moving and stands on
		// nothing within the sweep is put into PHYS_Falling; UE3's physFalling / processLanded land it and base it.
		if( !_bHadBase )
		{
			GDisLocoFalls++;
			m_pPawnOwner->setPhysics( PHYS_Falling );
		}
		return;
	}
	if( Hit.bStartPenetrating )
	{
		// DISHONORED(port, agent GA): the swept cylinder already overlaps something (a rail, a wall, the deck's own
		// collision) and the hit is the start point, not a floor - measured on the boat's guard, whom a hit taken at
		// face value lifted a step height per frame. A plain line from the cylinder's centre says what is under its
		// bottom; a floor more than a step height from the bottom is not this pawn's.
		const FLOAT Height = m_pPawnOwner->CollisionComponent->Bounds.BoxExtent.Z;
		Hit = FCheckResult( 1.f );
		if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, m_pPawnOwner->Location - FVector( 0.f, 0.f, Height + Step * 3.f ), m_pPawnOwner->Location, 0x1020209F )
			|| Abs( ( m_pPawnOwner->Location.Z - Height ) - Hit.Location.Z ) > Step )
		{
			return;
		}
	}
	if( !Hit.Actor )
	{
		return;
	}
	if( Hit.Actor == m_pPawnOwner->Base || !m_pPawnOwner->IsBlockedBy( Hit.Actor, Hit.Component ) )
	{
		m_pPawnOwner->Floor = Hit.Normal;
		return;
	}
	GDisLocoBased++;
	m_pPawnOwner->SetBase( Hit.Actor, Hit.Normal, TRUE, NULL, NAME_None );
	if( !_bHadBase && m_pPawnOwner->Base )
	{
		m_pPawnOwner->RelativeLocation = m_pPawnOwner->Location - m_pPawnOwner->Base->Location;
	}
}
