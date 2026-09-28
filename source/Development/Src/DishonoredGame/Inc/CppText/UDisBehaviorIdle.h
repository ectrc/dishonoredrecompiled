// UDisBehaviorIdle cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. The fallback behaviour of every NPC and the only one that answers the BrainInit stim.
// DISHONORED(written): agent DF completed it - the three masks, the resume that asks for the Stand sub-state, and the tick
// that keeps the standing focus in step with the conversation system. This is the behaviour that takes an NPC out of
// DisAISubStateInit, because it is the one every brain starts in.
// Bodies in Src/disbehavioridle.cpp.
public:
	virtual const BYTE* BuildEvaluateStimMask();
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );
	virtual const BYTE* BuildFilterStimMask();
	virtual const BYTE* BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual void OnBehaviorResume();
	virtual void TickBehavior( FLOAT _fDeltaSeconds );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }

private:
	UBOOL FilterTeleported( const struct FAIStimStruct& _rStim );
