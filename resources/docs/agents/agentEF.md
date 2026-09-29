# Agent EF (PHASE11 EF) — the saved session, not just the saved position

Branched from the merged HEAD `79ead3d`. Own worktree `build/agentEF_wt`, own build dirs `build/agentEF_release`
and `build/agentEF_clean`, own IDA copy `build/agentEF_ida/retail2013_agentEF.i64`, headless decompiles only
(50 retail functions into `build/agentEF/dec2013/`), no IDA MCP tools and no FModel tools, no junctions,
nothing deleted under `Dishonored_Latest2026`. Nothing committed, nothing staged.

## The answer in nine lines

* **The saved session comes back, and the numbers are the save's own bytes.** `Dishonored0.sav` restores
  **health 70** and **mana 100** and reads the inventory loadout; `Dishonored1.sav` restores **health 70**,
  **mana 100**, **ammo 4/0/8/3/2**, **5 health elixirs**, **6 abstract items**, **4 keys on the key ring** and
  a **darkness score of 12** - and the save names those four keys "Corvo's Cell Key", "Yard Walkway Key",
  "Yard Key" and "Dunwall Sewer Gate Key". Every one of those was decoded independently out of the save file,
  in Python, without the game and without the C++ reader (section 3.1).
* **The object stream ran from 4,141 bytes to 23,827 of 619,631**, and objects restored from **57 to 305**.
  On the second save, from 2,716 of 294,961 to **17,842**, and 41 objects to **180**.
* **Twenty-seven retail bodies are ported** across **ten new override classes**: the five active-power
  component classes the stream stopped on, the whole of `ADishonoredPlayerPawn::GameLoad` past its Super call
  with its four helpers, the key ring and its element serializer, the darkness manager, the visibility
  component, the audio-log dummy, the NPC controller, three struct serializers, `TScriptInterface`'s
  save-archive member, and the half of `SpawnInventoryLoadout` that needs no item factory.
  **Ported override classes 37 → 47, unported 75 → 65.** `agentEF_status.csv` has every address.
* **The new stopping point is the AI brain of an NPC controller** -
  `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredNPCController_<n>.AIBrain`, class
  `UDishonoredAIBrain`, retail `0x7256a0` (the instance number varies with spawn order; the class, the byte
  count and the census do not). `0x7256a0` is retail's own vtable slot 70; `match_2012_2013.csv` leaves the
  2012 body unmatched. Seven of the seventeen bodies still missing from this save are the AI stack behind
  it, records 716..751.
* **Three retail addresses in this package exist nowhere but retail's own vtables**, because
  `match_2012_2013.csv` has no row for the 2012 body: `UDishonoredActivePowerComponent_DevouringSwarm::GameLoad`
  (`0x7fe3c0`), `UDishonoredActivePowerComponent_BendTime::PostGameLoad` (`0x7e71d0`) and
  `ADishonoredNPCController::GameLoad` (`0x763570`). `build/agentEF/dump_vt2013.py` is how they were resolved,
  and it also settles a fact no brief had stated generally: **retail's save five are vtable slots 67..71**, one
  further along than the 2012 build's 66..70, so retail's `GameSave` is slot 69 and `GameLoad` slot 70.
* **Four places where the 2012 build is not retail, and retail won.** `UDisActivePowerComponent_DarkVision`
  has two bodies in retail where 2012 folds them; `UDisDarknessManager` folds in retail where 2012 has two;
  `ADishonoredPlayerPawn::m_Upgrades` is serialised through `TArray<UObject*>`, not the `TArray<float>` the
  match table names; and `ADisPlayerAudioLogDummyActor`'s dialog answers come from retail's vtable, not the
  2012 fold.
* **One defect of my own, caught by looking rather than by a crash, and it is the lesson of this package.**
  The first version of `GameLoad_Dialog` stopped at retail's `FDisConvSaveData` branch and left out
  `GameLoad_Dialog_Derived`, ~100 bytes. The stream did not stop on a bogus count: it ran on and stopped at
  `ADisPlayerAudioLogDummyActor` — **which is the object the first line of the missing tail names** — so the
  stop looked exactly like an honest unported-override frontier. A plausible stopping point is not proof.
* One crash: `operator<<(FArchive&, FDisKeyInfo&)` assigned to the struct's two `FString`s where retail
  memzeroes the 28 bytes raw. `TArray::operator<<` placement-news each element into uninitialised memory, so
  the assignment freed a garbage pointer. `Dishonored1.sav`, which has four keys, died on it; raw zeroing is
  retail's own instruction sequence and fixes it.
* Regression **31 ok, 0 failed, 0 skipped, 449 s**; clean full release build of all three targets, 0 errors;
  `rva_sweep` over this package's files: **385 citations, 0 mislabelled, 0 unknown**, and every address this
  package cites in source is `ok-2013`, a retail function start. Agent EC's hand-over 5 is done: the
  ICF-fold check is a step of `gen_classlists.py`, reports 0 at this HEAD, and `make_foldselftest.py` proves it
  can fire by reproducing agent EC's own defect exactly.

## 1. What the stream stopped on, and what came after it

Agent EC's frontier was `DishonoredPlayerPawn.PowerBlink`, class `UDishonoredActivePowerComponent`, byte 4141.
That stop is inside `ADishonoredPawn::GameLoad`'s `m_ActivePowers` loop, which is the **third** thing the pawn
reads — so the transform and `Health` were already back and everything after was not. The save names six power
components there, and five classes:

| class | retail `GameSave` / `GameLoad` | what it reads |
|---|---|---|
| `UDishonoredActivePowerComponent` | `0x7e70e0` (one body, both slots) | `m_CurrentLevel`, and nothing else. 24 bytes of x86 |
| `_BendTime` | `0x7e7150` / `0x7f8980`, PostGameLoad `0x7e71d0` | the level, a packed byte of three of its four bits, and the elapsed running time. `PostGameLoad` turns that back into an absolute `m_fTimeAtStart` |
| `_DevouringSwarm` | `0x7e75e0` / **`0x7fe3c0`** | the level, the rat-spawner level, the spawn point, the swarm, two floats, a packed byte of three bits, and the elapsed bend time when the swarm is pending |
| `_Possess` | `0x7ed930` / `0x7edb00` | the level, the possessee through `TScriptInterface::SerializeForSaveLoad`, the timer, the saved rat spawner, the possessee location, one bit, the status message and its tutorial index |
| `UDisActivePowerComponent_DarkVision` | `0x7e8b70` / **`0x7f8620`** | the level and the three fade/run/fade-out timers |

`_Blink` and `_WindBlast` inherit the base's body, which is why six components need five classes.

Past them, `ADishonoredPawn::GameLoad` finishes (agent EC's port, unchanged) and
**`ADishonoredPlayerPawn::GameLoad` (retail `0x6b8b60`, 1105 bytes to the 2012 body's 771) runs in full** for
the first time. Agent ED had ported it as far as the Super call. In order, past that call:

| # | reads |
|---|---|
| 1 | `m_Mana`, `m_ManaRegenAmount`, `m_fManaRegenCountDown` — **the mana triple** |
| 2 | `GameLoad_Inventory` (`0x6ad350`): the equipped power component, the ranged weapon's ammo type, the crossbow's charge count |
| 3 | `GameLoad_Dialog`: `m_DialogSaveData` as a binary script struct, then `GameLoad_Dialog_Derived` (`0x6b2fe0`) — `m_pAudioLogConvActor`, `m_DialogSaveData_Global`, and a counted list of `FDisStoryFlagInstance` |
| 4 | `GameLoad_Stealth` (`0x6d3950`): `m_pScriptedPlayerVisSettings` and `FDisPlayerStealthSaveVars` |
| 5 | the master and upper player FSMs' `LoadPartialState` |
| 6 | the adrenaline pair |
| 7 | `m_pKeyRing`, `m_pDarknessManager` |
| 8 | one crouch byte, then **one byte retail reads and does nothing with** |
| 9 | `m_StatValues`, `m_MissionStatValues` |
| 10 | **80** `FAchievementTracker` structs, each followed by its streak time stamps turned from absolute to elapsed. The count is the array size for the save's version (81 at `Ver >= 23`, 71 at 22, 51 below) and retail walks indices **1..Count-1**, leaving element 0 out of the stream in every version |
| 11 | `m_Upgrades`, an INT flag, and `m_Upgrades_Backup` behind it |
| 12 | ten `m_WhaleBoneCharmSlots`, `m_WhaleBoneCharms`, `m_AvailableBoneCharmInfoIndexes` |
| 13 | `m_LastSecondLocation`, `m_pVisibilityComponent`, `Velocity`, `m_pClimbable`, `m_PowerInhibitedMessageID` |
| 14 | **43** tutorial-note bits, eight to a byte (40 at version 22, 33 below) |

Rows 7 and 13 name three objects whose own `GameLoad` had to come with the body — `UDisKeyRing` (`0x6dd250`),
`UDisDarknessManager` (`0x852ca0`) and `UDishonoredVisibilityComponent` (`0x6c64d0` through `SaveLoadCommon`
`0x6c1950`) — and row 3 names a fourth, `ADisPlayerAudioLogDummyActor` (`0x644220`, `PostGameLoad` `0x644280`).

With those in, the stream reached `ADishonoredNPCController::GameLoad` (`0x763570`), which agent EC had
confirmed offset by offset and left ready; porting it is five reads and moved the frontier one more class.

## 2. Where the 2012 build is not retail

Four, and each was found by checking retail's own vtable rather than the match table.

1. **`UDisActivePowerComponent_DarkVision` has two bodies in retail** — `0x7e8b70` (slot 69, `GameSave`) and
   `0x7f8620` (slot 70, `GameLoad`) — where the 2012 build folds both onto `0x828730`. `vtables.csv` therefore
   reports a fold retail does not have. The two read the same four values, which is why one ported body serves
   both here; the difference is that retail's load side then re-arms the power's post-process node and sky
   light, and the save side does not.
2. **`UDisDarknessManager` is the other way round**: retail folds `GameSave` and `GameLoad` onto `0x852ca0`
   where the 2012 build has two bodies (`0x8c13a0` / `0x8c13d0`).
3. **`ADishonoredPlayerPawn::m_Upgrades` is a `TArray` of object references**, serialised through
   `operator<<(FArchive&, TArray<UObject*>&)` at retail `0x24c3f0`, whose elements go through the archive's own
   `operator<<(UObject*&)`. `match_2012_2013.csv` names that address
   `??6@YAAAVFArchive@@AAV0@AAV?$TArray@MVFDefaultAllocator@@@@@Z` — `TArray<float>`. It is not: `TArray<T*>`
   folds across every `T` and the matcher picked the wrong member of the fold. Reading those elements as floats
   would have desynchronised the stream at the first upgrade the player has.
4. **`ADisPlayerAudioLogDummyActor`'s two dialog answers** are `GetConversationComponent_Derived` =
   `return *(UDisConversationComponent**)((BYTE*)this + 8)` — `m_pConvComponent` at 592, off the
   `IDisConvSpeakerInterface` sub-object at 584 — and an empty `GameLoad_Dialog_Derived`. Both were read out of
   retail's vtable at rva `0xd1a200`, the table whose slot 0 is the `adjustor{584}` thunk; the demangled name
   on that table says `{for IDisStoryGroupInterface}`, which is a name-propagation artefact, and the adjustor
   is what identifies it.

And one fact that is not a difference so much as a correction to how every address in this area must be read:
**retail's five save virtuals are vtable slots 67..71**, not the 2012 build's 66..70, because retail inserted
one virtual ahead of them (`sub_46F370`, before `UObject::ScriptConsoleExec`). Agent EC noticed this for
`URB_BodyInstance` and stated it as a fact about that class; it is a fact about `UObject`.

## 3. The measurement

`build/agentEF_release`, `-disrestoreslot=16` and `-disrestoreslot=17`, null RHI, no `-noscenerender`.
`build/agentEF/agentEF_base.log` is HEAD `79ead3d`; `agentEF_final16.log` and `agentEF_slot17d.log` are the
final state.

| | HEAD `79ead3d` (= agent EC) | agent EF |
|---|---:|---:|
| **Dishonored0.sav ("0 - Dunwall Tower")** | | |
| dictionary records read | 11321/11324 | **11321/11324** |
| dictionary bytes | 91844/91844 | **91844/91844** |
| records resolved | 6227 | **6227** |
| objects restored | 57 | **305** |
| object-stream bytes | 4141/619631 | **23827/619631** |
| stopped on | `DishonoredPlayerPawn.PowerBlink` (`DishonoredActivePowerComponent`) | **`DishonoredNPCController_0.AIBrain` (`DishonoredAIBrain`)** |
| **Dishonored1.sav ("1 - Dunwall Sewers")** | | |
| dictionary records read | 6433/6436 | **6433/6436** |
| objects restored | 41 | **180** |
| object-stream bytes | 2716/294961 | **17842/294961** |
| stopped on | unported `GameLoad` | **`DisLoadPhysicsAssetInstanceBodies`, 18 rigid bodies, no `URB_BodyInstance::GameLoad`** |
| ported override classes | 37 | **47** (unported 75 → 65) |

The census line in full, slot 16:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 305 restored, 432 skipped, 23827/619631 bytes; 50 spawned, 5093 not found, 1 unported, 0 unresolved,
202 untrusted skips, 0 partial bodies, 305 PostGameLoad; STREAM ABORTED
```

### 3.1 Accept 1, measured against the save

The restore now reports the session as well as the position:

```
DisRestore: player pawn DishonoredPlayerPawn at X=9826.257 Y=23064.211 Z=2412.241 rotation P=0 Y=-513 R=0
DisRestore: session: health 70 (min scripted 0), mana 100 of 0 (regen 0, countdown -131.767), adrenaline 0.000
DisRestore: inventory pInventory: 0 slot(s), 12 ammo type(s) [0/0/0/0/0/0/0/0/0/0/0/0], elixirs 0/0, 0 abstract item(s)
DisRestore: powers: 6 active, 0 upgrade(s) (0 backed up), 0 key(s), darkness 0, 0 charm(s)
```

```
DisRestore: player pawn DishonoredPlayerPawn at X=7241.508 Y=5174.333 Z=3719.230 rotation P=0 Y=-58720 R=0
DisRestore: session: health 70 (min scripted 0), mana 100 of 0 (regen 0, countdown -1597.762), adrenaline 0.000
DisRestore: inventory pInventory: 0 slot(s), 12 ammo type(s) [4/0/8/3/2/0/0/0/0/0/0/0], elixirs 5/0, 6 abstract item(s)
DisRestore: powers: 6 active, 0 upgrade(s) (0 backed up), 4 key(s), darkness 12, 0 charm(s)
```

`build/agentEF/verify_session.py` decodes the save files in Python, without the game and without the C++
reader, and finds exactly those values:

```
Dishonored0.sav / DishonoredGameFull_P: 619631 bytes of object data
Health 70 followed by the m_ActivePowers count 6:
  byte  4131  raw 46 00 00 00 06 00 00 00        1 match in the first 20000 bytes
the mana triple (m_Mana=100, m_ManaRegenAmount=0, m_fManaRegenCountDown=-131.767):
  byte 14390  raw 64 00 00 00 00 00 00 00 64 c4 03 c3      1 match in the whole blob
the inventory loadout (FDisInventoryLoadout, binary and untagged):
  byte  4241  m_Items    3  -> dictionary records [196, 196, 199]
  byte  4251  m_Ammo     12 entries, every count 0
  byte  4315  m_ElixirCounts [0, 0]        byte 4323  m_AbstractItems 0
  byte  4327  default equip types -> dictionary records 59 and 59

Dishonored1.sav / DishonoredGameFull_P: 294961 bytes of object data
Health 70 followed by the m_ActivePowers count 6:   byte 2706, 1 match
the mana triple (100, 0, -1597.762):                byte 4431, 1 match in the whole blob
  byte  2816  m_Items    5  -> dictionary records [93, 93, 96, 99, 101]
  byte  2830  m_Ammo     12 entries: (0,4) (1,0) (2,8) (3,3) (4,2) and seven zeros
  byte  2894  m_ElixirCounts [5, 0]        byte 2902  m_AbstractItems 6
  byte  2906  default equip types -> dictionary records 105 and 1
```

Health is at byte 4131 of `Dishonored0.sav` and 2706 of `Dishonored1.sav`, in each case immediately followed by
the active-power count, and in each case the only such pair in the first 20,000 bytes. The mana triple has
exactly one match in each whole blob. The loadout decodes in place, and the numbers the running game reports —
`4/0/8/3/2`, `5/0`, `6` — are the loadout's own.

`Dishonored1.sav`'s key ring and darkness score decode the same way, from the offsets `-dispropertytrace`
names for the `DisKeyInfo` walk:

```
byte 15400  WORD 391   -> DishonoredPlayerPawn.pKeyRing
byte 15402  INT  4     m_Keys
    key 0  "Corvo's Cell Key"        tweaks record 395
    key 1  "Yard Walkway Key"        tweaks record 396
    key 2  "Yard Key"                tweaks record 396
    key 3  "Dunwall Sewer Gate Key"  tweaks record 395
byte 15578  INT  0     m_bBackupIsValid
byte 15582  WORD 398   -> DishonoredPlayerPawn.pDarknessMgr
byte 15584  INT  12    m_DarknessScore
byte 15588  INT  9     m_DeedCounters
```

`-131.767` is minus that save's world clock, and `-1597.762` is minus the other's: retail writes
`m_fManaRegenCountDown` relative to the clock, which is the same shape agent EC found for `USequenceEvent`'s
activation time. It is not a coincidence and it is not a misread — those twelve bytes are one match in
619,631, and the two flanking values are the mana and the regen amount.

The byte offsets came from `-disstreamdebug=400` and `-dispropertytrace` (agent EC's two diagnostics): the
player pawn's `FDisConvSaveData` walk begins at byte 14406, `GameLoad_Inventory` is the four bytes before it,
and the mana triple is the twelve before that — 14390. The decode above then reads the file at that offset
without using the reader at all.

### 3.2 What "the inventory is restored" means, exactly

`UDishonoredInventory::GameLoad` reads an `FDisInventoryLoadout` and retail hands it to
`ADishonoredPawn::SpawnInventoryLoadout` (`0x7689c0`). Agent ED's port read the struct and threw it away,
because that function lives in `dishonoredpawn_inventory.cpp`, which is still an `import_reference.py` stub.
Two of its four parts need nothing this tree lacks, so they are ported here: **the ammo**
(`SpawnInventoryLoadout_Ammo`, `0x751a50` — all twelve types cleared, then the loadout's entries set) and
**the abstract items and the two elixir counts**. The **items** half (`SpawnInventoryLoadout_Items`,
`0x7c8690`) and the two default equip types are not: both need the item factory, so
`m_Slots` stays empty and the diagnostic says `0 slot(s)`.

Getting the elixirs to land needed one more thing, and it is worth naming because it was dead code until now:
`UDishonoredInventory::m_pOwner` is set by `ADishonoredPawn::PreBeginPlay_Inventory` (`0x7c38a0`), which this
tree does not have, so **nothing in this build had ever set it** — and both `SetElixirCount` and `AddElixir`
answer 0 without it. `GameLoad` now resolves the owner from the component's outer, which is the pawn (it is
how the save's dictionary names it: `DishonoredPlayerPawn.pInventory`), and says so in a `DISHONORED(bringup)`
comment.

## 4. The frontier now

The stream stops at `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredNPCController_<n>.AIBrain`, class
`UDishonoredAIBrain`, at byte **23827** of 619631 (the instance number varies with spawn order - two runs of
the same build gave `_0` and `_26` - while the class, the byte count and every census figure do not). Retail `UDishonoredAIBrain::GameLoad` is **`0x7256a0`**
(vtable slot 70; the 2012 body `0x74d790` has no `match_2012_2013.csv` row). It reads the object's own
properties behind `Ar.Ver() >= 22`, then a counted list of stimuli through `UDisStimManager::LoadStim`, each
with an INT beside it.

`build/agentEF/frontier_dish0_final.txt` lists every record of `Dishonored0.sav`'s persistent level with its
body and its state. **Seventeen** override bodies are still missing, down from 27, and the first seven of them
are one package — the AI stack:

| record | class | retail `GameLoad` | retail bytes |
|---:|---|---|---|
| 716 | `UDishonoredAIBrain` | `0x7256a0` (`PostGameLoad` `0x711c20`) | 490 |
| 730 | `UDisAIBrainProcess` | `0x7362c0` (`GameSave` `0x736290`) | 41 |
| 732 | `UDisAttentionInfo_Complex` | `0x74ff80` | 121 |
| 737 | `UDishonoredAIBehavior` | `0x6f6f80` (`PostGameLoad` `0x6e8250`) | 162 |
| 739 | `UDishonoredNativeStateMachine` | `0x67a9e0` | 31 |
| 741 | `UDisAISubState` | `0x7126c0` (`PostGameLoad` `0x705e10`) | 86 |
| 751 | `UDisAISubProcess` | `0x73d580` (`PostGameLoad` `0x72ae40`) | 86 |

then `UArkComponentContainer` (1066, `0x534220`), `UDisConv_Soiree_InGameData` (1814), `USeqAct_Interp` (1817),
`ADishonoredRoute` (3307), `ADishonoredUsableObject` (8019), `ADisRatSpawner` (8033),
`ANavMeshBlockToggleable` (8115), `ADisStatPickup` (8142), `UDisSeqAct_NPCTrackTarget` (9054) and
`UDisConvGlobalMan` (11323). On `Dishonored1.sav` the stop is elsewhere and is agent EC's own gate:
`DisLoadPhysicsAssetInstanceBodies` with 18 rigid-body states and no `URB_BodyInstance::GameLoad`
(2012 `0x3ca2f0` / `0x3c0f00`, retail `0x3a8620` / `0x39e7f0`).

**A fully restored session is not far.** The stream is in step for 23,827 bytes and stops on a named class
rather than on a bogus count; what stands between here and the end of the blob is those seventeen bodies plus
`URB_BodyInstance`, and seven of the seventeen are one connected area.

## 5. Which override classes are ported and which are not

`DishonoredGame/Inc/dissaveload_classlists.h`, regenerated by `build/agentEF/gen_classlists.py`:
**562 entry classes, 47 ported, 65 unported.**

The 47, by where their bodies live:

* `Engine/Src/UnSequence.cpp` — `USequenceObject`, `USequenceOp`, `USequence`, `USequenceEvent`,
  `USeqAct_Latent`
* `Engine/Src/UnActor.cpp`, `UnWorld.cpp`, `UnParticleComponents.cpp`, `UnSceneCapture.cpp`, `UnLight.cpp`,
  `UnPhysActor.cpp` — `AActor`, `ATrigger`, `ATargetPoint`, `AInterpActor`, `AWorldInfo`, `AEmitter`,
  `ASceneCaptureActor`, `ALight`, `AKActor`
* `DishonoredGame/Src/dissavegame.cpp` — `UDishonoredMapInfo`, `UDisGlobalFactionManager`,
  `UDishonoredInventory`, `UDisAttributes`, `UDisDialogTree_InGameBind`, `UDisAttentionInfo_Base`,
  `UDishonoredGlobalAIManager`, `UDisAIBlackboard`, `UDishonoredObjectivesComponent`, `UDishonoredObjective`,
  `UDishonoredTask_Base`, `UDishonoredPowersComponent`, `ADishonoredPawn`, `ADishonoredPlayerPawn`,
  `ADishonoredGameInfo`, `ADishonoredSpawner`, `UDisNPCTravelManager`, `UDisGlobalUIManager`,
  `UDisGFxMoviePlayerHUD`, `UDisGFxMoviePlayerPowerWheel`, `UDisPostProcessManager`,
  `ADishonoredPlayerController`, `ADishonoredNPCPawn`, and — **new in this package** —
  `UDishonoredActivePowerComponent`, `UDishonoredActivePowerComponent_BendTime`,
  `UDishonoredActivePowerComponent_DevouringSwarm`, `UDishonoredActivePowerComponent_Possess`,
  `UDisActivePowerComponent_DarkVision`, `UDisKeyRing`, `UDisDarknessManager`,
  `UDishonoredVisibilityComponent`, `ADisPlayerAudioLogDummyActor`, `ADishonoredNPCController`

Plus the three roots of `UDisAttentionInfo_Base`'s ICF fold that agent EC declared
(`UDisConv_Node_InGameData`, `UDisHideoutComponent`, `UDisSteeringInfluence`), through which sixteen more
classes reach a ported body, and `_Blink` and `_WindBlast`, which inherit the base power component's.

Still not ported: the 65 in `GDisUnportedGameLoadClasses`. The whole writing half (`FLevelSaver` `0x615730`,
`FGameState::SaveLevel` `0x613cb0`, `SaveGameState` `0x602920`, `ProcessSaveLoadCmd` `0x6162d0`) remains out,
as agents ED, EB and EC left it, and every `GameSave` this package met that retail does not fold onto its
`GameLoad` is left undeclared for the same reason.

## 6. The ICF-fold check is now a step of the generator (agent EC's hand-over 5)

`build/agentEF/gen_classlists.py` writes `build/agentEF/folds.txt` on every run. For every ported body it
lists the classes retail's linker folded onto the same address that are **not** covered — covered meaning one
of the fold's ported owners is in their class chain, or one of the roots this tree declares the body on by
hand, or they own a body of their own (in which case they are in the unported list under their own name and
the gate stops on them anyway). It reports the slot, because slot 68 is `GameSave` and this tree has no writer.

At this HEAD it reports **0**. A check that reports zero has to be shown capable of reporting something, so
`build/agentEF/make_foldselftest.py` rewrites the generator with agent EC's three hand-declared fold
roots removed and runs it — the state of
the tree before agent EC — and it reproduces agent EC's defect exactly: `DisAttentionInfo_Base`, slot 69,
**16 classes**.

Two reports it raises on the way to zero are worth recording, because both look alarming and neither is:

* `DisAttentionInfo_Base`'s **slot 68** fold is shared by 46 AI behaviour and state-machine classes. They all
  own a `GameLoad` of their own and are in the unported list, so the gate stops on them; and `GameSave` is
  never called here.
* `DisPlayerAudioLogDummyActor`'s slot 69 fold is shared with `DisDialogInanimateDummy` and
  `DisSpeakerGroup_PA` **in the 2012 build**. In retail they are a different body (`0x643cb0`, shared with
  `ADisAudioLogPlayer`), and `DisDialogInanimateDummy` is already in the unported list under its own name. The
  2012-derived list was right by accident and the check says why.

## 7. Deviations, stated plainly

1. `UDishonoredActivePowerComponent_BendTime::PostGameLoad` and `_DevouringSwarm::GameLoad` convert a saved
   elapsed time back into an absolute stamp against retail's bend-time clock. **This tree has no bend-time
   clock**, so `AWorldInfo::TimeSeconds` — the clock `AWorldInfo::GameLoad` restores — stands in. Neither
   reads a stream byte differently; the restored timer is off by whatever bend time the save had accumulated.
2. The same stand-in is used for the 80 achievement trackers' streak time stamps.
3. `_BendTime::GameLoad`'s "power was running" branch (`GetSettings` and the dilation re-entry),
   `_DevouringSwarm::GameLoad`'s spawn-point re-aim and Ak events, and `DarkVision::GameLoad`'s post-process
   and sky-light re-arm are read past, not called. None reads a stream byte.
4. `GameLoad_Inventory`'s three values are read and not acted on: retail sets the current power through
   `UDisItemPowers::SetCurrentPower`, the ammo type on the equipped ranged weapon and the charge count on the
   equipped item, and none of those three is ported.
5. `GameLoad_Stealth` reads both stealth bits and applies only `m_bSneakModeToggled`; retail also replays
   `APawn::Crouch` for `m_bIsAutoCrouched` and carries two timers forward. No stream bytes.
6. The crouch byte in `GameLoad` sets `bWantsToCrouch` without calling `APawn::Crouch(FALSE)` first.
7. The adrenaline value is assigned rather than added through `AddAdrenaline` (`0x6ac510`), which clamps it
   against the tweaks and notifies the HUD.
8. `SpawnInventoryLoadout` ports the ammo, abstract items and elixir counts and not the items or the default
   equip types (section 3.2).
9. `UDishonoredInventory::GameLoad` sets `m_pOwner` from the component's outer when it is NULL, because
   `PreBeginPlay_Inventory` is not ported and nothing else in this tree ever sets it (section 3.2).
10. `ADisPlayerAudioLogDummyActor::GameLoad` stops the stream when the actor has no attached conversation
    component, for the same reason agent EC's `ADishonoredNPCPawn::GameLoad_Dialog` does: retail's other
    branch serialises the component's own `FDisConvSaveData`, which there would be nothing to serialise into.
11. `ADishonoredNPCController::GameLoad` does not call `AController::Possess( Pawn )` or
    `UArkComponentContainer::StartAllComponents`. Neither reads a stream byte, and possessing there would
    re-run the whole controller bring-up on a pawn the restore has not finished with.
12. `SpawnInventoryLoadout_Ammo` bounds its loop by `m_AmmoInfo.Num()` where retail does not: retail's
    inventory always has `eDisAmmoType_MAX` entries from the class default, and a tree with a partial class
    default would assert inside `TArray::operator()` instead, which says nothing about which body was reading.
    Every inventory in both saves has the twelve entries, so the bound never fires.
13. Every `GameSave` this package met is ported only where retail folds it onto the `GameLoad` this tree needs
    (`UDishonoredActivePowerComponent`, `UDisDarknessManager`, `UDishonoredVisibilityComponent`,
    `UDisActivePowerComponent_DarkVision`, `_BendTime`, `_DevouringSwarm`). The rest are declared nowhere, for
    agent ED's reason: the writing half of the object layer does not exist here.

## 8. Verification

* **Accept 1** — health, mana and inventory against the save's own bytes: section 3.1,
  `build/agentEF/verify_session.py`, `build/agentEF/agentEF_final16.log` and `agentEF_slot17d.log`.
* **Accept 2** — the census against agent EC's figures and the new stopping point with its address:
  sections 3 and 4.
* **Accept 3** — the override list: section 5.
* **Accept 4** — `python resources/tools/run_regression.py --build-dir build/agentEF_release --no-build
  --exe-name DishonoredGame_EF.exe --log-prefix agentEF_reg`: **31 ok, 0 failed, 0 skipped, 449 s**
  (`build/agentEF_release/regression/summary.txt`; `d3d9_frames` 2850, `d3d9_draw_elements` 6506,
  `inputtest_moved` 1028.8, `layout_mismatching` 0, `layout_contract` 0, `verify_phase2` 2/2). Clean full release build of `DishonoredGame`, `CoreSmoke`
  and `LayoutProbe` from an empty `build/agentEF_clean`, **0 errors** (`build/agentEF_cleanbuild.log`,
  `build/agentEF_clean_build.cmd`).
  **A first regression run reported 4 d3d9 failures, all reading 0**: it was launched while two restore runs
  were still staging the same `DishonoredGame_EF.exe` into the same retail install. Nothing was running for
  the run above. The coordinator's open item on making `d3d9_frames` load-aware is exactly this.
* **Layouts** — unchanged. This package adds no reflected member and changes no class size; the one Core
  change is a member function template on `TScriptInterface`.
* **Addresses** — `python resources/tools/rva_sweep.py --csv build/agentEF/rva_sweep.csv`: of the **385**
  citations in the files this package touched, **0 mislabelled and 0 unknown** (`build/agentEF/sweep_mine.py`),
  and all 26 of the addresses this package cites in source resolve to `ok-2013`, a retail function start. The three
  `MISLABELLED-2012` the whole-tree sweep reports are pre-existing and in other agents' files
  (`CppText/UDisAISubState.h:16`, which names both builds in the same comment, and two in
  `GFxUI/Src/gfxuirenderer.cpp:1669`, a block explicitly labelled "2012").
* **The dictionary halves are still exact**: 11321/11324 records and 91844/91844 bytes on slot 16,
  6433/6436 and 52672/52672 on slot 17, in every run.
* Everything was built from `build/agentEF_wt`, detached at `79ead3d`, with only this package's own files
  copied in (`build/agentEF_sync.py`).

## 9. Hand-overs

1. **Coordinator, at merge.** This package changes **two files outside `DishonoredGame`** and one of them is
   load-bearing:
   * `Core/Inc/ScriptInterface.h` — `TScriptInterface<T>::SerializeForSaveLoad`, retail's own member at
     `Core/Inc/ScriptInterface.h:205` (2013 `0x7eb500`). It is a new member function template; nothing calls
     it but `UDishonoredActivePowerComponent_Possess::GameLoad`, and it changes nothing that existed.
   * `DishonoredGame/Src/dishonoredengine.cpp` — the restore diagnostic now reports the session (health, the
     mana triple, the adrenaline, the inventory, the powers, the keys, the darkness score). Diagnostic only.

   **Regeneration is required**: this package adds `Inc/CppText` hooks for
   `UDishonoredActivePowerComponent`, `UDishonoredActivePowerComponent_BendTime`,
   `UDishonoredActivePowerComponent_DevouringSwarm`, `UDishonoredActivePowerComponent_Possess`, `UDisKeyRing`,
   `UDisDarknessManager`, `UDishonoredVisibilityComponent` and `ADisPlayerAudioLogDummyActor`, and appends to
   `UDisActivePowerComponent_DarkVision`, `ADishonoredPlayerPawn`, `ADishonoredPawn` and
   `ADishonoredNPCController`, so
   `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`
   must run at merge — **agent EF did not regenerate.** The eight new `#include "CppText/<Class>.h"` lines were
   inserted by hand at the place the generator emits them (the last line of the class body, after
   `StaticConfigName()`). `Sources.cmake` does not change: no new source file.
   `dissaveload_classlists.h` is regenerated by `build/agentEF/gen_classlists.py`; it reads 562 entry classes,
   47 ported, 65 unported.

   The twenty source files, for the merge - twelve modified and eight new and therefore untracked:
   `Core/Inc/ScriptInterface.h`, `DishonoredGame/Src/dissavegame.cpp`,
   `DishonoredGame/Src/dishonoredengine.cpp`, `DishonoredGame/Inc/dissaveload_classlists.h`,
   `DishonoredGame/Inc/dishonoredgameclasses.h`, `DishonoredGame/Inc/DishonoredGamePowerClasses.h`,
   `DishonoredGame/Inc/DishonoredGameDarknessClasses.h`,
   `DishonoredGame/Inc/DishonoredGameVisibilityClasses.h`,
   `Inc/CppText/{ADishonoredPawn,ADishonoredPlayerPawn,ADishonoredNPCController,UDisActivePowerComponent_DarkVision}.h`
   and the eight new
   `Inc/CppText/{UDishonoredActivePowerComponent,UDishonoredActivePowerComponent_BendTime,UDishonoredActivePowerComponent_DevouringSwarm,UDishonoredActivePowerComponent_Possess,UDisKeyRing,UDisDarknessManager,UDishonoredVisibilityComponent,ADisPlayerAudioLogDummyActor}.h`.
   `build/agentEF_sync.py` is the authoritative list.

2. **The next package is the AI stack behind `UDishonoredAIBrain`.** Section 4 has the seven classes, their
   retail addresses and their sizes; every one of them is between 31 and 490 bytes, and they are consecutive
   in the save's dictionary (records 716..751). The loop is unchanged and is fast: add a body, add the name to
   `build/agentEF/gen_classlists.py`'s `PORTED`, rerun it, rerun `-disrestoreslot=16`, read
   `data N restored, X/619631 bytes` — and when the byte count stops adding up, rerun with
   `-disstreamdebug=400 -dispropertytrace` and decode the save at the offsets it prints. The one thing
   `UDishonoredAIBrain::GameLoad` needs that this tree does not have is `UDisStimManager::LoadStim`.

3. **A plausible stopping point is not proof, and this is the case that shows it.** My first
   `GameLoad_Dialog` left out `GameLoad_Dialog_Derived` (2013 `0x6b2fe0`), about 100 bytes. The stream did not
   hit a count guard: it read on and stopped at `ADisPlayerAudioLogDummyActor`, which is precisely the object
   the first line of the missing tail names, and every gate and census counter said the restore was healthy.
   What caught it was reading retail's vtable for the class rather than trusting that a body ending at a
   sensible-looking frontier was complete. **When a body's last statement is a virtual call, resolve that slot
   in retail before believing the body is finished.**

4. **`TArray::operator<<` placement-news each element into uninitialised memory.** A struct with an `FString`
   or a `TArray` in it must be zeroed **raw** before a binary walk writes into it, which is exactly what
   retail does (`operator<<(FArchive&, FDisKeyInfo&)`, 2013 `0x6c94a0`, seven DWORD stores). Assigning
   `FString()` instead frees a garbage pointer. `Dishonored1.sav` crashed on it and said nothing but
   "Fatal error!".

5. **Agent EC's hand-over 4 — the generated-header hazards — is NOT cheap, and here is why, with the fix
   located.** `ADishonoredPawn::m_LatentInteractables` is declared `TArrayNoInit<class UDisInteractableInterface*>`
   (4-byte elements) where the reflected property is an `ArrayProperty` of `InterfaceProperty` (8-byte
   elements), and `m_ActivePowers` is `BYTE[12]` where it is an `ArrayProperty` of `ComponentProperty`.
   `resources/docs/types/script_classes_2013.json` carries **both inner types exactly**
   (`DishonoredGame.DisInteractableInterface` and `DishonoredGame.DishonoredActivePowerComponent`), and
   `gen_classes_header.py` already maps `InterfaceProperty` to `struct FScriptInterface` (`prop_type`, line
   ~1023) and sizes it 8 (`prop_size_align`, line ~1039). The two places to change are
   `gen_classes_header.py:1658` — the single-candidate gap branch that emits `BYTE <name>[gap]` where
   `prop_type(p)` would answer — and the SDK type conversion at `gen_classes_header.py:1177`, which turns the
   dump's `TArray<class UX*>` into `TArrayNoInit<class UX*>` without consulting the package's inner kind.
   It is a two-line change to a generator whose output feeds 12,502 layout asserts across 764 units, and its
   result cannot be verified inside a package that is forbidden to regenerate. It wants its own package, in
   which the generator is run over the whole tree and the diff read.

6. **`ADishonoredPawn::SpawnInventoryLoadout_Items` (2013 `0x7c8690`) is what stands between the restored
   loadout and a restored inventory the player can see.** The ammo, elixirs and abstract items are back; the
   items are not, so `m_Slots` is empty and nothing is equipped. It belongs with whoever owns the item
   factory, not with the save layer, and `dishonoredpawn_inventory.cpp` — retail's own unit for it, and for
   `PreBeginPlay_Inventory`, which is why `m_pOwner` was never set — is still an `import_reference.py` stub.

7. **`URB_BodyInstance::GameSave` / `GameLoad`** (2012 `0x3ca2f0` / `0x3c0f00`, **retail `0x3a8620` /
   `0x39e7f0`**) is now the whole distance between `Dishonored1.sav` and the rest of its blob: the restore
   stops there with 18 rigid-body states to read. Retail's `GameLoad` is vtable **slot 70**, as agent EC said,
   and section 2 explains why: it is slot 70 for every class.

8. **`ADishonoredPlayerPawn::GameLoad` leaves `m_AchievementTrackers[0]` out of the stream**, because retail
   does: the loop runs indices 1..Count-1 in all three version branches. If a future save is found whose
   element 0 carries a streak, that is the place to look, but the count matches the array size exactly (81 at
   version 24, and the array is `FAchievementTracker[81]`) so the asymmetry is retail's own.

9. **`ADishonoredPlayerPawn::GameLoad` reads one byte and discards it** (between the crouch byte and
   `m_StatValues`). Retail's body reads it into a stack local and never uses it. It is in the stream in both
   builds, and both this port and retail skip it.
