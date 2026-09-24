// DishonoredGame/src/dishonoredinventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (60):
//   0x849b70  public: static void __cdecl UDishonoredInventory::InitializePrivateStaticClassUDishonoredInventory(void)
//   0x849b90  public: void __thiscall UDishonoredInventory::DropEquippedItem(enum EDisEquipUsage, class FVector const &, class FVector const &)
//   0x851d60  public: void __thiscall UDishonoredInventory::AddInventorySlot(class UClass *, enum EDisEquipUsage)
//   0x851db0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::GetEquippedItem(enum EDisEquipUsage)const
//   0x851e10  public: enum eDisTransitionItemResult __thiscall UDishonoredInventory::PlayerTransitionItemIn(enum EDisEquipUsage, enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x851e60  protected: int __thiscall UDishonoredInventory::GetRequiredSlot(class UClass *, unsigned int, enum EDisEquipUsage, struct FPawnInventorySlot * *)
//   0x851f10  protected: int __thiscall UDishonoredInventory::GetNextEmptySlot(int, class UDishonoredInventoryItem *)
//   0x852060  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindEquippableItem(class UClass const * const, enum EDisEquipUsage)const
//   0x852180  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItemByClass(class UClass const * const, unsigned int, unsigned int, unsigned int, enum EDisEquipUsage)const
//   0x8522d0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItemBySelectionType(enum eDisUISelectionType)const
//   0x852340  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItem(class UDisTweaks_InventoryItem const *)const
//   0x8523f0  public: void __thiscall UDishonoredInventory::AddElixir(enum EElixirType, int)
//   0x8524a0  public: void __thiscall UDishonoredInventory::SetElixirCount(enum EElixirType, int)
//   0x8525a0  public: unsigned int __thiscall UDishonoredInventory::ConsumeElixir(enum EElixirType)
//   0x8527c0  public: unsigned int __thiscall UDishonoredInventory::HasAmmo(enum eDisAmmoType, int)const
//   0x852820  public: unsigned int __thiscall UDishonoredInventory::ConsumeAmmo(enum eDisAmmoType, unsigned int)
//   0x852880  public: int __thiscall UDishonoredInventory::AddAmmo(enum eDisAmmoType, unsigned int)
//   0x852920  public: void __thiscall UDishonoredInventory::SetAmmo(enum eDisAmmoType, unsigned int, unsigned int)
//   0x852990  public: struct FDisAmmoInfo const * __thiscall UDishonoredInventory::GetAmmoInfo(enum eDisAmmoType)const
//   0x8529e0  public: unsigned int __thiscall UDishonoredInventory::ConsumeStatPickup(class ADisStatPickup *, int *)
//   0x852a90  public: void __thiscall UDishonoredInventory::SetReequipItem(enum EDisEquipUsage, class UClass *)
//   0x852b70  public: class UClass * __thiscall UDishonoredInventory::GetReequipItem(enum EDisEquipUsage, unsigned int)const
//   0x852bf0  public: void __thiscall UDishonoredInventory::RegisterActiveContext(class UDisItemContext *)
//   0x852c10  public: virtual unsigned int __thiscall UDishonoredInventory::FillUIInteractions(struct FDisUIInteractionContext &)const
//   0x8559f0  public: int __thiscall UDishonoredInventory::GetRequiredSlot(class UDishonoredInventoryItem *)
//   0x855a50  public: unsigned int __thiscall UDishonoredInventory::AttachItemToSocket(class UDishonoredInventoryItem * const, enum EItemSocket)
//   0x855bc0  public: unsigned int __thiscall UDishonoredInventory::HasItemByClass(class UClass const *, unsigned int)const
//   0x855be0  public: unsigned int __thiscall UDishonoredInventory::HasItem(class UDisTweaks_InventoryItem const *)const
//   0x855c00  public: void __thiscall UDishonoredInventory::DamageInventoryItems(int &, class AController *, class FVector, class FVector &, class UClass * &, struct FTraceHitInfo, class AActor *)
//   0x855ce0  public: void __thiscall UDishonoredInventory::AddAbstractItem(class UDisAbstractItem *, int, unsigned int)
//   0x855d70  public: void __thiscall UDishonoredInventory::HideAbstractItem(class UDisAbstractItem *)
//   0x855db0  public: int __thiscall UDishonoredInventory::GetAbstractItemQuantity(class UDisAbstractItem *)const
//   0x855df0  protected: unsigned int __thiscall UDishonoredInventory::CanEquipItemType(class UClass *, enum EDisEquipUsage, int &)
//   0x857f00  public: unsigned int __thiscall UDishonoredInventory::CanEquipItemType(class UClass *, enum EDisEquipUsage, class UDisTweaks_InventoryItem const * *)
//   0x857fb0  protected: unsigned int __thiscall UDishonoredInventory::ShowInventoryItem(class UDishonoredInventoryItem *, unsigned int, enum EDisEquipUsage)
//   0x858040  public: unsigned int __thiscall UDishonoredInventory::ShowEquippedItem(unsigned int, enum EDisEquipUsage)
//   0x858080  public: void __thiscall UDishonoredInventory::RemoveAbstractItem(class UDisAbstractItem *, int)
//   0x859440  protected: void __thiscall UDishonoredInventory::BackupIntoLoadout(struct FDisInventoryLoadout &)
//   0x8596d0  public: virtual void __thiscall UDishonoredInventory::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8598f0  public: virtual void __thiscall UDishonoredInventory::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x85a580  public: static class UClass * __cdecl UDishonoredInventory::GetPrivateStaticClassUDishonoredInventory(wchar_t const *)
//   0x85a610  public: void __thiscall UDishonoredInventory::TerminateInventory(void)
//   0x85a730  public: void __thiscall UDishonoredInventory::OnCameraUpdate(struct FDishonoredViewTarget const &)
//   0x85a7b0  protected: void __thiscall UDishonoredInventory::DoEquip(int, enum EDisEquipUsage, unsigned int)
//   0x85a940  public: unsigned int __thiscall UDishonoredInventory::EquipItemType(class UClass *, enum EDisEquipUsage, unsigned int)
//   0x85a9b0  public: void __thiscall UDishonoredInventory::RemoveInventoryItem(class UDishonoredInventoryItem *, unsigned int)
//   0x85aab0  public: void __thiscall UDishonoredInventory::RemoveItemsByClass(class UClass const *, unsigned int)
//   0x85b760  public: static class UClass * __cdecl UDishonoredInventory::StaticClassNoInline(void)
//   0x85b790  public: void __thiscall UDishonoredInventory::InitInventory(class ADishonoredPawn *, class UDisTweaks_InventoryItem *)
//   0x85b930  public: void __thiscall UDishonoredInventory::GiveAllItems(class ADishonoredPawn *)
//   0x85bb60  protected: void __thiscall UDishonoredInventory::DropItemsInSlot(struct FPawnInventorySlot &, class FVector const &, class FVector const &)
//   0x85bc60  public: class UClass * __thiscall UDishonoredInventory::EquipNextItem(unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85bd60  public: class UClass * __thiscall UDishonoredInventory::EquipPrevItem(unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85be60  public: unsigned int __thiscall UDishonoredInventory::AddInventoryItem(class UDishonoredInventoryItem * const, unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85c760  public: void __thiscall UDishonoredInventory::TickInventory(float)
//   0x85c940  public: void __thiscall UDishonoredInventory::DropAttachedItems(class FVector const &, class FVector const &)
//   0x85c9c0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::SpawnAndAddInventoryItem(class UDisTweaks_InventoryItem * const, unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85ca60  public: void __thiscall UDishonoredInventory::RestoreFromBackup(void)
//   0x861a90  public: void __thiscall UDishonoredInventory::ClearInventory(void)
//   0x861c70  public: void __thiscall UDishonoredInventory::BackupAndClear(void)
