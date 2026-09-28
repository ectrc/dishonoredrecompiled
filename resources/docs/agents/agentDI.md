# Agent DI — The NPCs have no heads: the modular character (2026-09-28)

Package DI of wave 9. The user reported it from the running game: the 26 NPC pawns agent CG's spawn path puts down in
`L_Tower_P` render **without heads**, and everything else about them draws. Status rows: `agentDI_status.csv`.
Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentDI.i64`); every "2013 rva" is a headless decompile in
`build/agentDI_decomp/r13` and the "2012" rva in brackets is `shipping2012_agentDI.i64` (`build/agentDI_decomp/s12`),
which carries the symbol names.

## The fault, in one line

A Dishonored character is **two** skeletal meshes — the body on `APawn::Mesh` and the head on
`ADishonoredNPCPawn::m_pHeadMesh` (retail offset 3312) — and the only function that ever puts a `USkeletalMesh` on the
head component is **`ADishonoredNPCPawn::PostBeginPlay_Body`, 2013 rva `0x77f8a0`** (2012 `0x7bc180`). It is reached
through `ADishonoredPawn::PostBeginPlay` (`0x75bce0`), and **nothing in this tree overrode `PostBeginPlay` at all**, so
`APawn::PostBeginPlay` ran and every one of the pawn's six per-aspect passes was skipped. The head component existed on
all 26 NPCs, from the archetype, with no mesh on it for its whole life.

This is the same defect class as the previous nine, one level up: not a storage-less `DISHONORED_SHIM_STATIC` member
reading 0, but a **whole virtual pass simply absent**, so a feature that was fully built in the content (the tweaks
carry a head mesh for every one of the 26 NPCs) silently did nothing.

The second half of the symptom falls out of the first and is worth writing down, because it is what made "the component
is there but not attached" look like a separate attachment bug: `USkeletalMeshComponent::IsValidComponent()` returns
`SkeletalMesh != NULL && Super::IsValidComponent()`, so a skeletal mesh component with no mesh **is never attached**.
`attached 0` was a consequence of `mesh set 0`, not an independent fault. Measuring both in the same line is what settled
it in one run.

## The census, before and after

`-dishead` is the measurement (free when the switch is absent, hung off the same per-frame script call as agent CG's
`-disai` and agent AU's `-dispickup`, so no engine file is touched for it). It reports, over the whole spawned NPC
population: how many hold a head component, how many of those are attached, how many have a `USkeletalMesh` set, how
many resolve at least one material, how many are bound to the body as their animation parent, and how many the renderer
drew this second — plus the flow through `PostBeginPlay_Body`.

**Before** (`agentDI_base.log`, HEAD `7be1685` plus the census only, 75 s d3d9 on `L_Tower_P`):

```
dishead census: 26 NPC pawns; head component 26, attached 0, mesh set 0, with material 0, parentAnim 0,
  drawn this second 0 (bodies drawn 5); head choices in tweaks 26;
  PostBeginPlay_Body 0 passes, 0 heads set, 0 detached
dishead first: DishonoredNPCPawn_0 tweaks=Pwn_EliteGuard_MTall_5_Helm1 headChoices=1
  body[DisSkeletalMeshComponent_1 mesh=Skm_EliteGuard_Body attached=1 mats=21]
  head[DisSkeletalMeshComponent_0 mesh=NULL attached=0 parentAnim=NULL mats=0 hidden=0] accessories=0/0 sel=0
dishead components of DishonoredNPCPawn_0: 12 components, 2 skeletal: CylinderComponent(a) ArkComponentContainer(a)
  DynamicLightEnvironmentComponent(a) DynamicLightEnvironmentComponent(a) DisAttentionTargetComponent(a)
  DisSkeletalMeshComponent(a+m) DisSkeletalMeshComponent(--m) DisConversationComponent(a) ...
```

Read the component line: two skeletal mesh components, one attached **with** a mesh (the body) and one unattached
**without** one (the head). `PostBeginPlay_Body 0 passes` is the cause; `head choices in tweaks 26` is the proof that
the content was never the problem.

**After** (`agentDI_after.log`, 110 s d3d9 on `L_Tower_P`):

```
dishead census: 26 NPC pawns; head component 26, attached 26, mesh set 26, with material 26, parentAnim 26,
  drawn this second 6 (bodies drawn 6); head choices in tweaks 26;
  PostBeginPlay_Body 26 passes, 26 heads set, 0 detached
dishead first: DishonoredNPCPawn_0 tweaks=Pwn_EliteGuard_MTall_5_Helm1 headChoices=1
  body[DisSkeletalMeshComponent_1 mesh=Skm_EliteGuard_Body attached=1 mats=21 lastRender=102.92]
  head[DisSkeletalMeshComponent_0 mesh=Skm_EliteGuard_Head03 attached=1 parentAnim=DisSkeletalMeshComponent_1
       mats=6 hidden=0 lastRender=102.92] accessories=0/0 sel=0
```

**26 of 26** spawned NPCs now have a head mesh component, attached, with a `USkeletalMesh` and materials on it, bound to
the body mesh as its animation parent. `drawn this second 6` equals `bodies drawn 6`: **every NPC the renderer draws
draws its head with it**, and the other 20 are the ones outside the frustum, which the body count says too.

## The screenshots

| file | what |
|---|---|
| `build/agentDI/npc_head.png` | **the accept shot**: an Elite Guard at 110 uu, head unmistakable (`Skm_EliteGuard_Head03` — face, hair, beard, collar) |
| `build/agentDI/npc_head_before.png` | the same guard, same framing, same binary, headless |
| `build/agentDI/npc_head_after.png` | the first after shot, taken at 170 uu (the whole character) |

The pair is **one controlled change on one binary**, not two builds: `-disnohead` skips the single call that sets the
head mesh and lets everything else run, so the two frames differ by that call and nothing else. Both were captured with
`-apshottime=52` (never `-apshot`, PHASE10 rule 2) after `-disheadcam=45` had held the same NPC in front of the player's
own view point — the NPCs of `L_Tower_P` come out of 41 spawners spread over the mission, so a run that starts at the
spawn anchor never has one in shot, and `UDishonoredCamera` is stubbed, so the switch moves the NPC rather than the
camera.

## What is ported

**The chain, top to bottom.** All four were absent; each is tagged at the site with its 2013 rva.

1. **`ADishonoredPawn::PostBeginPlay`** `0x75bce0` (2012 `0x7995f0`) — `Super::PostBeginPlay()` then
   `PostBeginPlay_Body()`. Retail's other five passes are named at the site rather than skipped in silence:
   `PostBeginPlay_Actions` (vtable +1400), `PostBeginPlay_Attachments` (`0x755ab0`), `PostBeginPlay_Inventory`
   (+1472), `PostBeginPlay_Combat` (`0x757cf0`), and the `UDishonoredMapInfo` ambient-shadow copy plus
   `InhibitPowersFor(0)`.
   **`PostBeginPlay_Health` is left out on purpose and that decision matters**: retail inlines it as
   `Health = HealthMax = GetAttributeValue(Attribute_HealthMax)`, and in this tree `GetAttributes()` builds the set
   lazily from `m_pAttributeTweaks[1]` (agent BF) — a pawn whose tweak object is missing answers the
   `UDisTweaks_Pawn_Attributes` class default, whose `HealthMax` is 0, which would set **every NPC's health to 0 on
   spawn**. Whoever finishes the attribute tweak chain should restore that one line.
2. **`ADishonoredPawn::PostBeginPlay_Body`** `0x76a130` — the two eye skel controls out of the body tweaks and the
   cached-position pair. `SetupHitRegions` (`0x769f30`) and the `m_Foot_Caches` build-out (`FDisFootCache::Init`,
   `0x750fe0`) are named bringup gaps.
3. **`ADishonoredNPCPawn::PostBeginPlay`** `0x76a610` — `SetParentAnimComponent( m_pHeadMesh, Mesh )`, which is what
   makes the two meshes move as one character, and the three per-spawn counters. The rigid-body channel, the inventory
   loadout, the plague component, `RegisterAvoidable` (`0x7796b0`), the `FArkGameEventDispatcher` registration (CG
   hand-over 4) and `FDisComponentLODManager::StartLOD` (`0x870a80`) are named.
4. **`ADishonoredNPCPawn::PostBeginPlay_Body`** `0x77f8a0` — the whole function: the weighted head draw out of
   `m_RandomHeadMeshes` into `m_iRandomHeadMeshSel`, `SetSkeletalMesh` on `m_pHeadMesh`, the gore-section hide, the head
   and body material variations, the hat and mask accessories on the body mesh's sockets, the spine bender, the rotation
   intents (retail 2013 inlines `Init_Rotation` here), the body intentions, and `m_fPossessCamZOffset`.

**The helpers it needs**, none of which existed: `DisChooseRandomMesh<T>` (`0x776390` / `0x7764b0`, a template in
retail's own `dishonoredutilities.h`), `DisHideGoreSections` (`0x7c7dd0`), both
`DisReplaceMatchingMaterialsInSkelMesh` overloads (`0x7c8640`, `0x7cfb50`),
`FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh` (`0x77b0d0`), `FDisMaterialsOverride::Set` (`0x776f80`),
`ADishonoredNPCPawn::CreateAccessoryStaticMeshComponent` (`0x7706b0`) and `GetAccessoryStaticMeshComponent`
(`0x76e0f0`), and `ADishonoredPawn::GetBone_ByName` (`0x757730`).

## Three places retail 2013 differs from the 2012 build, all of which a 2012-faithful port would have got wrong

* **The possession-camera bone comes from the vision tweaks, not the body tweaks.** Retail's tail reads
  `*(Tweaks + 0x260)` — `UDisTweaks_NPCPawn::m_pVisionTweak` @608 — and then an `FName` at +140, which is
  `UDisTweaks_Vision::m_Vision_BoneName`. The 2012 decompile reads a different object. Confirmed on the disassembly
  (`0xb7fb91`: `mov eax,[eax+260h]` … `GetBone_ByName`), not on the pseudocode.
* **`UDisTweaks_NPCPawn::GetAccessoryMeshes` does not exist as a call in retail 2013** — it is inlined into
  `PostBeginPlay_Body` as the two-way choice between `m_HatMeshes` @548 and `m_MaskMeshes` @560. That matters here
  because `distweaks_npcpawn.cpp` is on the generator's exclude list, so a faithful port needed no new compile unit.
* **`ADishonoredNPCPawn::Init_Rotation` is inlined** in retail 2013 where 2012 calls it, and retail clears **two**
  soiree counters in `PostBeginPlay` where 2012 clears one.

Two smaller reads were decoded off the disassembly rather than the pseudocode, because the masks are the whole content:
`CreateAccessoryStaticMeshComponent`'s three bitfield writes (`@280 & 0xFFFFF3AF | 0x800` = clear `CollideActors`,
`BlockActors`, `BlockRigidBody`, set `bDisableAllRigidBody`; `@276 & 0xFFFE1FFF` = clear the four decal bits;
`m_CollisionTraceTypes @320 & 0xFFFFFF80` = clear all seven `FDisPrimTraceMask` bits), and the bit
`PostBeginPlay_Body` clears at @2404 mask 0x200, which the 2012 PDB's bitfield order identifies as
`m_bLocoModifierNeedsApplication`.

## Deliberate deviations, all documented at the site

* **`USkeletalMeshComponent::ShowMaterialSections`** (Engine, 2013 rva `0x34ddf0`) is the Arkane batch form
  `DisHideGoreSections` uses; this Engine tree has only the per-index `ShowMaterialSection`. The port calls that one per
  section — same effect, N render commands instead of one, and **no Engine file touched**. Retail passes
  `LODModels.Num() - 1` as the LOD index, i.e. it only ever hides gore sections on the coarsest LOD; that is kept as
  retail has it.
* **`SetFaceFXAsset`** (`0x312b90`) is not called: `WITH_FACEFX` is off in this build, so the Engine tree has no such
  method. Faces do not lip-sync; the head mesh itself is unaffected.
* **`FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh` and `FDisMaterialsOverride::Set` are spelled as
  file-local functions**, `DisApplyMaterialVariationToMesh` and `DisMaterialsOverrideSet`, in retail's own unit
  (`dishonorednpcpawn_body.cpp`). The bodies are retail's; only the spelling differs. Both structs are **generated**, and
  a member on a generated struct needs a new `Inc/CppText/<Struct>.h` plus a full DishonoredGame regeneration — which is
  this project's single largest source of broken HEADs (PHASE10 records two). **This package changes nothing the
  generator produces, adds no compile unit and needs no regeneration.** Restoring the member spelling is a two-file,
  one-command follow-up for whoever is regenerating anyway.
* Retail dereferences `Tweaks->m_pVisionTweak` and `Tweaks->m_pBodyTweaks` unchecked; both are guarded here, so an
  NPC whose tweaks have neither keeps its default `m_fPossessCamZOffset` and a NULL spine bender instead of crashing.
  Identical behaviour whenever the tweak objects exist, which on `L_Tower_P` is all 26.

## Runs

Build: `cmd /c build\agentDI_release.cmd` (Release, snapshot worktree `build/agentDI_wt` → `build/agentDI`), **0 errors,
0 unresolved symbols** (`build/agentDI_build7.log`; the screenshots are from the identical `build4`, which
differs only in comment text), and a **clean full Release build** of the same sources into `build/agentDI_clean`
(`build/agentDI_clean.cmd`, `build/agentDI_clean.log`), plus `CoreSmoke` and `LayoutProbe` so the whole harness runs.

| log | what | result |
|---|---|---|
| `agentDI_base.log` | `L_Tower_P` 75 s d3d9, census only, no port | the "before" census above, 0 `Critical` |
| `agentDI_before.log` | `L_Tower_P` 110 s d3d9, `-disnohead` | 26 pawns, **0** heads, 26 detached, 0 `Critical`; `npc_head_before.png` |
| `agentDI_after.log` | `L_Tower_P` 110 s d3d9 | 26/26/26/26/26, 6 drawn of 6 bodies drawn, 0 `Critical`; `npc_head.png` |
| `agentDI_head.log` | the first framing run at 170 uu | same census; `npc_head_after.png` |
| `build/agentDI/regression/summary.txt` | `run_regression.py --build-dir build/agentDI --no-build` | **31 ok, 0 failed, 0 skipped, 422 s**; `unported_natives` 0, `probe_natives` 6, `physics_actors` 1,369, `inputtest_moved` 1029.5 |

`-inputtest` reports the player pawn moving 1010.8 uu with peak 2D speed 500.4 in both the before and after runs, so the
port costs the walking path nothing; `unported_natives` stayed 0 and **no new warning line appears anywhere in the
log** (570 `Warning`/`Error` lines before, 570 after, with an empty set difference over the normalised text).

## Hand-overs

1. **The player pawn does not share this defect, and the reason is worth knowing.** `ADishonoredPlayerPawn` derives from
   `ADishonoredPawn`, so it now gets `PostBeginPlay` and `PostBeginPlay_Body` too, but it has no `m_pHeadMesh` — the head
   is on `ADishonoredNPCPawn` alone, and the player is first-person. What the player *does* now get for the first time is
   the eye skel controls and the cached-position pair. **Other character types do share it**: every class that inherits
   `ADishonoredNPCPawn` (`ADisTallboyNPCPawn`, the DLC NPC pawns) reaches the same pass, and `ADisTallboyNPCPawn`
   overrides `PostBeginPlay_Body` in retail (2013 rva `0x781270`, 295 bytes — the stilts mesh) while
   `ADisPossessionProxyPawn` overrides it with a 19-byte body (`0x7e7910`). Neither override is ported, so a tallboy gets
   the base NPC pass and no stilts. That is the next appearance package and it is small.
2. **The head is tilted back in both screenshots** because the body mesh's animation pose puts it there — the head bone's
   orientation comes from the body through `ParentAnimComponent`, which is now correctly bound. That points at the
   animation tree, not at this package: `FArkComponentLookat` / `FDisLookAtRequest::Initialize` (`0x74f180`) is unported
   (CG's gap), so nothing ever issues a head-look request and the head keeps whatever the base pose gives it.
3. **The five passes named in `ADishonoredPawn::PostBeginPlay`** are now one coherent piece of work with one call site
   waiting for each of them. `PostBeginPlay_Health` is one line and the highest value of the five, but it is gated on the
   attribute tweak chain answering a non-zero `HealthMax` — check that before restoring it.
4. **Accessories drew nothing on this map** (`accessories=0/0` on every NPC): the guards' tweaks carry no hat or mask
   entries, so the ported accessory path is exercised but has nothing to place. A map whose NPCs wear masks (the
   Boyle party, `L_Soiree`) is what would prove it.
5. **`UDisTweaks_NPCPawn::m_BodyVariationMaterials` and each head's `m_MaterialVariation` are applied** but this
   content's entries are empty for the Elite Guard, so every one of the 26 draws the mesh's own materials. The material
   variation is what makes two guards in the same uniform look different; a map with authored variations is what would
   show it.

## Files

**Mine** (10, listed in `build/agentDI_work/files.txt`). No generated file touched, no compile unit added, no generator
change — **the module does not need regenerating for this package**.

* `Inc/disheadcensus.h` (new; the census, the framing switch and the A/B switch)
* `Inc/dishonoredutilities.h` (the `DisChooseRandomMesh` template and the three free-function declarations)
* `Inc/CppText/ADishonoredPawn.h`, `Inc/CppText/ADishonoredNPCPawn.h` (declarations only; both files already existed, so
  the generated class bodies already include them)
* `Src/dishonoredutilities.cpp` (`DisHideGoreSections`, both `DisReplaceMatchingMaterialsInSkelMesh`)
* `Src/dishonoredpawn.cpp` (`PostBeginPlay`), `Src/dishonoredpawn_body.cpp` (`PostBeginPlay_Body`, `GetBone_ByName`)
* `Src/dishonorednpcpawn.cpp` (`PostBeginPlay`)
* `Src/dishonorednpcpawn_body.cpp` (`PostBeginPlay_Body`, the material and accessory helpers, and the census)
* `Src/dishonoredplayercontroller.cpp` (four lines: the census and framing hooks, beside CG's `-disai` and AU's
  `-dispickup`)
* this report and `agentDI_status.csv`

**Scratch** (not repo tools): `build/agentDI_work/` — `write_census.py`, `write_port.py`, `write_port2.py`,
`write_headcam.py`, `write_ab.py` (every edit re-applied idempotently), `crlf.py`, `disasm.py`, `files.txt`. Decompiles
in `build/agentDI_decomp/{r13,s12}`. Snapshot worktree `build/agentDI_wt`, build dir `build/agentDI`, build logs
`build/agentDI_build{0..6}.log`. IDA copies `resources/docs/idb/retail2013_agentDI.i64` and
`shipping2012_agentDI.i64`.

## Commands

```
python resources/tools/make_snapshot.py DI --list build/agentDI_work/files.txt   # refresh the snapshot overlay
python build/agentDI_work/crlf.py                                               # sources back to CRLF
cmd /c build\agentDI_release.cmd                                                # Release build into build/agentDI
cmd /c build\agentDI_release.cmd CoreSmoke                                      # and LayoutProbe
python resources/tools/run_regression.py --build-dir build/agentDI --no-build    # 31 ok, 0 failed
```

The measured run, in full:

```
python resources\tools\build_and_smoke.py --build-dir build/agentDI --no-build --exe-name DishonoredGame_DI.exe ^
  --log-name agentDI_after.log --ini-dir build/agentDI/config --rhi d3d9 --timeout 110 ^
  --skip-native OnlineSubsystemPC --milestone "Initializing Engine..." --expect "Initial startup" --forbid "Critical" ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -dishead -disheadcam=45 -apshottime=52 -forcelogflush ^
   -windowed -ResX=1280 -ResY=720 -nomovie"
```

Add `-disnohead` for the before half of the pair. `resources/play.cmd` and `resources/build-release.cmd` were not
touched. No commits, no `git add`.
