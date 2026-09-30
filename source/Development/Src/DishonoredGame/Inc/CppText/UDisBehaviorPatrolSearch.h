// UDisBehaviorPatrolSearch cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(written): agent EP (PHASE14 EP). This class exists here for one reason, and it is a defect this package
// caused and then measured: UDisBehaviorPatrolSearch DERIVES from UDisBehaviorPatrol, so the moment UDisBehaviorPatrol
// answered EAIStimID_PatrolRequest (71) this subclass answered it too, and because it comes first in the brain's
// m_BehaviorArray it took the slot for all eight of L_Tower_P's patrolling NPCs. Retail's overrides answer
// EAIStimID_PatrolSearchRequest (72) and nothing else - 2013 rvas 0x6ef220, 0x6fd5e0, 0x6e9ae0, each a single
// `if( id == 72 )`. Measured before: "disai slot0: DisBehaviorIdle=26 DisBehaviorPatrolSearch=8"; after: Patrol takes it.
//
// Not ported, each because its dependency is a skeleton unit: InitBehavior (0x6eca30,
// UDisAISubProcessPersonalSpace::SetAmbientActionManagement), OnBehaviorResume (0x6f3c00,
// DisBreakOutOfCasualConversations and UDisAISubProcessGenericBark), OnPostGameLoad (0x6f3c80, see UDisBehaviorPatrol).
// Bodies in Src/disbehaviorpatrolsearch.cpp.
public:
	virtual const BYTE* BuildEvaluateStimMask();
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetEvaluateStimDelegate( BYTE _StimID );
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual FDisStimSetupDelegate GetSetupFromStimDelegate( BYTE _StimID );

private:
	UBOOL EvaluatePatrolSearchRequest( const struct FAIStimStruct_PatrolSearchRequest& _rStim ) const;
	UBOOL FilterPatrolSearchRequest( const struct FAIStimStruct_PatrolSearchRequest& _rStim );
	void SetupFromPatrolSearchRequest( const struct FAIStimStruct_PatrolSearchRequest& _rStim );
