# Agent AE — Engine/GameFramework retail natives without a C++ body (2026-09-26)

Package **AE** of `resources/docs/PHASE6.md` (wave 4), steps 1–3. Every number below comes from the build
`build\agentAE` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`, the four module options ON) of the HEAD snapshot
`build/agentAE_wt` (HEAD 571bd0e + my files + agent AD's nav-mesh fix); my edits themselves live in the **shared
working tree**.
"2013 rva" = retail `Dishonored_Latest2026` exe (named db, private copy `resources/docs/idb/retail2013_agentAE.i64`);
"2012 rva" = the 2012 Shipping build (readability only, `shipping2012_agentAE.i64`). Decompiles:
`resources/reference/decomp/agentAE/{2013,2013b,2013c,2013d,2013e,2012}` (git-ignored, 250 functions).
Status rows: `resources/docs/agents/agentAE_status.csv` (107 rows). No commits, no `git add`.

## Result

| Measure | Before | After |
|---|---|---|
| Retail Core/Engine/GameFramework script natives (`iNative == 0`) with **no C++ exec** | **145** | **36** |
| of those, native *interface* functions that legitimately bind to `UObject::ProcessInternal` (`UFunction::Bind`, `CLASS_Interface`) | 7 | 7 |
| real gaps | **138** | **29** |
| `MAP_NATIVE` entries added (each with a `DECLARE_FUNCTION` and a body) | — | **109** |

109 natives got a retail body (accept threshold: ≥ 70). Re-run at hand-off the count is **32 missing / 25 real** because agent AI's package landed four more `UEngine`/`UGameEngine` execs in the meantime; the 36/29 column is my own delta. The inventory is reproducible:
`python build/agentAE_work/inventory.py` walks `types/script_classes_2013.json`, keeps every `Native` function
with `native == 0` of Core/Engine/GameFramework, and reports the ones without a `MAP_NATIVE` registration in the
tree (`build/agentAE_work/inventory.txt` before, `inventory_after.txt` after). It replaces agent Z's in-engine
diagnostic and needs no run; Z's 147-entry list and mine agree except for Z's two duplicates and the 2013-only
`DecalManager.CanSpawnDecals` / `Pawn.UpdateAnimSetList`.

## Builds and runs

The wave-4 agents' in-flight edits (a half-written `EngineArkaneClasses.h`, agent AI's new `UEngine` virtuals in
`EngineGameEngineClasses.h`) broke the shared tree mid-wave, so the accept build is a HEAD snapshot:
`python resources/tools/make_snapshot.py AE <my 30 files> ` → the detached worktree `build/agentAE_wt`
(HEAD 571bd0e + agent AD's `UnPath.h` / `UnNavigationMesh.cpp` + my files) plus the generated
`build\agentAE_wt_build.cmd`, which configures and builds it into `build\agentAE`. The 17 shared generated headers
were restored from HEAD inside the snapshot (`git -C build/agentAE_wt checkout --`) and my lines re-applied with
`python build/agentAE_work/apply_ae.py --root build/agentAE_wt`, so the snapshot is exactly HEAD + my edits.
`build/agentAE_work/cmp_ae.py` confirms the shared working tree carries every one of those lines (0 snapshot-only
lines in all 30 files) — the coordinator merges the shared tree, not the snapshot.

| # | command | result |
|---|---|---|
| build5 | `build\agentAE_wt_build.cmd DishonoredGame` (log `build/agentAE_build5.log`) | EXIT=2, 2 errors: `UnCamera.cpp(1665) C3861 'GetCameraViewPoint'` (mine) and `UnEngine.cpp(1333) C2084 UEngine::PlayLoadMapMovie already has a body` (AI's in-flight header dragged in by the overlay) |
| build6 | same, after replacing the `GetCameraViewPoint` call with `CameraCache.POV` and restoring HEAD's headers (log `build/agentAE_build6.log`) | EXIT=2, 12 errors, all one cause: `apply_ae.py`'s `UAnimNodeSlot` anchor `DECLARE_FUNCTION(execPlayCustomAnim)` first matches `UAnimNodePlayCustomAnim`, so `AddToSynchGroup` landed in the wrong class |
| build7 | same, after re-anchoring on `void TickChildWeights(FLOAT DeltaSeconds);` (unique to `UAnimNodeSlot`, `build/agentAE_work/fix_slot.py`, log `build/agentAE_build7.log`) | **EXIT=0**, clean `DishonoredGame.exe` link (110/110) |
| CoreSmoke | `build\agentAE_wt_build.cmd CoreSmoke` then `build/agentAE/Binaries/Win32/CoreSmoke.exe` | **99 passed, 0 failed, 0 skipped**, RC=0 |
| LayoutProbe | `build\agentAE_wt_build.cmd LayoutProbe`, `LayoutProbe.exe > build/agentAE/layout_probe.txt`, then `python resources/tools/sdk/xcheck_sdk_layout.py build/agentAE/layout_probe.txt --header EngineCameraClasses.h … --header CoreClasses.h` (14 headers) | `0 differ from retail`, RC=0 (expected: my edits add no members — only `DECLARE_FUNCTION`, `MAP_NATIVE` and method declarations; the shared `retail_sdk_delta.md` was not rewritten) |

### Accept run (null RHI, retail content)

```
python resources/tools/build_and_smoke.py --build-dir build/agentAE --no-build --exe-name DishonoredGame_AE.exe \
  --log-name agentAE.log --ini-dir build/agentAE/config --rhi null --timeout 300 --skip-native OnlineSubsystemPC \
  --milestone "Committed map change via DishonoredEngine" --expect "Initial startup" \
  --expect "Committed map change via DishonoredEngine" --forbid "Engine native not ported" \
  --forbid "GameFramework native not ported" "--extra-args=-forcelogflush"
#   -> expect ok: Initial startup
#      expect ok: Committed map change via DishonoredEngine
#      expect ok: no Engine native not ported
#      expect ok: no GameFramework native not ported
#      RC=0
```

`-forcelogflush` is required (agent AD's accept line uses it too): without it the log stops mid-line at ~24.7 s and
the map-change line never reaches the file, which is what made two earlier runs report `expect MISSING`.
The run reaches `Committed map change via DishonoredEngine Transient.DishonoredEngine_0` with
`Dishonored_MainMenu_Env` / `_FX` added to the world, and **no Engine or GameFramework `native not ported` line
appears anywhere in `agentAE.log`** — the warn-once lines Z listed (`Camera:UpdateCamera`, `HUD:DisplayConsoleMessages`,
the DLC trio, `Pawn:Died`, `Camera:ClearCameraLensEffects`, `InterpActor:SetShadowParentOnAllAttachedComponents`)
are all gone. This is the criterion of the package that is mine.

### `-strictnatives`

`-strictnatives` turns *every* unbound native in *every* module into `appErrorf`, so it aborts on the first gap of
any package, mine or not:

| run | extra args / skips | outcome |
|---|---|---|
| A | `--skip-native OnlineSubsystemPC`, `-strictnatives -forcelogflush` | RC=1 at `[0003.28]`, **before** `Initial startup`: `appError called: OnlineSubsystemSteamworks native not ported: UOnlineSubsystemSteamworks::execReadFriendsList` (agent X's package; `--skip-native OnlineSubsystemPC` does not cover Steamworks) |
| B | `--skip-native OnlineSubsystemPC,OnlineSubsystemSteamworks`, `-strictnatives -forcelogflush` | `expect ok: Initial startup`, then RC=1 at `[0004.01]`: `appError called: DishonoredGame native not ported: ADishonoredPawn::execChooseAndTriggerDeathEvent_Native` (DishonoredGame's package — see hand-over 7) |
| C | `--skip-native OnlineSubsystemPC`, no `-strictnatives`, `--forbid` on both my modules | **RC=0**, all four checks (the accept run above) |

So the `-strictnatives` half of the accept line cannot pass until the OSS and DishonoredGame packages close their own
gaps; the Engine/GameFramework half is proven by run C's two `--forbid` checks, which are exactly what
`-strictnatives` would abort on for my modules. Both outcomes are recorded here rather than papered over.

## Ported natives (2013 rvas; `exec` = the script entry point, `body` = the C++ function it calls)

Exec rvas confirmed in the named retail db (`resources/docs/symbols/functions_2013.csv`), not taken from
`match_2012_2013.csv`; where the 2013 exec is an unnamed folded stub (identical 53-byte bodies are shared by
several classes) the note says so and the body rva carries the evidence. Bodies were decompiled from the 2013
db and the 2012 decompile used only as the readable copy.

### Camera (`Engine/Src/UnCamera.cpp`, `EngineCameraClasses.h`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `Camera.UpdateCamera` | 0x1cffd0 | 0x1e5eb0 | ported | Camera.uc `UpdateCamera`/`DoUpdateCamera` in C++; **Arkane addition**: the FOV is widened to keep the horizontal field of view of the closest of 16:9 / 16:10 / 4:3 on wider screens; no reference audio fade |
| `Camera.FindCameraLensEffect` | 0x1d0250 | 0x1e5c60 | ported | `EmittersToTreatAsSame` both ways, skips `bDeleteMe`/pending-kill |
| `Camera.AddCameraLensEffect` | 0x1d02c0 | 0x1e5d90 | ported | **returns** the effect in retail (the reference returns nothing) |
| `Camera.RemoveCameraLensEffect` | 0x1d0360 | 0x1f2d40 | ported | |
| `Camera.ClearCameraLensEffects` | 0x1d03c0 | 0x1f2d60 | ported | unregisters the camera before destroying, then empties the array |
| — `ACamera::UpdateViewTarget` | (C++) | 0x1e6350 | ported | CameraActor FOV/aspect/`m_CamOverridePostProcess` copied straight; `CalcCamera` raised as a script event; FirstPerson/ThirdPerson/FreeCam/Fixed styles; no reference mesh-translation offsets |
| — `ACamera::ProcessViewRotation` | (C++) | 0x1e5ac0 | ported | modifier loop over `ModifierList` |
| — `ACamera::FillCameraCache` | (C++) | 0x1bd0a0 | ported | |
| — `ACamera::BlendViewTargets` | (C++) | 0x1dd9c0 | ported | |

### PlayerController / Pawn movement (`UnController.cpp`, `UnPawn.cpp`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `PlayerController.PlayerMove_Walking` | 0x1d3a00 | 0x12fdb0 | ported | state `PlayerWalking`'s `PlayerMove`+`ProcessMove`; ends in the virtual `CheckJumpOrDuck` (vtable +1240, empty in Engine, `ADishonoredPlayerController::CheckJumpOrDuck` 0x6ae490 is AF's) |
| `PlayerController.ProcessViewRotation` | 0x1d3840 | 0x133010 | ported | camera modifiers, then the pawn; pawnless path clamps pitch to ±16383 |
| `PlayerController.UpdateRotation` | 0x1d37e0 | 0x1292d0 | ported | `SetDesiredRotation` on the pawn, `PlayerInput->aTurn/aLookUp`, `FaceRotation` |
| `PlayerController.HandleWalking` | 0x1d35c0 | 0x243000 | ported | `bRun` drives `Pawn->bIsWalking` directly (no `SetWalking` event in retail) |
| `PlayerController.LimitViewRotation` | 0x1d3930 | 0x129410 | ported | |
| `PlayerController.IsLookInputIgnored` | 0x1d3ab0 | 0x243050 | ported | |
| `PlayerController.IsMoveInputIgnored` | 0x1d3a70 | 0x243040 | ported | |
| `PlayerController.CleanOutSavedMoves` | 0x1d3310 | 0x129450 | ported | |
| `PlayerController.ResetTimeMargin` | 0x1d32e0 | 0x130160 | ported | |
| `PlayerController.CleanUpBeforeLevelTransition` | 0x1d3af0 | 0x1ca1f0 | ported | the whole body is `GWorld->CleanUpBeforeLevelTransition()` |
| `PlayerController.Sentinel_TakeScreenshotEnabled` | 0x1d3d30 | 0x2cc170 | ported | `-nosentinelscreenshots` |
| `PlayerController.Sentinel_TakeScreenshot` | 0x1ee6a0 | 0x2e0780 | bringup | retail queues `<name>#<changelist>.png` through Sentinel globals we do not have; logs the request |
| `PlayerController.SetControllerTiltDesiredIfAvailable` | 0x1d3380 | 0x1cb0c0 | ported | empty in retail |
| `Pawn.TakeDamage` | 0x1c1640 (AActor's) | 0x183700 | ported | no `Role` check, no vehicle, C++ `GameInfo::ReduceDamage`, `NotifyTakeHit` event, inline pain-time stamp |
| `Pawn.Died` | 0x1dafd0 | 0x17f600 | ported | `bPlayedDeath` bail; `PreventDeath` / `ChooseAndTriggerDeathEvent` / `NotifyKilled` / `PlayDying` through `FindFunction`+`ProcessEvent` (no reference C++ wrappers); no vehicle/weapon/inventory |
| `Pawn.FaceRotation` | 0x1daf30 | 0x2a1460 | written | retail first asks a script event (name index 0x1447934) whether the rotation is locked; that name is not resolved, so the Pawn.uc body runs unconditionally |
| `Pawn.AddVelocity` | 0x1dade0 | 0x16e070 | ported | |
| `Pawn.HandleMomentum` | 0x1dac90 | 0x1754b0 | ported | |
| `Pawn.PlayHit` | 0x1db0a0 | 0x1694e0 | ported | retail keeps only the `LastPainTime` stamp |
| `Pawn.IsRagdoll` | 0x1daa80 | 0x382d00 | ported | `PHYS_RigidBody && CollisionComponent == Mesh` |
| `Pawn.GetNavigationHandle` | 0x1da990 | 0x1ca5d0 | ported | |
| `Pawn.InitNavigationHandle` | 0x5f27b0 | 0x1e09f0 | ported | |
| `Pawn.IsValidTargetFor` | 0x1d4c40 | 0x233610 | ported | `FALSE` in Shipping |
| — `APawn::ProcessViewRotation` | (C++) | 0x12ffc0 | ported | pitch limit inlined for player-controlled pawns |

### Actor / HUD / GameInfo / WorldInfo / InterpActor (`UnActor.cpp`, `EngineClasses.h`, `EngineGameEngineClasses.h`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `Actor.TakeDamage` | 0x1c1640 | 0x1753f0 | ported | `SeqEvent_TakeDamage::HandleDamage` over `GeneratedEvents`; the Pawn/KActor/KAsset/SkeletalMeshActor/GameCrowdAgent execs all forward to this one exec |
| `Actor.CheckHitInfo` | 0x1c1800 | 0x16dd40 | ported | 128-unit component trace (retail `AActor::TraceComponent` 0x2ccfd0 = the component's `LineCheck` + hit info) |
| `Actor.FindEventsOfClass` | 0x1fcc40 | 0x175310 | ported | |
| `Actor.VolumeBasedDestroy` | 0x1c03a0 | 0x168420 | ported | `UWorld::DestroyActor` (0x252cf0) |
| `Actor.DoKismetAttachment` | 0x1df6c0 | 0x18ab50 | bringup | parameters consumed only: the 36-byte `FAttachmentInfos` still lives in `DishonoredGameEngineShims.h` (hand-over 1) |
| `Actor.PostAkEvent` | 0x1c1050 | 0x2cd730 | bringup | retail forwards to `UAkAudioDevice::Get()`; our AkAudio module is the silent stub with no device → no-op |
| `Actor.SetRTPCValue` | 0x1c10b0 | 0x2d8040 | bringup | same |
| `Actor.SetSwitch` | 0x1c11e0 | 0x2d81f0 | bringup | same |
| `Actor.SetState` | 0x1c1150 | 0x2d8100 | bringup | same |
| `Actor.PostTrigger` | 0x1c1270 | 0x2d8300 | bringup | same |
| `Actor.ActivateOcclusion` | 0x1c12e0 | 0x2cd770 | bringup | same |
| `Actor.PlayActorFaceFXAnim` | 0x1e76c0 | vtable +340 = 0x169520 | ported | Actor's returns 0; `APawn`'s override (0x175520, `Mesh->PlayFaceFXAnim`) is left to the FaceFX port |
| `Actor.StopActorFaceFXAnim` | 0x1c1ac0 | vtable +344 = 0x59d10 | ported | empty; `APawn`'s override 0x169530 likewise |
| `InterpActor.SetShadowParentOnAllAttachedComponents` | 0x1c26e0 | 0x174cd0 | ported | the no-argument InterpActor variant (own StaticMeshComponent / LightEnvironment), no nested-attachment stack |
| `HUD.DisplayConsoleMessages` | 0x5f25d0 | 0x2f21e0 | ported | Shipping only empties `ConsoleMessages` |
| `HUD.ShouldDisplayDebug` | 0x1c3c50 | vtable +936 = 0x9c4b50 | ported | `return 0` in Shipping |
| `HUD.ShowDebug` | 0x1c3bd0 | vtable +932 = 0x128ad0 | ported | empty in Shipping |
| `GameInfo.ReduceDamage` | 0x1c6170 | 0x2cd830 | ported | neutral zone or god mode zeroes the damage |
| `WorldInfo.GetGlobalGravityZ` | 0x1c69a0 | 0x2a1650 | ported | caches `WorldGravityZ` from `GlobalGravityZ`/`DefaultGravityZ` |

### DLC, settings, storage, sequence, cheats (`DownloadableContent.cpp`, `USettings.cpp`, `UOnlinePlayerStorage.cpp`, `UnSequence.cpp`, `UnController.cpp`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `DownloadableContentManager.BackupDLCList` | 0x1d4070 | 0xf85b0 | ported | `InstalledDLCBackup = InstalledDLC` |
| `DownloadableContentManager.RemoveUnavailableDLC` | 0x5efc40 | 0xf5bf0 | ported | every backup name uninstalled through the engine's enumerator, then the backup emptied |
| `DownloadableContentManager.UninstallDLC` | 0x1fe610 | 0x10c3a0 | written | list bookkeeping; the "is active" query (`AGameInfo` vtable +1012) and the package / non-package unload (0xf5d80 / 0x10bfd0) are follow-ups |
| `DownloadableContentManager.UninstallDLCs` | 0x205aa0 | 0xfdbf0 | ported | |
| `DownloadableContentEnumerator.UninstallDLC` | 0x1eea40 | 0xfdaf0 | ported | |
| `DownloadableContentEnumerator.CleanLaunchedDLC` | 0x1d3fd0 | vtable +312 = 0x1cb0c0 | ported | empty in retail |
| `Settings.GetSettingsDataString` | 0x1e8e40 | 0x4f4730 | ported | |
| `Settings.SetSettingsDataString` | 0x1e8d30 | 0x4f1620 | ported | |
| `OnlinePlayerStorage.GetRangedProfileSettingValueFloat` | 0x1cd110 | 0x4f77e0 | ported | |
| `OnlinePlayerStorage.GetRangedProfileSettingValueInt` | 0x1cd210 | 0x4f7860 | ported | |
| `OnlinePlayerStorage.SetRangedProfileSettingValueFloat` | 0x1ccec0 | 0x4f74a0 | ported | clamps to the mapping range, truncates when it formats as int |
| `OnlinePlayerStorage.SetRangedProfileSettingValueInt` | 0x1ccf50 | 0x4f75d0 | ported | clamps, rounds to the nearest half |
| `SeqEvent_TakeDamage.HandleDamage` | 0x1d9e60 | 0x2efaf0 | ported | "Damage Taken" float variables get the accumulated damage |
| `SeqEvent_TakeDamage.IsValidDamageType` | 0x1d9e00 | 0x2dd410 | ported | |
| `CheatManager.SetTargetedActor` | 0x1c9c60 | folded stub | written | stores `m_pTargetedActor` (retail SDK @88) |
| `CheatManager.ShowActor` | 0x1dffa0 | — | ported | reads the name, does nothing in Shipping |
| `LocalPlayer.ZeroOverridePPDeltaSettings` | — | — | bringup | no exec of this name in either db; no-op |

### Anim / physics / decals / particles (`UnPhysic.cpp`, `DecalManager.cpp`, `EngineAnimClasses.h`, `EnginePhysicsClasses.h`, `PrimitiveComponent.h`, `UnSkeletalMesh.h`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `AnimNodeSlot.AddToSynchGroup` | 0x1dc970 | 0x1af140 | ported | finds the tree's `AnimNodeSynch` once (`SynchNode`, SDK @256) |
| `SkeletalMeshActorMAT.ClearAnimNodes` | 0x1db540 | 0x321f70 | ported | `SlotNodes.Empty()` |
| `SkeletalMeshActorMAT.UpdateAnimSetList` | 2012 0x1f0990 | (virtual) | ported | calls the existing `ASkeletalMeshActor::UpdateAnimSetList` |
| `SkeletalMeshActor.TakeDamage` | 0x1c1640 (AActor's) | 0x313e90 | ported | impulse on the hit component |
| `SkeletalMeshActor.PostBeginPlaySkeletalMeshIsHidden_Native` | folded with 0x1c26e0 | 0x59d10 | ported | empty |
| `SkeletalMeshComponent.GetBoneMatrixLocal` | 0x318810 | 0x318630 | ported | `LocalAtoms(BoneIndex).ToMatrix()`, identity out of range |
| `SkeletalMeshComponent.PlayParticleEffect` | 0x348d50 | (event) | ported | |
| `KActor.ApplyImpulse` | 0x1cfb20 | 0x382f40 | ported | |
| `KActor.TakeDamage` | 0x1c1640 (AActor's) | 0x385fe0 | ported | |
| `KAsset.TakeDamage` | 0x1c1640 (AActor's) | 0x3a8860 | ported | `CheckHitInfo` on the skeletal component, then the bone impulse |
| `DecalManager.CanSpawnDecals` | folded | vtable +932 = 0xd9f20 | ported | `AreDynamicDecalsEnabled()` |
| `DecalManager.SetDecalParameters` | 0x1d40f0 | 0xde1f0 | ported | the material goes through the decal's pooled `MaterialInstanceConstant` (`SetParent`) |
| `DecalManager.GetPooledComponent` | 0x1d4410 | 0x104d00 | ported | pooled decals carry a pooled MIC (`PoolMICs`, SDK @600) |
| `DecalManager.OnDecalFinished` | 2012 0x1d94f0 | 0x104c20 | ported | MIC loses its parent, both go back to the pools |
| `DecalManager.SpawnDecal` | 0x1d4450 | 0xe42e0 | ported | |
| `EmitterCameraLensEffectBase.ActivateLensEffect` | 0x1bb590 | 0x4bebe0 | ported | gore / non-extreme system through `GRI->ShouldShowGore` |
| `EmitterCameraLensEffectBase.RegisterCamera` | folded | 0x49d5c0 | ported | |
| `EmitterCameraLensEffectBase.NotifyRetriggered` | — | — | written | empty in the reference script, no named 2013 exec |
| `PrimitiveComponent.PutRigidBodyToSleep_Debug` | 0x3a27e0 | 0x3a27a0 | bringup | PhysX off (`WITH_NOVODEX=0`) → no-op |
| `PrimitiveComponent.RigidBodyIsAwake_Debug` | 0x3a3460 | 0x3a3420 | bringup | → `FALSE` |
| `PrimitiveComponent.SetRBPosition_Debug` | 0x3a31a0 | 0x3a2ff0 | bringup | → no-op |
| `PrimitiveComponent.SetRBRotation_Debug` | 0x3a32a0 | 0x3a30c0 | bringup | → no-op |
| `PrimitiveComponent.ShouldComponentAddToPrimitiveOctree` | 0x129270 | vtable +620 = 0x17d890 | ported | `return 1` |

### GameFramework crowd (`GameFramework/Src/GameCrowd.cpp`, `GameController.cpp`, `GameFrameworkClasses.h`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `GameCrowdAgent.InitializeAgent` | 0x5577a0 | 0x56c2a0 (2099 B) | written | spawn-destination pick and the view-biased warm-up position from the decompile; the nav-mesh intermediate point goes through the reference `UpdateIntermediatePoint` event; retail has no player-info array / group |
| `GameCrowdAgent.OnDestroyedByKismet` | 0x55a790 | 0x55fe80 (`KillAgent`) | ported | see hand-over 2 for the pool return |
| `GameCrowdAgent.TakeDamage` | 0x1c1640 (AActor's) | 0x559800 | ported | retail's `PlayDeath(Killer, Momentum, DamageType, Causer)`; ours keeps the reference `PlayDeath(FVector)` (hand-over 3) |
| `GameCrowdAgent.VolumeBasedDestroy` | folded | vtable +316 = 0x559930 | ported | `KillAgent()` |
| `GameCrowdAgent.FellOutOfWorld` / `OutsideWorldBounds` | folded / unnamed | — | ported | `KillAgent()` like the reference script |
| `GameCrowdAgentSkeletal.OnAnimEnd` | 0x557910 | 0x559c60 | ported | ended idle animation → `PlayIdleAnimation` |
| `GameCrowdInteractionPoint.SetEnabled` | 0x5579d0 | 0x559530 | ported | |
| `GameCrowdDestinationQueuePoint.ActuallyAdvance` | folded | 0x56abb0 | ported | `HasSpace` 0x55a1c0, `AddCustomer` 0x55a180, `AdvanceCustomerTo` 0x55a130 ported with it |
| `GameCrowdDestination.ReachedDestination` | 0x557a40 | 0x5678f0 | ported | kismet event, kill-when-reached, next-destination pick; retail has no interaction tags / behaviors / groups |
| `GameCrowdDestination.PickNewDestinationFor` | 0x557aa0 | 0x5613b0 | ported | frequency-weighted pick, visible-destination bonus |
| `GameCrowdDestination.AllowableDestinationFor` | folded with 0x1d4c40 | 0x561820 | ported | result cached in `bLastAllowableResult` (SDK @600 mask 4) |
| `GameCrowdDestination.IncrementCustomerCount` | folded | 0x55e5c0 | ported | queue switch when the arriving agent is closer |
| `GameCrowdDestination.DecrementCustomerCount` | 2012 0x63c740 | 0x561af0 | ported | |
| `GamePlayerController.CrowdFocus` | 0x5f3bf0 | 0x566940 | written | kills stale / far agents; the population manager's debug list (+608) is not declared here |
| `GamePlayerController.CrowdToggle` | 2012 0x1e8140 | 0x566af0 | written | kills spawned agents; the spawner toggle bit (+148 mask 1) is not declared here |

### Core (`Core/Src/UnCorSc.cpp`, `CoreClasses.h`, `UnObjBas.h`)

| Native | exec | body | status | note |
|---|---|---|---|---|
| `Object.VSmerp` | 0x9af0 | (in the exec) | ported | smoothstep vector lerp (Arkane addition to Object.uc) |
| `Object.RSmerp` | 0xad80 | (in the exec) | ported | smoothstep rotator lerp, optional shortest path |
| `Commandlet.Main` | 0x34270 | 0x5d8a0 | ported | the `Main` virtual → `eventMain` |
| `HelpCommandlet.Main` | unnamed | shares `UCommandlet::execMain` | ported | |
| `AutoTestManager.DoSentinelActionBeforeExit_Native` | 0x5ed7c0 | vtable +948 = 0x59d10 | bringup | empty in retail |
| `AutoTestManager.DoSentinelActionPerLoadedLevel_Native` | 0x5f6340 | 0x30be60 | bringup | retail writes `PROFILING\MapInfos\<map>.inf` under `-dumpmapinfos`; Sentinel-only → no-op |

## What is left (29 real gaps, `build/agentAE_work/inventory_after.txt`)

1. **UEngine / UGameEngine (6)** — `Engine.OnControllerDisconnected` (exec 0x1d47c0, body vtable +304),
   `Engine.OpenContentUnavailableMenu` (0x1c5e90, +312), `Engine.OpenControllerConnectionMenu` (2012 0x1d00d0),
   `Engine.OpenPauseMenu` (0x1c8910, +300), `Engine.WaitMovie` (0x1e2600 → `GFullScreenMovie->GameThreadWaitForMovie`),
   `GameEngine.GetDLCManagementBridge` (0x2140f0 → `UGameEngine::DLCManagementBridge`, SDK @1688).
   **Left to agent AI on purpose**: the first four are the UEngine retail virtuals AI declares in vtable order this
   wave (PHASE6 facts table); their execs are one line each once the virtuals exist. `UDishonoredEngine` already
   implements `OpenPauseMenu` / `OnControllerDisconnected` / `OpenContentUnavailableMenu` (agent AC).
2. **UI data stores (22)** — `UIDataProvider.GetProviderFieldType` (0x1e9f10) / `ParseArrayDelimiter` (0x1ea010 →
   0x3dfe60), `UIDataProvider_MenuItem.IsFiltered` (0x1c85a0 → 0x3e7990),
   `UIDataProvider_OnlinePlayerStorage.OnReadStorageComplete_Native` (0x5efc80 → 0x3e1660), `UIDataStore.OnCommit`
   (folded with 0x5efc40), `UIDataStore_{Dynamic,Game}Resource.{FindProviderIndexByFieldValue, GenerateProviderAccessTag,
   GetProviderCount, GetProviderFieldValue, GetResourceProviderFields}` (0x1f4dc0/0x1f5050, 0x1c70b0/0x1c7280,
   0x1c7320/—, 0x1f4c60/0x1f4ef0, 0x1fa1f0/0x1fa410 → bodies 0x3f9c80/0x3f85a0, 0x3f9e80/0x3f8790, 0x3fdd40/0x3fdc20,
   0x3f9f40/0x3f8850, 0x401490/0x400f00), `UIDataStore_MenuItems.{AppendToSet, ClearSet, GetSet}` (0x1c7410, 0x1c73a0 →
   0x3fbe90, 0x1fa520), `UIDataStore_OnlinePlayerData.OnSettingProviderChanged` (0x1c74b0 → 0x3e1140),
   `UIDynamicDataProvider.{Bind,Unbind}ProviderInstance` (0x1c8490/0x1c84f0 → 0x3cf010/0x3cf090),
   `UIRoot.{Get,Set}DataStoreFieldValue` (0x1f9bc0/0x1f9a80 → 0x3ee650/0x3ee540).
   **All 22 are already decompiled** in `resources/reference/decomp/agentAE/2013b` — they are pure UI plumbing, not on
   the startup or map path (no warn-once line for any of them in either run below), so they were left for last and the
   wave ran out of scope before them. Whoever picks them up: they belong in `UnUIDataStores.cpp` / `UnUIObjects.cpp`
   with `DECLARE_FUNCTION`/`MAP_NATIVE` lines in `EngineUIPrivateClasses.h` / `EngineUserInterfaceClasses.h`.
3. **7 interface natives are not gaps** — `UIDataStoreSubscriber.{ClearBoundDataStores, GetBoundDataStores,
   GetDataStoreBinding, NotifyDataStoreValueUpdated, RefreshSubscriberValue, SetDataStoreBinding}` and
   `UIDataStorePublisher.SaveSubscriberValue` belong to `CLASS_Interface` classes, so `UFunction::Bind` gives them
   `UObject::ProcessInternal` and never reaches `FindNative` (`UnClass.cpp:2753`). The implementors (GFxUI's
   `UGFxDataStoreSubscriber`, 2012 0x5b6080…) are GFxUI's, not mine.

## Hand-overs outside my files

1. **`FAttachmentInfos` → agent AI** (`DishonoredGame/Inc/DishonoredGameEngineShims.h:12592`, 36 bytes,
   `DishonoredGameLayouts.h:240`). `Actor.DoKismetAttachment` (exec 0x1df6c0) reads it as its second parameter and
   `AActor::DoKismetAttachment` (0x18ab50) / `APawn::DoKismetAttachment` (0x2a1180) take it by value. Until the struct
   moves into Engine, my exec only consumes the parameters. Once it is an Engine type, the virtual pair is a
   half-hour port (both decompiles are in `2013b`/`2013c`).
2. **`UGameCrowdSpawner` / `AddedToPool` → whoever owns the crowd pool.** Retail `AGameCrowdAgent::KillAgent`
   (0x55fe80) notifies the spawner (`UGameCrowdSpawner` vtable +304) and returns the agent to the population pool
   (`AddedToPool`, vtable +944). `UGameCrowdSpawner` is only declared in `DishonoredGameEngineShims.h:13726`, i.e. it
   is invisible to GameFramework, so my `KillAgent` expires the agent the reference way (`LifeSpan = -0.1`,
   `TimeSinceLastTick = 1000`) and `InitializeAgent`'s beyond-spawn-distance branch uses `MaxSpawnDist = 0`.
3. **`AGameCrowdAgent::PlayDeath` signature.** Retail's virtual (+928) is
   `PlayDeath(AController* Killer, FVector Momentum, UClass* DamageType, AActor* DamageCauser)`; our
   `GameFrameworkClasses.h` declares the reference `PlayDeath(FVector KillMomentum)`. `TakeDamage` therefore builds the
   momentum vector from `KDamageImpulse`/`KDeathUpKick` and calls the reference form. Changing the signature touches
   `GameCrowd.cpp` + `disgamecrowdagentskeletalrat.cpp` (0x8d2640) and belongs to one owner.
4. **`EngineGameEngineClasses.h` is AI's file** — I added only `DECLARE_FUNCTION`/`MAP_NATIVE`/method-declaration
   lines for `AWorldInfo.GetGlobalGravityZ` and the six DLC natives (the shared-header rule of PHASE6
   "Coordination and merge"). Same for `PrimitiveComponent.h`, `UnSkeletalMesh.h`, `EngineSkeletalMeshClasses.h`,
   `EngineDecalClasses.h`, `EngineParticleClasses.h`, `EngineAnimClasses.h`, `EnginePhysicsClasses.h`,
   `EngineSequenceClasses.h`, `EngineMeshClasses.h`, `CoreClasses.h`, `UnObjBas.h`: declaration lines only, no layout edits.
5. **`APlayerController::CheckJumpOrDuck` → agent AF.** `PlayerMove_Walking` calls it (vtable +1240); the Engine body
   is empty and `ADishonoredPlayerController::CheckJumpOrDuck` (0x6ae490) is the real one. `UpdateRotation` reads
   `PlayerInput->aTurn`/`aLookUp`, so AF's input chain feeds it directly.
6. **`UnPhysic.cpp` gained two includes** (`EngineAnimClasses.h`, `EngineParticleClasses.h`) because the
   `UAnimNodeSlot` and `AEmitterCameraLensEffectBase` bodies live there.

7. **`ADishonoredPawn::ChooseAndTriggerDeathEvent_Native` → the DishonoredGame owner.** My `APawn::Died`
   (0x17f600) raises the retail chain `PreventDeath` -> `ChooseAndTriggerDeathEvent` -> `NotifyKilled` -> `PlayDying`
   through `FindFunction` + `ProcessEvent`, so the DishonoredGame native `ChooseAndTriggerDeathEvent_Native` is now
   reached during startup (`[0004.01]` of the `-strictnatives` run B). It has no C++ body yet, so under
   `-strictnatives` it aborts and without it the warn-once stub consumes the parameters. Engine-side nothing more is
   needed; the body belongs to whoever owns `dishonoredpawn.cpp`.

## Method

* Inventory: `build/agentAE_work/inventory.py` (script_classes_2013.json × the tree's `MAP_NATIVE` set).
* Exec/body pairing: the exec decompile shows the vtable slot (`(*(...)(*(_DWORD *)this + N))`), then
  `build/agentAE_work/dump_vtables2.py` resolves slot → function for each class. It reads `??_7<Class>@@6B…`
  symbols and, for the classes whose vtable symbol the named 2013 db lacks (ACamera, ADecalManager,
  AEmitterCameraLensEffectBase, AGameInfo, AGamePlayerController, UCheatManager, UUIDataStore,
  ASkeletalMeshActor), locates the vtable by signature: it looks up every 2012 slot function by name in the 2013
  db, takes data xrefs to them and votes for the base address (140/141 slot matches for the ones above).
  Multiple-inheritance classes pick the primary vtable (`??_7C@@6B@`, else `…6B<Base>@@@`).
* All 250 decompiles are headless (`resources/tools/ida/run.py`), private db copies only, never the IDA MCP tools;
  FModel was not used (it is UE4-only).
* Every edit carries `// DISHONORED(port|written|bringup): <2013 rva>`.
