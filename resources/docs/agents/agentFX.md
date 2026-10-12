# Agent FX — the NOEXPORT bitfield audit, and the generator that learned four waves of hand-edits

Wave 22 (`PHASE18.md`), package FX. Worktree `build/agentFX_wt`, base `da77f0f`, with main `f14326b` (README,
PHASE18's FZ/GA packages) merged by `git merge` as the coordinator asked. Working build `build/agentFX_rel`
(Release, `DISHONORED_LAYOUT_CHECKS=ON`, through `build_queued.py`: `build/agentFX/fx_build.cmd`, log
`fx_build0.log`); the gate's in-harness build is `build/agentFX_reg` (`build/agentFX/fx_regression.log`).
Sync `build/agentFX_sync.py` (20 files, every function named, self-checked against the worktree diff).

No game was launched by this package: both halves are about the tree's declared and generated surface, and the
measurement that matters for the bitfield is static (the cooked masks against the C++ declaration order). The
gate's runs are the only runs, and they go through the harness with `-windowed`. No retail `Dishonored.exe` was
launched. No real input was touched.

## 0. The answer in six lines

1. **The audit found 2 real bitfield faults in the whole tree, both in `USkeletalMeshComponent`, both read, both
   fixed.** `resources/tools/bitfield_audit.py` compares every `BITFIELD x:1` run of every header (153 headers,
   1,232 classes and script structs, 1,297 DWORD units) against the cooked class's masks. It reported 8 units;
   5 were the reference engine's `_DEPRECATED` renames of a bool at the same bit (not faults), 1 is a native
   struct whose cooked mirror retail's own C++ does not follow either (allow-listed with the measurement, §2),
   and 2 were real: C++ `bPauseAnims` sat on retail's `m_bDisableFaceFx` and shifted `m_bSkipUpdate` /
   `bEnableLineCheckWithBounds` one bit up, and `bUpdateComposeSkeletonPasses` / `bValidTemporarySavedAnimSets`
   read two 2013 bools (`bNeedsInstanceWeightUpdate`, `bAlwaysUseInstanceWeights`) this tree never declared on
   the component. Found 8, fixed 2, left 0 unexplained (§2).
2. **The audit is in the gate**: `verify_phase2.py retail` runs it as a third row, `verify_phase2_total` is 3 in
   `regression_baseline.json` (a data change, said here). It reads `mismatches=0 read=0 missing=3 allowed=1`.
3. **A regeneration now diffs clean**: `gen_classes_header.py --sdk --module-header --sources-cmake --out-dir
   <scratch>` for the four `--sdk` modules produces **100 files byte-identical to the tree** (§4).
4. The generator learned the hand-edits through its own mechanisms, never by special-casing a file: interface
   CppText (FI's three virtuals), `<Module>Native.h` in the first header (AN's Wwise include), a class the tree
   declares elsewhere is not a shim (FS's `UArkAnimNodeStack`, whose registration moved to the Engine where it
   belongs), link anchors generated from `void DishonoredLink<X>()` definitions (FI/FK/FN's three hand tables),
   ported natives detected from their exec bodies (seven stubs FP/FN had deleted by hand and never listed),
   hand-maintained `Sources.cmake` files left alone, and LF output (§3).
5. **The brief's second premise was wrong in a useful way**: the Engine edits it named (FP's
   `Pre/PostCommitMapChange`, FS's `SlotNodes` type and `UArkAnimNodeStack` class, FT's `ChildrenHasChanged`)
   are in reference-engine headers the generator never writes - Engine is not an `--sdk` module - so no
   regeneration can drop them. Of FR's "long list of CppText includes", every one already regenerated cleanly
   (the generator emits an include whenever the CppText file exists). The real drift was smaller and elsewhere
   (§5).
6. Gate: **47 ok, 0 failed, 0 skipped** (2,511 s), built inside the harness on this worktree; `verify_phase2_passed 3 / total 3`,
   every `*_criticals` 0, `unported_natives 2` at its named bound; the gate's build log has 0 errors, 0 C4263, 0 C4264. The
   working build: 1,002 steps, **0 errors, 0 C4263, 0 C4264**.

## 1. The audit: `resources/tools/bitfield_audit.py`

The rule it checks is the one FS measured: `UStruct::Link` (`Core/Src/UnClass.cpp:344`, `SavedBitMask`) restores
each `UBoolProperty`'s `BitMask` from the cooked class after linking, so a reflected bool's bit is the cooked
package's, whatever order the C++ declares. The C++ side is MSVC's packing: adjacent bit fields of 4-byte integral
types share one allocation unit until 32 bits are used; access labels, comments and preprocessor lines do not
break a run, any other declaration does.

Inputs: `retail_sdk_layout.json` (`bitfield: true`, `mask`, per class and per struct), `script_classes_2013.json`
(the fallback for the classes the dump lacks - `ADishonoredNPCPawn` and the like: consecutive `BoolProperty`
children share one DWORD in order), `native_class_sizes.csv` (which names are native classes), every `*.h` under
`source/Development/Src/*/{Inc,Src}` (comments stripped, class bodies brace-matched, nested bodies skipped), and
every module's `Sources.cmake` exclude list (to count a wrong bit's references in **compiled** units only).

What it does per unit: picks the cooked DWORD with the most names in common, compares each shared name's declared
bit against `log2(mask)`, accepts a C++ bit the dump has no property for (a native-only bool) at any bit the cooked
masks leave free and reports it as a clash where a cooked bool owns that bit, lists cooked bools the C++ lacks, and
treats `X_DEPRECATED` / `XDeprecated` as the same name as `X` (the reference engine renames a bool it stops using;
the cooked class keeps the old name - same bit, not a fault). `--verbose` also prints the units that line up, the
native-only runs and the units that merely lack a trailing cooked bool; `--fix` rewrites a mismatching run in the
cooked order (it was used for the two fixes below and the result reviewed by hand; it keeps a run's comment lines
ahead of the reordered members, so a comment that belongs to one member is moved back by hand); `--json` dumps
the findings. The allow list is `resources/docs/types/bitfield_audit_allow.txt` (`Class.member reason`, `Class.*
reason`, `Class.cppname=cookedname`).

Summary line, which `verify_phase2.py` reads: `bitfield_audit: classes=1232 units=1297 mismatches=0 read=0
missing=3 allowed=1`. `read` is static - a wrong bit's name occurring in a compiled `.cpp` - not a proof the path
is reached; both real faults were read (5/7 and 2/1/2 references).

Runtime 6 s. It needs the gitignored `resources/docs/types/` inputs, like the rest of the layout stage.

## 2. Every mismatch found, with its disposition

| class, DWORD | declared vs cooked | disposition |
|---|---|---|
| `USkeletalMeshComponent` +760 (`UnSkeletalMesh.h:334`) | C++ `bPauseAnims` at bit 23 where the cooked class has `m_bDisableFaceFx`; `m_bDisableFaceFx` / `m_bSkipUpdate` / `bEnableLineCheckWithBounds` one bit high (24/25/26 vs 23/24/25). Retail has no `bPauseAnims` (a reference-engine bit; the 2012 PDB's bit 7 of that DWORD was `bRootMotionModeChangeNotify`, which 2013 removed) | **fixed**: `bPauseAnims` moved after `bEnableLineCheckWithBounds` (bit 26, a bit the cooked masks leave free). Read: `m_bSkipUpdate` 1, `bEnableLineCheckWithBounds` 2, `bPauseAnims` 2 references in compiled code |
| `USkeletalMeshComponent` +776 (`UnSkeletalMesh.h:363`) | C++ `m_bDontUpdateKinematic, bRecentlyRendered, bCacheAnimSequenceNodes, bUpdateComposeSkeletonPasses, bValidTemporarySavedAnimSets`; cooked inserts `bNeedsInstanceWeightUpdate` (0x8) and `bAlwaysUseInstanceWeights` (0x10) before the last two. Retail 2013 reflects those two on the component **as well as** on `FSkelMeshComponentLODInfo` (where the reference, the 2012 PDB and this tree have them; the struct's own masks 0x1/0x2 @12 match) | **fixed**: the two declared on the component at bits 3/4. Read: `bUpdateComposeSkeletonPasses` 5, `bValidTemporarySavedAnimSets` 7 references. Every existing use of the two names is qualified through a `FSkelMeshComponentLODInfo` (`LODInfo(i).`, `MeshLODInfo.`), so nothing re-resolves |
| `FMaterialViewRelevance` (`Scene.h:1139`) | ours: `bOpaque, bMasked, bTranslucency, bDistortion, bBloomPart, bOneLayerDistortionRelevance, bInheritDominantShadowsRelevance, bLit, bUsesSceneColor, ...` (15 bits); cooked mirror `Engine.PrimitiveComponent.MaterialViewRelevance`: `bOpaque, bTranslucent, bDistortion, bBloomPart, bOneLayerDistortionRelevance, bLit, bUsesSceneColor` (7) | **allow-listed, with the measurement**: retail's own C++ does not follow the cooked mirror either - the 2012 PDB lays it out `bOpaque 0, bMasked 1, bTranslucency 2, bDistortion 3, bBloomPart 4, bOneLayerDistortionRelevance 5, bLit 6, bUsesSceneColor 7` (`types.json`), so the mirror's masks only reach UnrealScript and serialisation; the one reflected member of this type in the whole dump, `UParticleSystemComponent::CachedViewRelevanceFlags`, is transient and filled by C++, and no function parameter is of this type. The cooked order is not this struct's truth; the PDB is, and ours differs from the PDB only by `bInheritDominantShadowsRelevance` at bit 6 (a reference bit retail lacks), which no raw-bit code here reads (retail's `mRaw` union does not exist in this tree) |
| `UMaterial` +752, +756; `USeqAct_CameraLookAt` +248; `UTexture` +56; `UStaticMeshComponent` +484 | `bAllowFog_DEPRECATED` / `bUsedWithFogVolumes_DEPRECATED` / `bUsedWithFracturedMeshes_DEPRECATED` / `bIsFallbackMaterial_DEPRECATED` / `bAdjustCamera_DEPRECATED` / `CompressionNoMipmaps_DEPRECATED` / `bOverrideLightMapResolution_DEPRECATED` vs the cooked `bAllowFog` ... ; `bDrawAfterFog` vs cooked `bDrawAfterFogDeprecated` | **not faults**: the same bit under the reference engine's rename; the audit normalises the suffix and reports them as `ok` |
| `UPhysicsAssetInstance` (`m_bIsFull`), `URB_BodySetup` (`bIncludeInPhysicsSensor`, `m_bSkipKinematicUpdate`), `ASplineLoftActor` (`bAllowCullDistanceVolume`) | the declared bits line up; the cooked class has one or two **trailing** bools the C++ never declared | **informational** (`missing=3`): nothing reads the wrong bit; a port that needs them declares them at the end of the run |

Count: **8 found, 2 fixed, 0 left unexplained** (5 renames, 1 allow-listed with its measurement). Both fixes are
in `Engine/Inc/UnSkeletalMesh.h`, class `USkeletalMeshComponent`; same DWORDs, same `sizeof`; the layout probe
(`probed=2341 exact=2316 contract_mismatches=0`, `types=2314 mismatching=0`) is unchanged by construction, since
`offsetof` cannot name a bit - which is exactly why the audit exists beside it.

**Why only two.** The four `--sdk` modules' headers are generated and the generator already sorted every bitfield
by cooked mask (`LayoutEmitter.emit_members`), so every DishonoredGame, GFxUI, AkAudio and OSS class was right.
The Engine's reference `*Classes.h` are in the reference `.uc` order, and Arkane's additions to those classes are
appended at the end of a run in almost every case (`UInterpTrack` was the exception FS hit: its seven new bools
sit in the middle). The hand-maintained NOEXPORT headers (`UnInterpolation.h`, `UnSkeletalMesh.h`, `UnActorComponent.h`,
`UnPrim.h`, `Scene.h`, ...) are where the reference engine's version and retail's diverge, and that is where both
real faults were. `UInterpTrack` itself now reads `ok` (FS's order).

**Classes FZ and GA touch**: neither package declares `USkeletalMeshComponent`; no DishonoredGame class had its
bitfield changed. They should expect the audit to fail their merge if a class they fill declares a `BITFIELD` run
in a non-cooked order - the summary names the class, header and line.

## 3. The generator: what it learned, in its own mechanisms

`gen_classes_header.py <Module> --sdk --module-header --sources-cmake`, all three flags, with a new `--out-dir DIR`
that writes `<DIR>/<Module>/{Inc,Src,Sources.cmake}` while reading everything (CppText files, ported lists, the
skeleton units, the tree's declared types) from the tree - the regenerate-and-diff loop the brief asked for.

| hand-edit | who | mechanism now |
|---|---|---|
| three inline virtuals on `IDisSpawnRequesterInterface` in `dishonoredgameclasses.h` | FI | moved into `Inc/CppText/IDisSpawnRequesterInterface.h`; the generator already included an interface's CppText when the file exists (agent AJ's rule) |
| `#include "CppText/UDisSeqAct_StartSpawn.h"` / `UDisSeqVar_SpawnerPawn.h` inserted above `DECLARE_FUNCTION` | FI, FN | the generator's position is just before `};`; the tree now has them there (the only difference was the place, both compile) |
| three link-anchor tables appended to `DishonoredGameRegistrants.cpp` | FI, FK, FN | `link_anchors()`: every `void DishonoredLink<X>()` defined at file scope in a compiled unit of `Src/` is referenced from one generated table `GDishonoredGameLinkAnchors[]` - when the generator owns the module's `Sources.cmake` (it owns the exclude list, so it owns the anchors) |
| seven native stubs deleted by hand, never added to a `.ported*.txt` (`UDisGFxMoviePlayerBase::execAdvance/Close/Start/PreLoad`, `UDisSeqVar_SpawnerPawn::execGetObjectValue`, `ADishonoredPlayerController::exec[Pre]SetCinematicMode_Native`) | FP, FN, earlier | `sdk_main` scans the module's compiled units for `void <Class>::exec<Native>( FFrame&` definitions and treats those as ported; the lists still work, they are no longer the only way. A regeneration had been putting the seven stubs back as duplicate definitions |
| `UArkAnimNodeStack` out of `DishonoredGameEngineShims.h` | FS | already excluded by the shim rule (`cpp not in d.declared` - the Engine declares it); what the tree still had was FS's deliberate residue (a forward declaration, `UArkAnimNodeStack::StaticClass();` in `AUTO_INITIALIZE_REGISTRANTS_DISHONOREDGAME___SHIMS`, `VERIFY_CLASS_SIZE_NODIE`, the `sizeof == 240` assert in `DishonoredGameLayouts.h`). The `StaticClass()` line is load-bearing - it is what registers the class before the cooked packages look it up - so it and the VERIFY line now live in `EngineAnimClasses.h`'s own macro and block, and the shim header is the generator's |
| `#include "AkAudioWwise.h"` and two forward declarations hand-added to `AkAudioClasses.h` | AN | `<Module>Native.h`, if present, is now included by the first generated class header as well as by `<Module>.h`; `AkAudioNative.h` carries the Wwise include and the forwards (`UAkBank` / `UAkEvent` are `EngineArkaneClasses.h` classes). `Engine/Src/akevent.cpp` keeps including `AkAudioWwise.h` directly |
| `GFxUI`, `AkAudio`, `OnlineSubsystemSteamworks` `Sources.cmake` hand-maintained (the Epic units, the `DISHONORED_WITH_GFX3` blocks) | T, AN, X/AM, BB, DC | a `Sources.cmake` whose first line is not the generator's banner is never rewritten (a note is printed). `--sources-cmake` had been one wrong invocation away from erasing them |
| `DishonoredGame/Sources.cmake`'s banner said 806 skeletons | several | the list was right, the count stale; regenerated: 776 |
| `UInterpTrack`'s bitfield (`UnInterpolation.h`) | FS | an Engine header - not generated. The audit of §1 is its source of truth from now on, in the gate |
| `Pre/PostCommitMapChange` on `AGameInfo`, `SlotNodes`' type, `UArkAnimNodeStack` in `EngineAnimClasses.h`, `ChildrenHasChanged` | FP, FS, FT | Engine headers - the generator never writes them; nothing to teach (§5) |
| CRLF output | - | `.gitattributes` says `eol=lf` and the checkout is LF; the writer emitted CRLF, so every regeneration left ~100 files "modified" by line endings alone. It writes LF now |

Everything the tree needs from the generator is written in its module docstring (the `--sdk` contract).

## 4. Proof: the regeneration diffs clean

```
for M in DishonoredGame GFxUI AkAudio OnlineSubsystemSteamworks:
    python resources/tools/symbols/gen_classes_header.py $M --sdk --module-header --sources-cmake --out-dir <scratch>
cmp every generated file against source/Development/Src/$M/...
```
**100 files compared, 0 differ** (DishonoredGame: 73 class headers + `DishonoredGame.h`, `Names.h`, `Layouts.h`,
`Registrants.cpp`, `NativeStubs.cpp`, `Sources.cmake`; GFxUI 4+5; AkAudio 1+5; OSS 1+5; the three hand-maintained
`Sources.cmake` are left untouched by the generator and so are trivially equal). Before this package the same loop
differed in 12 files (the §3 table; the `Sources.cmake` of the three small modules would have been overwritten).

The tree itself was then regenerated once (not hand-edited) so that what it holds is the generator's output: the
`git diff --name-only` of the package is 14 modified + 4 new files, all named in `build/agentFX_sync.py`.

## 5. Premises of the brief, corrected

| the brief said | what is measured |
|---|---|
| "hook it into `verify_phase2.py` so it runs in the gate's **build** stage" | `verify_phase2.py retail` is run by `run_regression.py`'s **layout** stage (`stage_layout`, after `xcheck_sdk_layout.py` and `gen_layout_probe.py compare`); the build stage runs `build-release.cmd` only. The audit row is in the layout stage, which is where a declaration-order check belongs |
| "`gen_classes_header.py --sdk --module-header --sources-cmake` will drop or reintroduce every one of them" - FP's `Pre/PostCommitMapChange`, FS's `UArkAnimNodeStack` into the Engine and the `UInterpTrack` bitfield, FT's `ChildrenHasChanged` | The generator writes the four `--sdk` modules only. `EngineClasses.h`, `EngineAnimClasses.h`, `EnginePawnClasses.h`, `UnInterpolation.h` are reference-engine headers nobody regenerates; a run cannot drop those edits. What a run **did** drop or reintroduce, measured by diffing: FI's interface virtuals, FI/FN's include position, the three anchor tables, seven native stubs (reintroduced as duplicates - a build break), FS's shim residue, AN's Wwise include, and three hand-maintained `Sources.cmake` files (erased) |
| "FR (a long list of CppText includes 'inserted as the generator would')" | All 28 of FR's includes regenerated byte-identical before any change here - FR's `insert_cpptext_includes.py` put them exactly where the generator does. FI's and FN's two were the ones in a different place |
| "Every other NOEXPORT class declared in reference order can carry the same fault" | True in principle, and in practice two classes did, both `USkeletalMeshComponent`; the 1,230 others line up or differ only by trailing bools. The generated modules were never at risk (sorted by mask since the `--sdk` writer was written). The 12 `DECLARE_CLASS_NOEXPORT` files are not the whole surface either: 153 headers declare bitfields, and the audit walks all of them |
| "list every mismatch with ... whether any mismatched bit is read on a reached path" | "Reached" is a runtime property this package did not measure (no run of its own); the audit reports **referenced in compiled code**, which is the static upper bound, and both real faults are referenced from `UnSkeletalComponent.cpp` / `UnSkeletalRender.cpp` paths every skeletal mesh takes |
| "a bitfield-order source of truth (the cooked class, which also closes half 1 permanently)" | For the generated modules the generator was already that source. For the hand-maintained Engine headers the source of truth is the audit in the gate: no generator writes them, so a check is the only thing that can close it permanently |
| the two-build rule | kept: `build/agentFX_rel` (fx_build.cmd) and the gate's `build/agentFX_reg`, nothing else; the harness's own `CoreSmoke` and `LayoutProbe` targets are part of the gate's build |

## 6. Addresses

None cited by this package beyond those already resolved by the agents whose edits it carried (FI's `0x65e790`,
`0x78acb0` in the CppText comment are FI's, unchanged). The source lines cited are `Core/Src/UnClass.cpp:344`
(`SavedBitMask`), `Engine/Inc/UnSkeletalMesh.h:334` and `:363`, `Engine/Inc/Scene.h:1139`.

## 7. Acceptance

| | |
|---|---|
| `run_regression.py` | **47 ok, 0 failed, 0 skipped**, 2,511 s; `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`, `verify_phase2_passed 3`, `verify_phase2_total 3`, `nullrhi/d3d9/inputtest/newgame/options_criticals 0`, `unported_natives 2` (bound 2), `newgame_on_mission 118`, `newgame_pawns 29`; the gate's `verify_phase2.txt` reads `bitfield_audit: classes=1232 units=1297 mismatches=0 read=0 missing=3 allowed=1`. `build/agentFX/fx_regression.log`, `build/agentFX_reg/regression/summary.txt`; the worktree's own copy, absolute build dir `D:/RecompileDishonored/Recompile/build/agentFX_reg`, built inside the harness (the package's second build), `--newgame-at 50 --game-extra-args=-windowed`, the five gitignored `resources/docs/types/` inputs present; the gate's own build log: 0 errors, 0 C4263, 0 C4264 |
| the working build | `build/agentFX_rel`, Release, `DISHONORED_LAYOUT_CHECKS=ON`, through `build_queued.py` (`fx_build.cmd`, `fx_build0.log`): 1,002 steps, 0 errors, 0 C4263, 0 C4264 |
| `verify_phase2.py retail` | 3/3 on the latest probe at hand (`build/agentFW_reg/layout_probe.txt`): `probed=2341 exact=2316 contract_mismatches=0`, `types=2314 mismatching=0 contract_mismatches=0`, `bitfield_audit: classes=1232 units=1297 mismatches=0 read=0 missing=3 allowed=1` |
| regeneration | 100 files byte-identical (§4) |
| `build/agentFX_sync.py` | 20 files, 0 flagged against main `f14326b`, self-checked: `git diff --name-only f14326b` + `ls-files --others --exclude-standard source resources` = the list (README.md and PHASE18.md are main's own merge, excluded by name) |
| headers | regenerated for DishonoredGame, GFxUI, AkAudio, OnlineSubsystemSteamworks (the deliverable); the tree differs from before in exactly the §3 table: `dishonoredgameclasses.h` (interface include), `dishonoredgamekismetclasses.h` (two include positions), `DishonoredGameEngineShims.h` (shim residue out, `DishonoredGameNative.h` in), `DishonoredGameLayouts.h` (one assert out), `Sources.cmake` (count), `DishonoredGameRegistrants.cpp` (one anchor table for three), `AkAudio.h` / `AkAudioClasses.h` (Native.h include, include positions), `OnlineSubsystemSteamworksClasses.h` (include position). No reflected member added or moved |
| the four rules | no launch of any kind by this package; the gate's runs are the harness's, all `-windowed`, input only through the harness's `-inputcmd` scripts; two builds; no retail launch |
