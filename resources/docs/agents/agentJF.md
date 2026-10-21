# Agent JF — the hide-and-seek choice (wave 27, PHASE24 package JF)

Worktree `build/agentJF_wt` (base main `777497b` = JE merged + its outcome, never moved), build dir `build/agentJF` (RelWithDebInfo),
staged `DishonoredGame_JF.exe`. **Two builds, both 0 errors, 0 C4263/C4264** (`build/agentJF/jf_build1.log`, `jf_build2.log`; build 2 only
re-reads the skip tweaks). IDA on private copies `build/agentJF/idb/{retail2013,shipping2012}_agentJF.i64` (`jf_ida.py`, `jf_vtfind.py`,
`jf_val.py`; decompiles in `build/agentJF/d12`, `d13`). Every run through `build/agentJF/jf_run.py` (JE's runner + `--cmds`, which queues
game_input commands after a given capture) -> `game_input.py launch`, `-windowed -nomovie`. No retail exe, no real input, no FModel. No commits.

Route of every choice run: `-nomovie -finewgame=50 -fosoiree -fwdiag -fwteleport=TriggerVolume_1@155 -frdiag -fwkismettrace -jediag -jfdiag`.

## 0. Step 0 — which fires at the choice point: the conversation node

The hand-played greeting is reached by teleporting into **`TriggerVolume_1`** after the ride (JF0a, `-foupstream` on the head exe):
`SOIREE_Emily.SeqEvent_Touch_3` (TriggerVolume_1) -> `SeqAct_ToggleCinematicMode_1` -> `DisSeqAct_DialogInputs_1 'Greetings'` ->
`DisDialogOneShot_15` `DisConversation_2` -> `DisConv_Soiree_0` -> `SOIREE_Emily.SeqAct_Interp_0` ("Emily_Greetings", 17.58 s) ->
`'Completed'` -> `SeqAct_ActivateRemoteEvent_36 'Soiree_EmilyGreetings_Completed'` -> `SeqEvent_RemoteEvent_55` ->
`DisSeqAct_DialogInputs_2` input 9 **`'Hide and Seek Choice'`** -> `DisConversation_14` -> **`DisConv_PlayerChoice_0`** (output 0 ->
`DisConv_Blurb_2` -> `'Start Hide and Seek game'`, output 1 -> `DisConv_Blurb_1` -> `'Refuse Hide and Seek game'`).

| measure | head (`DishonoredGame_JE.exe`, JF0b) | this package (build 2) |
|---|---|---|
| what fires | `played DisConv_PlayerChoice_0 (DisConv_PlayerChoice) -> output 0` at 174.43 (the base node's default) | `DisConv_PlayerChoice_0 PlayNode: 2 static + 0 optional choice(s) ... waiting` |
| `UDisSeqAct_DialogScriptedChoice` | **none in the level**: `-foupstream=DialogScriptedChoice` matches no op (JF0a); no log of any run names one | not ported (see §3.2) |
| follow-up | `'Start Hide and Seek game'` at 178.05, no input | only after input (JF7 / JF8) |

## 1. What landed (2012 / 2013 rva; 2013 bodies)

| unit | functions |
|---|---|
| `disconv_playerchoice.cpp` | `UDisConv_PlayerChoice::PlayNode_Derived` 0x8dd8e0 / 0x89c570, `TickNode` 0x8dd950 / **0x89c610** (vtable slot 74), `InitNode` 0x8ea740 / 0x89ac90, `MatchChoicesToOutputLinks` 0x8e8070 / 0x8999f0, `GetClassVars_Derived` 0x8d9860 / 0x88aff0 (start of dialog), `GetOptionalChoiceForIndex` 0x8daa00 / 0x88b810, `IsValidChoice`, `GetChoice`, both `GetOptionalChoice`, `GetNumChoices` / `GetNumOptionalChoices`; `FDisPlayerChoice_Static` / `_Optional::GetChoiceText` 0x8dde30 / 0x8934c0, 0x8ddcd0 / 0x893360 (INT -> authored text, else the one localized entry, else `<Lang not found!>`) |
| `stateplayermasterindialog.cpp` | `SetActivePlayerChoice` 0x6b8e30 / 0x669410, `GetNumChoices` 0x6af980 / 0x6608c0, `GetChoiceInfo` 0x6b4190 / 0x663dc0, NEW `IsDialogViewTrackBlending` (2013 0x663d50, unnamed) |
| `stateplayermasterchoice_base.cpp` (JE's half kept) | `ShowChoiceSelection` 0x6c7450 / **0x676a10**, `MakePlayerChoice` 0x6b8820 / 0x669240, `ReadyToMakeChoice` 0x6af930 / 0x660850, `TickState` now shows the list, `HideChoiceSelection` now calls the HUD, `PauseBendTime` / `UnpauseBendTime` now call `BendTime`; base choice virtuals (+388 GetNumChoices, +392 GetChoiceInfo, +396 GetTitleText, +400 MakePlayerChoice_Derived; the dialog state's +396/+400 are retail's `return 0` / empty folds) |
| `disgfxmovieplayerhud.cpp` | `ShowChoiceSelection` 0x803620 / 0x79f390 (`playerChoice_mc.SetMenu([{txt}], title)`, the movie takes focus), `HideChoiceSelection` 0x7f68c0 / 0x78d260 (`Close`, focus back), `OnPlayerChoiceConfirm` 0x7f6990 / 0x78d360 + `execOnPlayerChoiceConfirm` |
| `dishonoredplayerinput.cpp` | `Dis_PlayerChoice_RequestSkip` 0x6e41a0 / 0x6b00e0, `Dis_PlayerChoice_RequestSkip_Released` 0x6f92e0 / 0x6b0140, both execs (0x1f0190 / 0x63a950) |
| `dishonoredgameinfo.cpp` | `ADishonoredGameInfo::BendTime` 0x63fcf0 / 0x5fa0d0, `GetBendTime` 0x62fa60 / 0x5e9e30, `TickBendTime` 0x642db0 / 0x5fcbc0 (partial, §2), its call in `Tick` |
| `dishonoredplayercontroller_tracking.cpp` | NEW `ADishonoredPlayerController::IsTrackingTarget` (2013 0x6a7750), `FDisPlayerViewTarget::IsBlending` (2013 0x6a0e80) |
| `dishonoredplayercamera.cpp` | NEW `GetFOVTargets` (2013 0x6c9fa0; `TickFOV` now calls it as 2013 does), `IsAtTargetFOV` (2013 0x6ca050) |
| `jfdiag.h/.cpp` | `-jfdiag`: node play/tick/hand-over, the channel dump and every `BendTime` / dilation change, the list built and shown, confirm, skip press/release |

Generator: `DishonoredGameNativeStubs.ported.agentJF.txt` (the three natives), new CppText `FDisPlayerChoice_Static.h` / `_Optional.h`;
79/79 generated files byte-identical (`build/agentJF/regen`); `bitfield_audit` mismatches 0.

## 2. Deviations (named in the banners)

- **`TickBendTime` does not write `WorldInfo->TimeDilation`.** Retail sets it to the composed player dilation (0.05 during the choice) and
  keeps the game info, HUD and conversation manager at real time through `m_bAlwaysOutOfBendTime`; this tree's engine has no
  out-of-bend-time tick, so the write would also freeze the unbend that ends the choice. The current dilations are composed as retail
  (Kismet's `ExecuteActiveOps` and the audio RTPC read them: Kismet holds during the choice). Not ported: `TickBendTimeParticleEffects`,
  the `Epp_BendTime` post-process, the bend-time power start/stop events. `TickBendTime` is called from `ADishonoredGameInfo::Tick` (retail:
  the engine world tick through an AGameInfo virtual).
- `ShowChoiceSelection` builds a heap `TArray` (retail: `TMemStackArray` on the main-thread mem stack).
- `TickNode`, `OnPlayerChoiceConfirm`: retail uses the logical / current state blind; a non-dialog / non-choice state is tested here.
- `TickState`: the GFx HUD pointer is tested before its `m_bChoiceSelection` (retail dereferences it).
- `Dis_PlayerChoice_RequestSkip_Released` reads `m_pTweaks`: this tree's `UDishonoredPlayerInput` does not override the tweaks interface
  slot (it answered NULL in build 1, JF4 `held 2.57 s of -1.00 -> no skip`).
- `UDisGFxMoviePlayerHUD::OnPlayerChoiceConfirm` is non-virtual (2012 virtual; no override exists).

## 3. Where the brief was wrong (with the measurement)

1. **"find the greeting's DialogInputs in agentGE.md"**: agentGE.md covers the gazebo fade and the guards' heads; it names neither the
   greeting nor the choice. Measured chain in §0; the route is `-fwteleport=TriggerVolume_1@155`.
2. **The Kismet branch**: `UDisSeqAct_DialogScriptedChoice` does not exist in the Tower level (JF0a); its 2013 `UpdateOp` is 333 B
   (> the 300 B bar; Activated 91 B, DeActivated 86 B), so the Kismet action and `UStatePlayerMasterInScriptedChoice` are **not ported**.
3. **The brief misses the bend-time system.** `ReadyToMakeChoice` / `MakePlayerChoice` test channel 3's *player* dilation < 0.1 through
   `ADishonoredGameInfo::GetBendTime`, and `BendTime` / `TickBendTime` were unported (JE's Pause/Unpause only moved flags): without them
   the list never shows. Ported (§1, §2). Channel defaults at load: all 1.0, max 1 (channel 8: 10) (`bend channel` lines, JF1).
4. **The 2013 node is not the 2012 one.** PlayNode hands over only when the dialog state is the *current* state and the dialog view track
   is not blending; when PlayNode runs the state is still pending (`dialog state active 0`, every run), so the 2013 **TickNode (0x89c610,
   not given)** retries the hand-over - five new unnamed 2013 helpers were needed (0x663d50, 0x6a7750, 0x6a0e80, 0x6c9fa0, 0x6ca050).
5. **"the second choice key ... check DishonoredInput.ini for the choice keys"**: there are none. The list is the HUD movie's own
   (`ShowChoiceSelection` gives the HUD movie focus); `Down` + `Enter` (or the mouse) select inside the movie, which calls
   `OnPlayerChoiceConfirm(index)` through ExternalInterface. `Dis_PlayerChoice_RequestSkip` (F / Enter / pad X) is the **dialog skip**, and
   is ignored while the choice is up.
6. **"skip path works"**: the input half works - JF6 `RequestSkip: in dialog 1, choice up 0` -> `Released: held 2.60 s of 1.00 ->
   PlayerRequestSkip` -> `UDisConv_Soiree::InterruptTickingNode` - but the greeting matinee keeps playing:
   `USeqAct_Interp::OnSkipRequest is not ported` (Engine matinee, out of this package); EndDialog at 174.22 as without the skip.
7. `execOnPlayerChoiceConfirm` 0x78d360 is the 2013 `OnPlayerChoiceConfirm` body, not the exec. `ShowChoiceSelection`'s 2013 rva is
   0x676a10 (0x6c7450 is 2012). `playerChoice_mc` is not "hidden" at :919 - that line lays it out (`SetDisplayInfo` Y).
8. The choice conversation is authored `blocking 0`, yet the player enters the dialog state (`player StartDialog blocking 1 ... dialog
   state requested`, JE's StartDialog); PlayNode's dialog-state test therefore holds.

## 4. Acceptance

| check | result |
|---|---|
| the choice is shown, two texts readable | **met**: 174.62-175.31 in every run `ShowChoiceSelection: ... 2 choice(s) 'Okay, go on...' / 'I'm sorry, Emily, but I have to go see your mother.'`; captures `build/agentJF/shots/jf1/w176.png`..`w202.png`, zoom `jf1/w190_crop.png` (index 0 ringed), `jf7/sel_crop.png` (index 1 ringed after `Down`) |
| index 1 -> output 1 | **met** JF7 (build 2): `key Down`, `key Enter` 179.79 -> `OnPlayerChoiceConfirm(1)` -> `MakePlayerChoice(1) taken` -> `TickNode: choice 1 made -> output 1` 180.00 -> `remote event 'Refuse Hide and Seek game'` 182.98 -> `SeqAct_ActivateRemoteEvent_2 'PlayerWin'` (also JF2, build 1) |
| index 0 -> output 0 | **met** JF8: `key Enter` 178.26 -> `OnPlayerChoiceConfirm(0)` -> `output 0` 178.45 -> `'Start Hide and Seek game'` 182.09 -> `SeqAct_ActivateRemoteEvent_1 'Emily_GoToHideAndSeek'` (also JF3, build 1) |
| nothing fires before input | **met** JF1 / JF6: the list stays up from 174.6 to the quit (~196-205), no follow-up event; the world bend holds (channel 3 world 0, player 0.05) |
| skip via game_input on the binding | input chain met (JF6, §3.6); the matinee skip itself is `USeqAct_Interp::OnSkipRequest`, unported |
| the Empress scene still plays | **met** JF5 (same build): `Emily_GoToGazebo_Success` / `Gate_37` / `Switch_8` 171.01, `Switch_8` 2 -> 'Regent Interrupted By Emily' 177.01, the five lines 161.1-173.9, bars 160.89-185.07 |
| gate d3d9 + inputtest + newgame | **23 ok, 0 failed** (`build/agentJF/gate.txt`): criticals 0/0/0, `unported_natives` 0, `probe_natives` 3 (OnAIStartDistraction, OnOverrideAwarenessDisplay, PlayDying_Native - the three choice natives are no longer stubs; they were never probe hits), `inputtest_moved` 810.8, `inputtest_peak_speed` 400.0, `newgame_pawns` 29 |

Runs: JF0a/JF0b head exe (step 0), JF1-JF5 build 1, JF6-JF8 + gate build 2. Logs `Dishonored_Latest2026/DishonoredGame/Logs/JF*.log`,
measured lines `build/agentJF/JF*_run.txt`.

## 5. Hand-back

`python build/agentJF_sync.py [--apply]` (base `777497b`). Status: `resources/docs/agents/agentJF_status.csv`.
Open: `USeqAct_Interp::OnSkipRequest` (the matinee skip), the world `TimeDilation` write once the engine has an out-of-bend-time tick,
the Kismet `UDisSeqAct_DialogScriptedChoice` + `UStatePlayerMasterInScriptedChoice` for levels that use them, the blurb subtitles of
Emily's answer.
