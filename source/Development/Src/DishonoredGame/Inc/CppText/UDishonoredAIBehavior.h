// UDishonoredAIBehavior cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. A behaviour is one slot of the brain's 19-slot stack. It owns a UDisAISubStateMachine
// (its sub-states) and a list of UDisAISubProcess (things that run beside the state machine), and it is activated by a
// stim through UDishonoredAIBrain::ProcessOneStim. Every UDisBehavior* subclass overrides the virtuals below; the
// ~150 On/Tick/Refresh/RequestStateExitCallback_<SubState> natives of those subclasses are script delegates bound by
// RegisterCallbacks from the sub-state's m_StateSuffix, which is why they only become real once the sub-state machine
// exists. Bodies in Src/dishonoredaibehavior.cpp.
public:
	void CallInitBehavior( class UDishonoredAIBrain* const _pAIBrain, class UDisTweaks_AIBehavior* const _pBehaviorTweaks );
	void CallTickBehavior( FLOAT _fDeltaSeconds );
	void CallRefreshThoughts( FLOAT _fTimeSinceLastThought );
	void CallOnBehaviorPause( UBOOL _bIsBeingTerminated );
	void CallOnBehaviorResume( const struct FDisBodyIntention& _rPreviousBodyIntention );
	UBOOL CallFilterAIStim( const struct FAIStimStruct& _rAIStim );
	UBOOL CallShouldFinishWhileDormant( const struct FAIStimStruct& _rAIStim );
	void OnBecomeDormant();
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );

	class ADishonoredNPCPawn* GetOwningPawn() const;
	class UDisAISubState* GetCurrentSubState() const;
	INT GetActiveSubStateIndex() const;
	INT GetLogicalSubStateIndex() const;
	class UDisAISubProcess* GetSubProcess( class UClass* const _pSubProcessClass ) const;

	class AActor* GetBehaviorActionTargetActor() const;
	UBOOL GetBehaviorActionTargetLocation( FVector& _rOut ) const;

	// The sub-state machine calls these when a sub-state is entered, so every subclass callback sees the same target.
	void RegisterCallbacks( class UDisAISubState* const _pAISubState, DWORD _Flags );

	virtual void BeginDestroy();
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );

	// The per-behaviour virtuals. Retail's base bodies are empty or trivial and the UDisBehavior* subclasses override
	// them; the vtable order here follows the 2012 PDB UDishonoredAIBehavior_vtbl.
	virtual void InitBehavior( class UDishonoredAIBrain* const _pAIBrain ) {}
	virtual void TickBehavior( FLOAT _fDeltaSeconds ) {}
	virtual void RefreshThoughts( FLOAT _fTimeSinceLastThought ) {}
	virtual void OnBehaviorPause( UBOOL _bIsBeingTerminated ) {}
	virtual void OnBehaviorResume() {}
	virtual void OnBehaviorStop( UBOOL _bIsBeingTerminated ) {}
	virtual UBOOL IsBehaviorFinished() const { return FALSE; }
	virtual UBOOL CanBeDormant() const { return TRUE; }
	virtual UBOOL BehaviorIgnoresTechnologyDanger() const { return FALSE; }
	virtual class ADishonoredPawn* GetCurrentEnemy() const;
	virtual BYTE GetAwarenessLevel() const;
	virtual UBOOL IsPlayerAllowedToPushMe() const;
	virtual UBOOL CanBlockSoiree( FGuid _SoireeGuid, BYTE _Priority ) const { return FALSE; }
	virtual class UObject* GetDesires() { return NULL; }
	// DISHONORED(port): 2012 vtable +348, called by UDishonoredAIBrain::ProcessOneStim the moment a behaviour takes its
	// slot. The base body is empty; every UDisBehavior* subclass uses it to request its first sub-state.
	virtual void OnBehaviorStart() {}

	// The three stim delegates a subclass binds per EAIStimID. Retail's base returns the null delegate for every id;
	// a subclass returns one bound to its own Evaluate<Stim> / SetupFrom<Stim> / Filter<Stim> member, which is how a
	// behaviour both decides whether a stim activates it and reads the stim's payload.
	virtual FDisStimPredicateDelegate GetShouldFinishWhileDormantDelegate( BYTE _StimID );
	virtual FDisStimSetupDelegate GetSetupFromStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );

	// The stim masks: one byte per EAIStimID saying whether this behaviour cares. Built once by CallInitBehavior.
	virtual const BYTE* BuildEvaluateStimMask() { return NULL; }
	virtual const BYTE* BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask );
	virtual const BYTE* BuildShouldFinishWhileDormantStimMask() { return NULL; }

	// DISHONORED(written): retail declares the three action-target setters protected and makes UDishonoredAIBrain a
	// friend (ProcessOneStim, ProcessAllStims and TerminateBrain all call ClearActionTarget). Public here rather than a
	// friend declaration, because the generated class body cannot carry one.
	void SetActionTargetActor( class AActor* _pActor );
	void SetActionTargetProxy( struct FDisAttentionProxy _ActionTarget );
	void ClearActionTarget();
