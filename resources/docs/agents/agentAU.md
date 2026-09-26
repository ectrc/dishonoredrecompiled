# Agent AU — DishonoredGame gameplay: pickups are collected end to end (2026-09-27)

Package AU of `PHASE7.md`. Agent AS made the world touch pickups but nothing could be taken, because
`DishonoredGame/Src/dispickup_base.cpp` was a 46-line comment stub and `DisPickup_Base` declares no script `Touch`:
collecting a pickup is entirely a C++ path. **That path is now ported and it works.** On `L_Pub_Day_P` the probe walks
the pawn to each of the map's 147 pickups and takes 122 of them; the inventory ends the run holding **1,874 coins, 5
notes, 20 books, 1 rewire tool and 4 sleep darts**, and each collected pickup flies to the camera, is consumed and
destroys itself, exactly as retail's `Tick` does it.

Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentAU.i64`, a copy of `retail2013_named.i64`); every
"2013 rva" is a headless decompile in `build/agentAU_decomp/r13`, and the "2012" rva in brackets is
`shipping2012_agentAU.i64` (`build/agentAU_decomp/s12`), which carries the symbol names and is the readable version of
the same function. Member offsets were resolved numerically against `resources/docs/types/retail_sdk_layout.json`.

**Read this first if you inherit the package.** Agent AS's hand-over list gave the pickup functions as "2013 rvas"; they
are **2012 Shipping rvas**. `dispickup_base.cpp`'s skeleton banner (and every other `import_reference.py` banner) is
attributed from the *2012* PDB. `0x680a00` is `ADisPickup_Base::PostBeginPlay` in the 2012 exe and a local class's
deleting destructor in the 2013 one. `build/agentAU_work/sym.py <regex>` prints both rvas side by side out of
`functions.csv` / `functions_2013.csv` / `match_2012_2013.csv`; `build/agentAU_work/off.py <Class> [offset]` names the
member at a retail byte offset. Those two helpers are what made this package tractable and are worth promoting to
`resources/tools/`.

## Result against the accept line

| Acceptance | State |
|---|---|
| **a pickup picked up end to end with AS** | **done.** 147 pickups probed on `L_Pub_Day_P`, **122 collected**, inventory reflecting every one of them (numbers below). The collection runs through retail's own chain: `IDisInteractableInterface::AttemptInteract` → `ADisPickup_Base::AttemptInteract_Derived` → `DoInteract_Impl` (the subclass gives the item) → `StartPickupTravel` → `Tick`'s travel interpolation → `ConsumePickup` → `DisDestroyActorNextTick` |
| at least 120 natives ported | **not met: 9.** 90 *functions* are written; only 9 of them are script natives. The reason is the same one agent AJ documented and it is now measured rather than argued — see "Why 9 natives and not 120" |
| `-strictnatives` reaches the mission map without aborting | **half.** It reaches it (`Finished loading level` twice, `Initial startup` 19.84 s) and then aborts at 22.54 s on the **one** remaining native, `ADishonoredPlayerPawn::execTakeFallingDamage_Native`. Before this package it aborted earlier, on `execHandleHeldButtons`. That native cannot be ported faithfully yet and the blocker is named exactly below |
| no DishonoredGame warn-once line on the walking path | **4 → 2.** `execHandleHeldButtons`, `execGameEnding` and `execDis_Zoom` are gone; `execLanded_Native` and `execTakeFallingDamage_Native` remain, both blocked on the same two subsystems (the contact system and the attributes system) |
| the dependency roots that gate everything else | **not ported; surveyed instead.** `UDishonoredAIBrain`, `UDisAISubProcess`/`UDisAISubState`, `UDisItemContext` and `FArkComponentLocomotion` were left alone because the pickup path was the package's visible win and took the whole budget. What this package *did* find is a **fifth root agent AJ's list does not name** — the attributes system — and that it is cheap (≈1.9 KB of code) but blocked on unreflected native members. Full costing below |
| 0 critical errors | **done** on every run: `L_Pub_Day_P` 180 s d3d9, `L_Tower_P` 130 s d3d9, plain null-RHI startup |
| no walking regression | **done.** `L_Tower_P`: `inputtest moved 1021.6`, physics 1 (agent AS: 1023.2, agent AQ: 972.2 … 1030), 0 `Could not create new Shape`, touch still live (`touching now 11 actors / 16 pairs`) |

## Pickups, end to end

`-dispickup` and `-dispickupprobe` (both off by default, free when off) are the census and the walk-in, modelled on
agent AS's `-distouch` / `-distouchprobe`. The probe moves the pawn to the pickup's collision-component bounds origin
with `UWorld::FarMoveActor` and then makes **the same call `UDisUseState_WaitForInput::TickState` makes on the crosshair
actor** (2013 rva 0x6d5ba0): `IDisInteractableInterface::AttemptInteract( PlayerPawn )`. Nothing about the collection is
faked — only the "the crosshair is on it and the use button is down" decision is, and that is the one thing this package
could not reach (see the hand-over).

```
python resources\tools\build_and_smoke.py --build-dir build/agentAU --no-build --exe-name DishonoredGame_AU.exe ^
  --log-name agentAU_final.log --ini-dir build/agentAU/config --rhi d3d9 --timeout 180 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "inputtest moved" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Pub_Day_P -startmapopen -inputtest -dispickup -dispickupprobe -distouch -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

`agentAU_final.log`, 180 s:

```
dispickup L_Pub_Day_P: target list 0 -> 147 pickups
dispickup probe   1/147 DisElixirHealth_0      (DisElixirHealth):      CanBePickedUp 1 crosshair 2 AttemptInteract 1 travelling 1
dispickup probe   3/145 DisAbstractItemPickup_9 (DisAbstractItemPickup): CanBePickedUp 1 crosshair 2 AttemptInteract 1 travelling 1
dispickup after  inventory: ... items 1 [Coins_AbsItm=5 ]
dispickup probe   5/143 DisStatPickup_11       (DisStatPickup):        CanBePickedUp 1 crosshair 2 AttemptInteract 1 travelling 1
...
dispickup 174.8s L_Pub_Day_P: pickups 25 (collidable 23), interacts attempted 147
dispickup now inventory: health 100/100 mana 0/0 elixirs 0/0 ammo total 4 [0/10 0/5 0/10 4/10 0/3 ...]
                         items 4 [Coins_AbsItm=1874 SampleNote=5 SampleBook=20 RewireTool_AbsItm=1 ]
```

| | value |
|---|---|
| pickups on the map | 147 (`DisAbstractItemPickup`, `DisAbstractItemPickupNote`, `DisStatPickup`, `DisElixirHealth`, `DisKey_Base`, …) |
| probed | 147 |
| `AttemptInteract` returned TRUE | **122** |
| left in the world at the end | 25 |
| abstract items in the inventory | **4 kinds, 1,900 units** (coins 1,874, notes 5, books 20, rewire tool 1) |
| ammo in the inventory | **4 sleep darts** (`eDisAmmoType_Arrow_Sleep`), through `UDishonoredInventory::ConsumeStatPickup` |
| `Critical` | 0 |

The 25 refusals are correct behaviour, not failures: a pickup whose `DoInteract_Impl` gives nothing keeps
`bDestroyPickup && bTravel` FALSE and stays in the world (`ADisAbstractItemPickupNote` and `ADisKey_Base` are not ported
and inherit the base's `DoInteract_Impl`, which returns FALSE), and the census counts them as still collidable.

### The one bug that made the first run silently wrong, and how it was found

The very first probe run consumed 147 pickups and **the inventory never changed**. The cause is worth recording because
it will bite every future pickup / weapon / NPC port: `ADisPickup_Base` holds no tweaks pointer of its own — each
subclass does (`ADisStatPickup::m_pStatPickupTweaks`, `ADisAbstractItemPickup::m_pTweaks`,
`ADisElixirHealth::m_pPickupTweaks`, …) — and every body opens with
`GetTweaks_Derived()`-or-the-class-default. Without the per-subclass `GetTweaks_Derived` / `SetTweaks_Derived` override
every pickup read the **class default** of its tweaks class, whose item, ammo, health and mana amounts are all zero. The
pickups were therefore taken and destroyed while giving nothing. Nine lines per class fixed it; the census's
before/after inventory pair is what made the defect visible at all.

## What is ported (90 functions, `agentAU_status.csv`)

**`ADisPickup_Base`** (27 of its 42 attributed functions): `PostBeginPlay` 2013 rva **0x62b4c0**, `Tick` **0x62b130**,
`ShouldTrace` **0x61d820**, `GetCrosshairStatus` **0x631ba0**, `AttemptInteract_Derived` **0x631bf0**,
`AttemptCannotUseInteract_Derived` **0x62adc0**, `ConsumePickup` **0x61d770**, `StartPickupTravel` **0x62ab40**,
`Steal` **0x63f8d0**, `Attach` **0x62b0a0**, `Detach` **0x6226c0**, `BaseChange` **0x61dbb0** + `execBaseChange`
**0x5f93a0**, `AdjustDetachedPosition` **0x61d990**, `ClearComponents` **0x622750**, `OnActorTerminated` **0x62acd0**,
`PostLoad` **0x619a60**, `ApplyTweakChanges_Derived` **0x62aac0**, `GetInteractableTweaks_Derived` **0x62ad90**,
`IsUseBlockedByPossession` **0x62ad30**, `GetPlayerPawnFromUser` **0x62afe0**, `WantsTick_Derived` **0x619be0**,
`OnRigidBodyStatusChange` **0x61d6e0**, `IgnoreBlockingBy` **0x61d6a0**, `setPhysics` **0x62b440**, `HasSoul`
**0x62af90**, `GetMovableWeightClass` **0x62b060**, `TakeDamage_Impl` **0x619a90**, `CanSplash` (2012 0x66c820 only).

**`IDisInteractableInterface`** (14): the interface had no C++ API at all. `AttemptInteract` **0x63df60**,
`AttemptCannotUseInteract` **0x63e200**, `GetInteractableTweaks` **0x621550**, the four text accessors (**0x6280a0**,
**0x628190**, **0x6281c0**, **0x6281f0**), `FormatText` **0x61c8c0**, `DoHighlight` **0x627ff0**, `UnDoHighlight`
**0x628060**, `SetHighlightBit` **0x630710**, `ClearHighlightBit` **0x630740**, `GainCrosshairFocus` **0x6307e0**,
`WitnessInteraction` **0x63b560**. Every virtual has retail's own default body, so the thirty-odd classes that implement
the interface and are still comment-only skeleton units keep compiling and behave as retail's base does.

**The pickup subclasses** (22): `ADisStatPickup` (11, the coin / ammo / potion pickup, including `DoInteract_Impl`
**0x6367a0**, `CanBePickedUp` **0x636980** and `Explode` **0x636af0**), `ADisAbstractItemPickup` (7),
`ADisElixirHealth` / `ADisElixirMana` (5), `ADishonoredInventoryPickup` (3), `ADisGenericPickup` (2).

**`UDishonoredInventory::ConsumeStatPickup`** **0x805440** — the inventory half of a stat pickup, on top of agent AJ's
`AddAmmo`.

**The three natives the coordinator's correction left on the walking path that could be ported**:
`ADishonoredPlayerController::HandleHeldButtons` **0x6ba9a0** (+ `_Context` **0x6ba900** and `_Context_Interactables`
**0x6a2e70**), `Dis_Zoom` **0x6a2e40**, `ADishonoredGameInfo::GameEnding` **0x5f9f60**.

**Five more natives outside the pickup path** that are self-contained: `ADishonoredKActor::TakeDamage_Native`
**0x621010** (which is what reaches the ported `TakeDamage_Impl`), `ADishonoredKActor::DestroyIfPlayerCantSeeMe`
**0x6185f0**, `ADishonoredKAsset::TakeDamage_Native` **0x6211d0**, `UDisParticleSystemComponent::InitializeForPool`
**0x5fa8a0**, `ADisProjectile::TakeDamage_Native` **0x83c940**.

**Twelve utilities** the pickup path calls and that were comment-only: `DisGetGFxHUD` **0x7bf730**,
`DisIsBendTimeOn` / `DisIsBendTimeFrozen` **0x7bf2c0** / **0x7bf2f0**, `DisPullFromBendTime` **0x7bec00**,
`DisGetPawnInstigator` **0x7bf060**, `DisGetValidInstigator` **0x7bf010**, `DishonoredSpawnEmitter` **0x7cf970**,
`DisDestroyActorNextTick` **0x7bafa0**, `DisFireKismetEvent` **0x7c7b30**, `DisIsObjectStateGoingToBeRestored`
**0x7ec6a0**, `FDisPhysicsUtil::PhysObjectShouldTraceCommon` **0x7e88b0**, `ADishonoredKActor::PropagateMaxDrawDistance`
**0x6210b0**.

### Retail details worth keeping

* A pickup's **light environment** is what carries "this thing moves": `PostBeginPlay`, `ConsumePickup`,
  `StartPickupTravel` and `Tick` all set `LightEnvironment->bDynamic` (`UDynamicLightEnvironmentComponent` @188 mask
  0x10) beside `SetTickIsDisabled`. The Hex-Rays output reads `*(component + 188) |= 0x10` and is unreadable until the
  offset is named.
* A moving pickup drops its **precomputed-visibility id**: `Tick`'s tail sets `UPrimitiveComponent::VisibilityId` (@304)
  to `INDEX_NONE` and reattaches, gated on `UActorComponent` @76 bit 0x10 (`bIsPrimitiveComponent`, i.e. exactly the
  `Cast<UPrimitiveComponent>`) — not on `bAttached`.
* `ShouldTrace` uses agent AQ/AS's trace mask: a consumed pickup and a **non-pawn movement trace** (`0x40000000`, the
  seventh `FDisPrimTraceMask` bit, `m_bTraceForMove_NonPawn`, which `Engine/Inc/UnLevel.h` does not name yet) see
  nothing; a pickup based on an NPC only answers `TRACE_DisTouchOverlap | TRACE_Visible`; a held pickup answers neither
  the melee/vision gameplay traces nor visibility.
* `AttemptInteract_Derived` asks **every `UDisSeqEvent_PickupPickedUp`** on the actor, and a single one whose
  `m_bDestroyPickup` is FALSE keeps the pickup in the world while still firing the event. The travel only starts when
  `DoInteract_Impl` succeeded **and** the tweaks' `m_bDontDestroyOnPickup` is clear.
* The travel is a quadratic ease toward `PlayerCamera->CameraCache.POV.Location` raised by the tweaks'
  `m_TravelCameraOffset` and offset by the pickup's own bounds centre, driven by `m_fPickupTravelTime` counting down
  from `m_fTravelToUserTime`; `ConsumePickup` hides the actor and sets
  `m_bPendingDestructionAfterOneFullTickCycle`, and the *next* `Tick` destroys it.
* `ADisPickup_Base::Steal` is literally `AttemptInteract` plus a stat.
* `ADisStatPickup::PostBeginPlay` rolls each of the **twelve** ammo types out of its own `FDisRangedInt` range, so two
  instances of the same coin pile hold different amounts. `eDisAmmoType_MAX` is 12 in retail and 8 in the 2012 build —
  another reason to decode against the 2013 layout.
* `ADisAbstractItemPickup::GetUseMessage` returns the tweaks' `m_SingleItemPickupMessage`. In 2012 that member sits at
  @232; in 2013 `m_bCountForStats` was inserted at @232 and the string moved to @236. Porting the 2012 body against the
  2013 layout by offset would have read a bitfield as an `FString`.

## Why 9 natives and not 120

Agent AJ ported 7 and explained why; this package can now put numbers on it. Of the 275 remaining stubs, 236 have an
identifiable virtual behind the exec in the 2012 PDB (`build/agentAU_work/cost.py` → `cost.txt`, the work list ordered
by the real body size rather than by AC's ICF-folded callee, which is wrong for most rows):

| group | rows | what the bodies need |
|---|---|---|
| `UDisBehavior*` / `UDisDLC0?Behavior*` AI callbacks | 150 | `UDisAISubState`, `UDisAISubProcess`, `UDishonoredAIBrain` |
| `ADishonoredNPCPawn` / `ADishonoredNPCController` | 37 | the brain, `FArkComponentLocomotion` |
| `ADishonoredPawn` | 19 | `UDisItemContext`, the contact system, the attributes system |
| `UDisGlobalMusicManager` ticks | 5 | the five `UDisMusicState_*` classes |
| everything else (actors, components, Kismet variables) | 30 | one small subsystem each |

**76 of the 236 have a body of 64 bytes or less**, so the exec wrappers and the bodies really are cheap — and that is
precisely the trap. A `UDisBehaviorGuard::RequestStateExitCallback_TakePosition` of 61 bytes is a real retail body, but
it is only ever called by the AI sub-state machine, which does not exist; porting it would delete a `DISHONORED(bringup)`
line that today tells the reader "the AI is not ported" and replace it with code that never runs. Hitting 120 that way
would make `run_regression.py`'s unported-native counter look good while the map got no healthier. The nine natives in
`DishonoredGameNativeStubs.ported.agentAU.txt` are the ones whose callers exist and whose effects are observable in the
log.

The honest shape of the remaining work is therefore: **one root per package**, and each root then unlocks 20–150 natives
in a single, verifiable sweep. Costings follow.

## The dependency roots, costed (this is the hand-over agent AJ's item 6 needed)

Every rva is 2013. Sizes are the retail function sizes, so the numbers are the real cost.

**1. The attributes system — the cheapest root, and agent AJ's list does not name it.** `IDisAttributesInterface` is a
base of `ADishonoredPawn`, and `GetAttributeValue` is read by the fall-damage native, the stat-pickup potency bonuses,
the abstract-item value bonus, the zoom levels and the elixir caps. The whole read/write core is ~1.9 KB:

| function | 2013 rva | size |
|---|---|---|
| `UDisAttributes::GetAttributeValue` | 0x8864d0 | 94 |
| `UDisAttributes::GetModifiedAttribute` | 0x889040 | 215 |
| `UDisAttributes::RecacheAttributeValue` | 0x8863f0 | 222 |
| `UDisAttributes::RefreshAttributes` | 0x889bc0 | 246 |
| `UDisAttributes::TickAttributes` | 0x886530 | 508 |
| `UDisAttributes::HasModifier` / `GetModifierValue` / `RemoveModifier` | 0x87e0e0 / 0x87e140 / 0x885280 | 83 / 123 / 97 |
| `IDisAttributesInterface::GetAttributeValue` / `HasAttributeModifier` / `GetAttributeModifierValue` / `RemoveAttributeModifier` / `AddAttributeModifier` | 0x886a50 / 0x87e1c0 / 0x87e1e0 / 0x8852f0 / 0x889170 | 25 / 29 / 29 / 29 / 109 |
| `ADishonoredPawn::GetAttributes` | 0x7497b0 | 7 |
| `UDisTweaks_Attributes::ConstructAttributes` | 0x88aca0 | 94 |

They are plain `TMap<FName, FDisModifiedAttribute>` lookups over two reflected members and all three structs
(`FDisModifiedAttribute`, `FDisAttribute`, `FDisAttributeModifier`) are already generated. The modifier semantics are in
`RecacheAttributeValue`: type 0 adds `m_fValue`, type 1 adds `m_fValue * 0.01 * m_fAttributeBaseValue`, type 2 overrides,
then `m_bEnforceRange` clamps into `m_fMinRange` / `m_fMaxRange`.
**The catch, and why this package stopped:** `UDisTweaks_Attributes` has **no reflected properties at all** in
`retail_sdk_layout.json`, so `RefreshAttributesFromSource` (2012 rva 0x8f8130, 496 bytes) reads native members the
generated class does not have. Whoever takes this root must first give `UDisTweaks_Attributes` its native members from
the 2012 PDB type, the way agent S filled native gaps. Until then `GetAttributeValue` would answer 0 for everything,
which is *worse* than not porting it: `ADishonoredPawn::TakeFallingDamage_Native` divides by (max − min) and would kill
the pawn on every landing.

**2. `UDishonoredAIBrain` + `UDisAISubProcess` / `UDisAISubState`** — unlocks the 150 behaviour callbacks and most of the
37 NPC natives. Unchanged from agent AJ's assessment; the 8-to-70-byte callbacks in `cost.txt` are the sweep to do
immediately after, in one package, so they land with a subsystem that can call them.

**3. `UDisItemContext`** — unlocks `ADishonoredPawn`'s remaining 19, `UDishonoredInventoryItem::StopZoom` /
`ToggleZoom` (which is all that is missing from the ported `Dis_Zoom`), and the equip half of
`ADishonoredInventoryPickup::DoInteract_Impl`.

**4. The contact system** (`UDisContactType_*`, `FDisContactContext`, `DisGetPhysicalMaterial`,
`DisConvertCheckResultToImpactInfo`) — the whole body of `ADishonoredPawn::Landed_Native` (0x74d120, 508 bytes) and the
tail of `TakeFallingDamage_Native` (0x74dcc0, 731 bytes). These two are the **last two warn-once lines on the walking
path** and the last `-strictnatives` abort, so this is the smallest package that closes that accept line. The player-pawn
overrides on top are trivial (`ADishonoredPlayerPawn::TakeFallingDamage_Native` 0x6a4fa0 is 84 bytes: an NPC floor means
no fall damage; `Landed_Native` 0x6b53a0 is 1,601).

**5. `FArkComponentLocomotion`** — move targets; unchanged.

## The real input path: what is left between the probe and the player pressing F

The probe stands in for exactly two steps, and both are small and now fully mapped:

1. **The crosshair.** `ADishonoredPlayerController::UpdateCrosshair` (0x6b7b80, 418 b) →
   `SetCrosshairActor` (0x6a6280, 450 b) → `GetPlayerCrosshairTarget` (0x6a5ff0, 220 b) fill `m_pCrosshairActor` @1692,
   `m_CrosshairStatus` @1598 (which is what the ported `GetCrosshairStatus` feeds) and `m_bCanInteractWithCrosshairActor`
   @1552 mask 0x2000. `CanInteractWithInteractable` (0x6a2dc0, 128 b) is the distance gate and reads the interactable
   tweaks' `m_fMaxDistance`, which this package ported access to.
2. **The use-interaction FSM.** `m_pUseInteractionFSM` @1640 is never created, because
   `ADishonoredPlayerController::PostBeginPlay` does not call `UDishonoredNativeStateMachine::InitFSM` (agent AJ's note
   in that body) and the `UDisUseState_*` classes are unported. The machinery itself **is** ported — agent AJ's native
   FSM — so the work is the five states and their parameter structs: `UDisUseState_WaitForInput::TickState` 0x6d5ba0
   (641 b, the function that calls `AttemptInteract`), `UDisUseState_AltInteract::TickState` 0x6d57e0 (389 b) and
   `OnEnterState` 0x6c69b0, `UDisUseState_Holster::TickState` 0x6d5a00 (416 b) and `OnEnterState` 0x6cca00,
   `UDisUseState_Finished::TickState` 0x6d5970 (140 b), plus `FDisUseState_WaitForInput_Param`,
   `FDisUseState_AltInteract_Param` (ctor 0x6d5790) and `FDisUseState_Holster_Param`.
   `WaitForInput::TickState` gates on `m_bUseButton` @1587, so the input side needs nothing new.

That is one package of about 2 KB of code, and at the end of it the player presses the use key and picks a coin up.

## Instrumentation

**`-dispickup`** — once a second, per world, re-armed for each new world: the number of pickups on the map, how many are
still collidable, how many interacts have been attempted, and the player's whole inventory (health, mana, both elixir
counts, all twelve ammo counts with their capacities, and every abstract item by name and quantity). The target list is
**re-collected every second**, because the pickups live in streamed sub-levels that are added seconds after the
persistent level (`L_Pub_Day_P`: 0 pickups at 0.4 s, 147 at 7.1 s) — the first version of the census armed once and
reported 0 for the whole run.

**`-dispickupprobe`** — after the `-inputtest` walk has finished (14 s in), one pickup per second: move the pawn to it,
log the inventory, ask `CanBePickedUp` and `GetCrosshairStatus`, call `AttemptInteract`, log the inventory again. The
before/after pair is the whole point: it is what caught the class-default-tweaks defect.

Both hang off the ported `HandleHeldButtons`, which script calls every frame from `PlayerTick` — no engine file was
touched for them.

## Runs

Build: `cmd /c build\agentAU_release.cmd` (Release, snapshot worktree `build/agentAU_wt` → `build/agentAU`), **0 errors,
0 unresolved symbols**, 805 units (`build/agentAU_build{0..6}.log`).

| log | what | result |
|---|---|---|
| `agentAU_final.log` | `L_Pub_Day_P` 180 s d3d9, `-inputtest -dispickup -dispickupprobe -distouch` | 147 probed / 122 collected, inventory as above, 0 `Critical` |
| `agentAU_tower.log` | `L_Tower_P` 130 s d3d9, `-inputtest -dispickup -distouch` | `inputtest moved 1021.6` physics 1, `touching now 11 actors / 16 pairs`, 0 `Could not create new Shape`, 0 `Critical` |
| `agentAU_strict.log` | `L_Tower_P` null RHI, `-strictnatives` | mission map reached, `Initial startup` 19.84 s, one abort at 22.54 s: `ADishonoredPlayerPawn::execTakeFallingDamage_Native` |
| `agentAU_plain.log` | no `-startmap`, null RHI | `Initial startup`, 0 `Critical` |
| `agentAU.log` | the package brief's own accept command (`L_Pub_Day_P`, `-distouch -distouchprobe`) | `Initial startup`, **0 `Critical`**; 4 warn-once natives left (`execTakeDamage`, `execCalcPlayerSwimAccelRate`, `execLanded_Native`, `execTakeFallingDamage_Native`) |
| `agentAU_base.log` | the same `L_Pub_Day_P` command on HEAD `bb005ec` before this package | 4 warn-once natives, and a `Critical` (see the follow-up below) |

## Follow-ups outside this package

1. **`AActor::eventTouch` / `eventUnTouch` appError on retail classes that declare no such event.** On
   `L_Pub_Day_P`, as soon as `DishonoredMovable_68` begins a touch with `DishonoredAudioVolume_58`, the baseline run
   dies with `Critical: appError called: Failed to find function Touch in DishonoredAudioVolume`. The generated
   `eventTouch` wrapper guards only with `IsProbing(NAME_Touch)` and then calls `FindFunctionChecked`, which appErrors;
   retail cannot be doing that, so the class simply has no such event and the notification is a no-op.
   `Engine/Src/UnActor.cpp` is agent AS's file, so the guard is **applied to my snapshot only**, through
   `python build/agentAU_work/patch_toucheventguard.py build/agentAU_wt` (it adds one `FindFunction` test and names each
   class once). **This must land somewhere: it is a hard crash on the hub map with agent AS's touch work merged**, and
   it is not reproducible from `L_Tower_P`, which is why agent AS did not see it. Thirteen retail classes hit the guard on
   `L_Pub_Day_P` alone: `DishonoredAudioVolume`, `DishonoredWaterVolume`, `PhysicsVolume`, `DisPossessionVolume`,
   `DisTrigger`, `InterpActor`, `DishonoredMovable`, `DishonoredPlayerPawn`, `DisClimbable`, and the four pickup classes.
2. **`Engine/Inc/UnLevel.h` has no name for the seventh trace bit.** `0x40000000` is
   `m_bTraceForMove_NonPawn` (the third of `FDisPrimTraceMask`'s three move bits; agent AQ named the other two as
   `TRACE_DisMove_NonPlayerPawn` / `TRACE_DisMove_Player`). `ADisPickup_Base::ShouldTrace` reads it, and so will every
   other `ShouldTrace` override. `0x00400000` and `0x80000000` in its `0x8C401000` mask also have no reader yet.
3. **`ADishonoredPlayerPawn::IncrementStat`** (0x6b9560, 1,050 b) is unported, so nothing scores
   `ePlayerStat_ItemsCollected`, `ItemsStolen`, `GoldFound` or `RuneFound`. Four ported bodies name it.
4. **`UDisGFxMoviePlayerHUD`** — `AddUseMessage`, `OnAmmoPickedUp`, `OnAbstractItemPickedUp`, `AddHeartMarker` and
   `SetGameMessage` are all reached by the ported path and all GFx. Under `-dispickup` the use message is logged instead
   (`dispickup use message: …`). This is a concrete input for agent AW's decision: the pickup path is the first place
   where the absence of the HUD is *felt* rather than merely unrendered.
5. **`ADishonoredPawn::IsPossessing` / `DisIsPossessed` / `GetPossessingPlayerPawn`** gate
   `IsUseBlockedByPossession` and `GetPlayerPawnFromUser`'s possessed-pawn branch; both are ported with the branch
   documented, so a pickup is never possession-blocked yet.
6. **`AActor::m_ActorTypeFlags`** (BYTE @266) is still never written (agent AS's follow-up 3). Retail's
   `GetPlayerPawnFromUser`, `Detach` and `BaseChange` all test it (36 = the player pawn, 34 / bit 0x20 = an NPC pawn);
   the ports use the reflected `Cast<>` instead, which is equivalent but slower.
7. **`build/agentAU_work/sym.py` and `off.py`** belong in `resources/tools/`. Every remaining DishonoredGame port has to
   translate a 2012 skeleton banner into a 2013 rva and a 2013 byte offset into a member name, and these two do exactly
   that in one line each.

## Files

**Mine** (33 source + 2 docs). Generated files were **not** touched in the shared tree; the module is regenerated in the
snapshot only, by `build/agentAU_work/sync_build.py`.

**Coordinator, at merge:** the shared tree will not compile these units until `gen_classes_header.py --sdk` has been run
for DishonoredGame there. Four of the units this package extends are already on the shared `Sources.cmake` compile list
(`dishonoredplayercontroller.cpp`, `dishonoredgameinfo.cpp`, `dishonoredinventory.cpp`,
`dishonoredutilities_accessors.cpp`) and their new code needs the regenerated class headers (the twelve new
`Inc/CppText/` files) and the fifteen units this package gives code to taken off the exclude list. That is the same
regeneration agent AJ's report describes, and `build/agentAU_work/sync_build.py` is exactly it with the output pointed at
a worktree.

* new hand-written `Inc/CppText/`: `IDisInteractableInterface.h`, `ADisPickup_Base.h`, `ADisStatPickup.h`,
  `ADisAbstractItemPickup.h`, `ADisElixirHealth.h`, `ADisElixirMana.h`, `ADisGenericPickup.h`,
  `ADishonoredInventoryPickup.h`, `ADishonoredKActor.h`, `ADishonoredKAsset.h`, `ADisProjectile.h`,
  `UDisParticleSystemComponent.h`; extended `ADishonoredPlayerController.h`, `ADishonoredGameInfo.h`,
  `UDishonoredInventory.h`
* `Inc/dishonoredutilities.h` (declarations + `FDisPhysicsUtil`)
* `Src/`: `dispickup_base.cpp`, `disinteractableinterface.cpp`, `disstatpickup.cpp`, `disabstractitempickup.cpp`,
  `diselixir.cpp`, `disgenericpickup.cpp`, `dishonoredinventorypickup.cpp`, `dishonoredinventory.cpp`,
  `dishonoredplayercontroller.cpp`, `dishonoredgameinfo.cpp`, `dishonoredkactor.cpp`, `dishonoredkasset.cpp`,
  `disprojectile.cpp`, `disparticlesystemcomponent.cpp`, `dishonoredutilities.cpp`,
  `dishonoredutilities_accessors.cpp`, `dishonoredutilities_saveload.cpp`, `dishonoredutilities_physics.cpp`
* `DishonoredGameNativeStubs.ported.agentAU.txt` (**9 natives**)
* this report and `agentAU_status.csv` (90 rows)

**Scratch** (not repo tools): `build/agentAU_work/` — `regen.py` re-emits every ported file in order
(`write_pickup.py`, `write_rest.py`, `write_pc.py`, `fix1.py`, `fix2.py`, `write_pickup_subs.py`, `write_natives.py`),
`sync_build.py` syncs the 35 files into the snapshot and regenerates the module there, `sym.py` / `off.py` / `cost.py`
are the analysis helpers, `patch_toucheventguard.py` is the snapshot-only Engine guard of follow-up 1, `l13_*.txt` /
`l12_*.txt` are the decompile lists. Decompiles in `build/agentAU_decomp/{r13,s12}` (127 functions). Build logs
`build/agentAU_build{0..6}.log`, snapshot worktree `build/agentAU_wt` + build dir `build/agentAU`, IDA copies
`resources/docs/idb/retail2013_agentAU.i64` and `shipping2012_agentAU.i64`. Logs in the retail `DishonoredGame/Logs`:
`agentAU_base.log`, `agentAU_p1.log`, `agentAU_p2.log`, `agentAU_p3.log`, `agentAU_final.log`, `agentAU_tower.log`,
`agentAU_strict.log`, `agentAU_plain.log`.

## Commands

```
python build/agentAU_work/regen.py                      # re-emit every ported file (idempotent)
python build/agentAU_work/sync_build.py                 # sync into build/agentAU_wt + regenerate the module there
python build/agentAU_work/patch_toucheventguard.py build/agentAU_wt   # snapshot-only Engine guard (follow-up 1)
cmd /c build\agentAU_release.cmd                        # Release build of the snapshot into build/agentAU
python build/agentAU_work/sym.py "DisPickup_Base@@"     # 2012 + 2013 rvas of a mangled-name pattern
python build/agentAU_work/off.py ADisPickup_Base 904    # name the member at a retail byte offset
python build/agentAU_work/cost.py                       # the 295 remaining stubs ordered by real body size
python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py resources/docs/idb/retail2013_agentAU.i64 build/agentAU_decomp/r13 rva:0x62b4c0
```

`resources/play.cmd` and `resources/build-release.cmd` were not touched. No commits, no `git add`.
