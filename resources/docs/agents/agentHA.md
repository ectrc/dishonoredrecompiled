# Agent HA — pose: the stack (wave 25, PHASE22 package HA)

Worktree `build/agentHA_wt` (base `9faefc0`), build dir `build/agentHA` (RelWithDebInfo, two builds through
`build_queued.py`), staged as `DishonoredGame_HA.exe`. Every number below says which build it comes from:
**HA1** = build 1 (steps 1–3), **HA2** = build 2 (step 4 + the Edge leaf mask), **HAC1** = control
`DishonoredGame-Win64-Shipping.exe` (`head_rel` at HEAD), `build/cmp/head_rel` = coordinator's HH baseline.

## 0. The answer

1. `UArkAnimNodeStack::GetBoneAtoms` is now `BuildEdgeAnimTreeEx` on the CPU. **The captain's None channels are
   skipped** (`skip(noseq)` on ch0/ch3 from matinee pos 0), `w0 1.00` on all 1,617 aipawn lines (HA2), and the
   HH frame at world 66 shows the captain walking with his arms down where head_rel shows the T-pose
   (`build/cmp/HA2/pairs/pair_w0066.031.png` vs `build/cmp/head_rel/pairs/pair_w0066.001.png`).
2. **The plan missed that the stack's channel test reads a bit nobody sets.** `m_bIsAnimSeq` is 0 in every cooked
   tree (`init` lines: `cooked 0000000`); retail's `UAnimNodeBlendBase::InitAnim` (2012 `0x1c2ed0`) writes it for
   every child and this tree's blend base has no `InitAnim`. The stack derives it in its own `InitAnim`; without
   that, the transliteration would have skipped nothing.
3. **The stand-in's mask was the wrong table.** Retail's leaf mask is `AnimLeafCallback`'s (2012 `0x58fb00`) weight
   array: a joint is defined iff the Edge blob's joint-name-hash list names it. FT's stand-in read the UE AnimSet
   linkup. HA2 uses the Edge list (`mask edge 114/118` on Blink, `40/118` on every BoatCpt body anim = face +
   back-pan bones, `58/118` on the hand gestures — verified offline against the cooked blobs, §4).
4. The arm that is still out in HA2's captures is **the animation**: at world 90 retail's captain leans on the
   hull with his right arm out (`pair_w0090.037.png`, top); ours plays the same lean in the middle of the deck,
   because he is not where retail puts him (HC's position/rotation package).
5. Root motion: WorkerB key 0 (`rootMotion 1`) now puts the mesh in `RMM_Translate` (`rmm 0/1`, the NPC
   `StartRootMotionMode` override), but **no stack ever reports root motion** (0 `rm` edge lines in HA2): the
   sequence carries its 47.5-unit move in the Edge **locomotion joint** (offline: `Empty_Distraction_Scavenge02_In`
   `offsetLocomotionDelta` 164, delta z 47.5), and Plan B (edgeanim.md §5) differentiates the root bone instead.
   `rmd > 0` / `_20 moves` cannot be met without the locomotion half of `AnimLeafCallback`.
6. Ride metrics (HH, median over world 56.2–150.4, vs head_rel): HA1 SSIM 0.523→0.525, MAD 29.4→28.6 (−2.7 %);
   HA2 SSIM 0.524, MAD 28.5 (−3.0 %); zoom 1.76 unchanged (the FOV fault, HD's — it dominates SSIM until HD merges).

## 1. What was ported

| function | retail | state |
|---|---|---|
| `UArkAnimNodeStack::GetBoneAtoms` | `BuildEdgeAnimTreeEx` 2012 `0x576920` (376 B), 2013 rva `0x5350c0` (VA `0x9350c0`, 376 B) | CPU transliteration (below) |
| `UArkAnimNodeStack::InitAnim` | 2012 `0x5782c0`; the bit writes are `UAnimNodeBlendBase::InitAnim` 2012 `0x1c2ed0` | `m_bIsAnimSeq` derived after `Super::InitAnim` |
| `UArkAnimNodeStack::MAT_SetAnimWeights` | 2012 `0x576790` / 2013 `0x534f30` (277 B both) | FS's child-0 complement removed: retail writes children N−1..1 only (both decompiles) |
| `UArkAnimNodeStack::MAT_SetAnimPosition` | 2013 VA `0x934cb0` (629 B) | previous position; `IssueNotifies` (2013 vtable +484) then `SetPosition`, `PreviousTime = prev` |
| `ADishonoredNPCPawn::StopRootMotionMode` | 2012 `0x7af1a0` / 2013 VA `0xb71280` (47 B, same bytes, sha1 equal with rel32 masked) | base, `FArkComponentLocomotion::Enable`, look-at `+88 bit 1` (`m_bHeadDisabled`) cleared |
| `ADishonoredNPCPawn::StartRootMotionMode` | 2012 `0x7ac100` / 2013 VA `0xb6e680` (136 B) | **not in the brief**: base, locomotion `Disable`, Translate/Velocity zero `DesiredSpeed`/`Velocity`/`Acceleration`. Stop alone would have re-enabled locomotion every frame with nothing ever disabling it |
| `UInterpTrackInstStretchAnimControl::UpdateTrackInst` | 2013 VA `0x905c70` (823 B, slot 76) | 2013 body: jumped-back → no notifies; `SetActorRootMotion` only when the anim position moved forward; special root-motion path (`m_bSpecialRootMotionExtract` && Translate → `m_bShouldBackupTransform`, clamp at key end, `m_fBackupAnimPosition` as previous position); FLT_MIN elsewhere |
| `UInterpTrackInstStretchAnimControl::GetKeyPlayLength` | 2013 VA `0x904e40` (156 B, unnamed) | new |
| `UInterpGroupInst::BackupTransform` | 2013 VA `0x61fcb0` (121 B, unnamed) | flag cleared, `m_fBackupAnimPosition = m_fCurrentAnimPosition` (sub_8FCDE0, 7 B); the actor's 2013 virtual +808 not called (below) |
| `UInterpGroup::UpdateGroup` tail | 2013 VA `0x62fed0` | backup on request or on Translate start; the +812 call on Translate stop not made |
| `AActor`/`APawn`/`ASkeletalMeshActorMAT`/`ADishonoredPawn::MAT_SetAnimPosition` | APawn 2013 VA `0x6b53a0` (retn 28h = 10 DWORDs), ADishonoredPawn 2013 VA `0xb4d3d0` | `FLOAT InPrevPosition` before `InPosition`; the two script exec wrappers pass `FLT_MIN` |
| `EdgeAnimGetDefinedSkeletonJoints`, `FEdgeAnimSequencePose::GetDefinedBones` | `AnimLeafCallback` 2012 `0x58fb00` stage 0 (the `0xFFFF` map → weight `0xFF`) | new; the stack's mask |

`GetBoneAtoms`, line for line against the Edge chain: base = child 0's subtree, or with no node
`UAnimNode::BuildEdgeAnimTree` (2012 `0x19e6f0`, 2013 VA `0x5929f0`: leaf flags 4, no header →
`_edgeAnimProcessCommandList` command 5 copies the skeleton's base pose, every joint defined, locomotion joint not —
so `FillWithRefPose`; **the brief's "0x5782f0" is `UArkAnimNodeStack::BuildEdgeAnimTree`, the 18-B thunk, not
`UAnimNode`'s**); child 0's weight never read; child i used iff raw `Weight > 0`; `m_bIsAnimSeq` (+64 bit 2) with
`AnimSeq` (+228 = `0xE4` in both disassemblies) NULL → skip; no node → base-pose leaf; `bIsAdditive` (+64 bit 1) →
op 1 (relative add-delta, `_edgeAnimBlendJointsRelative` 2012 `0x613a90` — **there is no `_edgeAnimBlendAdditive`
in either binary**), else op 0 linear, alpha = weight. `_edgeAnimBlendPose` (2012 `0x6232f0`) blends
`numJoints + 1` joints, the last being the locomotion joint, so root motion follows the branch: lerp when both
sides carry it, the channel's when only it does, additive accumulates. Masked joints keep the left pose
(`m_bDontUseRefPoseOnNonDefinedBones` or an additive sequence, `BuildEdgeAnimTreeLeaf` 2012 `0x1a46b0` +
`UAnimSequence+136 bit 1`).

2013 confirmation: slot 91 (+364) of `??_7UArkAnimNodeStack@@6B@` (VA `0x10a3b78`) is `BuildEdgeAnimTree` VA
`0x937220` (18 B) → `BuildEdgeAnimTreeEx( ctx, −1 )` VA `0x9350c0`. **The two bodies are the same 116
instructions**; with addresses and register aliases normalised the only difference is the assert line constant
(`238h` vs `254h`). Disassemblies `build/agentHA/d12/576920_*.asm`, `build/agentHA/d13/9350c0_*.asm`.

The track comment at `interptrackstretchanimcontrol.cpp:404` is corrected (NAME_None every frame after a key plays
out is retail's; the stack skips it). The track's behaviour is otherwise unchanged in build 1.

## 2. Diagnostics (`-fkdiag`, existing lines kept)

- `fkdiag stack <node> on <owner> init: N children, w0 <cooked>, m_bIsAnimSeq cooked <bits> derived <bits>` — once per InitAnim.
- `fkdiag stack <pawn> <node> eval base <class> | ch<i> w <w> lerp|add [refpose] [rm] <anim> mask edge|linkup|none <masked>/<desired> | skip(noseq) | skip(w0) | rm <has> <x/y/z/yaw>` — once a second per stack, plus a line whenever the stack's root-motion flag changes.
- `fkdiag aipawn ... w0 <w> refdev <mean> n<bones> L <deg> R <deg>` — mean angle of the mesh's `LocalAtoms` from the
  reference pose over `upper_arm`/`lower_arm` L/R (4 bones on the NPC skeleton). Note: this is the mesh output, not
  the stack's (the stack is not the tree root), and the boat NPCs' reference pose is an A-pose, so an idle with
  hanging arms reads 10–20°.

## 3. Acceptance, as read

Route `-finewgame=50 -fosoiree -fkdiag -frdiag -gadiag -diskismet`, quit `wait 168`. **World times**: the
coordinator measured world = log − 12.1 on this route (ride world 56.3–150.6); the brief's 64–158 were log times, so
HA1/HA2 shots are world 58,73,88,103,118,133,145 and the refdev window is world 56.3–150.6. The control HAC1 was run
before that correction with shots at world 70..157.

| check | HAC1 (control) | HA1 | HA2 |
|---|---|---|---|
| `_26` refdev > 10° every sample world 56.3–150.6 | no refdev in the control build; HH frame world 66 shows the T-pose (`build/cmp/head_rel/pairs/pair_w0066.001.png`) | 89 samples, min 11.2 | 89 samples, min 10.9 |
| `_27` refdev > 10° | — | 89, min 19.3 | 93, min 16.3 |
| `w0 1.00` on every aipawn line | `w0 0.00` throughout (FS's complement) | 1,575/1,575 | 1,617/1,617 |
| captain None channels `skip(noseq)` at pos 0–3 | — | ch0, ch3 from pos 0 (log 73.26..79.36) | same |
| WorkerB key 0 `rootMotion 1` → rmd > 0, `_20` moves | rmm 0/1 at pos 0.09, rmd 0.00, no move | rmm 0/1, rmd 0.00, no move | rmm 0/1, rmd 0.00, 0 rm edges: **not met**, §0.5 |
| `Critical` | 0 | 0 | 0 |
| `Failed, with animsequence` | 0 | 0 | 0 |
| HH ride SSIM / MAD (head_rel 0.523 / 29.4) | — | 0.525 / 28.6 | 0.524 / 28.5 |
| player ride unchanged (fz6/gb2_a) | — | — | HH frames head_rel vs HA2, ours only: grey MAD 3.0–3.1 at world 148–154 (detach facing the gangway, identical framing, `pair_w0151`), 1.6–14.5 mid-ride where NPC poses differ |

Gate (HA2, worktree copy, `--build-dir D:/RecompileDishonored/Recompile/build/agentHA --no-build --only
nullrhi,d3d9,inputtest,newgame,options --newgame-at 50`): see §6. `coresmoke`/`layout` need `CoreSmoke.exe` and
`LayoutProbe.exe`, which this package's two `DishonoredGame` builds do not produce — the coordinator's
clean-checkout gate covers them.

## 4. Where the plan was wrong, with the measurement

| the plan said | measured |
|---|---|
| test `+64 bit 2` as retail does | the bit is 0 in every cooked stack (`init` lines, HA1); retail's `UAnimNodeBlendBase::InitAnim` (2012 `0x1c2ed0`) writes it; ported into the stack's `InitAnim` |
| "keeping FT's mask" | FT's mask reads the AnimSet linkup; retail's leaf mask is the Edge hash list (`AnimLeafCallback` `0x58fb00`). Offline (`resources/tools/pdb/extract_edgeanim.py` on `L_Tower_Script.upk`, Elite body skeleton 118 joints): `BoatCpt_SC_Tower_Idle`/`SlowDown` name 78 joints (missing exactly the 32 face + 8 back-pan joints), `Blink` names 4 — the same 40/118 and 114/118 HA2 logs |
| "`UAnimNode::BuildEdgeAnimTree` 2012 0x5782f0" | `0x5782f0` is `UArkAnimNodeStack::BuildEdgeAnimTree` (18 B); `UAnimNode::BuildEdgeAnimTree` is `0x19e6f0` (61 B), a flags-4 base-pose leaf |
| "verify `_edgeAnimBlendAdditive`" | no such function in either binary; op 1 is `_edgeAnimBlendPose`'s relative branch (`_edgeAnimBlendJointsRelative` `0x613a90`), over `numJoints + 1` joints |
| refdev > 10° ⇒ no T-pose | refdev is ~11–16° on an idle with hanging arms (the boat NPCs' reference is an A-pose); the number separates T-pose (head_rel's captain) only weakly. Read the frames |
| captain "leaning where the retail video does" | he plays `SlowDown`/the lean, arm out toward a hull that is not there: position/rotation (HC), not pose |
| WorkerB `rmd > 0` | the anim's root motion is in the Edge locomotion joint; Plan B reads the root bone (edgeanim.md §5) |
| "declaration in Inc/CppText/ADishonoredNPCPawn.h only through the generator" vs "never hand-edit Inc/CppText" | `Inc/CppText/<Class>.h` is the generator's **input** (its docstring: "what the tree hand-edits survive through"); the generator never writes it. Edited there, then the generator re-run into a scratch dir: **79 of 79 generated files byte-identical** to the tree (both builds) |
| step 4 = "previous-position float + backup" | the backup half calls two 2013-only `AActor` virtuals (slots 202 +808 / 203 +812; empty on `AActor`, overrides unidentified); ported up to those calls, which are named `DISHONORED(bringup)` |
| only `StopRootMotionMode` | the matching `StartRootMotionMode` override (136 B) is needed or Stop re-enables locomotion that nothing disables; ported |

## 5. Addresses, resolved by hand

Private copies `build/agentHA/idb/{shipping2012,retail2013}_agentHA.i64`; helpers
`resources/tools/ida/ha_ida.py` (dec/dis/vt/find/hash/xref) and `ha_scan.py`. 2012 (imagebase 0): `0x576920` 376,
`0x5782f0` 18, `0x19e6f0` 61, `0x1c3c60` 342, `0x576790` 277, `0x7af1a0` 47, `0x7ac100` 136, `0x78e4c0` 15,
`0x78e4b0` 14, `0x1c2ed0` 255, `0x1a46b0` 878, `0x1a4a20` 149, `0x6232f0` 1476, `0x613a90` 3542, `0x6228a0` 418,
`0x6238c0` 4287, `0x58fb00` 13540 — all function starts at those sizes. 2013 (VA): `0x9350c0` 376, `0x937220` 18,
`0x5929f0` (call target in `0x9350c0`), `0x934f30` 277, `0xb71280` 47, `0xb6e680` 136, `0xb49190` 15, `0x934cb0` 629,
`0x905c70` 823, `0x904e40` 156, `0x612e40` 5 (`this[27] |= 2` = `m_bShouldBackupTransform`), `0x61fcb0` 121,
`0x8fcde0` 7, `0x62fed0` 930, `0x6b53a0` 179, `0xb4d3d0` 112. Vtables: `??_7UArkAnimNodeStack@@6B@` VA `0x10a3b78`
slot 91; `UInterpTrackStretchAnimControl` vtable at VA `0x1099108` (slot 89 +356 = `GetKeyframeTime` `0x901080`);
`UAnimNodeSequence` 2013 slot 121 +484 = `IssueNotifies` `0x599a50`; `AActor` 2013 slots 202/203 = 1-B `ret`.

## 6. Gate

`build/agentHA/ha_regression.log`, `build/agentHA/regression/summary.txt`: **32 ok, 0 failed, 0 skipped**, 984 s, on
HA2 (the second build, `build/agentHA`): `nullrhi_criticals 0`, `d3d9_criticals 0`, `d3d9_startup_seconds 4.0`,
`d3d9_frames 1920`, `unported_natives 0`, `inputtest_moved 1062.3`, `inputtest_criticals 0`, `newgame_on_mission 121`,
`newgame_pawns 29`, `newgame_criticals 0`, `options_criticals 0`. Builds: 0 errors, 0 C4263, 0 C4264 in
`build/agentHA/build1.log` and `build2.log`. `bitfield_audit.py`: mismatches 0. Generator
(`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentHA/regen2`): 79 of 79
files byte-identical to the tree. `Failed, with animsequence`: 0 in HA1.log and HA2.log.

## 7. Open items

1. The locomotion half of `AnimLeafCallback` (root motion from the Edge locomotion joint, `FLocomotionState`) —
   until then no matinee root motion on an Edge sequence reaches the pawn (WorkerB, every Translate key).
2. The 2013 `AActor` virtuals +808/+812 (transform backup / restore around Translate keys) — identify the pawn
   overrides in the 2013 vtables, then `BackupTransform` and the UpdateGroup tail call them.
3. Pose is right; placement is not: the captain leans at the hull from mid-deck (HC), FOV (HD).
4. STATUS "Build and link" pitfall text for the coordinator (replaces the `GetBoneAtoms` bullet):
   > **Retail PC does not evaluate skeletons through `GetBoneAtoms`.** It builds an EdgeAnim tree
   > (`BuildEdgeAnimTree[Ex]`) per node and runs it through `GEdgeAnimManager`. `UArkAnimNodeStack::GetBoneAtoms` is
   > now that node's `BuildEdgeAnimTreeEx` (2012 `0x576920`) written for the CPU: child 0 is the base whatever its
   > weight, a channel counts iff `Weight > 0`, a sequence channel without an `AnimSeq` is skipped, and a channel's
   > mask is the Edge leaf's (the joints the blob's hash list names, `AnimLeafCallback` `0x58fb00`), not the AnimSet
   > linkup. Two traps: `FAnimBlendChild::m_bIsAnimSeq` is 0 in every cooked tree (retail's
   > `UAnimNodeBlendBase::InitAnim` writes it; the stack derives it), and root motion stored in an Edge sequence's
   > locomotion joint does not reach the pawn under Plan B.
