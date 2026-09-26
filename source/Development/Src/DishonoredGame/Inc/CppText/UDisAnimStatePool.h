// UDisAnimStatePool cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): bodies in disanimstatepool.cpp, 2013 rvas in agentAV_status.csv.
public:
	UDishonoredAnimTree* GetPooledTree( const UDishonoredAnimTree* Template, USkeletalMeshComponent* SkelComp, class UDishonoredAnimNodeTreeRef_Dynamic* ParentAnimNode );
	void ReturnToPool( const UDishonoredAnimTree* Template, UDishonoredAnimTree* PooledTree );
	void UpdatePoolSettings( const FDynamicTreeTemplate& Settings );

protected:
	FDisAnimTreePool* FindAnimTreePool( const UDishonoredAnimTree* Template );
	void AddTreesToPool( FDisAnimTreePool* Pool, UDishonoredAnimTree* Template, INT NumNewTrees );
	void CallPostAnimNodeInstance( UAnimNode* SourceNode, UAnimNode* DestNode );
