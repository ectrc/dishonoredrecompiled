// UDisAISubStateMenace cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. Standing off and taunting: face the enemy, disengage from combat, and fire a taunt
// animation on the pawn's own state machine every m_fTauntMinTime..m_fTauntMaxTime seconds.
// Bodies in Src/disaisubstatemenace.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }
