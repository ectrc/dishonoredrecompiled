# Agent JA — the GFx text-field write (wave 27, PHASE24 package JA)

Worktree `build/agentJA_wt` (base main `96cdf21`, never moved), build dir `build/agentJA` (RelWithDebInfo), staged
`DishonoredGame_JA.exe`. IDA on private copies `build/agentJA/idb/{retail2013,shipping2012}_agentJA.i64`; decompiles in
`build/agentJA/{d12,d13}/`. **One build** (of two), clean. No fmodel, no real input.

## 1. What retail does (2012 rva / 2013 rva, byte-identical per match_2012_2013.csv, re-checked by disassembly)

`GFxValue::ObjectInterface::SetText(void*, const char*, bool)` 0x9afdb0 / **0x9a8dd0**, the wide overload 0x9afee0 / 0x9a8f00,
`GetText` 0x9b0640 / 0x9a9660:

1. resolve the `GFxCharacterHandle`; gone -> false;
2. ask the character's **type virtual** (vtable +0xB0, slot 44 of the primary vftable: `GFxASCharacter` 1, `GFxSprite` 2,
   `GFxEditTextCharacter` 4 - read from the three vftables, the slot bodies are `mov eax, n; retn`);
3. type 4: call the field's own virtual `SetText(const char*, bool)` (+0x174 = 0xa2ce10 / 0xa234c0; wide +0x170 = 0xa2ce60 /
   0xa23510; `GetText(env, html)` +0x16C = 0xa2d1e0 / 0xa23890) and return true. The field's `SetText` sets/clears its html flag
   (bit 1 of the word at +228) only if it changes, then calls `SetTextValue(s, html, true)` (+0x1D4 = 0xa32e50 / 0xa293e0);
4. anything else: `"htmlText"` or `"text"` through the object interface's **ordinary** `SetMember` / `GetMember` (slots 3 / 4 of the
   +120 subobject, i.e. the isDObj `GFxValue::ObjectInterface::SetMember` path), never `SetMemberRaw`.

Ours wrote `"text"` through `SetMemberRaw`, which `GFxEditTextCharacter` does not override, so the write became a plain AS2
property and the field kept its authored `Description`.

## 2. What landed

| File | Function |
|---|---|
| `External/GFx3/GFxPlayerRoot.cpp` | `GFxValue::ObjectInterface::GetText`, `SetText(char)`, `SetText(wchar_t)` as above; the non-field branch reuses this file's `GetMember`/`SetMember(obj, name, val, true)` (retail inlines exactly that); NEW static `GFxValueTextField`, `GFxValueTextTrace`; `-jadiag` lines in `SetMember` ("text"/"htmlText") and `Invoke` ("SetText") |
| `External/GFx3/GFxTextField.{h,cpp}` | `GFxEditTextCharacter::SetText(const char*, bool)` (was an inline that skipped the html flag), NEW `SetText(const wchar_t*, bool)` (UTF-8 on the stack below 256 chars, heap above, as retail), NEW `GetText(GASEnvironment*, bool)` |
| `External/GFx3/GFxAS2Runtime.h` | `extern int GFxValueTextTraceCount` |
| `GFxUI/Src/gfxuiengine.cpp` | `-jadiag` arms the trace (4000 lines) |
| `GFxUI/Src/gfxuimovie.cpp` | `UGFxObject::execSetText`: `-jadiag` line with the calling script function, object, IsDisplayObject, text |
| `DishonoredGame/Src/dishudmarkers.cpp` | `FDisTaskMarker` ctor: `-jadiag` line (name, loc file/section, IsDisplayObject of `_description_mc` and `txt`) |

Deviations (named in the code):
- No type virtual in this tree: `GetObjectType() == Object_TextField` (13) stands in for retail's `+0xB0 == 4`; it is the test
  `GFxDrawing.cpp` and the TextField methods already use and answers the same question (only the edit text is type 4).
- The html flag lives in our view flags (`VF_Html`, what the AS2 `html` getter reads), not in a character flag word.
- `GFxEditTextCharacter::GetText(env, true)` returns the markup-free text: `GFxTextDocView::GetHtml` and the style-sheet
  original-string branch are unported (the same answer our `htmlText` getter already gives).

## 3. Step 0 - the audit (every SetText / SetMember("text"|"htmlText") / Invoke("SetText") site)

Runs: `JA1` = `cutscene_compare.py run --tag JA1 --times 55:185:2.5 -- -hidiag -jadiag` (route `-finewgame=50 -fosoiree -nomovie`);
`JA2` = `build/agentJA/ja_run.py --tag JA2 --times 150:178:1 --steps build/agentJA/ja2_steps.txt -- -hidiag -jadiag` (same route,
20 in-process `move` steps from world 147 turning the view toward the Empress); options stage with `--game-extra-args=-jadiag`
(`JAo__options.log`).

| Site (ours) | Path | IsDisplayObject | Before (head) | After (JA) |
|---|---|---|---|---|
| `FDisTaskMarker::FDisTaskMarker` dishudmarkers.cpp:167 (2013 0x7ce750) | `GFxValue::SetText` wide | `_description_mc` 1, `txt` 1 | SetMemberRaw -> plain member, field kept `Description` | `_level0.markers_mc.DisInst_0._description_mc.txt field 1 ... before 'Description' text 'Empress Jessamine' now 'Empress Jessamine'` (world 150.6) |
| `FDisTaskMarker::Update` :221 (2013 0x7c57f0) | `GFxValue::SetText` wide, focused only | 1 | same defect | `before 'Empress Jessamine' text 'Empress Jessamine [62m]'` (JA2 world 155.8, first second with flags 0x3) |
| `UGFxObject::SetText` gfxuimovie.cpp:461 (script `execSetText`) | `GFxValue::SetText` wide | logged per call | same defect | **0 calls** on the newgame route (JA1, JA2) and on the options stage |
| `UDisGFxMoviePlayerHUD::UpdateCrosshairFocusText` disgfxmovieplayerhud.cpp:2578 | `Invoke("SetText")` = the HUD clip's **AS2 method**, not GFxValue::SetText | - | unaffected (an AS2 `.text` assignment is the SetMember opcode, which always reached SetTextValue) | 0 calls (the interaction context is empty until JH) |
| any `SetMember("text"/"htmlText")` (native: none in this tree; script `SetString`) | `ObjectInterface::SetMember` -> `GFxEditTextCharacter::SetMember` -> SetTextValue | - | already correct | 0 calls on both routes |
| `UGFxObject::GetText` (2013 0x57e350 is the only GetText caller) | `GFxValue::GetText` | - | read the plain member | now the field's text; 0 calls observed |

Retail-only callers of the two SetText overloads with no body in this tree (2013 xrefs): `FDisHeartMarker` ctor/Update
(0x7cebd0 / 0x7c5d00, unported per agentHI.md), a second `FDisTaskMarker` ctor 0x7ce490, the main menu's missions/DLC page 0x7da400
(`SetMissionsMenu`, `t_DownloadableContent_Caps`), an aligned-label setter 0x8c1b10 and the combo/countdown HUD 0x8c9460 (narrow
overload). All get the retail write once ported.

## 4. Step 2 - the focus bit

`UDisGFxMoviePlayerHUD::PreRender_Markers` (ours disgfxmovieplayerhud.cpp:1995-2007) = 2013 0x7a0890: one index, the visible
`m_bCanFocus` marker closest to the movie centre within `m_fMarkerFocusDistance`^2, gets `|= 2 * (index == focused)`; nothing else
sets bit 1. `FDisTaskMarker::Update` shows `_description_mc` only with bit 1 (and not in combat). Measured: JA1 (no look input)
flags 0x1 throughout, description hidden, as retail; JA2 0x1 until the marker reached x 472 of 1280, then 0x3. Not every marker
gets bit 1: nothing to port.

## 5. Acceptance

- Capture `build/agentJA/JA2/w0170.014.png` (and w0156..w0178): the chevron over the Empress reads **"Empress Jessamine [62m]"**;
  no "Description" in any frame. JA1 frames (`build/cmp/JA1/ours`, no look input): chevron only, as retail when unfocused.
- `options_as2_errors` 39 (head_rel 39 per agentHJ/agentID; bound 41), on both options runs.
- Gate `run_regression.py --build-dir build/agentJA --no-build --exe-name DishonoredGame_JA.exe --log-prefix JAg_ --only
  d3d9,inputtest,newgame,options` (`build/agentJA/gate.txt`): 26 ok, 2 failed - `d3d9_frames` 930 and `inputtest_moved` 327.9/217.2
  under the load of five other agents' builds; rerun `JAg2_ --only d3d9,inputtest` (`gate2.txt`) 18/18 ok: d3d9_frames 1410,
  inputtest_moved 1030.4. All criticals 0, `unported_natives` 0, `probe_natives` 3, `newgame_pawns` 29, `options_steps` 6.

## 6. Where the brief was wrong (with the measurement)

1. **"Accept: a capture at world 60-80"** - at world 60-80 the player is on the boat ride (soiree exit at world 150.5) and no
   objective exists yet; the marker is created at world 150.6 (`jadiag marker` line). And on the unattended route the marker is never
   focused (JA1: flags 0x1 every second), so its description is hidden there exactly as in retail: a look input is needed (JA2).
2. **"Retail 0x9afdb0 calls SetTextValue"** (PHASE24 §4 lists 2013 rvas): 2013 0x9afdb0 is `GMutex::GMutex`; the function is 2012
   0x9afdb0 = 2013 0x9a8dd0. And it does not call SetTextValue directly: it type-tests through vtable +0xB0 and calls the field's
   virtual `SetText` (+0x174), which updates the html flag and then calls `SetTextValue` (+0x1D4); non-fields go through
   `SetMember`, not `SetMemberRaw`.
3. **"Defect 4's SetText also blanks crosshairInfosTxt_mc"** (§7): `UpdateCrosshairFocusText` invokes the HUD's AS2 method
   `SetText` (`Invoke`), which never went through `GFxValue::SetText`; that field was not affected by this defect.
4. **"Every UGFxObject::SetText is affected"** - in principle, but none runs: 0 script SetText calls on the newgame route and on the
   options stage. The visible victims were the two marker writes.
5. Retail `UGFxObject::SetText` (2012 0x5d5380 / 2013 0x595680) runs `TTranslator::Translate_Helper<FOutputToGfxPolicy>` and
   sends markup through `SetMember("htmlText")`, plain text through the **narrow** overload; ours passes the raw string to the wide
   overload. Same result for markup-free text (no call on the measured routes); left as a boundary (it needs FOutputToGfxPolicy from
   the excluded ScaleformMovie.cpp).

## 7. Open

- `GFxTextDocView::GetHtml` (the `htmlText` getter and `GetText(env, true)`).
- `UGFxObject::SetText`'s translator branch (6.5).
