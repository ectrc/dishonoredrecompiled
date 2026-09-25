"""Phase 2b (package H): sizeof() of every native UE3 class, read from the UClass(EStaticConstructor, ...) registrations.

Every native class is registered through
    UClass::UClass(EStaticConstructor, DWORD InSize, DWORD InClassFlags, DWORD InOtherClassFlags, DWORD InClassCastFlags,
                   const TCHAR* InNameStr, const TCHAR* InPackageName, const TCHAR* InConfigName, QWORD InFlags,
                   void (*InClassConstructor)(void*), void (UObject::*InClassStaticConstructor)(), void (UObject::*InClassStaticInitializer)())
The two Dishonored builds feed that constructor differently:
  * 2012 Shipping ("direct"): every GetPrivateStaticClass<X>(const TCHAR* Package) pushes the 13 arguments itself
    (sizeof(X) and the name string are immediates, Package is its own parameter which X::StaticClassNoInline pushes
    as a wide-string literal).
  * 2013 retail ("descriptor"): every X::StaticClass pushes a pointer to a 12-dword descriptor in .data (the start
    of the in-place UClass storage) and calls one common GetPrivateStaticClass(desc) that copies the 12 dwords to
    the stack as the constructor arguments. The descriptor holds the same fields in argument order.
Both builds use the same InitializePrivateStaticClass<X> shape
    call Within::StaticClass ; push eax ; mov eax, X::PrivateStaticClass ; push eax ; call Super::StaticClass ; push eax ; call Common
so Super and Within are recovered by mapping StaticClass function addresses back to class names.

Usage:
  python resources/tools/ida/run.py resources/tools/ida/export_class_sizes.py <db.i64> <out.csv> [--debug]
  python resources/tools/ida/export_class_sizes.py merge <2012.csv> <2013.csv> <out.csv> [--sizes resources/docs/types/sizes.csv]
The merge sub-command does not need IDA.
"""
import csv
import sys
from collections import Counter, defaultdict
from pathlib import Path

ARG_FIELDS = ["size", "class_flags", "other_flags", "cast_flags", "name_ptr", "package_ptr", "config_ptr", "flags_lo", "flags_hi", "ctor", "static_ctor", "static_init"]
ARG_COUNT = len(ARG_FIELDS)
MIN_REGISTRATIONS = 1000
CTOR_NAME_PREFIX = "??0UClass@@QAE@W4EStaticConstructor"
EXPORT_HEADER = ["class", "name", "package", "size", "class_flags", "other_flags", "cast_flags", "config", "flags", "super", "within",
                 "registrant_rva", "staticclass_rva", "init_rva", "container_rva", "ctor_rva", "static_ctor_rva", "static_init_rva", "mode"]
CORE_CONTRACT = ["UObject", "UField", "UStruct", "UState", "UClass", "UFunction", "UProperty", "UByteProperty", "UIntProperty", "UBoolProperty",
                 "UFloatProperty", "UObjectProperty", "UComponentProperty", "UClassProperty", "UInterfaceProperty", "UNameProperty",
                 "UStrProperty", "UArrayProperty", "UMapProperty", "UStructProperty", "UDelegateProperty", "UEnum", "UScriptStruct",
                 "UPackage", "ULinkerLoad", "ULinker", "ULinkerSave", "UConst", "UTextBuffer", "USystem", "UMetaData", "UObjectRedirector"]


def read_wide_string(ea: int, limit: int = 512) -> str | None:
    import ida_bytes
    raw = ida_bytes.get_bytes(ea, limit * 2)
    if not raw:
        return None
    chars = []
    for i in range(0, len(raw) - 1, 2):
        code = raw[i] | (raw[i + 1] << 8)
        if code == 0:
            return "".join(chars)
        if code < 0x20 or code > 0x7E:
            return None
        chars.append(chr(code))
    return None


def is_code_address(ea: int) -> bool:
    import ida_segment
    seg = ida_segment.getseg(ea)
    return seg is not None and seg.type == ida_segment.SEG_CODE


def decode(ea: int):
    import ida_ua
    insn = ida_ua.insn_t()
    return insn if ida_ua.decode_insn(insn, ea) else None


def call_target(insn) -> int | None:
    import ida_ua
    if insn.get_canon_mnem() != "call" or insn.ops[0].type not in (ida_ua.o_near, ida_ua.o_far):
        return None
    return insn.ops[0].addr


def code_callers(ea: int) -> list[int]:
    import idautils
    return [x.frm for x in idautils.XrefsTo(ea, 0) if x.iscode and x.type in (16, 17)]


def function_instructions(func):
    import ida_bytes
    ea = func.start_ea
    while ea < func.end_ea:
        insn = decode(ea)
        if insn is None:
            break
        yield insn
        ea = ida_bytes.next_head(ea, func.end_ea)


class Registration:
    def __init__(self, mode: str, registrant: int, call_site: int):
        self.mode = mode
        self.registrant = registrant
        self.call_site = call_site
        self.args: dict[str, int | None] = {}
        self.package_param_from_caller = False
        self.staticclass = None
        self.init = None
        self.private_static_class_global = None
        self.container = None
        self.super_staticclass = None
        self.within_staticclass = None
        self.symbol_class = None
        self.derived_prefix = None

    def value(self, field: str):
        return self.args.get(field)

    def name(self) -> str | None:
        ptr = self.value("name_ptr")
        return read_wide_string(ptr) if ptr else None

    def literal_prefix(self) -> str | None:
        """The registered name is TEXT(#TClass) + 1: the U/A prefix sits one character before InNameStr, unless the
        compiler folded the literal into a pooled L"Name" (44 Engine classes in the 2013 exe, AActor among them)."""
        import ida_bytes
        ptr = self.value("name_ptr")
        if not ptr:
            return None
        prefix = ida_bytes.get_word(ptr - 2)
        before = ida_bytes.get_word(ptr - 4)
        if before == 0 and prefix in (ord("U"), ord("A")):
            return chr(prefix)
        return None

    def cpp_name(self) -> str | None:
        name = self.name()
        if name is None:
            return None
        prefix = self.literal_prefix()
        if prefix:
            return prefix + name
        if self.symbol_class:
            return self.symbol_class
        return self.derived_prefix + name if self.derived_prefix else None


def derive_prefix(reg: Registration, by_staticclass: dict) -> str | None:
    """Actors are the AActor subtree; everything else is U. Walk the super chain until a class with a literal prefix."""
    seen = set()
    current = reg
    while current is not None and current.registrant not in seen:
        seen.add(current.registrant)
        if current.name() == "Actor":
            return "A"
        literal = current.literal_prefix()
        if literal and current is not reg:
            return literal
        if current.symbol_class:
            return current.symbol_class[0]
        current = by_staticclass.get(current.super_staticclass)
    return None


def direct_arguments(call_site: int, func) -> tuple[list, bool] | None:
    """Walk back over the pushes feeding the constructor call: 13 pushes, optionally interleaved with mov/lea.
    Walking backwards from the call meets the last push first, so the list comes out in argument order."""
    import ida_bytes
    import ida_ua
    pushes = []
    ea = call_site
    while len(pushes) < ARG_COUNT + 1 and ea > func.start_ea:
        ea = ida_bytes.prev_head(ea, func.start_ea)
        insn = decode(ea)
        if insn is None:
            return None
        mnem = insn.get_canon_mnem()
        if mnem == "push":
            op = insn.ops[0]
            if op.type == ida_ua.o_imm:
                pushes.append(op.value & 0xFFFFFFFF)
            elif op.type == ida_ua.o_reg:
                pushes.append(("reg", op.reg, ea))
            else:
                pushes.append(("unknown", ea))
        elif mnem not in ("mov", "lea"):
            return None
    if len(pushes) != ARG_COUNT + 1:
        return None
    resolved = []
    from_param = False
    for item in pushes:
        if isinstance(item, tuple) and item[0] == "reg":
            value = resolve_register(item[2], item[1], func)
            if value == "param":
                from_param = True
                resolved.append(None)
            else:
                resolved.append(value)
        elif isinstance(item, tuple):
            resolved.append(None)
        else:
            resolved.append(item)
    return resolved[1:], from_param


def resolve_register(ea: int, reg: int, func):
    import ida_bytes
    import ida_ua
    while ea > func.start_ea:
        ea = ida_bytes.prev_head(ea, func.start_ea)
        insn = decode(ea)
        if insn is None:
            return None
        if insn.get_canon_mnem() in ("mov", "lea") and insn.ops[0].type == ida_ua.o_reg and insn.ops[0].reg == reg:
            src = insn.ops[1]
            if src.type == ida_ua.o_imm:
                return src.value & 0xFFFFFFFF
            if src.type == ida_ua.o_mem and insn.get_canon_mnem() == "lea":
                return src.addr
            if src.type == ida_ua.o_displ:
                return "param"
            return None
    return None


def descriptor_arguments(desc: int) -> list[int]:
    import ida_bytes
    return [ida_bytes.get_dword(desc + 4 * i) for i in range(ARG_COUNT)]


def arguments_plausible(args: list) -> bool:
    size = args[0]
    name_ptr, package_ptr = args[4], args[5]
    if size is None or not (4 <= size <= 0x100000) or not name_ptr:
        return False
    name = read_wide_string(name_ptr)
    if not name or (package_ptr and read_wide_string(package_ptr) is None):
        return False
    return args[9] is not None and is_code_address(args[9])


def classify_call_site(call_site: int) -> Registration | None:
    """Direct: the site is preceded by the 13 constructor pushes. Descriptor: the site is preceded by push offset <desc>.
    Function chunks are used instead of functions because the bare 2013 analysis absorbs some X::StaticClass bodies as
    tail chunks of a neighbouring function that ends in jmp X::StaticClass."""
    import ida_bytes
    import ida_funcs
    import ida_ua
    func = ida_funcs.get_fchunk(call_site)
    if func is None:
        return None
    prev = decode(ida_bytes.prev_head(call_site, func.start_ea))
    if prev is not None and prev.get_canon_mnem() == "push" and prev.ops[0].type == ida_ua.o_imm:
        desc = prev.ops[0].value & 0xFFFFFFFF
        if not is_code_address(desc) and ida_bytes.is_loaded(desc):
            args = descriptor_arguments(desc)
            if arguments_plausible(args):
                reg = Registration("descriptor", func.start_ea, call_site)
                reg.args = dict(zip(ARG_FIELDS, args))
                reg.container = desc
                reg.staticclass = func.start_ea
                return reg
    direct = direct_arguments(call_site, func)
    if direct is not None:
        args, from_param = direct
        if arguments_plausible(args):
            reg = Registration("direct", func.start_ea, call_site)
            reg.args = dict(zip(ARG_FIELDS, args))
            reg.package_param_from_caller = from_param
            reg.container = resolve_register(call_site, 1, func)
            return reg
    return None


def find_registration_target() -> tuple[int, str]:
    """The constructor (2012, named) or the common descriptor wrapper (2013): the function whose callers all look
    like registrations. Falls back to sampling every function with >= MIN_REGISTRATIONS call sites."""
    import ida_idaapi
    import idautils
    for ea, name in idautils.Names():
        if name.startswith(CTOR_NAME_PREFIX):
            return ea, "named"
    best = None
    for ea in idautils.Functions():
        callers = code_callers(ea)
        if len(callers) < MIN_REGISTRATIONS:
            continue
        sample = callers[:: max(1, len(callers) // 64)][:64]
        modes = Counter(r.mode for r in map(classify_call_site, sample) if r is not None)
        if not modes:
            continue
        mode, hits = modes.most_common(1)[0]
        if hits >= len(sample) * 0.9 and (best is None or len(callers) > best[2]):
            best = (ea, mode, len(callers))
    if best is None:
        raise SystemExit("no function with >= %d registration-shaped call sites" % MIN_REGISTRATIONS)
    return best[0], "pattern:%s" % best[1]


def symbol_class_name(func_ea: int) -> str | None:
    import ida_funcs
    import ida_name
    name = ida_funcs.get_func_name(func_ea) or ""
    demangled = ida_name.demangle_name(name, ida_name.MNG_NODEFINIT | ida_name.MNG_NOPTRTYP | ida_name.MNG_NOECSU | ida_name.MNG_NOTYPE) or name
    marker = "GetPrivateStaticClass"
    if marker in demangled:
        tail = demangled.split(marker)[-1]
        return tail.split("(")[0].strip()
    return None


def complete_direct(reg: Registration) -> None:
    """The 2012 GetPrivateStaticClass<X>(Package) has one caller, X::StaticClassNoInline, which pushes the package literal
    and then calls InitializePrivateStaticClass<X>."""
    import ida_bytes
    import ida_funcs
    import ida_ua
    reg.symbol_class = symbol_class_name(reg.registrant)
    callers = code_callers(reg.registrant)
    if len(callers) != 1:
        return
    site = callers[0]
    func = ida_funcs.get_fchunk(site)
    if func is None:
        return
    reg.staticclass = func.start_ea
    prev = decode(ida_bytes.prev_head(site, func.start_ea))
    if reg.package_param_from_caller and prev is not None and prev.get_canon_mnem() == "push" and prev.ops[0].type == ida_ua.o_imm:
        reg.args["package_ptr"] = prev.ops[0].value & 0xFFFFFFFF
    complete_staticclass(reg, func, site)


def complete_staticclass(reg: Registration, func, site: int) -> None:
    """After the registration call: mov X::PrivateStaticClass, eax ; call InitializePrivateStaticClass<X>."""
    import ida_ua
    seen_site = False
    for insn in function_instructions(func):
        if insn.ea == site:
            seen_site = True
            continue
        if not seen_site:
            continue
        if insn.get_canon_mnem() == "mov" and insn.ops[0].type == ida_ua.o_mem and reg.private_static_class_global is None:
            reg.private_static_class_global = insn.ops[0].addr
        target = call_target(insn)
        if target is not None:
            reg.init = target
            return


def complete_init(reg: Registration, staticclass_functions: set[int]) -> None:
    """InitializePrivateStaticClass<X>: call Within::StaticClass ; push ; mov eax, PSC ; push ; call Super::StaticClass ; push ; call Common."""
    import ida_funcs
    if reg.init is None:
        return
    func = ida_funcs.get_fchunk(reg.init)
    if func is None:
        return
    calls = [t for t in (call_target(i) for i in function_instructions(func)) if t is not None]
    static_calls = [t for t in calls if t in staticclass_functions]
    if len(static_calls) == 2:
        reg.within_staticclass, reg.super_staticclass = static_calls
    elif len(static_calls) == 1 and len(calls) == 2:
        reg.within_staticclass = reg.super_staticclass = static_calls[0]


def export(database_out: Path, debug: bool) -> None:
    import ida_funcs
    from _common import image_base, log, open_csv, rva

    target, how = find_registration_target()
    sites = code_callers(target)
    log("registration target %#x (%s), %d call sites, image base %#x" % (target, how, len(sites), image_base()))
    regs = []
    rejected = []
    for site in sites:
        reg = classify_call_site(site)
        if reg is None:
            rejected.append(site)
            continue
        if reg.mode == "direct":
            complete_direct(reg)
        else:
            complete_staticclass(reg, ida_funcs.get_fchunk(site), site)
        regs.append(reg)
    log("decoded %d registrations (%s), %d rejected call sites" % (len(regs), Counter(r.mode for r in regs), len(rejected)))
    for site in rejected[:20]:
        log("  rejected call site %#x" % site)
    staticclass_functions = {r.staticclass for r in regs if r.staticclass is not None}
    by_staticclass = {}
    for reg in regs:
        complete_init(reg, staticclass_functions)
        if reg.staticclass is not None:
            by_staticclass[reg.staticclass] = reg
    for reg in regs:
        if reg.literal_prefix() is None:
            reg.derived_prefix = derive_prefix(reg, by_staticclass)
            log("  no literal prefix for %s: derived %s (super chain), symbol %s" % (reg.name(), reg.derived_prefix, reg.symbol_class))

    def class_of(staticclass_ea):
        reg = by_staticclass.get(staticclass_ea)
        return reg.cpp_name() if reg else ""

    f, writer = open_csv(database_out, EXPORT_HEADER)
    with f:
        for reg in sorted(regs, key=lambda r: r.registrant):
            package = read_wide_string(reg.value("package_ptr")) if reg.value("package_ptr") else None
            config = read_wide_string(reg.value("config_ptr")) if reg.value("config_ptr") else None
            flags = ((reg.value("flags_hi") or 0) << 32) | (reg.value("flags_lo") or 0)
            writer.writerow([
                reg.cpp_name() or "", reg.name() or "", package or "", reg.value("size"),
                "%#x" % (reg.value("class_flags") or 0), "%#x" % (reg.value("other_flags") or 0), "%#x" % (reg.value("cast_flags") or 0),
                config or "", "%#x" % flags,
                class_of(reg.super_staticclass), class_of(reg.within_staticclass),
                "%#x" % rva(reg.registrant), "%#x" % rva(reg.staticclass) if reg.staticclass else "", "%#x" % rva(reg.init) if reg.init else "",
                "%#x" % rva(reg.container) if reg.container else "",
                "%#x" % rva(reg.value("ctor")) if reg.value("ctor") else "", "%#x" % rva(reg.value("static_ctor")) if reg.value("static_ctor") else "",
                "%#x" % rva(reg.value("static_init")) if reg.value("static_init") else "", reg.mode,
            ])
    if debug:
        for reg in regs[:3]:
            log("sample %s: %s" % (hex(reg.call_site), {k: hex(v) if isinstance(v, int) else v for k, v in reg.args.items()}))
    log("wrote %s" % database_out)


def merge(csv_2012: Path, csv_2013: Path, out: Path, sizes_csv: Path | None) -> None:
    rows_2012 = {r["class"]: r for r in csv.DictReader(csv_2012.open(encoding="utf-8")) if r["class"]}
    rows_2013 = {r["class"]: r for r in csv.DictReader(csv_2013.open(encoding="utf-8")) if r["class"]}
    pdb_sizes = {}
    if sizes_csv is not None:
        pdb_sizes = {r["name"]: int(r["size"]) for r in csv.DictReader(sizes_csv.open(encoding="utf-8"))}
    header = ["class", "package_2013", "size_2012", "size_2013", "delta", "flags_2012", "flags_2013", "registrant_rva_2012", "registrant_rva_2013",
              "package_2012", "super_2012", "super_2013", "within_2013", "cast_flags_2012", "cast_flags_2013", "other_flags_2012", "other_flags_2013",
              "config_2013", "staticclass_rva_2012", "staticclass_rva_2013", "init_rva_2012", "init_rva_2013", "container_rva_2012", "container_rva_2013",
              "ctor_rva_2012", "ctor_rva_2013", "pdb_size_2012"]
    out.parent.mkdir(parents=True, exist_ok=True)
    mismatches = []
    with out.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        for name in sorted(set(rows_2012) | set(rows_2013)):
            a, b = rows_2012.get(name), rows_2013.get(name)
            size_a = int(a["size"]) if a else None
            size_b = int(b["size"]) if b else None
            pdb = pdb_sizes.get(name)
            if a and pdb is not None and pdb != size_a:
                mismatches.append((name, size_a, pdb))
            get = lambda r, k: r[k] if r else ""
            writer.writerow([
                name, get(b, "package"), size_a if a else "", size_b if b else "", (size_b - size_a) if a and b else "",
                get(a, "class_flags"), get(b, "class_flags"), get(a, "registrant_rva"), get(b, "registrant_rva"),
                get(a, "package"), get(a, "super"), get(b, "super"), get(b, "within"), get(a, "cast_flags"), get(b, "cast_flags"),
                get(a, "other_flags"), get(b, "other_flags"), get(b, "config"), get(a, "staticclass_rva"), get(b, "staticclass_rva"),
                get(a, "init_rva"), get(b, "init_rva"), get(a, "container_rva"), get(b, "container_rva"), get(a, "ctor_rva"), get(b, "ctor_rva"),
                pdb if pdb is not None else "",
            ])
    print_summary(rows_2012, rows_2013, pdb_sizes, mismatches)


def print_summary(rows_2012, rows_2013, pdb_sizes, mismatches) -> None:
    both = sorted(set(rows_2012) & set(rows_2013))
    only_2012 = sorted(set(rows_2012) - set(rows_2013))
    only_2013 = sorted(set(rows_2013) - set(rows_2012))
    print("classes 2012: %d, 2013: %d, both: %d, only 2012: %d, only 2013: %d" % (len(rows_2012), len(rows_2013), len(both), len(only_2012), len(only_2013)))
    if pdb_sizes:
        checked = [n for n in rows_2012 if n in pdb_sizes]
        print("2012 validation: %d of %d classes have a PDB size, %d mismatches, %d not in sizes.csv" % (
            len(checked), len(rows_2012), len(mismatches), len(rows_2012) - len(checked)))
        for name, got, pdb in mismatches:
            print("  mismatch %s: registered %d, PDB %d" % (name, got, pdb))
        for name in sorted(set(rows_2012) - set(pdb_sizes)):
            print("  not in sizes.csv: %s" % name)
    changed = [(n, int(rows_2012[n]["size"]), int(rows_2013[n]["size"])) for n in both if rows_2012[n]["size"] != rows_2013[n]["size"]]
    print("size changes: %d of %d shared classes" % (len(changed), len(both)))
    by_package = defaultdict(list)
    for n, a, b in changed:
        by_package[rows_2013[n]["package"]].append((n, a, b))
    for package in sorted(by_package):
        print("\n### %s (%d changed)\n" % (package, len(by_package[package])))
        print("| class | size_2012 | size_2013 | delta | super_2013 |")
        print("|---|---|---|---|---|")
        for n, a, b in sorted(by_package[package]):
            print("| %s | %d | %d | %+d | %s |" % (n, a, b, b - a, rows_2013[n]["super"]))
    pkg_counts = Counter(rows_2013[n]["package"] for n in rows_2013)
    print("\n2013 classes per package: " + ", ".join("%s %d" % kv for kv in sorted(pkg_counts.items(), key=lambda kv: -kv[1])))
    flag_changes = [n for n in both if rows_2012[n]["class_flags"] != rows_2013[n]["class_flags"]]
    print("class_flags changes: %d" % len(flag_changes))
    for n in flag_changes[:50]:
        print("  %s: %s -> %s" % (n, rows_2012[n]["class_flags"], rows_2013[n]["class_flags"]))
    super_changes = [n for n in both if rows_2012[n]["super"] != rows_2013[n]["super"]]
    print("super changes: %d" % len(super_changes))
    for n in super_changes:
        print("  %s: %s -> %s" % (n, rows_2012[n]["super"], rows_2013[n]["super"]))
    print("\n### Only in 2013 (%d)\n" % len(only_2013))
    print("| class | package | size_2013 | super_2013 |")
    print("|---|---|---|---|")
    for n in only_2013:
        print("| %s | %s | %s | %s |" % (n, rows_2013[n]["package"], rows_2013[n]["size"], rows_2013[n]["super"]))
    print("\n### Only in 2012 (%d)\n" % len(only_2012))
    print("| class | package | size_2012 | super_2012 |")
    print("|---|---|---|---|")
    for n in only_2012:
        print("| %s | %s | %s | %s |" % (n, rows_2012[n]["package"], rows_2012[n]["size"], rows_2012[n]["super"]))
    print("\n### Core contract types\n")
    print("| class | size_2012 | size_2013 | delta | super_2013 |")
    print("|---|---|---|---|---|")
    for n in CORE_CONTRACT:
        a, b = rows_2012.get(n), rows_2013.get(n)
        if a or b:
            sa = int(a["size"]) if a else None
            sb = int(b["size"]) if b else None
            print("| %s | %s | %s | %s | %s |" % (n, sa if a else "-", sb if b else "-", ("%+d" % (sb - sa)) if a and b else "-", b["super"] if b else "-"))


def main(argv: list[str]) -> None:
    if argv and argv[0] == "merge":
        rest = [a for a in argv[1:] if not a.startswith("--")]
        sizes = None
        if "--sizes" in argv:
            sizes = Path(argv[argv.index("--sizes") + 1])
            rest = [a for a in rest if a != str(sizes)]
        merge(Path(rest[0]), Path(rest[1]), Path(rest[2]), sizes)
        return
    out = Path(argv[0]) if argv else Path("class_sizes.csv")
    export(out, "--debug" in argv)


if __name__ == "__main__":
    main(sys.argv[1:])
