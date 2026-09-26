# Debug tools (`resources/tools/debug/`)

Promoted in wave 4 (agent AK) from the per-agent helpers of wave 3: agent W's `stack_sample.py`, agent X's
`agentX_dbg.py` / `agentX_dbgrun.py`, agent Z's `zdbg.py` (`--hang`, `--attach`). Pure Python + ctypes, no
debugger install; a 64-bit Python debugs the x86 exe through the WOW64 context calls.

| Tool | Stops on | Output |
|---|---|---|
| `dbg.py` | first access violation / any real exception; every `appErrorf`, `appError`, `check` failure (with a debugger attached `FOutputDeviceWindowsError::Serialize` calls `DebugBreak()` = 0x80000003 inside the image); `--hang=N` seconds without exit | registers + EBP-chain stack of the faulting thread symbolized through the linker `.map`; on a hang the stacks of every thread; exit 0 game exited, 1 exception, 2 launch failure, 3 hang |
| `dbg.py --attach=PID <map>` | nothing: dumps every thread of a running exe once and detaches | same stack format |
| `stack_sample.py` | nothing: suspends, prints every thread's stack, resumes (`--pid`, `--name DishonoredGame_X.exe`, or `--after S <exe> args…` to launch first) | repeatable samples of a stall (`appSleep` spin in an async loader, a lock) |
| `dbgrun.py` | wraps `dbg.py` with `stage_retail.py` and the `build_and_smoke.py` isolation switches (`--exe-name`, `--log-name`, `--ini-dir`, `--rhi`, `--skip-native`, `--extra-args`) | `<build-dir>/dbg/<tag>.txt` (stacks) + `<tag>.log` (copy of the agent's Launch.log, `-forcelogflush` on) |

## Typical use

```
# crash or assert: where did it happen?
python resources/tools/debug/dbgrun.py --build-dir build/agentAD --exe-name DishonoredGame_AD.exe --log-name agentAD.log ^
       --ini-dir build/agentAD/config --rhi null --skip-native OnlineSubsystemPC --hang 120 --tag navmesh1

# hang / stall: sample the running smoke without stopping it
python resources/tools/debug/stack_sample.py build/agentAD/Binaries/Win32/DishonoredGame.map --name DishonoredGame_AD.exe

# one-shot dump of a process you already have (PID from tasklist), then it keeps running
python resources/tools/debug/dbg.py --attach=12345 build/agentAD/Binaries/Win32/DishonoredGame.map
```

## Speed

A debugged run is several times slower than the same smoke run: with a debugger attached `FOutputDeviceDebug` is active and
every log line becomes an `OutputDebugString` debug event (the null-RHI baseline reaches its `appErrorf` in 27 s alone and
in 2–4 minutes under `dbg.py`). `dbgrun.py` therefore defaults to `--hang 400`; a hang dump that shows the main thread in
`appOutputDebugString` / `FOutputDeviceConsoleWindows::Serialize` means "still running", not a hang. `stack_sample.py`
does not attach, so the sampled process runs at full speed.

## What the stack means

- Symbols come from the `.map` next to the exe (`build\agent<X>\Binaries\Win32\DishonoredGame.map`; `stage_retail.py` copies
  `.exe`, `.pdb` and `.map` together). `Name+0x1c` is the nearest public symbol at or below the return address; static
  functions resolve to the preceding public, so read the offsets with the size of the previous function in mind.
- The EBP chain needs frame pointers. Debug builds keep them; a frame from a `/Oy` function or from a system DLL ends the
  chain (`(outside exe)`), so a stack that stops at `KERNELBASE`/`ntdll` is the wait/sleep the thread is in, not a bug.
- Breakpoints: `appDebugBreak()` is `DebugBreak()` when `IsDebuggerPresent()` (`Core.h`, `_MSC_VER` Debug branch), and an
  `int 3` in 32-bit code reaches a 64-bit debugger as `0x4000001F` (`STATUS_WX86_BREAKPOINT`), the same code as the
  WOW64 loader's initial breakpoint. `dbg.py` therefore classifies a breakpoint by its stack: no frame inside the exe
  image → loader/system breakpoint, continue; a frame of ours below it → the game's `appDebugBreak()`, stop. (Agent X's
  loop stopped on the WOW64 breakpoint after start-up; agent Z's skipped every `0x4000001F` and so never stopped on an
  `appErrorf`.)
- `appErrorf/DebugBreak 0x80000003|0x4000001f` stops carry the message in the Launch.log (`appError called: …`, flushed by
  `-forcelogflush`); the stack shows the caller of `appFailAssertFunc` / `FOutputDeviceWindowsError::Serialize`.
- Access violations print `read|write at <address>`: `0x00000000..0xffff` is a NULL-ish dereference, `0xcdcdcdcd` /
  `0xdddddddd` the MSVC debug-heap fill of uninitialized / freed memory, `0x3f800000`-like values a float read where a
  pointer was expected (a serializer out of phase).
- Without a debugger the Debug exe's own filter (`Launch.cpp` `DishonoredDebugUnhandledException`, wave 4 pre-wave) logs an
  access violation on any thread through `CreateMiniDump` + `GError->HandleError()`; these tools add the symbolized
  stack and the stop-before-exit.

## Rules

The exe runs from the retail `Binaries\Win32` (staged copy `DishonoredGame_<X>.exe`), with `-LOG=agent<X>.log` and the
`-*INI=` switches of the agent's `--ini-dir`; never another agent's exe, log or ini. Nothing here writes into the
retail tree beyond that log and the staged exe.
