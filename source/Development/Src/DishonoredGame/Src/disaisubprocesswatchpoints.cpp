// DishonoredGame/src/disaisubprocesswatchpoints.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent EP ports (PHASE14 EP): the three accessors the patrol behaviour reads ----
//
// See Inc/CppText/UDisAISubProcessWatchPoints.h for what is deliberately left out and what that costs.

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x72b860 (2012 0x78f0b0): a post is being watched only while a container is held AND that
// container still names at least one watch point.
UBOOL UDisAISubProcessWatchPoints::IsWatchingPoints() const
{
	return m_pWatchPointContainer && m_pWatchPointContainer->m_WatchPoints.Num() > 0;
}

// DISHONORED(port): 2013 rva 0x7289b0 (2012 0x78d250)
UBOOL UDisAISubProcessWatchPoints::HasWatchCycled() const
{
	return m_bWatchHasCycled ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x728980 (2012 0x78d220)
void UDisAISubProcessWatchPoints::StopWatchingPoints()
{
	m_pWatchPointContainer = NULL;
	m_fRemainingWatchDuration = 0.f;
}
