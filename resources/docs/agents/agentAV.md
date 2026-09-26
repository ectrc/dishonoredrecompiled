# Agent AV report — Arkane animation nodes: the `-distweakanimtree` gate is gone, and characters animate (2026-09-27)

Package AV of `resources/docs/PHASE7.md`, started on HEAD `bb005ec`. IDA copies `retail2013_agentAV.i64` /
`shipping2012_agentAV.i64`; decompiles in `resources/reference/decomp/agentAV/s12` (213 functions, headless).
Status rows: `resources/docs/agents/agentAV_status.csv` (61 rows). Build `build/agentAV` = the HEAD snapshot
`build/agentAV_wt` + my 20 files, Release (`build/agentAV_snap_build.cmd`), because AU's in-flight
`dishonoredplayercontroller.cpp` / `dispickup_base.cpp` edits do not compile in the shared tree.

## Merge note, read first

HEAD moved to `2132d72` while this package ran, and two of my files were swept into other agents' commits:
`Engine/Src/UnPawn.cpp` by the coordinator's `2132d72` and `DishonoredGame/Inc/DishonoredGameNative.h` by AU's
`c8105a5`. `UnPawn.cpp` at HEAD therefore calls `DisAnimTickArray(Mesh)`, which is declared in `UnAnimTree.h` and
defined in `UnAnimTree.cpp` — both still uncommitted here. **HEAD does not link until this package is merged** (or
until those two files are reverted). Everything else of mine is still a working-tree change. The snapshot
`build/agentAV_wt` is detached at `bb005ec`, so rebase it before re-measuring:
`git -C build/agentAV_wt checkout --force --detach <HEAD>` then
`python resources/tools/make_snapshot.py AV --sync`.

I did rebase the snapshot onto `2132d72` to re-measure there, and `2132d72` does not compile for a second,
independent reason: AU's `disinteractableinterface.cpp`, `dishonoredkactor.cpp`, `dispickup_base.cpp`,
`disstatpickup.cpp` and `disabstractitempickup.cpp` are committed but the generated module is not regenerated for
them — their five `#include "CppText/<Class>.h"` lines are missing from `DishonoredGameItemClasses.h` /
`dishonoredgameclasses.h`, the units are still on `DishonoredGame_EXCLUDE` in `Sources.cmake`, and
`DishonoredGameNativeStubs.cpp` still defines the three natives they now implement. Adding the five includes by hand
(`build/agentAV_work/patch_snapshot_au.py`, snapshot only) clears the compile errors and leaves the link errors, which
is the regeneration step. All AV numbers in this report are therefore measured on `bb005ec` + the 20 AV files, and the
snapshot has been put back there.

## Result

| Accept criterion | State |
|---|---|
| the game runs with the tweak animation tree applied and **no gate** | **met** — `ADishonoredPawn::ApplyTweakChanges_Derived` assigns `Tweaks->m_pAnimTreeTemplate` unconditionally; `-distweakanimtree` no longer exists |
| zero critical errors over 60 s | **met** — the package's accept command, 90 s, `--forbid Critical`, exit 0, `Initial startup` reached |
| evidence that bones actually move | **met** — pose census: the player pawn's 79-bone tree moves **48 of 79 bones** (max), 1,195 of 1,200 consecutive evaluations with motion |
| which nodes are ported, which still warn | below |

The gate was not the whole story. **The reason nothing in the game animated is a layout shim, not a missing node.**

## The real defect: four anim arrays that every skeletal mesh component shared

`retail_sdk_layout.json` gives retail's `USkeletalMeshComponent` only `SkelControlTickArray` (@484). Arkane moved
`AnimTickArray`, `AnimAlwaysTickArray`, `AnimTickRelevancyArray` and `AnimTickWeightsArray` onto **`UAnimTree`**
(@424 / @436 / @448 / @460) — which is exactly why retail has `UAnimTree::InitAnimTree(USkeletalMeshComponent*)`
(2012 rva 0x1ae3c0) and `UAnimTree::TickTree(FLOAT, FLOAT)` (2013 rva 0x19c0d0) where the reference engine has
`USkeletalMeshComponent::InitAnimTree` / `TickAnimNodes`.

Agents S/M kept the four reference members as `DISHONORED_SHIM_STATIC` in `UnSkeletalMesh.h` — storage-less C++17
inline statics, i.e. **one** node list, **one** relevancy array and **one** weight array shared by every skeletal mesh
component in the process. Consequences, all of which were visible in the run:

- no component owned its node list, so `UAnimNodeBlendBase::UpdateChildWeight` wrote child weights into whatever
  component had initialised last: **weights never propagated down any tree**;
- relevancy was global, so `OnBecomeRelevant` / `OnCeaseRelevant` fired for the wrong nodes;
- at teardown `USkeletalMeshComponent::DeleteAnimTree` iterated another component's freed nodes — the access violation
  in `DeleteAnimTree+0xc4` that appeared the moment the pawn finally had a tree.

Ported against retail (all four functions above), so the arrays are per tree:

| Function | rva | Change |
|---|---|---|
| `UAnimTree::InitAnimTree(USkeletalMeshComponent*)` | 2012 0x1ae3c0 | new; the reference's node-list build, on the tree's own arrays. `InitAnimNodeListFastSearch` is not ported (Arkane's `FindAnimNodeFast` table does not exist here) |
| `UAnimTree::TickTree(FLOAT, FLOAT)` | 2013 0x19c0d0 | new; the reference's per-node tick loop on the tree's own arrays, root weight passed in, ending in `UpdateAnimNodeSeqGroups`. `bPauseAnims` is not consulted, as in retail |
| `USkeletalMeshComponent::InitAnimTree` | 2012 0x364e20 | `++InitTag` then `Tree->InitAnimTree(this)`, plus retail's single-node branch |
| `USkeletalMeshComponent::TickAnimNodes` | 2012 0x334d30 | `++TickTag` then `Tree->TickTree(DeltaTime, 1.f)`, plus retail's single-node branch |
| `UAnimNode::GetNodes` | 2012 0x1a6b00 | retail always traverses; the node's **own** tree's tick array is only a reserve hint (the reference shortcut read the shared shim array, which is not this node's tree) |
| `UAnimNodeBlendBase::UpdateChildWeight` | from 2012 0x19ec90 | the weights array is `m_pParentAnimTree`'s, not `SkelComponent`'s — a component can be evaluating a nested tree |
| `USkeletalMeshComponent::DeleteAnimTree` / `Detach` / `UpdateAnimations`, `UAnimNode::GetNodesByClass`, `UAnimNodeSlot`'s `AnimAlwaysTickArray` uses, `APawn`'s slot-node scan | — | routed through the owning tree via three accessors declared in `UnAnimTree.h` (`DisAnimTickTree`, `DisAnimTickArray`, `DisAnimAlwaysTickArray`) |

No member was added to any class, so the layout is unchanged; the four shims stay in `UnSkeletalMesh.h` with nothing
reading them any more.

## The Arkane node set

The player's tweak tree `Ply_Player_at` (dumped in-engine with the new `-disanimdump`) routes the **entire** body
pose through three `DishonoredAnimNodeStatePicker`s whose children are `DishonoredAnimNodeTreeRef_Dynamic` slots:

```
DishonoredAnimTree
  DisAnimNodeBlendByStepUpMantle 'StepUpMantle'
    ArkAnimNodeBlendPose 'PoseBlender'
      ArkAnimNodeStack 'PlayerStack'
        DisAnimNodeBlendPerBone 'LeftArmFilter'
          DisAnimNodeBlendPerBone 'UpperBodyFilter'
            DishonoredAnimNodeStatePicker 'FullBodyStatePicker'   <- 3 x TreeRef_Dynamic, 14 templates
            DishonoredAnimNodeStatePicker 'UpperBodyStatePicker'  <- 3 x TreeRef_Dynamic, 4 templates
          DisAnimNodeBlendHeartAdditive -> DisAnimNodeBlendItemAimAdditive
            DishonoredAnimNodeStatePicker 'LeftArmStatePicker'    <- 3 x TreeRef_Dynamic, 7 templates
        4 x DishonoredAnimNodeSeq (PlayerSeq1..4)
```

With nothing driving the pickers every child weight is 0, which is legal in retail: the picker's own `GetBoneAtoms`
(2013 rva 0x67de60) answers the reference pose below `ZERO_ANIMWEIGHT_THRESH` instead of entering
`UAnimNodeBlendBase::GetBoneAtoms`. Our build had no override, so the base asserted
(`check(LastChildNonAddIndex != INDEX_NONE)`, `UnAnimTree.cpp:1646` — the assert is outside `#ifdef _DEBUG`, so the
Release build died too). That single missing override is what the gate was hiding.

**Ported, with retail bodies** (2013 rvas in `agentAV_status.csv`):

| Class | Functions |
|---|---|
| `UDishonoredAnimNodeStatePicker` | `InitAnim`, `InitStateSlots`, `TickAnim`, `GetBoneAtoms`, `OnCeaseRelevant`, `OnRemoveChild`, `PostAnimNodeInstance`, `RenameChildren`, `ClearState`, `CleanupForAnimStates`, `IsStateFullWeight`, `GetDominantTree`, `HasActiveState`, `SetState` (both overloads) — 15 |
| `UDishonoredAnimNodeTreeRef` | `InitTreeInstance`, `InitAnim`, `TickAnim`, `GetBoneAtoms`, `GetNodesInternal`, `CallDeferredInitAnim`, `PostAnimNodeInstance` — 7 |
| `UDishonoredAnimNodeTreeRef_Dynamic` | `InitAnim`, `PostAnimNodeInstance`, `BuildParentNodesArray`, `BuildTickArray`, `CheckForReleaseToPool`, `ClearActiveTreeReference`, `CleanupForAnimStates`, `OnCeaseRelevant`, `SetActiveTreeReference` — 9 |
| `UDishonoredAnimTree` | `InitAnim`, `OnCeaseRelevant`, `EndAnimState`, `GetAnimStateInfo`, `IsAnimStateActive`, `GetAnimStateFireInterface` — 6 |
| `UDisAnimStatePool` | `FindAnimTreePool`, `AddTreesToPool`, `CallPostAnimNodeInstance`, `GetPooledTree`, `ReturnToPool`, `UpdatePoolSettings` — 6 |
| Engine | the 7 rows of the table above |

**Still warning / not ported** (each with its rva in the status CSV):

| What | Why |
|---|---|
| `UDishonoredAnimNodeStatePicker::GetChildBoneAtoms` (2013 0x69ad60) | the anchor-bone fixup: while one state blends out, retail overwrites the anchor bone (a `UDisTweaks_Pawn` bone name) of every non-dominant child with the dominant state tree's atom, so both pivot around the same joint. A warn-once `DISHONORED(bringup)` line fires the first time two states overlap; a cross-fade can drift at the anchor until it is ported |
| every `BuildEdgeAnimTree` override (picker, both tree-refs) | the whole-tree Edge path (`FEdgeAnimTreeContext`, `BuildEdgeAnimTreeEx`) does not exist in this tree — agent W's Plan B is per-sequence and bit-exact, and his follow-up 5 owns the whole-tree route |
| `UDishonoredAnimTree::GetAnimStateEquipUsage` (2013 0x69a1a0), `TransformSingleBone` | need `UDisItemContext` / `UStatePlayerAction` (agent AU) and `GetChildBoneAtoms` respectively |
| `GetConnectionLocation`, `GetNodeTitle`, `DrawAnimNode` on the tree-ref | editor only |
| `UDisAnimStateComponent` (18), `IDisAnimStateOwnerInterface` (4+5), `ADishonoredPlayerPawn::PreBeginPlay_AnimStates` and its 6 helpers | the retail **driver**, see "The remaining link" |
| the `DisAnimNodeBlendBy*` / `DishonoredAnimNodeBlendBy*` family (24 units), `DishonoredAnimNodeSeq`, `DisAnimNodeSeqPlayer`, `DisAnimNodeSlot`, `DisAnimNode3StateBlend` | still comment-only stubs. They inherit working reference bases, so the tree evaluates; what is missing is the per-frame *selection* (`DishonoredAnimNodeBlendByState::TickAnim` 2012 0x6d8ea0, `DishonoredAnimNodeBlendBySpeed::CalcSpeed` 2012 0x6f7100, `DisAnimNode3StateBlend::TickAnim` 2012 0x6b1d70, `DisAnimNodeSlot::TickAnim` 2012 0x6eebb0, `DisAnimNodeSeqPlayer::GetBoneAtoms` 2012 0x6ef0f0 with its camera bob) so the active child is whatever the cooked defaults left |

## Evidence that bones move

Two bring-up censuses, both in `Engine/Src/UnAnimTree.cpp`:

- **`-disanimdump`** prints each instanced tree once (`UAnimTree::InitAnim`): class, node name, child names and
  weights, and for a sequence node its `AnimSeqName`, resolved `AnimSeq`, `bPlaying`, rate and time.
- **`-disanimcensus`** keeps the previous root pose per tree (7 floats a bone) and reports, every 120 root
  evaluations, how many bones changed (>0.0005 on a quaternion component, >0.01 on a translation component); on the
  first evaluation it also reports how many of the tree's sequence nodes resolved their animation.

Baseline, HEAD, L_Tower_P, 90 s d3d9: **11 anim trees, 0 moving bones anywhere**, and the player pawn had no tree at
all (with the gate on, `AnimTreeTemplate` stayed NULL, so the pawn was a reference pose by construction):

```
animcensus owner=SkeletalMeshActorMAT_8 tree=AnimTree_5 bones=17 evals=2400 movingEvals=0 movedBones last=0 max=0
```

After, same map and command line, the player pawn has its tweak tree (`DishonoredAnimTree_88`, 79 bones) and 176
state-tree instances; with an anim state set (see below) the pose census reads:

```
animcensus owner=DishonoredPlayerPawn tree=DishonoredAnimTree_88 bones=79 evals=1200 movingEvals=1195 movedBones last=29 max=48
animcensus owner=DishonoredPlayerPawn tree=DishonoredAnimTree_94 bones=79 evals=1080 movingEvals=1076 movedBones last=28 max=48
```

`DishonoredAnimTree_94` is the `Ply_Player_Nav_Swimming_at` instance inside the picker's slot, so the chain proven end
to end is: cooked Edge sequence (`Empty_SwimIdle`, `Empty_SwimN/S/E/W`) → agent W's per-sequence evaluator →
`AnimNodeBlendDirectional` → the state tree's root → `UAnimTree::TickTree` / `GetBoneAtoms` →
`UDishonoredAnimNodeTreeRef_Dynamic` → `UDishonoredAnimNodeStatePicker` → the two `DisAnimNodeBlendPerBone` filters →
`ArkAnimNodeStack` → `ArkAnimNodeBlendPose` → the pawn's root tree. 48 of 79 bones move.

Screenshots (`-apshot=400`, `build/agentAV/shot_refpose.bmp` and `shot_swim.bmp`) are the same frame with the state
picker idle and with `Nav_Swim` set; L_Tower_P starts in first person, so the census is the stronger exhibit, as the
package said it would be.

### Bringing a state up without the retail driver: `-disanimstate[=<TreeName>]`

Nothing in the build calls `SetState` yet, so `UDishonoredAnimNodeStatePicker::TickAnim` carries a bring-up switch
that sets the first (or the named) entry of the picker's `m_PossibleTemplates` once. It is not a retail code path and
it is off by default; it exists so the ported chain can be exercised and measured. The dump also lists the templates:

```
picker 'FullBodyStatePicker'  template[0] Nav_Default tree Ply_Player_Nav_Default_at pooled=0   ... 14 templates
picker 'UpperBodyStatePicker' template[0] SimpleAction tree Ply_Player_SimpleAction_at          ... 4
picker 'LeftArmStatePicker'   template[0] SimpleAction tree Ply_Player_SimpleAction_at          ... 7
```

Measured per state (movedBones max, of 79):

| `-disanimstate=` | tree | moved bones | note |
|---|---|---|---|
| `Nav_Swim` | `Ply_Player_Nav_Swimming_at` | **48** | looping full-body swim, the exhibit above |
| `Mantle` | `Ply_Player_Mantle_at` | 20 | one-shot, so it plays out and stops |
| (default `Nav_Default`) | `Ply_Player_Nav_Default_at` | 1 | its full-body nodes are the runtime `Custom*` slots (`CustomIdle`/`CustomWalk`/`CustomAirIdle`, `anim=None`), which `FDisComponentAnimPlayer` fills; the one bone is the `ADD_Head_OutOfbreath` additive |
| `Stunned` | `Ply_Player_Stunned_at` | 0 | `Stun_Loop` is not in the pawn's 9 anim sets |

Census of animation resolution (first evaluation of each tree), which is worth keeping:
`animSets=9 namedSeqNodes=9 resolved=1 firstMissing=ADD_Head_WalkN` for `Ply_Player_Nav_Default_at` — the additive
head-bob set is not among the pawn's tweak anim sets, so 8 of its 9 named sequences do not resolve.

## The remaining link (hand-over to AU)

Retail's driver is, in order:

```
ADishonoredPawn::PreBeginPlay                     2012 0x78e220 (78 bytes)
  -> ADishonoredPlayerPawn::PreBeginPlay_Actions  2013 0x6a4a50
     -> ADishonoredPlayerPawn::PreBeginPlay_AnimStates 2013 0x6e2060
        -> FindStatePickers 2012 0x71ed50, BuildAnimStateMap 2013 0x6dfc70, GatherAnimStateArrays
        -> ADishonoredPlayerPawn::SetAnimState 2013 0x6e1da0
           -> UDisAnimStateComponent::SetAnimState 2013 0x6cc0d0
              -> UDishonoredAnimNodeStatePicker::SetState   (ported)
```

`PreBeginPlay_AnimStates` reads the pawn's first full-body / upper-body / special anim state out of
`UDisTweaks_Pawn`'s `m_pFullBodyTweaks` / `m_pUpperBodyTweaks` / `m_pSpecialTweaks`; for the player that is
`Nav_Default`. Everything below `SetAnimState` is now ported, and `ADishonoredPawn::PreBeginPlay` is 78 bytes
(`AActor::PreBeginPlay`, `UArkComponentContainer::StartAllComponents` — not ported — and four `PreBeginPlay_*`
virtuals). The rest of the chain lives in `dishonoredpawn.cpp`, `dishonoredplayerpawn_action.cpp`,
`displayerpawn_animstates.cpp` and `disanimstatecomponent.cpp`, i.e. AU's package and the player FSM states
(`UStatePlayerMasterWalk`, `UStatePlayerUpperIdle`, whose `OnEnterState` calls `SetAnimState` for every later state);
the full-body `Custom*` sequence names come from `FDisComponentAnimPlayer` (2012 0x8af2d0 ff), also AU's.
Decompiles for all of it are in `resources/reference/decomp/agentAV/s12`, which is why the 213 files are kept.

## Other findings, outside my files

1. **396 `*** Found bad AnimNotify` errors at load** (`Ply_Sword_Locomotion_as`, `Npc_SmallRat_as`, `Ply_Generic_as`,
   …). The check is `Notify->GetOuter() == Sequence`; every failure names a *different* `AnimSequence` of the same
   AnimSet as the notify's outer, and the two indices are consistently off. That reads as an export-index mapping
   defect in `UAnimSet` / `UAnimSequence` serialization, not as content damage. It is present on HEAD with the gate
   on as well, so it is not mine; it belongs with the serialization owner (agents O / AA).
2. `USkeletalMeshComponent::AnimTickRelevancyArray` / `AnimTickWeightsArray` are now dead shims with no readers; the
   header owner can delete all four from `UnSkeletalMesh.h` when convenient (agent S's follow-up 3d pattern).
3. Frame rate on d3d9 L_Tower_P is **2,640 scene frames in the 90 s accept run (~29 fps)**, and identical with the
   animation playing and with the picker idle. AU's and AT's runs of the same map on the same day are 3,090/129 s and
   3,390/119 s, i.e. 24 and 28 fps, so the animated player costs nothing measurable; the `20,370 frames / 90 s`
   figure in PHASE7's baseline table is a null-RHI number and should be labelled as such (AX).
4. 176 state trees are `CopyAnimTree`'d per player pawn because every cooked `FDynamicTreeTemplate` of the three
   pickers has `m_bUseAnimTreePooling = 0`. That is what retail does; `UDisAnimStatePool` is ported and ready for the
   content that does enable pooling.

## Accept runs

```
# the package's accept command, no -distweakanimtree anywhere
python resources/tools/build_and_smoke.py --build-dir build/agentAV --no-build --exe-name DishonoredGame_AV.exe
  --log-name agentAV.log --ini-dir build/agentAV/config --rhi d3d9 --timeout 90
  --milestone "Initializing Engine..." --expect "Initial startup" --forbid "Critical" --skip-native OnlineSubsystemPC
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
# exit 0; Initial startup reached; 0 Critical over 90 s; inputtest moved 1012.6 turned 7800 (baseline 1030.1);
# scene census unchanged (2672 prims, 126/6381 draw lists)

# the same with the pose census and a state set
  "--extra-args=-disanimcensus -disanimstate=Nav_Swim -startmap=L_Tower_P -startmapopen -forcelogflush -windowed
    -ResX=1280 -ResY=720 -nomovie"                                                            # exit 0, 48/79 bones
# and L_Pub_Day_P, 60 s                                                                        # exit 0
```

`DishonoredNPCPawn` is not spawned in any map reachable today (L_Tower_P and L_Pub_Day_P both bring up only the player
pawn plus `SkeletalMeshActorMAT`s and `DishonoredUsableObject`s), so "one non-player character animates" cannot be
shown yet: the level's skeletal actors are matinee-driven (`MatineeStackNode`, `AnimNodeSequence` with no anim — agent
AT) and the usable objects are driven by the use path (agent AS). The NPC pawn's own tree goes through the same three
ported classes, and `UDishonoredAnimTree::IsAnimStateActive` already returns TRUE unconditionally for an NPC.

## Files

- Changed, mine: `Engine/Src/UnAnimTree.cpp`, `Engine/Inc/UnAnimTree.h` (three accessors),
  `Engine/Inc/EngineAnimClasses.h` (two `UAnimTree` declarations), `Engine/Src/UnSkeletalComponent.cpp`,
  `Engine/Src/UnPawn.cpp` (one loop), `DishonoredGame/Src/dishonoredpawn.cpp` (the gate deleted only),
  `DishonoredGame/Inc/DishonoredGameNative.h` (three forward declarations),
  `DishonoredGame/Inc/{DishonoredGameAnimClasses.h,dishonoredgameclasses.h}` (five CppText includes),
  `DishonoredGame/Sources.cmake` (five units off `DishonoredGame_EXCLUDE`).
- New: `DishonoredGame/Inc/CppText/{UDishonoredAnimNodeStatePicker,UDishonoredAnimNodeTreeRef,`
  `UDishonoredAnimNodeTreeRef_Dynamic,UDishonoredAnimTree,UDisAnimStatePool}.h`.
- Bodies written into the five stub units: `dishonoredanimnodestatepicker.cpp`, `dishonoredanimnodetreeref.cpp`,
  `dishonoredanimnodetreeref_dynamic.cpp`, `dishonoredanimtree.cpp`, `disanimstatepool.cpp`.
- The generated-header includes and the `Sources.cmake` edit are reproduced by the coordinator's regeneration
  (agent AJ's two generator hooks are in HEAD), so nothing there has to be kept by hand.
- Scratch, not repo tools: `build/agentAV_work/` (`patch_census.py`, `patch_census2.py`, `patch_census3.py`,
  `patch_engine.py`, `patch_engine_hdr.py`, `patch_dishonoredgame.py`, `patch_tickarrays.py`,
  `patch_initanimtree.py`, `patch_bringupstate.py`, `crlf.py`, `xrefs_to.py`, `list_names.py`, `sumdump.py`,
  `files.txt`); `build/agentAV_snap_build.cmd`; snapshot `build/agentAV_wt`; IDA copies
  `resources/docs/idb/{retail2013,shipping2012}_agentAV.i64`.

## Commands

```
python resources/tools/make_snapshot.py AV --list build/agentAV_work/files.txt   # or --sync after each edit
cmd /c build\agentAV_snap_build.cmd                                             # Release build of the snapshot
python build/agentAV_work/crlf.py                                               # CRLF (Path.read_text eats it)
python build/agentAV_work/sumdump.py <log>                                       # baked anim names per instanced tree
python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py resources/docs/idb/shipping2012_agentAV.i64 resources/reference/decomp/agentAV/s12 rva:0x1ae3c0
python resources/tools/ida/run.py build/agentAV_work/xrefs_to.py resources/docs/idb/shipping2012_agentAV.i64 out.txt rva:0x71fe40
```

New switches, all off by default: `-disanimdump`, `-disanimcensus`, `-disanimstate[=<TreeName>]`.
