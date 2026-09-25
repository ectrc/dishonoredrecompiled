# Script opcodes: 2012 exe vs reference UnStack.h / UnCorSc.cpp

UObject natives with GNatives index < 0x80. Identical COMDAT folding merges byte-identical exec
functions (execFalse/execIntZero/execNoObject, ...), so a folded function lists every index it
serves; the name shown is the one the linker kept. Cooked packages contain Dishonored's numbering,
so the ported enum must match the Dishonored column wherever it is not folded-ambiguous.

112 slots, 6 real differences.

| Index | Dishonored (natives.csv) | Reference 10897 | Status |
|---:|---|---|---|
| 0 (0x00) | LocalVariable | LocalVariable | same |
| 1 (0x01) | InstanceVariable | InstanceVariable | same |
| 2 (0x02) | DefaultVariable | DefaultVariable | same |
| 3 (0x03) |  | StateVariable | missing in exe (folded or unused) |
| 5 (0x05) | Switch | Switch | same |
| 6 (0x06) | Jump | Jump | same |
| 7 (0x07) | JumpIfNot | JumpIfNot | same |
| 8 (0x08) | Stop | Stop | same |
| 9 (0x09) | Assert | Assert | same |
| 10 (0x0a) | Case | Case | same |
| 11 (0x0b) |  | Nothing | missing in exe (folded or unused) |
| 13 (0x0d) | GotoLabel | GotoLabel | same |
| 14 (0x0e) | EatReturnValue | EatReturnValue | same |
| 15 (0x0f) | Let | Let | same |
| 16 (0x10) | DynArrayElement | DynArrayElement | same |
| 17 (0x11) | New | New | same |
| 18 (0x12) | ClassContext | ClassContext | same |
| 19 (0x13) | MetaCast | MetaCast | same |
| 20 (0x14) | LetBool | LetBool | same |
| 22 (0x16) | EndFunctionParms | EndFunctionParms | same |
| 23 (0x17) | Self | Self | same |
| 25 (0x19) | Context | Context | same |
| 26 (0x1a) | ArrayElement | ArrayElement | same |
| 27 (0x1b) | VirtualFunction | VirtualFunction | same |
| 28 (0x1c) | FinalFunction | FinalFunction | same |
| 29 (0x1d) | IntConst | IntConst | same |
| 30 (0x1e) | FloatConst | FloatConst | same |
| 31 (0x1f) | StringConst | StringConst | same |
| 32 (0x20) | ObjectConst | ObjectConst | same |
| 33 (0x21) | NameConst | NameConst | same |
| 34 (0x22) | RotationConst | RotationConst | same |
| 35 (0x23) | VectorConst | VectorConst | same |
| 36 (0x24) | ByteConst | ByteConst | same |
| 37 (0x25) | False | IntZero | **DIFF** |
| 38 (0x26) | True | IntOne | **DIFF** |
| 39 (0x27) | True | True | same |
| 40 (0x28) | False | False | same |
| 41 (0x29) | NativeParm | NativeParm | same |
| 42 (0x2a) | False | NoObject | **DIFF** |
| 44 (0x2c) | IntConstByte | IntConstByte | same |
| 45 (0x2d) | BoolVariable | BoolVariable | same |
| 46 (0x2e) | DynamicCast | DynamicCast | same |
| 47 (0x2f) |  | Iterator | missing in exe (folded or unused) |
| 48 (0x30) | IteratorPop | IteratorPop | same |
| 50 (0x32) | StructCmpEq | StructCmpEq | same |
| 51 (0x33) | StructCmpNe | StructCmpNe | same |
| 52 (0x34) | UnicodeStringConst | UnicodeStringConst | same |
| 53 (0x35) | StructMember | StructMember | same |
| 54 (0x36) | DynArrayLength | DynArrayLength | same |
| 55 (0x37) | GlobalFunction | GlobalFunction | same |
| 56 (0x38) | PrimitiveCast | PrimitiveCast | same |
| 57 (0x39) | DynArrayInsert | DynArrayInsert | same |
| 58 (0x3a) | ReturnNothing | ReturnNothing | same |
| 59 (0x3b) | EqualEqual_DelegateFunction | EqualEqual_DelegateDelegate | **DIFF** |
| 60 (0x3c) | NotEqual_DelegateDelegate | NotEqual_DelegateDelegate | same |
| 61 (0x3d) | EqualEqual_DelegateFunction | EqualEqual_DelegateFunction | same |
| 62 (0x3e) | NotEqual_DelegateDelegate | NotEqual_DelegateFunction | **DIFF** |
| 63 (0x3f) | EmptyDelegate | EmptyDelegate | same |
| 64 (0x40) | DynArrayRemove | DynArrayRemove | same |
| 65 (0x41) | DebugInfo | DebugInfo | same |
| 66 (0x42) | DelegateFunction | DelegateFunction | same |
| 67 (0x43) | DelegateProperty | DelegateProperty | same |
| 68 (0x44) | LetDelegate | LetDelegate | same |
| 69 (0x45) | Conditional | Conditional | same |
| 70 (0x46) | DynArrayFind | DynArrayFind | same |
| 71 (0x47) | DynArrayFindStruct | DynArrayFindStruct | same |
| 72 (0x48) | LocalOutVariable | LocalOutVariable | same |
| 73 (0x49) | DefaultParmValue | DefaultParmValue | same |
| 74 (0x4a) | EmptyParmValue | EmptyParmValue | same |
| 75 (0x4b) | InstanceDelegate | InstanceDelegate | same |
| 81 (0x51) | InterfaceContext | InterfaceContext | same |
| 82 (0x52) | InterfaceCast | InterfaceCast | same |
| 83 (0x53) | EndOfScript | EndOfScript | same |
| 84 (0x54) | DynArrayAdd | DynArrayAdd | same |
| 85 (0x55) | DynArrayAddItem | DynArrayAddItem | same |
| 86 (0x56) | DynArrayRemoveItem | DynArrayRemoveItem | same |
| 87 (0x57) | DynArrayInsertItem | DynArrayInsertItem | same |
| 88 (0x58) | DynArrayIterator | DynArrayIterator | same |
| 89 (0x59) | DynArraySort | DynArraySort | same |
| 90 (0x5a) | JumpIfNotEditorOnly | JumpIfNotEditorOnly | same |
| 96 (0x60) | HighNative0 |  | not an opcode in reference |
| 97 (0x61) | HighNative1 |  | not an opcode in reference |
| 98 (0x62) | HighNative2 |  | not an opcode in reference |
| 99 (0x63) | HighNative3 |  | not an opcode in reference |
| 100 (0x64) | HighNative4 |  | not an opcode in reference |
| 101 (0x65) | HighNative5 |  | not an opcode in reference |
| 102 (0x66) | HighNative6 |  | not an opcode in reference |
| 103 (0x67) | HighNative7 |  | not an opcode in reference |
| 104 (0x68) | HighNative8 |  | not an opcode in reference |
| 105 (0x69) | HighNative9 |  | not an opcode in reference |
| 106 (0x6a) | HighNative10 |  | not an opcode in reference |
| 107 (0x6b) | HighNative11 |  | not an opcode in reference |
| 108 (0x6c) | HighNative12 |  | not an opcode in reference |
| 109 (0x6d) | HighNative13 |  | not an opcode in reference |
| 110 (0x6e) | HighNative14 |  | not an opcode in reference |
| 111 (0x6f) | HighNative15 |  | not an opcode in reference |
| 112 (0x70) | Concat_StrStr | Concat_StrStr | same |
| 113 (0x71) | GotoState | GotoState | same |
| 114 (0x72) | EqualEqual_IntInt | EqualEqual_ObjectObject | **DIFF** |
| 115 (0x73) | Less_StrStr | Less_StrStr | same |
| 116 (0x74) | Greater_StrStr | Greater_StrStr | same |
| 117 (0x75) | Enable | Enable | same |
| 118 (0x76) | Disable | Disable | same |
| 119 (0x77) | NotEqual_ObjectObject | NotEqual_ObjectObject | same |
| 120 (0x78) | LessEqual_StrStr | LessEqual_StrStr | same |
| 121 (0x79) | GreaterEqual_StrStr | GreaterEqual_StrStr | same |
| 122 (0x7a) | EqualEqual_StrStr | EqualEqual_StrStr | same |
| 123 (0x7b) | NotEqual_StrStr | NotEqual_StrStr | same |
| 124 (0x7c) | ComplementEqual_StrStr | ComplementEqual_StrStr | same |
| 125 (0x7d) | Len | Len | same |
| 126 (0x7e) | InStr | InStr | same |
| 127 (0x7f) | Mid | Mid | same |
