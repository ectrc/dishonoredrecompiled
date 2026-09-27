"""Agent AV: every file of the package back to CRLF (the patch scripts used Path.read_text, whose universal-newline
translation silently wrote them out as LF). Prints the files it changed."""
import sys
from pathlib import Path

REPO = Path(sys.argv[1] if len(sys.argv) > 1 else r"D:\RecompileDishonored\Recompile")
LIST = REPO / "build/agentAV_work/files.txt"

for rel in LIST.read_text(encoding="utf-8").split():
    p = REPO / rel
    data = p.read_bytes()
    fixed = data.replace(b"\r\n", b"\n").replace(b"\n", b"\r\n")
    if fixed != data:
        p.write_bytes(fixed)
        print("crlf", rel)
    else:
        print("ok  ", rel)
