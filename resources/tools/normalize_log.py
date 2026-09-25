"""Normalize a UE3 Launch.log for diffing: strip timestamps, dates, addresses, machine-specific names.

Usage: python resources/tools/normalize_log.py <log> [<log> ...]   -> writes <log>.norm.log next to each input
"""
import re
import sys
from pathlib import Path

RULES = [
    (re.compile(r"^\[\d{4}\.\d{2}\]\s*"), ""),
    (re.compile(r"\b\d{2}/\d{2}/\d{2,4}\b"), "<date>"),
    (re.compile(r"\b\d{2}:\d{2}:\d{2}\b"), "<time>"),
    (re.compile(r"\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}"), "<stamp>"),
    (re.compile(r"0x[0-9A-Fa-f]{6,}"), "0x<addr>"),
    (re.compile(r"\b[A-Za-z]:\\[^\s\"']+"), "<path>"),
    (re.compile(r"\b\d+(\.\d+)?\s*(ms|MB|GB|KB)\b"), "<n> \\2"),
    (re.compile(r"(Computer|User): .*$"), "\\1: <redacted>"),
    (re.compile(r"\b\d+\.\d{2,} seconds\b"), "<t> seconds"),
]


DROP_MARK = "DISHONORED(bringup)"  # our own bring-up diagnostics never take part in the golden diff


def normalize(text: str) -> str:
    out = []
    for line in text.splitlines():
        if DROP_MARK in line:
            continue
        for pattern, repl in RULES:
            line = pattern.sub(repl, line)
        out.append(line.rstrip())
    return "\n".join(out) + "\n"


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    for arg in argv[1:]:
        src = Path(arg)
        dst = src.with_suffix(".norm.log")
        dst.write_text(normalize(src.read_text(encoding="utf-8", errors="replace")), encoding="utf-8")
        print(f"{src} -> {dst}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
