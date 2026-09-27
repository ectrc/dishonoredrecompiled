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
