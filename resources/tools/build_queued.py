"""Every build of this project goes through here, so builds queue instead of racing, and so a build
never takes more than half the machine.

  python resources/tools/build_queued.py --build-dir build/release [--target DishonoredGame] [-- extra ninja args]

Two things, both asked for by the user after four agents building at once pinned every core:

1. **One build at a time, machine-wide.** A lock file at %LOCALAPPDATA%\\Temp\\dishonored_build.lock is
   taken with O_EXCL before cmake runs and released after. A second build waits, printing a line every
   30 s saying whose build it is waiting on. A lock whose owning PID is dead is stale and is removed.
   The lock is per machine, not per worktree, because that is what the CPU is.

2. **Half the CPU, as a hard cap.** ninja gets -j <cores/2> AND the build runs with its affinity mask
   set to the low half of the logical cores, so cl.exe cannot spread onto the other half however many
   jobs ninja starts. The mask and the priority class (below normal) are set on THIS process before
   cmake is spawned, and child processes inherit both, so there is no window in which a compiler runs
   unpinned.

Overrides, all by environment variable so a script does not need to know about them:
  DISHONORED_BUILD_CPU_PERCENT   default 50; 100 removes the affinity cap and uses all cores
  DISHONORED_BUILD_JOBS          default cores * percent / 100, minimum 1
  DISHONORED_BUILD_NOLOCK=1      skip the queue (for a build you know is alone)

Exit code is cmake's. Anything that used to call `cmake --build <dir> --target <t> -- -k 0` calls this
instead; the only ones that may not are scripts an agent writes for itself, and the wave plan forbids
those from calling cmake or ninja directly for exactly this reason.
"""
from __future__ import annotations

import argparse
import ctypes
import os
import subprocess
import sys
import time
from pathlib import Path

LOCK = Path(os.environ.get("LOCALAPPDATA", os.environ.get("TEMP", "."))) / "Temp" / "dishonored_build.lock"
if not LOCK.parent.exists():
    LOCK = Path(os.environ.get("TEMP", ".")) / "dishonored_build.lock"

BELOW_NORMAL_PRIORITY_CLASS = 0x00004000
PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
STILL_ACTIVE = 259


def pid_alive(pid: int) -> bool:
    k32 = ctypes.windll.kernel32
    h = k32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
    if not h:
        return False
    try:
        code = ctypes.c_ulong()
        if not k32.GetExitCodeProcess(h, ctypes.byref(code)):
            return False
        return code.value == STILL_ACTIVE
    finally:
        k32.CloseHandle(h)


def acquire(desc: str) -> None:
    waited = 0.0
    last_report = -30.0
    while True:
        try:
            fd = os.open(str(LOCK), os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        except FileExistsError:
            owner = "unknown"
            try:
                owner = LOCK.read_text(encoding="utf-8", errors="replace").strip()
                opid = int(owner.split()[0])
                if not pid_alive(opid):
                    print(f"build_queued: stale lock from dead pid {opid}, removing", flush=True)
                    LOCK.unlink(missing_ok=True)
                    continue
            except (ValueError, IndexError, OSError):
                pass
            if waited - last_report >= 30.0:
                print(f"build_queued: waiting for another build to finish ({owner}) - {int(waited)} s so far",
                      flush=True)
                last_report = waited
            time.sleep(2.0)
            waited += 2.0
            continue
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            f.write(f"{os.getpid()} {time.strftime('%H:%M:%S')} {desc}\n")
        if waited:
            print(f"build_queued: lock acquired after {int(waited)} s", flush=True)
        return


def release() -> None:
    try:
        text = LOCK.read_text(encoding="utf-8", errors="replace")
        if text.split()[0] == str(os.getpid()):
            LOCK.unlink()
    except (OSError, IndexError):
        pass


def cap_cpu(percent: int) -> tuple[int, int, int]:
    """Pins this process (and so every child) to the low `percent` of logical cores.
    Returns (cores used, jobs, mask actually in effect as read back from the OS)."""
    cores = os.cpu_count() or 1
    use = max(1, (cores * percent) // 100)
    # Typed deliberately. The first version let ctypes guess, and GetCurrentProcess()'s pseudo-handle
    # (-1) came back as a 32-bit int, so SetProcessAffinityMask was handed an invalid HANDLE and failed
    # silently - the test run printed "8 of 16 cores (affinity)" while running on all sixteen.
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    k32.GetCurrentProcess.restype = ctypes.c_void_p
    k32.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    k32.SetProcessAffinityMask.restype = ctypes.c_int
    k32.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t),
                                           ctypes.POINTER(ctypes.c_size_t)]
    k32.GetProcessAffinityMask.restype = ctypes.c_int
    k32.SetPriorityClass.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    k32.SetPriorityClass.restype = ctypes.c_int
    me = k32.GetCurrentProcess()
    if percent < 100 and use < cores:
        mask = (1 << use) - 1
        if not k32.SetProcessAffinityMask(me, mask):
            print(f"build_queued: SetProcessAffinityMask failed (error {ctypes.get_last_error()}), "
                  f"building unpinned", flush=True)
        k32.SetPriorityClass(me, BELOW_NORMAL_PRIORITY_CLASS)
    proc_mask = ctypes.c_size_t()
    sys_mask = ctypes.c_size_t()
    in_effect = proc_mask.value if k32.GetProcessAffinityMask(me, ctypes.byref(proc_mask),
                                                                ctypes.byref(sys_mask)) else 0
    jobs_env = os.environ.get("DISHONORED_BUILD_JOBS")
    jobs = max(1, int(jobs_env)) if jobs_env else use
    return use, jobs, in_effect


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--build-dir", required=True)
    ap.add_argument("--target", default="DishonoredGame")
    ap.add_argument("extra", nargs="*", help="extra arguments for ninja, after --")
    args = ap.parse_args()

    # A machine-wide override file, so the user can lift the cap and the queue for the night without
    # restarting every agent whose shell already inherited the old environment. Lines of KEY=VALUE for
    # the three DISHONORED_BUILD_* names; an explicit environment variable still wins.
    override = LOCK.parent / "dishonored_build_overrides.txt"
    if override.exists():
        for line in override.read_text(encoding="utf-8", errors="replace").splitlines():
            if "=" in line and line.strip().startswith("DISHONORED_BUILD_"):
                k, v = line.strip().split("=", 1)
                os.environ.setdefault(k.strip(), v.strip())
        print(f"build_queued: overrides from {override}", flush=True)

    percent = int(os.environ.get("DISHONORED_BUILD_CPU_PERCENT", "50"))
    percent = min(100, max(1, percent))
    build_dir = Path(args.build_dir)
    desc = f"{build_dir} {args.target}"

    use, jobs, in_effect = cap_cpu(percent)
    cores = os.cpu_count() or 1
    pinned = bin(in_effect).count("1") if in_effect else cores
    print(f"build_queued: {desc}: affinity mask 0x{in_effect:x} = {pinned} of {cores} cores, -j {jobs}, "
          f"below-normal priority", flush=True)
    if percent < 100 and pinned != use:
        print(f"build_queued: WARNING the OS reports {pinned} cores in the mask, not the {use} asked for",
              flush=True)

    nolock = os.environ.get("DISHONORED_BUILD_NOLOCK") == "1"
    if not nolock:
        acquire(desc)
    try:
        cmd = ["cmake", "--build", str(build_dir), "--target", args.target, "--", "-k", "0", "-j", str(jobs)]
        cmd += [a for a in args.extra if a != "--"]
        return subprocess.call(cmd)
    finally:
        if not nolock:
            release()


if __name__ == "__main__":
    sys.exit(main())
