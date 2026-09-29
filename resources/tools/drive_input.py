"""Drive the running game with REAL Windows input, timed off the log rather than the clock.

  python resources/tools/drive_input.py --log <name> [--mark "<line>"] [--exe <image>] <step> ...

A step is `<seconds after the mark appeared>:<action>` where action is
  hover:<fx>,<fy>      walk the pointer to that fraction of the client area over ~0.3 s
  hoverclick:<fx>,<fy> the same, then press and release the left button
  key:<vk hex>[,<s>]   press that virtual key and hold it <s> seconds (default 1.5)
  wait                 no-op, a place to stop the schedule
  move:<fx>,<fy>       DEPRECATED, see below
  click:<fx>,<fy>      DEPRECATED, see below

Promoted here from agent EH (its hand-over 2) because every driver in this tree was a private copy
and three of them carried the same defect. Four things in it are measurements, not preferences:

1. **Movement is `SendInput` with `MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE`, never `SetCursorPos`.**
   `UWindowsClient::ProcessInput` reads the mouse from DirectInput8 (`WinClient.cpp:652`,
   `DirectInput8Mouse->GetDeviceData`) and `SetCursorPos` puts nothing in that buffer: 0 axis calls
   over a 110 s run, however many times the pointer was moved (agent EG).

2. **`hover` sends twelve deltas, and that is why `move` is deprecated.** DirectInput reports one
   buffered delta per motion, `move` produces exactly one, and a single delta is lost whenever the
   viewport flushes its buffer that frame - the first motion after the window is activated always
   is. Measured: four `move` steps produce three axis calls. This is not a nicety. A one-move
   schedule reported a dead mouse on a build whose mouse worked, that report became the premise of
   a whole package's brief, and the package spent its first hours reproducing a driver artifact
   (agents EG then EH).

3. **`--exe` binds the run to a process image name.** Concurrent agents run their own copies of this
   game and every window carries the title "Dishonored Game", so without it a run can be driven
   against another agent's window; one was, at another resolution, and read as a dead mouse.

4. **The foreground is re-taken immediately before a button goes down.** Three consecutive clicks
   were lost to a window that took the foreground in the 0.7 s between the step's `activate()` and
   the press.

Inherited from agents DM and DQ, with their reasons (agentDM.md 7): the schedule is keyed to a log
line rather than to the clock; only the tail of the log is read; and `AttachThreadInput` around
`SetForegroundWindow` gives the keyboard without generating input.

Read the driver's own output before believing a measurement taken through it. Every wrong conclusion
listed above was visible in it at the time.
"""
import ctypes
import ctypes.wintypes as wt
import sys
import time
from pathlib import Path

LOGDIR = Path(r"D:\RecompileDishonored\Dishonored_Latest2026\DishonoredGame\Logs")
MARK = "-gfxuimenu: opened UI_MainMenu"

user32 = ctypes.WinDLL("user32", use_last_error=True)
MOUSEEVENTF_MOVE = 0x0001
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_ABSOLUTE = 0x8000
MOUSEEVENTF_VIRTUALDESK = 0x4000
INPUT_MOUSE = 0
INPUT_KEYBOARD = 1
KEYEVENTF_KEYUP = 0x0002
SM_XVIRTUALSCREEN, SM_YVIRTUALSCREEN = 76, 77
SM_CXVIRTUALSCREEN, SM_CYVIRTUALSCREEN = 78, 79


class MOUSEINPUT(ctypes.Structure):
    _fields_ = [("dx", wt.LONG), ("dy", wt.LONG), ("mouseData", wt.DWORD),
                ("dwFlags", wt.DWORD), ("time", wt.DWORD),
                ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))]


class KEYBDINPUT(ctypes.Structure):
    _fields_ = [("wVk", wt.WORD), ("wScan", wt.WORD), ("dwFlags", wt.DWORD),
                ("time", wt.DWORD), ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))]


class _IU(ctypes.Union):
    _fields_ = [("mi", MOUSEINPUT), ("ki", KEYBDINPUT)]


class INPUT(ctypes.Structure):
    _fields_ = [("type", wt.DWORD), ("u", _IU)]


def send_mouse(flags, dx=0, dy=0, data=0):
    inp = INPUT(type=INPUT_MOUSE)
    inp.u.mi = MOUSEINPUT(dx, dy, data, flags, 0, None)
    user32.SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))


def send_key(vk, up=False):
    scan = user32.MapVirtualKeyW(vk, 0)
    inp = INPUT(type=INPUT_KEYBOARD)
    inp.u.ki = KEYBDINPUT(vk, scan, KEYEVENTF_KEYUP if up else 0, 0, None)
    user32.SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))


def move_absolute(x, y):
    """Move through the injection path DirectInput can see, in virtual-desktop coordinates."""
    vx, vy = user32.GetSystemMetrics(SM_XVIRTUALSCREEN), user32.GetSystemMetrics(SM_YVIRTUALSCREEN)
    vw, vh = user32.GetSystemMetrics(SM_CXVIRTUALSCREEN), user32.GetSystemMetrics(SM_CYVIRTUALSCREEN)
    nx = int((x - vx) * 65535 / max(1, vw - 1))
    ny = int((y - vy) * 65535 / max(1, vh - 1))
    send_mouse(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK, nx, ny)


EXE_NAME = None


def _pids_for_exe(name):
    """Agents EI, EJ and EK run their own copies of this game at the same time and their windows
    carry the same title. Binding the driver to the process whose image name is this agent's is the
    only way a measurement is about this agent's build: one run was driven against another agent's
    1920x1080 window and reported a dead mouse."""
    out = set()
    if not name:
        return out
    import subprocess
    try:
        txt = subprocess.check_output(["tasklist", "/FI", "IMAGENAME eq " + name, "/FO", "CSV", "/NH"],
                                      text=True, stderr=subprocess.DEVNULL)
    except Exception:
        return out
    for line in txt.splitlines():
        parts = [c.strip('"') for c in line.split('","')]
        if len(parts) > 1 and parts[0].lower() == name.lower():
            try:
                out.add(int(parts[1]))
            except ValueError:
                pass
    return out


def find_window():
    pids = _pids_for_exe(EXE_NAME)
    found = []

    @ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)
    def cb(hwnd, _):
        if not user32.IsWindowVisible(hwnd):
            return True
        n = user32.GetWindowTextLengthW(hwnd)
        if n > 0:
            buf = ctypes.create_unicode_buffer(n + 1)
            user32.GetWindowTextW(hwnd, buf, n + 1)
            if "Dishonored Game" in buf.value:
                pid = wt.DWORD(0)
                user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
                if pids and pid.value not in pids:
                    return True
                r = wt.RECT()
                user32.GetClientRect(hwnd, ctypes.byref(r))
                if r.right > 0 and r.bottom > 0:
                    found.append((r.right * r.bottom, hwnd, buf.value))
        return True

    user32.EnumWindows(cb, 0)
    if not found:
        return None
    # Agent EH: the game also owns a log console whose title matches, and taking the first match put
    # the foreground on the console for a whole measurement. The render window is the largest one.
    found.sort(reverse=True)
    for area, hwnd, title in found:
        print("candidate 0x%x area %d %r" % (hwnd, area, title), flush=True)
    return found[0][1]


def point(hwnd, fx, fy):
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    pt = wt.POINT(int(rect.right * fx), int(rect.bottom * fy))
    user32.ClientToScreen(hwnd, ctypes.byref(pt))
    return pt, rect


def wait_for_mark(log, mark, timeout=300.0):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            if log.exists():
                with log.open("rb") as f:
                    f.seek(max(0, log.stat().st_size - 65536))
                    if mark.encode() in f.read():
                        return True
        except OSError:
            pass
        time.sleep(0.25)
    return False


def activate(hwnd):
    # Agent EH: retry. One SetForegroundWindow is refused often enough that a whole measurement was
    # taken against a window that never had the foreground - the run reported 0 mouse events and
    # would have read as a dead mouse.
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    for attempt in range(40):
        fg = user32.GetForegroundWindow()
        if fg == hwnd:
            return True
        # AttachThreadInput is refused when the foreground belongs to a higher-integrity process (an
        # elevated Task Manager cost a whole measurement here, which then read as a dead mouse). A
        # synthetic ALT tap releases the system's foreground lock without touching that window.
        send_key(0x12)
        send_key(0x12, up=True)
        tid_fg = user32.GetWindowThreadProcessId(fg, None)
        tid_me = kernel32.GetCurrentThreadId()
        user32.AttachThreadInput(tid_me, tid_fg, True)
        user32.SetForegroundWindow(hwnd)
        user32.BringWindowToTop(hwnd)
        user32.SetActiveWindow(hwnd)
        user32.AttachThreadInput(tid_me, tid_fg, False)
        time.sleep(0.25)
    return user32.GetForegroundWindow() == hwnd


def main():
    argv = sys.argv[1:]
    log_name = "EH.log"
    mark = MARK
    steps = []
    i = 0
    while i < len(argv):
        if argv[i] == "--log":
            log_name = argv[i + 1]; i += 2; continue
        if argv[i] == "--mark":
            mark = argv[i + 1]; i += 2; continue
        if argv[i] == "--exe":
            global EXE_NAME
            EXE_NAME = argv[i + 1]; i += 2; continue
        when, _, action = argv[i].partition(":")
        steps.append((float(when), action))
        i += 1
    log = LOGDIR / log_name
    if not wait_for_mark(log, mark):
        print("the mark never appeared:", mark)
        return 1
    hwnd = None
    while hwnd is None:
        hwnd = find_window()
        time.sleep(0.2)
    activate(hwnd)
    time.sleep(0.4)
    print("window 0x%x, foreground 0x%x" % (hwnd, user32.GetForegroundWindow()), flush=True)
    t0 = time.time()

    for when, action in steps:
        delay = when - (time.time() - t0)
        if delay > 0:
            time.sleep(delay)
        activate(hwnd)
        kind, _, arg = action.partition(":")
        if kind in ("hover", "hoverclick"):
            # A real hand does not teleport: it produces a stream of small deltas over ~0.3 s, and
            # DirectInput reports one buffered delta per motion. `move` sends exactly ONE delta, and
            # a single delta is lost whenever the viewport flushes its buffer that frame (measured:
            # the first motion after the window is activated is always eaten by
            # UWindowsClient::FlushMouseInput). Every earlier driver in this tree used `move`, which
            # is why a one-move run reported a dead mouse on a build whose mouse works.
            fx, fy = (float(v) for v in arg.split(",")[:2])
            pt, rect = point(hwnd, fx, fy)
            cur = wt.POINT()
            user32.GetCursorPos(ctypes.byref(cur))
            steps_n = 12
            for s in range(1, steps_n + 1):
                x = cur.x + (pt.x - cur.x) * s // steps_n
                y = cur.y + (pt.y - cur.y) * s // steps_n
                move_absolute(x, y)
                time.sleep(0.025)
            if kind == "hoverclick":
                time.sleep(0.4)
                # Re-take the foreground immediately before the button goes down: with other agents
                # driving their own games on this desktop, three runs lost their click to a window
                # that took the foreground in the 0.7 s between the step's activate() and the press.
                activate(hwnd)
                move_absolute(pt.x, pt.y)
                time.sleep(0.1)
                send_mouse(MOUSEEVENTF_LEFTDOWN)
                time.sleep(0.25)
                send_mouse(MOUSEEVENTF_LEFTUP)
            print("%+6.1fs %s %.3f,%.3f -> client %d,%d of %dx%d in %d steps (foreground 0x%x)"
                  % (time.time() - t0, kind, fx, fy, int(rect.right * fx), int(rect.bottom * fy),
                     rect.right, rect.bottom, steps_n, user32.GetForegroundWindow()), flush=True)
        elif kind in ("move", "click"):
            fx, fy = (float(v) for v in arg.split(",")[:2])
            pt, rect = point(hwnd, fx, fy)
            # Two steps: DirectInput reports a delta, so the first move seeds the position and the
            # second is the one the interface reads.
            move_absolute(pt.x, pt.y)
            time.sleep(0.12)
            move_absolute(pt.x, pt.y)
            if kind == "click":
                time.sleep(0.4)
                send_mouse(MOUSEEVENTF_LEFTDOWN)
                time.sleep(0.25)
                send_mouse(MOUSEEVENTF_LEFTUP)
            print("%+6.1fs %s %.3f,%.3f -> client %d,%d of %dx%d"
                  % (time.time() - t0, kind, fx, fy, int(rect.right * fx), int(rect.bottom * fy),
                     rect.right, rect.bottom), flush=True)
        elif kind == "key":
            parts = arg.split(",")
            vk = int(parts[0], 16)
            hold = float(parts[1]) if len(parts) > 1 else 1.5
            send_key(vk)
            print("%+6.1fs key 0x%02x down for %.2fs (foreground 0x%x)"
                  % (time.time() - t0, vk, hold, user32.GetForegroundWindow()), flush=True)
            time.sleep(hold)
            send_key(vk, up=True)
        elif kind == "wait":
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
