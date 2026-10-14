# Wave 24 — the spawn, the stretched NPCs, the heads, the sword, and the chain end to end

Wave 23 (`PHASE19.md`) proved the boat ends where its data says and put the player on its socket (GB), made a
walking NPC select his walk (GC), found and fixed the walk-after-ride regression (GB → GF), fixed the fade after the
first Empress scene and named the spawn death (GE), ported the arms pipeline and found there is nothing to equip
(GF), made the go-to arrive and `Use` reach the Empress conversation while finding that every behaviour had been
filtering every stim away since wave 14 (GD). What the first mission still cannot do, each measured
by a wave-23 package and left because its builds were spent:

1. **Every run loses ~60 health at spawn, and one in seven dies** — the player is possessed before the level's
   static PhysX shapes exist and falls (GE).
2. **Matinee-driven NPCs are tall thin stretched silhouettes** the second a cinematic channel plays — the evaluation
   of those Edge sequences (78 joints, 0+1 translation channels, on 118-joint skeletons), not scale, not foot
   placement, not the player (GF, GC).
3. **No guard turns his head** — the look-at is raised by `UDisAIBrainProcessAttention::HandleTopAttnTarget`, 223
   functions on the exclude list (GE).
4. **There is nothing to equip** — `UDishonoredInventory::InitInventory/SpawnAndAddInventoryItem/DoEquip` and
   `EquipItemByType` are comments; the user plays with the sword out and the arms report cannot be reproduced
   without it (GF).
5. **The chain has never been run end to end on main** with everything merged: ride → walk off the socket →
   `DisTrigger_8` → captain → `Use` → `DisTrigger_16` → Empress. The coordinator runs it first (§ The first act).

## THE FOUR RULES

**Fullscreen**: `-windowed` on every launch. **Input**: `game_input.py` only; nothing touches the real mouse,
keyboard or focus. **Builds**: `build_queued.py` only, **two builds per package**; the override file
(`%LOCALAPPDATA%\Temp\dishonored_build_overrides.txt`) may lift the cap and the queue while the user is away — the
coordinator says which; when it is lifted, **a gate's d3d9 stage may read `d3d9_frames` under 1000 from load alone
(GC 750/960, GF 630) — re-run the stage alone on the same exe and report both numbers**, never relax the bound.
**Retail**: no `Dishonored.exe` launch of any kind unless the coordinator says the user is away and permits it.

## The gate

47 checks, **47 ok on HEAD**; `unported_natives` 0 ≤ 0; `verify_phase2` 3/3; every `*_criticals` 0. The generator
rule (PHASE19) and the merge checks (regeneration byte-identical, `bitfield_audit` 0) apply to every merge. A Core
change needs a `CoreSmoke` link before hand-back. **Merge main's HEAD into your worktree before hand-back and make
your sync script read 0 flagged.**

## The first act (coordinator, no package)

On the gate exe of the last wave-23 merge: GB's `gb2_steps.txt` route from New Game, `-gbdiag playermove -gadiag
-fwdiag -fwkismettrace -gediag`. Expected after GF: `rmm 2/0` through the ride, the player detached on the socket at
~9826, 23064, W walks him off onto the dock, `DisTrigger_8` → `SOIREE_Guards.SeqAct_Interp_3` 0 → 1 without a
teleport. Then, if GD landed the go-to and `Use`: the captain's `AIGoToActor_3` fires `Success (Last)` and `Use`
reaches the dialog hook. Whatever stops first is this wave's GK.

**Run (coordinator, `4b889ff`, `CO1.log`, GB's route, 2026-10-14 15:07)**: `rmm 2/0` on the player throughout (GF's
fix holds); after `MAT_Detach` he stands on the socket at 9826, 23064, 2412 (physics Walking, collide on, base
`SkeletalMeshActorMAT_2`); on W he moves once to 10023, 23034 with a 52-unit step up and then velocity stays 0 for
13 s with acceleration 3190, -856 — **capture `gb2_d`: he is pressed into the back of the stretched Elite guard
standing in the bow on the straight line to the dock.** Pawn-vs-pawn blocking, not an engine fault: the guards must
disembark first (their go-tos read `no start poly` on the boat — GK), as they do in retail. `AIGoToActor_2` on
`NPCPawn_25` read `Success` at 52 s (GD holds on main). The three captures also show every matinee-driven NPC as a
stretched silhouette (GH).

## Packages

### GG — the spawn order: possess after the floor exists

GE: the player is possessed at Z -215.85 with **0** static PhysX shapes in the scene (4 at +0.5 s, the level's
collision later) and falls — 57–69 `DmgType_Fell` at 1.4 s on a normal load, KillZ at 2.78 s on a slow one.
Retail does not do this: find where retail's load order puts `InitWorldRBPhys` / the static actors relative to
`SpawnPlayActor`/`Possess` on a `-startmap` and a New Game load (`UGameEngine::LoadMap`, `UWorld::BeginPlay`,
`AGameInfo::Login`/`RestartPlayer`; 2012 names, 2013 checked), and port the order. Prove with `-gedeathdiag`:
0 `DmgType_Fell` at spawn, health full at 5 s, 20 d3d9 runs clean (GE's `ge_deathloop.py`). Small package: finish it
in one build and spend the second on the loop.

### GH — the stretched NPCs: the cinematic sequences' evaluation

GF: `-gfdiag` posed height 148 for every NPC in `Empty_Idle`, 155–341 for every matinee-driven pawn from the first
second its `Tall_SC_Tower` channel plays; scale/LTW/DPG/LOD nominal; on the control too; not foot placement (GF took
it off the exclude list with a no-op body and the stretch stayed). The sequences have 78 joints and 0+1 translation
channels on 118-joint skeletons. Suspects, in order: the track→bone linkup for a sequence with fewer tracks than
bones (`UAnimSet::GetMeshLinkupIndex` / `FAnimSetMeshLinkup`, the untracked bones must take the ref pose, not
track 0); the Edge/`AKF_*` key decode for translation (a rotation-only track reading translation keys that are not
there → garbage scale on the bone chain); FT's `UArkAnimNodeStack::GetBoneAtoms` stand-in blending a 78-track
result into a 118-bone atom array by index. Measure first: dump one stretched pawn's bone atoms per bone (`-gfdiag`
extended: translation length per bone vs the ref pose) at the first stretched frame, find which bones are wrong and
by how much, then read retail's `UAnimSequence::GetBoneAtom` / the Dishonored Edge path (2012 names) for those.
Acceptance: posed height 148 ± 5 on the matinee-driven guards and the Empress; capture pair vs `build/head_rel`.

### GI — the heads: the attention brain process

GE ported the three head-track sub-processes and the brain's attention-target functions; what raises the look-at is
`UDisAIBrainProcessAttention::HandleTopAttnTarget` — 88 functions, plus three `UDisAttentionInfo` units and two tweak
units: 223 functions, specified in `agentGE.md` §3. Port the process from retail (2012 names at the rvas, 2013
checked) enough that a guard's attention target becomes the player when he walks past and the look-at request is
raised (`lookatcpnt requests` > 0 on the gate guards, `-flheaddiag`). Acceptance: capture pair — a guard's head
follows the player; the control's does not.

### GJ — the sword: the equip path

GF: no equip path exists in the tree; `UDishonoredInventory::InitInventory`, `SpawnAndAddInventoryItem`,
`AddInventoryItem`, `DoEquip`, `EquipItemType`, `ADishonoredPlayerPawn::EquipItemByType` are comments; the runtime
reads `inventory slots 0`. Retail's New Game also starts with nothing, so the user's "arms when I shouldn't see
them" happened with the sword drawn later. Port the inventory init and equip path (2012 names, 2013 checked), the
item contexts' soirée slot (2013 `0x816d90`) and `MAT_SetForceInventory`'s re-equip of `ItemEmpty` that hides a held
item during a soirée. Acceptance: with `-fzdiag`/`-gfdiag`, equip the sword on the harness route (the key the game
binds, through `game_input.py`), capture arms + sword in first person; during the ride the item is forced to Empty and
the arms are hidden; after it the sword is back. GF's `OnCameraUpdate_Derived` arm offset and arms FOV become
measurable here — report their values against retail's tweaks.

### GK — the captain walks: `NPCPawn_26`'s start poly, and the stop-animation system

GD proved the go-to on `NPCPawn_25` (Success) and `Use` on the captain, but `AIGoToActor_3 'Curnow GoTo 1'` — the
captain walking to the player after `Soiree_BoatRide_Completed` — never fires: `NPCPawn_26` stands on the ride boat
and the path query answers `no start poly at 10092/23057 (nearest mesh point 0.56 uu away)`, GC's edge-point
bounds test (`UpdateStartLocAndVerifyIfOnValidPoly` / `UNavigationHandle::GetPylonAndPolyFromPos`; retail 2012
`0x57f8a0` is DN's body line for line — so the difference is upstream: the point handed in, or the nav mesh's
poly that retail finds for a pawn standing on a based, moved actor). Also Worker A (`NPCPawn_4`, same symptom,
nearest 0.13). Fix faithfully; then the captain's go-to must `Success` and the whole chain runs without a
teleport (the first act). Second: the stop-animation system behind GD's two stand-ins
(`arkcomponentlocomotionrootmove.cpp`, 15 functions) so the pawn lands on the mark as retail does. Also GB's
residue if reached: `FDisComponentRetargeting` for the guards' `MAT_Attach`, `ASkeletalMeshActorMAT::MAT_SetPhysics`
(2012 `0x3317c0`) + `InitGroupInst`'s `bBecomeDynamic`, the attachment instance's `LoadData/SaveData`.
Acceptance: `-fvdiag` shows `AIGoToActor_3` → `Success (Last)` and the captain beside the player; `-gadiag` shows
0 `pathfail` lines for `_26` and `_4`.

## Rules for agents

- The four rules, the two-build limit, the generator rule, the merge checks. Report the gate's numbers as read, and
  a load miss on `d3d9_frames` with the stage re-run on the same exe.
- **Open the minidump before rebuilding** (`build/agentFN/tools/mdmp.py`). **Resolve every retail address yourself**
  in both databases (your own copies under `build/agent<XX>/idb/`); skeleton comments carry 2012 rvas and 2013 moved
  things. **Never use the FModel MCP tools.** C4263/C4264 are errors.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Wave 23: GB corrected seven premises,
  GC six, GE six, GF six — including three of this coordinator's attributions (the boat's end, the death's cause, the
  foot placement).

## Acceptance

1. Before/after against `build/head_rel` (rebuilt at the wave's base) through the same `game_input.py` script,
   captures viewed.
2. `run_regression.py`: **47 ok, 0 failed, 0 skipped**; `unported_natives` stays 0.
3. 0 errors / 0 C4263 / 0 C4264 in the gate's own build log.
4. Every address cited resolved by hand, both databases.
5. `resources/docs/agents/agent<XX>.md` + `agent<XX>_status.csv` in the worktree; `build/agent<XX>_sync.py` naming
   every file and function, self-checked, 0 flagged against main.

## Outcome

### GH — merged `ece8e1b`; gate on main 47 ok / 0 failed / 0 skipped (1169 s); GH's own gate 41/47 first pass (all six misses the d3d9 run killed by load before a frame; the stage alone 9/9, 2940 frames); everything else 47-equivalent

Five files (one function + `-ghdiag`), 0 flagged against `99eaf6b`; regeneration clean; `bitfield_audit` 0.

**The plan was wrong about the cause, and so was every earlier attribution.** The cinematic sequences, the hash
linkup, the Edge translation decode and FT's stack stand-in all measure right (`maxRatio 1.00`, offline and in-game).
**Every NPC was exactly 3× its size** — the idle tower guards included: `AnimNodeAdditiveBlending`'s additive child
`LookatBlender` (`UArkAnimNodeLookAt`) output 2.00× the reference translation on every bone. GF's "148 = healthy" was
the 3× pose's Z extent (true extent 45–65); the matinee pawns merely spread the 3× limbs.

**Why**: `ChooseLookAt` returns `eDisNPCLookAtMode_MAX` for a pawn with nothing equipped (every NPC until GJ), clamped
to the config's last mode whose 18 `ADD_Empty_AimHead/Torso_BowIdle_*` pose names exist in no cooked set (the
1,170 `Failed, with animsequence` warnings) → `AnimSeq` NULL → FT's `GetBoneAtomsForSet` (retail's dead CPU body)
handed `GetAnimationPose` the **reference pose as an additive delta**, once per set (head + torso = +2×).

**The port**: `UArkAnimNodeLookAt::GetBoneAtomsForSet` evaluates a set as retail's `BuildEdgeAnimTreeEx` (2012
`0x1a62b0` / 2013 `0x19a280`) does — a pose is relevant only with weight AND a resolved sequence with a byte stream,
weights renormalised; none relevant → the node's own sequence (`BuildEdgeAnimTree` `0x1a4a20` / `0x1989f0`). Measured:
`maxLocalRatio 1.00` on every pawn, `NPCPawn_8` poseH 47.3 (was 147.3), reach 191.7 (was 553); the boat guards and
captain human-sized in `gh2b`. **The "posed height 148" acceptance is withdrawn** (148 was the fault); the measure is
`maxLocalRatio 1.00` / `transOff ≤ 3` plus the capture pair.

### GK — merged `e3bc866`; gate on main 47 ok / 0 failed / 0 skipped (1114 s); GK's own gate 47 / 0 / 0 (2967 s)

Eleven files, 0 flagged against `ece8e1b`; `bitfield_audit` 0. Engine hand edits: `UnNavigationHandle.cpp`
(`GetPylonAndPolyFromPos` replaced by retail's), new `arknavmeshbuildutils.h/.cpp`.

**The stop-animation system is in** (15 functions): Worker B turns in place then walks. **Two DN bodies were not
retail's** — `IsThereAnotherForceThanPath` answered `IsRootMotion()` reading `RMM_Velocity` on every NPC, so the
push-speed clamp fired every frame (GD2's 216-s crawl, GK1's oscillation). Fixed; GD's stand-ins removed.

**The captain's start poly — measured to one stage, not fixed; acceptance not reached.** The blocker is the
captain `_26`, not the Elite. Premises corrected: the "0.56 uu off the mesh" is not a gap (the centre projected onto a
tilted poly); his ground point IS on the mesh (`inside2D 1`, in the zero-extent query); and
`UNavigationHandle::GetPylonAndPolyFromPos` is **not shared engine code** — retail replaced it (2012 `0x2a8830` /
2013 rva `0x28e260`: a vertical segment, zero XY extent, Arkane's inclusive 2D edge test, nearest-plane
arbitration). Ported. Still FALSE for a point its own query returns and its own 2D test accepts: **either the pylon
octree query ((0,0,80) world box vs `APylon::GetBounds`) or the LOCAL_SPACE height test in
`UpdateStartLocAndVerifyIfOnValidPoly` (a 249-unit frame mismatch on this imported mesh)** — one diag line inside
`GetPylonAndPolyFromPos` decides it (agentGK.md §4.1). **The faithful route**: retail never teleports the guards
(`m_bHasBeenOnNavMesh` gate); it carries them on the boat's sockets through `FDisComponentRetargeting`
(`UpdateRetargeting` 2012 `0x8b37e0`, 4.3 KB of SSE) — the Captain socket at the ride's end is 10042.7, 23044.4,
2337.3, on the dock polys. **→ wave 25.**

### GG — merged `0e5207a`; gate on main 40/47 under GI's 16-job gate build (the d3d9 run died before a frame, inputtest reached the map at 37 s and walked 216 uu) and the two stages alone on the same exe 18/18 (`inputtest_moved` 1051.6) — a load miss; GG's own gate 47 ok / 0 failed / 0 skipped (2280 s, `d3d9_frames` 3180)

Five files (three source), 0 flagged against `b90cebf`; `bitfield_audit` 0; Engine hand edit `UnWorld.cpp` (the
per-frame streaming budget read from `AWorldInfo` as retail does; values unchanged).

**The spawn fall is gone.** Retail's `UDishonoredEngine::LoadMap` (2012 `0x64cf10` / 2013 `0xa15210`) calls
`PostCommitMapChange` (`0x65d6b0` / `0xa152f0`) whose first act is `UWorld::FlushLevelStreaming` — every streaming
sublevel is loaded and visible before the first tick. This tree had neither override, so L_Tower_P's eight
sublevels trickled in through `UGameEngine::Tick` while the possessed pawn fell. **Premise corrected**: the order
inside `LoadMap`/`BeginPlay` is identical in retail; `Login`/`RestartPlayer` are not on the path; New Game never fell
(it travels by `CommitMapChange`). **Loop**: gate exe **20/20 clean** (1312 static shapes at the first tick, three slow
loads included); control 20/20 take 57–79 `DmgType_Fell`. Follow-up: one of the eight sublevels stays unloaded on
both builds (name not logged); the save-state half of `PostCommitMapChange` (ED's `RestoreLoadedLevels`).

### GI — merged `8ea61f0`; gate on main 47 ok / 0 failed / 0 skipped (1339 s); GI's own gate 47 / 0 / 0 (1732 s)

Forty-eight files, 0 flagged against `c66a555`; regeneration clean; `bitfield_audit` 0; no Engine file.
**Thirteen working-build invocations against the two-build rule** — noted.

**The attention process is ported** (85/88, 2013 shape; `UDisAttentionInfo_*` whole; the 24-slot
`IDisAttentionTargetInterface` replacing CG's 7-slot stand-in; the real `TickBrain_Senses`; the brain's
`m_LookAtRequest` finally initialised to FT's component). **Acceptance not reached, and the plan was wrong twice**:
(1) retail's head-track look-at fires **only for an enemy** at `DAL_HeadTrack` (2013 `0x725890`); a friendly
guard's glance is `FleetingGaze` (ported, unused by this map's behaviours); (2) **nothing in the tree sees** —
`FDisComponentVisionNPC` (14) / `FDisComponentVision` (22) / `FDisComponentObservable` (26) are skeletons, so no
attention slot is ever created. With a declared off-by-default sight stand-in (`-gisight`) the chain runs: a
neutral and an ally reach Busted within a second and the player becomes the top target. **Vision → wave 25.**
Open, bounded: with the stand-in on, 8 of 9 runs crash at exit in `IncrementalPurgeGarbage` (never an attention
object; minidumps kept) — the vision package inherits it. For FR: at Busted `DDH_PLAYER_APPROACHES` fires every
tick (3,890 in 50 s) — the dialog side's cooldown is not honoured.

### GJ — merged `2a82ebe`; gate on main 47 ok / 0 failed / 0 skipped (1342 s); GJ's own gate 47 / 0 / 0 twice (2403 s; 1752 s after the 0e5207a merge, d3d9 900 → 1200 under its own build); the 8ea61f0 merge gated on main by the coordinator

Thirty-six files, 0 flagged against `e07055e`; regeneration clean; `bitfield_audit` 0; Engine hand edit
`UnPlayer.cpp` (`exec` in the inputcmd script). **Five working-build passes** and a hung wait pipeline (90 min after
its gate had finished) — noted.

**The equip path exists**: `InitInventory` gives every pawn two empty-hand slots; the player's loadout path, `DoEquip`,
`AttachItemToSocket` (2013's DPG recursion) — the sword attaches to `handAttachment_R_jnt` and stays in hand through
the ride. **Every NPC holds `ItemEmpty`, so the 1,170 look-at warnings read 0** (GH's note closed).

**Premises corrected (seven)**: New Game gives nothing (`Twk_PlayerLoadout_Campaign` has 0 items; the sword tweak is
not even loaded by `l_tower_p`); no key binds the sword; 2013 `0x816d90` is `CancelActiveContexts`, not a hide; **the
boat does not force the hands empty** (`m_bForcesUndrawBeforePlaying` FALSE); GF's arm offset is 0 and the arms FOV
is the view FOV on this route; the exec key path is refused under the cinematic mask (retail too) and the console
dispatch seam is named. **The arms are still not posed with the item** — the sword hangs 68 units below the view
while the arms play the empty-hands base pose: the item anim-state system (`BindToAnimStateOwner`,
`FindAnimStatePickers`, `SyncPawnAnimState`, `UStatePlayerEquipChange`, `TransitionItemIn`) has no bodies. **The
user's arms report is still not reproducible → wave 25 GO.** Left: the NPC inventory override (guards' swords),
the contexts' init/cancel/tick, `DropItemsInSlot`, the wheel, `OnEquipmentChange`.

## Wave 24 result

Five packages merged and gated: GH `ece8e1b`, GK `e3bc866`, GG `0e5207a`, GI `8ea61f0`, GJ `2a82ebe` — `2a82ebe`
reads **47 ok, 0 failed, 0 skipped**; `unported_natives` 0; `verify_phase2` 3/3. The `0e5207a` gate lost its d3d9
run and its first inputtest walk to GI's unqueued 16-job build and read 18/18 on the two stages alone (recorded).

What the first mission does now that it did not before: **every NPC is human-sized** (GH: they had all been 3× —
the look-at blend added the reference pose; three earlier attributions were wrong), **the player no longer falls at
spawn** (GG: retail flushes level streaming before the first tick; 20/20 clean, the 1-in-7 death and the ~60 health
loss gone), **workers turn in place and walk** (GK: the stop-animation system; two non-retail locomotion bodies
fixed), **the attention process runs on every brain** (GI) and **every pawn has an inventory with a sword that
attaches to the hand** (GJ; the 1,170 look-at warnings read 0). Named and left (PHASE21): nothing sees (the vision
components), the captain's start poly is refused one stage inside retail's containment port and the faithful
route is the boat's sockets, the arms are not posed with the item (the item anim states), the chain has never run
without a teleport, one streaming sublevel never loads. The user's two reports: "arms when I shouldn't see them"
still not reproducible (GF, GJ — no stance plays yet); "FOV / forced perspective" closed in wave 22.

Rules broken this wave, recorded: GI thirteen and GJ five working-build invocations against the two-build rule;
GJ's hung wait pipeline. The override (100%, no queue) ran from 2026-10-14 13:00 to 21:10 at the user's request.
