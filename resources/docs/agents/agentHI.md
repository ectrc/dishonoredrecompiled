# Agent HI — the objective system and the task markers (wave 25, PHASE22 follow-up of HF)

Worktree `build/agentHI_wt` (base f147830, fast-forwarded to main `bcc7f4e` before the second build: HD's projection, HG's
Bink page), build dir `build/agentHI` (RelWithDebInfo), staged as `DishonoredGame_HI.exe`. IDA on private copies
`build/agentHI/idb/{retail2013,shipping2012}_agentHI.i64`; decompiles in `build/agentHI/{d12,d12x,d13,m12,m13}/`.

Builds: build 1 (objective system) clean. Build 2 (markers, after the merge with main) failed on ONE unit,
`dishonoredutilities.cpp`, because the stash-pop of the merge split a closing brace between main's and my appended block;
everything else of build 2 compiled. Build 3 (that one unit + link) is the measured exe. That is one build over the
budget of two, and the cause was mine (merge resolution), not the code.

## 1. The retail call graph (2012 decompiles, 2013 checked; 2013 rva / 2012 rva)

```
Kismet DisSeqAct_AddObjective (and Complete/Fail/Remove/SetObjectiveHidden; all DisSeqAct_ObjectiveAction : DisSeqAct_PlayerAction)
  UDisSeqAct_PlayerAction::Activated (- / 0x7d13d0)   no target linked -> Targets += PC->Pawn; USequenceAction::Activated
    USequenceAction handler dispatch: HandlerName OnObjectiveAction, pawn -> controller redirect
      ADishonoredPlayerController::execOnObjectiveAction (0x5eebe0 / 0x6351d0)
        OnObjectiveAction (0x6b80d0 / 0x6ef780)
          UDisSeqAct_ObjectiveAction::GetObjectives (0x7a81d0 / 0x7dd5d0)   object vars that are UDishonoredObjective
          HUD->HandleObjectiveEvents(m_bShowHUDNotification) (0x787ec0 / 0x7f49c0)
          per objective: vtable +400 HandlePackageObjective (0x786780.. / 0x7cbaa0..)
            UDishonoredObjectivesComponent::AddObjective (0x6e0b60 / 0x7404e0)
              FindObjectiveInstance (0x6d0bf0 / 0x72d370) by GetTemplateName (0x6d0a60 / 0x72d0e0)
              StaticDuplicateObjectEx(package objective, outer = component)   (tasks are its subobjects)
              insert before the first active objective it outranks (mandatory > optional, then m_Priority)
              UDishonoredObjective::OnAdded(package) (0x6dc1c0 / 0x72d1c0)
                m_TemplateObjectiveName = package path; profile setting 95 (marker option) via PC->GetProfileSettings
                m_bIsHidden = m_bInitiallyHidden; m_bShowHUDMarkers = !initially hidden && setting95 == 1
                DisSendObjectiveChangedNotify(GotObjective) (0x7be6a0 / 0x8266c0) -> PC->m_pObjectivesComponent
                  ->OnObjectiveChangedNotify (0x6d4f60 / 0x7390a0)
                    OnGotObjectiveNotify (0x6cc050 / 0x72a600) -> HUD::OnObjectiveAdded (HF)
                per task: UDishonoredTask_Base::OnAdded(template name) (0x6c6300, 2013 only)
                  state = initial, hidden = initially hidden, m_TaskID = ++component.m_LastTaskID, markers
                  notify NewTask -> OnNewTaskNotify (0x6d1080 / 0x738020): pawn's DisSeqEvent_TaskActivated + HUD::OnTaskAdded (HF)
                  notify TaskStateChanged -> OnTaskStateChangedNotify (0x6d1320 / 0x7383a0): DisSeqEvent_TaskStateChanged
                       (output = state), HUD OnTaskCompleted/Failed (0x7b2a00/0x7b2b20), objective OnTaskCompletedNotify
          HUD->HandleObjectiveEvents(TRUE)
Kismet DisSeqAct_SetTaskState / SetTaskHidden / UpdateTaskTarget / UpdateCustomTask (DisSeqAct_TaskAction)
  UDisSeqAct_TaskAction::Activated (0x7a8300 / 0x7dd710): DisGetTaskVars("Task") (0x7dd150 / 0x839de0) -> DisSeqVar_Task
    (GetObjectRef 0x79cdb0 -> GetTaskPtr 0x792210: the package objective's task of m_TaskName) -> component GetTaskInstance
    (0x6d0dc0) -> vtable +400 HandleTaskInstance: SetTaskState (0x6c17b0), SetTaskHidden (0x6c63c0), UpdateTaskTarget (0x7a84d0:
    target actor + loc file/section of the action's m_TargetName + vanish preset), OnDisSeqAct_UpdateCustomTask (+292)
Conditions: DisSeqCond_HasObjective (0x78ec20), IsObjectiveComplete (0x78ef30), TaskState (0x7a78c0)
HUD (PreAdvance): Tick_ObjectivePopup (HF) ... Tick_Markers (0x7b00c0) -> UpdateTaskMarkerInstances (0x7ad550 / 0x80cd40)
    -> FindActiveTaskTargets (0x6d9910 / 0x731350) -> new FDisTaskMarker (0x7ce750) attached to _root.markers_mc
HUD (PreRender): PreRender_Markers (0x7a0890 / 0x8049d0) -> ComputeMarkerScreenPosition (0x7be1a0) -> ComputeScreenPosition
    (0x7bdd20: Canvas->SceneView->WorldToScreen) -> FDisTaskMarker::Update (0x7c57f0) -> SortMarkers (0x799480)
Save/load: component GameSave/GameLoad (m_Objectives, m_LastTaskID), objective and task GameSave/GameLoad - agent ED/EC, unchanged.
```

Who sets `m_pObjectivesComponent`: no native code in either exe writes it (scan of every instruction naming the member in
2012); it is a script default subobject and is already set in this tree (the run logs it non-null).

## 2. What landed (files and functions)

| Unit | Functions |
|---|---|
| `dishonoredobjectivescomponent.cpp` (off the exclude list) | IsDiagEnabled (-hidiag), FindObjectiveInstance, AddObjective, RemoveObjective, PopInactiveObjectiveInstance, CompleteObjective, FailObjective, SetObjectiveHidden, IsObjectiveComplete, HasObjective, ClearObjectives, GetTaskInstance, FindActiveTaskTargets, OnObjectiveChangedNotify, DisTaskEventMatches (static, 2013 0x6d0f80), On{Got,Completed,Failed}ObjectiveNotify, On{NewTask,TaskUpdated,TaskStateChanged}Notify |
| `dishonoredobjective.cpp` | IsActive, IsHidden, ShouldShowHUDMarkers, ShowHUDMarkers, GetTemplateName, OnAdded, OnTaskCompletedNotify, FailObjective, SetHidden, GetTaskInstance |
| `dishonoredtask_base.cpp` | IsComplete, IsFailed, IsHidden, GetTaskState, SetTaskState, GetTargets, ShouldShowHUDMarkers, ShowHUDMarkers, SetTaskHidden, GetStatusString (+ the +292/+296/+300 virtual defaults in CppText), OnAdded, OnDisSeqAct_UpdateCustomTask |
| `dishonoredtask_custom.cpp` | GetStatusString_Impl (m_ActiveStatus) |
| `disseqact_{add,complete,fail,remove}objective.cpp`, `disseqact_setobjectivehidden.cpp` | HandlePackageObjective |
| `disseqact_objectiveaction.cpp` | GetObjectives |
| `disseqact_taskaction.cpp` | Activated |
| `disseqact_{settaskhidden,settaskstate,updatecustomtask,updatetasktarget}.cpp` | HandleTaskInstance |
| `disseqcond_{hasobjective,isobjectivecomplete,taskstate}.cpp` | Activated |
| `disseqvar_task.cpp` | PostLoad, GetTaskPtr, GetObjectValue, GetObjectRef, GetValueStr, execGetObjectValue |
| `dishonoredkismet.cpp` | UDisSeqAct_PlayerAction::Activated (NEW) |
| `dishonoredplayercontroller.cpp` | OnObjectiveAction, execOnObjectiveAction |
| `dishonoredutilities.cpp/.h` | DisSendObjectiveChangedNotify, DisGetTaskVars |
| `dishudmarkers.cpp` + NEW `Inc/dishudmarkers.h` | FDisHUDMarker (ctor, dtor, CreateGFxInstance, DestroyGFxInstance, GetLocation, Update, SwapDepth), FDisTaskMarker (ctor, dtor, GetLocation, Update), ComputeScreenPosition (static), DisComputeMarkerScreenPosition |
| `disgfxmovieplayerhud.cpp` (functions changed, for HJ) | ShowObjectivePopup (status string: retail's ")" line), OnObjectiveAdded (comment only); NEW HandleObjectiveEvents, DisHUDQueueFinishedObjective (static), OnObjectiveCompleted, OnObjectiveFailed, OnTaskUpdated, OnTaskCompleted, OnTaskFailed, FDisHUDMarkerVariation::ComputeValue, DisHUDMarkerDiagDue (static), Tick_Markers, UpdateTaskMarkerInstances, PreRender_Markers, SortMarkers, ClearMarkerInstances; one added line each in PreRender, PreClose, DisGFxMoviePlayerHUDPreAdvance; the include |
| CppText (generator input) | ADishonoredPlayerController, UDisGFxMoviePlayerHUD, UDishonoredObjective, UDishonoredObjectivesComponent, UDishonoredTask_Base (edited); UDishonoredTask_Custom, FDisHUDMarkerVariation, UDisSeqAct_{PlayerAction,ObjectiveAction,AddObjective,CompleteObjective,FailObjective,RemoveObjective,SetObjectiveHidden,TaskAction,SetTaskHidden,SetTaskState,UpdateCustomTask,UpdateTaskTarget}, UDisSeqCond_{HasObjective,IsObjectiveComplete,TaskState}, UDisSeqVar_Task (new) |
| Generated | Sources.cmake (20 units off the exclude list), Registrants (link anchors), NativeStubs (2 stubs gone via `DishonoredGameNativeStubs.ported.agentHI.txt`), Objectives/UI/Kismet class headers (CppText includes) |

`-hidiag`: objective add/OnAdded (template, hidden, markers, profile 95), task add (template, id, state, targets, text),
every notify (type, object), every task state / hidden change, every task target set/clear (actor, loc file/section, vanish),
OnObjectiveAction (objective count, notification flag, HUD open), each TaskAction's task resolution, marker add/remove and one
line per marker per second (movie position, tracked position, distance, flags, depth).

## 3. Results

Route `-finewgame=50 -fosoiree -puidiag -hidiag -diskismet -ResX=1920 -ResY=1080` (HI1 build 1, HI2 build 3), then
`cutscene_compare.py run --tag HI1|HI2 --baseline build/cmp/HF1/scores.csv`.

The real Kismet chain, HI1.log (world from `-fwkismettrace`):

```
fwkismet t=150.60 handler OnObjectiveAction on DishonoredPlayerController for DisSeqAct_AddObjective_1
hidiag OnObjectiveAction DisSeqAct_AddObjective_1: 1 objectives, notification 1, HUD open
hidiag objective add Tower_Objectives.TowerEmpress_Report -> DishonoredObjective_0 at 0 of 1, 1 tasks, 'Report to the Empress'
hidiag objective OnAdded DishonoredObjective_0 template Tower_Objectives.TowerEmpress_Report hidden 0 markers 1 (profile 95 = 1)
hidiag task add DishonoredTask_Custom_2 ... id 1 state 1 hidden 0 markers 1 targets 0 'Meet the Empress in the gazebo'
puidiag clip 18.SetObjectives('Report to the Empress', 4 args) -> ok
hidiag task DishonoredTask_Custom_2 target 0 = DishonoredNPCPawn_5 (loc L_Tower_Script / ...DisSeqAct_UpdateTaskTarget_3 ...)
```
HI2.log: `hidiag marker add DishonoredNPCPawn_5 'Empress Jessamine' symbol objectiveMarker_primary depth 0 instance ok`,
then every second `at 406.6,212.3 (tracked 406.6,212.3) dist 6234 flags 0x1` (movie space, 1280x720 movie).

| Detector (frames on) | HF1 (baseline) | HI1 (build 1) | HI2 (build 3, HD merged) | retail on at (tool's mapping) |
|---|---|---|---|---|
| objective | 0/126 | 4/118, world 153.1-156.0 | 3/106, world 153.1-155.0 | 147.2-150.0 (HI2 fit), 159.1-163.0 (HI1 fit) |
| median zoom (ride) | 1.745 | 1.720 | 0.995 | 1.00 |
| median SSIM (ride) | 0.517 | 0.543 | 0.576 | |

Capture `build/agentHI/hi2_pair_w154_retail_t109.8.png` (retail frame 109.8 s top, ours world 154.0 bottom; constant offset
-44.2 s measured at the tool's E1/E2/F anchors): same popup text ("NEW OBJECTIVE / REPORT TO THE EMPRESS / Meet the Empress
in the gazebo") in the same place, and the white chevron at x 305, y 160 (ours) vs x 305, y 157 (retail) in the 960x540 pair.

## 4. Where the plan was wrong (with the measurement)

1. "The chevron uses `ULocalPlayer::Project`" - it does not. 2013 ComputeScreenPosition (0x7bdd20) projects through
   `Canvas->SceneView->WorldToScreen` and reads the view origin and `ViewMatrix` column 2 directly. HD's CalcSceneView fix is
   still the dependency (it builds that view); HD merged before my measured build, so it is in: marker at 406.6,212.3 movie
   = 305,159 in the pair, retail 305,157.
2. "objective > 0 in the frames aligned with retail t 108-110" - the tool's alignment is a two-point fit (E1, E2, 6 s apart)
   and its slope is free: HI1 fitted b = 0.938, HI2 b = 1.059, which moves the ride's end by +-8 s (retail popup "at" world
   159-163 in HI1 and 147-150 in HI2 against the same retail video). With the constant offset the same tool measures at
   E1/E2/F (-44.2 +- 0.4 s) retail t 108-110 is ours world 152-154 and our objective is on at 153.1-155.0: on time within
   ~1 s. Under the HI2 fit the detector reads 0 at retail 108-110; the fit, not the objective, is the reason. The plan's
   "retail popup at mapped world 146-149" was the same artefact.
3. Our popup goes on screen 2.5 s after the Invoke (Invoke at world 150.6, first frame on at 153.1): that delay is the movie's
   own show tween, not the feeder.
4. The objective system needed `UDisSeqAct_PlayerAction::Activated` (2012 0x7d13d0, not in the plan): without it the action has
   no target and its handler never runs. Porting it also gives the other player actions on this route their retail target:
   HI1 shows OnToggleTutorial x2, OnToggleChoke, OnToggleJournal, OnToggleAchievementEval now dispatched to the player pawn
   (script handlers, no native stub hit, the tutorial hints still show at their times).
5. The 2013 objective model differs from 2012 in ways a 2012 port would miss: instances keep an automatic name and are found
   by `m_TemplateObjectiveName` (the package path, which `Dishonored0.sav` records as `Tower_Objectives.TowerEmpress_Report`),
   tasks get ids from `m_LastTaskID`, markers obey profile setting 95, task targets carry a localisation file/section, the
   marker base gained a "tracked pawn visible" flag, and `UpdateTaskMarkerInstances` honours `m_bShowObjectiveMarkers`.
6. The m_Observers loops of every notify are dead in retail: no class in either PDB implements
   IDisObjectivesObserverInterface (no vtable in vtables.csv) and nothing fills the array. Left out, documented.

## 5. Boundaries (named, not ported)

- The popup's look: title "REPORT TO THE EMPRESS" overflows the right edge (the text field's right alignment) and the task line
  sits on a blue band - GFx runtime (same family as HF's location_mc mask), HJ / GFx owner.
- `UDishonoredTask_Custom::OnDisSeqAct_UpdateCustomTask_Impl` (2013 0x6d5010) + EvaluateOneVar<> - no UpdateCustomTask runs on
  the route; m_ActiveStatus stays as cooked.
- The zoom-lens shrink in PreRender_Markers (UDisPostProcessManager +412 / effect state +457, 2013 0x7eb020) - taken as 0.
- UpdateAwarenessMarkerInstances (2013 0x7ad960), the grenade/heart markers.
- `UDishonoredMapInfo::GameSave/GameLoad`'s task-target block (dissavegame.cpp) still writes none / drops the restored targets;
  GetTargets now exists, so it can be finished there.
- The interaction prompt "Geoff Curnow [..] Talk" stays HF's boundary (FillUIInteractions).

## 6. Checks

- Regeneration into `build/agentHI/regen`: 79/79 files byte-identical (ignoring CR).
- `bitfield_audit.py`: mismatches=0.
- Gate (`run_regression.py --build-dir build/agentHI --no-build --exe-name DishonoredGame_HI.exe --log-prefix HIg_`,
  `build/agentHI/gate.txt`): **31 ok, 3 failed**, 0 Critical in every stage. options_as2_errors 39 (<= 41), newgame ok.
  - coresmoke / layout: CoreSmoke.exe / LayoutProbe.exe not built by this package's DishonoredGame builds (as HF).
  - inputtest_moved 288.4 twice in the full gate, then **1058.8** in `--only inputtest` (HIg2_) and 1048.3 in a traced rerun:
    the stage's load-sensitive flake (other agents were building), not this package.
  - **unported_natives 4 (wanted 0) - a real consequence of this package.** `UDisSeqAct_PlayerAction::Activated` now gives
    every player action its retail target (the pawn), so four player actions on the Tower start reach their handlers for the
    first time, and those handlers are bodiless natives: `ADishonoredPlayerPawn::execOnToggleTutorial`, `execOnToggleAchievementEval`,
    `execOnToggleChoke`, `execOnToggleJournal` (2013 0x5ef420 / 0x5ef4e0 / 0x5ef7e0 / 0x5ef840, 96 B execs, each a thunk to a small
    virtual). Before, the actions had no target and silently did nothing; now the stub logs and does nothing, so behaviour is
    unchanged, but the ratchet counts them. Porting the four is the next step (I had no build left: see the build note).
    The alternative - limiting the target rule to the objective actions - would not be retail.
- Save: no `save` stage in this gate's stage list; the objective records' GameSave/GameLoad (agent ED/EC) are untouched and
  `Dishonored0.sav`'s objective template name matches what OnAdded now stores.
