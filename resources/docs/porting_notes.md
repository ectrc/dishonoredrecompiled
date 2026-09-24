# Porting notes — reference UE3 10897 → MSVC 2022 / C++17

Policy (PLAN.md §3, Phase 2 P2.4): mechanical fixes only, no behavior changes. Every non-trivial
edit to a file copied from the reference carries a `// DISHONORED: <why>` comment so
`import_reference.py --diff <Module>` stays readable. Layout changes proven by the PDB use
`// DISHONORED(layout): <PDB member / size>`.

## Build environment decisions

| Decision | Value | Source |
|---|---|---|
| Game slot | `GAMENAME=DISHONOREDGAME`, `DISHONOREDGAME=9` | `Launch/Inc/LaunchGames.h` assigns 2–8 to Epic's games |
| Windows target | `_WIN32_WINNT=0x0502`, `WINVER=0x0502` | `UE3BuildWin32.cs`; raised only if the Windows 10 SDK refuses it |
| Include model | every `source/Development/Src/*/Inc` is on every module's include path | UnrealBuildTool adds all module Inc dirs globally; Core's `UnVcWin32.h` includes WinDrv's `PreWindowsApi.h`, `UnFile.h` includes `../../Engine/Inc/UnConsoleTools.h` |
| Relative includes | kept as in the reference (`../../Engine/Inc/...`, `../../Launch/Resources/...`) | they resolve because the tree mirrors `Development/Src/<Module>` |
| zlib | FetchContent v1.3.1 (`cmake/Dependencies.cmake`), `UnMisc.cpp` includes `<zlib.h>` | reference used `Development/External/zlib` which the clone lacks |
| LZO | `WITH_LZO=0` for now | LZOPro not available; cooked packages use zlib only (`package_summary.md`) |
| Excluded units | `Core/Src/UnitTest.cpp` | test harness |
| Launch | reference `Launch.cpp`/`LaunchEngineLoop.cpp` imported but not compiled; `DishonoredLaunchStub.cpp` builds the exe until milestone 1 | |

## Compile error categories (Core)

Filled in during P2.4 from `core_build*.log`. One row per category: count at first sight, fix
applied, files touched.

| Category | Count | Fix | Notes |
|---|---:|---|---|
| (pending first full compile) | | | |
