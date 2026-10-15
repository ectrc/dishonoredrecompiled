# Agent HF — location banner, objective popup, tutorial hints, interaction prompt, NPC marker (wave 25, PHASE22)

Worktree `build/agentHF_wt` (base `9faefc0`), build dir `build/agentHF` (RelWithDebInfo), staged as `DishonoredGame_HF.exe`.
Two builds through `build_queued.py`: build 1 stopped on one error (`GFxMovie::CreateObject` needs its arguments spelled out in this
tree, as every other caller does), build 2 is the measured one. Every number below comes from build 2.

## 1. What landed

| Area | Functions (2013 rva / 2012 rva) | State |
|---|---|---|
| Location banner | `UDisSeqAct_ShowLocationDiscovery::Activated` (0x7998a0 / 0x7ff700), `UDisGFxMoviePlayerHUD::ShowLocationDiscovery` (0x795160 / 0x7fb080) | ported; fires on time; the movie draws it wrong (§3.1) |
| PreAdvance tick list | `DisGFxMoviePlayerHUDPreAdvance` (0x7b2280): `Tick_GameMessages` (0x78d3b0), `Tick_Tutorials`, `Tick_ObjectivePopup` added in retail's order | ported (Tick_Stealth, the probe checks, Tick_Markers, Tick_Subtitles still not) |
| Objective popup | `Tick_ObjectivePopup` (0x7afe70 / 0x812080), `ShowObjectivePopup` (0x798a40 / 0x7fe8b0), `OnObjectiveAdded` (0x7b2480 / 0x814f50), `OnTaskAdded` (0x7b27c0 / 0x815290), `FDisObjectivePopupInfo::AddTask` (0x79ee60 / 0x802fd0) | ported; **nothing feeds it** (§2) |
| Tutorial line | `Tick_Tutorials` (0x7afa60 / 0x80c6b0), `ShowTutorial` (0x7ac5a0 / 0x811810), `InsertTutorial` (0x7a6c10), `FindTutorial` (0x7988e0), `FindTutorialInfo` (0x79eef0), `RemoveTutorialAt` (0x7a7390 + the info dtor 0x794b90), `SetTutorialMessage` (0x7ad060), `ClearTutorialMessage` (0x798960), `ShowTutorialWindow` (0x7ad2f0), `HideTutorialWindow` (0x78d6d0), `ResolveMarkupString` (0x7acdc0), `UDisSeqAct_SetTutorialMessage::Activated` (0x7ae4a0 / 0x8125a0) | ported; both route hints draw at retail's times |
| Key glyphs | `UDisGFxMoviePlayerBase::FormatInteractionText` (0x7a4620 / 0x8085b0), `FormatGamepadKeyName` (0x78c590), `FormatMouseKeyName` (0x78c630), `LocalizeKeyboardKeyName` (0x793dc0 / 0x7fa350); `UDishonoredPlayerInput::GetKeyBindFromAction` (0x6a7870 / 0x6e4100), `InitGameActionBindings` (0x6bac10 / 0x6f3fe0, run from `BuildBindings` where retail's `ApplyGameSettings` runs it) | ported; text differs from retail (§3.2) |
| Interaction prompt (HUD half) | `OnCrosshairActorSet` (0x7af540 / 0x8117c0), `Tick_Interaction` (0x7acb10 / 0x80c450), `UpdateCrosshairFocusText` (0x7984f0 / 0x7fe040), `UpdateCrosshairInteractions` (0x7a6150 / 0x8097d0), the group compare (0x787da0); `ADishonoredPlayerController::SetCrosshairActor` now calls the HUD on every call as retail (0x6a6280) | ported; nothing to show on this route (§2) |
| NPC chevron | — | not ported: upstream is the objective system (§2) |
| Diagnostics | `-puidiag`: every HUD Invoke this package makes (clip, method, first argument, ok / NOT FOUND / CLIP NOT BOUND), every ShowTutorial accept/refuse with its loc location, every tutorial change | new |

Every new tick is guarded on its clip (`IsUndefined`), and every Invoke goes through `DisHUDInvoke`, which refuses an unbound clip.
The anchor line the compare tool reads is emitted exactly: `HUD: ShowLocationDiscovery '<name>' sound <0|1>`; the duration,
the sound event's name (retail posts it through `UAkAudioDevice::PostEvent`; audio is deferred, so it is only named) and the
Invoke result follow on a separate `HUD location card:` line.

## 2. Boundaries (where the upstream is unported, and I stopped)

- **Objective popup** — the whole objective system is a skeleton: `UDishonoredObjectivesComponent` (23 functions incl.
  `AddObjective` 2012 0x7404e0, `OnGotObjectiveNotify` 0x72a600 — the only caller of `OnObjectiveAdded`, `OnNewTaskNotify` 0x738020 —
  the only caller of `OnTaskAdded`), `UDishonoredObjective`, `UDishonoredTask_Base` (incl. `GetStatusString` 0x72d7d0, which
  `ShowObjectivePopup` reads), `UDisSeqAct_AddObjective`, `UDisSeqAct_ObjectiveAction`, and `ADishonoredPlayerController::OnObjectiveAction`
  (2012 0x6ef780, a native stub; it and `UDisSeqAct_TaskAction::Activated` 0x7dd710 are what call `HandleObjectiveEvents`). "REPORT TO THE
  EMPRESS" is a `DisSeqAct_AddObjective` reaching that chain. The HUD consumer is ported and idle; the queue stays empty.
- **NPC chevron** — it is a task marker: `Tick_Markers` (2013 0x7b00c0) → `UpdateTaskMarkerInstances` (2012 0x80cd40) reads
  `UDishonoredObjectivesComponent::FindActiveTaskTargets` (2012 0x731350). Same boundary; HD's projection fix only matters after it.
- **Interaction prompt** — measured: with `-gddiag` the run logs **zero** `gddiag crosshair` lines from world 50 to 175, i.e. the
  crosshair actor never changes from none on this route, so `OnCrosshairActorSet` is called with null and the prompt has nothing to
  show (retail's prompt at world ≈146 names Geoff Curnow; our view is not on him — see PHASE22's heading item, HD). Even with an actor
  under the crosshair, the "[key] Talk" line is filled by `ADishonoredPlayerPawn::FillUIInteractions` (2013 0x6a9130 / 2012 0x7033f0),
  which reaches the player state machine's `FillUIInteractions` (vtable +372 / +416) and the interactable interface's slots +60/+64 —
  none declared in this tree — so the crosshair group would carry the focus text ("Geoff Curnow") and no line. The context, special and
  QTE groups (0x7a63c0, 0x7a6630, 0x7a6920) never change with an empty context and are not ported.

## 3. Results (build 2, `-finewgame=50 -fosoiree -puidiag -diskismet -gddiag -ResX=1920 -ResY=1080`)

Run `HF1.log` (`python build/agentHF/hf_run.py ... --log HF1 --tag hf1`), then `cutscene_compare.py run --tag HF1 --baseline
build/cmp/head_rel/scores.csv` (report `build/cmp/HF1/report.md`).

| Detector (frames on / 126) | head_rel | HF1 | retail on at (ours' world) |
|---|---|---|---|
| banner | 0 | **5, world 52.0–56.0** | 52.0–56.0 |
| hint | 0 | 0 (values 1.9–2.5 vs threshold 3.0; drawn — see below) | 66–70, 147–149 |
| objective | 0 | 0 | 146–149 (boundary §2) |
| prompt | 1 | 1 | 146 (boundary §2) |

Log, HF1 (world = log − 7.4 in this run, read off the `-apshottime` lines; the coordinator's −12.1 belongs to another launch):

- `[0058.66] AS2 trace: SetLocation (Wrenhaven River, 5)` / `puidiag clip 17.SetLocation('Wrenhaven River', 2 args) -> ok` /
  `HUD: ShowLocationDiscovery 'Wrenhaven River' sound 0` — world ≈51.3, during the fade (the fade completes ≈58.5). `-diskismet`
  shows it inside the arrival chain; `sound 0` because the action's `m_bNoSound` is set.
- `[0072.83] ShowTutorial id 1 ... [TheWorld:PersistentLevel.Main_Sequence.SOIREE_Boat.DisSeqAct_SetTutorialMessage_5
  DisSeqAct_SetTutorialMessage] m_TutorialMessage in L_Tower_Script` → `SetTutorialMessage('Use your mouse to look around') -> ok`,
  cleared at `[0078.86]`: world 65.5–71.5 against retail's 66–70.
- `[0158.50] ... DisSeqAct_SetTutorialMessage_8 ...` → `SetTutorialMessage('Press [W] [S] [A] [D] to move') -> ok`, cleared at
  `[0164.51]`: world 151.1–157.1.
- `player input bindings: 127 base + 36 pc + pad set 0 -> 172 bindings, 50 game actions`.

Captures: `build/agentHF/shots/hf1_005.png` (world 157.0) shows "Press [W] [S] [A] [D] to move" bottom-centre in retail's font and
place; `build/cmp/HF1/pairs/pair_w0068.002.png` shows "Use your mouse to look around" in the same place and size as retail's.

### 3.1 The banner draws wrong (not the feed)

The card is up on exactly retail's frames, centred where retail's is, but reads "W W" over a blue rectangle with only the edge
flourishes of the frame (`build/agentHF/banner_strip.png`, world 52–57, static across all five). The log line next to the call says
why it is the movie, not this package: `AS2 setMask: _level0.location_mc._location_mc masked by _level0.location_mc._mask_mc, mask
stage bounds 627.5,21.3..652.5,101.3 px` — a 25-px-wide mask that is never set again; retail's reveal opens it to the title's width.
The text field resolves its font (`$TitleFont` -> 'Emerge BF') and both drop shadows apply. The mask's opening animation and the blue
fill are a GFx/AS2 runtime defect (masking / the reveal tween) for the GFx owner; the SetLocation arguments are retail's.

### 3.2 "[W] [S] [A] [D]" where retail shows "[Unbound]"

The plan said `ResolveMarkupString` produces "[Unbound]". It does not: it only expands `<Strings:Pkg.Section.Key>`.
`FormatInteractionText` turns `` `GBA_MoveForward` `` into `[<LocalizeKeyboardKeyName>]`, which is `Settings.int [Keys] Unbound_Game` =
"Unbound" for a key named None. Retail's key is None because `UDishonoredPlayerInput::ReadPCBindingsFromProfile` (2013 0x6b8240) rewrites
every `m_PCBindings` key from the profile (`GetSettingValue(29 + i)` → `UArkProfileSettings::GetKeyName`) before `InitGameActionBindings`
runs; in the recording the profile answered None for the four movement actions (and Mouse Scroll Down for Use, which is the prompt's
"[Mouse Scroll Down]"). This tree's `BuildBindings` is agent DO's stand-in without the profile read, so the DefaultInput W/S/A/D survive.
Porting `ReadPCBindingsFromProfile` is the next step and belongs with the input-settings owner; the HUD side is literal.

## 4. Where the plan was wrong (with the measurement)

1. "`ResolveMarkupString` produces [Unbound]" — no, see §3.2 (2013 0x7acdc0 decompile; the string "Unbound_Game" has one xref, in
   `LocalizeKeyboardKeyName` 2012 0x7fa483).
2. "`Tick_Interaction` csv 0x7acb10 vs comment 0x78d3b0" — 0x7acb10 is `Tick_Interaction`, and it is **not** in PreAdvance: retail calls
   it from `OnCrosshairActorSet`, which `SetCrosshairActor` (0x6a6280) calls on every call, changed or not. 0x78d3b0, which the old
   PreAdvance comment called `Tick_Interaction`, is a 2013-only `gameMsg_mc._visible` toggle on show bit 0x2000 (ported as
   `Tick_GameMessages`). `Tick_Tutorials` = 0x7afa60 (the comment was right; size 1018 vs 2012's 1001).
3. 2013 changed the tutorial model: `FDisTutorialInfo` is 64 bytes and holds the loc file/section/key, not the resolved text; the text is
   localised on every show (`SetTutorialMessage` 0x7ad060 has no 2012 body). Porting from the 2012 names alone would have stored text.
4. "Never hand-edit `Inc/CppText/*.h`" — those files are the generator's *input* (gen_classes_header.py docstring: "What the tree hand-edits
   survive through ... Inc/CppText/<Class>.h"); a new declaration can only go there. I added three CppText files and edited three, then
   regenerated; the only generated change is three `#include "CppText/..."` lines in `DishonoredGameUIClasses.h`.
5. "drive location_mc with duration and vertical alignment" — the duration is SetLocation's second argument; the vertical alignment
   is `PreRender_Layout`'s (already ported) and `ShowLocationDiscovery` does not read it.
6. The world/log offset is per launch: 7.4 s in HF1, not 12.1.
7. The chevron's dependency is the objective system first, HD's projection second.
8. `ShowObjectivePopup` keeps retail's bug: a task with a status shows ")" alone (three `wcscpy_s` at 0xb98d50/0xb98d97/0xb98daa where
   `wcscat_s` was meant).

## 5. Gate and checks

- Regeneration: `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir <scratch>` is byte-identical to the
  worktree (79/79 files).
- `bitfield_audit.py`: mismatches=0.
- `run_regression.py --build-dir build/agentHF --no-build --only nullrhi,d3d9,inputtest,newgame,options`: see `build/agentHF/gate.txt`
  (GATE_RESULT below). `coresmoke`/`layout` need the CoreSmoke/LayoutProbe targets, which this package's two builds did not build.
- New AS2 errors: `call of a value that is not a function: 'setImageSubstitutions' ... on EditText _level0.tutorialMsg_mc._txt_mc.txt`,
  once per hint shown — the GFx `TextField.setImageSubstitutions` extension agent FF listed as missing. The text draws regardless. The
  `options` stage shows no tutorial, so `options_as2_errors` is unaffected.

Gate (build 2): **32 ok, 0 failed** in 987 s (`build/agentHF/regression/summary.txt`): nullrhi/d3d9/inputtest/newgame/options criticals
all 0, d3d9 startup 3.9 s, d3d9_frames 2010, unported_natives 0, inputtest_moved 1062.7, newgame_on_mission 121, newgame_pawns 29,
options_steps 6, **options_as2_errors 39** (bound 41).

## 6. Follow-ups outside this package

1. GFx: `location_mc`'s mask reveal (§3.1).
2. Input: `ReadPCBindingsFromProfile` (2013 0x6b8240) and `TranslateBaseBindings` (0x6baa80) in `BuildBindings` (§3.2).
3. The objective system (§2), then `Tick_Markers` / `UpdateTaskMarkerInstances` / `PreRender_Markers`.
4. `ADishonoredPlayerPawn::FillUIInteractions` and the interactable slots +60/+64, then `UpdateContextInteractions` / special / QTE.
5. GFx `TextField.setImageSubstitutions`.
6. `UDisSeqAct_SetTutorialMessage::PostLoad` (0x7b02f0) not ported: it only adds a default variable link to an action cooked without one.
7. `execFormatText` (disgfxmovieplayerbase.cpp) can now call `FormatInteractionText`; left alone so the menus' text does not change in this package.
