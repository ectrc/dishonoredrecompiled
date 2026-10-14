# Agent GH — the stretched NPCs: every NPC bone was at 3.00x its reference translation, added by the look-at node, not by the cinematic sequences

Wave 24 (`PHASE20.md`), package GH. Worktree `build/agentGH_wt` on branch `agentGH`, base `4b889ff` (main after GD), main `99eaf6b`
merged by `git merge` before hand-back (clean). Working build `build/agentGH_rel` (Release, `DISHONORED_LAYOUT_CHECKS=ON`,
`build/agentGH/gh_build.cmd` through `build_queued.py`, `gh_build0.log`: 0 errors, 0 C4263, 0 C4264); the gate's in-harness build
`build/agentGH_reg` (`gh_gate.cmd`). Sync `build/agentGH_sync.py` (5 files, every function named). Control `build/head_rel` (main at
`4b889ff`, rebuilt 15:04) staged as `DishonoredGame_GHbase.exe`; the working exe `DishonoredGame_GH.exe`; the gate's exe
`DishonoredGame_GH2.exe`. Every launch `game_input.py launch` with `-windowed` and isolated inis (`build/agentGH/config/<log>/`);
no real input; no retail `Dishonored.exe`; no FModel. IDA: private copies `build/agentGH/idb/GH_retail2013.i64` /
`GH_shipping2012.i64`, read headlessly (`resources/tools/ida/run.py build/agentGE/ge_decomp.py`, the MCP workers were all taken).

## 0. The answers

1. **The stretch is not the cinematic sequences' evaluation.** Measured per bone (`-ghdiag`, §2): on every NPC - the idle tower
   guards included - every bone's local translation is **exactly 3.00x** its reference length (`Root_jnt` 325.3 vs 108.7,
   `spine_0_jnt` 17.55 vs 5.85, `lower_leg_L_jnt` 137.0 vs 45.7, the face bones too), scale 1.00. The pose that leaves the matinee
   stack / `PoseBlender` is at true scale (`maxRatio 1.00`, 2 of 118 bones outside 5 %, reach 190); the node that turns it into
   3x is `AnimNodeAdditiveBlending` whose additive child is `LookatBlender` (`UArkAnimNodeLookAt`, weight 1.00), and that node's
   own output is **2.00x the reference translation on every bone** (`maxRatio 2.00 at Root_jnt`, all 114 measurable bones off).
2. **Why**: `ADishonoredNPCPawn::ChooseLookAt` returns `eDisNPCLookAtMode_MAX` for a pawn with no equipped item (this tree's
   inventory is empty - GF §2, GJ's package), the component clamps it to the config's last mode, whose eighteen pose names
   (`ADD_Empty_AimHead_BowIdle_*` / `ADD_Empty_AimTorso_BowIdle_*`) exist in no cooked set (1,170 `Failed, with animsequence`
   warnings in `GH1.log`, 36 per guard), so `SetAnimInfo` leaves every pose's `AnimInfo.AnimSeq` NULL. FT's
   `GetBoneAtomsForSet` - retail's CPU body line for line - hands the unresolved pose to `GetAnimationPose`, which fills the
   **reference pose**; the two sets (head, torso) each contribute a whole reference skeleton as an additive delta, so the body
   gets +2x every reference translation and the reference rotations composed again (the face bones read 11.9°).
3. **Retail PC never runs that CPU body.** It evaluates a set through the Edge tree `UAnimNodeSequenceBlendBase::BuildEdgeAnimTreeEx`
   builds (2012 `0x1a62b0`, 2013 `0x19a280`): a pose is relevant only when its weight passes the threshold AND its
   `AnimInfo.AnimSeq` is set with a byte stream (`+224 > 0`); the relevant poses are lerped in a chain whose alphas renormalise
   their weights; with none relevant the set is `UAnimNodeSequence::BuildEdgeAnimTree` (2012 `0x1a4a20`, 2013 `0x1989f0`): the
   node's own sequence as one leaf. Ported into `GetBoneAtomsForSet` (§3). One stand-in named: a node without an Edge sequence
   of its own gets the additive identity where retail emits a cached-pose leaf (`EdgeAnimBlendLeaf.flags` 4).
4. **Premises corrected** (§1): the task's three suspects (the track-to-bone linkup, the Edge translation decode, the stack
   stand-in) are all measured clean; "148 is the right height" is wrong - 148 is the 3x pose's Z extent (the skeleton stands
   along -Y in component space; its true Z extent is 45-65), so the acceptance number changes with the fix (§4).
5. Gate: §6. Builds used: two (the working `agentGH_rel` with the instrument, the gate's `agentGH_reg` with the fix).

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "the evaluation of those Edge sequences (78 joints, 0+1 translation channels, on 118-joint skeletons)" | the sequences evaluate right: offline (`build/agentGH/gh_pose.py` over `EdgeAnimSmoke --dump`, `smoke_dump*.txt`) and in-game (`ghdiag edge`: `BoatCpt`/`Posture_IdleBack` through `FEdgeAnimSequencePose::GetAnimationPose` → `transOff 0 rotOff 0 maxRatio 1.00 reach 190.1`). `Tall_SC_Tower` has `T 4+1` (four constant, one animated translation channel), not 0+1; `Empty_Idle` `T 3+1` |
| "untracked bones must take the ref pose, not track 0" | they do: `EdgeAnimEvaluateSkeletonPose` maps by joint-name hash with the rolling hint and fills the base pose for the 40 unchannelled joints (`anim->skel mapped 78 unmapped []`) |
| "a rotation-only track reading translation keys that are not there → garbage scale" | no: `maxScale 1.00` everywhere, and the one animated translation channel is `Root_jnt` (108.1 vs ref 108.7) |
| "FT's `UArkAnimNodeStack::GetBoneAtoms` stand-in blending a 78-track result into a 118-bone array by index" | the stack's output is the sequence's (`ghdiag eval node 5 MatineeStackNode: maxRatio 1.00, transOff 2, poseH 65.0`), and it is the same on the idle guard (`PoseBlender` 46.1). The stand-in is not touched |
| "posed height 148 for every NPC in `Empty_Idle` ... scale/LTW/DPG/LOD nominal" (GF, taken as the healthy value) | 148 is itself the fault: the idle guard's `LocalAtoms` are 3.00x on all 118 bones (`ghdiag local DishonoredNPCPawn_8 transOff 114 maxRatio 3.19`), and its true pose has Z extent 45.7 (`ghdiag eval node 11 Empty_Idle`). The tower guards are 3x big in every capture since the look-at component landed (FT, wave 21) and the mode went to the last one (ChooseLookAt, wave 23); the matinee pawns look *stretched* rather than *big* only because their poses spread the 3x limbs |
| "Acceptance: posed height 148 ± 5" | after the fix the posed Z extent is the true one (45-65 by pose); the acceptance measure that survives is `maxRatio 1.00` / `transOff ≤ 2` on `LocalAtoms` and the capture pair |
| "scale/LTW/DPG/LOD nominal; not foot placement" (GF) | right, and consistent: the fault is in the additive blend of the body tree, above the controls |

## 2. The measurement (`GH1`, `DishonoredGame_GH.exe` = `agentGH_rel`, `-ghdiag` once per pawn at the first frame its `poseH` passes 152, and once for an idle control)

Route `build/agentGH/gh_run.py` (GF's: `-finewgame=50 -fosoiree -fkdiag -frdiag -fzdiag -gadiag -gfdiag -ghdiag -diskismet`, shots at
70/100/140/175 s, quit 200). `GH1.log`, 0 `Critical`; captures `build/agentGH/shot_gh1a..d*.png`.

| pawn (mesh) | `LocalAtoms` vs reference | the chain, node by node (`ghdiag eval`, each node's own `GetBoneAtoms`) |
|---|---|---|
| `DishonoredNPCPawn_0` (`Skm_EliteGuard_Body`, 118 bones, required 118; `Posture_IdleBack` + `Gesture_Think` on the matinee stack) at t 51.4 | poseH 155.5, reach 552.3, **114 of 118 bones' translations off, 3.00x** (`spine_2_jnt`), 45 rotations > 20°, scale 1.00 | sequence nodes 31/32: `maxRatio 1.00` (transOff 0/2) → `MatineeStackNode` 65.0 / 189.8 / `1.00` → `PoseBlender` the same → **`AnimNodeAdditiveBlending` 184.4 / 551.5 / `3.00`** = root → `LookatBlender` alone: 124.9 / 411.6 / **`2.00 at Root_jnt`**, 114 off, root rotation 180° (an additive delta pose carrying the reference translations) |
| `DishonoredNPCPawn_2` (`skm_Sokolov_Body`, 110 bones) | poseH 184.4, reach 548.5, 106 off, 3.51x at `handAttachment_L_jnt` | the same chain |
| `DishonoredNPCPawn_8` (idle tower guard, `Empty_Idle`, required 40) at t 60.3 | poseH 147.3, reach 553.3, **114 off, 3.19x** (`handAttachment_L_jnt`, the one bone with a constant translation channel: 14.1 + 2 × 11.5) | `Empty_Idle` node 11: 45.7 / 190.5 / `1.22` → `UpperBodyFilter` / `MatineeStackNode` the same → `LookatBlender` 124.9 / 411.6 / `2.00` (the per-node rows of this pawn above node 5 are invalid: only 40 bones required, the rest uninitialised) |

Per bone (`ghdiag bone`, all 118 for `_0` and `_8`): `Root_jnt |t| 325.25 ref 108.66 x2.99`, every other bone `x3.00` to two decimals,
`handAttachment_R/L` 2.52x / 3.19x (their constant channels), `aim_jnt` 473.6 vs 157.9 (the 553 reach), rotations vs reference
0-30° on the body, 11.9° on every face bone, 106-179° on the fingers (the idle's curled hand).

The names that fail: `SetAnimInfo`'s warning, 1,170 lines, 36 per NPC (two `SetLookAtMode` passes × 18 poses), all
`ADD_Empty_AimHead_BowIdle_*` / `ADD_Empty_AimTorso_BowIdle_*`; the cooked `Npc_EliteGuard_HeadTracking_as` (Tower, Prison, Isl)
carries `AimHeadIdle / AimHeadRun / AimHeadWalk / AimTorso* / Gun / Patrol` only (`build/agentGH/edgedata_tower/index.json`).
`ChooseLookAt` (`dishonorednpcpawn_locomotion.cpp`): `Primary ? Primary->GetDefaultNPCLookAtMode() : eDisNPCLookAtMode_MAX`, and
`FArkComponentLookat::SetLookAtMode` clamps to `m_LookAtModes.Num() - 1` (retail 2012 `0x58f570`, the same clamp).

Offline, before the build (`build/agentGH/gh_pose.py` on `build/agentDJ_rel/Binaries/Win32/EdgeAnimSmoke.exe --dump`, data
`build/agentGH/edgedata` from `L_Tower_Script.upk` / `L_Tower_P.upk`, the `Skm_EliteGuard_Body` Edge skeleton from W's
`edgedata2`, byte-identical to the Tower package's): `Empty_Idle`, `BoatDriver_SC_Tower_Idle`, `BoatCpt_SC_Tower_EmptyIdle`,
`BoatCpt_SC_Tower_LookingBack` all compose to reach 190 with every bone's translation at its reference length except the constant
channels (`handAttachment_L` 1.22x); the composed reference pose (Z extent 44.9, reach 191.4, the same convention as the game's
`FBoneAtom::operator*` - checked against gfdiag's `refH`/`refReach`) stands along -Y in component space.

## 3. What was ported

`UArkAnimNodeLookAt::GetBoneAtomsForSet` (`dishonoredplayercontroller.cpp`): retail's Edge set builder
`UAnimNodeSequenceBlendBase::BuildEdgeAnimTreeEx` (2012 `0x1a62b0`, 1,021 B in 2013 at `0x19a280`, the same `+224` byte-stream test
and the same two `UAnimNodeSequence::BuildEdgeAnimTree` fallbacks) expressed in the CPU evaluator:

- relevant poses: `Weight > ZERO_ANIMWEIGHT_THRESH && AnimInfo.AnimSeq && AnimSeq->CompressedByteStream.Num() > 0`;
- one relevant pose carrying the whole relevant weight → its `GetAnimationPose`, cached (retail's single-leaf case);
- several → accumulated with `Weight / RelevantWeight` (the lerp chain's `alpha = w / (sum + w)` is this normalisation);
- none → `GetAnimationPose( AnimSeq, AnimLinkupIndex )`, the node's own sequence (`UAnimNodeSequence::BuildEdgeAnimTree`, 2012
  `0x1a4a20` / 2013 `0x1989f0`: an Edge `AnimSeq` becomes one leaf); without one, the additive identity (stand-in for retail's
  cached-pose leaf, flags 4).

Retail's CPU body (2012 `0x55bab0` / 2013 `0x51b420`, FT's source) stays what FT wrote otherwise. Nothing else in the evaluation
path is changed: `UArkAnimNodeStack::GetBoneAtoms` (FT's stand-in), `FEdgeAnimSequencePose`, `EdgeAnimEvaluateSkeletonPose`, the
linkup, `MAT_SetAnimPosition` (FS) are all measured right (§1) and untouched. `-ghdiag` (§5) is appended to `gfarmsdiag.cpp`.

What this does to the look-at in the broken mode: both sets fall back to the node's own cooked sequence
(`ADD_Empty_AimHeadIdle_UpLeft`, an additive head pose at time 0), as retail would with the same unresolved names. The right mode
needs an equipped item (`GetDefaultNPCLookAtMode`) - GJ's package; until then every NPC sits in the last config mode with the
up-left delta, and the stretch is gone.

## 4. The runs

| run | exe | log | result |
|---|---|---|---|
| GH1 | `DishonoredGame_GH.exe` (`agentGH_rel`: instrument only) | `GH1.log`, `gh1_run.txt`, `shot_gh1a..d` | §2; the boat guards' legs fill the view at 100 s (`gh1b`) |
| GHC1 | `DishonoredGame_GHbase.exe` (`head_rel`, main `4b889ff`) | `GHC1.log`, `ghc1_run.txt`, `shot_ghc1a..d` | the control: `NPCPawn_8` poseH 147.3-148.2, `NPCPawn_26` 210.2 at 100 s, 148.2 after; `ghc1b` the same legs; 0 `Critical` |
| GH2 | `DishonoredGame_GH2.exe` (`agentGH_reg`: the fix) | `GH2.log`, `gh2_run.txt`, `shot_gh2a..d` | **the 3x is gone**: `gfdiag skel` reads `maxLocalRatio 1.00` at `Root_jnt` / `spine_*` (1.22 on the one constant-channel bone, as offline) on every pawn for the whole run; the idle guard `NPCPawn_8` `poseH 47.3` (true scale, was 147.3), `NPCPawn_26` 42-111 through the ride by pose (was 148-331), reach 191.7 (was 553); the only `ghdiag` dump the old 152 threshold still fires (`NPCPawn_17` at 97.9 s, Z extent 164.7 from `BoatCpt_SC_Tower_TurnBack_In`'s spread) reads `transOff 3 rotOff 16 maxRatio 1.08`; `gh2b` (100 s) shows the two boat guards at human size where `ghc1b` had their legs filling the view; 0 `Critical`, no minidump |

## 5. The instrument: `-ghdiag`

In `gfarmsdiag.cpp`, called at the top of `DisGFReportPawn` (every `ADishonoredPawn::Tick`, rate-limited to 2 Hz per pawn, its own
switch). For the first two pawns whose `SpaceBases` Z extent passes 152 and for one idle control (140-152, after 60 s), once:
`ghdiag dump` (mesh, bone counts), `ghdiag local` (the `LocalAtoms` summary: poseH, reach, bones whose local translation length is
outside 5 % of the reference, bones rotated > 20° from it, the largest ratio and its bone, the root's rotation delta, the largest
scale, NaNs), `ghdiag tree` + `ghdiag node` (every node: class, name, weight, relevance, children with weights and the additive /
mirror marks; sequence nodes with their sequence, set, additive flag, linkup, time, play state, `m_bDontUseRefPoseOnNonDefinedBones`,
whether the Edge path can evaluate it), `ghdiag eval` (every weighted node's own `GetBoneAtoms` summarised the same way - valid for
the bones the mesh requires), `ghdiag edge` (the top non-additive sequence through `FEdgeAnimSequencePose::GetAnimationPose` at the
node's time), `ghdiag bone` (all bones: local translation, reference, ratio, rotation delta, scale, posed and reference Z). Nothing
under the switch changes behaviour.

## 6. Acceptance

| check | result |
|---|---|
| gate `run_regression.py` (worktree copy, `--build-dir D:/RecompileDishonored/Recompile/build/agentGH_reg --newgame-at 50 --game-extra-args=-windowed`, in-harness build of the merged tree with the fix) | **41 ok, 5 failed, 1 skipped** on the first pass (`build/agentGH/gh_regression.log`, 2,377 s): the five failures and the skip are the one d3d9 stage, whose 90-s run reached the input hook at 86 s and was killed before a frame (`d3d9_log_lines 362`, `d3d9_frames -1`; `d3d9_smoke.txt`: `run: timeout, killed`) - the control run GHC1 was on the machine beside the gate's build and this stage. **Re-run of the stage alone on the same exe** (`--no-build --only d3d9`, `build/agentGH/gh_regression_d3d9.log`): `d3d9_startup_seconds 20.1`, **`d3d9_frames 2940`**, `d3d9_draws_per_frame` / `d3d9_draw_elements` / `d3d9_visible_prims` / `texture_census` ok, 9 ok / 0 failed / 0 skipped. Every other row ok on the first pass: `unported_natives 0`, `nullrhi/inputtest/newgame/options_criticals 0`, `verify_phase2 3/3`, `layout_mismatching 0`, `coresmoke 99/0`, `inputtest_moved 1023.2`, `newgame_pawns 29`, `options_as2_errors 39`. The `--only` run rewrote `summary.txt` to its nine rows; the 47-row result is in `gh_regression.log` |
| in-harness build | `build/agentGH_reg/regression/build_DishonoredGame.log`: 0 `error C`, 0 C4263, 0 C4264; `CoreSmoke` / `LayoutProbe` 0 |
| working build | `build/agentGH_rel`: `gh_build0.log` 0 errors, 0 C4263, 0 C4264 (one pass) |
| builds used | two: `agentGH_rel` (the instrument) and the gate's `agentGH_reg` (the fix, merged tree) |
| before / after | GHC1 (`head_rel`) vs GH2 (the gate exe): §4, captures `shot_ghc1a..d` / `shot_gh2a..d` |
| the stretched NPCs | gone on the gate exe (GH2, §4): every NPC's `LocalAtoms` at its reference lengths (`maxLocalRatio 1.00`, the constant channels aside), the matinee-driven guards and the captain at human size in `gh2b` against `ghc1b`; the brief's "148 ± 5" is replaced by the measure that holds (`maxRatio 1.00`, `transOff ≤ 3`), because 148 was the 3x pose (§1) |
| addresses | every 2012 rva from the PDB-named database, every 2013 rva from the named database by name (§7) |
| rules | every launch `game_input.py launch` + `-windowed` + isolated inis; no real input; no retail `Dishonored.exe`; no FModel; two builds; no minidump newer than the builds in `DishonoredGame/Logs` |

## 7. Files and addresses

`build/agentGH_sync.py`: `dishonoredplayercontroller.cpp` (one function), `gfarmsdiag.cpp` / `.h` (the instrument), this file,
`agentGH_status.csv`. No generated file, no Engine edit, no unit off the exclude list. Addresses: every 2012 rva from the PDB-named
database, every 2013 rva from `GH_retail2013.i64` by name (`build/agentGH/decomp2013/`: `0x19a280` 1,021 B, `0x1989f0` 149 B,
`0x51b420` 2,141 B, `0x50cc80` the look-at's own tree builder, `0x5350c0` the stack's, `0x1ba240` the additive node's, `0x192360`
`SetAnimInfo`, `0x199f90` `CheckAnimsUpToDate`), each read in `build/agentGH/decomp2012*/` and `decomp2013/`.
