# Retail (2013) reconciliation of Core — C1 notes

Target: retail `Dishonored.exe` (engine 9411). Sources: `native_class_sizes.csv` (agent H, sizes
of every native class in both exes), `script_classes_2013.json` / `script_delta_2012_2013.md`
(agent I, member lists from the cooked packages), decompiles of retail functions
(`resources/reference/decomp/retail_uclass/`, via the 2013 database).

## Core contract types

All Core contract types (`FName`, `FString`, `FArchive`, `UObject`, `UField`, `UStruct`, `UState`,
`UFunction`, `UProperty` family, `UEnum`, `UScriptStruct`, `UPackage`, `ULinkerLoad`, `FFrame`,
`FStateFrame`, package-summary structs) have **identical sizes in 2012 and 2013** except:

### UClass: 456 (2012) → 436 (2013)

Evidence from the retail `UClass::Serialize` (vftable slot 11 of `??_7UClass` at rva 0xbbe218,
function rva 0x9a6c0) and the retail static constructor (rva 0x77060):

| Member | 2012 offset | 2013 offset | Note |
|---|---:|---:|---|
| ClassFlags | 200 | 200 | |
| m_OtherClassFlags | 204 | 204 | serialized at Ver ≥ 796 in both |
| ClassCastFlags / ClassUnique | 208 / 212 | 208 / 212 | |
| ClassWithin / ClassConfigName | 216 / 220 | 216 / 220 | |
| ClassReps / NetFields | 228 / 240 | **one of them removed** | ctor zero-fills the block; `UClass::Link` in the named retail db decides which (`FRepRecord` is 8 bytes, `UField*` 4) |
| HideCategories / AutoExpandCategories / AutoCollapseCategories / DontSortCategories | 252 / 264 / 276 / 288 | 240 / 252 / 264 / 276 | −12 |
| DependentOn | 300 | 288 | −12 |
| bForceScriptOrder | 312 | 300 | −12 |
| m_DropdownCategory | 332 | **removed** | retail still deserializes the FName (LicenseeVer ≥ 10) into a discarded local, so the package format is unchanged |
| ClassHeaderFilename | 316 | 304 | −12 |
| ClassDefaultObject | 328 | 316 | −12 |
| ClassConstructor / StaticConstructor / StaticInitializer | 340 / 344 / 348 | 320 / 324 / 328 | −20 |
| ComponentNameToDefaultObjectMap | 352 | 332 | −20 |
| Interfaces | 412 | 392 | −20 |
| m_pInterfaceOffsets / DefaultPropText / bNeedsPropertiesLinked / ReferenceTokenStream | 424 / 428 / 440 / 444 | 404 / 408 / 420 / 424 | −20 |

Action for the headers: drop `m_DropdownCategory` (keep the discarded read in `Serialize`), drop
`ClassReps` or `NetFields` once identified, regenerate `DishonoredLayouts.h` with
`sizeof(UClass) == 436`. Pending J (name propagation) for the `UClass::Link` check.

## Script-side deltas relevant to Core

From `script_delta_2012_2013.md`: Core package classes unchanged in member lists; `UObject`
children identical (no `NetIndex`, same natives, `GotoState` 113). Engine/DishonoredGame deltas
are handled in C2 / Phase 3.

## Open

- `UClass::Link` (retail) → which of `ClassReps`/`NetFields` survives.
- Confirm `FPackagePrecacheInfo` (+3 Arkane members, 24 bytes) and `FAsyncIORequest` (`Event` @44,
  `LoadDataWithEvent`) in retail once names are propagated.
- Rerun `LayoutProbe`/`CoreSmoke` after the UClass header change.
