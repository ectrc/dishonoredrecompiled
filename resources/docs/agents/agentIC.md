# Agent IC: the soirée stim, the controller list, FaceTo (wave 26, PHASE23 package IC)

Worktree `build/agentIC_wt`, made at main `97349ae` (= `63c3c91` plus PHASE23 lines); both builds are on that tree. After build 2 it
was fast-forwarded to `fec772f` and then to `f90db27` (= IK's `8dcbd03` + docs): IK's `arkcomponentlocomotion.cpp` and
`dishonorednpcpawn_locomotion.cpp` merged with this package's hunks without conflict (git stash / checkout / pop), the generated
`dishonoredgameclasses.h` / `Sources.cmake` regenerated (79/79 identical, `build/agentIC/regen3`; audit 0). **The merged tree is not
compiled** - the coordinator's message about IK arrived after build 2; the overlap is two disjoint hunks (SetConfig / Starting, and
one line in SpawnedLocomotion) and arkcomponentlocomotionpath.cpp is untouched.
Build dir `build/agentIC` (RelWithDebInfo). No commits. Staged exes: `DishonoredGame_IC1.exe` (build 1), `DishonoredGame_IC2.exe` =
`DishonoredGame_IC.exe` (build 2, the hand-back tree). Two builds; build 2's first invocation stopped on one compile error (an enum
declared twice, `build/agentIC_build2.log`), fixed and resumed (`agentIC_build2b.log`), as HC's build 1. 0 errors, 0 C4263/C4264 in
all logs. Every run went through `cutscene_compare.py run` (= `game_input.py launch`, `-windowed -nomovie`, ini isolation).
Decompiles: `build/agentIC/decomp2012/` (headless, PDB names) and `build/agentIC/decomp2013/` (headless, `<va>.c` + vtable dumps
`vt_*.txt`); IDA on private copies in `build/agentIC/idb/`. Never fmodel.

## 0. The answer

1. **The soirée behaviour and the stim are ported and work** (2013 bodies; §1). With the controller list filled
   (`-hbcontrollers`, IC1g / IC2f): every NPC a matinee binds gets the start stim; the 12 party NPCs bound by non-blocking matinees
   at world 51.03 go `DisBehaviorIdle/Patrol -> DisBehaviorSoiree` (slot 2 or 4 by priority); the boat's three (`_26`, `_27`, `_16`)
   get `Start ... blocking 1` at 56.2 and the Stop stim at the ride end (150.45); after the Stop `_26` goes `Idle -> GoTo` (the
   Kismet go-to) and `_27` `Idle -> Soiree` (the dock matinee SeqAct_Interp_0 binds it, non-blocking, in the same frame).
2. **The brief's premise is wrong: the soirée stim does not keep the game alive with the controller list on.** The boat matinee is
   *blocking* (`m_bIsBlocking 1` on all three boat stims), and `UDisBehaviorSoiree::EvaluateSoiree` (retail 2013 0xafb200, 2012
   0x7420b0, byte-identical) refuses blocking stims - in retail too, nothing takes them. With the list filled the render thread
   dies in `FParticleSystemSceneProxy` 0.1-1.4 s after the boat binds in **6 of 6 runs** (IC1a, IC1b, IC1e, IC1f, IC2b, and IC1d
   with `-icnostim`); with `-icnosoireeattn` (the list filled, the soirée attention `UpdateSoireeAttention` not holding the co-actors
   at Busted) it runs to world 205 in **2 of 2** (IC1g, IC2f). Retail's `UpdateSoireeAttention` (2013 0xb3f120) is ours line for
   line (min-attention type 5, level 4 on every other ADishonoredPawn of the pawn's matinees), so the trigger is real; what it
   breaks downstream is not found (§3). **`MatineeControllerAdd` therefore stays OFF by default** (agent HB's switch
   `-hbcontrollers` turns it and the stim on together); the acceptance "5/5 ride runs with controllers on" is not met.
3. `UDishonoredEngine::Init` now sets `SetFaceToPriority(15)`, `SetLookatPriority(1, 5, 19)` (retail order with the Locomotion one):
   the LockRequests look-forwards run at priority 1 / 5 (IC1g `fkdiag lookattrack ... prio 1`, `prio 5`; HB measured 0).
4. **FArkComponentFaceTo is ported whole** (ctor, Initialize, the request API, the private update chain, PreAsyncWorkTick, the
   request-manager events, `ArkSmoothFixAngleRotation`), created in `FArkComponentLocomotion::SetConfig`, started with it, cached by
   `Spawned`; the rotation intents start / update / stop real requests. It is behind **`-icfaceto` (default off)**: the pawn's yaw is
   applied by retail's `FArkComponentLocomotion::FullMovePawn` (2012 0x580ae0: `NewRotation.Yaw = Rotation.Yaw + m_CurBodyYawDelta`,
   and the move runs when only the yaw changed), which is package IK's function, and the locomotion component's own face-to
   requests (`m_FaceToRequestID`, `IsLocoFaceToRequestActive`) are unported. IC2f (`-icfaceto`): the only face-to requests on this
   route are `StartRetargeting`'s Lock/Cancel pairs on `_17`, `_27`, `_26` (start + stop in the same frame, 6 lines); Worker B
   `_3` still reaches 11633/22105. `UInterpTrackInstFaceTo::UpdateTrackInst` is **not** ported (§4) - it is the next package with
   the FullMovePawn yaw.
5. Default build 2 (controllers off, FaceTo off): ride SSIM **0.738** vs IB2's 0.728 (MAD 10.57 -> 10.40, zoom 0.99), 0 Criticals,
   `Failed, with animsequence` 0, one `BlendNow 0.20` per binding.

## 1. What was ported

| function | 2012 | 2013 (VA) | where |
|---|---|---|---|
| `UDisBehaviorSoiree::OnBehaviorStart` (clears m_bShouldForceFinish, m_PendingBark = DDH_INVALID) | 0x724ac0 | 0xae4590 (vtable 88) | disbehaviorsoiree.cpp |
| `OnBehaviorStop` (NPC-soiree game event 23 ESE_Stop, foot placement on, ExitFromMatinees(guid)) | 0x728f40 | 0xae78f0 | same |
| `OnBehaviorResume` (event 23 ESE_Start) | 0x741ea0 | 0xafafd0 | same |
| `TickBehavior` (2013 only: the held bark fires once the pawn is in UStateNPCMasterWalk) | - | 0xaf4d20 (vtable 96) | same |
| `IsBehaviorFinished` / `BehaviorShouldFinish` (forced, or no non-blocking controller at the tweaks' priority) | 0x743190 / 0x742110 | 0xafd930 / 0xafb260 | same |
| `CanBlockSoiree`, `CanBeSurprised`, `GetSoireePriority` | 0x7421c0, 0x742210, 0x73fb00 | 0xafb310, 0xafb360, 0xaf4d90 | same |
| `BuildEvaluateStimMask` (Soiree), `BuildFilterStimMask` (19 stims, read off 2013 byte offsets), `BuildBehaviorFilterStimMasks` | 0x724ba0, 0x724ad0, 0x728fd0 | 0xae7a50 (80), 0xae7980 (82), 0xaef6d0 | same |
| `GetEvaluateStimDelegate`, `GetSetupFromStimDelegate`, `GetFilterStimDelegate` (2013 table: NoticeBegin/SearchBegin gone, TopAttnProxyUpdated, ActorTamperedWith / DeathByWoL / ReactionRequest hold a bark) | 0x743b50, 0x72c340, 0x7448e0 | 0xafee20, 0xae9f90, 0xaff360 | same |
| `EvaluateSoiree`, `SetupFromSoiree`, `FilterSoiree`, `FilterGoToRequest`, `FilterBlockingDialogRequest`, `FilterBlockAtHighPriority`, `FilterFinishAtLowPriority`, `FilterWitnessMagic` | 0x7420b0 ... 0x7430e0 | 0xafb200, 0xae45b0, 0xafb050, 0xafb0d0, 0xae45a0, 0xafb120, 0xafb180, 0xafd880 | same |
| 2013 thunk targets: ActorTamperedWith 0xafd840, DeathByWoL 0xafd860, ReactionRequest 0xafd810, TopAttnProxyUpdated 0xafb1e0; FilterReactions (UDisBehaviorBlockingDialog 0xac25e0, bound by fold) | | | same |
| `GetResumingBodyIntentionDesire` (fold of BlockingDialog's 0xac2700) + `DisContinuePreviousBodyIntention` | 0x827500 | 0xbbf7a0 | same; dishonoredutilities_ai.cpp |
| FAIStimStruct_Soiree start / stop in `MAT_BeginAIGroup` / `MAT_FinishAIGroup` (ctor 0xb19470: id 95, guid, event, priority, blocking; stop = ESE_Stop, ESAP_MAX, not blocking, before MatineeControllerRemove) | 0x7c0570 / 0x7c0830 | 0xb65080 / 0xb65350 | dishonorednpcpawn.cpp |
| `GetMatineeControllerCount` | 0x7b1a60 | 0xb53570 | same |
| `ExitFromMatinees(FGuid)`, `ExitFromMatinees(priority)`, `StopMatinees` (TermGroupInst(TRUE) unless CanInterruptMatinee, MAT_Detach on bShouldBeDetached, else StopFromNPC = Stop() unless m_bShouldNotBeInterruptedByDialog 0x612a60) | 0x7bdfd0, 0x7bde30, 0x7b6b40 | 0xb65940, 0xb657a0, 0xb5a120 | same |
| `MAT_ShouldForceInventory`, `MAT_GetBodyIntention` | 0x7ad720, 0x7ad740 | | same |
| `UDishonoredEngine::Init`: SetFaceToPriority(15), SetLookatPriority(1, 5, 19) | 0x65d280 | | dishonoredengine.cpp |
| `FArkComponentFaceTo` ctor, Initialize, GetFaceToComponent, Starting, Stopping, OnLODChanged | 0x5795d0, 0x575400, 0x577860, 0x57dc00, 0x57dcc0, 0x575430 | 0x938a90, -, 0x9365b0, 0x93d4f0, -, 0x9339a0 | arkcomponentfaceto.cpp (NEW) |
| StartFaceTo{Actor,Location,Yaw}, UpdateFaceTo{...}, StopFaceTo, StopAllFaceToFromAsker, GetActiveRequestID / Priority, GetRequestsCount | 0x577880-0x5779e0, 0x5797b0-0x579960, 0x577a20, 0x575b20 | 0x9365d0-0x936770, 0x938c70-0x938e20, 0x934290 | same |
| UpdateCurYawAndRotationSpeed, ResetCurRequestWorkData, FindTargetYaw, UpdateFaceToRequest, SendFaceToEvent, HandleOrientationReachedEvent, OnRequestManagerEvent, CanKeepSnapped, IsRequestHas{Infinite,Default} | 0x577a60-0x577f40, 0x5799c0, 0x579b60, 0x574990, 0x574930/60 | 0x9367b0-0x936c90, 0x938e80, 0x939020, 0x933030, 0x932fd0/0x933000 | same |
| PreAsyncWorkTick (1,151 B both) | 0x57ab20 | 0x93a160 | same |
| AllowRootMotion, DisableRootMotion, AllowTurnControl, DisableTurnControl, ForceBodySnapOnMovement, SetRealVelocity, HandleTeleport, the prop InitAllByDefaultExcept x3 | 0x579970 ... 0x574aa0 | 0x938e30 ... 0x933160 | same, arkcomponentfaceto.h |
| `ArkSmoothFixAngleRotation` | 0xafbd0 | | same |
| created in `FArkComponentLocomotion::SetConfig`, started in `Starting`; `m_pCpntFaceTo` in Spawned; `GetComponentFaceTo` | 0x584010, 0x58b750, 0x7b0a60, 0x7ab8d0 | | arkcomponentlocomotion.cpp, dishonorednpcpawn_locomotion.cpp, dishonorednpcpawn_rotation.cpp |
| rotation intents: SetTargetRotation starts / updates the location request, UnlockNPCRotation / CancelAllNPCRotations stop it | 0x7cbee0, 0x7ce540, 0x7c95a0 | 0xb714c0, 0xb74a00, 0xb6e920 | dishonorednpcpawn_rotation.cpp |

Named deviations (also in the file banners): the soirée `GetDialogAvailability` (0xafb2c0) has no slot in this tree's behaviour
base; FilterReactions is a Soiree member (retail binds BlockingDialog's folded body); ExitFromMatinees / MAT_ queries are plain members
(retail virtual, nothing overrides); TMemStackArray -> TArray; a pawn group that is not UInterpGroupAI is skipped in StopMatinees
(retail reads through NULL); the FaceTo request manager is the shared template (retail's FaceTo `Update` keeps a one-frame request one
more frame and double-decrements the timed count - every face-to request here is untimed); `ESoireeAIPriority` declared in
dishonoredutilities_ai.h (script enum without a C++ declaration); `EArkCpntFaceToEvent` stays in disdesirestructs.h (agent DF's).

## 2. Diagnostics

- `-icdiag`: `icdiag stim <pawn> <EAIStimID> (id) [event prio blocking guid] n | stack <slot:class*>` (every soirée stim; any other
  (brain, stim) its first 5 times and every 100th), `icdiag behavior <pawn> <old> -> <new> | stack`, `icdiag controller <pawn>
  add|remove`, `icdiag soireestim <pawn> send Start|Stop`, `icdiag soiree <pawn> resume|stop`, `icdiag exitmatinee`,
  `icdiag faceto <pawn> start|stop req prio type asker speed accel` and once a second per pawn with a request
  `icdiag faceto <pawn> req ... | pawn yaw body target delta rotspeed snapped reached`.
- `-icpsysdiag` (Engine/Src/UnParticleComponents.cpp): every UParticleSystemComponent BeginDestroy / Attach / Detach /
  ResetParticles / SetTemplate / ActivateSystem / DeactivateSystem / KillParticlesForced after the first blocking soirée start.
- A/B switches: `-hbcontrollers` (list + stim on), `-icnostim`, `-icnosoireeattn`, `-icfaceto`.

## 3. The crash, measured

| run | exe | switches | result |
|---|---|---|---|
| IC1a, IC1b | IC1 | list on, stim on | Critical at world 56.3 (+0.9 s): `FParticleSystemSceneProxy::ReleaseRenderThreadResources` <- `UpdateData_RenderThread` |
| IC1d | IC1 | list on, `-icnostim` | same crash |
| IC1c | IC1 | list on, `-icnosoireeattn` | clean to world 80 |
| IC1e, IC1f | IC1 | list on | crash; IC1f sampled (`build/agentIC/IC1f_samples.txt`, `tools/sample_on.py`): the game thread is in `FEngineLoop::Tick` waiting on the render fence; the render thread is in `CreateMiniDump` from the second sample on - the game thread is not stuck, it waits for a dead render thread (HB's "game thread stalled" is that wait) |
| IC1g | IC1 | list on, `-icnosoireeattn` | clean to world 205; the Start/Stop/behaviour table of §0.1 |
| IC2b | IC2 | list on, `-icpsysdiag` | crash at +1.4 s, this time in `FParticleSystemSceneProxy::DrawDynamicElements`; **0** particle-component lifecycle calls between the boat bind and the crash |
| IC2f | IC2 | list on, `-icnosoireeattn -icfaceto` | clean to world 205 |

Reading: the co-actors of the blocking boat matinee (the player included) held at Busted by every boat NPC is what kills the render
thread, and it does so without a single particle component being activated, reset or detached on the game thread - a stale
`DynamicData` (two different render entry points) points at memory the game thread changed under it. The suspects for the next
package: a heap overwrite on the attention path the player target takes (`m_pPlayerAttentionInfo` is a UDisAttentionInfo_Complex,
1644 B asserted; `ADishonoredPlayerPawn::OnAttentionReceived` / the HUD's walk of `m_pAttentiveBrainsList`), confirmed with a
debug-heap run; HB's crash and this one are the same.

## 4. Where the brief was wrong, with the measurement

1. "The soirée stim keeps the brain quiet so the list can be on": the boat matinee is blocking, the soirée behaviour never takes a
   blocking stim (retail's EvaluateSoiree), and `-icnostim` crashes the same way (§3). The list stays off by default.
2. "`HandleAIStim_Internal<FAIStimStruct_SoireeRejected>`": its only caller is `ADishonoredNPCPawn::MAT_IsAllowed` (2013 0xb65560,
   2012 0x7c0c20), and this tree's Engine calls no `MAT_IsAllowed` anywhere - the instantiation exists (the template) but nothing
   sends the stim. Porting MAT_IsAllowed changes which NPCs a matinee may bind; not done.
3. "An NPC with a rotation intent (gadiag rot) turns toward it within 2 s": on this route the only intents are `StartRetargeting`'s
   Lock + Cancel in one frame (IC2f: 6 face-to lines, 3 pawns, 0 `gadiag rot` lines; HC2 measured 0 too), and the yaw the component
   computes reaches the pawn only through FullMovePawn (IK's). Not measurable here.
4. "FaceTo fits the second build": the component did (compiled, ticked, requests logged), but turning the pawn is FullMovePawn's and
   `UInterpTrackInstFaceTo::UpdateTrackInst` needs an Engine override + an Engine->DishonoredGame hook (`GDisLookAtTrackUpdateHook`'s
   pattern; the class declares no UpdateTrackInst today). Decompiles for the next package: 2012 `UInterpTrackInstFaceTo_UpdateTrackInst_53eb90.c`,
   `..._TermTrackInst_53bb30.c`, 2013 `8ff8f0.c`, `8fc9a0.c`; FullMovePawn's head in IB's `d12/580ae0_*`.
5. "2013 instantiations near 0x720420": `HandleAIStim_Internal<FAIStimStruct_Soiree>` is 2013 VA **0xb5af50** and `<SoireeRejected>`
   0xb5afe0 (rva 0x75af50 / 0x75afe0); UDisBehaviorSoiree's 2013 bodies are 0xae4590-0xaff360.
6. 2013 is not 2012 for this behaviour: the bark is held (m_PendingBark) and fired by a 2013-only TickBehavior; NoticeBegin and
   SearchBegin left the filter list; TopAttnProxyUpdated was added. Ported from 2013.

## 5. Acceptance, as measured

| check | result |
|---|---|
| 5/5 ride runs to world 200 with controllers on, 0 Criticals | **not met**: 0/6 with the soirée attention on (§3); 2/2 with `-icnosoireeattn` (IC1g, IC2f) |
| default build 2 ride runs to world 200 (quit 205) | IC2a, IC2c, IC2d, IC2e: 4/4, 0 Criticals, `Failed, with animsequence` 0; plus IC2f (list on, `-icnosoireeattn -icfaceto`) and IC1g (list on, `-icnosoireeattn`) clean |
| Start/Stop stim per ride NPC, behaviour change after Stop | IC1g: Start for _26/_27/_16 at 56.19, Stop at 150.45; _26 Idle -> GoTo, _27 Idle -> Soiree (dock matinee) |
| one BlendNow 0.20 per NPC | IC2a (default): every binding blends once (14 NPCs 1x, the boat pair 2-3x = bind + blend-out + dock bind); IC1g (list on): first-controller rule, party NPCs 1x |
| "Failed, with animsequence" | 0 (IC1a, IC1c, IC1g, IC2a) |
| ride SSIM >= .72 vs build/cmp/IB2 | **0.738** (IB2 0.728), MAD 10.40, zoom 0.99 (IC2a) |
| gate d3d9 + inputtest + newgame (`build/agentIC/ic_regression.log`) | 21 ok, 2 load failures: `unported_natives` 0, `probe_natives` 4 / 3, every `*_criticals` 0, `newgame_pawns` 29, `d3d9_frames` 660 then 420 (d3d9 startup 25.6 s then 57.4 s: the machine was loaded), `inputtest_moved` 365.0 then 367.3. Alternation of inputtest alone: IC 405.0 / 369.3, IB exe (`DishonoredGame_IB.exe`, the pre-IC tree) 23534.7 (outlier) then 304.3, IC 317.0 - the metric follows the load, not the tree |
| generator / audit | 79/79 identical (`build/agentIC/regen`, and `regen2` after the fast-forward); bitfield_audit mismatches 0 |
