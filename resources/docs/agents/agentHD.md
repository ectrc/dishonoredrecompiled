# Agent HD — projection/FOV, post-ride heading (wave 25, PHASE22 package HD)

## 0. The answer

1. **The projection is retail's again.** Build 9014 has no `AspectRatioAxisConstraint`; the storage-less shim read 0 =
   `AspectRatio_MaintainYFOV`, so `ULocalPlayer::CalcSceneView` applied the FOV vertically (75° v = 107° h at 16:9).
   Retail's unconstrained branch (2013 `0x6c4710`, 2012 `0x2df1c0`; checked in the disassembly at `0x6c4c9e..0x6c4d10`,
   FViewport slots 2/3/34 = GetSizeX/GetSizeY/GetDesiredAspectRatio) is the pre-9014
   `FPerspectiveMatrix(fFOV*PI/360, SizeX*DesiredAspect/CurViewAspect, SizeY, NEAR)` - horizontal. Ported; the shim is
   deleted. **Zoom over the ride 1.76 → 0.99** (`cutscene_compare`, below).
2. **The post-ride heading was not the camera at all - it was the pawn's facing.** `ADishonoredPlayerPawn::FaceRotation`
   (2013 `0x6b0d40`) was a native stub, so `APlayerController::UpdateRotation` reached `APawn::FaceRotation`, which sets the
   pawn to the controller's rotation. During the ride the boat's `Boat_Jnt` bone moves the pawn every tick (pawn yaw -912 at the
   end), but the controller never leaves its pre-ride -12736. The Attachment track detaches the pawn two frames before the
   soirée EXIT (world 150.344 vs 150.42); in those frames the pawn snapped to -12736, the camera bone followed, the AnimDriven
   influence (still at weight 1, `m_bMatchControllerRot` set) recorded -12736 as the reset target, and the PlayerControl reset
   faithfully applied it. Retail's FaceRotation keeps the pawn's yaw while the PlayerControl influence has no weight. Ported:
   **the view is -26/-912 from EXIT to world 175** (was -26/-12736, 65° off) and the frame shows the deck and the doorway ahead.
3. `StartTrackingFixedAngles` and the fixed-angle tracking it needs (8 functions) are ported, as the plan asked (3b), and the
   PlayerControl reset uses it as retail does; with (2) fixed the pawn yaw equals the target, so the tracking has nothing to turn.

## 1. Where the plan was wrong, with the measurement

- **3a/3b/3c named the wrong suspect order.** The last `UDisInterpTrackAllowPlayerLook` key does carry `m_bMatchControllerRot`
  (flags 1 to the end), `m_bResetControllerRot` is raised (`OnActivate_Derived first 0 -> reset 1 goto 1`), and the missing
  `StartTrackingFixedAngles` only changes how the yaw gets to its target, not where it ends. The fault was (c)-adjacent: the pawn's
  `Rotation` is right under the socket attachment (it follows `Boat_Jnt`), and wrong for the two frames after the detach,
  because of an unported `FaceRotation` (HD1 `pcamdiag pre-exit` lines: `pawn 0/-912 ... base SkeletalMeshActorMAT_2 bone
  Boat_Jnt` → `pawn 0/-12736 ... base none`).
- **"Every 3D view narrows 1.78x - the menu too" is right but the menu cannot be scored as asked.** FB's retail capture
  (`build/agentFB/retail/shot_02_mainmenu.png`) has a Windows firewall dialog over the centre and shows another of the menu
  level's eleven cameras than ours; `resources/reference/menu/1.png`'s centre is the 2D options overlay (fit 1.00 whether the
  world is wide or not). What is measured instead: the menu camera's 90° now renders 90.00° h / 58.72° v (`pcamdiag proj`),
  which is FE's image fit of retail (61 ± 2° *vertical* in the old axis convention).
- **Item 2 (hand scale with the sword) cannot be checked by image**: with `-gjgive=DishonoredWepSword -gjequip` (HD3/HDC3) and with
  GJ's `exec Dis_EquipItemByType` after the ride (HD4/HDC4, shots world 168/172) neither build draws an arm mesh. Analytically
  the arms are independent of the view projection: `FDishonoredPlayerSkeletalSceneProxy::GetWorldMatrices` re-projects through
  `MeshProjection * Proj^-1 * View^-1`, so clip = Local · Shifted · View · MeshProjection whatever `Proj` is.
- **The two-build budget became three**: build 1 needed a compile fix (`FString::Printf` takes at most 27 arguments) and was
  re-run incrementally; build 3 is the coordinator's requested merge of main (`f147830`) re-built for the gate.

## 2. Changes (worktree `build/agentHD_wt`, base `f147830`; `build/agentHD_sync.py` lists every file and function)

Engine
- `Src/UnPlayer.cpp` `ULocalPlayer::CalcSceneView`: retail's horizontal projection; `-pcamdiag` once a second (FOV, axis,
  M00/M11, implied h/v FOV, view and viewport size, desired aspect). `FastDeProject` / `FastProject` (reference-only, not in
  retail) build the same matrix. Constrained branch and `DeProject` (2013 `0x6c5180`, through CalcSceneView) unchanged - same
  as retail.
- `Inc/EngineClasses.h`: the `AspectRatioAxisConstraint` shim deleted; `shim_ceiling.json` 887 → 886.

DishonoredGame
- `ADishonoredPlayerPawn::FaceRotation` + exec (`dishonoredplayerpawn.cpp`, 2012 `0x70afd0` / 2013 `0x6b0d40`): keep pitch/yaw
  while PlayerControl's weight ≤ 1e-8 or the mesh is `RMRM_RotateActor`; else drop the pitch for a base/action state that ignores
  it (unless `bCheatFlying`); walking/falling drop the pitch; roll 0. Guard (written): no camera → face as asked.
- Tracking (`dishonoredplayercontroller_tracking.cpp`, off the exclude list): `FDisPlayerViewTarget::SetTarget` (fixed) /
  `ClearTarget` / `TickTracking` / `IsTracking`, `ADishonoredPlayerController::StartTrackingFixedAngles` / `StopTrackingTarget` /
  `FetchTrackedTargetRot` (2013 drops the player delta from the untracked answer) / `TickCameraTracking`; `OnCameraUpdate` pass 0
  calls it with (pawn X, pawn Y, view Z) as 2013 `0x6ba860`; `DisLerpAngle` (2013 `0x7bb620`); `DisGetPlayerInputTweaks` shared.
- `UDishonoredCamera_PlayerControl::GatherInfluence_Derived`: the reset tracks the yaw (`StartTrackingFixedAngles(pitch, yaw,
  m_TransitionSpeed, 0x22)`, final yaw = the controller's) when the transition is finite, as 2013 `0x6cba40`.
- `-pcamdiag` (`fzsoireediag.cpp/.h`): per second, and every frame for 2 s after `ExitSoireeMode` with the 8 frames before
  replayed: view/controller/pawn/mesh/camera-bone rotation, the pawn's base and bone, physics, body mode, PlayerControl and
  AnimDriven weights and reset/goto/first flags and target; `OnActivate_Derived` and the reset print their own lines.

Not ported (named): `ProcessViewRotation` (2013 `0x6afb50`: the input delta into `ApplyPlayerControlDelta` while tracking, the
limits, the dispersion), the actor/pawn tracking starts, `UpdateBlindedEffect`.

## 3. Results (exact commands; all on the staged `DishonoredGame_HD.exe`)

`python resources/tools/cutscene_compare.py run --exe ...DishonoredGame_HD.exe --tag HD<n> --baseline build/cmp/head_rel/scores.csv -- -pcamdiag`

| run | build | zoom median (p10/p90) over ride | within ±0.04 | SSIM | MAD | post-EXIT view yaw |
|---|---|---|---|---|---|---|
| head_rel (baseline) | HEAD | 1.76 (1.74/1.78) | 0 % | 0.523 | 29.4 | -12736 |
| HD1 | build 1 (projection, tracking, diag) | 0.99 (0.96/1.00) | 76 % | 0.595 | 21.8 | -12736 |
| HD2 | build 2 (+ FaceRotation) | 0.99 (0.95/1.00) | 81 % | 0.592 | 21.6 | **-912** |
| HD3 | build 3 (main f147830 merged: HA, HE, HF) | 0.99 | | 0.572 | 26.7 | **-912** |

The view yaw before the detach is -912 (the camera bone on `Boat_Jnt`); after EXIT it stays -912 at every `pcamdiag` sample to
world 175 (`CC_HD2.log`). Body mode 1 (full body, no arm mesh) through the ride, 0 after EXIT. The camera height (item 4): after
the zoom the doorway arch in `pair_w0153` sits within ~10 px of retail's (of 540); no item opened.

## 4. Open items

- `ProcessViewRotation` 2013 `0x6afb50` (input during tracking, pitch/yaw limits, ranged dispersion) - input-path risk, own package.
- The arms are not drawn after the ride in either build (GJ/GO territory), so the hand scale under the new projection is
  unverified by image.
- HF's markers (`Tick_Markers`, `ULocalPlayer::Project`) now read the horizontal projection - re-check their screen positions.

## 5. Gate

`python resources/tools/run_regression.py --build-dir build/agentHD --no-build` on build 3 (main f147830 + HD): every game stage
ok - nullrhi, d3d9, inputtest (`inputtest_moved` 1050.7, HEAD 1029.6), newgame (29 pawns, 120 on-mission lines), options
(6 steps, 39 AS2 errors), every `*_criticals` 0. `coresmoke`/`layout` first failed only because the CoreSmoke and LayoutProbe
targets had not been built in this build dir; built and re-run with `--only coresmoke,layout`: 9 ok. Generator regeneration
byte-identical (modulo CRLF), `bitfield_audit` 0 mismatches, `shim_ratchet` OK (ceiling lowered by one).

HD3's MAD (26.7) is above HD2's (21.6): HD3 carries main's HA/HE/HF merges; the zoom (0.99) and the post-EXIT yaw (-912) are
unchanged by them.
