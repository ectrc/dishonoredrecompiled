// DishonoredGame/src/arkpathbuildutils.cpp
// DISHONORED(port): agent DN. The eight free functions that turn the nav-mesh edge list an A* produced into a polyline a
// pawn can walk. Retail's unit is Engine/Src/arkpathbuildutils.cpp (8 functions, 6,530 bytes); it is here rather than in
// Engine only because it shares a header with the locomotion component (see that header's banner).
//
// Two passes, and they do different jobs:
//   AdjustPathFindingEdges  pulls both ends of every crossed edge inwards by the pawn's radius, so a corner the funnel
//                           picks is a place the pawn's cylinder actually fits, and it asks the mesh whether the pulled-in
//                           point is still reachable before using it.
//   BuildStraightPath       runs Arkane's funnel over those adjusted edges and then AdjustStraightPath over the result.
// The funnel is the classic "simple stupid funnel": an apex plus a left and a right bound that tighten as edges are
// consumed, and a corner is emitted whenever the bounds cross.
// Agent IO (PHASE23 IO): FindFirstEdgeIndex, AdjustPathFindingEdges, MinimizeStraightPath_FunnelAlgorithm and AdjustStraightPath
// are retail's whole (2012 0xb4d10 / 0xb54d0 / 0xb5cf0 / 0xb5fe0). DN's left/right winding was the mirror of retail's while its
// funnel step (ApplyFunnelAlgorithm_FindTurn) is retail's, so every path zigzagged between the two ends of its edges - the
// "odd routes" (agentIO.md). Agent IO2: IsEdgeBorderExtentReachable asks Arkane's PointReachableZeroExtent (2012 0x2a9ba0).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

/** The distance below which two corners count as the same point (retail's literal). */
static const FLOAT GArkPathSamePointEpsilon = 0.0001f;
/** The distance below which two edge ends count as shared (retail's literal in ApplyFunnelAlgorithm_FindTurn). */
static const FLOAT GArkPathSharedVertEpsilon = 0.1f;
/** How far along a too-narrow edge each end is moved when both walls have to give (retail's literal). */
static const FLOAT GArkPathNarrowEdgeFraction = 0.44999999f;

/** 2D cross product of (B-A) x (C-A); > 0 means C is left of AB. Retail writes it inline everywhere. */
static inline FLOAT ArkPathCross2D( const FVector& _A, const FVector& _B, const FVector& _C )
{
	return ( _B.Y - _A.Y ) * ( _C.X - _A.X ) - ( _C.Y - _A.Y ) * ( _B.X - _A.X );
}

static inline UBOOL ArkPathSamePoint2D( const FVector& _A, const FVector& _B, FLOAT _fEpsilon )
{
	return Abs( _A.X - _B.X ) < _fEpsilon && Abs( _A.Y - _B.Y ) < _fEpsilon;
}

// DISHONORED(port): 2012 rva 0xacd50, 319 bytes. The pulled-in corner is tested twice, once in each direction, between the
// edge centre and the corner, each shifted by the perpendicular probe: both navmesh walks must reach (agent IO2: Arkane's
// UNavigationHandle::PointReachableZeroExtent, which replaced the reference PointReachable stand-in).
UBOOL IsEdgeBorderExtentReachable( const FVector& _EdgeBorder, const FVector& _EdgeCenter, const FVector& _BorderExtent, const FVector& _PerpExtent, UNavigationHandle* _pNavHandle )
{
	const FVector Border = _EdgeBorder + _BorderExtent;
	if( !_pNavHandle->PointReachableZeroExtent( Border - _PerpExtent, _EdgeCenter - _PerpExtent, NULL, NULL, FALSE ) )
	{
		return FALSE;
	}
	return _pNavHandle->PointReachableZeroExtent( Border + _PerpExtent, _EdgeCenter + _PerpExtent, NULL, NULL, FALSE );
}

// DISHONORED(port): 2012 rva 0xb5110, 949 bytes. Four cases, in retail's order:
//   both walls give and the edge is narrower than twice the pull-in  -> each end moves 45 % of the edge length inwards
//   the left wall gives and the edge is narrower than the pull-in    -> both ends move by the full pull-in vector
//   the right wall gives and the edge is narrower than the pull-in   -> both ends move by minus the pull-in vector
//   otherwise                                                        -> each end moves only if its own wall gives
// The signed "adjusted distance" it stores with each end is how far that end was moved and in which direction, and it is
// what AdjustStraightPath reads to put the corner back on the real edge.
void AddAdjustedPathFindingEdge( const FVector& _LeftVert, const FVector& _RightVert, const FVector& _EdgeDir, FLOAT _fEdgeSize, UBOOL _bAdjustOnLeft, UBOOL _bAdjustOnRight, const FVector& _PerpExtent, TArray<FArkCpntLocoPathFindingEdge>& _AdjustedPathFindingEdges )
{
	const FLOAT fAdjustExtentSize = _PerpExtent.Size();
	FArkCpntLocoPathFindingEdge NewEdge;

	if( _bAdjustOnLeft && _bAdjustOnRight && ( fAdjustExtentSize * 2.f ) >= _fEdgeSize )
	{
		const FVector Shrink = _EdgeDir * GArkPathNarrowEdgeFraction;
		NewEdge.m_EdgeLeftPoint					= _LeftVert + Shrink;
		NewEdge.m_fEdgeLeftAdjustedDistance		= _fEdgeSize * GArkPathNarrowEdgeFraction;
		NewEdge.m_EdgeRightPoint				= _RightVert - Shrink;
		NewEdge.m_fEdgeRightAdjustedDistance	= -( _fEdgeSize * GArkPathNarrowEdgeFraction );
	}
	else if( _bAdjustOnLeft && fAdjustExtentSize >= _fEdgeSize )
	{
		NewEdge.m_EdgeLeftPoint					= _LeftVert + _PerpExtent;
		NewEdge.m_fEdgeLeftAdjustedDistance		= fAdjustExtentSize;
		NewEdge.m_EdgeRightPoint				= _RightVert + _PerpExtent;
		NewEdge.m_fEdgeRightAdjustedDistance	= fAdjustExtentSize;
	}
	else if( _bAdjustOnRight && fAdjustExtentSize >= _fEdgeSize )
	{
		NewEdge.m_EdgeLeftPoint					= _LeftVert - _PerpExtent;
		NewEdge.m_fEdgeLeftAdjustedDistance		= -fAdjustExtentSize;
		NewEdge.m_EdgeRightPoint				= _RightVert - _PerpExtent;
		NewEdge.m_fEdgeRightAdjustedDistance	= -fAdjustExtentSize;
	}
	else
	{
		NewEdge.m_EdgeLeftPoint					= _bAdjustOnLeft  ? ( _LeftVert  + _PerpExtent ) : _LeftVert;
		NewEdge.m_fEdgeLeftAdjustedDistance		= _bAdjustOnLeft  ? fAdjustExtentSize : 0.f;
		NewEdge.m_EdgeRightPoint				= _bAdjustOnRight ? ( _RightVert - _PerpExtent ) : _RightVert;
		NewEdge.m_fEdgeRightAdjustedDistance	= _bAdjustOnRight ? -fAdjustExtentSize : 0.f;
	}
	_AdjustedPathFindingEdges.AddItem( NewEdge );
}

// The poly two consecutive edges of the path share (retail inlines this in both functions below).
// DISHONORED(bringup): retail reads the centre of a NULL poly when two edges share none; the callers here use the edge's own
// P0 as the centre instead (the cross is then 0: left is P0).
static FNavMeshPolyBase* ArkPathSharedPoly( FNavMeshEdgeBase* _pEdgeA, FNavMeshEdgeBase* _pEdgeB )
{
	FNavMeshPolyBase* pPoly = _pEdgeA->GetPoly0();
	FNavMeshPolyBase* pPolyB0 = _pEdgeB->GetPoly0();
	FNavMeshPolyBase* pPolyB1 = _pEdgeB->GetPoly1();
	if( pPoly == pPolyB0 )
	{
		return pPoly;
	}
	if( pPoly != pPolyB1 )
	{
		pPoly = _pEdgeA->GetPoly1();
	}
	return ( pPoly == pPolyB0 || pPoly == pPolyB1 ) ? pPoly : NULL;
}

// DISHONORED(port): agent IO, retail whole: 2012 rva 0xb4d10 (1,024 bytes). Walks the edges from the first. An edge's "left"
// vert (retail's name; in Unreal's left-handed frame it is the vert on the negative side of the 2D cross product) is P0 when
// the cross of the two verts about the centre of the poly beyond the edge is <= 0, or - on the last edge - the vert on that
// side of the start->end line (both verts on one side: by retail's DistanceToLine). The first edge whose left->right line has
// the start point on its positive side is the first one not yet crossed; its index and the left-is-P0 flag are returned (all
// crossed: the edge count). DN's version always answered edge 0 with its own winding rule, the mirror of retail's, which made
// the funnel zigzag between the two ends of every edge (agent IO, -iodiag path).
INT FindFirstEdgeIndex( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, const FPathStore& _PathFindingEdges, UBOOL& _rbFirstEdgeLeftIsP0 )
{
	INT RemainingEdges = _PathFindingEdges.EdgeList.Num();
	INT FirstEdgeIndex = 0;
	FNavMeshEdgeBase* pEdge = _PathFindingEdges.EdgeList( 0 );
	FVector EdgeP0 = pEdge->GetVertLocation( 0 );
	FVector EdgeP1 = pEdge->GetVertLocation( 1 );
	while( TRUE )
	{
		UBOOL bLeftIsP0;
		FNavMeshEdgeBase* pNextEdge = NULL;
		if( RemainingEdges == 1 )
		{
			const FLOAT fDirX = _EndPathPoint.X - _FirstPathPoint.X;
			const FLOAT fDirY = _EndPathPoint.Y - _FirstPathPoint.Y;
			const FLOAT fSide0 = ( EdgeP0.Y - _FirstPathPoint.Y ) * fDirX - ( EdgeP0.X - _FirstPathPoint.X ) * fDirY;
			const FLOAT fSide1 = ( EdgeP1.Y - _FirstPathPoint.Y ) * fDirX - ( EdgeP1.X - _FirstPathPoint.X ) * fDirY;
			bLeftIsP0 = fSide0 <= 0.f;
			if( fSide0 > 0.f && fSide1 > 0.f )
			{
				bLeftIsP0 = DistanceToLine( _FirstPathPoint, _EndPathPoint, EdgeP1 ) > DistanceToLine( _FirstPathPoint, _EndPathPoint, EdgeP0 );
			}
			else if( fSide0 <= 0.f && fSide1 <= 0.f )
			{
				bLeftIsP0 = DistanceToLine( _FirstPathPoint, _EndPathPoint, EdgeP0 ) > DistanceToLine( _FirstPathPoint, _EndPathPoint, EdgeP1 );
			}
		}
		else
		{
			pNextEdge = _PathFindingEdges.EdgeList( FirstEdgeIndex + 1 );
			FNavMeshPolyBase* pSharedPoly = ArkPathSharedPoly( pEdge, pNextEdge );
			const FVector PolyCenter = pSharedPoly ? pSharedPoly->GetPolyCenter() : EdgeP0;
			bLeftIsP0 = ( ( EdgeP1.Y - PolyCenter.Y ) * ( EdgeP0.X - PolyCenter.X ) - ( EdgeP0.Y - PolyCenter.Y ) * ( EdgeP1.X - PolyCenter.X ) ) <= 0.f;
		}
		const FVector& Left = bLeftIsP0 ? EdgeP0 : EdgeP1;
		const FVector& Right = bLeftIsP0 ? EdgeP1 : EdgeP0;
		if( ( _FirstPathPoint.Y - Left.Y ) * ( Right.X - Left.X ) - ( Right.Y - Left.Y ) * ( _FirstPathPoint.X - Left.X ) > 0.f )
		{
			_rbFirstEdgeLeftIsP0 = bLeftIsP0;
			return FirstEdgeIndex;
		}
		if( RemainingEdges == 1 )
		{
			_rbFirstEdgeLeftIsP0 = bLeftIsP0;
			return FirstEdgeIndex + 1;
		}
		++FirstEdgeIndex;
		--RemainingEdges;
		pEdge = pNextEdge;
		EdgeP0 = pEdge->GetVertLocation( 0 );
		EdgeP1 = pEdge->GetVertLocation( 1 );
	}
}

// DISHONORED(port): agent IO, retail whole: 2012 rva 0xb54d0 (1,207 bytes). From the first uncrossed edge: order each edge's
// verts left / right (the first by FindFirstEdgeIndex's flag, every later one against the centre of the poly it shares with
// the previous edge: left is P0 when the cross is <= 0, else P1 - DN had the mirror), the pull-in vector of length
// _fMinDistanceFromWall along the edge, a five-unit perpendicular probe, IsEdgeBorderExtentReachable on each end, then
// AddAdjustedPathFindingEdge. The first edge's size is its EdgeLength, a later edge's its 2D length (retail).
void AdjustPathFindingEdges( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, UNavigationHandle* _pNavHandle, FPathStore& _PathFindingEdges, FLOAT _fMinDistanceFromWall, TArray<FArkCpntLocoPathFindingEdge>& _AdjustedPathFindingEdges )
{
	const INT EdgeCount = _PathFindingEdges.EdgeList.Num();
	UBOOL bFirstEdgeLeftIsP0 = FALSE;
	const INT FirstEdgeIndex = FindFirstEdgeIndex( _FirstPathPoint, _EndPathPoint, _PathFindingEdges, bFirstEdgeLeftIsP0 );
	if( FirstEdgeIndex == EdgeCount )
	{
		return;
	}
	_AdjustedPathFindingEdges.Reserve( 2 * EdgeCount );

	FNavMeshEdgeBase* pPrevEdge = _PathFindingEdges.EdgeList( FirstEdgeIndex );
	{
		const FVector LeftVert = pPrevEdge->GetVertLocation( bFirstEdgeLeftIsP0 ? 0 : 1 );
		const FVector RightVert = pPrevEdge->GetVertLocation( bFirstEdgeLeftIsP0 ? 1 : 0 );
		const FLOAT fEdgeSize = pPrevEdge->EdgeLength;
		const FVector Edge = RightVert - LeftVert;
		const FVector PerpExtent = Edge * ( _fMinDistanceFromWall / fEdgeSize );
		const FVector AlongProbe( -Edge.Y * ( 5.f / fEdgeSize ), Edge.X * ( 5.f / fEdgeSize ), 0.f );
		const FVector EdgeCenter = ( LeftVert + RightVert ) * 0.5f;
		const UBOOL bLeftReachable = IsEdgeBorderExtentReachable( LeftVert, EdgeCenter, -PerpExtent, AlongProbe, _pNavHandle );
		const UBOOL bRightReachable = IsEdgeBorderExtentReachable( RightVert, EdgeCenter, PerpExtent, AlongProbe, _pNavHandle );
		AddAdjustedPathFindingEdge( LeftVert, RightVert, Edge, fEdgeSize, !bLeftReachable, !bRightReachable, PerpExtent, _AdjustedPathFindingEdges );
	}

	for( INT EdgeIdx = FirstEdgeIndex + 1; EdgeIdx < EdgeCount; ++EdgeIdx )
	{
		FNavMeshEdgeBase* pEdge = _PathFindingEdges.EdgeList( EdgeIdx );
		FNavMeshPolyBase* pSharedPoly = ArkPathSharedPoly( pPrevEdge, pEdge );
		const FVector V0 = pEdge->GetVertLocation( 0 );
		const FVector PolyCenter = pSharedPoly ? pSharedPoly->GetPolyCenter() : V0;
		const FVector V1 = pEdge->GetVertLocation( 1 );
		const UBOOL bLeftIsP0 = ( ( V0.Y - PolyCenter.Y ) * ( V1.X - PolyCenter.X ) - ( V1.Y - PolyCenter.Y ) * ( V0.X - PolyCenter.X ) ) <= 0.f;
		const FVector& LeftVert = bLeftIsP0 ? V0 : V1;
		const FVector& RightVert = bLeftIsP0 ? V1 : V0;
		const FVector Edge = RightVert - LeftVert;
		const FLOAT fInvEdgeLength = 1.f / pEdge->EdgeLength;
		const FVector PerpExtent = Edge * ( fInvEdgeLength * _fMinDistanceFromWall );
		const FVector AlongProbe( -Edge.Y * ( fInvEdgeLength * 5.f ), Edge.X * ( fInvEdgeLength * 5.f ), 0.f );
		const FVector EdgeCenter = ( LeftVert + RightVert ) * 0.5f;
		const UBOOL bLeftReachable = IsEdgeBorderExtentReachable( LeftVert, EdgeCenter, -PerpExtent, AlongProbe, _pNavHandle );
		const UBOOL bRightReachable = IsEdgeBorderExtentReachable( RightVert, EdgeCenter, PerpExtent, AlongProbe, _pNavHandle );
		AddAdjustedPathFindingEdge( LeftVert, RightVert, Edge, appSqrt( Edge.Y * Edge.Y + Edge.X * Edge.X ), !bLeftReachable, !bRightReachable, PerpExtent, _AdjustedPathFindingEdges );
		pPrevEdge = pEdge;
	}
}

// DISHONORED(port): 2012 rva 0xb5990, 863 bytes. One step of the funnel: from the apex, tighten the left and right bounds
// over successive edges until one crosses the other, and when it does move the apex to the bound that was crossed and
// report the edge index it came from. Retail treats two ends within 0.1 units of the current bound as "the same vert",
// which is how an edge that shares a corner with its predecessor does not fold the funnel shut.
static UBOOL ApplyFunnelAlgorithm_FindTurn( const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, INT& _rNextEdgeIdx, FArkCpntLocoPathPoint& _rFunnelApex, FVector& _rLeftCorner, FVector& _rRightCorner, INT& _rLeftCornerEdgeIdx, INT& _rRightCornerEdgeIdx )
{
	const INT EdgeCount = _PathFindingEdges.Num();
	if( _rNextEdgeIdx < 0 || _rNextEdgeIdx >= EdgeCount )
	{
		return FALSE;
	}
	_rFunnelApex.m_EdgeIndex = _rNextEdgeIdx;

	FVector LeftCorner  = _PathFindingEdges( _rNextEdgeIdx ).m_EdgeLeftPoint;
	FVector RightCorner = _PathFindingEdges( _rNextEdgeIdx ).m_EdgeRightPoint;
	INT LeftCornerEdgeIdx  = _rNextEdgeIdx;
	INT RightCornerEdgeIdx = _rNextEdgeIdx;
	UBOOL bFoundTurn = FALSE;

	for( INT EdgeIdx = _rNextEdgeIdx + 1; EdgeIdx < EdgeCount; ++EdgeIdx )
	{
		const FArkCpntLocoPathFindingEdge& rEdge = _PathFindingEdges( EdgeIdx );
		const UBOOL bLeftShared  = ArkPathSamePoint2D( LeftCorner,  rEdge.m_EdgeLeftPoint,  GArkPathSharedVertEpsilon );
		const UBOOL bRightShared = ArkPathSamePoint2D( RightCorner, rEdge.m_EdgeRightPoint, GArkPathSharedVertEpsilon );

		const UBOOL bTightenLeft  = bLeftShared  || ArkPathCross2D( _rFunnelApex.m_Point, LeftCorner,  rEdge.m_EdgeLeftPoint  ) <= 0.f;
		const UBOOL bTightenRight = bRightShared || ArkPathCross2D( _rFunnelApex.m_Point, RightCorner, rEdge.m_EdgeRightPoint ) >= 0.f;

		if( bTightenLeft )
		{
			LeftCorner = rEdge.m_EdgeLeftPoint;
			LeftCornerEdgeIdx = EdgeIdx;
			if( ArkPathCross2D( _rFunnelApex.m_Point, RightCorner, LeftCorner ) <= 0.f )
			{
				_rNextEdgeIdx = RightCornerEdgeIdx + 1;
				_rFunnelApex.m_Point = RightCorner;
				_rFunnelApex.m_EdgeIndex = RightCornerEdgeIdx;
				bFoundTurn = TRUE;
				break;
			}
		}
		if( bTightenRight )
		{
			RightCorner = rEdge.m_EdgeRightPoint;
			RightCornerEdgeIdx = EdgeIdx;
			if( ArkPathCross2D( _rFunnelApex.m_Point, LeftCorner, RightCorner ) >= 0.f )
			{
				_rNextEdgeIdx = LeftCornerEdgeIdx + 1;
				_rFunnelApex.m_Point = LeftCorner;
				_rFunnelApex.m_EdgeIndex = LeftCornerEdgeIdx;
				bFoundTurn = TRUE;
				break;
			}
		}
	}

	_rLeftCorner = LeftCorner;
	_rRightCorner = RightCorner;
	_rLeftCornerEdgeIdx = LeftCornerEdgeIdx;
	_rRightCornerEdgeIdx = RightCornerEdgeIdx;
	return bFoundTurn;
}

// DISHONORED(port): agent IO, retail whole: 2012 rva 0xb5cf0 (739 bytes). The apex starts at the first path point and is
// emitted each time it moves (never twice at the same 3D place, the 0.0001 test). When FindTurn runs out of edges, the goal
// is tested against the surviving bounds: on the right side of both, the apex moves to the right corner; on the left side of
// both, to the left corner; the funnel then continues past that corner's edge. Once the apex reaches the last edge it is
// emitted; the goal always ends the path. DN's version took the right corner when the goal was between the bounds' signs
// (left <= 0, right >= 0), not retail's same-sign test, and capped the loop.
void MinimizeStraightPath_FunnelAlgorithm( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out )
{
	const INT EdgeCount = _PathFindingEdges.Num();
	FArkCpntLocoPathPoint FunnelApex;
	FunnelApex.m_Point = _FirstPathPoint;
	FunnelApex.m_EdgeIndex = INDEX_NONE;
	FVector LeftCorner( 0.f, 0.f, 0.f );
	FVector RightCorner( 0.f, 0.f, 0.f );
	INT LeftCornerEdgeIdx = INDEX_NONE;
	INT RightCornerEdgeIdx = INDEX_NONE;
	INT NextEdgeIdx = 0;
	do
	{
		while( TRUE )
		{
			const INT PointCount = _StraightPath_Out.Num();
			if( PointCount == 0 || !_StraightPath_Out( PointCount - 1 ).m_Point.Equals( FunnelApex.m_Point, GArkPathSamePointEpsilon ) )
			{
				_StraightPath_Out.AddItem( FunnelApex );
			}
			if( !ApplyFunnelAlgorithm_FindTurn( _PathFindingEdges, NextEdgeIdx, FunnelApex, LeftCorner, RightCorner, LeftCornerEdgeIdx, RightCornerEdgeIdx ) || NextEdgeIdx >= EdgeCount )
			{
				break;
			}
		}
		UBOOL bMovedApex = FALSE;
		if( NextEdgeIdx < EdgeCount )
		{
			const FLOAT fToGoalX = _LastPathPoint.X - FunnelApex.m_Point.X;
			const FLOAT fToGoalY = _LastPathPoint.Y - FunnelApex.m_Point.Y;
			const FLOAT fLeftSide = ( LeftCorner.Y - FunnelApex.m_Point.Y ) * fToGoalX - ( LeftCorner.X - FunnelApex.m_Point.X ) * fToGoalY;
			const FLOAT fRightSide = ( RightCorner.Y - FunnelApex.m_Point.Y ) * fToGoalX - ( RightCorner.X - FunnelApex.m_Point.X ) * fToGoalY;
			if( fLeftSide <= 0.f && fRightSide <= 0.f )
			{
				FunnelApex.m_Point = RightCorner;
				FunnelApex.m_EdgeIndex = RightCornerEdgeIdx;
				NextEdgeIdx = RightCornerEdgeIdx + 1;
				bMovedApex = TRUE;
			}
			else if( fRightSide >= 0.f && fLeftSide >= 0.f )
			{
				FunnelApex.m_Point = LeftCorner;
				FunnelApex.m_EdgeIndex = LeftCornerEdgeIdx;
				NextEdgeIdx = LeftCornerEdgeIdx + 1;
				bMovedApex = TRUE;
			}
		}
		if( !bMovedApex )
		{
			FArkCpntLocoPathPoint Last;
			Last.m_Point = _LastPathPoint;
			Last.m_EdgeIndex = INDEX_NONE;
			_StraightPath_Out.AddItem( Last );
			return;
		}
	}
	while( NextEdgeIdx != EdgeCount );

	const INT PointCount = _StraightPath_Out.Num();
	if( PointCount == 0 || !_StraightPath_Out( PointCount - 1 ).m_Point.Equals( FunnelApex.m_Point, GArkPathSamePointEpsilon ) )
	{
		_StraightPath_Out.AddItem( FunnelApex );
	}
	FArkCpntLocoPathPoint Last;
	Last.m_Point = _LastPathPoint;
	Last.m_EdgeIndex = INDEX_NONE;
	_StraightPath_Out.AddItem( Last );
}

// DISHONORED(port): agent IO, retail whole: 2012 rva 0xb5fe0 (1,270 bytes). The funnel's corners sit on the pulled-in edge
// ends. For each inner corner whose end was pulled in by more than one unit, on the side the path turns (the right end when
// the corner is on the positive side of prev->next, else the left end): when the path arrives at it, or leaves it, at more
// than 135 degrees from the edge direction (cos < -0.7071), a corner is inserted before / after it, the pulled-in distance away
// on the bisector of the edge and the leg - which rounds the corner around the wall instead of cutting it. A corner inserted
// before the second point is dropped when it would send the pawn back past the first. DN's version only re-derived each
// corner's Z from the real edge.
void AdjustStraightPath( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _PathPoints )
{
	INT PathPointIdx = 1;
	while( PathPointIdx < _PathPoints.Num() - 1 )
	{
		const FVector PrevPos = _PathPoints( PathPointIdx - 1 ).m_Point;
		const FVector CurPos = _PathPoints( PathPointIdx ).m_Point;
		const FVector NextPos = _PathPoints( PathPointIdx + 1 ).m_Point;
		const INT EdgeIndex = _PathPoints( PathPointIdx ).m_EdgeIndex;
		if( EdgeIndex < 0 || EdgeIndex >= _PathFindingEdges.Num() )
		{
			++PathPointIdx;
			continue;
		}
		const FArkCpntLocoPathFindingEdge& rEdge = _PathFindingEdges( EdgeIndex );
		const UBOOL bLeft = ( ( CurPos.X - PrevPos.X ) * ( NextPos.Y - PrevPos.Y ) - ( CurPos.Y - PrevPos.Y ) * ( NextPos.X - PrevPos.X ) ) <= 0.f;
		const FVector PathEdgeTurnPos = bLeft ? rEdge.m_EdgeLeftPoint : rEdge.m_EdgeRightPoint;
		const FLOAT fTurnPointDistance = bLeft ? rEdge.m_fEdgeLeftAdjustedDistance : rEdge.m_fEdgeRightAdjustedDistance;
		if( fTurnPointDistance * fTurnPointDistance > 1.f )
		{
			const FLOAT fEdgeX = rEdge.m_EdgeRightPoint.X - rEdge.m_EdgeLeftPoint.X;
			const FLOAT fEdgeY = rEdge.m_EdgeRightPoint.Y - rEdge.m_EdgeLeftPoint.Y;
			const FLOAT fEdgeSize2D = appSqrt( fEdgeX * fEdgeX + fEdgeY * fEdgeY );
			const FLOAT fToPrevX = PrevPos.X - PathEdgeTurnPos.X;
			const FLOAT fToPrevY = PrevPos.Y - PathEdgeTurnPos.Y;
			const FLOAT fToPrevSize2D = appSqrt( fToPrevX * fToPrevX + fToPrevY * fToPrevY );
			const FLOAT fToNextX = NextPos.X - PathEdgeTurnPos.X;
			const FLOAT fToNextY = NextPos.Y - PathEdgeTurnPos.Y;
			const FLOAT fToNextSize2D = appSqrt( fToNextX * fToNextX + fToNextY * fToNextY );
			if( fEdgeSize2D >= 0.0001f && fToPrevSize2D >= 0.0001f && fToNextSize2D >= 0.0001f )
			{
				const FLOAT fEdgeDirX = fEdgeX / fEdgeSize2D;
				const FLOAT fEdgeDirY = fEdgeY / fEdgeSize2D;
				const FLOAT fSignedEdgeSize = bLeft ? fEdgeSize2D : -fEdgeSize2D;
				const FLOAT fPrevCos = Clamp( ( fToPrevY * fEdgeY + fToPrevX * fEdgeX ) / ( fSignedEdgeSize * fToPrevSize2D ), -1.f, 1.f );
				if( fPrevCos < -0.707106781187f )
				{
					FArkCpntLocoPathPoint Item;
					Item.m_Point.X = ( fToPrevY / fToPrevSize2D - fEdgeDirX ) * fTurnPointDistance + PathEdgeTurnPos.X;
					Item.m_Point.Y = ( -fToPrevX / fToPrevSize2D - fEdgeDirY ) * fTurnPointDistance + PathEdgeTurnPos.Y;
					Item.m_Point.Z = PathEdgeTurnPos.Z;
					Item.m_EdgeIndex = INDEX_NONE;
					const UBOOL bBehindStart = PathPointIdx == 1
						&& ( ( CurPos.Z - Item.m_Point.Z ) * ( Item.m_Point.Z - _FirstPathPoint.Z ) + ( CurPos.Y - Item.m_Point.Y ) * ( Item.m_Point.Y - _FirstPathPoint.Y )
							+ ( CurPos.X - Item.m_Point.X ) * ( Item.m_Point.X - _FirstPathPoint.X ) ) <= 0.f;
					if( !bBehindStart )
					{
						_PathPoints.InsertItem( Item, PathPointIdx );
						++PathPointIdx;
					}
				}
				const FLOAT fNextCos = Clamp( ( fToNextX * fEdgeX + fToNextY * fEdgeY ) / ( fSignedEdgeSize * fToNextSize2D ), -1.f, 1.f );
				if( fNextCos < -0.707106781187f )
				{
					FArkCpntLocoPathPoint Item;
					Item.m_Point.X = ( -fToNextY / fToNextSize2D - fEdgeDirX ) * fTurnPointDistance + PathEdgeTurnPos.X;
					Item.m_Point.Y = ( fToNextX / fToNextSize2D - fEdgeDirY ) * fTurnPointDistance + PathEdgeTurnPos.Y;
					Item.m_Point.Z = PathEdgeTurnPos.Z;
					Item.m_EdgeIndex = INDEX_NONE;
					_PathPoints.InsertItem( Item, PathPointIdx + 1 );
					++PathPointIdx;
				}
			}
		}
		++PathPointIdx;
	}
}

// DISHONORED(port): 2013 rva 0xb9310 (2012 0xb64e0, 159 bytes, arkpathbuildutils.cpp:642). No edges means the start and
// the goal are in the same poly, so the path is those two points; otherwise the funnel runs and its result is adjusted.
void BuildStraightPath( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out )
{
	if( _PathFindingEdges.Num() <= 0 )
	{
		FArkCpntLocoPathPoint Point;
		Point.m_EdgeIndex = INDEX_NONE;
		Point.m_Point = _FirstPathPoint;
		_StraightPath_Out.AddItem( Point );
		Point.m_Point = _LastPathPoint;
		_StraightPath_Out.AddItem( Point );
	}
	else
	{
		MinimizeStraightPath_FunnelAlgorithm( _FirstPathPoint, _LastPathPoint, _PathFindingEdges, _StraightPath_Out );
		AdjustStraightPath( _FirstPathPoint, _LastPathPoint, _PathFindingEdges, _StraightPath_Out );
	}
}
