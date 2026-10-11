// UArkAnimNodeLookAt cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// Engine.ArkAnimNodeLookAt is the NPC anim tree's head- and torso-aim blender: eighteen additive poses on two
// 3x3 grids (head 0..8, torso 9..17, index = column*3 + row). Bodies, and the measurement that found the fault,
// are in Src/dishonoredplayercontroller.cpp under "agentFL:lookatnode" - they live there because this class is
// declared in DishonoredGameEngineShims.h and Engine cannot see it, and because a unit that only ever reaches
// the link through IMPLEMENT_CLASS's folded vtable COMDAT can be dropped whole (agent FI). Their retail home is
// Engine/Src/arkanimnodelookat.cpp once the class moves into Engine.
public:
	// DISHONORED(port): 2013 rva 0x5138d0 (2012 0x567330). Zeroes all eighteen pose weights.
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	// DISHONORED(port): 2013 rva 0x526320 (2012 0x5672c0). Inert while m_pConfig is NULL, as in retail.
	virtual void TickAnim( FLOAT DeltaSeconds );
	// DISHONORED(port): 2013 rva 0x51bc80 (2012 0x55c310). Composes the head set and the torso set.
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	// DISHONORED(port): 2013 rva 0x513990 (2012 0x553fc0). The nine weights of one set.
	FLOAT GetAnimInfoTotalWeight( INT SetIndex );
	using UAnimNodeSequenceBlendBase::GetAnimInfoTotalWeight;
	// DISHONORED(port): 2013 rva 0x50a0b0 (2012 0x54a940)
	void SetAim( const FVector2D& NewAim );
	// DISHONORED(port): 2013 rva 0x512bf0 (2012 0x5532e0). Aim[0] head, Aim[1] torso, both in [-1,1].
	void UpdateNodeWeights( const FVector2D* Aim );

private:
	// DISHONORED(port): 2013 rva 0x51b420 (2012 0x55bab0). Retail declares this virtual and nothing overrides
	// it; a second virtual GetBoneAtoms overload that overrides no base virtual is C4263, an error in this
	// tree, so it is a private non-virtual helper with retail's body. Its only caller is GetBoneAtoms above,
	// which is retail's only caller too.
	void GetBoneAtomsForSet( INT SetIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	// DISHONORED(port): 2013 rva 0x50a720 (2012 0x54afb0). Editor-only; a no-op in the game.
	void AutoInitConfigInAnimTreeEd();
