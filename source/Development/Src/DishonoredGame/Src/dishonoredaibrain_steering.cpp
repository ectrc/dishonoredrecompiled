// DishonoredGame/src/dishonoredaibrain_steering.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG): the brain's steering pass ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x70c2e0 (2012 0x74a860, 324 bytes): each steering influence is ticked, its faded
// contribution is summed into m_FinalSteeringForce, and the result is handed to the pawn's locomotion component as a
// steering force (FArkComponentLocomotion::AddSteeringForce, 2013 rva 0x53dfe0).
// DISHONORED(bringup): two things are missing and both are named in agentCG.md. The UDisSteeringInfluence classes are
// generated with their members but have no bodies in the tree (no cpptext, no unit), so an influence cannot compute its
// contribution; and FArkComponentLocomotion is the AI root this package does not own, so there is nothing to hand a
// force to. The sum is therefore left at zero. InitBrain_Steering does wire m_pSteeringInfluence_Combat /
// _EnemyPush / _Danger and their owner and fade speed, so the moment the influences get bodies this pass is a
// transcription of the retail loop.
void UDishonoredAIBrain::TickBrain_Steering( FLOAT _fDeltaSeconds )
{
	m_FinalSteeringForce = FVector( 0.f, 0.f, 0.f );
}

// DISHONORED(port): 2013 rva 0x70c270 (2012 0x74a7f0, 103 bytes): the thought-clock half, which refreshes each
// influence's settings from the brain tweaks' m_pSteeringTweaks rather than recomputing the force.
// DISHONORED(bringup): same two gaps as TickBrain_Steering.
void UDishonoredAIBrain::RefreshBrainThoughts_Steering( FLOAT _fTimeSinceLastThought )
{
}
