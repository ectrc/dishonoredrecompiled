// UDishonoredAnimNodeTreeRef cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(written): bodies in dishonoredanimnodetreeref.cpp, 2013 rvas in agentAV_status.csv.
public:
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual void TickAnim( FLOAT DeltaSeconds );
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	virtual void GetNodesInternal( TArray<UAnimNode*>& Nodes );
	virtual void CallDeferredInitAnim();
	virtual void PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap );

	static void InitTreeInstance( UDishonoredAnimTree* Tree, UAnimTree* ParentTree, USkeletalMeshComponent* MeshComp );
