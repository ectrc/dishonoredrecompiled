# The FaceFX 1.7.3.1 program (FX1..FX3)

FX1 owns this file (core: memory, names, strings, containers, classes, archive); FX2's section follows FX1's.

## FX1 - the core (agent FX1, wave 26)

The inventory of FX1's scope (every function of its compilands with 2012 rva/size/file, 2013 twin, owner, status) is
`resources/docs/facefx/fx1_inventory.csv`; regenerate it with `python build/agentFX1/py/mk_inventory.py <tree>`.
A function counts as ported when its 2012 rva is written in a file under `External/FaceFX` (banner or
`/** 2012 0x... */`), as in the Wwise program.

### What retail has (measured)

- The SDK is OC3's FaceFX 1.7.3.1 compiled from source into the exe: 1,478 functions in modules `facefx` +
  `fxsdk_unreal` of the 2012 PDB (43 compilands; `fxsdk_unreal` holds the compiler-generated deleting destructors and
  dynamic initialisers of the same units). `FxSDKGetVersion` returns 1731 in both 2012 (0xb14a10) and 2013 (0xafee60,
  bytes `b8 c3 06 00 00`); 1,448 of the 1,478 have a 2013 twin in `match_2012_2013.csv`, almost all at ratio 1.000:
  the 2013 SDK is the same build.
- Every cooked head archive (`UFaceFXAsset::RawFaceFXActorBytes`, extract with
  `python resources/tools/facefx_archive_dump.py --extract <dir> L_Tower_Script.upk`) is sdk 1731, file format 0,
  licensee "Unreal Engine 3 Licensee" / "Unreal Engine 3 Project" / 1000, and its class directory lists exactly ten
  classes: FxBone 0, FxCompiledFaceGraph 0, FxObject 0, FxActor 3, FxMasterBoneList 2, FxNamedObject 0, FxName 1,
  FxPhonemeMap 1, FxAnimGroup 0, FxLinkFnParameters 1. `NumObjectPointers` is 0: the actor is serialised by
  reference and the face graph is stored **compiled** (FxCompiledFaceGraph) - no FxFaceGraphNode object is ever
  constructed from a cooked head. 134-219 archive names per head (bone names, node names, phoneme names).

### Where things live

```
source/Development/Src/External/FaceFX/
  include/        public headers, OC3Ent::Face, PDB layouts (sizes static_asserted in the units), retail header names
    FxPlatform.h  types (FxSize = unsigned int, FxReal = float, FxBool = bool), FxStrcpy/FxStrncmp/FxItoa/FxMem*
    FxUtil.h      FxMin/FxMax/FxClamp/FxSquare/FxSwap/FxByteSwap, FxSqrt, FxRealEquality(Relaxed), FxTypeTraits
    FxMemory.h    allocation policy, FxAllocate/FxFree, FxUseAllocator, FxConstruct/FxDefaultConstruct/FxDestroy/FxDelete
    FxArray.h FxList.h FxString.h FxStringHash.h   the container templates (retail growth rules, see banners)
    FxName.h FxClass.h FxObject.h FxNamedObject.h FxRefObject.h (+ FxRefString)
    FxArchive.h   archive, directory, lazy loader, and the Serialize bodies of every container template
    FxArchiveStore.h (+ FxArchiveStore{Null,Memory,MemoryNoCopy,File,FileFast}.h shims)  FxFile.h
    FxVec3.h (+ FxQuat.h shim) FxBone.h FxKey.h (FxAnimKey) FxSDK.h
  Src/            one file per retail compiland family (FxMemory, FxName, FxClass, FxObject, FxRefObject,
                  FxNamedObject, FxArchive, FxArchiveStore, FxFile, FxSDK, FxBone [+Key, Vec3, Quat])
cmake/FaceFX.cmake   GLOB_RECURSE over Src/, target facefx / Dishonored::facefx, /Zp4, DISHONORED_WITH_FACEFX_SDK=1
source/Tests/CoreSmoke/FaceFXSmoke.cpp   the checks (below)
resources/tools/facefx_archive_dump.py   independent stdlib reader (header, directory, hashing)
```

FX2's files sit in the same include/ and in Src/FaceGraph; FX3 adds its own the same way (cmake globs Src/*.cpp and
Src/FaceGraph/*.cpp - add a folder there if FX3 uses one). File names follow the PDB's
(`fxfacegraphnode.h` -> `FxFaceGraphNode.h`): `FxSDK.cpp` and `FxName.cpp` look for them with `__has_include`.

### Rules the FX1 code follows (keep them)

1. **One retail function, one port function, 2012 rva in its comment.** A shape change is named in the banner.
   Header inlines and template instantiations are listed in the header banner with the rva of one instantiation.
2. **All memory through FxAllocate/FxFree with the block size** (pools need it). SDK classes derive from
   `FxUseAllocator`; arrays/lists construct with `FxConstruct`/`FxDefaultConstruct` (global placement new) and free with
   `FxDelete` (destroy + `FxFree(p, sizeof(T))` of the static type, as retail).
3. **Containers are the SDK's.** `FxArray` PushBack doubles (1 from empty), Insert grows by one, Clear frees, Remove
   copy-assigns down; `FxList` allocates its sentinel on first Begin/End; `FxStringHash` bins are djb2 over signed
   chars masked to HashBits. Arithmetic element types serialise as one POD block (`FxTypeTraits::is_arithmetic`),
   everything else element by element through `operator<<(FxArchive&, T&)`.
4. **Classes use the macros** in `FxObject.h`: `FX_DECLARE_CLASS(Name, Base)` first in the class body (the vtable
   starts GetClassDesc, GetCurrentVersion, dtor, Serialize - FxObject_vtbl in the PDB), `FX_IMPLEMENT_CLASS(Name,
   version, Base)` in the unit (the version is the one retail's AllocateStaticClass passes; it is what the archive
   directory records). Abstract classes: `FX_DECLARE_ABSTRACT_CLASS`. The macro also defines
   `operator<<(FxArchive&, Name&)` (Serialize) and `operator<<(FxArchive&, Name*&)` (the object-pointer protocol,
   retail 0xb21920).
5. **Serialize as retail**: `Super::Serialize(arc); arc.SerializeClassVersion("Name"); ...members`. Since sdk 0x6A4
   SerializeClassVersion reads nothing (the version comes from the directory) and records the class when saving;
   non-FxObject types (FxName, FxLinkFnParameters ...) pass `bIsNonFxObjectDerived = FxTrue` and their version.
6. **Register in FxSDKStartup**: it already calls `StaticClass()` of every FX2/FX3 class in retail's order, compiled
   when the header exists. Do not add registrations elsewhere.

### CoreSmoke (FaceFXSmoke.cpp)

`FACEFX_CORE_SMOKE_DIR=<dir with .fxa + .txt> <build>/Binaries/Win32/CoreSmoke.exe` from the tree root (default
`build/agentFX1/fxa`); the `.txt` files come from `facefx_archive_dump.py --smoke <dir>/*.fxa`. Checks:
- memory pools (block size steps, reuse, chunk return), FxName (proofing, sharing, ref counts, rename), version
  string "1.7.3.1", build id, class registration;
- save/load round trip of an FxBone pointer (twice: second resolves to the first object), FxAnimKey, FxString, bool,
  in little and big endian, through the memory, file and fast-file stores (the FxSaveActorToMemory sequence:
  CreateDirectory pass on a null store, SetInternalDataState, save);
- for every retail head archive: the 12-bit FxName hash bin of every name and the bins-in-use/insertions/collisions
  equal the python simulation; FxArchive opens it, sdk/format/licensee/version as python reads them, the 10 class
  names with their versions sit in the bins retail stored them in (the 7-bit hash), names count and every name live
  as an FxName, NumObjectPointers, `Tell()` at the directory end, the header + directory re-saved byte for byte equal
  to retail's, and FxNamedObject::Serialize reads the actor's name (= the asset name).

FX2's FaceFXGraphSmoke (its own `FACEFX_SMOKE_DIR`, default `../agentFX2`) runs in the same process right after,
on the same SDK instance: FaceFXSmoke starts the SDK with the default policy and never shuts it down (a restart would
leave every class's cached `_pClassDesc` dangling - retail never restarts it either). FX3 adds the actor load
(`FxLoadActorFromMemory`, every byte consumed) next to it.

### Handover

**Reconciliation with FX2 (done in FX1's tree, measured).** FX2's graph (`Src/FaceGraph`, its headers in include/)
compiles unchanged against this core once `FxObject` has `IsA` / `IsKindOf` / `IsExactKindOf` and `FxCast<T>` (added,
fxobject.h inlines). `cmake/FaceFX.cmake` builds `Src/*.cpp` + `Src/FaceGraph/*.cpp` into `facefx`; CoreSmoke links
it and runs FaceFXSmoke (core) and FaceFXGraphSmoke (FX2) in one process: 532 passed, 0 failed, 1 skipped (Wwise's
missing CSV), with `FACEFX_CORE_SMOKE_DIR=build/agentFX1/fxa FACEFX_SMOKE_DIR=build/agentFX2` from the tree root -
every head graph and anim set FX2 measures loads on the real core. `FxSDKStartup` registers FX2's classes in retail's
order (0xb14930) and calls `FxLinkFn::Startup`. FX2's shim (`include/stub`, `Src/Stub`, `FX2_FACEFX_CORE_STUB`) is
left in place but no longer compiled when the `facefx` target exists (CoreSmoke's FX2 block is `NOT TARGET facefx`);
FX3 deletes it. Of the shim, the real core replaces all of `FxCoreStub.h` (types, memory, FxArray/FxString/FxName/
FxStringHash/FxClass/FxObject/FxNamedObject/archive and stores, the object-pointer operator) and `stub/FxBone.h`,
`stub/FxKey.h`, `Src/Stub/FxBoneStub.cpp` (FxBone, FxAnimKey, FxVec3/FxQuat serialisation).

**FX3 (runtime + engine).**
- Library side left: FxActor, FxActorInstance, FxMasterBoneList, FxPhonemeMap, FxPhonemeEnum (2 fns),
  FxLoad/SaveActorFromMemory etc. (FX2 did FxAnim/FxAnimGroup/FxAnimSet/FxAnimCurve and the FUnreal* nodes).
  `FxName::Startup` names `FxAnimGroup::Default` (now compiled, FX2's FxAnim.h) and `FxActor::NewActor` once
  FxActor.h exists; FxSDKStartup registers FxActor / FxActorInstance / FxPhonemeMap and calls `FxActor::Startup` /
  `Shutdown` the same way (`__has_include`).
- Engine side: switch `WITH_FACEFX=1` (cmake/DishonoredDefines.cmake:48) and link Dishonored::facefx from
  `dishonored_apply_defines` (every module must agree, as for Wwise); point `Engine/Inc/UnFaceFXSupport.h` at
  `include/` (it includes `../../../External/FaceFX/FxSDK/Inc/Fx*.h`; the shims keep the retail file names).
  `UnInitFaceFX` (2012 0x568670 -> 2013 0x527510) starts the SDK with `MAT_Custom`, no pools, appMalloc/appFree
  (reference `UnFaceFXSupport.cpp:125`).
- The list from agentIR §3: `UpdateFaceFX` 0x347b90 -> 0x325ea0 (with `DisFaceFxStartAudio` 0x8f7e30 -> 0x8a0120 on
  APS_StartAudio, which moves the voice post off IH's path), `PlayFaceFXAnim` 0x379870 -> 0x356ef0,
  `UFaceFXAsset::Serialize` 0x56c360 -> 0x52b280 (Arkane delta; allocates the FxActor as 0x60A4 bytes),
  `UFaceFXAnimSet::Serialize` 0x56be00 -> 0x52b050, Mount/Unmount, `SetFaceFXAsset`'s SDK half (IR ported the rest,
  2013 0x312b90), `PlayDialogBlurb`'s FaceFX branch, IsDonePlayingBlurb / StopDialogBlurb, `APawn::PlayActorFaceFXAnim`,
  `UInterpTrackFaceFX`, `FArkComponentLookAt::m_bFaceFxIsPlaying`.
- **Where the bones are applied.** The reference engine calls `UpdateFaceFX(LocalAtoms, bTickFaceFX)` at the end of
  `USkeletalMeshComponent::UpdateSkelPose` (our `Engine/Src/UnSkeletalComponent.cpp:4168`, guarded by
  `!bDisableFaceFX && SkeletalMesh->FaceFXAsset`): after the anim tree (and our Edge evaluator) has written the local
  atoms, before they are composed to component space. Retail moved this tail into `UpdateSkelPoseEnd` (2012 0x354c10,
  2013 0x3471d0; agent FT's port at `UnSkeletalComponent.cpp:4190` documents it) - FX3 must make sure the FaceFX call
  sits where retail's is in that function. The asset is on the **body** component (IR), so the face bones are
  blended into the body's LocalAtoms and the head reads them through `ParentAnimComponent`; when the body is at LOD
  >= 1 its RequiredBones has none of the 19 face bones (IR), so FaceFX bones only show at body LOD 0, as in retail.

### Deviations (all named in the banners)

- `FxSDKStartup` / `FxSDKShutdown` and `FxName::Startup/Shutdown` compile the FX3 lines (FxActor, FxActorInstance,
  FxPhonemeMap) only when their headers exist (`__has_include`).
- `FxArchiveDirectory` initialises `NumObjectPointers` to 0 (retail leaves it unset until read or set).
- `FxStringHash::GetBinIndex` / `GetBin` exist for the smoke only.
- The per-operator progress update of FxArchive is one private member (`_updateProgress`); same arithmetic.
- `FxFixedSizeMemoryAllocator` is private to FxMemory.cpp (retail declares it in fxmemory.h; nothing else uses it).

## FX2 - the face graph (agent FX2, wave 26)

### Where things live

```
source/Development/Src/External/FaceFX/
  include/FxFaceGraphPrerequisites.h   the single core include: FX1's headers, or include/stub when FX2_FACEFX_CORE_STUB
  include/FxLinkFn.h                   link function types, FxLinkFnParameters, the 11 classes, FxLinkFnEval (0xb12570),
                                       FxHermiteInterpolate(start, end, alpha) (0xb12500)
  include/FxFaceGraphNode.h            FxFaceGraphNodeUserProperty, FxFaceGraphNodeLink, FxFaceGraphNode
  include/Fx{Combiner,Delta,CurrentTime,BonePose}Node.h, FxGenericTargetNode.h (+ FxMorphTargetNode and the three
                                       FUnrealFaceFX* nodes of fxunrealsupport.h)
  include/FxFaceGraph.h                the editable graph, jump lists, Iterator/Begin/End
  include/FxCompiledFaceGraph.h        FxCompiledFaceGraph(+Node/Link), FxRegister (FX3's struct, needed by Tick)
  include/FxAnimCurve.h, FxAnim.h      FxAnimCurve, FxAnim, FxAnimBoneWeight, FxAnimGroup, FxAnimSet
  include/stub/                        SHIM: FxCoreStub.h (FX1's API), FxBone.h, FxKey.h  - delete when FX1 merges
  Src/FaceGraph/*.cpp                  the ports (rva in each function's doc comment)
  Src/Stub/FxBoneStub.cpp              SHIM: FxVec3/FxQuat/FxBone/FxAnimKey - FX1 ported the same (its Src/FxBone.cpp)
resources/tools/facefx_graph_dump.py   Python reader of actor archives (graph, bone list, anims) and anim-set archives
resources/docs/facefx/fx2_inventory.csv  705 functions: 2012 rva/size/file, 2013 twin, status
```

CoreSmoke compiles `Src/FaceGraph` + `Src/Stub` into itself (`source/Tests/CoreSmoke/CMakeLists.txt`) until FX1's
library target exists. Reconciling with FX1 (measured on FX1's worktree of 2026-10-20 by hand: FX2's sources + FX1's
`include`/`Src` + the CoreSmoke checks, all 32 pass, 44 heads and 518 anim sets): delete `include/stub` and `Src/Stub`,
drop `FX2_FACEFX_CORE_STUB` and the stub include dir, link FX1's target. FX1 must add `FxCast<T>` (fxobject.h:44,
2012 0xb134c0 for FxFaceGraphNode) - the only symbol FX2 needed that FX1's headers lacked - and its `FxSDKStartup` should
register the graph classes and call `FxLinkFn::Startup()` in retail's order (0xb14930: FxFaceGraphNode,
FxFaceGraphNodeUserProperty, FxFaceGraphNodeLink, FxBonePoseNode, FxCombinerNode, FxCurrentTimeNode, FxDeltaNode,
FxGenericTargetNode, FxMorphTargetNode, FUnrealFaceFXNode, FUnrealFaceFXMaterialParameterNode, FUnrealFaceFXMorphNode,
FxFaceGraph, FxAnimCurve, FxAnimGroup, FxAnimSet, FxAnim, ... FxLinkFn::Startup). FX_REAL_MAX / FX_REAL_MIN /
FX_INT32_MIN are defined in FxFaceGraphPrerequisites.h when FX1's FxPlatform.h does not.

### What the cooked data uses (measured, facefx_graph_dump.py over every package)

- 44 head actor archives (`UFaceFXAsset::RawFaceFXActorBytes`): sdk 1731 (0x6C3), FxActor class version 3 - the
  compiled graph is stored, the editable graph is never in the data. Every head has 99 compiled nodes: 43 Combiner +
  56 BonePose; no Delta / CurrentTime / GenericTarget / Morph / UE3 material or morph nodes, no user properties, every
  node IO_Sum. Links: 6286 linear (2 parameters) and 44 quadratic (1 parameter), nothing else (135-165 per head). Bones: 33 on
  36 heads, 34 on one, 109 on five, 118 on two; 263-337 bone links. The actor archives hold no animations (default group empty).
- 518 anim-set archives (`UFaceFXAnimSet::RawFaceFXAnimSetBytes`, the dialogue): FxAnim version 8 (Arkane's Wwise event
  fields, all empty in the data), FxAnimCurve version 1 (keys packed per anim), 4466 anims, 88667 curves,
  1,020,584 keys, every curve Hermite, no bone weights.

### Handover to FX3 (runtime + engine)

FX3 owns FxActor, FxActorInstance, FxMasterBoneList, FxPhonemeMap (FxMasterBoneList stays FX3's, as agentIR §3 split it;
FX2 did not port it). What FX3 calls in FX2's code:

- Load: `FxActor::Serialize` (0xb0cdf0) reads `arc << _masterBoneList`, then (version >= 3) `arc << _compiledFaceGraph`
  (`operator<<(FxArchive&, FxCompiledFaceGraph&)` 0xb12220), then `arc << _defaultAnimGroup << _animGroups`, then the
  phoneme map. CoreSmoke's head reader walks exactly this with FX2's types - copy its order.
- Per frame: `FxActorInstance::Tick(time, ...)` (0xb0af70) evaluates the playing anim's curves
  (`FxAnimCurve::EvaluateAt`, 0xb16700) into the compiled nodes' `trackValue` (node index from
  `FxCompiledFaceGraph::FindNodeIndex(curve name)`, 0xb12180), then calls
  `FxCompiledFaceGraph::Tick(time, registers, hasBeenTicked)` (0xb114a0). `registers` has one FxRegister per compiled
  node (index = node index); Tick advances each register's interpolation and applies it as the node's user value, then
  evaluates the nodes in array order (inputs always precede their node). After Tick, `nodes[i].finalValue` is the node's
  value clamped to [min, max] and `registers[i].value` holds it too.
- Bones: the engine (UpdateFaceFX 0x347b90, FX3) walks the master bone list; each entry's links name a compiled node
  (`nodeIndex`) and an optimized bone delta; the bone pose node's `finalValue` is the link's weight. The bone pose
  node's own `FxBone` list is only used by `FxMasterBoneList::PullBones` / `Prune` (studio-side, through
  `FxFaceGraph::Begin/End(FxBonePoseNode::StaticClass())`).
- Anim playback inputs: `FxAnim::GetStartTime/GetEndTime/GetBlendInTime/GetBlendOutTime`, the curves
  (`GetNumAnimCurves/GetAnimCurve`), `FxAnimGroup::FindAnim/GetAnimPtr` (0x22bad0 / 0x568720 are engine-compiled
  copies), `FxAnim::SetAkEventIndex/SetAkEventPointer` (Arkane: the Wwise event the anim plays), anim sets through
  `FxLoadAnimSetFromMemory` (0xb15bd0).
- Compile / decompile (`FxActor::CompileFaceGraph` 0xb0c550 / `DecompileFaceGraph` 0xb0c5e0) are not on the playback
  path for these archives; UE3's `FxPreCompileCallbackFuncUE3` etc. go through
  `FxCompiledFaceGraph::SetCompilationCallbacks` / `SetPreDestroyCallback` (ported).
- `FxRegister` and `FxRegister::GetUserOpForRegOp` are defined inline in FxCompiledFaceGraph.h (FxActorInstance.obj,
  0xb0b990 / 0xb0bc30): FX3 should use them from there rather than define them again.
