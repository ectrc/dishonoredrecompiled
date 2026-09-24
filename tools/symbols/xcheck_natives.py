"""P1.8: cross-check exe native bindings against `native` declarations in the DFSDK UnrealScript sources.

Usage: python tools/symbols/xcheck_natives.py [--uc D:/DishonoredMapMaking/DishonoredEditor/Development/Src]
Reads docs/symbols/natives.csv, writes docs/symbols/natives_xcheck.md.
"""
import argparse
import csv
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CLASS_RE = re.compile(r"^\s*class\s+(\w+)\s+extends", re.IGNORECASE | re.MULTILINE)
NATIVE_FUNC_RE = re.compile(
    r"^\s*native(?:\s*\(\s*\d+\s*\))?[\w\s]*?\b(?:function|event)\b\s*(?:[\w<>.]+(?:\s*<[^>]*>)?\s+)?(\w+)\s*\(",
    re.IGNORECASE | re.MULTILINE,
)


def parse_uc(root: Path) -> tuple[dict[str, set[str]], dict[str, str]]:
    """Returns natives per class and the DFSDK module folder each class came from.
    Only the DishonoredGame folder holds UE Explorer decompiles of the shipped game; Engine/Core
    there are stock UDK 2010 sources, so mismatches in those are expected."""
    out: dict[str, set[str]] = {}
    module_of: dict[str, str] = {}
    for uc in root.rglob("*.uc"):
        text = uc.read_text(encoding="utf-8", errors="replace")
        m = CLASS_RE.search(text)
        cls = (m.group(1) if m else uc.stem)
        module_of[cls] = uc.relative_to(root).parts[0]
        funcs = set(NATIVE_FUNC_RE.findall(text))
        if funcs:
            out.setdefault(cls, set()).update(funcs)
    return out, module_of


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--uc", default=r"D:\DishonoredMapMaking\DishonoredEditor\Development\Src")
    ap.add_argument("--natives", default=str(REPO / "docs" / "symbols" / "natives.csv"))
    args = ap.parse_args(argv[1:])

    exe: dict[str, set[str]] = {}
    with open(args.natives, newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            cls = row["class"]
            cls = cls[1:] if cls[:1] in "UA" and cls[1:2].isupper() else cls
            exe.setdefault(cls, set()).add(row["func"])
    uc, module_of = parse_uc(Path(args.uc))

    matched, exe_only, uc_only = [], [], []
    dis_matched, dis_exe_only = 0, 0
    for cls in sorted(set(exe) | set(uc)):
        e, u = exe.get(cls, set()), uc.get(cls, set())
        matched += [f"{cls}::{n}" for n in sorted(e & u)]
        exe_only += [f"{cls}::{n}" for n in sorted(e - u)]
        uc_only += [f"{cls}::{n}" for n in sorted(u - e)]
        if module_of.get(cls) == "DishonoredGame":
            dis_matched += len(e & u)
            dis_exe_only += len(e - u)

    total = len(matched) + len(exe_only)
    dis_total = dis_matched + dis_exe_only
    out = REPO / "docs" / "symbols" / "natives_xcheck.md"
    with out.open("w", encoding="utf-8") as f:
        f.write("# Native function cross-check: exe `exec*` vs DFSDK `.uc` `native` declarations\n\n")
        f.write("Only the DFSDK `DishonoredGame` classes are decompiled from the shipped game; its `Engine`/`Core` are stock UDK 2010 (engine 7026), so the all-classes numbers include an engine-version mismatch.\n\n")
        f.write(f"| Set | Count |\n|---|---:|\n| matched | {len(matched)} |\n| exe-only | {len(exe_only)} ({100.0 * len(exe_only) / max(total, 1):.1f}% of exe natives) |\n| uc-only | {len(uc_only)} |\n")
        f.write(f"| DishonoredGame classes: matched | {dis_matched} |\n| DishonoredGame classes: exe-only | {dis_exe_only} ({100.0 * dis_exe_only / max(dis_total, 1):.1f}% of exe natives in those classes) |\n\n")
        for title, items in [("Exe-only", exe_only), ("UC-only", uc_only)]:
            f.write(f"## {title} ({len(items)})\n\n")
            f.write("\n".join(f"- `{i}`" for i in items) + "\n\n")
    print(f"matched={len(matched)} exe_only={len(exe_only)} uc_only={len(uc_only)} -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
