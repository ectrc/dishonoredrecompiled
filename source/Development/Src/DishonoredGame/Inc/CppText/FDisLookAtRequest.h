// FDisLookAtRequest cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Look there" / "aim there", with a head-and-torso influence pair, an optional procedural
// pattern and a duration. Unlike the other two it ticks: a look-at with a positive duration expires on its own
// (TickRequest / SyncRequest), which is how a glance ends without anyone cancelling it. Bodies in Src/disdesirestructs.cpp.
// DISHONORED(bringup): FArkComponentLookat is not ported, so ADishonoredNPCPawn::GetComponentLookat answers NULL, no
// request id is issued and DoRequest stops at the component boundary with a counted note.
// DISHONORED(retail): the default constructor (2013 rva 0x6cd760, 2012 0x72c8b0) sets m_LookAtInfluence to (1,1,TRUE).
// It cannot be declared here because the generator emits `FDisLookAtRequest() {}` above this include; nothing observes
// the difference, because SetParams overwrites the influence from its argument on every request and ClearRequest resets
// it to FDisLookAtInfluence::Torso, which is the same (1,1,TRUE).
public:
	// DISHONORED(port): 2013 rva 0x8b2400 (2012 0x8ffb90)
	void Initialize( class FArkComponentLookat* _pLookAtComponent, class UObject* _pAsker, BYTE _Priority );
	// DISHONORED(port): 2013 rva 0x8ae2c0 (2012 0x8fce80)
	void Finalize();
	// DISHONORED(port): 2013 rvas 0x8b7cd0 / 0x8b7d20 / 0x8b7da0 / 0x8b7e20 (2012 0x9024b0 / 0x902500 / 0x902580 / 0x902600)
	void RequestProxyTarget( const struct FDisAttentionProxy& _rProxyTarget, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void RequestActorTarget( class AActor* _pActorTarget, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void RequestActorAimTarget( class AActor* _pActorTarget, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void RequestProceduralLookAt( BYTE _ProceduralPatternIndex, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	// DISHONORED(port): 2013 rva 0x8ba9d0 (2012 0x905b10)
	void TickRequest( FLOAT _fDeltaTime );
	// DISHONORED(port): 2013 rva 0x8aa040 (2012 0x8fa430)
	void OnActorTerminated( const class AActor& _rActor );
	// DISHONORED(port): 2013 rva 0x8baa40 (2012 0x905b80)
	void PostGameLoad( class FArkComponentLookat* _pLookAtComponent, class UObject* _pAsker );
	// DISHONORED(port): 2013 rva 0x8b42d0 (2012 0x9008d0): drops a request the component has finished or let expire.
	void SyncRequest();
	// DISHONORED(port): 2013 rva 0x8a9fe0 (2012 0x8fa3d0)
	void ClearRequest();
	// DISHONORED(port): 2013 rva 0x8aa060 (2012 0x8fa450), 1,068 bytes: six component entry points, chosen by
	// aim-or-look, actor-or-location-or-nothing and procedural-or-not.
	void DoRequest( enum EDisDesireRequestStatus _Status );

private:
	/** DISHONORED(retail): SetParams is private in retail's PDB and yet the four IDisDesiresInterface setters that have no
	    Request* form of their own (SetLookAtLocationDesire, SetLookAtProceduralProxyDesire,
	    SetLookAtProceduralLocationDesire, SetAimAtProxyDesire, 2013 rvas 0x8ba7f0 / 0x8ba910 / 0x8ba970 / 0x8ba850) call
	    it directly, so retail declares the interface a friend. */
	friend class IDisDesiresInterface;

	/** DISHONORED(port): the target half shared by RequestActorTarget and RequestActorAimTarget, which retail spells out
	    in both (2013 rvas 0x8b7d20 / 0x8b7da0). */
	enum EDisDesireRequestStatus SetActorOrLookAtTarget( class AActor* _pActorTarget );
	// DISHONORED(port): 2013 rva 0x8a7fa0 (2012 0x8f8fc0)
	void SetParams( UBOOL _bLocal, UBOOL _bAimAtTarget, BYTE _ProceduralPatternIndex, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration, enum EDisDesireRequestStatus& _rStatus );
	void ClearParams();
