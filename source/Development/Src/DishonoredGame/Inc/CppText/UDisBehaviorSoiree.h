// UDisBehaviorSoiree cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent IC (PHASE23 IC). The behaviour a brain runs while its pawn plays in a matinee: the
// FAIStimStruct_Soiree start stim from ADishonoredNPCPawn::MAT_BeginAIGroup activates it, it keeps the brain quiet while the
// pawn's matinee controller list holds a non-blocking entry at its tweaks' priority, and it finishes when that list empties
// (MAT_FinishAIGroup's MatineeControllerRemove) or a filter forces it. Bodies in Src/disbehaviorsoiree.cpp; rvas there.
public:
	virtual void OnBehaviorStart();
	virtual void OnBehaviorStop( UBOOL _bIsBeingTerminated );
	virtual void OnBehaviorResume();
	virtual void TickBehavior( FLOAT _fDeltaSeconds );
	virtual UBOOL IsBehaviorFinished() const;
	virtual UBOOL CanBlockSoiree( FGuid _SoireeGuid, BYTE _Priority ) const;
	virtual UBOOL CanBeSurprised() const;
	virtual const BYTE* BuildEvaluateStimMask();
	virtual const BYTE* BuildFilterStimMask();
	virtual const BYTE* BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask );
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual FDisStimSetupDelegate GetSetupFromStimDelegate( BYTE _StimID );
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return &m_BodyIntentionRequest; }
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const;
	BYTE GetSoireePriority() const;

private:
	const class UDisTweaks_AIBehavior_Soiree* GetSoireeTweaks() const;
	UBOOL BehaviorShouldFinish() const;
	UBOOL EvaluateSoiree( const struct FAIStimStruct_Soiree& _rSoireeStim ) const;
	void SetupFromSoiree( const struct FAIStimStruct_Soiree& _rSoireeStim );
	UBOOL FilterSoiree( const struct FAIStimStruct_Soiree& _rSoireeStim );
	UBOOL FilterGoToRequest( const struct FAIStimStruct_GoToRequest& _rStim );
	UBOOL FilterBlockingDialogRequest( const struct FAIStimStruct_BlockingDialogRequest& _rStim );
	UBOOL FilterBlockAtHighPriority( const struct FAIStimStruct& _rStim );
	UBOOL FilterFinishAtLowPriority( const struct FAIStimStruct& _rStim );
	UBOOL FilterReactions( const struct FAIStimStruct& _rStim );
	UBOOL FilterActorTamperedWith( const struct FAIStimStruct& _rStim );
	UBOOL FilterDeathByWoL( const struct FAIStimStruct& _rStim );
	UBOOL FilterReactionRequest( const struct FAIStimStruct_ReactionRequest& _rStim );
	UBOOL FilterTopAttnProxyUpdated( const struct FAIStimStruct_TopAttnProxyUpdated& _rStim );
	UBOOL FilterWitnessMagic( const struct FAIStimStruct_WitnessMagic& _rStim );
