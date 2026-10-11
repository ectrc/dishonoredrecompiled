# Project status — 2026-10-11 (wave 18 complete: the opening scene plays, and four of the five faults are measured against retail)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `PHASE1.md`–`PHASE10.md` (waves 1–8,
done — read each "Wave result"), **`PHASE11.md` (waves 9–15)**, **`PHASE12.md` (wave 16)**,
**`PHASE13.md` (wave 17, done)**, **`PHASE14.md` (wave 18, done)**. Decisions and fixes: `porting_notes.md`. Per-function status:
`progress.md` + `function_status.csv`. Agent reports: `agents/agent<A..FI>.md`. The retail
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

## Where we are (HEAD `21db165`)

**The game starts from its own main menu and puts you in the first mission, with the opening scene's
pawns on the boat and the letterbox bars drawn.** Launch the staged
`Binaries\Win32\DishonoredGame-Win64-Shipping.exe` with no arguments: Space past the start screen, then
NEW GAME, the difficulty screen and the brightness screen, then YES on the confirmation.

**Between `b540be6` (wave 17 FE) and `8027ab6` (wave 18 FI) that was not true**: the front end closed
its own menu one second after opening it, so New Game was unreachable. It passed 37/37 anyway — see
"The gate's blind spot" under Known pitfalls. **Enter does not activate a main-menu row** either
(defect 5); Space does, and `-finewgame` drives past it.

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
| Regression | `resources/tools/run_regression.py`, **47 checks** when built inside the harness (37 before `cd5fadd`) (31 with `--no-build`, which omits the six build-stage checks). Every merge gates on a clean checkout of its own commit in `build/gate_wt`, never on the working tree |
| Audio | **deferred to Phase 10 as polish** (the user's call): the silent backend loads the real banks and resolves the real event ids. Do not open an audio package until the game plays |

## The gate is RED, deliberately

`run_regression.py` is **47 checks** since `cd5fadd` (wave 19 FM), and HEAD reports **46 ok, 1
failed**. The failing row is `inputtest_criticals == 1` and **it is a real crash, not a flaky
threshold**:

> At **45.85 s of a 150 s budget**, `-distouchprobe` walks the pawn onto `DisTrigger_9`, which fires
> `SOIREE_Guards.SeqEvent_Touch_0`, which runs `DisSeqAct_StartSpawn_0`, and **the spawn kills the
> process.** Every metric that stage measures is emitted at 45.67 s, which is exactly why it used to
> pass. The one credible stack frame is `UArkComponentContainer::AddNewComponent_Internal+0x2d`; nine
> of the ten others resolve thousands of bytes past a public symbol and should not be believed.

It is **pre-existing and intermittent** — the identical line is in agent FJ's, agent FI's and the
coordinator's own gate runs of HEAD, and absent from FG's and FH's. **Do not relax the bound to
`<= 1` to get a green board**: that one line would hide the next crash as well. **Triaging this is
wave 20's first item.**

**Why nobody saw it before:** the three `*_criticals` metrics matched the literal `Critical: `, which
is what `appError` logs through `NAME_Critical`. An access violation **never produces that line** —
it arrives as `=== Critical error: ===` from the structured-exception handler. So **a hard crash
inside a stage was invisible to this harness for every wave up to 19**, and every "37 ok, 0 failed"
in the tables below was worth less than it says. Confirmed twice over: the user's crash log has
`Critical: ` zero times, and so does the coordinator's own passing gate log that contains a crash.

## Open defects the user has seen

Waves 16 and 17 closed most of the interface list. What the user has reported and is **still open**:

1. **The opening cutscene's own matinee does not run.** Specified by agent FI rather than guessed at:
   seven matinees *are* live in `l_tower_p` and every one is set dressing with **`director NONE`** —
   rats, planks, a whale, the whaleboat — and five frames 45–110 s apart differ by a mean 2.0/255.
   Three named gaps: **`UInterpGroupPlayer`/`UInterpGroupInstPlayer` entirely unported *and*
   `USeqAct_Interp::InitInterp` has no branch that would build one**; **`ADishonoredNPCPawn::MAT_*`**,
   the whole matinee-AI contract, ~40 functions with zero bodies; and **`UpdateTrack` missing on every
   Dishonored-added `UInterpTrack`**. Alongside it, **twelve of fourteen
   `UDisSeqCond_*::Activated` are missing** and two of the fourteen already fire on this mission's
   path, all twelve 2013 addresses hand-resolved — **the skeleton comments carry 2012 rvas**.
2. **The menu's framing.** Our FOV is **right** — agent FB captured retail's live value as **90.0**,
   post-`UpdateViewTarget`. Our *camera* is in the wrong place: there are eleven identically named
   `CameraActor`s in that level, all at `FOVAngle 90`, and only one is the view target. Retail's exact
   transform is published in `build/agentFB/retail/camera.md`. Agent FE's earlier 61 ± 2° image fit was
   absorbing the wrong camera's position into the FOV.
3. ~~No letterbox bars in the opening cutscene.~~ **Closed twice.** Wave 18 FI (`8027ab6`) ported
   the bodies; the user then reported the bars still absent, because `_root.blackStripes_mc` lives in
   the HUD movie and **the HUD movie was behind `-dishud`, which was not in the play defaults** — so
   FI's work was real and invisible to anyone launching the game normally. Wave 19 FM (`cd5fadd`)
   flipped it after proving retail has **no gate at all** on opening the HUD, and a play-defaults New
   Game now logs `HUD: SetBlackStripes(1) ok`. FE's original blocker was never one: nine booleans, no
   optional tenth — the "tenth" is `P_FINISH`'s `EX_DebugInfo` (`0x41`) tail — and
   `retail_sdk_layout.json` names all nine. **The lesson is the one that recurs: a feature measured
   only in an instrumented run is not a feature the user has.** The remaining cinematic gap is
   defect 1, the matinee itself.
   Still open underneath it: the letterbox exists in **two** movies with different `SetBlackStripes`
   arities and the `UI_Global` one (2013 rva `0x7aa610`) is **absent**; and the HUD is opened by two
   racing bring-ups where retail has one owner — `DisHUDBringUp` is latched behind the menu as a
   guard, and the real fix is to port `UDisGlobalUIManager::Init` and delete both.
4. **The menu's brightness and haze.** Not Kismet post-process — FE walked 1,858 sequence objects
   across all four loaded levels and found none. Our menu already draws 2 fog layers and 24 bloom parts
   with `0 passes not ported`, so the difference is **parameters**; retail's whole graph with all 42
   nodes is captured in `build/agentFB/retail/postprocess.md`.
5. **The menu consumes key events and the selection does not move** on the brightness and difficulty
   screens. Keys reach the movie and are handled (13 events, 13 listener calls) but no mouse events
   arrive and the probe reports `focus NULL | canFocus 0 | canInput 0` (agent FB). **And Enter does
   not activate a main-menu row at all** — agent FI measured SPACE working and running
   `TransitionTo(MainMenuScreen)` while four Enters produced **no AS2 trace whatsoever**. This is the
   keyboard path the README tells the user to use; `-finewgame[=<secs>]` is the instrument that gets
   past it.
6. **Text clarity** is better but not perfect, and **nobody has measured it** — no claim either way.
   `GTessellator` is still unported; agent EX ruled it out only for the DPI magnification fault.
7. **The menu exhausts the address space in about three and a half minutes.** `appError called: Ran
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
   two, same switches, same isolated ini dir (agent FH; FF saw 1008x567 in every run). **Read a
   capture's actual size; never assume it.**

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

**The user's standing instruction is "get the UI as perfect to the original as possible", so the
interface list closes before anything else.** Wave 15's four packages (ET perception, EU the patrol's
blockers, EV the last 82 KB of the save, EW the five layout defects) stay paused, not cancelled.

Open, in the order the evidence supports:

1. **`UpdateSettings`** — the one genuine missing prototype method of the three FF raised, and it
   does not occur on any screen FG's driver reaches, so it needs a screen nobody has driven yet.
   **Not** the 127 `SetWidgetFocus` errors: FG showed those are `_curWidget_mc` undefined on the
   first selection, a normal AS2 no-op this runtime logs as an error, with every options symbol
   tracing healthy to `MovieClip.prototype`.
3. **The matinee** (defect 1), which is now specified down to three named gaps.
4. **A New Game check in the regression harness**, per the blind spot below.
5. The menu camera's transform (defect 2) and its post-process parameters (defect 4), both already
   captured out of retail in `build/agentFB/retail/`.

**Used and absent in the AS2 library**, from FF's survey — evidence, not a checklist, and thirteen of
the fifteen classes previously listed are referenced **nowhere** in any cooked movie:
`MovieClipLoader` (4 `new`; Missions, gamepad mapping, save thumbnails, `EquipmentIcon`),
`TextField.setImageSubstitutions` (6 sites, the inline `$A$` button glyphs, pairs with
`ParseHtmlImpl`), `Selection.disableFocusKeys` (6 sites, one property write). Separately: the 19
`X360_*` `loadBitmap` failures are `_common.EmbedImg`'s PC branch falling through into the Xenon
block, and those images are exports of `lib.gfx` — **image exports are not resolving across the
import**.

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
- **A crash address cannot be decoded by subtracting `0x400000`.** The executable is
  `/DYNAMICBASE`, so the load address is randomised per boot: agent FJ's `0x1cc148a - 0x400000`
  landed 644 KB past the end of the image. Windows caches an image's ASLR base per boot, so
  `CreateProcessW(CREATE_SUSPENDED)` on the same unmodified file plus `NtQueryInformationProcess`
  and `PEB->ImageBaseAddress` recovers the base that a crash from the same boot session used
  (3 of 3 launches identical). Then resolve the RVA against the build's `.map`, checking its PE
  timestamp against the exe.
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
