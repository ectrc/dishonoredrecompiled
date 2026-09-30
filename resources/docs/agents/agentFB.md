# agent FB — the ground-truth harness, and the fonts (wave 17)

Base: `e02f5f8`. Worktree `build/agentFB_wt`. Captures `build/agentFB/`.

Three things landed: the dismod harness works and is written down; retail's own answers for this
wave's group A, group B and group D are published under `build/agentFB/retail/`; and **the wrong
font on every header is fixed at its root cause**, which is not what the brief supposed.

---

## 0. The headline

The header text on the brightness, difficulty and options screens was never a *font* problem. Retail's
menu has exactly two fonts, we have both of them registered, and nothing in the menu ever fails to
find a font it names. What fails is **which name a text field is asked to look up**.

Every cooked Dishonored text field's content is HTML, and every one of them opens with
`<p align="..."><font face="$TitleFont">` or `<font face="$NormalFont">`. **That `face` attribute is
what decides the font.** Retail applies it in `GFxStyledText::ParseHtmlImpl` (2013 rva `0xa9d870`,
6,268 bytes), which this tree does not port: `GFxEditTextCharacter::SetTextValue` strips the markup
and throws the attribute away. The font then comes from the DefineEditText **font id** instead, and
that id resolves through the movie's export table, which for the cook answers:

| field | `Desc.FontId` | what `GetExportedName(id)` gave | what the HTML asked for | what it got |
|---|---|---|---|---|
| options header / tabs | 220 | `__Packages.OptionsList` (an AS2 class export) | `$TitleFont` | **font 0 = ChaletComprime** |
| options header / tabs | 10 | *nothing* (UI_MainMenu) | `$TitleFont` | **font 0 = ChaletComprime** |
| menu bar entries | 4 | `m_nGame_bkgdMenu -nopack` (UI_MainMenu) / `$NormalFont` (UI_OptionsMenu) | `$NormalFont` | ChaletComprime — right by luck |
| start screen, bar | 53, 32, 31 | `$TitleFont` / `$NormalFont` | the same | right |

`GFxEditTextCharacter::GetInitialFormats` then falls back to `GetFontByIndex(0)`, which is
`ChaletComprime-CologneEighty` because it is registered first. **That is the whole fault**: a field
whose id happens to resolve gets the right font, a field whose id does not gets font 0, and font 0 is
right for body text and wrong for every header.

It matches the user's clue exactly — "the main menu bar is right and every header is wrong" — because
the bar entries' ids resolve (or fall back to the font they wanted anyway) and the headers' do not.

**The fix**: read the first `<font face="...">` out of the field's HTML and resolve *that* first,
keeping the font id as the fallback. 31 lines in `GFxTextField.cpp`.

---

## 1. Measured, before and after, on one binary

`-gfxuinohtmlface` (added here) turns the new lookup off, so both sides are the same executable and the
same run configuration. Both runs: `-gfxuimenu=UI_OptionsMenu.OptionsMenu -apshottime=30 -windowed
-ResX=1600 -ResY=900`, screenshots taken **in process** on the game thread.

| | fields | by html face | by font id | fell back to font 0 |
|---|---|---|---|---|
| `-gfxuinohtmlface` (= HEAD's behaviour) | 34 | 0 | 25 | **9** |
| default (the fix) | 34 | 33 | 0 | **1** |

The nine were the `OPTIONS` header and the `GENERAL` / `CONTROLS` / `GRAPHIC` / `AUDIO` / `GAMEPLAY` /
`USER INTERFACE` tab labels — every field the user listed for that screen.

Side by side, same run, same second:

* **`build/agentFB/shots/optA_before.png` and `optA_after.png` are the pair to look at**: the same
  binary, back to back, one with `-gfxuinohtmlface` and one without, at the same shot time, with the
  same background and the same option list. `OPTIONS`, `AUDIO`, `CONTROLS`, `GRAPHIC` and `GAMEPLAY`
  are in the condensed sans in the first and in **Emerge BF** in the second — the face the retail
  reference `resources/reference/menu/4.png` shows on its left half. Measured over the two crops:
  the header-and-tabs band differs in **12,241** pixels above a 24-level threshold (mean 6.45) and the
  option list below it in **127** (mean 0.43), i.e. the change is confined to the headers;
* `build/agentFB/shots/options_before.png` (the untouched HEAD exe `DishonoredGame_FBH.exe`) and
  `options_after.png` — the same comparison across two different executables rather than one, which
  is the literal before/after the acceptance asks for. A pixel diff of *these* two is meaningless
  because the 3D scene behind the menu is animating and the two runs are not frame-locked;
* `build/agentFB/shots/mainmenu_before.png` / `mainmenu_after.png` and `bar_before.png` /
  `bar_after.png` — the start screen, unchanged, to show nothing that was already right regressed.

The remaining fallback is one field, id 51, export name `o_options_line_s`, whose text is the plain
string `OK` with no HTML at all, so there is no `face` to read. It is left as it was.

**What is NOT shown side by side, and why.** The brightness screen (`gammaSetting_mc`) and the
difficulty screen (`newGame_mc`) are sub-clips of movies that only open through the menu's own
navigation, and **the menu could not be navigated in this wave**: keys reach the movie (the GFx input
census counts `6 key downs, 7 key ups, 13 events HE_Handled, 13 listener calls`) but the selection
never moves, and `0 mouse events` reach it at all. The GFx focus probe says
`focus NULL | canFocus 0 | canInput 0`. That is a separate defect and not this package's; the font
evidence for those two screens is the route census above, which covers **every** field the cook
creates, in both movies.

---

## 2. Every address cited, resolved by hand

`rva_sweep.py` over the whole tree: 7,350 citations, `MISLABELLED-2012` 2 and `UNKNOWN-CLAIMED-2013` 1,
all three pre-existing in `gfxuirenderer.cpp` and `disbehaviorpatrol.cpp` and none of them mine. Over
`source/Development/Src/External/GFx3` alone: 630 citations, **0 suspects**.

That is necessary and not sufficient, and this wave proved why. Three addresses in the comments this
package touched pass `rva_sweep` as `ok-2013-mid` and are **not 2013 addresses at all** — they are the
2012 ones, and each lands mid-function inside an unrelated 2013 class:

| cited | what the comment claims | what VA `+0x400000` actually is in the 2013 database |
|---|---|---|
| `0xa27860` | `GFxEditTextCharacter::GetInitialFormats` | inside `GFxEditTextCharacter::PropagateMouseEvent` (`0xe277e0`) |
| `0xa32e50` | `GFxEditTextCharacter::SetTextValue` | inside `GFxPathPacker::SetMoveTo` (`0xe32cc0`) |
| `0xaa7630` | `GFxStyledText::ParseHtmlImpl` | inside `GFxMorphCharacterDef::ComputeBound` (`0xea71a0`) |

GFx3's convention is 2012 RVAs (`GFxFont.h` says so once, at the top of one file), so those comments
are not wrong — they are unlabelled. The three I touched now carry both builds:

| function | 2012 rva | 2013 rva | how the 2013 one was pinned |
|---|---|---|---|
| `GFxEditTextCharacter::GetInitialFormats` | `0xa27860` | **`0xa1df60`** | `func_query` by mangled name, `?GetInitialFormats@GFxEditTextCharacter@@QAEXPAVGFxTextFormat@@PAVGFxTextParagraphFormat@@@Z` |
| `GFxEditTextCharacter::SetTextValue` | `0xa32e50` | **`0xa293e0`** | same, `?SetTextValue@GFxEditTextCharacter@@UAEXPBD_N1@Z` |
| `GFxStyledText::ParseHtmlImpl<wchar_t>` | `0xaa7630` | **`0xa9d870`** | same; size `0x187c` = 6,268 bytes, which is the byte count the existing 2012 comment gives, so the two are the same function |

The addresses the harness itself hooks are all **2013** and every one was looked up by address *and*
confirmed by its first logged argument at runtime (`$TitleFont` out of `FindOrCreateHandle` is not
something an unrelated function would print). The table is in `resources/docs/dismod_harness.md` §8.

---

## 3. The harness

`resources/docs/dismod_harness.md` is the whole loop: build, deploy, run, capture, drive, and the
traps. The short version:

* one change to an existing dismod file other than two call sites — `include/logger.h`'s log path,
  now `%DISMOD_LOG%` → `%DISMOD_LOG_DIR%\dismod.log` → a project default;
* one new pair of files, `src/mods/uiprobe.{h,cpp}`, off unless `DISMOD_UIPROBE=1`;
* it reads and never writes: eight hooks that log and forward, plus `GFxMovieView::GetVariableStringW`
  and `GFxMovieRoot::Invoke` for the display list and `UObject::GObjObjects` for UE3 reflection;
* it is driven by a **command file**, not hotkeys, because window focus on this machine is not
  reliable.

Four helper scripts are in `resources/tools/`: `dismod_build.cmd`, `dismod_run.ps1`,
`retail_input.ps1`, `retail_shot.ps1`.

### The foreground, again

Two `Windows Security Alert` dialogs held the foreground for this whole wave, as they did in wave 16.
The measurement that is new, and that the other packages can use:

* `SetForegroundWindow` on the game **succeeds** if you `AttachThreadInput` to the **current
  foreground window's** thread as well as the target's. Attaching only to the target's thread silently
  fails. `resources/tools/retail_input.ps1` does it and it worked every time.
* With the foreground taken, `SendInput` **keyboard** reaches our own build (proved: the GFx input
  census counts the presses). It does **not** steer retail's menu, and `PostMessage(WM_KEYDOWN)` does
  not either, and `SendInput` mouse motion produced `0 mouse events` in our build's census. Each of
  those was checked by re-dumping the display list or re-reading the census afterwards, not assumed.
* PowerShell is not DPI-aware by default and the game window sits at x≈3000 on a second monitor here;
  without `SetProcessDPIAware()` a screen capture lands hundreds of pixels away and looks like a
  different screen entirely. That cost an hour and is why `retail_shot.ps1` exists.
* **The dialogs were not answered.** They are the user's decision.

---

## 4. What retail says (`build/agentFB/retail/`, with its own README)

For **FB** — `fonts.log`. Two fonts, `$TitleFont` → `Emerge BF` and `$NormalFont` →
`ChaletComprime-CologneEighty`, always with flags `0x00`, and **no fallback anywhere**: each resolves
`moviedef:HIT fontmap:HIT fontlib:HIT` and every later request is answered from the handle cache.
Across every movie loaded at the main menu, **141 text fields — 95 on `$NormalFont` and 46 on
`$TitleFont`**, all `bold=0 italic=0`. `GFxFontLib::AddFontsFrom` runs twice per menu load, with two different
`GFxMovieDef*` (the probe logs the pointers, not the names). There is no third font and no style variant
anywhere in retail's menu, which is what makes "the header has the wrong font" a two-way choice and
not a missing asset.

For **FC** — `displaylist.log`. The group-B clips at the main menu, with depth and scale:
`_root.help` at (1184, 651) depth 0 with `_alpha = 0`, and `_root.help._props._x = 1184` — which
confirms agent EY's diagnosis from the other side, because ours reads 0;
`_root.optionsMenu_mc` at (640, 360) depth −16290; `_root.newGame_mc` at (640, 360) depth −16334 with
`_menu_mc` at (−426.5, −25.95) rotated −4.0003°; `_root.vignette_mc` at (640, 360) **depth +1**.
Note that `_root.optionsMenu_mc._x` is **640, not −450** — the −450 EY measured is a child inside the
separate `OptionsMenu.gfx` root.

For **FE** — `camera.md`, `camera.log`, `postprocess.md` and `ue.log`. The coordinator asked for the
**live** menu FOV after `ADishonoredPlayerCamera::UpdateViewTarget` (2013 `0x6d80a0`) has run, because
every `CameraCache` FE had read was a class default (`TimeStamp=0`, zero location). The probe grew a
`cam` / `camwatch` pair for exactly that, and the answer is measured, live and stable:
`PlayerCamera.CameraCache.POV.FOV` = **90.0**, `TimeStamp` advancing 68.77 → 71.99 across twelve
samples, `ViewTarget.Target` = `CameraActor Dishonored_MainMenu.TheWorld.PersistentLevel.CameraActor`,
`AspectRatio` 1.77778, camera at `(4223.34, -7119.98, 1894.32)` rotated
`Pitch 456 / Yaw -9904 / Roll -1` (`+2.5049° / -54.4043° / -0.0055°`). **There are eleven identically
named `CameraActor`s in that level, all `FOVAngle = 90`, at eleven different transforms** — only one
is the view target, and picking another would move the frame without changing the FOV, which is the
most likely thing an FOV-only image fit absorbed into its 61 ± 2. `ue.log` is 11,224 lines ending in
its own summary; `ue_raw.log` has a second, incomplete dump appended — that pass **froze the game
thread** 331 lines in, so treat `ue` as one-shot. The rendered menu FOV is `CameraCache.POV.FOV` = **90** (not the camera's
`DefaultFOV = 75`, not the controller's `FOVAngle = 85`); the view target is the menu map's
`CameraActor`, itself `FOVAngle = 90`. The active post-process chain is `Transient.PostProcessChain`,
an instance of `AltScreen_Effects.PostProcessChain.Test_PPG`, **42 `UArkPpNode`s**, switches
`bUnderWater = false` and `bBendTime = false`, every node's parameters dumped three struct levels deep.
Our own render census says `FArkPp 1 nodes rendered, 5 draws`.

---

## 5. Where the brief was wrong

1. **"a font the headers name that we have not registered, so they fall back"** — no. Both fonts are
   registered and both resolve. The headers never name a font at all through the route the code reads;
   the name they *do* carry is in the HTML the tree throws away.
2. **"Agent EX's `GFxTextDocView::FindFont` trace names a `FALLBACK` explicitly and is already in the
   tree: one run over these screens tells you what each field resolves to."** It does not, and I had
   this half wrong myself before checking: that trace fires exactly **twice** per run, on the main
   menu and on the options movie alike, and **both lines take the `handle` route**:

   ```
   GFx font resolved: 'ChaletComprime-CologneEighty' -> 'ChaletComprime-CologneEighty' (handle)
   GFx font resolved: 'Emerge BF'                    -> 'Emerge BF'                    (handle)
   ```

   By the time a format reaches `GFxTextDocView::FindFont` it already carries the handle that
   `GFxEditTextCharacter::GetInitialFormats` put on it, so that function **can never report a
   fallback**, whatever the screen looks like. EX's "zero fallbacks on the start screen and main
   menu" is true and tells you nothing: it is measuring a branch that is unreachable on this path.
   The trace that answers the question had to be added at `GetInitialFormats`, and it is now in the
   tree.
3. **`GFxTextDocView::FindFont`'s own comment says "Retail falls back to the *first* registered font
   rather than to nothing".** Retail does not. `0xa92ca0` decompiles to: the cached handle, then the
   format's own handle, then `CreateFontHandle` on the font list, then `LogError("Missing font \"%s\"
   in \"%s\". Search log:\n%s")` and **`GFxFontManager::GetEmptyFont()`**. There is no font-0 fallback
   in retail at all. Left alone here because changing it is a behaviour change on a path nothing
   reaches; flagged for whoever ports the styled-text engine.
4. **`resources/docs/symbols/functions_2013.csv` names `GFxFontManager::CreateFontHandle` at
   `0xa489d0` with the signature `(char const*, unsigned, bool, FontSearchPathInfo*)`.** That overload
   exists, but the one the text engine actually calls is the licensee overload at **`0xa66300`**,
   `(char const*, bool bold, bool italic, bool device, bool splitList, FontSearchPathInfo*)`, which
   assembles the flag word as `(device?0x10:0) | (italic?1:0) | (bold?2:0)`. Hooking the one the CSV
   names would have logged a fraction of the calls.
5. **`-gfxuimenu` is required to get a menu at all.** `DISHONORED_PLAY_DEFAULTS` adds it (and
   `-nomovie -windowed -ResX/-ResY -nosteam -skipnativepkgs`) to the *user's* build, but
   `resources/build-release.cmd` does not define that, so a plain run of a worktree build loads
   `DishonoredGameFull_P` and sits there with `0 movie(s) open`. Two runs were wasted on that.
   `-unattended` is worse: it suppresses the menu outright.

---

## 6. Acceptance

| | |
|---|---|
| harness working, documented, captures published | `resources/docs/dismod_harness.md`, `build/agentFB/retail/README.md` |
| each header font matching the Menu Action Buttons | options screen: yes, side by side with the retail reference. Brightness and difficulty: the route census covers their fields, **the screens themselves could not be opened** (§1) |
| before/after against untouched HEAD | `options_before.png` (HEAD exe) vs `options_after.png`, plus the same-binary `-gfxuinohtmlface` control |
| `run_regression.py` | **37 ok, 0 failed, 0 skipped**, twice: 836 s on the first build and 454 s on a second, full run made after the last source edit so the gate matches the merged source exactly (`build/agentFB/logs/regression.log`, `regression_run1.log`) |
| clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | **0 errors, 0 C4263, 0 C4264**, exit 0, 995 targets (`build/agentFB/logs/build_clean.log`) |
| `rva_sweep.py` + by-hand resolution | §2 |
| this file and `agentFB_status.csv` | here, in the worktree |
| `build/agentFB_sync.py` run worktree → main | 11 copied, **0 flagged**, 0 missing — nothing this package touches was touched by FC, FD or FE |

## 7. Files changed

In **our** tree (11):

```
 M source/Development/Src/External/GFx3/GFxTextField.cpp     the fix, the trace, the census
 M source/Development/Src/External/GFx3/GFxTextField.h       GFxTextNoHtmlFace
 M source/Development/Src/External/GFx3/GFxTextDocView.h     GetDefaultParagraphFormat accessor
 M source/Development/Src/GFxUI/Src/gfxuiengine.cpp          one line: parse -gfxuinohtmlface
 ?? resources/docs/dismod_harness.md
 ?? resources/docs/agents/agentFB.md
 ?? resources/docs/agents/agentFB_status.csv
 ?? resources/tools/dismod_build.cmd
 ?? resources/tools/dismod_run.ps1
 ?? resources/tools/retail_input.ps1
 ?? resources/tools/retail_shot.ps1
```

`gen_classes_header.py` is **not** required and was not run: no reflected class or property changed.
`Sources.cmake` is unchanged: no translation unit was added or removed.

Outside `External/GFx3`: `source/Development/Src/GFxUI/Src/gfxuiengine.cpp` (four lines, one
`ParseParam`) and the seven files under `resources/`.

In the **user's** repository `D:\Christmas\github\dismod\` — not ours, not merged, nothing
committed:

```
 M include/logger.h        the log path: %DISMOD_LOG% -> %DISMOD_LOG_DIR%\dismod.log -> a project
                           default, the directory created if missing, a session banner per run.
                           Marked "dismod-harness". Nothing else about the logger changed.
 M library.cpp             one #include, one dismod_uiprobe_enabled() helper and one guarded
                           uiprobe::init() call. Off unless DISMOD_UIPROBE=1, so a normal run of
                           their mod behaves exactly as before.
 M CMakeLists.txt          one source line, src/mods/uiprobe.cpp
 ?? src/mods/uiprobe.h     new, the probe's interface and the command list
 ?? src/mods/uiprobe.cpp   new, the whole probe: eight read-only hooks, the AS display-list reader,
                           the UE3 reflection dump and the command-file watcher
```

Their `Binaries\Win32\dinput8.dll` in the Steam install was replaced with the built DLL several
times — that is the documented loop — and the contents found there first were copied to
`dinput8.dll.preFB.bak` beside it before the first overwrite. Their own `_dinput8.dll` is a
**different** build (different md5) and was not touched.

Nothing was committed in either repository.

## 8. Hand-overs

1. **`GFxMovieDataDef::GetExportedName` answers with the wrong kind of symbol.** Font id 220 gives
   `__Packages.OptionsList`, id 4 gives `m_nGame_bkgdMenu -nopack`. The same function is what
   `GFxSprite::AddDisplayObject` uses to key `Object.registerClass`
   (`GFxPlayerSprite.cpp:1694`), so a class binding is being looked up in the same mixed table. The
   font path no longer depends on it, but that does not make it right.
2. **`GFxStyledText::ParseHtmlImpl` (2013 `0xa9d870`, 6,268 bytes) is still unported**, and with it
   `<font size>`, `<font color>`, `<b>`, `<i>`, `<a>`, `<img>` and per-run formatting. This package
   reads one attribute out of the markup; the rest is still dropped by `SetTextValue`. Anything that
   needs two fonts or two sizes *in one field* will still be wrong.
3. **Retail's missing-font path is `GetEmptyFont()`, not font 0** (§5.3) — and retail also
   synthesises a bold/italic face from the unstyled one when the styled face is missing
   (`CreateFontHandleFromName`, 2013 `0xa486f0`, logs `Font "X" will be generated from "X"`). Neither
   is in this tree.
4. **The menu takes key events and does nothing with them** (§1). `13 events HE_Handled`,
   `13 listener calls`, `focus NULL | canFocus 0 | canInput 0`, selection unchanged; and
   `0 mouse events` reach the movie at all under `-gfxuimenu`. Whoever owns menu input should start
   from the census rather than from a driver.
5. **`GTessellator` and edge quality**: not measured. This package changed which font a field resolves,
   which changes the glyphs but not how they are rasterised, and no claim about text clarity is made
   either way. The brief asked for a measurement before a claim; there is none, so there is no claim.
