"""P1.9: recover the hardcoded FName table (UnNames.h) from FName::StaticInit.

FName::StaticInit expands REGISTER_NAME(num, name) into
    AutoName_<name> = FName::AllocateNameEntry(L"<name>" / "<name>", num, NULL, bPureAnsi); FName::Hardcode(AutoName_<name>);
so the (index, string) pairs are read from the decompiled call arguments.

Outputs docs/symbols/hardcoded_names.csv (index,name).
Usage: python tools/ida/run.py tools/ida/export_names.py <db.i64> [suffix]
"""
import re
import sys

import ida_hexrays
import ida_name

from _common import SYMBOLS_DIR, demangle, iter_functions, function_name, log, open_csv

ALLOC_RE = re.compile(r"AllocateNameEntry\(\s*(?:\(const void \*\))?\s*L?\"((?:[^\"\\]|\\.)*)\"\s*,\s*(0x[0-9A-Fa-f]+|\d+)")
FALLBACK_RE = re.compile(r"L?\"((?:[^\"\\]|\\.)*)\"\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*,\s*0")


def find_function(short_name: str) -> int:
    for func in iter_functions():
        if demangle(function_name(func)) == short_name:
            return func.start_ea
    return ida_name.BADADDR if hasattr(ida_name, "BADADDR") else 0xFFFFFFFF


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    if not ida_hexrays.init_hexrays_plugin():
        log("Hex-Rays not available")
        return
    ea = find_function("FName::StaticInit")
    text = str(ida_hexrays.decompile(ea))
    pairs = ALLOC_RE.findall(text)
    source = "AllocateNameEntry"
    if len(pairs) < 100:
        pairs = FALLBACK_RE.findall(text)
        source = "fallback"
    names = {}
    for name, index in pairs:
        names.setdefault(int(index, 0), name.encode("latin-1", "backslashreplace").decode("unicode_escape"))

    f, w = open_csv(SYMBOLS_DIR / f"hardcoded_names{suffix}.csv", ["index", "name"])
    for index in sorted(names):
        w.writerow([index, names[index]])
    f.close()
    indices = sorted(names)
    gaps = [i for i in range(indices[0], indices[-1] + 1) if i not in names] if indices else []
    log(f"source={source} names={len(names)} first={indices[:1]} last={indices[-1:]} gaps={len(gaps)} pseudocode_lines={text.count(chr(10))}")


if __name__ == "__main__":
    main()
