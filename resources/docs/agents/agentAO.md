# Agent AO — the skeletal mesh LOD index assert, and the rendered world frame (2026-09-26)

Package: `UnSkeletalMesh.cpp:4133`'s

```
Assertion failed: DynamicIndexBuffer != NULL || PT_12_ControlPointPatchList == Mesh.Type ||
  BatchElement.FirstIndex + kIndicesPerPrimitive * BatchElement.NumPrimitives <=
  static_cast< DWORD >( LODModel.MultiSizeIndexContainer.GetIndexBuffer()->Num() )
```

Target: the retail 2013 exe (`retail2013_agentAO.i64`, copy of `retail2013_named.i64`); `shipping2012_agentAO.i64` is the
readable version. Decompiles in `resources/reference/decomp/agentAO/2013` (git-ignored).

## Result

| Acceptance | State |
|---|---|
| 1. the assert is gone and frames are drawn | **done**. `L_Tower_P` with all 8 sub-levels streamed in: **20,370 frames** under d3d9, windowed 1280x720, 90 s, **0 critical errors** (`build/agentAO_smoke19.txt`); **37,470 frames** under the null RHI, 60 s, 0 criticals (`build/agentAO_smoke20.txt`). The menu map (`DishonoredGameFull_P`) also renders with 0 criticals |
| 2. does agent AF's sub-level association hang go away | **yes for the hang, and the cause is named.** The hang was NOT a skeletal-mesh serialization bug: it is C++17 **aligned** `operator new`/`delete` (see §3). With `/Zc:alignedNew-` (snapshot only) `L_Tower_P` associates its sub-levels, the pawn is possessed and the run lives for 35 s+ with 0 criticals instead of hanging in `FSkeletalMeshObject::FinishCleanup` |
| 3. every fix cites a 2013 rva | **done**, `agentAO_status.csv` |

Two changes in the shared tree, both in **`Engine/Inc/UnSkeletalMesh.h`** (+22/-4, nothing else):

1. `operator<<(FArchive&, FSkelMeshSection&)` — the assert's root cause.
2. `ETriangleSortOption` — a latent off-by-one in the same struct's `TriangleSorting`.

`Engine/Src/UnSkeletalMesh.cpp`, `UnSkeletalComponent.cpp` and `GPUSkinVertexFactory.*` are **unchanged**: the
instrumentation that found the bug was reverted once it had done its job (§4).

## 1. The assert: an uninitialised temporary written back to a live field

Retail's section serializer (**2013 rva 0x30c2d0**, 2012 0x32edd0) is, field by field:

```
ByteOrderSerialize(&MaterialIndex, 2)   // +0
ByteOrderSerialize(&ChunkIndex,    2)   // +2
ByteOrderSerialize(&BaseIndex,     4)   // +4
ByteOrderSerialize(&NumTriangles,  2)   // +8   <- the field itself, a WORD
if (Ar.Ver() >= 599) Ar.Serialize(&TriangleSorting, 1)   // +10
```

`FSkelMeshSection` is 12 bytes in retail (the `12 * SectionIndex` stride is visible in the section iterator at
**2013 rva 0x335180**). The reference widened `NumTriangles` to a `DWORD` and kept a compatibility path for packages
below `VER_DWORD_SKELETAL_MESH_INDICES` (806) — Dishonored's packages are Ver 801 / LicenseeVer 30, so that path is
always taken:

```cpp
if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
{
    WORD NumTriangles;          // uninitialised
    Ar << NumTriangles;         // only meaningful when the archive is LOADING
    S.NumTriangles = NumTriangles;   // assigned back for EVERY archive
}
```

`FStaticLODModel::Serialize` is reached by every archive that walks a `USkeletalMesh`, not only the linker. The
**garbage collector's reference collector** (`Ar.IsObjectReferenceCollector()`) is one of them, and it runs on every
collection. For such an archive `Ar << NumTriangles` does nothing, and the write-back then replaces the live triangle
count with whatever was in that stack slot — measured values `0x8148`, `0x8888`, `0xcb18`, `0x8308`: in each case the
**low word of a stack address** (`esi = 0x024fcb18` when `eax = 0x0000cb18`).

Proof, from the run's own log (instrumentation since reverted):

```
[0000.73] lodtrace Ply_Player.Skm_Player LOD 0 ver 801/30: offsets 3896807 -> 3896822 -> 3923518, 1 sections, 13344 indices
[0000.73]   lodtrace section 0 at 0x01fc7950 (stride 16): material 0 chunk 0 BaseIndex 0 NumTriangles 4448 sorting 0
[0001.53] inittrace Ply_Player.Skm_Player section 0 at 0x01fc7950: BaseIndex 0 NumTriangles 4448, 13344 indices, 2771 vertices
[0002.53] index overrun on Ply_Player.Skm_Player LOD 0 section 0/1 ... NumTriangles 34952 FirstIndex 0 -> 104856 > indices 13344
[0002.53]   section 0 at 0x01fc7950: ... NumTriangles 34952 sorting 0
```

Same address, correct on load (4448 = 13344/3 exactly), garbage at draw time. The `Sections` array consumed
3896822-3896807 = **15 bytes** for one element = 4 (count) + 11 (element), byte-identical to retail, so the format was
never the problem. A hardware write watchpoint (DR0, 4-byte write, vectored exception handler) on
`&Sections(0).NumTriangles` caught the writer inside
`operator<<(FArchive&, TArray<FSkelMeshSection>&)` with `edi` holding the section array and `eax` the low word of
`esi`, a stack pointer — and the archive that re-serializes an already loaded LOD reports
`archive 'FArchive' (saving 0, counting 0, collecting refs 1)`.

The fix keeps the reference's DWORD field but makes the temporary a copy of the live value, so loading, saving,
counting and reference collection all behave as retail does:

```cpp
WORD NumTriangles = (WORD)S.NumTriangles;
Ar << NumTriangles;
S.NumTriangles = NumTriangles;
```

Retail's 12-byte layout (WORD field at +8) is recorded in `agentAO_status.csv` as a documented deviation; making the
field a `WORD` would match retail exactly but touches every arithmetic use of `Section.NumTriangles` across the
renderer, the sorters and the merge tools, so it was left alone.

**This is a bug class, not a one-off**: any reference version shim of the shape "read into an uninitialised local, then
assign it to the member unconditionally" corrupts live data on every non-loading archive. `FSkelMeshSection` was the
only instance in `UnSkeletalMesh.h/.cpp`; the pattern is worth grepping for elsewhere.

## 2. `ETriangleSortOption` is missing `TRISORT_Tootle`

`FSkeletalMeshSceneProxy::DrawDynamicElementsSection` (**2013 rva 0x314150**) selects the CustomLeftRight second index
set with `*((BYTE*)Section + 10) == 6`, and the script enum `TriangleSortOption`
(`resources/docs/types/script_classes_2013.json`) is

```
TRISORT_None, TRISORT_CenterRadialDistance, TRISORT_Random, TRISORT_Tootle,
TRISORT_MergeContiguous, TRISORT_Custom, TRISORT_CustomLeftRight
```

The reference C++ enum has no `TRISORT_Tootle`, so `MergeContiguous`/`Custom`/`CustomLeftRight` were 3/4/5 instead of
4/5/6. A section stored as `TRISORT_Custom` (5) would have been drawn as CustomLeftRight, and
`BatchElement.FirstIndex += Section.NumTriangles * 3` would have walked off the end of the index buffer — the same
assert, from a different cause. It is **latent**: no skeletal mesh in the startup set or in `L_Tower_P` stores a value
>= 3 (a probe logging every section with `TriangleSorting >= TRISORT_Tootle` fired zero times), so fixing the enum
alone did not clear the assert. Fixed anyway, with `TriangleSortOptionToString` extended.

A sweep of every `enum E<Name>` in `source/Development/Src` against the 782 script enums of
`script_classes_2013.json` found `ETriangleSortOption` to be the **only** mismatched enum in the tree.

## 3. Agent AF's B2 / B4 heap corruption: C++17 aligned operator new/delete

Not a retail delta, and not in my files — but the evidence landed on it, so it is recorded here in full.

With the assert gone, `L_Tower_P`'s sub-levels associate and the run then dies in exactly agent AF's B4 stack, now as
a crash instead of a hang (resolved through `build/agentAO_relwt/Binaries/Win32/DishonoredGame.map`, the exe is
ASLR-relocated so the module base must be read from the run):

```
WinMain -> GuardedMain -> FEngineLoop::Tick -> FSkeletalMeshObject::FinishCleanup (ICF-folded onto FTerrainObject::FinishCleanup)
  -> FSkeletalMeshObjectGPUSkin scalar deleting destructor
  -> ~TArray<FSkeletalMeshObjectLOD>
  -> FGPUSkinDecalVertexFactory scalar deleting destructor
  -> operator delete(void*, unsigned int, std::align_val_t)     <- ??3@YAXPAXIW4align_val_t@std@@@Z
  -> ucrtbase -> ntdll
```

Mechanism:

* `CMakeLists.txt:11` sets `CMAKE_CXX_STANDARD 17`, so MSVC's `/Zc:alignedNew` is on.
* `FGPUSkinVertexFactory`'s shader data holds `MS_ALIGN(16)` types (`FSkinMatrix3x4`, `FBoneQuat`,
  `GPUSkinVertexFactory.h:32/99`), so `FGPUSkinDecalVertexFactory` has `alignof` 16 > `__STDCPP_DEFAULT_NEW_ALIGNMENT__`
  (8 on x86) and every `new`/`delete` of it resolves to the **aligned** overloads.
* UE3's Core overrides only the unaligned globals (`Core/Inc/UnFile.h:2147-2163` → `appMalloc`/`appFree`). The aligned
  overloads are the CRT's, i.e. `_aligned_malloc` / `_aligned_free`.
* The factories are created with **placement new into the array's own memory**:
  `new(VertexFactories) VertexFactoryType(...)` (`UnSkeletalRenderGPUSkin.cpp:1064/1066`, `TIndirectArray` operator new
  in `Core/Inc/Array.h:1984`), i.e. UE3-heap memory, and destroyed with `delete` → the CRT's aligned free is handed a
  UE3 heap pointer. That is the heap corruption, and it explains why agent AF saw identical stacks under `FMallocDebug`,
  `FMallocBinned` and `-onethread`: it is an allocator mismatch, not a race.

Verified by adding `add_compile_options(/Zc:alignedNew-)` to **`build/agentAO_wt/CMakeLists.txt` (the snapshot only —
the shared `CMakeLists.txt` was not touched, agent AM is editing cmake right now)**. Over-aligned types then use the
ordinary `operator new`/`delete`, which UE3 routes to `appMalloc`/`appFree`, and `appMalloc` already returns 16-byte
aligned memory. Result with the switch, the sub-levels streaming and `-inputtest`:

```
[0002.68] possessed DishonoredPlayerPawn in l_tower_p at X=-3900.598 Y=36639.262 Z=-215.850
[0002.84] inputtest streaming levels: 8
... 35 s of ticking, no hang, no crash, exit 0, 0 criticals ...
[0035.63] inputtest moved 0.0 turned 22832 (peak 2D accel 3303.0, physics 0)
```

**Hand-over to the coordinator**: apply `/Zc:alignedNew-` in `CMakeLists.txt` next to the other global compile options
(or, equivalently, add the four `std::align_val_t` overloads next to `Core/Inc/UnFile.h:2147`). One line, and it retires
agent AF's B2 and B4.

Note the two blockers agent AF listed are now distinct:

* **B4 (sub-level association)** — retired by `/Zc:alignedNew-`.
* **B2 (menu map-change teardown)** — still open. With `/Zc:alignedNew-` it no longer crashes (0 criticals), but the
  game thread still stops after `Committed map change via DishonoredEngine`, now in
  `UGameEngine::Tick+0x147f -> FMallocBinned::GetAllocationInfo+0x42` (`build/agentAO_relwt/dbg/hang1.txt`,
  27 threads dumped, every other thread idle in `FEventWin::Wait`). Agent AI's zero-length matinee guard (B1) was
  applied in the snapshot too (`build/agentAO_snapfix.py`, derived from `build/agentAF_snapfix.py`) and does not change
  this, so B2 is a third thing: the pool walk in the memory-stat call at the end of `UGameEngine::Tick`. Not
  investigated further. `-startmapopen` still routes around it, which is how the 20,370-frame run above is made.
* **B3 (reference height fog assert)** did **not** fire in `L_Tower_P` with the scene renderer on, under d3d9 or the
  null RHI — Arkane's DisFog is what that map uses.

## 4. How the bug was found (method, for the next agent)

1. The retail serializers are the specification. `FStaticLODModel::Serialize` (0x354710), the section (0x30c2d0), chunk
   (0x323430), vertex buffer (0x34e330), colour buffer (0x3349f0), influences (0x353d80) and the index buffer
   (0x159940 → `TArray<WORD>::BulkSerialize` 0x156d20) were all compared field by field: **every one already matched**
   (agent AA's port holds up). That ruled serialization out early and was the most useful negative result.
2. A load-time trace (`Ar.Tell()` before/after each member, the section fields, the index count) showed the data is
   read correctly, so the corruption had to be post-load.
3. A hexdump of the heap neighbourhood showed only bytes +8..+9 of the section changing, with neighbouring 16-byte
   blocks stable — a targeted 2-byte write, not a block reuse.
4. A hardware watchpoint (DR0 via `GetThreadContext`/`SetThreadContext` on the game thread + a vectored exception
   handler, armed behind a `-disskelwatch` switch) named the writing instruction. **The exe is ASLR-relocated**: log
   `GetModuleHandle(NULL)` and subtract it before looking the address up in the `.map`, or the symbol is wrong (that
   cost one wrong answer, `TArray<BYTE>::Remove`).
5. `.map` lookups must expect `/OPT:ICF` folding: `FSkeletalMeshObject::FinishCleanup` shows up as
   `FTerrainObject::FinishCleanup`.

## Files

* shared tree: `source/Development/Src/Engine/Inc/UnSkeletalMesh.h` (+22/-4) — the only source change.
* snapshot only (`build/agentAO_wt`, `python resources/tools/make_snapshot.py AO --sync`):
  `CMakeLists.txt` (`/Zc:alignedNew-`) and `Engine/Src/UnInterpolation.cpp` (agent AI's matinee guard, via
  `build/agentAO_snapfix.py`). Build with `build/agentAO_wt_release.cmd` into `build/agentAO_relwt` (Release; the
  shared tree's OSS/Steamworks work in flight broke the link in `build/agentAO_rel`, which is why the snapshot exists).
* runs: `build/agentAO_smoke19.txt` (d3d9, 20,370 frames), `build/agentAO_smoke20.txt` (null RHI, 37,470 frames),
  `build/agentAO_smoke15.txt` (`L_Tower_P` sub-levels, no hang), `build/agentAO_relwt/dbg/hang1.txt` (B2 stacks).
* docs: `resources/docs/agents/agentAO.md`, `agentAO_status.csv`.

## Accept command

```
python resources\tools\build_and_smoke.py --build-dir build\agentAO_relwt --no-build --exe-name DishonoredGame_AO.exe ^
  --log-name agentAO_tower.log --ini-dir build\agentAO\config --rhi d3d9 --timeout 90 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```
