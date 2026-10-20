// Engine/Inc/arknavmeshbuildutils.h
// DISHONORED(port): agent GK. Arkane's point-in-poly tests over the navigation mesh (Engine/Src/arknavmeshbuildutils.cpp
// in the 2012 PDB). Retail's UNavigationHandle::GetPylonAndPolyFromPos is built on the first one; the reference engine's
// FNavMeshPolyBase::ContainsPoint is a different test (a 3D box with a one-unit epsilon, then a strict crossing count)
// and refuses a point on a poly's edge or more than one unit above its surface - both of which the retail test accepts.
#ifndef ARKNAVMESHBUILDUTILS_H
#define ARKNAVMESHBUILDUTILS_H

struct FNavMeshPolyBase;

/** DISHONORED(layout): agent IO2, 2012 PDB FNavMeshCheckResult (32 bytes): the navmesh-walk equivalent of FCheckResult that
    UNavigationHandle::PointReachableZeroExtent fills. */
struct FNavMeshCheckResult
{
	FVector		m_HitLocation;		// @0
	FVector		m_HitNormal;		// @12
	FLOAT		m_fHitTime;			// @24
	BITFIELD	m_bHit:1;			// @28 0x1
};

/** 2012 rva 0x53d5f0: the hit on edge Start-End against the 2D edge EdgeStart-EdgeEnd at time _fHitTime: location, 2D normal
    facing the start, the hit flag. */
void ArkNavMeshComputeHitPropertiesFromEdge( const FVector& _rStartPt, const FVector& _rEndPt, const FVector& _rEdgeStartPt, const FVector& _rEdgeEndPt, FLOAT _fHitTime, FNavMeshCheckResult* _pHit );
/** 2012 rva 0x53d840: the farthest of (at most two) poly edges the segment crosses, as an edge hit. */
void ArkNavMeshComputeHitPropertiesFromPoly( const FVector& _rStartPt, const FVector& _rEndPt, const FNavMeshPolyBase* _pPoly, FNavMeshCheckResult* _pHit );

/** 2013 rva 0x501880 (2012 0x5409e0, 365 B): the 2D side test against every edge, inclusive - a point on an edge is inside. */
UBOOL ArkNavMeshPolyContainsPoint2DAlreadyInBound( const FNavMeshPolyBase* _pPoly, const FVector& _Point );
/** 2013 rva 0x5062a0 (2012 0x544270, 66 B): the poly's XY bounds (inclusive), then the side test. */
UBOOL ArkNavMeshPolyContainsPoint2D( const FNavMeshPolyBase* _pPoly, const FVector& _Point );
/** 2013 rva 0x5062f0 (2012 0x5442c0, 249 B): the 2D test, then the point's height over the poly's plane within [-extent, PolyHeight + extent). */
UBOOL ArkNavMeshPolyContainsPoint3D( const FNavMeshPolyBase* _pPoly, const FVector& _rPoint, FLOAT _fExtentZ );
/** 2012 0xacfa0 (45 B, Engine/Src/arkutils.cpp): UNavigationHandle::GetPylonAndPolyFromPos at the default scout's WalkableFloorZ. */
UBOOL ArkIsPointOnNavMesh( const FVector& _rCandidatePosition );

#endif
