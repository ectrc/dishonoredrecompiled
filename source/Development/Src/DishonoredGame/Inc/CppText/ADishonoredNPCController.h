// ADishonoredNPCController cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. The controller is what owns an NPC's brain: PostBeginPlay gives it its vision
// component, InitNPC constructs the UDishonoredAIBrain from the pawn's UDisTweaks_AIBrain and calls InitBrain, and Tick
// ticks the brain every frame until the pawn dies. Bodies in Src/dishonorednpccontroller.cpp.
public:
	void InitNPC( class UDisTweaks_AIBrain* const _pAIBrainTweak, BYTE _SuspicionLevel );
	class UDishonoredAIBrain* GetAIBrain() const { return m_pAIBrain; }

	virtual void PostBeginPlay();
	virtual UBOOL Tick( FLOAT _fDeltaTime, enum ELevelTick _TickType );
	virtual void UnPossess();
	virtual void ClearComponents();
	virtual UBOOL IsDead() const;
	virtual void OnOtherActorTerminated( const class AActor& _rActor );
	virtual FVector GetMoveTargetLocation() const;

	void SetGoToActionStatus( BYTE _Status, class UDisSeqAct_AIGoToActor* _pExpectedAction );
	void SetShootActionStatus( BYTE _Status );

	virtual void OnAISetBrainFlags( class UDisSeqAct_AISetBrainFlags* _pAction );
	virtual void OnAIGetBrainFlags( class UDisSeqAct_AIGetBrainFlagValue* _pAction );
	virtual void OnAISetSuspicionLevel( class UDisSeqAct_AISetSuspicionLevel* _pAction );
	virtual void OnAIProtectNeutralsOverride( class UDisSeqAct_AIProtectNeutralsOverride* _pAction );

	static FName s_DisAIBrainDefaultName;
	// ---- agent CG natives sweep ----
	virtual void OnAISetSenses( class UDisSeqAct_AISetSenses* _pAction );
	virtual void OnAIAmbush( class UDisSeqAct_AIAmbush* _pAction );
	virtual void OnAIDoSearch( class UDisSeqAct_AIDoSearch* _pAction );
	virtual void OnAISetPatrol( class UDisSeqAct_AISetPatrol* _pAction );
	virtual void OnAIRingAlarm( class UDisSeqAct_AIRingAlarm* _pAction );
	virtual void OnAIGuard( class UDisSeqAct_AIGuard* _pAction );
