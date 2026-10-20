# Agent IP — "the camera shakes too much" (wave 26, PHASE23 launch 4)

Worktree `build/agentIP_wt` (base `242fb05` = `b08193f` + the launch-4 docs line), build dir `build/agentIP` (RelWithDebInfo), staged
`DishonoredGame_IP1.exe` (build 1: diagnosis + bump smoother + head-bob node + blend-by-speed) and `DishonoredGame_IP.exe` (build 2: + the
fix). Base exe for "before": `DishonoredGame_IP0.exe` = the gate's `a47f38b` build (same code as `b08193f`). IDA only on
`build/agentIP/idb/{retail2013,shipping2012}_agentIP.i64` (decompiles `build/agentIP/d12`, `d13`). No FModel, no retail exe, no real input;
every run `game_input.py launch -windowed -nomovie`. Tools: `build/agentIP/ip_run.py` (run), `ip_jitter.py` (optical jitter, phase
correlation on the grey frame, centre crop, px of a 1920-wide frame; `log:<LOG>:<tag>:t0:t1` picks the -apshottime captures by world
time), `ip_parse.py` (contributor table from `-ipdiag`).

**Builds: three invocations.** Build 1's first invocation stopped on one compile error (a 31-argument `debugf` in `ipdiag.cpp`, over the
vararg limit); it was fixed and the same build resumed (only that unit + link). Build 2 is the final one. Named here because the rule
is two.

## 1. The cause

The camera was not shaking from a camera anim, a camera shake or low health. It was the **out-of-breath head layer**:
`Ply_Player_at`'s `DisAnimNodeBlendByPhysicalCondition_0` (an `AnimNodeAdditiveBlending`) adds `ADD_Head_OutOfbreath` on top of the body.
Retail weights it every tick by `ADishonoredPlayerPawn::GetAimStability` (breathlessness x `m_fBreathlessnessAnimWeight`, and **0 in a
dialog or a soirée**); our `TickAnim` was unported, so the additive stayed at the package's weight 1 for the whole game — the head
breathing as hard as after a long sprint. `m_fBreathlessness` itself was never written either (the player `Tick` was an empty shell).

What it does to the view depends on the camera influence:
- **The ride / any soirée** (`DishonoredCamera_AnimDriven`: view = camera bone location **and rotation**): pitch swung ±2.3° at ~0.8 Hz
  for the whole ride (world 140–150: range 4.64°, 0.70–0.80° per 0.1 s). This is the shake.
- **Gameplay** (`DishonoredCamera_PlayerControl`: bone location only, rotation from the controller): the bone bobbed 0.6 uu up/down while
  walking; small.

Ported: `UDisAnimNodeBlendByPhysicalCondition::TickAnim` (2012 0x6bded0 / 2013 0x688d80), `ADishonoredPlayerPawn::GetAimStability`
(0x7032f0 / 0x6a9030), the breathlessness head of the player `Tick` (0x715b10 / 2013 0x6bb1e0, found by its writes of +2920), and
`UDishonoredInventoryItem::IsItemZoomed` (0x849cc0 / 0x7fed40) which it needs.

Also ported in the same pass (faithful, measured):
- `UDisAnimNodeBlendByHeadBob::TickAnim` (2013-only class, 0x688c30; registration VA 0x1309e68, vtable 0x110bad8 slot 85 the only override):
  child 0 = `m_BobAmount`, child 1 = the rest. **Unported both weights were 0 and the first sprint failed `UAnimNodeBlendBase::GetBoneAtoms`'
  `LastChildNonAddIndex` check** (`IP0s.log`, `DisAnimNodeBlendByHeadBob_14` / CustomSprint) — on `main` sprinting from the L_Tower_P spawn
  kills the game.
- The bump smoother (`FDisBumpDetectInfo::DetectBump` 0x6c5030, `OnActivate_Derived` 0x6c07a0, `GatherInfluence_Derived` 0x6c51a0,
  2013 body with the arm-follow weight): the reaction group's first influence, the one IF named as missing. On the dock steps it is active
  40 of 55 walking frames, offset mean 4.1 uu (max 16.5) — the pawn's step-ups (up to 21 uu in one frame) now reach the view at most
  1.07 uu/frame of residual instead of the full step.
- `UDishonoredAnimNodeBlendBySpeed::CalcSpeed` (0x6f7100 / 0x689900): stick / 2D speed for the player, top speed for NPCs.

## 2. Contributor table (per-frame view change; `-ipdiag`, `-ipdiag` A/B switches restore each pre-port behaviour in the same binary)

| window | contributor | before | after |
|---|---|---|---|
| ride, world 140–150 (AnimDriven) | out-of-breath layer, view pitch range / per 0.1 s | 4.65° / 0.80° (IP2kb, all switches) — 4.64° / 0.70° (IP1k, breath unported) | **0.11° / 0.021°** (IP2k) |
| ride | optical jitter, median px1920 per frame pair (world 120–150.4, 303 pairs) | 13.21 (0.52°), \|dy\| 14.3 | **0.52** (0.020°), \|dy\| 0.60 |
| ride | camera bone in the pawn frame, range fwd/right/up uu | 1.06 / 1.25 / 0.97 | 0.74 / 1.25 / 0.42 |
| ride | "CamShake" sequence on the player tree | weight 0.18 → 0.01 over 140–145, stopped | unchanged (not a contributor) |
| standing post-ride (152–160, PlayerControl) | everything | 0.02 px | 0.01 px |
| walking on the boat/dock (233.5–236.4) | camera bone up/down (bob) | 0.6 uu range (breath layer) | 0.00 |
| walking | pawn step-ups reaching the view | full step (up to 21 uu/frame; 75 uu in IP1k) | bump smoother: residual 1.07 uu/frame mean |
| walking | view rotation | 0 (PlayerControl takes the controller's) | 0 |
| any | engine camera anims / shakes | none: `CameraAnim`/`CameraShake` are stripped from 2013 (agent I); `ModifierList` holds one `CameraModifier_CameraShake` that the first-person path never applies | – |
| any | low health | Health 100/100 in every frame of every run; no post-process target active | – |

Optical jitter against `retail.mp4` (same tool, 10 fps, centre crop):

| | retail | ours before | ours after |
|---|---|---|---|
| ride end (retail t 97–107.5 = world 140–150.3) | 0.40 median / 0.95 mean | 13.21 / 14.42 (world 120–150.4) | **0.52 / 1.02** (1.3x / 1.07x) |
| standing after the ride (retail t 108.4–112) | 0.04 / 0.70 | 0.02 (IP0k2, 0.05 median at 198–202) | 0.01 / 0.01 |
| walking at the dock | **no reference** (see §3) | 2.46 (IP0k2, 9 pairs at 4 fps under load) | 0.76 / 1.61 (28 pairs) |

Ride (`cutscene_compare run --tag IP2`, re-scored at the constant offset a = 13.569 − soirée enter 56.32 = −42.751 with
`build/agentIK/rescore.py`): **SSIM .837, MAD 7.78, zoom 1.00** (main: .72–.747 / MAD ~10.2). The tool's own fit read .666 (slope).

## 3. Where the brief was wrong (with the measurement)

1. **"low-health camera anim"**: Health 100/100 on every frame; it is the out-of-breath layer (physical condition), driven by
   breathlessness, at weight 1 because its node was unported. The user's guess was close in kind (a condition-driven head anim).
2. **"camera anims, shakes, UCameraAnimInst"**: none exist at runtime (stripped in 2013; the one shake modifier is never applied in first
   person). The Dishonored path is the influence groups + the skeleton.
3. **"retail.mp4 t 104–110 is hand-played after the ride"**: t 104–107.8 is still the letterboxed ride; 107.8–112.1 the player stands
   (the "Press [Unbound]... to move" hint: the user's bindings are 0). There is no walking in retail.mp4, so the walking 1.5x criterion
   has no retail number; the standing one is met (0.01 vs 0.04).
4. **The shake is in the ride, not in walking**: AnimDriven takes the bone's rotation, PlayerControl does not. Walking showed no view
   rotation change at all.
5. **GHOST/SLOMO**: inactive unless typed; no delta applied twice (`git diff` of UnPlayer.cpp / UnController.cpp read, not touched).
6. **The influence groups** at runtime are `[0 Core, 1 NULL, 2 Reaction, 3 Disable]`, so `UpdateViewTarget`'s `Groups(1)->m_Weight =
   m_BobAmount` hits nothing; and on the New Game route `m_BobAmount` is **0.50** (roll 0.70) while the user's retail OPTIONS.SAV has id
   108 = **1.0** (the spawn route reads 1.0) — a profile-defaults difference for IJ.
7. `DetectBump`'s game-info bit (+980 bit 0x10) is `m_bNewNPCPhysWalking`, logged 1: every fast height change counts as a bump.

## 4. Open (not fixed, no build left)

- **Second sprint crash** on the L_Tower_P spawn route with build 2 (`IP2s.log`): `AnimNodeBlendDirectional_33` "SprintDirectionalBlend"
  whose weighted children (`CustomSprintN_Camera`, `CustomSprintW_Camera`) are all `bIsAdditive` fails the same
  `LastChildNonAddIndex` check one node below the head-bob node. Retail's engine evidently allows an all-additive blend there (or flags
  the children differently); an Engine `UnAnimTree.cpp` package.
- With the breath layer off, the walking camera bone does not move at all (range 0.00): the head-bob additive (`Head Bob On`) shows no
  bob on this route; whether retail bobs while walking cannot be checked against retail.mp4.
- `-ipdiag anim` cannot list the player's sequences (their `NodeTotalWeight` is not maintained on the Edge path); `exec getall` was used.
- The rest of the player `Tick` (§ status csv), `DisGetCarryCorpseState` in `CalcSpeed`.

## 5. Gate

`run_regression.py --build-dir ../agentIP --no-build --exe-name DishonoredGame_IP.exe --log-prefix IPg_ --only d3d9,inputtest,newgame`
(`build/agentIP/gate.txt`): **23 ok, 0 failed**; criticals 0/0/0, `unported_natives` 0, `probe_natives` 3, `inputtest_moved` 1007.1,
`d3d9_frames` 1620, `newgame_on_mission` 88, `newgame_pawns` 29. Generator run in the worktree (`--sdk --module-header --sources-cmake`):
two CppText includes per header, four units off the exclude list, four anchors.

Runs: `IP0s` / `IP0k2` (base), `IP1k`, `IP1r` (ride anim dump), `IP2k`, `IP2kb` (A/B), `IP2s`, `build/cmp/IP2`, `build/cmp/IP2_const`.
