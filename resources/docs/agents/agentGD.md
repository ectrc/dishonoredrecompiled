# Agent GD (PHASE19 GD) — the go-to arrives, and Use

Worktree `build/agentGD_wt` at `9f214c5` with main `785e91d` (FZ) merged by `git merge` (no conflicts). Own build
`build/agentGD_rel` (`build/agentGD/gd_build.cmd`, through `build_queued.py`), the gate in `build/agentGD_reg` (its in-harness
build is the second and last). Own IDA copies `build/agentGD/idb/GD_retail2013.i64` / `GD_shipping2012.i64`; the 2013 copy was
read through the idalib MCP session until the worker pool dropped it, the 2012 copy and the rest of 2013 through
`resources/tools/ida/run.py` with `build/agentGD/gd_decomp.py` / `gd_xrefs.py` / `gd_findstim.py` (decompiles in
`build/agentGD/decomp2012`, `decomp2013`, xrefs in `xrefs2012*.txt`). Every address below was resolved by hand in both
databases (2013 rva = VA − 0x400000). No FModel tool. Every launch through `game_input.py launch` with `-windowed`
(`build/agentGD/gd_run.py`, an agent-private ini directory); no retail `Dishonored.exe` launch at any point; nothing real
was touched. Control: `build/head_rel` rebuilt at `9f214c5` (exe 20:58, the coordinator's), staged as
`DishonoredGame_GDbase.exe`.

## 0. The answer

1. **The go-to arrives.** `DisSeqAct_AIGoToActor_2` → `DishonoredNPCPawn_25` walks its 89 uu at 134 u/s, the component raises
   THRESHOLD then DESTINATION_REACHED (exact), the behaviour's filter takes it, `goto finished`, **`SetGoToStatus Success`**
   (GD1 at 43 s, GD3/GD7 at 62.5 s - 3 s after the request); on the control (GDC1/GDC2) the same pawn stands 36 uu short with
   `arrived` and `speed 0/0` until quit, as FV measured. Two faults, neither the one the brief named (§1): agent DN's invented
   arrival radius, and the behaviour filter masks that no behaviour in this tree ever had.
2. **Use works end to end.** With the player facing the captain (`DishonoredNPCPawn_7`), the crosshair lands on him with
   `CHS_OVER_USABLE` (his brain's `AllowPlayerUse` → `CheckDialogHook( DDH_PLAYER_USED )` found the 'Three days late...' line);
   the Use button (both `-gduse` and game_input's real `key F`) runs `UDisUseState_WaitForInput::TickState` →
   `AttemptInteract` → `ADishonoredNPCPawn::AttemptInteract_Derived` → `UDishonoredAIBrain::HandleUsedByOther` →
   **`FireDialogHook( DDH_PLAYER_USED )`** → `DisDialogOneShot_2 ... DisConv_Soiree_5: StartFromDialog on
   SOIREE_Empress_Regent.SeqAct_Interp_86` (activations 1, the captain's scene playing) - GD7 on the pre-merge tree and GD9b on the
   tree merged with GB/GE/GF (where the Use must be pressed within half a second of facing him: the Empress scene then takes
   the player - `useInput 0`, the pawn moved to 16242/21620 by the scene - which is retail's own blocking, now reached because
   GE's `PlayFromNode` fix lets the second one-shot fire). The control has no crosshair, no use FSM and no hook (GDC2: the
   key does nothing).
3. **`AIGoToActor_3 'Curnow GoTo 1'` does not fire Success** on any run, control or ours: its pawn is `DishonoredNPCPawn_26`,
   standing on the ride boat at 10092/23058/2319 after the lift, and his locomotion answers `nopath` on every tick (116 in
   GD7). GC's `-gadiag` on the merged tree (GD8) names it: `pathfail DishonoredNPCPawn_26: no start poly at
   X=10092.222 Y=23057.514 Z=2318.995 (nearest X=10092.222 Y=23058.072 Z=2318.999 found 1)` - a navigation-mesh point 0.56 uu away
   in Y that `GetPylonAndPolyFromPos` does not contain: **the edge-point bounds test GC named for Worker A**
   (`UpdateStartLocAndVerifyIfOnValidPoly` / `FindNearestLocationOnNavMesh`), not the boat's 750 uu (GB) alone. The go-to
   machinery is proven on `_25`; `_26` needs that bounds fix (§5).
4. Gate: §4. Sync: `build/agentGD_sync.py` reads 0 flagged against `d96a331` (GC merged into the worktree, no conflicts).

## 1. Premises of the brief, corrected with the measurement

1. **"`HandleArrival` never runs for the go-to pawn because `ComputeDynamic` exits on `IsDisabled() || STOP_END` first"** (FV §0.2,
   PHASE19 §GD) - wrong on both counts. Nothing in this tree ever writes `CPNT_LOCO_ROOT_STATE_STOP_END` (the only writers of
   `m_RootState` are the constructor, `OnDisable` and `HandleTeleport`, all `NONE`) and `IsDisabled()` is `m_bDisabled` -
   written only by `Disable()`, which has no caller - or a pawn being deleted. `HandleArrival` DID run: FV2's own line
   `path 1/2 arrived pathend 37` is `m_fSq2DDistToPathEnd` = 37², and that member has exactly one writer,
   `ComputeIsArrivedFlagAndSq2DDistToPathEnd`, called from `HandleArrival`. The go-to pawn was "arrived" at 37 uu because
   agent DN's arrival rule was invented: "inside the request's end threshold, or with none (−1, the go-to's) inside the
   collision radius" - 37 < the Elite guard's radius, so `m_bIsArrived` went TRUE, `ComputeTargetMoveSpeed` answered 0 and the
   pawn stood still 37 uu short. Retail's rule (2013 `0x544390` / 2012 `0x585330`, §2.1) is **0.1 uu** for an accurate-stop
   request, measured from where the pawn will be after this sub-step's move.
2. **"The only emitter of DESTINATION_REACHED ... never runs"**: the event WAS raised for NPCPawn_25 (FV2: `3 → 10 arrivals` and
   `DestinationReached(24)=3 → 10` between 10.93 s and 11.94 s, the pawn's arrival among them), and the brain enqueued the stim.
   What never ran is the behaviour's filter: `UDishonoredAIBehavior::BuildBehaviorFilterStimMasks` in this tree was the retail
   BASE body (2013 `0x6e4b60` / 2012 `0x12e290`, 27 bytes: NULL for all three masks) and no derived override existed, so
   `CallFilterAIStim`'s first guard (`!CompleteMask`) dropped every stim before any delegate - DestinationReached for the
   go-to, for every patrol (wave 14 EP's "stops at its first point"), for every sub-state. Retail overrides it in EVERY
   concrete behaviour with the same body on per-class statics (UDisBehaviorGoTo 2013 `0x6edb90` / 2012 `0x74c850`,
   GoTo_Base `0x74c900`, 20 more named in the 2012 PDB): copy `BuildFilterStimMask()` into the complete mask once, roll the
   sub-processes' and sub-states' masks in with `BuildInternalFilterStimMasks`. §2.2 is that body, keyed by class.
3. **"The player-use path (`UDisPlayerUse*`) is skeleton"**: the classes are `UDisUseState_Base / _WaitForInput / _Finished /
   _Holster / _AltInteract` (five comment-only units) - and the path is longer than the brief's: the crosshair must first
   be put on the NPC by `ADishonoredPlayerController::UpdateCrosshair`, which the camera's `UpdateViewTarget` calls through
   `OnCameraUpdate` on pass 1 (2012 `0x6fa840`, FZ's "unported"), and the NPC must answer `CHS_OVER_USABLE` from
   `GetCrosshairStatus` → `UDishonoredAIBrain::AllowPlayerUse` → `CheckDialogHook( DDH_PLAYER_USED )`. None of that existed.
4. **"`DDH_PLAYER_USED` → the dialog hook"** is fired by the BRAIN, not by a behaviour: `UDishonoredAIBrain::HandleUsedByOther`
   (2013 `0x723640` / 2012 `0x758500`) fires `FireDialogHook( DDH_PLAYER_USED, player )` on its pawn itself when the crosshair
   said USABLE and the two are not enemies, and only then hands the brain a `FAIStimStruct_PlayerUsed` stim - which no
   behaviour, sub-process, sub-state or brain process filters (a decompile sweep of every `GetFilterStimDelegate` /
   `BuildFilterStimMask` / `FilterAIStim_BrainProcess` in the 2012 database for the ids 73 (2012) and 76 (2013):
   `build/agentGD/stim7*.txt`, empty). The hook is the whole consequence of Use on a talking NPC.
5. **Use reaches the dialog through the conversation component already ported by FR**: `ADishonoredNPCPawn::FireDialogHook` →
   `UDisConversationComponent::FireDialogHook` → `UDisDialogTree_InGameBind::FireDialogHook` → the one-shot's `DDH_PLAYER_USED`
   hook row → `DisConv_KismetActivateRemoteEvent` → `DisSeqEvent_DialogOutputs 'Give_Letter'`. Nothing new was needed
   below the pawn.
6. **The 2013 `GetCrosshairStatus` and `AllowPlayerUse` are not the 2012 ones** (the skeleton comments carry 2012): 2013 refuses
   the brain's answer when the awareness is Fearful (4) or Begging (6), tests a second master-FSM state class beside
   PostPossessed (its `StaticClassNoInline` is 2013 `0xcec7b0`, unnamed; not ported), starts the answer at CANNOT_USE and
   refuses a swimming player. Ported in the 2013 shape, differences noted at the site.
7. **`head_rel`'s exe**: rebuilt at `9f214c5` at 20:58 (the coordinator's note) - it IS main's code of that commit; FZ's
   `785e91d` landed after, so the control lacks FZ's player state. Both runs here are against it.
8. `ADishonoredPlayerController::OnCameraUpdate` is NON-virtual (PDB: `public: void __thiscall`); `GetCrosshairStatus` is a
   virtual of `IDisInteractableInterface` that this tree's interface CppText lacked (it declared the pickup's as a class
   virtual of its own). Adding the interface virtual makes `ADisPickup_Base::GetCrosshairStatus` an override, as retail.

## 2. The ports, with both rvas

### 2.1 The locomotion component (`arkcomponentlocomotiondynamic.cpp`)

| function | 2013 rva | 2012 | what changed |
|---|---|---|---|
| `ComputeIsArrivedFlagAndSq2DDistToPathEnd` | `0x544390` | `0x585330` (249 B) | no computed path → arrived; distance from **`m_CurPawnLocation`** (the position after this sub-step's move) to the last path point; accurate stop or no stop anim → `sq < 0.01`; otherwise inside `m_fUnaccuracyRadius` and slower than `m_fVelocityToStopMoving`, root state NONE/5 only |
| `HandleArrival` | `0x5487b0` | `0x589560` (977 B) | transcribed: the overshoot clamp onto the last point (`m_bPathAtEnd && !m_bHandleStop`, move² > dist² → move = dist, speed = the end speed), THRESHOLD then DESTINATION once each while the path is not dirty, both re-armed when the pawn is not arrived / outside the threshold again |
| `ComputeDynamic` | `0x54bec0` | `0x58ca10` (3,026 B) | retail's two roads and the **0.04 s sub-step loop** (`UpdatePathProperties`, yaws, `ComputeTargetMoveSpeed`, `UpdateMoveSpeed`, `ComputePathMove` accumulated, `m_CurPawnLocation = Location + m_CurPathMove`, the arrival flag, `HandleArrival` per sub-step), the stuck-in-a-wall test, the push-speed clamp, the root-motion delta cleared |

Left as DN had them: `ComputeTargetMoveSpeed` (its arrived → 0 rule is retail's, `0x548050`), `ComputePathMove`, `ApplySteering`,
`ClampMove`, `FullMovePawn` (FV/GA's). Still unported and named in the unit: `UpdateTurn / UpdateStartMove / UpdateStopMove`,
`UpdateFollow`, the avoidance force, `ComputeLookatSpeedMultiplier`.

### 2.2 The behaviour filter masks (`dishonoredaibehavior.cpp`)

`UDishonoredAIBehavior::BuildBehaviorFilterStimMasks` (base 2013 `0x6e4b60` / 2012 `0x12e290`; the per-class override, e.g.
UDisBehaviorGoTo 2013 `0x6edb90` / 2012 `0x74c850` 104 B, GoTo_Base `0x74c900` 139 B): one body for all 30 behaviours, the
per-class statics being a `TMap<UClass*, FClassMasks*>`. The base CppText gains `virtual const BYTE* BuildFilterStimMask()
{ return NULL; }` so the five existing declarations (GoTo, GoTo_Base, Idle, Patrol, PatrolSearch) override it; a behaviour
without one keeps an empty complete mask plus its sub-processes' and sub-states' interests, which is retail's answer for
a class whose `BuildFilterStimMask` is the base's.

### 2.3 Use

| function | 2013 rva | 2012 | note |
|---|---|---|---|
| `ADishonoredPlayerController::OnCameraUpdate` | `0x6ba860` (sub_ABA860, 155 B) | `0x6fa840` (104 B) | pass 1 → `UpdateCrosshair`; pass 0's `TickCameraTracking` and pass 1's `UpdateBlindedEffect` and `ADishonoredPawn::OnCameraUpdate` (2012 `0x7aa7a0`) named |
| `::UpdateCrosshair` | `0x6b7b80` | `0x6f7910` (418 B) | transcribed; the aim-assist direction (item contexts) named |
| `::SetCrosshairActor` | `0x6a6280` | `0x6e2090` (450 B) | transcribed; the HUD call (2013 sub_BAF540) and the highlight-actor slot (+32) named |
| `::GetPlayerCrosshairTarget` | `0x6a5ff0` | `0x6e1e00` (220 B) | `FindCombatTarget_Dir` (2012 `0x6d6b20` / pawn `0x6fea40`) unported → empty impact |
| `::FindInteractableActor` | `0x6b70f0` (sub_AB70F0) | `0x6f73f0` (1,302 B) | the narrow probe as `UWorld::SingleLineCheck` with the tweaks' extent (`DisLineProbe_Interact` 2012 `0x83d530` is a skeleton); the broad multi-probe fallback left out |
| `::CanInteractWithInteractable` | `0x6a2dc0` | `0x6d6cc0` (128 B) | transcribed |
| `::PostBeginPlay` (the `InitFSM`) | — | `0x6d6950` | `m_pUseInteractionFSM` (a script subobject) into WaitForInput |
| `::HandleHeldButtons_Context_Interactables` | `0x6a2e70` | `0x6d6e10` (75 B) | retail's body |
| `UDisUseState_WaitForInput::TickState` | `0x6d5ba0` | `0x735210` (639 B) | transcribed; AttemptEndClimb, the zoom, DropMovableObject, CancelActiveContexts, EndInteract, CanHolsterWeapons named |
| `UDisUseState_Finished::TickState` | `0x6d5970` | `0x734fe0` (140 B) | the +48 slot named |
| `UDisUseState_Holster::OnEnterState / TickState` | `0x6cca00` / `0x6d5a00` | `0x72ad60` / `0x735070` | CanHolsterWeapons / HolsterWeapons / the reload named |
| `UDisUseState_AltInteract::OnEnterState / TickState` | `0x6c69b0` / `0x6d57e0` | `0x727940` / `0x734e50` | transcribed |
| `FDisUseState_*_Param` | `0x6d5790` (AltInteract ctor) | `0x731c20` / `0x731bc0` / `0x731bf0` / `0x734e00`, OnPending `0x72daa0` | `Inc/disusestate.h` |
| `ADishonoredNPCPawn::GetCrosshairStatus` | `0x759c40` | `0x7c02b0` (196 B) | 2013 shape (§1.6) |
| `::GetCrosshairStatus_Helper` | `0x759a50` | `0x7c00d0` (465 B) | IsStunned, HasAttachedObjectsToTake, HasStatsSet, CalcPlayerAttnScale/IsSneaking named |
| `::AttemptInteract_Derived` | `0x7678c0` | `0x7c93b0` (240 B) | USABLE → the brain; PICKUP/LOOT/STEAL (OnLooted/OnGave/StealableInteract/OnStolen) named |
| `UDishonoredAIBrain::HandleUsedByOther` | `0x723640` | `0x758500` (309 B) | transcribed; `DisIsActorFrozenByBendTimePower` (2013 `0xbbf370` / 2012 `0x827170`) is `DisIsBendTimeOn()` here |
| `::AllowPlayerUse` | `0x70b630` | `0x7477b0` (149 B) | 2013 shape |
| `IDisInteractableInterface` slots | vtable `0x1141458` (+12, +36, +40) | — | GetCrosshairStatus, CanAltInteract, GetAltInteractTime added; AttemptAltInteract / EndInteract (2013 `0x6307b0`) named stubs |

`ADishonoredPlayerCamera::UpdateViewTarget` (FZ's unit) gains the pass-1 call; `-gddiag` / `-gdface` / `-gduse` are
`gddiag.cpp` (free when absent). Generator: the five use-state units came off the exclude list through
`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir` (three regenerated files copied:
`DishonoredGamePlayerUseClasses.h`, `DishonoredGameRegistrants.cpp`, `Sources.cmake`); two link anchors
(`DishonoredLinkDisUseStates`, `DishonoredLinkDishonoredNPCPawnInteract`).

## 3. The runs

All through `build/agentGD/gd_run.py` (`game_input.py launch`, `-windowed`, private inis in `build/agentGD/config`); logs copied to
`build/agentGD/*.log`, captures to `build/agentGD/captures/<run>/` (BMP + PNG, 1280x720, every first capture checked for content:
1,440-2,111 distinct sampled colours; the `d` captures after the Empress scene are FW's black screen, 101-103). d3d9 route =
FV's (`-startmap=L_Tower_P -startmapopen -nomovie -nodishud -disai -fvdiag -gddiag`, `gd1_cmd.txt`: wait 40 / shot / wait 35 /
shot / quit); soirée route = FW's (`-finewgame=50 -fosoiree -fkdiag -frdiag -fwdiag -diskismet -fvdiag -gddiag`, `gd2_cmd.txt`,
`-fwteleport=DisTrigger_8@150`, `-gdface=DishonoredNPCPawn_7@250@120` - the player 120 uu in front of the captain facing him -
`-gduse=256@0.6`, then `key F down/up` at 262 s script time; shots a..d).

| run | exe | route | what it measured |
|---|---|---|---|
| GDC1 | `head_rel` (9f214c5) as `DishonoredGame_GDbase.exe` | d3d9 | `_25`: `arrived` at dist 36 from 10.94 s to quit, `speed 0/0`, 11 arrivals, no stim reaches the behaviour; `goto stop ... Interrupted` |
| GDC2 | same | soirée | `AIGoToActor_3` → `_26` `nopath`, never moves (`locomoved` never lists him); no crosshair / use FSM; F key does nothing |
| GD1 | build 0+1 (`gd_build0/1.log`) | d3d9 | `_25`: THRESHOLD 11.10 s, exact DESTINATION 43.21 s, **Success** 43.25 s; the last 1 uu took 32 s (the stop system, §5); 0 criticals |
| GD2 | build 1 | soirée | `_25` crawled the last 1 uu for 216 s (`speed 3/3`, moves zeroed by the push clamp) - the landing test added; crosshair on `DisTrigger_16`'s brush (the probe lacked `TRACE_Blocking`) so Use hit the trigger: `use on DisTrigger_16 status 0 -> handled 0` (both `-gduse` and `key F`) |
| GD3 | build 2 (`TRACE_Blocking`, the landing) | soirée | `_25` exact arrival 0.03 s after the threshold, Success at 62.52 s; crosshair `DishonoredNPCPawn_7 status 4 canInteract 1`; the Use press produced no tick line (see GD4-6) |
| GD4, GD5, GD6 | build 3 (diag) | soirée | `useInput 1`, `use button 1` sampled, state `WaitForInput`, yet no `TickState` activity - with the Use at 256 (GD4), at 250.15 right after the teleport (GD5), and with the Empress trigger fired at 175 so the scene was over (GD6). Unexplained: see below |
| GD7 | build 4 (main `d96a331` merged; the interactable resolved by the C++ cast first, the tick-branch diag) | soirée | **the whole chain** (§0.2): `InterfaceCast` and the C++ cast give the same pointer (`02A5F6AC`), `use on DishonoredNPCPawn_7 status 4 -> handled 1`, `HandleUsedByOther ... FireDialogHook DDH_PLAYER_USED`, `SeqAct_Interp_86` activations 1; `_25` Success; `_26` `nopath` (116 ticks); 0 criticals |

| GD8 | build 6 (main `622fc3a` = GB + GE + GF merged; `-gadiag`) | soirée | crosshair on the captain `status 4`, but the Use at 256 found `use input disabled` (115 ticks): on the merged tree the Empress scene (now routed right by GE) takes the player at ~250.5 and moves him to 16242/21620; `_25` Success; `_26`: GC's `pathfail ... no start poly ... nearest 0.56 uu away found 1` (§0.3); 0 criticals |
| GD9 | build 6 | soirée | inconclusive: the merged tree runs the world clock ~65 s behind real time at 250 s, the script quit at world 242 |
| GD9b | build 6 (`gd3_cmd.txt`, longer waits; `-gduse=250.08@0.5`) | soirée | **the whole chain on the merged tree**: `face t=250.01`, `crosshair DishonoredNPCPawn_7 status 4`, `use t=250.08`, `AttemptInteract_Derived ... status 4`, `HandleUsedByOther ... FireDialogHook DDH_PLAYER_USED`, `use on DishonoredNPCPawn_7 status 4 -> handled 1`, `DisConv_Soiree_5: StartFromDialog on SOIREE_Empress_Regent.SeqAct_Interp_86`, activations 1 (85 samples); then the captain answers `status 0` (in conversation) and the scene disables use input; `_25` Success; 0 criticals |

GD4-GD6 against GD7: the only code differences are GC's merge (`arkcomponentlocomotion.cpp/.h`, `arkanimnodelocomotion.cpp`),
the order of the two casts in `DisGDInteractableOf` (measured equal) and two diag lines in `TickState`; none of them touches
`HandleHeldButtons` → `TickStateMachine`. I could not name what kept `UDisUseState_WaitForInput::TickState` from running in
GD3-GD6 (no `Critical`, no `pressed but use input is disabled` line - that line did not exist before build 3 - the FSM sampled
in `WaitForInput` while the button read 1), and I did not spend a third build on it; GD7 and GD9b are the measurements that
stand, on two trees.

The d3d9 stage never logged `execPlayDying_Native` in any of these runs (grep on every log, the gate's included).

## 4. Gate

`run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGD_reg --newgame-at 50` from the worktree copy, its own
in-harness build (the package's second build), on the tree at `0d75bf9` (GD + main `d96a331`): **47 ok, 0 failed, 0 skipped,
2060 s** (`build/agentGD/gd_gate.log`, `build/agentGD_reg/regression/summary.txt`); `unported_natives 0 <= 0`;
`build_DishonoredGame_errors 0`, `build_CoreSmoke_errors 0`, `build_LayoutProbe_errors 0`; 0 `C4263` / `C4264` in the three
build logs; `nullrhi_criticals 0`, `d3d9_criticals 0`, `inputtest_criticals 0`, `newgame_criticals 0`, `options_criticals 0`;
`options_as2_errors 39`.

**For the coordinator's hunt**: the gate's **inputtest** stage logged `ADishonoredPlayerPawn::execPlayDying_Native (parameters
consumed, result zeroed)` once, at 49.88 s - 0.03 s after `-distouchprobe` moved the player onto `DishonoredAudioVolume_1`
at 14243/21649/2084 with `physics 2` (falling) and `PhysicsVolume DefaultPhysicsVolume_1`. The log is kept whole as
`build/agentGD/gate_inputtest_execPlayDying.log`. It is the touch probe's teleport, not the d3d9 spawn: the probe put the pawn
into a falling state at an audio volume's origin; none of this package's eight d3d9 / soirée runs logged the native.

**After the gate**, main moved three times and all three were merged (`cc6b0e7` = GB + GE, `622fc3a` = GF; §6): the merged tree
was rebuilt incrementally in the working build (`gd_build6.log`, 0 errors; `gd_build5` failed on a comment seam the cc6b0e7 conflict had split in dishonoredaibrain.cpp, restored by hand) and run once more on the soirée route (GD8, §3) -
not re-gated; the coordinator's gate on the merge commit is the measurement of that tree.

## 6. The merges

`785e91d` (FZ) and `d96a331` (GC) merged clean. `cc6b0e7` (GB + GE) conflicted in `CppText/ADishonoredNPCPawn.h`,
`CppText/UDishonoredAIBrain.h`, `dishonoredaibrain.cpp`, `dishonoredplayercontroller.cpp` (GB's / GE's appended blocks beside this
package's - both sides kept, concatenated) and `Sources.cmake` (regenerated). `622fc3a` (GF) conflicted in
`CppText/ADishonoredPlayerController.h`, `dishonoredplayercamera.cpp` and `Sources.cmake`: **GF ported
`ADishonoredPlayerController::OnCameraUpdate` and the camera's per-pass call** (pass 1 comes from
`UDishonoredPlayerSkeletalComponent::UpdateChildForTickEnd_Derived`, retail's structure), so GF's declaration, body and camera
call are the ones kept, this package's duplicate OnCameraUpdate and its camera call were dropped, and the pass-1
`UpdateCrosshair( _VT )` was put inside GF's body (named there). The generator was re-run after every merge and diffed clean
against the tree except the files it then wrote (`Sources.cmake`; the five use-state units stay off the exclude list: 745 on 622fc3a, 740 here).

## 5. What is left

1. **`AIGoToActor_3` / the captain's walk**: `DishonoredNPCPawn_26` on the ride boat is 0.56 uu from a navigation-mesh point the
   start-poly test refuses (GD8, GC's `gadiag pathfail`) - GC's edge-point bounds problem in `UpdateStartLocAndVerifyIfOnValidPoly`
   / `FindNearestLocationOnNavMesh` / `UNavigationHandle::GetPylonAndPolyFromPos`; and the boat parks 750 uu short (GB). Once
   his path builds the chain is `_25`'s.
2. **The stop-animation system** (`arkcomponentlocomotionrootmove.cpp`: `UpdateStopMove` 2013 `0x547300`, `UpdateStartMove`
   `0x546f60`, `UpdateTurn` `0x54b9d0`, 15 functions) is what retail lands the last unit with (`m_bHandleStop` → end speed = walk);
   the landing test in `HandleArrival` and the `!m_bIsArrived` on the push clamp are its stand-ins, named at the sites.
3. **The crosshair probe family** (`DisLineProbe_Interact` / `Multi`, `MaskedLineProbeShared`, `FMaskedProbeFunctor_Interact`,
   2012 `0x83d530` / `0x83d590` / `0x83b5f0`) and `FindCombatTarget_Dir` (`0x6d6b20` / `0x6fea40`): the crosshair finds interactables
   through one swept line check and never finds an enemy.
4. The use path's named gaps: `AttemptEndClimb`, `DropMovableObject`, `CanHolsterWeapons` / `HolsterWeapons`, the item zoom and
   contexts, `EndInteract`, `AttemptAltInteract`, `OnLooted` / `OnGave` / `StealableInteract` / `OnStolen`, `IsStunned`,
   `HasAttachedObjectsToTake`, the HUD's crosshair update (`UDisGFxMoviePlayerHUD`, 2013 sub_BAF540), the interface's
   highlight-actor slot (+32) and released-button slot (+48).
5. **FR's check-only hook walk is noisy**: `AllowPlayerUse` calls `CheckDialogHook( DDH_PLAYER_USED )` every crosshair update
   (retail does too), and FR's `-frdiag` logs the whole walk (`played DisConv_DialogHook_0 ... FireDialogHookNow hook 1 ... fired
   (hook present, no content)`) at frame rate - 1,137 lines in GD3. Rate-limit or silence the check-only walk in `frdiag`.
6. `-fvdiag`'s `goto tick` line is now one a second PER go-to (`disbehaviorgoto_base.cpp`); FV's single static hid every go-to
   but the first (GDC2's pawn 26 had 0 lines for that reason).
7. GC's note on Worker A (the teleport onto an edge point, `GetPylonAndPolyFromPos`) was not needed for `_25`; it may be `_26`'s
   second problem once the boat is at the dock.
