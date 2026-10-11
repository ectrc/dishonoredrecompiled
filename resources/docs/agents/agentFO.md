# Agent FO — what activates the opening cutscene: the conversation system, by GUID, and none of it is in this tree

Wave 20 (`PHASE16.md`), package FO: the cutscene's activation, then the `UpdateTrackInst` seam, then the
`MAT_*` contract. Worktree `build/agentFO_wt`, base commit `15e8a10`, with main's `b759fd5` (the
`-inputcmd` / `game_input.py` change) merged in by `patch -p1` before the second game run, as the
coordinator asked. Two source files touched, both in `DishonoredGame`: one new (`fokismetupstream.cpp`)
and one line added to `fkmatineediag.cpp`. **Nothing in `UnInterpolation.cpp`, nothing in the
`UInterpGroupInst*` family, nothing in any header** — FN owns `UnInterpolation.cpp` this wave and
this package did not need it.

## 1. The headline

**`SOIREE_Empress_Regent.SeqAct_Interp_12` has no Kismet input at all, and never had one.** Measured
in the running game with a tracer that walks every output link of every op in all nine levels and
then serializes every object in memory looking for a pointer to it (`-foupstream`, §3):

```
FO2.log [0058.49]  foupstream target ...SOIREE_Empress_Regent.SeqAct_Interp_12 (SeqAct_Interp) activations 0
                   input 0 'Play' impulse 0 queued 0 linkedOp none      (and Reverse, Stop, Pause, Change Dir: the same)
                   output 0 'Completed' -> SeqAct_ActivateRemoteEvent_4 ... output 20 'Slomo_Stop' -> SeqAct_ActivateRemoteEvent_240
                   var 2 'Empress' : DisSeqVar_SpawnerPawn_67 = DishonoredSpawner_12 ... (33 variable links)
                   referenced by L_Tower_Script...Main_Sequence.SOIREE_Empress_Regent (Sequence) x1 via SequenceObjects
                   1 referencer(s) of SeqAct_Interp_12
                   depth 1: SeqAct_Interp_12 has NO feeder
```

Its five inputs are connected to nothing; the only object in the whole process that points at it is its
owning sequence's `SequenceObjects` array. Twenty of its twenty-one outputs are wired (to remote events
that run the splatter FX, the post-process, the slow-motion, Daud's and the assassin's show/hide) — so the
level's author expected it to be *started by something that is not a link*. **That something is the
conversation system**, and it is on the exclude list end to end:

```
UDisSeqAct_DialogInputs::Activated         2013 0x79bf00 (578 B; 2012 0x7d75c0, 578)   Kismet -> for each Target speaker:
  IDisConvSpeakerInterface::GetHookParameters -> m_KismetDialogHook = <this input link's name>
  speaker vtable slot 8 ( hook DDH_Kismet = 10, initiator )                               = IDisConvSpeakerInterface::FireDialogHook
IDisConvSpeakerInterface::FireDialogHook   2013 0x8a7360 (41; 2012 0x8f9540, 41)       -> UDisConversationComponent::FireDialogHook
UDisConversationComponent::FireDialogHook  2013 0x8a6bd0* (908; 2012 0x8f52e0, 804)
UDisConversationComponent::FireDialogHookNow 2013 0x8a5670* (630; 2012 0x8dea10, 408)  -> one-shot trees, generic trees, m_pGenericDialogTree:
UDisDialogTree_InGameBind::FireDialogHook  2013 0x8a5150 (602; 2012 0x9145a0, 602)     -> EvaluateDialogHook 0x8a4690 (479) picks the conversation
FDisDialogRunningInstance::StartInstance   2013 0x88bda0 (84; 2012 0x8fae70, 84)
FDisDialogRunningInstance::StartDialog     2013 0x897100* (178; 2012 0x900b20, 166)    -> the tree's nodes play in turn (UDisConv_Node::PlayNode, 14 B)
UDisConv_Soiree::PlayNode_Derived          2013 0x8a7a60 (91; 2012 0x8f8ab0, 91)       -> PlayNodeMatinee (2012 0x904c10, 245; unmatched in 2013)
UDisConv_Soiree::FindMatinee               2013 0x8b5de0 (328; 2012 0x900d80, 328)     TObjectIterator<USeqAct_Interp>: Interp->m_MatineeGUID (@472) == node->m_SoireeGUID
USeqAct_Interp::StartFromDialog            2013 0x215020 (77; 2012 0x22d1b0, 77)       m_pDialogTree_RunningInst = inst; m_ConversationNodePointer = node;
                                                                                        bit 16 = bBlocking; ForceActivateInput(0); Position = 0
```

(`*` = 2013 address from `match_2012_2013.csv`'s "callee"/"neighbours" method, not independently named;
the others are named in `functions_2013.csv` at the size shown.)

So in retail the Empress assassination is **a node in a dialog tree**: a `UDisConv_Soiree` node whose
`m_SoireeGUID` is the matinee's `USeqAct_Interp::m_MatineeGUID` (retail SDK @472, declared here at
`EngineSequenceClasses.h:2134`; the sibling `FindMatineeLinked`, 2013 `0x8b5ed0`-family, matches the node's
`m_NodeGUID` against `FRuntimeMatineeData::m_ConversationNodeGUID` on the matinee *data* instead), reached
through `FindMatinee`'s object iterator and started with `StartFromDialog`, which is a `ForceActivateInput(0)` after stamping the
running dialog instance onto the matinee. **The Kismet half of the chain is alive on our route** —
`-diskismet` counts `DisSeqAct_DialogInputs x1` executed on every new-game run (FO2, FK10) — and the
first thing it calls, `IDisConvSpeakerInterface::GetHookParameters`, does not exist in this tree.
Neither does anything after it: `UDisSeqAct_DialogInputs::Activated` is a comment-only skeleton,
`disconversationcomponent.cpp` has 0 bodies, `FDisDialogRunningInstance` has 0, `UDisConv_Soiree` has 0,
`USeqAct_Interp::StartFromDialog` is not declared. **66 units** matching `disconv*`, `disdialog*`,
`disconversation*`, `disseqact_dialoginputs`, `disseqevent_dialogoutputs` are on
`DishonoredGame/Sources.cmake`'s exclude list; three conversation units in the whole module have a body
(`disconv_blurb.cpp`, `disconv_playerchoice.cpp`, `disdialogtree.cpp`, one function each).

Retail's conversation runtime by size, non-boilerplate functions only (2012 PDB):

| class | functions | bytes |
|---|---|---|
| `UDisDialogTree` | 13 | 21,335 |
| `UDisConversationComponent` | 42 | 10,335 |
| `UDisConv_Blurb` | 14 | 6,924 |
| `UDisDialogTree_InGameBind` | 17 | 4,881 |
| `UDisConv_Soiree` | 16 | 4,314 |
| `UDisConversation_InGameData` | 13 | 4,105 |
| `UDisConversation` | 9 | 2,342 |
| `FDisDialogRunningInstance` | 11 | 1,649 |
| `UDisSeqEvent_DialogOutputs` | 5 | 1,502 |
| `FDisSoireeControlState` | 3 | 961 |
| `UDisSeqAct_DialogInputs` | 4 | 914 |
| `UDisConv_Node` | 21 | 707 |
| the rest (`*_InGameData`, `UDisConv_Action`) | 11 | 432 |
| **total** | **179** | **~60 KB** |

That is the port that stands between the user and the opening cutscene, and it is a **dialog-system
package**, not a matinee one and not a Kismet-condition one. §9 specifies the minimum slice of it.

## 2. Premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "Trace the Kismet upstream of it; an unported `USequenceCondition` kills a chain silently" | **There is no Kismet upstream.** Five inputs, zero links, one referencer. No condition is on a chain to it because there is no chain. The eight blocked `UDisSeqCond_*` of FK §8 are irrelevant to *this* cutscene |
| "this is the only *known* blocker to the user-visible cutscene" | It was the only *named* one. Behind it: the conversation runtime (§1, ~60 KB), FN's `UInterpData` walk crash that fires on the first Visibility/Event/Toggle track of any shared-asset `MatineeData` (FN's finding; `Tower_EmpressRegent` is one), the `UpdateTrackInst` seam (§5), and `UArkAnimNodeStack::MAT_SetAnimPosition` (§6) before an AI pawn moves a bone |
| "`m_pMatineeBlender` and `UInterpGroup::HasMoveTrack` do not exist here" (FK §7, repeated in PHASE16) | **Both exist.** `ADishonoredPlayerPawn::m_pMatineeBlender` is declared in `dishonoredgameclasses.h:2481` and layout-asserted at 2428; `UInterpGroup::HasMoveTrack` is declared at `EngineInterpolationClasses.h:243` and defined at `UnInterpolation.cpp:3164`. What is missing for `MAT_BeginPlayerGroup` is `UStatePlayerMasterSoiree` (excluded) and `UStatePlayerMasterInDialog::StartSoireeControl` — the player FSM half, as FK said |
| "`UpdateTrack` ... only `AIControl`, `Attachment`, `DisAllowPlayerLook` use `UpdateTrack`" (FK §6) | Right, and **the seam is bigger than one virtual**: retail's `UInterpGroupInst::UpdateGroupInst` (2012 `0x25b4e0`, 858 B) runs **five update passes** keyed on `UInterpTrack::TrackUpdatePass` (`+105`, which this tree already declares), skips passes 2 and 3 of the normal loop while the matinee is in skip mode (flag `0x800` on `USeqAct_Interp+400`), and then **re-steps passes 2 and 3 in 0.1 s increments** with `FArkComponentLocomotion::MovePawn` between steps when skipping. Our `UInterpGroup::UpdateGroup` has two passes (FaceFX last). The seam is `UInterpTrackInst::UpdateTrackInst(FLOAT,UBOOL)` **plus the pass loop**; §5 |
| "FK's instruments ... `-finewgame` drives a new game" and the run scripts press Space | **The run scripts are retired.** `fo_run.ps1` (a copy of FK's) was used once (FO1) before the rule changed; every later run went through `game_input.py launch` with `-inputcmd`, and the one scripted key the route needs is none — `-finewgame=50` alone reached `l_tower_p` in FO2 and FO3 |
| FK §9 item 2: "whether retail swaps `GroupActor` for the pawn before chaining to the base. Read it; do not guess" | Read: `UInterpGroupInstAI::InitGroupInst` (2013 `0x2304f0`, 196 B) **does not swap**. It chains to the base only when `!GIsGame || !InGroup || InGroupActor`, ORs each track's `+120` bit 3 into its own flags, then `InitPawn` (`Cast<APawn>(GetGroupActor())` → `MAT_SetForceInventory`, `MAT_BeginAIGroup`, `GetBuildAnimSetList` with the Gesture/Posture/Distraction/Face filter). A NULL actor in game is left unbound, and `UInterpGroupInstAI::UpdateGroupInst` (2013 `0x2305c0`) binds it **lazily** every frame through `USeqAct_Interp::FindUnusedGroupLinkedVariable( GroupName )` — which is how a group whose spawner has not spawned yet picks up its pawn later. FN found the spawner-as-actor fault one level up (`UDisSeqVar_SpawnerPawn::GetObjectRef`); the two readings agree |

## 3. The instrument: `-foupstream`, `-foupstreamrefs`, `-fosoiree`

`DishonoredGame/Src/fokismetupstream.cpp`, hooked from `DisFKReportMatineeData` so it prints on each
`-fkdiag` pass (map arrival and `-fkdiagat`). Nothing when the switches are absent.

```
-foupstream=<substr>[,<substr>]   every USequenceOp whose path contains a substring: its activation count,
                                  bActive, pending input impulses; every input link (desc, impulse, queued,
                                  linkedOp), every output link and what it feeds, every variable link with the
                                  linked SeqVar_Objects' values; then breadth-first upstream to
                                  -foupstreamdepth (default 8): "X <- Y (class) activations N via output i 'desc'
                                  -> input j, delay, disabled, impulse" plus Y's gate/event/delay state
-foupstreamrefs                   also serialize every UObject through FArchiveFindCulprit and name each one
                                  that holds a pointer to the target, with the property
-fosoiree                         every UDisConv_Soiree in memory with its m_SoireeName / m_SoireeGUID and the
                                  USeqAct_Interp whose m_MatineeGUID matches; every matinee that carries a
                                  GUID no loaded node names
```

The walker crosses the three indirections a cooked sequence uses: a `SeqEvent_SequenceActivated` is fed
by whatever links into its owning sequence's matching input; a `SeqEvent_RemoteEvent` by every
`SeqAct_ActivateRemoteEvent` with the same name in any level; a sequence op's output by the
`SeqAct_FinishSequence` inside it. Three "targets matched" for one path is the same object reached
through three `GameSequences` entries (the pointer is printed to show it), not three objects.

## 4. The runs

Every capture in-process, `-apshotname=fo<N>`, globbed by tag, size read off the BMP header.

| run | how | what it measured |
|---|---|---|
| FO1 | `fo_run.ps1` (FK's script, **before** the input rule; retired since), `-finewgame=50 -fkdiag -fkdiagat=95 -foupstream=SOIREE_Empress_Regent.SeqAct_Interp_12 -diskismet`, shot at 120 s | `SeqAct_Interp_12 has NO feeder` at 57 s. `fo1_0.bmp` 1600x900, 4,320,054 bytes. `FocusLost` 1 (the old driver) |
| FO2 | `game_input.py launch` + `-inputcmd` (no real input), merged `b759fd5`, same switches plus `-foupstreamrefs`, shot at 184.6 s world time, `quit` scripted | §1's trace: no feeder, one referencer, all 33 variable links listed; identical at the 154 s second pass. `fo2_0.bmp` 1600x900, 4,320,054 bytes. `FocusLost` 1 — **under `-inputcmd`** (the window is never activated, so the one `FocusLost` is the engine's own startup line, not a stolen foreground); `inputcmd` lines at t=20.77 (`wait 165`), 184.64 (`shot`), 186.71 (`quit`) |
| FO3 | as FO2 with `-foupstream=DisSeqAct_DialogInputs -foupstreamdepth=1 -fosoiree` | §4a |

`run_regression.py`'s `newgame` stage still passes `-finewgame=22` in this worktree's copy (FN raises it
to 50); the hand runs here used 50, and `LoadMap` of `l_tower_p` began at 51–53 s in all three.

### 4a. FO3 — the data link, measured

`-fosoiree` at the 60 s pass (and again at the 157 s pass, identically):

```
fosoiree node L_Tower_Script...DisDialogOneShot_0.DisDialogTree_OneShot_0.DisConversation_8.DisConv_Soiree_1
              soiree 'None' guid AA4143A34799F2D8515A7F8B0AF4362C preview 0
              -> matinee L_Tower_Script...SOIREE_Empress_Regent.SeqAct_Interp_12 (activations 0)
fosoiree: 31 soiree node(s), 31 matched to a matinee, 60 matinee(s) with a guid and no node
```

**Every one of the 31 `UDisConv_Soiree` nodes loaded with `l_tower_p` names exactly one `USeqAct_Interp`
by GUID, and `SeqAct_Interp_12` is named by `DisDialogOneShot_0`'s tree, conversation 8.** The 31 cover
`SOIREE_Empress_Regent` ×13 (Interp 0, 4, 7, 9, 10, 11, 12, 13, 14, 16, 20, 26, 86 — including the two FK
forced), `SOIREE_Campbell_Sokolov` ×7, `SOIREE_Emily` ×6, `SOIREE_Guards` ×3, `SOIREE_Boat` ×1,
`SOIREE_Workers` ×1. So **the mission's whole scripted-scene layer is driven by `DisDialogOneShot` actors**
(ten of them in the level), not by Kismet; the 60 matinees with a GUID and no node are the main menu's,
`DishonoredGameFull_P`'s and the set dressing. `m_SoireeName` is `None` on all 31 — the GUID is the key,
the name is editor-only.

The Kismet half, with `-foupstream=DisSeqAct_DialogInputs -foupstreamdepth=1` (123 ops matched, all
nine levels): **exactly one has fired on the new-game route**, three times listed because its sequence is
reachable through three `GameSequences` entries:

```
foupstream target L_Tower_Script...Main_Sequence.SOIREE_Boat.DisSeqAct_DialogInputs_10 (DisSeqAct_DialogInputs) activations 1
   input 0 'Intro' ... linkedOp none            output 0 'Out' -> 0 link(s)
   var 0 'Target' prop Targets: SeqVar_Object_177 (= NULL)
   var 1 'DialogOneShot' prop m_pDialogOneShot: SeqVar_Object_57 (= DisDialogOneShot_8)
   depth 1: <- SOIREE_Boat.SeqEvent_RemoteEvent_38 (activations 0, triggers 1/0) via output 0 'Out' -> input 0
```

and `DisDialogOneShot_8`'s tree names **`SOIREE_Boat.SeqAct_Interp_1`** — the boat ride, the first scene
of the mission. In retail, `UDisSeqAct_DialogInputs::Activated` with hook `Intro` on that one-shot starts
the boat conversation, whose `UDisConv_Soiree` node plays `SOIREE_Boat.SeqAct_Interp_1`, and the
conversation layer walks from there to the Empress (`DisDialogOneShot_0`) — through
`UDisSeqEvent_DialogOutputs` (57 in the level, Kismet listening to the dialog), `DisConv_KismetActivateRemoteEvent`
nodes and the rest of the tree, none of which this package traced. **Here, the op activates, its native
`Activated` is a skeleton, `USequenceAction::Activated` passes the impulse through, and the whole scene
layer of the mission stays dark.** That is the measured first link of the chain and the measured last
link; what is between them is the ~60 KB of §1.

`FO3.log`: 12,842 lines, `FocusLost` 1 (startup, under `-inputcmd`), `inputcmd` at t=0.001 / 158.73 /
160.81, `L_Tower_Script 47/1629` ops ever activated (FK10 read 55 with the two forced matinees). `fo3_0.bmp`
1600x900, 4,320,054 bytes.

## 5. The `UpdateTrackInst` seam — read, specified, not landed

Retail's per-frame group update, `UInterpGroupInst::UpdateGroupInst( NewPosition, bPreview, bJump )`
(2012 `0x25b4e0`, 858 B; 2013 `0x22fed0`, 930 B by "callee" match), read in full:

```c
Seq = (USeqAct_Interp*)Outer;  bHasAnim = m_bHasAnimControlTrack;  Pawn = GetGroupPawn();  RootMotionMode = 0;
if( GIsEditor && bPreview && bHasAnim ) Group->UpdateAnimWeights( NewPosition, this, bPreview, bJump );
for( Pass = 0; Pass < 5; Pass++ )
    for( each TrInst )
        if( !Track->bDisableTrack && !(Track flags & 0x4000) && Track->TrackUpdatePass == Pass
         && ( !(Seq->flags & 0x800 /*skipping*/) || (Pass != 2 && Pass != 3) ) )
            TrInst->UpdateTrackInst( NewPosition, bJump );                 // vtable slot 74 (+296)
if( Seq->flags & 0x800 )            // skip mode: re-step passes 2 and 3 in 0.1 s increments, moving the pawn between steps
{
    Loco = FArkComponentLocomotion::GetLocomotionComponent( Pawn ); FaceTo = FArkComponentFaceTo::GetFaceToComponent( Pawn ); Loco->DisableInertia();
    for( Pos = Seq->Position + 0.1; Pos <= NewPosition; Pos += 0.1 ) { Seq->DeltaTime = step; for Pass in {2,3} UpdateTrackInst( Pos, FALSE ); if( Pawn && Pawn->Physics == PHYS_Walking ) { Loco->MovePawn( step, 0 ); Loco->PreAsyncWorkTick( step ); FaceTo->PreAsyncWorkTick( step ); } }
    Loco->EnableInertia();
}
if( bHasAnim ) { Group->UpdateAnimWeights( NewPosition, this, bPreview, bJump ); if( Actor ) Actor->MAT_SetRootMotionMode( RootMotionMode ); }
```

`UInterpTrackInst::UpdateTrackInst` (2013 `0x212570`, 36 B) is `Track->UpdateTrack( NewPosition, this,
bJump )`; `ConditionalUpdateTrackInst` (2013 `0x212af0`, 44 B) guards it with `!Track->bDisableTrack`.
So the base behaviour is exactly what `UInterpGroup::UpdateGroup` does today, and the Dishonored
instances override the instance-side virtual. **This tree has the group-side shape**
(`UInterpGroup::UpdateGroup( pos, GrInst, bPreview, bJump )`, two passes, FaceFX second); FK put the
player blend on it for that reason. The port is:

1. `virtual void UpdateTrackInst( FLOAT NewPosition, UBOOL bJump )` on `UInterpTrackInst`
   (`EngineInterpolationClasses.h:2131`, beside `NeedsSynchronizing`, which is the same kind of seam
   and already dispatches), base body `Track->UpdateTrack( NewPosition, this, bJump )`;
2. `UInterpGroup::UpdateGroup`'s loop becomes five passes on `Track->TrackUpdatePass` (the byte is at
   `UnInterpolation.h` `+105` already) and calls `TrInst->UpdateTrackInst` where it now calls
   `Track->ConditionalUpdateTrack` (preview keeps `ConditionalPreviewUpdateTrack`);
3. then the bodies, in the order their dependencies allow (all in the Engine package in this tree):

| body | 2012 | 2013 | bytes | depends on | state in this tree |
|---|---|---|---|---|---|
| `UInterpTrackInstStretchAnimControl::UpdateTrackInst` | `0x543f40` | unmatched | 512 | `GetAnimForTime` (`0x543a00`, 1285; 2013 `0x505740`), `ConditionallyReversePosition` (`0x540860`, 112), `UInterpTrackStretchAnimControl::{GetAnimName 0x540620, GetAnimStartOffset 0x5406a0, GetAnimCollisionStatus 0x540710, GetAnimRootMotionMode 0x540780, GetAnimMeshTranslationMode 0x5407f0}` (all 98–124 B, key-array reads: `AnimSeqs` stride 56, `Properties` at `+52`, `m_MatineeRootMotionMode` `+56`, `m_MeshTranslationMode` `+57`, `m_bDisableCollision` `+60` bit 0), `FindAnimSequenceFromName` (`0x540410`, 527), `GetMarkerFromNotify` (`0x5430b0`, 189), `CalcChannelIndex` (`0x549690`, 1070) from `InitTrackInst` (`0x549ac0`, 71), `UInterpGroupInst::SetActorRootMotion` (2013 `0x212e20`, 17), `UInterpGroupInstAI::SetActorCollision` (`0x212df0`, 23) / `SetMeshTranslationMode` (`0x212dc0`, 37), and the actor's `MAT_SetAnimPosition` | **self-contained**: every dependency is a key-array read or a 17–37 B setter. ~4.2 KB. This is the one to port first — 267 of 694 tracks, every player group |
| `UInterpTrackInstLocomotion::UpdateTrackInst` | `0x542630` | `0x503c30` | 1144 | `UpdateTrackInst_Jump` (2013 `0x500260`, 370), `ComputeLocoProp`, `UpdateStopObsoleteLocoRequest`, `StopCurrentLocoRequest`, `UInterpTrackLocomotionKeyProperties::FindKeyTargetActor`, `GetGameSpecificLocomotionModifier` — **none exist here**; `FArkComponentLocomotion::{StartLocoToActor, UpdateLocoToActor, StopLoco, SetModifier, GetLocomotionComponent}` **do** (`arkcomponentlocomotiongoto.cpp`) | five helpers to port first |
| `UInterpTrackInstFaceTo::UpdateTrackInst` | `0x53eb90` | `0x4ff8f0` | 592 | `FArkComponentFaceTo::{GetFaceToComponent, IsInitialized, StartFaceToActor, StopFaceTo}`, `FArkCpntFaceToActorProp::InitAllByDefaultExcept`, `OverwriteExactRotation` | `FArkComponentFaceTo` has **0 bodies** here (`Engine/Src/arkcomponentfaceto.cpp` is a skeleton) |
| `UInterpTrackInstDialog::UpdateTrackInst` | `0x8ec050` | `0x873750`* | 854 | `FDisDialogRunningInstance::GetInGameConv`, `UDisConv_Blurb::{PlayBlurb_Shared, TickNode_Shared}`, `UDisConversation_InGameData::GetSpeaker`, `GetAltBlurb` | the conversation runtime of §1 |
| `UInterpTrackLookAt::StaticUpdateTracks` | `0x547010` | `0x5082c0`* | 3756 | `FArkComponentLookat` (wave 21, deferred in PHASE16) | called once per `UInterpGroupInstAI` from `UpdateGroupInst` before the base |

**Not landed, deliberately.** Items 1 and 2 touch `UnInterpolation.cpp`, which FN owns this wave and is
changing in the same region (`UInterpGroup::GetMatineeData`, the Visibility/Event/Toggle `UpdateTrack`
callers, `InitInterp`); landing a five-pass rewrite of `UpdateGroup` beside FN's unmerged fix would be the
wave-18 collision again. And with §1 unported there is no cutscene to measure the seam on — every track
instance in `l_tower_p` that is *reachable* today is reference-engine (Move, Toggle, Visibility, Event),
and FK forced the two soirée matinees only through `-diskismetforce`.

## 6. The `MAT_*` contract — what was read, and the gap under it

The player five (2012 rvas from the PDB, bodies read):

| | 2012 | 2013 | B | body |
|---|---|---|---|---|
| `MAT_BeginPlayerGroup( UInterpGroupPlayer*, UInterpGroupInstPlayer* )` | `0x70fcc0` | `0x6ab570`* (296) | 257 | flags: bit 1 from `Group+124 & 4` (`m_bDisablePlayerCollision`), bit 2 from `UInterpGroup::HasMoveTrack`; `m_pMatineeBlender->SetEnable(1)`; `StopSneaking(0)`; `SetBodyMode( Group+124 & 8 )`; if the master FSM's logical state is `UStatePlayerMasterInDialog`: `StartSoireeControl( Matinee, m_fTimeToBlendToStageMark, Flags )`, else `RequestStateChange( FStatePlayerMasterSoiree_Param{ Matinee, blend, Flags } )` |
| `MAT_FinishPlayerGroup` | `0x7051f0` | `0x6ab6a0`* (100) | 82 | `SetBodyMode( BODY_MODE_ARMS_ONLY )`; `InDialog::StopSoireeControl(0)` or `Soiree::OnSoireeFinished()` |
| `MAT_BlendOut( UInterpGroupInst* )` | `0x705120` | `0x6ab4a0` | 193 | `GetBlendOutTime`; forwards to `InDialog::SetSoireeBlendOutTime` / `Soiree::SetSoireeBlendOutTime`; `m_pMatineeBlender->BlendNow( t, 1 )` unless the group is a `FolderOnly`/no-blend one, `t <= 1e-8`, no blender, or `Physics == 10`, in which case `SetEnable(0)` |
| `MAT_IsUnequipped` | `0x6fb030` | `0x6a1e80` | 39 | `HasNothingEquipped() && !IsHoldingMovableObject()` |
| `MAT_SetForceInventory( UBOOL )` | `0x6fe570` | `0x6a4d40` | 222 | unless the master FSM is in `UStatePlayerMasterInDialog`: `m_pInventory->SetReequipItem( Primary/Secondary, UDishonoredItemEmpty )`, drop a held movable, `EquipItemByType_KeepTrying( UDishonoredItemEmpty, Primary/Secondary )` |

`FDisSoireeControlState::EnterSoireeMode` (2012 `0x69a4b0`, 525 B; 2013 `0x666ff0`*, 591) is what the
Soiree state runs: `EnableCoreCamInfluence( UDishonoredCamera_AnimDriven, blend )`, `DisableArmFollow`
both hands, bump-smoother and physical-react camera influences to weight 0, `EnableControllerInput(
0x8002, 0, DisInputMaskLevel_Soiree )`, `DisPushDisableSave( DDST_Soiree )`, `StopSneaking`, collision
off and `setPhysics( PHYS_Interpolating=7 )` on flags 2/4, `CancelAllActivePowers`, drop a carried
corpse. **That is the player FSM package FK named**, and it is still the prerequisite: `UStatePlayerMasterSoiree`
(`stateplayermastersoiree.cpp`, excluded), `UStatePlayerMasterInDialog` (excluded) and
`FDisSoireeControlState` (no unit).

**The gap under all of it, which no brief has named:** retail's `APawn::MAT_SetAnimPosition` (2012
`0x2d0880`, 166 B; 2013 `0x2b53a0`*) walks `SlotNodes` and calls **`UArkAnimNodeStack::MAT_SetAnimPosition`**
(2012 `0x576570`, 536 B; 2013 `0x534cb0`*) directly — Dishonored's pawns have `UArkAnimNodeStack` slots,
not `UAnimNodeSlot`. This tree's `APawn::MAT_SetAnimPosition` (`UnPawn.cpp:3645`) is the reference
engine's and calls `UAnimNodeSlot::MAT_SetAnimPosition`; `UArkAnimNodeStack` here derives from
`UAnimNodeBlendBase` with one reflected bit and **zero bodies** (`arkanimnodestack.cpp`: `MAT_SetAnimPosition`
`0x576570`, `MAT_SetAnimWeights` `0x576790`/2013 `0x534f30` 277 B, `MAT_FinishAnimControl`
`0x5768b0`/2013 `0x535050` 102 B, all comment-only). Retail's stack body: child `ChannelIndex+1`, bit 2 of
the child's flags (`Children[i]+64 & 4`) gates it, `SetAnim` + `SetPosition( pos, FALSE )` when the
sequence or name changed, clears bit 0, root-bone axis/rotation options `2,2,2` or `0,0,0` on
`bEnableRootMotion`, `Rate = 1`, `bLooping` into bit 3, `bZeroRootTranslation`-style bit 4 from
`GIsGame && name != NAME_None`, then `SetPosition( pos, bFireNotifies )`. **Until this exists, a
StretchAnimControl track on an NPC or on the player animates nothing even with §5 landed**, because
`SlotNodes` is collected by class and will not contain the Ark stack. It is ~1 KB and belongs with §5.

`ADishonoredPawn`'s eight (2012): `MAT_SetAnimPosition` `0x791070` (chains `APawn::`, then
`StopRootMotionMode` when root motion was asked with `NAME_None`), `MAT_SetRootMotionMode` `0x7910e0`
(`StartRootMotionMode( RMM_Translate|RMM_Accel, RMRM_RotateActor )` / `StopRootMotionMode`),
`MAT_SetMeshTranslationMode` `0x791190` (2013 `0x74d520`; `MAT_DisableMeshTranslation( 0|1 )` or
`FArkComponentMeshOffset::EnableByFlags( 2 )`), `MAT_DisableMeshTranslation` `0x791130`,
`MAT_EnableMeshTranslation` `0x791170`, `MAT_RootMotionEnabled` `0x78e4d0`, `MAT_StopRootMotion`
`0x78e500`, `MAT_OnDestroyPreviewPawn` `0x79ed70`. `dishonoredpawn.cpp` is **built** (26 bodies) and
declares none of them; `FArkComponentMeshOffset` has 0 bodies. None was ported: nothing reachable calls
them until §5 lands, and the 22 NPC ones need the pawn FN is making the group actor.

## 7. `InitInterp`'s three retail differences, `CanPlay`, `NeedWaitToStart`

Read, not changed (FN's file):

* `USeqAct_Interp::InitInterp` (2012 `0x25ad70`, 1410 B; 2013 `0x2342e0`*, 1471) returns `UBOOL`:
  `TermInterp` if instanced; `InterpData = FindInterpDataFromVariable()`; **`if( !CanPlay() ) { InterpData = NULL; return FALSE; }`**;
  then per group — Director (one `InterpGroupInstDirector` per compatible `APlayerController`, else one with
  NULL), Player (`InterpGroupInstPlayer` on the local PC's pawn, FK's branch), else non-folder: `GetObjectVars`
  + `GetNamedObjVars` + **`FilterObjectDuplicate( ObjectVars )`** (FN is porting this), then **`for( i = 0; i < ObjectVars.Num() || i == 0; i++ )`** — an AI group with no object var still gets one `InterpGroupInstAI` with a NULL actor (which `UpdateGroupInst` binds lazily, §2) — `InterpGroupInstAI` for `UInterpGroupAI`, `InterpGroupInst` otherwise; after the loop
  `RemoveLevelNavList( CurrentLevel, 0 )` / `AddLevelNavList( CurrentLevel, 0 )`, `SetupCameraCuts()`, return `TRUE`.
* `USeqAct_Interp::Activated` (2013 `0x238f70`, 951 B): when `InitInterp` returns FALSE it pulses
  **output 2 ("Interrupted")** and sets flag `0x400000`; when TRUE it pulses output 4 ("Started"), plays /
  reverses / changes direction by the impulsed input, and calls `eventInterpolationStarted` on every
  linked actor that has a group instance.
* `USeqAct_Interp::CanPlay` (2012 `0x24db10`, 953 B; 2013 `0x22f420`*, 862): for every `UInterpGroupAI`,
  for every linked object var, the pawn (`m_ActorTypeFlags & 0x20`, or a controller's `Pawn`) must answer
  `MAT_IsAllowed( this, AIGroup, m_SoireeAIPriority byte at Seq+(1*…), bit, &Error )` and must not already
  be in `ListPawn` (one pawn, one group). A spawner is neither a pawn nor a controller and is **skipped**,
  so `CanPlay` alone does not refuse the Empress matinee. `ADishonoredNPCPawn::MAT_IsAllowed` is 2012
  `0x7c0c20` (549 B; 2013 `0x765560`*, 561).
* `USeqAct_Interp::NeedWaitToStart( FLOAT& NewPos, UBOOL& bJump, UBOOL& bNeedsBlending, UBOOL& bSync, UBOOL bPreview )`
  (2012 `0x230400`, 433 B; 2013 `0x2193e0`*, 441) is called from **`StepInterp`** (2013 `0x239700`, 654 B),
  first thing after clearing `BlendOutTimeOverride`; its result lands in `Seq+400` bit 18 and `bSync` in
  bit 23, and bit 18 set makes `StepInterp` call `UpdateInterp( Position, 0, 1, 0 )` instead of
  advancing. Inside: per group instance, `GrInst->vslot 79 ()` (`IsDisabled`) skips; if `Seq+400 & 0x20000`
  and `Position == 0` and `Group->NeedWaitToStart( GrInst )` (vslot 72 on `UInterpGroup`) → hold at
  `Position`; otherwise each track instance's `NeedsSynchronizing( Position, NewPos, Clamped, 0 )` (vslot 76,
  **the seam this tree already has**) may clamp `NewPos` and set `bJump` / `bNeedsBlending`.
  `UInterpGroupAI::NeedWaitToStart` (2012 `0x22e220`, 52 B) is `GetGroupActor() && GetGroupActor()->Base == NULL`
  — hold while the pawn is still falling in. FK's `UInterpGroupPlayer::NeedWaitToStart` stays unreachable
  until `UInterpGroup::NeedWaitToStart` exists as a virtual and this call is added to `StepInterp`.

## 8. FN coordination

* **`UInterpGroupInstAI::InitGroupInst`**: read (§2); not touched. FN's reading (no swap; the spawner
  came from `UDisSeqVar_SpawnerPawn::GetObjectRef`) is consistent with the decompile, and the lazy
  `FindUnusedGroupLinkedVariable` binding in `UpdateGroupInst` is the retail mechanism for a group whose
  pawn appears later.
* **`UnInterpolation.cpp`**: this package touched **no function in it**. FN's `GetMatineeData` /
  `FilterObjectDuplicate` / `InitInterp` work and §5's seam land in the same file; §5 is specified for the
  package that lands it after FN.
* FN's `UInterpData` walk crash would have been the next thing to fire had §1 been ported first: the
  Empress matinee's data is `Tower_EmpressRegent`, a shared asset, by FN's export-table read.

## 9. What is left, specified — the order the measurements support

1. **The conversation runtime, minimum slice to start a soirée.** In call order (§1): `UDisSeqAct_DialogInputs::Activated`
   + `GetDialogHookInitiator` (2013 `0x79bf00`, `0x790ac0`; 832 B); `IDisConvSpeakerInterface::GetHookParameters` /
   `_Derived` (7 + 31 B), `FireDialogHook` (41), `StartDialog` (43), `EndDialog` (32); `UDisConversationComponent::{FireDialogHook 804, FireDialogHookNow 408, CheckDialogHook 375, StartDialog 127, EndDialog 33, BeginPlay 124, PostSpawned 630}`;
   `UDisDialogTree_InGameBind::{FireDialogHook 602, EvaluateDialogHook 479, FireDialogHook_Helper 223}`;
   `FDisDialogRunningInstance::{StartInstance 84, StartDialog 166, GetRunningObjects 65, GetInGameNode 411, GetInGameConv 184, StopInstance 512}`;
   `UDisConv_Node::PlayNode` (14) and the node dispatch; `UDisConv_Soiree::{PlayNode_Derived 91, PlayNodeMatinee 245, FindMatinee 328, TickNodeMatinee 255, InterruptTickingNode, IsTickingNodeInterruptible}`;
   `USeqAct_Interp::StartFromDialog` (77) and `SetConversationNode`. Plus whatever `UDisDialogTree`'s
   21 KB does to build the in-game binding (`m_RunningInstances`, `m_NodeLookupMapByID`) — that is the
   part this package did not read, and it is the largest. ~10 KB on the direct path, ~60 KB for the class
   set. **The gate on it is FO3's `-fosoiree` line: a `UDisConv_Soiree` whose GUID is `SeqAct_Interp_12`'s
   must be in memory for this to be the route; see §4a.**
2. **FN's `UInterpData` walk** (FN's), before any soirée matinee is allowed to play.
3. **The `UpdateTrackInst` seam** (§5 items 1–2) and `StretchAnimControl` (§5's first row) **together with
   `UArkAnimNodeStack::MAT_SetAnimPosition / MAT_SetAnimWeights / MAT_FinishAnimControl`** (§6) — the
   three are one feature: without the stack body the seam moves nothing, without the seam the stack body
   is never called. Control: FK's `-fknoplayergroup` and `-diskismetforce=SeqAct_Interp_26` on one binary.
4. **The player FSM**: `UStatePlayerMasterSoiree`, `UStatePlayerMasterInDialog::StartSoireeControl /
   StopSoireeControl / SetSoireeBlendOutTime`, `FDisSoireeControlState::{EnterSoireeMode 525, ExitSoireeMode, TickSoireeMode}`,
   then the player five of §6 replacing FK's five `DisFKWarnOnce` sites in `interpgroupplayer.cpp`.
5. `ADishonoredPawn`'s eight and `ADishonoredNPCPawn`'s 22 `MAT_*` (§6), once FN's pawn is the group actor.
6. `UInterpGroup::NeedWaitToStart` as a virtual + `USeqAct_Interp::NeedWaitToStart` + its `StepInterp`
   call (§7); `InitInterp`'s `UBOOL`/`CanPlay` return and `Activated`'s Interrupted pulse (§7);
   `AActor::MAT_SetPhysics` / `BackupCollisionType` and `MAT_BeginAnimControl` on `AActor` (FK §9 items 4–5, unchanged).
7. `Locomotion`, `FaceTo`, `Dialog`, `LookAt` track bodies in that order (§5), each behind its component.

## 10. Addresses

Every 2012 rva above is the Shipping PDB's (`pdb_functions.csv`, imagebase 0, `shipping2012_v1.i64`),
read from a private copy of the database (`build/agentFO/idb/FO_shipping2012.i64`) through the headless
`resources/tools/ida/decompile_funcs.py`; the decompiles are in `build/agentFO/decomp2012{,b,c}/` with
the rva and size in each file's header. Every 2013 rva is `functions_2013.csv`'s `rva` column (VA −
`0x400000`) where the function is **named** there, at the size quoted; the ones marked `*` come from
`match_2012_2013.csv` by the method and ratio recorded in that file and are **not independently
pinned** — none of the port decisions here rests on one. No address in this package is written into
source except in `fokismetupstream.cpp`'s banner (`0x900d80`, the 2012 `FindMatinee`). `rva_sweep.py`
over the worktree: see §11.

## 11. Acceptance

| | |
|---|---|
| `run_regression.py` | **46 ok, 1 failed, 0 skipped**, 3,821 s (under three other agents' builds). The worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentFO_reg`, built inside the harness, `--newgame-at 50`, the five gitignored `resources/docs/types/` inputs copied in first. **The one failure is `inputtest_criticals 1 (wanted == 0)` — the known FN row** (STATUS "The gate is RED, deliberately"); every other row including `newgame_*` and `options_*` passes. `build/agentFO/fo_regression.log`, `build/agentFO_reg/regression/summary.txt` |
| clean full Release build | directory deleted first, `-DDISHONORED_LAYOUT_CHECKS=ON`, `build/agentFO/fo_clean_build.cmd` through `build_queued.py`: **0 errors, 0 C4263, 0 C4264**, 969 build steps, `DishonoredGame.exe` 24,116,736 bytes. `build/agentFO/fo_clean_build.log`. The `LayoutProbe` target is built separately (`fo_clean_probe.cmd`; a first attempt from a shell without `VsDevCmd` failed on `windows.h`, which is the shell and not the code) — see `fo_clean_layoutprobe.log` |
| `rva_sweep.py` | **7,548 citations, 3 suspects**, the same three pre-existing ones FK reported (`disbehaviorpatrol.cpp:319`, `gfxuirenderer.cpp:1679` twice), none in a file this package touched. `build/agentFO/fo_rva_sweep.log`. Every address in this report also resolved by hand, §10 |
| `build/agentFO_sync.py` | 4 files (2 source, 2 docs), **0 flagged** against main at `b759fd5`; names every function per file |
| headers | `gen_classes_header.py` **not run**: no reflected member added or moved, no class added, no generated header touched |
| before/after | §1/§4a are measurements of the untouched activation path (this package changes no behaviour); the one binary difference between FO1 and FO2/FO3 is main's `b759fd5` and the tracer. FK's `-diskismetforce` runs (FK5/FK10) remain the only times the cutscene has played here |
| captures | `fo1_0.bmp`, `fo2_0.bmp`, `fo3_0.bmp`: each 1600x900, 4,320,054 bytes, read off the BMP header; all taken in-process with `-apshotname=fo<N>` |
| the input rule | FO1 predates it and used FK's `fo_run.ps1` (retired, kept in `build/agentFO/` for the record only); FO2 and FO3 used `game_input.py launch` + `-inputcmd` with `wait / shot / quit` scripted and nothing else — `-finewgame=50` needs no key |
