# Reference engine source: CodeRedModding/UnrealEngine3

Cloned 2026-09-25 to `D:\RecompileDishonored\UnrealEngine3` (outside the repo; never copied in
wholesale). Commit `601d6a1` (2026-03-05).

| Property | Reference tree | Dishonored 2012 (Shipping PDB) | Dishonored 2013 (retail) |
|---|---|---|---|
| Engine version | **10897** | 9014 | 9411 |
| Changelist | 1532151 (Epic), 2013-02-13 | 254295 (Arkane), 2012-06-20 | 334700 (Arkane), 2013-08-20 |
| Content | full `source/Development/Src` C++ (2,339 files), 2,014 `.uc`, UBT, VC80/VC90 solutions, UDK/UTGame sample game | binaries + PDB only | binaries only |

Same engine generation (UE3, 2012–2013). The reference is ~8 months of Epic changes newer than
Dishonored's engine branch and contains none of Arkane's modifications. It is the starting point
for every engine module; Dishonored's deltas are recovered from the PDB-named decompile.

## File-level overlap (Shipping PDB source files present in the reference tree)

| Module | Present / PDB files | Notes |
|---|---:|---|
| engine | 467 / 577 | missing files are Arkane additions (e.g. `Unity_*` batches are build artifacts, not sources) |
| core | 112 / 118 | |
| d3d9drv | 21 / 21 | complete |
| ipdrv | 16 / 16 | complete |
| windrv | 6 / 6 | complete |
| gameframework | 17 / 26 | |
| launch | 3 / 3 | complete |
| onlinesubsystemsteamworks | 4 / 6 | |
| gfxui | 5 / 21 | Arkane extended GFxUI heavily |
| dishonoredgame | 0 / 1058 | game module: nothing to reuse |
| akaudio | 0 / 6 | Wwise glue: not in reference (reference has XAudio2/ALAudio/CoreAudio) |
| edge, disjobs, ps3 | 0 | Arkane/PS3-specific |
| **total** | **651 / 1884** | |

## Function-level overlap (PDB `Class::Method` with a same-named definition in the reference)

Rough lower bound (simple regex over the reference sources; template and inline definitions are
under-counted).

| Module | Same-named definition | Share |
|---|---:|---:|
| core | 1,371 / 2,066 | 66 % |
| windrv | 54 / 68 | 79 % |
| ipdrv | 56 / 93 | 60 % |
| d3d9drv | 134 / 235 | 57 % |
| launch | 4 / 9 | 44 % |
| engine | 6,558 / 15,846 | 41 % |
| gfxui | 193 / 578 | 33 % |
| onlinesubsystemsteamworks | 48 / 161 | 30 % |
| gameframework | 97 / 427 | 23 % |

The Engine gap is mostly script-generated `exec*` thunks for Arkane's engine-level `.uc`
classes and Arkane's own additions; the core engine systems (object system, linker, GC, renderer,
physics glue) are present.

## What the reference tree provides

* Full source for Core, Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, D3D11Drv, GFxUI (engine
  side), OnlineSubsystemSteamworks/PC/Live, Launch, XAudio2, UnrealEd and tools.
* `Development/External`: DirectX9, Novodex (PhysX 2.8 SDK headers), libPNG, libogg,
  libtheora, nvTriStrip, nvtt, wxWindows 2.4.
* UnrealBuildTool and VC80/VC90 solutions (not used: this project builds with CMake + MSVC 2022).
* A working `UnNames.h`, `UnObjVer.cpp`, native registration macros, `UStruct::Link`, package
  loader — the exact contracts Phase 2/3 must preserve, in readable form.

## What it does not provide

* Scaleform GFx SDK sources/libs (only the GFxUI glue module; `GFxUI.h` includes SDK headers
  that are not in the tree). `Binaries/GFx` holds the Scaleform tools (gfxexport, AMP), not the SDK.
* Wwise SDK (Dishonored's `AkAudio`), FaceFX SDK (only the 6 `Engine/FaceFX` glue files),
  Steamworks SDK headers, LZOPro. Bink: only `Binaries/Win64/binkw64.lib`, no header, no Win32 lib.
* PhysX: headers and Win32 `.lib`s for the SDK's own version (`Development/External/Novodex`);
  Dishonored ships PhysX 2.8.4-era DLLs, so the versions must be reconciled in Phase 4.
* Any Arkane change: DishonoredGame (21.8k functions), engine modifications, Edge, DisJobs.

Regenerate the overlap tables with `python resources/tools/symbols/xref_reference.py` (Phase 2 deliverable).
