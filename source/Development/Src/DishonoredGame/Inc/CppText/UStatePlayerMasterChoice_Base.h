// UStatePlayerMasterChoice_Base cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent JE (PHASE24 JE), the enter / exit / bend-time half of the dialog and scripted-choice states' base.
// 2012 / 2013 rvas (stateplayermasterchoice_base.cpp): OnEnterState 0x6b8560 / 0x673640, OnExitState 0x6b8600 / 0x66c7e0,
// PauseBendTime 0x6b8680 / 0x6690a0, UnpauseBendTime 0x6b8750 / 0x669170, PlayerRequestSkip 0x6af8f0 / 0x660810, IsSkippable
// 0x6af910 / 0x660830, TickState 0x6c9470 / 0x679230. agent JF (PHASE24 JF), the choice half: ShowChoiceSelection 0x6c7450 /
// 0x676a10, MakePlayerChoice 0x6b8820 / 0x669240, ReadyToMakeChoice 0x6af930 / 0x660850; the derived state's choice virtuals
// (2013 vtable +388 GetNumChoices, +392 GetChoiceInfo, +396 GetTitleText, +400 MakePlayerChoice_Derived). Bodies in
// Src/stateplayermasterchoice_base.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	void PlayerRequestSkip();
	UBOOL IsSkippable() const;
	UBOOL MakePlayerChoice( INT _iChoice );
protected:
	virtual INT GetNumChoices() { return 0; }
	virtual void GetChoiceInfo( INT _iIndex, struct FDisPlayerChoiceInfo& _rChoiceInfo ) {}
	virtual UBOOL GetTitleText( FString& _rTitle ) { return FALSE; }
	virtual void MakePlayerChoice_Derived() {}
	virtual void PlayerRequestSkip_Derived() {}
	virtual UBOOL IsSkippable_Derived() const { return FALSE; }
	void PauseBendTime();
	void UnpauseBendTime();
	void HideChoiceSelection();
	void ShowChoiceSelection();
	UBOOL ReadyToMakeChoice() const;
public:
