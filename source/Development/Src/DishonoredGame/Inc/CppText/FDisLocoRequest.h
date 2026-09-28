// FDisLocoRequest cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(written): agent DF. "Walk there." Owns an FArkComponentLocomotion request id, the speed index and the arrival
// parameters; the target lives in FDisDesireRequest. Bodies in Src/disdesirestructs.cpp.
// DISHONORED(bringup): FArkComponentLocomotion (117 functions, agentCG.md "What moving still needs") is a package of its
// own and is NOT this one, so ADishonoredNPCPawn::GetComponentLocomotion answers NULL and DoRequest stops at the
// component boundary with a counted note. This is the point the brief means by "have it request and say what it
// requested": every move an NPC wants is resolved, filtered and counted here, and only the execution is missing.
public:
	// DISHONORED(port): 2013 rva 0x8b4280 (2012 0x900880)
	void Initialize( class FArkComponentLocomotion* _pLocoComponent, class UObject* _pAsker, BYTE _Priority, void* _pCallback );
	// DISHONORED(port): 2013 rvas 0x8b7980 / 0x8b7a30 / 0x8b7ba0 (2012 0x902160 / 0x902210 / 0x902380)
	void RequestProxyTarget( const struct FDisAttentionProxy& _rProxyTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	void RequestActorTarget( class AActor* _pActorTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	void RequestLocationTarget( const FVector& _rLocationTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent );
	// DISHONORED(port): 2013 rva 0x8b7ae0 (2012 0x9022c0)
	void RequestFollowTarget( class AActor* _pActorTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent, UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist );
	// DISHONORED(port): 2013 rva 0x8b7c50 (2012 0x902430)
	void UpdateRequest();
	// DISHONORED(port): 2013 rva 0x8ae200 (2012 0x8fcdc0)
	void ClearRequest();
	// DISHONORED(port): 2013 rva 0x8ae170 (2012 0x8fcd30)
	void PauseRequest();
	// DISHONORED(port): 2013 rva 0x8a9d80 (2012 0x8fa170)
	void DoRequest( enum EDisDesireRequestStatus _Status );
	// DISHONORED(port): 2013 rva 0x8a7f80 (2012 0x8f8fa0)
	void StopRequest();

private:
	// DISHONORED(port): 2013 rva 0x8a7ea0 (2012 0x8f8ec0)
	void SetParams( INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent, UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist, enum EDisDesireRequestStatus& _rStatus );
	/** DISHONORED(port): retail inlines this prologue everywhere; see FDisFaceToRequest::SyncRequest. */
	void SyncRequest();
	void ClearParams();
