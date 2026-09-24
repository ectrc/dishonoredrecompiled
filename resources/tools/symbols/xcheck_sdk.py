"""P1.6: compare PDB-derived member offsets (docs/types/types.json, 2012) with the CodeRed runtime dump
of the 2013 exe used by dismod (defs.hpp: `Type Name; // 0xOFFS (0xSIZE)`).

Writes docs/types/xcheck_sdk.md.
Usage: python resources/tools/symbols/xcheck_sdk.py [--sdk D:/Christmas/github/dismod/external/sdk/include/defs.hpp]
"""
import argparse
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
CLASS_RE = re.compile(r"^(?:class|struct)\s+(\w+)(?:\s*:\s*public\s+(\w+))?\s*$")
MEMBER_RE = re.compile(r"^\s*(?:class |struct )?[\w:<>*&\s]+?\s\*?(\w+)(?:\[\d+\])?;\s*//\s*0x([0-9A-Fa-f]+)\s*\(0x([0-9A-Fa-f]+)\)")


def parse_sdk(path: Path) -> dict[str, dict[str, tuple[int, int]]]:
    classes: dict[str, dict[str, tuple[int, int]]] = {}
    current = None
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = CLASS_RE.match(line.strip())
        if m:
            current = m.group(1)
            classes[current] = {}
            continue
        if current is None:
            continue
        m = MEMBER_RE.match(line)
        if m:
            classes[current][m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
        if line.strip() == "};":
            current = None
    return {k: v for k, v in classes.items() if v}


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--sdk", default=r"D:\Christmas\github\dismod\external\sdk\include\defs.hpp")
    args = ap.parse_args(argv[1:])
    sdk = parse_sdk(Path(args.sdk))
    with (REPO / "resources" / "docs" / "types" / "types.json").open(encoding="utf-8") as f:
        pdb = {t["name"]: t for t in json.load(f)["types"]}

    out = REPO / "resources" / "docs" / "types" / "xcheck_sdk.md"
    compared = 0
    mismatches = 0
    with out.open("w", encoding="utf-8") as f:
        f.write("# Layout cross-check: 2012 PDB types vs 2013 CodeRed runtime dump (dismod defs.hpp)\n\n")
        f.write("Mismatches are expected where the 2013 build added or moved fields; they seed the Phase 7 delta list.\n\n")
        for cls, members in sorted(sdk.items()):
            t = pdb.get(cls)
            f.write(f"## {cls}\n\n")
            if t is None:
                f.write("Not present in PDB types.\n\n")
                continue
            pdb_members = {m["name"]: m for m in t["members"] if not m["is_base"]}
            f.write(f"PDB size {t['size']}. | Member | SDK offset | PDB offset | Match |\n|---|---:|---:|---|\n")
            for name, (off, size) in members.items():
                pm = pdb_members.get(name)
                compared += 1
                if pm is None:
                    f.write(f"| {name} | 0x{off:X} | – | missing in PDB |\n")
                    mismatches += 1
                elif pm["offset"] != off:
                    f.write(f"| {name} | 0x{off:X} | 0x{pm['offset']:X} | **no** |\n")
                    mismatches += 1
                else:
                    f.write(f"| {name} | 0x{off:X} | 0x{pm['offset']:X} | yes |\n")
            f.write("\n")
        f.write(f"Compared {compared} members across {len(sdk)} classes; {mismatches} mismatches.\n")
    print(f"classes={len(sdk)} compared={compared} mismatches={mismatches} -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
