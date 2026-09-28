// UDisAISubStateMeleeEngage cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. At melee range: hold the formation position the attack manager assigns, face the enemy and
// keep the weapon up. Bodies in Src/disaisubstatemeleeengage.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

private:
	void SetFormationPosition();
