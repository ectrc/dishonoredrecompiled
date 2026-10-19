# Agent IF — player crouch, sprint, jump, swim (wave 26, PHASE23 package IF)

Worktree `build/agentIF_wt` (made at `5ddc8a1`; moved to `bda2769` (IA) before build 1 by stash + checkout + 3-way pop, one
conflict in `CppText/UDishonoredPlayerInput.h` resolved by keeping both; moved to `97349ae` (IB, no shared file, not rebuilt) at
hand-back). Build dir `build/agentIF` (RelWithDebInfo), staged `DishonoredGame_IF.exe`. **One build, clean on the first try**
(the second was not needed). IDA on private copies `build/agentIF/idb/{retail2013,shipping2012}_agentIF.i64` through
`build/agentIF/if_ida.py`; decompiles in `build/agentIF/d12`, `build/agentIF/d13`. No FModel.

## 0. Step 0 — HEAD baseline (`DishonoredGame_IF0.exe` = the gate's `build/gate_wt/build/gw` exe, HEAD code)

`inputtest` alone (`build/agentIF0/step0_inputtest.txt`): 8 ok, `inputtest_moved` 385.8 / 399.6 under four agents' builds
(load-sensitive, see below). **The 4 `probe_natives`**: `ADishonoredPawn::execOnTeleport_Native`,
`ADishonoredPlayerController::execCalcPlayerSwimAccelRate`, `ADishonoredPlayerPawn::execOnOverrideAwarenessDisplay`,
`ADishonoredPlayerPawn::execPlayDying_Native` (the same four agent HL named).

Hand route: inputcmd scripts from `build/agentIF/if_steps.py`, measured with `exec getall` (no build change needed on HEAD).
The brief's dock route (`-finewgame=50 -fosoiree`) could not measure speed: the player is blocked a few units from the boat
socket (walk peak 117.8 u/s, then 0, on HEAD and on this build alike) and there is no way down to the water (no cheat manager:
`AddCheats` logs `Accessed None 'CheatManager'`, so no `BugItGo`). The numbers are from `-startmap=L_Tower_P -startmapopen`
(the gate's spawn, open ground), same keys, `-nomovie`.

| | HEAD (`IF0s`) | build 1 (`IF1s`, `-ifdiag`) |
|---|---|---|
| `key C` collision height | 87.5 -> 87.5 (stub `execDis_SneakOrSlide` fired) | 87.5 -> **65.0** (`m_fSneakHeight`), 2nd press -> 87.5 |
| `m_bSneakModeToggled` / `IsSneaking` | False | **True**, then False |
| eye (camera bone above the feet) | 167.7 | 167.7 -> **122.7** sneaking -> 167.7 (`EyeHeight` property stays 85 in both: the camera is the bone) |
| walk peak 2D speed | 500.7 | 500 (modifier 1.000; the 517 read includes a fall off a ledge) |
| sprint (`LeftShift` + `W`) peak | 500.1 (no sprint) | **600.1** = GroundSpeed 500 x 1.200 (`Attribute_GroundSpeedSprint` / GroundSpeed) |
| `key SpaceBar` Z apex | 0 (stays PHYS_Walking) | **106.2** at `Attribute_JumpZ` 800 u/s; Walk -> Jump -> Falling -> Walk, 0.53 s |
| sneak speed | - | 344 (CrouchedPct 0.688 = GroundSpeedCrouch / GroundSpeed), blended down at `m_fSpeedBlendDownSpeed` |
| physics in water | PHYS_Swimming with acceleration 0 (gate log: the script calls the zeroed stub 30 ms after the probe moves the pawn in) | **PHYS_Swimming**, master Swim, acceleration 500 on `W`, peak 262 u/s (WaterSpeed 381 x the crouched 0.688), collision 29 (swim crouch) |
| leaving the water | - | Swimming -> Falling (refused by Swim, as retail) -> **Walking**, Swim -> Walk, height back to 87.5 |

## 1. What was ported (2012 / 2013 rvas; the 2013 body wherever they differ)

- Crouch (sneak): `UDishonoredPlayerInput::Dis_SneakOrSlide` 0x6f9170 / 0x6aff70 + exec 0x63a8e0; `StartSneaking` 0x70d450 /
  0x6d3860; `StopSneaking` 0x70d570 / 0x6cd960 (completed); `IsSneaking` 0x707470 / 0x6c8280 (2013 adds the mantle's
  `m_bSneakRequired`; moved to retail's unit `dishonoredplayerpawn_stealth.cpp`); `AttemptChangeToSneakHeight` 0x7074c0 / 0x6cda60;
  `AttemptChangeCollisionHeight` 0x6feca0 / 0x6c3430 (2013 head-room extent Z 1.0); `StartCrouch` 0x6fcfe0 / 0x6a3720, `EndCrouch`
  0x6fd080 / 0x6a37c0 + execs; `UnCrouch` 0x70ae50 / 0x6a7f60; `GetSpeedFraction` 0x6fb8a0 / 0x6c33b0.
- Upper-FSM requests: `RequestAction` 0x70bb90 / 0x6b1d00, `RequestActionHelper` 0x704ad0 / 0x6aad90, `FStatePlayerAction_Param`
  0x6c4330 / 0xa71690 (VA), `FStateSharedActionBase_PlayerOnly_Param` 0x695a20. **Retail has no null test here** (the machines
  always exist); the upper/left-arm machines are never initialised in this tree (FZ), so a request to a machine with no state is
  refused and named (`ifdiag action request Upper_GoTo_Crouch_Ready refused`).
- Sprint: `HandleHeldButtons_Movement` 0x6f8590 / 0x6aeb10, `_Movement_Gamepad` 0x6f80b0 / 0x6ae610, `_Movement_Movable` 0x6d6e60 /
  0x6a2ec0 (boundary: `UDisMovableComponent::GetMaxPawnSpeed` 0x672640 / 0x64ade0 — nothing is ever grabbed); `HandleHeldButtons`
  now runs _Movement, names _Lean (0x6b3ff0, the lean state is a skeleton), runs _Context. `ADishonoredPawn::MaxSpeedModifier`
  0x794b10 / 0x756c40 + `MaxSpeedModifier_Derived` (base 1.0; player 2013 0x6a4700: sword sheathed/unsheathed attribute) +
  player `IsCarryingCorpse` 0x6febc0 / 0x6a5220 + `IsHoldingMovableObject` 0x7afe30 / `GetHeldMovableObject` 0x7afe60.
- Jump: `ADishonoredPlayerController::CheckJumpOrDuck` 0x6e2b10 / 0x6ae490 (the Dishonored override; UnController.cpp untouched);
  `DoJump` 0x715fe0 / 0x6a3820 + exec; `DoJump_Mantle` 0x712b10 / 0x6dbe70, `IsMantleRequested` 0x707170, `IsPressingForward`
  0x7071a0; `PerformMantle` = FALSE (agent IG's, named). `UStatePlayerMasterJump` (13 bodies + param), `UStatePlayerMasterFalling`
  (5 + param), `ADishonoredPlayerPawn::setPhysics` 0x6fe380 / 0x6a4b50 (the physics hook that drives Walk/Falling/Swim) and
  `ADishonoredPawn::setPhysics` 0x7946c0.
- Swim: `CalcPlayerSwimAccelRate` 0x6e3b50 / 0x6af330 + exec; `UStatePlayerMasterSwim` (8 bodies + param; `GetSwimStrokeAccel`
  2013 0x660690); `ADishonoredWaterVolume::CheckPoint`, `GetSurfaceZ`; `ADishonoredPawn::OnEnter/OnExitWaterVolume`, `Tick_Water`
  (now the second tick phase); player `OnEnter/OnExit/OnChangeWaterVolume`, `Tick_Water` (2013, with the apnea refresh 0x6d4240),
  `GetSwimmingState`, `GetApneaDuration`, `OnCameraEnter/ExitWaterVolume`.
- `-ifdiag` (`Inc/ifdiag.h`, `Src/ifdiag.cpp`): GBA actions, heights, MaxSpeedModifier inputs/result, FSM transitions and
  refusals (hooks in `dishonorednativestatemachine.cpp`), physics/water, a 4 Hz player line, and the `IFWATER [EXIT]` console
  command (a measurement teleport like `-distouchprobe`, only under the switch).

## 2. Gate (`run_regression.py --build-dir build/agentIF --no-build --exe-name DishonoredGame_IF.exe --log-prefix IFg --only d3d9,inputtest,newgame`)

**23 ok, 0 failed.** Criticals 0/0/0, `unported_natives` 0, **`probe_natives` 3** (CalcPlayerSwimAccelRate left the list),
`inputtest_moved` 1131.6 (HEAD's step-0 run read 386/400 under load; the baseline note says 1029.6 quiet; the player's walking
modifier is 1.000, so the walk itself is unchanged), `newgame_on_mission` 98, `newgame_pawns` 29. Generator: 79/79 byte-identical
(`--out-dir build/agentIF/regen`); `bitfield_audit` mismatches 0. NPCs: no NPC ever reached `MaxSpeedModifier` in either run (no
`ifdiag speed DishonoredNPCPawn` line) — they walk through their own physWalking, so their speed cannot have changed.

## 3. Where the brief was wrong (with the measurement)

1. **"crouch collision height (class default)"**: the sneak is not the engine's crouch. It goes to the pawn tweaks'
   `m_fSneakHeight` (65.0), not `CrouchHeight` (29), and leaves `bIsCrouched` alone; 29 is the swim state's `APawn::Crouch`.
   `EyeHeight` does not drop in retail's code either — the camera bone does (167.7 -> 122.7; StartCrouch moves both the pawn and the
   arms' mesh down by the 22.5 given up, retail's own double drop).
2. **"sprint >= 30 % over walking, = GroundSpeed x the tweak multiplier"**: retail's sprint is the absolute
   `Attribute_GroundSpeedSprint` over GroundSpeed — measured 600 vs 500, **+20 %**. (GroundSpeed itself is still the class default
   500: `ApplyAttributes` is unported, BF's note; were it ported GroundSpeed would follow `Attribute_GroundSpeed`.)
3. **"Z apex >= tweak jump height - 10 %"**: there is no jump-height tweak; the jump speed is `Attribute_JumpZ` (800, not the pawn's
   `JumpZ` 533). Apex measured 106.2 both routes.
4. **"UStatePlayerMasterSwim::GetSwimStrokeAccel 2013 0x6b3f50"** is the 2012 address; 2013 is 0x660690 (275 B, with DLC upgrade
   multipliers 2012 lacks). Also `CheckJumpOrDuck` is called at `UnController.cpp:3734` in HEAD (3761 is the user's working tree).
5. **Water Touch/UnTouch cannot be reached**: retail's `ADishonoredWaterVolume::Touch/UnTouch` override native AActor virtuals
   (`AVolume::Touch` 0x2bf090, `APhysicsVolume::Touch` 0x2c64e0) that this engine does not have (TouchTo raises script events only),
   so they are named, not written; `m_CurrentWaterVolumes` stays empty, the camera-in-water/breath path never starts. Swimming does
   not depend on it (the zone change starts PHYS_Swimming).
6. The route needs `-nomovie` (Bink briefing page, IA), and the dock route cannot walk (above).
7. One build sufficed: everything of the brief's "build 2" went into build 1.

## 4. Boundaries (named in the code, `DISHONORED(bringup)`), new stubs reached

No new native stub fires (`native not ported`: none of the player's in either run). Reached and named once (`if ...` warnings):
upper/left-arm FSM not initialised (all actions), `UStatePlayerMasterSlide` (slide request, CancelSlide), `HandleHeldButtons_Lean`,
`ADishonoredPawn::setPhysics` powers notification, `PerformMantle` (IG), camera bump smoother, projectiles cancel, fall statistic,
2013 fall events, swim anim state (`SetAnimState`), fish attractor, `LockPickerChildren`, `InvalidateCurAnimState`. Deferred:
`UDisWaterVolumeManager` (`OnTriggerNotify`, rigid-body buoyancy; not reached by the player), the breath gauge HUD calls, the
exit lens effect's activation (`ADisEmitterCameraLensEffect` skeleton), the possession fail sound in `CheckJumpOrDuck`.

## 5. Deviations

- Null tests retail does not have: the master FSM in `setPhysics`/`IsSneaking`, the upper FSM in `IsCarryingCorpse`, a NULL
  equipped item (taken as empty hands) in `MaxSpeedModifier_Derived`/`PlayJumpInActions`/swim, a NULL camera before `SetFOVTarget`,
  a non-Dishonored pawn in `_Movement_Gamepad`.
- `ADishonoredPawn::IsPossessing` is the player's `DisIsPossessing` in `setPhysics`/`Tick_Water`.
- `GetSwimStrokeAccel`: the DLC branch (2013 0x7bf1f0) is not written; the campaign's game info is `DishonoredGameInfo`, for which
  it is FALSE.
