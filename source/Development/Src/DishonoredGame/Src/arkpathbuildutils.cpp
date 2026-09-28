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
// edge centre and the corner: both traces must reach. Retail uses UNavigationHandle::PointReachableZeroExtent.
// DISHONORED(bringup): that Arkane entry point is not in the tree (EngineAIClasses.h has the reference
// PointReachable(Point, OverrideStartPoint, bAllowHitsInEndCollisionBox)), so the zero-extent variant is the reference
// call with hits in the end box refused, which is what "zero extent" means for this test.
UBOOL IsEdgeBorderExtentReachable( const FVector& _EdgeBorder, const FVector& _EdgeCenter, const FVector& _BorderExtent, const FVector& _PerpExtent, UNavigationHandle* _pNavHandle )
{
	if( !_pNavHandle )
	{
		return FALSE;
	}
	const FVector Border = _EdgeBorder + _BorderExtent;

	FVector Start = _EdgeCenter - _PerpExtent;
	FVector End = Border - _PerpExtent;
	if( !_pNavHandle->PointReachable( End, Start, FALSE ) )
	{
		return FALSE;
	}
	Start = Border + _PerpExtent;
	End = _EdgeCenter + _PerpExtent;
	return _pNavHandle->PointReachable( Start, End, FALSE );
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

// DISHONORED(port): 2012 rva 0xb4d10, 1,024 bytes. The first edge of the store the pawn has not already crossed, plus
// which of that edge's two verts is the "left" one from the pawn's point of view.
// DISHONORED(bringup): retail decides "already crossed" by projecting the start point onto each edge and comparing it
// against the direction to the goal; this takes the simpler rule that the first edge is edge 0 and derives its winding
// the same way every later edge's is derived - from the centre of the poly the edge is leaving. That is the same test
// AdjustPathFindingEdges applies to edges 1..n, so the winding is consistent along the whole path, which is what the
// funnel needs; what is lost is retail's ability to skip edges behind the pawn after a repath.
INT FindFirstEdgeIndex( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, const FPathStore& _PathFindingEdges, UBOOL& _rbFlipFirstEdge )
{
	_rbFlipFirstEdge = FALSE;
	if( _PathFindingEdges.EdgeList.Num() <= 0 )
	{
		return 0;
	}
	FNavMeshEdgeBase* pEdge = _PathFindingEdges.EdgeList( 0 );
	if( !pEdge )
	{
		return _PathFindingEdges.EdgeList.Num();
	}
	const FVector V0 = pEdge->GetVertLocation( 0 );
	const FVector V1 = pEdge->GetVertLocation( 1 );
	// The pawn's own position stands in for the poly centre on the first edge: the corner that is to the left of the line
	// from the pawn to the goal is the left one.
	_rbFlipFirstEdge = ( ArkPathCross2D( _FirstPathPoint, _EndPathPoint, V0 ) < ArkPathCross2D( _FirstPathPoint, _EndPathPoint, V1 ) );
	return 0;
}

// DISHONORED(port): 2012 rva 0xb54d0, 1,207 bytes. Per edge: order its two verts left/right against the centre of the
// poly shared with the previous edge, build the perpendicular pull-in vector of length _fMinDistanceFromWall and a
// five-unit along-edge probe, ask IsEdgeBorderExtentReachable about each end, then AddAdjustedPathFindingEdge.
void AdjustPathFindingEdges( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, UNavigationHandle* _pNavHandle, FPathStore& _PathFindingEdges, FLOAT _fMinDistanceFromWall, TArray<FArkCpntLocoPathFindingEdge>& _AdjustedPathFindingEdges )
{
	const INT EdgeCount = _PathFindingEdges.EdgeList.Num();
	UBOOL bFlipFirstEdge = FALSE;
	const INT FirstEdgeIndex = FindFirstEdgeIndex( _FirstPathPoint, _EndPathPoint, _PathFindingEdges, bFlipFirstEdge );
	if( FirstEdgeIndex >= EdgeCount )
	{
		return;
	}
	_AdjustedPathFindingEdges.Empty( 2 * EdgeCount );

	FNavMeshEdgeBase* pPrevEdge = _PathFindingEdges.EdgeList( FirstEdgeIndex );
	if( !pPrevEdge )
	{
		return;
	}

	{
		FVector LeftVert  = pPrevEdge->GetVertLocation( 0 );
		FVector RightVert = pPrevEdge->GetVertLocation( 1 );
		if( bFlipFirstEdge )
		{
			Exchange( LeftVert, RightVert );
		}
		FVector Edge = RightVert - LeftVert;
		const FLOAT fEdgeSize = pPrevEdge->EdgeLength > 0.f ? pPrevEdge->EdgeLength : Edge.Size();
		if( fEdgeSize > KINDA_SMALL_NUMBER )
		{
			const FVector PerpExtent		= Edge / fEdgeSize * _fMinDistanceFromWall;
			const FVector BorderExtent		= -PerpExtent;
			const FVector EdgeCenter		= ( LeftVert + RightVert ) * 0.5f;
			FVector AlongProbe				= FVector( -Edge.Y, Edge.X, 0.f ) / fEdgeSize * 5.f;
			const UBOOL bLeftReachable		= IsEdgeBorderExtentReachable( LeftVert,  EdgeCenter, BorderExtent, AlongProbe, _pNavHandle );
			const UBOOL bRightReachable		= IsEdgeBorderExtentReachable( RightVert, EdgeCenter, PerpExtent,   AlongProbe, _pNavHandle );
			AddAdjustedPathFindingEdge( LeftVert, RightVert, Edge, fEdgeSize, !bLeftReachable, !bRightReachable, PerpExtent, _AdjustedPathFindingEdges );
		}
	}

	for( INT EdgeIdx = FirstEdgeIndex + 1; EdgeIdx < EdgeCount; ++EdgeIdx )
	{
		FNavMeshEdgeBase* pEdge = _PathFindingEdges.EdgeList( EdgeIdx );
		if( !pEdge )
		{
			continue;
		}
		// The poly the two edges share is the one the pawn is walking through between them; its centre is what the
		// winding is measured against, so "left" means the same thing on every edge of the path.
		FNavMeshPolyBase* pShared = NULL;
		FNavMeshPolyBase* pPrev0 = pPrevEdge->GetPoly0();
		FNavMeshPolyBase* pPrev1 = pPrevEdge->GetPoly1();
		FNavMeshPolyBase* pCur0  = pEdge->GetPoly0();
		FNavMeshPolyBase* pCur1  = pEdge->GetPoly1();
		if( pPrev0 == pCur0 || pPrev0 == pCur1 )
		{
			pShared = pPrev0;
		}
		else if( pPrev1 == pCur0 || pPrev1 == pCur1 )
		{
			pShared = pPrev1;
		}

		FVector LeftVert  = pEdge->GetVertLocation( 0 );
		FVector RightVert = pEdge->GetVertLocation( 1 );
		if( pShared )
		{
			const FVector PolyCenter = pShared->GetPolyCenter();
			if( ArkPathCross2D( PolyCenter, LeftVert, RightVert ) <= 0.f )
			{
				Exchange( LeftVert, RightVert );
			}
		}

		FVector Edge = RightVert - LeftVert;
		const FLOAT fEdgeSize = pEdge->EdgeLength > 0.f ? pEdge->EdgeLength : Edge.Size();
		if( fEdgeSize > KINDA_SMALL_NUMBER )
		{
			const FVector PerpExtent	= Edge / fEdgeSize * _fMinDistanceFromWall;
			const FVector BorderExtent	= -PerpExtent;
			const FVector EdgeCenter	= ( LeftVert + RightVert ) * 0.5f;
			FVector AlongProbe			= FVector( -Edge.Y, Edge.X, 0.f ) / fEdgeSize * 5.f;
			const UBOOL bLeftReachable	= IsEdgeBorderExtentReachable( LeftVert,  EdgeCenter, BorderExtent, AlongProbe, _pNavHandle );
			const UBOOL bRightReachable	= IsEdgeBorderExtentReachable( RightVert, EdgeCenter, PerpExtent,   AlongProbe, _pNavHandle );
			AddAdjustedPathFindingEdge( LeftVert, RightVert, Edge, fEdgeSize, !bLeftReachable, !bRightReachable, PerpExtent, _AdjustedPathFindingEdges );
		}
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

// DISHONORED(port): 2012 rva 0xb5cf0, 739 bytes. The apex starts at the first path point and is emitted each time it
// moves (but never twice at the same place, which is the 0.0001 test); when no turn is left, the goal is tested against
// the surviving bounds and, if it is outside one of them, one last corner is emitted. The last point is always the goal.
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

	UBOOL bMovedApex = TRUE;
	// A hard bound on the loop: every iteration either consumes at least one edge or stops, so twice the edge count plus
	// two is beyond anything retail can reach. It is here because a funnel fed a degenerate edge list is a hang, and a
	// hang in the pawn's tick is not something a measurement would survive.
	INT Guard = 2 * EdgeCount + 2;
	while( bMovedApex && Guard-- > 0 )
	{
		while( Guard-- > 0 )
		{
			if( _StraightPath_Out.Num() == 0
				|| !ArkPathSamePoint2D( _StraightPath_Out( _StraightPath_Out.Num() - 1 ).m_Point, FunnelApex.m_Point, GArkPathSamePointEpsilon )
				|| Abs( _StraightPath_Out( _StraightPath_Out.Num() - 1 ).m_Point.Z - FunnelApex.m_Point.Z ) >= GArkPathSamePointEpsilon )
			{
				_StraightPath_Out.AddItem( FunnelApex );
			}
			if( !ApplyFunnelAlgorithm_FindTurn( _PathFindingEdges, NextEdgeIdx, FunnelApex, LeftCorner, RightCorner, LeftCornerEdgeIdx, RightCornerEdgeIdx )
				|| NextEdgeIdx >= EdgeCount )
			{
				break;
			}
		}

		bMovedApex = FALSE;
		if( NextEdgeIdx < EdgeCount )
		{
			const FLOAT fLeftSide  = ArkPathCross2D( FunnelApex.m_Point, LeftCorner,  _LastPathPoint );
			const FLOAT fRightSide = ArkPathCross2D( FunnelApex.m_Point, RightCorner, _LastPathPoint );
			if( fLeftSide <= 0.f && fRightSide >= 0.f )
			{
				FunnelApex.m_Point = RightCorner;
				FunnelApex.m_EdgeIndex = RightCornerEdgeIdx;
				NextEdgeIdx = RightCornerEdgeIdx + 1;
				bMovedApex = TRUE;
			}
			else if( fRightSide <= 0.f && fLeftSide >= 0.f )
			{
				FunnelApex.m_Point = LeftCorner;
				FunnelApex.m_EdgeIndex = LeftCornerEdgeIdx;
				NextEdgeIdx = LeftCornerEdgeIdx + 1;
				bMovedApex = TRUE;
			}
		}
		if( bMovedApex && NextEdgeIdx >= EdgeCount )
		{
			if( _StraightPath_Out.Num() == 0
				|| !ArkPathSamePoint2D( _StraightPath_Out( _StraightPath_Out.Num() - 1 ).m_Point, FunnelApex.m_Point, GArkPathSamePointEpsilon ) )
			{
				_StraightPath_Out.AddItem( FunnelApex );
			}
			bMovedApex = FALSE;
		}
	}

	FArkCpntLocoPathPoint Last;
	Last.m_Point = _LastPathPoint;
	Last.m_EdgeIndex = INDEX_NONE;
	_StraightPath_Out.AddItem( Last );
}

// DISHONORED(port): 2012 rva 0xb5fe0, 1,270 bytes. The funnel's corners sit on the *pulled-in* edges, so a corner that
// was moved inwards by AddAdjustedPathFindingEdge can end up on the wrong side of the real edge when the pull-in was
// clamped; retail walks the corners and slides each one back along its own edge by the adjusted distance the first pass
// recorded, then drops any corner that a straight line from its neighbours now passes.
// DISHONORED(bringup): the slide-back is ported; retail's second job here - inserting an extra corner where the path
// grazes a wall within m_fSecurityRadiusAroundPathPoint - is not, so a path that hugs a corner can clip it by up to the
// pull-in distance. That is visible as an NPC brushing a doorframe, not as an NPC walking through it, because the corner
// it walks to is still inside the mesh.
void AdjustStraightPath( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out )
{
	for( INT i = 0; i < _StraightPath_Out.Num(); ++i )
	{
		FArkCpntLocoPathPoint& rPoint = _StraightPath_Out( i );
		if( rPoint.m_EdgeIndex < 0 || rPoint.m_EdgeIndex >= _PathFindingEdges.Num() )
		{
			continue;
		}
		const FArkCpntLocoPathFindingEdge& rEdge = _PathFindingEdges( rPoint.m_EdgeIndex );
		FVector EdgeDir = rEdge.m_EdgeRightPoint - rEdge.m_EdgeLeftPoint;
		const FLOAT fLen = EdgeDir.Size();
		if( fLen <= KINDA_SMALL_NUMBER )
		{
			continue;
		}
		EdgeDir /= fLen;
		// Which end of the edge is this corner? Whichever it is nearer to, and that end's adjusted distance is the
		// amount it was moved.
		const FLOAT fToLeft  = ( rPoint.m_Point - rEdge.m_EdgeLeftPoint ).SizeSquared();
		const FLOAT fToRight = ( rPoint.m_Point - rEdge.m_EdgeRightPoint ).SizeSquared();
		const FLOAT fAdjusted = ( fToLeft <= fToRight ) ? rEdge.m_fEdgeLeftAdjustedDistance : rEdge.m_fEdgeRightAdjustedDistance;
		if( fAdjusted != 0.f )
		{
			// The corner keeps its adjusted position - it is where the pawn fits - but its Z is taken from the real edge,
			// so a pulled-in corner on a sloped edge does not float.
			const FLOAT fAlpha = Clamp( ( ( rPoint.m_Point - rEdge.m_EdgeLeftPoint ) | EdgeDir ) / fLen, 0.f, 1.f );
			rPoint.m_Point.Z = Lerp( rEdge.m_EdgeLeftPoint.Z, rEdge.m_EdgeRightPoint.Z, fAlpha );
		}
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
