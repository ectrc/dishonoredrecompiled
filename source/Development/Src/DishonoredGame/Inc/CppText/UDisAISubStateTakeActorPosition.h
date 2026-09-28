// UDisAISubStateTakeActorPosition cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. TakePosition against a moving target: it keeps the actor and re-points the destination at
// it, stopping short by m_fMinAllowedDistanceFromActor. This is what a patrol route point and a follow order use.
// Bodies in Src/disaisubstatetakeactorposition.cpp.
public:
	virtual void RefreshSubState( const FLOAT TimeSinceLastThought );
	virtual void BeginDestroy();

	// DISHONORED(port): 2013 rva 0x741d80 (2012 0x77bf00): re-pointing at another actor moves the termination subscription.
	void SetDestinationActor( class AActor* _pDestinationActor );
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );
