"""P2.7: compare Dishonored's script opcode / numbered-native table with the reference sources.

UObject natives with a GNatives index below 0x80 are the bytecode opcodes (EX_* in UnStack.h) and
the first numbered natives. natives.csv carries every index a (possibly COMDAT-folded) exec
function owns, ';'-joined. Writes resources/docs/symbols/opcodes.md.

Usage: python resources/tools/symbols/xcheck_opcodes.py
"""
import csv
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
SYMBOLS = REPO / "resources" / "docs" / "symbols"
CORE = REPO / "source" / "Development" / "Src" / "Core"
LIMIT = 0x80


def dishonored_table() -> dict[int, set[str]]:
    table: dict[int, set[str]] = {}
    with (SYMBOLS / "natives.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            if r["class"] != "UObject" or not r["native_index"]:
                continue
            for idx in r["native_index"].split(";"):
                i = int(idx)
                if 0 <= i < LIMIT:
                    table.setdefault(i, set()).add(r["func"])
    return table


def reference_table() -> dict[int, str]:
    enum = {}
    for name, val in re.findall(r"^\s*(EX_\w+)\s*=\s*(0x[0-9A-Fa-f]+|\d+)", (CORE / "Inc" / "UnStack.h").read_text(encoding="utf-8", errors="replace"), re.M):
        enum[name] = int(val, 0)
    ref = {}
    src = (CORE / "Src" / "UnCorSc.cpp").read_text(encoding="utf-8", errors="replace")
    for cls, num, fn in re.findall(r"IMPLEMENT_FUNCTION\(\s*(\w+)\s*,\s*(EX_\w+|\d+)\s*,\s*exec(\w+)\s*\)", src):
        if cls != "UObject":
            continue
        val = int(num) if num.isdigit() else enum.get(num)
        if val is not None and val < LIMIT:
            ref[val] = fn
    return ref


def main(argv: list[str]) -> int:
    dis = dishonored_table()
    ref = reference_table()
    rows = []
    for i in sorted(set(dis) | set(ref)):
        d = dis.get(i, set())
        r = ref.get(i, "")
        if not d:
            status = "missing in exe (folded or unused)"
        elif not r:
            status = "not an opcode in reference"
        elif r in d:
            status = "same" if len(d) == 1 else "same (folded with " + ", ".join(sorted(d - {r})) + ")"
        else:
            status = "**DIFF**"
        rows.append((i, ", ".join(sorted(d)), r, status))
    diffs = [r for r in rows if r[3] == "**DIFF**"]
    out = SYMBOLS / "opcodes.md"
    with out.open("w", encoding="utf-8") as f:
        f.write("# Script opcodes: 2012 exe vs reference UnStack.h / UnCorSc.cpp\n\n")
        f.write("UObject natives with GNatives index < 0x80. Identical COMDAT folding merges byte-identical exec\n"
                "functions (execFalse/execIntZero/execNoObject, ...), so a folded function lists every index it\n"
                "serves; the name shown is the one the linker kept. Cooked packages contain Dishonored's numbering,\n"
                "so the ported enum must match the Dishonored column wherever it is not folded-ambiguous.\n\n")
        f.write(f"{len(rows)} slots, {len(diffs)} real differences.\n\n| Index | Dishonored (natives.csv) | Reference 10897 | Status |\n|---:|---|---|---|\n")
        for i, d, r, s in rows:
            f.write(f"| {i} (0x{i:02x}) | {d} | {r} | {s} |\n")
    print(f"slots={len(rows)} diffs={len(diffs)} -> {out}")
    for r in diffs:
        print("  ", r)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
