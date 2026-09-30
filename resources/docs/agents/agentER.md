# Agent ER (PHASE14 ER) — the conversation gate, and the thing the gate could not see

Worktree `build/agentER_wt`, detached at `2d08c15`. Own build dirs `build/agentER_release` (every restore
measurement), `build/agentER_clean` (the clean-build acceptance) and `build/agentER_regwt` (the regression),
own IDA copy `build/agentER_ida/retail2013_agentER.i64`, headless decompiles only, no IDA MCP tools and no
FModel tools. Nothing committed, nothing staged; nothing outside the worktree was edited.

## The answer in ten lines

* **`Dishonored0.sav` ran from 97,422 bytes to 537,208 of 619,631, and objects restored from 1,111 to 6,569.**
  `PostGameLoad` 1,111 → 6,569. That is 86.7% of the persistent level's object blob, up from 15.7%.
* **`Dishonored1.sav` ran from 18,660 to 21,081 of 294,961, and 180 objects to 183.** Its frontier is a named
  class again (`ADisPickup_Base::GameLoad`) rather than a partial-state gate.
* **Both of the brief's two frontiers were smaller than the brief says, and one of them was not there at all.**
  There is no `UStateNPCMasterDead_Limp::LoadPartialState` in retail — not in `functions_2013.csv`, not in the
  2013 image's own vftable. What the save reaches is `UStateNPCInstigatedMasterAction::LoadPartialState`
  (2013 `0x664c60`), **23 bytes**, one object reference. And the conversation system is not a subsystem: both
  members the brief says do not exist already exist, and what was missing is two accessors of 16 and 7 bytes.
* **The single most valuable line of this package is one line.** `USeqAct_Interp::GameLoad` (2013 `0x2e74d0`) is
  nine bytes of retail and a tail `jmp` to `USeqAct_Latent::GameLoad`, which agent ED ported a wave ago. Writing
  `USeqAct_Latent::GameLoad( Ar, _Location );` moved `Dishonored0.sav` from 97,424 bytes to 229,816 —
  **1,398 objects and 132,392 bytes** — and the tree's own comment beside it said it needed
  `UInterpGroupInst::SaveData/LoadData` and could not be done.
* **A defect class this layer's safety net cannot see, found and fixed.** `USequenceFrame::IsSaveable` is the
  return-FALSE ICF fold in retail: no state for a Kismet comment frame is in the blob. This tree inherited
  `USequenceObject`'s TRUE and read 27 bytes that were not there. Because the WORD that then lost its place had
  its high bit set, `FLevelLoader::operator<<` took it for an unshared-sub-level reference and the object loop
  ended **quietly** — census `STREAM ENDED EARLY`, `0 unported`, no class named. `UInterpData` is the only other
  class in retail that turns this slot off against a saveable ancestor; both are now declared, and
  `build/agentER/gen_classlists.py` has a generated check for it with a self-test that fires.
* **Frontier 1 is delivered, and the reason it did not deliver itself is a second finding.** The ported body
  could not run: `ADishonoredNPCPawn::m_pNPCMasterFSM`'s `m_NativeStateMap` is **empty** in this tree — nothing
  calls `InitFSM` on it — so `UDishonoredNativeStateMachine::LoadPartialState`'s lookup always fails and retail's
  two bytes were silently dropped. The read now goes through a scratch instance of the named class. That is the
  +2,421 bytes on `Dishonored1.sav`.
* **Five bodies ported, two accessors, two `IsSaveable` overrides, one signature corrected, one dispatch
  amended.** Ported override classes 56 → 59, unported 56 → 53.
* **Seven more bodies handed over with every offset and every mask resolved**, in stream order, in
  `agentER_status.csv`: `ADishonoredUsableObject`, `ADisPickup_Base`(+`ADisStatPickup`), `ADisRatSpawner`
  (+`ADisGameCrowdDynamicSpawnPoint`), `ANavMeshBlockToggleable`, `UDisSeqAct_NPCTrackTarget`,
  `UDisConvGlobalMan`. Four of them are one or two byte-reads each and are behind record 8019.
* **`Dishonored0.sav`'s new frontier is not an unported class at all**: `0 unported`, and the stop is a
  **2-byte position loss at byte 537,208**, with the last body the stream entered before it being
  `AActor::GameLoad` through `AInterpActor::GameLoad` (2013 `0x18c160`) for `l_tower_p.InterpActor_0`.
  Section 4 is the evidence that it is a position loss and not a missing tail.
* **Regression 37 ok, 0 failed, 0 skipped.** Clean full release build with `DISHONORED_LAYOUT_CHECKS=ON`,
  directory deleted first: 0 errors, 0 C4263, 0 C4264. `rva_sweep.py` 0 suspects in this package's nine files.
  Fold check 0 with a self-test that reports 2/32. My own address audit found one defect of my own (below).

## 1. Frontier 1: the body the brief says exists, does not

### 1.1 There is no `UStateNPCMasterDead_Limp::LoadPartialState`

`functions_2013.csv` names eight `LoadPartialState` overrides and none of them is `_Limp`:

```
0x663260   74  UStatePlayerMasterFalling::LoadPartialState
0x66eb60  150  UStatePlayerMasterLeaning::LoadPartialState
0x675960  135  UStatePlayerMasterClimb::LoadPartialState
0x675da0  155  UStatePlayerMasterHolePeeking::LoadPartialState
0x669980   65  UStatePlayerGrabMovable::LoadPartialState
0x677a10  178  UStatePlayerCarryCorpseIdle::LoadPartialState
0x664c60   23  UStateNPCInstigatedMasterAction::LoadPartialState
0x66a9a0   46  UStateNPCMasterActionImmolate::LoadPartialState
```

`UStateNPCMasterDead_Limp : UStateNPCMasterDead : UStateNPCInstigatedMasterAction`, so it inherits the third
from last. Pinned twice, as the brief requires:

* `vtables.csv` (2012): `UStateNPCMasterDead_Limp{for UStateNPCMasterDead}` slot **88** = `0x6b4ec0`
  `UStateNPCInstigatedMasterAction::LoadPartialState`, slot 87 = `SavePartialState` `0x6b4ea0`.
* the 2013 image's own `??_7UStateNPCMasterDead_Limp{for UStateNPCMasterDead}` at rva `0xd0a988`, read with
  `build/agentER/probe2.py`: slot **89** = `0x664c60`, slot 88 = `0x664c40`. **The 2013 table is shifted by one
  for this class**, which is exactly the hazard `vtables.csv`'s 2012-ness is kept for.

`UDishonoredNativeState`'s own vftable has unrelated functions in those slots
(`UParticleModuleTypeDataBase::PreUpdate`, `FNullDynamicRHI::FinalizeAsyncMipCopy`) — the ICF folds of the empty
base bodies, which is the independent confirmation that a state without an override reads nothing.

### 1.2 The body, and the base signature it corrected

```
00664c60  push ebp / mov ebp,esp
00664c63  mov eax,ecx / mov ecx,[ebp+arg_8]     ; arg_8 = FArchive&
00664c68  mov edx,[ecx] / add eax,64h           ; this + 100
00664c6d  push eax / mov eax,[edx+18h] / call eax   ; FArchive slot 6 (offset 24) = operator<<(UObject*&)
00664c73  pop ebp / retn 10h
```

`retail_sdk_layout.json` gives `UStateNPCInstigatedMasterAction` span 100..104 with one member,
`m_pInstigator` at **100** — `this+0x64`. `SavePartialState` at `0x664c40` is the same instruction sequence
with `retn 0Ch`. So:

```cpp
void UStateNPCInstigatedMasterAction::LoadPartialState( UDishonoredNativeStateMachine*, UObject*, FArchive& Ar, ESaveLoadLocation )
{
    Ar << *(UObject**)&m_pInstigator;
}
```

**The base declaration was wrong and would have hidden the override.** `CppText/UDishonoredNativeState.h`
declared `SavePartialState( UDishonoredNativeStateMachine*, FArchive& )` — two parameters. Every
`SavePartialState` the PDB names takes three (`..., FArchive&, ESaveLoadLocation`); the 2012 comment beside
`LoadPartialState` had already noticed the same thing for the Load half and fixed only that one. With the
two-parameter form, adding the three-parameter override is **C4264**, which is an error in this tree. The base
is corrected. Nothing calls `SavePartialState` (the writing half of the object layer does not exist), so the
change is inert at runtime.

### 1.3 And then it did not run

With `StateNPCInstigatedMasterAction` removed from `GDisPartialStateReaders`, `Dishonored1.sav` went 180 → 181
objects and 18,660 → 18,706 bytes, and then aborted on an out-of-range dictionary index. The property trace
says why, exactly:

```
DisStream 382: byte  18660 ref 413 -> DishonoredGame.StateNPCMasterDead_Limp
DisBin        byte  18661 DisConversationComponent.m_pGenericDialogTree (ObjectProperty)
```

The state-class WORD ends at 18,660 and the **next** read is at 18,661 — one byte, which is
`ADishonoredNPCPawn::GameLoad`'s `Bools`. The instigator's two bytes were never consumed. The cause:

```cpp
UDishonoredNativeState** ppState = m_NativeStateMap.Find( pStateID );   // always NULL here
m_pPartiallyLoadedState = ( ppState != NULL ) ? *ppState : NULL;
```

`m_NativeStateMap` is built by `UDishonoredNativeStateMachine::BuildNativeStateMap` from `m_NativeStates`, and
the only caller of `InitFSM` in this tree is `UDishonoredAIBehavior` (`dishonoredaibehavior.cpp:163`) for the
behaviour FSM. **Nothing initialises `ADishonoredNPCPawn::m_pNPCMasterFSM`**, so its state map is empty for
every NPC and no NPC state's `LoadPartialState` can ever dispatch, whatever is ported. Retail dereferences the
lookup with no NULL check because the session that wrote the save had the state registered.

Reading nothing is not a no-op here: it is a two-byte hole that misplaces every byte after it. The dispatch now
falls back to a **scratch instance** of the named class:

```cpp
UDishonoredNativeState* pReader = m_pPartiallyLoadedState;
if( pReader == NULL && pStateID != NULL && pStateID->IsChildOf( UDishonoredNativeState::StaticClass() ) )
{
    static TMap<UClass*,UDishonoredNativeState*> ScratchStates;   // at most nine, one per state class named
    ...StaticConstructObject( pStateID, UObject::GetTransientPackage(), NAME_None, RF_Transient ); AddToRoot();
}
if( pReader != NULL ) pReader->LoadPartialState( this, m_pManagedObject, _rArchive, _Location );
```

It cannot be the class default object: `m_pInstigator` would then be inherited by every state constructed
afterwards. It is rooted because the archive stores an object pointer in it. **This is a mechanism retail does
not have, and section 7 is the measurement that says the package does not work without it**: with the scratch
instance, `Dishonored1.sav` went from 18,706 bytes and an out-of-range index to **21,081 bytes and a named
class**, 183 objects.

## 2. Frontier 2: the conversation gate is two accessors, not a subsystem

`build/agentER/disasm1.txt` holds the full disassembly. Agent EN's transcription in its report section 4.1 is
correct instruction for instruction; what is wrong is what it concludes about the two members.

```
008a954f  call UDisAttentionInfo_Base::GameSave (0x88af60)   ; 20 bytes: DisSaveLoadObject( Ar, this ). An ICF
                                                             ;   fold with UDisConv_Node_InGameData's own pair,
                                                             ;   which this tree already has - so Super:: IS
                                                             ;   retail's instruction here, not a substitute
008a955f  call [FArchive+18h]   ; slot 6, operator<<(UObject*&)  -> arg_4, pre-set to 0
008a9577  call ByteOrderSerialize( &arg_0, 4 )                   ; INT, pre-set to -1
008a957c  cmp [esi+3Ch],0 ; jz    ; esi = this; +0x3C = 60 = m_PlayingMatinee
008a9585  test ecx,ecx    ; jz    ; the owner
008a958c  cmp eax,-1      ; jz    ; the index
008a9591  mov edx,[esi+3Ch] / imul eax,4Ch / lea ecx,[eax+ecx+48h]
008a959b  mov [edx+1D4h],ecx     ; 0x1D4 = 468
008a95a1  mov eax,[esi+40h]      ; 0x40 = 64 = m_PlayingNode
008a95a8  call USeqAct_Interp::SetConversationNode (0x212a50)
```

**Every offset resolves to a member this tree already declares**, from `retail_sdk_layout.json`:

| retail offset | class | member | in this tree |
|---:|---|---|---|
| 60 (`0x3C`) | `UDisConv_Soiree_InGameData` | `m_PlayingMatinee` (`USeqAct_Interp*`) | yes |
| 64 (`0x40`) | `UDisConv_Soiree_InGameData` | `m_PlayingNode` (`UDisConv_Node*`) | yes |
| **468 (`0x1D4`)** | `USeqAct_Interp` | **`m_pDialogTree_RunningInst`** (`FPointer`) | **yes** |
| **516 (`0x204`)** | `USeqAct_Interp` | **`m_ConversationNodePointer`** (`FPointer`) | **yes** |
| 68 (`0x44`) | `UDisDialogTree_InGameBind` | `m_iActiveRunningInstance` (INT) | yes |
| 72 (`0x48`), stride 76 (`0x4C`) | `UDisDialogTree_InGameBind` | `m_RunningInstances[2]` (`FDisDialogRunningInstance`, size 76) | yes |
| 0 | `FDisDialogRunningInstance` | `m_pTreeGameBinding` | yes |

So `Owner + 76*index + 72` is `&pOwningBind->m_RunningInstances[RunningInstanceIdx]`, and `USeqAct_Interp+0x1D4`
is `m_pDialogTree_RunningInst` — the member `UDisConversationComponent::SerializeForGameLoad` already assigns
the same way (agent EN's own package wrote that line). The **only** thing missing was
`USeqAct_Interp::SetConversationNode`, 16 bytes, `mov [ecx+204h],eax`, and its 7-byte getter. Both are in
`UnInterpolation.cpp`, which is where retail declares them (the 2012 PDB puts them at `uninterpolation.cpp`
lines 3322 and 3327).

`GameSave` (`0x8a94e0`) is the exact inverse and confirms the semantics: it reads
`m_PlayingMatinee->m_pDialogTree_RunningInst`, takes `*that` (`m_pTreeGameBinding`) as the object reference and
`*(that + 68)` (`m_iActiveRunningInstance`) as the INT.

**The stop was not where the brief says either.** The property trace at the old state:

```
DisStream 4644: byte  97422 ref 1814 -> ...DisConversation_InGameData_81.DisConv_Soiree_InGameData_8
DisBin        byte  97422 DisConv_Soiree_InGameData.m_PlayingMatinee (ObjectProperty)
DisStream 4645: byte  97424 ref 1817 -> L_Tower_Script...Main_Sequence.SOIREE_Workers.SeqAct_Interp_3
```

The gate fired **inside the Soiree node's own property walk**, on `m_PlayingMatinee`, whose `ObjectProperty` is
written inline — the matinee's whole `GameSave` follows the WORD. So the Soiree record's first two bytes are the
matinee's index and record 1817 is reached from *inside* record 1814, not after it. That is why opening the
Soiree gate alone moved the census by exactly **two bytes** (97,422 → 97,424).

It is also **the second time this project has caught `CPF_DisNoSaveGame` in the act, from the other side**:
`m_PlayingMatinee` and `m_PlayingNode` are both `CPF_Transient` in `retail_sdk_layout.json` and both **are** in
the DisSaveLoad property stream. Reasoning from the JSON's 32-bit `flags` would have called them absent.

## 3. The matinee: nine bytes of retail, 132,392 bytes of stream

`USeqAct_Interp`'s save five, read out of the 2013 image's own `??_7USeqAct_Interp` at rva `0xc2c738`
(`build/agentER/probe4.py`) — slots **67..71** because the 2013 table is shifted one against `vtables.csv`:

| slot | 2013 | what |
|---:|---|---|
| 67 | `0x5ea9d0` | `UObject::IsRefSaveable` |
| 68 | `0x2cf930` | `USequenceObject::IsSaveable` |
| 69 | `0x2e73b0` | `USeqAct_Interp::GameSave` (unnamed in 2013; 2012 `0x318cf0`) |
| 70 | `0x2e74d0` | `USeqAct_Interp::GameLoad` |
| **71** | **`0x2e74e0`** | `USeqAct_Interp::PostGameLoad` (unnamed in 2013 and **unmatched** in `match_2012_2013.csv`) |

`0x2e74d0` is nine bytes: `push ebp / mov ebp,esp / pop ebp / jmp sub_6E7280`, and `sub_6E7280` is
2012 `0x318bc0` = **`USeqAct_Latent::GameLoad`**, whose body this tree already carries verbatim in
`UnSequence.cpp` (`USequenceOp::GameLoad`, then every `LatentActors(i)->LatentActions.AddItem(this)`). So
`USeqAct_Interp::GameLoad` reads **no bytes of its own** and the port is one line.

**Two things the tree's own comment beside it got wrong**, both corrected in place:

1. it said all three virtuals "need `UInterpGroupInst::SaveData / LoadData`". `GameSave` and `PostGameLoad` do;
   `GameLoad` does not touch them.
2. it gave the third address as **`0x2ea1c0`**. In 2013 that address is inside
   `USequenceOp::ConvertObjectInternal` — a different function. `rva_sweep.py` passes it, because it lands
   inside *some* 2013 function. The right address is `0x2e74e0`, from the vftable slot.

`GameSave` and `PostGameLoad` are still not ported, and neither reads the save stream: `GameSave` flattens every
group instance into the reflected `m_SavedGroupInstData` with an `FMemoryWriter` *before* calling Super, and
`PostGameLoad` reads that byte array back with an `FMemoryReader`, applies it only when the saved group count
equals this session's, and empties it. Their absence therefore cannot desynchronise anything; what it costs is
that **a matinee that was mid-playback comes back stopped**. Said plainly because the Tower's opening is a
matinee-driven conversation and the next package will care.

## 4. `USequenceFrame`: 27 bytes retail never wrote, and why the gate could not see it

With the matinee in, `Dishonored0.sav` reached 229,816 bytes and stopped on `ADishonoredRoute` (section 5).
With the Route in, it reached **498,266 bytes with `0 unported` and the census line `STREAM ENDED EARLY`** — no
class named, no abort, the object loop simply finished. That is the failure mode this whole layer exists to
prevent, appearing for the first time.

`build/agentER/agentER_trace16b.log` places it to the byte. Of the **28,293** object references the stream
resolved, **only three** have a dictionary index outside the level state's 11,324 records, and all three are the
last three (bytes 498,258 / 498,264 / 498,266). Everything before byte 498,256 resolved to a live object whose
path is consistent with its enclosing object. The last record is:

```
DisStream 28293: byte 498231 ref 6117 -> ...Main_Sequence.SequenceFrame_154
DisProp byte 498231 SequenceFrame.SizeX ... 498254 FillTexture ... 498256 FillMaterial
DisStream 28294: byte 498258 ref 29184 (unshared) -> <not in this session>
```

`USequenceFrame`'s `IsSaveable` slot is **the return-FALSE ICF fold** — `0xa6cca0` in `vtables.csv` slot 67,
`0x233610` in the 2013 image's own `??_7USequenceFrame` slot 68, and `match_2012_2013.csv` maps one to the
other. Retail writes **no state at all** for a Kismet comment frame. This tree inherited
`USequenceObject::IsSaveable`'s `return TRUE` and read the frame's ten properties: 27 bytes that belong to the
next object.

**Why it was silent, and this is the part worth carrying:** `FLevelLoader::operator<<` treats bit 15 of the WORD
as "this reference belongs to an unshared sub-level, do not read its state here" and returns without an error.
The first WORD read at the wrong place happened to have that bit set, so the loop's `pObject` came back NULL
from a *deferred* index rather than from a terminator, and `do { *this << pObject; } while( pObject != NULL )`
ended. **A garbage index with the high bit set is indistinguishable from a legitimate unshared reference**, so
the class-list gate — which only fires on a *resolved* object of an unported class — never got a chance.

The fix is retail's own: declare the override. `UInterpData` is the only other class in retail that turns this
slot off against a saveable ancestor (`USequenceVariable`'s is the real `USequenceObject::IsSaveable`), and it
is declared too. `build/agentER/gen_classlists.py` now carries `check_issaveable_off`, which walks every class
whose slot 67 is the FALSE fold, finds the nearest ancestor whose slot 67 is not, and reports any that this tree
does not declare:

```
IsSaveable-off check: 2 class(es) retail turns the save off on, 0 not declared here [] -> issaveable_off.txt
```

and with `ISSAVEABLE_OFF_DECLARED` emptied — a check that reports zero must be shown able to report something —

```
IsSaveable-off check: 2 class(es) retail turns the save off on, 2 not declared here ['InterpData', 'SequenceFrame']
```

The negative claim is also verified against the file itself, in `build/agentER/verify_conv.py`: the three INTs
the reader used to take for `SizeX`, `SizeY` and `BorderWidth` at byte 498,231 are **401020902, 401151976 and
401283050**, ascending in steps of ~131,074 — the signature of reading someone else's floats as integers, and
not any comment frame that was ever drawn.

## 5. The census, and every restored value against the save's own bytes

`build/agentER_release` (configured from `build/agentER_wt` by `build/agentER_build.cmd`),
`-disrestoreslot=16` and `17`, null RHI, `build/agentER/run_restore.py`. Logs in `build/agentER/`:
`agentER_base16.log`, `agentER_base17.log` (HEAD), `agentER_r5_16.log`, `agentER_r5_17.log` (final),
`agentER_trace16b.log`, `agentER_trace16c.log`, `agentER_trace17.log` (`-disstreamdebug=40000
-dispropertytrace`).

| | HEAD `2d08c15` (= agent EN) | agent ER |
|---|---:|---:|
| **Dishonored0.sav ("0 - Dunwall Tower")** | | |
| dictionary records / bytes | 11321/11324, 91844/91844 | **unchanged** |
| records resolved | 6227 | **6227** |
| objects restored | 1111 | **6569** |
| object-stream bytes | 97422/619631 | **537208/619631** |
| skipped | 3532 | **23381** |
| untrusted skips | 411 | **844** |
| unported / partial bodies | 1 / 0 | **0 / 0** |
| PostGameLoad run | 1111 | **6569** |
| stopped on | `DisConv_Soiree_InGameData_8`, an unported override | **dictionary index 30375, which resolves to no record: a 2-byte position loss** |
| **Dishonored1.sav ("1 - Dunwall Sewers")** | | |
| dictionary records / bytes | 6433/6436, 52672/52672 | **unchanged** |
| objects restored | 180 | **183** |
| object-stream bytes | 18660/294961 | **21081/294961** |
| unported / partial bodies | 0 / 1 | **1 / 0** |
| stopped on | `StateNPCMasterDead_Limp::LoadPartialState` | **`DisAbstractItemPickup_85` (`DisPickup_Base`), an unported override** |
| ported override classes | 56 (unported 56) | **59** (unported 53) |

The census line in full, slot 16:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 6569 restored, 23381 skipped, 537208/619631 bytes; 50 spawned, 5093 not found, 0 unported, 1 unresolved,
844 untrusted skips, 0 partial bodies, 6569 PostGameLoad; STREAM ABORTED
```

The intermediate steps, because each one is a separate claim:

| after | objects | bytes | stopped on |
|---|---:|---:|---|
| HEAD | 1111 | 97,422 | `DisConv_Soiree_InGameData` (unported) |
| + the Soiree pair and the two accessors | 1112 | 97,424 | `SeqAct_Interp` (unported) |
| + `USeqAct_Interp::GameLoad`, one line | 2510 | 229,816 | `DishonoredRoute` (unported) |
| + `ADishonoredRoute` | 5279 | 498,266 | nothing — `STREAM ENDED EARLY`, `0 unported` |
| + `USequenceFrame`/`UInterpData` `IsSaveable` | 6569 | 537,208 | index 30375, a position loss |

### 5.1 Every newly restored value against the save's own raw bytes

`build/agentER/verify_conv.py` decompresses the level state's object blob in Python (through agent CF's
`parse_sav`) and reads the bytes at the offsets **the running game's own two diagnostics named**, then counts
how often that byte pattern occurs in the whole blob. It does not use this tree's reader.

```
Dishonored0.sav / DishonoredGameFull_P: 619631 bytes of object data
  byte  97422  u16  DisConv_Soiree_InGameData.m_PlayingMatinee = 1817   raw 19 07    occurs     3/619631  OK
  byte  98234  u16  DisConv_Soiree_InGameData.m_PlayingNode    = 1829   raw 25 07    occurs     3/619631  OK
  byte  98238  u16  the owning DisDialogTree_InGameBind        = 1802   raw 0a 07    occurs     6/619631  OK
  byte  98240  i32  m_iActiveRunningInstance                  = 0      raw 00000000           plausible  OK
  byte 229848  u8   ADishonoredRoute.m_bIsActive              = 1      raw 01                 plausible  OK
  byte 229849  f32  ADishonoredRoute.m_NeglectTimer           = 0.0    raw 00000000           plausible  OK
Dishonored1.sav / DishonoredGameFull_P: 294961 bytes of object data
  byte  18658  u16  the state class WORD                      = 413    raw 9d 01    occurs    11/294961  OK
  byte  18660  u16  UStateNPCInstigatedMasterAction.m_pInstigator = 0  raw 00 00                         OK
  byte  18662  u8   ADishonoredNPCPawn::GameLoad Bools        = 8      raw 08       occurs  1428/294961  OK
```

* **1817 is the record the running game resolved to `SeqAct_Interp_3`**, and the pattern `19 07` occurs three
  times in 619,631 bytes. 1829 is `DisConv_Soiree_1`, three occurrences. 1802 is
  `DisDialogTree_InGameBind_24`, six.
* `m_iActiveRunningInstance` is 0. `m_RunningInstances` is `FDisDialogRunningInstance[2]`, so **-1, 0 and 1 are
  the only values retail's `GameSave` can have written**, which is the independent constraint on that field.
* the Route's five bytes are the last five of a 37-byte record (the stream log puts `DishonoredRoute_2` at
  229,816 and the enclosing `DisBehaviorPatrol`'s next property step at 229,853; `AActor::GameLoad` accounts
  for 32). A flag byte of 1 and a neglect timer of 0.0 are the only two values a route that has never been
  neglected can hold; five random bytes would give a byte in {0,1} and a float in [0, 10^6) about one time in
  a hundred.
* **the instigator is NULL**, which is why it never appeared in the stream log:
  `FLevelLoader::operator<<` returns before its debug print when the index is 0. The two bytes are real and in
  the file; the value is 0. `Bools` = 8 is `m_bNotifiedFakeDeath`, which a corpse is.
* the state-class WORD 413 with 11 occurrences is `StateNPCMasterDead_Limp`, agent EN's frontier, read at the
  offset EN's own report gives (18,660 as Tell-after-WORD).

**One tool defect found while doing this, in another agent's file.** `build/agentCF/parse_objdict.py` drifts on
`Dishonored0.sav`: it decodes **11,483** records where the level state declares 11,324 and leaves 2 bytes over,
so its record→name mapping is not usable for this save. The game's own reader read 11,321/11,324 records and
91,844/91,844 bytes in step, so the offline decoder is the one that is wrong. The values above are read
directly from the object blob and do not depend on it; only the "record N is X" annotations do, and those are
taken from the running game's stream log instead.

## 6. The new frontiers, and the evidence each is a real frontier

### 6.1 `Dishonored1.sav`: `ADisPickup_Base::GameLoad`, 2013 `0x62ae30`, vftable slot 70

```
DisStream 398: byte 21081 ref 422 -> L_PrsnSewer_Script...DisAbstractItemPickup_85
Warning, DisSaveLoad: stopping the level restore at '...DisAbstractItemPickup_85' (DisPickup_Base): its GameLoad
is one of the 53 retail overrides this tree has not ported
```

`1 unported, 0 partial bodies`: the class-list guard fired before any body of ours read a byte, which is the
definition of a frontier that is not a missing tail — there is no body of ours to have a tail.
`ADisPickup_Base` is `ADisAbstractItemPickup`'s nearest override; `agentER_status.csv` has its thirteen reads
with every offset resolved.

### 6.2 `Dishonored0.sav`: a position loss at byte 537,208, and the last body the stream entered

This one is **not** an unported class (`0 unported`) and it is the honest hard part of this package.

```
DisStream 29948: byte 537170 ref 7773 -> l_tower_p.TheWorld:PersistentLevel.InterpActor_0
DisStream 29949: byte 537204 ref 7775 -> l_tower_p.TheWorld:PersistentLevel.DisFog_4
DisStream 29950: byte 537206 ref 3330 -> ...DisBehaviorTriggerAlarm_6.DisAISubStateTakePosition_43
DisStream 29951: byte 537208 ref 30375 -> <not in this session>
Warning, DisSaveLoad: stopping the level restore at dictionary index 30375, which resolved to no live object
```

The evidence that it is a position loss and not a missing tail:

* of the **29,951** references the stream resolved, index 30375 is **the only one** outside the dictionary's
  11,324 records, and **none** of the 29,950 before it had the unshared bit set. The stream was in step for
  537,206 of 619,631 bytes.
* the three objects the stream names immediately before it account for 34, 0 and 0 bytes. `DisFog_4` and
  `DisAISubStateTakePosition_43` read nothing (the first because `AActor`'s `IsSaveable` slot is the FALSE fold,
  so a plain script actor is not a save entry; the second because it was already restored). So the divergence
  is inside the 34 bytes of `l_tower_p.InterpActor_0`.
* `AInterpActor::GameLoad` (2013 **`0x18c160`**, 127 bytes) is `AActor::GameLoad` plus a PhysX pose push and
  reads **no bytes of its own** — `build/agentER/disasm7.txt`. So all 34 bytes are `AActor::GameLoad`'s, and
  retail's body is `sub_58AD70` at 2013 `0x18ad70`, **1,286 bytes**.

So the next package's first job is to compare this tree's `AActor::GameLoad` (`Engine/Src/UnActor.cpp:5801`)
against retail's 1,286-byte body read for read. I did not do it, and I will not claim it is wrong: what I can
claim is that the first byte `Dishonored0.sav` cannot account for is inside that body's record and that nothing
else in the 537,206 bytes before it lost the reader's place.

### 6.3 A guard this layer should grow

`FLevelLoader::operator<<`'s unshared branch is where a position loss hides. Two cheap checks would have turned
`STREAM ENDED EARLY` into a named stop: **a deferred index must still be inside the dictionary's record count**,
and **the top-level loop should only accept a deferred index when the level state declares unshared objects**
(`numObjects != numSharedObjects`). I did not add them — this package's budget went into the bodies — but they
are the difference between the four-hour hunt in section 4 and one log line.

## 7. Deviations and defects, stated plainly

1. **One defect of my own, an address mislabel, caught by a check written for the purpose, and
   `rva_sweep.py` never saw it.** `build/agentER/addr_audit.py` resolves all 42 addresses this package cites
   against `functions_2013.csv` **by name**. Its first pass reported `0x197f70` for
   `AInterpActor::GameLoad` as `NOT A 2013 FUNCTION START` — it is the 2012 `target_va` out of `vtables.csv`,
   which I had read as a 2013 address. The truth is `0x18c160`. It was cited only in this report and in the
   audit table, never in the tree, which is exactly why the sweep could not catch it. The audit now reports
   **42 addresses: 33 resolved by name, 9 cited without a name (each said so in the comment), 0 wrong.**
2. **The scratch-state instance is a mechanism retail does not have.** Section 1.3 is the measurement that says
   the package does not work without it (18,706 bytes and an out-of-range index, versus 21,081 bytes and a
   named class). It is nine lines, it is bounded at one object per state class the stream names, and the
   alternative — the class default object — would put a stale actor pointer on an archetype. The clean fix is
   elsewhere: initialise `ADishonoredNPCPawn::m_pNPCMasterFSM`, which no package has done yet.
3. **Two `GameSave` bodies and one `SavePartialState` are ported and can never run.** The writing half of the
   object layer does not exist in this tree. They are transcriptions of retail's instruction sequence and are
   listed as such in `agentER_status.csv`.
4. **`USeqAct_Interp::GameSave` and `PostGameLoad` are not ported** (section 3), so a matinee that was
   mid-playback comes back stopped.
5. **The base `SavePartialState` signature change is a correction to another agent's declaration.** Nothing
   calls it, so it cannot regress anything, but it is an edit to a file this package does not own.
6. **The regression's `--build-dir` was mangled on the first run.** Passing
   `--build-dir D:\\RecompileDishonored\\...` through Git Bash stripped the backslashes, so the harness built
   into a relative directory inside the worktree and still reported **37 ok, 0 failed, 0 skipped** with real
   layout numbers (`layout_types 2314`, not `-1`). It was re-run with `D:/RecompileDishonored/Recompile/build/agentER_regwt`
   (forward slashes survive the shell) and the stray directory was deleted. Both runs are recorded:
   `build/agentER/regression.txt` and `regression_abs.txt`.

## 8. What this package ports that neither save exercises

A body that has never run is not a body that has been shown to work.

* **`USeqAct_Interp::GetConversationNodePointer` has never run.** Its retail callers are the soiree node's own
  tick and interrupt paths, none of which is ported. The setter runs on every restored soiree node whose
  matinee and running instance are both live.
* **`UDisConv_Soiree_InGameData::GameLoad`'s `if` body has never been entered with all three conditions true on
  `Dishonored0.sav`**: `m_iActiveRunningInstance` reads 0 and the binding resolves, but `m_PlayingMatinee` is
  NULL at that point in the restore for the one soiree node the save carries, because the matinee's own record
  is read *inside* `m_PlayingMatinee`'s property step and the property walk assigns it there. The read of the
  six bytes is exercised; the assignment and `SetConversationNode` are not.
* **`UStateNPCInstigatedMasterAction::LoadPartialState` runs, and reads NULL.** Both `Dishonored1.sav`
  instances of `StateNPCMasterDead_Limp` have a NULL instigator (section 5.1), so the assignment stores NULL.
  The two bytes are consumed, which is what the stream needs; the non-NULL path is untested.
* **`ADishonoredRoute::GameLoad`'s `m_bIsActive == 0` path has never run.** All four routes in
  `Dishonored0.sav` read 1.
* **`UInterpData::IsSaveable` has never been asked.** No `UInterpData` is a top-level entry of either save's
  persistent level; it is declared because retail declares it and because the generated check would otherwise
  report it as missing.
* **`ADishonoredRoute::GameSave`, `UDisConv_Soiree_InGameData::GameSave` and
  `UStateNPCInstigatedMasterAction::SavePartialState` have never run and cannot.**

And the correction to an earlier package's list: agent EN reported `FArkComponentBase::Serialize` as newly
exercised at 48 calls. At this package's state `Dishonored0.sav` restores 6,569 objects instead of 1,111, so
every component container the level holds is now read, not just the sixteen AI brains.

## 9. Hand-overs

1. **`AActor::GameLoad` against retail's 1,286-byte `sub_58AD70`, read for read.** Section 6.2. This is the
   whole of `Dishonored0.sav`'s remaining 82,423 bytes and it is two bytes wide.
2. **The two guards in section 6.3.** A deferred index outside the record count, and a deferred index in a
   level state with no unshared objects, are both provably impossible in a healthy stream.
3. **`ADisPickup_Base::GameLoad` (2013 `0x62ae30`) and `ADisStatPickup::GameLoad` (`0x62b640`).** The frontier
   of `Dishonored1.sav` and record 8142 of `Dishonored0.sav`, both in `agentER_status.csv` with every offset.
   `ADisPickup_Base` needs `AKActor::GameLoad` (ported), `IDisInteractableInterface::DoHighlight` and
   `DishonoredSpawnEmitter`; its byte reads are independent of both.
4. **`ADishonoredUsableObject::GameLoad` (`0x657840`), record 8019 of `Dishonored0.sav`.** Thirteen reads,
   listed in the CSV, then an application tail that drives a `UAnimNodeSequence` through its vtable slots
   +388/+392/+312 and looks up `UDisTweaks_UsableObject`. The reads and the tail are separable — every byte is
   consumed before the first application — but porting the reads without the tail leaves a door that comes back
   at the wrong animation position, so it is a judgement for the next package, not an omission.
5. **`UDisConvGlobalMan::GameLoad` (`0x8a07b0`), record 11323.** Versioned three ways on `Ar.Ver()` (22 and 23)
   and the only body of the nine that reads a counted loop of object references.
6. **`ADisRatSpawner` / `ADisGameCrowdDynamicSpawnPoint` / `ANavMeshBlockToggleable` /
   `UDisSeqAct_NPCTrackTarget`** are one or two byte-reads each, fully resolved in the CSV, and all four are
   behind record 8019 in the stream, so porting them alone moves no number.
7. **`ADishonoredNPCPawn::m_pNPCMasterFSM` is never initialised** (section 1.3). Until it is, no NPC state's
   `LoadPartialState`, `PostLoadPartialState`, `OnEnterState` or `TickState` can dispatch, and a restored corpse
   cannot enter `StateNPCMasterDead_Limp`. This is the same gap `ADishonoredNPCPawn::PostGameLoad`'s
   not-ported note already describes from the other end.
8. **`gen_classlists.py` has grown a third check** (`check_issaveable_off`). Carry it forward with the
   generator; `ISSAVEABLE_OFF_DECLARED` must be kept in step with the C++ overrides or the check silently
   passes.

## 10. Merging

`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` — **all three flags** — **is
required** and **was run** (in the worktree, so it wrote the worktree's headers). The two new
`Inc/CppText/<Class>.h` files only reach the generated class bodies through it.

**Its diff is two lines**, both `#include "CppText/<Class>.h"`:

```
DishonoredGameConversationClasses.h  +1   after DECLARE_CLASS(UDisConv_Soiree_InGameData,...)
dishonoredgameclasses.h              +1   after DECLARE_CLASS(UStateNPCInstigatedMasterAction,...)
```

plus a third for `ADishonoredRoute` in `dishonoredgameclasses.h` — three include lines in two files. Every
other header the generator rewrites comes back byte-identical in content: `git status` lists ~80 files as
modified and `git diff --stat` lists **seven**, because the generator writes CRLF where the index holds LF.

**`Sources.cmake` did not change: 0 units added, 0 removed.** Every body this package writes went into a unit
the build already compiles (`dissavegame.cpp`, `UnSequence.cpp`, `UnInterpolation.cpp`), so the 818-unit exclude
list is byte-identical. `DishonoredGameRegistrants.cpp` and `DishonoredGameNativeStubs.cpp` are likewise
content-identical.

**Files outside `DishonoredGame`: three, all in `Engine`** — `Inc/EngineSequenceClasses.h`,
`Src/UnInterpolation.cpp`, `Src/UnSequence.cpp`.

**Total: 11 files** — 3 new, 8 edited, of which 3 are generated and should be regenerated rather than copied
(`build/agentER_sync.py` prints the list and the count).
`build/agentER_sync.py` is the authoritative list, worktree → main, and `--apply` copies in that direction only.
**I committed nothing and staged nothing.**

### 10.1 Main moved under this package, and three of the eleven files now collide

This worktree is detached at `2d08c15`. While it was working, five commits landed on `main`
(`cd04626` the shim backlog, `e372f88` `rva_sweep.py`, `84e35f4` the settings republish, `e845cd5` the
regression harness, `89aefaf` agent EP's patrol). `git diff --name-only 2d08c15..HEAD` is 96 files and three of
them are files this package also changes. **`--apply` must not be run blind.**

1. **`DishonoredGame/Inc/CppText/ADishonoredRoute.h` — agent EP created the same new file.** EP's version
   declares the patrol route's own interface (`PostBeginPlay`, `Tick`, `Adopt`, `CanAdopt`, …). Mine declares
   `GameSave`/`GameLoad` and nothing else. **The merge appends my two declarations and my `DISHONORED(port)`
   banner to EP's file; it must not overwrite it.** `agentER_sync.py` reports this file as `differs` and copying
   it would lose EP's whole port.
2. **`Engine/Inc/EngineSequenceClasses.h` — `cd04626` removed `DISHONORED_SHIM_STATIC` members**, including two
   inside `UInterpData`'s shim block immediately above where this package inserts `UInterpData::IsSaveable`, and
   it deleted `VERIFY_CLASS_OFFSET_NODIE(UInterpData,InterpData,CachedDirectorGroup)`. Textual conflict in the
   same region; the resolution is to take `cd04626`'s deletions and add this package's four insertions
   (the two `USeqAct_Interp` accessor declarations, `USeqAct_Interp::GameLoad`, and the two `IsSaveable`
   overrides).
3. **`DishonoredGame/Inc/dishonoredgameclasses.h`** — generated, and EP's cpptext include for `ADishonoredRoute`
   is already in main's copy. Regenerating in main after the merge produces both include lines and is the only
   correct resolution; do not copy the worktree's copy.

The other eight files are untouched by those five commits and copy cleanly. **The whole package was built,
measured and regression-tested against `2d08c15`, not against `89aefaf`**, so the census numbers in section 5
are this package against agent EN's tree and nothing else — which is what makes them comparable to EN's — and
the coordinator should re-run `run_regression.py` after the merge.

### 10.2 One thing another agent did to this package's files

`89aefaf` (agent EP) **committed `resources/docs/agents/agentER.md` and `agentER_status.csv`** along with its own
work — they were on disk when it staged, and they are now tracked at that commit. This package committed nothing
itself. It is worth saying because the brief tells every agent in this wave not to commit, and a `git add -A`
from one worktree picks up every other agent's untracked deliverables.

## 11. Verification

1. **Census**: section 5, both saves, before and after, from `build/agentER_release` with the worktree's own
   sources. Baselines reproduced exactly at HEAD (1,111 / 97,422 and 180 / 18,660).
2. **Raw bytes**: `build/agentER/verify_conv.py`, section 5.1 — nine values, each at the offset the running
   game's own diagnostic named, each with its occurrence count in the whole blob.
3. **Frontiers**: sections 6.1 and 6.2, with the reason each is not a missing tail.
4. **Regression**: `build/agentER/regression_abs.txt` — **37 ok, 0 failed, 0 skipped**, the worktree's own
   `run_regression.py`, absolute `--build-dir`, built inside the harness, the five gitignored layout inputs
   copied into the worktree first (`layout_types 2314`, `layout_probed 2341`, not `-1`).
5. **Clean build**: `build/agentER/cleanbuild.log`, `build/agentER_clean` deleted first,
   `-DDISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**, 951 targets.
6. **`rva_sweep.py`**: `build/agentER/rva_sweep.csv`, 7,110 citations in the worktree, 2 suspects — both
   pre-existing in `GFxUI/Src/gfxuirenderer.cpp:1669`, the same two agent EN reported. Narrowed to this
   package's nine files: **365 citations, 0 suspects**.
7. **Fold check**: `0 ported bodies are ICF folds reached by 0 class(es) that do not inherit them`, and
   `build/agentER/make_foldselftest.py` reports **2 bodies / 32 classes** with agent EC's fold roots removed,
   so the check can fire.
8. **IsSaveable-off check**: 2 classes, 0 undeclared, and 2 undeclared in the self-test.
9. **Addresses**: `build/agentER/addr_audit.py`, 42 addresses, 0 wrong, after finding one of my own (section 7).
   Every vtable was pinned twice — `vtables.csv` and the 2013 image's own `??_7` table — and every 2012↔2013
   pair was cross-checked against `match_2012_2013.csv`.

## 12. Where this brief is wrong

* **"`Dishonored1.sav` stops at `UStateNPCMasterDead_Limp::LoadPartialState` — a missing leaf under a body that
  is already ported."** There is no such body in retail (section 1.1), and the leaf was not what was missing:
  the body it inherits is 23 bytes, and the reason the save stopped there is that the NPC master FSM has no
  registered states at all, so the lookup in the already-ported parent could never succeed (section 1.3).
* **"`USeqAct_Interp::SetConversationNode` and `USeqAct_Interp+0x1D4`, neither of which exists — that is the
  conversation system."** `USeqAct_Interp+0x1D4` is `m_pDialogTree_RunningInst` and `+0x204` is
  `m_ConversationNodePointer`; **both members already exist** in this tree's `USeqAct_Interp`, at the retail
  offsets, and another package already assigns the first one. What did not exist was a 16-byte setter and a
  7-byte getter (section 2). The conversation system is not on the save's critical path at all.
* **"Take frontier 1 first; it is the cheaper of the two."** Frontier 2 was the cheaper by a wide margin and it
  is where all the movement is: the Soiree pair plus two accessors plus one line of `USeqAct_Interp::GameLoad`
  is +1,399 objects and +132,394 bytes, against +3 objects and +2,421 for frontier 1.
* **"After frontier 2, `Dishonored0.sav` has the rest of EJ's nine, all under 170 bytes."** Two of the nine are
  not under 170 bytes — `ADishonoredUsableObject::GameLoad` is 785 and `UDisConvGlobalMan::GameLoad` is 290 —
  and EJ's own table said "—" for exactly those two rather than a number. More to the point, after frontier 2
  the save no longer stops on any of the nine: it stops on a byte-count deficit, and `0 unported` says the gate
  has nothing left to say about this save.
* **The tree's own comment beside `USeqAct_Latent::GameLoad` gives `USeqAct_Interp::PostGameLoad` as
  `0x2ea1c0`.** In 2013 that is inside `USequenceOp::ConvertObjectInternal`. The right address is `0x2e74e0`
  (section 3). `rva_sweep.py` passes the wrong one.
