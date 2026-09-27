# Agent CF report — the save system: the retail container, proved against 51 real save files (2026-09-27)

Package "**CF**" of wave 7 (`PHASE9.md`), PLAN.md milestone 6. Status rows: `agentCF_status.csv` (73 rows).
Nothing committed, nothing staged. Own build dir `build/agentCF`, own snapshot worktree `build/agentCF_wt`, own
IDA copies `resources/docs/idb/{shipping2012,retail2013}_agentCF.i64`, headless decompiles only (96 functions
into `build/agentCF/dec2012/`, 12 into `build/agentCF/dec2013/`), no IDA and no FModel MCP tools, no junctions,
nothing deleted under `Dishonored_Latest2026`.

## The answer in five lines

* **There are 51 real retail save files on this machine**, in Steam's cloud folder
  (`C:\Program Files (x86)\Steam\userdata\1919909936\205100\remote`, app 205100 = Dishonored): the nine mission
  checkpoints, two autosaves, a quicksave and 39 user saves, from August 2025 to March 2026. They are the
  evidence this package is measured against. Copies (with mtimes preserved) live in `build/agentCF/savetest/`;
  the originals were only read.
* **All 51 load in our build through the ported `DisSaveLoad::FGameState::Load`, with zero bytes unconsumed**,
  and **all 51 round-trip through our `FGameState::Save` and back byte-for-byte**, including every compressed
  per-level object blob. One line of our own log says it:
  `DisSaveTest census: 52 files, 51 read, 0 rejected, 0 failed, 146 level states, round trip 51 ok / 0 failed`
  (the 52nd is `OPTIONS.sav`, which retail's own slot scan also rejects as not a game state).
* **The menu's load list populates from those real files.** The engine's list is 51 rows sorted newest first,
  42 of them user-visible, and **all 51 resolve an `FMapConfig`**, so the chapter name, the date and the
  thumbnail path the AS2 list wants all have real inputs:
  `DisSaveTest list census: 51 rows, 42 shown in the load list, 51 resolved a map config, HasSaveGame(0) 1, next auto slot 13, next user slot 26`
* **Agent BE's blocker 1 is cleared.** `FDisSaveGame`, the engine's save list and the four `UDishonoredEngine`
  declarations exist; the load list, `Continue`, `Req_CanLoadGame`, `OnDeleteSaveConfirm` and `PostStart`'s
  `bHasSaveGame` are all wired to the real thing.
* **The player is not placed yet, and that is one named dependency, not a gap in this work.** A retail save
  loads and yields the right map and the right area — `-disloadslot=16` prints
  `DisLoadGame: slot 16 loaded '0 - Dunwall Tower' (mission -1, 1 level states, 1068 names); root level DishonoredGameFull_P`
  / `the player was in l_tower_p (flags 7)`. Restoring the objects inside that level needs
  `UObject::GameSave`/`GameLoad`/`IsSaveable`/`IsRefSaveable`/`PostGameLoad`, which this tree declares on
  `UObject` **not at all**, and which retail overrides **105 and 108 times**. Section 5 costs it.

`python resources/tools/run_regression.py --build-dir build/agentCF --no-build` is **31 ok, 0 failed** (22 in
the run stages plus the 9 of `coresmoke` and `layout`, which needed their own targets built).

## 1. The one thing the coordinator must not miss

`GBuiltFromChangeList` was **334700** in `Core/Src/UnObjVer.cpp`, which is **not either build's value**. The
retail 2013 exe carries **1274963** at rva `0xe6a6e4` (`?GBuiltFromChangeList@@3HA`); 2012 Shipping carries
254295 at `0xe2a6e8`. `DisSaveLoad::FGameState::Load` (2013 rva `0x614020`) refuses any save whose stored
changelist is **greater than ours**, and every retail `.sav` stores 1274963 — so with 334700 in place **every
retail save file was rejected, by construction**. The constant is corrected to 1274963 with the evidence in the
comment. Nothing else reads it but `ADishonoredGameInfo::GetChangelist` and two log lines, so the change is
safe; but it is in Core, it is load-bearing for milestone 6, and it is one line.

## 2. The file format, term for term

Derived from `DisSaveLoad::FGameState::Save` (2013 rva `0x612340`) and `::Load` (`0x614020`), with the 2012
decompiles beside them for the PDB names, and then checked byte-exactly against all 51 files:

```
INT          SaveLoadVersion   == GBuiltFromChangeList; must be >= 258971 and <= ours
INT          SaveVersion       == 24; present only when SaveLoadVersion > 258971; must be >= 15 and <= 24
FString      SaveDetails       "<chapter> - <map friendly name>", e.g. "1 - Coldridge Prison"
INT          DLCMask           UArkDLCManagementBridge's installed-content bits (0x7f on this machine)
BYTE         bHasObjectMarkers
TArray<BYTE> CompressedStringDictionary
TArray<BYTE> CompressedGameState
per level state, in m_LevelStates order:
  TArray<BYTE> CompressedObjectDictionary
  TArray<BYTE> CompressedObjectData
```

Each `TArray<BYTE>` is a **sequence of 64 KB zlib blocks**, each one a whole
`FArchive::SerializeCompressed` unit (`PACKAGE_FILE_TAG`, `GSavingCompressionChunkSize`, the summary, the chunk
infos, the data). The 64 KB comes from the block-size argument Arkane added to
`FArchiveSaveCompressedProxy` / `FArchiveLoadCompressedProxy`; this tree's reference proxies hard-coded
`LOADING_COMPRESSION_CHUNK_SIZE` (128 KB), so the argument is added with that as its default — four lines in
Core, no other caller affected.

The **string dictionary** is `INT NumNames` then, for indices 1..N-1, each name's ANSI characters *and its
terminator*; index 0 is always `NAME_None`, interned by the constructor. Names come back through `FNAME_Add`.
An `FName` in the stream is `WORD DictionaryIndex, WORD FName::Number` — the **raw** instance number, which in
this engine is what `FName::GetNumber()` returns and what `FName(EName, INT)` takes.

The **game state** blob is:

```
BYTE NumSubLevels;    per sub-level: WORD nameIdx, WORD nameNumber, BYTE Flags
BYTE NumLevelStates;  per level state: WORD levelName{idx,num}, WORD packageName{idx,num}, BYTE Location,
                      WORD NumObjects, NumSavedObjects, NumSharedObjects, NumSavedSharedObjects,
                      INT SaveVersion, INT 0, INT 0      (two reserved DWORDs retail writes as literal zeroes)
INT  m_nMissionIndex
```

and `m_RootLevelName` is not in the file at all: `Load` copies it out of **the last level state's package
name** after the loop. Every retail save on this machine has `DishonoredGameFull_P` there, which is the
persistent map our build already opens, and the area the player was in is the one sub-level whose `Flags` are
non-zero (`L_Prison_P(7)`, `l_tower_p(7)`, `L_Ovrsr_P(7)`, ...). That pair — root map plus flagged sub-level —
is the placement information the container carries, and it is right for all 51 files.

### 2012 is not 2013 here, in four places

| | 2012 | 2013 |
|---|---|---|
| header | changelist, then the details FString | changelist, **then a save version**, the FString, **then the DLC mask** |
| the version gate | `changelist != GBuiltFromChangeList` -> reject | `258971 <= changelist <= ours` and `15 <= version <= 24`, with a warning when it is an older build |
| the inner archives' `ArVer` | the changelist | the **save version** (24), so the object layer can branch on it |
| slot numbering | 1..9 missions, 10/11 autosave, 12 quicksave, 13..52 user | **1 OPTIONS, 2/3 the DLC02 chaos markers, 4..12 missions, 13/14 autosave, 15 quicksave, 16..55 user** |

The slot renumbering is why `Dis_Load` clamps to 0..55 rather than 2012's 0..52, and it is settled by three
independent readings: the 2013 name builder (rva `0x5fb660`) switches on 1/2/3 by name and formats
`Dishonored%i` from `Slot - 16`; `GetNextAutoSaveSlot` (`0x60b050`) writes literal 13 and 14;
`GetNextUserSaveSlot` (`0x5fb930`) keys a 40-entry bitmap on `Slot - 16`.

## 3. What is in the tree

| file | what |
|---|---|
| `DishonoredGame/Inc/dishonoredutilities_saveload.h` | the whole save layer's declarations, where retail declares them (the PDB attributes every `_DishonoredGetScriptStruct<>` instantiation of the module to this header, and there is no `dissavegame.h` in the retail source list). `ESaveLoadLocation`, the slot constants, `FDisSaveGame`, `FAsyncWorkBase`, the async lister and deleter, and `namespace DisSaveLoad` with `FDeletedActor`, `FSubLevelState`, `FLevelState`, `FGameStateData`, `FStringDictionary`, `FGameState`. Every layout is the 2012 Shipping PDB through `dia_types.py` (`build/agentCF/types_saveload.txt`). |
| `DishonoredGame/Src/dissavegame.cpp` | was a comment-only skeleton. `FStringDictionary` (all seven), `FGameState`'s container `Save`/`Load`/`LoadMapName` and the level-state bookkeeping, `IsSubLevelUnshared`, and the three level lookups. |
| `DishonoredGame/Src/dishonoredengine.cpp` | appended: `GDisSaveGameSlotNames`, `GetSaveGameSlot`, the save-path helpers, `FAsyncWorkBase`, `FDisAsyncSaveGameLister`, `FDisAsyncSaveGameDeleter`, the eleven `UDishonoredEngine` list methods, `LoadGame`, a real `Dis_Load`, and `DisSaveGameSelfTest`. |
| `DishonoredGame/Inc/CppText/UDishonoredEngine.h` | eleven declarations. |
| `DishonoredGame/Src/disgfxmovieplayermenubase.cpp` | the load list built from the real save list, `FindSaveName` / `FormatSaveDate` / `FindSaveImagePath`, `Req_CanLoadGame`, `OnDeleteSaveConfirm`. |
| `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | `OnContinueClicked` and `PostStart`'s `bHasSaveGame`. |
| `Core/Inc/UnArchive.h`, `Core/Src/UnArchive.cpp` | the compressed proxies' Arkane block-size argument, defaulted. |
| `Core/Src/UnObjVer.cpp` | `GBuiltFromChangeList` 334700 -> 1274963 (section 1). |
| `DishonoredGame/Sources.cmake` | one line: `Src/dissavegame.cpp` leaves the exclude list. Not generated by `gen_classes_header.py --sdk`, per agent BE's check. |

No natives are ported, so `DishonoredGameNativeStubs.ported.agentCF.txt` does not exist and no module needs
regenerating for this package. The three menu leaves are file-static functions rather than the protected
methods retail has, because `UDisGFxMoviePlayerMenuBase` has no `Inc/CppText` header and adding one would mean
editing a generated header; this is agent BE's precedent for the same class.

## 4. Verification

Run it with:

```
python resources/tools/build_and_smoke.py --build-dir build/agentCF --no-build --timeout 120 \
  --extra-args="-forcelogflush -savetest -savetestwrite -savedir=D:\RecompileDishonored\Recompile\build\agentCF\savetest\\"
```

`-savetest` reads every `*.sav` of the directory through the ported container and prints what it found;
`-savetestwrite` writes each one back out through `FGameState::Save` and re-reads it, comparing the save
version, the details string, the mission index, the root level, every sub-level name and flag, every level
state's names, location, four object counts and save version, and both compressed blobs with `appMemcmp`, plus
the whole dictionary index by index. `-savedir=` keeps the writes out of the retail Steam-cloud save folder.
Three lines from `Dishonored_Latest2026/DishonoredGame/Logs/Launch.log`:

```
DisSaveTest:   Dishonored11.sav       slot 27 v24 dlc 0x7f names  2096 sub  8 levels 6 mission  1 root DishonoredGameFull_P left 0 :: 3 - High Overseer's Office
DisSaveTest:       level galvani (package L_Galvani1_P) loc 1 objects 2770/3470 shared 1413/1679 v24 dict 13977 data 44000
DisSaveTest:       placement: open DishonoredGameFull_P and stream L_Ovrsr_Kennel_P (flags 7)
```

`left 0` is the whole point: the file ends exactly where the reader stops, on all 51. The `galvani` line also
shows the level-state table doing something a guess would not have produced — a save carries the *memory*
state (`loc` 0 and 1, `SLL_MEMORY_PARTIAL` / `_COMPLETE`) of every level the player has already visited
alongside the `SLL_FILE` state of the persistent one, which is how Dishonored remembers a hub you walk back
into. `Dishonored11.sav` carries six.

The second half of `-savetest` prints the engine's own list, which is what the menu leaf consumes:

```
DisSaveTest list: row  0 slot 14 shown 2026-07-04 23:30  mapconfig L_Prison_P               :: Coldridge Prison
DisSaveTest list: row 49 slot 16 shown 2025-08-13 20:42  mapconfig L_Tower_P                :: Dunwall Tower
DisSaveTest list: row 50 slot  4 hidden 2025-08-05 18:19  mapconfig L_Prison_P               :: Coldridge Prison
DisSaveTest list census: 51 rows, 42 shown in the load list, 51 resolved a map config, HasSaveGame(0) 1, next auto slot 13, next user slot 26
```

A single save through the real engine entry point:

```
python resources/tools/build_and_smoke.py --build-dir build/agentCF --no-build --timeout 120 \
  --extra-args="-forcelogflush -disloadslot=16 -savedir=...\savetest\\"
->  DisLoadGame: slot 16 loaded '0 - Dunwall Tower' (mission -1, 1 level states, 1068 names); root level DishonoredGameFull_P
    DisLoadGame:   the player was in l_tower_p (flags 7)
```

The full log of the run quoted above is kept at `build/agentCF/savetest_final.log`; the offline Python model
the format was first derived and checked with is `build/agentCF/parse_sav.py` (it prints one line per file and
ends `51/51 parsed`), and the PDB layout dump is `build/agentCF/types_saveload.txt`.

**Regression: 31 ok, 0 failed.** `coresmoke` 2/2, `layout` 7/7, `nullrhi` 4/4, `d3d9` 8/8, `inputtest` 9/9
(`build/agentCF/regression/summary.txt`). `layout_types` 2314, `layout_mismatching` 0, `layout_contract` 0 —
nothing this package adds is a reflected type, so no layout moved.

**Built from a snapshot, not the shared tree.** The shared tree does not compile right now: package CE's Arkane
post-process classes are half-moved into `Engine/Inc/enginearkppclasses.h` while
`DishonoredGame/Inc/DishonoredGameEngineShims.h` still declares `EPpNodeAAType`, `EPpNodeBlurType`,
`EPpNodeCommonTarget` and `EPpNodeRenderStage`, so every DishonoredGame unit fails with `C2011`. That is CE's
file and CE's to finish; this package built `build/agentCF_wt` (HEAD `8e61755` plus its own ten files) instead.

## 5. What is left, and what it costs

The object layer. `FGameState::Save`/`Load` carry each level's two blobs verbatim, which is why the round trip
is byte-exact, but nothing here *reads inside* them. Decoding them means porting:

* `DisSaveLoad::FLevelSaver` and `FLevelLoader` (2013 rvas `0x615730` / `0x613070`, 1742 and 3337 bytes) and
  their four `operator<<` overloads, `GetObjectIndex`, `ShouldSaveObject` / `ShouldLoadObject`,
  `SerializeLevel`, `NotifyPostGameLoad` — about 12 functions, all decompiled into `build/agentCF/dec2012/`.
* `FGameState::SaveLevel` / `LoadLevel` / `SaveGameState` / `ContainsObjectState` and
  `DisSaveLoad::InitializeSequencePostLoad`.
* `UDishonoredEngine::ProcessSaveLoadCmd` (`0x6162d0`) and `DoSaveGame` (`0x6095a0`) — the state machine whose
  `SLC_PostLoad` does the map travel and the per-level restore.
* **and the part that is a wave of its own: `UObject::GameSave`, `GameLoad`, `IsSaveable`, `IsRefSaveable` and
  `PostGameLoad`.** This tree declares none of them on `UObject`; retail has **105 `GameSave` and 108
  `GameLoad` overrides**, plus 11 `IsSaveable`. `AActor::GameSave`/`GameLoad` (2013 `0x1750e0` / `0x18ad70`)
  are the base pair. It cannot be done partly: `FLevelSaver::operator<<(UObject*&)` writes a `WORD` index and
  then the object's own `GameSave` **inline**, with no length prefix, so one missing override desynchronises
  every object after it in the stream. Half the overrides buys nothing.

The object **dictionary** format is decoded and worth writing down, because it is the half that carries
transforms and it is not obvious (`GetObjectIndex`, 2013 rva `0x611b80`). Per new object: a `WORD` index, then

* if that index names an already-registered `DisTweaks` object -> `FVector Location`, `FRotator Rotation`
  (a spawn-on-load actor with tweaks);
* else the index is a class, and: `ULevel` -> `WORD LevelNameIdx`; an Actor class ->
  `BYTE bSpawnActorOnSaveGameLoad`, then either the two transforms or `WORD Outer, WORD NameIdx, WORD NameNum`;
  anything else (including index 0, which is how a `UClass` is written) -> `WORD Outer, WORD NameIdx, WORD NameNum`.

Indices 0, 1 and 2 are seeded by the constructor with `NULL`, `UPackage`'s class and the `Core` package, so the
stream starts at 3. It is decodable only with the class hierarchy in hand, which is why the loader resolves each
record against the objects it has already built. `build/agentCF/parse_objdict.py` is a partial Python decoder
left for whoever takes this on; it is **not** part of the acceptance and its entry counts do not yet match
`FLevelState::m_NumObjects`.

## 6. Deviations, stated plainly

1. **`DisGetInstalledContentMask()` returns `0x0f`.** Retail computes it from
   `UGameEngine::DLCManagementBridge` (2013 rva `0xb7aa0`, one bit per content entry whose state byte is 2) and
   gets `0x7f` on this machine; `UArkDLCManagementBridge` is not declared in this tree. `Load` gates on
   `(saved & ~current & 0x0f) != 0`, so answering `0x0f` means **no save is ever refused for missing content**,
   which is the behaviour that lets a save made with the campaign packs load in a build that has no DLC
   manager. The cost is that **our writer writes `0x0f` where retail writes `0x7f`** — the one header field
   where our output is not byte-identical to retail's. Retail would still accept it
   (`0x0f & ~0x7f & 0x0f == 0`).
2. **The lister timestamps from the file's `st_mtime`, not Steam.** 2013's lister asks
   `ISteamRemoteStorage::GetFileTimestamp` (vtable +36) on the bare file name; with `-nosteam` there is no
   cloud, and for a synced file the two agree.
3. **The lister walks the directory with `GFileManager->FindFiles`**, not `FindFirstFileW`. Same base names.
4. **The campaign file-name prefix is the base game's empty one.** 2013 prepends `DLC02_` or `DLC03_` after two
   build-configuration queries (`0xbbf110` / `0xbbf1b0`); this executable is the base campaign.
5. **`Dis_Load` calls `LoadGame` directly** instead of setting `SLC_Load` and waiting for
   `ProcessSaveLoadCmd`. Same body, one tick earlier.
6. **The chaos-marker mode of `FGameState::Load`** (its third argument, which reads a DLC02 low/high marker
   instead of a game state) is not ported; nothing in the base campaign asks for it. Slots 2 and 3 return a
   failure from `LoadGame` and say so.
7. **`OnContinueClicked` loads save-list row 0.** Retail's body is behind a vtable slot (main menu +604) that
   has no PDB name in either build; the list is sorted newest first by `m_Time` (2013 `0x6086f0`), which is
   what that sort exists for. Flagged here and at the site as inferred.
8. **`FAsyncWorkBase` is declared in `dishonoredutilities_saveload.h`**, not in `Core/Inc/UnAsyncWork.h` where
   retail has it, because that header is still the reference one. Fold it back when Core is reconciled.
9. **Without a thread pool the lister and the deleter run inline.** `RefreshSaveGameList` falls back to
   `DoThreadedWork()` when `GThreadPool` is NULL; `Wait()` then returns immediately because `Dispose()` has
   already triggered the event.
10. **`m_pMenuBaseTweaks` is read but not yet exercised.** The three menu leaves take the autosave and
    quicksave prefixes, the date format and the image package off that per-subclass tweaks pointer, which is
    exactly the trap that has caught two agents: read through a base pointer it yields an all-zero class
    default. Each read is NULL-guarded and `DisFillLoadGameMenu` logs the pointer's path name and all four
    strings the first time it runs — but **the leaf has not run**, because the load list needs a live movie and
    the GFx renderer is packages CB/CC/CD's. The instrumentation is in place for whoever gets there first.

## 7. Hand-overs

1. **Coordinator, at merge** — section 1 (`GBuiltFromChangeList`) is the one change outside this package's own
   area that has to survive, and `Core/Inc/UnArchive.h` + `Core/Src/UnArchive.cpp` (the defaulted block-size
   argument) is the other. `DishonoredGame/Sources.cmake` loses one exclude line and is not generated. No
   module needs regenerating for this package: it adds no native and no reflected type.
2. **Whoever takes the object layer (the obvious wave-8 package)** — section 5 is the costing, the 12 level
   saver/loader functions are already decompiled in `build/agentCF/dec2012/`, the object dictionary's record
   format is written down, and `FGameState` already holds each level's two blobs after a load, so the work
   starts at `FLevelLoader`'s constructor with real data in hand. `UObject`'s five virtuals are the gate and
   they are an engine-wide declaration, i.e. a coordinator decision before the package starts.
3. **Packages CB / CC / CD** — when a movie runs, `DisFillLoadGameMenu`'s census line is the first thing worth
   reading: it says how many rows went in and what the tweaks pointer actually held.
4. **Package CE** — the shared tree's `C2011` on the four `EPpNode*` enums (section 4) is yours; it is the
   reason this package built a snapshot.
5. **Whoever wants the retail saves** — they are Steam cloud files, not in
   `Documents\My Games\Dishonored\DishonoredGame\SaveData` (which holds only `Puid.txt` because retail has not
   run on this machine since the sync). `build/agentCF/savetest/` is a copy with mtimes preserved;
   `-savedir=<path>` points the whole save layer at it, which is how the round-trip test avoids writing
   anything into the real save folder.
