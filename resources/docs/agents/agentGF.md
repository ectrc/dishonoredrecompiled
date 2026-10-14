# Agent GF — FZ's residue: the arms' camera pass and mesh component are ported; nothing can be equipped in this tree; the silhouettes are the matinee-driven animations' poses, not the player side

Wave 23 (`PHASE19.md`), package GF. Worktree `build/agentGF_wt`, base `785e91d` (main after FZ), main `17d90a1`, `d96a331` (GC),
`ff5de25`, `f7bc754` (GB) and `cc6b0e7` (GE) merged by `git merge` before hand-back (the last with the two generator-owned files
in conflict, resolved by regeneration; the others clean). Working build `build/agentGF_rel` (Release,
`DISHONORED_LAYOUT_CHECKS=ON`, `build/agentGF/gf_build.cmd` through `build_queued.py`; three passes `gf_build0..2.log` -
the first two stopped on compile errors in the new units, two then one, the third is clean: 0 errors, 0 C4263, 0 C4264);
the gate's in-harness build is `build/agentGF_reg` (`gf_gate.cmd`). Sync `build/agentGF_sync.py` (26 files, every function
named). The control is `build/head_rel` at `9f214c5` (= `ad4a35d`'s code: FV and GA merged, FZ not), staged as
`DishonoredGame_GFbase.exe`; FZ's final exe (`785e91d`'s code) is `DishonoredGame_rg_leDishonored_Recompile_build_agentFZ_reg.exe`;
this package's `DishonoredGame_GF.exe`. Every launch `game_input.py launch` with `-windowed`, no real input, no retail
`Dishonored.exe`.

## 0. The answers, in nine lines

1. **Nothing can be equipped in this tree, so the user's "arms when I shouldn't" cannot be reproduced on any harness
   route.** The equip path - `UDishonoredInventory::InitInventory` (2012 `0x85b790`), `SpawnAndAddInventoryItem`,
   `AddInventoryItem`, `DoEquip`, `EquipItemType`, `ShowInventoryItem`, `ADishonoredPlayerPawn::EquipItemByType` - has no
   body at all; the inventory unit holds `GetEquippedItem` and the ammo / elixir / abstract-item accessors only (§2). Retail's
   New Game route equips nothing either (Corvo has empty hands until the sewer), so the premise "equip the sword before the
   ride" is wrong twice: there is no route, and there is no code to take it.
2. **What hides the arms with an item equipped in retail is not `SetBodyMode`.** During a soirée the player mesh goes
   FULL_BODY at LOD 2 in the world depth group (FZ, re-measured: `body mode 0 -> 1` at 65.19 s, `1 -> 0` at 160.06 s in `GF1`),
   and its pose stays the first-person tree's: the player group `PlayerGroup0` carries one stretch-anim track (`CamShake`,
   weight 0.13-0.18 for twenty seconds of the ride, §4.2), so the `PlayerStack` channels are `None` for the rest of it and the
   base tree (`DisAnimNodeBlendPerBone` at 1.00 - the hands' stance) is what the LOD-2 body plays. With a sword that stance
   holds the sword up in front of the camera bone. Retail's `EnterSoireeMode` (2013 `0x666ff0`) tells every equipped item's
   contexts a soirée started (`UDisItemContext` slot 74, 2013 `0x816d90`) and `MAT_SetForceInventory` re-equips
   `UDishonoredItemEmpty`; both need the item contexts and the equip path, neither of which exists. Named, not ported (§2).
3. **The arms' camera pass is ported** (§3): `ADishonoredPlayerCamera::UpdateViewTarget` now calls the controller's
   `OnCameraUpdate` (2012 `0x6fa840`) after the placement, which runs `ADishonoredPawn::OnCameraUpdate` (`0x7aa7a0`) ->
   `ADishonoredPlayerPawn::OnCameraUpdate_Derived` (`0x71e3b0` / 2013 `0x6abdd0`: the arm offset, the three single-bone skel
   controls, the arms' FOV) and on pass 1 the inventory's `OnCameraUpdate` (`0x85a730`) over every ticking item.
4. **`UDishonoredPlayerSkeletalComponent` is ported whole** (19 of 19 PDB functions: `SetOffset` / `SetFOV` / `DisableFOV` with
   their render commands, `UpdateBounds`, `CreateSceneProxy`, `UpdateChildComponents`, `UpdateChildForTickEnd_Derived`, and
   `FDishonoredPlayerSkeletalSceneProxy` whose `GetWorldMatrices` re-projects the arms at their own FOV); the component
   tick-end phase (`UDisSkeletalMeshComponent::TickEnd` / `UpdateChildForTickEnd`) is in, on a hand-added
   `USkeletalMeshComponent::TickEnd` virtual (§3.2).
5. **`Tick_Begin` is ported** (`0x715a00` / 2013 `0x6b0850`) and `ADishonoredPawn::Tick` dispatches it in retail's place;
   `Tick_PlayerAction` is reached from it as in retail. Still stand-ins, named: `Tick_Mantle`, the deferred corpse drop, the
   rest of `ADishonoredPlayerPawn::Tick` (breathlessness, dispersion, crouch events, stats, avoidance, mana, trackers,
   drowning, tattoo glow), `PreBeginPlay_AnimStates` / `PreBeginPlay_PlayerCombat` (§5).
6. **The silhouettes are not the player side and were never FZ's.** They are on the control at `9f214c5` (`gfc1b`, §4.1): FZ
   compared against `head_rel` at `da77f0f`, before FV/GA made the guards ride and the matinee drive their animations. Every
   stretched pawn is a pawn whose active `MatineeStackNode` channel is a `Tall_SC_Tower` cinematic sequence (or
   `NPCPawn_20`'s `Empty_Distraction_Scavenge02_Loop`), and the stretch is in the posed skeleton itself: the component-space
   height of the pose is 148 units for every NPC standing in `Empty_Idle` and 155-341 for the matinee-driven ones (§4.3).
   Mesh scale, actor scale, mesh translation, base transform, depth group and LOD are all nominal. It is the animation
   evaluation of those sequences (78 Edge joints, 0 constant + 1 animated translation channel, on a 118-joint skeleton)
   through the anim stack - GC's area (`UArkAnimNodeStack::GetBoneAtoms`, FT's stand-in, and `FEdgeAnimSequencePose`).
   Handed over with the numbers (§4.3); not fixed here.
7. **GC named the silhouette while this package ran, and the coordinator handed the fix here**: the two
   `UDisSkelControl_FootPlacement` controls on every NPC (strength 1.00) ran the *reference* `USkelControlFootPlacement` limb IK
   because the retail class's unit was comment-only, and under a mesh floor the IK's trace reads the riverbed. The faithful port
   is out of reach of the one remaining build (28 functions, 30 KB: `CalculateNewBoneTransformsOriginal` 7,302 B,
   `UpdateFootOrientation` 6,565 B ...), so the measurement the coordinator allowed instead is taken the retail way round:
   the unit is off the exclude list with the real entry `CalculateNewBoneTransforms` (2012 `0x6ed860`: the bone and the
   component recorded, the two IK bodies named and not called), so the control stays at its cooked strength and outputs no
   transform - the reference IK no longer runs on it. **Measured on the gate build (`GF2`, §4.4): the silhouettes stay.** The
   override runs (its notice fires at 106.78 s), the guards' posed height still goes 47 (T-pose) -> 190..331 the second their
   matinee channel starts, and `gf2_a` shows the captain, the elite and a dock figure as tall as before. GC's prediction is
   corrected: the foot-placement control is not the stretch. What the numbers leave is §4.3's reading - the evaluation of the
   `Tall_SC_Tower` sequences (and `NPCPawn_20`'s scavenge loop) on these skeletons. `EnableFootPlacement` and
   `LockFootPlacement` are ported with it (§3.4).
8. **The player walks after the ride again (the coordinator's message from GB)**: GA's base `StopRootMotionMode` put the player
   mesh in `RMM_Velocity` at the ride's first second (`rmm 1/1`, `rmd 0.00`), so `APawn::physWalking` took the velocity from
   the zero root-motion delta for the rest of the level. Retail's player overrides both: `StopRootMotionMode` 2012 `0x6facb0`
   / 2013 `0x6a1af0` = `ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore )`, and `ChangeAnimStateMotionModes` 2012 `0x6fae70` /
   2013 `0x6ab1d0` (the pending mode on the mesh, the `m_NextPhysicsMode` physics swap around `RMM_Translate`). Both ported
   (§3.5); the proof is `GF2`'s `rmm 2/0` after the ride and the pawn moving off the boat on `W` (§4.4).
9. **`-gfdiag`** (§7): the arms state once a second and, for every pawn, the posed skeleton against the reference one.
10. Premises corrected: §1. Gate: §10.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "Equip the sword/hand before the ride (`-fzdiag` + the inventory path)" | there is no inventory path: of `UDishonoredInventory`'s 60 PDB functions the tree has the accessors only; `InitInventory`, `SpawnAndAddInventoryItem`, `AddInventoryItem`, `DoEquip`, `EquipItemType`, `ShowInventoryItem`, `SetReequipItem`, `EquipItemByType_KeepTrying` and the player's `EquipItemByType` are comments (`agentGF_status.csv`). Nothing creates `m_pInventory`'s slots; `HasNothingEquipped` is TRUE for the inventory's emptiness, not for a choice |
| "the user plays with the sword out" (as something this build can show) | not on any route this build has; retail's New Game gives empty hands until the sewer |
| "arms hidden during the ride ... `SetBodyMode`" | `SetBodyMode` only moves the one mesh between depth groups and LODs; the body it shows plays the first-person base tree because the player group has no body animation (`PlayerStack` channels `None` except `CamShake` at 0.13-0.18, §4.2). What would hide a held item is the item contexts' soirée notification and `MAT_SetForceInventory`'s re-equip of `UDishonoredItemEmpty` - the equip path again |
| "the silhouettes ... absent from the control" | present on the control at `9f214c5` (`gfc1b`); FZ's control `FZC1` was `da77f0f`, before the guards rode the boat and before the matinee drove their stack |
| "if the mesh translation, `SetBodyMode`'s depth-group/LOD switch, or the based pawn's transform, fix here" | none of the three: `meshScale 1/1/1/1`, `drawScale 1/1/1/1`, `ltwAxes 1.00/1.00`, `meshT 0/0/0` (or the mesh offset's -11..-17), DPG 1, LOD 0 on every stretched pawn; the stretch is in `SpaceBases` (§4.3) |
| "port `OnCameraUpdate_Derived`'s arm-follow and `SetOffset/SetFOV`" | ported, but they do nothing visible on this route: `m_ArmFollowWeight` is applied to a mesh whose arms play the empty stance, and the arms' FOV equals the view's (`m_MeshSpecificFOVWeight` 0 in the soirée, §3) |
| "`Tick_Begin` / `PreBeginPlay_Actions` dispatch FZ stood in for: port the real ones if the arms path needs them" | the arms path needs neither; `Tick_Begin` is ported anyway (it is where the mesh transform is brought up before anything reads a bone, and where the audio-log dummy actor follows the camera bone); `PreBeginPlay_Actions` stays FZ's partial (the upper / left-arm FSM states are skeletons) |

## 2. The inventory: what exists, what does not

`dishonoredinventory.cpp` (agent AJ and later): `GetEquippedItem`, the abstract items, ammo, elixirs, `ConsumeStatPickup`,
`FindItemByClass` / `HasItemByClass` / `FindEquippableItem`, `GameSave` / `GameLoad`, and now `OnCameraUpdate`.
`dishonoredplayerpawn_inventory.cpp` is a 29-function skeleton; `dishonoreditemempty.cpp`, `dishonoreditemskeletalcomponent.cpp`
and every `disitemcontext_*.cpp` are skeletons. The runtime consequence, measured by `-gfdiag`'s `inventory` field on the gate
build (§4.4): the player has an inventory object with no equipped item in either hand. `MAT_SetForceInventory` (FZ's, logged once)
and `MAT_IsUnequipped` are right for that state.

What retail does to a held item in a soirée (so the next package knows where the arms go): `FDisSoireeControlState::EnterSoireeMode`
2013 `0x666ff0` -> for each hand's equipped item, each context slot's `UDisItemContext` virtual 74 (2013 `0x816d90`) - the
contexts hide / holster; `MAT_SetForceInventory( TRUE )` (2012 `0x6fe570`) -> `SetReequipItem( hand, UDishonoredItemEmpty )`
twice, `DropMovableObject`, `EquipItemByType_KeepTrying( UDishonoredItemEmpty )` twice (2012 `0x7afbe0`:
`EquipItemByType` 2012 `0x70ce90` with the keep-trying flag). Then `HandsAppearEmpty` (2012 `0x7acc50`: both equipped items'
meshes absent or hidden) is what the rest of the player code reads.

## 3. What was ported

### 3.1 The camera pass (`dishonoredplayercamera.cpp`, `dishonoredplayercontroller.cpp`, `dishonoredpawn_camera.cpp`, `dishonoredplayerpawn_camera.cpp`, `dishonoredinventory.cpp`, `dishonoredinventoryitem.cpp`)

Retail's `ADishonoredPlayerCamera::UpdateViewTarget` (2012 `0x7135c0`, +0x899) ends every pass with
`PCOwner->OnCameraUpdate( VT, m_PassCount )`; FZ's port of the influence branch left it out. Added after the camera actor's
placement. `ADishonoredPlayerController::OnCameraUpdate` (2012 `0x6fa840` / 2013 `0x6ba860`, 104 / 155 B - the 2013 body builds
the tracking target from the pawn's X/Y and the view's Z): the pawn's `OnCameraUpdate`, then pass 0 `TickCameraTracking` and
pass 1 `UpdateCrosshair` + `UpdateBlindedEffect` - the three are skeletons (`dishonoredplayercontroller_tracking.cpp`), named
once. `ADishonoredPawn::OnCameraUpdate` (`0x7aa7a0` / `0x749f50`, 54 B, same): the `_Derived` virtual (empty base), then on
pass 1 `UDishonoredInventory::OnCameraUpdate` (`0x85a730` / `0x817c90`, same): every slot's item that `ShouldItemTick`
(`0x853da0` / `0x80c770`: the tweaks' `m_ItemTickMode` against the item's usage and socket) gets
`UDishonoredInventoryItem::OnCameraUpdate` (`0x859da0` / `0x816e30`): `UpdateMeleeExtents` and the active contexts'
`OnCameraUpdate` are skeletons (named), the `_Derived` hook is virtual (empty here; `UDishonoredItemEmpty`'s `0x8a5330` moves a
grabbed movable).

`ADishonoredPlayerPawn::OnCameraUpdate_Derived` (2012 `0x71e3b0`, 2013 `0x6abdd0` by neighbours 0.740 and checked by shape -
the same three `FindSkelControl` off `m_pCameraTweaks` at +148/+156/+164, the same `SetOffset` / `DisableFOV` / `SetFOV`, the
power virtual at +408 where 2012 has +404): the player skeletal component's offset is
`(POV.Location - m_NonAdditive_Pos) * m_ArmFollowWeight`; the right-hand, left-hand and camera `USkelControlSingleBone`s
named by `UDisTweaks_PlayerPawn_Camera::m_LookAtControlName_*` get `BoneRotation` = the view pitch over the non-additive pitch
plus the controller offset pitch, plus `m_ArmFollowOffset_Rot_*` eased by `(1 - cos(w * PI)) / 2`, times
`m_ArmFollowWeight_Rot_*`, minus the pawn's pitch (the camera control takes the larger of the two hands' weights and no offset
rotation); the arms' FOV is `m_fCurFOV_Arms` blended into the view FOV by `m_MeshSpecificFOVWeight` - `DisableFOV` when they
coincide, which on this route they always do (the soirée's weight is 0 and `m_fCurFOV_Arms` is never set by anything ported).
On pass 1 the upper FSM's logical state and the active powers would get the view: `UStatePlayerUpperBase` and the power
components have no bodies - named once.

### 3.2 `UDishonoredPlayerSkeletalComponent` and the tick-end phase (`dishonoredplayerskeletalcomponent.cpp`, `dishonoredpawn_body.cpp`, Engine `UnSkeletalMesh.h` / `UnSkeletalComponent.cpp`)

Every function of the unit, named in both databases at the same size (2013 rva = VA - 0x400000): `SetOffset` `0x71dc90` /
`0x6cdec0`, `SetFOV` `0x71d880` / `0x6cdab0`, `DisableFOV` `0x71dab0` / `0x6cdce0` (each: the member, a render command to the
proxy - `ENQUEUE_UNIQUE_RENDER_COMMAND` here, retail's hand-rolled ring-buffer allocation - then the same on every attached
player skeletal component, which is what the item meshes are: `UDishonoredItemSkeletalComponent` derives from it),
`UpdateBounds` `0x7203d0` / `0x6c8880` (the offset, and on the player pawn the bounds re-centred from the root bone onto the
camera bone), `CreateSceneProxy` `0x71aeb0` / `0x6df220`, `UpdateChildComponents` `0x707580` / `0x6c8360` (the reference body
plus the offset on attachments that are neither skeletal nor static meshes), `UpdateChildForTickEnd_Derived` `0x720360` /
`0x6ce100` (the second camera pass: `m_PassCount = 1; UpdateCamera( 0 ); m_PassCount = 0`).
`FDishonoredPlayerSkeletalSceneProxy` (ctor `0x71a860` / `0x6debd0`, `GetWorldMatrices` `0x71a8d0` / `0x6dec40` - byte-identical
in both): the local-to-world the renderer gets is `LocalToWorld` shifted by the offset and, when the FOV flag is up,
`LTW * View * P(arms FOV, view aspect, near * 0.999) * P(view)^-1 * View^-1` - the arms drawn at their own field of view inside
the world's. The reference `FSkeletalMeshSceneProxy::GetWorldMatrices` virtual is exactly the hook retail overrides.

The tick-end phase: retail's component manager calls `USkeletalMeshComponent::TickEnd` after the component ticked;
`UDisSkeletalMeshComponent::TickEnd` (2012 `0x799d10`) runs `UpdateChildForTickEnd` (`0x794e50` / `0x750f20`: the `_Derived`
hook, then every attached player skeletal component's) for a component that is not slaved to a parent animation component.
The reference engine has no such phase, so **`UnSkeletalMesh.h` gains an empty `virtual void TickEnd( FLOAT )` on
`USkeletalMeshComponent` and `USkeletalMeshComponent::Tick` calls it at its end (`UnSkeletalComponent.cpp`) - two hand edits to
reference-engine files, said so here and in the sync script.** Not ported in `TickEnd`: retail's per-component skip flag (the
dword after `m_pSoulMaterial`, unnamed in the SDK) and `ADishonoredNPCPawn::SyncPhysicsSensor`.

Measured on `GF1`: the pass-1 notice fires at 6.10 s (the chain runs from the first frame), `fz body mode` and the soirée
enter/exit are at the same world times as FZ7 (65.19 / 160.06 s log time), the camera line is FZ's to the unit (`t=96.26 FOV
75.00 loc 1506/24980/-703 rot -26/-6800/0 AnimDriven 1.00`, after the ride `PlayerControl 1.00 rot -26/-12736/0`): the second
pass changes nothing the camera reports, as it should with no time passing.

### 3.3 `Tick_Begin` (`dishonoredpawn.cpp`, `dishonoredplayerpawn.cpp`)

`ADishonoredPawn::Tick` (FV's) now calls the `Tick_Begin` virtual before `Tick_Body`, where retail's `0x790c10` has it (+0x3a,
before `Tick_Water`); the base body `0x745410` (8 B, ICF-folded with `IDisRatTargetInterface::IsRatSocketValid`) returns TRUE.
`ADishonoredPlayerPawn::Tick_Begin` (`0x715a00` / 2013 `0x6b0850`, 269 B both; the 2013 one writes the camera bone into the
object at +2524, which the SDK layout names `m_pAudioLogConvActor`): the base, `Tick_Mantle` (`0x7129d0` / `0x6dbd00`; the
mantle unit is a skeleton - named), `Mesh->ConditionalUpdateTransform( Owner->LocalToWorld() )` so the bone reads of the frame
see the pawn's transform, the camera bone written into the audio-log dummy actor's location and rotation (FR's speaker for the
player's audio logs), `Tick_PlayerAction` through the vtable (FZ's three-FSM tick, which the stand-in `Tick` override used to
call directly, after `Tick_Body` - now before it, as retail), then retail's deferred corpse drop on two transient bits at +2016
(`UStatePlayerCarryCorpseIdle::StartDroppingCorpse` is a skeleton - the bits are left alone). `ADishonoredPlayerPawn::Tick`
(`0x715b10` / `0x6bb1e0`) keeps the base call and `m_fLastDeltaSeconds`; the rest is named in its banner.

### 3.4 The foot-placement control (`disskelcontrol_footplacement.cpp`, after the coordinator's message)

GC (`agentGC.md` §4) measured the stretched Dock Worker B: 110/110 bones, no scale, no suspect blend, two
`DisSkelControl_FootPlacement` controls at strength 1.00 - and `disskelcontrol_footplacement.cpp` on the exclude list, so the
cooked instances ran the reference engine's `USkelControlFootPlacement::CalculateNewBoneTransforms` (a limb IK that traces for
the floor under the foot and pulls the leg to it) over Dishonored's own properties, with GA's finding that the trace under a
skeletal-mesh floor (the boats, the dock) reads the riverbed 1,000 units down. It seemed to match §4.3 (the stretched pawns
stand on meshes, the tower guards on BSP read 148), so the measurement was taken: with the control's entry outputting no
transform (below), `GF2` reads `NPCPawn_26` at `poseH 47.1` in T-pose and `190.1` from the first second of its
`BoatCpt_SC_Tower_Idle` channel, `263.7 ... 331.5` through the ride, and `gf2_a` shows the three silhouettes unchanged. So the
stretch is not the foot-placement IK; it comes with the matinee channel's sequence, as §4.3 reads it. GC's premise corrected,
by the strength-0 measurement the coordinator asked for.

Retail's control: `CalculateNewBoneTransforms` 2012 `0x6ed860` (98 B; 2013 unnamed, reached through the vtable) writes
`m_FootBoneIdx` and `m_pSkelComp`, then `CalculateNewBoneTransformsNewComer` (`0x6e8ed0`, 830 B) when the game info's flag
(+976 bit 0x20) and `m_bCanUseNewIK` are set and `m_bDisableNewIK` is not, else `CalculateNewBoneTransformsOriginal`
(`0x6e4d70`, 7,302 B) - the IK proper, with `UpdateFootOrientation` 6,565 B, `UpdateFootOrientationTarget` 2,674 B,
`UpdatePredictionProperties` 2,308 B, `HandleFootRotationLock` 1,455 B, `DoLineCheck` 823 B, `InitializeFootProperties` 797 B
and nineteen more (every one inventoried with its size in `agentGF_status.csv`). 30 KB of IK, nav-mesh tests and foot
prediction cannot be ported faithfully in the one build this package had left, so: the unit is off the exclude list (through the
generator) with the entry ported as far as its two writes, the two bodies named and not called, `EnableFootPlacement`
(`0x6d01c0`: the two initialised bits cleared on enable, `SetSkelControlActive` or an instant `SetSkelControlStrength`) and
`LockFootPlacement` (`0x6d0230`: the lock count, the broken-lock bit cleared). The control therefore runs at its cooked strength
and outputs no transform, which `USkeletalMeshComponent`'s control pass applies as nothing (`NewBoneTransforms.Num() == 0`) -
the strength-0 measurement without touching the cooked properties, and the shape retail has for a control whose IK declines.
The bit names come from the generated class in declaration order (`m_bDisableNewIK` bit 0, `m_bIsInitialized` 1,
`m_bInitialized` 8, `m_bLockBroken` 10, `m_bCanUseNewIK` 25), which is what retail's `0xFFFFFEFD`, `~0x400` and `0x2000000`
masks read.

### 3.5 The player's root-motion modes (`dishonoredplayerpawn_action.cpp`)

`ADishonoredPlayerPawn::ChangeAnimStateMotionModes` (2012 `0x6fae70`, 222 B; 2013 `0x6ab1d0`, 375 B, unnamed in the 2013
database - matched by the vtable (0.533) and checked by shape: `Mesh+908` is `RootMotionRotationMode`, `Mesh+902` `PendingRMM`,
the pawn's `+2234` `m_NextPhysicsMode`, `setPhysics` at vtable +536): the rotation mode goes to the mesh; a mode other than
`RMM_Translate` is queued in the mesh's `PendingRMM` (the mesh applies it at its next root-motion step) and restores the physics
a `RMM_Translate` parked; `RMM_Translate` remembers the physics in `m_NextPhysicsMode`, parks the pawn in `PHYS_None` and clears
the pending mode. The 2013 body also stops an active `UDishonoredActivePowerComponent_Blink` before parking (vtable +352 then
+368 on the power) - the power components have no bodies here, named once. `StopRootMotionMode` (2012 `0x6facb0`, 15 B,
protected virtual; 2013 `0x6a1af0` by vtable neighbours, GB's resolution) = `ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore )`:
the player leaves root motion by ignoring it where the base pawn (GA's `0x78e4c0`) takes `RMM_Velocity` / `RMRM_RotateActor`.
The declaration lives in `CppText/ADishonoredPlayerPawn.h`; `MAT_StopRootMotion` -> `StopRootMotionMode` (GA's) now reaches
the override through the base's virtual.

## 4. The runs

All through `build/agentGF/gf_run.py` (FZ's `fz_run.py` plus `-gadiag -gfdiag` and, before three of the five shots, a mouse move
from the top of the viewport to the bottom through the `-inputcmd` file - a `MouseY` delta the in-game look reads as pitch
down; the pitch it produced was small, §4.1). Route `-finewgame=50 -fosoiree -fkdiag -frdiag -fzdiag -gadiag -gfdiag -diskismet`,
shots at command-file times 53 / 100 / 103 / 140 / 175 (world 52 / 99 / 103 / 140 / 176), quit at 200. Captures
`Screenshots/Win32Console/apshottime_<tag>[a-e]0000[0-4].bmp`, 1920x1080, 6,220,854 B each, thumbnails
`build/agentGF/shot_<tag>*.png`, all viewed. No `Critical` in any log; no minidump.

| run | exe | log | what it showed |
|---|---|---|---|
| GFC1 | control `DishonoredGame_GFbase.exe` (`head_rel`, `9f214c5`) | `GFC1.log` | **the silhouettes are on the control**: `gfc1b` (world 99) has two stretched figures on the deck and one on the far boat; `gfc1a` black (the fade at 52 s); `gfc1e` the dock interior after the ride, no arms (nothing equipped) |
| GFZ1 | FZ's `785e91d` exe | `GFZ1.log` | the same silhouettes from the camera bone (`gfz1b`, `gfz1c`); the look-down did not pitch the view inside the soirée (the AnimDriven clamp) and pitched it a few degrees outside it (`gfz1e`); no arms |
| GF1 | this package, `DishonoredGame_GF.exe` (working build) | `GF1.log` | the same silhouettes (`gf1b`), the same camera, the pass-1 chain live, the per-pawn skeleton measurement of §4.3; the `gfdiag arms` line was silent (a once-a-second key clash with the pawn report - fixed for the gate build, §4.4) |

GFC1 and GFZ1 ran at the same time (two windowed processes); world times line up to within 0.1 s between them.

### 4.1 The look-down

`move 0.5 0.05` then `move 0.5 0.95` fires a `MouseY` delta of 0.9 of the viewport height (972 px at 1080). Outside the soirée
that pitched the controller down by a few degrees (`gfz1e`, `gfc1e`: the dock floor comes into the lower frame); inside it the
AnimDriven influence's player-look clamp absorbed it (`gfz1c` / `gf1c` are level). It does not reach the arms either way:
nothing equipped, nothing to see. The amount is the mouse sensitivity's; a larger pitch would take several moves.

### 4.2 The player during the ride (GF1, `gfdiag skel DishonoredPlayerPawn`)

`DPG 1 LOD 2/0` from 65 to 160 s, `DPG 2 LOD 1/0` outside; `poseH 27.5 refH 27.5 (x1.00) ... maxLocalRatio 1.00` the whole run:
the player mesh is never stretched. Its tree: `ArkAnimNodeBlendPose PoseBlender [0:1.00 ArkAnimNodeStack]`,
`ArkAnimNodeStack PlayerStack [0:1.00 DisAnimNodeBlendPerBone 1:0.00 None 2:0.00 None 3:0.00 None 4:0.00 None]` for every
sample except 139-147 s (`1:0.13..0.18 CamShake`, the stretch-anim track `fkdiag stretch ... 'PlayerGroup0' slot PlayerStack
ch 0: anim CamShake`, keys at matinee 7.10 and later). So the FULL_BODY mesh of the ride is posed by the first-person base tree
at weight 0.82-1.00; with an item in hand that is the item's stance in front of the camera bone (§0.2).

### 4.3 The silhouettes (GF1, `gfdiag skel <pawn>` once a second per pawn)

The measure: `poseH` = the Z extent of `SpaceBases`' translations (the posed skeleton's standing height in component space),
`refH` the same for the reference skeleton composed through the parent chain (44.9 for the 118-bone guard skeleton, which lies
along another axis in its bind pose - a guard in T-pose before his group starts reads `poseH 47.1`, `NPCPawn_26` at 63 s),
`maxBoneScale` the largest `FBoneAtom` scale, `maxLocalRatio` the largest ratio of a local translation to the reference bone
length (3.0-3.2 on every pawn, stretched or not - a constant of this tree's `LocalAtoms` against the cooked `RefSkeleton`, not
the fault). Every pawn: `drawScale 1.00/1.00/1.00/1.00 meshScale 1.00/1.00/1.00/1.00 ltwAxes 1.00/1.00 maxBoneScale 1.00`,
`DPG 1`, `LOD 0`, `hidden 0`; `meshT` 0 or the mesh offset's -11..-17.

| pawn | base | active stack channels at 100-101 s | `poseH` |
|---|---|---|---|
| `NPCPawn_8..14, 16, 24` (tower guards, `Empty_Idle`) | none / WorldInfo | `MatineeStackNode [0:1.00 ...]`, no channel | **147.8-148.3** |
| `NPCPawn_3` (Dock Worker B, walking, `Empty_Idle`) | WorldInfo | none | 121.5 (110-bone civilian skeleton) |
| `NPCPawn_26` (Guard Captain A, on the ride boat) | `SkeletalMeshActorMAT_2` | `1:1.00 BoatCpt_SC_Tower_Idle 2:1.00 BoatCpt_SC_Tower_LookingBack` (set `Tall_SC_Tower`) | **340.8, 271.9** (47.1 at 63 s in T-pose, 190-249 through the ride, 148 after it) |
| `NPCPawn_27` (EliteGuard on Boat) | `SkeletalMeshActorMAT_2` | `2:1.00 BoatDriver_SC_Tower_Idle 4:1.00 ..LeftArm_Tower_Brake 5:1.00 ..KeepUp 6:0.18-0.91 Gesture_HeadBob_UpDown` | 155.0, 148.2 |
| `NPCPawn_17` (the other boat's captain) | none | `1:1.00 BoatCpt_SC_Tower_Idle` | 247.4, 248.4 |
| `NPCPawn_20` (the other boat's guard) | none | `Empty_Distraction_Scavenge02_Loop` stopped, matinee-positioned (set `Npc_EliteGuard_Distractions_as`) | 167.0, 220.6 |

The `Tall_SC_Tower` sequences: `tracks 78 transBones 0 rotOnly 0 fmt t7 r7 k0 frames 91 edge joints 78 T 0+1` against `skel
joints 118 bones 118` - an Edge animation of 78 joints with one animated translation channel and no constant ones, evaluated
on a 118-joint skeleton; `Empty_Idle` (`Npc_Cityguard_empty_as`, 78 tracks, same formats) on the same skeleton gives 148. So the
stretched pose comes out of the sequence evaluation for these cinematic sequences, before the stack blends them (`NPCPawn_17`
has a single channel at weight 1.00 and is stretched to 247 - with one channel at 1.00 `UArkAnimNodeStack::GetBoneAtoms`'s
`Blend` returns the child's atoms unchanged). Where to look, for GC or whoever owns the evaluator: `FEdgeAnimSequencePose::
GetAnimationPose` (`AnimationEncodingFormat_EdgeAnim.cpp`: the pose evaluated against `USkeletalMesh::m_EdgeSkeleton` and copied
joint i -> bone i), `EdgeAnimEvaluateSkeletonPose` (`EdgeAnimEvaluate.cpp`) for joints the animation has no translation channel
for, and `UArkAnimNodeStack::GetBoneAtoms`'s `m_bDontUseRefPoseOnNonDefinedBones` masking. The retail C++ `GetAnimationPose`
(2012 `0x1a4ac0`, 4032 B, still present beside the Edge job) takes animation translation for every non-root bone unless
`ForceMeshTranslationBoneNames` lists it - retail's `UAnimSet` has no `bAnimRotationOnly` (the SDK dump moved it to the shim
block), so the tree's "translation from the animation" rule is retail's; the difference is in what the Edge decode yields for
those channels.

### 4.4 The gate build's run (`GF2`, GB's walk-off route)

`DishonoredGame_GF2.exe` = `build/agentGF_reg`'s exe (the tree with `ff5de25` merged; GB's `f7bc754` landed after it was built),
driven by `build/agentGB/gb_run.py` with `build/agentGB/gb2_steps.txt` (the ride, then two mouse turns and `W` held three times
through the `-inputcmd` file) plus `-fzdiag -gadiag -gfdiag`. `GF2.log`, 0 `Critical`, captures `gb2_a..f` under the `gf2` tag
(`build/agentGF/shot_gf2_*.png`), all real.

| what | measured |
|---|---|
| the player's root-motion mode (`-gadiag rmm`) | **`rmm 2/0`** on every line of the run: before, during (`at -3744/36518/-783 physics 7 base SkeletalMeshActorMAT_2 rmm 2/0`, 129 s) and after the ride - GA's `1/1` is gone (§3.5) |
| the walk after the ride | not shown by this exe: without GB's attachment track the player leaves the ride where FW measured it, 750 units short, and is in the water when the steps start (`at 10272/22758/2137 physics 3`, `PHYS_Swimming`, `vel 0` through the three `W` holds). The walking itself is the gate's `inputtest` row (`inputtest_moved 976.5`, `peak_speed 500.0`) on the same exe; the walk off the boat needs GB's track and this port together - main after both syncs |
| the arms (`gfdiag arms`) | `body 1 DPG 1 LOD 2` from 65 to 160 s, `body 0 DPG 2 LOD 1` outside; `offset 0.0/0.0/0.0` (the final view is the non-additive position on this route, so the arm follow has nothing to add), `useFOV 0` (`armsFOV 75.0` = the view's); the three controls found (`LookAtControl_RightHand/LeftHand/Camera`, strength 1.00) and at `rot 0/0/0`; `armFollow 1.00 rotP 1.00 rotS 1.00 offW 1.00/1.00 meshFOVw 1.00` outside the soirée, all `0.00` inside it (FZ's `DisableArmFollow`); `inventory pInventory slots 0 primary none secondary none` - the inventory object exists with no slot at all (§2); `ctrlPitch 2758` after the second turn |
| the silhouettes | unchanged with the foot-placement control outputting nothing (§3.4): `NPCPawn_26` `poseH` 47.1 -> 190.1 at its channel's start, 190-331 through the ride; `gf2_a` (175 s, from the boat's end) shows the captain, the elite and a figure on the dock stretched |

## 5. Stand-ins and deviations, named

- `ADishonoredPlayerPawn::Tick`: the base call and `m_fLastDeltaSeconds` only (the banner lists the ten retail parts left out).
- `Tick_Begin`: `Tick_Mantle` and the deferred corpse drop not called. `PreBeginPlay_Actions` stays FZ's (master FSM only);
  `PreBeginPlay_AnimStates` (`0x7200e0` / `0x6e2060`) and `PreBeginPlay_PlayerCombat` (`0x71a710` / `0x6be690`) are inventory.
- `UDisSkeletalMeshComponent::TickEnd` called from the end of `USkeletalMeshComponent::Tick`, not from a component manager;
  its skip flag and `SyncPhysicsSensor` are out. The reference-engine hand edits: `UnSkeletalMesh.h` (the virtual),
  `UnSkeletalComponent.cpp` (the call).
- `ADishonoredPlayerController::OnCameraUpdate`: the pawn half only. `UDishonoredInventoryItem::OnCameraUpdate`: the derived
  hook only (`UpdateMeleeExtents`, the contexts).
- `OnCameraUpdate_Derived` pass 1: the upper FSM and the powers are not told. The `|= 8` on the mesh's component flags at the
  end of retail's body is taken as the transform dirty (`BeginDeferredUpdateTransform`).
- The equip path, the item contexts, `MAT_SetForceInventory`'s callees: §2.
- The generator was re-run after the two units came off the exclude list (`dishonoredgameclasses.h` +2 includes,
  `DishonoredGameRegistrants.cpp` +2 anchors, `Sources.cmake` -2 excludes, count 750); no reflected member added or moved.

## 6. What is left, specified

1. **The equip path** (a package of its own): `InitInventory` 413 B, `SpawnAndAddInventoryItem` 147, `AddInventoryItem` 451,
   `DoEquip` 399, `EquipItemType` 110, `ShowInventoryItem` 141, `AttachItemToSocket`, the item's `PostBeginPlay` / anim states /
   skeletal component, `UDishonoredItemEmpty`, then `MAT_SetForceInventory`'s two callees and the item contexts' soirée slot.
   With it the user's report can be reproduced: equip, ride, and see whether the item's stance sits in front of the camera bone.
2. **The silhouettes**: §4.3, the Edge evaluation of the `Tall_SC_Tower` sequences on the guard skeleton (GC / the evaluator's
   owner). A quick discriminator for them: `-gfdiag`'s `poseH` on `NPCPawn_17` (one channel, 247 vs 148).
3. The player's body during a soirée: the first-person base tree at weight 1 (§4.2) - whether retail's player group carries a
   body animation this seam does not instance (the `PlayerGroup0` tracks: one `InterpTrackStretchAnimControl`, `CamShake`) or
   whether retail too shows the stance, is answered only with an item in hand.
4. `UStatePlayerUpperBase::OnCameraUpdate`, the powers' `OnCameraUpdate`, `TickCameraTracking`, `UpdateCrosshair`,
   `UpdateBlindedEffect`, `Tick_Mantle`, `m_fCurFOV_Arms`'s writers (`TickFOV`'s arms half).

## 7. The instrument: `-gfdiag`

`gfarmsdiag.cpp` / `.h`. Once a second per pawn, from `ADishonoredPawn::Tick`: `gfdiag pawn <name> t= at physics base [skel]
[bone] hard rel`, `gfdiag scale <name> drawScale meshScale meshT ltwAxes`, `gfdiag skel <name> DPG LOD hidden attach bounds
bones poseH refH (x) reach maxBoneScale maxLocalRatio at <bone> anims <every sequence node above 0.01 weight with its set,
track count, translation bone count, the set's rotation-only flag, the three compression formats, the frame count, the Edge
joint and translation-channel counts against the mesh's Edge skeleton and bone count, weight, time, stopped> [every stack /
slot / blend-pose node's child weights, for the player and for based pawns]`. Once a second for the player, from
`ADishonoredPlayerPawn::Tick`: `gfdiag arms t= body mesh DPG LOD hidden offset fov useFOV ctrlPitch` and `gfdiag arms controls
right/left/cam <control rot w>; armFollow ... armsFOV pass; inventory <slots primary secondary nothingEquipped>`. `GFDIAG_ONCE`
notes for the callees this package could not make. Nothing under the switch changes behaviour.

## 8. Files

`build/agentGF_sync.py`'s header: 30 files, every function named per file. Three units off the exclude list through the generator
(`dishonoredplayerskeletalcomponent.cpp`, `dishonoredinventoryitem.cpp`, `disskelcontrol_footplacement.cpp`, each with a
`void DishonoredLinkGF_*()` anchor), one new unit (`gfarmsdiag.cpp`), three new `CppText` headers, five edited, one new header,
two Engine hand edits, the four generated files. FX's rule kept: `dishonoredgameclasses.h`, `DishonoredGameRegistrants.cpp` and `Sources.cmake` are the generator's output.

## 9. The merges

Into the worktree branch `agentGF` after the package commit `73747d9`: `17d90a1` (STATUS), `d96a331` (GC: the locomotion node,
`arkcomponentlocomotion.h/.cpp`), `ff5de25` (PHASE19), `f7bc754` (GB: the Attachment track, `MAT_Attach/Detach`, `-gbdiag`, four
Engine headers), then `cc6b0e7` (GE: the fade, the AnimNotify noise, the spawn death) - `Sources.cmake` and
`DishonoredGameRegistrants.cpp` in conflict on the last, both generator-owned, resolved by taking main's and re-running the
generator (GF's three anchors and three units came back); the generator re-run after each merge. The gate exe predates
`f7bc754` and `cc6b0e7`. then `75cb0a6` (PHASE19, clean). `python build/agentGF_sync.py`: base `75cb0a6`, main HEAD `75cb0a6`, 30 clean, 0 flagged, 0 missing; `git diff --name-only 75cb0a6` + `git ls-files --others
--exclude-standard source resources` equals the sync list (the script checks both ways).

## 10. Acceptance

| check | result |
|---|---|
| gate `run_regression.py` (worktree copy, `--build-dir D:/RecompileDishonored/Recompile/build/agentGF_reg --newgame-at 50 --game-extra-args=-windowed`, in-harness build on the tree with `ff5de25` merged: the camera pass, the foot-placement entry and the root-motion overrides all in) | **46 ok, 1 failed, 0 skipped** (`build/agentGF/gf_regression.log`; 138,069 s wall: the build waited a night for the lock, then ran while other agents' builds used the whole machine): the one failure `d3d9_frames 630 (wanted >= 1000)` with `d3d9_startup_seconds 47.96` (5 s on every earlier gate) - the stage starved for CPU. **Re-run of that stage alone on the same exe** (`--no-build --only d3d9`, `build/agentGF/gf_regression_d3d9.log`): `d3d9_startup_seconds 4.5`, `d3d9_frames 3780`, 9 ok / 0 failed. Every other row ok: `unported_natives 0`, `nullrhi/d3d9/inputtest/newgame/options_criticals 0`, `verify_phase2 3/3`, `layout_mismatching 0`, `coresmoke 99/0`, `inputtest_moved 976.5`, `newgame_pawns 29`, `options_as2_errors 39`; no `execPlayDying_Native` in the d3d9 log. The `--only` run rewrote `summary.txt` to its nine rows; the 47-row result is in `gf_regression.log` |
| in-harness build | `build_DishonoredGame.log` 0 `error C`, 0 C4263 / C4264; `CoreSmoke` / `LayoutProbe` 0 |
| working build | `build/agentGF_rel`: `gf_build2.log` 0 errors, 0 C4263, 0 C4264 (`gf_build0.log` two compile errors in the new units - a protected `Owner`, a `Cast` on a const pointer, a `debugf` over the argument limit - `gf_build1.log` one, both fixed) |
| builds used | two: the working `agentGF_rel` (three passes of one tree state, the first two stopped by compile errors) and the gate's `agentGF_reg` (one pass, which also carried the foot-placement entry and the root-motion overrides added while it queued, before it reached the game module - 0 `DishonoredGameModule` objects built when the files changed) |
| before / after against the control | GFC1 (`9f214c5`) vs GFZ1 (`785e91d`) vs GF1 / GF2 (this): §4; captures `gfc1a..e`, `gfz1a..e`, `gf1a..e`, `gf2_a..f` |
| arms with an item equipped | not reproducible: no equip path (§0.1, §2) |
| the silhouettes | named and narrowed: not the mesh translation, not the body mode, not the base transform (§4.3), not the foot-placement control (§3.4, measured on `GF2`); what is left is the evaluation of the matinee-driven `Tall_SC_Tower` sequences on these skeletons; not gone |
| addresses | every 2012 rva from the PDB-named database, every 2013 rva from the named database or the match table (`*` in `agentGF_status.csv`, each checked by decompile shape where the body was ported) |
| input / launch rules | every launch `game_input.py launch` + `-windowed`; no real input; no retail `Dishonored.exe` |
| crashes | none; no minidump in `DishonoredGame/Logs` newer than the build |
