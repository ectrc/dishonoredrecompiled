# Agent GJ — the equip path exists: the inventory initialises, the sword is spawned, equipped and attached to the arms' hand bone; the arms do not come into frame because the item's anim states are not ported; the boat does not force the hands empty; the look-at warnings drop from 1,170 to 0

Wave 24 (`PHASE20.md`), package GJ. Worktree `build/agentGJ_wt`, base `4b889ff`; main `99eaf6b`, `0e5207a` (GH, GK, GG; both merges clean) and
`8ea61f0` (GI; two pawn CppText headers in conflict, both sides kept; `Sources.cmake` and `DishonoredGameRegistrants.cpp` taken as
main's and regenerated) merged by `git merge`. Working build `build/agentGJ_rel` (`build/agentGJ/gj_build.cmd` through `build_queued.py`;
passes `gj_build0..4.log`: 0 stopped on two compile errors (an include, a duplicate `DECLARE_FUNCTION`), 1 clean, 2 the item glue + the
diag equip, 3 after the `0e5207a` merge, 4 the diag's view line - every pass 0 errors, 0 C4263, 0 C4264). Gate `build/agentGJ_reg`
(`gj_regression.log`, run twice: before and after the `0e5207a` merge; not re-run after `8ea61f0` - the coordinator's call, no fifth
build). Sync `build/agentGJ_sync.py` (36 files). Control `build/head_rel` (`99eaf6b` = `4b889ff`'s code) staged as
`DishonoredGame_GJbase.exe`; this package `DishonoredGame_GJ.exe`. Every launch `game_input.py launch` + `-windowed`, no real input, no
retail `Dishonored.exe`. Every address resolved in `build/agentGJ/idb/GJ_shipping2012.i64` / `GJ_retail2013.i64` (the headless
decompiles are in `build/agentGJ/decomp2012`, `decomp2013`; 81 + 56 functions).

## 0. The answers

1. **The equip path is ported** (§2): `PreBeginPlay_Inventory` → `InitInventory` gives every Dishonored pawn two empty-hand slots holding
   a `UDishonoredItemEmpty` (spawned from the pawn tweaks' `m_pEmptyHandsItem`) plus the tweaks' `m_InventorySlots`;
   `PostBeginPlay_Inventory` (player) spawns the powers item and applies the map's or the default player loadout; `SpawnAndAddInventoryItem`
   → `InitItem` (the tweaks applied: the item meshes get their skeletal mesh, physics asset, anim sets, anim tree) → `AddInventoryItem` →
   `OnAddToInventory` (the item contexts built); `EquipItemByType` → `EquipItemType` → `DoEquip` → `AttachItemToSocket` attaches the item's
   player mesh to the owner mesh's socket named by the item tweaks (`handAttachment_R_jnt` for the sword), in the arms' depth group.
   Measured (`GJ2`, `-gjdiag`): `slot 2 item DishonoredWepSword equipUsage 1 socket 1 mesh pPlayerMesh skel AssassinSword hidden 0
   attached 1 parent pMesh at handAttachment_R_jnt DPG 2`, `hand 1 equipped DishonoredWepSword reequip DishonoredWepSword` from 0.0 s to
   the end of the run; 0 `Critical`, no minidump.
2. **Retail's New Game gives the player nothing, and there is no sword tweak in the campaign loadout**: `Twk_PlayerLoadouts.
   Twk_PlayerLoadout_Campaign` (the default - `l_tower_p`'s `UDishonoredMapInfo` has no map-specific loadout) carries 0 items and no default
   equip types; the only things the player gets are the two empty hands and the powers item (`Twk_Inv_Powers`); 9 slots. The sword's
   own tweak is not loaded by this map at all: the first loaded `UDisTweaks_InventoryItem` whose spawned class is `DishonoredWepSword` is
   `Twk_Inv_Assassin.Twk_Inv_SwordAssassin_Player` (18 candidates loaded, none the player's own `Twk_Inv_Sword*`). So the measurement
   equips that one (`-gjgive=DishonoredWepSword -gjequip`, the loadout's own instant-equip path: `SpawnAndAddInventoryItem` +
   `EquipItemByType( InstantEquip )`). GF's premise (the sword comes "later") stands: it is given by a later mission's loadout / pickup.
3. **The key the game binds**: none for the sword. The bindings are `Dis_EquipItemByType <class> <usage>` (pistol, crossbow, grenade,
   spring razor, heart - all secondary) and the wheel shortcuts 1..0 (`Dis_WheelShortcuts_*`, the GFx wheel, unported). The sword is the
   primary hand's loadout default and is re-equipped by `GetReequipItem` after an empty-hand scene. `game_input.py` gains `exec <console
   command>` (an Engine hand edit in `UnPlayer.cpp`'s inputcmd handler, `UGameViewportClient::ConsoleCommand`) and
   `Dis_EquipItemByType` + its exec are ported, but **on this route the exec never reaches the controller** (§4.2): under the intro the
   input mask 0x10 is cleared by the cinematic mask (retail refuses it too), and the console path through `ULocalPlayer::Exec` →
   `ScriptConsoleExec` did not call the native even after the ride (`GJ2`, `exec` at 156.9 s: no trace) - the console dispatch is a
   seam of its own (§6).
4. **The arms with the sword drawn: not in frame** (§4.1). The sword is attached to the arms mesh's hand bone and drawn in the arms' depth
   group, and the frames before and after the ride are identical to the control's: the arms mesh plays the first-person tree's empty-hands
   base pose, in which the hand bone sits below the view. What moves the arms into the sword stance in retail is the item's anim-state
   system (`UDisAnimStateComponent::BindToAnimStateOwner`, `FindAnimStatePickers`, `SyncPawnAnimState`, the `UStatePlayerTransitionItemIn`
   / `UStatePlayerEquipChange` action states) - none of it has a body (named once each). The user's "arms when I shouldn't" is therefore
   still not reproducible: with the sword equipped nothing of the arms is seen, before, during or after the soirée.
5. **GF's arm offset and arms FOV**: `offset 0.0/0.0/0.0`, `useFOV 0`, `armsFOV 75.0` = the view's on every sample of `GJ2` (FZ's camera
   unit sets `m_fCurFOV_Arms = DefaultFOV` and interpolates it towards an arms FOV target that nothing on this route changes). Retail's
   tweak values are not measurable from a cooked object the diag can name: `UDisTweaks_PlayerPawn_Camera` carries the three look-at control
   names and no FOV; the arms FOV comes from the item's zoom (`UDisTweaks_InventoryItem_PlayerSpecific::m_fZoomFOV`) and the camera's
   `m_fFOVScaleMultipliers`, which only a zoomed item reads - no item zooms here.
6. **The soirée does not force the hands empty on the boat** (§4.3): `MAT_SetForceInventory` is now retail's whole body (the two re-equip
   memories set to `UDishonoredItemEmpty`, the movable drop, two `EquipItemByType_KeepTrying( UDishonoredItemEmpty )`), but the boat scene
   calls it with `m_bForcesUndrawBeforePlaying` FALSE: the hand holds the sword through enter (65 s) and exit (160 s), `desired none pending
   0` on every sample. The premise "during the ride the item is forced to Empty and the arms hidden" is wrong for this scene; a scene
   whose matinee has the flag would take the ported path. And what `EnterSoireeMode` (2013 `0xa66ff0`) calls on both hands' items at
   2013 `0x816d90` is **`CancelActiveContexts( TRUE )`** (2012 `0x859af0` without the flag) - a cancel of the swing in progress, not a
   hide or a holster: GF's "item contexts' soirée slot" is `UDisItemContext::CancelContext` (slot 74) on each active context.
7. **GH's note - the look-at warnings**: `Failed, with animsequence` 1,170 on the control (`GJC1`, `GJC2`) → **0** on this build (`GJ2`):
   every NPC now holds a `UDishonoredItemEmpty` in both hands, so `ChooseLookAt` → `GetDefaultNPCLookAtMode` answers EmptyIdle / Walk /
   Run instead of `eDisNPCLookAtMode_MAX`. The NPCs do not get their weapons yet (their loadout tweaks are reached through
   `ADishonoredNPCPawn`'s inventory override, not ported - §6).
8. Gate (§7): 47 ok / 0 failed / 0 skipped on both runs. Premises corrected: §1.

## 1. Where the brief was wrong, with the measurement

| the brief said | measured |
|---|---|
| "the key the game binds to draw it" | no key binds the sword; the `GBA_Equip*` keys are `Dis_EquipItemByType` for the secondary items, 1..0 are the GFx wheel; the sword is the loadout's primary default and the re-equip memory |
| "retail starts with nothing in hand - find what the first mission gives the player" | nothing: `Twk_PlayerLoadout_Campaign` has 0 items, no default types; the powers item and two empty hands are all (`GJ2`, `gj loadout default`) |
| "the item contexts' soirée slot 2013 0x816d90" | `UDishonoredInventoryItem::CancelActiveContexts( bForced )` → `UDisItemContext::CancelContext` on each active context; cancels, does not hide |
| "during the ride the item is forced to Empty and the arms hidden" | the boat's `m_bForcesUndrawBeforePlaying` is FALSE: the sword stays in hand through the ride (`hand 1 equipped DishonoredWepSword` 0..181 s) |
| "capture arms + sword in first person" | the sword is on the hand bone in the arms' depth group, but the arms play the empty-hands base pose and are out of frame; the item anim states that pose them have no bodies |
| "GF's arm offset and arms FOV become measurable" | offset 0, arms FOV = view FOV 75 on every sample; the retail tweak behind the arms FOV is the item's zoom FOV, read only by a zoomed item |
| "the retail save's inventory objects are a truth source" | not needed: the loadout tweaks are the truth for a New Game and they are logged; `Dishonored0.sav` is a later mission's inventory (ER) |
| "Dis_EquipItemByType through game_input.py" | the exec is ported and reachable in principle; the console dispatch did not call it on this route (§4.2) |

## 2. What was ported (every rva in `agentGJ_status.csv`, 2012 / 2013)

`UDishonoredInventory` (`dishonoredinventory.cpp`): `InitInventory` `0x85b790` / `0x818c80`, `AddInventorySlot`, `GetRequiredSlot` x2,
`GetNextEmptySlot`, `CanEquipItemType` x2, `DoEquip` `0x85a7b0` / `0x817d10`, `EquipItemType` `0x85a940` / `0x817eb0` (the
`DisGameEventType_ItemEquipped` event, id 14), `AttachItemToSocket` `0x855a50` / `0x80f6e0` (the 2013 depth-group recursion over the
attachments taken), `ShowInventoryItem`, `ShowEquippedItem`, `SetReequipItem`, `GetReequipItem`, `AddInventoryItem` `0x85be60` / `0x819380`
(2013's extra `_bRemoveIfNeeded` not taken), `SpawnAndAddInventoryItem` `0x85c9c0` / `0x819cd0`, `TickInventory`, `ClearInventory`,
`RemoveItemsByClass`, `PlayerTransitionItemIn`. `UDishonoredInventoryItem` (`dishonoredinventoryitem.cpp`): `InitItem` `0x8580f0` /
`0x80f9f0` (attributes from `m_pAttributeTweak[1]`, the difficulty event registered), `OnAddToInventory` (the five context slots built -
`InitItemContextSlot` constructs the contexts with their slot and tweaks; `InitContext` named), `OnEquipItem` / `OnUnEquipItem`,
`IsEquippable`, `ApplyTweakChanges_Derived` `0x85cc80` / `0x819ff0` (the meshes: skeletal mesh, physics asset, anim sets, anim tree
template; the reference engine's `AnimSets.AddUniqueItem` / `Animations` for retail's `AddExtAnimSetUnique` / `SetAnimationsSeq`),
`TickInventoryItem`, `GetItemAimForAnims_Derived`, `ModifyPlayerStance`, `GetContextsForSlot`, `IsItemContextActive`, `HasActiveContexts`,
`CancelActiveContexts`, `AnimState_Get*`, `PlayerTransitionItemIn_Derived` (the on-screen half), the two tweaks-interface glue virtuals
(without `GetUObjectInterfaceDisTweaksInterface` returning the item, `ApplyTweakChanges` never reached the derived hook - the first run's
`skel none`). The concrete items' tweaks slots: `UDishonoredWepSword` (+ `InitItem`'s transient material instance), `UDishonoredItemEmpty`,
`UDisItemPowers`; `UDishonoredItemSkeletalComponent::SetInventoryItem`. `ADishonoredPawn` (`dishonoredpawn_inventory.cpp`):
`GetEquippedItem`, `ShowEquippedInventoryItem`, `PrimaryHandAppearEmpty`, `HandsAppearEmpty`, `AttachEquippedItem`, `EquipItemByType`
`0x7c8500` / `0x75d400`, `EquipItemByType_KeepTrying`, `PreBeginPlay_Inventory` `0x7c38a0` / `0x758860` (dispatched from `PreBeginPlay`
after `StartAllComponents`), `PostBeginPlay_Inventory` (dispatched from `PostBeginPlay` before `_Body`), `Tick_Inventory` (the fifth tick
phase, from `Tick`), `SpawnInventoryLoadout_Items`; EF's `SpawnInventoryLoadout` completed (the items and the two instant default equips).
`ADishonoredPlayerPawn` (`dishonoredplayerpawn_inventory.cpp`): `PostBeginPlay_Inventory` `0x718360` / `0x6ad200`, `EquipItemByType`
`0x70ce90` / `0x6b3210` (**stand-in**: retail requests `UStatePlayerEquipChange` from the hand's FSM and only the instant flag takes the
direct path; the state is a skeleton, so every equip is taken the instant way - said once), `TransitionItemIn`, `GetDesiredPlayerStance`,
`GetActualPlayerStance`, `SetPlayerStance`, `IsLeftHandDisabled`, `IsSneaking` (the toggle); `MAT_SetForceInventory` completed.
`ADishonoredPlayerController`: `Dis_EquipItemByType` `0x6ec850` / `0x6b4a70`, its exec, `CanEquip`. `UDisTweaks_PlayerLoadout::
ApplyPlayerLoadoutToPlayer` (the inventory half; upgrades, charms, powers named), `UDishonoredMapInfo::GetMapSpecificPlayerLoadout`.
Seven units off the exclude list through the generator (`dishonoredplayerpawn_inventory`, `dishonoreditemempty`,
`dishonoreditemskeletalcomponent`, `dishonoredwepsword`, `disitempowers`, `distweaks_playerloadout`, `dishonoredmapinfo`), each with a
`DishonoredLinkGJ_*` anchor; the exec stub dropped by the generator. Engine hand edit: `UnPlayer.cpp` (`exec` in the inputcmd script).

## 3. The instrument: `-gjdiag`, `-gjgive=<class>`, `-gjequip`

`gjequipdiag.cpp`. Once a second for the player: `gjdiag view` (camera, FOV, the arms mesh bounds, the `handAttachment_R_jnt` bone),
`gjdiag slot <n>` per slot (required type / usage, the item, its usage, socket, tick flag, the player mesh: skeletal mesh, hidden, attached,
parent, socket, DPG, LOD, anim tree, anim sets, bounds), `gjdiag hand 1/2` (equipped, slot, re-equip, non-empty re-equip, the keep-trying
desire, pending count); `MAT_SetForceInventory( flag )` and `Dis_EquipItemByType` traced. `-gjgive=<class>` spawns one item from the
first loaded tweak whose spawned class matches (map loadout, default loadout, every loaded `UDisTweaks_InventoryItem`); `-gjequip`
equips it the loadout's instant way. `GJDIAG_ONCE` names every callee not made (`InitContext`, `OnEquipmentChange`, `TickAttributes`, the
anim-state bind / pickers / sync, `ClearAnimState`, the item-in action, `DropItemsInSlot`, the contexts' tick / cancel, the upgrades,
charms and powers of a loadout, `EquipBestSuitableItem`, `NextItemOption`, the left-arm FSM reset).

## 4. The runs (`build/agentGJ/gj_run.py`, FZ/GF's route `-finewgame=50 -fosoiree ... -gjdiag`, captures 1920x1080, thumbnails `shot_*.png`)

| run | exe | log | what |
|---|---|---|---|
| GJC1 | control | `GJC1.log` | `gj_steps.txt` (exec at 51 s, shots 55 / 94 / 133 / 158 / 168 s): `exec` is an unknown command to the control; 1,170 look-at warnings; captures `gjc1_a..e` |
| GJ1 | build 1 | `GJ1.log` | the same steps + `-gjgive`: the sword spawned (`slots 9`) but **`skel none`** (the glue, §2) and the exec refused (§4.2); captures identical to the control |
| GJ2 | build 3 | `GJ2.log` | `gj2_steps.txt` + `-gjgive -gjequip`: the sword equipped at 0.0 s and attached; shots 51 (black, the fade) / 89 / 124 / 149 / 159 s; 0 warnings; `gj2_a..e` |
| GJC2 | control | `GJC2.log` | `gj2_steps.txt`: `gjc2_a..e` - identical to GJ2's frames; 1,170 warnings |
| GJ3 | build 4 | `GJ3.log` | GJ2 + the view line and the force-inventory trace (§4.1, §4.3); 0 warnings, 0 `Critical`; the exec answered '' and never entered the native (§4.2) |

### 4.1 The arms with the sword (GJ2 / GJ3 vs GJC2)

`gjdiag slot 2 ... mesh pPlayerMesh skel AssassinSword hidden 0 attached 1 parent pMesh at handAttachment_R_jnt DPG 2 LOD 0 tree none` from
0 s; `DPG 1` from 63 s to 160 s (the arms mesh's `SetBodyMode` switch carries its attachments). `gfdiag arms`: `body 0 DPG 2 LOD 0` outside
the ride, `body 1 DPG 1 LOD 2` inside, `offset 0.0/0.0/0.0 fov 0.0 useFOV 0`, the three look-at controls at `rot 0/0/0`. The captures
before (`gj2_e`, 159 s, looking down at the boat's gunwale from the lock) and during (`gj2_b`, `gj2_d`: the captain and the elite, normal
size since GH) show no arm and no sword, and are pixel-for-pixel the control's view. The item's player mesh has no anim tree
(`tree none`: `Twk_Inv_SwordAssassin_Player`'s player-specific tweaks name no `m_pItemAnimTree`) and the arms play the empty-hands base
tree: the hand bone is below the frame. GJ3's `gjdiag view` at 1.34 s: camera `8/500/-129` pitch 0, FOV 75.0, arms mesh origin
`17/500/-177` radius 136, `handAttachment_R_jnt` at `9/550/-197` - 50 units ahead of and 68 below the camera, against a vertical half-angle of
about 23 degrees (21 units at that distance); at 59.9 s on the boat the same geometry (`camera -3900/36567/-314`, hand `-3853/36583/-381`).

### 4.2 The key-bound path

`GJ1`: `exec Dis_EquipItemByType ...` at 51.0 s, before the ride: no `gjdiag Dis_EquipItemByType` trace, hand unchanged. `GJ2`: the same
exec at 156.9 s, after the soirée exit (152 s): no trace either. The inputcmd handler runs `UGameViewportClient::ConsoleCommand`, which
is `Exec` → `ULocalPlayer::Exec` → `Actor->ScriptConsoleExec`; the cooked `Dis_EquipItemByType` is an `exec` native, so the dispatch
should land. GJ3 (exec at 168.7 s, after the ride): the console answered the empty string and `Dis_EquipItemByType` was not
entered (no trace) - `ScriptConsoleExec` did not resolve the exec native on the controller; the seam is in §6. Under the intro, retail refuses it anyway: `IsInputEnabled( 0x10 )` is
masked by `m_bInputIgnoreInput_Cinematic` (`0xFFF81303` clears bit 4).

### 4.3 The soirée

`fzdiag soiree ENTER` at 113 s log / 57 s world, `EXIT` at 217 s log / 152 s world (GJ2); `gjdiag hand 1 equipped DishonoredWepSword slot 2
reequip DishonoredWepSword nonEmpty DishonoredWepSword desired none pending 0` on every one of the 180 samples: `MAT_SetForceInventory` was
called with FALSE: GJ3 `gjdiag MAT_SetForceInventory( 0 ) t=55.96, in dialog 0` - the scene's `m_bForcesUndrawBeforePlaying` is cooked FALSE for
`SeqAct_Interp_1`.

## 5. Stand-ins and deviations, named

- `ADishonoredPlayerPawn::EquipItemByType`: the FSM equip-change action is not requested (skeleton) - instant path for every equip.
- `PlayerTransitionItemIn_Derived`: the offscreen branch (the item-in / ready-stance actions) answers TransitionFailure, named.
- `OnAttached` / `OnDetached` / `InitItemContextSlot` / `TickInventory`: the anim-state system and the contexts have no bodies - named.
- `AddInventoryItem`: `DropItemsInSlot` not ported (an occupied slot refuses); 2013's `_bRemoveIfNeeded` not taken.
- `ApplyPlayerLoadoutToPlayer`: the inventory half only. `ADishonoredNPCPawn`'s `AttachEquippedItem` and `Tick_Inventory` overrides not ported.
- `StopZoom`: the zoomed half named. `IsSneaking`: the choke state's flag not read. `GetItemClassBeingEquipped`: NULL.
- The sword's `InitItem` wraps material 0 in a transient `UMaterialInstanceConstant` as retail; `m_bSkipUpdate` for a mesh without anim sets.
- Engine hand edit: `UnPlayer.cpp` (the `exec` inputcmd command). `game_input.py`'s `exec` text landed on main through GH's merge
  (`ece8e1b`): the worktree's copy is byte-identical to main's and is not in the sync list.

## 6. What is left, specified

1. **The item anim states** - what poses the arms with an item: `UDisAnimStateComponent::BindToAnimStateOwner` (2012 `0x6b1ab0`),
   `FindAnimStatePickers` (`0x861370`), `SyncPawnAnimState` (`0x85ad40`), `ClearAnimState` (`0x6c0c20`), the pickers of
   `UDisTweaks_InventoryItem_PawnSpecific::m_AnimStatePickerName` / `m_DefaultAnimState`, and the upper FSM's `UStatePlayerEquipChange`
   (`DoEquipChange` 2013 `0xa643b0`), `UStatePlayerTransitionItemIn`, `UStatePlayerChangeReadyStance` (all skeletons). With them the
   user's report becomes measurable on this route: `-gjgive=DishonoredWepSword -gjequip` then the ride.
2. **The console dispatch**: why `ScriptConsoleExec` did not call `execDis_EquipItemByType` (GJ3's answer line); `DisNotify_AttachItemInSlot`.
3. **The NPCs' items**: `ADishonoredNPCPawn`'s inventory override (`UpdateEquipment` `0x7b7b90`, `GetIntendedItemToEquip` `0x7aee10`,
   its `AttachEquippedItem` `0x7b7950`, the NPC tweaks' loadout) - the guards' swords.
4. `UDisItemContext::InitContext` / `CancelContext` / the contexts' tick; `DropItemsInSlot`; `UDisWheelTracker`; `OnEquipmentChange`.

## 7. Acceptance

| check | result |
|---|---|
| gate, before the `0e5207a` merge (`gj_regression_pre_merge.log`, 2403 s) | **47 ok, 0 failed, 0 skipped**; `d3d9_frames 5820`, `d3d9_startup 5.92`; `unported_natives 0`; every `*_criticals 0`; `verify_phase2 3/3`; `coresmoke 99/0`; `inputtest_moved 1024.4` (the harness re-ran inputtest once: first attempt `peak_speed 0.0`) |
| gate, after the `0e5207a` merge (`gj_regression.log`, 1752 s) | **47 ok, 0 failed, 0 skipped**; d3d9 first attempt `d3d9_frames 900` (build 4 compiling beside it), the harness's own re-run `1200` with `d3d9_startup 44.29` (still under load); every other row as above; not re-run idle - the coordinator's instruction |
| gate build log | `build/agentGJ_reg/regression/build_DishonoredGame.log`: 0 `error C`, 0 C4263, 0 C4264 |
| working build | `gj_build1..4.log` 0 errors, 0 C4263, 0 C4264 |
| builds used | two build directories: `agentGJ_rel` (five passes) and `agentGJ_reg` (two gate runs) |
| sync | `python build/agentGJ_sync.py`: base `8ea61f0`: 36 files, 0 flagged, 0 missing; the list equals `git diff --name-only 8ea61f0` + the untracked |
| generated files | regenerated after every merge; `git status` clean after the regeneration |
| crashes | none in GJC1, GJ1, GJ2, GJC2, GJ3; the one new minidump of the evening (`18.26.25.dmp`) is `DishonoredGame_GI.exe`'s |
| rules | every launch `game_input.py launch` + `-windowed`; no real input; no retail `Dishonored.exe` |
