"""P1.4: attribute every IDA function to a source file and module using the DIA line records.

Reads  docs/symbols/functions.csv, lines.csv
Writes docs/symbols/functions.csv (adds file,line,module,origin), functions_nofile.csv,
       functions_nofile_summary.md
Usage: python tools/symbols/join.py [suffix]
"""
import collections
import csv
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SYMBOLS = REPO / "docs" / "symbols"
SRC_RE = re.compile(r"development\\src\\([a-z0-9_]+)\\")
EXT_RE = re.compile(r"development\\external\\([a-z0-9_.\-]+)\\")
CRT_MARKERS = ("f:\\dd\\vctools", "\\crt\\src\\", "\\vc\\include\\", "\\atlmfc\\")
NAMESPACE_RE = re.compile(r"^((?:[A-Za-z_]\w*::)+)")


def classify(file: str) -> tuple[str, str]:
    m = SRC_RE.search(file)
    if m:
        return m.group(1), "game"
    m = EXT_RE.search(file)
    if m:
        return m.group(1), f"external:{m.group(1)}"
    if any(marker in file for marker in CRT_MARKERS):
        return "crt", "crt"
    return "", "unknown"


def bucket(demangled: str) -> str:
    m = NAMESPACE_RE.match(demangled)
    if m:
        return m.group(1).rstrip(":").split("::")[0]
    for prefix in ("GFx", "GRenderer", "GFile", "Fx", "OC3", "Nx", "Ak", "SpeedTree", "CSpeedTree", "PathEngine", "tinyxml", "png_", "inflate", "deflate", "lzo", "ogg_", "vorbis_"):
        if demangled.startswith(prefix):
            return prefix
    return demangled.split("(")[0].split("::")[0][:12]


def main(argv: list[str]) -> int:
    suffix = argv[1] if len(argv) > 1 else ""
    first_line: dict[str, tuple[str, str]] = {}
    with (SYMBOLS / f"lines{suffix}.csv").open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            key = row["function_rva"]
            if key not in first_line:
                first_line[key] = (row["file"], row["line"])

    path = SYMBOLS / f"functions{suffix}.csv"
    with path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        base_fields = [c for c in reader.fieldnames if c not in ("file", "line", "module", "origin")]

    nofile = []
    stats = collections.Counter()
    for row in rows:
        file, line = first_line.get(row["rva"].lower(), ("", ""))
        module, origin = classify(file) if file else ("", "unknown")
        row["file"], row["line"], row["module"], row["origin"] = file, line, module, origin
        stats[origin] += 1
        if not file:
            nofile.append(row)

    fields = base_fields + ["file", "line", "module", "origin"]
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)

    with (SYMBOLS / f"functions_nofile{suffix}.csv").open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["va", "rva", "size", "mangled", "demangled", "is_lib", "bucket"])
        w.writeheader()
        for row in nofile:
            w.writerow({k: row[k] for k in ("va", "rva", "size", "mangled", "demangled", "is_lib")} | {"bucket": bucket(row["demangled"])})

    buckets = collections.Counter()
    bucket_bytes = collections.Counter()
    for row in nofile:
        b = bucket(row["demangled"])
        buckets[b] += 1
        bucket_bytes[b] += int(row["size"])
    non_crt = sum(v for k, v in stats.items() if k != "crt")
    attributed = non_crt - stats["unknown"]
    with (SYMBOLS / f"functions_nofile_summary{suffix}.md").open("w", encoding="utf-8") as f:
        f.write("# Functions without PDB line records\n\n")
        f.write(f"Total functions: {len(rows)}. Attributed to a module: {attributed} of {non_crt} non-CRT ({100.0 * attributed / max(non_crt, 1):.1f}%). CRT: {stats['crt']}. No file: {len(nofile)}.\n\n")
        f.write("| Bucket (namespace / prefix) | Functions | Bytes |\n|---|---:|---:|\n")
        for b, n in buckets.most_common(60):
            f.write(f"| `{b}` | {n} | {bucket_bytes[b]:,} |\n")
    print(f"functions={len(rows)} attributed={attributed}/{non_crt} crt={stats['crt']} nofile={len(nofile)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
