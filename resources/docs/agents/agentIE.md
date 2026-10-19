# Agent IE — the boat's matinee lines: `UInterpTrackDialog`, the Wwise listener and the propagation cache (wave 26, PHASE23 package IE)

Worktree `build/agentIE_wt` (made at `fec772f` = `ea91599` + the PHASE23 docs commit), build dir `build/agentIE` (RelWithDebInfo,
`build/agentIE/ie_build.cmd`), staged as `DishonoredGame_IE.exe`. Two builds: build 1 failed to compile (a `?:` between `FString` and
`FStringNoInit`, a missing `dishonoredutilities.h` include), build 2 = 0 errors and is the measured one. IDA on private copies
`build/agentIE/idb/IE_{retail2013,shipping2012}.i64` (copied from ID's, which carry its names) and headless 2012 decompiles in
`build/agentIE/decomp2012/`. Runs through `build/agentIE/ie_run.py` (cutscene_compare's own `run_game`, with the input script
queuing `exec set Engine bSubtitlesEnabled True` before the ride when asked). No retail exe, no real input, no FModel.

`Engine/Src/UnPlayer.cpp` is delivered as `build/agentIE/unplayer.patch` (checked: `git apply --check` passes on the dirty main tree).
**The measured build had that patch applied in the worktree** - the listener cannot be fed without it; the sync script does not copy
`UnPlayer.cpp`.

## 1. Headline

**The boat's lines play at their key times, each line's subtitle is drawn and cleared, and the ride still ends on time.**
IE2 (`-finewgame=50 -fosoiree -nomovie -iddiag -iediag`, subtitles off), the three tracks of `Tower_IntroOnBoat` and the one of
`Tower_BoatLifted`:

```
world   matinee  key start  group                 key  blurb             text
 56.70    0.038     0.000   Guard Captain A        0   DisConv_Blurb_9   'Steady hand. That's it, watch it!'
 60.64    3.982     3.839   GuardBigBoard C        0   DisConv_Blurb_8   'Cast off line!'
 62.01    5.352     5.348   Guard Captain A        1   DisConv_Blurb_7   'Casting off. We're away.'
 68.67   12.014    12.006   Guard Captain A        2   DisConv_Blurb_10  'Take us straight to Dunwall Tower. ...'
 75.33   18.669    18.555   EliteGuard on Boat     0   DisConv_Blurb_2   'A long way to bring bad news. ...'
 81.32   24.661    24.580   Guard Captain A        3   DisConv_Blurb_6   'Superstition. ...'
 86.57   29.916    29.832   EliteGuard on Boat     1   DisConv_Blurb_39  'Maybe. We live in strange times.'
 90.25   33.591    33.566   EliteGuard on Boat     2   DisConv_Blurb_3   'Sending the Empress' body guard away ...'
 95.37   38.713    38.451   Guard Captain A        4   DisConv_Blurb_1   'Well, this was important. ...'
113.84   57.180    57.177   Guard Captain A        5   DisConv_Blurb_40
117.54    0.108     0.075   (BoatLifted) AI_0      0   DisConv_Blurb_2
129.77   73.113    73.112   Guard Captain A        6   DisConv_Blurb_12
146.12   89.460    89.231   Guard Captain A        7   DisConv_Blurb_11
```

13 keys, 13 fires, 13 dones, every one in the first matinee update at or after its start (lag 0.000-0.26 s of matinee time = one
frame under load; the first `BoatLifted` update is at 0.108). No key on these tracks has `WaitForEndOfBlurb`, so there is no sync wait
and the duration hint cannot delay the ride: **soirée ENTER 56.68 / EXIT 150.91 (ride 94.23 s; head_rel 56.3-150.6, the control IEc
56.21-150.52)**.

IE1 (the same, subtitles on from world 45): 12 lines added to the main list, **12 `OnMainSubtitleChanged(text)` with a
`subtitles_mc.SetSubtitles(null, text) -> ok` each and 17 clears** (each line's own kill plus the owner change to `<none>`), owner
chosen in the frame after each add. The `subtitle` detector (re-measured, §4) is on in **34 of 92 ride frames of IE1 and 0 of 94 in
IE2 and in the control** (world 59-60, 64, 70-89, 92-100, 115-116, 148-149: the spans of the lines). IE1 quit at world 150.0 (the
script's waits count wall-clock-ish frame time; later runs waited longer), so its EXIT is not in the log; IE2 is the ride-end measurement.

Ride metrics vs retail with one alignment for all three runs (fixed b = 1 at each run's E2 offset, `build/agentIE/fixed_align.py`;
cutscene_compare's own E1+E2 fit put IE2 on b = 1.075 and the control on b = 1, which alone moved IE2's median SSIM to .630):

| run | exe | subtitles | ride | median SSIM | median MAD |
|---|---|---|---|---|---|
| IEc | `DishonoredGame_ID.exe` (control: ID's build 2, no IE code) | off | 56.21-150.52 | 0.692 | 11.52 |
| IE2 | `DishonoredGame_IE.exe` | off | 56.68-150.91 | **0.691** | 11.71 |
| IE1 | `DishonoredGame_IE.exe` | on | 57.89-(quit 150.0) | 0.744 | 10.23 |

IE2 vs the control: SSIM -0.001, MAD +0.19 - unchanged. (IE1's higher number is that run's offset/phase, not the subtitles.)

## 2. What was ported (2013 rva, 2012 in brackets; every 2013 match by bytes or vtable slot)

| unit | functions |
|---|---|
| `interptrackinstdialog.cpp` (off the exclude list) | `UInterpTrackInstDialog::InitTrackInst` 0x882330 [0x8e9a90], `TermTrackInst` 0x873ab0 [0x8e0290], `GetBlurb` **0x86ee10** [0x8dc6e0], `GetAltBlurb` 0x86ee60 [0x8dc730] |
| `interptrackdialog.cpp` (off the exclude list) | `UInterpTrackInstDialog::UpdateTrackInst` 0x873750 [0x8ec050], `NeedsSynchronizing` 0x8735c0 [0x8e0100]; `UInterpTrackDialog::GetKeyProperties` **0x86ebd0** [0x8e9960], `IsPlayingASkippableItem` **0x8699a0** [0x8da7a0], `FindNextSkipAnchor` **0x86ed00** [0x8dc5d0], `FindLastSkipAnchor` **0x86eda0** [0x8dc670] (vtable `??_7UInterpTrackDialog` 0x118bf70); `-iediag` |
| `disconv_soiree.cpp` | `UDisConv_Soiree::FindBlurbs` 0x8b1800 [0x8fbdc0] (FR had it as editor-only) |
| `disconversation.cpp` | `UDisConversation::GetSpeakerName` 0x895dc0 [0x8e1390] |
| `disconv_speakers.cpp` | `GetSpeakerName` (slot 86, +344) of `UDisConv_Speaker` 0x8ad320 [0x8fc460], `UDisConv_MeSpeaker` 0x8b1cc0 [0x8ff210], `UDisConv_InitiatorSpeaker` 0x8b1e80 [0x8ff3d0], `UDisConv_OneShotSpeaker` 0x8b6380 [0x8fc0e0] |
| `disdialogoneshot.cpp` | `FDisOneOffSpeaker::GetOneShotName` 0x6511f0 [0x69d2e0] |
| `disaudiopropagationinfo.cpp` (off the exclude list) | `DisGetAudioPropagationInfo` 0x792290 [0x7f7ea0], `UDisAudioPropagationInfo::PostLoad` 0x7a3070 (vtable 0x115c380 slot 6) [0x807670] |
| `dishonoredaudiosystem.cpp` | `UDishonoredAudioSystem::GetCellAtPoint` 0x792d10 (slot 75) [0x7f8a50], `DisSetAkSoundPositions` 0x792fe0 [0x7f8d20], `Update` 0x7b3520 [0x818ff0]: the listener cell and the per-component propagation cache (the component loop) |
| `dishonoredviewportclient.cpp` + `Engine` seam | the listener half of `UGameViewportClient::Draw` 0x2c59a0 (0x2c6010..0x2c604d): `ApplyListenerLocationModifier` (+344) then `UAkAudioDevice::SetListener` 0x5aecb0, through `GDisSetListenerHook` (`UnWorld.h`, defined in `UnGame.cpp`) called from `Draw` (`unplayer.patch`) |
| `cutscene_compare.py` | the `subtitle` detector re-measured (§4) |

The track: InitTrackInst asks the soirée node that started the matinee (`USeqAct_Interp::GetConversationNodePointer`) for the blurbs
the group speaks (`FindBlurbs`: from the soirée's second output link, depth first, a blurb is taken when its speaker's undecorated
name equals the group name, the group is unnamed, or the speaker has no name) and maps each key's `m_BlurbGUID` / `m_AltBlurbsGUID`
onto them by `m_iBlurbGUID` (2013 +136; 2012 read +128). UpdateTrackInst plays the last key crossed since the previous update through
`PlayBlurb_Shared(…, bSkipLookTargets = TRUE, seek = position − key start)` (the 2013 thunk 0x89fb70) and ticks it with
`TickNode_Shared(Matinee->DeltaTime, …, TRUE)` (thunk 0x89fbd0); a jump interrupts with `DisConvoEndReason_MatineeSkipBlurb` (13),
TermTrackInst with `BehaviorChange` (2). Alternatives are drawn with `rand()` among the main and alt blurbs, the previous one excluded.

The listener: retail's Draw gets the Wwise device from `GEngine->Client` (vtable +0x140), applies the viewport client's listener
modifier to the view location, and calls `UAkAudioDevice::SetListener(PlayerIndex, Location, Up, Right, Front)`; it never calls
`UAudioDevice::SetListener` or `SetAudioSettings`. Our `Client->GetAudioDevice()` is NULL and Engine cannot name AkAudio, so
DishonoredGame installs both calls into the seam. Measured: `m_ListenerPosition` follows the camera (IE1: (8 500 −129) in the menu,
(−3897 36632 −145) at the ride start).

The cache: `l_tower_p` has a propagation info (`DisAudioPropagationInfo_4`, 6 cells); the listener sits in `DishonoredAudioVolume_2`
from world 55.7 and so does every speaking guard, so `GetShortestAudiblePathLength` is the straight distance: 188-413 for the captain
(`DishonoredNPCPawn_26`), 207-211 for the elite guard (`_27`), 1141 for `_16` ('Cast off line!'), all live. With radius 3000 the
ranking `1 − d/r` is 0.62-0.94, so each line owns the subtitle.

## 3. Deviations (named in the file banners)

- `UpdateTrackInst` / `NeedsSynchronizing` do nothing when `m_DialogKeysBlurbs` and `DialogKeys` differ in length (InitTrackInst found
  no soirée node); retail indexes them in step and would assert.
- `UDisConv_OneShotSpeaker::GetSpeakerName`: retail dereferences the tree's one-shot unchecked; a tree bound to none answers like an
  out-of-range index. 2013 lists the one-shot's speakers through `ADisDialogOneShot`'s virtual +940 (0x655f00, which lists
  `m_OtherSpeakers` in order); the index reads `m_OtherSpeakers` directly.
- `UDisConversation::GetSpeakerName` reads the root tree through `GetDialogTree()` (retail: the root of the `m_pOwner` chain, no cast).
- `DisGetAudioPropagationInfo` uses `GWorld->GetWorldInfo(TRUE)` (retail reads the cached `m_pWorldInfoCheckStreamingPersistent`
  unchecked; `DishonoredGetMapInfo` notes it is not filled in this tree).
- `UDishonoredAudioSystem::Update` iterates the component set through `Cast<UDisAkComponent>` (retail casts nothing).
- `UInterpTrackDialog::GetKeyProperties` constructs with this tree's `ConstructObject` defaults.
- The skip functions have no caller: `UInterpTrack` declares no skip surface here (retail's matinee skip calls them by vtable).

Not ported, named: `UpdateAudioPropagationCache` [0x807d30] with `FindShortestPath` / `FinalizeAudioPath` / `DisAudioPropagationFindPathInfo`
(the portal path search - IH3): a sound in another cell than the listener keeps no path and reads FLT_MAX, so its subtitle can only
win as the dominant conversation (99). Also the rest of `Update`: ambient sounds, the listener cell's ambient event and RTPCs,
per-object environment (aux bus) values, the time-dilation RTPCs, the overseer effect.

## 4. The `subtitle` detector (cutscene_compare.py, owned by HH/ID - changed here because it could not see a real line)

ID's test (thin strokes > 170 whose pixel 2 px down-right is < 70, band y .80-.92) read 0.04-0.28 % on frames with and without a
line (on > 1.0): the game's line is grey ~180 on the ~95 hull, its shadow ~80, not dark; the captures are 1008x567 (the window, not
the `-ResX` 1920), and the first text row is at y .797, above the band. Measured on IE1 (33 frames inside a line's span) vs IE1
between lines + IE2 + IEc (238 frames), `build/agentIE/subtitle_probe.py`: the densest row of sharp horizontal steps
(|dx| > 40 next to a pixel > 140, 5-row mean) reads on: min 4.5, p10 9.5, median 28; off: p90 1.1, median 0.5 (the two "off"
frames at 19 were IE1 148-149, inside a line the log had not cleared before the quit). New test, band y .78-.92, on > 3.0;
`STATS_VERSION` 8 (the retail cache is recomputed). `selftest`: PASS - drawn line 27.7 % vs clean 0.3-1.0 %; retail on only at
1.4-2.3 and 4.0-5.5 (menu and briefing text, as before), never during the ride.

## 5. Where the brief was wrong (with the measurement)

1. **InitTrackInst does not resolve the keys "via ID's FindTextBlurb".** Both 2012 0x8e9a90 and 2013 0x882330 call
   `UDisConv_Soiree::FindBlurbs` (2013 0x8b1800) with the group's name, which needs `UDisConversation::GetSpeakerName` and the four
   speaker `GetSpeakerName` virtuals - FR had all of them as editor-only. FindTextBlurb is the voice-data lookup, used later by
   PlayDialogBlurb.
2. **GetBlurb is 2013 0x86ee10, not 0x86f600.** 0x86f600 is an array iterator's `operator*` (62 B; the 2012 "match" is by order
   only); TermTrackInst 0x873ab0 calls 0x86ee10, which is byte-for-byte the 2012 GetBlurb.
3. **The boat has no `WaitForEndOfBlurb` key** (all 13 keys read wait 0), so the "over-long duration hint delays the ride" risk does
   not exist on this route; 0 sync waits, ride 94.23 s.
4. **Subtitles get an owner on the boat through distance, not dominance**, and the brief's premise "every voice reads FLT_MAX because
   nothing feeds the cache" needed both halves: the listener feed alone would leave `m_fMaxAudibleDistance` 0 (cache never filled),
   the cache alone would measure from (0,0,0). Both are in; distances 188-1141 (§2).
5. **The listener feed is not in ULocalPlayer code** but in `UGameViewportClient::Draw` (the only caller of `SetListener`, 2013
   0x2c6048), and retail drops `UAudioDevice::SetListener` / `SetAudioSettings` there entirely.
6. **The `subtitle` detector could not see a real subtitle** (§4); ID's band and shadow test were set on a synthetic PIL line.
7. **"the ride still ends at world 150.6 ± 1.0"** holds (150.91), but **"ride SSIM unchanged" needs one alignment for both runs**:
   cutscene_compare's E1+E2 fit (E1 exists since HF) chose b = 1.075 for IE2 and clamped the control to 1, which by itself reads
   .630 vs .691 for identical-looking frames; at a fixed b the two are .691 / .692.
8. `UInterpTrack` here has no `IsPlayingASkippableItem` / skip-anchor virtuals; the ported ones are declared on the class and have no
   caller until the matinee skip surface exists.
9. The input script's `wait` is frame time, not world time: `wait 53` ran out at world 45.4 and a `wait 170` total quit at world 150.

## 6. Runs (`DishonoredGame/Logs/CC_IE*.log`, frames and scores in `build/cmp/IE*/`)

| run | exe | switches | what it measured |
|---|---|---|---|
| IE1 | IE | `-iddiag -iediag`, subtitles on at world 45.4 | 13 key fires / 12 lines with text, 12 SetSubtitles + 17 clears, owners, listener + distances; detector on 34/92 |
| IE2 | IE | `-iddiag -iediag`, subtitles off | key times (table §1), ride 56.68-150.91, SSIM .691 (fixed b), detector 0/94 |
| IEc | ID (control) | `-iddiag`, subtitles off | ride 56.21-150.52, SSIM .692 (fixed b), detector 0/94; 0 key fires (no dialog track before IE) |

Every run: 0 Critical; `native not ported` = `execOnNPCMarkForVanish` / `execOnFocusLost` / `UOnlineSubsystemSteamworks::execExit`,
the same on the control.

## 7. Gate

`run_regression.py --build-dir build/agentIE --no-build --exe-name DishonoredGame_IE.exe --log-prefix IEg_ --only d3d9,inputtest,newgame`
(`build/agentIE/gate.txt`, build 2 at fec772f): **22 ok, 1 failed**. `unported_natives` **0**, `probe_natives` **3**, `d3d9_criticals`,
`inputtest_criticals`, `newgame_criticals` **0**; `d3d9_frames` 1800, `newgame_on_mission` 105, `newgame_pawns` 29. The failure is the
load-sensitive `inputtest_moved`: **491.8, then 218.0 on the stage's retry**. Both runs reported: the control (`DishonoredGame_ID.exe`,
same minutes, `build/agentIE/gate_control.txt`) read 218.0 then 1049.4; a re-run of the stage on the IE exe right after
(`build/agentIE/gate_inputtest2.txt`) read **1047.8, 9 ok, 0 failed**.

Generator: `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentIE/regen_check`: 79/79
files byte-identical (CR ignored); the regenerated tree adds the four CppText includes, drops `disaudiopropagationinfo.cpp`,
`interptrackdialog.cpp`, `interptrackinstdialog.cpp` from the exclude list (691 -> 688) and the two `DishonoredLinkIE_*` anchors.
`bitfield_audit.py`: mismatches 0.

## 8. Hand-back

Sync: `python build/agentIE_sync.py` (base c772395 = main at hand-back: the worktree was moved there by stash / checkout / pop after
the measurements; one conflict, `Sources.cmake`, regenerated; generator 79/79 byte-identical and `bitfield_audit` 0 on the moved tree;
**the moved tree was not rebuilt** - two builds used); then `git apply build/agentIE/unplayer.patch` on the main tree (`--check` passes
at c772395 with the user's edits). The
generator inputs under `resources/docs/{types,symbols}` were copied into the worktree (ignored files) to run it there.
