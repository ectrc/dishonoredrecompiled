// UDisAISubState cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): agent CG. A sub-state is one state of a behaviour's UDisAISubStateMachine, i.e. the unit of
// "what the NPC is doing right now" (Stand, TakePosition, Investigate, MeleeChase, FirePistol, ...). It is a
// UDishonoredNativeState, so the machine drives it through agent AJ's FSM; on top of that it forwards each transition to
// the owning behaviour through one of six FScriptDelegate hooks (see RegisterDelegate_*), which is how the
// UDisBehavior*::OnEnterCallback_<Suffix> / TickCallback_<Suffix> / ... natives are reached.
// Retail vtable tail (2012 PDB UDisAISubState_vtbl +368..+416, after UDishonoredNativeState's own tail; the base bodies
// are identical-code folded, which is how their return values were read off): ArePreconditionsMet_Derived (TRUE),
// InitSubState_Derived, BeginSubState_Derived, ResumeSubState_Derived (empty), PauseSubState_Derived,
// EndSubState_Derived (empty), RefreshSubState (ported), BuildFilterStimMask (NULL),
// GetFilterStimDelegate_SubState (the null delegate), GetPathConstraints (TRUE), GetPathGoals (TRUE),
// OnOtherActorTerminated_AISubState_Derived (empty), GetDesires (NULL).
public:
	// DISHONORED(port): the tweak-interface pair, 2012 vtable UDisAISubState{for IDisTweaksInterface} slots 4 and 5
	// (GetTweaks_Derived folded onto UDistributionFloatUniformCurve::GetNumKeys 0x9bd380, SetTweaks_Derived named at
	// 0x16f40 / 2013 0x7059f0; both are a single load/store at interface+0xC, i.e. object+76 = m_pSubStateTweaks).
	// Same reason as UDisAISubProcess's: without it a sub-state reads the all-zero class default of its tweaks class.
	// DISHONORED(retail): UDisAISubStateInit re-overrides both back to NULL and a no-op (its own interface vtable slots 4
	// and 5 are the folded return-0 and empty bodies), because the idle sub-state has no tweaks object of its own.
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks );

	virtual UBOOL ArePreconditionsMet_Derived() { return TRUE; }
	virtual void InitSubState_Derived() {}
	virtual void BeginSubState_Derived() {}
	virtual void ResumeSubState_Derived() {}
	virtual void PauseSubState_Derived( UBOOL bIsBeingTerminated ) {}
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated ) {}
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask() { return NULL; }
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID ) { return FDisStimPredicateDelegate(); }
	virtual UBOOL GetPathConstraints( const FVector& Destination, UBOOL bIsFleeing, TArray<class UNavMeshPathConstraint*>& OutConstraints ) { return TRUE; }
	virtual UBOOL GetPathGoals( const FVector& Destination, TArray<class UNavMeshPathGoalEvaluator*>& OutGoals ) { return TRUE; }
	virtual void OnOtherActorTerminated_AISubState_Derived( const AActor& Actor ) {}
	virtual class IDisDesiresInterface* GetDesires() { return NULL; }

	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual UBOOL OnResetState();
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void BeginDestroy();

	void InitSubState( UDishonoredAIBehavior* const OwningBehavior, UDisTweaks_AISubState* const SubStateTweaks );
	UBOOL ArePreconditionsMet( struct FDisNativeStateParam& Params );
	void OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention );
	void OnOwningBehaviorPause( UBOOL bIsBeingTerminated );
	void OnOtherActorTerminatedEvent( const struct FArkGameEvent& Event );
	void PostGameLoad_SubState();

	/** The six delegate hooks. The suffix form takes the string the behaviour composes; the two that take the behaviour
	    alone compose it themselves from m_StateSuffix. */
	void RegisterDelegate_OnEnterCallback( UObject* Owner, const FString& Suffix );
	void RegisterDelegate_TickCallback( UObject* Owner, const FString& Suffix );
	void RegisterDelegate_OnExitCallback( UObject* Owner, const FString& Suffix );
	void RegisterDelegate_OnResetCallback( UObject* Owner, const FString& Suffix );
	void RegisterDelegate_RefreshCallback( UDishonoredAIBehavior* const OwningBehavior );
	void RegisterDelegate_RequestStateExitCallback( UDishonoredAIBehavior* const OwningBehavior );

protected:
	virtual void RequestStateExit_Derived();
	void SetActionTargetProxy( FDisAttentionProxy ActionTarget );
	void ClearActionTargetProxy();
