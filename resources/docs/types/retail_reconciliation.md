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
| ClassReps | 228 | 228 | kept: retail `UClass::Link` (vftable slot 77, rva 0xa4290, `decomp/retail_uclass/uclass_vslot77_0xa4290.c`) copies the super's array at +228 and appends to it |
| NetFields | 240 | **removed** | retail `Link` never touches +240; `UPackageMap::GetClassNetCache` (reference-only, in neither exe) now gathers CPF_Net / FUNC_Net fields itself (`UnCoreNet.cpp`) |
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

Done 2026-09-27: `UnClass.h` has neither member (`// DISHONORED(layout)` markers), `UClass::Serialize`
reads the FName into a discarded local, `Link`/ctors/`EmitObjectArrayReference` lost their NetFields
code, `Core/Inc/DishonoredLayouts.h` asserts 436. CoreSmoke 99/99, milestone-1 smoke exit 0 after the change.

## Script-side deltas relevant to Core

From `script_delta_2012_2013.md`: Core package classes unchanged in member lists; `UObject`
children identical (no `NetIndex`, same natives, `GotoState` 113). Engine/DishonoredGame deltas
are handled in C2 / Phase 3.

## Engine contract types (C2)

Agent M converged 29 Engine contract types on the 2012 PDB. Retail sizes (`native_class_sizes.csv`)
equal 2012 for all of them except two, and agent I's package member lists explain both exactly:

| Class | 2012 | 2013 | Retail member change (script order from `script_classes_2013.json`) |
|---|---:|---:|---|
| UTexture2D | 368 | 372 | `INT MinResidentMipCount` added after `Timer` (last script property before the `Texture2DMipMap` struct) |
| USkeletalMeshComponent | 1056 | 1088 | `m_FaceFxAudioHandler` renamed `m_pFaceFxAudioHandler` (same slot, after `m_fFaceFxTickTime`); `bRootMotionModeChangeNotify` / `bRootMotionExtractedNotify` bitfields removed (also their `RootMotion*` delegate functions); `FBoneAtom RawExtractedRootMotionDelta` (32 bytes) appended after `m_TickData`; `TickData` flags become `BoolProperty` (4 × BYTE → BITFIELD, no size change per H) |

Done 2026-09-27: `EngineTextureClasses.h` (`INT MinResidentMipCount` after `Timer`; the three
`UTexture2D` subclasses `ULightMapTexture2D` / `UShadowMapTexture2D` / `UTextureFlipBook` grow with it
to 376 / 376 / 436, confirmed by `native_class_sizes.csv`), `UnSkeletalMesh.h` (`m_pFaceFxAudioHandler`
was already the name M chose; the two `bRootMotion*Notify` bitfields and their two uses in
`UnSkeletalComponent.cpp` are gone; `FBoneAtom RawExtractedRootMotionDelta` after `m_TickData`).

### Layout tooling now targets retail

`gen_layout_probe.py compare` and `gen_layout_asserts.py` take the **retail 2013 sizeof** from
`native_class_sizes.csv` for every class that exists in the retail exe and fall back to the 2012 PDB
size otherwise. Member offsets are still only known from the 2012 PDB, so they are checked only where
the retail size equals the 2012 size. Probe after C1/C2: 1,899 types, 1,374 exact, **0 contract
mismatches against retail**. Regenerating `DishonoredLayouts.h` (Core 278 asserts + 12 pending, Engine
669 + 284) changed only the rows below plus the assert message text.

### Remaining Engine classes whose retail size differs from 2012 (non-contract, Phase 3 per-module work)

| Class | 2012 | 2013 | Ours | Retail script delta (`script_delta_2012_2013.md`) |
|---|---:|---:|---:|---|
| AMatineePawn | 1184 | 1200 | 1184 | script members identical; `APawn` gained `PushBoxCollisionChannel` + `m_iPreventRBVelFromAnimFrameCount` yet stays 1184 in the descriptor. Descriptor at rva 0xe9f438 confirms 1200 (`decomp/retail_matineepawn/descriptor_0xe9f438.txt`); resolve with the `APawn` retail pass (pending assert) |
| UArkComponentLocomotionConfig | 332 | 336 | — | `+ FLOAT m_fReturnToNavMeshSpeed` |
| UArkDLCManagementBridge | 96 | 112 | — | `+ TArray<UBOOL> m_aDLC_LicenseOnly` (12) `+ BITFIELD m_bIsInitialized` (4) |
| UDownloadableContentManager | 180 | 192 | 192 | four DLC functions added (no data members in the package); ours already matches retail, now asserted |
| UInterpTrackAIControlBodyIntentionKeyProperties | 64 | 68 | — | `+ BITFIELD m_bHackForbidEquipState` |
| UInterpTrackInstStretchAnimControl | 92 | 100 | — | `+ FLOAT m_fCurrentAnimPosition, m_fBackupAnimPosition` |
| UInterpTrackStretchAnimControl | 164 | 168 | — | `+ BITFIELD m_bSpecialRootMotionExtract` |
| UOnlineSubsystem | 168 | 180 | 252 | `+ TArray<FNamedSession> Sessions` (12); ours is still the unconverged reference layout (pending) |
| USeqAct_SetMatInstScalarParam | 264 | 272 | 264 | `+ AActor* m_pMaterialOwnerPawn, INT m_iMaterialIndex` |

`—` = not reachable from `Engine.h` (not probed). The retail class registration descriptor
(`GetPrivateStaticClass(desc)`, rva 0x79370) starts with the sizeof at +0 (AMatineePawn: 1200 at
rva 0xe9f438, then 4, 0, 0, three pointers, 0x4000, 0x04084084, rva 0x56d810, two nullsubs); the
other fields are not decoded yet. H's `ctor_rva_2012` column can point at a COMDAT-folded function of
another class (AMatineePawn's is `AGamePawn::InternalConstructor`), so verify before relying on it.

## Open

- Confirm `FPackagePrecacheInfo` (+3 Arkane members, 24 bytes) and `FAsyncIORequest` (`Event` @44,
  `LoadDataWithEvent`) in retail (names are propagated now: `retail2013_named.i64`).
- `AMatineePawn` / `APawn` retail pass (table above); the other seven non-contract Engine classes when
  their module code is ported.
- Done 2026-09-27: NetFields resolved (removed), UClass/UTexture2D/USkeletalMeshComponent applied, tooling targets retail sizes.
- Done 2026-09-26: `FPackageInfo` (68 in both builds) had the reference-only `LoadingPhase` byte removed; the three seamless-travel uses in Engine collapse to the non-phased path.
