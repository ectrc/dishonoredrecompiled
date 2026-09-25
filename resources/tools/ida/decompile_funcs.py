"""Decompile selected functions of the 2012 Shipping exe to files (headless, no MCP session needed).

Usage:
  python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py <db.i64> <out_dir> <pattern> [<pattern> ...]
  python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py <db.i64> <out_dir> --file <listfile>

A pattern is matched (case-sensitive substring or regex with the `re:` prefix) against the demangled
name; `rva:0x1234` selects by address. One `<out_dir>/<sanitized name>.c` per function, with the
demangled name, rva and PDB file:line (from functions.csv) as a header comment.
"""
import csv
import re
import sys
from pathlib import Path

import ida_hexrays

from _common import SYMBOLS_DIR, demangle_full, function_name, iter_functions, log, rva


def sanitize(name: str) -> str:
    short = re.sub(r"\(.*$", "", name)
    short = short.split(" ")[-1] if " " in short else short
    return re.sub(r"[^A-Za-z0-9_]+", "_", short)[:120]


def main() -> None:
    args = sys.argv[1:]
    if len(args) < 2:
        log(__doc__)
        return
    out_dir = Path(args[0])
    out_dir.mkdir(parents=True, exist_ok=True)
    patterns = args[1:]
    if patterns[:1] == ["--file"]:
        patterns = [l.strip() for l in Path(patterns[1]).read_text(encoding="utf-8").splitlines() if l.strip() and not l.startswith("#")]
    if not ida_hexrays.init_hexrays_plugin():
        log("Hex-Rays not available")
        return
    files = {}
    try:
        with (SYMBOLS_DIR / "functions.csv").open(newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                files[int(row["rva"], 16)] = (row["file"], row["line"])
    except FileNotFoundError:
        pass

    def matches(name: str, ea: int) -> bool:
        for p in patterns:
            if p.startswith("rva:") and int(p[4:], 16) == rva(ea):
                return True
            if p.startswith("re:") and re.search(p[3:], name):
                return True
            if not p.startswith(("rva:", "re:")) and p in name:
                return True
        return False

    done = 0
    for func in iter_functions():
        name = demangle_full(function_name(func))
        if not matches(name, func.start_ea):
            continue
        try:
            text = str(ida_hexrays.decompile(func.start_ea))
        except ida_hexrays.DecompilationFailure as e:
            log(f"failed: {name}: {e}")
            continue
        file, line = files.get(rva(func.start_ea), ("", ""))
        header = f"// {name}\n// rva 0x{rva(func.start_ea):x}  size {func.size()}  {file}:{line}\n\n"
        (out_dir / f"{sanitize(name)}.c").write_text(header + text, encoding="utf-8")
        done += 1
    log(f"decompiled {done} functions -> {out_dir}")


if __name__ == "__main__":
    main()
