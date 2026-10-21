// DishonoredGame/Inc/jddiag.h
// DISHONORED(diag): agent JD (PHASE24 JD). -jddiag[=census,walk,turn,avoid] (no list = all), -jdwatch=<pawn> adds a pawn to the
// watched pair (the post-ride captain DishonoredNPCPawn_26 and Worker B DishonoredNPCPawn_3). -jdnomoderate declines
// ModerateMovePawn (agent JD's build-2 A/B; build 1 had -jdoldpath / -jdnoavoid / -jdoldreach, removed).
#pragma once

UBOOL DisJDDiag( const TCHAR* _Topic );
UBOOL DisJDSwitch( const TCHAR* _Switch );
UBOOL DisJDWatched( const AActor* _pActor );
void DisJDCensus();
