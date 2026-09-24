"""P1.2: export every function and segment of the open database.

Outputs docs/symbols/functions.csv and docs/symbols/segments.csv.
Usage: python tools/ida/run.py tools/ida/export_functions.py <db.i64> [suffix]
The optional suffix (e.g. _2013) is appended to the output file names.
"""
import sys

import ida_funcs
import ida_segment
import idautils

from _common import SYMBOLS_DIR, demangle_full, function_name, iter_functions, log, open_csv, rva


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    f, w = open_csv(SYMBOLS_DIR / f"functions{suffix}.csv", ["va", "rva", "size", "mangled", "demangled", "is_thunk", "is_lib", "has_pseudocode"])
    count = 0
    for func in iter_functions():
        name = function_name(func)
        w.writerow([
            f"0x{func.start_ea:08x}",
            f"0x{rva(func.start_ea):x}",
            func.size(),
            name,
            demangle_full(name),
            int(bool(func.flags & ida_funcs.FUNC_THUNK)),
            int(bool(func.flags & ida_funcs.FUNC_LIB)),
            "",
        ])
        count += 1
    f.close()

    f, w = open_csv(SYMBOLS_DIR / f"segments{suffix}.csv", ["name", "start", "end", "size", "perm", "class"])
    for ea in idautils.Segments():
        seg = ida_segment.getseg(ea)
        w.writerow([ida_segment.get_segm_name(seg), f"0x{seg.start_ea:08x}", f"0x{seg.end_ea:08x}", seg.size(), seg.perm, ida_segment.get_segm_class(seg)])
    f.close()
    log(f"functions={count} (ida reports {ida_funcs.get_func_qty()})")


if __name__ == "__main__":
    main()
