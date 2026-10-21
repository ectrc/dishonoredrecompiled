// UStatePlayerMasterChoice_Base cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent JE (PHASE24 JE), the enter / exit / bend-time half of the dialog and scripted-choice states' base.
// 2012 / 2013 rvas (stateplayermasterchoice_base.cpp): OnEnterState 0x6b8560 / 0x673640, OnExitState 0x6b8600 / 0x66c7e0,
// PauseBendTime 0x6b8680 / 0x6690a0, UnpauseBendTime 0x6b8750 / 0x669170, PlayerRequestSkip 0x6af8f0 / 0x660810, IsSkippable
// 0x6af910 / 0x660830, TickState 0x6c9470 / 0x679230. The choice half (ShowChoiceSelection, MakePlayerChoice, ReadyToMakeChoice,
// GetNumChoices / GetChoiceInfo) is agent JF's. Bodies in Src/stateplayermasterchoice_base.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	void PlayerRequestSkip();
	UBOOL IsSkippable() const;
protected:
	virtual void PlayerRequestSkip_Derived() {}
	virtual UBOOL IsSkippable_Derived() const { return FALSE; }
	void PauseBendTime();
	void UnpauseBendTime();
	void HideChoiceSelection();
public:
