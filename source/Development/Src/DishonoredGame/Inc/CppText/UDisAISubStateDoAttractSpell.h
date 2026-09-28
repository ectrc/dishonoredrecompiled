// UDisAISubStateDoAttractSpell cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. An assassin casting the pull spell at you. Bodies in Src/disaisubstatedoattractspell.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

	UBOOL FilterAttackedByEnemy( const struct FAIStimStruct& _rStim );
