# Agent JG — jump feel: the landing spring, the bump smoother in flight, the landing contact, attributes (wave 27, PHASE24 JG)

Worktree `build/agentJG_wt` (made at `96cdf21`; main moved to `810f13a` (JA) and `d7123aa` (JB) meanwhile, none of this package's files
touched — the sync script's base is `d7123aa`). Build dir `build/agentJG` (RelWithDebInfo), staged `DishonoredGame_JG.exe`. **One build,
clean on the first try** (`build/agentJG/build1.log`); the second was not needed. Baseline exe `DishonoredGame_JG0.exe` = a copy of the
staged `DishonoredGame-Win64-Shipping.exe` (head_rel, `2127b56` code = `96cdf21` code). IDA only on `build/agentJG/idb/{retail2013,
shipping2012}_agentJG.i64` (`jg_ida.py`, `jg_xref.py`, `jg_fname2.py`; decompiles `build/agentJG/d12`, `d13`). No FModel, no retail exe,
no real input; every run `game_input.py launch -windowed -nomovie` (`build/agentJG/jg_run.py`, steps `jg_steps*.txt`). Trace parser
`build/agentJG/jg_parse.py <log> [--frames]` (pawn / view Z per frame from `-ipdiag`, per jump).

## 1. What was ported

- **`UDishonoredCamera_PhysicalReact` whole** (`dishonoredcamera_physicalreact.cpp`, off the exclude list; CppText
  `UDishonoredCamera_PhysicalReact.h`, `FDisSpringPoint.h`): the spring point (`UpdateSpringPos` 0x6c0a00, `IsSpringing` 0x6c09a0,
  `StorePreviousPhysResult`, `ApplyMaxConstraint`, `GetPos_Interpolated`), `OnTeleport` 0x6c0900, `PostBeginPlay_Derived` 0x6c0c50,
  `OnActivate_Derived` 0x6c0cf0, `OnCameraCollided_Derived` 0x6c0e10, `GatherInfluence_Start` 0x6d49b0, `_FixedTimeStep` 0x6c0f30,
  `_Derived` 0x6c5be0, `OnImpact` 0x6c10f0 (2013 vtable found at 0x111eadc / 0x111ec24 through the xrefs of the two named slots).
  HitReact, Shake and Recoil derive from it and now run the same code (their springs sleep after the first gather; nothing calls
  their `OnImpact` yet — `ShakeCam`/`ApplyCamRecoil` are the 2013 callers).
- The base influence's unnamed empty slot is `OnCameraCollided_Derived` (named in `CppText/UDishonoredCameraInfluence.h`).
- **Bump smoother weight**: `SetWeight(0, 0.1)` entering Jump (0x675ee0) and Falling (0x6759f0), `SetWeight(m_fDefaultWeight, 0.1)` leaving
  Jump (0x65ffa0) and Falling (0x663110), in retail's place in each body (replacing IF's `IFDIAG_ONCE`s).
- **`Landed_Native`** (0x6b53a0): `OnImpact(Velocity)` on the physical react unless the big land's action was requested,
  `ClearFOVTarget(1)`, the contact type (JumpLand_Big / JumpLand_Stealth from the fall's `m_bIsStealthFall` / JumpLand),
  `m_fTimeOfLastLandedContact`, the 2013 Kismet event `DisDLC06SeqEvent_Landed` (class name read from the registration at 0x13171d0).
  A landing whose fall does not animate now skips the actions and the contact and goes straight to the camera, as retail.
- **`ADishonoredPawn::Tick_Attributes`** (0x749780) called as the fourth tick phase, **`ApplyAttributes`** (0x756880, 2013's eleven ammo
  slots; FNames read back from their initialisers 0x9e5d96..0x9e6dee), **`ADishonoredPlayerPawn::ApplyAttributes`** (0x6a7910: mana max,
  view pitch from the camera tweaks, air control, spring razor) and **`performPhysics`** (0x6a1670, `m_NetExternalVelocity`).
- `-jgdiag` (`Inc/jgdiag.h`, `Src/jgdiag.cpp`): react state/impacts/output, the per-frame player line around a jump, every smoother
  `SetWeight`, every landing, `ApplyAttributes` results, a NaN alarm. Measurement only.

## 2. Measurements (L_Tower_P spawn, SpaceBar; `JG0` = HEAD, `JG1`/`JG1b` = this build; `-ipdiag` pov per frame)

| | HEAD (JG0, 2 jumps) | build (JG1 3 + JG1b 3 standing jumps) |
|---|---|---|
| pawn apex | +105.4..106.4 at 0.26..0.30 s | +105.4..106.7 at 0.26..0.30 s (unchanged physics) |
| view apex | +95.5..96.7 at 0.30..0.31 s (lags the pawn, 10 uu short) | **= the pawn's, same frame** (+106.3..106.5) |
| pawn lands | 0.60 / 0.61 s | 0.54..0.56 s |
| view reaches the ground rest | 0.81 / 0.84 s (comes down from above, no dip) | dips immediately |
| view dip after landing | 0 | **-45.4..-47.1 uu**, -11 uu on the landing frame, deepest at +0.15..0.18 s |
| rebound | none | one: back through rest at +0.50..0.54 s, overshoot to ~+6.5 uu, settled ~+1.1 s (≈1 Hz) |
| running jump (v2d 420) | - | dip -38.7 uu at +0.18 s, view apex +108.5 |
| NaN in the spring | - | 0 (`jgdiag NAN` never logged; no black screen) |
| smoother weight | 1 through the flight | 0 within 0.1 s of the jump, 1 again 0.1 s after landing (`jgdiag smoother`) |

Spring check by hand (60 Hz, k 75 / d 8 / cap 700, landing at -800): deepest -47 uu at 0.133 s, through rest at 0.40 s - the run
matches (the frame rate here is 17..50 fps, so the measured minimum lands one frame later).

Attributes (`jgdiag attrib`, Hard difficulty on this route): player GroundSpeed **400** (was the class default 500), WaterSpeed 600,
AccelRate 2000 (3303), **MaxFallSpeed 2100** (1524), CrouchHeight 33 (29), WalkableFloorZ 0.643 (0.7), AirControl **0.2** (0.05),
ViewPitch -15473..16384. NPCs: GroundSpeed 762 (360), WalkableFloorZ 0.5, CrouchHeight 0.

- Walk peak 400 (`inputtest_peak_speed` 400.1), sprint peak **600.0** (unchanged: it is `Attribute_GroundSpeedSprint` / GroundSpeed x
  GroundSpeed). The 400 is the attribute itself (`-disattrib`: `Attribute_GroundSpeed base 400`, `GroundSpeedCrouch 275`,
  `GroundSpeedSprint 600`); the sneak multiplier CrouchedPct 0.6875 = 275/400 exactly, i.e. the tweaks were tuned against 400 - IF's
  "500 walk" was the unported class default.
- NPC walkers: Worker B (`DishonoredNPCPawn_3`, new-game route, `CC_JG1r.log`) **123.0 u/s** at world 118..123 - unchanged
  (NPC locomotion does not read GroundSpeed).

Landing sound (`-akdiag`): at every landing frame the only player post is the `Empty_JumpLandSmall` notify **`Snd_P_Fall_Wind_Stop`**
(HEAD and build alike) - the wind loop's stop, not the land thud. The thud is the JumpLand contact: `UDishonoredContactSystem::
ApplyContact` (2013 0x7e61a0, 365 B) needs the whole contact system - FindIntersection / FindClosestIntersection / FindListHead /
UpdateLinkedLists / Flatten* / Fixup / PostBeginPlay_Contacts (~2.6 KB), DisGetPhysicalMaterial / DisConvertCheckResultToImpactInfo /
DisGetPawnFeet (~0.4 KB) and the sub-intersections (SoundCue 174, AINoise 566, CameraShake 224, Rumble 156, Decal 1187, Particle 1966,
CameraEffectEmitter 296 B) - about 7.6 KB, and the footfalls go through it too. **Not one build: left named**, a package of its own.

## 3. Gate (`run_regression.py --build-dir ../agentJG --no-build --exe-name DishonoredGame_JG.exe --log-prefix JGg_ --only d3d9,inputtest,newgame`)

**23 ok, 0 failed** (`build/agentJG/gate.txt`): criticals 0/0/0, `unported_natives` 0, `probe_natives` 3, **`inputtest_moved` 813.0**
(>= 800; was ~1030), **`inputtest_peak_speed` 400.1** (>= 400; was 500), `newgame_on_mission` 115, `newgame_pawns` 29. Both inputtest
numbers now sit on their bounds because the walk is 400: the baseline notes (500 / 1030) should be re-read against the attribute,
and the bounds lowered with a margin (suggest peak >= 380, moved >= 700) so load noise does not flake the gate.
Generator: 79/79 byte-identical (`--out-dir build/agentJG/regen`), `bitfield_audit` mismatches 0.

## 4. Where the brief was wrong (with the measurement)

1. **"the view apex within 0.05 s of the pawn apex (0.53 s)"**: 0.53 s is the whole air time. The pawn's apex is at **0.26..0.30 s**
   (800 / 3000 with the double gravity); it lands at 0.54..0.60 s. HEAD's view apex was 0.30..0.31 s / +96 (not "0.37 s / 92"), back on
   the ground at 0.81..0.84 s (not 1.1 s).
2. **"MaxFallSpeed 2000"**: the tweak gives **2100**.
3. **"500 walk must hold"**: retail's walk is `Attribute_GroundSpeed` = 400 (see §2); 600 sprint holds.
4. **"back to the default in Landed (0xa63110)"**: 0xa63110 is `UStatePlayerMasterFalling::OnExitState` (2012 0x696260).
5. **"GetCamera_BumpSmoother (0x6fbfd0, a symbol)"**: it already had a body (inline in `CppText/ADishonoredPlayerCamera.h`);
   `dishonoredplayercamera.cpp` is untouched. 2013 rva 0x6c02b0.
6. **"does Empty_JumpLandSmall carry the land sound?"**: it carries `Snd_P_Fall_Wind_Stop` only; the thud needs the contact system.
7. "Where this plan is most likely wrong" #4 (the land action's blend): the view trace shows the spring makes the dip; the land action
   is the upper body only.
8. The dip is ~46 uu, far above the >= 8 uu bar: OnImpact adds the full landing velocity (-750..-850, capped at 700 in the first
   step) to a k 75 spring. Faithful; whether it reads right to the user is the hand-play test.

## 5. Deviations / named

- Written guards: ammo slots missing from the inventory are skipped (retail asserts on the index).
- `OnCameraCollided_Derived`'s caller (`ADishonoredPlayerCamera::HandleCollision`, the collision branch of UpdateViewTarget) is unported.
- Landed_Native: the floor trace + `ApplyContact` named (JGDIAG_ONCE); the big land's vtable +1144 call (argument 0) before its
  RequestAction is not written (IM's branch, untouched).
- 2013's OnTeleport body 0x6c0900 is shared by the bump smoother's slot too; the bump smoother still has the base's empty OnTeleport.
- `dishonoredpawn.cpp` (outside the brief's list) gained the three-line Tick_Attributes call in `ADishonoredPawn::Tick`.
