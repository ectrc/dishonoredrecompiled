# Agent GI — the heads: the attention brain process

Wave 24 (`PHASE20.md`, package GI). Worktree `build/agentGI_wt`, base `4b889ff`. Working build `build/agentGI_rel`
(`build/agentGI/gi_build.cmd` → `build_queued.py`, two invocations of the one working build: `gi_build0.log` stopped on
this package's own header fault - a namespace the cpptext named before its header - and four identifier faults;
`gi_build1.log` linked), the gate's in-harness build `build/agentGI_reg`. Control `build/head_rel` (rebuilt at `4b889ff`,
exe of 15:04) staged as `DishonoredGame_GIctl.exe`; this build staged as `DishonoredGame_GI.exe`. Every launch through
`build/agentGI/gi_run.py` → `game_input.py launch` (`-inputcmd -unattended -windowed`, the build directory's private inis);
no retail `Dishonored.exe` launch; no real input; no FModel. IDA: private copies `build/agentGI/idb/GI_retail2013.i64`
(imagebase 0x400000) / `GI_shipping2012.i64` (imagebase 0), the 2012 side read headlessly through
`resources/tools/ida/run.py` with `build/agentGI/gi_decomp.py` (decompiles in `build/agentGI/decomp2012/`, 380 files) and
`gi_vtable.py` (`vt2012_attention.txt`), the 2013 side through the idalib MCP session until its worker dropped, then
headlessly (`decomp2013/`). Every address below was resolved by hand in both databases; `resources/docs/symbols/match_2012_2013.csv`
was the first pointer and is cited where it was wrong.

## 0. The answer in eight lines

1. **The attention brain process is ported and live**: `UDisAIBrainProcessAttention` (85 of 88 functions, the three
   class-registration helpers are the macro's), the three `UDisAttentionInfo_*` units (whole), the two tweak units, the
   attention-target half of every pawn (18 functions - which this tree had never given a pawn: a pawn answered the
   interface defaults, location 0,0,0 and no actor), the global attention manager's pool, 29 brain helpers the process
   calls (busted / unbusted handling, the top enemy, the empathy questions, the senses pass retail actually has), and
   `UDisAISubProcessFleetingGaze`. Sections 2 and 3; `agentGI_status.csv` has every function with both rvas.
2. **The plan's premise was wrong twice** (§1): the brain's head-track look-at that GE identified fires only for an
   *enemy* at `DAL_HeadTrack` (2013 `0x725890`, the disassembly at `0xb25910..33`); a friendly guard glancing at the
   player is `UDisAISubProcessFleetingGaze` (TopAttnProxyReplaced → a head-only look-at while the proxy is Busted,
   within the gaze radius and angle), ported here. And nothing in retail's attention process *sees*: the sense is
   `FDisComponentVisionNPC` + `FDisComponentObservable` (36 + 26 functions, skeletons), reporting through
   `ADishonoredNPCController::StartSeeingVisibleThing`. Named, not invented (§4).
3. What reaches the process in this tree is measured in §6: the soirée minimum attention (`UpdateSoireeAttention`:
   every pawn acting in a matinee with the guard is held at Busted - the dock-gate guards attend each other), touch,
   damage, the dialog / Kismet / psychic stims, and the brain's own `SetTopAttnTarget` chain.
4. `-gisight` is a declared STAND-IN for the vision component (a facing test, a range, one world line check per brain
   every 0.2 s, a lit visibility of 1), off by default, written to prove the chain downstream of a sighting; §6 reports
   the control, this build without it and this build with it, separately. It is not retail's vision.
5. Three readings of earlier agents corrected: CG's `TickBrain_Senses` ("asks the vision component") - retail's body
   (2012 `0x7562d0`, 2013 `0x717880`) never touches vision; the brain's `m_LookAtRequest` was never `Initialize`d (CG's
   note said the component did not exist; FT's does) so GE's request went nowhere; and the match table's
   `FilterTargetTouched` row (`0x73f7b0`) is `FilterSetMinAttentionForTarget` in 2013 - the real one is `0x73f420`.
6. One open defect, measured and bounded (§6): with the stand-in on, the exit purge frees one allocation that reads
   garbage (`FMallocBinned::Free` under `UObject::StaticExit`, 8 of 9 `-gisight` runs); not one of the attention objects,
   not the stims, not the dialog, not the diag (bisected); never reached without a sense, so the default build, the
   control-shaped runs and the gate are clean. Whoever ports the vision component inherits it with the dumps.
7. Gate: §8.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "what raises the look-at is `HandleTopAttnTarget`... a guard's attention target becomes the player when he walks past and the look-at request is raised" | `HandleTopAttnTarget` elects the top target and `SetTopAttnTarget` → `ChangeTopAttnTargetAttentionLevel` requests the look-at **only when the top target is an enemy at HeadTrack** (2013 `0xb25910`: `cmp byte [esi+45h],2; setz cl; ... cmp level,1; jnz skip; mov ebx,edi` - ebx, the request condition, is (relationship==Enemy) and is only ever set at level 1). The dock-gate guards are the player's allies; retail's glance at a passing ally is `UDisAISubProcessFleetingGaze` (§3), also downstream of the attention process |
| "the attention process... enough that a guard's attention target becomes the player when he walks past" (i.e. by sight) | the process has no sense of its own: sight enters as `TargetSighted` / `TargetUnsighted` (from `StartSeeingVisibleThing`, the vision component's callback) and as the observable component's `GetVisibilityLevel` / `GetSpottedDistance` read every tick by `SetupTickParamsFromTarget`; without them the complex info's sight branch is dark (`m_bAttnTargetHasObservableComponent` FALSE) and a walking player raises nothing. Both components are skeletons (§4) |
| GE: "the senses (`TickBrain_Senses`, ported) already raise what it filters" | CG's `TickBrain_Senses` was a warning line; retail's (2012 `0x7562d0`, 118 B) rebuilds the sense masks, fires the PlayerHeard Kismet event and the approach / loiter / look-at dialog hooks, runs down the magical incident and ticks the brain's look-at request. Rewritten (§3) |
| GE's `ChangeTopAttnTargetAttentionLevel` "asks the look-at component for the proxy" | it asked `m_LookAtRequest`, which `InitBrain` never `Initialize`d (CG: "FArkComponentLookat is not ported" - FT ported it in wave 21): `m_pLookAtComponent` NULL, `DoRequest` stopped at the component boundary. `InitBrain` now wires it (`FDisLookAtRequest::Initialize`, 2013 `0x8b2400`) and `TerminateBrain` `Finalize`s it |
| the match table: `0x7a7ed0 FilterTargetTouched → 0x73f7b0` (ratio 0.5) | 2013's delegate switch binds id 100 (TargetTouched) to the thunk `0xb41610` → `0x73f420` (262 B: the 2013 body adds the unpossess-touch bump) and id 91 (SetMinAttentionForTarget) to `0x73f7b0`; the table's row is the mislabel |
| CG: every pawn "implements IDisAttentionTargetInterface" | the pawn carried the interface but none of its 24 slots: CG's stand-in declared 7 (location 0,0,0, no actor), `dishonoredpawn_attentiontarget.cpp` was a skeleton. So every `FDisAttentionProxy` on a pawn had no cached actor (`HasActorReference` FALSE) and `GetProxyLocation` was the origin. The 24-slot interface and the 18 pawn bodies are ported (§3) |
| "223 functions" | 88 + 7 + 20 + 45 + 4 + 10 = 174 PDB functions in the six units named; the rest of GE's 223 are the delegate thunks and registration helpers the macros generate. This package touched 49 more outside them (the pawns, the brain, the manager, FleetingGaze) |

## 2. The process, in call order

`UDishonoredAIBrain::TickBrain` → `TickBrain_Processes` → `UDisAIBrainProcessAttention::TickBrainProcess_Derived`
(2012 `0x7aa770` / 2013 `0x7464b0`) → `UpdateSoireeAttention` (`0x7a7ce0` / `0x73f120`: every pawn in a matinee with this
pawn is held at Busted through the soirée minimum-attention type) → `TickAttention` (`0x7aa040` / `0x745910`, written in the
2013 shape where `sub_B60C10` folds the per-slot calls): per `FDisAttentionSlot`: the clear timer, `UDisAttentionInfo_Base::HandleStateChanges`
(`0x7a72a0` / `0x75b520`: a changed relationship, tweaks or life state resets the info with one message), the info's
`TickAttnInfo_Derived`, `GetCurrentCorpseResponse` (`0x7a79e0` / `0x74bfc0`, the witnessed-death state machine),
`TickAttention_Messages` (`0x798860` / `0x73b350`: every level change → `UDishonoredAIBrain::SendAttentionLevelChanged`
`0x75c540` / `0x726b30` → `OnAttentionLevelChanged_Stims` EnemyBusted / AllyBusted, `HandleTargetBusted` /
`HandleTargetUnbusted`; then `TickAttention_Kismet` `0x792e10` / `0x7363d0`, the DisSeqEvent_AttentionIncreasedTo /
DecreasedTo events); a slot with nothing left is `RemoveAttentionInfoIdx`'d (`0x7a0730` / `0x73b5c0`: Unaware announced,
`OnAttentionLost`, a simple info back to the pool); outside combat `MarkActiveReactionContexts` (`0x7a6c70` / `0x736680`)
and the seen non-enemies are collected → `CheckAttentionReactions` (`0x7a6fe0` / `0x7369f0`) → `HandleTopAttnTarget`
(`0x7a67a0` / `0x7305a0`, 1,217 / 1,239 B, unmatched by the table - the callee between the reactions and the empathy pass:
the priority ladder live enemy 5 > dead ally 4 > live neutral 3 > live ally 2 > dead neutral or enemy 1, then the level,
then for two live enemies at one level whether they attack us and the distance with `m_fDistanceDeltaForNewTopEnemy`
hysteresis for the current top; a kind of target the brain `IsBlindTo` is skipped) → `UDishonoredAIBrain::SetTopAttnTarget`
(GE's) and `SetTopEnemyAttnTarget` (`0x75a200` / `0x724ae0`: BattleVictory on losing a dead busted enemy,
NPCReachAttentionLevel broadcast in the tweaks' radius) → the top info's `SetAsTopAttnTarget_Derived` (TopAttnProxyUpdated
when the proxy moved) → `TickAttention_Empathy` (`0x7a9f50` / `0x744d80`: `ReactToAlly` `0x7a9900`, `CheckForMurder`
`0x7a9a60`, `CheckForCarryingCorpse` `0x7a6710` in its 2013 shape).

The meter (`UDisAttentionInfo_Complex::TickAttnInfo_Derived`, `0x7a74d0` / `0x769150`, 666 B both): `SetupTickParamsFromTarget`
(`0x7a2120` / `0x768e10`), `SetAccumulatorParams` (the leak, the floor the minimum level implies, the ceiling), the sight
branch (`UpdateSightedFlag` `0x793410` / `0x74f970`, `MakePendingChange` `0x7900c0` / `0x74fa50`: vision scalar × the
distance-tuned visibility per second), touch, psychic, `ApplyAttentionIncreases` (`0x79ce40` / `0x760ca0`: the 21 pending
reasons poured into the `FDisLeakyAccumulator`, each raised to its threshold, tagged, turned into an applied change with
a proxy status), `ApplyAttentionDrain` (`0x79b180` / `0x761040`), `UpdateAttentionInfo` (`0x798d20` / `0x75b680`: value →
level, up against the start thresholds, down against this brain's rolled stop thresholds, the Investigate clamp), and
`UpdateAttentionProxy` (`0x798b30` / `0x755390`: the proxy follows the strongest applied change). The simple info
(`UDisAttentionInfo_Simple::TickAttnInfo_Derived` `0x798ea0` / `0x75b7d0`) is Unaware or Busted with a lost-contact timeout.

The 2012 / 2013 differences, named where written: `IsBlindTo` (2013 `0x701020`: the mask bit *or* a soirée fake-death
request; 2012 read the bit) in `HandleTopAttnTarget` and `BumpAttention`; `FilterTargetTouched`'s unpossess bump;
`TickAttention_CheckForCarryingCorpse`'s two tweak bits; the 2013 filter mask of 38 ids (2012: 32; the six 2013-only
filters are inventoried in `agentGI_status.csv`: ReachabilityChange `0x7464e0`, WitnessShadowKill `0x73f9e0`,
DLC06_SmokeBombed `0x73fd10`, DLC06_WitnessSilentExplosion `0x73fe40`, DLC07_Resurrected `0x740450`, DLC07_Pulled
`0x740520`, with no 2012 body - their ids are admitted by the mask and left to the behaviours, which is what an unbound
delegate does in retail).

## 3. Around the process

* **The pawns** (`dishonoredpawn_attentiontarget.cpp`): the 24-slot `IDisAttentionTargetInterface` (2012 PDB struct 23580)
  replaces CG's 7-slot stand-in; `ADishonoredPawn` answers all of it (focal point = the look-at point - retail's tweak
  FName at `UDisTweaks_Pawn` +416 is untyped in the SDK dump, so `head_jnt`, the bone FT measured, stands in and is
  named; the cached `FDisAttnTargetParams` block is rebuilt per call since nothing invalidates it per frame here);
  `ADishonoredNPCPawn::GetAttnTargetBrain`; the player's `CollectAttnTargetParams` (`m_bIsPlayer`), `IsAttnTargetPlayer`,
  `OnAttentionReceived` / `OnAttentionLost` (the `m_pAttentiveBrainsList` chain the stealth HUD walks).
* **The brain** (`dishonoredaibrain.cpp`, 29 bodies; `dishonoredaibrain_senses.cpp` rewritten): `GetAttentionLevel` /
  `GetAttentionProxyInfo` / `ClearAllMinAttention` forward to the process (they were CG's stubs); `InitBrain` wires
  `m_LookAtRequest`; `TerminateBrain` clears both top targets and `Finalize`s. Named gaps in the bodies:
  `IncrementStat`, `AcknowledgeHostility`, `OnNPCCorpseUsed`, `GetCarriedCorpseOrBodyPart`, the Surprised master state,
  `IsSeenBy` (no observable component), the distraction component's flag, `DisIsActorFrozenByBendTimePower` (the global
  `DisIsBendTimeOn()`, GD's hand-over).
* **FleetingGaze** (`disaisubprocessfleetinggaze.cpp`, 6 bodies, every 2013 address a 1.000 match): `BeginSubProcess_Derived`
  `0x789a00` / `0x73d8e0`, `RefreshSubProcess_Derived` `0x789ba0` / `0x739a00` (Busted proxy within `m_fGazeRadius` and
  `m_fGazeAngleDegrees` → `SetLookAtProxyDesire( proxy, FDisLookAtInfluence::Head, -1 )` for a random duration),
  `FilterTopAttnProxyReplaced` `0x778100` / `0x73da80`. It runs on a behaviour whose tweaks list it (the generic
  `InitBehavior` loop, CG's).
* **The manager** (`UDisGlobalAIAttentionManager`, 3 bodies + `DisGetGlobalAIAttentionManager`, made on first use under the
  game info: retail constructs it at game-info init, not ported); `UDishonoredGlobalAIManager::GetAttentiveBrains` /
  `HasAttentiveBrains` / `IsIgnoreDeathOverridden` / `MakeNewAttentionTag`; `UDishonoredAIBehavior::ShouldLowerAlertness`
  and the four behaviour virtuals the brain asks (base folds: FALSE / NULL; no override ported).
* **The structs**: `FDisAttentionChangeReason` / `FDisAttentionIncrease` / `FDisAttentionIncreaseInfo` constructors,
  `FDisAttentionLevelThreshold::CalculateStoppingThreshold`, `FDisAttentionReactionContext::EvaluateAttnChangeReason`,
  `FDisLeakyAccumulator`, `FDisVisCalcTuningParams` (the snapshot aliases the tweak's point table where retail deep-copies -
  a read-only alias of a live tweak), `FDisRangedFloat::GetRandomFloatValue`, `FDisAttribute::GetBaseValue`, all as
  struct cpptext through CG's generator hook; the native `FDisTickAttentionParams`, `FAttentionTweakCategoryInfo`,
  `DisAttention::FDisAttentionChangeMessage` and the two unreflected enums in `Inc/disattentioninfo.h`.
* **Not ported, inventoried** (`agentGI_status.csv`): the corpse filters (`FilterDiscoveredCorpse` / `FilterWitnessDeath` /
  `FilterHeardCorpseSplat` / `HandleDiscoveredCorpse` `0x7a8c80`, 526 B - `IDisCorpseInterface` has no methods and
  `FDisNPCDeathInfo` is unported), `FilterPlayerHideoutTransition` (`0x7a8570`, the hideout component), the six 2013-only
  filters, `UDisTweaks_PawnAttention::EditConditionAskObj_IsConditionMet` (editor).

## 4. The sense that is missing, named

Retail's chain for a guard seeing the player: `FArkComponentManager` ticks `FDisComponentVisionNPC::TimeSlicedTick`
(2012 `0x8c6ad0` → `FDisComponentVision::TimeSlicedTick` `0x8c63e0`, 36 functions in `discomponentvision.cpp` /
`discomponentvisionnpc.cpp`): the cone (`FindObservablesInVisionCone` `0x8c2ec0`), the observables' visible points,
`CheckPointInLOS` `0x8b4a30`, then `VisionStatusChanged` `0x8afc70` → `ADishonoredNPCController::StartSeeingVisibleThing`
(2012 `0x7b08c0` / 2013 `0x7591a0`) / `StopSeeingVisibleThing` (`0x7b0970` / `0x759270`) →
`UDishonoredAIBrain::HandleTargetSighted` / `HandleTargetUnsighted` (ported here) → `FilterTargetSighted` → the info's
`GainLOS`. Every tick after that, `SetupTickParamsFromTarget` reads the target's `FDisComponentObservable`
(`discomponentobservable.cpp`, 26 functions): `GetVisibilityLevel` (`0x8af680`, the lighting-driven value) and
`GetSpottedDistance( this vision )` (`0x8b99e0`), which the pawn-attention tweaks' `m_DistanceValueTuning` turns into the
accrual rate. None of the three units is ported, `ADishonoredNPCController::m_pComponentVisionNPC` and
`ADishonoredPawn::m_pCpntObservable` stay NULL (CG's bring-up note in `dishonorednpccontroller.cpp`), and so no NPC in
this tree sees anything: every reading of the process in §6 is with the sight branch dark unless the stand-in is on.

What the stand-in is and is not: `-gisight[=range]` (`DisGISightStandIn*` in `disaibrainprocessattention.cpp`): per brain
with an attention process, every 0.2 s, the player's focal point within 1,800 uu and 70° of the pawn's yaw and clear on one
`SingleLineCheck( TRACE_World )` from the pawn's `GetLookAtPoint` → `HandleTargetSighted` on the transition (and
`HandleTargetUnsighted` back), and `SetupTickParamsFromTarget` then reports an observable with visibility 1 and the
normalised distance - for the player only. No cone shape, no LOD, no lighting, no time slicing, no other observables.
It fakes awareness, says so in every log line, and is off unless asked for.

## 5. The instruments

`-gidiag[=bits]`: once a second per brain with slots, each slot's target, level, normalised value, relationship,
deadness, proxy status / tag / location, and for the complex info the accumulator, LOS / sighted / touched / psychic flags
and the accrual / drain rates (bit 2); then the brain's top target, its level and relationship, the top enemy, and
`m_LookAtRequest`'s desired flag and priority (bit 4); a brain with no slots reports every 5 s that its process ticks, its
sense masks, whether the look-at component is wired, and the tweak objects its four categories resolve to with the ally
tweak's max / leak / thresholds / curve points; the stand-in's reason for not sighting a pawn in range (bit 8); and at exit
the destruction order of every process / info / manager with its address. `-gisight[=range]`: §4. `gi_run.py` (GE's
launcher pattern), `gi_decomp.py` / `gi_xrefs.py` / `gi_vtable.py` (headless IDA), `gic1/gi1/gi2_steps.txt`.

## 6. The runs

Every run: `gi_run.py` → `game_input.py launch`, `-windowed -inputcmd -unattended`, the `startmap` route
(`-startmap=L_Tower_P -startmapopen -ResX=1280 -ResY=720 -nomovie`, GE's GEC2 recipe), the launcher's private inis, the
step script `wait 28 / shot a / wait 7 / shot b / wait 10 / shot c / wait 5 / quit` (`-fwteleport=DisTrigger_8@30` puts the
player among the two dock-gate guards `DishonoredNPCPawn_22` / `_23` at t=30). Logs copied to `build/agentGI/<RUN>.log`,
captures (BMP + PNG) to `build/agentGI/captures/<run>/`. The first capture of every run has content (1,280x720, mean
57-97, 1,500-2,100 distinct colours at 64x36).

| run | exe | switches | what it measured |
|---|---|---|---|
| GIC1 | control `head_rel` @4b889ff (`DishonoredGame_GIctl.exe`) | `-flheaddiag -fwteleport=DisTrigger_8@30 -gidiag` | guards `_22` / `_23`: `lookatcpnt requests 0` (6 samples each); the two civilians `_19` / `_20` in their matinee: `requests 1 type 4` (the matinee look-at, not attention); `-gidiag` unknown to the control: 0 lines; exit clean |
| GI1 (final `GI1F`; earlier `GI1`, `GI1R`, `GI1E` the same) | this build | GIC1's switches | **identical request counts** (`_22` / `_23` 0, `_19` / `_20` 1); `gidiag`: all 26 brains carry a ticking `DisAIBrainProcessAttention_N` with 0 slots, sense masks `0x0/0x0`, `lookat component set` (the brain's request now reaches FT's component; before this package `m_pLookAtComponent` was NULL on every brain), the process tweaks resolved to assets (`DisTweaks_AIBrainProcess_Attention_3`: `Twk_Attn_Default_Neutral` max 1.00 leak 2.000 thresholds 0.25/1/1/1, `Twk_Attn_Guard_Unsuspecting` / `_Suspecting`; civilians `AttentionProcessTweaks`: `Twk_Attn_Default_Ally` max 0.25); 0 criticals, exit clean. Nothing sees, so nothing is attended: the measurement of §4 |
| GI2 (final `GI2F`; `GI2`, `GI2R`, `GI2D`, `GI2E`, `GI2F`, `GIB1-5` before it) | this build | GI1's + `-gisight` (the declared STAND-IN) | the chain downstream of a sighting runs: `_16` (the elite guard at the spawn, 1,110 uu) sights the player at t=0.8 and unsights at the teleport; `_9` (a civilian, 610 uu, neutral) and `_21` (a civilian, 1,674 uu, ally) sight him at t=30.3; each gets `slot 0 target DishonoredPlayerPawn complex`, `los 1 sighted 1 visrate 1.300` (the `m_DistanceValueTuning` curve, 3 points, times the player scalar), the accumulator climbs from 0 past HeadTrack (0.25) to **Busted (1.00 / 0.25)** within the first second and the level stays Busted (34-58 one-second samples), the proxy is `status 3` (verified) with a fresh tag each tick, and `HandleTopAttnTarget` makes the player **`top DishonoredPlayerPawn level Busted`** on both brains (`rel 0` / `rel 1`). `lookat desired 0` on both and `requests 0` on every NPC: the premise of §1 measured - a neutral or ally at Busted raises no head-track request, only an enemy would. The gate guards `_22` / `_23` never sight: `cos to player -0.02 / -0.92` - both face away from the teleport point (the player lands 15 uu behind `_23`), honestly reported by the stand-in. **Exit: a critical** (next paragraph) |

The GI2 exit crash, measured and open. Every `-gisight` run but one (GIB3) ends in `FMallocBinned::Free+0x77` ←
`appFree` ← `UObject::IncrementalPurgeGarbage+0x988` ← `UObject::StaticExit` (the delete pass of the exit purge, after
every FinishDestroy: an object's memory handed to the allocator reads a garbage pool entry, i.e. freed twice or never
allocated), 8 of 9 runs with slots, 0 of 6 without (GIC1, GI1, GI1R, GI1E, GI1F, and GI2's own GIB3). The minidumps
(`Logs/unreal-v9411-2026.10.14-17.31.18` .. `18.17.58.dmp`, read with FN's `mdmp.py`) give the freed pointer; it is never
one of the attention objects (`-gidiag` logs every process / info / manager with its address at `FinishDestroy`, 63-65 per
run, and none matches). Excluded by bisection with temporary switches (removed again): the level-change stims and the
top-target hand-over (`GIB2`), `CheckForImportantDialog` (`GIB1`, the dialog hooks fire every tick at Busted - 3,890
`hook 9` fires in GI2F, retail's shape, the dialog side's cooldown question is FR's), the diag itself (`GIB5` without
`-gidiag` crashed too; `GIB3` without it did not: one clean sample). Excluded by reading: the mem-stack arrays (every
`DIS_MEMSTACK_ARRAY` sits under an `FMemMark`, as retail's `TMemStackArray`s do), the pending-change tables (indices
guarded), the snapshot structs (no array members). Found and fixed on the way, though not the cause: retail's
`GetAttentionInfo` resets the reused info with `UObject::InitializeProperties( template )`, which in this engine (and in
retail, 2012 `0x93c20`) is an in-place `StaticConstructObject` on the live object - `GI2D` showed the live info
`FinishDestroy`'d mid-game and again at exit; the reset is now `ResetFromTemplate` (`ExitProperties` + `InitProperties`).
What remains: something the attention path touches corrupts one allocation that the exit purge frees last; the default
build never reaches it (no sense produces a slot), so the gate and every non-`-gisight` run are clean. The vision port
that replaces the stand-in will reach it on its first run; the dumps and logs in `build/agentGI/` are the starting point.

Builds: `build/agentGI_rel`, thirteen invocations of the one working build (`gi_build0..12.log`): 0 the header fault and
four identifiers, 1 a missing include, 2 two interface bodies (`CollectAttnTargetParams` /
`GetAttnTargetIncomingRelationship` base folds), 3 linked, 4-6 diag / GH / GK merges, 7 the tweak accessors (the
measurement that found them: `acc 0.000/0.000`, the class default's zeros), 8 the destroy log, 9 the template reset, 10
the bisect switches, 11 their levels / GG merge, 12 the final tree (switches removed). Final: **0 errors, 0 C4263, 0
C4264**. The gate's `build/agentGI_reg` is §8.

## 7. Files

`build/agentGI_sync.py`'s header names every file and function. Generated files regenerated, not hand-edited:
`dishonoredgameclasses.h`, `DishonoredGameAttributesClasses.h`, `DishonoredGameGlobalStructsClasses.h` (the struct cpptext
includes), `Sources.cmake` (nine units off the exclude list, 740 → 731), `DishonoredGameRegistrants.cpp` (nine anchors);
`build/agentGI/regen/` is the generator's output, diffed clean against the tree except those. `DishonoredGameNative.h`
(hand-written by design) gains three forward declarations; `disattentiontargetinterface.cpp` (CG's) gains the two base
folds the interface still lacked (`CollectAttnTargetParams`, `GetAttnTargetIncomingRelationship`: the link found them).
No Engine file touched. 48 files in the sync list, equal to `git diff --name-only c66a555` plus the untracked ones.

## 8. Acceptance

| asked | measured |
|---|---|
| the capture pair | `build/agentGI/captures/gic1/` (control) against `gi1f/` (this build, same switches) and `gi2f/` (this build + the stand-in): the same dock gate, the player among the guards; the control still shows GH's stretched silhouettes (`head_rel` predates GH), this build the merged tree |
| the request count | guards `_22` / `_23`: `lookatcpnt requests 0` on control, on this build, and on this build with the stand-in; the attention target becomes the player only where something sees him (`_9`, `_21`, `_16` under the stand-in: `top DishonoredPlayerPawn level Busted`), and a non-enemy top target raises no look-at by retail's rule (§1). The request a *walking past* would raise needs the vision component (§4) and, for a guard who is the player's ally, FleetingGaze on his behaviour's tweak list; neither exists in this tree today |
| the gate | `run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGI_reg --newgame-at 50` from the worktree's copy, the tree at `0e5207a` merged: **47 ok, 0 failed, 0 skipped**, 1,732 s; `unported_natives 0`, `nullrhi_criticals` / `d3d9_criticals` / `inputtest_criticals` / `newgame_criticals` / `options_criticals` all 0, `d3d9_frames 7,890` (one run, above 1,000), `probe_natives 4`; the in-harness build (the package's second build): 0 errors, 0 C4263, 0 C4264 in `build/agentGI_reg/regression/build_*.log` |
| hand-back | `build/agentGI_sync.py`: base `c66a555` (main's HEAD, the wave-24 fast-forwards merged into the worktree: GH `ece8e1b`, `9c44722`, GK `e3bc866`, GG `0e5207a`, `c66a555`), 48 files, **0 flagged, 0 missing** |
