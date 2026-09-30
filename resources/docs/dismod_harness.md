# The dismod harness — reading the answer out of the retail game

`D:\Christmas\github\dismod\` is the user's own mod DLL for **retail Dishonored**. It is a `dinput8.dll`
proxy built on MinHook + libhat + a CodeRed UE3 `sdk.hpp`, and it is the only instrument this project
has that reads retail's *runtime* behaviour rather than inferring it from a disassembly or from a truth
source that is itself wrong.

Agent FB got the loop working end to end in wave 17 and added a read-only probe (`src/mods/uiprobe.cpp`)
that dumps fonts, the Scaleform display list, the UE3 camera and the post-process graph. This file is the
whole loop; you should not have to rediscover any of it.

**`D:\Christmas\github\dismod\` is the user's repository, not ours.** Keep changes additive, marked
`dismod-harness`, and off by default. Nothing in it is merged by `agentFB_sync.py`.

---

## 1. The binary you are instrumenting

| | |
|---|---|
| Path | `C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32\Dishonored.exe` |
| md5 | `204f3c1a0de5e6ad77efa75236b0990e` |
| Image base | `0x400000`, and it is **not** relocated: the CodeRed SDK hardcodes `UObject::ProcessEvent` at `0x470640` and that works |
| Same file as | `D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\Dishonored.exe` (byte-identical, checked) |
| Same file as | the input of `resources/docs/idb/retail2013_named.i64` — IDA reports `input_path` = the Steam copy |

That last row is the important one: **every RVA in `resources/docs/symbols/functions_2013.csv` is directly
usable as a dismod hook address**, as `GetModuleHandle(nullptr) + rva`. There is no pattern scanning to do
and no image-base arithmetic to get wrong.

`functions_2013.csv` names are *propagated* from the 2012 PDB, so they can be wrong. Two checks that cost
nothing and caught a real mislabel in this wave:

* the Hex-Rays header comment IDA writes on a propagated function, `// 2012 rva 0xNNNN ratio 1.000 bytes`
  — ratio 1.000 on a function of a few hundred bytes or more is conclusive;
* the first thing the hook logs. If `GFxFontManager::FindOrCreateHandle`'s first argument prints
  `$TitleFont`, the address is right, and no argument about it is needed.

## 2. Build

```
cmd /c "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cmake --build D:\Christmas\github\dismod\build\debug
```

`build\debug` is already configured (Ninja, MSVC 14.44 `Hostx64\x86`, DirectX SDK June 2010). A full build
is 42 objects and about 90 s; an incremental one is seconds. `vcvarsall.bat` prints
`'vswhere.exe' is not recognized` on this machine and that is harmless.

There is a ready-made wrapper at `resources/tools/dismod_build.cmd` (below).

## 3. Deploy and run

```
copy D:\Christmas\github\dismod\build\debug\dismod.dll ^
     "C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32\dinput8.dll"
```

A pre-FB backup of whatever was there is at `dinput8.dll.preFB.bak` in the same directory (the user's own
older copy is `_dinput8.dll`, which is a *different* build — do not assume the two are interchangeable).

Then launch the game **directly**, not through Steam:

```powershell
$env:DISMOD_UIPROBE   = "1"
$env:DISMOD_PROBE_DIR = "D:\RecompileDishonored\Recompile\build\agent<X>\retail"
$env:DISMOD_LOG       = "D:\RecompileDishonored\Recompile\build\agent<X>\dismod.log"
Start-Process "C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32\Dishonored.exe" `
  -WorkingDirectory "C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32" `
  -ArgumentList "-windowed","-ResX=1600","-ResY=900","-nostartupmovies"
```

Steam must be running (it is, as a background process). It takes about 40–50 s to reach the main menu.

**Traps found the hard way:**

* `Start-Process` from Git Bash (`start ...`) is refused with *Access is denied*; use PowerShell.
* `Dishonored.exe` also opens a **console window** of its own (class `ConsoleWindowClass`), which is a
  second top-level window of the same process. `Get-Process ... | MainWindowHandle` happens to return the
  game window, but if you enumerate windows, filter on class `LaunchUnrealUWindowsClient`.
* Retail's own `Launch.log` (under `Documents\My Games\Dishonored\DishonoredGame\Logs`) is **not written**
  by a normal shipping run — do not look there for a crash reason. dismod's own log is all you get.

## 4. Where the output goes

`include/logger.h` used to hardcode `C:\Users\User\Desktop\asd\dismod.log`, which does not exist here.
FB changed it (the only change to an existing dismod file other than one `#include` and one call in
`library.cpp`, plus one line in `CMakeLists.txt`) to:

1. `%DISMOD_LOG%` — full path to the log file, or
2. `%DISMOD_LOG_DIR%\dismod.log`, or
3. `D:\RecompileDishonored\Recompile\build\agentFB\dismod.log` (the default).

The directory is created if missing and each run appends a `===== dismod session ... =====` banner, so one
file can hold several runs. Nothing else about the logger changed.

The probe writes its captures to `%DISMOD_PROBE_DIR%` (default
`D:\RecompileDishonored\Recompile\build\agentFB\retail`):

| File | What |
|---|---|
| `fonts.log` | every font request, what it resolved to and by which route; every `GFxTextDocView::FindFont` per text field |
| `displaylist.log` | `_x _y _xscale _yscale _width _height _rotation _alpha _visible _name _target _currentframe _totalframes _url text htmlText`, plus `getDepth()`, for every path in `probe_paths.txt`, for every live movie root |
| `ue.log` | every `Camera`, `CameraActor`, `PlayerController`, `LocalPlayer`, `PostProcessChain` (with its `UArkPpNode` graph) and `GFxMoviePlayer`, dumped through UE3 reflection |
| `camera.log` | the `cam` / `camwatch` output: the live cameras only, with the fields that decide the rendered view |

## 5. Driving the probe

There are **no hotkeys** — the probe watches a command file, because window focus on this machine is not
reliable (see §7). Write commands into `%DISMOD_PROBE_DIR%\probe_cmd.txt`; the watcher notices any change
within 200 ms and the commands run on the game thread inside `GFxMovieRoot::Advance`.

| Command | Effect |
|---|---|
| `label <text>` | writes `##### <text>` into all three capture files — put one before each screen |
| `dump` | queries every path in `probe_paths.txt` against every live movie root |
| `paths` | re-reads `probe_paths.txt` without restarting the game |
| `ue` | dumps cameras, controllers and post-process graphs |
| `fonts` | writes how many distinct font resolutions have been seen |
| `cam` | dumps every **live** camera, player controller and `CameraActor` into `camera.log`, with `CameraCache`, `LastFrameCameraCache`, `ViewTarget`, the FOV fields and the transform |
| `camwatch` | repeats that dump every 120 frames, twelve times, so an advancing `CameraCache.TimeStamp` proves the values are live rather than class defaults |

Lines beginning `#` are ignored. **Write the file from Git Bash with `printf > file`, in a retry loop** —
the watcher holds a read handle for a moment every 200 ms and PowerShell's `Set-Content` fails with
*used by another process* about half the time:

```bash
R=D:/RecompileDishonored/Recompile/build/agentFB/retail
for i in 1 2 3 4 5 6; do printf 'label 02-mainmenu\ndump\n' > $R/probe_cmd.txt 2>/dev/null && break; sleep 0.3; done
```

`probe_paths.txt` is one ActionScript path per line (`_root.optionsMenu_mc`, …). A path that does not exist
simply produces no line, so an over-long candidate list is free — that is how the menu's clip names were
found.

## 6. Selecting the hooks

`DISMOD_UIPROBE_HOOKS` is a comma list of short keys; unset means all of them.

```
advance  moviedef  fontlib  fontmap  provider  addfonts  foch  docview
```

`advance` alone gives the display-list and UE3 dumps with no font tracing at all, which is the
configuration to start from when bisecting a probe that destabilises the game.

## 7. The foreground problem, and how to get round it

Two **Windows Security Alert** dialogs (`rundll32`, raised by other agents' `dishonoredgame_*.exe`) held
the foreground for the whole of wave 17, exactly as `STATUS.md` records for wave 16. With them up:

* `SetForegroundWindow` on the game **fails** if you attach only to the game's input thread;
* it **succeeds** if you also attach to the *current foreground window's* thread first. This is the fix,
  it does not answer or dismiss the dialog, and it is what `resources/tools/retail_input.ps1` does:

  ```
  AttachThreadInput(me, threadOf(GetForegroundWindow()), true)
  AttachThreadInput(me, threadOf(gameWindow),           true)
  BringWindowToTop / SetForegroundWindow / SetActiveWindow / SetFocus
  ```

* keyboard sent with `SendInput` after that still did **not** steer the retail menu, and `PostMessage`
  of `WM_KEYDOWN`/`WM_KEYUP` did not either; a `SendInput` mouse hover did not move the retail cursor
  sprite. Measured, not assumed: the display list was re-dumped after each attempt and nothing moved.
  So **retail can be launched, probed and screenshotted, but not yet navigated** while those dialogs are
  up. Everything in `build/agentFB/retail/` was captured on the screen the game reaches by itself.

* the same `SendInput` keyboard **does** reach *our* build: the GFx input census counted the presses
  (`6 key downs, 7 key ups, 13 events HE_Handled, 13 listener calls`). It still did not move the menu's
  selection, and `0 mouse events` reached the movie, with `focus NULL | canFocus 0 | canInput 0` on the
  GFx input probe. That is a defect in our menu's input, not in the driver, and the census is the
  evidence to start from.

* **Never click `Allow access` or `Cancel`** on those dialogs. That is the user's decision about their
  firewall.

**Screenshots**: PowerShell is not DPI-aware by default, and the game window is on a secondary monitor at
x≈3000 on this machine. Without `SetProcessDPIAware()` the capture lands hundreds of pixels away and looks
like a completely different screen — that cost an hour. `resources/tools/retail_shot.ps1` calls it.

## 8. What the probe hooks, and why each address is what it says

All RVAs; add `0x400000` for the VA. Every one was resolved by hand with IDA on
`retail2013_named.i64` (see `resources/docs/agents/agentFB.md` for the decompiles).

| RVA | Symbol | Why it is hooked |
|---|---|---|
| `0xa08280` | `GFxMovieRoot::Advance(float,unsigned)` | per-frame, game thread: the command channel and the movie-root census |
| `0xa47d00` | `GFxFontManager::FindOrCreateHandle(char const*,unsigned,GFxFontResource**,FontSearchPathInfo*)` | the one call that actually resolves a font name |
| `0xa17090` | `GFxMovieDefImpl::GetFontResource` | route 1 of the search |
| `0x9bd940` | `GFxFontMap::GetFontMapping` | route 2 |
| `0x9be3e0` | `GFxFontLib::FindFont` | route 3 |
| `0xa4be30` | `GFxFontResource::CreateFontResource` | route 4 (the font provider) |
| `0x9be9b0` | `GFxFontLib::AddFontsFrom` | which movies contribute fonts to the library |
| `0xa92ca0` | `GFxTextDocView::FindFont(FindFontInfo*,bool)` | one line per text field |

and it *calls* (never hooks):

| RVA | Symbol |
|---|---|
| `0x9bfc60` | `GFxMovieView::GetVariableStringW(char const*)` — any AS property as a string |
| `0xa04ce0` | `GFxMovieRoot::Invoke(char const*, GFxValue*, char const*, ...)` — `getDepth()` |
| `0x1023630` | `UObject::GObjObjects` (from `globals_2013.csv`) |
| `0x1035674` | `FName::Names` |

Retail struct offsets the probe depends on, each read out of a decompile rather than guessed:

* `GFxFontHandle + 0x1C` → `GFxFontResource*`, `GFxFontResource + 0x0C` → `GFxFont*`,
  `GFxFont` **vtable slot 15 (offset 60)** → `const char* GetName()`  (all three from `??0GFxFontHandle`
  at `0x9ea970`, which calls exactly that chain).
* `GFxTextDocView::FindFontInfo + 0x04` → `GFxTextFormat*`, `+ 0x0C` → the resolved `GFxFontHandle*`.
* `GFxTextFormat + 0x08` → the font-list `GString`, valid only when bit 2 of the `WORD` at `+ 0x2A` is
  set; `+ 0x28` bit 0 = bold, bit 1 = italic (`GFxTextFormat::GetFontList` at `0xa87510`).
* a `GString` is one pointer; the characters are at `(ptr & ~3) + 8`.
* `GFxValue` is `{ void* pObjectInterface; int Type; union { double; ... } }`, `Type == 3` is a number —
  **confirmed at runtime**, not assumed: `displaylist.log` prints the raw 24 bytes next to the decode.

## 9. Helper scripts

Copied into this tree so a later wave does not have to write them again:

| Script | What |
|---|---|
| `resources/tools/dismod_build.cmd` | vcvarsall x86 + `cmake --build` of `dismod\build\debug` |
| `resources/tools/dismod_run.ps1` | sets the three env vars, resets the capture files, launches retail, reports alive/dead |
| `resources/tools/retail_input.ps1` | foreground grab (§7), `SendInput` hover/click/keys, window screenshot |
| `resources/tools/retail_shot.ps1` | DPI-aware screenshot of a named process's game window |

## 9a. Running *our* build, not retail

Not part of dismod, but it cost two runs here and belongs next to it. A worktree build made by
`resources/build-release.cmd` does **not** define `DISHONORED_PLAY_DEFAULTS`, so it does not add the
switches the user's own build adds. Launched without them it loads `DishonoredGameFull_P` and sits
there with `0 movie(s) open` and no menu. The set that opens the menu is

```
-gfxuimenu -gfxuicensus -nomovie -nosteam -skipnativepkgs=OnlineSubsystemPC -startmapopen
-windowed -ResX=1600 -ResY=900
```

and `-gfxuimenu=<Package>.<Movie>` opens one movie on its own, which is how the options screen was
captured without navigating to it: `-gfxuimenu=UI_OptionsMenu.OptionsMenu`. The resident movies are
`UI_Global.Global`, `UI_HUDFX.HUDFX`, `UI_Note.Note`, `Common_assets.lib`, `UI_LoadGame.LoadGame`,
`UI_OptionsMenu.OptionsMenu`, `UI_MainMenu.MainMenu`, `DisFonts.gfxfontlib`, `DisFonts.fonts_efigs`.
**Never `-unattended`**: it suppresses the menu outright.

Screenshots of our build should go through `-apshottime=<seconds>`, which captures inside the process
on the game thread: four agents' windows and two security dialogs were on this desktop at once and a
screen capture picked up another agent's game more than once. The bitmaps land in
`<retail>\DishonoredGame\Screenshots\Win32Console\apshottime*.bmp`, a directory every agent shares,
and every one of them is exactly 4,320,054 bytes at 1600x900 — **so claim them by modification time,
not by size**. Claiming by size picked up agent FE's screenshot here.

## 10. Known limits

* The probe is **read-only**. It never writes a game value; `dump`/`ue` only read.
* The `ue` dump walks all ~70 000 `GObjObjects` and writes ~11 000 lines. It stalls the game thread
  while it runs, and on its **second** invocation in one session it froze the game outright, 331 lines
  in. Treat it as one-shot; a run that stops printing `uiprobe: alive, N advances` has hung rather
  than finished. `cam` walks the same array but prints only the handful of live camera objects, so it
  is cheap enough to repeat — that is what `camwatch` does.
* **A `Default__` object's `CameraCache` reads `TimeStamp=0`, a zero location and `FOV=90`.** That is
  the class default, not a measurement. The live one has a `TimeStamp` that advances between samples.
  Reading the wrong one is the easiest mistake in `ue.log`, and it is why `cam` exists: it only prints
  objects with a live `PCOwner` / `PlayerCamera`.
* `_root.gamma_mc` does not exist until the brightness screen is opened, and the difficulty screen's
  `_root.newGame_mc` is present but `_visible=false` until then — see §7 for why those screens were not
  reached.
* The struct dumper recurses three levels, which is enough for `CameraCache.POV.FOV` and for a
  post-process node's `m_UberParameters`; anything deeper prints `{...}`.
* The font hooks were stable for the whole run once the `GFxTextDocView::FindFont` line stopped going
  through an ever-growing dedupe vector. If retail dies about a minute in, bisect with
  `DISMOD_UIPROBE_HOOKS`.
