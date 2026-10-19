// UStateSharedActionBase cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The action base every player and NPC action state shares: entering takes the last
// action's m_fTimeToGo (else -1) and runs DoAction; a reset runs DoAction again; RequestStateExit_WithTime leaves and keeps the
// blend time for the next state. 2012 / 2013 rvas: OnEnterState 0x6a3da0 / 0x66e050, OnResetState 0x692a20 / 0x65fa50,
// SetFiringObject 0x692a30 / 0x65fa60, RequestStateExit_WithTime 0x6959d0 / 0x662880, OnFiringObjectChanged 0x69a3c0 / 0x666ee0,
// OnLinkSyncEnd 0x848ff0. DoAction (vtable +372) and OnLinkedActionExit are pure in retail; their defaults here do nothing.
// Bodies in Src/statesharedactionbase.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual UBOOL OnResetState();
	virtual void SetFiringObject( UObject* FiringObject );
	virtual UObject* GetFiringObject() const { return (UObject*)m_pFiringObj; }
	virtual void OnFiringObjectChanged( UObject* OldFiringObject );
	virtual UBOOL DoAction() { return TRUE; }
	virtual void RequestStateExit_WithTime( FLOAT TimeToGo );
	virtual void OnLinkedActionExit() {}
	void OnLinkSyncEnd() { OnLinkedActionExit(); }
