# Agent HL — the player Kismet toggles that HI's PlayerAction target rule reaches (wave 25, PHASE22 follow-up of HI)

Worktree `build/agentHL_wt` (made at `33201ec`, fast-forwarded to `835dc2b` before build 1 and to `9da5dbc` before build 2), build
dir `build/agentHL` (RelWithDebInfo), staged as `DishonoredGame_HL1.exe` (build 1) and `DishonoredGame_HL.exe` (build 2, the measured
one). IDA on private copies `build/agentHL/idb/{retail2013,shipping2012}_agentHL.i64` through `resources/tools/ida/ha_ida.py`;
decompiles in `build/agentHL/{d12,d13}/`. Two builds, both clean (0 errors).

## 1. What was ported (all in `Src/dishonoredplayerpawn.cpp`, declarations in `Inc/CppText/ADishonoredPlayerPawn.h`)

| Native | exec 2012 / 2013 rva | body 2012 / 2013 rva (2013 vtable offset) | what it does |
|---|---|---|---|
| OnToggleTutorial | 0x635de0 / 0x5ef840 | 0x704680 574 B / 0x6aa300 574 B (+2016) | for each `m_AffectedTutorials` byte, `s_pInstance->m_TutorialTrackers[i]->m_bEnabled` set / clear / flip on input link 0 / 1 / 2 |
| OnToggleAchievementEval | 0x6359c0 / 0x5ef420 | 0x704080 559 B / 0x6a9df0 561 B (+1972) | `m_AchievementTrackers[m_Achievement].m_bEnabled` set / clear / flip; on an off->on edge zeroes every `m_StatValues` entry the achievement's `m_StatEvals` names (`m_pPlayerStats->m_Achievements` of the tweaks from `GetTweaks_Derived`, else the `UDisTweaks_PlayerPawn` CDO) |
| OnToggleChoke | 0x635d80 / 0x5ef7e0 | 0x704480 336 B / 0x6b1bd0 300 B (+2012) | input bit 0x20000 at `DisInputMaskLevel_Kismet`: enable / disable / toggle |
| OnToggleJournal | 0x635a80 / 0x5ef4e0 | 0x70b200 204 B / 0x6b5c40 220 B (+1980) | input bit 0x10000 at the Kismet level: link 0 enables, link 2 toggles, link 1 or no impulse disables |
| OnTogglePowerWheel | 0x635a20 / 0x5ef480 | 0x70b130 204 B / 0x6b5b60 220 B (+1976) | the same as Journal with bit 0x800 |

The execs are the stock `P_GET_OBJECT; P_FINISH; virtual call`. The 2013 bodies were resolved from the exec thunks' `call [vtbl+N]`
and the `ADishonoredPlayerPawn` vtables found through the data xrefs of the only named body (`OnToggleTutorial`, VA 0xaaa300; vtables at
VA 0x111a3b0 / 0x11b0b50 / 0x11cdba8, identical in slots 490-505). 2013 differs from 2012 only by inlining
`ADishonoredPlayerController::EnableControllerInput` (`PC+1644+4*7` = `m_InputEnableMask[Kismet]`) and by Choke's toggle path calling
the pawn's `EnableControllerInput` (2013 rva 0x6b1160 = 2012 0x702550); the behaviour is the same. Retail's `OnToggleTutorial` reads the
trackers of `s_pInstance`, not `this`; kept. The one deviation: a NULL tracker is logged and skipped where retail would dereference it
(never hit; both trackers on the route exist).

`-hidiag` (HI's switch) logs each call: the tracker or achievement and its new state, or the Kismet mask after the change.

## 2. Results

Route `-finewgame=50 -fosoiree -hidiag -diskismet -fwkismettrace` on build 2 (`build/agentHL/HL2_run.txt`, HL2.log; build 1's HL1.log
is identical):

```
fwkismet t=0.00  OnToggleTutorial   DisSeqAct_ToggleTutorial_4 link 1: tracker 1 pPerfectBlockTracker enabled 0
fwkismet t=0.00  OnToggleTutorial   DisSeqAct_ToggleTutorial_2 link 1: tracker 7 pDefenseTutorialTracker enabled 0
fwkismet t=0.00  OnToggleAchievementEval DisSeqAct_ToggleAchievementEval_0 achievement 22 enabled 1 -> 0, 0 stats reset
fwkismet t=50.44 OnToggleChoke      DisSeqAct_ToggleChoke_0: kismet mask 0x5ffff      (0x20000 cleared)
fwkismet t=50.44 OnToggleJournal    DisSeqAct_ToggleJournal_0: kismet mask 0x4ffff    (0x10000 cleared)
fwkismet t=53.98 OnTogglePowerWheel DisSeqAct_TogglePowerWheel_0: kismet mask 0x4f7ff (0x800 cleared)
```

No `native not ported` line for any of the five. What remains on the route is `UDisGFxMoviePlayerBase::execOnFocusLost`,
`ADishonoredNPCPawn::execOnNPCMarkForVanish` and `UOnlineSubsystemSteamworks::execExit`, all also present in HI's runs.

Gate (`run_regression.py --build-dir ../agentHL --no-build --exe-name DishonoredGame_HL.exe --log-prefix HLg_ --only
d3d9,inputtest,newgame`, `build/agentHL/gate.txt`): **23 ok, 0 failed**. `unported_natives` **0** (HI: 4); `probe_natives` **4** (HI: 8;
the four toggles left the list, leaving OnTeleport_Native, CalcPlayerSwimAccelRate, OnOverrideAwarenessDisplay, PlayDying_Native);
`d3d9_criticals`, `inputtest_criticals` and `newgame_criticals` 0; `newgame_on_mission` 121; `newgame_pawns` 29. `inputtest_moved` read
402.7 on the first attempt and 1050.2 on the stage's built-in load-sensitive retry, while another agent's build was running.
Generator into `build/agentHL/regen`: 79/79 files byte-identical ignoring CR. `bitfield_audit.py`: mismatches=0.

## 3. Where the brief was wrong (with the measurement)

1. **Five natives, not four.** The route also reaches `execOnTogglePowerWheel` (HI1.log/HI2.log: `native not ported:
   ADishonoredPlayerPawn::execOnTogglePowerWheel` at world ~54). The gate's d3d9 stage never gets that far, so the ratchet said 4. I
   ported it with the other four.
2. **"Each a thunk to a small virtual"** (agentHI.md §6) is not right. The virtuals are 204-574 B; Tutorial and AchievementEval contain
   loops, and AchievementEval resets player stats through the tweaks.
3. **HI's "no native stub hit, script handlers"** (agentHI.md §4.4) is contradicted by HI's own HI1.log line 1147 (`native not ported:
   ADishonoredPlayerPawn::execOnToggleTutorial`) and by its gate.
4. **None of the bodies calls a further unported system** (achievement evaluation, journal UI, tutorial manager). They only flip
   tracker bits and mask words and zero stats, so there was no subsystem boundary to stop at inside them.
5. **2013 addresses:** the brief's 0x5ef420 etc. are rvas (VA = rva + 0x400000). In `retail2013_named.i64` the 2013 bodies are unnamed
   except OnToggleTutorial.

## 4. Boundaries (named, not ported)

- **Readers of the mask bits** 0x800 (power wheel), 0x10000 (journal) and 0x20000 (choke): no native code in this tree tests them. The
  disable is recorded in `m_InputEnableMask[Kismet]` but changes nothing visible until the power-wheel, journal and choke input
  handlers are ported.
- **Readers of `UDisTutorialTracker::m_bEnabled`** (the tracker subclasses' evaluation) and of `FAchievementTracker::m_bEnabled`
  (achievement and stat-streak evaluation, `execOnEvalAchievement`, etc.) are not ported and were not measured.
