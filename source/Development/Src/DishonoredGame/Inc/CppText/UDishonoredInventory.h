// UDishonoredInventory cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(written): the abstract-item / ammo / elixir core of the retail inventory, the part the Kismet sequence actions of
// ADishonoredPawn reach (bodies with their 2013 rvas in dishonoredinventory.cpp). Equipping, slots, item spawning, save/load
// and the UI interactions are not ported (they need UDisItemContext and the item tweaks; see agentAJ.md).
public:
	UDishonoredInventoryItem* GetEquippedItem( BYTE Usage ) const;

	void AddAbstractItem( UDisAbstractItem* Item, INT Quantity, UBOOL bAddToFront );
	void RemoveAbstractItem( UDisAbstractItem* Item, INT Quantity );
	void HideAbstractItem( UDisAbstractItem* Item );
	INT GetAbstractItemQuantity( UDisAbstractItem* Item, UBOOL* bOutHidden = NULL ) const;

	const FDisAmmoInfo* GetAmmoInfo( BYTE AmmoType ) const;
	UBOOL HasAmmo( BYTE AmmoType, INT Count ) const;
	INT AddAmmo( BYTE AmmoType, INT Count );
	void SetAmmo( BYTE AmmoType, INT Count, UBOOL bIgnoreCapacity );
	UBOOL ConsumeAmmo( BYTE AmmoType, INT Count );

	void AddElixir( BYTE ElixirType, INT Count );
	void SetElixirCount( BYTE ElixirType, INT Count );
	// DISHONORED(written): agent AU. ConsumeStatPickup 2013 rva 0x805440 (2012 0x8529e0), body in
	// dishonoredinventory.cpp; the inventory side of ADisStatPickup::DoInteract_Impl.
	UBOOL ConsumeStatPickup( class ADisStatPickup* Pickup, INT* OutConsumedAmmoCount );

	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x8169e0 / 0x816bc0. Body in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

	// ---- agent FV (PHASE18 FV): the three lookups the NPC's look-at choice and the go-to's resume read ----
	// 2013 rvas 0x80b4a0 / 0x80f870 / 0x804c80 (2012 0x852180 / 0x855bc0 / 0x852060). Bodies in dishonoredinventory.cpp.
	UDishonoredInventoryItem* FindItemByClass( const UClass* const _pItemClass, UBOOL _bExactMatch, UBOOL _bSkipEmptyHands, UBOOL _bSkipNonRemovable, BYTE _EquipUsage ) const;
	UBOOL HasItemByClass( const UClass* _pItemClass, UBOOL _bExactMatch ) const;
	UDishonoredInventoryItem* FindEquippableItem( const UClass* const _pItemClass, BYTE _EquipUsage ) const;

	// agent GF (PHASE19 GF): OnCameraUpdate 2012 0x85a730 / 2013 0x817c90 (118 B, the same): every slot's item that should tick gets the view
	void OnCameraUpdate( const struct FDishonoredViewTarget& _rVT );

// ---- agent GJ (PHASE20 GJ): the equip path ----
// DISHONORED(port): 2012 / 2013 rvas - InitInventory 0x85b790 / 0x818c80, AddInventorySlot 0x851d60 / 0x804b80, GetRequiredSlot
// (class) 0x851e60 / 0x80b2a0, GetRequiredSlot (item) 0x8559f0 / 0x80f680, GetNextEmptySlot 0x851f10 / 0x80b350, CanEquipItemType
// (slot) 0x855df0 / 0x80f920, CanEquipItemType (tweaks) 0x857f00 / 0x811b50, DoEquip 0x85a7b0 / 0x817d10, EquipItemType 0x85a940 /
// 0x817eb0, AttachItemToSocket 0x855a50 / 0x80f6e0, ShowInventoryItem 0x857fb0 / 0x811c00, ShowEquippedItem 0x858040 / 0x811c90,
// SetReequipItem 0x852a90 / 0x805500, GetReequipItem 0x852b70 / 0x8055e0, AddInventoryItem 0x85be60 / 0x819380 (2013 adds
// _bRemoveIfNeeded), SpawnAndAddInventoryItem 0x85c9c0 / 0x819cd0, TickInventory 0x85c760, ClearInventory 0x861a90 / 0x8180a0,
// RemoveItemsByClass 0x85aab0, PlayerTransitionItemIn 0x851e10 / 0x804c30. Bodies in dishonoredinventory.cpp.
public:
	void InitInventory( class ADishonoredPawn* _pOwner, class UDisTweaks_InventoryItem* _pEmptyHandsType );
	void AddInventorySlot( UClass* _pRequiredClass, BYTE _RequiredUsage );
	INT GetRequiredSlot( UDishonoredInventoryItem* _pItem );
	UBOOL CanEquipItemType( UClass* _pType, BYTE _EquipUsage, const class UDisTweaks_InventoryItem** _ppItemThatWillEquip );
	UBOOL EquipItemType( UClass* _pType, BYTE _EquipUsage, UBOOL _bTemporary );
	UBOOL AttachItemToSocket( UDishonoredInventoryItem* const _pItem, BYTE _ItemSocket );
	UBOOL ShowEquippedItem( UBOOL _bShow, BYTE _EquipUsage );
	void SetReequipItem( BYTE _EquipUsage, UClass* _pType );
	UClass* GetReequipItem( BYTE _EquipUsage, UBOOL _bNonEmptyRequired ) const;
	UBOOL AddInventoryItem( UDishonoredInventoryItem* const _pItem, UBOOL _bDropIfNeeded, UBOOL _bShowIfNeeded, BYTE _EquipUsage );
	UDishonoredInventoryItem* SpawnAndAddInventoryItem( class UDisTweaks_InventoryItem* const _pItemTweaks, UBOOL _bDropIfNeeded, UBOOL _bShowIfNeeded, BYTE _EquipUsage );
	void TickInventory( FLOAT _fDeltaTime );
	void ClearInventory();
	void RemoveItemsByClass( const UClass* _pClass, UBOOL _bExactMatch );
	BYTE PlayerTransitionItemIn( BYTE _EquipUsage, BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip );
protected:
	INT GetRequiredSlot( UClass* _pRequiredClass, UBOOL _bOwnerSpecific, BYTE _EquipUsage, struct FPawnInventorySlot** _ppOutSlot );
	INT GetNextEmptySlot( INT _iCur, UDishonoredInventoryItem* _pForItem );
	UBOOL CanEquipItemType( UClass* _pType, BYTE _EquipUsage, INT& _iSlot );
	void DoEquip( INT _iSlot, BYTE _EquipUsage, UBOOL _bTemporary );
	UBOOL ShowInventoryItem( UDishonoredInventoryItem* _pItem, UBOOL _bShow, BYTE _EquipUsage );
public:
