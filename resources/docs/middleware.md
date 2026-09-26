# Middleware: retail versions, evidence and the Phase 4 plan

Written 2026-09-27 by agent U (Phase 3 wave 2, `PHASE4.md` package U). Plan of record for Phase 4
(`PLAN.md`, "Phase 4 — Third-party dependencies"). Every version below cites the binary it was read
from; the scanner is `resources/tools/binaries/scan_versions.py` (ASCII + UTF-16 string patterns per
library and a hand parser for `VS_VERSIONINFO`, no `pefile`), raw output in
`build/agentU/scan_*.txt`. Offsets are RVAs of the **retail 2013** `Dishonored.exe` unless marked 2012.

Binaries: `D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\` (retail 2013) and
`D:\RecompileDishonored\Dishonored_Debug2012\Binaries\Win32\` (2012 QA build + PDBs). Every shipped
DLL is byte-identical between the two (`resources/docs/binaries.md` SHA-256), so the 2012 PDB library
names describe the retail DLLs too.

## 1. Version table

| Library | Retail version | Evidence | How it is linked |
|---|---|---|---|
| **Scaleform GFx** | **3.3.89** | `Dishonored.exe` rva `0xdf7374` = `"3.3.89"`, 12 bytes after `"gfxVersion"` (`0xdf7368`, the AS `gfxVersion` global the player exposes); 2012 exe rva `0xdaf8e4` same string. 2012 PDB compilands: `libgfx` (5,443 fns, 10.0 % of code) + `libgfx_ime` (192), headers from `development\external\gfx\include\gfxplayer.h` (GFx 3 API: `GFxPlayer.h`, `GRenderer.h`, `GArray.h`) | static libs in the exe; IME via `imm32` (21 imports) |
| **Wwise** | **2012.1.x** (bank format v65) | 2012 PDB source root `d:\branches\wwise_v2012.1\` (291 source files of `aksoundengine`, `akmusicengine`, `akstreammgr`, `ak*fx`, `akvorbisdecoder`, `akmemorymgr`); retail content: `CookedPCConsole\Init.bnk` header `BKHD` version **65**, every `.pck` embeds banks of version 65 (311 `.pck`, 1 `.bnk`). Wwise refuses banks whose version differs from its `AK_BANK_READER_VERSION`, so the retail exe's runtime is the 2012.1 generation as well. The exe carries no version string (only `AK::BankManager`/`AK::EventManager`/`AK::IOThread` thread names, rva `0xdd579c`..) | static libs in the exe (~4 % of code, ≈3,300 fns) |
| **FaceFX** | **SDK 1.7.3.1** (`FxSDKGetVersion() == 1731`) | 2012 PDB `OC3Ent::Face::FxSDKGetVersion` rva `0xb14a10` = `mov eax, 0x6C3; ret`; the identical 10-byte function sits at retail rva `0xafee60`. FaceFX encodes major*1000+minor*100+bugfix*10+revision. Sources compiled from `v:\alt\dishonored\unrealengine3tech\development\external\facefx\fxsdk\{src,inc}` (1,317 fns) + `fxsdk_unreal` lib (122) | static, 1.3 % of code; the SDK *source* was in Arkane's tree |
| **PhysX** | **2.8.4** SDK, DLLs **2.8.4.6** (`PhysXExtensions.dll` 2.8.4.1 "Epic Games" special build) | `PhysXLoader.dll`, `PhysXCore.dll`, `PhysXCooking.dll`, `NxCharacter.dll`: `FileVersion 2.8.4.6`, `InternalName *_FC6_GPU`, PDB path `UnrealEngine3-PhysX\bin\win32\PhysXLoader.pdb` (Epic's UE3 PhysX build); `PhysXCore.dll` source paths `physx\PhysXSDK\2.8.4\trunk\...`; `PhysXExtensions.dll` `2.8.4.1`, `SpecialBuild = Epic Games`. Exe: `push 0x02080400` (= `NX_PHYSICS_SDK_VERSION` 2.8.4.0) before the two `NxCreatePhysicsSDK` calls at rva `0x3d57aa` / `0x3d587b` (2012: `0x3f803a`); 2012 PDB headers `external\physx\sdks\{physics,foundation}\include` | `PhysXLoader.dll` import (6 functions: `NxCreatePhysicsSDK`, `NxGetCookingLib`, `NxGetPhysicsSDK`, `NxGetPhysicsSDKAllocator`, `NxGetUtilLib`, `NxReleasePhysicsSDK`); the rest through the SDK's vtables. Plus the SDK's static `libnxdoublebuffered_release` (1,289 fns, `NxdScene`/`NxdActor`/`NxdCloth`...) in the exe. `cudart32_30_9.dll` / `cudart32_40_12.dll` (CUDA 3.0.9 / 4.0.12) are `PhysXCore`'s GPU path only (no `cudart` string in the exe) |
| **APEX** | shipped but **not used**: DLLs `1.0.0.0` `APEX_B1` (2009), source path `APEXSDK\0.9\feature\UE3_APEX\` | `APEX_release.dll` `FileVersion 1.0.0.0`, `InternalName APEX_B1`, string `APEXSDK\0.9\feature\UE3_APEX\shared\internal\include\ApexSharedSerialization.h`; the exe imports nothing from any `APEX_*.dll`, contains no `NxApex*`/`physx::apex` symbol or `"APEX_*.dll"` string, and the 2012 PDB has 0 `NxApex`/`physx::apex` functions (the `ApexClothing`/`ApexDestructible` names are UE3's `UApex*Asset` classes and the `apexgpuskinvertexfactory.usf` shader, which exist with `WITH_APEX=0`) | not linked; `WITH_APEX=0` matches retail |
| **Bink** | **1.9p** (`binkw32.dll` 1.9.16.0, linked 2009-09-03) | `binkw32.dll` `VS_VERSIONINFO`: `FileVersion 1.9.16.0`, string `1.9p`, `RAD Video Tools`, `Copyright 1994-2009`; PE link timestamp `4A9F8FC2` (2009-09-03); 72 exports (`dumpbin /exports`, `build/agentU/binkw32_exports.txt`); `Dishonored_Debug2012\Binaries\Win32\binkw32.pdb` present (same GUID). The exe imports 20 of them (`imports_2013.csv`) | `binkw32.dll` import; glue `Engine/Bink` + Arkane's `DisFullScreenMovieBink.cpp` (2012 PDB) |
| **Steamworks** | interfaces `SteamClient012`, `SteamUser016`, `SteamFriends011`, `SteamUtils005`, `SteamMatchMaking009`, `SteamMatchMakingServers002`, `SteamUserStats010`, `SteamApps005`, `SteamNetworking005`, `SteamRemoteStorage006`, `SteamHTTP001`, `SteamScreenshots001`, `SteamGameServer011`, `SteamGameServerStats001`, `SteamContentServer002` → **SDK 1.18–1.19 generation (spring 2012)**; `steam_api.dll` `1.30.50.46` | `steam_api.dll` strings at `0x13480`–`0x139f4` (this SDK generation still exports the `SteamUser()`-style accessors, so the interface version strings live in the DLL, not the exe); `VS_VERSIONINFO` `FileVersion 01.30.50.46`, `FileDescription Steam Client API (buildbot_winslave04_steam_steam_rel_client_win32)`. 18 imports in 2013 (`SteamAPI_Init/Shutdown/RunCallbacks/Register*/Unregister*`, `Steam{Apps,Friends,GameServer,Matchmaking,MatchmakingServers,Networking,RemoteStorage,User,UserStats,Utils}`, `SteamGameServer_Shutdown`); 2012 additionally `SteamAPI_RestartAppIfNecessary`. 2012 PDB `external\steamworks\sdk\public\steam` (20 inline fns) | `steam_api.dll` import; module `OnlineSubsystemSteamworks` (250 fns) |
| **libcurl** | `libcurl.dll` **7.77.0** (2021, `The curl library, https://curl.se/`) — a DLL newer than the exe | `libcurl.dll` `VS_VERSIONINFO` `7.77.0`, strings `libcurl/7.77.0`; the retail install's DLL was replaced after 2013 (no PDB, OpenSSL OIDs inside). Exe imports 7 functions (`curl_easy_init/setopt/perform/cleanup`, `curl_global_init/cleanup`, `curl_slist_append`); the 2012 exe has no curl | `libcurl.dll` import, 2013 only |
| DirectX | `d3d9.dll` (`Direct3DCreate9`, `D3DPERF_*`), `dinput8.dll`, `xinput1_3.dll`; `nvapi.dll` loaded dynamically (`nvapi_QueryInterface`, rva `0xdf508c`), 10 `NvAPI_` stubs from `nvapi.lib` (2012 PDB) | `imports_2013.csv`; 2012 PDB `lib:nvapi` from `r195` driver branch | already handled (`cmake/ReferenceExternals.cmake`, `Dependencies.cmake`) |
| zlib / libpng / bzip2 / LZO | zlib **1.2.3** (`"1.2.3 Copyright 1995-2005 Jean-loup Gailly"`, rva `0xdf1478`.. two copies: zlib and the one inside libpng), libpng **1.2.8** (`"1.2.8"` at rva `0xe04e34` next to the `PNG` signature and the `png_read_png` messages; the `"1.2.34"` at `0xc3dad0` beside `PNG Error: %s` is the header-side `PNG_LIBPNG_VER_STRING` UnPNG.cpp passes to `png_create_read_struct`, i.e. Arkane compiled against 1.2.34 headers and linked the 1.2.8 library), bzip2 **1.0.6** (`"1.0.6, 6-Sept-2010"` rva `0xbbf5dc` with `BZ2` error strings; not in the reference tree, probably Wwise's or FaceFX's), LZO Pro (`lzopro_lzo1x_decompress_safe`, 65 fns; no version string) | strings; 2012 PDB `lib:zlib` 49, `lib:libpng` 191, `lib:lzopro` 65 | zlib/libpng via FetchContent (newer versions, data-compatible); LZO via lzokay (done) |
| ogg / vorbis | `ogg.dll`, `vorbis.dll` 1.2.0.4889 "Compiled by Epic Games 2009", `vorbisenc.dll`, `vorbisfile.dll` | `VS_VERSIONINFO`; the exe imports none of them (audio is Wwise) | not linked (`WITH_OGGVORBIS=0`) — editor/cooker leftovers, same for `wxmsw28u_*.dll` (wxWidgets 2.8 for UnrealEd) and `SteamAPIUpdater.dll` |

Build ids for reference: 2013 exe `ZeniMax Media Inc.`, `Copyright 2012`, branch string `Dishonored_Campfire`
(rva `0xba35c0`); 2012 exe source root `v:\dishonored\unrealengine3qatest\` (Arkane) and
`v:\alt\dishonored\unrealengine3tech\` (FaceFX SDK source).

## 2. What retail needs from each library, and what we can do

Three categories:

* **DLL imports** (Bink, PhysX loader, Steam, curl): the retail DLL is the runtime. An import
  library plus reconstructed headers is enough; the glue in our tree calls into the DLL exactly like
  retail. Nothing to reimplement.
* **Static libraries in the exe** (Scaleform, Wwise, FaceFX, `libnxdoublebuffered`): retail's copy
  of the library is the ~16 % of the exe that is not Epic/Arkane code. Either the matching SDK is
  obtained and linked (identical behaviour, zero porting), or the library is rewritten from the
  decompile (large), or the engine-facing subset is reimplemented on a different runtime.
* **Nothing** (APEX, ogg/vorbis, wx): shipped but not linked. Keep the switches off.

### 2.1 Bink (done: import lib + header behind `DISHONORED_WITH_BINK`)

* What the exe needs: 20 stdcall entry points (`_BinkOpen@8`, `_BinkClose@4`, `_BinkDoFrame@4`,
  `_BinkNextFrame@4`, `_BinkWait@4`, `_BinkShouldSkip@4`, `_BinkPause@8`, `_BinkGoto@12`,
  `_BinkGetKeyFrame@12`, `_BinkGetRects@8`, `_BinkGetRealtime@12`, `_BinkGetFrameBuffersInfo@8`,
  `_BinkRegisterFrameBuffers@8`, `_BinkGetError@0`, `_BinkSetIOSize@4`, `_BinkSetSoundSystem@8`,
  `_BinkOpenDirectSound@4`, `_BinkSetSoundTrack@8`, `_BinkSetVolume@12`, `_BinkSetPan@12`) and the
  `BINK`/`BINKFRAMEBUFFERS`/`BINKREALTIME` layouts (the glue reads `Bink->Width/Height/Frames/
  FrameNum/FrameRate/FrameRateDiv/NumTracks/ReadError` and fills `BINKFRAMEBUFFERS` planes).
* Retail behaviour pinned from the exe: `BinkSetSoundSystem(BinkOpenDirectSound, 0)` at rva
  `0xe1fa8` (the only call site) — retail uses DirectSound, not the reference's XAudio2 path;
  `BinkOpen` flags `BINKFROMMEMORY|BINKSNDTRACK|BINKNOFRAMEBUFFERS` (`0x04004400`, rva `0xfc2a1`)
  for memory movies and `BINKFROMMEMORY|BINKALPHA|BINKNOFRAMEBUFFERS` (`0x04100400`, rva `0x1beb45`)
  for `UCodecMovieBink`; `BinkGetRects(.., BINKSURFACEFAST=0)` (`0x1e71e4`); `BinkGetKeyFrame(..,
  BINKGETKEYNEXT=1)` + `BinkGoto(.., BINKGOTOQUICK=1)` (`0xfc5ae`); no async Bink (no
  `BinkDoFrameAsync`/`BinkStartAsyncThread` imports), no `BinkSetMemory`, no `BinkSetSpeakerVolumes`.
  Per-track mixing is `BinkSetVolume`/`BinkSetPan` (2012 `FBinkMovieAudio::SetAudioChannels` rva
  `0xd95f0`, ported into `FullScreenMovieBink.inl` as `HandleDirectSoundVolumes`).
* Structures: the 2012 PDB carries the full Bink types (`types.json`: `BINK` 928 bytes, `BINKIO`
  324, `BINKSND` 384, `BINKFRAMEBUFFERS` 120, `BINKREALTIME` 56, `BINKRECT`, `BINKPLANE`,
  `BINKFRAMEPLANESET`, `BUNDLEPOINTERS`), so `Engine/Bink/Src/bink.h` is exact where it matters;
  `BINKSUMMARY` and the unused flag values are public-header knowledge and marked so.
  `binkw32.pdb` (2012 tree) is available if any other type is ever needed.
* Import library: `lib.exe /def` cannot express "import `_BinkOpen@8` by that decorated name" on
  x86 (a `.def` entry `BinkOpen@8` imports `BinkOpen@8`, `_BinkOpen@8` yields the symbol
  `__BinkOpen@8`; only `@ordinal NONAME` works, and that pins the DLL build). RAD's own import lib
  comes from `__declspec(dllexport)` stdcall functions, so `cmake/Bink.cmake` builds
  `Engine/Bink/Src/binkw32_stub.cpp` into a throw-away `build/<dir>/bink/stub/binkw32.dll` whose
  import library makes our exe import `binkw32.dll!_BinkOpen@8` exactly like retail (verified with
  `dumpbin /imports` on a test exe, `build/agentU/deftest/`). The stub DLL is never copied next to
  the exe; the retail DLL is the runtime.
* Licence/availability: Bink SDK is licensed by RAD/Epic; nothing to obtain — the shipped DLL is
  all the runtime we need. Bink 1.9p is 2009; the DLL runs on current Windows.
* Milestone 4 still needs (follow-ups, not in this package): Arkane's `FDisFullScreenMovieBink`
  (2012 PDB 324 bytes, `[DisFullScreenMovieBink]` ini: `fLoadingDelay`, `bAlwaysAutoStart`,
  `MapsToAutoStart`, `IntroMovieToPlay`), `UArkBinkOverlayManager`/`UDisBinkOverlayManager`
  (text overlays on loading movies), `appInitFullScreenMoviePlayer` choosing
  `FDisFullScreenMovieBink::StaticInitialize` (2012 rva `0x57dad0`), and `Engine/Shaders/
  BinkShaders.usf` at runtime (retail has the compiled global shader in the shader cache).
  `DefaultEngine.ini` `[FullScreenMovie]`: `BinkIOStreamReadBufferSize=1000000`, `StartupMovies`
  `Black_266ms, ZenimaxLegal, ZenimaxLegalFR, LogoBethesda, LogoArkane, UE3_logo, Legal, Loading`;
  movies live in `DishonoredGame\Movies\*.bik`.

### 2.2 PhysX 2.8.4 (+ `libnxdoublebuffered`)

* What `UnNovodexSupport.h` needs: `NxFoundation.h`, `NxStream.h`, `NxPhysics.h`, `NxCooking.h`,
  `NxSceneQuery.h`, `NxExtensions.h`, `NxExtensionQuickLoad.h`, `fluids/NxFluid.h`,
  `fluids/NxParticleData.h`, `fluids/NxFluidEmitterDesc.h`, `NxdScene.h` (double-buffered scene)
  and, under `WITH_APEX`, `NxApex.h`/`PxMat34Legacy.h` (off). `WITH_NOVODEX` guards 110 Engine files
  (`UnPhysLevel.cpp`, `UnPhysAsset.cpp`, `UnSkeletalComponent.cpp`, particles, vehicles, cloth,
  fluids ...). The reference tree's `External/Novodex` is NovodeX **2.1.2** (2004) and cannot compile
  this header (no `NxCooking.h`, no `NxSceneQuery.h`, no fluids, no `Nxd*`).
* Runtime: the retail DLLs are the Epic build of PhysX 2.8.4.6 (`*_FC6_GPU`); `PhysXLoader.dll`
  resolves `PhysXCore.dll`/`PhysXCooking.dll` next to the exe (strings `PhysXCore Path`,
  `PhysX\Runtimes`) and falls back to the system PhysX install. Everything else is reached through
  the `NxPhysicsSDK` vtables, so the headers must be the **2.8.4** ones (vtable order is per
  release). `NxCharacter.dll` (2.8.4.6) is the character controller the reference glue uses via
  `NxControllerManager` (`NxCharacter.h`, loaded by `PhysXLoader`).
* `libnxdoublebuffered_release` (1,289 functions in the exe, `NxdScene`/`NxdActor`/`NxdCloth`/
  `NxdFluid`/`NxdSoftBody`/`NxdWheelShape`...) is the SDK's "double buffered" helper library shipped
  as source + prebuilt lib inside `PhysX SDK 2.8.x\SDKs\NxdDoubleBuffered`. It is not in a DLL, so
  it is part of what the SDK must provide.
* **Done (agent AL, 2026-09-26): no SDK is needed.** `source/Development/Src/External/PhysX284` is our own
  reconstruction of the 2.8.4 API, read out of the PDBs that ship next to the DLLs in `Dishonored_Debug2012`
  (`resources/tools/pdb/dia_types.py`: 109 descriptors, 68 interfaces with exact vtable slots, 103 enums), with every
  descriptor default decoded from the DLLs' own compiled `setToDefault()` and `NxPhysicsSDKDesc`'s from retail
  `InitGameRBPhys` (2013 rva `0x3d5710`). `cmake/PhysX.cmake` builds a `PhysXLoader.dll` import library the Bink way,
  `WITH_NOVODEX=1` compiles and links, and the shipped SDK creates scenes, actors, shapes, materials and convex meshes
  at runtime. `NX_DISABLE_FLUIDS=1`, `USE_QUICKLOAD_CONVEX=0` and `SUPPORT_DOUBLE_BUFFERING=0` (no
  `libnxdoublebuffered`); see `agents/agentAL.md`. The rest of this section is the pre-AL analysis.
* Availability: the PhysX 2.8.4 SDK (installer `PhysX_2.8.4_SDK_Core.msi`, 2010) was a registered
  NVIDIA developer download, EOL; copies exist in UE3 licensee trees (`Development/External/PhysX/
  SDKs`) and are mirrored. The reference 10897 tree does not carry it. **Blocker for the user:**
  obtain `SDKs/{Foundation,Physics,Cooking,PhysXLoader,NxCharacter}/include` + `NxdDoubleBuffered`
  from a PhysX 2.8.4 SDK (2.8.4.x; the vtable layout is stable within 2.8.4); keep it outside the
  repo like the reference tree (`DISHONORED_REFERENCE_DIR`-style path in `ReferenceExternals.cmake`).
* Recommendation: **link the shipped DLLs through the 2.8.4 SDK headers** (`WITH_NOVODEX=1`,
  `WITH_PHYSX_COOKING=1`, `NX_DISABLE_FLUIDS` as UnBuild.h sets, `WITH_APEX=0` permanently — retail
  does not load APEX). The alternative (reconstruct `NxPhysicsSDK`/`NxScene`/`NxActor`/... vtables
  from `PhysXCore.pdb`, which the 2012 tree ships) is feasible since the PDB names every virtual,
  but it is hundreds of interfaces and only worth it if no SDK copy can be found. Cooked collision
  data in the packages (`FKCachedConvexData`, cloth/softbody meshes) is 2.8.4 format either way.
  Not needed before milestone 5 (maps): milestone 4 only needs `GNovodexSDK` to initialize or the
  physics calls to be skipped, which `WITH_NOVODEX=0` already does.

### 2.3 Scaleform GFx 3.3.89

* What retail uses: `GFxUI` (Arkane, 1,041 fns, 20 files) is written against the GFx **3** API
  (`GFxPlayer.h`, `GFxLoader`, `GFxMovieView`, `GRenderer`, `GFxImageInfo`, `GFxIMEManagerWin32`);
  2012 PDB `external\gfx\include\*` (the 3.3 headers) and `libgfx`/`libgfx_ime` (10.4 % of the exe,
  5,635 functions: `GFxMovieRoot`, `GASEnvironment`, `GFxSprite`, `GFxTextDocView`, `GTessellator`,
  `GFxFontCacheManagerImpl`...). Our `GFxUI` folder mixes the reference's GFx **4** glue
  (`Scaleform*.cpp`, `Render/RHI_HAL.h`, `Kernel/SF_*.h`) — useless against 3.3 — with the Arkane
  GFx-3 PDB stubs (`gfxui*.cpp`). The main menu, HUD, loading overlays and every `.swf`/`.gfx` in
  `CookedPCConsole` are GFx 3 content (AS2, `gfxexport` 3.3 output).
* Availability: Scaleform was bought by Autodesk (2011) and discontinued (2018); GFx 3.3 SDKs were
  licensee-only source drops (`Src/GFx`, `Src/Kernel`, prebuilt `libgfx.lib` per compiler), never
  public. No legal source exists. UE3 licensee trees of the 2011 era carried
  `Development/External/GFx/` (headers + `Lib/Win32/Msvc90/libgfx.lib`); an MSVC 9 static lib links
  into an MSVC 2022 x86 exe (C ABI/object format compatible; the CRT mismatch is the risk).
* Options: (a) **matching SDK** (3.3.89 headers + libs): zero porting of the 5,635 functions, exact
  behaviour, but depends on a leaked licensee drop and an MSVC-9 lib; (b) **rewrite from
  decompile**: 1.1 MB of code without symbols in the retail exe (the 2012 PDB has names for
  `libgfx` functions but no source lines) — the biggest single item of the whole project, not
  realistic before Phase 3 finishes; (c) **subset**: implement the GFx 3 interfaces `GFxUI` calls
  (`GFxLoader`, `GFxMovieDef`, `GFxMovieView`, `GFxValue`, `GFxExternalInterface`,
  `GFxFSCommandHandler`, `GRenderer`, `GFxImageInfo`, IME) on an open runtime. The AS2 VM +
  renderer needed is essentially a Flash player; the only open candidates (Ruffle is Rust/AS2+AS3
  complete; Gnash, Lightspark dead) would be a second, foreign runtime with a large adapter.
* Recommendation: **(a) if a 3.3.x licensee SDK can be sourced by the user, else (c) with Ruffle
  behind a `GFxUI` adapter, and never (b)**. Do not block on it: `WITH_GFx=0` keeps everything else
  compiling, and milestone 4's "main menu renders" is the first point that needs any of it. For the
  interim, the D3D9 device + Bink startup movie half of milestone 4 is reachable without Scaleform;
  the main-menu half should be declared reached when `GFxUI`'s `UGFxMoviePlayer::Start` opens
  `Dishonored_MainMenu` (retail string rva `0xcc53a8`) through whichever runtime lands.
  Whatever runtime is chosen, `GFxUI`'s Arkane side (`gfxuiengine.cpp`, `gfxuimovie.cpp`,
  `gfxuidatastore.cpp`, 1,041 fns) is ported from the 2012 decompile regardless, so that work can
  start now against the GFx 3 header names (2012 PDB `external\gfx\include`).
* If a 3.3.x SDK turns up, pin it: `GFxPlayer.h` `#define GFC_FX_VERSION_STRING "3.3.89"`; a
  different 3.3 build is fine if `GFxUI` compiles (the API was stable inside 3.3), and the cooked
  `.gfx` files load with any 3.3.

### 2.4 Wwise 2012.1

* What retail uses: `AkAudio` (Arkane, 216 fns: `UAkAudioDevice`, `UAkComponent`, `UAkEvent`,
  `UAkBank`, blocking IO hook) on top of the Wwise runtime (`AK::SoundEngine`, `AK::MusicEngine`,
  `AK::StreamMgr`, `AK::MemoryMgr`, plug-ins `AkVorbisDecoder`, `AkRoomVerb`, `AkMatrixReverb`,
  `AkHarmonizer`, `AkPitchShifter`, `AkStereoDelay`, `AkTremolo`, `AkDelay`, `AkTimeStretch`,
  `AkParametricEQ`, `AkSilenceSource`, `AkSink` (DirectSound/XAudio2)). Content: `Init.bnk` +
  311 `.pck` (`AKPK` packages with embedded v65 banks and Vorbis-encoded media), `WwiseGlobal.upk`.
* Availability: Audiokinetic keeps every Wwise version downloadable through the Wwise Launcher
  (account required; "Legacy versions" list goes back to 2012.x). The SDK (`SDK/include/AK/*.h`,
  `SDK/Win32_vc90/{Release,Profile}/lib/*.lib`) is free to download; the licence is per-title
  (Dishonored's was Bethesda's), so a *distributable* recompiled exe would need Audiokinetic's
  consent, but a private build is unproblematic. **Blocker for the user:** install Wwise 2012.1.x
  (latest 2012.1 patch; the bank reader version 65 is shared by all 2012.1 patches) via the Launcher
  and point the build at its `SDK` folder (outside the repo).
* Options: (a) **matching SDK**: link `AkSoundEngine.lib`, `AkMusicEngine.lib`, `AkStreamMgr.lib`,
  `AkMemoryMgr.lib`, `AkVorbisDecoder.lib`, the effect plug-ins and `AkSink`; the `AkAudio` module
  is ported from the 2012 decompile against the real headers — exact behaviour, banks load as-is;
  (b) rewrite the 3,300-function runtime: no; (c) **subset**: a stub `AkAudio` (device initializes,
  `PostEvent` returns an id, no sound) satisfies the engine and the script layer (`AkComponent`
  natives return defaults). Vorbis decoding of the `.pck` media through an open decoder would
  give sound effects without Wwise's graph, but Dishonored's mix (RTPCs, states, switches, music
  engine) is bank logic that only the real runtime evaluates.
* Recommendation: **(c) now, (a) as soon as the user installs 2012.1** — audio stays silent through
  milestones 4–5 (`bring-up` runs are `-nosound` anyway). The retail exe was linked against MSVC 9
  Wwise libs; Audiokinetic shipped 2012.1 for vc90 and vc100, both link into an MSVC 2022 x86 exe
  with the CRT-mismatch caveat (`/NODEFAULTLIB` juggling); if that bites, 2012.1's libs can be
  wrapped in a small MSVC-9-built DLL. The `AkAudio` port itself (216 fns) is unaffected by the
  choice and can be written from the 2012 decompile against the public 2012.1 headers as soon as
  they exist.

### 2.5 FaceFX 1.7.3.1

* What retail uses: the FaceFX SDK compiled from source (`external\facefx\fxsdk\src`, 1,317 fns +
  `fxsdk_unreal` 122) behind the reference glue `Engine/FaceFX` (`UnFaceFXSupport.h` includes
  `FxSDK.h`, `FxMemory.h`, `FxActor.h`, `FxActorInstance.h`, `FxArchiveStore*.h`);
  `WITH_FACEFX` guards `UFaceFXAsset`/`UFaceFXAnimSet` serialization (the cooked `.upk`s contain
  the FaceFX archives, format fixed) and the skeletal-mesh morph/bone passes; `Dishonored.exe`
  strings `FaceFX Anim: %s.%s.%s. Open and resave FaceFXAsset (%s).` (rva `0xc9f960`) show the
  loader still checks archive versions.
* Availability: the FaceFX SDK was a paid OC3 licence with source; the Unreal integration ("FaceFX
  for UE3") was included in UE3 full-source licences, so 2011-era licensee trees carry
  `Development/External/FaceFX/FxSDK/{Inc,Src}`. Not public; the reference 10897 tree lacks it.
  FaceFX 1.7.x ended in 2013 (FaceFX 2). OC3 still exists (`facefx.com`).
* Options: (a) source from a licensee tree — same caveat as Scaleform; (b) rewrite from decompile:
  1,317 functions *with* PDB names and inlined header paths (`FxArrayImpl.h`, `FxAnim.h`, ...),
  i.e. a well-structured C++ library whose data format (`FxArchive`, `FxActor`, `FxFaceGraph`,
  `FxAnim` curves) is fully visible; (c) subset: load the archives just far enough to skip them
  (`WITH_FACEFX=0` path in the reference: `UFaceFXAsset::Serialize` reads the raw bytes into
  `RawFaceFXActorBytes` and keeps them), no lip-sync.
* Recommendation: **(c) until milestone 6, then (b)** — `WITH_FACEFX=0` is what the reference glue
  supports today: the assets round-trip as byte blobs, characters simply do not lip-sync. The
  runtime is the smallest of the three static libraries and the one with the most symbols, so a
  decompile-driven rewrite is tractable when facial animation matters (campaign polish), and it
  avoids a licence we cannot obtain. If a licensee `FxSDK` source drop appears, (a) is a drop-in.

### 2.6 Steamworks

* What retail uses: `OnlineSubsystemSteamworks` (250 fns; leaderboards, achievements, cloud saves
  through `ISteamRemoteStorage006`, presence) and `SteamAPI_Init` at startup; DLC checks through
  `ISteamApps005::BIsDlcInstalled` (`Req_DLC05_*`). `bEnableSteam=false` is already set in the
  retail `DefaultEngine.ini` for this install (`:649`), and `dismod` hooks `SteamRemoteStorage`
  to redirect saves to `LocalFiles\` (`D:\Christmas\github\dismod\src\hooks\steam.h`).
* Availability: the Steamworks SDK is free with a partner account; every past version is in the
  partner site's download archive (`https://partner.steamgames.com/downloads/list`). The interface
  set above must match **exactly** (each accessor asks the client for e.g. `"SteamUser016"` and
  gets `NULL` otherwise), so the SDK is one of 1.18/1.19 (2012): pick by diffing the
  `STEAM*_INTERFACE_VERSION` constants in the candidate's `public/steam/isteam*.h` against the
  15 strings in the table. **Blocker for the user:** download it (never committed; path outside the
  repo like the reference tree). Modern `steam_api.dll` builds still serve these old interfaces, so
  the retail `steam_api.dll` (1.30.50.46) or any newer one works.
* Offline path (Phase 8 design): `WITH_STEAMWORKS=1` but `OnlineSubsystemSteamworks::Init`
  early-outs when `-nosteam` or `bEnableSteam=false` (already the retail ini state), returning the
  `OnlineSubsystemPC`-style behaviour: local profile, achievements logged, leaderboards empty,
  `RemoteStorage` → `FFileManager` under `Saves\`. `SteamAPI_Init` is never called, so the exe
  runs without the Steam client and without `steam_api.dll` if the 18 imports are delay-loaded
  (`/DELAYLOAD:steam_api.dll`, as done for `nvtt.dll`). Milestone 4 needs none of this; keep
  `WITH_STEAMWORKS=0` until the SDK is in place, then build the module with `-nosteam` as default.

### 2.7 libcurl (2013 only)

7 imports used by the 2013-only online feature (`curl_easy_*`); the shipped DLL is a 2021 rebuild
that Bethesda pushed later. Phase 8 deletes the feature (PLAN.md). If a build ever needs it before
that, libcurl is open (`curl.se`, FetchContent) and any 7.x satisfies these 7 entry points.

### 2.8 Not linked: APEX, ogg/vorbis, wxWidgets, cudart, SteamAPIUpdater

Keep `WITH_APEX=0`, `WITH_OGGVORBIS=0`; nothing to do. The `cudart*.dll`s belong to
`PhysXCore.dll`'s GPU path and are only needed next to the exe when PhysX GPU acceleration is on.

## 3. Minimum for milestone 4 (D3D9 device, Bink startup movie, Scaleform main menu)

| Piece | Needs | State |
|---|---|---|
| D3D9 device | `D3D9Drv` module (agent P), DirectX SDK from the reference tree | in progress (P) |
| Bink startup movies | `DISHONORED_WITH_BINK=ON` (this package), `FDisFullScreenMovieBink` + overlay manager port, `BinkShaders.usf` | import lib + header + reference glue compile behind the option; Arkane glue is a follow-up |
| Scaleform main menu | a GFx 3.3 runtime (2.3) + the `GFxUI` Arkane port | **decision pending on the user** (SDK availability); `WITH_GFx=0` until then |
| Audio | none — `-nosound`; `AkAudio` stub module (T's registrants) so `UAkAudioDevice` binds | silent is fine |
| Physics | none for the menu (`WITH_NOVODEX=0`); maps need 2.2 | milestone 5 |
| Steam | none (`bEnableSteam=false`) | Phase 8 |

## 4. Blockers the user must resolve (SDKs to obtain, never committed)

1. ~~**PhysX 2.8.4 SDK**~~ — **resolved without it** (agent AL): the API was reconstructed from the shipped
   DLLs and their PDBs, `WITH_NOVODEX=1` is on by default. Only the SDK's static `libnxdoublebuffered`
   (`NxdScene`) has no substitute, so the build runs single-buffered.
2. **Wwise 2012.1.x SDK** — free from Audiokinetic's Launcher (legacy versions), needed to link the
   real audio runtime; until then audio is a stub.
3. **Steamworks SDK 1.18/1.19** — free from the partner site archive; needed for
   `WITH_STEAMWORKS=1` (Phase 8 offline path); which of the two is settled by the header constants.
4. **Scaleform GFx 3.3.x** — decide between a licensee SDK drop (if any can be sourced) and the
   Ruffle-adapter route; this is the gating decision for the second half of milestone 4.
5. **FaceFX 1.7.3 SDK source** — optional; without it `WITH_FACEFX=0` (no lip-sync) and later a
   decompile-driven rewrite.

## 5. Tools and artefacts

* `resources/tools/binaries/scan_versions.py <files|dir> [--grep RE] [--all]` — the scanner used for
  every row of section 1.
* `cmake/Bink.cmake` (`DISHONORED_WITH_BINK`, `DISHONORED_RETAIL_DIR`), `cmake/DishonoredDefines.cmake`
  (`USE_BINK_CODEC=1` + `Dishonored::bink` on every target when ON),
  `source/Development/Src/Engine/Bink/Src/bink.h` (reconstructed header),
  `source/Development/Src/Engine/Bink/Src/binkw32_stub.cpp` (import-library source, export table in
  its header comment), `FullScreenMovieBink.inl` Windows path switched to DirectSound like retail.
* `build/agentU/`: `scan_dlls.txt`, `scan_exe2013.txt`, `scan_exe2012.txt`, `binkw32_exports.txt`,
  `steam_api_exports.txt`, `physxloader_exports.txt`, `apex_release_exports.txt`,
  `nxcharacter_exports.txt`, `decomp/` (2012 Bink glue decompiles), `deftest/` (the `.def` vs
  stub-DLL import-name experiment).
