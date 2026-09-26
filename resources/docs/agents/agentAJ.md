# Agent AJ — DishonoredGame infrastructure: tweaks, native FSM, inventory, the 10 missing serializers (2026-09-26)

Package AJ of `resources/docs/PHASE6.md`. The retail 2013 exe is the target: every "2013 rva" is `retail2013_agentAJ.i64`
(copy of `retail2013_named.i64`), "2012 rva" is `shipping2012_agentAJ.i64`, used as the readable version of the same
function. Decompiles: `resources/reference/decomp/agentAJ/{r13,s12}` (git-ignored, 923 functions).
Status rows: `resources/docs/agents/agentAJ_status.csv` (74 rows, all `written`).

*Power cut during the wave:* eight files in `resources/reference/decomp/agentAJ/r13` were zero-filled (found by AG).
They were the `UDisGlobalMusicManager::TickCallback_*` bodies, `ADishonoredKActor::DestroyIfPlayerCantSeeMe`,
`ADisWatchTower::OnSetDisposition`, `ADisRiverKrust::OnRiverKrustSpitAtTarget`, `ADishonoredNPCPawn::SetDesiredRotation`
and `sub_B70AA0`. **None of them was used for a port** — they read as empty when I looked at them, which is why those
natives are in the "not ported" list. All eight have been re-decompiled (`build/agentAJ_work/redecomp.log`); the whole
`r13` directory is now clean (0 of 2,948 files damaged), and the music-manager bodies the restored files revealed are
written up as the cheapest next batch.

## Result

| Package step | State |
|---|---|
| 1. The 10 `Serialize` overrides without a CppText hook (agent X's `needed` list) | **done**, all 10 + 3 helpers (`FDisRelationshipOverrideInfo::Serialize`, `UDisTweaksBase::Serialize`/`PostLoad`) |
| 2. Tweak interface: `IDisTweaksInterface` / `IDisEngineTweaksInterface` vtables, `FSpawnActor_TweakObj::DoInit` before `PostBeginPlay`, `SetTweaks` on spawned actors | **done**, including the retail call order: `SpawnActor_Derived` passes `FSpawnActor_TweakObj(this)` to the init-functor parameter AI landed mid-wave, so `DoInit` (`SetTweaks`) runs before the begin-play chain |
| 3. Native FSM (`InitFSM`, `RequestStateChange`, `DemandStateChange`, `DoStateChange`, `TickStateMachine`, …) + the transition logic; `UDishonoredInventory` core | **done** for the FSM (18 functions incl. `UDishonoredNativeState` and `UDisNativeStateTransitionLogic`); inventory: the abstract-item / ammo / elixir core (12 functions) — equipping, slots and item spawning are **not** ported (they need `UDisItemContext` + the item tweaks) |
| 4. `ADishonoredGameInfo` (`GameEnding`, `PostBeginPlay` → `InitGlobalManagers`, `Pre/PostCommitMapChange`, `Tick`), NPC pawn (19) / NPC controller (17) / pawn (17) stubs | **partial**: the pawn's tweak interface (`GetTweaks_Derived`, `SetTweaks_Derived`, `ApplyTweakChanges_Derived`, `ApplyTweakChanges_Body`), `PossessedBy` and the 5 Kismet inventory natives are ported; `ADishonoredGameInfo`'s manager/tick/map-change functions and the NPC natives are **not** (reasons below) |
| 5. Natives moved off the warn-once stub | **7** `ADishonoredPawn` natives (ported list final): the five Kismet inventory ones (`execOnAddAbstractItem`, `execOnRemoveAbstractItem`, `execOnGetAbstractItemQuantity`, `execOnModifyElixirCount`, `execOnModifyAmmo`) plus `execChooseAndTriggerDeathEvent_Native` and `execPlayDying_Native` (the death chain AE's `APawn::Died` raises) — the `≥ 120` target of the package is **not met**; see "Why 7 and not 120" |

**Total: 74 functions written** (10 serializers + 3 serializer helpers + 17 tweak-interface/fallback functions +
18 FSM functions + 5 pawn tweak functions + 7 pawn natives (exec + virtual each) + 12 inventory functions + 2 more),
every one with its 2013 rva in `agentAJ_status.csv`.

## Which build the numbers come from (read this)

All final numbers come from **HEAD `f8dfe78`** (wave-4 packages AK, AD, AI, AE, AG, AH all merged):

- **`build/agentAJ`** = snapshot worktree `build/agentAJ_wt` (detached at `f8dfe78`) + the 35 AJ files, built with
  `build/agentAJ_build.cmd` (all four module options, `DISHONORED_SDK_LAYOUT_CHECKS=ON`,
  `FETCHCONTENT_FULLY_DISCONNECTED=ON`): **0 errors, 0 unresolved symbols**, 12,502 layout asserts, 0 pending
  (`build/agentAJ_build14.log`). The only non-AJ change in it is the snapshot-only anim-event guard of hand-over 4.
- **The shared working tree**: the AJ units compiled there with **0 errors** while its generated headers were current
  (`build/agentAJ3_objs2.log`, 19 object files through `build/agentAJ3_objs.cmd`). Repeating it now gives 1,938 errors,
  **none of them in an AJ file**: the shared tree's *generated* DishonoredGame headers still predate AI's move of the 19
  Arkane classes into Engine (950 errors in `DishonoredGameEngineShims.h` — `FEditorMatineeData` /
  `FFaceToControlTrackKey` redefined, `UAudioSystem` base undefined — and 741 in `DishonoredGameLayouts.h`). That is the
  regeneration the coordinator does at this merge; my snapshot at the same HEAD **with** the module regenerated
  (`build/agentAJ_work/apply_gen.py`) builds and links cleanly, which is the same state the shared tree reaches after it.
  My `patch_shared.py` / `gen_sources.py` edits to the shared tree's generated files (the 15 `#include` lines and the
  `Sources.cmake` exclude list) are therefore transient: regeneration reproduces them, because both generator hooks are
  now in HEAD (AI committed them in `03f5335`).

## The 10 serializers (agent X's `needed` rows, all closed)

| Class | 2013 rva | 2012 rva | What the body does |
|---|---|---|---|
| `ADisMovableLimb::Serialize` | 0x642c30 | 0x6910d0 | `AActor::Serialize` + `FDisRelationshipOverrideInfo::Serialize(m_RelationshipOverrideInfo)` |
| `ADisWatchTower::Serialize` | 0x6195c0 | 0x66a4d0 | same, `m_PersonalRelationships` |
| `ADishonoredPawn::Serialize` | 0x748ee0 | 0x78e1f0 | same, `m_PersonalRelationships` |
| `UDisAIBlackboard::Serialize` | 0x72ffc0 | 0x792c30 | counting-memory only: the object and every `FAIBlackboardRecord` (`GetSize`+`GetExtraSize`). `DISHONORED(bringup)`: the record classes are not ported (`m_Records` holds `FPointer`s), so only the object is counted |
| `UDisDialogTree::Serialize` | 0x88b210 | 0x8f97f0 | a save that is not a transaction clears `m_bNeedsResaving` |
| `UDisTweaks_DefenceTower::Serialize` | 0x6208a0 | 0x646bb0 | version ≤ 3, loading, not a class default, not the commandlet → `m_FriendlyFactions.AddItem(m_pGuardFaction)` |
| `UDisTweaks_SkeletalBreakable::Serialize` | 0x618c30 | 0x669cd0 | licensee < 24 → break-step upgrade through a `UDisSkeletalBreakStepsInterface` virtual. Retail content is licensee 30 (`UnObjVer.cpp`), so the branch is dead; ours warns instead |
| `UDisTweaks_StaticBreakable::Serialize` | 0x6209a0 | 0x646c00 | licensee < 24 → every `m_Steps` entry's `m_AINoiseContext = 4` |
| `UDisTweaks_UsableObject::Serialize` | 0x64e170 | 0x671590 | licensee < 29 → `m_bUsableWhileCarryingSomething = m_bUsableWhileCarryingCorpse`, and the fallback skip moves with it |
| `UDishonoredGlobalAIManager::Serialize` | 0x860430 | 0x8cecd0 | object-reference collectors see the `m_Corpses` keys |
| *(helper)* `FDisRelationshipOverrideInfo::Serialize` | 0x85e3d0 | 0x89f900 | the `m_FactionOverrideMap` keys, collectors only (`disglobalenums.cpp:29`) |
| *(helper)* `UDisTweaksBase::Serialize` | 0x882e70 | 0x8f0600 | a non-cooker save records `m_VersionNum` and the `m_BaseClassVersions` chain |
| *(helper)* `UDisTweaksBase::PostLoad` | 0x882a70 | 0x8ea190 | `m_bBeenLocalized`, then `ApplyFallbackChain` for instances |

**Verification** (agent AD's `-loadall` with **my** exe, so my serializers run):
`--extra-args="-loadall=@build/agentAJ/tower.txt"` over the nine `L_Tower_P` packages → see "Accept" below.

## Tweak interface and the fallback chain

The vtables come from the 2012 PDB (`IDisTweaksInterface_vtbl` 28 bytes: `~`, `GetUObjectInterfaceDisEngineTweaksInterface`,
`HasTweaks_Derived`, `GetUObjectInterfaceDisTweaksInterface`, `GetTweaks_Derived`, `SetTweaks_Derived`,
`ApplyTweakChanges_Derived`; `IDisEngineTweaksInterface_vtbl` 12 bytes) and the bodies from the 2013 decompiles:
`SetTweaks` 0x661fe0, `ApplyTweakChanges` 0x882a30, `IsTweaksValid` 0x5fb110, `HasTweaks_Derived` 0x873d60,
`IDisEngineTweaksInterface::HasTweaks` 0x11b1d0. `UDisTweaksBase` got its retail vtable tail
(`GetSpawnedObjectClass` +300, `ApplyFallbackChain_Derived`, `GatherTweakChildren_Derived`, `FixupDefaults_Derived`,
`SpawnActor_Derived` +316) plus `ApplyFallbackChain` (0x87ed80), `ApplyFallbackChain_Struct` (2012 0x8e0500),
`InvalidateFallback_Recurse` (0x87ba20), `SkipFallback` (0x87f3e0), `FindFallbackSkip` (0x86f2e0),
`IsFallbackDerivedFrom` (0x86f360, seek-free compares the **cooked** fallback names), `IsFallbackRelated` (0x86f5c0).

- The fallback chain copies the properties whose `PropertyFlags` carry Arkane's tweak-fallback bit
  `0x0000400000000000` (`CPF_DisTweakFallback` in `DishonoredGameNative.h`; the reference headers have no name for it),
  recursing into struct properties per array element, and honours the `m_FallbackSkip` list.
- `FSpawnActor_TweakObj::DoInit` (0x885650) resolves the actor's `IDisTweaksInterface` through
  `GetInterfaceAddress(UDisTweaksInterface::StaticClass())` and calls `SetTweaks`.
- Not ported: the `GatherTweakChildren_Derived` overrides of the tweak subclasses (so only top-level tweak objects are
  chained), the editor-only `EditConditionAskObj_*` slots, and `FSpawnNPCPawn_TweakObj`'s spawner member (not in the
  retail SDK dump).

## Native FSM

`UDishonoredNativeStateMachine`: `InitFSM` 0x67ba50 (state map, the default `FDisNativeStateParam` kept as raw bytes,
first transition), `DestroyFSM` (2012 0x6a89e0, unmatched in the 2013 db), `BuildNativeStateMap` 0x67a8f0,
`RequestStateChange` 0x674fa0, `DemandStateChange` 0x672190, `DoStateChange` 0x65f8c0, `TickStateMachine` 0x66de60,
`CanTransitionTo` 0x674f20, `ClearPendingState` 0x65fa00, `IsCurState` 0x666df0, `LockFSM` 0x65f9b0,
`OnPawnShutDown` 0x666e40, `GetAllStateIDs` 0x670e50, `DebugStoreRejectedStateInfo` 0x666d70.
`UDishonoredNativeState`: `RequestStateExit` 0x662750, `RequestStateExit_Derived` 0x674ee0, `DemandStateChange` 0x674eb0,
plus the 20 empty base virtuals in retail vtable order (`OnEnterState` … `RequestStateExit_Derived`, 2012
`UDishonoredNativeState_vtbl` +288…+364).
`UDisNativeStateTransitionLogic`: `CanTransition` 0x672720 and `InitTransitionLogic` 0x67bbe0 (the designer's
`m_Transitions` become a per-state-pair map, resolved along both class chains to `UObject`, default
`EStateTransitionLogicResult_Call`).

Details worth keeping: a state change moves the *firing object* of the leaving and entering states and notifies
`OnFiringObjectChanged` **after** the parameter's `OnPending`; rejected requests are recorded only while the player's
HUD has `bShowDebugInfo` (`ADishonoredPlayerController::s_pInstance->myHUD`), and the record list ages out by
`m_fDebug_KeepFailedStateTime`.

## Inventory core and the pawn's Kismet natives

`UDishonoredInventory` (12 functions): `GetEquippedItem` 0x804bd0, `AddAbstractItem` 0x80b780,
`RemoveAbstractItem` 0x80f8b0, `HideAbstractItem` 0x80b850, `GetAbstractItemQuantity` 0x80b890,
`GetAmmoInfo` 0x8053f0, `HasAmmo` 0x805200, `AddAmmo` 0x8052e0, `SetAmmo` 0x805380, `ConsumeAmmo` 0x805260,
`AddElixir` 0x804e10, `SetElixirCount` 0x804ec0. Elixirs exist on the player pawn only (retail tests
`m_ActorTypeFlags == 36` in `AddElixir` and the class in `SetElixirCount` — both kept as they are) and clamp to
`UDisTweaks_PlayerPawn::m_nMaxHealthElixir` / `m_nMaxManaElixir`.

`ADishonoredPawn` (5 natives, exec + virtual each, vtable +1352…+1368): `OnAddAbstractItem` 0x74a560/0x5eca70,
`OnRemoveAbstractItem` 0x74a5d0/0x5ecad0, `OnGetAbstractItemQuantity` 0x74a610/0x5ecb30,
`OnModifyElixirCount` 0x74a650/0x5ee590, `OnModifyAmmo` 0x751cc0/0x5eca10. Plus the tweak side:
`GetTweaks_Derived` 0x749110, `SetTweaks_Derived` 0x74d350, `ApplyTweakChanges_Body` 0x757060 (collision cylinder +
`MaxStepHeight`), `ApplyTweakChanges_Derived` 0x769720 (anim sets/tree, physics asset, skeletal mesh,
`m_bHasJiggleBones`, faction and story-group tweaks) and `PossessedBy` 0x748f60.

`DISHONORED(bringup)` notes carried in the code: the GFx HUD notification (`UDisGFxMoviePlayerHUD` slot +500), the
equipped-item ammo notification (item slot +396), the infinite-ammo cheat virtual (game-info slot +1060) and the two
`FArkGameEventDispatcher` events (65 abstract item, 57 elixir) are not fired — the dispatcher's event structs are not
ported.

## Why 7 natives and not 120

Of the 275 warn-once stubs in AJ-owned classes, the groups are: **124 AI behaviour callbacks**
(`UDisBehavior*::On/Tick/Refresh/RequestStateExitCallback_*`), **56 other actors/components**, **42 NPC
pawn/controller**, **20 projectile/gadget `TakeDamage`/`BaseChange`**, **18 pawn**, 6 anim nodes, 5 music-manager FSM
ticks, 4 Kismet sequence variables. Their exec wrappers are mechanical, but every retail *body* behind them needs a
subsystem this package does not own and that nothing else in wave 4 ports: `UDishonoredAIBrain` (senses, suspicion,
psychic attention), `UDisAISubProcess`/`UDisAISubState` (the 124 callbacks call `GetSubProcess` /
`EnableSubProcess_Internal` / `ResetSubprocess`), `FArkComponentLocomotion` (move targets), `UDisItemContext` (equip,
melee, use), `FDisAttentionProxy`, the contact system and `DisSaveLoad`. Porting the exec alone would replace a
*documented* warn-once stub with a silent no-op, which is worse than the stub: the map would look healthy while AI and
item behaviour quietly did nothing. I ported the chain that is complete instead (tweaks → FSM → inventory → the pawn's
Kismet natives → the two death-chain natives that `-strictnatives` needed) and left the rest warning. The trivial-native harvest that made AC's numbers large
(247 of 289) is already spent: what remains in DishonoredGame is substantive work.

AC's `build/agentAC_work/triage.csv` rows for the AJ classes (exec rva, vtable slot, callee rva/size) are extracted to
`build/agentAJ_work/my_triage.txt` (295 rows) so the next agent can pick the next batch without redoing the analysis.
The **cheapest next batch** (decompiles already in `resources/reference/decomp/agentAJ/r13`): the five
`UDisGlobalMusicManager::TickCallback_*` natives (`Suspense` / `Exploration` / `RatAttack` share the 16-byte body
2013 rva 0x852bf0, `Combat` 0x852c00, `Chase` 0x852c20) — all one-liners over `ChooseNewState` (0x852a40, 419 bytes),
which needs the five `UDisMusicState_*` classes and the two `FDisMusicState_*_Param` state parameters. They tick on the
map path, so they are worth having early. After them: `ADishonoredKActor::DestroyIfPlayerCantSeeMe` (0x6185f0, 29 bytes),
`ADisWatchTower::OnSetDisposition` (0x619690, 15 bytes) and `ADishonoredPawn::OnSetDisposition` (0x749100, 15 bytes) —
all three need `IDisRelationshipInterface::OnSetDispositionHelper` (0x776910).

## Generated-code plumbing (two generator hooks — overlap with agent AI)

`resources/tools/symbols/gen_classes_header.py` belongs to **AI** this wave, but the AJ ports cannot exist without two
hooks, so they are applied (idempotently, tagged `DISHONORED(written)`, `build/agentAJ_work/patch_shared.py`):

1. **interface cpptext**: `iface_lines` includes `Inc/CppText/<IName>.h` inside a generated interface body when that
   file exists — the same rule AC added for classes. `IDisTweaksInterface` / `IDisEngineTweaksInterface` get their
   retail virtuals that way.
2. **module header**: `sdk_module_header` includes `Inc/<Module>Native.h` before the class headers. That new
   hand-written header holds the native types the cpptext declarations name: `FDisNativeStateParam`,
   `FSpawnActor_TweakObj` / `FSpawnNPCPawn_TweakObj` (+ a temporary `FSpawnActorInitFunctor` base, guarded by
   `DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR` so Engine can take it over), `CPF_DisTweakFallback` and
   `DisSerializeRelationshipOverrideInfo`.

Because the shared tree's generated headers are only regenerated at merge, `patch_shared.py` also inserts exactly the
15 `#include` lines that regeneration would add (14 CppText files + `DishonoredGameNative.h`), and
`build/agentAJ_work/gen_sources.py` regenerates `DishonoredGame/Sources.cmake` — the 13 skeleton units the AJ ports gave
code to were still on its `DishonoredGame_EXCLUDE` list, i.e. **they were not compiled at all** before this. Both are
reproduced by the coordinator's regeneration at merge, so nothing here needs to be kept by hand.

## Accept

Package accept line (PHASE6 AJ), measured on HEAD `f8dfe78` + the AJ files:

1. `≥ 120 more DishonoredGame natives written` — **not met** (7; the 74 written functions are infrastructure, see
   "Why 7 natives and not 120").
2. `AD's -loadall over the L_Tower_P packages reports 0 DishonoredGame serializer errors` — **met**.
3. `no warn-once line from AJ classes before the map change` — **met**, and stronger: under `-strictnatives` the run
   passes `Initial startup` and the only abort left in the process is `ADishonoredPlayerPawn::execPlayDying_Native`,
   which is agent AF's class.
4. `the module builds with DISHONORED_SDK_LAYOUT_CHECKS=ON` — **met** (0 errors, 12,502 asserts, 0 pending).

### `-loadall` over the nine `L_Tower_P` packages (my serializers active)

```
python resources/tools/build_and_smoke.py --build-dir build/agentAJ --no-build --exe-name DishonoredGame_AJ.exe
  --log-name agentAJ_loadall.log --ini-dir build/agentAJ/config --rhi null --timeout 300
  --skip-native OnlineSubsystemPC,OnlineSubsystemSteamworks --milestone "Initializing Engine..."
  --expect "Initial startup"
  --extra-args="-loadall=@D:/RecompileDishonored/Recompile/build/agentAJ/tower.txt -forcelogflush"   # exit 0
```

`L_Tower_P` 147, `L_Tower_Audio` 91, `L_Tower_Block` 77, `L_Tower_Env` 5089, `L_Tower_Fx` 184, `L_Tower_Light` 62,
`L_Tower_Nav` 47, `L_Tower_Script` 5016, `L_Tower_Water` 14 exports — **0 errors each**,
`loadall done: 9 packages, 0 errors`, and 0 `Serial size mismatch` / `Bad export index` / `Bad name index` in the log
(`build/agentAJ_loadall2.txt`). Agent AD's sweep reports the same 0/9 with their own exe, so this run shows the ported
serializers read those packages without a size mismatch rather than that they were the fix; the ten `needed` rows agent X
handed over are closed either way (a size mismatch in any of them aborts a load that contains the class).

### `-strictnatives` startup + tick run

```
python resources/tools/build_and_smoke.py --build-dir build/agentAJ --no-build --exe-name DishonoredGame_AJ.exe
  --log-name agentAJ.log --ini-dir build/agentAJ/config --rhi null --timeout 180
  --skip-native OnlineSubsystemPC,OnlineSubsystemSteamworks --milestone "Initializing Engine..."
  --expect "Initial startup" --forbid "Serial size mismatch" --forbid "Bad export index"
  --extra-args="-strictnatives -forcelogflush"                                                        # exit 0
```

- `Initializing Engine...` and `Initial startup` reached, 0 `Serial size mismatch`, 0 `Bad export index`.
- The **only** `native not ported` line (and the only `-strictnatives` abort) is
  `DishonoredGame native not ported: ADishonoredPlayerPawn::execPlayDying_Native` — **agent AF's file**. Before this
  package the chain aborted one step earlier, on `ADishonoredPawn::ChooseAndTriggerDeathEvent_Native` (the item the
  coordinator handed over from AE); with both base natives ported the chain walks
  `Died` → `PreventDeath` → `ChooseAndTriggerDeathEvent` → `NotifyKilled` → `PlayDying`.
- No Engine or GameFramework stub line is left (AE's package), and no AJ-class line at all.
- `--extra-args=-forcelogflush` is used on every run (AE's finding: without it the log stops mid-line and a milestone
  that did occur never reaches the file).

### The tweak chain runs on the map path

`DISHONORED(bringup): pawn tweaks keep the level's anim tree (-distweakanimtree applies Ply_Player_at.Ply_Player_at)` is
printed from `ADishonoredPawn::ApplyTweakChanges_Derived`, i.e. the player pawn received its `UDisTweaks_PlayerPawn`
through `IDisTweaksInterface::SetTweaks` → `UDisTweaksBase::ApplyFallbackChain` → `ApplyTweakChanges_Derived`.

**New blocker this package found (Arkane anim nodes).** With the tweaks applied the pawn's mesh gets the tweaks' anim
tree (`Ply_Player_at`) and the reference evaluator asserts
`Abs(GetChildWeightTotal() - 1.f) <= ZERO_ANIMWEIGHT_THRESH` (`UnAnimTree.cpp:1583`), because the Arkane anim nodes are
not ported: the log shows `DishonoredAnimNodeStatePicker_0` / `FullBodyStatePicker` with `StateTree1..3` all at weight
0.000000. To keep AF's milestone-5 path alive the anim-tree assignment is gated — skipped by default with one
`DISHONORED(bringup)` line naming the tree, applied with `-distweakanimtree`. Everything else the tweaks drive (anim
sets, physics asset, skeletal mesh, jiggle bones, collision cylinder, step height, faction, story group) is applied
unconditionally.

`Committed map change via DishonoredEngine` is still not reached by a plain run: the main-menu map change is issued by
the GFx menu (not ported) or by AF's `-startmap`, neither of which exists in HEAD, so that milestone stays AF's.

## Hand-overs

1. ~~**AI — `UWorld::SpawnActor` init functor.**~~ **Closed during the wave**: AI landed `FSpawnActorInitFunctor` in
   `UnWorld.h` (with `DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR`, which switches off the temporary copy in
   `DishonoredGameNative.h`) and calls `InitFunctor->DoInit(Actor)` at the retail point (`UnLevAct.cpp`, 2013 rva
   0x256990, after the transform and before the begin-play chain). `UDisTweaksBase::SpawnActor_Derived` now passes
   `FSpawnActor_TweakObj(this)`, so a tweak-spawned actor has its tweaks in `PostBeginPlay`, as retail does.
2. **AI — generator ownership.** The two hooks above are in the shared generator; keep them when you touch it.
3. **AF — `ADishonoredPlayerPawn::execPlayDying_Native`.** It is the only `-strictnatives` abort left in the process
   (at ~9 s, on the `APawn::Died` chain). `ADishonoredPawn`'s base natives are ported (`PlayDying_Native` 2013 rva
   0x748fd0, `ChooseAndTriggerDeathEvent_Native` 0x233610); the player pawn overrides both slots and keeps its own exec
   entries, so the two overrides are AF's to port (the NPC pawn's are the next package's, 0x778040 / 0x77bf50).
4. **Engine owner (AI) — two reference-only anim events.** `USkeletalMeshComponent` raises `Actor.PostInitAnimTree`
   (`UnSkeletalComponent.cpp:5646`) and `Actor.AnimTreeUpdated` (`:5411`); retail's `Engine.Actor` declares **neither**
   (421 children in `script_classes_2013.json`, checked by name), so the first pawn with an anim tree aborts with
   `Failed to find function PostInitAnimTree in DishonoredPlayerPawn` and then the same for `AnimTreeUpdated`. My
   snapshot guards both with `Owner->FindFunction( FName(TEXT("…")) )` (`build/agentAJ_work/patch_animtree.py`, snapshot
   only — the file is not mine). **This blocks the map path as soon as a pawn has an anim tree and must land somewhere.**
   (`NAME_PostInitAnimTree` does not exist as a hardcoded name in this build; `INITANIM_CUSTOM` discards its argument.)
5. **AF — pawn tweaks on spawn.** `ADishonoredGameInfo::SpawnPlayer` → `UDisTweaksBase::SpawnActor` now applies the
   player-pawn tweaks (mesh, physics asset, anim tree, cylinder, step height) through `ApplyTweakChanges_Derived`. If
   the possessed pawn looks wrong, that is the place to look.
6. **Coordinator / wave 5 — next DishonoredGame package.** The order that unblocks the most stubs:
   `UDishonoredAIBrain` (unblocks ~124 behaviour callbacks + 42 NPC natives) → `UDisItemContext` (equip/melee/use,
   the pawn's remaining 18) → `FArkComponentLocomotion` (move targets) → `DisSaveLoad`/`FGameState` (milestone 6) →
   the `FArkGameEventDispatcher` event structs (the HUD/UI notifications every ported body currently omits). Before any
   of that, the **Arkane anim nodes** (`UDishonoredAnimNodeStatePicker`, `UDishonoredAnimNodeTreeRef_Dynamic`, the
   `UDisAnimNodeBlendBy*` family): they are what the `-distweakanimtree` gate defers, and a pawn cannot animate without
   them.

## Files (no commits, no `git add`)

- New: `DishonoredGame/Inc/DishonoredGameNative.h`; `Inc/CppText/` × 14 (`IDisTweaksInterface`,
  `IDisEngineTweaksInterface`, `UDisTweaksBase`, `UDishonoredNativeState`, `UDishonoredNativeStateMachine`,
  `UDisNativeStateTransitionLogic`, `UDishonoredInventory`, `ADishonoredPawn`, `ADisMovableLimb`, `ADisWatchTower`,
  `UDisAIBlackboard`, `UDisDialogTree`, `UDisTweaks_DefenceTower`, `UDisTweaks_SkeletalBreakable`,
  `UDisTweaks_StaticBreakable`, `UDisTweaks_UsableObject`, `UDishonoredGlobalAIManager`);
  `DishonoredGameNativeStubs.ported.agentAJ.txt` (**final, 7 natives**, all `ADishonoredPawn::`).
- Bodies in `DishonoredGame/Src/`: `distweaksbase.cpp`, `dishonorednativestatemachine.cpp`, `dishonorednativestate.cpp`,
  `disnativestatetransitionlogic.cpp`, `dishonoredinventory.cpp`, `dishonoredpawn.cpp`, `disglobalenums.cpp`,
  `dismovablelimb.cpp`, `diswatchtower.cpp`, `disaiblackboard.cpp`, `disdialogtree.cpp`, `disdefencetower.cpp`,
  `disskeletalbreakable.cpp`, `dishonoredbreakable.cpp`, `dishonoredusableobject.cpp`, `dishonoredglobalaimanager.cpp`.
- Shared-tree plumbing (regenerated at merge): `resources/tools/symbols/gen_classes_header.py` (2 hooks),
  `DishonoredGame/Sources.cmake`, and the `#include` lines in `Inc/DishonoredGame.h`, `Inc/dishonoredgameclasses.h`,
  `Inc/DishonoredGameEngineShims.h`, `Inc/DishonoredGameItemClasses.h`, `Inc/DishonoredGameConversationClasses.h`,
  `Inc/DishonoredGameNPCDeathClasses.h`.
- Snapshot-only bring-up patch (not an AJ file, hand-over 4): `Engine/Src/UnSkeletalComponent.cpp` through
  `build/agentAJ_work/patch_animtree.py`.
- Scratch: `build/agentAJ_work/` (`patch_shared.py`, `patch_gen.py`, `sync.py`, `apply_gen.py`, `gen_ser.py`,
  `gen_natives.py`, `gen_sources.py`, `patch_animtree.py`, `dump_vtbl_types.py`, `vtbl_2012*.txt`, `my_stubs.txt`, `my_triage.txt`);
  builds `build/agentAJ` (snapshot), `build/agentAJ3` (shared-tree object check); worktree `build/agentAJ_wt`;
  IDA copies `resources/docs/idb/retail2013_agentAJ.i64`, `shipping2012_agentAJ.i64`.

## Commands

```
git -C build/agentAJ_wt checkout --force --detach <HEAD>          # rebase the snapshot on a new HEAD
python resources/tools/make_snapshot.py AJ --sync                 # re-copy the 35 AJ files over it
python build/agentAJ_work/patch_animtree.py build/agentAJ_wt      # snapshot-only anim-event guard (hand-over 4)
python build/agentAJ_work/apply_gen.py build/agentAJ_wt           # regenerate the DishonoredGame module in the snapshot
cmd /c build/agentAJ_build.cmd                                    # build the snapshot into build/agentAJ
python build/agentAJ_work/patch_shared.py                         # shared tree: generator hooks + the 15 include lines
python build/agentAJ_work/gen_sources.py D:/RecompileDishonored/Recompile   # shared tree: Sources.cmake exclude list
cmd /c build/agentAJ3_objs.cmd                                    # compile the AJ units against the shared tree
python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py resources/docs/idb/retail2013_agentAJ.i64 resources/reference/decomp/agentAJ/r13 rva:0x882e70
python resources/tools/ida/run.py build/agentAJ_work/dump_vtbl_types.py resources/docs/idb/shipping2012_agentAJ.i64 out.txt IDisTweaksInterface_vtbl
```

Accept runs: the two `build_and_smoke.py` command lines are quoted in full in the "Accept" section above
(`--extra-args="-loadall=@…/tower.txt -forcelogflush"` and `--extra-args="-strictnatives -forcelogflush"`).
