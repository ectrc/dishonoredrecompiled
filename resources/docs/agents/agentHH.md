# Agent HH — `resources/tools/cutscene_compare.py` (wave 25, PHASE22 package HH)

Opus 5.5, worktree `build/agentHH_wt` (base `9faefc0`). No game code was touched and nothing was built. Every number
below comes from the staged control exe `Dishonored_Latest2026/Binaries/Win32/DishonoredGame-Win64-Shipping.exe`
(= `build/head_rel`, HEAD 9faefc0 RelWithDebInfo), or from `resources/reference/mission0/{retail,ours}.mp4`.

## What the tool does

| subcommand | what it does |
|---|---|
| `run --tag T [--exe E] [--times 50:175:1] [--quit-at 185] [--baseline csv]` | launches through `game_input.py launch` with `-windowed -ResX=1920 -ResY=1080 -finewgame=50 -fosoiree -fzdiag -apshottime=<times>` and ini isolation (`-ENGINEINI/-GAMEINI/-INPUTINI/-UIINI` under `build/cmp/T/config/`), queues `wait 185` / `quit`, waits for the pid to exit, claims only `apshottime_cc_T_NNNNN.bmp` (the tag ends in `_` so `hr` cannot claim `hr2`'s files), pairs them in order with the log's `-apshottime: screenshot i of n ... world time w` lines, rejects non-16:9 captures, writes `build/cmp/T/ours/w<world>.png` at native size, deletes the BMPs, then scores |
| `run --ours-video X.mp4 --tag T [--step 1]` | decodes the mp4 once (10 fps, 960x540, `build/cmp/video_cache/<stem>/`) and scores it; "world" = video time |
| `score --tag T [--baseline csv]` | re-scores stored frames without a launch |
| `compare A B [--times ...]` | A/B = `retail`, a dir of `w<t>.png`, or an mp4 |
| `selftest` | retail vs retail (SSIM 1.0000, zoom 1.00), synthetic wider field 1.5 / 1.78 recovered to ±0.01, retail detector maxima and on-spans, retail events |

Retail: decoded once to `build/cmp/retail_cache/f_NNNNN.png` (frame k = t k/10, 1121 frames, 462 MB) with per-frame
stats cached in `stats.json`. ffmpeg path is a constant, `CC_FFMPEG` overrides it (`CC_RETAIL_MP4` likewise). Outputs
always go to the MAIN checkout's `build/cmp/` (resolved through `git rev-parse --git-common-dir`), so a worktree copy
of the tool writes to the same place.

Output `build/cmp/T/`: `pairs/pair_w<world>.png` (retail top, ours bottom, 960x540 each), `diff/` (|blurred grey|x3),
`scores.csv` (world, retail_t, mad, ssim, zoom, zoom_ncc, ncc_at_1, centre brightness, 7 detectors for ours and for the
retail frame), `report.md`, `index.html`, `run.json` (shots with log ts and world, anchors, d3d9 start, rejects).

Metrics (numpy, 960x540 grey): MAD after two 5x5 box blurs; SSIM-lite with 8x8 sliding windows by integral images;
zoom = FE's fit: ours' centre crop (0.4W x 0.36H, divided by s) resized onto retail's centre crop, NCC on band-passed
grey (box3 - box21), s = 0.50..2.20 step .01; s > 1 = ours sees a wider field.

Detectors (percent of area unless noted; "on" thresholds were set from the retail cache so that retail is off where it
truly shows nothing):

| detector | area (x, y) | test | on > | retail max / where |
|---|---|---|---|---|
| vials | 0–.16, .05–.30 | red R>150,G<90 or blue B>150,B>R+60 | 3 | 1.85 (captain's red cuffs, t 43); ours 5.5 |
| stance | template at (32,446) ±4 px | NCC of the crouch icon (`cutscene_compare_stance.png`, cut from ours.mp4 @110 s) | 0.5 NCC | 0.18; ours 0.92–1.0 |
| debugtext | 0–1, .08–.14 | cyan (G,B>100, abs(G-B)<20, R<G-60) and white top-hat > 25 | 0.2 | 0.09 (t 18–21); ours 0.63 |
| banner | .36–.64, .24–.34 | grey>150, only while the rest (x>.2) has mean < 20 | 2 | 12.4, on 8.8–13.9 |
| objective | .6–1, 0–.2 | top-hat(7) > 60 and grey > 150 (thin bright strokes) | 3 | 8.7, on 108.1–112 |
| hint | .3–.7, .62–.72 | same | 3 | 6.7, on 22.9–28.7 ("Use your mouse...") and 108.5–112 |
| prompt | .5–.75, .33–.45 | same | 3 | 7.8, on 107.9–108.5 (Geoff Curnow), 1.4–2.3 (menu) |

The plan's brightness/colour-only tests for `stance`, `objective`, `hint`, `prompt` fired on the white hull and the sky
for most of retail (stance "on" 17–65 s); the top-hat and the template fixed that.

Alignment: E1 = first banner-on sample; E2 = first sample after the longest black run (centre crop x .3–.7, y .4–.8
mean < 6, run ≥ 1.5 s) whose centre brightness reaches 50 % of the median after the run, linearly interpolated between
samples; F = first non-black sample after the run. retail_t = a + b·world fitted over the events found on BOTH sides
(b free with 2 points, clamped back to 1 outside .9–1.1; b = 1 with one); fallback = `soiree ENTER` world vs retail F.
Every offset and the fallback residual are printed.

## Commands

```
python resources/tools/cutscene_compare.py selftest
python resources/tools/cutscene_compare.py run --tag head_rel
python resources/tools/cutscene_compare.py run --tag head_rel2 --baseline build/cmp/head_rel/scores.csv
python resources/tools/cutscene_compare.py run --tag ours_mp4 --ours-video resources/reference/mission0/ours.mp4
python resources/tools/cutscene_compare.py run --tag dbgprobe --times 52,53,54,55,56 --quit-at 64
python resources/tools/cutscene_compare.py compare build/cmp/head_rel/ours build/cmp/head_rel2/ours --times 52:174:8
```

Packages: `run --exe <your staged exe> --tag <X> --baseline build/cmp/head_rel/scores.csv`. A run is 195 s wall here
(d3d9 first scene at log 6.0–6.5 s, i.e. the loads were not slowed by the other agents' builds), scoring ~2 min.

## head_rel baseline (`build/cmp/head_rel/`), ride = world 56.3–150.6 from the log's soiree ENTER/EXIT

| metric | median over ride | median all frames |
|---|---|---|
| MAD (0–255) | 29.43 | 29.41 |
| SSIM-lite | 0.523 | 0.524 |
| zoom | **1.76** (p10 1.74, p90 1.78, 86 % within ±.03) | 1.76 |
| zoom NCC | 0.171 | 0.166 |

| detector | ours on at world | retail on at (mapped to ours world) |
|---|---|---|
| vials | **51.1–175.0** (125/126 frames; over the black 51–57 too) | – |
| stance | 151.0–175.0 (from soiree EXIT 150.6 on) | – |
| debugtext | – (instrument artefact, see below) | – |
| banner | **– (0/126)** | 52–56 |
| objective | – | 151–154 |
| hint | – | 66–71, 152–154 |
| prompt | 50.0 (the menu frame's difficulty rows) | 151 |

Accept check: median zoom 1.76 (in 1.7–1.85) yes; vials > 0 during the black yes; banner = 0 yes; **debugtext > 0
near +8 s: NO on a `run`, for a reason in the engine, not in the detector** — see findings 1. The detector is proven on
`ours.mp4` (`--ours-video`: debugtext on at video 6–9 s, the same moment the user saw it) and on the probe run (on at
world 52.0, 0.63 %).

Determinism (two runs, same exe, `head_rel2` vs `head_rel`): MAD 29.255 vs 29.425 (**-0.6 %**), SSIM +0.3 %, zoom
1.760 = 1.760, every detector count identical (vials 125, stance 25, prompt 1, others 0); alignment a = -42.825 vs
-42.772 (0.05 s). Frame-to-frame between the two runs: median SSIM .944, MAD 1.5; the worst pairs (world 108, 140,
MAD 6–9) are NPC animation phase, the static post-ride frames are SSIM .99.

`ours.mp4` (the user's OBS session) through `--ours-video`: zoom 1.75, MAD 29.1, SSIM .516 — matching the game-run
baseline, which is the cross-check that the capture path (BMP vs OBS H.264) does not bias the metrics.

Self-test: retail vs retail SSIM 1.0000, MAD 0, zoom 1.00 at 7 times; a synthetic wider field 1.5 → 1.50 and
1.78 → 1.77–1.78.

## What the alignment found

- Retail: E1 (banner on) 8.80 s, black 8.2–14.5, F 14.60, E2 15.75. The banner is on 8.8–13.9, so the plan's
  "≈10–14 s" is the plateau; the fade starts at 8.8.
- head_rel: no E1 (no banner), black 51.1–57.0, F 58.01, E2 58.53 → retail_t = world − 42.77 (E2 only, b = 1).
  The fallback (soiree ENTER 56.33 vs retail F 14.6) agrees to +1.04 s (+1.20 on the rerun); F-to-F offset −43.41,
  i.e. our boat goes from black to half brightness 0.5 s faster than retail's (E2−F 0.52 s vs 1.15 s).
- Our black lasts 5.9 s (51.1–57.0) vs retail's 6.3 s, and retail's banner sits in it from 0.6 s in. Until HF lands
  the banner, b cannot be fitted (one event); once E1 exists the fit gets two points and reports b and a residual.
- ours.mp4: E2 at video 13.22 vs retail 15.75: the user's two recordings started 2.5 s apart relative to the fade.

## Findings / where the plan is wrong

1. **`-apshottime` turns on-screen messages off for the rest of the run.** `DishonoredTimedScreenShot`
   (`Engine/Src/SceneRendering.cpp:5136-5139`) sets `GScreenShotRequest` without saving
   `GScreenMessagesRestoreState`; the readback (`UnPlayer.cpp:2208`) then does `GAreScreenMessagesEnabled =
   GScreenMessagesRestoreState`, which is still its default FALSE (`Core.cpp:469`). After the first capture (world 50,
   the menu) `UMaterialInstance::CheckSafeToModifyInGame` neither draws nor logs: `CC_head_rel.log` has **0**
   `Modifying '...'` lines, the probe run whose first capture is at world 52 has 3 (log 58.39) and the frame at 52.02
   shows the cyan line. So **HE's "debugtext = 0 in all frames" is vacuously true on any `run` today** (the pitfall
   "a check can be green because its pattern never matches"). The fix is one line in that function —
   `GScreenMessagesRestoreState = GAreScreenMessagesEnabled;` before `GScreenShotRequest = TRUE;` — which is engine
   code outside HH; it belongs to HE (who owns the message) or the coordinator. Until then, judge debugtext from a
   probe run whose first mark is after the message (`--times 52,...`) or from an OBS video.
2. **The ride is world 56.3–150.6, not 64–158.** The plan's numbers are log timestamps (`[0068.39] soiree ENTER`,
   `[0162.64] EXIT`); world = log − 12.1 on this run (−10.4 on the rerun). Use world times; the tool reads ENTER/EXIT
   from the log each run.
3. **World time does not restart at the New Game** (the persistent world stays, menu and mission are streamed), so
   one `-apshottime` list covers the menu frame (world 50) and the whole ride.
4. The plan's colour/brightness detectors for stance/objective/hint/prompt cannot work on daylight frames (retail
   fired for most of the ride); see the table above for what replaced them.
5. Zoom 1.76 = 16/9 within the fit's step: consistent with root cause 1 (75° applied vertically). The NCC at the best
   scale is only ~0.17 because the camera position and the captain differ as well — the scale is a robust argmax (86 %
   of ride frames within ±.03) but the NCC value itself is not a quality score.

## Caveats

- The capture size was 1920x1080 on all three launches; a non-16:9 one is rejected and listed in `report.md`.
- Pairs are matched to the nearest 10 fps retail frame (±0.05 s); our marks are 1 s apart, so E2 on our side is
  interpolated between samples (±~0.3 s).
- `retail_*` detector columns describe the retail frame paired to each world; retail.mp4 ends at 112.1 s = world ~154.9,
  later frames have no retail pair and are scored on detectors only.
- The stance template is cut from an OBS frame on a floor background; it matched our own BMP captures at 0.92–1.0
  after the ride. If the stance icon's art changes, re-cut it.
- Generated data (`build/cmp/`, ~1.8 GB with both caches) is not in the sync list.
