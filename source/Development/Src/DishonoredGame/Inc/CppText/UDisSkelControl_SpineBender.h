// UDisSkelControl_SpineBender cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DishonoredGame.DisSkelControl_SpineBender is the single-bone controller every NPC anim tree carries on spine_0_jnt
// (ControlStrength 1.00): its pitch and yaw targets are set in degrees by the pawn (ADishonoredNPCPawn::
// Tick_SpineBending_Apply - the slope and turn offsetting) and blended toward at a speed per axis in TickSkelControl;
// CalculateNewBoneTransforms is the stock single-bone rotation plus a record of the unbent bone. Bodies in
// Src/disskelcontrol_spinebender.cpp (agent FT).
public:
	// DISHONORED(port): 2013 rva 0x68cbe0 (2012 0x6dae30), 256 bytes.
	virtual void TickSkelControl( FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp );
	// DISHONORED(port): 2013 rva 0x68cce0 (2012 0x6daf30), 309 bytes.
	virtual void CalculateNewBoneTransforms( INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FBoneAtom>& OutBoneTransforms );
	// DISHONORED(port): 2013 rvas 0x67eb90 / 0x67ebe0 (2012 0x6d02a0 / 0x6d02f0).
	void EnableSpineBending( UBOOL _bEnable, UBOOL _bSetInstantly );
	UBOOL IsSpineBendingEnabled();
	// DISHONORED(port): 2013 rvas 0x684370 / 0x6843d0 (2012 0x6d6150 / 0x6d61b0). Degrees.
	void SetBendPitchTarget( FLOAT _fBendPitchTarget, UBOOL _bSetInstantly, const FLOAT* const _pfBendPitchBlendSpeed );
	void SetBendYawTarget( FLOAT _fBendYawTarget, UBOOL _bSetInstantly );
