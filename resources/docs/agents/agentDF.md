# agent DF — the AI sub-states, the desire layer, and the event dispatcher nobody had ever started

Package: the 24 `UDisAISubState*` classes and their `FDisAISubState*_Param` structs, the `IDisDesiresInterface` request
layer they talk through, and the one engine start-up line that creates `FArkGameEventDispatcher`.

Base: HEAD `9b77df6` (after `f13ad82` CG, `b66b8bd` DB, `9b77df6` DE). Worktree `build/agentDF_wt`, build dir
`build/agentDF`, build script `build/agentDF_release.cmd`. No commits, no `git add`. `resources/play.cmd` and
`resources/build-release.cmd` untouched.

Agent CG left the AI brain running but mute: every NPC entered `DisAISubStateInit` and stopped there, because the
sub-state a behaviour wanted to request did not exist and the `_Param` struct that would have named it was a typedef. The
constraint CG inherited from agent AU — **never land a `_Param` without its sub-state body** — is why this package is 24
classes wide rather than one deep.

## The census, before and after

Same map, same switches, same build script; `L_Tower_P`, `-disai`, 0 `Critical` in every run.

| | before (`f13ad82`, own build) | after |
|---|---|---|
| NPC pawns / controllers / brains | 26 / 26 / 26 initialized | unchanged |
| sub-state enters | 684, **all `DisAISubStateInit`** | **710: `DisAISubStateInit`=684, `DisAISubStateStand`=26** |
| sub-state transitions | **0** | **26** |
| behaviour in slot 0 | (not measured — the counter did not exist) | `DisBehaviorIdle`=26 |
| desire set calls | 0 | 26 |
| loco requests new/update/stop | 0 / 0 / 0 | **26 / 0 / 0** |
| faceto, lookat, body intentions | 0 | 0 / 0 / 0 (faithful — see below) |
| ark event dispatcher | **no instance; 0 registrations, 0 dispatches** | instance yes; 1 global registration, 2 unregistrations, 1 deferred, 3 dispatches, 3 callbacks invoked; self-test ok |

Every NPC in the tower now picks up `DisBehaviorIdle`, that behaviour requests `DisAISubStateStand` through a real
`FDisAISubStateStand_Param`, the sub-state machine performs the transition, the sub-state runs its tick, and the tick
issues exactly one locomotion desire that stays open. That is the whole idle loop, end to end, for the first time.

`faceto 0` is not a gap: `UDisBehaviorIdle` passes `bAlwaysStrafe = FALSE`, so `m_bReadyToFocus` is FALSE and
`EnsureProperRotation` withholds the facing until arrival — and nothing can move the NPC yet, so arrival never happens.
`body intentions 0` likewise: `GetResumingBodyIntentionDesire` answers FALSE in retail (see DEFECT 4), and no behaviour in
this map pauses and resumes.

Runs kept: `agentDF_final_tower.log` (150 s null RHI) and `agentDF.log` (180 s d3d9, `-distouch`). The d3d9 run reaches
100,412 brain ticks and 100,360 sub-state ticks with the same 26 transitions and 0 `Critical`, i.e. the loop is stable
over six figures of ticks rather than merely reached once.

## What the NPCs still cannot do, and what they are asking for

Locomotion is a separate package (`FArkComponentLocomotion`, 117 functions, four nav-mesh entry points) and was **not
started**. Every sub-state that needs movement issues its request and the request is counted, not executed. What the idle
NPCs ask for today, per NPC: one `FDisLocoRequest` with the pawn's own location as the target, transit speed walk, which
`DisLocoSpeedIndexForTransitSpeed` maps through `UDisTweaks_NPCPawn::m_TransitSpeedToLocomotionSpeed`. The component
boundary lives in one place, `DisDesireStructs::AcceptRequest` / `IsComponentRequestStarted` / `NoteComponentGap` in
`Src/disdesirestructs.cpp`, so the locomotion agent has a single seam to fill and a counter per kind that says whether
it is being asked.

## Natives

CG measured 236 remaining `DishonoredGame` stubs and 41 behaviour callbacks blocked behind the `_Param` structs. Of those
41, **CG had already hand-written 18** (mostly as empty bodies), so this package's honest new count of exec wrappers is
**10**, listed in `DishonoredGameNativeStubs.ported.agentDF.txt`; 9 of the 10 construct a parameter this package
introduced:

`UDisBehaviorAmbush::execRequestStateExitCallback_TakeActorPosition`,
`UDisBehaviorAssassinCombat::execRequestStateExitCallback_DoAttractSpell`,
`UDisBehaviorGuard::execRequestStateExitCallback_TakePosition`,
`UDisBehaviorNotice::exec{OnEnterCallback_Init, OnExitCallback_Init, RequestStateExitCallback_GenericAction}`,
`UDisBehaviorPanic::execRequestStateExitCallback_GenericAction`,
`UDisBehaviorSearch::exec{OnEnterCallback_GenericAction, OnExitCallback_GenericAction}`,
`UDisBehaviorTriggerAlarm::execRequestStateExitCallback_TakePosition`.

The bulk of the package is not exec wrappers but the 24 sub-state classes, their 25 `_Param` structs and the 34
`IDisDesiresInterface` methods those callbacks dispatch into: 353 new function definitions, +6,685 / -777 lines across 62
existing files and 37 new `Inc/CppText/` headers. `agentDF_status.csv` is generated from the tags in the tree
(`build/agentDF_work/mkstatus.py`), so it cannot drift from it: 160 tagged sites, 54 with a 2013 rva.

### The classification the next agent inherits

`build/agentDF_work/classify4.py` is CG's `classify3.py` re-pointed at this worktree, so the number is measured after the
package rather than before it (`build/agentDF_work/sweep_classified_df.csv`):

| | CG (before) | now |
|---|---|---|
| blocked | 109 | **81** |
| self-contained | 69 | **90** |
| own-helper | 35 | **42** |
| empty | 12 | 12 |
| absent in retail 2013 | 11 | 11 |
| portable without new plumbing | 104 | **132** |

Every `_Param` and every `IDisDesiresInterface` entry has left the blocker list. The blockers that remain concentrate in
`UDishonoredAIBehavior::FireDialogHook` (5), `FDisRangedFloat::GetRandomFloatValue` (4),
`FDisAIMonitorPawnReachability` (6 across two methods), `IDisRelationshipInterface` (5) and
`UDisAISubProcessManageAttacks` (7) — none of them large, and the reachability pair is the one that needs nav mesh.

## Defects found

**DEFECT 1 — `UDisBehaviorIdle::OnBehaviorResume` did not override its base, and the build was green.**
`Src/disbehavioridle.cpp`. The hand-written signature took `const FDisBodyIntention&`; the base declares
`OnBehaviorResume()`. The whole transition path was dead code and the census read `0 transitions` with no error anywhere.
MSVC had said so — `warning C4263: member function does not override any base class virtual member function` and
`warning C4264: no override available for virtual member function ... function is hidden` — and the warnings were lost in
the noise. Fixing the signature took transitions **0 → 26**.
**Hand-over: make C4263 and C4264 errors in this codebase.** In a tree built from decompiles, a hand-written override
whose signature drifts is the single most likely silent failure, and it is exactly what these two warnings detect. Any
agent porting a virtual can otherwise ship dead code that passes every gate.

**DEFECT 2 — 1,523,652 locomotion requests from 26 standing NPCs in 119 seconds.**
`Src/disdesirestructs.cpp`, `FDisDesireRequest::GetRequestStatus` (2013 body, 881 bytes). With no component behind the
request, `m_RequestID` stayed `INDEX_NONE`, so the status was `NewRequestNeeded` on every single `TickDesires` and the
redundancy filter that exists precisely to stop re-issuing orders never engaged. The fix is to keep the component
*contract* at the boundary even though the component is absent: `IsComponentRequestStarted` answers TRUE and
`AcceptRequest` returns a synthetic increasing id. **1,523,652 → 26.** Worth stating plainly: a stubbed-out component must
return what the caller's state machine expects, not the zero value, or the state machine above it runs open-loop.

**DEFECT 3 — retail's `UDisBehaviorTriggerAlarm::RequestStateExitCallback_TakePosition` is not empty; it rings the bell.**
`Src/disbehaviortriggeralarm.cpp`. CG recorded (agentCG_status.csv line 239) that the retail vtable slot `+444` reached
from exec 2013 `0x5f5750` "is an empty body ... 2013 rva `0x1cb0c0`", and wrote a body that does nothing but
`P_FINISH`. `0x1cb0c0` is `nullsub_3`, a three-byte ICF-folded `ret` — the classic symptom of resolving a slot against
the wrong vtable base. Resolved against the `UDisBehaviorTriggerAlarm` vtable itself, located by raw dword scan for CG's
own two identified pointers (`0x6e4980` = `RequestStateExitCallback_GenericAction` and `0x6f6330` =
`OnExitCallback_GenericAction`, adjacent at `0xd295a8` and `0xd295ac`), the slot holds **2013 `0x6f62f0`**: 0x2b bytes,
the same size as 2012's `0x75f240`, calling the `FDisAISubStateGenericAction_Param` constructor and then
`RequestSubStateChange`. The NPC walked to the bell and stood there. Now it requests generic action `0x23` — the ring —
against slot 2's `TakePosition` tweaks, which is retail's own arrangement and the reason `RequestSubStateChange` takes a
slot index *and* a parameter: the slot selects the settings, the parameter selects the class, and they need not agree.
**Hand-over: retail slot arithmetic must be checked against a vtable located from two known pointers, never from a single
offset.** The retail/2012 offsets differ by exactly one slot on these classes (`456 → 460`, `440 → 444`), so an off-by-one
resolves to a *plausible* neighbour and, when that neighbour is folded with every other empty body in the image, looks
like proof that the function does nothing.

**DEFECT 4 — the base `GetResumingBodyIntentionDesire` returns FALSE, not TRUE.**
`Inc/CppText/IDisDesiresInterface.h`. The body is identical-code-folded at 2012 `0x723660`, where IDA names it
`FFileManagerError::MakeDirectory`; that function is `return 0`. Reading it as TRUE would make every resuming sub-state
re-request a zeroed `FDisBodyIntention`, blanking the NPC's stance and both its hands each time a behaviour returned to
slot 0.

**DEFECT 5 — `FArkGameEventDispatcher::CreateInstance` had never been called.**
`Launch/Src/LaunchEngineLoop.cpp`, after `appCleanFileCache()`. Retail calls it at 2013 `0x5e249d`, and that is the only
code xref to `0x5572d0` in the whole exe. Nothing in the tree had ever exercised the dispatcher, so it was turned on with
a measurement rather than on faith: `FDisAIArkEventProbe` / `DisAIArkEventSelfTest` in `Src/disaicensus.cpp` register,
dispatch, defer a registration *during* a dispatch, and unregister, and the census prints the six counters. Result:
`delivered 1/1 then 2/2, deferred registration delivered 0 then 1, after unregister 2 (expect 2 and 1)` — deferred
registration and unregister-during-dispatch both behave as retail's design requires.

## Retail-versus-2012 differences worth carrying forward

These are the traps this package hit. All three are cases where porting the 2012 name would have produced a tree that
compiles and is wrong.

1. **`UDisBehaviorAttentionBase` is new in 2013** (retail sizeof 176, reflected span 160..176; `off12.py` cannot find the
   type in the 2012 PDB at all). Retail hoisted the per-behaviour attention target — 2012's
   `UDisBehaviorNotice::m_NoticedProxy`, `UDisBehaviorSearch::m_SearchTargetProxy` and the combat behaviours'
   `m_EnemyProxy`, each at offset 160 of its own class — into one `FDisAttentionProxy m_AttentionTargetProxy` on a shared
   base that `Notice`, `Search` and `Combat` all derive from. Porting the 2012 names adds three members retail does not
   have.
2. **`EAIStimID` was renumbered.** Retail has 122 entries against 2012's 112: `AttentionBehaviorBegin` inserted at 7,
   `InhibitBegin` / `InhibitEnd` at 53 and 54, which moves 105 of the 112 ids 2012 already had. Stim masks must be written
   as enumerators, never as the numbers 2012 mask addresses imply. Proven on `Stand`'s own mask: its four bytes are 0, 9,
   28 and 76 apart in the 2012 image, its delegate switch names `DestinationReached`, `EndPossession`, `IncomingDamage`
   and `Teleported` = 2013 ids 24, 33, 52, 102, i.e. 0, 9, 28 and 78 apart. The first three agree because nothing was
   inserted below 52; the fourth is two out, which is exactly `InhibitBegin` and `InhibitEnd`.
   `build/agentDF_work/stimid.py --check` derives the shift rule (+1 below 52, +3 above) and it predicts 110 of 112 ids.
3. **`m_bHasSeenEnemyRecently` and `SetHasSeenEnemyRecently` do not exist in retail 2013.** 8 of that class's 17 2012
   functions have no 2013 match; the setter was removed rather than stubbed, with the reason recorded at the site.

## Cross-checks performed instead of assertions

* every one of the 12 sub-state stim masks resolves identically two ways — from the 2012 mask byte offsets and from the
  symbolic names in the class's `GetFilterStimDelegate_SubState` switch (`build/agentDF_work/checkmasks.py`, 12 of 12
  agree);
* all twelve desire priority constants land on their own enumerator across the four bands
  (Behavior < SubState < SubProcessBase);
* the `_pState[N].Field` → 2012 member translation used throughout (`build/agentDF_work/off.py`, stride 64 =
  `sizeof(UDishonoredNativeState)`) was validated against `FDisAISubStateStand_Param::OnPending`, where six independent
  fields land on six named members;
* `FDisLookAtInfluence`'s four constants recovered from folded targets: `Eyes(0,0,TRUE)`, `Head(1,0,TRUE)`,
  `Torso(1,1,TRUE)`, `TorsoSpeedIndependent(1,1,FALSE)`.

## Places this package stubs, and says so at the site

* the three Ark components (`FArkComponent{FaceTo,Locomotion,Lookat}`) — requests are accepted and counted, never
  executed. One seam, `DisDesireStructs`, in `Src/disdesirestructs.cpp`.
* `IDisConvSpeakerInterface::GetConvLookTarget` — a 30-slot interface in retail. Declaring one slot would put a false
  vtable in the tree, so the call site takes retail's "not speaking to anyone" branch behind a counted
  `DISHONORED(bringup)` note.
* `UDisBehaviorCombatWolfhound::SetHasSeenEnemyRecently` — removed, not stubbed: the member does not exist in 2013.

## Gates

* `build\agentDF_release.cmd` for `DishonoredGame`, `CoreSmoke` and `LayoutProbe`: green, and **zero warnings** — the last
  ones cleared were a batch of C4099 from `FArkGameEvent` being forward-declared `struct` in the AI CppText headers while
  `Engine/Inc/arkgameeventdispatcher.h` defines it as `class` (retail's own mangled names say `class FArkGameEvent`, so
  the headers were changed, not the definition).
* `python resources/tools/run_regression.py --build-dir build/agentDF --no-build` → **31 ok, 0 failed, 0 skipped**,
  `unported_natives` **0**, `probe_natives` **6**, all three `*_criticals` 0.
* 0 `Critical` in every game run.

## Generated files — read this before merging

This package changes files the DishonoredGame generator produces. The full command, run inside `build/agentDF_wt` by
`build/agentDF_work/regen.py`, always with all three flags:

```
python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake
```

Generated files now differing from HEAD: `Inc/DishonoredGameAICombatClasses.h`, `Inc/DishonoredGameAIWolfhoundClasses.h`,
`Inc/DishonoredGameSearchClasses.h`, `Inc/DishonoredGameNative.h`, `Src/DishonoredGameNativeStubs.cpp` and
**`Sources.cmake`**.

**`Sources.cmake` must be regenerated.** 28 units came off the `DishonoredGame_EXCLUDE` skeleton list:
`disaisubstatewithdesires.cpp`, `disdesiresinterface.cpp`, `disdesirestructs.cpp`, `disbehaviorguard.cpp`,
`disbehaviornotice.cpp`, and `disaisubstate{init,stand,takeposition,takeactorposition,stareatunreachable,combatbase,`
`genericaction,cower,menace,maintaindistance,meleechase,meleeengage,follow,tracktarget,lieinwait,doattractspell,`
`doweaponmanoeuver,whcombatshortdistance,whcombatlongdistance,investigate,flee,firepistol,findshootingposition}.cpp`.
The skeleton count in the header comment moves 863 → 834.

**DE's removal of `Src/dispostprocesscontrollers.cpp` from that list is preserved** — verified: the only line the
regeneration *adds* to `Sources.cmake` is the skeleton-count comment, and `dispostprocesscontrollers.cpp` does not
reappear.

## Files

Engine, outside `DishonoredGame` (three files, one behavioural line):
`Engine/Inc/arkgameeventdispatcher.h`, `Engine/Src/arkgameeventdispatcher.cpp` (six counters),
`Launch/Src/LaunchEngineLoop.cpp` (the `CreateInstance` call and its include).

`DishonoredGame`: 37 new `Inc/CppText/` headers (the 22 sub-state classes with bodies, the six desire structs, the two
interfaces, the three `WithDesires` mix-ins, and the behaviour headers the new callbacks needed); rewritten
`Inc/disaisubstate.h` (all 25 `_Param` structs, `FDisStimFilterMask`, the shared `GDisAIEvent_OtherActorTerminated = 3`,
and the two `UDishonoredAIBehavior::RequestSubStateChange` template definitions, which must sit after the sub-state
classes are complete); new `Inc/disdesirestructs.h` and `Src/disdesirestructs.cpp`; `Src/disdesiresinterface.cpp` (34
methods); `Src/disaisubstatewithdesires.cpp`; 24 `Src/disaisubstate*.cpp` units, each holding its class's bodies **and its
`_Param`'s bodies**, which is retail's own layout; eight `Src/disbehavior*.cpp` units; and the census extensions in
`Inc/disaicensus.h` / `Src/disaicensus.cpp`.

Two of CG's headers were edited: `Inc/CppText/UDisAISubState.h` and `Inc/CppText/UDisAISubProcess.h` (the `FArkGameEvent`
class/struct fix, and the six delegate-hook declarations the sub-states need).

The full list of 101 changed and new source paths is `build/agentDF_work/files.txt`; `build/agentDF_work/crlf.py` normalises all
of them to CRLF and is the last thing run before a build.

Tooling left behind, all in `build/agentDF_work/`: `natlookup.py` and `vtslot.py` (name a 2012 function, dump a vtable
with names, raw-scan retail for a pointer, decompile by rva — this is what caught DEFECT 3 and is the general answer to
"which retail function is behind this slot"), `off.py`, `stimid.py`, `checkmasks.py`, `classify4.py`, `mkstatus.py`,
`regen.py`, `crlf.py`, `mklist.py`, and the idempotent writer scripts.

## Hand-overs

1. **Make C4263 and C4264 errors** (DEFECT 1). One dead override cost this package a full build-and-measure cycle and
   would have shipped silently.
2. **Locomotion is the next bottleneck and it is now the *only* thing between the tower's NPCs and visible motion.**
   `FArkComponentLocomotion`, 117 functions, four nav-mesh entry points. The seam is `DisDesireStructs` and the counters
   already say what is being asked for: 26 open loco requests, 0 faceto, 0 lookat. Nothing else needs to change for the
   NPCs to start walking.
3. **`UDisAIBrainProcessAttention`** (~12 blocked natives) and **`FDisAIMonitorPawnReachability`** (6, and it needs the
   nav mesh) are the next two clusters; with the `_Param` and desire blockers gone they are the top of the list.
4. **Re-check any port that reasoned from a single vtable offset against retail.** Retail inserts one virtual on the
   behaviour classes, so 2012 offsets are +4 out, and an off-by-one slot lands on a folded empty body that reads as
   evidence. `build/agentDF_work/vtslot.py ptr <rva>` locates a vtable from two known pointers in a few seconds.
5. `ADishonoredSpawner`'s secondary vtable `FDisDialogSelNotify` is still undeclared, so the generator emits a
   polymorphic placeholder. Harmless today; it will matter to whoever ports the dialog selection path.
