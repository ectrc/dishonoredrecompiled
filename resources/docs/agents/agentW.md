# Agent W report — Edge animation: retail parity (steps 1–6), then the per-sequence evaluator (step 7)

Build dir `build\agentW` = the HEAD snapshot `build\agentW_wt` (da24f65) with my files synced by `build\agentW\sync.py`
(the shared tree's `GameFrameworkClasses.h:1085` is mid-edit by another agent and does not compile; the shared
`external\libpng-build` is regenerated concurrently by other build dirs, so the snapshot uses its own FetchContent binary
dirs). Configure `build\agentW_configure.cmd` (all four module options, `DISHONORED_REAL_LAUNCH=ON`), build
`build\agentW_build.cmd DishonoredGame` → `build\agentW\Binaries\Win32\DishonoredGame.exe` (63.8 MB). IDA copies
`shipping2012_agentW.i64`, `retail2013_agentW.i64`; decompiles in `build\agentW\decomp2012`, `decomp2013`.

## Part 1 — retail parity (steps 1–6): done, mergeable on its own

### Edits (all tagged `DISHONORED(...)`)

| Step | File | Change | Evidence |
|---|---|---|---|
| 1 | `Engine/Inc/EngineAnimClasses.h` | `ACF_EdgeAnim = 7`, `ACF_MAX = 8`, `op(ACF_EdgeAnim)` | retail SDK `Engine.AnimSequence.AnimationCompressionFormat` (`ACF_EdgeAnim = 7`, `ACF_END = 8`) |
| 1 | `Engine/Classes/AnimSequence.uc` | `ACF_EdgeAnim` appended to the enum (comment) | same |
| 1 | `Engine/Src/AnimationEncodingFormat.cpp` | 8th entry `0` in `CompressedTranslationStrides/Num`, `CompressedRotationStrides/Num`, `0,0,0,0,0,0,0,0` in `PerTrackNumComponentTable`; `case ACF_EdgeAnim: break;` in the **ConstantKeyLerp** translation and rotation switches of `AnimationFormat_SetInterfaceLinks` (codecs stay NULL); `#include "EdgeAnim.h"` (compiles the step-5 static_asserts) | 2013 rva 0xc8250 (2012 0xc4420): `case 7: break;` / `case 7: return result;` only in the `KeyEncodingFormat == 0` branch; the VariableKeyLerp branch has no case 7 in retail either, so it is left as is |
| 2 | `Engine/Src/UnSkeletalAnim.cpp` `UAnimSequence::Serialize` | load: `if (RotationCodec) ByteSwapIn(...) else CompressedByteStream = SerializedData;` save/count: `if (RotationCodec) ByteSwapOut(...) else SerializedData = CompressedByteStream;` (the two `check(RotationCodec)` are gone) | 2013 rva 0x34c0d0 (2012 0x368be0): `if (this->RotationCodec) codec->ByteSwapIn(...) else TArray copy (this+220 = CompressedByteStream)`, save symmetric |
| 3 | `Engine/Src/UnSkeletalAnim.cpp` `UAnimSequence::PostLoad` | "No animation compression exists" only when `CompressedTrackOffsets.Num() == 0 && CompressedByteStream.Num() == 0`, as `appErrorf`; no `CompressAnimSequence`; no `ForcedRecompressionSetting`; `EncodingPkgVersion != CURRENT` → `appErrorf("Animation compression method out of date…")` + empty both arrays | 2013 rva 0x34c4a0 (2012 0x370150): `if (!NumFrames) {editor remove} else if (Offsets.Num() \|\| ByteStream.Num()) {} else GError->Logf("No animation compression exists…")`; `if (EncodingPkgVersion @308) { GError->Logf("…out of date…"); Empty; Empty; }`. `ANIMATION_ENCODING_PACKAGE_ORIGINAL == 0` = `CURRENT_ANIMATION_ENCODING_PACKAGE_VERSION` (`AnimationEncodingFormat.h:13-16`); cooked sequences carry no `EncodingPkgVersion` tag (extractor), so 0 passes |
| 4 | `Engine/Src/UnAnimPlay.cpp` `UAnimNodeSequence::GetAnimationPose` | after the linkup checks: `RotationCompressionFormat == ACF_EdgeAnim && CompressedTrackOffsets.Num() == 0` → `FillWithRefPose` (identity for `bIsAdditive`), `GetCurveData`, return; root motion stays the identity set at the top; the editor-only `GetAdditiveBasePoseBoneAtom` block at the end is never reached for Edge sequences | `DISHONORED(bringup)`; every other `UAnimSequence::GetBoneAtom` caller (root-motion extraction, `UnSkeletalMesh.cpp:4824`) already falls to the raw path → identity, as in retail (`GetBoneAtom` 2013 0x315920 = reference) |
| 5 | `Engine/Inc/EdgeAnim.h` (new) | declarations with 2012 PDB layouts and 27 `static_assert`s: `EdgeAnimAnimation` 96, `EdgeAnimSkeleton` 64, `EdgeAnimFrameSetInfo` 4, `EdgeAnimJointTransform` 48, `EdgeAnimBlendLeaf`/`Branch` 16, `EdgeAnimPoseInfo` 32, `EdgeAnimCustomDataTable` 16, `FEdgeSkelToAnimMapping` 4, `FEdgeAnimToSkelMapping` 2, `FLocomotionStateBase`/`FLocomotionState` 48, `FEdgeAnimTreeContext` 96 (retail nests it in `UAnimNode`), `FEdgeAnimData` 176 align 16, `FEdgeAnimManager` 16, `FDisJobDesc` 28, `FEdgeAnimJobDesc` 84; no behaviour. `USkeletalMeshComponent::m_pEdgeAnimData` stays NULL | `types.json` (2012 PDB). Note: the PDB gives `Vectormath::Aos::Quat` align 4, so `EdgeAnimJointTransform` is 48 bytes align 4 as declared, not 16-aligned |
| 5 | check | `USkeletalMesh::Serialize` (agent O, 2013 0x355260) reads `m_EdgeSkeleton` between `RotOrigin` and `RefSkeleton`: the same field order parses all 37 meshes of `Engine`/`DishonoredGame`/`Startup` to the byte (extractor), each blob has tag `0x45533033`, `sizeTotal == Num()`, `numJoints == RefSkeleton.Num()` and base pose == `RefSkeleton` BonePos (< 1e-4) | `resources/tools/pdb/extract_edgeanim.py`, check script in §"Commands" |
| 6 | `resources/docs/edgeanim.md` (new) | format as observed, evaluation chain with rvas, minimal function set with 2012/2013 rvas and sizes, what drops out on PC, integration (per-sequence, 6–9 days vs whole tree +8–12), verification list | decompiles + extracted data |

Not changed: `AnimationUtils.cpp:434/439` (`Clamp(..., ACF_None, ACF_MAX)`, editor compression settings) and
`AnimationCompressionAlgorithm.cpp:147` (`< ACF_MAX` check) — both editor-only paths, correct with `ACF_MAX = 8`;
`UnSkeletalAnim.cpp:3602` (`[ACF_MAX]` stats arrays) grows with the enum.

### Results (build `build\agentW`, snapshot da24f65 + my files)

- `cmake --build build\agentW --target DishonoredGame`: 727/727, 0 errors (the 27 `EdgeAnim.h` static_asserts hold).
- `python resources/tools/build_and_smoke.py --build-dir build/agentW --no-build --exe-name DishonoredGame_W.exe --log-name agentW.log --ini-dir build/agentW/config --milestone "objects as part of root set" --skip-native OnlineSubsystemPC --extra-args "-NoLoadStartupPackages -allowunboundnatives"`
  → **the animation load passes** (no `AnimationEncodingFormat.cpp:613` / "7: unknown or unsupported translation
  compression"); `DishonoredGame.upk`'s 36 Edge sequences (exports 16819–16854) and 4 skeletal meshes (26056–26059)
  deserialize. The run then **hangs** (timeout 120 s, "milestone NOT reached") in `LoadStartupPackages` →
  `LoadPackageList` (native script packages) → `ULinkerLoad::LoadAllObjects` → `Preload` →
  **`UStaticMesh::Serialize`+0x648 → `operator<<(FArchive&, TArray<FLOAT>&)`** → `FArchiveAsync::Serialize` spinning in
  `appSleep` (a read past the end of the export: a mis-serialized count). Stack from `build\agentW\stack_sample.py`
  (samples every thread of the running exe and resolves EBP chains through `DishonoredGame_W.map`). The only native
  package with `StaticMesh` exports is `DishonoredGame.upk` (33, from export 26176 `Crossbows.Bolt_Sleep_brk1`), so the
  blocker is the first static mesh of `DishonoredGame.upk` — outside my package (AA: `UStaticMesh::Serialize`). The
  `EngineMaterials.DefaultMaterial` warnings before it are the expected `-NoLoadStartupPackages` noise.
- Same command without `-NoLoadStartupPackages`: stops at the same `UStaticMesh::Serialize` stall (the native script
  packages load before `Startup.upk`), so `Startup.upk`'s 428 Edge sequences are not reached in-engine yet. At data level
  (extractor, same field order as `UAnimSequence::Serialize`): all 428 + 36 sequences parse with
  `NumBytes == blob size`, 0 trailing bytes; 462 are `ACF_EdgeAnim` with tag `0x45413035`, 2 are UE-codec sequences.

### Follow-ups outside my files

1. `UStaticMesh::Serialize` (AA): TArray<FLOAT> read overrun on `DishonoredGame.upk` static meshes (stack above).
2. Root `CMakeLists.txt`: step 7's test adds `add_subdirectory(source/Tests/EdgeAnimSmoke)` (see Part 2).
4. Part 1 alone (without step 7) is `agentW_parity.patch`; the tree's `UnAnimPlay.cpp` has since moved to the step-7 routing.
3. `gen_classes_header.py` regenerates `EngineAnimClasses.h` from `AnimSequence.uc`: the enum entry is in the `.uc`, so a
   regeneration keeps `ACF_EdgeAnim = 7`.

## Part 2 — per-sequence evaluator (step 7, Plan B): done

**Merge note.** The tree's `UnAnimPlay.cpp` now carries the step-7 routing (it needs the new step-7 files below). The
parity-only state of Part 1 is kept as `resources/docs/agents/agentW_parity.patch` (the five parity files + `EdgeAnim.h`)
for a merge of Part 1 alone. Everything below is additive: new files plus `UnAnimPlay.cpp`, `EdgeAnim.h` (API
declarations appended) and the root `CMakeLists.txt` test line.

### Format spec

`resources/docs/edgeanim.md` §2/§2b (layouts, frame sets, intra-key bits, packing specs, the three key encodings, key
selection and interpolation, table padding) — everything there is confirmed by the bit-exact comparison below, not
only read from the decompiles. Retail facts that were not in the plan:
- the animation tag is `0x45413035` (bytes "50AE"), the skeleton tag `0x45533033` ("30SE");
- only two decoder pairs matter for the cooked data: `flags` is 3 (R and T bit-packed) in 4,698 of 4,704 surveyed
  sequences and 2 in the 6 `Doors_as` `Player_*` sequences (48-bit rotations); no sequence has scale or user channels
  or joint weights;
- Edge animation joint order is not the AnimSet track order (413 of 458 sequences differ), so the mapping has to go
  through the joint name hashes onto the mesh's Edge skeleton, whose joint *i* is `RefSkeleton(i)`;
- retail copies Edge joints into `LocalAtoms` without `FlipSignOfRotationW` (`UpdateSkelPoseEnd` 2012 0x354c10);
- the evaluator rounds *absolute* addresses inside a frame set (align 4/16), so the blob must be evaluated from a
  16-aligned base (retail copies frame sets keeping the source address mod 128);
- the stored `offsetLocomotionDelta` translation is the locomotion joint's parent-space translation at `duration`
  minus at 0 (checked on 790 sequences).

### Decoders (all in `Engine/Src/EdgeAnimEvaluate.cpp`, plain C++ with scalar SSE `rcp/rsqrt/sqrt/min/max` where retail uses them)

| Retail function | 2013 rva (2012) | Port | Verified by |
|---|---|---|---|
| `__edgeAnimEvaluate` (dispatcher, frame-set addresses, channel tables, spec walking) | 0x5dada0 (0x622a50) | `EdgeAnimEvaluate` | oracle, all data |
| `__edgeAnimEvaluateBitPackedConst` | 0x5cd3a0 (0x615050) | `EvaluateBitPackedConst` | oracle: 3,892 sequences with bit-packed const R, 3,166 with const T |
| `__edgeAnimEvaluateBitPacked` | 0x5ce380 (0x616030) | `EvaluateBitPacked` + `SlerpQuaternion` | oracle: 3,032 sequences animated R, 2,512 animated T |
| `__edgeAnimEvaluateRConst` | 0x5d4d90 (0x61ca40) | `EvaluateRConst` | oracle, 300 synthetic animations (no cooked sequence has such channels) |
| `__edgeAnimEvaluateR` | 0x5d5390 (0x61d040) | `EvaluateR` | oracle: the 6 `Doors_as` sequences + synthetic |
| `__edgeAnimEvaluateSTConst` / `ST` | 0x5d6ca0 / 0x5d6d80 (0x61e950 / 0x61ea30) | `EvaluateSTConst` / `EvaluateST` | oracle, synthetic (translations and scales) |
| `_edgeAnimProcessCommandList` fetch + evaluate stages (time → frame set, frame, fraction) | (0x6238c0) | `EdgeAnimLocateFrame` | oracle (same sample fed to both) |
| `AnimSkeletonGetJointIndexByHash` | 0x54de20 (0x58e9f0) | `EdgeAnimSkeletonGetJointIndexByHash` | all sequences resolve |
| `AnimLeafCallback` stage 0, mapping + base pose part | 0x54ee60 (0x58fb00) | `EdgeAnimEvaluateSkeletonPose` | poses finite/normalized, locomotion check |
| `UpdateSkelPoseEnd` joint → `LocalAtoms` copy | (0x354c10) | `FEdgeAnimSequencePose` (`AnimationEncodingFormat_EdgeAnim.cpp`) | builds; not reachable in-engine yet |

Not ported (not needed by the cooked data or outside Plan B): `*User*` decoders, Edge blends/mirroring, local↔world,
the leaf callback's locomotion extraction, the whole-tree job.

### Integration

`UAnimNodeSequence::GetAnimationPose` (`UnAnimPlay.cpp`): `ACF_EdgeAnim` → `FEdgeAnimSequencePose::GetAnimationPose` on
`SkelComponent->SkeletalMesh` at `CurrentTime` (channelled joints from the sequence, Edge base pose elsewhere, identity for
additive sequences, no W flip), then the reference root handling (`ExtractRootMotion`, `bZeroRootRotation`,
`bZeroRootTranslation`) and `GetCurveData`. `ExtractRootMotionUsingSpecifiedTimespan` reads the root of an Edge sequence
through `GetRootMotionBoneAtom` (Edge pose of bone 0) instead of `UAnimSequence::GetBoneAtom` (identity for Edge data).
`-edgerefpose` keeps the Part-1 reference-pose gate. `AnimationFormat_SetInterfaceLinks` stays retail (NULL codecs) —
deviation from the PHASE5 wording "select it in `SetInterfaceLinks`": the codec interface gets no mesh, Edge joints do
not follow the AnimSet tracks, and a non-NULL codec would change `UAnimSequence::Serialize` (edgeanim.md §5).

### Test: `source/Tests/EdgeAnimSmoke`

`EdgeAnimSmoke <data dir> [--retail <Dishonored.exe>] [--filter <s>] [--dump <s>] [--skeletons <dir>] [--synthetic <n>] [--verbose 1]`
links Core + the CoreSmoke stubs + `EdgeAnimEvaluate.cpp`; reads the `.anim`/`.skel` blobs written by
`resources/tools/pdb/extract_edgeanim.py`; with `--retail` it maps `Dishonored.exe` (base relocations applied,
`/BASE:0x10000000` keeps its own image out of the way) and calls the retail `__edgeAnimEvaluate` on the same frame set
as the port; `--dump` prints the joint transforms of named sequences (port and retail), `--synthetic` builds `flags = 0`
animations for the decoders the cooked data never selects. Exit code = failed checks.

Commands and results (build `build\agentW`, snapshot da24f65 + my files):
```
python resources/tools/pdb/extract_edgeanim.py <CookedPCConsole>/Engine.upk <..>/DishonoredGame.upk <..>/Startup.upk --out build/agentW/edgedata
python resources/tools/pdb/extract_edgeanim.py <..>/L_Prison_Script.upk <..>/L_Isl_Script_Master.upk <..>/L_Pub_Assault_P.upk <..>/L_OutsiderDream_Script.upk --out build/agentW/edgedata2
build\agentW_build.cmd EdgeAnimSmoke
build\agentW\Binaries\Win32\EdgeAnimSmoke.exe build\agentW\edgedata --retail D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\Dishonored.exe
  37 skeletons, 462 animations, 31754 samples; 13451232/13451232 floats bit-exact vs retail, max |diff| 0; max |quat|-1 0.000388
  (skeleton pose 0.000353); locomotion delta = root delta in 122/122 (parent space, max |d| 0.0334); 0/1885 checks failed
build\agentW\Binaries\Win32\EdgeAnimSmoke.exe build\agentW\edgedata2 --skeletons build\agentW\edgedata --retail <same> --synthetic 300
  90 skeletons, 2581 animations (2281 cooked + 300 synthetic), 195179 samples; 85020992/85020992 floats bit-exact vs retail,
  max |diff| 0; max |quat|-1 0.000421 (skeleton pose 0.000357); locomotion delta = root delta in 668/668 (max |d| 0.0408); 0/9814 checks failed
build\agentW\Binaries\Win32\EdgeAnimSmoke.exe build\agentW\edgedata --filter Npc_SmallRat_as --dump Npc_SmallRat_as.Npc_SmallRat_as.AnimSequence_0.anim --retail <same>
  (joint transforms of the rat's Jump_Attack_In at t = 0 and t = 0.0137, port and retail identical)
```
Totals: 2,743 cooked sequences, 213,117 samples, 96,758,504 floats + 1,713,720 synthetic floats, all bit-exact against the
retail evaluator (on this machine's `rcpps`/`rsqrtps`).

`DishonoredGame` builds with Plan B (`build\agentW_build.cmd DishonoredGame`, 0 errors); the smoke command of Part 1
still passes the animation load and stops at the same `UStaticMesh::Serialize` stall, so no `USkeletalMeshComponent`
ticks an Edge sequence in-engine yet — the in-engine check of the step-4 accept criterion waits for X/Z's map load
(then: `-nullrhi` run with an NPC in view, compare `-edgerefpose` vs default).

### Files (all mine)

New: `Engine/Inc/EdgeAnim.h` (Part 1 + API declarations), `Engine/Src/EdgeAnimEvaluate.cpp`,
`Engine/Inc/AnimationEncodingFormat_EdgeAnim.h`, `Engine/Src/AnimationEncodingFormat_EdgeAnim.cpp`,
`source/Tests/EdgeAnimSmoke/{CMakeLists.txt,EdgeAnimSmoke.cpp}`, `resources/tools/pdb/extract_edgeanim.py`,
`resources/docs/edgeanim.md`, `resources/docs/agents/agentW_parity.patch`. Changed: `Engine/Src/UnAnimPlay.cpp`,
root `CMakeLists.txt` (3 lines: `add_subdirectory(source/Tests/EdgeAnimSmoke)`, next to CoreSmoke; the file already
had another agent's uncommitted edits, which I left alone). Helper scripts (not repo tools): `build\agentW\*.py`
(`stack_sample.py` samples a running exe's thread stacks through the `.map`, `edge_probe.py`, `sync.py`, patch scripts).

### Remaining / follow-ups

1. In-engine check once the map loads (above); the first NPC in view is the real test of the mapping/W convention
   (data-level evidence: Edge base pose == `RefSkeleton`, `UpdateSkelPoseEnd` copies without W flip).
2. Additive composition: UE's additive nodes vs Edge's relative blend order is not verified (edgeanim.md §5).
3. `USkeletalMeshComponent::ExtractRootMotionCurve` (`UnSkeletalMesh.cpp:4695`, not mine) and the editor compression
   tools still call `UAnimSequence::GetBoneAtom`, identity for Edge data (as in retail).
4. `function_status.csv` rows (coordinator; shared file): `__edgeAnimEvaluate` 0x5dada0, `__edgeAnimEvaluateBitPackedConst`
   0x5cd3a0, `__edgeAnimEvaluateBitPacked` 0x5ce380, `__edgeAnimEvaluateRConst` 0x5d4d90, `__edgeAnimEvaluateR` 0x5d5390,
   `__edgeAnimEvaluateSTConst` 0x5d6ca0, `__edgeAnimEvaluateST` 0x5d6d80, `AnimSkeletonGetJointIndexByHash` 0x54de20 →
   `verified` (bit-exact oracle); `AnimLeafCallback` 0x54ee60 → `ported` (mapping/base pose part only);
   `AnimationFormat_SetInterfaceLinks` 0xc8250, `UAnimSequence::Serialize` 0x34c0d0, `UAnimSequence::PostLoad` 0x34c4a0 → `ported`.
5. Retail whole-tree Edge path (`BuildEdgeAnimTree` on the node classes, Edge blends, locomotion in the leaf callback):
   +8–12 days, the only route to retail-identical blending and root motion.
