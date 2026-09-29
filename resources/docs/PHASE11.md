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
