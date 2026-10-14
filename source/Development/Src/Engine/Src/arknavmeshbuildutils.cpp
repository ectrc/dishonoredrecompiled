// Engine/src/arknavmeshbuildutils.cpp
// DISHONORED(port): agent GK - the three point tests; the ten build-time helpers stay named below.
// PDB functions attributed to this file (13):
//   0x53d5f0  void __cdecl ArkNavMeshComputeHitPropertiesFromEdge(class FVector const &, class FVector const &, class FVector const &, class FVector const &, float, struct FNavMeshCheckResult *)
//   0x53d840  void __cdecl ArkNavMeshComputeHitPropertiesFromPoly(class FVector const &, class FVector const &, struct FNavMeshPolyBase const *, struct FNavMeshCheckResult *)
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
