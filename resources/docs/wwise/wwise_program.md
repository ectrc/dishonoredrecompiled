# The Wwise 2012.1 runtime program (IH1..IH4): conventions

Written by agent IH1 (wave 26) for the packages after it. The inventory of every runtime function, its 2013
twin, its owning package and IH1's status is `resources/docs/wwise/ih1_inventory.csv` (regenerate with
`python build/agentIH1/mk_inventory.py <tree>`: a function counts as ported when its 2012 rva appears in a comment
of a ported source file, or its `Class::Method` is defined there).

## Where things live

```
source/Development/Src/External/Wwise2012/
  include/                     our 2012.1 public headers (unchanged by IH1 except DishonoredWwiseSilent.h)
  Src/AkMemoryMgr/             AK::MemoryMgr + TLSF (Conte 2.x, the 3rdparty unit of the PDB)
  Src/StreamManager/           AK::StreamMgr: CAkStreamMgr, CAkDeviceBase/Blocking, CAkIOThread, CAkIOMemMgr
  Src/AkAudioLib/Common/       one folder per SDK folder (wwise\sdk\source\soundengine\akaudiolib\common)
    AkPrivateTypes.h           CAkLock, AkArray, AkListBare(Light), AkHashList(Bare), CAkKeyArray, CAkList2,
                               AkPropBundle, CAkConversionTable: the SDK's header templates, PDB layouts
    AkAudioLibIndex.h          CAkIndexable, CAkIndexItem<T> (800 bytes), CAkAudioLibIndex (0x1F40)
    AkAudioLibGlobals.h        g_settings, g_PDSettings, g_csMain, g_p* managers, AkBank chunk formats, AKDIAG
    AkHierarchy.h/.cpp         the HIRC classes (data + loading half)
    AkGameSyncs.h/.cpp         registration side of CAkStateMgr / CAkRTPCMgr / CAkEnvironmentsMgr (IH3 replaces)
    AkBankMgr.h/.cpp, AkBankReader.cpp, AkRegistryMgr.h/.cpp, AkAudioLib.cpp
  Src/AkSilent*.cpp            what is left of agent AN's silent backend (playback side, music stub)
cmake/Wwise.cmake              GLOB_RECURSE over Src/, each module folder a private include root
```

## Rules the IH1 code follows (keep them)

1. **One retail function, one port function, rva in its doc comment** (`/** 2012 0x92c320. */`). The
   inventory script counts on it. A function that changed shape names the deviation in the file banner.
2. **Allocations go through AK::MemoryMgr** with the pool retail uses (`g_DefaultPoolId` for objects, the
   slot's pool for bank data, `AkMediaEntry::m_uPrepareEventMemoryPoolID` for prepared media). Objects are
   placement-new'd into `AK::MemoryMgr::Malloc` and released with an explicit dtor call + `Free`, never `new`.
3. **Containers are the SDK's** (AkPrivateTypes.h): their layouts are the PDB's, `TGrowBy 0` arrays never grow
   (the usage slot's loaded-item list is sized by the HIRC count first, as retail).
4. **Locks are retail's**: `g_csMain` around every object's SetInitialValues and every Query; each index item
   has its own lock and AddRef/Release of an indexed object happens under it; `CAkBankList::m_BankListLock`
   around slot lookups; the bank queue under `m_queueLock`.
5. **Threads are retail's**: `AK::IOThread` (one per device, CAkIOThread::IOSchedThread) and `AK::BankManager`
   (CAkBankMgr::BankThreadFunc), created with `AKPLATFORM::AkCreateThread` and the AkThreadProperties retail
   passes (`g_PDSettings.threadBankManager`, `AkDeviceSettings::threadProperties`). Synchronous API calls queue
   an item and wait on `DefaultBankCallbackFunc`'s event, exactly as `AkSyncCaller`. IH2 adds the audio thread
   (`CAkAudioMgr` / `CAkLEngine`); do not run playback work on the bank thread.
6. **-akdiag** (`DishonoredWwise::IsDiagnostics()`, set by AkAudio from the command line): use
   `AKDIAG( L"akdiag: ..." )`. It is thread-safe (the log hook is debugf, which buffers off the game thread).
   IH1 logs every bank operation (name, id, result, bytes, HIRC objects by type, failed, misparsed), every
   GetIDFromString, every Query::QueryAudioObjectIDs / GetPositioningInfo answer, every stream-manager file
   open, and every PostEvent of an event no loaded bank declares.
7. **The parse-end check stays on**: every top-level SetInitialValues stores where it stopped in
   `g_pHircParseEnd`; CAkBankMgr::CheckParseEnd counts objects that did not read exactly their HIRC section.
   IH3/IH4 must set it in every reader they add (the music readers included) - CoreSmoke's sweep fails otherwise.
8. **CoreSmoke** (`source/Tests/CoreSmoke/WwiseSmoke.cpp`) loads Init.bnk through the stream manager and every
   packaged bank from memory, compares the HIRC counts by type with `wwise_bank_dump.py --csv` and requires 0
   failed / 0 misparsed objects and a clean unload. Run it with
   `WWISE_SMOKE_CSV=<csv> <build>/Binaries/Win32/CoreSmoke.exe` from the tree root; `--all` CSV = 981 banks.

## What IH1 leaves for the next packages (the seams)

- **IH2** replaces `AkSilentSoundEngine.cpp`'s PostEvent / RenderAudio / voices with CAkAudioMgr + the renderer.
  The bank side it will need is in place: `CAkBankMgr::GetMedia` (in-memory media of loaded banks),
  `CAkSound::m_Source` (source id, file id, offset, size, codec, prefetch bits as `LoadSource` decoded them),
  `CAkEvent::m_actions` and `CAkActionPlay::m_fileID`. Not ported (IH2): media preparation for
  PrepareBank(AllInclusive) / PrepareEvent (`PrepareMedia`, `LoadSingleMedia`, `PrepareNodeData`), `KillSlot`'s
  audio-thread message 26 (IH1 releases the slot directly), `CAkFxBase::SetFX`'s plug-in parameter object.
  Retail PostEvent returns `AK_INVALID_PLAYING_ID` for an event absent from the index: `-akdiag` lists them
  (`akdiag: PostEvent ... not in any loaded bank`), so the switch-over can be measured first.
- **IH3** replaces AkGameSyncs.* with the real CAkStateMgr / CAkRTPCMgr / CAkEnvironmentsMgr (the call sites in
  AkHierarchy.cpp and AkBankMgr.cpp use retail's signatures), and adds the playback half of the node classes.
  The class data members are the PDB's; add virtuals to `CAkParameterNodeBase` rather than new classes.
- **IH4** installs `AkMusicBank::LoadBankItem` as `g_pExternalBankHandlerCallback` (today AkSilentMusicEngine
  declines every music item with AK_PartialSuccess and the bank manager skips it).

## Closing (agent IH4, end of the IH program)

**Ported.** Of the 3,070 Wwise rows of `ih1_inventory.csv` (the ~3,100 of the plan), the last owner's status counts
**2,812 ported** (rva cited in a port file, ported by name, or a body in an earlier package's file), plus 62 SDK header
templates instantiated by their users (`AkPrivateTypes.h`) and 2 static initialisers: **2,876 of 3,070 (93.7 %)**.
Per package: IH1 memory + stream + bank manager, IH2 the audio thread, renderer, lower engine, Vorbis, sink (989 of 1,170
rows), IH3 the parameter graph, positioning, containers (542 of 620), IH4 the music engine, nine effects + the silence
source, insert / bus effects, environmental sends (811 of 829).

**Named, not ported (194 rows), each for a measured reason:**

| rows | what | why |
|---|---|---|
| 69 | feedback (motion) device, feedback bus / node, `CAkSplitterBus` | no feedback device is ever created, no feedback object in any bank |
| 38 | deferred lined-up stream device | the blocking scheduler is the one the glue creates |
| 26 | IH1-scope leftovers (agentIH1.md) | not on the game's path |
| 20 | playback functions off Dishonored's path (agentIH2.md) | |
| 14 | dialogue events / decision tree | 0 of 42,548 objects |
| 8 | unreached or folded symbols (agentIH3.md) | |
| 7 | default-output-device-change reset | named deviation |
| 7 | mislabelled handovers (IH3's `DynamicSequence::*`, a folded engine symbol) | ported elsewhere / not Wwise |
| 4 | authoring tool / profiler link | no profiler |
| 1 | `CAkListener::ComputeConeEffect` (handed between IH2/IH3/IH4) | the cone path IH3 ported does not reach it |

**What the A/B still shows** (IH4 build 2, `build/agentIH4/ab/IH4b..e_ab.md`, world 57-150 against retail.mp4's audio):
Pearson r of the per-second RMS **0.59-0.62** (IH2 0.20, IH3 0.51), ours a median **4.8-5.1 dB** under retail (IH3 7.2).
What remains: the second at w60 before our first ambience / music posts (60.4 / 62.1), retail's sub-150 Hz drone in
w105-118 (10-13 dB), and a constant ~4 dB that is the recording level of retail.mp4 (an OBS capture at the user's unknown
sliders). The `-startmap` load Critical the music manager exposed is closed by IH4b (retail's pawn termination chain,
agentIH4.md section 6; build 5: r 0.659, -4.8 dB). Open: 3 of 16 cutscene exits end silently right after the exit GC
unprepares `WwiseDefaultBank_WwiseGlobal.bnk`, while continuous ambience containers still post their next item.

**IQ (wave 26, after IH4):** the water / landing / lock sounds were posted but silent: `UAkBank::Load` / `Unload` prepared every
bank with `GenerateDefinition` structure-only, where retail (2013 0xc7210 / 0xc7680) prepares only the `AutoGenerated` `Bk_*`
dialogue banks; 33 `Bank_*` banks of the opening (16.7 MB) had no media for their in-memory sounds (waterfalls, the boat
engine, the lift impact, the lock gates, the dock water laps). Fixed with retail's `UEngine::PreExit` StopAllSounds
(2013 0x1d8850); 19 of 19 exits clean; ride A/B r 0.76-0.82, median -3.5 .. -3.8 dB (IH4 0.66 / -4.8). New diagnostics:
`-akdiag` "voice out" lines (per-voice source and output RMS), `-akdump=<file>`. `resources/docs/agents/agentIQ.md`.
