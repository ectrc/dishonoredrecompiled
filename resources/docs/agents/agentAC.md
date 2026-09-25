# Agent AC report: DishonoredGame natives on the startup path (2026-09-25)

(Saved by the coordinator from the agent's hand-back message; the harness refused the subagent's own write.
The generated tables it points to are in `build/agentAC_work/`: `table_manual.md`, `table_trivial.md`
(all 247 trivial rows) and `table_left.md`.)

## Summary
- **289 of 974 natives are ported.** 42 are written by hand; the other 247 have a trivial body in retail. The code they need is also ported: 55 C++ members and accessors, among them `UDishonoredEngine::Init` and `PlayLoadMapMovie`, and viewport client `Draw`, `Exec` and `PostRender_Native`.
  - "Trivial" means the exec calls a 2013 vtable slot that is empty, returns FALSE or returns TRUE, and no retail subclass overrides that slot.
  - Most of the 247 are the 179 `UDishonoredCheatManager` natives, which are compiled out of the shipping exe.
  - Only about 40 of the 289 are substantive startup-class ports. The ≥80 target is met on the total, not on startup-path work.
- **685 stubs are left.**
- **No DishonoredGame native is hit before `LoadMap` today.** Agent X's run passes `Initializing Engine...` and reaches `LoadMap: DishonoredGameFull_P` without a single DishonoredGame warning; the only stubs it hits are 3 OnlineSubsystemSteamworks ones (ported by X).
- **Possible slowdown:** X's log is silent for 17 s after `Initializing Engine...`. The `Init` port reloads config and localisation for every config/localized object, like retail does, so it may be the cause; not confirmed.

## Build, checks and results (all from `build\agentAC`)
- Configured from the snapshot worktree `build\agentAC_wt` (HEAD da24f65 plus AC files), all four module options on; own `external/` copies (`FETCHCONTENT_FULLY_DISCONNECTED=ON`).
- `DishonoredGameModule` and the exe build with 0 errors, with `DISHONORED_SDK_LAYOUT_CHECKS=ON` and `DISHONORED_LAYOUT_CHECKS=ON`.
- CoreSmoke: 99 passed, 0 failed.
- Isolated smoke exits at the Edge crash of the time (`7: unknown or unsupported translation compression`), 0 `native not ported` lines.

## Generator changes (`resources/tools/symbols/gen_classes_header.py`, 3 hunks)
1. **cpptext hook.** If `DishonoredGame/Inc/CppText/<Class>.h` exists, the generated class body ends with `#include "CppText/<Class>.h"`. Ports put their C++ declarations there, so they survive regeneration. Other agents already use it (Y for `UArkBinkOverlayManager`, Z for `UDisConv_Blurb` / `UDisConv_PlayerChoice`).
2. **Sources.cmake rule.** Only skeleton units that are still comment-only are excluded (`is_comment_only`), so a unit that gets code is compiled without hand edits.
3. The header comment of `NativeStubs.cpp` describes both.

**Skip-list location:** the generator reads `<Module>NativeStubs.ported.txt` from the module root: `DishonoredGame/DishonoredGameNativeStubs.ported.txt`.

## Workflow the merge needs to know
- `build/agentAC_work/cycle.py [--main]`: merges AC's lines into the shared skip list, regenerates the snapshot; with `--main` regenerates the shared tree in place. It never rewrites `DishonoredGameLayouts.h`.
- Trivial natives come from `gen_trivial.py`, using `triage.csv` and `vt13_all.json` (214 retail vtables found through `??_7` symbols or the constructor store, `ctor_rva_2013`).
- 3 candidates skipped: `ADishonoredPawn::ChooseAndTriggerDeathEvent_Native` (overridden in NPC pawns), `ADishonoredNPCPawn::SetDesiredRotation` and `DisplayDebug_Native` (no SDK signature).
- `resources/docs/function_status.csv` has 343 AC rows (`rva,written|stubbed,note`); the rva column is the 2012 rva when one matches, the 2013 rva is always in the note.

## Hand-written natives (class | native | 2013 rva / 2012 rva)
- ADishonoredGameInfo: execCanStartMatch 0x5ed020 / 0x633a80; execSpawnPlayer 0x5ed060 / 0x633ac0; execPreventDeath_Native 0x5ed140 / 0x633ba0; execGetChangelist 0xcf20 / 0xcf90; execGetDishonoredEngineVersion 0xcee0
- ADishonoredHUD: execDrawHUD_Native 0x5f6340 / 0x638b40; execPlayerDisplayDebug_Native 0x5f8400 / 0x63e120; execDebugClear 0x5f26f0 / 0x63e400; execRenderNonHUD_Native 0x5f8390 / 0x63e0b0
- ADishonoredPawn: execOnDLC05SetStoryGroup 0x5ecc30
- ADishonoredPlayerController: execGetProfileSettings 0x5eeca0 / 0x635290; execIsMoveInputIgnored 0x1d3a70; execIsLookInputIgnored 0x1d3ab0; execReceivedPlayer_Native 0x5edbf0; execOnControllerChanged_Native 0x5eed20
- UArkProfileSettings: execSetToDefaults 0x1cd3a0
- UDisBehaviorFollow: execNoMoreTargetCallback_Follow 0x5f5a50 / 0x63bf40
- UDisDLC06BehaviorSummonedAssassinIdle: execRequestStateExitCallback_GenericAction 0x5e47a0
- UDisGFxMoviePlayerMainMenu: execOnCampaignTabSelected 0x5f0f80; execOnDLCTabSelected 0x5f7e10; execOnMissionsMenuClosed 0x5f1030; execReq_PSStoreEnabled 0x5f7f50
- UDishonoredCheatManager: execStartVisSettingsMode 0x5f3bb0 / 0x639de0; execStopVisSettingsMode 0x5f3bf0 / 0x639e20; execForceCorpseDropTypeOff 0x5f3e70 / 0x63a0a0
- UDishonoredEngine: all 15 (PublishRichPresence, UpdateRichPresenceChaos/Chapter, OpenContentUnavailableMenu, OpenControllerConnectionMenu, OnControllerDisconnected, OnControllerChanged, OpenPauseMenu, PlayLoadMapMovie, Push/PopIgnoreAutosave, Push/PopDisableSave, Dis_Load, Dis_Save); rvas in `table_manual.md`
- UDishonoredPlayerInput: execDis_Jump_ButtonDown 0x5efd00 / 0x1f0150
- UDishonoredViewportClient: execPostRender_Native 0x5ed500 / 0x633de0

Trivial natives per class: UDishonoredCheatManager 179, UUIScreenObject 17, UDisDLC07CheatManager 6, UDisDLC06CheatManager 5, 4 each for UDisGFxMoviePlayerMainMenu / ADishonoredPlayerPawn / ADishonoredHUD, 28 more across AI behaviours, GFx players, NPC/debug controllers, note, rat spawner, autotest, emitter.

## What the retail code does that we do not
- **`UDishonoredEngine::Init`** (2013 0x614e20, 1006 bytes; 2012 0x65d280). Ported: `UGameEngine::Init`, controller ids from `m_bInitControllerToZero`, DLC root dir, `RefreshDLCFromNative`, the config/localisation reload of every object, `m_dLastAutoSaveTime`, DLC06→07 chaos-bit clear, `m_TransitionSaveType` reset. Left out: `DisSaveLoad::FGameState` (`m_pGameState` NULL), the GetPowerClasses cache warm-up, `UDisBinkOverlayManager::Initialize`, the three UInterpTrack priority setters, and — on purpose — 2013 rva 0x601210, the 2026 patch's **telemetry** (libcurl `game_start` POST, 120 s heartbeat, `SaveData\Puid.txt`). 2012's `FDisAsyncSaveGameLister` is no longer queued in 2013.
- `PlayLoadMapMovie`: 2013 chapters 16..25 also hide the chaos level; Bink calls inert. `Dis_Save` / `Dis_Load`: warn once and drop (save system not ported). `OpenPauseMenu` etc.: GFx part inert. Not ported: `Tick`, `PreExit`, `Pre/PostCommitMapChange`, `LoadMap`, `StopMovie`, `NotifyActorDestroyed`.
- **`UDishonoredViewportClient`:** `Exec` (SafeFrame, KUWA), `Draw` (GFx PreRender inert), `PostRender_Native`, listener-override functions. `UDisLocalPlayer` has nothing to port in 2013.
- **Player controller:** `s_pInstance`, `PostBeginPlay` (sets `CheatClass = NULL`: retail never creates a cheat manager), `BeginDestroy`, `IsInputEnabled`, `IsMove/LookInputIgnored`, `ReceivedPlayer_Native`, `GetProfileSettings`. Applying tweaks and initialising the use-interaction FSM are bring-up.
- **Game info:** `SpawnPlayer` loads the default/campaign pawn tweak and spawns through `UDisTweaksBase::SpawnActor`; `SpawnActor_Derived` cannot apply `FSpawnActor_TweakObj::DoInit` (retail calls `SetTweaks` before `PostBeginPlay`).
- **HUD:** `DrawHUD_Native` etc.; `UDisPowerMenu::Render` has only its hidden-menu early-out.
- Golden lines `DevDlc: Looking for DLC...` and `SaveGameList done` exist in neither shipped exe (ArkProfile-only logging).
- **Audio system:** retail creates it in `UWorld::Init` (2013 0x3945f0) from `engine-ini:DishonoredMods.AudioSystemClass` (+ `MapInfoClass`); `UDishonoredAudioSystem::Init` (0x7a3120) is not ported.

## Remaining stubs in the classes that exist at map start
- Player controller 42 (`HandleWalking`, `HandleHeldButtons`, `ProcessViewRotation`, `CalcPlayerSwimAccelRate`, cinematic mode, teleport, input/wheel/item handlers); player input 4; player pawn 37, pawn 17, NPC pawn 19, NPC controller 17; `ADishonoredGameInfo::GameEnding`, 5 music-manager tick callbacks, 3 KActor natives, `ADishonoredPlayerCamera::ApplyDebugCam_Native`.
- Blocked by DishonoredGame infrastructure, not natives: the tweak interface (`IDis(Engine)TweaksInterface` vtable), native FSM transitions (`RequestStateChange`, `DemandStateChange`, `DoStateChange`, `TickStateMachine`, `InitFSM`), `UDishonoredInventory`, the DisSaveLoad save system, GFx movie players and Bink.

## Follow-ups outside AC's files
1. **UEngine is missing retail virtuals**: `PlayLoadMapMovie(Map, Movie)` (0x2097d0), `OpenPauseMenu`, `OnControllerDisconnected`, `OpenControllerConnectionMenu`, `OpenContentUnavailableMenu`, `IsLoadingGame`; `StopMovie` virtual, `IsDebugMenuVisible` (+392), `RenderDebugMenu` (+388).
2. **APlayerController:** `IsMove/LookInputIgnored` and `ProcessViewRotation` are C++ natives in retail (0x1d3a70 / 0x1d3ab0 / 0x1d3840); ours are script.
3. Missing Engine statics/calls: `UInterpTrackFaceTo/Locomotion/LookAt` priority setters (0x4fc8a0 / 0x4fcc30 / 0x4fcc90), `ArkSettings::ApplyCurrentSettings` (0x53b790), `UGameViewportClient::ApplyListenerLocationModifier`.
4. `UWorld::Init` should create the audio system and MapInfo from `[DishonoredMods]` and fill `m_pWorldInfoCheckStreamingPersistent`; UAudioSystem needs its virtual interface.
5. `UWorld::SpawnActor` needs retail's trailing init-callback parameter for tweak spawns.
6. Next DishonoredGame package: tweak interface, FSM transitions, inventory, save system.

## Files (no commits)
- Generator: `resources/tools/symbols/gen_classes_header.py`. Skip list: `DishonoredGame/DishonoredGameNativeStubs.ported.txt` (new).
- Declarations: 11 files in `DishonoredGame/Inc/CppText/`; accessor declarations in `Inc/dishonoredutilities.h`.
- Hand-written units: `dishonoredengine`, `dishonoredviewportclient`, `dishonoredplayercontroller`, `dishonoredplayerinput`, `dishonoredgameinfo`, `distweaksbase`, `dishonorednativestatemachine`, `dishonoredutilities_accessors`, `dishonoredhud`, `dispowermenu`, `dishonoredplayerpawn`, `dishonoredpawn`, `disbehaviorfollow`, `dishonoredcheatmanager`, `disgfxmovieplayermainmenu`; new units `arkprofilesettings.cpp`, `disdlc06behaviorsummonedassassinidle.cpp`, `uiscreenobject.cpp`, `disdlc06cheatmanager.cpp`, `disdlc07cheatmanager.cpp`, `disdlc07playerpawn.cpp`; trivial-native blocks in ~20 more skeleton units.
- Regenerated: `DishonoredGameNativeStubs.cpp`, the class headers, `Sources.cmake`. Status rows: `resources/docs/function_status.csv`.
- Scratch: `build/agentAC_work/` (`cycle.py`, `gen_trivial.py`, `triage.csv`, `vt13_all.json`, native-table extracts). IDA copies `retail2013_agentAC.i64`, `shipping2012_agentAC.i64`; worktree `build/agentAC_wt`.
