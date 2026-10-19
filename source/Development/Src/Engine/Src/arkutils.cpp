// Engine/src/arkutils.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0xacfa0  unsigned int __cdecl ArkIsPointOnNavMesh(class FVector const &)   [ported, agent IK2]
//   0xacfd0  void __cdecl ArkRemoveRoll(class FMatrix &)
//   0xafbd0  float __cdecl ArkSmoothFixAngleRotation(float, float, float, float, float, float &)
//   0xafd90  unsigned int __cdecl ArkComputeSegmentOverPlane(class FVector, class FVector, class FVector const &, class FVector const &, class FVector &, class FVector &, float)
//   0xc79e0  class FString __cdecl ArkGetEnumString(class UEnum const *, unsigned char, unsigned int)

#include "EnginePrivate.h"
#include "UnPath.h"
#include "EngineAIClasses.h"
#include "arknavmeshbuildutils.h"

// DISHONORED(port, agent IK2): 2012 0xacfa0 (45 B): a pylon and a poly hold the point, at the default scout's walkable floor Z.
UBOOL ArkIsPointOnNavMesh( const FVector& _rCandidatePosition )
{
	APylon* pDummyPylon = NULL;
	FNavMeshPolyBase* pDummyPoly = NULL;
	return UNavigationHandle::GetPylonAndPolyFromPos( _rCandidatePosition, AScout::GetGameSpecificDefaultScoutObject()->WalkableFloorZ, pDummyPylon, pDummyPoly );
}
