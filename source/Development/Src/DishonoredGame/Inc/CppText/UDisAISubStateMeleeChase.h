// UDisAISubStateMeleeChase cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. Closing on an enemy with a drawn weapon: a loco desire onto the enemy proxy, refreshed as
// it moves, and the Equipped stance. Bodies in Src/disaisubstatemeleechase.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }
