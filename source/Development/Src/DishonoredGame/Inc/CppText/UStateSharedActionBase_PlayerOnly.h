// UStateSharedActionBase_PlayerOnly cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The player's action base: the current FPawnAction (m_pCurAction), its anim state chosen
// at random without repeat, the exit and interrupt rules. 2012 / 2013 rvas: DoAction 0x692a50 / 0x65fa80, OnResetState 0x692a60 /
// 0x65fa90, OnExitState 0x692a70 / 0x65faa0, CanTransitionAction 0x692a80 / 0x65fab0, ChooseAnimState 0x69a430 / 0x666f70,
// CanTransitionExternal 0x6a3d10 / 0x66dfc0, RequestStateExit_Derived 0x6a5d20 / 0x675290, the param's OnPending 0x695a40.
// Bodies in Src/statesharedactionbase_playeronly.cpp.
public:
	virtual UBOOL DoAction();
	virtual UBOOL OnResetState();
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual UBOOL CanTransitionAction( const struct FPawnAction* Action ) const;
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
	const struct FPawnAction* GetCurAction() const { return (const struct FPawnAction*)m_pCurAction; }
protected:
	virtual void RequestStateExit_Derived();
	virtual FName ChooseAnimState();
public:
