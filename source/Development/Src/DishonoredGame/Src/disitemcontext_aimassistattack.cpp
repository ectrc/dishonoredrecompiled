// DishonoredGame/src/disitemcontext_aimassistattack.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x862000  public: virtual void __thiscall UDisItemContext_AimAssistAttack::InitContext(class UDishonoredInventoryItem *)
//   0x862040  protected: virtual void __thiscall UDisItemContext_AimAssistAttack::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x8630b0  unsigned int __cdecl GetProjectedBoundsSize(struct FBoxSphereBounds const &, class FVector const &, class FVector const &, class FMatrix const &, struct FVector2D &, struct FVector2D &)
//   0x8633f0  protected: virtual unsigned int __thiscall UDisItemContext_AimAssistAttack::GetAimVectorForTarget_Derived(class AActor const *, enum eDisHitRegion, class FVector const &, class FVector const &, class FVector &, class FVector &, struct FDisPlayerViewTracking_ExtraParams &)const
//   0x863550  void __cdecl CalcCameraViewProjMat(class ADishonoredPlayerController const *, class FMatrix &)
//   0x865e70  protected: unsigned int __thiscall UDisItemContext_AimAssistAttack::GetAimVectorForTarget(class AActor const *, enum eDisHitRegion, class FVector const &, class FVector const &, class FMatrix const &, class FVector &, class FVector &, struct FVector2D &, struct FVector2D &, struct FDisPlayerViewTracking_ExtraParams &)const
//   0x8661d0  private: struct FDisAimAssistOther const * __thiscall UDisItemContext_AimAssistAttack::FindAimAssist(struct FDisAimAssistGroup const &, class AActor const *)const
//   0x8662e0  protected: unsigned int __thiscall UDisItemContext_AimAssistAttack::ShouldUseHomingProjectile(struct FDisAimAssistGroup const &, class AActor const *)const
//   0x86d6a0  public: static class UClass * __cdecl UDisItemContext_AimAssistAttack::GetPrivateStaticClassUDisItemContext_AimAssistAttack(wchar_t const *)
//   0x870500  public: static void __cdecl UDisItemContext_AimAssistAttack::InitializePrivateStaticClassUDisItemContext_AimAssistAttack(void)
//   0x870520  protected: void __thiscall UDisItemContext_AimAssistAttack::CheckActorForAssist(class AActor const *, enum eDisHitRegion, class FVector const &, class FVector const &, class FMatrix const &, struct FDisAimAssistInfo const &, struct FDisAimAssistChoiceInfo const &, float, struct UDisItemContext_AimAssistAttack::FDisBestAimAssist &, struct TMemStackArray<struct UDisItemContext_AimAssistAttack::FDisAimAssistDebug> *)const
//   0x874830  public: static class UClass * __cdecl UDisItemContext_AimAssistAttack::StaticClassNoInline(void)
//   0x874860  protected: void __thiscall UDisItemContext_AimAssistAttack::ChooseBestAimAssistTarget(struct UDisItemContext_AimAssistAttack::FDisBestAimAssist &, struct FDisAimAssistGroup const &, struct FDisAimAssistChoiceInfo const &, struct TMemStackArray<struct UDisItemContext_AimAssistAttack::FDisAimAssistDebug> *)const

#include "DishonoredGame.h"

// DISHONORED(port): agent EQ, 2013 rva 0x7ff710 (2012 0x862040). The aim-assist context keeps its own copy of
// the five settings the attack code reads, in retail's own order: the pad's auto-aim and its strength, then the
// mouse's, then the elixir flag. The two strengths are the profile's 0..100 integers divided by a hundred.
// m_bUseAutoAimSettings and m_fAutoAimStrengthSettings are NOT written here - they are the resolved pair the
// attack picks per input device.
void UDisItemContext_AimAssistAttack::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	m_bGamepadUseAutoAimSettings = Parameters->m_bGamepadAutoAim ? TRUE : FALSE;
	m_fGamepadAutoAimStrengthSettings = (FLOAT)Parameters->m_GamepadAutoAimStrength * 0.01f;
	m_bMouseUseAutoAimSettings = Parameters->m_bMouseAutoAim ? TRUE : FALSE;
	m_fMouseAutoAimStrengthSettings = (FLOAT)Parameters->m_MouseAutoAimStrength * 0.01f;
	m_bAutoUseManaElixirSettings = Parameters->m_bAutoUseManaElixir ? TRUE : FALSE;
}
