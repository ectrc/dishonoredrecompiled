# Agent workflow (wave 3 onwards)

One working tree, many agents. These rules keep runs and edits from colliding.

## Build

```
build\agent<X>_configure.cmd  (write it with the Write tool)
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64 -no_logo
  cmake -S D:\RecompileDishonored\Recompile -B D:\RecompileDishonored\Recompile\build\agent<X> -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-x86.cmake -DDISHONORED_REAL_LAUNCH=ON -DDISHONORED_ENABLE_GFXUI=ON -DDISHONORED_ENABLE_AKAUDIO=ON -DDISHONORED_ENABLE_OSS=ON -DDISHONORED_ENABLE_DISHONOREDGAME=ON
  cmake --build D:\RecompileDishonored\Recompile\build\agent<X> --target DishonoredGame -- -k 0
```

Run it with the PowerShell tool (`& cmd /c "<path>"`). Put every `-D` on the cmake line inside the `.cmd`:
`cmd` splits `-DX=Y` script arguments at `=`. If other agents' in-flight edits break your build, snapshot
HEAD + your files (`git worktree add --detach build\agent<X>_wt HEAD`, copy your files over it, build there).
**A worktree must never contain a stage directory or a junction** (see below).

## Run

Only through the smoke tool, with your own exe name, log name and ini directory:

```
python resources\tools\build_and_smoke.py --build-dir build\agent<X> --no-build ^
  --exe-name DishonoredGame_<X>.exe --log-name agent<X>.log --ini-dir build\agent<X>\config ^
  --rhi null --milestone "<golden line>" --expect "<our line>" [--skip-native A,B] [--extra-args "..."]
```

By hand, the same isolation switches are mandatory: `-LOG=agent<X>.log -ENGINEINI=<dir>\DishonoredEngine.ini
-GAMEINI=<dir>\DishonoredGame.ini -INPUTINI=<dir>\DishonoredInput.ini -UIINI=<dir>\DishonoredUI.ini`.
The exe is staged **into the retail tree** `D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32` next
to `Dishonored.exe`; logs land in `DishonoredGame\Logs\agent<X>.log`.

- Never delete anything under `Dishonored_Latest2026`. Yours are only `DishonoredGame_<X>.exe/.pdb/.map`,
  your log, and the inis in your `--ini-dir`.
- Never create junctions or symlinks into the retail or reference trees, and never recursively delete a
  directory that may contain one (`rm -rf`, `Remove-Item -Recurse`, `git worktree remove`). On 2026-09-25
  such a delete went through staging junctions and wiped the retail content. Links are removed with
  `python resources\tools\unlink_junctions.py <dir> --apply` only.
- A `DishonoredGame.exe` under 1 MB is the `DishonoredLaunchStub` build; `stage_retail.py` refuses it.
- `-strictnatives` makes every unported native abort instead of warning once (the golden run uses it).

## Evidence and reporting

- IDA: private copies `resources\docs\idb\shipping2012_agent<X>.i64` (from `shipping2012_v1.i64`) and
  `retail2013_agent<X>.i64` (from `retail2013_named.i64`); `python resources\tools\ida\run.py
  resources\tools\ida\decompile_funcs.py <db> <out_dir> <name|re:regex|rva:0x...>`; never another agent's copy,
  never the IDA MCP tools (they hold one shared database).
- The retail 2013 exe is the target: cite the 2013 rva (`match_2012_2013.csv` maps 2012 → 2013); the 2012
  decompile is the readable version of the same function.
- Every edit: `// DISHONORED(port|written|layout|retail|bringup): <evidence>`. No commits, no `git add`.
- Report to `resources\docs\agents\agent<X>.md`: what changed, evidence, results with the exact commands,
  what is left, follow-ups outside your files; say which build every number comes from.
- Tool hygiene: the Bash tool mangles backslashes in heredocs — write scripts with the Write tool;
  sources are CRLF; `git add -A` is banned.
