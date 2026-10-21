// UStatePlayerMasterAction cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The master FSM's full-body action: the input it takes away, the full-body anim state,
// the camera (anim driven when the action cannot look), the arms' follow. 2012 / 2013 rvas: TickState 0x692ba0, EnableCamInfluences
// 0x692bd0 (2013 0x65fc90), OnOtherActorTerminated 0x692ce0, DoAction 0x695c40 / 0x662ae0, OnExitState 0x695ec0 / 0x66e0a0,
// AnimState_AnimNotFound_Derived 0x695f90 / 0x662d70, OnLinkedActionExit 0x695fd0 / 0x662db0, CanTransitionExternal 0x6a7160 /
// 0x6727a0. Bodies in Src/stateplayermasteraction.cpp.
public:
	virtual void TickState( FLOAT DeltaSeconds );
	virtual UBOOL DoAction();
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void OnOtherActorTerminated( const AActor& Actor );
	virtual void OnLinkedActionExit();
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
	virtual UBOOL AnimState_AnimNotFound_Derived( const FString& SeqName );
	UBOOL ShouldIgnorePitch() const { return m_bIgnorePitch; }
	UBOOL AreIncomingAttacksAllowed() const { return m_bAllowIncomingAttacks; }
private:
	void EnableCamInfluences( UBOOL bEnabled ) const;
public:
// ---- agent JH (PHASE24 JH): the action's interaction fill, vtable +416 (2013 rva 0x233610, `xor eax, eax; retn 4`, as the base state).
public:
	virtual UBOOL FillUIInteractions( struct FDisUIInteractionContext& _rContext ) const { return FALSE; }
