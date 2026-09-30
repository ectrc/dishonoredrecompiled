"""Check every retail address this tree cites against the retail function list.

Agent EE found that four of the five 2013 addresses in one comment block were **fabricated** - the
2012 values with a single nibble changed, one of them naming a different function entirely - and
three separate briefs today quoted *2012* addresses as retail, all copied out of one comment block
that does not say which build it is from. Both failures are mechanical to detect and neither was.

For every `0x...` in a DISHONORED comment or tag in the tree, this classifies the address as:

  ok-2013      a function start in the retail 2013 database
  ok-2013-mid  inside a retail 2013 function but not its start (usually a call site; fine when the
               comment says so, worth a look when it claims to name the function)
  is-2012      not a 2013 function start, but *is* a 2012 one - a mislabelled address. The mapped
               2013 value is printed, so the fix is mechanical
  unknown      neither. Either fabricated, or data rather than code, or from a build we do not have

Usage:
    python resources/tools/rva_sweep.py                 # whole tree, summary + every suspect
    python resources/tools/rva_sweep.py --csv out.csv   # every citation, machine readable
    python resources/tools/rva_sweep.py --path source/Development/Src/Engine
"""
from __future__ import annotations

import argparse
import bisect
import csv
import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SYMBOLS = REPO / "resources" / "docs" / "symbols"
SRC = REPO / "source" / "Development" / "Src"

# A citation is a hex literal inside a comment that also mentions rva/retail/2013/2012, which is how
# every DISHONORED tag in this tree writes one. Bare hex in code (masks, flags) is not a citation.
HEX = re.compile(r"0x[0-9a-fA-F]{5,8}")
# A line only cites an address if it says so in words. Bare hex is a mask or a constant.
CITES = re.compile(r"(rva|retail|DISHONORED\(|2012 0x|2013 0x)", re.I)


def load_functions(name: str) -> tuple[set[int], list[tuple[int, int, str]]]:
    """Return (starts, sorted [start, end, name]) from a functions csv."""
    starts: set[int] = set()
    spans: list[tuple[int, int, str]] = []
    p = SYMBOLS / name
    if not p.is_file():
        return starts, spans
    with p.open(newline="", encoding="utf-8", errors="replace") as fh:
        for row in csv.DictReader(fh):
            raw = row.get("rva") or row.get("rva_2013") or row.get("address") or ""
            try:
                start = int(raw, 16)
            except (TypeError, ValueError):
                continue
            try:
                size = int(row.get("size") or 0)
            except ValueError:
                size = 0
            starts.add(start)
            spans.append((start, start + max(size, 1), row.get("name") or ""))
    spans.sort()
    return starts, spans


def load_map() -> dict[int, int]:
    """2012 rva -> 2013 rva."""
    out: dict[int, int] = {}
    p = SYMBOLS / "match_2012_2013.csv"
    if not p.is_file():
        return out
    with p.open(newline="", encoding="utf-8", errors="replace") as fh:
        for row in csv.DictReader(fh):
            try:
                out[int(row["rva_2012"], 16)] = int(row["rva_2013"], 16)
            except (KeyError, TypeError, ValueError):
                continue
    return out


def containing(spans: list[tuple[int, int, str]], addr: int) -> str | None:
    i = bisect.bisect_right(spans, (addr, 1 << 62, "")) - 1
    if i >= 0 and spans[i][0] <= addr < spans[i][1]:
        return spans[i][2]
    return None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--path", default=str(SRC))
    ap.add_argument("--csv")
    args = ap.parse_args()

    starts13, spans13 = load_functions("functions_2013.csv")
    starts12, _ = load_functions("functions.csv")
    to13 = load_map()
    if not starts13:
        print("no functions_2013.csv - cannot classify")
        return 2

    lo = min(min(starts13), min(starts12) if starts12 else min(starts13))
    hi = max(e for _, e, _ in spans13)
    rows = []
    # A relative --path is resolved against the repo, not the shell's directory: the rows below are
    # reported as paths relative to REPO, so a bare `--path source/...` used to rglob correctly and
    # then die in relative_to (agent ES, which worked around it by always passing an absolute path).
    root = Path(args.path)
    if not root.is_absolute():
        root = (REPO / root).resolve()
    for path in sorted(root.rglob("*")):
        if path.suffix.lower() not in (".cpp", ".h", ".inl"):
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for n, line in enumerate(text.splitlines(), 1):
            bare = line.strip()
            if not (bare.startswith("//") or bare.startswith("*") or "/*" in bare):
                continue
            if not CITES.search(line):
                continue
            for m in HEX.finditer(line):
                tok = m.group(0)
                addr = int(tok, 16)
                # A mask or a round constant is not an address citation.
                body = tok[2:]
                if len(set(body.lower())) <= 1 or body.lower() in ("7fffffff", "80000000"):
                    continue
                # Outside the image's own code range it cannot be an rva of either build.
                if not (lo <= addr <= hi):
                    continue
                # How the citation labels itself, from the 40 characters before it.
                before = line[max(0, m.start() - 40):m.start()].lower()
                says_2012 = "2012" in before
                says_data = any(w in line.lower() for w in ("global", "static of", " data ", "vtable", "string"))

                if addr in starts13:
                    verdict, note = "ok-2013", ""
                elif (fn := containing(spans13, addr)) is not None:
                    verdict, note = "ok-2013-mid", "inside %s" % fn
                elif addr in starts12:
                    mapped = to13.get(addr)
                    if says_2012:
                        verdict = "ok-2012-labelled"
                        note = "2013 is 0x%x" % mapped if mapped else ""
                    else:
                        # A 2012 address presented as retail. This is the defect three briefs hit.
                        verdict = "MISLABELLED-2012"
                        note = "2013 is 0x%x" % mapped if mapped else "no 2013 match"
                elif says_data:
                    verdict, note = "data-or-global", ""
                elif says_2012:
                    verdict, note = "unknown-2012", ""
                else:
                    # Claims to be retail, is not a function in either build.
                    verdict, note = "UNKNOWN-CLAIMED-2013", ""
                rows.append((verdict, str(path.relative_to(REPO)), n, tok, note,
                             line.strip()[:120]))

    counts: dict[str, int] = {}
    for r in rows:
        counts[r[0]] = counts.get(r[0], 0) + 1
    print("citations: %d" % len(rows))
    for k in ("ok-2013", "ok-2013-mid", "ok-2012-labelled", "data-or-global",
              "unknown-2012", "MISLABELLED-2012", "UNKNOWN-CLAIMED-2013"):
        print("  %-22s %d" % (k, counts.get(k, 0)))

    suspects = [r for r in rows if r[0] in ("MISLABELLED-2012", "UNKNOWN-CLAIMED-2013")]
    if suspects:
        print("\nsuspect citations (%d):" % len(suspects))
        for verdict, rel, n, addr, note, line in suspects[:200]:
            print("  %-8s %s:%d  %s  %s" % (verdict, rel, n, addr, note))
            print("           %s" % line)

    if args.csv:
        with open(args.csv, "w", newline="", encoding="utf-8") as fh:
            w = csv.writer(fh)
            w.writerow(["verdict", "file", "line", "address", "note", "text"])
            w.writerows(rows)
        print("\nwrote %s" % args.csv)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
