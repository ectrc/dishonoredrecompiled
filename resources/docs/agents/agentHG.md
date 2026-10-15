# Agent HG — the mission briefing page and the clouds shot (wave 25, PHASE22 package HG)

Opus 5.5, worktree `build/agentHG_wt` (detached at `fcad6f8`), build dir `build/agentHG` (RelWithDebInfo,
`-DDISHONORED_WITH_BINK=ON`), staged as `Dishonored_Latest2026/Binaries/Win32/DishonoredGame_HG.exe`. Two builds
(build 1 = everything below except the two fixes marked *build 2*; build 2 = final). Every number below is from
build 2 unless it says otherwise.

## Result in one paragraph

**After New Game our build now shows the same briefing page as retail and then the same clouds.** The page is the
Bink loading movie `LoadingEmpressTower.bik` with Arkane's overlay manager drawing `DUNWALL TOWER`, the hint and
`LOADING` while the level loads, then `RETURNING HOME` on its dark band, the briefing paragraph and
`PRESS ANY KEY TO CONTINUE`, and it **waits** - the game thread blocks in `GameThreadWaitForMovie` until a key
arrives (an `-inputcmd` `key`, never real input). The key starts `INTRO_LOC.bik` (the clouds), which Escape skips,
and the game goes on to the boat. Overlay areas against retail t 4-5.5: **SSIM 0.88-0.99** (bound .7); clouds
**SSIM 0.98** against retail t 8.1. `-nomovie` is untouched and stays the gate default.

## 1. What the clips are (measured before porting)

ffmpeg 8.1 decodes Bink 1. Decodes in `build/agentHG/movies` (2 fps) and `movies10` (10 fps, first 16 s).
`build/agentHG/match_movies.py` scores every retail.mp4 frame against every movie frame with HH's SSIM-lite:

| retail t | what it is | best match |
|---|---|---|
| 1.5-2.3 | the menu | nothing (<.40) |
| **2.4-5.5** | the briefing page | **LoadingEmpressTower.bik** from its frame 0 (SSIM .93 at 2.5, .86-.87 at 4-5.5 once the overlays are on); `Loading.bik` is a Corvo poster, never on screen here |
| 2.4-3.8 / 4.0-5.5 | page state | 2.4-3.8 hint + pulsing `LOADING`; from 4.0 chapter title + paragraph + `PRESS ANY KEY` (contact sheet `build/agentHG/retail_page_seq.png`) |
| 5.6-8.1 | the clouds | **INTRO_LOC.bik** from its frame 0, 1:1 in time (5.6→0.0, 6.0→0.4, 8.0→2.4 s; SSIM .96-1.0) |
| 8.2- | black, banner at 8.8 | INTRO_LOC cut at movie time 2.6 of 38.9 s: the user skipped it |

So the clouds are **not** a level Kismet sky shot; `-diskismet`/`-dismatinee` were not needed. They are the intro
movie `[FullScreenMovie] l_tower_p=INTRO_LOC` (`DefaultEngine.ini:709`) that `UEngine::PlayLoadMapMovie` hands to
`FDisFullScreenMovieBink::GameThreadPlayLoadingMovieAndIntro`, played by `GameThreadRequestDelayedStopMovie` after the
page's key. Retail's sequence: menu → page (key wait from 4.0) → key at 5.5 → INTRO_LOC → skip at 8.2 → black + banner.

## 2. Bink on

* `DISHONORED_WITH_BINK=ON` built clean the first time (the glue of agents U/Y compiles and links; the exe imports
  `binkw32.dll!_BinkOpen@8` etc. from the retail DLL already in `Binaries/Win32`).
* **The Bink global shaders needed nothing**: with `USE_BINK_CODEC=1` the three `IMPLEMENT_SHADER_TYPE`s of
  `binktexturesrhi.inl` exist and the cooked cache loads them - `HG1.log`: `263 shaders, 261 loaded, 2 undeclared
  types` (only the two MeshPaint types remain) where `play_w24e.log` had 258 loaded / 5 undeclared.
* Startup movies obey `-nomovie` (unchanged `appInitFullScreenMoviePlayer`); without it they play and end on their
  own (startup 29.2 s vs ~4.4 s with `-nomovie`), the last one (`Loading`) ends on `MapsToAutoStart` for the menu.
  The display mode is never changed (the movie renders into the game viewport, `-windowed` holds).
* **build 2 fix, the reason nothing would ever have been drawn**: the reference `FFullScreenMovieBink::Tick` passed
  `bIsLoadingMovie = MovieMode == MM_LoopFromSuppliedMemory` to `RenderFrame`, so for Arkane's `MM_LoopFromStream`
  loading movies `RenderFrame` never entered its overlay block and never called `OnBinkRenderFrame`. Retail 2013
  `0x4fc720` passes `(MovieMode >> 7) & 1` = `MF_LoopPlayback`. Ported (`fullscreenmoviebink.inl`).

## 3. The overlay manager (16 of 16 functions; 2013 is the target, 2012 the readable decompile)

`DishonoredGame/Src/disbinkoverlaymanager.cpp` (was a comment-only unit on the exclude list):

| function | 2013 rva | 2012 rva | notes |
|---|---|---|---|
| Initialize | 0x79e440 | 0x8028e0 | upper-cases the three texts, registers with `GFullScreenMovie` (2013 vtable +36), new critical section |
| CleanUp | 0x787530 | 0x7f40c0 | ported; **not called yet** (retail calls it from `UDishonoredEngine::PreExit` 0x5e4520, unported) |
| LoadContentPackage | 0x7934f0 | 0x7f9240 | `UI_Loading_SF_LOC_INT` + `UI_Loading_SF`, fonts `Emerge_BF` / `ChaletComprime-CologneEighty`, `X360_A`, `ChapterTitleBg`; 2013 hands the normal font to the movie player as its subtitle font |
| OnLoadingMovieStopped | 0x787570 | 0x7f4100 | 2013 clears the movie's subtitle font, no longer clears bit 0 |
| OnPlayLoadingMovie | **0x7b60e0** (verified: 3 args in 2013) | 0x81a1e0 | localized `[MapNames]`, hint set by `m_MapHintBindings`, `Hint_Gamepad`/`Hint_Keyboard` filter by `m_bUsingGamepad` (bit 10 @1812) |
| OnPlaySaveNotificationMovie | 0x7b69c0 | 0x81a9d0 | `[DisBinkOverlayManager_SaveNotificationLoadingScreen] Text_PC` |
| Tick | 0x79e570 | 0x802a10 | chapter title/description from `ADishonoredGameInfo::GetCurrentChapter` |
| OnBinkTick / OnBinkRenderFrame | 0x7a9d30 / 0x7b9880 | 0x80bb20 / 0x802b30 | |
| OnRenderLoadingMovie | **0x7b8b20** (verified, ratio .59 because 2013 sets RT + depth state first; not reproduced, the canvas flush sets its own) | 0x7f9570 | 1280x720 layout fitted to the viewport; map name drawn 3x (shadow passes (-3,3) -3.95° and (-5,-3) -3.42° in `m_fMapNameShadowColor`) |
| DrawTextOverlay | **0x7b7130** (verified, same code) | 0x7f5710 | |
| GetRandomHintIndex / ChangeHint | 0x793800 / 0x79e6a0 | 0x7f9ea0 / 0x802b60 | |

What had to come with it (all unported before):

* **Three 2013 virtuals of `FFullScreenMovieSupport`** (`FullScreenMovie.h`): 2013 inserts `IsLoadingMovie`,
  `IsMovieSkippable`, `SetOverlayManager` at slots 7-9 (base vtable `0xfcde60`; `FDisFullScreenMovieBink`'s
  `0x10a4610`: `0x932cc0 mov eax,[ecx+130h]`, `0xc64da0 mov eax,[ecx+78h]` = `bIsMovieSkippable`, the member
  `OnRequestLoadingMovieExit` sets at `0x933ece`, `0x932d00` store). The overlay manager reaches the player only
  through them, so DishonoredGame needs no Bink header. Plus `SubtitleFont` (2013 `GFullScreenMovie+0x18`, read by
  `RenderFrame` at `0x4fa522`).
* `DrawStringWrapped` / `DrawStringWrappedZ` (Arkane additions to `uncanvas.cpp`, 2013 `0x40d0f0` / `0x409120`) - the
  paragraph's word wrap.
* `FindLocalizedConfigSection` (2013 `0x7c86c0`, `dishonoredutilities.cpp`).
* `ADishonoredGameInfo::GetCurrentChapter` (`0x5fd320`) and **`SetCurrentChapter` (`0x5fd0e0`) +
  `UDisSeqAct_SetCurrentChapter::Activated` (`0x7861e0`)**: the chapter tag had no writer at all, so the title could
  never have appeared. `HG2.log`: `chapter Chapter1Info_ReturningHome: title 'RETURNING HOME'` at the key wait.
* `UDishonoredEngine::Init` calls `Initialize` (2013 `0x614e20` at `0xa15194`) and `PlayLoadMapMovie` the two hooks
  (`0x601b80` at `0xa01be7` / `0xa01c34`, the no-config branch `OnPlayLoadingMovie(MapName, FALSE,
  m_bShowLoadingMapNameAndHints)`).

## 4. The key wait, unattended

* The wait is retail's own: `CommitMapChange` → `StopMovie(TRUE)` → `FDisFullScreenMovieBink::
  GameThreadRequestDelayedStopMovie` → `OnRequestLoadingMovieExit` (`l_tower_p` is not in `MapsToAutoStart`) →
  `bIsMovieSkippable`, the UI paused, `GameThreadWaitForMovie` blocks the game thread.
* `-inputcmd`'s script is ticked from `UGameViewportClient::Tick`, which does not run while the game thread blocks.
  `FFullScreenMovieBink::GameThreadWaitForMovie` now also ticks the script (`DishonoredTickInputScriptOutsideViewportTick`,
  `UnPlayer.cpp`), so a queued `key` reaches `UGameViewportClient::InputKey` → `GFullScreenMovie->InputKey` exactly as a
  real key does. Nothing reads real input.
* **Measured, and the plan did not foresee it**: retail's `FDisFullScreenMovieBink::InputKey` (`0x933bb0`) takes keys
  only from `GamePlayers(0)->ControllerId`, and our first player is still **-1** at New Game: run 1 logged `movie key
  Escape controller 0 (first player -1) ... consumed 0` and the page never left. Pressing Space on the start screen
  (`--start-key`, run 2) does **not** assign it either (`-finewgame: first local player controller id -1`). In
  retail the start screen's script assigns it; that path is not ported. *build 2*: `-finewgame` (which exists to skip
  that screen) gives an unassigned first player controller 0 (`dishonoredplayercontroller.cpp`, `DISHONORED(bringup)`).
  **A user who plays through the menu by hand, with movies on, cannot leave the briefing page with the keyboard until
  the start screen's controller assignment is ported** - that is why `-nomovie` must stay in the play defaults.
* The intro is a play-once movie: Space does not skip it (`movie key SpaceBar ... loading 0 ... consumed 0`), Escape
  does (the reference skip keys). Unskipped it plays its 36 s and ends on its own (`HG2.log` 85.6 → 124.8).

## 5. Measurements (build 2)

`python build/agentHG/hg_run.py --exe .../DishonoredGame_HG.exe --log HG2 --tag hg2 --start-key -- -ResX=1920 -ResY=1080`
(`-windowed -finewgame=50 -fosoiree -pbinkdiag`, ini isolation under `build/agentHG/config/<log>`; the window came up
1008x567 - the desktop flaps, see STATUS pitfalls - the BMPs are scaled to 960x540 for scoring).

`-pbinkdiag` (HG2.log): `movie open for map l_tower_p (config 1, ...): map name 'DUNWALL TOWER', hint set 3 of 9 hints,
hint index 6 '...'` → overlay state lines (map name at (105,109), LOADING at (1180,625), hint at (106,550)) → `key wait
start on map l_tower_p` 0.64 s after the movie opened → `overlay press to continue ... chapter title 'RETURNING HOME' at
(110,490) bg 1 ... chapter description ... at (106,550)` → `movie key Escape controller 0 (first player 0) ... consumed 1`
→ `key wait end, intro 'INTRO_LOC' starts` → `intro 'INTRO_LOC' ended`. `-pbinkdiag` also serves a pending `shot` from
the movie frame (`apshottime_<tag>_bink*.bmp`), since `-apshottime` cannot see a movie.

`python build/agentHG/measure.py build/agentHG/shots/hg2_page.png build/agentHG/shots/hg2_clouds.png`
(pairs `build/agentHG/shots/pair_page.png`, `pair_clouds.png`):

| area (960x540 fractions) | SSIM vs retail t 4.0-5.5 min / median / max | page2 (2 s later) |
|---|---|---|
| map name (0-.62, .08-.26) | .878 / .909 / .946 | .923 / .950 / .988 |
| chapter title + paragraph (.04-.96, .66-.84) | .923 / .939 / .962 | .945 / .965 / .978 |
| PRESS ANY KEY (.62-.96, .86-.93) | .884 / .915 / .949 | .916 / .954 / .975 |
| full frame | .756 / .802 / .886 | .822 / .894 / .982 |
| control: retail 4.0 vs retail 5.5 | .904 / .952 / .921 / .789 | (the movie background moves) |

Clouds: best retail match **SSIM .980 at t 8.1**, .952 at t 8.0 (MAD 5.6) - our shot is 2.4 s into INTRO_LOC.

**Five clean runs with movies** (build 2, `hg_run.py`, HG2..HG6; HG4's first attempt starved - startup 140 s while
other agents built - and quit before New Game, so it was re-run; startups 29-79 s on the others): every run shows the
page, waits, takes the inputcmd key (`consumed 1`), plays INTRO_LOC, reaches `EnterSoireeMode` on the boat, 0 Criticals.

| run | key wait start | key → intro | intro ended | boat (EnterSoiree) | map name / chapter+paragraph / PRESS ANY KEY SSIM (median, page2) | clouds shot = INTRO_LOC frame |
|---|---|---|---|---|---|---|
| HG2 | 81.6 | 85.6 | 124.8 (unskipped, Space) | 130.4 | .950 / .965 / .954 | 2.5 s, SSIM .974; vs retail t 8.1 .980 |
| HG3 | 122.3 | 144.5 | 159.6 (Escape) | 165.3 | .854 / .941 / .946 | 5.9+ s (shot late) |
| HG4 | 110.3 | 119.5 | 124.9 (Escape) | 130.8 | .846 / .902 / .895 | 4.5 s, SSIM .975 |
| HG5 | 151.5 | 176.9 | 185.2 (Escape) | 191.1 | .864 / .913 / .904 | 5.9+ s (shot late) |
| HG6 | 108.5 | 112.5 | 115.7 (Escape) | 121.3 | .950 / .964 / .953 | 2.5 s, SSIM .974; vs retail t 8.1 .980 |

The clouds capture is timed by `wait 2.4` queued when the log shows the intro start; on the three starved runs the
command file was read seconds late, so the frame is later in the same movie - the movie itself is INTRO_LOC every
time. Every overlay area is above the .7 bound on every run (lowest .833, HG4 map name).

Gate, `python resources/tools/run_regression.py --build-dir build/agentHG --no-build` (`-nomovie` via `-playdefaults`
as before): **32 ok, 2 failed**; 0 Criticals in nullrhi/d3d9/inputtest/newgame/options, `unported_natives` 0,
`newgame_on_mission` 107, `newgame_pawns` 29, `newgame_menu_open_seconds` 50.3. The two failures are
`coresmoke_passed` / `layout_types`: `CoreSmoke.exe` / `LayoutProbe.exe` were never built in `build/agentHG` (the
build cmd builds the DishonoredGame target only; a third build was not spent on them). A `-nomovie` hg_run on build 1
(HG0N) also reached the boat with 0 Criticals.

## 6. Where the plan was wrong (with the measurement)

1. **"The clouds may be a level Kismet sky shot"** - no: INTRO_LOC.bik frame-for-frame (table in 1).
2. **"Declare and compile the Bink global shaders the cache skips"** - nothing to declare: they were skipped only
   because `USE_BINK_CODEC=0`; with it on 261 of 263 load (2 MeshPaint remain).
3. **The overlay manager alone could not have produced the page**: the reference `RenderFrame` gate on
   `MM_LoopFromSuppliedMemory` hid every overlay, and the chapter title had no writer (`SetCurrentChapter` /
   `UDisSeqAct_SetCurrentChapter` unported, the unit on the exclude list). Both fixed.
4. **"m_pBinkOverlayManager is never constructed natively"** (my own reading of the xrefs) - true, but it is not NULL:
   it is the engine CDO's default subobject `Default__DishonoredEngine:pBinkOverlayManager` (`HG2.log` 4.30). The NULL
   fallback in `Init` never fires.
5. **"PRESS ANY KEY waits for an inputcmd key"** needed two things the plan did not name: the script must be ticked
   inside `GameThreadWaitForMovie`, and the first player's controller id must be assigned (section 4).
6. **"The newgame stage passes without -nomovie"** cannot be run as written: `-playdefaults` appends `-nomovie`
   whenever the command line lacks the substring `nomovie`, and `appInitFullScreenMoviePlayer` tests the same
   substring, so the stage can only be run with movies by changing the play-defaults table - and with movies the
   stage's `-fmdrive` script never presses a key, so it would block on the page by design. The with-movies evidence
   is `hg_run.py` (same `-finewgame=50`, reaches `EnterSoireeMode` on the boat).
7. **"Our run is ~4 s ahead of retail"**: with movies, ours shows the page 0.6 s after New Game (load 0.57 s) and the
   key wait starts at once; retail showed LOADING for 1.6 s first (its load was slower). The remaining offset is the
   user's key timing, not code.

## 7. Open items

* The start screen's controller assignment (real-input path): unported; `-finewgame` emulates it.
* `UDishonoredEngine::PreExit` (`0x5e4520`, calls `CleanUp`) and `UDishonoredEngine::StopMovie` (`0x5e4590`, sleeps
  until `m_dSaveNotificationMovieEndTime`) are unported.
* 2013 `GameThreadPlayLoadingMovieAndIntro` / `GameThreadStopMovie` call the online subsystem (+320/+324) and the audio
  system's resume (`GWorld+688` vtable +316) - existing TODOs in `disfullscreenmoviebink.cpp`.
* `DISHONORED_WITH_BINK` stays OFF by default in `cmake/Bink.cmake`; flip it (and the build line) when the coordinator
  takes this package. Bink audio plays through DirectSound (audio is polish).
* Subtitles over INTRO_LOC (`INTRO_LOC.txt`) are drawn only when `GEngine->bSubtitlesEnabled`; retail's frames show none.

## Files

Engine: `Inc/FullScreenMovie.h`, `Inc/UnCanvas.h`, `Src/UnCanvas.cpp`, `Src/UnPlayer.cpp`, `Src/disfullscreenmoviebink.cpp`,
`bink/src/DisFullScreenMovieBink.h`, `bink/src/fullscreenmoviebink.inl` (all hand-edited reference/Arkane units).
DishonoredGame: `Src/disbinkoverlaymanager.cpp`, `Src/disseqact_setcurrentchapter.cpp` (both taken off the exclude
list by the generator), `Src/dishonoredengine.cpp`, `Src/dishonoredgameinfo.cpp`, `Src/dishonoredutilities.cpp`,
`Src/dishonoredplayercontroller.cpp`, `Inc/dishonoredutilities.h`, `Inc/CppText/UDisBinkOverlayManager.h` (new),
`Inc/CppText/UDisSeqAct_SetCurrentChapter.h` (new), `Inc/CppText/ADishonoredGameInfo.h`; regenerated:
`Inc/dishonoredgameclasses.h`, `Inc/DishonoredGameChapterClasses.h`, `Src/DishonoredGameRegistrants.cpp`, `Sources.cmake`.
Regeneration into `build/agentHG/regen` is byte-identical (79 files, 0 differ); `bitfield_audit.py` mismatches 0.
Tools (not synced): `build/agentHG/hg_run.py`, `measure.py`, `match_movies.py`; `build/agentHG_sync.py`.
