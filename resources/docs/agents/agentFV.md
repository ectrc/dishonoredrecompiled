# Agent FV (PHASE18 FV) — the two natives, the AI go-to, the relationship-override API, the NPC tick, the pawn's Base

Worktree `build/agentFV_wt` at `da77f0f` with main `da77f0f..f14326b` applied by `patch -p1` (README.md and PHASE18.md only).
Own build dir `build/agentFV_rel` (one iteration build), the gate in `build/agentFV_reg` (its in-harness build is the second
and last). Own IDA copies in the scratchpad (`fv2013.i64`, `fv2012.i64`), both databases opened by hand for every address
below; `rva_sweep.py` was not relied on. No FModel tool. Every launch through `game_input.py launch` with `-windowed`
(`build/agentFV/fv_run.py`); no retail `Dishonored.exe` launch at any point - the overnight allowance was never used, and it
was void by the time the measured runs happened. The display was off for the first two captures: both read back with a real
size and real content (§4). Nothing written into the main checkout outside `build/`.

## 0. The answer in eight lines

1. **Both named natives have bodies and the gate's `unported_natives` reads 0.** `ADishonoredNPCController::execOnAIGoToActor`
   and `ADishonoredPawn::execOnSetDisposition` fire at 9.1 s of the d3d9 stage and run retail's bodies (§1, §2); the only
   `native not ported` line left in a run is Steamworks' `execExit` at quit, which the metric does not count.
2. **The AI go-to walks the pawn.** `DisSeqAct_AIGoToActor_2` sends `DishonoredNPCPawn_25` (the pawn the spawner's Kismet calls
   "the buggy pawn") to `DishonoredNavPoint_22`, 89 uu away; the brain takes the request (`UDisBehaviorGoTo`, 45 functions),
   the locomotion component gets one request, builds a 2-point path and puts the pawn on it - the pawn's position moves
   52 uu toward the mark (16744 → 16796 on X, dist 89 → 37) against 0 on the control (§4). **It does not reach the mark**:
   it stops 37 uu short, inside the component's fallback arrival radius, and the component never raises
   `DESTINATION_REACHED` - `IsArrived()` reads `m_bIsArrived`, whose constructor default is TRUE, and its only writer
   `HandleArrival` (also the only emitter of the event and the `GDisLocoArrivals` counter: 11 in every run, control
   included) never runs for this pawn because `ComputeDynamic` exits before it (`IsDisabled() || STOP_END`); the 52 uu are
   the run's extra "teleport onto the mesh" (13 against the control's 12), not a walk (`speed 0/0`, `vel 0`). That is
   agent DN's component, now GA's package - the go-to behaviour sits in stage 0 waiting for a stim the component owes it
   (`goto tick ... stage 0 reached 0 ... loco req 1 path 1/2 arrived pathend 37 speed 0/0 stuck 0`, 73 lines a second
   apart). The Kismet half is complete: the action's `SetGoToStatus` fires `Success (Last)` / `Failed` / `Interrupted` and
   unlinks the controller, which is what the Empress's `DisSeqAct_AIGoToActor_13` chain waits on (FW).
3. **The relationship-override API is in** (§2): the disposition action writes the pawn's `FDisRelationshipOverrideInfo` and,
   reciprocally, the recipient's; the global faction manager's rows and cache clears are ported and guarded on an instance
   this tree publishes from `InitGlobalManagers`. On `DishonoredNPCPawn_25` the action wrote 1 individual override and the
   pawn raised `m_bNotifyAIOfRelationshipChange`; on `_21` an individual and a faction override.
4. **The NPC tick is ported as far as the look-at and the spine bender** (§3): `ADishonoredPawn::Tick` → `Tick_Body`,
   `ADishonoredNPCPawn::Tick` → `SetLookAtMode( ChooseLookAt() )` every frame, `Tick_Body` → `Tick_SpineBending` →
   `Tick_SpineBending_Apply`'s slope branch. **What it does not do is turn a head unprompted**, and the brief's premise that
   `SetLookAtMode(ChooseLookAt())` is "what asks" is the correction of this package (§5.1): the mode picks the look-at node's
   pose set; the *target* is a look-at request from the head-track sub-processes, three skeleton units.
5. **The pawn's Base** (§3.4): retail bases a standing pawn through `FullMovePawn`'s ground check, which agent DN's swept
   move never ran; it runs now (`UpdateFloorAndBase`). FV2's census (every NPC, every 5 s, 15 samples): **11 of 26 NPCs
   are based** (`_0 _2 _3 _4 _10 _11 _18 _24` on `WorldInfo_0` - BSP; `_17` on `SkeletalMeshActorMAT_3`), where the
   control bases none; 15 stay `none` with `floor 1.00`, so the sweep from their position hits nothing it may base on
   (static-mesh actors that are not world geometry are the likely floor - DN's `IsBlockedBy` policy, not checked here).
   The boat guard himself is FV3 (§4).
6. Two addresses this tree carried were wrong (`SetGoToStatus` 0x79a520, `GetCurrentTransitSpeed` 0x779850) and FT's two
   "mode 1 for a sword" lines were wrong - weapons answer EmptyIdle (§5).
7. Gate: see §6.
8. **GA overlaps this package in `ADishonoredPawn::Tick_Body` and `FArkComponentLocomotion::FullMovePawn`** - the functions
   are named per file in `build/agentFV_sync.py` so GA merges rather than overwrites (§7).

## 1. The go-to, in call order

`DisSeqAct_AIGoToActor` (a `UDisSeqAct_Latent`) → `USequenceAction::Activated` → handler `OnAIGoToActor` on the target's controller:

| | 2013 rva | 2012 | what |
|---|---|---|---|
| `ADishonoredNPCController::OnAIGoToActor` | 0x763890 | 0x7bd370 | 490 B in both. Dead → nothing. Abort input on the remembered action → `FAIStimStruct_BehaviorAbort( UDisBehaviorGoTo )` through `HandleAIStim`. Otherwise a different remembered action is told Interrupted, this one is remembered; with a destination: `m_bSetNewHomeActor` writes the brain's `m_Home`, an idle request then the go-to request through `HandleAIStim_Internal`, each only while `!CanBeDormant()`. **2013 calls `SetGoToStatus` through the action's vtable (+404)**; 2012 called it directly - the CppText declares it virtual |
| `UDisSeqAct_AIGoToActor::GetDestinationActor` | 0x78ae40 | 0x7cdc20 | a controller means its pawn |
| `::SetGoToStatus` | 0x7a1fc0 | 0x7d8da0 | any status but Out unlinks controller ↔ action; the output named by the status fires unless it is Success with controllers still walking - **output 1 is `Success (Last)`** |
| `::GetDesiredTransitSpeed` / `::ShouldAlignRotation` / `::DeActivated` | 0x786360 / 0x786370 / 0x7a2070 | 0x7cb780 / 0x7cb790 / 0x7d8e50 | Walk-or-Run; `m_bAlignRotation`; `UDisSeqAct_Latent::CleanUpLatentActors` (2012 0x7d8be0, unnamed in 2013) |
| `UDisBehaviorGoTo::EvaluateGoToRequest` | 0x6f0700 | 0x75d2a0 | wanted when the action's `m_bInterruptable` differs from the tweaks' `m_bRespondToUninterruptableStims` |
| `::SetupFromGoToRequest` | 0x6f0750 | 0x75d2f0 | destination (tracked actor or fixed snapshot), accurate-stop / exact-rotation from `UDisTweaks_AIBehavior_TakePosition`, align, speed, interruptable, the action |
| `UDisBehaviorGoTo_Base::PrepareGoTo` | 0x6e4de0 | 0x7455c0 | stage 0, face-to cleared, **one locomotion desire** (actor: threshold -1, funnel 1, accurate; fixed: the destination's accurate-stop bit) |
| `::FilterDestinationReached` | 0x6e4ea0 | 0x745690 | this behaviour's request and an exact arrival → `m_bHasReachedDestination` |
| `::TickBehavior` | 0x6edc00 | 0x74c8c0 | stage 0 + reached → `Start_TurningToFace` (0x6e8590: align → face-to yaw desire, stage 1, wait for `RotationReached`; else stage 2); stage 2 → `m_bIsFinished = m_bSuccess = TRUE`, stage 3 |
| `IsBehaviorFinished` | 0xae4510 | 0x724a60 | **not a function of this class in either database**: UObject vtable slot 92 of `??_7UDisBehaviorGoTo_Base@@6BUObject@@@` (2012 0xd25e60) is `UDisBehaviorShoot::IsBehaviorFinished`, `return *((DWORD*)this+41) & 1` - both classes keep their finished bit at offset 164 bit 0, ICF folded them. The brain's `CallTickBehavior` reads it and pops the behaviour |
| `::OnBehaviorStop` | 0x6e4d60 | 0x745560 | `SetGoToActionStatus( m_bSuccess ? Success : Interrupted, action )` |
| `::FilterPathingFail` | 0x6e8600 | 0x746db0 | retail tries `AttemptTeleportSpell` (2012 0x832a50, not ported) and reports Failed when refused; this reports Failed at once and says so once |

The masks are the names in the delegate accessors: evaluate `{GoToRequest}`, finish-while-dormant `{GoToRequest, BehaviorAbort}`,
filter the base's three (`DestinationReached, RotationReached, PathingFail`) plus the nineteen the switch at 2012 0x760eb0
names (eight interrupt stims, the abort, two distractions swallowed with the always-true delegate, four Kismet AI actions,
the go-to, three touches). Retail's 2012 mask (0x746bc0) sets 22 bits; the list has 22 names. Stim ids: `EAIStimID_GoToRequest`
is **41 in 2012 and 42 in 2013** (the 2013 constructor the named database labels `FAIStimStruct_GuardRequest::FAIStimStruct_GuardRequest`
at 0x7186c0 writes 42 - a mislabel; our enum is 2013's).

`UDisBehaviorGoTo::GetResumingBodyIntentionDesire` (2012 0x748e90) is read off the disassembly: `this` is the `IDisDesiresInterface`
sub-object at +156, so `[this-0x60]` is `m_pOwningBrain`, `[brain+0x41]` `m_SuspicionLevel`, `[brain+0x15c]+0x594` the pawn's
`m_pInventory`, `[this+5]` `m_TransitSpeed`. It needs `UDishonoredInventory::HasItemByClass` / `FindItemByClass`, and
`ChooseLookAt` needs `FindEquippableItem` - all three ported into `dishonoredinventory.cpp` from 2012 0x855bc0 / 0x852180 / 0x852060.

**Bring-up that stays named in the code**: `FArkComponentFaceTo` has no body (`Engine/Src/arkcomponentfaceto.cpp` is a skeleton), so
a go-to with `m_bAlignRotation` would wait on `RotationReached` forever; `Start_TurningToFace` goes to stage 2 directly when the
pawn has no face-to component and logs once. With the component in, retail's wait is the path taken. `UDisSteeringInfluence_Danger::EnableDanger`
(2012 0x765080) and `UDisAISubProcessPersonalSpace::SetAmbientActionManagement` (0x766240) are one bit each and written inline.

## 2. The disposition, in call order

`DisSeqAct_SetDisposition::Activated` (2013 0x790a50) remembers which of Set / Clear / Clear All fired → handler on each target:

* a pawn: `ADishonoredPawn::OnSetDisposition` (15 B) → **`IDisRelationshipInterface::OnSetDispositionHelper`** (0x70f700, 858 B): per
  recipient (a `UDisTweaks_Faction` or anything `FromUObjectOrController` 0x70f6a0 resolves - the player controller means the
  player pawn), Set writes `OverrideFactionRelationship` / `OverrideIndividualRelationship` on this side, clears the manager's
  cache for the pair and raises `OnRelationshipChange`; with `m_bReciprocate` a faction recipient gets the manager's
  `OverrideRelationToIndividual( faction, this )` and an individual recipient its own override of this. Clear and Clear All
  mirror it.
* a faction: `UDisTweaks_Faction::OnSetDisposition` (0x879000, 692 B) writes the manager's rows (`OverrideRelationToFaction` /
  `OverrideRelationToIndividual` / the clears / `m_GlobalRelationships.Remove` + `ClearFactionCache`).

The interface itself had no slots in this tree (`class IDisRelationshipInterface { virtual ~...; typedef ... }`). Retail's has six,
read off `??_7ADishonoredPawn@@6BIDisRelationshipInterface@@@` (2012 0xd1a050) and the NPC's (0xd39bf8): dtor,
`GetUObjectInterfaceDisRelationshipInterface` (0x71ce00), `GetRelationshipOverrideInfo` (0x765290, `lea eax,[this+5Ch]` - the
interface sits at 1208 in `ADishonoredPawn`, 1208+92 = 1300 = `m_PersonalRelationships`), a slot returning `[this+54h]` =
1292 = `m_pCurFactionTweak` (0x964ef0; its retail name is in neither database, this tree calls it `GetFactionTweak`),
`DetermineRelationship_Derived` (0x764c30, returns its argument), `OnRelationshipChange` (pawn: an empty body; NPC: 0x7c04e0).
FR's "slots 2 and 5" were these two. The four "unnamed `UDisTweaks_Faction` functions" of the brief are not faction functions:
`sub_C5E610` / `sub_C60C40` are `FDisRelationshipOverrideInfo::ClearFactionOverride` / `OverrideFactionRelationship`
(2012 0x89fb60 / 0x8a0670) and `sub_C63BB0` / `sub_C638E0` are `UDisGlobalFactionManager::ClearRelationToIndividualOverride` /
`OverrideRelationToIndividual` (0x8d7e60 / 0x8d7ba0) - all four ported.

**`UDisGlobalFactionManager::s_pInstance`**: retail's constructor (2012 0x8d8a30) publishes the one non-archetype instance and every
caller dereferences it unguarded. This tree had the class with only ED's `GameSave`/`GameLoad`; it has the game info's
`m_pGlobalFactionManager` sub-object, so `InitGlobalManagers` publishes it (where retail calls `RebuildCaches` on it) and every
use is guarded. Ported: the three `ClearIndividualCache`, three `ClearFactionCache`, two `ClearDerivedFactions`, the two
individual-override writers, the two faction-override writers, `CachedIsFallbackDerivedFrom` as the uncached
`UDisTweaksBase::IsFallbackDerivedFrom`. **Not ported, and the consequence**: the faction tables behind `DetermineRelationship`
(`IsTrespassing` 0x8cea70, `GetGlobalIndividualOverride` 0x8c7d40, `GetFactionRelationshipToIndividual` 0x8ceaf0) - every
faction answers Neutral and nothing is cached (a cached Neutral would outlive the tables); an individual override is honoured.
`UDishonoredGlobalAIManager::HandleRelationshipChanged` (0x8b10f0 / 0x8b1080) is named at its two call sites.
`ADishonoredNPCPawn::OnRelationshipChange` raises `m_bNotifyAIOfRelationshipChange` (agent CG's `ShouldAIBeNotifiedOfRelationshipChange`
reads it); `UpdateAvoidableCollisionGroupFlags` (`UArkAvoidable::SetGroupFlags`, no ported methods) and the game-over-when-hostile
check (`GetPlotSettings` / `DisGameOver`) are named.

## 3. The tick, the look-at mode, the spine bender, the Base

### 3.1 `ADishonoredPawn::Tick` (2013 0x750720, 2012 0x790c10)

Was not ported at all: `APawn::Tick` ran and none of retail's eight phases did (agent BF's note in `dishonoredpawn_attributes.cpp`
said so for `Tick_Attributes`; it was true of all of them). Retail: `Tick_Begin`; `Tick_Water` when `m_bAllowTickPawnMinorSubsystems`
and `m_bWaterVolumesRelevant`; `Tick_Health`; `Tick_Attributes` when minor; `Tick_Inventory`; `Tick_Combat` when minor; **`Tick_Body`
when `m_bAllowTickBody`**; `m_bCachedAttnParamsValid = FALSE`; `Tick_Stealth`. The bitfield at 1280 was read by name off the
generated header (bit 7 `m_bAllowTickPawnMinorSubsystems`, bit 8 `m_bAllowTickBody`, bit 6 `m_bCachedAttnParamsValid`, bit 0
`m_bWaterVolumesRelevant`). Only `Tick_Body` is called; the seven others are named in the comment (BF's `Tick_Attributes` reason
stands). `Tick_Body` (0x7510b0) runs `Tick_Body_CachedPositions` (0x74d850, the position-prediction pair, ported) unless the
retargeting component says the pawn is being retargeted - `FDisComponentRetargeting` is absent and `m_pCpntRetargeting` NULL, read
as "not retargeting".

### 3.2 `ADishonoredNPCPawn::Tick` (0x76da40, 1085 B) and `Tick_Body` (0x783380)

Ported in retail's order with nine callees named and not run (`Tick_Rotation`, `CheckForVanish`, `CollideContactSystemEnd`,
`UpdateMeshOriginOffset`, `UpdateEquipment`, `UpdateTetherVolume`, `SeverLimb` + `DropAttachedItems`, the ragdoll impulse flush,
`Tick_NPCFootPlacement`, `RegisterThreat`); what runs: `m_bNoTick`, the master FSM's `TickStateMachine` (it exists and was
never ticked before), the sleep-damage countdown, the two deferred arrays emptied, `UpdateLocomotion`, **`GetComponentLookat()->SetLookAtMode( ChooseLookAt() )`**,
`m_bLastRenderedByPlayer = m_bRenderedByPlayer`, `bJustTeleported` cleared on `PHYS_None`. The NPC bitfield at 2180 (2013: 2404)
was mapped by declared order: 0x4 `m_bLastNeedSlopeOffset`, 0x10 `m_bTickedNPCBody`, 0x20 `m_bRenderedByPlayer`, 0x40
`m_bLastRenderedByPlayer`, 0x80 `m_bUseSpineBendingTarget`, 0x100 `m_bWasRecentlyPossessed`, 0x1000 `m_bAllowFootPlacement`,
0x2000 `m_bNoTick`. The AActor word at 296 bit 12 is `bJustTeleported`, the word at 288 bit 3 `bDeleteMe`;
`0x2000000000000000` is `RF_PendingKill`.

`ChooseLookAt` (0x7794d0, 475 B) is ported whole; it needs `GetCurrentTransitSpeed` (0x779430: `FArkComponentLocomotion::GetMostRelevantSpeedIdx`
through `m_LocomotionSpeedToTransitSpeed` - DN's component has the accessor) and the item's `GetDefaultNPCLookAtMode`, a new
virtual on `UDishonoredInventoryItem` (UObject vtable slot 85: the base body 0xa6cca0 is `return 0`, **and `UDishonoredWeapon`'s and
`UDisWepMelee`'s tables carry the same body**; only `UDishonoredItemEmpty` 0x887cf0, `UDisWeapon_WolfhoundBite` 0x886d00 and
`UDisWepMusicBox` 0x8993f0 override it).

### 3.3 The spine bender's driver

`Tick_SpineBending` (0x77fbd0, the enable decision, ported whole), `IsActionSpineBendingEnabled` (0x7772c0), `IsSlopeOffsettingEnabled`
(0x77b300), `IsFootPlacementEnabled` (2012 0x7b2a70), `Tick_SpineBending_Apply` (0x77b3d0, 2006 B): the slope branch is ported
(`cos(90 - m_fMaxSlopeAngle)`, the floor against the mesh's up axis, `SetBendPitchTarget( ±m_fMaxSlopeBend*, !m_bLastRenderedByPlayer,
&m_fSlopeBendPitchBlendSpeed )`); the action branch needs the hit-region focus bones (DI's gap) and is unreached (`SetSpineBendingTarget`
has no caller); the turn branch needs `ComputeTurnAngle` (0x76fe70, 1,321 B) and logs once per pawn. **Two gates the brief did not
know**: `IsSlopeOffsettingEnabled` requires `IsFootPlacementEnabled`, which requires `m_bRenderedByPlayer`, and nothing in this tree
sets that bit (retail sets it from the render path); and the body tweaks' `m_bIsSlopeOffsettingEnabled`. So the bender is enabled
instantly and bent by the slope branch only once the render flag is real - until then `Tick_SpineBending` takes the "neither"
branch and disables it, which is retail's answer for an unrendered pawn.

### 3.4 The pawn's Base on the boat (FS §6.1)

Retail's `FArkComponentLocomotion::FullMovePawn` (2012 0x580ae0, 4,707 B; decompiled whole) ends with a ground check: the cylinder swept
from `MaxStepHeight` above the location to twice it below, trace flags 0x1020209F, then `if( Hit.Actor == Base || !IsBlockedBy(
Hit.Actor, Hit.Component ) ) Floor = Hit.Normal; else { SetBase( Hit.Actor, Hit.Normal, TRUE ); if( !bHaveBase ) RelativeLocation =
Location - Base->Location; }` - and it runs **even for a pawn that is not moving when it has no valid base** (`bFallAllowed || !_bHaveBase`).
Agent DN's `FullMovePawn` is `UWorld::MoveActor` and returns on a zero move before any ground check, so no NPC was ever based
on anything (the `base none` FS measured). `UpdateFloorAndBase` is that ground check, called on the zero-move path when the pawn
has no valid base and after every move. A pawn based on the boat is then carried by `UWorld::MoveActor`'s attached-actor update
when the Move track moves the boat. Measured in §4 (FV2's base census).

## 4. The runs

All: `fv_run.py` - the d3d9 stage's configuration (`-startmap=L_Tower_P -startmapopen -ResX=1280 -ResY=720 -nomovie -nodishud`) plus
`-disai` and `-fvdiag`, `-windowed`, `game_input.py launch` + `wait 75 / shot / quit`. Logs in the retail install's `DishonoredGame/Logs`.

| run | exe | result |
|---|---|---|
| FVBASE1 | `build/head_rel` (`da77f0f`) as `DishonoredGame_FVbase.exe` | natives fire at 29.4 s (`execOnSetDisposition`, `execOnAIGoToActor`), `Kismet: Name of the buggy pawn : DishonoredNPCPawn_24`; `disai loco: requests 26 ... 11 arrivals ... 304.7 uu moved`; `locomoved: 3 of 26 NPCs moved: _15=269 _12=36 _17=15`. Capture `apshottime_fvbase100000.bmp` 1008x567, 214 distinct sampled values |
| FV1 | `build/agentFV_rel` as `DishonoredGame_FV.exe` | **0 `native not ported` on the walking path** (Steamworks' `execExit` at quit only), 0 criticals, 198 frames. 9.12 s: `fvdiag relationship changed on DishonoredNPCPawn_25 (1 individual override)`, `fvdiag goto DishonoredNPCPawn_25 action ...DisSeqAct_AIGoToActor_2 -> DishonoredNavPoint_22 at 16833/21686/3221 (pawn at 16744/21686/3261, dist 89) track 1 home 1 align 1 speed 0 interruptable 1 dormant 0`, `goto prepare ... speed 1 accurate 1 align 1`; `_21`: an individual and a faction override. `disai loco: requests 27 start 0 update 1 stop; 11 arrivals; 360.0 uu moved`; `locomoved: 3 of 26: _15=262 **_25=29** _17=15`; `loco trace move_enter: move X=0.140 from X=16743.787 Y=21685.605` (the go-to pawn is the run's first mover). No arrival stim; at quit `goto stop DishonoredNPCPawn_25 ... Interrupted`. Capture `apshottime_fv100000.bmp` 1280x720, 225 distinct |
| FV2 | the gate's `build/agentFV_reg` exe | see below |

FV2: 83.6 s, 0 criticals, `execExit` the only unported native, capture `apshottime_fv200000.bmp` 1280x720 (224 distinct
sampled values). The go-to: `goto prepare` at 9.79 s; the per-second line reads `path -1/0 nopath pathend 27397` at 9.79 s,
then from 11.33 s to 82.94 s unchanged: `stage 0 reached 0 at 16796/21686/3261 dist 37 base none vel 0 | loco req 1 path 1/2
arrived pathend 37 speed 0/0 stuck 0`; no `destination-reached stim` line at all; `goto stop ... Interrupted` at quit.
`disai stims` counts `EAIStimID_DestinationReached(24)=3` for the whole run, none of them for this pawn; `disai loco ...
13 teleports onto the mesh` (control 12), `locomoved ... _25=5`. The base census is in §0.5.

FV3 (the Boat scene, FS's FSBASE3 route: `-inputtest -distrace -distouch -diskismet -distouchprobe` added, wait 115 s): 123 s, 0 criticals, capture `apshottime_fv300000.bmp` 1280x720. `StartSpawn DisSeqAct_StartSpawn_0` at 44.84 s as in FS's
FSBASE3; the Elite is `DishonoredNPCPawn_26`, sampled 15 times from 49.06 s to 119.19 s: **`on none at -2934/17434/-764 vel
0/0/0 physics 1 floor 0.00`, every sample identical** - FS's `base none` stands. `floor 0.00` is the tell: every other NPC
reads `floor 1.00`, so `UWorld::MoveActor` and the ground check never ran for this pawn at all - `ADishonoredNPCPawn::physWalking`
(which drives `MovePawn` → `FullMovePawn` → `UpdateFloorAndBase`) is not reached for the matinee's AI-group pawn, while it is
for the 26 level NPCs. The ground check is in the right place for a pawn that walks (11 of 26 based in FV2 against 0 before);
the boat's guard needs whatever retail ticks a matinee AI-group pawn's locomotion with (retail's `UInterpGroupInst::UpdateGroupInst`
step: `DisableInertia`, `MovePawn( step )` per step - FS ported only the skip-path version, `DisFSSkipStep`, which `MovePawn`
now has a body for). **Not fixed here**: both builds were spent before FV3 could be measured.

## 5. Premises of the brief, corrected

1. **`SetLookAtMode( ChooseLookAt() )` is not what makes a guard turn his head toward the player.** The mode selects which of the
   `UArkComponentLookatConfig::m_LookAtModes` the node uses (EmptyIdle / EmptyWalk / ...). The *target* is a `FDisLookAtRequest`
   (`IDisDesiresInterface::SetLookAtActorDesire`), raised by the head-track sub-processes `UDisAISubProcessHeadTrackBase / Bark /
   Pause` - three units on the exclude list - or by a sub-state (`UDisAISubStateTakePosition` already does for its focus target).
   `-flheaddiag` cannot show an unprompted turn until one of those is ported; the tick now runs the retail line that FT's §6
   named, and that line was never the asker.
2. FT §6: "wrong for one with a sword (mode 1)". Retail's weapons answer `EmptyIdle` (UObject vtable slot 85 of `UDishonoredWeapon`
   0xd72ae0 and `UDisWepMelee` 0xd72cd8 is 0xa6cca0, `return 0`); only empty hands, the wolfhound's bite and the music box differ.
3. "Four unnamed `UDisTweaks_Faction` functions" are two `FDisRelationshipOverrideInfo` and two `UDisGlobalFactionManager` functions (§2).
4. `UDisBehaviorGoTo_Base::IsBehaviorFinished` exists in neither database because ICF folded it onto `UDisBehaviorShoot`'s (§1).
5. `UDisSeqAct_AIGoToActor::SetGoToStatus` is "2013 rva 0x79a520" in agent CG's note - it is 0x7a1fc0, and virtual in 2013.
6. `GetCurrentTransitSpeed` is "2013 rva 0x779850" in agent DN's note and the brain's - it is 0x779430; and
   `FArkComponentLocomotion::GetMostRelevantSpeedIdx` does exist in DN's component (the brain's `GetCurrentDesiredTransitSpeed`
   says it does not).
7. The wave-22 plan says `ADishonoredPawn::Tick_Body` is "90 code lines, partial" - it had no body; neither did `ADishonoredPawn::Tick`
   or `ADishonoredNPCPawn::Tick`. Every Dishonored tick phase was unported.
8. The go-to's pawn is named by spawn order: `DishonoredNPCPawn_24` in FR's route (menu, `-finewgame`), `_25` in the d3d9 stage's.
9. `EAIStimID_GoToRequest` is 41 in 2012 and 42 in 2013; the 2013 named database labels the go-to request's constructor
   `FAIStimStruct_GuardRequest::FAIStimStruct_GuardRequest` (0x7186c0).
10. The brief's "the pawn's Base ... Spawner/basing path": the spawner is not where retail bases a pawn; the locomotion
    component's ground check is (§3.4) - and for the boat's guard that check is never reached because the matinee pawn's
    `physWalking` never runs (§4, FV3).
11. "`DisNotify_NPCDisableLookAt` needs `Notify_EndRange` if a scene reaches it": no scene reaches it - the class fails at
    package load: `Error, *** Found bad AnimNotify in Npc_CityGuard_Generic_as...:AnimSequence_0 : DisNotify_NPCDisableLookAt`,
    143 lines in FV2 and 143 in FV3 at 5.7 s (load, not play), every guard anim set. The notify class is not a loadable class
    in this tree, so no `Notify*` on it can be called; that is the fix, before `Notify_EndRange`.

## 6. Acceptance

`run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentFV_reg --newgame-at 50`, run from the worktree
copy, its own in-harness build (the package's second and last build, half CPU, one at a time): **47 ok, 0 failed, 0 skipped,
4796 s**; `unported_natives 0 <= 0`; `build_DishonoredGame_errors 0`; no C4263 / C4264 in the gate log; `layout_mismatching 0`,
`layout_types 2314`; `d3d9_criticals 0`, `inputtest_criticals 0`, `newgame_on_mission 120`, `options_criticals 0`.
`resources/docs/regression_baseline.json`: `unported_natives` back to `<= 0` with the history line (FV: the two natives of
the Kismet handlers). Sync: `python build/agentFV_sync.py` - 46 files clean, 0 flagged, 0 missing (= 34 changed + 12 new
under `source/` and `resources/`; README.md / PHASE18.md are main's `f14326b` and excluded).

## 7. Files, and the two seams with GA

`build/agentFV_sync.py` names every file and every function. **GA**: `ADishonoredPawn::Tick_Body` in `dishonoredpawn_body.cpp` is
this package's (the cached-positions call and the retargeting test); GA's root-motion application goes beside it - merge, do not
overwrite; `ADishonoredPawn::Tick` in `dishonoredpawn.cpp` is the dispatcher GA will also want (`Tick_Body` gated on `m_bAllowTickBody`).
`FArkComponentLocomotion::FullMovePawn` in `arkcomponentlocomotiondynamic.cpp` carries this package's two `UpdateFloorAndBase` calls;
GA's `MovePawn` work lands in the same function. `Inc/arkcomponentlocomotion.h`: `UpdateFloorAndBase`, `IsStuckInAWall` added.
