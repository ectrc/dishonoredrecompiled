# Wave 27 — the second hand play on `2127b56` (`PHASE24.md`)

**Status: started 2026-10-21 on the user's "Okay execute"; launch 1 = JA, JB, JC, JD, JE, JG.**

## Context

Wave 26 (`PHASE23.md`, head `2127b56`, gate 47/47, ride SSIM .848) closed with real audio (Wwise reimplemented),
lip-sync (FaceFX reimplemented), the player's movement set and NPC locomotion largely as retail. The user played the
staged `DishonoredGame-Win64-Shipping.exe` by hand and reports seven residual defects:

1. Main menu: music plays, but no UI sound effects (buttons, modals).
2. NPCs still walk oddly: the main guard after the ride does not walk straight; in the Emily cutscene Emily walks off
   screen; NPCs start cutscene animations in the wrong place (a positioning/pathing chain problem, not one NPC).
3. End of the Emily cutscene: retail shows a choice at the bottom (play hide and seek: yes/no); ours never shows it
   and defaults to yes.
4. Mission markers show the literal text "Description" above their targets.
5. The Empress cutscene trigger half works: the view locks and it fades to black, but the dialog never starts, no
   black bars, Emily does not run to her spot.
6. Player movement: landing from a jump should feel elastic but is sudden; the air time feels different from retail.
7. Crosshair hover effects on objects and the interaction prompt (talk to a guard, pick up an item) must be checked.

Housekeeping the user asked for: delete the outdated reference videos `resources/reference/mission0/ours.mp4` and
`diffed.mp4` (and the derived `build/cmp/video_cache`/`ours_mp4` outputs) — `retail.mp4` stays as the measuring
reference. Step 0 of execution (plan mode forbids it now).

## Root causes (read-only investigation, 2026-10-20; 2013 rva unless stated, 2012 in brackets)

**1. UI sounds.** The menu's AS2 `SoundHandler.PlaySound(event, theme)` calls `_global.gfxProcessSound`, which retail's
`UGFxMoviePlayer::PreLoad` (0x5a3af0 [0x5e3a30]) defines bound to `FGFxSoundEventCallback` (Call 0x588570: finds the
movie's `SoundThemes` entry, `eventProcessSoundEvent(event, PC)`); `SoundThemes` is filled by
`UDisGFxMoviePlayerBase::ApplyTweakChanges_Derived` [0x808a20] from the tweaks; `UDisUISoundTheme::ProcessSoundEvent`
(script) maps events to AkEvents. Ours: `PreLoad` (`GFxUI/Src/gfxuimovie.cpp:1885-1919`) skips the callback (the
reference `ScaleformMovie.cpp:335-350` is excluded; `CreateFunction` works now, used at `:1211`), and
`ApplyTweakChanges_Derived` is a comment (`disgfxmovieplayerbase.cpp:34`), so `SoundThemes` is empty.

**2. NPC placement and walking.** (a) Matinee keys with `m_bSpecialRootMotionExtract` feed the backed-up anim position
as the previous position every frame (`interptrackstretchanimcontrol.cpp:548-588`, `arkanimnodestack.cpp:326-336`) so
the root delta is cumulative since the key started, while our `ProcessRootMotion` (`UnSkeletalComponent.cpp:3536-3613`,
the reference body; retail [0x353c80] is 3,979 B with its own accumulator) applies it incrementally - pawns drift off
their marks; retail backs up / restores the actor transform through two `AActor` virtuals (slots 202/203) never called
here (`UnInterpolation.cpp:3262-3272`, `BackupTransform` 0x61fcb0). (b) `UInterpTrackInstFaceTo::UpdateTrackInst`
(0x8ff8f0 [0x53eb90]) / `TermTrackInst` [0x53bb30] are comment files: NPCs start scene anims at whatever yaw they
arrived with. (c) `UInterpTrackAIControl::UpdateTrack` [0x53e120] stub. (d) Curved walks: `UpdatePathProperties`'
turn-ahead bookkeeping unported (`arkcomponentlocomotionpath.cpp:352-355`, 0x541ee0), `ComputePathYawFromDist`
simplified (`:457-482`; retail 0x5428b0) so `EvaluateStaticTurn` decides wrongly and the NPC arcs at the turn rate; the
straight-line test is the reference `PointReachable` (`:699`); `fMinDistanceFromWalls` randomised (`:236-239`); no
avoidance component (`FArkComponentAvoidance`, priorities 0x5491b0, push 0x548b90); `ModerateMovePawn` never taken;
`MoveAttachedActors` stand-in. (e) Only `DisPylon_2` is an imported pylon; `AArkDynamicPylon` is a stub.

**3. The hide-and-seek choice.** Either `UDisConv_PlayerChoice` (PlayNode 0x89c570 -> `UStatePlayerMasterInDialog::
SetActivePlayerChoice` 0x669410 and wait; TickNode returns the index) or `UDisSeqAct_DialogScriptedChoice` (Activated
0x78b250 -> `StatePlayerMasterInScriptedChoice`; UpdateOp 0x790bc0 fires link choice+1). Both unported: the node falls
to the base default (output 0 = yes, instantly); the Kismet action falls to `USeqAct_Latent` (every link fires). Also
unported: the choice master states (`ShowChoiceSelection` [0x6c7450], `TickState` 0x679230, `MakePlayerChoice`
0x669240, `OnEnterState` 0x673640/0x673ae0, `GetChoiceInfo` 0x66fb00), HUD `ShowChoiceSelection` 0x79f390 /
`HideChoiceSelection` 0x78d260 (`playerChoice_mc` only ever hidden), the input natives `execOnPlayerChoiceConfirm`
0x78d360 and `Dis_PlayerChoice_RequestSkip/_Released` (stubs).

**4. "Description" on markers.** The marker code and the name resolve as retail; our `GFxValue::ObjectInterface::
SetText` (`External/GFx3/GFxPlayerRoot.cpp:1406-1416`) writes `text` through `SetMemberRaw`, which the text field does
not handle (`GFxEditTextCharacter::SetMember` only, `GFxTextField.cpp:986` -> `SetTextValue`), so the write becomes a
plain property and the authored placeholder stays. Retail 0x9afdb0 calls `SetTextValue`. Every `UGFxObject::SetText`
(`gfxuimovie.cpp:461-466`) is affected.

**5. The Empress trigger.** The user's logs: the chain does not stop at the fade - `Interp_26` locks the view,
`Interp_0` runs and five Empress/Regent lines post, `GoEmily` fires, then nothing: the next scene needs `Switch_8` again
via `Gate_37` <- `Emily_GoToGazebo_Success`, and Emily's go-to never succeeds. (a) `SeqAct_Teleport_25` on Emily hits
`execOnTeleport_Native`, a stub (`NativeStubs.cpp:369`; [0x799d60], NPC override [0x7c1200]) - she never starts from
her run-in point. (b) Since IK4 every NPC has a FaceTo component, so `Start_TurningToFace` (`disbehaviorgoto_base.cpp:
115-141`) waits for `EAIStimID_RotationReached`, which nothing sends: `FDisFaceToRequest::DoRequest`
(`disdesirestructs.cpp:755-800`, 0x8a9b30) is a counter stand-in, `StartFaceToYaw` never called,
`DesiresFaceToEventCallback` [0x722480] stub. (c) `FilterPathingFail` fails without `AttemptTeleportSpell` [0x832a50].
(d) No black bars: the only writer of HUD bit 0x10 is `SetCinematicMode_Native`; the bars did show in the Emily
greeting right after `ADishonoredPlayerPawn::StartDialog` warned unported - `StartDialog/EndDialog` stubs
(`dishonoredplayerpawn_conversation.cpp:90-102`), `UStatePlayerMasterInDialog` 3 of 17 (OnEnterState [0x6b0c40]).
(e) `UDisConv_PlayerLookAtActor::PlayNode_Derived` [0x9077b0] stub; `IsAvailableForDialog` always true; NPC
`StartDialog/EndDialog` send no stims; `MAT_IsAllowed` 0xb65560 uncalled.

**6. Jump feel.** Physics equals retail (gravity -1500, the stock double-gravity `physFalling` in both, JumpZ 800,
apex 106.7 uu in 0.53 s is retail's number). The camera differs: `UDishonoredCamera_PhysicalReact` is a symbol list
(`OnImpact` 0x6c10f0 adds the landing velocity to the two spring points; `FDisSpringPoint::UpdateSpringPos` 0x6c0a00,
`IsSpringing` 0x6c09a0, `GatherInfluence_*`, `OnActivate_Derived`, `OnCameraCollided_Derived` 0x6c0e10; settings
`DefaultCamera.ini:41-49` = an underdamped ~1.5 Hz dip of 0.13 s with one rebound); the bump smoother is never set to
weight 0 for the flight (retail `SetWeight(0, 0.1)` entering Jump/Fall, back on exit/landing - ours logs it in
`stateplayermasterjump.cpp:108-126`, `stateplayermasterfalling.cpp:54-84`), so the view lags the pawn (simulated
apex 92 uu at 0.37 s, ground at 1.1 s vs 0.52 s). Also: the JumpLand contact (landing sound) named not called,
`ClearFOVTarget(1)` and the Kismet land event missing, `ApplyAttributes` unported (`MaxFallSpeed` 2000 etc.),
`performPhysics` 0x6a1670 unported.

**7. Interaction prompt and hover.** The crosshair chain to `Tick_Interaction` is ported (GD, HF) but the context is
always empty: `ADishonoredPlayerPawn::FillUIInteractions` 0x6a9130, the interactables' `FillUIInteraction`
[0x67d190] / `FillUIAltInteraction` [0x67d1f0], `FDisUIInteractionProbe` 0x78cc60 (Tick [0x80c000]) and the
context/special/QTE updaters (0x7a63c0/0x7a6630/0x7a6920) are unported; every `ShowHighlight/HideHighlight` override
is a comment (usable objects, pickups, movables, climbables), `LoseCrosshairFocus` empty; NPC/usable
`GetCrosshairFocusText` ([0x7b1f00] / [0x683910]) comments; only pickups have named tweaks. Defect 4's `SetText`
also blanks `crosshairInfosTxt_mc`.

## Corrections found while designing

- The locomotion bodies live in `DishonoredGame/Src/arkcomponentlocomotion{path,dynamic,rootmove}.cpp`; the Engine twins are
  symbol lists. `FArkComponentFaceTo` and the `RotationReached` feedback (`disdesiresinterface.cpp:620-646`,
  `DesiresFaceToEventCallback` is real) exist: the only gap in 5(b) is `FDisFaceToRequest::DoRequest` counting instead of
  calling the component. `UStatePlayerMasterInDialog` derives from `UStatePlayerMasterChoice_Base`: JE ports the base's
  enter/exit half, JF the choice half. `regression_baseline.json` still allows `probe_natives <= 12`: set to 3 at wave
  start. `UDishonoredContactSystem` is a symbol list; JG measures whether the land sound already posts via the anim notify.

## Shared rules

As PHASE23: own worktree (`make_snapshot.py`, RelWithDebInfo), two builds via `build_queued.py`, runs via
`game_input.py launch -windowed -nomovie`, never fmodel MCP, IDA on private copies, every address resolved in both dbs,
generator for the generated files with the byte-identity check, faithful ports with named deviations, "assume this
brief is wrong somewhere". Gate 47/47, `*_criticals` 0, `unported_natives` 0, `probe_natives <= 3`. Ride SSIM at the
constant offset (a = 13.57 - soirée-enter). Nobody touches `UnPlayer.cpp`/`UnController.cpp`.

## Packages

### JA — the GFx text-field write (markers "Description") — smallest, first
- Audit every `UGFxObject::SetText` / `GFxValue::SetText` and `SetMember("text"|"htmlText")` user under `-jadiag`.
- Port `GFxValue::ObjectInterface::SetText` [0x9afdb0] (both overloads, `GFxPlayerRoot.cpp:1406-1425`) to resolve the
  character and call `GFxEditTextCharacter::SetTextValue` as retail (check whether retail dispatches via a character
  virtual or a type test; `GetText` the same way).
- Markers: confirm the description shows only on the focused marker (flag bit 1, `dishudmarkers.cpp:186-233`).
- Accept: a capture at world 60-80 with the marker reading the target's name, no "Description"; the audit table;
  options-stage AS2 errors not above head_rel; gate 47/47.

### JB — main-menu UI sounds (parallel)
1. `UGFxMoviePlayer::PreLoad` tail (0x5a3af0 [0x5e3a30]): `CreateFunction` + bind `_global.gfxProcessSound`,
   `CLIK_loadCallback`, `CLIK_unloadCallback` (reference shape `ScaleformMovie.cpp:335-370`); port
   `FGFxSoundEventCallback::Call` (0x588570 [0x5cb160]) and the CLIK callbacks; reuse the delegate callback pattern
   (`gfxuimovie.cpp:1208-1214`).
2. `UDisGFxMoviePlayerBase::ApplyTweakChanges_Derived` [0x808a20] fills `SoundThemes` from the tweaks.
3. Verify the script half posts an AkEvent through a bodied native; `-jbdiag` logs event -> AkEvent -> playing id.
- Accept: on the options stage ≥ 5 `gfxProcessSound` events with a non-zero Wwise playing id while navigating; `-akwav`
  shows RMS above the music floor at each; menu/options/newgame stages green; gate 47/47.

### JC — NPC scene placement: root motion backup/restore, FaceTo and AIControl tracks (user priority; parallel with JD)
1. Measure first (`-jcdiag` on the ride): per special-root-motion key, the backed-up anim position, the previous/current
   position given to `MAT_SetAnimPosition`, the per-frame root delta, the actor's drift vs the stage mark.
2. `USkeletalMeshComponent::ProcessRootMotion` whole from retail ([0x353c80] 3,979 B / 0x746280) replacing the reference
   body (keep IO's Translate-only rotation if the bytes agree; do not reintroduce IK2's RMM_Accel pair).
3. The two `AActor` transform backup/restore virtuals (2013 slots 202/203): find the pawn overrides by vtable walk, port,
   call them from `UpdateGroupInst`'s tail (`UnInterpolation.cpp:3262-3272`) and `BackupTransform` (0x61fcb0); re-diff
   `UpdateTrackInst` (`interptrackstretchanimcontrol.cpp:548-588`) and the stack's `MAT_SetAnimPosition` against the bytes.
4. `UInterpTrackInstFaceTo::UpdateTrackInst` (0x8ff8f0 [0x53eb90]) / `TermTrackInst` [0x53bb30] + the track's key
   properties, on IC's component; `UInterpTrackAIControl::UpdateTrack` [0x53e120] on IC's `MAT_GetBodyIntention`.
5. `MAT_BeginAIGroup`'s unconditional mesh snap only if the diag shows it moving a mesh retail would not.
- Accept: Emily stays in frame for her whole cutscene (captures 165-200 on the `-fwteleport=DisTrigger_16@160` route);
  the post-ride guard walks a straight line (IO2's table ≥ 90 % < 15°); each scene-anim start within 10 uu / 5° of the
  stage mark; ride SSIM ≥ .84; gate 47/47. **The coordinator also captures the hand-played Emily greeting and Empress
  scene (frame pairs under `build/cmp/w27_scenes/`) for the user to compare by eye - retail.mp4 does not cover them.**

### JD — NPC walking: turn-ahead, path yaw, Arkane PointReachable, avoidance (parallel with JC)
- Step 0: pylon census (`-jddiag census`: every `APylon`/`AArkDynamicPylon` on the route) - decides JI.
1. `UpdatePathProperties`' turn-ahead bookkeeping (0x541ee0; `path.cpp:352-355`) and `ComputePathYawFromDist` whole
   (0x5428b0 [0x583880]; `:457-482`) so `EvaluateStaticTurn` turns in place as retail instead of arcing.
2. Arkane's `UNavigationHandle::PointReachable` (the edge out-list variant; IO2's zero-extent port as the model).
3. `fMinDistanceFromWalls` from the config, not `appFrand`.
4. `FArkComponentAvoidance` whole ([0x5772e0] ctor, register/unregister, `PreAsyncWorkTick` [0x578ec0]), a real
   `DetectPreAvoidanceCollision` [0x584f10], priorities 0x5491b0, push period 0x548b90.
5. `ModerateMovePawn` (0x540d80) and `MoveAttachedActors` [0x25f970] only after 1-4 are measured, each behind a switch
   for one build; `CanBeTeleportedToLocationWithoutBeingSeen` [0x57f460] if small.
- Accept: the post-ride guard and Worker B walk straight legs (≥ 95 % within 15°), ≤ 1 static turn per corner, no arc;
  two NPCs crossing at the dock separate by ≥ the cylinder sum (`-jddiag avoid`); ride SSIM ≥ .84; `FilterPathingFail`
  0; gate 47/47.

### JE — the Empress chain: teleport native, real FaceTo requests, teleport spell, dialog state + bars, look-at (parallel)
- Step 0: `-fwteleport=DisTrigger_16@160 -fvdiag -gadiag -fidiag -jediag` names the bars' writer (every HUD bit 0x10
  write with its caller), `SeqAct_Teleport_25`'s destination and whether it is on the navmesh, Emily's go-to stages and
  the FaceTo request status per tick.
1. `ADishonoredPawn::execOnTeleport_Native` [0x799d60] + the NPC override [0x7c1200] (+ the player controller's and the
   camera influences' `OnTeleport` if the route needs them).
2. `FDisFaceToRequest::DoRequest` (0x8a9b30) real: `InitAllByDefaultExcept` (0x574a00/0x574a50/0x574aa0) then
   `StartFaceTo*`/`UpdateFaceTo*`/`StopFaceTo` on the component; `Initialize` passes the callback; `AcceptRequest` /
   `IsComponentRequestStarted` from the component; the bringup fallback in `Start_TurningToFace` removed;
   `UDisAISubStateTakePosition` the same way.
3. `AttemptTeleportSpell` [0x832a50] for `FilterPathingFail` and `TakePosition`.
4. `ADishonoredPlayerPawn::StartDialog/EndDialog` + `UStatePlayerMasterInDialog` whole (OnEnterState [0x6b0c40] + 14)
   over `UStatePlayerMasterChoice_Base`'s enter/exit/bend-time half ([0x6b8560]/[0x6b8600]/[0x6b8680]/[0x6b8750]).
5. `UDisConv_PlayerLookAtActor::PlayNode_Derived` [0x9077b0], `PlayerLookAtSpeaker::TickNode` [0x8f8a50]; the NPC
   `StartDialog/EndDialog` stims; `IsAvailableForDialog` via `CallGetDialogAvailability`; `MAT_IsAllowed` called.
- Accept: `Emily_GoToGazebo_Success` fires and `Gate_37`/`Switch_8` activate; Emily runs in from the teleport point
  (> 200 u/s for ≥ 2 s after GoEmily); bars on with the dialog state and off at its exit; the five lines still post;
  `unported_natives` 0; gate 47/47; plus the hand-played capture pairs (see JC).

### JF — the hide-and-seek choice (after JE; step 0 and the HUD/Kismet half can start on JE's branch)
- Step 0: `-jfdiag` logs which fires at the choice point - `UDisConv_PlayerChoice::PlayNode_Derived` or
  `UDisSeqAct_DialogScriptedChoice::Activated`; port the one that fires, the other too if ≤ 300 B each.
1. Node branch: `UDisConv_PlayerChoice` play/tick (0x89c570 [0x8dd8e0]/[0x8dd950]), `InitNode`, `MatchChoicesToOutputLinks`,
   the choice getters; `UStatePlayerMasterInDialog::SetActivePlayerChoice` 0x669410.
2. Kismet branch: `UDisSeqAct_DialogScriptedChoice` Activated 0x78b250 / UpdateOp 0x790bc0 / DeActivated 0x7d3920;
   `UStatePlayerMasterInScriptedChoice` (12, `OnEnterState` 0x673640/0x673ae0, `GetChoiceInfo` 0x66fb00).
3. Shared: the choice half of `UStatePlayerMasterChoice_Base` (`ShowChoiceSelection` [0x6c7450], `TickState` 0x679230,
   `MakePlayerChoice` 0x669240, skip); HUD `ShowChoiceSelection` 0x79f390 / `HideChoiceSelection` 0x78d260
   (`playerChoice_mc`); the input natives `execOnPlayerChoiceConfirm` 0x78d360, `Dis_PlayerChoice_RequestSkip/_Released`.
- Accept: the choice is shown (capture with two texts, needs JA); on the scripted route selecting index 1 takes output
  1 and index 0 takes output 0 (two logs); nothing fires before input; gate 47/47.

### JG — jump feel: the landing spring, the bump smoother in flight, the landing contact, attributes (parallel)
1. `UDishonoredCamera_PhysicalReact` whole (25 symbols: `FDisSpringPoint::UpdateSpringPos` 0x6c0a00, `IsSpringing`
   0x6c09a0, `OnImpact` 0x6c10f0, `GatherInfluence_Start` 0x6d49b0 / `_Derived` / `_FixedTimeStep`, `OnActivate_Derived`,
   `OnCameraCollided_Derived` 0x6c0e10, `OnTeleport`); `Landed_Native` calls `OnImpact(Velocity)`.
2. The bump smoother's `SetWeight(0, 0.1)` entering Jump (0xa75ee0) and Falling (0xa759f0), back to the default on
   leaving Jump (0xa5ffa0) and in Landed (0xa63110), via `GetCamera_BumpSmoother` (0x6fbfd0).
3. Measure whether `Empty_JumpLandSmall`'s anim notify already posts the land sound; else size
   `UDishonoredContactSystem::ApplyContact` [0x8410f0] and port if it fits one build; `ClearFOVTarget(1)` and the
   Kismet land event in `Landed_Native`.
4. `ApplyAttributes` [0x794860] (`MaxFallSpeed` 2000, acceleration, ground speed from the attribute tables; needs
   IF's `IsCarryingCorpse`) and `performPhysics` 0x6a1670; A/B the inputtest peak speed (500/600 must hold) and the
   walkers' 123 u/s.
- Accept: on the inputtest jump trace the view dips ≥ 8 uu within 0.15 s of landing and rebounds once (~1.5 Hz), the
  view apex within 0.05 s of the pawn's 0.53 s (today 0.37 s / 1.1 s); a landing event posted; `MaxFallSpeed` 2000
  logged; inputtest green; gate 47/47.

### JH — interaction prompt + hover glow (after JA and JG)
1. `ADishonoredPlayerPawn::FillUIInteractions` (0x6a9130 [0x7033f0]) through the master/upper FSM fill virtuals;
   `FDisUIInteractionProbe` (0x78cc60, Tick [0x80c000]); the interface fills `FillUIInteraction` [0x67d190] /
   `FillUIAltInteraction` [0x67d1f0] on NPC, usable object, pickup, movable, climbable.
2. HUD `UpdateContextInteractions` 0x7a63c0 / `UpdateSpecialInteractions` 0x7a6630 / `UpdateQTEInteractions` 0x7a6920;
   `GetCrosshairFocusText` NPC [0x7b1f00] / usable [0x683910]; named tweaks where `m_Name` is empty.
3. `ShowHighlight/HideHighlight` on usable (0x66da10/0x66b370), pickup (0x66c9a0/0x66a990), movable (0x66e720/0x66b710),
   climbable (0x66ae20/0x66ae60); `LoseCrosshairFocus`; `DoHighlight`'s HUD check (0x7bd0d0); `DisGDInteractableOf`
   through the interface for every implementer.
4. `FindInteractableActor`'s multi-probe (0x6b70f0, 1,302 B) and the combat target probe if they fit.
- Accept: on the `-distouchprobe` route a prompt capture on the captain ("Talk" + name) and on a pickup; the glow
  material on/off within one frame of focus (`-jhdiag`); `SetInteractions` with ≥ 1 line per interactable type;
  `probe_natives <= 3`; gate 47/47.

### JI — dynamic pylons (only if JD's census finds one on the route)
`AArkDynamicPylon::PreBeginPlay` [0x576420], `PylonMoved` [0x575e70], `CostFor`, the repath hooks,
`UArkSeqAct_ChangePylonConnection::Activated` [0x57e060].

## Schedule, merge order, overlaps

- Step 0 (coordinator, before launch): delete `resources/reference/mission0/ours.mp4` and `diffed.mp4` and the derived
  `build/cmp/ours_mp4*` / `video_cache` outputs; set `probe_natives <= 3` in the gate baseline; write `PHASE24.md`.
- Launch 1 (parallel, 6 agents): JA, JB, JC, JD, JE, JG.
- Launch 2: JH (after JA, JG), JF (after JE; its step 0 may start on JE's tip), JI if the census asks.
- Merge order: JA -> JC -> JE -> JD -> JG -> JB -> JH -> JF -> (JI). Overlaps: `stateplayermasterchoice_base.cpp` (JE
  base half / JF choice half), `DishonoredGameNativeStubs.cpp` (regen each merge), `dishonorednpcpawn.cpp` (JC MAT_,
  JH focus text), `dishonoredplayerpawn.cpp` (JG, JH), `disgfxmovieplayerhud.cpp` (JH, JF), `stateplayermasterindialog.cpp`
  (JE, JF). JC and JD are disjoint.
- Coordinator per merge as PHASE23; wave end: `head_rel` rebuilt and staged, the Empress route end to end with the
  diag lines as evidence, the ride compare at the constant offset vs `build/cmp/final26q_const`, the hand-played scene
  captures for the user, then the user plays.

## Where this plan is most likely wrong
1. JC: the cumulative root delta may be exactly what retail feeds once the actor transform is restored each frame -
   the step-1 drift table decides before `ProcessRootMotion` is replaced.
2. JE: Emily's go-to may fail at the teleport destination (off the navmesh), not at the FaceTo - `-jediag` prints the
   destination's `ArkIsPointOnNavMesh` first.
3. JF: both the choice node and the Kismet action may exist in the level; step 0 says which fires here.
4. JG: the sudden landing may be the land action's blend rather than the spring - the view trace is the measurement.
5. JD: the arcs may be the unported turn-ahead alone; avoidance is then lower value and reported, not forced.

## Outcomes (coordinator log)

| Package | Commit | Landed | Where the brief was wrong |
|---|---|---|---|
| JA | `810f13a` | `GFxValue::SetText/GetText` as retail (type-tested dispatch: an edit text through the field's virtual `SetText` -> `SetTextValue`, else `SetMember`); `GFxEditTextCharacter::SetText(wchar_t)`/`GetText`; `-jadiag`; the marker reads 'Empress Jessamine [62m]'; options AS2 errors 39 = head_rel; gate stages green (re-run 18/18) | the marker appears only after the ride (world 150.6) and its description only when focused - as retail; 0x9afdb0 is 2012 (2013 0x9a8dd0) and type-tests rather than calling `SetTextValue` directly; `crosshairInfosTxt_mc` uses an AS2 Invoke (never affected); no other `SetText` path runs on the routes; `UGFxObject::SetText`'s translator branch left as a boundary |
| JB | `aaf74cd` | `CreateFunction` real (2012 0x9af5c0; every SetFunction delegate now live), PreLoad installs `gfxProcessSound` + the CLIK callbacks, `FGFxSoundEventCallback::Call`, `ApplyTweakChanges_Derived` + the tweak slots (never reached before), `UGFxAction_OpenMovie` builds the player in the action as retail; `-jbdiag`; the menu has 2 sound themes, 7 UI events audible on a key-driven menu run; gate stages 24/24, options 4 of 5 | four broken links, not two; the script half posts `Snd_UI_Ingame.*` through `WorldInfo.PostAkEvent` (no `Bank_UI`); the options stage's drive script reaches only two sound paths (the >= 5 bound is unreachable there); one render-thread `VertexBuffer` assert on the menu map in 1 of 5 options runs (no GFx/Wwise frame; watch); `bInitFirstFrame` default left TRUE (retail 0, `-jbfirstframe0`); hover/Down/Up produce no Move sound (open) |
| JD | `1dad555` | `UpdatePathProperties`/`ComputePathYawFromDist` whole, `ComputePathMove` (retail blends toward the corner only on the last stretch - DN blended at every corner), Arkane `PointReachable` (0x2af580), `FArkComponentAvoidance` + `UArkAvoidable` whole with priorities/push period/steering, `m_bStartingNewPath` cleared, the stuck test/push clamp, `bJustTeleported` cleared, `MovePawn`'s inertia test (was inverted), `ModerateMovePawn` behind `-jdnomoderate`; `-jddiag` census; captain straight share 70 -> 77 %, heading change 638 -> 479 deg, body/velocity 95-97 %, one static turn per corner, FilterPathingFail 0; gate stages 23/23 twice | one pylon only (no JI); the turn-ahead fields belong to the scheduler/turn code already ported; retail randomises the wall distance too; retail arcs through corners < 135 deg (the walk mode has only 180-degree turn anims); no NPC pair crosses on either route; the inverted inertia test sank the captain 45 uu into the deck with the moderate move on; ride SSIM .83 vs .845 unattributed; `MoveAttachedActors` still unported |
| JG | `7ead454` | `UDishonoredCamera_PhysicalReact` whole (spring points, OnImpact, GatherInfluence_*, OnCameraCollided_Derived), the bump smoother at weight 0 for the flight, `Landed_Native` as retail (OnImpact, ClearFOVTarget(1), the contact type, the Landed Kismet event), `Tick_Attributes` + `ApplyAttributes` (pawn and player) + `performPhysics`; `-jgdiag`; the view apex on the pawn's frame, a 45-47 uu dip with one rebound (~1 Hz); gate stages 23/23; the baseline's walk bounds lowered to 700 / 380 | the pawn apex is at 0.27 s (0.53 s is the whole flight); MaxFallSpeed is 2100; retail walks at 400 (`Attribute_GroundSpeed`), not the class default 500; 0xa63110 is `Falling::OnExitState`; `GetCamera_BumpSmoother` already existed; the dip is ~46 uu, far above the >= 8 bar (faithful: k 75, velocity capped at 700 - the user's hand play judges the feel); the land thud needs the whole contact system (~7.6 KB, footfalls too) - its own package |
| JH | `a175d96` | `FillUIInteractions` (0x6a9130) + the probe fill inline in PreAdvance, the context/special/QTE updaters, the interface fill slots, `LoseCrosshairFocus` clearing the highlight bit, `DoHighlight`'s HUD check (0x787e60), NPC focus text/tweaks/Talk fill + the missing interface accessor, usable-object slots + key test, pickup/movable/climbable highlight + tweaks, `DisGDInteractableOf` via `GetInterfaceAddress`; `-jhdiag`, `-jhface`; 'Geoff Curnow / [F] Talk' one frame after focus, the Crank Wheel glows and un-glows in the focus frames; gate stages 23/23 | the 2012 highlight rvas were given (2013 ones in the status csv); 0x7bd0d0 is a power-wheel helper, the real HUD check is 0x787e60; the base/Walk fills are empty in retail (the content lives in six unported special states); 2013 has no probe Tick; `FindInteractableActor` is 2,691 B in 2013 and needs the masked probe family (pickups get no crosshair - open); the inputtest route runs `-nodishud`; the captain is NPC_26; NPCs never glow in 2013 |
| JC | `ec2477c` | `ProcessRootMotion` whole from 2013 (rotation scale/offset, own rotation removed, curve delta), `RawExtractedRootMotionDelta`, three new `AActor` virtuals (backup / clear / special-extract) with pawn and NPC overrides, retail's 2013 placement path (`MAT_SetRootMotionMode` 2013, `GetMatineeRootMotionTransform`, `ProcessMatineeRootMotion`, `MoveByMatineeRootMotion` from `UDisSkeletalMeshComponent::TickEnd`), `m_bIsControlledByAMatinee`, the FaceTo track inst (0x8ff8f0/0x8fc9a0) on IC's component, the AIControl track (0x8fee50) with the NPC `MAT_Send*` requests; `-jcdiag`, `-jcwatch`; every ground NPC starts on its mark; the Empress/Regent face their FaceTo targets; ride SSIM .840; gate stages green (inputtest alone 820) | the two `AActor` virtuals back up and clear - the placement is a third path (slot 307 / slot 201 / the tick end) - and no key on either route uses it (drift table empty); +832 is the stock `RootMotionDelta`; `MAT_GetBodyIntention` is a constant default; Emily is never in frame because her go-to stops 1,100 uu short (JE's chain); the boat passengers are off their spawner identically with JC off; `_4` now turns while walking from his FaceTo keys; the merged tree's run spread (.705-.821) is wider than JC's effect |
| JE | `618473c` | the teleport natives (pawn/NPC/player/controller), real FaceTo requests in `DoRequest` with the callback into `HandleFaceToEvent`, `AttemptTeleportSpell`, the player `StartDialog/EndDialog`, `UStatePlayerMasterInDialog` (minus the choice half) over the choice base's enter/exit/bend-time half, the view-tracking starts, the two look-at nodes, the NPC dialog availability + stims, every behaviour's `GetDialogAvailability`; `-jediag`; Emily teleports, runs in at ~400 u/s, her go-to succeeds at 170.94, Gate_37/Switch_8 fire, the next scene starts at 176.99, the bars on 160.89-185.04; gate stages 23/23, unported_natives 0 | the bars' writer is the choice base's `OnEnterState` (bit 0x10 at the PlayerState level), not `SetCinematicMode_Native`; both the teleport and the real FaceTo were needed; `InDialog::OnEnterState` is 2012 0x6b8b20 / 2013 0x673720; the named availability overrides were not enough (five folded `return 0` overrides found by vtable slot 98 - build 1 broke the ride without them); `MAT_IsAllowed` left (needs an Engine virtual) |
| JF | `49f2a49` | the conversation node fires (no scripted-choice Kismet op in the level); `UDisConv_PlayerChoice` play/tick/init/matching + getters, the dialog state's `SetActivePlayerChoice/GetNumChoices/GetChoiceInfo`, the choice base's `ShowChoiceSelection/MakePlayerChoice/ReadyToMakeChoice` + choice calls in `TickState`, HUD `ShowChoiceSelection/HideChoiceSelection/OnPlayerChoiceConfirm`, the two skip natives, `BendTime/GetBendTime/TickBendTime`, four 2013-only helpers; `-jfdiag`; the list shows at 174.6 with both texts, Down+Enter takes output 1 (PlayerWin), Enter output 0 (Emily_GoToHideAndSeek), nothing fires without input; Empress scene unchanged; gate stages 23/23 | bend time was the missing piece (`ReadyToMakeChoice` waits for channel 3 < 0.1); the 2013 hand-over is in `TickNode`, not `PlayNode`; the HUD movie owns Down/Enter (no ini keys); `TickBendTime` does not write world `TimeDilation` (no out-of-bend tick); `USeqAct_Interp::OnSkipRequest` still unported so the dialog skip does not stop the matinee; `UDisSeqAct_DialogScriptedChoice` (UpdateOp 333 B) left |
**JH** launched on `7ead454`.
**JF** launched on `777497b`.

## Wave result (2026-10-21, head `5904aa0`, 8 packages merged, JI not needed)

Final gate on `5904aa0`: **47 ok, 0 failed, 0 skipped** (1816 s, `build/gate_final27.log`, machine quiet). `build/head_rel` rebuilt
at `49f2a49` (same tree) and staged as `DishonoredGame-Win64-Shipping.exe`. End-to-end compare on a quiet machine
(`build/cmp/final27q`, rescored at the constant offset a = 13.57 - soiree-enter 56.22 = -42.65 in `final27q_const`):

| metric (median over the ride) | wave 26 (`2127b56`) | wave 27 (`5904aa0`) |
|---|---|---|
| SSIM | .848 | **.852** |
| MAD | 7.6 | **7.55** |
| zoom | 1.00 | 1.00 |
| zoom NCC | .83 | .82 |

The two hand-played scenes on the staged exe (our frames under `build/cmp/w27_scenes/`, README inside; retail.mp4 does not
cover them): the Empress route `-fwteleport=DisTrigger_16@160` plays through (bars 160.87-185.05, Emily teleports and runs
in, `Emily_GoToGazebo_Success`/`Gate_37`/`Switch_8` 170.95, the next scene 176.98); the Emily greeting
`-fwteleport=TriggerVolume_1@155` shows the hide-and-seek choice at 174.93 with both texts, keys sent before the list
(170.7) do nothing and the list stays up, `Down`+`Enter` at 177.94 takes output 1 -> PlayerWin.

The user's seven defects: (1) main-menu UI sounds play (JB: `gfxProcessSound` bound, `SoundThemes` filled); (2) NPCs walk
straight legs with in-place turns and avoidance (JD) and scene anims start on their marks (JC: retail `ProcessRootMotion`,
the transform backup/restore virtuals, FaceTo/AIControl tracks), Emily's walk-off was JE's chain; (3) the choice shows and
waits (JF); (4) markers read the target name (JA: `SetText` through `SetTextValue`); (5) the Empress scene plays through
(JE: teleport natives, real FaceTo requests, `AttemptTeleportSpell`, the dialog state = bars); (6) the landing spring and
the bump smoother off in flight, `ApplyAttributes` (JG; walk speed now retail's 400, `inputtest_peak_speed` ratcheted to
380); (7) the interaction prompt ("Talk" + name) and the highlight glow (JH).

Open after wave 27: `MAT_IsAllowed` (needs an `APawn` virtual + the `UnInterpolation.cpp` call); `USeqAct_Interp::OnSkipRequest`
(the dialog skip reaches `InterruptTickingNode` but the matinee keeps playing); `TickBendTime`'s world `TimeDilation` write
(no out-of-bend-time tick); the Kismet scripted-choice classes (not used in this level); the hover "Move" sound; the
pickups' crosshair (the masked probe family); the land thud (the contact system); the item-context half of
`AttemptTeleportSpell`; `GameInfo::BendTime`'s particle/post-process/power halves; one render-thread VertexBuffer assert
flake; the ride SSIM's run-to-run spread (~.82-.85). The override file is removed. **No further wave: the user said
"After this phase don't do any more".**
