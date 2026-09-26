# Agent AD — streaming serialization: nav mesh, `-loadall` sweep of the main-menu and `L_Tower_P` packages (2026-09-26)

Package "### AD" of `PHASE6.md`. Build dirs: `build\agentAD` (working tree, used until other agents' in-flight edits broke it:
AJ's `UDishonoredNativeStateMachine.h` / AG's `MaterialShader.cpp`), then the snapshot `build\agentAD_wt` (`git worktree add
--detach build/agentAD_wt HEAD` at 571bd0e + my files) with its own build dir `build\agentAD_wt\build\agentAD` (no junction, no
stage dir). IDA copies `resources/docs/idb/retail2013_agentAD.i64`, `shipping2012_agentAD.i64`; decompiles in `build\agentAD\dec2013`,
`dec2012` (headless `decompile_funcs.py`). Patch scripts `build\agentAD\patch_navmesh.py`, `patch_navmesh2.py`; sweep driver
`build\agentAD\loadall_driver.py`; package lists `build\agentAD\menu.txt`, `tower.txt`. Status rows: `agentAD_status.csv`.

## Result

| Check | Before (HEAD 4035b87, `build\agentAD` build0) | After |
|---|---|---|
| null-RHI smoke through the main-menu map change | dies 0.5 s after `Initial startup`: `Bad export index 1065353215/6389 [Dishonored_MainMenu_Env.upk, NavigationMeshBase DisPylon_0.NavigationMeshBase_4469]` | `[0006.02] Log: Committed map change via DishonoredEngine Transient.DishonoredEngine_0` (golden :365), `Dishonored_MainMenu_Env` / `_FX` added to the world |
| `-loadall` main menu (`menu.txt`: `Dishonored_MainMenu`, `_Env`, `_FX`) | — | 3 packages, **0 errors** |
| `-loadall` `L_Tower_P` + its 8 streaming levels (`tower.txt`) | — | 9 packages, **0 errors** |
| `Bad export index` / `Serial size mismatch` in `agentAD.log` | 1 | **0** |
| `sizeof(UNavigationMeshBase)` | 688 (retail 464) | **464** (`checkAtCompileTime` in `UnNavigationMesh.cpp`) |
| `Engine.lib` / exe | 1.191 GB / 65.4 MB | 1.189 GB / 65.4 MB (the shims are shared `static`, not `inline static`) |

The accept command exits 0 on `build_wt10` with all four checks (`Initial startup`, `Committed map change via DishonoredEngine`,
no `Bad export index`, no `Serial size mismatch`; see the timing note in §5). The process itself still ends with exit code 3 right after the commit line: an `appErrorf` on the
**rendering thread** in `FMaterialInstanceResource::GetMaterial` (see hand-overs); that is the renderer's material shader maps
(AG), not a serializer, and it happens after the milestone line and after the `-loadall` sweep (which runs before the map change).

## 1. The blocker

Frame chain (agent AK's `dbgrun.py` on the baseline, confirmed by the coordinator): `ULinkerLoad::operator<<(UObject*&)` <-
`FActorReference` <- `FCoverReference` <- `TArray<FCoverReference>` <- `operator<<(FNavMeshPolyBase&)` <-
`UNavigationMeshBase::Serialize`. The reference poly serializer reads a `TArray<FCoverReference> PolyCover` for `Ver() >= 586`;
retail `operator<<(FArchive&, FNavMeshPolyBase&)` (2013 rva 0x2780f0 = 2012 0x2928c0, identical bytes) reads a dummy
`TArray<BYTE>` there only for `LicenseeVer() < 27` and goes straight to `PolyHeight` for Dishonored's licensee 30. So the first
`PolyHeight` float (0x3F7FFFFF = 1065353215) was taken as the cover array count and the next floats as object indices.

## 2. What changed (per file, evidence)

### `Engine/Inc/UnPath.h`
- `operator<<(FArchive&, FNavMeshPolyBase&)`: retail shape (dummy `TArray<BYTE>` for 586 <= Ver, licensee < 27; `PolyHeight` from 588). 2013 rva 0x2780f0.
- `FNavMeshEdgeBase`: `FLOAT EdgeLength` added (2012 PDB @24; read by 2013 `FNavMeshEdgeBase::Serialize` 0x279d40 for every nav version); both constructors initialise it (`FNavMeshEdgeBaseConstructor` 2013 rva 0x272640 stores -1.0 in EdgeLength and EffectiveEdgeLength).
- Nav mesh versions: `VER_DIS_IMPORTED_MESH_OFFSET 28` = `VER_LATEST_NAVMESH` (retail ctor 2013 rva 0x294fb0 / 2012 0x2aeac0 stores 28; the reference 29..43 never appear in cooked data); `VER_MIN_PATHING` = 23 (`APylon::PostBeginPlay` 2013 rva 0x25f8b0 / 2012 0x279cd0 clears `bPathsRebuilt` when `VersionAtGenerationTime < 23`; the reference 43 would have flagged every retail mesh as out of date).
- `UNavigationMeshBase` to the 464-byte 2012 PDB layout (retail `native_class_sizes.csv` 464; the class is not in the SDK dump): member order `Verts @56 … BorderPolys @164, SavedSessionID @180, bMeshHasBeenCleanedUp @184, BorderEdgeSegments @188, ActiveEdgeToHandleMap @200 (60, storage only: the reference marks active edges through ExtraEdgeCost), BoxBounds @260, PolyOctree @288, VertHash @292, KDOPInitialized DWORD @296, NavMeshVersionNum @300, VersionAtGenerationTime @304, KDOPTree @308 (ours 24, 2012 28; LocalToWorld is 16-aligned to 336 either way), LocalToWorld @336, WorldToLocal @400`. Reference-only members: `DynamicEdges`, `IncomingDynamicEdges`, `PolyObstacleInfoMap`, `SubMeshToParentPolyMap`, `SubMeshPolyIDToLinkeObstacleMap`, `DynamicSnapRevertData` are shared `static` shims (one definition in `UnNavigationMesh.cpp`; see the note there on why not `DISHONORED_SHIM_STATIC`); `bNeedsTransform`, `bNeedsObstacleRecompute`, `bSkipDynamicSnapRevert` are bits of the `KDOPInitialized` DWORD (per instance, so dynamic pylons still transform correctly); the `OwningPylon` cache is gone, `GetPylon()` = `Cast<APylon>(GetOuter())` as 2013 `Serialize` does (rva 0x2909e0), and the five inline users take a local.
- `AddDynamicCrossPylonEdge<T>` (header template) gated with `DISHONORED(bringup)` (warn once, no edge): dynamic edges would live in the shared `DynamicEdges` shim; with the producers gated the shared containers stay empty, so `FinishDestroy`/`CleanupMeshReferences` on level unload never touch another mesh's data.

### `Engine/Src/UnNavigationMesh.cpp`
- `UNavigationMeshBase::Serialize` ported from 2013 rva 0x2909e0 (2012 0x2b0000, 1915 bytes, same code): reference-collector branch emits only the same-package cross-pylon pylon refs and `DropEdgeMesh`; load order `NavMeshVersionNum`, `VersionAtGenerationTime` (>= 11, `FPathBuilder::LoadedPathVersionNum` max), `Verts`, `EdgeStorageData`, `Polys`, dummy object ref below 7, `LocalToWorld`/`WorldToLocal` >= 8, `BorderEdgeSegments` >= 9 **only when the outer `APylon` is not `bStatic`** (AActor bit @288 mask 1; the reference's `VER_EDGE_WIDTH_GENERATION` clause does not exist), `ConstructLoadedEdges`, `BuildBounds` below 12 / `BoxBounds` from 12, every edge. Then the Arkane tail (loading, version < 28, outer pylon `bImportedMesh` @860 mask 1): the `LocalToWorld` translation is baked into the verts, poly bounds/centers rebuilt (`ExpansionPolyBoundsDownOffset`), edge centers updated, the offset stored in `APylon::m_ImportedMeshOffset` (@896), transforms reset to identity, `BuildBounds`. The 2013 exe asks the pylon (vtable +988) for each poly's up vector; ours uses `PolyNormal` and logs a `DISHONORED(bringup)` line when the branch runs (it did not run for any swept package: all cooked meshes are version 28).
- `FNavMeshEdgeBase::Serialize` (2013 0x279d40): `SerializeEdgeVerts`, `Poly0`, `Poly1`, `EdgeLength`, `EffectiveEdgeLength` from nav version 10 (else `EffectiveEdgeLength = EdgeLength`, non-cross-pylon edges recompute `EdgeLength`), `EdgeCenter`, `EdgeType`; no `EdgeGroupID`/`EdgePerp` (reference 30/41 data does not exist; `EdgePerp` zeroed).
- `FNavMeshCrossPylonEdge::Serialize` (2013 0x279fc0, identical to 2012): `Poly0Ref`/`Poly1Ref` from nav version 4 (`FPolyReference` with the < 620 fix-up), no `ObstaclePolyID`, `EdgeLength` recomputed below 10.
- `FNavMeshDropDownEdge::Serialize` (2013 0x27e5b0): no `EdgeType = GetEdgeType()` after the stream.
- `FNavMeshEdgeBase(Mesh, V1, V2)` initialises `EdgeLength`; `GetPylon()` without cache; `checkAtCompileTime(sizeof(UNavigationMeshBase) == 464)`; the single definitions of the six shared shim members.
- The six reference-only container shims are plain `static` members with one definition here, **not** `DISHONORED_SHIM_STATIC` (= `inline static`): as inline statics each container's ctor/dtor/atexit registration and its full Debug type info were emitted in every one of the 19 translation units that include `UnPath.h`, which grew `Engine.lib` from 1.191 to 1.266 GB and made `link.exe` abort the ~1.3 GB link with an access violation (exit code 0, truncated exe). With plain `static` the library is 1.189 GB, i.e. below the pre-change size, and the exe links at the normal 65.4 MB. Same sharing semantics, same (zero) effect on `sizeof`. `patch_navmesh3.py`.
- Gates (`DISHONORED(bringup)`, warn once, return): `BuildSubMeshForPoly` and `IInterface_NavMeshPathObstacle::RegisterObstacleWithNavMesh` (the reference obstacle sub-mesh system; Arkane's path objects use `FNavMeshPathObjectEdge`, 2013 rva 0x27ad50).
- Verified identical (rows in `agentAD_status.csv`): `SerializeEdgeVerts` (both), `FNavMeshPathObjectEdge::Serialize`, `ConstructLoadedEdges` 0x290820, `InitializeEdgeClasses` 0x28d0f0 (six classes; ours also registers the three reference-only `SpecialMove`/`Mantle`/`CoverSlip` classes, harmless), `PopulateEdgePtrCache`, `Cache`, `UpdateEdgeCenter` 0x26ac40, `operator<<(TArray<FMeshVertex>&)` 0x283810.

### `Engine/Src/DishonoredLoadAll.cpp` (new) + `Launch/Src/LaunchEngineLoop.cpp` (5-line hook after `Initial startup`)
`-loadall=<pkg+pkg|@listfile>`: `UObject::LoadPackage` per package, one line
`DISHONORED(bringup): loadall <pkg>: <N> exports, <E> errors (<C> objects created, <T>s)`, then `appRequestExit(FALSE)`. `N` is the
linker's `ExportMap.Num()` when the package still has a linker, else the number of objects whose outermost is the package; after a
completed synchronous `LoadPackage` the `ULinkerLoad` is already detached (`GetLinker()` returns NULL), so in practice both `N` and
`C` are that object count — which is what the table below reports. A linker abort (`Bad export/import/name index`, `Serial size mismatch`) is an
`appErrorf`, which under `GIsGuarded` runs `UObject::StaticShutdownAfterError` and throws (`FOutputDeviceWindowsError::Serialize`);
the tool catches it, logs the package line with 1 error and the `GErrorHist` text (package, object, class from the coordinator's
linker diagnostics) and exits with `appRequestExit(TRUE)` because the object system is shut down. `build\agentAD\loadall_driver.py
<list> --build-dir <dir>` restarts the sweep at the next package until the list is done and prints the per-package table and the
per-class error summary. The `@file` path is resolved by the exe (cwd = retail `Binaries\Win32`), so pass it absolute:
`--extra-args=-loadall=@D:/RecompileDishonored/Recompile/build/agentAD/tower.txt` (note the `=` form: argparse rejects a value
starting with `-` otherwise).

### `Engine/Src/UnLevel.cpp`, `Core/Inc/UnIOBase.h`, `Core/Src/UnAsyncLoading.cpp` — no change needed
- `ULevel::Serialize` (agent Z's port) re-checked against 2013 rva 0x259f30 (identical to 2012 0x2737b0): gates 771 (`DynamicTextureInstances`), 681 (APEX blob skipped), 690, 585, 607, 780, 799 and licensee 27 all match; the reference 797/798 gates named in the plan are already gone.
- `FAsyncIORequest`: ours is the 2012 layout (64 bytes; the header comment already documents the 2013 `NormalizedFileName` @24 / 76 bytes and the handle cache keyed by it, rva 0x74130). 2013 `QueueIORequest` (rva 0x519b0) fills the same fields. No async-IO failure (`~FAsyncIORequest`, `FindCachedFileHandle`) appeared in any run of this wave (map change, 12 loadall packages), so the 2013 cache is not ported.
- The `StaticMeshComponent` 306/302 mismatch agent Y saw on D3D9 did not reproduce: no `Serial size mismatch` in the null-RHI map change or in the 12-package sweep (agent AA's `FStaticMeshComponentLODInfo` port covers it).

## 3. Nav mesh layout table (retail = 2013 native size; 2012 = PDB `types.json`; ours = after this wave)

| Type | retail / 2012 | ours before | ours now | note |
|---|---|---|---|---|
| `UNavigationMeshBase` | 464 / 464 | 688 | **464** | member order above; 6 shims, 3 bits |
| `FMeshVertex` | – / 40 | 40 | 40 | X Y Z + `PolyIndices` serialized (2013 0x283810) |
| `FEdgeStorageDatum` | – / 16 | 16 | 16 | |
| `FPolyReference` | 24 / 24 | 24 | 24 | agent AB |
| `FNavMeshEdgeBase` | – / 52 | 112 | 116 | + `EdgeLength`; the reference edge keeps its A* bookkeeping (`bAlreadyVisited`, `NextOpenOrdered`, `PreviousPathEdge`, `EdgePerp`, `EdgeGroupID`, …) because the reference path finder (edge A*) uses it; Arkane's 52-byte edge goes with a poly A* (2012 `FNavMeshPolyBase` carries `bAlreadyVisited`/`visitedWeight`/`nextOrdered`/`previousPath`) — a runtime port, not a load fix |
| `FNavMeshPolyBase` | – / 156 | 136 | 136 | same reason (the poly-side A* fields are the retail design) |
| `FNavMeshCrossPylonEdge` | – / 104 | 168 | 172 | follows the edge base |
| `NavMeshKDOPTree` | – / 28 | 24 | 24 | reference compact tree; in-memory only |

Edge/poly objects are constructed by our own edge constructors into `EdgeDataBuffer` (`DataSize` of the stream is not used on load),
so their in-memory size does not affect the format. The 10 pending `UnPath.h` rows of the layout probe drop to the edge/poly rows.

## 4. loadall results per package (snapshot build, `loadall_driver.py`, logs `build\agentAD\loadall_{menu,tower}_run1.log`)

| package | exports (`.upk` table, package reader) | objects loaded (the tool's count) | errors |
|---|---:|---:|---:|
| Dishonored_MainMenu | 884 | 365 | 0 |
| Dishonored_MainMenu_Env | 6389 | 4394 | 0 |
| Dishonored_MainMenu_FX | 1051 | 314 | 0 |
| L_Tower_P | 428 | 147 | 0 |
| L_Tower_Audio | 125 | 91 | 0 |
| L_Tower_Block | 369 | 77 | 0 |
| L_Tower_Env | 7195 | 5089 | 0 |
| L_Tower_Fx | 888 | 184 | 0 |
| L_Tower_Light | 70 | 62 | 0 |
| L_Tower_Nav | 54 | 47 | 0 |
| L_Tower_Script | 20261 | 5016 | 0 |
| L_Tower_Water | 278 | 14 | 0 |

The reader's export count includes the forced exports of other packages cooked into each map (materials, tweaks, …), whose
outermost is not the map package. Every package loads without a linker abort, DishonoredGame export classes included, so there
is no serializer hand-over to AJ from these packages. Warnings during the sweep are the same as in the baseline run: 159×
`Failed to load 'MaterialInterface EngineMaterials.DefaultMaterial'` (`MaterialInstance.cpp:216` `engine-ini:` LoadObject, AG's
area), `LocalizationWarning` sections, 2× `CreateImport: Failed to load Outer for resource 'txr_grass02_m': Package tguichard_Material…`
(a cooked reference to an uncooked artist package, retail behaviour).

## 5. Commands and builds

```
# baseline (build0 = HEAD 4035b87 + nothing): dies in the nav mesh
python resources/tools/build_and_smoke.py --build-dir build/agentAD --no-build --exe-name DishonoredGame_AD.exe --log-name agentAD.log --ini-dir build/agentAD/config --rhi null --timeout 120 --skip-native OnlineSubsystemPC --milestone "Initializing Engine..." --expect "Initial startup"
# accept, on build_wt10 (the definitive build: snapshot + serializer + 464 layout + static shims + loadall): exit 0, all four checks
python resources/tools/build_and_smoke.py --build-dir build/agentAD_wt/build/agentAD --no-build --exe-name DishonoredGame_AD.exe --log-name agentAD.log --ini-dir build/agentAD/config --rhi null --timeout 300 --milestone "Committed map change via DishonoredEngine" --expect "Initial startup" --expect "Committed map change via DishonoredEngine" --forbid "Bad export index" --forbid "Serial size mismatch" --skip-native OnlineSubsystemPC "--extra-args=-forcelogflush"
#   -> expect ok: Initial startup / Committed map change via DishonoredEngine / no Bad export index / no Serial size mismatch
# sweeps on build_wt10 (identical numbers to the first run on build_wt1)
python build/agentAD/loadall_driver.py build/agentAD/menu.txt  --tag menu2  --build-dir build/agentAD_wt/build/agentAD --timeout 600
python build/agentAD/loadall_driver.py build/agentAD/tower.txt --tag tower2 --build-dir build/agentAD_wt/build/agentAD --timeout 600
# debugger run that found the render-thread abort after the commit (build2)
python resources/tools/debug/dbgrun.py --build-dir build/agentAD --exe-name DishonoredGame_AD.exe --log-name agentAD.log --ini-dir build/agentAD/config --rhi null --skip-native OnlineSubsystemPC --hang 400 --tag commit1
```
Builds: `build\agentAD\build0.log` (baseline), `build2.log` (working tree, serializer), `build_wt1.log` (snapshot: serializer +
loadall), **`build_wt10.log` (the definitive build: + 464 layout + static shims)**; smoke outputs `smoke0.txt` … `smoke2.txt`,
`smoke_final.txt`, `smoke_final2.txt`, `smoke_try1.txt` (the passing accept), `sweep_menu.txt` / `sweep_tower.txt` and the re-runs
`sweep_menu2.txt` / `sweep_tower2.txt`, `dbg_commit1.txt` / `build\agentAD\dbg\commit1.*`.

**Power-cut recovery (2026-09-26 ~01:40).** The machine lost power while ninja was archiving `DishonoredGameModule.lib` in the
snapshot build dir. Afterwards every link of that dir died with `Access violation` (link.exe returning exit code 0, a truncated
124,725,984-byte exe and a `.ilk` stopping at exactly 512 MiB), which survived deleting the exe/pdb/ilk/map, re-archiving the
library (byte-identical, so it was never truncated) and forcing a non-incremental link. The cause was the **compiler** PDBs:
`cl` reported `fatal error C1051: program database file … DishonoredGameModule.pdb has an obsolete format`. Deleting the 15
`*.pdb` files under the build dir (`find … -name "*.pdb" -delete`, no recursive directory delete, no links present) and
rebuilding fixed it (`build_wt10.log`, EXIT=0). Nothing outside my own build dir was touched; the retail tree and the sources
were intact (`git status` showed exactly my four files, and the snapshot copies were byte-identical to the working tree).

**Accept timing note.** The run is a race between the game thread logging the commit and the render-thread material abort of
hand-over 1. With several agents' builds running (18-25 concurrent `cl.exe`), `Initial startup` takes 50-56 s instead of 5.5 s
and the abort can win, so the accept reported `expect MISSING: Committed map change` twice at `--timeout 120` and `300`
(`smoke_final.txt`, `smoke_final2.txt`) with both `--forbid` checks passing in every case. With `-forcelogflush`, as in the
earlier passing run, it passes (`smoke_try1.txt`, exit 0). No serializer error appeared in any of these runs.

## 6. Hand-overs outside my files

1. **AG (renderer, blocks 30 s of ticking after the commit for AF/AE)**: right after `Committed map change via DishonoredEngine` the
   rendering thread hits an `appErrorf` in `FMaterialInstanceResource::GetMaterial+0x2a4` (`MaterialInstance.cpp:23`: the
   `checkSlow(IsCompilationFinalized/CompiledSuccessfully)` of a static-permutation instance, or its default-material fallback) from
   `FStaticMesh::AddToDrawLists` <- `FPrimitiveSceneInfo::AddToScene` <- `FScene::AddPrimitive` while `Dishonored_MainMenu_Env`'s
   primitives are added to the scene; the rendering thread is not guarded, so the `throw(1)` of `FOutputDeviceWindowsError::Serialize`
   terminates the process with exit code 3 (`abort`) and the log line is not written (the `-forcelogflush` log ends at the
   `SeqAct_Interp_14` warnings). Stack in `build\agentAD\dbg_commit1.txt`. Under the null RHI the coordinator's skip in
   `RenderViewFamily_RenderThread` does not cover `AddPrimitive`.
2. **AJ**: nothing — no DishonoredGame serializer error in the 12 packages.
3. **AF**: the map-change path now reaches the commit; `-loadall` is available for any further package (`@listfile`, absolute path).
4. **Coordinator / wave 5**: the retail nav mesh runtime (poly-based A*, 52-byte edges, dynamic pylons through `AArkDynamicPylon`,
   `IInterface_NavMeshPathObject` edges) is still the reference implementation with the shims/gates above; `AddDynamicCrossPylonEdge`,
   `RegisterObstacleWithNavMesh` and `BuildSubMeshForPoly` log once and do nothing.

## 7. What is left

- The imported-mesh fix-up branch (nav version < 28) uses `PolyNormal` for the pylon's per-poly up vector (2013 vtable +988 of
  `APylon`); no cooked mesh of the swept packages has a version below 28, so it never ran.
- `FNavMeshEdgeBase` / `FNavMeshPolyBase` / `FNavMeshCrossPylonEdge` keep the reference in-memory layout (table above).
- `NavMeshKDOPTree` 24 vs 28 (in-memory only).
- Agent Y's D3D9 `StaticMeshComponent` 306/302 line could not be reproduced on the null RHI; if it returns on D3D9 it is a material
  or LOD-info path, not the null-RHI serializer set.
