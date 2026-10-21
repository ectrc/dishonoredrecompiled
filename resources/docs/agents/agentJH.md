# Agent JH — interaction prompt + hover glow (wave 27, PHASE24 package JH)

Worktree `build/agentJH_wt` (base main `7ead454` = JD + JG, never moved), build dir `build/agentJH` (RelWithDebInfo), staged
`DishonoredGame_JH.exe`. IDA only on private copies `build/agentJH/idb/{retail2013,shipping2012}_agentJH.i64` (`jh_ida.py`, `jh_vt.py`,
`jh_names.py`, `jh_off.py`; decompiles in `build/agentJH/d12`, `d13`). **Two builds**: build 1 stopped on two compile errors
(`FString::Num` is protected; a `void`/`UBOOL` mismatch in the inventory fill), build 2 clean and measured. No FModel, no retail exe,
no real input; every run `game_input.py launch -windowed -nomovie` through `build/agentJH/jh_run.py` (JA's runner). Sync:
`python build/agentJH_sync.py [--apply]`. No commits.

## 1. What landed (every function with both rvas in `agentJH_status.csv`)

- **The context the prompt is built from**: `ADishonoredPlayerPawn::FillUIInteractions` (2013 0x6a9130 / 2012 0x7033f0) whole -
  master FSM state (+372) or action (+416) first, spyglass, the crosshair actor's interact / alt-interact lines, the inventory's
  active contexts (`UDishonoredInventory::FillUIInteractions` 0x805770), lean, power, movable / corpse drop+throw.
- **The interaction structs** (CppText + bodies in `disgfxmovieplayerhud.cpp`): `FDisUIInteraction` ctors (0x787c90 text, 0x78cb00
  HUD tweaks text with the gamepad / adrenaline variants), `FDisUIInteractionGroup::Add` (0x787cb0, sorted insert),
  `FDisUIInteractionContext::Add` (0x787d20, 2013's group ranges incl. DLC06/07) and `HasGroupChanged` (0x787da0).
- **The probe**: `FDisUIInteractionProbe::FillUIInteractions` (0x78cc60) and `CheckMantle` (0x78cbf0) run every frame from PreAdvance
  in retail's place (2013 has no probe Tick/timers); the four item-context checks are named and answer FALSE.
- **HUD**: `Tick_Interaction` (0x7acb10) now fills the context and sends every changed group; new `UpdateContextInteractions`
  (0x7a63c0: text + `_txtAttached=false` + `InitConvertButtonTags`), `UpdateSpecialInteractions` (0x7a6630) / `UpdateQTEInteractions`
  (0x7a6920: arrays of `{iconIdx, txt}`).
- **Interface slots**: `FillUIInteraction` (0x6280d0) / `FillUIAltInteraction` (0x628130), `LoseCrosshairFocus` (0x630810: clears
  highlight bit 1 - was empty, so a glow could never go off), `DoHighlight`'s HUD test (`m_Settings.m_bShowHighlight`, 0x787e60).
- **NPC**: `GetUObjectInterfaceDisInteractableInterface` (was missing: the interface default answered NULL for every NPC, which
  also made `AttemptInteract`'s Kismet event name no actor), `GetCrosshairFocusText` (0x753a30, the only function in
  `dishonorednpcpawn.cpp`), `GetInteractableTweaks_Derived` (0x752980), `IsAsleep` (0x74e870), `FillUIInteraction` (0x74ea80).
- **Usable object**: interactable slots +4/+8/+24/+52/+60/+84/+88/+92, the tweaks interface Get/SetTweaks (`m_pUsableObjTweaks`),
  `GetActiveStage`, the stage accessors on `UDisTweaks_UsableObject`, the key test (`HasKey` helper, `ADishonoredPawn::HasKey` base,
  `ADishonoredPlayerPawn::HasKey`, `UDisKeyRing::FindKey`).
- **Pickup / movable / climbable**: `ShowHighlight` / `HideHighlight` (highlight mesh copy; the climbable swaps its first material),
  `GetInteractableTweaks_Derived`, Get/SetTweaks and `GetHighlightFlags` where missing. `dishonoredmovable.cpp`, `disclimbable.cpp`,
  `diskeyring.cpp`, `dishonoredpowerscomponent.cpp` (`GetPowerLevel` 0x6c8a00 for the adrenaline text) came off the exclude list
  through the generator (link anchors).
- **`DisGDInteractableOf`** answers `UObject::GetInterfaceAddress` for every implementer, as retail; for the five native classes the
  C++ base is compared and kept on a mismatch (none measured: NPC, usable "same").
- **`-jhdiag`** (`Inc/jhdiag.h`): every prompt Invoke with its arguments + frame, every Show/Hide/refused highlight with the flags,
  every focus change with the frame, the context once a second; **`-jhface=<name|class:Cls>@t@dist`** places and aims the player.
- Generator: `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentJH/regen` byte-identical
  to the tree (79/79); `bitfield_audit` mismatches 0.

## 2. Measurements (build 2, route `-finewgame=50 -fosoiree`, 1920x1080)

| run | what | result |
|---|---|---|
| R1 (`-fwteleport=DisTrigger_8@150 -gdface=DishonoredNPCPawn_7@250@120`) | Talk on the ride's captain | world 150.65, frame 3244: focus `DishonoredNPCPawn_26` status 4 -> frame 3245 `crosshairInfosTxt_mc.SetText(Geoff Curnow)`, `interactions_window_mc.SetInteractions('Geoff Curnow', '[F] Talk')`; focus lost frame 3255 -> both null |
| R1 | Talk on `DishonoredNPCPawn_7` | world 250.21: `SetInteractions('Spymaster Burrows', '[F] Talk')`, capture `build/agentJH/R1/w0250.404.png` (name + "[F] Talk" drawn); off at 250.63 when the Empress scene takes the player |
| R2 / R3 (`-jhface`) | usable `DishonoredUsableObject_9` | focus frame 3524 = `ShowHighlight ... flags 1 material vfx_interactivity.interactivity_INST` = `SetInteractions('Crank Wheel', 'Hold [F] Turn')`, same frame; capture `R2/w0164.426.png` (yellow glow + prompt); R3 focus loss at frame 3668 = `HideHighlight ... flags 0`, same frame |
| R2 / R3 | pickup `DisStatPickup_0` at 80 / 40 / 30 uu, view on it (`R2/w0156.430.png`) | **no focus**: the crosshair never lands on the pickup (see 4.2) - no "Pick up" prompt measured |
| R2 | `class:DishonoredMovable`, `class:DisClimbable` | none exist in the loaded levels ("no actor for ...") |
| all | context / special / QTE clips | 0 invokes: nothing on the route fills them (no lean, carry, mantle spot under the probe, powers) |

Gate: see §5.

## 3. Deviations (named in the code)

- `Tick_Interaction`, `ADishonoredNPCPawn::FillUIInteraction`, `ADishonoredPlayerPawn::HasKey` test a pointer retail reads blind
  (player pawn, controller, key ring).
- Possession branches (`DisIsPossessed`, unported) of the usable fill / key test use the pawn itself.
- `ADishonoredUsableObject::LoseCrosshairFocus` does not call `HandleUseStopped` (the use path is unported).
- The power branch of the pawn fill ends the fill as retail but the power components' fills are unported.
- The four item-context probe checks answer FALSE (the item-context family is unported).
- `DisGDInteractableOf`: C++-base check on the five native classes (diagnostic guard; equal on every class measured).
- The adrenaline text's FName is `BloodThirsty` (2012 `DNAME_BloodThirsty`); unreachable while its check answers FALSE.

## 4. Where the brief was wrong (with the measurement)

1. **"ShowHighlight/HideHighlight on usable (0x66da10/0x66b370), pickup (0x66c9a0/0x66a990), movable (0x66e720/0x66b710), climbable
   (0x66ae20/0x66ae60)"** are **2012** rvas; 2013: 0x645630/0x6422c0, 0x61d8c0/0x619bc0, 0x645e90/0x6428e0, 0x61a200/0x61a240.
2. **"DoHighlight's HUD check (2013 0x7bd0d0)"**: 0x7bd0d0 lies in sub_BBD090 (85 B), called only from
   `HandleHeldButtons_Context_PowerWheel`. DoHighlight's check is 0x787e60 (13 B): HUD settings bit 3 = `m_bShowHighlight`.
3. **"port the base state's and the walk/idle states' fills"**: they are `xor eax, eax; retn 4` (2013 rva 0x233610) - nothing to port;
   the fills with content are HolePeeking/Versus/Climb/Choke/Assassinate/Minigame, all in unported (excluded) units.
4. **"FDisUIInteractionProbe 0x78cc60; Tick 2012 0x80c000"**: 0x78cc60 is the probe's `FillUIInteractions`; 2013 has no Tick - the
   five checks (incl. the new `CheckChoke` 0x7a5250) run inline in PreAdvance every frame and the struct shrank 28 -> 12 bytes.
5. **"FindInteractableActor 0x6b70f0, 1,302 B"**: 1,302 B is 2012's; the 2013 body is 2,691 B and needs the masked probe family
   (`DisLineProbe_Interact`, `DisLineProbeMulti_Interact`, `DisLineSegBoxIntersect`) - not ported, nor `FindCombatTarget_Dir`.
6. **"-distouchprobe route"**: that is the inputtest stage, run with `-nodishud` - there is no HUD movie, so no prompt and no glow
   (DoHighlight needs the HUD) can be measured there; the measurements are on the newgame route.
7. **"Talk on the captain"**: GD's `DishonoredNPCPawn_7` is Spymaster Burrows; the captain is Geoff Curnow (`DishonoredNPCPawn_26`),
   whose prompt appears by itself at world 150.6 on the ride. Both measured.
8. **"the glow on the captain"**: in 2013 the NPC's highlight slots (+88/+92) are empty bodies - NPCs never glow.
9. **"only pickups have m_Name"**: the missing piece on NPC / usable / movable / climbable was the whole `GetInteractableTweaks_Derived`
   (and the tweaks getter it reads); the NPC also lacked `GetUObjectInterfaceDisInteractableInterface`.
10. HF's `HasGroupChanged` helper compared indices (2012); 2013 compares the text pointers.
11. **4.2 the pickup**: the pickup prompt is filled by the interface default (`m_InteractText`) and the glow by
    `ADisPickup_Base::ShowHighlight`, both ported, but the crosshair never selects `DisStatPickup_0` even at 30 uu looking at it
    (no focus line in R2/R3). The selector is GD's stand-in narrow probe (`UWorld::SingleLineCheck` for retail's
    `DisLineProbe_Interact`): the next step is that probe family / the 2013 `FindInteractableActor` (item 5), out of this package's builds.

## 5. Gate

`run_regression.py --build-dir ../agentJH --no-build --exe-name DishonoredGame_JH.exe --log-prefix JHg_ --only d3d9,inputtest,newgame`
(`build/agentJH/gate.txt`): **23 ok, 0 failed** (542 s). Criticals 0/0/0, `unported_natives` 0, **`probe_natives` 3** (bound 3; the
same three as before this package: `OnTeleport_Native`, `OnOverrideAwarenessDisplay`, `PlayDying_Native` - no new stub is reached by
the fills), `inputtest_moved` 798.3, `inputtest_peak_speed` 400.0, `newgame_on_mission` 120, `newgame_pawns` 29.

## 6. Open

- The masked interact probe + 2013 `FindInteractableActor` (pickups never get the crosshair; item 4.11).
- `CanAltInteract` / `CanBeCarriedBy` / NPC `FillUIAltInteraction` (carry prompt on bodies); `HandleUseStopped`; movable
  `GetCrosshairStatus` / `CanInteract`; the special master states' fills; the item-context probe checks; the power components' fills.
- `-jhdiag census` runs on the first world only (the menu's), so it listed 0 interactables; use `-jhface=class:...` instead.
