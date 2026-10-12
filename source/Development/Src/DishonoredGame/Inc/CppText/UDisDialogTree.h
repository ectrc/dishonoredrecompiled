// UDisDialogTree cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x88b210 (2012 0x8f97f0), disdialogtree.cpp
public:
	virtual void Serialize( FArchive& Ar );

// DISHONORED(port): agent FR (PHASE17 FR). The tree-level pieces the direct path needs: the hook-info table every
// hook lookup indexes (GetDialogHookInfo 2013 rva 0x892170's sibling, InitHookInfo 0x88d820 - 18,920 bytes of
// straight-line initialisation, ported as a table in disdialogtree.cpp), the one-shot binding
// (BindToOneShot 2013 0x88b2c0's sibling, 2012 0x8f86c0), InitNode (vtable slot 73, 2012 0x914190) and
// SupportsDialogHook (2013 0x892280). Bodies in disdialogtree.cpp.
public:
	static TArray<struct FDisDialogHookInfo>& GetDialogHookInfo();
	void BindToOneShot( class ADisDialogOneShot* _pOneShot );
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	UBOOL SupportsDialogHook( eDisDialogHook _Hook ) const;
	UBOOL InFallbackChain( const UDisDialogTree* _pTree ) const;
	virtual void PostLoad();
protected:
	static void InitHookInfo();
