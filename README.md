# Dishonored Recompilation

Not fully complete yet, will pick this up next month

Functional (non-matching) rebuild of Dishonored's Win32 native executable, targeting the **retail
2013 build** (`Dishonored.exe`, engine 9411, DLC05–07): its layouts, serialization and behavior are
the contract. The symbolized 2012 QA build only supplies names and decompiles; its structs differ
from 2013 in many places and are never the final word. The base is the UE3 build 10897 source tree at `../UnrealEngine3`
(CodeRedModding/UnrealEngine3), a very close engine build to Dishonored's 9014/9411; see
`resources/docs/engine_reference.md`. Inputs: the retail 2013 build (`../Dishonored_Latest2026`,
the target) and the symbolized 2012 QA build (`../Dishonored_Debug2012`, the helping hand for
names and decompiles). Game binaries, PDBs, content, IDA databases and the reference tree stay
outside the repo.

Start with [PLAN.md](PLAN.md) for the overall strategy and phases, then
[resources/docs/PHASE1.md](resources/docs/PHASE1.md) for the current task tracker. Symbol exports live in
`resources/docs/symbols/` and `resources/docs/types/`; scripts that produce them are under `resources/tools/`.

## Running it

The playable build is one command. From the repo root, in any shell:

```
resources\build-play.cmd
```

That configures and builds a Release x86 binary with every module on and stages it into the retail
tree as:

```
D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\DishonoredGame-Win64-Shipping.exe
```

**Run it with no arguments and it comes up on the main menu.** `appInit` appends the switches a
playable run needs when they are absent (`Core/Src/UnMisc.cpp`, `DISHONORED_PLAY_DEFAULTS`):
`-gfxuimenu -nosteam -skipnativepkgs=OnlineSubsystemPC -nomovie -windowed -ResX=1600 -ResY=900`.

From the menu: **Space** past the start screen, then **Enter** through NEW GAME, the difficulty screen
(Up/Down to choose) and the brightness screen, then **YES** on the confirmation. That commits the map
change and puts Corvo on the boat landing at Dunwall Tower.

Anything you pass yourself wins over the defaults, so

```
DishonoredGame-Win64-Shipping.exe -startmap=L_Pub_Day_P
```

opens that map directly instead of the menu, and `-ResX=2560 -ResY=1440` overrides the window size.

`resources\build-play.cmd shipping` adds `FINAL_RELEASE` / `SHIPPING_PC_GAME` / `NO_LOGGING`.

**The name is a name, not a description**: the executable is 32-bit, like the retail game. The
middleware it links against — PhysX 2.8.4, Bink, Wwise 2012, Scaleform 3.3 — ships only as 32-bit
DLLs in the retail tree, and the layout `static_assert`s that keep this a faithful recompilation are
written against retail's 32-bit offsets under `/Zp4`.

Logs land in `Dishonored_Latest2026\DishonoredGame\Logs\Launch.log`; pass `-LOG=<name>.log` to
separate a run, and `-forcelogflush` if you need the tail of a run that is still going.

### Other builds

| | |
|---|---|
| `resources\build-game.cmd [target]` | the development build, every option on, into `build\game` |
| `resources\build-release.cmd [target]` | the Release build the regression harness uses |
| `python resources\tools\run_regression.py --build-dir build/<dir>` | the 37-check regression gate |

## State of play

`resources/docs/STATUS.md` is the thing to read when resuming: what works, what does not, and the
pitfalls that cost a wave each to learn.

All generated with claude code.

<img width="3202" height="941" alt="image" src="https://github.com/user-attachments/assets/3928471b-3744-4573-9abc-5a31e8596e6d" />
<img width="3204" height="936" alt="image" src="https://github.com/user-attachments/assets/6ba9858a-c7b2-4e73-bbf0-e84c296631cb" />
<img width="3218" height="944" alt="image" src="https://github.com/user-attachments/assets/c13eacaa-b430-43e3-a9fb-3507cc14346a" />
<img width="3205" height="934" alt="image" src="https://github.com/user-attachments/assets/428f5a65-a2fc-406a-9906-26d577982b61" />
<img width="3214" height="937" alt="image" src="https://github.com/user-attachments/assets/6524153a-08a6-42ab-b2ae-89df491f320d" />
<img width="1621" height="965" alt="image" src="https://github.com/user-attachments/assets/27976b10-fc5c-44df-bb07-78607ff69bbb" />
