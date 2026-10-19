# Agent IG: mantle (wave 26, PHASE23 package IG)

Worktree `build/agentIG_wt` (made at `184c234` = `e1926a1` (IF) + docs; main has since moved to `d2db1e9`, and the sync script flags
what main changed). Build dir `build/agentIG` (RelWithDebInfo), staged `DishonoredGame_IG.exe`. **Two builds, both clean on the
first try.** Build 1 ported the code. Build 2 fixed the finder's tweaks (see §3.1), added
`ADishonoredPlayerPawn::ApplyTweakChanges_Derived`, and rewrote the IGLEDGE scan. IDA was run only on private copies
(`build/agentIG/idb/{retail2013,shipping2012}_agentIG.i64`, scripts `build/agentIG/ig_*.py`; decompiles in `build/agentIG/d12`, `d13`;
whole-db hashes `h13_all.tsv`; vtables `vt12.txt`/`vt13.txt`). FModel was not used.

## 1. What was ported (2012 / 2013 rvas; the 2013 body wherever they differ)

Every 2013 match was resolved by bytes: masked-hash identity, vtable slot (2013 vtables have one extra slot at index 72), or
the call/xref chain.

- **Player pawn** (`dishonoredplayerpawn_mantle.cpp`):
  - `PerformMantle` 0x70ff10 / 0x6d78d0, `Tick_Mantle` 0x7129d0 / 0x6dbd00 (2013 inlines `SetMantleCheckDisabled`).
  - `DoMantle_StepUp` 0x706f00 / 0x6d35e0: three sweeps (to the start, up the edge height, forward 20), the camera step-up
    influence, the power-jump cancel, the arms' `StepUpMantle` node, a 0.4 s window of `m_bCanRunOffLedges` = FALSE.
  - `DoMantle_Anim` 0x70d380 / 0x6cd820, `IsMantleAllowed` 0x6fb830.
  - `Tick_Mantle` is now called in `Tick_Begin` (`dishonoredplayerpawn.cpp`, where agent GF's slot named it).
  - `ClearVisAndMantleSettings` 0x707400 / 0x6d7a20 lives in retail's unit `dishonoredplayerpawn_stealth.cpp`. Its caller
    (`ADishonoredGameInfo::PostCommitMapChange`'s share) is not ported.
  - **New, needed, not in the brief:** `ADishonoredPlayerPawn::ApplyTweakChanges_Derived` 0x71d1d0 / 0x6be9b0, partial. It does
    only the edge finders' `SetTweaks` (`m_pMantleTweaks`, and in 2013 `m_pMantleBlinkTweaks`), then the pawn base. The
    anim-state arrays, health effects and component combat tweaks are named, not written.
- **Edge finder** `UDishonoredMantleEdgeFinderComponent` (17 PDB functions):
  - `Attach` 0x7354c0 / 0x6d5e30 (InitFSM), `Detach` 0x722280 / 0x6c1c40.
  - `Tick` 0x744a30 / 0x6da380 (vtable slot 79): one check per frame within `m_fMaxMantleCheckTimePerTick_ms`, with single-step
    mode.
  - `OnMantleCheckEnd`, `HasValidMantleSpot`, `SetMantleCheckDisabled`, `SetMantleStartPos`, the four Debug* functions,
    `GetMantleSpot`, `ResetMantleFinder` 0x7354f0 / 0x6d5ed0, `GetCurrentMaxTickTime`.
  - `GetTweaks_Derived` / `SetTweaks_Derived` (`m_pTweaks`). The constructor only zeroes the rolling averages, which the
    allocation already does.
  - `FDisRollingAverage::AddValue` 0x887e10 / 0x83d230 (struct CppText).
- **Finder FSM** (`dismantlefinderstate_*.cpp`):
  - Base: `OnExitState`, `InitMantlerInfo` 0x727a20 / 0x6c6b80, `SetMantleCheckSucceeded` 0x727be0 / 0x6c6d40, and the 2013
    failure tail 0x6c1eb0 as `MantleCheckFailed`.
  - `FindClearDistanceAboveOwner` (OnEnter, Tick 0x7440e0 / 0x6d6410).
  - `ForwardSearches` (OnEnter 0x744da0 / 0x6da6f0, Tick 0x7442e0 / 0x6d6600, CalcSearchStartHeight 0x7446a0 / 0x6d69b0).
  - `CheckDestinationValid` (Tick 0x744b70 / 0x6da4c0, `VerifyDestinationIsValid_Grid` 0x743c10 / 0x6d5f40).
  - `CheckCanStand` (Tick 0x72b050 / 0x6ccad0).
  - All four params: ctors and OnPending, `Inc/dismantlefinderparams.h`.
  - `IDisMantleableInterface::ShouldRejectPlayerMantle` and `ADisStaticCollisionActor::ShouldRejectPlayerMantle` 0x631000 /
    0x618020.
- **`UStatePlayerMasterMantle`**, all 18 PDB functions:
  - OnEnterState 0x6be610 / 0x66ec20 (slot 73; 2013 adds `m_bSneakRequired`), OnExitState 0x6be7b0 / 0x66ee00, TickState
    0x6c1e20 / 0x66c590.
  - Intro transition 0x660270 / 0x6636c0, impact anim 0x667c50 / 0x667cb0, OnExitState_Crouching 0x6602e0.
  - The getters and setters (2013 SetAllowPlayerEnd 0x660380, SetAllowPlayerCancel 0x660390), StartCrouchTransition.
  - `FStatePlayerMasterAction_Param` (0x695c10) and `FStatePlayerMasterMantle_Param` (ctor 0x6c3b20 / 0x673020, OnPending 0x6b3980 /
    0x663620).
- **Supporting functions:**
  - `UDisAnimNodeBlendByStepUpMantle` TickAnim / OnStepUpMantle / GetMantleSeqNode.
  - `UDishonoredCamera_CrouchMantleOffset::SetOffset`.
  - `ADishonoredPlayerCamera::EnableCollision` and `GetCamera_CrouchMantleOffset`.
  - `DisAngleDiff` 0x823fc0 / 0x7bb600, `DisFindExtentInDirection` 0x827cc0 / 0x7bff60.
- **`-igdiag`** (`Inc/igdiag.h`, `Src/igdiag.cpp`):
  - The finder's attach (FSM states, which tweaks, their values).
  - Each change of the finder's result, with the spot or the reason there is none.
  - Steps and milliseconds per tick every 5 s.
  - Each `PerformMantle` outcome.
  - The step-up and a 1.5 s Z, physics and state track with a summary.
  - Mantle state enter and exit.
  - `IGLEDGE [x y z yaw]`, a teleport or ledge scan available only under the switch.
  - The generator output is 79/79 byte-identical (`--out-dir build/agentIG/regen`); `bitfield_audit` reports mismatches=0.

## 2. Measurements

| | value |
|---|---|
| finder tweaks, build 1 | `m_pTweaks` None, so the class default with every value 0: one FSM step per tick and never a check (`IG1.log`: "0 checks") |
| finder tweaks, build 2 | `Twk_Pawn_Corvo_Release:pMantleTweaks`: step 12, min 55, max 230, low (step-up) max 120, step-up on, forward 20, search 50, grid 6, budget 0.25 ms (`IG2.log`) |
| finder cost (build 2) | 17 steps per check, one check per tick; 0.017-0.09 ms per tick on average, worst 1.49 ms (load spikes); the blink finder is disabled by its default (`m_bMantleCheckDisabled`) |
| `d3d9_frames` (gate, 90 s, under other agents' builds) | **1680** (bound 1000; the baseline note gives 4,380 under load and 20,220 idle); measured with the jump released, because the gate never holds jump |
| plain jump (Z at Jump to Falling, which is demanded when the pawn stops rising) | 99.9 (spawn), 98.9 / 96.0 / 104.1 (street level); landed in Walk every time. IF measured apex 106.2. Same code path: `PerformMantle` answers "no valid spot" and the jump runs |
| **forward + SpaceBar at a ledge** | **not measured: no mantleable face was ever found** (§3.3) |
| gate `d3d9,inputtest,newgame` | **23 ok, 0 failed**; criticals 0/0/0, `unported_natives` 0, **`probe_natives` 3**, `inputtest_moved` 1067.7 (one run), `newgame_on_mission` 110, `newgame_pawns` 29 (`build/agentIG/gate.txt`) |

Runs, all through `game_input.py` with `-windowed -nomovie`, under `build/agentIG/IG*_run.txt` and `Logs/IG*.log`:

- IG0, IG0b: HEAD (`_IF.exe`) tweak values.
- IG1, IG2: build 1 and 2 on the spawn route.
- IG3 to IG6: the street level below the spawn, with IGLEDGE scans and walks in 8 directions.
- IG7, IG8: the dock after the ride (`-finewgame=50 -fosoiree`).

## 3. Where the brief was wrong (with the measurement)

1. **"The anim path through the upper FSM is the boundary"** is wrong. `DoMantle_Anim` requests a *full-body* action, which
   `RequestAction` sends to the master FSM, into `UStatePlayerMasterMantle`. The real boundary is the master action base:
   `UStateSharedActionBase::OnEnterState` calls DoAction (vtable +372, `UStatePlayerMasterAction::DoAction` 2012 0x695c40) and
   `SetAnimState`, both unported. Without them the state clears the pawn's `bCollideWorld`, has no root motion and waits for
   its 5 s timeout (`m_fMaxTimeInMantleState` 5.0, IG0). So `PerformMantle` refuses edges above 120 uu (named deviation:
   returns FALSE, so the jump still happens) instead of requesting the state. The state's bodies are ported and not reached.
2. **The finder had no tweaks.** Nothing in the tree called `SetTweaks` on the finder: `m_pTweaks` stayed None (IG0 getall),
   and the class default has every value 0. Retail does it in `ADishonoredPlayerPawn::ApplyTweakChanges_Derived` (2013
   0x6be9b0: interface `this`+0x3DC/0x3E0 are pawn +2200/+2204, the two finders, given tweaks +0x708/+0x70C), which was
   unported. That partial port is part of this package.
3. **No ledge to measure the step-up on.** The brief says "pick a crate or ledge from -distouchprobe's geometry list", but
   `-distouchprobe` lists only volumes, triggers and pickups, not geometry. With the finder working (build 2), every check on
   both routes ended "no face to mantle in reach":
   - L_Tower_P `-startmapopen` loads a platform at Z -225 over an open plane at Z -1440. The pawn walked 4,500 uu in each of
     8 directions on the plane without being blocked (IG4, IG6).
   - The IGLEDGE scan (24 rays, 2,000 uu, 60-115 uu steps) found none at the spawn, on the plane or at the dock (IG2, IG3, IG7).
   - At the dock the pawn is stopped 230 uu ahead by something the finder's trace flags do not include. Retail's flags
     (0x800020C6) leave out `TRACE_Volumes`, so the blocker is most likely a BlockingVolume, which retail cannot mantle either.
   - The finder's traces do hit geometry: during the ride the upward clear-height trace shortened some checks to 16 steps
     (IG8, 16.4-16.8 steps per tick).
   - **The acceptance "Z rises by the edge height ±10 within 1.5 s" is therefore unmeasured.** `-igdiag`'s
     `stepup ... summary` line is ready for a route that has a real 55-120 uu ledge.
   - The two remaining builds were spent before I found this, so an explicit `IGLEDGE x y z yaw` at a known ledge is the next
     step: the Hound Pits pub, or the dock steps once their coordinates are known.
4. `UStatePlayerMasterClimb` is the ladder state of `ADisClimbable` (its unit is still excluded), not part of the mantle. Nothing
   on either route requests it. I left it a skeleton and named it.
5. `PHASE23`'s "Accept: ... gate 47/47" is the coordinator's full gate. This package ran the three stages the brief names.

## 4. Deviations (named in the code)

- `PerformMantle` refuses the animated path (§3.1).
- `IDisMantleableInterface::ShouldRejectPlayerMantle` has a FALSE base body where retail's slot is pure.
- `UStatePlayerMasterMantle::OnExitState`: `FindSpot` takes the engine's 3-argument form (retail passes a further `-1.0`).
- `ApplyTweakChanges_Derived` is partial (§1).
- `FDisRollingAverage` and the finder use `appSeconds()`; 2013 drops the 2^24 offset 2012 added, and both sides of every
  difference use the same clock.
- The finder's constructor is not declared (it only zeroes memory the allocation zeroes).
- `IGLEDGE` is a measurement teleport, present only under `-igdiag`.
