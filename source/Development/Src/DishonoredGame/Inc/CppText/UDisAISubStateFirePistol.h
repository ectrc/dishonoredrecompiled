// UDisAISubStateFirePistol cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. A guard shooting. Its own five-state machine (m_AIPistolState: Relaxed, Aiming, Firing,
// Recovering, Reloading) is what makes an NPC raise, hold, fire and lower a pistol rather than snapping between poses.
// Bodies in Src/disaisubstatefirepistol.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void BeginDestroy();
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );
	void ForceStateExit();
	FVector GetTargetLocation() const;
	UBOOL UsingLastSeenLocation() const;

protected:
	virtual void RequestStateExit_Derived();

private:
	void ChangePistolState( BYTE _NewState );
