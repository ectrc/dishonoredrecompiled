"""Stage our exe for a run against the retail content: copy DishonoredGame.exe (+ .pdb, .map) INTO the
retail tree's Binaries\\Win32 next to the retail Dishonored.exe, and make sure DishonoredGame\\Logs exists.

Nothing else is touched. The engine then finds ..\\..\\DishonoredGame and ..\\..\\Engine exactly as the
retail exe does, writes Launch.log to DishonoredGame\\Logs and the generated Dishonored*.ini next to the
read-only Default*.ini in DishonoredGame\\Config (the retail exe does the same).

Why no separate stage directory any more (incident, wave 2 merge): the previous design junctioned the
retail content folders (CookedPCConsole, DLC, Localization, Movies, Engine) into build\\stage. Any
recursive delete that follows reparse points (git worktree remove, some rm -rf / Remove-Item -Recurse
implementations) then deletes the RETAIL files through the junction, which is what happened. Rule:
never create junctions or symlinks that point into the retail or reference trees; remove a link with
os.rmdir/rmdir only (build/unlink_junctions.py lists and removes them safely).

Usage: python resources/tools/stage_retail.py [--build-dir build\\agentN] [--retail <dir>]
       (--stage is accepted for compatibility and ignored)
"""
import argparse
import shutil
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEFAULT_RETAIL = REPO.parent / "Dishonored_Latest2026"
GAME = "DishonoredGame"
OUR_EXE = f"{GAME}.exe"  # never the retail exe name: Dishonored.exe stays untouched


def stage(build_dir: Path, retail: Path, stage_dir: Path | None = None, exe_name: str | None = None) -> Path:
    """Copies build_dir\\Binaries\\Win32\\DishonoredGame.exe (+ .pdb/.map) into the retail Binaries\\Win32 as exe_name
    (default DishonoredGame.exe; agents stage DishonoredGame_<X>.exe so runs never overwrite each other)."""
    target_name = exe_name or OUR_EXE
    if not retail.is_dir():
        raise SystemExit(f"retail tree not found: {retail}")
    if target_name.lower() == "dishonored.exe" or not target_name.lower().endswith(".exe"):
        raise SystemExit("refusing to overwrite the retail exe / bad exe name")
    exe = build_dir / "Binaries" / "Win32" / OUR_EXE
    if not exe.is_file():
        raise SystemExit(f"exe not built: {exe}")
    retail_binaries = retail / "Binaries" / "Win32"
    if not (retail_binaries / "Dishonored.exe").is_file():
        raise SystemExit(f"{retail_binaries} does not look like the retail Binaries\\Win32 (no Dishonored.exe)")
    for required in ("Engine/Config/BaseEngine.ini", f"{GAME}/Config/DefaultEngine.ini", f"{GAME}/CookedPCConsole"):
        if not (retail / required).exists():
            raise SystemExit(f"retail content missing: {retail / required} (restore the install, e.g. Steam 'Verify integrity of game files')")
    if exe.stat().st_size < 1_000_000:
        raise SystemExit(f"{exe} is {exe.stat().st_size} bytes: the DishonoredLaunchStub build, not the real Launch (configure with -DDISHONORED_REAL_LAUNCH=ON)")
    stem = Path(target_name).stem
    for suffix in (".exe", ".pdb", ".map"):
        src = exe.with_suffix(suffix)
        if src.is_file():
            shutil.copy2(src, retail_binaries / (stem + suffix))
    (retail / GAME / "Logs").mkdir(exist_ok=True)
    print(f"staged {retail_binaries / target_name} ({exe.stat().st_size:,} bytes)")
    return retail_binaries / target_name


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", type=Path, default=REPO / "build" / "agentN")
    parser.add_argument("--retail", type=Path, default=DEFAULT_RETAIL)
    parser.add_argument("--stage", type=Path, default=None, help="ignored (kept for older command lines)")
    parser.add_argument("--exe-name", default=None, help="staged exe name (default DishonoredGame.exe)")
    args = parser.parse_args(argv[1:])
    exe = stage(args.build_dir.resolve(), args.retail.resolve(), exe_name=args.exe_name)
    print(f"staged {exe}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
