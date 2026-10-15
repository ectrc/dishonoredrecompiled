"""Agent HA: headless IDA queries on a private copy.

  python resources/tools/ida/run.py build/agentHA/ha_ida.py <db.i64> <out_dir> <cmd> ...

  dec:<ea>          decompile the function containing ea to <out_dir>/<ea>_<name>.c
  dis:<ea>          disassemble the function containing ea to <out_dir>/<ea>_<name>.asm
  vt:<symbol>:<n>   print the first n slots of the vtable named <symbol> (mangled or demangled substring)
  find:<regex>      list functions whose (demangled) name matches
  hash:<ea>         print the size and sha1 of the function bytes (relocations masked by 0 for E8/E9 rel32)
  xref:<ea>         list code xrefs to ea with their function
"""
import hashlib
import re
import sys
from pathlib import Path

import ida_bytes
import ida_funcs
import ida_hexrays
import ida_lines
import ida_nalt
import ida_name
import idautils
import idc

from _common import demangle_full


def safe(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]+", "_", name)[:80]


def fname(ea: int) -> str:
    n = idc.get_func_name(ea) or idc.get_name(ea) or ""
    return demangle_full(n) or n


def main() -> None:
    out_dir = Path(sys.argv[1])
    out_dir.mkdir(parents=True, exist_ok=True)
    base = ida_nalt.get_imagebase()
    hexrays = ida_hexrays.init_hexrays_plugin()
    names = None
    for a in sys.argv[2:]:
        kind, _, arg = a.partition(":")
        if kind in ("dec", "dis", "hash"):
            ea = int(arg, 16)
            f = ida_funcs.get_func(ea)
            if f is None:
                print(f"{a}: NOT IN A FUNCTION (imagebase 0x{base:x})")
                continue
            name = fname(f.start_ea)
            size = f.end_ea - f.start_ea
            head = f"// {name}\n// start 0x{f.start_ea:x} (rva 0x{f.start_ea - base:x}) size {size} imagebase 0x{base:x}\n"
            if kind == "dec":
                if not hexrays:
                    print("no hexrays")
                    continue
                try:
                    text = str(ida_hexrays.decompile(f.start_ea))
                except Exception as e:  # noqa: BLE001
                    text = f"// decompile failed: {e}"
                path = out_dir / f"{f.start_ea:x}_{safe(name)}.c"
                path.write_text(head + "\n" + text, encoding="utf-8")
                print(f"{a}: 0x{f.start_ea:x} {size} B {name} -> {path}")
            elif kind == "dis":
                lines = [head]
                for item in idautils.Heads(f.start_ea, f.end_ea):
                    dis = ida_lines.tag_remove(idc.generate_disasm_line(item, 0) or "")
                    raw = ida_bytes.get_bytes(item, idc.get_item_size(item)) or b""
                    extra = ""
                    for x in idautils.XrefsFrom(item, 0):
                        if x.type in (16, 17, 18, 19, 20, 21):
                            extra = f"   ; -> 0x{x.to:x} {fname(x.to)}"
                    lines.append(f"{item:08x}  {raw.hex():<24} {dis}{extra}")
                path = out_dir / f"{f.start_ea:x}_{safe(name)}.asm"
                path.write_text("\n".join(lines), encoding="utf-8")
                print(f"{a}: 0x{f.start_ea:x} {size} B {name} -> {path}")
            else:
                raw = bytearray(ida_bytes.get_bytes(f.start_ea, size) or b"")
                for item in idautils.Heads(f.start_ea, f.end_ea):
                    off = item - f.start_ea
                    op = raw[off] if off < len(raw) else 0
                    if op in (0xE8, 0xE9) and idc.get_item_size(item) == 5:
                        raw[off + 1:off + 5] = b"\0\0\0\0"
                print(f"{a}: 0x{f.start_ea:x} {size} B {name} sha1 {hashlib.sha1(bytes(raw)).hexdigest()}")
        elif kind == "vt":
            sym, _, n = arg.rpartition(":")
            count = int(n)
            ea = int(sym, 16) if sym.startswith("0x") else ida_name.get_name_ea(idc.BADADDR, sym)
            if ea == idc.BADADDR:
                for nea, nm in idautils.Names():
                    if sym in nm or sym in (demangle_full(nm) or ""):
                        ea = nea
                        print(f"vt match {nm} at 0x{nea:x}")
                        break
            if ea == idc.BADADDR:
                print(f"{a}: vtable not found")
                continue
            for i in range(count):
                tgt = ida_bytes.get_dword(ea + 4 * i)
                f = ida_funcs.get_func(tgt)
                size = (f.end_ea - f.start_ea) if f else -1
                print(f"slot {i:3d} +{4 * i:4d}: 0x{tgt:x} {size:5d} B {fname(tgt)}")
        elif kind == "find":
            pat = re.compile(arg)
            if names is None:
                names = [(ea, fname(ea)) for ea in idautils.Functions()]
            for ea, name in names:
                if pat.search(name):
                    f = ida_funcs.get_func(ea)
                    print(f"0x{ea:x} (rva 0x{ea - base:x}) {f.end_ea - f.start_ea} B {name}")
        elif kind == "xref":
            ea = int(arg, 16)
            for x in idautils.XrefsTo(ea):
                f = ida_funcs.get_func(x.frm)
                print(f"0x{x.frm:x} in {fname(f.start_ea) if f else '?'} (0x{f.start_ea if f else 0:x})")
        else:
            print(f"unknown {a}")


if __name__ == "__main__":
    main()
