# Project status — 2026-10-14 (wave 24 complete; **wave 25 is `PHASE22.md`: the opening cutscene 1:1 with the user's retail video** — PHASE21's GL..GO plan was never run and is void)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `PHASE1.md`–`PHASE10.md` (waves 1–8,
done — read each "Wave result"), **`PHASE11.md` (waves 9–15)**, **`PHASE12.md` (wave 16)**,
**`PHASE13.md` (wave 17, done)**, **`PHASE14.md`** (wave 18, done), **`PHASE15.md`** (wave 19, done), **`PHASE16.md`** (wave 20, done), **`PHASE17.md`** (wave 21, done), **`PHASE18.md`** (wave 22, done), **`PHASE19.md`** (wave 23, done), **`PHASE20.md`** (wave 24, done), `PHASE21.md` (a wave-25 draft that was never run — void), **`PHASE22.md` (wave 25, current: HA..HH)**. Decisions and fixes: `porting_notes.md`. Per-function status:
`progress.md` + `function_status.csv`. Agent reports: `agents/agent<A..GK>.md`. The retail
instrumentation loop: `dismod_harness.md`.

## Incident 2026-09-25 (read before running anything)

The retail install `D:\RecompileDishonored\Dishonored_Latest2026` once lost `Engine/`, `DishonoredGame/CookedPCConsole`,
`DLC`, `Localization`, `Movies` during the wave-2 merge: `git worktree remove --force` on an agent worktree whose
`build/stage` junctioned those folders followed the junctions (restored since through Steam). Rules since: no junctions
into the retail or reference trees, ever; `stage_retail.py` copies our exe into the retail `Binaries\Win32` (as
`DishonoredGame_<X>.exe` per agent) and refuses stub exes; never recursively delete a directory that may contain a
junction (`resources/tools/unlink_junctions.py` removes links only); each agent runs with its own `--log-name`,
`--ini-dir` and `--exe-name` (`build_and_smoke.py`).

## Target

**Retail 2013 build** (`Dishonored_Latest2026`, engine 9411, CL 1274963, DLC05–07) is what we rebuild. The 2012
symbolized build is a helping hand only: names, types, decompiles. Retail truth in hand:
native class sizes (`types/native_class_sizes.csv`, 2,857 classes), package member lists
(`types/script_classes_2013.json`, 3,043 classes), runtime member offsets of every reflected member
(`types/retail_sdk_layout.json` from the CodeRed dump, `sdk_dump.md`), and a named retail IDA database
(`idb/retail2013_named.i64`; mangled names; addresses are VAs, image base 0x400000).

**The truth sources are wrong often enough that checking is part of the job** — see "Known pitfalls".

## Where we are (HEAD `2a82ebe`)

**The game starts from its own main menu and puts you in the first mission, with the opening scene's
pawns on the boat and the letterbox bars drawn.** Launch the staged
`Binaries\Win32\DishonoredGame-Win64-Shipping.exe` with no arguments: Space past the start screen, then
NEW GAME, the difficulty screen and the brightness screen, then YES on the confirmation.

**Between `b540be6` (wave 17 FE) and `8027ab6` (wave 18 FI) that was not true**: the front end closed
its own menu one second after opening it, so New Game was unreachable. It passed 37/37 anyway — see
"The gate's blind spot" under Known pitfalls. **Enter does not activate a main-menu row** either
(defect 5); Space does, and `-finewgame` drives past it.

**As of wave 24**: **every NPC had been three times its size** — not the matinee ones only: the look-at node's
additive blend handed the reference pose as a delta whenever its pose names did not resolve, which was always,
because no NPC had anything in hand (GH; one function; the guards are men now). **The player no longer falls at
spawn**: retail's `UDishonoredEngine::LoadMap` flushes level streaming before the first tick — ported; 20 of 20
runs clean where the control took 57–79 fall damage every time and died 1 in 7 (GG). **Workers turn in place
and walk** (GK: the stop-animation system, 15 functions; two DN locomotion bodies that were not retail's fixed).
**The attention process runs on all 26 brains** (GI: 85 of 88 functions) — but **retail's head-track look-at is
enemy-only, and nothing in the tree sees** (the vision components are skeletons), so no head turns yet. **Every
pawn has an inventory**; the player's sword attaches to the hand and stays there through the ride (GJ) — but the
arms play the empty-hands pose because the item anim states have no bodies, so **the arms report is still not
reproducible**. **The captain still does not disembark**: his start poly is refused one stage inside retail's
`GetPylonAndPolyFromPos` (ported by GK), and the faithful route is the boat's sockets (`FDisComponentRetargeting`).
Wave 25 is `PHASE21.md`: vision (GL), the captain (GM), the chain end to end (GN), the arms stance (GO).

**As of wave 23**: **the boat ends exactly where its cooked keys say** and the "750 units short" was the dock's
dressing boat — retail holds the player on the boat's `Player` socket through the `InterpTrackAttachment` track,
now ported, and leaves him there (GB). **The player can walk after the ride again** (GB named it: GA's unconditional
`MAT_StopRootMotion` left the player at `RMM_Velocity`; GF ported the player's `StopRootMotionMode` /
`ChangeAnimStateMotionModes`). **An AI go-to arrives** and **`Use` on the captain reaches the Empress
conversation** (GD) — and GD found that **every AI behaviour had been dropping every stim since wave 14**
(`BuildBehaviorFilterStimMasks` was the base body). **The first Empress scene fades back in** (GE: a dialog hook
swallowed by the wrong one-shot). **A walking NPC selects his walk** (GC: `UArkAnimNodeLocomotion`). **The arms
pipeline is ported** (GF) but **there is nothing to equip** — no inventory/equip path exists, so the user's arms
report is wave 24's GJ. **The intermittent death is named**: the player is possessed before the level's static
PhysX shapes exist and falls — ~60 health lost on every run, KillZ on a slow load (GE; wave 24 GG). **The stretched
NPCs are the cinematic sequences' evaluation** (78-joint sequences on 118-joint skeletons), not foot placement
(GF; wave 24 GH). The first act on `4b889ff`: the player walks off the socket and into the back of the Elite who
has not disembarked — his go-to has `no start poly` on the boat (wave 24 GK). Wave 24 is `PHASE20.md`.

**As of wave 22**: the boat ride is a scene, not a slideshow. **The two guards ride the boat with the
player** (GA: retail's `FullMovePawn` floor sweep bases them on `SkeletalMeshActorMAT_2`; 11 of 28 NPCs
based vs 1), **the props animate** (FW: the `ASkeletalMeshActorMAT` anim-control path; the boat and the
rats), **the camera is forced onto the camera bone and the look is clamped through the ride, the body
switches to arms-only and back** (FZ: the soirée player state, `UpdateViewTarget`, the influences; FOV is
75 in retail too - the brief's 90 was the menu camera), **the AS2 machine frees its objects** (FY:
`GASRefCountCollector`, private bytes flat on the menu where the control grew 0.77 MB/s), **a scripted
run never activates the window** (FY: `appShowGameWindow`'s `SW_SHOWNORMAL`, verified 0 activation
lines), **both named natives have bodies and `unported_natives` is 0** (FV: the AI go-to chain and the
relationship-override API; the Dishonored pawn tick, which had no body at all), the **bitfield audit is
the gate's third `verify_phase2` row** and **a regeneration diffs byte-identical** (FX: two real
`USkeletalMeshComponent` bitfield faults fixed). **What the player still cannot do**: leave the boat -
it ends 750 units short of the dock (FW, the matinee's Move track; wave 23 GB); a walking NPC slides
(GA: `UArkAnimNodeLocomotion` is a skeleton; GC); a go-to never arrives (FV: `HandleArrival` never
runs; GD); `Use` does nothing (GD); the screen stays black after the first Empress scene (GE). Wave 23
is `PHASE19.md`. **Process**: the build queue is FIFO since `62f3682` (a gate lost the lock three times
in a row to builds that arrived later); the two-build rule stands (FZ used five).

**As of wave 21**: **the boat ride starts and plays on its own** - the conversation system's direct
path is ported (`DialogInputs` → hook → soirée node → matinee by GUID), the first Dishonored scripted
scene to start without being forced; the guard aboard animates through it (the five-pass track seam,
the anim stack retail drives); NPC heads aim where asked, 1:1; the mouse cursor is gone from the
mission; and the gate is **47 of 47**. **The Empress scene does not start**, and the reason is exact:
its ten entry points sit behind gates and an AI go-to - retail needs the player to **walk** up through
the captain and guard scenes, and unattended nothing walks. That is wave 22's spine (`PHASE18.md`).
Three rules were written mid-wave because agents cost the user their evening: **no fullscreen, no
focus or gamepad, no retail launches at the machine** - all enforced in the engine for a scripted run.

**As of wave 19**: the Options screen no longer crashes on a tab switch (a raw pointer in the action
queue, FJ); the NPCs' heads are level (`UArkAnimNodeLookAt`, FL); the letterbox bars draw from the
shipped command line (FM, after proving retail has no gate on the HUD); and **a Dishonored cutscene
matinee initialises and plays when forced** (FK) — the first one to reach `InitGroupInst` had been
killing the process every time on a script event retail never raises. **The opening cutscene is still
not played on the real route, because nothing activates it** (`activations 0`), and that is wave 20's
package FO. Also: real mouse input works again — the firewall prompts were three of our own
listeners (`98e22d4`).

| Milestone (PLAN.md Phase 6) | State |
|---|---|
| 1. Core+Engine+Launch compile and link | done (wave 1) |
| 2. `Init: Object subsystem initialized` | done (wave 1/2) |
| 3. Packages load, `GEngine->Init()` | done (wave 3) |
| 4. D3D9 device, Scaleform menu | done (wave 8 DC flipped the runtime; waves 9–14 made it work) |
| 5. Mission map, player spawns, input | done (wave 4/5) |
| 6. Save and load | **86.7 % of the object stream** (wave 14 ER): `Dishonored0.sav` restores 6,569 objects and 537,208 of 619,631 bytes, `PostGameLoad` 6,569, 0 unported and 0 partial bodies. `Dishonored1.sav` 183 objects, 21,081 of 294,961 |
| 7. Full campaign | not met |
| 8. Test suite | not met — the regression harness is 37 checks, but milestone 8's suite is not written |

| Area | State |
|---|---|
| Front end | The menu draws, the mouse hovers and clicks, YES quits, Options opens and Escape leaves it, the brightness screen reads a real value and its slider moves the gamma. **It does not yet look like retail's** — nine fidelity faults are the current wave, `PHASE12.md` |
| In-game HUD | Health and mana vials and the stance icon draw and read the live pawn (waves 13 EK, 13 EM). Three of `UI_HUD`'s eight atlases are never drawn |
| AI | A guard adopts a patrol route and starts walking it (wave 14 EP): 8 patrol behaviours, all six routes registered, squads correct, 4 routes adopted. It stops at its first point — three measured blockers, wave 15 EU |
| Save | above. Remaining 82,423 bytes of `Dishonored0.sav` are two bytes wide inside `AActor::GameLoad` |
| Shims | **887** (wave 14 ES, from 1,098), held by `resources/tools/shim_ratchet.py`. Of the original 1,098: 966 provable absences, 121 undecidable from any truth source, **5 members retail actually has** (live defects, wave 15 EW) |
| `Sources.cmake` | an **exclude list**: 806 comment-only skeleton units whose code never compiles. A **port queue**, not a deletion backlog — 813 of the 818 inventoried are alive in retail. Inventory: `agents/agentES_exclude_inventory.csv`, 875 rows |
| Regression | `resources/tools/run_regression.py`, **47 checks** when built inside the harness (37 before `cd5fadd`); `unported_natives` **0 ≤ 0** again since wave 22 FV; `verify_phase2` **3 rows** since FX (the bitfield audit). **Every build on this machine goes through `resources/tools/build_queued.py`**: one build at a time behind a machine-wide **FIFO** lock (`62f3682`), pinned to half the cores by affinity, `-j 8`, below-normal priority - the user's request after four agents building at once pinned all sixteen (31 with `--no-build`, which omits the six build-stage checks). Every merge gates on a clean checkout of its own commit in `build/gate_wt`, never on the working tree |
| Audio | **deferred to Phase 10 as polish** (the user's call): the silent backend loads the real banks and resolves the real event ids. Do not open an audio package until the game plays |

## The gate is GREEN - 47 of 47 - and how it got there

`run_regression.py` is **47 checks**. Every wave-22 to wave-24 merge read **47 ok, 0 failed, 0 skipped** on a
clean checkout (wave 24: `ece8e1b`, `e3bc866`, `8ea61f0`, `2a82ebe`; `0e5207a` lost two game stages to an unqueued
16-job build beside it and read 18/18 on them alone; wave 23: `f7bc754`, `4b889ff`; `d96a331` read 46 on `d3d9_frames` under full-machine builds and 9/9
on the stage alone; `ad4a35d` read 46 - see defect 12), and that number means more than any earlier green board did: since `cd5fadd` (wave 19) the
crash detectors match the structured-exception banner as well as `Critical: `, so a hard crash inside a
stage is now a red row rather than an invisible one.

**The row that was red from `cd5fadd` to `191ef85`** was `inputtest_criticals`, and it was not the
spawn crash the plan called it. It was `UInterpGroup::GetInterpData()`, **a walk this tree invented**
(group → Outer `UMatineeData` → Outer, cast to `UInterpData`), returning NULL for every matinee whose
data is a shared asset under a package rather than an inline object - **51 of the 81 in `l_tower_p`,
every real cutscene among them**. Retail reads the length off the `UMatineeData` and never does the
walk. It was deterministic, not intermittent: it needed FI's spawn port to make the matinee reachable,
and the two "clean" gates were worktrees based before that port. Agent FN named it from ten minidumps,
all faulting at the same instruction. **Do not relax a gate bound to get green; this one was worth
waiting for.**

**Hard rule that follows**: every *_criticals row reads 0 or the merge waits. A pawn death 10 s into
the d3d9 stage with nothing in the log before it (seen once in FO's gate on `15695d8`, wave 20, and once
in GA's on `ad4a35d`, wave 22; absent on every other gate - defect 12) is recorded as unexplained rather
than attributed to load.

## Open defects the user has seen

Waves 16 and 17 closed most of the interface list. What the user has reported and is **still open**:

1. **The Empress scene needs the player to walk there — and now he can, up to the Elite's back.** Wave 23: the
   boat's end is data (GB); the player rides on the socket and walks after it (GB, GF); the go-to arrives and `Use`
   works (GD); the fade is back (GE). What stops the chain on `4b889ff`: the Elite guard standing in the bow does
   not disembark because his go-to has `no start poly` on the boat (wave 24 GK). The earlier text follows.
   ~~**The Empress scene needs the player to walk there, and the boat stops 750 units short of the dock.**~~
   Wave 21 FR ported the conversation system's direct path (the boat ride starts on its own); wave 22 FW
   mapped the whole chain (`agentFW.md` §3: lift → dock → `DisTrigger_8` guards → captain via go-to or
   `Use` → `DisTrigger_16` 'Empress & Regent' → `Use` on the Empress → ... → `Empress Death`, which is
   the LAST of nine scenes, not the door) and proved everything after the entries works when teleported
   to. What stops the walk is the ride boat's matinee ending at 9773,23112 while the moored collision boat
   stands at 10305,22565 (wave 23 GB); then the captain's go-to must arrive (GD) and `Use` must exist
   (GD). The earlier text follows for the record.
   **The opening cutscene is started by the conversation system, and none of it existed here (wave 20).**
   Agent FO proved `SOIREE_Empress_Regent.SeqAct_Interp_12` has **zero input links and one referencer**
   - there is no Kismet chain. Retail starts it from a dialog tree: `DisSeqAct_DialogInputs` →
   `FireDialogHook` → `UDisConv_Soiree::PlayNode_Derived` → `FindMatinee` **by GUID** →
   `USeqAct_Interp::StartFromDialog` (2013 `0x215020`). **All 31 soirée nodes in `l_tower_p` match a
   matinee by GUID**; the whole scripted-scene layer hangs off ten `DisDialogOneShot` actors. The
   first link (`SOIREE_Boat.DisSeqAct_DialogInputs_10`) already fires on our route and dies on a
   comment-only skeleton. **Port size: 66 excluded units, ~60 KB, 179 retail functions, ~10 KB on the
   direct path, specified in call order in `agents/agentFO.md` §9.** Behind it: the
   `UpdateTrackInst` seam is a **five-pass loop** on `TrackUpdatePass`, not a virtual
   (`StretchAnimControl` is 267 of 694 tracks); `UArkAnimNodeStack` - what retail's
   `MAT_SetAnimPosition` actually drives - has **zero bodies**; the `MAT_*` contract is blocked on the
   player FSM. The matinee *runtime* itself now works: FK made a forced cutscene initialise and play,
   FN made the Boat matinee play to the end with its spawned guard as AI actor.
2. **The menu's framing.** Our FOV is **right** — agent FB captured retail's live value as **90.0**,
   post-`UpdateViewTarget`. Our *camera* is in the wrong place: there are eleven identically named
   `CameraActor`s in that level, all at `FOVAngle 90`, and only one is the view target. Retail's exact
   transform is published in `build/agentFB/retail/camera.md`. Agent FE's earlier 61 ± 2° image fit was
   absorbing the wrong camera's position into the FOV.
3. ~~No letterbox bars; HUD on every screen.~~ **Closed, third time, for real.** Wave 18 FI ported
   the bars; they lived in a HUD movie behind `-dishud`. Wave 19 FM opened the HUD unconditionally,
   as retail does - and the HUD then sat on every screen, because retail's `RefreshGlobalUIState`
   hides it per movie and ours had only the blur arm. Wave 20 FP (`c9ac0f1`) ported the whole thing
   and, with it, **retail's owners**: `UDisGlobalUIManager::Init` and `Pre/PostCommitMapChange`
   for the HUD, and the menu map's own Kismet `DisGFxActionOpenMovie_0` for the menu - whose class
   had **no `Activated` at all**, the gap the `-gfxuimenu` bring-up had papered over since wave 8.
   Both bring-ups are deleted; `-gfxuimenu` is inert. The `UI_Global` letterbox (one-argument
   `SetBlackStripes`) is in too. Measured: menu, difficulty and brightness screens with no HUD, the
   mission with gauge and letterbox, in-process, against the same route on HEAD.
   Still open underneath: the mouse cursor stays drawn in the mission (`m_bMouseCursorAllowed` is
   computed; the GFxUI cursor glue cannot read it) - one seam.
4. **The menu's brightness and haze.** Not Kismet post-process — FE walked 1,858 sequence objects
   across all four loaded levels and found none. Our menu already draws 2 fog layers and 24 bloom parts
   with `0 passes not ported`, so the difference is **parameters**; retail's whole graph with all 42
   nodes is captured in `build/agentFB/retail/postprocess.md`.
5. ~~Enter does not activate a main-menu row.~~ **Never true - closed by wave 21 FU.** Through
   `game_input.py` on HEAD, `key Enter` on NEW GAME → `APressed` → `TransitionTo (NewGameScreen)`; Down
   moves the difficulty; Escape goes back. Agent FI's four silent Enters were measured with the retired
   real-input tool against the `-gfxuimenu` bring-up movie (`canInput 0`) that FP has since replaced.
   The README carried the wrong instruction for three waves. ~~**What is still true, and narrower**: on the
   brightness screen Left/Right/Up/Down reach the gamma widget's own `LeftPressed` and the slider does
   not move.~~ **Never true either - closed by wave 22 FY**: `LeftPressed` → `DisOnSettingChange 2.1` →
   `gamma 2.1`, the thumb moves 461..492 → 456..487 → 452..483 on two Lefts and back on Right; FU's two
   captures bracketed Left+Right+Up+Down, which nets to zero.
6. **Text clarity** is better but not perfect, and **nobody has measured it** — no claim either way.
   `GTessellator` is still unported; agent EX ruled it out only for the DPI magnification fault.
7. ~~**The menu exhausts the address space in about three and a half minutes - because the AS2 machine never
   frees an object.**~~ **CLOSED by wave 22 FY (`69bc81f`)**: `GASRefCountCollector` is retail's
   `GRefCountCollector<323>`, Bacon-Rajan over the counts, plus the creator's reference retail's `GPtr` held.
   Measured on the idle menu: control `1,604,634 live / 1,604,634 created` at 325 s and +0.77 MB/s; this
   build `4,990 live / 1,638,260 created / 1,633,270 freed`, private bytes 712.7 → 712.2 MB, flat. The
   earlier text follows for the record. Wave 21 FU: private bytes +0.63 MB/s on the idle menu, flat with `-nogfxui`; the GFx
   census reads `AS2 objects 66471 live / 66471 created` at 30 s and `644869 / 644869` at 150 s - ~4,800
   objects/s, **live always equal to created**, because `GASObjectCollector` (`GFxAS2Object.h:171`) is the
   teardown half of retail's `GASRefCountCollector` only. At the user's 216 fps that is 2 GB in 217 s.
   Wave 22 FY ports the collector. The earlier text of this entry follows for the record. `appError called: Ran
   out of virtual memory` at **217.73 s**, one frame after a normal `scene rendered (46890 so far)`
   census, with the game still on the main menu - a 32-bit process running out of its 2 GB. Agent EZ
   corrected this entry: there is **no exit teardown fault**, the user's log has no `Exit:` line at
   all, and the "thirteen unsymbolised frames" I recorded here are not frames - decoded as UTF-16 they
   are the error message itself, so the stack walker had been handed a text buffer. Open it as "what
   allocates per frame at the menu", and do not start from those addresses.
8. ~~"Windows Security Alert" prompts hold the foreground and no mouse measurement is possible.~~
   **CLOSED by `98e22d4` + `28cf7ec`.** The cause was ours: three dev-only listeners bound a port on
   every launch, so Windows raised a firewall prompt each time — `FDebugServer` (a TCP `listen(16)`
   plus UDP 13650), `FListenPropagator` (UDP 9989) and `FStatNotifyProvider_UDP` (13000/13002, and it
   was enabled by **the retail install's own `DishonoredEngine.ini`**, which retail's shipping build
   ignores because `STATS` is compiled out there). All three are now opt-in — `-debugchannel`,
   `-objectpropagation`, `-UDPStats`. Retail is built `SHIPPING_PC_GAME` and has none of them, so
   off is closer to the shipped game. Measured with `netstat` against the running process: three
   bound sockets before, **none** after. Agent FJ then confirmed the effect: `grep -c FocusLost` is
   **0** across eight runs and about **80 clicks all landed**. It cost five waves of mouse
   measurement before anyone asked *why* the prompts appeared. Useful coordinates it left: Options
   main tabs at **y = 0.216**, x = **0.259** GENERAL / **0.419** CONTROLS / **0.581** GRAPHICS /
   **0.741** AUDIO, selected tab rising to y ≈ 0.138.

9. **The window comes up 1008x567 with `-ResX=1600 -ResY=900`** on eight of ten runs and 1600x900 on
   two, same switches, same isolated ini dir (agent FH; FF saw 1008x567 in every run); 1920x1080 with the
   user at the machine (FW, FZ). **Read a capture's actual size; never assume it.**
10. **"The other two guards on the boat just float" - CLOSED by wave 22 GA (`ad4a35d`)**: the guards were
    unbased pawns on a moving boat; retail's `FullMovePawn` floor sweep (2012 `0x580ae0`) bases them and
    they ride to the lock beside the player. What remains of the user's report: **a walking NPC slides**
    - Dock Worker B walks 850 units at 123 u/s with `Empty_Idle` playing because `UArkAnimNodeLocomotion`
    is a comment-only skeleton (wave 23 GC); and standing NPCs hovered 1-17 units above their floor until
    GA's `FArkComponentMeshOffset` put the mesh on it.
11. **"The FOV and forced perspective is broken" / "I can see my arms when I shouldn't"** - wave 22 FZ:
    the camera IS forced onto the camera bone and the look IS clamped on the ride now (`agentFZ.md` §4);
    the FOV is 75 in retail too and never changes on this route. Arms: retail has no arms component -
    `ADishonoredPlayerPawn::SetBodyMode` switches the one mesh ARMS_ONLY ↔ FULL_BODY, measured 0 → 1 → 0;
    **nothing is equipped on the harness route, so the arms report is not reproduced yet** (wave 23 GF
    equips the sword and measures).
12. ~~**The player is possessed before the level's static PhysX shapes exist and falls**~~ **CLOSED by wave 24 GG
    (`0e5207a`)**: retail's `UDishonoredEngine::LoadMap` → `PostCommitMapChange` → `FlushLevelStreaming` before the
    first tick; this tree had neither override and L_Tower_P's eight sublevels trickled in while the pawn fell.
    20/20 d3d9 runs clean (1312 static shapes at the first tick); the control 20/20 took 57–79 fall damage.
13. ~~**Tall, thin, stretched dark silhouettes**~~ **CLOSED by wave 24 GH (`ece8e1b`)**, and every earlier
    attribution was wrong: **every NPC was exactly 3× its size**, idle guards included — `UArkAnimNodeLookAt`'s
    additive output was 2× the reference translation on every bone because `GetBoneAtomsForSet` handed the
    reference pose as a delta whenever the look-at mode's pose names did not resolve (always: no NPC had an item,
    so `ChooseLookAt` fell to the config's last mode). GF's "148 = healthy" was the 3× pose's extent. One function.
14. **No guard turns his head, because nothing sees.** The attention process is ported (wave 24 GI, 85/88) and
    runs on all 26 brains; retail's head-track look-at fires **only for an enemy**, a friendly glance is
    `FleetingGaze`; and `FDisComponentVisionNPC` / `Vision` / `Observable` are skeletons, so no attention slot is
    ever created. Wave 25 GL. With a sense stand-in on, 8 of 9 runs crash at exit in `IncrementalPurgeGarbage`.
15. **"I can see my arms when I shouldn't" is still not reproducible.** The equip path exists now (wave 24 GJ):
    the sword attaches to the hand and stays there through the ride (the boat does NOT force the hands empty) —
    but the arms play the empty-hands pose, the hand bone 68 units below the view: the item anim-state system
    has no bodies. Wave 25 GO. Also: New Game gives nothing and no key binds the sword (retail too).
16. **The captain does not disembark**, so the player walks off the socket into his back: his go-to's start poly
    is refused one stage inside retail's `GetPylonAndPolyFromPos` (ported, wave 24 GK) — the pylon octree query
    or a LOCAL_SPACE height test on this imported mesh; and retail never teleports the guards, it carries them on
    the boat's sockets (`FDisComponentRetargeting`). Wave 25 GM.
17. One of L_Tower_P's eight streaming sublevels never loads, on the control too (GG; name not logged). Wave 25 GN.

## Wave 24 — the 3× NPCs, the spawn fall, the stop animation, the attention process, the sword

Five packages, all merged and gated (`PHASE20.md` has each outcome; `agents/agentGG.md` … `agentGK.md`).

| Package | Commit | What landed | Where the brief was wrong |
|---|---|---|---|
| GH | `ece8e1b` | `UArkAnimNodeLookAt::GetBoneAtomsForSet` as retail's Edge tree build (one function); `-ghdiag` | the cause was never the cinematic sequences, the linkup, the decode or the stack: every NPC was 3× from the look-at blend |
| GK | `e3bc866` | the stop-animation system (15); `IsThereAnotherForceThanPath`/`IsRootMotion` fixed; retail's `GetPylonAndPolyFromPos` + Ark containment helpers | "0.56 uu off the mesh" is not a gap; `GetPylonAndPolyFromPos` is not shared engine code; the blocker is the captain not the Elite |
| GG | `0e5207a` | `UDishonoredEngine::LoadMap`/`PostCommitMapChange` (flush streaming before the first tick); `UnWorld.cpp` budget from `AWorldInfo` | the order inside `LoadMap`/`BeginPlay` is retail's already; New Game never fell |
| GI | `8ea61f0` | `UDisAIBrainProcessAttention` (85/88), `UDisAttentionInfo_*`, the 24-slot target interface, the real `TickBrain_Senses`, the brain's look-at request wired | the head-track look-at is enemy-only; the process has no sense of its own |
| GJ | `2a82ebe` | the inventory init and equip path, `AttachItemToSocket`, loadouts, `MAT_SetForceInventory`, `exec` in the inputcmd script | New Game gives nothing; no key binds the sword; the boat does not force the hands empty; the arms are posed by the item anim states |

Coordinator's lessons: under the no-queue override a gate's game stages can starve (one gate lost its d3d9 run
and a 216-uu walk; both stages alone 18/18) — re-run the stage alone and report both; a `… | head -N` wait
pipeline hangs a package for hours once the log goes quiet (GJ); the two-build rule was broken by two packages
and is restated in PHASE21.

## Wave 23 — the socket, the walk, the go-to, Use, the fade, the arms

Six packages, all merged and gated (`PHASE19.md` has each outcome; `agents/agentGB.md` … `agentGF.md`).

| Package | Commit | What landed | Where the brief was wrong |
|---|---|---|---|
| GC | `d96a331` | `UArkAnimNodeLocomotion` (34 of 52), driven by the locomotion component; Worker A's `no start poly` named | the angle is `FArkComponentFaceTo`'s; the Empty_Idle slide was retail's own fallback; the silhouettes are not the stack |
| GB | `f7bc754` | `UInterpTrackAttachment` + inst (the player on the `Player` socket), `DoKismetAttachment` ×3, `MAT_Attach/Detach`; the walk regression named (`RMM_Velocity`) | the boat's end is data; the far boat is the dock's; the player IS held in retail; a 2012 rva in a header comment was wrong |
| GE | `cc6b0e7` | the fade (a swallowed dialog hook), the `bad AnimNotify` check removed, head-track sub-processes, the spawn death reproduced and named, parameter-logging native stubs | the fade-out is a dialog-started matinee's track; the death is the spawn fall, not wave-22 code |
| GF | `622fc3a` | the player's `StopRootMotionMode` / `ChangeAnimStateMotionModes` (the walk), the arms pipeline, `UDishonoredPlayerSkeletalComponent` whole, `Tick_Begin` | no equip path exists; `SetBodyMode` is not the arms mechanism; the stretch is not foot placement |
| GD | `4b889ff` | `HandleArrival`/`ComputeDynamic` as retail, `BuildBehaviorFilterStimMasks` per class (every behaviour had been dropping every stim since wave 14), the use FSM and the whole `Use` path to the Empress conversation | nothing enters `STOP_END`; `HandleArrival` did run; the Use path is longer than the brief |

Coordinator's lessons: `d3d9_frames` is a load cliff under unqueued full-machine builds — re-run the stage alone
and report both; the build queue is FIFO but worktree copies of `build_queued.py` predate it until they rebase; three
of the plan's attributions were wrong (the boat's end, the death, the foot placement) and the packages said so.

## Wave 22 — walking to the Empress, the two natives, the bitfield audit, the AS2 leak, the soirée state

Six packages, all merged and gated (`PHASE18.md` has each outcome; `agents/agentFV.md`, `agentFW.md`,
`agentFX.md`, `agentFY.md`, `agentFZ.md`, `agentGA.md`).

| Package | Commit | What landed | Where the brief was wrong |
|---|---|---|---|
| FV | `02eb527` | `execOnAIGoToActor` + the go-to behaviour (45 functions), `execOnSetDisposition` + the relationship-override API, `ADishonoredPawn::Tick`/`ADishonoredNPCPawn::Tick` (had NO body), `UpdateFloorAndBase`; `unported_natives` 0 | the tick bodies did not exist; three addresses wrong; `IsBehaviorFinished` ICF-folded |
| FW | `0e45ea1` | `ASkeletalMeshActorMAT` anim control (props animate); the Empress chain mapped end to end; `-fwteleport`, `-fwdialogmap`; FO's `-foupstream` comma bug | `Interp_12` is the END of the Empress sequence; the guard scene is a player touch; the boat stops 750 uu short |
| FX | `90d490e` | `bitfield_audit.py` in the gate (2 real faults on `USkeletalMeshComponent`); the generator regenerates 100 files byte-identical | Engine headers are never generated, so FP/FS/FT's edits could not be dropped |
| FY | `69bc81f`, `03f7579` | `GASRefCountCollector` (the 2 GB leak); the WH_CBT hook that named `appShowGameWindow` as the activation; `SW_SHOWNOACTIVATE` (verified) | the brightness slider was never broken; Core cannot call WinDrv (`CoreSmoke` LNK2019 - the coordinator moved the function) |
| GA | `ad4a35d` | the guards ride the boat (`FullMovePawn` floor sweep); `UInterpTrackInstLocomotion`; `FArkComponentMeshOffset`; `ChangeAnimStateMotionModes` and the MAT_ root-motion/mesh-translation bodies | the floaters were unbased pawns, not root-motion anims; "`MovePawn` never reached for a matinee pawn" is wrong |
| FZ | `785e91d` | the player master FSM (Base/Walk/Soiree/InDialog; 23 states inventoried), `UpdateViewTarget` + influences, `SetBodyMode`, `GetBone_ByName`'s angle fixup | FOV 75 not 90, never forced; no arms component; five builds against the two-build rule |

Coordinator's lessons: run the generator and the bitfield audit on every merge (two of FV's hand edits
were regenerated away cleanly); a Core change needs a `CoreSmoke` link before hand-back; the build lock
had to become FIFO; the harness overwrites a stage's log on re-run - copy a red run's log aside before
re-running.

## Wave 21 — the conversation system, the track seam, the look-at, the cursor

`PHASE17.md`. Five of five merged, each 47/47 on a clean checkout of main; four coordinator commits
the user asked for mid-wave.

| | gate | |
|---|---|---|
| `e60c5c6` | — | **no fullscreen, no focus theft, no gamepad in a scripted run** - splash, resize, D3D9 `NOWINDOWCHANGES`, joystick poll; measured by sampling the foreground every 3 s |
| `14cad0f` FU | 47/47 | the cursor leaves the mission in retail's shape; **Enter was never broken**; the 2 GB is the AS2 heap |
| `d24f1bf` FS | 47/47 | the five-pass track seam, 18 `StretchAnimControl` functions, `UArkAnimNodeStack` in the Engine; **`UInterpTrack`'s bitfield was read in the wrong order** - every NOEXPORT class may share it |
| `2ff7daa` FR | 47/47 (`d1f282c`) | **the boat ride starts on its own**: 32 dialog units, 135 functions; the Empress needs the player to walk |
| `c1c4945` FT | 47/47 | `FArkComponentLookat` 36/51, heads aim 1:1; **every NPC bone was at scale 2.0** - retail PC evaluates skeletons through EdgeAnim, not `GetBoneAtoms` |

**Found in nobody's brief**: the bitfield order (FS), EdgeAnim (FT), the spawner never raising
`DisSeqEvent_Spawned` (FR), the cursor bit reading the wrong movie (FU), the AS2 leak (FU).
FR corrected nine premises, FT seven, FS six, FU six.

## Wave 20 — the red row, the cutscene's activation, one owner for the HUD, the unload list

`PHASE16.md`. Four of four merged, plus two coordinator commits the user asked for:

| | gate | |
|---|---|---|
| `15e8a10` | — | **`build_queued.py`**: every build behind one machine-wide lock, pinned to half the cores by affinity, `-j 8`, below-normal priority |
| `b759fd5` | — | **`-inputcmd` + `game_input.py`**: scripted input injected inside the process after DirectInput; the window is never activated and the real mouse never read. `drive_input.py` retired |
| `15695d8` FO | 45/2 | the opening cutscene is **the conversation system**, 66 units, specified; nothing behavioural ported, deliberately |
| `e14b934` FQ | 46/1 | the movie-root unload list; the **mask hit test** (after a scroll every tab click hit a hidden row); clips name themselves; the user's crash path run clean six times |
| `191ef85` FN | **47/47** | the red row: `GetInterpData()` is a walk this tree invented; `UDisSeqVar_SpawnerPawn` handed out the spawner. The Boat matinee plays to the end |
| `c9ac0f1` FP | 47/47 (after `80cc219`) | the HUD hidden where retail hides it; `UDisGlobalUIManager::Init`, `Pre/PostCommitMapChange`, the Kismet open/close actions; both bring-ups deleted |

**Every package corrected five to seven premises of its brief**, and the two biggest findings were in
neither plan: the gate's crash detector matched a string an access violation never produces (wave
19), and the cutscene has no Kismet upstream at all (wave 20).

## Wave 19 — the crash, the cutscene, the heads, and the gate that missed both

`PHASE15.md`. Four of four merged:

| | gate | |
|---|---|---|
| `562f047` FJ | 37/37 | the Options crash: `ActionEntry::pTarget` was a raw pointer where retail holds a `GPtr` and refuses to run an unloaded entry. Found the minidump nobody had opened; recovered the ASLR base without a debugger |
| `98e22d4`, `28cf7ec` | — | the firewall prompts were three of our own listeners, now opt-in; and the exit fault that removing them exposed |
| `ee2313f` FL | 37/37 | the heads: `UArkAnimNodeLookAt::InitAnim` + `GetBoneAtoms` — neither candidate in the brief. The 396 AnimNotify errors are an asset artefact stock UE3 already repairs |
| `cd5fadd` FM | **46/47** | **the gate could not see a crash for nineteen waves**; the New Game and Options stages; `-dishud` flipped after proving retail has no gate; the bars draw |
| `af98872` FK | **46/47** | a cutscene matinee plays when forced; every Dishonored cutscene had been dying on a script `BeginAnimControl` retail never raises; `l_tower_p` has 91 matinees, not 7 |

**Every package found its cause somewhere the plan had not pointed.** FJ corrected six premises of
its brief, FL six, FK five, FM five. The `46/47` is the pre-existing spawn crash, above.

## Wave 18 — the ActionScript library, the portrait, and getting into the game

`PHASE14.md`. **Four of four merged and gated**, each 37/37 on a clean checkout of its own commit:

| | |
|---|---|
| `63c2539` FF | `MovieClip.setMask` was never registered: the options list drew four of twelve rows over the tabs above it. Ported end to end, plus retail's AS depth bias (+0x4000) pinned three independent ways |
| `f72f9f5` FH | the difficulty portrait lands. A `PlaceObject2` carrying `Move|HasCharacter` is a **replace**, and retail's `ReplaceDisplayObject` inherits the transform it replaces for every field the tag omits — ours left it at identity. **+127, +194 off retail → +1, +0** |
| `8027ab6` FI | the opening scene is populated and the letterbox bars draw. And the menu stops closing itself — see the gate's blind spot below |
| `c702a38` FG | the modal's text: the section-sign token was truncated from `C2 A7` to `A7` by a wide-to-narrow copy, so it could never match, and the same AS loop applies its colour. `ParseHtmlImpl` ported. Plus fault 2 — **one row at a time, the row the pointer just left**, because `MarkAllEntriesForRemoval` marked every entry where retail tests the depth |

**Four of the user's five faults are closed with side-by-side measurements against retail.** Fault
1's panel **does not reproduce**: nine scanline measurements give steps with no consistent sign and
the two background scenes differ by far more than the quantity in dispute, so FG withdrew it rather
than pick a side. Its one real component — `maskContent` being rendered — was fixed by FF.

**Five things from this wave worth carrying forward.**

- `MovieClip.setMask` had been noticed missing by **three** separate packages and left each time
  because it never blocked them.
- **Two of the wave's four root causes only work because another package's fix landed first**: FG's
  depth test is a no-op without FF's depth bias.
- **The wave's one semantic collision would have survived a three-way textual merge** and was caught
  by a build: FF deleted `bScriptCreated`, which FG's first fix used.
- **Two packages corrected each other.** FF could not reproduce fault 2; FG showed why (the pointer
  must *leave* the row). FG's reading of FF's 127 `SetWidgetFocus` errors — mostly a normal AS2
  no-op this runtime logs as an error — replaced FF's.
- **Every package corrected at least three premises of its own brief**, and between them they
  overturned two things this file had recorded as fact: the "PC" frame errors are an asset bug retail
  shares, and `execSetCinematicMode_Native`'s nine booleans are named in `retail_sdk_layout.json`,
  so the dismod run the plan insisted on was never needed.

## Waves 16–17 — the interface

`PHASE12.md` and `PHASE13.md`. Merged and gated, each at 37/37 on a clean checkout of its own commit:

| | |
|---|---|
| `8a04f5f` EX | the window was 1.25x, not the stage: the process is now DPI-aware. And one enum - `VET_Color` where retail holds `VET_UByte4N` - was swapping red and blue on every GFx vertex |
| `a677a21` FA | the six GFx 3.3 3D display properties, gated on `_global.gfxExtensions`, at retail's 55° FOV; and the modal's background blur, which is the uber post-process's depth of field |
| `8901d85` EZ | New Game starts the mission - the play-defaults `-startmap` had `LoadMap`-replaced the world owning all 53 `SeqEvent_Console` nodes - and Options survives a second visit |
| `31bc42c` EY | the brightness screen's five marks exist; the option rows' labels were freed memory |
| `3c7b90d` FC | `attachMovie`'s init object now runs before the class function: the options offset, the ACCEPT/BACK bar and the gamma marks' position |
| `1a34f1c` FD | the five marks are tinted (`flash.geom.ColorTransform` was never installed); and `_xmouse` was **20x too large** - twips where the inverse wanted pixels, which is why any click set the slider to maximum |
| `b540be6` FE | the menu movie is closed on map change, so its vignette no longer survives into the level |
| `6fa1676` FB | the headers were never a font problem: the HTML `<font face>` attribute was being thrown away. Plus the dismod harness and the retail captures |

**The instrument that changed how this works**: `dismod_harness.md`. Agent FC checked a fix against the
running retail game rather than a disassembly, and agent FB's live capture overturned an image-fit
measurement that would have sent the next wave chasing a camera FOV that was never wrong.

## Waves 9–15

`PHASE11.md` is the tracker. Merged and gated, each at 37/37 on a clean checkout of its own commit:

| | |
|---|---|
| `48115da` EH | the mouse re-resolves the topmost entity once a frame — a second caller of `GFx_GenerateMouseButtonEvents` we never had |
| `772434e` EJ | the AI stack behind the brain: 502 objects restored |
| `bdb9c26` EK | the HUD opens, binds 31 of 31 clips and feeds itself from the live pawn |
| `223e5e2` EI | YES starts the mission and YES quits |
| `41cf1c0` EL | Corvo stands in Dunwall Tower, reached from the menu by five key presses |
| `bbcfd86` EM | the HUD is on screen: an atlas index was being read as a character id |
| `d42ccdc` EO | the brightness screen has a label, a value and a slider that moves; B leaves Options |
| `4e92b19` EN | the Ark component layer: 1,111 objects restored |
| `cd04626` ES | the shim backlog 1,098 → 887, the exclude inventory, the ratchet |
| `84e35f4` EQ | the settings republish runs: nine bodies, a tenth listener, no abort |
| `89aefaf` EP | a guard walks a patrol in the Tower |
| `f2963c0` ER | 86.7 % of the save stream |

Wave 15 (ET perception, EU the patrol's blockers, EV the last 82 KB, EW the five layout defects) was
**stopped by the user before any of it landed**. Its four worktrees still hold partial work:
`build/agent{ET,EU,EV,EW}_wt`. `PHASE11.md` has the wave-15 plan as written.

## Next

**Wave 25 is `PHASE21.md`.** GL: vision — `FDisComponentVisionNPC`/`Vision`/`Observable` so the guards see the
player (and the exit crash with a sense present). GM: the captain walks off the boat — the start-poly stage GK
named, then retail's socket retargeting. GN: the chain end to end on main, every stop named, and the streaming
level that never loads. GO: the item anim states so the sword stance plays and the arms report can be measured.
All four running on `2a82ebe`; `build/head_rel` is the control at `2a82ebe`.

**Deferred and said so**: the rest of the UI list (menu camera, haze, text clarity); the retail dismod
measurements; faction tables; the probe family; the NPC inventory override (guards' swords); `DropItemsInSlot`,
the wheel; **wave 15**; audio.

**Process, from the user, all absolute**: `-windowed` on every launch; input only through `game_input.py`; builds
only through `build_queued.py` (FIFO, half the cores, two per package); no retail launches while the user is at the
machine. The override file (`%LOCALAPPDATA%\Temp\dishonored_build_overrides.txt`) lifts the cap and the queue only
when the user says so (it was in place 2026-10-14 13:00–21:10 and is gone). **Never hand-edit a generated
header**: `Inc/CppText/<Class>.h` and `DishonoredLink<X>()` anchors; the coordinator regenerates on merge.

## Known pitfalls

### The truth sources

- **`resources/docs/symbols/vtables.csv` is the 2012 table.** 2013's is shifted for some classes — by
  four slots for `UArkProfileSettings` (agent EO), by one for `UStateNPCMasterDead_Limp` (ER). Pin the
  slot per class.
- **`match_2012_2013.csv` propagates wrong vtables.** A 27-byte `InternalConstructor` matches many
  others, the wrong match wins the global-table vote, and the result is a real function at a real
  address belonging to a different class (agent EL). **Pin a vtable a second way before reading a body
  out of it.**
- **The 2013 database itself mislabels functions** — at least `0x612f00` (EQ) and four the tree had
  copied (EP). `retail_sdk_layout.json` is missing `ADishonoredNPCPawn` outright while listing 25
  siblings, and `script_classes_2013.json` omits the Core intrinsics (ES).
- **`rva_sweep.py` is necessary but not sufficient.** It passes any address that lands inside *some*
  2013 function. Six mislabels were found in one wave only by a by-name audit with `ida_funcs.get_func`.
  Resolve every cited address by hand as well.
- Retail's save five are vtable slots **67..71**: `IsRefSaveable` 67, `IsSaveable` 68, `GameSave` 69,
  `GameLoad` 70 — but see the per-class shift above. `FArchive::operator<<(UObject*&)` is slot **6**.

### Defects that hide

- **An unported `USequenceCondition` kills every Kismet chain it sits on, silently**: it does not
  auto-activate its output links, so the chain ends with no log line and no warning. An unported plain
  `USequenceAction` passes through via `DeActivated` (agent EL).
- **A wrong `IsSaveable` ends the save's object loop quietly.** A misread WORD with bit 15 set is taken
  for an unshared sub-level reference — `STREAM ENDED EARLY`, `0 unported`, no class named. That is how
  one wrong override hid from five consecutive packages (ER). The two guards ER asked for in
  `FLevelLoader::operator<<` are still unwritten.
- **A value that is NULL never appears in the stream log at all**, because `operator<<` returns before
  its debug print on index 0 (ER). Absence from the log is not absence from the file.
- `DISHONORED_SHIM_STATIC` expands to **`inline static`** — shared process-wide, not per-instance. That
  one misreading caused about seventeen defects, **five members retail actually has** are currently
  declared this way, and 108 writes to such a member happen from inside a constructor (ES).
- **C4263 and C4264 are errors** (`CMakeLists.txt`): a drifted override signature becomes a silent
  overload, which already cost a completely dead AI transition path that built green.
- **The gate's blind spot: nothing in the harness presses New Game.** Every stage drives a map
  directly with `-startmap`, so the menu → mission handover the README advertises is never exercised.
  Wave 17's `b540be6` closed the front end's own menu one second after it opened — making New Game
  unreachable — and **passed 37/37 on a clean checkout of its own commit.** Agent FI caught it a wave
  later, while looking for something else. A check that boots with no arguments and reaches the
  mission is wave 19's first item. Until it exists, **a 37/37 gate does not mean the game is
  playable**; launch it.
- **`rva_sweep.py` passes a VA silently.** Agent FH wrote `0xdf0a00` where it meant `0x9f0a00` (the
  image base is `0x400000`) and the sweep reported nothing — the by-hand pass caught it. An address
  resolved without subtracting the base lands mid-function in an unrelated class, which is a
  convincing-looking wrong answer rather than an obvious failure.
- **A unit taken off `Sources.cmake` can compile, link green, and not be in the executable.**
  `IMPLEMENT_CLASS`'s vtable is a folded COMDAT, not a reference into the unit, so MSVC drops the
  object and the symptom is a virtual that will not dispatch (agent FI). It needs an
  external-linkage anchor.
- **A screenshot claimed by "arrived after my launch" can belong to another package.** Agent FF's
  first Options run claimed **nine** bitmaps belonging to FH and FI. `-apshotname=<tag>` plus a glob
  of that tag is the only reliable claim, and it matters most when four packages share one machine.
- **A crash address cannot be decoded by subtracting `0x400000`, and you do not need to recover
  the base either.** The executable is `/DYNAMICBASE`: agent FJ's `0x1cc148a - 0x400000` landed
  644 KB past the end of the image. **Open the minidump instead - its module list carries the
  base.** Agent FN corrected the earlier version of this pitfall: the "per-boot cached base" FJ
  recovered (`0xdb0000`) was FJ's own file's; FJ's gate exe loaded at `0x490000` and HEAD's at
  `0xa60000` in the same session. Resolve the RVA against the build's `.map`, checking its PE
  timestamp against the exe. `build/agentFN/tools/mdmp.py` reads the dump.
- **There are minidumps in `DishonoredGame/Logs/` and nobody had opened one.** `unreal-v9411-*.dmp`
  is written by the same handler that prints the useless two-line stack. FJ's confirmed every number
  it had predicted and handed over the dangling object with its page captured. **Open the dump
  before rebuilding anything.** The printed stack is useless only because no PDB sits beside the
  staged exe, and `CaptureStackTraceHelper` writes `BackTrace[n] = AddrPC` even when `StackWalk64`
  fails - **which is why one failed walk prints as two identical frames**, and why a two-frame stack
  means the walker failed rather than that the fault is one frame deep.
- **Making something optional means auditing everything that assumed it existed.** `98e22d4` made
  the debug channel opt-in; the reference engine's shutdown writes `GDebugChannel->Destroy()`
  unconditionally and got away with it only because the object was always constructed. Every exit
  faulted until `28cf7ec`.
- **A pixel coordinate off a capture is not a pixel coordinate for the input driver.** In one of
  FJ's runs `drive_input.py` reported a 1280x720 client while `-apshottime` wrote a 1600x900 bitmap
  **on the same command line** - a consequence of the desktop resolution flapping. Read both, and
  scale.
- **A check can be green because its pattern never matches anything.** The gate's three crash
  detectors looked for `Critical: `; an access violation logs `=== Critical error: ===` and no
  `Critical: ` line at all, so **the harness could not see a hard crash for nineteen waves**. When a
  metric has never once fired, that is evidence about the metric, not about the code. Ask what a
  failing run of it would look like, and make one.
- **A feature measured only in an instrumented run is not a feature the user has.** The letterbox
  bars were ported, measured and reported as working while being unreachable from the shipped command
  line, because the movie that owns them sat behind `-dishud`. Check a claim against **the way the
  user launches the game**, not only against the switches a package happened to pass.
- **`rva_sweep.py` does not scan banner `/* ... */` comment blocks.** Six of agent FM's seven
  addresses were invisible to a clean sweep. Between this, the VA-for-RVA miss in both directions
  (FH's `0xdf0a00`, FL's eight), the sweep is a lint, not a gate.

### Build and link

- **A native class's bitfield must be declared in the COOKED class's order, not the reference
  engine's.** `UStruct::Link` takes the bit masks from the serialized script class (`SavedBitMask`),
  so a C++ bitfield declared in a different order reads the wrong bits silently: `UInterpTrack`'s
  `bDisableTrack` read retail's `bAIGroupOnly`, and every non-player anim group said "no anim-control
  track" for twenty waves (agent FS). Every NOEXPORT class declared in reference order can carry the
  same fault - wave 22 FX audits them against `retail_sdk_layout.json`.
- **Retail PC does not evaluate skeletons through `GetBoneAtoms`.** It goes through `GEdgeAnimManager`
  (EdgeAnim jobs), whose leaf masks the bones a channel does not define. Our stock
  `UAnimNodeBlendBase::GetBoneAtoms` sums children, which put every NPC bone at scale 2.0 the moment a
  blink channel reached weight 1 (agent FT). `UArkAnimNodeStack::GetBoneAtoms` is the one deliberate
  non-retail function standing in for that masking; it goes if the Edge path is ever ported.

- **An agent's sync script lists what it remembered touching, not what it touched.** Agent FP's
  `agentFP_sync.py` carried 22 files and missed `Inc/CppText/ADishonoredGameInfo.h`, where the two
  declarations for functions it defined live; the worktree built, a clean checkout failed with
  `C2509`, and the gate caught it. Agent FI's generated-header edits were the same shape a wave
  earlier. **Before every merge, diff the worktree against its base** -
  `git -C build/agentXX_wt diff --name-only <base>` plus `ls-files --others --exclude-standard` - and
  compare that list with the sync script's. The gate's clean-checkout build is the backstop, not the
  first line.
- **A package that does four full builds on a serialised half-CPU queue waits longer than it works.**
  Its own build, a "clean build, directory deleted first", a HEAD control build, and the gate's
  in-harness build: 16 builds per four-package wave at 15-25 min each. The separate clean build is
  redundant with the gate's; the per-agent HEAD control is one shared build. Wave 21 drops both.

- **Builds queue; a waiting build is not a hung build.** `build_queued.py` holds a machine-wide lock
  (`%LOCALAPPDATA%\Temp\dishonored_build.lock`, owner pid inside; a dead owner's lock is removed
  automatically) and prints `build_queued: waiting for another build to finish (<owner>)` every 30 s.
  It also pins the build to **half the logical cores by affinity mask** and `-j 8`, and prints the
  mask it read back from the OS - `affinity mask 0xff = 8 of 16 cores` - so a cap that silently failed
  to apply is visible. (The first version printed "8 of 16 cores" while running on all sixteen: ctypes
  guessed the pseudo-handle's type and `SetProcessAffinityMask` failed on an invalid HANDLE. Typed
  argtypes and a read-back fixed it.) `DISHONORED_BUILD_CPU_PERCENT=100` lifts the cap,
  `DISHONORED_BUILD_NOLOCK=1` skips the queue. **Never call `cmake --build` or `ninja` directly.**

- **`Sources.cmake` is an exclude list**, and taking a unit off it is not enough: each module is a
  static library, MSVC takes a member only to resolve an undefined symbol, and a dynamic initializer is
  not one, so a unit nothing calls into is dropped whole. A registrant must be named from a unit the
  link always pulls in, with **external** linkage — written `static ... * const` the compiler drops it
  before the linker sees it (agent EN).
- `gen_classes_header.py <Module> --sdk --module-header --sources-cmake` — **all three flags**. A run
  without `--sources-cmake` once cost 113 unresolved externals.
- `/Zc:alignedNew-` is required: UE3 overrides the global `operator new`/`delete` but not C++17's
  aligned overloads.
- A serializer that writes a member back unconditionally corrupts it, because `Serialize` is also
  walked by the GC's reference collector.
- MSVC lays a run of consecutive virtual overloads out in **reverse** declaration order.
- The Engine link is close to its limit. Reference-only shims in widely included headers must be plain
  `static` with one definition in a `.cpp`.
- A `DECLARE_FUNCTION(execX)` in the generated `*Classes.h` has no trailing semicolon when an inline
  body follows; insert new declarations before such a line, never between it and its `{`.

### Running and measuring

- **The no-grab mode is only as good as its last activation path.** It showed the window
  `SW_SHOWNOACTIVATE` and still took the user's foreground three times over: the splash screen
  (`SW_SHOW`, t=3 s), the `SetWindowPos` in `FWindowsViewport::Resize` on the first map load (no
  `SWP_NOACTIVATE`, t=33 s), and the gamepad poll gated on `HasFocus()` alone, so once the window had
  focus the user's connected controller drove the run (agent FU's Options stage was quit by
  `XboxTypeS_DPad_Left`). All closed under `DishonoredInputGrabDisabled()`, plus
  `D3DCREATE_NOWINDOWCHANGES` on the D3D9 device. **Measure focus by sampling `GetForegroundWindow`
  every few seconds for the whole run, not once** - the first two verification runs passed because
  they looked at the wrong moments. After the fix: 0 of 50 samples over three runs.
- **One residual, measured and named as unexplained**: `WM_ACTIVATE` now logs every activation with
  its cause, and each scripted run records one `WA_ACTIVE` (non-click) at the first rendered frame -
  with `-nosplash`, with the D3D flag confirmed applied, with nothing in the log at that instant. It
  never coincides with a foreground change, so it is thread-local activation (a `SetActiveWindow` or
  `SetFocus` on our own thread), which does not take the user's focus. Source not yet found.

- **A launch without `-windowed` goes fullscreen, because the retail ini says so.** One wave-21 run
  took the user's whole display. Every launch passes `-windowed`; `game_input.py launch` adds it. The
  engine now refuses fullscreen outright under `-unattended`, `-inputcmd` or `-noinputgrab` - at
  startup in `UGameEngine::Init` (`UnGame.cpp`) and on every runtime resize in
  `FWindowsViewport::Resize` (Alt+Enter, Options -> Graphics, `SetRes`) - and logs
  `fullscreen refused`. Never drive the display-mode option in a scripted run.

- **Never touch the user's real mouse, keyboard or window focus.** The user works on this machine
  while waves run, and agents driving the game with `SendInput` + `SetForegroundWindow`
  (`drive_input.py`, now retired and refusing to run) kept stealing the foreground from them. Drive
  the game through **`resources/tools/game_input.py`**: `launch` starts it with `-inputcmd=<file>`,
  which forces the no-grab mode (`SW_SHOWNOACTIVATE`, no `SetActiveWindow`, no capture or clip, every
  real key and mouse message ignored); then `hover X Y`, `click`, `key Enter`, `wait S`, `shot TAG`,
  as fractions of the viewport. Input is injected inside the process after DirectInput through
  `UGameViewportClient::InputKey/InputAxis`, and `FWindowsViewport::GetMousePos` returns the scripted
  pointer so Scaleform follows it. The game logs `DISHONORED(inputcmd): t=<world time> <command>` and
  `idle` - correlate against those. Verified: the foreground HWND is unchanged across a scripted run,
  a scripted click advanced the start screen, a scripted key was delivered, and a script runs exactly
  once. (The first version re-ran the script every poll because `appSaveStringToFile("")` writes
  nothing; the file is now consumed by deletion.)

- **Never `git clean -x` a build worktree.** Nine gitignored reference files live there
  (`retail_sdk_layout.json`, `vtables.csv`, `pdb_functions.csv` and six more) and without them every
  layout check fails with `-1`, "the tool printed no summary line" — which reads exactly like a layout
  regression in the package being gated. `git clean -fd`.
- **A fresh worktree has none of the layout stage's five gitignored inputs** (`resources/docs/types/{all_types.h,
  retail_sdk_layout.json, script_classes_2012.json, script_classes_2013.json, types.json}`). Copy them in
  or every layout metric records `-1`.
- **Run the harness from the worktree's own copy with an absolute build dir.** `build-release.cmd` does
  `cd /d "%~dp0.."` then `cmake -S .`, so it always configures the repo root; a worktree `--build-dir`
  gives `build_*_exit 1` with `build_*_errors 0`, which reads like a broken build and is a path.
- **Throughput metrics measure the machine.** `d3d9_frames` has read 510, 900 and 990 against a bound of
  1000 and then 2040–2370 minutes later; `inputtest_moved` read 662.2 and then 1074.5 on the same
  binary. The harness now re-runs a stage whose failures are **all** load-sensitive and prints both
  numbers; `--no-load-retry` turns that off. Read `d3d9_startup_seconds` first — 3.4 s quiet, 141.9 s
  under another agent's build.
- `L_Tower_P` streams eight levels, and when they arrive late the pawn falls out of the world and the
  "walk" becomes `-distouchprobe`'s teleports (agent EM).
- **Real input**: use the shared `resources/tools/drive_input.py` and pass `--exe <image name>` — every
  agent's window is titled "Dishonored Game" and a run was once driven against another agent's window
  and read as completely dead. Use `hover`/`hoverclick`, not `move`/`click`: a `move` is one DirectInput
  delta and a single delta is droppable, which made a working mouse look dead for a whole wave.
- **Read the driver's own log before believing a real-input measurement.** Every wrong conclusion above
  was visible in it at the time.
- Prefer `-apshottime=<seconds>` for screenshots: it captures inside the process on the game thread, so
  an overlapping window cannot corrupt it. Never use `-apshot`.
- `stage_retail.py` re-copies whatever the build dir holds, so a staged "HEAD" exe stops being HEAD the
  moment a run is launched without `--no-stage`.
- `appStrfind` only matches at a non-alphanumeric boundary, so `-nostartmap` does **not** suppress the
  play defaults' startmap; `-startmap=` does.
- Smoke runs need `-forcelogflush` (the `"--extra-args=-forcelogflush"` form).
- Never use the FModel MCP tools (`mcp__fmodel__*`): UE4-only and the user has forbidden them.
- Never open one IDA database from two processes; agents copy `retail2013_named.i64`.
- The Bash tool collapses `\\` and `\n` in heredocs: write patch scripts with the Write tool. Sources are CRLF.
- **Never `git add -A` while another agent's untracked files sit in the tree** — that swept one package's
  report into another package's commit (ER caught it). Stage explicit paths.
- Agents work **only** in their own worktree, and a sync script runs **worktree → main**, the merge
  direction. One that ran the other way would silently overwrite an agent's work at the next merge.
