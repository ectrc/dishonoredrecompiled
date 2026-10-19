# Agent IB: NPCs after the cutscene, the locomotion tick (wave 26, PHASE23 package IB)

Worktree `build/agentIB_wt`, made at main `5ddc8a1` (= `33d1c80` plus one PHASE23 line, no source difference). Build dir `build/agentIB`
(RelWithDebInfo). There are no commits. Staged exes:
- `DishonoredGame_IB0.exe` is HEAD. It is the warm-up of build 1: the whole tree compiled before my edits, and the edits went into the same build-1 invocation.
- `DishonoredGame_IB1.exe` is build 1: `-ibdiag` plus two A/B switches.
- `DishonoredGame_IB.exe` and `DishonoredGame_IB2.exe` are build 2, the hand-back tree.

Two builds, both clean (0 errors, 0 C4263/C4264). Every run went through `cutscene_compare.py run` (which uses `game_input.py launch`) with `-windowed -nomovie` and ini isolation.

Measurement scripts:
- `build/agentIB/ib_measure.py <log>`: the `-ibdiag` table.
- `build/agentIB/leg.py <log>`: Worker B's first leg.

Runs are `CC_IB1a..d` and `CC_IB2`. The shots at world 150/155/160/170/185 are in `build/cmp/IB1{a,b,c,d}/ours/`; `build/cmp/IB2/` covers world 50..175 plus 185.

**Deviation from the brief (named): 1a and 1b are one binary.** Build 1 carried two runtime switches:
- `-ibphystick` is exactly `build/agentHC/revert_step5_loco.py`: no policy registration, and `PreAsyncWorkTick` runs from `physWalking` before `MovePawn`. The revert's `UpdateLocomotion()` call is left out because that function is empty.
- `-ibnovel` restores HEAD's velocity source.

So 1a = `-ibnovel`, 1b = `-ibnovel -ibphystick`, 1c = the candidate fix on the manager tick, and 1d = the fix on the physWalking tick. All four ran from the same exe under the same load, A concurrently with B and C concurrently with D. That freed build 2 for the fix. Build 2 removes both switches.

## 1. The A/B table

Pawn names:
- `_26` is the captain.
- `_27` is the tiller guard.
- `_20` is the WorkerB of SeqAct_Interp_11.
- `_3` is the 865-uu walker HC measured. HC's "Worker B walk" is `_3`, not `_20`.

The window is world 150.6..170.6, after the ride.

| | 1a HEAD (manager tick, HEAD velocity) | 1b revert step 5 (physWalking tick) | 1c manager + retail velocity | 1d physWalking + retail velocity | **IB2 = 1c + retail start-poly trace** |
|---|---|---|---|---|---|
| `_3` first leg, interior speed (once-a-second samples inside the walk) | 121.3 u/s, **samples 72..130** | 123.2, all 123 | 122.9, all 123 | 122.9, all 123 | 119.4 (one 36 sample while accelerating, then 123 flat) |
| `_3` leg 10845/22461 -> 11636/22102 | 870 uu | 861 uu | 863 uu | 866 uu | 860 uu, 123 u/s steady |
| `_26` physics / base at 150.6 | 1 / none (teleported off the deck, §2.2) | same | same | same | same |
| `_26` go-to | Kismet `DisSeqAct_AIGoToActor_3` -> `DishonoredNavPoint_0` (2418 uu); **FilterPathingFail at world 155.6: no start poly** | same | same | same | **path 17 points / 2447 uu, FilterPathingFail 0**; stuck in the start-move loop (roots 8:173 7:34 6:17 9:25), moved 0 |
| `_26` moved in 20 s | 0 | 0 | 0 | 0 | 0 |
| `_27` | phys 1, base boat, 1 request at its own spot (arrived), no go-to; moved 0 | same | same | same | same |
| `_20` | in WorkerB's looping matinee SeqAct_Interp_11 the whole run, phys 2, 1 arrived request; moved 0 after key 0's 46.6 uu | same | same | same | same |
| `m_bStuckedInAWall` while `Velocity.Size2D() > m_fVelocityToStartToMove` | 0 frames (all 4 pawns) | 0 | 0 | 0 | 0 |
| FilterPathingFail (`_26`/`_27`/`_20`) | 1/0/0 | 1/0/0 | 1/0/0 | 1/0/0 | **0/0/0** |
| in PreAsyncWork policy / ticks per s / moves per s | 1 / 22 / 22 | 0 / 22 / 22 | 1 / 23 / 23 | 0 / 23 / 23 | 1 / 19 / 19 |
| physics after detach (`StopRetargeting`) | `_26`, `_27` PHYS_Walking | same | same | same | same |
| Critical | 0 | 0 | 0 | 0 | 0 |

The game ran at about 22 fps on every run, with four other agents loading the machine.

### Reading the table
1. **The tick slot is not the cause** (this is the plan's "most likely wrong" #2).
   - Both slots walk at about 121..123 u/s.
   - HC's 95 u/s comes from `walk.py`. It measures from the last stationary sample to the end of the second leg, so the turn and the stop are included. On these logs the same script reads 90 (1a) and 73 (1b).
   - The real A-vs-B difference is that 1a's per-second speed swings between 72 and 130. That comes from the velocity source (§2.1), and 1c removes it on the manager tick.
2. **The ride NPCs do not stand still because of the dynamics.**
   - `_26` never had a path, because its start-poly test failed (§2.2).
   - `_27` has no go-to on this route.
   - `_20` is in a looping matinee for the whole game.
   - Physics comes back to PHYS_Walking at the detach. So the physics restore, ApplyLocomotionModifier and the policy Register/Insert are not implicated.
3. **With the start poly fixed (IB2), `_26` gets its path and then shows exactly the user's symptom: start animations in place** (§3).

## 2. What was ported
Every address was resolved by hand in `build/agentIB/idb/{shipping2012,retail2013}_agentIB.i64`. Decompiles are in `build/agentIB/d12/`, `d13/`.

### 2.1 The velocity source (`arkcomponentlocomotiondynamic.cpp`)
**`ComputeDynamic` tail.** 2012 `0x58ca10` and 2013 VA `0x94bec0` have the same tail.
- It runs after the root-motion reset, unless the pawn is PHYS_RigidBody or PHYS_Falling, or `IsRootMotion() && Mesh->RootMotionMode == RMM_Ignore`.
- On a flat floor: `Velocity = m_CurMove / dt * GetFinalSpeedMultiplier()`.
- On a floor with Z between 0 and 0.99: the speed is `|m_CurMove.XY| / dt * multiplier`, along `m_CurMove` projected onto the floor plane.
- `Acceleration = Velocity / dt`.
- `m_PreviousVelocity = Velocity`.

**`FullMovePawn`** (2012 `0x580ae0` / 2013 VA `0x93fb00`):
- The move is `Velocity * dt` of the move's own frame.
- It falls back to `m_CurMove` only when that move is non-zero while `m_fCurMoveSpeed == 0` and `!IsRootMotion()`.
- After the move, `Velocity = ClampLength(actual / dt, |Velocity before the move|)`.

HEAD moved `m_CurMove` (a distance computed for the manager tick's dt) in the next frame's `physWalking`, then re-derived Velocity from the measured move. The speed therefore followed the frame-time jitter between the two calls. That is 1a's 72..130.

Named deviation: the move itself is still `UWorld::MoveActor` (HEAD's bring-up), not retail's 4.7 KB swept move.

### 2.2 The start-poly trace flags (`arkcomponentlocomotionpath.cpp`)
Retail traces with `0x30DF` (`TRACE_AllBlocking | TRACE_Visible`) in both databases, in two functions:
- `UpdateStartLocAndVerifyIfOnValidPoly`: 2012 `0x57f8a0` / 2013 VA **`0x93f240`**. HEAD's banner said 0x53ed10, which is inside another function.
- `CanNPCPathfindFromLocation`: 2012 `0x587460` / 2013 VA `0x946770`.

HEAD used `TRACE_World`, which ignores non-mover actors. The captain's ground trace (he stands on `SkeletalMeshActorMAT_2`, the boat) therefore hit nothing, and the test failed with the pawn on the mesh. The log shows it:
- candidate poly 2/3 has `inside2D 1`, and the nearest point equals the start;
- 5 s of "return to mesh" follow, then a teleport onto the same spot (hence base "none");
- then FilterPathingFail, and Kismet gets Failed.

The same `nearest == start, found 1, 5.01 s` pattern hits `_4`, `_6`, `_8` and `_20` at world about 55.

Named deviation: 2013 adds two out-parameters, the navigation handle and a flag cleared when the start is the pawn itself. The 2012 signature is kept, since nothing here reads either.

### 2.3 Re-diffed, faithful, unchanged
- **`PreAsyncWorkTick`** (2012 `0x58d5f0` / 2013 VA `0x94caa0`). Retail has **no physics and no `m_bStarted` gate**: only editor, the `m_bNoTick` bit and dt > 0.0001. It ticks PHYS_None pawns too. **2013 `0x54ac70` is `Starting`**, not `PreAsyncWorkTick` (the plan's address). `Starting` (2012 `0x58b750`) registers with the PreAsyncWork policy unconditionally, as HC wrote it.
- **The policy.** `Register` `0x578ba0`, `Insert` `0x577fc0`, `DoneTicking` `0x578c50` and `Tick` `0x579d80` are what `arkcomponentpolicy.h` does, quirk for quirk:
  - register mid-tick into the iterating list;
  - insert AT the last not-greater index;
  - Count snapshot;
  - `RemoveSwap`.

  They are retail's own behaviour, so there was nothing to fix.
- **`physWalking`** (2012 `0x7ad240`) also tests a GameInfo flag at `+0x3D0 & 0x10` that ours does not. Named, not touched.
- **`ApplyLocomotionModifier`** (2012 `0x7aeec0`, 72 B) needs the unported virtual `GetModifierFromBodyStance` (2012 `0x7aef10`, 648 B, decompiled in d12). Not implicated: the modifier is 0 on every sample. The other missing call in the `MAT_FinishAIGroup` tail is the soirée Stop stim, which is IC's.

## 3. Still open, with the measurement
On IB2, `_26` and `_3` (after its first leg) cycle the start-move states 6 -> 7 -> 8 -> 9 and never move:
- `-gadiag` stack `rm 1 0.00/0.00/0.00`;
- about 18 frames per second flagged stuck;
- measured velocity 0, with one `vel 13/1/0` sample.

Two causes, which two builds could not separate:
- **(a) Root motion.** The start and turn animations carry their root motion in the Edge locomotion joint, which Plan B does not read (agentHA.md §0.5, open item 1). In START the animation owns the pawn, so it moves 0.
- **(b) Blocked move.** On the path, `MoveActor` is blocked almost every frame (`Hit.Time < 0.1`). The captain stands at the boat's bow (`build/cmp/IB2/ours/w0165.024.png`), so the boat's collision is the likely blocker. Retail's own swept `FullMovePawn` is not ported.

The next step is a third build: either a diag build (hit actor and time per `FullMovePawn`) or the locomotion half of `AnimLeafCallback`. I did not take it.

The captain may walk but will not turn until IC (FaceTo).

`_27` and `_20` have nothing to do on this route. `_27`'s only request is its own arrived spot, and `_20` stays inside SeqAct_Interp_11's loop.

## 4. Acceptance, measured on build 2
| check | result |
|---|---|
| Worker B 865 uu at 115..125 u/s | `_3` 860 uu, 123 u/s steady (about 7.4 s with the acceleration) — **met** |
| each ride NPC > 200 uu in 20 s after 150.6, or the brain says why | **not met for `_26`** (path found, start-move loop, §3); `_27`: no go-to; `_20`: looping matinee |
| `m_bStuckedInAWall` never TRUE while vel2D > m_fVelocityToStartToMove | 0 frames, every run — **met** |
| FilterPathingFail 0 for the three | 0/0/0 — **met** (it was 1 on `_26` in every build-1 run) |
| HH ride SSIM ≥ .69 vs `build/cmp/final` | **0.728**, MAD 11.58 -> 10.57, zoom 0.99 — **met** |
| gate d3d9+inputtest+newgame | 21 ok and 2 load failures: `d3d9_frames` (330 / 450 / 600) and `inputtest_moved` (403 / 218). Same-machine alternation of inputtest alone: IB 1072.6 then 516.0, HEAD (IB0) 484.4 then 477.8 (`build/agentIB/seq_*.txt`). The metric follows the load, not the tree. `unported_natives` 0, `probe_natives` 3..4, all criticals 0, `newgame_pawns` 29 |
| retargeting components still tick | 186 `anchor err` samples on `_26`/`_27`, max 0.08 uu |
| generator / audit | `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentIB/regen`: 79/79 identical; `bitfield_audit.py` mismatches 0 |

## 5. `-ibdiag`
Once a second for each of `_26`, `_27`, `_20` and `_3`:

`ibdiag <pawn> world at moved phys base retarget possessed ctrls goto pfail | started inpolicy disabled hidden root stuck (frames fast) curyaw real vel2d req arrived pathend path pf curmove target mod ticks moves`

There is also one line per FilterPathingFail hit. The counters are kept outside the 616-byte component, in a static map. `FArkComponentPolicy::IsRegistered` exists for the diag only.

## 6. Where the plan was wrong
1. The tick slot does not explain 95 vs 123 (§1). Both slots walk at about 123 u/s; the measurement method and the velocity source account for the rest.
2. The ride NPCs were not "stuck in locomotion":
   - the captain had no path (trace flags);
   - the tiller guard has no order;
   - `_20` is in a looping matinee.
3. Addresses: `PreAsyncWorkTick` is 2013 `0x54caa0`, and `0x54ac70` is `Starting`. `UpdateStartLocAndVerifyIfOnValidPoly` is 2013 `0x53f240`.
4. The policy "bugs" (Register mid-tick, Insert shift) are retail's code.
5. Physics does not stay PHYS_None after the matinee: StopRetargeting gives both boat NPCs PHYS_Walking.
