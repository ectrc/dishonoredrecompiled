// IDisDesiresInterface cpptext: included inside the generated interface body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. How an AI thinks out loud. A behaviour, a sub-state or a sub-process does not drive the
// pawn; it states four desires - "walk there" (loco), "turn to face that" (face-to), "look at that" (look-at) and "hold
// this stance with these items" (body intention) - and this interface owns the bookkeeping that turns a stream of
// identical desires into at most one order per change. The state itself lives on the implementing class, reached through
// the four request accessors below, which is why the same interface serves a behaviour, a sub-state and a sub-process
// with different priorities: EDisFaceToPriority / EDisLocoPriority / EDisLookAtPriority / EDisBodyIntentionPriority.
//
// It is the second-largest of the three blockers agent CG measured (~23 of the remaining natives) and it is what
// `DisAINoteDesiresGap` marked in the brain, the behaviour and the sub-process.
//
// DISHONORED(layout): the interface has no data of its own (retail sizeof 56 = UInterface, and the C++ sub-object is a
// bare vtable pointer). 2012 PDB IDisDesiresInterface_vtbl, 14 slots, in this declaration order: +0 destructor,
// +4 GetUObjectInterfaceDisDesiresInterface, +8 GetDesiresOwningPawn (both purecall in the base), +12..+40 the four
// request accessors and the four priorities (all eight identical-code-folded onto a `return 0`), +44/+48 the two event
// callbacks, +52 GetResumingBodyIntentionDesire (folded onto a `return FALSE`, 2012 rva 0x723660).
// DISHONORED(retail): GetDesiresOwningPawn returns `ADishonoredNPCPawn&` in retail and is purecall in the base. It is a
// pointer here and answers NULL in the base, because a behaviour whose brain has no pawn yet would otherwise form a
// reference from NULL; every call site guards, which is the same control flow.
// Bodies in Src/disdesiresinterface.cpp.
public:
	virtual class UObject* GetUObjectInterfaceDisDesiresInterface() { return NULL; }
	virtual class ADishonoredNPCPawn* GetDesiresOwningPawn() { return NULL; }

	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return NULL; }
	virtual struct FDisLocoRequest* GetDesiresLocoRequest() { return NULL; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return NULL; }
	virtual struct FDisBodyIntentionRequest* GetDesiresBodyIntentionRequest() { return NULL; }

	virtual BYTE GetDesiresFaceToPriority() const { return 0; }
	virtual BYTE GetDesiresLocoPriority() const { return 0; }
	virtual BYTE GetDesiresLookAtPriority() const { return 0; }
	virtual BYTE GetDesiresBodyIntentionPriority() const { return 0; }

	/** DISHONORED(bringup): retail hands the Ark component a pointer-to-member of the asker so the component can report back
	    (EArkCpntFaceToEvent / EArkCpntLocoEvent). FArkComponentFaceTo and FArkComponentLocomotion are not ported, so
	    nothing calls back; the accessors stay in the vtable (slots +44 / +48) and answer NULL. */
	virtual void* GetDesiresFaceToEventCallback() const { return NULL; }
	virtual void* GetDesiresLocoEventCallback() const { return NULL; }

	/**
	 * DISHONORED(port): 2012 vtable +52. Asked on resume: "given the body intention you had before you were paused, what
	 * should it be now?" FALSE means "I have no opinion", and ResumeDesires then leaves the pawn's stance alone.
	 * DISHONORED(retail): the base answers FALSE, not TRUE. The body is identical-code-folded at 2012 rva 0x723660 (IDA
	 * names it FFileManagerError::MakeDirectory), and that function is `return 0`. Reading it as TRUE would make every
	 * resuming sub-state re-request a zeroed FDisBodyIntention and blank the NPC's stance and both its hands each time a
	 * behaviour came back to slot 0; only the overrides that mean it (UDisAISubStateStand::GetResumingBodyIntentionDesire,
	 * 2013 rva 0x7127c0, is the first) answer TRUE.
	 */
	virtual UBOOL GetResumingBodyIntentionDesire( const struct FDisBodyIntention& _rPreviousBodyIntention, struct FDisBodyIntention& _rResumingBodyIntention ) const { return FALSE; }

	// DISHONORED(port): the life cycle. 2013 rvas 0x8ba1f0 InitializeDesires, 0x8b22d0 StopDesires, 0x8ba360 PauseDesires,
	// 0x8bbf50 ResumeDesires, 0x8bc0e0 PostGameLoad_Desires, 0x8bc080 TickDesires, 0x8b3cb0 FinalizeDesires
	// (2012 0x905330 / 0x8ffa30 / 0x9054a0 / 0x907ee0 / 0x908070 / 0x908010 / 0x9002d0).
	void InitializeDesires();
	void StopDesires();
	void PauseDesires();
	void ResumeDesires( const struct FDisBodyIntention& _rPreviousBodyIntention );
	void PostGameLoad_Desires();
	void TickDesires( FLOAT _fDeltaTime );
	void FinalizeDesires();

	// DISHONORED(port): the face-to desires. 2013 rvas 0x8ba3f0 / 0x8ba420 / 0x8ba450 / 0x8ba480 / 0x8b2350
	void SetFaceToProxyDesire( const struct FDisAttentionProxy& _rProxy, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void SetFaceToActorDesire( class AActor* _pActor, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void SetFaceToLocationDesire( const FVector& _rLocation, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void SetFaceToYawDesire( INT _Yaw, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void ClearFaceToDesire();

	// DISHONORED(port): the loco desires. 2013 rvas 0x8ba4b0 / 0x8ba540 / 0x8ba670 / 0x8ba5d0 / 0x8b2370, plus the two
	// readers the AI uses to ask "where am I going and how fast" (0x8adec0 / 0x8adf60). The ETransitSpeed the caller
	// passes is turned into the pawn's own locomotion speed index through UDisTweaks_NPCPawn::m_TransitSpeedToLocomotionSpeed.
	void SetLocoProxyDesire( const struct FDisAttentionProxy& _rProxy, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	void SetLocoActorDesire( class AActor* _pActor, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	void SetLocoLocationDesire( const FVector& _rLocation, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	void SetLocoFollowDesire( class AActor* _pActor, BYTE _TransitSpeed, FLOAT _fFollowAngle, FLOAT _fFollowDist );
	void ClearLocoDesire();
	FVector GetLocoDesireDestination();
	BYTE GetLocoDesireTransitSpeed();

	// DISHONORED(port): the look-at and aim-at desires. 2013 rvas 0x8ba700 / 0x8ba760 / 0x8ba7c0 / 0x8ba7f0 / 0x8ba8e0 /
	// 0x8ba910 / 0x8ba970 / 0x8ba850 / 0x8ba8b0 / 0x8ae020
	void SetLookAtProxyDesire( const struct FDisAttentionProxy& _rProxy, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtTargetDesire( class IDisLookAtInterface* _pTarget, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtActorDesire( class AActor* _pActor, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtLocationDesire( const FVector& _rLocation, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtProceduralDesire( BYTE _ProceduralPatternIndex, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtProceduralProxyDesire( const struct FDisAttentionProxy& _rProxy, BYTE _ProceduralPatternIndex, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetLookAtProceduralLocationDesire( const FVector& _rLocation, BYTE _ProceduralPatternIndex, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetAimAtProxyDesire( const struct FDisAttentionProxy& _rProxy, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void SetAimAtActorDesire( class AActor* _pActor, const struct FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration );
	void ClearLookAtDesire();

	// DISHONORED(port): the body intention. 2013 rvas 0x8a9a70 / 0x8ae040
	UBOOL SetBodyIntentionDesire( BYTE _BodyStance, class UClass* _pPrimaryItemClass, class UClass* _pSecondaryItemClass );
	void ClearBodyIntentionDesire();

	// DISHONORED(port): what the Ark components report back. 2013 rvas 0x8b7230 HandleFaceToEvent, 0x8b7330 HandleLocoEvent
	// (2012 0x901a10 / 0x901b10): the component's answer is turned into an AI stim, which is how "I have arrived" and
	// "I cannot get there" reach the behaviour stack.
	void HandleFaceToEvent( BYTE _FaceToEvent );
	void HandleLocoEvent( BYTE _LocoEvent );
