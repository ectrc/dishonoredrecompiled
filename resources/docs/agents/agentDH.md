# Agent DH — the menu stays up: two faults, one on each thread (2026-09-28)

Package: the only task was "the menu dies a second after it appears". It does not any more. Both of the
crashes that were open on `UI_MainMenu.MainMenu` are fixed, they were **two different defects on two
different threads**, and neither is the one the two earlier reports named.

## Result

| | before | after |
|---|---|---|
| `-startmap=Dishonored_MainMenu -startmapopen -gfxuimenu`, no `-gfxuifreeze` | dies at **8.06 s**, ~2.3 s after the interface appears | **130 s, 29,730 rendered frames, 0 critical errors** (killed by the harness timeout, not by the game) |
| the same with `-gfxuicensus` | — | **75 s**, 16,430 movie frames advanced, the census still reads `movies open 1 [UI_MainMenu.MainMenu], drawn 1` on the last frame |
| `-gfxuifreeze=30` (the coordinator's evidence run) | render-thread exception, `Address = 0x0` | no longer needed; the switch can go |
| screenshot ≥ 30 s in | impossible | `build/agentDH/menu_60s.png`, the frame at **72.70 s** |

Four files changed, all in this package's own territory: `External/GFx3/GFxPlayerSprite.cpp`,
`External/GFx3/GTypes.h`, `GFxUI/Src/gfxuirenderer.cpp`, `GFxUI/Src/gfxuiengine.cpp`.

## 1. How the stacks were got, because both reports had lost theirs

Both earlier characterisations rested on addresses with no symbol (`0x70003`, `Address = 0x0`), resolved
by hand against a link map. That is what made them wrong. The fix was to stop guessing:
`build/agentDH_wt_build.cmd` with `SYMBOLS=1` configures the same Release build with `/Z7` and
`/DEBUG`, which costs one rebuild and makes UE3's own crash handler print **function, file and line**
for every frame, on the game thread and on the render thread alike. `/Z7` rather than `/Zi` so Ninja
has no shared compiler PDB to serialise on.

Two notes for the next agent who reads a crash log here:

* The addresses in `Critical:` frames are **runtime VAs of a rebased image**, not map addresses. In my
  runs the delta was `0x290000` (`RenderingThreadMain` map `0x0065b2d0` -> log `0x008eb45f`). Resolve two
  frames, take the difference, and only then subtract.
* The address printed **for a frame** is its return address, so it lands on the statement *after* the
  call. `DrawBitmaps_RenderThread ... line 3017` is the return of the call on line 3016.
  `build/agentDH_resolve.py` (map, nearest public) and `build/agentDH_disasm.cmd` (one object file
  through `dumpbin /disasm`, so a byte offset inside a function can be read) are both in `build/`.

## 2. Fault A — the game thread: the character was freed by the list it was just handed to

**Where:** `GFxSprite::AddDisplayObject`, `GFxPlayerSprite.cpp:955`, which is `ch->IsASCharacter()`.
**What the process did:** jumped to `0x43ff8ac0` — a freed-and-reused heap block's first word read as a
vtable. Agent DC's run jumped to `0x70003` from the same place; it is the same fault with different
heap garbage.

```
Address = 0x43ff8ac0
GFxSprite::AddDisplayObject()          GFxPlayerSprite.cpp:955
GASExecuteTag::ExecuteWithPriority()   GFxPlayer.h:136
GFxSprite::ExecuteFrameTags()          GFxPlayerSprite.cpp:681
GFxSprite::AdvanceFrame()              GFxPlayerSprite.cpp:746   (a child)
GFxSprite::AdvanceFrame()              GFxPlayerSprite.cpp:758   (its parent)
GFxMovieRoot::Advance()                GFxPlayerRoot.cpp:576
UGFxMoviePlayer::Advance()             gfxuimovie.cpp:2007
FGFxEngine::Tick()                     gfxuiengine.cpp:1294
```

The sprite created the instance and then handed it to `GFxDisplayList::AddDisplayObject`, which had a
branch that did this when the depth was already occupied by an entry marked for removal and carrying the
same character id:

```cpp
if (Entries[i].pChar->GetId().Id == ch->GetId().Id)
{
    ch->Release();      // the only reference: the character is deleted here
    return;
}
```

and the caller then went on to use `ch` for another twenty lines. On a timeline loop every entry is
marked for removal and the frame's `PlaceObject` tags run again, so this fires on the first loop of the
menu — about 2.3 s after it appears, which is exactly the symptom the user watched.

**Retail does not do this.** `GFxDisplayList::AddDisplayObject` (2013 `0x9cc8f0`) is one AddRef, one
`InsertAt` and one Release: it never releases the character it is handed and never returns without
taking it. The revive is real, but it lives one level up, and it happens **before anything is created**
— `GFxSprite::AddDisplayObject` (2013 `0x9f5830`) reads the depth first, and when the character sitting
there is this very character under this very name it copies the placement, calls
`GFxSprite::MoveDisplayObject` (2013 `0x9eb080`) and returns 0. The decompile is unambiguous
(`build/agentDH_decomp/GFxSprite_AddDisplayObject_9fee10.c`, its `LABEL_48` is the create path and the
tail below it is the move):

```
v19 = GFxDisplayList::GetCharacterAtDepth(&this[24].RefCount, depth, &marked);
if ( !v19 || v19->Depth < -1 || v19->Id != pos.CharacterId ) goto LABEL_48;   // create
... name and owner checks, each of which also goes to the create path ...
GFxCharPosInfo::GFxCharPosInfo(copy, pos);
GFxSprite::MoveDisplayObject(this, copy);
return 0;
```

Both halves are now retail's. The same branch was also a latent use-after-free in
`GFxSprite::AttachMovie` and `GFxSprite::CreateEmptyMovieClip`, which both use `child` after handing it
to the list; removing it fixes all three.

## 3. Fault B — the render thread: `GAtomicInt` was not atomic

**Where:** `FGFxRenderer::DrawBitmaps_RenderThread + 0x1E5`, which the object file's disassembly shows is
the return of `call dword ptr [eax+3Ch]` — `GTexture` vt[15], `FGFxTexture::Bind` (2013 `0x5775b0`),
`gfxuirenderer.cpp:3016`. **What the process did:** the exception address was `0x3eb48000`, i.e. it
executed inside a reused heap block.

```
Address = 0x3eb48000
FGFxRenderer::DrawBitmaps_RenderThread()          gfxuirenderer.cpp:3017
FGFxDrawBitmapsCommand::Execute()                 gfxuirenderer.cpp:2935
RenderingThreadMain()                             RenderingThread.cpp:213
FRenderingThread::Run()                           RenderingThread.cpp:331
```

The texture is the glyph atlas — the one `GTexture` that `GFxDisplayGetGlyphTexture` creates once and
keeps for the whole run. `FGFxDrawBitmapsInternal` AddRefs it on the **game thread** when it enqueues
the draw and `DrawBitmaps_RenderThread` Releases it on the **render thread** when the draw executes, so
its count is written from two threads several hundred times a second. And `GAtomicInt`, which is the
type that count is declared with, read:

```cpp
T operator++() { return ++this->Value; }
T operator--() { return --this->Value; }
```

A plain read-modify-write on a `volatile long`. One lost increment takes the count to zero while the
global still holds the pointer, the object is deleted, the heap hands the block straight back out, and
the next `Bind` jumps through whatever overwrote the vptr. That is why the fault was **not
reproducible at a fixed time**: 6.94 s in the coordinator's run, 39.91 s in mine, and one 70-second run
that never hit it at all. It is also why freezing the movie did not help — the display list keeps
drawing glyphs whether or not the timeline advances, which is what made the coordinator's
`-gfxuifreeze` evidence look like a second, unrelated fault. It was a second fault, but not the one the
freeze implied.

`GAtomicInt::operator++/--` are now `_InterlockedIncrement` / `_InterlockedDecrement`. That is the whole
point of the type in GFx — `GRefCountImpl` versus `GRefCountNTSImpl` is precisely the thread-safe /
not-thread-safe split, and `GAtomicValueBase` stays `{volatile T Value}` because interlocked operations
need no extra member (which is why the PDB shows none). The fix covers every count that crosses the
seam: `GTexture`, `GRenderTarget` and `FGFxRendererImpl`'s vertex, index and bitmap-descriptor element
stores, all of which use the same AddRef-on-the-game-thread / Release-on-the-render-thread pattern.

This is the **fourth** defect in the refcount family in this corner, after agent CC's `GTexture` and
`GRenderTarget` starting at zero and agent DC's `GFxResource`. The first three were about the initial
value; this one is about the update.

## 4. Two smaller things measuring turned up

* **`FGFxSeamNote` could dereference a name it had not written.** The seam census is written from both
  threads (`GFXUI_SEAM_TRACE` sits in game-thread entry points and in `_RenderThread` bodies alike) and
  `FGFxSeamReset`, on the game thread out of `FGFxEngine::LogCensus`, rewinds the slot count without
  clearing the slots. The array is static, so an unwritten name is `NULL`, and `appStrcmpANSI` would
  take it. It now skips a slot whose name is not yet there. This is a bring-up counter with no retail
  counterpart; it is hardened, not ported.
* **`-gfxuishot=` and `-gfxuikey=` only ever saw their first entry.** Core's
  `Parse(Stream, Match, FString&, UBOOL bShouldStopOnComma = TRUE)` stops at a comma, and both switches
  are documented as comma lists and loop over commas afterwards — dead code as written. Both now pass
  `FALSE`. Found while trying to photograph the menu a minute into a run: `-gfxuishot=30,5000,12000`
  produced exactly one screenshot, at frame 30.

## 5. What the interface is doing a minute and a quarter in

The last census line of the 75-second `-gfxuicensus` run, at `[0075.00]`:

```
GFx UI census (frame): movies open 1 [UI_MainMenu.MainMenu], drawn 1,
display objects 168 (76 sprites, 57 shapes, 31 text fields, 13 bitmap fills),
63 draws, 130 triangles, 31 glyph batches / 190 glyphs, 0 masks,
atlas 56 glyphs rasterised / 60 missed;
machine: 16430 frames advanced, 460 sprites created, 1031 display objects placed,
227 action buffers, 5602 opcodes (1 unimplemented), 34 script errors
```

16,430 movie frames advanced for 1,031 placements and 460 sprites created is the shape the fix predicts:
a timeline loop that re-places the same character at the same depth is now a move, so it neither creates
an instance nor re-runs its frame-0 events. Before the fix it created one every loop.

`build/agentDH/menu_60s.png` is drawn frame 14,000 of a 95-second run of the plain Release build, at
**`[0072.70]`**, 1280x720 untouched. It is the same gamma-calibration screen as agent DC's
`mainmenu.png` and as the frame-30 shot of the same run (`[0014.26]`): more than a minute in, the
interface has neither decayed nor moved. The symbolized build's run photographed drawn frames 9,000 and
14,000 at `[0043.72]` and `[0064.57]` and looks identical.

## 6. What is NOT fixed, and is not this package's

* The **7 script errors of `UGFxMoviePlayerMainMenu::PostStart`** and the missing background and logo
  (agent DC section 5) are untouched. The census still reports 34 script errors.
* **`GFxTranslator`** is still absent, so the placeholder strings ("Text", "CHAPTER NAME") are still
  placeholders (agent DG section 5).
* **`-apshottime` does not capture the interface.** Its screenshot of the menu map is the world with no
  UI over it, although `DishonoredGFxRenderUI` (UnPlayer.cpp:1806) runs well before the `ReadPixels` at
  1915 in the same `Draw`. `-gfxuishot` does capture it, because it raises the request from inside
  `FGFxEngine::RenderUI`. Whoever owns the screenshot path should look at why; I used `-gfxuishot` and
  left `-apshottime` alone rather than touch Engine code from this package.
* **`-gfxuifreeze`** (HEAD `7be1685`) has served its purpose and can be removed; it is the coordinator's
  switch, so I left it in place.

## 7. Verification

* **Menu, no freeze, symbolized worktree build:** 130 s wall, 29,730 rendered frames at 1280x720 d3d9
  windowed, **0 `Critical:` lines**. Killed by the harness timeout.
* **Menu, the run the screenshots come from:** 100 s, 22,500 rendered frames, **0 `Critical:` lines**,
  three screenshots at `[0006.33]`, `[0043.72]` and `[0064.57]`.
* **Menu, `-gfxuicensus`:** 75 s, 32,856 census lines (16,428 drawn frames), **0 `Critical:` lines**, the
  movie still open and drawn on the last one.
* **Menu, the plain Release build (`build/agentDH_wtrel`, no `/Z7`):** 95 s, 19,320 rendered frames,
  **0 `Critical:` lines**, screenshots at `[0014.26]` and `[0072.70]`. 400 s of continuous drawing over
  the four runs, against a fault that used to fire between 7 and 40 s.
* **Before the fix, same build directory and same command line:** critical error at `[0008.06]`.
* `resources/tools/run_regression.py --build-dir build/agentDH_wtrel --no-build`: **31 ok, 0 failed,
  0 skipped, 423 s** (`build/agentDH_wtrel/regression/summary.txt`).
* Clean full Release build of the snapshot worktree `build/agentDH_wt` (`DishonoredGame`, `CoreSmoke`,
  `LayoutProbe`), 928 units, 0 errors.

## 8. A note on the shared tree

The shared working tree was being edited by another package while this one ran (a head census in
`DishonoredGame`: `dishonorednpcpawn_body.cpp`, `dishonoredplayercontroller.cpp`, a new
`Inc/disheadcensus.h`), and at one point it did not compile
(`error C2039: 'IsHiddenGame': is not a member of 'USkeletalMeshComponent'`). Everything measured and
reported here was built and run from `build/agentDH_wt`, a detached worktree at `7be1685` with only this
package's four files copied in (`build/agentDH_sync.py`), so none of it depends on that work.

## 9. Files

| File | Change |
|---|---|
| `source/Development/Src/External/GFx3/GFxPlayerSprite.cpp` | the depth is read before the instance is created and a re-place of the same character is a move (2013 `0x9f5830`); the list no longer releases the character it is handed (2013 `0x9cc8f0`) |
| `source/Development/Src/External/GFx3/GTypes.h` | `GAtomicInt::operator++/--` are interlocked |
| `source/Development/Src/GFxUI/Src/gfxuirenderer.cpp` | `FGFxSeamNote` skips a slot whose name is not written yet |
| `source/Development/Src/GFxUI/Src/gfxuiengine.cpp` | `-gfxuishot=` and `-gfxuikey=` read their whole comma list |
| `resources/docs/agents/agentDH.md`, `agentDH_status.csv` | this report |

Tooling left in `build/`, none of it committed material: `agentDH_wt_build.cmd` (`SYMBOLS=1` for a
symbolized Release), `agentDH_sync.py`, `agentDH_run.py`, `agentDH_resolve.py`, `agentDH_disasm.cmd`,
`agentDH_patch1..4.py`, `agentDH_decomp/` (the three 2012 decompiles this rests on).
