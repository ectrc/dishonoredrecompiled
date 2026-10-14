// UDisSkelControl_FootPlacement cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// agent GF (PHASE19 GF, the coordinator's mid-wave message on GC's finding): the foot-placement control is a retail class
// whose unit was comment-only, so its cooked instances (two per NPC, strength 1.00) ran the REFERENCE engine's
// USkelControlFootPlacement limb IK over Dishonored properties - the stretched silhouettes. Retail's entry
// CalculateNewBoneTransforms 2012 0x6ed860 (98 B) records the bone and the component and dispatches to
// CalculateNewBoneTransformsOriginal 0x6e4d70 (7,302 B) or, under the game-info's new-IK switch with m_bCanUseNewIK and not
// m_bDisableNewIK, CalculateNewBoneTransformsNewComer 0x6e8ed0 (830 B). Neither body is ported (the control's 28 functions are
// 30 KB of IK: UpdateFootOrientation 6,565 B, UpdateFootOrientationTarget 2,674 B, UpdatePredictionProperties 2,308 B ...),
// so the entry outputs no transform: the control is live at its cooked strength and contributes nothing, which is the
// strength-0 measurement the coordinator asked for, taken without touching the cooked properties. EnableFootPlacement
// 0x6d01c0 and LockFootPlacement 0x6d0230 are ported. Bodies in disskelcontrol_footplacement.cpp.
public:
	virtual void CalculateNewBoneTransforms( INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FBoneAtom>& OutBoneTransforms );
	void EnableFootPlacement( UBOOL _bEnable, UBOOL _bSetInstantly );
	void LockFootPlacement( UBOOL _bLock );
