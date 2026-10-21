// UStatePlayerMasterInDialog cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ): the soiree hand-over the player MAT_* contract reaches when the master FSM is already
// in dialog: StartSoireeControl 2012 0x6afa80 (2013 0x6696e0), StopSoireeControl 0x6afab0 (2013 0x663e20), SetSoireeBlendOutTime
// 0x6afae0 (2013 0x6609c0, inline here). agent JE (PHASE24 JE): the dialog state itself, 2012 / 2013 rvas: OnEnterState 0x6b8b20 /
// 0xa73720 - 0x400000, OnExitState 0x6b8d30 / 0x6739f0, TickState 0x6c94c0 / 0x679280, RequestStateExit_Derived 0x6b8ea0,
// TrackPawnForDialog 0x6b8ec0 / 0x669480, TrackActorForDialog 0x6b8ff0 / 0x6695a0, StopTrackingForDialog 0x6b4160 / 0x663d90,
// CalcZoomFOVBasedOnRadius 0x6b40d0 / 0x663cc0, PlayerRequestSkip_Derived 0x6afa30 / 0x660970, IsSkippable_Derived 0x6afa50 /
// 0x660990. agent JF (PHASE24 JF), the choice half: SetActivePlayerChoice 0x6b8e30 / 0x669410, GetNumChoices 0x6af980 / 0x6608c0,
// GetChoiceInfo 0x6b4190 / 0x663dc0, IsDialogViewTrackBlending 2013 0x663d50 (unnamed, new in 2013). Bodies in
// Src/stateplayermasterindialog.cpp.
public:
	void StartSoireeControl( class USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags );
	void StopSoireeControl( UBOOL bInterrupted );
	void SetSoireeBlendOutTime( FLOAT BlendOutTime ) { m_fSoireeBlendOutTime = BlendOutTime; }
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	void TrackPawnForDialog( class ADishonoredPawn* _pPawn, FLOAT _fBlendTime, BYTE _FocusHitRegion, const struct FDisConvLookAtConstraints* _pConstraints );
	void TrackActorForDialog( class AActor* _pActor, FLOAT _fBlendTime, const struct FDisConvLookAtConstraints* _pConstraints );
	void StopTrackingForDialog();
	void SetActivePlayerChoice( const class UDisConv_PlayerChoice* _pPlayerChoice );
	UBOOL IsDialogViewTrackBlending() const;
protected:
	virtual INT GetNumChoices();
	virtual void GetChoiceInfo( INT _iIndex, struct FDisPlayerChoiceInfo& _rChoiceInfo );
	virtual void RequestStateExit_Derived();
	virtual void PlayerRequestSkip_Derived();
	virtual UBOOL IsSkippable_Derived() const;
	FLOAT CalcZoomFOVBasedOnRadius( const FVector& _rTargetPos, FLOAT _fTargetRadius ) const;
public:
