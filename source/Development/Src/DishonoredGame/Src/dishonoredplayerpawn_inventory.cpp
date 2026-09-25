// DishonoredGame/src/dishonoredplayerpawn_inventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x6fb600  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::ShouldEquipNewPickup_Derived(enum EDisEquipUsage)
//   0x6fb650  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::AddInventoryItem(class ADishonoredInventoryPickup *)
//   0x6fb6e0  public: void __thiscall ADishonoredPlayerPawn::EquipNextItem(unsigned int)
//   0x6fb720  public: void __thiscall ADishonoredPlayerPawn::EquipPrevItem(unsigned int)
//   0x6fb760  public: enum eDisTransitionItemResult __thiscall ADishonoredPlayerPawn::TransitionItemIn(enum EDisEquipUsage, enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x6fb7a0  public: virtual void __thiscall ADishonoredPlayerPawn::DropCorpse(unsigned int)
//   0x6fb7d0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DisIsPossessing(void)const
//   0x6feb60  public: virtual void __thiscall ADishonoredPlayerPawn::EquipNextItem(void)
//   0x6feb90  public: virtual void __thiscall ADishonoredPlayerPawn::EquipPrevItem(void)
//   0x6febc0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsCarryingCorpse(void)const
//   0x706710  protected: void __thiscall ADishonoredPlayerPawn::GameSave_Inventory(class FArchive &, enum ESaveLoadLocation)
//   0x7067c0  protected: void __thiscall ADishonoredPlayerPawn::GameLoad_Inventory(class FArchive &, enum ESaveLoadLocation)
//   0x7068b0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::CanUseItemContext(class UDisItemContext const *)const
//   0x706a70  public: virtual int __thiscall ADishonoredPlayerPawn::CanUseObjectTypes(void)const
//   0x706b90  public: struct FPawnAction const * __thiscall ADishonoredPlayerPawn::GetUnequipAction(enum EDisEquipUsage, unsigned int)const
//   0x706cd0  public: unsigned int __thiscall ADishonoredPlayerPawn::UseEquippedItem(enum EDisEquipUsage)
//   0x706d20  public: unsigned int __thiscall ADishonoredPlayerPawn::AlternateUseEquippedItem(enum EDisEquipUsage, unsigned int)
//   0x706d80  public: virtual class UClass * __thiscall ADishonoredPlayerPawn::GetItemClassBeingEquipped(enum EDisEquipUsage)const
//   0x706dd0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DropMovableObject(unsigned int)
//   0x706e10  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::CanDropCorpse(void)const
//   0x706e60  public: virtual class IDisCorpseInterface * __thiscall ADishonoredPlayerPawn::GetCarriedCorpseOrBodyPart(void)const
//   0x70ce90  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::EquipItemByType(class UClass *, enum EDisEquipUsage, enum ADishonoredPawn::EEquipItemFlags)
//   0x70d140  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::GrabMovableObject(class UDisMovableComponent *)
//   0x70d2d0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::GrabCorpse(class ADishonoredNPCPawn *)
//   0x70d370  public: virtual void __thiscall ADishonoredPlayerPawn::UseKey(class FString const &)
//   0x70fef0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::HasKey(class FString const &)const
//   0x7128d0  public: void __thiscall ADishonoredPlayerPawn::RestoreUpgradesFromBackup(void)
//   0x716e10  public: void __thiscall ADishonoredPlayerPawn::BackupAndClearUpgrades(void)
//   0x718360  public: virtual void __thiscall ADishonoredPlayerPawn::PostBeginPlay_Inventory(void)
