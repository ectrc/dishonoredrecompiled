# Agent ID — voice lines and subtitles: the conversation path (wave 26, PHASE23 package ID)

Worktree `build/agentID_wt` (made at `5ddc8a1` = `33d1c80` + docs; fast-forwarded to main `3ef4426` at hand-back, see §6), build dir
`build/agentID` (RelWithDebInfo, `build/agentID/id_build.cmd`), staged as `DishonoredGame_ID.exe`. Two builds, both 0 errors.
IDA on private copies `build/agentID/idb/ID_{retail2013,shipping2012}.i64`; decompiles in `build/agentID/decomp2013/` and
`decomp2012/`. Runs through `build/agentID/id_run.py` (game_input.py launch, `-windowed -nomovie`, own inis; quits after the last
`-apshottime` mark; `--exec` queues one console line after a given shot). No retail exe, no real input, no FModel.

## 1. Headline

**A conversation line now takes the time of its voice, and the voice is posted.** On the unattended route (ID2, build 2, no teleport)
the Kismet hook `SOIREE … DisSeqAct_DialogInputs_20 'SheWaiting'` plays `DisDialogOneShot_13 … DisConv_Blurb_0` on the boat's elite
guard (`DishonoredNPCPawn_27`) every 10 s:

```
iddiag t=162.56 blurb start …DisConv_Blurb_0 speaker DishonoredNPCPawn_27 event Play_F3EAC6D40633C67014DEC53E298F9F7E
        text 'Corvo, she's waiting, you should go see the empress.' radius 3000 main 1
iddiag t=162.56 voice duration 3.47 s on DishonoredNPCPawn_27            (AK_Duration -> m_fPlayingAudioDuration)
iddiag t=162.56 voice hint 3.47 s (source: 52 characters / 15 cps, clamped 1..12; the silent backend has no media length)
iddiag t=162.56 subtitle add main 'Corvo, she's waiting, …' on DishonoredNPCPawn_27, 1 voice(s), radius 3000
iddiag t=166.53 blurb done …DisConv_Blurb_0 speaker DishonoredNPCPawn_27, end delay 0.00
```

7 lines in world 162.6–227.2 (start→done 3.5–4.2 s of world time for a 3.47 s wall-clock hint), all with text, voice data found by
GUID in `DisDialogVoiceData_OneShot_2`. Before: FR's base defaults finished every blurb in the frame it played (ID0 `played
DisConv_Blurb_0 -> output 0`), and blurbs had no in-game data at all.

**No subtitle is drawn, and the reason is measured (§3).** The line is added to the main list, but `FDisSubtitlesInfo::Tick` never
picks an owner: no conversation is dominant and every voice is FLT_MAX away because the audio propagation cache and the Wwise
listener are not fed in this tree. 0 `OnMainSubtitleChanged`, 0 `SetSubtitles`; the new `subtitle` detector reads 0.02–0.05 % on all
31 ID2 captures.

**The Empress scene still fades back** (ID3, `-fwteleport=DisTrigger_16@160`): `gediag fade amount 0.000` from world 163 to 206,
`Interp_0` finishes at 16.06 s, no Critical.

## 2. What was ported (2013 rva, 2012 in brackets; * = by vtable slot or caller)

| unit | functions |
|---|---|
| `disconv_blurb.cpp` | `UDisConv_Blurb::PostLoad` 0x897fa0* [0x8dd430; 2013 is 2300 B: + text clean-up], `InitNode` 0x894810*, `GetClassVars_Derived` 0x8948e0, `ExtractText_Derived` 0x892ce0, `PlayNode_Derived` 0x89fb90, `TickNode` 0x89fbb0, `PlayBlurb_Shared` 0x89deb0*, `TickNode_Shared` 0x89e0f0, `StartBlurb` 0x89bb50*, `InterruptTickingNode` 0x89bb00, `GetBlurbLookTarget` 0x89b770* [0x8e0e30]; `UDisConv_Blurb_InGameData::GetBlurbSpeakerData` 0x8946c0 [0x8e0cc0], `StopBlurbSpeakerData` 0x88c550 [0x8dd3f0]; `-iddiag` |
| `UDisConv_Node.h` | `ExtractText_Derived` 0x88af20 (empty string; NonWord's is the same) |
| `disconvspeakerinterface.cpp` | `StreamDialogBlurb` 0x88bcc0, `PlayDialogBlurb` 0x8a0930 (retail's 8 parameters; FR declared 4), `FindTextBlurb` 0x8a2920, `IsReadyToSpeak` 0x88d7e0, `IsDonePlayingBlurb` 0x89a380 |
| `disconversationcomponent.cpp` | `PlayDialogBlurb` 0x89fbf0, `StreamDialogBlurb` 0x88ba50*, `UnprepareAkEvent` 0x88b0b0, `IsReadyToSpeak` [0x8df100], `IsDonePlayingBlurb` 0x89a160, `IsAnyAkEventPlaying` 0x896740, `FindTextBlurb` 0x8a2410, `StopAkEvents` 0x88bae0, `StopDialogBlurb` 0x8a02f0* [0x8ef700], `ConvPrepareEventCB` 0x88b090, `DisConvAkEventCB` 0x8938b0* (incl. the `ADisSpeaker_PA` group branch), `ms_VoiceBoneName` "head_jnt" (0xb8f290) |
| `disdialogvoicedata.cpp` | `PostLoad` 0x892450* (vtable 0x1199cf8 slot 6), `RebuildLookupMaps` 0x8a1440, `FindTextBlurb` 0x8a1550 |
| `disconvglobalman.cpp` | `FDisSubtitlesInfo::AddSubtitle` 0x89cea0*, `KillSubtitle` 0x89d160*, `Tick` 0x89e660* (2013 adds the paused list), the three finders 0x88d5a0*/0x88d610*/0x88d680*; `TickGlobalConvMan`'s tail; `Term_GlobalConv` 0x8a5a90* (2013 body) |
| `disgfxmovieplayerhud.cpp` | `Tick_Subtitles` 0x79fe20 in PreAdvance's slot, `OnMainSubtitleChanged` 0x7af5d0, `OnSecondarySubtitleChanged` 0x7af6e0, `SplitSubtitleText` 0x7adf80, `ShowSubtitleText` 0x799690, `ShowNextMain/SecondarySubtitlePart` 0x79f210*/0x79f2d0* [0x8034a0/0x803560]; every Invoke through `DisHUDInvoke` (refuses an unbound clip) |
| `dishonoredaudiosystem.cpp` | the end-of-event queue: `DisAkEndOfEventCB` 0x793340, `ConsumeEndOfEventNotifies` 0x7a9970, `Init`'s 0x834-byte queue, `FinishDestroy` [0x80b7d0], `Update` 0x7b3520 (drain only); `UDisAkComponent::IsPlayingID` 0x7931b0, `GetShortestAudiblePathLength` 0x793220* |
| `disbehaviorblockingdialog.cpp` | `UDisBehaviorBlockingDialog::IsReadyToSpeak` 0x6c2640 [0x722ab0] (unit leaves the exclude list) |
| `akaudiodevice.cpp` | `UAkAudioDevice::GetAkComponent` search fixed (§4) |
| Wwise silent backend | `DishonoredWwise::SetDurationHint` + `EndElapsedPlaying` (the one deviation, §5) |

Named, not ported: FaceFX (WITH_FACEFX=0) in PlayDialogBlurb (StopFaceFXAnim, the PlayFaceFXAnim branch whose failure falls through to
the Wwise post), IsDonePlayingBlurb (IsPlayingFaceFXAnim), StopDialogBlurb (StopFaceFXAnim); StartBlurb's AI noises (hook flags 8 /
0x1020 → `UDisAINoiseManager::MakeNoiseForAI` 0x899d80, unported; the Kismet hook's flags 0x500 set neither); PlayBlurb_Shared's
2013 skip of "Hey, you in the mask! You look like a clown." under the game-info class at 0x14721f8 (never named) and its call into a
3-byte `retn 14h` (0xc715d0); blurb PostLoad's per-language override from `Localization\<LANG>\Dialog_*` (INT has 659 files, no
Dialog_*); voice-data PostLoad's m_TextHash (`DisStriHashWithScanFunc` unported); `DuckNonDominantAudio` 0x896cb0*; the rest of the
audio system's Update. Look targets reach `SetLookTarget`, whose body is the look-at package's.

## 3. Why no subtitle shows (measured)

1. **Subtitles are off by default in English.** `arksettings.cpp:317` gives profile setting 130 (`m_SubtitlesMode`) 0 for INT;
   ID1: `no subtitle … subtitles enabled 0 forced off 0 hide secondary 1`. retail.mp4 has no subtitle on any frame (the detector is on
   only at t 4.0–5.5, the Bink briefing text), so the user's retail ran with them off too. ID2/ID3 emulate "Subtitles: On" with
   `exec set Engine bSubtitlesEnabled True` after the second capture.
2. **No owner is chosen.** `FDisSubtitlesInfo::Tick` ranks a main line by 99 when its conversation is the dominant one, else by
   `1 - distance / voice radius`. `TickGlobalConvMan` never finds a dominant conversation for `DisDialogOneShot_13` (CalcGlobalHookDominanceWeight
   0: not blocking-player, no make-quiet; `iddiag dominant` fires only for the blocking Empress conversation in ID3). The distance is
   `UDisAkComponent::GetShortestAudiblePathLength`, which reads the component's audio propagation cache: filled by
   `UDishonoredAudioSystem::Update` (2012 0x818ff0's component loop: m_fMaxAudibleDistance = largest playing radius, sound point from the
   owner's bone, listener point/cell) — unported — and from `UAkAudioDevice::m_ListenerPosition`, which nothing sets
   (`UGameViewportClient` feeds `UAudioDevice::SetListener`, UnPlayer.cpp:1642, off-limits). So every line is FLT_MAX away.
   That is IH3's positioning work plus a listener feed; the HUD half is ported and waits for it.

## 4. Two defects outside the brief, found on the path

- **`UAkAudioDevice::GetAkComponent` never found its own component.** Retail (2013 0x5b17e0, 2012 0x5f8e40) searches the actor's
  `AllComponents` (2012 PDB name) by bone only; ours searched `Components` and also compared bStopWhenOwnerDestroyed, so every call
  built a new component and the voice posted on one was never seen playing. ID1 (build 1): blurb done 0.05 s after start; ID2 (build
  2, fixed): 3.5–4.2 s.
- **The silent backend's PrepareEvent callback passed 0**, where `CAkBankMgr::NotifyCompletion` (2012 0x929970) passes the event id
  for a one-event prepare; `ConvPrepareEventCB` stores it as `m_ReadyAkEventID` and `IsReadyToSpeak` waits for non-zero. Fixed for the
  measured builds; on the merge to 3ef4426 IH1's ported `NotifyCompletion` does the same, so this hunk was dropped.

## 5. The deviation (to be removed by IH2)

The silent backend ends every event on the next `RenderAudio`, so a faithful `IsDonePlayingBlurb` is TRUE one frame later.
`DishonoredWwise::SetDurationHint(playing id, seconds)` (in the public seam header, not the private `AkSilent` namespace the brief named:
that namespace is the backend's own and is not visible to the game) keeps one voice alive that many wall-clock seconds and fires its
AK_Duration callback; unhinted voices keep the next-frame end (FI). `PlayDialogBlurb` calls it under `#if DISHONORED_WWISE_SILENT` with
`clamp(characters / 15 cps, 1 s, 12 s)`: the backend parses only a bank's BKHD chunk (measured: every `Loaded bank` line), so there is
no media length to give. The hint ignores a seek time (only matinee dialog keys seek; IE). Named in the banners of
`disconversationcomponent.cpp` and `AkSilentSoundEngine.cpp`.

## 6. Where the brief was wrong (with the measurement)

1. **The Empress scene has no blurbs.** ID0/ID3 with `-frdiag`: `DisDialogOneShot_0 … DisConversation_3 → DisConv_Soiree_0`
   and `_27 … DisConversation_31 → DisConv_Soiree_4`, soirée nodes only; its spoken lines are the matinees' `UInterpTrackDialog` keys
   (IE). In world 150–220 the only `DisConv_Blurb` on the route is 'SheWaiting' (7× in ID2 without the teleport; once in ID1, none in
   ID3, because the teleport takes the player away before or after its first fire). "≥ 5 blurbs" is met by that one line repeated.
2. **The unattended route does not reach the Empress** (FW/GE): it needs `-fwdiag -fwteleport=DisTrigger_16@160` (the teleport is
   inert without `-fwdiag`). And without `-nomovie` the Bink briefing waits for a key forever (ID0 first attempts: world stuck at 50.2
   for 1000 s) — the coordinator's note.
3. **"One OnMainSubtitleChanged per blurb and a clear after" cannot hold** with the propagation cache and listener unfed (§3).
4. **Subtitles are off by default for INT**, and retail.mp4 has none, so the HH/IE comparison against retail has nothing to compare.
5. **`-apshottime=a:b:step` is not a range** in the game (only cutscene_compare expands it); a comma list is needed.
6. The brief's "AkSilent::SetDurationHint" is `DishonoredWwise::SetDurationHint` (§5). "PlayDialogBlurb's 4th parameter is a duration"
   (FR's declaration) — it is `m_fBlurbEndDelay`, unused by the 2013 body; the 6th is the voice radius.
7. Not in the brief but on the path: blurbs had no in-game data (`GetClassVars_Derived` unported, so no singleton), the end-of-event
   queue that lets a voice stop "playing" did not exist, and GetAkComponent was broken (§4).

## 7. Runs (`DishonoredGame/Logs/ID*.log`)

| run | exe | switches | what it measured |
|---|---|---|---|
| ID0 | head_rel | `-nomovie -finewgame=50 -fosoiree -frdiag -gediag -fwdiag -fwteleport=DisTrigger_16@160` | baseline: blurbs finish at once; Empress scene = soirée nodes; AS2 errors 14 |
| ID1 | build 1 | + `-iddiag -puidiag` | blurb starts, hint 3.47 s, done 0.05 s later (GetAkComponent); subtitles enabled 0 |
| ID2 | build 2 | no teleport, `exec set Engine bSubtitlesEnabled True` | 7 blurbs 3.5–4.2 s, 7 `subtitle add main`, 0 owner, 0 SetSubtitles; 31 captures, detector ≤ 0.05 % |
| ID3 | build 2 | teleport | Empress conversation dominant, fade amount 0.000 world 163–206, AS2 errors 14 (= ID0), 0 Critical |

Route AS2 errors: 14 on the control (ID0, head_rel) and on ours — 11× `no frame named 'PC'` + 2× `ClearAnimation` + 1×
`removeMovieClip`; the brief's 11 predates `-nomovie`'s front end path. Not changed by this package.

## 8. Gate (`build/agentID/gate.txt`, build 2 at 5ddc8a1)

`run_regression.py --no-build --exe-name DishonoredGame_ID.exe --only d3d9,inputtest,newgame,options`: **26 ok, 2 failed**.
`unported_natives` 0, `probe_natives` 4, criticals 0 in all four stages, `options_as2_errors` 39, `newgame_pawns` 29.
The two failures are load: `inputtest_moved` 216/218 — the control (`head_rel`, same minutes, `build/agentID/gate_control.txt`) reads
216/218 too; `options_log_lines` 1918 vs 2000 — the shortfall is the once-a-second census lines of a run that lasted fewer wall seconds
(the control's 3004 lines carry 195 `fmdrive alive` against our 118; the same per-frame content otherwise).
Generator: 79/79 files byte-identical (CR ignored) at 5ddc8a1 and regenerated on the merged tree; `bitfield_audit` mismatches 0.

## 9. Tooling

`resources/tools/cutscene_compare.py`: a `subtitle` detector — area x .2–.8, y .80–.92 (bottom-centre band), thin bright strokes
whose pixel 2 px down-right is dark (the line's drop shadow), so the hull/rail/plank edges that the plain text test fired on in retail
(53–65 s, 98–112 s) stay off. Selftest: a line drawn into retail frames 3.4–3.8 % vs clean 0.04–0.28 % (on > 1.0); retail on only at
4.0–5.5 (briefing text). Stats cache renamed `stats_v7.json` so a version change does not clobber other agents' cache. The band is
a choice (bottom-centre), not a measurement: no capture of ours shows a subtitle yet (§3); re-check it on the first one.

## 10. Hand-back state

The worktree was fast-forwarded to 3ef4426 (IH1) after the measurements: stash / checkout / pop. Conflicts only in
`AkSilentSoundEngine.cpp` (banner, includes, the bank region IH1 moved to AkAudioLib — this package's prepare-event fix there dropped);
`Sources.cmake` and the two generated headers regenerated. **The merged tree was not rebuilt** (two builds used): the first build of
the merge compiles my SetDurationHint hunks against IH1's backend (they sit in PostEventCommon / Voice / RenderAudio, untouched by IH1
apart from its -akdiag check). Sync: `python build/agentID_sync.py` (base 3ef4426).
