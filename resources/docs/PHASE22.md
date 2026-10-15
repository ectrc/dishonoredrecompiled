# Wave 25 — the opening cutscene 1:1 with retail (packages HA..HH)

> Tracker of record for the real wave 25 (2026-10-14). `PHASE21.md`'s GL..GO wave was never run and is void.
> Source: the user's `resources/reference/mission0/{retail,ours,diffed}.mp4`.


## Context

The user recorded the first ~110 s of a New Game on `build/head_rel` (`ours.mp4`) beside the retail game
(`retail.mp4`) in `resources/reference/mission0/`. The ask: make the difference practically nothing.
STATUS.md / PHASE21.md's "wave 25 running (GL..GO)" is stale and is NOT this wave: no GL..GO reports exist. This
wave is written as `resources/docs/PHASE22.md`, packages **HA..HH**, agents on **Opus 5.5** (`model: opus`).
HEAD `9faefc0`; the working tree has uncommitted SLOMO/GHOST edits in `UnPlayer.cpp`/`UnController.cpp` — leave them
alone, packages branch from HEAD in their own worktrees.

Frame pairs (top retail / bottom ours, ffmpeg at 2..110 s, scratchpad `frames/pair_<t>.png`):

| t (s) | retail | ours |
|---|---|---|
| 5 | full-screen briefing page: "DUNWALL TOWER", "RETURNING HOME", paragraph, "PRESS ANY KEY TO CONTINUE" | menu-map backdrop |
| 8 | sky/clouds shot | black + health/mana vials + on-screen engine warning line (`MIC::SetScalarParameterValue ... PlayerArms_Tatoo_inst`) |
| 10–14 | black with centred "WRENHAVEN RIVER" banner | black with vials; boat fades in ~4 s early |
| 16–60 | camera low, FOV narrow; captain close on the left leaning on the hull, then gesturing; tiller guard on deck facing forward; "Use your mouse to look around" hint | camera high/far, FOV very wide; captain far away in A/T-pose ~40 s; guard standing ON the engine housing, arm out; vials top-left; no hint |
| 70–100 | captain faces AWAY, hands behind back | captain faces camera, hands on hips |
| 104–110 | ride ends looking forward; "NEW OBJECTIVE / REPORT TO THE EMPRESS" popup; "Geoff Curnow [Mouse Scroll Down] Talk"; white chevron on an NPC; "Press [..] to move" | view ~90° left of heading; crouch icon bottom-left; no prompts |

Note: the red/blue "sword glow" top-left in ours is the **health/mana vials** (same shape over pure black at t=8), not
a drawn weapon. GJ measured retail's New Game gives nothing to equip. So "arms" = HUD, package HE.

## Root causes (exploration + design agents; read-only, nothing edited)

1. **FOV, confirmed in code.** `Engine/Inc/EngineClasses.h:19638` declares `ULocalPlayer::AspectRatioAxisConstraint`
   as a storage-less shim that always reads 0 = `AspectRatio_MaintainYFOV` (retail has no such member:
   `reference_member_delta.md:3468`, Epic added it after 9014). `UnPlayer.cpp:4519-4544` (+ copies :4875, :4996) then
   applies the 75° FOV **vertically** → ~107° horizontal at 16:9, a factor of exactly 16/9. FZ's "75 in both builds"
   measured the number, not the projection; FE's 61±2° menu fit (90° h → 58.7° v) was the same fault.
2. **T-pose / tween: the stack stand-in, not the track.** Retail's `UInterpTrackInstStretchAnimControl::UpdateTrackInst`
   (2012 `0x543f40` / 2013 `0x505c70`, decompiles in `build/agentFS/decomp2012|2013/`) also sends `NAME_None` every
   frame after a key plays out — our track is right, the comment at `interptrackstretchanimcontrol.cpp:404` is wrong.
   Retail's `UArkAnimNodeStack::BuildEdgeAnimTreeEx` (2012 `0x576920`, 376 B) **skips any channel whose sequence has
   `AnimSeq == NULL`**; our `GetBoneAtoms` stand-in (`arkanimnodestack.cpp:88-150`) blends the reference pose in at the
   channel's weight (`:124` mask only with a sequence). GJ3.log: captain `ch1 SlowDown w1 | ch3 None w1` → ch3's ref pose
   covers the lean = the 40 s T-pose; a decaying weight on `None` = the slow tween. Also the flagged child-0 complement
   (`:236-242`, agent FS) drops the base tree to weight 0 (frozen, never ticked; stays 0 after the scene), and channel
   root motion is computed and dropped (`:101` vs `:120`).
3. **Rotation / position.** Retail carries the boat guards on the boat's `Captain`/`Driver` sockets through
   `FDisComponentRetargeting` (`UpdateRetargeting` 2012 `0x8b37e0`; `Sources.cmake:111` excluded); ours only unbases
   them in `MAT_Attach` and GA's floor sweep drops the Elite on the engine housing. NPC `physicsRotation` override and
   `Tick_Rotation` unported (stock `APawn::physicsRotation` turns the pawn to its controller); `UInterpTrackLookAt::
   StaticUpdateTracks` (2012 `0x547010`) never called (`UnInterpolation.cpp:13340`); `FArkComponentFaceTo` and
   `UInterpTrackInstFaceTo::UpdateTrackInst` (2012 `0x53eb90`) bodiless; NPC `MAT_BlendOut` (2012 `0x7ad5e0`) and
   `UArkAnimNodeBlendPose` tick/capture unported. NPC locomotion ticks only from `physWalking`
   (`dishonorednpcpawn_locomotion.cpp:115-138`), so a scene pawn in `PHYS_None` never advances a go-to.
4. **HUD visible.** The HUD movie is closed+reopened on map change (`disglobaluimanager.cpp:376-415`) but
   `UDisGFxMoviePlayerHUD::PostStart` (:1084) resets only `m_bPlayerStatusVisible=FALSE`, `PreClose` (2012 `0x811e40`) has
   no body, and `Tick_PlayerStatus`/`Tick_PlayerState` send `Open/Close`/`SetIcon` only on a cached edge → the fresh
   movie is never told to close (user log `play_w24e.log`: reopen at 68.42 s, next `masterHUD_mc` call at 169.50 s).
   `DisSeqAct_ToggleHUDElement::Activated` (2012 `0x7f75b0`) skeleton although five fire at level load.
   The on-screen warning is `UMaterialInstance::CheckSafeToModifyInGame` (`MaterialInstance.cpp:1724-1740`, 2012
   `0x118b00`, no 2013 match) + `GAreScreenMessagesEnabled` defaulting TRUE.
5. **Missing UI.** `UDisSeqAct_ShowLocationDiscovery::Activated` (2013 `0x7998a0`, 42 B) and
   `UDisGFxMoviePlayerHUD::ShowLocationDiscovery` (2012 `0x7fb080`) bodiless, clip `_root.location_mc` bound;
   `Tick_ObjectivePopup`, `Tick_Tutorials`, `Tick_Interaction`, `Tick_Markers`, `Tick_Subtitles` unported
   (`disgfxmovieplayerhud.cpp:1211-1218`). The briefing page is the **Bink loading movie** `LoadingEmpressTower`
   (`DefaultEngine.ini:149`) with text overlays placed by `DishonoredUI.ini:39-53`, owned by `UDisBinkOverlayManager`
   (16 functions, all unported); Bink is `DISHONORED_WITH_BINK=OFF` and its global shaders undeclared.
6. **No measurement tool.** Nothing compares a run to `retail.mp4`. ffmpeg 8.1:
   `C:\Users\Frederik\AppData\Local\Microsoft\WinGet\Packages\Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-8.1-full_build\bin\ffmpeg.exe`
   (not on PATH); Python 3.13 has numpy + pillow. Route: `-finewgame=50 -fosoiree` (ride = world 64–158 s; GB's runner
   `build/agentGB/gb_run.py`).

## Packages

Shared rules in every agent prompt: Opus 5.5; own worktree via `make_snapshot.py <X>` (build Release, not the
Debug default); builds only via `build_queued.py`, **two per package**; runs only via `game_input.py launch` with
`-windowed`; never real input/focus; no retail `Dishonored.exe`; **never fmodel MCP**; IDA only on a private copy
`build/agent<X>/idb/`; resolve every address by hand in both dbs (2013 VA = rva + 0x400000); never hand-edit
`Inc/CppText/*.h`, `DishonoredGameRegistrants.cpp`, `Sources.cmake` (run `gen_classes_header.py DishonoredGame --sdk
--module-header --sources-cmake`); report `agents/agent<X>.md` + `agent<X>_status.csv` + `build/agent<X>_sync.py`;
"assume this plan is wrong somewhere and say where, with the measurement".

### HH — `resources/tools/cutscene_compare.py` (tool, first; no game code)
- `run`: `game_input.py launch --exe <exe> --log CC_<tag>.log --tag cc_<tag> -- -windowed -ResX=1920 -ResY=1080
  -finewgame=50 -fosoiree -apshottime=<50..175 step 1|2>`; cmd file `wait 185`/`quit`. Reject non-16:9 BMPs. Read real
  world times from the `-apshottime: screenshot i of n ... world time w` line (`SceneRendering.cpp:5136`) and anchors
  (`soiree ENTER/EXIT`, `SetBlackStripes(1)`, HF's `ShowLocationDiscovery`). BMP→PNG, delete BMPs. `--ours-video` accepts
  an mp4 (OBS sessions, Bink phase).
- retail: cached 10 fps 960x540 decode under `build/cmp/retail_cache/`.
- Alignment: E1 banner-on (banner box x .36–.64, y .24–.34 bright, rest black), E2 fade-in done; fit retail_t = a + b·world
  (b≈1); fallback `soiree ENTER` vs first boat frame.
- Metrics (numpy): blurred MAD, SSIM-lite, **zoom scale** (NCC over s∈[0.5,2.2], FE's method), per-area pixel detectors:
  `vials`, `stance`, `debugtext` (cyan), `banner`, `objective`, `hint`, `prompt`.
- Output `build/cmp/<tag>/`: `pairs/`, `diff/`, `scores.csv`, `report.md`, `index.html`; `--baseline` prints before/after.
- Accept: on `head_rel` reproduces today (median zoom 1.7–1.85, `vials`>0 during black, `debugtext`>0 ≈ +8 s, `banner`=0);
  retail-vs-retail SSIM ≥ .99 scale 1.00; two runs same exe MAD within ±5 %.

### HA — pose: the stack (lands first; unblocks HB/HC measurement)
1. `UArkAnimNodeStack::GetBoneAtoms` rewritten as a CPU transliteration of `BuildEdgeAnimTreeEx` (2012 `0x576920`;
   2013 via the `BuildEdgeAnimTree` vtable slot, confirm bytes): child 0 always the base, weight ignored; channel used iff
   `Weight>0`; **sequence channel with `AnimSeq==NULL` skipped**; non-sequence child = subtree; NULL child = ref pose
   (verify `UAnimNode::BuildEdgeAnimTree` `0x5782f0`); additive accumulates, else per-bone lerp keeping FT's
   `m_bDontUseRefPoseOnNonDefinedBones` mask; root motion follows the branch (`RootMotionDelta` blended, `bHasRootMotion |=`).
   Do NOT port the whole Edge tree (8–12 days, AK memo `edgeanim.md` §7.2). Update the STATUS pitfall text.
2. `MAT_SetAnimWeights`: delete the child-0 complement (`:236-242`); retail (2012 `0x576790`/2013 `0x534f30`) writes channels only.
3. `ADishonoredNPCPawn::StopRootMotionMode` override (2012 `0x7af1a0`, 47 B; 2013 resolve) — base runs `RMM_Velocity/RMRM_RotateActor` every frame.
4. Second build only: 2013 StretchAnimControl refinements (agentFS.md §6.3, `decomp2013/va_905c70.c`, `va_934cb0.c`,
   `va_62fed0.c`, `va_6b53a0.c`): previous-position float through `MAT_SetAnimPosition` (notifies fire), `SetActorRootMotion`
   only on forward movement, reverse-playback test, `m_bSpecialRootMotionExtract` (`sub_612E40`), `m_bShouldBackupTransform` (`sub_61FCB0`).
- Files: `Engine/Src/arkanimnodestack.cpp`, `interptrackstretchanimcontrol.cpp`, `UnPawn.cpp`, `UnInterpolation.cpp`,
  `DishonoredGame/Src/dishonoredpawn.cpp`, `dishonorednpcpawn.cpp`, `Inc/CppText/ADishonoredNPCPawn.h` (generated).
- Diag: extend `-fkdiag`: `stack eval base | ch<i> w lerp|add|skip(noseq)|skip(w0) | rm`, `aipawn w0 <> refdev <deg>` (mean
  arm-bone angle from ref pose).
- Risk: FT's BlinkBlender is a stack (its channel has a sequence, unchanged); player groups (FZ soirée, GB ride) call
  `MAT_SetAnimWeights` → re-shoot `fz6`/`gb2_a` frames; step 4 touches the player `MAT_SetAnimPosition` signature.
- Accept (route `-finewgame=50 -fosoiree -fkdiag -frdiag -gadiag -diskismet`, shots 70/85/100/115/130/145/157 s, control
  `head_rel`): `_26`/`_27` `refdev`>10° every sample 64–158 s (control ≈0 until ~104 s); `w0 1.00` always; `skip(noseq)`
  on the captain's None channels; WorkerB key 0 `rootMotion 1` → `rmd`>0 and `_20` moves; HH zoom-independent SSIM up
  over the ride; gate 47/47, 0 Critical.

### HB — blend in/out + LookAt track (parallel with HA; merge after)
1. `UArkAnimNodeBlendPose::InitAnim/TickAnim/SetActiveChild` (2012 `0x1c3040`, `0x550080`, `0x550360`,
   `build/agentCG_decomp/s12/`; 2013 by vtable) in `arkanimnodeblendpose_dis.cpp`; CPU `GetBoneAtoms` of its
   `BuildEdgeAnimTree` (2012 `0x55dd40`): live child 0 lerped with captured `m_PreviousPose`.
2. Writer of `ADishonoredNPCPawn::m_pBlendPoseNode` (@3456; resolve native xref vs script `FindAnimNode`), `PostInitAnimTree` (`0x7ad200`).
3. `MAT_BeginAIGroup`: `MatineeControllerAdd(Seq, m_AIBehaviorPriority, m_bIsBlocking)` then `BlendNow(0.2, TRUE)`;
   `MAT_FinishAIGroup`: `MAT_BlendOut` (`0x7ad5e0`: GetBlendOutTime>0, not Thrown/Assassinated/Choked, Physics≠10 → BlendNow)
   + `MatineeControllerRemove`; fix the banner at `dishonorednpcpawn.cpp:350`.
4. `UInterpTrackLookAt::StaticUpdateTracks` (2012 `0x547010`, 3,756 B; 2013 `0x5082c0` confirm) called from
   `UInterpGroupAI::UpdateGroup` (`UnInterpolation.cpp:13340`); `StartLookForward(.., GetGameLookatPriority(..), 0.5, -1)` on
   `LockRequests` in Begin, `StopAllLookAtFromAsker` in Finish via FT's `FArkComponentLookat` API (`arkcomponentlookat.cpp:489-611`).
- Diag: `fkdiag blendpose <pawn> BlendNow <t> captured <n> w c0/c1`, `fkdiag lookattrack <group> key -> target prio req`, `-flheaddiag`.
- Risk: the player's `m_pMatineeBlender` is the same class → re-shoot FZ frames; GH's `Failed, with animsequence` must stay 0.
- Accept: one `BlendNow 0.20` per bound NPC (~63.7 s), one `MAT_BlendOut` per group (~158 s), no pop in captures at bind
  and 157–158 s; look-at requests cover crossed keys (captain 27/31, Elite 62/63), head yaw follows within 1 s.

### HC — position, rotation, movement (parallel; merge after HA, HB)
1. `FArkComponentManager` ctor/dtor/`Tick`/`GetPolicy<PreAsync|PostAsync>` (2012 `0x57afa0`, `0x579cf0`, `0x57b070`, `0x765610`;
   2013 + `UWorld::Tick` call site resolve), `UWorld::m_pComponentManager` (@712).
2. `FDisComponentRetargeting` whole (2012 `0x8af8b0`, `0x8d2400`, `0x8b37e0`, `0x8d0eb0`, `0x8d1100`, `0x8d2330`, `0x8d23c0`, `0x8d10f0`;
   decompiles `build/agentGK/decomp2012/`); off the exclude list via the generator.
3. Wiring: component creation (resolve `PreBeginPlay_NativeComponents`, `m_pCpntRetargeting` @1924); full `MAT_Attach`/`MAT_Detach`
   (`0x7ab5e0`, `0x7ab690`); retargeting tests in `MAT_SetCollision`/`MAT_SetMeshTranslationMode`/`MAT_DisableMeshTranslation`
   (`0x7ab7a0`, `0x7ab6e0`, `0x7ab720`); replace raw `+60` reads (`dishonoredpawn_body.cpp:242`, `dishonorednpcpawn_body.cpp:614`).
4. NPC rotation: `physicsRotation` override, `Tick_Rotation` (from `ADishonoredNPCPawn::Tick`), `Init_Rotation`,
   `CancelAllNPCRotations`, `FDisNPCRotationIntent::SetTargetRotation`, `SetNPCRotation(Focus)`, `Lock/Unlock/OnUnlockNPCRotation`
   (2012 `0x7c95f0`, `0x7d4d40`, `0x7c9570`, `0x7c95a0`, `0x7cbee0`, `0x7ce4a0`, `0x7cbf90`, `0x7cc020`, `0x7ce540`, `0x7cc0f0`) in `dishonorednpcpawn_rotation.cpp`.
5. Move `FArkComponentLocomotion::PreAsyncWorkTick` off `physWalking` onto the manager's policy as retail `Starting` (2013 `0x54ac70`)
   after confirming `MovePawn` callers (xrefs of 2012 `0x584ca0`). Revertable alone.
6. Stretch: `UInterpTrackInstFaceTo::UpdateTrackInst` (`0x53eb90`/2013 `0x4ff8f0`) + `FArkComponentFaceTo` (`0x57ab20`, `0x57dc00`,
   `0x57dcc0`, `0x577880`–`0x579920`, managers `0x576b60`–`0x57a9b0`), created in `FArkComponentLocomotion::SetConfig`. Else a follow-up package.
- Diag: `gadiag retarget <pawn> master socket anchor start|stop` + per-second offset/yaw; `gadiag cpntmgr`; `gadiag rot <pawn> yaw cur/intent prio`.
- Risk: player ride is `DoKismetAttachment` (unchanged); GA's sweep stops basing the two guards (as retail); GK's detach
  position changes (re-measure); GC/GK walkers under step 5 — measure Worker B's 850-uu walk, revert 5 if it regresses.
- Accept: `_27` within 2 uu / 1° of `Driver` and `_26` of `Captain` every sample 64–152 s; `_27` Z on the deck; captures:
  guard on deck at the tiller, captain faces away during the hands-behind-back segment at the retail times; WorkerB still
  walks 850 uu at ~123 u/s; gate 47/47.

### HD — projection/FOV, post-ride heading
1. Decompile `ULocalPlayer::CalcSceneView` 2013 `0x2c4710` (2012 `0x2df1c0`): expect the unconstrained branch =
   `FPerspectiveMatrix(fFOV*PI/360, SizeX, SizeY, GNearClippingPlane)` (horizontal). Replace `UnPlayer.cpp:4517-4545` and the
   copies in `DeProject` (2013 `0x2c5180`), `FastDeProject` (:4864-4895), `FastProject` (:4985-5016) each against its own retail
   body; delete the dead `AspectRatioAxisConstraint` shim (`shim_ratchet.py`). Compare the constrained branch too.
2. Arms projection (`dishonoredplayerskeletalcomponent.cpp:80-91`) already horizontal — verify one frame with the sword (`-gjgive/-gjequip`).
3. Post-ride heading: FZ saw yaw snap to −12736 (pre-ride controller yaw) at exit. Check in order: (a) last
   `UInterpTrackDisAllowPlayerLook` key's `m_bMatchControllerRot` and whether `m_bResetControllerRot` is raised when PlayerControl
   reactivates (`OnActivate_Derived` 2012 `0x721e30`, `ExitSoireeMode` 2013 `0x6628f0`); (b) `StartTrackingFixedAngles` (2013
   `0x6af6f0`) has no body and `dishonoredcamera_playercontrol.cpp:104` applies instantly — port; (c) pawn `Rotation` under the
   socket attach (`:116`). Fix whichever retail body differs.
4. Camera height only if a vertical offset remains after the zoom is right (FZ: bone z −703 at world 72).
- Diag `-pcamdiag`: FOV axis, M[0][0]/M[1][1], view size, controller/pawn/camera yaw around the soirée exit, reset flags.
- Risk: every 3D view narrows 1.78× (menu, scene captures, HUD markers' projection). Full gate + `newgame` + `options` stages; `inputtest_moved` unchanged.
- Accept (HH): median zoom 1.78 → 1.00±0.04 over 64–158 s and the menu frame; after `soiree EXIT` yaw within ±10° of the
  boat heading, `pair_w165` shows the captain ahead; body mode 1 (no arm mesh) during the ride.

### HE — cinematic HUD state, ToggleHUDElement, debug text, stance icon
1. Decompile retail `PostStart` (2013 `0x795650` ratio .687 — resolve; 2012 `0x7fb450`), `PreClose` (2012 `0x811e40`; 2013 via the
   vtable slot next to PostStart) and the head of `Tick_PlayerStatus` (2013 `0x79f6e0`); port which caches they reset
   (`m_bTickedOnce`, `m_PlayerState`, health/mana/equipment caches, `m_bCinematicMode`, `m_bPlayerStatusVisible`) and whether a
   `Close` is sent on the first tick; verify the `bBecameValid && m_HUDVisibility==1` forced-open (:686-693).
2. `UDisSeqAct_ToggleHUDElement::Activated` (2013 `0x791940`; 2012 `0x7f75b0`) in `disseqact_togglehudelement.cpp` (off the exclude
   list via the generator): Enable/Disable on mask level 4; log the five load-time activations with `-diskismet`.
3. `SetCinematicMode_Native` body stays; port `DisPushDisableSave/DisPopDisableSave` (2013 `0x7ec770/0x7ec7a0`, verify) only if small.
4. `Tick_PlayerState` (2013 `0x795ed0`): port the skipped gates (`IsPossessing` 2013 `0x74a550`, vtable +1660 test, `IsSliding`).
5. Debug text: show 2013 has no `CheckSafeToModifyInGame` call in the MIC setters (2012 `0x118b00` unmatched) and compile it out;
   `GAreScreenMessagesEnabled` default FALSE (`LaunchEngineLoop.cpp:3298-3303`) unless `-screenmessages`; grep tools for reliance.
- Diag `-phuddiag`: six mask levels, ANDed flags, `m_bPlayerStatusVisible`, every `masterHUD_mc` Open/Close, each ToggleHUDElement, `SetIcon` args.
- Risk: HUD must still open in normal play (vials appear when the tutorial enables them, retail ≈ t 110+); `options` stage HUD setting.
- Accept (HH): `vials`=0 from load to world 175, `stance`=0 from soirée exit to end, `debugtext`=0 in all frames; gate 47/47.
- Shares `disgfxmovieplayerhud.cpp` with HF in different functions (HE: PostStart/PreClose/Tick_PlayerStatus/Tick_PlayerState); second to merge rebases.

### HF — location banner, objective popup, tutorial hints, interaction prompt, NPC marker
1. `UDisSeqAct_ShowLocationDiscovery::Activated` (2013 `0x7998a0`): `DisGetGFxHUD()->ShowLocationDiscovery(m_LocationName,
   (bits&1)==0)` + `USequenceAction::Activated`; `UDisGFxMoviePlayerHUD::ShowLocationDiscovery` (2013 `0x795160`; 2012 `0x7fb080`)
   driving `_root.location_mc` with `m_fLocationDiscoveryDuration`, `m_fLocationDiscoveryVerticalAlignment`; log the sound event (audio deferred).
2. `Tick_ObjectivePopup` (2013 `0x7afe70`), `ShowObjectivePopup` (2012 `0x7fe8b0`), `OnObjectiveAdded/OnTaskAdded` (2012 `0x814f50/0x815290`),
   `FDisObjectivePopupInfo::AddTask` (2012 `0x802fd0`); trace what fires "REPORT TO THE EMPRESS"; stop at the HUD boundary if the objective system is unported, and say so.
3. `Tick_Tutorials` (2013 `0x7afa60` vs the code comment — resolve), `ShowTutorial` (2012 `0x811810`), `SetTutorialMessage`
   (2012 `0x8125a0`), `ResolveMarkupString` (2012 `0x80caa0`; produces "[Unbound]" exactly as retail shows).
4. `Tick_Interaction` (csv `0x7acb10` vs comment `0x78d3b0` — resolve), `FDisUIInteractionProbe`, `UpdateCrosshairInteractions`
   (2012 `0x8097d0`), `OnCrosshairActorSet` — measure whether it is ever called before porting more.
5. `Tick_Markers` (2013 `0x7b00c0`), `UpdateTaskMarkerInstances` (2012 `0x80cd40`), `PreRender_Markers` (2012 `0x8049d0`) — uses `ULocalPlayer::Project`, re-check after HD merges.
- Diag `-puidiag`: every Invoke on location_mc/objectives/tutorials/interactions/markers + Kismet activations.
- Risk: these ticks run every frame; guard each on its clip binding; `options_as2_errors` ≤ 39. Work 1→5, stop with a report where upstream is missing.
- Accept (HH): `banner`>0 aligned to retail t 10–14 (E1 residual ≤ .5 s); `objective`,`hint`>0 at retail t 110; `prompt`>0 at t 108; area SSIM ≥ .6.

### HG — mission briefing page + clouds shot (Bink; highest risk, last to merge)
1. Decode `Movies/LoadingEmpressTower.bik`, `Loading.bik`, `INTRO_LOC.bik` with ffmpeg; match against retail t 4–10 with HH's
   SSIM to identify the briefing background and the clouds (or a level Kismet sky shot via `-diskismet`).
2. `DISHONORED_WITH_BINK=ON` (`cmake/Bink.cmake`, `FDisFullScreenMovieBink` exists); declare/compile the Bink global shaders the cache skips.
3. Port `UDisBinkOverlayManager` (16 functions; 2012 rvas in the file header; 2013 `OnPlayLoadingMovie 0x7b60e0`,
   `OnRenderLoadingMovie 0x7b8b20` verify, `DrawTextOverlay 0x7b7130` verify): overlays from `DishonoredUI.ini:39-53` at 1280x720 coords.
4. Wire the two `DISHONORED(bringup)` calls in `UDishonoredEngine::PlayLoadMapMovie` (`dishonoredengine.cpp:190-196`); the press-any-key
   wait (`bShouldStopMovieAtEndOfLoadMap`, `MapsToAutoStart`); `-nomovie` stays the gate default until 5 clean runs.
- Diag `-pbinkdiag`. Accept: `--ours-video` overlay areas SSIM ≥ .7 vs retail t 4–6; "PRESS ANY KEY" waits for an inputcmd `key`;
  clouds frame matches at t≈8; `newgame` stage passes with and without `-nomovie`.

## Scheduling

- Launch 1 (one message, parallel): **HH, HA, HD, HE, HF** (5 Opus 5.5 agents). HH needs no build; others 2 builds each through the FIFO queue.
- Launch 2 after HA merges: **HB, HC, HG**. Merge order HA → HB → HC; HD/HE/HF as they land (HE/HF rebase on each other); HG last.
- Coordinator per merge: generator regen byte-identical, `bitfield_audit` 0, gate on a clean checkout in `build/gate_wt` (47/47,
  every `*_criticals` 0), HH before/after table vs `head_rel`, update STATUS/PHASE22/progress, commit per package (no self-attribution
  beyond the session's required trailer; never `git add -A`).
- Then rebuild `head_rel`, re-run HH end to end against `retail.mp4`, and open the next wave from the residual score table
  (expected residue: subtitles, the LOD manager, soirée AI stim, FaceTo if HC's stretch slips, audio deferred).

## Verification (end to end)

1. `python resources/tools/cutscene_compare.py run --exe <staged exe> --tag <X>` → `build/cmp/<X>/report.md` with `--baseline build/cmp/head_rel/scores.csv`.
2. Targets after the wave: median zoom 1.00±0.04; `vials`/`stance`/`debugtext` 0 everywhere ours is black or riding; `banner` at
   retail t 10–14; `objective`/`hint`/`prompt` at 108–110; captain `refdev`>10° throughout; `_26`/`_27` on their sockets; ride SSIM
   clearly above `head_rel`'s (number recorded in HH's first report).
3. `run_regression.py` 47/47 on each merge commit; `newgame` + `options` stages; `unported_natives` 0; `verify_phase2` 3/3.
4. Final: a fresh OBS-free capture series (HH) viewed side by side with `retail.mp4` frames by the coordinator before closing.

## Outcomes (coordinator log)

| Package | Commit | Landed | Where the brief was wrong |
|---|---|---|---|
| HH | `fcad6f8` | `resources/tools/cutscene_compare.py` (run/score/compare/selftest; zoom NCC, MAD, SSIM-lite, area detectors, event alignment); baseline `build/cmp/head_rel` zoom 1.76 = 16/9, SSIM .523, vials 125/126, banner 0 | the ride is world 56.3-150.6 (world time never resets at New Game; the plan's 64-158 were log times, and the offset varies per launch); `-apshottime` silenced screen messages after its first capture (fixed in `2683f3e`); the brightness detectors fired on the hull and sky (replaced by template / stroke detectors) |
| HE | `86193e5` | 2013 `PostStart` hides `masterHUD_mc` (ours hid `_dot_mc`), `PreClose` (2013 0x7af820), `ToggleHUDElement::Activated`, `Tick_PlayerState` gates (the crouch icon was the missing sneaking test), `UpdatePlayerInfoDisplay` un-inverted, cinematic save lock, `CheckSafeToModifyInGame` compiled out, screen messages off by default; vials 125 -> 0, stance 25 -> 0, SSIM .523 -> .537 | 2013 PostStart sends no Close; the save-lock addresses were 0x7ec820/0x7ec850 (FZ had the bodies); no ToggleHUDElement maps to 0xE00; the vials reappearing is not reachable on GB's route |
| HF | `94bdb00` | `ShowLocationDiscovery` (both halves; fires world 52-56 = retail's time, anchor line `HUD: ShowLocationDiscovery '<name>' sound <0|1>`), the tutorial system (`Tick_Tutorials`, `ShowTutorial`, `SetTutorialMessage`, 2013's 64-byte `FDisTutorialInfo`), `Tick_GameMessages`, the objective popup's HUD half, `FormatInteractionText`/`LocalizeKeyboardKeyName`, `InitGameActionBindings`, `SetCrosshairActor` -> `OnCrosshairActorSet`; gate 32/32 on the game stages | "[Unbound]" comes from `Settings.int` via `LocalizeKeyboardKeyName`, not `ResolveMarkupString`; `Tick_Interaction` is 0x7acb10 and is called from `OnCrosshairActorSet`, not `PreAdvance`; the banner draws "W W" over a box (a 25 px AS2 mask never reopens - HJ); the objective system is a skeleton (HI); `[W][S][A][D]` because `ReadPCBindingsFromProfile` is unported (HJ) |
| HA | `f147830` | `UArkAnimNodeStack::GetBoneAtoms` = `BuildEdgeAnimTreeEx` (2012/2013 byte-identical), `m_bIsAnimSeq` derived in `InitAnim`, the channel mask from the Edge joint list, the child-0 complement gone (w0 0.00 -> 1.00), NPC Start/StopRootMotionMode, 2013 track refinements; the captain walks arms-down at world 66 where head_rel stood in T-pose; refdev min 10.9 deg over 89 samples | WorkerB's Translate key cannot move him: the 47.5 uu sit in the Edge locomotion joint and our evaluator reads the root bone (HC); `0x5782f0` is a thunk, `UAnimNode::BuildEdgeAnimTree` is 0x19e6f0; no `_edgeAnimBlendAdditive` exists (op 1 = `_edgeAnimBlendJointsRelative` 0x613a90); refdev > 10 deg does not prove "no T-pose" on an A-pose reference |

| HG | `ccdc25c` | the Bink briefing page (`LoadingEmpressTower.bik`: "DUNWALL TOWER", hint, LOADING, then "RETURNING HOME", the wrapped briefing, PRESS ANY KEY which waits for a key) and the intro clouds (`INTRO_LOC.bik`); `UDisBinkOverlayManager` whole (16); overlay areas SSIM .83-.99 vs retail t 4-5.5, clouds .98; 5 clean runs with movies | the clouds are a movie, not Kismet; no shaders to declare; the reference player only treated memory-played movies as loading movies (retail 0x4fc720 uses the loop flag); nothing ever set the current chapter; the movie accepts keys only from controller 0, unassigned on the hand route (HK) - `DISHONORED_WITH_BINK` stays OFF until then |
| HD | `bcc7f4e` | `CalcSceneView`'s horizontal projection as retail 0x6c4710 (ride zoom 1.76 -> 0.99, SSIM .523 -> .592, MAD 29.4 -> 21.6); the `AspectRatioAxisConstraint` shim gone; `ADishonoredPlayerPawn::FaceRotation` (2013 0x6b0d40) - the post-ride snap to -12736 was this stub falling through to `APawn`'s; the view-tracking unit (`StartTrackingFixedAngles`, `TickCameraTracking`); `-pcamdiag` | 3a/3b were not the heading fault; the menu frame cannot be image-scored (FB's capture has a dialog over it and another camera); `ProcessViewRotation` 0x6afb50 still unported |
| HI | `33201ec` | the objective system whole (`UDishonoredObjectivesComponent`, objective/task classes, the `DisSeqAct_*Objective*`/`*Task*` actions and conditions, `UDisSeqVar_Task`, `OnObjectiveAction`), `UDisSeqAct_PlayerAction::Activated` (0x7d13d0, not in the brief - without it the action has no target), task markers (`dishudmarkers.{h,cpp}`, `Tick_Markers`/`UpdateTaskMarkerInstances`/`PreRender_Markers`); "Report to the Empress" added at world 150.6 and the chevron at x305,y160 vs retail x305,y157; objective detector 0 -> 3-4 frames at retail t 108.9-110.8; `-hidiag` | the chevron projects through `Canvas->SceneView->WorldToScreen` (2013 0x7bdd20), not `ULocalPlayer::Project`; the compare tool's two-anchor slope (0.94/1.06) moves retail's popup by +-6 s - the plan's "world 146-149" was that artefact, use the constant offset; the 2013 objective model differs from 2012 (template-name lookup, task IDs, profile setting 95); the observer loops are dead in retail; PlayerAction reaches four bodiless pawn natives (`OnToggle{Tutorial,AchievementEval,Choke,Journal}`) - `unported_natives` 0 -> 4, package HL |
| HC | `835dc2b` | `FArkComponentManager` + six policies ticking from `UWorld::Tick`; `FDisComponentRetargeting` whole (8); `MAT_Attach/Detach/SetCollision/SetMeshTranslationMode/DisableMeshTranslation` as retail; the ten NPC rotation functions + `FDisNPCRotationIntent`; `physWalking` = `MovePawn` only, locomotion on the manager; `TickAnim` keeps the matinee `PreviousTime` (root motion moves pawns: WorkerB 45.8/47.5 uu); `_27`/`_26` on `Driver`/`Captain` within 0.07 uu / 0.11 deg over 88 samples; ride SSIM .592 -> .690, MAD 21.6 -> 12.0 | retail's `physWalking` never ran the locomotion tick (manager does); retargeting registers in PreAsyncWork; root motion was never in an unread joint (all 36 skeletons have `locomotionJointIndex` 0 - the zero was `TickAnim` overwriting `PreviousTime`); retail ignores the captain's +94 X key offset; `StartRetargeting` locks then cancels the intents; socket yaw jumps +-180 (rolled frames). Open: Worker B walks 865 uu at ~95 u/s not 123 (`build/agentHC/revert_step5_loco.py`); FaceTo (step 6) not started; `FLocomotionState` half of `AnimLeafCallback` |
| HJ | `9da5dbc` | the banner reads "WRENHAVEN RIVER" centred with the ornament, on at world 52.0-56.0; banner-box SSIM .33 -> .889 (retail/retail .998); `GASPrimitiveObject::GetMemberRaw` (string.length), masks no longer drawn as content (`GFxDisplayList::Display` 0x9cbf20), text-field bounds 4 px (`GetViewRect` 0xa9f2c0, auto-size shrink 0xa9e3a0), `setImageSubstitutions` + per-context TextField prototype, `ReadPCBindingsFromProfile`, `TranslateBindingSet`, `UArkProfileSettings::SetToDefaults` (2013 0x53b8c0), `InitGameActionBindings` trim, `execFormatText`, `SetTutorialMessage::PostLoad`, `-puidiag`; route AS2 errors 14 -> 11, 0 new | the mask tween was never broken (three GFx defects were); `setImageSubstitutions` failed after install because the TextField prototype was one global pointer; retail's "[Unbound]" / "[Mouse Scroll Down] Talk" come from the user's Steam cloud `OPTIONS.SAV` (2013 `CreateProfileName` 0x5a76a0) - without a profile retail's offline fallback `SetToDefaults` gives "[W] [S] [A] [D]"; five builds. Open: the cloud profile read, `TranslateBaseBindings` 0x6baa80, `GTextFormatProto` global, right/centre auto-size anchoring |
| HL | `d9be103` | `OnToggleTutorial` (2013 0x6aa300), `OnToggleAchievementEval` (0x6a9df0), `OnToggleChoke` (0x6b1bd0), `OnToggleJournal` (0x6b5c40), `OnTogglePowerWheel` (0x6b5b60) + exec thunks; gate d3d9/inputtest/newgame 23/23, `unported_natives` 4 -> 0, `probe_natives` 8 -> 4 | five natives, not four (PowerWheel fires at world ~54, past the d3d9 stage); the bodies are 204-574 B with loops, not thunks; nothing yet reads the three input-mask bits or the tracker flags |

Gates: `f147830` (HE+HF+HA) **47 ok, 0 failed, 0 skipped** (2276 s) in `build/gate_wt`; `bcc7f4e` (HG+HD) **46 ok, 1 failed**: `d3d9_frames` 450/390 with d3d9 startup 49 s/37 s against 3.4 s idle - the machine at 100 % CPU with five agents building (user's override), re-run when quiet. `33201ec` (HI) running; the gate on `33201ec` died across the user's PC sleep (04:55-13:18); one gate on `9da5dbc` (HI+HC+HJ) running (`build/gate_HICJ.log`); `d9be103` (HL) after it.
Launch 3: **HL** ports the four player-pawn toggle natives HI's PlayerAction now reaches (`unported_natives` back to 0).
Launch 2 adds **HK** (retail's start-screen controller assignment so Bink can go ON by default; `PreExit`/`StopMovie`).

Coordinator: `2683f3e` (the screenshot restore state). One gate for `f147830` (HE+HF+HA) in `build/gate_wt`.
Merge note: HF's two overlapping files were 3-way merged over HE with `git merge-file` (0 conflicts). `git add -u` once swept the
user's uncommitted SLOMO/GHOST edits into HA's commit; recreated without them - stage explicit paths only.

### Launch 2 (on `f147830`, user away, override on): HB, HC, plus two follow-ups

- **HI — the objective system and the task markers** (HF's boundary): `UDishonoredObjectivesComponent` (`OnGotObjectiveNotify` 0x72a600,
  `OnNewTaskNotify` 0x738020), `UDishonoredObjective`/`UDishonoredTask_Base`, the `DisSeqAct_*Objective*`/`*Task*` Kismet actions
  (all instantiated in L_Tower_P), `ADishonoredPlayerController::OnObjectiveAction`, `FindActiveTaskTargets` 0x731350,
  `UpdateTaskMarkerInstances` 0x80cd40, `PreRender_Markers` 0x8049d0, `Tick_Markers` 2013 0x7b00c0. Accept: "REPORT TO THE EMPRESS"
  at retail t 108-110 from the real Kismet chain; the chevron over the target NPC; the save stage still restores.
- **HJ — the banner's AS2 mask and the key names** (HF's defects): `DHMC_LocationDiscovery`'s mask tween never runs (25 px mask,
  "W W" text); `TextField.setImageSubstitutions` missing on `tutorialMsg_mc`; `ReadPCBindingsFromProfile` 2013 0x6b8240 so the
  hints read retail's "[Unbound]"/"[Mouse Scroll Down]"; `UDisSeqAct_SetTutorialMessage::PostLoad` 0x7b02f0; `execFormatText`.
  Accept: banner SSIM >= .6 in its box vs retail video 8.8-13.9 s and the text reads "WRENHAVEN RIVER"; 0 new AS2 errors.
- HC inherits HA's open item 1 (the Edge locomotion joint's root motion, `AnimLeafCallback` 0x58fb00's locomotion half).
