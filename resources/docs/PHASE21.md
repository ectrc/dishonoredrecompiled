# Wave 25 — the guards see, the captain walks off the boat, and the chain runs without a teleport

Wave 24 (`PHASE20.md`) ended the spawn fall (GG: retail flushes level streaming before the first tick), found
that every NPC had been 3× its size (GH: the look-at blend added the reference pose), ported the stop-animation
system and retail's nav-mesh poly containment (GK), ported the attention process (GI), and GJ's equip-path
result is in its outcome section. What stops the first mission now, each measured and left:

1. **Nothing sees.** `FDisComponentVisionNPC` (14), `FDisComponentVision` (22), `FDisComponentObservable` (26)
   are skeletons; without them no attention slot is ever created and no guard ever looks at, reacts to or
   challenges the player. The attention process is proven with a stand-in (GI).
2. **The captain does not disembark**, so the player walks into his back on the straight line to the dock: his
   go-to's start poly is refused one stage inside retail's `GetPylonAndPolyFromPos` port — the pylon octree query
   or the LOCAL_SPACE height test (GK §4.1). And the faithful route is different: retail never teleports the
   guards — it carries them on the boat's sockets through `FDisComponentRetargeting` (the Captain socket ends on
   the dock polys, 10042.7, 23044.4, 2337.3).
3. **The chain has never run without a teleport**: ride → walk off → `DisTrigger_8` → the captain walks up →
   `Use` → `DisTrigger_16` → the Empress. Items 2 and GJ's result decide it.
4. One of eight streaming sublevels stays unloaded on every run, on the control too (GG; its name is not logged).
5. At Busted, `DDH_PLAYER_APPROACHES` fires every tick (the dialog side's cooldown is not honoured — FR's area).
6. With a sense present, 8 of 9 runs crash at exit in `IncrementalPurgeGarbage` (a freed pointer with a garbage
   pool entry; never an attention object) — the vision package inherits it (GI).

## THE FOUR RULES

**Fullscreen**: `-windowed` on every launch. **Input**: `game_input.py` only; nothing touches the real mouse,
keyboard or focus. **Builds**: `build_queued.py` only, **two builds per package** (GI used thirteen working-build
invocations and FZ five — the rule stands: a third build is a hand-back with the measurement, not a fourth
attempt); the override file lifts the cap and the queue only when the coordinator says the user is away, and
under it **a gate's game stages can starve** (GG: every run stalled at "Initializing Engine" under three
concurrent builds; the 0e5207a gate lost its d3d9 run and walked 216 uu) — re-run the stage alone on the same
exe and report both numbers, never relax a bound. **Retail**: no `Dishonored.exe` launch unless the coordinator
says the user is away and permits it.

## The gate

47 checks, **47 ok on HEAD**; `unported_natives` 0 ≤ 0; `verify_phase2` 3/3; every `*_criticals` 0. The generator
rule and the merge checks (regeneration byte-identical, `bitfield_audit` 0) on every merge; a Core change needs a
`CoreSmoke` link; merge main's HEAD into your worktree before hand-back and make your sync read 0 flagged.

## Packages

### GL — vision: the guards see the player

GI: `StartSeeingVisibleThing` (2012 `0x7b08c0` / 2013 `0x7591a0`) → `HandleTargetSighted` is ported and waits for a
sense. Port `FDisComponentVisionNPC`, `FDisComponentVision` and `FDisComponentObservable` from retail (62 functions;
2012 names at the rvas, 2013 checked by decompile), the vision tweaks they read, and whatever creates them on the NPC
(`PreBeginPlay_NativeComponents` — GA's pattern) and registers observables on the player. Remove GI's `-gisight`
stand-in once the real sense drives the same chain. Then the exit crash GI bounded (8 of 9 runs with a sense; the
minidumps in `Logs/unreal-v9411-2026.10.14-17.31.18..18.17.58.dmp`) is yours: open the dump before rebuilding.
Acceptance: on GB's route the two gate guards (`_22`, `_23`) sight the player as he walks up (`-gidiag` slot →
Busted), `lookatcpnt requests` > 0 for an enemy relationship if the map has one, and a clean exit on 10 of 10 runs
with the sense on.

### GM — the captain walks off the boat: the start poly, then the sockets

GK §4.1 names the test: inside the ported `GetPylonAndPolyFromPos`, log the pylons the octree query returns and the
polys before and after the filter for `NPCPawn_26` at 10092/23057 and `NPCPawn_4` (Worker A); the (0,0,80) world box
against `APylon::GetBounds(WORLD_SPACE)` or `UpdateStartLocAndVerifyIfOnValidPoly`'s LOCAL_SPACE height test on this
imported mesh (LocalToWorld origin X=249) is the fault. Fix it faithfully. Then the faithful route: retail carries the
guards on the boat's sockets by `FDisComponentRetargeting` (`UpdateRetargeting` 2012 `0x8b37e0`, 4,308 B, plus the game
event dispatcher it calls) and never teleports them (`m_bHasBeenOnNavMesh`); port enough that the captain rides on the
`Captain` socket and stands on the dock polys at the ride's end. Also GB's residue if reached:
`ASkeletalMeshActorMAT::MAT_SetPhysics` (2012 `0x3317c0`) + `InitGroupInst`'s `bBecomeDynamic`. Acceptance: `-gadiag` 0
`pathfail` for `_26` and `_4`; `AIGoToActor_3 'Curnow GoTo 1'` → `Success (Last)` and the captain beside the player
on GB's route without `-fwteleport`; the player's W reaches the dock and `DisTrigger_8` fires the guards' scene.

### GN — the chain end to end, and the streaming level that never loads

The first act, as a package: from New Game through the ride, the dock, the guards' scene, the captain, `Use`, the
Empress scene, to the second scene starting — measured on main with GM merged (or with GM's sync applied in your
worktree if it has not landed), every stop named with the log line. Plus GG's follow-up: which of L_Tower_P's eight
streaming levels stays unloaded on both builds, why (`-gedeathdiag`/`ggdiag` extended to name it; retail's
`PostCommitMapChange` save-state half — `FGameState::LoadLevel`, `PrepareForArrivals` 2012 `0x721070` — and ED's
`RestoreLoadedLevels`), and whether it is the one the chain needs. Acceptance: a capture series of the chain, the
furthest point reached stated as a fact, the eighth level named.

### GO — the arms with the sword: the item anim states

GJ: the sword attaches to `handAttachment_R_jnt` and stays in hand through the ride, but **the arms play the
empty-hands base pose** — the hand bone sits 68 units below the view, so the sword is never in frame and the user's
"arms when I shouldn't see them" is still not reproduced. What poses the arms with an item is the item anim-state
system, bodiless: `UDishonoredInventoryItem::BindToAnimStateOwner`, `FindAnimStatePickers`, `SyncPawnAnimState`,
the upper-FSM `UStatePlayerEquipChange` (GJ's instant-equip stand-in replaces its request) and `TransitionItemIn`.
Port them from retail (2012 names at the rvas, 2013 checked) so the sword stance plays in first person; then measure
the arms through the ride (GJ: the boat does NOT force the hands empty — `m_bForcesUndrawBeforePlaying` is FALSE —
so whatever hides the arms in retail's soirée is in the stance/body-mode path, FZ's `SetBodyMode` + the item's
soirée context slot `CancelActiveContexts`, not a force-to-empty). Also: the NPC inventory override so the guards
carry swords (`-gjdiag` names the slots), and the console dispatch seam GJ named (`ScriptConsoleExec` after the
ride). Acceptance: capture pairs with the sword drawn — the stance in frame before the ride, whatever retail does
during it (state the measurement), the stance back after; the user's report reproduced or shown impossible on this
route with the reason.

## Rules for agents

- The four rules, the two-build limit, the generator rule, the merge checks. Report the gate's numbers as read,
  and a load miss with the stage re-run on the same exe.
- **Open the minidump before rebuilding** (`build/agentFN/tools/mdmp.py`). **Resolve every retail address yourself**
  in both databases (your own copies under `build/agent<XX>/idb/`; if the IDA MCP workers are all taken, GE's
  headless `build/agentGE/ge_decomp.py` pattern works). **Never use the FModel MCP tools.** C4263/C4264 are errors.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Wave 24: GH corrected the cause of
  the stretch (three earlier attributions were wrong), GG the order premise, GK two nav premises, GI two.

## Acceptance

1. Before/after against `build/head_rel` (rebuilt at the wave's base) through the same `game_input.py` script,
   captures viewed.
2. `run_regression.py`: **47 ok, 0 failed, 0 skipped**; `unported_natives` stays 0.
3. 0 errors / 0 C4263 / 0 C4264 in the gate's own build log.
4. Every address cited resolved by hand, both databases.
5. `resources/docs/agents/agent<XX>.md` + `agent<XX>_status.csv` in the worktree; `build/agent<XX>_sync.py` naming
   every file and function, self-checked, 0 flagged against main.
