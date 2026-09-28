// UDisAISubStateGenericAction cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Play this action and tell me when it is done." The sub-state that makes an NPC do
// something scripted - open a door, look at a note, bang on a wall - by requesting an anim action on the pawn's own
// FSM and waiting for the end notification. Bodies in Src/disaisubstategenericaction.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void PauseSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void OnExitState( class UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );

	// DISHONORED(port): 2012 rvas 0x768df0 / 0x768e20: the per-object action-end event the pawn raises (dispatcher event
	// type 8), and the load-time restart.
	void OnNpcHActionEnded( const class FArkGameEvent& _rEvent );
	void PostGameLoad_GenericAction();

private:
	UBOOL RequestActionToPlay();
	void KillAction();
	UBOOL FilterIncomingAttack( const struct FAIStimStruct& _rStim );
