// UDisAISubStateMachine cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): agent CG. One per behaviour: agent AJ's UDishonoredNativeStateMachine specialised for
// UDisAISubStates. It adds the "logical" index pair the behaviours address their states by (a behaviour's tweaks list
// sub-states by slot, so a request carries both the state class, through the FDisNativeStateParam, and the slot index,
// through m_PendingAISubStateIndex) and the pending sub-state tweaks that PostStateChange_Derived applies at the moment
// the change lands. Retail vtable: only the four UDishonoredNativeStateMachine _Derived slots are overridden
// (+292..+304 in 2013).
public:
	virtual void PostStateChange_Derived();
	virtual void PostDestroyFSM_Derived();
	virtual UBOOL PendingStateIsCurrentState_Derived();
	virtual void ClearPendingState_Derived();

	void TickAIFSM( FLOAT DeltaSeconds );
	void RefreshAIFSM( FLOAT TimeSinceLastThought );
	UBOOL FilterAIFSM( const struct FAIStimStruct& AIStim );
	void OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention );
	void OnOwningBehaviorPause( UBOOL bIsBeingTerminated );
	void OnOwningBehaviorStop( UBOOL bIsBeingTerminated );
	UDisAISubState* GetSubState( UClass* const SubStateClass ) const;
	UDisAISubState* GetCurrentAISubState() const { return (UDisAISubState*)GetCurrentlyActiveState(); }
	INT GetActiveSubStateIndex() const { return m_CurrentAISubStateIndex; }
	INT GetPendingSubStateIndex() const { return m_PendingAISubStateIndex; }
	INT GetLogicalSubStateIndex() const;
	UBOOL RequestSafeAIStateChange( struct FDisNativeStateParam& Params, INT SubStateIndex, UDisTweaks_AISubState* SubStateTweaks );
	void SetPendingSubStateTweaks( UDisTweaks_AISubState* SubStateTweaks );
