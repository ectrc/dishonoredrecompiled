// DishonoredGame/src/dishonoredinventorypickup.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20):
//   0x861ce0  public: static void __cdecl ADishonoredInventoryPickup::InitializePrivateStaticClassADishonoredInventoryPickup(void)
//   0x861d00  public: static void __cdecl UDisTweaks_InventoryPickup::InitializePrivateStaticClassUDisTweaks_InventoryPickup(void)
//   0x861d20  public: void __thiscall ADishonoredInventoryPickup::ClearInventoryItem(void)
//   0x861d30  public: virtual void __thiscall ADishonoredInventoryPickup::PostBeginPlay(void)
//   0x861d40  public: void __thiscall ADishonoredInventoryPickup::SetInventoryItem(class UDishonoredInventoryItem *)
//   0x861d50  public: void __thiscall ADishonoredInventoryPickup::ApplyStatChanges(class ADishonoredPlayerPawn *)
//   0x861d60  protected: virtual class UDisTweaksBase * __thiscall ADishonoredInventoryPickup::GetTweaks_Derived(void)
//   0x862fb0  protected: virtual void __thiscall ADishonoredInventoryPickup::SetTweaks_Derived(class UDisTweaksBase *)
//   0x87d4f0  public: static class UClass * __cdecl ADishonoredInventoryPickup::GetPrivateStaticClassADishonoredInventoryPickup(wchar_t const *)
//   0x87fcd0  public: static class UClass * __cdecl ADishonoredInventoryPickup::StaticClassNoInline(void)
//   0x880600  public: static class UClass * __cdecl UDisTweaks_InventoryPickup::GetPrivateStaticClassUDisTweaks_InventoryPickup(wchar_t const *)
//   0x8828b0  public: static class UClass * __cdecl UDisTweaks_InventoryPickup::StaticClassNoInline(void)
//   0x883110  public: class UDishonoredInventoryItem * __thiscall ADishonoredInventoryPickup::GetInventoryItem(void)
//   0x8831d0  private: unsigned int __thiscall ADishonoredInventoryPickup::CanPickupItem(class ADishonoredPlayerPawn *)const
//   0x883290  protected: virtual unsigned int __thiscall ADishonoredInventoryPickup::CanBePickedUp(class ADishonoredPlayerPawn *)const
//   0x8832f0  protected: virtual unsigned int __thiscall ADishonoredInventoryPickup::CanInteract(struct FCanInteractParams const &)const
//   0x883340  public: virtual unsigned int __thiscall ADishonoredInventoryPickup::DoInteract_Impl(class ADishonoredPlayerPawn *)
//   0x8833d0  private: virtual void __thiscall ADishonoredInventoryPickup::OnThrow(class FVector const &, class FVector const &)
//   0x883440  public: virtual void __thiscall ADishonoredInventoryPickup::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x883690  protected: virtual int __thiscall ADishonoredInventoryPickup::GetMaxDamage(void)const
// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): ADishonoredInventoryPickup's IDisTweaksInterface slots. Both slots are identical-COMDAT folded in the 2013 exe.
UDisTweaksBase* ADishonoredInventoryPickup::GetTweaks_Derived()
{
	return m_pInvPickupTweaks;
}

void ADishonoredInventoryPickup::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pInvPickupTweaks = (UDisTweaks_InventoryPickup*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x810560 (2012 0x883340): the stat half first (ammo, health, mana), then the inventory
// item itself if the player may carry it.
// DISHONORED(bringup): ADishonoredInventoryPickup::CanPickupItem and ADishonoredPlayerPawn's "take this inventory
// pickup" slot (retail vtable +1492) are not ported - they need UDishonoredInventory's slot / equip half and
// UDisItemContext (agentAJ.md) - so only the stat half of a weapon or gadget pickup is given.
UBOOL ADishonoredInventoryPickup::DoInteract_Impl( ADishonoredPlayerPawn* PlayerPawn )
{
	return ADisStatPickup::DoInteract_Impl( PlayerPawn );
}
