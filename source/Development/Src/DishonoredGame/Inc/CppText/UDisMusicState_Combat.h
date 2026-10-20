// UDisMusicState_Combat cpptext: included inside the generated class body (DishonoredGameMusicClasses.h).
// DISHONORED(port): agent IH4. 2012 rvas: ChooseCombatIntensity 0x8b8db0, OnEnterState 0x8bc830, TickState 0x8bc850.
// Bodies in Src/dismusicstate_combat.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void TickState( FLOAT DeltaSeconds );
private:
	void ChooseCombatIntensity();
