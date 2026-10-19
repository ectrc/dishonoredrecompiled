// UDishonoredAnimNodeSeqMantle cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The mantle's sequence: its root motion's world Z is scaled from the animation's own climb
// (m_fMantleTravelDistanceZ, cooked) to the edge the mantle state climbs. 2012 / 2013 rvas: PostAnimNodeInstance 0x6d0d00 / 0x67fc90,
// GetBoneAtoms 0x6e10d0 / 0x68f7a0, SetAnim 0x6ead00 / 0x698000. Bodies in Src/dishonoredanimnodeseqmantle.cpp.
public:
	virtual void SetAnim( FName Sequence );
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	virtual void PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap );
