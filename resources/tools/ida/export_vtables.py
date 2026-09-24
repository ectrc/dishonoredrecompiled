"""P1.7: export every MSVC vftable (??_7<Class>@@6B...@) with its resolved slots.

Outputs docs/symbols/vtables.csv.
Usage: python resources/tools/ida/run.py resources/tools/ida/export_vtables.py <db.i64> [suffix]
"""
import re
import sys

import ida_funcs
import ida_name
import idautils

from _common import SYMBOLS_DIR, demangle_full, log, open_csv, ptr_size, read_ptr, rva

VFTABLE_RE = re.compile(r"^const (.+?)::`vftable'(?:\{for `(.+)'\})?$")


def class_label(mangled: str) -> str:
    dem = demangle_full(mangled)
    m = VFTABLE_RE.match(dem)
    if not m:
        return dem
    return f"{m.group(1)}{{for {m.group(2)}}}" if m.group(2) else m.group(1)


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    f, w = open_csv(SYMBOLS_DIR / f"vtables{suffix}.csv", ["class", "vtable_va", "vtable_rva", "slot", "target_va", "target_mangled", "target_demangled", "is_purecall"])
    tables = 0
    slots = 0
    for ea, name in idautils.Names():
        if not name.startswith("??_7"):
            continue
        label = class_label(name)
        tables += 1
        p = ea
        slot = 0
        while True:
            if slot > 0 and ida_name.get_name(p):
                break
            target = read_ptr(p)
            func = ida_funcs.get_func(target)
            if func is None or func.start_ea != target:
                break
            tname = ida_name.get_name(target)
            w.writerow([label, f"0x{ea:08x}", f"0x{rva(ea):x}", slot, f"0x{target:08x}", tname, demangle_full(tname), int("purecall" in tname)])
            slot += 1
            slots += 1
            p += ptr_size()
    f.close()
    log(f"vtables={tables} slots={slots}")


if __name__ == "__main__":
    main()
