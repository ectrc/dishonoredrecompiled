// DishonoredGame/src/disattentiontargetinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6): the three virtuals below plus the class-registration helpers.

// ---- agent CG ports (PHASE9 CG): IDisAttentionTargetInterface ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7488f0 (2012 0x78de90, 22 bytes): the base focal point is simply the target's location.
// A pawn overrides this with its eye or its chest socket.
FVector IDisAttentionTargetInterface::GetAttnTargetFocalPoint() const
{
	return GetAttnTargetLocation();
}

// DISHONORED(port): 2013 rva 0x748910 (2012 0x78deb0, 24 bytes): the base focal height is the Z half-extent of the
// target's own attention extent, i.e. how far its focal point sits above its feet.
FLOAT IDisAttentionTargetInterface::GetAttnTargetFocalHeight() const
{
	return GetAttnTargetExtent().Z;
}

// ---- agent GI (PHASE20 GI): the base folds of the two slots every target answers ----
// The interface's own bodies are the identical-code folds the PDB does not name separately: the parameter block is
// filled from the other slots, and a target with no relationship interface is neutral to every relator.
void IDisAttentionTargetInterface::CollectAttnTargetParams( FDisAttnTargetParams& _rOut_Params )
{
	_rOut_Params.m_pTargetTweaks = (FPointer)GetAttnTargetTweaks();
	_rOut_Params.m_pObservableComponent = (FPointer)GetObservableComponent();
	_rOut_Params.m_Location = GetAttnTargetLocation();
	_rOut_Params.m_Velocity = GetAttnTargetVelocity();
	_rOut_Params.m_Extent = GetAttnTargetExtent();
	_rOut_Params.m_FocalPoint = GetAttnTargetFocalPoint();
	_rOut_Params.m_bIsPlayer = IsAttnTargetPlayer();
	_rOut_Params.m_bIsDead = IsAttnTargetDead();
	_rOut_Params.m_bIsCombatable = IsAttnTargetCombatable();
	_rOut_Params.m_pActor = GetAttnTargetActor();
}

BYTE IDisAttentionTargetInterface::GetAttnTargetIncomingRelationship( const IDisRelationshipInterface* ) const
{
	return (BYTE)Relationship_Neutral;
}
