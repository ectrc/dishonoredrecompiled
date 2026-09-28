// UDisAISubStateLieInWait cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. An assassin at an ambush point, waiting. Its ten-stim filter mask is the widest of any
// sub-state, because almost anything - a noise, a touch, damage, a teleport, the plague - is a reason to stop waiting.
// Bodies in Src/disaisubstatelieinwait.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

private:
	UBOOL FilterAbortingStim( const struct FAIStimStruct& _rStim );
	UBOOL FilterHeardSomething( const struct FAIStimStruct& _rStim );
	UBOOL FilterRotationReached( const struct FAIStimStruct_RotationReached& _rStim );
	UBOOL FilterTeleported( const struct FAIStimStruct& _rStim );
	void TickState_Waiting( FLOAT _fDeltaSeconds );
	void TickState_DoneTurning( FLOAT _fDeltaSeconds );
	void TickState_Ambushing( FLOAT _fDeltaSeconds );
