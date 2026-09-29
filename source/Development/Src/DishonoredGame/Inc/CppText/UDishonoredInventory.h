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
