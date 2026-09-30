#pragma once
// DishonoredGame/inc/dishonoredutilities_math.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x631840  class FVector __cdecl DisLerp<float, class FVector>(float const &, float const &, float const &, class FVector const &, class FVector const &)
//   0x6322c0  float __cdecl DisLerp<float, float>(float const &, float const &, float const &, float const &, float const &)
//   0x641770  float __cdecl DisLerpClamped<float, float>(float const &, float const &, float const &, float const &, float const &)

// ---- agent EL (PHASE12 EL) ----
// DISHONORED(port): agent EL, 2013 rva 0x7bf830 (2012 0x827590). Retail declares this beside the rest of
// dishonoredutilities_math.cpp's free functions; only this one has a body in this tree so far.
UBOOL DisTeleportPlayer( const FVector& _rLocation, const FRotator& _rRotation );
