"""agent ES: inventory the *_EXCLUDE skeleton units and say, per unit, whether retail 2013 still needs it.

`source/Development/Src/<Module>/Sources.cmake` carries `set(<Module>_EXCLUDE ...)`: an exclude list of .cpp
units the module target does NOT compile. Almost all of them are the comment-only skeletons of
resources/tools/import_reference.py -- a banner naming the 2012 PDB functions attributed to that file and no
code at all. Nothing in the tree says which of those skeletons are still worth porting, so a port wave has no
way to tell a real hole in the game from a unit whose class retail 2013 does not even have.

This tool answers that per unit. It reads the owner types out of each banner's demangled symbols, then asks the
truth sources (resources/docs) whether those types and those functions exist in the retail 2013 image.

THE RVAs IN THE BANNERS ARE 2012 ADDRESSES. They come from the 2012 shipping PDB; the target of this project is
retail 2013 (engine 9411, CL 1274963). This tool therefore never reports a banner rva as a 2013 address: the
only 2013 addresses it emits are registrant_rva_2013 / staticclass_rva_2013, read out of the 2013 binary, and
the 2013 match counts, which are counts and not addresses.

Verdicts, one per unit:

  retail-has-class    an owner type is confirmed present in 2013 by a binary-backed source (a non-empty
                      registrant_rva_2013 or staticclass_rva_2013 in native_class_sizes.csv) or by the cooked
                      2013 packages (script_classes_2013.json). A skeleton waiting for a port: keep it.
  retail-has-code     no owner is class-confirmed -- a non-UObject F* class, or free functions -- but at least
                      one banner function has a strong 2012->2013 match, so the code is in the 2013 image.
                      Also a legitimate skeleton.
  2013-unconfirmed    no class confirmation and no strong match, but at least one weak match. Hand work; this
                      is NOT a statement that the unit is absent from retail.
  retail-lacks        no class confirmation, no strong match, no weak match, and every banner function is
                      unmatched. Nothing in the unit resolves to 2013, so it is a CANDIDATE for deletion --
                      a candidate, not a proof: a unit carrying no banner at all lands here too, and its
                      evidence column says exactly that.
  not-a-skeleton      the unit has real code and is on the exclude list anyway: real code that never compiles.
                      A latent bug; these are listed in full in the summary.
  missing             on the exclude list but not on disk.

Match strength (symbols/match_2012_2013.csv has been wrong before -- a 27-byte InternalConstructor matched many
candidates and the bad match propagated through the global-table / global-votes votes):

  strong   method in (bytes, string, import, vtable), ratio >= 0.95, and size_2012 >= 32
  weak     any other row with a non-empty rva_2013 -- the vote/order/neighbour/callee methods, anything with a
           trailing `?`, and every match on a function under 32 bytes whatever the method
  none     no row for that 2012 rva, or a row whose rva_2013 is empty (method `unmatched`)

banner_count can exceed parsed_count: import_reference.py truncates a banner's symbol list at 200 entries with a
`... N more` line, and the ported units rewrote some banners by hand. `unlisted` is that difference and those
functions are counted nowhere, so match_strong + match_weak + match_none == parsed_count always holds.

Usage:
  python resources/tools/exclude_inventory.py [--repo DIR] [--truth DIR] [--out FILE]

Writes resources/docs/agents/agentES_exclude_inventory.csv, one row per exclude-listed unit, and prints the
per-module counts, the verdict histogram and every not-a-skeleton / missing unit.
"""
import argparse
import csv
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
UPSTREAM_TRUTH = Path("D:/RecompileDishonored/Recompile/resources/docs")

BANNER_COUNT_RE = re.compile(r"PDB functions attributed to this file \((\d+)")
SYMBOL_RE = re.compile(r"^//\s*(0x[0-9a-fA-F]+)\s+(\S.*)$")
CONV_RE = re.compile(r"__(?:thiscall|cdecl|stdcall|fastcall|clrcall|vectorcall)\b")
ACCESS_RE = re.compile(r"^(?:public|private|protected):\s*")
LEADING_KW_RE = re.compile(r"^(?:static|virtual|extern|inline)\s+")
DYNAMIC_INIT_RE = re.compile(r"^_dynamic_(?:initializer|atexit_destructor)_for_+")
ANON_NAMESPACE_RE = re.compile(r"`anonymous namespace'::")
OPERATOR_RE = re.compile(r"^operator\b")
NAME_RE = re.compile(r"[A-Za-z_~][\w~]*(?:::[A-Za-z_~][\w~]*)*")
STRONG_METHODS = ("bytes", "string", "import", "vtable")
STRONG_RATIO = 0.95
STRONG_MIN_SIZE = 32
VERDICT_ORDER = ["retail-has-class", "retail-has-code", "2013-unconfirmed", "retail-lacks",
                 "not-a-skeleton", "missing"]


def truth_file(truth_dir: Path, rel: str):
    local = truth_dir / rel
    if local.exists():
        return local
    upstream = UPSTREAM_TRUTH / rel
    if upstream.exists():
        print("truth fallback: %s not present, using %s" % (local, upstream), file=sys.stderr)
        return upstream
    raise SystemExit("missing truth source %s (neither %s nor %s)" % (rel, truth_dir, UPSTREAM_TRUTH))


def exclude_units(cmake: Path, module: str):
    """The units named inside set(<module>_EXCLUDE ... ), in file order, # comments dropped."""
    target = "set(%s_EXCLUDE" % module
    units, inside, depth = [], False, 0
    for line in cmake.read_text(encoding="utf-8", errors="replace").splitlines():
        code = line.split("#")[0]
        if not inside:
            if target not in code:
                continue
            inside = True
            code = code[code.index(target) + len(target):]
        for tok in re.findall(r"\(|\)|[^\s()]+", code):
            if tok == "(":
                depth += 1
            elif tok == ")":
                if depth == 0:
                    inside = False
                    break
                depth -= 1
            elif tok.endswith(".cpp"):
                units.append(tok)
        if not inside:
            break
    return units


def conditional_edits(cmake: Path, module: str):
    """Units an if() branch adds to or removes from _EXCLUDE: reported, never inventoried."""
    text = cmake.read_text(encoding="utf-8", errors="replace")
    out = []
    for m in re.finditer(r"list\s*\(\s*(APPEND|REMOVE_ITEM)\s+%s_EXCLUDE(.*?)\)" % module, text, re.S):
        body = "\n".join(l.split("#")[0] for l in m.group(2).splitlines())
        for tok in re.findall(r"\S+\.cpp", body):
            out.append((m.group(1), tok))
    return out


def code_lines_of(path: Path):
    """Non-blank lines that survive stripping // and /* */ comments. 0 for a true comment-only skeleton."""
    count, in_block = 0, False
    for raw in path.read_bytes().decode("utf-8", "replace").splitlines():
        line, kept = raw, []
        while line:
            if in_block:
                end = line.find("*/")
                if end < 0:
                    line = ""
                    break
                line, in_block = line[end + 2:], False
                continue
            slash, block = line.find("//"), line.find("/*")
            if block >= 0 and (slash < 0 or block < slash):
                kept.append(line[:block])
                line, in_block = line[block + 2:], True
                continue
            if slash >= 0:
                kept.append(line[:slash])
                break
            kept.append(line)
            break
        if "".join(kept).strip():
            count += 1
    return count


def strip_templates(text: str):
    kept, depth = [], 0
    for ch in text:
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth = max(0, depth - 1)
        elif depth == 0:
            kept.append(ch)
    return "".join(kept)


def name_before_params(text: str):
    depth = 0
    for i, ch in enumerate(text):
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth = max(0, depth - 1)
        elif ch == "(" and depth == 0:
            if text[:i].rstrip().endswith("operator"):
                continue
            return text[:i].strip()
    return text.strip()


def qualified_name(sym: str):
    """The demangled symbol's qualified name, with return type, parameters and template arguments removed.

    The candidates are the text after each calling-convention keyword, longest-reach first, then the whole
    symbol: a function returning a pointer-to-member puts a __thiscall in its return type, so the first
    candidate that still carries * or & or ( is rejected and the next one tried. An operator name is cut back
    to the `operator` token (its type or punctuation is never part of the owner) and an anonymous-namespace
    qualifier is dropped, which makes those functions free functions, which is what they are."""
    sym = ACCESS_RE.sub("", sym.strip())
    while LEADING_KW_RE.match(sym):
        sym = LEADING_KW_RE.sub("", sym)
    candidates = [sym[m.end():].strip() for m in CONV_RE.finditer(sym)] + [sym]
    for cand in candidates:
        cand = ANON_NAMESPACE_RE.sub("", cand).strip()
        at_operator = cand.find("::operator")
        if at_operator >= 0:
            cand = cand[:at_operator] + "::operator"
        elif OPERATOR_RE.match(cand):
            cand = "operator"
        name = strip_templates(name_before_params(cand)).strip()
        if not name or "*" in name or "&" in name or "(" in name:
            continue
        if NAME_RE.fullmatch(name):
            return name
    return ""


def owner_of(sym: str):
    """The X:: qualifier of a demangled symbol, or <free> for a free function."""
    name = qualified_name(sym)
    if not name:
        return ""
    parts = [p for p in name.split("::") if p]
    if len(parts) < 2:
        return "<free>"
    return DYNAMIC_INIT_RE.sub("", parts[-2]).strip("_ ") or "<free>"


def read_banner(path: Path):
    text = path.read_bytes().decode("utf-8", "replace")
    m = BANNER_COUNT_RE.search(text)
    banner_count = int(m.group(1)) if m else 0
    rvas, owners = [], Counter()
    for raw in text.splitlines():
        line = raw.strip()
        if not line.startswith("//"):
            continue
        m = SYMBOL_RE.match(line)
        if not m:
            continue
        owner = owner_of(m.group(2))
        if not owner:
            continue
        rvas.append(int(m.group(1), 16))
        owners[owner] += 1
    return banner_count, rvas, owners


def load_match_strength(path: Path):
    """2012 rva -> strong / weak / none, per the rule in this module's docstring."""
    out = {}
    with path.open(newline="", encoding="utf-8", errors="replace") as f:
        for r in csv.DictReader(f):
            rva = (r.get("rva_2012") or "").strip()
            if not rva.startswith("0x"):
                continue
            key = int(rva, 16)
            if key in out:
                continue
            rva_2013 = (r.get("rva_2013") or "").strip()
            matched = bool(rva_2013) and rva_2013 not in ("0", "0x0")
            try:
                ratio = float(r.get("ratio") or 0)
            except ValueError:
                ratio = 0.0
            try:
                size_2012 = int(r.get("size_2012") or 0)
            except ValueError:
                size_2012 = 0
            method = (r.get("method") or "").strip()
            strong = (matched and method in STRONG_METHODS and ratio >= STRONG_RATIO
                      and size_2012 >= STRONG_MIN_SIZE)
            out[key] = "strong" if strong else ("weak" if matched else "none")
    return out


def load_native_sizes(path: Path):
    with path.open(newline="", encoding="utf-8", errors="replace") as f:
        return {r["class"].strip(): r for r in csv.DictReader(f)}


def script_name_of(owner: str):
    """The script-class name for a C++ UObject name, or None when the name is not U/A + uppercase."""
    if len(owner) >= 2 and owner[0] in "UA" and owner[1].isupper():
        return owner[1:]
    return None


class OwnerVerdict:
    def __init__(self, owner, script, sdk_classes, sdk_structs, native):
        script_name = script_name_of(owner)
        row = native.get(owner) or {}
        self.owner = owner
        self.script_2013 = "n-a" if script_name is None else ("yes" if script_name in script else "no")
        self.sdk_2013 = "yes" if (owner in sdk_classes or owner in sdk_structs) else "no"
        self.size_2013 = (row.get("size_2013") or "").strip()
        self.registrant_rva_2013 = (row.get("registrant_rva_2013") or "").strip()
        self.staticclass_rva_2013 = (row.get("staticclass_rva_2013") or "").strip()
        self.binary_confirmed = bool(self.registrant_rva_2013 or self.staticclass_rva_2013)
        self.confirmed = self.binary_confirmed or self.script_2013 == "yes"

    def why(self):
        reasons = []
        if self.registrant_rva_2013:
            reasons.append("registrant_rva_2013=" + self.registrant_rva_2013)
        if self.staticclass_rva_2013:
            reasons.append("staticclass_rva_2013=" + self.staticclass_rva_2013)
        if self.script_2013 == "yes":
            reasons.append("script_classes_2013")
        if self.sdk_2013 == "yes":
            reasons.append("retail_sdk_layout")
        return "%s: %s" % (self.owner, " + ".join(reasons) or "nothing")

    def columns(self):
        return {"script_2013": self.script_2013, "sdk_2013": self.sdk_2013, "size_2013": self.size_2013,
                "registrant_rva_2013": self.registrant_rva_2013,
                "staticclass_rva_2013": self.staticclass_rva_2013}


def inventory_unit(module, unit, path, header, script, sdk_classes, sdk_structs, native, strength_of):
    row = {"module": module, "unit": unit, "exists": int(path.exists()),
           "code_lines": "", "banner_count": "", "parsed_count": 0, "unlisted": 0,
           "header": "", "header_decl_lines": "", "header_shim": "",
           "primary_owner": "", "primary_owner_symbols": 0, "owner_count": 0, "owners": "",
           "script_2013": "", "sdk_2013": "", "size_2013": "",
           "registrant_rva_2013": "", "staticclass_rva_2013": "",
           "match_strong": 0, "match_weak": 0, "match_none": 0,
           "verdict": "missing", "evidence": "on the exclude list but not on disk"}
    if not path.exists():
        return row

    row["code_lines"] = code_lines_of(path)
    banner_count, rvas, owners = read_banner(path)
    row["banner_count"] = banner_count
    row["parsed_count"] = len(rvas)
    row["unlisted"] = max(0, banner_count - len(rvas))
    row["header"] = int(header.exists())
    if header.exists():
        row["header_decl_lines"] = code_lines_of(header)
        row["header_shim"] = header.read_bytes().decode("utf-8", "replace").count("DISHONORED_SHIM_STATIC")

    strength = Counter(strength_of.get(rva, "none") for rva in rvas)
    row["match_strong"] = strength["strong"]
    row["match_weak"] = strength["weak"]
    row["match_none"] = strength["none"]

    verdicts = [OwnerVerdict(o, script, sdk_classes, sdk_structs, native) for o, _ in owners.most_common()]
    row["owner_count"] = len(verdicts)
    row["owners"] = " ".join("%s:%d" % (o, n) for o, n in owners.most_common())
    if verdicts:
        row["primary_owner"] = verdicts[0].owner
        row["primary_owner_symbols"] = owners.most_common(1)[0][1]
        row.update(verdicts[0].columns())
    confirmed = [v for v in verdicts if v.confirmed]

    if row["code_lines"] > 0:
        row["verdict"] = "not-a-skeleton"
        row["evidence"] = "%d code lines in a unit the build excludes" % row["code_lines"]
    elif confirmed:
        row["verdict"] = "retail-has-class"
        row["evidence"] = "; ".join(v.why() for v in confirmed[:4])
    elif row["match_strong"]:
        row["verdict"] = "retail-has-code"
        row["evidence"] = "no owner class-confirmed (%s); %d of %d banner functions strongly matched into 2013" % (
            row["primary_owner"] or "no owner", row["match_strong"], banner_count)
    elif row["match_weak"]:
        row["verdict"] = "2013-unconfirmed"
        row["evidence"] = "no class confirmation and no strong match; %d of %d banner functions weakly matched" % (
            row["match_weak"], banner_count)
    elif banner_count == 0:
        row["verdict"] = "retail-lacks"
        row["evidence"] = "no banner: no PDB function is attributed to this unit, so nothing resolves"
    else:
        row["verdict"] = "retail-lacks"
        row["evidence"] = "owner(s) %s in no 2013 source and all %d listed banner functions unmatched" % (
            row["owners"] or "none", row["parsed_count"])
    return row


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--repo", default=str(REPO), help="tree to inventory (default: the tree this script lives in)")
    ap.add_argument("--truth", default=None, help="truth-source directory (default: <repo>/resources/docs)")
    ap.add_argument("--out", default=None, help="output csv (default: <repo>/resources/docs/agents/agentES_exclude_inventory.csv)")
    args = ap.parse_args()

    repo = Path(args.repo).resolve()
    src = repo / "source" / "Development" / "Src"
    truth = Path(args.truth).resolve() if args.truth else repo / "resources" / "docs"
    out_path = Path(args.out).resolve() if args.out else \
        repo / "resources" / "docs" / "agents" / "agentES_exclude_inventory.csv"

    script = json.loads(truth_file(truth, "types/script_classes_2013.json").read_text(encoding="utf-8"))["classes"]
    sdk = json.loads(truth_file(truth, "types/retail_sdk_layout.json").read_text(encoding="utf-8"))
    sdk_classes, sdk_structs = sdk["classes"], sdk["structs"]
    native = load_native_sizes(truth_file(truth, "types/native_class_sizes.csv"))
    strength_of = load_match_strength(truth_file(truth, "symbols/match_2012_2013.csv"))

    rows, conditionals = [], []
    for cmake in sorted(src.glob("*/Sources.cmake")):
        module = cmake.parent.name
        for kind, tok in conditional_edits(cmake, module):
            conditionals.append((module, kind, tok))
        for unit in exclude_units(cmake, module):
            path = cmake.parent / unit
            header = cmake.parent / "Inc" / (Path(unit).stem + ".h")
            rows.append(inventory_unit(module, unit, path, header, script, sdk_classes, sdk_structs,
                                       native, strength_of))

    cols = ["module", "unit", "verdict", "exists", "code_lines", "banner_count", "parsed_count", "unlisted",
            "header", "header_decl_lines", "header_shim",
            "primary_owner", "primary_owner_symbols", "owner_count", "owners",
            "script_2013", "sdk_2013", "size_2013", "registrant_rva_2013", "staticclass_rva_2013",
            "match_strong", "match_weak", "match_none", "evidence"]
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols)
        w.writeheader()
        for r in rows:
            w.writerow({c: r.get(c, "") for c in cols})

    per_module = defaultdict(Counter)
    for r in rows:
        per_module[r["module"]][r["verdict"]] += 1
    verdicts = Counter(r["verdict"] for r in rows)

    print("exclude inventory: %d units across %d modules" % (len(rows), len(per_module)))
    print("")
    print("%-28s %5s  %s" % ("module", "units", " ".join("%-17s" % v for v in VERDICT_ORDER)))
    for module in sorted(per_module, key=lambda m: -sum(per_module[m].values())):
        c = per_module[module]
        print("%-28s %5d  %s" % (module, sum(c.values()), " ".join("%-17d" % c[v] for v in VERDICT_ORDER)))
    print("%-28s %5d  %s" % ("TOTAL", len(rows), " ".join("%-17d" % verdicts[v] for v in VERDICT_ORDER)))

    print("")
    print("verdict histogram:")
    for v in VERDICT_ORDER:
        print("  %-18s %5d" % (v, verdicts[v]))

    for label in ("not-a-skeleton", "missing"):
        hits = [r for r in rows if r["verdict"] == label]
        print("")
        print("%s (%d):" % (label, len(hits)))
        for r in hits:
            print("  %-28s %-48s %s" % (r["module"], r["unit"], r["evidence"]))

    if conditionals:
        print("")
        print("conditional _EXCLUDE edits (inside an if(), so not part of the unconditional set above):")
        for module, kind, tok in conditionals:
            print("  %-12s %-12s %s" % (module, kind, tok))
    print("")
    print("-> %s" % out_path)


if __name__ == "__main__":
    main()
