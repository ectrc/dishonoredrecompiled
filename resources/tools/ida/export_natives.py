"""P1.8: export UnrealScript native bindings (Class::execFunc), the GNatives index table and StaticClass functions.

Outputs docs/symbols/natives.csv and docs/symbols/classes.csv.
Usage: python resources/tools/ida/run.py resources/tools/ida/export_natives.py <db.i64> [suffix]
"""
import re
import sys

import ida_bytes
import ida_funcs
import ida_idaapi
import ida_name
import ida_ua
import idautils

from _common import SYMBOLS_DIR, demangle, demangle_full, function_name, iter_functions, log, open_csv, ptr_size, read_ptr, rva

EXEC_RE = re.compile(r"^(?:(?P<ns>[\w:]+)::)?(?P<cls>\w+)::exec(?P<func>\w+)$")
STATIC_CLASS_RE = re.compile(r"^(?P<cls>\w+)::StaticClass$")
GNATIVES_COUNT = 0x1000


def find_named(short_name: str) -> int:
    for ea, name in idautils.Names():
        if demangle(name) == short_name:
            return ea
    return ida_idaapi.BADADDR


def static_gnatives_entries(gnatives: int) -> dict[int, int]:
    out = {}
    if gnatives == ida_idaapi.BADADDR:
        return out
    for i in range(GNATIVES_COUNT):
        target = read_ptr(gnatives + i * ptr_size())
        if ida_funcs.get_func(target) is not None:
            out.setdefault(target, i)
    return out


def register_native_calls(register_fn: int) -> dict[int, int]:
    """Every IMPLEMENT_FUNCTION expands to a dynamic initializer of the form
        push offset _int<Class>exec<Func>   ; static Native member-pointer variable (in .data)
        push <iNative>                       ; -1 for name-bound natives, else the GNatives index
        call GRegisterNative
    so both operands are immediates two instructions back from each call site. The variable holds
    the exec function pointer, which is read to key the result by function address."""
    out = {}
    if register_fn == ida_idaapi.BADADDR:
        return out
    for xref in idautils.XrefsTo(register_fn, 0):
        if not xref.iscode:
            continue
        imms = []
        ea = xref.frm
        for _ in range(4):
            ea = ida_bytes.prev_head(ea, 0)
            insn = ida_ua.insn_t()
            if ida_ua.decode_insn(insn, ea) == 0 or insn.get_canon_mnem() != "push" or insn.ops[0].type != ida_ua.o_imm:
                break
            imms.append(insn.ops[0].value)
            if len(imms) == 2:
                break
        if len(imms) != 2:
            continue
        index, var = imms
        index &= 0xFFFFFFFF
        var &= 0xFFFFFFFF
        index = index - 0x100000000 if index >= 0x80000000 else index
        target = read_ptr(var)
        if ida_funcs.get_func(target) is not None:
            out.setdefault(target, index)
    return out


def inlined_gnatives_stores(gnatives: int) -> dict[int, int]:
    """Inside Core, GRegisterNative is inlined into the dynamic initializers:
        mov eax, _int<Class>exec<Func>      ; static Native variable holding the function pointer
        mov GNatives[index*4], eax
    Scan every 'dynamic initializer for ...exec...' symbol for a memory operand inside the GNatives
    array (gives the index) and an operand naming an _int* variable (gives the function)."""
    out = {}
    if gnatives == ida_idaapi.BADADDR:
        return out
    lo, hi = gnatives, gnatives + GNATIVES_COUNT * ptr_size()
    for ea, name in idautils.Names():
        dem = demangle_full(name)
        if ("dynamic initializer for" not in dem and "_dynamic_initializer_for_" not in dem) or "exec" not in dem:
            continue
        index = None
        target = None
        cur = ea
        for _ in range(24):
            insn = ida_ua.insn_t()
            if ida_ua.decode_insn(insn, cur) == 0:
                break
            mnem = insn.get_canon_mnem()
            for n, op in enumerate(insn.ops):
                if op.type == ida_ua.o_void:
                    break
                if op.type == ida_ua.o_mem:
                    # the array base also appears as the rep-stosd target of the one-time clear;
                    # only a store to it (mov [GNatives], reg) means index 0 (EX_LocalVariable)
                    if lo <= op.addr < hi and (op.addr != lo or (mnem == "mov" and n == 0)):
                        index = (op.addr - lo) // ptr_size()
                    elif ida_name.get_name(op.addr).startswith("_int"):
                        candidate = read_ptr(op.addr)
                        if ida_funcs.get_func(candidate) is not None:
                            target = candidate
            if insn.get_canon_mnem() in ("retn", "ret"):
                break
            cur += insn.size
        if index is not None and target is not None:
            out.setdefault(target, index)
    return out


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    gnatives = find_named("GNatives")
    index_by_target = static_gnatives_entries(gnatives)
    register_fn = find_named("GRegisterNative")
    registered = register_native_calls(register_fn)
    inlined = inlined_gnatives_stores(gnatives)
    for func, index in list(inlined.items()) + list(registered.items()):
        index_by_target.setdefault(func, index)
    log(f"inlined GNatives stores: {len(inlined)}")
    numbered = sum(1 for v in registered.values() if v >= 0)
    log(f"GNatives at 0x{gnatives:08x}: {len(index_by_target) - len(registered)} static entries; GRegisterNative at 0x{register_fn:08x}: {len(registered)} call sites, {numbered} numbered")

    fn, wn = open_csv(SYMBOLS_DIR / f"natives{suffix}.csv", ["class", "func", "va", "rva", "size", "native_index", "mangled"])
    fc, wc = open_csv(SYMBOLS_DIR / f"classes{suffix}.csv", ["class", "staticclass_va", "staticclass_rva"])
    natives = 0
    classes = 0
    for func in iter_functions():
        mangled = function_name(func)
        short = demangle(mangled)
        m = EXEC_RE.match(short)
        if m:
            wn.writerow([m.group("cls"), m.group("func"), f"0x{func.start_ea:08x}", f"0x{rva(func.start_ea):x}", func.size(), index_by_target.get(func.start_ea, ""), mangled])
            natives += 1
            continue
        m = STATIC_CLASS_RE.match(short)
        if m:
            wc.writerow([m.group("cls"), f"0x{func.start_ea:08x}", f"0x{rva(func.start_ea):x}"])
            classes += 1
    fn.close()
    fc.close()
    log(f"natives={natives} classes={classes} indexed={len(index_by_target)}")


if __name__ == "__main__":
    main()
