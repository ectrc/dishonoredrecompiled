# Agent FK — the opening cutscene: a Dishonored matinee now initialises, and what stopped it was not in the brief

Wave 19 (`PHASE15.md`), package FK, fault 3: *"all of the animations and sequences do not happen"*.
Worktree `build/agentFK_wt`. **Base commit `cd5fadd`** — started on `17a7c32`, moved onto `98e22d4` at
the coordinator's instruction before any edit was made, then rebased onto `ee2313f` and `cd5fadd` as FJ,
FL and FM landed. Each rebase was a clean `git stash` / detach / `stash pop` with no conflict; FL's 703
new lines in `dishonoredplayercontroller.cpp` and this package's six sit side by side, verified by
grepping both `UArkAnimNodeLookAt::TickAnim` and `DisFKReport` out of the merged file.

Nineteen files: eleven changed, eight new. Nothing in `External/GFx3` (FJ's), nothing in
`Engine/Src/UnSkeletalAnim.cpp` (FL's), nothing in `Core/Src/UnMisc.cpp` or the GFx movie players (FM's).

## 1. The headline

**A Dishonored cutscene matinee with a director group and a player group now initialises and plays, with
the player's pawn bound to its player group and moved to its stage mark.** Measured, and with a control
on the same binary:

```
FK12.log [0081.83]  matinee SeqAct_Interp_26: player group 'PlayerGroup' instanced on DishonoredPlayerPawn
FK12.log [0081.83]  fkdiag blend SeqAct_Interp_26: DishonoredPlayerPawn moved 595.8 units,
                    -3901/36639/-225 -> -3428/36287/-141, mark 16243/21620/3355,
                    blendTime 0.20 t 0.00, rot 0/-11653/0
```

| `SOIREE_Empress_Regent.SeqAct_Interp_26`, forced, one binary | branch on (`FK10`) | `-fknoplayergroup` (`FK11`) |
|---|---|---|
| group 0 `PlayerGroup_StageMark` | `InterpGroupInst` on `MatineePreviewPlayerSpawner_3` | the same |
| group 1 `PlayerGroup` (`InterpGroupPlayer`) | **`InterpGroupInstPlayer` on `DishonoredPlayerPawn`** | `InterpGroupInst`, actor **none** |
| group 2 `DirGroup` (`InterpGroupDirector`) | `InterpGroupInstDirector` on `DishonoredPlayerController` | the same |
| the matinee | `playing 1 pos 2.53` | `playing 1 pos 2.54` |
| track instances | all three built, incl. `InterpTrackInstStretchAnimControl` | the same |

**And the thing that was actually stopping every Dishonored cutscene was none of the brief's three
gaps.** It was `UInterpGroupInst::InitGroupInst` raising the *script* event `BeginAnimControl`:

```
FK5.log:5247  Critical: appError called: Failed to find function BeginAnimControl in
              DishonoredSpawner L_Tower_Script.TheWorld:PersistentLevel.DishonoredSpawner_12
FK6.log       Critical: appError called: Failed to find function BeginAnimControl in
              DishonoredPlayerPawn DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredPlayerPawn
```

`eventBeginAnimControl` is `FindFunctionChecked`, Dishonored's script `Actor` and `Pawn` do not declare
these functions, and **retail does not use the script path at all**: `UInterpGroupInst::InitGroupInst`
(2012 rva `0x231210`) calls the native virtual `AActor::MAT_BeginAnimControl( Group )`. So the first
Dishonored cutscene to reach `InitGroupInst` killed the process, every time, on the first group that had
an anim-control track — which is every AI group and every player group in the game. §4 has the chain and
the four edits that close it.

## 2. Five premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "all seven matinees live in `l_tower_p` are set dressing with `director NONE`" | **True of the seven that run, and it hid the level.** `l_tower_p`'s nine levels hold **91 `SeqAct_Interp`, 396 groups, 694 tracks, 4 director groups, 6 player groups and 99 AI groups** (`-fkdiag`, identical in four independent runs). Director and player groups are in the data in quantity; what was missing was any matinee that owns one ever getting past `InitGroupInst` |
| "Without a player group nothing else can be observed, so this is first" | **The order is wrong and it cost this package most of its budget to find out.** The player group is third in line: the cutscene is never activated, and when forced it dies on the anim-control handshake before any group finishes instancing. The right first item was a one-line `appError` nobody had seen because nothing had ever made a Dishonored cutscene initialise |
| "`UpdateTrack` is missing on every Dishonored-added `UInterpTrack`" | **Wrong for six of nine.** Only `AIControl`, `Attachment` and `DisAllowPlayerLook` drive from `UpdateTrack`. `FaceTo`, `Locomotion`, `StretchAnimControl` and `Dialog` drive from `UInterpTrackInst*::UpdateTrackInst(FLOAT,UBOOL)`, **a virtual that exists nowhere in this tree**, and `LookAt` from a **static**, `UInterpTrackLookAt::StaticUpdateTracks`. The gap is a missing seam, not missing overrides — and `StretchAnimControl`, 267 of the level's 694 tracks and present on every one of the six player groups, is in the seam-less family (§6) |
| "`ADishonoredNPCPawn::MAT_*`, about forty functions" | **22** are on `ADishonoredNPCPawn`; **8** on `ADishonoredPawn`; **5** on `ADishonoredPlayerPawn`; 6 more on the matinee-preview classes and `UArkAnimNodeStack`. Forty-one in the layer, not on the class. The player group needs the **player** five, and `ADishonoredPlayerPawn::MAT_BeginPlayerGroup` requests a state change into `UStatePlayerMasterSoiree`, whose unit is on the exclude list — that half is blocked on the player FSM, not on the matinee (§7) |
| "twelve of fourteen `UDisSeqCond_*::Activated` are missing … these are not cosmetic" | **Count right, consequence wrong on this route.** Two of the twelve (`UDisSeqCond_DLC05_*`) have no `.cpp` at all. Of the ten that do, **eight cannot be ported without first porting a helper that is itself in an excluded unit** (§8). And the one FI singled out as firing on this mission, `DisSeqCond_CompareTweaks`, fires six times and **in every observed case the output link it takes has `0 connections`** — the missing body was killing nothing here |

Two more things the brief could not have known:

* **`UInterpGroupInst::UpdateGroupInst` and `UInterpGroup::NeedWaitToStart` — the two base virtuals this
  family overrides in retail — do not exist in this tree at all.** This engine still has the reference
  UE3 shape, `UInterpGroup::UpdateGroup( pos, GrInst, bPreview, bJump )`, so the per-frame blend is
  ported onto `UInterpGroupPlayer::UpdateGroup`: same test, same call, same position in the frame, a
  different object carrying it (§5).
* **`-finewgame=<secs>` is measured on the world clock, which starts before the first map loads.** Under
  another agent's build the first `LoadMap` did not begin until **t = 27.6 s**, so `-finewgame=22`
  issued `ce ChangeLvl_StartNewGame` into the middle of an async load and the process died on
  `Assertion failed: !Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading)` in `UObject::StaticAllocateObject`
  (`FK2.log:838`, 33.11 s). **This matters to FM**, whose New Game check is built on `-finewgame` with
  `--newgame-at` defaulting near 22: the value has to exceed the first map's load time. 50 worked in six
  runs; 22 failed in one of two.

## 3. The instrument: `-fkdiag`

`-fediag` reports the matinees that are *running*, off `USeqAct_Interp::GroupInst`. That array only
exists after `InitInterp`, so a matinee that never started reports nothing and one whose groups could
not be instanced reports `director NONE` whether the data has a director group or not. Both readings are
what this package had to tell apart, so `-fkdiag` walks **the `UInterpData` the package shipped**:

```
fkdiag interp   one line per USeqAct_Interp in the loaded world, playing or not, with its activation count
fkdiag group    one line per group in its UInterpData, with the group's CLASS and its matching instance
fkdiag track    one line per track in that group, with the track's class, key count and instance
fkdiag cond     the census of USequenceCondition classes in the loaded sequences, with activations
fkdiag CompareTweaks  which output link the condition took, and how many connections that link has
fkdiag blend    the first FarMoveActor of each UInterpGroupInstPlayer: from, to, mark, blend time, rotation
fkdiag census   the totals
```

Twice per map (`-fkdiagat=<seconds>` moves the second pass, default 45) plus a condition census every two
seconds. Nothing when the switch is absent. `Src/fkmatineediag.cpp`, `Inc/fkmatineediag.h`.
`-fknoplayergroup` is the control: it leaves the `InitInterp` branch off so one binary gives both
readings.

**The census, four runs, identical every time:**

| map | SeqAct_Interp | groups | tracks | director | player | AI |
|---|---|---|---|---|---|---|
| `DishonoredGameFull_P` | 1 | 1 | 2 | 1 | 0 | 0 |
| `Dishonored_MainMenu` | 13 | 29 | 57 | 10 | 0 | 0 |
| **`l_tower_p`** | **91** | **396** | **694** | **4** | **6** | **99** |

**`l_tower_p`'s track classes, by count** — 614 of the 694 are Dishonored-added:

```
267 InterpTrackStretchAnimControl   46 InterpTrackSoireeControl   21 InterpTrackAttachment
 99 InterpTrackLookAt               45 InterpTrackDialog           6 DisInterpTrackAllowPlayerLook
 53 InterpTrackFaceTo               42 InterpTrackLocomotion       4 InterpTrackAIControl
                                    31 InterpTrackAkEvent
```

and the reference-engine remainder: `Event` 35, `Move` 34, `Toggle` 19, `Visibility` 14, `FaceFX` 14,
`Director` 9, `Fade` 7, `FloatProp` 3, `VectorProp`/`Slomo`/`ParticleReplay`/`FloatMaterialParam` 1 each.

**The opening cutscene is
`L_Tower_Script.TheWorld:PersistentLevel.Main_Sequence.SOIREE_Empress_Regent.SeqAct_Interp_12`.**
Thirty-eight groups: eight `InterpGroupAI` named Empress, Emily, Lord_Regent, Campbell, Elite_A,
Elite_B, Daud and assassin; their eight stage marks; `PlayerGroup` (`InterpGroupPlayer`, 4 tracks);
`DirGroup` (`InterpGroupDirector`, 4 tracks); and the FX groups `Empress_Splatter` and `Empress_Hit`.
That is the assassination, in the data, with a director and a player group. **`activations 0` at 57 s and
again at 153 s of a new-game run** (`FK3.log`) — it is still never reached, and §9 item 2 says where to
look next.

**The six player groups** — every one carries a `StretchAnimControl` and a `DisAllowPlayerLook`:

| matinee | group | tracks |
|---|---|---|
| `l_tower_p…SeqAct_Interp_1` | 7 `PlayerGroup0` | StretchAnimControl(8 keys), AllowPlayerLook |
| `L_Tower_Fx…SeqAct_Interp_0` | 3 `PlayerGroup` | StretchAnimControl(1), AllowPlayerLook, AkEvent |
| `L_Tower_Script…SeqAct_Interp_12` | 19 `PlayerGroup` | StretchAnimControl(8), StretchAnimControl(16), AllowPlayerLook, AkEvent |
| `SOIREE_Empress_Regent.SeqAct_Interp_26` | 1 `PlayerGroup` | StretchAnimControl, SoireeControl, AllowPlayerLook |
| `SOIREE_Empress_Regent.SeqAct_Interp_27` | 1 `PlayerGroup` | StretchAnimControl, SoireeControl, AllowPlayerLook |
| `L_Tower_Script…SeqAct_Interp_9` | 1 `PlayerGroup` | StretchAnimControl(4), AllowPlayerLook, Attachment, AkEvent |

## 4. The anim-control handshake: the real blocker, and four edits

`-diskismetforce=<substring>` (agent EP's, already in the tree) fires input 0 of the matching ops once.
Pointed at the cutscene, it got three `appError`s in three successive measured runs, each one step deeper
down the same chain, each a reference-UE3 **script event** that Dishonored's script classes do not have
and that `FindFunctionChecked` turns into a fatal error rather than a no-op:

| run | the appError | the fix |
|---|---|---|
| `FK5`/`FK6` | `Failed to find function BeginAnimControl in DishonoredSpawner` / `in DishonoredPlayerPawn` | `UInterpGroupInst::InitGroupInst` and `TermGroupInst` route through `DisMatBeginAnimControl` / `DisMatFinishAnimControl` |
| `FK8` | `Failed to find function RestoreAnimSetsToDefault in DishonoredPlayerPawn` | `APawn::RestoreAnimSetsToDefault` gains a `FindFunction` guard before asking script |
| `FK9` | `Failed to find function BuildScriptAnimSetList in DishonoredPlayerPawn` | `APawn::BuildAnimSetList` gains the same guard; `UpdateAnimSetList`'s `eventAnimSetListUpdated` too |
| `FK10` | **no appError**; the player group instances and the matinee plays | — |

**Retail's `UInterpGroupInst::InitGroupInst`, 2012 rva `0x231210`, 424 bytes, read in full:**

```c
Group = InGroup;  GroupActor = InGroupActor;
m_pGroupPawn = GetPawn( InGroupActor );                       // this tree never wrote this member
for( each track ) { TrInst = ConstructObject( Track->TrackInstClass, this ); TrackInst.AddItem; TrInst->InitTrackInst( Track ); }
m_bHasAnimControlTrack = Group->HasAnimControlTrack();        // cached on this+108 bit 0; never written here either
if( GroupActor && !GroupActor->bDeleteMe && !(ObjectFlags & RF_PendingKill) )
{
    if( m_bHasAnimControlTrack )  GroupActor->MAT_BeginAnimControl( Group );   // NATIVE, not the script event
    BackupCollisionType = GroupActor->CollisionType;
    if( Group->bBecomeDynamic )  GroupActor->MAT_SetPhysics( FALSE, BackupCollisionType );
}
```

So retail has **no script path at all** and the reference engine has **only** a script path. Three
differences from this tree, all now closed except the last:

1. the native `MAT_BeginAnimControl` instead of `eventBeginAnimControl` — closed, see below;
2. `m_pGroupPawn` and `m_bHasAnimControlTrack` were never written in this tree — now written;
3. `BackupCollisionType` / `MAT_SetPhysics( FALSE, … )` on `Group->bBecomeDynamic` (`UInterpGroup+108`
   mask `0x20`, which is `bBecomeDynamic` in this tree's bit order) — **not ported**, because
   `AActor::MAT_SetPhysics` does not exist here either. Specified, §9 item 4.

**The deviation, stated plainly.** Retail's `MAT_BeginAnimControl` is a virtual on `AActor`; in this tree
it is a virtual on `APawn` only (`EnginePawnClasses.h:534-535`), and the reference actors the ambient
matinees already drive — `ASkeletalMeshActorMAT` and friends — **do** have the script function and are
working through it today. `DisMatBeginAnimControl` therefore prefers the script function when the actor's
class has it and falls back to `APawn::MAT_BeginAnimControl` when it does not, with a one-line warning
when it has neither. That is strictly safer than retail's single path, changes no actor that works now,
and is the smallest thing that stops the crash. Putting `MAT_BeginAnimControl` on `AActor` is retail's
own shape and belongs to whoever ports the `MAT_*` contract.

The three `APawn` guards are the same shape and do not change any actor whose script class has the
function. `APawn::RestoreAnimSetsToDefault` already had the native fallback written —
`if( (GIsEditor && !GIsGame) || !eventRestoreAnimSetsToDefault() )` — so all the guard does is stop the
engine from *asking* a class that cannot answer.

## 5. `UInterpGroupPlayer` and `UInterpGroupInstPlayer`, ported

`Engine/Src/interpgroupplayer.cpp` stays a comment-only skeleton and has to: **both classes are declared
in `DishonoredGame/Inc/DishonoredGameEngineShims.h`** (they are Engine-package classes the Engine header
generation left out) and `IMPLEMENT_CLASS`'d in `DishonoredGameRegistrants.cpp`, so no Engine unit can
name them. The bodies are in `DishonoredGame/Src/interpgroupplayer.cpp`, which is where the C++ vtable
that carries them is emitted.

| retail | 2013 rva | 2012 rva | bytes | ported |
|---|---|---|---|---|
| `UInterpGroupPlayer::NeedWaitToStart` | `0x4fc450` | `0x53b610` | 66 | yes, **unreachable** (no base virtual, no caller — §9) |
| `UInterpGroupInstPlayer::GetGroupActor` | `0x4fc4a0` | `0x53b660` | 5 | no: a single `jmp` to the base |
| `UInterpGroupInstPlayer::GetGroupPawn` | `0x4fc4b0` | `0x53b670` | 5 | no: a single `jmp` to the base |
| `UInterpGroupInstPlayer::HasActor` | `0x4fc4c0` | `0x53b680` | 50 | no: byte for byte the `return GetGroupActor() == InActor;` the base already has inline |
| `UInterpGroupInstPlayer::GetStageMarkPosition` | `0x4fc500` | `0x53b6c0` | 581 | yes |
| `UInterpGroupInstPlayer::BlendPlayerToMark` | `0x4fd880` | `0x53c5c0` | 607 | yes |
| `UInterpGroupInstPlayer::UpdateGroupInst` | `0x4fe8c0` | `0x53dd00` | 66 | yes, as `UInterpGroupPlayer::UpdateGroup` |
| `UInterpGroupInstPlayer::TermGroupInst` | `0x4fe910` | `0x53dd50` | 653 | yes |
| `UInterpGroupInstPlayer::InitGroupInst` | `0x5027f0` | `0x541960` | 309 | yes |

**Every one of those nine is the same size in both executables**, which is the second pin: the bodies
were read out of the 2012 binary where the Shipping PDB names them, and the 2013 database names all nine
independently at the rvas above (`functions_2013.csv`, confirmed function starts with those sizes).

* **`InitGroupInst`** stores `Group`, `GroupActor` and `m_pPlayerGroup`, then in game saves
  `m_StartPos = Actor->Location` and `m_StartRot` from **`APawn::Controller->Rotation`** (actor `+620`,
  `APawn::Controller` in the 2012 PDB; its `+208` is `AActor::Rotation`) falling back to the actor's own
  rotation — the *view* rotation, not the mesh's. Clears `m_bFinishedBlendIn` (`this+148` bit 0), chains
  to the base, then registers the matinee as a latent action on the player both ways round:
  `Seq->LatentActors.AddUniqueItem( Actor )` (`Seq+248`) and `Actor->LatentActions.AddUniqueItem( Seq )`
  (actor `+572`). Retail then calls `MAT_BeginPlayerGroup` and `MAT_SetForceInventory`; neither is
  reachable here and each logs once instead (§7).
  **Deviation**: retail reads `+620` off the group actor with no type check, because for a player group
  the group actor is the player's pawn. This uses `Cast<APawn>`, identical when it is a pawn and safe
  when it is not.
* **`GetStageMarkPosition`** finds the group instance named by `m_pPlayerGroup->m_StageMarkGroup`
  (`USeqAct_Interp::FindFirstGroupInstByName`), takes its actor's location and yaw/pitch with the roll
  dropped, and drops the point to the floor with a 100-unit `TRACE_World` line check **unless**
  `m_bDisablePlayerCollision` (`UInterpGroupPlayer+124` mask `0x4`). Retail's trace-flag literal `0x2086`
  is exactly `TRACE_Level|TRACE_Movers|TRACE_LevelGeometry|TRACE_Terrain` = `TRACE_World`
  (`Engine/Inc/UnLevel.h:321`).
* **`BlendPlayerToMark`** adds `APawn::CylinderComponent->CollisionHeight` (pawn `+992`, component `+452`)
  to the mark's Z, because the mark is on the floor and the pawn's location is its centre; lerps the
  position and `SlerpQuat`s the rotation from `m_StartPos`/`m_StartRot` over
  `m_fTimeToBlendToStageMark` (clamped 0..1; `<= SMALL_NUMBER` means snap and finish); then
  `GWorld->FarMoveActor( Player, NewPos, FALSE, m_bDisablePlayerCollision, TRUE )` and **two** rotation
  writes — the controller gets the whole rotation, the pawn gets it with `Pitch = 0`. Retail's
  `FarMoveActor` takes a sixth `FLOAT` this tree's does not; the five that exist line up one for one.
  Measured working: `FK12.log`'s `fkdiag blend` line in §1.
* **`TermGroupInst`** calls `MAT_BlendOut` unless the actor is both `m_bLegalToTerminate` and
  `m_bHasTerminated` (actor `+300` masks `0x10` and `0x8`), chains to the base, calls
  `MAT_FinishPlayerGroup`, then optionally snaps the pawn to its root bone:
  `Mesh->GetBoneMatrix(0)` (pawn `+988`), `+ CollisionHeight` for the location, and for the rotation
  `FRotationTranslationMatrix( FRotator(-16384,0,0), FVector::ZeroVector ) * RootTM` with pitch and roll
  zeroed — retail's literal `-16384` is a 90° pitch correction, the root bone points up where the pawn
  points forward.

### The `InitInterp` branch

Retail's `USeqAct_Interp::InitInterp` (2012 `0x25ad70`, **returns `UBOOL`**) has four branches —
Director, **Player**, AI, plain. This tree's has Director, AI, Camera, plain. The Player branch builds a
`UInterpGroupInstPlayer` bound to `GWorld->GetWorldInfo()->GetALocalPlayerController()->Pawn`
(`AController::Pawn` at `+584`), and `NULL` out of game.

`UnInterpolation.cpp` cannot name `UInterpGroupInstPlayer`, so the branch is installed through
**`GDisInterpPlayerGroupHook`**, the same function-pointer seam shape `GDisEngineTickHook`,
`GDisPlayerPreRenderHook` and `GDisAudioFrameHook` already use: declared in `Engine/Inc/UnWorld.h`,
defined `NULL` in `UnInterpolation.cpp`, installed from `UDishonoredEngine::Init`. `InitInterp` calls it
in retail's position — after the director branch, before the AI/plain ones — and falls through when it
returns `FALSE`, so the Engine alone behaves exactly as before.

Three further differences between retail's `InitInterp` and this tree's, **read and not changed**,
because none of them is fault 3 and each changes behaviour on every matinee in the game:

1. retail returns `UBOOL` and returns `FALSE` early when `USeqAct_Interp::CanPlay()` (2012 `0x24db10`)
   is false, clearing `InterpData`; ours returns `void` and has no `CanPlay`;
2. retail calls `FilterObjectDuplicate( ObjectVars )` (2012 `0x23a680`) before instancing plain groups;
3. retail brackets the loop's end with `GWorld->RemoveLevelNavList( CurrentLevel, FALSE )` /
   `AddLevelNavList`. Ours also has a `UInterpGroupCamera` branch retail does not have.

## 6. `UpdateTrack` — the gap is a seam, not a set of overrides

| track class | retail's per-frame entry point | 2012 rva | in `l_tower_p` | this tree |
|---|---|---|---|---|
| `UInterpTrackAIControl` | `UpdateTrack( FLOAT, UInterpTrackInst*, UBOOL )` | `0x53e120` | 4 | base virtual exists |
| `UInterpTrackAttachment` | `UpdateTrack` | `0x53e560` | 21 | base virtual exists |
| `UDisInterpTrackAllowPlayerLook` | `UpdateTrack` | `0x8dc4a0` | 6 | base virtual exists |
| `UInterpTrackInstFaceTo` | **`UpdateTrackInst( FLOAT, UBOOL )`** | `0x53eb90` | 53 | **no such virtual anywhere** |
| `UInterpTrackInstLocomotion` | **`UpdateTrackInst`** | `0x542630` | 42 | **no such virtual** |
| `UInterpTrackInstStretchAnimControl` | **`UpdateTrackInst`** | `0x543f40` | **267** | **no such virtual** |
| `UInterpTrackInstDialog` | **`UpdateTrackInst`** | `0x8ec050` | 45 | **no such virtual** |
| `UInterpTrackLookAt` | **`static StaticUpdateTracks( FLOAT, UInterpGroupInstAI*, UBOOL, UBOOL )`** | `0x547010` | 99 | static, no caller |
| `UInterpTrackSoireeControl` | nothing in `interptracksoireecontrol.cpp` | — | 46 | — |
| `UInterpTrackAkEvent` | no unit in the tree at all | — | 31 | — |

**None is ported by this package.** The four `UpdateTrackInst` ones need a new virtual on
`UInterpTrackInst` and a call site in `UInterpGroup::UpdateGroup`'s track loop, and `LookAt` needs its
static called once per `UInterpGroupInstAI`; both are Engine edits that change the update order of
**every** matinee in the game, which is not something to land at the end of a budget without a control.

## 7. The `MAT_*` contract — forty-one functions, and which half matters

| class | functions | the rvas (2012, from the skeleton banners) |
|---|---|---|
| `ADishonoredNPCPawn` | 22 | `MAT_BeginAIGroup` `0x7c0570`, `MAT_FinishAIGroup` `0x7c0830`, `MAT_SendBodyIntentionRequest` `0x7c0a10`, `MAT_IsAllowed` `0x7c0c20`, `MAT_PausedAIGroup` `0x7ad570`, `MAT_ResumedAIGroup` `0x7ad5c0`, `MAT_BlendOut` `0x7ad5e0`, `MAT_IsUnequipped` `0x7ad6b0`, `MAT_ShouldForceInventory` `0x7ad720`, `MAT_GetBodyIntention` `0x7ad740`, `MAT_SendAimRequest` `0x7ad790`, `MAT_SendAimStopRequest` `0x7ad810`, `MAT_SendFireRequest` `0x7ad840`, `MAT_BlendStop` `0x7ab5b0`, `MAT_SetForceInventory` `0x7ab5c0`, `MAT_Attach` `0x7ab5e0`, `MAT_Detach` `0x7ab690`, `MAT_SetMeshTranslationMode` `0x7ab6e0`, `MAT_PermitLODPause` `0x7ab710`, `MAT_DisableMeshTranslation` `0x7ab720`, `MAT_SetCollision` `0x7ab7a0`, `MAT_IsCollisionDisable` `0x7ab8a0` |
| `ADishonoredPawn` | 8 | the shared base half |
| `ADishonoredPlayerPawn` | 5 | `MAT_IsUnequipped` `0x6fb030` (27 B), `MAT_SetForceInventory` `0x6fe570` (222 B), `MAT_BlendOut` `0x705120` (193 B), `MAT_FinishPlayerGroup` `0x7051f0` (82 B), `MAT_BeginPlayerGroup` `0x70fcc0` (257 B) |
| preview + `UArkAnimNodeStack` | 6 | editor only |

**`MAT_BeginPlayerGroup` is blocked on the player FSM, not on the matinee.** Read out of the 2012 binary
at `0x70fcc0`: enable `m_pMatineeBlender`; `StopSneaking(FALSE)`;
`SetBodyMode( m_bPutPlayerInWorldDepthGroup )`; a flag word from `m_bDisablePlayerCollision` (bit 1) and
`UInterpGroup::HasMoveTrack()` (bit 2); then either
`UStatePlayerMasterInDialog::StartSoireeControl( Matinee, m_fTimeToBlendToStageMark, Flags )` when the
master FSM is already in dialog, or `RequestStateChange( FStatePlayerMasterSoiree_Param )` into
**`UStatePlayerMasterSoiree`**. In this tree `UStatePlayerMasterSoiree` and
`UStatePlayerMasterInDialog::StartSoireeControl` are in excluded units, and
`ADishonoredPlayerPawn::m_pMatineeBlender` and `UInterpGroup::HasMoveTrack` **do not exist at all**.

The five call sites log once each instead of calling, so the gap is visible in any log. Measured in
`FK10.log` and `FK12.log`:

```
Warning: the player group reached ADishonoredPlayerPawn::MAT_BeginPlayerGroup and it is not ported
         in this tree; the matinee continues without it
Warning: the player group reached ADishonoredPlayerPawn::MAT_SetForceInventory and it is not ported …
```

**One deviation worth naming**: retail declares the whole set as virtuals on `APawn`. Every call site in
this package is in `DishonoredGame`, so nothing was added to the generated `EnginePawnClasses.h`; a later
package porting the NPC half will have to decide whether to put the virtuals on `APawn` (retail's shape,
and it changes the Engine pawn vtable) or to keep casting in `DishonoredGame`.

## 8. The conditions — two ported, and the one that fires changes nothing

Fourteen `UDisSeqCond_*` classes. Two already had bodies (`CheckStoryFlag`, `IsSentinel`). Two
(`UDisSeqCond_DLC05_IsDollAlreadyCollected`, `UDisSeqCond_DLC05_IsExpertMode`) have **no `.cpp` at all**.
Ten have a comment-only skeleton on the exclude list, and **eight of those ten depend on a function that
is itself unported**:

| condition | 2013 rva | blocked on |
|---|---|---|
| **`CompareTweaks`** | `0x78eaf0` (296 B, 2012 `0x7d1a50`, **296 B — identical**) | nothing. **Ported** |
| **`CompareBoolExtended`** | `0x79b3e0` (521 B, 2012 `0x7d6cb0`, 540 B) | nothing. **Ported** |
| `InStoryGroup` | `0x78ecb0` | `IDisStoryGroupInterface::InStoryGroup` (2012 `0x8e0370`) |
| `IsPlayerInCombat` | `0x78efc0` | `ADishonoredPlayerPawn::IsInCombat` (`0x6fb440`) |
| `IsDoorOpen` | `0x78eec0` | `ADisDoor::GetDoorState` (`0x66aff0`, `disdoor.cpp` excluded) |
| `PawnIsPossessed` | `0x79b5f0` | `ADishonoredNPCPawn::DisIsPossessed` (`0x7af1d0`), `ADishonoredPlayerPawn::DisIsPossessing` (`0x6fb7d0`) |
| `HasObjective` | `0x78ec20` | `UDishonoredObjectivesComponent::HasObjective` (`0x72d7b0`, unit excluded) |
| `IsObjectiveComplete` | `0x78ef30` | `UDishonoredObjectivesComponent::IsObjectiveComplete` (`0x72f7d0`) |
| `TaskState` | `0x7a78c0` | `GetTaskInstance` (`0x72d5d0`), `UDishonoredTask_Base::GetTaskState` (`0x721f80`), `DisGetTaskVars` (`0x839de0`) |
| `IsDLCUnlocked` | `0x78ee30` | `UDisGlobalDLCManager::GetDLCStatus` (`0x8b6f10`); DLC, on no mission path |

`CompareBoolExtended`'s 2013 address is **not independently pinned**: `match_2012_2013.csv` maps it by
"neighbours" at ratio 0.838 and the sizes differ (540 vs 521). Nothing in the port rests on it — the body
was read from the 2012 binary where the Shipping PDB names it. Same for `PawnIsPossessed` (0.748) and
`HasObjective` (0.865). `CompareTweaks` needs no caveat: named in both databases at identical size, and
`IsSentinel`'s already-ported `0x78f070` sits in the same block, which is the mapping's own check.

**And the result on this mission is negative, which is the point of measuring.** With
`CompareTweaks::Activated` in place and `-fkdiag` logging which link it takes:

```
FK5.log  fkdiag CompareTweaks …SOIREE_Emily.DisSeqCond_CompareTweaks_1:
         A Twk_Pawn_Empress          B Pwn_LadyEmily -> output 2, link pulsed 0 connection(s)
         A Twk_Pawn_LordRegentPrison B Pwn_LadyEmily -> output 2, link pulsed 0 connection(s)
```

The condition resolves both tweaks, takes the correct link (output 2, "unrelated"), pulses it — **and
that link has no connections in the data.** `ops ever activated per level` reads
`l_tower_p 65/1670, L_Tower_Script 47/1629`, **exactly** what agent FI measured on the wave-18 build that
had no `CompareTweaks` body at all. The body is right, retail does the same thing, and the chain was
never alive to kill. **The "an unported condition kills its chain silently" rule is true in general and
false for this one on this route**, and the next package should not spend the other eight on the strength
of it without the same check — `-fkdiag`'s per-activation link log exists for exactly that.

## 9. What is left, specified

In the order the measurements support, which is **not** the order the brief gave:

1. **What activates `SOIREE_Empress_Regent.SeqAct_Interp_12`.** It is `activations 0` across a whole
   new-game route. The `SOIREE_Empress_Regent` sub-sequence is entered (FI measured that) and the
   matinee inside it is not reached. Walk its input links back with `-diskismet`'s inventory. This is now
   the only thing between the user and the opening cutscene that is *known* to be missing.
2. **`UInterpGroupInstAI::InitGroupInst` and a spawner group actor.** The AI groups' object variables
   resolve to `ADishonoredSpawner`, not to pawns (`FK5.log:5246`:
   `Skipping instancing group - an Actor may only be connected to one Group! [DishonoredSpawner_12]`,
   and the `BeginAnimControl` error named the spawner). With §4's fix the spawner no longer crashes the
   process, but an anim-control track on a spawner animates nothing. Retail's
   `UInterpGroupInstAI::InitGroupInst` and `ADishonoredNPCPawn::MAT_BeginAIGroup` (2012 `0x7c0570`) are
   the pair to read; this tree's `UInterpGroupInstAI::InitGroupInst` already reaches
   `eventMAT_BeginAIGroup` through `GetPawn(InGroupActor)`, so the question is whether retail swaps
   `GroupActor` for the pawn before chaining to the base. **Read it; do not guess.**
3. **The `UpdateTrackInst` seam** (§6): one virtual on `UInterpTrackInst`, one call in
   `UInterpGroup::UpdateGroup`'s track loop, then `StretchAnimControl` `0x543f40` (267 instances),
   `Dialog` `0x8ec050` (45), `FaceTo` `0x53eb90` (53), `Locomotion` `0x542630` (42); plus
   `UInterpTrackLookAt::StaticUpdateTracks` `0x547010` (99) called once per `UInterpGroupInstAI`, and the
   three that already have a seam: `AIControl` `0x53e120`, `Attachment` `0x53e560`,
   `DisAllowPlayerLook` `0x8dc4a0`. **Measure the frame cost**: this adds a per-track virtual call to
   every matinee in the game.
4. **`AActor::MAT_SetPhysics` and `UInterpGroupInst::BackupCollisionType`.** Retail's `InitGroupInst`
   backs up `CollisionType` and calls `MAT_SetPhysics( FALSE, BackupCollisionType )` when the group has
   `bBecomeDynamic`; `MAT_SetPhysics` does not exist in this tree. `BackupCollisionType` is a reflected
   member of `UInterpGroupInst` that nothing writes.
5. **`AActor::MAT_BeginAnimControl` / `MAT_FinishAnimControl` as `AActor` virtuals**, which is retail's
   shape, replacing §4's script-first fallback.
6. **The `MAT_*` contract** (§7): 22 + 8 + 5 bodies, and the player five need
   `UStatePlayerMasterSoiree` (`stateplayermastersoiree.cpp`, excluded),
   `UStatePlayerMasterInDialog::StartSoireeControl`, `ADishonoredPlayerPawn::m_pMatineeBlender` and
   `UInterpGroup::HasMoveTrack` first. That is a player-FSM package, not a matinee one.
7. **`UInterpGroup::NeedWaitToStart` and `USeqAct_Interp::NeedWaitToStart`** (2012 `0x230400`, 433 B).
   `UInterpGroupPlayer::NeedWaitToStart` is ported here and **cannot be reached**: the base virtual does
   not exist on `UInterpGroup` and nothing calls the `USeqAct_Interp` one. It is what makes retail hold a
   matinee until the player has put his hands down.
8. **The eight remaining conditions** and the ten leaf helpers they are blocked on (§8), with `-fkdiag`'s
   link log used first to see whether each one's chain is alive at all. **The two `UDisSeqCond_DLC05_*`**
   need new units; DLC, no mission path.
9. **`InitInterp`'s three retail differences** (§5): the `UBOOL`/`CanPlay` return,
   `FilterObjectDuplicate`, and the nav-list bracket.
10. **An intermittent assertion on the front end's own `PrepareMapChange`.** `UnObj.cpp:7979`,
    `check(!Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading))` in `UObject::StaticAllocateObject`'s
    replace branch, 0.02 s after `-gfxuimenu` opens `UI_Global.Global`. Seen in **2 of 10** runs
    (`FK2.log:838` at 33.11 s, `FK6.log:1078` at 18.05 s) and not in the other 8 with the same binary.
    Not attributable to this package — its only start-up addition sets a function pointer — and not
    diagnosed. The stack walker printed UTF-16 text again (STATUS defect 7). Worth owning by whoever
    next looks at the front end.

## 10. Main at `cd5fadd` does not pass its own gate, and it is not this package

`run_regression.py` from this worktree's copy, built inside the harness, absolute build dir, FM's new
total of 47 — and one check fails:

```
FAIL inputtest_criticals      1 (wanted == 0)
```

The crash is in the **animation blend tree**, not in the matinee, and it is preceded by the same two
lines every time:

```
FKR_inputtest.log:3759  [0042.49]  StartSpawn DisSeqAct_StartSpawn_0: 1 spawner(s) asked,
                                   spawnEvenIfVisible 1, latent actors 1
FKR_inputtest.log:3760  [0042.51]  Warning: Skipping instancing group - an Actor may only be connected
                                   to one Group! [DishonoredSpawner_11]
FKR_inputtest.log:3761  [0042.64]  === Critical error: ===  Fatal error!
```

**Three measurements, in this order, on the same machine and the same stage:**

| | `inputtest_criticals` |
|---|---|
| this package | 1 |
| this package with `-fknoplayergroup` (the player-group branch off) | 1 |
| **a clean checkout of `cd5fadd` with none of this package's changes, rebuilt from scratch** | **1** |

The third row is the one that matters: `git stash push -u`, a full rebuild into
`build/agentFK_base`, the same stage, and the failure is identical — same world time (38.2 s), same
preceding two log lines, same log line number (3761). **So `inputtest_criticals` is already failing in
main and this package neither caused it nor fixes it.** Every other check passes; §13 has the totals.

The coordinator has since recorded the same red gate in `STATUS.md` (`c40952a`, "The gate is RED,
deliberately") with a better decode than this package got: **`UArkComponentContainer::AddNewComponent_Internal+0x2d`**.
Take that over anything here — my own map-file lookup put the frames in the
`UAnimNodeBlendPerBone` / `UAnimNode_MultiBlendPerBone::GetBoneAtoms` family, but the module was not
loaded at its link base in either run, so **those names are a hint and not a decode**. What this package
adds to the record is the control: **a clean rebuild of `cd5fadd` with none of these changes fails the
same check the same way**, so the crash is in main and not in any wave-19 package's unmerged work. The
trigger is a pawn spawned by `DisSeqAct_StartSpawn_0` one frame earlier, and the warning on the line
before it — `an Actor may only be connected to one Group! [DishonoredSpawner_11]` — is a matinee group
binding that spawner, which is the same spawner-as-group-actor problem as §9 item 2.

## 11. The runs

Every screenshot was taken in-process with `-apshotname=fk<N>` and claimed by globbing that tag; no
desktop or window capture was used, each run's log names the tag it wrote, and **every capture is
1600x900 read off the file** (`struct.unpack` of the BMP header, not assumed).

| run | what it is |
|---|---|
| FK1 | `-finewgame=22 -fkdiag -fediag -dishud`; the census of §3. `grep -c FocusLost` = **1**, so its frame timings are not used — the census is package data and not frame-dependent |
| FK2 | the `-finewgame` timing crash of §2: `appError` at 33.11 s, `LoadMap` had not begun until 27.6 s |
| FK3 | `-finewgame=50 -fkdiagat=95`; the census twice (57 s and 153 s); the cutscene `activations 0` in both |
| FK4 | `-diskismetforce` fired on the wrong world: `fired 0 ops after 30.9s`. The mission is streamed into `DishonoredGameFull_P`'s world by `CommitMapChange`, so `GWorld` never changes, the census's `Reset()` never re-arms `bForceDone`, and the force delay is measured from the **first** world's start. A delay past the map change (78) works |
| FK5 | the cutscene (`SeqAct_Interp_12`) forced: the `BeginAnimControl` `appError` on `DishonoredSpawner_12` and the `CompareTweaks` link log |
| FK6 | `SeqAct_Interp_26` forced: the same `appError` on `DishonoredPlayerPawn` — the player group's own half of §4 |
| FK8 | after the `DisMatBeginAnimControl` edit: `RestoreAnimSetsToDefault` |
| FK9 | after the first two `APawn` guards: `BuildScriptAnimSetList` |
| **FK10** | after all four edits: **no appError**, `InterpGroupInstPlayer` on `DishonoredPlayerPawn`, `playing 1 pos 2.53`, run alive to 137 s |
| **FK11** | the same binary, same force, **`-fknoplayergroup`**: `InterpGroupInst`, actor `none`. The control |
| **FK12** | FK10 plus the `fkdiag blend` line: the pawn moved 595.8 units towards the mark |

`build/agentFK/` holds `logs/FK{1,2,3,4,5,6,8,9,10,11,12}.log` plus `FKR_inputtest.log`,
`FKR_newgame.log` and `FKB_inputtest.log` (the clean-`cd5fadd` control), `shots/fk*.bmp`, `fk_run.ps1`,
`fk_clean_build.cmd`, `fk_rva_sweep.log`, `fk_regression.log`, `fk_clean_build.log` and `idb/` (this
package's copies of the 2013 and 2012 databases).

## 12. Addresses, resolved by hand

`rva_sweep.py` from the worktree: **7,496 citations, 3 suspects, none in a file this package touched**
(`disbehaviorpatrol.cpp:319` and `gfxuirenderer.cpp:1679` twice, all three pre-existing and identical to
what agents FE and FI reported). `build/agentFK/fk_rva_sweep.log`.

Every address cited was **also** resolved by hand, and the method is stated per address:

* The nine `UInterpGroupPlayer` / `UInterpGroupInstPlayer` functions: named in **both** databases —
  `functions_2013.csv` for 2013, the Shipping PDB (`pdb_functions.csv`) for 2012 — at **identical
  sizes**: 66/5/5/50/581/607/66/653/309.
* `UDisSeqCond_CompareTweaks::Activated` `0x78eaf0` and `ExtractTweak` `0x78ea60`: named in the 2013
  database at 296 and 142 bytes, identical to the 2012 PDB's 296 and 142.
* `UInterpGroupInst::InitGroupInst` 2012 `0x231210` (424 B) and `UInterpGroup::HasAnimControlTrack` 2012
  `0x230f40`: PDB-named, and the body is confirmed by the five offsets it reads
  (`this+108` bit 0, `GroupActor+288` bit `0x8` = `bDeleteMe`, `Group+108` bit `0x20` = `bBecomeDynamic`,
  `Track+100` = `TrackInstClass`, `CollisionType`).
* `UDisSeqCond_IsSentinel::Activated` `0x78f070` is used as the mapping's control: `disseqcond_issentinel.cpp`
  already carries that number from agent EL and the 2012 PDB puts the body at `0x7d1fd0`.
* `UDisSeqCond_CompareBoolExtended` `0x79b3e0`, `PawnIsPossessed` `0x79b5f0`, `HasObjective` `0x78ec20`:
  function starts of 521, 403 and 140 bytes, **unnamed** in the 2013 database (`sub_B9B3E0`, `sub_B9B5F0`,
  `sub_B8EC20`) and matched only by `match_2012_2013.csv`'s "neighbours" method at 0.838, 0.748, 0.865
  with **different sizes** in the two builds. Flagged as not independently pinned; §8.
* All other rvas here (`MAT_*`, the track entry points, the condition helpers, `InitInterp` `0x25ad70`,
  `CanPlay` `0x24db10`, `FilterObjectDuplicate` `0x23a680`, `NeedWaitToStart` `0x230400`) are quoted as
  the **2012** rvas the Shipping PDB names them at, and marked as such in every table. **Remember
  `+ 0x400000`**: the 2013 database's own addresses are VAs and every 2013 rva here is `VA - 0x400000`,
  checked against `functions_2013.csv`'s `va,rva` pair. Agent FL reports `rva_sweep.py` passing eight VAs
  silently when they sat beside their own RVAs; nothing in this package writes a VA, and the by-hand pass
  above is what would have caught it.
* The member offsets each body is pinned by — `APawn::Controller` 620, `AActor::Rotation` 208,
  `AActor::Location` 196, `AActor::LatentActions` 572, the `AActor` flag word 300 (`m_bHasTerminated`
  `0x8`, `m_bLegalToTerminate` `0x10`), `AActor` flag word 288 (`bDeleteMe` `0x8`), `APawn::Mesh` 988,
  `APawn::CylinderComponent` 992, `UCylinderComponent::CollisionHeight` 452, `AController::Pawn` 584,
  `USeqAct_Interp::LatentActors` 248, `USeqAct_Interp` bit 17 at 400 = `m_bForcesUndrawBeforePlaying`,
  `UInterpGroup::bIsFolder` 108 bit 2, `FSeqOpOutputLink` stride 56 with `bHasImpulse` at `+24` — come
  from `resources/docs/types/types.json` (the 2012 PDB) and each is the offset the disassembly reads.
* The `ADishonoredPlayerPawn` vtable slots the group player dispatches through — 295
  `MAT_BeginPlayerGroup`, 296 `MAT_FinishPlayerGroup`, 301 `MAT_BlendOut`, 302 `MAT_IsAllowed`, 305
  `MAT_IsUnequipped`, 306 `MAT_SetForceInventory` — are `vtables.csv`'s `ADishonoredPlayerPawn{for AActor}`
  rows, cross-checked against the raw offsets the bodies use (`0x49C`/4 = 295, `0x4C4`/4 = 305,
  `0x4C8`/4 = 306). **`vtables.csv` is the 2012 table** and only the function set is taken from it; no
  slot number appears in generated code.

## 13. Merging, and the acceptance numbers

* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` was NOT run**, and did
  not need to be: no reflected member was added or moved and no class was added. The four edits inside
  generated headers are the `#include "CppText/...h"` mechanism agents EL, FE and FI used in the same
  header family — two in `DishonoredGameEngineShims.h` (`UInterpGroupPlayer`, `UInterpGroupInstPlayer`)
  and two in `dishonoredgamekismetclasses.h` (`UDisSeqCond_CompareTweaks`,
  `UDisSeqCond_CompareBoolExtended`). The `GDishonoredFKLinkAnchors` block in
  `DishonoredGameRegistrants.cpp` **is** the kind of edit a regeneration drops; flagged in
  `build/agentFK_sync.py`.
* **`Sources.cmake` loses two units**: `Src/disseqcond_compareboolextended.cpp` and
  `Src/disseqcond_comparetweaks.cpp`. Both carry an **external-linkage anchor**
  (`DishonoredLinkDisSeqCondCompareTweaks`, `DishonoredLinkDisSeqCondCompareBoolExtended`) referenced
  from `DishonoredGameRegistrants.cpp`, because `IMPLEMENT_CLASS`'s vtable is a folded COMDAT and MSVC
  drops a static-library member nothing undefined refers to — agent FI measured exactly that. Checked in
  the map file: `?Activated@UDisSeqCond_CompareTweaks@@UAEXXZ`,
  `?Activated@UDisSeqCond_CompareBoolExtended@@UAEXXZ`, `?InitGroupInst@UInterpGroupInstPlayer@@…`,
  `?BlendPlayerToMark@UInterpGroupInstPlayer@@…` and `?UpdateGroup@UInterpGroupPlayer@@…` are all in
  `DishonoredGame.map`, and the behaviour was then confirmed in a run.
* **No member was added to any class.** The `fkdiag blend` line's once-per-instance flag is a file-static
  `TArray` in `interpgroupplayer.cpp`, not a member: `UInterpGroupInstPlayer` is asserted at 152 bytes in
  `DishonoredGameLayouts.h` and a diagnostic is not worth a layout change.
* `build/agentFK_sync.py` is the authoritative copy list, worktree → main, and **names every function per
  file**. It flags rather than copies anything main changed since `cd5fadd`.

### Acceptance

| | |
|---|---|
| `run_regression.py` | **46 ok, 1 failed, 0 skipped**, 1,251 s. The worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentFK_reg`, built inside the harness (no `--no-build`), the five gitignored `resources/docs/types/` inputs already present. The one failure is `inputtest_criticals` and **§10 proves it pre-existing with a clean rebuild of `cd5fadd`**. FM's new `newgame` and `options` stages both pass. `build/agentFK/fk_regression.log` |
| clean full Release build | directory deleted first, `-DDISHONORED_LAYOUT_CHECKS=ON`, via `build/agentFK/fk_clean_build.cmd`: **0 errors, 0 C4263, 0 C4264**, 1,023 targets, `DishonoredGame.exe` linked at 24,072,704 bytes and `LayoutProbe.exe` built and run. The only `C42xx` warnings in the whole log are the pre-existing C4100/C4127/C4191/C4244/C4267/C4316/C4335/C4644/C4828/C4838/C4996 set. `build/agentFK/fk_clean_build.log` |
| `rva_sweep.py` | 7,496 citations, 3 suspects, none in a file this package touched; every cited address resolved by hand in §12 |
| `build/agentFK_sync.py` | 19 files, 8 new, **0 flagged** against main at `cd5fadd` |
| before/after | §1's table: one binary, `-fknoplayergroup` as the control, and §10's clean-`cd5fadd` rebuild for the one failing check |
