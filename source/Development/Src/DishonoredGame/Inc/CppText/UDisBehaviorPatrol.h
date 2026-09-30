// UDisBehaviorPatrol cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent EP (PHASE14 EP). The behaviour that makes an NPC walk. It is activated by one stim,
// EAIStimID_PatrolRequest (71), which ADishonoredSpawner::OnSpawned already raises for every spawner whose
// m_bPatrolUponStartup is set - measured on L_Tower_P before this file existed: "EAIStimID_PatrolRequest(71)=8", eight
// requests raised and dropped, because this class had no evaluate mask and so no behaviour ever answered them.
//
// Four sub-state slots, from the tweaks this brain is built with:
//   0  TakeActorPosition  walk to the actor the stim named (m_pStartingActor)
//   1  TakeActorPosition  walk to the current route point
//   2  Stand              stand at a guard post
//   3  Stand              stand where you are, because there is no route to take
// GetActiveSubStateIndex() answers that slot, and it is what every callback here switches on.
// Bodies in Src/disbehaviorpatrol.cpp.
public:
	virtual void OnBehaviorStart();
	virtual void OnBehaviorStop( UBOOL _bIsBeingTerminated );
	virtual void OnBehaviorPause( UBOOL _bIsBeingTerminated );
	virtual void OnBehaviorResume();
	virtual const BYTE* BuildEvaluateStimMask();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual FDisStimSetupDelegate GetSetupFromStimDelegate( BYTE _StimID );

	virtual void OnEnterCallback_Stand( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pLastState );
	virtual void TickCallback_Stand( class UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void TickCallback_TakeActorPosition( class UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	virtual void RequestStateExitCallback_TakeActorPosition( class UDishonoredNativeState* _pThisState );

	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }

	/** UObject vtable slot 110 (2013 0x6ec9e0), pinned twice: the ctor 0x6f9160 names the table at rva 0xd28340 and
	    the 2012 table has the same body one slot earlier. UDisBehaviorPatrolSearch overrides it. */
	virtual void OnNewRouteChosen();

protected:
	void ResetPatrol();

private:
	UBOOL EvaluatePatrolRequest( const struct FAIStimStruct_PatrolRequest& _rStim ) const;
	UBOOL FilterPatrolRequest( const struct FAIStimStruct_PatrolRequest& _rStim );
	void SetupFromPatrolRequest( const struct FAIStimStruct_PatrolRequest& _rStim );

	UBOOL ChooseNewRoute();
	void FindNextPoint();
	void SetGuardPoint( class ADishonoredNavPoint* _pGuardPoint );
	void TickGuarding( FLOAT _fDeltaSeconds );
	void OnReachedDestination( class AActor* _pPoint, UBOOL _bIsGuardPoint );
	void RequestTakeActorPosition( BYTE _Slot, class AActor* _pPoint );
	void RequestStandWhereIAm();
