# Agent FR — the conversation system's direct path: a DisConv_Soiree node starts its matinee

Wave 21 (`PHASE17.md`), package FR. Worktree `build/agentFR_wt`, base commit `d7d083a`, with main's `e60c5c6` (the
coordinator's fullscreen / foreground / gamepad commit), `4390017` and FS's `d24f1bf` (the matinee AI-group runtime) merged in
by `patch -p1` before the FR5 / FR6 runs as asked. Two builds:
the working build in `build/agentFR_rel` and the gate's in-harness build; the shared control at `build/head_rel` (HEAD
`80cc219`), staged as `DishonoredGame_FRctl.exe`. Every launch went through `game_input.py launch` with `-inputcmd`, and
every launch passed `-windowed` explicitly (the coordinator's rule arrived mid-package; no run of this package was ever
without it).

## 1. The headline

**The boat ride starts on its own.** On the `-finewgame=50` route, with no input at all, `SOIREE_Boat.DisSeqAct_DialogInputs_10`
fires `Intro` on `DisDialogOneShot_8`, the ported path walks hook -> component -> bound tree -> running instance ->
`DisConversation_3` -> `DisConv_Soiree_1` -> `USeqAct_Interp::StartFromDialog`, and **`SOIREE_Boat.SeqAct_Interp_1` goes
from `activations 0` to `activations 1`**, plays its 94.23 s and ends through the node (`SeqAct_Interp_1 finished at position
94.23`, instance stopped, `EndDialog` on all three speakers). The scene it unlocks follows the same way: the boat's
`Completed` reaches `SOIREE_Boat.DisSeqAct_DialogInputs_1` (`Boat_LiftedUp`, `DisDialogOneShot_9`) and
**`SOIREE_Workers.SeqAct_Interp_3` goes `0 -> 1`** (35.88 s). Three runs in a row on the merged tree (FR6, FR7, FR8), same
numbers within a second of load jitter; the control `build/head_rel` (FRC1) shows both at `activations 0` at 164 s:

```
FRC1 (head_rel, control)   [0061] SOIREE_Boat.SeqAct_Interp_1 ... activations 0        SOIREE_Workers.SeqAct_Interp_3 ... activations 0
FR8  (this tree)           [0062.92] frdiag inputs SOIREE_Boat.DisSeqAct_DialogInputs_10: 1 target(s) ... impulsed 'Intro'
                           [0062.92] frdiag soiree ...DisDialogOneShot_8...DisConversation_3.DisConv_Soiree_1: StartFromDialog on ...SOIREE_Boat.SeqAct_Interp_1 (blocking 1, activations 0)
                           [0123.37] frdiag inputs SOIREE_Boat.DisSeqAct_DialogInputs_1 ... impulsed 'Boat_LiftedUp'   -> StartFromDialog on SOIREE_Workers.SeqAct_Interp_3
                           [0157.17] frdiag soiree ...: SeqAct_Interp_1 finished at position 94.23
                           [0159.28] frdiag soiree ...: SeqAct_Interp_3 finished at position 35.88
FR7 second pass [0338.44]  SOIREE_Boat.SeqAct_Interp_1 playing 0 pos 94.23 activations 1 | SOIREE_Workers.SeqAct_Interp_3 pos 35.88 activations 1
                           SOIREE_Empress_Regent.SeqAct_Interp_12 playing 0 pos 0.00 activations 0
```

The captures agree: the control at 164 s sits in the boat at the starting pier (`frc1`, Samuel standing in the bow, the river
behind him); FR6 at 149 s and FR8 at 172 s are inside the water-lock hall at the end of the ride, the boat moored under the
"Slaughterhouse Row" sign, Samuel on the dock. What the guard and Samuel do *during* the ride is FS's runtime: their groups
bind (`fkdiag` lists the AI groups with pawns and `MatineeStackNode` channels, FR8 `0 MAT_BeginAIGroup` warnings), the
player group warns once each for `MAT_BeginPlayerGroup` / `MAT_SetForceInventory` / `MAT_FinishPlayerGroup` / `MAT_BlendOut`
(not ported, FK's package), and the `SkeletalMeshActorMAT` groups (`Boat`, `Wheel`, `drawbridge_01`) warn that they have
no `BeginAnimControl` — the boat itself moves by its Move track, so the ride is seen; the skeletal props do not animate.
No `Critical`, no minidump, in any FR run.

**The Empress does not start, and it is not the conversation system that stops her.** `SOIREE_Empress_Regent.SeqAct_Interp_12`
stays at `activations 0` in every run, 330 s included (FR7's second pass). FO showed it has no Kismet input; it is played by
`DisDialogOneShot_0`'s conversation 8, and FR8 traced every `DisSeqAct_DialogInputs` under `SOIREE_Empress_Regent`
(`-foupstream=SOIREE_Empress_Regent.DisSeqAct_DialogInputs -foupstreamdepth=4`, 10 ops, all `activations 0`): their entry
inputs are fed by `SeqEvent_RemoteEvent_34 -> SeqAct_Gate_14 -> DisSeqAct_DialogInputs_8` (the remote event is `enabled 0`
until something upstream enables it), `SeqEvent_RemoteEvent_86/87 -> SeqAct_Gate_1 -> DialogInputs_1`, and
`DisSeqAct_AIGoToActor_13 'Success (Last)' -> DialogInputs_25 '3Days_Forced'` (`DisDialogOneShot_2`, the captain at the
dock, an AI move the tracer calls "NO feeder"). Between the lock and the Empress the retail game needs the player to walk
up the dock stairs through the captain's and the guards' scenes; on an unattended route nothing walks, and the AI-goto /
trigger-volume half of that chain is not this package's. So: boat **yes**, Workers **yes**, Empress **no** — and the
"no" is measured as a Kismet/AI gap after the dialog system, not inside it.

## 2. Premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "~10 KB on the direct path" of a ~60 KB system | **The direct path is ~45 KB, not 10.** The hook FO traced is the last 10 KB. Before it: the one-shot actor has to hand its voice data to the spawners (`ADisDialogOneShot::NotifyOneShotSpeakers`, the `IDisConvOneShotOwnerInterface`), the spawner has to hand it to the pawn (`PostSpawned_Dialog`, not in agent EP's `OnSpawned`), the pawn's component has to instance and bind the tree (`InstanceAndBindDialogTree`, `InitAndBind`, `BuildFallbackChain`, the per-conversation in-game data), the Kismet action's hook has to go through the game info's conversation manager queue (`UDisConvGlobalMan`, which did not exist), the hook table that every lookup indexes has to exist (`InitHookInfo`, 18,920 bytes of straight-line code in retail, 126 rows), and the conversation has to acquire its speakers (`AcquireSpeakers` / `BindSpeaker`) before the soirée node is even reached. 135 functions ported, 13 partial, in 32 source units (§5). |
| "`UDisSeqAct_DialogInputs::Activated` ... already fires on our route and dies on a comment-only skeleton" | It fires, and its Target is **NULL**: `SeqVar_Object_177 = NULL` (FO's own trace said so; FR1/FR2 measured it on the ported body too, `target <null> is not a speaker`). The variable is written by `SOIREE_Boat.DisSeqEvent_Spawned_10`'s `Instigator` link, and **this tree's spawner never fired the Spawned Kismet event** — agent EP's `OnSpawned` names it as "agent AT's package". Retail fires it in `OnSpawned` on both paths (2012 `0x6a1010`, `DisFireSingleKismetEvent( this, UDisSeqEvent_Spawned, this, _pPawn, 0, 0 )`). Without it the dialog system was complete and had nobody to talk to. Found with a four-line referencer scan under `-frdiag`, FR2.log. |
| "the first link already fires" — the Kismet half of the chain is alive | The Kismet half fires **without** the remote event FO named: `SeqEvent_RemoteEvent_38` reads `activations 0` while `DisSeqAct_DialogInputs_10` reads 1 in FO3, FR1..FR4 alike; the op is reached some other way (not traced; it does not matter to the port). |
| FO §1: `IDisConvSpeakerInterface` with `FireDialogHook` at "vtable slot 8" and the rest implied | **This tree's `IDisConvSpeakerInterface` was method-less** (agent EC's note in `CppText/ADishonoredNPCPawn.h`): 30 slots had to be declared in retail order before anything could dispatch through a speaker. Same for `IDisConvOneShotOwnerInterface` (5 slots). |
| FO §1 / PHASE17: "`UDisConversationComponent::FireDialogHook` 0x8a6bd0* (908) ... `FireDialogHookNow` 0x8a5670* (630)" from the 2012 shape | **2013 moved the refire-time stamping** (own and global `m_fLastFireTime`) out of `FireDialogHook` into `FireDialogHookNow`, which is why the 2013 bodies are 908 / 630 against 2012's 804 / 408. Ported in the 2013 shape (read at `0x8a6bd0` / `0x8a5670`). |
| `UDisConv_Soiree` per the 2012 decompiles (PlayNodeMatinee 245 B, TickNodeMatinee 255 B) | 2013's are 109 / 124 B and test `m_bMatineeHasBeenRejected` (bit 23 of the flag word) where 2012 tested bit 22, ending the conversation with `DisConvoEndReason_SoireeRejection` (12; 2012's enum said 11). Read off the 2013 vtable (`??_7UDisConv_Soiree@@6B@` at `0x119ed30`, slots 87-90). The 2013 vtable also has one more slot than 2012's. |
| `dishonoredgameinfo.cpp`: "`UDisConvGlobalMan::InitGlobalConvMan( m_pConvGlobalMan )` 2013 rva 0x8b2e80" (agent CG) | `0x8b2e80` is a 26-byte `TArray` helper. The 2013 `InitGlobalConvMan` is `0x89caa0` (467 B), the second call of `InitGlobalManagers` (`0x5e9bd0`). |
| The 2012 decompiles' bit tests on `UDisConversation`'s flag word (`0x100` "player must be near", `0x200`, `0x400`) | 2013 inserted `m_bAlwaysInterruptForCombat` and `m_bCanStartDuringEnemyAwareness` at bits 8-9: `m_bPlayerMustBeNear` is `0x400`, `m_bFireConvOnlyOnce` `0x800`, `m_bNotWhenPlayerIsInCombat` `0x1000` (`retail_sdk_layout.json` masks, confirmed on 2013's `UpdateOutputLinks` `0x899e00`). Every 2012 mask above bit 7 was translated by name. |
| the hook table is 122 entries (2012: `g_DialogHookInfo.ArrayNum != 122`) | 2013's is **126** (`!= 126` in every 2013 reader; `DDH_INVALID = 126`), the four additions being the DLC06/07 hooks 122..125. The cooked trees carry 122-, 136-, 141- and 142-slot `m_DialogHooks` arrays and retail's `InitNode` grows them to the table size. |
| FO §9: "`UDisDialogTree`'s 21 KB ... `m_RunningInstances`, `m_NodeLookupMapByID` — the part this package did not read, and it is the largest" | 18.5 of those 21 KB are `InitHookInfo`, the hook table, which is data; the binding (`m_RunningInstances`) is `UDisDialogTree_InGameBind`'s 4.9 KB, and `m_NodeLookupMapByID` is editor-only. |
| the direct path ends at `StartFromDialog` once the hook reaches the soiree node | **The tree the hook walks had no in-game data to walk.** Every `UDisConv_Node::m_iConvNodeIndex` loaded as -1 (FR3/FR4: `DisConv_Soiree_1 has no in-game data`), because the index is not serialized: retail `UDisDialogTree::PostLoad` (2013 `0x88b1f0`, 28 B) calls `InitNode( NULL, 0, 0 )` on the tree, which numbers every node and fills `m_InGameDataClasses` before any bind. The skeleton's `PostLoad` did nothing. Ported. |
| `UDisConversationComponent::InFallbackChain` / `UDisDialogTree::InFallbackChain` ("does this pawn support that tree?") against the bound one-shot tree | FR5: `DishonoredNPCPawn_27 does not support tree ...DisDialogTree_OneShot_7`. The 2012 body (`0x8df160`) tests the **one-shot voice data's** tree (`m_pOneShot_DialogData->m_pDialogTree->InFallbackChain( _pTree )`), not a tree the component already bound; the first port demanded a bound instance and rejected every speaker whose tree is only reachable through the one-shot. Corrected to the 2012 shape. |
| FR7 (my own run, not the brief): `-inputcmd=buildgentFRr7_cmd.txt` | A relative `--cmd-file` is resolved against the game's working directory (`Binaries\Win32`), never found, and the run never quits; it was stopped with `taskkill` at 597 s with no capture. Every other run passed the absolute path. Noted so nobody repeats it. |

## 3. What was ported — the path, in call order

```
ADisDialogOneShot::PostBeginPlay -> NotifyOneShotSpeakers              each FDisOneOffSpeaker's actor (a spawner, PostLoad moved
    -> IDisConvOneShotOwnerInterface::AddDialogOneShot                  m_pSpawner to m_pAttachedActor) gets the voice data
ADishonoredSpawner::OnSpawned -> IDisConvSpeakerInterface::PostSpawned_Dialog( generics, m_DialogOneShots )
    -> UDisConversationComponent::PostSpawned -> InstanceAndBindDialogTree -> UDisDialogTree_InGameBind::InitAndBind
       -> BuildFallbackChain (one UDisConversation_InGameData per conversation, one UDisConv_Node_InGameData per node
          whose class asks for one - UDisConv_Soiree_InGameData for a soiree node), BuildSingletonData_Tree
    -> BuildDialogHookSupportedCache (UDisDialogTree::SupportsDialogHook over the one-shot, additional and generic trees)
    -> DisFireKismetEvent( spawner, UDisSeqEvent_Spawned, pawn )        the event writes the pawn into SeqVar_Object_177
ADishonoredGameInfo::InitGlobalManagers -> UDisConvGlobalMan::InitGlobalConvMan   (fire-group caps and hook overrides from UDisTweaks_ConvSettings)
UDisSeqAct_DialogInputs::Activated -> speaker->GetHookParameters()->m_KismetDialogHook = 'Intro'; speaker->FireDialogHook( DDH_KISMET_ACTIVATED )
    -> UDisConversationComponent::FireDialogHook: SupportsDialogHook, EvaluateDialogHook over the bound trees (the hook node's class
       must match the table's, UDisConv_Hook_KismetActivated::EvaluateForFiring), the refire delays, then
       UDisConvGlobalMan::QueueFireDialogHook (fire group 12, loud / quiet by distance)
ADishonoredGameInfo::Tick -> UDisConvGlobalMan::TickGlobalConvMan -> ManageGlobalDialogHooks: CalcPendingWeights, the per-group and
    global caps, UDisConversationComponent::FireDialogHookNow
    -> UDisDialogTree_InGameBind::FireDialogHook -> FireDialogHook_Helper -> BindAllSpeakers; PlayFromNode( hook node )
       -> UDisConv_Hook_KismetActivated::PlayNode_Derived picks the output named 'Intro'
       -> UDisConv_ConversationRef -> the conversation's in-game data AcquireSpeakers (BindSpeaker per slot, IsSpeakerAvailableForDialog,
          speaker->StartDialog) -> FDisDialogRunningInstance::StartDialog
       -> UDisConversation (output 0 'Start') -> UDisConv_Soiree::PlayNode_Derived -> PlayNodeMatinee -> FindMatinee (TObjectIterator
          on m_MatineeGUID) -> USeqAct_Interp::StartFromDialog -> ForceActivateInput( 0 )
UDisConversationComponent::Tick -> UDisDialogTree_InGameBind::TickDialogTree -> UDisConv_Soiree::TickNodeMatinee until the matinee
    stops playing with Position > 0 -> output 0 -> the next node (UDisConv_KismetActivateRemoteEvent back into Kismet) or StopInstance
```

Every 2013 address resolved by hand in `retail2013_named.i64` (a private copy, `build/agentFR/idb/FR_retail2013.i64`) and
every 2012 one in `shipping2012_v1.i64` (`FR_shipping2012.i64`), through the headless `decompile_funcs.py`; the decompiles
are in `build/agentFR/decomp2012/` (425 + 4 later batches) and `decomp2013/`. Functions the 2013 database does not name were
pinned by vtable slot (`build/agentFR/tools/vtable_dump.py`) or by caller (`callees.py`, `xrefs.py`), and are marked `*` in
the source banners and in `agentFR_status.csv`. The hook table came out of the 2013 `InitHookInfo` (`0x88d820`) row by row
(`tools/extract_hookinfo3.py` -> `hookinfo_2013.txt` -> `gen_hookinfo_table.py` -> `Src/disdialogtree_hookinfo.inl`), and was
diffed against the 2012 table: identical for rows 1..121, four DLC rows added.

## 4. What was deliberately left — the inventory

`agentFR_status.csv` is the function-level record: **135 ported, 13 partial, 148 inventoried, 68 editor-only** of the
2013 class set (513 functions, 53.8 KB with boilerplate; 249 / 43.8 KB without). The partial ones, each named at its site
with the package it belongs to:

| left out of a ported body | belongs to |
|---|---|
| `UDisConversationComponent::Tick`'s look-at drive (`FDisLookAtRequest::TickRequest/RequestActorTarget/RequestProxyTarget`), `BeginPlay`'s `FDisLookAtRequest::Initialize`, `Detach`'s `Finalize` | FT (`FArkComponentLookat`) |
| `StopDialogBlurb`'s Wwise unload / Ak stop / FaceFX stop / subtitle kill; `Term_GlobalConv`'s HUD subtitle resets; `TickGlobalConvMan`'s `DuckNonDominantAudio` and `FDisSubtitlesInfo::Tick`; `GetShortestAudibleDistForSpeaker`'s audio-cell propagation (plain distance to the player here) | the voice / subtitle packages (audio is Phase 10 polish) |
| `UDisDialogTree_InGameBind::ChangeAttentionForSpeaker`'s `FAIStimStruct_DialogAttentionChange` on the brain; `ADishonoredNPCPawn::StartDialog/EndDialog`'s soirée LOD and `FAIStimStruct_BlockingDialogRequest/End`; `IsAvailableForDialog`'s behaviour query (`UDishonoredAIBehavior::CallGetDialogAvailability`, `UStateNPCMasterAction::GetDialogAvailability_Action`); `UDisConv_Speaker::BindSpeaker`'s awareness filter and vision-component LOS | FS (the AI contract that hands a pawn to the scene) / the AI perception package |
| `ADishonoredPlayerPawn::StartDialog/EndDialog/IsAvailableForDialog` through `UStatePlayerMasterInDialog` | FK's FSM package (agentFO.md §9.4); the state is excluded, so a blocking-player conversation rejects the player (logged) |
| `UDisConv_Soiree::InterruptTickingNode`'s `StopFromNPC` (`Stop()` here) and `OnSkipRequest`, `IsTickingNodeInterruptible`'s `IsSkippable` | FS (`UnInterpolation.cpp`) |
| `ADishonoredSpawner::CopyEventsToSpawned` (2012 `0x697450`, 681 B) | the Kismet package that owns the spawner's events |

Not ported at all, with 2013 rvas where named: the blurb / voice surface (`UDisConv_Blurb` 14 functions, `UDisConversationComponent::PlayDialogBlurb 0x89fbf0, StreamDialogBlurb, StopAkEvents 0x88bae0, UnprepareAkEvent 0x88b0b0, IsAnyAkEventPlaying 0x896740, IsDonePlayingBlurb 0x89a160, FindTextBlurb 0x8a2410`, `UDisDialogVoiceData`'s 15 lookup / bank functions), the player-choice UI (`UDisConv_PlayerChoice`), the condition nodes' `PlayNode_Derived` bodies (15, listed in `disconv_conditions.cpp`; an unported condition plays its output 0), the branch nodes (`UDisConv_RandomBranch`, `SequentialBranch`, `FactionBranch`, `SpawnerBranch`, `SpeakerHasTweaks`, `SpeakerInStoryGroup`, `SpeakerSupportsDialogTree 0x89c430`), the hook nodes other than `KismetActivated` (`Distance`, `Distraction`, `Investigate`, `Notice`, `PlayerLoiter`, `PlayerLookAt`, `SuspicionLevel`, `WitnessedMagic`, `DeathMode`, `HeartTargeted`, `PlayAudioLog`), `UDisConversationComponent::SerializeForGameSave 0x88b9c0`, the `UDisConvGlobalMan` save / terminate / branch-info functions, `UDisSeqEvent_DialogOutputs`' five editor functions, `ADisPlayerAudioLogDummyActor`'s audio-log playback (`Tick 0x6a8ee0` and the Ak callback), `ADisDialogInanimateDummy`, `ADisAudioLogPlayer`, `ADisSpeakerGroup_PA`, and the whole `UDisDialogTree` editor surface. `agentFR_status.csv` names each with its address and size.

## 5. Files

See `build/agentFR_sync.py`'s header: 78 files, every function named per file. 24 units came off `Sources.cmake`'s exclude
list, 3 units are new (`frdialogdiag.cpp`, `disconvsavedata.cpp`, `dishonoredspawner_conversation.cpp`), 28 `CppText`
headers are new and 10 existing ones gained declarations, 4 generated headers gained only the `#include "CppText/..."` lines
the generator emits when the file exists (no reflected member moved, no regeneration needed), and `Engine` gained
`USeqAct_Interp::StartFromDialog` / `ClearDialogConvNode` beside `SetConversationNode` in `UnInterpolation.cpp` (FS's file;
two functions appended, nothing else touched). The two SDK-dump gaps the bodies read (`FDisDialogRunningObjs::m_Conversations`,
`FDisConvSaveData_DialogTree::m_ConvData`) are typed through accessors in struct `CppText`, not by changing the layout.

## 6. The instrument: `-frdiag`

`frdialogdiag.cpp`. `frdiag inputs` (the Kismet action: targets, their speaker interface, the initiator, and for a NULL
target variable every op that links it and whether it writes it), `frdiag hook` (the component's decisions), `frdiag queue`
(the manager), `frdiag bind` (binding, acquisition, every node played with its output), `frdiag soiree` (the node, its
owner's blocking flags, the matinee found and started, its end), `frdiag oneshot` / `frdiag spawned` (the hand-off), and
`frdiag census` on `-fkdiag`'s second pass. Nothing under the switch changes behaviour.

## 7. The runs

All runs: `game_input.py launch` (`-inputcmd`, `-unattended`, `-windowed`), `-finewgame=50 -fosoiree -fkdiag -frdiag -diskismet`,
exe staged by `stage_retail.py` as `DishonoredGame_FR.exe`; the control exe is the shared `build/head_rel` one. Captures are
in-process (`shot <tag>` -> `Screenshots/Win32Console/apshottime_<tag>00000.bmp`); sizes read off the file. FR1-FR3 and FRC1
were taken before the `-windowed` rule and are 1920x1080 (6,220,854 B); FR4 onwards are 1008x567 (1,714,662 B), every one
with real content (viewed).

| run | tree | switches beyond the common set | what it measured | capture |
|---|---|---|---|---|
| FRC1 | `head_rel` (`80cc219`), control | `-fkdiagat=160`, shot 164 s, quit 167 s | `SOIREE_Boat.SeqAct_Interp_1` and `SOIREE_Workers.SeqAct_Interp_3` `activations 0`; `DisSeqAct_DialogInputs_10` activates with a `<null>` target; no `frdiag` (switch unknown to that exe). | `frc100000.bmp`, 6,220,854 B, boat at the starting pier |
| FR1 | first port, base `d7d083a` | `-fkdiagat=160`, shot 164 s | the hook reaches `UDisSeqAct_DialogInputs::Activated`; `target <null> is not a speaker`. | `fr100000.bmp`, 6,220,854 B |
| FR2 | + referencer scan in the diag | `-fkdiagat=110`, shot 114 s | `SeqVar_Object_177` is written only by `SOIREE_Boat.DisSeqEvent_Spawned_10` (Instigator) and that event never fired: the spawner skipped it. | `fr200000.bmp`, 6,220,854 B |
| FR3 | + `FireSpawnedKismetEvent` | `-fkdiagat=160` (never reached), shot 164 s | target is `DishonoredNPCPawn_26`; hook 10 queued and fired; `DisConv_Soiree_1 has no in-game data` (`m_iConvNodeIndex -1`). | `fr300000.bmp`, 6,220,854 B |
| FR4 | + `UDisDialogTree::PostLoad -> InitNode` | `-fkdiagat=130`, shot 167 s | in-game data present; conversation rejected: `DishonoredNPCPawn_27 does not support tree`. | `fr400000.bmp`, 1,714,662 B |
| FR5 | + `InFallbackChain` on the one-shot's tree; main `e60c5c6` merged | `-fkdiagat=130`, shot 164 s | speakers acquired, node reached, `StartFromDialog` called — matinee runtime before FS's merge. | `fr500000.bmp`, 1,714,662 B |
| FR6 | + main `4390017`, FS `d24f1bf` merged | `-fkdiagat=130`, shot 149 s, quit 152 s | **boat `0 -> 1`** (start 95.17 s, end 195.43 s), Workers `0 -> 1`; the `MAT_*` warnings of §1. | `fr600000.bmp`, 1,714,662 B, inside the lock |
| FR7 | same exe | `-fkdiagat=280`; relative `--cmd-file`, never read, stopped by `taskkill` at 597 s | second pass at 330.66 s: boat `activations 1 pos 94.23`, Workers `1 / 35.88`, Empress `0`; `frdiag census`: 18 one-shot actors, 28 components, 35 bound trees, 0 running. | none (no input script) |
| FR8 | same exe | `-fkdiagat=200`, `-foupstream=SOIREE_Empress_Regent.DisSeqAct_DialogInputs -foupstreamdepth=4`, shot 172 s, quit 202 s | boat 62.92 -> 157.17 s, Workers 123.37 -> 159.28 s; the Empress Kismet/AI feeders of §1. 200 warnings, 0 `MAT_BeginAIGroup`, 0 `Critical`. | `fr800000.bmp`, 1,714,662 B, inside the lock |

The runs' logs are `DishonoredGame/Logs/FRC1.log`, `FR1.log` ... `FR8.log`; the input scripts are `build/agentFR/fr<N>_cmd.txt`.
Thumbnails of the captures: `build/agentFR/shot_fr600000.png`, `shot_fr800000.png`, `shot_frc100000.png`.

Measurements not taken on retail (the real `Dishonored.exe` was off limits for this wave): the retail timing of the same
route (when `Intro` fires after load, how long the boat takes), and whether retail's unattended route reaches the Empress
without the player walking. FB's captures in `build/agentFB/retail/` are the only retail reference used.

## 8. Acceptance

Gate: `python resources/tools/run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentFR_reg --newgame-at 50`
from the worktree's own copy, built in-harness (`build-release.cmd` -> `build_queued.py`, all cores, no queue lock, as the
coordinator allowed overnight): **46 ok, 1 failed, 0 skipped, 1517 s** (`build/agentFR_reg/regression/summary.txt`,
`build/agentFR/fr_gate.log`). The three builds: `build_DishonoredGame_errors 0`, `build_CoreSmoke_errors 0`,
`build_LayoutProbe_errors 0`; **0 `C4263`, 0 `C4264`** in `build/agentFR_reg/regression/build_DishonoredGame.log` (305
`warning C` lines, the tree's usual set). coresmoke 99/0, layout 2314 / 0 mismatching, nullrhi, d3d9, inputtest, newgame
(`newgame_pawns 29`, `newgame_criticals 0`) and options all green.

The one failure is honest and is caused by this package on purpose:

```
FAIL unported_natives  2 (wanted <= 0)   d3d9 stage, 7.09 s into L_Tower_P
  DishonoredGame native not ported: ADishonoredPawn::execOnSetDisposition
  DishonoredGame native not ported: ADishonoredNPCController::execOnAIGoToActor
```

Both fire because `ADishonoredSpawner::OnSpawned` now fires `DisSeqEvent_Spawned` as retail does (§2). The spawner's Kismet
in `L_Tower_P` answers it with `DisSeqAct_SetDisposition` and `DisSeqAct_AIGoToActor` on `DishonoredNPCPawn_24` (the log's
`Kismet: === BUG === Name of the buggy pawn : DishonoredNPCPawn_24` is the level's own script print when the go-to action
returns without moving), and those two script natives are still `DISHONORED_NATIVE_STUB`s. The metric's 0 was measured
on a tree whose spawners never fired the event, i.e. on a walking path that never reached this Kismet; the bound is doing
its job. They are not conversation code and were not ported here; what they need, counted in this tree (0 bodies each,
skeleton comments only):

| native | retail | needs |
|---|---|---|
| `ADishonoredNPCController::OnAIGoToActor` | 2013 `0x763890`, 490 B (2012 `0x7bd370`, decompile `resources/reference/decomp/agentAJ/s12`) | `UDisSeqAct_AIGoToActor::GetDestinationActor`, `::SetGoToStatus` (2012 `0x7d8da0`), `FAIStimStruct_GoToRequest` ctor, `UDishonoredAIBrain::m_Home` write, the `HandleAIStim` / `HandleAIStim_Internal` templates (present), `UDisBehaviorTriggerAlarm::CanBeDormant` (present) — FS/FK's AI package |
| `ADishonoredPawn::OnSetDisposition` | 15 B -> `IDisRelationshipInterface::OnSetDispositionHelper` 2013 `0x70f700`, 858 B (2012 `0x776910`) | the whole relationship-override API: `IDisRelationshipInterface::FromUObjectOrController` (2012 `0x7768b0`), `ClearRelationToIndividualOverride` (`0x76f0f0`), `FDisRelationshipOverrideInfo::ClearAllOverrides / ClearIndividualOverride / OverrideIndividualRelationship`, `UDisGlobalFactionManager::ClearIndividualCache` (two overloads), four `UDisTweaks_Faction` functions unnamed in 2013 (`sub_C63BB0`, `sub_C5E610`, `sub_C60C40`, `sub_C638E0`), and the interface's vtable slots 2 and 5 — agent AJ's relationship package (AJ listed both `OnSetDisposition`s as its own next batch) |

The exec thunks themselves are two lines each (`P_GET_OBJECT`, `P_FINISH`, call the virtual); the virtuals are what is
missing. Until they land, every merge of this package will show `unported_natives 2` on the d3d9 stage; the alternative of
not firing the Spawned event would put the dialog target back to `NULL` and the boat back to `activations 0`.

Rules kept: no real input (every launch through `game_input.py launch` + `-inputcmd` scripts, `-windowed` on all of them,
captures in-process); every build through `build_queued.py` (`fr_build.cmd`, `build-release.cmd`); no clean-delete, no
HEAD control build; no retail `Dishonored.exe` launch; no FModel; the sync list checked against `git diff --name-only
d24f1bf` + `git ls-files --others --exclude-standard source resources` (79 = 79, §5; the `agentFS*` / `agentFU*` docs the
merge patches dropped into the worktree are theirs and are excluded); reflected members unchanged, so no header
regeneration — the `#include "CppText/..."` lines were inserted by `build/agentFR/tools/insert_cpptext_includes.py` exactly
where the generator would put them.
