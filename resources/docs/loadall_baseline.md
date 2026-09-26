# Load-all baseline — the milestone 3 exit check over the whole retail content tree

Agent AX, 2026-09-26. Rows: `loadall_baseline.csv` (974 packages, one row each). Measured on the Release build of
HEAD `a64f9b4` in the clean snapshot worktree `build/agentAX_wt` (`build/agentAX_wt/build/agentAXR`), null RHI,
`-skipnativepkgs=OnlineSubsystemPC`, `-forcelogflush`, `-loadallpurge`.

Reproduce:

```
python resources/tools/debug/loadall_sweep.py upk  --build-dir build/agentAX_wt/build/agentAXR --timeout 3600
python resources/tools/debug/loadall_sweep.py pck  --build-dir build/agentAX_wt/build/agentAXR --dirs all
python resources/tools/debug/loadall_sweep.py report --build-dir build/agentAX_wt/build/agentAXR
```

## Result

| kind | packages | objects / contained files | load errors | teardown crashes |
|---|---:|---:|---:|---:|
| `.upk`, `DishonoredGame/CookedPCConsole`, through `UObject::LoadPackage` | **471** | **524,807** | **1** | **30** |
| `.pck`, CookedPCConsole + DLC05 + DLC06 + DLC07, through the AKPK header parser | **503** | **2,411** | **0** | n/a |

The `.upk` sweep is 471 `LoadPackage` calls in one process (31 processes in practice, because a teardown crash ends
one), 115 s wall, 7.9 s of it inside the loads. The live-object count after each collect stays inside
65,619..89,149, i.e. the purge really reclaims: without `-loadallpurge` (agent AD's `RF_Standalone` keep flags) the
32-bit address space is gone after a few dozen packages.

Largest packages by objects created: `Engine` 20,032, `DishonoredGame` 19,472, `L_Isl_Geom_Slave` 11,458,
`L_Isl_Geom_Master` 11,130, `L_Prison_Env` 9,861, `L_Streetsewer_Block` 9,408, `L_Pub_Day_Env` 9,100. Two packages
create no object: `GlobalPersistentCookerData` (a cooker bookkeeping package with no exports — correct) and
`OnlineSubsystemPC` (below). `L_Tower_Env` loads 5,089 objects, identical to agent AD's wave-4 number, which
cross-checks the counter.

**Milestone 3 is exited**: every cooked `.upk` of the retail content tree deserializes without a single linker abort
— no `Bad export index`, no `Bad name index`, no `Serial size mismatch`, no `Serialize` assertion, in 524,807
objects across 471 packages. That was the open item in `STATUS.md`'s "Next".

## Why `.pck` is not a `-loadall` package

The 503 `.pck` files are **Wwise 2012.1 file packages**, magic `AKPK`, read by the AkAudio low-level IO
(`AkFilePackage`), never by `UObject::LoadPackage` — pointing `-loadall` at one would only prove that the package
search path does not resolve it. They are verified instead by parsing the header the engine itself parses: magic,
version 1, the four section sizes (language string map, soundbank LUT, streamed-file LUT, external LUT) and every
lookup-table entry's `startBlock * blockSize + fileSize` against the file length. All 503 parse, 2,411 contained
files, 0 errors. The engine agrees: a run's log carries `Wwise: file package <name>.pck: AKPK v1, 1 banks, N
streamed files` for each one it opens.

## Triage of every error

### 1 load error — `OnlineSubsystemPC`: benign, self-inflicted

`Can't bind to native class OnlineSubsystemPC.OnlineSubsystemPC`. `OnlineSubsystemPC.upk` is a **native** script
package whose registrant this build does not link, which is exactly why every run in the project passes
`-skipnativepkgs=OnlineSubsystemPC` (`STATUS.md`'s run row, `appGetScriptPackageNames`). With the switch on, the
class exists in the package but has no C++ counterpart to bind to, so `UStruct::Link` raises. Loading it is not a
serializer failure and nothing in the game loads it. **No package owns this**; it disappears when the
`OnlineSubsystemPC` module is generated, or stays a documented skip.

### 29 teardown crashes — one out-of-bounds `delete` in `UMaterial::FinishDestroy`

Every one of these 29 packages **loads cleanly** (its `loadall <pkg>: N exports, 0 errors` line is in the log) and
then faults during the garbage collect that frees it. `resources/tools/debug/dbgrun.py` on three of them
(`L_ArtDealer_Block`, `L_PrsnSewer_P`, `L_Streets1_Light`) gives the same stack and the same faulting address:

```
EXCEPTION 0xc0000005 at ?FinishDestroy@UMaterial@@UAEXXZ+0x56 (read at 0xfffffcfd)
    UMaterial::FinishDestroy+0x56
    UObject::IncrementalPurgeGarbage+0x228
    UObject::CollectGarbage+0xb66
    DishonoredLoadAllPackages+0x2a6
```

`Engine/Src/Material.cpp:1925` `UMaterial::FinishDestroy` deletes `DefaultMaterialInstances[0]`, `[1]` **and `[2]`**,
but the member is `class FDefaultMaterialInstance* DefaultMaterialInstances[2]` — the retail / 2012 PDB layout
(`Engine/Inc/EngineMaterialClasses.h:3949`, two quality levels). Index 2 is one DWORD past the array, i.e. the next
member `INT EditorX`, so the code runs `delete (FDefaultMaterialInstance*)EditorX` on a value deserialized from the
package. `0xfffffcfd` is `EditorX == -771`: a material editor graph coordinate. Reference UE3 has three instances
(normal, selected, hovered) and `UMaterial::UMaterial` still writes `[2]` under `GIsEditor`; Arkane's two-element
array is what the layout probe and `native_class_sizes.csv` pin. Only packages that carry a material with a
non-zero `EditorX` fault, which is why it is 29 packages and not 471.

**It is not caused by `-loadallpurge`**: the same stack appears under `UObject::StaticExit` at ordinary process
shutdown with the switch off (`build/agentAX/dbg/nopurge_exit.txt`). The sweep only finds it 471 times faster.

Fix (one line, and the constructor already does it right for `MaterialResources`):

```cpp
for( INT i = 0; i < ARRAY_COUNT(DefaultMaterialInstances); i++ ) { delete DefaultMaterialInstances[i]; DefaultMaterialInstances[i] = NULL; }
```

**Owner**: `Engine/Src/Material.cpp` belongs to no wave-5 package (`PHASE7.md` gives AR the D3D9 and texture units,
not the material units). Coordinator item, or AR by extension; AX did not edit it. Affected packages, for the
after-check: `L_ArtDealer_Block`, `L_Boyle_Int_Fx`, `L_Bridge_Part1a_Env`, `L_Bridge_Part1c_Env`, `L_Distillery_P`,
`L_Distillery2_P`, `L_Flooded_FGate_Env`, `L_Flooded_FRefinery_Env`, `L_Flooded_FRefinery_Env2`,
`L_Flooded_FStreets_Env`, `L_Galvani1_Block`, `L_Isl_Script_Master`, `L_Isl_Script_Slave`, `L_LightH_LowChaos_Geom`,
`L_Out_HighChaos_EmilyAlive_Env`, `L_Out_HighChaos_emilyDead_Env`, `L_Out_HighChaos_emilyDead_P`,
`L_Ovrsr_Back_Env`, `L_Ovrsr_Light`, `L_Prison_Script`, `L_PrsnSewer_geo`, `L_PrsnSewer_P`, `L_PrsnSewer_Script`,
`L_Streets1_Light`, `L_Streets2_Script`, `L_Streetsewer_Block`, `L_TowerRtrn_Int_Env`, `L_TowerRtrn_Int_Script`,
`L_TowerRtrn_Yard_Script`. (`L_Out_HighChaos_emilyDead_P` reports `Error reentered: Illegal call to
StaticFindObject() while serializing object data or garbage collecting!` — the same fault, caught one frame later by
the error handler.)

### 1 teardown crash — `LEVEL`: a divide by zero in `FTexture2DResource::GetData` → **agent AR**

```
EXCEPTION 0xc0000094 (integer divide by zero) at ?GetData@FTexture2DResource@@AAEXIPAXI@Z+0xbd
    FTexture2DResource::GetData+0xbd
    FTexture2DResource::InitRHI+0x603
    FRenderResource::InitResource+0xab
    FRenderingThread::Run+0x4b
```

`Engine/Src/Texture2D.cpp:2590` divides by `GPixelFormats[EffectiveFormat].BlockSizeX`. The only row of
`GPixelFormats` (`Engine/Src/UnRenderUtils.cpp:83`) with `BlockSizeX == 0` is `PF_Unknown`, so a `UTexture2D` in
`LEVEL.upk` reaches `InitRHI` with `Format` (or `UTexture2D::GetEffectivePixelFormat`'s result) equal to
`PF_Unknown`, and the mip column count divides by zero on the rendering thread. The package itself loads cleanly.

**Owner: AR.** `Texture2D.cpp` and `FTexture2DResource::InitRHI` are named in its package (`PHASE7.md` AR step 3),
it is the same function family as the corrupt-texture defect, and AR's own texture census already reports
`6 pitch mismatches` in the `L_Tower_P` run. The cheap next step is one census line naming the texture and its
`Format`, then either the guard retail has or the format mapping our `EPixelFormat` order gets wrong.

## What is not covered

The 434 `.upk` under `DishonoredGame/DLC/PCConsole/DLC05..07` are **not** in this baseline: the plan's exit check is
the 471 cooked packages, and the DLC packages are mounted (`DevDlc: Found DLC dir DLC07 … 161 package files`) but
never loaded by the bring-up path. `loadall_sweep.py upk --dirs all` sweeps them when someone wants that number.
