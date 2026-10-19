# Agent IL: the render-thread particle crash with the matinee controller list on (wave 26, PHASE23 package IL)

Worktree `build/agentIL_wt` (made at `d2db1e9` = main `b83f983` + PHASE23 lines), build dir `build/agentIL` (RelWithDebInfo). Two
builds, both on that tree: `DishonoredGame_IL1.exe` (instrumentation) and `DishonoredGame_IL2.exe` = `DishonoredGame_IL.exe` (the
fix plus instrumentation, the hand-back tree). 0 errors and 0 C4263/C4264 in `build/agentIL_build1.log` / `agentIL_build2.log`. All
game runs went through `cutscene_compare.py run` / `run_regression.py` (both use `game_input.py launch`, `-windowed -nomovie`). One
start-up diagnosis attached a debugger to a `game_input` launch (`build/agentIL/tools/attach_launch.py`, §3). Decompiles came from
the private copy `build/agentIL/idb/retail2013_IL.i64` (headless, `tools/dec.py`, `tools/vt.py`) into `build/agentIL/decomp2013/`.
fmodel was not used. No commits.

## 0. The answer

**The corruption is a double free, not a stray write.** `DisAttentionInfo_Complex::SetupTickParamsFromTarget` (2013 0xb68e10)
refreshes the player attention info's pawn-attention snapshot, `m_PawnAttentionTweaks`, from the `UDisTweaks_PawnAttention` object.
Our port copied `m_DistanceValueTuning` with a plain struct assignment. In the generated header, `FDisVisCalcTuningParams::m_Points`
is a `BYTE m_Points[12]` placeholder for a `TArray<FDisVisCalcTuningParamsCurvePoint>`, so that assignment copied the TArray header
bit for bit. The snapshot therefore pointed at **the tweaks object's own curve buffer**.

Retail copies the curve field by field and runs `TArray<FDisVisCalcTuningParamsCurvePoint>::operator=` (0x4448d0) on the points,
so its snapshot owns its own buffer (`decomp2013/b68e10.c` line 120, `4448d0.c`). `m_PawnAttentionTweaks` is a reflected property
(retail SDK: `0x0538 (0x00D0) (CPF_NeedCtorLink)`). Every `UDisAttentionInfo_Base::ResetFromTemplate` (GI's ExitProperties +
InitProperties) therefore ran the TArray destructor on the aliased header and called `appFree` on the tweaks' buffer. The pointer
in the tweaks object was left dangling, and the next dirty refresh copied it again, so it was freed again.

On the boat, `UpdateSoireeAttention` makes each of the three boat brains create and drop a player slot **every tick** (§1), so
`ResetFromTemplate` runs three times per frame. Each run frees the same pointer once more. FMallocBinned then hands one block out
twice. The victim is whatever the engine allocates most often, which is the particle dynamic data the game thread builds every
frame. That explains why the crash site moved around (Release / Create / Draw / ClearEmitterDataArray / sprite vertex fill), why a
vtable read garbage `0x90008` (IC1f), and why no particle component did anything on the game thread (IC2b).

**Fix** (`disattentioninfo_complex.cpp`, SetupTickParamsFromTarget): retail's field-by-field copy plus `TArray::operator=` on the
points. **`MatineeControllerAdd` is now always called, as in retail. The `-hbcontrollers` switch is removed and the soirée stim is
always sent** (`-icnostim` stays as an A/B switch).

## 1. Measurements

| run | exe | switches | result |
|---|---|---|---|
| IL1b | IL1 | list on, `-ilnoplayersoiree` (NPC co-actors only) | clean to quit |
| IL1c | IL1 | list on, `-ilnonpcsoiree` (player co-actor only) | Critical, `FParticleSystemSceneProxy::ReleaseRenderThreadResources`, 7 s after the bind (world 56.46) |
| IL1c `-ildiag` | | | `attnslot DishonoredNPCPawn_16/_26/_27 -> DishonoredPlayerPawn info DisAttentionInfo_Complex` is logged again **every tick** (56.46, 56.50, 56.57, 56.62 ...), with a new `m_AttentionInfos` buffer each time |
| IL2g | IL2 | `-ilguardheap -ildiag -icdiag` (list on) | clean to quit. The guard armed at the first controller add: 317,007 guarded allocations (each with a no-access page after its end, decommitted on free), a 48 MB pattern quarantine scanned every tick. **0 FAULT, 0 WRITE AFTER FREE, 0 DOUBLE FREE, 0 FOREIGN FREE, 0 OVERFLOW** (`Logs/ILGuard.log`) |
| IL2a..IL2e | IL2 | default (list + stim on), `-fkdiag -icdiag` | 5/5 with 0 Criticals, all 126 shots to world 175. The scripted quit came at world 190.5 / 181.9 / 196.2 / 175.6 / 194.9 (`wait 205` is not world time) |
| IL2h..IL2l | IL2 | default, `--quit-at 250` | **5/5 with 0 Criticals past world 200**: the last shot is at world 205.0-205.1 and the quit at 211.7 / 231.9 / 215.8 / 243.6 / 243.2 |

`-ildiag` on the removal (IL2g): `remove DishonoredNPCPawn_16 -> DishonoredPlayerPawn level 0 value 0.00 min 0.00 max 1.00 minlevel
soiree 4 clear -0.06 tweaks Twk_Attn_Default_Neutral busted start 1.00`. The soirée minimum is stored (4) but
`GetMinAttentionLevel` loops over types 0..4 only (2013 0x74c6e0, GI's reading), so the accumulator floor stays at 0. The slot
has no attention, and `TickAttention` (2013 0x745910, keep test 0xb48430 = death state 1/2/3 || clear timer || vtable +364
`HasAnyAttention_Derived`) removes it in the same tick it was created. Retail does the same, so retail also resets the player info
every tick. Retail's reset is `UObject::InitializeProperties(template)` (vtable slot 56, confirmed with `vt.py`), which leaks the
snapshot's own buffer instead of freeing a shared one.

Acceptance with the controller list on:

| check | result |
|---|---|
| the corruption named and fixed | double `appFree` of `UDisTweaks_PawnAttention::m_DistanceValueTuning.m_Points`, through the aliased `UDisAttentionInfo_Complex::m_PawnAttentionTweaks.m_DistanceValueTuning.m_Points` (ExitProperties in ResetFromTemplate). The violated retail rule: `TArray::operator=` (0x4448d0) in 0xb68e10 |
| 5/5 ride runs to world 200, 0 Criticals, list on, stim sent | **met**: IL2h-IL2l (and IL2a-e, IL2g: 11/11 clean) |
| one BlendNow 0.20 per NPC | IL2b: one per binding. Party NPCs and the player 1x each; `_16` / `_26` 2x and `_27` 3x (boat bind, then the dock / other matinees bind them again after Remove emptied the list: HB's first-controller rule) |
| soirée | IL2b: 12 behaviour changes `-> DisBehaviorSoiree`; Start for `_26`/`_27` at 56.71, Stop at 150.94, `_27` Start (dock, non-blocking) in the same frame |
| `Failed, with animsequence` | 0 in IL2a-e |
| ride SSIM >= .73 at the constant offset | **not met by median**: IL2a .723, IL2b .725, IL2c .716, IL2d .689, IL2e .727 (MAD 10.58-11.81, zoom 0.99). Offset `a = 13.569 - bind world`, the same rule as IK, with bind = the boat's `_27` BlendNow world. IC2a (list off) rescored the same way: **.731**. Per-10 s window deltas against IC2a are within ±0.02 for a/b/c/e, with no systematic sign. IL2d is shifted throughout (its bind is late, 57.24). So turning the list on does not degrade the ride: the 0.005-0.015 gap is run-to-run spread |
| gate d3d9 + inputtest + newgame (`build/agentIL/il_regression.log`) | **23 ok, 0 failed**: `unported_natives` 0, `probe_natives` 3, all `*_criticals` 0, `d3d9_frames` 1860, `inputtest_moved` 1043.9, `newgame_on_mission` 119, `newgame_pawns` 29. Only one reading of each load-sensitive metric (no retry triggered) |

Audio: all runs are on `d2db1e9` (before IE / IH2). The audio-thread timing was not measured with this fix.

## 2. What was changed

| where | what |
|---|---|
| `DishonoredGame/Src/disattentioninfo_complex.cpp` | SetupTickParamsFromTarget: copy `m_DistanceValueTuning` as retail does (ApplyOp, scalar, bias, exponent, bit, then `TArray<FDisVisCalcTuningParamsCurvePoint>::operator=` through the placeholder, the same cast `FixupDefaults_DisVisCalcTuningParams` uses) |
| `DishonoredGame/Src/dishonorednpcpawn.cpp` | MAT_BeginAIGroup: `MatineeControllerAdd` always; the stim always (`-icnostim` A/B); `GMalloc->Exec("ILGUARD ARM")` before the add (a no-op without `-ilguardheap`); banner |
| `DishonoredGame/Src/disaibrainprocessattention.cpp` | UpdateSoireeAttention: `-ilnoplayersoiree`, `-ilnonpcsoiree` A/B switches; `-ildiag` lines in GetAttentionInfo and RemoveAttentionInfoIdx (complex infos, first 30); NEW `DisILDiagEnabled` |
| `DishonoredGame/Inc/disattentioninfo.h` | declaration |
| `Launch/Inc/FMallocILGuard.h` (NEW), `Launch/Src/LaunchEngineLoop.cpp` | `-ilguardheap`: a GMalloc wrapper. Every block gets a 16-byte header and an 8-byte tail canary. Once armed, blocks up to 4 KB / 64 KB end at a no-access page and are decommitted on free (FIFO slot reuse). Freed header blocks hold a 0xDD pattern in a 48 MB quarantine that is checked every tick. A vectored handler logs the slot (alloc and free stack scans) on a fault. Diagnostic only, off by default |

No generated file was touched. The ResetFromTemplate deviation (ExitProperties + InitProperties; GI's documented reason) is kept:
with the snapshot owning its own buffer, freeing it there is correct, and retail's InitializeProperties simply leaks it.

## 3. Where the brief was wrong, with the measurement

1. **"Held at Busted" is not what kills it.** The player is never at Busted: level 0, value 0, floor 0 at every removal (IL2g
   `-ildiag`), because the complex info's minimum loop skips the soirée type (2013 0x74c6e0). The trigger is the player slot being
   **created and removed every tick**, with `ResetFromTemplate` each time. NPC co-actors (simple infos, no tweak snapshot) are
   harmless: IL1b clean, IL1c crashes.
2. **"A write past a fixed array / stale pointer / layout"**: none of those. It was a double free of a buffer one object owned and
   another object aliased: a value-semantics divergence (struct `operator=`) hidden by the generated `BYTE[12]` placeholder. The
   class layout matches (1644 B, `m_PawnAttentionTweaks` at 0x538 in both).
3. **"The target array of 24 slots from GI's package"**: there is no such array on this path. 0xb3f120 walks
   `m_MatineeControllersInfos` (pawn +3408, 12-byte entries) and each matinee's `GroupInst`, then calls GetAttentionInfo +
   SetMinAttentionLevel (vtable +348). Ours matches it line for line.
4. **"CRT debug heap via _CrtSetDbgFlag"**: RelWithDebInfo allocates through FMallocBinned, so the CRT heap never sees these
   blocks, and the engine's own FMallocDebug exists only under `_DEBUG`. A GMalloc wrapper was needed. Its first form (build 1)
   passed alignment 16 to FMallocBinned, which asserts `Alignment == DEFAULT_ALIGNMENT` ("Alignment currently unsupported in
   Windows", `MallocBinned.h:263`, checked in release too). IL1 therefore died 0.5 s into start-up with `-ilguardheap`, before the
   log opened. The attached debugger showed the recursive assert, and build 2 fixed the wrapper. So the guard never ran on the
   broken tree. The proof is the bisection (IL1b / IL1c), the per-tick reset (IL1c `-ildiag`), retail's `operator=` (0x4448d0 in
   0xb68e10), the CPF_NeedCtorLink flag, and 11/11 clean runs plus 0 guard reports after the fix.
5. **"The game thread only waits"** is right. **"0 particle lifecycle calls"** is right too, and is explained: the particle data
   was only the most likely victim of the doubly handed-out block.
6. **"Run to world 200"** with `cutscene_compare --quit-at 205`: the scripted `wait 205` is not world time. IC's and my first five
   runs quit at world 175-196, so five more runs (`--quit-at 250`) reached 211-243.
7. **Ride SSIM >= .73**: not reached by median (.716-.727, one outlier at .689) at IK's constant-offset rule. IC2a rescored under
   the same rule gives .731, not .738, and the per-window comparison shows run-to-run spread, not a regression from the list.

## 4. Open items

1. FMallocILGuard can stay as a diagnostic (`-ilguardheap`). Arming it needs `GMalloc->Exec("ILGUARD ARM")`, which only the
   matinee controller add sends.
2. FDisVisCalcTuningParams' `m_Points` stays a `BYTE[12]` placeholder in the generated header. Any other value copy of that struct
   would alias the same way. This tree has no other one (grep `m_DistanceValueTuning`); the generator should type the array.
