/*=============================================================================
	indiag.h: agent IN (PHASE23 IN) - the -indiag switch: which primitive collides at a point, and why not.

	  INPROBE [X Y Z [R]]   console command (through the viewport client): every primitive component whose bounds come within
	                        R (default 300) of the point (default 10327 23053 2415, the dock stair foot), from every source -
	                        TObjectIterator over all live primitive components, so actor Components / AllComponents,
	                        StaticMeshCollectionActor members, InterpActors and skeletal meshes alike. One row each: class,
	                        owner and its level, the source, attachment, the collision flags of component and owner, the trace
	                        mask, ShouldCollide, whether the octree holds it (OctreeNodes and a Hash->GetIntersectingPrimitives
	                        query), the mesh's collision data (BodySetup elements, kDOP nodes/triangles, UseSimple*, the
	                        elements with EnableCollision; PhysicsAsset bodies for skeletal meshes) and the component's own
	                        LineCheck straight down (zero extent, NPC extent, and the per-poly TRACE_ComplexCollision variant).
	                        Before the rows: the world's levels and streaming levels, then the world traces at the point.
	  -inprobeat=W,W,...    the same probe at those world times, at the default point (or -inprobept=X,Y,Z) and at the player
	                        pawn's feet (R 150, plus eight 80 uu player sweeps naming what blocks him).
	Nothing here changes behaviour. Body in Src/indiag.cpp.
=============================================================================*/
#ifndef _INC_INDIAG
#define _INC_INDIAG

UBOOL DisINDiagEnabled();
void DisINReport( class APlayerController* Controller );
UBOOL DisINDiagExec( const TCHAR* Cmd, FOutputDevice& Ar );

#endif
