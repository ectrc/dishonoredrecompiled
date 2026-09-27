# Agent BC — the GFx 3.3 ActionScript 2 machine: the value model, the display list, the interpreter (2026-09-27)

Package **BC** of `PHASE8.md`: port the AS2 virtual machine and the player onto agent BB's reconstructed
API in dependency order — the value model and object interface first, then the display list and the
tag/character model, then the bytecode interpreter — drive it from a harness rather than from the game,
and report honestly what fraction of the opcode set and the class library is implemented.

**All three are done, and the machine runs further than the acceptance asked for.** The main menu's own
cooked asset gets through five frames: 85 sprites instantiated, 170 display objects placed, 76 action
buffers executed, 19 AS2 classes registered, 3,783 opcodes executed and **not one opcode without an
implementation**. All 22 cooked movies in the retail cook run five frames each with exit code 0.

## Result

| Step | State |
|---|---|
| 1. The value model and `GFxValue::ObjectInterface` | **done**: `GASString`/`GASStringManager` (interned, cached hash), `GASValue` with its 12-value type enum read out of the retail setters one by one, `GASObject` with the prototype-chain member hash, `GASArrayObject`, `GASFunctionObject`, `GASSuperObject`, `GASPrimitiveObject`. **All 25 `GFxValue::ObjectInterface` methods are defined** — the 1,096 of the engine's 1,350 calls into libgfx that agent BB left declared-and-undefined |
| 1b. The display list and the tag/character model | **done**: `GFxCharacter` → `GFxASCharacter` → `GFxSprite`, `GFxDisplayList` (depth-sorted, with retail's mark-and-sweep loop rebuild), `GFxCharacterHandle`, `GFxTimelineDef`/`GFxSpriteDef`/`GFxMovieDataDef`, the execute tags, `GFxStream`, `GFxMovieDefImpl` and `GFxMovieRoot` — **all 73 `GFxMovieView` slots implemented** |
| 1c. The bytecode interpreter | **done for 94 of retail's 98 opcodes** (95.9 %). The set is not a guess: it is the 98 case labels of `GASActionBuffer::Execute`, decompiled |
| 2. The harness | **done**: `GFx3Run --run <asset>.gfx --frames N` loads a cooked payload, instantiates its root movie clip, advances frames and reports every object created and every action executed. `--opcodes` and `--classes` print the acceptance tables |
| 3. The honest fractions | **opcodes 94 / 98 = 95.9 %; class library 10 of 28 groups = 277 of 898 retail functions = 30.8 %**. Section 6 names what remains, with each item's retail function count |
| Regression / playability | **green**: `run_regression.py --build-dir build/agentBC_rel --no-build` = **30 ok, 1 failed, 0 skipped**, and the one failure is a bound a package that merged *after* my snapshot base tightened (section 7). `-newgame` runs the retail two-map-change route with **0 criticals** |

## 0. The one thing that changes the method: the PDB has no layout for any of this

Agent BB's `GFx3Gen.h` is generated, with every member at its recorded offset and 380 `offsetof`
assertions, because the 2012 PDB carries the layout of every type the *engine's own* translation units
name. **The runtime's internal types are not in that PDB at all.** Measured first, before writing a line:

```
python resources/tools/pdb/dia_types.py <DishonoredGame-Shipping.pdb> --udt-re "^GAS" --enum-re "^GAS"
  udts=1 enums=0            # one UDT: GASStringContext, size 0, no members
python resources/tools/pdb/dia_types.py <same> --udt GASObject GASValue GFxSprite GASEnvironment \
        GASActionBuffer GFxMovieRoot GFxASCharacter GASStringNode
  udts=2                    # GFxASCharacter sizeof 0, GFxMovieRoot sizeof 0; the rest not present
```

`libgfx` was linked in as a library whose type stream the game's PDB does not carry. So the four sources
of evidence this package works from are different from BB's, and each one is cited at the point of use:

1. **Every libgfx function's fully demangled MSVC signature** — class, method, every parameter type and
   its constness — for all 5,635 of them (`resources/docs/symbols/functions.csv`, `module=libgfx`;
   `build/agentBC/surface.py` reads them out and groups them). That fixes the *API* of every internal
   class exactly: `GASObject::SetMemberRaw(GASStringContext*, GASString const&, GASValue const&,
   GASPropFlags const&)` is the retail declaration, not an invention.
2. **`resources/docs/symbols/vtables.csv`** — IDA's own export, no PDB involved — for the dispatch shape.
   `GASObjectInterface` and `GASObject{for GASObjectInterface}` give 21 slots each, and that table is
   why the interface in `GFxAS2Object.h` has both a `SetMember` (slot 3) and a `SetMemberRaw` (slot 10).
3. **Headless Hex-Rays decompiles of the load-bearing bodies**: 72 functions in `build/agentBC/dec`,
   `dec2` and `dec3`, on my own database copy `resources/docs/idb/shipping2012_agentBC.i64`. These fix
   the *semantics*, and where a body reads a constant they fix the constant.
4. **`resources/docs/symbols/match_2012_2013.csv`** for the 2013 address of each one.

**Consequence, stated plainly: behaviour is ported, member offsets are not reproduced and are not
reproducible.** That costs nothing, because nothing here is ABI-visible to retail — we replace libgfx
rather than call into it — but it does mean these files carry no `GFX3_ASSERT_OFFSET` and
`GFx3Layout.cpp` is untouched by this package. Every one of the 231 retail functions cited is recorded
in `agentBC_status.csv` with its 2013 rva, and **230 of the 231 are byte-identical between the 2012 and
2013 builds** (ratio 1.000); the one exception is `GFxPlaceObject2::Unpack` at ratio 0.890.

## 1. What is in the tree

`source/Development/Src/External/GFx3/` gains 13 files, 9,979 lines. None of them includes an engine
header, which is what lets `build/agentBC_run.cmd` compile the whole machine with no engine at all.

| File | Lines | What it is |
|---|---|---|
| `GFxAS2.h` | 387 | `GASStringNode`/`GASString`/`GASStringManager`/`GASStringContext`, the 52 builtin strings, **`GASValue`** and its type enum, `GASPropFlags`, `GASMember` |
| `GFxAS2Object.h` | 344 | `GASObjectInterface` (the 21 vtable rows as a method set), `GASObject`, `GASArrayObject`, `GASFnCall`, `GASFunctionObject`, `GASSuperObject`, `GASPrimitiveObject`, `GASObjectCollector` |
| `GFxAS2Runtime.h` | 412 | the **98-opcode enum**, the 10 ActionPush types, `GASActionBuffer`, `GASWithStackEntry`, `GASLocalFrame`, `GASEnvironment`, `GASGlobalContext` |
| `GFxPlayer.h` | 870 | `GFxStream`, `GASExecuteTag` and the five concrete tags, `GFxCharPosInfo`, `GFxTagList`, `GFxTimelineDef`, `GFxCharacterDef`/`GFxPlaceholderDef`/`GFxSpriteDef`, `GFxCharacterHandle`, `GFxCharacter`, `GFxDisplayList`, `GFxASCharacter`, `GFxGenericCharacter`, `GFxSprite`, `GFxMovieDataDef`, `GFxMovieDefImpl`, `GFxMovieRoot` |
| `GFxAS2Value.cpp` | 795 | string interning, the value setters, every conversion (`ToBool`/`ToNumber`/`ToInt32`/`ToString`/`ToPrimitive`/`Typeof`), the eleven arithmetic and three comparison operators |
| `GFxAS2Object.cpp` | 634 | the member hash, the prototype walk, the property (getter/setter) path, arrays, the collector |
| `GFxAS2Runtime.cpp` | 651 | the value stack, local frames, the DefineFunction2 register window, **AS2 name resolution** (four lookups in a fixed order), path parsing and `FindTarget`, `OperatorNew`, `GASGlobalContext` |
| `GFxAS2Interp.cpp` | 1,342 | **`GASActionBuffer::Execute`** — the interpreter — plus the factored-out `*OpCode` cases and `GFxAS2InvokeScriptFunction` |
| `GFxAS2Lib.cpp` | 1,106 | the class library: Object (with `registerClass` and `addProperty`), Function (`call`, `apply`), Array (12 methods), String (12), Number, Boolean, Math (18 + 8 constants), MovieClip (13), Error, and the eleven `_global` free functions including **`ASSetPropFlags`** |
| `GFxPlayerData.cpp` | 920 | `GFxStream` (the bit reader), the **tag walk**, the timeline, the character dictionary, `GFxMovieDataDef::Read`, `GFxMovieDefImpl` |
| `GFxPlayerSprite.cpp` | 886 | `GFxDisplayList`, `GFxASCharacter` (the display properties and the member store), **`GFxSprite`'s frame machine** |
| `GFxPlayerRoot.cpp` | 1,252 | `GFxMovieRoot` — all 73 `GFxMovieView` slots, the action queue, the `GASValue`↔`GFxValue` bridge — and **all 25 `GFxValue::ObjectInterface` methods** |
| `Tools/GFx3Run.cpp` | 380 | the harness |

Plus a targeted edit to `cmake/GFx.cmake`: the eight units join the `gfx3` static library and a new
`GFx3Run` target (`EXCLUDE_FROM_ALL`) is added beside `GFx3Dump`. **Nothing else outside the directory
is touched** — in particular `GFxUI/Sources.cmake` is not, as the coordinator asked.

## 2. The evidence behind the parts that are easy to get wrong

### 2.1 `GASValue::ValueType` is measured enumerator by enumerator

Agent BB was caught out by three hand-guessed enums in `GFxValue.h` (its section 2.4a) and now guards
every enum with an assertion. There is no PDB enum to assert against here, so each value was read out of
the retail body that writes it — thirteen decompiles of 13-to-67-byte functions in `build/agentBC/dec2`:

| value | evidence (2012 rva) |
|---|---|
| `UNDEFINED = 0` | `GASValue::SetUndefined` 0x9acbc0 writes 0 |
| `NULLTYPE = 1` | `SetNull` 0x9acbd0 writes 1 |
| `BOOLEAN = 2` | `SetBool` 0x9acc20 writes 2 |
| `NUMBER = 3` | `SetNumber` 0x9acc00 writes 3 |
| `INT = 4` | `SetInt` 0xa65810 writes 4 |
| `STRING = 5` | `SetString` 0x9acbe0 writes 5 |
| `OBJECT = 6` | `SetAsObject` 0x9cbd30 writes 6 |
| `CHARACTER = 7` | `SetAsCharacter` 0x9cb7e0 writes 7 |
| `FUNCTION = 8` | `SetAsFunction` 0x9cb830 writes 8 |
| `PROPERTY = 9` | `GFxValue::ObjectInterface::GetMember` 0x9b0450 calls `GetPropertyValue` when the raw type is 9 |
| `UNSET = 10` | `IsUndefined` 0x9ca960 is `t == 0 \|\| t == 10` |
| `RESOLVE_HANDLER = 11` | `IsFunction` 0x9ca940 is `t == 8 \|\| t == 11` |

Two more facts from the same bodies, both load-bearing: `IsPrimitive` (0x9ca9a0) is types 1..5, and
**every setter only calls `DropRefs` when the old type is ≥ 5**, i.e. only `STRING` and above hold a
reference. `SetAsObject` also forwards to `SetAsFunction` when the object answers `GetObjectType() == 23`
— and 23 is measured, from the 6-byte `GASFunctionObject::GetObjectType` at 0x9af140.

`GASObjectType` is anchored the same way: nine values are read out of their 6-byte bodies
(`TextField` 13 at 0xa26c80, `Key` 22, `Function` 23, `MovieClipLoader` 25, `BitmapData` 26,
`LoadVars` 27, `TextFormat` 30, `StyleSheet` 31, `Date` 35), and the two *spans* come from
`GFxValue::ObjectInterface::GetMember` (0x9b0450), which routes a property's owner to `ToASCharacter()`
when the type is in [2,5] and to `ToASObject()` when it is in [6,44]. The values inside the spans that no
body in the cook reaches are interpolated and the header says so per line.

### 2.2 The opcode set is the retail switch, not an AVM1 reference

`GASActionBuffer::Execute` (2012 **0x9ea900**, 13,931 bytes, 2013 0x9e3600) decompiles to 2,876 lines
with two switch statements: 80 case labels for the single-byte opcodes and 18 for the length-prefixed
ones. The enum in `GFxAS2Runtime.h` is transcribed from those 98 labels. The published AVM1 tables agree,
which is the cross-check rather than the source — and the *disagreements* are informative:

* retail has **no case for `0x08 ToggleQuality` or `0x09 StopSounds`**; they fall through the default
  and are no-ops. Neither does this implementation, deliberately, and the default arm does not count
  them as unimplemented;
* `0x16`, `0x1A`, `0x1B`, `0x1E`, `0x1F`, `0x2D`–`0x2F`, `0x38`, `0x39` and `0x56`–`0x5F` are not
  opcodes at all and have no labels.

The nine cases retail factors out into `GASExecutionContext` are factored out here too, as static
functions named after them: `ExtendsOpCode` (0x9e64a0), `ImplementsOpCode` (0x9e6290),
`CastObjectOpCode` (0x9e6120), `InstanceOfOpCode` (0x9e66a0), `EnumerateOpCode` (0x9ea7a0),
`SetTargetOpCode` (0x9ea610), `Function1OpCode` (0x9e8060), `Function2OpCode` (0x9e8490),
`WaitForFrameOpCode` (0x9e58f0).

### 2.3 Member lookup: three behaviours the class registrations depend on

`GASObject::GetMemberRaw` (0x9dc890) and `SetMemberRaw` (0x9de1c0) decompile to 666 and 652 bytes and
between them settle three things a from-scratch implementation gets wrong:

1. the lookup walks `__proto__` and stops at the first owner that has the name — and it compares
   **interned node pointers** before it ever hashes, which is why `GASString` is an interned node with a
   cached hash rather than a buffer;
2. `__proto__` and `__constructor__` are **not members**. Both bodies intercept them by name, on the
   read side and the write side, before the hash is consulted (`*(ctx+320)` and `*(ctx+336)` in the
   decompile are those two builtin strings);
3. when the member found on a *prototype* is a `PROPERTY`, the getter runs against the object the lookup
   **started** from, not the object it was found on. `GFxValue::ObjectInterface::GetMember` (0x9b0450)
   and `GASEnvironment::GetMember` (0x9e5a10) both pass the starting object as the property's self.
   Every CLIK class in the cook exposes its properties through `Object.prototype.addProperty`, so this
   is the common path and not an exotic one.

`GetMemberRaw` also takes a case-insensitive branch when the string context's SWF version is ≤ 6
(`*((_BYTE *)a2 + 4) <= 6u`), which is ported and which the version-8 fontlib movies exercise.

### 2.4 Name resolution is four lookups in a fixed order

`GASEnvironment::GetVariableRaw` (0x9e9270) and `SetVariable` (0x9ea060): the local frames of the
innermost function (`FindLocal` 0x9e6be0), then the with-stack innermost first (the
`GArrayLH_POD<GASWithStackEntry>` every one of those signatures carries), then the current target
character, then `_global` and each loaded level (`CheckGlobalAndLevels` 0x9e0100). A dotted or slashed
name is split first (`ParsePath` 0x9e0390, `IsPath` 0x9df680) and the path part resolved to a character
(`FindTarget` 0x9e0e70) before the leaf name is looked up on it.

### 2.5 The frame machine, and why the action **queue** matters

`GFxSprite::AdvanceFrame` (0x9f93f0) increments, calls `IncrementFrameAndCheckForLoop` (0x9f4500), and
**only if the frame number actually changed** runs `ExecuteInitActionFrameTags`, then the EnterFrame
event, then `ExecuteFrameTags`. `ExecuteFrameTags` (0x9f60a0) runs the init actions first and then each
playlist tag through `ExecuteWithPriority(this, 4)`. `IncrementFrameAndCheckForLoop` wraps to 0 past the
end and, when the timeline has more than one frame, marks **every display-list entry for removal** so the
next frame's `PlaceObject` tags rebuild the list — that mark-and-sweep pair (`MarkAllEntriesForRemoval`
0x9d5670 / `UnloadMarkedObjects` 0x9d60a0) is why a looping clip does not accumulate children.

A `DoAction` tag does **not** run inline. `GASDoAction::ExecuteWithPriority` (0x9e42b0) pushes onto the
movie root's action queue and `GFxSprite::CallFrameActions` (0x9f63b0) opens a session and drains it.
That ordering, with `GFxAP_Init` sorting ahead of `GFxAP_Frame`, is what makes a `__Packages` class
registered by a `DoInitAction` visible to the frame-1 timeline code that instantiates it. Getting this
wrong shows up as exactly the failure mode the harness would otherwise report: classes registered after
they were needed.

### 2.6 `PlaceObject2`'s flag bits

Read out of `GFxPlaceObject2::UnpackBase` (0xa0bab0, 871 bytes): bit 0 Move, 1 HasCharacter, 2 HasMatrix,
3 HasCxform, 4 HasRatio, 5 HasName, 6 HasClipDepth, 7 HasClipActions — and the reason that body starts
its field walk at +5 instead of +1 when bit 7 is set is that `RestructureForEventHandlers` (0xa01ba0)
inserts a four-byte event-handler pointer in front of the data.

### 2.7 A `GFxValue` that names a display object holds a *handle*

`GFxValue::ObjectInterface::GetMember` (0x9b0450) does not treat `obj` as a character pointer when
`isDObj` is set: it calls `GFxCharacterHandle::ResolveCharacter` first and returns undefined when the
character is gone. So the union slot holds a refcounted `GFxCharacterHandle`, and the engine can keep a
`GFxValue` across a frame in which the clip is removed without dangling. That indirection is ported
rather than shortcut, and `GFxASCharacter`'s destructor nulls the handle's character rather than freeing
it. **The corollary is an ownership rule agent BE needs**: a managed `GFxValue` dereferences
`pMovieRoot` in `ObjectRelease` (0x9aed40), so every one of them must be destroyed *before* the movie
view is released. The harness demonstrates the rule with an explicit scope and a comment; getting it
wrong is an access violation at teardown, which is exactly how I found it.

## 3. Step 2's acceptance, measured: the main menu's own asset

`GFx3Run --run Dishonored_MainMenu.MainMenu.gfx --frames 5`, from the cmake-built binary
(`build/agentBC_rel/Binaries/Win32/GFx3Run.exe`), full output in `build/agentBC/runs/`:

```
  header         GFX v10  1280 x 720 px  30.0 fps  5 frames
  tags           4532 total, 4236 handled, 152 skipped by length
  dictionary     237 characters (143 sprites, 94 placeholders)
  exports        75   imports 10
  actions        107 DoAction, 64 DoInitAction, 5807 bytes of bytecode
  root clip      _level0, 5 frames, def 'MovieDataDef'

  -- after 5 advance(s) --
  frames advanced        5   current frame 5 of 5
  sprites created        85
  display objects        170 placed, 8 moved, 0 removed; list holds 10
  action buffers run     76   queued and drained
  AS2 objects created    1136 (live 1136)
  interned strings       1512
  classes registered     19 via Object.registerClass
  script errors          7
  opcodes executed       3783, of which 0 had no implementation
  23 distinct opcodes used, 23 of them implemented

  _level0 display list:
    depth      0  Sprite       id 47     DLCManagement_mc
    depth      1  Sprite       id 242    mainMenu_mc
    depth     50  Sprite       id 201    newGame_mc
    depth     68  Sprite       id 165    startScreen_mc
    depth     96  Character    id 243    gamepadMapping_mc
    depth     97  Character    id 244    gammaSetting_mc
    ...

  GFxValue::ObjectInterface round trip:
    _root is a display object; SetMember ok; GetMember ok -> 1234.5
    _root.mainMenu_mc resolved; Invoke(toString) ok
    CreateArray + PushBack -> size 1
```

`startScreen_mc`, `mainMenu_mc` and `newGame_mc` are the three instance names agent AW read out of
`UDisGFxMoviePlayerMainMenu::PostStart` (`gfx_decision.md` 2.4) and agent BB corrected the symbol names
for (`agentBB.md` 3.7) — **the runtime creates them, the engine-facing `GetVariable("_root.mainMenu_mc")`
resolves one of them, and `Invoke` on it reaches an AS2 method**. That is the whole `PostStart` shape,
minus the `Open` call whose own handler the content has not installed yet (section 6).

### All 22 cooked movies, five frames each

Every payload in the retail cook, through the same harness, **all exit 0**:

| asset | sprites | placed | buffers | classes | opcodes | distinct | errors |
|---|---|---|---|---|---|---|---|
| `UI_HUD_SF.HUD` | 135 | 262 | 133 | 31 | 3,902 | 22 | 2 |
| `UI_HUD_DLCTest_SF.HUD` | 122 | 238 | 122 | 31 | 5,065 | 32 | 1 |
| `UI_PauseMenu_SF.PauseMenu` | 90 | 169 | 47 | 11 | 2,644 | 20 | 2 |
| `Dishonored_MainMenu.MainMenu` | 85 | 170 | 76 | 19 | 3,783 | 23 | 7 |
| `UI_PowerWheel_SF.powerwheel` | 76 | 143 | 69 | 7 | 2,184 | 23 | 2 |
| `DishonoredGame.lib` / `Startup.lib` | 68 | 135 | 75 | 22 | 4,558 | 36 | 45 |
| `UI_MissionStats_SF.MissionStats` | 63 | 120 | 40 | 9 | 1,874 | 22 | 8 |
| `UI_Shop_SF.Shop` | 54 | 106 | 43 | — | 2,731 | 21 | 8 |
| `DishonoredGame.Note` | 47 | 105 | 29 | 5 | 1,468 | 20 | 2 |
| `Startup.OptionsMenu` | 44 | 121 | 80 | 21 | 5,699 | 34 | 6 |
| `UI_Journal_SF.Journal` | 34 | 66 | 83 | 27 | 4,412 | 30 | 15 |
| `Startup.LoadGame` | 34 | 66 | 31 | 5 | 4,454 | 34 | 24 |
| `DishonoredGame.HUDFX` | 17 | 28 | 13 | 3 | 523 | 20 | 1 |
| `DishonoredGame.Global` | 7 | 7 | 36 | 7 | 1,993 | 21 | 2 |
| `UI_Gamma_SF.GammaImage` | 2 | 1 | 15 | 1 | 981 | 20 | 4 |
| the 6 `DisFonts*` movies | 1 each | 2 each | 0 | 0 | 0 | 0 | 0 |

**Across the whole cook: 39 distinct opcodes are executed and every one of them is implemented.** The
union is `End, Play, Stop, Subtract, Multiply, Divide, LogicalNot, Pop, GetVariable, SetVariable, Trace,
Delete, DefineLocal, CallFunction, Return, New, InitArray, InitObject, Add2, Less2, Equals2,
PushDuplicate, GetMember, SetMember, Increment, CallMethod, NewMethod, BitAnd, BitRShift, StrictEquals,
Greater, Extends, StoreRegister, ConstantPool, DefineFunction2, Push, Jump, DefineFunction, If`. Note
what that means: **the four opcodes this package does not implement are not reached by any asset in the
game in its first five frames**, so they are not on the near-term critical path (they are, however, on
the *interaction* path — section 6).

## 4. Step 3's acceptance: the two tables

`GFx3Run --opcodes --classes` (full output `build/agentBC/tables.txt`).

### Opcodes: 94 of 98 (95.9 %)

The four remaining, and why each one is blocked rather than skipped:

| code | name | why it is not here |
|---|---|---|
| `0x27` | `StartDragMovie` | needs the mouse state, i.e. the input path. The operands are consumed so the stack stays balanced |
| `0x28` | `StopDragMovie` | same |
| `0x83` | `GetURL` | this is AS2's route to the host. Retail's case 131 of 0x9ea900 splits on `strncmp(url, "FSCommand:", 10)` and hands the rest to the loader queue. The handler is **agent BE's** (`FGFxFSCommandHandler::Callback`, 2013 0x586450), so the call is recorded rather than given an invented destination |
| `0x9A` | `GetURL2` | same |

Everything else is implemented, including the awkward ones: `DefineFunction`/`DefineFunction2` with the
full preload/suppress register protocol, `With`, `Try`/`Throw` (a try/catch inside one buffer and
propagation out of a call; a `finally` after a `return` is not resumed — section 6), `Extends`,
`ImplementsOp`, `CastOp`, `Enumerate`/`Enumerate2`, `CallFrame`, `GotoFrame2`, `StoreRegister`,
`ConstantPool`, and all 10 ActionPush payload types including the high-word-first double.

### Class library: 10 groups of 28, 277 of 898 retail functions (30.8 %)

| implemented | retail fns | what is in it |
|---|---|---|
| `Object` | 32 | prototype, `toString`, `valueOf`, `hasOwnProperty`, `isPropertyEnumerable`, `isPrototypeOf`, **`addProperty`**, **`Object.registerClass`** |
| `Function` | 8 | `call`, `apply` |
| `Array` | 129 | `push`, `pop`, `shift`, `unshift`, `splice`, `slice`, `concat`, `join`, `reverse`, `indexOf`, `sort` (comparator-aware), `toString`, `length` |
| `String` | 35 | `charAt`, `charCodeAt`, `indexOf`, `lastIndexOf`, `substring`, `substr`, `slice`, `split`, `toUpperCase`, `toLowerCase`, `fromCharCode`, `length` |
| `Number` / `Boolean` | 21 | constructors, `toString`, `valueOf`, `MAX_VALUE`/`MIN_VALUE`/`NaN`/±`INFINITY` |
| `Math` | 20 | 18 functions and 8 constants |
| `MovieClip` | 16 | `play`, `stop`, `gotoAndPlay`, `gotoAndStop`, `nextFrame`, `prevFrame`, `createEmptyMovieClip`, `attachMovie`, `removeMovieClip`, `getDepth`, `getNextHighestDepth` |
| `Error` | 4 | constructor with `message` |
| `_global` free functions | 12 | **`ASSetPropFlags`** (both forms), `trace`, `parseInt`, `parseFloat`, `isNaN`, `isFinite`, `getTimer`, `setInterval`/`setTimeout`/`clearInterval`/`clearTimeout` |

| remaining | retail fns | note |
|---|---|---|
| `TextField` | 125 | `GFxEditTextCharacter`; needs the text engine and the glyph rasteriser |
| `Date` | 109 | `GASDate` 68 + `GASDateProto` 41 |
| `Matrix`/`Point`/`Rectangle`/`Transform` | 55 | `GASMatrixProto` 16 + `GASRectangleProto` 17 + … |
| the five filter classes | 48 | drop shadow, glow, bevel, colour matrix, blur |
| `XML` / `XMLNode` | 47 | |
| `Stage` | 34 | `GASStageCtorFunction` + `GASStageProto`; `onResize`, `TranslateToScreen` |
| `TextFormat` | 33 | |
| `LoadVars` / `MovieClipLoader` | 42 | need the loader queue |
| `BitmapData` | 22 | needs the renderer; **this is `loadBitmap`, the single most-called missing function in the cook** |
| `Selection` | 19 | needs the text engine |
| `Sound` | 18 | audio is out of scope (`PLAN.md` Phase 10) |
| `StyleSheet` | 14 | |
| `NetConnection`/`NetStream` | 14 | video, out of scope |
| `Key` | 12 | needs the input path |
| `SharedObject` | 11 | |
| `Color` / `Mouse` | 18 | |

## 5. What the remaining script errors actually are

Across all 22 assets × 5 frames — 50,829 opcodes executed — the machine logs **182 script errors**, and
they are not 182 different problems. Aggregated (`build/agentBC/runs/`):

| count | error | what it is |
|---|---|---|
| 71 | `loadBitmap` is not a function | `flash.display.BitmapData.loadBitmap`. One class, 22 retail functions, needs the renderer and agent BE's `FGFxImageLoader`. All 71 come from the two `lib` movies' image helper |
| 44 | call of `undefined` | `ActionCallMethod` with an empty method name on an undefined receiver, i.e. the consequence of an earlier failure rather than a cause |
| 16 | `GotoLabeledFrame: no frame named 'PC'` | only in the two `lib` movies: a platform-switch label (`PC`/`360`/`PS3`) inside a CLIK component whose definition is imported from another movie, and imports are not bound yet (section 6) |
| 22 | `getNextHighestDepth` / `attachMovie` / `swapDepths` / `createEmptyMovieClip` / `gotoAndPlay` | the receiver is a `GFxGenericCharacter` — a shape, text field or button placeholder — rather than a sprite, because the character definition for it is one of the 31 tag loaders not ported |
| 25 | content methods (`InitHelpBar`, `SetInput`, `EnableInputs`, `InitTabs`, `SetTitle`, `CreateContainer`, `OpenHelp`, `Open`, `SetMenu`, `FillCategories`, `SetSaveGame`, `SaveProperties`, `PlaySound`, `LoadImage`, `InitScrollBar`, `tweenTo`) | the content's own methods, on clips whose class was installed by a registration that has not run yet or whose symbol is imported |
| 2 | `Stack overflow: more than 64 nested function calls` | a genuine defect, section 6 item 2 |

So the honest summary is: **the machine executes every opcode the content uses; what it is still missing
is library classes and character definitions, and the errors are overwhelmingly one of those two.**

## 6. What the next wave must add, in the order the measurements put it

1. **The 31 unported tag loaders, shapes first.** 152 tags per movie are skipped by length and each one
   defines a character that becomes a `GFxPlaceholderDef`. That is what produces the 22 "method on a
   generic character" errors and it is what a renderer will need anyway:
   `GFx_DefineShapeLoader` (2012 0xa36850), `GFx_DefineEditTextLoader` (0xa272a0),
   `GFx_ButtonCharacterLoader` (0xa35ac0), `GFx_DefineTextLoader` (0xa8d500),
   `GFx_DefineFontLoader` (0xa357d0), `GFx_DefineExternalImageLoader2` (0xa36210),
   `GFx_DefineSubImageLoader` (0xa369d0), `GFx_Scale9GridLoader` (0xa36680),
   `GFx_LoadFilters` (0xa93b60), `GFx_CSMTextSettings` (0xa26ca0), and the rest.
2. **Import binding.** `ImportAssets2` (tag 71) is parsed and its symbols keep dictionary slots, but they
   are placeholders: nothing resolves `..\DisFonts\gfxfontlib.swf` to the movie that exports
   `$NormalFont`. That needs `GFxLoader`'s state bag and `FGFxFileOpener` (agent BB's seam, already
   implemented) plus `GFx_ImportLoader` (0xa385f0). It is what the 16 missing-frame-label errors and a
   good share of the 44 `undefined` calls reduce to.
3. **The text engine and the glyph rasteriser.** `TextField` is the largest single remaining class (125
   functions) and agent BB's section 3.4 established that the fonts are **glyph outlines, not textures**
   — there is no font-texture tag anywhere in the cook — so `GFxTextField` 294 + `GFxStyledText` 134 +
   `GFxTextDocView` 121 + the glyph caches, 270 KiB and ~1,140 functions, cannot be skipped. **Nothing
   the UI does will show a character of text until this lands.** It is the biggest remaining item in the
   whole GFx project and it deserves a package of its own.
4. **A real garbage collector.** `GASObjectCollector` here is only the *teardown* half of retail's
   `GASRefCountCollector`: every `GASObject` is registered and freed in one pass when the movie dies,
   and `Release()` reaching zero mid-frame frees nothing. AS2 object graphs are full of cycles — every
   one of the 669 `__Packages` registrations builds one, because a class's prototype names its
   constructor and the constructor names its prototype — so a plain refcount cannot free them and retail
   uses mark-and-sweep (the `GRefCountBaseGC<323>` `ForEachChild_GC`/`ExecuteForEachChild_GC` family, and
   `GASRefCountCollector`, 6 functions in `GFxAction.obj`). Today a movie that runs for an hour grows.
   The two-pass `PrepareForCollection` + delete in `GASObjectCollector::FreeAll` is correct and the
   harness proves it (no leak across movies, exit 0 on all 22), but it is not a collector.
5. **One genuine recursion defect.** `UI_Shop_SF.Shop` and one other asset hit the 64-activation guard.
   The guard is what keeps it from being a stack overflow (and the `ToPrimitive`/`ToString` depth counter
   at 255 — which is retail's own, `if (v18 >= 0xFFu)` in 0x9cbe50 — is what keeps *that* loop bounded),
   but the underlying cause is almost certainly `super` resolution when a method is invoked with a
   prototype rather than an instance as `this`: `GASSuperObject` is built from
   `this.__proto__.__proto__`, so if `this` is already a prototype the parent lookup finds the same
   function and recurses. Reproduce with `GFx3Run --run UI_Shop_SF.Shop.gfx --frames 5` and read the
   error; the fix is to carry the *declaring* prototype on `GASFunctionObject::pOwnerProto` (the field is
   declared and unused for exactly this reason) instead of deriving it from `this`.
6. **`GFxLoader`'s non-virtual API is not declared.** `GFxLoader::CreateMovie` (2013 **0x9b4030**) and
   `GetMovieInfo` (2013 **0x9b3fe0**) are the two functions the engine calls to open a movie, and
   `GFx3Gen.h` carries only `GFxLoader`'s members and its one virtual — the generator emits virtuals, not
   the non-virtual member functions. So this package constructs `GFxMovieDataDef` + `GFxMovieDefImpl`
   directly and the harness bypasses `GFxLoader` entirely. **Agent BB:** one generator change (emit
   public non-virtual members for the hand-picked classes, or hand-write `GFxLoader`'s five) unblocks the
   engine-side path; I did not edit your headers, as you asked.
7. **`GFxFunctionHandler`'s parameter struct.** `GFxMovieRoot::CreateFunction` (2012 0x9af5c0) must wrap a
   `GFxFunctionHandler`, whose `Call(Params&)` signature needs `GFxFunctionHandler::Params`, which the
   generated header does not carry. The body returns undefined and says so. Same fix as item 6.
8. Then, in rough order of what the menu needs: `Stage` (34), `Key` and `Mouse` (21, with the input
   path), `TextFormat` (33), the `Matrix`/`Point`/`Rectangle` trio (55), `Color` (9), the filter classes
   (48), `XML` (47), `Date` (109). And the four opcodes of section 4 as their dependencies arrive.
9. **Smaller and recorded**: `SetMatrix3D` returns false rather than inventing a 3D transform
   (`GFxCharacter::CreateMatrix3D` 0x9ce370 plus the renderer's 3D path); `GetText`/`SetText` read and
   write a member named `text` rather than a real text field, which is where AS2 writes it anyway;
   `HandleEvent`, `HitTest` and the mouse-state slots of `GFxMovieView` answer from state because the
   button and focus model (`GFxButtonCharacter` 38, `GFx_GenerateMouseButtonEvents` 0xa66a90) is not
   here; `swapDepths` needs `GFxDisplayList::SwapDepths` (0x9d5c10); `Display`/`DisplayPrePass` walk
   nothing because drawing is the seam's and agent BD's.

## 7. Verification

* **The machine on its own** (`build/agentBC_run.cmd`, VS 2022 x86, `/Zp4`, no engine): 0 errors, 0
  warnings at `/W3`, `GFx3Run.exe` produced. This is the fast loop and it proves the same thing BB's
  seam script proves: the runtime needs no engine.
* **Warnings at the engine's own level: 0 from these units.** The first full build produced 182 (180
  `C4100` unreferenced-parameter from the inline interface stubs, plus one `C4189` and one `C4701`) and
  all of them are gone. The `C4701` was a real latent defect rather than noise: `ActionCallFrame`'s frame
  number was read uninitialised when the operand was a string with no matching label.
* **Isolated full RELEASE build**: `python resources/tools/make_snapshot.py BC --list
  build/agentBC_files.txt` then `build/agentBC_relbuild.cmd`. **838 units, 0 errors, 0 link errors**,
  all five targets produced (`DishonoredGame.exe`, `CoreSmoke.exe`, `LayoutProbe.exe`, `GFx3Dump.exe`,
  `GFx3Run.exe`), `-- GFx: DISHONORED_WITH_GFX3=1` in the log (`build/agentBC_relbuild.log`).
  The cmake-built `GFx3Run.exe` reproduces the numbers of section 3 exactly, which is what proves the
  build wiring rather than only the hand-rolled script.
* **The snapshot base is `4db19d1` (agent BB's merge), not HEAD, and that is a finding the coordinator
  needs**: HEAD `f2fbc4c` **does not compile**. `GFxUI/Src/gfxuimovie.cpp`,
  `gfxuiexternalinterface.cpp` and `gfxuinatives.cpp` call `UGFxObject::GetASValue`,
  `UGFxDataStoreSubscriber::PublishValues` and `UGFxFSCmdHandler_Kismet::FSCommand`, which live in
  `GFxUI/Inc/CppText/*.h` and are reachable only through `#include "CppText/…"` lines in the generated
  `GFxUI/Inc/GFxUIClasses.h`. The shared working tree has three of the five added (uncommitted:
  `CppText/UGFxInteraction.h`, `CppText/UGFxMoviePlayer.h`, `CppText/UGFxObject.h`) and is still missing
  `CppText/UGFxDataStoreSubscriber.h` and `CppText/UGFxFSCmdHandler_Kismet.h`, both of which exist on
  disk; `DishonoredGame/Src/dishonoredpawn.cpp` has
  the same shape of problem with `UDisAttributes::IsCensusEnabled` (agent BF's). **The module-header
  regeneration for packages BE and BF has not landed.** I verified this against a clean worktree of
  HEAD with none of my files in it, so it is not mine. Building on `4db19d1` + my 14 files isolates this
  package exactly, which is what a snapshot is for.
* **Regression**: `python resources/tools/run_regression.py --build-dir build/agentBC_rel --no-build`
  → **30 ok, 1 failed, 0 skipped, 433 s** (`build/agentBC_regression.txt`). The single failure is
  `d3d9/unported_natives 2 (wanted <= 0)`: agent BF tightened that bound from 4 to 0 in
  `resources/docs/regression_baseline.json` when it merged, *after* my snapshot base, and the two natives
  are the two BF ported. Re-running the same stage against the baseline **committed at my base commit**
  (`python build/agentBC_wt/resources/tools/run_regression.py … --only d3d9`) gives **8 ok, 0 failed**
  (`build/agentBC_regression_base.txt`). Everything else is at or above HEAD: CoreSmoke 99/0, layout
  2,314 types with 0 mismatches and 0 contract mismatches, nullrhi 0 criticals, d3d9 2,580 frames /
  0 criticals / 6,506 draw elements / 16,257 textures, inputtest **pawn walks 1,013.7 units** with
  0 criticals, 895 PhysX actors, 1,312 static shapes, touch census 236, sequence census 10,219.
* **`-newgame` still works** on the same exe (`build_and_smoke.py … --exe-name DishonoredGame_BC.exe
  --rhi null "--extra-args=-newgame -forcelogflush"`, exit 0): `Initial startup: 2.55s`, then
  `Committed map change via DishonoredEngine`, then `DISHONORED(bringup): startmap: 'ce
  ChangeLvl_StartNewGame' after the Dishonored_MainMenu commit`, then the second commit at 3.10 s — the
  retail New Game route end to end, **0 criticals in 9,917 log lines**. `-startmap` is what the
  regression's d3d9 and inputtest stages already run. (A first attempt with the shared
  `DishonoredGame_AX.exe` name failed to stage because another agent's run held the file; use a distinct
  `--exe-name` when six packages share one machine.)
* **Nothing instantiates the runtime in a normal run.** The eight new units are in the `gfx3` static
  library that `dishonored_apply_defines()` already linked into every target for BB; they include no
  engine header, they declare no `UObject`, and no engine or `GFxUI` translation unit names
  `GFxMovieRoot`, `GASObject` or `GASValue` (checked: the only match outside the directory is a comment
  in `DishonoredGame/Inc/dishonoredgameaiclasses.h`). The switch is unchanged and
  `-DDISHONORED_WITH_GFX3=OFF` still removes the whole directory from the build.
* **All 231 cited retail functions** are in `agentBC_status.csv` with their 2013 rvas: **224 ported, 7
  named as remaining, 230 of 231 byte-identical 2012↔2013** (ratio 1.000), the exception being
  `GFxPlaceObject2::Unpack` at 0.890.

## 8. Hand-overs

**Agent BB — four things, none of which needs you to change a layout.**
1. `GFxLoader`'s non-virtual API (`CreateMovie`, `GetMovieInfo`, the constructor, `SetState`) is not in
   `GFx3Gen.h`; the generator emits virtuals only. Section 6 item 6. Same for
   `GFxFunctionHandler::Params` (item 7).
2. `GFx3RuntimeStubs.cpp`'s 57 entries are **not all work items**, and the workflow note should say so.
   Most are genuine base-class defaults that retail also implements as base defaults —
   `GFxResource::GetKey` returning an empty key, `GTexture::ChangeHandler::OnChange` doing nothing,
   `GRenderer`'s optional slots. Deleting those would break the link for every derived class that does
   not override them. I overrode `GFxStateBag`'s four in `GFxMovieRoot` and `GFxMovieDefImpl` with real
   storage rather than deleting the base bodies, which is the right shape; I deleted nothing.
3. `GFxValue::ObjectInterface`'s 25 methods are now **all defined** (`GFxPlayerRoot.cpp`). Your
   `GFxValue.h` is unchanged.
4. The container parser is used as-is: `GFxMovieDataDef::Read` calls `GFxGfxParseFile` for the header and
   the summary and then walks the tags itself, which is exactly the plug-in point your hand-over
   described.

**Agent BE — the ownership rule, and three answers.** (1) A managed `GFxValue` holds a reference through
the movie's own `ObjectInterface` and `ObjectRelease` dereferences `pMovieRoot`, so **every `GFxValue`
must be destroyed before the movie view is released** — `UGFxObject::BeginDestroy` has to drop its value
before the movie player closes, or it is an access violation at teardown. Section 2.7. (2) The same rule
applies to `GASString`: an interned node belongs to the movie's string manager, so a `const char*` taken
from a `GFxValue` string is valid only while the movie lives. (3) `GetURL`'s `FSCommand:` split is
recorded but not routed, because the destination is your `FGFxFSCommandHandler::Callback` (2013
0x586450) — wire it when the runtime is instantiated. Your `gfxui_gfx3.h` and every `GFxUI` unit are
untouched by me, and so is `GFxUI/Sources.cmake`.

**Agent BD** — nothing in this package touches the renderer, and `GFxMovieRoot::Display` deliberately
draws nothing. When the GFx pixel-shader families land, the traversal order a renderer needs is already
in `GFxDisplayList` (depth-ascending, clip depth carried per entry) and `GFxSprite::Display`'s retail rva
is 0x9faeb0.

**Coordinator** — three things. (1) **HEAD `f2fbc4c` does not compile**: the module-header regeneration
for packages BE and BF is missing (section 7, second bullet). That needs doing before anything can be
measured on HEAD. (2) `gfx_decision.md` section 4's second row ("the menu is presentation, not
playability") can now be answered with a measurement rather than a guess: the HUD, journal, power wheel
and shop assets all run their first frames on this machine, and what stops them being *visible* is one
thing — the text engine and the glyph rasteriser of section 6 item 3. That is the single item to
schedule next and it is large enough to be its own package. (3) `middleware.md` 2.3 should gain the line
that the reconstruction now has a working AS2 machine at 94 of 98 opcodes, with the fractions of
section 4.

## 9. Files

Mine (16): the 13 files of `source/Development/Src/External/GFx3/` listed in section 1, a targeted edit
to `cmake/GFx.cmake`, plus this report and `agentBC_status.csv`. The snapshot list is
`build/agentBC_files.txt`.

Scratch (not repo tools): `build/agentBC/surface.py` (the libgfx surface from the demangled names),
`build/agentBC/gas_types.json` (the PDB dump that proves the layouts are absent),
`build/agentBC/dec{,2,3}` (72 headless decompiles) and `dec{1,2,3}.txt` (their function lists),
`build/agentBC/rvas.txt` (the 231 cited rvas with their 2013 addresses and match ratios),
`build/agentBC/runs/*.txt` (all 22 assets, five frames, verbose), `build/agentBC/tables.txt` (the two
acceptance tables), `build/agentBC/cmake_run.txt`, `build/agentBC_run.cmd` (the no-engine build),
`build/agentBC_relbuild.cmd` + `build/agentBC_relbuild*.log`, `build/agentBC_regression.txt` and
`build/agentBC_regression_base.txt`, `build/agentBC_newgame_out.txt`, snapshot `build/agentBC_wt`
(detached at `4db19d1`) and build directory `build/agentBC_rel`.

IDA: one own copy, `resources/docs/idb/shipping2012_agentBC.i64`, opened headlessly through
`resources/tools/ida/run.py` only; no MCP tool of any kind was used, and no FModel tool. No commits, no
`git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.
