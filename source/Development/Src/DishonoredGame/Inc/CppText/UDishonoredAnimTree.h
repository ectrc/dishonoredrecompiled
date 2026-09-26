// UDishonoredAnimTree cpptext: included inside the generated class body (DishonoredGameAnimClasses.h).
// DISHONORED(written): bodies in dishonoredanimtree.cpp, 2013 rvas in agentAV_status.csv.
public:
	virtual void InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual void OnCeaseRelevant();

	void EndAnimState();
	void GetAnimStateInfo( FDisAnimStateTreeInfo* Out ) const;
	UBOOL IsAnimStateActive() const;
	class IDisAnimStateFiringInterface* GetAnimStateFireInterface() const;
