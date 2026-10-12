# Agent FS — the five-pass track seam, StretchAnimControl, UArkAnimNodeStack and the AI pawn's MAT_ contract

Wave 21 (`PHASE17.md`), package FS. Worktree `build/agentFS_wt`, base `d7d083a`, with main's `e60c5c6`
(the fullscreen refusal) merged in by `patch -p1 -d` as the coordinator asked. Build `build/agentFS_rel`
(Release, `DISHONORED_LAYOUT_CHECKS=ON`, through `build_queued.py`); the gate's in-harness build is
`build/agentFS_reg`. Sync `build/agentFS_sync.py` (21 files, every function named). The shared control
is `build/head_rel` (HEAD `80cc219`), staged as `DishonoredGame_fsbase.exe`.

## 0. The answer in six lines

1. **The Boat scene's AI pawn animates.** On the shared control the spawned Elite guard was bound and
   nothing drove it; on this build its `MatineeStackNode` plays `BoatDriver_SC_Tower_Idle` at channel
   weight 1.00, the sequence time advancing with the matinee and looping (`FS5.log`:
   `ch0 w 1.00 BoatDriver_SC_Tower_Idle t 0.03 -> 1.05 -> 2.07 -> 0.21 ...` for 58 s, `AnimSeq found`).
   The set-dressing rats (`SkeletalMeshActorMAT`, the same 267-track family) animate the same way (`FS1.log`).
2. **It does not move.** `DishonoredNPCPawn_26` stays at `-2934/17434/-764`, `physics 1`, `vel 0`,
   **`base none`**, while the boat (`InterpActor_18`, the Move track) sails from `-2888/17150` to
   `-1854/23735`. The Elite group has one track (`StretchAnimControl`, 1 key, `rootMotion 0`): retail
   carries the guard by *basing* him on the boat, not by a track, and the spawned pawn here has no
   `Base`. That is the spawner's / basing's business (FI, FN), measured here, not fixed (§6).
3. **Three things stood between the seam and the pawn, none of them in the brief**: `APawn::SlotNodes`
   was `UAnimNodeSlot*` where retail's is `UArkAnimNodeStack*` (the array was empty on every Dishonored
   pawn); nothing called retail's native `PostInitAnimTree` / `UpdateNodeSlot`, so the slots were never
   cached; and **`UInterpTrack`'s bitfield was in the reference order while a native class keeps the
   cooked class's masks**, so `HasAnimControlTrack()` read `bPlayerGroupOnly` and every AI group's
   StretchAnimControl answered "no anim-control track" - no anim sets, no weights (§2).
4. Ported, in FO's order: the five-pass seam + `UInterpTrackInst::UpdateTrackInst` (§1.1); the whole
   StretchAnimControl runtime, 18 functions (§1.2); `UArkAnimNodeStack`'s five, moved into the Engine with
   retail's `SlotNodes` type (§1.3); `InitPawn`, `GetBuildAnimSetList`, `PostInitAnimTree`, the AI
   `TermGroupInst` / `UpdateGroupInst` halves and 16 pawn `MAT_*` bodies, `MAT_BeginAIGroup` /
   `MAT_FinishAIGroup` partial (§1.4). `Locomotion`, `FaceTo`, `Dialog` not reached (§6).
5. Gate: **47 ok, 0 failed, 0 skipped** (2,654 s), built inside the harness on this worktree; every `*_criticals` row 0; the gate build log has 0 errors, 0 C4263, 0 C4264 (§7).
6. **One run of this package went fullscreen on the user's display** (`FSBASE1`, the first control
   run, before the rule existed: `presented frame (1600x900, fullscreen)`). Every run after it passed
   `-windowed` (`fs_run.py` adds it unconditionally) and main's `e60c5c6` is merged into the worktree.

## 1. What was ported, in FO's order

### 1.1 The seam: five passes on `TrackUpdatePass`, and `UInterpTrackInst::UpdateTrackInst`

`UInterpGroup::UpdateGroup` (`Engine/Src/UnInterpolation.cpp`) is now retail's
`UInterpGroupInst::UpdateGroupInst` (2012 `0x25b4e0`, 858 B; 2013 `0x22fed0`, 930 B - `sub_62FED0`,
matched by callee at 0.671, read in full), carried on the group as this tree's seam has always been:

* five passes `0..4` on `UInterpTrack::TrackUpdatePass` (`ETrackUpdatePass`: SoireeControl 0, Generic 1,
  Locomotion 2, Attachment 3, FaceFx 4 - the 2012 PDB's `types.json`), skipping disabled and recording
  tracks (retail's `+120` masks `0x10` = `bDisableTrack`, `0x4000` = `bIsRecording`), calling the
  **instance's** virtual `UpdateTrackInst( NewPosition, bJump )` - new on `UInterpTrackInst`, beside
  `NeedsSynchronizing`; the base (2013 `0x212570`, 36 B) is `Track->ConditionalUpdateTrack`;
* while `USeqAct_Interp::m_bSkipping` (`+400` mask `0x800`), passes 2 and 3 are left out and re-stepped
  afterwards in 0.1 s increments from `Position` to `NewPosition` with `DeltaTime` set to the step, the pawn
  moved between steps through `GDisInterpSkipStepHook` (`UnWorld.h`; DishonoredGame installs
  `DisFSSkipStep` in `interpgroupplayer.cpp`: `DisableInertia` / `PreAsyncWorkTick` / `EnableInertia` on the
  locomotion component - `FArkComponentLocomotion::MovePawn` 2012 `0x584ca0` and `FArkComponentFaceTo` have
  no body here, so the pawn is not translated; the player's own pawn is left alone as in retail);
* `UpdateAnimWeights` **only when the group has an anim-control track**, then
  `Actor->MAT_SetRootMotionMode( m_CurrentRootMotionMode )` with what the keys collected.

`UInterpGroup::UpdateAnimWeights` gained retail's second half (2012 `0x256000`): every enabled
`StretchAnimControl` track weighs its slot with `GetWeightForTime`. `UInterpGroupInst` gained
`GetGroupPawn` (2012 `0x22a9c0`), `SetActorRootMotion` (`0x22ac60`) and the two empty virtuals
`SetActorCollision` / `SetMeshTranslationMode` that `UInterpGroupInstAI` fills (`0x22ac30`, `0x22e1f0`).
`UInterpTrackInst::Track` (Arkane's member at +56) is now written for every instance in
`UInterpGroupInst::InitGroupInst` and the base `InitTrackInst` - the reference instances never set it and
the first Move/Visibility instance to reach the seam read NULL (`FS1`'s crash, §4).

**2013 differences read and not ported** (`build/agentFS/decomp2013/va_62fed0.c`): the 2013 loop calls
vtable `+300` = `ConditionalUpdateTrackInst` (2012 called `+296` = `UpdateTrackInst`; same effect - the
condition is re-checked), and after the weights it reads the previous `m_CurrentRootMotionMode == 1` and
`UInterpGroupInst+108` bit 1 (`m_bShouldBackupTransform`) to call `sub_61FCB0( this )` - the 2013-only
transform backup for a root-motion key. The 2013 `UInterpTrackInst` vtable has two extra virtuals at
slots 73/74 (`sub_9016E0`, `sub_901760`; the latter asks the group for a vcall `+1228` when the track's
`+164` bit 0 - `m_bSpecialRootMotionExtract` - is set), so **the seam is slot 76 in 2013, not 74**.

### 1.2 `StretchAnimControl` - 267 of 694 tracks, every player group

`Engine/Src/interptrackstretchanimcontrol.cpp`, from the 2012 bodies (the Shipping PDB names every one;
2013 names all but two - see §5):

| | 2012 | 2013 | B |
|---|---|---|---|
| `UInterpTrackInstStretchAnimControl::UpdateTrackInst` | `0x543f40` | `0x505c70` (`sub_905C70`, by vtable slot 76) | 512 / **823** |
| `UInterpTrackInstStretchAnimControl::GetAnimForTime` | `0x543a00` | `0x505740` (`sub_905740`) | 1285 / 1269 |
| `UInterpTrackInstStretchAnimControl::ConditionallyReversePosition` | `0x540860` | `0x501550` | 112 |
| `UInterpTrackInstStretchAnimControl::InitTrackInst` | `0x549ac0` | - | 71 |
| `UInterpTrackStretchAnimControl::CalcChannelIndex` | `0x549690` | - | 1070 |
| `UInterpTrackStretchAnimControl::FindAnimSequenceFromName` | `0x540410` | - | 527 |
| `GetTimeRange`, `GetKeyframeTime`, `GetKeyframeLength`, `GetTrackEndTime` | `0x540170`, `0x5403a0`, `0x5431a0`, `0x543930` | `0x500e50`, `0x501080`, `0x504ce0`, `0x505670` | 177, 100, 343, 194 |
| `GetAnimName`, `GetAnimStartOffset`, `GetAnimCollisionStatus`, `GetAnimRootMotionMode`, `GetAnimMeshTranslationMode` | `0x540620`, `0x5406a0`, `0x540710`, `0x540780`, `0x5407f0` | `0x501310`, `0x501390`, ... | 98-124 |
| `GetMarkerFromNotify`, `GetWeightForTime` | `0x5430b0`, `0x543f10` | `0x500d90`, `0x505c40` | 189, 39 |

What the track does (nobody had written it down): a key names an animation, a start/end offset, a play
rate, `bReverse`, and **six soiree offsets** that move the animation's `AnimNotify_SoireeAccent / Apex /
Loop / End / In / Out` notifies in the stretched timeline. `GetAnimForTime` maps the matinee time onto the
animation piecewise-linearly so each mark lands on its moved time, and between the mark before
`SoireeEnd` and `SoireeEnd` it loops a whole number of times (count rounded from the stretched segment
over the original). The instance tracks the loop (`bAnimIsLooping`, `LoopStart/End`,
`OffsetForLoopWhileSyncing`) so a matinee holding its position - a SoireeControl pause, a dialog wait -
keeps the loop playing, re-sends the key's start offset once per key change, and every frame hands
`MAT_SetAnimPosition( SlotName, ChannelIndex, Anim, Pos, bFireNotifies=TRUE, bLooping=FALSE,
RootMotionMode != 0 )` to the actor while collecting the key's collision / root-motion /
mesh-translation choice on the group instance. `GetMarkerFromNotify` compares class **names** (the six
`UAnimNotify_Soiree*` classes are DishonoredGame shims an Engine unit cannot name).

**2013's `UpdateTrackInst` is 311 B bigger** (`decomp2013/va_905c70.c`): it detects reverse playback
(`bReversePlayback` vs. `LastUpdatePosition`), passes the previous animation position as an extra
`FLOAT` to `MAT_SetAnimPosition` (which is how `m_fCurrentAnimPosition` / `m_fBackupAnimPosition`, the two
members 2013 added to the instance, are used), only calls `SetActorRootMotion` when the animation moved
forward, and has a `m_bSpecialRootMotionExtract` path through `sub_612E40( GroupInst, ... )`. The 2013
`UArkAnimNodeStack::MAT_SetAnimPosition` (`sub_934CB0`, 629 B vs 536) takes that extra float and, when
`bFireNotifies` and the float is not `FLT_MIN`, advances the sequence by `InPosition - CurrentTime` through
vtable `+484` before `SetPosition` so the notifies between the two positions fire. **This port is the 2012
shape**; the 2013 refinements are specified for the next pass (§6).

### 1.3 `UArkAnimNodeStack` - the node retail actually animates

`Engine/Src/arkanimnodestack.cpp`, five bodies: `InitAnim` (2012 `0x5782c0`), `ForceDontUseRefPoseOnChild`
(`0x576aa0`), `MAT_SetAnimPosition` (`0x576570`), `MAT_SetAnimWeights` (`0x576790`), `MAT_FinishAnimControl`
(`0x5768b0`); 2013 `0x534cb0*`, `0x534f30`, `0x535050`, `0x535240`, `0x5371f0`. Read against this tree's
layouts: `FAnimBlendChild` stride 80 with `Anim` @8, `Weight` @12, `bMirrorSkeleton` @64 bit 0,
`m_bIsAnimSeq` bit 2 (**never written in this tree**, so the child test is `Cast<UAnimNodeSequence>`);
`UAnimNodeSequence::AnimSeqName` @204, `Rate` @212, flags @216 (`bLooping` 0x8, `bMatineePlaying` 0x10,
`m_bDontUseRefPoseOnNonDefinedBones` 0x20000), `CurrentTime` @220, `PreviousTime` @224, `AnimSeq` @228 -
every offset the decompile reads is a member this tree declares at that offset.

**And the class moved into the Engine.** `APawn::SlotNodes` and `ASkeletalMeshActorMAT::SlotNodes` are
`TArray<UArkAnimNodeStack*>` in retail (`retail_sdk_layout.json`, `Pawn` @1056, `SkeletalMeshActorMAT`
@644); this tree's generated headers said `UAnimNodeSlot*`, `APawn::CacheAnimNodes` collected
`UAnimNodeSlot`, and so **on a Dishonored pawn `SlotNodes` was empty and every `MAT_SetAnimPosition` went
nowhere** - the gap FO named in §6. Retail's `APawn::CacheAnimNodes` (2012 `0x2d6770`), `MAT_SetAnimPosition`
(`0x2d0880`), `MAT_SetAnimWeights` (`0x2d0930`), `MAT_FinishAnimControl` (`0x2d6940`) and
`ASkeletalMeshActorMAT::CacheAnimNodes` (`0x343ee0`) / `MAT_SetAnimPosition` (`0x33e340`) /
`MAT_SetAnimWeights` (`0x33e400`) all walk stacks; so the class is declared in `EngineAnimClasses.h`,
`IMPLEMENT_CLASS`'d in its own unit, removed from `DishonoredGameEngineShims.h` and
`DishonoredGameRegistrants.cpp` (the shim header's `AutoInitializeRegistrants` macro and
`VERIFY_CLASS_SIZE_NODIE` still name it and still compile), and `SlotNodes` has retail's type in both
generated headers. The slots are cached where retail caches them: `APawn::PostInitAnimTree` (2012
`0x2dd5c0`, 79 B; a native `AActor` virtual now called from `USkeletalMeshComponent::InitAnimTree` beside
the reference script event) and `ADishonoredNPCPawn::UpdateNodeSlot` (`0x7bac70`, 554 B: the stacks whose
`CanBeUsedByMatinee` is set, an unnamed one taking `MatineePreview/DefaultAnimSlotName`), the first thing
`MAT_BeginAIGroup` does. **Regenerating `DishonoredGameEngineShims.h` would put the class back and break
the build loudly (a redefinition)** - the generator has to be told.

**One deviation, flagged in the code**: retail's `MAT_SetAnimWeights` writes channels 1..N-1 and leaves
child 0 alone; this tree's `UAnimNodeBlendBase::GetBoneAtoms` is the reference weighted sum, normalised
only for `BlendType != ABT_Linear`, so child 0 is given `1 - sum(channels)` (what `UAnimNodeSlot` does).
`FS5.log` shows the stack at `0.00/1.00/0/0/0/0/0` during the scene. Whether retail's blend base handles
the stack's weights some other way is a dismod read of the live children weights during a scene (§6).

### 1.4 `UInterpGroupInstAI::InitPawn` and the NPC's `MAT_*`

`UInterpGroupInstAI::InitGroupInst` no longer carries FN's `FindFunction( MAT_BeginAIGroup )` guard: it
sets `bHaveMoveTrack` / `bHaveLocomotion` from the instanced tracks' `bIsMoveControlTrack` /
`bIsAIControlTrack` (retail's bits, which `UInterpTrack` now declares) and calls **`InitPawn()`** (2013
`0x22bbc0`, 373 B; 2012 `0x240e10`): `Pawn->MAT_SetForceInventory( Seq->m_bForcesUndrawBeforePlaying )`,
`Pawn->MAT_BeginAIGroup( this )`, then `Pawn->GetBuildAnimSetList( {Gesture, Posture, Distraction, Face},
GroupAnimSetsPawn )` (2012 `0x2dd380`: the mesh's anim sets whose name contains a filter - 8 on the Elite).
`TermGroupInst` is retail's (2012 `0x244840`): once, `GetGroupPawn()->MAT_FinishAIGroup( this )`,
`UpdatePhysics( FALSE )`, both anim-set lists emptied, the base, `bAlreadyTerminated`.
`UInterpGroupAI::UpdateGroup` wraps the base the way retail's `UInterpGroupInstAI::UpdateGroupInst` (2012
`0x25bfd0`) does: `bDisableCollision`, `bHaveLocomotion` and `m_MeshTranslationMode` reset before,
`Pawn->MAT_SetCollision( bDisableCollision, bHaveMoveTrack )` and `MAT_SetMeshTranslationMode` after (the
`UInterpTrackLookAt::StaticUpdateTracks` call retail makes there is FT's).

The contract is on `APawn` / `AActor` as retail has it (`EnginePawnClasses.h`, `EngineClasses.h`: empty
virtuals `GetBuildAnimSetList`, `PostInitAnimTree`, `MAT_SetForceInventory`, `MAT_BeginAIGroup`,
`MAT_FinishAIGroup`, `MAT_SetCollision`, `MAT_SetMeshTranslationMode` on `APawn`; `MAT_SetAnimPosition`,
`MAT_SetRootMotionMode`, `PostInitAnimTree` on `AActor`) and the Dishonored pawns override it:

| | 2012 | 2013 | B | state |
|---|---|---|---|---|
| `ADishonoredNPCPawn::MAT_BeginAIGroup` | `0x7c0570` | `0x765080` | 696 / 720 | **partial** - `UpdateNodeSlot`, FSM `RequestStateExit` on the pending and current master state, `m_iNumReactionDisableRequestsFromSoiree++` on `bDisableHitReact`, locomotion `DisableSteering` / `AllowToFall(TRUE)` / `SetModifier(0)`. Missing (helpers absent): `FDisComponentLODManager::StartLOD`, `FArkComponentMeshOffset::ForceInstantSnapMesh`, `m_pBlendPoseNode->BlendNow(0.2)` on empty `m_MatineeControllersInfos`, `MatineeControllerAdd`, the `FAIStimStruct_Soiree` ESE_Start stim to the brain, `bDisableJiggleBones -> Mesh->m_bDontUpdateKinematic`, `FArkComponentLookat::StartLookForward` on `LockRequests` (FT) |
| `ADishonoredNPCPawn::MAT_FinishAIGroup` | `0x7c0830` | - | 478 | **partial** - `MAT_SetCollision(0,0)`, `MAT_SetRootMotionMode(Disable)`, `MAT_EnableMeshTranslation`, `m_bOverrideReactionDisableFromSoiree = 0`, `SetBodyIntention( Soiree, None, 0, 0 )`, `EnableSteering` / `AllowToFall(FALSE)`, reaction count back, `MAT_StopRootMotion`, `m_bDisableCollisionForSoiree = 0`. Missing: `StopAllLookAtFromAsker`, `MAT_BlendOut`, `StopAllLODByAsker`, `MatineeControllerRemove`, the ESE_Stop stim, `ApplyLocomotionModifier` (-> `m_bLocoModifierNeedsApplication`) |
| `ADishonoredNPCPawn::UpdateNodeSlot` | `0x7bac70` | - | 554 | ported |
| `MAT_SetForceInventory`, `MAT_SetCollision`, `MAT_SetMeshTranslationMode`, `MAT_DisableMeshTranslation`, `MAT_IsCollisionDisable`, `MAT_PermitLODPause` | `0x7ab5c0`, `0x7ab7a0`, `0x7ab6e0`, `0x7ab720`, `0x7ab8a0`, `0x7ab710` | - | 30, 252, 46, 33, 13, 15 | ported; `MAT_SetCollision` without the `FDisComponentRetargeting` test (no such component). Bits pinned by the 2012 PDB: flag word @2180, bit 21 `m_bShouldForceBodyIntention`, bit 22 `m_bDisableCollisionForSoiree`, bit 15 `m_bOverrideReactionDisableFromSoiree`, bit 5 `m_bRenderedByPlayer` |
| `ADishonoredPawn::MAT_SetAnimPosition`, `MAT_SetRootMotionMode`, `MAT_SetMeshTranslationMode`, `MAT_DisableMeshTranslation`, `MAT_EnableMeshTranslation`, `MAT_RootMotionEnabled`, `MAT_StopRootMotion` | `0x791070`, `0x7910e0`, `0x791190`, `0x791130`, `0x791170`, `0x78e4d0`, `0x78e500` | `MAT_SetMeshTranslationMode` `0x74d520` | 103, 71, 81, 57, 25, 33, 10 | ported with two deviations: `Start/StopRootMotionMode` are `ChangeAnimStateMotionModes` on the anim-state machine (unported, `0x6fae70` / `0x78e550`), so the modes are written onto the mesh; the mesh-translation trio needs `FArkComponentMeshOffset` (0 bodies) and says so once (`FS5.log`: 1 line) |

Not ported, not reached by the Boat scene on this route: the other fourteen NPC `MAT_*`
(`MAT_BlendStop 0x7ab5b0`, `MAT_Attach 0x7ab5e0`, `MAT_Detach 0x7ab690`, `MAT_PausedAIGroup 0x7ad570`,
`MAT_ResumedAIGroup 0x7ad5c0`, `MAT_BlendOut 0x7ad5e0`, `MAT_IsUnequipped 0x7ad6b0`,
`MAT_ShouldForceInventory 0x7ad720`, `MAT_GetBodyIntention 0x7ad740`, `MAT_SendAimRequest 0x7ad790`,
`MAT_SendAimStopRequest 0x7ad810`, `MAT_SendFireRequest 0x7ad840`, `MAT_SendBodyIntentionRequest
0x7c0a10`, `MAT_IsAllowed 0x7c0c20` - `CanPlay` is still absent from `InitInterp`), `ADishonoredNPCPawn::
PostInitAnimTree` (`0x7ad200`, adds `FDisComponentAnimPlayer`, absent), and the player five (blocked on
`UStatePlayerMasterSoiree`, FO §6; FK's five `DisFKWarnOnce` sites stand).

## 2. Premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "`UArkAnimNodeStack` ... ~1 KB, belongs with the seam" | Right about the size and wrong about where the hole was. The stack's bodies were a third of it: **`APawn::SlotNodes` was the wrong type** (`UAnimNodeSlot*` where retail's SDK says `UArkAnimNodeStack*`, @1056), `APawn::CacheAnimNodes` collected `UAnimNodeSlot`, and nothing called the native `PostInitAnimTree` that caches the slots, so on a Dishonored pawn the array was empty and the stack's bodies would never have been called (`FS2.log`: `no stack`). The fix is in `EnginePawnClasses.h` / `EngineAnimClasses.h` / `EngineClasses.h` / `UnPawn.cpp` / `UnSkeletalMesh.cpp` / `UnSkeletalComponent.cpp` and moved the class into the Engine - a hand edit of three generated headers, which the generator must learn (§1.3) |
| the premise nobody wrote: "`UInterpTrack`'s bools are what their names say" | **The biggest finding, in nobody's brief.** `UnInterpolation.h` declared `UInterpTrack`'s bitfield in the reference order (`bDisableTrack` at `0x4`, `bIsAnimControlTrack` at `0x8`), but a native class keeps the bit masks **serialized from the cooked script class** (`UStruct::Link`, `SavedBitMask`, `Core/Src/UnClass.cpp`), and retail's are `0x10` and `0x20` (`retail_sdk_layout.json`, `+120`). So C++ `bDisableTrack` read retail's `bAIGroupOnly`, C++ `bIsAnimControlTrack` read `bPlayerGroupOnly`, and `HasAnimControlTrack()` was FALSE for every StretchAnimControl group that is not player-only: `fkdiag aipawn group 'Elite' has NO anim-control track` (`FS4.log`) - so `MAT_BeginAnimControl` never added the matinee's anim sets to the pawn, `UpdateAnimWeights` never ran, and the stack received `BoatDriver_SC_Tower_Idle` with `AnimSeq NULL` at weight 0. FK's "player group has an anim-control track" was the `bPlayerGroupOnly` bit. The bitfield is retail's order now, with the seven bits the reference did not have; it is the same DWORD and the layout check is unchanged. **Every other NOEXPORT class whose bools were declared in reference order has the same fault**; §6 |
| "the Boat matinee runs to the end with its spawned Elite guard as AI actor (FN)" and "Before/after ... through the same `game_input.py` script (`-finewgame=50 -fkdiag`)" | **On the `-finewgame=50` route the Boat matinee never starts**: `Main_Sequence.SeqAct_Interp_3 playing 0 activations 0` at 60 s and 155 s on the control (`FSBASE1.log`), and on the `-startmap=L_Tower_P -inputtest` route without the probe too (`FSBASE2.log`, 103 s). It is `DisSeqAct_StartSpawn_0` fired by `-distouchprobe`'s teleports that spawns the Elite and starts it: FN measured it in the harness's `inputtest` stage, whose baseline appends `-distouch -diskismet -distouchprobe`. With those the control reproduces FN (`FSBASE3.log`: `StartSpawn` at 44.9 s, the `MAT_BeginAIGroup` warning at 44.93, `playing 1 pos 57.94 activations 1 instances 6` at 102.8 s). The before/after is on that route |
| "`UInterpTrackInst::UpdateTrackInst` ... vtable slot 74 (+296)" | 2012's slot. The 2013 `UInterpTrackInst` vtable has two extra virtuals at 73/74 and the seam is **slot 76 (+304)**; 2013's pass loop calls slot 75, `ConditionalUpdateTrackInst`. The 2013 `UpdateTrackInst` of the Stretch instance is `0x505c70` (823 B, unnamed, found through the vtable) and is 311 B bigger than 2012's (§1.2) |
| "`StretchAnimControl` first (`0x543f40`, 512 B; self-contained ~4.2 KB ...)" | Right, with one dependency the list did not have: `UInterpGroup::UpdateAnimWeights` (2012 `0x256000`) weighs the Stretch tracks too; the reference body only knows `UInterpTrackAnimControl`, so without the second half every channel's weight stayed 0 |
| "`ADishonoredNPCPawn::MAT_BeginAIGroup` (2013 `0x765080`, 720 B) and retail's `InitPawn` ... replacing FN's guard with the native call" | `InitPawn` and the call are in; **the body is a third of retail's** because eleven of its callees do not exist in this tree (LOD manager, mesh offset, blend-pose node, `MatineeControllerAdd`, the soiree AI stim, the look-at request). What the Boat scene needs from it - the slots re-cached, the master FSM told to exit, steering off, the locomotion modifier cleared - is there; the rest is inventoried in the function's banner and §6 |
| "`FaceTo`, `Locomotion`, `Dialog` use the same seam" and item 4 | Not reached this wave (the three bodies and their components are §6). `FArkComponentLocomotion::MovePawn`, which the skip-mode re-step calls, has no body either: the `-skipping` path re-steps the tracks and ticks the component but does not translate the pawn |
| "the Boat matinee's AI actor and the player group's pawn move and animate" | The Boat matinee (`Main_Sequence.SeqAct_Interp_3`: `Boat`, `Elite_StageMark`, `Elite`, `BoatCommands`, `FX`) **has no player group**; the six player groups are on the soiree matinees the dialog system starts (FR) and on FK's forced `SeqAct_Interp_26`. The AI actor animates and does not move (§0) |
| the premise nobody wrote: "a diagnostic can live on the instance" | 2013 added `m_fCurrentAnimPosition` / `m_fBackupAnimPosition` to the Stretch instance for the previous-position hand-over to `MAT_SetAnimPosition`; this port writes the first and keeps its once-a-second diagnostic timestamps in file-static maps, not in the object |

## 3. The instrument

`-fkdiag` (FK's switch) now also prints, from the Engine:

```
fkdiag stretch <matinee> group '<g>' slot <s> ch <c>: key <i> anim <name> start <t> rate <r> rootMotion <m> meshTrans <m> noCollision <b> on <actor>, matinee pos <p>
fkdiag stretch <matinee> group '<g>' slot <s> ch <c>: anim <name> time <t> weight <w> looping <b>, matinee pos <p>, <actor> at x/y/z       (once a second per instance)
fkdiag aipawn  <matinee> group '<g>': bound <pawn> at x/y/z physics <n> base <b>, <k> pawn anim set(s), <s> slot node(s) of <n> stack(s) in the tree, haveMove <b> haveLoco <b>   (InitPawn)
fkdiag aipawn  <matinee> group '<g>' pos <p>: <pawn> at x/y/z physics <n> base <b> vel <v>, stack <node> w0 <w> | ch0 w <w> <anim> t <t> | ...  (once a second per AI group)
fkdiag aipawn  group '<g>' has NO anim-control track (bIsAnimControlTrack unset on all <n> tracks): the weights are never sent     (every 5 s, AI groups only)
fkdiag stack   <node> on <actor>: ch <c> <- <anim> pos <t> (seq now <anim> t <t>, AnimSeq found|NULL), weights w/w/...              (once a second per stack)
fkdiag stack   <node> weights <- [channel weights] -> children w/w/...
```

The second `aipawn` line is the equivalent of FK's `fkdiag blend` for the AI group: the pawn's position,
base and velocity and what its first `UArkAnimNodeStack` is playing on each channel with the weights -
i.e. whether the pawn **moves** and whether it **animates**. `DishonoredGame/Src/fkmatineediag.cpp` is
untouched; the lines are in `UnInterpolation.cpp`, `interptrackstretchanimcontrol.cpp` and
`arkanimnodestack.cpp`, keyed on the switch with a cached `appStrfind`.

## 4. The runs

Every run through `build/agentFS/fs_run.py` -> `game_input.py launch` (`-inputcmd`, `-unattended`,
`-forcelogflush`) with the step file `wait 140 / shot fs / log / wait 4 / quit`; `FocusLost` 0 in every
log; the one scripted key the route needs is none. Logs in `Dishonored_Latest2026/DishonoredGame/Logs/`,
captures `Screenshots/Win32Console/apshottime_fs0000N.bmp` (1280x720 = 2,764,854 B read off the file,
copies in `build/agentFS/shots/`; the probe's camera, not the boat's - the proof is the log). The control
is `build/head_rel` staged as `DishonoredGame_fsbase.exe`, this build `DishonoredGame_fs.exe`.

| run | exe | route | what it measured |
|---|---|---|---|
| FSBASE1 | control | `-finewgame=50 -fkdiag -fkdiagat=95` (the brief's) | **fullscreen** (before the rule, §0.6). `Main_Sequence.SeqAct_Interp_3 activations 0` at 60 s and 155 s: the Boat matinee does not start on the New Game route |
| FSBASE2 | control | `-startmap=L_Tower_P -startmapopen -inputtest -distrace` + render args, windowed | the same, `activations 0` at 103 s: not the inputtest stage's whole command line either |
| FSBASE3 | control | + `-distouch -diskismet -distouchprobe` (the harness baseline's `inputtest` extras) | FN's state reproduced: `StartSpawn` 44.9 s, `matinee AI group 'Elite': DishonoredNPCPawn_26 has no script MAT_BeginAIGroup ...` 44.93 s, `playing 1 pos 57.94 activations 1 instances 6` at 102.8 s. **The before** |
| FS1 | this | FSBASE3's | the rats animate through the new seam (`fkdiag stretch ... Idle1/Idle2/Idle4 weight 1.00` on `SkeletalMeshActorMAT_5..8`); `InitPawn` binds the Elite at 44.81 s; **crash at 45.11 s**: minidump `unreal-v9411-2026.10.12-03.58.50.dmp` (base `0xf20000`, `build/agentFS/fs1_mdmp.txt`), `UInterpTrackInst::UpdateTrackInst+0xe` reading `Track` (+56) == NULL under `UInterpGroup::UpdateGroup+0x252` - the reference instances never set Arkane's `Track` member. Fixed in `InitTrackInst` and `InitGroupInst` |
| FS2 | this | FSBASE3's | no crash to the end of the stage; the Boat moves, the Elite's StretchAnimControl drives `BoatDriver_SC_Tower_Idle` at weight 1.00 - but `no stack`: `SlotNodes` empty |
| FS3 | this | FSBASE3's | with `PostInitAnimTree` / `UpdateNodeSlot`: `1 slot node(s) of 2 stack(s)`, `MatineeStackNode` with six channels - all `None` at weight 0 |
| FS4 | this | FSBASE3's | with the stack diagnostics: `ch 0 <- BoatDriver_SC_Tower_Idle ... AnimSeq NULL`, and **`group 'Elite' has NO anim-control track (bIsAnimControlTrack unset)`** - the bitfield |
| **FS5** | this | FSBASE3's | with retail's bit order: **`ch0 w 1.00 BoatDriver_SC_Tower_Idle t 0.03 ... AnimSeq found`**, weights `[1.00] -> 0.00/1.00/0/0/0/0/0`, looping for 58 s; `base none`, `vel 0`, not moved. **The after** |

Before/after on one route, one control: `FSBASE3` -> `FS5`. 0 `Critical` in FS2..FS5 and in all three
controls. **No retail run was made**: the coordinator withdrew the dismod side-by-side for this wave (the
retail exe takes the user's display and foreground), and no capture in `build/agentFB/retail/` shows the Boat
scene. The one retail measurement this package wants - the live `Children(i).Weight` of a pawn's
`UArkAnimNodeStack` during a scene, to settle the child-0 complement of §1.3 - is recorded in §6.4 as a
measurement not taken, and why.

## 5. Addresses, resolved by hand

Every 2012 rva above is a function start in `build/agentFS/idb/FS_shipping2012.i64` (imagebase 0): the 65
addresses this package cites were resolved headlessly (`build/agentFS/fs_resolve.py`,
`fs_resolve_2012.txt`) - **65 of 65 are function starts with the sizes quoted**. The 2013 side
(`fs_resolve_2013.txt`, VAs = rva + `0x400000`): `UInterpTrackInst::UpdateTrackInst 0x212570` (36),
`UInterpGroupInstAI::InitGroupInst 0x2304f0` (196), `InitPawn 0x22bbc0` (373), `UpdateGroupInst 0x2305c0`
(309), `ADishonoredNPCPawn::MAT_BeginAIGroup 0x765080` (720), `UArkAnimNodeStack::MAT_SetAnimWeights
0x534f30` (277), `MAT_FinishAnimControl 0x535050` (102), `ForceDontUseRefPoseOnChild 0x535240`, `InitAnim
0x5371f0`, `UInterpGroupInst::SetActorRootMotion 0x212e20` (17), `UInterpGroupInstAI::SetActorCollision
0x212df0` (23), `SetMeshTranslationMode 0x212dc0` (37), `ADishonoredPawn::MAT_SetMeshTranslationMode
0x74d520` (83), `APawn::GetBuildAnimSetList 0x2c30d0` (577), `ASkeletalMeshActorMAT::CacheAnimNodes
0x321e90` (219) and the fourteen named `UInterpTrackStretchAnimControl` functions are **named** in
`retail2013_named.i64` at the sizes quoted. **Unnamed** in 2013 and pinned another way: `0x22fed0`
`UpdateGroupInst` (930 B, `match_2012_2013.csv` callee 0.671, read and matched to the 2012 body pass by
pass), `0x505c70` `StretchAnimControl::UpdateTrackInst` (823 B, **by the class vtable**:
`??_7UInterpTrackInstStretchAnimControl@@6B@` at `0x1096e80`, slot 76 `+304`; slot 75 is the named
`ConditionalUpdateTrackInst`), `0x505740` `GetAnimForTime` (1269 B, the only call `0x505c70` makes with
five out-parameters), `0x534cb0` `UArkAnimNodeStack::MAT_SetAnimPosition` (629 B, callee 0.816; body
matched), `0x2b53a0` `APawn::MAT_SetAnimPosition` (179 B). **The brief's `0x505740` for GetAnimForTime
and `0x534cb0` for the stack are right; its "vtable slot 74" is the 2012 slot - 2013 is 76.**
`rva_sweep.py`: §7. The decompiles are in `build/agentFS/decomp2012/` (139 + 54 functions, headless
`decompile_funcs.py` on private copies of both databases) and `build/agentFS/decomp2013/`.

## 6. What is left, specified

In the order the measurements support:

1. **The pawn does not ride the boat.** `base none` for the whole scene (`FS5.log`). Retail's Elite stands
   on `InterpActor_18` and the Move track carries him through basing; the spawner (`DishonoredSpawner_11`,
   also the group's stage mark) puts the pawn where it stands but gives it no `Base`. Read
   `ADishonoredSpawner`'s spawn placement and `UDisSeqAct_StartSpawn` for the base hand-over (FI's), or
   whether the pawn's walking physics is meant to find the boat as its floor (`FArkComponentLocomotion`'s
   `physWalking`, DN's port, ignores the floor's motion); then the measurement is the same `aipawn` line
   with `base InterpActor_18` and a moving position.
2. **Every NOEXPORT class whose bools were declared in the reference order.** `UInterpTrack` was one;
   `UStruct::Link` keeps the cooked masks for all of them. Diff each `DECLARE_CLASS_NOEXPORT` class's
   bitfield order in the hand-maintained headers against `retail_sdk_layout.json`'s masks (`bitfield: true`,
   `mask`) and re-declare in retail's order - a script over the headers, then one build.
3. **The 2013 refinements of the seam** (§1.1, §1.2), all read, none ported: the previous-position
   `FLOAT` through `APawn::MAT_SetAnimPosition` (2013 `0x2b53a0`, 179 B) into `UArkAnimNodeStack::
   MAT_SetAnimPosition` (`0x534cb0`, 629 B: advance by `InPosition - CurrentTime` through vtable `+484`
   when `bFireNotifies` and the float is not `FLT_MIN`, so the notifies between the two positions fire);
   `m_bSpecialRootMotionExtract` (`UInterpTrackStretchAnimControl+164` bit 0) -> `sub_612E40( GroupInst,
   ... )` and the group's vcall `+1228` from the new slot-74 virtual; `SetActorRootMotion` only on forward
   movement; the reverse-playback detection; `m_bShouldBackupTransform` -> `sub_61FCB0` after the weights.
   One pass over `decomp2013/va_905c70.c`, `va_934cb0.c`, `va_62fed0.c`, `va_6b53a0.c`.
4. **The stack's weight model**: read the live `Children(i).Weight` of a pawn's `UArkAnimNodeStack` in
   retail during the Boat scene with dismod (the stack's `NodeName`, the channel weights, child 0) and
   compare with this tree's `fkdiag aipawn` line (`0.00/1.00/...`); if retail leaves child 0 at 1.0 and
   blends layered, retail's `UAnimNodeBlendBase::GetBoneAtoms` (2012 `0x1b0ec0`, ~1.4 K lines decompiled
   in `build/agentFS/decomp2012/`) is where the stack semantics live and the complement in
   `UArkAnimNodeStack::MAT_SetAnimWeights` comes out.
5. **The rest of `MAT_BeginAIGroup` / `MAT_FinishAIGroup`**, each behind its system: `FDisComponentLODManager`
   (`StartLOD` / `StopAllLODByAsker`), `FArkComponentMeshOffset` (`GetMeshOffsetComponent`, `EnableByFlags`,
   `DisableByFlags`, `ForceInstantSnapMesh`; also `ADishonoredPawn::SetMeshTranslation`), `m_pBlendPoseNode` /
   `m_MatineeControllersInfos`, `MatineeControllerAdd` / `Remove` (the `m_AIBehaviorPriority` bookkeeping
   `MAT_IsAllowed` reads), the `FAIStimStruct_Soiree` stim through `UDishonoredAIBrain::HandleAIStim_Internal`,
   `MAT_BlendOut` (`0x7ad5e0`) through `m_pMatineeBlender`, `ApplyLocomotionModifier`, the look-at half (FT),
   `ADishonoredNPCPawn::PostInitAnimTree` (`0x7ad200`, `FDisComponentAnimPlayer`). Then `MAT_IsAllowed`
   (`0x7c0c20`) with `USeqAct_Interp::CanPlay` in `InitInterp` (FO §7).
6. **`Locomotion`** (`UInterpTrackInstLocomotion::UpdateTrackInst` 2013 `0x503c30`, 1144 B;
   `UpdateTrackInst_Jump` `0x500260`, 370 B; `ComputeLocoProp`, `UpdateStopObsoleteLocoRequest`,
   `StopCurrentLocoRequest`, `UInterpTrackLocomotionKeyProperties::FindKeyTargetActor`,
   `GetGameSpecificLocomotionModifier`) and `FArkComponentLocomotion::MovePawn` (2012 `0x584ca0`, 160 B,
   decompiled in `build/agentFS/decomp2012/`), which the skip hook then calls at phase 1.
7. **`FaceTo`** (`0x4ff8f0`, 592 B) behind `FArkComponentFaceTo` (0 bodies, 4 PDB functions in the unit;
   `GetFaceToComponent` 2012 `0x577860` is `UArkComponentContainer::GetFirstComponent<FArkComponentFaceTo>`),
   then the FaceTo `PreAsyncWorkTick` in the skip hook.
8. **`Dialog`** (`0x873750*`, 854 B) once FR's runtime exists; **`LookAt`** (`StaticUpdateTracks`
   `0x5082c0*`, 3756 B) is FT's and is called from `UInterpGroupAI::UpdateGroup` before the base - the call
   site is marked.
9. `ADishonoredPawn::StartRootMotionMode` / `StopRootMotionMode` through the anim-state machine's
   `ChangeAnimStateMotionModes` (replacing the mesh writes in §1.4), and the player five behind
   `UStatePlayerMasterSoiree` (FO §6, unchanged).
10. The generator: `gen_classes_header.py` must leave `UArkAnimNodeStack` out of `DishonoredGameEngineShims.h`
    and emit `TArrayNoInit<class UArkAnimNodeStack*> SlotNodes` for `Pawn` and `SkeletalMeshActorMAT`, keep the
    `AActor` / `APawn` virtuals of §1.4, and `UInterpTrack`'s bitfield in retail's order.

## 7. Acceptance

| | |
|---|---|
| `run_regression.py` | **47 ok, 0 failed, 0 skipped**, 2,654 s; `inputtest_criticals 0`, `newgame_criticals 0`, `options_criticals 0`, `d3d9_criticals 0`, `nullrhi_criticals 0`; `layout_mismatching 0`, `layout_contract 0`, `layout_types 2314`; `newgame_on_mission 120`, `newgame_pawns 29`; `inputtest_moved 841.8` - the worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentFS_reg`, built inside the harness (the package's second build), `--newgame-at 50 --game-extra-args=-windowed`, the five gitignored `resources/docs/types/` inputs present. `build/agentFS/fs_regression.log`, `build/agentFS_reg/regression/summary.txt` |
| the working build | `build/agentFS_rel`, Release, `DISHONORED_LAYOUT_CHECKS=ON`, through `build_queued.py` (`build/agentFS/fs_build.cmd`, logs `fs_build0..6.log`): **0 errors, 0 C4263, 0 C4264** in the final pass; the six earlier passes fixed one missing include, one const `Cast`, the NULL `Track`, and added `UpdateNodeSlot`, the stack diagnostics and the bitfield in turn. The gate's clean-checkout build is the clean build |
| `rva_sweep.py` | 3 suspects, the same three pre-existing ones FK and FO reported (`disbehaviorpatrol.cpp:319`, `gfxuirenderer.cpp:1679` twice), none in a file this package touched; `build/agentFS/fs_rva_sweep.log`. Every address in this report resolved by hand in §5 |
| `build/agentFS_sync.py` | 21 files (19 source, 2 docs), **0 flagged** against main at `14cad0f` (FU's merge touches none of them); the seven files of main's `e60c5c6` merged into the worktree are listed as merged-from-main and never copied back; the script itself checks its list against `git diff --name-only d7d083a` + `ls-files --others --exclude-standard` and reports anything missing |
| headers | `gen_classes_header.py` **not run**. No reflected member added or moved; three generated headers hand-edited (`EngineClasses.h`, `EnginePawnClasses.h`, `EngineAnimClasses.h`: new virtuals, `SlotNodes`' element type, the `UArkAnimNodeStack` class) and one shim block removed from `DishonoredGameEngineShims.h`; the generator must learn §6.10 or a regeneration reintroduces the class and fails to compile |
| before/after | §4: `FSBASE3` (control) -> `FS5` (this), one route, same driver, same script |
| the input rule | every launch through `game_input.py launch` + `-inputcmd`; `FocusLost` 0 in all eight logs; no `SendInput`, no `SetForegroundWindow`, no key pressed |
| the window rule | one fullscreen launch (`FSBASE1`) before the rule; `-windowed` on every launch since, `e60c5c6` merged |
