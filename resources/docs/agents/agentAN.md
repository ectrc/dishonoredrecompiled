# Agent AN report — our own Wwise 2012.1 API and a silent backend behind it (2026-09-26)

Package "**AN**" of wave 5: write the Wwise 2012.1 (AK SoundEngine) headers from the evidence in the
binaries, put a silent backend behind them, and port `AkAudio` plus Engine's audio path onto that surface so
Arkane's audio code stops being a wall of stubs.

Build dir `build\agentAN`, snapshot `build\agentAN_wt` (**the shared tree does not configure**: another agent
is mid-flight on PhysX 2.8.4 and `WITH_NOVODEX=1`, see "The shared tree" below), baseline build
`build\agentAN_off`, IDA copy `resources\docs\idb\retail2013_agentAN.i64` (headless only), 86 decompiles in
`build\agentAN\dec2013`, patch scripts `build\agentAN\patch_*.py` / `write_*.py` / `snapshot_files.py`,
evidence tools `build\agentAN\bank_stid.py` and `build\agentAN\akpk_sweep.py`.
`resources\docs\agents\agentAN_status.csv` has 111 rows (82 `ported`, 23 `written`, 3 `bringup`, 3 `pending`).
**No commits, nothing staged.**

## 1. The honest frame: what is and is not achievable

Wwise is **statically linked into `Dishonored.exe`** — ~3,300 functions of `aksoundengine`, `akmusicengine`,
`akstreammgr`, `ak*fx`, `akvorbisdecoder`, `akmemorymgr` with the 2012 PDB source root
`d:\branches\wwise_v2012.1\` (`middleware.md` 1 and 2.4). There is no DLL to import and therefore **no
runtime to bind against**, unlike Bink (a shipped DLL), PhysX (shipped DLLs) or Steam (`steam_api.dll`).
Reimplementing the sound engine is out of scope and was not attempted.

What *was* achievable, and is done:

| | State |
|---|---|
| Our own AK 2012.1 headers, from the binaries | **done**: 9 headers, every type / enum value / signature carrying its evidence |
| A backend behind them that the game can run on | **done**: bookkeeping plus *real* bank IO and BKHD parsing, no DSP |
| `cmake/Wwise.cmake` with a `Dishonored::wwise` target and an option | **done**, and the same include path takes the licensed SDK when it arrives |
| `AkAudio` and the Engine audio path ported onto it | **done** for the device, the IO hook, the component, the ambient sounds, the nine Kismet actions, `UAkBank`, `UAkEvent` and `AActor`'s five Wwise natives |
| Real audio | **not possible without the licensed SDK or a reimplementation** — section 7 |

## 2. Our Wwise 2012.1 headers (`source/Development/Src/External/Wwise2012/include`)

The directory mirrors the SDK's own layout on purpose, so that pointing the include path at an installed
`SDK/include` is the whole switch:

```
AK/AkWwiseSDKVersion.h
AK/SoundEngine/Common/{AkTypes,AkCallback,AkSoundEngine,AkModule,AkMemoryMgr,IAkStreamMgr,AkStreamMgrModule,AkQueryParameters}.h
AK/MusicEngine/Common/AkMusicEngine.h
DishonoredWwiseSilent.h        (ours, not part of the AK API: the backend's seam)
```

Nothing of Audiokinetic's source or headers is reproduced. Every declaration comes from one of two places:

* **the 2012 PDB type database** `resources/docs/types/types.json` (extracted with
  `resources/tools/pdb/dia_dump.py`) for every struct's members and size and every enum's values — 20
  structs and 12 enums, each with a `static_assert` of the PDB size in the header;
* **the exact demangled signatures** in `resources/docs/symbols/functions.csv` for every function, with the
  2012 rva in a trailing comment. The surface is the 62 distinct AK entry points that appear in the
  decompiles of `AkAudio`, Engine's `akbank`/`akevent` and `UDishonoredAudioSystem`, plus same-family
  neighbours where the signature was free.

Three points where the signatures pinned something a public header would not have told us:

* **the scalar widths are Wwise's Win32 ones**: `GetIDFromString(wchar_t const *)` returns `unsigned long`,
  so `AkUInt32` is `unsigned long`; `LoadBank(wchar_t const *, long, unsigned long &)` makes `AkMemPoolId` =
  `long` = `AkInt32`; `SetBankLoadIOSettings(float, char)` makes `AkPriority` = `AkInt8`.
* **`AkGameObjectID` is `AkUIntPtr`, not `AkUInt32`**: `RegisterGameObj(unsigned int, char const *)`. That is
  not cosmetic — retail passes the `UAkComponent*` itself as the game object id
  (`UAkComponent::Attach` 2013 rva `0x5b3d20` is `RegisterGameObj(this)`).
* **the bank callback of 2012.1 takes four scalars**, not a struct:
  `void (__cdecl *)(unsigned long, enum AKRESULT, long, void *)`. Later Wwise generations pass an
  `AkBankCallbackInfo`, and that struct is absent from this PDB — which is itself the confirmation.

Packing: the AK structs carry `#pragma pack(8)` (what Audiokinetic builds with) while every UE3 module is
`/Zp4`, so the sizes hold in both worlds; the `static_assert`s in the headers are what proves it.

The interface vtables (`AK::IAkStreamMgr`, `IAkStdStream`, `IAkAutoStream`,
`AK::StreamMgr::IAkLowLevelIOHook` / `IAkIOHookBlocking` / `IAkFileLocationResolver`) are the 2012 PDB's
synthesized `*_vtbl` records slot for slot; the reference-or-pointer shape of each parameter comes from the
demangled signatures of Arkane's own overrides, e.g.
`CAkUnrealIOHookBlocking::Read(struct AkFileDesc &, struct AkIoHeuristics const &, void *, struct AkIOTransferInfo &)`
(2013 rva `0x5b0c20`), so it is retail's, not a guess.

## 3. The silent backend (`source/Development/Src/External/Wwise2012/Src`, 6 units)

`AkSilent{Common,Hooks,MemoryMgr,StreamMgr,SoundEngine,MusicEngine}.cpp`. Plain C++ and the CRT — no
Core/Engine dependency, because it stands in for a third-party static library. It logs through a hook
`AkAudio` installs (`debugf`) and allocates through `AK::AllocHook`, which `AkAudio` points at `appMalloc`.

**What it does for real**

* **the Wwise name hash.** `GetIDFromString` is FNV-1 32-bit (`h = h*16777619; h ^= byte`, basis
  2166136261) over the lowercased name. That is not assumed: the 2012 PDB carries
  `AK::FNVHash<AK::Hash32>`, and the variant and constants are **verified against the retail content** —
  `build/agentAN/akpk_sweep.py` over all 981 containers in `CookedPCConsole` (311 `.pck` + 670 `Bk_*.<lang>`)
  finds the hash of each package's own base name in its soundbank table **981/981**, and `Init.bnk`'s BKHD
  bank id is `0x50c63a23` = `fnv1("Init")`. FNV-1a matches none of them. Every event, bank, RTPC, switch and
  state id the game computes is therefore the id the retail banks contain.
* **real bank IO.** `LoadBank` / `PrepareBank` ask the game's registered `IAkFileLocationResolver` for
  `<bank>.bnk` and read its `BKHD` chunk through the registered `IAkIOHookBlocking`, so a load either reports
  the file's real bank version and id (and warns when the id does not match the name hash, or when the
  version is not 65) or reports the file as missing. That is what turns the log into evidence.
* **playing ids and end-of-event.** `PostEvent` hands out monotonic playing ids and remembers the game
  object, event, flags and callback; `StopAll` / `StopPlayingID` / `ExecuteActionOnEvent(Stop|Break)` /
  `Term` / `UnregisterGameObj` fire `AK_EndOfEvent`. `USeqAct_AkPostEvent` and
  `UDishonoredAudioSystem`'s end-of-event queue only ever finish because that callback arrives.
* **game syncs and positions.** Listener and game-object positions, the active-listener mask,
  obstruction/occlusion, dry level, and the last value of every RTPC, switch (global and per object), state
  and trigger are recorded, and the `AK::SoundEngine::Query` getters answer from them.
* **global callbacks** run once per `RenderAudio`, which is the hook retail's `DisAkGlobalCallbackFunc`
  rides on.

**What it does not do**, and what that costs

* no Vorbis decoding, no mixing, no output device — nothing is audible;
* **no bank logic**: an event's action list, attenuation curves, RTPC curves, state/switch containers and the
  whole music hierarchy live in the banks' `HIRC`/`STMG` sections and are not parsed. The visible consequence
  is that `Query::QueryAudioObjectIDs` reports no objects, so `UAkEvent::ComputeMaxRadius` keeps `-1` ("this
  event has no attenuation") and every event is treated as audible everywhere;
* `CreateStd`/`CreateAuto` return `AK_NotImplemented` (no lower engine asks for a stream);
* `PrepareEvent` / `PrepareGameSyncs` succeed without pulling media, because there is no media to pull.

Every one of those is a `DISHONORED(bringup)` comment at the function, not a silent lie.

## 4. `cmake/Wwise.cmake`

* `DISHONORED_WITH_WWISE`, default **ON** because the bindings are in the tree; OFF restores the previous
  all-stub state and is the baseline of section 6. It is forced OFF when `DISHONORED_ENABLE_AKAUDIO` is OFF,
  because `UAkAudioDevice`, the low-level IO hook and the AK allocation hooks all live in that module (so the
  option is declared just before the include now).
* `DISHONORED_WWISE_SDK=<path>`: uses `<path>/include` and `<path>/Win32_vc100|vc90/Release/lib`
  (`AkSoundEngine`, `AkMusicEngine`, `AkStreamMgr`, `AkMemoryMgr`, `AkVorbisDecoder`, the effect plug-ins,
  `AkSink` + `dsound`) instead, sets `DISHONORED_WWISE_SILENT=0`, and does not compile the backend. The
  `AkAudio` port needs no change: the `#if DISHONORED_WWISE_SILENT` branches are only the log hook, the
  allocator installation and who defines `AK::AllocHook`.
* `dishonored_apply_defines()` links `Dishonored::wwise` onto every module, for the same reason Bink is: the
  switch changes bodies in Engine (`akbank.cpp`, `akevent.cpp`, `UnActor.cpp`), AkAudio and DishonoredGame,
  so it must be identical in every unit.

## 5. The port

### AkAudio (`Src/akaudiodevice.cpp`, `Src/akunrealiohookblocking.cpp{,.h}`, `Src/akaudioclasses.cpp`)

The three `import_reference.py` skeletons are real units now (`Sources.cmake`: only `akaudio.cpp` stays
excluded, its one function is in the generated registrant). 13 `Inc/CppText/<Class>.h` blocks declare the
methods, and `Inc/AkAudioWwise.h` (ours) brings the AK headers plus the three things that are Arkane's
rather than Audiokinetic's: the global game object id **2** (`RegisterGameObj(2, "Unreal Global")` at
`0x5b1430`), the codec/company ids, and the Unreal-to-Wwise vector conversion `(-X, Z, Y)` that
`UAkAudioDevice::SetListener` (`0x5aecb0`) performs.

`UAkAudioDevice::EnsureInitialized` (`0x5b1430`) is retail's order argument for argument: pool counts
(384 / 2048 in the editor), stream memory (`0x20000` / `0x80000`), device granularity `0x8000` and
`fTargetAutoStmBufferLength` 1.5, the `0x800000` / `0x1000000` engine pools, the speaker mask per
configuration override, the music engine, then the **eight effect plug-in ids and the Vorbis codec id in
retail's order** (115, 106, 105, 118, 130, 136, 131, source 101, codec 4), the bank directory, the global
game object and `LoadAllReferencedBanks`. The one place it is not retail-identical is marked in the body: the
plug-in *factories* are Audiokinetic's, so the registrations pass the ids with `NULL` factories.

The low-level IO is Arkane's own copy of the Wwise samples (which is why the 2012 PDB attributes
`CAkUnrealIOHookBlocking` and `CAkFilePackageLowLevelIO<CAkUnrealIOHookBlocking, CAkDiskPackage>` to Arkane's
units), so it is ours to write: `CAkFileLocationBase` (base path plus the DLC directories),
`CAkUnrealIOHookBlocking` (blocking positioned reads through `GFileManager`) and the flattened
`CAkFilePackageLowLevelIO` (one template instantiation exists in the exe). Its AKPK reader is documented from
measurement, not from a header — see `FAkFilePackage::Load` and section 6's sweep.

### Engine

* `UAkBank::Load` / `LoadAsync` / `Unload` / `UnloadAsync` (`0xc7210`, `0xc74f0`, `0xc7680`, `0xc7760`): the
  file package is `<name>.pck`, or `<name>.<language>` for a seek-free `BK_` bank — and the language there is
  remapped once per run, because POL, CZE and HUN all ship the Russian voice set. A `GenerateDefinition` bank
  outside the editor is *prepared* (metadata only) instead of loaded. The gate retail uses is
  `GEngine->bUseSound`: the decompile's `*((BYTE*)GEngine + 696) & 0x10` is exactly that bitfield bit.
* `UAkEvent::ComputeAkID` (`0xc7840`) is the name hash; `ComputeMaxRadius` (`0xc7aa0`) is the two-pass
  `QueryAudioObjectIDs` + widest `fMaxDistance`.
* `AActor::PostAkEvent` (`0x2cd730`) is ported; `SetRTPCValue`, `SetSwitch`, `SetState` and `PostTrigger` are
  `DISHONORED(written)` onto the matching device methods (their bodies were not decompiled, only their native
  rvas are known); `ActivateOcclusion` stays a documented stub.

### Deliberately not ported this wave

`UInterpTrackAkEvent` / `UInterpTrackAkRTPC` and their track instances (matinee audio),
`UActorFactoryAkAmbientSound` (editor only), `AAkAmbientSound::GameSave`/`GameLoad`,
`UAkAudioDevice::ApplyGameSettings` (needs `UDishonoredAudioSystem`'s RTPC ids), and everything of
`UDishonoredAudioSystem` beyond what agent AI wrote. Recorded in the status CSV as `pending`.

## 6. Numbers

All from the snapshot build `build/agentAN` (`build\agentAN_wt_build.cmd`, 792 units, 0 errors) and the
baseline `build/agentAN_off` (the same snapshot with `-DDISHONORED_WITH_WWISE=OFF`, 788 units, 0 errors).
Runs: `build_and_smoke.py --build-dir build/agentAN --no-build --exe-name DishonoredGame_AN.exe --log-name
agentAN.log --ini-dir build/agentAN/config --rhi null --skip-native OnlineSubsystemPC --milestone
"Initializing Engine..." --expect "Finished loading level" --expect "Initial startup" --expect "Committed map
change via DishonoredEngine" --forbid "Critical" "--extra-args=-forcelogflush -noscenerender"`.

| Check | before (`DISHONORED_WITH_WWISE=OFF`) | after (ON) |
|---|---|---|
| `Initializing Engine...` / `Finished loading level` / `Initial startup` | ok | ok |
| `Committed map change via DishonoredEngine` | ok | ok |
| `Critical` lines | 0 | **0** |
| `Warning:` / `Error:` lines | 165 / 71 | **165 / 71** (the ON-only set is empty) |
| `Initial startup` | 26.15 s | **6.80 s** (both figures are dominated by concurrent agent builds on this machine; the whole audio bring-up takes 0.05 s of wall clock, `[0035.57]`–`[0035.62]` in the first run) |
| audio log lines on the path | **1** (`DISHONORED(bringup): audio system DishonoredAudioSystem`) | **149** `Wwise:` lines plus that one |
| AKPK file packages registered | 0 | **65** |
| banks loaded / prepared / missing | 0 / 0 / 0 | **39 / 27 / 0** (plus `Init.bnk`) |
| banks whose BKHD version was not 65, or whose id did not match its name hash | — | **0 / 0** |
| `AkEvent` objects with a resolved Wwise id | 0 of 468 | **463 of 468** |

The 5 unresolved `AkEvent`s are class default objects, whose `PostLoad` never runs.

The summary line the device prints, verbatim:

```
Wwise: 65 AkBank objects -> 39 banks loaded, 27 prepared, 0 missing, 65 file packages; 468 AkEvent objects, 463 with a resolved id
Wwise: Loaded bank Init: id 0x50c63a23, BKHD version 65, bank id 0x50c63a23, 14052 bytes
Wwise: file package Bk_70D947D44D90C9FAF55C35A9554EA8A8.INT: AKPK v1, 1 banks, 479 streamed files
Wwise: bank directory ..\..\DishonoredGame\CookedPCConsole\
Wwise: StreamMgr::SetCurrentLanguage: English(US)
```

**About "the AkAudio warn-once stub lines": there were none.** The package brief expected warn-once stubs on
the map path; the baseline log proves the audio layer was silent by *omission*, not by warning —
`AkAudio` has no script natives at all, and the `DISHONORED(bringup)` bodies in `akbank.cpp` / `akevent.cpp`
returned `FALSE`/`0` without logging. So the honest before/after is the table above: **1 audio line becomes
149**, and 0 banks become 67 real bank loads with 0 failures.

**`PostEvent` is called 0 times on this path**, and that is not the surface's fault — it is two missing
wirings outside this package, both recorded in section 8: nothing ticks `UAkAudioDevice::Update` (retail:
`UWindowsClient::Tick`), and `UDishonoredAudioSystem::Update` / `RegisterAmbientSound` are DishonoredGame
stubs, so no `AAkAmbientSound` ever reaches `StartEvent`. The event path itself is complete and exercised by
the nine Kismet actions and `AActor::PostAkEvent`.

Other checks on the same build: `LayoutProbe` and `CoreSmoke` build and pass
(`xcheck_sdk_layout.py build/agentAN/layout_probe.txt`: 2,314 types, **0 rows, 0 contract mismatches**;
`CoreSmoke.exe`: **99 passed, 0 failed**). The layout probe links `Dishonored::wwise` and still links, which
is why the AK allocation hooks have a default inside the backend rather than only in `AkAudio`.

### Bank-format evidence (`build/agentAN/akpk_sweep.py`, read-only over the retail install)

```
packages 981 (bad magic 0, version != 1 0)
bank entries 981, streamed entries 3772
package base-name hash present in the bank table: 981 yes / 0 no
BKHD found at uStartBlock*uBlockSize: 981 yes / 0 no
bank versions: {65: 981}
Init.bnk BKHD version 65 bank id 0x50c63a23, fnv1('Init') 0x50c63a23 -> MATCH
```

The AKPK layout this confirms (and which `FAkFilePackage::Load` reads): `'AKPK'`, `uHeaderSize`,
`uVersion` (=1), the four table sizes (language map, soundbanks, streamed files, externals), then the four
tables; each of the soundbank and streamed tables is `uNumFiles` plus 20-byte entries
`{ AkFileID fileID; AkUInt32 uBlockSize; AkUInt32 uFileSize; AkUInt32 uStartBlock; AkUInt32 uLanguageID; }`;
data starts at `8 + uHeaderSize` and a file sits at `uStartBlock * uBlockSize`. Every retail package uses
`uBlockSize` 1, the externals table is always empty, and streamed media are `RIFF` (Vorbis WEM).

## 7. What real audio would require — the assessment wave 6 needs

Three routes, in order of cost:

1. **The licensed Wwise 2012.1 SDK (the user's blocker, `middleware.md` 4.2).** Free from Audiokinetic's
   Launcher ("Legacy versions"); the per-title licence was Bethesda's, so a private build is fine and a
   *distributable* recompiled exe is not. Work left after the install: set `DISHONORED_WWISE_SDK`, restore the
   nine plug-in/codec factories (`AkRoomVerbFXFactory.h` and friends — one `#include` and the real function
   names in `EnsureInitialized`), give `AkPlatformInitSettings::hWnd` the viewport's window so `AkSink`
   creates its DirectSound/XAudio2 output, and re-check the MSVC 9/10 CRT mismatch (`/NODEFAULTLIB` juggling,
   or wrap the libs in a small MSVC-9 DLL). The `AkAudio` port itself is unaffected: it is written against
   these headers, whose shapes are the SDK's. **This is by far the cheapest route to real sound and the one
   to recommend.**
2. **Reimplement the engine-facing subset on an open runtime.** The banks' media is Vorbis in WEM
   containers, which is decodable, and this package already reads AKPK and BKHD. What it would *not* give is
   the mix: Dishonored's audio is bank logic — event action lists, attenuation and RTPC curves, state and
   switch containers, the interactive-music hierarchy. Parsing `HIRC` far enough to evaluate a simple event
   ("play this sound at this attenuation on this object") is a realistic wave; reproducing the mix is not.
   Estimate: `HIRC` + `STMG` readers and a minimal voice/attenuation/RTPC evaluator on top of an open mixer,
   several waves, and the result is *a* mix rather than *the* mix.
3. **Rewrite the 3,300-function runtime from the decompile.** Not worth considering.

Whichever is chosen, **this package is the prerequisite and not wasted work**: the headers are the SDK's
shapes, so route 1 is a cmake switch, and route 2 replaces only the backend behind an API the game already
compiles and runs against.

## 8. Follow-ups outside this package

1. **Nothing ticks the audio device.** Retail's `UWindowsClient::Tick` calls `UAkAudioDevice::Update`, which
   calls `UWorld::m_pAudioSystem->Update(device, components)`. `UClient` has no `GetAkAudioDevice` virtual in
   this tree (the engine's own body is the pure-virtual logger at `0x1e68e0`, `UWindowsClient` supplies the
   object), so `UAkAudioDevice::Get()` is a `DISHONORED(bringup)` lazy construction and `Update` is never
   called. Wiring `UClient::GetAkAudioDevice` (vtable slot 320) and the `UWindowsClient` tick is a small
   WinDrv/Engine change that no caller in this package would notice.
2. **`UDishonoredAudioSystem` is where events start.** `RegisterAmbientSound` (2013 `0x793460`) appends
   `{ AAkAmbientSound*, MaxRadius², (MaxRadius+100)² }` — the start/stop hysteresis — and `Update`
   (`0x7b3520`) walks the listener's cell and calls `StartEvent`/`StopEvent`. Until those two exist no
   ambient sound plays, which is why `PostEvent` is 0 on the map path. `ConsumeEndOfEventNotifies`
   (`0x7a9970`) and `PostAkEventAtPoint` (`0x7a95c0`) belong with them; all four are decompiled in
   `build/agentAN/dec2013`.
3. **`UAkAudioDevice::ApplyGameSettings`** (`0x5b3e50`) pushes the four volume sliders into their RTPCs and
   the global volume into the Bink player; it needs `UDishonoredAudioSystem::Init`'s RTPC ids (which agent AI
   already documented) and `FFullScreenMovieSupport` to have a volume member.
4. **`GetAkComponent`** leaves out retail's read of the actor's fade value at `@256` (below 1e-8 it pauses
   the freshly posted event); that member is not in our `AActor` yet.
5. **The matinee audio tracks** (`UInterpTrackAkEvent`, `UInterpTrackAkRTPC`) are the last `AkAudio` classes
   without bodies; the map path's matinees would drive them.
6. **`middleware.md` section 2.4 is now out of date** (it says "audio is a stub" and recommends option (c)).
   I deliberately did not edit it: `middleware.md` is being edited concurrently by the PhysX and Steamworks
   packages of this wave, and a three-way clobber of a shared doc is worse than a stale paragraph. The
   coordinator should fold section 1, 3 and 7 of this report into it at merge.

## 9. The shared tree (read before merging)

The shared working tree **did not configure** while this package was written: another agent had turned
`WITH_NOVODEX` on and `source/Development/Src/External/PhysX284` plus ~100 Engine physics units were red
(≈2,000 errors, none of them in this package's files). Per the wave rules the work was therefore verified in
the snapshot `build/agentAN_wt` = HEAD + this package's 43 files, and the two shared cmake files are
**deliberately not overlaid** into the snapshot — `build/agentAN/patch_defines.py [--root <tree>]` re-adds
only this package's lines and is idempotent, so it can be re-run on the merged tree:

* `CMakeLists.txt`: the `DISHONORED_ENABLE_AKAUDIO` option moved above a new `include(cmake/Wwise.cmake)`;
* `cmake/DishonoredDefines.cmake`: the `Dishonored::wwise` block at the end of `dishonored_apply_defines`.

`cmake/DishonoredDefines.cmake` was already overwritten once mid-session by another agent's full-file write,
which silently dropped the Wwise block; that is what `patch_defines.py` exists for. Everything else of this
package lives in files no other package touches.

One hand edit sits in a generated header: `AkAudioClasses.h` gained `#include "AkAudioWwise.h"` and the 13
`#include "CppText/<Class>.h"` lines. Those are exactly what `gen_classes_header.py --sdk` emits when the
CppText files exist, so a regeneration reproduces them — except the `AkAudioWwise.h` line, which is marked in
place and re-added by `build/agentAN/patch_classes_header.py`.
