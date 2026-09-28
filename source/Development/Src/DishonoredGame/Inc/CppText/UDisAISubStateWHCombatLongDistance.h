// UDisAISubStateWHCombatLongDistance cpptext: included inside the generated class body (DishonoredGameAIWolfhoundClasses.h).
// DISHONORED(written): agent DF. A wolfhound circling at range, repositioning every few seconds to keep line of sight.
// Bodies in Src/disaisubstatewhcombatlongdistance.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void EndSubState_Derived( UBOOL bIsBeingTerminated );
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }

	void SetHasSeenEnemyRecently( UBOOL _bHasSeenEnemyRecently );

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
	void ResetRepositionRequest();
