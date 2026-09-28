# Agent DP report — the three mission maps run, and the shim audit (2026-09-28)

Package **DP** of wave 9, written against **HEAD `dd760e4`**. It takes agent DO's hand-over 3: `L_Tower_P` was
the only mission map this tree survived. It now runs three. Status rows: `agentDP_status.csv`. Build dir
`build/agentDP`, snapshot worktree `build/agentDP_wt`, decompiles in `build/agentDP_decomp/{r13,s12}`, IDA copies
`resources/docs/idb/{retail2013,shipping2012}_agentDP.i64`. No commits, nothing staged.

HEAD compiles: `dd760e4` is the coordinator's fix of the private `FGFxEngine::InputKey` overload that agent DO
had to patch locally, so this package needed no repair of anyone else's file.

## Commands

```
python resources/tools/make_snapshot.py DP --sync                    # refresh the snapshot overlay
cmd /c build\agentDP_release.cmd                                     # Release into build/agentDP (+ CoreSmoke, LayoutProbe)
cmd /c build\agentDP_clean.cmd                                       # a clean full Release build into build/agentDP_clean
python resources/tools/stage_retail.py --build-dir build/agentDP --exe-name DishonoredGame_DP.exe
python build/agentDP_work/run.py fix_boyle --map=L_Boyle_Ext_P -apshottime=45 --timeout=95
python build/agentDP_work/shim_audit.py                              # the audit, into build/agentDP_work/
python resources/tools/run_regression.py --build-dir build/agentDP --no-build
```

## Result

| Accept | State |
|---|---|
| 1. all three mission maps load and run | **done**: `L_Tower_P` 108.6 s, `L_Boyle_Ext_P` 104.2 s, `L_Streets1_P` 95.2 s, **0 `Critical:` lines each**, one screenshot each (section 5). Boyle died at 6.38 s and Streets1 at 6.19 s on the same binary before this package |
| 2. the layout probe still clean | `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`, `layout_compare_contract 0`, `verify_phase2 2/2` |
| 3. the shim audit, its script and the ranked list | `build/agentDP_work/shim_audit.py` -> `shim_audit.csv` + `shim_audit.txt`; **1,098 declarations, 68 rank A, 303 rank B, 350 rank C** and the finding that changes what to do about them (section 4) |
| 4. `run_regression.py` green + a clean full build | **31 ok, 0 failed, 0 skipped, 423 s** on this package's own `--build-dir`, plus a clean full Release build of the snapshot worktree (section 6) |

## 1. The fault, and it is not the one the brief expected

The brief's diagnosis was that `BoneVisibilityStates` is a `DISHONORED_SHIM_STATIC` — one array shared by every
`USkeletalMeshComponent` in the process — and that the fix is to give it real per-instance storage at retail's own
offset and port retail's initialisation. The sharing is real and it is half the mechanism. The other half, and the
remedy, are different, and the binaries say so:

**retail 2013 has no `BoneVisibilityStates` and no bone visibility feature at all.**

| evidence | what it says |
|---|---|
| `USkeletalMeshComponent::HideBone`, 2013 rva **`0x3bcdc0`** (2012 `0x3e2030`, ratio 0.984), 179 bytes | the whole body is `if (BoneIndex != INDEX_NONE) if (PhysBodyOption != PBO_None && PhysicsAssetInstance) { Name = SkeletalMesh->RefSkeleton(BoneIndex).Name; PBO_Term -> TermBodiesBelow, PBO_Disable -> EnableCollisionBodiesBelow(FALSE) }`. No visibility write, no `RebuildVisibilityArray`, no `bRequiredBonesUpToDate`, and **not even the reference's `LocalAtoms(BoneIndex).SetScale(0)`** |
| `USkeletalMeshComponent::UnHideBone`, 2013 rva **`0x3b4250`** (2012 `0x3d59b0`), 122 bytes | one `EnableCollisionBodiesBelow(TRUE, Name, this)` and nothing else |
| 2012 `USkeletalMeshComponent::RecalcRequiredBones`, rva **`0x3536b0`**, 1,485 bytes | the LOD copy, the mirror table, the physics-asset bodies, the per-poly bones, `EnsureParentsPresent`, `bUpdateComposeSkeletonPasses`. **No purge of invisible bones** |
| 2012 `USkeletalMeshComponent::UpdateSkelPoseBegin`, rva **`0x3737a0`** | allocates `SpaceBases` and `LocalAtoms` against `RefSkeleton.Num()` and **no third array**, then `if (!SpaceBases.Num()) return` |
| 2012 `USkeletalMeshComponent::UpdateRBJointMotors`, rva **`0x3cd660`** | the joint gate is `BoneIndex != INDEX_NONE && BoneIndex != 0 && (bSwingPositionDrive \|\| bTwistPositionDrive)`. No visibility term |
| the 2012 PDB (`types/types.json`, `symbols/functions.csv`) | `USkeletalMeshComponent` is 1,056 bytes with **no `BoneVisibilityStates` member**, and there is no `RebuildVisibilityArray` and no `IsBoneHidden` symbol anywhere in the executable |
| the retail SDK dump and `script_classes_2013.json` | retail's `SkeletalMeshComponent` spans 468..1088 with 183 reflected members and **`BoneVisibilityStates` is not one of them**; its script functions do include `HideBone`/`UnHideBone`/`HideBoneByName`/`UnHideBoneByName`, which is what a first look mistakes for the feature being there |

The last row is worth dwelling on, because it is how a careful reader gets this wrong: retail **does** ship
`HideBone`. It just does not mean what the reference means by it. In Dishonored's engine branch hiding a bone is
a physics operation — terminate or disable the bodies below it — and there is no per-bone graphical state, so
there is nothing to store, nothing to propagate to children and nothing to purge from `RequiredBones`.

One more thing pins this down rather than leaving it to inference: **`check()` is live in both shipped builds.**
Every `TArray::operator()` bounds check (`"i>=0 && (i<ArrayNum||(i==0 && ArrayNum==0))"`, `Core/Inc/Array.h:596`)
appears in both decompiles. So a `check(BoneVisibilityStates.Num() == ...)` or a `LocalAtoms(BoneIndex)` access
would be visible in retail's `HideBone` if it were there. The only bounds check that function compiles is the
`RefSkeleton(BoneIndex)` lookup for the bone's `FName`.

`VERIFY_CLASS_OFFSET_NODIE(USkeletalMeshComponent, SkeletalMeshComponent, BoneVisibilityStates)` at
`EngineSkeletalMeshClasses.h:376` is the reference script compiler's output, not evidence of a retail offset: the
loop it expands to iterates retail's own `UProperty` list, which has no such property, so it has never compared
anything. That line is why the layout probe was clean through all of this.

### 1.1 Measured first, on the unmodified snapshot, in two passes

Reproducing DO's crash on the baseline (`build/agentDP/base_boyle.log`, `base_streets1.log`): `L_Boyle_Ext_P`
dies at **6.38 s**, `L_Streets1_P` at **6.19 s**, both on
`Assertion failed: BoneVisibilityStates.Num() == SkeletalMesh->RefSkeleton.Num()`,
`Engine/Src/UnSkeletalComponent.cpp:3357`.

Pass one replaced the three `check`s with a census and guarded the reads so the run continues
(`build/agentDP_work/diag_bones.py`, `build/agentDP/diag_boyle.log`):

```
dpbones: L_Boyle_Ext_P...DishonoredNPCPawn_1.DisSkeletalMeshComponent_2 mesh Skm_EliteGuard_Head02
  refbones 118 visstates 2 array 0x01A1C684 parentanim DisSkeletalMeshComponent_3 owner DishonoredNPCPawn_1
dpbones: L_Boyle_Ext_P...DishonoredNPCPawn_4.DisSkeletalMeshComponent_8 mesh Skm_EliteGuard_Head
  refbones 118 visstates 2 array 0x01A1C684 parentanim DisSkeletalMeshComponent_9 owner DishonoredNPCPawn_4
```

Two different components, two different meshes, **one array address**. And the component that trips it is an
NPC's *head* mesh — agent DI's pass — which has a `ParentAnimComponent`.

Pass two named the other end (`diag_resize.py`, `build/agentDP/diag2_boyle.log`). The sizing block in
`UpdateSkelPose` is the only code in the tree that grows the array, and in one second of `L_Boyle_Ext_P` it is
reached by, in order:

```
dpbones resize: DishonoredUsableObject_5.SkeletalMeshComponent0 mesh door_medium_rich_ext_metal_sk  to 2   array 0x0170D684
dpbones resize: DishonoredUsableObject_6.SkeletalMeshComponent0 mesh DBdoor_big_rich_ext_metal_sk   to 3   array 0x0170D684
dpbones resize: DisFish_21.pSkeletalComp                        mesh fish                          to 7   array 0x0170D684
dpbones resize: DishonoredPlayerPawn...                         mesh Skm_Player                    to 79  array 0x0170D684
dpbones resize: DishonoredNPCPawn_0.DisSkeletalMeshComponent_1  mesh Skm_EliteGuard_Body           to 118 array 0x0170D684
```

Three doors, a fish, the player and a guard, resizing **the same store** dozens of times a second. A door is the
last writer before the head component's `RecalcRequiredBones` runs, so the head reads 2 against its own 118 and
the process aborts.

That is the whole mechanism, and it explains DO's observation exactly: **`L_Tower_P` survives by coincidence of
tick order**, not by being different in kind. Put a two-bone door in front of a 118-bone head in the update
order of any map and it dies.

### 1.2 Why real storage would not have fixed it

The reference initialisation is inside `if (ParentAnimComponent == NULL)` in `UpdateSkelPose`. The component that
asserts is a head mesh, whose `ParentAnimComponent` is the body. So with per-instance storage that component's
array would have been **length 0 against 118** and the same `check` would have fired on the same line — sooner,
in fact, and on `L_Tower_P` too. The shim hid a second defect behind the first: the reference's own initialisation
never covers a parent-animated component, and this tree grew parent-animated components in wave 9 when agent DI
gave the NPCs heads (`9dc346b`). That is why a map that ran yesterday stopped running today.

### 1.3 What this package did

Removed the feature, which is what makes the tree match the binary:

| site | change |
|---|---|
| `UnSkeletalMesh.h` | the `DISHONORED_SHIM_STATIC TArrayNoInit<BYTE> BoneVisibilityStates` declaration, the `EBoneVisibilityStatus` enum and the `RebuildVisibilityArray` declaration |
| `UnSkeletalComponent.cpp` `UpdateTransform` | the block that copied the parent transform and zeroed the scale of invisible bones before `MeshObject->Update` |
| `UnSkeletalComponent.cpp` `ComposeSkeleton` | the third `check` |
| `UnSkeletalComponent.cpp` | `RebuildVisibilityArray` itself, replaced by a note with the evidence |
| `UnSkeletalComponent.cpp` `RecalcRequiredBones` | the purge block, which is the assert of the crash |
| `UnSkeletalComponent.cpp` `UpdateSkelPose` | the sizing block |
| `UnPhysAnim.cpp` `UpdateRBJointMotors` | the visibility term of the joint gate |
| `UnPhysComponent.cpp` | `HideBone`, `UnHideBone` and `IsBoneHidden` put back to the shipped bodies |

`IsBoneHidden` is kept, returning FALSE, because two callers in `ParticleModules_Location.cpp` need it to
compile and retail's condition there is the rest of the expression — deleting the term would be the same change
in someone else's file. It carries the evidence at the site.

A `-dpbones` census counts the three entry points and says so on the first call of each. **On all three mission
maps it prints nothing**: across 308 s of play, nothing in this content calls `HideBone`, `UnHideBone` or
`IsBoneHidden`, so the behaviour removed here was not merely wrong, it was unreached — the assert was the only
thing any of it ever did.

## 2. A note on line endings, because it cost a build

`source/Development/Src/Engine/Inc/UnSkeletalMesh.h` and the three `.cpp` are **CRLF in the working tree and LF
in a `git worktree` checkout of the same commit**. A patch script whose string literals came from a CRLF file
matched nothing in the worktree and everything in the working tree, or the reverse, with no error other than
"0 hits". `build/agentDP_work/fix_bonevis.py` detects each target's own line ending and converts its needles to
it; anyone writing a patch script here should do the same rather than assume either.

## 3. The audit: `build/agentDP_work/shim_audit.py`

The script finds every `DISHONORED_SHIM_STATIC` declaration with its owning class, finds every use of the name,
and ranks them. Three filters make the counts mean something:

* **only compiled code** — the 899 source files the ninja graph of this build actually compiles, out of a
  reference tree full of editor-only units;
* **only live preprocessor regions** — the `-D` table is read straight out of `build.ninja` (63 macros), so
  `WITH_APEX`, `WITH_FACEFX`, `WITH_EDITOR`, `WITH_GFx`, `WITH_SUBSTANCE_AIR` blocks do not inflate a row. A
  condition the evaluator does not understand is treated as live, so the filter only ever removes what it is
  sure about;
* **ambiguity is declared, not hidden** — a name that is also real storage on some other class (`Materials`,
  `PathList`, `Score`) cannot be counted by identifier, so those rows get a `?` suffix and are ranked separately
  instead of taking the top of the list, which is exactly what they did in the first version of this script.

`VERIFY_CLASS_OFFSET`/`VERIFY_CLASS_SIZE` lines are counted in their own column and excluded from the use counts:
on a shim, `STRUCT_OFFSET` takes the address of a process-wide static, so such a line can never match a script
offset and is not a runtime read.

Ranks: **A** a container shim that is indexed, size-compared or checked (the `BoneVisibilityStates` shape — it
aborts the process); **B** written at runtime, so one store serves the whole process; **C** read and never
written, so permanently zero (the `MaxFilterBlurSampleCount` shape — a feature that silently does nothing);
**D** unused in compiled code today.

```
1,098 declarations in 33 headers
A-fatal 68   A-fatal? 12   B-shared-write 303   B-shared-write? 93   C-dead-read 350   C-dead-read? 51   D-unused 199
UnSkeletalMesh.h 29/44/109/5 · EngineGameEngineClasses.h 7/22/31/28 · EngineClasses.h 5/18/25/44
EngineControllerClasses.h 5/13/12/9 · GameFrameworkClasses.h 4/18/42/36 · EngineMaterialClasses.h 0/76/17/30
```

### 3.1 The finding that decides what to do about all of them

The script cross-references every shim against the CodeRed dump of the running retail executable
(`types/retail_sdk_layout.json`), which is the `in_retail` column. Of **1,098 shims, five** name a member that
retail actually has. 1,053 do not; 40 are unknown because the owning class is not in the dump.

**So these are overwhelmingly not members that lost their storage. They are features Dishonored's engine branch
does not have**, whose reference code this tree still compiles: the legacy path network (`ANavigationPoint::PathList`,
`AController::MoveTarget`/`CurrentPath`/`NextRoutePath`/`MoveTimer` — retail's `AController` has `RouteCache`,
`CurrentPathDir` and `FailedMoveTarget` and none of those four), morph targets (retail's `USkeletalMeshComponent`
has no morph member at all), cloth and soft body (retail's `USkeletalMesh` has **zero** `Cloth*`/`SoftBody*`
members), secondary viewports, touch input.

That changes the remedy, and it is why this package deleted rather than allocated. **Giving a rank-A shim real
storage makes the tree do something retail does not do**; removing its code path makes the tree match the binary
and costs no bytes and no layout risk. The five exceptions are the opposite case and are the ones that want a
layout change.

### 3.2 The five shims retail really has — most likely to bite next, in order

| shim | why |
|---|---|
| **`UPrimitiveComponent::ReplacementPrimitive`** (`PrimitiveComponent.h:918`, 11 writes / 6 reads) | retail has it as real storage; here **every primitive component in the process shares one pointer**, so the last component to set a replacement sets it for all of them. It is on the MassiveLOD / replacement path, which is live drawing code, and it is a wrong-picture defect rather than a crash — the kind this project has repeatedly found late |
| **`UMaterial::bAllowFog`** (`EngineMaterialClasses.h:3975`, 7 reads / 5 writes) | one fog flag for every material in the process. Wave 6 already spent a package on a fog figure that was wrong for a different reason |
| **`UMaterial::bUsedWithFogVolumes`** (`:3977`, 6 reads / 2 writes) and **`bUsedWithFracturedMeshes`** (`:3980`) | the same shape, on the material usage flags that gate shader permutations |
| `AWorldInfo::ProcBuildingRulesetOverride` (`EngineGameEngineClasses.h:698`, 1 read) | harmless in practice: there is one `AWorldInfo`, so a shared store and a member are the same thing |

All four of the first ones are bit or pointer members with a retail offset and mask in the SDK dump, so each is a
layout edit of the kind agents R and S did by the dozen — but they are in `PrimitiveComponent.h` and
`EngineMaterialClasses.h`, not in this package's files, and none of them aborts the process, so they are named
here rather than changed under two other live agents.

### 3.3 The rank-A reference-only shims, ranked by how close they are to firing

| # | shim(s) | reachable today? |
|---|---|---|
| 1 | **`ANavigationPoint::PathList`** — 97 reads, 10 writes, **55 indexed**, 41 `Num()` comparisons, across `UnNavigationPoint.cpp`, `UnRoute.cpp`, `UnPath.cpp`, `UnVehicle.cpp` | the single largest one in the tree. Retail has no `PathList`: Dishonored navigates on the nav mesh. Agent DN is in the nav-mesh runtime now, and the moment anything walks the reference path network this is `BoneVisibilityStates` again, one array of reach specs for every navigation point in the world |
| 2 | **`AController::MoveTarget` / `CurrentPath` / `NextRoutePath` / `MoveTimer`** (rank B, 84/68/36/10 reads) | **26 NPCs now have controllers** (agent CG) and locomotion is the next package. These four are the legacy move-target state and they are one store for every controller in the process, so the last AI to pick a target picks it for all of them. Not a crash — a behaviour that will look like an AI bug and be hunted as one |
| 3 | the **cloth and soft-body family**, ~20 rank-A rows on `USkeletalMesh` and `USkeletalMeshComponent` (`ClothWeldingMap` 47 reads / 9 indexed / 40 `Num()` comparisons, `ClothToGraphicsVertMap`, `ClothMeshPosData`, `ClothMesh`, `GraphicsIndexIsCloth`, `SoftBodyTetra*` …) | latent behind `bEnableClothSimulation`, which is itself a storage-less shim that reads 0, so cloth never initialises. `WITH_NOVODEX` is 1, so the code is compiled. Retail has none of these members: the right move is to delete the reference cloth path from the skeletal side, not to allocate twenty shared arrays |
| 4 | **`USkeletalMeshComponent::ActiveMorphs` / `ActiveCurveMorphs` / `MorphSets` / `MorphTargetIndexMap`** | `ActiveMorphs` is handed to `MeshObject->Update(UseLOD, this, ActiveMorphs)` every frame for every skeletal mesh, and it is one array for all of them. It is empty today, which is why nothing shows; retail has no morph members at all (Dishonored uses FaceFX and the Edge path), so this is reference code to remove |
| 5 | `UGameEngine::SecondaryViewportFrames` / `SecondaryViewportClients` (3 `check`s each), `UAnimTree::SavedPose` (9 writes) and `RootMorphNodes`, `ACamera::ActiveAnims` / `FreeAnims`, `ULevel::CoverIndexPairs` / `CrossLevelCoverGuidRefs`, `UInput::CurrentTouches` / `Cached*InputEvents` | each is a shared container that is indexed; none is on a path this content reaches today. `UAnimTree::SavedPose` is the one to watch, because anim trees are per-pawn and the save/restore pose path is animation work |

The full ranking, with reads, writes, `check` counts, index counts, `Num()` comparisons, the `in_retail` column
and a first use site per row, is `build/agentDP_work/shim_audit.csv` (1,098 rows) and `shim_audit.txt`.

### 3.4 What the audit cannot tell you, said plainly

The ranking is static. "Reachable today" in the table above is hand-read from the use sites and from what this
tree runs, not measured per shim — measuring 1,098 of them would need a build per shim. The one runtime fact the
script does use is which files the build compiles and which preprocessor regions survive, and that is worth
having: it is what moved `UApexGenericAsset::Materials` off the top of the first version of this list.

## 4. The measurements

Every run is `build/agentDP_work/run.py`: the map as the command line's **first token** (agent DE's route),
`-benchmark -fps=30`, `-windowed -ResX=1280 -ResY=720 -nomovie`, `-nogfxui`, `-apshottime`, never `-apshot`.

| run | before this package | after |
|---|---|---|
| `L_Tower_P` | ran | **108.6 s, 0 `Critical:`** |
| `L_Boyle_Ext_P` | **died at 6.38 s** | **104.2 s, 0 `Critical:`** |
| `L_Streets1_P` | **died at 6.19 s** | **95.2 s, 0 `Critical:`** |

Agent DO's hand-over 5 was right about the shared `Screenshots/Win32Console`: agent DN's game was running
throughout and its `-apshottime` frames land in the same directory, and the first Tower run of this package
picked up the wrong one of two files 1.8 s apart. `run.py` now takes the log timestamp of **its own**
`-apshottime` line and picks the new file whose mtime matches it, printing the gap: `+0.04 s`, `+0.19 s`,
`+2.49 s` for the three accepted frames, and it warns rather than copying if nothing lands within three seconds.
Anyone taking frames on this machine should do the same.

`unported_natives 0` and the walking numbers are unchanged (`inputtest_moved 1020.3` against 1029.6 at HEAD,
`peak_speed 500.4`, `physics_actors 1369`, `physics_static_shapes 1312`, `touch_census 308`,
`sequence_census 87,655`, `d3d9_draw_elements 6506`), so nothing removed here was on the walking, touch, sequence
or physics path.

## 5. The screenshots

| file | what |
|---|---|
| `build/agentDP/fix_boyle.png` | **the accept shot**: `L_Boyle_Ext_P`, the Boyle mansion garden at night with its three lanterns, hedges and lamp rows — a map this tree could not reach past six seconds |
| `build/agentDP/fix_streets1.png` | `L_Streets1_P` from the bow of the boat: the river, the bridge, the whale carcass and the moon |
| `build/agentDP/fix_tower.png` | `L_Tower_P`, the deck of the boat, unchanged from before this package |
| `build/agentDP/*.bmp` | the frames as captured |

## 6. The harness and the builds

`python resources/tools/run_regression.py --build-dir build/agentDP --no-build`: **31 ok, 0 failed, 0 skipped,
423 s** (`build/agentDP/regression/summary.txt`). `layout_types 2314`, `layout_mismatching 0`,
`layout_contract 0`, `layout_probed 2341`, `layout_compare_contract 0`, `verify_phase2 2/2`,
`coresmoke_passed 99`, `nullrhi_criticals 0`, `d3d9_frames 19380`, `d3d9_criticals 0`,
`d3d9_draw_elements 6506`, `texture_census 16257`, `unported_natives 0`, `touch_census 308`,
`sequence_census 87655`, `inputtest_moved 1020.3`, `inputtest_peak_speed 500.4`, `physics_actors 1369`,
`physics_static_shapes 1312`, `probe_natives 6`, `inputtest_criticals 0`.

| build | result |
|---|---|
| `build/agentDP_release.cmd` (DishonoredGame, CoreSmoke, LayoutProbe into `build/agentDP`) | 0 errors (`build/agentDP_build3.log`, `build4.log`) |
| `build/agentDP_clean.cmd` (a CLEAN full Release build of the same worktree into `build/agentDP_clean`) | **960 steps, 0 errors**, all three binaries (`build/agentDP_clean.log`); its own `layout_probe.txt` reads `types=2314 exact=1681 mismatching=0 contract_mismatches=0` |

## 7. Files

Mine (4, `build/agentDP/snapshot_files.txt`), all in `Engine`, none generated:

| File | Change |
|---|---|
| `Engine/Inc/UnSkeletalMesh.h` | the `BoneVisibilityStates` shim, the `EBoneVisibilityStatus` enum and the `RebuildVisibilityArray` declaration removed; a note on `IsBoneHidden` |
| `Engine/Src/UnSkeletalComponent.cpp` | `RebuildVisibilityArray` removed; the invisible-bone block of `UpdateTransform`, the purge block of `RecalcRequiredBones`, the sizing block of `UpdateSkelPose` and the `ComposeSkeleton` check removed, each replaced by the retail evidence |
| `Engine/Src/UnPhysAnim.cpp` | the visibility term of `UpdateRBJointMotors`'s joint gate |
| `Engine/Src/UnPhysComponent.cpp` | `HideBone`, `UnHideBone`, `IsBoneHidden` put back to the shipped bodies, plus the `-dpbones` census |

**No generated file is touched**, so nothing needs regenerating at merge.

Helpers in `build/agentDP_work/` (not repo tools): `shim_audit.py` (the audit, with `shim_audit.csv` and
`shim_audit.txt`), `fix_bonevis.py` and `fix_bonevis2.py` (the two halves of the change, idempotent, line-ending
aware), `diag_bones.py` and `diag_resize.py` (the two diagnostic builds), `run.py`, `shim_miss.py`,
`dbg_edits.py`, `declist_r13.txt`, `declist_s12.txt`.

## 8. Bring-up switches

| Switch | What it does |
|---|---|
| `-dpbones` | prints `HideBone` / `UnHideBone` / `IsBoneHidden` call counts on every call instead of only the first. Read on first use, never in a file-scope static (agent CA's finding) |

## 9. Hand-overs

1. **`ANavigationPoint::PathList` is the next `BoneVisibilityStates`, and it is the biggest one in the tree.**
   97 reads, 10 writes, 55 indexed accesses and 41 `Num()` comparisons of one array shared by every navigation
   point in the process, and retail has no such member — Dishonored navigates on the nav mesh. It has not fired
   because nothing walks the reference path network yet. Whoever owns navigation should delete that path rather
   than allocate it; `build/agentDP_work/shim_audit.csv` has every site.
2. **`AController::MoveTarget`, `CurrentPath`, `NextRoutePath` and `MoveTimer` are one store for all 26 NPCs.**
   Retail's `AController` has `RouteCache`, `CurrentPathDir` and `FailedMoveTarget` and none of these four. This
   will not crash; it will look like an AI bug when locomotion lands, and it is the cheapest thing to rule out
   first when it does.
3. **Five shims are the other kind and want a layout edit, not a deletion**:
   `UPrimitiveComponent::ReplacementPrimitive`, `UMaterial::bAllowFog`, `bUsedWithFogVolumes`,
   `bUsedWithFracturedMeshes`, `AWorldInfo::ProcBuildingRulesetOverride`. Retail has all five as real storage with
   an offset and mask in the SDK dump. `ReplacementPrimitive` is the one that draws.
4. **The reference cloth, soft-body and morph-target code on the skeletal side is dead and should go.** Retail's
   `USkeletalMesh` has zero `Cloth*`/`SoftBody*` members and retail's `USkeletalMeshComponent` has no morph
   member at all, yet `WITH_NOVODEX` compiles the cloth path and `ActiveMorphs` is handed to the render thread
   every frame for every mesh. That is ~25 rank-A and rank-B shims removed by deleting code, in the same files
   this package touched, and it would make the next reader of `UnPhysComponent.cpp` much less likely to port
   something Dishonored never had.
5. **Agent DO's blocked accept is unblocked.** `L_Boyle_Ext_P` carries the two tallboy tweak objects DO found
   (`Pwn_Tallboy_Wood_SearchLight`, `Pwn_Tallboy_Wood`, both naming `Skm_TallBoyStilts`) and now runs for a
   hundred seconds, so DO's stilts and searchlight passes, agent DI's accessories and material variations and
   every other piece of content the first mission does not carry can be exercised at last.
