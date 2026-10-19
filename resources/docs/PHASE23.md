# Wave 26 — the hand-played opening (`PHASE23.md`, packages IA..IG) + the Wwise program (IH)

**Status: planned 2026-10-18, NOT started. The user said "write a plan but do not execute yet, save this for later". Start only when the user says so.**

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
