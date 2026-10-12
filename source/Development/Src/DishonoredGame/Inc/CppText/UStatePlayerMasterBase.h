// UStatePlayerMasterBase cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). Retail 2012 rvas (stateplayermasterbase.cpp): OnEnterState 0x6a72c0 (2013
// 0x6729d0), OnExitState 0x692d20 (2013 0x65fe00), GetBlendInTime 0x692d60, UpdateWalkNavAnimState 0x69ab60 (2013
// 0x675450), the five const getters 0x692d30..0x692da0. Bodies in Src/stateplayermasterbase.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	UBOOL CanBeInLinkedAction() const { return m_bCanBeInLinkedAction; }
	UBOOL CanApplyWalkFlags() const { return m_bCanApplyWalkFlags; }
	UBOOL ShouldIgnorePitch() const { return m_bIgnorePitch; }
	UBOOL AllowAutoFootfalls() const { return !m_bSupressAutoFootfalls; }
	UBOOL AreIncomingAttacksAllowed() const { return m_bAllowIncomingAttacks; }
protected:
	FLOAT GetBlendInTime() const;
	void UpdateWalkNavAnimState( class ADishonoredPlayerPawn* Pawn, UBOOL bHandleCrouch );
public:
