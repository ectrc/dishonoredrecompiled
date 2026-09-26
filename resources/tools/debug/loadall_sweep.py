"""The milestone 3 exit check: load every cooked package of the retail tree and verify every Wwise file package.

Two sweeps, one baseline file.

`upk`   drives agent AD's `-loadall=@<listfile>` switch (`Engine/Src/DishonoredLoadAll.cpp`) over every `.upk`
        under the retail cooked directories through `build_and_smoke.py`, with agent AX's `-loadallpurge` so the
        whole tree fits in one 32-bit process. The exe logs one line per package,
          DISHONORED(bringup): loadall <pkg>: <N> exports, <E> errors (<C> objects created, <T>s, <L> objects live)
        and only a linker abort (`appErrorf` -> object system shut down) ends the process early; this driver then
        restarts the sweep at the next package until the list is exhausted.

`pck`   the `.pck` files are **not** UE3 packages - they are Wwise 2012.1 file packages ("AKPK"), read by the
        AkAudio low-level IO, never by `UObject::LoadPackage`. Loading them with `-loadall` would only prove that
        the package search path does not resolve them. They are verified here by parsing the AKPK header: magic,
        version, the language string map and the three lookup tables (soundbanks, streamed files, externals), and
        checking that every entry's `startBlock * blockSize + fileSize` lies inside the file.

usage:
  python resources/tools/debug/loadall_sweep.py upk  --build-dir build/agentAX --exe-name DishonoredGame_AX.exe
                                                     --log-name agentAX_loadall.log --ini-dir build/agentAX/config
                                                     [--dirs CookedPCConsole] [--chunk 0] [--timeout 1800]
  python resources/tools/debug/loadall_sweep.py pck   [--dirs CookedPCConsole DLC/PCConsole/DLC05 ...]
  python resources/tools/debug/loadall_sweep.py report                 -> rewrite the baseline from the csv rows
Output: <build-dir>/loadall/<kind>_rows.csv (package,kind,exports,objects,seconds,live,errors,error) and
        resources/docs/loadall_baseline.{csv,md}. Exit code 1 when a package fails, 0 otherwise.
"""
import argparse
import csv
import re
import struct
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
RETAIL = Path(r"D:\RecompileDishonored\Dishonored_Latest2026")
GAME_DIR = RETAIL / "DishonoredGame"
LOGS = GAME_DIR / "Logs"
BASELINE_CSV = REPO / "resources" / "docs" / "loadall_baseline.csv"
BASELINE_MD = REPO / "resources" / "docs" / "loadall_baseline.md"
DEFAULT_DIRS = ["CookedPCConsole"]
ALL_DIRS = ["CookedPCConsole", "DLC/PCConsole/DLC05", "DLC/PCConsole/DLC06", "DLC/PCConsole/DLC07"]
LOADALL_RE = re.compile(r"loadall (\S+): (\d+) exports, (\d+) errors(.*)")
EXTRA_RE = re.compile(r"\((\d+) objects created, ([\d.]+)s\)")
PURGE_RE = re.compile(r"loadall purged (\S+): (\d+) objects live")
CRIT_RE = re.compile(r"Critical: (?:appError called: )?(.+)")
# an access violation is not an appErrorf: the log carries "=== Critical error: ===" and then "Fatal error!"
FATAL_RE = re.compile(r"=== Critical error: ===\s*\r?\n(.+)")
FIELDS = ["package", "kind", "exports", "objects", "seconds", "live", "errors", "error", "purge_errors", "purge_error"]


def content_files(dirs: list[str], suffix: str) -> list[Path]:
    out: list[Path] = []
    for rel in dirs:
        base = GAME_DIR / rel
        if not base.is_dir():
            raise SystemExit(f"no such content directory: {base}")
        out += sorted(p for p in base.iterdir() if p.is_file() and p.suffix.lower() == suffix)
    return out


def crash_text(text: str, rc: int) -> str:
    """The reason the process died, as the log gives it: an appErrorf message, or the unhandled-exception
    banner. Neither carries a stack; resources/tools/debug/dbgrun.py on the single package does."""
    crit = CRIT_RE.search(text)
    if crit:
        return crit.group(1).strip()[:300]
    fatal = FATAL_RE.search(text)
    if fatal:
        return f"{fatal.group(1).strip()[:200]} (unhandled exception, no appErrorf; run dbgrun.py on the package for the stack)"
    return f"the process died with no critical error line in the log (rc {rc})"


class Sweep:
    """One `-loadall` sweep: a package list, the runs it took, and the rows the log yielded."""

    def __init__(self, args: argparse.Namespace):
        self.args = args
        self.out_dir = (REPO / args.build_dir).resolve() / "loadall"
        self.out_dir.mkdir(parents=True, exist_ok=True)
        self.log = LOGS / args.log_name
        self.rows: list[dict] = []
        self.runs = 0

    def run_once(self, packages: list[str]) -> int:
        listfile = self.out_dir / "current.txt"
        listfile.write_text("\n".join(packages) + "\n", encoding="ascii")
        extra = f"-loadall=@{str(listfile).replace(chr(92), '/')} -forcelogflush"
        if not self.args.no_purge:
            extra += " -loadallpurge"
        cmd = [sys.executable, str(REPO / "resources/tools/build_and_smoke.py"),
               "--build-dir", self.args.build_dir, "--no-build", "--exe-name", self.args.exe_name,
               "--log-name", self.args.log_name, "--ini-dir", self.args.ini_dir, "--rhi", "null",
               "--timeout", str(self.args.timeout), "--skip-native", "OnlineSubsystemPC",
               "--milestone", "Initial startup", "--expect", "DISHONORED(bringup): loadall",
               f"--extra-args={extra}"]
        proc = subprocess.run(cmd, cwd=REPO, capture_output=True, text=True, errors="replace")
        (self.out_dir / f"run{self.runs}.txt").write_text(proc.stdout + proc.stderr, encoding="utf-8")
        return proc.returncode

    def parse_log(self, rc: int) -> list[dict]:
        """One row per package the run reported. A package with a load line but no "purged" line crashed in the
        teardown of its own objects, which is counted apart from a load error: the milestone 3 exit check is about
        the serializers, and the sweep records the teardown crash rather than hiding it."""
        text = self.log.read_text(encoding="utf-8", errors="replace") if self.log.is_file() else ""
        (self.out_dir / f"run{self.runs}.log").write_text(text, encoding="utf-8", errors="replace")
        fatal = crash_text(text, rc)
        purged = {m.group(1): int(m.group(2)) for m in PURGE_RE.finditer(text)}
        rows = []
        for m in LOADALL_RE.finditer(text):
            tail = m.group(4).strip()
            tail = tail[1:].strip() if tail.startswith(":") else tail  # ": <GErrorHist>" on a failure
            extra = EXTRA_RE.search(tail)
            rows.append({"package": m.group(1), "kind": "upk", "exports": int(m.group(2)),
                         "objects": int(extra.group(1)) if extra else 0,
                         "seconds": float(extra.group(2)) if extra else 0.0,
                         "live": purged.get(m.group(1), 0),
                         "errors": int(m.group(3)), "error": "" if extra else tail,
                         "purge_errors": 0, "purge_error": ""})
        if rows and not rows[-1]["errors"] and rows[-1]["package"] not in purged:
            rows[-1]["purge_errors"] = 1
            rows[-1]["purge_error"] = fatal
        return rows

    def sweep(self, packages: list[str]) -> None:
        remaining = list(packages)
        while remaining:
            self.runs += 1
            rc = self.run_once(remaining)
            rows = self.parse_log(rc)
            if not rows:
                text = self.log.read_text(errors="replace") if self.log.is_file() else ""
                rows = [{"package": remaining[0], "kind": "upk", "exports": 0, "objects": 0, "seconds": 0.0, "live": 0,
                         "errors": 1, "error": crash_text(text, rc), "purge_errors": 0, "purge_error": ""}]
            done = {r["package"] for r in rows}
            self.rows += rows
            last = max(i for i, p in enumerate(remaining) if p in done)
            print(f"run {self.runs}: rc={rc}, {len(rows)} package lines, {len(remaining) - last - 1} remaining", flush=True)
            remaining = remaining[last + 1:]


def sweep_upk(args: argparse.Namespace) -> list[dict]:
    files = content_files(args.dirs, ".upk")
    packages = [p.stem for p in files]
    print(f"{len(packages)} .upk packages under {', '.join(args.dirs)}")
    sweep = Sweep(args)
    rows: list[dict] = []
    chunk = args.chunk if args.chunk > 0 else len(packages)
    for start in range(0, len(packages), chunk):
        sweep.sweep(packages[start:start + chunk])
    rows = sweep.rows
    write_rows(sweep.out_dir / "upk_rows.csv", rows)
    return rows


def akpk_check(path: Path) -> dict:
    """Parses a Wwise 2012.1 AKPK file package header and bounds-checks every lookup-table entry.

    AKPK: 'AKPK', headerSize, then headerSize bytes = version, the four section sizes (language string map,
    soundbank LUT, streamed-file LUT, external LUT) and those four sections. Each LUT is a count followed by
    fixed-width entries: 20 bytes (fileId, blockSize, fileSize, startBlock, languageId) for banks and streams,
    24 for externals (a 64-bit id). A contained file lives at startBlock * blockSize."""
    size = path.stat().st_size
    with path.open("rb") as fh:
        data = fh.read(4 << 20)
    row = {"package": path.stem, "kind": "pck", "exports": 0, "objects": 0, "seconds": 0.0, "live": 0, "errors": 0,
           "error": "", "purge_errors": 0, "purge_error": ""}

    def fail(text: str) -> dict:
        row["errors"] = 1
        row["error"] = text
        return row

    if len(data) < 28 or data[:4] != b"AKPK":
        return fail(f"not an AKPK file package (magic {data[:4]!r})")
    header_size, version, lang_size, bank_size, stream_size, ext_size = struct.unpack_from("<6I", data, 4)
    if version != 1:
        return fail(f"AKPK version {version}, expected 1")
    if 8 + header_size > size:
        return fail(f"header of {header_size} bytes does not fit in a {size}-byte file")
    if 20 + lang_size + bank_size + stream_size + ext_size != header_size:
        return fail(f"section sizes {lang_size}/{bank_size}/{stream_size}/{ext_size} do not add up to a {header_size}-byte header")
    langs = struct.unpack_from("<I", data, 28)[0]
    base = 28 + lang_size
    entries = 0
    for name, lut_size, width, value_offset in (("soundbank", bank_size, 20, 4), ("streamed file", stream_size, 20, 4),
                                                ("external", ext_size, 24, 8)):
        if lut_size == 0:
            continue
        count = struct.unpack_from("<I", data, base)[0]
        if 4 + count * width > lut_size:
            return fail(f"{name} table of {count} x {width}-byte entries does not fit in {lut_size} bytes")
        for i in range(count):
            block_size, file_size, start_block = struct.unpack_from("<3I", data, base + 4 + i * width + value_offset)
            end = start_block * block_size + file_size
            if file_size == 0 or end > size:
                return fail(f"{name} entry {i}/{count} spans {start_block * block_size}..{end}, file is {size} bytes")
        entries += count
        base += lut_size
    row["exports"] = entries
    row["objects"] = entries
    row["live"] = langs
    return row


def sweep_pck(args: argparse.Namespace) -> list[dict]:
    files = content_files(args.dirs, ".pck")
    print(f"{len(files)} .pck Wwise file packages under {', '.join(args.dirs)}")
    rows = []
    for path in files:
        rows.append(akpk_check(path))
    bad = [r for r in rows if r["errors"]]
    print(f"pck: {len(rows)} packages, {sum(r['exports'] for r in rows)} contained files, {len(bad)} with errors")
    for r in bad:
        print(f"  {r['package']}: {r['error']}")
    out = (REPO / args.build_dir).resolve() / "loadall"
    out.mkdir(parents=True, exist_ok=True)
    write_rows(out / "pck_rows.csv", rows)
    return rows


def write_rows(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {path} ({len(rows)} rows)")


def read_rows(path: Path) -> list[dict]:
    if not path.is_file():
        return []
    with path.open(newline="", encoding="utf-8") as fh:
        return list(csv.DictReader(fh))


def report(args: argparse.Namespace) -> int:
    """Merges the per-kind row files into the committed baseline (csv + md summary)."""
    out = (REPO / args.build_dir).resolve() / "loadall"
    rows = read_rows(out / "upk_rows.csv") + read_rows(out / "pck_rows.csv")
    if not rows:
        raise SystemExit(f"no rows under {out}: run the upk and pck sweeps first")
    with BASELINE_CSV.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    upk = [r for r in rows if r["kind"] == "upk"]
    pck = [r for r in rows if r["kind"] == "pck"]
    bad = [r for r in rows if int(r["errors"])]
    purge = [r for r in rows if int(r["purge_errors"])]
    lines = ["| kind | packages | objects | load errors | teardown crashes |", "|---|---:|---:|---:|---:|",
             f"| `.upk` (LoadPackage) | {len(upk)} | {sum(int(r['objects']) for r in upk)} | "
             f"{sum(int(r['errors']) for r in upk)} | {sum(int(r['purge_errors']) for r in upk)} |",
             f"| `.pck` (AKPK header) | {len(pck)} | {sum(int(r['objects']) for r in pck)} | "
             f"{sum(int(r['errors']) for r in pck)} | - |", ""]
    if bad:
        lines += ["| package | load error |", "|---|---|"] + [f"| `{r['package']}` | {r['error'][:300]} |" for r in bad] + [""]
    if purge:
        lines += ["| package | teardown crash |", "|---|---|"] + [f"| `{r['package']}` | {r['purge_error'][:200]} |" for r in purge]
    print("\n".join(lines))
    print(f"baseline rows: {BASELINE_CSV}")
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("kind", choices=["upk", "pck", "report"])
    parser.add_argument("--build-dir", default="build/agentAX")
    parser.add_argument("--exe-name", default="DishonoredGame_AX.exe")
    parser.add_argument("--log-name", default="agentAX_loadall.log")
    parser.add_argument("--ini-dir", default="build/agentAX/config")
    parser.add_argument("--dirs", nargs="+", default=None, help=f"content directories under DishonoredGame (default {DEFAULT_DIRS}; 'all' = {ALL_DIRS})")
    parser.add_argument("--chunk", type=int, default=0, help="packages per process (0 = one process for the whole list)")
    parser.add_argument("--timeout", type=float, default=1800.0)
    parser.add_argument("--no-purge", action="store_true", help="keep agent AD's RF_Standalone keep flags (exhausts the heap over a few dozen packages)")
    args = parser.parse_args(argv[1:])
    if args.dirs == ["all"]:
        args.dirs = ALL_DIRS
    elif args.dirs is None:
        args.dirs = DEFAULT_DIRS
    start = time.time()
    if args.kind == "report":
        return report(args)
    rows = sweep_upk(args) if args.kind == "upk" else sweep_pck(args)
    bad = [r for r in rows if int(r["errors"])]
    purge = [r for r in rows if int(r["purge_errors"])]
    print(f"{args.kind}: {len(rows)} packages, {len(bad)} load errors, {len(purge)} teardown crashes, {time.time() - start:.0f}s")
    for r in bad:
        print(f"  LOAD  {r['package']}: {str(r['error'])[:300]}")
    for r in purge:
        print(f"  PURGE {r['package']}: {str(r['purge_error'])[:200]}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
