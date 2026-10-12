# Agent FY — the AS2 collector, the brightness keys that were never broken, and the window our own `appShowGameWindow` activates

Wave 22, PHASE18 package FY. Cut from `da77f0f`, merged to `f14326b` mid-package when the coordinator asked (the
two commits between touch `README.md` and `PHASE18.md` only). Worktree `build/agentFY_wt`; working build
`build/agentFY_wt/build/fyrel` (build 1, `build/agentFY/fy_build1.log`, 0 errors), the gate's in-harness build
`build/agentFY_wt/build/fygate` (build 2, `fygate/regression/build_DishonoredGame.log`: 0 errors, 0 C4263,
0 C4264). Every game run through `resources/tools/game_input.py` with `-windowed`, every build through
`build_queued.py`, no retail `Dishonored.exe` launch at all (none was needed), no FModel tool. Evidence in
main's `build/agentFY/` (`fy_*.log` are in the retail `DishonoredGame/Logs`, captures under `shots/`, memory
series `fy_*_mem.txt`, scripts `fy_idle.txt`, `fy_bright.txt`, `fy_opt.txt`, the runner `fy_run.sh`, the gate
`fy_regression.log`).

> **The headline.** `GASRefCountCollector` is ported - Bacon & Rajan's synchronous cycle collection over the
> reference counts, as 2013 `0xda78b0` / `0xda7b50` / `0xdd81a0` spell it out - and the idle menu's AS2 census now
> reads **live flat at about 5,000 while created climbs without bound** (`4992 live / 26412 created / 21420 freed`
> at 65 s, §2) with **private bytes flat** where the control gains 0.25 MB/s at the same frame rate. **The
> brightness slider was never stuck**: Left moves it 4-5 px per 0.1 of gamma (§3), and the brief's "before and
> after" compared a Left+Right pair that nets to zero. **The residual `WA_ACTIVE` is ours**: a `WH_CBT` hook caught
> `HCBT_SETFOCUS` with `appShowGameWindow+0x10a` under `UGameEngine::Tick` on the stack - `SW_SHOWNORMAL` through
> `SetWindowPlacement` at the first frame (§4). The gate: **47 ok, 0 failed, 0 skipped**, `unported_natives` 2
> of bound 2.

## 0. What landed

| | |
|---|---|
| `GASRefCountCollector` (`GFxAS2Object.h/.cpp`) | retail's `GRefCountCollector<323>` under the name `GASGlobalContext::GetGC` hands out: `Collect` (2013 `0xda78b0` / 2012 `0x9ae880`), `AdvanceFrame` (`0xdd81a0` / `0x9e1a00`), `ForceCollect` (`0xdd82e0` / `0x9e1b40`), `ForceEmergencyCollect` (`0xdd8330` / `0x9e1b90`), `SetParams` (`0xdd6130` / `0x9df990`), the constructor (`0xddb610` / `0x9e4e60`), `VisitChild` = the five `GRefCountBaseGC<323>` functors as inlined into `GASObject::ForEachChild_GC<F>` (`0xdd19d0` MarkInCycle, `0xdd1af0` ScanInUse, `0xdd1c00` Scan, `0xdd2100` Release, `0xdd2200` CollectGarbage, which is `0xda77c0`); `Register`, `AddRoot`, `AddToKillList`, `ExecuteKillList`, `Free`, `FreeAll` |
| `GASObject::Release` | `GRefCountBaseGC<323>::Release` `0xda7b50` / `0x9aeb20`: zero releases the children and frees, non-zero colours purple and buffers once |
| `ExecuteForEachChild_GC` / `Finalize_GC` | on `GASObject` (`0xdd24d0` / `0xdd3970`), `GASArrayObject` (`0xdcd650` / `0xdce510`), `GASSuperObject` (`0xdd8e40` / `0xdd8bb0`), `GASPrimitiveObject`, and `Finalize_GC` on `GASFunctionObject` (`0xde9290`); `GASValue::ForEachChild_GC` / `Finalize_GC` (`0xdc1700` family / `0xdc1fd0`) |
| the references retail's GPtrs hold and this tree's raw pointers did not | `GASGlobalContext` on `_global`, the eight prototypes and constructors; `RegisterClass`'s registry; `GFxASCharacter` on its AS object and prototype (`EnsureASObject`, `Set__proto__`, the sprite constructor, released in `~GFxASCharacter`); `GFxDrawing`'s four file-scope prototypes |
| `GFxMovieRoot` | `GCFrameCnt` / `GCLastCollectionFrame` (retail +24 / +28), the `AdvanceFrame` call at the tail of `Advance` (`0xe088f0`), `ForceCollectGarbage` (`0xdf9b70`) |
| the census | `AS2 objects %u live / %u created / %u freed (%u in cycles, %u collections)` on the "machine" line (`gfxuiengine.cpp`) |
| `-gfxuiwatch=<method>` | a watched call now also logs the receiver's primitive own members (`GFxAS2WatchThisVisitor`), which is what read the slider's `_selectedValue` |
| `WinViewport.cpp` | the activation hook: `SetWindowsHookEx(WH_CBT)` on the game thread under the no-grab mode, logging `HCBT_ACTIVATE` / `HCBT_SETFOCUS` with a frame walk and a raw stack scan resolved to module+rva |
| `Core/Src/UnVcWin32.cpp` | `appShowGameWindow`: `SW_SHOWNOACTIVATE` under `DishonoredInputGrabDisabled()` - **one line, not verified in a build** (§4.3) |

## 1. Where the brief was wrong

1. **"The brightness slider does not move" (STATUS defect 5's remainder, FU §1.5).** It moves. On the gate's exe,
   `key Left` on the brightness screen: `watch INVOKE 'LeftPressed' ... _selectedValue=2.2 _minValue=0.5
   _incrementValue=0.1 _bWidgetEnabled=true` → `DisOnSettingChange: id 112 (PSI_Graphics_Gamma) ... value 2.1000` →
   `ArkSettings::OnSettingsChanged ... gamma 2.1000`, and the white thumb's span on the slider row (1008x567
   capture, y 356) goes **461..492 → 456..487 → 452..483** over two Lefts and back to **456..487** on Right
   (`shots/fyb1_fy_bright0*.png`, `*_l1*`, `*_l2*`, `*_r1*`, `fy_b1.log:1532-1596`). FU's two captures bracket
   `Left, Right, Up, Down`: 2.2 → 2.1 → 2.2, and Up/Down are not slider keys (`P_Slider` defines no `UpPressed`),
   so the marker was where it started. FU's own `fu_r1.log:1632-1656` already showed the two `DisOnSettingChange`
   lines (2.1 then 2.2) and was read as "the slider does not move". Nothing to fix; `P_Slider.LeftPressed`
   (`OptionsMenu.as2.txt:16572`, pc 2611, 290 B: `if (_selectedValue > _minValue && _bWidgetEnabled) {
   _selectedValue = Math.max(_selectedValue - _incrementValue, _minValue); SoundHandler.PlaySound("Change");
   UpdateValue(true) }`) reaches `UpdateThumb(true)` → `_thumb_mc.tweenTo({_x: ...}, 0.1, Strong.easeOut)`, and
   the tween runs.
2. **"`GASObjectCollector` is the teardown half of retail's collector"** - right, and the half that was missing is
   not only the cycle collector. Retail starts every `GASObject` at count 1 for its creator's GPtr (`0xdd2ea0`);
   this tree's creators hold raw pointers and start it at 0, so a `Release()` that frees at zero would have freed
   nothing the content never stored and freed in use what it still named. Hence the two deviations in §2.2.
3. **"Find who calls `SetActiveWindow`/`SetFocus` on our own thread."** Neither, by name: `SetWindowPlacement`
   with `showCmd = SW_SHOWNORMAL` in `appShowGameWindow`, the window-restore at the first `UGameEngine::Tick`.
   The frame walk from inside the hook could not see it (every frame between the kernel's hook callback and the
   message pump is lost: `hook < USER32 < ntdll < FEngineLoop::Tick+0x576`); the raw stack scan did (§4.2).
4. **"At the user's 216 fps that is the 2 GB in 217 s."** The rate is per advance, not per render: the census's
   `frames advanced` is the movie's, and the control makes **16.7 objects per advance** (23686 advances, 561346
   created, `fy_h1.log`) at 77 fps under a build and the same ratio at 24 fps (`fy_r1`'s control-side numbers).
   The 2 GB is reached by advances, and the user's frame rate sets how many of those there are.
5. **"Two builds per package"** held, and it cost the one-line Core fix its compile (§4.3): the hook's caller was
   only known after build 2 had been spent on the gate. Said so in the sync script and the table above.

## 2. The collector

### 2.1 What retail does (2013, by hand; the 2012 names match at the rvas FR's database propagated)

`GRefCountBaseGC<323>` is a 16-byte header: vtable (`ExecuteForEachChild_GC(op)`, `Finalize_GC`), the collector
pointer (reused as the kill-list link once the object is on it), a word with the count in the low 28 bits, the
colour in bits 28..30 (0 black, 1 gray, 2 white, 3 purple, 4 in the kill list) and the buffered bit at 31, and
the root index. `Release` (`0xda7b50`): a count already at zero is left alone; a count that reaches zero runs op 0
(Release) over the children, un-buffers, `Finalize_GC`s and frees; a count that stays above zero paints purple and,
if not buffered, pushes the object on the root array (with the free-list dance `0xda7770` / `0xda7350`).
`Collect` (`0xda78b0`): for each root, purple → gray + op 1 (MarkInCycle: `--count` on every child, recurse while
not gray); then gray roots with a count left → black + op 2 (ScanInUse: `++count`, recurse while not black), gray
roots without → white + op 4 (Scan); then every root is un-buffered and a white one is painted black, op 3
(CollectGarbage: a white unbuffered child goes black, recurses, and is pushed on the kill list as colour 4) and
pushed itself; the kill list is then `Finalize_GC`'d and freed, and the whole thing repeats while the root array
refilled. `Finalize_GC` (`0xdd3970`) destroys the member hash through its `_GC` node-entry type, which forgets the
values without releasing them - the counts of a dead cycle were already subtracted by MarkInCycle - and that is why
`GASValue::Finalize_GC` (`0xdc1fd0`) exists beside `DropRefs`. `AdvanceFrame` (`0xdd81a0`, called from
`GFxMovieRoot::Advance` at `0xe088f0` with the movie's +24 / +28 counters) collects when the root count passes an
adaptive bound seeded by `SetParams` (`0xdd6130`, called from `GFxMovieDefImpl::CreateMemoryContext` `0xe138c0`
with the def's `MemoryParams`: 1000 roots, 0 frames between collections, the constructor's defaults).

### 2.2 What this tree does differently, and why

- **The free at zero is deferred to the kill list**, which runs at the tail of every advance (`AdvanceFrame`) and
  inside `Collect`. Retail frees inside `Release`. This reconstruction's interpreter and library hold raw
  `GASObject*` across statements (`OperatorNew`'s own comment describes one such window), and a free inside a
  temporary's destructor would hand them freed memory; at the tail of the advance the value stack and the local
  frames are empty. The count is re-read before the free, so an object a raw holder stored in the meantime is
  skipped.
- **Every new object goes on the kill list at birth** (`Register`), standing in for the creator's GPtr: an object
  nothing ever stored dies at the end of the frame instead of at the movie's close. `PrimitiveToTempObject`'s
  boxes and the `arguments` arrays are the bulk of the 16.7 per advance.
- **Teardown is still `FreeAll`** - `Finalize_GC` on every registered object, then delete - because not every
  holder in this tree is counted (the four `GFxDrawing` prototypes are process globals overwritten per movie, as
  before). Retail reaches the same end through its last holders and `ForceCollect` in `~GFxMovieRoot`.
- One collector per movie root rather than per memory context; the +24/+28 dance is kept as written.
- `MarkInCycle` guards `--count` at zero (retail does not): a missed `AddRef` then shows as a leak, not a free.

### 2.3 The one wrong count, found by the first run

`fy_r1` (the working build, idle menu) crashed at 95 s: `GASValue::SetProperty+0xd` with `esi = 0x3f2c1135`, from
`GASObject::GetMemberRaw+0x7f` (the `__resolve` arm) under `GFxASCharacter::GetMemberRaw` - the minidump
(`unreal-v9411-2026.10.12-09.40.16.dmp`, base `0x720000`, opened with `mdmp.py` against `fyrel`'s map) read a
double where a prototype's `pResolveHandler` had been. `GFxSprite::GFxSprite` (`GFxPlayerSprite.cpp:1559`)
assigns `pProto = MovieClip.prototype` raw, and the `Release` this package added to `~GFxASCharacter` took that
prototype's count to zero with the first sprite that died. The sprite constructor now takes the reference. The
gate build carries the fix; nothing else moved.

### 2.4 Measured

Idle main menu after one Enter, `-gfxuicensus`, the external sampler `build/agentFU/fu_mem.py` every 5 s.
`fy_h1` (control `build/head_rel`) and `fy_r1` (build 1) ran while build 1 / the gate build were compiling - the
machine, not the game, set the frame rates, and `fy_r1` ended in §2.3's crash at 95 s; `fy_r2` (the gate's exe)
and `fy_h2` (the control again) ran back to back on a quiet machine after the coordinator's pause.

| run | exe | AS2 objects at ~30 s on the menu | at the end | private bytes |
|---|---|---|---|---|
| `fy_h1` | control | `66482 / 66482` (131 s) | `561346 live / 561346 created` (402 s) | 661 → 722 MB over 93..339 s: **+0.25 MB/s** at 77 fps |
| `fy_r1` | build 1 | `4992 / 13326 / 8334 freed` (50 s) | `4966 / 40834 / 35868` (84 s) | 657 → 648 MB over 61..186 s: **flat** |
| `fy_r2` | gate exe | `4992 / 66310 / 61318 freed` (30 s) | `4990 live / 1638260 created / 1633270 freed (2 in cycles, 7 collections)` (325 s, `Exit: Exiting`) | 712.7 → 712.2 MB over 60..300 s: **flat** at ~310 advances/s |
| `fy_h2` | control | `68058 / 68058` (30 s) | `1604634 live / 1604634 created` (325 s) | 740.8 → 925.1 MB over 60..300 s: **+0.77 MB/s** at ~300 advances/s |

`live` never rises above about 5,000 on the menu, `created - freed = live` on every line, `in cycles` reads
2 over 325 s and 7 collections (the purple root buffer rarely passes its 1000 bound on the idle menu, whose garbage is
acyclic temporaries; the Options screen is where the cycle pass works, §2.5). The first capture of every run is the start screen or the menu at
its real size (1008x567 on this machine, `shots/`).

### 2.5 The Options screen (the AS2-heaviest thing the gate drives)

The gate's own `options` stage: `options_steps 6`, `options_alive 180.9`, `options_as2_errors 39` (bound 41),
`options_criticals 0`. The same drive re-run with `-gfxuicensus` (`fy_o1`, 185 s, all six `fmdrive` steps ok, `Exit: Exiting`): `5879 live / 98045
created / 92166 freed (128 in cycles, 3 collections)` at 30 s, `5555 live / 865663 created / 860108 freed (610 in cycles,
5 collections)` at the end - the tab switches build the class/prototype cycles that only the Bacon-Rajan pass can free,
and it frees them; private bytes 723.4 → 717.2 MB over 60..180 s, flat.

## 3. The brightness keys

§1.1. The path, on the gate's exe (`fy_b1.log`), with `-gfxuiwatch=LeftPressed`:

| key | log | thumb span (y 356) |
|---|---|---|
| (screen opens) | `_selectedValue=2.2` | 461..492 (`fyb1_fy_bright000003.png`) |
| Left | `LeftPressed` → `DisOnSettingChange ... value 2.1000` → `gamma 2.1000` | 456..487 |
| Left | → `value 2.0000` → `gamma 2.0000` | 452..483 |
| Right | → `value 2.1000` | 456..487 |
| Up, Down | no slider call, no change | 456..487, 456..487 |
| Escape | the screen's cancel restores `value 2.2000` | - |

## 4. The residual activation

### 4.1 The hook

`DishonoredInstallActivationHook` (WinViewport.cpp) installs a thread-local `WH_CBT` hook before the viewport
window is created, only under `DishonoredInputGrabDisabled()`, and removes it with the viewport. The proc logs
`HCBT_ACTIVATE` (with `CBTACTIVATESTRUCT::fMouse`) and `HCBT_SETFOCUS` with the target's class and title, the
foreground window, `appCaptureStackBackTrace` resolved to module+rva through `VirtualQuery`, and - because the
frame walk loses everything between the kernel's hook callback and the message pump - a scan of the top 16 KB of
the stack for addresses in executable image pages. Everything is passed on with `CallNextHookEx`; nothing changes.

### 4.2 What it saw

Every scripted run, once, at the first rendered frame (`fy_r1.log:796`, `fy_b1.log:797`, `fy_r2`):

```
activation hook at 7.94 s: HCBT_SETFOCUS, losing focus 00000000 of 008409CA class 'LaunchUnrealUWindowsClient'
  title 'Dishonored Game (32-bit, DX9)' (foreground 0001050E, thread 18708);
  stack hook < USER32+0x3c316 < USER32+0x3c169 < USER32+0x3c046 < ntdll+0x756cd < FEngineLoop::Tick+0x576 < GuardedMain ...
activation hook stack scan (40 code addresses in the top 3680 bytes): ... appShowGameWindow+0x10a UGameEngine::Tick+0x16b6 ...
window activated at 7.95 s by a non-click activation (WA_ACTIVE)
```

No `HCBT_ACTIVATE` precedes it and the foreground stays the user's window (`0001050E`), which is the thread-local
activation STATUS describes. The caller is `appShowGameWindow` (`Core/Src/UnVcWin32.cpp:1843`), called once from
`UGameEngine::Tick` after `appHideSplash` (`UnGame.cpp:3821`): it swaps the startup window procedure for
`UWindowsClient::StaticWndProc` and restores the game window with `SetWindowPlacement`, `showCmd =
SW_SHOWNORMAL` - which activates the window on its own thread. `FEngineLoop::Tick+0x576` is the return address
after `GEngine->Tick`, which is why the frame walk stopped there.

### 4.3 The fix, and its standing

`Placement.showCmd = DishonoredInputGrabDisabled() ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL;` with the `extern`
declaration `SplashScreen.cpp:318` already uses for the same gate. `SW_SHOWNOACTIVATE` restores a minimized
window as `SW_SHOWNORMAL` does, without activating. **Not compiled by this package**: both builds were spent (the
gate build was running when the scan named the caller), so the gate's clean build on merge is its first compile,
and the first scripted run on main after the merge should read **no** `window activated` line and **no**
`HCBT_SETFOCUS` from the hook. The hook stays in; it costs nothing when nothing activates.

## 5. Addresses, by hand, both databases

Private copies `scratchpad/idb/fy2013.i64` / `fy2012.i64` (from FR's). 2013 rvas are VA − `0x400000`; 2012 rvas
have image base 0 and were looked up by name at the address the 2013 decompile header names.

| function | 2013 (size) | 2012 (size) | pinned by |
|---|---|---|---|
| `GRefCountCollector<323>::Collect` | `0xda78b0` (0x29d) | `0x9ae880` (0x29d) | the three root loops, colours 3→1, 1→0/2, 2→4, the kill loop |
| `GRefCountBaseGC<323>::Release` | `0xda7b50` (0x179) | `0x9aeb20` (0x179) | `& 0xFFFFFFF` count, `0x30000000` purple, `PushBackSafe` |
| `GRefCountBaseGC<323>::CollectGarbage` | `0xda77c0` (0x67) | `0x9ae790` (0x67) | white && !buffered → op 3, kill-list push, colour 4 |
| `GASObject::ForEachChild_GC<MarkInCycle>` | `0xdd19d0` (0x119) | `0x9db220` (0x119) | the member hash walk, `GASFunctionRefBase` at +32, watchpoints at +44, `__proto__` at +24 |
| `...<ScanInUse>` / `<Scan>` / `<Release>` / `<CollectGarbage>` | `0xdd1af0` / `0xdd1c00` / `0xdd2100` / `0xdd2200` | `0x9db450` (Scan), `0x9dba50` (CollectGarbage) | same walk, the functor inlined |
| `GASObject::Finalize_GC` | `0xdd3970` (0x69) | `0x9dd210` (0x69) | `~GHashSetBase<..., GHashsetNodeEntry_GC<...>>` at +28 |
| `GASObject::GASObject(GASRefCountCollector*)` | `0xdd2ea0` (0x59) | `0x9dc6e0` (0x59) | count set to 1 |
| `GASValue::ForEachChild_GC<MarkInCycle>` / `<ScanInUse>` / `<Scan>` | `0xdc1700` / `0xdc1690` / `0xdc1780` | `0x9cae60` / `0x9cadf0` / `0x9caee0` | types 6, 8, 9 |
| `GASValue::Finalize_GC` | `0xdc1fd0` (0x18) | - | |
| `GASFunctionRefBase::ForEachChild_GC<MarkInCycle>` | `0xde83d0` (0x68) | `0x9f1bb0` (0x68) | the two refs |
| `GASRefCountCollector::GASRefCountCollector` | `0xddb610` (0x54) | `0x9e4e60` (0x54) | +40 and +68 = 1000, +24 = −1 |
| `GASRefCountCollector::SetParams` | `0xdd6130` (0x27) | `0x9df990` (0x27) | `~0u` → 0 / 1000 |
| `GASRefCountCollector::AdvanceFrame` | `0xdd81a0` (0x132) | `0x9e1a00` (0x132) | the 0.7 factor, +36..+72 |
| `GASRefCountCollector::ForceCollect` / `ForceEmergencyCollect` | `0xdd82e0` (0x47) / `0xdd8330` (0x57) | `0x9e1b40` / `0x9e1b90` | `ClearAndRelease` of the root pages |
| `GFxMovieRoot::Advance` | `0xe08280` (0x7a0) | `0xa11830` (0x7a0) | `AdvanceFrame(root+20→+12, root+24, root+28)` at `0xe088f0` |
| `GFxMovieRoot::ForceCollectGarbage` | `0xdf9b70` (0xb) | - | |
| `GFxMovieDefImpl::CreateMemoryContext` | `0xe138c0` (0x151) | `0xa1cf90` (0x151) | `SetParams(params+40, params+36)` |
| `GASArrayObject::ExecuteForEachChild_GC` / `Finalize_GC` | `0xdcd650` / `0xdce510` | - | |
| `GASSuperObject::ExecuteForEachChild_GC` / `Finalize_GC` | `0xdd8e40` / `0xdd8bb0` | - | |
| `GASAsFunctionObject::Finalize_GC` | `0xde9290` (0x73) | - | |

## 6. Left open, on purpose

- The Core line of §4.3 waits for its first compile and its first run on main.
- `GFxDrawing`'s four prototypes are still process globals shared by every open movie (pre-existing); they take a
  reference and never give it back, which keeps them alive until their movie's `FreeAll`.
- `GASObject::Release` with no collector (a string context without a global context) does nothing at zero, as
  before.
- The deferred free (§2.2) means an object held only by a raw pointer across advances is freed at the next tail;
  every such holder this package found is now counted (§0), and a missed one would show as `fy_r1`'s crash shape -
  the minidump names it in one read.

## 7. Files, and every function touched

`build/agentFY_sync.py`'s docstring is the per-function list; fourteen source files, two docs. The script diffs the
worktree against `f14326b` itself and names anything changed that it does not list.
