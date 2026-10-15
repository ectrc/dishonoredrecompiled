# Agent HB — matinee pose blend in/out + the LookAt track (wave 25, PHASE22 package HB)

Worktree `build/agentHB_wt` (base `f147830`, then main applied by patch: `2eb9b57`, `835dc2b`, `9da5dbc`, `d9be103`), build dir
`build/agentHB` (RelWithDebInfo), staged `DishonoredGame_HB.exe`. **Five builds, not two** (§5). Final exe = build 5
(tree = main `835dc2b` + HB; HJ `9da5dbc` and HL `d9be103` were applied to the worktree afterwards and touch no file of this package).

## 0. The answer

1. **Blend in and out work.** Every bound NPC gets `BlendNow 0.20` at bind and its child-1 weight goes 1 → 0 in 0.2 s;
   every AI group's end runs `MAT_BlendOut` (`GetBlendOutTime` 0.20) and blends back over 0.2 s (HB3.log, CC_HB2.log):

   | event | world | lines |
   |---|---|---|
   | boat bind: captain `_26`, Elite `_27`, `_16` | 56.31 (HB3) / 57.59 (CC_HB2) | `BlendNow 0.20 captured 118 bones w 0.00/1.00` |
   | blend done | 56.53 (HB3: 0.22 s, one held frame) | `done ... w 1.00/0.00` |
   | ride end, three groups | 150.82 (HB3) / 151.50 (CC_HB2) | `MAT_BlendOut group 'Guard Captain A' / 'EliteGuard on Boat' / 'GuardBigBoard C' blendOut 0.20` |
   | done | 151.13 / 151.73 | |
   | player (FZ's blender, same class) | 150.82 | `DishonoredPlayerPawn BlendNow 0.20 captured 79 bones` → done 151.13 |

   Also the party matinees' 10 NPCs at world 51.25 and `_3/_4` at 117.0 — one `BlendNow` per binding.
2. **The LookAt track runs.** `UInterpTrackLookAt::StaticUpdateTracks` drives FT's component from
   `UInterpGroupAI::UpdateGroup`: captain keys 0–7 (targets Babord, the player (type 0), EliteGuard, Tribord, Workers2;
   `head_end_jnt`), Elite keys 0–13 (Guard Captain A, Tribord, Babord, BoatFront, Workers2, the player) crossed in order
   (HB3.log). The captain's head cursor follows the request within a second wherever the head grid allows: request yaw /
   cursor yaw 2.8/2.5 (w 67.1), −53.0/−47.7 (70.2), −35.5/−31.9 (85.4), 14.9/13.5 (102.1); requests beyond the grid
   clamp at ±65° (head) / ±125° (torso key 5) — e.g. request −122.9, cursor −65.0 (135.6), because the captain faces the
   wrong way (HC's rotation; §4).
3. **The plan's NPC blend cannot happen with retail's code alone in this tree**: the NPC pose blender is armed only by
   `ADishonoredNPCPawn::OnLODChanged` (2012 `0x7bfce0`: `SetEnable( LOD flags bit 8 )`) from the unported LOD manager;
   cooked `m_bEnabled` is 0 on all 29 NPC PoseBlenders (HB1.log), so every `BlendNow` was refused. Stand-in: `Spawned`
   arms it (`DISHONORED(bringup)`).
4. **`MatineeControllerAdd` crashes the game without the soirée stim** (measured, §3): it is ported but gated off
   (`-hbcontrollers`).
5. HH (`cutscene_compare.py run --tag HB2 --baseline build/cmp/HA2/scores.csv`, ride world 57.7–151.5): MAD 28.89 →
   12.26, SSIM 0.523 → 0.687, **zoom 1.76 → 0.99 — HD has merged, the zoom term is no longer 1.76** (most of this delta is
   HD/HC/HI). HB's own share, same exe with HB's paths switched off (`HB2base`: `-hbbasepose -hbnolooktrack
   -hbnolockreq`, over 58.3–122.1 where both runs have frames): MAD 14.91 → 12.40, SSIM 0.681 → 0.693.

## 1. What was ported

| function | retail (2012 / 2013) | where |
|---|---|---|
| `UArkAnimNodeBlendPose::InitAnim` | `0x1c3040` (242 B, folded with BlendList's) / `0x91b0a0` (252, slot 80) | `arkanimnodeblendpose_dis.cpp` |
| `::TickAnim` | `0x550080` / `0x90f990` (729 both): disable timer, full-weight-on-relevance, weights to targets, one frame held by `m_bMustBlendNextFrame` | same |
| `::SetActiveChild` | `0x550360` / `0x90fc70` (1001 both): BlendList's, `GIsGame` (not `!GIsEditor`), clamped index | same |
| `::ForceDisable` | `0x54a290` | same |
| `::GetBoneAtoms` | `BuildEdgeAnimTree` `0x55dd40` (770) / `0x91d150` (784, slot 91) on the CPU | same |
| `::BlendNow`, `::SetEnable` | FZ's, diag added | same |
| `UInterpTrackLookAt::StaticUpdateTracks` | `0x547010` (3756) / `0x9082c0` (3386; only caller `UInterpGroupInstAI::UpdateGroupInst` `0x6305c0`) | `DishonoredGame/Src/interptracklookat_dis.cpp` (NEW) via `GDisLookAtTrackUpdateHook` |
| `UInterpTrackLookAt::GetLookAtInfos` | `0x542c10` / `0x904390` (637 both) | same |
| `UInterpTrackLookAt::GetGameLookatPriority` | `0x53bdb0` / `0x8fcc60` | `Engine/Src/interptracklookat.cpp` |
| `UInterpTrackInstLookAt::InitTrackInst / TermTrackInst` | `0x53bc90` / `0x53bcb0` | same (+ `GDisLookAtTrackTermHook`) |
| `ADishonoredNPCPawn::MAT_BlendOut` | `0x7ad5e0` (194) / `0xb4ec80` (209: 2013's else → `m_iPreventRBVelFromAnimFrameCount = 2`) | `dishonorednpcpawn.cpp`; `APawn::MAT_BlendOut` base (`0xc8750`) added |
| `MatineeControllerAdd / Remove` | `0x7b19a0` / `0xb53500`; `0x7baea0` / `0xb5f760` | same (Add gated, §3) |
| `MAT_BeginAIGroup` | `0x7c0570` / `0xb65080`: FSM exits, `UpdateNodeSlot`, `BlendNow(0.2, TRUE)` if no controller, controller add, hit-react, jiggle bones (2013 bit `0x20` → `Mesh->m_bDontUpdateKinematic`), locomotion, `StartLookForward(group, name, prio, 0.5, -1)` on `LockRequests` | same |
| `MAT_FinishAIGroup` | `0x7c0830` / `0xb65350`: `StopAllLookAtFromAsker(group)`, `MAT_BlendOut`, …, controller remove, jiggle restore, `StopLookAt(m_iMatineeLookAtRequestID)` | same; stale banner rewritten |
| `ADishonoredNPCPawn::Spawned` tail | `0x7b0a60`: `m_pBlendPoseNode = FindAnimNodeFast('PoseBlender')`, `m_fLastTeleportTime` | `dishonorednpcpawn_locomotion.cpp` |

`BuildEdgeAnimTree` as read from the Edge command processor (`_edgeAnimProcessCommandList` 2012 `0x6238c0`,
`_edgeAnimRecursiveProcessBlendTree` `0x6228a0`): one branch, left = child 0's tree, right = a leaf, op 0, alpha = child 1's
Weight. Leaf flags 2 (alpha ≥ 0.0001 and a frozen pose) = command 5 copies `m_BlendPose`; flags 4 = the skeleton's base
pose; both define every joint but the locomotion joint, so root motion stays the live tree's. Branch flags 4 (node
enabled): blend only if alpha > 0, then **copy the branch output into userVal = `m_PreviousPose`** — the node captures
what it outputs every evaluation; `m_bDoBlend` (raised by `BlendNow`) freezes that capture into `m_BlendPose` on the next
evaluation. CPU form stores `FBoneAtom` per bone index; the frozen pose is copied out per bone (the BYTE array is 8-byte
aligned, `FBoneAtom` is a 16-byte SSE type).

## 2. Diagnostics (`-fkdiag`)

`fkdiag blendpose <pawn> <node> init: ... m_bEnabled cooked <b>`, `... Spawned: tree .. PoseBlender .. enabled`,
`fkdiag blendpose <pawn> BlendNow|BlendNow refused|blending|done <t> captured <n> bones w <c0>/<c1> enabled blend world`,
`... MAT_BlendOut group '<g>' blendOut <t> node physics handled`, `fkdiag lookattrack <group> key <k> -> <target> (type infl
bone) w prio req pawn pos world` (on key change), `fkdiag lookattrack <group> <pawn> request yaw pitch dist head torso speed |
cursor <yaw>/<pitch> active prio requests` (1/s), `fkdiag lookattrack <group> LockRequests forward prio req`. A/B
switches (bringup): `-hbnocapture`, `-hbbasepose`, `-hbnolooktrack`, `-hbnolockreq`, `-hbcontrollers`.

## 3. Where the plan was wrong, with the measurement

| the plan said | measured |
|---|---|
| "writer of `m_pBlendPoseNode`: native xref vs script `FindAnimNode`" | native: `ADishonoredNPCPawn::Spawned` 2012 `0x7b0bf2` (`mov [esi+0C90h]` — 2012 offset 0xC90, 2013 3456); no other writer in either binary |
| blend 0.2 s at bind | refused on every NPC (HB1.log: 13 `BlendNow refused ... enabled 0`): only `OnLODChanged` (LOD manager) arms the node; the 2013 vtable call (`+480`) has no other caller. Bringup: `Spawned` arms it |
| `MatineeControllerAdd` then `BlendNow` | the list feeds `UDisAIBrainProcessAttention::UpdateSoireeAttention`, which holds every other pawn of the matinee — the player — at Busted; retail's brain is kept quiet by the `FAIStimStruct_Soiree` start stim (unported). With the list filled, every run died 1.5–6 s after the boat bind (render thread in `FParticleSystemSceneProxy::Create/ReleaseRenderThreadResources`, game thread stalled): HB1, HB1b, HB1c, HB1d, HB2a, and with `-hbnocapture` / `-hbnolooktrack`. Gated off: HB3 runs the whole ride; `-hbcontrollers` crashes again (HB3ctl, 2.6 s after the soirée). Consequence: `BlendNow` fires on every binding (the "first controller" test is always true) |
| `PostInitAnimTree` add/start `FDisComponentAnimPlayer` | not overridden: the component is unported (ctor 242 B, `Starting` 506 B registers on the manager and caches 7 slots/filters nobody here reads); a Super-only override would be dead code — named gap |
| `UInterpTrackLookAt::StaticUpdateTracks` called from `UInterpGroupAI::UpdateGroup` | retail's caller is `UInterpGroupInstAI::UpdateGroupInst` (`0x25c092` / `0x630682`), signature `(pos, GrInstAI, bJump, bPreview)`; same spot in this tree's flow (after the collector reset, before the passes) |
| "captain 27+31 keys, Elite 62+63" | the captain's LookAt track crosses keys 0–7, the Elite's 0–13 (key indices, HB3.log); the numbers in the plan match nothing I can find |
| captain/Elite bind ≈ world 55.7 | 56.31 (HB3), 57.51 (HB1), 57.59 (CC_HB2): varies with load time; the bind falls in the black before the fade-in, so a pop there is invisible by construction — the end is the visible test |
| "CRLF sources" | the tree is LF (`.gitattributes eol=lf`); edits kept LF |
| `GetGameLookatPriority` result | the three priorities are all 0 at runtime (`prio 0` on every request) — `UDishonoredEngine::Init`'s `SetLookatPriority(1, 5, 19)` is a bringup comment (`dishonoredengine.cpp:143`), not called |
| two builds | five: §5 |

Captures (HB3, `build/agentHB/shots/`, `hb_shots.py`): at the end (world 150.39 → 151.13, 0.15–0.27 s apart) consecutive
centre-crop grey differences 1.61, 1.60, 1.57 through the blend (the captain's arm leaves the hands-behind-back pose over
150.82–151.13, `hb3_end.png`), 7.9–10.9 after it as the dock matinee turns him. Player ride A/B (HH frames, HB2 vs HB2base,
world 60–120): zoom 1.00 every sample, MAD 0.5–7.2 (NPC heads and poses differ; framing does not).

## 4. Open items

1. `FDisComponentLODManager` (`StartLOD`/`StopAllLODByAsker`, `OnLODChanged`): would replace the `Spawned` stand-in.
2. The soirée stim (`FAIStimStruct_Soiree`, `UDishonoredAIBrain::HandleAIStim_Internal`) — then turn
   `MatineeControllerAdd` on (drop `-hbcontrollers`) and the blend reverts to "first controller only".
3. `UDishonoredEngine::Init` → `UInterpTrackLookAt::SetLookatPriority(1, 5, 19)` and `UInterpTrackFaceTo::SetFaceToPriority(15)`
   — all matinee look-at requests run at priority 0 today.
4. Captain head yaw saturates at the grid limit when the target is behind him: his facing (HC).
5. 2013 `MAT_BeginAIGroup` fake death (`sub_B5A0B0`, `m_iNumFakeDeathRequestsFromSoiree`) and the `+1288` bit `0x2000`.

## 5. Builds and gate

1. HB1 (items 1–4 at once): 0 errors, 0 C4263/C4264; measured the refusal and the crash.
2. HB2 (+ main `2eb9b57`, LOD stand-in, IsA guard, A/B switches): crash persisted, bisected to neither capture nor track.
3. HB3 (controllers gated): full ride, all measurements above.
4. HB4: built on a tree where the `835dc2b` patch had **not** applied (my `--3way` apply failed silently) — no new information.
5. HB5: main `835dc2b` + HB; HH `HB2`/`HB2base`; gate below. 0 errors / C4263 / C4264 in every build log.

Generator `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentHB/regen`: 79 of
79 files byte-identical (CRLF ignored). `bitfield_audit.py`: 0 mismatches. `Failed, with animsequence`: 0 (HB3.log,
CC_HB2.log). Gate (HB5, `build/agentHB/hb_regression.log`): **30 ok, 4 failed, 0 Critical** — `coresmoke`/`layout` need CoreSmoke.exe/LayoutProbe.exe (not built by this target, as HA), `unported_natives` 4 = the player-pawn toggle natives HI's PlayerAction reaches (HL, `d9be103`), `inputtest_moved` 702.8 at d3d9 startup 19.4 s (machine loaded); re-run alone when quieter: 1053.1 with HB, 1058.4 with HB's paths off (`-hbbasepose -hbnolooktrack -hbnolockreq -hbnocapture`).
