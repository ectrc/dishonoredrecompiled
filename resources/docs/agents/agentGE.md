# Agent GE — the fade, the heads, and the intermittent death

Wave 23 (`PHASE19.md`, package GE). Worktree `build/agentGE_wt`, base `785e91d`, with main `62f3682` and `17d90a1`
merged by `git merge` before hand-back (build_queued FIFO, docs; no source overlap). Working build `build/agentGE_rel`
(`build/agentGE/ge_build.cmd` → `build_queued.py`; two invocations: `ge_build0.log` stopped on one error of this
package's own, a duplicate body, `ge_build1.log` linked), the gate's in-harness build `build/agentGE_reg`
(`ge_gate.cmd`). Control `build/head_rel` (`9f214c5`) staged as `DishonoredGame_GEctl.exe`; this build staged as
`DishonoredGame_GE.exe`. Every launch through `build/agentGE/ge_run.py` → `game_input.py launch` (`-inputcmd`,
`-unattended`, `-windowed`, the build directory's private inis), every capture in-process and viewed (1280x720,
2,764,854 B). No retail `Dishonored.exe` launch; no real input; no FModel. IDA: private copies
`build/agentGE/idb/GE_retail2013.i64` / `GE_shipping2012.i64`, read headlessly through `resources/tools/ida/run.py`
with `build/agentGE/ge_decomp.py` and `ge_xrefs.py` (the MCP's four workers were all taken by GB/GC/GD); decompiles in
`build/agentGE/decomp2012/`, `decomp2013/`, vtables in `vt2012.txt` / `vt2013.txt`.

## 0. The answer in eight lines

1. **The screen comes back after the first Empress scene.** The black was never a fade-out that this tree failed to
   run: `SeqAct_CameraFade_3` (FadeTime 0.5, opacity 1, persistent) is meant to be undone by the Fade track of
   `SOIREE_Empress_Regent.SeqAct_Interp_26` ("Simple Soirée: Player Overhearing Empress"), which Kismet starts through
   `DisSeqAct_DialogInputs_26 'BlockPlayer'` on **DisDialogOneShot_27** — the SECOND one-shot bound to the dummy elite
   `DishonoredNPCPawn_9` (spawner 35 holds two). FR's `UDisDialogTree_InGameBind::PlayFromNode` answered TRUE for a hook
   node that matched nothing, so the first one-shot (`_7`, 'Guard_Prison') swallowed the hook and `_27` was never tried.
   Retail's body (2012 `0x905f40`) answers TRUE only once a conversation plays — ported (§2). GE1: `Interp_26`
   activation 1 at t 45.58, `gediag fade` amount 0.343 → 0.000 within 1.2 s; captures `ge1b/c/d` show the gazebo where
   the control's `gec1b/c/d` are black.
2. **The "bad AnimNotify" lines are gone** (0 in GE1/GE2 against 396 in FZ7): retail's `UAnimSequence::PostLoad` (2012
   `0x370150`, 2,585 B) has no owner test — neither database contains the string. The reference engine's check and its
   `Rename` are removed; the mismatch is still counted (`flnotify totals: 5637 loaded, 379 outered to a sibling`). The
   class `UDisNotify_NPCDisableLookAt` was never failing to load (§1).
3. **The player death is named, and it is every run's fall at spawn.** With `-gedeathdiag`: `t=1.42 TakeDamage
   DishonoredPlayerPawn: 62 of type DmgType_Fell ... health 100 -> 38 ... Z=-1420.6 velocity Z -2658 physics 2 (Falling)
   base none` — the pawn is possessed at Z −215.85 (t 5.10 wall) while the PhysX scene has **0 → 4 static shapes**
   (t 4.70 / 6.69 wall), falls 1,200 units through the floor that is not there yet and lands when the static shapes
   arrive (1,097 by t 8.26). 62, 60, 57 and 69 in four measured runs; the death (loop run 3 of 20, a 41.6 s load) is the run where the
   floor is still absent at 2.78 s and KillZ fires `KillZDamageType` 1000 - `Killer=None` (§4, §8). The stubs of the three `PlayDying_Native` natives
   now log their killer, damage type and hit location on every call (generator mechanism, §4.1).
4. **No guard's head follows the player, and the three sub-processes were never the asker.** The head-track look-at is
   raised by `UDishonoredAIBrain::ChangeTopAttnTargetAttentionLevel` (2012 `0x75a420`) from `SetTopAttnTarget`, whose only
   in-game caller is `UDisAIBrainProcessAttention::HandleTopAttnTarget` — the attention brain process, 88 functions, on
   the exclude list with the three `UDisAttentionInfo_*` units (223 functions, 30.9 KB together). Ported: the three
   sub-processes (they react to the level, Bark barks, Pause stops and turns), the brain's four attention-level functions
   and `GetCurrentDesiredPosition`, `UDishonoredAIBehavior::FireDialogHook`. Measured: `lookatcpnt requests 0` on the two
   dock-gate guards beside the player on control and on this build alike (GEC2 / GE2) — nothing changes until the
   attention process lands (§3).
5. `USeqAct_Switch::Activated` exists inline in `EngineSequenceClasses.h`; `SOIREE_Empress_Regent.SeqAct_Switch_8`
   (LinkCount 2, default Indices) fires its unlinked 'Link 1' on the first activation by design and 'Link 2' →
   `DisSeqAct_DialogInputs_6:1` on the next (`Gate_37` ← `Emily_GoToGazebo_Success`, GD's go-to). Not a fault.
6. Gate: §8. Builds: 0 errors, 0 C4263, 0 C4264 in `ge_build1.log` and the gate's log.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "`CameraFade_3` in, no fade out — director track / `UDisSeqAct_CameraFade` / the player camera's fade state" | The fade-out exists and is a director-group `InterpTrackFade` (3 keys) on `SeqAct_Interp_26`, started by the dialog system from `DialogInputs_26 'BlockPlayer'`, not by Kismet after `Interp_0`. `-gelinks`: `CameraFade_3` FadeTime 0.5, everything else default (opacity 1, persist). The camera's fade state was right all along (`gediag fade`: enable 1 amount 1.0 held because nothing asked for less). |
| "FZ's soirée state may own the camera now" | It does not touch the fade; `UInterpTrackFade::UpdateTrack` writes `FadeAmount` on the player camera and FZ's `UpdateViewTarget` leaves it alone. |
| FV: "the look-at target is a `FDisLookAtRequest` from `UDisAISubProcessHeadTrackBase / Bark / Pause`" | None of the three raises a look-at: Base keeps the proxy of the head-track stim, Bark fires `DDH_ATTENTION_HEADTRACK`, Pause sets a loco and a face-to desire. The request is the brain's `m_LookAtRequest.RequestProxyTarget` in `ChangeTopAttnTargetAttentionLevel` at `DAL_HeadTrack` on an enemy (xrefs: `FDisLookAtRequest::RequestProxyTarget` ← that function and `UDisConversationComponent::Tick`). |
| "`DisNotify_NPCDisableLookAt` fails at package load" | The class loads and registers (`IMPLEMENT_CLASS`, size 64 asserted). The line is the reference engine's `*** Found bad AnimNotify in <seq> : <notify>` for a notify whose outer is a sibling sequence of the same set — 396 per run on rats and guards alike, `AnimNotify_AkEvent` included. Retail has no such check (string absent from both databases). |
| "the intermittent death ... new with wave 22 (FV's `Tick` body, GA)" — corrected mid-package by the coordinator: seen once in wave 20 | It is the spawn fall: every run takes 60–62 `DmgType_Fell` at t 1.4 with `PhysX static shapes` 0 at possession; neither FV's tick nor GA's base is on the path (`instigator none causer none`, `base none`, physics Falling). |
| "the death is 1 in 7" | The fall damage is deterministic in kind, variable in amount (60 vs 62 in two runs); the distribution over the loop is §4.2. |
| `USeqAct_Switch::Activated` missing (this package's own first reading of `grep` on `Engine/Src`) | It is the `.uc` cpptext inline in the generated header; the duplicate body cost one build pass and is gone. |
| FZ/GF: "stretched dark silhouettes on NPC pawns standing on a matinee-moved boat, absent from the control" | Control `gec2b` (`head_rel`, pre-FZ): the two dock-gate guards on static ground are the same tall thin silhouettes; so are the Empress and the Regent in the gazebo on this build (`ge1b`). Not boat-specific and not FZ's — GC's anim stack. |

## 2. The fade

Chain (GEC1, control, `-fwkismettrace -foupstream`): `-fwteleport=DisTrigger_16@45` → `SeqEvent_Touch_69` → `Gate_67` →
`SeqAct_CameraFade_3` (t 45.04; 'Finished' after 0.5 s) → `DisSeqAct_DialogInputs_23 'Empress & Regent'` on
`DisDialogOneShot_0` (NPCPawn_5) → `DisConv_Soiree_0` → `StartFromDialog` → `SeqAct_Interp_0` (t 45.54, 16.06 s), and in
the same tick `DialogInputs_23 'Out'` → `DisSeqAct_DialogInputs_26 'BlockPlayer'` on `DisDialogOneShot_27`. The
component of `DishonoredNPCPawn_9` holds `m_OneShotInfo` = [`DisDialogOneShot_7` (bind 15), `DisDialogOneShot_27` (bind
16)]; `UDisConversationComponent::FireDialogHookNow` tries them in order and stops at the first that "fires".

Control: `bind kismet hook DisDialogOneShot_7...KismetActivated_0: input 'BlockPlayer' matches none of 1 name(s)` →
`played ... -> output -1` → **`FireDialogHook hook 10 branch 0 on DishonoredNPCPawn_9 -> fired`** → bind 16 never
tried → `Interp_26` activations 0 → black (`gec1b` at 50 s, `gec1c` at 65 s, `gec1d` at 75 s).

Retail `UDisDialogTree_InGameBind::PlayFromNode` (2012 `0x905f40`, 426 B, decompiled): the result starts as
`m_pCurPlayingConversation != NULL` and becomes TRUE only inside the branch that acquires the speakers of a
start-of-dialog node (or when a conversation already plays); a hook node that plays alone keeps it FALSE. FR's body set
`bRet = TRUE` before every `PlayNode`. Fixed in `disdialogtree_ingamebind.cpp` (one assignment moved, one else added).

This build (GE1): `...KismetActivated_0: matches none` → `...DisDialogOneShot_27...KismetActivated_4: input 'BlockPlayer'
is output 0 of 1` → `DisConversation_31` → `DisConv_PlayerLookAtActor_2` → `DisConv_Soiree_4: StartFromDialog on
SeqAct_Interp_26` → `SeqAct_Interp_26 activation 1 via 'Play'` (t 45.58) → `player group 'PlayerGroup' instanced on
DishonoredPlayerPawn` (FZ's soirée state: physics 7 from 46.07) → `gediag fade t=46.07 enable 1 amount 0.343`,
`t=47.29 amount 0.000`. Captures `ge1b` (50 s, the gazebo with the two speakers), `ge1c` (65 s), `ge1d` (75 s).
After `Interp_0` completes (t 61.66): `ActivateRemoteEvent_45 'Soiree_EmpressAndRegent_Completed'` → `Switch_8`
activation 1 ('Link 1', unlinked, by design — §0.5).

## 3. The heads

What was ported, resolved by hand in both databases (2012 rva / 2013 rva, sizes in the unit banners):

| function | 2012 | 2013 |
|---|---|---|
| `UDisAISubProcessHeadTrackBase::GetHeadTrackProxy` | `0x797450` | `0x734fd0` |
| `::SetHeadTrackProxy` | `0x79abe0` | `0x73acd0` (callee of FilterHeadTrackBegin) |
| `::OnOtherActorTerminatedEvent` | `0x78f0f0` | `0x72b9d0` |
| `::BeginSubProcess_Derived` / `::EndSubProcess_Derived` | `0x79bd90` / `0x79bdc0` | `0x73e5d0` / `0x73e5f0` (vtable slots 96/97, one slot above 2012's 95/96) |
| `::FilterHeadTrackBegin` / `::FilterNoticeBegin` | `0x79bdf0` / `0x79be10` | `0x73e610` / thunk `0x7415d0` |
| `::BuildFilterStimMask` | `0x78d390` (bits 43/44/60, the 2012 ids) | `0x728a60` (bits 44/45/63 = this tree's enum) |
| `::GetFilterStimDelegate_SubProcess` | `0x7a04c0` | `0x744d00` |
| `::BeginDestroy` | `0x79be40` | unnamed |
| `UDisAISubProcessHeadTrackBark::OnHeadTrackProxySet` / `::TickSubProcess_Derived` | `0x78d380` / `0x79ab50` | `0x728a50` / `0x73ac60` |
| `UDisAISubProcessHeadTrackPause::BeginSubProcess_Derived` / `::TickSubProcess_Derived` | `0x79be90` / `0x7a4eb0` | `0x73e710` / `0x73adb0` |
| `UDishonoredAIBrain::SetTopAttnTarget` / `ChangeTopAttnTargetAttentionLevel` / `OnIncrease...` / `OnDecrease...` | `0x75ba20` / `0x75a420` / `0x757eb0` / `0x757d30` | `0x725e50` / `0x725890` / `0x724d00` / `0x722ec0` |
| `UDishonoredAIBrain::GetCurrentDesiredPosition` | `0x746110` | `0x700eb0` |
| `UDishonoredAIBehavior::FireDialogHook` | `0x74f550` | `0x6f00f0` |

Deviations, named: Bark's 2013 tick skips the bark when the target answers TRUE to `IDisAttentionTargetInterface` slots
13 (`IsAttnTargetDead`, 2012 `0x78e740`) and 17 (unnamed, folded on return-FALSE) — this tree's interface stand-in
declares neither, so the 2012 body is used; `ChangeTopAttnTargetAttentionLevel`'s `SetupWalkUpperBodyAnim` /
`ClearWalkUpperBodyAnim` branch (unported pawn functions behind an unnamed `UDisTweaks_AIBrain` flag) is named and not
made; `IsAttnTargetDead` in the Increase/Decrease stims is the cached pawn's `Health <= 0 || m_bIsOfficiallyDead`;
`DisAngleDiff` is written inline (`DisGEYawDiff`); the `m_bTopAttnTargetDead` compare is on the bit.

Measured (GEC2 control / GE2 this build, `-flheaddiag -fwteleport=DisTrigger_8@30`, the player among the two gate guards
`DishonoredNPCPawn_23` at 81 units and `_22`): `lookatcpnt requests 0 type 1 target none notlooking 1` on both guards in
both builds for the whole stay; the only requests in the level are the two patrol procedural ones (`_19`, `_20`, type 4,
prio 6, FT's). The sub-processes are instantiated only from a behaviour tweak's `m_SubProcessTweaks` whose spawned class
is one of the three (the generic `UDishonoredAIBehavior::InitBehavior` loop, CG's) — and `FAIStimStruct_HeadTrackBegin`
is raised by nothing until `UDisAIBrainProcessAttention` is ported. **What a head needs, specified**:
`disaibrainprocessattention.cpp` (88 PDB functions: `TickBrainProcess_Derived` → `TickAttention` →
`HandleTopAttnTarget` → `SetTopAttnTarget`; `FilterTargetSighted` → `BumpAttention`; `GetAttentionLevel`,
`GetAttentionProxyInfo`, `FindAttentionInfo`, the 40 filters) plus `disattentioninfo_base/simple/complex.cpp` and
`distweaks_aibrainprocess_attention.cpp` / `distweaks_pawnattention.cpp` — 223 functions, 30,912 bytes in the 2012
build (`build/agentGE/vt2012.txt`). The senses (`TickBrain_Senses`, ported) already raise what it filters.

## 4. The death

### 4.1 The instruments

- `-gedeathdiag`: `ADishonoredPawn::TakeDamage` logs every damage event on every pawn (damage, type, instigator, causer,
  hit location, momentum, health before/after, location, velocity, physics, base, volume, touching, and the PhysX
  primary scene's static shape count through `DisEnginePhysXStaticShapes()` in `UnPhysLevel.cpp`); `APawn::Died` logs
  the death with killer, damage type, hit location and the same pawn state (Engine, hand-edited, both switched).
- The generator: `<Module>NativeStubs.logparams.txt` (+ `.logparams.agent<X>.txt`) lists natives whose stub is
  `DISHONORED_NATIVE_STUB_LOGPARAMS` instead of `DISHONORED_NATIVE_STUB`; `Core/DishonoredNativeStub.cpp`'s
  `ConsumeAndZero` exports each consumed parameter with `UProperty::ExportTextItem` and the stub warns on every call:
  `native not ported: ADishonoredPlayerPawn::execPlayDying_Native on <pawn> with _pKiller=... _pDamageType=... _HitLoc=...`.
  Listed: the three `PlayDying_Native` (player, NPC, possession proxy). `DishonoredGameNativeStubs.cpp` is the
  generator's output (three lines differ).

### 4.2 The measurement

Every run of the startmap route (GE1, GE2): `possessed DishonoredPlayerPawn in l_tower_p at Z=-215.85` (wall 5.10 s);
`PhysX scene: 2 actors, 0 static shapes` (4.70 s) → `6 actors, 4 static shapes` (6.69 s); then
`gedeathdiag t=1.42 TakeDamage DishonoredPlayerPawn: 62 of type DmgType_Fell ... at Z=-1420.6 ... health 100 -> 38 ...
velocity Z -2658 physics 2 base none touching 0, PhysX static shapes 1097` (GE1; GE2: 60, health → 40 at t 1.37). The
player falls 1,205 units from the spawn before the level's collision exists and lands on it as it arrives. UE3's
`TakeFallingDamage` scales with the landing speed: 60–62 points for this fall, 100 for a longer one, which is what a slow
load gives (the static shapes reach the scene later, the pawn falls further). `instigator none causer none` is what
`APawn::Died` then sees — the "nothing in the log before it" of the two earlier sightings (FO wave 20, GA wave 22).

The d3d9 loop (`build/agentGE/ge_deathloop.py`, `run_regression.py --build-dir agentGE_reg --no-build --only d3d9
--no-load-retry --game-extra-args=-gedeathdiag`, each log copied to `build/agentGE/deathloop/d3d9_<n>.log`): §8.

What would fix it, not done here (no build left): retail possesses the player after the level's static shapes are in
the scene (or holds the pawn until they are); the spawn order in this tree's `UGameEngine::LoadMap` / `UWorld::Tick`
puts the possession ~1.2 s before `InitWorldRBPhys` finishes the static actors. Also: `FArkComponentLocomotion` does not
run on the player (GA), so the NPC floor sweep is not what saves the NPCs - they spawn later.

## 5. The instruments (all under switches, nothing changes behaviour)

`gediag.h/.cpp`: `-gediag` (the camera fade state once a second + every active `SeqAct_CameraFade` with its opacity,
time, persist flag, object version), `-gelinks=<substr,...>` (once per map: an op's output links with targets, its
variable links with values, every property that differs from the class default — the way `CameraFade_3`'s FadeTime 0.5
and `Switch_8`'s LinkCount 2 were read), `-gedeathdiag` (§4.1). `ge_run.py` (the launcher with the build directory's inis
and a measured-line summary), `ge_deathloop.py`, `ge_decomp.py` / `ge_xrefs.py` (headless IDA: decompile by rva or
name regex, string xrefs, callers, callees, vtables by slot range).

## 6. The runs

| run | exe | route / switches | result | captures |
|---|---|---|---|---|
| GEC1 | control | startmap, `-fwteleport=DisTrigger_16@45 -foupstream=...CameraFade,...Switch_8 -fwkismettrace` | the chain of §2, black after 45.5 s, `Interp_26` activations 0 | `gec1a` (spawn view), `gec1b/c/d` black |
| GEC2 | control | startmap, `-flheaddiag -fwteleport=DisTrigger_8@30` | guards: `requests 0`; silhouettes on static ground | `gec2a/b/c` |
| GE1 | this build | GEC1's switches + `-gediag -gedeathdiag -gelinks=... -flnotify` | `Interp_26` activation 1, fade 0.343 → 0.000, `bad AnimNotify` 0, fall damage 62 at spawn | `ge1a`, `ge1b/c/d` the gazebo |
| GE2 | this build | GEC2's switches + `-gedeathdiag` | guards: `requests 0`; fall damage 60 | `ge2a/b/c` |
| loop | gate exe | `run_regression --only d3d9` ×N | §8 | — |

Logs `DishonoredGame/Logs/GEC1.log`, `GEC2.log`, `GE1.log`, `GE2.log`; captures (BMP + PNG) `build/agentGE/captures/<run>/`.

## 7. Files

`build/agentGE_sync.py`'s header names every file and function. Engine (reference files, hand-edited, said so):
`UnSkeletalAnim.cpp` (the check removed), `UnPawn.cpp` (`Died` log), `UnPhysLevel.cpp` (`DisEnginePhysXStaticShapes`).
Core: the stub pair. Generator: `gen_classes_header.py` (logparams). DishonoredGame: four units off the exclude list
through the generator (`Sources.cmake` 752 → 748), three new CppText headers, three link anchors in the generated
registrants, the three LOGPARAMS stubs in the generated stubs unit, the dialog-tree fix, the brain/behaviour additions,
`gediag`. No reflected member added or moved; the generator run after the edits reproduces every other generated file
byte for byte (`build/agentGE/gen0_diff.txt` before the edits: clean).

## 8. Acceptance

| check | result |
|---|---|
| gate `run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGE_reg --newgame-at 50 --game-extra-args=-windowed` from the worktree's copy, built in-harness (the package's second build; the tree at commit `3e9d417` = this package on `17d90a1`) | **47 ok, 0 failed, 0 skipped**; `build/agentGE/ge_gate.log`, `build/agentGE_reg/regression/summary.txt` |
| the gate's build | `build_DishonoredGame_errors 0`, 0 `error C` / `error LNK`, **0 C4263, 0 C4264** (`build/agentGE_reg/regression/build_DishonoredGame.log`); `coresmoke` / `layout` / `verify_phase2_total 3` |
| the rows | `unported_natives 0 (<= 0)`, `nullrhi` / `d3d9` / `inputtest` / `newgame` / `options` `_criticals 0`, `d3d9_startup_seconds 5.36`, `inputtest_moved 850.4`, `newgame_pawns 29` |
| the fade (item 1) | GE1 against GEC1: `Interp_26` activation 1, `gediag fade` 0.343 -> 0.000, captures `ge1b/c/d` the gazebo vs `gec1b/c/d` black |
| the bad AnimNotify lines (item 2) | 0 in GE1, GE2 and every loop log (396 in FZ7) |
| a guard's head follows the player (item 2) | **not met**, and not reachable from the three sub-processes: the asker is the attention brain process (§3); control and this build both read `requests 0` on the guards (GEC2 / GE2) |
| the death (item 3) | **reproduced and named on loop run 3 of 20** (`build/agentGE/deathloop/d3d9_3.log`): `startup 41.59 s`; possessed at Z -215.85 (wall 44.04) with `PhysX scene: 6 actors, 4 static shapes` (44.55); `gedeathdiag t=2.78 Died DishonoredPlayerPawn killer none damage type KillZDamageType hit Z=-6197.6: health -900 ... velocity Z -4000 physics 2 base none`; the stub: `ADishonoredPlayerPawn::execPlayDying_Native on l_tower_p...DishonoredPlayerPawn with Killer=None DamageType=Class'Engine.KillZDamageType' HitLoc=(X=-3900.6,Y=36639.3,Z=-6197.6)`; `TakeDamage 1000 of type KillZDamageType ... health 100 -> -900 ... static shapes 1297`. Runs 1 and 2: 57 and 69 `DmgType_Fell` at t 1.35 / 1.41 (health -> 43 / 31, landing speed 2,705 u/s, 52 static shapes at impact on run 2). The loop stops at the first death as asked. |
| the sync | `python build/agentGE_sync.py`: 30 files, **0 flagged, 0 missing** against main `f7bc754` (merged into the worktree with one conflict in `dishonoredplayercontroller.cpp`, both diagnostics kept; the generator re-run over the merged tree changes nothing); `git diff --name-only f7bc754` + `ls-files --others` = the sync list |
| the rules | every launch `ge_run.py` -> `game_input.py launch` (`-inputcmd -unattended -windowed`); no real input; no retail `Dishonored.exe`; no FModel; builds through `build_queued.py` (working: two invocations of one build, the first stopped by this package's own duplicate `USeqAct_Switch::Activated`; gate: one); every address resolved by hand in both databases |

**The death, named.** The player is possessed at the water-lock spawn (Z -215.85) before the level's static PhysX
shapes exist (0 at possession, 4 half a second later, ~1,100-1,300 only two seconds after that). He falls. On a normal
load the floor arrives under him within ~1.4 s and he lands with 57-69 `DmgType_Fell` (every run, health -> 31..43);
on a slow load (run 3: 41.6 s to first frame, another process on the machine) the floor is still not there when he
passes KillZ at 2.78 s (Z -6197, terminal velocity) and `AWorldInfo::KillZ` kills him with `KillZDamageType` 1000 -
`Killer=None`, which is the "nothing in the log before it" of FO's wave-20 sighting and GA's wave-22 one. Not FV's
tick, not GA's base: `instigator none causer none`, `base none`, physics Falling from the first frame. The fix is the
spawn order (possess after `InitWorldRBPhys` has the static actors, or hold the pawn's physics until they exist) -
Engine's `UGameEngine::LoadMap` / `UWorld` bring-up, not attempted here (no build left); every run of the d3d9 stage
loses 60 health at spawn until it is.
