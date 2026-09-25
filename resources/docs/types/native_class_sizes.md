# Native class sizes: 2012 Shipping vs 2013 retail

Package H (Phase 2b). Source: `resources/tools/ida/export_class_sizes.py` run on `shipping2012_agentH.i64`
(2012 Shipping build, PDB names, image base 0) and `retail2013_agentH.i64` (retail `Dishonored.exe`, engine 9411,
bare auto-analysis, image base 0x400000), merged into `resources/docs/types/native_class_sizes.csv`.
**Every `size_2013` number comes from the retail exe; `size_2012` from the 2012 exe.** All RVAs are image-base relative.

## Method

Every native class is registered through `UClass::UClass(EStaticConstructor, DWORD InSize, DWORD InClassFlags,
DWORD InOtherClassFlags, DWORD InClassCastFlags, const TCHAR* InNameStr, const TCHAR* InPackageName,
const TCHAR* InConfigName, QWORD InFlags, ctor, static ctor, static init)` with `sizeof(TClass)` as `InSize` and
`TEXT("UClassName") + 1` as the name. The two builds feed that constructor differently:

| | 2012 Shipping | 2013 retail |
|---|---|---|
| Registrant | `X::GetPrivateStaticClassX(const TCHAR* Package)`: pushes the 13 arguments itself (2,538 call sites of the named ctor, rva 0x7a1a0). `Package` is its own parameter, pushed as a literal by `X::StaticClassNoInline` | `X::StaticClass()` pushes a pointer to a 12-dword descriptor in `.data` (`InSize` ... `InClassStaticInitializer` in argument order) and calls one common `GetPrivateStaticClass(desc)` (rva 0x79370, 2,857 call sites) that copies the 12 dwords onto the stack and calls the ctor (rva 0x77060) with `this = desc`: the descriptor is the start of the in-place `UClass` storage |
| Super / Within | `InitializePrivateStaticClassX`: `call Within::StaticClass; push; mov eax, X::PrivateStaticClass; push; call Super::StaticClass; push; call InitializePrivateStaticClassCommon` | identical shape (common function rva 0x24880) |

The script auto-detects the mode per call site (`push`-walk or descriptor), reads the wide strings, takes the U/A
prefix from the character before `InNameStr`, and resolves Super/Within by mapping the `StaticClass` function
addresses of the init function back to classes. In the 2013 exe 44 class-name literals were pooled with a plain
`L"Name"` (no prefix character; `AActor`, `UWorld`, ... among them): their prefix is derived from the super chain
(`AActor` subtree = `A`, everything else `U`). The 2012 exe has 49 such literals; the rule reproduces the PDB
prefix letter for all 49, and the derived name is the PDB name for 44 of them (the other five are the four
`UDEPRECATED_*` classes and `UDEPRECATED_PBRuleNodeBase`, which `IMPLEMENT_CLASS` registers under the name without
`DEPRECATED_`; on the 2012 side the PDB symbol wins, so they are keyed by their C++ name). Two 2013 `StaticClass` bodies are tail chunks of a neighbouring
function in the bare analysis (`jmp X::StaticClass`); the script works on function chunks for that reason.

## Validation (2012 against the PDB)

- 2,538 registrations decoded, 0 call sites rejected, 2,538 distinct classes (no duplicates).
- **2,538 of 2,538 registered `InSize` values equal `sizes.csv` (PDB `sizeof`); 0 mismatches, 0 classes missing from `sizes.csv`.**
  Every class name recovered from the string literal equals the class in the mangled `GetPrivateStaticClass<X>` symbol.
- Super/Within resolved for all 2,538 (all `InitializePrivateStaticClass<X>` match the pattern).
- Independent check of the method on both exes: the in-place `UClass` containers in `.data` are never closer than
  `sizeof(UClass)` rounded to its 8-byte alignment. 2012: minimum container spacing 456 = `sizeof(UClass)` 456.
  2013: minimum spacing 440 = 436 rounded up to 8. The 2013 table is therefore self-consistent with its own `UClass` size.

## Totals

| | 2012 | 2013 |
|---|---|---|
| Native classes registered | 2,538 | 2,857 |
| In both builds | 2,512 | 2,512 |
| Only in this build | 26 | 345 (132 `DLC05*`, 91 `DLC06*`, 103 `DLC07*`, 19 other) |
| Shared classes whose size changed | | 274 (255 DishonoredGame, 14 Engine, 1 Core, 1 GameFramework, 1 IpDrv, 1 OnlineSubsystemSteamworks, 1 AkAudio) |
| `ClassFlags` changed | | 2 |
| Super class changed | | 5 |
| Classes per package (2013) | | DishonoredGame 1,823, Engine 915, Core 44, GameFramework 34, AkAudio 18, GFxUI 17, IpDrv 2, OnlineSubsystemSteamworks 2, WinDrv 2 |

`InFlags` is `0x0408408400004000` and `InOtherClassFlags` is 0 for every class in both builds. Package names are
identical for every shared class (`package_2012` = `package_2013`).

## Core contract types

All Core contract types keep their 2012 size in the retail build **except `UClass` (456 -> 436)**.

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|


### `UClass` 456 -> 436 (retail)

Diff of the two `UClass::UClass(EStaticConstructor, ...)` decompiles (2012 rva 0x7a1a0, 2013 rva 0x77060; both
call `UState::UState` first, and `UState` keeps its 200 bytes):

| 2012 (PDB offsets) | 2013 (ctor stores) |
|---|---|
| `ClassFlags` @200, `m_OtherClassFlags` @204, `ClassCastFlags` @208, `ClassUnique` @212, `ClassWithin` @216, `ClassConfigName` (FName) @220 | same: dwords 50-56 |
| `ClassReps` @228 ... `DependentOn` @300 (7 TArrays), `bForceScriptOrder` @312, `ClassHeaderFilename` (FString) @316: 25 dwords zeroed | 22 dwords zeroed (dwords 57-78): **one 12-byte TArray/FString of this block is gone** |
| `ClassDefaultObject` @328, `m_DropdownCategory` (FName) @332, `ClassConstructor` @340, `ClassStaticConstructor` @344, `ClassStaticInitializer` @348 | `ClassDefaultObject` = dword 79 (@316) immediately followed by the three function pointers (dwords 80-82, @320-@328): **`m_DropdownCategory` (8 bytes) is gone** |
| `ComponentNameToDefaultObjectMap` @352 (TSparseArray ctor + 2 zeroed dwords), `Interfaces` @412, `m_pInterfaceOffsets` @424, `DefaultPropText` @428, `bNeedsPropertiesLinked` @440 = 1, `ReferenceTokenStream` @444 | same order from dword 83 (@332): map ctor at @332, 9 zeroed dwords, `bNeedsPropertiesLinked` = dword 105 (@420), `ReferenceTokenStream` dwords 106-108 (@424), end @436 |

So the retail `UClass` lost exactly `m_DropdownCategory` and one of the seven 12-byte members between `ClassReps`
and `ClassHeaderFilename` (`ClassHeaderFilename` is the most likely candidate: an editor-only header path, the block
is `#if !CONSOLE && !DEDICATED_SERVER` in the reference, and the 2012 PDB layout already lacks the reference's
`ClassGroupNames`). Which of the seven it is cannot be told from the constructor (all are zero-initialised);
`UClass::Serialize` in the retail exe (reachable through the retail `UClass` vftable at rva 0xbbe218) decides it and
is on package L's path anyway. Everything at or after `ClassDefaultObject` shifts by -12, everything after
`m_DropdownCategory` by -20: `ClassConstructor` 340 -> 320, `ComponentNameToDefaultObjectMap` 352 -> 332,
`Interfaces` 412 -> 392, `m_pInterfaceOffsets` 424 -> 404, `DefaultPropText` 428 -> 408, `bNeedsPropertiesLinked` 440 -> 420,
`ReferenceTokenStream` 444 -> 424.

## Size changes by package (shared classes, 2012 -> 2013)

274 of the 2,512 shared classes changed. All but two grew (`UClass` -20, `UOnlineSubsystemSteamworks` -152, the
latter with `UOnlineSubsystemCommonImpl` +16 and `UOnlineSubsystem` +12 above it: an online-layer rework).
Actor-side growth concentrates in the pawn hierarchy (`ADishonoredPawn` +192, `ADishonoredNPCPawn` +256,
`ADishonoredPlayerPawn` +832, `ADisTallboyNPCPawn` +240) and in `ADishonoredUsableObject` (+16, inherited by its
subtree). The Engine changes are small (`UTexture2D` +4 propagating to its subclasses, `USkeletalMeshComponent`
+32, `AGamePawn`/`AMatineePawn` +16 with `APawn` itself unchanged, DLC management +16/+12).

### AkAudio (1 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| USeqAct_AkPostEvent | 276 | 280 | +4 | USeqAct_Latent |

### Core (1 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| UClass | 456 | 436 | -20 | UState |

### DishonoredGame (255 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| ADisAbstractItemPickupNote | 960 | 976 | +16 | ADisAbstractItemPickup |
| ADisAlarmBell | 912 | 928 | +16 | ADishonoredUsableObject |
| ADisAudioLogPlayer | 800 | 848 | +48 | ADishonoredUsableObject |
| ADisDialogInanimateDummy | 624 | 656 | +32 | AActor |
| ADisFish | 672 | 688 | +16 | AActor |
| ADisKey_Base | 960 | 976 | +16 | ADisPickup_Base |
| ADisMovableLimb | 1040 | 1072 | +32 | ADishonoredKAsset |
| ADisPickup_Base | 944 | 960 | +16 | ADishonoredKActor |
| ADisPlayerAudioLogDummyActor | 624 | 656 | +32 | AActor |
| ADisPossessablePawn | 2032 | 2224 | +192 | ADishonoredPawn |
| ADisPossessionProxyPawn | 2048 | 2224 | +176 | ADisPossessablePawn |
| ADisProjectileLauncher | 688 | 704 | +16 | ADisSkeletalBreakable |
| ADisRiverKrust | 992 | 1008 | +16 | AActor |
| ADisSkeletalMeshActorMAT | 656 | 672 | +16 | ASkeletalMeshActorMAT |
| ADisSpeakerGroup_PA | 640 | 672 | +32 | ADisDialogInanimateDummy |
| ADisSpeaker_PA | 1024 | 1072 | +48 | ADishonoredBreakable |
| ADisStatPickup | 992 | 1024 | +32 | ADisPickup_Base |
| ADisTallboyAttachment | 1056 | 1072 | +16 | ADisNPCAttachment |
| ADisTallboyNPCPawn | 3392 | 3632 | +240 | ADishonoredNPCPawn |
| ADisTravelSpawner | 1184 | 1200 | +16 | ADishonoredSpawner |
| ADisTripwire | 656 | 672 | +16 | ASkeletalMeshActor |
| ADisWaterSource | 768 | 784 | +16 | ADishonoredUsableObject |
| ADisWhaleBoneCharm | 960 | 976 | +16 | ADisPickup_Base |
| ADisWhaleOilBattery | 1104 | 1120 | +16 | ADishonoredMovable |
| ADishonoredDynamicUsableObject | 768 | 784 | +16 | ADishonoredUsableObject |
| ADishonoredGameInfo | 5632 | 5680 | +48 | AGameInfo |
| ADishonoredInventoryPickup | 1008 | 1024 | +16 | ADisStatPickup |
| ADishonoredMovable | 1056 | 1072 | +16 | ADishonoredBreakable |
| ADishonoredNPCPawn | 3328 | 3584 | +256 | ADisPossessablePawn |
| ADishonoredPawn | 2000 | 2192 | +192 | AGamePawn |
| ADishonoredPlayerPawn | 3856 | 4688 | +832 | ADishonoredPawn |
| ADishonoredUsableObject | 768 | 784 | +16 | ADisSkeletalBreakable |
| UDisAIBrainProcessBattleSense | 84 | 88 | +4 | UDisAIBrainProcess |
| UDisBehaviorCombatRatSwarmEliteGuard | 200 | 204 | +4 | UDisBehaviorCombatRatSwarm |
| UDisBehaviorEscapeExplosion | 204 | 288 | +84 | UDisAIBehaviorWithDesires |
| UDisBehaviorSoiree | 200 | 204 | +4 | UDisAIBehaviorWithDesires |
| UDisConvGlobalMan | 2016 | 2044 | +28 | UObject |
| UDisConv_Action | 128 | 136 | +8 | UDisConv_Node |
| UDisConv_Blurb | 228 | 236 | +8 | UDisConv_Node |
| UDisConv_Branch | 128 | 136 | +8 | UDisConv_Node |
| UDisConv_CheckSpeakerRelationship | 132 | 140 | +8 | UDisConv_Condition |
| UDisConv_CheckSpeakerSuspicionLevel | 128 | 136 | +8 | UDisConv_Condition |
| UDisConv_CheckStoryFlag | 148 | 156 | +8 | UDisConv_Condition |
| UDisConv_CheckTaskState | 136 | 144 | +8 | UDisConv_Condition |
| UDisConv_Comment | 160 | 168 | +8 | UDisConv_Node |
| UDisConv_CompareDarknessLevel | 132 | 140 | +8 | UDisConv_Comparison |
| UDisConv_Comparison | 128 | 136 | +8 | UDisConv_Condition |
| UDisConv_Condition | 128 | 136 | +8 | UDisConv_Node |
| UDisConv_ConversationFired | 140 | 148 | +8 | UDisConv_Condition |
| UDisConv_ConversationRef | 128 | 136 | +8 | UDisConv_Node |
| UDisConv_DialogHook | 148 | 156 | +8 | UDisConv_Node |
| UDisConv_FactionBranch | 140 | 148 | +8 | UDisConv_Branch |
| UDisConv_HadConversation | 140 | 148 | +8 | UDisConv_Condition |
| UDisConv_HasAbstractItem | 136 | 144 | +8 | UDisConv_Condition |
| UDisConv_HasInventoryItem | 136 | 144 | +8 | UDisConv_Condition |
| UDisConv_HasInventoryItemEquipped | 132 | 140 | +8 | UDisConv_Condition |
| UDisConv_HasObjective | 132 | 140 | +8 | UDisConv_Condition |
| UDisConv_Hook_DeathMode | 148 | 156 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_Distance | 160 | 168 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_Distraction | 160 | 168 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_HeartTargeted | 172 | 180 | +8 | UDisConv_Hook_SplitByTypeAndActor |
| UDisConv_Hook_Investigate | 148 | 156 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_KismetActivated | 184 | 192 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_Notice | 148 | 156 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_NoticeBroken | 172 | 180 | +8 | UDisConv_Hook_SplitByTypeAndActor |
| UDisConv_Hook_PlayAudioLog | 184 | 192 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_PlayerLoiter | 160 | 168 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_PlayerLookAt | 164 | 172 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_SplitByTypeAndActor | 172 | 180 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_SuspicionDist | 152 | 160 | +8 | UDisConv_Hook_SuspicionLevel |
| UDisConv_Hook_SuspicionLevel | 148 | 156 | +8 | UDisConv_DialogHook |
| UDisConv_Hook_WitnessedInteraction | 172 | 180 | +8 | UDisConv_Hook_SplitByTypeAndActor |
| UDisConv_Hook_WitnessedMagic | 160 | 168 | +8 | UDisConv_DialogHook |
| UDisConv_InitiatorSpeaker | 156 | 164 | +8 | UDisConv_Speaker |
| UDisConv_IsObjectiveComplete | 132 | 140 | +8 | UDisConv_Condition |
| UDisConv_IsPossessed | 128 | 136 | +8 | UDisConv_Condition |
| UDisConv_IsSpeakerAvailable | 128 | 136 | +8 | UDisConv_Condition |
| UDisConv_KismetActivateRemoteEvent | 144 | 152 | +8 | UDisConv_Action |
| UDisConv_MeSpeaker | 152 | 160 | +8 | UDisConv_Speaker |
| UDisConv_Node | 128 | 136 | +8 | UObject |
| UDisConv_NonWord | 228 | 236 | +8 | UDisConv_Blurb |
| UDisConv_OneShotSpeaker | 152 | 160 | +8 | UDisConv_Speaker |
| UDisConv_PlayerChoice | 168 | 176 | +8 | UDisConv_Node |
| UDisConv_PlayerLookAtActor | 172 | 180 | +8 | UDisConv_Action |
| UDisConv_PlayerLookAtSpeaker | 172 | 180 | +8 | UDisConv_Action |
| UDisConv_PlayerStopLookAt | 128 | 136 | +8 | UDisConv_Action |
| UDisConv_RandomBranch | 144 | 152 | +8 | UDisConv_Branch |
| UDisConv_RandomSequentialBranch | 144 | 152 | +8 | UDisConv_SequentialBranch |
| UDisConv_SeenDialogLabel | 148 | 156 | +8 | UDisConv_Condition |
| UDisConv_SequentialBranch | 144 | 152 | +8 | UDisConv_Branch |
| UDisConv_SetStoryFlag | 152 | 160 | +8 | UDisConv_Action |
| UDisConv_Soiree | 172 | 180 | +8 | UDisConv_Action |
| UDisConv_SpawnerBranch | 140 | 148 | +8 | UDisConv_Branch |
| UDisConv_Speaker | 148 | 156 | +8 | UDisConv_Node |
| UDisConv_SpeakerCombatStatus | 128 | 136 | +8 | UDisConv_Condition |
| UDisConv_SpeakerHasTweaks | 140 | 148 | +8 | UDisConv_Branch |
| UDisConv_SpeakerInStoryGroup | 140 | 148 | +8 | UDisConv_Branch |
| UDisConv_SpeakerSupportsDialogTree | 140 | 148 | +8 | UDisConv_Branch |
| UDisConv_TimeLimit | 136 | 144 | +8 | UDisConv_Condition |
| UDisConv_TryFallbackTree | 128 | 136 | +8 | UDisConv_Action |
| UDisConversation | 192 | 200 | +8 | UDisConv_Node |
| UDisDialogTree | 364 | 372 | +8 | UDisConversation |
| UDisDialogTree_OneShot | 364 | 372 | +8 | UDisDialogTree |
| UDisGFxMoviePlayerBase | 424 | 440 | +16 | UGFxMoviePlayer |
| UDisGFxMoviePlayerGamma | 436 | 456 | +20 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerHUD | 1484 | 1448 | -36 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerHUDFX | 424 | 440 | +16 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerJournal | 1172 | 928 | -244 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerMainMenu | 508 | 784 | +276 | UDisGFxMoviePlayerMenuBase |
| UDisGFxMoviePlayerMenuBase | 476 | 504 | +28 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerMissionStats | 444 | 460 | +16 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerNote | 1252 | 1992 | +740 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerPauseMenu | 504 | 520 | +16 | UDisGFxMoviePlayerMenuBase |
| UDisGFxMoviePlayerPowerWheel | 564 | 628 | +64 | UDisGFxMoviePlayerBase |
| UDisGFxMoviePlayerStore | 452 | 472 | +20 | UDisGFxMoviePlayerBase |
| UDisGlobalCombatManager | 332 | 340 | +8 | UObject |
| UDisGlobalUIManager | 672 | 792 | +120 | UObject |
| UDisItemContext | 168 | 176 | +8 | UObject |
| UDisItemContext_AimAssistAttack | 188 | 196 | +8 | UDisItemContext |
| UDisItemContext_Assassinate | 308 | 316 | +8 | UDisItemContext_Finisher |
| UDisItemContext_AttackNPCTallboyMelee | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_Choke | 172 | 180 | +8 | UDisItemContext |
| UDisItemContext_DropAssassinate | 200 | 208 | +8 | UDisItemContext |
| UDisItemContext_Fatality | 320 | 328 | +8 | UDisItemContext_Finisher |
| UDisItemContext_Finisher | 296 | 304 | +8 | UDisItemContext_MeleeAttackPlayer |
| UDisItemContext_FireCrossbow | 248 | 256 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_FirePistol | 244 | 252 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_MeleeAttack | 248 | 256 | +8 | UDisItemContext |
| UDisItemContext_MeleeAttackNPCBase | 252 | 264 | +12 | UDisItemContext_MeleeAttack |
| UDisItemContext_MeleeAttackPlayer | 280 | 288 | +8 | UDisItemContext_MeleeAttack |
| UDisItemContext_MeleeBlock | 168 | 176 | +8 | UDisItemContext |
| UDisItemContext_MeleeBlockPlayer | 208 | 216 | +8 | UDisItemContext_MeleeBlock |
| UDisItemContext_Minigame | 268 | 288 | +20 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_MultiFatality | 280 | 288 | +8 | UDisItemContext_MeleeAttackPlayer |
| UDisItemContext_NPCAmbush | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackLeft180 | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackLeft90 | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackLong | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackMedium | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackRight180 | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackRight90 | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackShort | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackStepBack | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackUnder | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCAttackUnder_Grenade | 268 | 276 | +8 | UDisItemContext_NPCThrowGrenade |
| UDisItemContext_NPCAttackUnder_Gun | 356 | 364 | +8 | UDisItemContext_NPCFireGun |
| UDisItemContext_NPCAttractSpell | 176 | 184 | +8 | UDisItemContext |
| UDisItemContext_NPCAttractSpellCoupDeGrace | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCBackStep | 168 | 176 | +8 | UDisItemContext |
| UDisItemContext_NPCBash | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCFatality | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCFireBow | 252 | 260 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_NPCFireGun | 356 | 364 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_NPCJumpAttack | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCLobGrenadeAtUnreachable | 268 | 276 | +8 | UDisItemContext_NPCThrowGrenade_Aimed |
| UDisItemContext_NPCPush | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCRatCrushAttempt | 268 | 280 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCReloadGun | 168 | 176 | +8 | UDisItemContext |
| UDisItemContext_NPCRiposte | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCSideStep | 172 | 180 | +8 | UDisItemContext |
| UDisItemContext_NPCStomp | 252 | 264 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_NPCTaunt | 192 | 200 | +8 | UDisItemContext |
| UDisItemContext_NPCTeleportSpell | 228 | 236 | +8 | UDisItemContext |
| UDisItemContext_NPCThrow | 268 | 276 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_NPCThrowGrenade | 268 | 276 | +8 | UDisItemContext_NPCThrow |
| UDisItemContext_NPCThrowGrenade_Aimed | 268 | 276 | +8 | UDisItemContext_NPCThrowGrenade |
| UDisItemContext_NPCThrowObject | 280 | 288 | +8 | UDisItemContext_NPCThrow |
| UDisItemContext_NPCThrowWhiskey | 268 | 276 | +8 | UDisItemContext_NPCThrow |
| UDisItemContext_NPCTune | 168 | 176 | +8 | UDisItemContext |
| UDisItemContext_NPCTune_Combat | 196 | 204 | +8 | UDisItemContext_NPCTune |
| UDisItemContext_NPCTune_Protection | 168 | 176 | +8 | UDisItemContext_NPCTune |
| UDisItemContext_NPCWhiskeyFire | 188 | 196 | +8 | UDisItemContext |
| UDisItemContext_NPC_OverseerJumpAway | 268 | 276 | +8 | UDisItemContext_NPCThrowGrenade |
| UDisItemContext_ParryNPC | 260 | 272 | +12 | UDisItemContext_MeleeAttackNPCBase |
| UDisItemContext_ProjectileAttack | 244 | 252 | +8 | UDisItemContext_AimAssistAttack |
| UDisItemContext_ReloadGunPlayer | 176 | 184 | +8 | UDisItemContext |
| UDisItemContext_ThrowGrenade | 256 | 264 | +8 | UDisItemContext_ProjectileAttack |
| UDisItemContext_UsePower | 192 | 200 | +8 | UDisItemContext_AimAssistAttack |
| UDisItemContext_UseSpringRazor | 276 | 284 | +8 | UDisItemContext |
| UDisItemContext_WHArmAttack | 272 | 288 | +16 | UDisItemContext_Minigame |
| UDisItemContext_WHBark | 196 | 204 | +8 | UDisItemContext_NPCTaunt |
| UDisItemContext_WHJumpAttack | 268 | 288 | +20 | UDisItemContext_Minigame |
| UDisItemContext_WeeperGrab | 268 | 288 | +20 | UDisItemContext_Minigame |
| UDisLocalPlayer | 616 | 620 | +4 | ULocalPlayer |
| UDisNotify_MakeAINoise | 64 | 68 | +4 | UAnimNotify |
| UDisPostProcessManager | 656 | 760 | +104 | UObject |
| UDisSkeletalMeshComponent | 1072 | 1104 | +32 | USkeletalMeshComponent |
| UDisTweaks_AIBehavior_EnemyUnreachable | 188 | 192 | +4 | UDisTweaks_AIBehavior |
| UDisTweaks_AIBehavior_EscapeExplosion | 192 | 196 | +4 | UDisTweaks_AIBehavior |
| UDisTweaks_AIBehavior_Notice | 176 | 188 | +12 | UDisTweaks_AIBehavior |
| UDisTweaks_AIBrain | 200 | 204 | +4 | UDisTweaksBase |
| UDisTweaks_AIBrainProcess_BattleSense | 152 | 160 | +8 | UDisTweaks_AIBrainProcess |
| UDisTweaks_AISubState_FirePistol | 164 | 172 | +8 | UDisTweaks_AISubState |
| UDisTweaks_AISubState_Investigate | 232 | 252 | +20 | UDisTweaks_AISubState |
| UDisTweaks_AbstractItemPickup | 244 | 248 | +4 | UDisTweaks_PickupBase |
| UDisTweaks_AbstractItemPickupAudioLog | 244 | 248 | +4 | UDisTweaks_AbstractItemPickup |
| UDisTweaks_AbstractItemPickupNote | 280 | 284 | +4 | UDisTweaks_AbstractItemPickup |
| UDisTweaks_AlarmBell | 472 | 476 | +4 | UDisTweaks_UsableObject |
| UDisTweaks_Arrow | 304 | 316 | +12 | UDisTweaks_Projectile |
| UDisTweaks_Arrow_Explosive | 320 | 332 | +12 | UDisTweaks_Arrow |
| UDisTweaks_Arrow_Flare | 352 | 364 | +12 | UDisTweaks_Arrow |
| UDisTweaks_Bullet | 320 | 332 | +12 | UDisTweaks_Projectile |
| UDisTweaks_Bullet_Explosive | 324 | 336 | +12 | UDisTweaks_Bullet |
| UDisTweaks_GFxMoviePlayerHUD | 2036 | 2328 | +292 | UDisTweaks_GFxMoviePlayerBase |
| UDisTweaks_GFxMoviePlayerMainMenu | 304 | 328 | +24 | UDisTweaks_GFxMoviePlayerBase |
| UDisTweaks_GFxMoviePlayerMenuBase | 1244 | 1268 | +24 | UDisTweaksBase |
| UDisTweaks_GameCrowdAgentSkeletalRat | 360 | 364 | +4 | UDisTweaksBase |
| UDisTweaks_InventoryPickup | 380 | 428 | +48 | UDisTweaks_StatPickup |
| UDisTweaks_MeleeBlockPlayer | 340 | 352 | +12 | UDisTweaks_MeleeBlock |
| UDisTweaks_NPCPawn | 916 | 924 | +8 | UDisTweaks_Pawn |
| UDisTweaks_NPCPawn_Attributes | 1572 | 1600 | +28 | UDisTweaks_Pawn_Attributes |
| UDisTweaks_NPCTeleportSpell | 260 | 264 | +4 | UDisTweaks_ItemContext |
| UDisTweaks_NPCTune_Protection | 184 | 180 | -4 | UDisTweaks_NPCTune |
| UDisTweaks_PlayerInput | 324 | 332 | +8 | UDisTweaksBase |
| UDisTweaks_PlayerPawn | 1232 | 1816 | +584 | UDisTweaks_Pawn |
| UDisTweaks_PlayerPawn_Attributes | 2264 | 2292 | +28 | UDisTweaks_Pawn_Attributes |
| UDisTweaks_PlayerStats | 1064 | 1544 | +480 | UDisTweaksBase |
| UDisTweaks_Possess | 512 | 524 | +12 | UDisTweaks_ActivePowerBase |
| UDisTweaks_Possessable | 256 | 248 | -8 | UDisTweaksBase |
| UDisTweaks_Projectile | 216 | 228 | +12 | UDisTweaks_KAsset |
| UDisTweaks_ProjectileLauncher | 280 | 296 | +16 | UDisTweaks_SkeletalBreakable |
| UDisTweaks_Projectile_Grenade | 256 | 268 | +12 | UDisTweaks_Projectile_GrenadeBase |
| UDisTweaks_Projectile_GrenadeBase | 228 | 240 | +12 | UDisTweaks_Projectile |
| UDisTweaks_Projectile_StickyGrenade | 252 | 264 | +12 | UDisTweaks_Projectile_GrenadeBase |
| UDisTweaks_Projectile_ThrownObject | 216 | 228 | +12 | UDisTweaks_Projectile |
| UDisTweaks_Projectile_Whiskey | 220 | 232 | +12 | UDisTweaks_Projectile |
| UDisTweaks_RiverKrust | 1192 | 1208 | +16 | UDisTweaksBase |
| UDisTweaks_SpringRazor | 272 | 276 | +4 | UDisTweaks_InventoryItem |
| UDisTweaks_SpringRazorPlaced | 268 | 272 | +4 | UDisTweaksBase |
| UDisTweaks_StatPickup | 360 | 408 | +48 | UDisTweaks_PickupBase |
| UDisTweaks_TallboyNPCPawn | 1004 | 1016 | +12 | UDisTweaks_NPCPawn |
| UDisTweaks_UseSpringRazor | 264 | 268 | +4 | UDisTweaks_ItemContext |
| UDisTweaks_Vulnerability | 252 | 280 | +28 | UDisTweaksBase |
| UDisTweaks_WatchTower | 568 | 572 | +4 | UDisTweaksBase |
| UDishonoredAIBrain | 488 | 500 | +12 | UObject |
| UDishonoredCallbackSkeletalComponent | 1056 | 1088 | +32 | USkeletalMeshComponent |
| UDishonoredCheatManager | 260 | 264 | +4 | UCheatManager |
| UDishonoredEngine | 1964 | 2052 | +88 | UGameEngine |
| UDishonoredGlobalAIManager | 248 | 252 | +4 | UObject |
| UDishonoredItemSkeletalComponent | 1104 | 1136 | +32 | UDishonoredPlayerSkeletalComponent |
| UDishonoredMapInfo | 176 | 184 | +8 | UMapInfo |
| UDishonoredObjective | 100 | 108 | +8 | UObject |
| UDishonoredObjectivesComponent | 108 | 112 | +4 | UActorComponent |
| UDishonoredPlayerSkeletalComponent | 1088 | 1120 | +32 | UDisSkeletalMeshComponent |
| UDishonoredTask_Base | 112 | 124 | +12 | UObject |
| UDishonoredTask_Custom | 124 | 136 | +12 | UDishonoredTask_Base |
| UDishonoredWepPistol | 320 | 332 | +12 | UDishonoredWeapon_Ranged |
| UStateNPCMasterActionMeleeSwarm | 116 | 120 | +4 | UStateNPCMasterAction |
| UStateNPCMasterBePossessed | 108 | 112 | +4 | UStateNPCInstigatedMasterAction |
| UStateNPCMasterThrown | 172 | 176 | +4 | UStateNPCInstigatedMasterAction |
| UStatePlayerMasterInScriptedChoice | 96 | 100 | +4 | UStatePlayerMasterChoice_Base |
| UStatePlayerMasterPossess | 124 | 128 | +4 | UStatePlayerMasterPossess_Base |
| UStatePlayerMasterPossess_Base | 120 | 124 | +4 | UStatePlayerMasterAction |
| UStatePlayerMasterPrePossess | 164 | 168 | +4 | UStatePlayerMasterPossess_Base |
| UStatePlayerMasterSoiree | 136 | 140 | +4 | UStatePlayerMasterBase |

### Engine (14 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| AMatineePawn | 1184 | 1200 | +16 | APawn |
| UArkComponentLocomotionConfig | 332 | 336 | +4 | UObject |
| UArkDLCManagementBridge | 96 | 112 | +16 | UObject |
| UDownloadableContentManager | 180 | 192 | +12 | UObject |
| UInterpTrackAIControlBodyIntentionKeyProperties | 64 | 68 | +4 | UInterpTrackAIControlKeyProperties |
| UInterpTrackInstStretchAnimControl | 92 | 100 | +8 | UInterpTrackInst |
| UInterpTrackStretchAnimControl | 164 | 168 | +4 | UInterpTrackFloatBase |
| ULightMapTexture2D | 372 | 376 | +4 | UTexture2D |
| UOnlineSubsystem | 168 | 180 | +12 | UObject |
| USeqAct_SetMatInstScalarParam | 264 | 272 | +8 | USequenceAction |
| UShadowMapTexture2D | 372 | 376 | +4 | UTexture2D |
| USkeletalMeshComponent | 1056 | 1088 | +32 | UMeshComponent |
| UTexture2D | 368 | 372 | +4 | UTexture |
| UTextureFlipBook | 432 | 436 | +4 | UTexture2D |

### GameFramework (1 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| AGamePawn | 1184 | 1200 | +16 | APawn |

### IpDrv (1 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| UOnlineSubsystemCommonImpl | 184 | 200 | +16 | UOnlineSubsystem |

### OnlineSubsystemSteamworks (1 changed)

| class | size_2012 | size_2013 | delta | super_2013 |
|---|---|---|---|---|
| UOnlineSubsystemSteamworks | 1284 | 1132 | -152 | UOnlineSubsystemCommonImpl |

## Other deltas between the builds

`ClassFlags` changes (2):

- `UDisContactType_Env_Foliage: 0x4 -> 0x2000004`
- `UDisGFxMoviePlayerGlobal: 0x0 -> 0x4`

Super class changes (5; `UDisBehaviorAttentionBase` is a new 2013 intermediate class):

- `ADisTallboyAttachment: ADishonoredMovable -> ADisNPCAttachment`
- `UDisBehaviorCombat: UDisAIBehaviorWithDesires -> UDisBehaviorAttentionBase`
- `UDisBehaviorNotice: UDisAIBehaviorWithDesires -> UDisBehaviorAttentionBase`
- `UDisBehaviorSearch: UDisAIBehaviorWithDesires -> UDisBehaviorAttentionBase`
- `UDisBehaviorStationarySearch: UDisAIBehaviorWithDesires -> UDisBehaviorAttentionBase`

### Only in 2013 (345)

| class | package | size_2013 | super_2013 |
|---|---|---|---|
| ADisDLC05GameInfo | DishonoredGame | 5744 | ADishonoredGameInfo |
| ADisDLC05SkeletalBreakable | DishonoredGame | 672 | ADisSkeletalBreakable |
| ADisDLC05WhaleOilBattery | DishonoredGame | 1120 | ADisWhaleOilBattery |
| ADisDLC05_Movable | DishonoredGame | 1072 | ADishonoredMovable |
| ADisDLC06AssassinNPCPawn | DishonoredGame | 3584 | ADisDLC06NPCPawn |
| ADisDLC06ButcherNPCPawn | DishonoredGame | 3584 | ADisDLC06NPCPawn |
| ADisDLC06DialogOneShot | DishonoredGame | 672 | ADisDialogOneShot |
| ADisDLC06Explosion | DishonoredGame | 656 | ADisExplosion |
| ADisDLC06Gadget_ArcMinePlaced | DishonoredGame | 688 | ASkeletalMeshActor |
| ADisDLC06GameInfo | DishonoredGame | 5728 | ADishonoredGameInfo |
| ADisDLC06NPCAttachment | DishonoredGame | 1072 | ADisNPCAttachment |
| ADisDLC06NPCPawn | DishonoredGame | 3584 | ADishonoredNPCPawn |
| ADisDLC06PlayerController | DishonoredGame | 1840 | ADishonoredPlayerController |
| ADisDLC06PlayerPawn | DishonoredGame | 4720 | ADishonoredPlayerPawn |
| ADisDLC06Projectile_Arrow_Explosive | DishonoredGame | 784 | ADisProjectile_Arrow_Explosive |
| ADisDLC06Projectile_SmokeGrenade | DishonoredGame | 752 | ADisProjectile_Grenade |
| ADisDLC06Spawner | DishonoredGame | 1184 | ADishonoredSpawner |
| ADisDLC06SummonedAssassinNPCPawn | DishonoredGame | 3616 | ADisDLC06AssassinNPCPawn |
| ADisDLC06_TeleportDest | DishonoredGame | 592 | AActor |
| ADisDLC07AssassinNPCPawn | DishonoredGame | 3616 | ADisDLC07NPCPawn |
| ADisDLC07GameInfo | DishonoredGame | 5744 | ADisDLC06GameInfo |
| ADisDLC07GravehoundNPCPawn | DishonoredGame | 3616 | ADisDLC07NPCPawn |
| ADisDLC07GravehoundSkull | DishonoredGame | 1072 | ADishonoredMovable |
| ADisDLC07GravehoundSpawner | DishonoredGame | 1360 | ADisDLC07Spawner |
| ADisDLC07MagicSuppressionVolume | DishonoredGame | 640 | ADisToggleableVolume |
| ADisDLC07NPCPawn | DishonoredGame | 3600 | ADisDLC06NPCPawn |
| ADisDLC07PlayerController | DishonoredGame | 1840 | ADisDLC06PlayerController |
| ADisDLC07PlayerPawn | DishonoredGame | 4752 | ADisDLC06PlayerPawn |
| ADisDLC07RiverKrust | DishonoredGame | 1024 | ADisRiverKrust |
| ADisDLC07RiverKrustProjectile | DishonoredGame | 784 | ADisProjectile_Arrow |
| ADisDLC07SkeletalMovable | DishonoredGame | 720 | ADishonoredKAsset |
| ADisDLC07Spawner | DishonoredGame | 1184 | ADisDLC06Spawner |
| ADisDLC07SummonedAssassinNPCPawn | DishonoredGame | 3632 | ADisDLC07AssassinNPCPawn |
| ADisDLC07TentacleNPCPawn | DishonoredGame | 3600 | ADisDLC07NPCPawn |
| ADisDLC07TentacleSpawner | DishonoredGame | 1200 | ADisDLC07Spawner |
| ADisDLC07TriggerGrannyRecipe | DishonoredGame | 656 | ADisTrigger |
| ADisDLC07WhaleBoneCharmCracked | DishonoredGame | 976 | ADisPickup_Base |
| ADisFog | Engine | 608 | AInfo |
| ADisInterpActor | DishonoredGame | 672 | AInterpActor |
| ADisNPCAttachment | DishonoredGame | 1072 | ADishonoredMovable |
| UDisAnimNodeBlendByHeadBob | DishonoredGame | 224 | UAnimNodeBlendBase |
| UDisAnimNodeBlendHeartAdditive | DishonoredGame | 256 | UAnimNodeAdditiveBlending |
| UDisBehaviorAttentionBase | DishonoredGame | 176 | UDisAIBehaviorWithDesires |
| UDisBehaviorInhibited | DishonoredGame | 160 | UDishonoredAIBehavior |
| UDisCamera_DLC05_FlyingKillCam | DishonoredGame | 256 | UDishonoredCameraInfluence |
| UDisDLC05BehaviorIdle | DishonoredGame | 244 | UDisBehaviorIdle |
| UDisDLC05ChallengeRule_Base | DishonoredGame | 56 | UObject |
| UDisDLC05ChallengeRule_Combo_Base | DishonoredGame | 84 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_Blast | DishonoredGame | 96 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_Boom | DishonoredGame | 92 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_BrutalStreak | DishonoredGame | 96 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_DeathStreak | DishonoredGame | 92 | UDisDLC05ChallengeRule_Combo_Base |
| UDisDLC05ChallengeRule_Combo_MultipleShots | DishonoredGame | 104 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_StealthyStreak | DishonoredGame | 96 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Combo_Survivor | DishonoredGame | 100 | UDisDLC05ChallengeRule_Multiplier |
| UDisDLC05ChallengeRule_Multiplier | DishonoredGame | 64 | UDisDLC05ChallengeRule_Base |
| UDisDLC05ChallengeScoringManager | DishonoredGame | 92 | UObject |
| UDisDLC05GlobalProjectileManager | DishonoredGame | 64 | UDisGlobalProjectileManager |
| UDisDLC05MoviePlayerBrief | DishonoredGame | 456 | UDisGFxMoviePlayerBase |
| UDisDLC05MoviePlayerChallengeMenu | DishonoredGame | 532 | UDisDLC05MoviePlayerLeaderboard |
| UDisDLC05MoviePlayerHUD | DishonoredGame | 1840 | UDisGFxMoviePlayerHUD |
| UDisDLC05MoviePlayerLeaderboard | DishonoredGame | 516 | UDisGFxMoviePlayerBase |
| UDisDLC05MoviePlayerPauseMenu | DishonoredGame | 524 | UDisGFxMoviePlayerPauseMenu |
| UDisDLC05MoviePlayerResultsMenu | DishonoredGame | 548 | UDisDLC05MoviePlayerLeaderboard |
| UDisDLC05ScoringBonus_BTM_Kills | DishonoredGame | 132 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_Expeditious | DishonoredGame | 80 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_Fencer | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_Ghost | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_Invincible | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_SharpShooter | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringBonus_Skinflint | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringModifier_Adrenaline | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Assassinate | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Base | DishonoredGame | 80 | UDisDLC05ChallengeRule_Base |
| UDisDLC05ScoringModifier_Bellup | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_DarkKill | DishonoredGame | 116 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_EatenAlive | DishonoredGame | 100 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_FactionCombo | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Getback | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Headshot | DishonoredGame | 100 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_InFlames | DishonoredGame | 100 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_LimbSevered | DishonoredGame | 100 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Payback | DishonoredGame | 108 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_PerfectBlock | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_PowerCombo | DishonoredGame | 104 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringModifier_Suicide | DishonoredGame | 100 | UDisDLC05ScoringModifier_Base |
| UDisDLC05ScoringRule_Accuracy | DishonoredGame | 96 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_BTM_Kill | DishonoredGame | 88 | UDisDLC05ScoringRule_WithModifiers |
| UDisDLC05ScoringRule_Base | DishonoredGame | 56 | UDisDLC05ChallengeRule_Base |
| UDisDLC05ScoringRule_BrutalStreak | DishonoredGame | 88 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_ChainKill | DishonoredGame | 80 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Chrono | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_ChronoBonus | DishonoredGame | 140 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Custom | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_DropAssassination | DishonoredGame | 108 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Ghost | DishonoredGame | 92 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Headshot | DishonoredGame | 88 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Hostage | DishonoredGame | 88 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Kill | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Mayhem | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_MysteryMan | DishonoredGame | 296 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_OilRain_Accuracy | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_OilRain_BoilingOil | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_OilRain_Combo | DishonoredGame | 84 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_OilRain_Destroy | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_OilRain_PerfectWave | DishonoredGame | 80 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_PlayerStat | DishonoredGame | 80 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Shoot | DishonoredGame | 88 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Slash | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_SurpriseShot | DishonoredGame | 88 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Thief | DishonoredGame | 236 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_TimeMarkerBonus | DishonoredGame | 80 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_TripwireFree | DishonoredGame | 76 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_Vanish | DishonoredGame | 104 | UDisDLC05ScoringRule_Base |
| UDisDLC05ScoringRule_WaveKill | DishonoredGame | 112 | UDisDLC05ScoringRule_WithModifiers |
| UDisDLC05ScoringRule_WithModifiers | DishonoredGame | 68 | UDisDLC05ScoringRule_Base |
| UDisDLC05Tweaks_AIBehavior_Idle | DishonoredGame | 168 | UDisTweaks_AIBehavior_Idle |
| UDisDLC05Tweaks_ChallengeScoringRuleset | DishonoredGame | 180 | UDisTweaksBase |
| UDisDLC05Tweaks_Movable | DishonoredGame | 288 | UDisTweaks_Movable |
| UDisDLC05Tweaks_WhaleOilBattery | DishonoredGame | 364 | UDisTweaks_WhaleOilBattery |
| UDisDLC06AIBrainProcessSummonedAssassin | DishonoredGame | 88 | UDisAIBrainProcess |
| UDisDLC06ActivePowerComponent_DarkVision | DishonoredGame | 208 | UDisActivePowerComponent_DarkVision |
| UDisDLC06ActivePowerComponent_SummonAssassin | DishonoredGame | 260 | UDishonoredActivePowerComponent |
| UDisDLC06ActivePowerComponent_Transversal | DishonoredGame | 368 | UDishonoredActivePowerComponent_Blink |
| UDisDLC06BehaviorAssassinSalute | DishonoredGame | 356 | UDisAIBehaviorWithDesires |
| UDisDLC06BehaviorButcherCombat | DishonoredGame | 304 | UDisBehaviorCombatMelee |
| UDisDLC06BehaviorSummonedAssassinIdle | DishonoredGame | 160 | UDishonoredAIBehavior |
| UDisDLC06CheatManager | DishonoredGame | 264 | UDishonoredCheatManager |
| UDisDLC06Conv_HasPower | DishonoredGame | 148 | UDisConv_Condition |
| UDisDLC06DamageType_WitchScream | DishonoredGame | 136 | UDishonoredDamageType |
| UDisDLC06Gadget_ArcMine | DishonoredGame | 284 | UDisGadget_SpringRazor |
| UDisDLC06Gadget_ArcMineStun | DishonoredGame | 284 | UDisGadget_SpringRazor |
| UDisDLC06GrenadeComponent | DishonoredGame | 208 | UDisGrenadeComponent |
| UDisDLC06ItemContext_NPCFireSaw | DishonoredGame | 304 | UDisItemContext_ProjectileAttack |
| UDisDLC06ItemContext_NPCSawCut | DishonoredGame | 268 | UDisItemContext_MeleeAttackNPCBase |
| UDisDLC06ItemContext_NPCWitchScream | DishonoredGame | 200 | UDisItemContext |
| UDisDLC06ItemContext_NPCWitchTeleport | DishonoredGame | 272 | UDisItemContext |
| UDisDLC06MoviePlayerHUD | DishonoredGame | 1448 | UDisGFxMoviePlayerHUD |
| UDisDLC06MoviePlayerMainMenu | DishonoredGame | 792 | UDisGFxMoviePlayerMainMenu |
| UDisDLC06Notify_ClearBodyIntention | DishonoredGame | 64 | UAnimNotify |
| UDisDLC06Notify_EquipItem | DishonoredGame | 68 | UAnimNotify |
| UDisDLC06SeqAct_DestroyArcMines | DishonoredGame | 248 | USequenceAction |
| UDisDLC06SeqAct_EndDLC06 | DishonoredGame | 268 | UDisSeqAct_Latent |
| UDisDLC06SeqAct_EndGame | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDisDLC06SeqAct_NPCAddPutPocketInteraction | DishonoredGame | 252 | USequenceAction |
| UDisDLC06SeqAct_NPCSetHeadMesh | DishonoredGame | 252 | USequenceAction |
| UDisDLC06SeqAct_SetObjectiveOptional | DishonoredGame | 268 | UDisSeqAct_ObjectiveAction |
| UDisDLC06SeqAct_VectorAdd | DishonoredGame | 284 | UDisDLC06SeqAct_VectorBinOp |
| UDisDLC06SeqAct_VectorBinOp | DishonoredGame | 284 | USequenceAction |
| UDisDLC06SeqAct_VectorNormalize | DishonoredGame | 272 | USequenceAction |
| UDisDLC06SeqAct_VectorScale | DishonoredGame | 276 | USequenceAction |
| UDisDLC06SeqAct_VectorSubtract | DishonoredGame | 284 | UDisDLC06SeqAct_VectorBinOp |
| UDisDLC06SeqEvent_AssassinSummoned | DishonoredGame | 272 | UDisSeqEvent_PlayerEvent |
| UDisDLC06SeqEvent_Attention | DishonoredGame | 272 | USequenceEvent |
| UDisDLC06SeqEvent_AttentionDecreasedTo | DishonoredGame | 272 | UDisDLC06SeqEvent_Attention |
| UDisDLC06SeqEvent_AttentionIncreasedTo | DishonoredGame | 272 | UDisDLC06SeqEvent_Attention |
| UDisDLC06SeqEvent_DropAssassinate | DishonoredGame | 272 | UDisSeqEvent_PlayerEvent |
| UDisDLC06SeqEvent_Landed | DishonoredGame | 272 | UDisSeqEvent_PlayerEvent |
| UDisDLC06SeqEvent_NPCPutPocketed | DishonoredGame | 272 | USequenceEvent |
| UDisDLC06StateNPCSmokeBombed | DishonoredGame | 140 | UStateNPCMasterPlayAnim |
| UDisDLC06Tweaks_AIBehavior_AssassinSalute | DishonoredGame | 176 | UDisTweaks_AIBehavior |
| UDisDLC06Tweaks_AIBehavior_ButcherCombat | DishonoredGame | 216 | UDisTweaks_AIBehavior_CombatMelee |
| UDisDLC06Tweaks_AIBehavior_SummonedAssassinIdle | DishonoredGame | 168 | UDisTweaks_AIBehavior |
| UDisDLC06Tweaks_AIBrainProcess_SummonedAssassin | DishonoredGame | 148 | UDisTweaks_AIBrainProcess |
| UDisDLC06Tweaks_ArcMine | DishonoredGame | 284 | UDisTweaks_SpringRazor |
| UDisDLC06Tweaks_ArcMinePlaced | DishonoredGame | 340 | UDisTweaksBase |
| UDisDLC06Tweaks_ArcMineStun | DishonoredGame | 284 | UDisDLC06Tweaks_ArcMine |
| UDisDLC06Tweaks_Arrow_Explosive | DishonoredGame | 336 | UDisTweaks_Arrow_Explosive |
| UDisDLC06Tweaks_AssassinNPCPawn | DishonoredGame | 932 | UDisDLC06Tweaks_NPCPawn |
| UDisDLC06Tweaks_ButcherNPCPawn | DishonoredGame | 940 | UDisDLC06Tweaks_NPCPawn |
| UDisDLC06Tweaks_DarkVision | DishonoredGame | 772 | UDisTweaks_DarkVision |
| UDisDLC06Tweaks_Effects | DishonoredGame | 280 | UDisTweaksBase |
| UDisDLC06Tweaks_Explosion | DishonoredGame | 308 | UDisTweaks_Explosion |
| UDisDLC06Tweaks_GrenadeComponent | DishonoredGame | 172 | UDisTweaks_GrenadeComponent |
| UDisDLC06Tweaks_NPCAttachment | DishonoredGame | 288 | UDisTweaks_NPCAttachment |
| UDisDLC06Tweaks_NPCFireSaw | DishonoredGame | 1192 | UDisTweaks_ProjectileAttack |
| UDisDLC06Tweaks_NPCPawn | DishonoredGame | 928 | UDisTweaks_NPCPawn |
| UDisDLC06Tweaks_NPCPawn_Shared | DishonoredGame | 148 | UDisTweaksBase |
| UDisDLC06Tweaks_NPCSawCut | DishonoredGame | 468 | UDisTweaks_MeleeAttackNPCBase |
| UDisDLC06Tweaks_NPCWitchScream | DishonoredGame | 228 | UDisTweaks_ItemContext |
| UDisDLC06Tweaks_NPCWitchTeleport | DishonoredGame | 272 | UDisTweaks_ItemContext |
| UDisDLC06Tweaks_PlayerPawn | DishonoredGame | 1892 | UDisTweaks_PlayerPawn |
| UDisDLC06Tweaks_PlayerPawn_Attributes | DishonoredGame | 2404 | UDisTweaks_PlayerPawn_Attributes |
| UDisDLC06Tweaks_Projectile_SmokeGrenade | DishonoredGame | 276 | UDisTweaks_Projectile_Grenade |
| UDisDLC06Tweaks_SummonAssassin | DishonoredGame | 516 | UDisTweaks_ActivePowerBase |
| UDisDLC06Tweaks_SummonedAssassinNPCPawn | DishonoredGame | 932 | UDisDLC06Tweaks_AssassinNPCPawn |
| UDisDLC06Tweaks_Transversal | DishonoredGame | 584 | UDisTweaks_Blink |
| UDisDLC06Tweaks_VulnerabilityEx | DishonoredGame | 148 | UDisTweaksBase |
| UDisDLC06Tweaks_WepCrossbow | DishonoredGame | 312 | UDisTweaks_WepCrossbow |
| UDisDLC06Tweaks_WepGrenade | DishonoredGame | 296 | UDisTweaks_WepGrenade |
| UDisDLC06Tweaks_WepGrenade_Attributes | DishonoredGame | 252 | UDisTweaks_WeaponRanged_Attributes |
| UDisDLC06Tweaks_WepSaw | DishonoredGame | 360 | UDisTweaks_WeaponRanged |
| UDisDLC06Tweaks_WepWitchHand | DishonoredGame | 296 | UDisTweaks_WeaponRanged |
| UDisDLC06WepCrossbow | DishonoredGame | 320 | UDisWepCrossbow |
| UDisDLC06WepSaw | DishonoredGame | 320 | UDishonoredWeapon_Ranged |
| UDisDLC06WepWitchHand | DishonoredGame | 312 | UDishonoredWeapon_Ranged |
| UDisDLC07AISubStateDoPullSpell | DishonoredGame | 320 | UDisAISubStateWithDesires |
| UDisDLC07AISubStateTentacleAttack | DishonoredGame | 260 | UDisAISubState |
| UDisDLC07AISubStateTentacleSpawn | DishonoredGame | 208 | UDisAISubState |
| UDisDLC07ActivePowerComponent_Pull | DishonoredGame | 188 | UDishonoredActivePowerComponent |
| UDisDLC07AnimNodeBlendByPull | DishonoredGame | 256 | UAnimNodeBlendList |
| UDisDLC07BehaviorAssassinVsRiverKrustCombat | DishonoredGame | 196 | UDisAIBehaviorWithDesires |
| UDisDLC07BehaviorTentacleIdle | DishonoredGame | 348 | UDisAIBehaviorWithDesires |
| UDisDLC07CheatManager | DishonoredGame | 264 | UDisDLC06CheatManager |
| UDisDLC07ContactType_SkeletalMovable | DishonoredGame | 84 | UDisContactType |
| UDisDLC07ContactType_TentacleAOE | DishonoredGame | 84 | UDisContactType |
| UDisDLC07DamageType_DeadEelsBottleExplosion | DishonoredGame | 136 | UDishonoredDamageType |
| UDisDLC07DamageType_Pull | DishonoredGame | 136 | UDishonoredDamageType |
| UDisDLC07DamageType_TentacleAOE | DishonoredGame | 136 | UDishonoredDamageType |
| UDisDLC07ItemContext_NPCPullSpell | DishonoredGame | 196 | UDisItemContext |
| UDisDLC07ItemContext_NPCPullSpellCoupDeGrace | DishonoredGame | 264 | UDisItemContext_MeleeAttackNPCBase |
| UDisDLC07ItemContext_NPCSummonTentacle | DishonoredGame | 176 | UDisItemContext |
| UDisDLC07ItemContext_NPCTentacleGrab | DishonoredGame | 292 | UDisItemContext_Minigame |
| UDisDLC07MoviePlayerHUD | DishonoredGame | 1452 | UDisDLC06MoviePlayerHUD |
| UDisDLC07MoviePlayerJournal | DishonoredGame | 928 | UDisGFxMoviePlayerJournal |
| UDisDLC07MoviePlayerMainMenu | DishonoredGame | 792 | UDisDLC06MoviePlayerMainMenu |
| UDisDLC07NavMeshGoal_CombatVsRiverKrustPosition | DishonoredGame | 2116 | UNavMeshPathGoalEvaluator |
| UDisDLC07NavMeshGoal_GravehoundSpawnPosition | DishonoredGame | 2716 | UNavMeshPathGoalEvaluator |
| UDisDLC07Notify_PullIn | DishonoredGame | 60 | UAnimNotify |
| UDisDLC07Notify_PullOut | DishonoredGame | 60 | UAnimNotify |
| UDisDLC07Notify_WitchScreamDamage | DishonoredGame | 60 | UAnimNotify |
| UDisDLC07PullTargetInterface | DishonoredGame | 56 | UInterface |
| UDisDLC07PullerInterface | DishonoredGame | 56 | UInterface |
| UDisDLC07SeqAct_CheckPlayerInventoryItem | DishonoredGame | 252 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqAct_CheckPlayerUpgrade | DishonoredGame | 252 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqAct_ClearPersonalRelationship | DishonoredGame | 264 | USequenceAction |
| UDisDLC07SeqAct_ClearTutorials | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqAct_EndDLC07 | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqAct_IgnoreIdealMaximumCountForCorpseCleanup | DishonoredGame | 248 | USequenceAction |
| UDisDLC07SeqAct_ImportTransitionPlayerData | DishonoredGame | 252 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqAct_NPCEnrage | DishonoredGame | 248 | USequenceAction |
| UDisDLC07SeqAct_PermaKill | DishonoredGame | 248 | USequenceAction |
| UDisDLC07SeqAct_SetBoneCharmCrackedEffect | DishonoredGame | 252 | USequenceAction |
| UDisDLC07SeqAct_SetPlayerArmsMesh | DishonoredGame | 252 | USequenceAction |
| UDisDLC07SeqAct_SetPullOverride | DishonoredGame | 248 | USequenceAction |
| UDisDLC07SeqAct_ToggleSaving | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDisDLC07SeqEvent_GrabbedByTentacle | DishonoredGame | 272 | USequenceEvent |
| UDisDLC07SeqEvent_GrannyRecipe | DishonoredGame | 280 | USequenceEvent |
| UDisDLC07SeqEvent_GravehoundSkullDestroyed | DishonoredGame | 272 | USequenceEvent |
| UDisDLC07SeqEvent_HostileToPlayer | DishonoredGame | 272 | USequenceEvent |
| UDisDLC07SeqEvent_NPCPulled | DishonoredGame | 272 | USequenceEvent |
| UDisDLC07SeqEvent_PlayerFiredWeapon | DishonoredGame | 276 | UDisSeqEvent_PlayerEvent |
| UDisDLC07StateNPCCoughing | DishonoredGame | 116 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCMasterDead_Pulled | DishonoredGame | 180 | UStateNPCMasterDead |
| UDisDLC07StateNPCMasterPullSpell | DishonoredGame | 116 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCMasterPulled | DishonoredGame | 172 | UStateNPCInstigatedMasterAction |
| UDisDLC07StateNPCTentacleAttractNPC | DishonoredGame | 180 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCTentacleAttractPlayer | DishonoredGame | 120 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCTentacleDespawn | DishonoredGame | 116 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCTentacleGrabWindup | DishonoredGame | 116 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCTentacleLoseGrab | DishonoredGame | 116 | UStateNPCMasterPlayAnim |
| UDisDLC07StateNPCTentaclePulled | DishonoredGame | 124 | UStateNPCMasterPlayAnim |
| UDisDLC07TentacleGlobalManager | DishonoredGame | 92 | UObject |
| UDisDLC07TentacleSpawnerComponent | DishonoredGame | 464 | UPrimitiveComponent |
| UDisDLC07Tweaks_AIBehavior_AssassinVsRiverKrustCombat | DishonoredGame | 204 | UDisTweaks_AIBehavior |
| UDisDLC07Tweaks_AIBehavior_TentacleIdle | DishonoredGame | 168 | UDisTweaks_AIBehavior |
| UDisDLC07Tweaks_AIBrain_Tentacle | DishonoredGame | 204 | UDisTweaks_AIBrain |
| UDisDLC07Tweaks_AISubState_DoPullSpell | DishonoredGame | 140 | UDisTweaks_AISubState |
| UDisDLC07Tweaks_AISubState_TentacleAttack | DishonoredGame | 444 | UDisTweaks_AISubState |
| UDisDLC07Tweaks_AISubState_TentacleSpawn | DishonoredGame | 144 | UDisTweaks_AISubState |
| UDisDLC07Tweaks_AssassinNPCPawn | DishonoredGame | 1008 | UDisDLC07Tweaks_NPCPawn |
| UDisDLC07Tweaks_GFxMoviePlayerJournal | DishonoredGame | 264 | UDisTweaks_GFxMoviePlayerJournal |
| UDisDLC07Tweaks_Gravehound | DishonoredGame | 1164 | UDisDLC07Tweaks_NPCPawn |
| UDisDLC07Tweaks_GravehoundSkull | DishonoredGame | 284 | UDisTweaks_Movable |
| UDisDLC07Tweaks_NPCPawn | DishonoredGame | 1004 | UDisDLC06Tweaks_NPCPawn |
| UDisDLC07Tweaks_NPCPullSpell | DishonoredGame | 308 | UDisTweaks_ItemContext |
| UDisDLC07Tweaks_NPCPullSpellCoupDeGrace | DishonoredGame | 408 | UDisTweaks_MeleeAttackNPCBase |
| UDisDLC07Tweaks_NPCSummonTentacle | DishonoredGame | 212 | UDisTweaks_ItemContext |
| UDisDLC07Tweaks_NPCTentacleGrab | DishonoredGame | 476 | UDisTweaks_Minigame |
| UDisDLC07Tweaks_PlayerPawn | DishonoredGame | 2376 | UDisDLC06Tweaks_PlayerPawn |
| UDisDLC07Tweaks_PlayerPawn_Attributes | DishonoredGame | 2432 | UDisDLC06Tweaks_PlayerPawn_Attributes |
| UDisDLC07Tweaks_Pull | DishonoredGame | 560 | UDisTweaks_ActivePowerBase |
| UDisDLC07Tweaks_RiverKrust | DishonoredGame | 1216 | UDisTweaks_RiverKrust |
| UDisDLC07Tweaks_RiverKrustProjectile | DishonoredGame | 316 | UDisTweaks_Arrow |
| UDisDLC07Tweaks_SummonedAssassinNPCPawn | DishonoredGame | 1008 | UDisDLC07Tweaks_AssassinNPCPawn |
| UDisDLC07Tweaks_TentaclePawn | DishonoredGame | 1024 | UDisDLC07Tweaks_NPCPawn |
| UDisDLC07Tweaks_WepTentacle | DishonoredGame | 268 | UDisTweaks_InventoryItem |
| UDisDLC07Tweaks_WhaleBoneCharmCracked | DishonoredGame | 220 | UDisTweaks_PickupBase |
| UDisDLC07Tweaks_WhaleBoneCharmCrackedList | DishonoredGame | 164 | UDisTweaksBase |
| UDisDLC07WepTentacle | DishonoredGame | 284 | UDishonoredWeapon |
| UDisDLCAssassinInterface | DishonoredGame | 56 | UInterface |
| UDisDLCSummonedAssassinInterface | DishonoredGame | 56 | UInterface |
| UDisDamageType_ExplosiveBullet | DishonoredGame | 136 | UDishonoredDamageType_Bullet |
| UDisDamageType_RiverKrustSpit | DishonoredGame | 136 | UDishonoredDamageType |
| UDisItemContext_DLC05_DropAssassinate | DishonoredGame | 208 | UDisItemContext_DropAssassinate |
| UDisItemContext_DLC05_NPCTeleportSpell | DishonoredGame | 240 | UDisItemContext_NPCTeleportSpell |
| UDisNPCAttachmentOwnerInterface | DishonoredGame | 56 | UInterface |
| UDisOnlineLeaderboards | Engine | 56 | UObject |
| UDisSeqAct_CameraShake | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_AIGoToActor | DishonoredGame | 284 | UDisSeqAct_AIGoToActor |
| UDisSeqAct_DLC05_DarkVision | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_DialogScriptedChoice | DishonoredGame | 300 | UDisSeqAct_DialogScriptedChoice |
| UDisSeqAct_DLC05_DollCollected | DishonoredGame | 248 | USequenceAction |
| UDisSeqAct_DLC05_FallSpeedFX | DishonoredGame | 316 | USequenceAction |
| UDisSeqAct_DLC05_FlyingKillCam | DishonoredGame | 276 | USequenceAction |
| UDisSeqAct_DLC05_ForceKillCam | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_GetAmmoInfo | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_GetDeathInfo | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_Heal | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_InfiniteAmmo | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_Interp | DishonoredGame | 524 | USeqAct_Interp |
| UDisSeqAct_DLC05_MarkerControl | DishonoredGame | 264 | USequenceAction |
| UDisSeqAct_DLC05_ModifyMultiplier | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_NpcWave | DishonoredGame | 308 | USequenceAction |
| UDisSeqAct_DLC05_PlayerBusted | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_PlayerResurrect | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_RemoveArrows | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_SendChallengeEvent | DishonoredGame | 256 | USequenceAction |
| UDisSeqAct_DLC05_SetBendTimeDesaturation | DishonoredGame | 256 | USequenceAction |
| UDisSeqAct_DLC05_SetDifficulty | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_SetMysteryFoe | DishonoredGame | 256 | USequenceAction |
| UDisSeqAct_DLC05_SetPowerWheelShortcuts | DishonoredGame | 264 | USequenceAction |
| UDisSeqAct_DLC05_SetScoringRules | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_SetStoryGroup | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_ShowCountdown | DishonoredGame | 264 | USequenceAction |
| UDisSeqAct_DLC05_ShowEquipmentUnlock | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_ShowHUDItem | DishonoredGame | 260 | USequenceAction |
| UDisSeqAct_DLC05_ShowPhaseResults | DishonoredGame | 276 | USequenceAction |
| UDisSeqAct_DLC05_ShowWaveNumber | DishonoredGame | 268 | USequenceAction |
| UDisSeqAct_DLC05_StreamLevels | DishonoredGame | 284 | USeqAct_MultiLevelStreaming |
| UDisSeqAct_DLC05_Teleport | DishonoredGame | 264 | USequenceAction |
| UDisSeqAct_DLC05_Timer | DishonoredGame | 280 | USequenceAction |
| UDisSeqAct_DLC05_TriggerCustomScoringRule | DishonoredGame | 256 | USequenceAction |
| UDisSeqAct_DLC05_UnlockAchievement | DishonoredGame | 252 | USequenceAction |
| UDisSeqAct_DLC05_WobWave | DishonoredGame | 252 | USequenceAction |
| UDisSeqCond_DLC05_IsDollAlreadyCollected | DishonoredGame | 224 | USequenceCondition |
| UDisSeqCond_DLC05_IsExpertMode | DishonoredGame | 224 | USequenceCondition |
| UDisSeqEvent_AttackedByRats | DishonoredGame | 272 | USequenceEvent |
| UDisSeqEvent_DLC05_Challenge | DishonoredGame | 272 | USequenceEvent |
| UDisSeqEvent_DLC05_DropKilled | DishonoredGame | 272 | USequenceEvent |
| UDisSeqEvent_DLC05_PlayerDeath | DishonoredGame | 272 | USequenceEvent |
| UDisSeqEvent_PlayerInventoryChanged | DishonoredGame | 280 | UDisSeqEvent_PlayerEvent |
| UDisSeqEvent_RiverKrustLooted | DishonoredGame | 272 | USequenceEvent |
| UDisSeqVar_DLC05_Multiplier | DishonoredGame | 160 | USeqVar_Float |
| UDisSeqVar_DLC05_Score | DishonoredGame | 156 | USeqVar_Int |
| UDisTweaks_AIBehavior_Inhibited | DishonoredGame | 168 | UDisTweaks_AIBehavior |
| UDisTweaks_DLC05MoviePlayerChallengeMenu | DishonoredGame | 204 | UDisTweaks_GFxMoviePlayerBase |
| UDisTweaks_DLC05MoviePlayerHUD | DishonoredGame | 260 | UDisTweaks_GFxMoviePlayerBase |
| UDisTweaks_DLC05_DropAssassinate | DishonoredGame | 1348 | UDisTweaks_DropAssassinate |
| UDisTweaks_DLC05_NPCAssassinHand | DishonoredGame | 296 | UDisTweaks_NPCAssassinHand |
| UDisTweaks_DLC05_NPCTeleportSpell | DishonoredGame | 268 | UDisTweaks_NPCTeleportSpell |
| UDisTweaks_DLC05_SkeletalBreakable | DishonoredGame | 200 | UDisTweaks_SkeletalBreakable |
| UDisTweaks_DLC05_WepSword | DishonoredGame | 268 | UDisTweaks_WepSword |
| UDisTweaks_DLC07MoviePlayerHUD | DishonoredGame | 2352 | UDisTweaks_GFxMoviePlayerHUD |
| UDisTweaks_NPCAttachment | DishonoredGame | 284 | UDisTweaks_Movable |

The 19 additions without a `DLC0x` prefix are new base/interface classes used by the DLC code
(`UDisBehaviorAttentionBase`, `UDisBehaviorInhibited`, `ADisNPCAttachment`, `UDisDLCAssassinInterface`, ...) plus a few
gameplay additions (`ADisFog`, `ADisInterpActor`, `UDisOnlineLeaderboards`, `UDisSeqAct_CameraShake`, ...).

### Only in 2012 (26)

| class | package | size_2012 | super_2012 |
|---|---|---|---|
| AFluidInfluenceActor | Engine | 608 | AActor |
| AFluidSurfaceActor | Engine | 592 | AActor |
| AFluidSurfaceActorMovable | Engine | 592 | AFluidSurfaceActor |
| AFoliageActor | Engine | 592 | AStaticMeshActor |
| AFoliageFactory | Engine | 672 | AVolume |
| AInteractiveFoliageActor | Engine | 688 | AFoliageActor |
| UActorFactoryFoliage | Engine | 104 | UActorFactoryStaticMesh |
| UActorFactoryInteractiveFoliage | Engine | 104 | UActorFactoryStaticMesh |
| UDEPRECATED_DisSeqAct_AddHeartTarget | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDEPRECATED_DisSeqAct_ClearHeartTargets | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDEPRECATED_DisSeqAct_RemoveHeartTarget | DishonoredGame | 248 | UDisSeqAct_PlayerAction |
| UDEPRECATED_DisTweaks_GFxMoviePlayerGameOver | DishonoredGame | 184 | UDisTweaks_GFxMoviePlayerBase |
| UDEPRECATED_PBRuleNodeBase | Engine | 56 | UObject |
| UDisGameplayEventsWriter | DishonoredGame | 320 | UGameplayEventsWriter |
| UFluidInfluenceComponent | Engine | 560 | UPrimitiveComponent |
| UFluidSurfaceComponent | Engine | 688 | UPrimitiveComponent |
| UFoliageComponent | Engine | 592 | UPrimitiveComponent |
| UGameStateObject | GameFramework | 92 | UObject |
| UGameStatsAggregator | GameFramework | 244 | UGameplayEventsHandler |
| UGameplayEvents | Engine | 316 | UObject |
| UGameplayEventsHandler | Engine | 84 | UObject |
| UGameplayEventsReader | Engine | 328 | UGameplayEvents |
| UGameplayEventsWriter | Engine | 320 | UGameplayEvents |
| UGenericParamListStatEntry | Engine | 64 | UObject |
| UInteractiveFoliageComponent | Engine | 592 | UStaticMeshComponent |
| UOnlineGameplayEvents | Engine | 148 | UObject |

The 2012-only classes are the fluid-surface / foliage feature set (removed from the retail Engine), the
`GameplayEvents` recording stack, and four `DEPRECATED_` script-object classes.

## Class-name tables per exe

`native_class_sizes.csv` is the full union of both exes' native class-name tables (2,883 rows): a class is present
in the 2012 exe iff `size_2012` is filled, in the 2013 exe iff `size_2013` is filled. Per-exe columns:
`registrant_rva_*` (2012: `GetPrivateStaticClass<X>`, 2013: `X::StaticClass`), `staticclass_rva_*`,
`init_rva_*` (`InitializePrivateStaticClass<X>`), `container_rva_*` (in-place `UClass` storage; in 2013 this is
also the 12-dword descriptor), `ctor_rva_*` (`X::InternalConstructor`), `super_*`, `within_2013`, `config_2013`,
`cast_flags_*`, `other_flags_*`, `pdb_size_2012` (the `sizes.csv` value the 2012 number was validated against).
