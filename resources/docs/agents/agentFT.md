# Agent FT — the look-at component, so a scene's pawns look at things

Wave 21 (`PHASE17.md`), package FT. Worktree `build/agentFT_wt`, base `d7d083a`, with main's `e60c5c6`
(fullscreen refused under `-inputcmd`), `d24f1bf` (agent FS) and `d24f1bf..871395b` (agent FR, the gate bound)
applied by `patch -p1 -d` mid-package. Fifteen changed source files, five new ones, nothing of FS's or FR's
touched beyond additions in four shared files (section 4.3).

## 1. The short version

**`FArkComponentLookat` exists, is created on every NPC in `Spawned`, hands `UArkAnimNodeLookAt` its config,
and the AI's look-at desires reach it and move heads.** On the New Game route a civilian in `l_tower_p`
(`DishonoredNPCPawn_19`, the one the AI gives a procedural patrol look-at) turns its head to the aim the
component computes, and the two scene pawns the user sees stay level at rest. The magnitude question FL could
not answer is answered by the two-delta diagnostic: **the head turns the angle the component asks for, 1:1**.

| the brief said | what is true |
|---|---|
| "when `m_pConfig` becomes non-NULL, `UpdateNodeWeights` fed a raw (0,0) puts 1.00 on the grid centre, worth 60° of head" (FL's trap) | **It puts 1.00 on BOTH centres and the head is level.** Retail's rest state with a config is exactly `4:ADD_Empty_AimHeadIdle_Center=1.00 13:ADD_Empty_AimTorsoIdle_Center=1.00` (measured, §3), and `head_jnt` reads **l1/c1** against the control's l2/c2. FL's 60° came from the head centre alone with the torso set at zero — the two grids are deltas from one base and cancel at the centre; FL's own caveat said as much. The assertion to keep is now "two centres at 1.00 and `head_jnt` within 2°", not "all zero" |
| "`CheckAnimsUpToDate` ... editor and AnimTree Ed" (FL §6, not ported) | **It is the function that chooses which eighteen poses the node plays.** `SetConfig` calls it; it copies the current mode's `Head_AnimName_LU..RD` / `Torso_AnimName_LU..RD` into `Anims(0..17).AnimName` and rebuilds the two `BlendInfos` ranges from the mode's eight range angles (2013 rva `0x512570`, 1,653 bytes). Mode 0 (`EmptyIdle`) of `DefaultLookatConfig` names the same `ADD_Empty_AimHeadIdle_*` set the cook left on the node, which is why FL's measurements of the cooked set were measurements of mode 0 |
| "`OnComposeSkeleton` ... the eyes" (FL) | The eyes, **and only while FaceFX is playing**: the whole body is behind `m_bFaceFxIsPlaying` (2012 `0x593220`, the first test). It clamps what FaceFX wrote to the two eye bones. `WITH_FACEFX` is compiled out of this tree, so it is ported and never runs |
| "`DisNotify_NPCDisableLookAt` ... becomes live the moment `FArkComponentLookat` lands, and it is two 91-byte functions" (FL §5) | **It cannot land yet.** `Notify` sets a disable bit on the component and `Notify_EndRange` clears it; this tree's `UAnimNotify` has no `Notify_EndRange` virtual (no notify ranges at all), so porting `Notify` alone would disable the look-at for good on the first notify. Named, not written (§6) |
| "`FArkComponentLookat::Starting / Stopping` unnamed in the 2013 db" | Resolved: vtable slots 7 and 8 of `??_7FArkComponentLookat@@6BFArkComponentBase@@@` at 2013 VA `0x10a6694` are **`0x556660` (915 bytes) and `0x556a00` (141 bytes)**; `match_2012_2013.csv` agrees (0.879 / 0.771 by vtable) |
| the PDB's `FArkCpntFaceToProp::OverwriteExactRotation` at 2012 `0x58df40`, attributed to `arkcomponentlookat.cpp` | It is **`FArkComponentLookat::SetConfig`**, a one-store function ICF folded with the face-to one; `ADishonoredNPCPawn::Spawned` calls it on the look-at component with `UDisTweaks_NPCPawn::m_pComponentLookatConfig` (2013 `0x54d370`) |
| "against retail with dismod if you can reach it" | **Not taken, and withdrawn by the coordinator mid-package.** One retail launch happened before the withdrawal (`build/agentFT/dismod_run_ft.ps1`, `l_tower_p` on the command line, `-windowed`); retail ignored the map and sat on its menu, the probe captured only its banner (`build/agentFT/retail/lookat.log`), and no second launch was made. The magnitude is measured against the shared control and against the component's own request instead (§3) |
| (not in the brief) every NPC's bones were scaled 2^depth once the component ran - a coat over the boat view, a canopy over the guards | **The look-at's `Blink` channel on the `BlinkBlender` stack, summed by the stock evaluator.** Retail PC composes the skeleton through EdgeAnim jobs (`UpdateSkelPoseBegin/End`, `GEdgeAnimManager`), whose leaf honours `m_bDontUseRefPoseOnNonDefinedBones` and so layers a stack's eyelid-only channel over the source; the tree's stock `UAnimNodeBlendBase::GetBoneAtoms` added it as a second full pose. Ported retail's `UAnimNodeBlendBase::ChildrenHasChanged` (+ its `SetAnim` call) and wrote `UArkAnimNodeStack::GetBoneAtoms` with the Edge leaf's masking - the one deliberate deviation (§3) |
| "the spine bender ... a silent no-op today" | Its six functions are ported and it is live; it is still a no-op in effect, because the pawn side that sets its targets (`Tick_SpineBending` and three helpers, reached from `ADishonoredPawn::Tick_Body`, which this tree has not ported) is not there. With both targets at the cooked zero it composes the bone it is given (§6) |

## 2. What was ported

### `FArkComponentLookat` — 36 of 51, in `DishonoredGame/Src/arkcomponentlookat.cpp`

Declared in `DishonoredGame/Inc/arkcomponentlookat.h` for agent DN's reason: the config and node classes live in
`DishonoredGameEngineShims.h`. 636 bytes, every member at the 2012 PDB's offset, `checkAtCompileTime`'d; the
request manager is DN's `FArkRequestManager<T>` (request 80 bytes, as the PDB says).

| | 2013 rva | 2013 size | 2012 rva | state |
|---|---|---|---|---|
| constructor | `0x555d60` | 1,147 | `0x5968f0` | ported; the six springs' tuning read off 2012 `.rdata` `0xcb204c..0xcb2088` (TURN 100/7, RANGE 50, EYES 50000, eye limits ±25 / −15..5) |
| `Starting` | `0x556660` | 915 | `0x596ee0` | ported. Bones `camera_jnt`→`head_jnt`, `eye_L_jnt`, `eye_R_jnt` (+parents); controls `EyeControl_Left/Right`; `BlinkBlender` child 1 at 1.0 and `Blink` rewound; the node by `LookatBlender`; the spine bender by the body tweaks' `m_SpineBenderName`; `SetConfig` on the node; queue init; LOD event 2 registered |
| `Stopping` | `0x556a00` | 141 | `0x597260` | ported |
| `PreAsyncWorkTick` | `0x555b40` | 162 | `0x596750` | ported; `m_bFaceFxIsPlaying` is FALSE (no FaceFX in this tree) |
| `UpdateVariablesFromRequest` | `0x553fb0` | 2,668 | `0x594a70` | ported |
| `UpdateHeadAndTorso` | `0x553050` | 2,341 | `0x593c50` | ported; the face-to angle is 0 (`FArkComponentFaceTo` unported, retail's own NULL branch) |
| `UpdateEyes` | `0x54e960` | 1,200 | `0x58f600` | ported |
| `UpdateBlink` | `0x552ec0` | 388 | `0x593ac0` | ported |
| `OnComposeSkeleton` | `0x552640` | 814 | `0x593220` | ported, unreachable without FaceFX |
| `ProceduralLookAt` | `0x552970` | 604 | `0x593550` | ported |
| `GetLookEyesPosition` | `0x54d380` | 961 | `0x58df50` | ported (head bone: +X up, +Y right, −Z at) |
| `GetLookAtPosition` | `0x552bd0` | 751 | `0x5937b0` | ported; the pawn virtual at 2013 vtable +1004 is read as `APawn::GetPawnViewLocation` (2012 vtables.csv slot 247 for `APawn`), the one view-location virtual a pawn has |
| `GetCurrentLookAtDirection` / `GetLookAtNeutralDirection` / `GetHeadAt` | `0x54d760` / `0x54d8b0` / `0x54d750` | 329 / 310 / 7 | | ported |
| `LookAt` ×2, `AimAt` ×2, `LookForward` | `0x54e5e0` `0x54e690` `0x54e740` `0x54e7f0` `0x54e8a0` | | | ported |
| `DynamicInterpolator<float>::Update` / `<FVector2D>::Update` | `0x54dff0` / `0x54e240` | 589 / 926 | | ported |
| `StartLookForward`, `StartLookAtActor`, `StartLookAtLocation`, `StartAimAtActor`, `StartAimAtLocation`, `StartProceduralLookAt`, `StartProceduralLookAtOnActor`, `StartProceduralLookAtOnLocation` | `0x554a80`..`0x554ef0` | 130–175 | | ported |
| `StopLookAt`, `StopAllLookAtFromAsker`, `GetActiveRequest`, `GetActiveRequestPriority`, `GetTimeLeft` | `0x555710` `0x555750` `0x554f90` `0x554fd0` `0x555010` | | | ported |
| `SetLookAtMode`, `SetDebugLookAtMode` | `0x54e920`, 2012 `0x58f5b0` (unmatched in 2013; its caller is the cheat manager's `LocoForceLookAtMode`) | | | ported |
| `ManageReferences`, `GetAllocatedSize`, `GetMemoryFootprint`, `OnLODChanged`, `GetLookatComponent`, `SetConfig`, the type trio | | | | ported |

**The two seams.** Retail ticks the component from `FArkComponentManager::Tick` (2013 `0x53a6b0`), policy slot 1
`PreAsyncWorkJustBeforeProceduralAnim`, and calls `OnComposeSkeleton` from `USkeletalMeshComponent::UpdateSkelPoseEnd`
(`0x3471d0`) right after `ComposeSkeleton`. Neither the manager nor the Dishonored split of `UpdateSkelPose` is
ported, and Engine cannot see a DishonoredGame class, so `Engine/Inc/arkcomponentbase.h` declares two function
pointers that `UnSkeletalComponent.cpp` tests on every call — one in `Tick` just before `TickAnimNodes` on the pawn's
body mesh, one in `UpdateSkelPose` after `ComposeSkeleton` — and `arkcomponentlookat.cpp` installs them from a static
initialiser. The within-frame order is retail's: request queue → target → eyes → head/torso aim → the node's
`TickAnim` reads it the same frame. One divergence: retail ticks components whether or not the mesh is rendered; this
tree's hook sits inside the `bRecentlyRendered || bTickAnimNodesWhenNotRendered` branch, so an unrendered NPC's
look-at pauses with its anim tree.

### `UArkAnimNodeLookAt` — the config half, in `DishonoredGame/Src/arkanimnodelookat.cpp`

| | 2013 rva | 2013 size | state |
|---|---|---|---|
| `BlendInfos::{UpdateRanges, UpdateDirection, Map, GetP1P2, IsOutside, MapIntoNormalizedSpace, staticInit}` | `0x50a140` `0x50c4c0` `0x5175c0` `0x50a370` `0x50a430` `0x511bc0` `0x50a2b0` | 362 234 296 188 228 2,479 185 | ported, 88-byte class, the corner order (centre, right, right-up, up, left-up, left, left-down, down, right-down) read off `staticInit`'s circle and `UpdateRanges`' permutation |
| `ComputeHeadAndTorsoAim` | `0x5176f0` | 1,111 | ported |
| `HandleBlendingBetweenLookatNodes` | `0x50c650` | 897 | ported; the crossfade branch of `GetBoneAtoms` it makes reachable is filled too (in `dishonoredplayercontroller.cpp`) |
| `CheckAnimsUpToDate` | `0x512570` | 1,653 | ported |
| `SetConfig`, `SetLookAtMode`, `SetUseHeadRange`, `SetUseTorsoRange`, `SetUseTorsoDeadZone`, `GetMaxAim`, `GetAnimNodeLookAt`, `BeginDestroy` | `0x50a090` `0x50c410` `0x50c440` `0x50c480` `0x50a110` `0x50c5b0` `0x517580` `0x50a820` | | ported |
| `TickAnim`'s config branch | inside `0x526320` | | filled (FL's shell, FL's switches untouched) |
| `UArkComponentLookatConfig::InitProceduralLookAt` / `UpdateProceduralLookAt` | `0x53c6b0` (the callee of the next, unnamed) / `0x53c8a0` | — / 1,678 | ported |

**One deliberate divergence.** Retail's `UArkAnimNodeLookAt` constructor `appMalloc`s the two `BlendInfos`; this tree's
shim class has no C++ constructor, so `EnsureBlendInfos` makes them on first use (`SetConfig`, `GetMaxAim`,
`ComputeHeadAndTorsoAim`) and `BeginDestroy` frees them as retail does.

### The wiring

* `ADishonoredNPCPawn::Spawned` (`dishonorednpcpawn_locomotion.cpp`): DN's locomotion block moved whole into
  `SpawnedLocomotion`, and the look-at component created **unconditionally** after it as retail does (2013 `0x752270`),
  `SetConfig` + `m_bStarted` + `Starting`. `GetComponentLookat` (`0x74b3f0`) added.
* `FDisLookAtRequest::DoRequest` / `SyncRequest` / `ClearRequest` (`disdesirestructs.cpp`): the six retail `Start*`
  calls with retail's arguments (speed 0.5, pattern 4 = `DisLookAtProceduralPattern_MAX` = "not procedural", the asker's
  name), `StopLookAt`, `GetTimeLeft` written back into `m_fDuration`. DF's counters stay, and a pawn without the
  component keeps DF's synthetic id.
* `UDisSkelControl_SpineBender` (`disskelcontrol_spinebender.cpp`, off the exclude list): `TickSkelControl` `0x68cbe0`,
  `CalculateNewBoneTransforms` `0x68cce0`, `EnableSpineBending` `0x67eb90`, `IsSpineBendingEnabled` `0x67ebe0`,
  `SetBendPitchTarget` `0x684370`, `SetBendYawTarget` `0x6843d0`. Declared through a new CppText file, with the one
  include line the generator emits for it hand-inserted in `DishonoredGameAnimClasses.h` (no reflected member moved, so
  no regeneration); same for `UArkComponentLookatConfig` in `DishonoredGameEngineShims.h`.

## 3. The measurement

Four runs through `game_input.py` (`-windowed` on every one), `-flheaddiag` every 3 s, the control at `build/head_rel`
staged as `DishonoredGame_FTHEAD.exe`, this package as `DishonoredGame_FT.exe`; logs `FT_{head,ft}_{ng,tw}.log`,
frames `build/agentFT/shots/`. `ng` is the user's route (`-playdefaults -finewgame=50`, l_tower_p at t≈72), `tw` is
`-startmap=l_tower_p -startmapopen`.

**The scene pawns at rest** (the two the opening scene spawns, `DishonoredNPCPawn_27` 249 units from the camera and
`_26` at 343, same distances in both runs, `-apshottime=115`):

| | control `head_rel` | this package |
|---|---|---|
| node | `config NULL ... nonzero: all zero` | `config set ... head 0.00 torso 0.00 ... nonzero: 4:ADD_Empty_AimHeadIdle_Center=1.00 13:ADD_Empty_AimTorsoIdle_Center=1.00` |
| component | — | `started 1 config DefaultLookatConfig node set mesh set headbone 10 eyes 31/34 ctrls 1/1 blink 1/1 spine set`, `requests 0 type 1 target none notlooking 1` |
| `neck_jnt` / `head_jnt` | l0/c0 / **l2/c2** | l0/c0 / **l1/c1** |

So the rest pose with the config is retail's two-centre state and it is level. FL's trap does not bite, for the reason
FL suspected: the centre of the head grid and the centre of the torso grid are deltas from a shared base, and together
they are the identity to within the idle animation's breathing.

**A pawn that is asked to look** — `DishonoredNPCPawn_19` (`skm_CivMidClass02_Body`) and `_20`, the two civilians the
AI gives a procedural patrol look-at (`type 4`, head influence 1.00, torso 0.00; FL's log line "StartProceduralLookAt
is not ported" is gone from both of this package's runs):

| time | component | node | `neck_jnt` | `head_jnt` |
|---|---|---|---|---|
| `ft_ng` t 63.0 | `cursor(-10.0,-5.1)deg` | `aim(-10.04,-5.05) blended(-0.18,-0.09)` `CenterLeft=0.13 DownLeft=0.02 Center=...` | l4/c4 | **l8/c11** |
| `ft_ng` t 66.0 | `cursor(-15.8,-8.1)deg` | `aim(-15.80,-8.11) blended(-0.28,-0.14)` `CenterLeft=0.19 DownLeft=0.06` | l6/c7 | **l12/c17** |
| `ft_ng` t 75.1 | `cursor(0.9,1.2)deg` | `Center=0.97 Up=0.02` | l1/c1 | **l1/c1** |
| `ft_tw` t 13.2 (`_20`) | `aim(-24.19,-11.64) blended(-0.42,-0.20)` | `CenterLeft=0.25 DownLeft=0.12` | l9/c9 | **l17/c26** |
| `ft_tw` t 46.2 (`_20`) | `cursor(-0.0,-0.0)` | two centres at 1.00 | l1/c2 | **l1/c1** |
| control, the same pawns, any time | — | `config NULL`, all zero | l2/c4 | l3/c1 |

**The magnitude.** The requested angle at t 66.0 is √(15.8² + 8.1²) = 17.8°; `head_jnt` composed is 17°. At t 13.2 the
request is √(24.2² + 11.6²) = 26.8°; `head_jnt` composed is 26°. The head turns the angle the component asks for,
within the diagnostic's rounding, and the aim rides on `neck_jnt` + `head_jnt` as the additive poses say it should
(the spine stays within 1°). Against the control the same pawns read 3° at the head and never move. The retail
comparison itself was not taken (§1); what FL asked for — "drive both grids from one direction and measure" — is what
these rows are.

**The frame that was not explained, explained.** The first pair of runs had this package's t 115 frame showing an
NPC's coat over the whole view and the tower route a canopy over the guards, with every pawn and camera number equal
to the control's. The sweep added to `DisFLHeadReport` (every NPC pawn: NaN bones, bounds radius, farthest bone)
found every NPC body with a bounds radius of ~12,000 and its fingers 170,000 units from the mesh, every bone's local
scale 2.000, 2^depth at the leaves. Not a bone at the origin and not the spine bender (skipping its base call gave
the same numbers; the rotation and translation flags were read and are harmless): `BlinkBlender`, the
`ArkAnimNodeStack` the look-at component's `UpdateBlink` fades its channel 1 into, had `Source` at 1.00 and
`Channel 01` (the `Blink` sequence, eyelids only, `bIsAdditive` 0, `Npc_EliteGuard_Blink_as`) at 1.00, and the stock
`UAnimNodeBlendBase::GetBoneAtoms` sums non-additive children by weight - two full poses, translation and scale 1+1.
Retail's stack is the base class's (`0x574db0` jumps to `UAnimNodeBlendBase::GetBoneAtoms`, which is stock
including the `BlendType != Linear` gate on its normaliser) and retail's `UpdateBlink` and `Starting` write the same
weights, so retail should double too - except retail PC does not evaluate the skeleton with `GetBoneAtoms`:
`USkeletalMeshComponent::UpdateSkelPoseBegin/UpdateSkelPose/UpdateSkelPoseEnd` (2013 `0x754510`, `0x754f1b`,
`0x747239`) run it through `GEdgeAnimManager` jobs (`FEdgeAnimData`, `FEdgeAnimJobDesc`), and the only reader of
`UAnimNodeSequence::m_bDontUseRefPoseOnNonDefinedBones` in either binary is `BuildEdgeAnimTreeLeaf` (2012
`0x1a474f`, 2013 `0x59871f`). The stack sets that flag on every channel sequence in `InitAnim`
(`ForceDontUseRefPoseOnChild`); in the Edge tree it masks the bones a channel's animation does not define, so a stack
layers partial channels per bone. This evaluator has no EdgeAnim. Two engine functions follow from that:

- `UAnimNodeBlendBase::ChildrenHasChanged( const UAnimNode*, UBOOL bChildIsAdditive )` - retail's last vtable slot
  (2012 `0x19f260`, 1,084 bytes, no override anywhere), called at the end of `UAnimNodeSequence::SetAnim` (2012
  `0x1a41f0`) for every parent: fixes the child's `bIsAdditive`, renormalises the non-additive weights when it changed,
  derives the blend's own `bIsAdditive` (only additive children active) and passes it up. The tree's `SetAnim` had no
  such call, so a sequence whose animation is chosen at run time (the look-at node's additive aim poses) kept its
  cooked child flag. Ported as written.
- `UArkAnimNodeStack::GetBoneAtoms` - not a retail function: Source is the base, each channel is lerped over it by
  its weight on the bones its sequence has a track for when `m_bDontUseRefPoseOnNonDefinedBones` is set (all bones
  otherwise), additive channels accumulate. This is the Edge leaf's masking written for the stock evaluator, and the
  one deliberate deviation in this package; it is why FS's "Source = 1 - channels" complement in
  `MAT_SetAnimWeights` ("to verify") is harmless here.

After those two: `flhead SUSPECT` 0 on both routes (`FT_ft_tw13.log`, `FT_ft_ng7.log`, 0 crashes), the t 40 tower
frame is the dock with the guards and the t 115 boat frame is the two guards on the boat (`shots/ft_tw1300000_small.png`,
`shots/ft_ng700000_small.png`), the civilians still turn their heads (`_19` at `aim(-5.58,-11.45)` to `CenterLeft 0.06
DownLeft 0.03 Center ...`) and the scene pawns at rest read `head_jnt` l1-2/c0-1 against the control's l2/c2.

## 4. Acceptance

1. Before/after against the shared control through the same driver: §3, `FT_head_ng.log` vs `FT_ft_ng.log`,
   `FT_head_tw.log` vs `FT_ft_tw.log`.
2. `run_regression.py` from this worktree's copy with an absolute build dir, built in-harness
   (`--build-dir D:/RecompileDishonored/Recompile/build/agentFT_reg --exe-name DishonoredGame_FTREG.exe
   --log-prefix FTREG`, on the worktree with main's `871395b` merged, the gate's own clean build):
   **47 ok, 0 failed, 0 skipped, 1311 s**, `build/agentFT_regression.log`
   (`build/agentFT_reg/regression/summary.txt`).
3. Own working builds: `build/agentFT_build4.log` through `build/agentFT_build14.log` (0 errors, 0 C4263, 0 C4264 on
   every green one). Builds 0-3 are the same tree failing on three typos (`Clamp` with a double `PI`, a protected
   `Owner`, a 30-argument `debugf`, an enum through `Logf`) and one `ninja: failed recompaction` while another agent's
   build held the directory; 5-13 are the diagnostic rounds of section 3 (one `ScalarRegister` argument fixed in 14's
   first try). Main's `e60c5c6` and `d24f1bf` (agent FS) were merged into the worktree between builds 4 and 5 with
   `patch -p1 -d`; FS's files were taken whole and three shared files hand-merged: `ADishonoredNPCPawn.h` (FS's
   `MAT_*` block after my two declarations), `DishonoredGameEngineShims.h` (FS removed `UArkAnimNodeStack`, my one
   include line kept), `UnSkeletalComponent.cpp` (FS's `PostInitAnimTree` hunk and my two hook calls). Every FS file
   not of mine was verified byte-identical to main after the merge; section 3's engine work then added to two more of
   FS's (`EngineAnimClasses.h`, `arkanimnodestack.cpp`). Main's `d24f1bf..871395b` (agent FR's conversation system,
   the gate bound `d1f282c`, `game_input.py` `080b322`) was patched in the same way before the gate (one hunk at an
   offset, no rejects: FR's block in `ADishonoredNPCPawn.h` sits after mine, its `Sources.cmake` lines are elsewhere),
   build 15 re-measured both routes on it (`FT_ft_tw14.log`, `FT_ft_ng8.log`: 0 `SUSPECT`, 0 crashes, the heads
   turning - FR's boat ride now plays on its own, so the opening scene's pawns row instead of standing).
4. `rva_sweep.py --path DishonoredGame/Src`: 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013 from this package (the one
   remaining is the pre-existing `disbehaviorpatrol.cpp:319`). Every address above was resolved by hand in
   `retail2013_named.i64` / `shipping2012_v1.i64` with its start and size; the two the 2013 database leaves unnamed
   (`Starting`, `Stopping`) are placed by the vtable bytes and the match table independently, `InitProceduralLookAt`
   by being `UpdateProceduralLookAt`'s callee.
5. This file and `agentFT_status.csv`, in the worktree.
6. `build/agentFT_sync.py`, naming every file and function, checked against `git diff --name-only d7d083a` and
   `git ls-files --others --exclude-standard`: every file the diff lists that is not in the sync list is main's own
   `e60c5c6`, `d24f1bf` or `871395b` applied here by patch and byte-identical to main. The script treats a main file
   whose content equals its version at one of those commits as unchanged, so the files shared with FS and FR
   (`EngineAnimClasses.h`, `arkanimnodestack.cpp`, `UnSkeletalComponent.cpp`, `ADishonoredNPCPawn.h`, `Sources.cmake`)
   copy cleanly while main stays there and flag the moment it moves.

## 5. The diagnostics added

`-flheaddiag` (FL's) now prints, per pawn, two more lines: the node's mode and crossfade state
(`lookatnode modes cur/target/stacked/prev fade blendnode cfgmodes`) and the component (`lookatcpnt started config node
mesh headbone eyes ctrls blink spine`, then `requests type target notlooking flags head torso cursor to`, and the active
request's data when there is one); the `seqs:` line names each playing sequence by full path. After the four nearest
pawns it sweeps every NPC pawn and prints `flhead SUSPECT` for any with a NaN local atom, a NaN aim, or a body bounds
radius over 2,000 - with the farthest bone's name, component-space position and scale, the count of bones beyond 500
units, the root's scale and the mesh origin - which is the line that found section 3's doubling. Each spine bender
reports once (`flhead spinebender`: strength, rotation, the four apply/add flags and spaces, the targets). Five counters in `arkcomponentlookat.cpp` (`GDisLookatComponents`, `Started`,
`RequestsStarted`, `RequestsStopped`, `ProceduralTicks`) are there for a census line to pick up.

## 6. Inventoried, not ported

`FArkComponentLookat`, the 15 of 51 not written: the three class-registration functions and the two `GetName`
helpers are the macro's; `FArkComponentCreatorRegister::CreatorFn<FArkComponentLookat>` is `ARKCOMPONENT_IMPLEMENT_TYPE`;
`GetRequestIdxByID` / `RemoveRequestByIdx` / `RemoveAllRequestsFromAsker` / `Update` are DN's template; `IsOfType` /
`GetType` / `GetInternalName` are `ARKCOMPONENT_DECLARE_TYPE`; the eight `_dynamic_initializer_for_s_*Name` are the eight
`FName` statics. Nothing of the component is left unwritten; what is left is **around** it:

| | 2013 rva | size | why not |
|---|---|---|---|
| `FArkComponentManager` and its policies | `0x53a6b0` `Tick`, `0x539240` `FArkComponentPreAsyncWorkPolicy::Tick` | | the two hooks stand in for the one policy this component needs; locomotion is driven from `physWalking` the same way |
| `ADishonoredNPCPawn::Tick` → `SetLookAtMode( ChooseLookAt() )` | `0x76da40` / `0x7794d0` (447 bytes) | | the NPC `Tick` is unported; `ChooseLookAt` reads the equipped item's `GetDefaultNPCLookAtMode` and the transit speed, neither ported. Every NPC stays in mode 0 (`EmptyIdle`), which is right for a standing guard with empty hands and wrong for one with a sword (mode 1) or walking (3) |
| `ADishonoredNPCPawn::Tick_SpineBending`, `Tick_SpineBending_Apply`, `IsActionSpineBendingEnabled`, `IsSlopeOffsettingEnabled` | `0x77fbd0` `0x77b3d0` (2,006) `0x7772c0` `0x77b300` | | the spine bender's driver (slope and turn offsetting: `GetSpineBendRefPoint` → `GetFocusBone_ByRegion`, `ComputeTurnAngle`, `GetCurrentTransitSpeed`, the face-to angle), reached from `ADishonoredPawn::Tick_Body`, unported |
| `UDisNotify_NPCDisableLookAt::Notify` / `Notify_EndRange` | `0x68e320` / `0x68e3d0` | 167 / 91 | needs `UAnimNotify::Notify_EndRange` (notify ranges) in Engine first; §1 |
| `UArkAnimNodeLookAt::HandleProceduralLookatInAnimTreeEd`, `Render`, `GetSlider*`, `HandleSliderMove`, `GetSliderDrawValue`, `BuildEdgeAnimTree`, `FDynInterpFVector2D::Update` | `0x526000` `0x50a520` ... `0x50c080` | | AnimTree editor and `GIsEditor` only |
| `FArkComponentFaceTo` | | | the face-to angle `UpdateHeadAndTorso` subtracts is 0 without it; FS's package |
| `UInterpTrackLookAt::StaticUpdateTracks` (`0x9082c0`, 3,386 bytes), `UInterpTrackInstLookAt::TermTrackInst`, `UDisSeqAct_NPCTrackTarget::ActiveLookAt`, `ADishonoredNPCPawn::MAT_BeginAIGroup` → `StartLookForward`, `MAT_SendAimStopRequest` → `StopLookAt`, `FArkComponentLocomotion::UpdateLookat` (`0x943490`) | | | the component's other callers; the scene's look-at track is FS's, and it calls this component |
| `FDisComponentLODManager` raising LOD event 2 | | | registered for, never raised; `m_bInLOD` / `m_bNoTick` stay 0 |

## 7. Files changed

See `build/agentFT_sync.py`'s docstring for every function per file; the Engine side is six files (`arkcomponentbase.h`, `UnSkeletalComponent.cpp`, and section 3's `EngineAnimClasses.h`, `UnAnimTree.cpp`, `UnAnimPlay.cpp`, `arkanimnodestack.cpp`). The dismod additions (`src/mods/ftlookat.cpp/.h`,
one line each in its `CMakeLists.txt` and `library.cpp`, off unless `DISMOD_FTLOOKAT=1`) live in the user's
`D:\Christmas\github\dismod` and are not merged by the sync script; they are additive and marked `dismod-harness`.
