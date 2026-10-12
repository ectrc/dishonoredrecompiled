// UArkAnimNodeLookAt cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// Engine.ArkAnimNodeLookAt is the NPC anim tree's head- and torso-aim blender: eighteen additive poses on two
// 3x3 grids (head 0..8, torso 9..17, index = column*3 + row). The blend bodies (agent FL) are in
// Src/dishonoredplayercontroller.cpp under "agentFL:lookatnode"; the config half - the triangle mapping, the head/torso
// split, the mode crossfade and the per-mode anim names (agent FT) - is in Src/arkanimnodelookat.cpp. Both live in
// DishonoredGame because this class is declared in DishonoredGameEngineShims.h and Engine cannot see it; their retail
// home is Engine/Src/arkanimnodelookat.cpp once the class moves into Engine.
public:
	// DISHONORED(port): 2013 rva 0x5138d0 (2012 0x567330). Zeroes all eighteen pose weights.
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	// DISHONORED(port): 2013 rva 0x526320 (2012 0x5672c0). Inert while m_pConfig is NULL, as in retail.
	virtual void TickAnim( FLOAT DeltaSeconds );
	// DISHONORED(port): 2013 rva 0x51bc80 (2012 0x55c310). Composes the head set and the torso set.
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	// DISHONORED(port): 2013 rva 0x512570 (2012 0x552c60). The current mode's eighteen anim names and the two grids' ranges.
	virtual void CheckAnimsUpToDate();
	// DISHONORED(port): 2013 rva 0x50a820 (2012 0x54b0b0). Frees the two BlendInfos.
	virtual void BeginDestroy();
	// DISHONORED(port): 2013 rva 0x513990 (2012 0x553fc0). The nine weights of one set.
	FLOAT GetAnimInfoTotalWeight( INT SetIndex );
	using UAnimNodeSequenceBlendBase::GetAnimInfoTotalWeight;
	// DISHONORED(port): 2013 rva 0x50a0b0 (2012 0x54a940). Degrees.
	void SetAim( const FVector2D& NewAim );
	// DISHONORED(port): 2013 rva 0x512bf0 (2012 0x5532e0). Aim[0] head, Aim[1] torso, both in [-1,1].
	void UpdateNodeWeights( const FVector2D* Aim );
	// DISHONORED(port): 2013 rva 0x50a090 (2012 0x54a920). What FArkComponentLookat::Starting hands the node.
	void SetConfig( const class UArkComponentLookatConfig* _pConfig );
	// DISHONORED(port): 2013 rvas 0x50c410 / 0x50c440 / 0x50c480 / 0x50a110 (2012 0x54cbc0 / 0x54cbf0 / 0x54cc30 / 0x54a9a0).
	void SetLookAtMode( INT _LookAtMode );
	void SetUseHeadRange( FLOAT _fUseHeadRange );
	void SetUseTorsoRange( FLOAT _fUseTorsoRange );
	void SetUseTorsoDeadZone( FLOAT _fUseTorsoDeadZone );
	// DISHONORED(port): 2013 rvas 0x50a0f0 / 0x50a100 / 0x50a710 (2012 0x54a980 / 0x54a990 / 0x54afa0).
	FLOAT GetUseHeadRange() const { return m_fUseHeadRange; }
	FLOAT GetUseTorsoRange() const { return m_fUseTorsoRange; }
	const FVector2D& GetCurrentBlendedAim() const { return m_CurrentBlendedAim; }
	// DISHONORED(port): 2013 rva 0x50c5b0 (2012 0x54cd60). Degrees.
	FVector2D GetMaxAim( const FVector2D& _AimDirection );
	// DISHONORED(port): 2013 rva 0x5176f0 (2012 0x557b80). The head and torso grids' normalised aims from m_Aim.
	void ComputeHeadAndTorsoAim( FVector2D* const _SafeAimArray );
	// DISHONORED(port): 2013 rva 0x517580 (2012 0x54cb90). The node named LookatBlender in the pawn's anim tree.
	static class UArkAnimNodeLookAt* GetAnimNodeLookAt( const APawn* const _pPawnOwner );

	// DISHONORED(retail): 2012 dynamic initialiser 0xb9f9d0, "LookatBlender".
	static const FName s_AnimNodeLookAtName;

private:
	// DISHONORED(port): 2013 rva 0x51b420 (2012 0x55bab0). Retail declares this virtual and nothing overrides
	// it; a second virtual GetBoneAtoms overload that overrides no base virtual is C4263, an error in this
	// tree, so it is a private non-virtual helper with retail's body. Its only caller is GetBoneAtoms above,
	// which is retail's only caller too.
	void GetBoneAtomsForSet( INT SetIndex, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	// DISHONORED(port): 2013 rva 0x50a720 (2012 0x54afb0). Editor-only; a no-op in the game.
	void AutoInitConfigInAnimTreeEd();
	// DISHONORED(port): 2013 rva 0x50c650 (2012 0x54ce00). TRUE when the weights must be recomputed this tick.
	UBOOL HandleBlendingBetweenLookatNodes( FLOAT _fDeltaSeconds );
	// DISHONORED(port): the constructor's two BlendInfos allocations (2013 rva 0x523e10), made on first use here.
	void EnsureBlendInfos();
