# Agent HJ — the location banner, TextField image substitutions, the profile key bindings (wave 25, PHASE22 follow-up to HF)

Worktree `build/agentHJ_wt` (base f147830, fast-forwarded to main `835dc2b` before hand-back; HI's `disgfxmovieplayerhud.cpp`
changes merged three-way and cleanly), build dir `build/agentHJ` (RelWithDebInfo), staged `DishonoredGame_HJ.exe`.
No IDA MCP and no FModel were used. All addresses below were checked in both private databases (`build/agentHJ/idb/`).

## 1. The banner: three runtime defects, none of them a tween

The brief's guess (the mask tween does not run) was wrong. With the `-puidiag` clip trace the reveal runs: `_mask_mc._width`
goes 10 -> 274.9 over the 0.95 s `tweenTo`, the tags `gotoAndPlay("open")`, `Close` fires at 5.1 s and `FadeOut` 0.6 s later.
The HUD's `__Packages.Location.SetLocation` (disassembled with `build/agentHJ/swf/as2dis.py`/`swftags.py`) builds the title from three
clips - `charAt(0)` big, `substr(1, length-2)` small, `charAt(length-1)` big - and every count comes from `locationName.length`.

1. **`"string".length` read undefined everywhere.** A primitive is boxed by `PrimitiveToTempObject`; retail boxes into a real
   `GASStringObject`, whose `GetMemberRaw` (2012 0xa62330 / 2013 0xa56180) answers `length`. This tree's `GASPrimitiveObject` had
   no such member, so `nbChar` became 3 through `undefined < 3`, `substr(1, NaN)` was empty and `charAt(NaN)` was `W`: **"W", "", "W"**.
   Ported onto `GASPrimitiveObject::GetMemberRaw` (string type only, stored member wins unless UNSET, case-insensitive name for SWF<=6).
2. **The blue box was the mask drawn as content.** `GFxDisplayList::Display` (2012 0x9d56c0 / 2013 0x9cbf20) skips every entry whose
   IsUsedAsMask (vtable +180) is true; ours drew `hud_location_Mask` (shape 3, `#0099ff` alpha 127) as an ordinary child. Agent FF's
   comment ("GFx does NOT hide a mask from ordinary rendering") read only `GFxSprite::Display` and is corrected.
3. **Every text field's `_width`/`_height` was 4 px short.** Retail `GFxTextDocView::GetViewRect` (0xa9f2c0) answers the rect at +248,
   which `SetViewRect` (0xa9a1e0) stores as given (our `TextRect`); the 40-twip inset copy is at +60. `GFxEditTextCharacter::GetBounds`
   (0xa2ed60) encloses that full rect; ours enclosed the inset one, so the banner's `prev._x + prev.txt._width - 3.5` set letters 4 px too
   close (measured: W->R gap 34 vs retail 38 px at 960 wide). Also: auto-size now shrinks as well as grows (0xa9e3a0 compares and
   resets the rect on any difference), with a re-layout of non-left-aligned text after a width change.

Before/after (banner box x .36-.64, y .24-.34, `build/agentHJ/banner_ssim.py`, SSIM vs retail banner frames t 8.8-13.9):

| run | what the capture shows | SSIM |
|---|---|---|
| HF1 | "W W" over a blue box, ornaments only at the edges | 0.33 |
| HJ1 (fixes 1+2) | "WRENHAVEN RIVER", reveal and tags animate; gaps after W and before R too small | 0.53 |
| HJ3 (all three) | "WRENHAVEN RIVER", indistinguishable by eye from retail; banner on world 52.0-56.0 = retail's | **0.889** |

`retail vs retail` in the same box is 0.998. The HJ1 run showed the card gone at world 55: the AS2 timers run on the wall clock in retail
too (`GFxMovieRoot::GetASTimerMs` 0xa03020 = `GTimer::GetTicks()/1000`), and that run's world clock lagged the wall clock by 70 % under load.

## 2. `TextField.setImageSubstitutions` / `updateImageSubstitution`

Ported: `SetImageSubstitutions` (2012 0xa2c2c0 / 2013 0xa22980), `UpdateImageSubstitution` (0xa2b6c0 / 0xa21d80), `ProceedImageSubstitution`
(0xa2bc60 / 0xa22320), the id table (`AddIdImageDescAssoc` 0xa2bba0, `RemoveIdImageDescAssoc` 0xa286e0, `ClearIdImageDescAssoc` 0xa2bb70),
`GFxTextImageDesc` (0xa26030), the doc view's substitutor (`AddImageDesc` 0xa9b1d0, `RemoveImageDesc` 0xa9b4e0, `ClearImageSubstitutor`
0xa2b670, `FindImageDesc` 0xa99da0), the image glyph in `GFxParagraphFormatter::Format` (0xa9d310: advance = ScreenWidth, ascent/descent from
the desc matrix) and the image draw of `GFxTextLineBuffer::Display` (0xa45bf0). Deviations stated in the code: the element list is searched
in insertion order rather than sorted; images are drawn after the glyph batches.

**Second defect found on the way:** the method was installed and still "not a function". The TextField prototype was ONE file-scope
pointer overwritten by every movie's install, so a field of any movie but the last loaded looked its members up with names from another
string manager. It is now `GASGlobalContext::Proto_TextField`, one per context (`GFxDrawingInstall` returns it). This also makes the
existing `getTextFormat` reachable from every movie. (`GTextFormatProto` has the same latent defect; not touched.)

Route: 0 `setImageSubstitutions` errors (HF1: 2); the tutorial field gets its 26 `$..$` entries (`puidiag imagesubst` lines). The two
route hints contain no `$..$` token on PC, so nothing visible changes in them - which is what retail shows ("Use your mouse to look around",
no icon; pair `build/cmp/HJ3/pairs/pair_w0068.022.png`).

## 3. Key names: `ReadPCBindingsFromProfile` and the profile's defaults

Ported in retail's order inside `UDishonoredPlayerInput::ApplyGameSettings` (2013 0x6bd610): `TranslateBindingSet` (0x6afe50, new),
`ReadPCBindingsFromProfile` (0x6b8240 / 2012 0x6ef8f0, new), Bindings = BaseBindings, the two `AddBindingSet`, `InitGameActionBindings`.
`OnInit` now ends with `ArkSettings::ApplyCurrentSettings(this)` as retail (0x6afeb0); agent DO's `BuildBindings` stand-in is gone.

Measured with the first build: **the profile answered 0 for all 36 actions, i.e. `m_BindableKeyMapping[0]` = None - every key unbound**
(and, exactly as in the user's recording, "[Unbound]" for movement). The fallback the brief asked for is `UArkProfileSettings::SetToDefaults`
(2013 0x53b8c0 / 2012 0x57c1b0), unported: it derives each action's default key from the PlayerInput class default's `m_PCBindings`. Ported
(new `Inc/CppText/UArkProfileSettings.h` on the shim, with `FillGBAList` 0x537610 and `GetKeyName` 0x533560). Now `id 29 value 59, W -> W`,
`id 38 value 42, F -> F`, and the move hint reads "Press [W] [S] [A] [D] to move".

**Where the plan is wrong (with the measurement):**
- Retail's "[Unbound]" / "[Mouse Scroll Down] Talk" is the USER'S PROFILE, not a code path we lack. Retail 2013 keeps the profile in the
  Steam cloud file `OPTIONS.SAV` (`UOnlineSubsystemSteamworks::CreateProfileName` 2013 0x5a76a0 returns `L"OPTIONS.SAV"`; 2012 answered
  `profile.bin`): `C:\Program Files (x86)\Steam\userdata\1919909936\205100\remote\OPTIONS.SAV` (584 bytes, written 2026-10-14 21:24, the
  recording session). Its 36 binding ids carry the same repeated value, consistent with all-zero = all-unbound. With zeros, our port
  reproduces both retail strings: W is renamed None ("[Unbound]"), F is renamed None and the first remaining `GBA_Use` binding is the user's
  ini `MouseScrollDown=GBA_Use` (`My Games\...\DishonoredInput.ini` line 126) -> "[Mouse Scroll Down]". The harness's isolated ini dir has
  no profile (and no `MouseScrollDown` Use line); Steam does not initialise there (`SteamAPI_Init: failed`), so retail's offline path
  (`SetToDefaults`) is what runs, and that now gives the ini keys. The cloud read itself (2013 `ReadProfileSettings` 0x5ad8f0's online
  branch) and the `FProfileSettingsReader` format are not ported.
- HF's `InitGameActionBindings` compared the parts untrimmed ("Trim returns a copy retail discards"). UE3's `FString::Trim`/`TrimTrailing`
  trim in place AND return a copy; 0x6bac10 compares the trimmed part. Fixed; before it `GBA_Use` (F's "GBA_Use | ...") never matched.
- Not ported: `TranslateBaseBindings` (0x6baa80) and SetToDefaults' keyboard-layout branch (both need the WindowsClient key maps), the
  2013 graphics-default tail of SetToDefaults (ids 119..123 from six startup globals), and the FGFxEngine glyph-key call (0x9a0f30).

## 4. Item 4

- `UDisSeqAct_SetTutorialMessage::PostLoad` (2012 0x8124f0, 2013 0x7b02f0, 168 bytes each; the 2013 decompile fails, 2012 used): a missing
  variable link is copied from the class default (type, description, property, writeable) and the package marked dirty.
- `execFormatText` -> `FormatText` (2013 0x600aa0 -> 0x7a9e20): `FormatInteractionText(text, FALSE, FALSE)` and the write-back is
  unconditional (an unreadable variable is written back empty, as retail). Options stage unchanged: `options_as2_errors` 39, 6 steps.

## 5. Diagnostics (`-puidiag`)

`GFxAS2ClipTracePrefix` = `_level0.location_mc` while `-puidiag`: every `_width/_height/_xscale/_yscale/_visible/onEnterFrame` SET (opcode
SetMember) and every `setMask/gotoAndPlay/gotoAndStop/tweenTo/tweenFrom/tweenEnd` CALL on that subtree, with the AS timer; every image
substitution element (`puidiag imagesubst`) and refusal; every profile key rewrite (`puidiag profile key`). The tween's per-frame writes
are not SetMember opcodes (only its start and end appear).

## 6. Numbers

| check | result |
|---|---|
| route `CC_HJ3` (`-finewgame=50 -fosoiree -puidiag -ResX=1920 -ResY=1080`) | banner world 52.0-56.0 (retail 52.0-56.0), no reset at New Game |
| banner SSIM (box) | 0.889 every banner frame (HF1 0.33); text reads "WRENHAVEN RIVER" |
| AS2 errors on the route | 11 (HF1 14), 0 new; all 11 are the known `no frame named 'PC'` |
| gate `run_regression.py --log-prefix HJg_` | 31 ok, 3 failed, 0 Critical. `coresmoke`/`layout`: targets not built (as HF). `inputtest_moved` 217/430, re-run 475 |
| inputtest control | HF's staged exe (gate value 1062 on its day) measured **359 / 360** in the same minutes: load, not this package. Re-run on a quiet machine |
| options | `options_steps` 6, `options_as2_errors` 39 (HF 39), criticals 0 |
| menu frame vs `head_rel` world 50 | same screen ("DISHONORED / PRESS ANY KEY"), SSIM 0.835 = foliage/water animation |
| `gen_classes_header.py` regen | tree unchanged (only generated change: `#include "CppText/UArkProfileSettings.h"` in the shim) |
| `bitfield_audit.py` | mismatches 0 |

Builds: five through the cmd, three over the two-build budget, each stated: 1 compile error (missing include), 2 the measured HJ1,
3 `SetToDefaults` (build 2 unbound the keyboard - not shippable), 4 the per-context prototype, 5 the text bounds. The worktree was
fast-forwarded to 835dc2b after build 5 and NOT rebuilt (HI's HUD edits and mine do not touch the same functions).

## 7. Files and functions

GFx3: `GFxAS2Object.{h,cpp}` GASPrimitiveObject::GetMemberRaw (new). `GFxDisplay.cpp` GFxDisplayList::Display (mask skip),
GFxSprite::Display (comment), GFxEditTextCharacter::Display (image glyphs). `GFxDisplay.h` GFxDrawingTextFieldProto removed.
`GFxTextDocView.{h,cpp}` GFxTextImageDesc (new), GlyphEntry::pImage, GFxTextLineBuffer::Clear/AddImageRef, GFxParagraphFormatter::Format
(image glyph), GFxTextDocView dtor/FindImageDesc/AddImageDesc/RemoveImageDesc/ClearImageSubstitutor (new), GFxTextDocView::Format (autosize).
`GFxTextField.{h,cpp}` SetImageSubstitutions/UpdateImageSubstitution/ProceedImageSubstitution/Add|Remove|Clear|FindIdImageDesc(Assoc)/
LogImageSubstWarning (new), dtor, GetBoundsTwips, GetMember (per-context proto). `GFxDrawing.cpp` GFxDrawingInstall (returns the proto,
installs the two methods), GFxDrawingBitmapImage (new). `GFxPlayer.h` their declarations. `GFxAS2Runtime.h` Proto_TextField, clip trace
globals. `GFxAS2Interp.cpp` clip trace in SetMember/CallMethod. `GFxAS2Lib.cpp` InitStandardLibrary (stores the proto).
GFxUI: `gfxuiengine.cpp` -puidiag arms the clip trace.
DishonoredGame: `dishonoredplayerinput.cpp` OnInit, TranslateBindingSet (new), ReadPCBindingsFromProfile (new), InitGameActionBindings
(trim), ApplyGameSettings; BuildBindings deleted. `arkprofilesettings.cpp` FillGBAList, SetToDefaults (new). `disgfxmovieplayerbase.cpp`
execFormatText. `disgfxmovieplayerhud.cpp` UDisSeqAct_SetTutorialMessage::PostLoad (new) - the only function of that file I touched.
CppText: `UDishonoredPlayerInput.h`, `UDisSeqAct_SetTutorialMessage.h`, `UArkProfileSettings.h` (new). Generated: `DishonoredGameEngineShims.h`.

## 8. Open

1. The Steam cloud profile read (`OPTIONS.SAV`, `FProfileSettingsReader`) - needed to see the user's own "[Unbound]" in our build.
2. `TranslateBaseBindings` and the keyboard-layout branch of `SetToDefaults`; the 2013 graphics defaults (ids 119..123).
3. `GTextFormatProto` is still one pointer for all movies (same defect as the TextField prototype had).
4. Auto-size alignment (right/centre) is not recorded by our view flags; resize is left/top anchored.
5. Re-run `inputtest` when the machine is quiet; then build `CoreSmoke`/`LayoutProbe` for the full 47.
