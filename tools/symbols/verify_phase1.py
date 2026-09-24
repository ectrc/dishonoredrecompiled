"""P1.12: Phase 0/1 exit check. Verifies every deliverable exists and re-runs the acceptance rules.

Usage: python tools/symbols/verify_phase1.py   (exit 0 = PASS)
"""
import csv
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DOCS = REPO / "docs"
SYM = DOCS / "symbols"
TYPES = DOCS / "types"

RESULTS: list[tuple[str, bool, str]] = []


def check(task: str, ok: bool, detail: str = "") -> None:
    RESULTS.append((task, ok, detail))


def read_csv(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def exists(task: str, *paths: Path) -> bool:
    missing = [p for p in paths if not p.exists()]
    check(task, not missing, "missing: " + ", ".join(str(p.relative_to(REPO)) for p in missing) if missing else "all files present")
    return not missing


def main() -> int:
    # P0
    exists("P0.1 repo files", REPO / ".gitignore", REPO / "README.md", REPO / "PLAN.md")
    if exists("P0.2 manifest", DOCS / "binaries.md"):
        text = (DOCS / "binaries.md").read_text(encoding="utf-8")
        check("P0.2 GUIDs", "5c4d3821aba1bf44944e54d2c991bd45/1" in text and "a172ba00a585a14481e541a22e4fc9c6/1" in text, "2013 exe and 2012 Shipping GUIDs listed")
    exists("P0.3 toolchain", DOCS / "toolchain.md")
    exists("P0.4 cmake", REPO / "CMakeLists.txt", REPO / "CMakePresets.json", REPO / "run-vcvars.cmd", REPO / "Development/Src/Launch/Src/Launch.cpp")
    if exists("P0.5 golden logs", DOCS / "golden/2012_arkprofile_launch.log", DOCS / "golden/2013_retail_launch.log", DOCS / "golden/README.md", REPO / "tools/normalize_log.py"):
        g = (DOCS / "golden/2012_arkprofile_launch.log").read_text(encoding="utf-8", errors="replace")
        check("P0.5 2012 log milestones", "Init: Object subsystem initialized" in g and "LoadMap:" in g and "Bringing World" in g)

    # P1.1
    if exists("P1.1 db verify", SYM / "db_verify.txt"):
        v = dict(line.split("=", 1) for line in (SYM / "db_verify.txt").read_text().splitlines() if "=" in line)
        unnamed_pct = float(re.search(r"\(([\d.]+)%\)", v.get("functions_unnamed", "(100%)")).group(1))
        core = all(v.get(f"type:{t}") == "yes" for t in ("UObject", "UClass", "UProperty", "FName", "FString", "FArchive"))
        check("P1.1 names and types", unnamed_pct <= 5.0 and core, f"unnamed={unnamed_pct}% core types={'ok' if core else 'MISSING'}")

    # P1.2 / P1.4
    funcs = read_csv(SYM / "functions.csv") if exists("P1.2 functions", SYM / "functions.csv", SYM / "segments.csv") else []
    if funcs:
        names = {r["demangled"].split("(")[0].split(" ")[-1] for r in funcs}
        wanted = ["FEngineLoop::Init", "UObject::Serialize", "appInit", "FName::StaticInit", "UObject::StaticConstructObject"]
        missing = [w for w in wanted if not any(w in r["demangled"] for r in funcs)]
        check("P1.2 spot-check", not missing, f"missing={missing}" if missing else f"{len(funcs)} functions, all spot-check names present")
        if "module" in funcs[0]:
            non_crt = [r for r in funcs if r["origin"] != "crt"]
            attributed = [r for r in non_crt if r["module"]]
            pct = 100.0 * len(attributed) / max(len(non_crt), 1)
            check("P1.4 attribution >= 90%", pct >= 90.0, f"{pct:.1f}% of {len(non_crt)} non-CRT functions have a module")
        else:
            check("P1.4 attribution >= 90%", False, "functions.csv not joined (no module column)")

    # P1.3
    if exists("P1.3 DIA outputs", SYM / "compilands.csv", SYM / "sourcefiles.txt", SYM / "pdb_functions.csv", SYM / "lines.csv"):
        src = (SYM / "sourcefiles.txt").read_text(encoding="utf-8")
        # Shipping is the Win32-OSSSteamworks configuration with Wwise audio: no OnlineSubsystemPC, no XAudio2
        modules = ["core", "engine", "gameframework", "ipdrv", "launch", "windrv", "d3d9drv", "gfxui", "akaudio", "onlinesubsystemsteamworks", "dishonoredgame"]
        missing = [m for m in modules if f"\\development\\src\\{m}\\" not in src]
        check("P1.3 runtime modules in sourcefiles", not missing, f"missing={missing}" if missing else f"{len(src.splitlines())} source files")
        pdb_unique = {r["rva"] for r in read_csv(SYM / "pdb_functions.csv")}
        ida = {r["rva"] for r in funcs}
        check("P1.3 DIA/IDA overlap", len(pdb_unique & ida) >= 0.85 * len(pdb_unique), f"{len(pdb_unique)} DIA RVAs, {len(pdb_unique & ida)} also IDA functions")

    # P1.5
    if exists("P1.5 globals/imports", SYM / "globals.csv", SYM / "imports.csv"):
        g = {r["name"] for r in read_csv(SYM / "globals.csv")}
        wanted = ["GObjObjects", "GEngine", "GWorld", "GNatives", "GMalloc"]
        missing = [w for w in wanted if not any(x.startswith(f"?{w}@") or x == w for x in g)]
        check("P1.5 core globals", not missing, f"missing={missing}" if missing else "GObjObjects GEngine GWorld GNatives GMalloc present")
        dlls = {r["dll"].lower().removesuffix(".dll") for r in read_csv(SYM / "imports.csv")}
        check("P1.5 import dlls", {"steam_api", "d3d9", "binkw32", "dinput8"} <= dlls, f"{len(dlls)} DLLs: {sorted(dlls)}")

    # P1.6
    if exists("P1.6 types", TYPES / "sizes.csv", TYPES / "types.json", TYPES / "all_types.h"):
        sizes = {r["name"]: int(r["size"]) for r in read_csv(TYPES / "sizes.csv")}
        ok = sizes.get("FName") == 8 and sizes.get("FString") == 12 and all(k in sizes for k in ("UObject", "UClass", "UProperty", "FArchive")) and any(k.startswith("TArray<") and v == 12 for k, v in sizes.items())
        check("P1.6 core sizes", ok, f"FName={sizes.get('FName')} FString={sizes.get('FString')} UObject={sizes.get('UObject')} UClass={sizes.get('UClass')}")

    # P1.7
    if exists("P1.7 vtables", SYM / "vtables.csv"):
        uobj = [r for r in read_csv(SYM / "vtables.csv") if r["class"] == "UObject"]
        classes = {r["class"] for r in read_csv(SYM / "vtables.csv")}
        static_classes = len(read_csv(SYM / "classes.csv")) if (SYM / "classes.csv").exists() else 0
        check("P1.7 UObject vtable", any("UObject::Serialize" in r["target_demangled"] for r in uobj) and len(classes) >= static_classes, f"UObject slots={len(uobj)} classes={len(classes)} StaticClass funcs={static_classes}")

    # P1.8
    if exists("P1.8 natives", SYM / "natives.csv", SYM / "classes.csv", SYM / "natives_xcheck.md"):
        nat = read_csv(SYM / "natives.csv")
        indexed = sum(1 for r in nat if r["native_index"])
        dlc05 = any(r["func"].startswith("Req_DLC05") for r in nat)
        xc = (SYM / "natives_xcheck.md").read_text(encoding="utf-8")
        m = re.search(r"DishonoredGame classes: exe-only \| (\d+) \(([\d.]+)%", xc)
        exe_only_pct = float(m.group(2)) if m else 100.0
        check("P1.8 natives", not dlc05 and indexed > 0 and exe_only_pct < 10.0, f"{len(nat)} natives, {indexed} with index, DLC05={dlc05}, DishonoredGame exe-only={exe_only_pct}%")

    # P1.9
    if exists("P1.9 hardcoded names", SYM / "hardcoded_names.csv"):
        rows = read_csv(SYM / "hardcoded_names.csv")
        idx = [int(r["index"]) for r in rows]
        by = {int(r["index"]): r["name"] for r in rows}
        unique = len(set(idx)) == len(idx) and len(set(by.values())) == len(rows)
        # UnNames.h indices are sparse by design (reserved holes), so contiguity is not required
        check("P1.9 names table", by.get(0) == "None" and len(rows) >= 400 and unique and "Engine" in by.values() and "Core" in by.values(), f"{len(rows)} names, index0={by.get(0)!r}, unique={unique}, max index={max(idx) if idx else None}")

    # P1.10
    if exists("P1.10 module map", DOCS / "module_map.md"):
        mm = (DOCS / "module_map.md").read_text(encoding="utf-8")
        pcts = [float(x) for x in re.findall(r"\| ([\d.]+) \|(?: runtime| editor)?", mm)]
        rows_ok = all(f"| {m} |" in mm for m in ("core", "engine", "dishonoredgame", "gfxui"))
        check("P1.10 module rows", rows_ok, "core/engine/dishonoredgame/gfxui rows present" if rows_ok else "rows missing")

    # P1.11 / P1.12
    exists("P1.11 2013 db verify", SYM / "db_verify_2013.txt")
    exists("P1.12 tracking", DOCS / "PHASE1.md", DOCS / "progress.md")

    width = max(len(t) for t, _, _ in RESULTS)
    failed = 0
    for task, ok, detail in RESULTS:
        failed += not ok
        print(f"{'PASS' if ok else 'FAIL'}  {task.ljust(width)}  {detail}")
    print(f"\n{len(RESULTS) - failed}/{len(RESULTS)} checks passed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
