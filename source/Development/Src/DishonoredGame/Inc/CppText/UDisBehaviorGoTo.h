// UDisBehaviorGoTo cpptext: included inside the generated class body (dishonoredgameclasses.h).
// agent FV (PHASE18 FV): the Kismet go-to (DisSeqAct_AIGoToActor -> ADishonoredNPCController::OnAIGoToActor ->
// FAIStimStruct_GoToRequest -> this). Bodies in Src/disbehaviorgoto.cpp. 2013 rvas (2012 in brackets):
// InitBehavior 0x6edae0 (0x74c7a0), OnBehaviorStart 0x6edb10 (0x74c7d0), BuildBehaviorFilterStimMasks 0x6edb90 (0x74c850),
// BuildFilterStimMask (0x746bc0), BuildEvaluateStimMask 0x6e4d10 (0x745510), BuildShouldFinishWhileDormantStimMask
// (0x7454e0), FilterGoToRequest 0x6f04e0 (0x75d0f0), FilterBehaviorAbort 0x6f0640 (0x75c150),
// ShouldFinishWhileDormantGoToRequest 0x6f0680 (0x75d250), ShouldFinishWhileDormantBehaviorAbort 0x6f06d0 (0x75c190),
// EvaluateGoToRequest 0x6f0700 (0x75d2a0), SetupFromGoToRequest 0x6f0750 (0x75d2f0), GetFilterStimDelegate 0x6fc030
// (0x760eb0), GetShouldFinishWhileDormantDelegate 0x6fc1c0 (0x761040), GetEvaluateStimDelegate 0x6fc200 (0x761080),
// GetSetupFromStimDelegate 0x6fc230 (0x7610b0), FilterInterruptStim 0x6e4c70 (0x7454a0), FilterTouchedAlly 0x6e4c80
// (0x7454b0; retail binds it for TouchedNeutral too and the PDB names it FilterTouchedNeutral in the delegate), FilterTouchedEnemy
// 0x6e4cb0 (0x748e60), FilterKismetAIAction 0x6e8470 (0x746cb0), GetResumingBodyIntentionDesire (0x748e90),
// GetDialogAvailability 0x6e4c50 (0x745480; the base's CallGetDialogAvailability is not ported, so it is not reached).
public:
	virtual void InitBehavior( class UDishonoredAIBrain* const _pAIBrain );
	virtual void OnBehaviorStart();
	virtual const BYTE* BuildEvaluateStimMask();
	virtual const BYTE* BuildFilterStimMask();
	virtual const BYTE* BuildShouldFinishWhileDormantStimMask();
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual FDisStimSetupDelegate GetSetupFromStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetShouldFinishWhileDormantDelegate( BYTE _StimID );
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }

private:
	UBOOL EvaluateGoToRequest( const struct FAIStimStruct_GoToRequest& _rStim ) const;
	UBOOL FilterGoToRequest( const struct FAIStimStruct_GoToRequest& _rStim );
	UBOOL ShouldFinishWhileDormantGoToRequest( const struct FAIStimStruct_GoToRequest& _rStim ) const;
	void SetupFromGoToRequest( const struct FAIStimStruct_GoToRequest& _rStim );
	UBOOL FilterBehaviorAbort( const struct FAIStimStruct_BehaviorAbort& _rStim );
	UBOOL ShouldFinishWhileDormantBehaviorAbort( const struct FAIStimStruct_BehaviorAbort& _rStim ) const;
	UBOOL FilterInterruptStim( const struct FAIStimStruct& _rStim );
	UBOOL FilterKismetAIAction( const struct FAIStimStruct& _rStim );
	UBOOL FilterTouchedNeutral( const struct FAIStimStruct_TouchedNeutral& _rStim );
	UBOOL FilterTouchedEnemy( const struct FAIStimStruct_TouchedEnemy& _rStim );
	void TakeDestinationFromAction( const struct FAIStimStruct_GoToRequest& _rStim );
