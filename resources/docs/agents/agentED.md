# Agent ED report — the five `UObject` save virtuals and the object layer of the save game (2026-09-29)

Package **ED** of wave 10 (`PHASE11.md`), PLAN.md milestone 7. Status rows: `agentED_status.csv`.
Nothing committed, nothing staged. Own build dirs `build/agentED` (Debug) and `build/agentED_release`, own
snapshot worktree `build/agentED_wt` (detached at `c598a21`), own IDA copies
`resources/docs/idb/{shipping2012,retail2013}_agentED.i64`, headless decompiles only (266 functions into
`build/agentED/dec2012/`, 17 into `build/agentED/dec2013/`), no IDA and no FModel MCP tools, no junctions,
nothing deleted under `Dishonored_Latest2026`.

## The answer in six lines

* **`UObject`'s five save virtuals are declared and their semantics are proved, not guessed.** They are
  vtable slots 66..70 (`IsRefSaveable`, `IsSaveable`, `GameSave`, `GameLoad`, `PostGameLoad`), in that order,
  from the PDB type dump and confirmed slot by slot against three derived classes' vtables. `IsRefSaveable`
  returns TRUE, **`IsSaveable` returns FALSE**, and the other three are empty.
* **The retail override census in every brief so far was incomplete, by a factor of six.** The PDB names 11
  `IsSaveable` overrides. The real number is **70**: 59 classes declare it inline as `return TRUE` and
  retail's linker folds all 59 onto one 8-byte body (the same one `UObject::IsRefSaveable` uses), so they
  have no symbol of their own. `resources/docs/symbols/vtables.csv` slot 67 is the only complete record, and
  it is what decides whether an object is in the stream at all.
* **The object dictionary decodes exactly, in this tree's own C++, on real retail saves.** Three saves,
  each through the real engine entry point: `dictionary 11321/11324 objects 91844/91844 bytes`,
  `8305/8308 objects 67665/67665 bytes`, `8215/8218 objects 67164/67164 bytes`. The record count matches
  each level state's own `m_NumObjects` (the +3 are the constructor's seeds) and every byte of the blob is
  consumed with none left. Agent CF's open question — its counts not matching `m_NumObjects` — is closed.
  The offline model does the same for **all 146 level states of all 51 real saves, 0 failures**.
* **The object *data* stream runs, and stops safely.** It reaches `AWorldInfo`, restores it, and halts at
  `DishonoredMapInfo` — the first of retail's 112 `GameSave`/`GameLoad` override classes that this tree does
  not have — naming the class and the object, after 26 of 619,631 bytes. It cannot desynchronise silently:
  **three** gates stop it: an unported override, a dictionary index that resolved to no live object, and
  an object retail would have saved whose `IsSaveable` this tree does not declare. Section 4 has all three.
* **14 of retail's 112 override classes are ported** (`AActor`, `AWorldInfo`, `ATrigger`, `ATargetPoint`,
  `AInterpActor`, `AEmitter`, `ASceneCaptureActor`, `ALight`, `AKActor`, `USequenceObject`, `USequenceOp`,
  `USequence`, `USequenceEvent`, `USeqAct_Latent`), all in Engine. Section 6 lists the other 98 and, more
  usefully, the 33 that a real save's persistent level actually reaches and the order it reaches them in.
* **Acceptance 1 is NOT met: a real save does not yet put the player back where it says.** The measurement
  that matters instead is section 5's: the player pawn is dictionary record **162**, and exactly **11**
  unported override classes sit between the start of the stream and it — 3,723 bytes of x86 between here and
  the milestone. That number is measured from the save files, not estimated.

`python resources/tools/run_regression.py --build-dir build/agentED_release --no-build` is **31 ok, 0
failed**, and `build/agentED_release_build.cmd` (Release, all three targets) builds with 0 errors.

## 1. What the five virtuals actually are

This tree declared none of them. They are now in `Core/Inc/UnObjBas.h`, right after `Get_bDebug` (slot 65),
which is where the PDB's `UObject_vtbl` puts them (`resources/docs/types/all_types.h` type 20177):

| slot | signature | `UObject`'s body | evidence |
|---|---|---|---|
| 66 | `virtual UBOOL IsRefSaveable( ESaveLoadLocation ) const` | `return TRUE` | 2013 rva `0x5ea9d0`, out of line, 8 bytes |
| 67 | `virtual UBOOL IsSaveable( ESaveLoadLocation ) const` | **`return FALSE`** | ICF-folded onto 2012 `0xa6cca0` (`UParticleModuleUber…::ConvertToUberModule`), 5 bytes, `xor eax,eax / ret 4` |
| 68 | `virtual void GameSave( FArchive&, ESaveLoadLocation )` | empty | ICF-folded onto 2012 `0xa26ea0`, 3 bytes, `ret 8` |
| 69 | `virtual void GameLoad( FArchive&, ESaveLoadLocation )` | empty | the same fold |
| 70 | `virtual void PostGameLoad( ESaveLoadLocation )` | empty | ICF-folded onto 2012 `0xc8750`, 3 bytes, `ret 4` — **one argument**, which no brief said |

`ESaveLoadLocation` moved from `DishonoredGame/Inc/dishonoredutilities_saveload.h` (agent CF put it there) to
Core, because `UObject` takes it. `PreGameSave()` is a *different* virtual (`ADishonoredPawn`,
`UDisItemContext_NPCTeleportSpell`) and is not one of the five.

The slot numbers were read off `resources/docs/symbols/vtables.csv`: `AActor` slots 68/69 hold
`AActor::GameSave`/`GameLoad`, `USequenceObject` slot 67 holds `USequenceObject::IsSaveable`, `AKActor` and
`ALight` the same, and `ADisAlarmBell`'s secondary vtable slot 70 holds its `PostGameLoad`. The folds were
then decompiled to read their return values rather than inferred.

## 2. The finding that changes the costing: 59 invisible `IsSaveable` overrides

`UObject::IsSaveable` is FALSE, so **an object is only in the stream when its class says otherwise**. The
PDB lists 11 out-of-line `IsSaveable` overrides, which would mean a save carries almost nothing; a real
save's persistent level carries 9,540 of 11,324. The PDB is not wrong, it is incomplete: 59 more classes
declare `IsSaveable` **inline** as `{ return TRUE; }`, and because that compiles to the same
`mov eax,1 / ret 4` as `UObject::IsRefSaveable`, the linker folds all 60 bodies into the one at 2012
`0x66a860`. An inline function with no unique code has no PDB symbol.

So the complete set of save entry points is the **slot-67 column of `vtables.csv`**, and it is 372 classes
once script-only subclasses are resolved to the native class whose vtable they share. That list is generated
into `DishonoredGame/Inc/dissaveload_classlists.h` by `build/agentED/gen_classlists.py` and the loader logs
its size on every run:

```
DisRestore: retail has 372 save entry classes and 112 GameSave/GameLoad override classes; this tree ports 14 of them
```

Three earlier counts should be read with this in mind: "105 `GameSave`, 108 `GameLoad`, 11 `IsSaveable`" is
the PDB's view. The vtable's view is 112 distinct `GameSave`/`GameLoad` bodies over 372 entry classes.

## 3. The two streams, term for term

Derived from `FLevelSaver::GetObjectIndex` (2013 rva `0x611b80`) on the writing side and the dictionary loop
of `FLevelLoader::FLevelLoader` (`0x613070`) on the reading side, which is the authority because it is what
decides how many bytes each record is. Verified against all 146 level states of all 51 real saves.

**`CompressedObjectDictionary`** — one record per object:

```
WORD idx                                  the dictionary index of this record's class, or of the DisTweaks
                                          object the actor is spawned from, or 0 when the object is a UClass
  idx names a UDisTweaksBase instance  -> FVector Location, FRotator Rotation
  idx == 0                             -> WORD Outer, WORD NameIdx, WORD NameNum
  idx names a UClass:
      child of AActor                  -> BYTE bSpawnActorOnSaveGameLoad
                                             1 -> FVector Location, FRotator Rotation
                                             0 -> WORD Outer, WORD NameIdx, WORD NameNum
      child of ULevel                  -> WORD LevelNameIdx
      otherwise                        -> WORD Outer, WORD NameIdx, WORD NameNum
  anything else                        -> WORD Outer, WORD NameIdx, WORD NameNum
```

Indices 0, 1 and 2 are seeded with `NULL`, `UPackage::StaticClass()` and the `Core` package, so records start
at 3. **The terminator is `idx == 0` followed by `Outer == 0`, four zero bytes.** The saver always writes
two of them - one after the shared pass and one at the end of `FLevelSaver`'s constructor - and the loader
consumes the second only when the level state carries unshared objects, which is what `bReadUnshared`
selects. Agent CF's note said "the two terminating WORD zeroes"; it is two WORDs per terminator and two
terminators, and reading one where the stream has two leaves four bytes behind and loses the last records.

**`CompressedObjectData`** — `WORD NumDeletedActors` and that many `FName` pairs, then, repeatedly, a `WORD`
dictionary index followed by that object's own `GameSave` **inline with no length prefix**, until a zero
index. The top bit of the index means "this reference belongs to an unshared sub-level, its state is in that
level's own stream". This is why the layer is all or nothing.

### One deliberate deviation, and why it is better than the original

Retail decides each dictionary record's shape from the **live object** at the index the record names — is it
a `UDisTweaksBase`, a `UClass`, or something else. It can afford that because in the session that wrote the
save everything the dictionary names is resident. In this build a lot of it is not: of 11,321 records of a
real save's persistent level, **2,259 resolve and 9,027 do not** in a bare `-startmap` run, because the
`Twk_*` packages and the streamed sub-levels of the session that saved are not all loaded.

A literal port therefore desynchronises on its own dictionary: measured, it read **132 of 11,324 records**
before one NULL turned a 26-byte transform record into a 6-byte name record. The record itself says what its
class is, so the port keeps that: `m_RecordClasses` and `m_RecordIsClass` run parallel to `m_Objects` and
hold the class each record *declared*. The shape then comes from the stream, and the decode went from
132/11,324 to **11,321/11,324 with all 91,844 bytes consumed**. That is what the offline model
(`build/agentED/objdict.py`) does too, which is why it decodes all 51 saves.

## 4. The safety net, and what happens at an unported class

`DishonoredGame/Inc/dissaveload_classlists.h` is generated from the vtable export and holds the 14 ported and
98 unported override classes. `FLevelLoader::operator<<(UObject*&)` walks each object's class chain against
it — the *first* class with a body of its own is the body that runs, so that is the one that has to be
ported — with the answer cached per `UClass` (a mission save asks twelve thousand times). Three outcomes:

1. no override anywhere in the chain → `UObject`'s empty body, zero bytes either way, safe;
2. the first override is ported → run it;
3. the first override is **not** ported → stop the restore, set `m_bDesynchronised`, and say so:

```
Warning, DisSaveLoad: stopping the level restore at
'DishonoredGameFull_P.TheWorld:PersistentLevel.WorldInfo_0.DishonoredMapInfo_0' (DishonoredMapInfo): its
GameLoad is one of the 98 retail overrides this tree has not ported, and the object stream carries no length
prefix, so reading on would desynchronise it. 0 objects were restored first.
```

A second gate covers the case retail cannot reach: a non-zero dictionary index whose object resolved to
NULL. Retail's `ShouldLoadObject` dereferences the object with no NULL check, so it never happens there;
here it is common, and skipping the object reads none of the bytes retail wrote for it. It aborts too, and is
counted separately as `unresolved`.

A **third** gate is the one section 2's finding makes necessary, and it is the subtlest of the three. An
object is in the stream iff its class's `IsSaveable` says so, retail says so for 372 classes, and this tree
declares `IsSaveable` on ten. So a class retail treats as an entry point whose nearest `IsSaveable` body
here is `UObject`'s FALSE - `ASkeletalMeshActor`, `ADisCrusher`, `ATrigger_LOS`, `APortalTeleporter` and the
rest of the 59, every one of which retail saves through `AActor::GameSave` - would be **skipped**, reading
none of the bytes retail wrote. The override gate does not catch it, because the body those objects need
(`AActor`'s) *is* ported. So when an object is not saveable here, the loader asks whether that answer is
retail's: if the nearest entry class in the chain is one of the ten whose `IsSaveable` this tree implements,
the answer is trusted (four of those ten are conditional, so a static comparison would cry wolf);
otherwise the restore stops and names the class. Counted as `untrusted skips`.

After an abort, `operator<<` returns NULL without reading, which ends the caller's
`do { Ar << pObject; } while( pObject )` loop, and the level state is discarded as retail discards it. The
consequence is stated plainly: **a level state whose stream aborts is not restored at all** — it is not half
restored, and the census says `STREAM ABORTED`.

## 5. The measurement

`-disrestoreslot=<slot>` arms the restore; it runs from Engine's new per-world-tick hook
(`GDisEngineTickHook`, `Engine/Inc/UnWorld.h`) once the map is up, which is where retail runs it
(`UDishonoredEngine::ProcessSaveLoadCmd`'s `SLC_PostLoad`, 2013 rva `0x6162d0`, not ported — this tree's
`UDishonoredEngine` has no `Tick`). Run it with:

```
python resources/tools/build_and_smoke.py --build-dir build/agentED --no-build --timeout 150 \
  --exe-name DishonoredGame_ED.exe --log-name agentED_s16.log --ini-dir build/agentED_config --rhi null \
  --skip-native OnlineSubsystemPC --expect "DisSaveLoad census" \
  "--extra-args=-forcelogflush -disrestoreslot=16 -disrestoredelay=2 -savedir=...\savetest\ "
```

Three real retail saves, three runs, the engine's own log:

| slot | save | level state | objects | `m_NumObjects` | dict bytes | data bytes read / expected | stopped at |
|---|---|---|---|---|---|---|---|
| 16 | `Dishonored0.sav`, "0 - Dunwall Tower" | `DishonoredGameFull_P` (SLL_FILE) | 11321 + 3 seeds | **11324** | **91844 / 91844** | 26 / 619631 | `DishonoredMapInfo` |
| 4 | `DisMission0.sav`, "1 - Coldridge Prison" | `DishonoredGameFull_P` | 8305 + 3 | **8308** | **67665 / 67665** | 34 / 320532 | `DishonoredMapInfo` |
| 12 | `DisMission8.sav`, "10 - Kingsparrow Isle" | `DishonoredGameFull_P` | 8215 + 3 | **8218** | **67164 / 67164** | 26 / 393344 | `DishonoredMapInfo` |

One census line in full:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (2259 resolved) 91844/91844 bytes;
data 1 restored, 0 skipped, 26/619631 bytes; 24 spawned, 9027 not found, 1 unported, 0 unresolved,
0 untrusted skips, 1 PostGameLoad; STREAM ABORTED
```

`dictionary 11321/11324 … 91844/91844 bytes` is the result: the dictionary halves of three real retail saves
are read by this tree's own code, record for record and byte for byte. `24 spawned` are the actors the
dictionary asked to be spawned from their tweaks, at the transforms it recorded. `data … 26/619631` is the
object half stopping where it should.

The offline model is the wider proof: `python build/agentED/objdict.py build/agentCF/savetest/*.sav` ends

```
146 level states OK, 0 BAD
```

— every level state of all 51 real saves, entry count equal to `m_NumObjects` and zero bytes left over.

**Acceptance 1, honestly.** The player is not placed. `DisRestore: player pawn DishonoredPlayerPawn at
X=0.000 Y=500.000 Z=-209.350 rotation P=0 Y=0 R=0` is the spawn transform, unchanged, because the stream
stops long before the pawn. The distance to it is measured, not guessed: `build/agentED/frontier.py` says the
player pawn is **dictionary record 162** of `Dishonored0.sav`'s persistent level, and **11** unported
override classes stand between the start of the stream and it:

| order | class | 2013 `GameSave` / `GameLoad` | 2012 bytes | objects in this save |
|---|---|---|---|---|
| 10 | `UDishonoredMapInfo` | `0x611780` / `0x60ba70` | 791 + 316 | 2 |
| 14 | `UDisGlobalFactionManager` | `0x85f8c0` / `0x863d80` | 276 + 379 | 1 |
| 52 | `UDishonoredInventory` | `0x8169e0` / `0x816bc0` | 529 + 379 | 27 |
| 62 | `UDisAttributes` | — / (no 2013 match) | 23 | 27 |
| 73 | `UDisDialogTree_InGameBind` | `0x8923d0` / `0x89d5b0` | 71 + 183 | 58 |
| 79 | `UDisAttentionInfo_Base` | `0x88af60` / — | 20 | 837 |
| 136 | `UDishonoredGlobalAIManager` | — / `0x841480` | 48 | 1 |
| 138 | `UDisAIBlackboard` | `0x730060` / `0x7350d0` | 210 + 207 | 25 |
| 150 | `UDishonoredObjectivesComponent` | (no 2013 match) / — | 23 | 1 |
| 152 | `UDishonoredObjective` | `0x6d4cd0` / `0x6d4d60` | 118 + 108 | 5 |
| 160 | `UDishonoredTask_Base` | — / (no 2013 match) | 42 | 7 |

**3,723 bytes of x86 in eleven classes**, then `ADishonoredPlayerPawn` (`0x6b8770` / `0x6b8b60`, 802 + 773)
and `ADishonoredPawn` (`0x755be0` / no match, 609 + 964), and the player's transform comes out of
`AActor::GameLoad`, which is already ported. That is the whole of what stands between this package and
milestone 7, and every address in the table was resolved against `retail2013_named.i64`.

## 6. What is ported, and what is not

**Ported (14 classes, all in Engine, in the files retail has them in):**

| file | classes |
|---|---|
| `Engine/Src/UnSequence.cpp` | `USequenceObject` (`IsSaveable`, `GameSave`, `GameLoad`), `USequenceOp`, `USequence` (+`IsSaveable`), `USequenceEvent`, `USeqAct_Latent` (`GameLoad`) |
| `Engine/Src/UnActor.cpp` | `AActor` (the base pair), `ATrigger`, `ATargetPoint` (one folded pair shared by both), `AInterpActor` |
| `Engine/Src/UnWorld.cpp` | `AWorldInfo` |
| `Engine/Src/UnParticleComponents.cpp` | `AEmitter` |
| `Engine/Src/UnSceneCapture.cpp` | `ASceneCaptureActor` |
| `Engine/Src/UnLight.cpp` | `ALight` (+`IsSaveable`) |
| `Engine/Src/UnPhysActor.cpp` | `AKActor` (+`IsSaveable`) |
| `DishonoredGame/Src/dissavegame.cpp` | `UDisSeqAct_AutoSave::PostGameLoad` |

**Not ported: 98 classes.** `DishonoredGame/Inc/dissaveload_classlists.h` has the list; reaching any of them
stops the restore and names it. The 19 a real save's persistent level actually reaches, in stream order, are
in `build/agentED/handover_table.txt`; the eleven that matter first are section 5's table. Also not ported:

* **`USeqAct_Interp`'s three save virtuals** (2013 `0x2e73b0` / `0x2e74d0` / `0x2ea1c0`). They flatten the
  matinee's `UInterpGroupInst` state into the reflected `m_SavedGroupInstData` byte array through
  `UInterpGroupInst::SaveData` / `LoadData`, two more Arkane additions this tree does not have. 91 of them in
  one save; the declaration is out of the header with a note, so the gate catches them.
* **`DisSaveLoad::FLevelSaver`** (2013 `0x615730`) and **`FGameState::SaveLevel`** (`0x613cb0`) and
  **`SaveGameState`** (`0x602920`) — the whole writing half. Deliberately left out rather than left
  half-right: with 14 of 112 override classes it would write files that look valid and are not, which is the
  defect shape this wave's rules single out. Their declarations were removed from the header again for the
  same reason. The 2012 decompiles are in `build/agentED/dec2012/`, and the two things the level selection
  needs are resolved below so the next agent does not have to.
* **`UDishonoredEngine::ProcessSaveLoadCmd`** (`0x6162d0`) and **`DoSaveGame`** (`0x6095a0`), the state
  machine. `RestoreLoadedLevels()` is `SLC_PostLoad`'s inner half only; the travel half is the map the engine
  already opened.

## 7. Everything the next agent would otherwise have to re-derive

* **`ULevelStreaming::IsSubLevelUnshared()`** is vtable slot 74. The base returns **TRUE** (2012 fold on
  `USequence::IsStandalone`); only `ULevelStreamingAlwaysLoaded` overrides it, as
  `!m_bConsiderForPartialSaves` (2012 rva `0x3b1050`, `return (flags@160 & 2) == 0`, and
  `m_bConsiderForPartialSaves` is the second bit of the DWORD at 160). Both are now in `EngineClasses.h`.
  `USequence::IsSaveable` and `DisSaveLoad::IsSubLevelUnshared` are the readers.
* **`AActor`'s packed flag byte** (the first byte of `AActor::GameSave`), resolved against the 2012 PDB bit
  offsets, not guessed from the decompiler's masks: bit 0 `m_bOutOfBendTime`, bit 1 `bHidden`, bit 2
  `m_bShutDown`, bit 3 `ReplicatedCollisionType != 9`, bit 4 `GeneratedEvents.Num() > 0`. A shut-down actor
  writes nothing else and on load runs its script `ShutDown` and stops. The transform is written only when
  `!bStatic && bMovable && ( !m_bSpawned || m_bPersistsAcrossLevelTransition )`.
* **`FSeqOpInputLink`'s one byte** is `QueuedActivations + bHasImpulse`, not `ActivateDelay`: offsets 12 and
  16 of the 2012 struct are `bHasImpulse` and `QueuedActivations`, and `ActivateDelay` is at 36.
  `0` means no impulse, `N` means an impulse with `N-1` activations queued.
* **`FArchiveLoadCompressedProxy::Serialize` calls itself recursively** to cross its buffer boundary, so a
  byte counter in an override double-counts (it reported 10,217 bytes for a 26-byte read). Use `Tell()`.
* **`FDisTaskTargetsSaveData` changed between the builds**: 2012 has `UDishonoredTask_Base* m_pTask` at 0,
  retail has `INT m_TaskID`. `UDishonoredMapInfo::GameLoad` needs a task lookup by id, and
  `UDishonoredTask_Base::GetTargets` does not exist in this tree.
* **A level state is restored per *loaded* level.** `FGameState::LoadLevel` matches on `DisGetLevelName`,
  which is the level's outermost package name; the memory states (`loc` 0 and 1) of levels the current map
  does not stream are simply skipped, which is why a `-disrestoreslot` run restores one level state and
  leaves the rest. `DisMission8.sav`'s `L_Pub_Assault_P` state is 45 objects / 16 saved / 716 bytes and its
  **entire** override set is the sequence classes this package ports plus `UDisSeqAct_AutoSave::PostGameLoad`
  — so it is the cheapest complete byte-exact restore of a real retail level state available, and it needs
  only that its level be resident. See the hand-over.

## 8. Deviations, stated plainly

1. **The dictionary reads each record's shape from the record, not from the live object** (section 3). This
   is not what retail does and it is the reason the decode works here at all.
2. **A dictionary index that resolves to NULL aborts the restore.** Retail would crash; doing nothing would
   desynchronise. Counted as `unresolved`.
3. **`DisSpawnActorFromTweaks` is a bring-up stand-in** for `UDisTweaksBase::SpawnActor(eDisTweaksSpawnType_InGame, …)`:
   it spawns the tweaks' `m_pSpawnedObjectClass` at the recorded transform and hands the actor its tweaks,
   which is the part the transform is for. The rest of retail's spawn path (the fixups, the spawn-type
   bookkeeping) is a hand-over. 24 actors came back this way in slot 16.
4. **`AInterpActor::GameLoad` does not push the rigid body** to the restored transform (retail's
   `setGlobalPosition` / `setGlobalOrientationQuat`); `UnActor.cpp` does not include the Novodex headers. It
   consumes no stream bytes, so it cannot desynchronise — a restored mover is simply not re-synced with
   PhysX until it next moves. `AKActor::GameLoad` does do it, because `UnPhysActor.cpp` has the headers.
5. **`UDisSeqAct_AutoSave::PostGameLoad` lives in `dissavegame.cpp`**, not retail's
   `disseqact_autosave.cpp`, because that unit is on `DishonoredGame_EXCLUDE` in a generated `Sources.cmake`
   and taking it off the list would not survive the next regeneration.
6. **The restore is driven from `UWorld::Tick`** through a new function pointer (`GDisEngineTickHook`, NULL
   by default) instead of `ProcessSaveLoadCmd`, because `UDishonoredEngine` has no `Tick` in this tree. It is
   seven lines in `UnLevTic.cpp` and nine in `UnWorld.h`, and it is the natural place for the state machine
   when someone ports it.
7. **`-disdictdebug=<n>`** logs the first n dictionary records and how each resolved. It is how the
   132-of-11,324 divergence was found; it costs nothing when unset.
8. **`AActor::GameLoad` resizes `GeneratedEvents` with `AddZeroed`, not `Add`.** Retail uses
   `TArray::Resize` and reads every element immediately after, so it never sees uninitialised memory; here
   the read can stop half way when the stream aborts, and the garbage collector walks that reflected
   `TArray<USequenceEvent*>` next. One word, and the class of defect that builds green and crashes later.

## 9. Verification

* **Dictionary, in the tree, on real saves**: the three runs of section 5.
  `build/agentED/agentED_s{16,4,12}.log` are copies of the engine logs.
* **Dictionary, offline, on all 51 saves**: `python build/agentED/objdict.py build/agentCF/savetest/*.sav`
  -> `build/agentED/objdict_all.txt`, 146 `OK`, 0 `BAD`.
* **Regression**: `python resources/tools/run_regression.py --build-dir build/agentED_release --no-build`
  -> **31 ok, 0 failed, 0 skipped, 423 s** (`build/agentED_release/regression/summary.txt`, and the run log is `build/agentED/regression.log`). Layout unchanged: this package adds no
  reflected type and no member, so `layout_types` 2314 / `layout_mismatching` 0 / `layout_contract` 0 stand.
* **Release build**: `build/agentED_release_build.cmd` -> DishonoredGame, CoreSmoke and LayoutProbe, 0 errors.
* **Built from the snapshot, not the shared tree.** Agents EA (GFx3/GFxUI) and EE (the renderer) are editing
  the shared tree at the same time; `build/agentED_sync.py` copies only this package's 22 files into
  `build/agentED_wt` (detached at `c598a21`), so nothing of theirs is reverted and nothing of theirs can
  break this build.

## 10. Hand-overs

1. **Coordinator, at merge.** This package changes `Core/Inc/UnObjBas.h` (five virtuals on `UObject` and the
   `ESaveLoadLocation` enum) and `Engine/Inc/UnWorld.h` + `Engine/Src/UnLevTic.cpp` (the tick hook) — both
   outside DishonoredGame and both load-bearing. `Engine/Inc/EngineClasses.h`,
   `EngineSequenceClasses.h`, `EngineGameEngineClasses.h`, `EngineLightClasses.h` and
   `EnginePhysicsClasses.h` gain declarations. **This package will change generated output**: it adds a
   `CppText` declaration to `UDishonoredEngine` (`RestoreLoadedLevels`) and to `UDisSeqAct_AutoSave`
   (`PostGameLoad`), so `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` must be
   run at merge — **agent ED did not regenerate**. One new file,
   `DishonoredGame/Inc/dissaveload_classlists.h`, is generated by `build/agentED/gen_classlists.py` and is a
   header, so `Sources.cmake` does not change. `ESaveLoadLocation` left
   `dishonoredutilities_saveload.h`; anything that included only that header for the enum now needs Core,
   which every DishonoredGame unit already has.
2. **Whoever takes the next slice — and the order to take it in.** Port the eleven classes of section 5, in
   that order, and then `ADishonoredPawn` and `ADishonoredPlayerPawn`. Each one is a name in
   `build/agentED/gen_classlists.py`'s `PORTED` set (and in `PORTED_ISSAVEABLE` too when you declare the
   class's own `IsSaveable`) away from being let through the gates, and the census tells you at once
   whether the next object's bytes still line up — the loop is: add a body, regenerate the lists, rerun
   `-disrestoreslot=16`, read `data N restored, X/619631 bytes`, and the stream says if you were wrong.
3. **The cheapest complete proof, for whoever wants one before the eleven.**
   `DisMission8.sav`'s `L_Pub_Assault_P` level state is 45 objects, 16 saved, 716 bytes, and its entire
   override set is already ported (`USequence`, `USequenceOp`, `USequenceObject`, `USequenceEvent`,
   `USeqAct_Latent`, plus `UDisSeqAct_AutoSave::PostGameLoad`). It needs that level resident; the objects it
   names live in `L_Pub_Craftsman`'s `Main_Sequence`. `python build/agentED/dump_level.py
   build/agentCF/savetest/DisMission8.sav L_Pub_Assault_P` prints all 42 records and the set.
4. **Whoever ports `FLevelSaver`.** Two things its level selection needs are resolved in section 7 and
   nowhere else: `ULevelStreaming::IsSubLevelUnshared` (slot 74, base TRUE) and the `ULevelStreaming`
   bitfield at 2012 offset 92 (`bIsVisible` bit 0, `bHasUnloadRequestPending` bit 2 — the `& 4` the saver
   tests, `bShouldBeLoaded` bit 7, `bShouldBeVisible` byte 93 bit 0). `bSaveWorldInfo` is "the streaming
   object is a `ULevelStreamingPersistent`".
5. **Whoever ports `ProcessSaveLoadCmd`.** `GDisEngineTickHook` is where the state machine goes; drop the
   hook and `DisSaveLoadArmRestore` when it lands. `RestoreLoadedLevels()` is `SLC_PostLoad`'s per-level
   arm and can be called from it unchanged.
6. **Agent DP's deletion backlog (package EF).** Nothing in this package's path was found to be a shim, but
   one member *had* changed shape between the builds and is worth adding to the audit:
   `FDisTaskTargetsSaveData::m_pTask` (2012) is `m_TaskID` in retail, and `UDishonoredTask_Base::GetTargets`
   has no declaration here at all.

## 11. Continuation (merged HEAD `6723640`): across the frontier, and what actually stops the restore

The brief for this continuation was section 5's accept 1: a real retail save putting the player back where it
says. The frontier measured there — "eleven unported override classes before the player pawn at dictionary
record 162" — was followed, and it turned out to be **an undercount of my own making**. Correcting it is the
main result of this continuation, because it changes what the remaining work is.

### 11.1 The correction: 190 classes were invisible to the vtable census

`build/agentED/gen_classlists.py` and `frontier.py` both read `resources/docs/symbols/vtables.csv` and skipped
every row whose class name contains `{for `. MSVC emits **one vftable per base of a multiply-inheriting
class**, and the demangler names each of them `X{for <base>}` — the primary one, the one that carries UObject's
slots, included. So every class with more than one base was dropped: `ADishonoredGameInfo`,
`ADishonoredPlayerPawn`, `ADishonoredPawn`, `UDisAttentionInfo_Base`, `UDisNPCTravelManager` and about 185
others. The generator now collapses the variants and keeps the one that has slot 68, which takes the
entry-class list from **372 to 562** — the untrusted-skip gate was blind to 190 classes, and the frontier table
was missing every multiply-inheriting class on it.

With that fixed, `build/agentED/frontier2.py` gives the real picture for `Dishonored0.sav`'s persistent level:
**60 override bodies are reached in the level state, 25 now ported and 35 not, and 8 of the missing ones are
reached before the player pawn**:

| order | class | 2013 `GameSave` / `GameLoad` | 2012 bytes | objects | note |
|---|---|---|---|---|---|
| 12 | `ADishonoredGameInfo` | `0x5fa210` / `0x5fa3f0` | 487 + 538 | 1 | **the stream stops here** |
| 43 | `ADishonoredSpawner` | `0x659380` / `0x65ec30` | 188 + 300 | 41 | |
| 133 | `UDisNPCTravelManager` | `0x6e0290` / `0x6e28a0` | 711 + 587 | 1 | |
| 140 | `UDisGlobalUIManager` | `0x856f10` / `0x856f70` | 93 + 104 | 1 | |
| 142 | `UDisGFxMoviePlayerHUD` | `0x7a5a90` / `0x7ac2e0` | 247 + 434 | 1 | **agent EA owns the movie players** |
| 144 | `UDisGFxMoviePlayerPowerWheel` | — / `0x7b9e50` | 133 | 1 | **agent EA** |
| 146 | `UDisPostProcessManager` | (no 2013 match) | 401 + 418 | 1 | |
| 148 | `ADishonoredPlayerController` | `0x6a3140` / `0x6a31e0` | 133 + 147 | 1 | |

Two of the eight are the `DishonoredGame` movie players, which are agent EA's for this wave, so the frontier
cannot be crossed without either EA's area or EA's agreement. That is stated rather than trespassed on.

### 11.2 What was ported, and the frontier moving

Fourteen more classes, each verified by the census advancing:

| class | what |
|---|---|
| `UDishonoredMapInfo` | 2013 `0x611780` / `0x60ba70`: the per-squad live counts, then one object reference and the objective task targets |
| `UDisGlobalFactionManager` + `FDisRelationshipOverrideInfo` | `0x85f8c0` / `0x863d80` and `0x8598b0` / `0x8599f0`: a counted list of factions, each with two counted lists of (object, relationship) pairs |
| `UDishonoredInventory` | `0x8169e0` / `0x816bc0`: the loadout struct, the re-equip item and type per slot, the backup loadout |
| `UDisAttributes` + `operator<<(FDisModifiedAttribute)` + `operator<<(FDisAttributeModifier)` | the modified-attribute map; the struct serializer is 2012 `0x8ef4d0` and is **not** a member walk (three floats, one packed byte, and the modifier map only when non-empty; `m_fCachedModifiedValue` is never in the stream) |
| `UDisDialogTree_InGameBind` | `0x8923d0` / `0x89d5b0`: the object's properties, then the active running instance as a script struct |
| `UDisAttentionInfo_Base` | `0x88af60`: the object's own properties. 871 of them in one save — the most numerous class in the set |
| `UDishonoredGlobalAIManager` | `0x841480`: the attention tag and the global blackboard reference |
| `UDisAIBlackboard` | `0x730060` / `0x7350d0`: the properties and a counted record list |
| `UDishonoredObjectivesComponent`, `UDishonoredObjective`, `UDishonoredTask_Base` | the objective tree, including both real `IsSaveable` bodies |
| `UDishonoredPowersComponent` | `0x6d3a60` / `0x6d3c40`: a BYTE count then (FName, level+1) pairs, the stored levels and one inhibit flag |
| `ADishonoredPlayerPawn` (`GameLoad` only) | `0x6b8b60`, as far as the Super call, with `SaveLoadTutorialTrackers` (2012 `0x6fa970`) and `GameLoad_Body` (2012 `0x6fe820`) |
| `ADishonoredPawn` (`GameLoad` only) | 2012 `0x79b360`, **only as far as `AActor::GameLoad`**, which is where the transform lands; it then stops the stream and says so |

Plus nine `IsSaveable` declarations taken from slot 67 — nine inline `return TRUE` bodies and, for
`UDishonoredGlobalAIManager` and `ADishonoredPlayerPawn`, the real one at 2012 `0x6fa960`,
`return Location == SLL_FILE`, which retail shares with `UDisNPCTravelManager`.

The census moved as the loop predicted:

```
before:  data 1 restored, 26/619631 bytes; 1 unported    -> stopped at DishonoredMapInfo
after:   data 2 restored, 28/619631 bytes; 1 unresolved  -> stopped at dictionary index 12
```

Two objects restored — `AWorldInfo` and `UDishonoredMapInfo` — with every byte accounted for. 28 bytes is the
whole of the stream to that point: 2 for the deleted-actor count, 2 for the object index, 8 for
`AActor::GameLoad`, 12 for the world clocks, 2 for `MyMapInfo`, 2 for the map info's own object reference.
Checked against the raw bytes with `build/agentED/head_bytes.py`, not inferred from the decompiler.

### 11.3 What actually stops the restore, and it is not the override count

The stream now stops at **dictionary record 12 because the object is not there**:

```
Warning, DisSaveLoad: stopping the level restore at dictionary index 12, which resolved to no live object in
this session: retail wrote that object's state inline with no length prefix, so its bytes cannot be skipped.
2 objects were restored first.
```

`-disdictdebug=170` names every record, and this is the shape of it:

```
DisDict  12: class DishonoredGameInfo             name DishonoredGameInfo         outer 6   -> NOT FOUND
DisDict 133: class DisNPCTravelManager            name pNPCTravelManager          outer 12  -> NOT FOUND
DisDict 140: class DisGlobalUIManager             name pGlobalUIManager           outer 12  -> NOT FOUND
DisDict 142: class DisGFxMoviePlayerHUD           name pHUD                       outer 140 -> NOT FOUND
DisDict 144: class DisGFxMoviePlayerPowerWheel    name pPowerWheel                outer 140 -> NOT FOUND
DisDict 146: class DisPostProcessManager          name pPpManager                 outer 12  -> NOT FOUND
DisDict 148: class DishonoredPlayerController     name DishonoredPlayerController outer 6   -> ...PersistentLevel.DishonoredPlayerController
DisDict 150: class DishonoredObjectivesComponent  name DisObjComp                 outer 148 -> NOT FOUND
DisDict 162: class DishonoredPlayerPawn           name DishonoredPlayerPawn       outer 6   -> ...PersistentLevel.DishonoredPlayerPawn
DisDict 164: class DishonoredPowersComponent      name PowersComp                 outer 162 -> NOT FOUND
```

**The player pawn and the player controller both resolve.** What does not exist in this build is:

1. **no `ADishonoredGameInfo` actor named `DishonoredGameInfo` in the persistent level** — retail's name is
   bare, not `_0`, so retail named it explicitly rather than letting `MakeUniqueObjectName` number it. Five of
   the eight frontier classes are its sub-objects (`pNPCTravelManager`, `pGlobalUIManager`, `pHUD`,
   `pPowerWheel`, `pPpManager`), so they cannot resolve either, whatever their `GameLoad` does;
2. **no `DisObjComp` on the player controller** (`UDishonoredObjectivesComponent`, outer 148);
3. **no `PowersComp` on the player pawn** (`UDishonoredPowersComponent`, outer 162).

So what stands between this tree and milestone 7 is no longer the override count. It is that **the game does
not construct the object trees a real save names**: the game info actor and two per-actor components. Porting
more `GameLoad` bodies cannot help until those objects exist, because the stream stops on the object, not on
the body. That is a different package from this one: it belongs with whoever owns `ADishonoredGameInfo`'s
construction and the component instancing, not with the save layer.

The last two bodies are ready for the day those objects appear. `ADishonoredPawn::GameLoad` calls
`AActor::GameLoad` first and the pawn's `Location` and `Rotation` come straight out of it, so the moment the
stream reaches record 162 the transform lands with no further porting; the diagnostic it prints when it stops
names the restored transform, which is the measurement accept 1 asks for.

### 11.4 Two more things this continuation had to find out

* **`-noscenerender` is how the restore can be run at all.** On the startup map `DishonoredGameFull_P`,
  `UWorld::Tick` runs **exactly twice** under the null RHI — measured, `-disrestoredelay=2` fires and
  `-disrestoredelay=3` never does — so no sub-level streaming can finish; and under d3d9 the first scene render
  asserts in `FHeightFogShaderParameters::Set` -> `SetShaderValue` (`Parameter.IsInitialized()`,
  `FogRendering.cpp:97`), which is agent EE's file and pre-existing at this HEAD. The regression's d3d9 stage
  never sees it because it runs `-startmapopen L_Tower_P` instead. `--rhi d3d9 ... -noscenerender` ticks
  normally and brings the sub-levels up: `DisRestore: the world is up (3 streaming level(s), all visible 1)`.
  Every number in this section comes from that path. This also matters for the map info: retail's
  `DishonoredGetMapInfo()` reads the *streaming persistent* world info (2012 rva `0x827020`), so with no
  sub-levels up the branch inside `UDishonoredMapInfo::GameLoad` does not match what the save recorded.
* **A ported body must go quiet after an abort.** The three gates stop `operator<<(UObject*&)`, but a body
  already entered keeps reading plain data; `UDishonoredMapInfo::GameLoad` read a garbage count that way and
  died allocating a 16 MB `FString`. `FLevelLoader::Serialize` now returns zeros without touching the stream
  once `m_bAborted` is set, so every ported body finishes on zeros and the level state is discarded. The byte
  count deliberately stays on `Tell()`: the proxy's own `Serialize` recurses to cross its buffer boundary, so a
  counter in an override double-counts — it once reported 10,217 bytes for a 26-byte read.

### 11.5 The coordinator's two follow-ups

1. **`DisMission8.sav`'s `L_Pub_Assault_P` level state** cannot be restored yet, for the same reason and not a
   new one: `FGameState::LoadLevel` matches a level state to a *loaded* level, and the objects that state names
   live in `L_Pub_Craftsman`'s `Main_Sequence`, which no run of this build has resident. Getting it resident
   needs either the mission travel (`-startmapopen` replaces the persistent map, so the save's
   `DishonoredGameFull_P` state then matches nothing) or the `STREAMMAP` path, which asserts in the fog code
   above. It is still the cheapest complete proof available, and it is now blocked on world state rather than
   on the override set.
2. **The writing half** stays out, as agreed. With 25 of 112 override classes and three object trees the game
   does not build, a saved file would not be honest.

### 11.6 Verification of the continuation

* The frontier moving, on the real path: `build/agentED/agentED_ns.log` (the census above) and
  `build/agentED/agentED_dd.log` (the 170-record dictionary dump).
* The dictionary halves are unchanged and still exact: 11321/11324 records and 91844/91844 bytes, now with
  2260 of 11321 records resolving instead of 2259 (the extra one is the map info's own reference).
* The offline model still decodes all 51 saves: 146 level states OK, 0 BAD.
* Regression **31 ok, 0 failed** on `build/agentED_release`, and a clean release build of all three targets,
  0 errors.
* Everything built from `build/agentED_wt`, now detached at the merged `6723640`, with only this package's own
  files synced in (`build/agentED_sync.py`).

### 11.7 Hand-overs from the continuation

1. **Coordinator, at merge.** The same regeneration as before and more of it: this continuation adds
   `Inc/CppText` hooks for `UDishonoredMapInfo`, `UDisGlobalFactionManager`, `UDishonoredInventory`,
   `UDisAttributes`, `UDisDialogTree_InGameBind`, `UDisAttentionInfo_Base`, `UDishonoredGlobalAIManager`,
   `UDisAIBlackboard`, `UDishonoredObjectivesComponent`, `UDishonoredObjective`, `UDishonoredTask_Base`,
   `UDishonoredPowersComponent`, `ADishonoredPawn`, `ADishonoredPlayerPawn`, `FDisRelationshipOverrideInfo`,
   `FDisModifiedAttribute` and `FDisAttributeModifier`, so
   `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` must run at merge. The
   `#include "CppText/<Class>.h"` lines are in the generated headers by hand for now — that is exactly what the
   generator emits, at the same place — and every touched file is listed in `build/agentED_sync.py`.
2. **The next package is not the save layer.** It is whoever can make the game construct
   `DishonoredGameFull_P.<level>.DishonoredGameInfo` (bare, not `_0`), `DishonoredPlayerController.DisObjComp`
   and `DishonoredPlayerPawn.PowersComp`. Section 11.3 has the evidence; `-disdictdebug=<n>` reproduces it in
   one run and will say when they appear.
3. **Agent EA, or whoever follows them.** `UDisGFxMoviePlayerHUD::GameLoad` (2013 `0x7ac2e0`, 434 bytes) and
   `UDisGFxMoviePlayerPowerWheel::GameLoad` (`0x7b9e50`, 133) are two of the eight bodies before the player
   pawn and they are in the movie players. Both classes are save entry points (their vtable slot 67 is the
   inline `return TRUE` fold).
4. **Agent EE.** `FHeightFogShaderParameters::Set` asserts `Parameter.IsInitialized()` on the first scene
   render of `DishonoredGameFull_P` under d3d9 (`FogRendering.cpp:97`, reached from
   `TBasePassVertexShader<FNoLightMapPolicy,FNoDensityPolicy>::SetParameters`). The regression never sees it
   because its d3d9 stage opens `L_Tower_P` instead. The full stack is in `build/agentED/agentED_d2.log`.
5. **Whoever audits the tooling.** Any script that reads `resources/docs/symbols/vtables.csv` and filters out
   `{for ` is silently ignoring every multiply-inheriting class — 190 of them here, including all the pawns and
   the game info. `build/agentED/gen_classlists.py`'s `base_name` collapse is the fix, and it is worth grepping
   the other tools for the same filter.
6. **One inference to check when someone has better evidence.**
   `operator<<(FArchive&, FDisAttributeModifier&)` is inline in retail with no PDB symbol; its three fields in
   declaration order are inferred. It is the only inference in this continuation, and if it is wrong the census
   desynchronises at the first attribute that carries a modifier, which is detectable rather than silent.
