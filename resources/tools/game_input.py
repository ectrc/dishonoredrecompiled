"""Drive the game without touching the user's mouse, keyboard or window focus.

This is the ONLY sanctioned way for an agent or a script to send input to a running build. It writes
commands to the file the game polls under `-inputcmd=<file>`; the game injects them inside its own
process, after DirectInput (Engine/Src/UnPlayer.cpp, DishonoredTickInputScript), and under that
switch the window is never activated, never captures or clips the cursor and ignores every real key
and mouse message. Nothing here calls SendInput, SetCursorPos, SetForegroundWindow or
AttachThreadInput, and drive_input.py - which did all four - now refuses to run.

  python resources/tools/game_input.py launch --exe <path> [--log NAME] [--tag TAG] [--cmd-file F] [-- extra game args]
      starts the game with -inputcmd=<F> -unattended -apshotname=<TAG> -LOG=<NAME> -forcelogflush and prints
      the command file path; the file is created empty first so nothing stale is consumed.
  python resources/tools/game_input.py --cmd-file F hover 0.419 0.216
  python resources/tools/game_input.py --cmd-file F click
  python resources/tools/game_input.py --cmd-file F click 0.5 0.5
  python resources/tools/game_input.py --cmd-file F key Enter
  python resources/tools/game_input.py --cmd-file F wait 2 shot tabs1
  python resources/tools/game_input.py --cmd-file F --script steps.txt       # a whole file of commands
  python resources/tools/game_input.py --cmd-file F raw "move 0.5 0.5" "down" "wait 0.1" "up"

Several commands on one invocation are allowed and are appended in order. Coordinates for move / hover /
click are FRACTIONS of the viewport (0..1) so a capture's pixel size does not matter - the user's desktop
resolution flaps and the window is 1008x567 on some runs and 1600x900 on others. `movepx X Y` takes
client pixels if you really have them.

The game logs every command it executes as `DISHONORED(inputcmd): t=<world time> <command>` and
`... idle` when the queue drains; correlate against that rather than against wall-clock sleeps.

Commands the game understands (one per line in the file):
  move X Y | movepx X Y | hover X Y | down [left|right|middle] | up [...] | click [X Y]
  key NAME [press|down|up] | wait SECS | shot TAG | log TEXT | quit
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ONE_ARG = {"wait", "shot", "key", "down", "up"}
TWO_ARG = {"move", "movepx", "hover"}
ZERO_OR_TWO = {"click"}
REST = {"log", "key"}


def group_commands(tokens: list[str]) -> list[str]:
    """Turns a flat token list like ['hover','0.4','0.2','click','key','Enter'] into command lines."""
    out: list[str] = []
    i = 0
    while i < len(tokens):
        op = tokens[i].lower()
        i += 1
        if op in TWO_ARG:
            out.append(f"{op} {tokens[i]} {tokens[i+1]}")
            i += 2
        elif op in ZERO_OR_TWO:
            if i + 1 < len(tokens) and _is_num(tokens[i]) and _is_num(tokens[i + 1]):
                out.append(f"{op} {tokens[i]} {tokens[i+1]}")
                i += 2
            else:
                out.append(op)
        elif op == "key":
            name = tokens[i]
            i += 1
            how = ""
            if i < len(tokens) and tokens[i].lower() in ("press", "down", "up"):
                how = " " + tokens[i].lower()
                i += 1
            out.append(f"key {name}{how}")
        elif op in ("down", "up"):
            if i < len(tokens) and tokens[i].lower() in ("left", "right", "middle"):
                out.append(f"{op} {tokens[i].lower()}")
                i += 1
            else:
                out.append(op)
        elif op in ("wait", "shot"):
            out.append(f"{op} {tokens[i]}")
            i += 1
        elif op == "log":
            out.append("log " + " ".join(tokens[i:]))
            i = len(tokens)
        elif op == "quit":
            out.append("quit")
        else:
            raise SystemExit(f"game_input: unknown command '{op}'")
    return out


def _is_num(s: str) -> bool:
    try:
        float(s)
        return True
    except ValueError:
        return False


def append(cmd_file: Path, lines: list[str]) -> None:
    # Append, never overwrite: the game truncates the file itself once it has consumed it, and two
    # appends in a row must both survive until then.
    with cmd_file.open("a", encoding="ascii", newline="\n") as f:
        for line in lines:
            f.write(line + "\n")
    print("\n".join(f"queued: {l}" for l in lines))


def launch(args: argparse.Namespace, extra: list[str]) -> int:
    exe = Path(args.exe)
    if not exe.exists():
        raise SystemExit(f"game_input: exe not found: {exe}")
    cmd_file = Path(args.cmd_file) if args.cmd_file else Path(tempfile.gettempdir()) / f"inputcmd_{args.tag or 'run'}_{os.getpid()}.txt"
    cmd_file.write_text("", encoding="ascii")
    game_args = [f"-inputcmd={cmd_file}", "-unattended", "-forcelogflush"]
    if args.log:
        game_args.append(f"-LOG={args.log}")
    if args.tag:
        game_args.append(f"-apshotname={args.tag}")
    game_args += [a for a in extra if a != "--"]
    # No window activation of any kind: the game itself shows SW_SHOWNOACTIVATE under -inputcmd, and
    # we do not touch it from here either.
    p = subprocess.Popen([str(exe)] + game_args, cwd=str(exe.parent))
    print(f"launched pid {p.pid}")
    print(f"cmd-file {cmd_file}")
    print("args " + " ".join(game_args))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cmd-file", help="the file the game was launched with as -inputcmd=<file>")
    ap.add_argument("--script", help="append every non-comment line of this file")
    ap.add_argument("--exe", help="launch: path to the game executable")
    ap.add_argument("--log", help="launch: -LOG=<name>")
    ap.add_argument("--tag", help="launch: -apshotname=<tag>, also the default cmd-file name stem")
    ap.add_argument("tokens", nargs="*", help="commands, or 'launch', or 'raw' followed by whole lines")
    args, extra = ap.parse_known_args()

    if args.tokens and args.tokens[0] == "launch":
        if not args.exe:
            raise SystemExit("game_input: launch needs --exe")
        return launch(args, extra + args.tokens[1:])

    if not args.cmd_file:
        raise SystemExit("game_input: --cmd-file is required (the path printed by launch)")
    cmd_file = Path(args.cmd_file)
    lines: list[str] = []
    if args.script:
        for raw in Path(args.script).read_text(encoding="utf-8", errors="replace").splitlines():
            s = raw.strip()
            if s and not s.startswith("#"):
                lines.append(s)
    if args.tokens:
        if args.tokens[0] == "raw":
            lines += [t.strip() for t in args.tokens[1:] if t.strip()]
        else:
            lines += group_commands(args.tokens)
    if not lines:
        raise SystemExit("game_input: nothing to queue")
    append(cmd_file, lines)
    return 0


if __name__ == "__main__":
    sys.exit(main())
