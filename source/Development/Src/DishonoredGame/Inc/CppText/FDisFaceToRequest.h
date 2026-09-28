// FDisFaceToRequest cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Turn to face that." Owns an FArkComponentFaceTo request id and the rotation parameters;
// the target itself lives in FDisDesireRequest. Bodies in Src/disdesirestructs.cpp.
// DISHONORED(bringup): FArkComponentFaceTo is not ported (it is part of the FArkComponentLocomotion package agent CG
// costed), so ADishonoredNPCPawn::GetComponentFaceTo answers NULL, no request id is ever issued and DoRequest stops at
// the component boundary with a counted note. Everything above the boundary - the target resolution, the redundancy
// filter and the status machine - runs, which is what makes "what did the NPC ask for" measurable.
public:
	// DISHONORED(port): 2013 rva 0x8b4230 (2012 0x900830)
	void Initialize( class FArkComponentFaceTo* _pFaceToComponent, class UObject* _pAsker, BYTE _Priority, void* _pCallback );
	// DISHONORED(port): 2013 rvas 0x8b76f0 / 0x8b7770 / 0x8b77f0 / 0x8b7870 (2012 0x901ed0 / 0x901f50 / 0x901fd0 / 0x902050)
	void RequestProxyTarget( const struct FDisAttentionProxy& _rProxyTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void RequestActorTarget( class AActor* _pActorTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void RequestLocationTarget( const FVector& _rLocationTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	void RequestYawTarget( INT _YawTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation );
	// DISHONORED(port): 2013 rva 0x8b7920 (2012 0x902100)
	void UpdateRequest();
	// DISHONORED(port): 2013 rva 0x8ae0f0 (2012 0x8fccb0)
	void ClearRequest();
	// DISHONORED(port): 2013 rva 0x8ae080 (2012 0x8fcc40)
	void PauseRequest();
	// DISHONORED(port): 2013 rva 0x8a9b30 (2012 0x8f9f20)
	void DoRequest( enum EDisDesireRequestStatus _Status );
	// DISHONORED(port): 2013 rva 0x8a7e80 (2012 0x8f8ea0)
	void StopRequest();

private:
	// DISHONORED(port): 2013 rva 0x8a7e00 (2012 0x8f8e20): the redundancy filter - identical parameters leave the status
	// alone, changed ones promote Unchanged to UpdateRequestNeeded while a request is live and unpaused.
	void SetParams( UBOOL _bUsingYaw, INT _YawTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation, enum EDisDesireRequestStatus& _rStatus );
	/** DISHONORED(port): retail inlines this prologue into ClearRequest, PauseRequest, UpdateRequest and all four
	    Request*Target (the `m_RequestID != INDEX_NONE && !m_pFaceToComponent->m_bStarted` guard plus the parameter
	    reset); FDisLookAtRequest keeps it as a real function (SyncRequest, 2013 rva 0x8b42d0), so it is named here too. */
	void SyncRequest();
	void ClearParams();
