# Agent FX3 - the FaceFX runtime and the engine integration: lip-sync on (wave 26, PHASE23 package FX3)

Worktree `build/agentFX3_wt` (made at `7845fca` = `b9eb647` + the "FX3 launched" line), build dir `build/agentFX3`
(RelWithDebInfo), staged as `DishonoredGame_FX3.exe`. Two DishonoredGame builds: build 1 failed on one line
(`FString::Num` is protected), build 2 = 0 errors, the measured one. CoreSmoke builds through `build_queued.py` (3, not
counted). IDA on private copies `build/agentFX3/idb/{shipping2012,retail2013}_agentFX3.i64`; engine decompiles in
`build/agentFX3/dec2012/`, `dec2013/`; FX1's SDK decompiles reused (`build/agentFX1/dec_by_obj/`). Runs through
`build/agentFX3/fx3_run.py` (IR's runner) and `cutscene_compare.py`; no retail exe, no real input, no FModel.

## 1. Headline

**Lip-sync is on: the speakers' jaws and lips move with their lines, mouths open with teeth showing, and the voice is
posted from `UpdateFaceFX` as retail posts it.**

- The SDK runtime is ported (FxActor, FxActorInstance, FxMasterBoneList, FxPhonemeMap/Enum, actor load/save): with FX1's
  core and FX2's graph, **803 of the 1,478 FaceFX functions are ported by rva, 673 more are templates / class macros /
  compiler generated / header inlines, 2 are not ported** (FX2's anim-set file load/save). FX2's shim is deleted.
- `WITH_FACEFX=1`; every module links `facefx`; `UnInitFaceFX` starts the SDK with retail's policy (custom, no pools,
  appMalloc/appFree, the pre-destroy / pre-compile callbacks).
- Engine: `UpdateFaceFX` (2013 0x325ea0), `PlayFaceFXAnim` (0x356ef0), `GetFaceFXAnimStartOffset` (0x357070),
  `StopFaceFXAnim`, `execPlayFaceFXAnim` (Arkane's 5 parameters), `Attach`'s `SetFaceFXAsset(m_pFaceFXAsset)`, the
  `UpdateSkelPoseEnd` call site (`m_pFaceFXAsset`, DeltaTime). Game: `PlayDialogBlurb`'s FaceFX branch,
  `IsDonePlayingBlurb`, `StopDialogBlurb`, `DisFaceFxStartAudio` (0x8a0120), `OnSpeakerStartedTalking` (0x89e610), the
  look-at's `m_bFaceFxIsPlaying`.
- **CoreSmoke 568 passed, 0 failed, 1 skipped** (532 before + 36 FX3): the elite guard's head loads as an FxActor to the
  last byte and re-saves byte for byte (all **44 heads** too), the 'SheWaiting' anim set mounts, an instance plays the
  anim, `APS_StartAudio` comes once on the first tick at anim time >= 0, and at anim times 0.5 / 1.0 / 1.5 s the 'open'
  node and the jaw / lower-lip / head bones equal an independent Python derivation (`fx3_handcalc.py`, from the archives
  via `facefx_graph_dump.py`) to 1e-4: jaw_jnt rot.y reference -0.0519 -> -0.0065 / **+0.0527** / +0.0238 (open .456 at 1.0 s).

## 2. Measurements

Runs (`DishonoredGame/Logs/FX3*.log`), all `-nomovie -finewgame=50 -fosoiree -iddiag -irdiag -fx3diag`:

| run | exe | extra | what |
|---|---|---|---|
| FX3a | FX3 | `-irface=40`, 5 shots 163.4-165.4 | first SheWaiting; every ride line through FaceFX; exit clean |
| FX3d | FX3 | `-irface=40`, 36 shots over two SheWaiting lines | per-frame jaw values, mouth-open captures |
| FX3b | FX3 | `-fwteleport=DisTrigger_16@160 -irface=35`, 36 shots 166-201 | Empress scene (Empress `_5`, Lord Regent `_7`) |
| FX3c | FX1's gate exe (WITH_FACEFX=0, the base) | none | control for the voice / line times |
| CC fx3 | FX3 | cutscene_compare | ride SSIM, captain frame pairs |

**SetFaceFXAsset** binds an instance on every NPC body (`fx3diag SetFaceFXAsset ... instance 1`): Skm_EliteGuard_Head
(118 ref bones) on the 118-bone elite body, every other head 33 ref bones on 109-118-bone bodies. Blend mode 1
(additive) on all 57 plays.

**SheWaiting (elite guard `_27`)**, FX3d, per frame (`fx3diag ... bones`; jaw angle against the mesh reference pose):

```
w 161.016 anim -0.019 open 0.227 jaw  5.94 deg     w 162.601 anim 1.566 open 0.393 jaw 10.13 deg
w 161.818 anim  0.782 open 0.344 jaw  9.05 deg     w 163.031 anim 1.996 open 0.487 jaw 12.83 deg
w 162.007 anim  0.971 open 0.487 jaw 12.82 deg     w 163.808 anim 2.773 open 0.000 jaw  0.04 deg (closed)
second line: w 172.023 anim 0.979 open 0.480 jaw 12.64 deg; w 173.022 anim 1.978 open 0.515 jaw 13.56 deg
```
lips_low_c_jnt turns up to 19.8 deg. Captures: `build/agentFX3/shots/fx3d_w173.02.png` (mouth open, teeth and tongue),
`fx3a_w*`/`apshottime_fx3a00000.png` (open, w163.40) vs `apshottime_fx3a00001.png` (closed, w163.90),
`fx3d_sheet.png` (36 frames with their open / jaw values), `fx3b_sheet.png` (Lord Regent and Empress; the Empress at
w176 open 0.49 shows her teeth). Before (IR): every jaw/lip bone `d=0.00 r=0.0` through every line.

**Voice and line times** (`build/agentFX3/py/blurb_table.py`, FX3a vs control FX3c; blurb start / voice post / done):

| line | control start -> post, done | FX3 start -> post (anim lead-in), done |
|---|---|---|
| captain 'Steady hand' | 56.35 -> 56.35, 58.97 (+2.62) | 56.45 -> 56.70 (0.255), 59.74 (+3.29) |
| captain 'Take us straight...' | 68.35 -> 68.35, 73.27 (+4.92) | 68.44 -> 68.74 (0.302), 74.05 (+5.61) |
| elite 'A long way...' | 74.91 -> 74.95, 80.49 (+5.58) | 74.99 -> 75.22 (0.255), 81.97 (+6.98) |
| 'Home at last!' | 129.42 -> 129.42, 130.76 (+1.34) | 129.52 -> 130.69 (1.179), 133.33 (+3.81) |
| SheWaiting | 160.67 -> 160.67, 163.42 (+2.75) | 160.70 -> 161.09 (0.359), 164.13 (+3.43) |

Blurb starts are unchanged (matinee keys, +0.03-0.1 s run jitter); the voice durations are identical (AK_Duration). The
**voice is now posted when the FaceFX anim reaches time 0** - 0.20-1.18 s after the blurb starts, the anim's own lead-in -
and the **line ends when the anim ends** (`IsDonePlayingBlurb` waits for `IsPlayingFaceFXAnim`), 0.1-2.5 s later than the
voice's end. Both are retail's code (PlayFaceFXAnim 0x356ef0 ticks with position = start offset + seek; UpdateFaceFX posts
on `APS_StartAudio`; IsDonePlayingBlurb 0x89a160). Subtitles: off in this config (`subtitles enabled 0`), so neither path
draws one; with them on the subtitle comes from `OnSpeakerStartedTalking` at the voice post.

**Ride** (CC fx3): soirée ENTER 56.54 / EXIT 150.79 (ride unchanged). SSIM at the constant offset (IK's rule,
-42.831 + (56.40 - 56.54) = -42.971, `build/cmp/fx3_const`): **0.819**, MAD 7.97 (the tool's slope fit: 0.658). Captain at
w71-73 (`build/agentFX3/shots/captain_w0072.050_retail_left_ours_right.png`): both mouths open mid-line (retail's a little
wider in that frame). At ride distance the mouth is a few pixels, so SSIM barely sees lip-sync; .819 is within the
.82-.84 spread of the last runs on main.

## 3. What was ported (files)

| file | content |
|---|---|
| NEW `External/FaceFX/include/FxActor.h`, `FxActorInstance.h`, `FxMasterBoneList.h`, `FxPhonemeMap.h` | layouts + header inlines (FindAnimGroup 0x238ac0, GetAnimGroup 0x238c20, GetAnimPtr 0xb0ba10, SetRefBoneCurrentWeight, ResetRefBoneWeights, _findMasterBoneListEntryIndex, the inlined bone blend as `GetBlendedBone`) |
| NEW `External/FaceFX/Src/Runtime/*.cpp` | FxActor.obj, FxActorInstance.obj, FxMasterBoneList.obj, FxPhonemeMap.obj, FxPhonemeEnum.obj by 2012 rva |
| `FxVec3.h`, `FxBone.h`, `FxUtil.h`, `FxGenericTargetNode.h`, `FxFaceGraphPrerequisites.h`, `FxSDK.cpp`, `FxName.cpp` | the operators FxMasterBoneList.obj emitted, FxDRealEquality, the UE3 node property slots, FX_INT32_MAX; stub branch gone |
| DELETED `include/stub/*`, `Src/Stub/*` | FX2's core shim |
| `cmake/FaceFX.cmake`, `cmake/DishonoredDefines.cmake` | Src/Runtime; WITH_FACEFX per target + link; WITH_FACEFX_STUDIO=0 |
| `Engine/Inc/UnFaceFXSupport.h`, `UnSkeletalMesh.h`; `Engine/Src/UnSkeletalComponent.cpp`, `UnFaceFXSupport.cpp`, `UnFaceFXAsset.cpp`, `UnFaceFXAnimSet.cpp`, `UnInterpolation.cpp`, `UnPawn.cpp`, `UnSkeletalMesh.cpp` | the engine side (section 1), `-fx3diag` |
| `DishonoredGame/Src/disconversationcomponent.cpp`, `disconvglobalman.cpp`, `Inc/CppText/UDisConvGlobalMan.h`, `arkcomponentlookat.cpp`, `dishonorednpcpawn_body.cpp` | the game side |
| NEW `source/Tests/CoreSmoke/FaceFXRuntimeSmoke.cpp`, `CMakeLists.txt`, `CoreSmoke.cpp` | the runtime checks |
| `resources/docs/facefx/fx3_inventory.csv`, `facefx_program.md` | 342 rows (SDK 293 + engine 49); the FX3 section with the 1,478 tally |

Generated headers: none change - `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir
build/agentFX3/regen` reproduces all 79 files byte for byte (CR ignored); `bitfield_audit.py` mismatches=0.

Named, not ported: `FixupReferencedSoundCues` (asset / anim set; editor only), `APawn` / `ASkeletalMeshActor::
PlayActorFaceFXAnim` (script and matinee FaceFX; needs a generated-header override; AActor's FALSE still answers),
`UWorld::MountPersistentFaceFXAnimSetOnPawn`, `UDisDialogVoiceData::GetFaceFXAnimSetName`. Deviations: see the banners and
facefx_program.md (FX3 section).

## 4. Where the brief was wrong (measured)

1. **"the voice start time unchanged vs ID/IH2's"** - it moves, by retail's design: the voice is posted when the anim reaches
   time 0, 0.20-1.18 s after the blurb (SheWaiting +0.39 s; the anim starts at -0.359 and PlayDialogBlurb passes seek -1,
   so the position is -1.359 and the first tick does not start the audio). And **"the line's end unchanged"** - it ends
   with the anim (SheWaiting +3.43 s from start vs +2.75), because retail's IsDonePlayingBlurb waits for
   IsPlayingFaceFXAnim. Blurb starts and voice lengths are unchanged.
2. **"coresmoke_passed >= 529 + yours"** - this tree counted 532 before FX3 (FX1 recorded 532 in its tree / 529 on main);
   with FX3 it is 568 (+36).
3. **"the bone application point: our UnSkeletalComponent.cpp:4168 vs retail's UpdateSkelPoseEnd"** - the call was there but
   gated on `bDisableFaceFX` (a storage-less shim) and `SkeletalMesh->FaceFXAsset`, which is NULL on every NPC body: the
   asset is on the head mesh. Retail tests the component's `m_pFaceFXAsset`. No Edge-path deviation: the anim tree (with
   the Edge evaluator) writes LocalAtoms first, FaceFX adds to them, then ComposeSkeleton - retail's order.
4. **"the elite guard's 'SheWaiting' line (world 162+)"** - with `-fosoiree` it starts at 160.7 and repeats every 10 s
   (170.7); ID's 162.56 was a run without `-fosoiree`.
5. **IR's "13 heads / 19 face bones"**: the elite guard head's master bone list is the whole 118-bone skeleton (body, arms,
   legs included), so the FaceFX pass touches every body bone; additive mode makes the unlinked ones a no-op. In
   FXBM_Overwrite it would pin the body to FaceFX's reference pose - all 57 plays measured additive.
6. **"UFaceFXAsset::Serialize ... Arkane delta (0.62)"** - the reference body is retail's; the ratio is the reference's
   warnf lines (Shipping strips them). Same for the anim set (0.795).
7. The SDK API the reference integration calls is not all 1.7.3.1's: `FxActorInstance::SetRegister`, `GetBone`,
   `GetNumBones`, `FxActor::IsAnimGroupMounted` have no out-of-line copy - retail inlined them (named reconstructions).

## 5. Gate

`run_regression.py --build-dir ../agentFX3 --no-build --exe-name DishonoredGame_FX3.exe --log-prefix FX3g_ --only
d3d9,inputtest,newgame` (`build/agentFX3/gate.txt`): **22 ok, 1 failed**.
- d3d9: criticals 0, **unported_natives 0**, 1,110 frames, 6,506 draw elements.
- inputtest: criticals 0, **probe_natives 3 / 3**; `inputtest_moved` **365.0 / 217.5** (both runs) against >= 800 - the
  load-sensitive metric. Control on the base exe (FX1's gate exe, WITH_FACEFX=0) in the same hour:
  **286.4 / 313.4** (`gate_control_inputtest.txt`), machine at ~60 % CPU: load, not FaceFX.
- newgame: criticals 0, on mission 106, 29 pawns.
- Exit clean in every run (`Exit: Exiting.`; FaceFX shuts down after the object purge, so no instance outlives the SDK).
- CoreSmoke 568 / 0 / 1 (`build/agentFX3/coresmoke2.log`, with `FACEFX_CORE_SMOKE_DIR=build/agentFX1/fxa FACEFX_SMOKE_DIR=build/agentFX2`).

## 6. Merge note

`python build/agentFX3_sync.py` (base b9eb647; main moved to e932dc1 with IQ - no file of this package touched) copies
38 files and deletes the 4 shim files. Nothing to regenerate.
