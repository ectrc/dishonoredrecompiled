// UArkAnimNodeLocomotion cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// Engine.ArkAnimNodeLocomotion is the NPC anim tree's walk blender ("LocoBlender", anim group "LocoGroup"): nine
// UAnimNodeSequence children, 0..7 for the movement animations and 8 for one "specific" animation (turn, start, stop or
// modifier transition). FArkComponentLocomotion feeds it a speed, a modifier (walk / run / ... set) and a speed
// multiplier, FArkComponentFaceTo feeds it the strafe angle, and TickAnim picks the two speed modes around the speed
// and the two direction animations around the angle, weights the children, synchronises them through the group and
// scales the group's rate so the chosen animation's foot speed matches the pawn's. Bodies in Src/arkanimnodelocomotion.cpp
// (agent GC), retail's Engine/Src/arkanimnodelocomotion.cpp (52 functions). It lives in DishonoredGame because this class
// and UArkComponentLocomotionConfig are declared in DishonoredGameEngineShims.h and Engine cannot see them.
public:
	// DISHONORED(port): 2013 rva 0x51b280 (2012 0x55b910), 413 bytes.
	virtual void TickAnim( FLOAT DeltaSeconds );
	// DISHONORED(port): 2013 rva 0x515ec0 (2012 0x556390) / 0x509c00 (2012 0x54a490).
	virtual void OnBecomeRelevant();
	virtual void OnCeaseRelevant();

	// DISHONORED(port): 2013 rva 0x51d4c0 (2012 0x55e050), 550 bytes: what FArkComponentLocomotion::SetConfig hands the node.
	void Initialize( const class UArkComponentLocomotionConfig* const _pConfig );
	// DISHONORED(port): 2013 rva 0x515930 (2012 0x54c370): the node named LocoBlender in the pawn's anim tree.
	static class UArkAnimNodeLocomotion* GetAnimNodeLocomotion( const APawn* const _pPawnOwner );

	// DISHONORED(port): 2013 rvas 0x509a40 / 0x509a60 / 0x509a90 / 0x509aa0 / 0x509ac0 (2012 0x54a2d0 / 0x54a2f0 / 0x54a320 / 0x54a330 / 0x54a350).
	void SetAngle( INT _Angle );
	void SetSpeed( FLOAT _fSpeed, UBOOL _bNoSmooth );
	FLOAT GetSpeed() const { return m_fSpeed; }
	void SetSpeedMultiplier( FLOAT _fSpeedMultiplier );
	void SetModifier( INT _Modifier );
	// DISHONORED(port): 2013 rvas 0x509ad0 .. 0x509c30 (2012 0x54a360 .. 0x54a4c0), the one-line accessors.
	void GetMovementAnimPrevAndCurProgress( FLOAT& _fPrevProgress, FLOAT& _fCurProgress ) const;
	UBOOL IsSpecificAnimRequested() const { return m_bSpecificAnimRequested; }
	UBOOL IsSpecificAnimPlaying() const { return m_bPlayingSpecificAnim; }
	UBOOL IsSpecificAnimRequestedOrPlaying() const { return m_bSpecificAnimRequested || m_bPlayingSpecificAnim; }
	EAnimNodeLocoSpecificAnimType GetTypeOfCurSpecificAnim() const { return (EAnimNodeLocoSpecificAnimType)m_SpecificAnimProps.m_Type; }
	UBOOL IsPlayingOnlySpecificAnim() const;
	void AbortSpecificAnimRequest( UBOOL _bResetEvenIfSpecificAnimPlaying );
	const struct FAnimNodeLocoModifierSlot* GetMostRelevantModifierSlot() const;
	FLOAT GetOutOfShapeRatio() const { return m_fOutOfShapeRatio; }
	FLOAT GetDeltaTime() const { return m_fCurDeltaTime; }
	// DISHONORED(port): 2013 rvas 0x510060 / 0x5101d0 / 0x510340 / 0x510470 (2012 0x550750 / 0x5508c0 / 0x550a30 / 0x550b60):
	// the specific-animation requests of FArkComponentLocomotion's turn / start / stop state machines.
	FLOAT GetDurationBeforePlayingTurnAnim( INT _SpeedModeIdx, INT _TurnAnimIdx ) const;
	void RequestTurnAnim( INT _SpeedModeIdx, INT _TurnAnimIdx, UBOOL _bPlayNow, UBOOL _bInMovement, UBOOL _bFastBlend );
	void RequestStartAnim( INT _SpeedModeIdx );
	void RequestStopAnim( INT _SpeedModeIdx, INT _StopAnimIdx );
	// DISHONORED(port): 2013 rva 0x515980 (2012 0x555e50). GetNextFootOnGroundProperties / GetDefaultLocalRootFootDelta
	// (2013 0x515a00 / 0x515d80) and GetFootIndexFromBoneIndex (0x5117c0) are the foot-placement half and are not ported.
	INT GetFootProperties( INT _BoneIdx, struct FArkCpntLocoFootProp& _FootProperties ) const;

	// DISHONORED(retail): 2012 dynamic initialisers 0xb9f990 / 0xb9f9b0 (2013 0xb83210 / 0xb83230): "LocoBlender", "LocoGroup".
	static const FName s_AnimNodeLocoName;
	static const FName s_AnimNodeLocoGroupName;

	/** -gadiag: one line of the node's state (speeds, modifier slots, child weights, group rate). */
	FString DescribeForDiag() const;

private:
	// DISHONORED(port): the TickAnim phases, in retail's call order (2013 rvas, 2012 in the unit):
	// 0x50bc00 BlendSpeedAndAngleProperties, 0x511230 HandleTransition, 0x50bdb0 UpdateSpecificAnim, 0x516770 UpdateMovementAnims,
	// 0x516950 UpdateMovementChildren, 0x517440 UpdateSpecificChild, 0x5114a0 UpdateFeetPropeties, 0x5113a0 UpdateProgressVariables.
	void BlendSpeedAndAngleProperties( FLOAT _fDeltaSeconds );
	void HandleTransition();
	void UpdateSpecificAnim( FLOAT _fDeltaSeconds );
	void UpdateMovementAnims( FLOAT _fDeltaSeconds );
	void UpdateMovementChildren();
	void UpdateSpecificChild();
	void UpdateFeetPropeties();
	void UpdateProgressVariables( FLOAT _fDeltaSeconds );
	// DISHONORED(port): 2013 rvas 0x5105b0 / 0x510970 / 0x510c90 / 0x510cf0 / 0x510e30 / 0x5118b0.
	void UpdateModifierSlot( INT _ModifierSlotIdx, FLOAT _fSnapOnOneSpeedThreshold );
	FLOAT UpdateMovementChild( const struct FAnimNodeLocoModifierSlot& _ModifierSlot, const struct FAnimNodeLocoSpeedSlot& _SpeedSlot, const struct FAnimNodeLocoAngleSlot& _AngleSlot, INT _ChildSlotIdx, UBOOL _bSetAnim );
	void ResetModifierSlot( INT _ModifierSlotIdx, UBOOL _bResetWeight );
	FLOAT FindFastestSpeed() const;
	FLOAT ComputeBlendingOutOfShapeRatio() const;
	FLOAT GetFinalSpeedMultiplier() const;
	const class UArkComponentLocomotionConfig* GetConfig() const { return (const UArkComponentLocomotionConfig*)m_pConfig; }
	const struct FArkCpntLocoSpeedModeProp& GetSpeedModeProp( INT _SpeedModeIdx ) const;
