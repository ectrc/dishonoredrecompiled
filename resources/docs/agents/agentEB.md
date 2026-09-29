# Agent EB (PHASE11 EB) — the three object trees the game never constructed, and the world the save was taken in

Branched from the merged HEAD `94889d4`. Own worktree `build/agentEB_wt`, own build dirs
`build/agentEB_release`, `build/agentEB_debug`, `build/agentEB_clean`, own IDA copies
`build/agentEB_ida/retail2013_agentEB.i64` and `shipping2012_agentEB.i64`. Nothing committed, nothing staged.

## The answer in eight lines

* The three trees now exist and resolve: `DishonoredGameInfo`, `DisObjComp`, `PowersComp` — and with them
  `GlobalFactionManager`, `pNPCTravelManager`, `pGlobalUIManager`, `pPpManager` and every other instanced
  sub-object and component in the tree.
* It was **not** three separate constructions. It was **three naming defects**, each a branch of Dishonored's
  engine that this tree carried as stock UE3 reference code, and two of the three are one-line.
* Eight `GameLoad` override bodies were ported — the ones agent ED's frontier named — plus ten helpers.
* And the **real** blocker turned out to be the one nobody had named: the harness restored into the *front
  end's* world instead of the mission's. The save says which world it wants; the restore now streams it in.
* The measured result: dictionary records resolving **2,259 → 6,227**, not-found **9,027 → 5,093**, actors
  spawned from their tweaks **24 → 50**, objects restored **2 → 4**, object-stream bytes **28 → 74**.
* The stream's stopping point changed kind three times and is now an honest override frontier again:
  `ADishonoredNPCPawn::GameLoad`, retail `0x76d3f0`, which needs `ADishonoredPawn::GameLoad` (retail
  `0x75c0a0`, 1,928 bytes) ported in full — and that one body also finishes the player pawn.
* Blocker 1 (the fog shader assert) is **fixed**, in the build class where the assert lives: a Debug d3d9 run
  renders 195 scene censuses of `DishonoredGameFull_P` where agent ED's stopped on the first frame.
* Blocker 2 (`UWorld::Tick` runs exactly twice) **does not reproduce and was a mis-attribution**: the startup
  map ticks **85 to 86** times before the arm fires under the null RHI. The number is now in the log.

## 1. The three trees were three naming defects

Agent ED had proved that porting more `GameLoad` bodies could not help, because the stream stops on the
*object*. It was right. What it could not see from the save layer is *why* those objects were missing, and the
answer is the same answer three times: **retail names them deterministically and this tree numbered them.**

Every one of the three was found by decompiling retail and reading the name that goes into the constructor.

### 1.1 The game info actor — `UWorld::SetGameInfo`, retail `0x38bfa0`

Retail's tail, from `build/agentEB/dec2013/sub_78BFA0_38bfa0.c`:

```
v50 = *((_DWORD *)v29 + 11);   // GameClass->Name.Number  (UObject::Name @40, Number @44)
v49 = *((_DWORD *)v29 + 10);   // GameClass->Name.Index
v48 = UWorld::SpawnActor(v56, v29, v49, v50, &Location, &Rotation, 0, 0, 0, 0, 0, 0, 0, 0);
```

`SpawnActor( GameClass, GameClass->GetFName() )`. This tree called `SpawnActor( GameClass )`, so the actor was
`DishonoredGameInfo_0` and the bare `DishonoredGameInfo` every save records matched nothing. Agent Z had
already ported exactly this shape into `AGameInfo::execSpawnPlayerController` (retail `0x2d13a0`, "with the
class name as the actor name") — which is why the player controller and the player pawn *did* resolve and the
game info did not. One line.

The offsets were checked, not assumed: `UObject` is vtable@0, `Index`@4, `ObjectFlags`@8, `HashNext`@16,
`HashOuterNext`@20, `StateFrame`@24, `_Linker`@28, `_LinkerIndex`@32, `Outer`@36, **`Name`@40**, `Class`@48,
`ObjectArchetype`@52, 56 bytes.

### 1.2 Every instanced component — `FObjectInstancingGraph::GetInstancedComponent`, retail `0x961b0`

Outside the archetype and UCC-make paths, retail names the instance after its template and renames whatever
already holds that name under the same outer out of the way first:

```
v23 = (char *)a3 + 40;                                   // SourceComponent->Name
ObjectFast = UObject::StaticFindObjectFast( SourceComponent->Class, ComponentOuter, Name );
if ( ObjectFast ) ObjectFast->Rename( NULL, ComponentOuter, 5 );   // REN_ForceNoResetLoaders|REN_DoNotDirty
ConstructObject<UComponent>( SourceComponent->Class, ComponentOuter, Name, ... );
```

Stock UE3 leaves `ComponentName` as `NAME_None` there, which is what this tree had, so
`DishonoredPlayerPawn.PowersComp` was `DishonoredPowersComponent_0` and
`DishonoredPlayerController.DisObjComp` was `DishonoredObjectivesComponent_0`. `5` is literally
`REN_ForceNoResetLoaders|REN_DoNotDirty` in this tree's own header.

### 1.3 Every instanced sub-object — `UObjectProperty::InstanceValue`, retail `0x95bb0`

This is the one that cost the most to find, because the obvious candidate is innocent.
`UObjectProperty::InstanceSubobjects` (retail `0x959d0`) is **identical** in this tree and in retail: both name
only when the owner is a template. The branch that matters is in `InstanceValue`, the `CopySingleValue` /
`CopyCompleteValue` half that `InitProperties` actually uses:

```
if ( !UObject::IsTemplate(DestOwnerObject, RF_ClassDefaultObject|RF_ArchetypeObject) )
{
    if ( SrcObject->Outer == DestOwnerObject ) -> NAME_None
    else                                      -> SrcObject->GetFName()
}
```

Stock UE3 has no such branch. That one `else` is what makes
`DishonoredGameInfo.pGlobalUIManager`, `.pNPCTravelManager`, `.pPpManager`, `.GlobalAIManager`,
`.GlobalFactionManager` and the other seven global managers the same name in every session — which is exactly
why a save can name them at all. Retail wrote this deliberately: the object dictionary is keyed on
`GetFName()` (`FLevelSaver::GetObjectIndex`, 2012 `0x656370`, writes `v2->Name`), so Arkane made every name on
that path deterministic. Three functions, one intent.

**This is the widest change in the package.** Every actor in the game now gets its components and instanced
sub-objects under their template names rather than `Class_0`. The regression's 31 checks are the evidence that
nothing depended on the old names.

### 1.4 The census, measured

`-disobjtree` (new, `dishonoredengine.cpp`) prints a class default object's sub-objects beside the live
object's, which is the comparison that answers "did instancing run, and under what name". Before and after,
from `build/agentEB/agentEB_final.log` and the run that preceded it:

| object | template sub-objects | instance, before | instance, after |
|---|---:|---|---|
| game info | 20 | `DisPostProcessManager_0`, `DisGlobalUIManager_0`, … (and the actor itself was `DishonoredGameInfo_0`) | `pPpManager`, `pGlobalUIManager`, `pNPCTravelManager`, `GlobalAIManager`, … |
| player controller | 4 | `DishonoredObjectivesComponent_0`, `CylinderComponent_0`, … | `DisObjComp`, `ThreatPerceptionComp`, `CollisionCylinder`, `pUseFSM` |
| player pawn | 24 | `DishonoredPowersComponent_0`, … | `PowersComp`, `pMesh`, `MyLightEnvironment`, … |

And the dictionary itself, `-disdictdebug=170`:

```
DisDict  12: class DishonoredGameInfo            name DishonoredGameInfo   outer   6 -> ...PersistentLevel.DishonoredGameInfo
DisDict  14: class DisGlobalFactionManager       name GlobalFactionManager outer  12 -> ...DishonoredGameInfo.GlobalFactionManager
DisDict 133: class DisNPCTravelManager           name pNPCTravelManager    outer  12 -> ...DishonoredGameInfo.pNPCTravelManager
DisDict 140: class DisGlobalUIManager            name pGlobalUIManager     outer  12 -> ...DishonoredGameInfo.pGlobalUIManager
DisDict 146: class DisPostProcessManager         name pPpManager           outer  12 -> ...DishonoredGameInfo.pPpManager
DisDict 150: class DishonoredObjectivesComponent name DisObjComp           outer 148 -> ...DishonoredPlayerController.DisObjComp
DisDict 164: class DishonoredPowersComponent     name PowersComp           outer 162 -> ...DishonoredPlayerPawn.PowersComp
```

All three of the brief's `NOT FOUND` records, and four more of agent ED's eight frontier classes, resolve.
**One of the eight still does not**: record 142, `pHUD`, outer 140 — `UDisGlobalUIManager::m_pHUD` is not a
component default and not an instanced sub-object; the movie player is constructed by script, and agent DC's
finding stands that `UDisGlobalUIManager`'s config set does not name it. That is a hand-over, not a defect of
this package.

## 2. The eight override bodies

Ported in `dissavegame.cpp`, each naming its retail address and the retail source file it belongs to. Agent ED
put `UDishonoredPowersComponent::GameLoad` there rather than in `dishonoredpowerscomponent.cpp`; these follow,
for the same reason — all eight of retail's own files are still `import_reference.py` stubs with no includes
and no bodies.

| class | retail `GameLoad` | what it reads |
|---|---|---|
| `ADishonoredGameInfo` | `0x5fa3f0` | `AActor::GameLoad`, the faction manager, then for a file state the live bend-time channels, the travel manager, the NPC id counter, the mission number, the chapter tag, two chapter targets, three managers, one flag |
| `ADishonoredSpawner` | `0x65ec30` | the spawned pawns, the pending-spawn ring, the retry state |
| `UDisNPCTravelManager` | `0x6e28a0` | the current level, the arrival spawners by level name, one `FDisNPCTravelInfo` per travelling NPC |
| `UDisGlobalUIManager` | `0x856f70` | the HUD and power wheel, the shown upgrades, `m_bFirstVisitToStore` |
| `UDisGFxMoviePlayerHUD` | `0x7ac2e0` | two packed bit pairs and the tutorial stack |
| `UDisGFxMoviePlayerPowerWheel` | `0x7b9e50` | four gamepad shortcuts, ten keyboard ones, the assign flags |
| `UDisPostProcessManager` | `0x7eb2c0` | 20 of 21 effect slots, thirteen colour floats, seven timers, `m_KismetPPParams` |
| `ADishonoredPlayerController` | `0x6a31e0` | `AActor::GameLoad`, two travel names, the objectives component, one bit, `m_InputEnableMask[7]`, then the HUD's show flags |

Helpers, also from retail: `operator<<` for `FDisTutorialInfo` (`0x7a5740`), `FArkUberPpParameters`
(`0x7e8220`), `FArkPpDofParameters` (`0x7e7fe0`), `FArkPpColorBalanceParameters` (`0x7e8060`),
`FArkPpHdrParameters` (`0x7e8170`) and `FDisMaterialIndexReplacement` (the element half of `0x6d3260`);
`FDisNPCTravelInfo::GameLoad` (`0x6df9f0`); `ADishonoredHUD::SerializeForGameLoad` (`0x5fa6f0`).

### 2.1 Three places where the 2012 build is not retail, and retail is what is ported

This matters because the 2012 decompiles are the readable ones and it would have been easy to port them.

1. **`ADishonoredGameInfo::GameLoad` has no difficulty byte in retail.** 2012 reads a `BYTE Difficulty`
   between the bend-time loop and the travel manager and dispatches `FArkGameEvent` type 9 when it changes;
   retail `0x5fa3f0` goes straight from the loop to `<< m_pNPCTravelManager`. One byte of stream, and it would
   have desynchronised everything after it.
2. **`ADishonoredPlayerController::GameLoad` reads two FNames in retail**, `m_PlayerTravelLocationName` @1700
   and `m_PlayerTravelOriginLevelName` @1708. 2012 reads only the first. Eight bytes.
3. **`FDisTutorialInfo` grew from 40 to 64 bytes**: retail's `m_MessageLocFile` / `m_MessageLocSection` /
   `m_MessageLocKey` replace 2012's single `m_Message`. The array stride in the retail body is `i << 6`.

Every bit position was resolved by name, not by index, against `resources/reference/DishonoredGameClasses.pdb.h`
(which gives the 2012 bit names) and `resources/docs/types/retail_sdk_layout.json` (which gives retail's
offsets and order), and the two agree on the *names* in all four bitfields the eight bodies touch:
`ADishonoredGameInfo::m_bIsInPlaytestMode` (@980 bit 1), `UDisGlobalUIManager::m_bFirstVisitToStore` (@772
bit 1), `ADishonoredPlayerController::m_bDisableStealthShroud` (@1552 bit 15) and, on the HUD (@508),
`m_bHasNewBoneCharm` / `m_bSystemicTutorialsEnabled` then
`m_bAdrenalineKill1TutoDisplayed` / `m_bAdrenalineKill2TutoDisplayed`.

### 2.2 The one retail address the diff tool has no match for, resolved by hand

`UDisPostProcessManager::GameSave`/`GameLoad` are unmatched in `match_2012_2013.csv`. The retail `GameLoad` is
**`0x7eb2c0`**, found by scanning retail's functions after the matched
`UDisPostProcessManager::TickPossession` (`0x7eae00`) and confirming its member offsets against
`retail_sdk_layout.json`: `m_RequiredEffects`@360 (INT[21]), `m_EffectStates`@444 (21 bytes),
`m_PCAntialiasingType`@465, `m_BendTimeIntensity`@468, the three `FLinearColor`s at 472/488/504, seven floats
at 520…544, `m_KismetPPParams`@548 — every one of the body's twenty-nine accesses lands on a named member, and
its one non-trivial callee is `operator<<(FArchive&, FArkUberPpParameters&)`.

It also shows retail's own oddity, kept: the loop serialises slots 0…19 of 21, then clears slot 18, and clears
19 and 20 again at the end. So a restored session never resumes the last three post-process effects.

### 2.3 Deviations, stated plainly

* `ADishonoredGameInfo::BendTime` (retail `0x5fa0d0`) does not exist in this tree, so each saved bend-time
  channel is **read** and not resumed. Marked `DISHONORED(bringup)`. No stream bytes are affected.
* `ADishonoredSpawner`'s two `FArkGameEventDispatcher` calls are not made, because
  `ADishonoredSpawner::OnOtherActorTerminatedEvent` does not exist here. Neither touches the stream.
* `FDisNoteParams` and its two subclasses do not exist here, so `operator<<(FDisTutorialInfo&)` reads the
  note's bytes **in place** — two `FString`s for note type 1, one `BYTE` for types 2 and 3, nothing otherwise,
  which is exactly what `FDisNoteParams::CreateNoteParams` (2012 `0x805080`) plus
  `FDisGenericNoteParams::Serialize` (2012 `0x66fb50`) and `FDisMapParams::Serialize` (2012 `0x669a20`) come
  to — and leaves `m_pNoteParams` NULL. The stream stays exact; the note is not reconstructible yet.
* Retail's `ADishonoredSpawner::GameLoad` re-reads every pending spawn into the **same** slot,
  `m_PendingSpawns[m_FirstPendingSpawn]`, because its loop never advances the pointer. Both builds do it and
  the byte count depends on it, so it is kept.
* The four Ark post-process `operator<<` are inline in retail's `engineclasses.h`; here they are declared
  there and defined in `UnPlayer.cpp`, which already holds this tree's other `FArkUberPpParameters` helpers,
  because an inline definition in a header that widely included is the 77 MB link problem in
  `STATUS.md`'s pitfalls.

## 3. What actually stopped the restore, and it was the world

With the trees built and the eight bodies in, the stream reached dictionary record **21** and stopped there
with "resolved to no live object". Record 20 is
`Twk_Pawn_LadyEmily.Pwn_LadyEmily_TowerEmpress`, and record 21 is the actor the loader spawns from it.

Porting retail's per-record seek-free retry (agent ED's `DISHONORED(bringup)` note inside
`FLevelLoader::FLevelLoader`, retail `0x613070`) made that visible rather than fixing it:

```
Warning, Failed to load 'Twk_Pawn_LadyEmily_SF': Can't find file 'Twk_Pawn_LadyEmily_SF'
Warning, Failed to load 'NPC_Emily_SF': Can't find file 'NPC_Emily_SF'
Warning, Failed to load 'tower_FX_SF': Can't find file 'tower_FX_SF'
```

None of those packages has a file. In the cooked PC build their contents live **inside the mission's
sub-levels** as forced exports — and the harness had restored into the front end's world, which has three
streaming levels and no mission at all. **9,001 of 11,321 dictionary records resolved to nothing because the
objects they name were not in memory.**

The save says which world it wants. `FGameStateData::m_SubLevels`' first entry carries flags 7 and is the
mission's streaming-persistent sub-level — `l_tower_p` for `Dishonored0.sav`, `L_Prison_P` for
`DisMission0.sav`, `L_Isl_LowChaos_P` for `DisMission8.sav`; every other entry is 0 or 6, so bit 0 is the
"this streaming object is a `ULevelStreamingPersistent`" flag agent ED identified on the saving side.

So the restore harness is now staged as retail's `UDishonoredEngine::ProcessSaveLoadCmd` (2013 `0x6162d0`,
still not ported) is staged — read the file, bring the save's own world up, then restore:

```
DisRestore: armed slot 16; the startup world is up after 85 world tick(s) (3 streaming level(s), settled 1) - reading the save
DisRestore: the save was taken in 'l_tower_p'; this world has committed 'Dishonored_MainMenu' - streaming it in
DisRestore: the world is up after 86 world tick(s) (9 streaming level(s), settled 1, committed 'l_tower_p', wanted 'l_tower_p') - restoring
```

The travel uses the engine's own `STREAMMAP` — `UGameEngine::Exec`, retail `0x23d020`, the same entry the
retail main menu's Kismet reaches — so it is not a new path, only a newly *aimed* one.

## 4. The measurement

`build/agentEB_release`, `-disrestoreslot=16`, `Dishonored0.sav` ("0 - Dunwall Tower"). The final column is
from the **null RHI with no `-noscenerender`** (`build/agentEB/agentEB_final.log`); d3d9 with and without the
scene renderer gives byte-identical numbers (`agentEB_t6.log`, `agentEB_fog.log`), which is section 5.2's
point. The earlier columns are from the same command against HEAD and against the two intermediate states.

| | HEAD `94889d4` | + the three names | + the eight bodies | + the travel |
|---|---:|---:|---:|---:|
| dictionary records read | 11321/11324 | 11321/11324 | 11321/11324 | **11321/11324** |
| dictionary bytes | 91844/91844 | 91844/91844 | 91844/91844 | **91844/91844** |
| records resolved | 2259 | 2285 | 2285 | **6227** |
| records not found | 9027 | 9001 | 9001 | **5093** |
| actors spawned from tweaks | 24 | 24 | 24 | **50** |
| objects restored | 2 | 2 | 4 | **4** |
| object-stream bytes | 28/619631 | 28/619631 | 74/619631 | **74/619631** |
| ported override classes | 28 | 28 | 36 | **36** |
| stopped on | record 12, no live object | `ADishonoredGameInfo`, unported | record 21, no live object | `DishonoredNPCPawn_0`, unported |

The census line in full:

```
DisSaveLoad census [restore]: 1 level(s); dictionary 11321/11324 objects (6227 resolved) 91844/91844 bytes;
data 4 restored, 1 skipped, 74/619631 bytes; 50 spawned, 5093 not found, 1 unported, 0 unresolved,
0 untrusted skips, 0 partial bodies, 4 PostGameLoad; STREAM ABORTED
```

**Accept 3, honestly: the player is still not placed.** `DisRestore: player pawn DishonoredPlayerPawn at
X=0.000 Y=500.000 Z=-209.350 rotation P=0 Y=0 R=0` is the spawn transform, unchanged. The stream does not
reach record 162, and section 6 says exactly what it would take.

## 5. The two blockers

### 5.1 The fog shader assert — fixed, and it was reference code

Agent ED's stack: `checkSlow(Parameter.IsInitialized())`, `ShaderManager.h:379`, from
`FHeightFogShaderParameters::Set`, `FogRendering.cpp:97`, from
`TBasePassVertexShader<FNoLightMapPolicy,FNoDensityPolicy>::SetParameters`.

`FogRendering.cpp:97` is the *else* branch — the one that writes the values that disable vertex fog — and the
parameter is `bUseExponentialHeightFogParameter`. The cause is not in the fog code:
`TBasePassVertexShader` **binds and sets** an `FHeightFogShaderParameters` that its own `Serialize` never
writes or reads. `FShaderParameter::Bind` is the only thing that sets `bInitialized`, and a shader that came
out of the cooked cache — which is every shader in a cooked run — is built through the serialising
constructor and never calls `Bind`. So the whole block was uninitialised.

Retail does not have the member. The block comment above this tree's own `Serialize` already said so:
"Retail `TBasePassVertexShader<LightMapPolicy>` has one template argument and no height fog / fog volume
parameters (2012 PDB: 196 bytes …)" — retail `0x424c10` serialises `FShader`, the light-map policy's vertex
parameters, the vertex factory parameters and the material parameters, and nothing else. The height fog
reaches the frame through the base pass **pixel** shader.

So this is the **seventeenth** instance of the package's defect class: reference code for a member retail's
class does not have, kept and still called. The fix is a three-line deletion — the `Bind`, the `Set` and the
member.

Verified in the build class where the assert exists, because `checkSlow` is `DO_GUARD_SLOW` and therefore
Debug-only: `build/agentEB_debug`, `--rhi d3d9 -windowed 640x480`, `DishonoredGameFull_P`, **195 scene
censuses, no assert** (`agentEB_dbgfog.log`). Agent ED's Debug run stopped at `[0007.56]` on the first frame.

**This also means the regression's d3d9 stage was never going to see it**, twice over: it opens `L_Tower_P`,
and it is a Release build.

### 5.2 `UWorld::Tick` running twice — does not reproduce, and was a mis-attribution

Agent ED read `-disrestoredelay=2` firing and `=3` never firing as "`UWorld::Tick` runs exactly twice on the
startup map under the null RHI". The alternative reading is that its arm was satisfied on the second call,
because the front end's three streaming levels are visible from the first frame — its condition was
`all streaming levels visible OR the delay expired`, and the first disjunct is true immediately.

The hook now counts its own calls, so this is a number rather than a reading. Under the null RHI on
`DishonoredGameFull_P`:

```
DisRestore: armed slot 16; the startup world is up after 85 world tick(s) (3 streaming level(s), settled 1) - reading the save
```

**85 ticks** (86 in the run before it - the front end's own streaming settles a frame earlier or later), and then the `STREAMMAP` travel completes and nine streaming levels come up — under the null
RHI, with no scene renderer at all. So sub-level streaming does finish on the startup map; `-noscenerender` is
no longer needed for the restore, and neither is d3d9. Both RHIs now give the same census.

## 6. The frontier now, with addresses

The stream stops on `DishonoredGameFull_P.TheWorld:PersistentLevel.DishonoredNPCPawn_0` — Lady Emily, spawned
by the loader from her tweaks at dictionary record 21. That is **before** the player pawn at record 162, so
agent ED's frontier table is superseded: it was computed in a world where the NPC pawns did not resolve.

| class | retail `GameLoad` | 2012 | size 2012 / 2013 | note |
|---|---|---|---|---|
| `ADishonoredNPCPawn` | **`0x76d3f0`** | `0x7c7c40` | 1013 / 1488 | where the stream stops now |
| `ADishonoredPawn` | **`0x75c0a0`** | `0x79b360` | 964 / 1928 | ported only as far as `AActor::GameLoad`, which then stops the stream deliberately |
| `ADishonoredNPCController` | `0x763570` (likely) | `0x7c91c0` | 102 / 124 | candidate, not confirmed |

Neither pawn address is in `match_2012_2013.csv`. Both were resolved by listing retail's callers of
`AActor::GameLoad` (retail `0x18ad70`, from the match table) and following the call graph: only two retail
functions call `sub_B5C0A0` (`0x75c0a0`), and one of them is `0x6b8b60`, which agent ED had already identified
as `ADishonoredPlayerPawn::GameLoad`. The other, `0x76d3f0`, is therefore the NPC pawn's — nothing else
derives from `ADishonoredPawn`. `build/agentEB/callers.py` reproduces it.

**`ADishonoredPawn::GameLoad` is the single highest-value body left**: it is on the path of every NPC pawn
*and* of the player pawn, and the player's transform comes out of the `AActor::GameLoad` it already calls
first. Porting it plus `ADishonoredNPCPawn::GameLoad` is about 3.4 KB of retail x86 — a package, not a
follow-up.

## 7. Verification

* **Accept 1** — the three trees resolve: `agentEB_final.log`, section 1.4.
* **Accept 2** — the stream advances: 28 → 74 bytes, 2 → 4 objects, and the dictionary from 2,259 to 6,227
  resolved records; the new stopping point is named in section 6.
* **Accept 3** — not reached; the player's transform is unchanged, stated in section 4.
* **Accept 4** — blocker 1 fixed and verified in a Debug build; blocker 2 disproved with a counter.
* **Accept 5** — `python resources/tools/run_regression.py --build-dir build/agentEB_release --no-build
  --exe-name DishonoredGame_EB.exe --log-prefix agentEB_reg`: **31 ok, 0 failed, 0 skipped, 423 s**
  (`build/agentEB_release/regression/summary.txt`). Clean full release build of `DishonoredGame`, `CoreSmoke`
  and `LayoutProbe` from an empty `build/agentEB_clean`, 0 errors; Debug build of all three, 0 errors.
* **Layouts** — unchanged, and the regression's own layout stage says so: `layout_types 2314`,
  `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`, `verify_phase2 2/2`. This package adds no
  reflected member.
* **Addresses** — `python resources/tools/rva_sweep.py --csv build/agentEB/rva_sweep.csv`: of the 395
  citations in the files this package touched, **0 mislabelled and 0 unknown**. The five `MISLABELLED-2012`
  the whole-tree sweep reports are all pre-existing and in other agents' files
  (`CppText/UDisAISubState.h:16`, `External/GFx3/GFxTextDocView.h:238`, `External/GFx3/GFxTextField.cpp:39`,
  `GFxUI/Src/gfxuirenderer.cpp:1669` twice) — a hand-over to whoever owns them.
* **The dictionary halves are still exact**: 11321/11324 records and 91844/91844 bytes, in every run.
* Everything was built from `build/agentEB_wt`, detached at `94889d4`, with only this package's own files
  copied in (`build/agentEB_sync.py`).

## 8. Hand-overs

1. **Coordinator, at merge.** This package changes four files outside `DishonoredGame` and all four are
   load-bearing: `Core/Src/UnProp.cpp` and `Core/Src/UnCoreNative.cpp` (the two instancing naming branches —
   these change the name of every instanced component and sub-object in the tree),
   `Engine/Src/UnWorld.cpp` (the game info actor's name) and `Engine/Src/BasePassRendering.h` (the fog
   deletion). `Engine/Inc/EngineClasses.h` and `Engine/Src/UnPlayer.cpp` gain the four Ark post-process
   serializers. **Regeneration is required**: this package adds `Inc/CppText` hooks for
   `UDisNPCTravelManager`, `UDisGlobalUIManager`, `UDisGFxMoviePlayerHUD`, `UDisGFxMoviePlayerPowerWheel` and
   `UDisPostProcessManager` and appends to `ADishonoredGameInfo`, `ADishonoredSpawner`,
   `ADishonoredPlayerController` and `ADishonoredHUD`, so
   `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`
   must run at merge — **agent EB did not regenerate.** The four new `#include "CppText/<Class>.h"` lines were
   inserted by hand at the place the generator emits them (last line of the class body). `Sources.cmake` does
   not change: no new source file. `dissaveload_classlists.h` is regenerated by
   `build/agentEB/gen_classlists.py` (agent ED's, with eight names added to `PORTED` and `PORTED_ISSAVEABLE`);
   it now reads 562 entry classes, 36 ported, 76 unported.
   The twenty files, for the merge - sixteen modified, four new and therefore untracked:
   `Core/Src/UnProp.cpp`, `Core/Src/UnCoreNative.cpp`, `Engine/Inc/EngineClasses.h`,
   `Engine/Src/UnWorld.cpp`, `Engine/Src/UnPlayer.cpp`, `Engine/Src/BasePassRendering.h`,
   `DishonoredGame/Src/dissavegame.cpp`, `DishonoredGame/Src/dishonoredengine.cpp`,
   `DishonoredGame/Inc/dissaveload_classlists.h`, `DishonoredGame/Inc/dishonoredgameclasses.h`,
   `DishonoredGame/Inc/DishonoredGameUIClasses.h`, `Inc/CppText/{ADishonoredGameInfo,ADishonoredSpawner,
   ADishonoredPlayerController,ADishonoredHUD,UDisPostProcessManager}.h` and the four new
   `Inc/CppText/{UDisNPCTravelManager,UDisGlobalUIManager,UDisGFxMoviePlayerHUD,
   UDisGFxMoviePlayerPowerWheel}.h`. `build/agentEB_sync.py` is the authoritative list.

2. **The next package is `ADishonoredPawn::GameLoad` (retail `0x75c0a0`) and
   `ADishonoredNPCPawn::GameLoad` (`0x76d3f0`).** Section 6. Those two bodies stand between this tree and
   milestone 7, and the second of them is reached at dictionary record 21 of 162, so there is no way round.
   The loop is unchanged: add a body, add the name to `build/agentEB/gen_classlists.py`'s `PORTED`, rerun it,
   rerun `-disrestoreslot=16`, read `data N restored, X/619631 bytes`.
3. **Whoever owns `UDisGlobalUIManager`'s movie players (agent EA's area).** Record 142, `pHUD`, is the one
   frontier object this package did not make exist. `m_pHUD` is neither a component default nor an instanced
   sub-object, so no naming fix reaches it: the movie player is constructed by script that does not run,
   which is agent DC's finding about the config set. `UDisGFxMoviePlayerHUD::GameLoad` (`0x7ac2e0`) and
   `UDisGFxMoviePlayerPowerWheel::GameLoad` (`0x7b9e50`) are ported here and waiting for it.
4. **Whoever ports `ProcessSaveLoadCmd` (retail `0x6162d0`).** Section 3 has its three stages and the
   sub-level flag that drives the travel one; `DisSaveLoadRestoreTick` in `dishonoredengine.cpp` is a
   three-state stand-in for it and should be deleted when the real thing lands. `-disrestoredelay=<n>` is now
   the per-stage patience, not the total.
5. **`-noscenerender` is no longer needed for the restore, and neither is d3d9.** Both RHIs give the same
   census; the null RHI streams the mission in. Section 5.2.
6. **Agent DP's deletion backlog (package EF).** One more entry, and it is the top of its class:
   `TBasePassVertexShader::HeightFogParameters` was a member retail's class does not have, still bound and
   still set, and it cost a whole map's Debug render. The audit's ranking should weight "still called" above
   "merely declared".
7. **One inference to check when someone has better evidence.** `m_SubLevels`' flag bit 0 is read here as
   "this streaming object is a `ULevelStreamingPersistent`". It is consistent across all 51 saves (exactly one
   entry per save has it, and it is always the mission's `_P`), and it agrees with agent ED's reading of
   `bSaveWorldInfo` on the saving side, but the writer's own bit assignment was not decompiled. If it is
   wrong the travel stage aims at the wrong level, which is loud rather than silent.
8. **`ADishonoredNPCController::GameLoad`'s retail address is a candidate, not a fact.** `0x763570` is the
   right size in the right place and calls `AActor::GameLoad`, but it was not confirmed. 2012 `0x7c91c0` is.
