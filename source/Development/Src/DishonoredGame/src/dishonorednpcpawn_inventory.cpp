// DishonoredGame/src/dishonorednpcpawn_inventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20):
//   0x7ac0b0  public: virtual class FName __thiscall ADishonoredNPCPawn::GetEquippedItemPrefix(enum EDisEquipUsage)const
//   0x7aee10  public: class UDishonoredInventoryItem * __thiscall ADishonoredNPCPawn::GetIntendedItemToEquip(enum EDisEquipUsage)const
//   0x7aee70  public: virtual class FName __thiscall ADishonoredNPCPawn::GetIntendedItemPrefix(enum EDisEquipUsage)const
//   0x7b42b0  public: virtual int __thiscall ADishonoredNPCPawn::CanUseObjectTypes(void)const
//   0x7b4370  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanUseItemContext(class UDisItemContext const *)const
//   0x7b4420  public: virtual unsigned int __thiscall ADishonoredNPCPawn::GrabMovableObject(class UDisMovableComponent *)
//   0x7b4460  public: virtual unsigned int __thiscall ADishonoredNPCPawn::DropMovableObject(unsigned int)
//   0x7b44a0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::HasKey(class FString const &)const
//   0x7b44d0  public: unsigned int __thiscall ADishonoredNPCPawn::IsEquipPending(class UDishonoredInventoryItem *, class UClass * &)const
//   0x7b45c0  public: int __thiscall ADishonoredNPCPawn::CalculateNPCArmorDamage(int, enum eDisPawnHitReactionType, enum eDisHitRegion, class UClass *, class FVector const &)const
//   0x7b47f0  protected: void __thiscall ADishonoredNPCPawn::DropArmorAtIndex(int, unsigned int, class FVector const &, class FVector const &)
//   0x7b4b40  public: virtual void __thiscall ADishonoredNPCPawn::OnPlayMusicBox(class UDisSeqAct_PlayMusicBox *)
//   0x7b4c00  public: virtual void __thiscall ADishonoredNPCPawn::OnEquipItemType(class UDisSeqAct_EquipItemType *)
//   0x7b7950  public: virtual unsigned int __thiscall ADishonoredNPCPawn::AttachEquippedItem(enum EItemSocket, enum EDisEquipUsage)
//   0x7b7b90  public: void __thiscall ADishonoredNPCPawn::UpdateEquipment(void)
//   0x7b7e00  protected: int __thiscall ADishonoredNPCPawn::HandleNPCArmorDamage(int, enum eDisPawnHitReactionType, enum eDisHitRegion, class UClass *, class FVector const &, class FVector const &)
//   0x7b8140  public: void __thiscall ADishonoredNPCPawn::DropEquippedArmor(enum eDisHitRegion, enum eDisPawnHitReactionType, class UClass *, class FVector const &, class FVector const &)
//   0x7b8350  public: void __thiscall ADishonoredNPCPawn::DropEquippedArmor(enum eDisHitRegion, class FVector const &, class FVector const &)
//   0x7bc5a0  public: void __thiscall ADishonoredNPCPawn::CreateNPCArmor(struct TMemStackArray<int> const *)
//   0x7be2c0  public: virtual void __thiscall ADishonoredNPCPawn::Tick_Inventory(float, enum ELevelTick)
