# Agent EJ (PHASE12 EJ) — the AI stack behind the brain, and what is behind that

Worktree `build/agentEJ_wt`, detached at `84b0c9d`. Own build dirs `build/agentEJ_release` and
`build/agentEJ_clean`, own IDA copy `build/agentEJ_ida/retail2013_agentEJ.i64`, headless decompiles only, no
IDA MCP tools and no FModel tools. Nothing committed, nothing staged; the shared checkout is clean.

**One process mistake, corrected mid-package and worth recording.** I edited the shared checkout and copied
*into* the worktree after every change. The build was always `cmake -S build/agentEJ_wt`, so every number here
is a worktree number — but the shared checkout was dirty, and a sync script that runs main → worktree would
have been overwritten by the next merge. Everything now lives in the worktree and `build/agentEJ_sync.py` runs
worktree → main, which is the merge direction.

## The answer in eight lines

* **The stream ran from 23,827 bytes to 43,137 of 619,631 on `Dishonored0.sav`, and objects restored from
  305 to 502.** Agent EF's frontier, `UDishonoredAIBrain` (retail `0x7256a0`, vtable slot 70), is behind us.
* **Fifteen retail bodies are ported across eight new override classes, plus the three helpers those bodies
  stand on.** The classes: the seven of the AI stack (`UDishonoredAIBrain`, `UDisAIBrainProcess`,
  `UDisAttentionInfo_Complex`, `UDishonoredAIBehavior`, `UDishonoredNativeStateMachine`, `UDisAISubState`,
  `UDisAISubProcess`) and `UArkComponentContainer` - eight `GameLoad`s and four `PostGameLoad`s. The helpers:
  `UDisStimManager::LoadStim`, `DisLoadAISubTweakReference<T,TOwner>` and `FArkComponentBase::Serialize`.
  **Ported override classes 47 → 55, unported 65 → 57.**
* **The new stopping point is a component class, not a save body.**
  `UArkComponentContainer::GameLoad` reads a count and then, per component, a type id and that component's own
  `Serialize`. The AI brain's container holds **3** components and the first is type **211**,
  `DisCpntType_AIMonitorReaction`, whose class does not exist in this tree. Retail *does* have a creator for
  it (`FArkComponentCreatorRegister::CreatorFn<FDisAIMonitorReaction>`, 2013 `0x5eb950`), so this is a class we
  have never ported and not a registrant to invent — I wrote none.
* **The frontier is proved not to be a missing tail**, which is the lesson agent EF handed over.
  `UArkComponentContainer::GameLoad` is 81 bytes and its disassembly ends `pop/pop/pop/mov esp,ebp/pop ebp/retn 8`
  immediately after the loop's back edge: there is nothing below the loop. Section 4 has the listing.
* **Every value this package restores is checked against the save file's own bytes**, in Python, without the
  game and without the C++ reader (`build/agentEJ/verify_ai.py`, section 3): the component count and type id
  at byte 43,129, two `DisLoadAISubTweakReference` pairs at bytes 25,719 and 25,743 with the tweak object and
  index the save names, the third instantiation at 25,188, and the 105 bytes of
  `m_PendingAttentionChangeAmount` that decide how much `UDisAttentionInfo_Complex::GameLoad` reads.
* **Two things this package ports are NOT exercised by either save, and I say so rather than claim them**:
  `UDisStimManager::LoadStim` and the brain's stim-queue loop (the brain's property walk stops at its
  component container before reaching them), and `FArkComponentBase::Serialize` (the stop is on the first
  component, before any component's `Serialize` runs). Both are transcribed from retail's own disassembly,
  which is quoted in section 5.
* **`Dishonored1.sav` did not move**: 180 objects and 17,842 of 294,961 bytes, exactly as agent EF left it.
  Its blocker is `URB_BodyInstance::GameLoad` and nothing in this package is ahead of it. What this package
  adds there is the wire format, read off retail's disassembly: **1 byte, one `FBoneAtom` (32 bytes, and our
  `Core/Inc/FBoneAtomVectorized.h` serialiser is byte-for-byte the one retail uses), six FLOATs, 1 byte — 58
  bytes per rigid body.**
* **Two corrections to shared facts**: retail's `IsSaveable` is vtable slot **68** and slot 67 is
  `IsRefSaveable`, and retail's `FArchive::operator<<(UObject*&)` is slot **6** with `operator<<(FName&)` at 7 -
  the opposite order to our `UnArc.h`. Both are section 6. One defect of my own (section 7.1), and one thing
  agent CG had already done that only needed connecting (section 2).

## 1. What the stream stopped on, and what came after it

Agent EF left the stream at `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredNPCController_<n>.AIBrain`,
class `UDishonoredAIBrain`, byte 23,827 of 619,631. Retail's `UDishonoredAIBrain::GameLoad` is `0x7256a0`
(vtable slot 70, vftable rva `0xd33ce0`; the 2012 body `0x74d790` has no `match_2012_2013.csv` row, so the
address is retail's own table and not the matcher's).

The body is short:

| # | reads |
|---|---|
| 1 | behind `Ar.Ver() >= 22`, `DisSaveLoadObject` — the brain's own script properties |
| 2 | an INT count, and per entry `UDisStimManager::LoadStim` followed by the `FDisStimRef` delay timer |

and then a tail that reads no stream byte (the global-AI-manager registration, two event subscriptions and one
avoidance-group refresh on the owning pawn).

Row 1 is where all the distance is. The brain's property walk is **19,300 bytes long**, because eleven of its
properties are object references and arrays of object references, and the object layer reads a referenced
object's whole `GameLoad` inline at the point the reference appears. Walking it therefore pulled in the six
other classes of the AI stack in one step:

| record | class | retail `GameLoad` | reached from |
|---:|---|---|---|
| 716 | `UDishonoredAIBrain` | `0x7256a0` (slot 70) | the level's own object list |
| 730 | `UDisAIBrainProcess` | `0x7362c0` (slot 70) | `m_pAttentionProcess` and `m_BrainProcesses` |
| 732 | `UDisAttentionInfo_Complex` | `0x74ff80` (slot 70) | `UDisAIBrainProcessAttention::m_AttentionInfos` |
| 737 | `UDishonoredAIBehavior` | `0x6f6f80` (slot 70) | `m_ActiveBehaviorStack` and `m_BehaviorArray` |
| 739 | `UDishonoredNativeStateMachine` | `0x67a9e0` (slot 70) | `UDishonoredAIBehavior::m_pBehaviorFSM` |
| 741 | `UDisAISubState` | `0x7126c0` (slot 70) | the FSM's `m_NativeStates` |
| 751 | `UDisAISubProcess` | `0x73d580` (slot 70) | `UDishonoredAIBehavior::m_SubProcesses` |

Three of those seven need something the tree did not have, and those three are the real content of this
package:

* **`UDisStimManager::LoadStim`** (`0x704f60`). Agent CG had written the pool, the 111-row stim type registry,
  the dispatch table and `DisStimRefInit`, and left `SaveStim`/`LoadStim` out with their retail bodies written
  down in a comment. The body is now in `Src/disstimmanager.cpp` in place of that comment.
* **`DisLoadAISubTweakReference<T, TOwner>`** (`0x731310`), in retail's own header for it,
  `DishonoredGame/Inc/dishonoredutilities_saveload_ai.h`, which was an `import_reference.py` stub. A sub-tweak
  is a cooked-package object the save's dictionary cannot name, so retail writes the *owning* tweak object plus
  the index of the sub-tweak inside one array of that owner — which is why the third argument is a pointer to
  member. Six bytes: a WORD dictionary index and an INT.
* **`UDisAttentionInfo_Complex::GameLoad`'s conditional tail**, which is the only body here whose length
  depends on values it has just read (section 3.3).

Past them the stream reaches the brain's last reference, `m_pBrainComponentContainer`, at byte 43,127 — and
that is `UArkComponentContainer`, Engine's, whose `GameLoad` is `0x534220`. Porting it moved the stream eight
more bytes and turned an unported-class stop into a named component type.

## 2. What agent CG had already done, and what was missing was the wiring

Three of the six `PostGameLoad` bodies here were already written, by agent CG, under names that could not be
mistaken for overrides — `UDisAIBrainProcess::PostGameLoad_BrainProcess`, `UDisAISubState::PostGameLoad_SubState`,
`UDisAISubProcess::PostGameLoad_SubProcess` — each with a comment saying that retail reaches them through a
DisSaveLoad vtable slot `UObject` did not declare in this tree, so nothing called them. Agent ED declared those
slots. This package connects them: `UDisAISubState::PostGameLoad` and `UDisAISubProcess::PostGameLoad` are one
line each, and `UDishonoredAIBrain::PostGameLoad` is the loop that calls
`PostGameLoad_BrainProcess` on every brain process. That is why the census's `PostGameLoad` counter tracks the
restored count exactly (502/502) rather than lagging it.

The same applies to `UDisAISubState::GameLoad` and `UDisAISubProcess::GameLoad`: agent CG had written both
retail bodies into the "not ported" comment at the bottom of `disaisubstate.cpp` and `disaisubprocess.cpp`,
correct to the line. Landing them was a transcription and those two comments are now gone, because a comment
saying a thing is not ported is worse than no comment once it is.

## 3. The measurement, and every value against the save's own bytes

`build/agentEJ_release` (configured `-S build/agentEJ_wt`), `-disrestoreslot=16` and `-disrestoreslot=17`, null
RHI. `build/agentEJ/agentEJ_base16.log` and `agentEJ_base17.log` are HEAD `84b0c9d`; `agentEJ_final16.log`,
`agentEJ_final17.log` and `agentEJ_trace16.log` are the final state.

| | HEAD `84b0c9d` (= agent EF) | agent EJ |
|---|---:|---:|
| **Dishonored0.sav ("0 - Dunwall Tower")** | | |
| dictionary records read | 11321/11324 | **11321/11324** |
| dictionary bytes | 91844/91844 | **91844/91844** |
| records resolved | 6227 | **6227** |
| objects restored | 305 | **502** |
| object-stream bytes | 23827/619631 | **43137/619631** |
| PostGameLoad run | 305 | **502** |
| stopped on | `DishonoredNPCController_26.AIBrain` (`DishonoredAIBrain`), an unported class | **inside `UArkComponentContainer::GameLoad`, on Ark component type 211** |
| **Dishonored1.sav ("1 - Dunwall Sewers")** | | |
| objects restored | 180 | **180** |
| object-stream bytes | 17842/294961 | **17842/294961** |
| stopped on | `DisLoadPhysicsAssetInstanceBodies`, 18 rigid bodies | **unchanged** |
| ported override classes | 47 | **55** (unported 65 → 57) |

The census line in full, slot 16:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 502 restored, 1452 skipped, 43137/619631 bytes; 50 spawned, 5093 not found, 0 unported, 0 unresolved,
206 untrusted skips, 1 partial bodies, 502 PostGameLoad; STREAM ABORTED
```

`0 unported` is new: no object in this save now reaches a class whose `GameLoad` this tree has not ported. The
stop is inside a ported body, which is what `1 partial bodies` says.

**`Dishonored1.sav` did not move and that is the honest number.** Its stream stops at
`DisLoadPhysicsAssetInstanceBodies` with 18 rigid-body states at byte 17,842, which is far ahead of any AI
brain, so nothing in this package can reach it. Section 8 hand-over 1 is what it needs.

### 3.1 The offsets, and why they are not circular

Three independent sources, and every number below is one of them plus arithmetic on the file:

* `-disstreamdebug` prints `DisStream <n>: byte <B> ref <R> -> <path>` for every object reference the stream
  resolves. `B` is `Tell()` **after** the WORD, so the reference occupies bytes `B-2..B`.
* `-dispropertytrace` prints `DisBin byte <B> <Class>.<Prop> (<Kind>)[ skipped]` for every property of every
  binary walk.
* the census byte count of a run that stopped somewhere is the number of bytes read before the stop, so two
  runs that stop at different places bracket the body between them.

`build/agentEJ/verify_ai.py` decodes `Dishonored0.sav` in Python — it decompresses the persistent level state's
object blob and reads bytes — and does not use the C++ reader for anything. Its output is
`build/agentEJ/verify_ai.txt`.

### 3.2 `UArkComponentContainer::GameLoad`, the object the stream stops in

The run without this body ported stopped having read **43,129** bytes; the run with it stopped having read
**43,137**. So the body read bytes 43,129..43,137, which is its INT count plus its first INT component type id
and nothing else. The property trace agrees from the other side:
`DishonoredAIBrain.m_pBrainComponentContainer` is at byte 43,127 and the property after it at 43,137.

```
byte 43129  03 00 00 00 d3 00 00 00
            m_Components.Num() = 3, first EArkComponentType = 211 (DisCpntType_AIMonitorReaction)
            those 8 bytes occur 16 times in the whole 619631-byte blob
```

Sixteen occurrences is corroboration rather than noise: every AI brain's container in this level holds the same
three components in the same order.

### 3.3 `DisLoadAISubTweakReference`, six bytes, three instantiations, one folded body

`m_BrainProcesses[0]` is `DisAIBrainProcessHideoutWatcher_0`. Its property walk ends at byte 25,719 (the trace:
`m_pOwningPawn` at 25,717, two bytes, then only skipped properties), and the stream log resolves the *next*
array element at byte 25,727, so that element's WORD is at 25,725. The six bytes between are the whole of the
tweak reference:

```
byte 25715  cc 02              record 716  m_pOwningBrain   (log: ref 716 -> ...NPCController_26.AIBrain)
byte 25717  78 02              record 632  m_pOwningPawn    (log: ref 632 -> ...DishonoredNPCPawn_30)
byte 25719  00 00 ff ff ff ff  owner record 0 (none), index -1 (INDEX_NONE)
byte 25725  da 02              record 730, the next m_BrainProcesses element
```

`m_BrainProcesses[1]` is `DisAIBrainProcessBattleSense_0` and its owner is **not** null:

```
byte 25743  df 02 01 00 00 00  owner record 735, index 1        (14 occurrences in the blob)
```

and the stream log resolves record 735 to `AI_BrainTweaks_Elite.BrainTweaks_Guard_Elite`, a
`UDisTweaks_AIBrain` — so the restored value is `BrainTweaks_Guard_Elite->m_BrainProcessTweaks(1)`, which is
exactly what the ported body assigns.

The other instantiation, on `DisAISubProcessStandardBodyIntention_6`, whose walk ends at 25,188 and whose
successor resolves at 25,196:

```
byte 25188  e8 02 00 00 00 00  owner record 744, index 0
```

record 744 is `AI_BehaviorTweaks_Idle.TweaksBehavior_Idle_Aggressive`, a `UDisTweaks_AIBehavior`, so
`m_pSubProcessTweaks = TweaksBehavior_Idle_Aggressive->m_SubProcessTweaks(0)`.

### 3.4 `UDisAttentionInfo_Complex::GameLoad`, the one body whose length it reads itself

The trace puts `m_PendingAttentionChangeAmount` at byte 24,354 and the property after it at 24,459 — **105
bytes = 21 × 5**, one `FDisAttentionPendingChangeAmount` (`FLOAT m_fByAmount`, `BYTE m_ToThreshold`) per slot.
And it marks `m_PendingAttentionChange` itself **skipped**, i.e. it is not in the property walk at all, which
is precisely why retail reads it separately and conditionally.

In this save all 21 amounts are zero, so the condition answers FALSE 21 times and the body reads 0 × 44 extra
bytes. The condition is therefore proved to be *evaluated* against the file; the 44-byte struct read is not
exercised by this save and I do not claim it is.

### 3.5 Where the brain's own body starts

The run before this package stopped having read 23,827 bytes **on** this object, so the brain's body begins at
byte 23,827; the trace confirms it from the other side, `DishonoredAIBrain.m_bCombatEngaged` at byte 23,827.

```
byte 23827  00 00 00 00 01 01 00 00 00 00 00 01 0e 00 00 00
```

— the brain's seven bits, one byte each (`m_bNeedsRefreshThoughts` is skipped), then the suspicion and
attention bytes.

## 4. The frontier now, and the proof that it is a frontier

The stream stops **inside** `UArkComponentContainer::GameLoad`, on the first component of the AI brain's
container, type 211. Agent EF's hand-over 3 says a plausible stopping point is not proof and that a body whose
last statement is a virtual call must be resolved in retail before it is believed finished. So:

```
00534220  push ebp / mov ebp,esp / push ecx / push ebx / push esi / push edi
00534227  mov  edi, [ebp+arg_0]                      ; FArchive&
0053422a  push 4 / lea eax,[ebp+arg_0] / push eax / mov ecx,edi
00534234  call FArchive::ByteOrderSerialize          ; INT count, into the argument slot
00534239  xor  esi, esi
0053423b  cmp  [ebp+arg_0], esi
0053423e  jle  short loc_934268                      ; count <= 0 -> done
00534240  loc_934240:
00534240  push 4 / lea ecx,[ebp+var_4] / push ecx / mov ecx,edi
00534248  call FArchive::ByteOrderSerialize          ; INT component type id
0053424d  mov  edx, [ebp+var_4] / push edx / mov ecx,ebx
00534253  call sub_934100                            ; UArkComponentContainer::AddNewComponentByID
00534258  mov  edx,[eax] / mov ecx,eax / mov eax,[edx+14h]
0053425f  push edi
00534260  call eax                                   ; FArkComponentBase vtable +0x14 = slot 5 = Serialize
00534262  inc  esi / cmp esi,[ebp+arg_0] / jl short loc_934240
00534268  loc_934268:
00534268  pop  edi / pop esi / pop ebx / mov esp,ebp / pop ebp / retn 8
```

The loop's exit label is five instructions from `retn 8`, all of them epilogue. **There is no tail.** The stop
is the component type and not a call below the loop.

And the type is a class we have not ported rather than a registrant we failed to write: retail has
`FArkComponentCreatorRegister::CreatorFn<FDisAIMonitorReaction>` at 2013 `0x5eb950`, one of **21** creator
instantiations. This tree registers exactly one, `FArkComponentLocomotion`. I wrote no creator and no component
class; inventing either would be a feature retail does not have in the place where retail has a class we have
not written.

`build/agentEJ/frontier.py` lists every record of `Dishonored0.sav`'s persistent level with its body and its
state. **Nine** override bodies are missing from the level, down from seventeen, and all nine are behind the
component gate rather than in front of it:

| record | class | retail `GameLoad` | retail bytes |
|---:|---|---|---:|
| 1814 | `UDisConv_Soiree_InGameData` | `0x8a9540` | 115 |
| 1817 | `USeqAct_Interp` | `0x2e74d0` | 9 |
| 3307 | `ADishonoredRoute` | `0x6438f0` | 88 |
| 8019 | `ADishonoredUsableObject` | `0x657840` | — |
| 8033 | `ADisRatSpawner` | `0x643500` | 62 |
| 8115 | `ANavMeshBlockToggleable` | `0x509360` | 60 |
| 8142 | `ADisStatPickup` | `0x62b640` | 42 |
| 9054 | `UDisSeqAct_NPCTrackTarget` | `0x7a8000` | 166 |
| 11323 | `UDisConvGlobalMan` | `0x8a07b0` | — |

Every one of them is under 170 bytes and seven have a named body in `functions_2013.csv`. They are not the next
package; the Ark component layer is.

## 5. The two bodies this package ports that neither save exercises

Both are transcriptions of retail's own instruction sequence, and both are said plainly because a body that has
never run is not a body that has been shown to work.

**`FArkComponentBase::Serialize`** was an empty inline stub in `Engine/Inc/arkcomponentbase.h`. An empty stub
is not a no-op here: it is the body **sixteen of retail's nineteen Ark component types use**
(`build/agentEJ/comp_serialize.py` reads every `FArk*`/`FDis*` component vftable's slot 5 out of retail and
compares it against `0x532e20`; only `FDisAIKnowledgeComponent` `0x7039f0`, `FDisAIMonitorPawnReachability`
`0x73b010` and `FDisAIMonitorReaction` `0x73b1a0` have their own). Retail's body, from the disassembly, not
from our reader:

```
00532e28  mov eax,[edi] / mov edx,[eax+18h]     ; FArchive vtable +0x18 = operator<<(UObject*&)
00532e2f  lea ecx,[esi+4] / push ecx / mov ecx,edi / call edx      ; m_pOwner
00532e37  push 4 / lea eax,[esi+8]  / push eax / call ByteOrderSerialize   ; m_bStarted
00532e44  push 4 / add esi,0Ch      / push esi / call ByteOrderSerialize   ; m_bPendingStop
00532e51  pop edi / pop esi / pop ebp / retn 4
```

Three values, in that order, and nothing else. It is not exercised because the stream stops on the *first*
component of the first container it reaches, before any component's `Serialize` runs.

**`UDisStimManager::LoadStim`** and the brain's stim-queue loop are not reached either, because the brain's
property walk stops at `m_pBrainComponentContainer` before the walk returns. Retail's body reads an INT stim
type id, an INT pool block index, and then the stim's own script properties through its `UScriptStruct`. Two
facts about it are worth carrying:

* the block index is a hint and not a promise — retail looks the named block up in *this* session's pool and
  uses it only while it is still on the free list, so a save reloaded into a pool that already holds live stims
  reads the properties into whatever is there rather than overwriting the live stim;
* `m_StimQueue` is **`skipped`** in the brain's property walk (the property trace says so), which is why retail
  has to read the queue separately at all. `m_BrainProcesses`, which has the same `CPF_Transient|CPF_NeedCtorLink`
  pair in the SDK dump, is *not* skipped — so the flag that skips `m_StimQueue` is one the SDK dump's 32-bit
  `flags` field cannot show, i.e. `CPF_DisNoSaveGame` (`0x0002000000000000`). That is the first time this
  project has caught that flag in the act on a specific property.

## 6. Two shared facts corrected

**`IsSaveable` is retail vtable slot 68, not 67; slot 67 is `IsRefSaveable`.** Agent EF established that
retail's save five are slots 67..71 and named `GameSave` 69 and `GameLoad` 70, both of which hold. The two
below them are in `UObject`'s declaration order (`Core/Inc/UnObjBas.h:2861` and `:2865`): 67 `IsRefSaveable`,
68 `IsSaveable`. `build/agentEJ/gen_classlists.py` says "slot 67 (IsSaveable)" and is **right**, because it
reads `vtables.csv`, which is the 2012 database where the five are 66..70 — but a comment that says *retail's*
slot 67 is `IsSaveable` is wrong, and two of mine did until I checked. The evidence is classes that have a real
`IsSaveable` body: `USeqAct_Interp`'s vftable carries `USequenceObject::IsSaveable` at slot **68**
(`0x2cf930`) and `ADisStatPickup`'s carries `ADishonoredNPCPawn::IsSaveable` at slot **68** (`0x74aa80`), with
`UObject::IsRefSaveable` at 67 in both. For every class this package declares `IsSaveable` on, retail's slot 68
is the ICF fold onto `UObject::IsRefSaveable`'s body (`0x5ea9d0`, `return TRUE`), so `return TRUE` is retail's
answer and the declarations are correct; only the slot number in the comment was not.


**Retail's `FArchive::operator<<(UObject*&)` is vtable slot 6 (offset 24) and `operator<<(FName&)` is slot 7
(offset 28)** — the opposite order to this tree's `Core/Inc/UnArc.h`, which declares the `FName` overload
first. It is settled by `DisSaveLoad::FLevelLoader`'s and `FLevelSaver`'s own vftables (rvas `0xcdbc80` and
`0xcdbcf8`), where the two bodies are distinct and named:

```
FLevelLoader  slot 6 off 24  rva 0x60c930  operator<<(class UObject * &)
FLevelLoader  slot 7 off 28  rva 0x5fe780  operator<<(class FName &)
```

The reason nobody had settled it before is an ICF fold: on a plain `FArchive` both base bodies are
`return *this`, the linker folds them, and IDA names **both** slots `operator<<(FName&)` — so the base tables
say the opposite of the truth. Everything that mattered was already right: agent ED's
`UDishonoredNativeStateMachine::LoadPartialState` reads offset 24 as an object reference and does so correctly,
and this package's `DisLoadAISubTweakReference` does the same. The ordering difference in our header is
cosmetic, because nothing in this tree dispatches `FArchive` by slot. It is recorded so the next agent reading
a decompile does not have to rediscover it.

## 7. Deviations and defects, stated plainly

1. **My own defect, caught by building.** `UDisStimManager::LoadStim` referenced `DisStopRestore` and
   `DisGetStimTypeInfo` without including `dishonoredutilities_saveload.h` or `aistimstruct.h`; the build gave
   twelve errors in one unit. Trivial, but it is the only thing that broke.
2. **`UDishonoredAIBrain::GameLoad` stops the stream on a save older than version 22.** Retail's other branch
   is a different property walk (`0x7e8910`) reading a different number of bytes. Every save this project has
   is version 24, and reading a version 21 save through the `>= 22` branch would desynchronise the whole level.
3. **`UDishonoredAIBrain::GameLoad`'s tail is read past, not called**: `AddBrain` on the global AI manager, two
   `FArkGameEventDispatcher` subscriptions and one avoidance-group refresh on the owning pawn. None reads a
   stream byte, and putting a half-restored brain on the manager's tick list is worse than not registering it.
4. **`UDishonoredAIBrain::PostGameLoad` ports one of retail's four actions** — the brain-process loop, which is
   the one whose pieces exist. Not called: `FDisMonitorNPCAttention::Starting` and `FDisAIMonitorReaction::Init`
   on the two monitor components the container holds (neither class exists here), and
   `FDisLookAtRequest::PostGameLoad` against the owning pawn's `FArkComponentLookat` (no accessor here).
5. **`UDishonoredAIBehavior::GameLoad`, `UDisAISubState::GameLoad` and `UDisAISubProcess::GameLoad` do not
   re-register the other-actor-terminated event** that retail re-registers around/after the property walk. No
   stream bytes; the event dispatcher's state during a restore is not this package's to own.
6. **`UDishonoredAIBehavior::PostGameLoad` does not call retail's last statement**, vtable slot 92 (offset 368)
   with `!m_bHasStarted` and `m_bIsPaused`. Its base body is the do-nothing fold `0x128ad0` and no behaviour
   class in this tree overrides it, so calling it would be calling that fold.
7. **`UDisStimManager::LoadStim` assigns `m_pStimManager` when the property walk left it NULL.** Retail does
   not, because `m_pStimManager` is a reflected member of `FAIStimStruct` with no `CPF_Transient` and the save
   carries it. A pooled stim whose manager is NULL never returns its block to the pool
   (`DisStimRefRelease`'s own rule), so the assignment is a leak guard and not a format change.
8. **`UDisStimManager::LoadStim` sets the stim's dispatch table after construction.** Retail keeps its vtable
   in the object so the placement-new installs it; this port keeps it beside the object (agent CG's
   `aistimstruct.h` explains why) and therefore has to set it, exactly as `DisNewStim` does.
9. **`UDisStimManager::LoadStim` checks the stim type row and the script struct**, where retail indexes
   `g_StimTypeInfos` and dereferences with no test. A tree with an incomplete stim table would construct
   through a NULL creator and the crash would say nothing about which body was reading; it stops with its own
   name instead.
10. **`UDishonoredAIBrain::GameLoad` stops when `m_pStimManager` is NULL and the save has queued stims**, and
    `UArkComponentContainer::GameLoad` stops when a component id has no creator. Retail dereferences in both
    places without a test, because the session that wrote the save had both.
11. **Every `GameSave` this package met is left out**, for agent ED's reason: the writing half of the object
    layer does not exist here. Nine of them, listed in `agentEJ_status.csv`.
12. **Two file-local helpers became declared functions.** `DisStopRestore` and `DisReadStreamCount` were
    `static` in `dissavegame.cpp`; six of the AI stack's own retail units need them, so they are now declared in
    retail's own save header beside `DisSaveLoadObject`. No behaviour change.
13. **One new Core member function.** `FArchive::SetError()` — `ArIsError` is `protected` and a ported
    DisSaveLoad body outside DishonoredGame cannot reach `DisStopRestore`, which lives in `DishonoredGame/Inc`.
    `UArkComponentContainer::GameLoad` raises the archive's error flag and says why itself, and
    `FLevelLoader::Serialize` turns that into the same abort, so the census still reports `STREAM ABORTED` and
    the warning names the class. One `FORCEINLINE`, no layout change, no new virtual.

## 8. Hand-overs

1. **`URB_BodyInstance::GameLoad` (2013 `0x39e7f0`, vtable slot 70) is still the whole distance between
   `Dishonored1.sav` and the rest of its blob**, and this package decoded its wire format so landing it is a
   transcription plus the physics:

   ```
   BYTE bHasBody;                                   Ar.Serialize( &bHasBody, 1 );
   if( bHasBody )
   {
       FBoneAtom BoneAtom;   Ar << BoneAtom;         // operator<<(FArchive&, FBoneAtom&), 32 bytes
       FLOAT LinVel[3];      three ByteOrderSerialize( , 4 )
       FLOAT AngVel[3];      three ByteOrderSerialize( , 4 )
       BYTE  Flags;          Ar.Serialize( &Flags, 1 );
       ... PhysX: setGlobalPosition, setGlobalOrientationQuat, and, unless the actor is kinematic,
           setLinearVelocity / setAngularVelocity and then putToSleep or wakeUp(0.4f) ...
   }
   ```

   **58 bytes per rigid body**, and 18 bodies in that save. The `FBoneAtom` is the vectorized one:
   `GameSave` (`0x3a8620`) builds it from the body's own `FQuat` at offset 160 and its translation at 176 with
   W forced to 1 — two 16-byte registers, no scale — which is byte-for-byte what our
   `Core/Inc/FBoneAtomVectorized.h` `operator<<` writes (`Ar << *(FVector4*)&Rotation; Ar << *(FVector4*)&TranslationScale;`).
   Nothing past the byte reads a stream byte.

2. **The next package on `Dishonored0.sav` is the Ark component layer, and it is bounded.**
   `UArkComponentContainer::GameLoad` needs every `EArkComponentType` a save names to have a registered
   creator. Retail has 21 (`functions_2013.csv`, `FArkComponentCreatorRegister::CreatorFn<...>`,
   `0x5eb640`..`0x5ebf00`); this tree registers one. **Sixteen of the nineteen component classes read exactly
   `FArkComponentBase::Serialize`, which this package ports**, so for those the save work is the class, not the
   serializer. The three that read more are `FDisAIKnowledgeComponent` (`0x7039f0`),
   `FDisAIMonitorPawnReachability` (`0x73b010`) and `FDisAIMonitorReaction` (`0x73b1a0`) — and 211,
   `FDisAIMonitorReaction`, is the one the stream stops on, so it cannot be deferred.
   `build/agentEJ/comp_serialize.py` regenerates that table from retail.

3. **`UDisStimManager::LoadStim` and `FArkComponentBase::Serialize` have never run.** Whoever lands hand-over 2
   will be the first to exercise both, and should re-read section 5 before trusting either. `LoadStim` in
   particular has a branch — the block-not-free path — that reads the properties into an object it did not
   construct, and that is retail's behaviour, not a simplification.

4. **A property that is `CPF_Transient` in the SDK dump may still be in the DisSaveLoad stream.**
   `m_BrainProcesses` and `m_StimQueue` carry the same 32-bit flags in `retail_sdk_layout.json`
   (`CPF_Transient|CPF_NeedCtorLink`) and the property trace shows the first serialised and the second skipped.
   The flag that separates them is above bit 32, where the SDK dump's `flags` field does not reach, and on this
   evidence it is `CPF_DisNoSaveGame`. Anyone reasoning about what a walk reads from the JSON alone will get
   this wrong; `-dispropertytrace` is the answer.

5. **`resources/tools/symbols/gen_classes_header.py` has not been run.** This package adds one
   `#include "CppText/UDisAttentionInfo_Complex.h"` line to `dishonoredgameclasses.h`, inserted by hand at the
   place the generator emits it (the line after `DECLARE_CLASS`), and appends to seven existing CppText files,
   which the generator does not own. `Sources.cmake` does **not** change: no new source file, and
   `dishonoredutilities_saveload_ai.h` is a header. See section 9.

6. **Agent EF's hand-over 5 (the `gen_classes_header.py` inner-type defect) and hand-over 6
   (`SpawnInventoryLoadout_Items`) are untouched by this package and still stand.**

## 9. Merging

**Regeneration is required but was not run here.** Run

```
python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake
```

with **all three flags** at merge — agent EF's merge cost 113 unresolved externals from a run without
`--sources-cmake`. What it must reproduce is one line: `#include "CppText/UDisAttentionInfo_Complex.h"` as the
last line of `class UDisAttentionInfo_Complex`'s body in `dishonoredgameclasses.h`. Nothing else in that header
changed. **`Sources.cmake` does not change.**

`dissaveload_classlists.h` is regenerated by `build/agentEJ/gen_classlists.py`, which writes into the worktree;
it reads **562 entry classes, 55 ported, 57 unported**, and its ICF-fold check reports **0**. The check is shown
capable of reporting something by `build/agentEJ/make_foldselftest.py`, which reruns it with agent EC's three
hand-declared fold roots removed and reproduces agent EC's own defect: `DisAttentionInfo_Base`, slots 68 and 69,
**32 classes**.

**Twenty-six files, and six of them are outside `DishonoredGame`** — four source, two documents:

| file | why |
|---|---|
| `Core/Inc/UnArc.h` | `FArchive::SetError()`, one `FORCEINLINE`. Deviation 13 |
| `Engine/Inc/arkcomponentbase.h` | `FArkComponentBase::Serialize`, which was an empty stub |
| `Engine/Inc/EngineArkaneClasses.h` | `UArkComponentContainer::GameLoad` + `IsSaveable`, where the class is hand-written |
| `Engine/Src/arkcomponentcontainer.cpp` | that body, in retail's own unit |
| `resources/docs/agents/agentEJ.md` | this report |
| `resources/docs/agents/agentEJ_status.csv` | this package's per-address data, 40 rows |

and twenty inside it: `Inc/dishonoredutilities_saveload_ai.h` (was an `import_reference.py` stub),
`Inc/dishonoredutilities_saveload.h`, `Inc/dissaveload_classlists.h`, `Inc/dishonoredgameclasses.h`, the new
`Inc/CppText/UDisAttentionInfo_Complex.h` and appends to
`Inc/CppText/{UDishonoredAIBrain,UDisAIBrainProcess,UDisAISubState,UDisAISubProcess,UDishonoredAIBehavior,UDishonoredNativeStateMachine,UDisStimManager}.h`,
and the bodies in
`Src/{disstimmanager,dishonoredaibrain,disaibrainprocess,disaisubstate,disaisubprocess,dishonoredaibehavior,dishonorednativestatemachine,dissavegame}.cpp`.

`build/agentEJ_sync.py` is the authoritative list and performs the copy (`--apply`, worktree → main).

## 10. Verification

* **Accept 1 — the census on both saves**: section 3. `build/agentEJ/agentEJ_base16.log` /
  `agentEJ_base17.log` (HEAD) and `agentEJ_final16.log` / `agentEJ_final17.log` / `agentEJ_trace16.log`.
* **Accept 2 — every restored value against the save's own bytes**: sections 3.2..3.5,
  `build/agentEJ/verify_ai.py` and `verify_ai.txt`. What is *not* exercised is section 5.
* **Accept 3 — the new stopping point and the evidence it is not a missing tail**: section 4, with the
  disassembly of `0x534220` to its `retn 8`.
* **Accept 4 — regression**: `python build/agentEJ_wt/resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEJ_regwt --exe-name DishonoredGame_EJR.exe --log-prefix
  agentEJ_reg` — **37 ok, 0 failed, 0 skipped, 872 s**, the build stage included (no `--no-build`), so all six
  build checks count (`build/agentEJ_regwt/regression/summary.txt`). Section 11 is the two things that make
  running the harness against a worktree different, because they cost me two runs first.
* **Accept 5 — clean full release build**: `build/agentEJ_clean_build.cmd` — an empty
  `build/agentEJ_clean`, the worktree as source, `DISHONORED_LAYOUT_CHECKS=ON`, all three targets
  (`DishonoredGame`, `CoreSmoke`, `LayoutProbe`): **0 errors, 0 C4263, 0 C4264**
  (`build/agentEJ_cleanbuild.log`, 972 ninja edges).
* **Accept 6 — `rva_sweep.py` over the worktree**: of the **702** citations in the files this package touched,
  **0 mislabelled, 0 unknown, 0 fabricated** (`build/agentEJ/sweep_mine.py` over
  `build/agentEJ/rva_sweep.csv`): 391 `ok-2013`, 309 `ok-2013-mid`, 2 `ok-2012-labelled`. The whole-worktree
  sweep reports 2 `MISLABELLED-2012`, both pre-existing in `GFxUI/Src/gfxuirenderer.cpp:1669`, another agent's
  file, in a block explicitly labelled "2012". A third, `CppText/UDisAISubState.h:16`, was in a file this
  package touches; it is agent CG's line naming both builds in one comment and it now says `2012 0x16f40`, so
  the sweep reads it correctly.
* **Layouts** — unchanged. This package adds no reflected member and changes no class size. The one Core
  change is a `FORCEINLINE` accessor; the one Engine layout-adjacent change is an inline body, not a member.
* **Which binary measured what.** The restore measurements of section 3 come from `build/agentEJ_release`,
  configured `-S build/agentEJ_wt`; a rebuild of it after the last source edit reported `ninja: no work to do`
  for all three targets at the time the numbers were taken. Two comment-only corrections landed in
  `Engine/Inc/arkcomponentbase.h`'s neighbour `EngineArkaneClasses.h` and in
  `Inc/CppText/UDishonoredAIBrain.h` afterwards (section 6, the slot-68 fix), and the regression's own build
  stage compiled the tree with those in, 0 errors. So no measurement here was taken against a tree that held
  another agent's work, and the only delta between the restore binary and the final worktree is two comments.
* **The dictionary halves are still exact**: 11321/11324 records and 91844/91844 bytes on slot 16,
  6433/6436 and 52672/52672 on slot 17, in every run.

## 11. The regression, and two things about running it from a worktree

`resources/tools/run_regression.py` has **37** checks over six stages, and the build stage is six of them. Two
facts about running it against a snapshot worktree cost me two full runs and are worth writing down, because
every worktree package after this one will hit both.

1. **`--build-dir <a worktree-configured dir>` cannot pass the build stage.** The stage shells out to
   `resources/build-release.cmd`, which does `cd /d "%~dp0.."` and then `cmake -S .` — so it always configures
   **the repo root** into the build dir it is given. Handed a build directory whose CMake cache was generated
   from `build/agentEJ_wt`, cmake refuses: *"The source ... does not match the source ... used to generate
   cache"*, and the three `build_*_exit` checks fail with `build_*_errors` at 0 —
   which is the signature of this, not of a broken build. The fix is to run **the worktree's own copy** of the
   harness: `python build/agentEJ_wt/resources/tools/run_regression.py --build-dir <absolute path>`. Then
   `REPO` is the worktree, `-S .` is the worktree, and the build stage builds the package. The build dir must be
   **absolute**, because `build-release.cmd` resolves a relative one against the worktree while the later stages
   resolve it against the caller's directory.
2. **A worktree does not contain the layout stage's inputs.** `resources/docs/types/types.json` (111 MB),
   `retail_sdk_layout.json`, `script_classes_2012.json`, `script_classes_2013.json` and `all_types.h` are
   gitignored, so a fresh worktree has none of them and `xcheck_sdk_layout.py`, `gen_layout_probe.py compare`
   and `verify_phase2.py` all raise `FileNotFoundError` — which the harness records as **-1** against every
   layout metric. Copying those five files into the worktree's `resources/docs/types/` fixes it and does not
   dirty its `git status`, because they are ignored there too.

The runs, on the same worktree and the same three binaries:

| | run 1 (`--build-dir build/agentEJ_release`) | run 2 (worktree harness, before the data files) | **run 3, the one that counts** |
|---|---|---|---|
| build (6) | 3 FAIL: `build_*_exit` 1, `build_*_errors` 0 (fact 1) | 6 ok | **6 ok** |
| coresmoke (2) | ok, 99 passed / 0 failed | ok | **2 ok** |
| layout (7) | 7 ok, `layout_mismatching` 0, `verify_phase2` 2/2 | 6 FAIL at -1 (fact 2) | **7 ok**, 2314 types, 0 mismatching, 0 contract, 2341 probed |
| nullrhi (4) | ok, startup 2.74 s | ok, startup 2.78 s | **4 ok**, startup 2.68 s, 0 criticals |
| d3d9 (9) | 9 ok, 2370 frames, startup 3.34 s | 1 FAIL: 390 frames, **startup 30.95 s** | **9 ok**, 2340 frames, startup 3.48 s, 6506 draw elements |
| inputtest (9) | 9 ok, moved 1025.7 | 1 FAIL: moved 780.9 | **9 ok**, moved 1031.3, 1369 physics actors |
| | 34 ok, 3 failed | 31 ok, 6 failed | **37 ok, 0 failed, 0 skipped, 872 s** |

The d3d9 and inputtest failures of run 2 are the machine, not the package, and the number that says so is
`d3d9_startup_seconds`: **3.34 s in run 1, 30.95 s in run 2, 3.48 s in run 3**, same exe all three times.
Seventeen `cl.exe` processes belonging to another agent were compiling through run 2; run 3 was launched the
minute that build finished. The baseline's own `_about` block records that the same run has measured 20,220
frames idle and 4,380 under load, and that its bounds exist to catch a counter going to zero — which is exactly
the trap: a loaded machine makes a load-sensitive bound look like a regression. **Read the load before
believing a d3d9 or inputtest failure**, and `d3d9_startup_seconds` is the number to read.
