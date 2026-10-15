"""Agent HA: list instructions in functions whose name matches a regex and whose disassembly matches another.

  python resources/tools/ida/run.py resources/tools/ida/ha_scan.py <db.i64> <func_regex> <insn_regex>
"""
import re
import sys

import ida_funcs
import ida_lines
import idautils
import idc

from _common import demangle_full


def main() -> None:
    fpat = re.compile(sys.argv[1])
    ipat = re.compile(sys.argv[2])
    for ea in idautils.Functions():
        n = idc.get_func_name(ea)
        name = demangle_full(n) or n
        if not fpat.search(name):
            continue
        f = ida_funcs.get_func(ea)
        for item in idautils.Heads(f.start_ea, f.end_ea):
            dis = ida_lines.tag_remove(idc.generate_disasm_line(item, 0) or "")
            if ipat.search(dis):
                print(f"0x{item:x} in 0x{ea:x} {name}: {dis}")


if __name__ == "__main__":
    main()
