// UDisAISubStateStand cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Stand here, facing that." The simplest sub-state that actually does something, and the
// one every idle, guard, patrol, interact and shoot behaviour spends most of its time in: it states one loco desire for a
// spot and one face-to desire for a focus, and re-states them whenever something could have moved the pawn.
// Bodies in Src/disaisubstatestand.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void PauseSubState_Derived( UBOOL bIsBeingTerminated );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

	// DISHONORED(port): 2013 rvas 0x7127b0 / 0x7127a0 (2012 0x7885e0 / 0x7658b0): the idle behaviour turns the facing off
	// while the pawn is in a conversation that wants it to look elsewhere (UDisBehaviorIdle::TickBehavior, 2012 0x72b920).
	void DisableRotationFocus( UBOOL _bDisable );
	UBOOL IsRotationFocusDisabled() const;

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPossibleMovement( const struct FAIStimStruct& _rStim );
	void EnsureProperLocation();
	void EnsureProperRotation();
