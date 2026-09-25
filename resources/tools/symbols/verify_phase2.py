"""P2.10: Phase 2 exit check. Verifies deliverables, the Core build, the layout delta, the UnNames
table and the skeleton counts.

Usage: python resources/tools/symbols/verify_phase2.py [--no-build]   (exit 0 = PASS)
       python resources/tools/symbols/verify_phase2.py retail build/<dir>/layout_probe.txt
           retail section only: `gen_layout_probe.py compare` and `xcheck_sdk_layout.py` on the probe; fails on any
           contract-type mismatch of either (the other rows are reported, not failed)
"""
import csv
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
DOCS = REPO / "resources" / "docs"
SYM = DOCS / "symbols"
TYPES = DOCS / "types"
SRC = REPO / "source" / "Development" / "Src"
RESULTS: list[tuple[str, bool, str]] = []
VSDEVCMD = r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"


def check(task: str, ok: bool, detail: str = "") -> None:
    RESULTS.append((task, ok, detail))


def exists(task: str, *paths: Path) -> bool:
    missing = [p for p in paths if not p.exists()]
    check(task, not missing, "missing: " + ", ".join(str(p.relative_to(REPO)) for p in missing) if missing else "all files present")
    return not missing


def read_csv(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def build(preset: str, target: str) -> tuple[bool, str]:
    cmd = f'call "{VSDEVCMD}" -arch=x86 -host_arch=x64 -no_logo && cd /d "{REPO}" && cmake --preset {preset} && cmake --build build\\{preset} --target {target}'
    r = subprocess.run(["cmd", "/c", cmd], capture_output=True, text=True, errors="replace")
    errors = [l for l in r.stdout.splitlines() + r.stderr.splitlines() if " error C" in l or "error LNK" in l or "CMake Error" in l]
    return r.returncode == 0, errors[0][:200] if errors else f"exit {r.returncode}"


def retail(probe: Path) -> None:
    if not probe.exists():
        check("retail probe", False, f"missing: {probe}")
        return
    for task, script in (("retail compare (2012 PDB / retail sizes)", REPO / "resources/tools/symbols/gen_layout_probe.py"),
                         ("retail xcheck (SDK dump offsets)", REPO / "resources/tools/sdk/xcheck_sdk_layout.py")):
        args = ["compare"] if script.name == "gen_layout_probe.py" else []
        r = subprocess.run([sys.executable, str(script), *args, str(probe)], capture_output=True, text=True, errors="replace")
        summary = next((l for l in reversed(r.stdout.splitlines()) if "contract_mismatches=" in l), (r.stdout + r.stderr).strip()[-200:])
        check(task, r.returncode == 0 and "contract_mismatches=0" in summary, summary)


def main(argv: list[str]) -> int:
    if len(argv) > 1 and argv[1] == "retail":
        if len(argv) < 3:
            print(__doc__)
            return 2
        retail(Path(argv[2]) if Path(argv[2]).is_absolute() else REPO / argv[2])
        return report()
    no_build = "--no-build" in argv

    # P2.1
    if exists("P2.1 xref", DOCS / "reference_xref.csv", REPO / "resources/tools/symbols/xref_reference.py"):
        rows = read_csv(DOCS / "reference_xref.csv")
        core = [r for r in rows if r["module"] == "core" and r["status"] in ("reference", "missing")]
        share = 100.0 * sum(1 for r in core if r["status"] == "reference") / max(len(core), 1)
        check("P2.1 rows and Core share", len(rows) == 66394 and share >= 60.0, f"rows={len(rows)} core reference share={share:.0f}%")

    # P2.2
    if exists("P2.2 Core import", SRC / "Core/Inc/Core.h", SRC / "Core/Src/UnObj.cpp", SRC / "Core/Sources.cmake", DOCS / "reference_import.md"):
        stubs = [SRC / "Core/Src/bspatch/bspatch.cpp", SRC / "Core/Inc/Pool.h"]
        check("P2.2 Arkane stubs", all(p.exists() for p in stubs), ", ".join(p.name for p in stubs))

    # P2.3
    exists("P2.3 cmake", REPO / "cmake/DishonoredDefines.cmake", REPO / "cmake/DishonoredModule.cmake", REPO / "cmake/ReferenceExternals.cmake")

    # P2.4
    if not no_build:
        for preset in ("x86-debug", "x86-release"):
            ok, detail = build(preset, "Core")
            check(f"P2.4 Core builds ({preset})", ok, detail)
    exists("P2.4 porting notes", DOCS / "porting_notes.md")

    # P2.5
    if exists("P2.5 layout files", SRC / "Core/Inc/DishonoredLayouts.h", REPO / "source/Tests/LayoutProbe/probe_Core.cpp", TYPES / "reference_layout_delta.md"):
        if not no_build:
            ok, detail = build("x86-debug", "LayoutProbe")
            check("P2.5 LayoutProbe builds", ok, detail)
        text = (TYPES / "reference_layout_delta.md").read_text(encoding="utf-8")
        m = re.search(r"Contract types \((\d+) probed, (\d+) mismatching\)", text)
        check("P2.5 contract types match", bool(m) and m.group(2) == "0", m.group(0) if m else "no contract summary")
    exists("P2.5b member delta", TYPES / "reference_member_delta.md")

    # P2.6
    if exists("P2.6 versions", SYM / "package_summary.md", SRC / "Core/Src/UnObjVer.cpp", SRC / "Core/Inc/UnNames.h"):
        ver = (SRC / "Core/Src/UnObjVer.cpp").read_text(encoding="utf-8", errors="replace")
        pinned = "GPackageFileLicenseeVersion" in ver and re.search(r"GPackageFileLicenseeVersion\s*=\s*30", ver) and re.search(r"GPackageFileVersion\s*=\s*801", ver)
        check("P2.6 UnObjVer pinned", bool(pinned), "801 / licensee 30" if pinned else "not pinned to Dishonored's package versions")
        r = subprocess.run([sys.executable, str(REPO / "resources/tools/symbols/gen_unnames.py"), "--check"], capture_output=True, text=True)
        check("P2.6 UnNames matches csv", r.returncode == 0, r.stdout.strip().splitlines()[-1] if r.stdout.strip() else r.stderr.strip()[:200])

    # P2.7
    if exists("P2.7 opcodes and natives", SYM / "opcodes.md", SYM / "natives.csv"):
        text = (SYM / "opcodes.md").read_text(encoding="utf-8")
        m = re.search(r"(\d+) slots, (\d+) real differences", text)
        check("P2.7 opcodes", bool(m), m.group(0) if m else "no summary")

    # P2.8
    if exists("P2.8 licensee inventory", SYM / "licensee_branches.csv", SYM / "licensee_branches.md"):
        rows = read_csv(SYM / "licensee_branches.csv")
        lic = [int(r["value"]) for r in rows if r["family"] == "licensee"]
        check("P2.8 thresholds <= 30", bool(lic) and max(lic) <= 30, f"{len(lic)} licensee comparisons, max threshold {max(lic) if lic else None}")

    # P2.9
    counts = {m: sum(1 for p in (SRC / m).rglob("*") if p.is_file()) for m in ("DishonoredGame", "AkAudio", "DisJobs")}
    check("P2.9 skeletons", counts.get("DishonoredGame", 0) >= 1000 and counts.get("AkAudio", 0) >= 6 and counts.get("DisJobs", 0) >= 6, str(counts))

    # P2.10
    exists("P2.10 tracking", DOCS / "PHASE2.md", DOCS / "progress.md")
    return report()


def report() -> int:
    width = max(len(t) for t, _, _ in RESULTS)
    failed = 0
    for task, ok, detail in RESULTS:
        failed += not ok
        print(f"{'PASS' if ok else 'FAIL'}  {task.ljust(width)}  {detail}")
    print(f"\n{len(RESULTS) - failed}/{len(RESULTS)} checks passed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
