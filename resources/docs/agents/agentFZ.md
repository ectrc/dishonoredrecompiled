# Agent FZ — the soirée player state: the player master FSM runs, the camera is forced to the camera bone during the boat ride, the body mode switches

Wave 22 (`PHASE18.md`), package FZ. Worktree `build/agentFZ_wt`, base `f14326b`, with main `f14326b..ad4a35d` (FY, FX,
FW, the FY link fix, FV, then GA) merged in by `git merge 02eb527` and `git merge ad4a35d` as the coordinator asked
(three conflicts, §9). Working build `build/agentFZ_rel` (Release, `DISHONORED_LAYOUT_CHECKS=ON`, through
`build_queued.py`, passes `fz_build0..5.log`); the gate's in-harness build is `build/agentFZ_reg`. Sync
`build/agentFZ_sync.py` (53 files, every function named). The
shared control is `build/head_rel`, staged as `DishonoredGame_FZbase.exe`.

## 0. The answer in eight lines

1. **The player master FSM exists and runs.** It was never initialised in this tree (agent AJ's note: no `InitFSM`
   on `m_pPlayerMasterFSM`). `ADishonoredPlayerPawn::PreBeginPlay_Actions` (2012 `0x720260`) now initialises it into
   `UStatePlayerMasterWalk` from its 23 cooked sub-object states, and `Tick_PlayerAction` ticks it every frame
   (`FZ5.log`: `master FSM initialised ... 23 states [... StatePlayerMasterSoiree ...], current StatePlayerMasterWalk`,
   `master FSM ticking ... first tick`).
2. **The boat ride enters `UStatePlayerMasterSoiree` and leaves it.** `MAT_BeginPlayerGroup` requests it with flags
   `0xa` (`DisablePlayerCollision | HasAnimControlTrack`; no move track - the player group has none), the state enters at
   world time 56.46 s (`soiree ENTER SeqAct_Interp_1 ... collision off, physics 7, body mode 1`) and leaves at 158.45 s
   through `MAT_FinishPlayerGroup -> OnSoireeFinished` (`soiree EXIT ... physics 1, body mode 0`). Physics is
   `PHYS_Interpolating` for the ride and `PHYS_Walking` after, collision off and restored, as retail does.
3. **The camera is forced.** The whole Dishonored player-camera influence pipeline was absent (`ADishonoredPlayerCamera`
   had two bodies; the stock `ACamera::UpdateViewTarget` ran the pawn's `CalcCamera`). It is ported: during the ride the
   core group blends to `UDishonoredCamera_AnimDriven` at weight 1 and the view is the player's camera bone; outside it
   `UDishonoredCamera_PlayerControl` at weight 1 reproduces the old view (the camera position and FOV are identical to the
   control before the ride: 75.00 and the pawn's eye, §4).
4. **The look is clamped.** `UDisInterpTrackAllowPlayerLook::UpdateTrack` feeds the AnimDriven limits every frame:
   yaw -40..40, pitch -20..90 at the first key, yaw -170..170, pitch -15..90 later in the ride (`FZ5.log`). **The brief's
   "0 keys" was FK's census reading the base track's keys; the Dishonored keys live in `m_TrackLookAngleKeys`.**
5. **The FOV does not change in the soirée, in retail either.** The state sets no FOV; `SetFOVTarget` has nine callers
   and none is a soirée (§2). The control's in-game FOV is **75**, not 90 (90 is the menu `CameraActor`); ours is 75
   throughout.
6. **The arms**: retail has no arms component to hide. `SetBodyMode` (2012 `0x702070`) is the mechanism: the one player
   mesh goes to `BODY_MODE_FULL_BODY` (world depth group, forced LOD 2) for a group with `m_bPutPlayerInWorldDepthGroup`
   - the boat's has it - and back to `BODY_MODE_ARMS_ONLY` (foreground, LOD 1) at the end. Measured: `fz body mode 0 -> 1
   ... DPG 1 LOD 2` at the start, `1 -> 0 ... DPG 2 LOD 1` at the end.
7. **One retail function this tree had wrong was in nobody's brief**: `ADishonoredPawn::GetBone_ByName`'s rotation
   branch skipped `PerformAngleOffsetFixup` (agent DI named it), so the camera bone answered pitch **16358 units (89.8°)**
   and the first forced-camera run looked straight up (`FZ5`, `shot_fz5b`). Ported (§5); FZ6 is the run with the fix.
8. Gate: §10.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "the state forces the camera (the FOV change, the clamped pitch/yaw)" | The state forces nothing itself: `FDisSoireeControlState::EnterSoireeMode` (2012 `0x69a4b0`) only *weights* the camera's core influence group to `UDishonoredCamera_AnimDriven` and the arm-follow / bump / physical-react influences, pushes the save lock, masks the input and sets the physics. **The forcing is the camera's influence pipeline** - `ADishonoredPlayerCamera::UpdateViewTarget` (5.8 KB), `UDishonoredCameraInfluenceGroup::GatherInfluences`, the two influences - which had no bodies at all. That is two thirds of this package |
| "the FOV it sets" | None. `SetFOVTarget` (2012 `0x709f40`) has nine callers in retail: HolePeeking, the walk state's crouch, two in Jump, `TickCameraTracking`, FollowProjectile, three item zooms. The soirée's `m_MeshSpecificFOVWeight = 0` only stops the arms' FOV. Retail's in-game FOV in this scene is the camera's `DefaultFOV` |
| "against the control (90°, unlimited)" | The control's in-game FOV is **75.00** (`FZC1.log` `fediag camera 31..97: POV.FOV 75.000, camDefaultFOV 75.000`); 90 is the main menu's `CameraActor`. "Unlimited" is right: the control's view rotation is the controller's, `0/-12736/0`, for the whole ride |
| "the arms and weapon hidden" / "the arms mesh component of `ADishonoredPlayerPawn`" | There is no arms component. The player has one `USkeletalMeshComponent` (`Mesh`, a `UDishonoredPlayerSkeletalComponent`); `SetBodyMode` moves it between the foreground depth group at forced LOD 1 (arms only) and the world depth group at LOD 2 (full body), or hides it. The items are attachments and follow the mesh's depth group. In the unattended control run no arms are visible at all in any capture (`shot_fzc1a..d`) - the user sees them when the view pitches down, which the script never does |
| "`UInterpTrackDisAllowPlayerLook` ... 0 keys" | 2 and more keys at runtime: the limits change at least once during the ride (`-40..40` to `-170..170` yaw). FK's `fkdiag track ... keys 0` counts `UInterpTrack`'s key array; this track keeps `FDisPlayerLookAngleKey`s in `m_TrackLookAngleKeys` |
| "27 units, 0 code lines each" | 26 state classes in the PDB plus `dissoireecontrolstate.cpp` = 27 units, right. The FSM's cooked sub-object list carries **23** of the 26 (`FZ4.log`): the three not instanced are the abstract bases `Base`, `Choice_Base` and `Possess_Base`; the list is in §3 |
| "`UStatePlayerMasterSoiree` ... `MAT_BeginPlayerGroup` enters for a cutscene" (as the whole of the mechanism) | True, and not enough: the state's `OnEnterState` reads `m_pPlayerPawn`, which only `FStatePlayerMasterBase_Param::OnPending` (2012 `0x6959c0`, 20 B, ICF-folded with `FDisUseState_Base_Param::OnPending`) writes. The first run with the state (FZ4) entered it and did nothing, silently, until that struct was found |
| "the player `MAT_*` five ... FO §6 has them with addresses" | FO's bodies are the 2012 ones. The 2013 `MAT_BeginPlayerGroup` (`0x6ab570`) adds two flag bits (`HasAnimControlTrack` -> 8, `m_bLeaveCameraControlToDialogTree` -> 0x20) and `EnterSoireeMode` (`0x666ff0`) branches on the second and notifies the equipped items' contexts; the 2013 shape is what is ported |

## 2. What a soirée does to the player, in retail's order, and what of it is here

`FDisSoireeControlState::EnterSoireeMode( Matinee, Pawn, BlendIn, Flags )` (2013 `0x666ff0`):

| step | here |
|---|---|
| unless `SOIREE_LeaveCameraToDialogTree`: `EnableCoreCamInfluence( UDishonoredCamera_AnimDriven, BlendIn )`; bump smoother and physical react to weight 0; input bits 1 and 15 (`0x8002`) off at `DisInputMaskLevel_Soiree` | ported |
| `DisableArmFollow( Primary/Secondary, TRUE, BlendIn )` - the two arm-follow influences to 1 | ported (the influence weights; the arm placement they feed, `ADishonoredPlayerPawn::OnCameraUpdate_Derived` 2012 `0x71e3b0`, 1227 B, is not) |
| `DisPushDisableSave( DDST_Soiree )`; `StopSneaking( FALSE )`; `bWantsToCrouch = FALSE` | ported (`StopSneaking` partial: the sneak bits; `AttemptChangeCollisionHeight` and the stand-up action are not) |
| 2013: every equipped item's contexts told a soirée started (`UDisItemContext` slot 74, 2013 `0x816d90`) | logged once; the item contexts have no bodies |
| `SOIREE_DisablePlayerCollision`: the four collision bits saved, `SetCollision( 0, 0, 1 )`, `bCollideWorld = 0`; with that or `SOIREE_HasMoveTrack`: `setPhysics( PHYS_Interpolating )` | ported (measured: `physics 7`, `collision off`) |
| `CancelAllActivePowers`; a carried corpse dropped through the upper FSM | logged once; the power components and the upper FSM have no bodies |

`ExitSoireeMode( Pawn, BlendOut, bInterrupted )` (2013 `0x6628f0`): unless re-entered, `EnableCoreCamInfluence(
UDishonoredCamera_PlayerControl, BlendOut, BlendOut > 0 )`, the arm follow back per hand whose stance is not NotReady
(`GetActualPlayerStance` unported: logged once), bump smoother / physical react back to their defaults; input back; the
save lock popped; `InvalidateCurAnimState` when the scene had an anim-control track (unported, logged); collision
restored; `PHYS_Walking`; `DeActivated` on the matinee when interrupted.

The states around it: `UStatePlayerMasterSoiree::OnEnterState` (base, master-action input bits off, `EnterSoireeMode`,
a held movable dropped), `OnExitState` (`ExitSoireeMode` interrupted, input back), `OnResetState`, `OnCancelState`,
`TickState` (`StopSneaking` every tick), `CanTransitionExternal` (only into a dead state), `SetSoireeBlendOutTime`,
`OnSoireeFinished` (`ExitSoireeMode` clean, `RequestStateExit`). `UStatePlayerMasterBase::OnEnterState` /
`OnExitState` / `GetBlendInTime`. `UStatePlayerMasterWalk::OnEnterState` (input on, `UpdateWalkNavAnimState`) /
`TickState`. `UStatePlayerMasterInDialog::StartSoireeControl` / `StopSoireeControl` / `SetSoireeBlendOutTime` (the
dialog state itself is not ported; nothing enters it yet).

**The player five** (`dishonoredplayerpawn_action.cpp`): `MAT_BeginPlayerGroup` (flags, the matinee blender enabled,
`StopSneaking`, `SetBodyMode`, then `StartSoireeControl` in the dialog state or `RequestStateChange(
FStatePlayerMasterSoiree_Param )`), `MAT_FinishPlayerGroup` (`BODY_MODE_ARMS_ONLY`, `StopSoireeControl` or
`OnSoireeFinished`), `MAT_BlendOut` (the group instance's blend-out time to the carrier; the blender's `BlendNow` or
`SetEnable( 0 )`), `MAT_IsUnequipped`, `MAT_SetForceInventory` (the re-equip of `UDishonoredItemEmpty` needs
`SetReequipItem` / `EquipItemByType_KeepTrying`, not in the tree - logged once). Retail declares the five on `APawn`;
here they stay on `ADishonoredPlayerPawn` and `interpgroupplayer.cpp` casts, as FS did for the NPC half.
`UInterpGroupInst::GetBlendOutTime` (2012 `0x2ea940`: `BlendOutTimeOverride` then `BlendOutTime`) is the one Engine
addition.

## 3. The player master FSM

`UDishonoredNativeStateMachine` had the whole transition machinery (agents AJ, EC, EJ); nothing had ever called
`InitFSM` on the player's. Retail: `ADishonoredPawn::PreBeginPlay` (2012 `0x78e220`) -> the `PreBeginPlay_Actions`
virtual (player `0x720260`): the matinee blender (`PoseBlender` in the anim tree), then `InitFSM( this,
FStatePlayerMasterWalk param, NULL )` on the master FSM, `UStatePlayerUpperIdle` on the upper and left-arm FSMs. Ported
for the master FSM only: every `UStatePlayerUpper*` unit is a skeleton, and initialising a machine into a state with no
bodies is the same as not initialising it. Retail ticks the three from `Tick_Begin` -> `Tick_PlayerAction` (2012
`0x6fae20`); FV's `ADishonoredPawn::Tick` dispatches `Tick_Body` only, so `ADishonoredPlayerPawn::Tick` chains to it and
calls `Tick_PlayerAction` (named as the stand-in it is). `ADishonoredPlayerPawn::PreBeginPlay` is the same shape for
`PreBeginPlay_Actions`.

The 23 states the cooked `DishonoredPlayerPawn` default object instances into `m_pPlayerMasterFSM->m_NativeStates`
(`FZ4.log`): Walk, Leaning, Swim, Jump, Falling, Action, Versus, ChangeReadyStance, Mantle, Stunned, Dead, InDialog,
**Soiree**, InScriptedChoice, Assassinate, HolePeeking, Climb, PrePossess, Possess, Slide, Minigame, Choke, InStore.
Not instanced: `Base`, `Choice_Base`, `Possess_Base` (abstract bases). `FindState( UStatePlayerMasterSoiree )` answers
present. No transition logic object is set, so every request is allowed, as retail's player master FSM has it.

**The inventory of the other 24 states** (`agentFZ_status.csv`, 366 rows; class, functions, bytes in the 2012 PDB):
Action 15 / 1904, Assassinate 10 / 1467, ChangeReadyStance 6 / 481, Choice_Base 18 / 1560, Choke 12 / 2825, Climb
9 / 2136, Dead 8 / 1384, Falling 7 / 799, HolePeeking 12 / 2296, InDialog 17 / 2263 (3 ported), InScriptedChoice
8 / 359, InStore 3 / 343, Jump 14 / 2504, Leaning 8 / 1138, Mantle 14 / 2256, Minigame 7 / 895, Possess 4 / 246,
Possess_Base 5 / 737, PrePossess 5 / 1289, Slide 14 / 6690, Stunned 8 / 796, Swim 9 / 1756, Versus 6 / 400, Walk
5 / 2381 (2 ported, `CheckForAutoCrouch` 2021 B and `UpdateWalkNavAnimState` 749 B read and left). Base 12 / 1013
(9 ported), Soiree 9 / 458 (8 ported; `InitializePrivateStaticClass` is generated). 43 KB of state bodies in all,
of which the two bases, Walk, Soiree and the dialog hand-over are 3.2 KB here. Each row carries the 2012 rva, the 2013
rva where the 2013 database names it (else the match table's, marked `*`), the size and the status.

## 4. The camera

Retail's `ADishonoredPlayerCamera::UpdateViewTarget` (2012 `0x7135c0` / 2013 `0x6d80a0`, both open with the same
instructions: `PCOwner->Pawn`, `ViewTarget.Target == Pawn`, `CameraStyle == 'FirstPerson'` (name index `0x4B9` in both
builds), `POV.FOV = m_fCurFOV`, the three influence groups) gathers every influence group into an `FDishonoredViewTarget`
over a delta capped at 1/15 s: group 0 is the core (`INF_GROUP_NON_ADDITIVE`: its influences' weights sum to one, the
view is their blend, `SetWeight` on one is a hand-over that shares the remainder among the others), groups 1 and 2 are
the bob and the reaction (additive, weights from `m_BobAmount` / `m_fReactionWeight`), group 3 the rest. The camera
actor is then placed at the blended non-additive position and rotation. Ported: that branch, with the no-collision
placement; `TickFOV` (the FOV targets, `DisInterpolateBySpeedWithDif`), `SetFOVTarget` / `ClearFOVTarget`,
`PostBeginPlay` (the groups' `PostBeginPlay`, the 19 cached influence pointers, the aim influence asleep, six
post-process slots, `OnTeleport`, the settings republish), `Tick`, `GetCamera_Influence`, the accessors. Not ported,
named in the banner: the collision branch (`CamMoveSmooth` 1048 B, `HandleCollision` 223 B, the cylinder resize through
every active influence's `AdjustCameraCollision`), the teleport detection, the per-pass `OnCameraUpdate` on the
controller (the arm placement and crosshair), the rain box, `TickPostProcess`, `ApplyDebugCam_Native`.

`UDishonoredCameraInfluence` (base: `SetWeight` with 2013's `bWaitToBlendOneFrame`, `GatherInfluence` with the
fixed-time-step loop, `OnActivate`, `GetFixedInterpScale`; the nine retail virtuals, slots 72..80 of the 2012 vtable),
`UDishonoredCameraInfluenceGroup` (`PostBeginPlay`, `GatherInfluences` 1473 B, `SetWeight`, `GetInfluence`,
`WeightRotator`, `OnTeleport`), `UDishonoredCamera_PlayerControl` (`GatherInfluence_Derived`: the pawn's
`GetCameraPos` and `GetViewRotation`, the controller offset, the arm-follow weights at 1, the items' arm-pitch offset
`CalcItemSpecificOffset`; the controller reset; `GoToTargetOnNextControllerReset`), `UDishonoredCamera_AnimDriven`
(`GetAnimDrivenTransform`: the camera bone rotated by the blended player look; `SetPlayerControlledLimit`;
`UpdatePlayerControlLimit` 844 B with `CalcBlendedPlayerDelta`'s softening; `GatherInfluence_Derived`; the two
activation hooks) - every function of the four classes with a body in retail is ported except
`UDishonoredCameraInfluence::PutToSleep` / `Wake` (inline). The other 15 influence classes keep their empty bases, which
is what `GatherInfluence_Derived` answering FALSE means: no contribution.

**Verified against 2013, not only 2012**: the influence bitfield. The 2012 PDB declares `m_bActive, m_bIsSleeping,
m_bUseFixedTimeStep, m_bHandleCameraCollision`; the 2013 SDK and the generated header declare `m_bActive, m_bIsSleeping,
m_bWaitToBlendOneFrame, m_bUseFixedTimeStep, m_bHandleCameraCollision` - the 2013 `GatherInfluence` (`0x6c4a40`)
tests mask 8 where the 2012 one tests mask 4. The header's order is the cooked one (FX's audit passes) and the 2013
body is the one ported. `m_InfluenceGroups` is the SDK dump's untyped 12-byte gap; `script_classes_2013` types it
`array<DishonoredCameraInfluenceGroup>` and the `InfluenceGroups()` accessor reads it as that. At runtime the camera
has **4 groups, 20 influences**, PlayerControl and AnimDriven both present (`fz camera ... PostBeginPlay`).

### 4.1 The measurement, FZ5 (the state and the camera; before the bone fix) against the control FZC1

Same route, same script (`fz_run.py`: `-finewgame=50`, shots at world 51 / 96 / 136 / 171 s, quit at 196 s), one
binary each, every launch through `game_input.py launch` with `-windowed`:

| | control `FZC1` (`head_rel`, built at `da77f0f`) | `FZ5` (this, before the bone fix) |
|---|---|---|
| in-game FOV, whole run | 75.000 (`fediag camera`) | 75.00 (`fzdiag camera`, `cur 75.00 default 75.00`) |
| boat ride | `SeqAct_Interp_1` 65.10 -> 159.35 s, `MAT_*` warn once each | 64.05 -> 158.45 s, state `StatePlayerMasterSoiree` from 56.46 to 158.45 world time |
| camera during the ride | the pawn's eye: `-3859/36626/-688` at 73.6 s, rot `0/-12736/0` throughout | the camera bone: `-3975/36675/-147` at the entry, then riding at z -703; the core group `DishonoredCamera_AnimDriven 1.00` |
| look limits | none (the controller's rotation) | `yaw -40..40 pitch -20..90` then `yaw -170..170 pitch -15..90`, flags 1 (`m_bMatchControllerRot`) |
| camera rotation during the ride | `0/-12736/0` | pitch **16358** (89.8°): the unfixed bone, §5; yaw follows the bone |
| after the ride | `0/-12736/0` | `10000/-12745/0`: the controller reset to the bone's pitch, clamped by the view-pitch limit |
| body mode | never set (0) | 0 -> 1 (world DPG, LOD 2) at the start, 1 -> 0 (foreground, LOD 1) at the end |
| physics | 1 | 7 during the ride, 1 after |
| captures | `fzc1a..d`, 1920x1080, 6,220,854 B each, real content (thumbnails `build/agentFZ/shot_fzc1*.png`) | `fz5a..d`, same size; `fz5b` is the sky (the 90° pitch), `fz5c` the lock, `fz5d` the dock |

### 4.2 FZ6 - with the bone fix

Same build line as FZ5 plus the bone fix (§5): `fz_build4.log`, 0 errors, 0 C4263/C4264. Launch line (from the log's
`Init: Command line`): `-inputcmd=build/agentFZ/fz6_cmd.txt -unattended -windowed -forcelogflush -LOG=FZ6.log
-apshotname=fz6 -finewgame=50 -fosoiree -fkdiag -fkdiagat=185 -frdiag -fzdiag -diskismet -fediag`. Log
`Dishonored_Latest2026/DishonoredGame/Logs/FZ6.log`, 0 `Critical`.

| event | log time | what the state did |
|---|---|---|
| FSM initialised | 5.13 s | 23 states instanced on `DishonoredPlayerPawn`, current `StatePlayerMasterWalk`, blender `ArkAnimNodeBlendPose_0` |
| `MAT_BeginPlayerGroup SeqAct_Interp_1 'PlayerGroup0'` | 65.51 s (world 56.5) | flags `0xa` (DisablePlayerCollision | HasAnimControlTrack; no move track), blendToMark 0.20, worldDPG 1 |
| soiree requested | 65.51 s | accepted 1, pending `StatePlayerMasterSoiree`, current `Walk` |
| soiree ENTER | 65.51 s | `flags 0xa blendIn 0.20 (collision off, physics 7, body mode 1)` - PHYS_Interpolating, FULL_BODY |
| `MAT_BlendOut PlayerGroup0` | 159.72 s | blendOut 0.20 (matinee `BlendOutTime`, `GetBlendOutTime`), blender yes |
| soiree EXIT | 159.72 s | `blendOut 0.20 interrupted 0 (physics 1, body mode 0)` - PHYS_Walking, ARMS_ONLY |

Per-second `fzdiag camera` line (every sixth sample shown; FOV is the view target's FOV, `cur/default` the camera's
`m_fCurFOV/m_fDefaultFOV`, `rot` the final view rotation in UE units, `limits` the AnimDriven influence's look clamp
in degrees, `flags 1` = `m_bMatchControllerRot`):

| t (world) | FOV | loc | rot | core group | limits yaw / pitch | state body physics |
|---|---|---|---|---|---|---|
| 48.36 | 75.00 (cur 75 default 75) | 4223/-7120/1894 (menu/intro cam) | 456/-9852/2 | PlayerControl 1.00 | none | Walk 0 1 |
| 54.50 | 75.00 | -3897/36632/-145 | 0/-11348/0 | PlayerControl 1.00 | none | Walk 0 1 |
| 60.60 | 75.00 | -3966/36650/-348 | **-26/-11588/0** | **AnimDriven 1.00** | **-40..40 / -20..90** | Soiree 1 7 |
| 66.65 | 75.00 | -3906/36684/-703 | -26/-10788/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 72.70 | 75.00 | -3655/36354/-703 | -26/-10280/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 84.84 | 75.00 | -1819/31406/-703 | -26/-11924/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 96.95 | 75.00 | 1728/24736/-703 | -26/-6464/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 109.08 | 75.00 | 6513/22985/-703 | -26/1016/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 121.14 | 75.00 | 9044/22993/-703 | -26/1260/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 133.23 | 75.00 | 9356/23022/332 (the lock lifts) | -26/783/-6 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 145.31 | 75.00 | 9726/23039/2455 | -26/328/0 | AnimDriven 1.00 | -170..170 / -15..90 | Soiree 1 7 |
| 151.36 | 75.00 | 9721/23031/2492 | **-26/-12736/0** | **PlayerControl 1.00** | none | Walk 0 1 |
| 157..193 | 75.00 | 9721/23031/2492 | -26/-12736/0 | PlayerControl 1.00 | none | Walk 0 1 |

So, against the control (§4.1): the camera *is* forced - the core group hands over from `DishonoredCamera_PlayerControl`
to `DishonoredCamera_AnimDriven` (weight 1.00 after the 0.20 s blend), the view sits on the camera bone (z -703 riding
instead of the control's pawn eye at -688, yaw turning with the boat from -11588 through 1260 and back to 328 while
the control stays at -12736), the pitch is level (-26 units = -0.14°, the bone's rest pitch after the
`PerformAngleOffsetFixup`), and the look *is* clamped - `UInterpTrackDisAllowPlayerLook` sets `-40..40 / -20..90` at
the first key, `-170..170 / -15..90` from the second (§4). FOV is **not** touched: 75.00 the whole run in both builds
(the state never calls `SetFOVTarget`; the brief's "FOV forced" is wrong). No look input was given (`look yaw 0.0
pitch 0.0`) so the clamp was not exercised by input; it is exercised by the code path (`UpdatePlayerControlLimit`
every tick, §4). After the ride the yaw snaps to the controller's -12736 exactly as the control's (the pawn's
`m_bMatchControllerRot` path on exit), the pitch keeps -26.

Captures (`Screenshots/Win32Console/apshottime_fz6[a-d]0000[0-3].bmp`, 1920x1080, 6,220,854 B each, real content;
thumbnails `build/agentFZ/shot_fz6*.png`): `fz6a` world 55 s before the state (the quay, as `fzc1a`); `fz6b` 100 s the
canal from the camera bone, boat and Samuel in frame; `fz6c` 140 s the lock from the bone (the control's `fzc1c` at
the same world time looks at the lock wall from the stern eye); `fz6d` 175 s the dock interior after the exit, arms
hidden exactly as the control's `fzc1d` (nothing equipped - `HasNothingEquipped`, §6, not the state).

One thing the captures show that the control's do not: in `fz6b` and `fz6c` two tall, thin, dark shapes with pointed
tops stand on the boat's deck in front of the camera. They are not in `fzc1b/c`. The only thing this package adds to
the world DPG during the ride is the player's own mesh (`SetBodyMode(FULL_BODY)`: `SDPG_World`, `ForcedLodModel 2`),
seen from its own camera bone. The group does have an anim-control track (flag `0x8` is in the `0xa`) and the
pawn's route for it exists (`ADishonoredPawn::MAT_SetAnimPosition` -> `APawn`'s, `dishonoredpawn.cpp`), so what pose
the player's LOD-2 body is in while FW's handling only covers `ASkeletalMeshActorMAT` was not measured by this package:
the full-body mesh at LOD 2 in the world DPG is new to the frame, the pose it shows is not. **FZ7 (§4.3, with GA merged)
narrows it:** `fz7b` shows the same stretched silhouettes on the *other* boat too - GA's guards riding it - where
`fzc1b` and `fz6b` show nothing of the kind, so the silhouette is a pawn-mesh matter shared with NPC pawns in a
matinee-moved boat, not something the player's body mode does. Open item for whoever owns pawns in moved boats: log the
`PlayerStack` slot's active anim and the blender's weight during the ride (`-fzdiag` has the hooks, §7) and compare the
mesh's `SpaceBases` with retail's.

### 4.3 FZ7 - the same run on the tree with `ad4a35d` (GA) merged

`fz_build5.log` (0 errors, 0 C4263/C4264), `FZ7.log`, 0 `Critical`, same launch line with `-LOG=FZ7.log -apshotname=fz7`.
Same shape as FZ6: `MAT_BeginPlayerGroup ... flags 0xa` and `soiree ENTER ... collision off, physics 7, body mode 1` at
66.97 s; `AnimDriven 1.00`, `rot -26/-11500/0` at world 60.6 with limits `-40..40 / -20..90`, then `-170..170 / -15..90`
through the ride (`-26/-10192/0` at 72.7, `-26/1348/0` at 121.2, `-26/416/0` at 145.4); `MAT_BlendOut ... blendOut 0.20`
and `soiree EXIT ... physics 1, body mode 0` at 161.60 s; after it `PlayerControl 1.00`, `rot -26/-12736/0`; FOV 75.00
the whole run. GA's guards riding the boat and the `ADishonoredPawn::PreBeginPlay` chain change nothing here. Captures
`fz7a..d`, 1920x1080, 6,220,854 B each (thumbnails `build/agentFZ/shot_fz7*.png`).

## 5. The camera bone, and the function this tree had wrong

`UDishonoredCamera_AnimDriven::GetAnimDrivenTransform` -> `ADishonoredPlayerPawn::GetCameraBone` (2012 `0x705540`: the
bone named by `UDisTweaks_PlayerPawn::m_pCameraTweaks->m_CameraBoneName`) -> `ADishonoredPawn::GetBone_ByName` (2012
`0x7956a0`). Retail's rotation branch is **not** `GetBoneMatrix(bone).Rotator()`: it is
`USkeletalMeshComponent::GetBoneMatrixLocal( bone )` (2012 `0x33afa0`: `SpaceBases(bone)` as a matrix - the
component-space atom, not the parent-relative one agent AE's `execGetBoneMatrixLocal` returns) followed by
`ADishonoredPawn::PerformAngleOffsetFixup( Matrix, TRUE )` (2012 `0x795080` / 2013 `0x757110`): with the body tweaks'
`m_fBone_AngleOffset_Pitch/Yaw/Roll` all zero the matrix is just taken to world space; otherwise the rotation part is
multiplied by `UDisTweaks_Pawn_Body::m_AngleOffsetMatrix` with the origin held aside, then to world space. The matrix
is built by `UDisTweaks_Pawn_Body::ApplyFallbackChain_Derived` (2012 `0x7911f0`) from the three degrees when the tweak's
fallback chain is applied (`UDisTweaksBase::ApplyFallbackChain`, which this tree runs). Agent DI's `GetBone_ByName`
left the fixup out and said so; nothing read a bone *rotation* until this package. Both are ported
(`dishonoredpawn_body.cpp`, `distweaks_pawn.cpp` off the exclude list, `Inc/CppText/UDisTweaks_Pawn_Body.h`).

## 6. The arms

Retail's mechanism is `SetBodyMode` (§0.6) plus the arm-follow weights: `UDishonoredCamera_PlayerControl` sets
`m_ArmFollowWeight = 1` (the arms follow the camera), the AnimDriven influence sets it to 0 (the arms stay where the
animation puts them), and `ADishonoredPlayerPawn::OnCameraUpdate_Derived` (2012 `0x71e3b0`) applies the weights to
`UDishonoredPlayerSkeletalComponent::SetOffset` and the three `m_pLookAtControl_*` skel controls - that application is not
ported (neither is `UDishonoredPlayerSkeletalComponent`'s `SetOffset` / `SetFOV`), so what the user sees of the arms
outside a soirée is unchanged by this package. During the boat ride the mesh is in the world depth group at LOD 2 and
animated by the `PlayerStack` CamShake channel the matinee drives (FR8: weight 0.13 -> 0.00 -> 0.11), as retail.

## 7. The instrument: `-fzdiag`

`fzsoireediag.cpp`. Once a second from `UpdateViewTarget`: `fzdiag camera t=<world> FOV <pov> (cur <m_fCurFOV> default
<DefaultFOV>) loc rot groups <n> [<core group's influences with weight -> target, * = active>] limits yaw a..b pitch c..d
deg flags <f>, look yaw/pitch (blended) state <master FSM's current state> body <m_BodyMode> physics <Physics>`. Plus the
FSM's state list at init, `soiree requested`, `soiree ENTER` / `EXIT` with flags, blend times, physics and body mode,
`MAT_BeginPlayerGroup` / `MAT_BlendOut`, and `FZDIAG_ONCE` notes for every retail call this package could not make
(§2). `fz body mode a -> b` and the camera's `PostBeginPlay` census log without the switch.

## 8. Files

`build/agentFZ_sync.py`'s header: 53 files, every function named per file. 16 units off `Sources.cmake`'s exclude list
(each with a `void DishonoredLinkFZ_*()` anchor the generator collects), 3 new units, 12 new `CppText` headers and 3
edited, 2 new headers, the Engine's `UInterpGroupInst::GetBlendOutTime`. **FX's rule kept**: `dishonoredgame*classes.h`,
`DishonoredGameEngineShims.h`, `DishonoredGameRegistrants.cpp` and `Sources.cmake` are the generator's output after the
merge (`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` run in the worktree; a run into a
scratch directory before that reproduced the merged tree byte for byte except the exclude list's count comment). The
first version of this package inserted the `CppText` includes by hand with a copy of FR's tool; the generator produces
the same lines.

## 9. The merges of `02eb527` and `ad4a35d`

`git merge 02eb527` in the worktree: two conflicts. `Inc/CppText/ADishonoredPawn.h` - FV appended its relationship /
tick block, this package its camera block; both kept. `Src/DishonoredGameRegistrants.cpp` - taken as main's, then
regenerated (the anchors come from the units). FV's `ADishonoredPawn::Tick` now exists and dispatches `Tick_Body`
only; `ADishonoredPlayerPawn::Tick` chains to it and adds `Tick_PlayerAction` (retail's `Tick_Begin` phase, which FV
names as unported). FW's `UnInterpolation.cpp` change and this package's `GetBlendOutTime` are in different places and
auto-merged.

`git merge ad4a35d` (GA) afterwards, on the coordinator's second message: one conflict, again `Inc/CppText/ADishonoredPawn.h`
(GA's root-motion / mesh-translation / `PreBeginPlay` block against this package's camera block; both kept). GA's
`ADishonoredPawn::PreBeginPlay` now exists and does `PreBeginPlay_NativeComponents` + `StartAllComponents`, naming
`PreBeginPlay_Inventory / Attributes / Actions` as unported; `ADishonoredPlayerPawn::PreBeginPlay` chains to it and adds
`PreBeginPlay_Actions`, so the master FSM is initialised once, after the components, in retail's order (the comment in
`dishonoredplayerpawn.cpp` says so). GA's `EngineInterpolationClasses.h` / `UnInterpolation.cpp` (Locomotion track)
and this package's `GetBlendOutTime` auto-merged. The generator was re-run after the merge (no derived file changed
beyond GA's own). The sync script compares against `ad4a35d` and flags 0 files: 53 clean, 0 flagged, 0 missing (§10).

## 10. Acceptance

| check | result |
|---|---|
| gate `run_regression.py` (worktree copy, `--build-dir D:/RecompileDishonored/Recompile/build/agentFZ_reg --newgame-at 50 --game-extra-args=-windowed`, in-harness build on the tree with `ad4a35d` merged) | **47 ok, 0 failed, 0 skipped**, 2393 s; `build/agentFZ/fz_regression.log`, `build/agentFZ_reg/regression/summary.txt` |
| in-harness build | `build_DishonoredGame_errors 0`, `CoreSmoke` / `LayoutProbe` 0; `build_DishonoredGame.log` 0 `error C`, 0 C4263/C4264 |
| coresmoke / layout | 99 passed / 0 failed; 2314 types, 0 mismatching, 2341 probed, `verify_phase2` 3/3 (FX's bitfield audit row included) |
| criticals | `nullrhi` / `d3d9` / `inputtest` / `newgame` / `options` all 0 |
| `unported_natives` | 0 (FV's baseline, bound `<= 0`); `probe_natives` 5 (`<= 12`) |
| walking path | `inputtest_moved 1008.0`, `peak_speed 500.1`, `physics_actors 1350`, `newgame_pawns 29`, `options_as2_errors 39` |
| working build | `build/agentFZ_rel`, `fz_build5.log` (after the `ad4a35d` merge): 0 errors, 0 C4263/C4264; `fz_build0..4.log` the earlier passes |
| `rva_sweep.py` (lint) | 8683 citations, 4 suspects - all pre-existing, none in this package's files (`fz_rva_sweep.log`); addresses resolved by hand, §11 |
| sync | `python build/agentFZ_sync.py`: base `ad4a35d`, main HEAD `ad4a35d`: 53 clean, 0 flagged, 0 missing; `git diff --name-only ad4a35d` = 53 = the script's list, `git ls-files --others --exclude-standard source resources` = 0 |
| generated files | `dishonoredgame*classes.h`, `DishonoredGameRegistrants.cpp`, `Sources.cmake` not hand-edited: the generator (`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`) re-run after each merge |
| before / during / after against the control | FZC1 (`head_rel` at `da77f0f`) vs FZ6 and FZ7: §4.1-4.3; captures `fzc1a..d`, `fz6a..d`, `fz7a..d`, 1920x1080, 6,220,854 B, real content |
| input rule | every launch through `game_input.py` (`fz_run.py`), `-windowed` on every launch, no SendInput / SetCursorPos / SetForegroundWindow, no retail `Dishonored.exe` launch |
| builds | through `build_queued.py` only (working `agentFZ_rel`, gate `agentFZ_reg`) |
| crashes | none in FZ1..FZ7 (0 `Critical`, no minidump written) |

## 11. Addresses, resolved by hand

Every 2012 rva is the Shipping PDB's (`pdb_functions.csv`, imagebase 0), decompiled from a private copy
(`scratchpad/idb/fz2012.i64`). Every 2013 rva marked without `*` is `functions_2013.csv`'s (VA − `0x400000`) at the
size quoted; the ones marked `*` are `match_2012_2013.csv`'s by the method and ratio recorded there and are not
independently pinned. The 2013 bodies of `EnterSoireeMode`, `ExitSoireeMode`, `MAT_BeginPlayerGroup`, `SetBodyMode`,
`GatherInfluence`, `SetWeight` (both), `UpdatePlayerControlLimit`, `AnimDriven::GatherInfluence_Derived`,
`PostBeginPlay` (camera) and the head of `UpdateViewTarget` were decompiled from `fz2013.i64` at their VAs
(`0xa66ff0`, `0xa628f0`, `0xaab570`, `0xaa7c20`, `0xac4a40`, `0xac49a0`, `0xac9c70`, `0xacb550`, `0xad08a0`,
`0xad42b0`, `0xad80a0`) and the differences from 2012 are named in each banner. The 2013 addresses the status csv could
not pin: `TickFOV`, `TickSoireeMode`, the AnimDriven activation pair, `OnPending`, `GetFixedInterpScale`,
`OnActivate`, the two `ShowComp*Helper`s, `GetCamera_PlayerControl`, `ApplyFallbackChain_Derived` - unmatched in the
2013 database; quoted as 2012. The one wrong address found: FO §6 quotes `EnterSoireeMode` 2013 as `0x666ff0*` at
591 B, which is right; the status csv's `rva_2013` for the player five is the match table's `*` for
`MAT_BeginPlayerGroup` / `MAT_FinishPlayerGroup` and the named table's for the other three.
