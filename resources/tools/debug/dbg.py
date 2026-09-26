"""Win32 debug loop for the x86 DishonoredGame exe (agents X and Z's `agentX_dbg.py` / `zdbg.py`, promoted).

Runs (or attaches to) the exe as its debugger and stops on the first real exception: an access violation, an
`appErrorf` (with a debugger attached FOutputDeviceWindowsError::Serialize calls DebugBreak(), so every appErrorf /
appError / check failure is a 0x80000003 inside the image) or any other non-initial exception. It prints the exception,
the registers and the EBP-chain call stack of the faulting thread symbolized through the linker .map, then kills the
process. After --hang seconds without an exit every thread is suspended and its stack dumped (hang sampler).

Usage:
  python resources/tools/debug/dbg.py [--hang=SECONDS] <exe> <map> <cwd> [game args...]
  python resources/tools/debug/dbg.py --attach=PID <map>                     (dump every thread of a running exe, then detach)

Exit code: 0 the game exited by itself (its exit code is printed), 1 exception stop, 2 launch/attach failure, 3 hang dump.
The .map is the one the exe was linked with (build\\agent<X>\\Binaries\\Win32\\DishonoredGame.map, staged next to the exe by
stage_retail.py). The EBP chain needs frame pointers: Debug builds have them; a frame without one ends the chain early.
Agents run this through dbgrun.py, which stages the exe and adds the per-agent isolation switches.
"""
import ctypes
import ctypes.wintypes as wt
import struct
import sys
import time
from bisect import bisect_right
from pathlib import Path

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
IS64 = struct.calcsize("P") == 8
P = 8 if IS64 else 4

DEBUG_ONLY_THIS_PROCESS = 0x00000002
EXCEPTION_DEBUG_EVENT = 1
CREATE_THREAD_DEBUG_EVENT = 2
CREATE_PROCESS_DEBUG_EVENT = 3
EXIT_THREAD_DEBUG_EVENT = 4
EXIT_PROCESS_DEBUG_EVENT = 5
OUTPUT_DEBUG_STRING_EVENT = 8
DBG_CONTINUE = 0x00010002
DBG_EXCEPTION_NOT_HANDLED = 0x80010001

STATUS_BREAKPOINT = 0x80000003
STATUS_WX86_BREAKPOINT = 0x4000001F
STATUS_WX86_SINGLE_STEP = 0x4000001E
STATUS_ACCESS_VIOLATION = 0xC0000005
MS_VC_THREAD_NAME = 0x406D1388
MS_CXX_EXCEPTION = 0xE06D7363
CONTEXT_X86_FULL = 0x10007


class STARTUPINFOW(ctypes.Structure):
    _fields_ = [("cb", wt.DWORD), ("lpReserved", wt.LPWSTR), ("lpDesktop", wt.LPWSTR), ("lpTitle", wt.LPWSTR),
                ("dwX", wt.DWORD), ("dwY", wt.DWORD), ("dwXSize", wt.DWORD), ("dwYSize", wt.DWORD),
                ("dwXCountChars", wt.DWORD), ("dwYCountChars", wt.DWORD), ("dwFillAttribute", wt.DWORD),
                ("dwFlags", wt.DWORD), ("wShowWindow", wt.WORD), ("cbReserved2", wt.WORD),
                ("lpReserved2", ctypes.c_void_p), ("hStdInput", wt.HANDLE), ("hStdOutput", wt.HANDLE), ("hStdError", wt.HANDLE)]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [("hProcess", wt.HANDLE), ("hThread", wt.HANDLE), ("dwProcessId", wt.DWORD), ("dwThreadId", wt.DWORD)]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [("dwDebugEventCode", wt.DWORD), ("dwProcessId", wt.DWORD), ("dwThreadId", wt.DWORD)] + ([("pad", wt.DWORD)] if IS64 else []) + [("u", ctypes.c_ubyte * 256)]


class X86_CONTEXT(ctypes.Structure):
    """x86 CONTEXT (WOW64_CONTEXT when the debugger is a 64-bit Python): 716 bytes."""
    _fields_ = [("ContextFlags", wt.DWORD), ("Dr", wt.DWORD * 6), ("FloatSave", ctypes.c_ubyte * 112),
                ("SegGs", wt.DWORD), ("SegFs", wt.DWORD), ("SegEs", wt.DWORD), ("SegDs", wt.DWORD),
                ("Edi", wt.DWORD), ("Esi", wt.DWORD), ("Ebx", wt.DWORD), ("Edx", wt.DWORD), ("Ecx", wt.DWORD), ("Eax", wt.DWORD),
                ("Ebp", wt.DWORD), ("Eip", wt.DWORD), ("SegCs", wt.DWORD), ("EFlags", wt.DWORD), ("Esp", wt.DWORD), ("SegSs", wt.DWORD),
                ("Ext", ctypes.c_ubyte * 512)]


class THREADENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ThreadID", wt.DWORD), ("th32OwnerProcessID", wt.DWORD),
                ("tpBasePri", wt.LONG), ("tpDeltaPri", wt.LONG), ("dwFlags", wt.DWORD)]


get_ctx = k32.Wow64GetThreadContext if IS64 else k32.GetThreadContext
get_ctx.argtypes = [wt.HANDLE, ctypes.POINTER(X86_CONTEXT)]
rpm = k32.ReadProcessMemory
rpm.argtypes = [wt.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
k32.WaitForDebugEvent.argtypes = [ctypes.POINTER(DEBUG_EVENT), wt.DWORD]
k32.ContinueDebugEvent.argtypes = [wt.DWORD, wt.DWORD, wt.DWORD]
k32.SuspendThread.argtypes = [wt.HANDLE]
k32.OpenThread.restype = wt.HANDLE
k32.OpenProcess.restype = wt.HANDLE
k32.CreateToolhelp32Snapshot.restype = wt.HANDLE


def uptr(buf: bytes, off: int) -> int:
    return struct.unpack_from("<Q" if IS64 else "<I", buf, off)[0]


class MapSymbols:
    """`Publics by Value` of a linker .map: sorted (VA, name) for nearest-symbol lookup."""

    def __init__(self, path: Path):
        self.base = 0x400000
        syms = []
        in_publics = False
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            if line.startswith(" Preferred load address is"):
                self.base = int(line.split()[-1], 16)
            if "Publics by Value" in line:
                in_publics = True
                continue
            if in_publics:
                parts = line.split()
                if len(parts) >= 3 and ":" in parts[0]:
                    try:
                        syms.append((int(parts[2], 16), parts[1]))
                    except ValueError:
                        continue
        syms.sort()
        self.addrs = [s[0] for s in syms]
        self.names = [s[1] for s in syms]

    def symbolize(self, addr: int, image_base: int) -> str:
        if not (image_base <= addr < image_base + 0x08000000):
            return f"0x{addr:08x} (outside exe)"
        va = addr - image_base + self.base
        i = bisect_right(self.addrs, va) - 1
        if i < 0:
            return f"0x{addr:08x}"
        return f"0x{addr:08x} {self.names[i]}+0x{va - self.addrs[i]:x}"


def dump_stack(hproc, hthread, image_base: int, syms: MapSymbols, depth: int = 64) -> None:
    ctx = X86_CONTEXT()
    ctx.ContextFlags = CONTEXT_X86_FULL
    if not get_ctx(hthread, ctypes.byref(ctx)):
        print("  (no thread context)", ctypes.get_last_error())
        return
    print(f"  EIP=0x{ctx.Eip:08x} ESP=0x{ctx.Esp:08x} EBP=0x{ctx.Ebp:08x} EAX=0x{ctx.Eax:08x} ECX=0x{ctx.Ecx:08x} EDX=0x{ctx.Edx:08x} ESI=0x{ctx.Esi:08x} EDI=0x{ctx.Edi:08x}")
    print("   ", syms.symbolize(ctx.Eip, image_base))
    ebp = ctx.Ebp
    for _ in range(depth):
        buf = (ctypes.c_ubyte * 8)()
        n = ctypes.c_size_t()
        if not rpm(hproc, ctypes.c_void_p(ebp), buf, 8, ctypes.byref(n)) or n.value != 8:
            break
        next_ebp, ret = struct.unpack("<II", bytes(buf))
        if ret == 0:
            break
        print("   ", syms.symbolize(ret, image_base))
        if next_ebp <= ebp:
            break
        ebp = next_ebp


def stack_has_exe_frame(hproc, hthread, image_base: int, depth: int = 64) -> bool:
    """True when the EBP chain returns into the exe image: a breakpoint raised by the game's own appDebugBreak()
    (DebugBreak lives in ntdll32, so the exception address itself is always outside the image), not by the loader."""
    ctx = X86_CONTEXT()
    ctx.ContextFlags = CONTEXT_X86_FULL
    if not get_ctx(hthread, ctypes.byref(ctx)):
        return False
    ebp = ctx.Ebp
    for _ in range(depth):
        buf = (ctypes.c_ubyte * 8)()
        n = ctypes.c_size_t()
        if not rpm(hproc, ctypes.c_void_p(ebp), buf, 8, ctypes.byref(n)) or n.value != 8:
            return False
        next_ebp, ret = struct.unpack("<II", bytes(buf))
        if image_base <= ret < image_base + 0x08000000:
            return True
        if ret == 0 or next_ebp <= ebp:
            return False
        ebp = next_ebp
    return False


def process_threads(pid: int) -> list[tuple[int, int]]:
    """(tid, handle) of every thread of pid (Toolhelp), for --attach where no CREATE_THREAD events were seen."""
    out = []
    snap = k32.CreateToolhelp32Snapshot(4, 0)
    te = THREADENTRY32()
    te.dwSize = ctypes.sizeof(te)
    ok = k32.Thread32First(snap, ctypes.byref(te))
    while ok:
        if te.th32OwnerProcessID == pid:
            out.append((te.th32ThreadID, k32.OpenThread(0x1FFFFF, False, te.th32ThreadID)))
        ok = k32.Thread32Next(snap, ctypes.byref(te))
    k32.CloseHandle(snap)
    return out


def exe_image_base(hproc) -> int:
    psapi = ctypes.WinDLL("psapi", use_last_error=True)
    mods = (wt.HMODULE * 1024)()
    need = wt.DWORD()
    if psapi.EnumProcessModulesEx(hproc, mods, ctypes.sizeof(mods), ctypes.byref(need), 1) and mods[0]:  # LIST_MODULES_32BIT
        return mods[0]
    return 0x400000


def dump_all(hproc, threads: dict, image_base: int, syms: MapSymbols, reason: str) -> None:
    print(f"{reason}: stacks of {len(threads)} threads")
    for tid, h in threads.items():
        if h:
            k32.SuspendThread(wt.HANDLE(h))
    for tid, h in threads.items():
        if h:
            print(f" thread {tid}:")
            dump_stack(hproc, wt.HANDLE(h), image_base, syms)


def attach_dump(pid: int, syms: MapSymbols) -> int:
    hproc = k32.OpenProcess(0x1F0FFF, False, pid)
    if not hproc:
        print("OpenProcess failed", ctypes.get_last_error())
        return 2
    threads = dict(process_threads(pid))
    dump_all(hproc, threads, exe_image_base(hproc), syms, f"ATTACH pid {pid}")
    for h in threads.values():
        k32.ResumeThread(wt.HANDLE(h))
    print("resumed, detached (the process keeps running)")
    return 0


def debug_loop(exe: str, args: list[str], cwd: str, syms: MapSymbols, hang: float | None) -> int:
    si = STARTUPINFOW()
    si.cb = ctypes.sizeof(si)
    pi = PROCESS_INFORMATION()
    cmd = " ".join([f'"{exe}"'] + args)
    if not k32.CreateProcessW(exe, cmd, None, None, False, DEBUG_ONLY_THIS_PROCESS, None, cwd, ctypes.byref(si), ctypes.byref(pi)):
        print("CreateProcess failed", ctypes.get_last_error())
        return 2
    deadline = time.time() + hang if hang else None
    threads = {pi.dwThreadId: pi.hThread}
    image_base = 0x400000
    ev = DEBUG_EVENT()
    while True:
        if deadline and time.time() > deadline:
            dump_all(pi.hProcess, threads, image_base, syms, f"HANG: {hang:g}s elapsed")
            k32.TerminateProcess(pi.hProcess, 1)
            return 3
        if not k32.WaitForDebugEvent(ctypes.byref(ev), 500):
            continue
        code = ev.dwDebugEventCode
        u = bytes(ev.u)
        status = DBG_CONTINUE
        if code == CREATE_PROCESS_DEBUG_EVENT:
            image_base = uptr(u, 3 * P)
            threads[ev.dwThreadId] = uptr(u, 2 * P)
            print(f"process {ev.dwProcessId} created, image base 0x{image_base:x}")
        elif code == CREATE_THREAD_DEBUG_EVENT:
            threads[ev.dwThreadId] = uptr(u, 0)
        elif code == EXIT_THREAD_DEBUG_EVENT:
            threads.pop(ev.dwThreadId, None)
        elif code == EXIT_PROCESS_DEBUG_EVENT:
            rc = struct.unpack_from("<I", u, 0)[0]
            print(f"process exited, code {rc} (0x{rc:08X})")
            k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status)
            return 0
        elif code == EXCEPTION_DEBUG_EVENT:
            exc_code = struct.unpack_from("<I", u, 0)[0]
            exc_addr = uptr(u, 8 + P)
            nparams = struct.unpack_from("<I", u, 8 + 2 * P)[0]
            info_off = 32 if IS64 else 20
            info = [uptr(u, info_off + i * P) for i in range(min(nparams, 2))]
            first_chance = struct.unpack_from("<I", u, info_off + 15 * P)[0]
            is_breakpoint = exc_code in (STATUS_BREAKPOINT, STATUS_WX86_BREAKPOINT)
            h = threads.get(ev.dwThreadId)
            if exc_code in (MS_VC_THREAD_NAME, MS_CXX_EXCEPTION):
                status = DBG_EXCEPTION_NOT_HANDLED  # SetThreadName / C++ throw: the game's own handlers take them
            elif exc_code == STATUS_WX86_SINGLE_STEP or (is_breakpoint and not (h and stack_has_exe_frame(pi.hProcess, wt.HANDLE(h), image_base))):
                pass  # the loader's initial breakpoints (64-bit ntdll, then WOW64 ntdll32) and system breakpoints: no frame of ours below
            else:
                # a breakpoint with our frames below it is the game's appDebugBreak(): appErrorf / appFailAssert / check
                # (FOutputDeviceWindowsError::Serialize, UnOutputDevices.cpp); int 3 in 32-bit code reaches a 64-bit debugger as 0x4000001F
                extra = f" ({'write' if info and info[0] == 1 else 'read'} at 0x{info[1]:08x})" if exc_code == STATUS_ACCESS_VIOLATION and len(info) == 2 else ""
                kind = "appErrorf/DebugBreak" if is_breakpoint else "EXCEPTION"
                print(f"{kind} 0x{exc_code:08x} at {syms.symbolize(exc_addr, image_base)} first_chance={first_chance}{extra}")
                if h:
                    print("  stack (EBP chain):")
                    dump_stack(pi.hProcess, wt.HANDLE(h), image_base, syms, depth=48)
                else:
                    print("  (no thread handle)")
                k32.TerminateProcess(pi.hProcess, 1)
                k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE)
                return 1
        k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status)


def main(argv: list[str]) -> int:
    hang = None
    attach = None
    while argv and argv[0].startswith("--"):
        opt = argv.pop(0)
        if opt.startswith("--hang="):
            hang = float(opt[7:])
        elif opt.startswith("--attach="):
            attach = int(opt[9:])
        else:
            print(__doc__)
            return 2
    if attach is not None:
        if len(argv) != 1:
            print(__doc__)
            return 2
        return attach_dump(attach, MapSymbols(Path(argv[0])))
    if len(argv) < 3:
        print(__doc__)
        return 2
    exe, mappath, cwd, *args = argv
    return debug_loop(exe, args, cwd, MapSymbols(Path(mappath)), hang)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
