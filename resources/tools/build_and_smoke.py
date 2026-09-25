"""Build DishonoredGame.exe, stage it next to the retail content, run it and diff the normalized
Launch.log against the golden log's prefix (milestone 1: `Init: Object subsystem initialized`).

Steps
  1. cmake --build <build-dir> --target DishonoredGame (inside VsDevCmd x86), unless --no-build
  2. resources/tools/stage_retail.py -> copies our DishonoredGame.exe into the retail Binaries\\Win32 (no junctions,
     nothing else touched; see the incident note in that script)
  3. run <retail>\\Binaries\\Win32\\DishonoredGame.exe -log -nosteam -seekfreeloadingpcconsole
     from that directory with a timeout (the process is killed when it expires)
  4. normalize <retail>\\DishonoredGame\\Logs\\Launch.log (normalize_log.py rules) and the
     golden log, cut the golden one at --milestone, print a unified diff of the two prefixes

Exit code: 0 when Launch.log contains the milestone line and every --expect substring, 1 otherwise,
2 for build/stage errors.
Usage: python resources/tools/build_and_smoke.py [--build-dir build\\agentN] [--timeout 120] [--no-build]
       [--golden resources/docs/golden/2012_arkprofile_launch.log] [--milestone "<golden line>"]
       [--rhi null|d3d9] [--expect "<our log line>" ...] [--skip-native GFxUI,AkAudio,...] [--extra-args "-nomovie"]
  --milestone  golden-log line the diff is cut at (must exist in the golden log); milestone lines further
               down: "Log: Shader platform (RHI): PC-D3D-SM3", "objects as part of root set at end of
               initial load", "Log: Initializing Engine..."
  --rhi        null (default) passes -nullrhi so RHIInit picks the null RHI (DynamicRHI.cpp); d3d9 does not
  --expect     substring that must appear in Launch.log (our own lines that the golden log lacks, e.g.
               "Finished loading startup packages"); repeatable; recorded in <build-dir>/smoke/expect.txt
  --skip-native  comma list -> -skipnativepkgs=<list> (bring-up switch in appGetScriptPackageNames)
  Lines containing DISHONORED(bringup) are dropped by normalize_log.py and never reach the diff.
"""
import argparse
import difflib
import os
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "resources" / "tools"))
from normalize_log import normalize  # noqa: E402
from stage_retail import DEFAULT_RETAIL, GAME, stage  # noqa: E402

VSDEVCMD = Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat")
MILESTONE = "Init: Object subsystem initialized"
# -unattended: appMsgf/appError must not block on a message box in a scripted run (UnOutputDevices.cpp HandleError)
GAME_ARGS = ["-log", "-nosteam", "-seekfreeloadingpcconsole", "-unattended"]


def build(build_dir: Path, log: Path) -> bool:
    cmd = f'call "{VSDEVCMD}" -arch=x86 -host_arch=x64 -no_logo && cmake --build "{build_dir}" --target DishonoredGame -- -k 0'
    with log.open("w", encoding="utf-8") as out:
        rc = subprocess.run(["cmd", "/c", cmd], stdout=out, stderr=subprocess.STDOUT, cwd=REPO).returncode
    text = log.read_text(encoding="utf-8", errors="replace")
    errors = [line for line in text.splitlines() if ": error " in line or ": fatal error " in line]
    print(f"build: exit {rc}, {len(errors)} error lines (log {log})")
    for line in errors[:20]:
        print("  " + line[-200:])
    return rc == 0


def run_game(exe: Path, extra_args: list[str], timeout: float, run_log: Path, log_name: str, ini_dir: Path | None) -> int | None:
    game_dir = exe.parent.parent.parent / GAME
    launch_log = game_dir / "Logs" / log_name
    if launch_log.exists():
        launch_log.unlink()
    # generated inis: the shared retail Config when no --ini-dir, else the agent's private directory
    ini_home = ini_dir if ini_dir else game_dir / "Config"
    for stale in ini_home.glob("Dishonored*.ini"):
        stale.unlink()
    isolation = [f"-LOG={log_name}"]
    if ini_dir:
        ini_dir.mkdir(parents=True, exist_ok=True)
        isolation += [f"-{kind}INI={ini_dir / ('Dishonored' + name + '.ini')}" for kind, name in (("ENGINE", "Engine"), ("GAME", "Game"), ("INPUT", "Input"), ("UI", "UI"))]
    cmd = [str(exe), *GAME_ARGS, *isolation, *extra_args]
    print("run:", " ".join(cmd), f"(cwd {exe.parent}, timeout {timeout:.0f}s)")
    with run_log.open("w", encoding="utf-8") as out:
        proc = subprocess.Popen(cmd, cwd=exe.parent, stdout=out, stderr=subprocess.STDOUT)
        try:
            rc = proc.wait(timeout=timeout)
            print(f"run: exit code {rc} (0x{rc & 0xFFFFFFFF:08X})")
            return rc
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
            print("run: timeout, killed")
            return None


def golden_prefix(golden: Path, milestone: str) -> list[str]:
    lines = normalize(golden.read_text(encoding="utf-8", errors="replace")).splitlines()
    for i, line in enumerate(lines):
        if milestone in line:
            return lines[: i + 1]
    raise SystemExit(f"milestone line not in golden log: {milestone}")


def first_divergence(expected: list[str], ours: list[str]) -> str | None:
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, expected, ours, autojunk=False).get_opcodes():
        if tag != "equal":
            return (f"first divergence: golden line {i1 + 1} / ours line {j1 + 1} ({tag})\n"
                    f"  golden: {expected[i1] if i1 < len(expected) else '<end>'}\n"
                    f"  ours:   {ours[j1] if j1 < len(ours) else '<end>'}")
    return None


def compare(launch_log: Path, golden: Path, milestone: str, out_dir: Path) -> bool:
    if not launch_log.is_file():
        print(f"no Launch.log at {launch_log}")
        return False
    raw = launch_log.read_text(encoding="utf-8", errors="replace")
    ours = normalize(raw).splitlines()
    (out_dir / "Launch.norm.log").write_text("\n".join(ours) + "\n", encoding="utf-8")
    expected = golden_prefix(golden, milestone)
    (out_dir / "golden_prefix.norm.log").write_text("\n".join(expected) + "\n", encoding="utf-8")
    reached = any(milestone in line for line in ours)
    cut = next((i + 1 for i, line in enumerate(ours) if milestone in line), len(ours))
    diff = list(difflib.unified_diff(expected, ours[:cut], "golden(prefix)", "Launch.log", lineterm="", n=2))
    (out_dir / "smoke_diff.txt").write_text("\n".join(diff) + "\n", encoding="utf-8")
    print(f"Launch.log: {len(ours)} lines, milestone {'reached' if reached else 'NOT reached'}")
    divergence = first_divergence(expected, ours[:cut])
    if divergence:
        print(divergence)
    print("\n".join(diff) if diff else "normalized prefix identical to the golden log")
    if not reached:
        print("--- last 15 lines of Launch.log ---")
        print("\n".join(ours[-15:]))
    return reached


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", type=Path, default=REPO / "build" / "agentN")
    parser.add_argument("--retail", type=Path, default=DEFAULT_RETAIL)
    parser.add_argument("--stage", type=Path, default=None, help="ignored: the exe is staged into the retail Binaries\\Win32 (stage_retail.py)")
    parser.add_argument("--golden", type=Path, default=REPO / "resources" / "docs" / "golden" / "2012_arkprofile_launch.log")
    parser.add_argument("--milestone", default=MILESTONE)
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--extra-args", default="", help="appended to the game command line")
    parser.add_argument("--rhi", choices=["null", "d3d9"], default="null", help="null adds -nullrhi (default)")
    parser.add_argument("--expect", action="append", default=[], help="substring that must appear in Launch.log; repeatable")
    parser.add_argument("--skip-native", default="", help="comma list of native script packages to skip (-skipnativepkgs=)")
    parser.add_argument("--log-name", default="Launch.log", help="-LOG= name under DishonoredGame\\Logs (per-agent isolation)")
    parser.add_argument("--ini-dir", type=Path, default=None, help="directory for the generated Dishonored*.ini (-ENGINEINI= etc.); default: the retail Config")
    parser.add_argument("--exe-name", default=None, help="name of the staged exe next to Dishonored.exe (default DishonoredGame.exe); agents use DishonoredGame_<X>.exe")
    args = parser.parse_args(argv[1:])

    build_dir = args.build_dir.resolve()
    out_dir = build_dir / "smoke"
    out_dir.mkdir(parents=True, exist_ok=True)
    stamp = time.strftime("%Y%m%d-%H%M%S")
    if not args.no_build and not build(build_dir, out_dir / f"build_{stamp}.log"):
        return 2
    try:
        exe = stage(build_dir, args.retail.resolve(), exe_name=args.exe_name)
    except SystemExit as e:
        print(e)
        return 2
    game_args = args.extra_args.split()
    if args.rhi == "null":
        game_args.insert(0, "-nullrhi")
    if args.skip_native:
        game_args.append(f"-skipnativepkgs={args.skip_native}")
    ini_dir = args.ini_dir.resolve() if args.ini_dir else None
    run_game(exe, game_args, args.timeout, out_dir / f"run_{stamp}.log", args.log_name, ini_dir)
    launch_log = exe.parent.parent.parent / GAME / "Logs" / args.log_name
    ok = compare(launch_log, args.golden.resolve(), args.milestone, out_dir)
    if args.expect:
        text = launch_log.read_text(encoding="utf-8", errors="replace") if launch_log.is_file() else ""
        missing = [e for e in args.expect if e not in text]
        (out_dir / "expect.txt").write_text("".join(f"{'MISSING' if e in missing else 'ok'}: {e}\n" for e in args.expect), encoding="utf-8")
        for e in args.expect:
            print(f"expect {'MISSING' if e in missing else 'ok'}: {e}")
        ok = ok and not missing
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
