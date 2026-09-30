# Project status — 2026-09-30 (the game starts from its own menu; the menu does not yet look like retail's)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `PHASE1.md`–`PHASE10.md` (waves 1–8,
done — read each "Wave result"), **`PHASE11.md` (waves 9–15)**, **`PHASE12.md` (wave 16, current)**.
Decisions and fixes: `porting_notes.md`. Per-function status: `progress.md` + `function_status.csv`.
Agent reports: `agents/agent<A..EW>.md`.

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

## Where we are (HEAD `e70f8c4`)

**The game starts from its own main menu and puts you in the first mission.** Launch the staged
`Binaries\Win32\DishonoredGame-Win64-Shipping.exe` with no arguments: Space past the start screen, Enter
through NEW GAME, the difficulty screen and the brightness screen, then YES on the confirmation.

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
| Regression | `resources/tools/run_regression.py`, **37 checks** when built inside the harness (31 with `--no-build`, which omits the six build-stage checks). Every merge gates on a clean checkout of its own commit in `build/gate_wt`, never on the working tree |
| Audio | **deferred to Phase 10 as polish** (the user's call): the silent backend loads the real banks and resolves the real event ids. Do not open an audio package until the game plays |

## Open defects the user has seen

1. **The menu's fidelity** — nine faults against the seven reference comparisons in
   `resources/reference/menu/{first..seventh}.png`. This is wave 16, `PHASE12.md`, and it is the whole
   current wave.
2. **New Game does not start the mission from the shipped build.** `OnNewGameConfirm(difficulty 3)`
   issues `ce ChangeLvl_StartNewGame` and **nothing follows** — no `SetPlayerTravelDestination`, no map
   change (`Logs/Launch720b.log:3770`). Agent EL measured the same path completing at difficulty 1 in
   its own worktree, so this is either a regression or a condition nobody has isolated. `PHASE12.md` EZ.
3. **Exit teardown faults.** A clean shutdown (`Exit: Game engine shut down`, `Exit: Windows client shut
   down`) is followed by a critical error with thirteen unsymbolised frames. Every quit ends this way;
   killing the process does not.
4. Two **Windows Defender Firewall prompts** raised by agent executables sit over the game window and
   have stolen the foreground from measured runs in three waves. The user has to answer or cancel them.

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

**Wave 16 (`PHASE12.md`) is the menu, and the user has asked for it before anything else.** Wave 15's
four packages are paused, not cancelled.

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
