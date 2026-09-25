"""List and remove every junction / symlinked directory under a directory (default: Recompile/build).

os.rmdir on a reparse point removes the LINK only; the target is never touched, and the walk never
descends into a reparse point. This is the only sanctioned way to get rid of a junction in this repo:
recursive deletes that follow reparse points (git worktree remove, some rm -rf / Remove-Item -Recurse)
deleted the retail game content through the old build\\stage junctions on 2026-09-25.

Usage: python resources/tools/unlink_junctions.py [<dir>] [--apply]
"""
import os
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]


def main(argv: list[str]) -> int:
    args = [a for a in argv[1:] if a != "--apply"]
    apply = "--apply" in argv
    root = Path(args[0]).resolve() if args else REPO / "build"
    found = []
    for dirpath, dirnames, _ in os.walk(root):
        keep = []
        for d in dirnames:
            p = Path(dirpath) / d
            try:
                attrs = os.lstat(p).st_file_attributes
            except OSError:
                keep.append(d)
                continue
            if attrs & 0x400:  # FILE_ATTRIBUTE_REPARSE_POINT: record, never descend
                try:
                    target = os.readlink(p)
                except OSError:
                    target = "?"
                found.append((p, target))
            else:
                keep.append(d)
        dirnames[:] = keep
    for p, target in found:
        print(f"{p} -> {target}")
        if apply:
            os.rmdir(p)
            print("  unlinked")
    print(f"{len(found)} reparse points under {root}{' removed' if apply else ' (dry run; --apply removes the links)'}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
