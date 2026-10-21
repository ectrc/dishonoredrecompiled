// UArkAvoidable cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// DISHONORED(port, agent JD): Engine.ArkAvoidable is an Engine class in retail (Engine/Src/arkavoidable.cpp, 2012 rvas
// 0x526c80..0x52f700); its methods are defined in DishonoredGame/Src/arkavoidance.cpp because this shim is the tree's only
// declaration of the class. None is virtual: the class adds no vtable slot to UObject's.
public:
	static UBOOL m_bDisableAvoidable;										// 2012 0xf42e40

	void RegisterAvoidable( AActor* const _pActorOwner );					// 2012 0x526d30
	void UnregisterAvoidable();												// 2012 0x526d80
	UBOOL IsRegistered() const { return m_bIsRegistered; }					// 2012 0x526dd0
	void SetRadius( FLOAT _fRadius ) { m_fTargetedRadius = _fRadius; }		// 2012 0x526de0
	void SetForceMultiplier( FLOAT _fForceMultiplier ) { m_fTargetedForceMultiplier = _fForceMultiplier; }	// 2012 0x526e00
	void SetForcedLocation( const FVector& _ForcedLocation ) { m_ForcedLocation = _ForcedLocation; m_bHaveForcedLocation = TRUE; }	// 2012 0x526e20
	void SetGroupFlags( INT _GroupFlags ) { m_GroupFlags = _GroupFlags; }	// 2012 0x526e40
	INT GetGroupFlags() const { return m_GroupFlags; }						// 2012 0xab2b50
	void SetPropertyFlags( INT _PropertyFlags ) { m_PropertyFlags = _PropertyFlags; }	// 2012 0x526e60
	INT GetPropertyFlags() const { return m_PropertyFlags; }				// 2012 0x526e50 (folded)
	FVector GetForce() const { return m_bDisableAvoidable ? FVector( 0.f, 0.f, 0.f ) : m_Force; }	// 2012 0x526e70
	INT GetAffectorCount() const { return m_AffectorCount; }				// 2012 0x526ea0
	const AActor* GetOwner() const { return m_pActorOwner; }				// 2012 0x526eb0
	UArkAvoidable* GetStrongerColAvoidable() const { return m_pStrongerColAvoidable; }	// 2012 folded getter of +84
	FLOAT GetStrongerColStrength() const { return m_fStrongerColStrength; }	// 2012 0x526ec0
	FLOAT GetTargetedRadius() const { return m_fTargetedRadius; }			// 2012 folded getter of +92
	FLOAT GetRadius() const { return m_fCurrentRadius; }					// 2012 folded getter of +96
	FLOAT GetTargetedForceMultiplier() const { return m_fTargetedForceMultiplier; }	// 2012 folded getter of +100
	FLOAT GetForceMultiplier() const { return m_fCurrentForceMultiplier; }	// 2012 0x526ee0
	void SetStrongerColAvoidable( UArkAvoidable* const _pStrongerColAvoidable, FLOAT _fStrongerColStrength )	// 2012 0x526ef0
	{
		m_pStrongerColAvoidable = _pStrongerColAvoidable;
		m_fStrongerColStrength = _fStrongerColStrength;
	}
	void OnOtherAvoidableUnregistered( UArkAvoidable* _pOther );			// 2012 0x529d00

	const FVector& GetLocation() const { return m_bHaveForcedLocation ? m_ForcedLocation : m_pActorOwner->Location; }	// 2012 0x526f10
	void UpdateRadius( FLOAT _fRadius ) { m_fCurrentRadius = _fRadius; }	// 2012 folded setter of +96
	void UpdateForceMultiplier( FLOAT _fForceMultiplier ) { m_fCurrentForceMultiplier = _fForceMultiplier; }	// 2012 0x526f30
	void ResetForceAndAffectorCount();										// 2012 0x526f50
	void AddForce( const FVector& _Force ) { m_Force += _Force; }			// 2012 0x526f80
	void SubForce( const FVector& _Force ) { m_Force -= _Force; }			// 2012 0x526fc0
	void IncrementAffectorCount() { ++m_AffectorCount; }					// 2012 0x527000
