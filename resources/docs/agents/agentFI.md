# Agent FI — into the game: the boat scene, the opening Kismet, and the letterbox bars

Wave 18 (`PHASE14.md`), package FI. Worktree `build/agentFI_wt`, base `18f36a3`. Sixteen changed files plus one new
header: eleven in `DishonoredGame`, three in `Engine`, one in `AkAudio`, one in `External/Wwise2012`. **Nothing in
`External/GFx3` or `GFxUI`** — FF, FG and FH own those and this package opened neither (§9).

## 1. The short version

The user's fault 5 is **three separate faults with one shared upstream blocker**, and two of the brief's
three premises are wrong.

| the brief said | what is true |
|---|---|
| "No pawns are placed. Nothing is spawned into the level at all." | **Half wrong.** `l_tower_p` already has 26 NPC pawns, every one with a controller and a skeletal mesh and six of them drawn (`-fidiag`, measured on HEAD as well as here). What is missing is the **two the opening scene spawns**, and they come from `DisSeqAct_StartSpawn_1`, a Kismet action whose class had no C++ body. **Fixed**: 27 → 29 pawns, 7 → 8 drawn, within two seconds of the action running (§4) |
| "The matinee does not start." | **Wrong as stated.** `l_tower_p`'s Kismet *stalls*, and it stalls on a **latent `SeqAct_AkPostEvent`** that can never complete because nothing in this tree ever issues an audio frame. **Fixed** (§3): the stuck action completes, the `SOIREE_*` sub-sequences run and seven `SeqAct_Interp` on `L_Tower_Script` are live. The *cutscene* matinee — the one with a director group — is still not among them, and §7 says exactly what it needs |
| "The bars do not draw." | **Right, and fixed** (§5). Four bodies, one of them FE's blocker, which turned out not to be a blocker at all: the nine parameters are named in a truth source this tree already holds |

And one thing the brief could not have known, which blocked everything else:

> **At HEAD `18f36a3` the main menu is closed one second after it opens and New Game is unreachable.**
> Agent FE's `DishonoredGFxNotifyGameSessionEnded()` in `UGameEngine::CommitMapChange` fires on the front
> end's *own* streaming commit. Measured twice, in two independent runs (§2). Fixed, and the retail-faithful
> call site found.

**The headline frame is `build/agentFI/fi_fault5_before_after_retail.png`**: the user's own half of
`resources/reference/menu/5.png`, this package, and retail's half, side by side — same boat, two NPCs,
letterbox bars.

## 2. The regression at HEAD: the menu is closed one second after it opens

The first thing this package did was reproduce the user's route on HEAD, and it could not: the main menu
never appeared at all.

```
FIH1.log:788  [0022.44]  -gfxuimenu: opened UI_MainMenu.MainMenu through DisGFxMoviePlayerMainMenu
FIH1.log:862  [0023.24]  Committed map change via DishonoredEngine
FIH1.log:865  [0023.25]  GFx UI focus: 1 local player(s), 1 player state(s), 0 movie(s) open
```

and again, 1.2 s apart instead of 0.8, in a second run with different ini files
(`FIH2.log:790 / :865 / :868`). `build/agentFI/shots/fih2_0.png` is what the user would see: the menu
world upside down with no interface on it, and the five driven key presses do nothing because there is no
movie to receive them.

**The mechanism.** `DishonoredGameFull_P`'s own Kismet commits a map change into `Dishonored_MainMenu` —
that is how the front end reaches its own menu level, and agent EZ's note in `Core/Src/UnMisc.cpp:5078`
says so explicitly. Agent FE put the GFx close in `CommitMapChange`, so the front end closes its own menu.
FE's report is candid that the retail call site was never pinned.

**Where it belongs, read out of the reference engine rather than guessed**:
`UWorld::CleanupWorld(bSessionEnded)` (`Engine/Src/UnWorld.cpp:3122`) → `GameViewport->eventGameSessionEnded()`
→ `GameViewportClient.uc:815` walks `GlobalInteractions` calling `NotifyGameSessionEnded` →
`GFxInteraction.uc:34` declares that native → `FGFxEngine::NotifyGameSessionEnded` →
`CloseAllMovies(TRUE)`. **No `GFxInteraction` is ever registered in `GlobalInteractions` in this tree**, so
the script half is a no-op and the C++ half had no caller — which is why FE found none.

This package does both halves:

* `UWorld::CleanupWorld` raises it directly when `bSessionEnded`, which is the reference's own position for
  it. That is load-bearing and not cosmetic: without it a movie open across a `LoadMap` keeps references
  into the dying world and the next collection dies with
  `World DishonoredGameFull_P.TheWorld not cleaned up by garbage collection!` — measured, `FIF6.log:1705`,
  the moment the front end stopped closing its movies at its own commit.
* `CommitMapChange` keeps FE's call but only for the commit that **replaces a level this engine had already
  committed as persistent**, which is the New Game commit and not the front end's first one. The
  `ULevelStreamingPersistent` the previous commit left behind is what tells the two apart, and the loop that
  finds it was already in the function.

Retail does not decide this here at all: `UDisGlobalUIManager::RefreshGlobalUIState` (2012 `0x8b75e0`)
opens and closes the movie set as the game moves between the front end and a level, and it is unported.

**After**: `build/agentFI/shots/fi3_0.png` is the main menu, drawn, with NEW GAME selected.

## 3. The blocker under the whole mission: a latent audio action that can never finish

With the menu back, New Game reaches `l_tower_p` — and the level's Kismet stops. `-diskismet` on HEAD:

```
FIH5.log  kismet census: 6 ops active now: l_tower_p...SeqAct_AkPostEvent_0 (SeqAct_AkPostEvent, latent 1, activations 1), ...
          kismet census: ops ever activated per level: l_tower_p 46/1670, L_Tower_Script 32/1629
```

`USeqAct_AkPostEvent::UpdateOp` returns `Signal == 0`, `Signal` counts the events still playing, and the
only thing that decrements it is `AkCallback` on `AK_EndOfEvent` (`akaudioclasses.cpp:48`). **No
`AK_EndOfEvent` is ever delivered in this tree**, for two reasons, both measured:

1. `AK::SoundEngine::RenderAudio()` has no caller. `UGameEngine::Tick` reaches the audio device through
   `UClient::GetAudioDevice`, and `UWindowsClient` only builds one when `GEngine->bUseSound` and a
   configured `AudioDeviceClass` are both present (`WinDrv/Src/WinClient.cpp:353`); neither is, so
   `UAkAudioDevice::Update` is called by nothing at all.
2. The silent sound engine records every posted voice and its callback but only ends them on
   `StopAll`/`Term`.

Both are fixed, each in the place retail has it:

* `AK::SoundEngine::RenderAudio()` is issued at the end of `UAkAudioDevice::Update`. Retail issues it from
  the tail of `UDishonoredAudioSystem::Update` (2013 rva `0x7b3520`); the call at `0x7b4783` is the **last
  instruction of that body** and the **only xref to `RenderAudio` (`0x913670`) outside Wwise itself**, and
  that 4,734-byte body is unported, so the one line of it the game cannot run without is issued from its
  immediate caller. Ordering is identical.
* `UAkAudioDevice::Update` gets a caller: `GDisAudioFrameHook`, the same function-pointer seam shape
  `GDisEngineTickHook` and `GDisPlayerPreRenderHook` already use, installed by `AkAudio` and called from
  `UGameEngine::Tick` immediately after the dead `Client->GetAudioDevice()` block.
* The silent engine's `RenderAudio()` ends every voice posted since the last audio frame. A silent event
  has no duration, so it ends on the next frame. **The deviation is only in the wait**: retail resumes when
  the sound really finishes, we resume one audio frame later. Nothing plays a sound either way.

**Measured, identical command lines, `-startmap=l_tower_p -startmapopen`:**

| | HEAD `18f36a3` (`FIH5.log`) | this package (`FIF7.log`) |
|---|---|---|
| ops active | **6**, including `SeqAct_AkPostEvent_0` latent for ever | **5**, and `SeqAct_AkPostEvent_0` is gone |
| ops ever activated | `l_tower_p 46/1670`, `L_Tower_Script 32/1629` | the same, `46/1670` and `32/1629` |

The op counts are identical on that route on purpose: `-startmapopen` skips the hub sequence that sets the
story state, so the opening scene does not run on either executable and the only thing the fix changes
there is the stall itself. The scene's own numbers come from the **New Game route**, which only this
package can reach: `l_tower_p 65/1670`, `L_Tower_Script 47/1629`, with `SOIREE_Empress_Regent`,
`SOIREE_Emily` and seven `SeqAct_Interp` live (`FI5.log`, `FI9.log`). There is no HEAD number to put beside
those, because at HEAD New Game cannot be reached at all (§2).

## 4. The boat's two NPCs

Two things were wrong and the second one is the interesting one.

### 4a. `UDisSeqAct_StartSpawn` had no C++ body

The class is registered, the Kismet nodes exist (21 of them across `l_tower_p`'s levels), and
`Src/disseqact_startspawn.cpp` was a comment-only skeleton on the exclude list, so the base
`USeqAct_Latent` ran: the op went latent and never came back. Ported, all four bodies:

| | 2013 rva | 2012 | size |
|---|---|---|---|
| `Activated` | `0x79bcc0` | `0x7d7480` | 305 |
| `UpdateOp` | `0x790050` | `0x7d2dd0` | 293 |
| `OnPawnSpawned` | `0x78aca0` | `0x7cdbd0` | 15 |
| `OnSpawnFailed` | `0x78acb0` | `0x7cdbe0` | 15 |

`Activated` counts the `ADishonoredSpawner` object variables, chains to the latent base (which attaches the
op to those actors and gets `ADishonoredSpawner::OnStartSpawn` called on each) and pulses output 0;
`UpdateOp` drains one pulse of output 2 per pawn that arrived and output 3 per refusal and finishes, with
output 1, when nothing is pending; the two interface bodies are the counters.

`IDisSpawnRequesterInterface` had no virtuals at all. Three are added in retail's own order:
`GetAllowedSpawnCount` at interface vtable **+8** (`DoSpawnNow`'s refusal is reason code 5),
`OnPawnSpawned` at **+12**, `OnSpawnFailed`. The interface subobject sits at `UDisSeqAct_StartSpawn+264`,
confirmed twice over: `OnStartSpawn` builds its `TScriptInterface` from `action+264`, and the adjusted
`this` of `OnPawnSpawned` is `+264` as well (its `--*(this+8)` is `m_PendingSpawnCount` at 272 and its
`*(this-88)` is `OutputLinks.Num()` at 176).

### 4b. The defect that made the spawn silent: a zeroed `FDisSpawnInfo`

`ADishonoredSpawner::OnStartSpawn` built `FDisSpawnInfo SpawnInfo( EC_EventParm )` — all zeroes — and
passed it. Retail's body (2013 `0x65c150`, 147 bytes, **2012 match ratio 1.000 by bytes**) fills two things:

```c
v6 = a2;  if (a2) v2 = (char*)a2 + 264;        // m_pSpawnRequester = { object: the action, interface: +264 }
v4 = ((BYTE)v9 ^ (BYTE)*((DWORD*)a2 + 67)) & 1 ^ v9;   // bit 0 of action+268 -> bit 0 of the spawn info
```

`action+268` is `m_bSpawnEvenIfVisible`. **That bit is the whole fault.** `DoSpawnNow`'s player-proximity
gate is `if( GIsGame && !m_bCanSpawnWhenPlayerNear && !_rSpawnInfo.m_bSpawnEvenIfVisible )`, and the
opening scene spawns its two actors a few metres from a player who is standing in the middle of it. With
the bit dropped the gate refused every one of them and **returned a reason code nobody read**. Measured:
`StartSpawn DisSeqAct_StartSpawn_1: 2 spawner(s) asked, spawnEvenIfVisible 1, latent actors 2`.

`DoSpawnNow` also tells the requester on success now, which is retail's own
`if( *(DWORD*)a3 ) { v29 = *(DWORD*)(a3+4); if (v29) (*(vtable+12))(v29, newPawn); }` immediately after
`OnSpawned`. Without it the latent op would spawn and then hang for ever on its own pending count.

**Measured** (`FIF9.log`, New Game at world time 25.02):

```
[0032.61] fidiag pawns 14: t 26.31 map l_tower_p, 27 pawns, 27 with a controller, 27 with a skeletal mesh, 0 bHidden, 7 drawn
[0035.38] StartSpawn DisSeqAct_StartSpawn_1: 2 spawner(s) asked, spawnEvenIfVisible 1, latent actors 2
[0036.51] fidiag pawns 16: t 30.37 map l_tower_p, 29 pawns, 29 with a controller, 29 with a skeletal mesh, 0 bHidden, 8 drawn
```

### 4c. The premise correction, with the number

"Nothing is spawned into the level at all" is wrong. Both at HEAD and here, before the opening scene runs,
`l_tower_p` holds **26 `ADishonoredNPCPawn` + the player**, every one with a controller and a skeletal mesh
and **none hidden**. They are up at the Tower, 17,000–25,000 units from the boat, which is why the user
could not see any of them: `DishonoredNPCPawn_0 ... dist 17106`, `_5 skm_Empress_Body dist 25286`,
`_7 Skm_LordRegent_Body dist 25386`. `-disai` agrees independently: `26 NPC pawns, 26 controllers (26 with
a brain, 26 initialized), 41 spawners`. The fault was always about the two the scene spawns.

## 5. The letterbox bars

FE named the chain end to end and stopped on one thing. That one thing was not a blocker.

> **FE: "`execSetCinematicMode_Native` reads nine booleans and an optional tenth off the script stack, and
> no truth source in this tree names them."** Nine, no tenth, and `resources/docs/types/retail_sdk_layout.json`
> names all nine:
> `DishonoredGame.DishonoredPlayerController.SetCinematicMode_Native`, `params_size 36`,
> `_bInCinematicMode _bWasInCinematicMode _bHidePlayer _bHideHUD _bAffectsMovement _bAffectsTurning
> _bAffectsButtons _bHideLetterbox _bAllowProjectilesAndExplosions`, every one `CPF_Parm` and **not one
> `CPF_OptionalParm`**. The "optional tenth" is `P_FINISH`: the `if( *Stack.Code == 65 )` at the end of the
> exec is `EX_DebugInfo` (0x41), the script-debugger tail every exec in this executable carries. The nine
> names line up one for one with the nine `Stack.Step` calls at `0x5eea00`.

Ported:

| | 2013 rva | what it does |
|---|---|---|
| `execSetCinematicMode_Native` | `0x5eea00` | nine `P_GET_UBOOL`, was a `DISHONORED_NATIVE_STUB` |
| `execPreSetCinematicMode_Native` | `0x5ee960` | two, was a stub |
| `SetCinematicMode_Native` | `0x6af150` (252 B, vtable +1412) | the two bitfield writes and the four HUD mask writes |
| `PreSetCinematicMode_Native` | `0x6a33a0` (49 B, vtable +1408) | hides and unhides the player pawn |
| `UDisGFxMoviePlayerHUD::Tick_Cinematic` | `0x7960e0` (358 B) | `_root.blackStripes_mc.SetBlackStripes( bShow, TRUE )` |

`0x6af150` is the address FE warned about — the match table's 0.630 "neighbours" — so it is pinned by its
body: it is the only function in the executable that calls `DisableHUDElements`, `EnableHUDElements`,
`DisPushDisableSave`, `DisPopDisableSave` and `DisGetGlobalProjectileManager`. Its two bitfield writes land
on `this+1552` bit `0x1000` and bit `0x20000`, which `retail_sdk_layout.json` names
`m_bInputIgnoreInput_Cinematic` and `m_bAllowProjectilesAndExplosionsInCinematicMode`; its HUD pointer is
`this+984`, which is `myHUD`; and its mask-level argument is the literal `0`, which is
`eDisHUDMaskLevel_Cinematic`. `Tick_Cinematic`'s clip is `m_pMovieClips + 448` and `DHMC_Cinematic` is 28
with a 16-byte `GFxValue`, so it is `Clips[DHMC_Cinematic]` three ways over.

Two retail calls are left out and marked, because neither has anything to call here:
`DisPushDisableSave` / `DisPopDisableSave` (the save lock) and `DisGetGlobalProjectileManager` plus the
manager's destroy-all (`0x8410b0`).

`SetBlackStripes` is **in the cooked movie**: two occurrences in the `UI_HUD_SF.UI_HUD.HUD.gfx` payload,
extracted and counted rather than assumed.

**And the HUD movie has to be open in the level.** It was not: `-dishud` opened it once at start-up, the
map change into the mission closed it, and the bring-up never asked again, so the mission had no HUD and
no bars even with everything above ported. The switch now re-arms when the movie it opened is no longer
open — which is what `UDisGlobalUIManager::RefreshGlobalUIState` does in retail.

**Measured** (`FIF9.log`): mask level 0 is `0x6050` in `l_tower_p` — `0x7fef` minus the non-cinematic set
`0x7FAF` is `0x0040`, plus the cinematic set `0x6010` is `0x6050`, exactly — the letterbox bit `0x10` is on,
`HUD: opened UI_HUD.HUD` at 32.13 and `HUD: SetBlackStripes(1) ok` at 32.18. On the **menu** the same mask
reads `0x6040`, because the menu's own `ToggleCinematicMode` passes `_bHideLetterbox` TRUE and the extra
`DisableHUDElements(0, 0x10)` runs — which is retail's behaviour and why there are no bars on the menu.

`build/agentFI/shots/fi9_1.png`, `fif9_4.png`: the bars, top and bottom, over the boat.

## 6. What the before/after is, exactly

* **HEAD, the user's own route, same driver** (`drive_input.py`, SPACE then four ENTERs keyed to
  `-gfxuimenu: opened UI_MainMenu`): the menu movie is closed 0.8–1.2 s after it opens, the game sits in
  the menu world with no interface, and the mission is unreachable. `build/agentFI/shots/fih2_0.png`.
* **This package, same driver**: the main menu is drawn (`fi3_0.png`) — and then stops there, because
  **Enter still does not activate a main-menu row**. That is STATUS open defect 5 and it is the front
  end's, not this package's; measured again here: SPACE reaches the movie and runs
  `TransitionTo(MainMenuScreen)` (`FI3.log:1153`), and the four Enters that follow produce **no AS2 trace
  at all**. `-finewgame[=<seconds>]` is shipped as the instrument that gets past it: it is the second half
  of `UDisGFxMoviePlayerMainMenu::OnNewGameConfirm` (2013 `0x7c1ff0`) and nothing else — the same
  `ce ChangeLvl_StartNewGame` through the same `ConsoleCommand` — so the hub sequence runs the identical
  `SetPlayerTravelDestination` → `CommitMapChange` → `GotoPlayerTravelDestination` it runs for a click.
* **The level, identical command lines on both executables** (`-startmap=l_tower_p -startmapopen
  -dishud -disai -diskismet`): HEAD 6 ops active with `SeqAct_AkPostEvent_0` latent, this package 5 and the
  stall gone. That route does **not** reach the opening scene on either build — it skips the hub sequence
  that sets the story state, and both read `l_tower_p 46/1670` — so it is a control for the audio fix and
  not for the scene.
* **The scene itself**, New Game route: `build/agentFI/fi_boat_head_vs_fi_vs_retail.png` and
  `fi_fault5_before_after_retail.png`.

Every frame cited here was claimed by `-apshotname=<tag>` with a tag unique to its run and copied out
immediately; none of them rests on an arrival-time claim, and each run's log names the tag it wrote
(`-apshottime: screenshot N of M, ... name 'fi9'`).

## 7. What is still not right, measured, for the next package

**The opening cutscene's own matinee does not run.** What runs in `l_tower_p` is seven ambient matinees and
none of them has a director group (`-fediag`, `FIF9.log:6289`):

```
SeqAct_Interp_6  playing 1 pos 2.05  [Plank0=>InterpActor_13]      director NONE
SeqAct_Interp_10 playing 1 pos 6.89  [Rat=>SkeletalMeshActorMAT_8] director NONE
SeqAct_Interp_2  playing 1 pos 9.89  [Rat=>SkeletalMeshActorMAT_6] director NONE
SeqAct_Interp_16 playing 1 pos 0.72  [Rat=>SkeletalMeshActorMAT_5] director NONE
SeqAct_Interp_15 playing 1 pos 0.72  [Rat=>SkeletalMeshActorMAT_7] director NONE
SeqAct_Interp_0  playing 1 pos 8.04  [Whale=>InterpActor_15]       director NONE
SeqAct_Interp_14 playing 1 pos 47.63 [WhaleBoat_A=>InterpActor_14] director NONE
```

Rats, planks, a whale and a whaleboat — the set dressing, advancing normally. **Nothing with a director
group and nothing bound to the two NPCs.** Five frames of the scene 45 s to 110 s apart differ by a mean of
2.0/255, so the camera does not move and the actors do not act.

Three things to settle, in this order, none of them this package's:

1. **`UInterpGroupPlayer` and `UInterpGroupInstPlayer` are entirely unported**
   (`Engine/Src/interpgroupplayer.cpp`, a 19-line skeleton: `NeedWaitToStart` `0x53b610`,
   `GetStageMarkPosition` `0x53b6c0`, `BlendPlayerToMark` `0x53c5c0`, `UpdateGroupInst` `0x53dd00`,
   `TermGroupInst` `0x53dd50`, `InitGroupInst` `0x541960`, all **2012** rvas), **and
   `USeqAct_Interp::InitInterp` has no branch that would build a `UInterpGroupInstPlayer`** — it has
   Director, AI, Camera and plain, so a player group silently gets a plain `UInterpGroupInst`. An opening
   scene the player rides is exactly a player group.
2. **`ADishonoredNPCPawn::MAT_*` — the whole matinee-AI contract — is a comment-only banner with zero
   bodies** (`dishonorednpcpawn.cpp:23-59`): `MAT_BeginAIGroup` `0x7c0570`, `MAT_FinishAIGroup` `0x7c0830`,
   `MAT_SendBodyIntentionRequest` `0x7c0a10`, `MAT_IsAllowed` `0x7c0c20`, `MAT_PausedAIGroup` `0x7ad570`,
   `MAT_SendAimRequest` `0x7ad790` and about forty more. Nothing can drive an NPC from a cutscene.
3. **`UpdateTrack` is missing on every Dishonored-added `UInterpTrack`**: `AIControl` (`0x53e120`),
   `Attachment` (`0x53e560`), `FaceTo`, `LookAt`, `Locomotion`, `StretchAnimControl`, `Dialog`,
   `DisAllowPlayerLook` (`0x8dc4a0`), `SoireeControl`, `AkEvent`. Those are the tracks a Dishonored
   cutscene is made of.

**And a sweep worth having**: of the fourteen `UDisSeqCond_*` classes, **twelve have no `Activated` body**,
and an unported `USequenceCondition` kills its whole chain silently (agent EL). The 2013 addresses, mapped
and checked (the skeleton comments carry the **2012** ones, which is a trap: `IsSentinel`'s comment says
`0x7d1fd0` and its ported body says `0x78f070`):
`CompareBoolExtended 0x79b3e0`, `CompareTweaks 0x78eaf0`, `InStoryGroup 0x78ecb0`, `IsDLCUnlocked 0x78ee30`,
`IsDoorOpen 0x78eec0`, `PawnIsPossessed 0x79b5f0`, `HasObjective 0x78ec20`, `IsObjectiveComplete 0x78ef30`,
`TaskState 0x7a78c0`, `IsPlayerInCombat 0x78efc0`. `CompareTweaks` and `CheckStoryFlag` are both already on
this mission's live path (`DisSeqCond_CompareTweaks x2`, `DisSeqCond_CheckStoryFlag x4` executed), so the
missing ten are not hypothetical.

Three smaller ones, also measured here: every NPC's head is craned up (agent DO's
`UArkAnimNodeLookAt::UpdateNodeWeights`, visible in every frame of §6); the scene has none of retail's
haze; and `-fediag`'s `InterpLength` column reads `0.00` for every matinee because it prints the
`UInterpData` shim member, not `UMatineeData`'s real one (agent DK). The column is wrong, not the data.

## 8. Addresses, resolved by hand

`rva_sweep.py`: **7,414 citations, 3 suspects, none of them in a file this package touched**
(`disbehaviorpatrol.cpp:319` and `gfxuirenderer.cpp:1679` twice, all three pre-existing and identical to
what agent FE reported). Every address below was also resolved by name or by body in
`retail2013_named.i64` (copied to `build/agentFI/idb/FI_retail2013.i64`; image base `0x400000`).

| RVA (2013) | what, and how it was pinned a second time |
|---|---|
| `0x5eea00` | `execSetCinematicMode_Native` — named; nine `Stack.Step` calls then `P_FINISH`, dispatch through vtable +1412 |
| `0x5ee960` | `execPreSetCinematicMode_Native` — named; two steps, vtable +1408 |
| `0x6af150` | `SetCinematicMode_Native` — the match table's 0.630 is not evidence; pinned by body (the only caller of that five-function set) and by its three offsets, 1552 / 984 / 0 |
| `0x6a33a0` | `PreSetCinematicMode_Native` — named, 49 bytes, two `AActor::SetHidden` on `ADishonoredPlayerPawn::s_pInstance` |
| `0x7960e0` | `Tick_Cinematic` — named at `disgfxmovieplayerhud.cpp:1179`; body reads `this+508` bit `0x40000` (`m_bCinematicMode`) and `this+512` (`m_pMovieClips`) `+448` (`DHMC_Cinematic` 28 × 16) |
| `0x79bcc0` | `UDisSeqAct_StartSpawn::Activated` — named in the 2013 database; 2012 `0x7d7480` at 0.971; body writes `this+272` and reads `this+184`, which is this class's own `VariableLinks` / `m_PendingSpawnCount` layout |
| `0x790050` | `UDisSeqAct_StartSpawn::UpdateOp` — named; `this+276` / `+280` and `OutputLinks` elements 1, 2, 3 at 56 bytes each |
| `0x78aca0`, `0x78acb0` | `OnPawnSpawned` / `OnSpawnFailed` — named; 2012 matches at ratio **1.000 by bytes**; the adjusted `this` is `+264`, the interface subobject |
| `0x65c150` | `ADishonoredSpawner::OnStartSpawn` — named; 2012 `0x6a36e0` at ratio **1.000 by bytes** |
| `0x65e790` | `ADishonoredSpawner::DoSpawnNow` — already ported by agent CG; read only, for the requester calls at interface vtable +8 and +12 |
| `0x6590e0` | `ADishonoredSpawner::OnSpawned` — already ported; read to confirm it is **not** where the requester is told |
| `0x7b3520` | `UDishonoredAudioSystem::Update` — `UAudioSystem` vtable slot 76, already named in this tree's `EngineArkaneClasses.h:170`; 2012 `0x818ff0` at 0.771; and it is the only non-Wwise xref to `RenderAudio` |
| `0x913670` | `AK::SoundEngine::RenderAudio` — named in `functions_2013.csv`; its five xrefs are four inside Wwise and `0x7b4783` |
| `0x8410b0` | the projectile manager's destroy-all — read only, not ported; identified by its `DisDestroyActorNextTick` / `GetNextProjInList` loop |

The twelve `UDisSeqCond_*` addresses in §7 were mapped through `match_2012_2013.csv`, checked against the
one class that is already ported (`IsSentinel`, 2012 `0x7d1fd0` → 2013 `0x78f070`, which is what
`disseqcond_issentinel.cpp` already says), and then resolved one by one with `ida_funcs.get_func` on the
2013 database: **all twelve are function starts**, nine of them named
`?Activated@UDisSeqCond_<name>@@UAEXXZ` outright, and the three the 2013 database leaves as `sub_` match
the match table's sizes exactly (`CompareBoolExtended` 521, `PawnIsPossessed` 403, `HasObjective` 140).
**They are handed over, not used**, because this package ports none of them.

## 9. Merging

* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is not required and was
  not run.** No reflected member was added or moved and no class was added. The two edits inside generated
  headers are the mechanism the generator preserves, or are additive C++:
  one `#include "CppText/UDisSeqAct_StartSpawn.h"` in `UDisSeqAct_StartSpawn`'s body
  (`dishonoredgamekismetclasses.h`, the same mechanism agents EL and FE used), and three virtuals with
  inline default bodies on `IDisSpawnRequesterInterface` in `dishonoredgameclasses.h`. **The interface
  edit is the one a regeneration would drop** — it is flagged here so a later `--sdk` run re-applies it.
* **`Sources.cmake` changed by one unit**: `Src/disseqact_startspawn.cpp` comes off the exclude list — and
  **it needs a link anchor, which agent FE's unit did not.** Measured the hard way: the first build
  compiled `disseqact_startspawn.cpp.obj`, the exe linked, and the executable did not contain one of that
  unit's strings — MSVC takes a member of a static library only to resolve an undefined symbol, and
  `IMPLEMENT_CLASS`'s vtable is a COMDAT the linker folds rather than a reference into this object. The run
  that proved it looked exactly like a virtual that was not being dispatched.
  `DishonoredLinkDisSeqActStartSpawn` has external linkage and is referenced from
  `DishonoredGameRegistrants.cpp`, which the link always pulls in.
* **Files outside `DishonoredGame`: five. None in `External/GFx3`, none in `GFxUI`.** FF merged as
  `63c2539` and every file it touched is in `External/GFx3`, so there is no overlap; the sync script
  re-checks that against main at merge time.
  * `Engine/Inc/UnWorld.h` — one `extern` declaration
  * `Engine/Src/UnGame.cpp` — the `CommitMapChange` guard, the audio-frame call and its pointer
  * `Engine/Src/UnWorld.cpp` — the GFx session-end close in `CleanupWorld`
  * `AkAudio/Src/akaudiodevice.cpp` — `RenderAudio()` in `Update`, and the seam that calls `Update`
  * `External/Wwise2012/Src/AkSilentSoundEngine.cpp` — `EndAllPlaying` in `RenderAudio`
* **Sixteen changed files plus one new header.** Eleven changed in `DishonoredGame` (one of them
  `Sources.cmake`), three in `Engine`, one in `AkAudio`, one in `External/Wwise2012`; the new file is
  `DishonoredGame/Inc/CppText/UDisSeqAct_StartSpawn.h`.
* `build/agentFI_sync.py` is the authoritative copy list, worktree → main, and flags rather than copies
  anything changed in main since `18f36a3`.

## 10. Hand-over

1. **The cutscene matinee is a package, and §7 names it**: `UInterpGroupPlayer` /
   `UInterpGroupInstPlayer` plus the `InitInterp` branch that builds one, the `MAT_*` contract on
   `ADishonoredNPCPawn`, and `UpdateTrack` on the Dishonored tracks. The ambient matinees already run, so
   the runtime underneath is sound.
2. **Twelve `UDisSeqCond_*::Activated` bodies are missing** and two of the fourteen conditions are already
   firing on this mission's path. The 2013 addresses are in §7. Watch the skeleton comments: they carry
   **2012** rvas.
3. **`-finewgame[=<seconds>]` starts a new game from the game thread** without touching the menu, and
   `-fidiag` reports every pawn with the three things that decide whether it is on screen, plus the
   cinematic mask state. Both cost nothing when absent.
4. **The menu's Enter key is the next thing between the user and the game.** §6 has the measurement.
5. **A unit taken off `Sources.cmake` can compile, link green and not be in the executable.** §9. The
   cheapest check is to count one of its strings in the exe as UTF-16.
6. **`-startmapopen` is not a substitute for New Game**: it reaches `l_tower_p` without the hub sequence
   that sets the story state, and the opening scene does not run on it on either executable.

## 11. Acceptance, and where the evidence is

| | |
|---|---|
| `run_regression.py` | **37 ok, 0 failed, 0 skipped**, 912 s. `build/agentFI/fi_regression.log` — the worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentFI_reg`, built inside the harness (no `--no-build`), with the five gitignored `resources/docs/types/` inputs copied in first |
| clean full release build | directory deleted first, `-DDISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**, `DishonoredGame.exe` linked at 23,993,856 bytes from 965 targets. The only `C42xx` warnings in the whole log are the pre-existing C4100/C4127/C4191/C4244/C4267/C4316/C4335/C4644/C4828/C4838/C4996 set. `build/agentFI/fi_clean_build.log` |
| `rva_sweep.py` | 7,414 citations, 3 suspects, none in a file this package touched; every cited address also resolved by hand in §8 |
| `build/agentFI_sync.py` | 16 files to copy, 3 new (one header and the two documents), **0 flagged** against main at `f72f9f5`, which already carries FF's `63c2539` and FH's merge |

`build/agentFI/`:

| file | what it is |
|---|---|
| `fi_fault5_before_after_retail.png` | **the headline**: the user's half of `reference/menu/5.png`, this package, retail's half |
| `fi_boat_head_vs_fi_vs_retail.png` | HEAD through the same driver (no menu at all), this package, retail |
| `shots/fih2_0.png` | HEAD: the menu world with no interface, the regression of §2 |
| `shots/fi3_0.png` | this package: the main menu, drawn |
| `shots/fi9_1.png`, `shots/fif9_*.png` | the boat scene: two NPCs, letterbox bars |
| `ref5_ours.png`, `ref5_retail.png` | the user's screenshot split into its two windows |
| `dec/`, `dec2/`, `dec3/`, `dec_spawn/`, `dec_sp2/`, `dec_sp3/` | the retail decompiles every body here was read from |
| `swf/UI_HUD_SF.UI_HUD.HUD.gfx` | the cooked HUD movie, extracted to count `SetBlackStripes` |
| `idb/FI_retail2013.i64` | this package's copy of the retail database |
| `fi_run.ps1` | one measured run: stage, launch, drive, claim the screenshots by name |
