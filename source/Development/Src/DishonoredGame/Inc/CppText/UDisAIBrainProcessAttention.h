// UDisAIBrainProcessAttention cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). The attention brain process: one slot per thing this brain has ever paid attention
// to (an FDisAttentionSlot: the target and its UDisAttentionInfo_Base), ticked every brain tick, raising the brain's top
// attention target (HandleTopAttnTarget -> UDishonoredAIBrain::SetTopAttnTarget, the asker of the head-track look-at).
// 2012 PDB UDisAIBrainProcessAttention_vtbl{for UObject} slots 72..80 (build/agentGI/vt2012_attention.txt): BuildFilterStimMask,
// InitBrainProcess_Derived, TickBrainProcess_Derived, (RefreshBrainProcess_Derived, folded), TermBrainProcess_Derived,
// OnOtherActorTerminated_AIBrainProcess_Derived, OnDifficultyChange_AIBrainProcess_Derived, PostGameLoad_BrainProcess_Derived,
// GetFilterStimDelegate_BrainProcess. Bodies in Src/disaibrainprocessattention.cpp; 2012 / 2013 rvas at each body.
public:
	// -gidiag: the destruction order at exit (the GI2 exit crash hunt).
	virtual void FinishDestroy();
	BYTE GetAttentionLevel( const class IDisAttentionTargetInterface* _pForTarget ) const;
	void GetAttentionProxyInfo( const class IDisAttentionTargetInterface* _pForTarget, struct FDisAttentionProxyInfo& _rResult ) const;
	UBOOL ShouldKeepAlarmRinging( const class IDisAttentionTargetInterface* _pForTarget ) const;
	void GetTargetsAtAttentionLevel( BYTE _AttentionLevel, TArray< class IDisAttentionTargetInterface*, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Targets,
		BYTE _LifeStateFilter, BYTE _RelationshipFilter, BYTE _MinStatus ) const;
	class UDisTweaks_PawnAttention* FindAttentionTweaks( const class IDisAttentionTargetInterface* const _pTarget ) const;
	class UDisTweaks_SimpleAttention* FindSimpleAttentionTweaks( const class IDisAttentionTargetInterface* const _pTarget ) const;
	UBOOL ShouldIgnoreDeathOf( const class IDisAttentionTargetInterface* const _pTarget, UBOOL* _pOut_bCanDeathBeWitnessed ) const;
	UBOOL ShouldSeeBodyAsHandled( const class IDisAttentionTargetInterface* const _pTarget ) const;
	UBOOL HandleWitnessedMagic( class IDisAttentionTargetInterface* _pMagicUserTarget );
	void TransferAttentionForTarget( class UDishonoredAIBrain* _pSourceBrain, class IDisAttentionTargetInterface* const _pTarget );

	// Retail has these private and makes the three UDisAttentionInfo_* classes friends; the generated class body cannot
	// carry a friend declaration, so the entry points the infos use are public here.
	void BuildAttnCategoryArray( struct FAttentionTweakCategoryInfo* const _TweakCategoryInfos ) const;
	INT FindAttentionTweakCategory( BYTE _RelationshipToTarget ) const;
	class UDisAttentionInfo_Base* FindAttentionInfo( const class IDisAttentionTargetInterface* _pTarget, INT* _pIndex ) const;
	class UDisAttentionInfo_Base& GetAttentionInfo( class IDisAttentionTargetInterface* _pTarget );
	void ClearAllMinAttention( BYTE _Type );
	void BumpAttention( class IDisAttentionTargetInterface* _pForTarget, const struct FDisAttentionIncrease& _rIncrease, const struct FDisAttentionChangeReason& _rReason, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo );
	class UDisTweaks_AIBrainProcess_Attention* GetAttentionProcessTweaks() const;

protected:
	virtual void InitBrainProcess_Derived();
	virtual void TermBrainProcess_Derived();
	virtual void TickBrainProcess_Derived( FLOAT _fDeltaSeconds );
	virtual void PostGameLoad_BrainProcess_Derived();
	virtual void OnOtherActorTerminated_AIBrainProcess_Derived( const class AActor& _rActor );
	virtual void OnDifficultyChange_AIBrainProcess_Derived();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_BrainProcess( BYTE _StimID );

private:
	void TickAttention( FLOAT _fDeltaSeconds );
	void TickAttention_Messages( class IDisAttentionTargetInterface* _pAttnTarget, const TArray< struct DisAttention::FDisAttentionChangeMessage, TMemStackAllocator< GMainThreadMemStack > >& _rMessages );
	void TickAttention_Kismet( class AActor* _pAttnTargetActor, BYTE _OldAttentionLevel, BYTE _NewAttentionLevel );
	void TickAttention_UpdateLatestAttentionTag( const struct FDisAttentionSlot& _rCurrentSlot );
	void TickAttention_ReactToCorpses( const struct FDisAttentionSlot& _rCurrentSlot, BYTE _WitnessDeathRequest );
	void TickAttention_Empathy( TArray< class IDisAttentionTargetInterface*, TMemStackAllocator< GMainThreadMemStack > >& _rNonEnemyTargets );
	void TickAttention_ReactToAlly( const class IDisAttentionTargetInterface* _pNonEnemyTarget );
	void TickAttention_CheckForMurder( class IDisAttentionTargetInterface* _pNonEnemyTarget );
	void TickAttention_CheckForCarryingCorpse( class IDisAttentionTargetInterface* _pNonEnemyTarget );
	void UpdateSoireeAttention();
	void HandleTopAttnTarget();
	void MarkActiveReactionContexts( struct FDisAttentionSlot& _rAttentionSlot, const TArray< struct FDisAppliedAttentionChangeInfo, TMemStackAllocator< GMainThreadMemStack > >& _rAttentionChangeInfos,
		TArray< class IDisAttentionTargetInterface*, TMemStackAllocator< GMainThreadMemStack > >& _rOut_ActiveContexts );
	void CheckAttentionReactions( const TArray< class IDisAttentionTargetInterface*, TMemStackAllocator< GMainThreadMemStack > >& _rFirstTargetsWithActiveContext );
	void DoAttentionReactions( class IDisAttentionTargetInterface* _pTargetThatCausedReaction, const struct FDisAttentionReaction& _rReaction );
	void RemoveAttentionInfoIdx( INT _Index );
	UBOOL CanHearAuralRequest( const TScriptInterface< class IDisAttentionTargetInterface >& _pRequestTarget ) const;

	UBOOL FilterTargetSighted( const struct FAIStimStruct_TargetSighted& _rStim );
	UBOOL FilterTargetUnsighted( const struct FAIStimStruct_TargetUnsighted& _rStim );
	UBOOL FilterTargetTouched( const struct FAIStimStruct_TargetTouched& _rStim );
	UBOOL FilterIncomingDamage( const struct FAIStimStruct_IncomingDamage& _rStim );
	UBOOL FilterMaxOutAttention( const struct FAIStimStruct_MaxOutAttention& _rStim );
	UBOOL FilterPsychicAttentionEnabled( const struct FAIStimStruct_PsychicAttentionEnabled& _rStim );
	UBOOL FilterPsychicAttentionDisabled( const struct FAIStimStruct_PsychicAttentionDisabled& _rStim );
	UBOOL FilterPsychicAttentionDisabled_AllTargets( const struct FAIStimStruct_PsychicAttentionDisabled_AllTargets& _rStim );
	UBOOL FilterSetMinAttentionForTarget( const struct FAIStimStruct_SetMinAttentionForTarget& _rStim );
	UBOOL FilterClearMinAttentionForTarget( const struct FAIStimStruct_ClearMinAttentionForTarget& _rStim );
	UBOOL FilterClearAllMinAttention( const struct FAIStimStruct_ClearAllMinAttention& _rStim );
	UBOOL FilterClearAttention( const struct FAIStimStruct_ClearAttention& _rStim );
	UBOOL FilterNoticeRequest( const struct FAIStimStruct_NoticeRequest& _rStim );
	UBOOL FilterSearchRequest( const struct FAIStimStruct_SearchRequest& _rStim );
	UBOOL FilterHelp( const struct FAIStimStruct_Help& _rStim );
	UBOOL FilterHelpRequest( class ADishonoredNPCPawn* _pRequester, class ADishonoredPawn* _pTroublemaker, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo );
	UBOOL FilterSuspicionLevelChanged( const struct FAIStimStruct_SuspicionLevelChanged& _rStim );
	UBOOL FilterDialogAttentionChange( const struct FAIStimStruct_DialogAttentionChange& _rStim );
	UBOOL FilterTraveled( const struct FAIStimStruct_Traveled& _rStim );
	UBOOL FilterHeardSomething( const struct FAIStimStruct_HeardSomething& _rStim );
	UBOOL FilterMeleeConnected_Outgoing( const struct FAIStimStruct_MeleeConnected_Outgoing& _rStim );
	UBOOL FilterEvadedMelee_Incoming( const struct FAIStimStruct_EvadedMelee_Incoming& _rStim );
	UBOOL FilterEvadedMelee_Outgoing( const struct FAIStimStruct_EvadedMelee_Outgoing& _rStim );
	UBOOL FilterForceRingAlarm( const struct FAIStimStruct_ForceRingAlarm& _rStim );
	UBOOL FilterEscapedBeingChoked( const struct FAIStimStruct_EscapedBeingChoked& _rStim );
	UBOOL FilterDoorUsedByPlayerWhileWary( const struct FAIStimStruct_DoorUsedByPlayerWhileWary& _rStim );
	UBOOL FilterDeathByWoL( const struct FAIStimStruct_DeathByWoL& _rStim );
	UBOOL FilterAlarm( const struct FAIStimStruct_Alarm& _rStim );
	void HandleAttackedEnemy( class ADishonoredPawn* _pEnemy );
	void HandleSearchRequest( class ADishonoredPawn* _pSearchTarget, BYTE _SearchRequestCause, BYTE _SearchRequesterTransitSpeed, const struct FDisAttentionIncreaseInfo& _rIncreaseInfo );
	void HandleHearSomething( const struct FDisAINoiseParameters& _rNoiseParams );
	void HandleHearAlarm( class IDisAttentionTargetInterface* _pTroubleMaker, class ADisAlarmBell& _rAlarm, INT _AttentionTag );
	void HandleDeathByWoL( FVector _Location, class ADishonoredPawn* _pSeenPawn );
