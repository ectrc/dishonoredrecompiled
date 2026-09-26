"""Stage an agent's exe into the retail Binaries\\Win32 and run it under dbg.py with the same isolation switches as
build_and_smoke.py (agent X's `agentX_dbgrun.py` / agent Z's `dbg.py`, promoted). Output: <build-dir>/dbg/<tag>.txt
(debugger output: exception or hang stacks) and <build-dir>/dbg/<tag>.log (a copy of the agent's Launch.log).

Usage (same switches as build_and_smoke.py, plus --hang / --tag):
  python resources/tools/debug/dbgrun.py --build-dir build/agentX --exe-name DishonoredGame_X.exe --log-name agentX.log
         --ini-dir build/agentX/config [--rhi null|d3d9] [--skip-native OnlineSubsystemPC] [--hang 90] [--tag crash1]
         [--extra-args "-startmap=L_Tower_P"]

-forcelogflush is added so the Launch.log holds every line up to the stop. Exit code = dbg.py's (0 exit, 1 exception,
2 launch failure, 3 hang dump).
"""
import argparse
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / "resources" / "tools"))
from build_and_smoke import GAME_ARGS, prepare_run  # noqa: E402
from stage_retail import DEFAULT_RETAIL, stage  # noqa: E402

DBG = Path(__file__).resolve().parent / "dbg.py"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--retail", type=Path, default=DEFAULT_RETAIL)
    parser.add_argument("--exe-name", required=True, help="DishonoredGame_<X>.exe")
    parser.add_argument("--log-name", required=True, help="agent<X>.log")
    parser.add_argument("--ini-dir", type=Path, required=True, help="build/agent<X>/config")
    parser.add_argument("--rhi", choices=["null", "d3d9"], default="null")
    parser.add_argument("--skip-native", default="")
    parser.add_argument("--extra-args", default="")
    # a debugged run is several times slower than the smoke: with a debugger attached every log line goes through
    # FOutputDeviceDebug -> OutputDebugString -> a debug event (the baseline needs ~2-4 minutes to its appErrorf, 27 s alone)
    parser.add_argument("--hang", type=float, default=400.0, help="seconds before every thread's stack is dumped (default 400)")
    parser.add_argument("--tag", default=None, help="output name under <build-dir>/dbg (default: a timestamp)")
    args = parser.parse_args(argv[1:])

    build_dir = args.build_dir.resolve()
    exe = stage(build_dir, args.retail.resolve(), exe_name=args.exe_name)
    map_file = build_dir / "Binaries" / "Win32" / "DishonoredGame.map"
    if not map_file.is_file():
        raise SystemExit(f"no linker map: {map_file}")
    launch_log, isolation = prepare_run(exe, args.log_name, args.ini_dir.resolve())
    game_args = args.extra_args.split()
    if args.rhi == "null":
        game_args.insert(0, "-nullrhi")
    if args.skip_native:
        game_args.append(f"-skipnativepkgs={args.skip_native}")
    out_dir = build_dir / "dbg"
    out_dir.mkdir(parents=True, exist_ok=True)
    tag = args.tag or time.strftime("%Y%m%d-%H%M%S")
    cmd = [sys.executable, str(DBG), f"--hang={args.hang:g}", str(exe), str(map_file), str(exe.parent),
           *GAME_ARGS, "-forcelogflush", *isolation, *game_args]
    print("dbgrun:", " ".join(cmd))
    out = out_dir / f"{tag}.txt"
    with out.open("w", encoding="utf-8") as f:
        rc = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, timeout=args.hang + 600).returncode
    print(out.read_text(encoding="utf-8")[:8000])
    if launch_log.is_file():
        shutil.copy2(launch_log, out_dir / f"{tag}.log")
        lines = launch_log.read_text(encoding="utf-8", errors="replace").splitlines()
        print(f"--- last 20 of {len(lines)} log lines ({out_dir / (tag + '.log')}) ---")
        print("\n".join(line[:300] for line in lines[-20:]))
    print(f"dbgrun: dbg.py exit {rc} -> {out}")
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv))
