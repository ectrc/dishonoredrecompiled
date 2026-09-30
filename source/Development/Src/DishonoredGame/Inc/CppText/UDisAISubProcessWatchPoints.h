// UDisAISubProcessWatchPoints cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent EP. The three accessors UDisBehaviorPatrol reads while it stands at a guard post. The rest
// of this sub-process - StartWatchingPoints (2013 rva 0x73a9d0, unnamed there), IncrementWatchPoint (0x73aaf0) and
// TickSubProcess_Derived (0x73e440), which walk the post's ADisGuardWatchPoint list and point the head at each one in
// turn - is NOT ported: it needs ADisGuardWatchPoint::GetWatchDuration and the look-at target desire, and it is a
// separate package. Because nothing therefore ever assigns m_pWatchPointContainer, IsWatchingPoints answers FALSE for
// every post and UDisBehaviorPatrol takes retail's own no-watch-points path - it stands for m_fGuardDuration, or
// forever when the post says so. Bodies in Src/disaisubprocesswatchpoints.cpp.
public:
	UBOOL IsWatchingPoints() const;
	UBOOL HasWatchCycled() const;
	void StopWatchingPoints();
	FLOAT GetRemainingWatchDuration() const { return m_fRemainingWatchDuration; }
