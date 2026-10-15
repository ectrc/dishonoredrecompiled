// UArkAnimNodeBlendPose cpptext: included inside the shim class body (DishonoredGameEngineShims.h; the class is an
// Engine-package class the Engine header generation left out, as UInterpGroupPlayer is).
// DISHONORED(port): agent FZ (PHASE18 FZ) SetEnable 2012 0x54a250, BlendNow 0x54ffb0; agent HB (PHASE22 HB) the blend
// itself: InitAnim 2012 0x1c3040 (2013 0x91b0a0, slot 80), TickAnim 0x550080 (2013 0x90f990), SetActiveChild 0x550360
// (2013 0x90fc70), ForceDisable 0x54a290, and GetBoneAtoms = BuildEdgeAnimTree 0x55dd40 (2013 0x91d150, slot 91) on the CPU.
// Bodies in Src/arkanimnodeblendpose_dis.cpp.
public:
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual void TickAnim( FLOAT DeltaSeconds );
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	virtual void SetActiveChild( INT ChildIndex, FLOAT BlendTime );
	virtual void SetEnable( UBOOL bEnable );
	void BlendNow( FLOAT BlendTime, UBOOL bWaitForNextFrame );
	void ForceDisable();
private:
	void ReportBlend( const TCHAR* Event );
