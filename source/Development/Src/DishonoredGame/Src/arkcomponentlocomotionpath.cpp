// DishonoredGame/src/arkcomponentlocomotionpath.cpp
// DISHONORED(port): agent DN. The path half of the locomotion component - and, with arkpathbuildutils.cpp, the whole of
// the AI's contact with the navigation mesh. Retail's unit is Engine/Src/arkcomponentlocomotionpath.cpp (10 functions,
// 10,027 bytes).
//
// Agent CG scoped this and found it is exactly four things and no more, which this file is:
//   * a path between two mesh points          - UpdatePathFindingEdges -> UNavigationHandle::FindPath, then
//                                               DefaultPathBuilder -> BuildStraightPath (the funnel)
//   * the goals and constraints the AI supplies - ADishonoredNPCPawn::SetupPathGoalsAndConstraints
//   * "where on the mesh is this point"        - FindNearestLocationOnNavMesh, UpdateStartLocAndVerifyIfOnValidPoly
//   * "can I path from here at all"            - CanNPCPathfindFromLocation
// Nothing else in the AI touches the mesh, which is why the AI could be measured without any of it.
//
// One thing worth stating because it is the opposite of what the name suggests: DefaultPathBuilder does NOT search.
// The A* is UNavigationHandle's, upstream in UpdatePathFindingEdges; DefaultPathBuilder turns the edge list that search
// produced into a polyline, and it is the *only* entry the rest of locomotion has into the mesh's results.

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "disdesirestructs.h"

/*-----------------------------------------------------------------------------
	"where on the mesh is this point"
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x545660 (2012 0x586560, 636 bytes). Every poly within a cube of _fSearchDist around the
// pawn is asked for its closest point to the pawn; the nearest one whose Z is within a step height wins. Retail keeps a
// *second* choice - the nearest point regardless of Z - and falls back to it, which is what lets an NPC that has fallen
// off the mesh be pulled back to the mesh below it rather than reporting failure.
UBOOL FArkComponentLocomotion::FindNearestLocationOnNavMesh( const FVector* const _pPawnLocOnGround, FLOAT _fSearchDist, FVector& _rNearestLocationOnNavMesh ) const
{
	if( !m_pPawnOwner )
	{
		return FALSE;
	}
	FVector PawnLocationOnGround = GetPawnGroundLocation();
	FVector PawnLocation = m_pPawnOwner->Location;
	if( _pPawnLocOnGround )
	{
		PawnLocationOnGround = *_pPawnLocOnGround;
		PawnLocation = *_pPawnLocOnGround;
		if( m_pPawnOwner->CylinderComponent )
		{
			PawnLocation.Z += m_pPawnOwner->CylinderComponent->CollisionHeight;
		}
	}

	const FVector Extent( _fSearchDist, _fSearchDist, _fSearchDist );
	const FLOAT fMoveMaxHeight = m_pPawnOwner->MaxStepHeight;

	TArray<FNavMeshPolyBase*> NavPolys;
	if( !UNavigationHandle::GetAllPolysFromPos( PawnLocation, Extent, NavPolys, FALSE ) )
	{
		return FALSE;
	}

	FVector SecondChoiceLocation = _rNearestLocationOnNavMesh;
	FLOAT fBestSquaredDist = BIG_NUMBER;
	FLOAT fBestSecondChoiceSquaredDist = BIG_NUMBER;
	UBOOL bFound = FALSE;

	for( INT i = 0; i < NavPolys.Num(); ++i )
	{
		const FVector Location = NavPolys( i )->GetClosestPointOnPoly( PawnLocation );
		const FLOAT fSquaredDist = ( Location - PawnLocationOnGround ).SizeSquared();
		if( fMoveMaxHeight > Abs( Location.Z - PawnLocationOnGround.Z ) && fBestSquaredDist > fSquaredDist )
		{
			_rNearestLocationOnNavMesh = Location;
			fBestSquaredDist = fSquaredDist;
			bFound = TRUE;
		}
		if( fBestSecondChoiceSquaredDist > fSquaredDist )
		{
			SecondChoiceLocation = Location;
			fBestSecondChoiceSquaredDist = fSquaredDist;
		}
	}

	if( !bFound && NavPolys.Num() > 0 )
	{
		_rNearestLocationOnNavMesh = SecondChoiceLocation;
		bFound = TRUE;
	}
	return bFound;
}

// DISHONORED(port): 2013 rva 0x53ed10 (2012 0x57f8a0, 474 bytes). A line check straight down from ten units above the
// given point finds the ground; the poly under that ground point is looked up, and the point counts as "on a valid poly"
// only when the poly's own surface is within a step height below the ground AND below the top of the pawn's cylinder -
// which is what stops an NPC standing on a table from thinking it is on the floor's poly.
UBOOL FArkComponentLocomotion::UpdateStartLocAndVerifyIfOnValidPoly( FVector& _rStartLocationOnGround, const FVector* const _pSpecificStartLoc ) const
{
	if( !m_pPawnOwner || !m_pPawnOwner->CylinderComponent )
	{
		return FALSE;
	}
	const FLOAT fPawnCollisionHeight = m_pPawnOwner->CylinderComponent->CollisionHeight;
	_rStartLocationOnGround = _pSpecificStartLoc ? *_pSpecificStartLoc : GetPawnGroundLocation();

	FVector Start = _rStartLocationOnGround;
	Start.Z += 10.f;
	FVector End = Start;
	End.Z -= ( fPawnCollisionHeight * 2.f ) + 10.f;

	FCheckResult Hit( 1.f );
	if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, TRACE_World, FVector( 0.f, 0.f, 0.f ) ) )
	{
		return FALSE;
	}
	_rStartLocationOnGround = Hit.Location;

	APylon* pPylon = NULL;
	FNavMeshPolyBase* pPoly = NULL;
	if( !UNavigationHandle::GetPylonAndPolyFromPos( _rStartLocationOnGround, 0.f, pPylon, pPoly ) || !pPoly )
	{
		return FALSE;
	}
	const FVector LocationOnPoly = pPoly->GetClosestPointOnPoly( _rStartLocationOnGround );
	return m_pPawnOwner->MaxStepHeight > ( _rStartLocationOnGround.Z - LocationOnPoly.Z )
		&& ( m_pPawnOwner->Location.Z + fPawnCollisionHeight ) > LocationOnPoly.Z;
}

// DISHONORED(port): 2013 rva 0x546770 (2012 0x587460, 377 bytes). The spawner's "is there room here" test and
// ADishonoredNPCPawn::MoveAtSafeLocation's answer: trace for the floor under the candidate location, then accept it if
// the floor is on a valid poly, or if a poly can be found within 150 units of it.
UBOOL FArkComponentLocomotion::CanNPCPathfindFromLocation( const FVector* const _pStartLocation ) const
{
	if( !m_pPawnOwner || !m_pPawnOwner->CylinderComponent )
	{
		return FALSE;
	}
	FVector StartLocation = _pStartLocation ? *_pStartLocation : m_pPawnOwner->Location;
	const FLOAT fPawnCollisionHeight = m_pPawnOwner->CylinderComponent->CollisionHeight;

	FVector Start = StartLocation;
	Start.Z += fPawnCollisionHeight;
	FVector End = StartLocation;
	End.Z -= fPawnCollisionHeight * 3.f;

	FCheckResult Hit( 1.f );
	if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, TRACE_World, FVector( 0.f, 0.f, 0.f ) ) )
	{
		return FALSE;
	}
	StartLocation.Z = Hit.Location.Z;

	FVector OnGround = StartLocation;
	if( UpdateStartLocAndVerifyIfOnValidPoly( OnGround, &StartLocation ) )
	{
		return TRUE;
	}
	FVector Nearest( 0.f, 0.f, 0.f );
	return FindNearestLocationOnNavMesh( &StartLocation, 150.f, Nearest );
}

/*-----------------------------------------------------------------------------
	the path
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x545270 (2012 0x586180, 401 bytes). Where the active request is actually asking the pawn to
// end up: an actor request resolves the actor now (optionally one tick ahead, from its velocity, which is how a chase
// leads its target), a location request answers its location; a "stop before end" distance pulls the target back along
// the line from the pawn.
FVector FArkComponentLocomotion::ComputeTargetedLocationForPath( FLOAT _fTimeStep, UBOOL _bForNextTick )
{
	if( !m_ReqMgr.HasRequest() )
	{
		return m_pPawnOwner ? m_pPawnOwner->Location : FVector( 0.f, 0.f, 0.f );
	}
	const FLocoRequestData& rData = m_ReqMgr.GetFirstRequest().m_Data;
	FVector Target;
	if( rData.m_Type == CPNT_LOCO_REQUEST_TYPE_ACTOR )
	{
		const AActor* pActor = rData.m_ActorProps.m_pActor;
		if( !pActor )
		{
			return m_pPawnOwner ? m_pPawnOwner->Location : FVector( 0.f, 0.f, 0.f );
		}
		Target = pActor->Location;
		if( _bForNextTick )
		{
			Target += pActor->Velocity * _fTimeStep;
		}
	}
	else
	{
		Target = rData.m_LocationProps.m_Location.Get();
	}

	const FLOAT fStopBefore = rData.m_CommonProps.m_fStopBeforeEndDist;
	if( fStopBefore > 0.f && m_pPawnOwner )
	{
		FVector ToPawn = m_pPawnOwner->Location - Target;
		ToPawn.Z = 0.f;
		const FLOAT fDist = ToPawn.Size();
		if( fDist > fStopBefore )
		{
			Target += ToPawn / fDist * fStopBefore;
		}
		else
		{
			Target = m_pPawnOwner->Location;
		}
	}
	return Target;
}

// DISHONORED(port): 2013 rva 0x5468f0 (2012 0x5875e0, 385 bytes). This is the AI's only call into the path search, and it
// is four steps: work out how far from the walls the corners must sit (the pawn's radius x 1.65, jittered per NPC so a
// queue of guards does not walk in one line), ask the pawn to put its behaviour's goal evaluators and constraints on its
// navigation handle, run the search into an FPathStore, and hand the edge list to AdjustPathFindingEdges.
// DISHONORED(bringup): retail calls UNavigationHandle::FindPath(NULL, NULL, &m_RawPathFindingEdges) - Arkane added a
// third parameter, an output FPathStore, to FindPath / GeneratePath / PathCache_Empty (2013 rva 0x295b40; the tree has the
// reference two-parameter signature, and UNavigationHandle::PathCache is already an FPathStore). Passing no store is
// retail's own NULL case, which means "use this->PathCache", so the search is identical and the result is copied out of
// PathCache here. Changing the signature means touching GeneratePath and AddSuccessorEdgesForPoly in the 22,000-line
// UnNavigationMesh.cpp, which this package does not need.
UBOOL FArkComponentLocomotion::UpdatePathFindingEdges( const FVector& _StartLocation, const FVector& _EndLocation )
{
	m_PathFindingEdges.Reset();
	if( !m_pPawnOwner || !m_ReqMgr.HasRequest() )
	{
		return FALSE;
	}

	const FLOAT fFunnelMultiplier = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_fMaxFunnelRadiusMultiplier;
	const FLOAT fRadius = m_pPawnOwner->CylinderComponent ? m_pPawnOwner->CylinderComponent->CollisionRadius : 0.f;
	const FLOAT fJitter = 1.f + ( appFrand() * ( fFunnelMultiplier - 1.f ) );
	const FLOAT fMinDistanceFromWalls = fRadius * fJitter * 1.65f;

	DISLOCO_MARK( pf_enter )
	if( !m_pPawnOwner->SetupPathGoalsAndConstraints( _EndLocation, FALSE ) )
	{
		m_LastPFError = PATHERROR_NOPATHFOUND;
		GDisLocoPathsFailed++;
		return FALSE;
	}

	UNavigationHandle* pNavHandle = m_pPawnOwner->GetNavigationHandle();
	if( !pNavHandle )
	{
		// DISHONORED(layout): retail's EPathFindingError is NOT the reference one. The 2012 PDB has six entries
		// (..., MAXIMUMVISITEXCEEDED=3, NOPATHFOUND=4, INVALIDNAVMESH=5, MAX=6); EngineClasses.h has the reference's
		// seven (..., NOPATHFOUND=3, COMPUTEVALIDFINALDEST_FAIL=4, GETNEXTMOVELOCATION_FAIL=5, MOVETIMEOUT=6). The
		// values collide, so this package uses the tree's names and the difference is named in agentDN.md rather than
		// renumbering an enum the whole nav-mesh runtime reads.
		m_LastPFError = PATHERROR_NOPATHFOUND;
		GDisLocoPathsFailed++;
		return FALSE;
	}

	m_RawPathFindingEdges.EdgeList.Reset();
	pNavHandle->SetFinalDestination( _EndLocation );
	DISLOCO_MARK( pf_findpath )
	const UBOOL bFoundPath = pNavHandle->FindPath();
	DISLOCO_TRACE( pf_findpath_done, "found %i edges %i", (INT)bFoundPath, pNavHandle->PathCache.EdgeList.Num() )
	if( !bFoundPath )
	{
		m_LastPFError = (INT)pNavHandle->LastPathError;
		GDisLocoPathsFailed++;
		return FALSE;
	}

	m_RawPathFindingEdges.EdgeList = pNavHandle->PathCache.EdgeList;
	m_RawPathFindingEdges.m_vComputedDestination = _EndLocation;
	if( m_RawPathFindingEdges.EdgeList.Num() <= 0 )
	{
		// A path with no edges is retail's "start and goal are in the same poly": a straight line is the path, so the
		// search did succeed and BuildStraightPath will produce the two endpoints.
		GDisLocoPathsBuilt++;
		return TRUE;
	}

	AdjustPathFindingEdges( _StartLocation, _EndLocation, pNavHandle, m_RawPathFindingEdges, fMinDistanceFromWalls, m_PathFindingEdges );
	DISLOCO_TRACE( pf_adjusted, "adjusted edges %i", m_PathFindingEdges.Num() )
	GDisLocoPathsBuilt++;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x545910 (2012 0x5867e0, 126 bytes). It refuses a zero or inconsistent speed envelope,
// reserves the edge count plus twelve path points, and calls the funnel. The reservation is retail's: BuildStraightPath
// and AdjustStraightPath together never produce more than one point per edge plus the two endpoints plus the corners
// AdjustStraightPath inserts.
UBOOL FArkComponentLocomotion::DefaultPathBuilder( const FVector& _StartLocation, const FVector& _EndLocation, FLOAT _fStartSpeed, FLOAT _fDestinationSpeed, FLOAT _fMaxSpeed, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _PathPoints )
{
	if( _fMaxSpeed <= 0.f || _fDestinationSpeed < 0.f || _fMaxSpeed < _fDestinationSpeed )
	{
		return FALSE;
	}
	_PathPoints.Empty( _PathFindingEdges.Num() + 12 );
	DISLOCO_MARK( pb_enter )
	BuildStraightPath( _StartLocation, _EndLocation, _PathFindingEdges, _PathPoints );
	DISLOCO_TRACE( pb_done, "path points %i", _PathPoints.Num() )
	GDisLocoPathPointsBuilt += _PathPoints.Num();
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x546a90 (2012 0x587770, 1,222 bytes). The speed envelope the funnel is given comes from the
// active request: the start speed is what the pawn is doing now, the destination speed is the request's end speed index
// and the maximum is its max speed index. Retail then records the distance between the first two points, which is the
// "am I close enough to the next corner" yardstick the dynamics use everywhere.
UBOOL FArkComponentLocomotion::UpdateBuiltPathPoints()
{
	if( !m_ReqMgr.HasRequest() )
	{
		return FALSE;
	}
	const FArkCpntLocoRequestCommonProps& rCommon = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps;
	const FLOAT fDestinationSpeed = GetForwardSpeedOfSpeedMode( rCommon.m_EndSpeedIdx, m_CurModifierIdx ) * rCommon.m_fSpeedMultiplier;
	const FLOAT fMaxSpeed = GetForwardSpeedOfSpeedMode( rCommon.m_MaxSpeedIdx, m_CurModifierIdx ) * rCommon.m_fSpeedMultiplier;

	if( !DefaultPathBuilder( m_StartLocationUsedToAskPath, m_EndLocationUsedToAskPath,
			m_fCurMoveSpeed, Max( fDestinationSpeed, 0.f ), Max( fMaxSpeed, 0.f ),
			m_PathFindingEdges, m_PathPoints ) )
	{
		return FALSE;
	}
	if( m_PathPoints.Num() < 2 )
	{
		return FALSE;
	}

	m_CurPathPointIdx = INDEX_NONE;
	m_LastPathPoint = m_PathPoints( m_PathPoints.Num() - 1 ).m_Point;
	m_fDistBetweenPathPoints = ( m_PathPoints( 1 ).m_Point - m_PathPoints( 0 ).m_Point ).Size2D();
	m_bBuiltPathUpToDate = TRUE;
	m_bPathAtEnd = FALSE;
	m_bStartingNewPath = TRUE;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x541ee0 (2012 0x582eb0, 2,499 bytes). What it does: advance m_CurPathPointIdx while the
// pawn is within m_fIntermediatePointValidDist of the current corner, keep m_PathPoint / m_LastPathPoint / the two path
// yaws and the two path directions in step with that index, ignore the very first corner when the pawn is already inside
// m_fIgnoreFirstPathPointDist of it, and raise m_bPathAtEnd when the index reaches the last point.
// DISHONORED(bringup): retail's body also keeps the "turn ahead" bookkeeping the root-motion system needs
// (m_bMustTestPathAfterTurn, m_fBeforeTurnSquaredDist, m_fBeforeTurnDuration and the next-corner yaw prediction through
// ComputePathYawFromDist, 2013 rva 0x5428b0) - that is the turn-in-place half, which is not ported, so those four are
// left at their reset values. The corner advance, the two directions and the end flag are retail's.
void FArkComponentLocomotion::UpdatePathProperties()
{
	if( !m_bBuiltPathUpToDate || m_PathPoints.Num() < 2 || !m_pPawnOwner )
	{
		return;
	}

	const FLOAT fIgnoreFirst = m_pConfig ? m_pConfig->m_fIgnoreFirstPathPointDist : 0.f;
	const FLOAT fIntermediate = m_pConfig ? m_pConfig->m_fIntermediatePointValidDist : 0.f;

	if( m_CurPathPointIdx == INDEX_NONE )
	{
		m_bPathAtEnd = FALSE;
		m_CurPathPointIdx = 0;
		if( m_PathPoints.Num() > 2 )
		{
			const FVector ToSecond = m_PathPoints( 1 ).m_Point - m_CurPawnLocation;
			if( ( fIgnoreFirst * fIgnoreFirst ) >= ( ToSecond.X * ToSecond.X + ToSecond.Y * ToSecond.Y ) )
			{
				m_CurPathPointIdx = 1;
			}
		}
	}

	// Advance through every corner the pawn is already close enough to. Retail loops rather than stepping once, so a
	// single long frame cannot leave the pawn walking back to a corner it has passed.
	while( m_CurPathPointIdx < m_PathPoints.Num() - 1 )
	{
		const FVector ToCorner = m_PathPoints( m_CurPathPointIdx ).m_Point - m_CurPawnLocation;
		const FLOAT fSq2D = ToCorner.X * ToCorner.X + ToCorner.Y * ToCorner.Y;
		if( fSq2D > fIntermediate * fIntermediate )
		{
			break;
		}
		++m_CurPathPointIdx;
	}

	m_PathPoint = m_PathPoints( m_CurPathPointIdx ).m_Point;
	m_LastPathPoint = m_PathPoints( m_PathPoints.Num() - 1 ).m_Point;

	FVector CurDir = m_PathPoint - m_CurPawnLocation;
	CurDir.Z = 0.f;
	if( CurDir.SizeSquared() > KINDA_SMALL_NUMBER )
	{
		m_CurPathDir = CurDir.SafeNormal();
		m_CurPathYaw = m_CurPathDir.Rotation().Yaw;
	}

	if( m_CurPathPointIdx + 1 < m_PathPoints.Num() )
	{
		FVector NextDir = m_PathPoints( m_CurPathPointIdx + 1 ).m_Point - m_PathPoint;
		NextDir.Z = 0.f;
		if( NextDir.SizeSquared() > KINDA_SMALL_NUMBER )
		{
			m_NextPathDir = NextDir.SafeNormal();
			m_NextPathYaw = m_NextPathDir.Rotation().Yaw;
		}
	}
	else
	{
		m_NextPathDir = m_CurPathDir;
		m_NextPathYaw = m_CurPathYaw;
		m_bPathAtEnd = TRUE;
	}

	m_fDistBetweenPathPoints = ( m_CurPathPointIdx > 0 )
		? ( m_PathPoints( m_CurPathPointIdx ).m_Point - m_PathPoints( m_CurPathPointIdx - 1 ).m_Point ).Size2D()
		: ( m_PathPoints( 1 ).m_Point - m_PathPoints( 0 ).m_Point ).Size2D();
}

// DISHONORED(port): 2013 rva 0x545410 (2012 0x586320, 562 bytes). The path is dropped whole: every cross-pylon edge the
// pawn had reserved is released (APylon::RemoveActorMovingThruPylon on both sides, and the special-move edges' own
// IInterface_NavMeshPathObject::RemoveActorMovingThruEdge), the raw edge list is emptied, the corner index and both yaws
// go back to "none", the path-at-end flag is raised and the error is cleared.
// DISHONORED(bringup): the edge reservations are not released - AddActorMovingThruPylon is never called either, because
// this package does not reserve edges (that is the door / wall-of-light cost system, agent AD's
// ADisDoor::AddActorMovingThruEdge family). Releasing something never taken would be the wrong half to port.
void FArkComponentLocomotion::ResetPathProperties()
{
	m_RawPathFindingEdges.EdgeList.Reset();
	m_PathFindingEdges.Reset();
	m_PathPoints.Reset();
	m_CurPathPointIdx = INDEX_NONE;
	m_CurPathYaw = -200000;
	m_NextPathYaw = -200000;
	m_CurPathDir = FVector( 0.f, 0.f, 0.f );
	m_NextPathDir = FVector( 0.f, 0.f, 0.f );
	m_PathPoint = FVector( 0.f, 0.f, 0.f );
	m_bPathAtEnd = TRUE;
	m_bBuiltPathUpToDate = FALSE;
	m_bPathfindingUpToDate = FALSE;
	m_bPathIsDirty = FALSE;
	m_bStartingNewPath = FALSE;
	m_bTryToReturnOnNavMesh = FALSE;
	m_LastPFError = PATHERROR_MAX;
	m_fTryToReturnOnNavMeshDuration = 0.f;
	m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
}

// DISHONORED(port): 2013 rva 0x5428b0 (2012 0x583880, 614 bytes): the yaw the path will have _fDist further along, which
// the turn system uses to decide whether to turn in place now.
// DISHONORED(bringup): only the part the dynamics need - walk the corners forward accumulating distance and answer the
// direction of the segment the distance lands in.
INT FArkComponentLocomotion::ComputePathYawFromDist( FLOAT _fDist ) const
{
	if( m_PathPoints.Num() < 2 || m_CurPathPointIdx < 0 )
	{
		return m_CurPathYaw;
	}
	FLOAT fRemaining = _fDist;
	FVector From = m_CurPawnLocation;
	for( INT i = m_CurPathPointIdx; i < m_PathPoints.Num(); ++i )
	{
		FVector Segment = m_PathPoints( i ).m_Point - From;
		Segment.Z = 0.f;
		const FLOAT fLen = Segment.Size();
		if( fLen >= fRemaining || i == m_PathPoints.Num() - 1 )
		{
			return ( fLen > KINDA_SMALL_NUMBER ) ? Segment.Rotation().Yaw : m_CurPathYaw;
		}
		fRemaining -= fLen;
		From = m_PathPoints( i ).m_Point;
	}
	return m_CurPathYaw;
}

// DISHONORED(port): 2013 rva 0x549890 (2012 0x58a4d0, 3,285 bytes) - the path scheduler, and the largest function in the
// path unit. Retail's decision tree, in order:
//   1. resolve where the active request wants the pawn (ComputeTargetedLocationForPath);
//   2. if the pawn is not on a valid poly, try to find the nearest mesh location at 150, then 300, then further, and
//      spend at most m_fMaxDurationToReturnOnNavMesh doing it before giving up on the request;
//   3. if the pathfinding is up to date but the polyline is not, rebuild the polyline (UpdateBuiltPathPoints) - this is
//      the cheap half, and it is what runs when only the pawn moved;
//   4. otherwise decide whether a new search is needed at all: the destination moved by more than the repath coefficient
//      times the distance already walked, or the path was forced dirty, or the pawn left the path;
//   5. rate-limit it against the shared budget ms_TickCountBeforePath (at most one search per component per
//      (component count + 1) ticks - which is how a map with 26 NPCs does not run 26 A* searches in one frame);
//   6. run the search (UpdatePathFindingEdges) and, on success, rebuild the polyline; on failure raise
//      CPNT_LOCO_EVENT_PATHFINDING_FAILED to the asker exactly once per request.
// DISHONORED(bringup): step 2's escalating search radius is ported as retail's first two attempts (150 then 300) and then
// gives up rather than growing without bound; the "push period" interaction (HandlePushPeriod, 2013 rva 0x548b90) and the
// PointReachable short-circuit that skips the search when the goal is in line of sight are left out with their rvas.
void FArkComponentLocomotion::UpdatePathOfHigherPriorityRequest( FLOAT _fTimeStep, UBOOL _bForcePFComputation, const FVector* const _pForcedDestination )
{
	if( !m_ReqMgr.HasRequest() || !m_pPawnOwner )
	{
		return;
	}

	DISLOCO_MARK( sched_enter )
	const FVector Destination = _pForcedDestination ? *_pForcedDestination : ComputeTargetedLocationForPath( _fTimeStep, FALSE );
	DISLOCO_TRACE( sched_dest, "destination %s", *Destination.ToString() )

	// 2. the pawn has to be on the mesh to search from. On L_Tower_P 11 of the 26 spawned NPCs are NOT: the nav-mesh
	// census reports 15 standing on a poly, 8 more with a poly within 150 units and 3 with neither, because the spawners
	// put them on stairs, balconies and ledges the cooked mesh does not cover.
	FVector StartOnGround = GetPawnGroundLocation();
	DISLOCO_MARK( sched_startloc )
	if( !UpdateStartLocAndVerifyIfOnValidPoly( StartOnGround, NULL ) )
	{
		// Once this request has been told its start poly could not be found, stop trying. Retail bounds the attempt with
		// m_fMaxDurationToReturnOnNavMesh alone, which is not enough here because ResetPathProperties clears the duration
		// it is measured against - so the give-up decision and the reset chase each other. Measured: 5,774 failed searches
		// and 47,331 teleports in 22 seconds, which slows the whole game down. The sub-state above has had its
		// CPNT_LOCO_EVENT_PATHFINDING_FAILED and it is its business what to do next.
		if( m_CurReqWorkData.m_bPathfindAlreadyFailed )
		{
			return;
		}
		m_bTryToReturnOnNavMesh = TRUE;
		m_fTryToReturnOnNavMeshDuration += _fTimeStep;
		UBOOL bFound = FindNearestLocationOnNavMesh( NULL, 150.f, m_NearestLocationOnNavMesh );
		if( !bFound )
		{
			bFound = FindNearestLocationOnNavMesh( NULL, 300.f, m_NearestLocationOnNavMesh );
		}
		// One attempt per stretch off the mesh, not one per tick: the clock below is what decides when to give up, and a
		// teleport that did not help must not be retried 240 times a second while it runs down (measured: 15,339 teleports in
		// 39 seconds).
		if( bFound && !m_bTeleportDisabled && m_fTryToReturnOnNavMeshDuration <= ( _fTimeStep * 2.f ) )
		{
			// DISHONORED(port): retail's answer, and it is a teleport: m_bTeleportRequired is raised with m_TeleportLocation
			// set to the nearest mesh point and PreAsyncWorkTick moves the pawn there with UWorld::FarMoveActor
			// (2013 rva 0x54caa0's teleport branch), once CanBeTeleportedToLocationWithoutBeingSeen agrees.
			// DISHONORED(bringup): that visibility test (2013 rva 0x53e310, 769 bytes) is not ported, so the teleport is
			// taken without it. It is taken rather than simply searching from the mesh point because the A* reads its start
			// from FNavMeshPathParams::SearchStart, which the PAWN fills from its own feet - a search "from" a corrected
			// point still starts at the pawn, and every one of those searches fails.
			m_TeleportLocation = m_NearestLocationOnNavMesh;
			m_bTeleportRequired = TRUE;
		}
		const FLOAT fMaxDuration = m_pConfig ? m_pConfig->m_fMaxDurationToReturnOnNavMesh : 0.f;
		if( !bFound || m_fTryToReturnOnNavMeshDuration > fMaxDuration )
		{
			m_CurReqWorkData.m_bPathfindAlreadyFailed = TRUE;
			m_LastPFError = PATHERROR_STARTPOLYNOTFOUND;
			NotifyActiveRequest( CPNT_LOCO_EVENT_PATHFINDING_FAILED );
			ResetPathProperties();
		}
		return;
	}
	m_bTryToReturnOnNavMesh = FALSE;
	m_fTryToReturnOnNavMeshDuration = 0.f;
	m_bHasBeenOnNavMesh = TRUE;

	// 3. the search is still valid, only the polyline is stale.
	if( m_bPathfindingUpToDate && !m_bBuiltPathUpToDate )
	{
		if( UpdateBuiltPathPoints() )
		{
			m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
			return;
		}
	}

	// 4. does the destination still agree with what the last search was asked for?
	// A search that already failed for this request is not retried until something changes: the destination moves, the
	// request is replaced, or someone forces it. Without that clause a goal the mesh cannot reach is searched for every
	// tick the budget allows - measured at 20,411 failed searches in 46 seconds.
	UBOOL bNeedNewSearch = _bForcePFComputation || m_bForcePathComputation
		|| ( !m_bPathfindingUpToDate && !m_CurReqWorkData.m_bPathfindAlreadyFailed );
	if( !bNeedNewSearch )
	{
		const FLOAT fRepathCoef = m_pConfig ? m_pConfig->m_fRepathCoef : 0.f;
		const FLOAT fDestMoved = ( Destination - m_EndLocationUsedToAskPath ).Size2D();
		const FLOAT fWalked = ( m_CurPawnLocation - m_StartLocationUsedToAskPath ).Size2D();
		if( fDestMoved > ( fRepathCoef * Max( fWalked, 1.f ) ) )
		{
			bNeedNewSearch = TRUE;
		}
	}
	if( !bNeedNewSearch )
	{
		return;
	}

	// 5. the shared budget.
	if( ms_TickCountBeforePath > 0 && !_bForcePFComputation && !m_bForcePathComputation )
	{
		return;
	}
	ms_TickCountBeforePath = ms_ComponentCount + 1;
	m_bForcePathComputation = FALSE;

	// 6. the search.
	ResetPathProperties();
	m_StartLocationUsedToAskPath = StartOnGround;
	m_EndLocationUsedToAskPath = Destination;
	if( UpdatePathFindingEdges( m_StartLocationUsedToAskPath, m_EndLocationUsedToAskPath ) )
	{
		m_bPathfindingUpToDate = TRUE;
		if( UpdateBuiltPathPoints() )
		{
			if( !m_CurReqWorkData.m_bPathfindAlreadySucceed )
			{
				m_CurReqWorkData.m_bPathfindAlreadySucceed = TRUE;
				NotifyActiveRequest( CPNT_LOCO_EVENT_PATHFINDING_SUCCEED );
			}
		}
	}
	else if( !m_CurReqWorkData.m_bPathfindAlreadyFailed )
	{
		m_CurReqWorkData.m_bPathfindAlreadyFailed = TRUE;
		NotifyActiveRequest( CPNT_LOCO_EVENT_PATHFINDING_FAILED );
	}
	m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
}

// DISHONORED(port): 2013 rva 0x53df60 (2012 0x57f0b0)
FVector FArkComponentLocomotion::GetDirectionToNextPathPoint() const
{
	if( m_PathPoints.Num() < 1 || m_CurPathPointIdx < 0 || m_CurPathPointIdx >= m_PathPoints.Num() )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	FVector Dir = m_PathPoints( m_CurPathPointIdx ).m_Point - m_CurPawnLocation;
	Dir.Z = 0.f;
	return Dir.SafeNormal();
}
