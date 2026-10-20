/*=============================================================================
	irdiag.h: agent IR (PHASE23 IR) - NPC mouths: what the head mesh renders and what moves while a line plays.

	  -irdiag         once per head mesh: the head skeletal mesh component's LOD sections (material index, triangles,
	                  hidden), every material slot (mesh material, component override, the MIC chain with its scalar /
	                  vector / texture parameters and the textures' resident mips, the base material's usage flags and
	                  whether its shader map holds GPU-skin shaders), the FaceFX asset, the parent anim component and the
	                  face bones; while a blurb plays, the speaker's face bones (local translation / rotation against the
	                  reference pose) every 0.1 s
	  -irface[=D]     while a blurb plays, the player pawn becomes a tripod D units (default 45) in front of the speaker's
	                  head_jnt, looking at it, so -apshottime captures the face close up
	Nothing here changes what an NPC does.
=============================================================================*/
#ifndef _INC_IRDIAG
#define _INC_IRDIAG

UBOOL DisIRDiagEnabled();
void DisIRNoteSpeaker( class USkeletalMeshComponent* FaceFXMesh, const FString& EventName );
void DisIRDumpHead( class ADishonoredNPCPawn* Pawn, const TCHAR* Why );
void DisIRTick( class APlayerController* PC );

#endif
