// UDisAISubStateFlee cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. A civilian running from a threat. Bodies in Src/disaisubstateflee.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void BeginDestroy();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }

	// DISHONORED(port): 2012 rvas 0x78c7b0 / 0x773870: setting the threat also subscribes to its termination, which is why
	// FDisAISubStateFlee_Param::OnPending calls this rather than writing the member.
	void SetThreat( class AActor* _pThreat );
	void RegisterDelegate_ThreatTerminated( class UDishonoredAIBehavior* const _pOwningBehavior );
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	void OnReachedFleePoint();
	UBOOL FindNewFleeDestination();
