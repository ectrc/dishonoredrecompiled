# Agent EN (PHASE13 EN) — the Ark component layer, and the two things that were keeping it out of the build

Worktree `build/agentEN_wt`, detached at `0cbe3fe`. Own build dirs `build/agentEN_release` (every restore
measurement) and `build/agentEN_clean` (the clean-build acceptance), own regression build dir
`build/agentEN_regwt`, own IDA copy `build/agentEN_ida/retail2013_agentEN.i64`, headless decompiles only, no IDA
MCP tools and no FModel tools. Nothing committed, nothing staged; nothing outside the worktree was edited.

## The answer in nine lines

* **`Dishonored0.sav` ran from 43,137 bytes to 97,422 of 619,631, and objects restored from 502 to 1,111.**
  Agent EJ's frontier — Ark component type 211, `DisCpntType_AIMonitorReaction`, inside
  `UArkComponentContainer::GameLoad` — is behind us, and with it the whole three-component container of all
  sixteen AI brains.
* **`Dishonored1.sav` moved too, for the first time since agent EF: 17,842 bytes to 18,660 of 294,961.**
  `URB_BodyInstance::GameLoad` (2013 `0x39e7f0`) is ported and its eighteen rigid-body states are read.
* **Three component classes, three creators and one archive helper.** `FDisAIMonitorReaction` (211),
  `FDisAIKnowledgeComponent` (210) and `FDisAIMonitorPawnReachability` (208) — the only three of retail's
  nineteen component types that read more than `FArkComponentBase::Serialize`, and all three of the ones the AI
  brain's container holds. Plus `FArkComponentBase::FGCHelper::manageReference`, which every component's
  `ManageReferences` stands on and which this tree did not have. **Ported override classes 55 → 56, unported
  57 → 56** (the one moved is `RB_BodyInstance`).
* **Two separate things were keeping those units out of the build, and the first one is a trap for every future
  package.** `Sources.cmake` is an *exclude* list — `DishonoredGame_EXCLUDE`, the 827 comment-only skeleton
  units — and all three component units were on it, so they were never compiled. That is what
  `gen_classes_header.py --sources-cmake` fixes (827 → 824). **And that was not enough**: this tree builds each
  module as a STATIC library, and a library member whose only externally visible effect is a dynamic
  initializer is never linked. Section 2 is the three measurements that separate the two causes.
* **Every restored value is checked against the save's own bytes and then against the running game**, which is
  the two-sided check this layer has earned. `build/agentEN/verify_components.py` walks the container in Python
  with the byte counts read off retail's disassembly; `build/agentEN/agentEN_trace16.log` confirms every one of
  its offsets from the other side, **including the two things the static decode could only infer**: that
  dictionary record 1067 is a `UDisAIBlackboard`, and that its nested `GameLoad` is exactly four bytes.
  Section 3.
* **The container record is 102 bytes, bytes 43,129..43,231, and the game agrees on both ends.**
  The property trace puts `DishonoredAIBrain.m_pBrainComponentContainer` at byte 43,127 and the brain's next
  property at byte **43,231**, which is exactly where the Python walk ends.
* **Two new frontiers, both named, and neither is a missing tail.** `Dishonored0.sav` now stops on
  `UDisConv_Soiree_InGameData` (2013 `0x8a9540`), an unported override — record 1814 of agent EJ's own
  hand-over list, and section 4 has its disassembly so the next agent does not need a run.
  `Dishonored1.sav` stops on `UStateNPCMasterDead_Limp::LoadPartialState`, reached through
  `UDishonoredNativeStateMachine::LoadPartialState` (`0x672670`), which *is* ported — the missing body is the
  state's, one of agent ED's nine.
* **Two of my own defects, both caught by a check I wrote for the purpose and both address mislabels** — the
  exact class of defect the brief warned about. `0x538c50` was a 2012→2013 address I *guessed* (the truth is
  `0x537ed0`), and `0x79c150` was a 2012 address copied out of a skeleton banner into a 2013 banner, where
  2013's own `0x79c150` is a different function. `rva_sweep.py` passed both. Section 7.
* **One correction to the brief.** The brief says `Dishonored1.sav` is "a transcription rather than an
  investigation": the wire format is exactly as it says (58 bytes per body, and I confirmed it independently),
  but the *object* it applies to does not exist at restore time — `USkeletalMeshComponent::PhysicsAssetInstance`
  is NULL when the level restore reads those eighteen states. Section 5.

## 1. What the stream stopped on, and the three classes behind it

Agent EJ left the stream inside `UArkComponentContainer::GameLoad` (2013 `0x534220`), on the first component of
`DishonoredNPCController_*.AIBrain.ComponentContainer`: Ark component type **211**. EJ proved that was a real
frontier and not a missing tail (81 bytes, the loop's exit label five epilogue instructions from `retn 8`), and
bounded the package by measurement: sixteen of retail's nineteen component types read exactly
`FArkComponentBase::Serialize`, and only three read more.

**I re-ran EJ's table rather than trusting the snapshot, and it reproduces exactly.**
`build/agentEN/probe1.py` reads every `FArk*`/`FDis*` component vftable out of retail and compares slot 5
against `0x532e20`; `build/agentEN/comp_vt.txt` is the output: **19 component vftables, 16 on the base body,
3 with their own**, and the three are the three the brief names —

| | class | slot 5 | vftable rva |
|---|---|---:|---:|
| OWN | `FDisAIKnowledgeComponent` | `0x7039f0` | `0xd328fc` |
| OWN | `FDisAIMonitorPawnReachability` | `0x73b010` | `0xd3a2e0` |
| OWN | `FDisAIMonitorReaction` | `0x73b1a0` | `0xd3a35c` |

One small thing the table shows that the brief does not: the `EArkComponentType` enumeration has **21** values
but only **19** component vftables answer the shape the probe looks for. The two missing are
`FArkComponentMeshOffset` and `FDisComponentAnimPlayer`, whose type virtuals the linker folded onto other
classes' (`FDisComponentAnimPlayer`'s `GetType` slot is `UDisBehaviorTriggerAlarm::CanBeDormant`,
`0x322980`, in its own secondary table). Neither is one of the three, so the bound holds.

**And the AI brain's container holds all three of them, and nothing else.** That is the single most useful fact
of this package: the container's three components are types 211, 210 and 208 — exactly the three classes with
their own serializer — so there was no way to reach past this gate without porting all three, and no need to
port any of the sixteen.

### 1.1 The three classes

All three derive from `FArkComponentBase` **and** from a second base at offset `0x10`,
`IArkComponentPreAsyncWorkJustBeforeProceduralAnim`. Its retail vftable (`0xc1bda4`) is exactly two
`__purecall` entries, and every component that implements it carries `PreAsyncWorkTick` at slot 0 and
`GetComponentBase` at slot 1 in that subobject's table — so the interface **adds no method of its own**; it is a
distinct type so `FArkComponentManager` can keep a second policy list
(`FArkComponentPolicy<IArkComponentPreAsyncWorkJustBeforeProceduralAnim,1>`, `DoneTicking` 2013 `0x537ed0`).
It is declared beside the `IArkComponentPreAsyncWork` it derives from, in
`DishonoredGame/Inc/arkcomponentlocomotion.h`.

| | retail sizeof | own span | serialised form |
|---|---:|---|---|
| `FDisAIKnowledgeComponent` | 28 | `m_pBlackboard` `0x14`, `m_pOwningBrain` `0x18` | base + two 2-byte references |
| `FDisAIMonitorReaction` | 64 | `m_pOwningBrain` `0x14`, ten `FResponseLogic_Base*` `0x18..0x3C` | base + one INT |
| `FDisAIMonitorPawnReachability` | 64 | eleven DWORDs, `0x14..0x3C` | base + 44 bytes |

Each size is retail's own arithmetic: `GetMemoryFootprint` is `GetAllocatedSize() + sizeof`, and the constant it
adds is `0x1C`, `0x40` and `0x40`. (The `[vptr+0x24]` it calls first is slot **9**, `GetAllocatedSize`, not
slot 6 — reading it as slot 6 would have made every one of these sizes wrong.)

The ten response-logic slots are retail's number three times over: `Init` (`0x735040`) loops
`while( id < 0xA )`, `DeleteLogicForAllResponseID` (`0x728af0`) and `GetAllocatedSize` (`0x728b30`) both loop ten
pointers from `+0x18`, and `CreateLogicForResponseID` (`0x72fa40`) is a switch over cases 0..9 storing into
`this+6` .. `this+15`. `FResponseLogic_Base` itself is a forward declaration here, because
`disaimonitorreaction_responses.cpp` (fourteen logic classes) is a comment-only skeleton — so the ten slots are
always NULL and the four retail bodies that walk them are not ported. They read no stream byte.

`FDisAIMonitorPawnReachability`'s eleven DWORDs are the one place a member could have been invented, so each
name is a body's:

```
+0x14  m_pOwningBrain              ctor 0;      Starting takes it from the controller's m_pAIBrain (@896)
+0x18  m_fCheckPeriod              ctor -1.0f;  reloaded into m_fTimeToNextCheck after a check  (PreAsyncWorkTick)
+0x1C  m_fReachableRecheckPeriod   ctor  1.0f;  reloaded into m_fTimeToNextRecheck when on-navmesh
+0x20  m_pMonitoredPawn            ctor 0;      ResetWithNewMonitoredPawn swaps it, ManageReferences as AActor*
+0x24  m_fTimeToNextCheck          ctor  FLT_MAX (0x7f7fc99e); Starting sets it to 0
+0x28  m_fTimeToNextRecheck        ctor  0.0f;   Starting sets it to m_fReachableRecheckPeriod
+0x2C  m_bNeedsMonitoredPawnReset  ctor  1;      ResetWithNewMonitoredPawn clears it
+0x30  m_eCachedReachability       ctor  0;      UpdateCachedReachability (0x747c10) writes it
+0x34  m_LastNavMeshCheckLocation  not initialised by the ctor; the check (0x746580) writes the FVector it
                                                path-found to, taken from the pawn's Location at +196..+204
```

`m_eCachedReachability` is the one member retail serialises **through an INT temp**
(`INT t = this[12]; ByteOrderSerialize(&t,4); this[12] = t`), which is what you write when the member is an
enumeration; `eDisReachability` already exists in this tree with exactly the four values the bodies use
(`Unknown` 0, `NotReachable` 1, `NotReachable_Below` 2, `Reachable` 3 — `PreAsyncWorkTick` answers 3 when the
navmesh check passes, else 1 or `1 + KnowPlayerIsWithinHideOut()`), so the port is the same three lines.

### 1.2 The archive helper the three stand on

`FArkComponentBase::FGCHelper::manageReference` (2013 `0x532de0` for `UObject*`, `0x533900` for `AActor*`) did
not exist in this tree, although `UArkComponentContainer::Serialize` and `::AddReferencedObjects` already built
an `FGCHelper` and handed it to every component. It is four lines:

```
if( m_pObjectArray ) { UObject::AddReferencedObject( *m_pObjectArray, p ); return p; }
else                 { *m_pArchive << p; return p; }          // FArchive slot 6, operator<<(UObject*&)
```

so with the object array it keeps the referent reachable for the collector, and with the archive it serialises
the reference — which is how a component's object reference survives a container `Serialize` at all. Retail has
two private overloads; the only difference between them is that the `AActor*` one uses
`UObject::_AddReferencedObjectNullable`, a retail-only private static this tree does not have, so this is one
template. Without it, the blackboard the save restores into `FDisAIKnowledgeComponent::m_pBlackboard` would have
had no reference keeping it alive.

### 1.3 One accessor that was answering NULL and now answers

`UDishonoredAIBrain::GetKnowledge` (2013 `0x711ce0`) is one call in retail —
`m_pBrainComponentContainer->GetFirstComponent<FDisAIKnowledgeComponent>( TRUE )` — and it returned NULL here
with a comment saying `FDisAIKnowledgeComponent` is unported. It now answers the component the container holds.
Retail returns a reference and dereferences the container with no test; this keeps the NULL-container guard.

## 2. The two reasons the classes were not in the build, measured apart

This is the part of the package worth reading if you are writing the next one.

**Run 1 — the classes existed, compiled clean, and changed nothing.** 502 objects, 43,137 bytes, and
`UArkComponentContainer::AddNewComponentByID: no component type is registered for id 211`, i.e. exactly agent
EJ's number and exactly EJ's stop. That run is a genuine baseline for this package, taken with *this* build
(`build/agentEN/build0.log`, `agentEN_final16.log` of the first run), and it reproduces HEAD `0cbe3fe` to the
byte.

**Cause 1: `Sources.cmake` is an exclude list, and the three units were on it.**
`cmake/DishonoredModule.cmake` globs `Src/**/*.cpp` and then removes `<Module>_EXCLUDE`, which
`Sources.cmake` sets to the 827 comment-only skeleton units of `import_reference.py`. All three component
units were listed, so none of them was ever compiled — the files existed, the code was right, and ninja never
touched them. `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` takes them off
(**827 → 824**) because it decides from the file's content, and that one diff is the *whole* content diff the
generator produced. I had spent a build diagnosing this as a linker problem before reading
`DishonoredModule.cmake`, which is the mistake; the exclude list is not mentioned anywhere in the brief and it
will catch the next agent who ports a skeleton unit.

**Cause 2: and that was not enough.** With the three units compiling (their `.obj` files in
`DishonoredGameModule.lib`, `build/agentEN/build3.log` lines 1430 and 1434) the restore *still* said
"no component type is registered for id 211", and the image still contained
`ArkCpntType_Locomotion` once and `DisCpntType_AIMonitorReaction` **zero** times. Each module here is a STATIC
library, MSVC's linker takes a library member only to resolve an undefined symbol, and a dynamic initializer is
not a symbol reference — so a component unit nothing else calls into is dropped whole. Retail does not have the
problem because UnrealBuildTool hands the linker the module's `.obj` files directly.
`ARKCOMPONENT_LINK_TYPE( C )` in `Engine/Inc/arkcomponentbase.h` is the fix: an `extern` declaration of the
registrant and one pointer that names it, used three times in
`DishonoredGame/Src/arkcomponentlocomotion.cpp`, a unit the link always pulls in (the locomotion component's
`PreAsyncWorkTick` is called from `dishonorednpcpawn_locomotion.cpp`). The registrants themselves stay in their
own components' units, which is where retail has them.

**Cause 2b, because the first attempt at the fix silently did nothing.** Written the way a pointer at namespace
scope wants to be written — `static FArkComponentCreatorRegister* const` — the compiler drops an unreferenced
internal-linkage constant before the linker ever sees it, and nothing changed. Without `const` it is a
definition the compiler must emit, its relocation names the registrant, and the library member comes in with
it. All four component strings are in the image after that (`build/agentEN/build4.log`).

## 3. The measurement, and every restored value against the save's own bytes

`build/agentEN_release` (configured `-S build/agentEN_wt`), `-disrestoreslot=16` and `-disrestoreslot=17`, null
RHI, `build/agentEN/run_restore.py`. Logs: `agentEN_final16.log`, `agentEN_final17.log`, `agentEN_trace16.log`
(the last with `-disstreamdebug=6000 -dispropertytrace`).

| | HEAD `0cbe3fe` (= agent EJ) | agent EN |
|---|---:|---:|
| **Dishonored0.sav ("0 - Dunwall Tower")** | | |
| dictionary records read | 11321/11324 | **11321/11324** |
| dictionary bytes | 91844/91844 | **91844/91844** |
| records resolved | 6227 | **6227** |
| objects restored | 502 | **1111** |
| object-stream bytes | 43137/619631 | **97422/619631** |
| skipped | 1452 | **3532** |
| untrusted skips | 206 | **411** |
| unported / partial bodies | 0 / 1 | **1 / 0** |
| PostGameLoad run | 502 | **1111** |
| stopped on | inside `UArkComponentContainer::GameLoad`, Ark component type 211 | **`DisConv_Soiree_InGameData_8`, an unported override** |
| **Dishonored1.sav ("1 - Dunwall Sewers")** | | |
| dictionary records / bytes | 6433/6436, 52672/52672 | **unchanged** |
| objects restored | 180 | **180** |
| object-stream bytes | 17842/294961 | **18660/294961** |
| stopped on | `DisLoadPhysicsAssetInstanceBodies`, 18 rigid bodies | **`StateNPCMasterDead_Limp::LoadPartialState`** |
| ported override classes | 55 (unported 57) | **56** (unported 56) |

The census line in full, slot 16:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 1111 restored, 3532 skipped, 97422/619631 bytes; 50 spawned, 5093 not found, 1 unported, 0 unresolved,
411 untrusted skips, 0 partial bodies, 1111 PostGameLoad; STREAM ABORTED
```

`1 partial bodies` → `0` and `0 unported` → `1` together say the shape of the frontier changed: the stop is no
longer *inside* a ported body, it is a class whose `GameLoad` this tree has not written. `1111 PostGameLoad`
tracks `1111 restored` exactly.

**`Dishonored1.sav`'s object count did not move and that is the honest number.** The 818 extra bytes are all
inside one object's body — the eighteen rigid-body states of one ragdolled NPC's skeletal mesh, plus the two
bytes of the next reference — and the next object is behind the state machine's frontier.

### 3.1 The container record, byte for byte, from the file

`build/agentEN/verify_components.py` decompresses the persistent level state's object blob in Python and reads
bytes; it does not run the game and does not use the C++ reader. Output:
`build/agentEN/verify_components.txt`. The only offset it is *given* is 43,129, and that comes from the running
game's own stream log (`DisStream 1954: byte 43129 ref 1066 -> ...AIBrain.ComponentContainer`). Everything else
is the byte counts read off retail's disassembly plus arithmetic:

```
byte 43129  03 00 00 00        m_Components.Num() = 3            (those 8 bytes occur 16x in 619631 - one per brain)
byte 43133  d3 00 00 00        component 0: type 211  AIMonitorReaction
      43137  ca 02 01 00 00 00 00 00 00 00   base: owner record 714, m_bStarted 1, m_bPendingStop 0
      43147  01 00 00 00                     policy membership = 1
byte 43151  d2 00 00 00        component 1: type 210  AIKnowledge
      43155  ca 02 01 00 00 00 00 00 00 00   base
      43165  2b 04                           m_pBlackboard   = record 1067
      43167  00 00 00 00                     ... UDisAIBlackboard::GameLoad, inline: record count 0
      43171  cc 02                           m_pOwningBrain  = record 716
byte 43173  d0 00 00 00        component 2: type 208  AIMonitorPawnReachability
      43177  ca 02 01 00 00 00 00 00 00 00   base
      43187  cc 02        m_pOwningBrain             record 716
      43189  00 00 00 3f  m_fCheckPeriod             0.5
      43193  00 00 80 3f  m_fReachableRecheckPeriod  1          (= the constructor's default)
      43197  00 00        m_pMonitoredPawn           record 0 (none)
      43199  00 00 00 00  m_fTimeToNextCheck         0          (= what Starting sets it to)
      43203  00 00 80 3f  m_fTimeToNextRecheck       1          (= m_fReachableRecheckPeriod, as Starting does)
      43207  01 00 00 00  m_bNeedsMonitoredPawnReset 1          (= the constructor's default)
      43211  00 00 00 00  m_eCachedReachability      0          (eDisReachability_Unknown)
      43215  06 00 00 00 / ff ff ff ff / 04 00 00 00            m_LastNavMeshCheckLocation
      43227  01 00 00 00  policy membership          1
the container record is bytes 43129..43231, 102 bytes, and occurs once in the blob
```

Three things in that make it a check rather than a reading:

* **every step lands on a legal `EArkComponentType`.** A wrong byte count anywhere would put the next `INT` in
  the middle of a value, and there are only 21 legal ids out of 2^32.
* **five of the eight restored values are the value the class's own constructor or `Starting` would produce,
  and they are consistent with each other.** `m_pMonitoredPawn` is record 0, and `m_bNeedsMonitoredPawnReset`
  is 1 and `m_eCachedReachability` is 0 — which is precisely the state of a reachability monitor that has never
  been given a pawn. If the field order were wrong, that agreement would not survive.
* **`m_LastNavMeshCheckLocation` is not a plausible FVector** (`6`, `-1`, `4` as raw words), and that is
  *also* the expected answer: it is the one member retail's constructor leaves uninitialised, the check that
  would write it has never run on this component (no monitored pawn), so the save recorded whatever was in that
  memory. A field that looked like a tidy vector there would have been the surprise.

### 3.2 The same offsets from the running game, including the two the file could not prove

`build/agentEN/agentEN_trace16.log`. The stream log resolves every reference:

```
byte 43129 ref 1066 -> ...DishonoredNPCController_0.AIBrain.ComponentContainer
byte 43139 ref  714 -> ...DishonoredNPCController_0                   component 0 m_pOwner
byte 43157 ref  714 -> ...DishonoredNPCController_0                   component 1 m_pOwner
byte 43167 ref 1067 -> ...DishonoredNPCController_0.DisAIBlackboard_0 component 1 m_pBlackboard
byte 43173 ref  716 -> ...DishonoredNPCController_0.AIBrain           component 1 m_pOwningBrain
byte 43179 ref  714 -> ...DishonoredNPCController_0                   component 2 m_pOwner
byte 43189 ref  716 -> ...DishonoredNPCController_0.AIBrain           component 2 m_pOwningBrain
```

and the property trace closes both ends:

```
DisBin byte 43127  DishonoredAIBrain.m_pBrainComponentContainer (ComponentProperty)
DisBin byte 43167  DisAIBlackboard.m_Records (ArrayProperty) skipped      ... and every UObject property skipped
DisBin byte 43231  DishonoredAIBrain.m_MandatoryBehaviors (ArrayProperty) skipped
```

Every offset the Python walk predicted is there, and two of them were inferences the file alone could not
settle:

* **record 1067 really is a `UDisAIBlackboard`** — which is what makes `m_pBlackboard` the first of
  `FDisAIKnowledgeComponent`'s two references rather than a guess about the order.
* **the four bytes at 43,167 really are `UDisAIBlackboard::GameLoad` running inline.**
  `DisSaveLoad::FLevelLoader::operator<<(UObject*&)` (`0x60c930`) reads a WORD and then, unless bit `0x8000` is
  set and unless the object is already loaded, runs the referenced object's whole `GameLoad` at that point; the
  blackboard's is agent EC's `DisSaveLoadObject` plus an INT record count, the property walk skips every
  property of this object, and the count is 0. That is the entire four bytes, and the next reference at 43,171
  proves it.
* and the brain's next property at **43,231** is exactly where the 102-byte walk ends.

One incidental thing worth recording: agent EJ's log names this container's owner
`DishonoredNPCController_26` and mine names it `DishonoredNPCController_0`, on the same dictionary records
(714, 716, 1066, 1067) and the same bytes. The instance suffix depends on spawn order within the session; the
records do not. Nobody should read a changed `_<n>` as a changed save.

### 3.3 The eighteen rigid bodies of `Dishonored1.sav`

The offset again comes from a census and not from this script: the run before this package stopped having read
**17,842** bytes and what it had just read was `DisLoadPhysicsAssetInstanceBodies`' BYTE count, so the count is
byte 17,841 and the states start at 17,842.

```
byte 17841  12        BYTE NumBodies = 18
14 of the 18 bodies carry a 57-byte state, so the loop reads 18 + 14 x 57 = 816 bytes and ends at byte 18658
```

and the census after the port reads **18,660** — two more, which is the 2-byte object reference
`UDishonoredNativeStateMachine::LoadPartialState` reads before it stops on the state it names. That is an exact
close: the 58-byte-per-body format (1 byte, and 57 more when that byte is set) is confirmed against the file,
and the count is confirmed twice.

## 4. The two new frontiers, and the evidence

### 4.1 `Dishonored0.sav`: `UDisConv_Soiree_InGameData::GameLoad`, 2013 `0x8a9540`, vtable slot 70

The stop is at
`...DishonoredNPCPawn_37.pNPCConvComponent.DisDialogTree_InGameBind_24.DisConversation_InGameData_81.DisConv_Soiree_InGameData_8`,
and the census says `1 unported, 0 partial bodies` — the class-list guard fired before the body read anything,
which is the definition of a frontier that is not a missing tail: there is no body of ours to have a tail.
It is **record 1814 of agent EJ's own list of nine**, first of the nine, so the gate opened exactly onto the
hand-over EJ wrote.

Its 115 bytes, so the next agent does not need a run:

```
008a954f  call UDisAttentionInfo_Base::GameSave     ; an ICF fold - the plain DisSaveLoadObject walk (0x88af60),
                                                   ;   which this tree already has on eighteen classes
008a9556  mov edx,[eax+18h] ... call edx           ; FArchive slot 6, operator<<(UObject*&): a node-array owner
008a9577  call FArchive::ByteOrderSerialize( ,4 )   ; an INT node index, initialised to -1
008a957c  cmp [esi+3Ch],0 / jz                     ; if m_pSeqActInterp and the owner and index != -1:
008a9594  imul eax,4Ch / lea ecx,[eax+ecx+48h]     ;   the node is owner + 76*index + 72
008a959b  mov [edx+1D4h],ecx                       ;   stored on the USeqAct_Interp
008a95a8  call USeqAct_Interp::SetConversationNode
008a95b0  retn 8
```

Six bytes on the wire past the base walk, and the tail reads none of them. `USeqAct_Interp::SetConversationNode`
and the member at `USeqAct_Interp+0x1D4` do not exist in this tree, which is why I did not land it inside this
package — it is the conversation system, not the component layer.

### 4.2 `Dishonored1.sav`: `UStateNPCMasterDead_Limp::LoadPartialState`

`UDishonoredNativeStateMachine::LoadPartialState` (`0x672670`) **is** ported, by agent ED: it reads the state's
class and then calls that state's own `LoadPartialState`. The save names `StateNPCMasterDead_Limp`, and that
state's body is one of retail's nine real ones that this tree does not have — agent ED's own list, in
`dissavegame.cpp` beside the body. So this frontier is a missing leaf under a ported body, not a missing tail
either.

## 5. One correction to the brief, and what the rigid bodies cost

The brief says `Dishonored1.sav` is now "a transcription rather than an investigation". The **format** is exactly
what it says, and I confirmed it independently from retail's own instructions rather than taking it:

```
BYTE bHasBody;   Ar.Serialize( &bHasBody, 1 );
if( bHasBody ) {
    FBoneAtom BoneAtom;  Ar << BoneAtom;           // 32 bytes: two FVector4s, ENABLE_VECTORIZED_FBONEATOM is on here
    FLOAT LinVel[3];     three ByteOrderSerialize( , 4 )
    FLOAT AngVel[3];     three ByteOrderSerialize( , 4 )
    BYTE  Flags;         Ar.Serialize( &Flags, 1 );
    ... PhysX ...
}
```

58 bytes, and `GameSave` (`0x3a8620`) mirrors it byte for byte and builds the atom from this object's own
`FQuat` at 160 (`m_CurrentRotation`) and `FVector` at 176 (`m_CurrentPosition`) with W forced to 1 — two 16-byte
registers, no scale, which is what `Core/Inc/FBoneAtomVectorized.h`'s `operator<<` writes.

**What the brief does not say is that the object the state applies to does not exist yet.** Measured:
`USkeletalMeshComponent::PhysicsAssetInstance` is NULL at the point the level restore reads those eighteen
states (the first run with this body reported "the save has 18 rigid-body states and this session's physics
asset instance has -1 bodies"). The articulated instance is created later than the restore. So the states are
read through `URB_BodyInstance::SerializeSavedState` whether or not there is a body to put them in, and only the
assignment is skipped — the byte count of a saved body state depends on nothing but its own leading
`bHasBody`, which is what makes that safe.

**And the PhysX tail is not applied at all**, on any path, for three reasons in order of weight:

1. placing eighteen rigid bodies and waking or sleeping them during a level restore changes the physics scene,
   and this package has no measurement of what that does to it. The regression's `d3d9` and `inputtest` stages
   both count physics actors, and a package that moves those numbers cannot tell a real regression from its own
   work.
2. the two bits retail takes out of `Flags` go into the bitfield at `URB_BodyInstance+104`, which holds five
   one-bit members here (`m_bFrozen`, `m_bWakeUpWhenUnfrozen`, `m_bIgnoreNextWakeupEvent`, `m_bIsPending`,
   `m_bSeveredLimb`) and retail's masks `0x1000`/`0x2000`/`0x4000` cannot be matched to names from the
   disassembly alone. Guessing them is inventing.
3. the part that *is* evidenced is the pose: retail's `GameSave` reads those 32 bytes out of
   `m_CurrentRotation` and `m_CurrentPosition`, so writing them back is a transcription, and that is what
   `URB_BodyInstance::GameLoad` does. The velocities are read and not stored, because `GameSave` takes them
   from PhysX (`GetUnrealWorldVelocity` / `GetUnrealWorldAngularVelocity`) and not from
   `m_RealLinearVelocity` / `m_RealAngularVelocity`, so storing them there would be a guess about which member
   they are.

So: **a ragdolled corpse in `Dishonored1.sav` still does not come back in its saved pose.** Every object behind
it does.

## 6. What this package ports that neither save exercises

Said plainly, because a body that has never run is not a body that has been shown to work.

* **`FArkComponentBase::Serialize` is now exercised** — agent EJ's hand-over 3 said it never had been, and all
  three components call it, and the slot-16 run reads all sixteen brain containers (the container's first eight
  bytes occur sixteen times in the blob and the stop is 54,000 bytes past the last of them), so it ran 48 times.
* **`FDisAIKnowledgeComponent::GetMemoryFootprint`, `FDisAIMonitorReaction::GetMemoryFootprint` and
  `FDisAIMonitorPawnReachability::GetMemoryFootprint` have never run.** Nothing in this tree asks a container
  for its footprint outside a counting archive, and no counting archive runs in a restore.
* **`FArkComponentBase::FGCHelper::manageReference`'s object-array branch has never run.** The archive branch is
  exercised the moment anything serialises a container; the collector branch needs a garbage collection with an
  `UArkComponentContainer` in the reachable set, which no measurement here covers.
* **`URB_BodyInstance::GameLoad`'s assignment to `m_CurrentRotation`/`m_CurrentPosition` has never run**, for
  the reason in section 5: there was no body on either save. Its *reading* half ran 18 times, through
  `SerializeSavedState`.
* **`UDishonoredAIBrain::GetKnowledge` now returns non-NULL but nothing calls it yet**: its five retail callers
  are the reachability monitor's static knowledge queries, none of which is ported.

## 7. Deviations and defects, stated plainly

1. **Two defects of my own, both address mislabels, both caught by a check written for the purpose, and
   `rva_sweep.py` passed both.**
   `build/agentEN/addr_audit.py` resolves all 70 addresses this package cites against `functions_2013.csv` **by
   name**, and found `0x538c50` — a 2012→2013 address I had *guessed* by changing a digit instead of resolving
   it. The truth is `0x537ed0`, from `match_2012_2013.csv` (ratio 0.786, by neighbours), and it is unnamed in
   2013.
   `build/agentEN/banner_check.txt` does the same to every `//   0x<addr>  <signature>` banner line in the six
   files this package owns, and found `0x79c150` copied out of the `import_reference.py` skeleton's 2012 banner
   into a banner whose other lines are 2013 addresses. `FDisAIMonitorReaction::CoS_OnTransgression` is 2013
   `0x73eab0`; **2013's own `0x79c150` is `UDisSeqAct_BodyShadowKill::Activated`**, a different function, which
   is exactly why an address that lands inside *some* function is not enough. Both are fixed and both checks
   now report clean.
2. **`ARKCOMPONENT_LINK_TYPE` is a feature retail does not have.** It exists because this tree's link model is
   not retail's (static libraries against `.obj` files), it is three lines of declaration, and section 2 is the
   measurement that says the package does not work without it. It is the only mechanism this package adds.
3. **`FArkComponentBase::FGCHelper::manageReference` is one template where retail has two private overloads.**
   They differ only in a retail-only private static (`UObject::_AddReferencedObjectNullable`), and the
   collector's job here — keeping the referent reachable — is what `AddReferencedObject` does in either.
4. **`FDisAIKnowledgeComponent`'s constructor zeroes `m_pOwningBrain` where retail leaves it uninitialised**,
   and `FDisAIMonitorPawnReachability`'s zeroes `m_LastNavMeshCheckLocation` where retail leaves it
   uninitialised. Retail gets away with both because `Starting` and the reachability check write them before
   anything reads them; neither is ported here, so a wild pointer in an object the container's GC walk visits is
   the alternative.
5. **The policy-membership INT of `FDisAIMonitorReaction::Serialize` and
   `FDisAIMonitorPawnReachability::Serialize` is read and discarded.** Retail asks the world's
   `FArkComponentManager` whether the component is on the `JustBeforeProceduralAnim` policy's list and, on a
   load, re-registers it when the answer was TRUE. `FArkComponentManager` is not ported
   (`Engine/Src/arkcomponentmanager.cpp` is a skeleton and `UWorld::m_pComponentManager` is never assigned), so
   there is no list to ask and none to join. The save says TRUE for both components of every brain.
6. **`PreAsyncWorkTick` is an empty body on all three classes.** The interface declares it pure so a body has to
   exist; retail's are the components' entire per-frame behaviour and all three need machinery this tree does
   not have (the fourteen response-logic classes, the blackboard record classes,
   `FDisAttentionProxy::GetProxyLocation`). `FDisAIKnowledgeComponent`'s is empty in retail too — the linker
   folded its slot 0 onto `APawn::MAT_BlendOut`.
7. **`Starting` and `Stopping` are not ported on any of the three.** Every one of the six either registers with
   the absent component manager, constructs and roots a `UDisAIBlackboard` on a path no measurement here covers,
   or subscribes an unported handler to the game-event dispatcher. None reads a stream byte.
   `FDisAIMonitorPawnReachability::ms_ComponentCount` is declared, because `Serialize`'s own tail increments it
   for a component the save says was already started, and that tail *is* ported; nothing reads the counter yet.
8. **`GameSave` is left out everywhere**, for agent ED's reason: the writing half of the object layer does not
   exist here. Four of them in this package (`0x3a8620` and the three components' mirrors of `Serialize`, which
   retail folds onto the loading halves).
9. **`DisLoadPhysicsAssetInstanceBodies` no longer stops when the body array is missing**, where retail
   dereferences `PhysicsAssetInstance` and indexes `Bodies` with no test. It warns once and reads the states
   without applying them. Section 5 is why that is the right trade and what it costs.
10. **`URB_BodyInstance::GameLoad`'s PhysX tail is not applied.** Section 5, three reasons.
11. **`Engine/Inc/EnginePhysicsClasses.h` is a generated header and this edits it by hand.** That is this
    tree's own convention for Engine save overrides — `EngineClasses.h:2510` carries `AActor::GameSave` and
    `GameLoad` the same way, and Engine has no `CppText` directory. The brief only requires the *DishonoredGame*
    generator to be run, and running it is what produced this package's `Sources.cmake` diff.

## 8. Hand-overs

1. **`UDisConv_Soiree_InGameData::GameLoad` (`0x8a9540`) is `Dishonored0.sav`'s whole distance now**, and
   section 4.1 has its disassembly. What it needs beyond the six wire bytes:
   `USeqAct_Interp::SetConversationNode` and the `USeqAct_Interp` member at `+0x1D4`, neither of which exists
   here. Behind it are the other **eight** of agent EJ's nine, all under 170 bytes; EJ's section 4 still lists
   them with retail addresses, and `build/agentEJ/frontier.py` regenerates that list. Note `USeqAct_Interp`
   appears in both places, which suggests doing the two together.
2. **`Dishonored1.sav` now needs `UStateNPCMasterDead_Limp::LoadPartialState`**, one of the nine state bodies
   agent ED listed beside `UDishonoredNativeStateMachine::LoadPartialState` in `dissavegame.cpp`. That is a
   leaf under a ported body, so it is the smallest next step on that save.
3. **`Sources.cmake` is an exclude list.** Porting a `resources/tools/import_reference.py` skeleton unit does
   nothing at all until `gen_classes_header.py <Module> --sdk --module-header --sources-cmake` takes it off
   `<Module>_EXCLUDE`. That cost this package a build and a game run, it is not written down anywhere else, and
   824 units are still on the list.
4. **A static-library member that holds nothing but a registrant is never linked.** `ARKCOMPONENT_LINK_TYPE`
   handles the Ark component creators; any other registry in this tree that relies on a file-scope dynamic
   initializer in a leaf translation unit has the same hole, and `UDisStimManager`'s 111-row stim registry is
   the one to check first. Note also that the reference must have *external* linkage or the compiler removes it
   before the linker is involved.
5. **The sixteen component types with no class are now the cheap part, and they are needed.** All three
   components the AI brain's container holds are ported, but a pawn's container holds others (types 0 and
   101..105, and 200..214 for the Dis ones), and the first save object that reaches one will stop on
   `AddNewComponentByID` with the same warning this package started from. Each of the sixteen is
   `ARKCOMPONENT_DECLARE_TYPE` + `ARKCOMPONENT_IMPLEMENT_TYPE` + `ARKCOMPONENT_LINK_TYPE` + whatever members its
   retail constructor writes; none needs a serializer. `build/agentEN/comp_vt.txt` is the table.
6. **Agent EK's open question is still open, and this package did not cross it.** `-disrestoreslot=16/17`
   reporting "0 level state(s) in the save" on `L_Tower_P` and `DishonoredGameFull_P` is not something any run
   here reproduced: every run of this package on slot 16 found `1 level(s)` in `DishonoredGameFull_P` and
   restored from it. One fact that may be the answer and that this package did measure:
   **`Dishonored1.sav` has two level states, `L_Prison_P` (575,391 bytes) and `DishonoredGameFull_P` (294,961),
   and `Dishonored0.sav` has exactly one, `DishonoredGameFull_P` (619,631).** Anything that picks a level state
   by index rather than by name will read the wrong one of the two on slot 17, and `L_Prison_P` is the larger.
7. **Agent EF's hand-over 5 (the `gen_classes_header.py` inner-type defect) and hand-over 6
   (`SpawnInventoryLoadout_Items`) are untouched and still stand.**

## 9. Merging

**Regeneration is required, and I ran it in the worktree.**

```
python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake
```

with all three flags. **Its whole content diff is one file**: `Sources.cmake`, where the three component units
come off `DishonoredGame_EXCLUDE` and the header comment goes from 827 skeleton units to 824. It rewrites about
seventy-five other generated headers and `DishonoredGameRegistrants.cpp`, and `git diff` reports **no content
change** in any of them. Its run reports 1823 classes, 0 pending layout checks, 9201 members checked against the
SDK with 0 differing.

**`Sources.cmake` DOES change**, which is the opposite of agent EJ's answer and the reason is section 2: this
package is the first to port comment-only skeleton units rather than add bodies to units that already compiled.

`dissaveload_classlists.h` is regenerated by `build/agentEN/gen_classlists.py` (agent EJ's, retargeted at this
worktree, with `RB_BodyInstance` added to `PORTED`): **562 entry classes, 56 ported, 56 unported**, and its
ICF-fold check reports **0**. `build/agentEN/make_foldselftest.py` shows the check can fire: with agent EC's
three hand-declared fold roots removed it reports **2 ported bodies reached by 32 classes that do not inherit
them** (`DisAttentionInfo_Base`, slots 68 and 69, 16 classes each).

**Fifteen source files and two documents. Three of the fifteen are outside `DishonoredGame`:**

| file | why |
|---|---|
| `Engine/Inc/arkcomponentbase.h` | `FGCHelper::manageReference`, `ARKCOMPONENT_LINK_TYPE`, and the registrant object made external so it can be named |
| `Engine/Inc/EnginePhysicsClasses.h` | `URB_BodyInstance::GameLoad` + `SerializeSavedState`, declared the way every Engine save override in this tree is |
| `Engine/Src/UnPhysAsset.cpp` | those two bodies, in retail's own unit |
| `resources/docs/agents/agentEN.md` | this report |
| `resources/docs/agents/agentEN_status.csv` | 66 rows, 34 ported / 32 not, with both builds' addresses |

and twelve inside it: `Inc/arkcomponentlocomotion.h` (the second-base interface),
`Inc/{disaiknowledgecomponent,disaimonitoractorreachability,disaimonitorreaction}.h` and the three matching
`Src/*.cpp` (all six were `import_reference.py` skeletons), `Inc/dissaveload_classlists.h` (regenerated),
`Sources.cmake` (generated), `Src/arkcomponentlocomotion.cpp` (the three `ARKCOMPONENT_LINK_TYPE`),
`Src/dishonoredaibrain.cpp` (`GetKnowledge`) and `Src/dissavegame.cpp`
(`DisLoadPhysicsAssetInstanceBodies`).

`build/agentEN_sync.py` is the authoritative list and performs the copy (`--apply`, **worktree → main**).
Nothing is committed and nothing is staged.

## 10. Verification

* **Accept 1 — the census on both saves**: section 3. `build/agentEN/agentEN_final16.log`,
  `agentEN_final17.log`, `agentEN_trace16.log`. The baseline is this package's own first run, which reproduced
  502 objects / 43,137 bytes with this build before the registrants were linked, and it matches agent EJ's
  `agentEJ_final16.log` exactly.
* **Accept 2 — every newly restored value against the save's own bytes**: sections 3.1 to 3.3,
  `build/agentEN/verify_components.py` and `verify_components.txt`, cross-checked against the running game's
  own stream and property traces. What is *not* exercised is section 6.
* **Accept 3 — the new stopping points and the evidence they are frontiers**: section 4, with the disassembly
  of `0x8a9540` to its `retn 8` for one and the class-list guard's own `1 unported, 0 partial bodies` for both.
* **Accept 4 — regression**: `python build/agentEN_wt/resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEN_regwt --exe-name DishonoredGame_ENR.exe --log-prefix
  agentEN_reg`, the worktree's own copy with an absolute build dir and the build stage included — see section 11
  for the result. The five gitignored layout inputs were copied into
  `build/agentEN_wt/resources/docs/types/` first, as agent EJ's fact 2 requires; they do not dirty the
  worktree's `git status`.
* **Accept 5 — clean full release build**: `build/agentEN_clean_build.cmd` — `build/agentEN_clean` deleted
  first, the worktree as source, `DISHONORED_LAYOUT_CHECKS=ON`, all three targets: **977 ninja edges,
  0 errors, 0 C4263, 0 C4264** (`build/agentEN/cleanbuild.log`).
* **Accept 6 — `rva_sweep.py` and the fold check**: of the **6,989** citations in the worktree, 2 are suspect
  and both are pre-existing in another agent's file (`GFxUI/Src/gfxuirenderer.cpp:1669`, in a block labelled
  "2012" — agent EJ reported the same two). Of the **467** in the fourteen files this package touches,
  **0 are suspect** (`build/agentEN/sweep_mine.py`): 273 `ok-2013`, 194 `ok-2013-mid`. And because that is
  necessary and not sufficient, `build/agentEN/addr_audit.py` resolves all **70** addresses this package cites
  by *name*: 61 matched, 9 deliberately unnamed (each says so in the comment), **0 wrong** — after the two
  defects of section 7.1. The fold check reports 0 and `make_foldselftest.py` proves it can report 2/32.
* **Layouts** — unchanged. This package adds no reflected member and changes no `UObject`'s size. The three
  component classes are not `UObject`s; the two Engine declarations are virtuals and a static; the one
  `URB_BodyInstance` change is two member functions.
* **Which binary measured what.** Every restore number comes from `build/agentEN_release`, configured
  `-S build/agentEN_wt`, rebuilt after the last source edit before the run (`build/agentEN/build6.log`, and
  the trace run and both final census runs came after it). The only source edits after those runs were the two
  address corrections of section 7.1, which are comment lines; both the clean build (`build/agentEN_clean`,
  deleted and reconfigured again afterwards) and the regression's own build stage compiled the tree with them
  in, each with 0 errors. So no measurement here was taken against a tree holding another agent's work, and the
  only delta between the measured binary and the final worktree is two comments.
* **The dictionary halves are still exact**: 11321/11324 records and 91844/91844 bytes on slot 16,
  6433/6436 and 52672/52672 on slot 17, in every run.

## 11. The regression

`resources/tools/run_regression.py`, **37** checks over six stages, run from the worktree's own copy with an
absolute `--build-dir` and no `--no-build`, so all six build checks count. Agent EJ's two facts about running
the harness against a snapshot worktree both held and both were followed: the harness has to be **the
worktree's copy** (`build-release.cmd` does `cd /d "%~dp0.."` then `cmake -S .`, so it always configures the
tree the harness lives in), and the five gitignored layout inputs
(`resources/docs/types/{all_types.h, retail_sdk_layout.json, script_classes_2012.json,
script_classes_2013.json, types.json}`) have to be copied into the worktree or every layout metric records -1.

**37 ok, 0 failed, 0 skipped, 871 s** (`build/agentEN_regwt/regression/summary.txt`, transcript
`build/agentEN/regression.txt`), first attempt, no retries:

| stage | checks | the numbers that matter |
|---|---:|---|
| build | 6 | all three targets exit 0 with 0 errors |
| coresmoke | 2 | 99 passed, 0 failed |
| layout | 7 | 2314 types, **0 mismatching**, **0 contract**, 2341 probed, 0 compare-contract |
| nullrhi | 4 | startup 2.7 s, 0 criticals, 7245 log lines |
| d3d9 | 9 | startup 3.3 s, 2430 frames, 6506 draw elements, 457 visible prims, 0 unported natives |
| inputtest | 9 | moved 1020.7, peak speed 500.0, **1369 physics actors**, 1312 static shapes, 6 probe natives |

Two of those are worth naming because this package touched their subject matter. **`physics_actors` is 1369,
the same as agent EJ's run** — which is the number that would have moved if `URB_BodyInstance::GameLoad` had
started placing and waking rigid bodies, and section 5 is why it does not. And **`layout_contract` and
`layout_mismatching` are both 0** with 2314 types, which is the check that the three new non-`UObject` classes
and the two Engine declarations changed no reflected layout.

