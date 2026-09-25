# Agent G — Engine compiles against the converged Core headers (2026-09-25)

Build dir: `build\agentG` (Ninja, Debug, x86 toolchain, `-DDISHONORED_ENABLE_ENGINE=ON`, stock
options: `/Zp4` global, `DISHONORED_LAYOUT_CHECKS` at its default). Driver: `build\agentG\build.cmd
[configure] <logname>`; analysis: `python build\agentG\analyse.py build\agentG\<logname>.log`
(errors grouped by code / message / file). Logs: `build\agentG\configure.log`, `build1.log`,
`build2.log`.

## Result

BUILD_STATUS_PLACEHOLDER

## What actually broke in Engine

Agent B's heads-up listed 7 Engine files "using `NetIndex`" (`UnLevel.cpp`, `UnChan.cpp`,
`UnEngine.cpp`, `UnScript.cpp`, `UnInterpolation.cpp`, `UnParticleComponents.cpp`,
`ParticleEmitterInstances.cpp`). Grepping them shows every one of those hits is already either
`X->GetNetIndex()` (the reference accessor, which agent A re-implemented on top of
`RF_DisNetIndexed`) or `FFieldNetCache::FieldNetIndex` (a different, still-existing member). No
Engine unit reads or writes the `UObject::NetIndex` member directly, so **no NetIndex edit was
needed** and none was made. `ClassGroupNames`, `bFixupExportMapDone`, `LocalVarsOwner` and
`UPackage::FileName` are likewise unused in Engine (`UnPatchCommandlets.cpp`'s `Package.FileName`
is `FPackageSummaryInfo::FileName` from `UnrealEd/Inc/UnPatchCommandlets.h`, its own `FString`).

The only real breakage is `FPackageInfo::FileName` (removed by agent A; PDB `FPackageInfo` is 68
bytes: `ForcedExportBasePackageName` @48 is directly followed by `Extension` @56, confirmed with
`gen_layout_probe.py show FPackageInfo`) in three units.

## Sites changed (all `source/Development/Src/Engine/Src`, each with `// DISHONORED(layout)`)

| File | Site | Change |
|---|---|---|
| `UnConn.cpp` | `UNetConnection::ParsePackageInfo` | `Info.FileName = FName(*FileName)` dropped; the `FileName` string is still received from the `NMT_Uses` bunch (8-parameter message unchanged) and discarded |
| `UnConn.cpp` | `UNetConnection::SendPackageInfo` | `FString FileName(Info.FileName.ToString())` → `FString FileName(FName(NAME_None).ToString())`, i.e. exactly what the reference sends when `FileName == NAME_None` ("None"); the wire format is untouched |
| `UnPenLev.cpp` | `NMT_Uses` handler (`UPendingLevel`) | log line loses the `FileName: %s` field; the `GetPackageNameToFileMapping()->Set(PackageName, FileName)` block removed (dead: `FileName` is always none); `PackageFileToLoad = Info.PackageName`; `GUseSeekFreeLoading && Info.FileName == NAME_None` → `GUseSeekFreeLoading` (both occurrences); the `if (Info.FileName != NAME_None) Info.Parent->Guid = Info.Guid;` block removed (dead) |
| `UnWorld.cpp` | `NMT_Uses` handler (`UWorld::NotifyControlMessage`) | log line loses the `FileName: %s` field |

Semantics chosen: every site collapses to the reference's `FileName == NAME_None` branch, which
is the only branch a Dishonored `FPackageInfo` can take. No other behavior changes.

Decompile check: the shipping PDB has **no** `UNetConnection::ParsePackageInfo`,
`UNetConnection::SendPackageInfo`, or `FNetControlMessage<7>` (`NMT_Uses`) function (grep of
`resources/docs/symbols/functions.csv` for `PackageInfo` finds only `FPackageInfo` ctors, the
`TArray<FPackageInfo>` instantiations, `UPackageMap::AddPackageInfo` and the `FDisTexturePackageInfo`
family), so Dishonored's shipped exe never ran this code and there was nothing to decompile; the
edits therefore follow the reference's own logic minus the member. No IDA copy was made.

## Core-related, left alone

- Nothing. Core (all units, `DishonoredLayouts.h` static_asserts active, `/Zp4` global) compiled
  with 0 errors in this build dir; no assert fired.
- `Core/Inc/UnObjBas.h` still defines `RF_CookedStartupObject` with the same value as
  `RF_DisNetIndexed` (bit 63). Engine does not reference `RF_CookedStartupObject`, so it is
  harmless here, but agent A may want to retire the alias.
