// DishonoredGame/src/dishonoredplayerpawn_combat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x6fb1c0  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsVulnerable_Derived(enum eDisVulnerabilityType, unsigned int)const
//   0x6fb200  public: void __thiscall ADishonoredPlayerPawn::SetPlayerStance(enum EDisEquipUsage, enum eDisPlayerStance)
//   0x6fb220  public: enum eDisPlayerStance __thiscall ADishonoredPlayerPawn::GetActualPlayerStance(enum EDisEquipUsage)const
//   0x6fb250  public: enum eDisPlayerStance __thiscall ADishonoredPlayerPawn::GetDesiredPlayerStance(enum EDisEquipUsage)const
//   0x6fb2f0  public: void __thiscall ADishonoredPlayerPawn::ClearAdrenaline(void)
//   0x6fb310  public: unsigned int __thiscall ADishonoredPlayerPawn::InSlomoRange(void)const
//   0x6fb330  public: void __thiscall ADishonoredPlayerPawn::EnterSlomoRange(float)
//   0x6fb3d0  public: void __thiscall ADishonoredPlayerPawn::ExitSlomoRange(void)
//   0x6fb440  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsInCombat(void)const
//   0x6fb450  public: void __thiscall ADishonoredPlayerPawn::ApplyExternalMovement(class FVector const &)
//   0x6fe9a0  public: void __thiscall ADishonoredPlayerPawn::SetExtraVulnerability(float, float)
//   0x6fea10  public: virtual void __thiscall ADishonoredPlayerPawn::GetMeleeCombatInfo(struct FDisMeleeInfo &, class UDisItemContext const *)const
//   0x6fea40  public: unsigned int __thiscall ADishonoredPlayerPawn::FindCombatTarget_Dir(class UDisItemContext const *, struct FDisLineProbeResult &, class FVector const &, class FVector const &, int, int, int, class FVector, unsigned int, class TArray<class AActor const *, class FDefaultAllocator> const *)const
//   0x6feaa0  public: unsigned int __thiscall ADishonoredPlayerPawn::FindCombatTargetMulti_Dir(class UDisItemContext const *, struct TMemStackArray<struct FDisLineProbeResult> &, class FVector const &, class FVector const &, int, int, int, class FVector, unsigned int, class TArray<class AActor const *, class FDefaultAllocator> const *)const
//   0x6feb00  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsFalling(void)const
//   0x705820  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::AnimState_PlayAnim_Derived(class UAnimNodeSequence *)
//   0x7058f0  public: float __thiscall ADishonoredPlayerPawn::CalcRayScale(void)const
//   0x7059c0  public: virtual void __thiscall ADishonoredPlayerPawn::OnMeleeOutgoing_AttackZone(class UDisItemContext_MeleeAttack *)
//   0x705a40  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsLinkedActionLegal(void)const
//   0x705a70  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsStunned(unsigned int &)const
//   0x705ab0  public: unsigned int __thiscall ADishonoredPlayerPawn::BlockEquippedItem(enum EDisEquipUsage, unsigned int)
//   0x705ba0  public: virtual void __thiscall ADishonoredPlayerPawn::DoWeakHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class UClass *)
//   0x705bf0  public: virtual void __thiscall ADishonoredPlayerPawn::OnMeleeStateChanged(class UDisItemContext_MeleeAttack *, enum eDisWeaponMeleeState)
//   0x705c20  public: virtual void __thiscall ADishonoredPlayerPawn::OnMeleeCombatTargetChanged(class ADishonoredPawn *)
//   0x705c50  public: void __thiscall ADishonoredPlayerPawn::ResetFocusEffectTimer(void)
//   0x705d00  protected: void __thiscall ADishonoredPlayerPawn::ProcessHealthEffects(float, float)
//   0x705e30  public: void __thiscall ADishonoredPlayerPawn::ApplyHealthEffectsPost(struct FArkPpConfig &)
//   0x705f20  protected: virtual enum eDisPawnHitReactionType __thiscall ADishonoredPlayerPawn::DetermineHitReactionType_Derived(class UClass *, int, class FVector const &, struct FTraceHitInfo const *, class AActor *, class ADishonoredPawn *)const
//   0x706050  public: void __thiscall FDisPlayerHealthEffect::OnEffectEnabled(class ADishonoredPlayerPawn *)
//   0x706270  public: void __thiscall FDisPlayerHealthEffect::OnEffectDisabled(class ADishonoredPlayerPawn *)
//   0x706350  public: virtual void __thiscall ADishonoredPlayerPawn::Choke(void)
//   0x70be70  protected: void __thiscall ADishonoredPlayerPawn::Tick_Combat_Vulnerable(float)
//   0x70bfb0  public: void __thiscall ADishonoredPlayerPawn::AddAdrenaline(float, unsigned int)
//   0x70c0b0  public: unsigned int __thiscall ADishonoredPlayerPawn::IsAdrenalineFull(void)const
//   0x70c110  protected: void __thiscall ADishonoredPlayerPawn::Tick_Combat_FocusEffect(float)
//   0x70c230  public: virtual void __thiscall ADishonoredPlayerPawn::OnStunned(float)
//   0x70c320  public: virtual void __thiscall ADishonoredPlayerPawn::DoStrongHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x70c5e0  public: virtual void __thiscall ADishonoredPlayerPawn::DoKnockdownHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x70caa0  protected: void __thiscall ADishonoredPlayerPawn::Tick_Combat_HealthEffects(float)
//   0x70ce60  public: void __thiscall ADishonoredPlayerPawn::OnRatKill(void)
//   0x70fdd0  public: void __thiscall ADishonoredPlayerPawn::Tick_Combat_Adrenaline(float)
//   0x712610  protected: void __thiscall ADishonoredPlayerPawn::RefreshCombatStatus(void)
//   0x716c80  public: virtual void __thiscall ADishonoredPlayerPawn::Tick_Combat(float, enum ELevelTick)
//   0x71a710  public: virtual void __thiscall ADishonoredPlayerPawn::PreBeginPlay_PlayerCombat(void)

// agentDO:healthpp
#include "DishonoredGame.h"
#include "arkpp.h"

/**
 * DISHONORED(port): 2013 rva 0x6ac910 (2012 0x705e30) - the player's health effects reach the colour grade here. An
 * entry contributes only while m_bApplyPostProcess is set and its blended weight is above 1e-8; the low-health red
 * wash is one of these.
 */
void ADishonoredPlayerPawn::ApplyHealthEffectsPost( FArkPpConfig& Config )
{
	for( INT EffectIndex = 0; EffectIndex < m_HealthEffects.Num(); EffectIndex++ )
	{
		const FDisPlayerHealthEffect& Effect = m_HealthEffects(EffectIndex);
		if( !Effect.m_bApplyPostProcess || Effect.m_fCurWeight <= 1e-8f )
		{
			continue;
		}
		if( Config.m_bOverrideUberPpParameters )
		{
			ArkUberPpSetDefaultOnNoOverride( Config.m_UberPpParameters );
		}
		else
		{
			ArkUberPpForceDefault( Config.m_UberPpParameters );
		}
		if( Effect.m_ArkPpSettings.m_bOverrideUberPpParameters )
		{
			ArkUberPpApplyTo( Effect.m_ArkPpSettings.m_UberPpParameters, Config.m_UberPpParameters, Effect.m_fCurWeight, FALSE );
			Config.m_bOverrideUberPpParameters = TRUE;
		}
		if( Effect.m_ArkPpSettings.m_bOverrideBloomPpParameters )
		{
			Config.m_bOverrideBloomPpParameters = TRUE;
		}
	}
}
