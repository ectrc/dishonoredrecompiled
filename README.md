# Dishonored Recompilation

**It boots to its own main menu and plays into the first mission.** From the shipped executable: NEW
GAME, a difficulty, the brightness screen, YES — and Corvo is standing on the boat landing at Dunwall
Tower, with the HUD drawn and a guard walking a patrol route. The interface is most of the way to
matching retail's; what is still wrong is listed in `resources/docs/STATUS.md`.

Functional (non-matching) rebuild of Dishonored's Win32 native executable, targeting the **retail
2013 build** (`Dishonored.exe`, engine 9411, DLC05–07): its layouts, serialization and behavior are
the contract. The symbolized 2012 QA build only supplies names and decompiles; its structs differ
from 2013 in many places and are never the final word. The base is the UE3 build 10897 source tree at `../UnrealEngine3`
(CodeRedModding/UnrealEngine3), a very close engine build to Dishonored's 9014/9411; see
`resources/docs/engine_reference.md`. Inputs: the retail 2013 build (`../Dishonored_Latest2026`,
the target) and the symbolized 2012 QA build (`../Dishonored_Debug2012`, the helping hand for
names and decompiles). Game binaries, PDBs, content, IDA databases and the reference tree stay
outside the repo.

Start with [resources/docs/STATUS.md](resources/docs/STATUS.md) — what works, what does not, and the
pitfalls that cost a wave each to learn. [PLAN.md](PLAN.md) has the overall strategy and phases;
`resources/docs/PHASE1.md`–`PHASE13.md` are the per-wave trackers, and `resources/docs/agents/` holds
every package's own report and measurements. Symbol exports live in `resources/docs/symbols/` and
`resources/docs/types/`; scripts that produce them are under `resources/tools/`.

## Where it is

| | |
|---|---|
| Engine, renderer, script VM | up; the first mission map renders under D3D9 and the pawn walks it |
| Main menu | draws, navigates by mouse and keyboard, and starts the game |
| In-game HUD | health and mana vials and the stance icon, fed from the live pawn |
| AI | a guard adopts a patrol route and walks it |
| Save/load | all 51 retail saves load; **86.7 %** of `Dishonored0.sav`'s object stream restores |
| Audio | deliberately last — the silent backend loads the real banks and resolves the real events |
| Full campaign | not yet |

Every merge is gated on a clean checkout of its own commit by a 37-check regression harness
(`resources/tools/run_regression.py`): build, layout `static_assert`s against retail's offsets, a null-RHI
smoke, a D3D9 render census and a scripted input run.

## Running it

The playable build is one command. From the repo root, in any shell:

```
resources\build-play.cmd
```

That configures and builds a Release x86 binary with every module on and stages it into your retail
install, beside the game's own executable:

```
<retail install>\Binaries\Win32\DishonoredGame-Win64-Shipping.exe
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

Logs land in `<retail install>\DishonoredGame\Logs\Launch.log`; pass `-LOG=<name>.log` to separate a
run, and `-forcelogflush` if you need the tail of a run that is still going.

The build scripts and the tools read the retail install, the 2012 QA build and the reference engine
tree from paths set in `cmake/` and `resources/tools/`; point those at your own copies. Nothing in
this repository ships game binaries, PDBs, content or IDA databases.

### Other builds

| | |
|---|---|
| `resources\build-game.cmd [target]` | the development build, every option on, into `build\game` |
| `resources\build-release.cmd [target]` | the Release build the regression harness uses |
| `python resources\tools\run_regression.py --build-dir build/<dir>` | the 37-check regression gate |

## State of play

`resources/docs/STATUS.md` is the thing to read when resuming: what works, what does not, and the
pitfalls that cost a wave each to learn.

## References and credits

**The reference engine source**

- [CodeRedModding/UnrealEngine3](https://github.com/CodeRedModding/UnrealEngine3) — the UE3 build
  10897 tree this is rebuilt from, a very close engine build to Dishonored's 9014/9411. Without it
  there is no project. See `resources/docs/engine_reference.md`.
- [CodeRed-Generator](https://github.com/CodeRedModding/CodeRed-Generator) — produced the retail SDK
  dump (`DishonoredSDK 1.4`) that gives the runtime offset of every reflected member, which is what
  the 2,314 layout `static_assert`s are written against. See `resources/docs/sdk_dump.md`.

**Tools**

- [IDA Pro and the Hex-Rays decompiler](https://hex-rays.com/) — the disassembly and decompiles both
  builds are read with.
- [Diaphora](https://github.com/joxeankoret/diaphora) — binary diffing between the 2012 and 2013
  builds.
- [dismod](https://github.com/ectrc/dismod) — hooks the running retail game, which is how several
  findings here were checked against the real thing rather than against a disassembly. See
  `resources/docs/dismod_harness.md`.

**Third-party code this build fetches**

- [zlib](https://github.com/madler/zlib) · [libpng](https://github.com/pnggroup/libpng) ·
  [nvapi](https://github.com/NVIDIA/nvapi) · [lzokay](https://github.com/jackoalan/lzokay)

**Middleware the game links**

PhysX 2.8.4, Scaleform GFx 3.3, Wwise 2012.1, Bink, FaceFX, Steamworks and libcurl. **No vendor SDK is
downloaded and nothing is redistributed**: the bindings in `source/Development/Src/External/` were
written from the DLLs that ship with the game and their exported symbols. See
`resources/docs/middleware.md`.

## Licence

**[GNU Affero General Public License v3.0](LICENSE)** — the strongest copyleft there is. If you build
on this, your work has to be open source too, under the same licence, including over a network: AGPL
section 13 means that if you let people use a modified version remotely, they are entitled to its
source. That is deliberate. This exists so the work is shared, not enclosed.

In short: use it, change it, ship it — but ship the source with it.

The licence covers **this repository's code**. It cannot and does not grant you anything over
Dishonored itself: the game is © Bethesda Softworks / Arkane Studios, this is an independent
reimplementation for study, it ships no game code, assets, binaries or PDBs, and it needs your own
legally obtained copy of the game to run.

All generated with claude code.

<img width="3202" height="941" alt="image" src="https://github.com/user-attachments/assets/3928471b-3744-4573-9abc-5a31e8596e6d" />
<img width="3204" height="936" alt="image" src="https://github.com/user-attachments/assets/6ba9858a-c7b2-4e73-bbf0-e84c296631cb" />
<img width="3218" height="944" alt="image" src="https://github.com/user-attachments/assets/c13eacaa-b430-43e3-a9fb-3507cc14346a" />
<img width="3205" height="934" alt="image" src="https://github.com/user-attachments/assets/428f5a65-a2fc-406a-9906-26d577982b61" />
<img width="3214" height="937" alt="image" src="https://github.com/user-attachments/assets/6524153a-08a6-42ab-b2ae-89df491f320d" />
<img width="1621" height="965" alt="image" src="https://github.com/user-attachments/assets/27976b10-fc5c-44df-bb07-78607ff69bbb" />
