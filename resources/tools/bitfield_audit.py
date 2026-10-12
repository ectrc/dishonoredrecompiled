"""bitfield_audit.py - every native class and script struct whose C++ bitfield is declared in an order other
than the COOKED class's.

Why: UStruct::Link (Core/Src/UnClass.cpp, `SavedBitMask`) keeps the bit masks the cooked script class was
serialized with, so a C++ `BITFIELD bX:1` run declared in another order (the reference engine's, say) reads the
wrong bits silently: UInterpTrack's `bDisableTrack` read retail's `bAIGroupOnly` for twenty waves (agent FS,
wave 21). This tool compares every bitfield run declared in the tree against the masks of the retail SDK dump
(resources/docs/types/retail_sdk_layout.json) and, for the classes the dump does not have, against the property
order of the cooked package (script_classes_2013.json: consecutive BoolProperty children share one DWORD).

    python resources/tools/bitfield_audit.py                 report, exit 0 when mismatches == 0
    python resources/tools/bitfield_audit.py --verbose       also list the units that match and the native-only runs
    python resources/tools/bitfield_audit.py --fix           rewrite the mismatching runs in the cooked order (review the diff)
    python resources/tools/bitfield_audit.py --json out.json

The last line is the summary verify_phase2.py reads:
    bitfield_audit: classes=<n> units=<n> mismatches=<n> read=<n> missing=<n> allowed=<n>
`mismatches` are DWORD units with at least one reflected bool at the wrong bit; `read` are those whose wrong bits
are named in a compiled .cpp of the tree (static: referenced, not proven reached); `missing` are units that line
up but lack a trailing retail bool (harmless, informational); `allowed` are listed with a reason in
resources/docs/types/bitfield_audit_allow.txt (`Class.member reason...`, one per line).

Rules of the comparison (MSVC packing): adjacent bit fields of 4-byte integral types share one allocation unit
until 32 bits are used; access labels, blank lines, comments and preprocessor lines do not break a run; any other
declaration does. A C++ bit the dump has no property for (a native-only bool) is a gap retail's masks skip, so it
is accepted at any bit the cooked masks leave free.
"""
import argparse
import csv
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TYPES = REPO / "resources" / "docs" / "types"
SRC = REPO / "source" / "Development" / "Src"
ALLOW = TYPES / "bitfield_audit_allow.txt"

BIT_RE = re.compile(r"^\s*(?:mutable\s+)?(BITFIELD|UBOOL|DWORD|INT|UINT|unsigned(?:\s+(?:int|long))?|int|long|bool)\s+(\w+)\s*:\s*(\d+)\s*;")
LABEL_RE = re.compile(r"^\s*(public|private|protected)\s*:\s*$")
TYPE_RE = re.compile(r"\b(class|struct)\s+(?:[A-Z_]+\s+)?([UAFI][A-Z]\w*)\s*(?::[^{;]*)?\{")
WORD_TYPES_4 = {"BITFIELD", "UBOOL", "DWORD", "INT", "UINT", "unsigned", "unsigned int", "unsigned long", "int", "long", "bool"}


def strip_comments(text: str) -> str:
    """comments become spaces of the same line count so line numbers survive"""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


class Unit:
    """one allocation unit (DWORD) of a declared bitfield run"""
    def __init__(self, header: Path, cls: str, first_line: int):
        self.header = header
        self.cls = cls
        self.first_line = first_line
        self.names: list[str] = []      # declared order, bit i = names[i]
        self.lines: list[int] = []      # 1-based line of each member
        self.width = 0

    def __repr__(self) -> str:
        return f"{self.cls}@{self.header.name}:{self.first_line}"


def declared_units(header: Path) -> list[Unit]:
    raw = header.read_text(encoding="utf-8", errors="replace")
    text = strip_comments(raw)
    lines = text.split("\n")
    units: list[Unit] = []
    for m in TYPE_RE.finditer(text):
        cls = m.group(2)
        depth, i = 1, m.end()
        while i < len(text) and depth:
            depth += {"{": 1, "}": -1}.get(text[i], 0)
            i += 1
        body_start = text.count("\n", 0, m.end())      # 0-based line of the opening brace
        body_end = text.count("\n", 0, i)
        unit: Unit | None = None
        inner = 0
        for ln in range(body_start + 1, body_end):
            line = lines[ln]
            if inner > 0:
                inner += line.count("{") - line.count("}")
                continue
            stripped = line.strip()
            if not stripped or stripped.startswith("#") or LABEL_RE.match(line):
                continue
            bm = BIT_RE.match(line)
            if bm:
                width = int(bm.group(3))
                if unit is None or unit.width + width > 32:
                    unit = Unit(header, cls, ln + 1)
                    units.append(unit)
                unit.names.append(bm.group(2))
                unit.lines.append(ln + 1)
                unit.width += width
                continue
            unit = None
            inner = line.count("{") - line.count("}")
            if inner < 0:
                inner = 0
    return units


def cooked_groups() -> tuple[dict, dict, dict]:
    """cpp name -> list of cooked DWORD groups [(offset or None, [(name, mask)])]; plus the source of each"""
    sdk = json.loads((TYPES / "retail_sdk_layout.json").read_text(encoding="utf-8"))
    groups: dict[str, list] = {}
    source: dict[str, str] = {}
    for table in ("classes", "structs"):
        for cpp, e in sdk[table].items():
            by_off: dict[int, list] = defaultdict(list)
            for m in e.get("members", []):
                if m.get("bitfield"):
                    by_off[m["offset"]].append((m["name"], m.get("mask", 0)))
            if by_off:
                groups[cpp] = [(off, sorted(v, key=lambda x: x[1])) for off, v in sorted(by_off.items())]
                source[cpp] = "sdk"
    script = json.loads((TYPES / "script_classes_2013.json").read_text(encoding="utf-8"))["classes"]
    sizes = {}
    with (TYPES / "native_class_sizes.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            sizes[r["class"]] = r
    stem_to_cpp = {}
    for cpp in sizes:
        stem = cpp[1:]
        stem_to_cpp[stem] = cpp
    for cpp in sdk["classes"]:
        stem_to_cpp.setdefault(cpp[1:], cpp)

    def runs_of(children: list) -> list:
        out, cur = [], []
        for ch in children:
            kind = ch.get("kind", "")
            if kind == "BoolProperty":
                for _ in range(ch.get("array_dim", 1) or 1):
                    cur.append((ch["name"], 1 << len(cur)))
                    if len(cur) == 32:
                        out.append((None, cur))
                        cur = []
            elif kind.endswith("Property"):
                if cur:
                    out.append((None, cur))
                cur = []
        if cur:
            out.append((None, cur))
        return out

    for sname, sc in script.items():
        cpp = stem_to_cpp.get(sname)
        if cpp is None:
            continue
        if cpp not in groups:
            r = runs_of(sc.get("children", []))
            if r:
                groups[cpp] = r
                source[cpp] = "script order"
        for ch in sc.get("children", []):
            if ch.get("kind") == "ScriptStruct":
                scpp = "F" + ch["name"]
                if scpp not in groups:
                    r = runs_of(ch.get("children", []))
                    if r:
                        groups[scpp] = r
                        source[scpp] = "script order"
    return groups, source, sizes


def compiled_sources() -> list[Path]:
    """every .cpp of the tree that is not on its module's Sources.cmake exclude list"""
    out = []
    for mod in SRC.iterdir():
        if not (mod / "Src").is_dir():
            continue
        excluded = set()
        cm = mod / "Sources.cmake"
        if cm.exists():
            excluded = {Path(x).name.lower() for x in re.findall(r"Src/([\w./-]+\.cpp)", cm.read_text(encoding="utf-8", errors="replace"))}
        for p in (mod / "Src").rglob("*.cpp"):
            if p.name.lower() not in excluded:
                out.append(p)
    return out


def reference_count(names: set[str], sources: list[Path], headers: list[Path]) -> dict[str, int]:
    counts = {n: 0 for n in names}
    if not names:
        return counts
    rx = re.compile(r"\b(" + "|".join(re.escape(n) for n in sorted(names)) + r")\b")
    for p in sources + headers:
        text = strip_comments(p.read_text(encoding="utf-8", errors="replace"))
        for m in rx.finditer(text):
            line_start = text.rfind("\n", 0, m.start()) + 1
            line_end = text.find("\n", m.end())
            line = text[line_start:line_end if line_end >= 0 else len(text)]
            if BIT_RE.match(line):
                continue  # the declaration itself
            counts[m.group(1)] += 1
    return counts


def load_allow() -> tuple[dict[str, str], dict[str, str]]:
    """`Class.member reason` or `Class.* reason` -> allowed; `Class.cppname=cookedname` -> the same bit under two names"""
    allow: dict[str, str] = {}
    renames: dict[str, str] = {}
    if ALLOW.exists():
        for l in ALLOW.read_text(encoding="utf-8").splitlines():
            l = l.strip()
            if not l or l.startswith("#"):
                continue
            key, _, why = l.partition(" ")
            if "=" in key:
                cpp, _, cooked = key.partition("=")
                renames[cpp] = cooked
            else:
                allow[key] = why.strip()
    return allow, renames


def allowed_reason(allow: dict[str, str], cls: str, name: str) -> str | None:
    return allow.get(f"{cls}.{name}") or allow.get(f"{cls}.*")


def bit_of(mask: int) -> int:
    return mask.bit_length() - 1


DEPRECATED_RE = re.compile(r"(_DEPRECATED|Deprecated)$")


def norm(name: str) -> str:
    """the reference engine renames a bool it stops using to X_DEPRECATED / XDeprecated and the cooked class keeps
    the old name (or the other way round): the same bit under two spellings is not an order fault"""
    return DEPRECATED_RE.sub("", name)


def compare(unit: Unit, groups: list, renames: dict[str, str]) -> dict | None:
    """pick the cooked DWORD this unit declares (most names in common) and compare bit positions"""
    def key(name: str) -> str:
        return norm(renames.get(f"{unit.cls}.{name}", name))
    declared = [key(n) for n in unit.names]
    best, overlap = None, 0
    for off, members in groups:
        n = len(set(declared) & {norm(name) for name, _ in members})
        if n > overlap:
            best, overlap = (off, members), n
    if best is None:
        return None
    off, members = best
    cooked = {norm(name): bit_of(mask) for name, mask in members}
    wrong = [(name, i, cooked[k]) for i, (name, k) in enumerate(zip(unit.names, declared)) if k in cooked and cooked[k] != i]
    missing = [name for name, _ in members if norm(name) not in declared]
    native_only = [name for name, k in zip(unit.names, declared) if k not in cooked]
    # a native-only C++ bit is fine where the cooked masks leave a hole; at a bit a cooked bool owns it is a clash
    taken = set(cooked.values())
    clash = [(name, i) for i, (name, k) in enumerate(zip(unit.names, declared)) if k not in cooked and i in taken]
    return {"offset": off, "cooked": [name for name, _ in members], "wrong": wrong, "missing": missing,
            "native_only": native_only, "clash": clash}


def fix_unit(unit: Unit, result: dict, edits: dict) -> str | None:
    """reorder the unit's member lines into the cooked order; returns a reason when it cannot"""
    raw_lines = edits.setdefault(unit.header, unit.header.read_text(encoding="utf-8", errors="replace").split("\n"))
    cooked = result["cooked"]
    members = {name: raw_lines[ln - 1] for name, ln in zip(unit.names, unit.lines)}
    natives = [n for n in unit.names if n not in cooked]
    masks = {name: i for i, name in enumerate(cooked)}
    slots: list[str | None] = [None] * 32
    for name in unit.names:
        if name in masks:
            slots[masks[name]] = name
    # the cooked bools this C++ does not declare keep their bit as an UnknownBit placeholder, so nothing after them shifts
    for i, name in enumerate(cooked):
        if slots[i] is None:
            slots[i] = f"__cooked__{name}"
    free = [i for i, s in enumerate(slots) if s is None]
    if len(natives) > len(free):
        return f"{len(natives)} native-only bits and only {len(free)} free bits in the cooked DWORD"
    for name, i in zip(natives, free):
        slots[i] = name
    last = max(i for i, s in enumerate(slots) if s is not None)
    indent = re.match(r"^\s*", members[unit.names[0]]).group(0)
    new_lines: list[str] = []
    for i in range(last + 1):
        s = slots[i]
        if s is None:
            new_lines.append(f"{indent}BITFIELD UnknownBit_{i:02d}:1;  // DISHONORED(bitfield): cooked bit 0x{1 << i:x} has no property in the SDK dump")
        elif s.startswith("__cooked__"):
            name = s[len("__cooked__"):]
            new_lines.append(f"{indent}BITFIELD {name}:1;  // DISHONORED(bitfield): cooked bit 0x{1 << i:x}, not declared before (bitfield_audit.py)")
        else:
            new_lines.append(members[s].rstrip("\r"))
    # splice: the unit's member lines are replaced in place; lines between them (labels, comments) are kept before the run
    first, lastln = unit.lines[0], unit.lines[-1]
    between = [raw_lines[ln - 1] for ln in range(first, lastln + 1) if ln not in unit.lines]
    kept = [l for l in between if l.strip() and not LABEL_RE.match(l)]
    crlf = raw_lines[first - 1].endswith("\r")
    block = kept + new_lines
    raw_lines[first - 1:lastln] = [l + ("\r" if crlf and not l.endswith("\r") else "") for l in block]
    # the lines after the run have moved: re-base every other unit of this header
    return None


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--fix", action="store_true")
    ap.add_argument("--json", default=None)
    args = ap.parse_args(argv[1:])
    groups, source, sizes = cooked_groups()
    headers = [p for mod in SRC.iterdir() for sub in ("Inc", "Src") if (mod / sub).is_dir() for p in (mod / sub).rglob("*.h")]
    units: list[Unit] = []
    for h in headers:
        units += declared_units(h)
    allow, renames = load_allow()
    results = []
    n_classes = set()
    n_units = 0
    for u in units:
        if u.cls not in groups:
            if args.verbose and u.cls in sizes:
                print(f"native-only  {u!r}: {', '.join(u.names)} (no reflected bool in the cooked class)")
            continue
        r = compare(u, groups[u.cls], renames)
        if r is None:
            if args.verbose:
                print(f"native-only  {u!r}: {', '.join(u.names)} (no name in common with a cooked DWORD)")
            continue
        n_units += 1
        n_classes.add(u.cls)
        r["unit"] = u
        r["source"] = source[u.cls]
        results.append(r)
    wrong_names = {name for r in results for name, _, _ in r["wrong"]} | {name for r in results for name, _ in r["clash"]}
    refs = reference_count(wrong_names, compiled_sources(), [h for h in headers if "CppText" in h.parts]) if wrong_names else {}
    mismatches = read = missing = allowed = 0
    fixed, unfixed = [], []
    edits: dict[Path, list[str]] = {}
    # units are fixed bottom-up per header so earlier line numbers stay valid
    for r in sorted(results, key=lambda r: (str(r["unit"].header), -r["unit"].first_line)):
        u = r["unit"]
        bad = r["wrong"] or r["clash"]
        reasons = [(name, allowed_reason(allow, u.cls, name)) for name, *_ in r["wrong"] + r["clash"]]
        if bad and all(why for _, why in reasons):
            allowed += 1
            print(f"allowed      {u!r}: {', '.join(name for name, _ in reasons)}: {reasons[0][1]}")
            continue
        if bad:
            mismatches += 1
            is_read = any(refs.get(name, 0) for name, *_ in r["wrong"] + r["clash"])
            read += is_read
            rel = u.header.relative_to(REPO)
            print(f"MISMATCH     {u.cls}  {rel}:{u.first_line}  (cooked order from the {r['source']}{', offset %d' % r['offset'] if r['offset'] is not None else ''})")
            print(f"    declared: {', '.join(u.names)}")
            print(f"    cooked:   {', '.join(r['cooked'])}")
            for name, got, want in r["wrong"]:
                print(f"    {name}: declared bit {got} (0x{1 << got:x}), cooked bit {want} (0x{1 << want:x}); references in compiled code: {refs.get(name, 0)}")
            for name, got in r["clash"]:
                print(f"    {name}: native-only C++ bit at {got} (0x{1 << got:x}) where the cooked class has {r['cooked'][got]}; references: {refs.get(name, 0)}")
            if r["missing"]:
                print(f"    not declared in C++: {', '.join(r['missing'])}")
            if args.fix:
                why = fix_unit(u, r, edits)
                (unfixed if why else fixed).append((u, why))
                if why:
                    print(f"    NOT FIXED: {why}")
        elif r["missing"]:
            missing += 1
            if args.verbose:
                print(f"missing      {u!r}: cooked bools not declared in C++: {', '.join(r['missing'])} (declared ones line up)")
        elif args.verbose:
            print(f"ok           {u!r}: {len(u.names)} bits")
    if args.fix:
        for h, lines in edits.items():
            h.write_text("\n".join(lines), encoding="utf-8", newline="")
            print(f"fixed: {h.relative_to(REPO)}")
    if args.json:
        Path(args.json).write_text(json.dumps([{"class": r["unit"].cls, "header": str(r["unit"].header.relative_to(REPO)), "line": r["unit"].first_line,
                                                "declared": r["unit"].names, "cooked": r["cooked"], "wrong": r["wrong"], "clash": r["clash"],
                                                "missing": r["missing"], "source": r["source"]} for r in results if r["wrong"] or r["clash"] or r["missing"]], indent=1), encoding="utf-8")
    print(f"bitfield_audit: classes={len(n_classes)} units={n_units} mismatches={mismatches} read={read} missing={missing} allowed={allowed}")
    return 1 if mismatches else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
