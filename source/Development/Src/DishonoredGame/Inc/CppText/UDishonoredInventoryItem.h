// UDishonoredInventoryItem cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// agent FV (PHASE18 FV): the look-at mode an NPC holding this item takes while idle, walking or running, read by
// ADishonoredNPCPawn::ChooseLookAt. UObject vtable slot 85 (2012 ??_7UDishonoredInventoryItem@@6BUObject@@@ 0xd5c3c8 + 340),
// whose body is 0xa6cca0 - `return 0`, EmptyIdle whatever the speed. UDishonoredWeapon (0xd72ae0) and UDisWepMelee (0xd72cd8)
// carry the same body in that slot, so a guard with a sword in hand answers EmptyIdle too; only UDishonoredItemEmpty
// (0x887cf0), UDisWeapon_WolfhoundBite (0x886d00) and UDisWepMusicBox (0x8993f0) override it.
public:
	virtual BYTE GetDefaultNPCLookAtMode( BYTE _TransitSpeed ) const { return eDisNPCLookAtMode_EmptyIdle; }

// ---- agent GF (PHASE19 GF): the camera pass on an item ----
// DISHONORED(port): ShouldItemTick 2012 0x853da0 / 2013 0x80c770 (132 / 110 B: the tweaks' m_ItemTickMode against the item's equip usage and
// socket), OnCameraUpdate 2012 0x859da0 / 2013 0x816e30 (158 B, the same: UpdateMeleeExtents for a ticking item, every active context's
// OnCameraUpdate, then the derived hook - UDishonoredItemEmpty's 0x8a5330 moves a grabbed movable). Bodies in dishonoredinventoryitem.cpp.
public:
	UBOOL ShouldItemTick() const;
	void OnCameraUpdate( const struct FDishonoredViewTarget& _rVT );
protected:
	virtual void OnCameraUpdate_Derived( const struct FDishonoredViewTarget& _rVT ) {}
public:
