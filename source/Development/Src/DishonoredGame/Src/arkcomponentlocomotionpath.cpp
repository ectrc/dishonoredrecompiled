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
#include "arknavmeshbuildutils.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "disdesirestructs.h"
#include "ibdiag.h"
#include "jddiag.h"

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

// DISHONORED(port): 2012 0x57f8a0 (474 bytes) / 2013 VA 0x93f240 (586 bytes; 0x53ed10 is inside another function). A line
// check straight down from ten units above the given point finds the ground; the poly under that ground point is looked up, and the point counts as "on a valid poly"
// only when the poly's own surface is within a step height below the ground AND below the top of the pawn's cylinder -
// which is what stops an NPC standing on a table from thinking it is on the floor's poly. Agent IB: the trace flags are retail's
// 0x30DF (TRACE_AllBlocking | TRACE_Visible, both databases), not TRACE_World, which misses every non-mover actor - the boat's
// deck the captain stands on after the ride, the floors under _4/_6/_8/_20 - and failed the start poly with the pawn on the mesh.
// DISHONORED(bringup): 2013 also returns the pawn's navigation handle through a third argument and clears a caller flag when
// the start is the pawn's own location; the 2012 signature is kept (no caller here reads either).
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
	if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, TRACE_AllBlocking | TRACE_Visible, FVector( 0.f, 0.f, 0.f ) ) )
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

// DISHONORED(port): 2013 rva 0x546770 (2012 0x587460, 377 bytes; trace flags 0x30DF in both, agent IB). The spawner's "is there room here" test and
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
	if( GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, TRACE_AllBlocking | TRACE_Visible, FVector( 0.f, 0.f, 0.f ) ) )
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
	// agent JD: retail draws the jitter too - 2012 0x5875e0: radius x (rand() x (multiplier - 1) / 32767 + 1) x 1.65, which is appFrand()
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
		if( DisGCLocoDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): gadiag pathfail %s: FindPath from %s to %s failed, error %d, anchor %s, dist %.0f"),
				*m_pPawnOwner->GetName(), *_StartLocation.ToString(), *_EndLocation.ToString(), (INT)pNavHandle->LastPathError,
				pNavHandle->AnchorPoly ? TEXT("set") : TEXT("none"), ( _EndLocation - _StartLocation ).Size() );
		}
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

// DISHONORED(port, agent JD): 2013 rva 0x541ee0 (2012 0x582eb0, 2,499 bytes), whole. m_CurPathPointIdx is the corner the pawn walks
// to, m_PathPoint that corner, m_CurPathDir / m_CurPathYaw the direction of the segment that ends there (not the pawn -> corner
// direction) and m_NextPathDir / m_NextPathYaw the one that leaves it. On a new polyline (index INDEX_NONE) the first corner is
// point 1, or point 2 when point 1 is within m_fIgnoreFirstPathPointDist; then, in a loop:
//   - the corner is left behind when the pawn is within m_fIntermediatePointValidDist of it (2D), or the corner lies behind the
//     pawn along the current segment, or the next corner can be seen through the corner's navmesh edge (the next corner lies
//     between the pawn -> left end and the pawn -> right end of the edge the corner was pulled in from; a corner without an
//     edge is its own two ends) - otherwise the loop ends;
//   - after one corner has been passed this tick, the following one is passed too as long as the corner after it is reachable
//     from the pawn in a straight line (UNavigationHandle::PointReachable with the pawn's extent);
//   - passing a corner makes the next segment the current one, advances the index, recomputes the next segment (or, at the
//     last point, raises m_bPathAtEnd and keeps the current one as the next) and measures m_fDistBetweenPathPoints (3D).
// This replaces DN's "advance within the intermediate distance only, aim the current direction at the corner from the pawn".
// The turn-ahead fields the brief placed here are not in this function in either database: m_bMustTestPathAfterTurn is the
// path scheduler's (UpdatePathOfHigherPriorityRequest, agent IK2) and m_fBeforeTurnSquaredDist / m_fBeforeTurnDuration are
// RequestTurn's and UpdateTurn's (agent GK).
void FArkComponentLocomotion::UpdatePathProperties()
{
	if( !m_bBuiltPathUpToDate )
	{
		return;
	}
	UBOOL bPassedACorner = FALSE;
	for( ;; )
	{
		if( m_CurPathPointIdx == INDEX_NONE )
		{
			m_bPathAtEnd = FALSE;
			m_CurPathPointIdx = 0;
			if( m_PathPoints.Num() > 2 )
			{
				const FVector& Second = m_PathPoints( 1 ).m_Point;
				const FLOAT fDX = Second.X - m_CurPawnLocation.X;
				const FLOAT fDY = Second.Y - m_CurPawnLocation.Y;
				if( m_pConfig->m_fIgnoreFirstPathPointDist * m_pConfig->m_fIgnoreFirstPathPointDist >= fDX * fDX + fDY * fDY )
				{
					m_CurPathPointIdx = 1;
				}
			}
			m_PathPoint = m_PathPoints( m_CurPathPointIdx ).m_Point;
			const FVector& Next = m_PathPoints( m_CurPathPointIdx + 1 ).m_Point;
			m_CurPathDir = FVector( Next.X - m_PathPoint.X, Next.Y - m_PathPoint.Y, 0.f ).SafeNormal();
			m_CurPathYaw = appTrunc( appAtan2( m_CurPathDir.Y, m_CurPathDir.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
		}
		else
		{
			if( m_bPathAtEnd )
			{
				return;
			}
			if( !bPassedACorner )
			{
				const FArkCpntLocoPathPoint& rCorner = m_PathPoints( m_CurPathPointIdx );
				const FVector ToCorner = rCorner.m_Point - m_CurPawnLocation;
				const FLOAT fSq2D = ToCorner.X * ToCorner.X + ToCorner.Y * ToCorner.Y;
				if( m_pConfig->m_fIntermediatePointValidDist * m_pConfig->m_fIntermediatePointValidDist < fSq2D )
				{
					const FVector ToCornerDir = FVector( ToCorner.X, ToCorner.Y, 0.f ).SafeNormal();
					if( ( m_CurPathDir | ToCornerDir ) >= 0.f )
					{
						const UBOOL bHasEdge = rCorner.m_EdgeIndex != INDEX_NONE;
						const FVector& EdgeLeft = bHasEdge ? m_PathFindingEdges( rCorner.m_EdgeIndex ).m_EdgeLeftPoint : rCorner.m_Point;
						const FVector& EdgeRight = bHasEdge ? m_PathFindingEdges( rCorner.m_EdgeIndex ).m_EdgeRightPoint : rCorner.m_Point;
						const FVector& Next = m_PathPoints( m_CurPathPointIdx + 1 ).m_Point;
						const FVector& Pawn = m_pPawnOwner->Location;
						const UBOOL bRightOfLeftEnd = ( ( EdgeLeft.Y - Pawn.Y ) * ( Next.X - Pawn.X ) - ( Next.Y - Pawn.Y ) * ( EdgeLeft.X - Pawn.X ) ) <= 0.f;
						const UBOOL bLeftOfRightEnd = ( ( Next.Y - Pawn.Y ) * ( EdgeRight.X - Pawn.X ) - ( EdgeRight.Y - Pawn.Y ) * ( Next.X - Pawn.X ) ) <= 0.f;
						if( !bRightOfLeftEnd || !bLeftOfRightEnd )
						{
							return;
						}
					}
				}
			}
			else
			{
				bPassedACorner = FALSE;
				UNavigationHandle* pNavHandle = m_pPawnOwner->GetNavigationHandle();
				if( !pNavHandle->PointReachable( m_PathPoints( m_CurPathPointIdx + 1 ).m_Point, FVector( 0.f, 0.f, 0.f ), TRUE, NULL, NULL ) )
				{
					return;
				}
			}
			m_CurPathDir = m_NextPathDir;
			m_CurPathYaw = m_NextPathYaw;
		}
		++m_CurPathPointIdx;
		if( m_CurPathPointIdx != m_PathPoints.Num() - 1 )
		{
			const FVector& Corner = m_PathPoints( m_CurPathPointIdx ).m_Point;
			const FVector& Next = m_PathPoints( m_CurPathPointIdx + 1 ).m_Point;
			m_NextPathDir = FVector( Next.X - Corner.X, Next.Y - Corner.Y, 0.f ).SafeNormal();
			m_NextPathYaw = appTrunc( appAtan2( m_NextPathDir.Y, m_NextPathDir.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
			bPassedACorner = TRUE;
		}
		else
		{
			m_bPathAtEnd = TRUE;
			m_NextPathDir = m_CurPathDir;
			m_NextPathYaw = m_CurPathYaw;
		}
		const FVector PreviousPathPoint = m_PathPoint;
		m_PathPoint = m_PathPoints( m_CurPathPointIdx ).m_Point;
		m_fDistBetweenPathPoints = ( m_PathPoint - PreviousPathPoint ).Size();
		JDDiagCorner();
	}
}

// DISHONORED(port): 2013 rva 0x545410 (2012 0x586320, 562 bytes); agent IK2: the flag word is `& 0xFFFFFF01 | 0x20` - bits 0x2..0x80
// cleared, the path-at-end bit raised, m_bTryToReturnOnNavMesh (0x1) KEPT (HEAD cleared it). The path is dropped whole: every cross-pylon edge the
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
	m_bForcePathComputation = FALSE;
	m_bMustTestPathAfterTurn = FALSE;
	m_LastPFError = PATHERROR_MAX;
	m_fTryToReturnOnNavMeshDuration = 0.f;
	m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
}

// DISHONORED(port, agent JD): 2013 rva 0x5428b0 (2012 0x583880, 614 bytes), whole: the yaw of the path _fDist further along it,
// which EvaluateStaticTurn compares with the move yaw. At the path's end, or while the distance stays within the stretch from the
// pawn to the current corner, it is m_CurPathYaw (the current segment's); otherwise the 2D direction of the segment the distance
// ends in, the last one when the path is shorter. DN's version answered the pawn -> corner yaw for the first stretch.
// DISHONORED(bringup): a negative index answers m_CurPathYaw (retail's array access would assert).
INT FArkComponentLocomotion::ComputePathYawFromDist( FLOAT _fDist ) const
{
	if( m_bPathAtEnd || m_CurPathPointIdx < 0 )
	{
		return m_CurPathYaw;
	}
	INT PathPointIdx = m_CurPathPointIdx;
	FLOAT fLeftDist = _fDist;
	FLOAT fFromX = m_CurPawnLocation.X;
	FLOAT fFromY = m_CurPawnLocation.Y;
	FLOAT fToNextX = 0.f;
	FLOAT fToNextY = 0.f;
	while( PathPointIdx < m_PathPoints.Num() )
	{
		const FVector& PathPoint = m_PathPoints( PathPointIdx ).m_Point;
		fToNextX = PathPoint.X - fFromX;
		fToNextY = PathPoint.Y - fFromY;
		fLeftDist -= appSqrt( fToNextX * fToNextX + fToNextY * fToNextY );
		if( fLeftDist <= 0.f )
		{
			break;
		}
		fFromX = PathPoint.X;
		fFromY = PathPoint.Y;
		++PathPointIdx;
	}
	if( PathPointIdx == m_CurPathPointIdx )
	{
		return m_CurPathYaw;
	}
	return appTrunc( appAtan2( fToNextY, fToNextX ) * 10430.219f * 2.f + 0.5f ) >> 1;
}

// DISHONORED(port, agent IK2): 2012 0x58a4d0 (3,132 B) / 2013 VA 0x949890 (3,285 B) - the path scheduler, retail's m_LastPFError
// state machine (decompiles build/agentIK/decomp2012/58a4d0_*, build/agentIK/decomp2013/949890_*).
//   1. A path that is up to date with no error is re-checked: stuck in a wall, or the targeted location moved (two cylinder
//      heights of Z ignored) by more than m_fRepathCoef x the distance left to the old end (always, once the path is at its
//      end), or - after a turn has lined the move up with the current path point - that point has drifted more than 5 uu
//      sideways off the path yaw and is no longer reachable: the path is marked for a new search (bit 0x8).
//   2. By error state: start poly / anchor pylon not found - while walking back onto the mesh the start is re-verified every
//      tick (search as soon as it holds, give up after m_fMaxDurationToReturnOnNavMesh), otherwise the error clears when the
//      pawn's ground point moved; goal poly / no path - the error clears when the targeted location left the request's end
//      threshold, or the goal is now on the mesh; no error - a pawn without an up-to-date path re-verifies its start and,
//      off the mesh, gets the nearest mesh point (150 uu, then doubling for a pawn that has had a path) and the teleport
//      only when it has had a path (m_bHasBeenOnNavMesh).
//   3. The search, when one is due and the shared budget allows it (or forced, or the path was at its end): the end is the
//      targeted location one tick ahead; a goal reachable in a straight line takes no A*, else UpdatePathFindingEdges, whose
//      failure leaves the navigation handle's error in m_LastPFError. A search that succeeded clears the error and raises
//      m_bHasBeenOnNavMesh.
//   4. The polyline is rebuilt when the search is newer than it; the asker is told PATHFINDING_SUCCEED once per request when
//      it is, and PATHFINDING_FAILED once per request on a goal error, or a start error with no walk back left.
// DISHONORED(bringup): EPathFindingError is the tree's (EngineClasses.h), not retail's (2012 PDB: MAXIMUMVISITEXCEEDED 3,
// NOPATHFOUND 4, INVALIDNAVMESH 5, MAX 6): retail's goal group {GOALPOLYNOTFOUND, MAXIMUMVISITEXCEEDED, NOPATHFOUND} is the
// tree's {GOALPOLYNOTFOUND, NOPATHFOUND}; the tree's path-following errors (4..6) fall to the default like retail's
// INVALIDNAVMESH. Retail's PointReachable (agent JD: Arkane's, with its out-list) hands the scheduler the edges the straight line
// crosses, which become the raw edges; retail then gives every crossed special-move / cross-pylon edge to its path object or
// pylon (vtable +16 / +956) - that reservation system is unported (see ResetPathProperties), so no edge is reserved. Retail's radius doubling has no upper bound; this stops at 2400 uu.
void FArkComponentLocomotion::UpdatePathOfHigherPriorityRequest( FLOAT _fTimeStep, UBOOL _bForcePFComputation, const FVector* const _pSpecificStartLoc )
{
	if( !m_ReqMgr.HasRequest() || !m_pPawnOwner || !m_pConfig )
	{
		return;
	}
	const FArkCpntLocoRequestCommonProps& rCommon = m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps;
	const FLOAT fCollisionHeight = m_pPawnOwner->CylinderComponent ? m_pPawnOwner->CylinderComponent->CollisionHeight : 0.f;
	UBOOL bRepath = FALSE;
	UBOOL bCloseToEndRepath = FALSE;

	if( m_bPathfindingUpToDate && m_LastPFError == PATHERROR_MAX )
	{
		if( m_bStuckedInAWall )
		{
			bRepath = TRUE;
		}
		else
		{
			const FLOAT fTwoHeights = m_pPawnOwner->GetCylinderExtent().Z * 2.f;
			FVector TargetMove = ComputeTargetedLocationForPath( _fTimeStep, FALSE ) - m_EndLocationUsedToAskPath;
			if( fTwoHeights > Abs( TargetMove.Z ) )
			{
				TargetMove.Z = 0.f;
			}
			const FLOAT fTargetMoved = TargetMove.Size();
			if( Abs( fTargetMoved ) >= 0.1f )
			{
				if( m_bPathAtEnd )
				{
					bRepath = TRUE;
					bCloseToEndRepath = TRUE;
				}
				else
				{
					FVector ToOldEnd = m_EndLocationUsedToAskPath - m_pPawnOwner->Location;
					if( fTwoHeights > Abs( ToOldEnd.Z ) )
					{
						ToOldEnd.Z = 0.f;
					}
					if( ( fTargetMoved / ToOldEnd.Size() ) > m_pConfig->m_fRepathCoef )
					{
						bRepath = TRUE;
					}
				}
			}
			if( !m_bInertiaDisabled && !bRepath && m_CurPathPointIdx != INDEX_NONE && !m_CurMove.IsZero() )
			{
				const FVector ToPathPoint = m_PathPoints( m_CurPathPointIdx ).m_Point - m_CurPawnLocation;
				if( !ToPathPoint.IsZero() )
				{
					const INT PathPointYaw = appTrunc( appAtan2( ToPathPoint.Y, ToPathPoint.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
					const INT MoveYaw = appTrunc( appAtan2( m_CurMove.Y, m_CurMove.X ) * 10430.219f * 2.f + 0.5f ) >> 1;
					const INT TurnLeft = (INT)(SWORD)(WORD)( MoveYaw - PathPointYaw );
					if( !m_bMustTestPathAfterTurn )
					{
						m_bMustTestPathAfterTurn = ( PathPointYaw != MoveYaw && TurnLeft > 3 ) ? TRUE : FALSE;
					}
					else if( PathPointYaw == MoveYaw || TurnLeft <= 3 )
					{
						const INT PathYawOff = (INT)(SWORD)(WORD)( m_CurPathYaw - PathPointYaw );
						if( Abs( ToPathPoint.Size2D() * appSin( (FLOAT)PathYawOff * 0.000095873802f ) ) > 5.f )
						{
							UNavigationHandle* pNavHandle = m_pPawnOwner->GetNavigationHandle();
							if( pNavHandle && !pNavHandle->PointReachable( m_PathPoints( m_CurPathPointIdx ).m_Point, m_CurPawnLocation, TRUE, NULL, NULL ) )
							{
								bRepath = TRUE;
							}
						}
						m_bMustTestPathAfterTurn = FALSE;
					}
				}
			}
		}
		if( bRepath )
		{
			m_bTryToReturnOnNavMesh = FALSE;
			m_bPathIsDirty = TRUE;
			m_LastPFError = PATHERROR_MAX;
			m_fTryToReturnOnNavMeshDuration = 0.f;
		}
	}

	const UBOOL bSearchDue = m_LastPFError == PATHERROR_MAX && ( !m_bPathfindingUpToDate || m_bPathIsDirty );
	UBOOL bSearchNow = FALSE;
	UBOOL bSkipSearch = FALSE;
	const TCHAR* Reason = TEXT("state");
	switch( m_LastPFError )
	{
	case PATHERROR_STARTPOLYNOTFOUND:
	case PATHERROR_ANCHORPYLONNOTFOUND:
		if( m_bTryToReturnOnNavMesh )
		{
			m_fTryToReturnOnNavMeshDuration += _fTimeStep;
			if( UpdateStartLocAndVerifyIfOnValidPoly( m_StartLocationUsedToAskPath, _pSpecificStartLoc ) )
			{
				m_LastPFError = PATHERROR_MAX;
				bSearchNow = TRUE;
				Reason = TEXT("back on the mesh");
			}
			else
			{
				m_bTryToReturnOnNavMesh = ( m_pConfig->m_fMaxDurationToReturnOnNavMesh > m_fTryToReturnOnNavMeshDuration ) ? TRUE : FALSE;
				Reason = TEXT("walking back to the mesh");
			}
		}
		else
		{
			const FVector Ground = m_pPawnOwner->Location - FVector( 0.f, 0.f, fCollisionHeight );
			if( !m_StartLocationUsedToAskPath.Equals( Ground, 0.1f ) )
			{
				m_LastPFError = PATHERROR_MAX;
				Reason = TEXT("start error cleared: the ground point moved");
			}
			else
			{
				Reason = TEXT("start error, ground point unchanged");
			}
		}
		break;

	case PATHERROR_GOALPOLYNOTFOUND:
	case PATHERROR_NOPATHFOUND:
	{
		FVector Target = ComputeTargetedLocationForPath( _fTimeStep, FALSE );
		Target.Z = m_EndLocationUsedToAskPath.Z;
		if( !m_EndLocationUsedToAskPath.Equals( Target, rCommon.m_fEndLocationThreshold )
			|| ( m_LastPFError == PATHERROR_GOALPOLYNOTFOUND && ArkIsPointOnNavMesh( Target ) ) )
		{
			m_LastPFError = PATHERROR_MAX;
			Reason = TEXT("goal error cleared: the target moved");
		}
		else
		{
			Reason = TEXT("goal error, target unchanged");
		}
		break;
	}

	case PATHERROR_MAX:
	{
		if( ( m_bPathfindingUpToDate && !m_bPathIsDirty ) || UpdateStartLocAndVerifyIfOnValidPoly( m_StartLocationUsedToAskPath, _pSpecificStartLoc ) )
		{
			Reason = bSearchDue ? TEXT("search due") : TEXT("path up to date");
			break;
		}
		bSkipSearch = TRUE;
		m_LastPFError = PATHERROR_STARTPOLYNOTFOUND;
		const UBOOL bFound = FindNearestLocationOnNavMesh( NULL, 150.f, m_NearestLocationOnNavMesh );
		if( !bFound )
		{
			if( !m_bTeleportDisabled && m_bHasBeenOnNavMesh && !m_bTeleportRequired )
			{
				ResetPathProperties();
				FLOAT fSearchDist = 300.f;
				while( !FindNearestLocationOnNavMesh( NULL, fSearchDist, m_NearestLocationOnNavMesh ) && fSearchDist < 2400.f )
				{
					fSearchDist += fSearchDist;
				}
				m_bTeleportRequired = TRUE;
				m_TeleportLocation = m_NearestLocationOnNavMesh + FVector( 0.f, 0.f, fCollisionHeight );
			}
		}
		else
		{
			m_bTryToReturnOnNavMesh = TRUE;
			if( !m_bTeleportDisabled && m_bHasBeenOnNavMesh )
			{
				m_bTeleportRequired = TRUE;
				m_TeleportLocation = m_NearestLocationOnNavMesh + FVector( 0.f, 0.f, fCollisionHeight );
			}
		}
		IKDiagStartPoly( m_StartLocationUsedToAskPath, bFound, m_bTeleportRequired );
		Reason = TEXT("off the mesh");
		break;
	}

	default:
		Reason = TEXT("error state without a rule");
		break;
	}

	if( !bSkipSearch && ( bSearchNow || bSearchDue ) )
	{
		if( _bForcePFComputation || bCloseToEndRepath || ms_TickCountBeforePath == 0 )
		{
			m_bTeleportRequired = FALSE;
			UBOOL bSearched = FALSE;
			m_EndLocationUsedToAskPath = ComputeTargetedLocationForPath( _fTimeStep, TRUE );
			UNavigationHandle* pNavHandle = m_pPawnOwner->GetNavigationHandle();
			TArray<FNavMeshEdgeBase*> CrossedEdges;
			if( pNavHandle && pNavHandle->PointReachable( m_EndLocationUsedToAskPath, FVector( 0.f, 0.f, 0.f ), TRUE, &CrossedEdges, NULL ) )
			{
				ResetPathProperties();
				m_bPathfindingUpToDate = TRUE;
				m_RawPathFindingEdges.EdgeList = CrossedEdges;
				m_PathFindingEdges.Reset();
				bSearched = TRUE;
				Reason = TEXT("searched: straight line");
			}
			else if( _bForcePFComputation || ms_TickCountBeforePath == 0 )
			{
				ResetPathProperties();
				ms_TickCountBeforePath = ms_ComponentCount + 1;
				m_bPathfindingUpToDate = UpdatePathFindingEdges( m_StartLocationUsedToAskPath, m_EndLocationUsedToAskPath );
				bSearched = TRUE;
				if( !m_bPathfindingUpToDate && pNavHandle )
				{
					m_LastPFError = (INT)pNavHandle->LastPathError;
				}
				Reason = m_bPathfindingUpToDate ? TEXT("searched: A* path") : TEXT("searched: A* failed");
			}
			else
			{
				Reason = TEXT("search due, waiting for the budget");
			}
			if( m_bPathfindingUpToDate && bSearched )
			{
				m_LastPFError = PATHERROR_MAX;
				m_bHasBeenOnNavMesh = TRUE;
			}
		}
		else
		{
			Reason = TEXT("search due, waiting for the budget");
		}
	}

	if( m_bPathfindingUpToDate && !m_bBuiltPathUpToDate && UpdateBuiltPathPoints() )
	{
		m_LastPathPoint = m_PathPoints( m_PathPoints.Num() - 1 ).m_Point;
		m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
	}

	if( m_CurReqWorkData.m_bPathfindAlreadySucceed || !m_bBuiltPathUpToDate )
	{
		if( !m_CurReqWorkData.m_bPathfindAlreadyFailed )
		{
			const UBOOL bStartFailed = !m_bTryToReturnOnNavMesh && ( m_LastPFError == PATHERROR_STARTPOLYNOTFOUND || m_LastPFError == PATHERROR_ANCHORPYLONNOTFOUND );
			const UBOOL bGoalFailed = m_LastPFError == PATHERROR_GOALPOLYNOTFOUND || m_LastPFError == PATHERROR_NOPATHFOUND;
			if( ( bStartFailed || bGoalFailed ) && rCommon.m_LocoEventCallback )
			{
				m_CurReqWorkData.m_bPathfindAlreadyFailed = TRUE;
				m_CurReqWorkData.m_bPathfindAlreadySucceed = FALSE;
				NotifyActiveRequest( CPNT_LOCO_EVENT_PATHFINDING_FAILED );
				Reason = TEXT("told the asker: pathfinding failed");
			}
		}
	}
	else if( rCommon.m_LocoEventCallback )
	{
		m_CurReqWorkData.m_bPathfindAlreadySucceed = TRUE;
		m_CurReqWorkData.m_bPathfindAlreadyFailed = FALSE;
		NotifyActiveRequest( CPNT_LOCO_EVENT_PATHFINDING_SUCCEED );
		Reason = TEXT("told the asker: pathfinding succeeded");
	}
	IKDiagScheduler( Reason, m_EndLocationUsedToAskPath );
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
