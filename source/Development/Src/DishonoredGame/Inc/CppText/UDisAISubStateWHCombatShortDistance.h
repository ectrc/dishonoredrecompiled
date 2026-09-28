// UDisAISubStateWHCombatShortDistance cpptext: included inside the generated class body (DishonoredGameAIWolfhoundClasses.h).
// DISHONORED(written): agent DF. A wolfhound in biting range. Bodies in Src/disaisubstatewhcombatshortdistance.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }
