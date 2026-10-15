# Agent HC — position, rotation, movement (wave 25, PHASE22 package HC)

Worktree `build/agentHC_wt` (base f147830, fast-forwarded to main `2eb9b57` before build 2 and to `6ae222a` after the gate; no commits; the only overlap both times was the generated `dishonoredgameclasses.h`/`Sources.cmake`, regenerated). Build 2 and the gate are on `2eb9b57` + this package (HI's `6ae222a` is not in the binary). Build dir `build/agentHC`
(RelWithDebInfo through `build\agentHC_wt_build.cmd`). Staged `DishonoredGame_HC.exe`; copies `DishonoredGame_HC1.exe` (build 1)
and `DishonoredGame_HC2.exe` (build 2). Runs: `HC1.log` (build 1), `HC2.log` (build 2), `CC_HC2` (compare tool), `HCg_*` (gate).
Every run went through `game_input.py launch` with `-windowed` and ini isolation (`build/agentHC/hc_run.py`, HA's runner).
Measurements: `build/agentHC/hc_measure.py <log>`, `build/agentHC/walk.py <log>`.

## 0. The answer

1. **The two boat pawns ride the boat's sockets.** `FDisComponentRetargeting` is ported whole and `MAT_Attach` starts it:
   `gadiag retarget DishonoredNPCPawn_26 master SkeletalMeshActorMAT_2 socket Captain anchor anchor_jnt start`, `_27 ... socket Driver`
   at the matinee's start; both stop at the Attachment track's detach. Over the whole ride (HC2, 88 once-a-second samples each)
   the pawn's `anchor_jnt` lies on the socket within **0.07 uu / 0.11°** (`_26`), **0.04 uu / 0.11°** (`_27`) (`anchor err` = the
   anchor bone's world transform rebuilt from the updated actor + mesh translation against the socket's, position and quaternion angle).
2. **The tiller guard stands on the deck at the tiller, facing forward; the captain leans on the hull on the left, and faces away
   with his hands behind his back in the second half** — captures `hc2_2` (world 90) vs `retail_40`, `hc2_6` (world 149) vs
   `retail_104` (§3). HC2's compare run against HD2: ride SSIM **0.592 → 0.690**, MAD **21.6 → 12.0** (part of the MAD drop is
   HE/HF's HUD fix merged on main since HD2: `vials` 125/126 → 0/126), zoom 0.99 (HD merged).
3. **The component manager exists and ticks** where retail ticks it (`UWorld::TickPreAW`/`TickAW`/`TickPostAW`): `gadiag cpntmgr 26
   preasync` (the locomotion components, step 5) plus the two retargeting components during the ride.
4. **Matinee root motion moves the pawn.** WorkerB key 0 (`rootMotion 1`): `DishonoredNPCPawn_20` goes from `-1230/33499` to
   `-1203/33462` = **45.8 uu** of the anim's 47.5 (HC1/HA: 0). Cause was not the Edge locomotion joint but `UAnimNodeSequence::TickAnim`
   (§4.3).
5. **Step 5 regresses Worker B's walk speed** (865 uu at 123 u/s on HC1 → the same 865 uu in 11.6 s, ~95 u/s between samples,
   instantaneous 0..122 on HC2). Kept in the tree and the gate exe (retail's tick source, and the two builds were spent);
   `build/agentHC/revert_step5_loco.py` undoes exactly that part (§5). Coordinator's call.
6. Rotation intents are ported, `physicsRotation` no longer turns NPCs to their controller; FaceTo (step 6) is not ported —
   follow-up (§6).

## 1. What was ported (2012 / 2013, every address resolved by hand in `build/agentHC/idb/*_agentHC.i64`)

| function | 2012 | 2013 (VA) | notes |
|---|---|---|---|
| `FArkComponentManager` ctor / dtor / `Tick` | 0x57afa0 (202) / 0x579cf0 (137) / 0x57b070 (144) | 0x93a5e0 / 0x9391b0 / 0x93a6b0 (117, same switch) | 180 B, six policies; ctor/dtor inline (empty TArrays) |
| `GetPolicy<PostAsyncWork>` | 0x765610 (4) | 0xb279c0 | `lea eax,[ecx+84]`; the PreAsyncWork one is the ICF-folded `mov eax,ecx` IDA calls `FStaticLightingVertexMapping::GetVertexMapping` (2013 0xc11820) |
| policy `Register` / `Unregister` / `DoneTicking` / `Insert` / `Tick` | 0x578ba0 / 0x578bc0 / 0x578c50 / 0x577fc0 / 0x579d80 | 0x937f70 / 0x937f90 / – / – / – | Insert puts the new component AT the index of the last not-greater type (asm checked, retail's quirk kept) |
| `FArkComponentTimeSlicedTickPolicy::Tick` / `Unregister` | 0x578120 / 0x578100 | | 0.9 ms budget, round over frames |
| `UWorld::Init` / `FinishDestroy` / tick sites | 0x3bbd00 / 0x3b0730 / TickPreAW 0x26ef30, TickAW 0x26f0a0, TickPostAW 0x26f150 | TickPreAW 0x6552c0, UWorld::Tick 0x65e0d0 | stage 5 then 0 after the pre-async actors |
| `FDisComponentRetargeting` ctor, `Stopping`, `PreAsyncWorkTick`, `OnStopRetargeting`, `SetMasterActor`, `StartRetargeting`, `StopRetargeting` | 0x8af8b0, 0x8d2330, 0x8d23c0, 0x8d10f0, 0x8d1100, 0x8d2400 (569), 0x8d0eb0 (563) | 0xc66c90, 0xc88370, 0xc88400, 0xc87680, 0xc87690, 0xc88440 (578), 0xc87430 (589) | 2013 bodies: Start also zeroes the mesh offset's pitch, Stop skips a terminated actor |
| `FDisComponentRetargeting::UpdateRetargeting` | 0x8b37e0 (4,308) | 0xc6ace0 (4,824, unnamed) | the slave mesh `SetTransformedToWorld` is vtable +516/+520 |
| `FArkComponentMeshOffset::ForcePitch` (name ours) | — | 0x8f1c10 via 0xb498c0 | 2013 only |
| `ADishonoredPawn::PreBeginPlay_NativeComponents` | 0x793f50 (149) | | retargeting added; `FDisComponentObservable` unported (named) |
| `ADishonoredNPCPawn::MAT_Attach` / `MAT_Detach` / `MAT_SetCollision` / `MAT_SetMeshTranslationMode` / `MAT_DisableMeshTranslation` | 0x7ab5e0 / 0x7ab690 / 0x7ab7a0 / 0x7ab6e0 / 0x7ab720 | 0xb4b080 | |
| `physicsRotation`, `Tick_Rotation`, `Init_Rotation`, `CancelAllNPCRotations`, `SetTargetRotation`, `SetNPCRotation`, `SetNPCRotationFocus`, `LockNPCRotation`, `UnlockNPCRotation`, `OnUnlockNPCRotation` | 0x7c95f0, 0x7d4d40, 0x7c9570, 0x7c95a0, 0x7cbee0, 0x7ce4a0, 0x7cbf90, 0x7cc020, 0x7ce540, 0x7cc0f0 | 0xb6e920 (Cancel), 0xb714c0, 0xb74960, 0xb71570, 0xb71600, 0xb74a00 (Unlock) | Tick_Rotation called from `Tick` (0x7c80e3) AND `Tick_Body` (0x7be1f2) as retail; FaceTo calls named |
| `FArkComponentLocomotion::Starting` / `Stopping` registration, `ADishonoredNPCPawn::physWalking` | 0x58b750 / 0x58b870 / 0x7ad240 (107) | | MovePawn's callers: physWalking and `UInterpGroupInst::UpdateGroupInst` 0x25b4e0 only |
| `UAnimNodeSequence::TickAnim` bMatineePlaying gate | 0x1aa480 (562) | | §4.3 |

## 2. Diagnostics (`-gadiag`)

- `gadiag retarget <pawn> master <actor> socket <s> anchor <b> (bone m/s) rot closer time volumes start` / `... stop at x/y/z yaw phys base`.
- once a second per carried pawn: `at`, `in socket` (actor origin in the socket frame), `anchor err <uu> <deg>`, `socket at`, `moved`, `meshT`, `phys`, `base`.
- `gadiag cpntmgr <n> preasync <n> justbefore <n> during <n> postasync <n> timesliced` once, when the PreAsyncWork list first fills.
- `gadiag rot <pawn> yaw <cur>/<intent> prio <p> rate active reached` once a second for an NPC with a non-zero current priority.

## 3. Acceptance, as measured

| check | before (HC1 = build 1 / HA / head) | HC2 (build 2) |
|---|---|---|
| `_27` on `Driver`, `_26` on `Captain`, every sample | HC1: carried from the ride's start (in-socket origin 97/0/0 at rest; anchor metric added in build 2); before HC: unbased in MAT_Attach, floor-swept onto the housing (GA) | 88 samples each, anchor err max 0.04 uu / 0.11° (`_27`), 0.07 uu / 0.11° (`_26`) |
| `_27` Z on the deck | before: on the engine housing (user video) | actor origin 97 uu above the Driver socket on the socket's axis at rest; end Z 2414.7 vs socket 2316.0; `hc2_2`, `hc2_6`: guard on deck at the tiller |
| captures world 60..149 | user video: guard on housing, captain mid-deck, facing camera | `hc2_1/2`: captain at the left hull leaning, guard at tiller (retail_40); `hc2_4/6`: captain facing away, hands behind back (retail_80, retail_104) |
| HH compare (`build/cmp/HC2`, baseline HD2) | SSIM 0.592, MAD 21.56 | SSIM 0.690, MAD 12.00, zoom 0.99 |
| WorkerB key 0 moves | 0 uu (HA1/HA2/HC1) | 45.8 uu (`-1230/33499` → `-1203/33462`); stack `rm 1` edge at key 0; `rmd` is 0 on the once-a-second samples (key lasts 0.6 s) |
| Worker B 850-uu walk at ~123 u/s | HC1: 10845/22461 → 11633/22104 (865 uu) in 7.1 s, 123 u/s | 865 uu to the same mark in 11.6 s, ~95 u/s; **regression, §5** |
| player ride unchanged | — | HD2 vs HC2 ours frames: grey MAD 2.1 at world 52/55; at 150–170 the view is the same framing (`scratchpad hd2_hc2_158.png`: same hall, same camera) and the difference is the two NPCs on the deck and HD2's vials (MAD ~10) |
| GK's captain detach | GK1: teleported near the dock (10092/23057/2319) | stop at **10120.7/23022.6/2496.8** yaw 64828, PHYS_Walking, base the boat (after retail's 100-uu drop); Captain socket 10042.7/23044.5/2337.2 (same as GK's measurement) |
| Critical | 0 | 0 (HC1, HC2) |
| gate | | §7 |

The zoom term is 0.99 (HD merged), not 1.76.

## 4. Where the plan was wrong, with the measurement

1. **"physWalking → PreAsyncWorkTick" is not retail's within-frame order.** `MovePawn` (2012 0x584ca0) has exactly two callers:
   `ADishonoredNPCPawn::physWalking` (0x7ad286) and `UInterpGroupInst::UpdateGroupInst` (0x25b75d). Retail's physWalking is
   `MovePawn` alone; the dynamics run from the manager AFTER the actors (TickPreAW), i.e. one frame ahead of the move.
2. **Retargeting registers in the PreAsyncWork policy**, not the JustBeforeProceduralAnim one, although its second base is
   `IArkComponentPreAsyncWorkJustBeforeProceduralAnim` (2013 StartRetargeting calls the folded `mov eax,ecx` = policy @0).
3. **The root motion was not in an unread joint.** Every NPC skeleton in `L_Tower_Script` (36 meshes, `build/agentHC/edge_probe.py`)
   has `locomotionJointIndex 0` = `root0_jnt`, which Plan B already differentiates; `Empty_Distraction_Scavenge02_In` stores the
   loop delta `t 0/0/47.477` at `offsetLocomotionDelta` 164. The zero was `UAnimNodeSequence::TickAnim` writing
   `PreviousTime = CurrentTime` on a node matinee had just positioned; retail (0x1aa480) skips that while `bMatineePlaying`.
   The full `FLocomotionState` path of `AnimLeafCallback` (re-evaluation at PreviousTime via a 0x10 leaf, loop count, the six option
   bits from `BuildEdgeAnimTreeLeaf` 0x1a46b0) is NOT ported; Plan B gives the same delta for a forward, non-wrapping key.
4. **The `+94 X` key offset of `Guard Captain A` is not applied by retail**: `MAT_Attach` 0x7ab5e0 passes only `AttachInfos.BoneName`.
   Measured: both pawns sit 97/0/0 in their socket's frame at rest.
5. **StartRetargeting locks then immediately cancels the rotation intents** (`LockNPCRotation(ReTargeting)` then
   `CancelAllNPCRotations`, both builds) — no intent stays active on the carried pawns: 0 `gadiag rot` lines on HC2.
6. **"within 1° yaw" cannot be read off the socket's yaw**: the boat sockets are rolled frames (the anchor sits on the socket's X
   axis), `FMatrix::Rotator` yaw jumps ±180°. Measured the full quaternion angle instead.
7. **The actor is not always on the socket — the mesh is.** With `m_bKeepActorCloser` the actor `Move`s and is blocked by the boat
   while it accelerates; the shortfall becomes the mesh translation (up to 48 uu on `_27`, 84 on `_26`), so the body stays on the
   socket (anchor err ≤ 0.07) while the cylinder lags. During the captain's lean the anchor bone itself is animated (in-socket
   159/15/80 for minutes) — that is the animation placing him at the hull, not an error.

## 5. Step 5 and the walker

Step 5 = the locomotion component in the manager's PreAsyncWork policy (`Starting`/`Stopping`), `physWalking` = `MovePawn` only.
On HC2 Worker B still reaches his mark (11634/22104, same as HC1) but slower: loconode `curmove 123` while `realscaled` reads
10–122 and the instantaneous velocity is 0 on some samples; HC2 also had 1.6-s hitches at log 132.4 and 154.6 inside the walk.
Not diagnosed within the two builds. `python build/agentHC/revert_step5_loco.py` (run in `build/agentHC`) reverts only the
locomotion half (keeps the TickAnim gate); one rebuild.

## 6. Open items

1. `FArkComponentFaceTo` + `UInterpTrackInstFaceTo::UpdateTrackInst` (step 6, not started): named follow-up; until then NPC intents
   (and FaceTo tracks) turn nobody, and `physicsRotation` no longer turns NPCs either (retail's).
2. Worker B's slower walk under step 5 (§5).
3. The full `FLocomotionState` locomotion half of `AnimLeafCallback` (loop wrap, re-evaluation at PreviousTime).
4. `FDisComponentObservable` (the other component `PreBeginPlay_NativeComponents` adds).
5. The mesh offset and look-at components still tick from their seams (`USkeletalMeshComponent::Tick`), not from the manager.
6. The NPC rotation callers (`UStateNPCMaster*`, `UDisBehaviorAmbush`, the teleport spell) are unported.

## 7. Gate

`run_regression.py --build-dir build/agentHC --no-build --exe-name DishonoredGame_HC.exe --log-prefix HCg_` on build 2: **32 ok, 2 failed, 0 skipped**, 0 Critical in every stage (nullrhi, d3d9, inputtest, newgame, options); the two failures are `coresmoke_passed` and `layout_types`, whose `CoreSmoke.exe` / `LayoutProbe.exe` this package's DishonoredGame builds do not produce (HA's §6 same; the coordinator's clean-checkout gate builds them). `inputtest_moved 1043.4`, `newgame_pawns 29`, `unported_natives 0`, `options_as2_errors 39`. Log `build/agentHC/hc_regression.log`. Builds: 0 errors in build 2; build 1's first
invocation stopped on one compile error (`TArray::Max`), fixed and resumed (`build1b.log`); 0 C4263/C4264 in all logs.
Generator into `build/agentHC/regen`: 79 of 79 files byte-identical (CRLF ignored). `bitfield_audit.py`: mismatches 0.
Generator re-run after the second merge (`regen2`): 79/79 identical; audit 0. Sync `build/agentHC_sync.py`: base 6ae222a, 24 files, 0 flagged, 0 missing, worktree diff = the list.

## Functions changed in files HB also edits

`dishonorednpcpawn.cpp`: `MAT_SetCollision`, `MAT_SetMeshTranslationMode`, `MAT_DisableMeshTranslation`, `MAT_Attach`, `MAT_Detach`,
`Tick` (one `Tick_Rotation` call + its banner line), one `#include`. Not touched: `MAT_BeginAIGroup`, `MAT_FinishAIGroup`,
`PostInitAnimTree`, `UnInterpolation.cpp`.
