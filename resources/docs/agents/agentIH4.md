# Agent IH4 (wave 26, PHASE23 IH4): music engine, effects, the exit death, the level ambience and music

Last package of the IH program. The Wwise 2012.1 interactive music engine, the effect plug-ins with their engine side
(insert effects, bus effects, environmental sends) and the silence source now run on ported Audiokinetic code. On the game
side, the target-less Kismet posts, `UDisGlobalMusicManager` with the music states and the map-commit `StopAllSounds` are
ported, so the opening posts its ambience bed, its level music and the music state, and the menu music stops at the commit.
The exit death IH3 left open is found and fixed. The gate regression the first hand-back left open is closed by IH4b
(section 6): retail's pawn termination chain. Open after IH4b: 3 of 16 cutscene_compare exits end silently in the exit's
Wwise bank unloads (section 6).

Worktree `build/agentIH4_wt` (made at `af990e9`), build dir `build/agentIH4` (RelWithDebInfo), staged as
`DishonoredGame_IH4.exe`. Two DishonoredGame builds (build 1: every fork; build 2: the map-commit StopAllSounds + the
environment wet diag). Four forks owned disjoint files: M (music engine), F1 (effects engine side, RoomVerb, ParametricEQ,
silence source), F2 (the other seven plug-ins, FFT), G (game side and the three open items). 2012 decompiles of all 829 rows:
`build/agentIH4/dec2012/`; private IDA copies `build/agentIH4/idb/`. No fmodel, no real input, no retail exe. Nothing
committed. Sync script `build/agentIH4_sync.py` (base `b08193f`).

## 1. What was ported

Paths under `S/External/Wwise2012/` unless stated; every file banner names the 2012 rvas (2013 where matched) and deviations.
Inventory `resources/docs/wwise/ih4_inventory.csv` (829 rows; generator `build/agentIH4/tools/make_inventory.py`): **811
ported**, 10 `CAkSplitterBus` not ported (feedback only), 6 mislabelled handovers already IH3's (`DynamicSequence::*`,
`NotifyEndOfDynamicSequenceItem`), 1 IH3 area (`CAkListener::ComputeConeEffect`), 1 folded engine symbol
(`USequence::IsStandalone`). Fork rva lists: `build/agentIH4/fk_{M,F1,F2}_rvas.txt` (355 / 251 / 275).

| Area | Files | Retail (2012) |
|---|---|---|
| Music engine (fork M): renderer, contexts, segments, tracks, switch / random-sequence containers and contexts, matrix sequencer, RS iterator, segment chain, schedule window, music PBI, transitions, bank readers, MusicEngine API; `AkSilentMusicEngine.cpp` deleted | `Src/AkMusicEngine/Common/*`, `include/AK/MusicEngine/Common/AkMusicEngine.h`; hunks in `AkPlayingMgr` (4 music notifications), `AkURenderer` (`ValidateLimits` 0x91f980), `AkSilentSoundEngine.cpp` (4-argument `SetState` 0x917320, `AddExternalBankHandler` 0x918260), `AkBankMgr.h` (friend) | all 343 akmusicengine rows + `CAkPlayingMgr` 0x9278c0 / 0x927fb0 / 0x928250 / 0x928320 |
| Plug-in interface (F1) | `include/AK/SoundEngine/Common/IAkPlugin.h`, `AkCommonDefs.h`, `include/AK/Plugin/PluginServices/*`, `include/AK/DSP/*`, 10 factory headers in `include/AK/Plugin/` | PDB vtable order; `IAkRTPCSubscriber` moved here as IH3 asked |
| FX contexts, allocator, registry | `AkFxContext.*`, `SoftwarePipeline/AkEffectsMgr.cpp`, `AkAudioLib.cpp` (pool) | akfxcontext, akfxmemalloc, CAkEffectsMgr |
| Voice insert effects | `SoftwarePipeline/AkVPLFilterNode.*`, `AkVPLSrcCbxNode.*`, `AkPBI.*` | filter nodes, `UpdateFx` 0x926ee0, `RefreshBypassFx` 0x926e80, `CAkPBI::UpdateFx` 0x93f500 / `GetFXDataID` 0x93f4f0 |
| Bus / master effects | `AkVPLMixBusNode.*`, `AkBus.cpp`, `AkBusCtx.*`, `AkHierarchy.*` (`CAkFxBase`), `AkParameterNode.cpp` | `SetInsertFx`, `DropFx`, final-mix FX, `CAkFxBase::SetFX` / `GetMediaID` 0x939030 |
| Environmental (aux) sends | `Win32/AkLEngine.*`, `AkEnvironmentsMgr.cpp` | `GetEnvironmentalBus` 0x91cef0, `CreateEnvMixBus` 0x91ca30, `DestroyEnvMixBus` 0x91dc50, `MergeLastAndCurrentValues` 0x91db80, env pass of `RunVPL` 0x91cf80 / `GetBuffer` 0x91d510, `UpdateMasterBusFX` 0x91c0d0, `UpdateMixBusFX` 0x91c620, `BypassEnvFx` 0x91c440, `ForAllPluginParam` 0x91cc10 |
| `SetEffect` (message 40), source plug-ins | `AkSilentSoundEngine.cpp`, `AkAudioMgr.cpp`, `AkSrcPhysModel.*` | |
| RoomVerb, ParametricEQ, silence source (F1) | `Src/Plugins/Effects/AkRoomVerb/*` (31 ER patterns bit-exact from the exe), `AkParametricEQ/*`, `Src/Plugins/Sources/AkSilenceSource/*`, helpers `Src/Plugins/Effects/Common/*` | akroomverb, akparametriceq (+win32), aksilencesource |
| Matrix reverb (FDN), Delay, StereoDelay, TimeStretch, PitchShifter, Harmonizer, Tremolo (F2) | `Src/Plugins/Effects/<name>/*`, `Src/Plugins/Common/*`, `Src/AkAudioLib/Common/ak_fft/*` | akfdnreverb (+win32), samples\akdelay, akstereodelay, aktimestretch, akpitchshifter, akharmonizer, aktremolo, plugins\common, ak_fft |
| Glue | `AkAudio/Src/akaudiodevice.cpp` | the 8 `RegisterPlugin` calls of 2013 0x5b1430 with the real create functions; `StopAllSounds` = `Flush(NULL)` + `StopAll(-1)` (2012 0x5fa200); `DisAkGlobalCallbackFunc` (2012 0x7f3f50 / 2013 0x7873a0) registered and unregistered as retail |
| Exit death | `AkAudio/Src/akunrealiohookblocking.*`, `AkAudio/Inc/AkAudioWwise.h` | reference-counted package reader (deviation, section 2) |
| Level ambience + music posts | `AkAudio/Src/akaudioclasses.cpp` | `USeqAct_AkPostEvent` without a Target posts once on the global object (2012 0x5fbc40) |
| Music manager | `DishonoredGame/Src/disglobalmusicmanager.cpp`, `dismusicstate_*.cpp` (6), `dishonoredgameinfo.cpp`, `dishonoredaudiosystem.cpp`, `Inc/dismusicstateparams.h`, 7 CppText | `UDisGlobalMusicManager` (Init 0x8c8310, Tick 0x8d22c0, Term 0x8b16b0, ChooseNewState 0x8c8390, combat intensity 0x8d2180), the five music states |
| Menu music at the map commit | `Engine/Src/UnLevAct.cpp` | `UWorld::CleanUpBeforeLevelTransition` (2012 0x26cec0, 2013 0x258bb0) starts with `UAkAudioDevice::StopAllSounds` |
| CoreSmoke | `source/Tests/CoreSmoke/WwiseMusicSmoke.cpp`, `WwiseFxSmoke.cpp`, `WwiseFx2Smoke.cpp`, hooks in `WwiseSmoke.cpp` / `CMakeLists.txt`; include roots in `cmake/Wwise.cmake` | |

Generated (regenerate on main, never copy): `DishonoredGameMusicClasses.h` (7 CppText includes), DishonoredGame
`Sources.cmake` (the music files off the exclude list), `DishonoredGameNativeStubs.cpp` (5 music natives leave it),
`DishonoredGameRegistrants.cpp`. Fork G's second `--out-dir` run was byte-identical.

## 2. Named deviations (each also in its file banner)

- Music: `AddChild` decrements a refused activity count where retail increments it a second time; `CAkMusicRenderer::Play`
  skips the monitor notification (empty in retail); `NotifyMusic` / `NotifyMusicUserCues` return when the playing id is gone
  (retail dereferences it); the music readers set `g_pHircParseEnd`; the switch context frees a context on one scheduling
  path where retail can leak one (the leak would hang `MusicEngine::Term`, which waits for every context).
- Effects: DSP loops not unrolled by four (summation order may differ by 1 ulp); `ERUnitDual` reads one frame at a time;
  the RoomVerb `ProcessSpread*Out` share their front half; `AddPipeline` skips a slot (retail drops the voice) when a
  filter node cannot be allocated; the EQ reads its 17-byte bands field by field and keeps the old coefficients on an
  unknown filter type; the Matrix reverb's 20 hand-unrolled routines are one template (same SSE operations, same order);
  unaligned SSE loads in Delay, StereoDelay and the FFT; FFT twiddles from SSE2 double cos/sin.
- Exit: the AKPK package reader is reference-counted (`FAkSharedReader`); retail reads by file name through the engine's
  async IO (2013 0x5b0c20) and has no shared reader that can die under a streaming voice.
- `DisAkGlobalCallbackFunc` reaches AkAudio through the `GDisAkGlobalCallback` function pointer (module boundary).
- Music manager: the rat half of the combat-intensity scan is skipped (crowd-agent iterator unported); the music tick is
  skipped without a player pawn.
- Diagnostics: `-akdiag` adds music playlist / clip / PBI / switch / transition lines, environment aux creation, and once a
  second each environment node's wet RMS (`AkEnvWetDiag`, build 2); `-ih4diag` (also on with `-akdiag`) the music manager.

## 3. Measurements

**CoreSmoke: 203 passed, 0 failed, 0 skipped** (`build/agentIH4/coresmoke_run1.log`; 155 before, 48 new). Every packaged
bank: 15,021 objects including the music ones, 0 failed, 0 misparsed, no index leak.

| check | measured | expected (bank / parameters) |
|---|---|---|
| Tower_Intro segment 0x2bbc989d, entry to end of event | 289,792 samples | 289,234 (bank), within one 1024-sample frame |
| AK_MusicSyncBar on that segment | every 2.0000 s | 120 bpm 4/4 |
| Streets switch 0xc5cc052 (134 bpm): state change at 230,400 samples | destination at 258,048 = 3.0016 bars (138 samples off the bar) | the next bar boundary, within one frame |
| RoomVerb share set 0x23ea221c: dry level | 0.000 dB | 0.000 dB |
| RoomVerb first reflection | sample 1477, -31.159 dB | 1477 (27.75 ms x 2^0.15), -31.159 dB |
| RoomVerb 250 Hz band RT60 | 1.755 s | DecayTime 1.800 s (-2.5 %) |
| ParametricEQ 1 kHz peaking band centre | 6.017 dB | 6.017 dB (RBJ) |
| ParametricEQ 40 Hz low shelf / 1.25 kHz skirt | 8.984 / 3.300 dB | 8.986 / 3.300 dB |
| Voice 0x57447bbe with an aux send to the RoomVerb environment | tail rms 35.9 (-27.2 dB under the line) | 0.00 without the send |
| Delay 0x29e9c8cb: echo / feedback | frame 48000, 0.000 dB / -16.478 dB | 1.0 s, 0 dB / 15 % |
| Tremolo: square rate / sine depth | 1.9598 Hz / gain 0.2..1.0 | 1.96 Hz / 80 % |
| PitchShifter 0x933614a5 on 1 kHz | 250.00 Hz | -2400 cents |
| Matrix reverb 0x1feda2f4 broadband RT60 (HF ratio 1) | 0.992 s | 1.0 s |

**Game** (`-finewgame=50 -fosoiree -nomovie -akdiag -akwav`, `build/agentIH4/cc/cutscene_compare_ih4.py`, outputs in
`build/agentIH4/cmp`, WAVs and A/B tables in `build/agentIH4/ab`). Logs `CC_IH4a..e.log` (a = build 1, b..e = build 2).

- **Menu:** `PostEvent Music_Menu` at 8.51 (a target-less Kismet post in Dishonored_MainMenu); playlist 0x246bb8a4 plays
  segment 0x1fb61cae. With build 2 it ends at the map commit (`playing id 2 ended` at 59.14), not at Term.
- **Level:** `Play_Amb_LVL_Tower_Intro` and `Snd_Fade_Out_Player_Detect` at 60.37 (L_Tower_Audio `SeqEvent_LevelLoaded_0`);
  `Music_Tower_Intro` and `Play_Music_Brothel` at 62.05 (a Gate in L_Tower_Script at location discovery, remote events into
  L_Tower_Audio); music switch 0x39a85206 on state group 0xf5792b03 plays segment 0x2b8e6f2a (3,383,364 samples).
- **The music state the opening sets:** `ih4diag ... music manager init: map music events ...Twk_Tower_Empress_MusicEvents`,
  then `music state Exploration posts Music_State_Brothel_None` and `PostEvent Music_State_Brothel_None` (0x8a577bca), one
  SetState of group 0xf5792b03 to None.
- **Tower interior reverb:** the listener enters DishonoredAudioVolume_4 at w111 (environment `sewer`, 0xec599db9);
  `environment 0xec599db9: aux node created on bus 0x28eb7999 (effect plug-in 0x00760003)`, a RoomVerb share set on the
  voice bus. Wet signal per second (IH4b) around line 117 (log 124.18-125.79): -43.7 / -38.1 / -31.0 / -32.2 / -40.5 dBFS at
  log 123-127: the wet stays at -31/-32 dBFS through the second after the line ends, then falls 8 dB. The full mix cannot
  isolate the tail (music + ambience bed under it): `build/agentIH4/ab/voice_tail.py`.
- **Ride A/B vs retail.mp4** (world 57-150, retail t = world + the run's E2 offset, IH3's `ab_rms.py`):

| run | exe | r linear RMS | r dB | median ours - retail |
|---|---|---|---|---|
| IH3b | IH3 | 0.514 | 0.371 | -7.2 dB |
| IH4a | IH4 build 1 (menu music never stopped) | 0.420 | 0.325 | -4.6 dB |
| IH4b | IH4 build 2 | 0.588 | 0.530 | -5.1 dB |
| IH4c | IH4 build 2 | **0.621** | 0.566 | -4.8 dB |
| IH4d | IH4 build 2 | 0.605 | 0.548 | -5.0 dB |
| IH4e | IH4 build 2 | 0.601 | 0.554 | -4.8 dB |

  Correlation target (>= 0.6) met on 3 of 4 build-2 runs (0.588 on the fourth). Gap target (< 3 dB) **not met**: -4.8 to
  -5.1 dB. What remains (IH4b): w60 -14.4 dB (the second between the soirée enter and the first posts; our ambience and
  music start at 60.4 / 62.1); w105-118 -10 to -13 dB (retail's low drone: under 150 Hz retail has 48.8 dB against our 36.3
  in w86-125, fork G's spectra); a constant ~-4 dB elsewhere (retail.mp4 is an OBS capture at the user's unknown sliders;
  ours are 100/90/90/100).
- **Exit:** 5 of 5 cutscene_compare runs clean, `Preparing to exit` -> `Exiting` 0.22 / 0.23 / 0.31 / 0.24 / 0.23 s
  (IH4a..e), `SoundEngine::Term` inside it. Before: 3 of 3 cutscene_compare-style runs of DishonoredGame_IH3.exe died
  (0xC0000005 on the IO thread, `CAkUnrealIOHookBlocking::Read` <- `CAkDeviceBlocking::ExecuteTask`,
  `build/agentIH4/G/r1_result.txt`, `r2_result.txt`). The brief's 10 of 10 was not run: the regression below needs a third
  build first.

**Gate** (`run_regression.py --only d3d9,inputtest,newgame`, `build/agentIH4/gate.txt`): **11 ok, 12 failed.** newgame passes
(criticals 0, pawns 29, on_mission 120, menu open 49.7 s); `unported_natives` 0. **d3d9 and inputtest die at load with a
Critical:** `UDisMusicState_Exploration::OnEnterState` -> `DisMusicPostStateEvent` -> `AActor::PostAkEvent` on
`ADishonoredPlayerPawn::s_pInstance` -> `UAkAudioDevice::GetAkComponent` reads a dead pawn (the `-startmap=L_Tower_P` route:
`LoadMap` -> `RouteBeginPlay` -> `InitGlobalManagers` -> `Init_GlobalMusic`). Retail (2012 0x8b17f0) posts on the same
pointer, but retail clears it in `ADishonoredPlayerPawn::ShutPawnDown_Derived` (2012 0x70ef70, write at 0x70efaa), called
from `ADishonoredPawn::ShutPawnDown` (0x79d360, 802 B) through `OnActorTerminated` (call at 0x79ed83) and
`MAT_OnDestroyPreviewPawn` (0x79ed70). That chain is unported here, so `s_pInstance` keeps the previous world's pawn and
the music manager is the first new caller to dereference it at BeginPlay. **Not fixed: it needs a third DishonoredGame
build** (port `ShutPawnDown` + `ShutPawnDown_Derived` and the `OnActorTerminated` hook, or as a named stop-gap clear
`s_pInstance` when the player pawn is destroyed). The IH4 tree must not merge before that.

## 4. Where the brief was wrong (with the measurement)

1. **"The exit defect: continuous voices post during UnloadAll"** is not the cause. The IO thread jumps to address 1: at
   exit `UAkBank::Unload` -> `UnloadFilePackage` deletes the package's file reader while a streamed voice still reads
   through it (3 of 3 IH3-exe runs, same last line `UnprepareBank Bank_UI_Stop_All`). Retail reads by name through the
   engine's async IO. Ours also never registered the global callback that retail unregisters first in Teardown (0x5b3de0).
2. **"The engine loop stopping at world ~95"** is retail's own data: the ride matinee posts `Snd_Motorboat_Small_Stop` at
   t=37.92. Retail's bed in w99-118 is the level ambience drone (< 150 Hz) and the music, both now posted.
3. **Play_Amb_LVL_Tower_Intro and the music** are target-less `SeqAct_AkPostEvent`s in L_Tower_Audio / Dishonored_MainMenu,
   not the audio system or AkAmbientSound actors; retail posts them on the global object, our port only on targets. The
   music state comes from `UDisGlobalMusicManager`, which nobody had ported. Music_Menu_Stop is never posted by retail on
   New Game: `UWorld::CleanUpBeforeLevelTransition` stops everything at the commit. Retail's `UnloadBank` does not stop a
   streamed music node either (`CAkPBI::IsUsingThisSlot` 0x93f620 is false without in-bank media).
4. **"Ten effect plug-ins"** are nine effects and one source: MatrixReverb *is* the FDN reverb (`CreateMatrixReverbFX`
   0x9881c0 constructs `CAkFDNReverbFX`, id 115). Retail registers 8 (2013 0x5b1430): StereoDelay and Harmonizer are linked
   into the 2012 exe but never registered (no 2013 match); TimeStretch is registered but no bank uses it. The banks use
   plug-ins 101, 105 (334 objects), 106, 115 (7, Init.bnk), 118 (44, Init.bnk), 131, 136. Delay lives in
   `samples\plugins\akdelay`.
5. **"The 96 IH2 handovers"**: 6 were IH3's (`DynamicSequence::*`), 10 are the feedback-only `CAkSplitterBus`, 1 is a folded
   engine symbol; the BypassFX action was already complete (IH3), only the effect instances were missing.
6. **"Music objects loaded as opaque records"**: IH1's loader skipped them through the declining handler; with M's readers
   the CoreSmoke sweep reads all 15,021 objects of the packaged banks exactly.
7. **The A/B gap target (< 3 dB)** assumes our mix can reach retail's level; retail.mp4's capture level is the user's
   OBS / slider state and stays unknown, a constant ~4 dB of the 4.8.
8. The PDB's `CAkFXSrcSilence` is 36 bytes, not the 40 its members imply. Retail mutes one of the Matrix reverb's 16 lines
   at HF ratio 4.5 (stability clamp), so its low-band RT60 is 0.80 s for a 1.0 s setting, as ours.
9. **"Clean exit 10 of 10 including the gate"**: the new music manager turns a latent defect (the player-pawn singleton
   never cleared, its shutdown chain unported) into a load Critical on the `-startmap` route; two builds were not enough to
   port that chain as well.

## 5. Open

- The gate regression: closed by IH4b (section 6). **The silent exit death** in the exit GC's bank unloads, 3 of 16
  cutscene_compare runs on build 5 (section 6).
- The remaining A/B gap: w60 (the first posts at 60.4), retail's low drone in w105-118, the capture level.
- `CAkSplitterBus` (feedback), `AkFXDurationHandler.h` (no user), the music manager's rat scan, the `PrepareMapChange`
  `StopAllSounds` (2013 0x5e9d50; `AGameInfo` lacks the virtual).

## 6. IH4b: the pawn termination chain (wave 26, after the shutdown)

The first IH4 agent died with the machine mid-build: `build/agentIH4/build_game_3.log` stops at [439/929] with `rc=1` and
no compiler error (the cl processes were killed), so no exe came out of it. IH4b reviewed the partial port against the 2012
and 2013 decompiles, completed it and took two DishonoredGame builds (4: the chain; 5: the NPC's part, which build 4's exit
runs showed is needed). The worktree was then fast-forwarded to main 2cb8df4 (see the sync script); the tested exe is
b08193f + this package.

**What the chain does (retail, 2012 rva / 2013 rva):**

- `AActor::PreBeginPlay` (0x25eb10 / 0x2449d0) starts with `SetActorLegalToTerminate` (0x1731c0 / 0x1694c0).
- An actor terminates once, through `ConditionalTerminateActor` (0x177460 / 0x16d770: never a class default or archetype,
  only a legal one not yet terminated, only while a world exists), called from `AActor::BeginDestroy` (0x17d010),
  `AActor::ClearComponents` (0x17efd0), `MarkComponentsAsPendingKill` (0x17f040), `ADishonoredPawn::ClearComponents`
  (0x79d300) and `OnLevelTerminated` (0x1774c0 / 0x16d7d0). `AActor::PostScriptDestroyed` **is** `OnLevelTerminated`:
  slot +0x1bc of `??_7AActor` (0xbdfe18) and of APawn's vtable hold 0x1774c0 / 0x2bb490, and `APawn::PostScriptDestroyed`
  (0x2bb490 / 0x6a0750) calls it before `RemovePawn`. So a destroyed actor terminates right after its script `Destroyed`,
  and every actor of a world terminates in `UWorld::CleanupWorld` -> `ClearComponents` at each `LoadMap` and at exit, with
  `GWorld` still set.
- `TerminateActor` (0x173180) -> `OnActorTerminated`; `ADishonoredPawn::OnActorTerminated` (0x79ed80 / 0x766830) ->
  `ShutPawnDown` (0x79d360 / 0x761e60), the base's empty body, the relationship interface's.
- `ShutPawnDown`: dispatcher event 18 + the OnDifficultyChange unregister, `BreakAllLOS`, the fish attractor, the map
  info's flee points, the projectiles the pawn instigated, the associated actors to `AddToPendingDestroyList` (0x6484a0 /
  0x601dc0), `m_pCpntObservable = 0`, **`ShutPawnDown_Derived`**, `RemoveAllComponents`, `TerminateInventory`,
  `DeleteAnimTree`, the attachments destroyed.
- `ADishonoredPlayerPawn::ShutPawnDown_Derived` (0x70ef70 / 0x6b5320): the anim states, the three player FSMs,
  **`s_pInstance = 0`** (0x70efaa), the tutorial trackers' unregister.
- `ADishonoredNPCPawn::ShutPawnDown_Derived` (0x7c1d20, 271 B / 0x76a880, 549 B): the squad count, the travel and AI
  managers, **the controller unpossessed and destroyed**, the master FSM, the dialog, the pawn channel, the avoidable, the
  conversation component, the six component pointers zeroed (2013 adds the spring-razor event and the stealable).

On the `-startmap` route the first world (`DishonoredGameFull_P`) has a player pawn; `LoadMap`'s `CleanupWorld` now
terminates it, `s_pInstance` is 0 when L_Tower_P's `InitGlobalManagers` runs, and `UDisMusicState_Exploration::OnEnterState`
posts `Music_State_Brothel_None` on the global object (`on game object 2`, IH4bg2__d3d9.log:575) as retail would. On the
New Game route the tower streams into the same world, the pawn stays and the post goes on the live pawn.

**Ported in IH4b:** `AActor::PostScriptDestroyed` = `OnLevelTerminated` (EngineClasses.h); `ShutPawnDown` completed with
the associated actors (`ADishonoredGameInfo::AddToPendingDestroyList`); `ADishonoredPlayerPawn::ShutPawnDown_Derived`
clears the singleton unconditionally as retail (the partial port cleared it only when it was `this`);
`ADishonoredNPCPawn::ShutPawnDown_Derived` (controller, master FSM, component pointers). **Named, not ported** (no bodies
in this tree; each in its function banner): the dispatcher events, `BreakAllLOS` (`FDisComponentObservable::ResetLOS`
0x8b9aa0), `DestroyAttractor` / `OnPawnShutDown_MapInfo` -> `UDisFleePointManager::OnPawnShutDown_FleePointManager`
0x8883f0, the projectile manager, `TerminateInventory` 0x85a610 (needs `TerminateAllContextsForSlot` 0x858330 ->
`UDisItemContext::TerminateContext`), the `m_DestroyWhenNotVisible` consumer in `ADishonoredGameInfo::Tick`, the NPC's
squad count / travel manager / AI manager / dialog / pawn channel / avoidable / LOD unregister / conversation shutdown,
`ADisTallboyNPCPawn::ShutPawnDown_Derived` 0x7f2700, `ADishonoredPawn::ClearComponents` 0x79d300 (retail also empties
`Components` / `AllComponents` after clearing), `UWorld::RemoveFromWorld`'s own `OnLevelTerminated` call (its
`ClearComponents` reaches the same termination). Deviations: NULL tests on `m_ComponentContainer`, the game info and the
FSMs (retail calls through them unconditionally).

**Measurements (build 5, staged `DishonoredGame_IH4.exe`):**

- Gate `run_regression.py --build-dir build/agentIH4 --no-build --exe-name DishonoredGame_IH4.exe --only
  d3d9,inputtest,newgame` (`build/agentIH4/gate_b4.txt`): **22 ok, 1 failed** - `d3d9_criticals`, `inputtest_criticals`,
  `newgame_criticals` **0**, `unported_natives` **0**, `probe_natives` **3**, `d3d9_frames` 1620, `d3d9_draw_elements`
  6506, `newgame_pawns` 29, `newgame_on_mission` 92; the failure is `inputtest_moved` 402.5 / 673.7 under load. inputtest
  alone (`gate_b6.txt`): **9 ok, 0 failed**, moved 1009.3. Load control: `DishonoredGame_IN.exe` (the a47f38b gate content)
  measured 384.0 / 218.4 in the same window as ours' 431.8 / 217.2 (`gate_ctl_IN.txt`, `gate_b5.txt`): W is held through a
  ~2 s first-frame stall on a loaded machine. Build 4: `gate_b.txt` 21 ok (criticals 0; d3d9_frames 840 and inputtest_moved
  497 under load), d3d9 re-run ok (`gate_b2.txt`), inputtest 1003.1 (`gate_b3.txt`).
- CoreSmoke (`build/agentIH4/coresmoke_run4.log`, cwd the worktree, `WWISE_VORBIS_REF` set): **203 passed, 0 failed, 0
  skipped**. Under load (`coresmoke_run2.log`) the two real-time checks (AK_EndOfEvent, the music transition bar) failed once.
- Exits. Build 4 died in 3 of 10 cutscene_compare exits (Critical: `UWorld::CleanupWorld` -> NPC controller
  `ClearComponents` -> `TerminateBrain` -> `StopLoco` on the locomotion component the pawn's `ShutPawnDown` had already
  removed, whenever the pawn came before its controller in the level's actor list; retail's NPC `ShutPawnDown_Derived`
  destroys the controller first - ported in build 5). Build 5: **cutscene_compare 13 of 16 clean** (IH4p..y, IH4za..zf),
  `Preparing to exit` -> `Exiting` 0.26 / 0.22 / 1.14 / 1.92 / 0.22 / 0.61 / 1.09 / 0.32 / 0.30 / 0.23 / 0.27 / 1.18 / 1.01 s
  (the > 1 s ones while other agents loaded the machine); **-startmap exits 9 of 9 clean** (`build/agentIH4/exits/`,
  0.23-0.36 s). **3 of 16 cutscene exits (IH4t, IH4v, IH4w) end silently**: no Critical, no `Exiting`, each right after
  the 27th `UnprepareBank` (`WwiseDefaultBank_WwiseGlobal.bnk`) of the exit GC, where the clean runs go on to "continuous
  PBI ended: next posted" and unprepare the level ambience banks (`Bank_Amb_*`). That is IH3's symptom ("2 of 9 runs end
  silently during the exit's bank unloads"): the continuous ambience containers keep posting their next item on the audio
  thread while the exit GC unprepares their banks. The -startmap exits have no continuous PBI at that point. Build 4 had 0
  such deaths in the 7 runs that got past CleanupWorld, the first IH4 0 of 5; build 5 changes nothing in Wwise, and whether
  the NPC controllers now destroyed at exit shift that race is not measured (build 5 overwrote build 4's exe). **The 10 of
  10 is not met.** No WER record or dump exists (the thread dies outside the engine's handler). Next: a minidump of the
  dying thread, and retail's order of `StopAllSounds` / `Teardown` (2013 0x5b3de0) against the exit purge.
- Ride A/B (IH4p, `build/agentIH4/ab/IH4p_ab.md`, IH3's `ab_rms.py`, retail t = world - 42.58): **r linear 0.659**, r dB
  0.619, median **-4.8 dB** (IH4 build 2: 0.588-0.621 / -4.8..-5.1); build 4's IH4f 0.633 / 0.570 / -4.8 dB. Ride SSIM 0.641.
- Regeneration: `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`, then `--out-dir`: 79 of 79
  byte-identical on b08193f, 2898de5, d947504, fd3f90b and 2cb8df4; `bitfield_audit.py` mismatches 0.

**Where the previous report was wrong:**

1. `MAT_OnDestroyPreviewPawn` (0x79ed70) is a caller of `ShutPawnDown` but editor-only; the game's only path is
   `OnActorTerminated` through `ConditionalTerminateActor`, from five places including `AActor::PostScriptDestroyed`,
   which the report did not name (folded onto `OnLevelTerminated`).
2. "port `ShutPawnDown` + `ShutPawnDown_Derived` and the `OnActorTerminated` hook" was not enough: the base chain alone
   turned 3 of 10 exits into a use-after-free (the pawn's components went before its controller's brain let go of them).
   Retail's NPC override destroys the controller inside the shutdown; it is part of the chain.
3. The partial port's "clear the singleton only when it is this pawn" deviation was unnecessary: the constructor never
   sets it for a class default or archetype, and retail clears it unconditionally.
4. "Clean exit 10 of 10 after one build": the IH3-era silent death in the exit's bank unloads is not gone (3 of 16).
