# Retail SDK dump (`D:\RecompileDishonored\Dishonored_DumpedSDK_Retail`)

A CodeRed-Generator (v1.1.5, "DishonoredSDK 1.4") dump taken **inside the running retail 2013
exe**. It walks `GObjects` and writes one C++ declaration per reflected class / script struct /
function, annotated with the values the live engine holds: `UProperty::Offset`, `ElementSize`,
`PropertyFlags`, bitfield masks, `UFunction::FunctionFlags`, parameter layouts. Kept outside the
repo (it is a helper, not a source); parsed on demand by `resources/tools/sdk/parse_codered_sdk.py`
into `resources/docs/types/retail_sdk_layout.json` (git-ignored) + `retail_sdk_layout.md`.

| Package | Classes | Structs | Functions |
|---|---:|---:|---:|
| Core | 43 | 50 | 251 |
| Engine | 1,002 | 369 | 1,221 |
| GameFramework | 38 | 12 | 32 |
| IpDrv / WinDrv / OnlineSubsystemSteamworks | 2 / 2 / 2 | 0 / 0 / 11 | 1 / 0 / 55 |
| GFxUI / AkAudio | 18 / 19 | 9 / 2 | 132 / 0 |
| DishonoredGame | 1,870 | 665 | 875 |
| DishonoredGameContent | 42 | 0 | 0 |

3,038 classes, 15,234 reflected members, 943 script structs, 4,548 exec-parameter structs.

## Why it matters: runtime offsets of the RETAIL exe

Cooked packages carry member lists but not offsets (`UStruct::Link` recomputes them at load), the
2012 PDB carries offsets but of the wrong build, and the retail exe has no symbols. The dump is the
only source that states, for every script-visible member, **the offset the retail `Link` computed**,
plus each script struct's size and alignment padding and each class's reflected span. That turns
the layout convergence from "sizes only" (`native_class_sizes.csv`) into offset-level checking:

- `python resources/tools/sdk/xcheck_sdk_layout.py build/<dir>/layout_probe.txt` compares every
  probed type's member offsets and span against the dump → `resources/docs/types/retail_sdk_delta.md`.
  First run (2026-09-27, `build/coord`): 1,132 probed Core+Engine types exist in the dump, 666 exact,
  234 differ (97 of them "too small": our sizeof ends before retail's last reflected member).
  Examples it caught immediately: `APawn` lacks `FRBCollisionChannelContainer PushBoxCollisionChannel`
  @1048 (every later member is 4 bytes early); `AMatineePawn::PreviewMesh` is at 1184 in retail (so
  retail was built `WITH_EDITORONLY_DATA=1`, like us, and the class is 1200 = 1188 aligned to 16);
  `UAnimNode*`, `UActorFactory*`, `ACamera`, `AGameInfo`, `ANavigationPoint` … are still at the
  unconverged reference layout.
- `UnknownDataNN` gaps (115 of them) mark where retail has native-only members between reflected
  ones; together with the 2012 PDB member list they pin down the retail position of the C++-only
  members.
- **State after wave 2 (2026-09-25)**: `sdk_props.py` regenerated 216 Engine PROPS blocks and
  `gen_classes_header.py --sdk` generated the DishonoredGame/GFxUI/AkAudio/OSS headers from the dump;
  the cross-check now covers 2,314 dump types (derived structs included): 1,677 exact, 4 rows left,
  0 contract mismatches.

## Where it is used in the plan

| Phase / package | Use |
|---|---|
| Phase 2b (layout truth) | third retail source next to `native_class_sizes.csv` (exe) and `script_classes_2013.json` (packages): offsets. `xcheck_sdk_layout.py` is the offset-level exit check; contract types must show 0 mismatches. |
| Phase 3 wave 2 (Engine convergence per module) | work list = `retail_sdk_delta.md`; each agent converges its module's classes until its rows disappear. Bitfields: the probe cannot `offsetof` them, so compare the dump's DWORD offset + mask against the header by eye. |
| Phase 3 DishonoredGame headers | `gen_classes_header.py` gets a `--sdk` input: 1,870 DishonoredGame classes and 665 structs with retail offsets, sizes and property flags, plus `UnknownData` gaps for the native members. The 2012 PDB only supplies names/types for those gaps. |
| Phase 3 natives / events | `*_parameters.hpp` = exact exec-parameter struct layouts (`FFrame` argument blocks) for 4,548 functions: generates the `eventFoo(...)` / `DECLARE_FUNCTION(execFoo)` wrappers with the right parameter order, sizes and `CPF_OutParm`/`CPF_ReturnParm` flags. `FunctionFlags` per function (`FUNC_Native`, `FUNC_Event`, `FUNC_Exec`, `FUNC_Simulated`, …) come from the same headers. |
| Phase 7 (2012 → 2013 delta) | `member_delta.py` / `script_delta_2012_2013.md` gain offsets: for every added retail member the dump says where it landed. |
| Phase 8 / dismod compatibility | dismod and every CodeRed-style tool address objects by these offsets. The rebuilt exe keeps `GObjects`/`GNames` and these reflected layouts so such tools keep working against it; the dump is the compatibility spec. |

## What it cannot tell (caveats)

- **Native-only members are invisible.** A class span ends at the last reflected property; the C++
  `sizeof` (retail descriptor, `native_class_sizes.csv`) is often larger (`AActor` 584 vs 592,
  `UMeshComponent` 468 vs 480). Never take a span end as the class size.
- **`iNative[n]` is garbage** (the generator read the wrong `UFunction` field for this engine
  version: values like 14510 or 6316). Numbered natives come from `resources/docs/symbols/natives_2013.csv`.
- Core reflection-less types (`FName`, `FString`, `TArray`, `FArchive`, `ULinkerLoad`, …) are only
  present as the generator's own hand-written definitions (`include/defs.hpp`), not as dumps.
- `UClass` shows as one 236-byte `UnknownData` block after `UState` (no reflected members): only the
  total (436) is usable, which agrees with `retail_reconciliation.md`.
- Enums are emitted as `uint8_t`/enum names without values in the class headers; values are in
  `script_classes_2013.json`.
- The dump reflects one process at one moment: `CPF_Transient` / `CPF_Config` flags are from the
  live `UProperty`, and any class the game never loaded is absent (DishonoredGameContent has 42).

Related: `resources/docs/types/xcheck_sdk.md` (Phase 1, 2012 PDB vs the older CodeRed defs shipped
with dismod: only six hand-written Core types), `retail_reconciliation.md` (retail sizes applied so far).
