# Dishonored Recompilation

A functional (non-matching) rebuild of Dishonored's Win32 executable from the UE3 source tree, targeting the
**retail 2013 build** (`Dishonored.exe`, engine 9411, DLC05–07). It runs against your own copy of the game's content:
build it, drop the executable into the retail `Binaries\Win32` folder, and it comes up on the game's own main menu.

[![The first cutscene playing in the recompiled build](https://img.youtube.com/vi/OjsDzG64K9w/maxresdefault.jpg)](https://www.youtube.com/watch?v=OjsDzG64K9w)

*Video: the opening boat ride running in the recompiled build.*

## What works

| | |
|---|---|
| Engine, renderer, script VM | up; the first mission renders under D3D9 and plays |
| Main menu | draws, navigates by mouse and keyboard, UI sounds, starts the game |
| Opening of mission 1 | the boat ride, the Emily greeting with its hide-and-seek choice, the Empress scene with Emily running in, markers, the interaction prompt and highlight glow |
| Player | walk, sprint, crouch, jump with the landing spring, swim, mantle; retail's speeds and attributes |
| NPCs | path following with turn-ahead and avoidance, scene placement through root motion and FaceTo, conversations |
| Audio | real: Wwise 2012.1 reimplemented from the decompile (music, effects, voice), FaceFX lip-sync |
| Save/load | all 51 retail saves load; 86.7 % of `Dishonored0.sav`'s object stream restores |
| Full campaign | not yet |

Every merge is gated on a clean checkout by a 47-check regression harness (`resources/tools/run_regression.py`): build,
layout `static_assert`s against retail's offsets, a null-RHI smoke, a D3D9 render census, a scripted input run, a New
Game from the menu and an Options sweep. It reads 47 of 47 on HEAD. `resources/docs/STATUS.md` has what is still wrong.

## Build it and run it in your retail install

What you need:

- Windows 10/11, 64-bit.
- **Visual Studio 2022 Community** with the "Desktop development with C++" workload, including the **C++ CMake tools
  for Windows** component (that gives you CMake and Ninja). The scripts call `VsDevCmd.bat` for an x86 toolchain.
- **Python 3.10+** on `PATH` (the build and staging scripts are Python; no extra packages are needed to build).
- **Git** (the first configure fetches zlib, libpng, nvapi and lzokay into `external/`, so it needs internet once).
- Your own **retail Dishonored** (the 2013 Steam/GOG build with the DLC). Nothing from the game ships in this repo.
- The **UE3 reference tree**: a checkout of
  [CodeRedModding/UnrealEngine3](https://github.com/CodeRedModding/UnrealEngine3). The build reads engine sources it
  does not override from there.

Step by step:

1. **Lay the folders out side by side.** The scripts default to the retail install and the reference tree sitting
   next to this repo:

   ```
   D:\RecompileDishonored\
       Recompile\                this repository
       Dishonored_Latest2026\    your retail install (contains Binaries\Win32\Dishonored.exe, DishonoredGame\, Engine\)
       UnrealEngine3\            the CodeRedModding/UnrealEngine3 checkout
   ```

   Any drive works; keep the three names and the layout. If you want other paths, change the two values in
   `resources\build-play.cmd` (`DISHONORED_REFERENCE_DIR`) and `cmake\Bink.cmake` (`DISHONORED_RETAIL_DIR`), and
   pass `--retail <path>` to `stage_retail.py`.

2. **Point the retail folder at a copy, not your only install.** Copy the whole game folder (the one that holds
   `Binaries`, `DishonoredGame`, `Engine`) to `Dishonored_Latest2026`. The build writes its executable, logs and
   screenshots into that tree.

3. **Open a plain command prompt** (not a Developer prompt, the script loads the toolchain itself) in the repo root
   and run:

   ```
   resources\build-play.cmd
   ```

   That configures a Release x86 build in `build\play` with every module on, builds `DishonoredGame.exe` (around
   380 compile steps; 10 to 30 minutes depending on the machine), and stages the result into the retail tree as:

   ```
   Dishonored_Latest2026\Binaries\Win32\DishonoredGame-Win64-Shipping.exe
   ```

   `resources\build-play.cmd shipping` builds the same with `FINAL_RELEASE` / `SHIPPING_PC_GAME` / `NO_LOGGING` on.

4. **Run it.** Double-click `DishonoredGame-Win64-Shipping.exe` in `Binaries\Win32`, or launch it from a shortcut.
   No command line is needed: the build appends `-nosteam -skipnativepkgs=OnlineSubsystemPC -nomovie -windowed
   -ResX=1600 -ResY=900` itself when they are absent, so it does not need Steam running and comes up windowed on
   the main menu. From there: **Space** past the start screen, NEW GAME, pick a difficulty, the brightness screen,
   then **YES**.

5. **Rebuild after pulling.** Run `resources\build-play.cmd` again; it is incremental and re-stages the executable
   over the old one. Close the game first, the copy fails while the exe is running.

Useful switches (anything you pass wins over the defaults):

```
DishonoredGame-Win64-Shipping.exe -ResX=2560 -ResY=1440
DishonoredGame-Win64-Shipping.exe -startmap=L_Pub_Day_P
DishonoredGame-Win64-Shipping.exe -LOG=mine.log -forcelogflush
```

Logs land in `Dishonored_Latest2026\DishonoredGame\Logs\Launch.log`. The in-game console opens on **Tilde** (the key
left of 1); **Tab** opens the one-line bar.

**The name is a name, not a description**: the executable is 32-bit like the retail game. PhysX 2.8.4, Bink, Wwise
2012 and Scaleform 3.3 ship only as 32-bit DLLs in the retail tree, and the layout `static_assert`s are written
against retail's 32-bit offsets under `/Zp4`. The retail `Dishonored.exe` is left untouched beside ours.

### Other builds

| | |
|---|---|
| `resources\build-game.cmd [target]` | the development build, every option on, into `build\game` |
| `resources\build-release.cmd [target]` | the Release build the regression harness uses |
| `python resources\tools\run_regression.py --build-dir build/<dir>` | the 47-check regression gate |
| `python resources\tools\game_input.py launch --exe <exe>` | drive a running build (`hover`, `click`, `key`, `shot`) without touching your own mouse or focus |

Builds from the scripts go through `resources/tools/build_queued.py` (one at a time, half the cores).

## How it is built

The retail 2013 binary is the contract: its layouts, serialization and behaviour. The symbolized 2012 QA build only
supplies names and decompiles; its structs differ from 2013 in places and are never the final word. The base is the
UE3 build 10897 tree (a very close engine build to Dishonored's 9014/9411, see `resources/docs/engine_reference.md`).
Game binaries, PDBs, content, IDA databases and the reference tree stay outside the repo.

Where to read: [resources/docs/STATUS.md](resources/docs/STATUS.md) first (what works, what does not, the pitfalls),
[PLAN.md](PLAN.md) for the strategy, `resources/docs/PHASE1.md`–`PHASE24.md` for the per-wave trackers,
`resources/docs/agents/` for every package's own report and measurements, `resources/docs/symbols/` and
`resources/docs/types/` for the symbol exports, `resources/tools/` for the scripts that produce them.

## References and credits

- [CodeRedModding/UnrealEngine3](https://github.com/CodeRedModding/UnrealEngine3) — the UE3 build 10897 tree this is
  rebuilt from. Without it there is no project.
- [CodeRed-Generator](https://github.com/CodeRedModding/CodeRed-Generator) — produced the retail SDK dump that gives
  the runtime offset of every reflected member, which the layout `static_assert`s are written against
  (`resources/docs/sdk_dump.md`).
- [IDA Pro and the Hex-Rays decompiler](https://hex-rays.com/), [Diaphora](https://github.com/joxeankoret/diaphora)
  (diffing the 2012 and 2013 builds), [dismod](https://github.com/ectrc/dismod) (hooking the running retail game for
  ground truth, `resources/docs/dismod_harness.md`).
- Third-party code the build fetches: [zlib](https://github.com/madler/zlib) ·
  [libpng](https://github.com/pnggroup/libpng) · [nvapi](https://github.com/NVIDIA/nvapi) ·
  [lzokay](https://github.com/jackoalan/lzokay).
- Middleware the game links: PhysX 2.8.4, Scaleform GFx 3.3, Wwise 2012.1, Bink, FaceFX, Steamworks and libcurl.
  **No vendor SDK is downloaded and nothing is redistributed**: the bindings in `source/Development/Src/External/`
  were written from the DLLs that ship with the game and their exported symbols; Wwise and FaceFX are reimplemented
  (`resources/docs/middleware.md`).

## Licence

**[GNU Affero General Public License v3.0](LICENSE)**. If you build on this, your work has to be open source too,
under the same licence, including over a network (AGPL section 13). Use it, change it, ship it, but ship the source
with it.

The licence covers this repository's code only. Dishonored itself is © Bethesda Softworks / Arkane Studios; this is an
independent reimplementation for study, it ships no game code, assets, binaries or PDBs, and it needs your own legally
obtained copy of the game to run.

All generated with Claude Code.
