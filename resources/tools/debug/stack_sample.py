"""Sample every thread of a running DishonoredGame exe: suspend, print the EBP-chain stack of each thread symbolized
through the linker .map, resume (agent W's `build\\agentW\\stack_sample.py`, promoted). Non-invasive: the process is not a
debuggee, so it can be sampled several times while it runs (a stall in `appSleep`, a spinning async loader, a hang).

Usage:
  python resources/tools/debug/stack_sample.py <map> --pid <pid>
  python resources/tools/debug/stack_sample.py <map> --name DishonoredGame_X.exe           (first process of that image name)
  python resources/tools/debug/stack_sample.py <map> --after <seconds> [--kill] <exe> [game args...]   (launch, wait, sample)

--kill ends a launched process after the sample (W's original behaviour); without it the process keeps running.
The .map is the one the exe was linked with (staged next to the exe by stage_retail.py). Sampling a process with the
Debug crash filter (Launch.cpp) or dbg.py attached is fine; both only read.
"""
import ctypes
import ctypes.wintypes as wt
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from dbg import MapSymbols, dump_stack, exe_image_base, k32, process_threads  # noqa: E402


class PROCESSENTRY32W(ctypes.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ProcessID", wt.DWORD), ("th32DefaultHeapID", ctypes.c_void_p),
                ("th32ModuleID", wt.DWORD), ("cntThreads", wt.DWORD), ("th32ParentProcessID", wt.DWORD), ("pcPriClassBase", wt.LONG),
                ("dwFlags", wt.DWORD), ("szExeFile", wt.WCHAR * 260)]


def pid_by_name(name: str) -> int | None:
    snap = k32.CreateToolhelp32Snapshot(2, 0)
    pe = PROCESSENTRY32W()
    pe.dwSize = ctypes.sizeof(pe)
    ok = k32.Process32FirstW(snap, ctypes.byref(pe))
    found = None
    while ok and found is None:
        if pe.szExeFile.lower() == name.lower():
            found = pe.th32ProcessID
        ok = k32.Process32NextW(snap, ctypes.byref(pe))
    k32.CloseHandle(snap)
    return found


def sample(pid: int, syms: MapSymbols) -> int:
    hproc = k32.OpenProcess(0x1F0FFF, False, pid)
    if not hproc:
        print(f"OpenProcess({pid}) failed", ctypes.get_last_error())
        return 2
    image_base = exe_image_base(hproc)
    threads = process_threads(pid)
    for _, h in threads:
        k32.Wow64SuspendThread(h)
    print(f"pid {pid}: {len(threads)} threads, image base 0x{image_base:x}")
    for tid, h in threads:
        print(f" thread {tid}:")
        dump_stack(hproc, h, image_base, syms, depth=40)
    for _, h in threads:
        k32.ResumeThread(h)
        k32.CloseHandle(h)
    k32.CloseHandle(hproc)
    return 0


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(__doc__)
        return 2
    syms = MapSymbols(Path(argv[0]))
    mode = argv[1]
    if mode == "--pid":
        return sample(int(argv[2]), syms)
    if mode == "--name":
        pid = pid_by_name(argv[2])
        if pid is None:
            print(f"no process named {argv[2]}")
            return 2
        return sample(pid, syms)
    if mode == "--after":
        rest = argv[3:]
        kill = rest[:1] == ["--kill"]
        if kill:
            rest = rest[1:]
        if not rest:
            print(__doc__)
            return 2
        exe = Path(rest[0])
        proc = subprocess.Popen([str(exe), *rest[1:]], cwd=str(exe.parent))
        time.sleep(float(argv[2]))
        if proc.poll() is not None:
            print(f"process exited before the sample, code {proc.returncode}")
            return 1
        rc = sample(proc.pid, syms)
        if kill:
            proc.kill()
            proc.wait()
            print("killed")
        return rc
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
