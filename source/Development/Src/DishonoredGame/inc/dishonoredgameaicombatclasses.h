#pragma once
// DishonoredGame/inc/dishonoredgameaicombatclasses.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (136):
//   0x63c040  public: static class UClass * __cdecl UDisBehaviorCombat::StaticClass(void)
//   0x63c060  public: void __thiscall UDisBehaviorCombatMelee::execOnEnterCallback_Stand(struct FFrame &, void * const)
//   0x63c0f0  public: void __thiscall UDisBehaviorCombatMelee::execRefreshCallback_Stand(struct FFrame &, void * const)
//   0x63c180  public: void __thiscall UDisBehaviorCombatMelee::execRequestStateExitCallback_FindShootingPosition(struct FFrame &, void * const)
//   0x63c1e0  public: void __thiscall UDisBehaviorAssassinCombat::execRequestStateExitCallback_DoAttractSpell(struct FFrame &, void * const)
//   0x63c240  public: void __thiscall UDisBehaviorCombatEliteGuard::execRefreshCallback_FindShootingPosition(struct FFrame &, void * const)
//   0x63c2d0  public: void __thiscall UDisBehaviorCombatEliteGuard::execOnEnterCallback_FirePistol(struct FFrame &, void * const)
//   0x63c360  public: void __thiscall UDisBehaviorCombatEliteGuard::execRequestStateExitCallback_FirePistol(struct FFrame &, void * const)
//   0x63c3c0  public: void __thiscall UDisBehaviorOverseerCombat::execOnEnterCallback_GenericAction(struct FFrame &, void * const)
//   0x63c450  public: void __thiscall UDisBehaviorOverseerCombat::execOnExitCallback_Stand(struct FFrame &, void * const)
//   0x63c4e0  public: void __thiscall UDisBehaviorCombatWolfhound::execRefreshCallback_MeleeChase(struct FFrame &, void * const)
//   0x63c5d0  public: void __thiscall UDisBehaviorTriggerAlarm::execRequestStateExitCallback_GenericAction(struct FFrame &, void * const)
//   0x63df30  public: void __thiscall UDisGFxMoviePlayerMenuBase::execOnLoadGameConfirm(struct FFrame &, void * const)
//   0x734690  protected: virtual __thiscall UDisBehaviorAmbush::~UDisBehaviorAmbush(void)
//   0x7347d0  protected: virtual __thiscall UDisBehaviorEnemyUnreachable::~UDisBehaviorEnemyUnreachable(void)
//   0x734930  public: virtual class UObject * __thiscall UDisBehaviorShoot::GetUObjectInterfaceDisAIRangedBehaviorInterface(void)
//   0x734970  protected: virtual __thiscall UDisBehaviorShoot::~UDisBehaviorShoot(void)
//   0x735d60  public: static void __cdecl UDisBehaviorAmbush::InternalConstructor(void *)
//   0x735da0  public: static void __cdecl UDisBehaviorEnemyUnreachable::InternalConstructor(void *)
//   0x735dc0  public: static void __cdecl UDisBehaviorShoot::InternalConstructor(void *)
//   0x754af0  protected: virtual __thiscall UDisBehaviorCombat::~UDisBehaviorCombat(void)
//   0x754c40  protected: virtual __thiscall UDisBehaviorCombatMelee::~UDisBehaviorCombatMelee(void)
//   0x754cc0  public: virtual class UObject * __thiscall UDisBehaviorCombatMelee::GetUObjectInterfaceDisAIRangedBehaviorInterface(void)
//   0x754d60  protected: virtual __thiscall UDisBehaviorAssassinCombat::~UDisBehaviorAssassinCombat(void)
//   0x754e40  protected: virtual __thiscall UDisBehaviorCombatCityGuard::~UDisBehaviorCombatCityGuard(void)
//   0x754ef0  protected: virtual __thiscall UDisBehaviorCombatEliteGuard::~UDisBehaviorCombatEliteGuard(void)
//   0x755000  protected: virtual __thiscall UDisBehaviorOverseerHMasterCombat::~UDisBehaviorOverseerHMasterCombat(void)
//   0x7550e0  protected: virtual __thiscall UDisBehaviorOverseerCombat::~UDisBehaviorOverseerCombat(void)
//   0x7551c0  protected: virtual __thiscall UDisBehaviorThugCombat::~UDisBehaviorThugCombat(void)
//   0x755310  protected: virtual __thiscall UDisBehaviorMusicalCombat::~UDisBehaviorMusicalCombat(void)
//   0x755460  protected: virtual __thiscall UDisBehaviorTallBoyCombat::~UDisBehaviorTallBoyCombat(void)
//   0x755510  protected: virtual __thiscall UDisBehaviorCombatRatSwarm::~UDisBehaviorCombatRatSwarm(void)
//   0x7555f0  protected: virtual __thiscall UDisBehaviorCombatRatSwarmEliteGuard::~UDisBehaviorCombatRatSwarmEliteGuard(void)
//   0x7556c0  protected: virtual __thiscall UDisBehaviorTallboyShoot::~UDisBehaviorTallboyShoot(void)
//   0x756690  public: static void __cdecl UDisBehaviorCombat::InternalConstructor(void *)
//   0x7566b0  public: static void __cdecl UDisBehaviorCombatMelee::InternalConstructor(void *)
//   0x7566f0  public: static void __cdecl UDisBehaviorAssassinCombat::InternalConstructor(void *)
//   0x756730  public: static void __cdecl UDisBehaviorCombatCityGuard::InternalConstructor(void *)
//   0x756770  public: static void __cdecl UDisBehaviorCombatEliteGuard::InternalConstructor(void *)
//   0x7567b0  public: static void __cdecl UDisBehaviorOverseerHMasterCombat::InternalConstructor(void *)
//   0x7567f0  public: static void __cdecl UDisBehaviorOverseerCombat::InternalConstructor(void *)
//   0x756830  public: static void __cdecl UDisBehaviorThugCombat::InternalConstructor(void *)
//   0x756870  public: static void __cdecl UDisBehaviorMusicalCombat::InternalConstructor(void *)
//   0x756890  public: static void __cdecl UDisBehaviorTallBoyCombat::InternalConstructor(void *)
//   0x7568b0  public: static void __cdecl UDisBehaviorCombatRatSwarm::InternalConstructor(void *)
//   0x7568f0  public: static void __cdecl UDisBehaviorCombatRatSwarmEliteGuard::InternalConstructor(void *)
//   0x756930  public: static void __cdecl UDisBehaviorTallboyShoot::InternalConstructor(void *)
//   0x7593d0  protected: virtual __thiscall UDisTweaks_AIBehavior_Combat::~UDisTweaks_AIBehavior_Combat(void)
//   0x759490  protected: virtual __thiscall UDisTweaks_AIBehavior_CombatMelee::~UDisTweaks_AIBehavior_CombatMelee(void)
//   0x759520  protected: virtual __thiscall UDisTweaks_AIBehavior_CombatCityGuard::~UDisTweaks_AIBehavior_CombatCityGuard(void)
//   0x7595b0  protected: virtual __thiscall UDisTweaks_AIBehavior_CombatEliteGuard::~UDisTweaks_AIBehavior_CombatEliteGuard(void)
//   0x759640  protected: virtual __thiscall UDisTweaks_AIBehavior_OverseerHMaster_Combat::~UDisTweaks_AIBehavior_OverseerHMaster_Combat(void)
//   0x7596d0  protected: virtual __thiscall UDisTweaks_AIBehavior_OverseerCombat::~UDisTweaks_AIBehavior_OverseerCombat(void)
//   0x759760  protected: virtual __thiscall UDisTweaks_AIBehavior_TallBoyCombat::~UDisTweaks_AIBehavior_TallBoyCombat(void)
//   0x7597f0  protected: virtual __thiscall UDisTweaks_AIBehavior_CombatRatSwarm::~UDisTweaks_AIBehavior_CombatRatSwarm(void)
//   0x759880  protected: virtual __thiscall UDisTweaks_AIBehavior_CombatRatSwarmEliteGuard::~UDisTweaks_AIBehavior_CombatRatSwarmEliteGuard(void)
//   0x759910  protected: virtual __thiscall UDisTweaks_AIBehavior_TallboyShoot::~UDisTweaks_AIBehavior_TallboyShoot(void)
//   0x75a780  public: static void __cdecl UDisTweaks_AIBehavior_Combat::InternalConstructor(void *)
//   0x75a7a0  public: static void __cdecl UDisTweaks_AIBehavior_CombatMelee::InternalConstructor(void *)
//   0x75a7c0  public: static void __cdecl UDisTweaks_AIBehavior_CombatCityGuard::InternalConstructor(void *)
//   0x75a7e0  public: static void __cdecl UDisTweaks_AIBehavior_CombatEliteGuard::InternalConstructor(void *)
//   0x75a800  public: static void __cdecl UDisTweaks_AIBehavior_OverseerHMaster_Combat::InternalConstructor(void *)
//   0x75a820  public: static void __cdecl UDisTweaks_AIBehavior_OverseerCombat::InternalConstructor(void *)
//   0x75a840  public: static void __cdecl UDisTweaks_AIBehavior_TallBoyCombat::InternalConstructor(void *)
//   0x75a860  public: static void __cdecl UDisTweaks_AIBehavior_CombatRatSwarm::InternalConstructor(void *)
//   0x75a880  public: static void __cdecl UDisTweaks_AIBehavior_CombatRatSwarmEliteGuard::InternalConstructor(void *)
//   0x75a8a0  public: static void __cdecl UDisTweaks_AIBehavior_TallboyShoot::InternalConstructor(void *)
//   0x766e40  protected: virtual __thiscall UDisAISubStateCombatBase::~UDisAISubStateCombatBase(void)
//   0x76ebf0  protected: virtual __thiscall UDisAISubProcessManageAttacks::~UDisAISubProcessManageAttacks(void)
//   0x76edb0  protected: virtual __thiscall UDisAISubStateDoAttractSpell::~UDisAISubStateDoAttractSpell(void)
//   0x76ee80  protected: virtual __thiscall UDisAISubStateDoWeaponManoeuver::~UDisAISubStateDoWeaponManoeuver(void)
//   0x7727e0  public: static void __cdecl UDisAISubProcessManageAttacks::InternalConstructor(void *)
//   0x772800  public: static void __cdecl UDisAISubStateCombatBase::InternalConstructor(void *)
//   0x7728b0  protected: virtual __thiscall UDisAISubStateMeleeChase::~UDisAISubStateMeleeChase(void)
//   0x772a30  protected: virtual __thiscall UDisAISubStateMeleeEngage::~UDisAISubStateMeleeEngage(void)
//   0x772b20  public: static void __cdecl UDisAISubStateDoAttractSpell::InternalConstructor(void *)
//   0x772b40  public: static void __cdecl UDisAISubStateDoWeaponManoeuver::InternalConstructor(void *)
//   0x772c40  protected: virtual __thiscall UDisAISubStateFirePistol::~UDisAISubStateFirePistol(void)
//   0x772dd0  protected: virtual __thiscall UDisAISubStateLieInWait::~UDisAISubStateLieInWait(void)
//   0x774dc0  public: static void __cdecl UDisAISubStateMeleeChase::InternalConstructor(void *)
//   0x774de0  public: static void __cdecl UDisAISubStateMeleeEngage::InternalConstructor(void *)
//   0x774ed0  protected: virtual __thiscall UDisAISubStateFindShootingPosition::~UDisAISubStateFindShootingPosition(void)
//   0x774fa0  public: static void __cdecl UDisAISubStateFirePistol::InternalConstructor(void *)
//   0x774fc0  public: static void __cdecl UDisAISubStateLieInWait::InternalConstructor(void *)
//   0x776890  public: static void __cdecl UDisAISubStateFindShootingPosition::InternalConstructor(void *)
//   0x7818a0  protected: virtual __thiscall UDisTweaks_AISubProcess_ManageAttacks::~UDisTweaks_AISubProcess_ManageAttacks(void)
//   0x781930  protected: virtual __thiscall UDisTweaks_AISubState_FindShootingPosition::~UDisTweaks_AISubState_FindShootingPosition(void)
//   0x782c50  public: static void __cdecl UDisTweaks_AISubProcess_ManageAttacks::InternalConstructor(void *)
//   0x782c70  public: static void __cdecl UDisTweaks_AISubState_FindShootingPosition::InternalConstructor(void *)
//   0x792460  protected: virtual __thiscall UDisAIBrainProcessBattleSense::~UDisAIBrainProcessBattleSense(void)
//   0x796fc0  public: static void __cdecl UDisAIBrainProcessBattleSense::InternalConstructor(void *)
//   0x7a19d0  protected: virtual __thiscall UDisTweaks_AIAttackPattern::~UDisTweaks_AIAttackPattern(void)
//   0x7a1a70  protected: virtual __thiscall UDisTweaks_AIBrainProcess_BattleSense::~UDisTweaks_AIBrainProcess_BattleSense(void)
//   0x7a3310  public: static void __cdecl UDisTweaks_AIAttackPattern::InternalConstructor(void *)
//   0x7a3330  public: static void __cdecl UDisTweaks_AIBrainProcess_BattleSense::InternalConstructor(void *)
//   0x8d5ab0  protected: virtual __thiscall UDisGlobalCombatManager::~UDisGlobalCombatManager(void)
//   0x8d5b30  public: static void __cdecl UDisGlobalCombatManager::InternalConstructor(void *)
//   0xba8490  _dynamic_initializer_for__UDisBehaviorAmbushexecRequestStateExitCallback_LieInWaitTemp__
//   0xba84b0  _dynamic_initializer_for__UDisBehaviorAmbushexecRequestStateExitCallback_TakeActorPositionTemp__
//   0xba84d0  _dynamic_initializer_for__UDisBehaviorCombatMeleeexecRequestStateExitCallback_FindShootingPositionTemp__
//   0xba84f0  _dynamic_initializer_for__UDisBehaviorCombatMeleeexecRefreshCallback_StandTemp__
//   0xba8510  _dynamic_initializer_for__UDisBehaviorCombatMeleeexecOnEnterCallback_StandTemp__
//   0xba8530  _dynamic_initializer_for__UDisBehaviorCombatMeleeexecRefreshCallback_MeleeChaseTemp__
//   0xba8550  _dynamic_initializer_for__UDisBehaviorCombatMeleeexecRefreshCallback_MeleeEngageTemp__
//   0xba8570  _dynamic_initializer_for__UDisBehaviorAssassinCombatexecRequestStateExitCallback_DoAttractSpellTemp__
//   0xba8590  _dynamic_initializer_for__UDisBehaviorCombatEliteGuardexecRequestStateExitCallback_FirePistolTemp__
//   0xba85b0  _dynamic_initializer_for__UDisBehaviorCombatEliteGuardexecOnEnterCallback_FirePistolTemp__
//   0xba85d0  _dynamic_initializer_for__UDisBehaviorCombatEliteGuardexecRequestStateExitCallback_FindShootingPositionTemp__
//   0xba85f0  _dynamic_initializer_for__UDisBehaviorCombatEliteGuardexecRefreshCallback_FindShootingPositionTemp__
//   0xba8610  _dynamic_initializer_for__UDisBehaviorCombatEliteGuardexecRefreshCallback_StandTemp__
//   0xba8630  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecOnExitCallback_StandTemp__
//   0xba8650  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecOnEnterCallback_StandTemp__
//   0xba8670  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecRefreshCallback_MeleeChaseTemp__
//   0xba8690  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecRefreshCallback_MeleeEngageTemp__
//   0xba86b0  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecRequestStateExitCallback_GenericActionTemp__
//   0xba86d0  _dynamic_initializer_for__UDisBehaviorOverseerCombatexecOnEnterCallback_GenericActionTemp__
//   0xba86f0  _dynamic_initializer_for__UDisBehaviorMusicalCombatexecRequestStateExitCallback_DoWeaponManoeuverTemp__
//   0xba8710  _dynamic_initializer_for__UDisBehaviorMusicalCombatexecRefreshCallback_MaintainDistanceTemp__
//   0xba8730  _dynamic_initializer_for__UDisBehaviorMusicalCombatexecRefreshCallback_InitTemp__
//   0xba8750  _dynamic_initializer_for__UDisBehaviorTallBoyCombatexecRequestStateExitCallback_FindShootingPositionTemp__
//   0xba8770  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecRequestStateExitCallback_TakeActorPositionTemp__
//   0xba8790  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecOnExitCallback_TakeActorPositionTemp__
//   0xba87b0  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecRefreshCallback_TakeActorPositionTemp__
//   0xba87d0  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecOnEnterCallback_TakeActorPositionTemp__
//   0xba87f0  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecRefreshCallback_StandTemp__
//   0xba8810  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmexecOnEnterCallback_StandTemp__
//   0xba8830  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmEliteGuardexecRequestStateExitCallback_FirePistolTemp__
//   0xba8850  _dynamic_initializer_for__UDisBehaviorCombatRatSwarmEliteGuardexecRefreshCallback_TakeActorPositionTemp__
//   0xba8870  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecRequestStateExitCallback_GenericActionTemp__
//   0xba8890  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecRequestStateExitCallback_TakePositionTemp__
//   0xba88b0  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecRequestStateExitCallback_DoWeaponManoeuverTemp__
//   0xba88d0  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecOnExitCallback_MenaceTemp__
//   0xba88f0  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecRefreshCallback_MenaceTemp__
//   0xba8910  _dynamic_initializer_for__UDisBehaviorEnemyUnreachableexecOnEnterCallback_MenaceTemp__
//   0xba8930  _dynamic_initializer_for__UDisBehaviorShootexecTickCallback_StandTemp__
//   0xba8950  _dynamic_initializer_for__UDisBehaviorShootexecRequestStateExitCallback_FirePistolTemp__
