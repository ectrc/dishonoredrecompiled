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
