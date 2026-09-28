// FDisDesireRequest cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. The target half of a desire, shared by the three request kinds. A desire names its
// target in one of four ways - an attention proxy, an IDisLookAtInterface, an actor, or a bare location - and this
// struct's job is to resolve whichever one is set into the (actor, location) pair the Ark components actually take, and
// then to answer ONE question: given what the target resolves to now and what the live request was issued with, does the
// component need a new request, an update, a stop, or nothing (EDisDesireRequestStatus). That answer is
// GetRequestStatus, and it is the whole reason the desire layer exists: without it every sub-state tick would re-issue
// the same movement or rotation order to the locomotion component.
// Bodies in Src/disdesirestructs.cpp.
public:
	/** DISHONORED(port): 2013 rva 0x8b3e30 (2012 0x900450). Drops the target and reports whether the component has a
	    live request that must now be stopped. */
	enum EDisDesireRequestStatus ClearTarget();

	/** DISHONORED(port): 2013 rva 0x8b3e90 (2012 0x9004b0), 881 bytes - the largest body of the desire layer and the
	    one that decides every transition. */
	enum EDisDesireRequestStatus GetRequestStatus( class AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _rLocationTarget );

	// DISHONORED(port): 2013 rvas 0x8b74e0 / 0x8b7570 / 0x8b75f0 / 0x8b7650 (2012 0x901cc0 / 0x901d50 / 0x901dd0 / 0x901e30)
	enum EDisDesireRequestStatus SetProxyTarget( const struct FDisAttentionProxy& _rProxyTarget );
	enum EDisDesireRequestStatus SetLookAtTarget( class IDisLookAtInterface* _pLookAtTarget );
	enum EDisDesireRequestStatus SetActorTarget( class AActor* _pActorTarget );
	enum EDisDesireRequestStatus SetLocationTarget( const FVector& _rLocationTarget );

	// DISHONORED(port): 2013 rva 0x8a9ad0 (2012 0x8f9ec0)
	UBOOL ReferencesActor( const class AActor* _pActor ) const;
	// DISHONORED(port): 2013 rva 0x8b3d70 (2012 0x900390): the request listens for its target's termination only while
	// it actually holds an actor reference, so a location-only desire costs no registration.
	void RegisterToActorTerminationIfNeeded();
	// DISHONORED(port): 2013 rva 0x8b2390 (2012 0x8ffb20): one handler for all three kinds; which ClearRequest to call
	// is decided by the m_bIsFaceToRequest / m_bIsLocoRequest / m_bIsLookAtRequest discriminator.
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );
	// DISHONORED(port): 2013 rva 0x8b3de0 (2012 0x900400)
	~FDisDesireRequest();
