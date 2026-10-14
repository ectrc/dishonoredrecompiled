// UDishonoredItemSkeletalComponent cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// agent GJ (PHASE20 GJ): SetInventoryItem 2012 0x861d90 (the write InitItem makes; 2013 inlined at 0xc0fa7d / 0xc0fa8a), GetOwningInventoryItem
// 0x861da0. The component is the item's mesh: UDishonoredPlayerSkeletalComponent's offset / FOV render path (agent GF) applies to it as retail's does.
public:
	void SetInventoryItem( class UDishonoredInventoryItem* _pItem ) { m_pItem = _pItem; }
	class UDishonoredInventoryItem* GetOwningInventoryItem() const { return m_pItem; }
