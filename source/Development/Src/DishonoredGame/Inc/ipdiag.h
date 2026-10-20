/*=============================================================================
	ipdiag.h: agent IP (PHASE23 IP) - the -ipdiag switch: what moves the first-person camera, frame by frame.

	  ipdiag cam      every frame from world time -ipdiagfrom=<t> (default 0): delta time, the view location and rotation and
	                  their change since the last frame, the pawn's location, speed and physics, the camera bone relative to the
	                  pawn (in the pawn's yaw frame: forward / right / up) and its change, health, and every camera influence
	                  that contributed this frame (group, class, weight, its location and rotation: an additive influence's
	                  own offset, the core influence's view relative to the pawn)
	  ipdiag anim     four times a second: every relevant sequence of the player's tree (name, total weight, rate, position),
	                  every relevant DisAnimNodeBlendByHeadBob (its two child weights) and blend-by-speed node (speed, weights)
	  ipdiag misc     once a second: the camera's modifier list, its post-process targets' weights, the head-bob and reaction
	                  amounts, the group weights
	A/B switches (diagnosis only, all default off): -ipnobump puts the bump smoother to sleep (its gather answers FALSE as
	before the port), -ipnoheadbob leaves the head-bob nodes' child weights as the package has them, -ipnospeed gives the player's
	blend-by-speed nodes the engine's CalcSpeed (each as before its port); -ipnoland skips Landed_Native's small-land upper-body and
	left-hand action requests (agent IM's port) to measure what the landing animations add;
	-ipnobreath leaves the out-of-breath layer (DisAnimNodeBlendByPhysicalCondition) at the package's weight (before the port).
	Body in Src/ipdiag.cpp.
=============================================================================*/
#ifndef _INC_IPDIAG
#define _INC_IPDIAG

UBOOL DisIPDiagEnabled();
UBOOL DisIPNoBump();
UBOOL DisIPNoHeadBob();
UBOOL DisIPNoSpeed();
UBOOL DisIPNoLand();
UBOOL DisIPNoBreath();
void DisIPNoteInfluence( class UDishonoredCameraInfluenceGroup* Group, class UDishonoredCameraInfluence* Influence, const struct FDishonoredViewTarget& CurVT, FLOAT CurWeight, APawn* Pawn );
void DisIPReportCamera( class ADishonoredPlayerCamera* Camera, const struct FTViewTarget& OutVT, APawn* Pawn, FLOAT DeltaTime );

#endif
