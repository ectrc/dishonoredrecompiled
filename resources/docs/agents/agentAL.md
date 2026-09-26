# Agent AL — PhysX 2.8.4 bindings written from the shipped binaries, `WITH_NOVODEX=1` (2026-09-26)

Package AL: reconstruct the PhysX 2.8.4 API from the shipped DLLs and their PDBs (no NVIDIA SDK anywhere), build an
import library for `PhysXLoader.dll` the way agent U did for `binkw32.dll`, turn `WITH_NOVODEX` on, and make the
engine's physics path compile, link and run against the retail runtime.

Evidence sources, in order of authority: the PDBs that ship next to the DLLs in
`D:\RecompileDishonored\Dishonored_Debug2012\Binaries\Win32` (`PhysXCore.pdb`, `PhysXCooking.pdb`,
`PhysXExtensions.pdb`, `NxCharacter.pdb`, `PhysXLoader.pdb`; every shipped PhysX DLL is byte-identical between the 2012
and the retail 2013 trees, `resources/docs/binaries.md`), the DLLs' own machine code, the DLL export tables, and the
retail 2013 exe (`retail2013_agentAL.i64`, a copy of `retail2013_named.i64`).

## Result

| Step | State |
|---|---|
| 1. Minimum 2.8.4 surface (SDK, scene, actor, shape, meshes, cooking, queries) | **done, and rather more than the minimum**: 109 descriptors/value types + 68 SDK interfaces + 103 enums generated from the PDBs, plus 12 hand-written headers. `NxCharacter`'s controller is **not** included: nothing in the tree uses `NxController`/`NxControllerManager` |
| 2. Import library the Bink way | **done**: `cmake/PhysX.cmake` builds `PhysXLoader_stub.cpp` into a throw-away `build/<dir>/physx/stub/PhysXLoader.dll` for its import library only. `dumpbin /imports` on our exe: `PhysXLoader.dll` → `NxCreatePhysicsSDK`, `NxGetCookingLib`, `NxGetPhysicsSDKAllocator`, `NxReleasePhysicsSDK` (retail imports those four plus `NxGetPhysicsSDK` and `NxGetUtilLib`, which no code path in the tree calls). No import library for `PhysXCooking.dll`, `PhysXExtensions.dll` or `NxCharacter.dll` — retail imports nothing from them either |
| 3. `WITH_NOVODEX=1` compiles and links | **done**: full `DishonoredGame` target, 0 errors. `WITH_PHYSX_COOKING=1`, `NX_DISABLE_FLUIDS=1`, `USE_QUICKLOAD_CONVEX=0`, `SUPPORT_DOUBLE_BUFFERING=0`, `WITH_APEX=0`; ~99 Engine files with a `WITH_NOVODEX` guard came alive and **none of them is stubbed out** |
| 4. Runtime | **the SDK, the scene, actors, shapes, a cooked convex mesh and the materials all come up through our vtables**: `DISHONORED(bringup): PhysX scene: 2 actors, 1 static shapes, 1 dynamic shapes; SDK: 0 triangle meshes, 1 convex meshes, 0 height fields, 3 materials, gravity X=0.000 Y=0.000 Z=-1500.000` in `l_tower_p`, and `Initial startup: 6.99s` with no critical error |
| 5. Pawn stands on the world | **not reached, and not for a PhysX reason** — see "The pawn" below. UE3 `PHYS_Walking` collision is the kDOP path (`Engine/Src/UnPhysic.cpp` has no `WITH_NOVODEX` guard at all), so PhysX was never in it. With agent AO's `/Zc:alignedNew-` added to a build of my own the run lives 39 s with 0 criticals, the `-inputtest` completes and `moved` is still 0.0: the pawn is spawned **0.7 s before** the streamed sub-levels are associated, falls past `KillZ` and freezes at `PHYS_None`. The eight `LevelStreamingAlwaysLoaded` entries report `bShouldBeLoaded 0 bShouldBeVisible 0` at the first tick, so nothing blocks on them during `LoadMap` — that is the remaining gap and it belongs to the level-streaming package |

## The pawn: PhysX was not the blocker

The package brief said `WITH_NOVODEX=0` was why agent AF's pawn could not walk. It is not, and the evidence is in this
build directory:

1. UE3 pawn movement (`APawn::physWalking` → `AActor::moveActor` → `UWorld::SingleLineCheck` → per-component
   `LineCheck`/`PointCheck`) is the **kDOP** path. `Engine/Src/UnPhysic.cpp` contains no `WITH_NOVODEX` at all; PhysX
   only carries `PHYS_RigidBody`, ragdolls, vehicles, cloth and the rigid-body queries.
2. Before touching anything I re-ran agent AF's accept line **without** `-nolevelstream` on the unmodified wave-4 tree
   (`build/agentAL_smoke1.txt`): the sub-levels `L_Tower_Env` and `L_Tower_Fx` are added to the world and the process
   then dies at 7.98 s. That is AF's B4, with `WITH_NOVODEX=0`.
3. With `WITH_NOVODEX=1` the same run dies at the same place (`build/agentAL_smoke4.txt`, `..._smoke6.txt`), only later
   in the level list (`L_Tower_Script`'s actors initialise first now).
4. New detail worth recording: **B4 is not a deadlock, it is `abort()`**. Every such run ends with exit code 3 and no
   `Critical:` line anywhere in the log — `build/agentAL_smoke1.txt` (baseline, PhysX off) and `build/agentAL_smoke4.txt`
   (PhysX on) both say `run: exit code 3 (0x00000003)`, and under the debugger `dbg.py` reports
   `process exited, code 3` with no exception (`build/agentAL/dbg/hang1.txt`). Exit code 3 with no first-chance
   exception is the MSVC CRT's `abort()`. That is consistent with agent AO's root cause (C++17 aligned
   `operator new`/`delete` against UE3's unaligned global overrides): the Debug CRT's aligned `free` rejects a UE3-heap
   block and aborts.

5. I then built the same tree into `build/agentAL_an` with agent AO's hand-over applied as a configure flag
   (`-DCMAKE_CXX_FLAGS=/Zc:alignedNew-`, `build/agentAL_alignedbuild.cmd`; the shared `CMakeLists.txt` was **not**
   touched) and re-ran the accept line **without** `-nolevelstream` (`build/agentAL_smoke7.txt`, exit 0, both
   `--expect` lines present):

   ```
   [0007.02] possessed DishonoredPlayerPawn in l_tower_p at X=-3900.598 Y=36639.262 Z=-215.850
   [0007.71] inputtest streaming levels: 8   (all eight LevelStreamingAlwaysLoaded: bShouldBeLoaded 0 bShouldBeVisible 0)
   [0007.71] inputtest waiting for ground: 1.0s, physics 2, pawn at Z=-215.850, floor none
   [0007.78] UWorld::AddToWorld: updating components for L_Tower_Env
   [0007.85] UWorld::AddToWorld: updating components for L_Tower_Fx
   [0009.71] inputtest waiting for ground: 3.0s, physics 0, pawn at Z=-5001.594, floor none
   [0039.74] inputtest moved 0.0 turned -7604 (peak 2D accel 3303.0, physics 0)
   ```

   No abort, no critical error, 39 s of ticking — but the pawn was spawned 0.7 s before the first sub-level became
   visible, so it was already through `KillZ` when the floor arrived and `PHYS_None` is terminal. The eight
   `LevelStreamingAlwaysLoaded` objects have `bShouldBeLoaded 0 bShouldBeVisible 0` at that first tick, i.e. nothing in
   `UGameEngine::LoadMap` blocks on them (its two `FlushLevelStreaming(NULL, TRUE)` calls only flush *visibility*).
   **That is the one remaining step to a standing pawn**, and it is level-streaming work, not physics work.
6. The retail route (`-startmap=L_Tower_P` without `-startmapopen`, which does block on the load) now gets all the way
   to `Committed map change via DishonoredEngine` twice and then dies at 8.12 s in
   `USequence::ExecuteActiveOps → UObject::IsA` (`build/agentAL_smoke8.txt`, `0xC0000005`) — a stale Kismet op of the
   torn-down menu level, i.e. agent AF's B2, which agent AO reports as still open. Also not physics.

So the remaining work for "the pawn stands on the world" is agent AO's compile option (already handed over) plus the
streamed-level blocking above — not anything in this package. What this package adds on top is that with PhysX on,
the level's rigid-body geometry is instanced as well (`URB_BodyInstance::InitBody` runs for 235 components of
`l_tower_p`'s streamed levels; the ones that fail do so on the reference's own `Cannot 3D-Scale rigid-body primitives`
guard, which is a scaled-blockout-mesh property, not a binding problem).

## What is in the tree

`source/Development/Src/External/PhysX284/` — 7,564 lines, 23 files. Two are generated; everything else is
hand-written with its evidence in the file header.

| File | Lines | What it is |
|---|---|---|
| `NxGenEnums.h` | 1,137 | **generated**: every top-level `Nx*` enum with the PDB's values (103 enums) |
| `NxGenerated.h` | 4,694 | **generated**: 109 descriptors/value types (members at the PDB's offsets, `setToDefault()` decoded from the DLLs) and 68 SDK interfaces (every virtual at the PDB's vtable slot) |
| `Nx.h`, `NxSimpleTypes.h`, `NxVersionNumber.h` | 55/57/21 | macros, the integer/float typedefs the PDB member widths imply, `NX_PHYSICS_SDK_VERSION` = `0x02080400` |
| `NxMath.h`, `NxVec3.h`, `NxQuat.h`, `NxMat33.h`, `NxMat34.h`, `NxBounds3.h` | 66/146/136/253/104/232 | the value types, layouts from the PDB, arithmetic hand-written. `NxBounds3.h` also carries `NxPlane`, `NxRay`, `NxSegment`, `NxSphere`, `NxBox`, `NxCapsule`, `NxTriangle`, `NxTriangle32`, `NxGroupsMask` |
| `NxArray.h` | 96 | `NxArray<T,Alloc>` = `{T* first; T* last; T* memEnd;}` (12 bytes in every PDB instantiation) over `NxGetPhysicsSDKAllocator()` |
| `NxUserAllocator.h` | 119 | the four callbacks the engine *implements*: `NxUserAllocator`, `NxUserOutputStream`, `NxStream`, `NxUserEntityReport<T>` |
| `NxActorDesc.h` | 116 | `NxActorDescBase` + `NxActorDesc`, including the `shapesStart` wiring (below) |
| `NxPhysics.h` | 110 | the loader entry points and `NxContactStreamIterator` |
| `NxCooking.h` | 79 | `NxGetCookingLib` and the free cooking functions as inline wrappers over `NxCookingInterface` |
| `NxForceFieldKernelDefs.h` | 172 | the SDK's force-field kernel DSL (`NX_START_FORCEFIELD` / `NxFConst` / `NxSelect` / …) that `Engine/Inc/ForceFunction*.h` is written in |
| `NxFoundation.h`, `NxStream.h`, `NxSceneQuery.h`, `NxExtensions.h`, `NxExtensionQuickLoad.h` | 24/16/14/38/33 | the umbrella headers `UnNovodexSupport.h` includes by name |
| `PhysXLoader_stub.cpp` | 30 | the import-library source (10 exports) |
| `NxAllocatorDefault.cpp` | 28 | the only translation unit: `NxArray`'s default allocator over `NxGetPhysicsSDKAllocator` |

Plus `cmake/PhysX.cmake` (new), four targeted lines in `cmake/Dependencies.cmake`, the PhysX block in
`dishonored_apply_defines()` (`cmake/DishonoredDefines.cmake`), three small edits in Engine
(`UnNovodexSupport.h`, `UnNovodexLibs.cpp`, `UnPhysLevel.cpp`) and one new repo tool
(`resources/tools/pdb/dia_types.py`).

## Method

### 1. `resources/tools/pdb/dia_types.py` (new repo tool)

`llvm-pdbutil` cannot read these PDBs and `resources/tools/ida/export_types.py` needs an IDA database, which the
middleware DLLs do not have. The new tool reads UDT layouts, **vtable slots** (`IDiaSymbol::virtualBaseOffset / 4`),
`const`-ness of methods and of members, bitfields, base classes and enums straight out of a PDB through DIA:

```
python resources/tools/pdb/dia_types.py <file.pdb> --udt-re '^Nx' --enum-re '^Nx' --json out.json
python resources/tools/pdb/dia_types.py <file.pdb> --udt NxScene --vtable-only
```

The five PhysX dumps are in `build/agentAL_pdb/*.json` (1,598 UDTs / 280 enums for `PhysXCore` alone) together with a
function table per DLL (`*_fns.csv`).

### 2. Descriptor defaults decoded from the DLLs

PhysX's descriptors set themselves up in header-inline `setToDefault()` bodies, so the *values* are not in any type
record. They are, however, compiled into the DLLs wherever the SDK instantiates a descriptor itself. `dumpbin
/disasm:nobytes` on `PhysXCore.dll`, `PhysXExtensions.dll` and `NxCharacter.dll` plus a small decoder
(`build/agentAL_defaults.py`, and the same decoder inside the generator) reads the stores back out — SSE (`movss`
from `__real@…`) in `PhysXCore`, x87 (`fld1`/`fldz`/`fst`/`fxch`) in `PhysXExtensions`:

* **33 descriptors** get their real defaults this way (31 from `PhysXCore.dll`, 2 from `PhysXExtensions.dll`), each
  carrying its rva in the header. Examples, all verified against the disassembly:
  `NxSceneDesc` (rva `0x1273c0`): `maxTimestep = 1/60`, `maxIter = 8`, `flags = 0x44`, `subdivisionLevel = 5`,
  `staticStructure = NX_PRUNING_STATIC_AABB_TREE`, `dynamicTreeRebuildRateHint = 100`, `solverBatchSize = 32`,
  three `NX_TP_NORMAL` thread priorities;
  `NxShapeDesc` (rva `0xa0b50`): `shapeFlags = 0x120008`, `density = 1`, `mass = -1`, `skinWidth = -1`;
  `NxBodyDesc` (`PhysXExtensions.dll` rva `0x1540`): `wakeUpCounter = 0.4`, `angularDamping = 0.05`,
  `maxAngularVelocity = -1`, `sleepLinearVelocity = -1`, `sleepAngularVelocity = -1`, `solverIterationCount = 4`,
  `flags = 0x900`, `sleepEnergyThreshold = -1`, `contactReportThreshold = FLT_MAX`, `massLocalPose = identity`.
* **76 descriptors** have no compiled copy in any shipped DLL; the generator zeroes their members and
  `build/agentAL_gen_notes.txt` lists every one of them. Four of those are on paths that matter and got hand values
  with named evidence:
  * `NxPhysicsSDKDesc` — **from retail**: `InitGameRBPhys` (2013 rva `0x3d5710`) builds the descriptor inline before
    `NxCreatePhysicsSDK`, and the decompile shows `[+0]=0x10000`, `[+4]=256`, `[+8]=2048`, `[+12]=0`, `[+16]=3`
    (`NX_SDKF_NO_HARDWARE|NX_SDKF_PER_SCENE_BATCHING`), `[+20]=32`, `[+24]=-1`;
  * `NxTriangleMeshDesc` — `heightFieldVerticalAxis = NX_NOT_HEIGHTFIELD` (255 in the PDB's `NxHeightFieldAxis`; a
    zeroed axis would mean `NX_X`) and `convexEdgeThreshold = 0.001f`;
  * `NxMaterialDesc` — `dirOfAnisotropy = (1,0,0)` so the anisotropic-friction path has a unit vector;
  * three descriptors decode all but a few instructions (`NxForceFieldDesc`, `NxPulleyJointDesc`,
    `NxWheelShapeDesc`), listed in the notes file.
* The const `type` member of every shape/joint/force-field descriptor is **not** written by the shipped
  `setToDefault()` (verified: `NxBoxShapeDesc::setToDefault` rva `0xf5870` leaves `@4` alone) — the constructor sets
  it. The generator sets it from the class name and the PDB's enum (`NX_SHAPE_BOX`, `NX_JOINT_D6`, …).

### 3. `NxActorDesc::shapesStart`, from `NxCharacter.dll`

`NxActorDescBase` ends with `NxShapeDesc*** shapesStart` at `@88` and has no shape count, so how the SDK finds an
actor's shapes is not obvious. `NxCharacter.dll` compiles its own `NxActorDesc::NxActorDesc()` (rva `0x19d0`), and the
disassembly settles it: `memset(this,0,0x68)`, `globalPose.id()`, every field zeroed, `mov [esi+54h],1`
(`type = NX_ADT_DEFAULT`), then `lea edi,[esi+5Ch]` (the `NxArray` at `@92`), `mov [edi+4],[edi]` (empty array) and
`mov [esi+58h],edi`. So `shapesStart` is the address of the array's `first` pointer and the SDK reads `shapesStart[0]`
and `shapesStart[1]` as begin/end — exactly the `NxArray {first,last,memEnd}` layout. `NxActorDesc.h` reproduces that,
including re-pointing `shapesStart` on copy and assignment.

### 4. MSVC reverses consecutive virtual overloads — the one thing that made the SDK crash

The first run with `WITH_NOVODEX=1` died inside `NxCreatePhysicsSDK` (`build/agentAL_smoke2.txt`). The cause is an MSVC
ABI detail: **a run of consecutive virtual functions with the same name is laid out in the vtable in reverse
declaration order**. Proven two ways:

* `cl /FAsc` on `build/agentAL_vt/vtprobe.cpp`: `virtual int f(int)` declared before `virtual int f(int,int)` emits
  `??_7A@@6B@ DD FLAT:?f@A@@UAEHHH@Z` first, i.e. `f(int,int)` in slot 0;
* the shipped `NxUserAllocatorDefault` vtable, read out of `PhysXCore.dll` at rva `0x3568e0` (and the abstract base's
  at `0x3568c0`, where slots 1/3/4/5 are `_purecall`): slot 0 `mallocDEBUG` (5 args, the forwarding one), slot 1
  `mallocDEBUG` (3 args, pure), slot 2 `malloc` (2 args), slot 3 `malloc` (1 arg, pure), slot 4 `realloc`, slot 5
  `free`, slot 6 `checkDEBUG`, slot 7 the destructor.

So every same-name run must be *declared* highest-slot-first. The generator does that for all 68 interfaces (a
self-check asserts that the emitted declaration order, after applying the reversal, is 0..n for every class) and
`NxUserAllocator.h` does it by hand. With the order fixed, `NxCreatePhysicsSDK` returns an SDK and the scene comes up.

### 5. Other PDB-derived facts worth keeping

* `NxMat33` stores **row major**: `Nx9Real` is a union of `float m[3][3]` and a struct whose fields are `_11,_12,_13,
  _21,…` in memory order, so element (r,c) is `m[r][c]`. Engine's `U2NMatrixCopy`/`N2UTransform` go through
  `setColumnMajor`/`getColumnMajor` with an Unreal basis row per PhysX column, i.e. PhysX is the column-vector
  convention (`NxMat33.h`).
* `NxUserAllocator::malloc` takes `size_t` in the header (`unsigned int` in the PDB on x86); the engine's
  `FNxAllocator` overrides the `size_t` spelling, so the header uses `size_t`.
* `NxCookingInterface`'s vtable (from `PhysXCooking.pdb`) is `NxSetCookingParams` 0 … `NxReportCooking` 12, destructor
  13; the free `NxCookTriangleMesh` & co. are inline wrappers over it, which is why retail imports only
  `NxGetCookingLib` and nothing from `PhysXCooking.dll`.
* `NxExtensionQuickLoad` (from `PhysXExtensions.pdb`) is declared for completeness but never used: the retail exe
  imports nothing from `PhysXExtensions.dll`, so `USE_QUICKLOAD_CONVEX` is 0 and `InitGameRBPhys` takes the
  four-argument `NxCreatePhysicsSDK` branch — which is exactly what the retail decompile shows.
* 2 of 264 `Nx*` types are forward-declared only, because they carry members that are internal to the DLLs:
  `NxSceneEvent` (an unnamed nested enum) and `NxSpinMutexLock` (a reference member). Neither is used by the engine.

## Per-type table (the interfaces the collision path goes through)

Sizes and vtable-entry counts are the PDB's; every virtual is emitted at its PDB slot.

| Interface | sizeof | vtable entries | base |
|---|---|---|---|
| `NxPhysicsSDK` | 4 | 37 | - |
| `NxScene` | 12 | 161 | - |
| `NxActor` | 8 | 104 | - |
| `NxShape` | 12 | 40 | - |
| `NxBoxShape` | 12 | 6 | `NxShape` |
| `NxSphereShape` | 12 | 6 | `NxShape` |
| `NxCapsuleShape` | 12 | 9 | `NxShape` |
| `NxConvexShape` | 12 | 5 | `NxShape` |
| `NxTriangleMeshShape` | 12 | 11 | `NxShape` |
| `NxHeightFieldShape` | 12 | 18 | `NxShape` |
| `NxWheelShape` | 12 | 28 | `NxShape` |
| `NxTriangleMesh` | 4 | 20 | - |
| `NxConvexMesh` | 4 | 12 | - |
| `NxHeightField` | 4 | 16 | - |
| `NxMaterial` | 8 | 24 | - |
| `NxJoint` | 12 | 26 | - |
| `NxD6Joint` | 12 | 8 | `NxJoint` |
| `NxCompartment` | 4 | 15 | - |
| `NxSceneQuery` | 4 | 20 | - |
| `NxSweepCache` | 4 | 3 | - |
| `NxCookingInterface` | 4 | 15 | - |
| `NxFoundationSDK` | 4 | 11 | - |
| `NxUtilLib` | 4 | 77 | - |
| `NxCloth` | 8 | 108 | - |
| `NxSoftBody` | 8 | 92 | - |
| `NxForceField` | 8 | 37 | - |
| `NxForceFieldShape` | 12 | 9 | - |
| `NxForceFieldShapeGroup` | 8 | 13 | - |
| `NxForceFieldKernel` | 8 | 8 | - |
| `NxEffector` | 12 | 6 | - |
| `NxRemoteDebugger` | 4 | 27 | - |
| `NxDebugRenderable` | 24 | 0 (plain struct) | - |

Key value types: `NxSceneDesc` 164, `NxActorDescBase` 92, `NxActorDesc` 104, `NxBodyDesc` 136, `NxShapeDesc` 108,
`NxBoxShapeDesc` 120, `NxSphereShapeDesc` 112, `NxCapsuleShapeDesc` 120, `NxConvexShapeDesc` 112,
`NxTriangleMeshShapeDesc` 120, `NxHeightFieldShapeDesc` 136, `NxMaterialDesc` 48, `NxSimpleTriangleMesh` 28,
`NxTriangleMeshDesc` 52, `NxConvexMeshDesc` 28, `NxPhysicsSDKDesc` 28, `NxCookingParams` 12, `NxSceneQueryDesc` 8,
`NxRaycastHit` 56, `NxSweepQueryHit` 48, `NxContactPair` 40, `NxContactStreamIterator` 60, `NxVec3` 12, `NxQuat` 16,
`NxMat33` 36, `NxMat34` 48, `NxBounds3` 24, `NxGroupsMask` 16.

## cmake wiring

* `cmake/PhysX.cmake` (new, follows `cmake/Bink.cmake` and agent AM/AN's pattern): `DISHONORED_WITH_PHYSX`, default
  **ON** when `source/Development/Src/External/PhysX284/NxPhysics.h` exists. It builds `physxloader_stub` (a
  throw-away `PhysXLoader.dll` in `build/<dir>/physx/stub`, import library in `build/<dir>/physx`), a one-file static
  library `physx284` that carries the header directory as a `PUBLIC` include, and exports
  `Dishonored::physx`. It fails the configure if `${DISHONORED_RETAIL_DIR}/Binaries/Win32/PhysXLoader.dll` is missing.
* `cmake/Dependencies.cmake`: `include(cmake/PhysX.cmake)` after `Bink.cmake` (which declares `DISHONORED_RETAIL_DIR`).
* `cmake/DishonoredDefines.cmake`: the hard-coded `WITH_NOVODEX=0` / `WITH_PHYSX_COOKING=1` rows are gone from
  `DISHONORED_DEFINES`; `dishonored_apply_defines()` now sets, per target,
  `WITH_NOVODEX=1 WITH_PHYSX_COOKING=1 NX_DISABLE_FLUIDS=1 USE_QUICKLOAD_CONVEX=0 SUPPORT_DOUBLE_BUFFERING=0
  DISHONORED_PHYSX_IMPORT_LIB=1` and links `Dishonored::physx` when the option is on, `WITH_NOVODEX=0
  WITH_PHYSX_COOKING=1` when it is off. `WITH_APEX=0` stays in the global list, permanently.
* Why those four switches:
  * `NX_DISABLE_FLUIDS=1` — `Core/Inc/UnBuild.h` only derives it when PhysX is *off*; keeping it on with PhysX on is a
    deliberate bring-up choice, because the fluid/particle path is not on the collision path and would pull in the
    `fluids/` headers and ~20 more files. Retail ships fluid code, so this is the one place where this build is
    knowingly narrower than retail.
  * `USE_QUICKLOAD_CONVEX=0` — retail imports nothing from `PhysXExtensions.dll`.
  * `SUPPORT_DOUBLE_BUFFERING=0` — `NxdScene` lives in the SDK's static `libnxdoublebuffered` (1,289 functions in the
    retail exe), which is not in any DLL and which we do not have.
  * `DISHONORED_PHYSX_IMPORT_LIB=1` — guards the `#pragma comment(lib, …)` block in `UnNovodexLibs.cpp`, which names
    SDK libraries (`PhysXCooking.lib`, `PhysXLoader.lib`, `libnxdoublebuffered_release.lib`) that do not exist here.

Engine edits (three files, all small and all tagged):

| File | Change | Evidence |
|---|---|---|
| `Engine/Src/UnNovodexSupport.h` | `SUPPORT_DOUBLE_BUFFERING` and `USE_QUICKLOAD_CONVEX` wrapped in `#ifndef` so cmake can set them | retail imports (`imports_2013.csv`), `InitGameRBPhys` 2013 rva `0x3d5710` |
| `Engine/Src/UnNovodexLibs.cpp` | the Win32 `#pragma comment(lib, …)` block guarded by `!DISHONORED_PHYSX_IMPORT_LIB`; new `GDishonoredLoggedPhysXSceneSummary` | our own import library is on the link line |
| `Engine/Src/UnPhysLevel.cpp` | a one-per-world `DISHONORED(bringup)` scene summary in `UWorld::TickWorldRBPhys`, reset in `UWorld::InitWorldRBPhys` | the numbers come out of the SDK vtables, so a wrong slot shows up immediately |

### Shared-file discipline

`cmake/DishonoredDefines.cmake` and `cmake/Dependencies.cmake` were changed with targeted edits only (never rewritten),
because the Steamworks and Wwise packages own blocks in them. After my edits `dishonored_apply_defines()` still carries
all four SDK blocks in order — Bink, PhysX, Steamworks (`DISHONORED_WITH_STEAMWORKS`), Wwise (`TARGET
Dishonored::wwise`) — and `DISHONORED_DEFINES` still carries agent AO's `WITH_STEAMWORKS=$<BOOL:…>` row. I did not
touch `CMakeLists.txt`.

## Build verification

* Working tree: `set BUILD_DIR=build\agentAL` + `resources\build-game.cmd` — 743 units, **0 errors, 0 link errors**
  (`build/agentAL_build10.log`, `..._build12.log`).
* Isolated: `python resources/tools/make_snapshot.py AL --list build/agentAL_files.txt` (HEAD `9b99346` + only my 31
  files) built into a separate directory with `build/agentAL_snap_build.cmd` — **798 units, 0 errors**
  (`build/agentAL_snapbuild.log`), and that exe runs: `build/agentAL_smoke9.txt`, exit 0, `Initial startup` and the
  `DISHONORED(bringup): PhysX scene` line both present. So the package does not depend on anybody else's in-flight
  edits.
* A third build, `build/agentAL_an` (`build/agentAL_alignedbuild.cmd`), is the same tree plus
  `-DCMAKE_CXX_FLAGS=/Zc:alignedNew-` for the pawn experiment; it is a configure flag, no file was edited for it.

## Runtime

Startup (`build/agentAL_smoke3.txt`, exit 0, `expect ok: Initial startup`):

```
python resources\tools\build_and_smoke.py --build-dir build\agentAL --no-build --exe-name DishonoredGame_AL.exe ^
  --log-name agentAL.log --ini-dir build\agentAL\config --rhi null --timeout 180 --skip-native OnlineSubsystemPC ^
  --milestone "Initializing Engine..." --expect "Initial startup" "--extra-args=-noscenerender -forcelogflush"
```

```
[0000.80] Log: PhysX GPU Support: DISABLED
[0006.39] Log: Primary PhysX scene will be in software.
[0006.39] Log: Creating Primary PhysX Scene.
[0006.99] Log: >>>>>>>>>>>>>> Initial startup: 6.99s <<<<<<<<<<<<<<<
```

Mission map (`build/agentAL_smoke6.txt`, `--expect "DISHONORED(bringup): PhysX scene"` satisfied):

```
python resources\tools\build_and_smoke.py --build-dir build\agentAL --no-build --exe-name DishonoredGame_AL.exe ^
  --log-name agentAL_map3.log --ini-dir build\agentAL\config --rhi null --timeout 200 ^
  --skip-native OnlineSubsystemPC --milestone "Initializing Engine..." ^
  --expect "DISHONORED(bringup): PhysX scene" ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -noscenerender -forcelogflush"
```

```
[0006.78] DevPhysics: DISHONORED(bringup): PhysX scene: 2 actors, 0 static shapes, 2 dynamic shapes;
          SDK: 0 triangle meshes, 1 convex meshes, 0 height fields, 2 materials, gravity Z=-1500.000   <- menu world
[0007.77] DevPhysics: DISHONORED(bringup): PhysX scene: 2 actors, 1 static shapes, 1 dynamic shapes;
          SDK: 0 triangle meshes, 1 convex meshes, 0 height fields, 3 materials, gravity Z=-1500.000   <- l_tower_p
```

Every number in those lines is read back through a reconstructed vtable (`NxScene::getNbActors`,
`getNbStaticShapes`, `getNbDynamicShapes`, `getNbMaterials`, `NxPhysicsSDK::getNbTriangleMeshes`,
`getNbConvexMeshes`, `getNbHeightFields`) after `setGravity` was written through another, and the convex mesh is one
the SDK built from a package's **precooked** `FKCachedConvexData` through our `NxStream` implementation. Nothing is
cooked at load time in this map (`COOKEDPHYSICS:` never appears — `bUsePrecookedPhysData` is TRUE and the cached data
is current), so the cooking interface is exercised only as far as `NxGetCookingLib` + `NxInitCooking`.

Then, at 7.96 s, the run ends the way the wave-4 baseline does: exit code 3 with no `Critical:` line, i.e. the
aligned-new `abort()` agent AO root-caused. The pawn is at `PHYS_Falling` at that moment.

## What is stubbed, and what is knowingly narrower than retail

Everything the engine calls is real except these, all tagged `DISHONORED(bringup)` in the source:

1. **`NxContactStreamIterator`** (`NxPhysics.h`) — correct 60-byte layout, but the iterator reports an empty stream
   (`goNextPair()` returns false). The contact-stream *format* is PhysX-internal and only
   `NxContactStreamIterator::goNextPoint` is compiled into `PhysXCore.dll` (rva `0x1a50`); the constructor and the rest
   are header-inline and exist in no shipped binary. Consequence: `FNxContactReport::onContactNotify`
   (`UnNovodexSupport.cpp:350`) sees no contact points, so per-contact gameplay feedback (impact sounds, contact
   damage) is silent. Collision itself does not go through it. Reconstructing the stream from `goNextPoint` is a
   bounded follow-up.
2. **Fluids** — `NX_DISABLE_FLUIDS=1`, so `PhysXParticle*`, `ParticleModules_PhysX.cpp` and the `fluids/` headers stay
   out. All the fluid interfaces (`NxFluid`, `NxFluidEmitter`, `NxFluidDescBase`, …) *are* generated, so turning
   fluids on is a matter of writing the three `fluids/Nx*.h` umbrella headers.
3. **Double buffering** — `NxdScene` (`libnxdoublebuffered`, a static SDK library) is not available; the
   single-buffered scene is used. Retail links the double-buffered one.
4. **`NxCharacter.dll`** — no binding at all, because nothing in the tree uses `NxController`. `NxCharacter.pdb` was
   still useful (it is where `NxActorDesc`'s constructor is compiled).
5. **`NxJointDesc::setGlobalAnchor` / `setGlobalAxis`** (`NxGenerated.h` tail) — header-inline in the SDK, so
   hand-written from their documented semantics: the anchor/axis is taken into each actor's local frame and the
   normal is an arbitrary orthogonal unit vector. If a D6/ragdoll joint ever looks rotated about its own axis, this is
   the place to check.
6. **`isValid()`** on the generated descriptors returns `true` — the SDK validates internally anyway, and the engine
   only uses it in `check()`s.
7. 76 descriptors have zeroed defaults (`build/agentAL_gen_notes.txt`); the four that matter are hand-set with
   evidence. Any future "PhysX behaves oddly" bug should look there first.

## What remains

1. **The pawn** (not mine, but this is what it needs). With `/Zc:alignedNew-` in `CMakeLists.txt` the `-startmapopen`
   run is already stable for 39 s; what is missing is that `LoadMap` must have the map's `LevelStreamingAlwaysLoaded`
   sub-levels **loaded and visible before gameplay starts**. Right now all eight report
   `bShouldBeLoaded 0 bShouldBeVisible 0` on the first tick and only become visible a frame or two later, after the pawn
   has already fallen through `KillZ` into the terminal `PHYS_None`. Either `ULevelStreamingAlwaysLoaded` is not setting
   those two flags (check `UpdateShouldBeLoaded` / the script defaults) or `UGameEngine::LoadMap` needs the full
   `FlushLevelStreaming()` that `UGameEngine::Tick` does behind `bRequestedBlockOnAsyncLoading`, instead of the two
   visibility-only `FlushLevelStreaming(NULL, TRUE)` calls it has. Once the floor is there at spawn time, `moved`
   follows from agent AF's numbers (peak 2D acceleration 3303 is already produced); PhysX is not in that path.
   The retail `STREAMMAP` route additionally needs agent AF's B2 (stale menu Kismet op, crash in
   `USequence::ExecuteActiveOps`).
2. **`NxContactStreamIterator`** (item 1 above) — the only piece of the API that is a behavioural stub.
3. **Fluids** (item 2) if the particle systems are ever wanted.
4. **`libnxdoublebuffered`** — either live single-buffered (fine) or reimplement `NxdScene` from the retail exe's
   1,289 functions.
5. Two 2.8.4 types are forward-declared only (`NxSceneEvent`, `NxSpinMutexLock`); harmless.
6. `resources/docs/middleware.md` §2.2 and §4.1 still say "PhysX 2.8.4 SDK" is a blocker for the user. It is not any
   more; §2.2's alternative ("reconstruct the vtables from `PhysXCore.pdb`, feasible but hundreds of interfaces") is
   what this package did, and it took one pass because the PDB is complete.

## Files

Mine (31, the list `build/agentAL_files.txt` used for the snapshot): the 23 files of
`source/Development/Src/External/PhysX284/`, `cmake/PhysX.cmake`, targeted edits in `cmake/Dependencies.cmake` and
`cmake/DishonoredDefines.cmake`, `Engine/Src/{UnNovodexSupport.h,UnNovodexLibs.cpp,UnPhysLevel.cpp}`,
`resources/tools/pdb/dia_types.py`, plus this report and `agentAL_status.csv`.

Scratch (not repo tools): `build/agentAL_gen_physx.py` (the header generator — re-run it after any PDB dump change),
`build/agentAL_defaults.py` and `build/agentAL_fn.py` (the `setToDefault` decoder and a one-function disassembly
extractor), `build/agentAL_pdb/*.json` + `*_fns.csv` (the five PDB dumps), `build/agentAL_gen_notes.txt`,
`build/agentAL_vt/` (the MSVC overload-order probe), `build/agentAL_build*.log`, `build/agentAL_smoke*.txt`,
`build/agentAL_err*.txt`, `build/agentAL/dbg/hang1.*`, snapshot `build/agentAL_wt` + `build/agentAL_snap*`,
IDA copy `resources/docs/idb/retail2013_agentAL.i64`, decompile `build/agentAL_decomp/InitGameRBPhys_3d5710.c`.
The three `dumpbin /disasm` dumps the generator reads live in the session scratchpad, not in the repo; re-create them
with `build/agentAL_disasm.cmd`, `_disasm2.cmd`, `_disasm3.cmd`.

No commits, no `git add`.
