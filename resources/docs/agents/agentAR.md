# Agent AR report — the world's textures are colour noise (2026-09-26)

Package **AR** of wave 5 (`PHASE7.md`), the user's first defect. Build dir `build\agentAR` (Release,
`set BUILD_DIR=build\agentAR` + `resources\build-release.cmd`), built and run on the **shared working tree**
(agents AS and AX had `UnActor.cpp`, `PrimitiveComponent.cpp` and the tools in flight the whole time; no file of
mine is touched by them, so no snapshot was needed). IDA copy `resources\docs\idb\retail2013_agentAR.i64`,
headless `decompile_funcs.py` only, decompiles in `build\agentAR\decomp13`. Patch scripts
`build\agentAR_patch1.py` (the census), `_patch2.py` (`-distexfill`), `_patch3.py` (`-disnostreamio`),
`_patch4.py` (`-disstreamsync`), `_patch5.py` (**the fix**), `_patch6.py` (removes the two
investigation-only switches). No commits, nothing staged. Status rows: `agentAR_status.csv`.

## Result

**Arkane's cook splits a streaming texture's mip chain across two files, and the engine only ever read one of
them.** The upper mips go into `TextureFileCacheName`'s `.tfc` with `BULKDATA_StoreInSeparateFile`; the lower
ones stay inline in the package. `FTexture2DResource::LoadMipData` (retail 2013 rva **0x16b090**) issues every
streamed request against the single `Filename` the resource was built with, which for those textures is the
`.tfc`. A level the cook left inline therefore cannot be streamed at all: its `BulkDataOffsetInFile` is a
*package* offset, and the read lands on unrelated bytes in `Textures.tfc` / `CharTextures.tfc` /
`Lighting.tfc`. With `GMinTextureResidentMipCount` (7) as the only floor, the first inline level sits exactly
one below the resident set, so **every streaming texture in the world got one level of garbage**, and it is the
level the big near surfaces sample.

| Measure (L_Pub_Day_P, d3d9 1280x720, `-benchmark -fps=30`) | Before (`-disnotfcfloor`) | After |
|---|---:|---:|
| streamed mip levels loaded **from the wrong file** | **2,102 of 3,842** (58 s run); **1,779 of 3,260** (40 s run) | **0 of 1,376** |
| streamed levels that went down the plain-I/O branch | 2,102 — *every one of them* a level not in the `.tfc` | 0 |
| streamed levels that went down the compressed branch | 1,740 — every one of them in the `.tfc`, all correct | 1,376 |
| textures created | 2,753 | 2,753 |
| mip levels created / filled / missing | 17,081 / 17,081 / 0 | 19,848 / 19,848 / 0 |
| scene census (AP's) prims / static elements / base pass | 5,039 / 5,195 / 5,108 | 5,039 / 5,195 / 5,108 |
| draw list elements drawn per frame | 187 of 12,995 | 187 of 12,995 |
| scene frames in 75 s | ~16,000 (542 census lines every 30th frame) | ~16,000 (542 census lines) |
| critical errors, 75 s | 0 | 0 |

The stream counters are cumulative, so their absolute values depend on how long a run lasts; the invariant is
the one that matters — **before the fix the plain-I/O branch and the "not in the `.tfc`" counter are the same
number, i.e. every level that did not come from the `.tfc` was read from the `.tfc` anyway; after it, both are
zero.**

`L_Tower_P` over the same run: 16,257 levels created, all filled, **0 not in the `.tfc`**, 1,859 stream
requests, 1,852 finalized, 0 failed, 0 criticals; AP's scene census still 2,672 prims / 2,721 static elements /
2,683 base pass adds. Null RHI (`--rhi null`, `L_Tower_P`, 75 s): exit 0, `Finished loading level`,
0 criticals, 0 levels not in the `.tfc`.

### Before / after

Exactly matched pairs: same map, same `-apshot=N`, same `-benchmark -fps=30` fixed time step, the "before" taken
with `-disnotfcfloor` (the bring-up switch that reproduces the defect on the fixed exe).

| | before | after |
|---|---|---|
| `L_Pub_Day_P`, frame 40 | `build\agentAR\shots\before_pub_f40.png` | `build\agentAR\shots\after_pub_f40.png` |
| `L_Pub_Day_P`, frame 150 | `build\agentAR\shots\before_pub_f150.png` | `build\agentAR\shots\after_pub_f150.png` |
| `L_Tower_P`, frame 40 | `build\agentAR\shots\before_tower_f40.png` | `build\agentAR\shots\after_tower_f40.png` |
| `L_Tower_P`, frame 300 (river, riverbed, Dunwall skyline) | — | `build\agentAR\shots\after_tower_f300.png` |

In the before shots the pub's ceiling, floor, railings and the boat's deck, carpet and crates are rainbow
blocks; in the after shots they read as concrete, planking, painted wood, carpet and stone. The user's original
exhibit `build\agentAP\shot_pub.png` is the same frame as `before_pub_f40.png`.

Accept commands (both exit 0, `--forbid Critical` clean; transcripts `build\agentAR\accept_pub.txt`,
`accept_tower.txt`, `accept_null.txt`):

```
python resources\tools\build_and_smoke.py --build-dir build/agentAR --no-build --exe-name DishonoredGame_AR.exe ^
  --log-name agentAR_accept_pub.log --ini-dir build/agentAR/config --rhi d3d9 --timeout 75 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "scene census" --expect "texture census" ^
  --forbid "Critical" --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Pub_Day_P -startmapopen -apshot=40 -benchmark -fps=30 -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
    ... same with -startmap=L_Tower_P
```

`-benchmark -fps=30` is worth knowing: without a fixed time step the pawn's fall makes the camera land
somewhere different every run and no two screenshots are comparable. With it, `-apshot=N` is reproducible
frame for frame.

## 1. Characterising it first: the texture census

Nothing was changed until the defect had a number. One `DISHONORED(bringup)` pair of lines a second, emitted
from `FStreamingManagerCollection::UpdateResourceStreaming`, in the style of agent AP's scene census
(counters declared in `UnTex.h`, defined in `Texture2D.cpp`):

```
DISHONORED(bringup): texture census: 2753 created (2163 DXT1, 0 DXT3, 298 DXT5, 0 BC5, 11 ARGB, 271 G8, 10 other),
  groups 901 lightmap/1281 world/41 char/530 other, 2435 streamed + 318 resident, mips 1..13 (0 with no mip tail)
DISHONORED(bringup): texture census: upload 19848 levels created, 19848 filled, 0 missing over 0 textures,
  552 pitch mismatches, 0 size mismatches; stream 1045 requests, 8761 shared levels copied (0 with none),
  new levels 0 inline/0 io/1376 io-compressed (0 NOT in the tfc), 0 pitch mismatches, 0 size mismatches,
  1042 finalized, 0 failed
```

| World | textures | DXT1 / DXT5 / G8 / ARGB / other | groups (lightmap / world / char / other) | streamed / resident | mip range | levels created / filled / missing |
|---|---:|---|---|---|---|---|
| `L_Pub_Day_P` | 2,753 | 2,163 / 298 / 271 / 11 / 10 | 901 / 1,281 / 41 / 530 | 2,435 / 318 | 1..13 | 19,848 / 19,848 / 0 |
| `L_Tower_P` | 2,304 | 1,755 / 300 / 231 / 11 / 7 | 775 / 851 / 133 / 545 | 1,983 / 321 | 1..13 | 16,257 / 16,257 / 0 |

Every created texture has a complete mip chain: **`levels created == levels filled`, 0 missing, 0 textures with
holes**, in both worlds and under both RHIs. No DXT3, no BC5 and no V8U8 normal maps survive the cook for world
geometry: normal maps are DXT5 (`fmt 5` in the per-level lines is DXT1, `7` DXT5).

Where the counters sit (all `DISHONORED(bringup)`-tagged):

| Counter | Site |
|---|---|
| `GDisTexCreated`, the format and LOD-group counters, `GDisTexStreamable/Resident`, `GDisTexMin/MaxMips`, `GDisTexNoMipTail` | `FTexture2DResource::InitRHI` (Texture2D.cpp) |
| `GDisTexLevelsCreated/Filled/Missing`, `GDisTexWithHoles` | the resident-mip upload loop in `InitRHI`; a hole also logs the texture, mip index and flags |
| `GDisTexPitchMismatch`, `GDisTexSizeMismatch` | `FTexture2DResource::GetData`, `SrcPitch` vs the RHI's `DestPitch` and `EffectiveSize` vs `BulkDataSize` |
| `GDisTexStreamRequests`, `GDisTexStreamSharedCopied/None` | `FTexture2DResource::LoadMipData` / `UpdateMipCount` |
| `GDisTexStreamLevelsInline/IOPlain/IOComp`, **`GDisTexStreamLevelsNotInTFC`** | the three branches of `LoadMipData`'s new-level loop |
| `GDisTexStreamPitchMismatch`, `GDisTexStreamSizeMismatch` | the streamed-level lock in `LoadMipData` |
| `GDisTexStreamFinalizeOk/Fail` | `FTexture2DResource::FinalizeMipCount` |

The first 24 streamed levels also log in full, which is what made the cause visible:

```
DISHONORED(bringup): stream level: Ply_Player.PlayerArms_N owner-mip 3 -> rhi-mip 3 (256x256 fmt 5),
  flags 0x00000019, offset 2076050, on-disk 28352, size 32768, dest pitch 512, group 4
DISHONORED(bringup): stream level: Ply_Player.PlayerArms_N owner-mip 4 -> rhi-mip 4 (128x128 fmt 5),
  flags 0x00000008, offset 4361334, on-disk 8192, size 8192, dest pitch 256, group 4
DISHONORED(bringup): stream level:   mips 12, miptail base 11, lod bias 0, resident 7, requested 12,
  MinResidentMipCount 0, tfc mips 4
```

`0x19` = `StoreInSeparateFile | SingleUse | SerializeCompressedLZO`; `0x08` = `SingleUse` only. Mips 0..3 are in
`CharTextures.tfc`, mips 4..11 are inline in the package — and mip 4's "offset" 4,361,334 is a package offset
being used as a `.tfc` offset. `PlayerArms_D` and `PlayerArms_S` have their inline mip 4 at 4,206,684 and
4,372,915, i.e. next to each other in one package, while their `.tfc` offsets are 4.7 MB, 2.6 MB and 0.5 MB
apart — the three inline offsets cannot be `.tfc` offsets.

## 2. The candidates in the plan, each eliminated or confirmed with evidence

| Candidate | Verdict | Evidence |
|---|---|---|
| lock pitch / row stride when filling a texture | **eliminated** | census: 0 stream pitch mismatches, 0 stream size mismatches. The 552 upload-side "pitch mismatches" are all 1x1 / 2x2 / 2x1 levels of uncompressed formats where D3D9 pads the lock pitch to 4 bytes; `GetData` copies those row by row with `Min(SrcPitch,DestPitch)`, and retail's `GetData` (2013 rva **0x16af50**) is the same function line for line |
| DXT block size and row stride | **eliminated** | `EffectiveSize == BulkDataSize` on all 19,848 levels; `DestPitch` equals `NumColumns*BlockBytes` on every streamed DXT level |
| cooked `FTexture2DMipMap` bulk-data offsets for seek-free packages | **confirmed, but not as a decoding bug**: the offsets are right, they are just offsets *into the wrong file* for the inline levels | `Textures.tfc` at the logged offset starts with `9e2a83c1` (`PACKAGE_FILE_TAG`), chunk size 131,072, a 5-entry chunk table and 4 chunks that decompress to exactly 4 × 131,072 = 524,288 bytes (`build\agentAR_lzotest.cpp`, built against the tree's own lzokay). `DO_CHECK` is on in this build, so `FUntypedBulkData::Serialize`'s `checkf(BulkDataOffsetInFile == Ar.Tell())` passed for every inline mip: those offsets really are package offsets |
| `TEXTUREGROUP` LOD bias selecting a mip that was never uploaded | **eliminated** | `-distexfill` paints every managed level magenta in its own format at `RHICreateTexture2D` time (4,000 textures, 28,880 levels in a pub run). Not one pixel of magenta reached the screen, so every level a visible surface samples *was* written — the bytes were wrong, not missing |
| the streamed-mip request uploading into the wrong mip level | **eliminated**; it is the wrong **file**, not the wrong level | the per-level line shows `owner-mip N -> rhi-mip N` with matching dimensions and pitch throughout |
| the async I/O system itself (`FulfillCompressedRead`, LZO) | **eliminated** | `-disstreamsync` replaced every async request with a plain synchronous `FArchive::Seek` + `SerializeCompressed` on the same file, offset, size and flags: the picture was *identically* corrupt. Separately, lzokay decodes the real `.tfc` chunks to the exact expected sizes with `EResult::Success` |
| shader caches, materials, geometry | not reopened | AG/AP already byte-verified them; the census shows 0 blend-skipped and the scene census is unchanged by this fix |

The three diagnostic frames are kept next to the before/after pairs, all `L_Pub_Day_P` frame 40:
`build\agentAR\shots\fill_pub_f40b.png` (`-distexfill`: the noise is unchanged, no magenta anywhere),
`nostreamio_pub_f40.png` (`-disnostreamio`: every corrupt surface turns magenta, so they all sample a streamed
level) and `sync_pub_f40.png` (`-disstreamsync`: identical corruption through a synchronous read).

The order of the three switches is what narrowed it: `-distexfill` said *written but wrong*, `-disnostreamio`
(paint the streamed level magenta instead of loading it) said *the wrong bytes arrive through the streamed
path*, `-disstreamsync` said *and not because the async I/O system is at fault*. Only then did the per-level
dump of the bulk-data flags make sense.

## 3. The fix

| File | Change | Evidence |
|---|---|---|
| `Engine/Src/Texture2D.cpp` | `UTexture2D::CreateResource` derives **`MinResidentMipCount`** — a member that exists in retail 2013 only (`script_classes_2013.json`, `UTexture2D` @368, the layout note in `EngineTextureClasses.h`) and that **nothing in the tree wrote or read** — as the length of the trailing run of mips without `BULKDATA_StoreInSeparateFile`, for textures that stream from a `TextureFileCacheName`. `RequestedMips` then floors on it next to the existing mip-tail floor | 2013 rva 0x16b090: retail's `LoadMipData` reads every streamed level from the one `Filename` and has no fallback, so the inline run must be resident |
| `Engine/Src/UnContentStreaming.cpp` | `FStreamingTexture` caches it as `MinResidentMips`; `FStreamingManagerTexture::CalcMinMaxMips` floors `MinAllowedMips` on it and `StreamOutTextureData` floors `NumRequiredResidentMips` on it, so the streamer can never shrink back below the inline run and re-create the fault | same |
| `Engine/Inc/UnTex.h`, `Engine/Src/Texture2D.cpp`, `Engine/Src/UnContentStreaming.cpp` | the census of section 1 (27 `extern UINT` declarations, no inline statics in a widely included header) plus `-disnotfcfloor`, which skips the derivation so the defect can be reproduced on a fixed exe for a before/after pair or a regression counter | bring-up only |
| `D3D9Drv/Src/D3D9Texture.cpp` | `-distexfill`, off by default: paints every managed 2D texture's every level magenta (`f81f` endpoints for DXT1/DXT5, `0xffff00ff` for A8R8G8B8, `0xff` bytes otherwise) right after `CreateTexture`, so a level nothing uploads into is unmistakable | bring-up only |

Cost: 17,081 -> 19,848 resident mip levels in `L_Pub_Day_P` (+2,767, +16 %) and 16,008 -> 16,257 in
`L_Tower_P`. Every added level is one the cook deliberately kept in the package — typically a single 128x128
DXT1 level per streaming texture — so this is the residency the cook was built for, not a new cost.

Retail's `FD3D9DynamicRHI::CreateTexture2D` (**0x5bea00**), `LockTexture2D` (**0x5beb80**),
`CopyMipToMipAsync` (**0x5b5db0**), `FTexture2DResource::GetData` (**0x16af50**) and `LoadMipData`
(**0x16b090**) were all decompiled and all match ours function for function; none of them needed porting.
`TexCreate_Dynamic` is `0x40` and the managed-pool test is `(Flags & 6) == 0` in both.

## 4. Follow-ups / hand-overs

1. **`FAsyncIOSystemBase::InternalRead` (`Core/Src/UnAsyncLoading.cpp:934`) never assigns the result of
   `PlatformReadDoNotCallDirectly` to `bRetVal`, so it always returns `FALSE`.** Nothing checks the return today,
   so it did not cause this defect, but it silently disables any future I/O failure reporting. One word.
   Core is nobody's file this wave — coordinator's call.
2. **A missing or unopenable streaming file is silent**: `FAsyncIOSystemBase::Tick` has
   `//@todo streaming: add warning once we have thread safe logging` where `PlatformIsHandleValid` fails, and
   `FulfillCompressedRead` ignores every read result. Had either logged, this defect would have been a
   one-line log read rather than four experiments.
3. **`UTexture2D::MinResidentMipCount` reads 0 in every cooked package examined**, so the cook does not
   populate it; I derive it at `CreateResource` from the bulk-data flags instead. If the retail exe obtains it
   some other way (a `UTexture2D::CreateResource` decompile would settle it — the function is unnamed in
   `retail2013_named.i64` and I did not locate it), that is the remaining convergence step for this member.
4. **`LoadMipData` still carries a reference-only `MipMap.Data.IsBulkDataLoaded()` branch that retail
   (0x16b090) does not have.** The census shows it fires **0 times** in both worlds, so it is dead code, but it
   is the next convergence step in that function.
5. **The null RHI reports every level as a pitch mismatch** (`16008 pitch mismatches` for 16,008 levels)
   because its `LockTexture2D` returns a placeholder pitch. That is a diagnostic artefact of the census, not a
   defect; ignore the pitch columns under `--rhi null`.
6. **The census, `-distexfill` and `-disnotfcfloor` are bring-up aids.** `GDisTexStreamLevelsNotInTFC` is the
   one worth wiring into agent AX's regression harness: it must stay **0**, and `-disnotfcfloor` makes it
   non-zero on demand so the check can be proven to fail.
7. **Seen on the shared tree, not mine: `Critical: appError called: Failed to find function Touch in
   DishonoredWaterVolume L_Pub_Day_Light.TheWorld:PersistentLevel.DishonoredWaterVolume_15`.** It fired once, at
   32 s, in a `-disnotfcfloor` pub run built on 2026-09-26 with agent AS's touch work in the tree, when the
   falling pawn reached a water volume; four 75 s accept runs (two per map) on the same exe never reached it.
   That is **agent AS's** path — a touch notification now dispatches to a script function the class does not
   declare — and it is worth AS knowing that it is reachable from `L_Pub_Day_P` by falling into
   `DishonoredWaterVolume_15`.
8. **Files touched outside the package's list: none.** `D3D9Drv/Src/D3D9Texture.cpp`,
   `Engine/Src/Texture2D.cpp`, `Engine/Src/UnContentStreaming.cpp` and `Engine/Inc/UnTex.h` are all in it;
   `UnLevel.cpp` was not needed, so nothing is handed to AT.
