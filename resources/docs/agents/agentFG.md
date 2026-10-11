# Agent FG (PHASE14 FG) — the modal's text, and the sweep that empties the options rows

Branched from `18f36a3`; **re-based twice while the package ran**, onto agent FF's `63c2539` and then
onto agent FH's `f72f9f5`, and every number quoted for acceptance is from `f72f9f5` with both of those
merged in. Worked only in `build/agentFG_wt`.

Build dirs: `build/agentFG_wt/build/agentFG_rel` (this package),
`build/agentFG_headsrc2/build/rel` (the **untouched** baseline, a `git archive` of `f72f9f5` built in a
directory this package never edits), `build/agentFG_wt/build/agentFG_reg` (the regression harness),
`build/agentFG_wt/build/agentFG_clean` (the clean gate build), and `build/agentFG_headrel` +
`build/agentFG_ffsrc` (the earlier `18f36a3` baseline and a scratch FG+FF combination, kept because
some measurements below were taken on them and are labelled as such).
IDA: own copies `resources/docs/idb/retail2013_agentFG.i64` and `shipping2012_agentFG.i64`, headless.
**No FModel tool.** No commits, nothing staged, nothing written into the main checkout outside
`build/`.

**Seven files. Six in `External/GFx3`, which agents FF and FH also entered — §8 names every function
and the merge evidence.**

## The answer in ten lines

* **Fault 3 is fixed, both halves, and they were one bug.** The token is substituted **in
  ActionScript** (`_common.ConvertVariableTags.Convert`), and it failed because the localisation
  string reached ActionScript as **Latin-1** where the SWF's own `"§CATEGORY§"` constant is
  **UTF-8**. Retail encodes UTF-8 at that boundary — `GString::operator=(const wchar_t*)`,
  2013 rva `0x9b5390`, through `GUTF8Util::EncodeString` — and this tree truncated `wchar_t` to
  `char`. §2.
* **The colour needed the parser, and `GFxStyledText::ParseHtmlImpl<wchar_t>` (2013 rva `0xa9d870`,
  0x187c = 6,268 bytes) is now ported.** §3.
* **Measured against retail.** Retail's coloured run is RGB (177, 192, 215) — the asset's own
  `#B1C0D7` — B−R **+38.0** against **−12.8** on the rest of the line. Ours is (171.0, 185.6, 207.7),
  B−R **+36.7** against **−12.1**, and the run starts within **2 px** of retail's. At HEAD there is no
  coloured run in the image at all. §4.
* **Fault 2 is one line of `GFxDisplayList::MarkAllEntriesForRemoval`, and I have retail's body.**
  2013 rva `0x9cbed0` marks an entry only when `depth <= 0x3FFF && CreateFrame > fromFrame`; this tree
  marked **every** entry. An options row's value widget is `attachMovie`'d, so the first time the row
  rewinds its own timeline — which `RemoveWidgetFocus`'s `gotoAndPlay("out")` makes the next
  `gotoAndPlay("over")` do — the sweep unloads the widget with it. §5.
* **And the argument is a frame, not an index.** `GFxSprite::GotoFrame` (2013 rva `0x9f7cc0`) passes
  `target - 1` on the rewind path; this tree passed `0`. Corrected. §5.
* **Fault 2 reproduced for me where it did not for agent FF, and the difference is the schedule**:
  the pointer has to be walked **down the label column, one row at a time, and then parked away from
  the list**. A single hover does not do it. §5 has the counts for both builds. §5.
* **Fault 1's "white panel on the wrong container" does not reproduce.** At a matched 1600x900 our
  pale panel and retail's sit in the same place and read the same strength; the background scenes
  differ by more than the quantity in dispute, and I say so rather than claiming either way. §6.
* **What is real on that screen is that our build renders the list's *mask*.** `maskContent` is one
  `DefineShape3`, `#E3F2D6` at **alpha 24/255**, stretched to 957x385 over the row area. Agent FF's
  `setMask` takes it out of the render and `setMask` has landed, so if the user still sees a panel
  difference it is not this. §6.
* Regression **37 ok, 0 failed, 0 skipped**; a clean full Release build, directory deleted first,
  `DISHONORED_LAYOUT_CHECKS=ON`, all three targets: **0 errors, 0 C4263, 0 C4264**; `rva_sweep` over
  the whole worktree: 7,430 citations, **3 suspects, none in a file this package touches, all three
  already at HEAD**. §7.
* **Three premises of my brief were wrong and one of my own conclusions was.** The token is
  `§CATEGORY§`, not `?CATEGORY?`, and it is substituted in ActionScript, not C++. Fault 2 is not "a
  shared container being hidden" and not "all six at once". Fault 1's panel is not misplaced. And my
  own first reading of the arrow chrome was a comparison against *retail*, not against our own HEAD,
  which is a different claim from agent FF's — §6 separates them.

## 1. What was measured, and with what

Every screenshot is an in-process `-apshottime` capture. Nothing in this package touched the desktop,
a window or GDI at any point. `build/agentFG_run.py` stages the exe under this agent's own name and
claims bitmaps **by name**: `-apshotname=<tag>` is at HEAD (`SceneRendering.cpp`, agent FE), the files
are `apshottime_<tag><N>.bmp` and the watcher globs `apshottime_<tag>*`. Nothing here claims a capture
by "arrived after my launch", which is the rule that cost agent FF nine of agent FH's and FI's
bitmaps. `build/agentFG/go.py` launches the run and `drive_input.py` together.

**The capture rule works and was proved before anything was built on it**: the first run of the package
(`base1`, mark at world time 32) produced `apshottime_base100000.bmp` with the OPTIONS screen in it.

Three facts about the machine that the coordinator should have, because they changed what the numbers
mean:

* **The desktop resolution flaps.** For most of this package it was **1024x768**, so
  `-ResX=1600 -ResY=900` was clamped and captures came back **1008x567** (`GFx stage map: … viewport
  buf 1008x567`). For one window it was 6144x1728 and the identical command line produced true
  **1600x900**. Agent FH saw the same thing eight-of-ten versus two-of-ten. **Every capture's size is
  read off the file here, never assumed**, and each number below says which size it came from.
* **`drive_input.py` reports the client area as 1280x720 while the backbuffer is 1600x900**, because
  the process is DPI-aware (agent EX) and the driver is not. It drives in fractions, so the steps land
  where asked; only the pixel numbers in its log are in the virtualised space.
* **A concurrent agent's game steals the foreground and kills the menu.** The symptom is one
  `UDisGFxMoviePlayerBase::execOnFocusLost` line followed by the movie never drawing again, and the
  capture is the 3D scene with no interface. `grep -c FocusLost <run>_log.txt` is a clean validity
  flag and every run below is scored with it: a run with a non-zero count is discarded, not averaged.

## 2. Fault 3, first half: the token

`?CATEGORY?` is not the token. The token is `§CATEGORY§` — U+00A7, SECTION SIGN — and it is
substituted **in ActionScript**, which is the question the brief asked to settle before assuming
either side. `build/agentEY/swf/OptionsMenu.as2.txt:8966` builds

```
{ variableTag: "§CATEGORY§", textValue: <the category name>, colorTag: _converter._blueColorTag }
```

and passes it with `texts.t_Q_RestoreSettings` to `_common.ConvertVariableTags.Convert`
(`__Packages._common.ConvertVariableTags`, disassembled at `GammaImage.as2.txt:2264`):

```
this.TextToFormat = s; ExternalInterface.call("FormatText", "_root.TextToFormat");
s = this.TextToFormat;
for each tag: if (s.indexOf(tag.variableTag) >= 0)
    s = s.split(tag.variableTag).join(tag.colorTag + tag.textValue + "</font>");
```

with `_blueColorTag = "<font color=\"#B1C0D7\">"`. **Both halves of fault 3 come out of that one
call** — the substitution and the `<font color>` that wraps it. It failed at `indexOf`, because the
two strings were in different encodings:

| | bytes for `§` |
|---|---|
| the SWF constant pool (`Startup.UI_OptionsMenu.OptionsMenu.gfx` @34240 and @143939) | `C2 A7` — UTF-8 |
| `DishonoredGame.INT`, read as UTF-16 by `appBufferToString`, handed to GFx as `SetStringW` | `U+00A7` |
| what `GFxMovieRoot::GFxValue2ASValue` made of it | `A7` — `buf[n] = (char)w[n]` |

**Retail does not truncate.** Resolved by hand in `retail2013_agentFG.i64`:

* `GFxMovieRoot::CreateStringW` 2013 rva `0x9a9030` (`// 2012 rva 0x9b0010 ratio 1.000`) calls
* `GASStringManager::CreateStringNode(const wchar_t*)` rva `0x9c0800`, whose whole body is
* `GString::operator=(const wchar_t*)` rva `0x9b5390`, which sizes with
  `GUTF8Util::GetEncodeStringSize` (`0x9e7e80`) and fills with `GUTF8Util::EncodeString` (`0x9e7f40`).

Three sites in `GFxPlayerRoot.cpp` truncated; all three now encode UTF-8 through a new
`GFxTextWideToUtf8` beside the `GFxTextUtf8ToWide` this tree already had. The one the localisation goes
through is the `VT_StringW` arm of `GFxValue2ASValue` (`DisGFxMoviePlayerInitTexts` →
`Value.SetStringW(*It.Value())` → `View->SetVariable`).

Measured, from the AS2 `trace` the movie writes itself, on the **untouched** baseline and on this
package:

```
f72f9f5   AS2 trace: ---ShowMessageBox (Do you want to restore settings of the current category (?CATEGORY?)?, YES, NO, …)
+ FG      AS2 trace: ---ShowMessageBox (Do you want to restore settings of the current category (<font color="#B1C0D7">Audio</font>)?, YES, NO, …)
```

## 3. Fault 3, second half: the parser

`GFxStyledText::ParseHtmlImpl<wchar_t>` is ported as `GFxStyledText::ParseHtml`, and
`GFxEditTextCharacter::SetTextValue` calls it instead of stripping the tags. The field's `text` is
taken back out of the parsed document, so the AS2 getter and the variable binding still answer
markup-free text.

The tag set is a census, not a guess (`build/agentFG/tag_census.py`): across the six cooked movies and
`DishonoredGame.INT` the content contains `font` (face, size, color, letterSpacing, kerning — 212),
`p` (align — 163), `br` (12 in the movies, 54 in the localisation), `b`, `u`, `a`. `textformat`, `li`
and `img` are implemented as well because they cost nothing beside the rest; `<img>` is **counted and
dropped**, because the image substitution it needs is not ported. Entities are
`&amp; &lt; &gt; &quot; &apos; &nbsp; &#N; &#xN;`, which is `GFxSGMLCharIter::DecodeEscapedChar`'s set
(rva `0xa971d0`); `<! … >` is skipped as `SkipComment` (`0xa97bf0`) does. The tokeniser retail uses,
`GFxSGMLParser<wchar_t>::GetNext` (rva `0xa988b0`), answers a token **kind** and never a tag name —
which is why the tag names appear in the image as neither wide nor narrow literals, and why this port
matches them inline as retail does.

Three deviations, each stated at the definition: no `HTMLImageTagInfo` out-parameter, no
`GFxTextStyleManager`, and `<font face>` is resolved against the font manager here rather than left to
`GFxTextDocView::FindFont`, whose font-map route (`GFxFontMap::GetFontMapping`) is what turns
`$TitleFont` into a family and is not ported. That is the same resolution
`GFxEditTextCharacter::GetInitialFormats` already does (agent FB), so the fonts behave exactly as they
did.

A new census line, `GFx html census`, prints only when a field carries markup this port does not know.
**It never printed, on any run of this package, on any screen the driver reaches.**

**One thing that cost a build and belongs in the record.** The first cut used `GArray<GFxHtmlAttr>`
where `GFxHtmlAttr` held two `GArray<wchar_t>`. `GArray::Reserve` copy-constructs its elements and
`GArray` has no copy constructor, so the first growth shallow-copied the inner buffers and freed them
twice: the run died at `0xC0000374`, STATUS_HEAP_CORRUPTION, before the menu. The hazard is already
written at the top of `GFxStyledText.cpp` — it is why `GFxTextCopyArray` exists — and it is now
written again at the struct that must not have it. The attribute table is plain fixed buffers.

## 4. Fault 3, measured side by side

`build/agentFG/sbs_modal.png`: the modal's line from the untouched build, this build and the right
half of `resources/reference/menu/3.png`, all at 1600x900.

| | the coloured run | B−R on the run | B−R on the rest of the line | run starts at x |
|---|---|---|---|---|
| ours, untouched | **there is none** | — | — | — |
| ours + this package | RGB (171.0, 185.6, 207.7) | **+36.7** | −12.1 | 1023 |
| retail (`3.png`) | RGB (177.0, 192.0, 215.0) = `#B1C0D7` | **+38.0** | −12.8 | 1021 |

Same window size, reached the same way, nothing forced visible. The 6 % on the absolute values is the
menu's known brightness difference (`STATUS.md` open defect 4): our *plain* text is 4 % darker than
retail's by the same measure. That pair is `build/agentFG/ba1_3.png` / `fx1_3.png`, taken on the
`18f36a3` base during the window in which the desktop allowed a 1600x900 window.

**Repeated on the current base**, `build/agentFG/sbs_modal_f72f9f5.png`, untouched `f72f9f5` above
this package, 1008x567, same schedule: the untouched build has **0 bluish pixels** anywhere in the
modal's line; this package's coloured run reads B−R **+28.2** against **−8.3** on the rest of it. The
AS2 traces of that pair are

```
f72f9f5   ShowMessageBox (Do you want to restore settings of the current category (?CATEGORY?)? …
+ FG      ShowMessageBox (Do you want to restore settings of the current category (<font color="#B1C0D7">General -> Gameplay</font>)? …
```

and the second one is worth having: the tab click landed on GENERAL rather than AUDIO, so that run
exercises the *other* arm of the asset's expression — `Category_Name + " -> " + SubCategory_Name`
(`OptionsMenu.as2.txt:8873`) — and it substitutes correctly too.

## 5. Fault 2: one sweep, not six widgets

**Two premises of the brief are wrong.** It is not "a shared container being hidden, emptied or masked
out", and it is not "all six rows at once" — it is **one row at a time, the row the pointer has just
left**, accumulating as the pointer walks down the list until most of the column is empty. That is why
`2.png` shows all six: the user walked the whole list.

Measured first with `-disclipdiag` (HEAD build, 1008x567). Each row is
`_root.optionsMenu_mc._options_mc._list_mc.widget<N>`, an `o_options_line` of **32 frames** with labels
`default`=1, `over`=9, `out`=16, `disabled`=23. The value widget is **not** a timeline child: it is
`attachMovie`'d (`_slider_mc`, `_stepper_mc`, `_stepperB_mc`, referred to as `_widget_mc`). After a
hover the rows read `_currentframe=22` — the end of the `out` animation, where
`_common.P_OptionsWidget.RemoveWidgetFocus`'s `gotoAndPlay("out")` leaves them — and the row that was
focused has lost its value widget.

`GFxSprite::GotoFrame`'s rewind arm is what runs, because `gotoAndPlay("over")` on a row sitting at 22
is a **backward** goto. At HEAD that arm was

```
DisplayList.MarkAllEntriesForRemoval(0);   // mark EVERY entry
… replay frames 0..target …
DisplayList.UnloadMarkedObjects();
```

and an `attachMovie`'d child is never re-placed by a frame tag, so the sweep takes it. Retail's body,
decompiled whole from 2013 rva `0x9cbed0` (`// 2012 rva 0x9d5670 ratio 1.000`):

```
for (i = 0; i < Size; i++)
    if (Entries[i].pChar->Depth <= 0x3FFF && Entries[i].pChar->CreateFrame > fromFrame)
        Entries[i].pChar->Flags |= 0x40;
```

Two things this tree did not have:

* **the depth test.** A clip `attachMovie`, `createEmptyMovieClip` or `duplicateMovieClip` made sits at
  AS depth **+ 0x4000**, above every timeline depth, so retail's sweep can never reach it. That
  convention is **agent FF's**, landed in `63c2539`, and it is what makes the test do work — before
  that commit the line would have been a no-op. The port is retail's own test, `GetDepth() >=
  GFxASDepthBias`. (A first cut spelled it `bScriptCreated`, the flag this tree used to carry for the
  same distinction; FF deleted that flag in favour of the depth, and the re-base caught it as a
  compile error, which is the right place to catch it.)
* **the argument is a frame.** `GFxSprite::GotoFrame` (2013 rva `0x9f7cc0`) passes `target - 1`;
  `GFxSprite::IncrementFrameAndCheckForLoop` (`0x9eace0`) passes `0`. This tree passed `0` from both.
  Corrected. The `CreateFrame` half of the test is **not** reproduced and is called out at the site:
  `GFxCharacter` in this tree has no create frame, and it costs nothing here because both callers
  replay the frames up to the target and `AddDisplayObject` clears the mark on every character those
  frames place.

Retail has three callers of `MarkAllEntriesForRemoval` — `IncrementFrameAndCheckForLoop`,
`GFxSprite::Restart` (rva `0x9efb80`) and `GotoFrame`; this tree has the first and the third.

### The counts

Schedule, identical on both builds, keyed to the log mark: Space ×3, Right ×2, Enter, then **six
hovers down the label column** at x = 0.228 and y = 0.333, 0.402, 0.469, 0.533, 0.600, 0.661, then the
pointer parked at (0.90, 0.88), then the capture. Runs with a non-zero `FocusLost` count are discarded; the discards are named.

| base | build | valid runs | runs where a widget vanished | worst run |
|---|---|---|---|---|
| `63c2539` | untouched | 1 (`hb2`) | **1** | **3 of 6 gone** |
| `63c2539` | + this package | 2 (`fx4`, `fx5`) | **0** | — |
| `f72f9f5` | untouched | 2 (`hb4`, `hb6`) | **1** | **4 of 6 gone** (`hb6`) |
| `f72f9f5` | + this package | 2 (`fx8`, `fx9`) | **0** | — |

Discarded for `FocusLost`: `hb5`, `fx7` — a concurrent agent's game took the foreground and the menu
never drew again. Across both bases: **2 of 3 valid untouched runs reproduce the fault and 0 of 4
valid runs with this package do.** The baseline is intermittent, so four clean runs are suggestive
rather than conclusive on their own; the evidence that carries the claim is the retail body above,
which says in two tests exactly which characters the sweep is allowed to touch and ours obeyed
neither.

`build/agentFG/sbs_fault2.png` is the untouched build above this one, same schedule, same size.

**Why agent FF could not reproduce it**: a single hover, or a hover that stays on the value column,
leaves the row focused — `RemoveWidgetFocus` never runs on it, so the row never rewinds. The pointer
has to *leave* each row.

## 6. Fault 1, and what re-measuring against FF and FH showed

**1b is agent FF's and is closed.** At `18f36a3` `_list_mc.setMask(maskContent)` logged `call of a
value that is not a function: 'setMask' … on Sprite _level0.optionsMenu_mc._options_mc._list_mc`, once
per list build; it is the only mask call on that screen. FF ported it and measured it on
Options → CONTROLS → KEYBOARD MAPPING (`build/agentFF/mask_band.png`: four of twelve rows outside the
mask at HEAD, none after). **This package produced no scrolled capture of its own and claims none** —
the acceptance item moved to FF with the fix.

**The panel.** What the asset says, read rather than guessed: `o_OptionsList_withTabs` (sprite 22 of
`Startup.UI_OptionsMenu.OptionsMenu.gfx`) has **exactly one timeline child**, `maskContent` at depth 1,
and `maskContent` is a one-frame sprite holding **one `DefineShape3`** — a single solid fill,
`#E3F2D6` at **alpha 24/255**, 165x165 px, scaled by the clip to 957x385. Live it read
`_visible=true _alpha=100 _xscale=580.0 _yscale=233.3 _width=957.08 _height=385.0`. Before FF's
`setMask`, our build drew that as ordinary geometry: a 9 % cream wash over exactly the row area which
retail does not have.

**It does not reproduce as a visible difference and I will not claim it does.**
`build/agentFG/sbs_options.png` is our 1600x900 AUDIO screen above the right half of
`resources/reference/menu/1.png`. The panel sits in the same place and reads the same strength in both.
Numerically: at nine scanlines through the panel the inside-minus-outside step is between −106 and +55
in ours and between −122 and +67 in retail, with no consistent sign — the two background scenes differ
by far more than a 9 % wash, so this comparison **cannot** resolve the quantity in dispute, and the
honest answer is that the fault as the brief states it is not visible at a matched size. Agent FF,
comparing our own two builds instead, found the extent identical.

**The arrow chrome — two different claims, and they do not contradict.** Agent FF compared our HEAD to
its own build and found the `<` `>` arrows drawn on both: nothing FF did changed them. What I measured
is a comparison against **retail**, at a matched 1600x900: retail's *focused* row carries a bold `◄` to
the left of its slider and a bold `►` after the value, and ours keeps the faint default pair
(`build/agentFG/sbs_options.png`, the top slider of the AUDIO tab). The mechanism the asset gives for
that is `_common.P_OptionsWidget.SetWidgetFocus` (`OptionsMenu.as2.txt:14855`) doing
`_leftArrow_mc.arrow_mc.gotoAndPlay("over")`, against `RemoveWidgetFocus`'s `gotoAndStop("locked")` and
`gotoAndPlay("out")` — the same labelled-frame family as fault 2 and as agent FH's portrait. **Agent
FH's `ReplaceDisplayObject` fix (`f72f9f5`) is in the same family and landed after that observation**;
it is handed over in §9 rather than claimed either way, because settling it needs the arrow clips' own
frame dump, which is FH's instrument.

## 7. The gates

* `run_regression.py`, the worktree's own copy, absolute build dir, built inside the harness, on
  `f72f9f5` + this package: **37 ok, 0 failed, 0 skipped**, 440 s (`build/agentFG_regression2.log`; the same harness also passed 37/37 on the `18f36a3` base earlier in the package, `build/agentFG_regression.log`)
* a clean full Release build, `build/agentFG_wt/build/agentFG_clean` **deleted first**, with
  `DISHONORED_LAYOUT_CHECKS=ON`, targets `DishonoredGame CoreSmoke LayoutProbe`:
  **0 errors, 0 C4263, 0 C4264** (`build/agentFG_cleanbuild.log`, 305 warnings, all pre-existing
  categories). All three executables produced.
* `rva_sweep.py` over the whole worktree: 7,430 citations, **3 suspects** —
  `disbehaviorpatrol.cpp:319` (`0x7398a0`) and two 2012 addresses in a comment list in
  `gfxuirenderer.cpp:1679`. **None is in a file this package touches; all three are already at HEAD.**
  Every address this package adds is resolved by hand in `agentFG_status.csv` — each confirmed a
  function *start*, with its mangled name and its size, and with the Hex-Rays `ratio 1.000` header
  where one exists.

## 8. Every file and function, for the merge

Regeneration is **not** required and was **not** run: no class, no native, no UnrealScript signature
and no member layout changed. `Sources.cmake` is **unchanged**. **Seven files**, all modified, none
added, none deleted, plus this report and `agentFG_status.csv` as new documents.
`build/agentFG_sync.py` is the authoritative copy list and flags rather than copies anything changed
in main since `f72f9f5`.

| file | functions touched |
|---|---|
| `External/GFx3/GFxText.h` | declarations only: `GFxTextWideToUtf8`, `GFxTextWideNulTerminated`, `GFxStyledText::ParseHtml`, `GFxStyledText::GetImageTagsDropped`, the `ImageTagsDropped` member |
| `External/GFx3/GFxStyledText.cpp` | **new** `GFxTextWideToUtf8`; **new** `GFxStyledText::ParseHtml` and its file-local helpers in an anonymous namespace; one member added to `GFxStyledText::GFxStyledText`'s init list |
| `External/GFx3/GFxTextField.cpp` | `GFxEditTextCharacter::SetTextValue` rewritten; **new** `GFxEditTextNoteHtmlParse` |
| `External/GFx3/GFxPlayerRoot.cpp` | `GFxMovieRoot::CreateStringW`, `GFxMovieRoot::GFxValue2ASValue` (the `VT_StringW` arm), `GFxValue::ObjectInterface::SetText(const wchar_t*)`, plus one `#include` |
| `External/GFx3/GFxPlayer.h` | one parameter renamed: `GFxDisplayList::MarkAllEntriesForRemoval(unsigned int fromFrame)` |
| `External/GFx3/GFxPlayerSprite.cpp` | `GFxDisplayList::MarkAllEntriesForRemoval` rewritten; one argument changed in `GFxSprite::GotoFrame` |
| `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | **outside this package's own area**: two entries added to `DisReportOneClip`'s `Names` array so `-disclipdiag` reports `_currentframe` and `_totalframes` |

**The overlap, measured rather than asserted.** The worktree was re-based twice —
`git checkout -f <commit>` then `git apply -3` of this package's own diff — onto `63c2539` and then
`f72f9f5`. **Both re-bases applied all seven files cleanly: no conflict, no reject.** Before FF
merged, FF's then-current working diff and this one were also applied in sequence to a clean
`git archive` of `18f36a3` with `patch -p1`: no rejects, FF's four `GFxPlayerRoot.cpp` hunks taking
offsets of +1, +1, −2 and −2 lines.

* `GFxPlayerRoot.cpp` — FF is in `CreateEmptyMovieClip` and `AttachMovie`; this package is in
  `CreateStringW`, `GFxValue2ASValue` and `SetText`, 400–900 lines above.
* `GFxPlayerSprite.cpp` — FF is in `GFxCharacterHandle`'s constructor, `GFxCharacter`'s destructor and
  `SetMask`, `GFxASCharacter`'s constructor, `EnsureASObject`, `SetName`, `GotoLabeledFrame`,
  `CreateEmptyMovieClip` and `AttachMovie`; FH is in `ReplaceDisplayObject`. **Neither is in
  `MarkAllEntriesForRemoval` or `GotoFrame`.** `GotoLabeledFrame` is the nearest, about ten lines from
  `GotoFrame`, and still merges.
* `GFxPlayer.h` — FF added `GFxCharacter`'s mask members, `GFxASDepthBias` and the drag declarations;
  this package renames one parameter of one declaration.
* `GFxStyledText.cpp`, `GFxText.h`, `GFxTextField.cpp` — **touched by neither FF nor FH.**
* `disgfxmovieplayermainmenu.cpp` — FH added `DisDriveDifficulty` and a call in
  `DisGFxMoviePlayerMainMenuPreAdvance`; this package is in `DisReportOneClip`. Merged clean.

**The one semantic collision was real**: FF deleted `GFxCharacter::bScriptCreated` in favour of the
depth test, and the first cut of `MarkAllEntriesForRemoval` used that flag. The re-base turned it into
a compile error and it is now retail's own `>= GFxASDepthBias`, which is better. A three-way textual
merge alone would not have caught it; a build did.

## 9. Hand-overs

1. **The arrow chrome on the focused row** (§6) is a labelled-frame question in the same family as
   agent FH's portrait, and FH's `ReplaceDisplayObject` fix landed after the observation. Settling it
   needs the frame dump of `Slider_widget` / `OptionsStepper_widget`'s `_leftArrow_mc.arrow_mc`.
2. **`SetWidgetFocus` / `RemoveWidgetFocus` "on undefined" is not, in the cases I could read, a
   prototype-binding fault.** Both sites in `__Packages.OptionsList` (a 5,861-byte buffer, pc 3940 in
   `HideList` and pc 5366 in `onSelectionUpdated`) are `this._curWidget_mc.RemoveWidgetFocus()`, and
   `_curWidget_mc` is **undefined on the first selection** — which is a normal AS2 no-op that this
   runtime logs as an error. Some of the 127 are that. What is **not** explained is
   `UpdateSettings` "on Sprite", which did not occur on any screen my driver reaches; that one is a
   genuine missing prototype method and needs the screen that raises it.
   The class-binding trace (`-gfxuiclasstrace`) is healthy for every options symbol it prints:
   `o_OptionsScreen`, `o_OptionsList_withTabs`, `o_options_line`, `Slider_widget`,
   `OptionsStepper_widget`, `OptionsStepperB_widget` all report `-> class, chain depth N, reaches
   MovieClip.prototype`. So the binding runs; whatever is wrong is narrower than "the registered class
   is not bound", and the next package should start from the `UpdateSettings` site rather than from
   the 127.
3. **`GFxCharacter::CreateFrame`** does not exist in this tree and retail's sweep tests it (§5). Not
   needed for anything measured here; the next package in the timeline should add it rather than
   rediscover it.
4. **`String.indexOf`, `split`, `length` and `charAt` are byte-based in this runtime.** The
   substitution works because both sides are now the same UTF-8 bytes; content that indexes *into* a
   non-ASCII string is still wrong.
5. **`GFxTextUtf8ToWide` decodes two- and three-byte sequences only.** The new encoder handles four
   (surrogate pairs); the decoder does not. Nothing in the cook needs it.

## 10. Where the numbers are

| | |
|---|---|
| `build/agentFG/sbs_modal.png` | untouched / this package / retail, the modal's line, 1600x900 |
| `build/agentFG/sbs_modal_f72f9f5.png` | the same pair repeated on the current base |
| `build/agentFG/sbs_fault2.png` | untouched above this package, same hover schedule |
| `build/agentFG/sbs_options.png` | our AUDIO screen above retail's |
| `build/agentFG/hb*_*.png` | the untouched `f72f9f5` baseline |
| `build/agentFG/fx*_*.png` | this package, same schedules |
| `build/agentFG/base*.png`, `rel*.png`, `ba1_*`, `ffA_*` | the earlier `18f36a3` runs, including every `-disclipdiag` measurement |
| `build/agentFG/*_log.txt` | each run's own `Launch.log` |
| `build/agentFG/tag_census.py` | the tag census that chose the parser's tag set |
| `build/agentFG/clean_build.cmd` | the gate build |
| `build/agentFG_sync.py` | worktree → main, report-only by default |
| `resources/docs/agents/agentFG_status.csv` | every retail address cited, resolved by hand |
