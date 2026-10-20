# The FaceFX 1.7.3.1 program (FX1..FX3)

FX1 owns this file (core: memory, names, strings, containers, classes, archive). FX2 created it because FX1 had not yet
when FX2 handed back; FX1 merges its own sections above FX2's.

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
