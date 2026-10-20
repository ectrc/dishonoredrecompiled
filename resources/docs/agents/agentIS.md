# Agent IS — sprinting asserts in the player anim tree (wave 26, PHASE23 package IS)

Worktree `build/agentIS_wt` (cut at `4ff8c11` = `2898de5` + the IS launch line; sync base `fd3f90b`, no package file changed in
between), build dir `build/agentIS` (RelWithDebInfo), **one build** (`build1.log`, rc 0), staged `DishonoredGame_IS.exe`. IDA only on
`build/agentIS/idb/{retail2013,shipping2012}_agentIS.i64` (decompiles `build/agentIS/d12`, `d13`; helper `is_ida.py`). No FModel, no
retail exe, no real input; every run `game_input.py launch -windowed -nomovie` through `build/agentIS/is_run.py`.

## 1. The answer

The sprint blend is a **stock `AnimNodeBlendDirectional`** ("SprintDirectionalBlend", `AnimNodeBlendDirectional_33`), the additive
input of an `AnimNodeAdditiveBlending` (`AnimNodeAdditiveBlending_21`, child `[1] Additive Anim Input`). Its four children are
`DisAnimNodeSeqPlayer`s (`CustomSprint{N,S,W,E}_Camera`) playing the additive head sprints `ADD_Head_Sprint{N,S,W,E}`. Once a sequence
has its animation, retail's `ChildrenHasChanged` marks that child additive, so while sprinting **every relevant child is additive** and
the blend itself becomes additive.

- **Retail's `UAnimNodeBlendBase::GetBoneAtoms` has no `LastChildNonAddIndex` check** (2013 VA `0x5a3bd0`, 2012 `0x1b0ec0`: no string
  but `Array.h`'s; with no non-additive child it blends nothing). The `warnf` + `check` is the reference engine's.
- **Retail does not evaluate the player with `GetBoneAtoms`** (agent FT): the tree is built into an Edge blend tree. The directional
  node has no Edge build of its own: slot 91 of both directional vtables (stock `0x1001fb8`, Dis `0x110cd98`) is the blend-base thunk
  `0x5ba7f0` -> `UAnimNodeBlendBase::BuildEdgeAnimTreeEx` (2013 VA `0x5b9000`, 2012 `0x1c3f60`). That function, read in both builds:
  non-additive children are relevant at `Weight > 1e-5` (then BlendType-curved and normalised to 1); an additive child needs `Weight >
  1e-5`, a node and a curved weight `> 1e-5`. **No relevant child -> a base-pose leaf (flags 4). Additive children only -> a leaf with
  flags `0x44`** — `_edgeAnimProcessCommandList` (2012 `0x6238c0`) command 5 with flag `0x40` and no header writes zero translation,
  identity rotation, unit scale for every joint — and each additive child is added on it with operation 1
  (`_edgeAnimBlendJointsRelative`). The blend's output is the weighted sum of its additive deltas, which the parent adds as an
  additive child. 2013 bytes: `mov byte ptr [ebp+var_28+2], 44h` at `0x5b978e`, `..., 4` at `0x5b9715`.
- **Ported**: that rule in our CPU evaluator (`GetBoneAtoms`: identity atoms for the desired bones + the existing additive pass, or the
  reference pose; root motion identity), the reference check removed. Not a null check: the branch is the Edge build's.
- **Also ported, found on the same path**: retail's `UAnimNodeBlendDirectional::TickAnim` (2013 `0x593c80`) takes the direction from the
  virtual `GetDirectionVector` (slot 119, `0x58cb60`) and uses the actor's rotation as the look direction only when it asks for it,
  else `+Y`; `UDisAnimNodeBlendDirectional::GetDirectionVector` (`0xa89210`, same bytes as 2012 `0x6d87e0`) substitutes the stick
  (`PlayerInput` `RawJoyRight`/`RawJoyUp`) when `m_bUseControllerInputDir` is set. Our engine had neither; the player tree has such a
  node (`DisAnimNodeBlendDirectional_1`, the `Empty_Swim*` directional, seen ticking at the dock). The weight rule itself was already
  retail's (compared line for line with the decompile).
- `-isdiag` (engine): `isdiag tick` from every directional of a human-controlled pawn (velocity, accel, yaw, LOD, weights in/out,
  DirAngle, node/children/parents), `isdiag eval` + an `animdump` subtree whenever a blend takes the all-additive/no-child branch, once a
  second per node. `-isoldcheck` restores the reference warning + check for a same-binary A/B.

## 2. Measurements (all build 1)

| run | what | result |
|---|---|---|
| `IS1sb` (`-isoldcheck`, spawn, W+LeftShift) | A/B: the old check | `LastChildNonAddIndex != INDEX_NONE - Node: AnimNodeBlendDirectional_33, NodeName: SprintDirectionalBlend` at the first sprint frame, assert, 3 Criticals |
| `IS1s10` (spawn, W+LeftShift held 10 s from the start) | the port | 0 asserts, 0 Criticals, peak 601.4 |
| `IS1h` (spawn, 8 x 4 s sprints, 32 s of sprint) | the port | 0 Criticals, steady **600.0** Walking, 31 `isdiag eval leaf identity+additive` lines (the state persists while sprinting) |
| `IS1d`, `IS1d10`, `IS1d10m686`, `IS1d10p3095`, `IS1dh` (dock after the ride, sprint held 10 s in four headings) | the port | 0 asserts, 0 Criticals in all five; peak **600.0** Walking (`IS1dh` heading 0) / 600.2 |
| `IS1im` (IM's `im_s5` crouch/stand/jump + IMSTATE) | IM regression | `Empty_Sneak_In` / `Empty_Sneak_Out` / `Empty_JumpLandSmall`, upper actions granted, 0 refusals, 0 Criticals — as IM's table |
| `IS1m` (IM's `im_s6`, dock) | mantle | animated mantle at the 181.9 uu edge: Z rise 173.64 (diff -8.26), 1.209 s in Mantle, ends Walking — as IM |
| `build/cmp/IS1` | ride | SSIM **.843**, MAD 7.64, zoom 1.00 at this run's constant offset (-43.121; see §3.5) |
| gate (`gate.txt`) | d3d9 + inputtest + newgame | **23 ok, 0 failed**; criticals 0/0/0, `unported_natives` 0, `probe_natives` 3, `inputtest_peak_speed` 500.1, `inputtest_moved` 1014.6 (first attempt 315.9 under load, re-run by the tool), `d3d9_frames` 1230, `newgame_on_mission` 119, `newgame_pawns` 29 |

Generator: `--sdk --module-header --sources-cmake` in the worktree (one CppText include, `disanimnodeblenddirectional.cpp` off the
exclude list, its link anchor), `--out-dir` regeneration byte-identical; `bitfield_audit` mismatches 0.

**The sprint blend's weights against retail's rule** (`isdiag tick`, IS1h): sprinting straight, DirAngle 0.0000 and
`F 1.0000 B 0 L 0 R 0` (or `0.9998/0.0002` at DirAngle ±0.0003); while DirAngle turns towards the target at DirDegreesPerSecond,
DirAngle 0.6610 -> `F 0.5792 R 0.4208` = `R = DirAngle/(pi/2)`, `F = 1 - R` (retail's forward-right quadrant). The one frame in which
the sequences get their additive animations shows `F 0.5853 R 1.0000` at eval after `F 0.5853 R 0.4147` at tick: retail's
`ChildrenHasChanged` (2013 `0x593570`, identical to FT's port) renormalises the still non-additive Right to 1 when Forward turns additive;
the next tick rewrites all four. This is IP's "0.99978 + 1.0".

## 3. Where the brief was wrong (with the measurement)

1. **"the blend node's class (an Ark/Dis blend with its own TickAnim/GetBoneAtoms, unported like BlendByHeadBob)"**: it is the stock
   `AnimNodeBlendDirectional` (`class AnimNodeBlendDirectional` in `isdiag eval`), no Edge build of its own (slot 91 = the blend-base
   thunk in both vtables). The divergence was the engine's blend base, not a game node.
2. **"one node lower than BlendByHeadBob"**: its parent is `AnimNodeAdditiveBlending_21` (as `[1] Additive Anim Input`, additive 1), not
   the head-bob node.
3. **"the weighted children (CustomSprintN_Camera, CustomSprintW_Camera) are all additive"** (IP2s.log: N 0.9998, W 1.0): those names are
   the sequence nodes; the animations are `ADD_Head_Sprint*`, and the weight pair summing to 2 is a one-frame transient of retail's own
   `ChildrenHasChanged`, not a wrong TickAnim (§2).
4. **"retail's GetBoneAtoms has the assert"**: neither 2012 nor 2013 has it; the reference engine's `warnf` + `check` were never retail's.
5. **"cutscene_compare ride SSIM >= .80 at the constant offset (IP's .837)"**: IP's -42.751 is IP's run's own anchor (13.569 - soirée enter
   56.32). This run entered the soirée at 56.69: at -42.751 the same frames score SSIM .760 / MAD 9.70; at its own anchor (-43.121)
   .843 / 7.64. The two ours-frames are visually identical at world 100 (`pair_w0100.278` vs IP2 `pair_w0100.042`); only the retail
   frame picked differs.
6. **"sprint held 10 s at the dock"**: the dock stop (9826, 23064, 2411.8) is a pocket — +Y and -X walls within ~100-180 uu, water at
   +X and -Y. Holding sprint 10 s is possible (0 asserts in every heading), but 600 u/s lasts ~1 s before the wall or the water (the
   player then swims at 260). The 600 at the dock is a ~1 s reading; the 10 s at 600 is the spawn route's.
7. **The spawn route**: walking W for 3 s first takes the player off the spawn platform (IS1s: fall from Z -225 to -1457,
   `DmgType_Fell`, Health 0); sprinting from the spawn survives the drop in some runs (IS1h) and not in others (IS1s10 ends at Health 0).
   That is fall damage, not the anim tree; IM's checks were run in their own runs.

## 4. Deviations (named)

- The Edge rule runs inside `UAnimNodeBlendBase::GetBoneAtoms` (our CPU stand-in for the Edge job), as HA did for the stack; retail's
  `GetBoneAtoms` itself would add onto an unset buffer here.
- Only the all-additive / no-child case of `BuildEdgeAnimTreeEx` is transliterated; the rest of the CPU blend (normalisation of the
  non-additive weights, which the Edge build does and `GetBoneAtoms` does not) is unchanged — not reached on this path, left for a later
  package.
- `-isoldcheck` keeps the reference warning + check behind a diagnosis switch.

Files: `Engine/Src/UnAnimTree.cpp`, `Engine/Inc/EngineAnimClasses.h`, `DishonoredGame/Src/disanimnodeblenddirectional.cpp`,
`DishonoredGame/Inc/CppText/UDisAnimNodeBlendDirectional.h`, generated `DishonoredGameAnimClasses.h` / `Sources.cmake` /
`DishonoredGameRegistrants.cpp`. Sync: `build/agentIS_sync.py`. Runs: `build/agentIS/IS1*_run.txt`, logs `IS1*.log`, `build/cmp/IS1`,
`build/cmp/IS1_const`, `build/agentIS/gate.txt`.
