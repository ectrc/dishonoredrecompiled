# Edge animation in Dishonored (retail 2013) — format, evaluator, integration

Agent W, 2026-09-25. Target = retail 2013 exe (`Dishonored_Latest2026`); 2012 Shipping decompiles are the readable
twin (every Edge function matches the 2013 exe byte for byte, `match_2012_2013.csv` ratio 1.000 "bytes").
Decompiles: `build\agentW\decomp2012\*.c`, `build\agentW\decomp2013\*.c` (from `shipping2012_agentW.i64` /
`retail2013_agentW.i64`). Data: `resources\tools\pdb\extract_edgeanim.py` (writes the Edge blobs of any package +
an `index.json`), probes in `build\agentW\edge_probe.py`.

## 1. What is cooked

| Object | Member | Content |
|---|---|---|
| `UAnimSequence` | `CompressedByteStream` | one `EdgeAnimAnimation` blob (tag `0x45413035` = bytes `35 30 41 45`, "50AE"), `TranslationCompressionFormat = RotationCompressionFormat = ACF_EdgeAnim (7)`, `KeyEncodingFormat = AKF_ConstantKeyLerp`, `CompressedTrackOffsets` empty, `RawAnimationData` empty. `UAnimSequence::Serialize` (2013 0x34c0d0) copies the blob raw (no codec). |
| `USkeletalMesh` | `m_EdgeSkeleton` (TArray<BYTE>, 2012 PDB @224, serialized between `RotOrigin` and `RefSkeleton`, Ver >= 775) | one `EdgeAnimSkeleton` blob, tag `0x45533033` ("30SE"), `sizeTotal == Num()` |

Census (extractor over `Engine`, `DishonoredGame`, `Startup` + `L_Prison_Script`, `L_Isl_Script_Master`,
`L_Pub_Assault_P`, `L_OutsiderDream_Script`): 4,704 Edge sequences, 2 non-Edge (`Ply_Empty_Locomotion_as…AnimSequence_0`
ACF_Fixed48NoW with track offsets, `SpringRazor_as.temp_placeholder_as.AnimSequence_0` ACF_Float96NoW), 97 meshes, all
with an Edge skeleton. Header `flags`: **3** (R and T bit-packed) for 4,698 sequences, **2** (T bit-packed, R not) for
the 6 `Doors_as` `Player_*` sequences. No sequence has scale or user channels, none has a joint-weight array; 773 are
additive (`bIsAdditive`); up to 6 frame sets; `sampleFrequency` 20–30 Hz (per sequence, = (NumFrames-1)/SequenceLength).

Checked for all 37 meshes of `Engine`/`DishonoredGame`/`Startup`: skeleton tag, `numJoints == RefSkeleton.Num()`,
`sizeTotal == m_EdgeSkeleton.Num()`, **Edge base pose == `RefSkeleton(i).BonePos` (quat and position, |d| < 1e-4, same W
sign)**, so Edge skeleton joint *i* is UE bone *i*. Every joint hash of the 462 Edge sequences of those packages exists
in a loaded skeleton.

## 2. Layouts (2012 PDB, `Engine/Inc/EdgeAnim.h` with static_asserts)

All `offsetX` fields are **self-relative** (`(BYTE*)&hdr->offsetX + hdr->offsetX`, 0 = absent).

`EdgeAnimAnimation` (96 bytes, then `WORD channelTables[]`): `tag`, `duration`, `sampleFrequency`, `sizeHeader` (bytes
before the first frame set, 336 in the rat sample), `numJoints` (joints the animation knows, in *animation* order),
`numFrames`, `numFrameSets` (**including a sentinel**), `evalBufferSizeRequired`, `numConst{R,T,S,User}Channels`,
`numAnim{R,T,S,User}Channels`, `flags` (bit0 R bit-packed, bit1 T, bit2 S, bit3 User), `sizeJointsWeightArray`,
`eaUserJointWeightArray`, `pad1`, `offsetJointsWeightArray`, `offsetFrameSetDmaArray`, `offsetFrameSetInfoArray`,
`offsetConst{R,T,S,User}Data`, `offsetPackingSpecs`, `offsetCustomData`, `sizeCustomData`, `offsetLocomotionDelta`.

`EdgeAnimSkeleton` (64 bytes, then `WORD simdHierarchy[]`): `tag`, `sizeTotal`, `sizeCustomData`, `sizeNameHashes`,
`numJoints`, `numUserChannels`, `numSimdHierarchyQuads`, `locomotionJointIndex`, `offsetBasePose`
(`EdgeAnimJointTransform[numJoints]`, 48 bytes each: quat xyzw, pos xyz + pad, scale xyzw), `offsetParentIndicesArray`
(SHORT), `offsetJointNameHashArray` (DWORD), user-channel arrays, `offsetCustomData`.

Channel tables (`channelTables`, WORD = animation joint index of each channel), in this order and padding:
`constR` (numConstR rounded up to **8**), `constT`, `constS`, `constUser`, `animR`, `animT`, `animS`, `animUser`
(each rounded up to **4**).

Custom data (`offsetCustomData`, `sizeCustomData`): if `offsetLocomotionDelta != 0` it starts with 32 bytes (the
locomotion joint's first-frame transform: quat xyzw, pos xyz, pad) — then `DWORD jointNameHash[numJoints]` (animation
joint → skeleton joint via `AnimSkeletonGetJointIndexByHash`), padded to 16. `offsetLocomotionDelta` points at 32 bytes
(quat + pos: the root delta over one loop).

Frame sets: `EdgeAnimFrameSetInfo[numFrameSets]` = {`baseFrame`, `numIntraFrames`}; `FrameSetDma[numFrameSets]`
(8 bytes: WORD ?, WORD `size`, DWORD `offset` from the blob start). The last entry is a sentinel (`baseFrame` = last
frame, size 0). Frame set *k* covers frames `baseFrame .. baseFrame + numIntraFrames + 1`: an initial key, `numIntraFrames`
intra frames, a final key (= next set's initial key). `numFrameSets == 1` means constant channels only.

Frame set data (16-byte aligned; addresses below are rounded on the *absolute* address, so evaluate from a 16-aligned
copy of the blob): 8 WORD sizes `s0..s7`, then
`initialR [s0] | initialT [s1] | initialS [s2] | initialUser [s3] | intraBits | intraR [s4] | intraT [s5] | intraS [s6] |
align4 | intraUser [s7] | align16 | 8 WORD sizes f0..f7 | finalR [f0] | finalT [f1] | finalS [f2] | finalUser`.
`intraBits` = `(numIntraFrames * (numAnimR + numAnimT + numAnimS + numAnimUser) + 7) >> 3` bytes: one bit per channel
per intra frame (channel-major: R channels first, then T at bit `numIntraFrames*numAnimR`, S, User) saying whether the
channel has a key in that intra frame.

## 2b. Channel encodings (ported in `Engine/Src/EdgeAnimEvaluate.cpp`, bit-exact against retail)

All key data is **big-endian** (the SPU layout; the x86 build byte-swaps on load) except the constant/animated float
vectors of the non-bit-packed translation/scale decoders, which are little-endian floats. Headers, tables, specs and
frame-set sizes are little-endian.

**Bit-packed (`flags` bit set; `__edgeAnimEvaluateBitPackedConst` / `__edgeAnimEvaluateBitPacked`).** A packing spec
DWORD per channel (one shared by all constant channels of a kind) gives, per component x/y/z, a sign bit count (1 bit
field), an exponent bit count E (4 bits) and a mantissa bit count M (5 bits): x = bits 31 / 27..30 / 22..26,
y = 21 / 17..20 / 12..16, z = 11 / 7..10 / 2..6; bits 0..1 = the omitted quaternion component (rotations). A key is
`[sx ex mx][sy ey my][sz ez mz]`, MSB first, packed back to back in a big-endian bit stream (key size = sum of the
nine counts; channel *c*'s initial/final key starts at the sum of the key sizes of channels 0..c-1, its intra keys at
the sum over channels 0..c-1 of `popcount(intraBits) * keySize`). A component decodes as
- E == 0: fixed point, `(float)(int)(m | (s ? ~0 << M : 0)) * (1 / (2^M - 1))`, the reciprocal as `rcpps` + one
  Newton step `(1 - r*d)*r + r` (0 when M == 0);
- E > 0: an IEEE float `s << 31 | (e + 128 - 2^(E-1)) << 23 | m << (23 - M)` (SSE shift semantics: a count of 32 or
  more gives 0).
Rotations: the three decoded values fill the non-omitted slots of (x, y, z, w) in order, the omitted one is
`sqrt(min(max(((1 - a*a) - b*b) - c*c, 0), 1))`. Translations/scales get w = 1.

**48-bit rotations (`flags & 1 == 0`; `RConst` / `R`).** 6 bytes per key: BE16 `a`, BE32 `b << 17 | c << 2 | omitted`,
each component `(float)v * 4.3159689e-05 + -0.70710677` (15 bits over ±1/√2), the omitted one `sqrt(((1 - a²) - b²) - c²)`
(no clamp), same slot placement.

**Float vectors (`flags & 2/4 == 0`; `STConst` / `ST`).** 12 bytes per key, three little-endian floats, w = 1.

**Constant channels** write one value per channel. **Animated channels** pick, per channel, the key at or before the
sample and the key after it: with F = `frameInteger` and the channel's intra bits (bit k = key at frame k+1), the left
key is the last intra key with k < F (else the initial key, frame 0), the right key the first intra key with k >= F
(else the final key, frame `numIntraFrames + 1`); `t = ((float)(F - left) + frameFraction) / ((float)(right - F) + (float)(F - left))`.
Translations/scales: `t*R + (1-t)*L`. Rotations: the slerp of the bit-packed decoder —
`d = ((Ry*Ly + Rx*Lx) + Rz*Lz) + Rw*Lw`, `s = d < 0 ? -1 : 1`, `c = s*d`, `θ = (1.5707963 - (0.2145988 -
(0.088978991 - (c*0.050174303)*c)*c)*c) * (r - r*c)` with `r = rsqrtps(1 - c)` (no refinement), `sin x ≈
((((x²*2.7526e-6 + -1.98409e-4)*x² + 8.3333319e-3)*x² + -0.16666667)*x² + 1)*x`, `1/sin θ` by `rcpps` + Newton,
weights `wL = s * sin((1-t)θ)/sin θ`, `wR = sin(tθ)/sin θ` (linear `1-t`, `t` when `c > 0.999`), `q = wR*R + wL*L`
(not renormalized: |q| - 1 up to 4e-4 on the cooked data). The approximate `rcpps`/`rsqrtps` make the exact bits
CPU-dependent, as in retail; the port uses the same instructions so it matches retail on the machine it runs on.

Padding: each channel table is padded (constant R to 8, the others to 4) with the joint index `numJoints`; retail
writes those padded lanes into a scratch joint `numJoints` (hence the `numJoints + 1` joints of the job output), the
port skips them.

## 3. Evaluation (retail call chain)

`USkeletalMeshComponent::UpdateSkelPoseBegin` (2012 0x3737a0) → `SetupEdgeAnimJob` (0x372120: `UAnimNode::GetEdgeAnimTree`
→ `BuildEdgeAnimTree` on the node classes; a `UAnimNodeSequence` leaf = `BuildEdgeAnimTreeLeaf` 0x1a46b0:
`EdgeAnimBlendLeaf{animationHeaderEa = CompressedByteStream.GetData(), evalTime = CurrentTime, userVal = FLocomotionState*}`)
→ job `FEdgeAnimJobDesc::Process` (0x592ff0: `edgeAnimSpuInitialize`, `edgeAnimProcessBlendTree` with
`AnimLeafCallback`/`AnimUserCallback`, copies `numJoints + 1` joints out) → `UpdateSkelPoseEnd` (0x354c10).

Per leaf, `_edgeAnimProcessCommandList` (0x6238c0) does, in three pipeline stages:
1. header fetch; tag check (`"edgeAnim: unexpected animation tag"`).
2. `frame = max(evalTime * sampleFrequency, 0)`, binary search in `FrameSetInfo[0 .. numFrameSets-1)` for the last set
   with `baseFrame <= (WORD)frame`, fetch its data (`FrameSetDma`); `AnimLeafCallback` stage -1 copies the custom data.
3. `rel = frame - baseFrame; frameInteger = (int)rel; frameFraction = rel - frameInteger; if frameInteger > numIntraFrames
   { frameInteger = numIntraFrames; frameFraction = 1 }` → `__edgeAnimEvaluate(joints, userChannels, header, frameSetData,
   numIntraFrames, frameInteger, frameFraction)` (0x622a50 / 2013 0x5dada0), writing `EdgeAnimJointTransform` per
   *animation* joint (R at +0, T at +16, S at +32) for channelled joints only.
   `__edgeAnimEvaluate` dispatches: `flags & 1` ? `BitPackedConst`+`BitPacked` : `RConst`+`R` for rotations;
   `flags & 2` ? bit-packed : `STConst`+`ST` for translations (output + 16); `flags & 4` the same for scales (+32);
   `flags & 8` the user channels. The bit-packed pair consumes one `packingSpec` DWORD for the const channels and one
   per animated channel (advance `4 * numAnim + 4`).
4. `AnimLeafCallback` (0x58fb00) stage 0: maps animation joints to skeleton joints by hash (`AnimSkeletonGetJointIndexByHash`
   0x58e9f0, linear search with a rolling hint), builds per skeleton joint `{animJoint, useRefRot, useRefTrans}`
   (= `FEdgeSkelToAnimMapping`; a skeleton joint uses the reference rotation unless a const or animated R channel targets
   it, likewise T), copies the skeleton base pose and overrides the channelled R/T, then the locomotion joint: root
   motion extraction against the previous `FLocomotionState` with the `RootBoneOption`/`RootRotationOption` bits that
   `BuildEdgeAnimTreeLeaf` packed into `m_LoopCount`'s neighbour word (flags at +40: bit0 first update, bit1 looping,
   bits 2..13 the six 2-bit options), writing the delta into the pose's extra joint.

`UpdateSkelPoseEnd` then writes `LocalAtoms(bone) = {rotation = joint.rotation, translation = joint.translation, scale 1}`
for every required bone **without `FlipSignOfRotationW`** (the Edge quaternions are already in the RefSkeleton convention),
and `ExtractedRootMotionDelta` from joint `numJoints` (the extra joint), `bHasRootMotion = joint[numJoints].translation.w > 0`.

## 4. Minimal function set for a per-sequence evaluator (dependency order)

| Function | 2012 rva | 2013 rva | Size | Needed for the cooked data |
|---|---|---|---|---|
| `AnimSkeletonGetJointIndexByHash` | 0x58e9f0 | 0x54de20 | 122 | yes |
| `AnimLeafCallback` (mapping + base pose part; locomotion part later) | 0x58fb00 | 0x54ee60 | 13,540 | yes |
| `__edgeAnimEvaluate` (dispatcher) | 0x622a50 | 0x5dada0 | 1,797 | yes |
| `__edgeAnimEvaluateBitPackedConst` | 0x615050 | 0x5cd3a0 | 4,051 | yes (R and T, flags 3) |
| `__edgeAnimEvaluateBitPacked` (the core) | 0x616030 | 0x5ce380 | 17,976 | yes (R and T) |
| `__edgeAnimEvaluateRConst` | 0x61ca40 | 0x5d4d90 | 1,536 | selected by the 6 `Doors_as` `Player_*` sequences (flags 2) but with 0 constant channels |
| `__edgeAnimEvaluateR` | 0x61d040 | 0x5d5390 | 6,408 | the 6 `Doors_as` sequences |
| `__edgeAnimEvaluateSTConst` / `ST` | 0x61e950 / 0x61ea30 | 0x5d6ca0 / 0x5d6d80 | 218 / 4,418 | only called with 0 channels (scale slot; no sequence has flags & 2 == 0) |
| `*User*`, `_edgeAnimBlend*`, `MirrorJoints`, `LocalJointsToWorldJoints`/`WorldJointsToLocal` | | | | no (no user channels; blending stays UE's CPU tree) |

Drops out on PC for a per-sequence codec: `edgeAnimSpuInitialize` local-store budget (`FEdgeAnimJobDesc::Process`
98,304-byte scratch), `_edgeAnimCopyQuadwords`, EA/DMA fields (`animationHeaderEa`, `eaUserJointWeightArray`, the
`FrameSetDma` copy becomes a pointer), `FJobBuffer` rounding, the pose stack / pose caches, `FDisJobRunner`.

## 5. Integration (implemented, Plan B)

Per-sequence evaluation on top of `__edgeAnimEvaluate*` + `m_EdgeSkeleton`, keeping the reference CPU blend tree:

- `Engine/Src/EdgeAnimEvaluate.cpp` (engine-independent, Core only): `EdgeAnimLocateFrame` (the fetch/evaluate stages
  of `_edgeAnimProcessCommandList`), `EdgeAnimEvaluate` (`__edgeAnimEvaluate` and the six decoders),
  `EdgeAnimSkeletonGetJointIndexByHash`, `EdgeAnimEvaluateSkeletonPose` (the mapping/base-pose part of `AnimLeafCallback`;
  copies a frame set to a 16-aligned scratch when the blob is not 16-aligned, because the evaluator rounds absolute
  addresses).
- `Engine/Src/AnimationEncodingFormat_EdgeAnim.cpp` (`FEdgeAnimSequencePose`): the `UpdateSkelPoseEnd` copy into
  `FBoneAtom`s (rotation/translation as they are, scale 1, **no W flip**) for `DesiredBones` on the component's mesh.
- `UAnimNodeSequence::GetAnimationPose` (`UnAnimPlay.cpp`): an `ACF_EdgeAnim` sequence goes through
  `FEdgeAnimSequencePose`, then the reference root handling (`ExtractRootMotion`, `bZeroRootRotation/Translation`);
  root motion reads the root at other times from the Edge pose (`GetRootMotionBoneAtom`), since `UAnimSequence::GetBoneAtom`
  is identity for Edge data (retail takes root motion from the Edge locomotion joint instead).
  `-edgerefpose` (or a mesh without Edge skeleton) keeps the step-4 reference pose.
- `AnimationFormat_SetInterfaceLinks` stays retail (`case 7: break;`, NULL codecs): the codec interface only sees the
  sequence and AnimSet track indices, while the Edge mapping needs the mesh's Edge skeleton (Edge joint order differs from
  the AnimSet track order in 413 of 458 checked sequences), and a non-NULL codec would change `UAnimSequence::Serialize`.
- The mapping is recomputed per evaluation, as retail does (hash lookup with a rolling hint, O(numJoints)).
- Missing channels: base pose for normal sequences, identity for additive ones (the cooked additive sequences hold deltas:
  identity rotations/zero translations where nothing moves). Retail blends additive leaves with Edge's relative blend;
  whether UE's `UAnimNodeAdditiveBlending` composes in the same order is not verified.

Not ported (retail-only behaviour): the whole-tree Edge job (`BuildEdgeAnimTree`, Edge blends/mirroring, the node cache),
locomotion extraction in the leaf callback (`FLocomotionState`, the six root option bits), `UAnimationCompressionAlgorithm_EdgeAnim`
(editor). The retail whole-tree path is +8–12 days and the only route to retail-identical blending.

## 6. Verification (source/Tests/EdgeAnimSmoke, results in agents/agentW.md)

1. Skeleton: tag, `numJoints == RefSkeleton.Num()`, base pose == `RefSkeleton` BonePos, joint hashes (§1): 37 + 53 skeletons.
2. Animation joint hashes resolve in a loaded skeleton: every sequence of both data sets (with the Startup skeletons).
3. Oracle: `Dishonored.exe` is mapped and relocated in-process and its `__edgeAnimEvaluate` (2013 rva 0x5dada0) is called
   on the same frame set as the port, `2 * numFrames + 1` samples per sequence (every frame, every frame + 0.37, the
   duration): 2,743 cooked sequences, 213,117 samples, 96,758,504 floats, **all bit-exact**; 300 synthetic `flags = 0`
   animations (constant/animated 48-bit rotations, float translations and scales, random intra-key bits) add 1,713,720
   floats, all bit-exact.
4. Skeleton poses (`EdgeAnimEvaluateSkeletonPose`) at every frame: all finite, |q| - 1 <= 3.6e-4.
5. Locomotion: for all 790 sequences with `offsetLocomotionDelta`, the stored delta translation equals the evaluated
   locomotion-joint translation at `duration` minus at 0, in the parent space (max 0.04 units, from quantization).
6. The 2 non-Edge sequences keep their UE codecs (`SetInterfaceLinks` unchanged for formats 0..6).

## 7. Decision memo — whole-tree Edge path vs Plan B (agent AK, wave 4, 2026-09-26)

**Question.** Retail evaluates the *whole* animation tree of a skeletal mesh component as one Edge job
(`UAnimNode::GetEdgeAnimTree` → `BuildEdgeAnimTree` per node class → `FEdgeAnimJobDesc::Process` →
`edgeAnimProcessBlendTree`, §3). We ship Plan B (§5): every `ACF_EdgeAnim` leaf is decoded by the ported evaluator into
`FBoneAtom`s and the reference UE3 CPU blend tree composes the pose. Do we port the whole-tree path in wave 5?

### 7.1 What each path does

| Stage | Retail whole-tree job | Plan B (HEAD) | Same result? |
|---|---|---|---|
| Leaf decode (`__edgeAnimEvaluate*`) | Edge, per leaf inside the job | the same functions, ported (`EdgeAnimEvaluate.cpp`) | **yes, bit-exact** (2,743 sequences, 213,117 samples, 96.8 M floats, §6.3) |
| Animation joint → skeleton joint, base pose | `AnimLeafCallback` stage 0 | `EdgeAnimEvaluateSkeletonPose` (same mapping, same rolling-hint hash search) | yes at data level: joint *i* == bone *i*, base pose == `RefSkeleton`, poses finite, \|q\|-1 ≤ 3.6e-4 (§6.1/6.4); **in-engine on an NPC still pending** (7.3) |
| Copy to `LocalAtoms` | `UpdateSkelPoseEnd`, no `FlipSignOfRotationW` | `FEdgeAnimSequencePose`, no W flip | yes by construction; the in-engine check is the confirmation |
| Blend of two poses (`UAnimNodeBlend*`, crossfades) | Edge `_edgeAnimBlend*` (linear joint blend with quaternion sign alignment, renormalized per joint) | UE3 `BlendFBABuffers` / `FBoneAtom::Blend` (lerp + `FastLerp` quats, shortest arc, normalize) | numerically close, not identical: both are normalized lerps but with different sign-alignment rules on the `w` side and different `rsqrt` precision; **not measured** |
| Additive layers (773 additive sequences) | Edge relative blend: `_edgeAnimBlendAdditive` applies the delta on the base in *Edge's* order | `UAnimNodeAdditiveBlending` → `ApplyAdditiveAnimation` (UE order: delta rotation × base rotation, translations added) | **order not verified** (§5, W follow-up 2). A mismatch shows as a mirrored/over-rotated additive on joints whose base rotation is far from identity (aim offsets, breathing, hit reactions), not as garbage |
| Mirroring (`UAnimNodeMirror`) | Edge `MirrorJoints` on the pose | UE3 mirror table on `FBoneAtom`s | equivalent design; retail's is the mesh's `SkelMirrorTable` fed to Edge; **not compared** |
| Root motion | locomotion joint delta against `FLocomotionState`, six 2-bit root options packed by `BuildEdgeAnimTreeLeaf` | `ExtractRootMotion` on the Edge pose root (`GetRootMotionBoneAtom`), `bZeroRootRotation/Translation` | retail extracts against the *previous* evaluation state and the loop wrap; Plan B differentiates the pose. Same value while a sequence plays forward without a wrap (§6.5: the stored locomotion delta equals the evaluated joint delta over one loop, ≤ 0.04 units); **loop-wrap and blend-weighted root motion differ** |
| Node cache / pose stack | Edge pose stack, `numJoints + 1` joints, 98,304-byte scratch | none (UE atoms) | not a correctness item |

### 7.2 Cost of the whole-tree path (W's estimate, re-checked against the decompile sizes)

+8–12 days for one agent: `BuildEdgeAnimTree` on every node class that overrides it (the 2013 vtables of
`UAnimNode`, `UAnimNodeBlendBase`, `UAnimNodeBlend`, `UAnimNodeBlendList`, `UAnimNodeBlendPerBone`,
`UAnimNodeBlendMultiBone`, `UAnimNodeAdditiveBlending`, `UAnimNodeMirror`, `UAnimNodeSlot`, `UAnimNodeSequence*`,
`UAnimNodeAimOffset`, `UAnimNodeSynch`, `UAnimNodeScalePlayRate`, the Arkane `UDis*`/`UArk*` nodes),
`FEdgeAnimJobDesc::Process` (job setup, scratch budget, `_edgeAnimCopyQuadwords`), `edgeAnimProcessBlendTree` and the
`_edgeAnimBlend*` / `MirrorJoints` / `LocalJointsToWorldJoints` kernels (SSE, same oracle method as §6.3), the locomotion
half of `AnimLeafCallback` (13,540 bytes) with `FLocomotionState`, plus the `UAnimNodeSequence` layout bits it reads
(`m_LoopCount`'s neighbour word). All of it is verifiable with the existing oracle (`Dishonored.exe` mapped in-process,
`source/Tests/EdgeAnimSmoke`): the blend kernels take plain joint arrays, so bit-exactness is testable per kernel before
any in-engine run.

### 7.3 In-engine check (step 5 of AK's package)

Requires an NPC in a loaded map: `L_Tower_P` through AF's `-startmap`, on AD's serializers. At the time of writing
neither has landed in the shared tree (AD's `Bad export index` in `Dishonored_MainMenu_Env.upk` still ends every null-RHI
run 0.5 s after `Initial startup`), so the check is **pending**. Procedure when it can run (AK snapshot with AD+AF files,
`build_and_smoke.py ... --extra-args "-startmap=L_Tower_P"`, then the same with `-edgerefpose`):

1. Both runs tick 30 s in `L_Tower_P` with no assert from `UnAnimPlay.cpp` / `AnimationEncodingFormat_EdgeAnim.cpp`.
2. `DISHONORED(bringup)` probe (to add in `FEdgeAnimSequencePose`, one warn-once per mesh): every `LocalAtoms` rotation finite,
   \|q\|-1 < 1e-3, root translation within the mesh bounds; the same mesh under `-edgerefpose` reports the reference pose.
3. `w` sign: the first frame of the first NPC sequence per mesh compared with `RefSkeleton` (`dot(q_anim, q_ref) > 0` for the
   ≥ 90 % of joints an idle barely moves); a systematic negative dot on all joints would be the W-flip bug.

### 7.4 Recommendation for wave 5

**Keep Plan B for wave 5.** Reasons, in order:

1. Milestone 5/6 do not depend on blend parity: player movement is `PHYS_Walking` against collision (PHASE6.md facts), the
   possess/input/save-load paths never read a pose, and NPC AI reads root motion only through the same `ExtractRootMotion`
   contract Plan B already serves. Every stage that can *break* (decode, mapping, W convention) is proven bit-exact or
   data-exact; what remains (blend/additive order, loop-wrap root motion) degrades quality, not stability.
2. The evidence that would justify +8–12 days is visual, and nothing renders an NPC before AG/AH's D3D9 world frame lands.
   Deciding now would be deciding blind; the 7.3 check plus one look at an additive-heavy NPC (a guard aiming, a
   weeper's idle) on the rendered frame is the cheap experiment that tells whether the additive order is wrong.
3. The whole-tree job is a self-contained package with its own oracle, so it does not get cheaper or dearer by waiting; it
   gets *safer* once the map runs and an NPC can be watched.

**Triggers that flip the decision to "port the whole tree" (wave 5 or 6):** (a) the 7.3 check or the first D3D9 NPC shows
an additive/blend artefact that `UAnimNodeAdditiveBlending`'s order cannot explain away with a one-line swap; (b) an NPC
behaviour that depends on root motion across loop wraps or on blend-weighted root motion (synchronized takedowns, ledge
mantles, `DisSynchronizedAnim`-style sequences) misplaces the pawn; (c) the multiplayer end goal (dismod) needs
pose/root-motion determinism identical to retail across clients (cosmetic poses do not; root-motion-driven pawn positions
would). Absent a trigger, the whole-tree path is the *last* animation item, after the renderer shows the world.

**Small wave-5 items that stay under Plan B (≤ 1 day total):** verify the additive order once against
`_edgeAnimBlendAdditive` in the 2013 decompile and fix `ApplyAdditiveAnimation`'s multiply order if it differs; the
7.3 probe; `USkeletalMeshComponent::ExtractRootMotionCurve` (W follow-up 3) reading the Edge locomotion delta instead of
`GetBoneAtom` identity.
