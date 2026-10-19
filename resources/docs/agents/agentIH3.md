# Agent IH3 (wave 26, PHASE23 IH3): the parameter graph

Third package of the IH program. RTPCs, states, switches, the node and bus parameter evaluation, 3D positioning
(attenuation, cone, spread, multi-position, obstruction/occlusion), user-defined paths, the random / sequence / switch /
layer containers with continuous playback, PlayAndContinue and dynamic sequences now run on ported Audiokinetic 2012.1
code. On the game side, the matinee Ak tracks, `UAnimNotify_AkEvent`, the ambient sounds, the rest of
`UDishonoredAudioSystem::Update` and the portal path search are ported, so the opening's sound effects are posted and
mixed at the banks' levels.

Worktree `build/agentIH3_wt` (made at `dfb1158`), build dir `build/agentIH3` (RelWithDebInfo), staged as
`DishonoredGame_IH3.exe`. Two DishonoredGame builds: build 1 had everything; build 2 added two shutdown fixes (section 4).
Five CoreSmoke builds. The work was split over five forks of this agent (R game syncs, N node graph, P positioning,
C containers, G game side), each owning its own files. All decompiles are under `build/agentIH3/` (`dec2012_ih3`,
`dec2012_ih2`, `G/dec12`, `G/dec13`). Private IDA copies are in `build/agentIH3/idb`. No fmodel, no real input, no retail exe.
Nothing was committed. The sync script is `build/agentIH3_sync.py`.

## 1. What was ported

All paths are under `S/External/Wwise2012/Src/AkAudioLib/Common` unless stated. Every file banner names the 2012 rvas and
deviations. Full list: `resources/docs/wwise/ih3_inventory.csv` (620 rows, generator `build/agentIH3/tools/make_inventory.py`).

| Area | Files | Retail (2012) |
|---|---|---|
| RTPC manager: entries per game object, transitions, subscriptions with curves, switch to RTPC, layers | `AkRTPCMgr.h/.cpp` | all 38 of akrtpcmgr.cpp/.h, 0x930ce0..0x933ea0; `CAkConversionTable::ConvertInternal` 0x930ec0 / 0x931040 |
| State manager, state transitions, triggers, preparation | `AkStateMgr.h/.cpp`; AkStateGroupChunk runtime | all 23, 0x92e760..0x92f6e0; chunk 0x945530 / 0x9444d0 / 0x944550 |
| Environments (obs/occ curves), switch aware, preparation aware | `AkEnvironmentsMgr.*`, `AkSwitchAware.*` | 0x933ec0..0x9340f0, 0x95e970..0x95eb60 |
| Game-sync API (queued as retail) + Query | `AkSilentSoundEngine.cpp`, `AkAudioMgr.*` (messages 2/3/5/6/7/8/10/20/21) | SetRTPCValue, ResetRTPCValue, SetSwitch, SetState, PostTrigger, environments; Query GetRTPCValue / GetSwitch / GetEnvironment* |
| Node base / node: state values and sync types, SIS (SetAkProp / Mute and their transitions), RTPC callbacks, notifications, virtual counts, FX bookkeeping | `AkParameterNodeBase.cpp`, `AkParameterNode.cpp`, `AkNodePlayback.cpp` | akparameternodebase.cpp, akparameternode.cpp, aksoundbase.cpp, akactiveparent.h (rvas in the files) |
| GetAudioParameters with states, RTPC, global and per-object SIS, mute map, ducking, layer crossfades | `AkNodePlayback.cpp` | node 0x938xxx, bus 0x93c890 |
| Bus: ducking state machine, duck items, bus volume / pitch / LPF tree as the lower engine reads it | `AkBus.cpp`, `AkBusCtx.cpp` | akbus.cpp, akduckitem.cpp, `GetBusEffectiveVolume` 0x93c890 |
| PBI parameter half | `AkPBI.cpp` | `ParamNotification` 0x9402e0, `UpdateFx` 0x93f500, `SetParam` 0x93f1a0 |
| Value-setting actions | `AkActionsSetValue.cpp` | SetValue, Mute, SetAkProp, SetGameParameter, SetState, SetSwitch, UseState, Trigger, Duck, BypassFX (bits), Seek |
| 3D: listener (`Get3DVolumes` 0x935400, 6447 B), cone, `GetListenerVolume`, `IsInitiallyUnderThreshold`, the speaker-matrix callback | `AkListener.*` | ak3dlistener.cpp |
| Voice 3D mix | `AkVPLSrcCbxNode.cpp` | `GetVolumes` 0x925970 3D branch, `AddSrc` 0x926b80 |
| PBI positioning, attenuation RTPC, paths, positions | `AkPositioning.cpp`, `AkPath.*`, `AkRegistryMgr.*` | `Get3DPositioningParams` 0x9411b0, `PositioningChangeNotification` 0x941030, CAkPath 0x952f90..0x953900, CAkPathManager 0x92f740..0x92fb80, `AkPositionKeeper::SetPosition` 0x937070, `Query::GetMaxRadius` 0x91b180 |
| Random / sequence containers, playlists, selection state | `AkRanSeqCntr.cpp`, `AkContainers.h` | 0x94d640..0x94f8e0, 0x95e630..0x95e910, 0x95dee0..0x95e280 |
| Multi-play node, continuation lists, continuous PBI, PlayAndContinue | `AkMultiPlayNode.cpp`, `AkContinuousPBI.*`, `AkActionPlayAndContinue.*` | 0x955220..0x955760, 0x94d2e0..0x94d490, 0x94bf30..0x94cdd0, 0x9493f0..0x949b30 |
| Switch container (incl. per-object switch history), layer container and layers | `AkSwitchCntr.cpp`, `AkLayer.cpp` | 0x94f8e0..0x951480, 0x951560..0x9517e0, 0x954060..0x954b40 |
| Dynamic sequences + API | `AkDynamicSequence.*`, `include/AK/SoundEngine/Common/AkDynamicSequence.h` | akdynamicsequence(pbi).cpp, messages 24/25 |
| Matinee Ak tracks | `S/AkAudio/Src/akaudioclasses.cpp` + 3 CppText | UInterpTrackAkEvent / InstAkEvent / AkRTPC (2012 0x5f5e30..0x5fb9f0, 2013 0x5afaa0..0x5b2e40) |
| Anim notify, audio system Update (ambient sounds, cell ambience + RTPCs, environments, time dilation RTPCs, pause), ApplyGameSettings, portal path search | `S/DishonoredGame/Src/dishonoredaudiosystem.cpp`, `disaudiopropagationinfo.cpp`, `dishonoredaudioportal.cpp`, `Inc/disaudiopropagation.h` | `UAnimNotify_AkEvent::Notify` 0x316bd0, `Update` 0x7b3520, `UpdateAudioPropagationCache` 0x7a3700, `FindShortestPath` 0x792320, `FinalizeAudioPath` 0x79d5e0, `DisAudioPropagationFindPathInfo` 0x786d50, `ApplyGameSettings` 0x7872b0 (2013) |
| Shutdown fixes (IH1 defects) | `AkHierarchy.cpp`, `AkBankMgr.cpp` | `CAkAction::AddToIndex` 0x94a0f0 / `Release` 0x94a140 key-0 test; `CAkBankMgr::ClearPreparedEvents` 0x92dbf0 iterator order |

Generated files: `gen_classes_header.py AkAudio` / `DishonoredGame --sdk --module-header --sources-cmake` in the worktree.
The changes are 5 CppText includes and `dishonoredaudioportal.cpp` off the exclude list (677 -> 676). A second `--out-dir`
run is byte-identical. **Main changed `dishonoredgameclasses.h` and `Sources.cmake` since `dfb1158`: regenerate at the
merge, do not copy** (the sync script flags them).

## 2. Named deviations (each also in its file banner)

**Game syncs and node graph**
- `AkStateMgr`: `AddStateGroupMember` refuses a group STMG did not declare, as retail does. Measured: 0 of 15,116
  objects reference one (`build/agentIH3/hirc_parse.py`). A state value new to a chunk starts its transition at 0.
- `AK::IAkRTPCSubscriber` lives in `AkRTPCMgr.h`, with only the SetParam slot; IH4 moves it next to the plug-ins.
- FX: the bypass bits are kept and refreshed, but no insert effect instance exists to honour them (IH4).
- `CAkStateSyncArray` holds AkUInt32 instead of the enum.

**Positioning**
- `InitRotationMatricesForNoFollowMode` writes without advancing the pointer, as the decompile does. The two agree with
  one listener, which is all Dishonored uses.
- The `PAN_FR` branch of `UpdateBaseParamsFromRTPC` scales `m_fPAN_RL`, as the 2012 bytes read.
- The multi-sources dB uses log10 where retail uses fyl2x.
- The feedback (motion) branches are not ported: no feedback device is ever created.

**Containers**
- Smart pointers are raw pointers with explicit AddRef/Release.
- `CAkContinueListItem` has a copy constructor, because the port's array grows by copy.
- The switch container reaches its extra bases by compiler adjustment instead of thunks.
- The six dynamic-sequence commands share one function.
- Decision tree and dialogue event: not ported, since no bank has one.
- Feedback bus/node/device: not ported, since no device is created and no feedback object exists in any bank.

**Game side**
- The key-properties flags are read through their `UBoolProperty`.
- 2013's ambient time dilation is not ported, because `SetAkComponentsTimeDilation` is absent here; the 2012 loop shape is kept.
- `GOverseerEffectRTPC` has no writer, so it stays 0.
- The Bink half of `ApplyGameSettings` is not ported: there is no volume member.
- `UAkAudioDevice::Update` renders through the audio system's `Update` tail, as retail does.
- Not ported: `IsSoundAudible`, `GetMinAudibleDistance`, `PostAkEventAtPoint`, `UDisAkComponent::OnTimeDilationChanged`
  (no hook), and the editor-only key functions.

**Diagnostics**
- `-akdiag` adds:
  - per voice, once a second or on change: the volume split as node chain / states / RTPC / SIS plus the bus tree, pitch and LPF;
  - for the first 3D voice each second: distance, curves, cone and obs/occ;
  - every SetState / SetSwitch / SetRTPCValue (rate-limited) / PostTrigger / SetValue action;
  - every container decision and PlayAndContinue;
  - every multi-position set;
  - every matinee Ak key post.
- `-ih3diag` (also on with `-akdiag`): the ambient sounds' register/start/stop, the cell ambience, the volume RTPCs, anim
  notifies, and once a second every voice whose sound cell differs from the listener's, with its paths.

## 3. Measurements

**CoreSmoke: 155 passed, 0 failed, 1 skipped** (`build/agentIH3/coresmoke_run4.log`). That is 144 + 1 skipped before plus 11
new checks in `WwiseSmoke.cpp` `TestGraph`. The skip is IH2's Vorbis SNR check, which needs reference PCM that is not in
this tree. The subjects were read from the banks by `build/agentIH3/hirc_parse.py`, an independent python reader of the node
bodies that consumes all 15,116 objects exactly. The voice line 0x57447bbe plays through actor-mixer 0x3720c159 (3D,
attenuation 0x823cc49) to bus 0x30b38306 (volume RTPC 0xcd107ced, curve 0..100 -> -96.3..0, dB scaling), then
0x28eb7999, 0x2aac29b6 and the master 0x26df8722 (-4 dB). Each figure is 10·log10 of a whole-play energy ratio through the
sink:

| check | measured | bank |
|---|---|---|
| RTPC 0xcd107ced = 50 | -6.02 dB | -6.02 dB |
| RTPC 0xcd107ced = 25 | -12.04 dB | -12.04 dB |
| state 0x82a425ff of group 0xad7ac6ca (default 2500 ms) | silent after 2.8 s | bus 0x28eb7999 Volume -96.3 |
| back to state none | 0.00 dB | 0 |
| state 0x531639fd of group 0x3c28da9f | -40.3 dB (the -akdiag voice volume reads exactly -20.00 more, plus LPF 43) | Volume -20, LPF 43 |
| 3D, 1145.6 vs 441.7 units | -4.03 dB | -4.11 dB |
| 3D, 1951.5 vs 441.7 units | -12.66 dB | -12.74 dB |
| 3D, 1951.5 vs 1145.6 units | -8.63 dB | -8.63 dB |
| 3D, 3500 units (curve ends at 3000) | silent | -96.3 |

**Game** (`-finewgame=50 -fosoiree -nomovie -akdiag -akwav`, through `build/agentIH3/cc/cutscene_compare_ih3.py`, a copy that
writes under `build/agentIH3/cmp`).

What is posted:
- **292 PostEvents by world 152** (IH2: 36).
- **Matinee keys:** the boat engine and hull (Snd_Motorboat_Small_Lift_Water_Impact / Start / Speed_Slow / WaterClap_Speed /
  Stop / WaterClap_Idle / Lift_End), Whale_Ship_Gear / Horn, Door_Metal_Huge, Snd_Drawbridge_Top_Impact.
- **Ambient sounds:** 28 registered; started inside their max radius and stopped outside it (Amb_Seagull, Amb_Ocean_Distant,
  Amb_Water_Lap_Dock / Lock / Lock_Wheel, Amb_Waterfall_Ground_Big_Tower, Amb_Metal_Groan, Amb_Wind_Moan_*, Snd_Amb_Steam).
- **Cell ambience:** listener cell DishonoredAudioVolume_2 posts Set_State_Amb_LVL_Tower_Intro_Ocean, later _In_01.
- **Anim notifies:** Foley (Civ_Soko_Paint, Snd_AI_Towel_Mvt, scrub floor, clothes).
- **Continuous containers:** 65 in the opening banks run PlayAndContinue chains (footsteps are trigger-rate 1200 ms ones).
- **Volume sliders:** Global 100, Music 90, SFX 90, Voices 100.

FLT_MAX resolved:
- DishonoredNPCPawn_4 talks from DishonoredAudioVolume_0 while the listener is in _4 (radius 10,000): 1 path, shortest 4201 uu, live.
- _3: 1 path of 3827 uu (raw 3785) with obstruction 0.06.
- Ambient sources in other cells also get 1 path each.
- Other-cell voices outside their radius are still not searched, as retail does.

**A/B against retail.mp4's audio track.** The track is extracted to `build/agentIH3/ab/retail_audio.wav` (ffmpeg, 48 kHz
stereo). Retail t = world + E2 offset (cutscene_compare's constant, b = 1). Each run's offset is its own (-42.42..-42.67 s).
The window is world 57-150. The table is in `build/agentIH3/ab/*_ab.md`, the plot in `build/agentIH3/ab/ride_rms_ab.png`.

| run | exe | Pearson r, linear RMS | r, dB | median ours - retail |
|---|---|---|---|---|
| IH3base | IH2 (before) | 0.203 | 0.214 | -6.5 dB (silent, below -80 dB, 37 of 93 s; IH3b: 1) |
| IH3a | IH3 build 1 | 0.491 | 0.368 | -7.3 dB |
| IH3b | IH3 build 2 | **0.514** | 0.371 | -7.2 dB |
| IH3c | IH3 build 2 | 0.518 | 0.386 | |
| IH3e | IH3 build 2 | 0.485 | 0.392 | |

The biggest gaps (IH3b, ours quieter):
- **w59, -60.7 dB.** Ours is silent here in every run, IH2's included: the second between the soirée enter and the first post.
- **w99-118, -10 to -22 dB.** w106 is -18.5, w115 -21.1; the region peaks at w117, where retail is at -21 dB. Our boat-engine
  loop stops at world ~95 (Snd_Motorboat_Small_Stop). Retail keeps a loud bed there, most likely the level music and the
  ambience bed (below).
- **w64 -22.8, w85 -17.0, w73 -15.4.** Short troughs between lines.
- **A constant ~-7 dB elsewhere.**

Named causes:
1. **Music.** Bank_Music_Tower_Intro, Music_Tower_Intro and the LVL_Tower_Intro music are music-engine objects (IH4). None plays.
2. **Play_Amb_LVL_Tower_Intro is never posted.** Its radius is computed at map commit, next to Snd_Fade_In_All (which is
   posted), but no post follows. It is a 2D + 3D ambience event (7 objects, one 3D with max distance 4000).
3. **Recording level.** OBS captured the OS mix at the user's settings. Retail's sliders are unknown; ours are 90 for
   SFX/Music. That gives a constant offset that does not move r.

**Gate:** `run_regression.py --only d3d9,inputtest,newgame` gave **23 ok, 0 failed** (`build/agentIH3/gate.txt`), every
metric first try:
- d3d9 / inputtest / newgame criticals 0 / 0 / 0
- unported_natives 0
- probe_natives 3
- newgame_pawns 29
- newgame_on_mission 120
- inputtest_moved 1059.6
- d3d9_frames 2010

**Term:** "Preparing to exit" -> "Exiting" takes 0.21 s (IH3b); SoundEngine::Term runs inside it.

## 4. Shutdown (the coordinator's IK4 crash and this package's own hang)

Two IH1 defects, both fixed in build 2:
1. **`CAkBankMgr::ClearPreparedEvents` advanced its index iterator before `UnprepareEvent`.** Retail 0x92dbf0 reads the next
   item after it. The unprepare releases a bank structure whose content can be the *next* event, so the loop walked into a
   destroyed `CAkEvent`. That is IK4's "Pure virtual function being called" in `UnloadAll` (CAkIndexable's vtable left
   behind by the destructor).
2. **IH1's `CAkAction::AddToIndex` / `Release` indexed every action, including runtime key-0 ones.** Retail 0x94a0f0 /
   0x94a140 skip key 0. IH2's StopAll action and IH3's PlayAndContinue are created with key 0, and `RemoveID(0)` unlinks some
   other key-0 action. The freed one stays in the chain. In build 1 the bank thread spun forever in `RemoveID` during exit
   (IH3a: 18 min CPU, stack `RemoveID <- CAkEvent::Clear <- CAkEvent::Release <- CAkUsageSlot::RemoveContent <- ReleasePrepare`).

Exits on build 2:
- **Clean in 7 of 9 runs:** IH3b, IH3e, three `game_input` ride runs quitting at world 75 / 152 / 152, and two runs under `dbg.py` (the second with cutscene_compare's arguments: -ResX 1920, -fzdiag, 107 -apshottime marks, -akwav).

- **Open: in 2 of 9 runs (IH3c, IH3d, both cutscene_compare runs) the process ended during the exit's bank unloads.** That was
  before SoundEngine::Term, with no Critical, no WER event and no WAV finalize. In IH3c, continuous PBIs were still posting
  PlayAndContinue (164.19) while the game's UnloadBank calls ran (164.24). The last line is the UnprepareBank of
  Bank_UI_Stop_All. Not caught yet; this is the first thing to look at in IH4 or a follow-up, under `dbg.py` with the
  cutscene_compare arguments.

## 5. Where the brief was wrong (with the measurement)

1. **"RTPC/switch/state calls are bookkeeping only" was not the only reason effects were missing.** The game posted no sound
   effect at all. IH2b had 36 posts, all voices plus Snd_Fade_In_All, because the posting paths on the game side were dead:
   the matinee Ak tracks (`akaudioclasses.cpp`: "not ported"), `UAnimNotify_AkEvent::Notify`, and ambient sounds registering
   with an empty `UAudioSystem` base. Fixing those took the run to 292 posts.
2. **IH2's "PlayAndContinue only comes from continuous containers (none in the banks)" is wrong.** There are 140 continuous
   random/sequence containers in the 312 packaged banks, 65 in the opening's banks (25 delay, 15 none, 11 cross-fade amp, 4
   cross-fade power, 10 trigger rate). The opening's footsteps (Bank_Footsteps, trigger rate 1200 ms) and seagull / wind
   one-shots use them. The run shows hundreds of PlayAndContinue executions.
3. **"Snd_Fade_In_All is a fade" (IH2) is wrong.** Action type 0x1204 is a SetState; with real states it drives the bus
   volumes from the start.
4. **CAkListener::Get3DVolumes (6,447 B) was marked "ported" in ih2_inventory.csv but did not exist.** Its name only matched.
   The real port is here, along with `CAkPBI::SetParam` (0x93f1a0, also listed as ported).
5. **"CAkLEngineCmds" (2 rows) are static initialisers**, already IH2's. **"CAkDynamicSequence"** has no caller in the game
   (grep of the glue: 0 uses). It is ported because the brief lists it.
6. **The bus volume RTPCs.** Every Init.bnk bus volume goes through an RTPC (Global / Music / SFX / Voices, 0..100 ->
   -96.3..0 dB, dB scaling). `UDishonoredAudioSystem::ApplyGameSettings` was an empty body. Without it, IH3's real RTPC
   evaluation falls back to the bank defaults (100). Ported; the game now sets 100/90/90/100.
7. **The IK4 shutdown crash is IH1's, not IH2's** (section 4). The IH3 build-1 exit hang had the same origin.
8. **"CAkSwitchMgr"** does not exist in 2012.1: switches live in `CAkRTPCMgr` (`SetSwitchInternal` / `GetSwitch`) and
   `CAkSwitchAware`. Feedback is 51 inventory rows, not "a stub". With no device, retail's own NULL test is all that runs.
9. **The FLT_MAX case is narrower than stated.** On the boat every speaker shares the listener's cell (IE). The other-cell
   voices of the opening are NPCs 3 and 4 after world 117. Those now resolve: paths of 4201 and 3827 uu.

## 6. Re-sized IH4

About **830 functions / ~182 KB**: the music engine (342), the effect plug-ins (387), IH2's 96 (insert effects
`SetInsertFx` / GetFX, environmental sends `GetEnvironmentalBus` / `MergeLastAndCurrentValues`, source plug-ins), and this
package's 3 (`GetEnvironmentalBus`, `MergeLastAndCurrentValues`, `CAkFxBase::GetMediaID`).

On this route IH4 decides most of the remaining A/B gap: Music_Tower_Intro and the LVL_Tower_Intro music. Also on its
list:
- the open exit death (section 4);
- the unposted Play_Amb_LVL_Tower_Intro (find its poster: Kismet `USeqAct_AkPostEvent` or the map's audio actors);
- `AK::IAkRTPCSubscriber` moving to IAkPlugin.h;
- footsteps: `UAnimNotify_Footstep` is stock, and Dishonored's material footstep path was not traced here.
