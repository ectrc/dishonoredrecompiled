// UDisAISubStateFindShootingPosition cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. Looking for somewhere to shoot from: it collects candidate points on the nav mesh, scores
// each by whether the target can actually be hit from it, and walks to the best.
// Bodies in Src/disaisubstatefindshootingposition.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubState( BYTE StimID );
	virtual UBOOL GetPathGoals( const FVector& Destination, TArray<class UNavMeshPathGoalEvaluator*>& OutGoals );
	virtual UBOOL GetPathConstraints( const FVector& Destination, UBOOL bIsFleeing, TArray<class UNavMeshPathConstraint*>& OutConstraints );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }

	// DISHONORED(port): 2012 rvas 0x788b60 / 0x788cb0: the shoot-range band the slot's tweaks and the pawn's weapon define,
	// and the test the behaviour callbacks ask.
	void CalculateShootDistances();
	UBOOL IsWithinLegalShootRange( const FVector& _rPosition ) const;

private:
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
	void JustStartWalkingSomewhere();
