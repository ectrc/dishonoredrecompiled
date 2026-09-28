// UDisAISubStateTakePosition cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Walk to there, then turn." The sub-state behind every patrol step, every investigation
// approach and every go-home: it states one loco desire, waits for the DestinationReached stim its own request raised,
// and only then - or earlier, depending on the slot's m_eRotationStarts tweak - states the face-to desire. It leaves by
// asking its machine to exit once both halves are done, which is what makes a patrol advance to its next point.
// Bodies in Src/disaisubstatetakeposition.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void BeginDestroy();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

	// DISHONORED(port): 2013 rvas 0x727d80 (2012 0x765990) / 0x727d90 (0x7659a0) / 0x73cbe0 (0x769620) / 0x741c90 (0x788650)
	UBOOL DestinationReached() const;
	void SetFullSpeed( UBOOL _bFullSpeed );
	FVector GetDestinationTarget() const;
	void StartRotation();
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
	UBOOL FilterRotationReached( const struct FAIStimStruct_RotationReached& _rStim );
