# Agent GB — the boat parks where its keys say; what retail does differently is attach the player to the boat

Wave 23 (`PHASE19.md`), package GB. Worktree `build/agentGB_wt`, base `9f214c5`, with main `785e91d` (FZ, commit `ad42139`) and
then `d96a331` (GC, commit `cc4a699`) merged by `git merge` as the coordinator asked, both without conflicts (the generator was
re-run after the GC merge: byte-identical). Two builds: the working build `build/agentGB_rel` (Release,
`DISHONORED_LAYOUT_CHECKS=ON`, `build/agentGB/gb_build.cmd` -> `build_queued.py`, three passes `gb_build0..2.log`) and the gate's
in-harness build `build/agentGB_reg`. Sync `build/agentGB_sync.py`. Control `build/head_rel`, rebuilt by the coordinator at
`9f214c5` (exe 20:58, the same code as `ad4a35d`; older than main's `785e91d` by FZ only), staged as `DishonoredGame_GBctl.exe`.
No retail `Dishonored.exe` launch; every launch `game_input.py launch` (`-inputcmd`, `-unattended`, `-windowed`, gate-style
`-ENGINEINI/-GAMEINI/-INPUTINI/-UIINI` under `build/agentGB/config/<log>/`); no real input; no FModel. IDA: private copies
`build/agentGB/idb/GB_retail2013.i64` / `GB_shipping2012.i64`; 2012 decompiles in `build/agentGB/decomp2012/`.

## 0. The answer in six lines

1. **The ride boat ends exactly where its cooked keys put it.** `-gbdiag` dumps the `Boat` group's 19-key Move track with every
   key's world position: key 18 (t 91.47) evaluates to **9713.2, 23114.8, 2253** and the boat is there at the matinee's end
   (`gbdiag move ... track says X=9713.248 ... actor SkeletalMeshActorMAT_2 at X=9713.248`). The track is `IMF_RelativeToInitial`
   (`moveFrame 1`); `InitialTM = RelTM(key 0)^-1 * ActorTM` maps key 0 onto the boat's placed transform (-3909, 36717, -394,
   yaw 52728) and the rest follow in that frame. Retail's runtime does the same arithmetic: `GetMoveRefFrame`,
   `ComputeWorldSpaceKeyTransform`, `GetLocationAtTime`, `CalcInitialTransform`, `MoveWithInterpMoveTrack`, `physInterpolating`,
   `FindInterpMoveTrack` and `GetBaseMatrix` were decompiled in both databases (§3) and match this tree's reference bodies
   (one Arkane addition, §3.1). **There is no 750-unit evaluation error.**
2. **The "moored boat 750 units away" is a different boat, and so is the one Boat_Arrive hides.** `SkeletalMeshActorMAT_1`
   (visible, no collision) and `SkeletalMeshActor_2` (hidden, collision) *both* stand at **10305, 22565, 2390, yaw 32768** — the
   dock's dressing boat and its collision twin, swapped at `Boat_Arrive`. Neither is the ride boat; FW's premise that the ride
   boat should park where `SkeletalMeshActor_2` stands has no support in the data (§2).
3. **What retail does and this tree did not: the Attachment track.** The `Boat` group's four `InterpTrackAttachment`s (a comment-only
   skeleton here, `keys 0` in every census) attach the TARGET group's actor to the boat: `PlayerGroup0` (the player pawn) to
   socket **`Player`**, `Guard Captain A` to `Captain` (+94 X), `EliteGuard on Boat` to `Driver`, `Boat_Mover` (`InterpActor_3`) to
   `Boat_Socket`, all hard, at t 0; the three pawns are detached at t 94.01 / 94.18 (`gbdiag attachkey`, §4). So in retail the
   player rides on the boat's `Player` socket and is left there when the ride ends — not where he spawned plus a drift.
4. **Ported, 2012 and 2013 resolved by hand (§5)**: `UInterpTrackAttachment` (10 bodies) and `UInterpTrackInstAttachment` (2), moved
   from the DishonoredGame shim header into the Engine with `FAttachmentInfos`, `FAttachmentControlTrackKey` and
   `KeyAttachmentType` (the generator regenerated the three DishonoredGame files, diffed, copied); `AActor::MAT_Attach` /
   `MAT_Detach` / `DoKismetAttachment`, `APawn::DoKismetAttachment`, `ASkeletalMeshActor::DoKismetAttachment`, the
   `execDoKismetAttachment` call (it consumed its parameters until now), `ADishonoredNPCPawn::MAT_Attach` / `MAT_Detach` (the
   retargeting component is absent — says so once). Measured: the player rides attached (`physics 7`, `relLoc` to the boat
   constant) and ends at **9826.3, 23064.2, 2411.8** facing the gangway steps (`gb2_a`), 60 units further +X and 49 units to
   starboard of where the control leaves him (9759, 23114 on `GBC1`; FW's 9731).
5. **Acceptance not reached: the pawn does not walk at the ride's end on any build of this wave** — not on the control
   (`GBC1`, `head_rel` at `9f214c5`), not on this tree before the port (`GB1`), not after it (`GB2`): `W` held for 14 s leaves
   `speed 0` and the position unchanged, where FW walked 260 units on `da77f0f` five times. The yaw turns, so input reaches the
   controller; the pawn's cylinder trace ahead is blocked by the boat's own `SkeletalMeshComponent0` at 10–58 units (never at 0
   on `GB2`), so it is not a wall at the feet either. This is a regression between `da77f0f` and `9f214c5` (FV's pawn tick /
   GA's floor sweep are the candidates), measured by `-gbdiag playermove` on the gate exe (§6, run `GB3`).
6. Gate: §7. Builds: 0 errors, 0 C4263, 0 C4264 on every pass.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "the ride boat ... ends at 9773, 23112 and the moored collision boat Boat_Arrive swaps in stands at 10305, 22565, 750 units away at the dock ... the Move track is evaluated wrong, or the group is attached to the wrong actor, or a second matinee/Teleport ... does not run" | The boat's actor ends at **9713.2, 23114.8, 2253** (9773 was the pawn's base-relative reading), which is key 18 of its own Move track evaluated through `IMF_RelativeToInitial` from the boat's placed transform — the data, not an error. The group binds `SeqVar_Object_1 = SkeletalMeshActorMAT_2`, the only boat at the start pier (`gbdiag varlink`). `SeqAct_Interp_8/9/14` (the other Move-track matinees named 'Boat'/'Whaleboat') play elsewhere (`_9` and `_14` at 74–79 s on other actors, `_8` never). No Teleport of the player exists in `SOIREE_Boat` (the sequence's exports: 40 `SeqVar_Object`, one `SeqVar_Named`, one `SeqVar_Player`, no `SeqAct_Teleport`, no `SeqAct_AttachToActor`). |
| "`Boat_Arrive`'s `ToggleHidden`/`ChangeCollision` pair is meant to run while the ride boat is already at the dock" | The pair swaps `SkeletalMeshActorMAT_1` (visible, `collide 0 block 0`) for `SkeletalMeshActor_2` (hidden, `block 1`) **at the same place, 10305, 22565, 2390** (`gbdiag actor`), 750 units from where the ride boat's keys end. It is the dock's moored boat getting collision when the player can reach it, not the ride boat's destination. |
| "`SkeletalMeshActorMAT_2`'s ... Move track in the lift matinee `SOIREE_Workers.SeqAct_Interp_3`" (FW §9) | The lift matinee's data is the asset `Tower_BoatLifted` and has **no Move track at all** (groups `SeaLevel`, `Valve`, `Railing`, `Engine`, `panel` have 0 tracks in the cooked package too — `build/agentGB/tower_script_exports.txt`); the rise from Z -942 to 2253 between 126 and 150 s is keys 16–18 of `SOIREE_Boat.SeqAct_Interp_1`'s own Boat track (t 69.9 → 87.5 → 91.5 of a 94.23 s matinee started at 56.04). |
| "FS/FN's `UInterpTrackMove` runtime is the suspect (`GetKeyframePosition`, `InitialTM`, the relative/world `MoveFrame`)" | All read in both databases and matched to the tree (§3). The one thing retail has that the reference does not is a distance cull in `MoveWithInterpMoveTrack` (`fLODDistance` @188: an unseen actor farther than it from the first local player's view target is not moved) — 0 on this track, and the player is on the boat. |
| `Boat_Mover` "group 19 ... tracks 0, actor InterpActor_3" (GA) as a possible mover | `InterpActor_3` is *based on* the boat (`relLoc 0,0,0`, `relRot 65532/49152/16384`) and carries the four look-at `TargetPoint`s (`BoatFront/BoatRear/Babord/Tribord`); the Attachment track re-attaches it to socket `Boat_Socket` at t 0. It moves nothing. |
| the player is "never held ... and W moves him from the first frame of the ride" (FW) | Retail attaches the player pawn to the boat's `Player` socket, hard, for the whole ride (`attachkey 0 t 0.00 type 0 ... bone Player hard 1`) and detaches him at t 94.18. The track class had no bodies here, so the player was carried by basing alone and drifted +46 X along the ride (`GBC1`: the pawn's offset from the boat is 46,-1,159 from t 102 on; FW's five runs ended 28 units further back). |
| FZ's silhouettes: "tall thin stretched dark silhouettes on the matinee-moved boat decks (fz6b/c, fz7b) absent from the control" | Present on **this wave's control** (`head_rel` at `9f214c5`, captures `gbc_a`, `gbc_d`: both guards on the deck stretched to the roof) and on `GB1`/`GB2` alike. They are in main since GA's merge (the guards ride the boat there), not FZ's. |
| the 2012 rva in `EngineClasses.h`'s comment, "`DoKismetAttachment 0x1df6c0 / 0x18ab50`" | 2012 `0x18ab50` is `UTexture2DComposite::UpdateCompositeTexture`; `AActor::DoKismetAttachment` is 2012 **`0x1963f0`** (543 B), 2013 `0x18ab50` (= VA `0x58ab50`). The 2012/2013 pair had been swapped into one column. |

## 2. The dock, as data (`GB1.log`, `GB2.log`, `-gbdiag actor`)

```
SkeletalMeshActorMAT_2  ride boat    -3909.5, 36718.4, -384 yaw 52732 (placed)  ->  9713.2, 23114.8, 2253 yaw 64624 (key 18)   physics 7, collide 1
InterpActor_3           Boat_Mover   based on MAT_2, relLoc 0/0/0, attached 4 (the TargetPoints)
SkeletalMeshActorMAT_1  dock boat    10305, 22565, 2390 yaw 32768 roll 1024, hidden 0, collide 0 block 0      hidden at Boat_Arrive (147.4 s)
SkeletalMeshActor_2     dock boat    10305, 22565, 2390 yaw 32768 roll 1024, hidden 1, block 1                 unhidden + collision at Boat_Arrive
player at the end       control 9759, 23114, 2412 (based, drifted)   |   ported 9826.3, 23064.2, 2411.8 (the 'Player' socket, detached at 94.18)
DisTrigger_8            12573, 23294, 2601   (2750 units +X of the socket; the gangway steps are straight ahead in gb2_a)
```

The Boat track's keys (`gbdiag key`, relative values in the track's own frame, world through `ComputeWorldSpaceKeyTransform`):
key 0 t 0 rel (1057.8, 27395.3, -900) euler Z -140.6° -> world (-3909.5, 36718.4, -384); ... key 16 t 69.94 -> (9132.6, 23060.3,
-942); key 17 t 87.46 -> (9713.2, 23114.8, 2123); key 18 t 91.47 rel (-7154.5, 9982.8, 1737) -> (9713.2, 23114.8, 2253), rot
0/64624/0; `InitialTM` origin (21524.1, 26483.5, 516) yaw 12796. Every `gbdiag move` line through the ride reads `track says ==
actor`. The boat's skeleton has 4 bones (`Root_jnt` ...); `Player`, `Captain`, `Driver`, `Boat_Socket` are **sockets** (no bone of
that name, `gbdiag bones`), which is why `DoKismetAttachment` takes its socket path (`SetBase` with the socket name; the bone
position/rotation copy is the non-socket path only).

## 3. Retail's Move runtime, read in both databases

| function | 2012 rva (imagebase 0) | 2013 VA | B | against the tree |
|---|---|---|---|---|
| `UInterpTrackMove::GetMoveRefFrame` | `0x22d7c0` | `0x615690` | 525 | same: `IMF_World` -> base matrix, `IMF_RelativeToInitial` -> `InitialTM * BaseTM`, scaling removed |
| `UInterpTrackMove::ComputeWorldSpaceKeyTransform` | `0x22d480` | `0x615350` | 822 | same |
| `UInterpTrackMove::GetLocationAtTime` | `0x2333b0` | `0x61c500` | 690 | same (`RotMode` 2 ignore, 1 look-at-group) |
| `UInterpTrackMove::GetKeyTransformAtTime`, `EvalPositionAtTime`, `GetKeyframePosition` | `0x232f70`, `0x232360`, `0x231bf0` | `0x61c0c0`, `0x61b4b0`, `0x61ad40` | 1082, 1701, 929 | reference bodies (LookupTrack honoured; none of the 19 keys looks up) |
| `UInterpTrackInstMove::CalcInitialTransform` | `0x24e150` | `0x62acb0` | 2066 | same, plus Arkane's stage-mark start for `UInterpGroupInstAI` (cylinder height + mesh translation) and `UInterpGroupInstPlayer`; no `bUseRawActorTMforRelativeToInitial` (a reference-only member, a shim static here, 0) |
| `AActor::MoveWithInterpMoveTrack` | `0x2d3ec0` | `0x6b8a10` | 1613 | same, plus the `fLODDistance` cull (§1) |
| `AActor::physInterpolating`, `FindInterpMoveTrack`, `GetBaseMatrix` | `0x2c09b0`, `0x25a600`, `0x22aab0` | `0x6a5c90`, unnamed, `0x612bc0` | 191, 909, 207 | same (`FindInterpMoveTrack` keeps only tracks with `bIsMoveControlTrack` @120 bit 7 and not `bDisableTrack` bit 4 — FS's bit order) |
| `UInterpGroupInst::InitGroupInst` | `0x231210` | unnamed | 424 | same, plus `BackupCollisionType = Actor->CollisionType` and `if( Group->bBecomeDynamic ) Actor->MAT_SetPhysics( FALSE, CollisionType )` — not ported; no group of the ride has the bit (`gbdiag attach ... becomeDynamic 0` on the gate exe) |

### 3.1 Not ported, read

`MoveWithInterpMoveTrack`'s `fLODDistance` cull; `InitGroupInst`'s `bBecomeDynamic` -> `MAT_SetPhysics`;
`ASkeletalMeshActorMAT::MAT_SetPhysics` (2012 `0x3317c0`, 302 B: collision type, physics-asset instance, `PHYS_RigidBody`,
two mesh vcalls) — `AActor::MAT_SetPhysics` is declared empty so the Attachment track's detach path has its call.

## 4. The Attachment track, as data (`GB1.log`, `-gbdiag attach` / `attachkey`)

```
SOIREE_Boat.SeqAct_Interp_1 'Boat' -> 'Boat_Mover'          InterpActor_3        key 0 t 0.00  attach  socket Boat_Socket  hard, offset 0, rot 0
                                   -> 'Guard Captain A'     DishonoredNPCPawn_26 key 0 t 0.05  attach  Captain  offset 94,0,0 rot 16384/0/0 hard;  key 1 t 94.01 detach
                                   -> 'EliteGuard on Boat'  DishonoredNPCPawn_27 key 0 t 0.00  attach  Driver   offset 94,0,0 rot 49152/0/0 hard;  key 1 t 94.18 detach
                                   -> 'PlayerGroup0'        DishonoredPlayerPawn key 0 t 0.00  attach  Player   offset 0 rot 0 hard;              key 1 t 94.18 detach
```

`UInterpTrackAttachment::UpdateTrack` (2012 `0x53e560`, 688 B; 2013 `0x8ff2b0`, vtable slot 79, 0.618 — the 2013 body adds
`sub_61FCB0( GroupInst )`, FS's transform backup, after each attach/detach): for the target group's actor, forwards it applies
every key whose time was crossed (`Last < KeyTime <= New`, or all three zero at the first update); backwards it re-applies the key
before each crossed key and detaches at the first. `m_KeyType` 0: `Target->MAT_Attach( GroupActor, Props->m_AttachmentInfo )`;
1: `Target->MAT_Detach()` then `MAT_SetPhysics( TRUE, GroupInst->BackupCollisionType )` when the target group is `bBecomeDynamic`.
`AActor::MAT_Attach` is `InBase->DoKismetAttachment( this, Info )`; `MAT_Detach` is `SetBase( NULL ) + SetHardAttach( FALSE )`.
`ASkeletalMeshActor::DoKismetAttachment` (2012 `0x358630` / 2013 `0x734ee0`, 659 B): with a bone or socket of that name the
attachment loses collision, takes the hard flag, is placed on the bone (not for a socket), based on the mesh at the name, offset
by the key's relative transform, and gets its collision back; otherwise `AActor::DoKismetAttachment` (2012 `0x1963f0` / 2013
`0x58ab50`): unbase, hard flag, the relative offset/rotation in the base's frame, base. `APawn`'s (2012 `0x2bc1f0` / 2013
`0x6a1180`) is the skeletal one on `Mesh` plus `setPhysics( PHYS_None )`. `ADishonoredNPCPawn::MAT_Attach` (2012 `0x7ab5e0` / 2013
`0xb4b080`) does **not** base the guard: it starts `FDisComponentRetargeting` on the socket and clears the base; the component
has no body here, so the guard is only unbased (once, warned) and GA's floor sweep re-bases him. `MAT_Detach` (`0x7ab690`) is the
retargeting stop with `m_bDisableCollisionForSoiree` saved around it — nothing without the component.

## 5. Addresses, resolved by hand

2012 (`GB_shipping2012.i64`, headless `ida_q.py names/dec`): everything in §3, §4 and: `AActor::MAT_Attach 0x172d50` (45),
`MAT_Detach 0x172d80` (83), `SetHardAttach 0x172c40` (258), `DoKismetAttachment` AActor `0x1963f0` (543) / APawn `0x2bc1f0` (731) /
ASkeletalMeshActor `0x358630` (659) / AKAsset `0x171150` / ADisWatchTower `0x66c4a0` / ADisRiverKrust `0x695640`;
`UInterpTrackAttachment::GetKeyProperties 0x546cb0` (300), `AddKeyframe 0x546de0` (380), `GetTimeRange 0x53e2a0` (168),
`GetTrackEndTime 0x53e350` (106), `GetKeyframeTime 0x53e3c0` (91), `DuplicateKeyframe 0x53e420` (309), `SetKeyframeTime 0x541b10`
(386), `RemoveKeyframe 0x541ca0` (36), `GetClosestSnapPosition 0x541cd0` (301), `UpdateTrack 0x53e560` (688, vtable
`??_7UInterpTrackAttachment@@6BUObject@@@` slot 78); `UInterpTrackInstAttachment::InitTrackInst 0x53baf0` (30), `TermTrackInst
0x53ee40` (72), `LoadData 0x53bf00`, `SaveData 0x53ee90` (not ported); `ADishonoredNPCPawn::MAT_Attach 0x7ab5e0` (163), `MAT_Detach
0x7ab690` (65); `AActor` vtable: slot 195 `MAT_SetPhysics` (empty), 199 `MAT_Attach`, 200 `MAT_Detach`.
2013 (`GB_retail2013.i64`, MCP): `CalcInitialTransform 0x62acb0` (0.991 to 2012), `MoveWithInterpMoveTrack 0x6b8a10` (0.991),
`GetMoveRefFrame 0x615690`, `GetLocationAtTime 0x61c500`, `physInterpolating 0x6a5c90`, `GetKeyframePosition 0x61ad40`,
`EvalPositionAtTime 0x61b4b0`, `GetBaseMatrix 0x612bc0`, `ComputeWorldSpaceKeyTransform 0x615350`, `GetKeyTransformAtTime
0x61c0c0`; `DoKismetAttachment` AActor `0x58ab50` (0.980) / APawn `0x6a1180` / ASkeletalMeshActor `0x734ee0` (0.991);
`SetHardAttach 0x568cd0`, `MAT_Attach 0x568de0`, `MAT_Detach 0x568e10`; `UInterpTrackAttachment::GetKeyProperties 0x9068c0`,
`DuplicateKeyframe 0x8ff170`, `GetKeyframeTime 0x8ff110`, `GetTimeRange 0x8feff0`, `GetTrackEndTime 0x8ff0a0`,
`GetClosestSnapPosition 0x902e30`, **`UpdateTrack 0x8ff2b0`** (unnamed; the vtable at `0x1098880`, slot 79 — 2013 has one more
virtual at slot 75 than 2012's 78), `UInterpTrackInstAttachment::InitTrackInst 0x8fc950`, `TermTrackInst 0x8ffb40`;
`ADishonoredNPCPawn::MAT_Attach 0xb4b080`. The brief's "2013 moved things": the `DoKismetAttachment` comment in
`EngineClasses.h` had the 2012 column wrong (§1).

## 6. The runs

All through `build/agentGB/gb_run.py` (`game_input.py launch` + the step file; common switches `-finewgame=50 -fosoiree -fkdiag
-frdiag -fwdiag -fwnear -fwkismettrace -diskismet`, `-gbdiag` on this tree). Logs `DishonoredGame/Logs/<run>.log`, captures
`build/agentGB/captures/<run>/` (BMP + PNG), 0 `Critical` in every run, no `execPlayDying`.

| run | exe | script | result |
|---|---|---|---|
| GBC1 | control `head_rel` (`9f214c5`) | `gb_ctl_steps.txt` (FW4's six headings, px x1.905) | window 1024x745 this time; boat ends 9713.2, 23114.8, 2253; pawn at **9759.4, 23113.8** from t 148 on, `speed 0` through all six W presses; after the first turn every `fwdiag ahead` reads `blocked at 0` by the boat's own hull. `gbc_a`..`gbc_f` |
| GB1 | this tree, diagnostics only | the same | window 1920x1080; the Boat track's 19 keys and the four attachment tracks dumped (§2, §4); pawn 9766.8, 23113.1, `speed 0`, `blocked at 0` 14x after the turn. `gb1_a`..`gb1_f` |
| GB2 | this tree, the port | `gb2_steps.txt` (two turns under 1000 px to +X, W 14 s, two more headings) | the player attached at 56.5 s (`physics 7`, `relLoc 153.8,-40.5,-117` to the boat for the whole ride), detached at 150.5 s at **9826.3, 23064.2, 2411.8** facing the gangway (`gb2_a`); `speed 0` through W, `blocked at 10..27` ahead, never 0. `gb2_a`..`gb2_f` |
| GB3 | the gate exe | `gb2_steps.txt` | §6.1 |

### 6.1 The walk regression, named (read against the symptom; GB3 on the gate exe pending)

**`speed 0` with the view turning is UE3's root-motion velocity on the player.** `GA3.log` (`-gadiag pawn DishonoredPlayerPawn`)
reads `rmm 2/0` (RootMotionMode RMM_Ignore, RotationMode RMRM_Ignore) from spawn to the ride, then **`rmm 1/1` (RMM_Velocity /
RMRM_RotateActor) from the ride's first second (t 56) to the end of the log, with `rmd 0.00`** on every line. `UnPhysic.cpp:1334`
(`APawn::physWalking`): `bDoRootMotionVelocity = Mesh->RootMotionMode == RMM_Velocity && Mesh->PreviousRMM != RMM_Ignore &&
!bForceRegularVelocity` -> the pawn's Velocity is the (zero) root-motion delta over the frame, whatever Acceleration the
controller gives. The input reaches the controller (the yaw turns on every run); the hull trace is a red herring.

**Who writes it**: the player group's StretchAnimControl key (`rootMotion 0`) -> `UInterpGroupInst::SetActorRootMotion` ->
`ADishonoredPawn::MAT_SetRootMotionMode` default branch -> `MAT_StopRootMotion` -> `StopRootMotionMode()` ->
`ChangeAnimStateMotionModes( RMM_Velocity, RMRM_RotateActor )` (GA's port of 2012 `0x7910e0` / `0x78e500` / `0x78e4c0`, faithful
for `ADishonoredPawn`). On `da77f0f` (FW walked) `MAT_StopRootMotion` wrote `RMM_Velocity` only when `MAT_RootMotionEnabled()`
(mode 0 or 3) - false for a player at `RMM_Ignore` - so the player kept `2/0`; GA made the stop unconditional (retail's shape)
and the player got `1/1`.

**What retail has that this tree does not**: `ADishonoredPlayerPawn::StopRootMotionMode` (2012 `0x6facb0`, 15 B, protected
virtual, **`ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore )`**) - the PLAYER's stop goes back to Ignore/Ignore, the NPC's to
Velocity - and `ADishonoredPlayerPawn::ChangeAnimStateMotionModes` (2012 `0x6fae70`, 222 B: `RootMotionRotationMode` written,
a non-zero mode goes to `Mesh->PendingRMM` with the physics swap `m_NextPhysicsMode` restored, `RMM_Translate` parks the pawn in
`PHYS_None` with the physics remembered). Neither is in the tree (`dishonoredplayerpawn_action.cpp` names both in its PDB list;
`CppText/ADishonoredPawn.h:109` notes the override and does not declare it on the player). 2013, resolved through the player
vtable's neighbours (`AnimState_SyncEnd` / `ShouldDropItemsOnAnimNotify`, `CanShowScene` / `HandleBendtimeTouch`; 2013 has two
more slots before `AnimState_SyncEnd`): **`StopRootMotionMode` = `0xaa1af0`** (unnamed, 15 B: vcall +1388 with (2, 0), ratio
0.833 to 2012), **`ChangeAnimStateMotionModes` = `0xaab1d0`** (unnamed; the 2013 body also stops an active Blink power in the
`RMM_Translate` branch); `StartRootMotionMode` `0xb49180`, `PreBeginPlay_NativeComponents` `0xb55a10`.

**The one-line fix** (not built - the coordinator's ask): `void ADishonoredPlayerPawn::StopRootMotionMode() { ChangeAnimStateMotionModes(
RMM_Ignore, RMRM_Ignore ); }` declared `virtual` in `Inc/CppText/ADishonoredPlayerPawn.h` (the base's is virtual in
`CppText/ADishonoredPawn.h:125`); the faithful second line is the player's `ChangeAnimStateMotionModes` (PendingRMM + physics
swap) in place of the base's direct mesh write. FZ's soiree EXIT at 161.6 s and my detach at 94.18 s are not the cause: the
mode is set at 56 s and never restored on any build; `GB2` leaves the pawn `physics 1`, `base SkeletalMeshActorMAT_2`,
`bHardAttach` cleared by `MAT_Detach` (`fwdiag player` reads `physics 1` from 150.55 s). GB3 (`-gbdiag playermove` + `-gadiag`)
on the gate exe is the confirmation: Acceleration non-zero, Velocity zero, `rmm 1/1`.

## 7. Acceptance

The gate `python resources	ools
un_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGB_reg --newgame-at 50
--game-extra-args=-windowed` from the worktree's copy, on the merged tree (`cc4a699`): **queued behind other packages' gates at
hand-back, not finished** (`build/agentGB/gb_gate.log`, `build/agentGB_reg/regression/summary.txt` when done). A first gate on the
pre-GC tree was killed in its queued build stage to take the GC merge first. The working build's three passes: `gb_build0.log`
(1016 steps, 0 errors), `gb_build1.log` (2 errors: include order in the two new units, fixed), `gb_build2.log` (0 errors, 0 C4263,
0 C4264). `build/agentGB_sync.py`: 19 files, **0 flagged** against main `ff5de25` (base `d96a331`); the list self-checked against
`git diff --name-only d96a331` + `ls-files --others` (`NOT IN SYNC LIST` empty). Rules kept: no retail launch, every launch
through `game_input.py` with `-windowed` / `-inputcmd` / `-unattended` and the gate-style ini switches, no real input, no FModel,
two build directories, every address resolved by hand in both databases (§5), the generator re-run for the DishonoredGame files.

## 8. Files

See `build/agentGB_sync.py`'s header: every file and function. Engine headers hand-edited (generated files the generator never
writes): `EngineClasses.h` (`FAttachmentInfos`, four `AActor` virtuals), `EnginePawnClasses.h`, `EngineAnimClasses.h`,
`EngineInterpolationClasses.h` (the three classes, the enum, the registrants, the `VERIFY_*`). DishonoredGame generated files
regenerated by `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentGB/gen` and the
three that differed copied in (`DishonoredGameEngineShims.h`, `DishonoredGameLayouts.h`, `DishonoredGameRegistrants.cpp` — the
moved shims dropped, nothing else changed). `Inc/CppText/ADishonoredNPCPawn.h` appended. New: `gbdiag.h/.cpp`.

## 9. What is left, specified

1. **The walk at the ride's end** (§0.5, §6.1): named - the player is left at `RootMotionMode RMM_Velocity` by the base
   `StopRootMotionMode`; port `ADishonoredPlayerPawn::StopRootMotionMode` (2012 `0x6facb0`: Ignore/Ignore) and
   `ChangeAnimStateMotionModes` (`0x6fae70`), then run `gb2_steps.txt` with `-gbdiag -gadiag` (GB3) for the walk off the socket.
2. From the `Player` socket the gangway steps are straight ahead (`gb2_a`); `DisTrigger_8` is 2750 units +X. Once the pawn walks,
   FW's chain from there is measured working (FW §4a).
3. `FDisComponentRetargeting` for the guards' `MAT_Attach` (retail carries them by retargeting onto `Captain` / `Driver`, not by
   basing); `ASkeletalMeshActorMAT::MAT_SetPhysics` and `InitGroupInst`'s `bBecomeDynamic`; `fLODDistance` in
   `MoveWithInterpMoveTrack`; `UInterpTrackInstAttachment::LoadData` / `SaveData` (the save layer).
4. The stretched guard silhouettes on the deck (`gbc_a`, `gb1_a`, `gb2_a`) are in main since GA; FZ's "absent from the control"
   was a `da77f0f` control.
