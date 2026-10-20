# Agent IR — NPC mouths: the mouth interior and FaceFX lip-sync (wave 26, PHASE23 package IR)

Worktree `build/agentIR_wt` (made at `242fb05` = `b08193f` + docs; main moved to `2898de5`/`4ff8c11` during the package, no file of
this package touched by those commits), build dir `build/agentIR` (RelWithDebInfo). IDA on private copies
`build/agentIR/idb/{shipping2012,retail2013}_agentIR.i64`; decompiles in `build/agentIR/dec2012/`, `dec2013/`. Runs through
`build/agentIR/ir_run.py` (game_input.py launch, `-windowed -nomovie`, own inis). Package reads with a Python reader
(`build/agentIR/pkg/*.py` on agent IN's `pkgactors.py`). No retail exe, no real input, no FModel.

Builds: **two** (plus the cold warm-up of the untouched tree, `agentIR_build0_warm.log`), 0 errors, 0 C4263/C4264.
- build 1 (`DishonoredGame_IRb1.exe`): `-irdiag` / `-irface`.
- build 2 (`DishonoredGame_IR.exe`): + `USkeletalMeshComponent::SetFaceFXAsset`, its `PostBeginPlay_Body` call, the parent-LOD rule,
  LOD / required-bone columns in `-irdiag`. Its link failed twice with LNK1201 on the PDB: three of my own `DishonoredGame_IRb1`
  runs were still alive after `ir_run.py` had sent `quit` and held the build PDB open. I killed my processes and re-ran the same build,
  which then only linked.

## 1. Headline

**(A) I could not reproduce a black mouth interior in this build, so nothing in rendering needed fixing.** I took close-ups of
seven NPCs while they spoke: the captain, the elite guards `_26`/`_27`, two civilians, the Empress and the Lord Regent, at
22, 40, 150 and 300 uu (IR1–IR6). In every capture the mouth is **closed, the lips are textured, and nothing is black**
(`build/agentIR/shots/apshottime_ir100001.png` is the elite guard mid-'SheWaiting'; `ir5_grid.png` shows the Empress and the
Lord Regent through the Empress scene). The head dump shows every mouth/teeth slot resolving correctly:
- `Shr_Characters.TeethEyes.Mat_TeethClean` / `Mat_TeethCleanAO`: MIC → `Shr_TeethEyes`, usage ok, 19 GPU-skin shaders, 0 compile
  errors.
- `Teeth_d`: 512×1024, one mip, resident 1/1.
- `Teeth_n`: resident 11/11.
- Parameters: `DiffuseBrightness` .75, `Sk_Fade_To_Black` 1.0.
- No LOD-0 section is hidden.

The real difference from retail is that **the jaw never opens**. retail.mp4 against our ride at the same world time
(`build/agentIR/retail/cmp2.png`, the captain at w70–76): retail shows teeth and an open mouth mid-line, ours keeps the lips
shut. The face bones stay exactly at the reference pose for every speaker (`d=0.00 r=0.0` on all 19 jaw/lip/tongue/cheek bones
whenever the body is at LOD 0). Nothing in the animation data keys them; only FaceFX moves them. The "black mouth" the user saw
is a mouth that stays shut, with the dark lip crease under the cutscene's backlight, while retail opens it to show teeth.
**Cause named: FaceFX is compiled out (`WITH_FACEFX=0`).**

**(B) The lip-sync subset is about 640–1,130 FaceFX SDK functions, well above the ~400 limit, so this package delivers the sized
plan, not the port.** I counted it from the 2012 call graph (§3). I ported the one piece of engine-side integration that does not
need the SDK: `USkeletalMeshComponent::SetFaceFXAsset`, which retail calls in `PostBeginPlay_Body`. With `WITH_FACEFX=0` it runs
retail's own no-actor branch, so every NPC body now carries its head's FaceFX asset (IR7: `faceFX asset Skm_EliteGuard_Head_FaceFX`
on `DishonoredNPCPawn_27.body`). I also ported retail's `ForcedLodModel` test on the parent-LOD rule.

## 2. Measurements

Runs (`DishonoredGame/Logs/IR*.log`). All use `-nomovie -finewgame=50 -iddiag -irdiag`; IR4/IR5 add `-fwdiag -fwteleport=DisTrigger_16@160`.

| run | exe | face cam | what it measured |
|---|---|---|---|
| IR1 | b1 | 40 uu | 21 speaker events; head dumps of 6 meshes; elite guard mid-'SheWaiting' w163–165: mouth closed, textured |
| IR2 / IR3 | b1 | 150 / 22 uu | ride shots are the director camera; civilians and guards: mouth closed |
| IR4 / IR5 | b1 | none / 35 uu | Empress scene: Empress `_5` and Lord Regent `_7` lines go through `PlayDialogBlurb`; mouths closed in all 9 frames |
| IR6 | b1 | 300 uu | gameplay distance: closed dark lip line, no black hole |
| IR7 | b2 | 40 uu | body carries the head's FaceFX asset; LOD/required-bone columns; 0 Critical |
| CC IRb1 | b1 | – | ride SSIM .608 by the tool's slope fit (b = 1.061); .647 at the constant offset −42.98 (IK's rule from soirée ENTER 56.55) |

Head mesh facts from the dump and the cooked `LODInfo` (both agree):
- `Skm_CaptainGuard_Head` LOD0 = teeth (603 tris) + skin (2670).
- `Skm_EliteGuard_Head` LOD0 = skin (2810) + teeth (521). Its **LOD1 has one section only** (skin, 1398 tris, `LODMaterialMap [0]`)
  and so no teeth/eyes section; LOD2 has 6 sections with the gore pair hidden.
- Civilian heads: 2 sections in every LOD.
- Gore sections are hidden only on the last LOD. That is retail's own `DisHideGoreSections` (2013 0x7c7dd0 passes
  `LODModels.Num()-1`, USkeletalMesh+316) and is not a defect.

One finding that is not a defect: when the body is at LOD ≥ 1, its `RequiredBones` holds **0 of the 19 face bones**
(IR7: `DishonoredNPCPawn_3.body ... 0 required ... lod 1`). The face bones' local atoms are then never written: you see the
uninitialised values, `jaw_end_jnt d=25001, r=180`. As soon as the body is at LOD 0 they are back at the reference pose. Retail is
identical here:
- `RecalcRequiredBones` (2012 0x3536b0) has no parent/child merge.
- `UpdateTransform` (2013 0x357840) renders the head at the parent's LOD.

The stale values are only ever read by a head that is itself at LOD ≥ 1.

## 3. FaceFX lip-sync sizing (the plan)

**Method.** I exported a call graph of every function in the 2012 db: code xrefs, plus every function in a vtable-like table that a
function references (`build/agentIR/py/ir_callgraph.py` → `cg2012.tsv`). I then walked it from the engine entry points of the
lip-sync path (`build/agentIR/seeds_lipsync.txt`, script `py/ir_reach.py`):
- `UpdateFaceFX` 0x347b90 (from `UpdateSkelPoseEnd` 0x354c10)
- `PlayFaceFXAnim` 0x379870, `StopFaceFXAnim`, `IsPlayingFaceFXAnim`, `SetFaceFXAsset`
- `UFaceFXAsset` / `UFaceFXAnimSet::Serialize`, `PostLoad`, `FixupReferencedSoundCues`, `Mount` / `Unmount`, `FinishDestroy`
- `UnInitFaceFX` / `UnShutdownFaceFX`
- `PlayDialogBlurb` 0x8f7910, `GetFxAnimGroup`, `DisFaceFxStartAudio` 0x8f7e30

The walk descends only into module `facefx` / `fxsdk_unreal`.

| count | functions | bytes |
|---|---|---|
| direct calls only (lower bound; misses node `Compute` and archive virtuals) | 666 (637 in `Fx*.obj`) | 63 KB |
| with vtables (upper bound; all virtuals of every class constructed) | **1,165 (1,128 in `Fx*.obj`)** | 110 KB |
| whole SDK in the PDB (`Fx*.obj`) | 1,439 (= middleware.md's 1,317 + 122) | 146 KB |

Lists: `build/agentIR/facefx_lipsync_2012.tsv`, `facefx_lipsync_direct_2012.tsv` (rva, size, compiland, name).

**Why no smaller subset exists.** The cooked heads hold FaceFX archives (`*_Head_FaceFX`, 30–40 KB each; 13 in `L_Tower_Script`).
Loading one deserialises the whole `FxActor`: the face graph with every node class, links and link functions, the master bone list,
the phoneme map and the anim groups. Playback then needs the compiled face graph, the curves and the actor instance. The archive
format fixes the class set; lip-sync-only cannot skip the graph.

Proposed packages (upper bound / lower bound):

| package | compilands | fns | KB |
|---|---|---|---|
| FX1 core | FxMemory, FxName, FxRefString/RefObject, FxArchive + memory/null/file stores, FxClass, FxObject, FxNamedObject, FxSDK, FxUnrealSupport, FxBone/Key/Vec3/Quat | 384 / 271 | 31 / 24 |
| FX2 face graph | FxFaceGraph, FxFaceGraphNode(Link), FxFaceGraphShared, FxLinkFn, FxCompiledFaceGraph, the 6 node classes | 430 / 189 | 43 / 21 |
| FX3 runtime + engine | FxActor, FxAnimSet, FxAnim, FxAnimCurve, FxActorInstance, FxMasterBoneList, FxPhonemeMap; then `WITH_FACEFX=1` and the engine side below | 314 / 177 + ~25 engine | 34 / 16 |

37 header templates (`FxArray`, `FxList`, `FxStringBase`) were compiled into engine units and are counted here, not in the table.

Engine side for FX3. 2012 → 2013 rvas; all matched 1.000 except where noted:
- `UpdateFaceFX` 0x347b90 → 0x325ea0 (5,485 B; the reference body plus `DisFaceFxStartAudio` on `APS_StartAudio`)
- `PlayFaceFXAnim` 0x379870 → 0x356ef0, `GetFaceFXAnimStartOffset` 0x3799f0
- `UFaceFXAsset::Serialize` 0x56c360 → 0x52b280 (0.62, Arkane delta), `UFaceFXAnimSet::Serialize` 0x56be00 → 0x52b050
- `Mount` / `Unmount`, `UnInitFaceFX` 0x568670 → 0x527510, `UnShutdownFaceFX`
- `FFaceFXRegMap`, the morph and material-parameter proxies. Dishonored has no morph members, so they are probably unused.
- `DisFaceFxStartAudio` 0x8f7e30 → 0x8a0120
- `PlayDialogBlurb`'s FaceFX branch: `m_pFaceFxPendingBlurb` / `Text` / main flag, then `PlayFaceFXAnim`, with fall-through to the Wwise
  post
- `IsDonePlayingBlurb`'s `IsPlayingFaceFXAnim`, `StopDialogBlurb`'s `StopFaceFXAnim`
- `APawn::PlayActorFaceFXAnim` 0x17ee20
- `UInterpTrackFaceFX` (matinee pass 4)
- `FArkComponentLookAt::m_bFaceFxIsPlaying` (eye clamp and blink; FT ported it as unreachable)
- the other `SetFaceFXAsset` callers: `USkeletalMeshComponent::Attach` 0x376a70, `UStateNPCMasterDead::OnEnterState`,
  `ADishonoredNPCPawn::RestoreAppearance`

**Audio coordination (IH).** With FaceFX on, retail does not post the voice from `PlayDialogBlurb`. `PlayFaceFXAnim` succeeds, and
FaceFX's `APS_StartAudio` posts it from `UpdateFaceFX` through `DisFaceFxStartAudio`: `PostEvent` with flags 9, `DisConvAkEventCB`,
`OnSpeakerStartedTalking`. FX3 moves the voice post off the path IH4b owns today.

## 4. What was ported / changed (build 2)

| file | change |
|---|---|
| `Engine/Src/UnSkeletalComponent.cpp`, `Engine/Inc/UnSkeletalMesh.h` | NEW `USkeletalMeshComponent::SetFaceFXAsset` (2013 0x312b90 / 2012 0x335400, 316 B). The SDK half (bind/create/delete the `FxActorInstance` when the actor's master bone list fits the skeleton) is under `#if WITH_FACEFX`. `UpdateTransform`'s parent-LOD rule now also requires `ForcedLodModel == 0`, as retail 0x357840 does (+596 `ParentAnimComponent`, +672 `ForcedLodModel`, +680 `PredictedLODLevel`; checked against the retail SDK dump) |
| `DishonoredGame/Src/dishonorednpcpawn_body.cpp` | `PostBeginPlay_Body` calls `Mesh->SetFaceFXAsset( HeadMesh->m_pMesh->FaceFXAsset )` before the head is set, as 2013 0x77f8a0 does; the bringup note for that call is removed |
| `DishonoredGame/Src/disconversationcomponent.cpp` | one line at the FaceFX site of `PlayDialogBlurb`: `DisIRNoteSpeaker( LocInfo(i).m_pSkelMeshCompForFaceFX, ... )` (diag only; IH4b's lines untouched) |
| `DishonoredGame/Src/dishonoredplayercontroller.cpp` | `DisIRTick( this )` beside the other diag ticks |
| NEW `DishonoredGame/Inc/irdiag.h`, `Src/irdiag.cpp` | `-irdiag` and `-irface[=D]` |

Deviations (named):
- `SetFaceFXAsset` without its SDK half (`WITH_FACEFX=0`).
- `PostBeginPlay_Body` calls it only when `Mesh` is non-null. Retail does not test this.

## 5. Merge note

Main moved during the package: 2898de5 (IP), then 24c7074 / d947504 (IO, which changes `UnSkeletalComponent.cpp`
`ProcessRootMotion`). I fast-forwarded the worktree to `d947504` with a stash. `UnSkeletalComponent.cpp` auto-merged; IO's hunk
and mine do not touch the same lines. The merged tree was **not rebuilt** because both builds were spent. Build 2 is the
`242fb05`-based tree. `build/agentIR_sync.py` is based on `d947504` and reports 9 files clean.

## 6. Where the brief was wrong (measured)

1. **There is no black mouth-interior render defect.** No section is missing, every material and texture loads, and every parameter
   is set. Mouths are closed and textured in 40+ captures of 7 NPCs. Accept (A) as written ("teeth/tongue visible") needs the jaw
   to open, which only FaceFX does, so (A) and (B) are one problem.
2. **Lip-sync is not ≤ ~400 functions:** 637–1,128 SDK functions (63–110 KB). The data format forces the full graph.
3. **"The FaceFX anim on the head's skeletal mesh component" is wrong:** retail sets the head's asset on the **body** component
   (`SetFaceFXAsset(Mesh, Head->FaceFXAsset)`), and `m_pSkelMeshCompForFaceFX` is the body `Mesh`. The head reads the face bones
   through `ParentAnimComponent`.
4. **Morph targets play no part:** retail has no morph members (DP), and FaceFX drives bones.
5. **The `cutscene_compare` ride SSIM of ≥ .72 is not reachable on this main.**
   - Mine: .647 at the constant offset; the tool's slope fit gives .608.
   - Agent IO on the same main: .645 / .648 (`build/cmp/IO1c_const`, `IO2a_const`).
   - IP's camera-shake fix, now on main at 2898de5, scores .837 (`IP2_const`).
   - Nothing in this package runs in the ride's render path except the `ForcedLodModel` test, which is 0 for every component here.
6. **`ir_run.py` (copied from ID) can leave the game alive after `quit`:** three processes survived and locked the build PDB.

## 7. Gate result

`run_regression.py --build-dir build/agentIR --no-build --exe-name DishonoredGame_IR.exe --only d3d9,inputtest,newgame`
(`build/agentIR/gate.txt`, re-runs `gate2.txt`, `gate3.txt`). The machine was at 87 % CPU, with other agents' links and games running.

**d3d9: all ok**
- `unported_natives` **0**
- `d3d9_criticals` 0
- 6,506 draw elements

**inputtest**
- `inputtest_criticals` 0
- `probe_natives` **3** (2 on one attempt)
- `inputtest_moved` 669–687 (402.7 on one attempt) against a bound of ≥ 800: load. HL recorded 402.7 → 1050 for the same stage
  under load.

**newgame: fails 3 of 3 attempts, and it is not this package**

The failure is always the same: `newgame_criticals` 3, from `appError: Streamed out levels were not completely garbage collected`.
The main-menu world (`DisGFxMoviePlayerMainMenu_0`, `ShadowMap2D`s, `Polys`) is held from the root by
`OnlineSubsystemSteamworks_0.DeviceIdCache.DeviceSelectionDelegates`. The two controls fail identically:
- **build 1** (base + the diag only, no `SetFaceFXAsset`): `gate_control_b1.txt`, `IRc__newgame.log`
- **head_rel = main's own exe** (`DishonoredGame-Win64-Shipping.exe`): `gate_control_headrel.txt`, `IRhr__newgame.log`

So this is pre-existing or environmental (a device-selection delegate the front end registers today and never clears; IO's and IP's
newgame gates passed earlier the same night). It is not caused by IR. It needs its own look: who adds to `DeviceSelectionDelegates`
from the main menu, possibly HK's `SelectController` path when a controller is present.
