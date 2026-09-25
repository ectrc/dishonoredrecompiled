// DishonoredGame/src/dishonoredpawn_inventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x7aab20  public: class UDishonoredInventoryItem * __thiscall ADishonoredPawn::GetEquippedItem(enum EDisEquipUsage)const
//   0x7aab30  public: unsigned int __thiscall ADishonoredPawn::PrimaryHandAppearEmpty(void)const
//   0x7aab60  public: void __thiscall ADishonoredPawn::ShowEquippedInventoryItem(unsigned int, enum EDisEquipUsage)
//   0x7aab80  public: virtual void __thiscall ADishonoredPawn::EquipNextItem(void)
//   0x7aaba0  public: virtual void __thiscall ADishonoredPawn::EquipPrevItem(void)
//   0x7aabc0  public: virtual unsigned int __thiscall ADishonoredPawn::DirectUseEquippedItem(enum EDisEquipUsage, class UClass *, struct FDisItemContextParams const *)
//   0x7aac00  public: virtual void __thiscall ADishonoredPawn::DropEquippedItem(enum EDisEquipUsage, class FVector const &, class FVector const &)
//   0x7aac20  public: virtual unsigned int __thiscall ADishonoredPawn::AttachEquippedItem(enum EItemSocket, enum EDisEquipUsage)
//   0x7aac60  public: unsigned int __thiscall ADishonoredPawn::IsPossessing(void)const
//   0x7aac70  public: virtual void __thiscall ADishonoredPawn::OnAddAbstractItem(class UDisSeqAct_AddAbstractItem *)
//   0x7aaca0  public: virtual void __thiscall ADishonoredPawn::OnRemoveAbstractItem(class UDisSeqAct_RemoveAbstractItem *)
//   0x7aace0  public: virtual void __thiscall ADishonoredPawn::OnGetAbstractItemQuantity(class UDisSeqAct_GetAbstractItemQuantity *)
//   0x7aad20  public: virtual void __thiscall ADishonoredPawn::OnModifyElixirCount(class UDisSeqAct_ModifyElixirCount *)
//   0x7acbb0  public: unsigned int __thiscall ADishonoredPawn::HasNothingEquipped(void)const
//   0x7acc50  public: unsigned int __thiscall ADishonoredPawn::HandsAppearEmpty(void)const
//   0x7acca0  public: virtual unsigned int __thiscall ADishonoredPawn::ReloadEquippedItem(enum EDisEquipUsage, enum eDisAmmoType)
//   0x7afac0  public: virtual void __thiscall ADishonoredPawn::Tick_Inventory(float, enum ELevelTick)
//   0x7afbe0  public: unsigned int __thiscall ADishonoredPawn::EquipItemByType_KeepTrying(class UClass *, enum EDisEquipUsage, enum ADishonoredPawn::EEquipItemFlags)
//   0x7afd10  public: virtual unsigned int __thiscall ADishonoredPawn::AddInventoryItem(class ADishonoredInventoryPickup *)
//   0x7afe30  public: virtual unsigned int __thiscall ADishonoredPawn::IsHoldingMovableObject(void)const
//   0x7afe60  public: virtual class UDisMovableComponent * __thiscall ADishonoredPawn::GetHeldMovableObject(void)
//   0x7afe90  public: void __thiscall ADishonoredPawn::SpawnInventoryLoadout_Ammo(class TArray<struct FDisInventoryAmmoEntry, class FDefaultAllocator> const &, unsigned int)
//   0x7aff20  public: virtual void __thiscall ADishonoredPawn::OnAddInventoryItem(class UDisSeqAct_AddInventoryItem *)
//   0x7b0050  public: virtual void __thiscall ADishonoredPawn::OnEquipItemType(class UDisSeqAct_EquipItemType *)
//   0x7b0100  public: virtual void __thiscall ADishonoredPawn::OnModifyAmmo(class UDisSeqAct_ModifyAmmo *)
//   0x7c38a0  public: virtual void __thiscall ADishonoredPawn::PreBeginPlay_Inventory(void)
//   0x7c8500  public: virtual unsigned int __thiscall ADishonoredPawn::EquipItemByType(class UClass *, enum EDisEquipUsage, enum ADishonoredPawn::EEquipItemFlags)
//   0x7c8690  public: void __thiscall ADishonoredPawn::SpawnInventoryLoadout_Items(class TArray<class UDisTweaks_InventoryItem *, class FDefaultAllocator> const &)
//   0x7c8bb0  public: void __thiscall ADishonoredPawn::SpawnInventoryLoadout(struct FDisInventoryLoadout const &, unsigned int)
