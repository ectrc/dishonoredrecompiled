"""P1.1: report whether a database has PDB names and the core UE3 types.

Usage: python tools/ida/run.py tools/ida/verify_db.py <db.i64> [output.txt]
"""
import sys

import ida_funcs
import ida_name
import ida_typeinf
import idautils

from _common import image_base, iter_functions, function_name

CORE_TYPES = ["UObject", "UClass", "UProperty", "UStruct", "UFunction", "FName", "FString", "FArchive", "TArray<int>", "FEngineLoop"]


def main() -> int:
    total = 0
    unnamed = 0
    for func in iter_functions():
        total += 1
        if function_name(func).startswith("sub_"):
            unnamed += 1

    til = ida_typeinf.get_idati()
    ntypes = ida_typeinf.get_ordinal_limit(til) - 1
    found = {}
    for name in CORE_TYPES:
        t = ida_typeinf.tinfo_t()
        found[name] = t.get_named_type(til, name)
    tarray_like = sum(1 for i in range(1, ida_typeinf.get_ordinal_limit(til)) if (ida_typeinf.get_numbered_type_name(til, i) or "").startswith("TArray<"))

    lines = [
        f"image_base=0x{image_base():08x}",
        f"functions={total}",
        f"functions_unnamed={unnamed} ({100.0 * unnamed / max(total, 1):.2f}%)",
        f"local_types={ntypes}",
        f"tarray_instantiations={tarray_like}",
        f"uobject_vtable={'yes' if ida_name.get_name_ea(0, '??_7UObject@@6B@') != ida_funcs.BADADDR else 'no'}",
    ]
    lines += [f"type:{name}={'yes' if ok else 'no'}" for name, ok in found.items()]
    text = "\n".join(lines) + "\n"
    print(text)
    if len(sys.argv) > 1:
        with open(sys.argv[1], "w", encoding="utf-8") as f:
            f.write(text)
    return 0


if __name__ == "__main__":
    main()
