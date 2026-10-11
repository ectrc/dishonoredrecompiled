# Agent FN — the red row: `inputtest_criticals == 1` (wave 20, PHASE16 package FN)

Worktree `build/agentFN_wt`, base `15e8a10`, fast-forwarded onto `b759fd5` (the `-inputcmd` commit; none of its six files is one this package touches) before the measurements. Build `build/agentFN_rel` (clean directory, Release,
`DISHONORED_LAYOUT_CHECKS=ON`, through `build_queued.py`). Sync `build/agentFN_sync.py`.

## 0. The answer in six lines

1. **The fault**: `UInterpTrackVisibility::UpdateTrack` dereferenced a NULL `UInterpData*` — `mov eax,[esi+0x94]`
   with `esi == 0`, `+0x94` being `UInterpData::m_Data`. The NULL came from `UInterpGroup::GetInterpData()`,
   a walk **this tree invented**: group → Outer (`UMatineeData`) → Outer, cast to `UInterpData`. For a
   **shared matinee asset** that second Outer is a `UPackage`, not an `InterpData`, and the cast is NULL.
2. **Retail never does that walk.** Retail's track code reads `Group->Outer + 68`, i.e. the
   `UMatineeData`'s own `m_RunData.InterpLength`, and stops there (2013 rva `0x21f680`, read in full, §3).
3. **The matinee** was `L_Tower_Script…Main_Sequence.SeqAct_Interp_3` ("Boat", 5 groups), whose data is the
   asset `Tower_BalconyBoat.Tower_BalconyBoat` — **51 of the 81 MatineeData objects in `L_Tower_Script`
   are such assets** (the opening cutscene's `Tower_EmpressRegent`, `Tower_IntroOnBoat`, `Tower_Daud` among
   them); only the 30 set-dressing matinees that already played are inline (`InterpData_N.MatineeData_0`).
4. **Why it looked intermittent**: it is not. It fails on every build that contains agent FI's spawn port
   (`8027ab6`) — 8 of 8 minidumps on disk from four packages' exes, same instruction, same NULL — and the two
   "absent" runs (FG, FH) were **worktrees based before `8027ab6`** whose `DisSeqAct_StartSpawn_0` never ran,
   so the Kismet chain never reached `SeqAct_Interp_3`. The stage passed for nineteen waves because the
   matinee that dies is only reachable once a pawn spawns, and because `CRITICAL_RE` could not see it.
5. **The fix**: the runtime reads the matinee data the way retail does (`UInterpGroup::GetMatineeData()`,
   `CastChecked<UMatineeData>(GetOuter())`), in the four tracks that read it; plus retail's
   `FilterObjectDuplicate` in `InitInterp` and the unported `UDisSeqVar_SpawnerPawn::GetObjectRef`, which is
   what put a *spawner* in a matinee AI group (FK's hand-over item 2 and the "one Group" line).
6. **The gate**: §8. Before (untouched HEAD build, same driver): `inputtest_criticals 1`. After: see §8.

## 1. Opening the minidumps first — what the brief got wrong about the frame

`DishonoredGame/Logs/unreal-v9411-*.dmp`: 29 dumps from 10 Oct 22:48 to 11 Oct 18:00. A minidump carries the
module list (so **no ASLR recovery is needed** — the exe's load base is in the dump: `0x490000` for FJ's gate
exe, not `0x400000` and not FJ's `0xdb0000`, which was a different file on a different boot), the exception
record, the faulting thread's context and its stack, **and the heap pages the registers point at**, which is
what made §2 possible. Parsed with `scratchpad/mdmp.py` (MDMP streams 3/4/5/6), resolved against the `.map`
beside each exe after checking the module timestamp (`6acba768` for FJ's gate exe = its `.map`).

| dump | exe | fault | esi |
|---|---|---|---|
| `…-00.14.13` | `DishonoredGame_FIR.exe` | `UInterpTrackVisibility::UpdateTrack+0xd5` | 0 |
| `…-16.17.50` | FJ's gate exe (the gate run in `regression_pile_build_agentFJ_wt_build_agentFJ_gate_inputtest.log`) | `+0xd5` | 0 |
| `…-16.37.31` | `DishonoredGame_rg_Recompile_build_agentF…` | `+0xd5` | 0 |
| `…-16.39.58`, `…-16.45.52` | `DishonoredGame_FLREG.exe` | `+0xd5` | 0 |
| `…-17.31.15`, `…-17.37.22`, `…-18.00.15` | `DishonoredGame_FKR.exe` | `+0xd5` | 0 |
| `…-17.46.58` | `DishonoredGame_FKB.exe` (FK's clean `cd5fadd` control) | `+0xd5` | 0 |

Nine dumps, four packages' executables, one instruction. (Six more dumps resolve to `UInterpTrackSound` /
`Toggle` / `VectorProp` `UpdateTrack` — those exes were **re-staged after the dump**, so the `.map` beside them
is a newer build's; the rva `0x551225` is the same bytes. The remaining dumps are `appError` exits, code
`0x1`, from FK's and FM's instrumented runs, not crashes.)

The faulting code, disassembled from FJ's gate exe (`scratchpad/disfn.py`, capstone, section-mapped):

```
+005d  call ?GetInterpData@UInterpGroup@@QBEPAVUInterpData@@XZ
+0066  mov esi, eax                      ; IData
…
+00d5  mov eax, [esi+0x94]               ; IData->m_Data          <<< fault, esi == 0
+00eb  movss xmm0, [eax+0x44]            ; ->m_RunData.InterpLength
```

`UInterpData::m_Data` is at **148** in `retail_sdk_layout.json`, `FRuntimeMatineeData::InterpLength` at 56+12 = 68:
the inlined `IData->GetInterpLength()` of `UnInterpolation.cpp:11973`.

**Where the brief was wrong, item 1**: the "one credible frame", `UArkComponentContainer::AddNewComponent_Internal+0x2d`,
is not in the crash at all. The printed stack is a failed `StackWalk64` (no PDB), and the coordinator's decode
subtracted the wrong base. The real stack, read off the dump's stack memory with the `.map`:
`UInterpTrackVisibility::UpdateTrack+0xd5 ← UInterpTrack::ConditionalUpdateTrack+0xb0 ← UInterpGroup::UpdateGroup+0x205
← USeqAct_Interp::UpdateInterp+0x56c ← USeqAct_Interp::StepInterp+0x350 ← USeqAct_Interp::UpdateOp+0x4ad
← USequence::ExecuteActiveOps+0x6fb ← USequence::UpdateOp ← UWorld::Tick+0x8dd`. Nothing in it is a component,
a blend tree or the spawn itself.

## 2. Reading the objects out of the dump

The registers at the fault point at heap pages the dump kept. Walking `UObject::Outer` (+36) and reading the
vtable pointers against the `.map`:

```
track  29675a00  ??_7UInterpTrackVisibility  outer 24a10e00
group  24a10e00  ??_7UInterpGroup            outer 2967bb80   <- not in the dump
actor  294e7b00  ??_7AInterpActor            (InterpActor_18, the boat)
trinst 4532b8b0  ??_7UInterpTrackInstVisibility
  -> grpinst 2ada0cb0  ??_7UInterpGroupInst
    -> seq    293b3980  ??_7USeqAct_Interp   name 0x1830:4  ("…_3")
      -> seq  294af480  ??_7USequence        outer 22edaf00  (the level)
```

`Cast<UMatineeData>(group->Outer)` had succeeded and `Cast<UInterpData>(MatineeData->Outer)` had failed, so the
group's data's Outer was not an InterpData. The level package says what it was. `L_Tower_Script.upk` is
LZO-compressed (`CompressionFlags 2`, 39 chunks) — `scratchpad/upk.py` + `lzo.py` decompress the header chunks
and read the name/import/export tables with this tree's own `FObjectExport` serializer as the format:

```
exp 13543 Tower_BalconyBoat.Tower_BalconyBoat.InterpGroup_0.InterpTrackVisibility_0
exp 13544 Tower_Daud.Tower_Daud.InterpGroupAI_0.InterpTrackVisibility_5     … (7 Daud groups)
exp 13550 Tower_GiveLetter.Tower_GiveLetter.InterpGroup_5.InterpTrackVisibility_0
exp 13541 DistractionSoiree.TableCleaning.InterpGroup_6.InterpTrackVisibility_2
exp 13542 TheWorld.PersistentLevel.Main_Sequence.SOIREE_Workers.InterpData_1.MatineeData_1.InterpGroup_4.InterpTrackVisibility_6
```

and `Main_Sequence.InterpData_3` has **no** `MatineeData` child export: its `m_Data` is
`Tower_BalconyBoat.Tower_BalconyBoat`, a `MatineeData` whose Outer is the forced-export **UPackage**
`Tower_BalconyBoat`. **30 of the level's 81 MatineeData are inline under their InterpData; 51 are assets under a
package** (`Tower_IntroOnBoat`, `Tower_EmpressRegent`, `Tower_Daud`, `Tower_GiveLetter`, `Tower_Empress_Emily`,
`Tower_HoldingEmily`, `SimpleSoiree.*`, `DistractionSoiree.TableCleaning`, …). Every one of those 51 would have
died on its first Visibility, Event or Toggle track update, and that set is the cutscenes.

FK's census confirms the matinee (`FK3.log:2978`): `Main_Sequence.SeqAct_Interp_3` — groups `Boat` (Visibility,
StretchAnimControl, Move, AkEvent), `Elite_StageMark`, `Elite` (InterpGroupAI, StretchAnimControl), `BoatCommands`
(Attachment), `FX` (Toggle). The Visibility key on `Boat` fires at the matinee's first update, 0.15 s after
`InitInterp` — which is the 0.15 s between the "one Group" warning and `=== Critical error: ===` in every log.

## 3. Retail, read in full (both databases, addresses by hand)

| function | 2013 rva (VA − 0x400000) | size | 2012 rva | size |
|---|---|---|---|---|
| `UInterpTrackVisibility::UpdateTrack` | `0x21f680` | 601 | `0x2364c0` | 601 |
| `UInterpTrackEvent::UpdateTrack` | `0x21d4f0` | 426 | `0x2342b0` | 426 |
| `UInterpTrackToggle::UpdateTrack` | `0x21c820` | 1099 | `0x2336d0` | 1099 |
| `USeqAct_Interp::InitInterp` | `0x2342e0` (`sub_6342E0`, pinned by its callers `Activated` `0x23907f`/`Initialize` and its call into `FindGroupLinkedVariable`) | 1471 | `0x25ad70` | 1410 |
| `FilterObjectDuplicate(TArray<UObject**>&)` | `0x223f40` (`sub_623F40`, the call before `FindGroupLinkedVariable` in `InitInterp`) | 435 | `0x23a680` | 537 |
| `USeqAct_Interp::FindGroupLinkedVariable` | `0x223ea0` | 158 | `0x23a5e0` | 158 |
| `UInterpGroupInstAI::InitGroupInst` | `0x2304f0` | 196 | `0x244770` | 196 |
| `UInterpGroupInstAI::InitPawn` | `0x22bbc0` | 373 | `0x240e10` | 358 |
| `UDisSeqVar_SpawnerPawn::GetObjectRef(INT)` | `0x7a2f30` | 316 | `0x807420` | 316 |
| `UDisSeqVar_SpawnerPawn::GetObjectValue` | `0x786c00` | 24 | `0x7f3c50` | — |
| `UDisSeqVar_SpawnerPawn::GetValueStr` | `0x79cc90` | 257 | `0x807560` | 257 |

(`idalib` directly from Python on private copies of both databases — the MCP server's four workers were held
by FP and FQ all session; `scratchpad/ida_q.py`.)

**`UInterpTrackVisibility::UpdateTrack` (2013 `0x21f680`)**: `Actor = GrInst->GetGroupActor()` (virtual +292),
return if NULL; `v7 = *(this->Outer + 36)` = **the group's Outer, used as the MatineeData**; `Seq = GrInst->Outer`;
`bIsPlayingBackwards` from `Seq+400` bits `0x1|0x100`; then `if (NewPosition == *(float*)(v7 + 68))` — **`+68` is
`UMatineeData::m_RunData.InterpLength`**. There is no `UInterpData` anywhere in the function. `Event` and `Toggle`
do exactly the same (`v7 + 68`, `v35 + 68`). This is the thing DK's comment at `UnInterpolation.cpp:3247`
("the reference reads Outer directly at nine sites and Arkane's object graph has one more level") got half
right: Arkane's graph does have one more level, but retail's code **stops at the MatineeData** — it never
needs the InterpData, so it never cares whether the MatineeData is inline or an asset. This tree's
`interptrackfaceto.cpp:59`, `interptracklocomotion.cpp:49`, `interptracklookat.cpp:53` already read
`CastChecked<UMatineeData>(Group->GetOuter())` for the same reason.

Retail's Visibility also gates `bFireEventsWhenJumpingForwards` on `!GIsEditor || GWorld->HasBegunPlay()` — not
ported, editor-only difference, left alone.

**`USeqAct_Interp::InitInterp` (2012 `0x25ad70`)**: `TermInterp` if instances linger; `InterpData =
FindInterpDataFromVariable()`; **`CanPlay()`** (returns FALSE → `InterpData = NULL`, return FALSE); per group:
Director → `UInterpGroupInstDirector` per compatible player; `UInterpGroupPlayer` → `UInterpGroupInstPlayer`
bound to the local player's pawn (FK's hook); otherwise, for a non-folder group: `GetObjectVars`, `GetNamedObjVars`,
**`FilterObjectDuplicate(ObjectVars)`**, then `FindGroupLinkedVariable(j)` per index with `UInterpGroupInstAI`
for an AI group and `UInterpGroupInst` otherwise. Ours matched except for `CanPlay` (FK item 9, FO's) and
`FilterObjectDuplicate`, now added.

**`FilterObjectDuplicate`**: two local arrays; for each `ObjectVars(i)`, if `*ObjectVars(i)` is already in
`lReferencedObject` the index goes on `lObjectToRemove`, else `AddUniqueItem`; then the removals. Ported as a
file-static of the same name.

**`UInterpGroupInstAI::InitGroupInst` (2013 `0x2304f0`)**: `Group = AIGroup = InGroup` (no cast); **if `(!GIsGame
|| !InGroup || InGroupActor)`**: `UInterpGroupInst::InitGroupInst(InGroup, InGroupActor)`, clear bits `0x18`
(`bHaveLocomotion`, `bHaveMoveTrack`), then for each `TrackInst(i)->InstancedTrack`: `bHaveMoveTrack |=
bIsMoveControlTrack`, `bHaveLocomotion |= bIsAIControlTrack`; `InitPawn()`; `UpdatePhysics(TRUE)`. **Retail does
NOT swap a spawner for its pawn here, and does nothing when the actor is NULL in game** (the deferred-attach
path, same as ours). So FK's question — "whether retail swaps GroupActor for the pawn before chaining to the
base" — has the answer **no**: the pawn arrives through the *variable*, `UDisSeqVar_SpawnerPawn::GetObjectRef`,
which this tree had never ported (a comment-only skeleton on the exclude list), so `USeqVar_Object::GetObjectRef`
answered with `&ObjValue` — **the spawner**. That is why every AI group on this mission bound a
`DishonoredSpawner`, why `BeginAnimControl` was asked of `DishonoredSpawner_12` in FK5, and why two groups
linked to the same spawner variable print "an Actor may only be connected to one Group! [DishonoredSpawner_11]".
Where ours differs from retail inside the function (`eventMAT_BeginAIGroup` in place of `InitPawn`, the two
flags) is FO's MAT contract. **The brief had it on my path and it was not the fault** - but once the variable
resolved to a pawn its `eventMAT_BeginAIGroup` (`FindFunctionChecked`) was the *next* crash (§8), so the two
script-event calls carry a `FindFunction` guard now; nothing else in the function changed.

**`UDisSeqVar_SpawnerPawn::GetObjectRef(Idx)` (2013 `0x7a2f30`)**: `ObjValue` (+148) → `Cast<ADishonoredSpawner>`;
NULL if none, `Idx < 0`, or `Idx >= Spawner->m_SpawnedPawns.Num()` (+612/+616); if this variable's
`m_pSpawnedPawns` (+176) has a different count, `Empty` + `AddZeroed(count)`; `m_pSpawnedPawns(Idx) =
Spawner->m_SpawnedPawns(Idx)`; return `&m_pSpawnedPawns(Idx)`. `GetObjectValue` = `*GetObjectRef(0)` or NULL.
`GetValueStr` prefixes `"SpawnerPawns: "` when `ObjValue` is a spawner. All three ported; `DrawExtraInfo` (editor
canvas) not.

## 4. What changed (every function named; `build/agentFN_sync.py` has the same list)

- `Engine/Inc/EngineInterpolationClasses.h`: `UInterpGroup::GetMatineeData()` declared; `UMatineeData::FindDirectorGroup()` declared.
- `Engine/Src/UnInterpolation.cpp`: `UInterpGroup::GetMatineeData()` (new); `UInterpTrackToggle::UpdateTrack`,
  `UInterpTrackEvent::UpdateTrack`, `UInterpTrackSound::UpdateTrack`, `UInterpTrackSound::PreviewUpdateTrack`,
  `UInterpTrackVisibility::UpdateTrack`, `UInterpGroup::EnsureUniqueName` read `MData->m_RunData` instead of the
  `UInterpData` walk; `FilterObjectDuplicate` (new static) called from `USeqAct_Interp::InitInterp`. `GetInterpData()`
  itself keeps its body (editor callers in `UnInterpolationDraw.cpp`, `UnEngine.cpp`'s anim-set lister,
  `PostEditChangeProperty`) with a corrected comment that says it may be NULL.
- `Engine/Src/matineedata.cpp`: `UMatineeData::FindDirectorGroup()` (new).
- `DishonoredGame/Src/disseqvar_spawnerpawn.cpp` (off the exclude list, anchored from `DishonoredGameRegistrants.cpp`
  as `GDishonoredFNLinkAnchors`): `GetObjectRef`, `GetObjectValue`, `GetValueStr`, `execGetObjectValue`;
  `CppText/UDisSeqVar_SpawnerPawn.h` included in the generated class; the native stub removed from
  `DishonoredGameNativeStubs.cpp`.
- `Core/Src/UnObj.cpp`: `UObject::StaticAllocateObject`'s replace-branch `check` → `checkf` naming the object (§6).
- `resources/tools/run_regression.py`: `--newgame-at` 22 → 50 (§5).

## 5. `--newgame-at`

FK measured `LoadMap` beginning at 27.6 s under another agent's build and `-finewgame=22` asserting inside the
async load (`FK2.log:838`); 50 passed in six of FK's runs. Default raised to 50 with that measurement in the
comment. The `newgame` stage's 300 s wall budget and its `newgame_menu_open_seconds >= 10` bound both still hold
(the menu is *open longer*, not shorter). Measured in §8.

## 6. The `PrepareMapChange` assertion (FK item 10) — specified, with one correction

FK reported it in **2 of 10** runs (`FK2.log:838`, `FK6.log:1078`). On disk, `FK6.log:1078` is a texture-streaming
line and the only `Assertion failed: !Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading)` in `build/agentFK/logs/`
is **FK2's** (FK6's critical at 83.52 s is the `BeginAnimControl` appError FK fixed). So it is **1 of 10 on the
evidence that survives**, and it fired 0.02 s after `-gfxuimenu: opened UI_Global.Global` with a UTF-16 text
buffer where the stack should be (STATUS defect 7's walker fault). It is `UObject::StaticAllocateObject`'s
*replace* branch — a `ConstructObject` for a name that already exists while that existing object is still
`RF_AsyncLoading` — so the candidates are the objects the front end constructs by fixed name right after
`UI_Global` opens while the menu map's `PrepareMapChange` is still streaming. I could not reproduce it (it is
one run in ten and the build queue made ten front-end boots unaffordable this wave), so instead of a guess the
`check` is now a `checkf` that prints **the object's full name, its flags, the new class and the outer**: the
next instance names its object. Owner: FP (it is the front end's bring-up).

## 7. Where this brief was wrong

1. **"The one credible frame is `UArkComponentContainer::AddNewComponent_Internal+0x2d`."** No frame of the
   crash is near it; the dump's EIP is `UInterpTrackVisibility::UpdateTrack+0xd5` in nine dumps across four
   packages' executables.
2. **"Intermittent: present in three gate runs of HEAD, absent from two."** Deterministic. The two absent runs
   (FG `f72f9f5`, FH `63c2539`) are worktrees that predate FI's `8027ab6`; without the spawn port `StartSpawn`
   never logs and `SeqAct_Interp_3` is never activated (their census shows `SeqAct_Interp_8` at 47.9 s instead).
   Same binary, same stage: 9 of 9 dumps.
3. **"These may be one fault" (the spawner-as-group-actor and the crash).** Two faults, one trigger. The spawner
   in the group is `UDisSeqVar_SpawnerPawn::GetObjectRef` unported; the crash is `GetInterpData()` on an asset
   matinee. A matinee bound to a spawner does not crash; a matinee whose data is an asset crashes with any actor.
4. **"Read retail's `UInterpGroupInstAI::InitGroupInst` and what it does when the group actor is a spawner."**
   It does nothing special — it has no idea what a spawner is; the resolution is in the Kismet variable.
5. **"Do not subtract `0x400000` … recover the ASLR base from a suspended relaunch."** Unnecessary: a minidump's
   module list holds the base. FJ's `0xdb0000` was true for *FJ's* file on *that* boot; FJ's gate exe loaded at
   `0x490000`, HEAD's gate exe at `0xa60000`. Open the dump, read `MINIDUMP_MODULE.BaseOfImage`. **For STATUS**: the
   "Known pitfalls" entry FJ wrote ("Windows caches an image's ASLR base per boot … recovers the base") is true
   only for the same unmodified file; every agent stages a different exe and the harness re-stages on every run,
   so the suspended-relaunch trick answers for one file only. The dump already holds the base for the file that
   crashed - that is the method, and it needs no relaunch.
6. **"`-finewgame=22` fired into an async `LoadMap` … FK's `PrepareMapChange` assertion, 2 of 10."** FK2's assert
   is at 33.11 s on a run where `LoadMap` began at 27.6 s — consistent with FK's reading — but it is **1 of 10**
   on the logs that exist, and the "front end's own `PrepareMapChange`" attribution is an inference from timing,
   not a stack. §6.
7. `FK5.log:5246` names `DishonoredSpawner_12`; the gate's line names `DishonoredSpawner_11`. Different matinees
   (`SOIREE_Empress_Regent.SeqAct_Interp_12`'s AI groups vs `Main_Sequence.SeqAct_Interp_3`'s `Elite`), same
   unported variable.

## 8. Measurements

All runs through the harness (`-unattended`, no real input; `build_and_smoke.py`), from this worktree's copy of
`run_regression.py` with the absolute build dir `D:/RecompileDishonored/Recompile/build/agentFN_rel`. Logs in
`build/agentFN_*.log|txt` and the per-run game logs in `build/agentFN/inputtest_*.log`.

**Before** - the untouched HEAD gate build (`build/gate_rel`, `af98872`+docs) through the same stage and driver,
staged as `DishonoredGame_FNbase.exe` (`build/agentFN_base_inputtest1.txt`, `fnbase_inputtest.log`):
`8 ok, 1 failed` - **`inputtest_criticals 1`**, lines 3340-3342: `StartSpawn DisSeqAct_StartSpawn_0` -> `an Actor
may only be connected to one Group! [DishonoredSpawner_11]` -> `=== Critical error: ===` at 44.45 s; its minidump
(`unreal-v9411-2026.10.11-19.27.14.dmp`, base `0xa60000`) is the tenth with EIP at
`UInterpTrackVisibility::UpdateTrack+0xd5`, `esi == 0`, `params [0, 0x94]`.

**The build**: `build/agentFN_build0.log` (configure + 147/970 before the pause killed it), `agentFN_build1.log`
(the rest; 3 LNK2019 from an `IMPLEMENT_CLASS` my patch script had removed twice over - restored),
`agentFN_build2.log` and `agentFN_build3.log` (0 errors). Across all four: **0 `error C`/`LNK` left standing, 0 C4263,
0 C4264**, `DISHONORED_LAYOUT_CHECKS=ON`; the gate's own build stage: `build_*_errors 0`, `layout_mismatching 0`,
`layout_contract 0`, `layout_types 2314`.

**First gate on the fix** (`build/agentFN_regression.log`, 5467 s under three other packages' builds): `46 ok, 1
failed` - `inputtest_criticals 3`, and it was a **different, better crash**: the Visibility fault and the "one Group"
line were gone, the spawner variable now resolved to the spawned pawn, and `UInterpGroupInstAI::InitGroupInst`
reached `PawnActor->eventMAT_BeginAIGroup(...)`, which is `FindFunctionChecked` on an engine script event
Dishonored's script never declares: `appError: Failed to find function MAT_BeginAIGroup in DishonoredNPCPawn
l_tower_p…DishonoredNPCPawn_26` (the appError's two `Critical:` lines plus the `Critical error:` banner = 3). Same
class of fault FK fixed for `BeginAnimControl`, one layer further in. Guarded the same way (`FindFunction`, one
warning naming the group and the pawn, FO's native `InitPawn`/`MAT_BeginAIGroup` named as the real port).

**Second gate** (`build/agentFN_regression2.log`, 2234 s): **47 ok, 0 failed, 0 skipped.** `inputtest_criticals 0`,
`inputtest_moved 923.4`, `touch_census 308`, `sequence_census 93613`, `physics_actors 1369`, `probe_natives 4`;
`newgame_menu_open_seconds 49.7` (the new `--newgame-at 50`), `newgame_on_mission 117`, `newgame_pawns 29`,
`newgame_criticals 0`; `options_alive 232.6`; `d3d9_startup_seconds 4.6`.

**Six more `inputtest` runs on the same binary** (the probe plus `--no-build --only inputtest` x5): `9 ok` each,
`inputtest_criticals 0` x6 (rep 5's first attempt read `inputtest_peak_speed 0.0` and the harness re-ran it as
load-sensitive; both attempts 0 criticals). In every one of the six game logs: `StartSpawn DisSeqAct_StartSpawn_0`
at ~43.7 s, **0** "one Group" lines, **0** `Critical`, `SeqAct_Interp_3` active in **89** census lines from the spawn
to the 150 s budget, and one `Warning: matinee AI group 'Elite': DishonoredNPCPawn_26 has no script
MAT_BeginAIGroup …` - the Boat matinee that killed every HEAD run plays to the end of the stage with the spawned
Elite guard as its AI group's actor. 10 of 10 crashes before, 0 of 6 after, same stage, same driver.

## 9. Hand-over

- **FO**: 51 of this level's 81 matinees are asset `MatineeData` under a `UPackage` — `Tower_EmpressRegent`,
  `Tower_IntroOnBoat`, `Tower_Daud`… — anything that walks Outer past the `UMatineeData` is wrong for every one
  of them; use `UInterpGroup::GetMatineeData()`. `UInterpGroupInstAI::InitGroupInst`/`TermGroupInst` carry only a
  `FindFunction` guard around the script events (§8); the retail shape is `InitPawn` (2013 `0x22bbc0`, 373 B):
  `GetGroupActor()`, test the pawn's class bit at `+266 & 0x20`, vcall `+1248` with `Seq->m_bForcesUndrawBeforePlaying`,
  **vcall `+1204` = `ADishonoredNPCPawn::MAT_BeginAIGroup(UInterpGroupInstAI*)` (2013 `0x765080`, 720 B)**, then vcall
  `+1268` with the names `Gesture/Posture/Distraction/Face` and `GroupAnimSetsPawn` (this+112 in game, `Group+96` in
  the editor); plus the `bHaveLocomotion`/`bHaveMoveTrack` flags from each track's `bIsAIControlTrack`/`bIsMoveControlTrack`.
  Until that lands the spawned guard is not moved to its stage mark - the warning names it on every run.
  `USeqAct_Interp::CanPlay` is still absent from `InitInterp`.
- **FP**: §6's `checkf` is armed; the next front-end boot that asserts names its object.
- `UInterpData::GetInterpLength()` and friends still exist for the 20-odd editor-side callers; the runtime no
  longer goes through them.
- The minidump + `.map` method is ~120 lines of Python (`mdmp.py`, `disfn.py`); the package parser with LZO
  (`upk.py`, `lzo.py`) reads any cooked package's export table. Both in the session scratchpad; copied to
  `build/agentFN/tools/`.
