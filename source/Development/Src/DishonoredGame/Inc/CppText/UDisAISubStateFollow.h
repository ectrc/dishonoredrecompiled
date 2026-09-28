// UDisAISubStateFollow cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. Walking behind another pawn at a given angle and distance - the escort and the "come with
// me" order. It is the only sub-state that uses the loco layer's follow form, and the only one with a delegate of its own
// besides the six every sub-state has: NoMoreTargetCallback, fired when the followed pawn dies.
// Bodies in Src/disaisubstatefollow.cpp.
public:
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }

	// DISHONORED(port): 2012 rvas 0x787d70 / 0x773970. Configure takes the follow parameters, or the slot's tweaks when the
	// caller passes none; the delegate registration composes "NoMoreTargetCallback" + m_StateSuffix, exactly as the six
	// base callbacks do (UDisAISubState::RegisterDelegate_*).
	void Configure( class ADishonoredPawn* const _pFollowedPawn, const struct FFollowParameters* const _pOverwriteFollowParameters );
	void RegisterDelegate_NoMoreTarget( class UDishonoredAIBehavior* const _pOwningBehavior );
