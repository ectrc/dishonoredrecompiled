# Agent ES (PHASE13 ES) — the shim backlog: what retail actually lacks, and the two numbers the brief had wrong

Worktree `build/agentES_wt`, detached at `2d08c15`. Own build dirs `build/agentES_base` (the ninja graph the
audits read) and `build/agentES_release` (the clean acceptance build), own IDA copy
`build/agentES_ida/retail2013_agentES.i64`, headless `idalib` only, no IDA MCP tools and no FModel tools.
Nothing committed, nothing staged; nothing outside the worktree was edited.

## The answer in ten lines

* **1,098 shim declarations → 887.** 211 `DISHONORED_SHIM_STATIC` declarations, 21 `VERIFY_CLASS_OFFSET_NODIE`
  lines, 32 shim-block banner comment lines this deletion left with nothing under them and 5 orphaned member doc
  comments, across 20 headers. **269 lines, pure deletions, not one insertion**, and every one of the 211 is
  named in `agentES_status.csv` with the evidence that retail 2013 lacks it.
* **The brief's 1,053 is wrong, and so was my first replacement for it.** 1,053 is `shim_audit.py`'s
  `in_retail == 0`, which means only *"this name is not a UProperty of a class resolved from its owner by prefix
  guessing"*. It cannot distinguish a feature retail does not have from a C++-only member no dump enumerates.
  The defensible split is **761 provably absent, 121 unproven, 5 that retail HAS** — section 2.
* **Five shims are the opposite of a placeholder: retail 2013 *has* those members, per-instance.**
  `UMaterial::bAllowFog`, `bUsedWithFogVolumes`, `bUsedWithFracturedMeshes` (bits 3, 12 and 16 of the dword at
  **752**), `UPrimitiveComponent::ReplacementPrimitive` (**196**) and `AWorldInfo::ProcBuildingRulesetOverride`
  (**1244**). All five are `inline static` here, so one object serves the whole process where retail gives every
  instance its own. Each offset is confirmed in the binary by bracketing against a neighbour whose own name
  matches the SDK dump — section 4. These are defects, not backlog.
* **`Sources.cmake`'s exclude total is 875, not 1,077 — and 874 of the 875 are legitimate.** My own first count
  swept up a second block, `<Module>_NOT_IN_PDB`, which **is compiled** and which no cmake code reads.
  DishonoredGame's **818 is right**; every other module's figure in my brief was wrong. Section 3.
* **The exclude list is not a deletion backlog. It is a port queue.** 813 of the 818 DishonoredGame skeletons are
  confirmed alive in retail 2013, all 818 are true skeletons with zero code lines, and the whole-tree deletion
  candidate list is **one unit** — `AkAudio/Src/akaudio.cpp`, already superseded by a compiling registrant.
* **53 excluded units are not skeletons at all: they hold real code that never compiles**, 6,385 lines of it in
  five already-ported GFxUI units gated behind `DISHONORED_WITH_GFX3`. Section 3.3.
* **I shipped a defect of exactly the kind this package exists to catch, and caught it by checking my own
  verdicts against a second source.** A rule of mine read the *declared type* for a class retail lacks — and
  `UBOOL`, `FLOAT`, `FName`, `FString` match that pattern while being Core typedefs in no dump. It was wrong on
  **52 of 204** rows and was the sole evidence for **7** deletions. I withdrew all 7, then reinstated 6 on
  binary evidence and left the seventh out. Section 5.
* **Six of the fourteen features I had clustered as absent are present in retail 2013** — APEX (five registered
  UClasses and five shipped DLLs), FaceFX (1,434 statically linked functions), crowd agents, instanced static
  meshes, Lightmass's runtime, and part of morph targets. Four are genuinely 0/0/0: `UReachSpec`, secondary
  viewports, peer-to-peer/voice, camera anims. No deletion of mine rested on feature absence, but the handover
  ranking would have misled the next wave — section 6.
* **Two truth sources are holed, both found this wave.** `retail_sdk_layout.json` is missing
  `ADishonoredNPCPawn` outright while listing 25 of its siblings; `script_classes_2013.json` omits the Core
  intrinsics (`UClass`, `UState`, `UStruct`, `UFunction`) because they are cooked into no package. Section 7.
* **The ratchet is `resources/tools/shim_ratchet.py`** against `resources/docs/shim_ceiling.json`, and it fires:
  section 8 is the demonstration, including that `--update-ceiling` refuses to launder a rise and that renaming
  a verdict cannot launder one either.

## 1. What a shim is, and why the question "does retail have it" is answerable

`DISHONORED_SHIM_STATIC` expands to `inline static` (`Engine/Inc/Engine.h:13`). A member declared with it is one
object for the whole process: never serialised, never a UProperty, never written by the loader, and shared by
every instance. `shim_audit.py` (agent DP) ranks the 1,098 by how their *use* goes wrong. This package answers
the prior question — is the member in retail at all — because the ranking is worthless if the honest fix for a
row is "give it real storage" rather than "delete it".

For a **script property** the answer is knowable, because retail's member set is enumerated twice over:

| source | what it is |
|---|---|
| `resources/docs/types/retail_sdk_layout.json` | a CodeRed SDK dump of the running retail 2013 exe: 3,038 classes, 1,118 script structs, UProperties with offsets |
| `resources/docs/types/script_classes_2013.json` | the classes of the cooked 2013 packages: 3,043 classes with their `children` |

They agree: on `SkeletalMeshComponent`, 107 properties each and **zero difference**; across the 50 owner classes
of my deletions, **42 agree exactly**, 5 differ only in how the CodeRed dump renames an InterfaceProperty
(`TitleFileInterface` → `TitleFileInterface_Interface`/`_Object`) or carries a name the other omits, and 3 are
missing from one side. No deleted name is in the union of the two.

What makes a shim a script property is read off the **reference UE3 tree**, never this one, because this tree has
already moved every shim out of the generated property block into a trailing shim block — **0 of 1,098 sit inside
a `//## BEGIN PROPS` region here**. In the reference tree the same member is either inside such a region, or named
by a generated `VERIFY_CLASS_OFFSET*(<CppClass>, <ScriptClass>, <Member>)` line, which the script compiler emits
for a property declared `var native` in script and by hand in C++. That second route matters: the entire cloth
block reaches it only that way — `ClothMeshPosData` is hand-written in the reference `UnSkeletalMesh.h:720` and
*also* carries `VERIFY_CLASS_OFFSET_NODIE(USkeletalMeshComponent,SkeletalMeshComponent,ClothMeshPosData)` in
`EngineSkeletalMeshClasses.h`. 9,757 PROPS pairs and 2,506 VERIFY pairs over 945 reference headers.

## 2. The count, and what the brief's 1,053 actually measures

`shim_audit.py --build-dir build/agentES_base` on the unmodified tree: **1,098 declarations, 1,061 distinct
names, 33 headers, 917 compiled units, 63 build macros**; `in_retail` is `0` for 1,053, `1` for 5, `?` for 40.
That is where the brief's figure comes from, and `in_retail == 0` is not "retail does not have this": it is
"this name is not in the member list `retail_members()` found for an owner resolved by trying `X`, `UX`, `AX`,
`FX` and `X[1:]`". A C++-only member of a class no dump enumerates scores 0 exactly like a deleted feature does.

Measured with the rules of section 1, on the tree as I leave it (887 shims):

| verdict | n | what it rests on |
|---|--:|---|
| `retail-lacks` | 737 | a reference script property of its class; retail has the class in **both** dumps and the name in neither |
| `retail-lacks-span` | 24 | a C++-only member of a script struct whose CodeRed record is **closed**: `span_start` 0, no gaps, named members tile every byte to `span_end`, so no unnamed byte exists for it |
| `retail-lacks-1src` | 0 | — |
| `retail-has` | 5 | **present** in retail's property set: a defect, section 4 |
| `class-absent` | 0 | — |
| `unproven` | 121 | a C++-only member of a class neither dump enumerates. Absence is **not shown** |

**761 provably absent, 121 unproven, 5 present** (761 + 121 + 5 = 887). Before the deletions the same rules give
**966 provable, 127 unproven, 5 present** of 1,098 — every one of the 211 deletions was provable, so the counts
move by exactly the deletions plus the 6 the binary settled out of `unproven`. So the brief's 1,053 overstates
the provable set by **87** and, more to the point, counts the 5 defects among the things to delete and says
nothing about the 127 that genuinely cannot be decided from a truth source.

The `retail-lacks-span` rule needs its own guard, and it has one: `span_start == 0`. The same arithmetic on a
*class* is unusable, because the CodeRed dump's span does not close against the binary's `PropertiesSize` —
`AActor` is 592 bytes with its last property ending at 584, and 447 classes have a `span_start` that is not
their super's size. On a top-level script struct there is no super and no alignment slack, so the closure is
real: `FLightmassWorldInfoSettings` is span 0..60 with 17 members tiling all 60;
`FApexModuleDestructibleSettings` is span 0..12 with 3, and `AWorldInfo::DestructibleSettings@1336` is **12 bytes
wide** in a layout the binary confirms, which pins the closure from outside the dump.

### 2.1 Why only 211 and not 761

A shim can only be deleted when nothing compiled uses it, or the using code goes with it. Joining the two audits:

| `shim_audit` rank | meaning | of the 1,098 |
|---|---|--:|
| A-fatal | a container that is indexed, size-compared or inside a `check()` — aborts once content reaches it | 80 |
| B-shared-write | written at runtime: one store for the whole process | 397 |
| C-dead-read | read and never written: permanently zero, so the reader silently does nothing | 400 |
| D-unused | no use in any compiled unit and no use in any header | 221 |

The 211 I deleted are the D-unused ones with proven absence, plus the 6 the binary settled. The other 558
provably-absent shims each need their uses deleted first — 13,421 uses in compiled code in total — which is code
surgery, not line deletion, and three other packages are editing these same files this wave. The ranking is in
`agentES_status.csv` as `shim_remaining`, and the measured cost is there too: **266 of the remaining provable,
unambiguous shims have at most one using file and at most two uses**, which is the cheapest next tier and the one
I would take.

## 3. `Sources.cmake` — the inventory, and the count that was wrong

`resources/tools/exclude_inventory.py` (new) parses every `set(<Module>_EXCLUDE ...)`, reads each unit's
`import_reference.py` banner, extracts the owner types from the demangled symbols, and decides retail-2013
existence per owner. Output: `resources/docs/agents/agentES_exclude_inventory.csv`, 875 rows, 24 columns.

**Banner RVAs are 2012 addresses.** They come from the 2012 shipping PDB. No banner RVA appears as a 2013
address anywhere in the inventory; the only 2013 addresses it emits are `registrant_rva_2013` and
`staticclass_rva_2013`, read out of the 2013 image via `native_class_sizes.csv`.

### 3.1 The total is 875, not 1,077

My brief's table was `grep -c '\.cpp'` over the whole file, which also counts `set(<Module>_NOT_IN_PDB ...)`.
`cmake/DishonoredModule.cmake:58` only ever iterates `${name}_EXCLUDE`; `_NOT_IN_PDB` is read by no cmake code
and is documentary ("compiled, but the Shipping PDB attributes no function to them").

| module | `_EXCLUDE` | `_NOT_IN_PDB` (compiled) | `_EXTRA` | my brief's figure |
|---|--:|--:|--:|--:|
| DishonoredGame | **818** | 0 | – | 818 — correct |
| GFxUI | **27** | 0 | – | 35 |
| Engine | **17** | 132 | 3 | 153 |
| OnlineSubsystemSteamworks | **9** | 0 | – | 9 — right by coincidence |
| IpDrv | **2** | 24 | – | 26 |
| Core | **1** | 18 | – | 19 |
| AkAudio | **1** | 0 | – | 2 |
| D3D9Drv / WinDrv / GameFramework / Launch | **0** each | 3 / 6 / 5 / 1 | – | 3 / 6 / 5 / 1 |
| **total** | **875** | 189 | 3 | 1,077 |

DishonoredGame is the only module whose `_EXCLUDE` is an `import_reference.py` skeleton list. **GFxUI's is not a
flat list**: `GFxUI/Sources.cmake:50-64` mutates it inside `if(DISHONORED_WITH_GFX3)`, removing five units and
appending one otherwise, so any tool that reads the `set()` alone must say so — mine prints the six conditional
edits separately.

### 3.2 Verdicts

| verdict | DishonoredGame | all 875 |
|---|--:|--:|
| `retail-has-class` (a skeleton waiting for a package) | 788 | 789 |
| `retail-has-code` (non-UObject or free functions, but a strong 2013 match) | 25 | 26 |
| `2013-unconfirmed` (weak matches only — hand work, **not** absence) | 5 | 6 |
| `retail-lacks` (nothing resolves to 2013) | 0 | 1 |
| `not-a-skeleton` (real code that never compiles) | 0 | 53 |
| `missing` (listed but absent from disk) | 0 | 0 |

All 818 DishonoredGame skeletons have zero code lines and **813 of 818 are confirmed alive in retail 2013**.
10,138 of 10,142 banner symbol lines resolved to an owner. **There is no deletion wave in the exclude list.**

The single `retail-lacks` unit is `AkAudio/Src/akaudio.cpp`: its one banner function,
`AutoInitializeRegistrantsAkAudio` (2012 rva `0x5fbfe0`), carries an explicit `method=unmatched` row in
`match_2012_2013.csv`, and the tree's own `AkAudio/Src/AkAudioRegistrants.cpp:52` already defines and compiles
that symbol with a guarded fallback in `Launch/Src/DishonoredStubs.cpp:23`.

Two owner **types** are genuinely 2012-only and should be dropped when their units are ported, though the units
themselves stay: `UGameplayEventsReader` (one symbol in `disaisubprocesswatchpoints.cpp`) and
`UDEPRECATED_DisTweaks_GFxMoviePlayerGameOver` (three in `disgfxmovieplayerpausemenu.cpp`).

### 3.3 The 53 that are not skeletons

None grew out of the skeleton pipeline; all are reference-engine or middleware units excluded on purpose
(editor-only, `WITH_GFx=0`, `WITH_STEAMWORKS=0`). The framing in my brief — that the exclude list is skeletons —
holds only for DishonoredGame. The five worth attention are **already ported and compile only when
`DISHONORED_WITH_GFX3` is on**: `gfxuirenderer.cpp` (3,658 lines), `gfxuiengine.cpp` (2,396), `gfxuiinteraction.cpp`
(103), `gfxuifile.cpp` (85), `gfxuiimageinfo.cpp` (43) — 6,385 lines of written-and-reviewed code that nothing
builds in the default configuration. Full list in the inventory CSV, `verdict == not-a-skeleton`.

### 3.4 The biggest ports waiting

`ADishonoredPlayerPawn` (4,688 bytes) is split across **11** excluded skeletons, so it is one coordinated job
rather than eleven. The largest single-owner units are `disriverkrust.cpp` (78 functions, 1 owner) and
`dishonoredcheatmanager_player.cpp` (59, 1). The three biggest by function count —
`dishonoredanimnotifies.cpp` (192), `dishonoreddamagetype.cpp` (147), `dishonoredcontactsystem.cpp` (130) — are
multi-class dumping grounds with 38–49 owners each, so their size overstates their coherence.

### 3.5 Agent EN's link rule, restated because it still applies

Taking a unit off `_EXCLUDE` is not enough to make it link. Each module is a static library, MSVC takes a member
only to resolve an undefined symbol, and a dynamic initializer is not one, so a unit nothing calls into is
dropped whole. A registrant must be named from a unit the link always pulls in, with **external** linkage —
written `static ... * const` the compiler drops it before the linker sees it. I did not change `Sources.cmake`,
so nothing in this package depends on that; the note is for whoever acts on the inventory.

## 4. The five defects: members retail has that this tree made process-wide

Both dumps carry all five, and the binary confirms each offset by bracketing against a neighbour whose own name
matches the dump. The UMaterial bitfield map is pinned by **seven independent bit positions with no
disagreement**, each from a function whose name matches the member it reads
(`?IsUsedWithMorphTargets@FMaterialResource@@` → `shr eax,1Fh` on the dword at 752, and six more).

| class | member | offset | how the binary pins it |
|---|---|--:|---|
| `UMaterial` | `bAllowFog` | 752 bit 3 | bit 4 is pinned by name to `bAllowDisFog` and `bAllowFog` is declared immediately before it. Retail rewrote `?AllowsFog@FMaterialResource@@` to `bAllowDisFog && !bUsedWithDecals` and never reads bit 3 — a **dead** bit, but a per-instance one |
| `UMaterial` | `bUsedWithFogVolumes` | 752 bit 12 | bit 13 is pinned by name, so bits 0..12 are occupied; this build's `EMaterialUsage` has only 20 entries and none maps to bit 12, consistent with `CPF_Deprecated` |
| `UMaterial` | `bUsedWithFracturedMeshes` | 752 bit 16 | `?GetUsageByFlag@UMaterial@@` pins bit 15 and bit 18 by name; exactly two unreachable slots lie between, which the dump names `bUsedWithFracturedMeshes` and `bUsedWithParticleSystem` |
| `UPrimitiveComponent` | `ReplacementPrimitive` | 196 | `?SetShadowParent@` writes `ShadowParent` at `+0xC0` and folds `bHasExplicitShadowParent` into the dword at 280 — a double name match; `?UpdateBounds@` writes `Bounds.Origin` at `+0xCC`, pinning `Bounds@204`. Exactly 8 bytes remain for two pointers. The wide literal `L"ReplacementPrimitive"` is in `.rdata` |
| `AWorldInfo` | `ProcBuildingRulesetOverride` | 1244 | `MyParticleEventManager@1240` and `MaxPhysicsDeltaTime@1248` are both pinned by semantically matching code; exactly 4 bytes lie between, and **nothing anywhere touches `+0x4DC` on an AWorldInfo** — textbook `CPF_Deprecated` |

These want real per-instance fields at retail's offsets. I did not add them: three of the five are in headers that
agents EP, EQ and ER are editing this wave, and a layout change there belongs in a package that owns the layout.

## 5. My own defect, and the seven deletions I withdrew

My first classification had a rule above the property rule: *the declared type names a class retail 2013 has no
record of, so the member cannot exist*. It is sound for `UReachSpec` and `UCameraAnimInst`. It is **wrong for
`UBOOL`, `FLOAT`, `FName`, `FString`, `FStringNoInit` and `FPointer`**, which match `[UAF][A-Z]\w+` while being
Core typedefs and intrinsics that appear in no cooked package and in no CodeRed class list — the second hole in
section 7, reached from the other direction. A shim declared `FLOAT Foo` scored as proof that retail lacks a class.

It was the verdict on **442 of 1,098** rows, and 52 of the first 204 deletions were labelled by it. For 45 of
those 52 the property rule reaches the same conclusion independently, so the deletion stood on other evidence.
For **7 it was the only evidence**, and I restored all 7 before building. The binary then settled 6:

| member | verdict | evidence |
|---|---|---|
| `FSystemSettings::bAllowD3D11` | deleted again | the ini-key table has `AllowD3D10` and no D3D11 key; the stem `D3D11` occurs **zero** times in the 18 MB image in either encoding |
| `FSystemSettings::bAllowOpenGL` | deleted again | `OpenGL` zero times either encoding; the exe imports `d3d9` and names only `Development/Src/D3D9Drv` |
| `FSystemSettings::TessellationAdaptivePixelsPerTriangle` | deleted again | not in the key table; `Tessellat` survives only in particle-beam and Scaleform tessellation |
| `FSystemSettings::PerObjectShadowTransition` | deleted again | `ShadowTransition` zero times, while 20 other `Shadow*` keys of the same struct **are** in the table |
| `FSystemSettings::PerSceneShadowTransition` | deleted again | same measurement |
| `UWorld::SaveGameSummary_DEPRECATED` | deleted again | zero occurrences in either encoding, no registrant, and the name never appears even as a mangled-name fragment |
| `UWorld::RedirectNetDriver` | **left in place** | see below |

`UWorld::RedirectNetDriver` is the one that stays. The *class* `UNetDriver` is gone (zero literals, no registrant,
zero member functions, the whole net layer stripped) — but the type name survives as the mangled return type of
`?GetDriver@UPendingLevel@@UAEPAVUNetDriver@@XZ` at 2013 rva `0x52a920` (resolved by hand, exact function start;
a pure-virtual stub on a live registered class). Retail therefore still forward-declares `UNetDriver`, so a
`UNetDriver*` **field** can exist with no class behind it. `UWorld` is native/noexport, so no layout source can
bracket the field. A zero I cannot trust means the member stays.

Two method corrections that came out of this and that later waves need:

* **The FSystemSettings table stores the ini key, not the C++ member name, and it is inconsistent about the
  prefix** — it keeps `bAllow` for `bAllowLightShafts` and seven others, and drops it for `DynamicShadows`,
  `MotionBlur` and `Distortion`. My first calibration member, `bAllowDynamicShadows`, came back **zero**; taken
  at face value it would have made a known-good member read as absent. The usable method is to dump the whole
  81-key table and test membership, then search the bare stem in both encodings.
* **UE3 TCHAR literals in this image are UTF-16LE, and this build emits RTTI only for third-party SDKs.**
  `.?AVUNetDriver@@`, `.?AVUGameEngine@@`, `.?AVUWorld@@` are all absent while `.?AVNxCloth@@` is present, so
  "check for a vtable / RTTI descriptor" gives a false zero on **every** UE3 class here, and an ASCII-only grep
  of the exe finds none of these names. Any earlier conclusion of absence drawn either way is worthless.

## 6. Feature absence is not member absence, in both directions

I clustered the remaining shims by feature for the handover. The binary says **six of the fourteen clusters I had
called absent are present in retail 2013**: APEX (five registered UClasses — `UApexAsset` StaticClassNoInline
2013 rva `0x527660`, `UApexComponentBase` `0x527690`, `UApexStaticComponent` `0x5286b0`, plus
`UApexGenericAsset` and `UApexDynamicComponent` — and five shipped APEX DLLs), FaceFX (the OC3Ent::Face SDK
statically linked, 1,434 functions, `UFaceFXAsset` `0x527450`), crowd agents (`AGameCrowdAgent` `0x55ce60`,
194 functions), instanced static meshes (`UInstancedStaticMeshComponent` `0x10edf0`), Lightmass's runtime, and
part of morph targets. Four are genuinely 0/0/0 with no registrant: `UReachSpec` (retail navigation is
NavMesh-only, 470 navmesh functions as the calibration), secondary viewports, peer-to-peer/voice, camera anims.
Cloth and soft body are a third case: 121 of 129 cloth functions are PhysX SDK internals present only because
PhysX is statically linked, while the four UE3 entry points (`AttachClothVerts`, `InitClothSim`, `TermClothSim`,
`UpdateClothState`) are all zero — the UE3 path is absent, the SDK is not.

No deletion of mine rested on feature absence; every verdict is per member. But the ranking would have told the
next wave to delete the APEX and crowd blocks wholesale, so each `shim_remaining` row in the status CSV now
carries its feature's binary verdict, including the four that say **do not delete on feature grounds**.

The converse also holds and is the sharper trap: `AWorldInfo::DestructibleSettings@1336` is a real property in a
layout the binary confirms, yet neither `ApexModuleDestructible` nor `DestructibleSettings` occurs anywhere in
the image in any encoding. Deprecated and script-only UProperties leave no literal and no instruction.
**String-absence is usable only for a C++-only member that goes through a name table** — which is exactly why it
was the right method for `FSystemSettings` and would be the wrong method for anything script-declared.

## 7. Truth sources

* **`retail_sdk_layout.json` is missing `ADishonoredNPCPawn`** — the base NPC pawn, the most load-bearing
  gameplay class in the module — while listing 25 of its siblings (`ADisTallboyNPCPawn`, `ADisDLC06NPCPawn`, …).
  `script_classes_2013.json` has it, and `native_class_sizes.csv` gives it `size_2013=3584` and
  `registrant_rva_2013=0x7592d0`. Anything using the SDK dump alone as an existence test concludes the base NPC
  pawn is not in retail.
* **`script_classes_2013.json` omits the Core intrinsics.** `Class`, `State`, `Struct` and `Function` are absent
  as keys, because it is the cooked-package class list and the intrinsic metaclasses are cooked into no package.
  `script_2013 == no` on a Core intrinsic means nothing — and this is the same hole that made my type rule wrong.
* Both are why `retail-lacks` demands **both** dumps carry the class and `retail-lacks-1src` is a separate,
  weaker verdict rather than being folded in. It is 0 today, and it should stay visible when it is not.
* `resources/tools/rva_sweep.py` **crashes when `--path` is given a relative value**: it rglobs the path as
  given, then calls `path.relative_to(REPO)` on a relative result. Pass an absolute path. Unfixed — it is not my
  file and three packages are editing this tree.
* The vtable warnings in my brief did not bear on this package: nothing here reads a vtable. The warning that
  *did* bear on it is the general one, and it caught my own defect: when the evidence for "retail does not have
  this" is a truth source, check the binary. Section 5 is what that check cost and what it saved.

## 8. The ratchet

`resources/tools/shim_ratchet.py` counts every shim, classifies it by the five rules of section 2, and compares
each count against `resources/docs/shim_ceiling.json`. It exits non-zero when any count rises.

```
$ python resources/tools/shim_ratchet.py
887 DISHONORED_SHIM_STATIC declarations in 32 headers
  total                  887   ceiling 887    OK
  retail-lacks           737   ceiling 737    OK
  retail-lacks-span       24   ceiling 24     OK
  retail-lacks-1src        0   ceiling 0      OK
  retail-has               5   ceiling 5      OK
  class-absent             0   ceiling 0      OK
  unproven               121   ceiling 121    OK
ratchet OK                                                     (exit 0)
```

Reintroducing one deleted shim, `TArrayNoInit<class UReachSpec*> InventedPathList`:

```
  total                  888   ceiling 887    RISEN
RATCHET FAILED: total 888 > 887                                (exit 1)
```

And the two ways the number could be laundered are both closed:

```
$ python resources/tools/shim_ratchet.py --update-ceiling        # with the shim still there
refusing to raise the ceiling: total 887 -> 888                 (exit 1)

$ python resources/tools/shim_ratchet.py --update-ceiling        # after I renamed the verdict set
ceiling shim_ceiling.json was measured under verdict schema None, this tool measures
'retail-lacks|retail-lacks-span|...': it is stale, and --reset-ceiling is the only way to replace it   (exit 1)
```

The schema guard exists because I hit it for real: folding the defective `retail-lacks-type` verdict into the
others moved 300 shims between buckets, which would have read as a rise and, under a tool that only compared
numbers, could have been rewritten away silently.

It also prints the `retail-has` rows by name on every run, so the five defects of section 4 cannot be forgotten,
and `--list <verdict>` dumps one bucket. It is **not** wired into `run_regression.py`: that would make the set 38
checks where the acceptance for this wave is 37, and `regression_baseline.json` is a file three other packages
may be touching. Wiring it in is a one-line data change for a later wave.

`build/agentES_work/ctor_write_audit.py` is the other check this package leaves. It finds **108 writes to a
process-wide shim from inside a constructor or initialiser, in 59 bodies** — the single misreading the brief
names, in its worst form, because a constructor write looks exactly like correct per-instance initialisation and
instead overwrites the value every other instance is reading. The worst are
`FDynamicSortableSpriteEmitterDataBase`'s constructor (12), `FEngineLoop::Init` (9) and
`FParticleMeshEmitterInstance::Init` (6).

## 9. Acceptance

**`run_regression.py`**, the worktree's own copy, absolute `--build-dir`, built inside the harness, no
`--no-build` and no `--only`. `build/agentES_release` deleted first, so the harness build **is** the clean full
release build, with `DISHONORED_LAYOUT_CHECKS=ON` (the cmake default `build-release.cmd` takes). The five
gitignored layout inputs were copied into the worktree first, so no layout metric records `-1`.

**The build: 0 errors, 0 C4263, 0 C4264** in all three targets (`DishonoredGame`, `CoreSmoke`, `LayoutProbe`),
all six build metrics green, exe linked.

**The layout stage is the interesting one, and it is byte-identical to baseline**: `layout_types` 2314,
`layout_probed` 2341, `layout_mismatching` 0, `layout_contract` 0, `layout_compare_contract` 0. Deleting 211
`inline static` members changed no class layout, which is the prediction the whole package rests on — an
`inline static` member contributes no storage, so if any of the 211 had been real storage, `layout_mismatching`
would have moved. It did not.

**The second full run: `37 ok, 0 failed, 0 skipped`, 783s.** Every check, no skips.

**The first full run had measured 36 ok, 1 failed, 0 skipped.** The failure was `inputtest_moved` at 662.2
against a bound of 800.0 — the distance the pawn walks. It was not my change:

* nothing I deleted is read or written by any compiled unit (all 211 are `shim_audit` rank D-unused over 917
  compiled units), and the layout is unchanged, so there is no mechanism by which pawn movement could differ;
* `inputtest_peak_speed` was **500.0**, its own ceiling, so the pawn was moving at full speed — it covered less
  ground, it did not move more slowly;
* the run was loaded: `startup_seconds` 32.11 and `d3d9_startup_seconds` 21.37, against a brief that records the
  same executable measuring 3.3, 26.3 and 141.9 seconds here;
* re-running **only that stage against the same binary** gave `inputtest_moved` **1074.5**, above both the bound
  and the 1029.6 measured at HEAD, with the other eight inputtest metrics green.

The second full run settles it and names the cause: `d3d9_startup_seconds` fell from **21.37 to 3.3** on the same
executable, and `inputtest_moved` came in at **1029.9** against HEAD's 1029.6 — a 0.03% difference. It was load.
Both logs are kept — `build/agentES_regression.log` and `build/agentES_regression2.log` — rather than only the
good one.

**`rva_sweep.py`** over both touched directories, with absolute paths because of the bug in section 7:
`Engine/Inc` 418 citations (301 `ok-2013`, 117 `ok-2013-mid`, **0** mislabelled, **0** unknown),
`GameFramework/Inc` 25 (22 / 3 / 0 / 0). I added no address to the source tree — the changes are deletions — so
these are the pre-existing citations, unchanged and clean.
* `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is **not required and was not
  run**: I changed no class layout, added no class, and did not touch `Sources.cmake`. Re-running it would
  rewrite `Sources.cmake` and the generated headers while three other packages are editing them.

## 10. Every file I touched

Deletions only, in twenty headers. Counts read out of `git diff -U0`, not out of my notes — my first version of
this table was wrong because the shim-block banner comment itself contains the string `DISHONORED_SHIM_STATIC`,
so a classifier that tests for the macro before testing for `//` counts 15 comment lines as declarations:

```
file                                                             shims  VERIFY  banner  doc  total
source/Development/Src/Engine/Inc/EngineAIClasses.h                  1       0       0    0      1
source/Development/Src/Engine/Inc/EngineAnimClasses.h                9       1       2    0     12
source/Development/Src/Engine/Inc/EngineCameraClasses.h              6       0       0    0      6
source/Development/Src/Engine/Inc/EngineClasses.h                   47       3      12    0     62
source/Development/Src/Engine/Inc/EngineControllerClasses.h          9       0       0    0      9
source/Development/Src/Engine/Inc/EngineDecalClasses.h                1       0       0    0      1
source/Development/Src/Engine/Inc/EngineGameEngineClasses.h         28       0       2    0     30
source/Development/Src/Engine/Inc/EngineMaterialClasses.h            31      1       0    0     32
source/Development/Src/Engine/Inc/EngineParticleClasses.h             2      0       2    0      4
source/Development/Src/Engine/Inc/EnginePawnClasses.h                 5      0       0    0      5
source/Development/Src/Engine/Inc/EngineReplicationInfoClasses.h      2      0       2    0      4
source/Development/Src/Engine/Inc/EngineSequenceClasses.h             4      1       4    0      9
source/Development/Src/Engine/Inc/EngineSkeletalMeshClasses.h         1      8       0    0      9
source/Development/Src/Engine/Inc/EngineTextureClasses.h              3      0       0    0      3
source/Development/Src/Engine/Inc/EngineUIPrivateClasses.h            4      2       2    0      8
source/Development/Src/Engine/Inc/EngineUserInterfaceClasses.h        3      1       4    0      8
source/Development/Src/Engine/Inc/SystemSettings.h                    5      0       0    5     10
source/Development/Src/Engine/Inc/UnSkeletalMesh.h                    8      0       0    0      8
source/Development/Src/Engine/Inc/UnWorld.h                           1      0       0    0      1
source/Development/Src/GameFramework/Inc/GameFrameworkClasses.h       41      4       2    0     47
TOTAL (20 files)                                                    211     21      32    5    269
```

New files:

```
resources/tools/shim_ratchet.py                          the ratchet
resources/tools/exclude_inventory.py                     the Sources.cmake inventory tool
resources/docs/shim_ceiling.json                         the committed ceiling, total 887
resources/docs/agents/agentES.md                         this file
resources/docs/agents/agentES_status.csv                 243 rows: 211 deletions with per-deletion evidence
resources/docs/agents/agentES_exclude_inventory.csv      875 rows, 24 columns
```

Work products, gitignored under `build/`, not for the main tree but referenced above:
`agentES_work/delete_shims.py` (the deleter; it refuses to run when a line no longer holds the shim it expects,
which makes it the safe way to re-apply these deletions onto a merged header), `agentES_work/make_status.py`,
`agentES_work/ctor_write_audit.py`, `agentES_work/deleted_final.json` (the authoritative deletion set),
`agentES_work/shim_audit.csv`, `agentES_work/shim_ratchet_{before,after}.csv`, `agentES_work/ranking.json`,
`agentES_ida/agentES_binary_evidence.csv` (128 rows; 93 addresses each resolved by hand with
`ida_funcs.get_func`, 47 exact starts, 32 mid-function, 3 data).

`build/agentES_sync.py` is the authoritative copy list, worktree → main. It refuses to copy a header that main
has also changed since `2d08c15` and says to re-run `delete_shims.py` on the merged file instead.

## 11. What I would do next, in order

1. **The five defects of section 4.** They are live wrongness, not backlog, and the offsets are in hand.
2. **The 266 provable shims with at most one using file and at most two uses.** The cheapest tier, and the uses
   concentrate: `GameCrowd.cpp` 41, `UnPhysComponent.cpp` 25, `UnPhysSkelComponent.cpp` 17,
   `UnInterpolation.cpp` 15.
3. **The 80 rank-A shims.** These are the ones that abort the process, and `ANavigationPoint::PathList`
   (`TArrayNoInit<class UReachSpec*>`, 97 reads, 10 writes) is both the largest and the best evidenced —
   `UReachSpec` is 0/0/0 in retail with no registrant.
4. **The 121 `unproven`.** Each needs a per-member binary argument. Section 5 and section 6 are the method and
   its two failure modes.
5. **The five GFxUI units of section 3.3** — 6,385 lines of ported code that the default configuration does not
   build. That is not a shim problem, but it is the same shape of problem: work that exists and does not run.
