# Agent AT — Kismet: the retail New Game route runs, the mission's sequences execute, and the Kismet-streamed level appears (2026-09-27)

Package AT of `PHASE7.md`. Three things were asked for, in order: agent AF's blocker **B2** (the menu teardown that
blocks the retail New Game route), **Kismet-driven streaming** (`L_Tower_Water`, a `LevelStreamingKismet` that nothing
drives), and a **census** of the sequence ops the first mission actually executes.

Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentAT.i64`, a copy of `retail2013_named.i64`); every
"2013 rva" is from a headless decompile in `build/agentAT_decomp`.

## Result

| Acceptance | State |
|---|---|
| `-newgame` reaches the mission map with 0 critical errors | **done**. `ce ChangeLvl_StartNewGame` → `SeqEvent_Console` → `SeqAct_PrepareMapChange` → `Committed map change … committed l_tower_p`, all 8 `L_Tower_*` levels visible, `levels 9`, `1 controllers (1 player)`, **0** `Critical` over 150 s (`agentAT_t11.log`, `build/agentAT_smoke11.txt`) |
| `L_Tower_Water` becomes visible when its Kismet trigger fires | **done**. `SeqAct_MultiLevelStreaming_0` (in `L_Tower_Script…Main_Sequence.SOIREE_Boat`) fires → `activated level L_Tower_Water: shouldBeLoaded 1 shouldBeVisible 1` → one second later `streaming level L_Tower_Water (LevelStreamingKismet) loaded 1 visible 1`, `levels 10` (`agentAT_t14.log`). The action is reached through `-diskismetforce` because its own branch waits behind a latent audio action — **named blocker below**, not a streaming defect |
| a census line counts sequence ops executed per tick and events fired | **done**, `-diskismet`: ops executed / activated / deactivated / latent / re-activated, outputs followed, delayed activations queued and fired, step caps, events fired / checked / queued / registered, per-class histograms of both, the ops active *right now*, the ops ever activated per level, and a one-shot inventory of the whole sequence tree |
| `Attach to Event SeqAct_AttachToEvent_3 has no targets!` accounted for | **done**: the warning now names the link — `Attachee links: L_Tower_Script…Main_Sequence.DisSeqVar_PlayerPawn_23 = None`. `UDisSeqVar_PlayerPawn::GetObjectRef` (2013 **0x7f4fc0**) is an unported comment stub and `execGetObjectValue` is a `DISHONORED_NATIVE_STUB`, so every `DisSeqVar_PlayerPawn` in the game resolves to None. Agent AU's |
| no regression on the `-startmapopen` route | **done**: `inputtest moved 1017.4`, 0 `Critical` (`agentAT_t15.log`) |

## 1. Agent AF's B2 — the menu map change destroyed the whole persistent level

Both earlier readings were symptoms. Agent AL's `USequence::ExecuteActiveOps -> UObject::IsA` and agent AO's
`FMallocBinned::GetAllocationInfo` are gone in this tree (agent AO's `/Zc:alignedNew-` landed); what was left was
**silence**: after `Committed map change via DishonoredEngine` the game ticked the menu world for ever and
`-newgame`'s `ce ChangeLvl_StartNewGame` did nothing at all, not even a "Command not recognized".

Measured, in this order:

1. `-diskismet`'s inventory: `GetGameSequence` resolves, the 53 `SeqEvent_Console` of `DishonoredGameFull_P` are all
   there and registered, `console event 'ce ChangeLvl_StartNewGame' … (enabled 1, registered 1, triggers 0/0)`.
2. The census counter for script `FindSeqObjectsByClass` never moved after the command, so
   `PlayerController.ServerCauseEvent` never ran.
3. `UGameEngine::Tick`'s `-newgame` branch: `GamePlayers 1, GamePlayers(0) DisLocalPlayer_0, its Actor NONE` and the
   world's controller list **empty**. `APlayerController::ConsoleCommand` (2013 **0x1f51d0**) returns immediately when
   `Player == NULL`, and agent AF's switch falls back to `UGameEngine::Exec`, which has no `ce`, hence the silence.
4. The 1 Hz census: `1 controllers (1 player)` before the commit, `0 controllers` after it.
5. `UWorld::CleanUpBeforeLevelTransition` was destroying **everything**: `WorldInfo_0`, `PlayerStart_0`,
   `DynamicBlockingVolume_1`, `DefaultPhysicsVolume_0`, `DishonoredGameInfo_0`, `BroadcastHandler_0`,
   `DishonoredEmitterPool_0`, `DisDecalManager_0`, `GameReplicationInfo_0`, `DishonoredPlayerController`,
   `DishonoredPlayerCamera_0`, `DishonoredPlayerPawn`, `DishonoredHUD_0`.

### The retail gate (2013 rva 0x258bb0)

```c
if( (*(_BYTE *)(Actor + 292) & 8) != 0 )              // bKillDuringLevelTransition
{
    v16 = *(_DWORD *)(Actor + 300);
    if ( (v16 & 1) != 0 && (v16 & 2) == 0 )           // m_bSpawned && !m_bPersistsAcrossLevelTransition
        DestroyActor( Actor, FALSE, TRUE );
}
```

`bKillDuringLevelTransition` is **TRUE on every actor the cook produces** — Arkane inverted the reference's policy and
made the two Arkane bits the real discriminator. `m_bSpawned` (@300 mask 0x1) is written by `UWorld::SpawnActor`
(store at **0x256bb5**, `or dword ptr [esi+12Ch], 1`) and was never written here at all;
`m_bPersistsAcrossLevelTransition` (@300 mask 0x2) is what the cook sets on the things that must survive. With both
ported the same transition reads:

```
level transition keeps WorldInfo … (spawned 0, persists 0)          <- level-placed, never spawned
level transition keeps DishonoredGameInfo_0 … (spawned 1, persists 1)
level transition keeps DishonoredPlayerController … (spawned 1, persists 1)
level transition keeps DishonoredPlayerPawn … (spawned 1, persists 1)
level transition left 1 controllers
```

and the chain unblocks itself: the player controller survives → the viewport draws a view → `bShouldBeVisible` comes
out TRUE for the new fake persistent level in `UWorld::UpdateLevelStreaming` → `levels 1` becomes `levels 4` and the
menu is actually in the world → `ConsoleCommand` finds `Player` → `Engine.PlayerController:CE` →
`ServerCauseEvent` → `FindSeqObjectsByClass(SeqEvent_Console, recursive 1) -> 53 objects` →
`SeqEvent_Console` fires → `SeqAct_PrepareMapChange` / `SeqAct_CommitMapChange` →
`committed l_tower_p` with all eight sub-levels.

The rest of retail's `CleanUpBeforeLevelTransition` is **not** ported and is listed under follow-ups
(`UAkAudioDevice::StopAllSounds`, `AGameInfo::FlushCrowdAgents` at vtable +996 = 2013 0x5ea010, two `AWorldInfo`
manager teardowns, and two `TMap<UClass*,FNavMeshPathGoalEvaluatorCacheDatum>` clears on `UWorld`).

## 2. `USequence::ExecuteActiveOps` against retail (2013 rva 0x300740)

Unnamed in the idb; found through `USequence::UpdateOp` (0x301550), which turned out to be an exact match already.
Three deltas, all retail's:

**a. Bend time.** Retail scales the delta:

```c
GameInfo = GWorld->GetGameInfo();
BendTimeDeltaTime = GameInfo ? GameInfo->GetBendTimeDilation(FALSE) * DeltaTime : DeltaTime;   // vtable +976
...
Delay -= Op->m_bAlwaysOutOfBendTime ? DeltaTime : BendTimeDeltaTime;                            // @140 mask 0x400
Op->bActive = !Op->UpdateOp( Op->m_bAlwaysOutOfBendTime ? DeltaTime : BendTimeDeltaTime );
```

`ADishonoredGameInfo::GetBendTimeDilation` (**0x5e9e90**) is
`bPlayer ? 1.0f : m_fCurrentWorldTimeDilation / m_fCurrentPlayerTimeDilation` (`AGameInfo` @936 / @940, both already
present in `EngineClasses.h`), so it is inlined at its one call site with a divide-by-zero guard retail does not need.
`m_bAlwaysOutOfBendTime` is the Arkane `SequenceOp` bit at @140 mask 0x400 ("tick at real time even while Bend Time
slows the world down"), verified against the 105-bit `AActor`/`USequenceOp` orders in
`resources/docs/types/retail_sdk_layout.json`.

**b. `USeqAct_Latent` does not exist as a special case.** Retail's `ExecuteActiveOps` contains no `IsA` call at all.
`script_classes_2013.json` confirms why: the 2013 `Sequence` class has **no `DelayedLatentOps`** and the 2013
`SeqAct_Latent` has **no `LatentActivationTime`** — both are reference-only. In this tree `DelayedLatentOps` is a
`DISHONORED_SHIM_STATIC` in `EngineSequenceClasses.h`, i.e. **one array shared by every sequence of every world**:

```c
while(DelayedLatentOps.Num() > 0) QueueSequenceOp(DelayedLatentOps.Pop(), FALSE);   // reference
...
if(NextOp->IsA(USeqAct_Latent::StaticClass())) { ... DelayedLatentOps.Push(LatentOp); continue; }
```

A latent op pushed while the main-menu level was up is popped and queued again by the next world's sequence, and the
`IsA` on that freed op is exactly agent AL's `ExecuteActiveOps -> UObject::IsA` access violation. All three blocks and
the drain are gone.

**c. The step limits** are tested before the pop (`(MaxSteps != 0 && Steps >= MaxSteps) || Steps + 1 >= 1000`) and
retail emits no warning when they bite; the census counts the caps instead. The reference's
`"Op %s still active while bLatentExecution == FALSE"` warning, which retail also does not have, became a counter.

Everything else — the delayed-activation sweep, `ActivateInputLink`, `OnReceivedImpulse(ActivatorOp, InputIdx)`, the
output-link walk with `ActivateDelay` and `QueueDelayedSequenceOp`, the input/output clearing with `QueuedActivations`,
the `NewlyActivatedOps` drain with `QueueSequenceOp(Op, TRUE)`, `PostDeActivated`, the `QueuedActivations` unqueue when
`ActiveSequenceOps` empties, and the `ActiveLatentOps` re-queue — matches retail field for field and offset for offset
(`FActivateOp` 0/4/8/12, `FSeqOpOutputLink` 56 bytes with `bHasImpulse` @24 and `ActivateDelay` @32,
`FSeqOpInputLink` 48 bytes with `bHasImpulse` @12, `QueuedActivations` @16 and `ActivateDelay` @36,
`FQueuedActivationInfo` with `ActivateIndices` @12 and `bPushTop` @24, `ActivateCount` @216).

## 3. Kismet-driven streaming: `L_Tower_Water`

The whole streaming path was already correct; nothing was driving it.

* `L_Tower_P.upk` holds 7 `LevelStreamingAlwaysLoaded` and exactly one `LevelStreamingKismet_0` (`L_Tower_Water`).
  `ULevelStreamingKismet::ShouldBeLoaded` (**0x3813e0**) is `return bShouldBeLoaded` and `ShouldBeVisible`
  (**0x3813b0**) is `bShouldBeVisible || (bShouldBeVisibleInEditor && !GIsGame)`, so such a level is driven *only* by
  a Kismet action — unlike the always-loaded ones, whose virtual returns TRUE.
* The driver is a single op: `L_Tower_Script.upk` export 17505,
  `TheWorld.PersistentLevel.Main_Sequence.SOIREE_Boat.SeqAct_MultiLevelStreaming_0`, with `L_Tower_Water` in its
  `Levels` array and `bMakeVisibleAfterLoad` set. `SOIREE_Boat` is the intro-boat sub-sequence and is entered through
  its two `SeqEvent_RemoteEvent`.
* `USeqAct_MultiLevelStreaming::Activated` (**0x2e5f90**), `UpdateOp` (**0x2dc370**),
  `USeqAct_LevelStreamingBase::ActivateLevel` (**0x2dc230**) and `UpdateLevel` (**0x2cf700**) were all checked against
  their decompiles and match. Two unconditional `debugf` calls the reference left in `ActivateLevel` and
  `MultiLevelStreaming::Activated` (which fired for every player on every active frame, and which retail does not have
  at all) are now `DISHONORED(bringup)` lines behind `-diskismet`.

With the action driven, the level loads and becomes visible one streaming update later:

```
[0015.59] kismet census: forcing input 0 of L_Tower_Script…SOIREE_Boat.SeqAct_MultiLevelStreaming_0
[0015.59] kismet census: L_Tower_Script…SeqAct_MultiLevelStreaming_0 activated level L_Tower_Water:
          shouldBeLoaded 1 shouldBeVisible 1 blockOnLoad 0 (load impulse 1, unload impulse 0)
[0016.62] kismet inventory: streaming level L_Tower_Water (LevelStreamingKismet) loaded 1 visible 1
          shouldBeLoaded 1 shouldBeVisible 1 blockOnLoad 0 loadPending 0
[0091.13] kismet census: world: … levels 10 …
```

### Why its own branch does not reach it yet — one named blocker

The census's "ops active now" line answers it in one line. Over a whole `-newgame` run exactly **two** ops are ever
left active, for ever:

```
2 ops active now: DishonoredGameFull_P…Main_Sequence.SeqAct_Interp_0 (SeqAct_Interp, latent 1, activations 1),
                  l_tower_p…Main_Sequence.SeqAct_AkPostEvent_0 (SeqAct_AkPostEvent, latent 1, activations 1)
```

* **`SeqAct_AkPostEvent_0`** is the blocker for the mission chain. `USeqAct_AkPostEvent::UpdateOp`
  (`AkAudio/Src/akaudioclasses.cpp:340`) is `return Signal == 0`, and `Signal` only comes down when `AK_EndOfEvent`
  reaches `AkCallback`. The silent Wwise backend holds every playing id until `StopAll` / `Term`
  (`External/Wwise2012/Src/AkSilentSoundEngine.cpp`, its own header comment says so), and — measured — **nothing in the
  tree ever runs an audio frame**: `AK::SoundEngine::RenderAudio()` has no caller anywhere in
  `source/Development/Src`, because retail's owner of the audio frame is `UDishonoredAudioSystem::Update` (reached from
  `UAkAudioDevice::Update`, 2013 0x5aec80) and that is still a stub; `UAkAudioDevice::Update` itself is never called
  either, because `UWindowsClient::Init` only builds an audio device when `GEngine->bUseSound`
  (`WinDrv/Src/WinClient.cpp:353`). So the callback can never arrive, the latent action never finishes, its output link
  is never followed, and `l_tower_p`'s Kismet stops at it — 25 of 1670 ops ever activated, 14 of 1629 in
  `L_Tower_Script`, and `SOIREE_Boat` never entered. Two candidate fixes were built and measured and then **reverted**
  as out of this package's scope (audio is out of scope for the wave, `PLAN.md` Phase 10): ending a posted event on the
  next `RenderAudio` in the silent backend, and calling `RenderAudio` from `UAkAudioDevice::Update`. Both are correct;
  neither helps until something ticks the audio device. **Hand-over, with the exact shape, below.**
* **`SeqAct_Interp_0`** of `DishonoredGameFull_P`'s own Main_Sequence is agent AI's **B1**, the zero-length matinee:
  `InterpLength == 0`, so `USeqAct_Interp::UpdateOp` never reports completion. It is in the menu's persistent Kismet
  and does not block the mission, but it is 2,000 pointless op executions per 100 s and it still needs
  `UnInterpolation.cpp`.

## 4. What the first mission's Kismet actually contains and executes

`-diskismet`'s one-shot inventory, re-run whenever the world's level count changes. After the New Game commit the
world holds 9 sequences (the menu's persistent `Main_Sequence`, which stays the *real* persistent level because a
UE3 seamless map change installs the new map as a `ULevelStreamingPersistent` "fake persistent level", plus one per
`L_Tower_*` level): **DishonoredGameFull_P 2,777 ops, l_tower_p 1,670, L_Tower_Script 1,629, L_Tower_Fx 7,
L_Tower_Audio 10, L_Tower_Env 2, L_Tower_Block / Light / Nav 0.**

`L_Tower_Script` alone declares 138 distinct sequence classes. The ones that **execute** on the New Game route
(`ops executed by class`, 150 s, `agentAT_t11.log`) are:

| op | executions | note |
|---|---|---|
| `SeqAct_Interp` | 1,868 | one stuck latent matinee (B1), re-queued every frame |
| `SeqAct_AkPostEvent` | 1,851 | one stuck latent audio action, re-queued every frame |
| `SeqAct_StreamInTextures` | 399 | latent by design, completes |
| `SeqAct_CameraFade` | 44 | |
| `SeqEvent_LevelLoaded` | 18 | fired 16 times |
| `SeqAct_PrepareMapChange` | 17 | the menu's map-change fan-out |
| `SeqAct_ActivateRemoteEvent` / `SeqEvent_RemoteEvent` | 6 / 6 | fired |
| `DisSeqCond_IsSentinel` | 5 | ported (`disseqcond_issentinel.cpp`) |
| `Sequence`, `SeqEvent_SequenceActivated` | 3 / 3 | |
| `SeqAct_SetMatInstScalarParam`, `SeqAct_Toggle` | 3 / 3 | |
| `SeqAct_ToggleHUD`, `SeqAct_CommitMapChange`, `SeqAct_ToggleCinematicMode`, `SeqAct_AkStartAmbientSound`, `SeqAct_DivideFloat`, `DisSeqAct_ToggleTutorial`, `DisSeqAct_SetStoryFlag` | 2 each | |
| `SeqEvent_Console`, `SeqAct_Gate`, `SeqAct_CastToFloat`, `SeqAct_SubtractFloat`, `SeqCond_CompareFloat`, `SeqAct_AttachToEvent`, `DisGFxActionOpenMovie`, `DisSeqAct_DiscardAllLevelStates`, `DisSeqAct_DiscardLevelState`, `DisSeqAct_ToggleAchievementEval`, `DisSeqAct_SetCurrentChapter`, `DisSeqAct_SetPlayerTravelDestination`, `DisSeqAct_ToggleChoke`, `DisSeqAct_ToggleJournal` | 1 each | |

and the ops that were reached inside `L_Tower_Script` are named individually by the census
(`DisSeqCond_IsSentinel_2/_4`, `DisSeqAct_ToggleChoke_0`, `DisSeqAct_ToggleJournal_0`, `SeqAct_AttachToEvent_3`, then
`SOIREE_Empress_Regent`'s `SeqAct_Toggle_15`, `SeqAct_SetMatInstScalarParam_35`, `SeqAct_Toggle_99` and
`FloatAnimation_2`'s float chain).

**Stubbed on this path** — every one a DishonoredGame native, i.e. agent AU's:

| 2013 rva | what | consequence |
|---|---|---|
| 0x7f4fc0 | `UDisSeqVar_PlayerPawn::GetObjectRef` (`disseqvar_playerpawn.cpp` is a comment stub; `execGetObjectValue` is a `DISHONORED_NATIVE_STUB`) | every `DisSeqVar_PlayerPawn` reads None — 42 of them in the first mission — which is the `SeqAct_AttachToEvent_3 has no targets!` warning, and it will silently empty every Attachee, Target and Instigator link that uses the player pawn |
| — | `UDisSeqAct_ToggleChoke`, `UDisSeqAct_ToggleJournal`, `UDisSeqCond_IsSentinel`, `UDisSeqAct_SetStoryFlag`, `UDisSeqAct_DiscardLevelState`, `UDisSeqAct_SetCurrentChapter`, `UDisSeqAct_SetPlayerTravelDestination`, `UDisSeqAct_ToggleTutorial`, `UDisSeqAct_ToggleAchievementEval` | executed on this path; each is a `IMPLEMENT_CLASS` with a comment-stub unit, so they activate, do nothing and pass their output link on. They do not block the chain |
| — | `DisGFxActionOpenMovie` | executes once; GFx is agent AW's decision doc |

Nothing in `UnSequence.cpp`'s engine op set needed porting beyond `ExecuteActiveOps`: every engine op the first
mission executes has a real body, and the level-streaming family was verified line by line against its decompiles.

## `-diskismet`, the instrumentation

One switch, off by default, free when off, re-armed for every new world (so the mission map is measured, not the
menu map). Driven from `USequence::UpdateOp` for root sequences, reporting once per second:

* **the counters**: sequence ticks, ops executed / activated / deactivated / latent-carried / still-active /
  re-activated, output links followed, delayed activations queued and fired, step caps, events fired / checked /
  queued / registered, and script `FindSeqObjectsByClass` calls (which is how `ce <name>` finds its event);
* **per-class histograms** of ops executed and events fired — the answer to "what does this map's Kismet run";
* **the ops active right now**, with path, class, `bLatentExecution` and `ActivateCount` — the line that names a
  stalled latent action in one look, and the single most useful thing in this package;
* **ops ever activated per level**, and, for a level with ≤ 40 of them, each one by path — how far a chain got;
* **the world**: controllers, player controllers, `GamePlayers(0)` and its `Actor`, level count, game sequence and
  `CurrentLevelPendingVisibility` (the `AddToWorld` state machine);
* **a one-shot inventory**, re-run whenever the level count changes: every root sequence with its object and nested
  counts, the class histogram of everything present, every `SeqEvent_Console` with its `ce` name and trigger state,
  every level-streaming action with the `ULevelStreaming` it resolves to, every `SeqAct_AttachToEvent` with its
  attachee / event / variable-link counts, every map-change op, and every `ULevelStreaming` of the world with its
  seven state bits;
* **level-transition lines** in `UWorld::CleanUpBeforeLevelTransition`: what the transition destroys and keeps, with
  the three gate bits.

Two more switches, both `DISHONORED(bringup)` and both off by default:

| switch | effect |
|---|---|
| `-diskismetforce=<substring>[,<substring>]` | fires input 0 of every sequence op whose path contains one of the substrings, once, through `USequenceOp::ForceActivateInput`. The Kismet counterpart of agent AS's `-distouchprobe`: it drives an op whose own branch is not walkable yet. `-diskismetforce=SeqAct_MultiLevelStreaming_0` is what proves the Kismet streaming path |
| `-diskismetforcedelay=<seconds>` | when, after the world's first Kismet tick, that happens (default 12) |

## Runs

Built from the HEAD snapshot `build/agentAT_wt` (`python resources/tools/make_snapshot.py AT --sync`) into
`build/agentATwt` with `build/agentAT_wt_release.cmd` (Release, same options as `resources\build-release.cmd`, only
the source dir differs). The shared tree was used until agent AV's in-flight anim edits broke it
(`DishonoredGame/Inc/CppText/UDisAnimStatePool.h`, 118 errors, none in my files); everything after that is the
snapshot. 805 units, **0 errors**.

```
python resources\tools\build_and_smoke.py --build-dir build/agentATwt --no-build --exe-name DishonoredGame_AT.exe ^
  --log-name agentAT.log --ini-dir build/agentAT/config --rhi d3d9 --timeout 120 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-newgame -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

Add `-diskismet` for the census and `-diskismetforce=SeqAct_MultiLevelStreaming_0` for the `L_Tower_Water`
proof. `build/agentAT` is the shared-tree build (stale once agent AV broke that tree); `build/agentATwt` is the
snapshot build every number here comes from.

| run | what | result |
|---|---|---|
| `agentAT_b0.log` | baseline `-newgame`, shared tree, before any change | menu commit at 23.3 s, `ce ChangeLvl_StartNewGame` issued at 25.1 s, **nothing happens** for the next 95 s |
| `agentAT_t1/t2/t3/t4/t5.log` | the B2 diagnosis, in five steps | see §1 |
| `agentAT_t9.log` | first run with the B2 fix | `committed l_tower_p` at 17.2 s, 8 sub-levels visible, 120 s, 0 `Critical` |
| `agentAT_t11.log` | the accept run | 150 s, 0 `Critical`, `levels 9`, `1 controllers (1 player)` |
| `agentAT_t12.log` | per-level activation lists | named the 14 `L_Tower_Script` ops that ran |
| `agentAT_t14.log` | `-diskismetforce=SeqAct_MultiLevelStreaming_0` | `L_Tower_Water loaded 1 visible 1`, `levels 10`, 0 `Critical` |
| `agentAT_t15.log` | `-startmap=L_Tower_P -startmapopen -inputtest` regression | `inputtest moved 1017.4`, physics 1, 0 `Critical` |
| `agentAT.log` | **the accept command exactly as the brief gives it**, no census switches | exit 0, `expect ok: Initial startup`, `expect ok: no Critical`, and **two** `Committed map change via DishonoredEngine` lines — the menu's at **3.93 s** and the mission's at **6.21 s** — which is the shape of the golden 2012 log (`2012_arkprofile_launch.log` :365 and :389, where the second one waits 83 s for a human to click New Game) |

## What changed, per file

| File | Change | Evidence |
|---|---|---|
| `Engine/Src/UnSequence.cpp` (+561/-53) | `USequence::ExecuteActiveOps` to retail; the `-diskismet` census, its inventory and `-diskismetforce`; the two reference `debugf` spams in the level-streaming actions gated; the `SeqAct_AttachToEvent` warning names its links | 2013 0x300740, 0x5e9e90, 0x2dc230, 0x2e5f90 |
| `Engine/Src/UnLevAct.cpp` (+42/-5) | `UWorld::CleanUpBeforeLevelTransition`'s three-bit gate and `DestroyActor(Actor, FALSE, TRUE)`; `m_bSpawned` in `UWorld::SpawnActor`; `TRACE_DisTouchOverlap` in both of `MoveActor`'s trace-flag initialisations (agent AS's hand-over) | 2013 0x258bb0, 0x256bb5, 0x24cf23, 0x24e14a |
| `Engine/Src/UnGame.cpp` (+22/-1) | `DISHONORED(bringup)` only: agent AF's `-newgame` branch says which controller runs the command, what its `Player` is, whether the `CE` exec function was found, and what the command returned | — |

`Engine/Inc/EngineSequenceClasses.h` needed no change: every offset the retail decompiles use is already right, and
`DelayedLatentOps` stays as the shim it is (nothing references it any more, so it can be deleted by whoever next
regenerates that header). `Engine/Src/UnLevel.cpp` needed no change either — the Kismet-streaming path is in
`UnSequence.cpp` and `UnWorld.cpp`, and `UnLevel.cpp`'s only part in it (`ULevel::GetGameSequence`) is correct.
Agent AR is therefore free of the hand-over: **`UnLevel.cpp` is untouched by AT.**

`Engine/Src/UnGame.cpp` and `Engine/Src/UnLevAct.cpp` are outside the file list the package brief named, flagged here
for the coordinator: `UnGame.cpp` is agent AF's (bring-up logging only, and B2 is AF's blocker), `UnLevAct.cpp` is
agent AQ's from wave 4 and agent AS explicitly handed its `MoveActor` trace flags to whoever next touched it.
Neither belongs to a wave-5 package.

## Follow-ups outside this package

1. **The audio frame** (agent AN's / whoever owns Phase 10). `AK::SoundEngine::RenderAudio()` has no caller in the
   tree; `UAkAudioDevice::Update` (2013 0x5aec80) is never called because `UWindowsClient::Init` only creates an audio
   device when `GEngine->bUseSound`. Until something ticks it, the silent backend fires no `AK_EndOfEvent`, and
   `USeqAct_AkPostEvent` — a latent Kismet action — stalls the mission's sequence for ever. The two-part fix, built
   and measured in this package and then reverted: end a posted event on the next `RenderAudio` in
   `AkSilentSoundEngine.cpp` (a deferred list, not a synchronous callback — `PlayEventOnTargets` increments `Signal`
   *after* `PostEvent` returns), and give `UAkAudioDevice::Update` the `RenderAudio()` call retail's audio system owns.
   `build/agentAT_patch11.py` and `build/agentAT_patch12.py` hold both, ready to re-apply.
2. **Agent AI's B1**, the zero-length matinee (`UnInterpolation.cpp`): `DishonoredGameFull_P`'s `SeqAct_Interp_0` has
   `InterpLength == 0` and is active for the rest of the run. Now visible as a census line rather than a hang.
3. **`UDisSeqVar_PlayerPawn::GetObjectRef`** (2013 **0x7f4fc0**) and `execGetObjectValue` — agent AU. Every
   `DisSeqVar_PlayerPawn` reads None, so the player pawn is missing from every Kismet link that uses it.
4. **`AGameInfo::GetBendTimeDilation`** is not declared in `EngineClasses.h`; its body is inlined in
   `ExecuteActiveOps` instead. Whoever owns that header should declare the virtual (vtable +976) and let
   `ADishonoredGameInfo` override it (2013 0x5e9e90), together with `IsBendTimeOn` (+980, 0x5e9f10) and
   `GetBendTimeDuration` (+972, 0x609c40).
5. **The rest of `UWorld::CleanUpBeforeLevelTransition`** (2013 0x258bb0): `UAkAudioDevice::StopAllSounds`,
   `AGameInfo::FlushCrowdAgents` (vtable +996, `ADishonoredGameInfo` 2013 0x5ea010), the two `AWorldInfo` manager
   teardowns at @1224 (vtable +944) and @1208, and two `TMap<UClass*,FNavMeshPathGoalEvaluatorCacheDatum>` clears on
   `UWorld`. None of them is needed for the route to work.
6. **`AActor::m_ActorTypeFlags`** (BYTE @266) again: retail's `ActivateLevel` and `MultiLevelStreaming::Activated`
   identify a player controller with `type byte == 20` rather than a cast. Same finding as `agentAS.md` follow-up 3.
7. **`USequence::BeginPlay`** (2013 0x300120) still differs: retail raises `SeqEvent_SequenceActivated` through
   `ActivateEvent` behind its own inline gates where ours calls `CheckActivateSimple`. Nothing was observed to
   misbehave.
8. **`SeqAct_MultiLevelStreaming_0` is reachable twice** from `-diskismetforce` because `UWorld::AddToWorld` adds a
   sub-level's sequence to the persistent root's `NestedSequences`, so walking any root reaches it. Harmless
   (`ForceActivateInput` is idempotent within a frame), noted so nobody reads the triple log line as a bug.

## Files

Mine (3 source + 2 docs): `Engine/Src/{UnSequence.cpp,UnLevAct.cpp,UnGame.cpp}`, plus this report and
`agentAT_status.csv`.

Scratch (not repo tools): `build/agentAT_patch{1..13}.py` (11 and 12 are the reverted audio fix, kept as the
hand-over), `build/agentAT_release.cmd`, `build/agentAT_wt_release.cmd`, `build/agentAT_names.py` (name-table grep
over cooked packages), `build/agentAT_{find,vtab,dis,disp}.py` (headless idalib helpers beside
`resources/tools/ida/decompile_funcs.py`: name search, vtable slot dump, a disassembly window, and a
displacement-operand search — `agentAT_disp.py` is what found the `m_bSpawned` store, worth promoting to
`resources/tools/ida/` next to agent AS's `agentAS_imm.py`), `build/agentAT_build{0..15}.log`,
`build/agentAT_smoke{0..15}.txt` + `agentAT_smoke_accept.txt`, `build/agentAT_decomp/` (27 decompiles), the `*.bak` copies of the four files that
were edited and reverted, snapshot `build/agentAT_wt` + build dir `build/agentATwt` (no stage directory, no
junction), IDA copy `resources/docs/idb/retail2013_agentAT.i64`. Logs in the retail `DishonoredGame/Logs`:
`agentAT_b0.log`, `agentAT_t1.log` … `agentAT_t15.log`.

`resources/play.cmd` and `resources/build-release.cmd` were not touched. No commits, no `git add`.
