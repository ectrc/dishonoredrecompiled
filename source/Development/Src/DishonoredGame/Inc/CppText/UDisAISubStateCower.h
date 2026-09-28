// UDisAISubStateCower cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. A civilian who has stopped running: hold the panicked stance and, if the slot's tweaks say
// so, keep facing what frightened it. Bodies in Src/disaisubstatecower.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

private:
	UBOOL FilterRotationReached( const struct FAIStimStruct_RotationReached& _rStim );
	UBOOL FilterTopAttnProxyReplaced( const struct FAIStimStruct& _rStim );
