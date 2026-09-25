// DishonoredGame/src/dishonoredweapon_ranged.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (40):
//   0x886a60  public: static void __cdecl UDisTweaks_WeaponRanged::InitializePrivateStaticClassUDisTweaks_WeaponRanged(void)
//   0x886a80  public: unsigned int __thiscall UDishonoredWeapon_Ranged::IsAmmoUnlocked(enum eDisAmmoType)const
//   0x886ae0  public: virtual unsigned int __thiscall UDishonoredWeapon_Ranged::ConsumeAmmo(int)
//   0x886b20  public: int __thiscall UDishonoredWeapon_Ranged::GetMaxLoadedAmmoCount(void)const
//   0x886b40  public: virtual unsigned int __thiscall UDisWepGrenade::ShouldDoOffhandAction(void)
//   0x886b50  public: int __thiscall UDishonoredWeapon_Ranged::GetLoadedAmmoCount(void)const
//   0x886b60  public: int __thiscall UDishonoredWeapon_Ranged::GetNotLoadedAmmoCount(void)const
//   0x886b80  public: virtual float __thiscall UDishonoredWeapon_Ranged::GetDispersion(void)const
//   0x886b90  public: virtual enum EDisCrosshairState __thiscall UDishonoredWeapon_Ranged::GetCrosshairState(void)const
//   0x886bd0  public: virtual void __thiscall UDishonoredWeapon_Ranged::TickInventoryItem_Derived(float)
//   0x886c40  public: virtual void __thiscall UDishonoredWeapon_Ranged::OnFire(void)
//   0x886c60  public: void __thiscall UDishonoredWeapon_Ranged::IncreaseDispersion(float, float)
//   0x889680  public: unsigned int __thiscall UDishonoredWeapon_Ranged::EquipRequiresAmmo(void)const
//   0x88db30  public: void __thiscall UDishonoredWeapon_Ranged::OnWeaponReloaded(void)
//   0x88db80  protected: virtual void __thiscall UDishonoredWeapon_Ranged::OnEquipItem_Derived(void)
//   0x88dbd0  protected: virtual void __thiscall UDishonoredWeapon_Ranged::OnAddAmmo_Derived_Reload(void)
//   0x892350  public: virtual void __thiscall UDishonoredWeapon_Ranged::OnAddToInventory(class UDishonoredInventory *)
//   0x8923b0  protected: virtual struct FVector2D const __thiscall UDishonoredWeapon_Ranged::GetItemAimForAnims_Derived(void)const
//   0x895620  private: virtual void __thiscall UDisTweaks_WeaponRanged::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x89d640  public: static class UClass * __cdecl UDisTweaks_WeaponRanged::GetPrivateStaticClassUDisTweaks_WeaponRanged(wchar_t const *)
//   0x89d6d0  public: static class UClass * __cdecl UDisTweaks_WeaponRanged_Attributes::GetPrivateStaticClassUDisTweaks_WeaponRanged_Attributes(wchar_t const *)
//   0x89f2c0  public: static class UClass * __cdecl UDisTweaks_WeaponRanged::StaticClassNoInline(void)
//   0x89f2f0  public: static void __cdecl UDisTweaks_WeaponRanged_Attributes::InitializePrivateStaticClassUDisTweaks_WeaponRanged_Attributes(void)
//   0x8a0230  public: static class UClass * __cdecl UDisTweaks_WeaponRanged_Attributes::StaticClassNoInline(void)
//   0x8a0900  public: virtual void __thiscall UDishonoredWeapon_Ranged::InitItem(class UDisTweaks_InventoryItem *)
//   0x8a0950  public: virtual void __thiscall UDishonoredWeapon_Ranged::SetCurAmmoType(enum eDisAmmoType, unsigned int)
//   0x8a0ad0  protected: void __thiscall UDishonoredWeapon_Ranged::FindAvailableAmmoType(void)
//   0x8a0c50  protected: enum eDisAmmoType __thiscall UDishonoredWeapon_Ranged::GetNextAvailableAmmoType(void)
//   0x8a0df0  protected: enum eDisAmmoType __thiscall UDishonoredWeapon_Ranged::GetPrevAvailableAmmoType(void)
//   0x8a0f90  public: unsigned int __thiscall UDishonoredWeapon_Ranged::HasAmmo(int, enum eDisAmmoType)const
//   0x8a1060  public: void __thiscall UDishonoredWeapon_Ranged::TickMelee(void)
//   0x8a10b0  public: virtual class FName __thiscall UDishonoredWeapon_Ranged::GetNavAnimState(void)const
//   0x8a53e0  protected: virtual void __thiscall UDishonoredWeapon_Ranged::ApplyTweakChanges_Derived(void)
//   0x8a5410  public: virtual unsigned int __thiscall UDishonoredWeapon_Ranged::NextItemOption(void)
//   0x8a5460  public: virtual unsigned int __thiscall UDishonoredWeapon_Ranged::PrevItemOption(void)
//   0x8a54b0  protected: virtual enum eDisTransitionItemResult __thiscall UDishonoredWeapon_Ranged::PlayerTransitionItemIn_Derived(enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x8a5560  protected: virtual void __thiscall UDishonoredWeapon_Ranged::OnAddAmmo_Derived(enum eDisAmmoType)
//   0x8aca80  public: static class UClass * __cdecl UDishonoredWeapon_Ranged::GetPrivateStaticClassUDishonoredWeapon_Ranged(wchar_t const *)
//   0x8ad8d0  public: static void __cdecl UDishonoredWeapon_Ranged::InitializePrivateStaticClassUDishonoredWeapon_Ranged(void)
//   0x8ada90  public: static class UClass * __cdecl UDishonoredWeapon_Ranged::StaticClassNoInline(void)
