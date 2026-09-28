// UDisAISubStateInvestigate cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(written): agent DF. "What was that?" - walk to what the attention system flagged, look at it, and report back
// through m_InvestigateHookOutput which bark the behaviour should fire. Bodies in Src/disaisubstateinvestigate.cpp.
public:
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void PauseSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void ResumeSubState_Derived();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

	// DISHONORED(port): the three readers the behaviour callbacks ask about (2012 rvas 0x765650 / 0x765660 / 0x7656a0);
	// agent CG's classification lists all three as blockers of UDisBehaviorSearch's callbacks.
	UBOOL ReachedProxy() const;
	UBOOL WantsStareAtUnreachable() const;
	UBOOL IsRunningInvestigate() const;
	class AActor* GetCorpseBeingInvestigated() const;
	struct FDisAttentionChangeReason GetCurrentInvestigateReason() const;

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
	UBOOL FilterRotationReached( const struct FAIStimStruct_RotationReached& _rStim );
	UBOOL FilterSearchReachedProxy( const struct FAIStimStruct& _rStim );
	FLOAT GetProximityThreshold() const;
	void RefreshInvestigation();
