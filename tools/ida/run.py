"""Headless runner: python tools/ida/run.py <script.py> <database.i64> [--save] [script args...]

Opens the database with idalib, executes the script with __name__ == "__main__", closes the
database without saving unless --save is present.
"""
import runpy
import sys
from pathlib import Path


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(__doc__)
        return 2
    script = Path(argv[1]).resolve()
    database = Path(argv[2]).resolve()
    save = "--save" in argv[3:]
    script_args = [a for a in argv[3:] if a != "--save"]

    import idapro  # noqa: E402  must be imported before any ida_* module

    if idapro.open_database(str(database), run_auto_analysis=False) != 0:
        print(f"failed to open {database}", file=sys.stderr)
        return 1
    try:
        sys.path.insert(0, str(script.parent))
        sys.argv = [str(script), *script_args]
        runpy.run_path(str(script), run_name="__main__")
    finally:
        idapro.close_database(save=save)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
