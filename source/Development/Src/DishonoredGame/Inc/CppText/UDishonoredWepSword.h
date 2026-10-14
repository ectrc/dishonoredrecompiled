// UDishonoredWepSword cpptext: included inside the generated class body (DishonoredGameWeaponClasses.h).
// agent GJ (PHASE20 GJ): the tweaks slots (GetTweaks_Derived 2012 0x886e00, 7 B, m_pSwordTweaks; SetTweaks_Derived 0x889920, 16 B) and
// InitItem 0x8896e0 (143 B: the base, then the player mesh's first material wrapped in a transient UMaterialInstanceConstant so the
// blood and opacity parameters are the sword's own). Bodies in dishonoredwepsword.cpp.
public:
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual void InitItem( class UDisTweaks_InventoryItem* _pItemTweaks );
protected:
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
public:
