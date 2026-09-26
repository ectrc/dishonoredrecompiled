"""One command that rebuilds the tree and re-measures every number the waves so far have earned.

Each check is a named metric with an expected value and a comparison operator held in
`resources/docs/regression_baseline.json`, so tightening a bound or adding a counter is a data change, not a
code change. The tool exits non-zero as soon as one metric regresses, which is what makes it usable as the
coordinator's merge gate and as a CI step.

    python resources/tools/run_regression.py --build-dir build/agentAX            (build + every stage)
    python resources/tools/run_regression.py --build-dir build/agentAX --no-build
    python resources/tools/run_regression.py --no-build --only d3d9,inputtest     (one or more stages)
    python resources/tools/run_regression.py --no-build --update-baseline         (rewrite the expectations)
    python resources/tools/run_regression.py --list                               (stages and metrics)

From a snapshot worktree, run **this tree's** copy with the snapshot's build directory and --no-build:
`python resources/tools/run_regression.py --build-dir build/agent<X>_wt/build/<dir> --no-build`. A worktree has no
resources/docs/types data (it is generated, not committed), so its own copy of the layout tools cannot run there.

Stages, in order
  build       resources/build-release.cmd for DishonoredGame, CoreSmoke and LayoutProbe into --build-dir
  coresmoke   <build-dir>/Binaries/Win32/CoreSmoke.exe from the repo root            -> passed / failed
  layout      LayoutProbe.exe -> <build-dir>/layout_probe.txt, then sdk/xcheck_sdk_layout.py,
              symbols/gen_layout_probe.py compare and symbols/verify_phase2.py retail on it. The shared
              resources/docs/types/retail_sdk_delta.md that xcheck rewrites is restored afterwards unless
              --write-deltas is given: a verification run must not dirty a committed document.
  nullrhi     the milestone run (null RHI, `Initializing Engine...` -> `Finished loading level`)
  d3d9        the rendered run of the first mission map (windowed 1280x720), frames and scene census
  inputtest   the same map with -inputtest -distrace: the distance the pawn walks, physics scene counts
Every game run goes through build_and_smoke.py with -forcelogflush and this agent's own --log-name /
--ini-dir / --exe-name, so it never collides with another agent's run.

Metrics are pulled out of Launch.log by the table in LOG_METRICS. A metric marked `optional` in the baseline
is SKIPPED when its line is absent instead of failing, which is how the census counters other packages add
(textures, touches, sequence ops) are wired in before their lines exist: once a line appears the metric is
measured on every later run, so a future wave cannot silently drop the counter. The contract for such a
census line is exactly what agent AP's scene census already does: `DISHONORED(bringup): <thing> census: <N> ...`,
whose first integer is the headline count. A metric is matched only on such a line, never on the command-line
echo at the top of the log (the switch that turns a census on would otherwise match its own metric).
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
RETAIL = Path(r"D:\RecompileDishonored\Dishonored_Latest2026")
LOGS = RETAIL / "DishonoredGame" / "Logs"
BASELINE = REPO / "resources" / "docs" / "regression_baseline.json"
SDK_DELTA = REPO / "resources" / "docs" / "types" / "retail_sdk_delta.md"
STAGES = ["build", "coresmoke", "layout", "nullrhi", "d3d9", "inputtest"]
MAP = "L_Tower_P"
RENDER_ARGS = "-windowed -ResX=1280 -ResY=720 -nomovie"

# name -> (stage, regex, group count, aggregator). "count" counts matching lines, "distinct" counts distinct
# first groups, everything else aggregates the numeric groups of the matching lines.
LOG_METRICS = {
    "startup_seconds":       ("nullrhi",   r"Initial startup: ([\d.]+)s", "last"),
    "nullrhi_criticals":     ("nullrhi",   r"Critical: ", "count"),
    "nullrhi_level_seconds": ("nullrhi",   r"Finished loading level: ([\d.]+)", "last"),
    "d3d9_startup_seconds":  ("d3d9",      r"Initial startup: ([\d.]+)s", "last"),
    "d3d9_criticals":        ("d3d9",      r"Critical: ", "count"),
    "d3d9_frames":           ("d3d9",      r"scene rendered \((\d+) so far", "max"),
    "d3d9_draws_per_frame":  ("d3d9",      r"draw lists (\d+)/(\d+) drawn", "max0"),
    "d3d9_draw_elements":    ("d3d9",      r"draw lists (\d+)/(\d+) drawn", "max1"),
    "d3d9_visible_prims":    ("d3d9",      r"(\d+) visible \(\d+ static", "max"),
    "texture_census":        ("d3d9",      r"DISHONORED\(bringup\): texture census[^0-9]*(\d+)", "max"),
    "touch_census":          ("inputtest", r"DISHONORED\(bringup\): (?:touch|distouch) census[^0-9]*(\d+)", "max"),
    "sequence_census":       ("inputtest", r"DISHONORED\(bringup\): (?:sequence|seqop|kismet) census[^0-9]*(\d+)", "max"),
    "inputtest_moved":       ("inputtest", r"inputtest moved ([\d.]+) turned", "max"),
    "inputtest_peak_speed":  ("inputtest", r"inputtest moved .*peak 2D speed ([\d.]+)", "max"),
    "physics_actors":        ("inputtest", r"PhysX (?:scene: )?(\d+) actors, (\d+) static shapes", "max0"),
    "physics_static_shapes": ("inputtest", r"PhysX (?:scene: )?(\d+) actors, (\d+) static shapes", "max1"),
    "unported_natives":      ("inputtest", r"native not ported: (\S+)", "distinct"),
    "inputtest_criticals":   ("inputtest", r"Critical: ", "count"),
}
OPS = {">=": lambda a, b: a >= b, "<=": lambda a, b: a <= b, "==": lambda a, b: a == b, ">": lambda a, b: a > b}


def unparsable(value, rule: dict):
    """A metric the baseline requires but this run could not measure fails every operator; one the baseline does
    not mention stays None and is reported as skipped."""
    if value is None and rule.get("value") is not None and not rule.get("optional"):
        return -1
    return value


class Report:
    """Collects one row per metric and decides the exit code."""

    def __init__(self):
        self.rows: list[tuple[str, str, object, str, object, str]] = []

    def add(self, stage: str, name: str, measured, op: str, expected, note: str = "") -> None:
        if measured is None:
            status = "SKIP"
        elif expected is None:
            status = "ok"
        else:
            status = "ok" if OPS[op](measured, expected) else "FAIL"
        self.rows.append((stage, name, measured, op, expected, status))
        shown = f"{measured:.1f}" if isinstance(measured, float) else measured
        want = "" if expected is None else f" (wanted {op} {expected})"
        print(f"  {status:4} {name:24} {shown}{want}{'  ' + note if note else ''}", flush=True)

    @property
    def failed(self) -> list[tuple]:
        return [r for r in self.rows if r[5] == "FAIL"]

    @property
    def skipped(self) -> list[tuple]:
        return [r for r in self.rows if r[5] == "SKIP"]


def aggregate(text: str, pattern: str, how: str):
    matches = list(re.finditer(pattern, text))
    if how == "count":
        return len(matches)
    if not matches:
        return None
    if how == "distinct":
        return len({m.group(1) for m in matches})
    index = int(how[-1]) if how[-1].isdigit() else 0
    values = [float(m.group(index + 1)) for m in matches]
    value = values[-1] if how.startswith("last") else max(values)
    return value if "." in matches[0].group(index + 1) else int(value)


def measure(stage: str, log: Path, report: Report, expect: dict) -> None:
    text = log.read_text(encoding="utf-8", errors="replace") if log.is_file() else ""
    name = f"{stage}_log_lines"
    rule = expect.get(name, {})
    report.add(stage, name, text.count("\n"), rule.get("op", ">="), rule.get("value"),
               "" if text else f"no {log}: the run did not start")
    for name, (owner, pattern, how) in LOG_METRICS.items():
        if owner != stage:
            continue
        rule = expect.get(name, {})
        measured = aggregate(text, pattern, how)
        if measured is None and not rule.get("optional") and rule.get("value") is not None:
            measured = -1  # a metric the baseline requires and the log no longer carries is a regression, not a skip
        report.add(stage, name, measured, rule.get("op", ">="), rule.get("value"), rule.get("note", ""))


def run(cmd: list[str], out: Path, cwd: Path = REPO, env: dict | None = None) -> int:
    print("  $ " + " ".join(cmd), flush=True)
    with out.open("w", encoding="utf-8", errors="replace") as fh:
        return subprocess.run(cmd, cwd=cwd, stdout=fh, stderr=subprocess.STDOUT, env=env).returncode


def stage_build(args: argparse.Namespace, out_dir: Path, report: Report) -> None:
    env = dict(os.environ, BUILD_DIR=str(args.build_dir))
    for target in ("DishonoredGame", "CoreSmoke", "LayoutProbe"):
        log = out_dir / f"build_{target}.log"
        rc = run(["cmd", "/c", str(REPO / "resources" / "build-release.cmd"), target], log, env=env)
        errors = sum(1 for line in log.read_text(encoding="utf-8", errors="replace").splitlines()
                     if ": error " in line or ": fatal error " in line)
        report.add("build", f"build_{target}_errors", errors, "==", 0, f"exit {rc}")
        report.add("build", f"build_{target}_exit", rc, "==", 0)


def stage_coresmoke(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    exe = REPO / args.build_dir / "Binaries" / "Win32" / "CoreSmoke.exe"
    if not exe.is_file():
        report.add("coresmoke", "coresmoke_passed", None, ">=", expect.get("coresmoke_passed", {}).get("value"), f"missing {exe}")
        return
    log = out_dir / "coresmoke.txt"
    run([str(exe)], log)
    text = log.read_text(encoding="utf-8", errors="replace")
    m = re.search(r"(\d+) passed, (\d+) failed", text)
    for name, value in (("coresmoke_passed", int(m.group(1)) if m else None), ("coresmoke_failed", int(m.group(2)) if m else None)):
        rule = expect.get(name, {})
        report.add("coresmoke", name, unparsable(value, rule), rule.get("op", ">="), rule.get("value"),
                   "" if m else f"no summary line in {log}")


def stage_layout(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    exe = REPO / args.build_dir / "Binaries" / "Win32" / "LayoutProbe.exe"
    probe = REPO / args.build_dir / "layout_probe.txt"
    if not exe.is_file():
        report.add("layout", "layout_types", None, "==", expect.get("layout_types", {}).get("value"), f"missing {exe}")
        return
    # never through the shell: PowerShell's > writes a BOM and the probe parser would see it
    probe.write_bytes(subprocess.run([str(exe)], cwd=REPO, capture_output=True).stdout)
    saved = SDK_DELTA.read_bytes() if (SDK_DELTA.is_file() and not args.write_deltas) else None
    xcheck = out_dir / "xcheck.txt"
    run([sys.executable, str(REPO / "resources/tools/sdk/xcheck_sdk_layout.py"), str(probe)], xcheck)
    compare = out_dir / "compare.txt"
    run([sys.executable, str(REPO / "resources/tools/symbols/gen_layout_probe.py"), "compare", str(probe)], compare)
    verify = out_dir / "verify_phase2.txt"
    run([sys.executable, str(REPO / "resources/tools/symbols/verify_phase2.py"), "retail", str(probe)], verify)
    if saved is not None:
        SDK_DELTA.write_bytes(saved)
    values = {}
    m = re.search(r"types=(\d+) exact=(\d+) mismatching=(\d+) contract_mismatches=(\d+)", xcheck.read_text(errors="replace"))
    if m:
        values |= {"layout_types": int(m.group(1)), "layout_mismatching": int(m.group(3)), "layout_contract": int(m.group(4))}
    m = re.search(r"probed=(\d+) exact=(\d+) contract_mismatches=(\d+)", compare.read_text(errors="replace"))
    if m:
        values |= {"layout_probed": int(m.group(1)), "layout_compare_contract": int(m.group(3))}
    m = re.search(r"(\d+)/(\d+) checks passed", verify.read_text(errors="replace"))
    if m:
        values |= {"verify_phase2_passed": int(m.group(1)), "verify_phase2_total": int(m.group(2))}
    for name in ("layout_types", "layout_mismatching", "layout_contract", "layout_probed",
                 "layout_compare_contract", "verify_phase2_passed", "verify_phase2_total"):
        rule = expect.get(name, {})
        note = "" if name in values else "the tool printed no summary line: see the stage output"
        report.add("layout", name, unparsable(values.get(name), rule), rule.get("op", "=="), rule.get("value"), note)


def smoke(args: argparse.Namespace, out_dir: Path, stage: str, extra: str, timeout: float, milestone: str, expects: list[str]) -> Path:
    log_name = f"{args.log_prefix}_{stage}.log"
    log = LOGS / log_name
    log.unlink(missing_ok=True)  # a run that never starts must not be measured on the previous run's log
    cmd = [sys.executable, str(REPO / "resources/tools/build_and_smoke.py"),
           "--build-dir", str(args.build_dir), "--no-build", "--exe-name", args.exe_name,
           "--log-name", log_name, "--ini-dir", f"{args.build_dir}/config", "--retail", str(args.retail),
           "--rhi", "null" if stage == "nullrhi" else "d3d9", "--timeout", str(timeout),
           "--skip-native", "OnlineSubsystemPC", "--milestone", milestone]
    for e in expects:
        cmd += ["--expect", e]
    cmd.append(f"--extra-args=-forcelogflush {extra} {args.game_extra_args}".strip())
    run(cmd, out_dir / f"{stage}_smoke.txt", env=None)
    return log


def stage_nullrhi(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    log = smoke(args, out_dir, "nullrhi", f"-startmap={MAP} -startmapopen", args.nullrhi_timeout,
                "Initializing Engine...", ["Initial startup", "Finished loading level"])
    measure("nullrhi", log, report, expect)


def stage_d3d9(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    extra = f"-startmap={MAP} -startmapopen {RENDER_ARGS} " + " ".join(expect.get("_extra_args", {}).get("d3d9", []))
    log = smoke(args, out_dir, "d3d9", extra, args.d3d9_timeout, "Initializing Engine...", ["Initial startup", "scene census"])
    measure("d3d9", log, report, expect)


def stage_inputtest(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    extra = (f"-startmap={MAP} -startmapopen -inputtest -distrace {RENDER_ARGS} "
             + " ".join(expect.get("_extra_args", {}).get("inputtest", [])))
    log = smoke(args, out_dir, "inputtest", extra, args.inputtest_timeout, "Initializing Engine...",
                ["Initial startup", "inputtest moved"])
    measure("inputtest", log, report, expect)


def update_baseline(report: Report, expect: dict, path: Path) -> None:
    """Rewrites the measured numbers into the baseline, keeping each metric's operator and note."""
    for stage, name, measured, op, expected, status in report.rows:
        if measured is None or name.startswith("build_"):
            continue  # the build stage's own rows are always "0 errors, exit 0" and need no baseline entry
        rule = dict(expect.get(name, {}))
        rule["op"] = rule.get("op", op or ">=")
        rule["value"] = measured
        expect[name] = rule
    path.write_text(json.dumps(expect, indent=2) + "\n", encoding="utf-8")
    print(f"baseline rewritten: {path}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", default="build/agentAX")
    parser.add_argument("--retail", type=Path, default=RETAIL, help="the retail install the exe is staged into")
    parser.add_argument("--exe-name", default="DishonoredGame_AX.exe")
    parser.add_argument("--log-prefix", default="regression")
    parser.add_argument("--baseline", type=Path, default=BASELINE)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--only", default="", help="comma list of stages: " + ",".join(STAGES))
    parser.add_argument("--update-baseline", action="store_true")
    parser.add_argument("--write-deltas", action="store_true", help="let xcheck_sdk_layout.py keep its rewrite of the shared delta doc")
    parser.add_argument("--game-extra-args", default="", help="appended to every game run (used to prove a broken counter is caught)")
    parser.add_argument("--nullrhi-timeout", type=float, default=180.0)
    parser.add_argument("--d3d9-timeout", type=float, default=90.0)
    parser.add_argument("--inputtest-timeout", type=float, default=150.0)
    parser.add_argument("--list", action="store_true", help="print the stages and their metrics and exit")
    args = parser.parse_args(argv[1:])
    if args.list:
        for stage in STAGES:
            print(stage, "->", ", ".join(n for n, v in LOG_METRICS.items() if v[0] == stage) or "(not log-derived)")
        return 0

    expect = json.loads(args.baseline.read_text(encoding="utf-8")) if args.baseline.is_file() else {}
    wanted = [s for s in (args.only.split(",") if args.only else STAGES) if s]
    for stage in wanted:
        if stage not in STAGES:
            raise SystemExit(f"unknown stage {stage!r}; known: {', '.join(STAGES)}")
    if args.no_build and "build" in wanted:
        wanted.remove("build")
    out_dir = (REPO / args.build_dir).resolve() / "regression"
    out_dir.mkdir(parents=True, exist_ok=True)
    report = Report()
    started = time.time()
    for stage in wanted:
        print(f"[{stage}]", flush=True)
        if stage == "build":
            stage_build(args, out_dir, report)
        elif stage == "coresmoke":
            stage_coresmoke(args, out_dir, report, expect)
        elif stage == "layout":
            stage_layout(args, out_dir, report, expect)
        elif stage == "nullrhi":
            stage_nullrhi(args, out_dir, report, expect)
        elif stage == "d3d9":
            stage_d3d9(args, out_dir, report, expect)
        elif stage == "inputtest":
            stage_inputtest(args, out_dir, report, expect)

    summary = out_dir / "summary.txt"
    summary.write_text("".join(f"{s[5]}\t{s[0]}\t{s[1]}\t{s[2]}\t{s[3]}\t{s[4]}\n" for s in report.rows), encoding="utf-8")
    if args.update_baseline:
        update_baseline(report, expect, args.baseline)
    print(f"\n{len(report.rows) - len(report.failed) - len(report.skipped)} ok, {len(report.failed)} failed, "
          f"{len(report.skipped)} skipped, {time.time() - started:.0f}s ({summary})")
    for stage, name, measured, op, expected, _ in report.failed:
        print(f"  REGRESSION {stage}/{name}: {measured} (wanted {op} {expected})")
    return 1 if report.failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
