# Agent FX2 (wave 26, PHASE23 FaceFX program): the FaceFX 1.7.3.1 face graph

Worktree `build/agentFX2_wt` (snapshot of `fd3f90b` = `91352d7` + the PHASE23 launch note), build dir `build/agentFX2`
(RelWithDebInfo), IDA copies `build/agentFX2/idb/{shipping2012,retail2013}_agentFX2.i64`, decompiles
`build/agentFX2/dec2012/`, extracted archives `build/agentFX2/fxa` (44 heads) and `build/agentFX2/fxs` (518 anim sets).
Builds: three CoreSmoke builds through `build_queued.py`, no DishonoredGame build, no game run.

## 1. Headline

The face graph is ported whole from the 2012 decompile: the link functions and `FxLinkFnEval`, the user property, the
node link, the node base and all eight node classes (Combiner, Delta, CurrentTime, GenericTarget, MorphTarget,
BonePose, FUnrealFaceFXNode/MorphNode/MaterialParameterNode), the editable `FxFaceGraph` (link/unlink/cycle check,
jump lists, rename, change type, clone, serialize), the compiled graph (`Compile`, `Decompile`, `Tick` - the per-frame
evaluation with register interpolation, input ops, corrective links, delta and clock nodes, clamping), and
`FxAnimCurve` / `FxAnim` (Arkane's version 8) / `FxAnimGroup` / `FxAnimSet` with the memory load/save. 304 functions
(40 KB) ported in `External/FaceFX/{include,Src/FaceGraph}`, 29 more (FxBone/FxKey/FxVec3/FxQuat) in the shim because
FX1 ports them too. CoreSmoke 155 -> 187 with 32 FaceFX checks, all passing: the hand-derived graph and curve values,
all 44 cooked heads load their compiled graph with the Python dumper's counts to the last byte, decompile, recompile
and tick within range, and all 518 cooked anim sets (1,020,584 keys) load with the dumper's counts **and re-save byte
for byte**. The same 32 checks pass when FX2's sources are compiled against FX1's in-progress core instead of the shim
(only `FxCast` had to be supplied).

## 2. Measurements

- Scope (PDB compilands FxFaceGraph*, FxLinkFn, FxCompiledFaceGraph, the six node units, FxAnimCurve, FxAnim,
  FxAnimSet, FxBone, FxKey, FxVec3, FxQuat + the FUnrealFaceFX* functions of FxUnrealSupport.obj): 705 functions,
  74.4 KB. 304 ported (40.4 KB), 29 shim (1.6 KB, FX1's), 223 core templates instantiated in these units (25.2 KB:
  FxArray/FxStringHash/FxConstruct..., FX1's headers), 96 class-macro functions (StaticClass/GetClassDesc/
  ConstructObject/the link-function parameter block), 51 compiler-generated, 2 not ported (`FxLoadAnimSetFromFile`,
  `FxSaveAnimSetToFile`: file stores are FX1's, the engine uses memory). `resources/docs/facefx/fx2_inventory.csv`.
- 2013: 695 rows by `match_2012_2013.csv` (299 by bytes), 9 of the 10 unmatched resolved by bytes at the link-order
  slot (0-10 differing bytes, the 10 being call/EH fixups), 1 unresolved (`FxOneClampLinkFn::ConstructObject`, a
  7-byte `return 0`; the 2013 slot holds a 35-byte function). The SDK is the same unoptimised build in both exes:
  `Tick` 1976/1957 B and `FxLinkFnEval` 1140/1095 B match 1.000; the `FxLinkFnEval` difference is `FxSqrt` inlined in
  2013 (decompiled both).
- The cooked data (`facefx_graph_dump.py`, every package): see `facefx_program.md` - 99 nodes per head, only Combiner
  and BonePose, only linear and quadratic links, no user properties; anim sets all Hermite, FxAnim v8, no event names.

## 3. What was ported

| file | contents (2012 rvas in the doc comments) |
|---|---|
| `include/FxLinkFn.h`, `Src/FaceGraph/FxLinkFn.cpp` | FxLinkFnParameterDesc, FxLinkFnParameters serialize (custom-curve skip), FxLinkFn table (Startup order: 12 functions, "custom"/"perlin noise" FxNullLinkFn, no "null"), the 10 typed classes with their own parameter blocks, FxLinkFnEval, FxHermiteInterpolate(start,end,alpha) |
| `include/FxFaceGraphNode.h`, `Src/FaceGraph/FxFaceGraphNode.cpp` | user property (string owns one choice; SetBoolProperty has no type check), node link (SetNode with null keeps the pointer), node base (Serialize v0/1/2, _hasCycles) |
| `include/Fx*Node.h`, `Src/FaceGraph/FxFaceGraphNodes.cpp` | the eight node classes; delta takes one input and sets +-(max-min); current time takes none; bone pose normalises rotations; UE3 nodes' default user properties; FUnrealFaceFXNode::ReplaceUserProperty sets ShouldLink |
| `include/FxFaceGraph.h`, `Src/FaceGraph/FxFaceGraph.cpp` | the graph: name hash (2048 bins), jump lists, Link (LEC codes), Unlink/Get/SetLink, DetermineLinkDirection (always false, as retail), Serialize v0/v1 + relink, ChangeNodeType, NotifyRenamed, clone, Iterator/Begin/End |
| `include/FxCompiledFaceGraph.h`, `Src/FaceGraph/FxCompiledFaceGraph.cpp` | node type table 0xdc5fc8, relevance table, input-op identities 0xdc607c, callbacks, Compile (isolated nodes first, then DFS post-order from each output-less node), Decompile, Tick, FindNodeIndex, serialize |
| `include/FxAnimCurve.h`, `include/FxAnim.h`, `Src/FaceGraph/FxAnimCurve.cpp`, `FxAnim.cpp` | curve (cached bounding keys, autocomputed slope, extents), key interpolators, FxAnim (packed keys v6+, sound-cue fields v1-7 dropped, Wwise fields v7+), group, set, FxLoad/SaveAnimSetToMemory |
| `include/stub/*`, `Src/Stub/FxBoneStub.cpp` | SHIM: FX1's API (`FxCoreStub.h`), FxVec3/FxQuat/FxBone/FxAnimKey |
| `resources/tools/facefx_graph_dump.py` | Python reader of actor and anim-set archives (header, directory, names, object pointers, the actor, graph, bone list, anims), `--nodes`, `--csv` |
| `source/Tests/CoreSmoke/FaceFXGraphSmoke.cpp`, `CMakeLists.txt`, `CoreSmoke.cpp` | the checks; CoreSmoke compiles the FaceFX sources until FX1's target exists |

Named deviations (also in the file banners): three SDK headers merged in FxFaceGraphNode.h, the seven node units in one
.cpp, FxBone/FxVec3/FxQuat in one shim header; the link-function parameter macro name `FX_LINKFN_PARAMETERS` is ours
(the PDB keeps only fxlinkfn.h:114); FxHermiteInterpolate/FxLinearInterpolate(FxAnimKey) live in FxAnimCurve.h until
FX1's FxKey.h carries them; the Unreal node classes are in FxGenericTargetNode.h (FX1 owns fxunrealsupport); float
arithmetic stays single precision where retail's x87 build rounds through double; FxRegister (FX3's struct) is
defined in FxCompiledFaceGraph.h; the file load/save helpers are not ported.

## 4. CoreSmoke (`build/agentFX2/Binaries/Win32/CoreSmoke.exe`, run from `build/agentFX2_wt`)

187 passed, 0 failed, 1 skipped (the pre-existing Vorbis-reference skip). The FaceFX data defaults to `../agentFX2`
(`FACEFX_SMOKE_DIR`): `fxa/*.fxa` + `fxa_counts.csv`, `fxs/*.fxa` + `fxs_counts.csv`. Regenerate with
`python build/agentFX2/py/extract_facefx.py build/agentFX2/fxa "*.upk"` (`--animset` for fxs) and
`python resources/tools/facefx_graph_dump.py --csv build/agentFX2/fxa_counts.csv build/agentFX2/fxa/*.fxa`. Without the
data the two archive checks skip.

Hand-derived values checked: combiner Joy 0.8 -> bone pose open 0.5 (linear 0.5x+0.1), frown 0 (quadratic -0.3,
clamped), PBM 0.56 (corrective 0.5 from Disgust 0.6 scales by 0.7), delta 0 then -0.5; register ease 0 / 0.15625 / 0.5
at alpha 0 / 0.25 / 0.5; curve Hermite 0.75 at t=1 and 0.921875 at 2.25, 0.4375 at 0.5 after a backwards jump past the
cached key, linear 0.875, autocomputed slope 0.5.

## 5. Where the brief was wrong (measured)

1. **"each head's archive loads the whole face graph and every node class"**: the 44 cooked heads use 2 of the 8 node
   classes (Combiner 43, BonePose 56) and 2 of the 11 link functions (linear, quadratic); no user properties, no
   delta/clock/morph/material nodes. Ported whole anyway (faithful), but only those paths are exercised by the data.
2. **"FxFaceGraph::Evaluate" does not exist** in 1.7.3.1: the editable graph is never evaluated. The runtime evaluates
   `FxCompiledFaceGraph::Tick` (0xb114a0), called from `FxActorInstance::Tick` / `ForceTick`; heads store only the
   compiled graph (FxActor class version 3).
3. **FxCurve/FxCurveKey and FxMaterialParameterNode do not exist** in the PDB: the curve is FxAnimCurve/FxAnimKey; the
   material node is `FUnrealFaceFXMaterialParameterNode` (fxunrealsupport, ported).
4. **Overlapping ownership**: the brief gives FX2 FxBone/FxBonePose and the anim classes, agentIR §3 gives FxBone/FxKey/
   FxVec3/FxQuat to FX1 and FxAnim/FxAnimCurve/FxAnimSet/FxMasterBoneList to FX3. FX1's worktree already has
   FxBone/FxKey/FxVec3/FxQuat ported (`Src/FxBone.cpp`), so FX2 keeps its copies in the shim only; FxAnim* are ported
   here (FX3 should not redo them); FxMasterBoneList stays FX3's.
5. **"a small shim header"**: the shim is 800 lines (FxCoreStub.h, plus 360 for the FX1-owned bone/key types) because the graph cannot be tested without names,
   arrays, string hashes, classes and a real archive; it is folder-isolated (`include/stub`, `Src/Stub`) so deleting
   it is the whole reconciliation.
6. **Sizing**: agentIR §3 put FX2 at 430 functions / 43 KB; the PDB scope of the brief is 705 / 74 KB, of which 333
   (42 KB) are hand-written and the rest templates, class macros and compiler output.
7. **Base**: the snapshot is `fd3f90b` (91352d7 + one PHASE23 line); main has since moved (3e77e76, eb2a05d) without
   touching External/ or cmake/ - the sync script compares against main at hand-back.
8. FX1's core lacks `FxCast<T>` (fxobject.h:44) and its FxSDKStartup does not yet register the graph classes or call
   `FxLinkFn::Startup` - noted for FX1 in `facefx_program.md`.

## 6. Merge note

`python build/agentFX2_sync.py` (report) / `--apply`. New files only, plus two CoreSmoke hunks. Keep the shim until
FX1 merges; when both land, follow `facefx_program.md` (delete `include/stub`, `Src/Stub`, the define and the stub
include dir; FX1 adds FxCast and the startup registrations).
