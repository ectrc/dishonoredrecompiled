"""Find instructions that reference [reg+<displacement>] in functions matching a name regex (headless).

  python resources/tools/ida/run.py build/agentAT_disp.py <db.i64> <displacement hex> [name regex] [mnemonic regex]
"""
import re
import sys

import ida_ua
import idautils

from _common import demangle_full, function_name, iter_functions, rva

disp = int(sys.argv[1], 16)
pat = re.compile(sys.argv[2]) if len(sys.argv) > 2 and sys.argv[2] else None
mpat = re.compile(sys.argv[3]) if len(sys.argv) > 3 else None

for func in iter_functions():
    name = demangle_full(function_name(func)) or function_name(func)
    if pat and not pat.search(name):
        continue
    for ea in idautils.Heads(func.start_ea, func.end_ea):
        insn = ida_ua.insn_t()
        if ida_ua.decode_insn(insn, ea) <= 0:
            continue
        mnem = insn.get_canon_mnem()
        if mpat and not mpat.search(mnem):
            continue
        for op in insn.ops:
            if op.type == ida_ua.o_displ and op.addr == disp:
                import ida_lines
                line = ida_lines.tag_remove(ida_lines.generate_disasm_line(ea, 0) or "")
                print(f"0x{rva(ea):x} {line}   <- {name}")
                break
