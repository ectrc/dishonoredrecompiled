# Agent EC (PHASE11 EC) — the pawn `GameLoad` bodies, and milestone 7: a real save puts the player back

Branched from the merged HEAD `fc1b24f`. Own worktree `build/agentEC_wt`, own build dirs
`build/agentEC_release` and `build/agentEC_clean`, own IDA copies `build/agentEC_ida/retail2013_agentEC.i64`
and `shipping2012_agentEC.i64`. Nothing committed, nothing staged.

## The answer in eight lines

* **Milestone 7 is met.** `Dishonored0.sav` ("0 - Dunwall Tower") restores the player pawn to
  `X=9826.257 Y=23064.211 Z=2412.241 rotation P=0 Y=-513 R=0`, and those are the save's own bytes: the
  `FVector` at offset **4041** of the level state's object-data blob is `07 89 19 46 6c 30 b4 46 da c3 16 45`
  and the `FRotator` at 4053 is `0 / -513 / 0`. Exact, to the float. The spawn transform it replaces was
  `X=0.000 Y=500.000 Z=-209.350`.
* The game then runs from that state: **600 s and 111,840 scene frames** under the null RHI with 0 criticals,
  and a d3d9 run at 1280x720 renders the Dunwall Tower gazebo from the restored position
  (`build/agentEC/agentEC_restored.png`, `-apshottime=40`).
* Both bodies the brief named are ported in full — `ADishonoredPawn::GameLoad` (retail `0x75c0a0`) and
  `ADishonoredNPCPawn::GameLoad` (`0x76d3f0`) — with eleven helpers, and `ADishonoredNPCController::GameLoad`
  at `0x763570` is **confirmed**, not a candidate.
* The two bodies were not what stood between this tree and the player. **Seven defects in the existing save
  layer were**, and six of them were invisible until the stream got past record 21. The largest is a property
  flag: retail skips any property carrying `0x0002000000000000` when the archive is a save-game archive, and
  this tree had no such flag, so *every* binary property walk in the save read far more bytes than retail
  wrote.
* The measured result: object-stream bytes **74 → 4141**, objects restored **4 → 57**, and the stream's
  stopping point moved from dictionary record 21 (Lady Emily) past record 162 (the player pawn) to record
  **164**'s sub-object.
* The new stopping point is an honest override frontier again:
  `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredPlayerPawn.PowerBlink`, class
  `UDishonoredActivePowerComponent`, whose `GameLoad` is one of the 75 retail overrides this tree has not
  ported.
* Regression **31 ok, 0 failed, 0 skipped, 423 s**; clean full release build of all three targets, 0 errors;
  `rva_sweep` over this package's files: **324 citations, 0 mislabelled, 0 unknown**.
* Two new diagnostics carry the package: `-disstreamdebug=<n>` (every object reference the data stream reads,
  with the byte offset it was read at) and `-dispropertytrace` (every property a DisSaveLoad binary walk
  reads, with the same). Six of the seven defects were found with them in one run each.

## 1. The two pawn bodies

`dissavegame.cpp`, beside the other ported override bodies, because retail's own units for them are still
`import_reference.py` stubs.

### 1.1 `ADishonoredPawn::GameLoad` — retail `0x75c0a0`, in full

Agent ED had ported it as far as `AActor::GameLoad` and then stopped the stream. The whole body is

| # | reads |
|---|---|
| 1 | `AActor::GameLoad` — where `Location` and `Rotation` come back |
| 2 | `Health` |
| 3 | a count, then that many object references into `m_ActivePowers`, emptied first |
| 4 | a count, then that many into `m_Attachments`, emptied and zeroed first |
| 5 | `m_pInventory` |
| 6 | **`Ar.Ver() >= 17`**: a count and that many into `m_AssociatedActors` |
| 7 | a count, then that many into `m_LatentInteractables` — **appended**, not emptied — each with its cached interface address; then every entry whose object or interface is NULL is pruned |
| 8 | `m_pAttributes`, `m_MinimumScriptedHealth`, `m_PersonalRelationships` |
| 9 | one packed byte: bit 0 `m_bIsControlledByAMatinee`, bit 1 `m_bSpecialRootMotionExtract` |
| 10 | `m_BackupedLocalToWorld`, `m_BackupedMeshLocalToWorld`, `m_vBackupedPosition`, `m_rBackupedRotation` |

**Where the 2012 build is not retail, retail is what is ported.** 2012's body (`0x79b360`, 964 bytes against
retail's 1928) stops after `m_PersonalRelationships`: rows 6, 7's prune, 9 and 10 are retail-only, and the
2012 class has no `m_bIsControlledByAMatinee` or `m_bSpecialRootMotionExtract` at all.

Two layout notes, both about the generated header rather than about this body:

* `m_ActivePowers` is reflected as an `ArrayProperty` of `ComponentProperty` and the CodeRed dump did not
  type the inner property, so the generator emits `BYTE m_ActivePowers[12]`. Those are the twelve bytes of a
  `TArray` of object pointers and the body reads them as one.
* `m_LatentInteractables` is an `ArrayProperty` of **`InterfaceProperty`**, so the live array's stride is 8 —
  the object and its cached interface address — and that is what the garbage collector walks. The generated
  declaration types the element as a bare pointer, because that is how the dump prints an interface array;
  the 2012 PDB spells it `TArrayNoInit<TScriptInterface<IDisInteractableInterface> >`. The body reads through
  the reflected stride. **Hand-over: the declaration is four bytes per element short of the reflected
  property, which is a live GC hazard independent of the save.**

### 1.2 `ADishonoredNPCPawn::GameLoad` — retail `0x76d3f0`

The whole body, in order: `m_iRandomHeadMeshSel`; the head and body material overrides; two accessory
selections; **`>= 23`** `m_MovableLimbs`; `m_fLastTeleportTime`; `ADishonoredPawn::GameLoad`; `m_NPCID`;
`m_TripID`; `m_NPCDeathInfo`; `FDisSpawnerInfo` as a script struct; one bit into
`m_PostGameLoadParams.m_bIsAsleep`; then a three-way branch on `Health` and the spawner's `m_bSpawnDead` /
`m_bTreatAsKnockedOut`; the master FSM's partial state; the faction and story-group overrides; a packed byte
of four bits; the dialog; the possession references; `m_fTimeBeforeSleep` and, when positive,
`FDisSleepDamageInfo`; the plague byte; `m_pStealable` and `m_pDroppedStealable`; **`>= 22`**
`m_bExpectingPutpocket`, `m_pMarkedForVanishAction` and its timer; **`>= 23`** `FDisNPCDamageInfo` and
`m_bShadowKilled`.

Retail against 2012 again: 2012 (`0x7c7c40`, 1013 bytes to retail's 1488) has none of the three version-gated
blocks, none of `m_pStealable` / `m_pDroppedStealable`, and **three** bits in the packed byte where retail has
four. The fourth is `m_bNotifiedFakeDeath`, which the 2012 class does not have; the first three are the same
bits in both builds (`m_bIsDramaAssassinationHandled`, `m_bNotifyAIOfRelationshipChange`,
`m_bDisableTeleportOnNavmesh`, resolved by name against the 2012 PDB's bit list and this tree's retail
header). A real retail save is version 24, so every gate fires.

`ADishonoredNPCPawn::IsSaveable` (retail `0x74aa80`, **byte-identical** to 2012's `0x7ab370`) had to come with
it — `Location == SLL_FILE || bKillDuringLevelTransition` — because without it `ShouldLoadObject` answers
FALSE for the very object the stream stops on.

### 1.3 The eleven helpers

| helper | retail | note |
|---|---|---|
| `operator<<( FArchive&, FDisMaterialReplacement& )` | `0x76fd50` | the default material, a flag, and either the custom material or — when it is a `UMaterialInstanceConstant` in the transient package, which a save cannot name — that instance's `Parent`, rebuilt into a fresh transient instance on load |
| `operator<<( FArchive&, TArray<FDisMaterialReplacement>& )` | `0x75b400` | the generic UE3 array serializer, reached through `FDisMaterialsOverride::m_MaterialReplacements` |
| `operator<<( FArchive&, FDisNPCDeathInfo& )` | `0x770770` | culprit, cause of death, damage causer, one awareness byte, instigator, then four bits in one byte. Neither `m_DamageSourceLocation` nor `m_NameOfDeadNPC` is in the stream |
| `UDishonoredNativeStateMachine::LoadPartialState` | `0x672670` | one object reference (the state class), looked up in `m_NativeStateMap`, then that state's own `LoadPartialState` |
| `UDisConversationComponent::SerializeForGameLoad` | `0x896600` | the component's script properties, then which of the bound dialog tree's two running instances was live |
| `ADishonoredNPCPawn::GameLoad_Dialog` | `0x897060` | retail has this on `IDisConvSpeakerInterface`; this tree's interface carries only its vptr, so it sits on the one class that needs it, with retail's two answers for that class resolved by name |
| `ADishonoredNPCPawn::GameLoad_Possession` | `0x771340` | two script-override references and, behind `>= 18`, `m_InitialPossessedRot` |
| `DisLoadPhysicsAssetInstanceBodies` | `0x7ec790` | the BYTE count is read; a non-zero count stops the stream, because `URB_BodyInstance::GameLoad` is not ported |
| `DisSerializeScriptStructBin` | `0x74f330` / `0x74f3c0` / `0x74f450` | retail's `DishonoredGetScriptStruct<T>`, one cached `StaticFindObjectChecked<UScriptStruct>` per call site, then `UStruct::SerializeBin` |
| `DisReadStreamCount` | — | written. Retail hands an element count straight to `TArray::Empty`, which asserts `Count>=0`; a partial override set can reach that assert and a crash says nothing about which body lost its place |
| `DisStopRestore` | — | written. The one way a ported body says "this branch is not here" and stops the stream by name |

### 1.4 The three branches of the NPC pawn that are read and not acted on

None of them touches the stream, so none can desynchronise it.

* `ADishonoredNPCPawn::RestoreAppearance` (`0x7834c0`) is not called, so the restored head-mesh selection,
  material overrides and accessories are read and not put back on the components.
* `StartRagdolling` and `SwitchToEatenMesh` on the corpse branches are not called.
* `SeverLimb` (`0x76cc10`) is not called: the dead-NPC branch reads its `TArray<FName>` of joint names and
  severs nothing.
* `ADishonoredNPCPawn::PostGameLoad` (`0x75e290`) is not ported. It reads no stream bytes, so the empty
  `UObject` body is safe; what it costs is that a restored corpse does not enter `StateNPCMasterDead_Limp`
  and a restored NPC does not re-grab the movable it was holding.

One branch **does** touch the stream and is gated: `DisLoadPhysicsAssetInstanceBodies` with a non-zero body
count stops the restore, because `URB_BodyInstance::GameSave` / `GameLoad` (2012 `0x3ca2f0` / `0x3c0f00`) are
not ported. It is on the corpse and ragdoll branches only.

And one is a risk rather than a gate: retail reads **one byte** if and only if
`UArkComponentContainer::GetFirstComponent<FDisComponentPlague>` finds a plague component.
`FDisComponentPlague` is an `import_reference.py` stub here, so no NPC has one and the byte is never in our
half of the stream. Retail's writer is symmetric with its reader, so this only matters for a save taken with
a weeper resident.

## 2. The seven defects in the existing save layer

This is the real content of the package. The two pawn bodies took the stream from 74 bytes to 126; the seven
below took it from 126 to 4141 and past the player pawn.

### 2.1 `CPF_DisNoSaveGame` — an Arkane property flag this tree did not have

Retail's `UProperty::ShouldSerializeValue` (`0x2fd0`, 2012 `0x2fe0`) has one disjunct more than this tree's:

```
&& ((PropertyFlags_high & 0x20000) == 0 || !*((DWORD*)Ar + 33))
```

`0x20000` in the high dword is `0x0002000000000000` and `Ar + 132` is `FArchive::ArIsDisSaveLoad`. So a
property carrying that flag is **skipped whenever the archive is a save-game archive**. The cooked packages
set it on hundreds of properties — nineteen of `USequenceOp`'s and `USequenceEvent`'s twenty-six among them —
so without it every binary property walk in the save read far more bytes than retail wrote, and the object
stream could not survive its first Kismet object.

The flag has no name in any of our evidence: property flags are `#define`s, so they are in neither the PDB nor
the cooked packages. `CPF_DisNoSaveGame` is a written name for the behaviour the binary proves.

### 2.2 `PPF_ForceBinarySerialization` on the two load archives

Retail's `FLevelLoader` constructor (`0x613070`) ORs `0x10000` into its own `ArPortFlags` and into the object
dictionary loader's. `0x10000` is `PPF_ForceBinarySerialization`, and it is the first disjunct of
`UByteProperty::SerializeItem` (`0x30830`, byte-identical to 2012's `0x31da0`): without it an **enum**
`ByteProperty` is serialised BY NAME — four bytes through the save's string dictionary — where retail wrote
one raw byte.

### 2.3 `USequenceEvent`'s save pair is four bytes longer than agent ED's port

Retail's `GameSave` (`0x2e7680`) and `GameLoad` (`0x2e7720`) write and read, after the archetype reference,
one FLOAT: the activation time **relative to now**, so that a restored event keeps its re-trigger delay in a
session whose clock started again. The clock it is relative to is the bend-time clock unless
`m_bAlwaysOutOfBendTime` (the bit retail tests at offset 140 mask 0x400).

`Dishonored0.sav`'s first one is **-131.767**, exactly minus the world clock the same save records twelve
bytes earlier — which is how the four bytes were identified before the decompile confirmed them.

### 2.4 A dictionary record whose outer is inside a `UWorld` must be remembered

Retail's `FLevelLoader` registers an `FObjectRef` for every record it did not retry through the package loader
— that is, whenever the outer is NULL, in the transient package, or inside a `UWorld` — and
`operator<<(UObject*&)` constructs the object the moment the data stream names it. Agent ED's port took that
branch only for a NULL outer.

The records this loses are exactly the ones a fresh session cannot have: a Kismet event **duplicate**, which
`USequenceEvent::CheckActivate` makes at run time and hangs on `AActor::GeneratedEvents`. Records 40 and 47 of
`Dishonored0.sav` are two of Lady Emily's, and they stopped the whole restore.

Two written guards go with it, because retail cannot reach either case:

* the outer is resolved **recursively** (`FLevelLoader::ResolveRecord`), because a remembered record's outer
  can be another remembered record — `DisConversation_InGameData_Base_147` inside
  `DisDialogTree_InGameBind_57`, both built by playing;
* a record whose outer cannot be resolved at all returns NULL and the caller stops by name, rather than
  reaching `StaticAllocateObject`'s hard `Object is not packaged`.

### 2.5 Retail's answer for an entry class with no declared `IsSaveable` is TRUE, not "stop"

Agent ED built the gate the right way round for a tree with fourteen override bodies: when retail treats a
class as a save entry point and this tree declares no `IsSaveable` anywhere in that chain, our FALSE means "no
override here", not "retail did not save it". What that implies, though, is not *stop* but *retail said TRUE*:
those 59 classes carry an inline `return TRUE` that retail's linker folded onto `UObject::IsRefSaveable`,
which is exactly why they have no PDB symbol. Loading them is retail's own behaviour; the gate that remains is
the `GameLoad` one.

And the chain walk had to be fixed with it: **the whole chain is searched for a ported `IsSaveable` before the
entry list is consulted.** Every subclass of a class that declares `IsSaveable` is itself a retail entry class,
because it inherits that vtable slot, so stopping at the first entry class called a class untrusted whose own
base really does answer. `UDishonoredTask_Custom` is the case that showed it — and it showed it as a *silent*
fault, because by then the untrusted answer meant "load it", so the template tasks in `Tower_Objectives`, which
`UDishonoredTask_Base::IsSaveable` exists to skip, were read out of the stream.

### 2.6 Three classes whose ported `GameLoad` this tree's dispatch could not reach

The class lists are computed from retail's vtable: the owner of a class's slot-69 body decides whether the body
is ported. When that body is an **ICF fold** shared by unrelated classes, the list says "ported" while this
tree's virtual dispatch lands on `UObject::GameLoad` and reads nothing — the stream then loses exactly that
object's bytes with no diagnostic, which is the one failure mode the gates exist to prevent.

`UDisAttentionInfo_Base::GameSave` / `GameLoad` (2012 `0x630910`) is such a fold: **eighteen** classes share it
and sixteen do not inherit it here. Their three roots are `UDisConv_Node_InGameData`, `UDisHideoutComponent`
and `UDisSteeringInfluence`; declaring the pair on those three covers all sixteen through ordinary C++
inheritance. `build/agentEC/folds.txt` is the whole-tree check — the only other folds of a ported body are
`ATargetPoint`'s with `ATrigger` (which agent ED declares on both) and two shim classes.

### 2.7 Three retail bodies that are **not** the 2012 bodies

All three have no entry in `match_2012_2013.csv`, and that turned out to be the tell.

| class | 2012 | retail | what retail adds |
|---|---|---|---|
| `UDishonoredTask_Base` | `0x727060`, 42 bytes | `0x6c1820` / `0x6c1870`, 80 and 86 | after the property walk, `m_TemplateTaskName` **again** — the same FName twice in a row in the stream — and, behind `Ar.Ver() >= 21`, `m_Description` and `m_Status`, which the walk itself leaves out because they carry `CPF_DisNoSaveGame` |
| `UDishonoredObjectivesComponent` | `0x72f800`, 23 bytes | `0x6d4fe0`, 42 | `m_LastTaskID` after the objective list |
| `UDishonoredObjective` | `0x72f600` | `0x6d4d60` | `m_TemplateObjectiveName` after the packed byte, and the byte carries **five** bits — `2 * b & 0x3E`, so `m_bInitiallyHidden` (mask 1) is not in the stream at all — where agent ED's port read four starting at mask 1 |
| `UDishonoredGlobalAIManager` | — | `0x841480` | one byte behind `Ar.Ver() >= 23`, into `m_bIgnoreIdealMaximumCount` |

The task's pair was found by listing the retail callers of `UObject::SerializeScriptPropertiesBin` — the whole
exe has five, and two of them are unnamed and sit together. The objectives component's was found the same way
against the object-array serializer, and it is the function immediately after the objective's pair.

`Dishonored0.sav` proves every part of the task's: the walk's own `m_TemplateTaskName` and the explicit one are
the same FName (`Tower_Objectives.TowerEmpress_Report:DishonoredTask_Custom` #3) twice in a row at bytes 3569
and 3573, then `m_Description` is `"Meet the Empress in the gazebo"` and `m_Status` is empty — 4 + 35 + 4 = the
43 bytes that stood between the task and the objective's own `m_TemplateObjectiveName`
(`Tower_Objectives.TowerEmpress_Report`, at byte 3618).

## 3. The measurement

`build/agentEC_release`, `-disrestoreslot=16`, `Dishonored0.sav` ("0 - Dunwall Tower"), null RHI, no
`-noscenerender`. `build/agentEC/agentEC_base.log` is HEAD, `agentEC_r34.log` is the final state.

| | HEAD `fc1b24f` | final |
|---|---:|---:|
| dictionary records read | 11321/11324 | **11321/11324** |
| dictionary bytes | 91844/91844 | **91844/91844** |
| records resolved | 6227 | **6227** |
| records not found | 5093 | **5093** |
| actors spawned from tweaks | 50 | **50** |
| objects restored | 4 | **57** |
| object-stream bytes | 74/619631 | **4141/619631** |
| ported override classes | 36 | **37**, plus 16 reached through the three fold roots |
| stopped on | `DishonoredNPCPawn_26`, unported `GameLoad` | `DishonoredPlayerPawn.PowerBlink` (`DishonoredActivePowerComponent`), unported `GameLoad` |
| player pawn | `X=0.000 Y=500.000 Z=-209.350 P=0 Y=0 R=0` (the spawn transform) | **`X=9826.257 Y=23064.211 Z=2412.241 P=0 Y=-513 R=0`** |

The census line in full:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 57 restored, 79 skipped, 4141/619631 bytes; 50 spawned, 5093 not found, 1 unported, 0 unresolved,
23 untrusted skips, 0 partial bodies, 57 PostGameLoad; STREAM ABORTED
```

**Accept 1, measured against the save.** `build/agentEC/verify_transform.py` searches the level state's
object-data blob for the restored values: there is exactly one match in its first 8,000 bytes, at offset
**4041**, and the `FRotator` that follows it at 4053 is `P=0 Y=-513 R=0`.

```
FVector at byte 4041: X=9826.257 Y=23064.211 Z=2412.241
  FRotator that follows at 4053: P=0 Y=-513 R=0
  raw: 07 89 19 46 6c 30 b4 46 da c3 16 45 00 00 00 00 ff fd ff ff 00 00 00 00
```

The restored pawn reports those numbers to the float. It is not a coincidence of the diagnostic: the world
clock lands too — the d3d9 run's timed screenshot is of *world time 132.167*, and the save records 131.767.

**Is it playable from that state?** It runs. The null-RHI run keeps ticking for **600 s** after the restore,
rendering **111,840** scene frames of the mission world (5,045 scene primitives, both fog layers drawn) with
**0 criticals**; the d3d9 run at 1280x720 renders the Dunwall Tower gazebo from the restored position with the
guards and civilians the save put there (`build/agentEC/agentEC_restored.png`). What is *not* restored is
everything the stream never reached: the restore stops 4,141 bytes into 619,631, so the player's health, mana,
inventory, powers and every object after `PowerBlink` are whatever a fresh session gave them. The player is in
the right place in the right world with the right clock; the session is not yet the saved session.

### 3.1 A second save, in a different world

`Dishonored1.sav` ("1 - Dunwall Sewers", `L_PrsnSewer_P`), slot 17, restores the same way and was not used
while any of this was being built:

```
DisRestore: the save was taken in 'L_PrsnSewer_P'; this world has committed 'Dishonored_MainMenu' - streaming it in
DisSaveLoad census [restore]: 1 level(s); dictionary 6433/6436 objects (4787 resolved) 52672/52672 bytes;
data 41 restored, 13 skipped, 2716/294961 bytes; 28 spawned, 1646 not found, 1 unported, 0 unresolved,
0 untrusted skips, 0 partial bodies, 41 PostGameLoad; STREAM ABORTED
DisRestore: player pawn DishonoredPlayerPawn at X=7241.508 Y=5174.333 Z=3719.230 rotation P=0 Y=-58720 R=0
```

Its own file has that `FVector` at byte **2648** of the level state's object data with exactly that
`FRotator` after it. Two saves, two different mission worlds, both to the float - which is also the second
piece of evidence for agent EB's `m_SubLevels` bit-0 inference, because the travel aimed at `L_PrsnSewer_P`
and the objects the dictionary named resolved there.

Two other slots were tried and neither is about this package:

* **slot 1 is `OPTIONS.sav`**, which `GetSaveGameSlot` deliberately never answers because it is not a game
  state; arming a restore on it reads it as one and dies in `appMalloc`. A `-disrestoreslot` on a non-state
  slot should be refused. Hand-over 9 below.
* **slot 4 (`L_Prison_P`)** never reaches the restore: the `STREAMMAP` travel asserts in
  `ParticleEmitterInstances.cpp:269`, `HighModule->GetClass() == ParticleModule->GetClass()`, while the level
  streams in. That is the particle system, not the save layer, and it is pre-existing on this path.

## 4. The frontier now

The stream stops at `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredPlayerPawn.PowerBlink`, class
`UDishonoredActivePowerComponent`, dictionary record 164's sub-object, at byte 4141.

| class | retail `GameSave` / `GameLoad` | note |
|---|---|---|
| `UDishonoredActivePowerComponent` | slot 68/69, address not resolved here | where the stream stops now. It is reached from `ADishonoredPawn::GameLoad`'s `m_ActivePowers` list, so every pawn with an active power reaches it |
| `URB_BodyInstance` | 2012 `0x3ca2f0` / `0x3c0f00` | `DisLoadPhysicsAssetInstanceBodies` stops on a non-zero body count; every ragdolled or knocked-out NPC in a save needs it |
| the nine partial-state classes | 2012 `0x696340`, `0x6a41c0`, `0x6a6120`, `0x6a63c0`, `0x6afb40`, `0x6b4ec0`, `0x6b96e0`, `0x6ba900`, `0x6c7990` | `UDishonoredNativeStateMachine::LoadPartialState` stops when the saved state is one of the nine whose `LoadPartialState` retail declares. Every other state reads nothing, which is what lets `StateNPCMasterWalk` through |

`ADishonoredNPCController::GameLoad` at retail **`0x763570` is confirmed**, not a candidate: it calls
`AActor::GameLoad`, then `Ar << Pawn` (`AController::Pawn` @584), a virtual with that pawn,
`UArkComponentContainer::StartAllComponents( m_ComponentContainer )` (@900), `Ar << m_pAIBrain` (@896),
`Ar << m_pLastGoToKismetAction` (@920) and, behind `Ar.Ver() >= 23`, `Ar << m_pLastShootKismetAction` (@924).
Every offset lands on a named member of `ADishonoredNPCController`. It is dictionary record 714 of
`Dishonored0.sav`, so it is past the player pawn and was not on this milestone's path; it is not ported.

## 5. Which override classes are ported and which are not

`DishonoredGame/Inc/dissaveload_classlists.h`, regenerated by `build/agentEC/gen_classlists.py` (agent EB's,
with `DishonoredNPCPawn` added to `PORTED` and `PORTED_ISSAVEABLE`): **562 entry classes, 37 ported, 75
unported.**

The 37, by where their bodies live:

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
  `ADishonoredPlayerController`, **`ADishonoredNPCPawn`** (agent EC), and `UDisSeqAct_AutoSave::PostGameLoad`

Plus, new in this package and not counted in the 37 because they are not separate retail bodies, the three
roots of `UDisAttentionInfo_Base`'s fold — `UDisConv_Node_InGameData`, `UDisHideoutComponent`,
`UDisSteeringInfluence` — through which sixteen more classes now reach the ported body.

Still not ported: 75 classes, listed in `dissaveload_classlists.h`. `USeqAct_Interp`'s three save virtuals
(2013 `0x2e73b0` / `0x2e74d0` / `0x2ea1c0`) and the whole writing half (`FLevelSaver` `0x615730`,
`FGameState::SaveLevel` `0x613cb0`, `SaveGameState` `0x602920`, `ProcessSaveLoadCmd` `0x6162d0`) remain out,
as agents ED and EB left them.

## 6. Deviations, stated plainly

1. `ADishonoredNPCPawn::RestoreAppearance`, `StartRagdolling`, `SwitchToEatenMesh`, `SeverLimb` and
   `PostGameLoad` are read past, not called (section 1.4). None reads stream bytes.
2. The corpse branches' `DisLoadPhysicsAssetInstanceBodies` stops the stream on a non-zero count rather than
   guessing at `URB_BodyInstance`'s bytes.
3. The plague byte is never read, because `FDisComponentPlague` does not exist in this tree (section 1.4).
4. `UDishonoredNativeStateMachine::LoadPartialState` does **not** stop when the saved state is simply not
   registered here — `Dishonored0.sav`'s Lady Emily is in `StateNPCMasterWalk`, which this tree's master FSM
   has not registered. It stops only when the state is one of the nine whose `LoadPartialState` retail
   declares, because `UDishonoredNativeState`'s own body is empty and every other state therefore reads
   nothing. That is a fact about retail's function list, not a guess; the nine are named in the source with
   their 2012 rvas.
5. `ADishonoredNPCPawn::GameLoad_Dialog` stops the stream when the pawn has no attached conversation
   component, because retail then serialises an `FDisConvSaveData` through
   `IDisConvSpeakerInterface::GetDialogSaveData`, which this tree does not have. Lady Emily has one.
6. `UDisConversationComponent::IsAttached()` is a written one-line accessor in that class's CppText hook:
   retail's `GameLoad_Dialog` reads `UActorComponent::bAttached` directly and it is `protected` here. It is
   the only place outside the component hierarchy that needs it.
7. `ADishonoredPawn::GameSave`, `ADishonoredNPCPawn::GameSave` (`0x764540`) and
   `UDishonoredNativeStateMachine`'s save virtuals are not ported and not declared, for the reason agent ED
   gave: the writing half of the object layer does not exist here, so nothing would call them and a
   half-written pair would look like the format without being it. The *save* sides of the four bodies this
   package corrected (`USequenceEvent`, `UDishonoredTask_Base`, `UDishonoredObjectivesComponent`,
   `UDishonoredObjective`, `UDishonoredGlobalAIManager`) **were** corrected with their loads, because those
   pairs already existed and leaving one half wrong is worse than either.
8. `FLevelLoader::Serialize` now refuses to read past the end of the object data and stops by name. Retail
   cannot reach that: its stream is always in step, so the terminating index arrives first.

## 7. Verification

* **Accept 1** — the transform, measured against the save: section 3, `build/agentEC/verify_transform.py`,
  `build/agentEC/agentEC_r34.log`, and the d3d9 screenshot `build/agentEC/agentEC_restored.png`.
* **Accept 2** — the stream did reach record 162; the new stopping point and the census are in sections 3
  and 4.
* **Accept 3** — the override list: section 5.
* **Accept 4** — `python resources/tools/run_regression.py --build-dir build/agentEC_release --no-build
  --exe-name DishonoredGame_EC.exe --log-prefix agentEC_reg`: **31 ok, 0 failed, 0 skipped, 423 s**
  (`build/agentEC_release/regression/summary.txt`, run log `build/agentEC/regression2.log`). Clean full
  release build of `DishonoredGame`, `CoreSmoke` and `LayoutProbe` from an empty `build/agentEC_clean`,
  **0 errors** (`build/agentEC_cleanbuild.log`, `build/agentEC_clean_build.cmd`).
* **Layouts** — unchanged, and the regression's own layout stage says so: `layout_types 2314`,
  `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`, `verify_phase2 2/2`. This package adds no
  reflected member; `CPF_DisNoSaveGame` is a flag constant, not a field.
* **Addresses** — `python resources/tools/rva_sweep.py --csv build/agentEC/rva_sweep.csv` then
  `build/agentEC/sweep_mine.py`: of the **324** citations in the files this package touched, **0 mislabelled
  and 0 unknown**. The five `MISLABELLED-2012` the whole-tree sweep reports are the same five agent EB
  reported, all pre-existing and in other agents' files.
* **The dictionary halves are still exact**: 11321/11324 records and 91844/91844 bytes, in every run.
* Everything was built from `build/agentEC_wt`, detached at `fc1b24f`, with only this package's own files
  copied in (`build/agentEC_sync.py`).

## 8. The two diagnostics, because the next agent will want them

* **`-disstreamdebug=<n>`** logs the first n object references the data stream reads, each with the byte
  offset it was read at and how it resolved. The gap between two lines is the previous object's body.
* **`-dispropertytrace`** names every property a DisSaveLoad binary walk reads — both
  `UObject::SerializeScriptPropertiesBin` and `UStruct::SerializeBin`, so structs and whole objects are
  covered — with the same byte offset, and says when a property was skipped.

Six of the seven defects in section 2 were found by reading one run of those two against the save's raw bytes.
The recipe is: run with both, find the last line whose value is plausible, and decode the save from there with
`build/agentED/head_bytes.py`. `build/agentEC/taskwalk.py` is the one case where that was not enough and the
answer came from enumerating the possibilities instead.

## 9. Hand-overs

1. **Coordinator, at merge.** This package changes **five files outside `DishonoredGame`** and four of them
   are load-bearing:
   * `Core/Inc/UnObjBas.h` and `Core/Inc/UnType.h` — `CPF_DisNoSaveGame` and the disjunct in
     `UProperty::ShouldSerializeValue`. **This changes what every DisSaveLoad archive serialises** and nothing
     else: the disjunct is guarded by `Ar.IsDisSaveLoad()`.
   * `Core/Src/UnObj.cpp` and `Core/Src/UnClass.cpp` — the `-dispropertytrace` diagnostic, off unless the
     switch is given, read on first use.
   * `Engine/Src/UnSequence.cpp` — `USequenceEvent`'s four bytes.

   **Regeneration is required**: this package adds `Inc/CppText` hooks for `UDisConversationComponent`,
   `UDisConv_Node_InGameData`, `UDisHideoutComponent` and `UDisSteeringInfluence`, and appends to
   `ADishonoredPawn`, `ADishonoredNPCPawn`, `UDishonoredNativeState` and `UDishonoredNativeStateMachine`, so
   `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`
   must run at merge — **agent EC did not regenerate.** The four new `#include "CppText/<Class>.h"` lines were
   inserted by hand at the place the generator emits them (the last line of the class body). `Sources.cmake`
   does not change: no new source file. `dissaveload_classlists.h` is regenerated by
   `build/agentEC/gen_classlists.py`; it now reads 562 entry classes, 37 ported, 75 unported.

   The nineteen source files, for the merge — fifteen modified and four new and therefore untracked:
   `Core/Inc/UnObjBas.h`, `Core/Inc/UnType.h`, `Core/Src/UnObj.cpp`, `Core/Src/UnClass.cpp`,
   `Engine/Src/UnSequence.cpp`, `DishonoredGame/Src/dissavegame.cpp`,
   `DishonoredGame/Inc/dishonoredutilities_saveload.h`, `DishonoredGame/Inc/dissaveload_classlists.h`,
   `DishonoredGame/Inc/dishonoredgameclasses.h`, `DishonoredGame/Inc/DishonoredGameConversationClasses.h`,
   `DishonoredGame/Inc/DishonoredGameSteeringClasses.h`,
   `Inc/CppText/{ADishonoredPawn,ADishonoredNPCPawn,UDishonoredNativeState,UDishonoredNativeStateMachine}.h`
   and the four new
   `Inc/CppText/{UDisConversationComponent,UDisConv_Node_InGameData,UDisHideoutComponent,UDisSteeringInfluence}.h`.
   `build/agentEC_sync.py` is the authoritative list.

2. **The next package is `UDishonoredActivePowerComponent::GameLoad`.** It is where the stream stops, it is
   reached from `ADishonoredPawn::GameLoad`'s `m_ActivePowers` list, and after it comes 615 KB of level state
   nobody has read yet. The loop is unchanged and is now much faster than it was: add a body, add the name to
   `build/agentEC/gen_classlists.py`'s `PORTED`, rerun it, rerun `-disrestoreslot=16`, read
   `data N restored, X/619631 bytes` — and when the byte count stops adding up, rerun with
   `-disstreamdebug=4000 -dispropertytrace` and compare against the save's own bytes.

3. **Whoever ports `URB_BodyInstance::GameSave` / `GameLoad`** (2012 `0x3ca2f0` / `0x3c0f00`). Every
   ragdolled, knocked-out or dead NPC in a save needs them, and `DisLoadPhysicsAssetInstanceBodies` stops the
   restore until they exist. Note that retail's `GameLoad` is **vtable slot 70**, not 69: retail inserted one
   virtual before the save five, which `FLevelLoader::operator<<`'s own `(*(vtable + 280))` confirms.

4. **Whoever owns the generated headers.** `ADishonoredPawn::m_LatentInteractables` is declared with a
   four-byte element where the reflected property is an eight-byte `InterfaceProperty`. That is a live
   garbage-collector hazard independent of the save, and `m_ActivePowers` is the same class of gap in the
   other direction (`BYTE[12]` for a `TArray`). `sdk_props.py` cannot see either, because the CodeRed dump
   prints an interface array as `TArray<class UX*>` and leaves an untyped inner alone.

5. **Whoever audits the class lists.** A ported body that is an ICF fold shared by unrelated classes is called
   ported for all of them, while this tree's dispatch reaches it only for those that inherit it — and the
   failure is *silent*, because the object's bytes are simply not read. `build/agentEC/folds.txt` is the
   whole-tree check and should become a step of `gen_classlists.py`.

6. **`ADishonoredNPCPawn`'s plague byte** is the one place where this package knowingly reads a different
   number of bytes from retail, and only for a save taken with a weeper resident (section 1.4).

7. **Agent EB's sub-level inference held.** `m_SubLevels`' bit 0 aimed the travel at `l_tower_p` in every run
   of this package, and the objects the dictionary names resolved there; if it had been wrong the 6,227
   resolved records would not have been.

8. **Two robustness gaps outside this package, found by arming the restore on other slots.** Slot 1 is
   `OPTIONS.sav` and is not a game state: `FGameState`'s reader takes its bytes as one and dies in
   `appMalloc` ("Ran out of virtual memory"). And slot 4's world, `L_Prison_P`, asserts in
   `ParticleEmitterInstances.cpp:269` (`HighModule->GetClass() == ParticleModule->GetClass()`) while the
   `STREAMMAP` travel streams it in, before the restore runs at all. Section 3.1.

9. **`UDishonoredEngine::ProcessSaveLoadCmd` (`0x6162d0`) is still the missing state machine.**
   `DisSaveLoadRestoreTick` in `dishonoredengine.cpp` is agent EB's three-state stand-in for it. Nothing in
   this package changed that, and the accept was measured through it.
