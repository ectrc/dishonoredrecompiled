# Agent Y report: first frame (window, D3D9 device, cooked shaders, clear/present) and the Bink startup movies (2026-09-25)

Build dir `build\agentY` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`, all four module options, `-DDISHONORED_WITH_BINK=ON`),
configured from the snapshot worktree `build\agentY_wt` (HEAD `da24f65` + my files, synced by `build\agentY_sync.py`;
`build\agentY_configure.cmd`, `build\agentY_build.cmd`). FetchContent sources come from the main tree's `external\*-src`.
IDA copies `resources\docs\idb\shipping2012_agentY.i64`, `retail2013_agentY.i64` (headless: `decompile_funcs.py`, plus my
`build\agentY\disasm_funcs.py` and `vtbl_slots.py`). Decompiles in `build\agentY\decomp12*`, `decomp13`. Logs of the runs
quoted below in `build\agentY\logs\`. **Every number below comes from the `build\agentY` build (retail 2013 exe = target, 2012 =
readable decompile).** No commits, nothing staged.

## Result

| Step | State |
|---|---|
| 1. D3D9 device, retail settings | **done.** `FWindowsViewport` → `UpdateViewportRHI` → `FD3D9Viewport` → `UpdateD3DDeviceFromViewports` creates the device at 1280x720 windowed; behaviour flags now retail (`PUREDEVICE` added) |
| 2. Cooked global shaders on the device | **done for everything this build declares:** `device InitRHI pass: 62 shader objects created, 0 rejected`. Cache inventory: 263 records, **62 load**, 58 layouts differ, 143 types undeclared (Arkane/GFx); 144 reference-only types have no cooked record. `resources\docs\renderer.md` = table + work list |
| 3. One presented frame | **done:** `DISHONORED(bringup): presented frame (1280x720, windowed)`; back buffer dumped with `-firstframe=` (clear + tile + canvas text in the cooked `EngineFonts.SmallFont`) |
| 4. Bink startup movies | **done:** `FDisFullScreenMovieBink` ported (2013 behaviour), all 8 `StartupMovies` play in the window (Black_266ms → ZenimaxLegal → ZenimaxLegalFR → LogoBethesda → LogoArkane → UE3_logo → Legal → Loading) through the cooked Bink shaders |
| Accept: 10 s without device-lost/assert | **not met, not a D3D9 fault:** after the first frame the game streams `Dishonored_MainMenu` and dies in content deserialization (`UNavigationMeshBase::Serialize`, `StaticMeshComponent` serial size), agent AA/Z territory; with movies the run lasts ~48 s |

Screenshots (back-buffer dumps, 1280x720, converted to PNG):

* `build\agentY\shots\firstframe_final.png`: first presented frame, `-nomovie`: RHIClear (dark blue), an untextured canvas tile
  (orange, FSimpleElement shaders) and two lines of canvas text (cooked font texture).
* `build\agentY\shots\movie\frame_030.png` (ZenimaxLegal), `frame_240.png` (ZenimaxLegalFR), `frame_480.png` (LogoArkane):
  Bink movie frames presented by the movie player (present #30, #240, #480 of `-dumpframes=`).

(A desktop screen-capture helper, `build\agentY\shot.ps1`, captured whatever was on top of the screen, not the game window;
those captures were deleted and are not evidence. Use the back-buffer dumps.)

## Exact commands

Build: `build\agentY_build.cmd DishonoredGame <log>` (syncs my files into the worktree, then `cmake --build`).
Runs use my files plus **local-only test bridges in the worktree** (see "Local-only bridges"); none of them is in the shared tree.

```
rem first frame, smoke tool, default (retail) ini: both expects ok, milestone reached
python resources\tools\build_and_smoke.py --build-dir build\agentY --no-build --exe-name DishonoredGame_Y.exe ^
  --log-name agentY.log --ini-dir build\agentY\config --rhi d3d9 --timeout 90 --milestone "Log: Initializing Engine..." ^
  --expect "DISHONORED(bringup): presented frame" ^
  --expect "DISHONORED(bringup): device InitRHI pass: 62 shader objects created, 0 rejected" ^
  --extra-args "-allowunboundnatives -skipnativepkgs=OnlineSubsystemPC -windowed -ResX=1280 -ResY=720 -nomovie -agentYnomatshaders -agentYnoplayer -agentYserialsize -agentYnocommitevents -firstframe=D:/RecompileDishonored/Recompile/build/agentY/shots/smoke_firstframe.bmp"

rem movies (private ini dir, test-only [Engine.Engine] keys, see below)
python build\agentY\run.py --timeout 60 -- -allowunboundnatives -skipnativepkgs=OnlineSubsystemPC -windowed -ResX=1280 -ResY=720 ^
  -agentYnomatshaders -agentYnoplayer -agentYserialsize -agentYnocommitevents ^
  -firstframe=D:/RecompileDishonored/Recompile/build/agentY/shots/movie/first.bmp -dumpframes=D:/RecompileDishonored/Recompile/build/agentY/shots/movie/frame
```

`build\agentY\run.py` = the smoke tool's isolation switches (`-LOG=agentY.log`, `-ENGINEINI=` … in `build\agentY\config_private`)
without deleting the ini dir, `--patch-ini` writes the **test-only private ini** keys `[Engine.Engine] GameEngine=Engine.GameEngine`,
`GameViewportClientClassName=Engine.GameViewportClient`, `LocalPlayerClassName=Engine.LocalPlayer` (never committed; both the
private ini and the retail `DishonoredGame.DishonoredEngine` ini reach the first frame now). For the movie log I also commented
`Suppress=DevMovie` in that private ini. `-firstframe=` needs forward slashes (UE3 `Parse` drops backslashes).

Timeline of the default-ini run (`build\agentY\logs\run_firstframe.log`): `Initializing Engine...` → global shader cache report
→ `device InitRHI pass: 62 shader objects created, 0 rejected` → `LoadMap: DishonoredGameFull_P` → `Bringing World ... up for
play` → `Initializing Engine Completed` → first `Present` → crash while `Dishonored_MainMenu` streams in.
`-nullrhi` (same flags): reaches the tick loop and dies in the post-LoadMap precache view: `FSceneRenderer` asks for
`FDownsampleSceneDepthPixelShader`, whose cooked layout differs (renderer.md item 3); not new (before this wave every global
shader was rejected).

## What changed (my files; every edit tagged `DISHONORED(retail|port|bringup)`)

### Shader system
| File | Change | Evidence |
|---|---|---|
| `Engine/Inc/ShaderManager.h` | `VER_MIN_SHADER` 836 → **786**, `LICENSEE_VER_MIN_SHADER` 0 → **1** | 2013 rva 0xb6fac0 / 0xb722a0 / 0xb7f2c0 (`FShaderType` ctor: `push 312h`, `push 1`). The reference gate (836 > the cooked 801) rejected all 263 global records |
| `Engine/Inc/ShaderCompiler.h` | `EShaderFrequency`: **`SF_Pixel = 1`**, D3D11 stages after it | 2012 PDB enum `SF_Vertex=0, SF_Pixel=1, SF_NumBits=1, SF_NumFrequencies=2`; with the reference numbering every cooked pixel shader was created as a hull shader (`D3D9 Render path does not support Hull shaders!`) |
| `Engine/Src/ShaderCache.cpp` | runtime inventory (`global shader cache: N shaders, L loaded, U undeclared, M mismatches`; each undeclared/mismatching type named, material caches once per type); mismatching records whose shader was never registered are no longer deregistered (assert); `FShaderLoadArchive` validates name/object indices against the linker tables and skips the record instead of aborting (`Bad name index`) | runs quoted in renderer.md |
| `Engine/Src/GlobalShader.cpp` | lists declared global types the cooked map has no shader for | |
| `Engine/Src/ShaderManager.cpp` | `FShader::InitRHI` counts created/rejected RHI shader objects and names the rejected ones | |
| `D3D9Drv/Src/D3D9Shaders.cpp` | a rejected shader is reported with HRESULT and size instead of `VERIFYD3D9RESULT` aborting | |
| `D3D9Drv/Src/D3D9Device.cpp` | `D3DCREATE_PUREDEVICE`; InitRHI pass summary line | 2013 rva 0x5bc1e0 (2012 0x603760): flags `(HWT&L ? 0x40 : 0x20) | 0x112` |
| `Engine/Bink/Src/BinkTexturesRHI.inl` | `FBinkYCrCbAToRGBAPixelShader::Serialize` = tex3 + `FShader::Serialize` | 2012 rva 0x1d43b0 / 2013 0x1be510 (the cooked record carries tex3 only) |

Retail material-shader gates were measured but **not applied** (`MaterialShader.h` unchanged): see renderer.md section 1 and
work-list item 2.

### First frame
| File | Change |
|---|---|
| `D3D9Drv/Src/D3D9Viewport.cpp` | `DISHONORED(bringup): presented frame (WxH, windowed)` at the first successful Present; `-firstframe=<file.bmp>` (back buffer before the first Present, `GetRenderTargetData` → 32-bit BMP, no D3DX); `-dumpframes=<prefix>` (presents 1, 10, 30, 60, 120, 240, 480, 960) |
| `Engine/Src/UnPlayer.cpp` | with `-firstframe` and no player view: skip the post-LoadMap precache view, and at the end of `UGameViewportClient::Draw` flush, `Clear`, one untextured tile and two lines of `DrawShadowedString` (the frame would otherwise be the black "no views" tile, which also covered anything drawn earlier) |

Retail present parameters were already the reference ones (A8R8G8B8, 1 back buffer, `LOCKABLE_BACKBUFFER`, no auto depth, COPY
windowed / DISCARD fullscreen). Retail takes `PresentationInterval` from the fullscreen viewport's `bWantsVSync` and from
`GSystemSettings.bUseVSync` otherwise (identical in windowed runs); the vsync argument Arkane threads through
`CreateViewportFrame` / `FWindowsViewport::Resize` / `UpdateViewportRHI` / `RHICreateViewport` (2012: 5-argument
`RHICreateViewport`, rva 0x1a950) is **not ported** (API change across UnClient/RHI/UnGame, follow-up).

### Bink
| File | Change | Evidence |
|---|---|---|
| `Engine/Inc/FullScreenMovie.h` | `FFullScreenMovieSupport`: virtual `InitAudio` (slot 6) and `GameThreadPlayLoadingMovieAndIntro` (slot 7) | 2012 vtables.csv `FFullScreenMovieSupport{for FTickableObject}` |
| `Engine/Bink/Src/FullScreenMovieBink.h/.inl` | `InitAudio` (`BinkSetSoundSystem(BinkOpenDirectSound,0)`, 2013 rva 0xe1fa0), `OnBinkTick`/`OnBinkRenderFrame` hooks (Tick ends with `OnBinkTick`, RenderFrame calls `OnBinkRenderFrame` before the canvas flush: 2013 rva 0xfde70 / 0xfa030), members protected for the subclass, `FBinkMovieRenderClient` takes `FFullScreenMovieBink*` (2012 rva 0xe0460). **Removed the reference tree's forced first startup movie `DukeIntro` (a "jmarshall" leftover)**: it failed and aborted the whole startup sequence | retail ctor 2013 rva 0x100ad0 reads `[FullScreenMovie] StartupMovies` only |
| `Engine/Bink/Src/DisFullScreenMovieBink.h` (new), `Engine/Src/disfullscreenmoviebink.cpp` | `FDisFullScreenMovieBink`, 2013 behaviour: `StaticInitialize` 0x53d470, ctor 0x53d130 (`[DisFullScreenMovieBink] fLoadingDelay`, `bAlwaysAutoStart`, `+MapsToAutoStart`), `Tick` 0x532b70, `InputKey` 0x533bb0 (first local player's controller only), `GameThreadPlayMovie` 0x532c00 (overlay `LoadContentPackage`, *LOADING* loops, *CREDITS* drops mode bit 0x100), `GameThreadPlayLoadingMovieAndIntro` 0x533790, `GameThreadStopMovie` 0x535f20, `GameThreadRequestDelayedStopMovie` 0x535e20, `OnBinkTick`/`OnBinkRenderFrame` 0x532d10/0x532d40, `OnRequestLoadingMovieExit` 0x533dc0 and `OnLoadingMovieSkipped` 0x532cd0 (`m_bBinkPause` + `UpdatePausedState(0)`, vtable +336; 2012 used `eventPauseGame`). The Engine-side `UArkBinkOverlayManager` body with its five virtuals (2013 `UDisBinkOverlayManager` vtable 0xd5d130: +292 Tick, +296 OnBinkTick, +300 OnBinkRenderFrame, +304 LoadContentPackage, +308 OnLoadingMovieStopped) | 2012 names from the PDB (`FDisFullScreenMovieBink` 324 bytes) |
| `DishonoredGame/Inc/CppText/UArkBinkOverlayManager.h` (new) | the same five virtuals for the generated DishonoredGame shim (the generator includes it; the include line is present in `DishonoredGameEngineShims.h`) | keeps both declarations identical |

Not ported (TODO in the code): the 2013 calls to the online subsystem's vtable +320/+324 and to `UWorld::m_pAudioSystem`'s
suspend/resume (+316/+320): both are DishonoredGame/OSS shim classes without these virtuals here. `UDisBinkOverlayManager`'s
methods (loading-movie hints, map name, save notification; 2012 rvas in `DishonoredGame/Src/disbinkoverlaymanager.cpp`'s stub
comment) are not ported: the startup movies do not need them and the manager is created by `UDishonoredEngine::Init`
(`m_pBinkOverlayManager`, agent AC/X), which must also call `FDisFullScreenMovieBink::SetOverlayManager`.
The retail caller of the new `InitAudio` virtual was not identified; agent U's `BinkSetSoundSystem` in the `FBinkMovieAudio`
ctor stays, so movie audio uses DirectSound (the log shows `Unmute bink movie` per movie; audibility not verified).

## Hand-offs (edits outside my files, not applied)

1. **Agent X, `Launch/Src/LaunchEngineLoop.cpp`**: retail `appInitFullScreenMoviePlayer` (2013 rva 0x5dd370 = 2012 0x625020,
   identical bytes): no `es2`/`simmobile` switches, and `FDisFullScreenMovieBink::StaticInitialize` instead of
   `FFullScreenMovieBink`, include `../Bink/Src/DisFullScreenMovieBink.h`. Ready to apply: `python build\agentY\patch_launch.py main`
   (three function-local replacements, tagged). My runs use it. Until it lands the shared tree plays movies through the
   reference `FFullScreenMovieBink` (works, but without the Arkane loading-movie logic).
2. **Agent X/Z, `UnGame.cpp`/`LaunchEngineLoop.cpp`** (bridged locally to get past them): `UGameEngine::Init` calls
   `InitGameSingletonObjects` (no such function in either PDB; aborts on `CloudStorageBase.Init`), `FEngineLoop::Init` calls
   `GameInfo->eventOnEngineHasLoaded` and `UGameEngine` calls `eventPreCommitMapChange`/`eventPostCommitMapChange` (events the
   retail script classes do not have); `FEngineLoop::Tick` dereferences `GamePlayers(i)->Actor` without a check at
   `GFrameCounter == 1`; `SpawnPlayActor` fails (`Engine.GameInfo:SpawnPlayerController` unbound).
3. **Agent AA/O, materials**: (a) `MaterialShared.cpp` aborts on a default material without a shader map on D3D9 (retail does the
   same, so the fix is loading the maps); (b) retail gates 786/23 (maps) and 798/23 (material shader types); (c) with them
   applied, every declared mesh-material type's layout differs and `FUniformExpressionSet::Serialize` stops the map; (d) retail
   `FMaterialShaderMap::IsComplete` (2013 rva 0x3ea7d0) is 116 bytes and returns FALSE only while compiling. Details in
   renderer.md, work-list item 2.
4. **Agent AA, `UnFont.cpp`**: `UFont::GetScalingFactor` returns the reference-only `ScalingFactor`, a `DISHONORED_SHIM_STATIC`
   whose value is 0, so every canvas string is drawn at scale 0 (invisible). Retail has no such member (no `GetScalingFactor` in
   either symbol table): return 1 / drop the scale. Bridged locally for the first-frame text.
5. **AA/Z, content**: `StaticMeshComponent` of `Dishonored_MainMenu` InterpActors over-read by 4 bytes (`Got 306, Expected 302`),
   `UNavigationMeshBase::Serialize` reads a garbage array size during the main-menu streaming (the current end of every run).
6. `RHI.cpp`: `GMobileTiledRenderer` defaults to TRUE and nothing on PC resets it (`UGameViewportClient::Draw` then issues an extra
   `ClearAll` per frame). Owner: renderer follow-up.
7. The reference tree has more "jmarshall" leftovers (`Core/Inc/UnMath.h`, `Engine/Src/SplashScreen.cpp`, `Engine/Src/UnAnimPlay.cpp`).

Other agents edited two of my files during the wave: `WinDrv/Src/WinClient.cpp` (DirectInput `verify` → checked calls, tagged
2013 rva 0x5c9d50) and `Engine/Src/ShaderManager.cpp` (a port comment on `FShader::Serialize`); both kept.

## Local-only bridges (worktree `build\agentY_wt` only, never in the shared tree)

`build\agentY\wt_local_*.py` and hand edits marked `agentY LOCAL`: W's Edge-animation parity; overlays of agent Z's in-flight
snapshot (`wt_local_overlay.py build\agentZ_wt`: static mesh, level, world, game-info fixes that get Startup.upk and the map
loaded); `-agentYnomatshaders` (default-material abort → warning), `-agentYnoplayer` (spawn failure non-fatal),
`-agentYserialsize` (serial-size mismatch → warning), `-agentYnocommitevents`, no `InitGameSingletonObjects`, no
`eventOnEngineHasLoaded`, null-actor check at the first tick, `UFont::GetScalingFactor` = 1, the `AActor::PostLoad` skip for the
bridged `DisGameCrowdPopulationManager` (an Actor here, a UObject in retail), appError stack logging, a `-heapcheck` walker,
and RHIClear state logging used to find the black-frame cause.

## Follow-ups (mine)

* renderer.md work list: material shader convergence (item 2) blocks every D3D9 world render; the 58 global layouts and 143
  undeclared Arkane types (items 3/4) follow with their passes.
* Arkane vsync plumbing (`bWantsVSync` through viewport creation), `UDisBinkOverlayManager` methods, the 2013 OSS/audio calls in
  `FDisFullScreenMovieBink`, the retail caller of `InitAudio`.
* `-firstframe`/`-dumpframes`/the inventory lines are bring-up aids; drop them when the renderer converges.
