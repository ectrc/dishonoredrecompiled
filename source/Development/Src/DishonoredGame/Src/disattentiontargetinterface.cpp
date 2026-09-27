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
