// UDisAISubStateDoWeaponManoeuver cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. One weapon move - a lunge, a parry-break, a grenade throw - driven entirely by the item
// context it starts and ended by the ItemContext_End stim that context raises.
// Bodies in Src/disaisubstatedoweaponmanoeuver.cpp.
public:
	virtual UBOOL ArePreconditionsMet_Derived();
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

	void PostGameLoad_DoWeaponManoeuver();

private:
	UBOOL FilterItemContext_End( const struct FAIStimStruct& _rStim );
