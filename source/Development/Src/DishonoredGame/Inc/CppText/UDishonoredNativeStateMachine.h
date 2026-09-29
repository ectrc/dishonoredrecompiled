// UDishonoredNativeStateMachine cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable after the UObject slots (2012 PDB +288..+300, 2013 +292..+304): PostStateChange_Derived,
// PostDestroyFSM_Derived, PendingStateIsCurrentState_Derived, ClearPendingState_Derived (empty / TRUE in the base). The
// transition machinery lives in dishonorednativestatemachine.cpp with its 2013 rvas (InitFSM 0x67ba50, RequestStateChange
// 0x674fa0, DemandStateChange 0x672190, DoStateChange 0x65f8c0, TickStateMachine 0x66de60, DestroyFSM 2012 0x6a89e0).
public:
	virtual void PostStateChange_Derived() {}
	virtual void PostDestroyFSM_Derived() {}
	virtual UBOOL PendingStateIsCurrentState_Derived() { return TRUE; }
	virtual void ClearPendingState_Derived() {}

	void InitFSM( UObject* const ManagedObject, FDisNativeStateParam& DefaultStateParams, const TArray<UDishonoredNativeState*>* FSMStates );
	void DestroyFSM();
	void BuildNativeStateMap();
	UBOOL RequestStateChange( FDisNativeStateParam& Param, UObject* const FiringObject, UBOOL bTestOnly );
	void DoStateChange();
	void TickStateMachine( FLOAT DeltaSeconds );
	UBOOL CanTransitionTo( UClass* StateID, UDishonoredNativeState** OutRejectingState ) const;
	UBOOL IsCurState( const UClass* StateID, UBOOL bExactClass ) const;
	void LockFSM( UBOOL bLock );
	void OnPawnShutDown( const ADishonoredPawn& Pawn );
	void GetAllStateIDs( TArray<UClass*>& OutStateIDs ) const;
	UDishonoredNativeState* GetCurrentlyActiveState() const;
	UDishonoredNativeState* GetPendingState() const;
	UDishonoredNativeState* GetLogicalState() const;
	const UClass* GetPendingStateID() const;
	UDishonoredNativeState* FindState( UClass* StateID ) const;
	UObject* GetManagedObject() const { return m_pManagedObject; }
	/** DISHONORED(port): agent EC (PHASE11 EC), 2013 rva 0x672670 (2012 0x6a3be0); body in dissavegame.cpp.
	    GameSave / GameLoad (slots 68 and 69) and SavePartialState are not ported. */
	void LoadPartialState( FArchive& _rArchive, ESaveLoadLocation _Location );
private:
	void DemandStateChange( UDishonoredNativeState* DemandingState, FDisNativeStateParam& Param );
	void ClearPendingState();
	void DebugStoreRejectedStateInfo( const FDisNativeFSMRejectedInfo& Info );
	friend class UDishonoredNativeState;
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad, 2013 rva 0x67a9e0, retail vtable slot 70. Slot 69 is the
// UDisAttentionInfo_Base::GameSave fold and is not ported. Body in Src/dishonorednativestatemachine.cpp. Not
// to be confused with LoadPartialState above, which is a different slot and a different body.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
