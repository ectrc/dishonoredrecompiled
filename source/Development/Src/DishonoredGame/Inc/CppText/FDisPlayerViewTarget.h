// FDisPlayerViewTarget cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(port): agent HD (PHASE22 HD). Bodies in dishonoredplayercontroller_tracking.cpp: the fixed-angle SetTarget 2012 0x6d15b0 /
// 2013 0x6a0da0, ClearTarget 0x6d1670 / 0x6a0e60, TickTracking 0x6f8840 / 0x6af3a0. IsTracking is the test every caller inlines: a fixed
// target, or an actor target whose actor is still there.
	// agent JE (PHASE24 JE): the actor SetTarget, 2012 0x6d14c0 (227 B)
	void SetTarget( class AActor* _pTarget, class USkeletalMeshComponent* _pMesh, FName _LookBone, const FVector& _rLookBoneOffset, FLOAT _fBlendTime, INT _Flags, INT _StartRot, FLOAT _fMaxRadius, FLOAT _fMinRadius, INT _MaxPlayerControlledDelta, INT _PlayerControlledDelta_SoftenThreshold, FLOAT _fZoomFOVOverride );
	void SetTarget( INT _TargetFixedAngle, FLOAT _fBlendTime, INT _Flags, INT _StartRot, FLOAT _fMaxRadius, FLOAT _fMinRadius, INT _MaxPlayerControlledDelta, INT _PlayerControlledDelta_SoftenThreshold, FLOAT _fZoomFOVOverride );
	void ClearTarget( INT _TrackID );
	void TickTracking( FLOAT _fDeltaTime );
	// DISHONORED(port): agent IA (PHASE23 IA), 2012 0x6d7420 / 2013 0x6a34d0 (same bytes)
	void ApplyPlayerControlDelta( INT _Delta );
	UBOOL IsTracking() const { return m_TrackType != 0 && ( m_TrackType != 1 || m_pTarget != NULL ); }
