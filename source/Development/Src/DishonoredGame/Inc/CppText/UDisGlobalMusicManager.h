// UDisGlobalMusicManager cpptext: included inside the generated class body (DishonoredGameMusicClasses.h).
// DISHONORED(port): agent IH4 (PHASE23 IH4). The game info's music manager: a native state machine of five music states
// (exploration, suspense, chase, combat, rat attack) chosen every tick from the combat intensity and the enemies near the
// player; each state posts its event of the map's UDisTweaks_MusicEvents on the player pawn. 2012 / 2013 rvas: Init_GlobalMusic
// 0x8c8310 / 0x8529c0, Term_GlobalMusic 0x8b16b0 / 0x841880, SetTweaks_Derived 0x8b16f0 / 0x8418c0, ChooseNewState 0x8c8390 / 0x852a40,
// TickCallback_Suspense 0x8c8540 / 0x852bf0 (retail vtable 2012 0xd765e0/e4/e8: the Exploration, RatAttack and Suspense slots share this folded body), TickCallback_Combat 0x8c8550 / 0x852c00,
// TickCallback_Chase 0x8c8570 / 0x852c20, Tick_GlobalMusic 0x8d22c0 / 0x85b610. Bodies in Src/disglobalmusicmanager.cpp.
public:
	void Init_GlobalMusic();
	void Term_GlobalMusic();
	void Tick_GlobalMusic( FLOAT _fDeltaSeconds );
	virtual void TickCallback_Exploration( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void TickCallback_RatAttack( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void TickCallback_Suspense( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void TickCallback_Combat( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void TickCallback_Chase( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
	class UDisTweaks_MusicEvents* GetMusicEventsTweaks();
private:
	void ChooseNewState( UDishonoredNativeState* const _pCurrentState );
