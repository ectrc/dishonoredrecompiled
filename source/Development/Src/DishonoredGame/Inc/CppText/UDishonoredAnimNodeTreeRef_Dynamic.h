// UDishonoredAnimNodeTreeRef_Dynamic cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(written): bodies in dishonoredanimnodetreeref_dynamic.cpp, 2013 rvas in agentAV_status.csv.
public:
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual void PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap );
	virtual void BuildParentNodesArray();
	virtual void BuildTickArray( TArray<UAnimNode*>& OutTickArray );
	virtual void OnCeaseRelevant();

	UDishonoredAnimTree* SetActiveTreeReference( FName TreeRefName );
	void ClearActiveTreeReference( UBOOL bCeaseRelevant );
	void CleanupForAnimStates( UBOOL bFullDestroy );

protected:
	void CheckForReleaseToPool( INT Index );
