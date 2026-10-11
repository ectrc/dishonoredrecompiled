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
  newgame     NO -startmap: -playdefaults boots the shipped configuration, the game's own main menu comes
              up, -fmmenu reports how long it stays open and -finewgame issues the menu's own New Game
              command; asserts the mission is reached (l_tower_p) and the opening scene is populated
  options     NO -startmap either: -fmdrive opens Options off that menu and switches tabs on the user's
              own schedule, and asserts the process is still alive afterwards with no new AS2 errors
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
STAGES = ["build", "coresmoke", "layout", "nullrhi", "d3d9", "inputtest", "newgame", "options"]

# DISHONORED(written): agent FM. Every stage above `newgame` drives a map directly with -startmap, so
# nothing in this harness ever pressed New Game or opened Options, and two user-visible regressions went
# through a green gate because of it: wave 17's b540be6 closed the front end's own main menu one second
# after it opened - New Game unreachable from the shipped executable - and passed 37 ok / 0 failed on a
# clean checkout of its own commit, and the Options crash this wave exists for was not seen either.
# `newgame` and `options` boot the configuration the user plays: no -startmap, the play-defaults table
# from Core/Src/UnMisc.cpp turned on with -playdefaults, and the game's own menu on screen.
#
# A hard crash was invisible to this harness for a second reason as well. The three `*_criticals` metrics
# matched the literal `Critical: `, which is what appError logs through NAME_Critical; an access
# violation reaches the log through the structured-exception handler as `=== Critical error: ===`
# (UnMisc.cpp:1289, UnOutputDevices.cpp:756) and carries no `Critical: ` line at all. The user's crash log
# has `Critical: ` zero times. CRITICAL_RE matches both forms, which is why every *_criticals metric uses it.
CRITICAL_RE = r"Critical error:|Critical: "

# Throughput metrics: what the game got through in a fixed wall-clock window, so they measure the
# machine as much as the build. Every one of them has failed here on an unchanged binary while
# another agent was running its own game, and every one cleared on a re-run when the machine was
# quiet: d3d9_frames read 510, 900 and 990 against a bound of 1000 and then 2040-2370 minutes later;
# inputtest_moved read 662.2 and then 1074.5 on the SAME binary (agents EM, ES, EQ and the
# coordinator, all in one wave). A stage whose only failures are in this set is therefore re-run once
# before it is believed, and both numbers are printed - the re-run is the verdict, the first attempt
# stays visible. A real regression fails both times; this only removes the false alarm, and --no-load-retry
# turns it off when you want the raw single measurement.
#
# The bounds themselves are deliberately not relaxed. They are cliff detectors - d3d9_frames catches a
# render loop that stopped, inputtest_moved catches a pawn that cannot walk - and a bound loose enough
# never to trip under load would not catch the cliff either.
#
# DISHONORED(written): agent FM. The two menu stages are in the set for a different reason than
# throughput: they are driven on the WORLD clock (-finewgame, -fmdrive, -apshottime all key off
# GWorld->GetTimeSeconds()) but killed on the WALL clock by build_and_smoke.py's --timeout, and
# d3d9_startup_seconds has read 3.4 s quiet and 141.9 s under another agent's build. A machine busy
# enough to spend two minutes in startup does not reach world time 22 inside the window, which is a
# measurement of the machine and not of the build. A real break fails both attempts.
LOAD_SENSITIVE = {
    "d3d9_frames", "d3d9_draws_per_frame", "d3d9_visible_prims",
    "inputtest_moved", "inputtest_peak_speed",
    "newgame_on_mission", "newgame_pawns", "newgame_menu_open_seconds",
    "options_steps", "options_alive",
}


def load_sensitive_failures(report: "Report", mark: int) -> list[tuple[str, object]]:
    """The rows this stage just added that failed, when ALL of its failures are load-sensitive.

    If anything outside the set failed too, the stage has a real problem and re-running it would only
    hide the load-sensitive half of a genuine failure, so this returns nothing.
    """
    fails = [(n, m) for _, n, m, _, _, st in report.rows[mark:] if st == "FAIL"]
    if not fails or any(n not in LOAD_SENSITIVE for n, _ in fails):
        return []
    return fails
MAP = "L_Tower_P"
RENDER_ARGS = "-windowed -ResX=1280 -ResY=720 -nomovie"
# DISHONORED(written): agent FM. -nodishud keeps the in-game HUD movie out of the three stages whose
# bounds were calibrated with no movie open. The shipped executable now opens it by default, because
# retail opens it unconditionally out of UDisGlobalUIManager::Init and _root.blackStripes_mc lives in
# it (disgfxmovieplayerhud.cpp). The HUD-open path is covered by the `newgame` stage, which passes
# -playdefaults and no -nodishud, so nothing is lost by suppressing it here and three tight thresholds
# - d3d9_frames, d3d9_draws_per_frame, texture_census - keep meaning what they were measured against.
NO_HUD = "-nodishud"

# DISHONORED(written): agent FM. The `options` stage schedule, read step for step off the user's own
# crash log (D:/RecompileDishonored/Dishonored_Latest2026/DishonoredGame/Logs/play_live.log). What that
# log shows is NOT what the plan assumed. Two OnTabSelected in one frame and InitScrollBar twice are
# both NORMAL in this build - they happen at 17.47, 21.64 and 31.98 with no crash. The step the process
# did not survive is a change of the TOP-LEVEL tab made AFTER a sub tab had been opened: at 33.46 the
# sub-tab row was already up (`_subTabs_mc._tabs_mc._bOpened = true`) and the new category had a
# different number of sub-categories (`_subCategoriesList.length : 3` where the first selection saw 2),
# so _options_mc is re-attached under a mask that is still installed on the old _list_mc. Hence the
# order here: the screen, a sub tab (which builds the scroll bar and installs the mask), then the
# top-level tab change. The times are world time and generous: nothing before 8 s has a menu at all.
OPTIONS_SCRIPT = ";".join([
    "8:start",
    "14:fill",
    "15:invoke=_root.mainMenu_mc.Close()",
    "16:invoke=_root.optionsMenu_mc.SetMenu()",
    "20:invoke=_root.optionsMenu_mc._subTabs_mc._tabs_mc.OnTabSelected(1)",
    "24:invoke=_root.optionsMenu_mc._tabs_mc.OnTabClicked(1)",
])

# name -> (stage, regex, group count, aggregator). "count" counts matching lines, "distinct" counts distinct
# first groups, everything else aggregates the numeric groups of the matching lines.
LOG_METRICS = {
    "startup_seconds":       ("nullrhi",   r"Initial startup: ([\d.]+)s", "last"),
    "nullrhi_criticals":     ("nullrhi",   CRITICAL_RE, "count"),
    "nullrhi_level_seconds": ("nullrhi",   r"Finished loading level: ([\d.]+)", "last"),
    "d3d9_startup_seconds":  ("d3d9",      r"Initial startup: ([\d.]+)s", "last"),
    "d3d9_criticals":        ("d3d9",      CRITICAL_RE, "count"),
    "d3d9_frames":           ("d3d9",      r"scene rendered \((\d+) so far", "max"),
    "d3d9_draws_per_frame":  ("d3d9",      r"draw lists (\d+)/(\d+) drawn", "max0"),
    "d3d9_draw_elements":    ("d3d9",      r"draw lists (\d+)/(\d+) drawn", "max1"),
    "d3d9_visible_prims":    ("d3d9",      r"(\d+) visible \(\d+ static", "max"),
    "texture_census":        ("d3d9",      r"DISHONORED\(bringup\): texture census[^0-9]*(\d+)", "max"),
    "touch_census":          ("inputtest", r"DISHONORED\(bringup\): distouch\s+[\d.]+s\s+[^:]*: touch begin (\d+)", "max"),
    "sequence_census":       ("inputtest", r"DISHONORED\(bringup\): kismet census:.*?ops executed (\d+)", "max"),
    "inputtest_moved":       ("inputtest", r"inputtest moved ([\d.]+) turned", "max"),
    "inputtest_peak_speed":  ("inputtest", r"inputtest moved .*peak 2D speed ([\d.]+)", "max"),
    "physics_actors":        ("inputtest", r"PhysX (?:scene: )?(\d+) actors, (\d+) static shapes", "max0"),
    "physics_static_shapes": ("inputtest", r"PhysX (?:scene: )?(\d+) actors, (\d+) static shapes", "max1"),
    "unported_natives":      ("d3d9",      r"native not ported: (\S+)", "distinct"),
    # measured on the plain d3d9 run, not the inputtest stage: that stage now walks the pawn to every
    # volume, trigger and pickup in the map (-distouchprobe), which reaches gameplay a plain walk never
    # does and fires natives that are honestly still stubbed. Keeping this metric on the plain run keeps
    # its meaning "the walking path", which is what the bound of 0 was measured against.
    "probe_natives":         ("inputtest", r"native not ported: (\S+)", "distinct"),
    "inputtest_criticals":   ("inputtest", CRITICAL_RE, "count"),
    # DISHONORED(written): agent FM - the New Game stage. Three independent things have to hold, and they
    # fail separately:
    #   newgame_menu_open_seconds  the LAST world time at which the front end's own main-menu movie was
    #       still open (-fmmenu reports it twice a second). This is the one that catches b540be6: that
    #       commit closed the menu about a second after it opened, and -finewgame would not have noticed
    #       because it issues `ce ChangeLvl_StartNewGame` from the game thread without touching the menu.
    #   newgame_on_mission         lines of -fidiag's census that name l_tower_p: the hub sequence's
    #       SetPlayerTravelDestination -> CommitMapChange -> GotoPlayerTravelDestination actually landed.
    #   newgame_pawns              the opening scene is populated once it lands (agent FI measured 29).
    "newgame_menu_open_seconds": ("newgame", r"fmmenu \d+: t ([\d.]+) movie 1", "max"),
    "newgame_on_mission":        ("newgame", r"fidiag pawns \d+: t [\d.]+ map l_tower_p", "count"),
    "newgame_pawns":             ("newgame", r"fidiag pawns \d+: t [\d.]+ map l_tower_p, (\d+) pawns", "max"),
    "newgame_criticals":         ("newgame", CRITICAL_RE, "count"),
    # DISHONORED(written): agent FM - the Options stage. options_alive is the aliveness assertion and it is
    # written so that a DEAD process FAILS rather than SKIPS: the baseline requires it, so when the line is
    # absent measure() records -1 (see `unparsable`). A metric declared `optional` would have skipped on a
    # crash, which is the shape of the hole this stage exists to close.
    "options_steps":             ("options", r"fmdrive step \d+/\d+ t=[\d.]+ \S+ ok", "count"),
    "options_alive":             ("options", r"fmdrive alive: t ([\d.]+)", "max"),
    "options_as2_errors":        ("options", r"DISHONORED\(bringup\): AS2 error:", "count"),
    "options_criticals":         ("options", CRITICAL_RE, "count"),
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

    def add(self, stage: str, name: str, measured, op: str, expected, note: str = "",
            fatal: bool = False) -> None:
        # fatal: nothing was measured because the thing that measures it is absent - a missing
        # executable, not a missing log line. That is a failure; a skipped stage is not a passed one.
        if measured is None:
            status = "FAIL" if fatal else "SKIP"
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
    if how == "distinct":
        # DISHONORED(written): agent BF. A distinct counter counts lines that only exist when something is wrong, so no
        # match means zero, not "the log stopped carrying the metric". Before this, unported_natives reaching 0 read as
        # the -1 regression sentinel and could never report the number the plan asks for.
        return len({m.group(1) for m in matches})
    if not matches:
        return None
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
        report.add("coresmoke", "coresmoke_passed", None, ">=", expect.get("coresmoke_passed", {}).get("value"),
                   f"missing {exe} - build the CoreSmoke target", fatal=True)
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
        report.add("layout", "layout_types", None, "==", expect.get("layout_types", {}).get("value"),
                   f"missing {exe} - build the LayoutProbe target", fatal=True)
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
    log = smoke(args, out_dir, "nullrhi", f"-startmap={MAP} -startmapopen {NO_HUD}", args.nullrhi_timeout,
                "Initializing Engine...", ["Initial startup", "Finished loading level"])
    measure("nullrhi", log, report, expect)


def stage_d3d9(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    extra = f"-startmap={MAP} -startmapopen {RENDER_ARGS} {NO_HUD} " + " ".join(expect.get("_extra_args", {}).get("d3d9", []))
    log = smoke(args, out_dir, "d3d9", extra, args.d3d9_timeout, "Initializing Engine...", ["Initial startup", "scene census"])
    measure("d3d9", log, report, expect)


def stage_inputtest(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    extra = (f"-startmap={MAP} -startmapopen -inputtest -distrace {RENDER_ARGS} {NO_HUD} "
             + " ".join(expect.get("_extra_args", {}).get("inputtest", [])))
    log = smoke(args, out_dir, "inputtest", extra, args.inputtest_timeout, "Initializing Engine...",
                ["Initial startup", "inputtest moved"])
    measure("inputtest", log, report, expect)


def stage_newgame(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    """New Game from the game's own main menu, the way the user launches it: NO -startmap.

    -playdefaults turns on the table the shipped executable compiles in (Core/Src/UnMisc.cpp, appInit), so
    this stage's command line is the user's command line plus the log/ini isolation every harness run has
    and plus the two instruments. The other five stages never see -playdefaults, which is what keeps their
    calibrated thresholds meaning what they meant: no -gfxuimenu, no movie, in a stage that never had one.
    """
    extra = (f"-playdefaults -finewgame={args.newgame_at} -fidiag -fmmenu -fmdrive={args.newgame_script} "
             + " ".join(expect.get("_extra_args", {}).get("newgame", [])))
    log = smoke(args, out_dir, "newgame", extra, args.newgame_timeout, "Initializing Engine...",
                ["Initial startup", "fidiag pawns"])
    measure("newgame", log, report, expect)


def stage_options(args: argparse.Namespace, out_dir: Path, report: Report, expect: dict) -> None:
    """Open Options off the main menu and switch tabs the way the user's crash log shows.

    The schedule is the user's own, read off play_live.log: the Options screen, then a SUB tab (which is
    what builds the scroll bar and installs the mask on _options_mc._list_mc), then a change of the
    TOP-LEVEL tab, which is the step the process did not survive - 0.15 s after the second InitScrollBar
    and the setMask on a _list_mc whose owner the tab change had just re-attached.

    READ THIS BEFORE TRUSTING IT TO COVER THAT CRASH: it does not, and agent FM measured why rather than
    shipping a check that quietly passes. The AS2 trace this produces is identical to the user's, down to
    `InitScrollBar (385.699066162109, ...)` twice and `mask stage bounds 170.0,214.6..1151.7,666.2 px` -
    the frame the process died on - and the process lives. The difference is the ENTRY POINT: the user's
    tab switch was dispatched from inside GFx's own mouse handling (`AS2 trace: OnMouseUp (0, undefined)`
    is the line before it, same hundredth of a second), re-entrant with respect to the runtime's advance
    and display state, where GFxMovieView::Invoke enters ActionScript at a clean point between advances.
    Invoking TabsHandler::OnTabClicked, the button's own handler, is not close enough either. What this
    stage IS: the Options screen built through the screen's own entry point, every AS2 path on it
    exercised, the error count watched, and the process asserted alive afterwards.
    """
    extra = (f"-playdefaults -fmdrive={args.options_script} -fmmenu "
             + " ".join(expect.get("_extra_args", {}).get("options", [])))
    log = smoke(args, out_dir, "options", extra, args.options_timeout, "Initializing Engine...",
                ["Initial startup", "fmdrive alive"])
    measure("options", log, report, expect)


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
    parser.add_argument("--exe-name", default=None, help="default: derived from --build-dir")
    parser.add_argument("--log-prefix", default=None, help="default: derived from --build-dir")
    parser.add_argument("--baseline", type=Path, default=BASELINE)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--only", default="", help="comma list of stages: " + ",".join(STAGES))
    parser.add_argument("--update-baseline", action="store_true")
    parser.add_argument("--write-deltas", action="store_true", help="let xcheck_sdk_layout.py keep its rewrite of the shared delta doc")
    parser.add_argument("--game-extra-args", default="", help="appended to every game run (used to prove a broken counter is caught)")
    parser.add_argument("--nullrhi-timeout", type=float, default=180.0)
    parser.add_argument("--d3d9-timeout", type=float, default=90.0)
    parser.add_argument("--inputtest-timeout", type=float, default=150.0)
    # DISHONORED(written): agent FM. These two stages boot through the front end and then wait on the WORLD
    # clock, so their wall-clock budget has to cover a loaded machine's startup (d3d9_startup_seconds has
    # read 141.9 s) plus the schedule plus the map change. They are the two longest stages in the harness.
    parser.add_argument("--newgame-timeout", type=float, default=300.0)
    parser.add_argument("--newgame-at", type=float, default=22.0,
                        help="world time at which -finewgame issues `ce ChangeLvl_StartNewGame`")
    parser.add_argument("--newgame-script", default="8:start",
                        help="-fmdrive schedule for the newgame stage: dismiss the start screen so the menu proper opens")
    parser.add_argument("--options-timeout", type=float, default=240.0)
    parser.add_argument("--options-script", default=OPTIONS_SCRIPT,
                        help="-fmdrive schedule for the options stage: <worldtime>:<step>[;...]")
    parser.add_argument("--no-load-retry", action="store_true",
                        help="do not re-run a stage whose only failures are load-sensitive (see LOAD_SENSITIVE)")
    parser.add_argument("--list", action="store_true", help="print the stages and their metrics and exit")
    args = parser.parse_args(argv[1:])
    # DISHONORED(written): the exe name and log prefix name the staged exe and the log files in the shared
    # retail install, so two runs sharing them collide: the second cannot stage and every stage then
    # measures an absent log as -1 (agent BE saw the PermissionError, the coordinator saw the silent form).
    if args.exe_name is None or args.log_prefix is None:
        tag = re.sub(r"[^A-Za-z0-9]+", "_", str(args.build_dir)).strip("_")[-40:] or "regression"
        args.exe_name = args.exe_name or "DishonoredGame_rg_%s.exe" % tag
        args.log_prefix = args.log_prefix or "regression_%s" % tag
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

    def run_stage(stage: str) -> None:
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
        elif stage == "newgame":
            stage_newgame(args, out_dir, report, expect)
        elif stage == "options":
            stage_options(args, out_dir, report, expect)

    for stage in wanted:
        print(f"[{stage}]", flush=True)
        mark = len(report.rows)
        run_stage(stage)
        failed = load_sensitive_failures(report, mark)
        if failed and not args.no_load_retry:
            first = ", ".join(f"{n}={v}" for n, v in failed)
            print(f"  RETRY {stage}: {first} - load-sensitive, re-running the stage once (§ LOAD_SENSITIVE)",
                  flush=True)
            del report.rows[mark:]
            run_stage(stage)
            print(f"  (first attempt of {stage} measured {first}; the numbers above are the re-run)",
                  flush=True)

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
