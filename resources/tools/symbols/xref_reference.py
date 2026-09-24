"""P2.1: cross-reference every PDB function against the reference engine source tree.

For each row of functions.csv decide whether its source file exists in the reference tree and
whether a same-named definition (Class::Method) exists there. Writes
resources/docs/reference_xref.csv and rewrites the xref section of resources/docs/engine_reference.md.

Usage: python resources/tools/symbols/xref_reference.py [--reference ../UnrealEngine3/Development/Src] [--rebuild-index]
"""
import argparse
import collections
import csv
import json
import re
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
DOCS = REPO / "resources" / "docs"
SYMBOLS = DOCS / "symbols"
CACHE = REPO / "resources" / "reference" / "reference_defs.json"
SRC_EXT = {".cpp", ".h", ".inl"}
SKIP_DIRS = {"intermediate", "_upgradereport_files", "unrealbuildtool", "targets"}

OUT_OF_LINE_RE = re.compile(r"^[ \t]*(?:[\w:<>,*&\s]+?\s+)?([A-Za-z_]\w*)::(~?[A-Za-z_]\w*|operator\S*)\s*\(", re.M)
# class header with or without the opening brace on the same line; forward declarations (ending in ;) excluded
CLASS_RE = re.compile(r"^\s*(?:class|struct)\s+(?:[A-Z_]+\s+)?([A-Za-z_]\w*)\s*(?::[^;{]*)?(?:\{|$)")
INLINE_RE = re.compile(r"^\s*(?:[\w:<>,*&~\s]+?\s+)?(~?[A-Za-z_]\w*|operator\S*)\s*\([^;{}]*\)\s*(?:const)?\s*(?:\{|:|$)", re.M)
FREE_RE = re.compile(r"^(?!\s*(?:if|for|while|switch|return|else|case|do|catch|sizeof|typedef|using|#|//)\b)(?:[A-Za-z_][\w:<>,*&\s]*?\s+)\*?&?([A-Za-z_]\w*|operator\S*)\s*\([^;]*$", re.M)
QUALIFIED_RE = re.compile(r"([A-Za-z_]\w*)::(~?[A-Za-z_]\w*|operator[^(\s]*)\(")
FREE_NAME_RE = re.compile(r"(?:^|\s|\*|&)([A-Za-z_]\w*|operator[^(\s]*)\(")
GENERATED_RE = re.compile(r"::exec\w+\(|::StaticClass\(|dynamic initializer|dynamic atexit|_dynamic_initializer_for_|_dynamic_atexit_destructor_for_|AutoInitializeRegistrants|AutoGenerateNames|\$initializer\$|`vector deleting|`scalar deleting|`vbase destructor|`default constructor closure|`copy constructor closure|\$E\d+|__unwindfunclet|__catch\$|__ehhandler")


# single-line only and no statement punctuation inside, so `a < b` ... `c > d` comparisons
# spanning lines are never paired up and deleted
TEMPLATE_ARGS_RE = re.compile(r"<[^<>;{}()\n]*>")


def strip_templates(s: str) -> str:
    """Remove template argument lists (innermost first) so TArray<T,A>::Add and
    template<class T> class TArray both reduce to plain identifiers."""
    prev = None
    while prev != s:
        prev = s
        s = TEMPLATE_ARGS_RE.sub("", s)
    return re.sub(r"^\s*template\s*", "", s)


COMMENT_RE = re.compile(r"//[^\n]*|/\*.*?\*/|\"(?:[^\"\\\n]|\\.)*\"|'(?:[^'\\\n]|\\.)*'", re.S)
HEADER_CLASS_RE = re.compile(r"(?:class|struct)\s+(?:[A-Z_]+\s+)?([A-Za-z_]\w*)\s*(?:final\s*)?(?::[^{;]*)?$")
HEADER_FUNC_RE = re.compile(r"(?:^|[\s*&])(?:([A-Za-z_]\w*)::)?(~?[A-Za-z_]\w*|operator[^\s(]*)\s*\([^;{}]*\)\s*(?:const)?\s*(?:volatile)?\s*(?:throw\s*\([^)]*\))?\s*(?::[^{;]*)?$")
KEYWORDS = {"if", "for", "while", "switch", "return", "else", "catch", "sizeof", "defined", "checkAtCompileTime", "check", "checkf", "__try", "__except", "__pragma"}


def scan_definitions(text: str, defs: set) -> None:
    """Find every function body `{` and classify what opened it, using a brace-depth scanner
    over comment- and string-stripped text. Class headers are recognised the same way, so
    in-class inline methods become Class::Method and multi-line headers are handled."""
    clean = COMMENT_RE.sub(lambda m: " " if m.group(0).startswith("/") else '""', text)
    clean = strip_templates(clean)
    stack: list[tuple[str, int]] = []  # (class name, depth at its opening brace)
    depth = 0
    start = 0
    i = 0
    n = len(clean)
    while i < n:
        c = clean[i]
        if c == "{":
            head = clean[start:i].strip()
            head = head.split(";")[-1].strip() if ";" in head else head
            head = head.split("}")[-1].strip() if "}" in head else head
            hm = HEADER_CLASS_RE.search(head)
            fm = HEADER_FUNC_RE.search(head) if not hm else None
            if hm:
                stack.append((hm.group(1), depth))
            elif fm and fm.group(2) not in KEYWORDS and not head.rstrip().endswith("="):
                cls, name = fm.group(1), fm.group(2)
                if cls:
                    defs.add(f"{cls}::{name}")
                elif stack and depth == stack[-1][1] + 1:
                    defs.add(f"{stack[-1][0]}::{name}")
                else:
                    defs.add(name)
            depth += 1
            start = i + 1
        elif c == "}":
            depth -= 1
            while stack and depth <= stack[-1][1]:
                stack.pop()
            start = i + 1
        elif c == ";":
            start = i + 1
        i += 1


def build_index(reference: Path) -> dict:
    defs: set[str] = set()
    files: dict[str, str] = {}
    n = 0
    for p in reference.rglob("*"):
        if not p.is_file():
            continue
        rel = p.relative_to(reference).as_posix()
        if any(part.lower() in SKIP_DIRS for part in p.relative_to(reference).parts):
            continue
        files[rel.lower()] = rel
        if p.suffix.lower() not in SRC_EXT:
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        scan_definitions(text, defs)
        n += 1
    return {"reference": str(reference), "source_files": n, "defs": sorted(defs), "files": files}


def load_index(reference: Path, rebuild: bool) -> dict:
    if CACHE.exists() and not rebuild:
        idx = json.loads(CACHE.read_text(encoding="utf-8"))
        if idx.get("reference") == str(reference):
            return idx
    t0 = time.time()
    idx = build_index(reference)
    CACHE.parent.mkdir(parents=True, exist_ok=True)
    CACHE.write_text(json.dumps(idx), encoding="utf-8")
    print(f"index: {idx['source_files']} source files, {len(idx['defs'])} definitions in {time.time() - t0:.1f}s", file=sys.stderr)
    return idx


def pdb_relative(file: str) -> str:
    key = "development\\src\\"
    if key not in file:
        return ""
    return file.split(key, 1)[1].replace("\\", "/")


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--reference", default=str(REPO.parent / "UnrealEngine3" / "Development" / "Src"))
    ap.add_argument("--rebuild-index", action="store_true")
    ap.add_argument("--suffix", default="")
    args = ap.parse_args(argv[1:])
    reference = Path(args.reference).resolve()
    idx = load_index(reference, args.rebuild_index)
    defs = set(idx["defs"])
    files = idx["files"]

    out_rows = []
    per_module = collections.defaultdict(collections.Counter)
    per_file_module = collections.defaultdict(lambda: [set(), set()])
    with (SYMBOLS / f"functions{args.suffix}.csv").open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            rel = pdb_relative(row["file"])
            ref_file = files.get(rel.lower(), "") if rel else ""
            plain = strip_templates(row["demangled"])
            q = QUALIFIED_RE.search(plain)
            if q:
                qualified = f"{q.group(1)}::{q.group(2)}"
            else:
                fm = FREE_NAME_RE.search(plain.split("(")[0] + "(") if "(" in plain else None
                qualified = fm.group(1) if fm else ""
            in_ref = bool(qualified) and qualified in defs
            if row.get("is_thunk") == "1":
                status = "thunk"
            elif GENERATED_RE.search(row["demangled"]):
                status = "generated"
            elif in_ref:
                status = "reference"
            else:
                status = "missing"
            module = row["module"] or f"<{row['origin']}>"
            per_module[module][status] += 1
            if rel:
                per_file_module[module][0].add(rel)
                if ref_file:
                    per_file_module[module][1].add(rel)
            out_rows.append({
                "rva": row["rva"], "demangled": row["demangled"], "module": module, "file": rel,
                "file_in_reference": int(bool(ref_file)), "reference_file": ref_file,
                "definition_in_reference": int(in_ref), "qualified": qualified, "status": status,
            })

    out = DOCS / f"reference_xref{args.suffix}.csv"
    with out.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(out_rows[0].keys()))
        w.writeheader()
        w.writerows(out_rows)

    lines = ["<!-- xref:begin (generated by resources/tools/symbols/xref_reference.py) -->", "",
             "## File-level overlap (Shipping PDB source files present in the reference tree)", "",
             "| Module | Present / PDB files |", "|---|---:|"]
    for module, (pdb_files, ref_files) in sorted(per_file_module.items(), key=lambda kv: -len(kv[1][0])):
        lines.append(f"| {module} | {len(ref_files)} / {len(pdb_files)} |")
    total_pdb = sum(len(v[0]) for v in per_file_module.values())
    total_ref = sum(len(v[1]) for v in per_file_module.values())
    lines += [f"| **total** | **{total_ref} / {total_pdb}** |", "",
              "## Function-level status (`reference_xref.csv`)", "",
              "`reference` = a same-named `Class::Method` definition exists in the reference tree; `generated` = script thunks, StaticClass, dynamic initializers; `thunk` = IDA thunk; `missing` = must be written from the decompile.", "",
              "| Module | Functions | reference | missing | generated | thunk | reference share of non-generated |", "|---|---:|---:|---:|---:|---:|---:|"]
    for module, c in sorted(per_module.items(), key=lambda kv: -sum(kv[1].values())):
        total = sum(c.values())
        real = c["reference"] + c["missing"]
        lines.append(f"| {module} | {total:,} | {c['reference']:,} | {c['missing']:,} | {c['generated']:,} | {c['thunk']:,} | {100.0 * c['reference'] / max(real, 1):.0f} % |")
    lines += ["", "<!-- xref:end -->"]
    section = "\n".join(lines)

    doc = DOCS / "engine_reference.md"
    text = doc.read_text(encoding="utf-8")
    if "<!-- xref:begin" in text and "<!-- xref:end -->" in text:
        pre = text.split("<!-- xref:begin", 1)[0]
        post = text.split("<!-- xref:end -->", 1)[1]
        text = pre + section + post
    else:
        text = text.rstrip() + "\n\n" + section + "\n"
    doc.write_text(text, encoding="utf-8")
    print(f"rows={len(out_rows)} -> {out}; engine_reference.md updated")
    for module in ("core", "engine", "dishonoredgame"):
        c = per_module.get(module)
        if c:
            real = c["reference"] + c["missing"]
            print(f"  {module}: reference {c['reference']}/{real} ({100.0 * c['reference'] / max(real, 1):.0f}%)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
