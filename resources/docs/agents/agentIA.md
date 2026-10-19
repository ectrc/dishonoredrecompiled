# Agent IA — FOV after the ride (wave 26, PHASE23 package IA)

Opus 5.5, worktree `build/agentIA_wt` (base `5ddc8a1` = `33d1c80` + the PHASE23 status line), build dir `build/agentIA`
(RelWithDebInfo), staged `DishonoredGame_IA.exe`. One build (the second was not needed). IDA only on the private copies
`build/agentIA/idb/{retail2013_agentIA,shipping2012_agentIA}.i64`; every address below resolved in both (2013 VA = rva + 0x400000).
No fmodel, no retail `Dishonored.exe`, no real input; every run through `game_input.py launch` / `cutscene_compare.py run`, windowed.

## 0. Step 0 — the measurement (before any port)

Route `-finewgame=50 -fosoiree -fzdiag -pcamdiag -nomovie` (`-nomovie`: see §4.1), marks world 50–60 and 140–162,
`cutscene_compare.py run --tag IA0` on the base exe (`DishonoredGame-Win64-Shipping.exe` = head_rel = d6c3aa4 code; 33d1c80 changes
docs only). Alignment = the tool's anchor offset with b fixed to 1: **retail_t = world − 42.805** (E1 −43.21, E2 −42.40, F −43.44;
residual 0.40 s). retail.mp4 ends at 112.1 s = world ≈ 154.9, so the post-ride comparison covers world 151–154.

| world | retail t | phase | zoom (ours/retail scale) | NCC | ours hfov / vfov | DefaultFOV | m_fCurFOV | FOV-target slots |
|---|---|---|---|---|---|---|---|---|
| 57–60 | 14.2–17.2 | ride start | 0.99–1.00 | | 75.00 / 46.69 | 75 | 75 | none |
| 140–150 | 97.2–107.2 | ride end | 0.98–1.01 (median 0.99) | .26–.64 | 75.00 / 46.69 | 75 | 75 | none |
| soirée EXIT 150.6 | 107.8 | | | | | | | |
| 151 | 108.2 | gameplay | 0.99 | .22 | 75.00 / 46.69 | 75 | 75 | none |
| 152 | 109.2 | gameplay | 1.00 | .30 | 75.00 / 46.69 | 75 | 75 | none |
| 153 | 110.2 | gameplay | 1.00 | .30 | 75.00 / 46.69 | 75 | 75 | none |
| 154 | 111.2 | gameplay | 1.00 | .35 | 75.00 / 46.69 | 75 | 75 | none |
| 155–162 | (past retail.mp4) | gameplay | – | | 75.00 / 46.69 | 75 | 75 | none |

hfov/vfov from `pcamdiag proj` (`CC_IA0.log`, axis horizontal, M00 1.3032), DefaultFOV / m_fCurFOV from `fzdiag camera`; the slot
column is from the build-1 run (`-iadiag`, `CC_IA1.log`: "slots 5 active: none" every second from world 0 to 162). The wave-25 final
run (`build/cmp/final`, same exe) agrees: post-ride zoom 0.99/1.00/1.00/1.00 at world 151–154.

**The user's retail FOV slider is 75.** `OPTIONS.SAV` read read-only
(`C:\Program Files (x86)\Steam\userdata\1919909936\205100\remote\OPTIONS.SAV`, 584 B, 2026-10-14 21:24). Format (it is UE3's
`FProfileSettingsWriter` with the digital signature on, `IpDrv/Inc/OnlineSubsystemUtilities.h` of the reference tree): bytes 0–19 SHA-1
of the rest (verified), bytes 20–23 big-endian uncompressed size (1277), then an LZO1X stream (`resources/tools/pdb/lzo1x.py`
decodes it) holding an `FNboSerialize` buffer: int32 BE count (115), then per setting `Owner` byte, `PropertyId` int32 BE, `Data.Type`
byte (1 Int32, 2 Int64, 5 Float, 6 Blob = int32 length + bytes), the value big-endian, `AdvertisementType` byte. **Id 118 = 75.**
Others of note: ids 29–64 (the 36 bindings) all 0 = unbound (HJ's "[Unbound]"), 112 = 2.2 (gamma), 71/73/82/84/101/126/129 = 100,
127/128 = 90, 78/79 = 20. The scratch reader is not committed (IJ's job); nothing was written.

**Answer: neither side of the ride is wrong.** Ours is 75° horizontal before, during and after the ride, and retail matches it on
both sides within the fit's step: ride zoom median 0.99 (±0.01 ≈ ±0.75°), post-ride 0.99–1.00 (hfov 75.0 ± 0.75°). Retail's
gameplay FOV is the slider's 75, ours is `SetToDefaults`' 75 (`ApplyGameSettings setting FOV 75 -> DefaultFOV 75.00`, reason 0 and 1).
Nothing in retail changes the FOV at the ride's end: the full list of `SetFOVTarget` callers in 2013 (xrefs of VA 0xaca500 = rva 0x6ca500) is
HolePeeking FinishFade 0x663400, Jump DoPowerJump_Fixed 0x671370 / TickJumpLogic_ContinuousPower 0x6762e0, UpdateWalkNavAnimState
0x675450 (crouch only), TickCameraTracking 0x6b4e60 (flag 0x100 zoom), two item zoom functions 0x80c150 / 0x80ffe0 (spyglass-type
`m_ZoomLevel`), 0x8aa910 (Item slot); none runs on this route.

## 1. Where the plan was wrong, with the measurement

1. **"Retail's ride FOV and gameplay FOV differ" is not a FOV difference.** Zoom ride 0.99 / post-ride 1.00 against the same 75°;
   the slider is 75 (above). What does differ between retail's ride and its gameplay, and is the same in ours: the ride is letterboxed
   (black bands ≈ 50 px of 540 top and bottom, both builds) and shows the full body from the boat's camera bone. Frame sharpness
   (Laplacian variance, letterbox excluded) jumps at the exit in both (retail 467 → 899, ours 428 → 765, world 150 → 152), centre
   brightness is within 3 grey levels. If the user still sees a difference by hand, it is not the projection: candidates outside IA are
   the arms (not drawn after the ride in ours, HD §4) and the stance (ours reads NotSet, see §3). Measured only up to world 154.9
   (retail.mp4 ends there); a later retail recording would extend it.
2. **"UpdateWalkNavAnimState sets the locomotion FOV targets"**: only the crouch one. Both bodies (2012 0x69ab60, 2013 0x675450) set
   `eDisCamFOVPriority_Locomotion` to `UStatePlayerMasterWalk`'s class-default `m_fCrouchedFOV` only when the crouched anim state was
   chosen, and clear it otherwise. No walking/sprint FOV.
3. **`tracking.cpp:344-345` is retail's own.** 2013 0x6b5228..0x6b52bd: the zoom starts at 0, the pitch target's override is taken,
   the yaw target's is `Min`'d in (`comiss` / `jnb`), so a yaw-only zoom does ask for FOV 0 (TickFOV skips a 0 target). Kept. The
   real difference was elsewhere: **2013 doubles the blend speed** (`mulss xmm0, 2.0` before `SetFOVTarget(4, ...)` at 0x6b52c4;
   2012 0x6f9ce0 passes it as is). Ported.
4. **`ExitSoireeMode` has no FOV write** (2013 0x6628f0, 2012 0x695a70). Its diff against ours found the stance-gated arm-follow
   re-enable, which our port had skipped because `GetActualPlayerStance` was missing (it exists now, HJ era); ported.
5. **The plan's step-0 route hangs today without `-nomovie`** (§4.1) — not an FOV matter but it blocked every launch-1 agent.

## 2. What was ported (all in build 1)

| function | 2012 | 2013 rva | file | notes |
|---|---|---|---|---|
| `UStatePlayerMasterBase::UpdateWalkNavAnimState` | 0x69ab60 (749 B) | 0x675450 (757 B) | stateplayermasterbase.cpp | blend time (0 under `m_bHandleInstantEquipChange`, cleared), `m_DesiredPlayerStance[1]`, secondary arm follow by its stance, primary arm follow by branch, crouch FOV target / clear. **Named, not faked:** `SetAnimState` (2013 0x6e1da0) and the stance `RequestAction` (0x6b1d00) + `FStatePlayerChangeReadyStance_Param` (0x673d40) — the picked anim state and the request are reported once. The zoom-item test is a direct call folded onto `xor eax,eax; retn` (2013 0x322980, 2012 0xd9220) in both builds, so it is the constant FALSE |
| `FDisSoireeControlState::ExitSoireeMode` (arm follow part) | 0x695a70 (matched .79) | 0x6628f0 | dissoireecontrolstate.cpp | each hand whose actual stance is not NotReady gets `DisableArmFollow(hand, FALSE, blend, wait)` |
| `ADishonoredPlayerController::TickCameraTracking` (zoom) | 0x6f9ce0 | 0x6b4e60 | dishonoredplayercontroller_tracking.cpp | blend speed × 2 (2013) |
| `FDisPlayerViewTarget::ApplyPlayerControlDelta` | 0x6d7420 | 0x6a34d0 | same | bytes identical between builds |
| `ADishonoredPlayerController::ProcessViewRotation` + exec | 0x6e3e00 (756 B) | 0x6afb50 | same | input dilation, delta into the tracked targets, Kismet pitch/yaw limits, then `APlayerController::ProcessViewRotation`. **Deviations:** the dilation is `WorldInfo->TimeDilation` (`ADishonoredGameInfo::GetBendTimeInputDilation`, 2013 vtable +1076 / 2012 0x62fb00, has no body here — the AnimDriven influence makes the same choice); `UDishonoredWeapon_Ranged::IncreaseDispersion` (2013 0x81cba0) unported, reported once |
| `UDishonoredPlayerInput::GetFOVScale` | 0x6f9380 (209 B) | 0x6b01e0 | dishonoredplayerinput.cpp | (cur FOV / FOV setting) ^ tweak exponent of the secondary item's zoom level (`m_fFOVScaleMultipliers[_Mouse]`); replaces UPlayerInput's FOV/90 for the look axes and the mouse sensitivity. **Written guards:** no item → zoom level 0; FOV setting 0 → 1 (retail divides) |
| `UDishonoredPlayerInput::execGetFOVScale` | 0x63a990 | 0x5f4840 | same | the virtual through vtable +352 |
| `-iadiag` | | | fzsoireediag.cpp/.h, dishonoredplayercamera.cpp | every FOV-target change (priority, value, speed, lockArms, previous), DefaultFOV at PostBeginPlay and each ApplyGameSettings (setting, reason), once a second DefaultFOV / settings / cur / arms / default speed / every active slot |

Generator: `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` dropped the two stubs
(`execProcessViewRotation`, `execGetFOVScale`); `--out-dir build/agentIA/genchk`: 79/79 files byte-identical ignoring CRLF.
`bitfield_audit.py`: mismatches=0.

## 3. Results after the port (build 1, `cutscene_compare.py run --tag IA1 --baseline build/cmp/IA0/scores.csv -- -pcamdiag -iadiag -nomovie`)

| metric | IA0 (base) | IA1 (build 1) |
|---|---|---|
| zoom over the ride (median, min–max) | 0.99 (0.98–1.01) | 0.99 (0.98–1.01) |
| post-ride zoom world 151/152/153/154 | 0.99/1.00/1.00/1.00 | 0.99/1.00/1.00/1.00 |
| post-ride hfov | 75.00 | 75.00 (retail 75 ± 0.75 from the zoom) |
| MAD / SSIM medians (34 marks) | 9.96 / .627 | 10.38 / .639 |
| FOV-target changes logged | – | 0 on the whole route (no crouch, no tracking zoom) |
| Criticals | 0 | 0 |

`-iadiag` at boot: `PostBeginPlay DishonoredPlayerCamera_0: DefaultFOV 75.00 settings 75.00 cur 90.00, 5 FOV target slots` — the camera
starts at its class default 90 (PostBeginPlay copies DefaultFOV into m_fCurFOV before the settings republish at its end) and TickFOV
blends it to 75 in the menu (75.00 by world 44); not on the ride's path.

Gate: see §5.

## 4. Boundaries and open items

1. **Every `cutscene_compare run` freezes at the New Game map change unless `-nomovie` is passed** (all launch-1 agents' logs today:
   CC_IB1a, ID0, IF0 stop at "Committed map change" at world ≈ 50.2). Stack scan of the frozen main thread (`build/agentIA/runs`,
   symbolised through the .map): `UGameEngine::Tick -> ConditionalCommitMapChange -> UEngine::StopMovie ->
   FDisFullScreenMovieBink::GameThreadRequestDelayedStopMovie -> FFullScreenMovieBink::GameThreadWaitForMovie` — the game thread waits
   for the Bink loading movie, which never ends (also under `-nullrhi`). HK made Bink default ON (d6c3aa4); the HH final run on the same
   exe passed on 2026-10-15, so the movie's end depends on something in today's environment (the window comes up 1008x567, i.e. the
   desktop is not the 1920x1080 one). `run_regression.py` already passes `-nomovie`; `cutscene_compare.py` does not. Coordinator: add
   `-nomovie` to `cutscene_compare.py`'s launch line (or fix the wait), not done here (tool outside IA).
2. **The stance system**: with `RequestAction` unported, `m_PlayerStance` stays `eDisPlayerStance_NotSet` (0), so the walk picks the
   `*_Ready` branch and keeps both hands' arm follow on (retail, after its stance action lands NotReady for the empty hand, disables the
   primary's). The code is retail's; the input is not. Lands with the upper-body action FSM (IF/IG territory).
3. `SetAnimState` (2013 0x6e1da0) — the walk's full-body nav anim state — named, not set.
4. `GetBendTimeInputDilation`, `IncreaseDispersion` — named above.
5. Step 2(5) (Options slider → hfov) not needed: the slider is 75. The settings path itself is confirmed by `-iadiag`
   (`ApplyGameSettings ... setting FOV 75 -> DefaultFOV 75.00`). IJ is not needed for FOV; it is still needed for the bindings
   (all 36 are 0 in the user's file).
6. `GetFOVScale` makes mouse look 90/75 = 1.2× faster than the engine fallback did (exponent from the tweaks at zoom level 0); this is
   retail's sensitivity. `inputtest` measures movement, not look.

## 5. Gate

`python resources/tools/run_regression.py --build-dir ../agentIA --no-build --exe-name DishonoredGame_IA.exe --log-prefix IAg_ --only
d3d9,inputtest,newgame` from the worktree (`build/agentIA/gate.txt`): **21 ok, 2 failed, 0 Critical**, both failures load-sensitive and reproduced by the base exe in the same minutes:
`d3d9_frames` 600 (bound 1000; 4,380 "under load" in the baseline note, four other agents at 100 % CPU here) and `inputtest_moved`
216 / 751 (retry). Interleaved A/B of the inputtest stage alone (`--no-load-retry`, `build/agentIA/inputtest_ab.txt`): base
(head_rel exe as `DishonoredGame_IAbase.exe`) 1046.2 then 347.5; IA 1073.0 then 481.6 — the two builds swing together with the load,
IA is not below base. `inputtest_peak_speed` 500.0–520.1 (base 500.0–501.6). `inputtest turned` 5504 vs base 5103 in the quiet pair
(`GetFOVScale`'s retail look sensitivity, §4.6). `unported_natives` **0**, `probe_natives` **3 / 4** (≤ 4), `d3d9_criticals`,
`inputtest_criticals`, `newgame_criticals` **0**; `newgame_on_mission` 96, `newgame_pawns` 29. Re-run the two load-sensitive metrics on
a quiet machine before the merge.

Files: `build/agentIA_sync.py` (base 5ddc8a1, every file and function), `agentIA_status.csv`, runs `build/cmp/IA0`, `build/cmp/IA1`,
logs `build/agentIA/runs/CC_IA0.log`, `CC_IA1.log`.
