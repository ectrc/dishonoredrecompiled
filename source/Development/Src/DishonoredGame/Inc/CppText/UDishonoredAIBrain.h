// UDishonoredAIBrain cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. The brain is the root of DishonoredGame's AI: one per NPC controller, holding the
// behaviour stack, the brain processes, the stim queue and the attention state. Bodies in Src/dishonoredaibrain.cpp
// (core), _senses.cpp, _stealth.cpp, _steering.cpp, _combat.cpp — the same split retail's source has.
//
// Two members of the 2012 build are NOT in retail 2013 and must not be reintroduced from a 2012 decompile:
// m_BrainInhibitors (a TArray<UClass*> at 2012 @224) together with AddBrainInhibitor / RemoveBrainInhibitor /
// IsBrainInhibited, and the 18th behaviour slot — retail 2013's m_ActiveBehaviorStack has 19 entries and every loop
// over it runs 0..18. 2013's InitBrain (rva 0x7252a0) has no inhibitor clear and 2013's TickBrain (0x724a10) has no
// inhibitor gate, which is how both were established. m_MandatoryBehaviors is the 2013-only addition that replaced
// the inhibitor list's slot region.
public:
	void InitBrain( class ADishonoredNPCPawn* const _pNPCPawn, class UDisTweaks_AIBrain* const _pBrainTweaks, BYTE _SuspicionLevel );
	void TerminateBrain();
	void TickBrain( FLOAT _fDeltaSeconds );
	void RefreshBrainThoughts();
	void MarkForRefreshBrainThoughts();
	void Forget();
	UBOOL IsBrainInitialized() const;

	class ADishonoredNPCPawn* GetOwningPawn() const { return m_pOwningPawn; }
	class ADishonoredNPCController* GetOwningController() const { return m_pOwningController; }
	class UDishonoredAIBehavior* GetCurrentBehavior() const { return m_pCurrentBehavior; }

	UBOOL IsCurrentBehavior( class UClass* const _pBehaviorClass ) const;
	UBOOL IsBehaviorOnStack( class UClass* const _pBehaviorClass ) const;
	UBOOL SupportsBehavior( const class UClass* _pBehaviorClass, UBOOL _bCheckStack ) const;
	class UDisTweaks_AIBehavior* GetAIBehaviorTweakForSlot( INT _iBehaviorTweakIndex ) const;
	class UDisAIBrainProcess* GetBrainProcess( class UClass* _pBrainProcessClass );

	UBOOL IsFlagSet( BYTE _Flag ) const;
	void SetFlagTo( BYTE _Flag, UBOOL _bValue );
	BYTE GetSuspicionLevel() const;
	void SetSuspicionLevel( BYTE _SuspicionLevel );
	void EscalateSuspicionLevel();
	UBOOL IsIgnoringTechnologyDanger() const;
	UBOOL IsCombatEngaged() const;
	UBOOL HasEngagedEnemy() const;
	UBOOL IsProtectingNeutrals() const;
	void OverrideProtectNeutrals( UBOOL _bProtect );
	class ADishonoredPawn* GetCurrentEnemy() const;
	void SetCombatRange( FLOAT _fRange );
	void SetDumbFlag( UBOOL _bSet );
	void SetDeafFlag( UBOOL _bSet );
	void SetBlindFlag( UBOOL _bSet );
	void SetNumbFlag( UBOOL _bSet );

	void EnqueueStim( struct FAIStimStruct* _pAddMe );
	void ProcessAllStims();

	virtual void BeginDestroy();

private:
	void InitBrain_Processes();
	void InitBrain_Steering();
	void TermBrain_Steering();
	void TickBrain_Stims( FLOAT _fDeltaTime );
	void TickBrain_Processes( FLOAT _fDeltaSeconds );
	void TickBrain_Senses( FLOAT _fDeltaSeconds );
	void TickBrain_Stealth();
	void TickBrain_Steering( FLOAT _fDeltaSeconds );
	void RefreshBrain_Processes( FLOAT _fTimeSinceLastThought );
	void RefreshBrainThoughts_Steering( FLOAT _fTimeSinceLastThought );
	void RefreshRelationshipStatus();
	void FlushStimQueue( UBOOL* const _bActiveStackCancels, UBOOL* const _bBecamePaused );
	void ProcessOneStim( const struct FAIStimStruct& _rAIStim, UBOOL* const _bActiveStackCancels, UBOOL* const _bBecamePaused );
	void AddBehaviorFromTweak( class UDisTweaks_AIBehavior* const _pBehaviorTweak );
	void StopBehaviorInSlot( INT _Slot, UBOOL _bIsBeingTerminated );
public:
	void OnOtherActorTerminated_AIBrain( const class AActor& _rActor );
	void OnDifficultyChange( const class FArkGameEvent& _rEvent );
	void OnPushedByAvoidable( const class FArkGameEvent& _rEvent );

	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );

	// ---- agent CG: the attention accessors FDisAttentionProxy reads ----
	// All five forward to m_pAttentionProcess and answer the "unaware" default when there is none, which is retail's
	// own behaviour for a brain whose tweaks did not give it the attention process.
	BYTE GetAttentionLevel( const class IDisAttentionTargetInterface* _pForTarget ) const;
	void GetAttentionProxyInfo( const class IDisAttentionTargetInterface* _pForTarget, struct FDisAttentionProxyInfo& _rResult ) const;
	const struct FDisAttentionProxy& GetTopEnemyProxy() const;
	const struct FDisAttentionProxy& GetTopEnemyProxy( BYTE& _rOutTopEnemyAttentionLevel ) const;
	const struct FDisAttentionProxy& GetTopAttnProxy() const { return m_TopAttnTargetProxy; }
	class FDisAIKnowledgeComponent* GetKnowledge() const;
	UBOOL CanBeDormant() const;

	// ---- agent CG: ClearAllMinAttention ----
	/** DISHONORED(port): 2012 rva 0x749d10: what ETransitSpeed the pawn is actually moving at, i.e. the locomotion
	    component's active request speed index mapped back through UDisTweaks_NPCPawn::m_LocomotionSpeedToTransitSpeed.
	    Read by UDisAISubStateTakePosition::RefreshSubState to pick the right row of m_fRotationStartDistance. */
	BYTE GetCurrentDesiredTransitSpeed() const;

	void ClearAllMinAttention( BYTE _LimitType );

	/** DISHONORED(port): agent DN. 2013 rva 0x700d40 (2012 0x749b20, 62 bytes): the current behaviour answers for the
	    whole brain, and the constraints are only asked for when the goals produced something. */
	UBOOL GetPathGoalsAndConstraintsFromBehavior( const FVector& _rFinalDestination, UBOOL _bForReachability, TArray<class UNavMeshPathGoalEvaluator*>& _rOutGoals, TArray<class UNavMeshPathConstraint*>& _rOutConstraints ) const;
// DISHONORED(port): agent EJ (PHASE12 EJ) - the DisSaveLoad pair. GameLoad 2013 rva 0x7256a0 and PostGameLoad
// 0x711c20, retail vtable slots 70 and 71; GameSave (0x717290, slot 69) is not ported. Bodies in
// Src/dishonoredaibrain.cpp. IsSaveable is declared because retail's slot 68 for this
// class is the fold onto UObject::IsRefSaveable's body (0x5ea9d0, return TRUE) - slot 67 is IsRefSaveable
// itself - so a FALSE from this tree would be the absence of an override rather than retail's answer.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
