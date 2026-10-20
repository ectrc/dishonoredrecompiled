// Engine/src/arknavmeshbuildutils.cpp
// DISHONORED(port): agent GK - the three point tests; the ten build-time helpers stay named below.
// PDB functions attributed to this file (13):
//   0x53d5f0  void __cdecl ArkNavMeshComputeHitPropertiesFromEdge(class FVector const &, class FVector const &, class FVector const &, class FVector const &, float, struct FNavMeshCheckResult *)   [ported, agent IO2]
//   0x53d840  void __cdecl ArkNavMeshComputeHitPropertiesFromPoly(class FVector const &, class FVector const &, struct FNavMeshPolyBase const *, struct FNavMeshCheckResult *)   [ported, agent IO2]
//   0x5409e0  unsigned int __cdecl ArkNavMeshPolyContainsPoint2DAlreadyInBound(struct FNavMeshPolyBase const *, class FVector const &)   [ported, 2013 0x501880]
//   0x540b50  void __cdecl ArkComputePolygonBoundingRectangle2D(class TArray<class FVector, class FDefaultAllocator> const &, class TArray<class FVector, class FDefaultAllocator> &)
//   0x541100  void __cdecl ArkComputeCircleShape2D(class FVector const &, float, int, class TArray<class FVector, class FDefaultAllocator> &)
//   0x541220  void __cdecl ArkNavGetLongestNormalEdge(class TArray<struct FNavMeshEdgeBase *, class FDefaultAllocator> &, struct FNavMeshEdgeBase * &, int &)
//   0x5412b0  void __cdecl ArkNavRemoveEdges(class TArray<struct FNavMeshEdgeBase *, class FDefaultAllocator> &)
//   0x541310  unsigned int __cdecl ArkNavHasBothWaysEdge(class TArray<struct FNavMeshEdgeBase *, class FDefaultAllocator> &)
//   0x541380  unsigned int __cdecl ArkNavMeshComputeIntersectingPolys(class TArray<class FVector, class FDefaultAllocator> const &, class TArray<struct FNavMeshPolyBase *, class FDefaultAllocator> const &, class TArray<struct FNavMeshPolyBase *, class FDefaultAllocator> &)
//   0x544270  unsigned int __cdecl ArkNavMeshPolyContainsPoint2D(struct FNavMeshPolyBase const *, class FVector const &)   [ported, 2013 0x5062a0]
//   0x5442c0  unsigned int __cdecl ArkNavMeshPolyContainsPoint3D(struct FNavMeshPolyBase const *, class FVector const &, float)   [ported, 2013 0x5062f0]
//   0x545640  void __cdecl ArkShrinkWrap2D(class TArray<struct FVector2D, class FDefaultAllocator> &)
//   0x545d40  void __cdecl ArkComputeOrientedBoundingShape2D(class FMatrix const &, class FBox const &, class TArray<class FVector, class FDefaultAllocator> &)

#include "EnginePrivate.h"
#include "UnPath.h"
#include "arknavmeshbuildutils.h"

// Retail walks the poly's vertex ids straight into the mesh's vertex array (the Dishonored meshes are in world space);
// GetVertLocation( WORLD_SPACE ) is the same read on a mesh that needs no transform.
UBOOL ArkNavMeshPolyContainsPoint2DAlreadyInBound( const FNavMeshPolyBase* _pPoly, const FVector& _Point )
{
	const INT PolyVertCount = _pPoly->PolyVerts.Num();
	if( PolyVertCount <= 0 )
	{
		return TRUE;
	}
	FVector PrevVert = _pPoly->GetVertLocation( PolyVertCount - 1, WORLD_SPACE );
	for( INT VertIdx = 0; VertIdx < PolyVertCount; ++VertIdx )
	{
		const FVector Vert = _pPoly->GetVertLocation( VertIdx, WORLD_SPACE );
		if( ( Vert.Y - PrevVert.Y ) * ( _Point.X - PrevVert.X ) - ( Vert.X - PrevVert.X ) * ( _Point.Y - PrevVert.Y ) > 0.f )
		{
			return FALSE;
		}
		PrevVert = Vert;
	}
	return TRUE;
}

UBOOL ArkNavMeshPolyContainsPoint2D( const FNavMeshPolyBase* _pPoly, const FVector& _Point )
{
	if( _Point.X < _pPoly->BoxBounds.Min.X || _pPoly->BoxBounds.Max.X < _Point.X
		|| _Point.Y < _pPoly->BoxBounds.Min.Y || _pPoly->BoxBounds.Max.Y < _Point.Y )
	{
		return FALSE;
	}
	return ArkNavMeshPolyContainsPoint2DAlreadyInBound( _pPoly, _Point );
}

UBOOL ArkNavMeshPolyContainsPoint3D( const FNavMeshPolyBase* _pPoly, const FVector& _rPoint, FLOAT _fExtentZ )
{
	if( _rPoint.X < _pPoly->BoxBounds.Min.X || _pPoly->BoxBounds.Max.X < _rPoint.X
		|| _rPoint.Y < _pPoly->BoxBounds.Min.Y || _pPoly->BoxBounds.Max.Y < _rPoint.Y
		|| !ArkNavMeshPolyContainsPoint2DAlreadyInBound( _pPoly, _rPoint ) )
	{
		return FALSE;
	}
	const FVector Normal = _pPoly->GetPolyNormal( WORLD_SPACE );
	const FVector Center = _pPoly->GetPolyCenter( WORLD_SPACE );
	FLOAT fPlaneZ = _rPoint.Z;
	if( Abs( Normal.Z ) >= 0.0001f )
	{
		fPlaneZ = Center.Z - ( ( _rPoint.Y - Center.Y ) * Normal.Y + ( _rPoint.X - Center.X ) * Normal.X ) / Normal.Z;
	}
	const FLOAT fHeightOverPlane = _rPoint.Z - fPlaneZ;
	if( fHeightOverPlane < 0.f ? ( fHeightOverPlane <= -_fExtentZ ) : ( ( _pPoly->PolyHeight + _fExtentZ ) <= fHeightOverPlane ) )
	{
		return FALSE;
	}
	return TRUE;
}

// DISHONORED(port): agent IO2, 2012 rva 0x53d5f0. The time is clamped to [0, 1]; the normal is the 2D perpendicular of the
// normalised edge (EdgeStart - EdgeEnd), turned to face the start of the segment.
void ArkNavMeshComputeHitPropertiesFromEdge( const FVector& _rStartPt, const FVector& _rEndPt, const FVector& _rEdgeStartPt, const FVector& _rEdgeEndPt, FLOAT _fHitTime, FNavMeshCheckResult* _pHit )
{
	if( !_pHit )
	{
		return;
	}
	const FVector Edge2D = FVector( _rEdgeStartPt.X - _rEdgeEndPt.X, _rEdgeStartPt.Y - _rEdgeEndPt.Y, 0.f ).SafeNormal();
	const FVector Move = _rEndPt - _rStartPt;
	const FLOAT fHitTime = Clamp( _fHitTime, 0.f, 1.f );
	_pHit->m_fHitTime = fHitTime;
	_pHit->m_HitLocation = _rStartPt + Move * fHitTime;
	_pHit->m_bHit = TRUE;
	_pHit->m_HitNormal = FVector( Edge2D.Y, -Edge2D.X, Edge2D.X * 0.f - Edge2D.Y * 0.f );
	if( ( _pHit->m_HitNormal | Move ) > 0.f )
	{
		_pHit->m_HitNormal *= -1.f;
	}
}

// DISHONORED(port): agent IO2, 2012 rva 0x53d840. The poly's edges (previous vert -> vert, starting from the last) are crossed by
// the segment at most twice; the crossing with the largest time on the segment becomes the hit.
void ArkNavMeshComputeHitPropertiesFromPoly( const FVector& _rStartPt, const FVector& _rEndPt, const FNavMeshPolyBase* _pPoly, FNavMeshCheckResult* _pHit )
{
	if( !_pHit )
	{
		return;
	}
	const INT VertCount = _pPoly->PolyVerts.Num();
	FVector PrevVertLoc = _pPoly->GetVertLocation( VertCount - 1, WORLD_SPACE );
	FVector EdgeStartPt( 0.f, 0.f, 0.f );
	FVector EdgeEndPt( 0.f, 0.f, 0.f );
	FLOAT fBestHitTime = -1.f;
	INT HitCount = 0;
	for( INT VertIdx = 0; VertIdx < VertCount; VertIdx++ )
	{
		const FVector VertLoc = _pPoly->GetVertLocation( VertIdx, WORLD_SPACE );
		FLOAT fHitTime = 1.f;
		if( SegmentIntersect2D( _rStartPt, _rEndPt, PrevVertLoc, VertLoc, fHitTime ) )
		{
			if( fHitTime > fBestHitTime )
			{
				EdgeStartPt = PrevVertLoc;
				EdgeEndPt = VertLoc;
				fBestHitTime = fHitTime;
			}
			if( ++HitCount == 2 )
			{
				break;
			}
		}
		PrevVertLoc = VertLoc;
	}
	if( HitCount >= 1 )
	{
		ArkNavMeshComputeHitPropertiesFromEdge( _rStartPt, _rEndPt, EdgeStartPt, EdgeEndPt, fBestHitTime, _pHit );
	}
}
