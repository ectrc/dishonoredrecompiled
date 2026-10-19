# Agent IN: the missing collision at the dock stair foot (wave 26, PHASE23 package IN)

Worktree `build/agentIN_wt` at `eb48851` (= `9850a24` + the PHASE23 docs commit). Build dir `build/agentIN` (RelWithDebInfo). No
commits. Two builds:
- build 1 (`DishonoredGame_IN1.exe`): `-indiag` (the probe). Its first compile stopped on two errors in `indiag.cpp` (a private
  `bStatic`, a 34-argument `debugf`); the fix was compiled incrementally in the same build directory before anything was staged.
  Counted as build 1, said here.
- build 2 (`DishonoredGame_IN.exe`): the fix (`USkeletalMeshComponent::UpdateBounds`, retail whole) on top of the probe.

## 1. The probe table (build 1, `CC_IN1b.log`, world 158.0, point 10327 / 23053 / 2415, r 300)

114 primitive components come within 300 uu (every live `UPrimitiveComponent`, not only actors' `AllComponents`); 9 of them are in
the collision octree's box query. The rows that matter (the rest are attention / spawner / audio / particle components with no
collision):

| # | component (owner, level) | source | octree | collision data | own LineCheck straight down | reason it does not answer |
|---|---|---|---|---|---|---|
| 36 | `SkeletalMeshComponent0` (`SkeletalMeshActorMAT_2`, L_Tower_Script) = **the Empress boat** (`EmpressBoat_anim`, PA `EmpressBoat_anim_PA`: one body on `Boat_Jnt`, 13 boxes) | Components / AllComponents | in (1 node) | collide 1 block 1, mask np/npc/pl 1/1/1, ShouldCollide 1 | **HIT z 2418.5** (zero extent), **HIT z 2449.4** (NPC extent) | **component Bounds X 9684..10290** (origin 9986.9, extent 302.9 / 138.8 / 218.4) - the point is 37 uu outside, so the octree's `LINE_BOX` against `Bounds` drops it before its LineCheck runs |
| 6 | `StaticMeshActor_SMC_3482` (`StaticMeshCollectionActor_4`, L_Tower_Env), `walls_01.floor_waterlock_01` | Components / AllComponents / SMCA | in (174 nodes) | kDOP 19 nodes / 42 tris, UseSimple* 0, 4 box elems | miss | the floor has its pit hole there: kDOP leaves cover X >= 10750 only for Y 22500..23600 (read from the package, `build/agentIN/pkgkdop.py`) |
| 35 | `StaticMeshComponent0` (`InterpActor_3`, based on the boat), `EmpressBoat.empress_boat_Model` - the rendered boat incl. its bow steps | Components / AllComponents | out | **CollideActors 0** (cooked), no BodySetup, kDOP 1551 / 2780, UseSimple* 1 | miss (complex: HIT 2416.4) | data: the render mesh does not collide in retail either |
| 14, 15 | `DisStaticCollisionActor_0/_1` (L_Tower_Block), `npcmovementcollision_model` | Components / AllComponents | in | NPC-only mask (cooked) | miss | not under the point (X 10265 / 10535, Y 22709 +-201) |
| 34, 37 | `InterpActor_24` / `_6` water planes | | out | CollideActors 0 | miss | data |
| 114 | `DishonoredWaterVolume_0` brush | | in | | HIT 2285 | a volume, not blocking |

So the brief's candidate list (SMCA registration, `FStaticMeshCollection` serialisation, a skipped kDOP, flags cleared at load, GG's
flush) is all clean here: the SMCA members are attached, in `Components` and `AllComponents`, in the octree with their kDOP data.
The primitive under the stair foot is the **boat's PhysicsAsset**; its own trace hits; only its bounds are wrong.

## 2. Why the bounds are wrong, and the retail difference

`EmpressBoat_anim`'s mesh bounds are extent 113.5 / 218.4 / 294.1 (read from the package) - the component's 302.9 / 138.8 / 218.4 is
exactly those rotated. Our `UpdateBounds` (the reference engine's) therefore took the mesh-bounds fallback, while the PhysicsAsset
spans ~2166 uu along the boat. The reference reaches the PA branch only if the component is "visible" (`DetailMode`, hidden flags),
not `bComponentUseFixedSkelBounds` and `bHasValidBodies`; one of those failed for the matinee skeleton (RefSkeleton 4 = SpaceBases 4,
so the scale/skeleton test was not it).

Retail `USkeletalMeshComponent::UpdateBounds` (2012 `0x364740`, 2013 rva `0x347a30` / VA `0x747a30`, both 1610 B, same shape) has
none of those three gates:
1. Arkane `m_UserBounds` (`USkeletalMesh` @56: bone, offset, radius): if the bone matches and not `bConsiderAllBodiesForBounds`
   (bit 2 @760), a cube of `m_fRadius x max|draw scale|` around the bone-transformed offset;
2. else the PhysicsAsset AABB whenever the draw scale is uniform and `SpaceBases.Num() == RefSkeleton.Num()`;
3. else the parent's PhysicsAsset under the same test (no `bUseBoundsFromParentAnimComponent` shortcut);
4. else the mesh bounds moved by `RootBoneTranslation`; then the per-poly kDOP boxes and `BoundsScale`. No cloth / APEX bounds.

Ported whole (`Engine/Src/UnSkeletalComponent.cpp`). Deviations: none in behaviour; the reference's stat timers / bounds logging
lines are dropped with the branches they sat in. `UPhysicsAsset::CalcAABB` (2012 `0x3d9bd0`, 2013 rva `0x3b8210`) was compared and
left as is (retail splits it into `CalcAllBodiesAABB` / `CalcBodiesAABB`, same result).

After the fix (build 2, `CC_IN2b.log`): the boat's bounds are X 8989..11168 (origin 10078.7, extent 1089.7 / 223.2 / 213.9). The
hidden drawbridge `SkeletalMeshActor_2` (`drawbridge_small_01_PA`) had the same fault and now collides too.

## 3. Results (build 2)

| check | result |
|---|---|
| floor under 10327/23053 for the NPC | the captain's floor sweep reports `walkable floor SkeletalMeshActorMAT_2` at 10308 / 23055 (153.75), 10349 / 23046 (154.02), 10381 / 23023 (154.27); no `no floor` line, never PHYS_Falling (`CC_IN2c.log`). Before (IK4c): `no floor within 3 x MaxStepHeight` at 10324 / 23054, world 153.68, fall into the slip |
| captain reaches the dock stairs within 25 s of world 150.6 | **met**: at the stair foot / bow at 153.9-154.5 (ours w0154 shows him at the foot of the steps, retail w0154 on them), then turns onto the lowered drawbridge (`SkeletalMeshActor_2`, 154.8-157.5) and stands on the dock floor (`SMC_3482`, Z 2500 = 2400 + height) at 157.76; walks on to 10948 / 22269 by 161.8. 1694 uu path, FullMovePawn 370 moves asked 1697.5 got 1697.5, 0 blocked |
| retail NPC flags | the hits above are the locomotion's own floor sweep with retail's `0x100030DF` |
| d3d9 scene census | unchanged against IK4's gate: draw elements 6506 (6506), visible prims 211 (209), draws/frame 555 (550) |
| physics | `physics_actors` 1350 (IK4 1350 / 1351), `physics_static_shapes` 1312 (1312): unchanged - the fix is in the octree's bounds, not in PhysX |
| gate `run_regression.py --only d3d9,inputtest,newgame` (`build/agentIN/gate1.txt`) | **23 ok, 0 failed**; criticals 0/0/0, `unported_natives` 0, `probe_natives` 3, `d3d9_frames` 7110, `inputtest_moved` 1000.1, `newgame_on_mission` 120, `newgame_pawns` 29 |

The player ("blocked next to the boat socket", IF): `-indiag`'s eight 80 uu player sweeps at 158 (build 1) are free forward and hit
only the boat PA's side boxes to the left; with W held after the ride (`-fwdiag`):
- build 1 (`CC_IN1w.log`): the pawn walks the deck, leaves the boat's (too small) bounds at X ~10490 and drops into the slip (PHYS
  Swimming at 162.2) - not blocked, but the same defect as the captain;
- build 2 (`CC_IN2w2.log`): walks the whole boat including the bow steps (Z 2511 -> 2580 on the foredeck box, based on the boat to
  X 10888), then walks off the bow tip into the lock channel ahead (X > 10888, Y 22900, no floor there: the waterlock floor's kDOP
  has no leaf in that channel) - straight-ahead W, as scripted.

## 4. Where the brief was wrong (with the measurement)
1. **"The stair/landing mesh has no collision in our runtime"**: there is no stair/landing static mesh at 10327/23053. The stairs
   are the bow steps of the boat (`empress_boat_Model`, render only, `CollideActors 0` in the cooked package); the floor there is the
   boat's PhysicsAsset, whose own LineCheck hits at Z 2418.5. The fault is the component's bounds (mesh bounds 606 uu instead of the
   PA's ~2180), an engine `UpdateBounds` difference, not level load.
2. **"The census only walked AllComponents"**: StaticMeshCollectionActor members are in `Components` and `AllComponents` in this
   tree (`AStaticMeshCollectionActor::Serialize` moves them there, as retail) - the census missed nothing for that reason; it fired
   at the player's first floor miss instead.
3. **"the navmesh has a poly at Z 2415 where collision existed"**: the navmesh is imported (`DisPylon_2.bImportedMesh`); the poly is
   over the docked boat, not over level geometry - the waterlock floor has its pit hole there.
4. **Candidates (SMCA, kDOP load, flags, `IncrementalUpdateComponents`, GG's flush)**: all measured clean (row 6: attached, octree 174
   nodes, kDOP 19/42). L_Tower's 8th streaming level that stays unloaded is `L_Tower_Water` (`LevelStreamingKismet_0`: particles and a
   water volume), not collision.
5. **IF's player block**: on HEAD the player is not blocked after the ride - he walks the deck and falls off where the boat's bounds
   end (same defect); after the fix he walks the whole boat.
6. Measurement caveat found on the way: a world trace with a NULL source actor never reports movers / non-world-geometry
   (`AActor::ShouldTrace` needs a source for `TRACE_Blocking`), so `INPROBE`'s own world rows read "nothing" even after the fix; the
   evidence is the per-component LineCheck and the pawns' own sweeps. `-inprobeat` takes one value (UE3 `Parse` stops at a comma).
7. Base: main moved to `af990e9` (IH3) during the package; no overlap with these files (`git diff --name-only eb48851 af990e9`).

## 5. Not done / open
- `ADisStaticCollisionActor::PostLoad` (2012 `0x630fc0`, NPC-only trace mask when `bCollideOnlyWithNPCMovement`): unported; the
  cooked dock actors already carry that mask, so it changes nothing here.
- The captain takes the drawbridge, retail.mp4 (ends world ~154.9) shows him on the bow steps at 154; whether retail then takes the
  steps or the drawbridge is not on the clip.
- Exit still ends with IH's Wwise pure-virtual at shutdown (known, IH3/IH4).

## 6. Files
`build/agentIN_sync.py` (base main `af990e9`, worktree diff against `eb48851`; no generated files): `Engine/Src/UnSkeletalComponent.cpp`,
`DishonoredGame/Src/indiag.cpp` (new), `DishonoredGame/Inc/indiag.h` (new), `DishonoredGame/Src/dishonoredviewportclient.cpp`
(Exec hook), `DishonoredGame/Src/dishonoredplayercontroller.cpp` (Tick hook). Tools (`build/agentIN/`): `pkgactors.py`, `pkgsmca.py`,
`pkgbounds.py`, `pkgkdop.py`, `pkgmesh.py`, `pkgagg.py` (cooked-package readers on `read_package_classes.py`), `in_probe_run.sh`,
`in_walk_run.sh`, `in_run.sh`.
