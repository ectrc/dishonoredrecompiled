"""P1.8: export UnrealScript native bindings (Class::execFunc), the GNatives index table and StaticClass functions.

Outputs docs/symbols/natives.csv and docs/symbols/classes.csv.
Usage: python tools/ida/run.py tools/ida/export_natives.py <db.i64> [suffix]
"""
import re
import sys

import ida_funcs
import ida_hexrays
import ida_idaapi
import ida_name
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


CALL_RE = re.compile(r"GRegisterNative\((0x[0-9A-Fa-f]+|\d+)")
EXEC_REF_RE = re.compile(r"\b((?:\w+::)+exec\w+)\b")


def register_native_calls(register_fn: int) -> dict[int, int]:
    """GRegisterNative(INT Index, const Native& Func) is called from one dynamic initializer per
    IMPLEMENT_FUNCTION. The member-function pointer is passed by reference, so the index and the
    exec function are recovered from the decompiled initializer rather than from push immediates."""
    out = {}
    if register_fn == ida_idaapi.BADADDR or not ida_hexrays.init_hexrays_plugin():
        return out
    callers = set()
    for xref in idautils.XrefsTo(register_fn, 0):
        if xref.iscode:
            func = ida_funcs.get_func(xref.frm)
            if func is not None:
                callers.add(func.start_ea)
    for ea in sorted(callers):
        try:
            text = str(ida_hexrays.decompile(ea))
        except ida_hexrays.DecompilationFailure:
            continue
        indices = [int(v, 0) for v in CALL_RE.findall(text)]
        refs = exec_functions_referenced(ea)
        if len(indices) == 1 and len(refs) == 1:
            out.setdefault(refs[0], indices[0])
        elif len(indices) == len(refs) > 1:
            for index, ref in zip(indices, refs):
                out.setdefault(ref, index)
    return out


def exec_functions_referenced(caller_ea: int) -> list[int]:
    """exec* functions referenced (code or data xref) from the body of a dynamic initializer, in address order."""
    found = []
    for item in idautils.FuncItems(caller_ea):
        for ref in idautils.XrefsFrom(item, 0):
            target = ida_funcs.get_func(ref.to)
            if target is not None and target.start_ea == ref.to and "::exec" in demangle(function_name(target)) and target.start_ea not in found:
                found.append(target.start_ea)
    return found


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    gnatives = find_named("GNatives")
    index_by_target = static_gnatives_entries(gnatives)
    register_fn = find_named("GRegisterNative")
    registered = register_native_calls(register_fn)
    for func, index in registered.items():
        index_by_target.setdefault(func, index)
    log(f"GNatives at 0x{gnatives:08x}: {len(index_by_target) - len(registered)} static entries; GRegisterNative at 0x{register_fn:08x}: {len(registered)} call sites")

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
