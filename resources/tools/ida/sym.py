"""agentAU: look up 2012/2013 rvas of a mangled-name pattern from the symbol CSVs.
  python build/agentAU_work/sym.py <regex> [--class]
"""
import csv, re, sys
from pathlib import Path
S = Path("resources/docs/symbols")
pat = re.compile(sys.argv[1])
m12 = {}
for r in csv.DictReader(open(S/"functions.csv", encoding="utf-8", errors="replace")):
    if pat.search(r["mangled"]) or pat.search(r["demangled"]):
        m12[r["mangled"]] = r
m13 = {}
for r in csv.DictReader(open(S/"functions_2013.csv", encoding="utf-8", errors="replace")):
    if pat.search(r["mangled"]) or pat.search(r["demangled"]):
        m13[r["mangled"]] = r
match = {}
for r in csv.DictReader(open(S/"match_2012_2013.csv", encoding="utf-8", errors="replace")):
    match[r["rva_2012"]] = r
out = []
for mang, r in sorted(m12.items(), key=lambda kv: int(kv[1]["rva"], 16)):
    mm = match.get(r["rva"])
    r13 = m13.get(mang)
    rva13 = (r13["rva"] if r13 else (mm["rva_2013"] if mm and mm["name"] == mang else "?"))
    meth = mm["method"] + "/" + mm["ratio"] if mm else "-"
    out.append((r["rva"], rva13, r["size"], (r13 or {}).get("size", mm["size_2013"] if mm else "?"), meth, r["demangled"][:110], r.get("line","")))
for o in out:
    print(f"2012 {o[0]:>10} ({o[2]:>5}b)  2013 {o[1]:>10} ({o[3]:>5}b)  {o[4]:<18} L{o[6]:<6} {o[5]}")
# names only in the 2013 set
for mang, r in sorted(m13.items(), key=lambda kv: int(kv[1]["rva"], 16)):
    if mang not in m12:
        print(f"2013-only {r['rva']} ({r['size']}b) {r['demangled'][:110]}")
