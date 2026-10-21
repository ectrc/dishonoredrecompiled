# Agent JC: NPC scene placement - the 2013 special root-motion path, ProcessRootMotion, the FaceTo and AIControl tracks (wave 27, PHASE24 JC)

Worktree `build/agentJC_wt`. It was made at `96cdf21`. Before build 2 it was moved to `7ead454` (JA, JB, JD and JG merged) by
stash / checkout / pop; the only conflict was the generated `DishonoredGameRegistrants.cpp`, which was regenerated. Build dir
`build/agentJC` (RelWithDebInfo). No commits.

Two builds:
- **Build 1** (`DishonoredGame_JC1.exe`, base 96cdf21): 0 errors, 0 C4263/C4264. It carries the A/B switches `-jcoldrm`,
  `-jcnofaceto` and `-jcnoaicontrol`.
- **Build 2** (`DishonoredGame_JC.exe`, base 7ead454): the merged tree plus an exact-name `-jcwatch`. See §6.

Every run went through `cutscene_compare.py run` (= `game_input.py launch`, with `-windowed -nomovie`), wrapped by
`build/agentJC/jc_run.sh`. Tables come from `build/agentJC/jc_table.py <log>`. IDA ran only on private copies in
`build/agentJC/idb/`; decompiles are in `build/agentJC/d12` and `build/agentJC/d13`. FModel was never used.

## 1. The step-1 measurement (build 1): the drift table

What `-jcdiag` logs:
- `keystart`: every StretchAnimControl key start, giving the group actor's distance and yaw from its AI group's stage mark actor.
- `special`: every frame of a key with `m_bSpecialRootMotionExtract`, giving the backed-up anim position and the previous and
  current position handed to `MAT_SetAnimPosition`.
- `backup` / `clearbackup` / `place`: the backed-up transform, the target transform (backup + root delta), and the actor's drift
  from that target, every frame.

| run | route | switches | key starts | special root-motion keys | `place` frames | Translate backups | Critical |
|---|---|---|---|---|---|---|---|
| JC1a | ride (`-finewgame=50 -fosoiree`) | - (2013 path) | 634 | **0** | 0 | WorkerB `_20` once (51.04 -> 51.70) | 0 |
| JC1b | ride | `-jcoldrm -jcnofaceto -jcnoaicontrol` (pre-JC) | 635 | **0** | 0 | WorkerB `_20` once | 0 |
| JC1e | Empress (`-fwdiag -fwteleport=DisTrigger_16@160`) | - | 651 | **0** | 0 | `_20`; Worker B `_3` x8 (153.99..183.45); `_4` once (180.11) | 0 |

**No key on either route has `m_bSpecialRootMotionExtract`.** The drift table PHASE24 asked for therefore has no rows here:
- The cumulative-delta path (a backed-up anim position handed over as the previous position) never runs on the ride or in the
  Empress scene.
- So it cannot be what puts Emily or the ride NPCs off their marks.
- The Translate keys that do run are plain ones: the previous position is FLT_MIN, so the delta is per frame.
  - WorkerB's `Empty_Distraction_Scavenge02_In` moves `_20` from -1230.5/33499.7 to -1202.9/33462.1, which is 46.6 uu of the
    animation's 47.5. The same move happens with `-jcoldrm`: 46.3 uu.
  - The backup and its clear bracket the key, as 2013 does.

Key starts against the stage mark (`build/agentJC/JC1a_table.md`, `JC1b_table.md`, `JC1e_table.md`):
- **Every ground NPC starts its scene animation on its stage mark (0.0 uu)**: the party NPCs `_0`, `_2`, `_6`, `_18`, `_20`,
  `_22`, `_23`, the Empress `_5` and the Regent `_7`.
- **The NPCs carried on the boats are off their spawner, identically in JC1a and JC1b**, so this is not this package:
  - GuardA `_17`: 2.7 uu / 178°, then 103 uu at key 1.
  - The captain `_26`: 2.0 uu, then 12.8 uu.
  - The driver `_27`: 4.4 uu.
- **The yaw differences are the FaceTo track, now ported** (JC1b, without FaceTo, shows 0.0°):
  - The Empress `_5` is 163.4° from her spawner's yaw. Her `SeqAct_Interp_25` key 0 turns her to `TargetPoint_4` (yaw 50345) at
    priority 15, and she holds that yaw at the next key starts.
  - The Regent `_7` is 10.8° off because of his key towards the Empress.

## 2. Emily in her cutscene (JC1e, world 160-200, `-jcwatch=DishonoredNPCPawn_21`)

At 160.36 the player is teleported to `DisTrigger_16`. From 161.9 the view is the scene camera: it sits at 16239/21616/3433, yaw
41188 (226°), pitch about 0, FOV 75.

Emily `_21`:
- **She is never bound by a matinee in this window.** She has 0 key starts and her stack weights are 0.
- **She walks** from 14182/22753 at 164.9 to 14423/21637 by 172.8: about 1,140 uu in 8 s.
- **Then she stands there** until 200.
- Against the camera she is 47° left and 35° below the view axis, screen x -0.20 / y 1.31. **She is out of frame on all 26
  samples from 162 to 200.**

The capture `build/cmp/JC1e/ours/w0181.033.png` shows the Empress and the Regent in the gazebo, framed, and no Emily. Emily's end
point is 1,100 uu short of the gazebo (the Empress is at 15569/20904).

**Emily leaving the frame is her go-to stopping short, not root motion and not a scene animation.** That is JE's chain:
- the teleport native that should start her from her run-in point;
- `Emily_GoToGazebo_Success` never firing.

The acceptance "Emily stays inside the frustum" cannot be met by this package. It is reported, not forced.

## 3. What retail does (2013 database) - and where the brief was wrong

1. **Neither of the two `AActor` virtuals restores the transform.**
   - Slot 202 (+808) backs it up. ADishonoredPawn 0xb491e0 stores `m_vBackupedPosition`, `m_rBackupedRotation`,
     `m_BackupedLocalToWorld` and the mesh `LocalToWorld`. The NPC override 0xb4b180 also disables the locomotion and sets
     PHYS_None.
   - Slot 203 (+812) clears it. ADishonoredPawn 0xb49260 zeroes the backup. The NPC override 0xb4b1e0 enables the locomotion,
     sets PHYS_Walking and clears `m_bDisableCollisionForSoiree`.
   - The placement is a third mechanism the brief did not name:
     - The stretch track's InitTrackInst (0x9016e0) / TermTrackInst (0x901760) set and clear the pawn's
       `m_bSpecialRootMotionExtract` (slot 307, 0xb493e0).
     - On such a pawn, the 2013 `MAT_SetRootMotionMode` (0xb4d440) does not put the mesh in RMM_Translate. It disables the
       locomotion and sets `m_bShouldProccessMatineeRootMotion` (slot 201).
     - `UpdateSkelPoseEnd` (0x7471d0) keeps the extracted root motion in `RawExtractedRootMotionDelta` (+1056).
     - `UDisSkeletalMeshComponent`'s tick end (0xb62270) calls 0xb5c890 for a pawn a matinee controls
       (`m_bIsControlledByAMatinee`, set by `MAT_BeginAIGroup` at 0xb650b0).
     - 0xb5c890 places the pawn absolutely: backup + the raw delta through the backed-up mesh transform (0xb492e0 / 0x70da70),
       with one MoveActor (0xb562d0) that writes the move back into `RootMotionDelta`.
   - So PHASE24's "most likely wrong #1" was right: the cumulative delta is retail's, applied absolutely from the backup.
2. **`ProcessRootMotion` has no accumulator of its own at +832.** +832 is the stock `RootMotionDelta` translation. Retail's
   differences from the reference, the same in 2012 0x353c80 and 2013 0x746280:
   - the interp-curve root motion is the change since the last frame;
   - the rotation is taken with the component's own Rotation removed, scaled by `RootRotationScale` (+884) and offset by
     `AdditionalRootRotation` (+888);
   - there is no bypass-simulated-physics nudge and no RootMotionProcessed notify.

   IO's Translate-only rotation rule is confirmed by the bytes. IK2's RMM_Accel tick stays out.
3. **`MAT_GetBodyIntention` is a constant default** (2012 0x7ad740: no stance, the empty item twice). The AIControl track reaches
   the pawn through four NPC requests (slots 318-321). The 2013 body-intention request (0xb59e70) differs from 2012:
   - the brain is read only when the pawn is not officially dead and its brain is initialized;
   - a melee primary is the brain tweaks' `m_SuspectingEquippedPrimaryItemClass`;
   - `m_bHackForbidEquipState` sets a 0.5 s forbid-equip duration.
4. **No route reaches the path the brief made step 1 about** (§1).

   The ride's and the Empress scene's placement defects are elsewhere:
   - the boat passengers' offsets, identical with `-jcoldrm`;
   - Emily's go-to (§2).

   No AIControl key fires on either route either (0 `aicontrol` lines).

## 4. What was ported

| function | retail | where |
|---|---|---|
| `USkeletalMeshComponent::ProcessRootMotion` | 2013 0x746280 (3,920 B) / 2012 0x353c80 (3,979 B), whole | UnSkeletalComponent.cpp |
| `RawExtractedRootMotionDelta` store | UpdateSkelPoseEnd 2013 0x7477c0 | UnSkeletalComponent.cpp |
| `USkeletalMeshComponent::GetRawRootMotionWorldDelta` (name ours) | 2013 0x70da70 | UnSkeletalComponent.cpp |
| `AActor::MAT_BackupTransform` / `MAT_ClearBackupTransform` (slots 202/203, empty) and their calls | BackupTransform 0x61fcb0, UpdateGroup tail 0x62fed0 | EngineClasses.h, UnInterpolation.cpp |
| `UInterpTrackInstStretchAnimControl::InitTrackInst` 2013, `TermTrackInst` | 0x9016e0 / 0x901760 | interptrackstretchanimcontrol.cpp |
| `ADishonoredPawn::MAT_SetRootMotionMode` 2013 | 0xb4d440 | dishonoredpawn.cpp |
| `ADishonoredPawn::MAT_BackupTransform`, `MAT_ClearBackupTransform`, `MAT_SetSpecialRootMotionExtract`, `GetMatineeRootMotionTransform`, `ProcessMatineeRootMotion`, `MoveByMatineeRootMotion` | 0xb491e0, 0xb49260, 0xb493e0, 0xb492e0, 0xb5c890, 0xb562d0 | dishonoredpawn.cpp |
| `ADishonoredNPCPawn::MAT_BackupTransform` / `MAT_ClearBackupTransform` | 0xb4b180 / 0xb4b1e0 | dishonorednpcpawn.cpp |
| `UDisSkeletalMeshComponent::TickEnd` (the matinee placement call) | 0xb62270 | dishonoredpawn_body.cpp |
| `MAT_BeginAIGroup` / `MAT_FinishAIGroup`: `m_bIsControlledByAMatinee`; `m_bDisableCollisionForSoiree` cleared | 0xb650b0 / 0xb65520, 0xb6553b | dishonorednpcpawn.cpp |
| `ADishonoredNPCPawn::MAT_SetCollision` 2013 else-branch (a walking / falling pawn is left alone) | 0xb4b2a0 | dishonorednpcpawn.cpp |
| `UInterpTrackInstFaceTo::UpdateTrackInst` / `TermTrackInst` | 2013 0x8ff8f0 / 0x8fc9a0 (2012 0x53eb90 / 0x53bb30) | Engine: interptrackfaceto.cpp / interptrackinstfaceto.cpp (hook calls); DishonoredGame: arkcomponentfaceto.cpp (bodies) |
| `UInterpTrackAIControl::UpdateTrack`, `UInterpTrackInstAIControl::InitTrackInst`, Aim / Fire `GetTarget` | 0x8fee50, 0x8fc900, 2012 0x53b990 / 0x53b930 | interptrackaicontrol_dis.cpp (NEW) |
| `ADishonoredNPCPawn::MAT_SendBodyIntentionRequest`, `MAT_SendAimRequest`, `MAT_SendAimStopRequest`, `MAT_SendFireRequest` | 0xb59e70, 0xb4ee40, 0xb4eec0, 0xb53470 | dishonorednpcpawn.cpp |

Re-diffed with no change needed: `UInterpTrackInstStretchAnimControl::UpdateTrackInst` (0x905c70: the pushes, the clamp, and
the backed-up anim position as the previous position) and the stack's `MAT_SetAnimPosition`.

Named deviations:
- `MAT_SetSpecialRootMotionExtract` is declared on `AActor`, because the Engine cannot name `ADishonoredPawn` (retail calls
  `ADishonoredPawn`'s slot 307 on the group actor).
- The Fire request only logs: the item-use paths (`AttemptUse_AIAttackAutoSelect` / `_DirectUse`) are not ported.
- The aim request's `RequestNPCMasterAimState` is not ported; only the torso look-at at priority 17 runs.
- The FaceTo track skips a group actor that is not a pawn, and a target group with no instance (retail reads through both).
- Retail formats the failed-move message in 0xb562d0 and then drops it; here it is printed under `-jcdiag`.
- The NPC requests (retail slots 318-321) are plain members, as IC did for the other `MAT_` queries.

Step 5 (`MAT_BeginAIGroup`'s mesh snap) was not changed: no measured key start sits off its mark because of it (§1).

## 5. Diagnostics and switches

- `-jcdiag` lines: `keystart`, `special`, `backup`, `clearbackup`, `place`, `faceto`, `aicontrol`.
- `-jcwatch=<name,...>`: twice a second per watched pawn, its mesh against the local view (angles, screen x/y, in frame).
- A/B switches (kept in build 2): `-jcoldrm`, `-jcnofaceto`, `-jcnoaicontrol`.

## 6. Build 2, acceptance and gate

Build 2 is the merged tree (7ead454 + JC), with 0 errors and 0 C4263/C4264. The only change from build 1 is that `-jcwatch`
now matches exact names; under build 1 `_21` also matched `_2`.

| check | result |
|---|---|
| special root-motion keys / `place` frames | JC2a: 635 key starts, 0 / 0. JC2e: 647 key starts, 0 / 0 |
| scene-anim start within 10 uu / 5° of the mark | Every ground NPC: 0.0 uu. The yaw offsets are the FaceTo keys (Empress 163.4°, Regent 10.8°). The boat passengers (2-103 uu) are the same pre-JC |
| Emily inside the frustum, 165-200 (JC2e) | **Not met**: 0 of 73 samples. She is 47° left and 35° down. Cause is JE's go-to (§2) |
| ride SSIM ≥ .84 at a = 13.57 − soirée-enter (`build/agentIK/rescore.py`) | Build 1: JC1a **0.840** (MAD 7.75) against JC1b (pre-JC) 0.842. Build 2: JC2a **0.821** (MAD 7.96) against JC2b 0.705, the same exe with `-jcoldrm -jcnofaceto -jcnoaicontrol` (soirée-enter 57.42 vs 56.38). On the merged tree the run-to-run spread is wider than JC's effect, and JC2a is the better of the pair |
| walkers 123 u/s (merged tree, NPC ApplyAttributes) | Worker B `_3`: peak 123 u/s; mean 110.7 over the moving samples |
| IO2 facing table, outside turns (`build/agentIO2/io_table.py`, JC2a vs JC2b) | `_3` 0.0/100% vs 0.0/100%; `_8` 100% vs 100%; `_9` 0.2/100% vs 0.7/100%; `_12` 1.1/100% vs 0.0/100%; `_26` 0.8/100% vs 9.9/95%; `_15` 21.0/50% vs 23.4/50%. `_4` is 28.4/72% vs 0.9/100%: the Dock worker's FaceTo keys (`TargetPoint_12` at 148.6, `TargetPoint_60` at 179.5) now turn him while he is near his walk |
| gate d3d9 + inputtest + newgame (`build/agentJC/gate2.txt`) | 22 ok, 1 failed. Passing: every `*_criticals` 0, `unported_natives` 0, `probe_natives` 3, d3d9 1,320 frames, `newgame_pawns` 29, `newgame_on_mission` 115. The failure is `inputtest_moved` 173.0, twice, under load: the game reached its start at log 20.3 s against 6.4 s for JH. Run alone back to back (`--only inputtest`): **JC 820.4, JH exe 817.0, JC 818.7**, all passing |
| `bitfield_audit` / generator | 0 mismatches / 79 of 79 byte-identical (`build/agentJC/regen`) |

## 7. Files

Engine:
- `Inc/EngineClasses.h`, `Inc/EngineInterpolationClasses.h`, `Inc/UnSkeletalMesh.h`, `Inc/UnWorld.h`
- `Src/UnInterpolation.cpp`, `Src/UnSkeletalComponent.cpp`, `Src/interptrackfaceto.cpp`, `Src/interptrackinstfaceto.cpp`,
  `Src/interptrackstretchanimcontrol.cpp`

DishonoredGame:
- CppText: `ADishonoredPawn.h`, `ADishonoredNPCPawn.h`, and five new AIControl files
- `Inc/jcdiag.h` (new)
- `Src/dishonoredpawn.cpp`, `Src/dishonorednpcpawn.cpp`, `Src/dishonoredpawn_body.cpp`, `Src/arkcomponentfaceto.cpp`
- `Src/interptrackaicontrol_dis.cpp` (new)
- generated: `DishonoredGameEngineShims.h`, `DishonoredGameRegistrants.cpp`

Sync: `build/agentJC_sync.py`. Its base is the worktree's commit, 7ead454: main's HEAD (e56851c) would hide JH's edits to the
shared files. It flags those four files for the hand 3-way merge:
- `ADishonoredPawn.h`
- `ADishonoredNPCPawn.h`
- `dishonorednpcpawn.cpp` (the `MAT_` hunks against JH's focus-text hunks)
- `DishonoredGameRegistrants.cpp` (regenerate it)
