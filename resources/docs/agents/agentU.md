# Agent U — Middleware versions, Phase 4 memo, Bink import library (2026-09-27)

Build dir `build\agentU` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON -DDISHONORED_WITH_BINK=ON`);
logs `build\agentU\{configure,build,build_objs}.log`, scanner output `build\agentU\scan_*.txt`,
dumpbin listings `build\agentU\*_exports.txt`, 2012 decompiles `build\agentU\decomp\`, the
import-library experiment `build\agentU\deftest\`. IDA copy `resources/docs/idb/shipping2012_agentU.i64`
(2013 copy not needed: strings and byte patterns were enough). The memo itself is
**`resources/docs/middleware.md`**; this file is the change log and evidence index.

## Version table (evidence per row; full detail in middleware.md section 1)

| Library | Retail | Evidence (file + string/resource) |
|---|---|---|
| Scaleform GFx | 3.3.89 | `Dishonored.exe` rva 0xdf7374 `"3.3.89"` next to `"gfxVersion"` (0xdf7368); 2012 exe rva 0xdaf8e4; PDB libs `libgfx`/`libgfx_ime`, headers `external\gfx\include\gfxplayer.h` |
| Wwise | 2012.1.x, bank v65 | 2012 PDB source root `d:\branches\wwise_v2012.1\` (291 files); `CookedPCConsole\Init.bnk` `BKHD` version 65, every `.pck`'s embedded bank v65; no version string in either exe |
| FaceFX | SDK 1.7.3.1 | `FxSDKGetVersion` = `mov eax,0x6C3` (1731): 2012 rva 0xb14a10 (PDB name), identical bytes at 2013 rva 0xafee60; SDK source compiled from `external\facefx\fxsdk\{src,inc}` |
| PhysX | SDK 2.8.4, DLLs 2.8.4.6 | `PhysXLoader/PhysXCore/PhysXCooking/NxCharacter.dll` VS_VERSIONINFO 2.8.4.6 `*_FC6_GPU`, `PhysXExtensions.dll` 2.8.4.1 `SpecialBuild=Epic Games`; `PhysXCore.dll` paths `physx\PhysXSDK\2.8.4\trunk`; exe `push 0x02080400` before `NxCreatePhysicsSDK` (2013 rva 0x3d57aa/0x3d587b, 2012 0x3f803a); static `libnxdoublebuffered_release` (1,289 fns) in the exe |
| APEX | shipped, not linked | `APEX_*.dll` 1.0.0.0 `APEX_B1`, path `APEXSDK\0.9\feature\UE3_APEX`; exe: no APEX import, no `NxApex`/`physx::apex` symbol or `APEX_*.dll` string; 2012 PDB 0 APEX functions |
| Bink | 1.9p (1.9.16.0, 2009-09-03) | `binkw32.dll` VS_VERSIONINFO (`1.9p`, `RAD Video Tools`, 1994-2009), PE timestamp 0x4A9F8FC2, 72 exports; `binkw32.pdb` in the 2012 tree |
| Steamworks | SDK 1.18/1.19 generation | `steam_api.dll` strings `SteamClient012 SteamUser016 SteamFriends011 SteamUtils005 SteamMatchMaking009 SteamMatchMakingServers002 SteamUserStats010 SteamApps005 SteamNetworking005 SteamRemoteStorage006 SteamHTTP001 SteamScreenshots001 SteamGameServer011 SteamGameServerStats001 SteamContentServer002`; VS_VERSIONINFO 1.30.50.46 |
| libcurl | DLL 7.77.0 (2021) | `libcurl.dll` VS_VERSIONINFO + `libcurl/7.77.0`; 7 imports, 2013 exe only |
| zlib / libpng / bzip2 | 1.2.3 / 1.2.8 (headers 1.2.34) / 1.0.6 | exe strings (rva 0xdf1478; 0xe04e34 next to the PNG signature, 0xc3dad0 the app-side version string; 0xbbf5dc) |
| ogg/vorbis, wx 2.8, cudart 3.0.9/4.0.12, SteamAPIUpdater | shipped, not imported | VS_VERSIONINFO; `imports_2013.csv` has no entry |

## Decisions (middleware.md sections 2–4)

* **Bink**: import library + reconstructed header now (done, behind `DISHONORED_WITH_BINK`); the
  shipped DLL is the runtime. Retail's Windows audio path is DirectSound
  (`BinkSetSoundSystem(BinkOpenDirectSound,0)` rva 0xe1fa8), not XAudio2: the reference glue's
  Windows branch was switched accordingly.
* **PhysX**: link the shipped 2.8.4.6 DLLs through 2.8.4 SDK headers + `NxdDoubleBuffered`
  (user must obtain); `WITH_APEX=0` permanently (retail does not use APEX). Needed at milestone 5.
* **Scaleform**: matching 3.3.x SDK if the user can source a licensee drop, else GFx-3 API subset on
  Ruffle behind a `GFxUI` adapter; never a decompile rewrite (1.1 MB, no symbols). Gating decision
  for the main-menu half of milestone 4; the Arkane `GFxUI` port can start now either way.
* **Wwise**: stub `AkAudio` now (silent), link Wwise 2012.1 SDK (free download) when installed.
* **FaceFX**: `WITH_FACEFX=0` (assets round-trip as byte blobs) until milestone 6, then a
  decompile-driven rewrite (1,317 named functions) unless a licensee `FxSDK` source drop appears.
* **Steamworks**: SDK 1.18/1.19 from the partner archive; `-nosteam`/`bEnableSteam=false` offline
  path with delay-loaded `steam_api.dll` (Phase 8).
* **libcurl**: removed in Phase 8.

## What changed (files)

| File | Change |
|---|---|
| `resources/docs/middleware.md` | new: the memo (version table with evidence, per-library plan, milestone-4 minimum, blockers) |
| `resources/tools/binaries/scan_versions.py` | new: ASCII/UTF-16 string scanner with per-library patterns + hand-parsed `VS_VERSIONINFO` (no `pefile` on this machine); `--grep RE`, `--all` |
| `cmake/Bink.cmake` | new: `DISHONORED_WITH_BINK` (OFF), `DISHONORED_RETAIL_DIR`; builds `binkw32_stub` (throw-away `build/<dir>/bink/stub/binkw32.dll`) for its import library, exposes `Dishonored::bink` |
| `cmake/Dependencies.cmake` | `include(cmake/Bink.cmake)` |
| `cmake/DishonoredDefines.cmake` | `dishonored_apply_defines`: `USE_BINK_CODEC=1` + `Dishonored::bink` on every target when the option is ON (the switch changes `UCodecMovieBink`'s declaration in `UnCodecs.h`, so it must be uniform); OFF leaves `UnBuild.h`'s default 0 |
| `source/Development/Src/Engine/Bink/Src/bink.h` | new: reconstructed Bink 1.9 header — RAD types/linkage, `BINK` (928 B), `BINKIO`, `BINKSND`, `BINKFRAMEBUFFERS`, `BINKREALTIME`, `BINKRECT`, `BINKPLANE`, `BINKFRAMEPLANESET`, `BUNDLEPOINTERS` from the 2012 PDB (`types.json`) with `static_assert`s; `BINKSUMMARY` and unverified flags marked; the 20 imported prototypes; flag values cross-checked against the exe's call sites |
| `source/Development/Src/Engine/Bink/Src/binkw32_stub.cpp` | new: 20 `extern "C" __declspec(dllexport) __stdcall` stubs = the import library source; full 72-export table with ordinals in the header comment |
| `source/Development/Src/Engine/Bink/Src/FullScreenMovieBink.inl` | Windows path: no `XAudio2Device.h`; `FBinkMovieAudio` ctor calls `BinkSetSoundSystem(BinkOpenDirectSound, 0)`; new `HandleDirectSoundVolumes` ported from the 2012 decompile of `FBinkMovieAudio::SetAudioChannels` (rva 0xd95f0); `HandleXAudio2Volumes` now `#if XBOX`; `FIOSystem::LoadData` call passes `AIORT_Bink` (Arkane's extra argument) — each edit tagged `DISHONORED(retail|port)` |

Not committed, generated: `build/agentU/bink/binkw32.lib` + `bink/stub/binkw32.dll` (git-ignored).

## The `.def` route does not work for this DLL (why there is a stub instead)

`binkw32.dll` exports the *decorated* stdcall names (`_BinkOpen@8`) and the retail exe imports
them by that name. With `lib /def /machine:x86` (test in `build/agentU/deftest/`, `dumpbin
/imports` on a linked test exe):

| `.def` entry | link | exe imports |
|---|---|---|
| `BinkClose@4` | ok | `BinkClose@4` — wrong name, `GetProcAddress` fails at load |
| `_BinkClose@4` | fails: lib emits `__BinkClose@4`, `__imp__BinkClose@4` unresolved | — |
| `BinkClose@4=_BinkClose@4` | ok | `BinkClose@4` — same as the first |
| `BinkClose@4 @16 NONAME` | ok | ordinal 16 — works, but pins this exact DLL build |
| stub DLL, `extern "C" __declspec(dllexport) __stdcall` | ok | `_BinkClose@4` — identical to retail |

The stub is a normal CMake `SHARED` target (`OUTPUT_NAME binkw32`, output under `build/<dir>/bink/
stub`, never in `Binaries/Win32`); only its `.lib` is consumed.

## Bink link check

* `binkw32_stub` builds and links (`[7/11] Linking CXX shared library bink\stub\binkw32.dll`).
* `FullScreenMovie.cpp` / `UnCodecs.cpp` (the two Engine units that include the Bink glue) compile
  with `USE_BINK_CODEC=1` as far as the Bink code is concerned: after the `LoadData` fix the only
  remaining errors in `build_objs.log` are foreign — `UnClient.h(683)`
  `IArkSettingsListenerInterface` (agent S, gone on the next try) and `DishonoredLayouts.h(419)`
  `FOnlinePlayerScore` size (agent S's Engine convergence in flight, `EngineClasses.h` changing
  every few minutes). The full link was therefore done with `-DDISHONORED_LAYOUT_CHECKS=OFF`
  (`build_nolayout.cmd`), which only disables the `static_assert`s; see "Build outcome".
* The 20 `__imp__Bink*` references now resolve against `bink/binkw32.lib`; the exe's import table
  will list `binkw32.dll` with the decorated names (verify with `dumpbin /imports` after the link).

## Blockers the user must resolve

1. PhysX 2.8.4 SDK (headers + `NxdDoubleBuffered` + `NxCharacter`), outside the repo — milestone 5.
2. Wwise 2012.1.x SDK from Audiokinetic's Launcher (free, account) — audio.
3. Steamworks SDK 1.18 or 1.19 from the partner download archive — Phase 8 offline path.
4. Scaleform GFx 3.3.x: decide SDK-drop vs Ruffle-adapter — gates the main menu.
5. (optional) FaceFX 1.7.3 SDK source.

## Follow-ups outside my files

* Launch: `appInitFullScreenMoviePlayer` should create `FDisFullScreenMovieBink` (2012 rva 0x57dad0,
  `[DisFullScreenMovieBink]` ini) instead of the reference `FFullScreenMovieBink`; the
  `UArkBinkOverlayManager`/`UDisBinkOverlayManager` classes (T's registrants) and
  `Engine/Shaders/BinkShaders.usf` are needed to actually play the startup movies (milestone 4).
* `FullScreenMovieBink.inl` still references `GEngine->Client->GetAudioDevice()` for the volume
  levels; with the `AkAudio` stub that returns NULL and the defaults apply (fine for bring-up).
* `cmake/ReferenceExternals.cmake`: add `dishonored_reference_sdk(PhysX284 ...)` once the SDK path
  exists; `WITH_NOVODEX=1` then re-enables 110 Engine files (compile-fix pass expected).
* `build_and_smoke.py`: `--with-bink` is unnecessary (the retail DLL is already in the staged tree);
  a `-nomovie` run stays the default until milestone 4.
* The scanner could feed `resources/docs/binaries.md` (version column) — not done.

## Build outcome

`build_nolayout.cmd` (Bink ON, `DISHONORED_LAYOUT_CHECKS=OFF` because agent S's Engine headers were
mid-change and tripped `DishonoredLayouts.h(419)` in every Engine unit): **640 compile units,
`[650/651] Linking CXX executable Binaries\Win32\DishonoredGame.exe`, `build exit 0`**
(`build/agentU/build.log`, exe 54.7 MB, 06:02). `DishonoredGame.map` confirms the link check:

* the 20 import thunks `_BinkOpen@8 ... _BinkGetRects@8` and their `__imp__Bink*` slots all
  resolve to `binkw32:binkw32.dll` — the exe will load the retail `binkw32.dll` by the same
  decorated names as `Dishonored.exe` (`imports_2013.csv`);
* the Bink glue is really compiled in: `FBinkMovieAudio::HandleDirectSoundVolumes`
  (`Engine:FullScreenMovie.cpp.obj`), `_Draw_Bink_textures` (`Engine:UnCodecs.cpp.obj`),
  794 `Bink` symbols in total.

With the option OFF (`build/agentU_off`, default configure) `build.ninja` contains no
`USE_BINK_CODEC`/`binkw32` at all, so the default build is unchanged. Re-run with layout checks on
once S's headers settle: `cmake -B build/agentU -DDISHONORED_LAYOUT_CHECKS=ON && ninja -C build/agentU DishonoredGame`.
Running the movie player is milestone 4 (needs the `FDisFullScreenMovieBink` port and `BinkShaders.usf`).
