// UDishonoredGlobalAIManager cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x860430 (2012 0x8cecd0), dishonoredglobalaimanager.cpp
public:
	virtual void Serialize( FArchive& Ar );

// ---- agent CG (PHASE9 CG): the brain registry and the stim manager ----
// The brains form an intrusive singly-linked list through UDishonoredAIBrain::m_pGlobalAI_NextBrain, with m_pBrainList
// as the head; that is why AddBrain and RemoveBrain are O(1) and O(n) respectively and why nothing else may write that
// member.
public:
	void Init_GlobalAI();
	void AddBrain( class UDishonoredAIBrain* _pAddMe );
	void RemoveBrain( class UDishonoredAIBrain* _pRemoveMe );
	class UDisStimManager* GetStimManager() const { return m_pStimManager; }
	INT GetNumBrains() const;

	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x841480 - retail folds GameSave onto it.
	// Body in dissavegame.cpp. Its blackboard reference is how UDisAIBlackboard enters the stream.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED), 2012 rva 0x6fa960 (dishonoredplayerpawn.cpp:1541), the body
	// retail shares between this class, UDishonoredGlobalAIManager and UDisNPCTravelManager.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return Location == SLL_FILE; }

// ---- agent GI (PHASE20 GI): the attention tags and the attentive-brain queries ----
// MakeNewAttentionTag 2012 rva 0x8af120 (13 B: ++m_iCurrentAttentionTag), GetAttentiveBrains 0x8c4ae0 / 2013 0x854b70 and
// 0x8c4b80 / 0x854c10, HasAttentiveBrains 0x8b1190, IsIgnoreDeathOverridden 0x8c0290 / 0x851d50 (54 B). Bodies in
// Src/dishonoredglobalaimanager.cpp.
public:
	INT MakeNewAttentionTag() { return ++m_iCurrentAttentionTag; }
	void GetAttentiveBrains( const class ADishonoredPawn* _pPawn, TArray< class UDishonoredAIBrain*, TMemStackAllocator< GMainThreadMemStack > >& _rOut_Brains, BYTE _RequiredRelationship, BYTE _RequiredAttentionLevel ) const;
	UBOOL HasAttentiveBrains( const class ADishonoredPawn* _pPawn, BYTE _RequiredRelationship, BYTE _RequiredAttentionLevel ) const;
	UBOOL IsIgnoreDeathOverridden( const class UDishonoredAIBrain& _rIgnorer, const class IDisCorpseInterface& _rIgnoree ) const;
