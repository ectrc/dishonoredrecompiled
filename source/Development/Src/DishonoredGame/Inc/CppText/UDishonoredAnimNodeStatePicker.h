// UDishonoredAnimNodeStatePicker cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(written): bodies in dishonoredanimnodestatepicker.cpp, 2013 rvas in agentAV_status.csv.
public:
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual void TickAnim( FLOAT DeltaSeconds );
	virtual void GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys );
	virtual void OnCeaseRelevant();
	virtual void OnRemoveChild( INT ChildNum );
	virtual void PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap );

	UDishonoredAnimTree* SetState( FName StateName, FName TreeName, UBOOL bNonLooping );
	UDishonoredAnimTree* SetState( FName StateName, FName TreeName, UBOOL bNonLooping, FLOAT BlendSpeed );
	void ClearState();
	void CleanupForAnimStates( UBOOL bFullDestroy );
	UBOOL HasActiveState() const { return m_iCurStateSlot != INDEX_NONE; }
	UBOOL IsStateFullWeight() const;
	UDishonoredAnimTree* GetDominantTree() const;

protected:
	void InitStateSlots();
	void RenameChildren();
