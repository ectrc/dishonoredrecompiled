"""Phase 3 J: propagate the 2012 PDB function names onto the 2013 retail database.

Sub-commands. The IDA ones run through run.py (one database per process), the offline ones with
plain python:

  analyze   python run.py match_functions.py <db.i64> --save analyze
            Finish IDA's auto-analysis (the retail db was saved before it completed) and define
            functions at every remaining code head that is a call target or follows a function.
  export    python run.py match_functions.py <db.i64> export <features.pkl>
            Dump per-function features: fixup-masked byte hash, instruction tokens, callees,
            string references (direct, and through the first 8 dwords of a referenced data
            struct: the 2013 build keeps class name/package in a static registration struct
            where 2012 pushes the literals), imports, data references, function-pointer tables.
  match     python match_functions.py match <features_2012.pkl> <features_2013.pkl> <out.csv>
            Match the two dumps: import thunks, unique masked-byte hashes, string anchors,
            call-graph propagation (callees/callers of matched pairs), unique token streams,
            link-order alignment between matched anchors; iterated until no pass adds a pair.
  apply     python run.py match_functions.py <db2013.i64> --save apply <match.csv> [min_ratio]
            Rename the 2013 functions with ratio >= min_ratio (default 0.9) and the matched globals, then
            name every exec function from the 2013 native registration table (authoritative; overrides).
  report    python match_functions.py report <match.csv> <features_2012.pkl> <features_2013.pkl> <out.md>

Ratio is the confidence of a pair: 1.0 for identical masked bytes and import thunks, otherwise the
difflib similarity of the two instruction-token streams (1.0 = identical instruction stream, only
addresses differ). Method names the evidence that produced the pair.
"""
import bisect
import csv
import difflib
import re
import hashlib
import pickle
import sys
from collections import Counter, defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
SYMBOLS_DIR = REPO / "resources" / "docs" / "symbols"
IMAGE_BASE_2013 = 0x400000
AUTO_NAME_PREFIXES = ("sub_", "nullsub_", "j_", "unknown_libname_", "loc_", "def_", "locret_")


def log(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)


def normalize_import(name: str) -> str:
    name = name.replace("__imp_", "", 1) if name.startswith("__imp_") else name
    name = name.lstrip("_")
    at = name.rfind("@")
    if at > 0 and name[at + 1:].isdigit():
        name = name[:at]
    return name


# ----------------------------------------------------------------------------------------------
# IDA side
# ----------------------------------------------------------------------------------------------

def ida_analyze() -> None:
    import ida_auto
    import ida_bytes
    import ida_funcs
    import ida_segment
    import ida_xref
    import idautils

    before = ida_funcs.get_func_qty()
    log(f"functions before: {before}, auto_is_ok={ida_auto.auto_is_ok()}")
    ida_auto.auto_wait()
    log(f"after auto_wait: {ida_funcs.get_func_qty()}")

    for _round in range(3):
        added = 0
        for seg_ea in idautils.Segments():
            seg = ida_segment.getseg(seg_ea)
            if not seg.perm & ida_segment.SEGPERM_EXEC:
                continue
            ea = seg.start_ea
            while ea < seg.end_ea:
                flags = ida_bytes.get_flags(ea)
                if ida_bytes.is_code(flags) and ida_funcs.get_func(ea) is None:
                    is_target = any(x.type in (ida_xref.fl_CN, ida_xref.fl_CF, ida_xref.fl_JN, ida_xref.fl_JF) and ida_funcs.get_func(x.frm) is not None for x in idautils.XrefsTo(ea, 0))
                    prev_flags = ida_bytes.get_flags(ida_bytes.prev_head(ea, seg.start_ea))
                    follows_func_end = ida_funcs.get_func(ida_bytes.prev_head(ea, seg.start_ea)) is not None or ida_bytes.is_align(prev_flags) or ida_bytes.is_data(prev_flags)
                    if (is_target or follows_func_end) and ida_funcs.add_func(ea):
                        added += 1
                        func = ida_funcs.get_func(ea)
                        ea = func.end_ea if func else ida_bytes.next_head(ea, seg.end_ea)
                        continue
                ea = ida_bytes.next_head(ea, seg.end_ea)
        ida_auto.auto_wait()
        log(f"round {_round}: added {added}, functions now {ida_funcs.get_func_qty()}")
        if added == 0:
            break
    log(f"functions after: {ida_funcs.get_func_qty()} (+{ida_funcs.get_func_qty() - before})")


def ida_export(out_path: Path) -> None:
    import ida_bytes
    import ida_fixup
    import ida_funcs
    import ida_idaapi
    import ida_nalt
    import ida_name
    import ida_segment
    import ida_ua
    import idautils

    base = ida_nalt.get_imagebase()
    fixups = set()
    ea = ida_fixup.get_first_fixup_ea()
    while ea != ida_idaapi.BADADDR:
        fixups.add(ea)
        ea = ida_fixup.get_next_fixup_ea(ea)
    log(f"fixups: {len(fixups)}")

    import_ranges = []
    data_ranges = []
    for seg_ea in idautils.Segments():
        seg = ida_segment.getseg(seg_ea)
        name = ida_segment.get_segm_name(seg)
        if name == ".idata":
            import_ranges.append((seg.start_ea, seg.end_ea))
        elif not seg.perm & ida_segment.SEGPERM_EXEC and name != "HEADER":
            data_ranges.append((seg.start_ea, seg.end_ea))

    def in_ranges(addr, ranges):
        return any(lo <= addr < hi for lo, hi in ranges)

    string_cache = {}

    def string_at(addr):
        if addr in string_cache:
            return string_cache[addr]
        result = None
        raw = ida_bytes.get_bytes(addr, 128)
        if raw:
            if raw[0] and raw[1] == 0 and raw[0] < 0x80:
                chars = []
                for i in range(0, len(raw) - 1, 2):
                    lo, hi = raw[i], raw[i + 1]
                    if hi != 0 or lo == 0:
                        if lo == 0 and hi == 0 and len(chars) >= 3:
                            result = "L:" + "".join(chars)
                        break
                    if lo < 0x20 and lo not in (9, 10, 13):
                        break
                    chars.append(chr(lo))
                else:
                    if len(chars) >= 8:
                        result = "L:" + "".join(chars)
            elif 0x20 <= raw[0] < 0x7F:
                n = 0
                while n < len(raw) and (0x20 <= raw[n] < 0x7F or raw[n] in (9, 10, 13)):
                    n += 1
                if n >= 4 and (n == len(raw) or raw[n] == 0):
                    result = "A:" + raw[:n].decode("latin-1")
        string_cache[addr] = result
        return result

    dtype_sizes = {ida_ua.dt_byte: 1, ida_ua.dt_word: 2, ida_ua.dt_dword: 4, ida_ua.dt_qword: 8}
    functions = {}
    insn = ida_ua.insn_t()
    count = 0
    for func_ea in idautils.Functions():
        func = ida_funcs.get_func(func_ea)
        name = ida_funcs.get_func_name(func_ea) or ""
        masked = bytearray()
        tokens = []
        mnems = []
        calls = []
        strings = []
        imports = []
        consts = []
        drefs = []
        n_insn = 0
        for chunk_start, chunk_end in _chunks(func):
            ea = chunk_start
            while ea < chunk_end:
                if not ida_bytes.is_code(ida_bytes.get_flags(ea)):
                    ea = ida_bytes.next_head(ea, chunk_end)
                    continue
                size = ida_ua.decode_insn(insn, ea)
                if size <= 0:
                    ea = ida_bytes.next_head(ea, chunk_end)
                    continue
                raw = bytearray(ida_bytes.get_bytes(ea, size) or b"")
                for off in range(size):
                    if ea + off in fixups:
                        for k in range(off, min(off + 4, size)):
                            raw[k] = 0
                mnem = insn.get_canon_mnem()
                parts = [mnem]
                for op in insn.ops:
                    if op.type == ida_ua.o_void:
                        break
                    t = op.type
                    if t == ida_ua.o_reg:
                        parts.append(f"r{op.reg}")
                    elif t == ida_ua.o_imm:
                        if ea + op.offb in fixups:
                            parts.append("A")
                            s = string_at(op.value)
                            if s:
                                strings.append(s)
                            elif in_ranges(op.value, data_ranges):
                                drefs.append(op.value - base)
                        else:
                            parts.append(f"i{op.value & 0xFFFFFFFF:x}")
                            if op.value > 0xFF and (op.value & 0xFFFFFFFF) < 0xFFFFFF00:
                                consts.append(op.value & 0xFFFFFFFF)
                    elif t == ida_ua.o_mem:
                        addr = op.addr
                        if in_ranges(addr, import_ranges):
                            imp = ida_name.get_name(addr) or ""
                            parts.append("I")
                            if imp:
                                imports.append(normalize_import(imp))
                        else:
                            parts.append("M")
                            s = string_at(addr)
                            if s:
                                strings.append(s)
                            elif in_ranges(addr, data_ranges):
                                drefs.append(addr - base)
                    elif t == ida_ua.o_displ:
                        if ea + op.offb in fixups:
                            parts.append(f"d{op.reg}A")
                            s = string_at(op.addr)
                            if s:
                                strings.append(s)
                            elif in_ranges(op.addr, data_ranges):
                                drefs.append(op.addr - base)
                        else:
                            parts.append(f"d{op.reg}:{op.addr & 0xFFFFFFFF:x}")
                    elif t == ida_ua.o_phrase:
                        parts.append(f"p{op.reg}")
                    elif t in (ida_ua.o_near, ida_ua.o_far):
                        parts.append("N")
                        width = dtype_sizes.get(op.dtype, 4)
                        for k in range(op.offb, min(op.offb + width, size)):
                            raw[k] = 0
                        target = op.addr
                        tf = ida_funcs.get_func(target)
                        if tf is not None and tf.start_ea != func.start_ea and mnem in ("call", "jmp"):
                            calls.append(tf.start_ea)
                    else:
                        parts.append(f"t{t}")
                masked += raw
                tokens.append(" ".join(parts))
                mnems.append(mnem)
                n_insn += 1
                ea += size
        for d in dict.fromkeys(drefs):
            for k in range(8):
                slot = base + d + 4 * k
                if slot in fixups:
                    s = string_at(ida_bytes.get_dword(slot))
                    if s and s not in strings:
                        strings.append(s)
        functions[func_ea - base] = {
            "rva": func_ea - base,
            "size": func.size(),
            "name": name,
            "thunk": bool(func.flags & ida_funcs.FUNC_THUNK),
            "hash": hashlib.md5(bytes(masked)).hexdigest(),
            "tokens": tokens,
            "mnems": mnems,
            "n_insn": n_insn,
            "calls": [c - base for c in calls],
            "strings": strings,
            "imports": imports,
            "consts": consts,
            "drefs": drefs,
        }
        count += 1
        if count % 5000 == 0:
            log(f"exported {count}")
    referenced = set()
    for fn in functions.values():
        referenced.update(fn["drefs"])
    vtables = []
    run_start = None
    run = []
    prev = None
    for ea in sorted(fixups):
        if not in_ranges(ea, data_ranges):
            continue
        target = ida_bytes.get_dword(ea)
        tf = ida_funcs.get_func(target)
        is_slot = tf is not None and tf.start_ea == target
        if is_slot and prev is not None and ea == prev + 4 and (ea - base) not in referenced:
            run.append(target - base)
        else:
            if run:
                vtables.append((run_start - base, run))
            run = [target - base] if is_slot else []
            run_start = ea if is_slot else None
        prev = ea if is_slot else None
    if run:
        vtables.append((run_start - base, run))
    globals_ = {}
    for ea, name in idautils.Names():
        if in_ranges(ea, data_ranges) and not name.startswith(AUTO_NAME_PREFIXES + ("off_", "dword_", "byte_", "word_", "unk_", "asc_", "stru_", "flt_", "dbl_", "xmmword_", "qword_", "a")):
            globals_[ea - base] = name
    with out_path.open("wb") as f:
        pickle.dump({"base": base, "functions": functions, "vtables": vtables, "globals": globals_}, f, protocol=pickle.HIGHEST_PROTOCOL)
    log(f"exported {count} functions, {len(vtables)} function-pointer runs, {len(globals_)} named globals to {out_path}")


def _chunks(func):
    import ida_funcs

    it = ida_funcs.func_tail_iterator_t(func)
    ok = it.main()
    while ok:
        chunk = it.chunk()
        yield chunk.start_ea, chunk.end_ea
        ok = it.next()


def ida_apply(csv_path: Path, min_ratio: float) -> None:
    import ida_funcs
    import ida_name
    import ida_nalt

    base = ida_nalt.get_imagebase()
    applied = 0
    skipped_auto = 0
    skipped_low = 0
    commented = 0
    globals_applied = 0
    with csv_path.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            if not row["rva_2013"] or not row["rva_2012"]:
                continue
            ratio = float(row["ratio"])
            ea = base + int(row["rva_2013"], 16)
            if row.get("kind") == "global":
                name = row["name"]
                if ratio >= min_ratio and name and not name.startswith(AUTO_NAME_PREFIXES) and ida_funcs.get_func(ea) is None:
                    if ida_name.get_name(ea) == name or ida_name.set_name(ea, name, ida_name.SN_NOCHECK | ida_name.SN_FORCE | ida_name.SN_NOWARN):
                        globals_applied += 1
                continue
            func = ida_funcs.get_func(ea)
            if func is None or func.start_ea != ea:
                continue
            ida_funcs.set_func_cmt(func, f"2012 rva {row['rva_2012']} ratio {ratio:.3f} {row['method']}", False)
            commented += 1
            if ratio < min_ratio:
                skipped_low += 1
                continue
            name = row["name"]
            if not name or name.startswith(AUTO_NAME_PREFIXES):
                skipped_auto += 1
                continue
            current = ida_funcs.get_func_name(ea) or ""
            if current == name:
                applied += 1
                continue
            if ida_name.set_name(ea, name, ida_name.SN_NOCHECK | ida_name.SN_FORCE | ida_name.SN_NOWARN):
                applied += 1
            else:
                log(f"set_name failed at {ea:#x}: {name}")
    log(f"applied {applied} function names (ratio >= {min_ratio}), skipped {skipped_low} low-ratio, {skipped_auto} auto-named; {commented} function comments; {globals_applied} global names")
    apply_native_table(base)


def native_table(base):
    import ida_bytes
    import ida_fixup
    import ida_funcs
    import ida_idaapi
    import ida_segment

    pattern = re.compile(r"^(\w+?)exec(\w+)$")
    records = []
    ea = ida_fixup.get_first_fixup_ea()
    while ea != ida_idaapi.BADADDR:
        seg = ida_segment.getseg(ea)
        if seg and not seg.perm & ida_segment.SEGPERM_EXEC:
            target = ida_bytes.get_dword(ea)
            raw = (ida_bytes.get_bytes(target, 96) or b"") if ida_segment.getseg(target) else b""
            text = raw.split(b"\0")[0].decode("latin-1") if raw else ""
            m = pattern.match(text) if text else None
            if m and ida_fixup.get_fixup(ida_fixup.fixup_data_t(), ea + 4):
                func_ea = ida_bytes.get_dword(ea + 4)
                func = ida_funcs.get_func(func_ea)
                if func is not None and func.start_ea == func_ea:
                    records.append((ea - base, m.group(1), m.group(2), func_ea - base))
        ea = ida_fixup.get_next_fixup_ea(ea)
    return records


def apply_native_table(base):
    """The 2013 exe registers name-bound natives from a static table of {"<Class>exec<Func>", &Class::execFunc}
    records (2012 used one dynamic initializer per native instead). The table is authoritative for the exec
    function names, so it overrides propagated names and names the DLC05-07 natives that have no 2012 counterpart."""
    import ida_funcs
    import ida_name

    names_at = defaultdict(list)
    for _ea, cls, func, rva in native_table(base):
        names_at[rva].append((cls, func))
    agree = disagree = new = 0
    exec_re = re.compile(r"^\?exec(\w+)@(\w+)@@")
    for rva, names in names_at.items():
        ea = base + rva
        current = ida_funcs.get_func_name(ea) or ""
        m = exec_re.match(current)
        if m and (m.group(2), m.group(1)) in names:
            agree += 1
            continue
        cls, func = names[0]
        mangled = f"?exec{func}@{cls}@@QAEXAAUFFrame@@QAX@Z"
        if m:
            disagree += 1
            comment = f"native table: {cls}::exec{func}; propagated 2012 name was {current}"
        else:
            new += 1
            comment = f"native table: {cls}::exec{func}"
        if len(names) > 1:
            comment += "; also " + ", ".join(f"{c}::exec{f}" for c, f in names[1:])
        ida_name.set_name(ea, mangled, ida_name.SN_NOCHECK | ida_name.SN_FORCE | ida_name.SN_NOWARN)
        func_t = ida_funcs.get_func(ea)
        old = ida_funcs.get_func_cmt(func_t, False) or ""
        ida_funcs.set_func_cmt(func_t, (old + "\n" if old else "") + comment, False)
    log(f"native table: {sum(len(v) for v in names_at.values())} records, {len(names_at)} exec functions; propagated names agree {agree}, overridden {disagree}, newly named {new}")


# ----------------------------------------------------------------------------------------------
# Offline matcher
# ----------------------------------------------------------------------------------------------

class Side:
    def __init__(self, path: Path):
        with path.open("rb") as f:
            dump = pickle.load(f)
        self.base = dump["base"]
        self.funcs = dump["functions"]
        self.vtables = dump.get("vtables", [])
        self.global_names = dump.get("globals") or load_global_names_csv()
        self.vt_start = {start: i for i, (start, _slots) in enumerate(self.vtables)}
        self.order = sorted(self.funcs)
        self.index = {rva: i for i, rva in enumerate(self.order)}
        self.callers = defaultdict(list)
        self.by_dref = defaultdict(set)
        self.by_hash = defaultdict(list)
        self.by_shape = defaultdict(list)
        self.shape = {}
        self.by_tokens = defaultdict(list)
        self.by_string = defaultdict(set)
        self.by_import_thunk = defaultdict(list)
        self.slots = defaultdict(list)
        for rva in self.order:
            fn = self.funcs[rva]
            for callee in fn["calls"]:
                self.callers[callee].append(rva)
        self.xstrings = {}
        for rva in self.order:
            fn = self.funcs[rva]
            for g in set(fn["drefs"]):
                self.by_dref[g].add(rva)
            self.by_hash[fn["hash"]].append(rva)
            self.by_tokens[hashlib.md5("\n".join(fn["tokens"]).encode()).hexdigest()].append(rva)
            shape = hashlib.md5("\n".join(shape_token(t) for t in fn["tokens"]).encode()).hexdigest()
            self.shape[rva] = shape
            self.by_shape[shape].append(rva)
            xstrings = list(fn["strings"])
            for callee in dict.fromkeys(fn["calls"]):
                if callee in self.funcs and len(set(self.callers[callee])) == 1:
                    xstrings += [x for x in self.funcs[callee]["strings"] if x not in xstrings]
            self.xstrings[rva] = xstrings
            for s in set(xstrings):
                self.by_string[s].add(rva)
            if fn["n_insn"] == 1 and fn["imports"] and fn["mnems"][0] == "jmp":
                self.by_import_thunk[fn["imports"][0]].append(rva)
        self.tables_of = defaultdict(set)
        for vt_index, (_start, slots) in enumerate(self.vtables):
            for i, f in enumerate(slots):
                self.slots[(f, i)].append(vt_index)
                self.tables_of[f].add(vt_index)


MAX_FANOUT = 64


class Matcher:
    def __init__(self, a: Side, b: Side):
        self.a = a
        self.b = b
        self.a2b = {}
        self.b2a = {}
        self.result = {}
        self.sim_cache = {}
        self.g_a2b = {}
        self.g_info = {}
        self.vt_a2b = {}
        self.vt_b2a = {}

    def add(self, ra, rb, ratio, method):
        if ra in self.a2b or rb in self.b2a:
            return False
        self.a2b[ra] = rb
        self.b2a[rb] = ra
        self.result[ra] = (rb, ratio, method)
        return True

    def sim(self, ra, rb):
        key = (ra, rb)
        if key in self.sim_cache:
            return self.sim_cache[key]
        fa = self.a.funcs[ra]
        fb = self.b.funcs[rb]
        if fa["hash"] == fb["hash"]:
            value = 1.0
        else:
            ta, tb = fa["tokens"], fb["tokens"]
            if ta == tb:
                value = 1.0
            elif len(ta) + len(tb) > 6000:
                value = ngram_sim(ta, tb)
            else:
                value = difflib.SequenceMatcher(None, ta, tb, autojunk=False).ratio()
        self.sim_cache[key] = value
        return value

    def size_plausible(self, ra, rb):
        na, nb = self.a.funcs[ra]["n_insn"], self.b.funcs[rb]["n_insn"]
        return min(na, nb) * 2 >= max(na, nb) or abs(na - nb) <= 4

    def agreement(self, ra, rb):
        fa = self.a.funcs[ra]
        fb = self.b.funcs[rb]
        parts = []
        mapped = [self.a2b[c] for c in fa["calls"] if c in self.a2b]
        if mapped:
            cb = set(fb["calls"])
            parts.append(sum(1 for c in mapped if c in cb) / len(mapped))
        mapped = [self.a2b[c] for c in self.a.callers.get(ra, ()) if c in self.a2b]
        if mapped:
            cb = set(self.b.callers.get(rb, ()))
            parts.append(sum(1 for c in mapped if c in cb) / len(mapped))
        mapped = [self.g_a2b[g] for g in fa["drefs"] if g in self.g_a2b]
        if mapped:
            gb = set(fb["drefs"])
            parts.append(sum(1 for g in mapped if g in gb) / len(mapped))
        return sum(parts) / len(parts) if parts else None

    def score(self, ra, rb):
        s = self.sim(ra, rb)
        agree = self.agreement(ra, rb)
        return s if agree is None else 0.5 * s + 0.5 * agree

    def best_candidate(self, ra, candidates, min_sim, method):
        scored = []
        for rb in candidates:
            if rb in self.b2a or not self.size_plausible(ra, rb):
                continue
            s = self.sim(ra, rb)
            if s < min_sim:
                continue
            scored.append((self.score(ra, rb), s, rb))
        if not scored:
            return 0
        scored.sort(reverse=True)
        best, s, rb = scored[0]
        second = scored[1][0] if len(scored) > 1 else 0.0
        if best >= 0.6 and best - second >= 0.1:
            return self.add(ra, rb, s, method)
        return 0

    # ---- anchor passes ---------------------------------------------------------------------

    def pass_imports(self):
        n = 0
        for imp, ras in self.a.by_import_thunk.items():
            rbs = self.b.by_import_thunk.get(imp, [])
            if len(ras) == 1 and len(rbs) == 1:
                n += self.add(ras[0], rbs[0], 1.0, "import")
        return n

    def shape_unique(self, ra, rb):
        return len(self.a.by_shape[self.a.shape[ra]]) == 1 and len(self.b.by_shape[self.b.shape[rb]]) == 1

    def pass_hash(self):
        n = 0
        for h, ras in self.a.by_hash.items():
            rbs = self.b.by_hash.get(h)
            if rbs and len(ras) == 1 and len(rbs) == 1 and self.a.funcs[ras[0]]["n_insn"] >= 2 and self.shape_unique(ras[0], rbs[0]):
                n += self.add(ras[0], rbs[0], 1.0, "bytes")
        return n

    def pass_strings(self):
        n = 0
        for key, ras in self.a.by_string.items():
            rbs = self.b.by_string.get(key)
            if not rbs or len(ras) > 3 or len(rbs) > 3:
                continue
            ras = [r for r in ras if r not in self.a2b]
            rbs = [r for r in rbs if r not in self.b2a]
            if not ras or not rbs:
                continue
            scored = sorted(((self.sim(ra, rb), ra, rb) for ra in ras for rb in rbs), reverse=True)
            best, ra, rb = scored[0]
            if best < 0.5:
                continue
            if len(scored) > 1 and best - scored[1][0] < 0.15:
                continue
            n += self.add(ra, rb, best, "string")
        keyed_a = defaultdict(list)
        keyed_b = defaultdict(list)
        for ra, fn in self.a.funcs.items():
            if ra not in self.a2b and self.a.xstrings[ra]:
                keyed_a[tuple(sorted(set(self.a.xstrings[ra])))].append(ra)
        for rb, fn in self.b.funcs.items():
            if rb not in self.b2a and self.b.xstrings[rb]:
                keyed_b[tuple(sorted(set(self.b.xstrings[rb])))].append(rb)
        for key, ras in keyed_a.items():
            rbs = keyed_b.get(key)
            if rbs and len(ras) == 1 and len(rbs) == 1:
                ratio = self.sim(ras[0], rbs[0])
                if ratio >= 0.5:
                    n += self.add(ras[0], rbs[0], ratio, "string-set")
        return n

    def pass_tokens(self):
        n = 0
        for h, ras in self.a.by_tokens.items():
            ras = [r for r in ras if r not in self.a2b]
            if len(ras) != 1:
                continue
            rbs = [r for r in self.b.by_tokens.get(h, []) if r not in self.b2a]
            if len(rbs) != 1 or self.a.funcs[ras[0]]["n_insn"] < 3:
                continue
            ra, rb = ras[0], rbs[0]
            if len([r for r in self.a.by_shape[self.a.shape[ra]] if r not in self.a2b]) == 1 and len([r for r in self.b.by_shape[self.b.shape[rb]] if r not in self.b2a]) == 1:
                n += self.add(ra, rb, 1.0, "tokens")
        return n

    # ---- propagation passes ----------------------------------------------------------------

    def pass_globals(self):
        votes = defaultdict(Counter)
        exact = set()
        for ra, rb in self.a2b.items():
            da = self.a.funcs[ra]["drefs"]
            db = self.b.funcs[rb]["drefs"]
            if da and len(da) == len(db) and self.sim(ra, rb) >= 0.9:
                identical = self.sim(ra, rb) == 1.0
                for ga, gb in zip(da, db):
                    votes[ga][gb] += 1
                    if identical:
                        exact.add((ga, gb))
        reverse = defaultdict(Counter)
        for ga, counter in votes.items():
            for gb, c in counter.items():
                reverse[gb][ga] += c
        n = 0
        for ga, counter in votes.items():
            gb, c = counter.most_common(1)[0]
            if reverse[gb].most_common(1)[0][0] == ga and self.g_a2b.get(ga) != gb:
                self.g_a2b[ga] = gb
                self.g_info[ga] = (1.0 if c >= 2 else 0.9 if (ga, gb) in exact else 0.8, "global-votes")
                n += 1
        return n

    def pass_vtables(self):
        n = 0
        for ia, (start_a, slots_a) in enumerate(self.a.vtables):
            if ia in self.vt_a2b:
                ib = self.vt_a2b[ia]
            else:
                votes = Counter()
                for fa in set(slots_a):
                    fb = self.a2b.get(fa)
                    if fb is None:
                        continue
                    for ib in self.b.tables_of.get(fb, ()):
                        votes[ib] += 1
                if not votes:
                    continue
                top = votes.most_common(2)
                ib, c = top[0]
                if len(top) > 1 and top[1][1] == c:
                    continue
                shortest = min(len(slots_a), len(self.b.vtables[ib][1]))
                if ib in self.vt_b2a or c < min(shortest, 2) or c < 0.2 * shortest:
                    continue
                self.vt_a2b[ia] = ib
                self.vt_b2a[ib] = ia
                start_b = self.b.vtables[ib][0]
                if start_a not in self.g_a2b:
                    self.g_a2b[start_a] = start_b
                    self.g_info[start_a] = (1.0, "global-table")
            n += self.align_slots(slots_a, self.b.vtables[ib][1])
        return n

    def align_slots(self, slots_a, slots_b):
        ta = [("m", self.a2b[f]) if f in self.a2b else ("h", self.a.funcs[f]["hash"]) for f in slots_a]
        tb = [("m", f) if f in self.b2a else ("h", self.b.funcs[f]["hash"]) for f in slots_b]
        n = 0
        blocks = difflib.SequenceMatcher(None, ta, tb, autojunk=False).get_matching_blocks()
        prev_a = prev_b = 0
        for block in blocks:
            gap_a = slots_a[prev_a:block.a]
            gap_b = slots_b[prev_b:block.b]
            if gap_a and len(gap_a) == len(gap_b):
                for fa, fb in zip(gap_a, gap_b):
                    n += self.try_pair(fa, fb, 0.5, "vtable")
            for k in range(block.size):
                fa, fb = slots_a[block.a + k], slots_b[block.b + k]
                if ta[block.a + k][0] == "h":
                    n += self.try_pair(fa, fb, 0.5, "vtable")
            prev_a, prev_b = block.a + block.size, block.b + block.size
        return n

    def try_pair(self, fa, fb, min_sim, method):
        if fa in self.a2b or fb in self.b2a or not self.size_plausible(fa, fb):
            return 0
        s = self.sim(fa, fb)
        if s < min_sim:
            return 0
        return self.add(fa, fb, s, method)

    def pass_callgraph(self):
        n = 0
        for ra in list(self.a2b):
            rb = self.a2b[ra]
            ca = self.a.funcs[ra]["calls"]
            cb = self.b.funcs[rb]["calls"]
            if not ca or not cb:
                continue
            if len(ca) == len(cb):
                for x, y in zip(ca, cb):
                    if x in self.a2b or y in self.b2a:
                        continue
                    if self.size_plausible(x, y):
                        s = self.sim(x, y)
                        if s >= 0.6:
                            n += self.add(x, y, s, "callee")
            else:
                ua = [x for x in dict.fromkeys(ca) if x not in self.a2b]
                ub = [y for y in dict.fromkeys(cb) if y not in self.b2a]
                for x in ua:
                    n += self.best_candidate(x, ub, 0.6, "callee")
        for ra in self.a.order:
            if ra in self.a2b:
                continue
            fa = self.a.funcs[ra]
            candidates = set()
            for caller in self.a.callers.get(ra, ()):
                mb = self.a2b.get(caller)
                if mb is not None:
                    candidates.update(self.b.funcs[mb]["calls"])
            for callee in fa["calls"]:
                mb = self.a2b.get(callee)
                if mb is not None:
                    callers_b = self.b.callers.get(mb, ())
                    if len(callers_b) <= MAX_FANOUT:
                        candidates.update(callers_b)
            for g in fa["drefs"]:
                gb = self.g_a2b.get(g)
                if gb is not None:
                    users = self.b.by_dref.get(gb, ())
                    if len(users) <= MAX_FANOUT:
                        candidates.update(users)
            if candidates:
                n += self.best_candidate(ra, candidates, 0.5, "neighbours")
        return n

    def pass_blocks(self):
        n = 0
        anchors = sorted((self.a.index[ra], self.b.index[rb]) for ra, rb in self.a2b.items())
        prev_i, prev_j = -1, -1
        for i, j in anchors + [(len(self.a.order), len(self.b.order))]:
            if j <= prev_j:
                continue
            ua = [self.a.order[k] for k in range(prev_i + 1, i) if self.a.order[k] not in self.a2b]
            ub = [self.b.order[k] for k in range(prev_j + 1, j) if self.b.order[k] not in self.b2a]
            prev_i, prev_j = i, j
            if not ua or not ub or len(ua) * len(ub) > 40000:
                continue
            sa = [self.a.shape[r] for r in ua]
            sb = [self.b.shape[r] for r in ub]
            count_a = Counter(sa)
            count_b = Counter(sb)
            for block in difflib.SequenceMatcher(None, sa, sb, autojunk=False).get_matching_blocks():
                for k in range(block.size):
                    shape = sa[block.a + k]
                    if count_a[shape] == count_b[shape]:
                        n += self.try_pair(ua[block.a + k], ub[block.b + k], 0.5, "block")
        return n

    def pass_order(self, window=16):
        n = 0
        anchors = sorted(self.a.index[ra] for ra in self.a2b)
        if not anchors:
            return 0
        for ra in self.a.order:
            if ra in self.a2b:
                continue
            i = self.a.index[ra]
            k = bisect.bisect_left(anchors, i)
            predictions = []
            for kk in (k - 1, k):
                if 0 <= kk < len(anchors):
                    ai = anchors[kk]
                    predictions.append(self.b.index[self.a2b[self.a.order[ai]]] + (i - ai))
            scored = []
            seen = set()
            for pred in predictions:
                for j in range(max(0, pred - window), min(len(self.b.order), pred + window + 1)):
                    rb = self.b.order[j]
                    if rb in seen or rb in self.b2a or not self.size_plausible(ra, rb):
                        continue
                    seen.add(rb)
                    sim = self.sim(ra, rb)
                    if sim < 0.6:
                        continue
                    dist = min(abs(j - q) for q in predictions)
                    scored.append((self.score(ra, rb), -dist, sim, rb))
            if not scored:
                continue
            scored.sort(reverse=True)
            best_score, neg_dist, sim, rb = scored[0]
            if best_score < 0.6:
                continue
            if len(scored) > 1:
                second_score, second_neg_dist = scored[1][0], scored[1][1]
                if best_score - second_score < 0.1 and not (-neg_dist <= 2 and second_neg_dist <= neg_dist - 2 and best_score >= 0.9):
                    continue
            siblings = sum(1 for _s, _d, _sim, other in scored if other != rb and self.b.shape[other] == self.b.shape[rb])
            if siblings:
                n += self.add(ra, rb, min(sim, 0.89), "order-sibling")
            else:
                n += self.add(ra, rb, sim, "order")
        return n

    def run(self):
        log(f"import thunks: {self.pass_imports()}")
        log(f"unique bytes: {self.pass_hash()}")
        log(f"strings: {self.pass_strings()}")
        for round_no in range(30):
            counts = {}
            counts["globals"] = self.pass_globals()
            counts["vtables"] = self.pass_vtables()
            counts["callgraph"] = self.pass_callgraph()
            counts["tokens"] = self.pass_tokens()
            counts["strings"] = self.pass_strings()
            counts["blocks"] = self.pass_blocks()
            counts["order"] = self.pass_order()
            n = sum(counts.values())
            log(f"round {round_no}: {counts} -> {len(self.a2b)} matched, {len(self.g_a2b)} globals, {len(self.vt_a2b)} vtables")
            if n == 0:
                break
        demoted = 0
        for ra, (rb, ratio, method) in self.result.items():
            if method in ("order", "block", "neighbours", "vtable", "callee") and ratio >= 0.9:
                agree = self.agreement(ra, rb)
                if agree is not None and agree < 0.5:
                    self.result[ra] = (rb, min(ratio, 0.85), method + "?")
                    demoted += 1
        log(f"demoted {demoted} propagated pairs with neighbour agreement < 0.5 to ratio 0.85")
        return self.result


SHAPE_DISP = re.compile(r"^d(\d+):[0-9a-f]+$")
SHAPE_IMM = re.compile(r"^i([0-9a-f]+)$")


def shape_token(token: str) -> str:
    parts = token.split(" ")
    out = [parts[0]]
    for part in parts[1:]:
        m = SHAPE_DISP.match(part)
        if m:
            out.append(f"d{m.group(1)}")
            continue
        m = SHAPE_IMM.match(part)
        if m and int(m.group(1), 16) >= 0x100:
            out.append("i")
            continue
        out.append(part)
    return " ".join(out)


def ngram_sim(ta, tb, n=3):
    ca = Counter(tuple(ta[i:i + n]) for i in range(max(1, len(ta) - n + 1)))
    cb = Counter(tuple(tb[i:i + n]) for i in range(max(1, len(tb) - n + 1)))
    inter = sum((ca & cb).values())
    return 2.0 * inter / (sum(ca.values()) + sum(cb.values()))


def run_match(path_a: Path, path_b: Path, out_csv: Path) -> None:
    a = Side(path_a)
    b = Side(path_b)
    log(f"2012: {len(a.funcs)} functions, 2013: {len(b.funcs)} functions")
    matcher = Matcher(a, b)
    result = matcher.run()
    counts = Counter(m for _, _, m in result.values())
    log(f"methods: {dict(counts)}")
    high = sum(1 for _, r, _ in result.values() if r >= 0.9)
    log(f"matched {len(result)} ({100.0 * len(result) / len(a.funcs):.1f}% of 2012), ratio>=0.9: {high} ({100.0 * high / len(a.funcs):.1f}%)")
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["rva_2012", "rva_2013", "name", "ratio", "method", "size_2012", "size_2013", "name_2013", "kind"])
        for ra in a.order:
            fa = a.funcs[ra]
            if ra in result:
                rb, ratio, method = result[ra]
                w.writerow([f"0x{ra:x}", f"0x{rb:x}", fa["name"], f"{ratio:.3f}", method, fa["size"], b.funcs[rb]["size"], b.funcs[rb]["name"], "function"])
            else:
                w.writerow([f"0x{ra:x}", "", fa["name"], "", "unmatched", fa["size"], "", "", "function"])
        matched_b = {rb for rb, _, _ in result.values()}
        for rb in b.order:
            if rb not in matched_b:
                fb = b.funcs[rb]
                w.writerow(["", f"0x{rb:x}", "", "", "new", "", fb["size"], fb["name"], "function"])
        named_globals = 0
        for ga in sorted(matcher.g_a2b):
            name = a.global_names.get(ga)
            if not name:
                continue
            gb = matcher.g_a2b[ga]
            ratio, method = matcher.g_info.get(ga, (1.0, "global"))
            w.writerow([f"0x{ga:x}", f"0x{gb:x}", name, f"{ratio:.3f}", method, "", "", b.global_names.get(gb, ""), "global"])
            named_globals += 1
    log(f"globals matched {len(matcher.g_a2b)}, with a 2012 name {named_globals}")


# ----------------------------------------------------------------------------------------------
# Report
# ----------------------------------------------------------------------------------------------

def load_global_names_csv() -> dict[int, str]:
    names = {}
    path = SYMBOLS_DIR / "globals.csv"
    if path.exists():
        with path.open(encoding="utf-8", newline="") as f:
            for row in csv.DictReader(f):
                names[int(row["rva"], 16)] = row["name"]
    return names


def load_modules() -> dict[int, tuple[str, str]]:
    modules = {}
    path = SYMBOLS_DIR / "functions.csv"
    if not path.exists():
        return modules
    with path.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            modules[int(row["rva"], 16)] = (row.get("module") or "?", row.get("file") or "")
    return modules


def run_report(csv_path: Path, path_a: Path, path_b: Path, out_md: Path, apply_log: str = "") -> None:
    a = Side(path_a)
    b = Side(path_b)
    modules = load_modules()
    all_rows = list(csv.DictReader(csv_path.open(encoding="utf-8", newline="")))
    rows = [r for r in all_rows if r.get("kind", "function") == "function"]
    global_rows = [r for r in all_rows if r.get("kind") == "global"]
    matched = [r for r in rows if r["rva_2012"] and r["rva_2013"]]
    unmatched = [r for r in rows if r["rva_2012"] and not r["rva_2013"]]
    new = [r for r in rows if not r["rva_2012"]]
    high = [r for r in matched if float(r["ratio"]) >= 0.9]
    total_a = len(a.funcs)

    per_module = defaultdict(lambda: [0, 0, 0])
    for r in rows:
        if not r["rva_2012"]:
            continue
        mod = modules.get(int(r["rva_2012"], 16), ("?", ""))[0]
        stats = per_module[mod]
        stats[0] += 1
        if r["rva_2013"]:
            stats[1] += 1
            if float(r["ratio"]) >= 0.9:
                stats[2] += 1

    methods = Counter(r["method"] for r in matched)
    methods_high = Counter(r["method"] for r in high)
    ratio_hist = Counter()
    for r in matched:
        v = float(r["ratio"])
        bucket = "1.0" if v >= 1.0 else f"{int(v * 10) / 10:.1f}"
        ratio_hist[bucket] += 1

    lines = []
    lines.append("# Function matching 2012 (Shipping, PDB) -> 2013 (retail)")
    lines.append("")
    lines.append(f"Produced by `resources/tools/ida/match_functions.py` from `shipping2012_agentJ.i64` ({total_a} functions, image base 0) and "
                 f"`retail2013_agentJ.i64` ({len(b.funcs)} functions after completing the auto-analysis, image base 0x400000). "
                 f"Full table: `match_2012_2013.csv` (rva_2012, rva_2013, name, ratio, method, sizes; rows with an empty side are the unmatched 2012 / new 2013 functions).")
    lines.append("")
    lines.append("## Totals")
    lines.append("")
    lines.append("| | count | % of 2012 |")
    lines.append("|---|---|---|")
    lines.append(f"| 2012 functions | {total_a} | 100 |")
    lines.append(f"| matched | {len(matched)} | {100.0 * len(matched) / total_a:.1f} |")
    lines.append(f"| matched with ratio >= 0.9 (names applied) | {len(high)} | {100.0 * len(high) / total_a:.1f} |")
    lines.append(f"| 2012 functions without 2013 counterpart | {len(unmatched)} | {100.0 * len(unmatched) / total_a:.1f} |")
    lines.append(f"| 2013 functions | {len(b.funcs)} | |")
    lines.append(f"| 2013 functions without 2012 counterpart (new) | {len(new)} | |")
    lines.append(f"| named 2012 globals matched to a 2013 data address | {len(global_rows)} | {sum(1 for r in global_rows if float(r['ratio']) >= 0.9)} with ratio >= 0.9 |")
    if apply_log:
        lines.append("")
        lines.append(apply_log)
    lines.append("")
    lines.append("## Methods")
    lines.append("")
    lines.append("Ratio: 1.0 = identical fixup-masked bytes or identical instruction-token stream; otherwise the difflib similarity of the token streams "
                 "(mnemonic + operand kinds + non-address immediates). Method = evidence that produced the pair.")
    lines.append("")
    lines.append("| method | pairs | ratio >= 0.9 | meaning |")
    lines.append("|---|---|---|---|")
    meanings = {
        "import": "one-instruction `jmp [import]` thunk, unique import name on both sides",
        "bytes": "fixup-masked byte hash unique on both sides, and no sibling with the same shape (displacements/large immediates abstracted) on either side - siblings differing only by a member or vtable offset swap identities when 2013 shifted the layout",
        "block": "link-order alignment of the unmatched functions between two consecutive matched anchors by shape (longest common subsequence), only for shapes with the same count on both sides",
        "bytes-callers": "byte hash shared by several functions (COMDAT-identical bodies), disambiguated through matched callers",
        "string": "a string literal referenced by at most three functions on each side (directly, through the first dwords of a referenced data struct, or by a callee that has no other caller); best pair by token similarity (>= 0.5, margin >= 0.15)",
        "string-set": "identical set of referenced string literals (same extended notion), unique on both sides",
        "callee": "i-th callee of a matched pair (same callee count) or best unmatched callee by similarity",
        "neighbours": "best candidate among the callees of matched callers, callers of matched callees and users of matched globals (score = 0.5 similarity + 0.5 neighbour agreement, margin >= 0.1)",
        "vtable": "aligned slot of two matched function-pointer tables (vtables, GNatives, CRT initializer table); tables matched by membership votes of matched functions, slots aligned by longest common subsequence so inserted virtuals do not break the pairing",
        "tokens": "identical instruction-token stream, unique among the unmatched functions on both sides",
        "order": "link-order window: candidate within 16 positions of the index predicted from the neighbouring matched anchors, scored by similarity + neighbour agreement, ties broken only by a clearly closer position; no other candidate of the same shape in the window",
        "order-sibling": "as `order` but another unmatched candidate with the same shape sits in the window; ratio capped at 0.89 so the name is not applied (validation against the 2013 native table showed ~15 % of these are sibling swaps)",
    }
    for m, c in methods.most_common():
        meaning = meanings.get(m) or (f"`{m[:-1]}` pair whose neighbour agreement ended below 0.5 after all rounds; ratio capped at 0.85, not applied" if m.endswith("?") else "")
        lines.append(f"| {m} | {c} | {methods_high.get(m, 0)} | {meaning} |")
    lines.append("")
    lines.append("Ratio histogram: " + ", ".join(f"{k}: {v}" for k, v in sorted(ratio_hist.items())))
    lines.append("")
    lines.append("## Per module (module of the 2012 function from `functions.csv`)")
    lines.append("")
    lines.append("| module | 2012 functions | matched | matched % | ratio >= 0.9 | unmatched |")
    lines.append("|---|---|---|---|---|---|")
    for mod, (tot, m, h) in sorted(per_module.items(), key=lambda kv: -kv[1][0]):
        lines.append(f"| {mod} | {tot} | {m} | {100.0 * m / tot:.1f} | {h} | {tot - m} |")
    lines.append("")

    lines.append("## Validation and caveats")
    lines.append("")
    lines.append("* Self-test: matching the 2012 dump against itself maps 66,384 of 66,394 functions to their own address; the 10 exceptions are two FaceFX classes with byte-identical bodies and identical call structure (`FxNullLinkFn` / `FxConstantLinkFn`).")
    lines.append("* 2013 native table: the retail exe registers name-bound natives from a static table of `{\"<Class>exec<Func>\", &Class::execFunc}` records (2012 used one dynamic initializer per native, which is why ~1,000 `_dynamic_initializer_for_*exec*` functions of 2012 have no 2013 counterpart). The `apply` step names every exec function from that table and reports how many propagated names agreed with it (see the apply line above): that is an independent accuracy measurement on the hardest population (exec thunks differ from their siblings only by a vtable offset).")
    lines.append("* Layout shifts: 2013 inserted virtuals and members, so functions that differ from a sibling only by a displacement swap identities under pure byte hashing (2012 `execSetRotation` was byte-identical to 2013 `execSetTranslation`). Hence the shape rule for `bytes`/`tokens` and the `order-sibling` cap; ratio < 0.9 pairs are candidates, not names.")
    lines.append("* `GetPrivateStaticClass<Class>` (2,538 per-class functions in 2012, 52 instructions each) has no per-class counterpart in 2013: the retail build keeps a static registration struct per class (size, name, package, within, flags, constructor) and calls one generic function; the 2013 `StaticClassNoInline` references that struct, which is how those clusters are anchored (strings through referenced data structs).")
    lines.append("* Diaphora (cloned into `resources/tools/diaphora`, gitignored) runs headless (`idat -A -S diaphora.py` with `DIAPHORA_AUTO`/`DIAPHORA_EXPORT_FILE`), but its export of the 2012 db progressed ~22 % of `.text` in 28 minutes without the decompiler; with two exports and the diff that is several hours per iteration, so the own matcher (export 1 min per db, match 1.5 min) was used instead.")
    lines.append("* Types are not propagated: `retail2013_named.i64` has names and function comments (`2012 rva ... ratio ... method`) but the 2012 PDB types are not applied.")
    lines.append("")
    lines.append("## Landmark functions")
    lines.append("")
    landmarks = ["FEngineLoop::Init", "ULinkerLoad::CreateLoader", "UClass::Serialize", "FName::StaticInit", "UObject::StaticInit", "UStruct::Link",
                 "UScriptStruct::SerializeBin", "FArchive::operator<<", "appUncompressMemory", "UGameEngine::Init", "UObject::ProcessEvent", "UObject::CallFunction"]
    lines.append("| name | rva_2012 | rva_2013 | ratio | method |")
    lines.append("|---|---|---|---|---|")
    for lm in landmarks:
        hits = [r for r in matched + unmatched if lm in demangled_hint(r["name"])]
        if not hits:
            lines.append(f"| {lm} | - | - | - | not in 2012 functions.csv |")
        for r in hits[:3]:
            lines.append(f"| `{r['name']}` | {r['rva_2012']} | {r['rva_2013'] or '-'} | {r['ratio'] or '-'} | {r['method']} |")
    lines.append("")

    lines.append("## 2012 functions with no 2013 counterpart")
    lines.append("")
    lines.append(f"{len(unmatched)} functions. Per module: " + ", ".join(f"{mod}: {tot - m}" for mod, (tot, m, h) in sorted(per_module.items(), key=lambda kv: -(kv[1][0] - kv[1][1])) if tot - m))
    lines.append("")
    unmatched_sorted = sorted(unmatched, key=lambda r: -int(r["size_2012"]))
    lines.append(f"Largest {min(150, len(unmatched))} (size in bytes, 2012 db):")
    lines.append("")
    lines.append("| rva_2012 | size | module | name |")
    lines.append("|---|---|---|---|")
    for r in unmatched_sorted[:150]:
        mod = modules.get(int(r["rva_2012"], 16), ("?", ""))[0]
        lines.append(f"| {r['rva_2012']} | {r['size_2012']} | {mod} | `{r['name']}` |")
    lines.append("")
    lines.append("Removed functions by name family (count of unmatched 2012 functions whose demangled name starts with the class, top 40):")
    lines.append("")
    families = Counter(name_family(r["name"]) for r in unmatched)
    lines.append(", ".join(f"{k}: {v}" for k, v in families.most_common(40)))
    lines.append("")

    lines.append("## 2013 functions with no 2012 counterpart (new code)")
    lines.append("")
    new_sorted = sorted(new, key=lambda r: -int(r["size_2013"]))
    total_new_bytes = sum(int(r["size_2013"]) for r in new)
    lines.append(f"{len(new)} functions, {total_new_bytes} bytes in total. Categories from the strings they reference (2013 db):")
    lines.append("")
    categories = {"DLC05": 0, "DLC06": 0, "DLC07": 0, "curl": 0, "Steam": 0, "other": 0}
    cat_sizes = Counter()
    tagged = {}
    for r in new:
        fb = b.funcs[int(r["rva_2013"], 16)]
        text = " ".join(fb["strings"]) + " " + " ".join(fb["imports"])
        tag = "other"
        for key in ("DLC05", "DLC06", "DLC07"):
            if key in text:
                tag = key
                break
        else:
            if "curl" in text.lower():
                tag = "curl"
            elif "Steam" in text:
                tag = "Steam"
        categories[tag] += 1
        cat_sizes[tag] += int(r["size_2013"])
        tagged[r["rva_2013"]] = tag
    lines.append("| category | functions | bytes |")
    lines.append("|---|---|---|")
    for k, v in categories.items():
        lines.append(f"| {k} | {v} | {cat_sizes[k]} |")
    lines.append("")
    lines.append(f"Largest {min(150, len(new))}:")
    lines.append("")
    lines.append("| rva_2013 | size | category | 2013 name | first strings |")
    lines.append("|---|---|---|---|---|")
    for r in new_sorted[:150]:
        fb = b.funcs[int(r["rva_2013"], 16)]
        strs = "; ".join(s[2:][:40].replace("|", "/").replace("\n", " ") for s in fb["strings"][:3])
        lines.append(f"| {r['rva_2013']} | {r['size_2013']} | {tagged[r['rva_2013']]} | `{r['name_2013']}` | {strs} |")
    lines.append("")
    dlc = [r for r in new_sorted if tagged[r["rva_2013"]] in ("DLC05", "DLC06", "DLC07")]
    if dlc:
        lines.append(f"All {len(dlc)} new functions referencing DLC05/06/07 strings:")
        lines.append("")
        lines.append("| rva_2013 | size | category | first strings |")
        lines.append("|---|---|---|---|")
        for r in dlc[:300]:
            fb = b.funcs[int(r["rva_2013"], 16)]
            strs = "; ".join(s[2:][:50].replace("|", "/").replace("\n", " ") for s in fb["strings"][:3])
            lines.append(f"| {r['rva_2013']} | {r['size_2013']} | {tagged[r['rva_2013']]} | {strs} |")
        lines.append("")
    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")
    log(f"wrote {out_md}")


def demangled_hint(mangled: str) -> str:
    if not mangled.startswith("?"):
        return mangled
    parts = mangled[1:].split("@@", 1)[0].split("@")
    return "::".join(reversed([p for p in parts if p]))


def name_family(mangled: str) -> str:
    hint = demangled_hint(mangled)
    return hint.split("::")[0] if "::" in hint else hint[:24]


# ----------------------------------------------------------------------------------------------

def main(argv: list[str]) -> int:
    if not argv:
        print(__doc__)
        return 2
    cmd = argv[0]
    if cmd == "analyze":
        ida_analyze()
    elif cmd == "export":
        ida_export(Path(argv[1]))
    elif cmd == "apply":
        ida_apply(Path(argv[1]), float(argv[2]) if len(argv) > 2 else 0.9)
    elif cmd == "match":
        run_match(Path(argv[1]), Path(argv[2]), Path(argv[3]))
    elif cmd == "report":
        run_report(Path(argv[1]), Path(argv[2]), Path(argv[3]), Path(argv[4]), argv[5] if len(argv) > 5 else "")
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
