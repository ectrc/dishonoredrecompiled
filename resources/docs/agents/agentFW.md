# Agent FW — the walk to the Empress: the chain mapped end to end, the player stopped at the bow, the props animate

Wave 22 (`PHASE18.md`), package FW. Worktree `build/agentFW_wt`, base commit `da77f0f`. Two builds: the iteration build in
`build/agentFW_rel` (`fw_build.cmd` → `build_queued.py`, the overnight override) and the gate's in-harness build in
`build/agentFW_reg`; plus a compile-only check of two single objects (`fw_compilecheck.cmd`, no link) for the diagnostic
edits made after the iteration build. The shared control `build/head_rel` (`da77f0f`) staged as `DishonoredGame_FWctl.exe`.
Every launch through `game_input.py launch` with `-inputcmd`, `-windowed`, `-unattended`; every capture in-process
(`shot <tag>`), 1008x567, 1,714,662 bytes each, viewed. The retail `Dishonored.exe` was **not** launched: nothing this
package needed could be measured on it without input, and input is `game_input.py` only.

## 1. The headline

**The route from the water-lock to `SOIREE_Empress_Regent.SeqAct_Interp_12` is mapped, as data, and it is not what the
brief said.** `SeqAct_Interp_12` is conversation 8 of `DisDialogOneShot_0`, **"Empress Death" — the assassination, the
LAST of nine Empress scenes**, reached through Kismet input `'Empress Death'` on `DisSeqAct_DialogInputs_2`, which is fed by
`SeqAct_ActivateRemoteEvent_19 'Last_Teleporter' ← SeqAct_Teleport_4 ← _28 ← _6` (the end of the dialog tree the player
plays through). The FIRST Empress scene is `'Empress & Regent'` (`SeqAct_Interp_0`), and its one entry is **the player
touching `DisTrigger_16`** (`SeqEvent_Touch_69 → SeqAct_Gate_67 (open) → SeqAct_CameraFade_3 → DisSeqAct_DialogInputs_23`).
Between the two sit `'So what are the news'` (Interp 14), the player **using** the Empress (`DDH_PLAYER_USED` hook 1 →
`DialogOutputs 'Give_Letter'`), `'GiveLetter'` (9), `'Empress ReadLetter'` (10), `'Regent Interrupted By Emily'` (4) /
`'...By Player'` (11), `'Holding Emily'` (13, behind a `DistanceCheck` sequence and `Interp_15 'InPlace'`), then the
teleporters and `'Empress Death'`. §3 has every link with what it needs; the map came out of a new tracer, `-fwdialogmap`,
which walks each one-shot's dialog tree from its hook nodes to the soirée nodes and names the matinee each one plays.

**The player walks, and stops at the bow of the boat.** A `game_input.py` script turns the view (1 px of `movepx` =
10.833 yaw units, measured) and holds W; on this tree and on the control alike the pawn rides the lift up
(Z −783 → 2411, based on `SkeletalMeshActorMAT_2`, the boat), walks 260 units forward and stops dead at
**X 9994.4, Y 23144** on every heading from −30° to +90° around +X (FW1, FW2, FW3, FW4, FWC1 — five runs, same coordinate
to the unit). Left of that heading is water: a strafe or a −30° walk steps over the gunwale (Z +42) and drops the pawn
into `DishonoredWaterVolume_0` at Z 2152, swimming, where W moves it no further (FW3, FW4). The dock and every one-shot of
the route lie 1,500–7,000 units further +X and 300–1,100 units higher (`DisDialogOneShot_1/9` Samuel at 11549,22720,2645;
guards `_11` 12551,23301,2724 and `_7` 13161,23163,2792; Emily `_15` 14169,22897,2688; the gazebo `_0`/`_2` 15570–15732,
20896–20949, 3546–3559). **The ride boat parks 750 units short of the dock**: retail's `Boat_Arrive` Kismet swaps it for
a moored boat with collision (`SeqAct_ToggleHidden_2 UnHide SkeletalMeshActor_2`, `SeqAct_ChangeCollision_0`, `ToggleHidden_0
Hide SkeletalMeshActorMAT_1`, all three run here through their script handlers, 0 missing), and that moored boat stands at
**10305, 22565** while our ride boat `SkeletalMeshActorMAT_2` ends at **9773, 23112** — the only thing a line check ahead of
the pawn hits is the ride boat's own hull, and the dock side of the bow is water (§4a). The walk is stopped by the boat
matinee's end position, not by Kismet, not by a trigger, and not by the dialog system. **Put the player in the triggers
and everything after them works**: `-fwteleport` into `DisTrigger_8` starts the guards' `SOIREE_Guards.SeqAct_Interp_3`
(0 → 1, through `SeqEvent_Touch_1 → DialogInputs_6 'Start' → StartFromDialog`), into `DisTrigger_16` starts the first
Empress scene `SOIREE_Empress_Regent.SeqAct_Interp_0` (0 → 1, through `Touch_69 → Gate_67 → CameraFade_3 → DialogInputs_23
'Empress & Regent' → StartFromDialog`, finished at 16.06 s, `Soiree_EmpressAndRegent_GoEmily` raised) — the first two
scripted scenes of the Tower after the boat, started by the route's own entries in this tree (§4a). Also measured: the
player can walk, turn and leave the boat **during** the ride — there is no soirée player state to hold him (FZ's package,
`MAT_BeginPlayerGroup` warns once); and after `Interp_0` the screen stays faded to black (FK/FZ: the director/fade side).

**The skeletal props animate.** `SkeletalMeshActorMAT` had no in-game `BeginAnimControl` path at all: the reference
dispatcher tried the script event and `APawn::MAT_BeginAnimControl` and warned. Retail reaches
`ASkeletalMeshActorMAT::MAT_BeginAnimControl` (2012 `0x343eb0`) through the `AActor` virtual, and caches the
`UArkAnimNodeStack` slots in `PostInitAnimTree` (2012 `0x349d60` → `CacheAnimNodes` `0x343ee0` / 2013 `0x321e90`) — neither
existed here. Ported (§5). Measured: the control logs **8** `has neither the script function BeginAnimControl` warnings and
**0** `fkdiag stack ... SkeletalMeshActorMAT` lines; FW1 logs **0** warnings and the stacks of `SkeletalMeshActorMAT_2`
(boat), `_3` (`Idle` advancing 1.07 → 2.07 → 3.03 with the matinee), `_5`–`_8` (the rats, `Idle4`/`Idle2`) and `_0`
driven by `MatineeStackNode` at weight 1.0 — the StretchAnimControl tracks reach the props' `UArkAnimNodeStack` through
`ASkeletalMeshActorMAT::MAT_SetAnimPosition`.

## 2. Premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "`SOIREE_Empress_Regent.SeqAct_Interp_12` ... the opening cutscene — the Empress — still does not start" | It is conversation 8 **"Empress Death"**, the assassination at the end of the Empress sequence, nine scenes and a player `Use` after the first one. The first scene the player reaches is `'Empress & Regent'` (`SeqAct_Interp_0`) behind `DisTrigger_16`. |
| "Its ten `DialogInputs` entry points are `RemoteEvent_34 → Gate_14 → DialogInputs_8`, `RemoteEvent_86/87 → Gate_1 → DialogInputs_1`, and `AIGoToActor_13 'Success (Last)' → DialogInputs_25 '3Days_Forced'`" | Those are the entries of `'So what are the news'` (8), `'Give Letter'` (1) and the captain's forced line (25) — all fed by **other Empress matinees' `Stopped`/`Completed` outputs**, i.e. they are inside the sequence, not its door. `DialogInputs_25`'s inputs 0–2 have no Kismet feeder at all: the captain's `'Three days late...'` is `DDH_PLAYER_USED` hook 1 on `DisDialogOneShot_2` (and Kismet input `'Three days late...'` behind two `CheckStoryFlag`s). `AIGoToActor_13` itself "has NO feeder" at depth 2 — nothing in Kismet activates it on this route; the captain's walk that does run is `DisSeqAct_AIGoToActor_3 'Curnow GoTo 1'` at `Soirée_BoatRide_Completed` (t 151.42), FV's. |
| "Retail needs the player to walk up from the water-lock through the captain scene and the guard scene" | The guard scene (`SOIREE_Guards.SeqAct_Interp_3`, `DisDialogOneShot_11` Kismet `'Start'`) is behind **`SOIREE_Guards.SeqEvent_Touch_1` on `DisTrigger_8`** at 12573, 23294 — a player touch, and it plays end to end when the player is put there (§4a). The captain scene needs the captain to have walked down (FV's `AIGoToActor_13`, which nothing feeds on this route) or the player to `Use` him (`DDH_PLAYER_USED`); standing beside him fires nothing. The 10 s `Delay_18 → GetDistance_8 → CompareFloat_8 → DialogInputs_20` loop that starts at `Soirée_BoatRide_Completed` is **not** the captain's: it measures the player against `DishonoredSpawner_13` for `'SheWaiting'` on `DisDialogOneShot_13`, the woman at the start pier. No `DialogInputs` on the route fires without its matinee starting; what stops the walk is the boat's end position (§4a). |
| "`-windowed` ... the window comes up 1008x567" (STATUS defect 9: 1008x567 on eight of ten runs) | With the user at the machine the window came up **1920x1080** (FW6, 6,220,854-byte captures; FW1–FW4 and FWC1 overnight were 1008x567). A pixel-based `movepx` turn is then 1.9x smaller than planned; the walk scripts here are written in pixels and must be scaled to the capture's width — read the first capture's size, as STATUS says. |
| the player rides the boat as a passenger until released | He is never held: `pawnYaw == viewYaw` turns freely and W moves him from the first frame of the ride — `MAT_BeginPlayerGroup` warns once and no soirée state enters (FZ's package, added to PHASE18 mid-wave). The walk measured here therefore starts from wherever the ride leaves the pawn, which is on the ride boat's deck at its final position. |
| "`-foupstream=<substr>[,<substr>]`" takes a comma list (FO §3) | `Parse()` stops at the first comma by default (`UnMisc.cpp:2331`, `bShouldStopOnComma=TRUE`), so every comma list ever passed matched **its first pattern only** (FW1: three patterns, `3 target(s) matched`, all `AIGoToActor_13`). One-line fix in `fokismetupstream.cpp` (`Parse(..., FALSE)`), in the gate build. |
| "FS put the `MAT_*` contract on `APawn`/`AActor`; the `ASkeletalMeshActorMAT` half is the reference engine's and is apparently not reached" | `AActor` has no `MAT_BeginAnimControl` here (only `ASkeletalMeshActor` and `APawn` declare it) and the reference `ASkeletalMeshActorMAT` half is **not the reference engine's**: the reference caches slots from the script `BeginAnimControl` event, which the cooked Actor lacks, so `SlotNodes` was never filled in game and `MAT_SetAnimPosition` iterated nothing. Retail's `PostInitAnimTree`/`CacheAnimNodes`/`MAT_BeginAnimControl` overrides were missing, not unreached. |
| "`-distouchprobe` walks the pawn into the touch-driven actors of the map" (EM) as a way to fire the triggers | It arms once per process (static target list) and so is spent on the **menu** world of a new-game route (`armed with 2 ... actors` at t 18.7, FW5). `-fwteleport=<actor>@<seconds>` in `fwdiag.cpp` does the same per named actor at a chosen time. |
| FR §2: "the op is reached some other way (not traced)" — `SeqEvent_RemoteEvent_38` reading `activations 0` while `DialogInputs_10` reads 1 | `-fwkismettrace`: `SOIREE_Boat.SeqEvent_RemoteEvent_38 (SeqEvent_RemoteEvent) trigger 1/0` fires at t 56.70 from `SeqAct_ActivateRemoteEvent_12 'Boat Start'` and `DialogInputs_10` activates on the same tick. Events count triggers, not activations; the remote event is the feeder. |

## 3. The chain, as data (`-fwdialogmap`, FW1.log)

Every `DisDialogOneShot` of `l_tower_p` with the hooks that lead anywhere (18 one-shot trees, 49 hook nodes, 401 node lines;
`DDH_` numbers from `DishonoredGameEngineShims.h`: 1 `PLAYER_USED`, 10 `KISMET_ACTIVATED`, 11/12 suspicion, 17 patrol):

```
lock, boat                        DisDialogOneShot_8  (-3792,36321,-13)  Kismet 'Intro' -> conv 3 -> Soiree -> SOIREE_Boat.SeqAct_Interp_1     [fires, FR]
lift                              DisDialogOneShot_9  (11550,22721,2645) Kismet 'Boat_LiftedUp' -> SOIREE_Workers.SeqAct_Interp_3            [fires, FR]
Samuel, dock                      DisDialogOneShot_1 / _9 / _10          hook 1 PLAYER_USED -> 'First Use' / 'Use' (not blocking, no matinee)
guards, dock gate                 DisDialogOneShot_11 (12551,23301,2724) Kismet 'Start' -> conv 2 'Around' (blocking) -> SOIREE_Guards.SeqAct_Interp_3
                                      <- SOIREE_Guards.DisSeqAct_DialogInputs_6 'Start' <- SeqEvent_Touch_1 (DisTrigger_8, 0/1 triggers)   PLAYER TOUCH
                                  DisDialogOneShot_7  (13162,23163,2792) Kismet hook (1 output) ; DialogInputs_0 <- SOIREE_Guards.SeqAct_Interp_1 'Stopped'
                                  DisDialogOneShot_6  (15928,23398,3011) Kismet hook (1 output) -> SOIREE_Guards.SeqAct_Interp_13
captain Curnow                    DisDialogOneShot_2  (15732,20949,3559) hook 1 PLAYER_USED -> 'Three days late...' (blocking, once) -> SOIREE_Empress_Regent.SeqAct_Interp_86
                                      Kismet '3Days_Forced' (same conv) <- DisSeqAct_AIGoToActor_13 'Success (Last)'   [no feeder; FV]
                                      Kismet 'Three days late...' <- CheckStoryFlag_1 -> _2 -> same conv ; 'Captain' -> KISMET DialogOutputs 'LeaveThemAlone'
                                      Soirée_BoatRide_Completed (151.4) -> AIGoToActor_3 'Curnow GoTo 1' [FV: the captain never walks down]
start pier (not on the route)     DisDialogOneShot_13 (-3744,36493,-105) Kismet 'SheWaiting' <- DialogInputs_20 <- CompareFloat_8 'A <= B' <- GetDistance_8 (player vs DishonoredSpawner_13) <- Gate_6 <- Delay_18 (10 s loop from 151.4)
the Empress                       DisDialogOneShot_0  (15570,20896,3546) hook 1 PLAYER_USED -> KISMET DialogOutputs 'Give_Letter'
                                      Kismet 'Empress & Regent'  -> conv 3  (blocking, once) -> Interp_0   <- DialogInputs_23 <- CameraFade_3 <- Gate_67 (open) <- SeqEvent_Touch_69 (DisTrigger_16)   PLAYER TOUCH
                                      Kismet 'So what are the news ? ' -> conv 5 -> Interp_14   <- DialogInputs_8 <- Gate_14 <- RemoteEvent_34 (enabled 0) <- Interp_2 'Stopped'
                                      Kismet 'Empress Welcomes Player' -> conv 1 -> Interp_20
                                      Kismet 'GiveLetter' -> conv 13 -> Interp_9    <- DialogInputs_1 <- Gate_1 <- RemoteEvent_86/87 (enabled 0) <- Interp_1 / Interp_2 'Stopped'
                                      Kismet 'Empress ReadLetter' -> conv 30 (9 speakers) -> Interp_10 ; out 1 -> DialogOutputs 'ReadLetter_Failed'   <- DialogInputs_14 <- RemoteEvent_88 'Soiree_GiveLetter_Completed' <- Interp_9 'Completed'
                                      Kismet 'Regent Interrupted By Emily' -> conv 7 -> Interp_4 ; 'Regent Interrupted By Player' -> conv 4 -> Interp_11 ; 'Emily Introduces Player' -> conv 11 -> Interp_7
                                      Kismet 'Holding Emily' -> conv 14 -> Interp_13   <- DialogInputs_15 <- Interp_15 'InPlace' <- RemoteEvent_30 'Start_Holding_Emily' <- DistanceCheck 'Distance Reached'
                                      Kismet 'Empress Death' -> conv 8 (10 speakers, 32 nodes) -> Interp_12 ; out 1 -> DialogOutputs 'EmpressDeath_Failed'
                                                                 <- DialogInputs_2 <- ActivateRemoteEvent_19 'Last_Teleporter' <- Teleport_4 <- Teleport_28 <- Teleport_6
                                      Kismet 'EmpressProtectsEmily' -> conv 9 (not blocking, 14 nodes)
block player                      DisDialogOneShot_27 (16804,22052,3305) conv 31 -> Interp_26   <- DialogInputs_26 'BlockPlayer' <- DialogInputs_23 'Out'
```

What each link needs, in order: (1) the lift (plays, FR); (2) the player off the boat onto the dock — **stops here**, §4;
(3) `DisTrigger_8` touched → guards' `Interp_3`; (4) the captain: `AIGoToActor_3` (FV) or the player's `Use` on him
(`DDH_PLAYER_USED`, not ported — the hook table's row 1 is `UDisConv_DialogHook` base, evaluated by the interaction
system, not by Kismet); (5) `DisTrigger_16` touched → `'Empress & Regent'` → `Interp_0`; (6) `Use` on the Empress →
`'Give_Letter'`; (7)–(9) the Empress matinees chain by their own `Stopped`/`Completed` outputs through gates that open on
the way; (10) `Interp_12`. Every matinee in the list reads `activations 0` on every run of this package and of FR's.

## 4. The walk — five runs, one coordinate

| run | exe | what the script did | where the pawn ended | capture |
|---|---|---|---|---|
| FWC1 | `head_rel` control | 8 yaw steps of +1000 px, W 6 s | **9994.4, 23144, 2440**, speed 0 for 4.6 of the 6 s | `fw1j` (dock ahead, bow rail below) |
| FW1 | this tree | same script | yaw 1 px = 10.833 units (503 px → +5449, 1000 px → +10833); W 5 s: 9731 → **9994.4**, speed 574 then 0 | `fw1a`..`fw1j` |
| FW2 | this tree | face +X (yaw 5613), W 18 s with `SpaceBar` at 1.2 s | **9994.4**; no jump (Z never above the 2454 of the first step) | `fw2a`..`fw2e` |
| FW3 | this tree | W 4 s, A 3 s, W 4 s, D 6 s, W 4 s, −60°, W 5 s | A took the pawn +X −Y over the gunwale (Z 2518) into the water at 10246, 22755, 2152 (`DishonoredWaterVolume_0`, physics 3); W/D/W move it 0 after that | `fw3a`..`fw3g` |
| FW4 | this tree | six headings from the bow: 0°, +30°, +90°, −30°, −60°, −90° (W 3 s each) | 0°, +30°, +90°: **9994.4** (speed 0); −30°: 9994 → 10605 over the side (Z 2481, falling) into the water at 10744, 22661, 2152; −60°/−90°: swimming, 0 | `fw4a`..`fw4f` |
| FW5 | this tree | `-distouchprobe`, no input | spent on the menu world (`armed with 2 ... actors` at 18.7 s); stopped at 7 min, no capture | none |
| FW6 | gate exe | W 4 s after a +1176 px turn (1920-wide window: +43°, heading −27°), then `-fwteleport` into `DisTrigger_8` @185, `DisTrigger_16` @215, beside the captain @245 | walk: over the −Y side into the water at 10415, 22849; teleports: guards' `Interp_3` 0 → 1, `Empress_Regent.Interp_0` 0 → 1, captain nothing (§4a) | `fw6a`..`fw6d`, 1920x1080 |

The player's yaw after the lift is **52800** on every run (FW1, FW4; `pawnYaw == viewYaw`), i.e. the boat's final rotation;
`movepx 1680 283` from the centred pointer is +1176 px = +12740 and lands on **yaw 4**, dead on +X (`fw4a`: the gangway
planks run straight ahead to the far doorway with the guards in it). Captures and logs: `build/agentFW/captures/<run>/`
(BMP + PNG), `DishonoredGame/Logs/FW1.log`..`FW4.log`, `FWC1.log`; scripts `build/agentFW/fw<N>_cmd.txt`.

The blocker's name and the state of the three `Boat_Arrive` actions come from the gate exe (`-fwdiag` now line-checks 200
units ahead with the pawn's cylinder and names the hit; `-fwkismettrace` prints every handler lookup): **§4a**.

### 4a. FW6 — the gate exe: the blocker, the handlers, and the chain fired from inside the triggers

Run FW6 (`DishonoredGame_FWreg.exe`, the gate's build; `-fwdiag -fwnear -fwkismettrace -distouch -frdiag
-fwteleport=DisTrigger_8@185,DisTrigger_16@215,DishonoredNPCPawn_7@245 -foupstream=SOIREE_Empress_Regent.SeqEvent_Touch_69,
SOIREE_Guards.SeqEvent_Touch_1,Main_Sequence.SeqAct_GetDistance_8,Main_Sequence.DisSeqAct_DialogInputs_20`; the user is at the
machine, the window came up **1920x1080** this time — captures 6,220,854 bytes — so the pixel-based turn was +43° instead of
+70° and this run's own walk went off the −Y side into the water like FW4's −30° heading; the bow stop itself is FW1–FW4's,
five runs). What it adds:

- **The three `Boat_Arrive` actions run.** `fwkismet handler OnToggleHidden on SkeletalMeshActor_2 ... Engine.Actor:OnToggleHidden`,
  `OnChangeCollision on SkeletalMeshActor_2 ... Engine.Actor:OnChangeCollision`, `OnToggleHidden on SkeletalMeshActorMAT_1`
  at t 147.41, and **0 `MISSING` handlers** in the whole run (every Kismet action handler the mission looks up exists in the
  cooked script). `SkeletalMeshActor_2` — the moored boat the swap unhides with collision — sits at **10305, 22565, 2390**
  (`fwdiag near`, hidden 0, blockActors 1, CollisionType 2): 530 units +X and 550 units −Y of where the ride boat
  `SkeletalMeshActorMAT_2` ends (**9773, 23112**; its track by `fwdiag near`: −3909,36718 at 55 s → 2991,23836 at 101 s →
  9132,23063 at 126 s → 9713,23115 at 152 s). The player is based on the ride boat at its end, and the hidden
  `DisStaticCollisionActor_0/_1` (CollisionType 0) are the only other actors within 600. **The ride boat stops ~750 units
  short of where retail parks the moored boat it swaps in**, which is where the dock is (Samuel's one-shots at Y 22720);
  walking +X from our bow reaches the ride boat's own hull (`fwdiag ahead` names `SkeletalMeshActorMAT_2
  SkeletalMeshComponent0` as the only hit, 58 units out, normal +Y, on the heading before the turn) and the water on the
  dock side. Why the ride boat's Move track ends 750 units short is the matinee's (FS/FN's `InterpTrackMove`, or FR's
  lift matinee `SOIREE_Workers.SeqAct_Interp_3` whose `Boat` group has its own Move track) — measured here, not fixed.
- **Standing in `DisTrigger_8` (12573, 23294, 2601) fires the guards' scene end to end**: `SeqEvent_Touch_1 trigger 1/1`
  → `DisSeqAct_DialogInputs_6 'Start'` → (`frdiag`) `DisDialogOneShot_11 ... DisConv_Soiree_0: StartFromDialog on
  SOIREE_Guards.SeqAct_Interp_3` → **`SeqAct_Interp_3` activation 1** → `finished at position 1.83` →
  `Soiree_GuardStoneBridge1_Completed`, `DisSeqAct_ShowLocationDiscovery_1`. Capture `fw6b` (the garden path, a guard in it).
- **Standing in `DisTrigger_16` (15590, 21497, 3566) fires the first Empress scene end to end**: `SeqEvent_Touch_69
  trigger 1/1` → `SeqAct_Gate_67` → `SeqAct_CameraFade_3` → `DisSeqAct_DialogInputs_23 'Empress & Regent'` (+ `_26
  'BlockPlayer'`) → `DisDialogOneShot_0 ... DisConversation_3.DisConv_Soiree_0: StartFromDialog on
  SOIREE_Empress_Regent.SeqAct_Interp_0` → **`SeqAct_Interp_0` activation 1**, `finished at position 16.06`, then
  `Soiree_EmpressAndRegent_GoEmily` and the Emily sequence's `SeqAct_Teleport_25`, `AIGoToActor_1 'Emily Go To Empress'
  Abort`, `Gate_36`... (`DialogOutputs`/remote events, 40 ops in the same tick). The **camera stays faded to black** after
  it (`fw6c` at t 242 and `fw6d` at 272: black with the HUD only) — the fade-in that retail's cutscene brings back is the
  matinee's director/fade side (FK/FZ), not this chain's.
- **Standing beside the captain (`DishonoredNPCPawn_7`, 15736, 20951, 3431) fires nothing** in 40 s (only the 10 s
  `Delay_18 → GetDistance_8 → CompareFloat_8` loop, activations 12–13): his first line is `DDH_PLAYER_USED`, and that loop
  is not his — `GetDistance_8` compares `SeqVar_Player_12` with `DishonoredSpawner_13` for `DisSeqAct_DialogInputs_20
  'SheWaiting'` on **`DisDialogOneShot_13`** (−3744, 36493, −105: the woman at the start pier), so §3's "captain distance
  loop" in the first draft of this report was wrong and is corrected above.

So **everything between the lift and `Interp_0` works once the player is in the trigger**: the touch, the gate, the fade,
the Kismet-activated hook, the conversation, the soirée node, `StartFromDialog`, the matinee, its `Completed` outputs.
What does not work is getting there on foot: the boat parks 750 units from the dock.

## 5. The props — what was ported (Engine)

`UnInterpolation.cpp` `DisMatBeginAnimControl` / `DisMatFinishAnimControl`: after the script event and `APawn`, a
`Cast<ASkeletalMeshActor>` reaches the native virtual (retail's dispatch is the `AActor` virtual; this tree keeps the
virtual on `ASkeletalMeshActor`, so the cast is the whole difference). `UnSkeletalMesh.cpp`, new bodies on
`ASkeletalMeshActorMAT`:

| function | 2012 | 2013 | body |
|---|---|---|---|
| `CacheAnimNodes` | `0x343ee0` (233) | `0x321e90` (219) | `GetNodesByClass(UArkAnimNodeStack)` → `SlotNodes.AddUniqueItem` |
| `PostInitAnimTree(SkelComp)` | `0x349d60` (64) | unnamed | `SlotNodes.Empty(); CacheAnimNodes()` — no component test, unlike `APawn`'s |
| `MAT_BeginAnimControl(Group)` | `0x343eb0` (38) | `0x321e60` (38) | own `InterpGroupList.AddUniqueItem`; `UpdateAnimSetList()` |
| `MAT_FinishAnimControl(Group)` | `0x33e210` (289) | `0x31ba30` (289) | every channel's `UAnimNodeSequence::bMatineePlaying` (+216, mask 0x10) cleared — `UArkAnimNodeStack::MAT_FinishAnimControl` per slot |
| `UpdateAnimSetList` | `0x365540` (74) | unnamed | `RestoreAnimSetsToDefault`; own `BuildAnimSetList`; `UpdateAnimations` under `bDisableWarningWhenAnimNotFound` |
| `BuildAnimSetList` | inlined | inlined | `SaveDefaultsToAnimSets` + `AddAnimSets` of each group on the **own** list (`ASkeletalMeshActor::InterpGroupList` is a `DISHONORED_SHIM_STATIC`, process-wide; the MAT's is retail SDK @632) |

Declarations in `EngineAnimClasses.h` (a generated header; C++-only members, nothing reflected, no regeneration —
FX's generator must learn them like FS's `SlotNodes` line). Addresses resolved by hand in `FW_shipping2012.i64` /
`FW_retail2013.i64` (`build/agentFW/idb/`), decompiles in `build/agentFW/decomp2012/`, `decomp2013/`; `CacheAnimNodes`'
one caller found with `xrefs.py` (`PostInitAnimTree`).

## 6. The instruments

- `-fwdiag` (`fwdiag.cpp`): once a second `fwdiag player` — location, pawn/view yaw, pitch, speed, physics, base, volume,
  touching count; `fwdiag ahead` — the pawn-cylinder line check 200 units along the view yaw (actor, component, hit,
  normal, hidden/collide/block/CollisionType); `-fwnear` the colliding actors within 600 units every 5 s;
  `-fwteleport=<actor>@<seconds>[,...]` places the pawn on an actor at a time (`FarMoveActor`).
- `-fwdialogmap` (`fwdiag.cpp`, on `-fkdiag`'s passes, once per map): §3.
- `-fwkismettrace` (`UnSequence.cpp`): `fwkismet t= op <path> (<class>) activation N via input i '<desc>' comment ...`
  at every op activation, `fwkismet t= event <path> trigger n/max originator instigator` at every event firing, and
  `fwkismet t= handler <name> on <target> (<class>) for <action>: <function path | MISSING>` at every Kismet action
  handler lookup (gate exe).
- `-foupstream=A,B,...` now keeps all its patterns (`fokismetupstream.cpp`).

## 7. Files

See `build/agentFW_sync.py`'s header: every file and function. Engine: `EngineAnimClasses.h`, `UnSkeletalMesh.cpp`,
`UnInterpolation.cpp`, `UnSequence.cpp`. DishonoredGame: `fwdiag.h`/`fwdiag.cpp` (new), `fkmatineediag.cpp` (one
declaration, one call), `dishonoredplayercontroller.cpp` (one include, one call), `fokismetupstream.cpp` (one argument).
Reflected members: none added or moved; no header regenerated.

## 8. Acceptance

Gate: `python resources\tools\run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentFW_reg --newgame-at 50`
from the worktree's own copy (`build/agentFW/fw_gate.cmd`), built in-harness: **47 ok, 0 failed, 0 skipped, 2780 s**
(`build/agentFW_reg/regression/summary.txt`, `build/agentFW/fw_gate.log`). `unported_natives` **2 (bound 2)** — FR's two
named natives, FV's. `build_DishonoredGame_errors 0`, **0 `C4263`, 0 `C4264`**, one `warning C` (`Launch.cpp:409` C4191, EX's,
pre-existing). `newgame_pawns 29`, every `*_criticals` 0, `d3d9_startup_seconds 4.86`. The gate was started overnight, killed
by the coordinator's pause mid-build (`build_CoreSmoke_exit -1` in the first log), and restarted after the fast-forward to
`f14326b` under the daytime wrapper (half CPU, queued); the restart resumed the same `agentFW_reg` directory.

Builds: the iteration build (`fw_build0.log`, 1003 steps, 0 errors, 0 C4263/C4264) and the gate's; one compile-only check
of two objects (`fw_compilecheck.cmd`: `fwdiag.cpp.obj`, `UnSequence.cpp.obj`, no link) for the diagnostics added between
them. Control: `build/head_rel` (`da77f0f`) staged as `DishonoredGame_FWctl.exe`, run FWC1 with FW1's script. Rules kept:
every launch `game_input.py launch` + `-inputcmd` + `-windowed`; no real input; no retail `Dishonored.exe` launch; no FModel;
no clean-delete; every address resolved by hand in both databases (`build/agentFW/idb/`, decompiles in `decomp2012/`,
`decomp2013/`); no reflected member added or moved, no header regenerated (the `EngineAnimClasses.h` lines are C++-only
declarations FX's generator must learn, like FS's `SlotNodes`); the sync list self-checked against `git diff --name-only
f14326b` + `git ls-files --others --exclude-standard source resources` (11 = 11, `NOT IN SYNC LIST` empty).

Captures (all in-process, viewed, real content): FWC1/FW1–FW4 1008x567 (1,714,662 B each), FW6 1920x1080 (6,220,854 B);
PNG thumbnails beside each BMP in `build/agentFW/captures/<run>/`. Logs `DishonoredGame/Logs/FWC1.log`, `FW1.log`..`FW6.log`.

## 9. What is left, specified

1. **The ride boat's end position.** `SkeletalMeshActorMAT_2` ends at 9773, 23112 where retail's swapped-in moored boat
   `SkeletalMeshActor_2` stands at 10305, 22565 (§4a) — 750 units of Move track the boat does not travel. The Kismet swap
   itself runs (handlers found, 0 missing). Suspects, in order: the `Boat` group's `InterpTrackMove` in the lift matinee
   `SOIREE_Workers.SeqAct_Interp_3` (8 keys) / `SOIREE_Boat.SeqAct_Interp_3` "DrawBridge" (the boat moves +X while the
   lift rises, 9132 → 9713 between 126 and 152 s, and stops when the lift stops), the Move track's frame
   (`IMF_RelativeToInitial` vs world) against the actor's placed transform, or a keyed move the matinee runtime drops
   (FS/FN's `UnInterpolation.cpp`). Measure with the boat's track by `-fwnear` against the Move track's last key. The
   walk script `build/agentFW/fw4_cmd.txt` (scaled to the window width) is the before/after; a mantle over the bow is
   **not** the answer — the water is on the dock side.
2. **FV**: `AIGoToActor_3 'Curnow GoTo 1'` at 151.42 (the captain walks down to meet the player) and `AIGoToActor_13`,
   which nothing on this route feeds.
3. **`DDH_PLAYER_USED`**: the captain's first line and the Empress's `Give_Letter` are the player's `Use` — the interaction
   package (`IDisInteractableInterface`), not Kismet.
4. `DisTrigger_8` and `DisTrigger_16` by walking, once the boat reaches the dock; both chains are measured working from
   inside the trigger (§4a). Beyond `Interp_0`: the fade back from black after the scene (FK/FZ), the Empress's `Use`
   (`Give_Letter`), then the gates that the Empress matinees' own `Stopped`/`Completed` outputs open, to `Interp_12`.
5. Not this package's, seen on the way: the forced camera and hidden arms during the ride (FZ), the guards aboard
   animating in place (GA); the props now animate but their physics-asset collision, if any, was not measured.
