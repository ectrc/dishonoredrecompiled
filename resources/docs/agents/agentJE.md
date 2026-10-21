# Agent JE — the Empress chain: teleport native, real FaceTo requests, teleport spell, dialog state + bars, look-at (wave 27, PHASE24 package JE)

Worktree `build/agentJE_wt` (made at `96cdf21`, moved to `7ead454` before build 1 and to `e56851c` (main with JA, JB, JD, JG, JH) before
build 2, each time by stash / checkout / pop; the conflicts were only the CppText tails of `ADishonoredPawn.h`, `ADishonoredNPCPawn.h`,
`ADishonoredPlayerPawn.h` (both sides kept) and the generated `Sources.cmake` / `DishonoredGameRegistrants.cpp` (regenerated)). Build dir
`build/agentJE` (RelWithDebInfo). **Two builds, both 0 errors, 0 C4263/C4264** (`build/agentJE/je_build1.log`, `je_build2.log`). Staged
`DishonoredGame_JE.exe` (build 2). IDA on private copies `build/agentJE/idb/{retail2013,shipping2012}_agentJE.i64` (`build/agentJE/je_ida.py`,
`je_vt.py`, `je_vt98.py`); decompiles in `build/agentJE/d12`, `d13`. Every run through `build/agentJE/je_run.py` (`game_input.py launch`,
`-windowed -nomovie`, own inis). No retail exe, no real input, no FModel. No commits.

Route of every run: `-nomovie -finewgame=50 -fosoiree -fwdiag -fwteleport=DisTrigger_16@160 -fvdiag -gadiag -fidiag -jediag -iediag -gediag
-frdiag -fwkismettrace -icdiag -apshottime=160,162,...,240`.

| run | exe | what it is |
|---|---|---|
| JE0 | `DishonoredGame-Win64-Shipping.exe` (head, staged 2026-10-20) | step 0, control (no `-jediag`, no `-iediag`) |
| JE0b | the same | `-gelinks` attempt for the teleport destination (gelinks matched nothing on the streamed level) |
| JE1 | build 1 (tree at `7ead454`) | everything ported, base-behaviour dialog rejection only (see §3.1: this run broke the ride and the overhear scene) |
| JE2 | build 2 (tree at `e56851c`) | the hand-back build |

Captures: `build/agentJE/shots/je0/`, `je1/`, `je2/` (`wNNN.png` = world time, every 2 s, 160-240).

## 0. Step 0 (head, JE0) and the same measurements on this package (JE2)

| measure | head (JE0) | JE2 |
|---|---|---|
| writer of HUD bit 0x10 (bars) | only `SetCinematicMode_Native` (level 0): on at 50.6 for the ride, off at 150.9 (`fidiag cinematic`: masks 7fef at every level 160-240, no bars in the scene) | **`UStatePlayerMasterChoice_Base::OnEnterState` via the dialog state** (level 1, PlayerState: disable 0x7FAF, enable 0x6010) at 160.89 -> `SetBlackStripes(1)`; off at its `OnExitState` 185.04 -> `SetBlackStripes(0)` (193.01 wall). The director/cinematic path (`SetCinematicMode_Native`) is not involved in this scene |
| `SeqAct_Teleport_25` | `execOnTeleport_Native` stub, Emily (`DishonoredNPCPawn_21`) stays at 14176/22752/2576 | destination **`TargetPoint_72` at 16848/22266/3169, `ArkIsPointOnNavMesh` 1**; Emily set to 16848/22266/3242 (half height up), brain home there, Teleported stim, locomotion reset (164.63) |
| Emily's go-to (`SOIREE_Empress_Regent.DisSeqAct_AIGoToActor_5` -> `TargetPoint_21` 15666/21074/3335, align 1) | from her painting spot, walks to 14426/21641/2576 by ~172 and stays there (dist 1363, path 1/2, speed 0, `stuck 1` at 194) - stage 0 to the end of the run; never reaches the FaceTo wait | stage 0 164.63 -> arrived 170.58 (**stage 1**, face-to request 2, yaw -17752 from -20281) -> **ORIENTATION_REACHED / RotationReached 170.94** -> stage 2 -> **Success** 170.94 |
| `Emily_GoToGazebo_Success` / `Gate_37` / `Switch_8` | none; `Switch_8` activation 1 only from `Interp_0`'s end (176.8, 'Link 1', unlinked) | `ActivateRemoteEvent_51 'Emily_GoToGazebo_Success'`, `Gate_37`, `Switch_8` activation 1 at 171.00; `Switch_8` activation 2 at 176.99 -> `DisSeqAct_DialogInputs_6 'Regent Interrupted By Emily'` -> the next scene (`Tower_Regent_Emily` keys 177.0-181.8, `Tower_Regent` 188.9) |
| Emily run-in speed | - | 228 u/s at 165.8, **357-408 u/s 166.3-169.4** (3.1 s), 236 at 169.9, 0 at 170.9 (`jediag goto path`, every 0.5 s) |
| the five Empress/Regent lines | posted (FR/IE path) | `Tower_EmpressRegent` keys fire at 161.11, 163.70, 167.34, 172.73, 173.89 (`iediag`) |
| fade back (`Interp_26`) | `Interp_26` activation 1, amount 0 | `Interp_26` activation 1 at 160.96; fade 0.28 at 161.4, 0 from 162.4 |
| player in the dialog state | `StartDialog` warning, no state | `player StartDialog ... dialog state requested` 160.89, entered from Walk, look-at `InterpActor_27` track 1 (flags 0x147), `EndDialog` 185.01, left for Soiree 185.04 |
| go-tos with rotation alignment | 1 (`_25`) completes through the bring-up skip | all 5 on the route complete through a real request: `_25` 52.66, `_8` 83.71, `_9` 164.47, `_21` 170.94, `_7` 230.44 (`arrived: stage 1` -> `rotation reached (mine 1)` -> `Success`) |
| ride stop stim | 150.88 (`_26`, `_27`) | 150.64 (`_26`, `_27`, `_16`); boat lines 13 key fires 56.4-122 as IE |

## 1. What was ported (2013 bodies where they differ; 2012 / 2013 rva)

| unit | functions |
|---|---|
| `dishonoredpawn_body.cpp` | `ADishonoredPawn::OnTeleport_Native` 0x799d60 / 0x756e40 + exec 0x633400 / 0x5ec880: first actor on 'Destination' (a controller's pawn instead), `SetLocation` + cylinder half height, `bUpdateRotation`: `FaceRotation(dest + RelativeRotation, 0)` and the controller's rotation |
| `dishonorednpcpawn_body.cpp` | `ADishonoredNPCPawn::OnTeleport_Native` 0x7c1200 / 0x77f640: `m_bTickedNPCBody` cleared, brain home, `FAIStimStruct_Teleported`, `FArkComponentLocomotion::HandleTeleport`, ragdoll bones, `m_fLastFallingStartHeight` (2013), `m_fLastTeleportTime` |
| `dishonoredplayerpawn_body.cpp` | `ADishonoredPlayerPawn::OnTeleport_Native` 0x6fb0c0 / 0x6a2000 |
| `dishonoredplayercontroller.cpp` | `ADishonoredPlayerController::OnTeleport_Native` 0x6e1d50 / 0x6a5f40 (camera `OnTeleport`, `m_LastSecondLocation`) + exec 0x634230 / 0x5edc30; `-jediag` writer line in `SetCinematicMode_Native` |
| `disdesirestructs.cpp` | `FDisFaceToRequest::DoRequest` 0x8f9f20 / 0x8a9b30 (the three `InitAllByDefaultExcept` + `OverwriteExactRotation`, `Start/UpdateFaceTo{Actor,Location,Yaw}` with the asker's name and priority), `StopRequest` 0x8f8ea0 (`StopFaceTo`), the inlined sync (`m_pFaceToComponent->m_bStarted`) |
| `disdesiresinterface.cpp`, `IDisDesiresInterface.h` | `GetDesiresFaceToEventCallback`: the address of one pointer-to-member that calls the asker's `HandleFaceToEvent` (what retail's three `DesiresFaceToEventCallback` bodies do) |
| `disbehaviorgoto_base.cpp` | bring-up skip in `Start_TurningToFace` removed; `FilterPathingFail` 0x746db0 / 0x6e8600 with `AttemptTeleportSpell` |
| `disaisubstatetakeposition.cpp` | `FilterPathingFail` 0x771130 with `AttemptTeleportSpell` |
| `dishonoredutilities_npc.cpp` (retail's unit; off the exclude list) | `AttemptTeleportSpell` 0x832a50 (unmatched in 2013) |
| `dishonoredplayerpawn_conversation.cpp` | `IsAvailableForDialog` 0x706580 / 0x6ad070, `StartDialog` 0x6fb550 / 0x6a2460 (2013: audio log broken out, carry-body flag 0x10), `EndDialog` 0x706600 / 0x6ad0f0 |
| `stateplayermasterindialog.cpp` | `FStatePlayerMasterInDialog_Param` ctor 0x6c3e40 / 0x6715a0 + `OnPending` 0x6b8ab0 / 0x6693a0; `OnEnterState` 0x6b8b20 / 0x673720, `OnExitState` 0x6b8d30 / 0x6739f0, `TickState` 0x6c94c0 / 0x679280, `RequestStateExit_Derived` 0x6b8ea0, `TrackPawnForDialog` 0x6b8ec0 / 0x669480, `TrackActorForDialog` 0x6b8ff0 / 0x6695a0, `StopTrackingForDialog` 0x6b4160 / 0x663d90, `CalcZoomFOVBasedOnRadius` 0x6b40d0 / 0x663cc0, `PlayerRequestSkip_Derived` / `IsSkippable_Derived` 0x6afa30 / 0x6afa50 (FZ's soirée hand-over kept) |
| `stateplayermasterchoice_base.cpp` (off the exclude list) | `OnEnterState` 0x6b8560 / 0x673640, `OnExitState` 0x6b8600 / 0x66c7e0, `HideChoiceSelection` (2013 0x669340, the HUD half), `TickState` 0x6c9470 / 0x679230 (without JF's `ShowChoiceSelection`), `PauseBendTime` / `UnpauseBendTime` 0x6b8680 / 0x6b8750, `PlayerRequestSkip` / `IsSkippable` |
| `dishonoredplayercontroller_tracking.cpp` | `StartTrackingActor` 0x6f8960, `StartTrackingPawn_ByRegion` 0x6f9c60, the actor `FDisPlayerViewTarget::SetTarget` 0x6d14c0 |
| `disconv_playerlookatactor.cpp`, `disconv_playerlookatspeaker.cpp` (off the exclude list) | `PlayNode_Derived` 0x9077b0 / 0x8acac0 and 0x907940 / 0x8acc80 (2013: also wait while the player falls), `TickNode` 0x8f8a50 / 0x8a7980 (vtable slot 73 of both, ICF-folded) |
| `dishonorednpcpawn_conversation.cpp` | `IsAvailableForDialog` 0x7c17c0 / 0x774280 (behaviour via `CallGetDialogAvailability`, then the master action state), `StartDialog` 0x7c1880 / 0x77fe50 (`FAIStimStruct_BlockingDialogRequest`), `EndDialog` 0x7c19b0 / 0x77ff80 (`FAIStimStruct_BlockingDialogEnd`) |
| behaviours | `UDishonoredAIBehavior::GetDialogAvailability` 0x7453e0 + `CallGetDialogAvailability`; overrides GoTo 0x745480, Soiree 0x742170, BlockingDialog 0x73c9c0, Follow 0x73cff0, Stolen 0x7238a0 (+ MagicResponse, RatStomp, React, which share its body), and the `return 0` fold 0x723660 of Idle, Guard, Patrol (PatrolSearch), GoHome, Interact (2012 vtable slot 98, `build/agentJE/je_vt98.py`); `UStateNPCMasterAction::GetDialogAvailability_Action` 0x6b02c0 (`statenpcmasteraction.cpp`, off the exclude list) |
| `jediag.h/.cpp`, `dishonoredhud.cpp` | `-jediag`: every HUD bit 0x10 write with the levels, the writers, teleports + navmesh test, face-to start/update/stop, go-to stages and the go-to path every 0.5 s, rotation reached, the dialog state, NPC/player Start/EndDialog, refused dialog availability |

Generator: `DishonoredGameNativeStubs.ported.agentJE.txt` (the two teleport execs), new CppText for Choice_Base, the two look-at nodes,
Follow, Stolen, React, GoHome, Interact, NPC master action; 79/79 generated files byte-identical on the hand-back tree
(`build/agentJE/regen3`); exclude list 649 -> 644; `bitfield_audit` mismatches 0.

## 2. Deviations (named in the banners)

- `FDisFaceToRequest`: a pawn without a face-to component keeps DF's counted stand-in (retail dereferences it).
- `GetDesiresFaceToEventCallback`: one shared pointer-to-member for every implementer instead of each class's own (all three are
  `HandleFaceToEvent`).
- `AttemptTeleportSpell`: the item-context half (`FindItemContext<UDisItemContext_NPCTeleportSpell>`, `PrepareManualTeleport`,
  `AttemptUse_DirectUse`) is unported; a pawn with a secondary weapon is refused (named once). Emily has none (logged), as retail.
- NPC teleport / Start / EndDialog: `FDisComponentLODManager` (StartLOD / StopAllLODByAsker), `FDisComponentVision::ResetLineOfSight`,
  `FDisComponentObservable::ResetLOS` unported. Player teleport: `RefreshSearchProxy`, `ResetVisibility`, the reachability hideout.
- Dialog state enter: the 2013 carried-corpse drop and the active-power deactivation loop not ported.
- `StartTrackingPawn_ByRegion` / `TrackPawnForDialog` zoom: no pawn hit-region table (`GetHitRegionInfo`, `GetBBox_ForRegion`) - the
  pawn is tracked at its collision bounds.
- `PauseBendTime` / `UnpauseBendTime`: `ADishonoredGameInfo::BendTime` unported; only the out-of-bend-time flags move.
- PC teleport: the player pawn write is skipped without a player.

## 3. Where the brief was wrong (with the measurement)

1. **"The only writer of HUD bit 0x10 is SetCinematicMode_Native"**: the dialog state's base (`UStatePlayerMasterChoice_Base::OnEnterState`,
   2013 enable 0x6010 at the PlayerState level, 2012 0x810) is the writer for this scene; `-jediag` JE2 160.89 / 185.04.
2. **"Emily's go-to never reports Success because of FaceTo"**: on head the go-to never reached the FaceTo wait - without the teleport
   Emily starts on the lower terrace and stalls at 14426/21641 (stage 0, `stuck 1`, 1363 u from a target 760 u higher). The teleport
   (destination on the navmesh) is what lets her path; the FaceTo wait then completes in 0.36 s. Both were needed.
3. **"UStatePlayerMasterInDialog::OnEnterState 2012 0x6b0c40"**: it is 2012 0x6b8b20 (521 B); 2013 0x673720 (unmatched in the match file,
   found as the caller of the base enter 0x673640).
4. **The NPC `IsAvailableForDialog` via `CallGetDialogAvailability` is not safe as the PDB names suggest**: the base
   `UDishonoredAIBehavior::GetDialogAvailability` rejects every blocking conversation (6), and Idle / Patrol / Guard / GoHome / Interact
   override it with an ICF-folded `return 0` that has no PDB name of its own. Build 1 had only the named overrides: the boat's
   `DishonoredNPCPawn_26` (Idle) refused the ride conversation (no boat matinee, no stop stim) and `DishonoredNPCPawn_9` refused
   'BlockPlayer' (no `Interp_26`: black screen from 161 on, `shots/je1/w168.png`). Build 2 reads slot 98 of every behaviour vtable.
5. `-gelinks` cannot name a streamed level's teleport destination (JE0b: matched nothing); `-jediag` does (`TargetPoint_72`).
6. `MAT_IsAllowed` (2013 0xb65560) is **not ported**: retail calls it through an `APawn` virtual from the engine's matinee binding (no
   such virtual is declared in this tree's Engine; the call site is `UnInterpolation.cpp`, agent JC's file). Its two helpers
   (`IsAvailableForSoiree_Brain` 2012 0x749bd0, `IsAvailableForSoiree_Action` 0x6b0310) are decompiled in `build/agentJE/d12`. The scene
   plays without it.

## 4. Acceptance

| check | result |
|---|---|
| `Emily_GoToGazebo_Success`, `Gate_37`, `Switch_8` | **met** JE2 171.00; `Switch_8` 2nd activation 176.99 -> next scene |
| Emily runs in from the teleport point, > 200 u/s for >= 2 s | **met**: 357-408 u/s 166.3-169.4 (3.1 s), path in `jediag goto path` |
| bars on with the dialog state, off at its exit, writer named | **met**: 160.89 / 185.04, `UStatePlayerMasterChoice_Base` via the dialog state |
| five Empress/Regent lines | **met** (161.1-173.9) |
| aligned go-tos complete, ORIENTATION_REACHED per request | **met**, 5/5; ride stop stim 150.64 (head 150.88) |
| `unported_natives` 0 (teleport line gone), `probe_natives` <= 3, criticals 0 on d3d9 + inputtest + newgame | see §5 |
| capture series 160-240 | `build/agentJE/shots/je2/` (control `je0/`) |

## 5. Gate

`run_regression.py --build-dir build/agentJE --no-build --exe-name DishonoredGame_JE.exe --log-prefix JEg_ --only d3d9,inputtest,newgame`
(build 2, tree `e56851c` + this package; `build/agentJE/gate.txt`): **23 ok, 0 failed, 0 skipped**. `unported_natives` **0** (the
`execOnTeleport_Native` line gone from every run), `probe_natives` **3**, `d3d9_criticals` / `inputtest_criticals` / `newgame_criticals` **0**,
`inputtest_moved` 806.8, `inputtest_peak_speed` 400.1, `newgame_pawns` 29. JE2 route: 0 Critical.

## 6. Hand-back

`python build/agentJE_sync.py` (base = main at hand-back, `e56851c`). Status: `resources/docs/agents/agentJE_status.csv`.
