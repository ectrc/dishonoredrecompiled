// UDisBehaviorGoTo_Base cpptext: included inside the generated class body (dishonoredgameclasses.h).
// agent FV (PHASE18 FV): the behaviour that walks an NPC to one destination and, when asked, turns it to face the
// destination's rotation; UDisBehaviorGoTo (the Kismet go-to) and the follow-ups derive from it. Bodies in
// Src/disbehaviorgoto_base.cpp. 2013 rvas (2012 in brackets): OnBehaviorStart 0x6e4d50 (0x745550), OnBehaviorStop
// 0x6e4d60 (0x745560), PrepareGoTo 0x6e4de0 (0x7455c0), FilterDestinationReached 0x6e4ea0 (0x745690), FilterRotationReached
// 0x6e4ed0 (0x7456c0), SetFixedDestination 0x6e4ef0 (0x7456e0), GetDesiresLocoRequest 0x6e4f60 (0x745760),
// IsPlayerAllowedToPushMe 0x6e4f70 (0x745770), Start_TurningToFace 0x6e8590 (0x746d40), FilterPathingFail 0x6e8600
// (0x746db0), TickBehavior 0x6edc00 (0x74c8c0), BuildFilterStimMask (0x745650), BuildBehaviorFilterStimMasks (0x74c900),
// GetFilterStimDelegate (0x74c990), OnBehaviorPause (0x746ce0), OnBehaviorResume (0x746d10).
// IsBehaviorFinished is not a function of this class in either database: UObject vtable slot 92 of
// ??_7UDisBehaviorGoTo_Base@@6BUObject@@@ (2012 0xd25e60) holds UDisBehaviorShoot::IsBehaviorFinished (0x724a60,
// `return *((DWORD*)this + 41) & 1`), ICF-folded because both classes keep their finished bit at offset 164 bit 0.
// GetDesiresFaceToRequest is slot 108 (0x745750, `lea eax,[this+0CCh]` = m_FaceToRequest @204).
public:
	virtual void OnBehaviorStart();
	virtual void OnBehaviorStop( UBOOL _bIsBeingTerminated );
	virtual void OnBehaviorPause( UBOOL _bIsBeingTerminated );
	virtual void OnBehaviorResume();
	virtual void TickBehavior( FLOAT _fDeltaSeconds );
	virtual UBOOL IsBehaviorFinished() const { return m_bIsFinished; }
	virtual UBOOL IsPlayerAllowedToPushMe() const;
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate( BYTE _StimID );
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return &m_LocoRequest; }

protected:
	void PrepareGoTo();
	void SetFixedDestination( const FVector& _rDestinationLocation, const FRotator& _rDestinationRotation, UBOOL _bAccurateStop, UBOOL _bExactRotation );
	void Start_TurningToFace();
	UBOOL FilterDestinationReached( const struct FAIStimStruct_DestinationReached& _rStim );
	UBOOL FilterRotationReached( const struct FAIStimStruct_RotationReached& _rStim );
	UBOOL FilterPathingFail( const struct FAIStimStruct_PathingFail& _rStim );
