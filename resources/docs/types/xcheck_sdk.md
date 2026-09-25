# Layout cross-check: 2012 PDB types vs 2013 CodeRed runtime dump (dismod defs.hpp)

> Superseded 2026-09-27 by the full retail dump (`D:\RecompileDishonored\Dishonored_DumpedSDK_Retail`,
> `resources/docs/sdk_dump.md`): `retail_sdk_delta.md` checks every probed member of 1,132 types
> instead of the six hand-written Core types below.

Mismatches are expected where the 2013 build added or moved fields; they seed the Phase 7 delta list.

## FName

PDB size 8. | Member | SDK offset | PDB offset | Match |
|---|---:|---:|---|
| FNameEntryId | 0x0 | – | missing in PDB |
| InstanceNumber | 0x4 | – | missing in PDB |

## FNameEntry

PDB size 2064. | Member | SDK offset | PDB offset | Match |
|---|---:|---:|---|
| Flags | 0x0 | 0x0 | yes |
| Index | 0x8 | 0x8 | yes |
| HashNext | 0xC | 0xC | yes |

## FPointer

Not present in PDB types.

## FQWord

Not present in PDB types.

## FScriptDelegate

PDB size 12. | Member | SDK offset | PDB offset | Match |
|---|---:|---:|---|
| Object | 0x0 | 0x0 | yes |
| FunctionName | 0x0 | 0x4 | **no** |

## FString

PDB size 12. | Member | SDK offset | PDB offset | Match |
|---|---:|---:|---|
| ArrayData | 0x0 | – | missing in PDB |
| ArrayCount | 0x8 | – | missing in PDB |
| ArrayMax | 0xC | – | missing in PDB |

Compared 10 members across 6 classes; 6 mismatches.
