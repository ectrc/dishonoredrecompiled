# Phase 3 wave 10 — the rest of the interface, then the campaign

HEAD `bb05b9d`, gated at 31 checks, 0 failures. Waves 8 and 9 produced a main menu that matches the
user's reference screenshots, navigates by keyboard and mouse, and hovers correctly; NPCs that walk,
have heads and think; all three mission maps running; the powers reaching the frame; and retail saves
loading. This wave finishes the interface the user is actually clicking through, then returns to the
campaign.

## Where this wave starts

The user played the menu and reported four faults in the **New Game flow** — the screens *after* the
main menu, which no package has ever exercised:

1. the mode buttons have no background (there should be a transparent bar that turns white on hover);
2. on *normal / hard / very hard* the images overlap;
3. clicking a difficulty gives a brightness screen reading **"undefined"** with an unresponsive slider;
4. continuing from there shows no modal — it just hides the last menu.

**The cause is measured, not guessed.** One run of that exact path gives this error distribution:

| count | missing built-in | what it breaks |
|---:|---|---|
| 54 | `loadBitmap` | **fault 2** — `flash.display.BitmapData`; the difficulty art never loads or places |
| 12 | `lineTo` | **fault 1** — the drawing API; the bars are drawn at run time, not authored |
| 12 | `tweenTo` | fault 4 — screen transitions |
| 4 | `tweenEnd` | fault 4 |
| 4 | `gotoAndPlay` | **fault 4** — the modal never plays in |
| 3 | `moveTo`, 3 `endFill`, 2 `beginBitmapFill` | fault 1 — the rest of the drawing API |
| 2 | `removeMovieClip` | fault 4 — the old screen is hidden, not removed |
| 2 | `getTextFormat`, 2 `getTextExtent` | **fault 3** — text metrics; the brightness label reads `undefined` |
| 2 | `ClearAnimation` | fault 4 |

So all four faults are **one package**: the ActionScript built-in surface these screens call and this
tree does not implement. That is a far better shape than four separate investigations, and it is why
this wave leads with it.

## Order and why

### EA — the ActionScript built-in surface (the user's four faults)

The drawing API (`beginFill`, `beginBitmapFill`, `moveTo`, `lineTo`, `curveTo`, `endFill`,
`lineStyle`, `clear`), `flash.display.BitmapData` and `loadBitmap`, `gotoAndPlay`/`gotoAndStop` on a
clip, `removeMovieClip`, `getTextFormat`/`getTextExtent`, and `tweenEnd` (its partner `tweenTo`
already runs). Then whatever the slider needs to respond — dragging is not ported, and agent DQ's
hand-over names `setMask`/`hitArea`, `QueueSetFocusTo` and the focus model as the adjacent gaps.

Acceptance is the user's own path: New Game → a difficulty → brightness → continue, with the bars
drawn, the art not overlapping, the label reading its real string and the modal appearing.

### EB — does `NEW GAME` start a mission, and the exit teardown

Two lifecycle questions in one package, because both are about the game starting and stopping rather
than about drawing. The menu reaches `fscommand(ToNewGameScreen)`; whether confirming a difficulty
travels into a level is **untested**. And quitting faults: agent DQ proved it is pre-existing by
reproducing it on the previous HEAD untouched, and noted `FEngineLoop::Exit`'s `delete GGFxEngine`
sits inside a block compiled out here, so the interface is never torn down at all.

### EC — the in-game HUD

The atlas sub-image UV offset (**483 of 1,015 cooked images are `BaseImageId` + `SubRect`** and the
offset is not applied — agent DC), `FGFxEngine::RenderTextures` (2013 `0x58e080`), and
`UDisGFxMoviePlayerHUD`'s natives. Blocked behind EA only because both are in `GFxUI`.

### ED — the five `UObject` save virtuals

105 `GameSave`, 108 `GameLoad`, 11 `IsSaveable`. All-or-nothing: each object's `GameSave` is written
inline with no length prefix, so one missing override desynchronises the stream. This is what makes
*Continue* put the player back where they were, and it is the last thing gating milestone 7. Agent CF
decoded the object dictionary and left a partial decoder.

### EE — rendering leftovers

The remaining filter passes (FXAA `0x5203c0`, MLAA `0x521720`, motion blur `0x521990`, Kuwahara
`0x523350`), the soul-part pass (same shape as bloom parts — relevance bit 23, the primitive set
already filled, shader types already declared, only the pass missing), and `RenderFogMaskStencil`
(`0x433f80`), which agent DB named as the reason the fog mask is bound but never written.

### EF — the deletion backlog

Agent DP's audit: **1,098 shimmed declarations, of which only 5 name a member retail actually has.**
The other 1,053 are features Dishonored's engine branch does not have whose reference code this tree
still compiles. Nine of the last fifteen defects were instances of this, each found by a crash. The
audit ranks them; the top of the list is the legacy path network, morph targets, and cloth/soft body.
Deleting a ranked slice is cheaper than waiting for each to surface.

### EG — milestone 8, the test suite

Load-all over all 471 packages with object counts against the reference build, save load-all, and a
scripted flythrough on two maps to catch physics and animation drift. Nothing of this exists yet.

### Held

`GTessellator` (~60 functions, edge AA only — three packages have now confirmed it was never the
cause of anything visible), the AS2 garbage collector (~147), head-look done properly
(`UArkAnimNodeLookAt::UpdateNodeWeights`, 43 functions, plus `FArkComponentLookat`, 51 — agent DO
left a labelled stand-in), the two open movies sharing one ActionScript registration, and audio,
which stays last where the user put it (`PLAN.md` Phase 10).

## Coordinator's own work, between packages

- **Make C4263 and C4264 errors.** A hand-written override whose signature drifts becomes a silent
  overload; that already cost a completely dead AI transition path that built green and passed every
  check. Open since agent DF named it three waves ago, deferred each time because agents were
  mid-build.
- **Make `d3d9_frames` and `inputtest_moved` load-aware.** Both measure throughput against a fixed
  threshold and both flagged noise as failure today, costing three re-runs to interpret. A check that
  cries wolf trains its reader to explain away failures, which is the habit that let two bad
  measurements stand earlier in this project.
- **Label the 2012 addresses.** A comment block in `DishonoredGame` is a 2012 inventory and does not
  say so; quoting it as retail produced three wrong addresses in briefs in a single day.


## Wave 11 — the saved session, the modal, and two things the coordinator owes

HEAD `79ead3d`, gated at 31 checks, 0 failures. The playable executable is staged and needs no command
line (`resources/build-play.cmd`).

### EF — the saved session (running)

Agent EC restored the player's transform, verified to the float on two saves in two worlds. Everything
past **byte 4,141 of 619,631** is still a fresh session: health, mana, inventory, powers. The stream
stops at `DishonoredPlayerPawn.PowerBlink`, class `UDishonoredActivePowerComponent`, reached from
`ADishonoredPawn::GameLoad`'s `m_ActivePowers` — so every pawn with a power reaches it. Then
`URB_BodyInstance` (note retail's `GameLoad` is vtable **slot 70**, not 69) and the nine partial-state
classes named in `dissavegame.cpp`.

### EG — the modal (the user's screenshots)

`resources/reference/menu/your_menu.png` against `real_menu.png`. With a modal up — QUIT GAME, or Enter
from the brightness screen — **our whole screen goes near-black**: the scene, the menu and the modal's
own content are all gone, leaving the cursor and a faint sliver. Retail keeps the scene behind a dim,
and draws the torn bar, the question, and a YES/NO row with YES highlighted.

**The cause is already measured, by two packages independently.** Agent DM: `tween__start`/`tweenEnd`
fail while `tweenTo` resolves *in the same call from the same prototype* — 5 in a burst on the menu's
movie and **971 on the global movie's, on the box's whole subtree, so its contents never leave alpha 0**.
Agent EA confirmed `tweenEnd` is still absent (4 calls a run) after its own work. So the dim layer sits
at full opacity while everything that belongs on top of it is still at zero alpha.

DM ruled out the obvious theories by measurement rather than reasoning, and its hand-over 3 names the
ten-line measurement that settles what remains: the two movies do **not** share a context
(`GFxMovieRoot` makes its own `GASGlobalContext`), the class **is** installed in the failing movie, the
assignment order does not match the failure set, and `ASSetPropFlags` removes nothing.

This also fixes something DM reported separately: the menu bar takes ~30 s of wall clock to fade in,
for the same reason.

### Coordinator's own, between packages

1. **Make `C4263` and `C4264` errors.** A hand-written override whose signature drifts becomes a silent
   overload. That already cost a completely dead AI transition path that built green and passed every
   check (agent DF), and the two warnings detect it exactly. Open since DF named it four waves ago,
   deferred each time because agents were mid-build.
2. **The two Shipping symbols**, so the executable's name can become literally true: `DishonoredStubs.cpp`
   references `FStatGroupFactory` and `Engine`'s `UnGame.cpp` references `GIsPrepareMapChangeBroken`,
   both outside their non-shipping guards. That configuration has never been built here.

### Still queued after this

Perspective support for the difficulty screen (`_z` is not a display property here); the options settings
tree behind the `undefined` gamma label; the in-game HUD; the exit teardown fault; the shim deletion
backlog (**1,053 of 1,098 placeholders are features retail does not have**, and nine of the last
seventeen defects were instances of it); two generated-declaration hazards agent EC found, one a live
GC hazard; and milestone 8's test suite. Audio stays last.

## Rules for agents

As `PHASE10.md`, plus three earned this wave:

- **Resolve every address against `retail2013_named.i64` yourself.** Addresses in briefs and in this
  tree's comment blocks are not all retail's — three were wrong today, each from the same 2012 block.
- **A test that passes through a fallback proves nothing about the real path.** The menu was reported
  navigable for a whole wave on a scripted key path carrying an "if focus is NULL use the topmost
  movie" fallback, while real key presses reached nothing; and a click was reported delivered while
  the interface's mouse position never left (0,0). Verify with real input.
- **When your snapshot predates another package that touched the same file, do not copy whole files.**
  That reverted a merged function and broke HEAD this wave. Diff against the commit you branched from.
- **Run the harness from the worktree's own copy, with an absolute build dir.** `--build-dir` pointing
  at a worktree-configured directory can never pass the build stage: `resources/build-release.cmd`
  does `cd /d "%~dp0.."` then `cmake -S .`, so it always configures the repo root, cmake refuses the
  mismatched cache, and you get `build_*_exit 1` with `build_*_errors 0` - a failure that reads like
  a broken build and is a path. Agent EJ lost two full runs to it.
- **A fresh worktree has none of the layout stage's gitignored inputs** - `types.json`,
  `retail_sdk_layout.json`, `script_classes_2012.json`, `script_classes_2013.json`, `all_types.h`.
  Without them the three layout tools raise `FileNotFoundError` and the harness records **-1**
  against every layout metric, which reads exactly like a layout regression in the package being
  gated. Copy the five in; they are ignored there too, so `git status` stays clean.
- **Read `d3d9_startup_seconds` before believing a d3d9 or inputtest failure.** The same executable
  measured 3.34, **30.95** and 3.48 seconds across three runs on this machine; the 30.95 run reported
  390 frames and 780.9 moved, and both would have been read as regressions.
- **Never `git clean -x` a build worktree.** The reference data the gate reads - `retail_sdk_layout.json`,
  `vtables.csv`, `pdb_functions.csv` and six others - is gitignored, so `-x` deletes it and every layout
  check then fails with `-1`, "the tool printed no summary line". That looked exactly like a layout
  regression from the package being gated and was the coordinator's own command. `git clean -fd`.

## Wave 12

Wave 11 closed with the saved session at 305 objects and the modal drawing correctly. What is left is
the gap between a menu that looks right and a game you can start: the mouse is dead, and YES does
nothing. Both are on the first screen a player sees.

### EH — the mouse

The oldest open defect in this tree, and the one the user reported first. A pointer over QUIT GAME
draws the game's own cursor on it (`build/agentEG/eg6_t01600004.png`), so the position and the hit test
reach the entry — and the selection still does not move and the click does nothing. Keyboard works.
So this is `GFx_GenerateMouseButtonEvents` and `GFxButtonCharacter`'s state machine, not the cursor.
Named by DM and EG independently; EG put it high and it belongs first.

### EI — making YES do something

`UDisGlobalUIManager`. YES on the quit modal does not quit, and YES on the New Game confirmation does
not start a mission — so the whole front end is currently a display. EA's hand-over 4: the id, the
timer and the `FArkGameEvent`. This is the package that turns the menu into a way into the game.

### EJ — the saved session, continued

EF left the stream at `UDishonoredAIBrain`, retail `0x7256a0`, vtable slot 70, with 305 objects and
23,827 of 619,631 bytes restored. Then the nine partial-state classes in `dissavegame.cpp`. EF's own
lesson applies directly here: when a body's last statement is a virtual call, resolve that slot in
retail before believing the body is finished.

### EK — the in-game HUD

Nothing draws over the world yet. Queued for four waves behind the front end; the front end is now
close enough that the HUD is the next thing a player would notice.

### Still queued after this

Retail blurs the scene behind a modal and this tree does not (EG's hand-over 2, EE's post-process
family — everything else about the modal matches). Perspective support for the difficulty screen; the
options settings tree behind the `undefined` gamma label; the exit teardown fault; the shim deletion
backlog; two generated-declaration hazards EC found, one a live GC hazard; milestone 8's test suite;
load-aware regression bounds for `d3d9_frames` and `inputtest_moved`. Audio stays last.

## Wave 13

Wave 12 ended with the front end working as a front end: the mouse re-resolves under a stationary
pointer, YES starts the mission and YES quits, the HUD opens and binds all 31 of its clips and feeds
itself from the live pawn, and the saved session restores 502 objects. Three things now stand between
that and a game a person can play, and each is one package.

### EL — Corvo in the Tower

`UDisSeqAct_SetPlayerTravelDestination::Activated`, retail **`0x78a0a0`**, whose unit
`Src/dishonoredkismet.cpp` is still in `DishonoredGame_EXCLUDE`. Agent EI measured the whole New Game
path end to end - the map change commits 0.65 s after the key, the Tower's fog actors and its intro
music bank load, the scene censuses 5017 primitives - and **the frame is still black, because the pawn
is never moved into the streamed level and the camera stays at the menu world's spawn**. EI confirmed
the same on the untouched HEAD executable, so it is one unported function and not a regression.

This is the single highest-value function in the tree right now: it is what turns everything the last
three waves built into a picture of the game.

### EM — the two gaps that keep the HUD invisible

Agent EK's hand-overs 1 and 2, both measured, neither worked around:

1. **Every bitmap fill in the HUD is an atlas sub-image and this tree cannot texture one** - nine
   distinct `fill NOT TEXTURED ... type 0x41 def SubImage`. `GFxImageCharacterDef::GetTexture` has the
   sub-image branch, but the `BaseImageId` it resolves is 0 (no def) or 5 (not `RT_Image`) for the
   failing fills; only 1 and 2 resolve.
2. **The stencil mask path clips away what survives** - 6 mask passes a frame, a valid depth-stencil
   bound, nothing reaching the frame. With the three mask entry points no-oped the HUD geometry
   appears at exactly the computed position. The control is the same map with `-gfxuimenu`, where
   `UI_Global`'s cursor draws fine - and that movie has 0 masks.

`External/GFx3` and `GFxUI`. Sequence this against anything else touching the GFx runtime.

### EN — the Ark component layer

Agent EJ's hand-over 1, and it is bounded by measurement rather than by guess: **sixteen of retail's
nineteen component types read exactly `FArkComponentBase::Serialize`**, which EJ ported, so for those
the work is the class and not the serializer. Only `FDisAIKnowledgeComponent` (`0x7039f0`),
`FDisAIMonitorPawnReachability` (`0x73b010`) and `FDisAIMonitorReaction` (`0x73b1a0`) read more - and
type 211 is the third of those, so it leads and cannot be deferred.
`build/agentEJ/comp_serialize.py` regenerates that table from retail live, so re-run it rather than
trusting the snapshot. After this gate `Dishonored0.sav` has **nine** missing bodies left, all under
170 bytes, listed with addresses in agent EJ's report section 4. `Dishonored1.sav` is then a
transcription: `URB_BodyInstance::GameLoad` is 58 bytes per rigid body, 18 bodies.

### EO — the options screen, and the key that closes it

Two faults with one cause each, both already resolved to an instruction:

1. **The rest of `UOnlinePlayerStorage::Read` is still on the 2012 profile ids.** Agent EK fixed the
   eleven HUD show flags (2013 uses 87..101 where we had 85..98) and found mouse sensitivity, gamma,
   volumes and subtitles all still two off, with the cooked profile dump that resolves every id. This
   is very likely the **`undefined` brightness label and the dead slider the user reported**.
2. **CLIK's `B` has no PC key binding.** Agent EH: a real click opens the Options screen, `BPressed`
   arrives, no `TransitionTo (MainMenuScreen)` follows, and the player is stuck. Agent EI: Escape and
   Backspace were both measured against a message box and neither answers it. EI judges these the same
   missing binding in `_common.InputsHandler` - one measurement, not a package, and this package
   should settle it either way.

### Still queued after this

Retail blurs the scene behind a modal and we do not (post-process). Perspective for the difficulty
screen. `UDisGlobalUIManager::Init` (`0x8b8d10`) and the rest of the manager, with
`DisGetGlobalMoviePlayer()` as the bring-up seam to delete. `execOnFocusLost` (`0x78c3d0`, three
lines) is still a stub and now fires every time a box takes focus. NO on the New Game box (its own
selection list). The exit teardown fault. The shim deletion backlog. Milestone 8's test suite.
Whether the save's level state matches the maps agent EK tried, which is why it could not reproduce
agent EF's restored mana. Audio stays last.

## Wave 14

Wave 13 delivered the thing the last five waves were for: **from the main menu, five key presses put
Corvo on the boat landing at Dunwall Tower, with the HUD drawn and the brightness screen working.**
What is left between that and a mission you can play is the world reacting to you.

Every package in wave 13 corrected at least one premise of its own brief, and two corrected a truth
source. Assume this brief is wrong somewhere too, and say where.

### EP — the guards

Nothing in the Tower reacts. The AI stack restores from a save (agents EJ, EN: 1,111 objects) and the
locomotion system works (agent DP found `SetupPathfindingParams` was an empty stub, so every path
search failed before it began), but no NPC has yet been seen to notice the player, walk a patrol, or
speak. Bring one guard to life in `L_Tower_P`: perception, a patrol, and a reaction to being seen.
Agent EN's hand-over names what the components still lack - `Starting`/`Stopping` are unported on all
three AI components, `PreAsyncWorkTick` is an empty body on all three, and `FArkComponentManager`
does not exist.

### EQ — the settings that abort

Agent EO gated retail's settings republish behind `-arksettings` because the first slider move
reached an unported `UDisPostProcessManager::ApplyGameSettings` and **the game aborted**. Nine
`appErrorf` bodies in DishonoredGame still carry that shape. Until they exist the options screen
applies only what `UEngine` applies, and a player who opens Options and moves anything in a build
with that switch on loses the session. Also here: `ArkSettings::SaveSettings`, so a setting survives
the session, and `PCResolutionSettingProvider`, the one row with no value.

### ER — the conversation gate

Agent EN's frontier: `Dishonored0.sav` now stops at `UDisConv_Soiree_InGameData::GameLoad`
(`0x8a9540`), record 1814, with its full 115-byte disassembly already in EN's report section 4.1, so
this needs no game run to start. It needs `USeqAct_Interp::SetConversationNode` and
`USeqAct_Interp+0x1D4`, neither of which exists - that is the conversation system, and the Tower's
opening is a conversation, so this is on the path to the first mission twice over. `Dishonored1.sav`
stops at `UStateNPCMasterDead_Limp::LoadPartialState`, a missing leaf under a ported body, which is
the cheaper of the two and should be taken first.

### ES — the shim backlog, at last

**1,053 of this tree's 1,098 shim placeholders are features retail does not have**, and nine of the
last seventeen defects were instances of something being invented that retail does not do. This has
been queued for six waves behind things that looked more urgent. Agent EN's second finding makes it
cheaper than it was: `Sources.cmake` is an **exclude list** holding 818 comment-only skeleton units,
and a unit taken off it still needs `ARKCOMPONENT_LINK_TYPE`-style external linkage or the linker
drops it whole. Delete what retail does not have, and write down what the 818 are.

### Still queued after this

Retail blurs the scene behind a modal and we do not. `UDisSeqAct_ShowLocationDiscovery::Activated`
(`0x7998a0`), the "Dunwall Tower" title card. The camera is not told it was teleported. Story flags
are transient and `ADishonoredPlayerPawn::GameSave/GameLoad` are unported, so they do not survive a
save. `UDisGFxMoviePlayerGamma` has no implementation file at all. The message box's `B`. A census of
unported `USequenceCondition`s, which agent EL showed kill a Kismet chain silently. Three of
`UI_HUD`'s eight atlases are never drawn. `execOnFocusLost`. Milestone 8's test suite. Audio stays
last.

## Wave 15

Wave 14 put a guard on a patrol route, made the settings republish without aborting, took the save
stream to **86.7%**, and cut the shim count by a fifth. Three of the four packages corrected a
premise of their own brief and two corrected a truth source - that is now every wave, so assume the
same here.

### ET — perception, and the reaction

Agent EP delivered the patrol and then **specified** this instead of half-porting it:
`build/agentEP/spec/vision_spec.md` (1,026 lines - full member layout of `FDisComponentVision`,
`VisionNPC` and `Observable` with per-name evidence, every vtable slot, the cone algebra) and
`build/agentEP/spec2/creation_spec.md` (both creation sites, which policy each `Starting` registers
with, and the fact that only tick phases 0/1/2/5 are live - 3 and 4 are dead). 180 decompiles sit in
`build/agentEP/dec2013/`.

The reaction has **exactly one door**: `FAIStimStruct_TargetSighted` has a single consumer in the
whole image, `UDisAIBrainProcessAttention::FilterTargetSighted`, 61 functions and 5,507 bytes.

And EP corrected the tree's own account of how this works: **vision is never ticked from the brain.**
`dishonoredaibrain_senses.cpp` claims retail's `TickBrain_Senses` asks the vision component what it
can see; the real body (2013 `0x717880`) is two inhibitor masks, `CheckForImportantKismetEvents`, a
player-proximity attention pass and one look-at tick. Vision is an Ark component on
`FArkComponentManager`'s time-sliced policy.

Deliver: **a guard notices Corvo and reacts.**

### EU — the guard finishes its patrol

Three separate measured blockers, all localised by agent EP rather than guessed:

1. **The patrol stops at its first point.** `DisDesireStructs::StartLoco` is called exactly 26 times
   in 266 s - once per NPC, for Idle's `Stand` - while four patrol sub-states hold a live destination
   with the loco component bound, `m_bDesired` set and `m_bPaused` clear. EP eliminated the two
   obvious causes by measurement; the next instrument is one counter per `EDisDesireRequestStatus` in
   `GetRequestStatus`.
2. **Three of four route destinations are off the loaded 668-poly navmesh** (`GoalPolyNotFound`) -
   a streaming bound, not an AI one.
3. **`ADishonoredNPCPawn::m_pNPCMasterFSM`'s `m_NativeStateMap` is empty**, because nothing in this
   tree calls `InitFSM`. Agent ER had to route a save read through a transient scratch instance to
   work around it and named this as the clean fix.

### EV — the last 82,423 bytes, and two guards

`Dishonored0.sav`'s remaining loss is **two bytes wide**, inside `AActor::GameLoad` (retail
`sub_58AD70`, 1,286 bytes) reached through `AInterpActor::GameLoad` (2013 `0x18c160`, which reads
nothing of its own). Compare ours to retail's read for read. Agent ER's evidence that it is a
position loss and not a missing tail: of 29,951 resolved references, index 30375 is the **only** one
outside the 11,324 records, and none before it had the unshared bit set.

Then **the two guards in `FLevelLoader::operator<<`** ER asked for: a deferred index must be inside
the record count, and must only be accepted when the level state declares unshared objects. Without
them a misread WORD with bit 15 set ends the object loop silently - `STREAM ENDED EARLY`, `0
unported`, no class named - which is exactly how `USequenceFrame::IsSaveable` hid for five packages.
That is the difference between a four-hour hunt and one log line.

Then `Dishonored1.sav`, which is at 21,081 of 294,961, and the named unported overrides ER handed
over with every offset and mask already resolved.

### EW — the five layout defects, and the next 266 shims

Agent ES found **five members where retail has per-instance storage and this tree has a
process-wide `inline static`** - the opposite of a placeholder, and live wrongness:
`UMaterial::bAllowFog`, `bUsedWithFogVolumes`, `bUsedWithFracturedMeshes` (bits 3, 12 and 16 of the
dword at **752**), `UPrimitiveComponent::ReplacementPrimitive` (**196**) and
`AWorldInfo::ProcBuildingRulesetOverride` (**1244**). Every offset is bracketed in the binary against
a neighbour whose own name matches the SDK dump. ES did not fix them because three sit in headers
other packages were editing.

Then the **266 provable, unambiguous shims with at most one using file and at most two uses**, ranked
in `agentES_status.csv`. Uses concentrate: `GameCrowd.cpp` 41, `UnPhysComponent.cpp` 25,
`UnPhysSkelComponent.cpp` 17, `UnInterpolation.cpp` 15. `resources/tools/shim_ratchet.py` holds the
ceiling at 887 and will not let it rise.

### Still queued after this

The options tab cannot be reached from the keyboard, so the graphics settings are unreachable without
a mouse: `FGFxEngine::InitKeyMap` ignores `[GFxUI.KeyMap]` and `GFxKey::Code` has no `GAMEPAD_*`
codes. The video sub-screen and its resolution row. The modal's background blur. The "Dunwall Tower"
title card (`0x7998a0`). The camera is not told it was teleported. Story flags are transient and the
player pawn's save virtuals are unported. `UDisGFxMoviePlayerGamma` has no implementation file. A
census of unported `USequenceCondition`s, which kill a Kismet chain silently. Three of `UI_HUD`'s
eight atlases are never drawn. Milestone 8's test suite. Audio stays last.
