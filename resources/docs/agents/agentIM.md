# Agent IM: the player's action base, the anim-state system, the mantle animation, the ledge test (wave 26, PHASE23 package IM)

Worktree `build/agentIM_wt` (made at `c772395`, moved to `8254e4e` (IK2) by stash + checkout + pop before build 1, because IK2 changed
the root-motion physics the mantle uses). Build dir `build/agentIM` (RelWithDebInfo), staged `DishonoredGame_IM.exe`. **Two builds, both
clean on the first try.** Build 1 ported the action base and the anim-state system; its runs found three more gaps (below), which
build 2 closed. IDA only on private copies `build/agentIM/idb/{retail2013,shipping2012}_agentIM.i64` (`build/agentIM/im_ida.py`;
decompiles in `build/agentIM/d12`, `d13`). FModel was not used. Runs through `game_input.py` (`build/agentIM/im_run.py`, steps
`im_s*.txt`), always `-windowed -nomovie`; logs `Logs/IM*.log`, run summaries `build/agentIM/IM*_run.txt`.

## 1. What was ported (2012 / 2013 rvas; every function, unit and deviation in `agentIM_status.csv`)

- **The upper-body and left-arm FSMs are initialised where retail initialises them**: `ADishonoredPlayerPawn::PreBeginPlay_Actions`
  (2012 0x720260 / 2013 0x6a4a50) puts all three machines in their default states (master: Walk; upper and left arm:
  `UStatePlayerUpperIdle`) after `PreBeginPlay_AnimStates`. Agent FZ had initialised the master only.
- **The action base**: `UStateSharedActionBase` (OnEnterState / OnResetState: DoAction; SetFiringObject; RequestStateExit_WithTime),
  `UStateSharedActionBase_PlayerOnly` (DoAction, the exit flag, CanTransitionAction / CanTransitionExternal, ChooseAnimState with the
  non-repeat helper, the param's OnPending), `UStatePlayerAction` (DoAction on the machine's bound picker, the exit to idle / nav),
  `UStatePlayerMasterAction` (DoAction 2013: input mask, the full-body state, camera influences, anim-driven camera, arm follow, stop
  sneaking; OnExitState 2013; CanTransitionExternal; AnimNotFound; OnLinkedActionExit), `UStatePlayerChangeReadyStance` (whole),
  `UStatePlayerUpperIdle` / `UStatePlayerUpperNav`.
- **The anim-state system**: `UDisAnimStateComponent` (all 15 bodies incl. `SetAnimState` 0x6cc0d0 / 0x69f610), the owner dispatchers of
  `IDisAnimStateOwnerInterface`, the player's `PreBeginPlay_AnimStates`, `FindStatePickers` (2013), `BuildAnimStateMap`,
  `GatherAnimStateArrays`, `FindAnimState`, `AnimState_AddItemPrefix`, **`SetAnimState` (2013 0x6e1da0)**, `SupportsAnimState`,
  `ClearAnimState`, the owner slots (`AnimState_TriggerExit_Derived`, `_LockoutPicker_Derived`, `AnimState_SyncEnd`, `_PlayAnim_Derived`);
  `ApplyTweakChanges_Derived` now copies the ten anim-state arrays the pickers map (they were never filled).
- **The animation side**: `UDishonoredAnimNodeSeq` (ApplySequenceNameModifiers, PlayAnim, OnBecomeRelevant, IsDominantAnimStateSequence,
  OnAnimComplete), the AnimStateExit / AnimStateAdvance / AnimStateSyncEnd / AnimStateUnlock / AllowMantleEnd / AllowMantleCancel
  notifies, and the mantle's own nodes: `UDishonoredAnimNodeSeqMantle` (root-motion Z scaled to the edge height),
  `UDishonoredAnimNodeBlendByMantleType` (low / medium / high), `UDishonoredAnimNodeBlendByMantleCrouchRequired`.
- **Callers that were waiting**: `UpdateWalkNavAnimState` sets its nav state and requests the stance change (agent IA's two named
  halves); `RequestActionHelper` / `RequestAction` (IF's refusal removed, `InvalidateCurAnimState`); the swim's `Nav_Swim`; the soirée
  exit's `InvalidateCurAnimState`; **`Landed_Native`'s land actions** (big land / small land / left hand, 2013 0x6b53a0; BF had left
  them out); `PerformMantle` requests the animated mantle (**IG's 120 uu refusal removed**); the mantle state runs `DoAction`,
  `UStatePlayerMasterAction::OnExitState` and finds its impact node.
- **The item side**: `GetItemPrefix` (2013), `GetNavAnimState`, `SetAnimState`, `SyncPawnAnimState`, `OnSyncPawnAnimStateFail`; the base
  `ModifyPlayerStance_Derived` (NotReady -> Ready) and **`UDishonoredItemEmpty::ItemIsPhysicallyEquipped` = FALSE** (see 3.4).
- **`-imdiag`** (`Inc/imdiag.h`, `Src/imdiag.cpp`): FSM / picker init, every anim state set or cleared (picker, requested -> resolved
  name, tree, result, custom sequences), upper / left-arm transitions, every action request and why it was refused, every exit /
  advance reaching the player, the animated mantle tracked tick by tick with a summary; console `IMSCAN [X Y Z]` (the ledge scan the brief
  asked for, then each of the nearest 24 tested with the real edge finder), `IMGO n`, `IMSTATE`.
- Generator: 79/79 byte-identical (`--out-dir build/agentIM/regen`); `bitfield_audit` mismatches 0.

## 2. Measurements

### 2.1 Key C / SpaceBar, spawn route (`-startmap=L_Tower_P -startmapopen`, IM5 = build 2; HEAD = IF's and IG's reports)

| input | HEAD | build 2 (picker: requested -> state (tree) [sequences]) |
|---|---|---|
| start | master Walk only; `UpdateWalkNavAnimState`: "SetAnimState not ported; Nav_Default_Ready not set" | FullBody `Nav_Default -> Empty_Nav_Default` (Nav_Default) [CustomWalk=Empty_Walk, CustomRun=Empty_Run, CustomIdle=Empty_Idle, CustomAirIdle=Empty_Jump]; upper / left arm `StatePlayerUpperIdle` |
| `key C` | `action request Upper_GoTo_Crouch_Ready refused: its machine is not initialised`; no anim state | collision 87.5 -> 65.0; FullBody `Nav_Crouched -> Empty_Nav_Crouched` (Nav_Crouched) [Empty_Sneak_Idle / _Walk / _Jump]; upper `Upper_GoTo_Crouch` granted -> `Empty_Upper_GoTo_Crouch` (SimpleAction) [Empty_Sneak_In]; AnimStateExit notify at +0.30 s -> UpperIdle, picker cleared |
| `key C` again | `Upper_Leave_Crouch_Ready` refused | 65.0 -> 87.5; FullBody `Empty_Nav_Default`; upper `Upper_Leave_Crouch` -> `Empty_Upper_Leave_Crouch` [Empty_Sneak_Out]; exit at +0.27 s |
| `key SpaceBar` | Walk -> Jump -> Falling -> Walk, no animation request | the same transitions (apex Z +105.6: -225.2 -> -119.6 at Jump->Falling); on landing upper `Upper_Jump_Land_Small` -> `Empty_Upper_Jump_Land_Small` [Empty_JumpLandSmall]; exit at +0.43 s |

`refused: ... not initialised` lines: 0 (IM5, IM6). `anim not found`: 0 in build 2. Criticals 0.

### 2.2 The ledges (`-finewgame=50 -fosoiree -nomovie`, after the ride; IMSCAN centre `9826.257 23064.211 2324.300` = the dock stop)

IMSCAN: 15,625 cells (7,323 with ground), 147 flat steps in 55..230 uu, 24 candidates kept, **23 of 24 accepted by the real finder**
(IM6; the same 24 in IM2 / IM3). Selected:

| IMGO | stand (x y z, yaw) | finder edge | path | result (IM6, build 2) |
|---|---|---|---|---|
| 4 | 9895 21672.2 1874.7, 0 | 61.9 | step-up | Z +61.9 at once, **58.4 at 1.5 s (diff -3.5)**, Walk |
| 9 | 10105 21528.2 1730.7, 32768 | 85.9 | step-up | **+85.9 (diff 0.0)**, Walk |
| 0 | 9894.9 24264.2 3540.3, 0 | 109.9 | step-up | +109.9 at once, then the pawn walks over a 110 uu wall and drops behind it (a wall, not a ledge) |
| 3 | 9094.9 24216.2 3540.3, 0 | 121.9 | **animated** | `Empty_MantleMedium`, Mantle state 0.67 s, **Z +114.2 (diff -7.7)**, exit by AnimStateExit, Walk, collideworld 1 |
| 18 | 11095 21720.2 1955.7, 0 | 133.9 | **animated** | `Empty_MantleMedium`, 0.72 s, **+134.5 (diff +0.6)**, Walk, collideworld 1 |
| 22 | 11095 21576.2 1811.7, 0 | 181.9 | **animated** | `Empty_MantleHigh`, 1.00 s, **+173.6 (diff -8.3)**, Walk, collideworld 1 |

Every animated mantle: `StatePlayerMasterMantle` entered and left (enter/exit logged), the full-body state `Mantle` (tree Mantle,
root motion Translate) set, physics parked in None during the climb and restored to Walking, `bCollideWorld` 1 and `bNoEncroachCheck` 0
after, collision height 87.5. Plain jump: apex 105.6 (IM5; IF 106.2, IG 96-104).

### 2.3 Gate (`run_regression.py --build-dir build/agentIM --no-build --exe-name DishonoredGame_IM.exe --log-prefix IMg --only d3d9,inputtest,newgame`)

22 ok, 1 failed (`build/agentIM/gate.txt`): criticals 0 / 0 / 0, `unported_natives` 0, **`probe_natives` 3**, `d3d9_frames` 1380,
`newgame_on_mission` 107, `newgame_pawns` 29; **`inputtest_moved` 218.2 and 511.1 (retry)** - see 2.4.

### 2.4 inputtest A/B

Interleaved `--only inputtest --no-load-retry`, base = `DishonoredGame_IG.exe` (IG's build, no anim-state code), same minutes:
IG **428.2**, IM **1045.4**, IG **1043.9**, IM **1063.1** (peak speed 500-516 in all four). The gate's 218 / 511 are load (other
agents' builds at 100 % CPU), as IA measured (216 / 751 for its base); the walk itself is unchanged - the player's actions take no
movement input while walking (only `Upper_*` actions on the upper machine, which mask nothing).

## 3. Where the brief was wrong (with the measurement)

1. **"find where retail creates/initialises them"**: retail initialises all three machines in the same `PreBeginPlay_Actions`
   (2012 0x720260: master Walk, upper Idle, left arm Idle); agent FZ wrote only the first. The subobjects exist and carry their states
   (`imdiag init`: upper `DisNativeStateMachine_PlayerAction` 11 states, left arm 6, picker bindings 1/1/1 and 2/2/2 with nav on the left).
2. **The boundary was the whole anim-state system, not DoAction / SetAnimState**: `UDisAnimStateComponent`, the picker maps (whose
   state arrays `ApplyTweakChanges_Derived` never filled - 0 states before, 87 / 49 / 44 after), the Arkane sequence node (the only
   caller of AnimState_TriggerExit: without it no action ever ends), the notifies, and the mantle's own nodes. Build 1 without the mantle
   nodes measured the animated mantle rising **20.0 uu at a 181.9 edge and 104.2 at a 133.9 edge** (always `Empty_MantleLow`, unscaled
   root motion); build 2 with `UDishonoredAnimNodeSeqMantle` / `BlendByMantleType`: 173.6 / 134.5.
3. **"jump-in animations play on SpaceBar"**: retail requests the jump-in actions only for a non-empty primary / left hand
   (`PlayJumpInActions`); the opening's empty hands get none, in retail too. What plays is the **land** action (`Landed_Native`, 2013
   0x6b53a0, unported until now): `Upper_Jump_Land_Small` measured in IM5.
4. **The empty hand is not physically equipped** (2012 vtable `UDishonoredItemEmpty` slot 76 = 0xd9220 `xor eax, eax / retn`; agent GJ's
   base TRUE applied to it). With TRUE, build 1 picked the `_Ready` nav states and requested `Upper_Exit_Ready` / `Upper_GoTo_Ready`, whose
   `Empty_Ready_*` sequences exist in no loaded anim set (`obj list class=AnimSet`; the empty hand's `m_PawnItemAnimSets` is empty): the
   stance action never ended and refused `Upper_GoTo_Crouch` 11 s later (IM1). The base `ModifyPlayerStance_Derived` was also wrong
   (2012 0x849c80: NotReady -> Ready, TRUE).
5. **"an edge above 120 uu"**: the threshold is the finder's edge height (`m_LowMantleMaxEdgeHeight` 120), which reads 12-15 uu above
   the floor-to-top step (geometric 107.4 -> finder 121.9 -> animated; 98.1 -> 109.9 -> step-up).
6. **"no ledge on either route"** (IG): the dock area has many (23 accepted within 3,000 uu). IG's scan traced with `TRACE_World` and a
   60..115 uu window; IMSCAN uses the finder's own flags 0x800020C6 and its 55..230 range.
7. **Teleports kill the player**: a measurement teleport between floors lands as a fall (`TakeFallingDamage`, Health -4900 in IM4 with
   IG's `IGLEDGE`), and `APawn::SetPostLandedPhysics` then lands a dead pawn in `PHYS_None` - which looked like a mantle / jump physics bug
   in build 1 (IM2, IM3). IMSCAN / IMGO keep the player alive for 1.5 s after each placement (diag only).
8. The worktree had to move to IK2's `8254e4e` before building: IK2 changed `APawn::performPhysics` / `ProcessRootMotion`, which the
   mantle's RMM_Translate climb depends on.

## 4. Deviations and named gaps (all in the code banners)

- `UStatePlayerMasterAction::TickState` clears `bWantsToCrouch`: retail copies a saved `bIsCrouched` from a bit of +112 the SDK layout
  does not declare (its first declared flag reads as bit 1 in retail's `ShouldIgnorePitch`).
- `UDishonoredAnimNodeSeq::OnBecomeRelevant`'s name re-evaluation and play are applied to player / item nodes only; `PostAnimNodeInstance`
  of the base node is not written (NPC sequences are driven by NPC anim states that are not ported; IB/IC/IK territory).
- The ranged notifies (`AddPendingRangedNotify` ... `NoLongerNotifyMaster`, `AllowMantleCancel::Notify_EndRange`) need virtual hooks in
  the engine's `UAnimNodeSequence::IssueNotifies`.
- `IDisAnimStateFiringInterface::FromObject` / `IDisAnimStateOwnerInterface::FromObject`: the player-side classes by their C++ base, the
  rest by `GetInterfaceAddress` (they agree: `imdiag init owner interface` prints the same address).
- NULL tests retail does not have: an `OnPending` with no pending state, an action machine's missing item, the anim sub-tweaks in
  `ApplyTweakChanges_Derived`, a picker index out of range; `ChooseAnimState` of an action without states answers None.
- `Landed_Native`: the floor test uses `IsStatic() || bWorldGeometry || ShouldTrace` for retail's +288 bits 0 / 7 + ShouldTrace; the
  landing contact and the camera's physical react stay named.
- Not written / named: `UDishonoredInventoryItem::FindAnimStatePickers` (the item mesh does not follow the pawn's states), `SetLinkedNPC`,
  `OnLinkedActionExit`'s game event, `UStatePlayerMeleeAttack` chain start, the swim's picker lock, `PreBeginPlay_PlayerCombat`,
  `InitNavigationHandle`, `ItemActionEnded`, the editor-only `FindTotalAnimZTrans`.
