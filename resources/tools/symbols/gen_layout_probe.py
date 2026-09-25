"""P2.5: compiled layout probe. Generates a console program that prints sizeof/offsetof for every
PDB type a module declares, then compares its output with the PDB layout.

Members are looked up with SFINAE so a PDB member the ported header lacks prints MISSING instead of
failing the build. Types the module's main header does not declare are collected from a build log
into resources/docs/types/probe_skip_<Module>.txt and left out on the next generate.

Usage:
  python resources/tools/symbols/gen_layout_probe.py generate Core [Engine ...]
      -> source/Tests/LayoutProbe/probe_<Module>.cpp (+ probe_main.cpp, CMakeLists.txt)
  python resources/tools/symbols/gen_layout_probe.py skip-from-log Core <build.log>
      -> appends undeclared types to probe_skip_Core.txt
  python resources/tools/symbols/gen_layout_probe.py compare build/x86-debug/layout_probe.txt
      -> resources/docs/types/reference_layout_delta.md
  python resources/tools/symbols/gen_layout_probe.py show FArchive [UObject ...]
      -> prints the PDB layout (offset, size, type, name) of each type
  python resources/tools/symbols/gen_layout_probe.py props Engine AActor [APawn ...]
      -> rewrites the //## BEGIN PROPS block of each script class in <Module>/Inc/*Classes.h from
         the PDB member list (reference lines kept by name, Arkane members synthesized, reference-only
         members dropped and listed)

The probe only evaluates sizeof/offsetof, so it must not pull the module libraries into the link
(Core.lib references Engine symbols). The only module symbols the probe objects reference are
appMalloc/appFree (UnFile.h inline operator new/delete); probe_main.cpp defines malloc-backed stubs
so no Core object is pulled in.
"""
import csv
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
TYPES = REPO / "resources" / "docs" / "types"
SOURCE = REPO / "source" / "Development" / "Src"
TESTS = REPO / "source" / "Tests" / "LayoutProbe"
TYPE_NAME_RE = re.compile(r"^[UAF][A-Z]\w*$")
IDENT_RE = re.compile(r"^[A-Za-z_]\w*$")
CONTRACT = ["FName", "FNameEntry", "FString", "FArchive", "UObject", "UField", "UStruct", "UState", "UClass", "UFunction",
            "UProperty", "UByteProperty", "UIntProperty", "UBoolProperty", "UFloatProperty", "UObjectProperty", "UClassProperty",
            "UNameProperty", "UStrProperty", "UArrayProperty", "UMapProperty", "UStructProperty", "UDelegateProperty",
            "UInterfaceProperty", "UComponentProperty", "UEnum", "UScriptStruct", "UConst", "FPackageFileSummary", "FObjectExport",
            "FObjectImport", "FGenerationInfo", "FCompressedChunk", "ULinkerLoad", "ULinker", "FFrame", "FStateFrame", "UPackage",
            # Engine (PHASE3.md package M)
            "AActor", "APawn", "AController", "APlayerController", "AWorldInfo", "UWorld", "ULevel", "UActorComponent",
            "UPrimitiveComponent", "UMeshComponent", "UStaticMeshComponent", "USkeletalMeshComponent", "UStaticMesh", "USkeletalMesh",
            "UTexture", "UTexture2D", "UMaterial", "UMaterialInstance", "UMaterialInstanceConstant", "UAnimSequence", "UAnimSet",
            "UPhysicsAsset", "UEngine", "UGameEngine", "UPlayer", "ULocalPlayer", "UNetDriver", "UNetConnection", "FPackageInfo",
            "USequence", "USequenceOp", "UInterpData"]
UNDECLARED_RE = re.compile(r"probe_(\w+?)(?:_\d+)?\.cpp\(\d+\): error C(?:2065|2061|2027|3861): (?:'([A-Za-z_]\w*)': undeclared identifier|syntax error: identifier '([A-Za-z_]\w*)'|use of undefined type '(?:class |struct )?([A-Za-z_]\w*)')")


def declared_types(module_dir: Path) -> set[str]:
    found = set()
    for p in list((module_dir / "Inc").rglob("*.h")) + list((module_dir / "Src").rglob("*.h")):
        # preprocessor lines are dropped so `class UObject #if ... : public UObjectBase #endif {` still matches
        text = "\n".join(l for l in p.read_text(encoding="utf-8", errors="replace").splitlines() if not l.lstrip().startswith("#"))
        for m in re.finditer(r"\b(?:class|struct)\s+(?:[A-Z_]+\s+)?([UAF][A-Z]\w*)\s*(?::[^{;]*)?\{", text):
            found.add(m.group(1))
    return found


def load_types() -> dict:
    with (TYPES / "types.json").open(encoding="utf-8") as f:
        return {t["name"]: t for t in json.load(f)["types"]}


def skip_file(module: str) -> Path:
    return TYPES / f"probe_skip_{module}.txt"


def load_skips(module: str) -> set[str]:
    p = skip_file(module)
    return {l.strip() for l in p.read_text(encoding="utf-8").splitlines() if l.strip() and not l.startswith("#")} if p.exists() else set()


def probe_members(t: dict) -> list[dict]:
    out = []
    for m in t["members"]:
        if m["is_base"] or m["is_vftable"] or m.get("bits") is not None or not IDENT_RE.match(m["name"] or ""):
            continue
        if "&" in m["type"] or m["name"] in ("__vftable", "__vecDelDtor") or m["name"].startswith("___u"):
            continue  # ___uN is DIA's name for an anonymous union/struct; it has no addressable name
        out.append(m)
    return out


# Headers a probe unit includes: the module's main header plus the generated class headers it does
# not reach (Engine.h stops at EngineGameEngineClasses.h; the rest are included per .cpp), in the
# dependency order Engine's own sources use (UnInterpolation.cpp, UnSequence.cpp).
MODULE_INCLUDES = {
    "Engine": ["Engine.h", "EngineSequenceClasses.h", "EngineInterpolationClasses.h", "EngineAnimClasses.h", "EngineParticleClasses.h",
               "EngineMaterialClasses.h", "EngineAudioDeviceClasses.h", "EngineMeshClasses.h", "EngineSoundClasses.h", "EngineAIClasses.h",
               "EngineDecalClasses.h", "EnginePhysicsClasses.h", "EngineUserInterfaceClasses.h", "EngineUIPrivateClasses.h",
               "EngineForceFieldClasses.h", "EnginePlatformInterfaceClasses.h", "EngineFogVolumeClasses.h", "EngineFluidClasses.h",
               "EngineProcBuildingClasses.h", "EngineFoliageClasses.h", "EngineSplineClasses.h", "EngineSpeedTreeClasses.h",
               "EnginePrefabClasses.h", "EngineLensFlareClasses.h"],
}
CHUNK = 100  # types per probe compile unit; Engine declares ~1,900, cl.exe stops a TU at 100 errors, and Engine.h parses in seconds


def probe_unit(module: str, types: dict, names: list[str], unit: str) -> str:
    lines = [f"// Generated by resources/tools/symbols/gen_layout_probe.py for module {module}",
             "#define _ALLOW_KEYWORD_MACROS 1  // MSVC STL xkeycheck: allow macroizing private/protected in this probe only",
             "#undef DISHONORED_LAYOUT_CHECKS", "#define DISHONORED_LAYOUT_CHECKS 0  // the probe measures; the asserts must not stop it while a module converges",
             "#define DISHONORED_SHIM_STATIC static  // shims of reference-only members stay declarations here: their ctors/dtors would need the module libs",
             "#define private public", "#define protected public",
             *[f"#include \"{h}\"" for h in MODULE_INCLUDES.get(module, [f"{module}.h"])], "#undef private", "#undef protected",
             "#include <cstdio>", "#include <cstddef>", "#include <type_traits>",
             "#define OFF(T, m) (reinterpret_cast<size_t>(&reinterpret_cast<T*>(0x10000)->m) - 0x10000)",
             "namespace {"]
    detectors = []
    body = []
    calls = []
    for n in names:
        # One function template per type: `if constexpr` only discards the untaken branch inside
        # a template, and an access failure inside decltype is a substitution failure, so
        # default-private members of `class` types report MISSING instead of breaking the build.
        fn = [f"template<class U> void probe_{n}()", "{", f"    std::printf(\"{n},%zu\\n\", sizeof(U));"]
        for m in probe_members(types[n]):
            tag = f"{n}_{m['name']}"
            detectors.append(f"template<class U, class = void> struct Has_{tag} : std::false_type {{}};")
            detectors.append(f"template<class U> struct Has_{tag}<U, std::void_t<decltype(&U::{m['name']})>> : std::true_type {{}};")
            fn.append(f"    if constexpr (Has_{tag}<U>::value) std::printf(\"{n}.{m['name']},%zu\\n\", OFF(U, {m['name']})); else std::printf(\"{n}.{m['name']},MISSING\\n\");")
        fn += ["}", ""]
        body += fn
        calls.append(f"    probe_{n}<{n}>();")
    lines += detectors + body + ["}", "", f"void {unit}()", "{"] + calls + ["}", ""]
    return "\n".join(lines)


def generate(modules: list[str]) -> None:
    types = load_types()
    TESTS.mkdir(parents=True, exist_ok=True)
    for stale in TESTS.glob("probe_*.cpp"):
        stale.unlink()
    all_names = []
    units = []
    for module in modules:
        skips = load_skips(module)
        names = sorted(n for n in declared_types(SOURCE / module) if n in types and TYPE_NAME_RE.match(n) and n not in skips)
        all_names.append((module, names))
        chunks = [names[i:i + CHUNK] for i in range(0, len(names), CHUNK)] or [[]]
        for i, chunk in enumerate(chunks):
            unit = f"probe_{module}" if len(chunks) == 1 else f"probe_{module}_{i}"
            (TESTS / f"{unit}.cpp").write_text(probe_unit(module, types, chunk, unit), encoding="utf-8")
            units.append(unit)
        print(f"{module}: {len(names)} types ({len(skips)} skipped) in {len(chunks)} unit(s) -> {TESTS / f'probe_{module}*.cpp'}")
    main_lines = ["// Generated by resources/tools/symbols/gen_layout_probe.py", "#include <cstdio>", "#include <cstdlib>",
                  "// Stubs so the linker does not pull Core.lib (and its Engine dependencies) into the probe; the",
                  "// probe only evaluates sizeof/offsetof and these are the only module symbols its objects reference.",
                  "void* __cdecl appMalloc(unsigned long Count, unsigned long) { return std::malloc(Count); }",
                  "void __cdecl appFree(void* Ptr) { std::free(Ptr); }",
                  "// Engine.h inline code (checks in TArray accessors, RHI wrappers) references these three as well.",
                  "void __cdecl appFailAssertFunc(const char*, const char*, int, const wchar_t*, ...) { std::abort(); }",
                  "void __cdecl appFailAssertFuncDebug(const char*, const char*, int, const wchar_t*, ...) { std::abort(); }",
                  "class FDynamicRHI* GDynamicRHI = nullptr;"]
    main_lines += [f"void {u}();" for u in units]
    main_lines += ["int main()", "{"] + [f"    {u}();" for u in units] + ["    return 0;", "}", ""]
    (TESTS / "probe_main.cpp").write_text("\n".join(main_lines), encoding="utf-8")
    cmake = ["# Generated by resources/tools/symbols/gen_layout_probe.py",
             "add_executable(LayoutProbe probe_main.cpp " + " ".join(f"{u}.cpp" for u in units) + ")",
             "# Take the modules' public include paths and defines without linking (or even building) their",
             "# libraries: the probe only evaluates sizeof/offsetof and must stay buildable while a module's",
             "# sources are red. ($<COMPILE_ONLY:Core> would still add Core as a build dependency.)"]
    for m, _ in all_names:
        cmake += [f"target_include_directories(LayoutProbe PRIVATE $<TARGET_PROPERTY:{m},INTERFACE_INCLUDE_DIRECTORIES>)",
                  f"target_compile_definitions(LayoutProbe PRIVATE $<TARGET_PROPERTY:{m},INTERFACE_COMPILE_DEFINITIONS>)"]
    cmake += [
             "# Engine headers include png.h; pnglibconf.h is generated by the libpng build, so depend on the third-party targets (not on Core/Engine).",
             "add_dependencies(LayoutProbe png_static zlibstatic)",
             "dishonored_apply_defines(LayoutProbe)",
             "target_compile_options(LayoutProbe PRIVATE ${DISHONORED_MSVC_WARNINGS} /wd4589 /bigobj)",
             "set_target_properties(LayoutProbe PROPERTIES FOLDER \"Tests\")",
             "add_custom_command(TARGET LayoutProbe POST_BUILD COMMAND $<TARGET_FILE:LayoutProbe> > \"${CMAKE_BINARY_DIR}/layout_probe.txt\" COMMENT \"layout probe -> layout_probe.txt\")", ""]
    (TESTS / "CMakeLists.txt").write_text("\n".join(cmake), encoding="utf-8")


def skip_from_log(module: str, log: Path) -> None:
    text = log.read_text(encoding="utf-8", errors="replace")
    found = set()
    for m in UNDECLARED_RE.finditer(text):
        if m.group(1) == module:
            found.add(m.group(2) or m.group(3) or m.group(4))
    skips = load_skips(module)
    new = sorted(n for n in found if TYPE_NAME_RE.match(n) and n not in skips)
    p = skip_file(module)
    with p.open("a", encoding="utf-8") as f:
        if not skips:
            f.write(f"# Types the {module} module's main header does not declare (collected from probe build logs)\n")
        f.write("".join(n + "\n" for n in new))
    print(f"{module}: {len(new)} new skips (total {len(skips) + len(new)}) -> {p.relative_to(REPO)}")


RETAIL_SIZES = TYPES / "native_class_sizes.csv"


def retail_sizes() -> dict[str, int]:
    """Retail (2013) sizeof per native class from agent H's descriptor export; the target the headers converge on."""
    if not RETAIL_SIZES.exists():
        return {}
    with RETAIL_SIZES.open(newline="", encoding="utf-8") as f:
        return {r["class"]: int(r["size_2013"]) for r in csv.DictReader(f) if r.get("size_2013")}


def compare(probe_output: Path) -> int:
    types = load_types()
    retail = retail_sizes()
    sizes = {}
    offsets = {}
    missing_members = set()
    for line in probe_output.read_text(encoding="utf-8", errors="replace").splitlines():
        if "," not in line:
            continue
        key, val = line.rsplit(",", 1)
        if "." in key:
            t, m = key.split(".", 1)
            if val.strip() == "MISSING":
                missing_members.add((t, m))
            else:
                offsets[(t, m)] = int(val)
        else:
            sizes[key] = int(val)
    rows = []
    for name in sorted(sizes):
        pdb = types[name]
        target = retail.get(name, pdb["size"])
        bad = []
        missing = []
        # member offsets are only known from the 2012 PDB; where retail changed the size the 2012 offsets no longer apply
        for m in (probe_members(pdb) if target == pdb["size"] else []):
            got = offsets.get((name, m["name"]))
            if got is None:
                missing.append(m["name"])
            elif got != m["offset"]:
                bad.append((m["name"], m["offset"], got))
        rows.append((name, target, sizes[name], target == sizes[name], bad, missing))
    contract = [r for r in rows if r[0] in CONTRACT]
    contract_bad = [r for r in contract if not r[3] or r[4] or r[5]]
    exact = sum(1 for r in rows if r[3] and not r[4] and not r[5])
    out = TYPES / "reference_layout_delta.md"
    with out.open("w", encoding="utf-8") as f:
        f.write("# Compiled layout delta: ported headers vs the target layout\n\n")
        f.write("Target size = retail 2013 sizeof (`native_class_sizes.csv`) where the class exists in the retail exe, else the 2012 Shipping PDB size. Member offsets come from the 2012 PDB and are only checked where the retail size equals the 2012 size.\n\n")
        f.write(f"Generated by `gen_layout_probe.py compare` from `{probe_output.name}`. {len(rows)} types probed; {exact} match exactly (size and every member offset).\n\n")
        f.write(f"## Contract types ({len(contract)} probed, {len(contract_bad)} mismatching)\n\n| Type | Target size | Ours | Size ok | Offset mismatches (member: pdb→ours) | Missing members (Arkane additions) |\n|---|---:|---:|---|---|---|\n")
        for name, ps, os_, ok, bad, missing in contract:
            f.write(f"| {name} | {ps} | {os_} | {'yes' if ok else '**no**'} | {', '.join(f'{m}: {p}→{g}' for m, p, g in bad[:8])} | {', '.join(missing[:8])} |\n")
        f.write("\n## All other probed types with differences\n\n| Type | Target size | Ours | First offset mismatch | Missing members |\n|---|---:|---:|---|---|\n")
        for name, ps, os_, ok, bad, missing in rows:
            if name in CONTRACT or (ok and not bad and not missing):
                continue
            first = f"{bad[0][0]}: {bad[0][1]}→{bad[0][2]}" if bad else ""
            f.write(f"| {name} | {ps} | {os_} | {first} | {', '.join(missing[:4])} |\n")
    print(f"probed={len(rows)} exact={exact} contract_mismatches={len(contract_bad)} -> {out}")
    return 1 if contract_bad else 0


def show(names: list[str]) -> None:
    types = load_types()
    for n in names:
        t = types.get(n)
        if t is None:
            print(f"{n}: not in types.json")
            continue
        print(f"{t['kind']} {n}: size {t['size']} align {t['align']} bases {t['bases']} vftable {t['has_vftable']}")
        for m in t["members"]:
            bits = f" :{m['bits']}@{m['bit_offset']}" if m.get("bits") is not None else ""
            size = m['size'] if m.get('size') is not None else 0
            print(f"  {m['offset']:5d} {size:5d}  {m['type']}  {m['name']}{bits}{'  [base]' if m['is_base'] else ''}")
        print()


PROPS_BLOCK_RE = re.compile(r"^([ \t]*)//## BEGIN PROPS (\w+)[ \t]*\r?\n(.*?)^[ \t]*//## END PROPS \2[ \t]*$", re.M | re.S)
PROPS_MEMBER_RE = re.compile(r"^\s*(?:BITFIELD\s+)?[^;{}/]*?\b(\w+)\s*((?:\[[^\]]*\])*)\s*(?::\s*\d+)?\s*;")
ACCESS_RE = re.compile(r"^\s*(public|private|protected):\s*$")


def parse_props_block(body: str) -> dict[str, tuple[str, str]]:
    """member name -> (declaration line, access) of a reference //## BEGIN PROPS block; preprocessor
    guards are dropped (the shipping build keeps WITH_EDITORONLY_DATA members, see types.json)."""
    out = {}
    access = "public"
    for line in body.splitlines():
        if line.lstrip().startswith("#"):
            continue
        m = ACCESS_RE.match(line)
        if m:
            access = m.group(1)
            continue
        m = PROPS_MEMBER_RE.match(line)
        if m and m.group(1) != "SCRIPT_ALIGN":
            out[m.group(1)] = (line.strip(), access)
    return out


def props_lines(name: str, t: dict, ref: dict[str, tuple[str, str]], indent: str) -> tuple[list[str], list[str], list[str]]:
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from gen_classes_header import declarator, fix_type
    lines = []
    added = []
    access = "public"
    pdb_names = set()
    byte_run = False
    for m in t["members"]:
        if m["is_base"] or m["is_vftable"]:
            continue
        pdb_names.add(m["name"])
        # the script compiler closes every run of BYTE properties with SCRIPT_ALIGN (BITFIELD:0)
        is_byte = m.get("bits") is None and m["size"] == 1
        if byte_run and not is_byte:
            lines.append(indent + "SCRIPT_ALIGN;")
        byte_run = is_byte
        text, want = ref.get(m["name"], (None, "public"))
        if text is None:
            if m.get("bits") is not None:
                text = f"BITFIELD {m['name']}:{m['bits']};"
            else:
                text = f"{declarator(fix_type(m['type']), m['name'])};".replace("struct class ", "class ")
            text += f"  // DISHONORED(layout): 2012 PDB @{m['offset']}"
            added.append(m["name"])
        if want != access:
            lines.append(f"{want}:")
            access = want
        lines.append(indent + text)
    if byte_run:
        lines.append(indent + "SCRIPT_ALIGN;")
    if access != "public":
        lines.append("public:")
    removed = [n for n in ref if n not in pdb_names]
    return lines, added, removed


SHIM_COMMENT = ["// DISHONORED(layout): reference-only members absent from the 2012 PDB. Kept as storage-less C++17",
                "// inline statics (DISHONORED_SHIM_STATIC, Engine.h) so unported reference code still compiles; they are not part of the object layout",
                "// and the module port has to remove their uses (resources/docs/agents/agentM.md lists them)."]


def shim_line(decl: str) -> str:
    decl = decl.split("//")[0].strip()
    m = re.match(r"^BITFIELD\s+(\w+)\s*:\s*\d+\s*;$", decl)
    if m:
        return f"DISHONORED_SHIM_STATIC BITFIELD {m.group(1)};"  # UBOOL (UINT) makes NEQ() ambiguous in replication code
    decl = re.sub(r"^mutable\s+", "", decl)
    return "DISHONORED_SHIM_STATIC " + decl


def shim_lines(removed: list[str], ref: dict[str, tuple[str, str]], indent: str) -> list[str]:
    if not removed:
        return []
    return [indent + c for c in SHIM_COMMENT] + [indent + shim_line(ref[n][0]) for n in removed]


def props(module: str, names: list[str]) -> int:
    """Rewrite the //## BEGIN PROPS block of each script class from the PDB member list: reference
    declaration lines are kept (by name, with their access specifier), Arkane members are
    synthesized from types.json, reference-only members become inline-static shims after the block."""
    from gen_classes_header import uc_stem
    types = load_types()
    headers = {p: p.read_text(encoding="utf-8") for p in (SOURCE / module / "Inc").glob("*Classes.h")}
    rc = 0
    for name in names:
        t = types.get(name)
        stem = uc_stem(name)
        hit = None
        for p, text in headers.items():
            for m in PROPS_BLOCK_RE.finditer(text):
                if m.group(2) == stem:
                    hit = (p, m)
        if t is None or hit is None:
            print(f"{name}: {'not in types.json' if t is None else f'no //## BEGIN PROPS {stem} block in {module}/Inc/*Classes.h'}")
            rc = 1
            continue
        p, m = hit
        indent = m.group(1)
        ref = parse_props_block(m.group(3))
        lines, added, removed = props_lines(name, t, ref, indent)
        if not added and not removed and [l.strip() for l in lines if not ACCESS_RE.match(l)] == [v[0] for v in ref.values()]:
            print(f"{name}: {p.name}: already matches the PDB member list, left untouched")
            continue
        header = [f"{indent}//## BEGIN PROPS {stem}",
                  f"{indent}// DISHONORED(layout): 2012 PDB size {t['size']}; member list and order regenerated from types.json"
                  f" (gen_layout_probe.py props{'; reference-only members moved to the shim block below: ' + ', '.join(removed) if removed else ''})"]
        block = "\n".join(header + lines + [f"{indent}//## END PROPS {stem}"] + shim_lines(removed, ref, indent))
        text = headers[p]
        headers[p] = text[:m.start()] + block + text[m.end():]
        print(f"{name}: {p.name}: {len(lines)} lines, +{len(added)} Arkane ({', '.join(added)}), -{len(removed)} reference-only ({', '.join(removed)})")
    for p, text in headers.items():
        if text != p.read_text(encoding="utf-8"):
            write_retry(p, text)
    return rc


def write_retry(p: Path, text: str) -> None:
    """Headers are transiently locked by concurrent compiles (PermissionError); retry for a while."""
    import time
    for attempt in range(30):
        try:
            p.write_text(text, encoding="utf-8")
            return
        except PermissionError:
            time.sleep(2)
    p.write_text(text, encoding="utf-8")


PDB_2012 = REPO.parent / "Dishonored_Debug2012" / "Binaries" / "Win32" / "DishonoredGame-Shipping.pdb"
SYM_TAG_DATA, SYM_TAG_UDT, SYM_TAG_ENUM, SYM_TAG_FUNCTION_TYPE, SYM_TAG_POINTER, SYM_TAG_ARRAY, SYM_TAG_BASE_TYPE, SYM_TAG_TYPEDEF, SYM_TAG_BASE_CLASS = 7, 11, 12, 13, 14, 15, 16, 17, 18
DATA_IS_MEMBER, LOC_IS_BITFIELD = 7, 6
BASE_TYPE_NAMES = {1: "void", 2: "char", 3: "wchar_t", 6: {1: "__int8", 2: "__int16", 4: "int", 8: "__int64"},
                   7: {1: "unsigned __int8", 2: "unsigned __int16", 4: "unsigned int", 8: "unsigned __int64"},
                   8: {4: "float", 8: "double"}, 10: "bool", 13: "int", 14: "unsigned int", 25: "char"}


def dia_type_name(t) -> str:
    tag = t.symTag
    if tag == SYM_TAG_BASE_TYPE:
        n = BASE_TYPE_NAMES.get(t.baseType, f"bt{t.baseType}")
        return n if isinstance(n, str) else n.get(t.length, f"{n[4]}{t.length * 8}")
    if tag == SYM_TAG_POINTER:
        return dia_type_name(t.type) + (" &" if t.reference else " *")
    if tag == SYM_TAG_ARRAY:
        inner = t.type
        count = t.length // inner.length if inner.length else 0
        return f"{dia_type_name(inner)}[{count}]"
    if tag in (SYM_TAG_UDT, SYM_TAG_ENUM, SYM_TAG_TYPEDEF):
        return t.name or "<unnamed>"
    if tag == SYM_TAG_FUNCTION_TYPE:
        return "fn"
    return f"tag{tag}"


def dia_members(udt) -> list[dict]:
    """Data members of one DIA UDT symbol (no bases, no statics), in the PDB's own layout."""
    out = []
    kids = udt.findChildren(SYM_TAG_DATA, None, 0)
    for i in range(kids.Count):
        m = kids.Item(i)
        if m.dataKind != DATA_IS_MEMBER:
            continue
        bits = m.locationType == LOC_IS_BITFIELD
        # DIA gives a bitfield's storage-unit offset plus a bit position up to 31; types.json (IDA)
        # uses byte offset plus bit within the byte, so normalize to that
        out.append({"name": m.name or "", "offset": m.offset + (m.bitPosition // 8 if bits else 0),
                    "bit_offset": m.bitPosition % 8 if bits else None, "size": None if bits else m.type.length,
                    "bits": m.length if bits else None, "type": dia_type_name(m.type), "is_base": False, "is_vftable": False})
    return out


def dia_udts(names: set[str]) -> dict:
    """name -> the defining DIA UDT symbol (the one with a length and the most children) for the
    requested names; every UDT in the PDB is visited once, which takes about a minute."""
    sys.path.insert(0, str(REPO / "resources" / "tools" / "pdb"))
    from dia_dump import load_dia
    source = load_dia()
    source.loadDataFromPdb(str(PDB_2012))
    session = source.openSession()
    best = {}
    all_udts = session.globalScope.findChildren(SYM_TAG_UDT, None, 0)
    for i in range(all_udts.Count):
        u = all_udts.Item(i)
        n = u.name
        if n not in names or not u.length:
            continue
        cur = best.get(n)
        if cur is None or u.findChildren(SYM_TAG_DATA, None, 0).Count > cur.findChildren(SYM_TAG_DATA, None, 0).Count:
            best[n] = u
    return best


def pdb_fix(names: list[str]) -> int:
    """MSVC lays derived-class members out inside an over-aligned base's tail padding (a class
    ending in FMatrix is 16-aligned; UMeshComponent::Materials sits at 452 inside sizeof
    (UPrimitiveComponent) == 464). IDA's PDB import drops or truncates such members (they overlap
    the base-class pseudo member: AWorldInfo::m_SunMeshesAndMaterials became _BYTE[4] @608), so
    types.json (exported from IDA) is wrong for them. This re-reads every requested UDT with DIA
    and replaces its data members with the PDB's own list; bases and vftables stay as IDA had them.
    The JSON is rewritten in place."""
    with (TYPES / "types.json").open(encoding="utf-8") as f:
        data = json.load(f)
    wanted = set(names) if names else {t["name"] for t in data["types"] if TYPE_NAME_RE.match(t["name"])}
    udts = dia_udts(wanted)
    fixed = 0
    log = []
    for t in data["types"]:
        u = udts.get(t["name"])
        if u is None or t["name"] not in wanted:
            continue
        if u.length != t["size"]:
            log.append(f"{t['name']}: PDB size {u.length} != types.json {t['size']}, not touched")
            continue
        old = [m for m in t["members"] if not m["is_base"] and not m["is_vftable"]]
        new = [m for m in dia_members(u) if m["name"] and IDENT_RE.match(m["name"])]
        key = lambda m: (m["name"], m["offset"], m["size"], m["bits"])
        if [key(m) for m in old if m["name"] and not m["name"].startswith("___u")] == [key(m) for m in new]:
            continue
        keep = [m for m in t["members"] if m["is_base"] or m["is_vftable"] or (m["name"] or "").startswith("___u")]
        members = keep + new
        members.sort(key=lambda m: (m["offset"], 0 if m["is_base"] else 1, m.get("bit_offset") or 0))
        old_names = {m["name"] for m in old}
        added = [m for m in new if m["name"] not in old_names]
        changed = [m for m in new if m["name"] in old_names and key(m) not in {key(o) for o in old}]
        dropped = [m for m in old if m["name"] and not m["name"].startswith("___u") and m["name"] not in {n["name"] for n in new}]
        t["members"] = members
        fixed += 1
        log.append(f"{t['name']}: +{len(added)} added ({', '.join(f'{m['name']}@{m['offset']}' for m in added)})"
                   f" ~{len(changed)} changed ({', '.join(f'{m['name']}@{m['offset']}' for m in changed)})"
                   f" -{len(dropped)} dropped ({', '.join(m['name'] for m in dropped)})")
    if fixed:
        with (TYPES / "types.json").open("w", encoding="utf-8") as f:
            json.dump(data, f, indent=1)
    print("\n".join(log))
    print(f"{fixed} types fixed in {TYPES / 'types.json'}")
    return 0


def structs(names: list[str]) -> None:
    """Print C++ declarations (gen_classes_header style) for PDB structs a converged header needs,
    recursing into undeclared F* member types first."""
    from gen_classes_header import emit_struct
    types = load_types()
    declared = set()
    for d in ("Core", "Engine"):
        for p in (SOURCE / d / "Inc").rglob("*.h"):
            declared |= set(re.findall(r"\b(?:class|struct|enum)\s+(?:[A-Z_]+\s+)?(\w+)\s*(?::[^{;]*)?\{", p.read_text(encoding="utf-8", errors="replace")))
    done = []

    def visit(n: str) -> None:
        if n in done or n in declared or n not in types:
            return
        for m in types[n]["members"]:
            for ident in re.findall(r"\bF[A-Z]\w*", m["type"]):
                visit(ident)
        done.append(n)

    for n in names:
        visit(n)
    for n in done:
        print("\n".join(emit_struct(n, types[n])))


def main(argv: list[str]) -> int:
    if len(argv) < 2 or argv[1] not in ("generate", "compare", "skip-from-log", "show", "props", "pdb-fix", "structs"):
        print(__doc__)
        return 2
    if argv[1] == "show":
        show(argv[2:])
        return 0
    if argv[1] == "props":
        return props(argv[2], argv[3:])
    if argv[1] == "pdb-fix":
        return pdb_fix(argv[2:])
    if argv[1] == "structs":
        structs(argv[2:])
        return 0
    if argv[1] == "generate":
        generate(argv[2:] or ["Core"])
        return 0
    if argv[1] == "skip-from-log":
        skip_from_log(argv[2], Path(argv[3]))
        return 0
    return compare(Path(argv[2]))


if __name__ == "__main__":
    sys.exit(main(sys.argv))
