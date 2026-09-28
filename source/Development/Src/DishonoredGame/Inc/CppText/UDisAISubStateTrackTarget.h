// UDisAISubStateTrackTarget cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(written): agent DF. "It went that way." The sub-state a guard uses to walk to where it last believed the
// target was, sweeping its head with the drawn-sword search pattern on the way. Bodies in Src/disaisubstatetracktarget.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
	void UpdateCurrentSearchDest();
