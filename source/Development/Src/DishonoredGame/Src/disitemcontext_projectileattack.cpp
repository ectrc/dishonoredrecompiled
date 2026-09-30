// DishonoredGame/src/disitemcontext_projectileattack.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20):
//   0x862210  public: virtual void __thiscall UDisItemContext_NPCFireGun::InitContext(class UDishonoredInventoryItem *)
//   0x862260  protected: virtual void __thiscall UDisItemContext_ProjectileAttack::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x863df0  protected: virtual void __thiscall UDisItemContext_ProjectileAttack::EndItemContext_Derived(enum eDisItemContextStatus)
//   0x863ea0  public: virtual unsigned int __thiscall UDisItemContext_ProjectileAttack::FireProjectile(void)
//   0x863f20  public: virtual void __thiscall UDisItemContext_ProjectileAttack::OnCameraUpdate(struct FDishonoredViewTarget const &)
//   0x863f80  protected: struct FVector2D __thiscall UDisItemContext_ProjectileAttack::GetDispersionOffset(void)const
//   0x867770  protected: struct FDisAimAssistGroup const & __thiscall UDisItemContext_ProjectileAttack::GetAimAssistSettings(unsigned int)const
//   0x8677c0  void __cdecl GetAimAssistPos(class AActor const *, enum eDisHitRegion, class FVector const &, class FVector const &, class FVector &)
//   0x867840  protected: void __thiscall UDisItemContext_ProjectileAttack::ApplyRecoil(void)
//   0x86d9b0  public: static class UClass * __cdecl UDisItemContext_ProjectileAttack::GetPrivateStaticClassUDisItemContext_ProjectileAttack(wchar_t const *)
//   0x86da40  protected: virtual void __thiscall UDisTweaks_ProjectileAttack::ApplyFallbackChain_Derived(void)
//   0x875190  public: static void __cdecl UDisItemContext_ProjectileAttack::InitializePrivateStaticClassUDisItemContext_ProjectileAttack(void)
//   0x8751b0  protected: void __thiscall UDisItemContext_ProjectileAttack::ConditionalKillCam(class ADisProjectile *, class ADishonoredPlayerPawn *, class ADishonoredNPCPawn *, struct FDisLineProbeResult const &)const
//   0x875620  protected: unsigned int __thiscall UDisItemContext_ProjectileAttack::GetCurAimAssist(struct UDisItemContext_AimAssistAttack::FDisBestAimAssist &, struct TMemStackArray<struct UDisItemContext_AimAssistAttack::FDisAimAssistDebug> *)const
//   0x8757f0  public: unsigned int __thiscall UDisItemContext_ProjectileAttack::GetCurAimAssistPos(class FVector &, class FVector &, struct FVector2D &, unsigned int &, struct TMemStackArray<struct UDisItemContext_AimAssistAttack::FDisAimAssistDebug> *)const
//   0x8759d0  public: class FVector const __thiscall UDisItemContext_ProjectileAttack::GetProjectileFireDir(struct FDishonoredViewTarget const &, class FVector &, struct FVector2D &)const
//   0x87c120  public: static class UClass * __cdecl UDisItemContext_ProjectileAttack::StaticClassNoInline(void)
//   0x8809b0  public: static class UClass * __cdecl UDisTweaks_ProjectileAttack::GetPrivateStaticClassUDisTweaks_ProjectileAttack(wchar_t const *)
//   0x884d70  public: static void __cdecl UDisTweaks_ProjectileAttack::InitializePrivateStaticClassUDisTweaks_ProjectileAttack(void)
//   0x8855d0  public: static class UClass * __cdecl UDisTweaks_ProjectileAttack::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(port): agent EQ, 2013 rva 0x7ffc10 (2012 0x862260) - the base's five writes spelled out again plus
// the kill-cam mode. Retail does not chain to the base: it is a separate 97-byte body with the same five stores
// and one more, so it is written the same way here.
void UDisItemContext_ProjectileAttack::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	m_bGamepadUseAutoAimSettings = Parameters->m_bGamepadAutoAim ? TRUE : FALSE;
	m_fGamepadAutoAimStrengthSettings = (FLOAT)Parameters->m_GamepadAutoAimStrength * 0.01f;
	m_bMouseUseAutoAimSettings = Parameters->m_bMouseAutoAim ? TRUE : FALSE;
	m_fMouseAutoAimStrengthSettings = (FLOAT)Parameters->m_MouseAutoAimStrength * 0.01f;
	m_bAutoUseManaElixirSettings = Parameters->m_bAutoUseManaElixir ? TRUE : FALSE;
	m_KillCamSettings = (BYTE)Parameters->m_KillCamMode;
}
