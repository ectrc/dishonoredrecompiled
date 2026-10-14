// UDisItemPowers cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// agent GJ (PHASE20 GJ): the tweaks slots (GetTweaks_Derived 2012 0x887b70, 7 B, m_pPowersItemTweaks; SetTweaks_Derived 0x88b250, 16 B).
// The powers item is the left hand's default (UDisTweaks_PlayerPawn::m_pPowersItem, spawned by PostBeginPlay_Inventory); it is not a
// physical item (ItemIsPhysicallyEquipped is overridden FALSE in retail's vtable slot 76) - the power components have no bodies here.
// Bodies in disitempowers.cpp.
public:
	virtual class UDisTweaksBase* GetTweaks_Derived();
	virtual UBOOL ItemIsPhysicallyEquipped() const { return FALSE; }
protected:
	virtual void SetTweaks_Derived( class UDisTweaksBase* _pTweaks );
public:
