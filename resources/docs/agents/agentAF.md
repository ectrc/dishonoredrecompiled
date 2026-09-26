# Agent AF — Milestone 5 driver: `L_Tower_P` streamed in, player spawned and possessed, input moves the pawn (2026-09-26)

Package AF of `resources/docs/PHASE6.md`. The retail 2013 exe is the target: every "2013 rva" is `retail2013_agentAF.i64`
(copy of `retail2013_named.i64`), "2012 rva" is `shipping2012_agentAF.i64`, readable version only. Decompiles:
`resources/reference/decomp/agentAF/{2012,2013}` (git-ignored, 100 functions).

**A power cut killed the first session before anything was applied.** Everything below was done in the second session, on top of
the wave-4 commits (AK `571bd0e`, AD `8af9425`, AI `03f5335`, AE `f8bad4f`, AG `ad3f9ae`, AH `f8dfe78`, AJ `6e9d1ad`).

## Result

| Step | State |
|---|---|
| 1. New Game path found, `-startmap` / `-newgame` | **done** — the menu's command is the tweak string `ce ChangeLvl_StartNewGame`; the retail route reaches the golden commit line |
| 2. Spawn + possess in a mission map | **done** — `DishonoredPlayerPawn` possessed by `DishonoredPlayerController` in `l_tower_p` at the PlayerStart |
| 3. Input chain | **done as a chain, not as a walk** — W produces a real movement acceleration (peak 2D 3303) and the mouse turns the view 13328 yaw units; the pawn cannot walk because the map has no floor (see blocker B4) |
| 4. `UDishonoredEngine` UEngine overrides | **done** for the movie virtuals (the recursion fix below); the rest were already agent AC's |
| 5. d3d9 `-dumpframes` shots | **not done** — the coordinator's constraint (FMallocDebug exhausts the 32-bit heap ~2 s into a d3d9 run) plus B1–B4; milestone 5 is judged on the null RHI |

### Accept command

The PHASE6 command, as specified, with the bring-up switches this tree needs:

```
python resources\tools\build_and_smoke.py --build-dir build\agentAF --no-build --exe-name DishonoredGame_AF.exe ^
  --log-name agentAF.log --ini-dir build\agentAF\config --rhi null --timeout 200 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --expect "DISHONORED(bringup): possessed" --expect "DISHONORED(bringup): inputtest moved" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -noscenerender -nolevelstream -forcelogflush"
```

**exit 0**, all three `--expect` lines present (`build/agentAF_smoke18.txt`). What that run actually proves:

```
[0006.52] DISHONORED(bringup): startmap: 'OPEN L_Tower_P' after the None commit
[0006.54] LoadMap: L_Tower_P?Name=Corvo?Team=255
[0006.77] DISHONORED(bringup): possessed DishonoredPlayerPawn (DishonoredPlayerPawn) in l_tower_p by
          DishonoredPlayerController at X=-3900.598 Y=36639.262 Z=-215.850
[0006.77] DISHONORED(bringup): player input bindings: 127 base + 36 pc + pad set 0 -> 172 bindings
[0036.62] DISHONORED(bringup): inputtest viewport interactions: 3 (Console, UIInteraction, PlayerManagerInteraction),
          the player's input is at -1
[0036.62] DISHONORED(bringup): inputtest controller state 'PlayerWalking', pawn state 'DishonoredPlayerPawn'
[0038.62] DISHONORED(bringup): inputtest W released: acceleration X=1533.514 Y=-2925.431 Z=0.000
[0038.62] DISHONORED(bringup): inputtest mouse-X handled 1, aMouseX 20.000
[0039.62] DISHONORED(bringup): inputtest moved 0.0 turned 13328 (yaw -11348 -> 788412, peak 2D accel 3303.0, physics 0)
```

So: the mission map loads, the retail pawn and controller classes are spawned and possessed, the retail binding set is built,
keyboard input reaches `UDishonoredPlayerInput` and comes out as a movement acceleration on the pawn, and the mouse turns the
view. **`moved` is 0.0 and the milestone is not complete**: the pawn spawns in a world with no collision, falls to the kill
plane and freezes at `PHYS_None`, so the acceleration never becomes movement (blocker B4).

The retail New-Game route (no `-startmapopen`) is the other half of the picture:

```
python resources\tools\build_and_smoke.py ... --timeout 300 --milestone "Committed map change via DishonoredEngine" ^
  --expect "Initial startup" --expect "DISHONORED(bringup): possessed" --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -inputtest -noscenerender -forcelogflush"
```

milestone **reached** (`build/agentAF_smoke6.txt`): `Committed map change via DishonoredEngine` at 7.33 s (golden :365 is 4.51 s),
then `STREAMMAP L_Tower_P` is issued, the three menu levels are removed and `l_tower_p` actors initialise
(`CameraActor_8`, `DisFog_4`, then `PlayerArms_Tatoo_inst ... Twk_Pawn_Corvo` — golden :390). The second commit never
arrives: B2.

## Which build the numbers come from

Snapshot `build/agentAF_wt` = HEAD `6e9d1ad` + **only agent AF's 10 files** (`python resources/tools/make_snapshot.py AF
--list build/agentAF_files.txt`), built into `build/agentAF` with `build/agentAF_wt_build.cmd`. 0 errors, 0 warnings that
matter. Two snapshot-only repairs are applied by `build/agentAF_snapfix.py` (both hand-overs, §Hand-overs B1/B3).

Earlier in the session, before AE/AI/AJ were committed, the snapshot also needed the reconciliation scripts
`build/agentAF_dedup_shims.py` (25 shim bodies + 19 `IMPLEMENT_CLASS` that agent AI had moved into Engine),
`build/agentAF_cpptext_hooks.py` (a `CppText/<Class>.h` hook and the generated duplicates it replaces) and
`build/agentAF_dedup_stubs.py` (6 stub bodies of natives agent AJ had ported). They are no longer needed now that the
DishonoredGame module has been regenerated in the shared tree, and they are kept only as tools for the next wave.

## What changed, per file (all with `DISHONORED(port|written|retail|bringup)` evidence in the source)

| File | Change | Evidence |
|---|---|---|
| `Engine/Src/UnGame.cpp` (+171/-56) | `PrepareMapChange` and `ConditionalCommitMapChange` ported; `UpdateWorldInfoCache` after `UWorld::Init`; the two reference commit events replaced by the C++ virtuals; `DishonoredTickStartMap` (`-startmap=`, `-startmapopen`, `-newgame`); `-noscenerender` / `-nolevelstream` in `RedrawViewports` | 2013 0x229d30, 0x232770, 0x22d570, 0x240b40, 0x38cac0 |
| `Engine/Src/UnLevTic.cpp` (+56/-5) | `APlayerController::PlayerTick` as a file-static helper and called from `Tick`; the paused-tick `eventPlayerInput` replaced by the C++ `PlayerInput` | 2013 0x247dd0, 0x255040 |
| `Engine/Src/UnIn.cpp` (+190/-9) | `UPlayerInput::PlayerInput`, `AdjustMouseSensitivity`, `GetFOVScale`, `SmoothMouse`, `ClearSmoothing`, `CatchDoubleClickInput`; the reference touch loop dropped from `UInput::Tick` | 2013 0x3f2730, 0x3fd250, 0x3d4010, 0x3ebcb0, 0x3ebc50, 0x3d3ea0, 0x3da5d0 |
| `Engine/Src/UnPlayer.cpp` (+197/-0) | `InputAxis` movie guard; unhandled key/axis routed to the local players' `UPlayerInput`; the `-inputtest` driver | 2013 0x2b9680; `script_classes_2013.json` GameViewportClient.InsertInteraction |
| `Engine/Inc/EngineUserInterfaceClasses.h` (+13/-0) | the six retail `UPlayerInput` virtuals (+340/+344/+348/+352 and the three helpers) | 2013 vtable of UPlayerInput |
| `DishonoredGame/Src/dishonoredengine.cpp` (+5/-3) | `PlayLoadMapMovie` chains to the **two-argument** `UEngine::PlayLoadMapMovie`, qualified | 2013 0x601b80 → 0x2097d0 |
| `DishonoredGame/Src/dishonoredplayercontroller.cpp` (+54/-0) | `Possess` (+ the milestone log + the bring-up `Restart`), `UnPossess`, `HandleWalking` | 2013 0x6a0ae0, 0x6a0af0, 0x6a0770 |
| `DishonoredGame/Src/dishonoredplayerinput.cpp` (+82/-1) | `PlayerInput` override, `AddBindingSet`, `BuildBindings` called from `OnInit` | 2013 0x6a12c0, 0x6b84f0, 0x6bd610, 0x6afe50 |
| `DishonoredGame/Inc/CppText/ADishonoredPlayerController.h` (+6/-0), `.../UDishonoredPlayerInput.h` (+5/-0) | the declarations for the above | retail vtable slots +956/+960/+1156 and +348 |

`DishonoredGameNativeStubs.ported.agentAF.txt` is **not** needed: every function above is a C++ virtual, so no native moved out
of the generated stub file and the DishonoredGame module needs no regeneration for agent AF.

## The New Game path (2013 rvas)

```
UDisGFxMoviePlayerMainMenu::execOnNewGameConfirm 0x5f7d70 -> vtable +600 = OnNewGameConfirm 0x7c1ff0
    ADishonoredPlayerController::s_pInstance->GetProfileSettings() (+1424), ArkSettings::OnSettingsChanged, SaveSettings,
    m_bStartingNewGame |= 4
  then the menu runs UDisTweaks_GFxMoviePlayerMainMenu::m_NewGameCommand through
    ADishonoredPlayerController::ConsoleCommand (2013 0x1f51d0; the 2012 body 0x820080 does it inline)
  m_NewGameCommand = "ce ChangeLvl_StartNewGame"   <- Default__DisTweaks_GFxMoviePlayerMainMenu in DishonoredGame.upk,
                                                     read with resources/tools/pdb/read_package_classes.py
  "ce <name>" is a Kismet console event of DishonoredGameFull_P whose sequence does
    AWorldInfo::PrepareMapChange (exec 0x1f9de0, body 0x3845f0) -> UGameEngine::PrepareMapChange 0x229d30
    AWorldInfo::CommitMapChange (exec 0x1c6d80, body 0x384680) -> bShouldCommitPendingMapChange
    UGameEngine::ConditionalCommitMapChange 0x232770 -> CommitMapChange 0x22d570
```

`UGameEngine::Exec`'s `STREAMMAP` (0x23d020) is the same three engine calls, which is what `-startmap=<map>` issues;
`-newgame` issues the console command itself. `OPEN` (`-startmapopen`) is the travel path instead: `SetClientTravel`
(0x213ff0) → `LoadMap` 0x240b40.

## Blockers that stop milestone 5, none of them in agent AF's files

**B1 — zero-length matinee spins forever (agent AI).** `USeqAct_Interp::StepInterp` wraps a looping matinee with
`while(NewPosition > InterpData->InterpLength) NewPosition -= InterpData->InterpLength;`. `Dishonored_MainMenu`'s
`SeqAct_Interp_14` has `InterpLength == 0`, so the **world tick never returns** from the first tick after the menu map change —
the baseline smoke stalls there too, with none of agent AF's switches. Arkane keeps the tracks in `UMatineeData::m_Data` and
leaves `InterpLength` at 0 (PHASE6 AI.4). Guarded snapshot-only in `build/agentAF_snapfix.py`; the fix belongs in
`UnInterpolation.cpp` (set `InterpLength` when loading `m_Data`, or guard both wraps).

**B2 — tearing the streamed-in main menu down deadlocks.** With B1 guarded, the second map change reaches
`CollectGarbage` inside `CommitMapChange` and deadlocks: the game thread sits in `UStaticMesh::BeginDestroy ->
BeginReleaseResource -> FRingBuffer::AllocationContext` waiting for ring-buffer space while the rendering thread sits in
`FScene::RemovePrimitiveSceneInfo_RenderThread -> FPrimitiveSceneInfo::RemoveFromScene -> ~FStaticMesh -> operator delete`
inside the CRT heap. **Identical stacks 60 s apart** (`build/agentAF/dbg/hang1.txt`, `hang2.txt`), with FMallocDebug and with
FMallocBinned, and with `-onethread` the single thread sits in the same CRT heap chain. This is what stops the retail New-Game
route, and `-startmapopen` exists to get around it.

**B3 — the reference height fog aborts the rendering thread (agent AH).** With the scene render on, the first world frame
after the menu commit hits `Assertion failed: Parameter.IsInitialized()` in `FHeightFogShaderParameters::Set` from
`TBasePassVertexShader<FNoLightMapPolicy,FNoDensityPolicy>::SetParameters` — one of the reference-only passes PHASE6 AH.2 lists
as "guarded so they are never reached" (Arkane has DisFog instead). `-noscenerender` is agent AF's switch around it; milestone 5
is a null-RHI milestone, so nothing is lost. Full stack in `agentAF.log` of `build/agentAF_smoke5.txt`.

**B4 — associating `L_Tower_P`'s sub-levels hangs, so the mission map has no floor.** `L_Tower_P` keeps its geometry in 7
`LevelStreamingAlwaysLoaded` sub-levels plus one `LevelStreamingKismet` (`L_Tower_Env/Light/Nav/Fx/Audio/Script/Block/Water`).
They do load and `L_Tower_Env` and `L_Tower_Fx` are added to the world, and then the game thread hangs in
`FPendingCleanupObjects::~FPendingCleanupObjects -> FSkeletalMeshObject::FinishCleanup -> ~FSkeletalMeshObjectGPUSkin ->
~TIndirectArray<FGPUSkinDecalVertexFactory> -> operator delete`, ten frames deep in the CRT heap, ending in ntdll
(`build/agentAF/dbg/hang4.txt`). Same signature as B2, so both are most likely one bug: a heap corruption whose symptom is a
hang inside `RtlFreeHeap`. Note agents X and Z both needed a `StaticAllocateObject` +256/+512 slack stand-in for exactly that
reason in wave 3, and those stand-ins are not in the tree. **This is the last thing between the current state and a walking
pawn**, and it is the first thing the next agent should chase. `-nolevelstream` skips the association so the input chain can be
measured at all, at the price of a floorless world.

**B5 — `APlayerController::Possess` / `UnPossess` are unported (agent AE).** `Engine/Src/playercontroller.cpp` is still a
13-line `import_reference.py` stub, so `AController::Possess` runs and the controller stays in **no state**
(`GetStateFrame()->StateNode` is the class itself): `PlayerMove` dispatched nothing, so neither `PlayerMove_Walking` nor
`UpdateRotation` ran and input moved and turned nothing. Retail's `APlayerController::Possess` (2013 0x1330b0) sets `Pawn`,
`SetTickIsDisabled(inPawn, FALSE)`, `TimeMargin = -0.1`, `MaxTimeMargin` from the `AGameInfo` default and then raises the
no-parameter event that enters the pawn's land movement state. Agent AF raises `Restart` from
`ADishonoredPlayerController::Possess` as the bring-up stand-in — after which the controller is in `PlayerWalking` and input
works. Port `Possess`/`UnPossess` (0x1330b0 / 0x130090) and that stand-in can go.

**B6 — the player's input is not in the viewport client's interaction list.** `GlobalInteractions` holds only
`Console, UIInteraction, PlayerManagerInteraction`; the player's `UDishonoredPlayerInput` is not among them, so
`UGameViewportClient::InputKey/InputAxis` dropped every key and axis (`handled 0`, `aMouseX 0.000`). In UE3 the insertion is
done by script — `GameViewportClient.InsertInteraction` is a script event in the 2013 packages too
(`Defined|Event|HasOptionalParms|Public`), not a native we are missing — and `PlayerController.InitInputSystem` does run (it is
what creates `PlayerInput`). Agent AF routes unhandled viewport input to the local players' `UPlayerInput` in
`UGameViewportClient::InputKey/InputAxis`; the routing is skipped as soon as the interaction is present, so it is a no-op once
the script path is understood. **Worth a look by whoever owns the UI/script bridge**: why the script `InsertInteraction` call
has no effect (most likely `GlobalInteractions` is written by a different code path, or the script's `LocalPlayer(Player)` cast
fails).

**B7 — `UDishonoredEngine::PlayLoadMapMovie` recursed until the stack overflowed** (fixed here). Agent AC's override chained to
the reference no-argument `UEngine::PlayLoadMapMovie()`, and agent AI made that one forward to the two-argument virtual, so the
override called itself: `0xC00000FD` in `FindMapConfig` on the very first `LoadMap` (`build/agentAF/dbg/crash1.txt`). Now the
chain is the retail one, `UEngine::PlayLoadMapMovie(MapName, MapConfig ? m_LoadingMovieName : MovieName)`, qualified.

## Bring-up switches added (all `DISHONORED(bringup)`, all off by default)

| Switch | Effect |
|---|---|
| `-startmap=<map>` | after the main-menu commit, issue `STREAMMAP <map>` — the engine calls the menu's Kismet action makes |
| `-startmapopen` | issue `OPEN <map>` as soon as the tick loop runs instead, before the menu streams in (works around B2) |
| `-newgame` | issue the menu's own `ce ChangeLvl_StartNewGame` through the player controller's `ConsoleCommand` |
| `-inputtest` | hold W for 2 s and sweep mouse-X for 1 s through the viewport entry points, then log distance, yaw, peak 2D speed and acceleration |
| `-noscenerender` | skip the viewport draw (works around B3); keeps the `UpdateLevelStreaming` call the draw owns |
| `-nolevelstream` | also skip that streaming update (works around B4) |

## What is left for milestone 5

1. **B4 / B2**: the CRT-heap hang on level association and on streamed-level teardown. Until it is fixed, no run can both have
   a floor and survive a map change. Everything else for milestone 5 is in place.
2. With a floor, re-run the accept line without `-nolevelstream` and confirm `moved > 0`; the acceleration and yaw numbers above
   say the chain will deliver it.
3. B5 (`APlayerController::Possess`), B6 (`InsertInteraction`) and B1 (`InterpLength`) so the three bring-up stand-ins can go.
4. The d3d9 `-dumpframes` shots (step 5) once B3 and the Debug-heap constraint are dealt with.

## Files

Working tree (10, all agent AF's): `Engine/Src/{UnGame,UnLevTic,UnIn,UnPlayer}.cpp`,
`Engine/Inc/EngineUserInterfaceClasses.h`, `DishonoredGame/Src/{dishonoredengine,dishonoredplayercontroller,
dishonoredplayerinput}.cpp`, `DishonoredGame/Inc/CppText/{ADishonoredPlayerController,UDishonoredPlayerInput}.h`,
plus `resources/docs/agents/agentAF.md` and `agentAF_status.csv`.

Scratch (not repo tools): `build/agentAF_patch_engine.py`, `_patch_engine2.py`, `_patch_engine3.py`,
`_patch_engine3_revert.py`, `_patch_game.py`, `_snapfix.py`, `_dedup_shims.py`, `_dedup_stubs.py`, `_cpptext_hooks.py`,
`_findtweak.py`, `_files.txt`, `build/agentAF_build*.log`, `build/agentAF_smoke*.txt`, `build/agentAF/dbg/*`,
snapshot `build/agentAF_wt` (contains no stage directory and no junction), IDA copies
`resources/docs/idb/{shipping2012,retail2013}_agentAF.i64`.

No commits, no `git add`.
