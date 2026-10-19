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
