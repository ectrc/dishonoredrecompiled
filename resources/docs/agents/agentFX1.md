# Agent FX1 - FaceFX 1.7.3.1 core: memory, names, archive (wave 26, PHASE23 package FX1)

Worktree `build/agentFX1_wt` (made at `fd3f90b` = `91352d7` + docs, moved with `git checkout` to `eb2a05d` and then to
`741aeae`, which carries FX2), build dir `build/agentFX1` (RelWithDebInfo). IDA on private copies
`build/agentFX1/idb/{shipping2012,retail2013}_agentFX1.i64`; all 1,478 FaceFX functions of the 2012 db decompiled
to `build/agentFX1/dec2012/` (per compiland: `dec_by_obj/`), PDB types extracted to `build/agentFX1/fx_types_short.h`.
Archives read with a Python package reader (`resources/tools/facefx_archive_dump.py --extract`). No game run of
my own, no retail exe run (its bytes were read for the 2013 version check), no real input, no FModel.

Builds: **two** DishonoredGame builds (`build_game1.log` on fd3f90b, `build_game2.log` on eb2a05d - the gate binary),
plus CoreSmoke builds through `build_queued.py` (not counted). 0 errors, 0 warnings in any FaceFX unit.

## 1. Headline

**The FaceFX 1.7.3.1 core is ported and opens the retail archives.** `source/Development/Src/External/FaceFX`:
26 public headers (`include/`, OC3Ent::Face, 2012 PDB layouts, static_asserted) and 11 units (`Src/`): the memory
system and fixed-size pools, the shared name table with its 4096-bin hash, strings and the container templates with
retail's growth and serialisation rules, the class registry and object macros, FxObject/FxRefObject/FxRefString/
FxNamedObject, FxArchive with its directory, all five archive stores and FxFile, FxSDK startup and version identity,
FxBone/FxAnimKey/FxVec3/FxQuat. Inventory: 565 functions in FX1's compilands (42 KB), 505 FX1-owned - 369 ported by
rva, 111 template instantiations, 22 compiler generated, 3 header inlines, **0 not ported**; 60 belong to FX2/FX3
(their StaticClass in FxSDK.obj, FxUnrealSupport.obj, engine-unit inlines of FxActor/FxAnim).

**CoreSmoke** (on 741aeae, the library holding FX1's core and FX2's graph):
`FACEFX_CORE_SMOKE_DIR=build/agentFX1/fxa FACEFX_SMOKE_DIR=build/agentFX2 build/agentFX1/Binaries/Win32/CoreSmoke.exe`
-> **532 passed, 0 failed, 1 skipped** (Wwise's missing CSV). The FX1 checks: memory pools; FxName proofing, sharing,
ref counts, rename; save/load round trip of an FxBone pointer + FxAnimKey + FxString + bool in little and big endian
through the memory, file and fast-file stores; and for each of the 12 head archives of `L_Tower_Script.upk`
(Skm_EliteGuard_Head_FaceFX among them): sdk 1731, format 0, licensee strings and version 1000, the 10 class names and
versions in the very hash bins retail stored them in (7-bit), 134-219 names live as FxNames whose 12-bit bins and
bin/collision counts equal the Python simulation, NumObjectPointers, `Tell()` at the directory end, the header +
directory **re-saved byte for byte** equal to retail's, and FxNamedObject::Serialize reading the actor's name (= the
asset name). FX2's whole FaceFXGraphSmoke (every head's compiled graph, every anim set) passes on the real core.

## 2. Measurements

| what | value | where |
|---|---|---|
| FaceFX functions in the 2012 PDB (`facefx` + `fxsdk_unreal`) | 1,478 in 43 compilands | functions.csv |
| FX1 inventory | 565 fns / 41,986 B; FX1-owned 505 / 38,501 B | fx1_inventory.csv |
| 2013 twins of the inventory | 547 / 565, 534 at ratio 1.000; unmatched = 17 trivial stats/heap fns + ForceLoad/ForceUnloadAllMountedArchives | match_2012_2013.csv |
| FxSDKGetVersion / LicenseeVersion in 2013 | 0xafee60 `mov eax,6C3h` (1731), 0xaff040 `mov eax,3E8h` (1000) | retail exe bytes |
| FaceFXAsset exports in L_Tower_Script.upk | 12 (heads), archives 28,514-39,695 B, session bytes 0 | extract_facefx.py |
| classes in every head archive's directory | 10: FxBone 0, FxCompiledFaceGraph 0, FxObject 0, FxActor 3, FxMasterBoneList 2, FxNamedObject 0, FxName 1, FxPhonemeMap 1, FxAnimGroup 0, FxLinkFnParameters 1 | facefx_archive_dump.py |
| NumObjectPointers | 0 in all 12 | idem |
| directory end | 2,610-4,114 B | idem |

## 3. What was ported (files)

| file | content |
|---|---|
| `External/FaceFX/include/*.h` (26 FX1 headers) | FxPlatform, FxUtil (+FxTypeTraits), FxMemory, FxArray, FxList, FxString, FxStringHash, FxName, FxClass, FxObject (macros, FxCast), FxNamedObject, FxRefObject (+FxRefString), FxArchive, FxArchiveStore (+5 retail-named shims), FxFile, FxVec3 (+FxQuat shim), FxBone, FxKey, FxSDK |
| `External/FaceFX/Src/*.cpp` | FxMemory, FxName, FxClass, FxObject, FxRefObject, FxNamedObject, FxArchive, FxArchiveStore, FxFile, FxSDK, FxBone |
| `cmake/FaceFX.cmake`, `CMakeLists.txt` | target `facefx` = Src/*.cpp + FX2's Src/FaceGraph/*.cpp, /Zp4, `DISHONORED_WITH_FACEFX_SDK=1`; no game module links it (checked in build.ninja), `WITH_FACEFX` stays 0 |
| `source/Tests/CoreSmoke/FaceFXSmoke.cpp`, `CMakeLists.txt`, `CoreSmoke.cpp` | the checks; links `facefx` and runs FX2's suite on it; FX2's stub block kept for `NOT TARGET facefx` |
| `resources/tools/facefx_archive_dump.py` | stdlib reader, `--extract` (cooked packages), `--smoke` (expectations) |
| `resources/docs/facefx/facefx_program.md`, `fx1_inventory.csv` | conventions, FX2 reconciliation, FX3 handover (FX1 section, then FX2's) |

Deviations (named in the banners): the FX3 registrations/`FxActor::NewActor` compile on `__has_include` of FX3's headers;
`FxArchiveDirectory::NumObjectPointers` initialised to 0; `FxStringHash::GetBinIndex/GetBin` for the smoke; the
progress update as one private member; `FxFixedSizeMemoryAllocator` private to FxMemory.cpp.

**FX2 shim:** the real core replaces all of `include/stub/FxCoreStub.h`, `include/stub/FxBone.h`, `include/stub/FxKey.h`
and `Src/Stub/FxBoneStub.cpp`. Only `FxCast<T>` and FxObject's `IsKindOf` were missing for FX2's graph (added as the
fxobject.h inlines). The shim is left in place, uncompiled when `facefx` exists; FX3 deletes it.

## 4. Gate (build 2 = eb2a05d + FX1, `run_regression.py --only d3d9,inputtest,newgame`, `gate.txt`, `gate_inputtest2.txt`)

Machine at 100 % CPU (other agents' builds).
- **d3d9**: criticals 0, `unported_natives` 0, draw elements 6,506; `d3d9_frames` 930 on the first attempt (load), 1,170 on the retry: stage ok.
- **inputtest**: criticals 0, `probe_natives` 3 / 2; `inputtest_moved` 389 / 218 / 291 / 420 against >= 800 - the
  known load-sensitive metric (IR recorded 402.7 under load). The FaceFX library is not linked into the game exe
  (DishonoredGame.exe's link block in `build.ninja` has no facefx), so nothing of this package runs in the game.
- **newgame**: **passes** on eb2a05d (IT's fix): criticals 0, 29 pawns, on mission.

## 5. Where the brief was wrong (measured)

1. **Base**: main HEAD at launch was `fd3f90b` (91352d7 + PHASE23 docs), not 91352d7; main moved to 3e77e76, eb2a05d,
   224ee15, 1e15b34 (FX2), 2c81a28, 741aeae during the package. `make_snapshot.py` made a **Debug** configure
   script at launch (RelWithDebInfo only became its default in eb2a05d); I edited the generated cmd.
2. **"facefx_lysync_direct_2012.tsv"** is `build/agentIR/facefx_lipsync_direct_2012.tsv`.
3. **"FxString, FxList, FxHash, FxUtil" are not compilands**: they are header templates (fxstringimpl.h, fxlistimpl.h,
   fxstringhashimpl.h - FxHash is FxStringHash - and fxutil.h) instantiated inside whichever unit uses them (142
   functions from fxarrayimpl.h alone, spread over every compiland).
4. **"Each head's archive loads the whole face graph"**: the 12 heads' directories list 10 classes and **0 object
   pointers**; the graph is stored compiled (FxCompiledFaceGraph) - no FxFaceGraphNode object is constructed from a
   cooked head. "Object count" in the accept line is therefore 0 for every head.
5. **IR's "13 FaceFX archives in L_Tower_Script"**: 12 FaceFXAsset exports.
6. **"FX1 core 384 fns"**: FX1's compilands hold 565 (505 FX1-owned, 38.5 KB vs IR's 31 KB), because the ICF-folded
   template copies and compiler-generated functions count; IR's 384 was call-graph reachability.
7. **FxUnrealSupport** was in IR's FX1 list, but its classes derive from FX2's FxGenericTargetNode; FX2 ported them.
8. middleware.md's **1,317 FaceFX functions** is the `facefx` module only; with `fxsdk_unreal` the PDB has 1,478.
9. The FX2/FX1 split "by file" did not hold for FxAnim/FxAnimCurve/FxAnimSet/FUnreal* (FX2 took them) - fine, no overlap
   remained; `FxKey` (FxAnimKey) is FX1's, FX2's FxAnimCurve uses it.
10. Gate: inputtest_moved cannot be met at 100 % CPU (four runs 218-420); newgame now passes on main (IT).

## 6. Merge note

`python build/agentFX1_sync.py` (base 741aeae) reports 46 files clean, 0 flagged. FX2's headers, Src/FaceGraph and
the stub are main's and not in the list; CoreSmoke's CMakeLists.txt / CoreSmoke.cpp and facefx_program.md are merged
on top of FX2's versions in the worktree.
