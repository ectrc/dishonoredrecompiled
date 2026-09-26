// UDishonoredNativeState cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable after the UObject slots (2012 PDB UDishonoredNativeState_vtbl +288..+364, 2013 +292..+368):
// OnEnterState, OnExitState, OnResetState, OnCancelState, TickState, CanTransitionExternal, OnRejectStateRequest,
// PerformCustomPhysics, AllowsCrouchAction, SetFiringObject, OnFiringObjectChanged, GetFiringObject, OnAnimNotify, OnPawnShutDown,
// OnPreCommitMapChange, SavePartialState, LoadPartialState, PostLoadPartialState, OnOtherActorTerminated, RequestStateExit_Derived.
// The base bodies are empty in retail (the state machine calls them unconditionally). Non-virtual: RequestStateExit 2013 rva
// 0x662750, DemandStateChange 0x674eb0 (dishonorednativestate.cpp). The DisSaveLoad partial-state slots take no archive here.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState ) {}
	virtual void OnExitState( UDishonoredNativeState* NextState ) {}
	virtual UBOOL OnResetState() { return FALSE; }
	virtual void OnCancelState() {}
	virtual void TickState( FLOAT DeltaSeconds ) {}
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const { return TRUE; }
	virtual void OnRejectStateRequest( const UClass* StateID ) {}
	virtual void PerformCustomPhysics( FLOAT DeltaSeconds, INT Iterations ) {}
	virtual UBOOL AllowsCrouchAction() const { return TRUE; }
	virtual void SetFiringObject( UObject* FiringObject ) {}
	virtual void OnFiringObjectChanged( UObject* OldFiringObject ) {}
	virtual UObject* GetFiringObject() const { return NULL; }
	virtual void OnAnimNotify( const struct FAnimPlayerNotificationParams* Params ) {}
	virtual void OnPawnShutDown( const ADishonoredPawn& Pawn ) {}
	virtual void OnPreCommitMapChange() {}
	virtual void SavePartialState( UDishonoredNativeStateMachine* StateMachine, FArchive& Ar ) {}
	virtual void LoadPartialState( UDishonoredNativeStateMachine* StateMachine, UObject* ManagedObject, FArchive& Ar ) {}
	virtual void PostLoadPartialState( UDishonoredNativeStateMachine* StateMachine, UObject* ManagedObject ) {}
	virtual void OnOtherActorTerminated( const AActor& Actor ) {}
protected:
	virtual void RequestStateExit_Derived();
public:
	UDishonoredNativeStateMachine* GetStateMachine() const { return m_pStateMachine; }
	void RequestStateExit();
protected:
	void DemandStateChange( struct FDisNativeStateParam& Param );
