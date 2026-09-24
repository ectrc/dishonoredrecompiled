"""P1.5: export named data items and the import table.

Outputs docs/symbols/globals.csv and docs/symbols/imports.csv.
Usage: python tools/ida/run.py tools/ida/export_globals.py <db.i64> [suffix]
"""
import sys

import ida_bytes
import ida_nalt
import ida_segment
import ida_typeinf
import idautils

from _common import SYMBOLS_DIR, demangle_full, log, open_csv, rva, segment_name


def type_string(ea: int) -> str:
    t = ida_typeinf.tinfo_t()
    return str(t) if ida_nalt.get_tinfo(t, ea) else ""


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    f, w = open_csv(SYMBOLS_DIR / f"globals{suffix}.csv", ["va", "rva", "name", "demangled", "type", "size", "segment"])
    count = 0
    for ea, name in idautils.Names():
        seg = ida_segment.getseg(ea)
        if seg is None or seg.perm & ida_segment.SEGPERM_EXEC:
            continue
        w.writerow([f"0x{ea:08x}", f"0x{rva(ea):x}", name, demangle_full(name), type_string(ea), ida_bytes.get_item_size(ea), segment_name(ea)])
        count += 1
    f.close()

    f, w = open_csv(SYMBOLS_DIR / f"imports{suffix}.csv", ["dll", "function", "ordinal", "iat_va"])
    rows = []
    for i in range(ida_nalt.get_import_module_qty()):
        dll = ida_nalt.get_import_module_name(i)

        def visit(ea, name, ordinal, dll=dll):
            rows.append([dll, name or "", ordinal, f"0x{ea:08x}"])
            return True

        ida_nalt.enum_import_names(i, visit)
    for row in sorted(rows):
        w.writerow(row)
    f.close()
    log(f"globals={count} imports={len(rows)}")


if __name__ == "__main__":
    main()
