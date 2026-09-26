"""Snapshot build tree for an agent: a detached git worktree of HEAD at build/agent<X>_wt plus the agent's own files
copied over it (the shared working tree is edited by every agent at once; when someone else's in-flight edit breaks your
build you build the snapshot instead). Also writes build/agent<X>_wt_build.cmd (configure -S build/agent<X>_wt -B
build/agent<X> with every module option, like resources/build-game.cmd).

Usage:
  python resources/tools/make_snapshot.py <X> [file ...] [--list <listfile>]   create or refresh the snapshot, overlay files
  python resources/tools/make_snapshot.py <X> --sync                            re-copy the files recorded on earlier runs
  python resources/tools/make_snapshot.py <X> --remove                          remove the worktree (only when it holds no link)

Files are paths relative to the repo (or absolute inside it); the list is remembered in build/agent<X>/snapshot_files.txt,
so `--sync` after each edit keeps the snapshot current. A file that no longer exists in the working tree is deleted from the
snapshot too (a delete is part of your edit).

Safety (incident 2026-09-25, STATUS.md): the tool REFUSES to touch a target that contains a reparse point (junction /
symlink) or a stage directory (`build/stage`, the old junction design), refuses reparse points among the overlay sources,
never follows links and never deletes recursively. --remove first walks the worktree without descending into links; one
reparse point found = refusal with the unlink_junctions.py command to run first. Nothing under the retail or reference trees
is ever read or written.
"""
import filecmp
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
BUILD = REPO / "build"
FILE_ATTRIBUTE_REPARSE_POINT = 0x400
CMAKE_OPTIONS = ("-G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-x86.cmake "
                 "-DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/UnrealEngine3 -DDISHONORED_REAL_LAUNCH=ON "
                 "-DDISHONORED_ENABLE_GFXUI=ON -DDISHONORED_ENABLE_AKAUDIO=ON -DDISHONORED_ENABLE_OSS=ON -DDISHONORED_ENABLE_DISHONOREDGAME=ON")


def is_reparse_point(p: Path) -> bool:
    try:
        return bool(os.lstat(p).st_file_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
    except OSError:
        return False


def reparse_points_under(root: Path) -> list[Path]:
    """Every junction/symlink under root, found without descending into any of them (unlink_junctions.py's walk)."""
    found = []
    if is_reparse_point(root):
        return [root]
    for dirpath, dirnames, filenames in os.walk(root):
        keep = []
        for d in dirnames:
            p = Path(dirpath) / d
            if is_reparse_point(p):
                found.append(p)
            else:
                keep.append(d)
        dirnames[:] = keep
        found += [Path(dirpath) / f for f in filenames if is_reparse_point(Path(dirpath) / f)]
    return found


def refuse_unsafe_target(target: Path) -> None:
    if not target.exists():
        return
    links = reparse_points_under(target)
    if links:
        raise SystemExit(f"REFUSED: {target} contains {len(links)} reparse point(s), e.g. {links[0]} -> {readlink(links[0])}. "
                         f"Remove them with `python resources/tools/unlink_junctions.py {target} --apply` (links only) and rerun.")
    stage_dirs = [p for p in (target / "build" / "stage", target / "stage") if p.exists()]
    if stage_dirs:
        raise SystemExit(f"REFUSED: {stage_dirs[0]} exists: a stage directory (the pre-incident junction design) is never part of a snapshot.")


def readlink(p: Path) -> str:
    try:
        return os.readlink(p)
    except OSError:
        return "?"


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=REPO, capture_output=True, text=True, check=True).stdout


def registered_worktrees() -> set[Path]:
    return {Path(line[9:]).resolve() for line in git("worktree", "list", "--porcelain").splitlines() if line.startswith("worktree ")}


def rel_to_repo(file: str) -> Path:
    p = Path(file)
    p = (p if p.is_absolute() else REPO / p).resolve()
    try:
        rel = p.relative_to(REPO)
    except ValueError:
        raise SystemExit(f"REFUSED: {file} is outside the repo") from None
    if rel.parts[:1] == ("build",) or rel.parts[:1] == ("external",):
        raise SystemExit(f"REFUSED: {file} is under build/ or external/, not a source file")
    return rel


def ensure_worktree(target: Path) -> None:
    if target.resolve() in registered_worktrees():
        head = git("-C", str(target), "rev-parse", "--short", "HEAD").strip()
        print(f"worktree {target} exists (HEAD {head})")
        return
    if target.exists():
        raise SystemExit(f"REFUSED: {target} exists but is not a registered git worktree; pick another letter or remove it by hand (no recursive delete through links)")
    print(f"git worktree add --detach {target} HEAD")
    subprocess.run(["git", "worktree", "add", "--detach", str(target), "HEAD"], cwd=REPO, check=True)


def overlay(target: Path, files: list[Path]) -> None:
    copied = removed = same = 0
    for rel in files:
        src, dst = REPO / rel, target / rel
        if is_reparse_point(src) or is_reparse_point(dst):
            raise SystemExit(f"REFUSED: {rel} is a reparse point")
        if not src.exists():
            if dst.exists():
                dst.unlink()
                removed += 1
                print(f"removed {rel}")
            continue
        if dst.exists() and filecmp.cmp(src, dst, shallow=False):
            same += 1
            continue
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        copied += 1
        print(f"copied  {rel}")
    print(f"overlay: {copied} copied, {removed} removed, {same} unchanged")


def write_build_cmd(letter: str, target: Path) -> Path:
    cmd = BUILD / f"agent{letter}_wt_build.cmd"
    build_dir = BUILD / f"agent{letter}"
    vsdevcmd = r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    text = "\r\n".join([
        "@echo off",
        f"rem generated by resources/tools/make_snapshot.py: builds the HEAD snapshot {target} into {build_dir}",
        f"rem usage: build\\agent{letter}_wt_build.cmd [target]   (default DishonoredGame)",
        "setlocal",
        "set TARGET=%1",
        'if "%TARGET%"=="" set TARGET=DishonoredGame',
        f'call "{vsdevcmd}" -arch=x86 -host_arch=x64 -no_logo || exit /b 1',
        f'cmake -S "{target}" -B "{build_dir}" {CMAKE_OPTIONS} || exit /b 1',
        f'cmake --build "{build_dir}" --target %TARGET% -- -k 0',
        "",
    ])
    cmd.write_text(text, encoding="ascii")
    return cmd


def remove(target: Path) -> int:
    if not target.exists():
        print(f"{target} does not exist")
        return 0
    links = reparse_points_under(target)
    if links:
        for p in links:
            print(f"link: {p} -> {readlink(p)}")
        print(f"REFUSED: {len(links)} reparse point(s) inside {target}. Run `python resources/tools/unlink_junctions.py {target} --apply` first "
              f"(removes the links only), then --remove again.")
        return 1
    if target.resolve() not in registered_worktrees():
        print(f"REFUSED: {target} is not a registered git worktree")
        return 1
    subprocess.run(["git", "worktree", "remove", "--force", str(target)], cwd=REPO, check=True)
    print(f"removed worktree {target} (0 reparse points were inside)")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    letter = argv[1]
    target = BUILD / f"agent{letter}_wt"
    record = BUILD / f"agent{letter}" / "snapshot_files.txt"
    args = argv[2:]
    if args == ["--remove"]:
        return remove(target)
    files: list[str] = []
    if "--sync" in args:
        args.remove("--sync")
        files += record.read_text(encoding="utf-8").splitlines() if record.is_file() else []
    if "--list" in args:
        i = args.index("--list")
        files += [l.strip() for l in Path(args[i + 1]).read_text(encoding="utf-8").splitlines() if l.strip() and not l.startswith("#")]
        del args[i:i + 2]
    files += args
    rels = sorted({rel_to_repo(f) for f in files}, key=str)
    refuse_unsafe_target(target)
    ensure_worktree(target)
    refuse_unsafe_target(target)
    overlay(target, rels)
    record.parent.mkdir(parents=True, exist_ok=True)
    record.write_text("".join(f"{r.as_posix()}\n" for r in rels), encoding="utf-8")
    cmd = write_build_cmd(letter, target)
    print(f"{len(rels)} files recorded in {record}; build with {cmd} [target]")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
