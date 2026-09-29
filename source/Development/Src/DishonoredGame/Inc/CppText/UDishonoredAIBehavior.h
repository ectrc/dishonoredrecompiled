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
	// DISHONORED(port): agent DF - the return type is the interface (retail: IDisDesiresInterface*); agent CG declared it
	// as UObject* while IDisDesiresInterface had no methods to call. UDisAIBehaviorWithDesires overrides it with `this`.
	virtual class IDisDesiresInterface* GetDesires() { return NULL; }
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

	/*-------------------------------------------------------------------------
		DISHONORED(port): agent DF. Requesting a sub-state, which is the one thing every UDisBehavior* subclass does and
		the reason its callbacks exist. Retail spells both of these as member templates over the behaviour's own tweaks
		class and the target sub-state's tweaks class, and the shipped exe therefore carries one instantiation per
		(behaviour, sub-state) pair - 28 of RequestSubStateChange and 3 of AreSubstatePreconditionsMet, all 135 and 264
		bytes (2012 rvas 0x740000 for <Idle, Stand> and 0x73fcb0 for <EnemyUnreachable, DoWeaponManoeuver>).

		The slot index is the whole trick: a behaviour's tweaks carry an ARRAY of sub-state tweaks, one per slot, and the
		slot the caller names selects both which sub-state object the machine changes to and which settings that
		sub-state is given while it is in that slot. That is how one UDisAISubStateStand class behaves differently as a
		guard's standing post and as a shooter's firing stance.

		The bodies are in Inc/disaisubstate.h, after the generated classes: they need UDisAISubStateMachine and
		UDisAISubState complete, and this cpptext is included inside UDishonoredAIBehavior, which the generator declares
		before both.
	-------------------------------------------------------------------------*/
	template< class BehaviorTweaksType, class SubStateTweaksType >
	void RequestSubStateChange( BYTE _SubStateArrayIndex, struct FDisNativeStateParam& _rAISubStateParam );

	template< class BehaviorTweaksType, class SubStateTweaksType >
	UBOOL AreSubstatePreconditionsMet( const BYTE _SubStateArrayIndex, struct FDisNativeStateParam& _rAISubStateParam ) const;

	/** DISHONORED(port): 2012 rva 0x74c450 (agent DF). Rolls every sub-process's filter mask into the sub-processes mask
	    and the complete mask, and every sub-state's into the complete mask only - so CallFilterAIStim can reject a stim
	    that nothing anywhere under this behaviour cares about with one array lookup. */
	void BuildInternalFilterStimMasks( BYTE* _pSubProcessesFilterStimMask, BYTE* _pCompleteFilterStimMask ) const;

	/*-------------------------------------------------------------------------
		agent DN: the goal evaluators and constraints the AI hands the nav mesh.

		This is the second of agent CG's "four things and no more" the AI needs from the navigation-mesh runtime. The
		behaviour gets first refusal, then the live sub-state, and the default pair is added only when BOTH of them said
		yes - which is how a fleeing sub-state can replace "walk to this point" with "walk away from it" without the
		behaviour above it knowing.
	-------------------------------------------------------------------------*/
public:
	/** DISHONORED(port): 2013 rva 0x6eabc0 (2012 0x7489c0): UNavMeshGoal_At at the destination, out of the world info's
	    evaluator cache, with bKeepPartial set so a blocked path still returns its best prefix. */
	static void GetDefaultPathGoals( const FVector& _rFinalDestination, TArray<class UNavMeshPathGoalEvaluator*>& _rOutGoals );
	/** DISHONORED(port): 2013 rva 0x6eaca0 (2012 0x748aa0): UNavMeshPath_Toward the destination. */
	static void GetDefaultPathConstraints( const FVector& _rFinalDestination, TArray<class UNavMeshPathConstraint*>& _rOutConstraints );
	/** DISHONORED(port): 2013 rva 0x6eac30 (2012 0x748a30) / 0x6ead10 (0x748b00). */
	UBOOL CallGetPathGoals( const FVector& _rFinalDestination, TArray<class UNavMeshPathGoalEvaluator*>& _rOutGoals ) const;
	UBOOL CallGetPathConstraints( const FVector& _rFinalDestination, UBOOL _bForReachability, TArray<class UNavMeshPathConstraint*>& _rOutConstraints ) const;
	/** The base answers TRUE, i.e. "I have nothing to add, use the default". */
	virtual UBOOL GetPathGoals( const FVector& _rFinalDestination, TArray<class UNavMeshPathGoalEvaluator*>& _rOutGoals ) const { return TRUE; }
	virtual UBOOL GetPathConstraints( const FVector& _rFinalDestination, UBOOL _bForReachability, TArray<class UNavMeshPathConstraint*>& _rOutConstraints ) const { return TRUE; }
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad 2013 rva 0x6f6f80 and PostGameLoad 0x6e8250, retail
// vtable slots 70 and 71. Slot 69 is the UDisAttentionInfo_Base::GameSave fold (0x88af60) and is not ported.
// Bodies in Src/dishonoredaibehavior.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
