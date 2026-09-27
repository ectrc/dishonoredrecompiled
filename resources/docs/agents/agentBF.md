# Agent BF — the attributes system, and the walking path's last unported natives (2026-09-27)

Package BF of `PHASE8.md`. **Done, and the accept line is met exactly**: the attributes system is ported against the
cooked data, `ADishonoredPlayerPawn::execLanded_Native` and `execTakeFallingDamage_Native` are ported, the harness metric
`unported_natives` reads **0** (2 at HEAD `24d8b8d`), and the pawn survives the intro fall on `L_Tower_P` taking **70 of
100 health** instead of dying:

```
[0008.61] disattrib fall   DishonoredPlayerPawn: speed 2709.0 in [2250.0 .. 2900.0] alpha 0.706 -> damage 70 of health 100/100
[0008.61] disattrib damage DishonoredPlayerPawn: 70 of type DmgType_Fell from none, health 100 -> 30
[0011.63] inputtest moved 1024.8 turned 7280 (peak 2D speed 500.0, physics 1)
```

That run also carries `-strictnatives`, so the fall happens with the abort-on-stub switch on and nothing aborts. Agent AU's
equivalent run died at 22.54 s on `execTakeFallingDamage_Native`.

Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentBF.i64`, a copy of `retail2013_named.i64`); every
"2013 rva" is a headless decompile or disassembly in `build/agentBF_decomp/r13`, and the 2012 rva in brackets is
`shipping2012_agentBF.i64` (`build/agentBF_decomp/s12`), which carries the symbol names. Member offsets were resolved
numerically against `resources/docs/types/retail_sdk_layout.json` with `resources/tools/ida/off.py`.

## Result against the accept line

| Acceptance | State |
|---|---|
| the attributes system ported | **done.** 74 attributes on the player pawn, read out of the cooked `DisTweaks_PlayerPawn_Attributes_1` — `Attribute_HealthMax` 70, `GroundSpeed` 400, `JumpZ` 800, `MaxSpeedBeforeFallingDamage` 2250, `MaxSpeedBeforeFallingDeath` 2900, `Attribute_Visibility` with its enforced range [-1 .. 1] |
| `execLanded_Native` + `execTakeFallingDamage_Native` | **done**, on `ADishonoredPawn`, `ADishonoredPlayerPawn` and (falling damage) `ADishonoredNPCPawn` |
| `unported_natives` reaches 0 | **done: 0.** It needed one native more than the plan names — see "The third native" |
| the pawn survives a fall with a sensible damage value | **done**, numbers above. 100 → 30 health, then it walks 1,024 units |
| `run_regression.py --build-dir build/release --no-build` green | **done: 19 ok, 0 failed, 4 skipped, 422 s.** Also run against this package's own build: **19 ok, 0 failed** |
| 0 critical errors | **done** on every run (d3d9, null RHI, `-strictnatives`, both regression sweeps) |
| no walking regression | **done.** `inputtest moved` 1,020.4 / 1,024.8 / 1,014.9 across runs (HEAD 1,029.6, bound >= 800), peak speed 500.0, physics 1, 895 PhysX actors, 1,312 static shapes |

## Agent AU's trap was a false alarm, and the real shape is better

Agent AU's hand-over said `UDisTweaks_Attributes` "has no reflected properties at all in `retail_sdk_layout.json`, so the
refresh path reads native members the generated class does not have", and that those members had to come from the 2012 PDB
type first. **It has no members at all, native or reflected.** Retail `sizeof(UDisTweaks_Attributes)` is 140, which is
`sizeof(UDisTweaksBase)`, and the 2012 PDB type agrees: the class adds nothing.

`UDisTweaks_Attributes::RefreshAttributesFromSource` (2012 rva 0x8f8130) is **entirely reflection-driven**. It walks its
own `Class` with a `TFieldIterator<UStructProperty>`, and for every property whose struct is named `DisAttribute` or
`DisAttribute_RangeLimits` it takes the address `(BYTE*)this + Property->Offset` into one of two arrays, then hands both to
`UDisAttributes::RefreshAttributes`. The properties it finds belong to the *subclass* — `UDisTweaks_Pawn_Attributes` has
seventy-odd of them and every one is already generated. So the blocker agent AU stopped on did not exist, and no PDB
member filling was needed at all. (The Hex-Rays output makes this hard to see: it types the field iterator's
`UStructProperty` as `UField[1]` and prints `UProperty::Offset` as `_LinkerIndex` and `UStructProperty::Struct` as
`Class`.)

`UDisTweaks_Attributes::Serialize` (2012 rva 0x8f0d30) does nothing on a 2013 package: its whole body is gated on
`ArIsLoading && ArLicenseeVer < 25`, where it copies each `FDisAttribute`'s pre-difficulty `m_fBaseValue` into all four
per-difficulty values. It is not ported and does not need to be.

## What is ported (35 rows, `agentBF_status.csv`)

**`UDisAttributes`** — the read/write core, `disattributes.cpp`: `GetAttributeValue` **0x8864d0**,
`GetModifiedAttribute` **0x889040**, `RecacheAttributeValue` **0x8863f0**, `RefreshAttributes` **0x889bc0**,
`TickAttributes` **0x886530**, `HasModifier` **0x87e0e0**, `GetModifierValue` **0x87e140**, `RemoveModifier`
**0x885280**.

**`IDisAttributesInterface`** — `disattributesinterface.cpp`: `GetAttributeValue` **0x886a50**, `HasAttributeModifier`
**0x87e1c0**, `GetAttributeModifierValue` **0x87e1e0**, `RemoveAttributeModifier` **0x8852f0**, `AddAttributeModifier`
**0x889170**, plus the `GetAttributes()` slot itself.

**`UDisTweaks_Attributes`** — `distweaks_attributes.cpp`: `ConstructAttributes` **0x88aca0**,
`RefreshAttributesFromSource` (2012 **0x8f8130**).

**The pawn's half** — `dishonoredpawn_attributes.cpp`: `GetAttributes` **0x7497b0**, `PreBeginPlay_Attributes`
**0x762190**, `OnDifficultyChange` (2012 0x794cd0).

**The natives** — `ADishonoredPawn::TakeFallingDamage_Native` **0x74dcc0** + exec **0x5ec5f0**
(`dishonoredpawn_body.cpp`), `ADishonoredPawn::Landed_Native` **0x74d120** + exec **0x5ec7e0**
(`dishonoredpawn.cpp`), `ADishonoredPlayerPawn::TakeFallingDamage_Native` **0x6a4fa0** + exec
(`dishonoredplayerpawn_body.cpp`), `ADishonoredPlayerPawn::Landed_Native` **0x6b53a0** + exec
(`dishonoredplayerpawn.cpp`), `ADishonoredNPCPawn::execTakeFallingDamage_Native` (`dishonorednpcpawn_body.cpp`),
`ADishonoredPawn::TakeDamage` **0x74a340** + `execTakeDamage` (`dishonoredpawn_health.cpp`).

### Retail details worth keeping

* **The modifier semantics** (`RecacheAttributeValue`): start at `m_fAttributeBaseValue`, then per modifier
  `eDisAttributeModifierType_AddVal` adds `m_fValue`, `_AddBasePercent` adds `m_fValue * 0.01 * m_fAttributeBaseValue`
  (a percentage **of the base**, not of the running total, so modifier order does not matter for it), `_SetVal`
  overrides; then `m_bEnforceRange` clamps into `[m_fMinRange, m_fMaxRange]`. The result is cached in
  `m_fCachedModifiedValue` and `m_bCachedValueIsDirty` (`FDisModifiedAttribute` @12 masks 0x1 and 0x2) is cleared.
  `GetAttributeValue` is a `const` method that recaches through the `const` pointer.
* **The pawn's attribute tweak object is `m_pAttributeTweaks[1]`**, not `[0]`. `UDisTweaks_Pawn::m_pAttributeTweaks` is a
  four-element array at @260 and `PreBeginPlay_Attributes` hard-codes `[eax+108h]` = @264 in both builds (2012 0x79b76d,
  2013 0x7621cd). Nothing in either exe reads @260, @268 or @272 (`disp.py` over both displacements finds only this one
  reader), because the per-difficulty variation lives *inside* each `FDisAttribute`, not across the array.
* **The attribute names are data, not code**: `FDisAttribute::m_Name` comes out of the cooked defaults and is the tweak
  member's name with `m_` replaced by `Attribute_` (`Attribute_MaxSpeedBeforeFallingDamage` is at 0xcd25e0 in the retail
  exe). Nothing in the engine derives them, so a reader must spell them exactly.
* **The difficulty picks one of four base values** inside `FDisAttribute`: @12 Easy, @16 Normal, @20 Hard, @24 VeryHard.
  `m_fBaseValue` @8 is the pre-difficulty legacy value and is never read outside the licensee-version-25 upgrade.
  `ADishonoredGameInfo::GetDifficulty` (2012 rva 0x62fbf0) is one byte read of `m_Difficulty` @1092 in both builds.
* **`RemoveModifier` has a retail defect** and is ported as written: the test that prunes `m_AttributesWithModifiers`
  reads `m_ModifiedAttributes.Num()`, not the attribute's own `m_Modifiers.Num()`, and sits outside the found-branch
  (2013 0x8852c5: `[esi+4] - [esi+2ch]` with `esi = this+56 = m_ModifiedAttributes`). The whole map is never empty once a
  pawn has attributes, so the name stays in the list and `TickAttributes` prunes it on the next tick instead.
* **`ADishonoredPawn::Landed_Native` is the launch half of a jump, not the landing.** Its whole body is gated on
  `Velocity.Z > +200` (2013 0x74d134 `comiss` against 200.0, `jbe` away), i.e. a "landing" that is still moving upwards.
  A real fall never enters it.
* **The fall-damage arithmetic** (`TakeFallingDamage_Native`): `FallSpeed = -Velocity.Z`; the two attribute thresholds are
  sorted, so the smaller is the no-damage floor and the larger the lethal ceiling regardless of how they were authored;
  `DamageToTake = (INT)(HealthMax * Clamp((FallSpeed - Min) / (Max - Min), 0, 1))`. `HealthMax` is an **INT** at APawn
  @840 (`cvtsi2ss`), not a float.
* **The player's override is one test**: landing on a pawn costs nothing (`FloorActor->m_ActorTypeFlags & 0x20`, the bit
  both pawn kinds carry — an NPC is 34, the player 36). `m_ActorTypeFlags` is still never written in this tree (agent AU
  follow-up 6), so the equivalent `Cast<ADishonoredPawn>` is used, as agent AU's ports do.

## The division agent AU warned about, and the guard that replaces it

Agent AU was right that this is the whole reason the two natives were stubbed: with both thresholds 0 the division
`(FallSpeed - Min) / (Max - Min)` divides by zero, the alpha clamps to 1 and the pawn takes `HealthMax` damage on every
landing. With the attributes ported the thresholds are 2250 and 2900 and the arithmetic is retail's own — but a pawn whose
attribute set is missing would still hit it, so `TakeFallingDamage_Native` refuses the degenerate `Max <= Min` case,
returns 0 and names the pawn once. Two more guards of the same kind, each marked `DISHONORED(bringup)` at its site:
`PreBeginPlay_Attributes` falls back to the class default when `m_pAttributeTweaks[1]` is missing (retail dereferences it
unchecked), and `TickAttributes` tests the map lookup retail writes into blind.

Following agent AU's instruction to instrument before trusting a result, the very first thing built was the census, not
the natives — and it is what proved the tweak object is the real cooked `DisTweaks_PlayerPawn_Attributes_1` and not the
class default whose every value is zero. That defect consumed 147 pickups for agent AU; here it would have killed the
pawn on every landing and looked exactly like a faithful port.

## The third native: the accept line needed one more than the plan names

`unported_natives` did not go 2 → 0. It went 2 → 1, because **porting the fall damage made a native fire that had never
fired before**: `ADishonoredPawn::execTakeDamage`. The stub returned 0 damage, so script never asked for the damage to be
applied; with 70 points coming back it does. That is the metric working as designed — a native counts only when it
actually runs — and the fix is small and correct: `ADishonoredPawn::TakeDamage` (2013 rva 0x74a340, 202 bytes) is the
Arkane pass over the damage followed by the engine's own `APawn::TakeDamage`, which is what subtracts it from `Health`.
The Arkane pass is `TakeDamage_Native` (2013 rva 0x75ccd0, 1,796 bytes: vulnerabilities, immunities, the minimum-health
floor, the HUD and camera hit reactions, the contact system) and is named as the gap; for a fall it contributes nothing,
so the damage reaches the engine unmodified and `Health` goes 100 → 30.

`ADishonoredNPCPawn::execTakeFallingDamage_Native` was ported in the same sweep (eight lines: the class re-declares the
native in script but has no C++ override, which is why retail's exec is ICF-folded onto `ADishonoredPawn`'s), so an NPC
falling cannot reintroduce the line.

## One shared-tool fix, and why it was necessary

`resources/tools/run_regression.py`'s `aggregate()` returned `None` for a `distinct` metric with no matches, and
`measure()` turns `None` into the `-1` regression sentinel. `unported_natives` is the only `distinct` metric, and its log
line exists *only* when something is wrong — so the metric could reach the plan's target and still print `-1` rather than
`0`. The `distinct` branch now runs before the empty-match guard (six lines including the `DISHONORED(written)` note).
Verified both ways: this package's build reports `unported_natives 0`, `build/release` at HEAD still reports `2`.

## What was deliberately NOT ported, and why

Three attributes functions exist, are cheap, and are left out on purpose. Each is named at its site with its rva:

* **`ADishonoredPawn::MaxSpeedModifier`** (2012 rva 0x794b10) is an `APawn` virtual the engine's movement code calls every
  frame. Its body needs `MaxSpeedModifier_Derived`, `IsSneaking` and `IsCarryingCorpse`, none of which exist, and a
  partial port would multiply the pawn's speed by 0 and stop it walking — the regression the accept line forbids.
* **`ADishonoredPawn::ApplyAttributes`** (2012 rva 0x794860) writes `GroundSpeed` / `WaterSpeed` / `AccelRate` from the
  attributes and the eight ammo capacities into `m_pInventory->m_AmmoInfo`, then `CrouchHeight` / `MaxFallSpeed` /
  `WalkableFloorZ` from the body tweaks. It needs `IsCarryingCorpse` too, and it would overwrite the movement values the
  walking path depends on the moment `Tick` is ported.
* **`ADishonoredPawn::Tick_Attributes`** (2012 rva 0x78e880) is `TickAttributes` + `ApplyAttributes`, and its caller
  `ADishonoredPawn::Tick` (2013 rva 0x750720) is unported. Nothing adds a timed modifier yet either — the only callers of
  `AddAttributeModifier` are `UDisTweaks_Upgrade::ApplyAttributes` (0x64f460) and two Kismet actions, none of them ported
  — so no modifier can need ageing.

This is agent AU's own rule applied to my own package: porting a body whose callers do not exist replaces a line that
tells the reader "this subsystem is missing" with code that never runs.

`m_pAttributes` is built lazily on the first `GetAttributes()` call rather than by `ADishonoredPawn::PreBeginPlay` (2013
rva 0x748f10), which is unported because it also needs `PreBeginPlay_NativeComponents`,
`UArkComponentContainer::StartAllComponents`, `PreBeginPlay_Inventory` and `PreBeginPlay_Actions`. Retail's
`PreBeginPlay_Attributes` body itself is ported and is what runs; only the trigger moved, and the one-line change is
marked at the site. Its `FArkGameEventDispatcher` registration of `OnDifficultyChange` on event 9 is left out because the
dispatcher (`Engine/Inc/arkgameeventdispatcher.h`) is a comment-only skeleton.

`ADishonoredPlayerPawn::Landed_Native` (1,601 bytes) is ported as its structure plus the one step that can run. Its five
steps are the big-fall stun action, the small-land action per stance and equipped item, the JumpLand contact, the camera
physical reaction, and `m_Debug_Player.m_LastJumpLandVel = Velocity`. Steps 1 to 4 stand on subsystems that do not exist:
`m_pPlayerMasterFSM` / `m_pPlayerUpperFSM` / `m_pPlayerLeftArmFSM` are never created (agent AJ's note:
`ADishonoredPlayerController::PostBeginPlay` does not call `UDishonoredNativeStateMachine::InitFSM`) and every
`UStatePlayerMaster*` / `UStateSharedActionBase*` unit is still a comment-only skeleton; `UDisTweaks_PlayerPawn_Actions`'
action structs, `ADishonoredPawn::GetDishonoredCamera`, `UDishonoredCamera_PhysicalReact` and the whole contact system are
unported too. Step 5 is ported for real.

## The contact system is the one remaining gap in these four natives

Agent AU's root 4 is exactly what is left, and it is named at three sites:
`DisGetPhysicalMaterial` (trace flags 0x20DF for the fall, 0x28DF for the land), `DisConvertCheckResultToImpactInfo`,
`DisGetContactSystem`, `UDishonoredContactSystem::ApplyContact`, and the contact types
`UDisContactType_{JumpLand, JumpLand_Big, JumpLand_Stealth, Environment, BreakingBones, BreakingBones_Player}`. None of
them exists in the tree (`dishonoredutilities_accessors.cpp` carries their banners only). What is missing is the *effect*
of a landing — sound, particles, camera shake — not the damage. `DisGetPawnFeet` (2013 rva 0x7baff0) is the only helper
those tails need that is trivial (`CylinderComponent->Bounds.Origin` with `BoxExtent.Z` subtracted, falling back to
`Location`); it is decoded in this report but deliberately not added, because its home
(`dishonoredutilities_accessors.cpp` + `Inc/dishonoredutilities.h`) belongs to agent AU's package and nothing of mine
needed it.

## Instrumentation

**`-disattrib`** — off by default, free when off (the command line is read once into a file-static). Four lines:

* on construction, the whole attribute set of a pawn: the tweak object it came from, the count, and per attribute its
  name, base value, enforced range and modifier count;
* on a fall, the fall speed, both thresholds, the alpha and the damage against the pawn's health;
* on the damage landing, the amount, the damage type, the causer and health before/after;
* on a landing, the velocity and floor actor, naming the contact/action/camera work that is not ported.

The first of those is what caught nothing this time precisely because it was written first — the data was right — and it
is the line to look at first if a future package finds an attribute reading 0.

## Runs

Build: `cmd /c build\agentBF_release.cmd` (Release, snapshot worktree `build/agentBF_wt` → `build/agentBF`), **0 errors,
0 unresolved symbols** (`build/agentBF_build{0..4}.log`; build0 and build1 caught a missing
`#include "dishonoredutilities.h"` and a non-idempotent emitter, both fixed).

| log | what | result |
|---|---|---|
| `agentBF.log` | `L_Tower_P` 120 s d3d9, `-inputtest -disattrib` | 74 attributes dumped, fall 2722.6 → damage 72, health 100 → 28, `inputtest moved 1010.0`, **0 `native not ported`**, 0 `Critical` |
| `agentBF_strict_d3d9.log` | the same with `-strictnatives` | fall 2709.0 → damage 70, health 100 → 30, `inputtest moved 1024.8`, no abort, 0 `Critical` |
| `agentBF_strict.log` | `L_Tower_P` null RHI, `-strictnatives` | `Initial startup 2.56s`, no abort, 0 `Critical` |
| `agentBF_reg_*.log` | `run_regression.py --build-dir build/agentBF` | **19 ok, 0 failed**, 4 skipped, `unported_natives 0`, `inputtest_moved 1020.4` |
| `regression_*.log` | `run_regression.py --build-dir build/release` (the stated gate) | **19 ok, 0 failed**, 4 skipped, `unported_natives 2` (HEAD) |

The four skips are pre-existing and not this package's: `coresmoke_passed` and `layout_types` need `CoreSmoke.exe` /
`LayoutProbe.exe`, which neither build dir builds; `touch_census` and `sequence_census` are the optional wave-5 counters
and no `DISHONORED(bringup): … census` line matching the harness regex appears in either build's inputtest log, including
`build/release` at HEAD. That is a follow-up for whoever owns those counters, not a change of mine.

## Files

**Mine** (17 source + 2 docs), all CRLF, every edit tagged with its 2013 rva. Generated files were **not** touched in the
shared tree; the module is regenerated in the snapshot only, by `build/agentBF_work/sync_build.py`.

* new `Inc/CppText/`: `UDisAttributes.h`, `IDisAttributesInterface.h`, `UDisTweaks_Attributes.h`,
  `ADishonoredNPCPawn.h`; extended `ADishonoredPawn.h`, `ADishonoredPlayerPawn.h`
* `Src/`: `disattributes.cpp`, `disattributesinterface.cpp`, `distweaks_attributes.cpp`,
  `dishonoredpawn_attributes.cpp`, `dishonoredpawn_body.cpp`, `dishonoredpawn.cpp` (appended),
  `dishonoredpawn_health.cpp`, `dishonoredplayerpawn_body.cpp`, `dishonoredplayerpawn.cpp` (appended),
  `dishonorednpcpawn_body.cpp`
* `DishonoredGameNativeStubs.ported.agentBF.txt` (**6 natives**)
* `resources/tools/run_regression.py` — the six-line `distinct` fix above. The one file outside the package, and the
  reason is in its own comment
* this report and `agentBF_status.csv` (35 rows)

Eight units leave `Sources.cmake`'s exclude list because they now hold code: `disattributes.cpp`,
`disattributesinterface.cpp`, `distweaks_attributes.cpp`, `dishonoredpawn_attributes.cpp`, `dishonoredpawn_body.cpp`,
`dishonoredpawn_health.cpp`, `dishonoredplayerpawn_body.cpp` and `dishonorednpcpawn_body.cpp` (verified against the
snapshot's regenerated `Sources.cmake`: excluded at HEAD, listed now). The generator works that out on its own from "is
the file comment-only", so nothing needs editing by hand.

**Coordinator, at merge:** `gen_classes_header.py --sdk` must be run for DishonoredGame in the shared tree, as for every
package that adds `Inc/CppText/` files — the three new attributes CppText headers have to be `#include`d into the
generated class bodies and `DishonoredGameNativeStubs.cpp` has to drop the six ported execs. Another package already
regenerated the shared module during this wave and it picked my CppText up, so the shared
`DishonoredGameAttributesClasses.h` may already carry the two includes; regenerate anyway.
**Also lower `regression_baseline.json`'s `unported_natives` bound from 4 to 0** — with this package merged the walking
path has none, and the bound is the only thing that would let one come back unnoticed.

**Scratch** (not repo tools): `build/agentBF_work/` — `w1.py` … `w6.py` re-emit every ported file idempotently,
`sync_build.py` re-syncs the snapshot and regenerates the module there, `disasm.py` prints a function's disassembly by
rva (the helper that settled the `m_pAttributeTweaks[1]` index and the `Velocity.Z > 200` direction), `findstr.py` finds a
string in an IDB, `files.txt` is the package's file list, `l12*.txt` / `l13*.txt` the decompile lists. Decompiles in
`build/agentBF_decomp/{r13,s12}` (58 functions). Snapshot worktree `build/agentBF_wt` + build dir `build/agentBF`, IDA
copies `resources/docs/idb/retail2013_agentBF.i64` and `shipping2012_agentBF.i64`.

## Follow-ups outside this package

1. **The attribute reads agent AU documented as zero are now live, and are three one-line changes in AU's files.**
   `disstatpickup.cpp`'s `DoInteract_Impl` names `Attribute_HealthPotionPotencyBonus` /
   `Attribute_ManaPotionPotencyBonus` (both 0 on an un-upgraded Corvo, so the behaviour will not change until an upgrade
   is applied, but the read is real now); `dishonoredinventory.cpp`'s `ConsumeStatPickup` names
   `Attribute_StatPickupCapacityBonusChance`; `disabstractitempickup.cpp` names the abstract-item value bonus. The eight
   ammo capacities (`Attribute_BulletCapacity` 10, `ArrowCapacity` 10, `GrenadeCapacity` 5, …) are in the map and are
   what `ApplyAttributes` would write into `m_AmmoInfo`.
2. **`UDisTweaks_Upgrade::ApplyAttributes` / `RevertAttributes`** (2013 rva 0x64f460 / 0x64f2a0, 554 + 446 bytes) are now
   unblocked: they are the only real callers of `AddAttributeModifier` / `RemoveAttributeModifier`, and with them a rune
   upgrade actually changes Corvo's numbers. That is the first place the modifier half of this package will be exercised,
   and `TickAttributes` needs `ADishonoredPawn::Tick` ported alongside it so timed modifiers expire.
3. **`ADishonoredPawn::Tick`** (2013 rva 0x750720, 314 bytes) gates `Tick_Attributes`, `Tick_Health`, `Tick_Inventory`,
   `Tick_Combat`, `Tick_Body` and `Tick_Stealth` on bits of the DWORD at @1288. Porting it is what makes
   `ApplyAttributes` and `MaxSpeedModifier` worth porting — and it must land in the same package as them, because
   `ApplyAttributes` overwrites `GroundSpeed` and `MaxSpeedModifier` scales it.
4. **The contact system** (agent AU's root 4) is the last gap in these four natives and the whole of
   `ADishonoredPawn::Landed_Native`. Sized in "The contact system…" above.
5. **`ADishonoredPawn::TakeDamage_Native`** (2013 rva 0x75ccd0, 1,796 bytes) is the damage system proper. Until it lands,
   every damage reaching a Dishonored pawn is applied raw: no vulnerability scaling, no immunity, no minimum-health
   floor, no hit reaction. A fall is the one case where that happens to be right.
6. **`touch_census` and `sequence_census` are skipping in the harness**, on `build/release` at HEAD as much as here. The
   baseline's note says to flip `optional` to false once the line is in HEAD; instead the line's wording and the regex
   `DISHONORED\(bringup\): (?:touch|distouch) census` do not meet. Worth ten minutes from whoever owns them, because two
   of wave 5's three new counters are currently unguarded.
7. **`m_pAttributeTweaks[0]`, `[2]` and `[3]` are dead in both retail builds.** If a future package finds a use, it is not
   in the exe; the array is almost certainly a per-difficulty authoring slot that was superseded by the four base values
   inside `FDisAttribute`.

## Commands

```
python build/agentBF_work/w1.py      # the three attributes CppText headers
python build/agentBF_work/w2.py      # disattributes.cpp
python build/agentBF_work/w3.py      # disattributesinterface.cpp + distweaks_attributes.cpp
python build/agentBF_work/w4.py      # the pawn wiring and the four fall natives
python build/agentBF_work/w5.py      # the pawn CppText headers + the ported-natives list
python build/agentBF_work/w6.py      # dishonoredpawn_health.cpp
python build/agentBF_work/sync_build.py                 # re-sync build/agentBF_wt + regenerate the module there
cmd /c build\agentBF_release.cmd                        # Release build of the snapshot into build/agentBF
python resources/tools/ida/run.py build/agentBF_work/disasm.py resources/docs/idb/retail2013_agentBF.i64 0x762190
python resources/tools/ida/sym.py "DisAttributes"       # 2012 + 2013 rvas
python resources/tools/ida/off.py UDisTweaks_Pawn 264   # name the member at a retail byte offset
```

The accept command, and the run the numbers at the top come from:

```
python resources\tools\build_and_smoke.py --build-dir build/agentBF --no-build --exe-name DishonoredGame_BF.exe ^
  --log-name agentBF_strict_d3d9.log --ini-dir build/agentBF/config --rhi d3d9 --timeout 120 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "inputtest moved" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -strictnatives -disattrib -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

`resources/play.cmd` and `resources/build-release.cmd` were not touched. No commits, no `git add`.
