# Agent FJ (PHASE15 FJ) — the Options crash: a dangling target in the action queue, and four premises of the brief that were wrong

Branched from `17a7c32`, **fast-forwarded onto `98e22d4`** mid-package when the coordinator landed
the firewall fix. Own worktree `build/agentFJ_wt`, own build dirs
`build/agentFJ_wt/build/agentFJ_rel` (this package) and `build/agentFJ_wt/build/agentFJ_clean` (the
gate build). Own IDA copies `resources/docs/idb/shipping2012_agentFJ.i64` and
`retail2013_agentFJ.i64`, headless only. **No FModel tool was used.** No commits, nothing staged,
nothing written into the main checkout outside `build/`. Every screenshot in this package was taken
inside the game process with `-apshottime` and `-apshotname=fj`; no desktop or window capture was
used anywhere.

**Three files, all in `External/GFx3`**: `GFxPlayer.h`, `GFxPlayerRoot.cpp`, `GFxPlayerSprite.cpp`.
They are named function by function in §8 and in `build/agentFJ_sync.py`.

## The answer in ten lines

* **`0x1cc148a` is `GASActionBuffer::Execute(GASEnvironment*, int, int, GASValue*, ExecuteType)`
  + 0x2DA**, and the faulting instruction is `call dword ptr [eax+0x40]` where
  `eax = *(target + 0x80)` — that is **`env->GetTarget()->ToSprite()`**, index 16 of the
  `GASObjectInterface` sub-object's vtable, on a target whose vtable word is garbage. §1.
* **There is a minidump of the user's crash in the same Logs directory and nobody had opened it**:
  `unreal-v9411-2026.10.11-14.36.05.dmp`. It gives the module base (`0xdb0000`, confirming §1.1),
  the exception (`0xc0000005`, a **read of `0x40`**, `eax = 0`), and the dangling target's own
  memory: **it is on the allocator's free list, chained to two more freed character blocks, and its
  `Depth` reads `0x4000` — `GFxASDepthBias`, the bias on a clip `attachMovie` created.** §1.3.
* **The brief's first instruction — "subtract the image base" — cannot work.** The shipped executable
  is `/DYNAMICBASE`, so `0x1cc148a` is a **runtime** address and `0x1cc148a - 0x400000` lands 644 KB
  past the end of the image. The module's real load address that run was **`0xdb0000`**. §1.1.
* **The cause is a raw pointer in the action queue.** `GFxMovieRoot::ActionEntry::pTarget` was a
  plain `GFxSprite*` with no reference taken, while retail's entry holds the character in a `GPtr`
  at `entry+8` — 2012 `GFxMovieRoot::ActionQueueType::AddToFreeList` (`0xa0cb40`) releases it — and
  **refuses outright to run an entry whose character has been unloaded**: 2012
  `GFxMovieRoot::ActionEntry::Execute` (`0xa024d0`) is
  `if (pCharacter && !(pCharacter->Flags76 & 0x10)) switch (Type)`. §2.
* **Measured, not argued: three action buffers per CONTROLS tab selection are drained through a
  character the queue alone was keeping alive** — that is, three per tab switch that on HEAD had
  already been freed. 5 selections, 15 entries, **0 on the other three tabs**, in one scripted run.
  The user crashed on `OnTabSelected (1, r)`. That is the same tab. §4.
* **The brief's log shape is not a crash signature.** Two `InitScrollBar` in one frame, the
  `removeMovieClip … on undefined`, and the `setMask` line appear on *every* CONTROLS selection.
  HEAD ran that exact sequence **28 times across three scripted runs without dying**. The crash is a
  use-after-free whose fatality depends on whether the freed block is reused before the drain, and
  it did not reproduce on demand. §3, §4.1 — **and this is the one acceptance item I did not meet.**
* The fix is retail's, in three parts: the entry takes a reference
  (`GFxMovieRoot::PushActionBuffer`), the drain refuses an unloaded target and releases on every
  path (`GFxMovieRoot::DoActions`), and the unload actually reaches the subtree —
  `GFxDisplayList::Clear` (2012 `0x9d5e30`) calls each entry's `OnEventUnload` and zeroes its parent
  pointer before the release sweep, and `GFxSprite::OnEventUnload` (2012 `0x9fdc80`) calls `Clear`.
  §2, §5.
* **The unload was reaching exactly one clip before this.** `GFxCharacter::OnEventUnload` was an
  empty inline body and `GFxDisplayList::Clear` was a release loop, so a clip removed by script was
  the only thing ever marked; its children were freed silently. That is why the first version of the
  instrument reported **zero** and why the defect had never been visible. §4.
* **`GFxDisplayPushScriptedMask` and `GFxCharacter::SetMask` are both innocent, and so is every
  other candidate in the brief.** The decoded address is in the AS2 interpreter, not in the mask or
  display-list render path. `MaskLevel`, `Mask_Increment`, the `>= GFxASDepthBias` test and
  `CloneDisplayObject` are all unrelated to this fault. §3.
* **One defect found and left, named in §7**: `GASFunctionObject::pDeclTarget` is the other raw
  character pointer that reaches the same faulting instruction, and retail guards it with the same
  unloaded bit (2012 `GASAsFunctionObject::Invoke` `0x9f3540`, 2013 `0x9e9d10`:
  `if (env->Target->Flags76 & 0x10) env = call.pEnv`). Writing that guard here would dereference the
  pointer it is meant to protect against, because **retail's real safety net — the movie root's
  unload list at `movieroot+0x27fc`, which takes a reference on every unloaded sprite — is not
  ported.** That list, not the guard, is the thing to port next.
* **Gates**: clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`:
  see §6. `rva_sweep.py` over `External/GFx3`: **698 citations, 0 MISLABELLED-2012,
  0 UNKNOWN-CLAIMED-2013**. `run_regression.py` from the worktree's own copy with an absolute build
  dir, built inside the harness: see §6.

## 1. Getting a real stack

### 1.1 The address is not where the brief says it is

The brief says `0x1cc148a` is a VA in our executable and to subtract the image base. Both halves
fail:

* The staged `DishonoredGame-Win64-Shipping.exe` (24,008,192 bytes, PE timestamp `6acb90bd`) has
  `DllCharacteristics 0x8140` — **`DYNAMICBASE` is set**. The preferred base `0x400000` is not where
  it loads.
* `SizeOfImage` is `0x181d000`, so at the preferred base the image ends at `0x1c1d000`.
  `0x1cc148a` is **0xA448A past the end of it**. Subtracting `0x400000` gives an RVA of `0x18c148a`,
  which is mapped to nothing.

What was actually needed was the module's **runtime** base. It is recoverable without a debugger and
without running the game: Windows caches an image's randomised base per boot, so launching the same
unmodified file again in the same boot session gives the same base.
`CreateProcessW(..., CREATE_SUSPENDED)` on it, `NtQueryInformationProcess(ProcessWow64Information)`
for the 32-bit PEB, then `PEB->ImageBaseAddress` at `+8`:

```
NtQuery status 0x0 peb32=0x4a5000
ReadProcessMemory ok=1 ImageBase=0xdb0000       (3 of 3 launches identical)
```

The process is killed before the loader runs; nothing of the game executes. The boot was
10 Oct 18:00, the user's crash 11 Oct 14:36, this read 11 Oct 14:55, and the exe file has not been
written since 14:35 — one boot session, one cached base.

So **RVA `0x1cc148a - 0xdb0000` = `0xf1148a`**, and against
`DishonoredGame-Win64-Shipping.map` (`Preferred load address 00400000`, timestamp `6acb90bd`, the
same timestamp as the exe) the address is `0x131148a`:

```
013110d0  ??1GASActionBuffer@@QAE@XZ
01311190  ?Execute@GASActionBuffer@@QAEXPAVGASEnvironment@@@Z
013111b0  ?Execute@GASActionBuffer@@QAEXPAVGASEnvironment@@HHPAVGASValue@@W4ExecuteType@1@@Z  <<< +0x2da
01315258  $LN2597                                     [gfx3:GFxAS2Interp.cpp.obj]
```

`GASActionBuffer::Execute(GASEnvironment*, int, int, GASValue*, ExecuteType)`, `0x131111b0`..
`0x1315258`, **0x40a8 bytes**, and the fault is **0x2DA** into it.

### 1.2 Confirming the base without trusting it

A 64 KB-aligned base means `RVA & 0xFFFF == 0x148a`, so 255 functions in `.text` are candidates and
picking one by name would be circular. The instruction stream settles it. At `+0x2b4`:

```
+02a3  a180339d01       mov eax, [0x19d3380]        ; GFxAS2OpsThisBuffer
+02a8  3b0508b88501     cmp eax, [0x185b808]        ; GFxAS2OpBudget
+02ae  0f87303d0000     ja  …                       ; the budget break
+02b4  8b4104           mov eax, [ecx+4]            ; this->Length
+02b7  8b4b3c           mov ecx, [ebx+0x3c]         ; env->pTarget
+02ba  a3e8379d01       mov [0x19d37e8], eax        ; GFxAS2ErrLen = (int)Length
+02bf  891510b88501     mov [0x185b810], edx        ; GFxAS2ErrPC  = pc
+02c8  8d8180000000     lea eax, [ecx+0x80]
+02d1  85c9             test ecx, ecx
+02d3  741b             je  +02f0
+02d5  8b00             mov eax, [eax]              ; the GASObjectInterface vtable
+02d7  8b4dd8           mov ecx, [ebp-0x28]
+02da  ff5040           call [eax+0x40]             ; <<< slot 16 = ToSprite()
```

That is, instruction for instruction and in order, the opcode preamble of our own source:

```c
    GFxAS2ErrPC = pc;
    GFxAS2ErrLen = (int)Length;
    GFxASCharacter* target = env->GetTarget();
    GFxSprite* sprite = target ? target->ToSprite() : 0;
```

preceded by the `GFxAS2OpsThisBuffer > GFxAS2OpBudget` break. No other function in the image writes
those two globals in that order immediately before a conditional virtual call. The base is right.

### 1.3 The minidump settles it, and the brief never mentioned it exists

**There is a minidump of the user's crash** and nobody had opened it:
`DishonoredGame/Logs/unreal-v9411-2026.10.11-14.36.05.dmp`, 465 KB, written by the same crash
handler that printed the two useless lines. Parsed by hand (`MDMP`, eight streams: exception,
thread list, module list, memory list):

```
exception thread 0x5ed4  code 0xc0000005  address 0x1cc148a  params [0x0, 0x40]   (a READ of 0x40)
MODULE DishonoredGame-Win64-Shipping.exe  base 0x00db0000  size 0x181d000
eip 0x01cc148a   -> rva 0xf1148a
eax 0x00000000   ecx 0x312861a0   ebx 0x0ffc9e78   esp 0x004fe1cc   ebp 0x004fea10
```

Every number above was predicted before the dump was opened. **The module base is `0xdb0000`**,
which is what the suspended re-launch gave, so §1.1's method is sound and the decode in §1 is right.

And it adds the two things the decode could not give:

* **`eax = 0`**: the word at `target + 0x80` was **NULL**, and the fault is the read of `0x40`.
* **`ecx = 0x312861a0` is `target + 0x80`, so the dangling target was `0x31286120`** — and the
  dump's memory list happens to contain that page.

`0x31286120` sits in a 0xE0-stride allocator pool of character objects. Every **live** object in
that pool begins with an `.rdata` vtable pointer (`0x020ba9d0` or `0x020baa18`; the first resolves
to `??_7GFxASCharacter@@6BGASObjectInterface@@@`), then `RefCount 1`, then an id. The crashing
target begins with **`0x31286040`** — a pointer to the block exactly one 0xE0 stride below it,
which is what a binned free list looks like, and the block at `0x31286040` in turn begins with
`0x31285f60`, one more stride down.

```
31286040  31285f60 00000001 00000047 31286120   <- freed, links down
31286120  31286040 00000001 0000004b 00000000   <- the crashing target: freed, links down
  +048    00004000                              <- Depth == GFxASDepthBias: a script-attached clip
  +080    00000000                              <- the word the faulting instruction read as a vtable
31286200  020ba9d0 00000001 000000b7 312862e0   <- a LIVE character: .rdata vtable
```

**Three freed character blocks chained together, one of them still named by the action queue, and
its depth is `0x4000` — the bias retail puts on a clip that `attachMovie` created.** The measured
count in §4 is three dangling entries per CONTROLS tab selection. The dump and the instrument agree
on the number and on the kind of object.

### 1.4 What faulted

`ecx` (the target) passed its null test. `eax = target + 0x80` was read successfully at `+0x2d5` —
so the memory is still mapped — and the **vtable pointer it yielded** was then dereferenced at
`+0x2da` and faulted. The object is freed memory whose first word has been overwritten;
`target + 0x80` is `GFxASCharacter`'s second base, the `GASObjectInterface` sub-object (retail's is
at `+120`; ours is at `+128`).

Index 16 of that vtable is `ToSprite`. Counted off the declaration order in `GFxAS2Object.h` with
MSVC's single deleting-destructor slot at 0: `GetObjectType` 1, `SetMember` 2, `GetMember` 3,
`FindMember` 4, `DeleteMember` 5, `SetMemberFlags` 6, `VisitMembers` 7, `HasMember` 8,
`SetMemberRaw` 9, `GetMemberRaw` 10, `ToFunction` 11, `Set__proto__` 12, `Get__constructor__` 13,
`ToASObject` 14, `ToASCharacter` 15, **`ToSprite` 16 = offset 0x40**. The "slot 17" in that header's
own comment is retail's 1-based `vtables.csv` numbering, not a byte offset; reading it as one would
have landed on `ToASCharacter` and sent this one function wrong.

**Two identical frames is the walker failing, exactly as STATUS warns.** It fails for a plain
reason: `appProgramCounterToHumanReadableString` asks DbgHelp for a symbol, there is **no PDB beside
the staged executable** (the Release build emits a `.map` and no `.pdb`), so `SymGetSymFromAddr64`
and `SymGetLineFromAddr64` both fail and `StackWalk64` has no unwind data to walk with. The printed
line is the only thing the walker can produce. **Staging the `.map` is what made this solvable, and
`CaptureStackTraceHelper` writing `BackTrace[n] = AddrPC` even when `StackWalk64` returned FALSE is
what turns one failure into two identical lines.**

## 2. What retail does, resolved by hand

Both databases were opened headless on this agent's own copies. **The 2012 image is
`DishonoredGame-Shipping.exe` and the 2013 image is `Dishonored.exe`** — which is worth writing down,
because the names invite the opposite guess. It was pinned without a database, by reading the
MovieClip prototype method table out of the file: the pair at `.rdata` rva `0xdb3660` is
`{"setMask", 0xdf5320}`, i.e. handler rva `0x9f5320`, which is agent FF's value. In
`DishonoredGame-Shipping.exe` that pair is there; in `Dishonored.exe` it is not. The 2012 database
reports `imagebase 0x0`, so its addresses **are** rvas; the 2013 database reports `imagebase
0x400000`, so **every 2013 number here is a VA minus `0x400000`**.

| what | 2012 rva | size | 2013 rva | size |
|---|---|---|---|---|
| `GFxMovieRoot::ActionEntry::Execute(GFxMovieRoot*) const` | `0xa024d0` | 0x75 | `0x9f8ed0` | 0x75 |
| `GFxMovieRoot::ActionQueueType::AddToFreeList(ActionEntry*)` | `0xa0cb40` | 0x92 | `0xa03570` | 0x92 |
| `GFxMovieRoot::DoActionsForSession(unsigned)` | `0xa0d9e0` | 0x74 | `0xa04410` | 0x74 |
| `GFxSprite::CallFrameActions(unsigned)` | `0x9f63b0` | 0xef | `0x9ecb10` | 0xef |
| `GFxCharacter::OnEventUnload()` | `0x9ce800` | 0x74 | `0x9c50d0` | 0x74 |
| `GFxSprite::OnEventUnload()` | `0x9fdc80` | 0x59 | *no distinct symbol* | — |
| `GFxSprite::ExecuteEvent(const GFxEventId&)` | `0x9f4a10` | 0xe7 | `0x9eb1f0` | 0xe7 |
| `GFxDisplayList::Clear()` | `0x9d5e30` | 0x97 | `0x9cc630` | 0x97 |
| `GRefCountNTSImpl::Release()` | `0x9b2160` | 0x14 | `0x9ab1c0` | 0x14 |
| `GASAsFunctionObject::Invoke(…)` | `0x9f3540` | 0x11f | `0x9e9d10` | 0x11f |
| `GFxDisplayList::MarkAllEntriesForRemoval(unsigned)` (FG's, re-checked) | `0x9d5670` | 0x48 | `0x9cbed0` | 0x48 |
| `GFxDisplayList::UnloadMarkedObjects()` | `0x9d60a0` | 0x4c | `0x9cc8a0` | 0x4c |

**The entry refuses an unloaded character.** `ActionEntry::Execute`, decompiled:

```c
v3 = (_BYTE *)*((_DWORD *)this + 2);            // entry+8, the character
if ( v3 && (v3[118] & 0x10) == 0 )              // +0x76, bit 0x10: unloaded
{
    switch ( *((_DWORD *)this + 1) ) { case 1..4: vtbl+332/336/340/344; default: return; }
}
```

**The character at `entry+8` is a strong reference.** `AddToFreeList`, which is what recycles an
entry, releases `entry+0xc` and `entry+8` through `GRefCountNTSImpl::Release` before putting the
entry on the free list (capacity 0x32) or freeing it.

**The unloaded bit is written in exactly two places in the whole 2012 image** — found by scanning
`.text` for `or word ptr [reg+0x76], 0x10`, two hits, and both sit one instruction from the
`Event_Unload` (`4`) dispatch: `0x9ce86a` inside `GFxCharacter::OnEventUnload`, and `0x9f4ac9`
inside `GFxSprite::ExecuteEvent`. The second one also does this:

```asm
8b86bc000000   mov eax, [esi+0xbc]        ; pMovieRoot
66834e7610     or  word [esi+0x76], 0x10  ; mark unloaded
8b90fc270000   mov edx, [eax+0x27fc]      ; the movie root's unload list head
89968c000000   mov [esi+0x8c], edx
89b0fc270000   mov [eax+0x27fc], esi
ff4604         inc dword [esi+4]          ; AddRef
```

— retail links every unloaded sprite onto a movie-root-owned list **with a reference**, which is the
general reason a raw `GFxASCharacter*` is safe anywhere in retail for the rest of an advance. §7.

**And the unload reaches the subtree.** `GFxSprite::OnEventUnload` (`0x9fdc80`) is: raise flag
`0x100`, drop the hit area, set the movie root's `0x400` dirty bit, **`GFxDisplayList::Clear()`**,
then the base. And `GFxDisplayList::Clear` (`0x9d5e30`) is three passes — for every entry call
virtual slot 34 (`OnEventUnload`) and zero the child's parent pointer, then release every entry,
then free the array.

## 3. Everything the brief pointed at, and why none of it is the fault

The brief listed four candidates and had already cleared two. The decoded address clears all of
them: the fault is inside the ActionScript interpreter's opcode preamble, not in the renderer, not
in `GFxDisplay.cpp`, and not in any mask code.

* `ctx.MaskLevel` / `MaskDrawDepth` / D3D9 stencil nesting — not on the path. The crash is in
  `Advance`, not in `Display`: the log has no `scene rendered` line between `[0033.51]` and the
  fault at `[0033.77]`, so the frame never reached its render.
* a mask whose owner is re-created by the tab switch — **the right instinct, the wrong object.**
  Something *is* destroyed and used afterwards, but it is the action queue's target, not a mask.
* the `>= GFxASDepthBias` test in `MarkAllEntriesForRemoval` — reads `GetDepth()` on a character
  that is still in the list; untouched by this package and not implicated.
* `GFxSprite::CloneDisplayObject` — `duplicateMovieClip` appears nowhere in this screen's trace.

**And the three AS2 lines the brief quotes as the signature are not one.** Every
`OnTabSelected (1, r)` produces the whole sequence — `>> UpdateSubTabs`, `_bOpened = true`,
`_subCategoriesList.length : 3`, `>>> InitScrollBar` twice, `removeMovieClip … on undefined`,
`setMask … 170.0,214.6..1151.7,666.2` — and the **user's own log already contains the same sequence
at `[0021.64]` and at `[0031.98]`, twelve and two seconds before the crash, with no crash.** The
sequence is the CONTROLS tab rebuilding its list. The crash is what sometimes happens afterwards.

## 4. The defect, measured

`OptionsScreen.SetList` removes `_options_mc` and re-attaches it, and `ItemsList.InitList` attaches,
removes and re-attaches `lib_scrollBar` — all in one frame, because
`_root.optionsMenu_mc._bIsOpening` makes the rebuild synchronous. Each `attachMovie` queues the new
clip's frame-0 `DoAction` buffer through `GFxMovieRoot::PushActionBuffer` at `GFxAP_Frame`, and its
class binding through `QueueClassBinding` at `GFxAP_Lowest`. The clip is then destroyed later in the
same frame, before `DrainActionSessions` runs. `QueueClassBinding` already took a reference; **
`PushActionBuffer` did not**, so its entry was left pointing at freed memory, and the drain did
`Env.SetTarget(e.pTarget)` and `e.pBuffer->Execute(&Env)` — which is `GASActionBuffer::Execute`,
whose first use of the target is `env->GetTarget()->ToSprite()` at `+0x2DA`.

The instrument is the reference itself: with the entry holding a reference, an entry whose target's
`RefCount` is down to **1** at the drain is an entry that, without it, had already been freed. It is
counted for every entry, dropped or not. Scripted run `relham2`, the fixed build, 18 main-tab
selections driven by `resources/tools/drive_input.py`:

| tab selected | selections | entries held by the queue alone |
|---|---|---|
| `OnTabSelected (0, …)` GENERAL | 5 | **0** |
| `OnTabSelected (1, r)` CONTROLS | 5 | **15** (exactly 3 each) |
| `OnTabSelected (2, r)` GRAPHICS | 4 | **0** |
| `OnTabSelected (3, r)` AUDIO | 4 | **0** |

```
[0037.05] AS2 action queue: an action buffer queued on _level0 is held by the queue alone -
          unloaded 1 - without the queue's own reference this entry would have drained through a
          freed character (1 so far)                                        (x3, at every tab 1)
```

`OnTabSelected (1, r)` is the selection the user's process died on. The target path prints as
`_level0` because `GFxDisplayList::Clear` has already detached the clip from its parent, so
`GetTargetPath` walks straight to the root — these are the attached children of the `_options_mc`
that `SetList` removed.

### 4.1 What I could not do: make HEAD crash on demand

Three scripted runs against the untouched HEAD executable (`build/play`, byte-identical to the
`DishonoredGame-Win64-Shipping.exe` the user launched — `cmp` reports no difference), same driver,
same schedules, no focus loss in any of them:

| run | main-tab selections | `OnTabSelected (1, r)` | crash |
|---|---|---|---|
| `headsweep` | 4 | 1 | no |
| `headham1` | 17 | 5 | no |
| `headalt1` | 24 | 11 | no |

and the matched pair on the fixed build, same driver, same schedules, no focus loss:

| run | main-tab selections | `OnTabSelected (1, r)` | held by the queue alone | crash |
|---|---|---|---|---|
| `relham1` (before the unload propagation) | 18 | 5 | 0 — the unload never reached the children | no |
| `relham2` | 18 | 5 | **15** (3 each, 0 on the other tabs) | no |
| `relalt1` | 24 | 12 | **at least 16** — the log line is capped at 16 and the expectation is 36 | no |

**About 51 dangling drains on HEAD and the process survived every one.** That is the nature of the defect:
the freed `GFxSprite` is only fatal if its first word is overwritten, or its page decommitted,
between the free and the drain — a few hundred microseconds later. The user's run hit it once in
four tab switches; mine did not hit it in seventeen. No page-heap tooling is available on this
machine (`gflags` is not installed; the SDK's Debuggers directory holds only `dbghelp` and
`symsrv`), so I could not force the reuse.

**So the before/after in this package is on the defect, not on the symptom**, and the acceptance
item "a before/after against the untouched HEAD executable through the same driver" is met only in
that weaker sense: HEAD runs the same schedule and cannot report anything, because the thing that
reports is the reference the fix adds. I would rather say that plainly than claim a green run means
the crash is gone. What is certain is the chain: the faulting instruction is
`env->GetTarget()->ToSprite()`; the only cross-frame holder of a raw target in this tree is the
action queue; and that holder is measured dangling three times on exactly the tab switch the user
crashed on.

## 5. The fix

Three parts, all retail's.

**`GFxMovieRoot::PushActionBuffer`** takes a reference on the target, as retail's `GPtr` at
`entry+8` does.

**`GFxMovieRoot::DoActions`** applies retail's `ActionEntry::Execute` test — an entry whose target
is unloaded is dropped before the type switch — and releases the target on every path. The release
that used to live in the class-binding arm moved to the common tail, so `QueueClassBinding`'s
existing reference is still balanced exactly once. The target leaves the environment *before* the
release, so the release that destroys a character cannot leave `env->Target` pointing at it for even
one statement. **`GFxMovieRoot::~GFxMovieRoot`** releases whatever is still queued before
`free(Actions)`, which is what retail's `ActionQueueType` destructor and `Clear` do through
`AddToFreeList`.

**The unload has to reach the subtree or the test is dead.** `GFxCharacter::OnEventUnload` was
`{}` and `GFxDisplayList::Clear` was a release loop, so the only character ever marked was the one
script named; its children were freed without a word. The first build of this fix reported **zero**
dropped entries over eighteen tab switches for precisely that reason. `GFxDisplayList::Clear` now
does retail's two missing passes — `OnEventUnload` and `pParent = 0` for every entry, then the
release sweep — and `GFxSprite::OnEventUnload` calls it. The script dispatch
(`ExecuteEvent(Event_Unload)`, the `onUnload` handler) is still **not** ported; only the flag and the
propagation are, and that is written down in the header comment so the next reader does not take the
override for a complete one.

Zeroing the parent is its own small fix: a child that something else still holds used to be left
pointing at the parent that was about to be freed.

## 6. Gates

| gate | result |
|---|---|
| clean full Release, `build/agentFJ_clean` deleted first, `DISHONORED_LAYOUT_CHECKS=ON`, targets `DishonoredGame CoreSmoke LayoutProbe` | **0 errors, 0 C4263, 0 C4264**, 996/997 steps, all three executables linked |
| `rva_sweep.py --path source/Development/Src/External/GFx3` | **698 citations, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013** (265 ok-2013, 428 ok-2013-mid, 5 ok-2012-labelled) |
| `run_regression.py`, worktree's own copy, absolute build dir `build/agentFJ_wt/build/agentFJ_gate`, built inside the harness | **37 ok, 0 failed, 0 skipped**, 767 s |

One more check, because the unload propagation is the riskiest line in the package: the Options
screen is pixel-for-pixel unchanged. Same driver, same schedule, `-apshottime=60 -apshotname=fj`,
in-process capture on both builds — `build/agentFJ/headsweep_apshottime_fj00000.bmp` and
`relsweep_apshottime_fj00000.bmp`, both 1600x900. Mean absolute difference **1.56/255**, which is
the animated street behind the menu; the panel, the four tabs, the six rows and the slider chrome
are identical. And the first-drawn-frame census is identical to the digit on both builds:
`3 frames advanced, 108 sprites created, 212 display objects placed, 1196 action buffers,
33195 opcodes (0 unimplemented), 5 script errors`.

## 6.1 Two machine facts worth carrying

* **Real mouse input works again.** Every run in this package was driven with
  `resources/tools/drive_input.py` `hoverclick`, and **every click landed**: `grep -c FocusLost` is
  **0** on all **eight** runs and the Options tab strip responded to 80-odd clicks. The firewall fix
  (`98e22d4`) is what did it — STATUS open defect 8 can be closed on this evidence. The Options main
  tabs are at **y = 0.216** of the client area, x = **0.259 GENERAL, 0.419 CONTROLS, 0.581 GRAPHICS,
  0.741 AUDIO**; the *selected* tab rises to y ≈ 0.138, so always click the one you are leaving for.
* **The window's client rect and the backbuffer disagree.** `drive_input.py` reported a 1280x720
  client while `-apshottime` wrote a 1600x900 bitmap in the same run, and an earlier run of the same
  command line reported 1600x900 for both. Fractional coordinates are unaffected; **a pixel
  coordinate taken off a capture is not a pixel coordinate for the driver.**

## 7. The one defect found and left

**`GASFunctionObject::pDeclTarget` is the other raw character pointer that reaches the same
instruction.** `GFxAS2InvokeScriptFunction` (`GFxAS2Interp.cpp:443`) does
`env->SetTarget(fn->pDeclTarget)` and the function object can outlive the timeline it was declared
on. Retail guards it with the same bit — `GASAsFunctionObject::Invoke`, 2012 `0x9f3540`, 2013
`0x9e9d10`:

```c
v6 = CreateEnvironment(fnCall, &v13);
if ( (*(_BYTE *)(*(_DWORD *)(v6 + 116) + 118) & 0x10) != 0 )   // env->Target->Flags76 & 0x10
    v6 = *((_DWORD *)a2 + 6);                                  // fall back to the CALLER's env
```

**I did not write that guard, on purpose**: reading `pDeclTarget->IsUnloaded()` dereferences exactly
the pointer the guard is supposed to protect against, so on this tree it would convert a sometimes-
fatal bug into an always-fatal one. Retail can afford the guard because of the thing that is really
missing:

**The movie root's unload list is not ported.** Retail's `GFxSprite::ExecuteEvent` links every
unloaded sprite onto a list at `movieroot+0x27fc` with an explicit `inc [esi+4]`, and releases the
list at the end of the advance. That is what makes every raw `GFxASCharacter*` in retail safe for
the rest of the frame — including `DragState::pCharacter`, which retail also copies as a bare dword
(`GFxMovieRoot::SetDragState`, 2012 `0x9e1260`, no AddRef) and which this tree copies the same way.
**Port the unload list, then the `pDeclTarget` guard, in that order.** Both are in `External/GFx3`
and neither is in this package.

**And one crash that is not mine, found in the same Logs directory while looking for the user's
dump.** `unreal-v9411-2026.10.11-15.17.56.dmp`, written by `DishonoredGame-Win64-Shipping.exe` at
base `0x2e0000`, is `0xc0000005`, a **read of `0x8`**, at rva **`0xc67fca`** =
**`FDebugServer::Destroy + 0x2a`** (`IpDrv:FDebugServer.cpp.obj`). A read of `0x8` there is a null
`GSocketSubsystemDebug` reached through its vtable, i.e. a teardown that runs on a debug server that
was never initialised. It is in the area `98e22d4` just changed and it belongs to whoever owns that
commit. **I did not touch it and did not investigate further.**

Two further things found while reading, both outside this package and both reported to the
coordinator rather than touched: our VM's MovieClip-to-String conversion yields `undefined`, so
`on undefined` in an AS2 error message does **not** mean the object was undefined; and
`removeMovieClip` fails on a `registerClass`-backed instance (`_common.ScrollBarBase`) while
succeeding on plain clips in the same frame.

## 8. Every function touched, per file

`source/Development/Src/External/GFx3/GFxPlayer.h`
  * `GFxCharacter::OnEventUnload` — was `{}`, now raises `bUnloaded`.
  * `GFxCharacter::IsUnloaded` — new accessor.
  * `GFxCharacter::bUnloaded` — new member, immediately after `bVisible`.
  * `GFxSprite::OnEventUnload` — new virtual declaration.
  * `GFxMovieRoot::Census::ActionsDroppedUnloaded`, `::ActionsDroppedDangling` — new counters.
  * Nothing else. `SetMask`, `GetMask`, `GetMaskOwner`, `pMaskCharacter`, `pMaskOwner`,
    `GFxASDepthBias`, `GFxCharacterHandle`, `DragState` and `CloneDisplayObject` are agent FF's and
    are untouched.

`source/Development/Src/External/GFx3/GFxPlayerRoot.cpp`
  * `GFxMovieRoot::~GFxMovieRoot` — releases queued targets before `free(Actions)`.
  * `GFxMovieRoot::PushActionBuffer` — `target->AddRef()`.
  * `GFxMovieRoot::DoActions` — the measurement, retail's unloaded test, one release per entry.
  * `GFxMovieRoot::QueueClassBinding` — **not changed**; only its matching release moved.

`source/Development/Src/External/GFx3/GFxPlayerSprite.cpp`
  * `GFxCharacter::GFxCharacter` — one member initialiser.
  * `GFxDisplayList::Clear` — retail's unload-and-detach pass.
  * `GFxSprite::OnEventUnload` — new.
  * `MarkAllEntriesForRemoval`, `UnloadMarkedObjects`, `UnloadAll`, `RemoveDisplayObject`,
    `RemoveAt`, `InsertAt`, `AddDisplayObject`, `MoveDisplayObject`, `ReplaceDisplayObject`,
    `GFxCharacter::SetMask` and `~GFxCharacter` are untouched.

## 9. Where the brief was wrong

1. **"`0x1cc148a` is a VA in our own executable — subtract the image base."** The executable is
   `/DYNAMICBASE`; `0x1cc148a - 0x400000` is 644 KB past the end of the image. The base that run was
   `0xdb0000`, recovered from the PEB of a suspended re-launch. §1.1.
2. **"The shape: … two `OnTabSelected`, `InitScrollBar` twice, `removeMovieClip`, `setMask`, then
   `Critical error`."** That shape is every CONTROLS tab selection. It occurs twice in the user's own
   log before the crash and 17 times across three HEAD runs that did not crash. §3.
3. **"This package owns `External/GFx3`'s mask and display-list code" — and all four open candidates
   were in it.** The fault is in `GFxAS2Interp.cpp`, in the interpreter's opcode preamble, reached
   from `GFxPlayerRoot.cpp`'s action-queue drain. The one display-list change that was needed
   (`GFxDisplayList::Clear`) is not a mask change and was not on the list. §2, §5.
4. **"Reproduce it in a scripted run so the fix has a control."** The sequence reproduces every time;
   the crash does not reproduce at all. 51 measured dangling drains on HEAD, zero deaths. The
   control had to become a measurement of the defect rather than of the symptom, and that is the one
   acceptance item this package does not meet as written. §4.1.
5. **"Get a real stack (a debugger, or `-forcelogflush` plus a crash handler that walks properly)."**
   Neither was needed and neither would have been enough. The crash handler already wrote a
   **minidump** next to the log — `unreal-v9411-2026.10.11-14.36.05.dmp`, 465 KB, 11 Oct 14:36:05 —
   and it carries the module base, the exception parameters, the register file and the freed
   object's own bytes. The reason the printed stack is useless is not the walker's logic but a
   missing PDB; the reason the address is unresolvable is ASLR; and the dump answers both. **Open
   the dump before rebuilding anything.**
6. Smaller: the brief says `removeMovieClip`'s guard is "FF's signed `>= 0x4000` test". The guard
   that matters in this fault is not a depth test at all; and the `removeMovieClip … on undefined`
   error in the crash frame is a failure to *resolve the method*, so nothing is removed by it. The
   clip that is actually destroyed is `_options_mc`, by `SetList` at AS pc 7637.
