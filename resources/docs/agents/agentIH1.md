# Agent IH1 (wave 26, PHASE23 IH1): Wwise 2012.1 memory + stream + bank manager

First package of the IH program (reimplement the Wwise 2012.1 runtime from the retail binaries, no SDK). The bank side
of the sound engine now runs on ported Audiokinetic code; playback stays agent AN's silent backend until IH2.

## 1. What was ported (all under `source/Development/Src/External/Wwise2012/Src`, behind `DISHONORED_WWISE_SILENT`)

| Module | Files | Functions (2012 PDB) |
|---|---|---|
| AK::MemoryMgr + TLSF 2.x | `AkMemoryMgr/AkMemoryMgr.cpp`, `tlsf.cpp/.h` | 21 + 13 |
| AK::StreamMgr (CAkStreamMgr, CAkDeviceBase/Blocking, CAkIOThread "AK::IOThread", CAkStdStmBase, CAkAutoStmBase, CAkIOMemMgr) | `StreamManager/*` | 161 of 185 (24 are the deferred lined-up device, see §3.5) |
| SDK container templates (AkArray, AkListBare(Light), AkHashList(Bare), CAkKeyArray, AkSortedKeyArray, CAkList2/CAkKeyList, AkPropBundle, CAkConversionTable) | `AkAudioLib/Common/AkPrivateTypes.h` | header templates, PDB layouts |
| CAkAudioLibIndex / CAkIndexable / CAkIndexItem (0x1F40 / 800 bytes, static_asserted) | `AkAudioLibIndex.h` | |
| CAkBankMgr ("AK::BankManager" thread + queue), CAkBankReader, CAkUsageSlot, CAkBankList, CAkBankCallbackMgr, AkMediaEntry, AkSyncCaller | `AkBankMgr.h/.cpp`, `AkBankReader.cpp` | 82 + 12 + 14 |
| HIRC hierarchy: CAkParameterNodeBase/Node, CAkSound+CAkSource, CAkActorMixer, CAkRanSeqCntr+playlists, CAkSwitchCntr, CAkLayerCntr+CAkLayer, CAkBus, CAkState, CAkAttenuation, CAkFxShareSet/Custom, CAkEvent, every CAkAction subclass, CAkGen3DParams(Ex), AkStateGroupChunk | `AkHierarchy.h/.cpp` | 107 (the whole SetInitialValues closure, `hirc_closure` of ProcessHircChunk) |
| Registration side of CAkStateMgr / CAkRTPCMgr / CAkEnvironmentsMgr (STMG, ENVS, RTPC + state chunks) | `AkGameSyncs.h/.cpp` | data stores (IH3 replaces) |
| CAkRegistryMgr / CAkRegisteredObj | `AkRegistryMgr.h/.cpp` | 21 |
| AK::SoundEngine bank API (LoadBank x8, UnloadBank x6, PrepareBank x6, PrepareEvent x6, PrepareGameSyncs x4, ClearBanks, ClearPreparedEvents, CancelBankCallbackCookie, SetBankLoadIOSettings), GetIDFromString / GetBankIDFromString, Init/Term bank half, default settings, Query (QueryAudioObjectIDs, GetPositioningInfo, GetPosition, GetActiveListeners, GetIsGameObjectActive, GetGameObjectEnvironmentsValues, GetGameObjectDryLevelValue, GetObjectObstructionAndOcclusion, GetState) | `AkAudioLib.cpp` | 49 |

Inventory: `resources/docs/wwise/ih1_inventory.csv` (3,070 runtime functions, 2,907 matched to 2013; IH1 owner 597: 471 ported,
62 header templates, 38 deferred-device, 26 left - media preparation, switch history/modified nodes, music/feedback readers,
3 static initialisers). Conventions for IH2-IH4: `resources/docs/wwise/wwise_program.md`.
`AkSilentMemoryMgr.cpp` and `AkSilentStreamMgr.cpp` are deleted; `AkSilentSoundEngine.cpp` keeps playback (PostEvent ids,
end-of-event on next RenderAudio for the Kismet latents, agent FI) and calls `AkAudioLibInit/Term`, mirrors game objects into
the registry and SetState into CAkStateMgr. `AkSilentMusicEngine.cpp` installs the music bank-item handler (declines; IH4).
Threads as retail: `AK::IOThread` (blocking device created by the glue's AK_SCHEDULER_BLOCKING) and `AK::BankManager`
(g_PDSettings.threadBankManager); every synchronous call queues and waits on DefaultBankCallbackFunc.

Named deviations (each in its file banner): KillSlot releases the slot directly instead of message 26 to the audio manager
(IH2); media preparation for PrepareBank(AllInclusive)/PrepareEvent not ported (Dishonored only prepares StructureOnly and never
calls PrepareEvent); plug-in parameter objects not created (the glue registers NULL factories, so retail's own
AllocParams-fails path is taken); feedback / dialogue-event objects skipped (none exist in the data); game-sync managers are data
stores; the registry is called on the game thread under g_csMain (no audio-thread message queue before IH2); a diagnostic
`g_pHircParseEnd` check (every created object must read exactly its HIRC section).

`-akdiag` (AkAudio sets `DishonoredWwise::SetDiagnostics`): every bank load / prepare / unload with name, id, result, bytes,
HIRC objects by type, failed and misparsed counts; every GetIDFromString with its answer; every QueryAudioObjectIDs /
GetPositioningInfo answer; every stream-manager open; every PostEvent of an event that no loaded bank declares.

New tool: `resources/tools/wwise_bank_dump.py` (stdlib; bank v65 + AKPK; HIRC counts by type, `--csv`, `--hirc`, `--fnv`,
`--find-media`). Init.bnk: v65, id 0x50c63a23, 14,052 bytes, chunks BKHD/STMG/HIRC/ENVS, HIRC 95 = State 13, Bus 27,
FxShareSet 55. Init + 311 `.pck`: 312 banks, 15,116 objects; `--all` (+670 Bk_* dialogue containers): 982 banks, 42,548
objects, 0 parse errors, no DialogueEvent / FeedbackBus / FeedbackNode anywhere.

## 2. Measurements

- **CoreSmoke** (`source/Tests/CoreSmoke/WwiseSmoke.cpp`, 28 checks; CoreSmoke 99 -> 127 passed, 0 failed): Init.bnk through
  MemoryMgr -> StreamMgr -> blocking device -> bank thread: result AK_Success, id 0x50c63a23, 14,052 bytes, 95 objects (State 13,
  Bus 27, FxShareSet 55) = python; index holds 27 buses / 13 states / 55 share sets; second load AK_BankAlreadyLoaded.
  **Sweep of every packaged bank** (in-memory load after Init, compare with `wwise_bank_dump.py --csv`, unload): 311 `.pck`
  banks -> 15,021 objects; with `--all` CSV 981 banks -> **42,453 objects, counts by type identical for every bank, 0 failed,
  0 misparsed** (every non-music object read exactly its section), every index back to Init-only after each unload, empty
  after the last; orderly Term of sound engine, device, stream manager, memory manager.
- **Game run** `-windowed -nomovie -finewgame=50 -fosoiree -akdiag` (IH1a.log): 119 bank operations in the opening (69 LoadBank,
  50 PrepareBank StructureOnly), all AK_Success, **119/119 identical to the python counts by bank id (size and every HIRC type),
  9,369 objects, 0 failed, 0 misparsed**; 284 GetIDFromString; 7 QueryAudioObjectIDs / 7 GetPositioningInfo; 68 UnloadBank at
  the map change; 0 PostEvent of an unknown event. The whole Wwise init including Init.bnk + 16 referenced banks takes < 1 s of
  the startup (log 24.78 -> 25.59 s on a loaded machine).
- **Gate** (`run_regression.py --build-dir ../agentIH1 --no-build --exe-name DishonoredGame_IH1.exe --log-prefix IH1g_ --only
  coresmoke,d3d9,inputtest,newgame --game-extra-args=-nomovie`, `build/agentIH1/gate.txt`): 24 ok, 1 failed:
  coresmoke_passed 127, coresmoke_failed 0; d3d9_criticals 0, inputtest_criticals 0, newgame_criticals 0; unported_natives 0;
  probe_natives 4; newgame_on_mission 105; **newgame_pawns 29**; inputtest_moved 1046.9 (on the load retry; 458.5 first);
  **d3d9_frames 480 (bound 1000) - FAILED under load**: 32 cl.exe of four other agents' builds and two other games were running
  (d3d9 startup 25.6 s against 3.4 s idle; texture upload alone took 20 s; the stage rendered 55 s at ~9 fps; Wwise is <1 s of
  it and off the render loop). A d3d9-only rerun under the same load did not even reach `Initializing Engine` before 43 s. See
  **rerun with the machine idle (0 cl.exe, `build/agentIH1/gate_d3d9.txt`): 9 ok, 0 failed - d3d9_frames 1860,
  startup 4.1 s (3.4 s at HEAD), criticals 0, unported_natives 0.** So all 25 gate metrics pass.

## 3. Where the brief was wrong (with the measurement)

1. **Base**: the worktree is `5ddc8a1`, not `33d1c80` (one PHASE23.md line). Main moved to bda2769 (IA), 63c3c91 (IB) and e1926a1 (IF)
   during the work; none touches a file of this package (sync base = main 184c234).
2. **CoreSmoke** lives in `source/Tests/CoreSmoke`, not `source/Development/Tools/CoreSmoke`, and already links the wwise
   library through `dishonored_apply_defines`.
3. **Sizes**: AkMemoryMgr is 34 functions (21 + 13 TLSF in `soundengine\3rdparty`), not 21; AkStreamMgr is 185, not 33;
   CAkBankMgr is 82 (67 in akbankmgr.cpp + 15 template readers in akbankmgr.h), not 60; Query 30 (31 with the overload).
4. **CAkFilePackage\*** is not Wwise runtime code: it is Audiokinetic's SDK *sample* compiled into Arkane's AkAudio
   (`akunrealiohookblocking.cpp`, ported by agent AN). Its deviations (linear search ignoring language, last-'.' base name,
   FArchive reads) change nothing measurable: every package holds exactly one bank, and the bank manager only ever asks for
   `<name>.bnk` (one dot; GetBankIDFromString strips at the first '.' as retail). The "file-package lookup of a known media id"
   check cannot run in CoreSmoke (AkAudio needs Engine), and no media id is looked up before IH2 (0 CreateStd/CreateAuto by id
   in IH1a.log); the python tool's `--find-media` covers the table.
5. **CAkDeviceDeferredLinedUp / CAkTransferDeferred** are never created: the glue passes AK_SCHEDULER_BLOCKING (2013 0x5b1430),
   so 38 deferred-scheduler functions are out of scope.
6. **"No behaviour change"** cannot hold for Query: real QueryAudioObjectIDs / GetPositioningInfo give UAkEvent::ComputeMaxRadius
   real radii (IH1a.log: 1 of the 7 events queried gets 4000 instead of -1), so UAkAudioDevice::PostEvent then routes a
   cookie-less actor post through DisAkEndOfEventCallback and records m_PlayingRadii, as retail does. The gate's newgame metrics
   are unchanged (pawns 29, on_mission 105, criticals 0).
7. **HIRC loading needs IH3's managers**: STMG, ENVS and every object's RTPC / state chunk register with CAkStateMgr / CAkRTPCMgr /
   CAkEnvironmentsMgr during the load; IH1 ports their registration entry points as data stores (`AkGameSyncs.*`).
8. **Music "opaque"** needs nothing stored: retail routes types 10-13 to MusicEngine's handler; until IH4 the handler declines and
   the bank manager skips the bytes exactly as retail does without a handler (2,461 music objects in the .pck set).
9. Retail PostEvent fails (AK_INVALID_PLAYING_ID) for an event absent from the index; measured 0 such posts in the opening, so
   IH2 can switch over without a Kismet risk on this route.

## 4. Re-sizing IH2-IH4 (from the inventory, functions / bytes of retail code)

- **IH2 (a line is audible): 1,044 functions, 174 KB** - akaudiolib.cpp playback half 111, CAkPBI 56, CAkAudioMgr 50, renderer 36,
  lower engine pipeline (~120 over aklengine/akvpl*), playing mgr 21, Vorbis decoder 74, file/bank sources ~40, sink 17, plus
  the hash/list template instantiations they use (already in AkPrivateTypes.h). Also owed to IH2 from IH1: media preparation
  (9 functions), KillSlot's audio-thread message.
- **IH3 (parameter graph): 699 functions, 86 KB** - the *playback* half of the classes IH1 loaded (node base 66, bus 47, node 38,
  RTPC mgr 38, state mgr 23, containers ~70, dynamic sequence 21, paths 17, feedback ~40 stubbed). Smaller than PHASE23 assumed
  because every class's data and loading is done.
- **IH4 (music + effects): 730 functions, 167 KB** - akmusicengine 343 (switch ctx, sequencer, renderer, segment, track) and the
  effect plug-ins 387 (room verb, FDN reverb, harmonizer, pitch shifter, delay, time stretch, tremolo, parametric EQ, silence
  source). Larger than "~300" for the plug-ins.
- IH1 itself: 471 functions / 68 KB ported.

## 5. Files

New: `Src/AkMemoryMgr/*`, `Src/StreamManager/*`, `Src/AkAudioLib/Common/*` (13 files), `source/Tests/CoreSmoke/WwiseSmoke.cpp`,
`resources/tools/wwise_bank_dump.py`, `resources/docs/wwise/ih1_inventory.csv`, `resources/docs/wwise/wwise_program.md`.
Changed: `Src/AkSilentSoundEngine.cpp`, `AkSilentMusicEngine.cpp`, `AkSilentCommon.cpp`, `include/DishonoredWwiseSilent.h`,
`AkAudio/Src/akaudiodevice.cpp` (one line), `cmake/Wwise.cmake`, CoreSmoke `CoreSmoke.cpp` / `CMakeLists.txt`.
Deleted: `Src/AkSilentMemoryMgr.cpp`, `Src/AkSilentStreamMgr.cpp`. Sync: `build/agentIH1_sync.py`.
Merge note: `AkSilentSoundEngine.cpp` overlaps ID (SetDurationHint); IH1 rewrote the bank / settings / Query regions and the
Init/Term bodies, ID's hunks belong in PostEventCommon / Voice / RenderAudio, which IH1 only extended by the -akdiag check.
