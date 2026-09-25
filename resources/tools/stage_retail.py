"""Create build\\stage\\: a runnable copy of the retail Dishonored tree around our own exe.

  Binaries\\Win32\\   our DishonoredGame.exe (+ .pdb, .map) plus every DLL the retail exe ships next
                    to it (binkw32, PhysX*, APEX_*, steam_api, ...) copied from the retail tree
  DishonoredGame\\   a real directory: Config\\ is a copy (the engine generates DishonoredEngine.ini
                    & co. and flushes them there), Logs\\ is real (Launch.log), the content
                    directories (CookedPCConsole, DLC, Localization, Movies) are junctions
  Engine\\           junction (Config, Localization, Stats: read only for a cooked game)

Nothing is ever written into the retail tree: everything the engine writes lands in a real
directory of the stage. Junctions are made with `mklink /J` (no admin rights needed) and are
recreated on every run; the stage directory can be deleted with `rmdir /s` safely because
junctions are removed as links (Python's rmtree does not descend into them).

Usage: python resources/tools/stage_retail.py [--build-dir build\\agentN] [--retail <dir>] [--stage build\\stage]
"""
import argparse
import os
import shutil
import stat
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEFAULT_RETAIL = REPO.parent / "Dishonored_Latest2026"
GAME = "DishonoredGame"
GAME_JUNCTIONS = ["CookedPCConsole", "DLC", "Localization", "Movies"]
GAME_COPIES = ["Config"]
GAME_REAL_DIRS = ["Logs"]
BINARY_JUNCTIONS = ["Microsoft.VC90.CRT"]
# DLLs our build imports that the retail exe does not ship (nvtt is delay-loaded by the exe and only
# needed once texture cooking is reached); taken from the UE3 reference tree next to the repo.
NVTT_BIN = REPO.parent / "UnrealEngine3" / "Development" / "External" / "nvtt" / "bin"
REFERENCE_DLLS = [NVTT_BIN / "nvtt.dll", NVTT_BIN / "cudart.dll"]
FILE_ATTRIBUTE_REPARSE_POINT = 0x400


def is_junction(path: Path) -> bool:
    try:
        return bool(os.lstat(path).st_file_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
    except FileNotFoundError:
        return False


def remove_junction(path: Path) -> None:
    if is_junction(path):
        os.rmdir(path)


def make_junction(link: Path, target: Path) -> None:
    if not target.is_dir():
        raise SystemExit(f"junction target missing: {target}")
    remove_junction(link)
    if link.exists():
        raise SystemExit(f"{link} exists and is not a junction; remove it by hand")
    link.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["cmd", "/c", "mklink", "/J", str(link), str(target)], check=True, stdout=subprocess.DEVNULL)


def force_writable_then_retry(func, path, exc) -> None:
    os.chmod(path, stat.S_IWRITE)
    func(path)


def copy_tree_fresh(src: Path, dst: Path) -> None:
    if is_junction(dst):
        os.rmdir(dst)
    elif dst.exists():
        shutil.rmtree(dst, onexc=force_writable_then_retry)
    # copyfile drops the retail read-only attribute: the engine rewrites the generated inis here
    shutil.copytree(src, dst, copy_function=shutil.copyfile)


def stage(build_dir: Path, retail: Path, stage_dir: Path) -> Path:
    if not retail.is_dir():
        raise SystemExit(f"retail tree not found: {retail}")
    exe = build_dir / "Binaries" / "Win32" / f"{GAME}.exe"
    if not exe.is_file():
        raise SystemExit(f"exe not built: {exe}")

    binaries = stage_dir / "Binaries" / "Win32"
    binaries.mkdir(parents=True, exist_ok=True)
    for suffix in (".exe", ".pdb", ".map"):
        src = exe.with_suffix(suffix)
        if src.is_file():
            shutil.copy2(src, binaries / src.name)
    retail_binaries = retail / "Binaries" / "Win32"
    for dll in sorted(retail_binaries.glob("*.dll")):
        dst = binaries / dll.name
        if not dst.is_file() or dst.stat().st_size != dll.stat().st_size:
            shutil.copy2(dll, dst)
    for dll in REFERENCE_DLLS:
        if dll.is_file():
            shutil.copy2(dll, binaries / dll.name)
    for name in BINARY_JUNCTIONS:
        if (retail_binaries / name).is_dir():
            make_junction(binaries / name, retail_binaries / name)

    game = stage_dir / GAME
    game.mkdir(parents=True, exist_ok=True)
    for name in GAME_COPIES:
        copy_tree_fresh(retail / GAME / name, game / name)
    for name in GAME_REAL_DIRS:
        (game / name).mkdir(exist_ok=True)
    for name in GAME_JUNCTIONS:
        if (retail / GAME / name).is_dir():
            make_junction(game / name, retail / GAME / name)
    for toc in (retail / GAME).glob("*.txt"):
        shutil.copy2(toc, game / toc.name)

    make_junction(stage_dir / "Engine", retail / "Engine")
    return binaries / exe.name


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", type=Path, default=REPO / "build" / "agentN")
    parser.add_argument("--retail", type=Path, default=DEFAULT_RETAIL)
    parser.add_argument("--stage", type=Path, default=REPO / "build" / "stage")
    args = parser.parse_args(argv[1:])
    exe = stage(args.build_dir.resolve(), args.retail.resolve(), args.stage.resolve())
    print(f"staged {exe}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
