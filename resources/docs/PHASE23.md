# Wave 26 — the hand-played opening (`PHASE23.md`, packages IA..IG) + the Wwise program (IH)

**Status: started 2026-10-19 on the user's order ("execute this at 100% build power with no queue"); launch 1 = IA, IB, ID, IF, IH1 on `33d1c80`.**

## Context

Wave 25 (`PHASE22.md`, head `d6c3aa4`, gate 47/47) made the boat-ride cutscene match `retail.mp4` (ride MAD 29.4 → 11.6,
SSIM .523 → .696, zoom 1.76 → 0.99). The user then played the opening by hand on `DishonoredGame-Win64-Shipping.exe`
(= `head_rel`) and found four defects:

1. After the cinematic the FOV never returns to normal. User's reading: **in ours the ride FOV and the gameplay FOV
   look identical; in retail the ride looks different from gameplay.**
2. No voice lines, no subtitles.
3. NPCs after the cutscene look like they try to move but stay in place.
4. Player: no crouch, no sprint, no mantle (jump probably dead too); entering water = stuck forever.

User decisions (asked 2026-10-18): **real voice audio now, by a full reimplementation of the Wwise 2012.1 runtime
from the 2012 decompile (no SDK install)**; mantle as its own package; FOV as above.
This supersedes the "audio is polish / deferred to the last stage" rule for voice: audio becomes a program (IH)
running beside the game packages. Update the memory file `feedback-audio-is-polish.md` when this plan starts.

Base: HEAD `2673c4c` (= `d6c3aa4` + docs). The working tree carries the user's uncommitted SLOMO/GHOST edits in
`Engine/Src/UnPlayer.cpp` and `UnController.cpp`: packages branch from HEAD in their own worktrees; the coordinator
stages explicit paths only and merges `UnPlayer.cpp` through a HEAD-based blob (PHASE22 merge note).

Paths: `S` = `source/Development/Src`. Addresses: 2013 rva unless stated (VA = rva + 0x400000 in
`retail2013_named.i64`), 2012 in brackets.

## Root causes (read-only investigation, 2026-10-18)

**FOV.** Nothing narrows the FOV during the ride; `hfov` is 75° before/during/after (HD4.log). `TickFOV`
(`S/DishonoredGame/Src/dishonoredplayercamera.cpp:207-228`) moves `m_fCurFOV` toward the highest `m_FOVTargets`
entry else `DefaultFOV` (profile setting 118; `SetToDefaults` gives 75, `S/Engine/Src/arksettings.cpp:305`).
`SetFOVTarget/ClearFOVTarget` have one caller, `TickCameraTracking`
(`dishonoredplayercontroller_tracking.cpp:349-359`, flag 0x100 only; side bug `:344-345` `Min(0,..)`).
`Enter/ExitSoireeMode` (`dissoireecontrolstate.cpp:39-168`) never touch FOV. Candidates for the retail difference:
`UStatePlayerMasterBase::UpdateWalkNavAnimState` [0x69ab60] unported (`stateplayermasterbase.cpp:60-72`: sets the
locomotion/crouch FOV targets); the user's Steam profile FOV slider (`OPTIONS.SAV`, `CreateProfileName 0x5a76a0`,
unported) so retail gameplay may run wider than 75 while the ride is pinned to 75; `execGetFOVScale` stub
(`DishonoredGameNativeStubs.cpp:1281`); `ProcessViewRotation 0x6afb50` unported (`:441`).

**NPCs stuck.** HC step 5 moved `FArkComponentLocomotion::PreAsyncWorkTick` onto the manager
(`S/Engine/Src/UnLevTic.cpp:3286-3324`, `arkcomponentmanager.cpp:19-46`, `arkcomponentpolicy.h:58-136`);
`physWalking` is `MovePawn` only (`dishonorednpcpawn_locomotion.cpp:139-153`). `ComputeDynamic`
(`S/DishonoredGame/Src/arkcomponentlocomotiondynamic.cpp:485`) now reads `m_fRealSpeed` from a `Velocity` that
`FullMovePawn` wrote last frame (`:735`, zeroed at `:682`) → the stuck-in-wall test (`:536-545`) sets
`m_CurMoveYaw = -200000` → turn requests (`arkcomponentlocomotionrootmove.cpp:400`) → start/turn animations in place.
HC measured 95 vs 123 u/s (`build/agentHC/revert_step5_loco.py` reverts the hook alone). Also: the policy ticks every
NPC incl. `PHYS_None`; `physicsRotation` no longer turns NPCs (`dishonorednpcpawn_rotation.cpp:52-56`) and
`FArkComponentFaceTo` is unported; physics can stay `PHYS_None` after the matinee (`MAT_SetCollision`
`dishonorednpcpawn.cpp:622-645`, `StopRetargeting` `discomponentretargeting.cpp:371`,
`S/Engine/Src/interptrackinstattachment.cpp:39`); `StartRootMotionMode` disables locomotion (`:338-352`) and
`ApplyLocomotionModifier` is unported (`dishonorednpcpawn.cpp:564-610`); the soirée start/stop AI stim is unported
and `MatineeControllerAdd` is gated off (`-hbcontrollers`, HB). Policy bugs: `Register` mid-tick
(`arkcomponentpolicy.h:58-70,122-133`), `Insert` index shift (`:89-103`). User log: `FilterPathingFail … Failed`
at 88.11.

**Voice/subtitles.** `UDisConv_Blurb` has only `Serialize` (`disconv_blurb.cpp:32`); the base defaults
(`Inc/CppText/UDisConv_Node.h:10,15`) make every line finish in zero time in `PlayFromNode`
(`disdialogtree_ingamebind.cpp:452-491`), so nothing ever reaches the voice or subtitle code. Retail chain:
`PlayNode_Derived 0x89fb90` → `PlayBlurb_Shared 0x89deb0`; `TickNode 0x89fbb0` → `TickNode_Shared 0x89e0f0` →
`StartBlurb 0x89bb50` → `IDisConvSpeakerInterface::PlayDialogBlurb 0x8a0930` →
`UDisConversationComponent::PlayDialogBlurb 0x89fbf0` (decompile `build/agentFR/decomp2013/`), `IsDonePlayingBlurb
0x89a160`, `IsAnyAkEventPlaying 0x896740`, `FindTextBlurb 0x8a2410`, `StreamDialogBlurb 0x88ba50`, `IsReadyToSpeak
0x88d7e0`, `InterruptTickingNode 0x89bb00`; `UDisDialogVoiceData::FindTextBlurb 0x8a1550`, `RebuildLookupMaps`
[0x90e1a0]; `FDisSubtitlesInfo::AddSubtitle 0x89cea0`, `KillSubtitle 0x89d160`, `Tick` [0x8ecd80, 1346 B] from
`TickGlobalConvMan` (`disconvglobalman.cpp:407`, `Term_GlobalConv :97`); HUD `Tick_Subtitles 0x79fe20` (PreAdvance
slot `disgfxmovieplayerhud.cpp:1328-1335`), `OnMainSubtitleChanged 0x7af5d0`, `OnSecondarySubtitleChanged 0x7af6e0`,
`SplitSubtitleText 0x7adf80`, `ShowSubtitleText 0x799690` (clip `DHMC_Subtitles` bound `:1091`). Ride lines: the
`UInterpTrackDialog`/`UInterpTrackInstDialog` track is unported (`interptrackdialog.cpp`, `interptrackinstdialog.cpp`
are stubs; decompiles `build/agentFO/decomp2012/`): `InitTrackInst 0x882330`, `UpdateTrackInst 0x873750`,
`NeedsSynchronizing 0x8735c0` (hook ready `S/Engine/Src/UnInterpolation.cpp:1967-2044`), `TermTrackInst 0x873ab0`,
`GetBlurb 0x86f600`, `GetAltBlurb 0x86ee60`. Audio device glue is ported (`S/AkAudio/Src/akaudiodevice.cpp:411,621,666`,
`akunrealiohookblocking.cpp`) on the silent backend `S/External/Wwise2012/Src/AkSilent*.cpp` (2,008 lines), which ends
every event on the next `RenderAudio` (`AkSilentSoundEngine.cpp:344-351`); `WITH_FACEFX=0`. Retail only adds a subtitle
when `PostEvent` returned a playing id and keeps it while the event plays, so line duration comes from audio.

**Player.** Bindings are fine (`DishonoredInput.ini:91-93`; HJ's CC_HJ3.log: 172 bindings). Crouch:
`execDis_SneakOrSlide` stub (`DishonoredGameNativeStubs.cpp:1293`, [0x6f9170] decompile
`build/agentAC_work/c12/0x6f9170.c`), `StartSneaking` [0x70d450], `AttemptChangeToSneakHeight` [0x7074c0],
`AttemptChangeCollisionHeight` [0x6feca0] unwritten; `execStartCrouch/EndCrouch` stubs (`:683,687`; `StartCrouch`
[0x6fcfe0]); `ADishonoredPawn::MaxSpeedModifier` [0x794b10] todo (`dishonoredpawn_attributes.cpp:85`). Sprint:
`HandleHeldButtons_Movement 0x6aeb10` and `_Movement_Gamepad 0x6ae610` unported (`HandleHeldButtons`
`dishonoredplayercontroller.cpp:330-368` runs `_Context` only; decompile `build/agentAU_decomp/s12/`). Jump/mantle:
`ADishonoredPlayerController::CheckJumpOrDuck 0x6ae490` [0x6e2b10] unwritten (engine slot empty
`UnController.cpp:3833`, called `:3761`), `DoJump` [0x715fe0] (`execDoJump` stub `:679`), `DoJump_Mantle` [0x712b10];
skeletons `dishonoredplayerpawn_mantle.cpp` (`PerformMantle` [0x70ff10], `Tick_Mantle` [0x7129d0] not called
`dishonoredplayerpawn.cpp:398-401`, `IsMantleRequested` [0x707170]), `stateplayermaster{mantle,jump,climb,slide}.cpp`,
`dishonoredmantleedgefindercomponent.cpp`, `dismantlefinderstate_*.cpp`; the master FSM is initialised with Walk only
(`dishonoredplayerpawn_action.cpp:84`). Water: `execCalcPlayerSwimAccelRate` stub (`:433`; retail [0x6e3b50], 97 B:
`UStatePlayerMasterSwim::GetSwimStrokeAccel` [0x6b3f50] in the swim state else the pawn's `AccelRate`) returns 0 →
acceleration 0 → stuck; engine `startSwimming/physSwimming` (`UnPhysic.cpp:2458,2700`) are stock; skeletons
`dishonoredwatervolume.cpp`, `dishonoredpawn_water.cpp`, `displayerpawn_water.cpp`, `stateplayermasterswim.cpp`.
The user's SLOMO/GHOST edits are not a cause.

**Wwise size** (2012 PDB, `resources/docs/symbols/functions.csv`): ~3,100 functions, 507 KB of code: sound engine
core ~1,900 (CAkParameterNodeBase/Node, CAkPBI, CAkBankMgr, CAkBus, CAkLEngine + VPL nodes, CAkRTPCMgr, CAkAudioMgr,
CAkURenderer, CAkPlayingMgr, CAkStateMgr, CAkRegistryMgr, containers, CAkPath, CAkSinkDirectSound), music engine 343,
Vorbis decoder 74 (Wwise's packed-codebook variant, not stock libvorbis), effect plug-ins ~300 (RoomVerb, FDNReverb,
MatrixReverb, ParametricEQ, Delay, StereoDelay, TimeStretch, PitchShifter, Harmonizer, Tremolo), stream manager 33,
memory manager 21, `AK::SoundEngine::Query` 31. Source root in the PDB `d:\branches\wwise_v2012.1\`, 291 files.
Build switch already exists: `cmake/Wwise.cmake` (`DISHONORED_WITH_WWISE`, our headers in
`S/External/Wwise2012/include`, `DISHONORED_WWISE_SILENT`).

## Shared rules (every agent prompt, from PHASE22)

Opus 5.5; own worktree via `make_snapshot.py <X>` (RelWithDebInfo, not the Debug default); builds only via
`build_queued.py`, **two per package**; runs only via `game_input.py launch` with `-windowed` (it supports
`key NAME down|up` and `exec`); never real input/focus; no retail `Dishonored.exe`; **never fmodel MCP**; IDA only
on a private copy `build/agent<X>/idb/`; resolve every address by hand in both dbs; never hand-edit
`Inc/CppText/*.h`, `DishonoredGameRegistrants.cpp`, `Sources.cmake` (generator `gen_classes_header.py DishonoredGame
--sdk --module-header --sources-cmake`, byte-identical check with `--out-dir`, CRLF care); report
`resources/docs/agents/agent<X>.md` + `agent<X>_status.csv` + `build/agent<X>_sync.py`; "assume this plan is wrong
somewhere and say where, with the measurement". Gate: `build/gate_run.cmd` on a clean checkout in `build/gate_wt`,
47/47, every `*_criticals` 0, `unported_natives` 0, `probe_natives` not above 4. Faithful ports from the decompiles;
every deviation named in the file banner.

## Packages

### IA — FOV after the ride (measurement first, smallest, merges first)
- **Step 0, required before any port**: shoot ours at world 151–160 (`-finewgame=50 -fosoiree -pcamdiag -fzdiag
  -apshottime`), pair with retail.mp4 at retail t = world − constant offset (HI: use the constant, not the two-anchor
  slope), run `cutscene_compare.py compare` and read post-ride `zoom`; log `hfov`, `DefaultFOV`, `m_fCurFOV`, every
  active `m_FOVTargets` slot. Read the user's retail FOV slider (ask the coordinator / read `OPTIONS.SAV` read-only
  under `C:\Program Files (x86)\Steam\userdata\<id>\205100\remote\`). The user says retail's ride and gameplay FOV
  differ, ours do not: find which side of the ride is wrong.
- **Then port what retail does**, in order, re-measuring after each: (1) `UStatePlayerMasterBase::UpdateWalkNavAnimState`
  [0x69ab60] (749 B; the locomotion/crouch FOV targets via `SetFOVTarget`; reuse `GetActualPlayerStance/
  GetDesiredPlayerStance` `dishonoredplayerpawn_inventory.cpp:166/202`; the anim-state half named, not faked);
  (2) `ExitSoireeMode 0x6628f0` diffed for FOV writes; (3) `tracking.cpp:344-345` against the bytes;
  (4) `ProcessViewRotation 0x6afb50`; (5) if the slider ≠ 75: verify our Options FOV slider drives `hfov` (setting
  118 → `ApplyGameSettings` `dishonoredplayercamera.cpp:99-106`) and report; the Steam cloud profile read
  (`CreateProfileName 0x5a76a0`, `OPTIONS.SAV`) is its own package **IJ** below.
- Always: `execGetFOVScale`; `-iadiag` (every FOV-target change: priority, value, speed; `DefaultFOV` at PostBeginPlay
  and each `ApplyGameSettings`).
- Files: `stateplayermasterbase.cpp`, `dishonoredplayercamera.cpp`, `dishonoredplayercontroller_tracking.cpp`,
  `dissoireecontrolstate.cpp`, `DishonoredGameNativeStubs.cpp`.
- Risk: the walk tick runs every frame (`inputtest_moved/peak_speed` unchanged); arm-follow influence changes the
  arms pose (re-shoot FZ frames).
- Accept: step-0 table; after the port, ride zoom still .99 and post-ride `hfov` within ±1° of retail's (derived from
  the slider or measured zoom); gate 47/47.

### IB — NPCs after the cutscene: the locomotion tick (A/B first)
- **Step 0**: build 1a = HEAD, build 1b = HEAD + `revert_step5_loco.py` (both count as the package's builds: the fix
  goes on top of the better tree; a third build is reported, not taken). Route `-finewgame=50 -fosoiree -gadiag
  -fvdiag -fkdiag -fsdiag -diskismet -ibdiag`, shots at world 150/155/160/170/185. `-ibdiag` per ride NPC
  (`_26`, `_27`, `_20`) once a second: `Physics`, loco root state, `IsDisabled()`, `m_bStuckedInAWall`,
  `m_CurMoveYaw`, `m_fRealSpeed`, `Velocity.Size2D()`, request count, go-to state, `FilterPathingFail`, in-policy.
  WorkerB 850-uu walk speed on 1a vs 1b (references 123 / 95).
- **Ports**, only what the table implicates, each against the bytes: `PreAsyncWorkTick` re-diffed vs 2013 `0x54ac70`
  (velocity source, physics/`m_bStarted` gate); `FArkComponentPolicy::Register/Insert` vs 2012 `0x578ba0/0x577fc0`;
  `ApplyLocomotionModifier` + the `MAT_FinishAIGroup` tail; the detach physics restore if physics stays `PHYS_None`.
  Brain/stim is IC.
- Files: `arkcomponentlocomotion.cpp`, `arkcomponentlocomotiondynamic.cpp`, `dishonorednpcpawn.cpp`,
  `dishonorednpcpawn_locomotion.cpp`, `S/Engine/Inc/arkcomponentpolicy.h`, `UnLevTic.cpp` (only if the slot is wrong).
- Risk: GC/GK walkers + the inputtest go-to share this; retargeting must keep ticking; the captain may walk but not
  turn until IC (say so).
- Accept: A/B table; WorkerB 865 uu at 115–125 u/s; each ride NPC moves > 200 uu in the 20 s after world 150.6 or its
  brain logs why; `m_bStuckedInAWall` never TRUE while `Velocity.Size2D() > m_fVelocityToStartToMove`;
  `FilterPathingFail` 0; HH ride SSIM ≥ .69; gate 47/47.

### IC — the soirée stim, the controller list on, FaceTo (after IB)
1. `UDishonoredAIBrain::HandleAIStim_Internal<FAIStimStruct_Soiree>` / `<…SoireeRejected>` (template exists
   `dishonoredaibrain.cpp:1421`; struct declared `aistimstruct.cpp:363`; 2013 instantiations near `0x720420`) +
   `UDisBehaviorSoiree` (`disbehaviorsoiree.cpp` skeleton); sent from `MAT_BeginAIGroup/MAT_FinishAIGroup`
   (`dishonorednpcpawn.cpp:352-358`).
2. `MatineeControllerAdd` always (invert the switch to `-hbnocontrollers` for one build); 5 clean ride runs.
3. `UDishonoredEngine::Init`: `SetLookatPriority(1,5,19)`, `SetFaceToPriority(15)`.
4. Build 2: `FArkComponentFaceTo` (ctor `0x5795d0`, `Initialize 0x575400`, requests `0x577880–0x5779e0`, update chain
   `0x577a60–0x577f40`, `PreAsyncWorkTick` + managers `0x576b60–0x57a9b0`; `GetComponentFaceTo 0x7ab560`), created in
   `FArkComponentLocomotion::SetConfig`; the rotation intents then make real requests; `UInterpTrackInstFaceTo::
   UpdateTrackInst` [0x53eb90] / 2013 `0x4ff8f0`. If it does not fit, report with decompiles — next package.
- `-icdiag`: stims received, behaviour stack, controller add/remove, FaceTo request/yaw per second.
- Risk: HB's render-thread particle crash; one `BlendNow 0.20` per NPC must still fire; GH's "Failed, with
  animsequence" stays 0.
- Accept: 5/5 ride runs to world 200 with controllers on, 0 Criticals; Start/Stop stim logged per ride NPC with a
  behaviour change after Stop; captain faces within 15° of his go-to direction within 2 s; HH ride SSIM ≥ .69.

### ID — voice lines and subtitles: the conversation path (parallel with IA/IB/IF)
1. `UDisConv_Blurb`: `PlayNode_Derived`, `PlayBlurb_Shared`, `TickNode`, `TickNode_Shared`, `StartBlurb`,
   `InterruptTickingNode`, `GetBlurbLookTarget` [0x8e0e30], `UDisConv_Blurb_InGameData::Get/StopBlurbSpeakerData`
   [0x8e0cc0/0x8dd3f0], `PostLoad` [0x8dd430] (fresh 2013 decompiles into `build/agentID/decomp2013/`).
2. Speakers: `IDisConvSpeakerInterface::PlayDialogBlurb 0x8a0930`, `StreamDialogBlurb`, `IsReadyToSpeak`,
   `IsDonePlayingBlurb 0x89a380`, `FindTextBlurb` [0x8f96f0]; `UDisConversationComponent::PlayDialogBlurb`,
   `IsDonePlayingBlurb`, `IsAnyAkEventPlaying`, `FindTextBlurb`, `StopDialogBlurb` body (`:478`), `StopAkEvents
   0x88bae0`, `UnprepareAkEvent 0x88b0b0`; `UDisDialogVoiceData::FindTextBlurb`, `RebuildLookupMaps`, `PostLoad`
   [0x8fd550] (the `m_ID_Lookup` map is empty today). FaceFX calls compiled out, each named.
3. Subtitles: `FDisSubtitlesInfo::AddSubtitle`, `KillSubtitle`, `Tick` (2013 from `TickGlobalConvMan 0x8a71c0`'s tail),
   wired into `TickGlobalConvMan` and `Term_GlobalConv`; HUD `Tick_Subtitles`, `OnMain/SecondarySubtitleChanged`,
   `SplitSubtitleText`, `ShowSubtitleText`, `ShowNextMain/SecondarySubtitlePart` [0x8034a0/0x803560].
4. **Interim duration on the silent backend** (until IH2 lands): `AkSilent::SetDurationHint(playingID, seconds)` in
   `AkSilentSoundEngine.cpp`; events without a hint keep the next-frame end (Kismet latents, agent FI). The
   `PlayDialogBlurb` port calls it under `#if DISHONORED_WWISE_SILENT` with the media length read from the loaded
   bank/pck if the silent backend exposes it, else `clamp(chars/15 cps, 1 s, 12 s)`; log value and source. Removed
   when IH2 plays real audio.
- `-iddiag`: blurb start/done/interrupt (node, speaker, event, text, duration, source); subtitle add/kill/part;
  `subtitles_mc` Invokes. Add a `subtitle` area detector to `cutscene_compare.py` (bottom-centre text band).
- Route: the Empress scene after the ride (`DisDialogOneShot_7/_27` → `SOIREE_Empress_Regent`), shots world 165–200.
- Files: `disconv_blurb.cpp`, `disconversationcomponent.cpp`, `disdialogvoicedata.cpp`, `disconvglobalman.cpp`,
  `disgfxmovieplayerhud.cpp`, `S/External/Wwise2012/Src/AkSilentSoundEngine.cpp`, CppText for `UDisConv_Blurb`,
  `IDisConvSpeakerInterface`, `UDisGFxMoviePlayerHUD`.
- Risk: PreAdvance every frame (guard on the clip; route AS2 errors ≤ 11); a blurb that never reports done = black
  screen in the Empress scene (`BlockPlayer` chain).
- Accept: ≥ 5 blurbs with text and 1–12 s durations in world 150–220; one `OnMainSubtitleChanged` per blurb and a
  clear after; a capture with subtitle text; the Empress scene still fades back; gate 47/47.

### IE — the boat's matinee lines: `UInterpTrackDialog` (after ID)
- `UInterpTrackInstDialog::InitTrackInst 0x882330`, `UpdateTrackInst 0x873750`, `NeedsSynchronizing 0x8735c0`,
  `TermTrackInst 0x873ab0`, `GetBlurb 0x86f600`, `GetAltBlurb 0x86ee60`; `UInterpTrackDialog::IsPlayingASkippableItem/
  FindNextSkipAnchor/FindLastSkipAnchor/GetKeyProperties` [0x8da7a0/0x8dc5d0/0x8dc670/0x8e9960]; off the exclude
  list via the generator. `-iediag`: each key (time, blurb, alt, `WaitForEndOfBlurb`), each sync wait.
- Files: `interptrackdialog.cpp`, `interptrackinstdialog.cpp`, `UnInterpolation.cpp` (only if the hook needs the class).
- Risk: an over-long duration estimate on a `WaitForEndOfBlurb` key delays the ride (HH ride-end time is the check);
  ride pawns speak through retargeted components (check `GetSpeakerLocInfo`).
- Accept: keys fire within ±0.5 s of retail (HH `subtitle` detector vs retail.mp4 if the user's retail had subtitles
  on; else key times vs the log); ride still ends at world 150.6 ± 1.0; HH ride metrics unchanged; gate 47/47.

### IF — player crouch, sprint, jump, swim (parallel; before IG)
- **Step 0**: run the `inputtest` stage alone and name the 4 `probe_natives`; a hand route after world 160 via
  inputcmd: `key C`, `key LeftShift down` + forward 5 s, `key SpaceBar`, a walk into the nearest
  `DishonoredWaterVolume` (`-distouchprobe` list). Baseline: collision/eye height, peak 2D speed, Z apex, physics in
  water.
- **Build 1**: crouch — `Dis_SneakOrSlide` [0x6f9170] (`IsInputEnabled(0x200)`, slide-cancel branch),
  `StartSneaking`, `AttemptChangeToSneakHeight`, `AttemptChangeCollisionHeight`, `StopSneaking` completed,
  `execStartCrouch/EndCrouch` + `StartCrouch`; the upper-FSM action request follows retail's own null test, else
  guard and name it. Sprint — `HandleHeldButtons_Movement 0x6aeb10`, `_Movement_Gamepad 0x6ae610`,
  `_Movement_Movable 0x6a2ec0` to the `UDisMovableComponent` boundary; `ADishonoredPawn::MaxSpeedModifier` +
  `MaxSpeedModifier_Derived` + `IsCarryingCorpse` together (never 0; `dishonoredpawn_attributes.cpp:85-97`);
  `GetSpeedFraction` [0x6fb8a0]. Swim — `CalcPlayerSwimAccelRate` [0x6e3b50] + thunk, `GetSwimStrokeAccel 0x6b3f50`.
- **Build 2**: jump — `CheckJumpOrDuck 0x6ae490` (Dishonored override only; never edit `UnController.cpp`),
  `DoJump` [0x715fe0] + exec, `DoJump_Mantle` [0x712b10] honest (`IsMantleRequested`, `IsPressingForward`;
  `PerformMantle` is IG's, returns FALSE), `UStatePlayerMasterJump` (16) + `UStatePlayerMasterFalling` + params
  (`stateplayermasterparams.h`). Swim — `UStatePlayerMasterSwim` (11), `ADishonoredWaterVolume::Touch/UnTouch/
  GetSurfaceZ/CheckPoint/OnTriggerNotify` [0x649d30/0x6568c0/0x630b50/0x630ab0/0x630b70], `ADishonoredPawn::
  OnEnter/OnExitWaterVolume/Tick_Water` [0x7b60b0/0x7b98d0/0x7b60e0], `ADishonoredPlayerPawn::OnEnter/OnExit/
  OnChangeWaterVolume/Tick_Water/GetSwimmingState/OnCameraEnter/ExitWaterVolume` [0x6faa30/0x70b300/0x70dea0/
  0x708ef0/0x705510/0x6ffdc0/0x7090c0]; `UDisWaterVolumeManager` named, deferred unless reached.
- `-ifdiag`: each GBA action, sneak heights before/after, `MaxSpeedModifier` inputs/result, master-FSM transitions
  (state, reason, rejected by), water enter/exit + physics.
- Files: `DishonoredGameNativeStubs.cpp`, `dishonoredplayerinput.cpp`, `dishonoredplayerpawn_stealth.cpp`,
  `dishonoredplayercontroller.cpp`, `dishonoredpawn_attributes.cpp`, `stateplayermaster{jump,falling,swim}.cpp`,
  `dishonoredwatervolume.cpp`, `dishonoredpawn_water.cpp`, `displayerpawn_water.cpp`.
- Risk: `MaxSpeedModifier` runs for every pawn (NPC 123 u/s must not change); new states reach new stubs (list each).
- Accept: `key C` → crouch collision height, eye height drops, `IsSneaking`, second press restores; sprint peak ≥ 30 %
  over walk and = GroundSpeed × tweak; `key SpaceBar` → Z apex ≥ tweak height −10 %, lands in Walk; water →
  `PHYS_Swimming`, non-zero acceleration, exits to `PHYS_Walking`; `probe_natives` ≤ 3, `unported_natives` 0;
  `inputtest_moved` unchanged; gate 47/47.

### IG — mantle (after IF)
- `PerformMantle` [0x70ff10], `Tick_Mantle` [0x7129d0] in `Tick_Begin` (`dishonoredplayerpawn.cpp:367-379`),
  `DoMantle_StepUp` [0x706f00], `DoMantle_Anim` [0x70d380], `IsMantleAllowed` [0x6fb830], `ClearVisAndMantleSettings`
  [0x707400]; `UStatePlayerMasterMantle` (18), `UStatePlayerMasterClimb`; `UDishonoredMantleEdgeFinderComponent` +
  `dismantlefinderstate_*.cpp`. The anim path through the upper FSM is the boundary: `DoMantle_StepUp` must work,
  `DoMantle_Anim` named if the upper FSM is still a skeleton. `-igdiag`: finder result per request, transitions.
- Risk: shares `dishonoredplayerpawn.cpp`/`DoJump_Mantle` with IF; the finder traces while jump is held (`d3d9_frames`).
- Accept: forward + `key SpaceBar` at a dock ledge (coordinates from `-distouchprobe`): Z rises by the edge height
  ±10 uu within 1.5 s, ends in Walk; a plain jump still IF's; gate 47/47.

### IH — the Wwise 2012.1 runtime, reimplemented from the 2012 decompile (a program, several waves)
The user chose this over installing the SDK. Replaces `S/External/Wwise2012/Src/AkSilent*.cpp` piece by piece behind
the existing `DISHONORED_WWISE_SILENT` switch, against our own headers in `S/External/Wwise2012/include` (public
2012.1 API surface as retail's `AkAudio` glue calls it). Each package: functions ported by file from the PDB names
(`d:\branches\wwise_v2012.1\…`), measured against retail with the same banks. A `-akdiag` prints bank loads, HIRC
object counts by type, every `PostEvent` resolution path and every playing id's start/end.
- **IH1 (wave 26, parallel with the game packages): memory + stream + bank manager.** `AkMemoryMgr` (21),
  `AkStreamMgr` (33: `CAkStreamMgr`, `CAkDeviceBlocking/DeferredLinedUp`, `CAkIOThread`, `CAkStdStmBase`,
  `CAkAutoStmBase`, `CAkIOMemMgr`, `CAkTransferDeferred`) over the ported blocking IO hook; the AKPK reader
  (`CAkFilePackage*`); `CAkBankMgr` (60) + `CAkBankReader`, HIRC loading for every object type into the real
  classes' *data* (`CAkEvent`, `CAkAction*`, `CAkSound`, `CAkRanSeqCntr`, `CAkSwitchCntr`, `CAkLayerCntr`, `CAkBus`,
  `CAkState`, `CAkAttenuation`, `CAkFxShareSet`, music objects as opaque until IH4), `CAkRegistryMgr`,
  `AK::SoundEngine::Query` (31). Accept: `Init.bnk` + every `.pck` bank the opening loads parse with the retail
  object counts (compare with a Python bank parser written for the test, `resources/tools/wwise_bank_dump.py`);
  `GetIDFromString`/`Query` answer as retail; no behaviour change in game (`newgame` stage).
- **IH2: a line is audible.** `AkVorbisDecoder` (74; packed codebooks, `CAkSrcBankVorbis`/`CAkSrcFileVorbis`),
  `CAkLEngine` + `CAkVPLSrcNode/SrcCbxNode/PitchNode/FilterNode/MixBusNode` (the lower pipeline), `CAkURenderer`,
  `CAkPlayingMgr` + `CAkPBI` (75) + `CAkContinuousPBI`, `CAkAudioMgr` (45) + the command queue,
  `CAkActionPlay/Stop/Pause/Resume`, `CAkActionPlayAndContinue`, `CAkSinkDirectSound` (20) or `AkSink` on XAudio2
  as retail's sink selection does, end-of-event/duration callbacks (`AK_EndOfEvent`, `AK_Duration`). Accept: the
  Empress scene's blurbs play audibly, `IsDonePlayingBlurb` follows the real end; ID's duration hint removed.
- **IH3: the parameter graph.** `CAkParameterNodeBase/Node` (126), `CAkBus` (59), `CAkRTPCMgr` (51),
  `CAkStateMgr` (22), switch/ranseq/layer containers, `CAkPath`, positioning + attenuation, `CAkRegisteredObj`,
  `CAkLEngineCmds`, `CAkDynamicSequence`, `CAkFeedbackDeviceMgr` (stub as retail without a feedback device).
  Accept: sound effects on the route at retail's volumes (A/B against a retail recording with `-apshottime`-style
  audio capture: OBS session by the user, `--ours-video` compare on waveform RMS per second).
- **IH4: music engine + effects.** `akmusicengine` (343), the ten effect plug-ins (~300), `CAkActionBypassFX`.
  Accept: the menu/level music plays; reverb in the tower interior audible; no RTPC assert.
- Order: IH1 → IH2 → IH3 → IH4, one agent per package, two builds each (CoreSmoke-style unit checks for the bank
  parser and the Vorbis decoder in `source/Development/Tools/CoreSmoke` so a decoder bug does not need a game run).
- Risk: 3,100 functions is the largest single subsystem in the project; the Vorbis variant's codebooks live in
  `packed_codebooks.obj` (data, extract from the exe); thread model (`AK::IOThread`, `CAkLEngine` audio thread) must
  match retail or the engine's `RenderAudio` cadence changes Kismet latents (agent FI).

### IJ — the Steam cloud profile (`OPTIONS.SAV`) — only if IA shows the slider matters
`CreateProfileName 0x5a76a0`, the profile read/write path under `UArkProfileSettings`, FOV/bindings/subtitle flags
from the user's own file so our hints read "[Unbound]" and the FOV matches the user's retail. Accept: setting 118 and
the key names match retail's; HJ's `SetToDefaults` stays the offline fallback.

## Schedule and merge order

- **Launch 1 (parallel, 5 agents):** IA, IB, ID, IF, IH1.
- **Launch 2 after their merges:** IC (after IB), IE (after ID), IG (after IF), IH2 (after IH1 and ID). IJ only if IA
  asks for it.
- **Merge order:** IA → IB → ID → IF → IH1 → IC → IE → IG → IH2 (IH3/IH4 are the next wave). Overlaps:
  `DishonoredGameNativeStubs.cpp` (IA, IF), `dishonorednpcpawn.cpp` (IB, IC), `dishonoredplayerpawn.cpp` (IF, IG),
  `AkSilentSoundEngine.cpp` (ID, IH1/IH2 — ID's hint stays until IH2 replaces the backend).
- Coordinator per merge: sync script, 3-way merge flagged files, generator regen byte-identical, `bitfield_audit` 0,
  commit explicit paths (never `git add -u`), gate in `build/gate_wt`, HH before/after vs `build/cmp/final`, PHASE23
  outcomes row. `UnPlayer.cpp` stays untouched by every package (HK's slot is in; IF edits only the Dishonored
  override).
- Wave end: rebuild `head_rel`, HH end to end (ride MAD ≤ 11.6, SSIM ≥ .69, zoom .99, `subtitle` detector on), one
  hand-route log with the `-ia/-ib/-id/-if/-ak` diag lines as the evidence for the four defects, then the user plays.

## Where this plan is most likely wrong
1. The FOV may be retail's already, or the user's slider: IA's step 0 decides before anything is ported.
2. HC's 95 vs 123 may be hitches rather than the tick slot: IB's A/B decides.
3. ID's silent-duration estimate is a guess by construction: IE's ride-end time is the check, IH2 removes it.
4. IF's states will reach stubs nobody has listed: the `probe_natives` list from its step 0 is the ledger.
5. IH's size estimate comes from symbol counts, not from reading the code; IH1's report must re-size IH2–IH4.

## Outcomes (coordinator log)

| Package | Commit | Landed | Where the brief was wrong |
|---|---|---|---|
| IA | `bda2769` | measured: hfov 75.00 before/during/after the ride, zoom vs retail 0.99-1.00 at world 151-154, the user's retail slider (OPTIONS.SAV setting 118; format SHA-1 + size + LZO1X settings) is 75 - the FOV is retail's on both sides; ported `UpdateWalkNavAnimState` (2013 0x675450; crouch FOV target, arm follow, stance; `SetAnimState` 0x6e1da0 / `RequestAction` 0x6b1d00 named), `ExitSoireeMode`'s arm-follow re-enable, the 2013 doubled zoom blend speed, `ApplyPlayerControlDelta` (0x6a34d0), `ProcessViewRotation` (0x6afb50), `GetFOVScale` (0x6b01e0, mouse look 1.2x), `-iadiag`; one build | the ride/gameplay difference the user sees is not FOV (letterbox, full-body camera and sharpness change the same way in both); `UpdateWalkNavAnimState` sets only the crouch target; `Min(0,..)` is retail's; `ExitSoireeMode` writes no FOV; IJ is not needed for FOV but the user's file has all 36 bindings = 0 (HJ's "[Unbound]"). **Blocker found:** with Bink ON the briefing page waits for a controller-0 key the `-finewgame` route never sends - every scripted run must pass `-nomovie` (`cutscene_compare.py` does since `e22a0b7`) |
| IB | `63c3c91` | A/B from one binary (`-ibphystick`, `-ibnovel`): both tick slots walk at ~123 u/s (HC's 95 was `walk.py` counting the turn and stop); retail's velocity source (`ComputeDynamic` tail 0x94bec0 writes Velocity/Acceleration from `m_CurMove`; `FullMovePawn` 0x93fb00 moves Velocity x dt, clamps); start-poly traces with retail's 0x30DF (`TRACE_World` missed the boat deck, failed every start poly); `-ibdiag`; Worker B 860 uu at 123 steady, stuck 0 frames, FilterPathingFail 0, ride SSIM .692 -> .728 | the tick slot was never the fault; `PreAsyncWorkTick` is 2013 0x54caa0 (0x54ac70 is `Starting`) and has no physics gate; the policy "bugs" are retail's; physics restores after the detach; the captain still stands - 17-point path, start-move loop with root-motion delta 0 and `MoveActor` blocked at the bow (package IK) |
| IF | `e1926a1` | crouch (`Dis_SneakOrSlide`, Start/StopSneaking, the height changes, StartCrouch/EndCrouch, `RequestAction`), sprint (`HandleHeldButtons_Movement/_Gamepad/_Movable`, `MaxSpeedModifier` + `_Derived`, `IsCarryingCorpse`), jump (`CheckJumpOrDuck` override, `DoJump`, `DoJump_Mantle`, the Jump/Falling states, `setPhysics`), swim (`CalcPlayerSwimAccelRate` - the stub returned 0 = the water hang, the Swim state, water volume `CheckPoint/GetSurfaceZ`, pawn/player water functions); `-ifdiag`; one build; collision 87.5 -> 65.0, camera bone 167.7 -> 122.7, sprint 500 -> 600, jump apex 106.2, swim accel 500 and exits to walking; gate stages 23/23, probe_natives 4 -> 3 | sneak uses the tweak `m_fSneakHeight` 65 (not CrouchHeight 29) and no `bIsCrouched`; sprint is +20 % (absolute `Attribute_GroundSpeedSprint` 600); there is no jump tweak (`Attribute_JumpZ` 800); `GetSwimStrokeAccel` 2013 is 0x660690; the water volume's native Touch/UnTouch cannot be reached (no AActor override here) so the under-water camera/breath path never starts; the upper/left-arm FSMs are never initialised (requests refused, logged); the route needs `-nomovie` and the player is blocked next to the boat socket after the ride on HEAD too |
| IH1 | `3ef4426` | AK::MemoryMgr + TLSF (34), AK::StreamMgr with the blocking device and AK::IOThread (185), CAkBankMgr + thread/queue/reader/usage slots/callback manager (82), HIRC loading into every non-music class, CAkRegistryMgr, bank API, GetIDFromString, Query; silent memory/stream backends deleted; `wwise_bank_dump.py`, `resources/docs/wwise/`, CoreSmoke WwiseSmoke; 981 banks / 42,453 objects identical to the Python dump, opening 119 ops / 9,369 objects identical, CoreSmoke 99 -> 127, gate stages 25/25 (d3d9 re-run idle 1860 frames) | sizes: memory 34, stream 185, bank 82 (not 21/33/60); CoreSmoke lives in `source/Tests/CoreSmoke`; CAkFilePackage is the SDK sample AN ported; the deferred device is never created (38 fns out of scope); Query now gives `UAkEvent` real radii (1 of 7 events 4000 instead of -1); HIRC needs IH3's game-sync managers (data stores in `AkGameSyncs.*` stand in); re-sized IH2 1,044 fns / 174 KB, IH3 699 / 86 KB, IH4 730 / 167 KB |
| ID | `ea91599` | `UDisConv_Blurb` play/tick whole, the speaker interface (8-parameter `PlayDialogBlurb`), `UDisConversationComponent` blurb path + Ak callbacks, `UDisDialogVoiceData` lookup maps, `FDisSubtitlesInfo` Add/Kill/Tick (2013), the HUD subtitle functions, the audio system's end-of-event queue, `UDisAkComponent::IsPlayingID`; `-iddiag`; `subtitle` detector in `cutscene_compare.py`; 'SheWaiting' plays 7x in world 162-223 lasting 3.5-4.2 s each (was the same frame); two defects fixed on the path (`GetAkComponent` rebuilt a component per call - retail 0x5b17e0 searches AllComponents by bone; the silent PrepareEvent callback passed event id 0); interim `DishonoredWwise::SetDurationHint` (chars/15 s) until IH2 | the Empress scene has no blurb nodes (its lines are matinee dialog tracks = IE); the only blurb in 150-220 is 'SheWaiting'; subtitles default OFF for INT and retail.mp4 shows none; no subtitle is drawn because nothing feeds the propagation cache / `m_ListenerPosition` (IE takes the listener feed; IH3 the positioning); the route needs `-fwdiag -fwteleport=DisTrigger_16@160` and `-nomovie`; route AS2 errors are 14 on head_rel too (the <= 11 bound is stale); blurbs had no in-game data (`GetClassVars` unported) |
| IK | `8dcbd03` | the L_Tower_P navmesh sat +249 X off retail (our `APylon` installed `DisPylon_2`'s `LocalToWorld`; retail `PostBeginPlay` 0x65f8b0 sets identity for imported meshes, `PylonMoved` only re-registers) - ported; retail's start-poly teleport rule (`m_bHasBeenOnNavMesh`, 2013 0x949890); `ComputeMoveYaw` 0x93e1f0 / `ComputeTargetMoveYaw` 0x947950 (were stand-ins); `-ikdiag`; 0 teleports (41 before), the captain stands on the bow as retail w150, ride SSIM .735 / MAD 10.2 at the constant offset | root motion was never 0 (140/167 frames, 6.6 uu/frame; IB sampled the zeroed frame); the block was the teleport into the boat's physics asset, not the bow; the detach position matches retail; the tool's slope fit (1.094) halves the ride SSIM - read it at the constant offset; Worker B now routes to the wheel on the corrected mesh (retail w150-154 shows a worker there); the captain still moves 0 uu: a pre-detach `FindPath` failure and our `m_bPathfindAlreadyFailed` gate skip every later search where retail's `m_LastPFError` state machine re-searches - **IK2** (same agent, fresh budget, on `8dcbd03`) |
| IC | `b83f983` | `UDisBehaviorSoiree` whole (2013 bodies 0xae4590-0xaff360), the Soiree/SoireeRejected stims (VA 0xb5af50/0xb5afe0) from Begin/FinishAIGroup, `ExitFromMatinees`/`StopMatinees`/`MAT_GetBodyIntention` etc., `SetFaceToPriority(15)` + `SetLookatPriority(1,5,19)`, `FArkComponentFaceTo` whole + `ArkSmoothFixAngleRotation` driving the rotation intents (behind `-icfaceto` until the FaceTo track and the `FullMovePawn` yaw land); `-icdiag`, `-icpsysdiag`; 12 party NPCs enter the behaviour at 51.0, boat NPCs get Start 56.2 / Stop 150.5 and change behaviour; 4/4 ride runs clean; ride SSIM .728 -> .738 | the controller list stays OFF: 6/6 runs kill the render thread in `FParticleSystemSceneProxy` after the bind, 2/2 clean with `-icnosoireeattn` - `UpdateSoireeAttention` holding Busted (retail 0xb3f120 does the same) exposes memory corruption (**IL**); the stim cannot keep the game alive (blocking matinee, `EvaluateSoiree` never takes a blocking stim); `SoireeRejected` has no sender (`MAT_IsAllowed` uncalled); the 2013 addresses are not near 0x720420; no rotation intent fires on this route; the `FullMovePawn` yaw application and the FaceTo track inst are the next package |
| IG | `ac5e77f` | `PerformMantle`, `Tick_Mantle` (from `Tick_Begin`), `DoMantle_StepUp/Anim`, `IsMantleAllowed`, the edge finder component (17) + its FSM, `UStatePlayerMasterMantle` (18), the finder tweaks from `ApplyTweakChanges_Derived` (0x6be9b0, partial), helpers; `-igdiag` + `IGLEDGE`; gate stages 23/23, probe_natives 3, d3d9_frames 1680 | no ledge on either route is accepted by the finder (spawn = platform over an open plane; the dock stop is a BlockingVolume retail cannot mantle either) so the step-up was never exercised - next: `IGLEDGE` to a known ledge; the anim boundary is the master action base (`DoAction`, `SetAnimState` unported), not the upper FSM - `PerformMantle` refuses edges > 120 uu for now; the finder had no tweaks (class default zeros = never finishes a check); `-distouchprobe` lists no geometry; `UStatePlayerMasterClimb` is the ladder state |
| IE | `c98da12` | `UInterpTrackInstDialog` whole (Init 0x882330, Update 0x873750, NeedsSynchronizing 0x8735c0, Term 0x873ab0, GetBlurb 0x86ee10, GetAltBlurb 0x86ee60) + `UInterpTrackDialog` key properties/skip anchors, `UDisConv_Soiree::FindBlurbs` 0x8b1800, the speaker-name chain, the listener feed from `UGameViewportClient::Draw` (0x2c6010-0x2c604d) through a `GDisSetListenerHook` seam (UnPlayer.cpp hunk merged on HEAD's blob), `UDishonoredAudioSystem::Update`'s listener cell + per-component cache, `GetCellAtPoint` 0x792d10, `DisSetAkSoundPositions` 0x792fe0; `-iediag`; subtitle detector re-tuned (stats_v8); 13 boat keys fire within 0.26 s, 12 subtitles drawn + cleared with subtitles on, ride ends 150.9, SSIM .691 = control, gate stages 22 ok + inputtest re-run 9/9 | `InitTrackInst` uses `FindBlurbs` + the speaker-name chain (FR had them editor-only), not `FindTextBlurb`; `GetBlurb` is 0x86ee10; no boat key has `WaitForEndOfBlurb`; the listener is set in `UGameViewportClient::Draw`, not `ULocalPlayer`; the feed and the cache were both needed; SSIM only compares at the same alignment (the tool's slope fit stretched to 1.075); inputcmd `wait` counts frame time; the portal path search (IH3) still leaves other-cell voices at FLT_MAX |
| IK2 | `82e4f2d` | retail's `m_LastPFError` scheduler state machine (2013 0x949890) replacing the `m_bPathfindAlreadyFailed` gate, `ArkIsPointOnNavMesh`, `ResetPathProperties`/`PreAsyncWorkTick` flag fixes, `FullMovePawn` head + `FindValidLocation` (0x57ffb0), the reference engine's RMM_Accel physics pair removed (`ProcessRootMotion` immediate tick, `performPhysics` skip - retail 0x746280/0x6a2230 have neither; the `UnPhysic.cpp` edit was unbuilt in the agent's tree, main builds clean with it); `-ikdiag sched`; the captain gets a 16-point 2420-uu path at 150.53; ride SSIM .733-.737 | re-searching gave him the path but not the walk: the reference RMM_Accel pair and retail's step-raised `FullMovePawn` sweep (from MaxStepHeight up, drop within 3x; ours sweeps from the cylinder, 348/348 moves refused by `EmpressBoat_anim_PA` with FaceTo on) decide it - **IK3**; Worker B's route changed with IK's navmesh, not here; the avoidance half needs `PointReachableZeroExtent` + `DetectPreAvoidanceCollision` 0x543f70 (absent) |
| IH2 | `a945bac` | the audio thread + message queue, playing/transition managers, renderer, lower engine + voice pipeline, DSP, PCM/ADPCM/Vorbis sources (packed codebooks from the retail exe), DirectSound sink, media preparation, retail Init/Term order, post-Term AK_Fail; `-akwav`; CoreSmoke 127 -> 145 (Vorbis = ffmpeg at 73 dB SNR; AK_Duration 2665.0 ms for 2664.99 ms media); in game 36 events / 36 end-of-event, every voice's media prepared, the WAV shows RMS ~1,400 in every play interval; gate stages 22 ok (inputtest_moved failed 4x under load) | media preparation is not deferrable (every line is PrepareEvent'd before PostEvent; banks are structure-only); the opening posts one event before the conversations (`Snd_Fade_In_All`); the symbol is `g_packedCodebookData` + `g_packedCodebooks`; 125 "IH3" functions sit on a single Play's path; `PlayAndContinue` only comes from continuous containers (none in the banks); retail mutes an unfocused window - the sink keeps global focus; the global callback runs on the audio thread in retail; re-sized IH3 ~620, IH4 ~825 |
| IL | `94d928d` | the crash was a double free: `SetupTickParamsFromTarget` (0xb68e10) struct-copied `m_DistanceValueTuning` whose generated `m_Points` is a `BYTE[12]` placeholder for a `TArray`, so the player's attention info aliased the tweaks' curve buffer and every `ResetFromTemplate` of the reflected `m_PawnAttentionTweaks` freed it (3x per frame on the boat); fixed as retail (field copy + `TArray::operator=` 0x4448d0); controller list ON by default, `-hbcontrollers` gone, stim always sent; `-ilguardheap` wrapper, `-ildiag`; 11/11 ride runs clean past world 200-243, guard heap 317,007 allocations 0 faults, gate stages 23/23 | not a stray write, overrun or layout mismatch; the player is never at Busted (the min-level loop skips the soirée type) - the per-tick slot reset is the trigger, retail's reset leaks instead; the CRT debug heap cannot see FMallocBinned blocks (hence the guard wrapper, which asserted on alignment 16 in build 1); `--quit-at` is not world time; IC2a is .731 under the constant-offset rule; ride SSIM .72 +-.02 is run spread. Open: the generator should type `m_Points` as a real `TArray` |
| IK3 | `cace044` | retail's `FullMovePawn` whole (2013 0x93fb00: the sweep from MaxStepHeight up with slide, the floor sweep within 3x with steep-slope slide, `FindValidLocation` + touch, location/rotation write, base/Floor, untouch, zone, push body, clamped velocity, fall decision), `SendTouchAndBumpEvents`, `UnTouchActors`; FV's floor stand-in removed; `-ikdiag floor`; the captain walks 684-813 uu, 1059 uu with `-icfaceto` (0 blocked) along the deck as retail w152-154; gate stages 23/23 twice | the merged tree restored root motion but `MoveActor` was still blocked inside the hull - the step-raised sweep clears it; at X ~10321 the deck ends and the NPC floor sweep finds nothing at the stair foot (poly 10 uu below), he falls into the slip and a falling NPC never lands - **IK4** (name the actor at the stair foot and its trace types; NPC landing; FaceTo on by default; the two avoidance helpers exist in retail: 0x2a9ba0, 0x584f10); ride SSIM .726-.747 depending on the offset (alignment, the ride frames do not run the move code) |
| IM | `a0a3aa2` | all three player FSMs initialised in `PreBeginPlay_Actions` (2012 0x720260), the action base (`UStateSharedActionBase`, `UStatePlayerMasterAction` DoAction/OnExitState 2013, `UStatePlayerChangeReadyStance`, UpperIdle/Nav), the anim-state system whole (`UDisAnimStateComponent` 0x69f610, pickers, the pawn's `SetAnimState` 0x6e1da0, the tweak arrays), `UDishonoredAnimNodeSeq` + notifies + the mantle nodes; `-imdiag` IMSCAN/IMGO/IMSTATE; crouch/stand play `Empty_Sneak_In/Out`, landing plays `Empty_JumpLandSmall`, animated mantles at 121.9/133.9/181.9 uu edges rise within 8 uu and end in Walk, 23 accepted ledges at the dock; gate stages 22 ok (inputtest_moved load, A/B 1045/1063) | the whole anim-state system was the gap, not DoAction alone; retail plays no jump-in with empty hands (the land action plays); the empty hand is not "physically equipped" (vtable slot 76 = xor eax,eax; GJ had TRUE -> `_Ready` states with no sequences, hung the upper FSM); the 120 uu threshold is on the finder's height (12-15 uu above the geometry); ledges are plentiful at the dock (IG's scan used `TRACE_World`); measurement teleports kill the player by fall damage |
| IK4 | `9850a24` | FaceTo on by default (`SetConfig` always creates it as 2012 0x584010; three clean ride runs, the captain walks 800-1059 uu straight onto his heading); `-ik4traceall`, the five-way downward probe; ride SSIM .726/.736; gate stages 23/23 twice | not a trace-mask divergence: at 10327/23053 no collision primitive exists within 600 uu even with every primitive forced to answer, while the navmesh has a poly at Z 2415 and the stairs render - the stair/landing mesh has no collision in our runtime (**IN**); a falling NPC never lands because retail's NPC `setPhysics` (0x7adaa0) drives the NPC master FSM (`UStateNPCMasterFalling` 0x6c9730, a skeleton here) and the water volume is unwired - its own package; the avoidance helpers still unported; found a Wwise shutdown pure-virtual in `CAkBankMgr::UnloadAll` (handed to IH3) |
| IH3 | `3e126e3` | RTPC/state/environment managers, the parameter node graph with full `GetAudioParameters`, bus ducking + volume tree, 3D positioning (`Get3DVolumes`, paths, max radius), containers (random/sequence/switch/layer, continuous, PlayAndContinue), the matinee Ak tracks, `UAnimNotify_AkEvent`, the rest of `UDishonoredAudioSystem::Update` (ambience, environments), the volume RTPCs, the portal path search; two IH1 shutdown defects fixed (`ClearPreparedEvents` iterator order 0x92dbf0, key-0 actions in the index 0x94a0f0); CoreSmoke 155 (RTPC/state/3D vs the bank curves within 0.1 dB); 292 events by world 152 (36 before), ambience + foley + boat engine audible, other-cell voices get paths; ride RMS correlation vs retail 0.20 -> 0.51; gate stages 23/23 first try | no sound effect had ever been posted (the Ak tracks, the anim notify and ambient registration were dead) - RTPCs were not the reason; the banks have 140 continuous containers (IH2 said none); `Snd_Fade_In_All` is a SetState; `Get3DVolumes`/`SetParam` were marked ported but absent; no `CAkSwitchMgr` in 2012.1; `CAkDynamicSequence` has no caller; `ApplyGameSettings` was empty; the IK4 crash was IH1's. Open: ours a median 7 dB under retail (no music, `Play_Amb_LVL_Tower_Intro` never posted, the engine loop stops at w95), 2 of 9 runs end silently during the exit's bank unloads - **IH4** |
| IN | `a47f38b` | retail's `USkeletalMeshComponent::UpdateBounds` (2013 0x747a30, 1610 B) whole: `m_UserBounds`, then the PhysicsAsset bounds whenever scale is uniform and SpaceBases matches, parent's, mesh, per-poly, BoundsScale, no cloth; `-indiag` / `INPROBE`; the captain's floor sweep finds the boat's physics asset from 153.75, walks the bow steps and the drawbridge, stands on the dock floor at 157.76 (1694 uu, 0 blocked); the drawbridge collides too; the player walks the whole boat; census and PhysX counts unchanged; gate stages 23/23 | there is no stair mesh: the "stairs" are the boat's bow steps, the floor is the boat's physics asset (`EmpressBoat_anim_PA`) which hits on its own LineCheck - our bounds were the render mesh's (X to 10290) so the octree dropped the boat; the reference `UpdateBounds` gated the PhysicsAsset bounds behind three checks retail does not have; the navmesh poly sits over the docked boat; IF's "blocked at the socket" was the player dropping into the slip at the old bounds; a world trace with no source actor never reports movers |
| IP | `2898de5` | the shake was `ADD_Head_OutOfbreath` stuck at weight 1 (`UDisAnimNodeBlendByPhysicalCondition::TickAnim` 0x688d80 unported; retail weights it by `GetAimStability` 0x6a9030 = breathlessness x weight, 0 in a soirée); the breathlessness Tick half, `IsItemZoomed`, `UDisAnimNodeBlendByHeadBob::TickAnim` (0x688c30), the camera bump smoother (`DetectBump` 0x6c5030 etc.), `BlendBySpeed::CalcSpeed`; `-ipdiag`; ride pitch range 4.65 -> 0.11 deg, frame jitter 13.2 -> 0.52 px (retail 0.40), ride SSIM .72 -> **.837**; gate stages 23/23 | not low health (100/100); no camera anims or shakes exist at runtime (stripped in 2013); the shake was in the ride, not walking; retail.mp4 after the ride is standing only (the user's bindings were "[Unbound]"); the bob setting's influence slot is NULL and `m_BobAmount` is 0.5 here vs 1.0 in the user's profile (IJ); sprint now asserts one node lower (`SprintDirectionalBlend`, all children additive) - **IS** |
| IO | `24c7074` | `UpdateFaceTo` (0x943190: nothing had ever requested a body turn - 0 requests), the `PreAsyncWorkTick` tail (lean into turns), `SetModifier` whole, `ComputeSpeedFromSpeedModeAndAngle`, `UpdateMoveYaw` (stand-in wrote the wrong member), `IsLocoFaceToRequestActive` + the turn conditions; the path builder's mirrored edge orientation fixed (`FindFirstEdgeIndex`, `AdjustPathFindingEdges`, the funnel, `AdjustStraightPath` from 2012); coordinator: `ProcessRootMotion` rotates the actor only in RMM_Translate as retail 0x746280 (turn anims were applied twice); walkers face their velocity within 0.3-8.5 deg; the captain faces his way and arrives at 192.5 | the rotation write existed, the request did not; no mesh rotation offset; the routes were the funnel, not avoidance; Worker B does not walk on the spawn route; `_8/_16/_19` are off the navmesh at 51-55 (no path); the captain stalls 156-174 on the bow edge where the straight path steps 40 uu east (the unported wall test); the ride SSIM bar fails on b08193f itself (.645, no IP) - **IO2** measures the rotation rule and chases the stall / off-mesh three |
| IR | `91352d7` | no rendering defect: seven speakers captured at 22-300 uu, mouths closed and textured, teeth materials/textures fine, the 19 face bones at the reference pose - `WITH_FACEFX=0`; ported `SetFaceFXAsset` (0x312b90, on the BODY mesh as retail 0x77f8a0), `UpdateTransform`'s parent-LOD rule (0x357840); `-irdiag`/`-irface`; lip-sync sized from the 2012 call graph: 1,128 fns / 110 KB with virtuals (637 direct) - FX1 core 384, FX2 graph 430, FX3 runtime 314 + ~25 engine | there is no black mouth to fix, only the shut jaw; FaceFX moves bones, no morphs; the asset lives on the body, the head reads the bones; retail posts the voice from `UpdateFaceFX` (`DisFaceFxStartAudio`) once FaceFX is on; `ir_run.py` can leave games running; the newgame gate stage fails on head_rel tonight ("Streamed out levels were not completely garbage collected", Steamworks `DeviceSelectionDelegates`) - **IT** |
| IS | `3e77e76` | retail's `UAnimNodeBlendBase::GetBoneAtoms` (0x5a3bd0) has no `LastChildNonAddIndex` assert (reference engine's); the all-additive case now follows `BuildEdgeAnimTreeEx` (0x5b9000: identity pose + additive pass, base pose with no relevant child); `UAnimNodeBlendDirectional::TickAnim` (0x593c80) via the virtual `GetDirectionVector` (0x58cb60) + `UDisAnimNodeBlendDirectional`'s override (0xa89210); `-isdiag`, `-isoldcheck`; sprint 600 u/s steady for 32 s, 0 asserts, child weights = retail's rule; IM's checks unchanged; ride SSIM .843; gate stages 23/23 | not an Ark/Dis node - the engine's blend base; the parent is `AnimNodeAdditiveBlending_21`; IP's weights-sum-to-2 was retail's own one-frame `ChildrenHasChanged` transient; the ride anchor moves with the soirée-enter time (.760 at IP's anchor, .843 at its own); the dock pocket only allows ~1 s at 600; W off the spawn platform kills the player by fall damage |
| IT | `eb2a05d` | main never failed: IR's runs were its own Debug build (the GC check lives under `DO_GUARD_SLOW`) and `run_regression --no-build --exe-name` had re-staged it over head_rel's exe; bisection in RelWithDebInfo all pass; the real leak since HK ported: the menu's `PreClose` (0x7d2790) unregisters the controller/common delegates so the MainMenu world is collected after New Game (10 -> 9 worlds, 0 menu objects); newgame 3/3; `make_snapshot.py` now defaults to RelWithDebInfo | nothing changed tonight but the build type; Steam has run since 10-10 and the gate uses `-nosteam`; retail's online subsystem does not clear the cache on map change - the menu does; `--no-build --exe-name` against another agent's build dir overwrites the staged exe (IR's controls); `DishonoredGame-Win64-Shipping.exe` must be re-staged from head_rel at wave end; a stray old exe sits as `DishonoredGame_ITbase.exe` |

Open: IA's gate stages ran under five agents at 100 % CPU (`d3d9_frames` 600, `inputtest_moved` 751 - the base exe swings the
same way); the wave-end gate on a quiet machine is the real number. The user's "FOV never returns" needs a retail clip past
the dock (retail.mp4 ends at world 154.9) or a description of what changes - asked.

Launch 2 (on `63c3c91`): **IC** (soirée stim, controllers, FaceTo) and new **IK** (the captain's start-move loop: the Edge locomotion joint's root motion in `AnimLeafCallback`'s locomotion half, or the boat collision blocking `MoveActor` - diag build first).
**IG** launched on `e1926a1`.
**IH2** launched on `3ef4426` (ID still in flight; its `SetDurationHint` stays as a shim until IH2 merges).
**IE** launched on `ea91599` (dialog track + the listener feed / propagation cache so subtitles get an owner).
**IL** launched on `b83f983` (the render-thread particle crash with the controller list on: debug heap on the attention path).
**IM** launched on `c772395` (the master action base: crouch/jump/mantle animations; the ledge test).
**IH3** launched on `a945bac` (the parameter graph: RTPC/state/switch, 3D positioning + attenuation, paths, the propagation path search IE named).
**IN** launched on `9850a24` (the missing level collision at the dock stair foot).
**IH4** launched on `3e126e3` (music engine, effect plug-ins, the exit defect, the missing level ambience post).

Gates: `a47f38b` (seventeen packages, everything but IH4) **47 ok, 0 failed, 0 skipped** (2202 s, `build/gate_w26pre.log`, one agent building alongside).

### Launch 4 (2026-10-20, on `b08193f`, user away, override on) — the user's second hand-played round

IH4's agent died in the machine shutdown mid third build (its tree: music engine, nine effects + silence source, sends, the
exit fix, `UDisGlobalMusicManager`, Kismet posts without Target, 2,876 of 3,070 Wwise functions; one gate regression: the
music state posts on a stale `ADishonoredPlayerPawn::s_pInstance` at `-startmap` load because retail's `ShutPawnDown` chain
(2012 0x79d360 / 0x70ef70) is unported). **IH4b** finishes it in the same worktree.
The user's new defects (played on `b08193f`): NPCs walk but face one fixed direction whatever way they move, and take odd
routes (**IO**); the camera shakes far more than retail, maybe the low-health camera anim (**IP**); much audio does not play -
water effects when the boat lands and when the lock fills to raise the tower (**IQ**, after IH4b merges: Kismet posts without
Target were one cause IH4 found; the water events are the next); NPC mouths are black and do not move while they speak
(**IR**: the mouth interior render + FaceFX lip-sync sizing/port).
**IS** launched on `2898de5` (the sprint anim-tree assert).
**FX1** + **FX2** (the FaceFX program: core; face graph) and **IT** (the newgame gate failure) launched on `91352d7`. FX3 (runtime + engine integration, lip-sync on) after FX1/FX2 merge.
