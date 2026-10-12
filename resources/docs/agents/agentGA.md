# Agent GA — NPC bodies move with their animations: the Locomotion track, FArkComponentMeshOffset, the root-motion modes, and the full move's floor

Wave 22 (`PHASE18.md`, package GA, added mid-wave). Worktree `build/agentGA_wt`, base `f14326b`, with main `02eb527`
(FX, FW, FY's link fix, FV) merged by `git merge` as the coordinator asked (commits `32cc340` the package,
`1ef5677` the merge). Working build `build/agentGA_rel` (Release, `DISHONORED_LAYOUT_CHECKS=ON`, through
`build_queued.py`: `build/agentGA/ga_build.cmd`, logs `ga_build0..3.log`); the gate's in-harness build is
`build/agentGA_reg`. Sync `build/agentGA_sync.py` (20 files, every function named). Control `build/head_rel`
(`da77f0f`), staged as `DishonoredGame_gabase.exe`. No retail `Dishonored.exe` launch; every launch
`game_input.py launch` with `-windowed` (`presented frame (1920x1080, windowed)` in every log).

## 0. The answer in six lines

1. **The two guards travel with the boat.** On the control the Elite (`DishonoredNPCPawn_27`) and the captain (`_26`)
   sit at `-3735/36472/-223` and `-3790/36315/-134`, `base none`, for all 94 s of `SOIREE_Boat.SeqAct_Interp_1` while
   the player rides away on `SkeletalMeshActorMAT_2` (the boat); on this build both get `base SkeletalMeshActorMAT_2`
   within 6 s of spawning and ride it to the water-lock: Elite at `9994/23174/2536` when the ride ends at pos 94.08
   (the player: `9766/23113/2413`). Captures `ga3_ga_b` (mid-ride: the captain on the deck beside the player) against
   `gac1_ga_b` (the same frame on the control: an empty deck). §3.
2. **What based them is the floor sweep of retail's `FullMovePawn`** (2012 `0x580ae0`), which agent DN's `MoveActor`
   port had left out: the swept cylinder's floor hit becomes `Base` when it blocks the pawn, and no floor at all puts
   the pawn in `PHYS_Falling` so UE3's `processLanded` bases it. Agent FV ported the same sweep in the same wave
   (`UpdateFloorAndBase`); the merge keeps FV's body and adds GA's two measured cases: the start-penetrating sweep (it
   lifted the Elite a step height per frame until handled) and the no-floor fall. §2.3.
3. **`FArkComponentMeshOffset` is ported whole** (13 of 13 PDB functions, 136-byte layout asserted) and runs on every
   NPC with a `UDisTweaks_ComponentMeshOffset`: an NPC whose cylinder hovers above its floor (11 of 28 do, by 1 to 17
   units, `-gadiag ground`) now has its mesh translated down onto the floor (`meshT -17.0` on `DishonoredNPCPawn_3`,
   `-11.0` on `_15`), which is the "animations play mid air" the user saw for NPCs standing around. §2.2.
4. **The `Locomotion` track is ported** (`UInterpTrackInstLocomotion`, all 10 bodies + the key-properties pair) and
   drives `FArkComponentLocomotion` through FS's seam: on `SOIREE_Workers.SeqAct_Interp_3` key 0 sends `Dock Worker B`
   (`DishonoredNPCPawn_3`) to `TargetPoint_57` and he walks 850 units at 123 u/s; `Dock Worker A`'s path fails
   (`PATHFINDING_FAILED` every 5 s, DN's A*, not this package's). The walker's body still slides: his animation stays
   `Empty_Idle`, because the walk animation is `UArkAnimNodeLocomotion`'s (a comment-only skeleton). §2.1, §6.
5. **`ChangeAnimStateMotionModes` is 46 bytes and is the two mesh fields.** FS's "deviation" was retail's body;
   `MAT_SetRootMotionMode` / `MAT_StopRootMotion` now go through `Start/StopRootMotionMode` as retail's do, and root
   motion's application was never missing (`USkeletalMeshComponent::ProcessRootMotion` is the reference engine's).
   No pawn in the Boat or Workers scenes extracts any root motion (`rmd 0.00` on every line of every run). §2.4, §5.
6. Gate: see §7. Build logs: 0 errors, 0 C4263, 0 C4264 on every pass.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "`FArkComponentLocomotion::MovePawn` (2012 `0x503c30`) with the five unported helpers FS lists" | `0x503c30` is the **2013** rva of `UInterpTrackInstLocomotion::UpdateTrackInst` (1,144 B); `MovePawn` is 2012 `0x584ca0` (160 B) and has had a body since agent DN (`arkcomponentlocomotiondynamic.cpp:461`). The unported five were the track *instance's* (`UpdateTrackInst_Jump`, `ComputeLocoProp`, `UpdateStopObsoleteLocoRequest`, `StopCurrentLocoRequest`, `FindKeyTargetActor`) plus `GetGameSpecificLocomotionModifier`; all ported |
| "`ChangeAnimStateMotionModes` (2012 `0x78e4b0` / `0x78e4c0`)" | those are `StartRootMotionMode` (14 B) and `StopRootMotionMode` (15 B). `ADishonoredPawn::ChangeAnimStateMotionModes` is `0x78e550`, 46 B, ICF-folded with `AMatineePreviewPlayerPawn`'s: `Mesh->RootMotionRotationMode = r; Mesh->RootMotionMode = m;`. Only the player's (`0x6fae70`, 222 B, FZ's unit) adds `PendingRMM` and the physics swap |
| "the per-frame application lives in `ADishonoredPawn::Tick_Body` (`dishonoredpawn_body.cpp`, 90 code lines, partial) ... you own its motion branch" | `Tick_Body` is 31 B (2012 `0x7959c0`, 2013 `0x7510b0`): `if( !m_pCpntRetargeting->m_bIsRetargeting ) Tick_Body_CachedPositions()`. There is no motion branch; the 90 lines were `TakeFallingDamage_Native`, `PostBeginPlay_Body` and `GetBone_ByName`, and `Tick_Body` itself is FV's (merged). Root motion is applied by `USkeletalMeshComponent::ProcessRootMotion` (`UnSkeletalComponent.cpp:3358`, 3,979 B in retail, present here) |
| "the user's 'other NPCs I interact with' float because an animation with root translation plays on a pawn whose actor never follows it" | not one pawn extracted root motion in three runs (`rmd 0.00` on 3,000 `gadiag` lines). What floats: (a) the boat's guards, unbased; (b) 11 of 28 NPCs whose cylinders stand 1-17 units above their floor with no mesh offset to put their feet down; (c) a walking NPC whose actor moves and whose body plays `Empty_Idle` (`UArkAnimNodeLocomotion` unported, §6) |
| "the boat sails off on its Move track" (FS's measurement, carried into the brief) | on the `-finewgame=50` route the boat is `SkeletalMeshActorMAT_2` (`SOIREE_Boat.SeqAct_Interp_1` group `Boat_Mover`, `tracks 0`, actor `InterpActor_3`); the player is based on it from 65 s. FS measured `InterpActor_18` on the `-distouchprobe` `StartSpawn` route |
| FV's finding relayed mid-wave: "physWalking -> MovePawn is never reached for a matinee AI-group pawn" | it is reached: `MAT_SetCollision` (retail `0x7ab7a0`, 252 B, FS's port is exact) forces `PHYS_Walking` on every frame of the scene, so `performPhysics` calls `ADishonoredNPCPawn::physWalking` -> `MovePawn` -> `FullMovePawn`'s zero-move branch -> `UpdateFloorAndBase`, which is where the guards got their base (GA2/GA3); the Elite's first based line is 6 s after his spawn, inside the scene |
| "retail bases the spawned guard on the boat so the Move track carries him" (FS §6.1) | right about the effect, wrong about the mechanism: nothing in the spawner bases him. He spawns `PHYS_Walking` on the deck and the first full move's floor sweep (`FullMovePawn` +0x6a0: `SetBase( Hit.Actor, Hit.Normal, 1 )` when `IsBlockedBy`) makes the boat his base, exactly as any NPC standing on BSP gets `WorldInfo_0` |

## 2. What was ported

### 2.1 The Locomotion track (Engine, `interptrackinstlocomotion.cpp` + `interptracklocomotion.cpp`; DishonoredGame, `interptracklocomotion_game.cpp`)

`UInterpTrackInstLocomotion` through FS's `UpdateTrackInst` seam (pass `ETUP_Locomotion`), split the way agent FK split
`UInterpGroupInstPlayer`: the bodies that touch `FArkComponentLocomotion` (declared in DishonoredGame) are in the game
unit, the rest in the Engine. 2012 rvas, all function starts at the sizes quoted (`build/agentGA/ga_resolve.txt`),
2013 named at the same sizes: `UpdateTrackInst 0x542630 / 0x503c30` (1,144 B both), `UpdateTrackInst_Jump 0x53f5e0 /
0x500260`, `ComputeLocoProp 0x5423d0 / 0x5037e0`, `UpdateStopObsoleteLocoRequest 0x53f760 / 0x5003e0`,
`StopCurrentLocoRequest 0x53ce50 / 0x4fdce0`, `CallbackLocoEvent 0x53bc00`, `TermTrackInst 0x53bb90 / 0x4fca10`,
`InitTrackInst 0x542380 / 0x503790`, `NeedsSynchronizing 0x53fa50 / 0x5006b0`, `CalcInitialTransform 0x53eff0 /
0x4ffc70` (partial: the AI stage-mark / base-relative half read, the tail written from the Move track's which the
reflected members mirror), `UInterpTrackLocomotionKeyProperties::FindKeyTargetActor 0x53cfa0 / 0x4fddc0`,
`UInterpTrackLocomotion::GetKeyProperties 0x544d10 / 0x503af0`, `GetGameSpecificLocomotionModifier 0x7ab590 /
0x74b030` (`EKLM_Sword -> 5`), the `"(Player)"` target name (dynamic initializer `0xb9ed40` / `0xb825c0`; a
function-local static here), and `UDishonoredEngine::Init`'s `SetLocomotionPriority( 11 )`. The request-property
defaults are `FArkCpntLocoGoToActorProp::InitAllByDefaultExcept` (2012 `0x5803d0`, 2013 `0x53f1c0`: max speed = the
config's `m_SpeedNames.Num() - 1`, end threshold 20, accurate stop) with the key's overrides (max speed index, end
threshold 0, stop distance = max( key's, 150 for the player ), accurate stop = key's bit or any end speed, end speed
index, speed multiplier = the track's float curve, >= 0.01). Retail passes the component a member-function pointer
for the callback; DN's callback type takes the owner, so `DisLocoTrackCallbackThunk` is the one line between them.
`LoadData` / `SaveData` (48 B, the save layer's) are not ported. `-gadiag` / `-fkdiag` log every request start and
every event (`gadiag locotrack`).

### 2.2 `FArkComponentMeshOffset` (Engine, `arkcomponentmeshoffset.h/.cpp`)

The 136-byte 2012 PDB layout (asserted), all 13 attributed functions plus the ctor's defaults: `Tick` (2012
`0x52f730`, 2,129 B; 2013 `0x4f5e20`, 2,140 B) - the ground-pitch turn, the target height from a trace below the
cylinder (cached while the base has not moved) or from the foot-placement props, the offset kept in world terms across
the pawn's and the base's Z moves, the eased approach, the mesh's rotation / translation written and the transform
dirtied; `GetGroundOrientation` (`0x529db0`), `PreAsyncWorkTick` / `PreAsyncWorkJustBeforeProceduralAnimTick`
(`0x5358e0` / `0x535990`, read off the disassembly because Hex-Rays mis-adjusts the two interface subobjects:
`build/agentGA/disasm2012/`), `ForceMeshOffset`, `ForceInstantSnapMesh`, `EnableByFlags`, `DisableByFlags`,
`GetMeshOffsetComponent`, `Starting`, `Stopping`, `GetInternalName`. The two `IArkComponentPreAsyncWork*` interfaces
moved from DishonoredGame's `arkcomponentlocomotion.h` to Engine's `arkcomponentbase.h` so the Engine class can derive
from them. Created where retail creates it, `ADishonoredNPCPawn::PreBeginPlay_NativeComponents` (2012 `0x7b0c10`, new
unit `dishonorednpcpawn_nativecomponents.cpp`, also the `ARKCOMPONENT_LINK_TYPE` anchor), from
`UDisTweaks_NPCPawn::m_pComponentMeshOffsetConfig`'s six values, under a new `ADishonoredPawn::PreBeginPlay`
(`0x78e220`: the base, `PreBeginPlay_NativeComponents`, `StartAllComponents`; `PreBeginPlay_Inventory` / `_Actions`
absent, `_Attributes` left on BF's lazy path - §5). Ticked where FT ticks the look-at (`USkeletalMeshComponent::Tick`,
just before the anim nodes; retail's component manager is not ported), both policies in order. Reached by:
`MAT_DisableMeshTranslation` / `MAT_EnableMeshTranslation` (real bodies now, flag `eMeshOffsetDisable_ByMatinee`),
`SetMeshTranslation` / `LockMeshTranslation` (2012 `0x78e960` / `0x78e920`), and `MAT_BeginAIGroup`'s
`ForceInstantSnapMesh`. `m_ExternalRotationOffset` has no writer yet (retail's is `FArkComponentLocomotion::UpdateMesh`,
the pitch/roll step 10 of `PreAsyncWorkTick` that DN left out).

### 2.3 The full move's floor and base (`arkcomponentlocomotiondynamic.cpp`, shared with FV)

FV's `UpdateFloorAndBase` (the swept-cylinder floor hit -> `Floor`, or `SetBase` when the hit actor blocks the pawn,
called from both of `FullMovePawn`'s branches) is kept as merged; GA adds, inside it, the two cases its first version
met in the runs: a sweep that **starts penetrating** (`Hit.bStartPenetrating`: the hit is the start point, and taken as a
floor it lifted the Elite 30 units up - GA2's `meshT -30.3` - and, in GA's own first version that also moved the pawn,
lifted `DishonoredNPCPawn_12` from Z 3094 to 4684 at a step height per frame) now re-tests with a plain line from the
cylinder's centre and only accepts a floor within one step height of the cylinder's bottom; and **no floor at all**
puts an unbased, non-moving pawn in `PHYS_Falling` (retail's tail at `0x580ae0+0x8a0`), which is how the Elite
reached the deck: his first sweep on the boat penetrates the boat's rail, the line finds the deck 47 units down, he
falls (Z -223 -> -270 at 69.0 s), lands and is based. Every full move also zeroes the mesh's extracted root-motion
translation (retail's `LABEL_84`). Retail's two things this does NOT do: move the pawn onto the hit
(`MoveAttachedActors`) and `FindValidLocation`'s nav-mesh snap - GA2 measured why: lowering a dock worker 17 units
onto his floor made his path query answer "arrived" without a step (`event 3, 4` at the request's first tick), so
the hover is left for the mesh offset to hide, which it does.

### 2.4 The root-motion modes (`dishonoredpawn.cpp`)

`ChangeAnimStateMotionModes` (`0x78e550`, retail's two mesh writes), `StartRootMotionMode` / `StopRootMotionMode`
(`0x78e4b0` / `0x78e4c0`), `MAT_SetRootMotionMode` (`0x7910e0`) through them, `MAT_StopRootMotion` (`0x78e500`) as
`StopRootMotionMode`. The `fkdiag aipawn` line now carries `rmm <mode>/<rotation> rmd <extracted delta> meshT <x/y/z>`.

## 3. The runs

All through `build/agentGA/ga_run.py` (`game_input.py launch`, `-inputcmd -unattended -windowed`), route
`-finewgame=50 -fosoiree -fkdiag -frdiag -diskismet` (FR's; the boat ride starts on its own at ~63 s), steps
`build/agentGA/ga_steps.txt` (shots at 78 / 110 / 150 s, quit 158 s). Captures are in-process, 1920x1080 windowed,
6,220,854 B each, all viewed (thumbnails `build/agentGA/*.png`). No `Critical`, no new minidump, `FocusLost` 0.

| run | exe | what it measured | capture |
|---|---|---|---|
| GAC1 | control `DishonoredGame_gabase.exe` (`head_rel`) | both guards at their spawn points, `base none`, for the whole ride (pos 0.02 -> 93.90); captain `physics 0` while his group's keys disable collision | `gac1_ga_a/b/c`: the player sails alone; the deck is empty at mid-ride |
| GA1 | build 1 (everything but the base) | mesh offset live on 28 NPCs (`offset z` = minus the hover); guards still `base none`; Elite `physics 2` from 74 s but not moving (`MAT_SetCollision` re-walks him every frame); Worker B walks 850 units in `Empty_Idle`; Worker A's path fails | `ga1_ga_b`: empty deck |
| GA2 | build 1 + the moving floor sweep | both guards based and riding, BUT the Elite lifted 30 units (`meshT -30.3`), six tower NPCs lifted by hundreds of units (`_12` Z 3094 -> 4684), Worker B "arrived" without walking | `ga2_ga_b`: the captain on the deck beside the player |
| GA3 | the merged tree (FV's `UpdateFloorAndBase` + GA's two cases, no pawn moved) | both guards based within 6 s and riding to the lock; every other NPC at its GA1 position; 11 of 28 NPCs based (1 on GA1); Worker B walks as on GA1 | `ga3_ga_b`: the captain on the deck beside the player; `ga3_ga_c`: the lock |

Logs: `DishonoredGame/Logs/GAC1.log`, `GA1.log`, `GA2.log`, `GA3.log`; the measured lines `build/agentGA/ga*_run.txt`.

## 4. The instrument: `-gadiag`

`UnSkeletalComponent.cpp` (`DisGAReportPawn`): once a second per pawn whose body mesh ticks - position, velocity,
physics, base, the distance from the cylinder's bottom to the ground below (`TRACE_World`; a skeletal-mesh floor like
the boat reads as the riverbed under it, 1,000+), cylinder height, step height, the mesh's root-motion modes and this
frame's extracted delta, the mesh translation / rotation, the offset component's Z and flags, and the anim-sequence node
with the most weight. `interptracklocomotion_game.cpp`: every Locomotion request and event. Nothing under the switch
changes behaviour.

## 5. Deviations, named

- `ADishonoredPawn::PreBeginPlay` does not call `PreBeginPlay_Attributes`: agent BF built it lazily from
  `GetAttributes()` because the tweaks may not be set when `PreBeginPlay` runs here, and a wrong set is cached for the
  pawn's life; `PreBeginPlay_Inventory` (`0x7c38a0`) and `PreBeginPlay_Actions` have no bodies.
- `ADishonoredNPCPawn::PreBeginPlay_NativeComponents` adds only the mesh offset; `FDisComponentHitReactionThrow`,
  `FDisComponentLODManager`, `FDisComponentRBDamages` have no bodies. `MAT_BeginAIGroup` snaps the offset whenever the
  component exists (retail asks the absent LOD manager first).
- The mesh offset is ticked from the skeletal mesh's tick (after the pawn moved), not from the component manager
  (before the actors tick); `m_ExternalRotationOffset` is never written (locomotion step 10).
- `UpdateFloorAndBase` never moves the pawn (§2.3). `CalcInitialTransform`'s tail is the Move track's.
- `GetGameSpecificLocomotionModifier` lives in the game unit beside its one caller; `KeyLocomotionModifier` is declared
  in `EngineInterpolationClasses.h` (hand-edited generated header, like FS's; the generator must learn the Locomotion
  declarations listed in the sync script). No reflected member added or moved; `gen_classes_header.py` not run.

## 6. What is left, specified

1. **The walking body**: `UArkAnimNodeLocomotion` (`Engine/Src/arkanimnodelocomotion.cpp`, comment-only) is what plays
   the walk / turn / start / stop animations for `FArkComponentLocomotion` (`m_bCanManageAnim`, `m_pAnimNodeLoco`); a
   locomotion-driven NPC here slides in `Empty_Idle` at 123 u/s. Also `FArkComponentLocomotion::UpdateMesh` (2012
   `0x57ef70`) and the pitch / roll step of `PreAsyncWorkTick`, which feed the offset's external rotation.
2. **Worker A's path**: `PATHFINDING_FAILED` every 5 s from `11555/22720` to `TargetPoint_24` (DN's A*).
3. **The captain's T-pose**: during his group's keys with `None` channels he stands with his arms out (`ga3_ga_b`);
   the stack's child 0 is at weight 0 with no sequence - FS's §6.4 weight model.
4. `UDisSkelControl_FootPlacement::InitializeFootProperties` (2012 `0x6da640`) is what fills `m_FeetProps`; without
   it the offset always runs the single-trace branch.
5. Retail's `FindValidLocation` / `MoveAttachedActors` snap in `FullMovePawn` (§2.3), once the nav-mesh query tolerates
   a pawn whose cylinder bottom is on the floor.

## 7. Acceptance

| | |
|---|---|
| `run_regression.py` | **47 ok, 0 failed, 0 skipped**, 4,441 s - the worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentGA_reg`, built inside the harness (the package's second build), `--newgame-at 50 --game-extra-args=-windowed`, the five gitignored `resources/docs/types/` inputs present. `unported_natives 0 (<= 0)`, `inputtest_criticals 0`, `newgame_criticals 0`, `options_criticals 0`, `d3d9_criticals 0`, `nullrhi_criticals 0`, `layout_mismatching 0`, `newgame_pawns 29`, `inputtest_moved 969.1`, `d3d9_startup_seconds 5.1`. `build/agentGA/ga_regression.log`, `build/agentGA_reg/regression/summary.txt` |
| the builds | `build/agentGA_rel` (working, four passes: one missing include fixed, then the floor helper twice, then the merge) and the gate's `build/agentGA_reg`: **0 errors, 0 C4263, 0 C4264** in both final logs (`build/agentGA/ga_build3.log`, `build/agentGA_reg/regression/build_DishonoredGame.log`) |
| `rva_sweep.py` | 0 unknown / 0 mislabelled on every touched unit; every address in this report resolved by hand in both databases (`build/agentGA/ga_resolve.txt`, 110 function headers from the two decompile runs; the 2013 side by name) |
| `build/agentGA_sync.py` | 20 files (18 source, 2 docs), **0 flagged, 0 missing** against main `02eb527`, and the worktree's `git diff --name-only 02eb527` + `ls-files --others` is exactly the sync list (the script checks both ways) |
| the merge | main `02eb527` merged into the worktree (`1ef5677`); four conflicts, all both-sides appends, resolved by keeping both; FV's `UpdateFloorAndBase` kept with GA's two cases folded in; the merged tree is what GA3 and the gate ran |
| headers | `gen_classes_header.py` not run; no reflected member added or moved; one generated Engine header hand-edited (`EngineInterpolationClasses.h`: the Locomotion declarations and `KeyLocomotionModifier`) for FX's generator to learn; no `dishonoredgame*classes.h`, `DishonoredGameRegistrants.cpp` or `Sources.cmake` edit |
| before/after | §3: GAC1 (control) -> GA1 -> GA2 -> GA3, one route, same driver, same script, matching world times |
| the rules | every launch `game_input.py launch` + `-windowed` (`presented frame (1920x1080, windowed)` in all four logs); no real input, no focus change; builds through `build_queued.py` only (queued behind FV's, FZ's and the gate's builds for most of the afternoon); no retail `Dishonored.exe` launch |
