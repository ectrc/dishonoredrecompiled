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

// ---- agent GJ (PHASE20 GJ): the item's half of the equip path ----
// DISHONORED(port): 2012 / 2013 rvas - InitItem 0x8580f0 / 0x80f9f0 (UObject vtable slot 75 / 76), OnAttached 0x85c030 / 0x8196a0
// (77 / 78), OnDetached 0x849ca0 / 0x7fed20 (78 / 79), OnAddToInventory 0x858390 (79 / 80; 2013 unnamed), InitItemContextSlot
// 0x8581c0, OnEquipItem 0x856240 / 0x80feb0, OnUnEquipItem 0x859e40 / 0x816ed0, IsEquippable 0x853cb0 / 0x80c680,
// ApplyTweakChanges_Derived 0x85cc80 / 0x819ff0, TickInventoryItem 0x852e00 / 0x80b990, GetItemAimForAnims_Derived 0x854070,
// ModifyPlayerStance 0x849c30 / 0x7fecd0, GetContextsForSlot 0x84dbb0 / 0x801bb0, IsItemContextActive 0x84dcc0,
// HasActiveContexts 0x859a70 / 0x816d10, CancelActiveContexts 0x859af0 / 0x816d90 (the 2013 body takes the flag it hands every
// context's CancelContext, slot 74 - what FDisSoireeControlState::EnterSoireeMode 0xa66ff0 calls on both equipped items),
// PlayerTransitionItemIn_Derived 0x858660 / 0x812110, AnimState_GetSkeletalComponent 0x84dce0, AnimState_GetAnimStateComp 0x84dd30,
// StopZoom 0x856370 (the zoomed half is not ported - no item zooms on this route), SetTweaks_Derived / GetTweaks_Derived are
// the concrete items' (UDishonoredWepSword 0x889920 / 0x886e00, UDishonoredItemEmpty 0x88b2c0 / 0x7cc530, UDisItemPowers
// 0x88b250 / 0x887b70): the base interface answers NULL and the class default tweaks stand in, as retail's
// IDisTweaksInterface::GetTweaks<UDisTweaks_InventoryItem> does. ItemIsPhysicallyEquipped is slot 76 in 2012, folded onto a
// `return TRUE` body (UDisItemPowers overrides it). Bodies in dishonoredinventoryitem.cpp.
public:
	// the interface glue retail has in every item vtable (GetUObjectInterfaceDisEngineTweaksInterface 2012 0x861660): without it
	// IDisTweaksInterface::ApplyTweakChanges finds no object and never reaches ApplyTweakChanges_Derived (the item meshes stay empty)
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	virtual UObject* GetUObjectInterfaceDisEngineTweaksInterface() { return this; }
	class UDisTweaks_InventoryItem* GetItemTweaks() const;
	virtual void InitItem( class UDisTweaks_InventoryItem* _pItemTweaks );
	virtual void OnAddToInventory( class UDishonoredInventory* _pInventory );
	virtual void OnAttached();
	virtual void OnDetached();
	virtual UBOOL ItemIsPhysicallyEquipped() const { return TRUE; }
	void OnEquipItem( BYTE _EquipUsage, UBOOL _bTemporary );
	void OnUnEquipItem();
	UBOOL IsEquippable( BYTE _EquipUsage, const class ADishonoredPawn* _pPotentialOwner ) const;
	virtual void ApplyTweakChanges_Derived();
	void TickInventoryItem( FLOAT _fDeltaTime );
	UBOOL ModifyPlayerStance( BYTE& _rStance ) const;
	UBOOL HasActiveContexts() const;
	UBOOL CancelActiveContexts( UBOOL _bForced = FALSE );
	UBOOL IsItemContextActive( const class UDisItemContext* _pContext ) const;
	void StopZoom();
	virtual class USkeletalMeshComponent* AnimState_GetSkeletalComponent();
	virtual class UDisAnimStateComponent* AnimState_GetAnimStateComp();
	virtual BYTE PlayerTransitionItemIn_Derived( BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip );
	void OnDifficultyChange( const class FArkGameEvent& _rEvent );
	class UActorComponent* GetPostUpdateTickComponent() const { return *(class UActorComponent* const*)m_pPostUpdateTickComponent; }
protected:
	void InitItemContextSlot( const class UDisTweaks_InventoryItem_PawnSpecific* _pPawnSpecificTweaks, TArrayNoInit<class UDisItemContext*>& _rSlot, const TArrayNoInit<struct FDisItemContextInfo>& _rInfo, BYTE _SlotEnum );
	TArrayNoInit<class UDisItemContext*>* GetContextsForSlot( BYTE _ContextSlot );
	virtual UBOOL IsEquippable_Derived( const class ADishonoredPawn* _pPotentialOwner ) const { return TRUE; }
	virtual void OnEquipItem_Derived() {}
	virtual void OnUnEquipItem_Derived() {}
	virtual void TickInventoryItem_Derived( FLOAT _fDeltaTime ) {}
	virtual UBOOL ModifyPlayerStance_Derived( BYTE& _rStance ) const { return FALSE; }
	virtual const FVector2D GetItemAimForAnims_Derived() const;
public:
