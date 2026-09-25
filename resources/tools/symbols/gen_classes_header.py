"""P2.7: generate a <Module>Classes.h-style header from the PDB for the UObject classes of a module.

The reference engine generates these headers from UnrealScript with its script compiler; we do not
have Dishonored's .uc for the engine and only decompiled ones for DishonoredGame, so the class
layouts come from the PDB types (exact), the native function list from natives.csv, the class list
from the PrivateStaticClass globals / InternalConstructor functions and the module attribution
from functions.csv (majority module of the class's member functions).

Script-declared classes (those with a DFSDK .uc or a declaration in a reference *Classes.h) are
emitted first in the reference layout (enums block, script structs, classes with BEGIN/END PROPS,
DECLARE_FUNCTION list, AUTOGENERATE_FUNCTION list, AUTO_INITIALIZE_REGISTRANTS, MAP_NATIVE
tables, VERIFY_CLASS_SIZES); native-only classes follow in a second section for reference.

Usage:
  python resources/tools/symbols/gen_classes_header.py <Module> [--out path]
      [--diff-reference] [--inventory path] [--reference-src dir] [--dfsdk dir]

Default output: resources/reference/<Module>Classes.pdb.h (gitignored; copied into the module by hand
once reviewed). The header is a starting point for hand editing, not a drop-in.
--diff-reference compares the script-declared classes against the reference <Module>*Classes.h.
--inventory writes the per-class cross-reference against the DFSDK .uc as markdown.
"""
import argparse
import csv
import json
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
DOCS = REPO / "resources" / "docs"
SYM = DOCS / "symbols"
TYPES = DOCS / "types"
REFERENCE_SRC = REPO.parent / "UnrealEngine3" / "Development" / "Src"
DFSDK_CLASSES = Path("D:/DishonoredMapMaking/DishonoredEditor/Development/Src/DishonoredGame/Classes")

UE_TYPEDEFS = {"INT", "UINT", "BYTE", "SBYTE", "WORD", "SWORD", "DWORD", "QWORD", "SQWORD", "FLOAT", "DOUBLE", "TCHAR", "UBOOL", "ANSICHAR", "UNICHAR", "BITFIELD", "PTRINT", "UPTRINT", "SIZE_T"}

TYPE_FIXES = [
    (re.compile(r"\bstruct (F\w+)"), r"\1"),
    (re.compile(r"\bclass (U\w+|A\w+|F\w+|T\w+|I\w+)"), r"\1"),
    (re.compile(r"\benum (\w+)"), r"\1"),
    (re.compile(r"\bunsigned __int64\b"), "QWORD"),
    (re.compile(r"\b__int64\b"), "SQWORD"),
    (re.compile(r"\bunsigned __int32\b"), "UINT"),
    (re.compile(r"\bunsigned __int16\b"), "WORD"),
    (re.compile(r"\bunsigned __int8\b"), "BYTE"),
    (re.compile(r"\b__int32\b"), "INT"),
    (re.compile(r"\b__int16\b"), "SWORD"),
    (re.compile(r"\b__int8\b"), "SBYTE"),
    (re.compile(r"\bunsigned int\b"), "UINT"),
    (re.compile(r"\bunsigned char\b"), "BYTE"),
    (re.compile(r"\bunsigned short\b"), "WORD"),
    (re.compile(r"\bunsigned long\b"), "DWORD"),
    (re.compile(r"\bsigned char\b"), "SBYTE"),
    (re.compile(r"\bint\b"), "INT"),
    (re.compile(r"\bshort\b"), "SWORD"),
    (re.compile(r"\bfloat\b"), "FLOAT"),
    (re.compile(r"\bdouble\b"), "DOUBLE"),
    (re.compile(r"\bwchar_t\b"), "TCHAR"),
    (re.compile(r"\bchar\b"), "ANSICHAR"),
    (re.compile(r"\bconst\b\s*"), ""),
    (re.compile(r",\s*FDefaultAllocator\b"), ""),
    (re.compile(r",\s*FDefaultSetAllocator\b"), ""),
    (re.compile(r"\s*\*"), "*"),
    (re.compile(r",(?! )"), ","),
]
CLASS_PTR_RE = re.compile(r"\b([UAI][A-Z]\w*)\*")
ARRAY_SUFFIX_RE = re.compile(r"^(.*?)((?:\[\d+\])+)$")
FUNCTION_PTR_RE = re.compile(r"^(.*)\((__\w+)\*\)(\(.*\))$")
IDENT_RE = re.compile(r"[A-Za-z_]\w*")


def fix_type(t: str) -> str:
    for rx, rep in TYPE_FIXES:
        t = rx.sub(rep, t)
    t = CLASS_PTR_RE.sub(lambda m: m.group(0) if m.group(1) in UE_TYPEDEFS else f"class {m.group(1)}*", t)
    return re.sub(r"\*(?=\w)", "* ", t).strip()


def declarator(t: str, name: str) -> str:
    m = FUNCTION_PTR_RE.match(t)
    if m:
        return f"{m.group(1).strip()} ({m.group(2)}* {name}){m.group(3)}"
    ty, arr = split_array(t)
    return f"{ty} {name}{arr}"


def split_array(t: str) -> tuple[str, str]:
    m = ARRAY_SUFFIX_RE.match(t)
    return (m.group(1), m.group(2)) if m else (t, "")


def uc_stem(cls: str) -> str:
    return cls[1:] if cls[:1] in "UAI" and len(cls) > 1 and cls[1].isupper() else cls


# ----------------------------------------------------------------------------------------------
# Symbol data


@dataclass
class Symbols:
    types: dict
    enums: dict
    class_module: dict[str, str]
    abstract: set[str]
    natives: dict[str, list[dict]]
    staticclass_rva: dict[str, str]
    member_function_modules: dict[str, Counter]


MEMBER_FN_RE = re.compile(r"(?:^|\s)(\w+)::(~?\w+)\(")
PRIVATE_STATIC_CLASS_RE = re.compile(r"^\?PrivateStaticClass@(\w+)@@")
EXEC_NAME_RE = re.compile(r"^(\w+)::exec(\w+)$")
IMPLEMENT_FUNCTION_INIT_RE = re.compile(r"^`dynamic initializer for '(\w+?)exec(\w+)Temp''$")


def add_folded_natives(natives: dict[str, list[dict]], by_rva: dict[str, dict]) -> None:
    """natives.csv keeps IDA's one name per address; identical COMDAT folding merged byte-identical
    exec thunks of different classes. pdb_functions.csv (all DIA names per RVA, uncommitted) restores
    the folded (class, func) pairs, and the IMPLEMENT_FUNCTION dynamic initializers list every
    registered pair even when the thunk has no name of its own at that address."""
    path = SYM / "pdb_functions.csv"
    if not path.exists():
        return
    exec_rva: dict[tuple[str, str], str] = {}
    registered: set[tuple[str, str]] = set()
    with path.open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            m = EXEC_NAME_RE.match(r["name"])
            if m:
                exec_rva[(m.group(1), m.group(2))] = r["rva"]
                continue
            m = IMPLEMENT_FUNCTION_INIT_RE.match(r["name"])
            if m:
                registered.add((m.group(1), m.group(2)))
    known = {(c, n["func"]) for c, lst in natives.items() for n in lst}
    for cls, func in sorted(registered | set(exec_rva)):
        if (cls, func) in known:
            continue
        rva = exec_rva.get((cls, func), "")
        folded = by_rva.get(rva)
        natives.setdefault(cls, []).append({
            "class": cls, "func": func, "rva": rva or "0x0", "size": folded["size"] if folded else "",
            "native_index": folded["native_index"] if folded else "-1",
            "folded_into": f"{folded['class']}::exec{folded['func']}" if folded else "",
            "registered": (cls, func) in registered,
        })


def load_symbols() -> Symbols:
    with (TYPES / "types.json").open(encoding="utf-8") as f:
        data = json.load(f)
    types = {t["name"]: t for t in data["types"]}
    enums = {e["name"]: e for e in data.get("enums", [])}

    module_of_rva: dict[str, str] = {}
    member_modules: dict[str, Counter] = {}
    abstract_candidates: set[str] = set()
    with (SYM / "functions.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            module_of_rva[r["rva"]] = r["module"]
            m = MEMBER_FN_RE.search(r["demangled"])
            if not m:
                continue
            cls, fn = m.group(1), m.group(2)
            if r["module"] not in ("", "crt", "unknown"):
                member_modules.setdefault(cls, Counter())[r["module"]] += 1
            if fn == "InternalConstructor":
                abstract_candidates.add(cls)

    classes: set[str] = set()
    with (SYM / "globals.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            m = PRIVATE_STATIC_CLASS_RE.match(r["name"])
            if m:
                classes.add(m.group(1))
    staticclass_rva: dict[str, str] = {}
    with (SYM / "classes.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            staticclass_rva[r["class"]] = r["staticclass_rva"]
            classes.add(r["class"])

    class_module: dict[str, str] = {}
    for c in classes:
        counts = member_modules.get(c)
        if counts:
            class_module[c] = counts.most_common(1)[0][0]
        else:
            class_module[c] = module_of_rva.get(staticclass_rva.get(c, ""), "")

    natives: dict[str, list[dict]] = {}
    by_rva: dict[str, dict] = {}
    with (SYM / "natives.csv").open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            natives.setdefault(r["class"], []).append(r)
            by_rva[r["rva"]] = r
    add_folded_natives(natives, by_rva)
    for lst in natives.values():
        lst.sort(key=lambda r: (int(r["rva"], 16), r["func"]))

    abstract = {c for c in classes if c not in abstract_candidates}
    return Symbols(types, enums, class_module, abstract, natives, staticclass_rva, member_modules)


# ----------------------------------------------------------------------------------------------
# Reference *Classes.h (UE3 10897) parsing


@dataclass
class RefModule:
    classes: dict[str, str] = field(default_factory=dict)
    structs: dict[str, str] = field(default_factory=dict)
    enums: dict[str, list[str]] = field(default_factory=dict)
    props: dict[str, list[str]] = field(default_factory=dict)
    natives: dict[str, list[str]] = field(default_factory=dict)


REF_DECL_RE = re.compile(r"^(class|struct) (\w+) : public (\w+)", re.M)
REF_ENUM_RE = re.compile(r"^enum (\w+)\s*\{(.*?)\};", re.M | re.S)
REF_PROPS_RE = re.compile(r"//## BEGIN PROPS (\w+)\n(.*?)//## END PROPS", re.S)
REF_MEMBER_RE = re.compile(r"^\s*(?:BITFIELD\s+)?[^;{}]*?\b(\w+)(?:\[[^\]]*\])*(?::\d+)?;", re.M)
REF_CLASS_BODY_RE = re.compile(r"^class (\w+) : public [^\n]*\n\{(.*?)^\};", re.M | re.S)
REF_DECLARE_FUNCTION_RE = re.compile(r"DECLARE_FUNCTION\((exec\w+)\)")


def load_reference(src: Path) -> dict[str, RefModule]:
    modules: dict[str, RefModule] = {}
    for header in sorted(src.glob("*/Inc/*Classes.h")):
        module = header.parts[-3].lower()
        ref = modules.setdefault(module, RefModule())
        text = header.read_text(encoding="utf-8", errors="replace")
        for kind, name, base in REF_DECL_RE.findall(text):
            (ref.classes if kind == "class" else ref.structs)[name] = base
        for name, body in REF_ENUM_RE.findall(text):
            ref.enums[name] = [v.split("=")[0].strip() for v in body.split(",") if v.strip()]
        for stem, body in REF_PROPS_RE.findall(text):
            members = [m for m in REF_MEMBER_RE.findall(body) if m != "SCRIPT_ALIGN"]
            ref.props[stem] = members
        for name, body in REF_CLASS_BODY_RE.findall(text):
            ref.natives[name] = REF_DECLARE_FUNCTION_RE.findall(body)
    return modules


# ----------------------------------------------------------------------------------------------
# DFSDK .uc parsing (UE Explorer output, hand-edited for UDK 2010; "udk" tagged comments hide
# retail declarations that UDK cannot compile, so they are treated as live)


@dataclass
class UcVar:
    name: str
    type: str
    dims: str
    modifiers: list[str]
    from_udk_comment: bool


@dataclass
class UcStruct:
    name: str
    extends: str
    vars: list[UcVar]


@dataclass
class UcClass:
    name: str
    extends: str
    is_native: bool
    modifiers: str
    is_interface: bool = False
    vars: list[UcVar] = field(default_factory=list)
    enums: dict[str, list[str]] = field(default_factory=dict)
    structs: dict[str, UcStruct] = field(default_factory=dict)
    natives: list[str] = field(default_factory=list)
    functions: list[str] = field(default_factory=list)


UDK_LINE_COMMENT_RE = re.compile(r"^(\s*)//(.*?)\s*udk\s*$", re.M | re.I)
UDK_BLOCK_COMMENT_RE = re.compile(r"/\*(?!\*)((?:(?!\*/).)*?)\s*udk\s*\*/", re.S | re.I)
LINE_COMMENT_RE = re.compile(r"//[^\n]*")
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.S)
UDK_MARK = "\x01UDK\x01"
CLASS_DECL_RE = re.compile(r"^\s*(class|interface)\s+(\w+)(?:\s+extends\s+([\w.]+))?(.*)$", re.S)
FUNC_DECL_RE = re.compile(r"\b(function|event|delegate|operator|preoperator|postoperator)\b(?:\s*\(\s*\d+\s*\))?\s+(?:[\w<>.]+\s+)?(\w+)\s*\(")
STRUCT_DECL_RE = re.compile(r"^\s*struct\b(.*?)\b(\w+)(?:\s+extends\s+([\w.]+))?\s*$", re.S)
ENUM_DECL_RE = re.compile(r"^\s*enum\s+(\w+)\s*$")
VAR_NAME_RE = re.compile(r"^(\w+)\s*((?:\[[^\]]*\])*)$")
TEMPLATE_SPACE_RE = re.compile(r"<[^<>]*>")


def strip_comments(text: str) -> str:
    text = UDK_BLOCK_COMMENT_RE.sub(lambda m: UDK_MARK + m.group(1), text)
    text = UDK_LINE_COMMENT_RE.sub(lambda m: m.group(1) + UDK_MARK + m.group(2), text)
    text = BLOCK_COMMENT_RE.sub(" ", text)
    text = LINE_COMMENT_RE.sub("", text)
    return text


def split_blocks(text: str):
    """Yield (statement, block_body) pairs at brace depth 0; block_body is None for ';' statements."""
    i, n = 0, len(text)
    start = 0
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if c == ";":
            yield text[start:i].strip(), None
            start = i + 1
        elif c == "{":
            depth, j = 1, i + 1
            while j < n and depth:
                ch = text[j]
                if ch == '"':
                    j += 1
                    while j < n and text[j] != '"':
                        j += 2 if text[j] == "\\" else 1
                elif ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                j += 1
            yield text[start:i].strip(), text[i + 1:j - 1]
            start = j
            i = j
            continue
        i += 1
    tail = text[start:].strip()
    if tail:
        yield tail, None


def parse_var(stmt: str) -> list[UcVar]:
    from_udk = UDK_MARK in stmt
    stmt = stmt.replace(UDK_MARK, " ")
    body = re.sub(r"^\s*var\s*(\([^)]*\))?", "", stmt, count=1).strip()
    prev = None
    while prev != body:
        prev = body
        body = TEMPLATE_SPACE_RE.sub(lambda m: m.group(0).replace(" ", ""), body)
    pieces = [p.strip() for p in body.split(",")]
    tokens = pieces[0].split()
    if len(tokens) < 2:
        return []
    names = [tokens[-1], *pieces[1:]]
    typ = tokens[-2]
    modifiers = tokens[:-2]
    out = []
    for nm in names:
        m = VAR_NAME_RE.match(nm)
        if m:
            out.append(UcVar(m.group(1), typ, m.group(2), modifiers, from_udk))
    return out


def parse_struct(head: str, body: str) -> UcStruct | None:
    head = head.replace(UDK_MARK, " ")
    m = STRUCT_DECL_RE.match(head)
    if not m:
        return None
    st = UcStruct(m.group(2), m.group(3) or "", [])
    for stmt, block in split_blocks(body):
        if block is None and re.match(r"^\s*(?:\x01UDK\x01\s*)?var\b", stmt):
            st.vars += parse_var(stmt)
    return st


def parse_uc(path: Path) -> UcClass | None:
    text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
    cls: UcClass | None = None
    for stmt, block in split_blocks(text):
        clean = stmt.replace(UDK_MARK, " ").strip()
        if cls is None:
            m = CLASS_DECL_RE.match(clean)
            if m and block is None:
                mods = " ".join(m.group(4).split())
                cls = UcClass(m.group(2), m.group(3) or "", bool(re.search(r"\bnative\b", mods)), mods, m.group(1) == "interface")
            continue
        if block is not None:
            em = ENUM_DECL_RE.match(clean)
            if em:
                cls.enums[em.group(1)] = [v.split("<")[0].strip() for v in block.replace(UDK_MARK, " ").split(",") if v.strip()]
                continue
            if re.match(r"^\s*struct\b", clean):
                st = parse_struct(stmt, block)
                if st:
                    cls.structs[st.name] = st
                continue
            fm = FUNC_DECL_RE.search(clean)
            if fm:
                cls.functions.append(fm.group(2))
            continue
        if re.match(r"^\s*var\b", clean):
            cls.vars += parse_var(stmt)
            continue
        fm = FUNC_DECL_RE.search(clean)
        if fm:
            cls.functions.append(fm.group(2))
            if re.search(r"\bnative\b", clean.split(fm.group(1))[0]):
                cls.natives.append(fm.group(2))
    return cls


def load_dfsdk(classes_dir: Path) -> dict[str, UcClass]:
    out: dict[str, UcClass] = {}
    if not classes_dir.is_dir():
        return out
    for path in sorted(classes_dir.glob("*.uc")):
        uc = parse_uc(path)
        if uc:
            out[path.stem] = uc
    return out


# ----------------------------------------------------------------------------------------------
# Header emission


def bases_of(t: dict) -> list[str]:
    return [fix_type(m["type"]) for m in t["members"] if m["is_base"]]


def base_of(t: dict) -> str:
    b = bases_of(t)
    return b[0] if b else ""


def data_members(t: dict) -> list[dict]:
    return [m for m in t["members"] if not m["is_base"] and not m["is_vftable"]]


def referenced_identifiers(t: dict) -> set[str]:
    out: set[str] = set()
    for m in data_members(t):
        out.update(IDENT_RE.findall(m["type"]))
    return out


def order_by_dependency(names: list[str], types: dict, deps_of) -> list[str]:
    done: list[str] = []
    seen: set[str] = set()
    pool = set(names)

    def visit(n: str) -> None:
        if n in seen or n not in types:
            return
        seen.add(n)
        for d in deps_of(types[n]):
            if d in pool and d != n:
                visit(d)
        done.append(n)

    for n in sorted(names):
        visit(n)
    return done


def order_classes(names: list[str], types: dict) -> list[str]:
    return order_by_dependency(names, types, lambda t: [base_of(t)])


def order_structs(names: list[str], types: dict) -> list[str]:
    return order_by_dependency(names, types, lambda t: bases_of(t) + sorted(referenced_identifiers(t)))


def emit_members(t: dict) -> list[str]:
    lines: list[str] = []
    run = None
    for m in data_members(t):
        kind = "bit" if m.get("bits") is not None else ("byte" if m["size"] == 1 and m["type"] in ("unsigned char", "char", "unsigned __int8", "__int8", "bool") else "other")
        if run in ("bit", "byte") and kind != run:
            lines.append("    SCRIPT_ALIGN;")
        run = kind
        name = m["name"] or "/*anonymous*/"
        if kind == "bit":
            dword = m["offset"] & ~3
            bit = (m["offset"] - dword) * 8 + (m.get("bit_offset") or 0)
            lines.append(f"    BITFIELD {name}:{m['bits']};  // 0x{dword:03x} bit {bit}")
            continue
        lines.append(f"    {declarator(fix_type(m['type']), name)};  // 0x{m['offset']:03x} ({m['size']})")
    if run in ("bit", "byte"):
        lines.append("    SCRIPT_ALIGN;")
    return lines


def native_index_of(n: dict) -> str:
    idx = n.get("native_index", "")
    return "-1" if idx in ("", "-1") else idx.split(";")[0]


def native_comment(n: dict) -> str:
    idx = n.get("native_index", "")
    text = f"// rva {n['rva']}, GNatives {'-1' if idx in ('', '-1') else idx}"
    if ";" in idx:
        text += " (COMDAT-folded index)"
    if n.get("folded_into"):
        text += f", folded onto {n['folded_into']}"
    elif n.get("registered") is True and n["rva"] == "0x0":
        text += ", registered (IMPLEMENT_FUNCTION initializer) but the exec thunk symbol was folded away"
    return text


def emit_struct(name: str, t: dict) -> list[str]:
    bases = bases_of(t)
    head = f"struct {name}" + (" : public " + ", public ".join(bases) if bases else "")
    lines = [f"// PDB size {t['size']}", head, "{"]
    lines += emit_members(t)
    lines += ["};", ""]
    return lines


def emit_class(name: str, t: dict, sym: Symbols, module: str) -> list[str]:
    bases = bases_of(t)
    base = bases[0] if bases else name
    stem = uc_stem(name)
    head = f"class {name}" + (f" : public {base}" if bases else "") + "".join(f", public {b}" for b in bases[1:])
    lines = [f"// PDB size {t['size']}, {len(data_members(t))} members" + (", abstract (no InternalConstructor)" if name in sym.abstract else ""), head, "{", "public:", f"    //## BEGIN PROPS {stem}"]
    lines += emit_members(t)
    lines.append(f"    //## END PROPS {stem}")
    lines.append("")
    natives = sym.natives.get(name, [])
    for n in natives:
        lines.append(f"    DECLARE_FUNCTION(exec{n['func']});  {native_comment(n)}")
    macro = "DECLARE_BASE_CLASS" if not bases else ("DECLARE_ABSTRACT_CLASS" if name in sym.abstract else "DECLARE_CLASS")
    lines += [f"    {macro}({name},{base},0,{module})", "};", ""]
    return lines


def emit_enum(e: dict) -> list[str]:
    lines = [f"enum {e['name']}", "{"]
    for m in e["members"]:
        val = f"0x{m['value'] & 0xFFFFFFFF:08x}" if e.get("is_bitmask") else str(m["value"])
        lines.append(f"    {m['name']:<24}={val},")
    lines.append("};")
    lines.append(f"#define FOREACH_ENUM_{e['name'].upper()}(op) \\")
    body = [m["name"] for m in e["members"] if not m["name"].endswith("_MAX")]
    lines += [f"    op({v}) \\" for v in body[:-1]] + ([f"    op({body[-1]}) "] if body else [])
    return lines


def guard_name(module: str) -> str:
    return re.sub(r"\W", "_", module).upper()


def native_names_block(module: str, classes: list[str], sym: Symbols) -> list[str]:
    g = guard_name(module)
    lines = ["// Native function names (per class, first occurrence), <Module>Names.h style", f"#if !defined(__{g}_NATIVE_NAMES_H__) || defined(NAMES_ONLY)", f"#ifndef __{g}_NATIVE_NAMES_H__", f"#define __{g}_NATIVE_NAMES_H__", "#endif", "", "#ifndef AUTOGENERATE_NAME", "#define DEFINED_NAME_MACRO", f"#define AUTOGENERATE_NAME(name) extern FName {g}_##name;", "#endif", ""]
    seen: set[str] = set()
    for c in classes:
        names = [n["func"] for n in sym.natives.get(c, []) if n["func"] not in seen]
        if not names:
            continue
        lines.append(f"// {c}")
        for nm in names:
            seen.add(nm)
            lines.append(f"AUTOGENERATE_NAME({nm})")
        lines.append("")
    lines += ["#ifdef DEFINED_NAME_MACRO", "#undef DEFINED_NAME_MACRO", "#undef AUTOGENERATE_NAME", "#endif", "", "#endif // HEADER_GUARD", ""]
    return lines


def collect_enum_names(classes: list[str], structs: list[str], sym: Symbols, declared: set[str]) -> tuple[list[str], list[str]]:
    referenced: set[str] = set()
    for n in classes + structs:
        referenced |= {i for i in referenced_identifiers(sym.types[n]) if i in sym.enums}
    wanted = referenced | {d for d in declared if d in sym.enums}
    missing = sorted(d for d in declared if d not in sym.enums)
    return sorted(wanted), missing


@dataclass
class ModuleSelection:
    module: str
    script_classes: list[str]
    native_classes: list[str]
    structs: list[str]
    enums: list[str]
    missing_enums: list[str]
    unknown_module: list[str]


def select_module(module: str, sym: Symbols, ref: dict[str, RefModule], uc: dict[str, UcClass]) -> ModuleSelection:
    key = module.lower()
    refmod = ref.get(key, RefModule())
    module_classes = sorted(c for c, m in sym.class_module.items() if m == key and c in sym.types)
    unknown = sorted(c for c, m in sym.class_module.items() if m == "" and c in sym.types)
    script = [c for c in module_classes if uc_stem(c) in uc or c in refmod.classes]
    native = [c for c in module_classes if c not in script]

    declared_structs: set[str] = set(refmod.structs)
    declared_enums: set[str] = set(refmod.enums)
    for c in script:
        u = uc.get(uc_stem(c))
        if u:
            declared_structs |= {"F" + s for s in u.structs}
            declared_enums |= set(u.enums)
    referenced: set[str] = set()
    for c in script + native:
        referenced |= referenced_identifiers(sym.types[c])
    structs = sorted(s for s in declared_structs if s in sym.types)
    pending = list(structs)
    while pending:
        s = pending.pop()
        for i in referenced_identifiers(sym.types[s]):
            if i in declared_structs and i in sym.types and i not in structs:
                structs.append(i)
                pending.append(i)
    enums, missing = collect_enum_names(script + native, structs, sym, declared_enums)
    return ModuleSelection(module, order_classes(script, sym.types), order_classes(native, sym.types), order_structs(structs, sym.types), enums, missing, unknown)


def emit_header(sel: ModuleSelection, sym: Symbols) -> str:
    module = sel.module
    g = guard_name(module)
    all_classes = sel.script_classes + sel.native_classes
    L: list[str] = []
    L += [f"/*===========================================================================",
          f"    {module}Classes.pdb.h — C++ class definitions recovered from the 2012 Shipping PDB.",
          "    Generated by resources/tools/symbols/gen_classes_header.py; layout comments (offset/size)",
          "    are exact, type spellings are IDA's normalized to UE3 typedefs, class flags are unknown (0).",
          f"    {len(sel.script_classes)} script-declared classes, {len(sel.native_classes)} native-only classes,",
          f"    {len(sel.structs)} script structs, {len(sel.enums)} enums.",
          "===========================================================================*/",
          "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (push,4)", "#endif", ""]
    L += native_names_block(module, all_classes, sym)
    L += ["#if !NO_ENUMS && !defined(NAMES_ONLY)", "", f"#ifndef INCLUDED_{g}_ENUMS", f"#define INCLUDED_{g}_ENUMS 1", ""]
    for e in sel.enums:
        L += emit_enum(sym.enums[e])
    if sel.missing_enums:
        L += ["", "// Declared in script but absent from the PDB (never used by native code):"]
        L += [f"//   {e}" for e in sel.missing_enums]
    L += ["", f"#endif // !INCLUDED_{g}_ENUMS", "#endif // !NO_ENUMS", "", "#if !ENUMS_ONLY", "", "#ifndef NAMES_ONLY", "#define AUTOGENERATE_FUNCTION(cls,idx,name)", "#endif", "", "", "#ifndef NAMES_ONLY", "", f"#ifndef INCLUDED_{g}_CLASSES", f"#define INCLUDED_{g}_CLASSES 1", "#define ENABLE_DECLARECLASS_MACRO 1", '#include "UnObjBas.h"', "#undef ENABLE_DECLARECLASS_MACRO", ""]
    if sel.structs:
        L += ["// ---- script structs ----", ""]
        for s in sel.structs:
            L += emit_struct(s, sym.types[s])
    L += ["// ---- script-declared classes ----", ""]
    for c in sel.script_classes:
        L += emit_class(c, sym.types[c], sym, module)
    L += ["// ---- classes without a known script declaration (not in the DFSDK subset nor the reference headers) ----", ""]
    for c in sel.native_classes:
        L += emit_class(c, sym.types[c], sym, module)
    L += ["#undef DECLARE_CLASS", "#undef DECLARE_CASTED_CLASS", "#undef DECLARE_ABSTRACT_CLASS", "#undef DECLARE_ABSTRACT_CASTED_CLASS", f"#endif // !INCLUDED_{g}_CLASSES", "#endif // !NAMES_ONLY", ""]
    for c in all_classes:
        for n in sym.natives.get(c, []):
            L.append(f"AUTOGENERATE_FUNCTION({c},{native_index_of(n)},exec{n['func']});")
    L += ["", "#ifndef NAMES_ONLY", "#undef AUTOGENERATE_FUNCTION", "#endif", "", "#ifdef STATIC_LINKING_MOJO", f"#ifndef {g}_NATIVE_DEFS", f"#define {g}_NATIVE_DEFS", "", f"#define AUTO_INITIALIZE_REGISTRANTS_{g} \\"]
    for c in all_classes:
        L.append(f"\t{c}::StaticClass(); \\")
        if sym.natives.get(c):
            L.append(f'\tGNativeLookupFuncs.Set(FName("{uc_stem(c)}"), G{module}{c}Natives); \\')
    L += ["", f"#endif // {g}_NATIVE_DEFS", "", "#ifdef NATIVES_ONLY"]
    for c in all_classes:
        natives = sym.natives.get(c)
        if not natives:
            continue
        L += [f"FNativeFunctionLookup G{module}{c}Natives[] = ", "{ "]
        L += [f"\tMAP_NATIVE({c}, exec{n['func']})" for n in natives]
        L += ["\t{NULL, NULL}", "};", ""]
    L += ["#endif // NATIVES_ONLY", "#endif // STATIC_LINKING_MOJO", "", "#ifdef VERIFY_CLASS_SIZES"]
    for c in all_classes:
        members = [m for m in data_members(sym.types[c]) if m.get("bits") is None and m["name"]]
        for m in (members[:1] + members[-1:] if len(members) > 1 else members):
            L.append(f"VERIFY_CLASS_OFFSET_NODIE({c},{uc_stem(c)},{m['name']})")
        L.append(f"VERIFY_CLASS_SIZE_NODIE({c})")
    L += ["#endif // VERIFY_CLASS_SIZES", "", "#endif // !ENUMS_ONLY", "", "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (pop)", "#endif", ""]
    return "\n".join(L)


# ----------------------------------------------------------------------------------------------
# Reference diff and DFSDK inventory


def lcs_len(a: list[str], b: list[str]) -> int:
    prev = [0] * (len(b) + 1)
    for x in a:
        cur = [0]
        for j, y in enumerate(b):
            cur.append(prev[j] + 1 if x == y else max(prev[j + 1], cur[j]))
        prev = cur
    return prev[-1]


def order_mismatches(a: list[str], b: list[str]) -> int:
    common_a = [x for x in a if x in set(b)]
    common_b = [x for x in b if x in set(a)]
    return len(common_a) - lcs_len(common_a, common_b)


def fmt_list(items: list[str], limit: int = 12) -> str:
    if not items:
        return "-"
    shown = ", ".join(f"`{i}`" for i in items[:limit])
    return shown + (f" (+{len(items) - limit} more)" if len(items) > limit else "")


def diff_reference(sel: ModuleSelection, sym: Symbols, ref: dict[str, RefModule]) -> str:
    refmod = ref.get(sel.module.lower(), RefModule())
    out = [f"# {sel.module}: PDB header vs reference {sel.module}*Classes.h", ""]
    pdb_script = set(sel.script_classes)
    out.append(f"- reference classes: {len(refmod.classes)}; generated script-declared classes: {len(pdb_script)}; native-only: {len(sel.native_classes)}")
    out.append(f"- only in reference: {fmt_list(sorted(set(refmod.classes) - pdb_script), 40)}")
    out.append(f"- only in generated (script section): {fmt_list(sorted(pdb_script - set(refmod.classes)), 40)}")
    out.append(f"- reference structs: {fmt_list(sorted(refmod.structs), 40)}; generated structs: {fmt_list(sel.structs, 40)}")
    out.append(f"- reference enums: {fmt_list(sorted(refmod.enums), 40)}; generated enums: {fmt_list(sel.enums, 40)}")
    out.append("")
    out.append("| class | base pdb/ref | members pdb/ref | only pdb | only ref | order mismatches | natives pdb/ref | native diffs |")
    out.append("|---|---|---|---|---|---|---|---|")
    for c in sel.script_classes:
        if c not in refmod.classes:
            continue
        t = sym.types[c]
        pdb_members = [m["name"] for m in data_members(t)]
        ref_members = refmod.props.get(uc_stem(c), [])
        pdb_nat = [f"exec{n['func']}" for n in sym.natives.get(c, [])]
        ref_nat = refmod.natives.get(c, [])
        only_pdb = [m for m in pdb_members if m not in ref_members]
        only_ref = [m for m in ref_members if m not in pdb_members]
        nat_diff = sorted(set(pdb_nat) ^ set(ref_nat))
        out.append(f"| {c} | {base_of(t)}/{refmod.classes[c]} | {len(pdb_members)}/{len(ref_members)} | {fmt_list(only_pdb)} | {fmt_list(only_ref)} | {order_mismatches(pdb_members, ref_members)} | {len(pdb_nat)}/{len(ref_nat)} | {fmt_list(nat_diff)} |")
    return "\n".join(out)


def inventory(sel: ModuleSelection, sym: Symbols, uc: dict[str, UcClass]) -> str:
    module = sel.module
    all_classes = sel.script_classes + sel.native_classes
    with_uc = [c for c in all_classes if uc_stem(c) in uc]
    pdb_stems = {uc_stem(c) for c in sym.class_module if c in sym.types}
    uc_without_pdb = sorted(s for s in uc if s not in pdb_stems)
    dis_elsewhere = sorted(c for c, m in sym.class_module.items() if m != module.lower() and uc_stem(c) in uc)
    out = [f"# {module} class inventory (2012 Shipping PDB vs DFSDK .uc)", "",
           "Generated by `python resources/tools/symbols/gen_classes_header.py DishonoredGame --inventory ...`.",
           "PDB classes are those with a `PrivateStaticClass` global whose member functions mostly live in the module.",
           "The DFSDK .uc are UE Explorer decompiles of the 2013 retail scripts, hand-edited for UDK 2010",
           "(`udk`-tagged comments hide retail declarations; they are treated as live here), so differences are expected.", "",
           "## Summary", "",
           f"- PDB classes attributed to {module}: **{len(all_classes)}** ({len(sel.script_classes)} with a .uc or reference declaration, {len(sel.native_classes)} native-only)",
           f"- .uc files: {len(uc)}; PDB classes with a .uc: {len(with_uc)}; .uc without a PDB class in any module: {len(uc_without_pdb)}",
           f"- script structs emitted: {len(sel.structs)}; enums emitted: {len(sel.enums)}; script enums missing from the PDB: {len(sel.missing_enums)}",
           f"- classes with a .uc attributed to another module (Arkane engine-side classes): {fmt_list(dis_elsewhere, 60)}",
           f"- classes whose module could not be attributed (no member function in functions.csv): {len(sel.unknown_module)}", ""]
    out += ["## 2012 classes with no .uc in the DFSDK subset", ""]
    out.append(fmt_list(sel.native_classes, 1000))
    out += ["", "## .uc with no 2012 class (added in 2013 or script-only)", ""]
    out.append(fmt_list(uc_without_pdb, 1000))
    out += ["", "## Per-class table", "", "| class | base | size | members | .uc | uc vars | only PDB | only .uc | order mismatches | natives PDB/.uc | native diffs | enum diffs |", "|---|---|---|---|---|---|---|---|---|---|---|---|"]
    details: list[str] = []
    for c in all_classes:
        t = sym.types[c]
        u = uc.get(uc_stem(c))
        pdb_members = [m["name"] for m in data_members(t)]
        row = [c, base_of(t), str(t["size"]), str(len(pdb_members))]
        if not u:
            row += ["no", "-", "-", "-", "-", f"{len(sym.natives.get(c, []))}/-", "-", "-"]
            out.append("| " + " | ".join(row) + " |")
            continue
        uc_vars = [v.name for v in u.vars if not v.name.startswith("VfTable_")]
        only_pdb = [m for m in pdb_members if m not in uc_vars]
        only_uc = [m for m in uc_vars if m not in pdb_members]
        pdb_nat = sorted(n["func"] for n in sym.natives.get(c, []))
        uc_nat = sorted(set(u.natives))
        nat_only_pdb = [n for n in pdb_nat if n not in uc_nat]
        nat_only_uc = [n for n in uc_nat if n not in pdb_nat]
        enum_diffs: list[str] = []
        for en, values in u.enums.items():
            pe = sym.enums.get(en)
            if not pe:
                enum_diffs.append(f"{en}: not in PDB")
                continue
            uc_vals = [v for v in values if not v.endswith("_MAX")]
            pdb_vals = [m["name"] for m in pe["members"] if not m["name"].endswith("_MAX")]
            if uc_vals != pdb_vals:
                only_uc_v = [v for v in uc_vals if v not in pdb_vals]
                only_pdb_v = [v for v in pdb_vals if v not in uc_vals]
                note = f"{en}: {len(uc_vals)} uc / {len(pdb_vals)} pdb values"
                if only_uc_v:
                    note += f", only uc {fmt_list(only_uc_v, 8)}"
                if only_pdb_v:
                    note += f", only pdb {fmt_list(only_pdb_v, 8)}"
                if not only_uc_v and not only_pdb_v:
                    note += ", same values in a different order"
                enum_diffs.append(note)
        struct_missing = [s for s in u.structs if "F" + s not in sym.types]
        base_uc = u.extends
        base_note = base_of(t) + ("" if not base_uc or uc_stem(base_of(t)) == base_uc else f" (uc: {base_uc})")
        row = [c, base_note, str(t["size"]), str(len(pdb_members)), ("interface" if u.is_interface else "yes") + (" native" if u.is_native else " script"), str(len(uc_vars)), str(len(only_pdb)), str(len(only_uc)), str(order_mismatches(pdb_members, uc_vars)), f"{len(pdb_nat)}/{len(uc_nat)}", f"+{len(nat_only_pdb)}/-{len(nat_only_uc)}", str(len(enum_diffs))]
        out.append("| " + " | ".join(row) + " |")
        if only_pdb or only_uc or nat_only_pdb or nat_only_uc or enum_diffs or struct_missing or order_mismatches(pdb_members, uc_vars):
            details.append(f"### {c}")
            details.append("")
            if only_pdb:
                details.append(f"- members only in PDB: {fmt_list(only_pdb, 30)}")
            if only_uc:
                details.append(f"- vars only in .uc: {fmt_list(only_uc, 30)}")
            om = order_mismatches(pdb_members, uc_vars)
            if om:
                details.append(f"- {om} common members out of .uc order")
            if nat_only_pdb:
                details.append(f"- natives only in PDB: {fmt_list(nat_only_pdb, 30)}")
            if nat_only_uc:
                details.append(f"- natives only in .uc: {fmt_list(nat_only_uc, 30)}")
            for d in enum_diffs:
                details.append(f"- enum {d}")
            if struct_missing:
                details.append(f"- .uc structs with no PDB type: {fmt_list(struct_missing, 30)}")
            details.append("")
    out += ["", "## Mismatch details", ""] + details
    return "\n".join(out)


# ==============================================================================================
# --sdk mode (Phase 3 wave 2, agent T): headers + registrants of a game module from the RETAIL
# layout. Inputs: retail_sdk_layout.json (CodeRed dump: member offsets/types, function parameter
# blocks, FunctionFlags), native_class_sizes.csv (retail sizeof, ClassFlags, Within, config),
# script_classes_2013.json (enum values, consts, property kinds/order, header groups), the 2012
# types.json (names/types for native-only gaps and the native tail), natives_2013.csv (numbered
# natives). Reuses sdk_props.map_type / sdk_groups / the 2012 relative-position gap rule.
# ==============================================================================================

sys.path.insert(0, str(REPO / "resources" / "tools" / "sdk"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_layout_probe import MODULE_INCLUDES, declared_types  # noqa: E402

_pcs = None
ALIGN16: set = set()
map_type = pdb_data_members = pdb_decl = pdb_size_of = sdk_groups = None


def _sdk_imports() -> None:
    """sdk_props imports this file for declarator/fix_type/uc_stem, so its symbols are bound lazily (no import cycle at load)"""
    global _pcs, ALIGN16, MIRROR_FALLBACK, map_type, pdb_data_members, pdb_decl, pdb_size_of, sdk_groups
    import parse_codered_sdk as pcs
    import sdk_props as sp
    _pcs = pcs
    ALIGN16 = sp.ALIGN16
    MIRROR_FALLBACK = sp.MIRROR_FALLBACK
    map_type, pdb_data_members, pdb_decl, pdb_size_of, sdk_groups = sp.map_type, sp.pdb_data_members, sp.pdb_decl, sp.pdb_size_of, sp.sdk_groups


SDK_SOURCE = REPO / "source" / "Development" / "Src"
CPF_CONST, CPF_OPTIONAL, CPF_PARM, CPF_OUT, CPF_RETURN = 0x2, 0x10, 0x80, 0x100, 0x400
FUNC_NATIVE, FUNC_EVENT, FUNC_DELEGATE = 0x400, 0x800, 0x100000
CLASS_ABSTRACT, CLASS_INTERFACE, CLASS_DEPRECATED, CLASS_INTRINSIC = 0x1, 0x4000, 0x2000000, 0x10000000
CLASS_FLAG_NAMES = [(0x1, "CLASS_Abstract"), (0x2, "CLASS_Compiled"), (0x4, "CLASS_Config"), (0x8, "CLASS_Transient"), (0x10, "CLASS_Parsed"),
                    (0x20, "CLASS_Localized"), (0x40, "CLASS_SafeReplace"), (0x80, "CLASS_Native"), (0x100, "CLASS_NoExport"), (0x200, "CLASS_Placeable"),
                    (0x400, "CLASS_PerObjectConfig"), (0x800, "CLASS_NativeReplication"), (0x1000, "CLASS_EditInlineNew"), (0x2000, "CLASS_CollapseCategories"),
                    (0x4000, "CLASS_Interface"), (0x200000, "CLASS_HasInstancedProps"), (0x400000, "CLASS_NeedsDefProps"), (0x800000, "CLASS_HasComponents"),
                    (0x1000000, "CLASS_Hidden"), (0x2000000, "CLASS_Deprecated"), (0x4000000, "CLASS_HideDropDown"), (0x8000000, "CLASS_Exported"),
                    (0x10000000, "CLASS_Intrinsic"), (0x20000000, "CLASS_NativeOnly"), (0x40000000, "CLASS_PerObjectLocalized"), (0x80000000, "CLASS_HasCrossLevelRefs")]
# SDK spellings sdk_props.map_type does not cover (generator built-ins)
SDK_EXTRA_TYPES = {"struct FDouble": "DOUBLE", "struct FQWord": "QWORD", "uint32_t": "INT", "bool": "UBOOL"}
CPP_KEYWORDS = {"class", "struct", "default", "new", "delete", "template", "operator", "this", "int", "float", "bool", "char", "short", "long",
                "double", "void", "union", "enum", "namespace", "typename", "virtual", "static", "const", "register", "auto", "switch", "case",
                "break", "continue", "return", "if", "else", "for", "while", "do", "goto", "try", "catch", "throw", "true", "false", "signed",
                "unsigned", "inline", "extern", "friend", "private", "public", "protected", "typedef", "sizeof", "explicit", "mutable", "volatile",
                "and", "or", "not", "xor", "export", "using", "asm", "far", "near", "interface", "min", "max"}
# members the module's generated headers must not shadow (macros / UObject API used by the DECLARE_CLASS machinery)
MEMBER_RENAMES = {"Result": "Result_", "Stack": "Stack_"}
MODULE_PACKAGE_DEPS = {"DishonoredGame": ["GFxUI", "AkAudio", "OnlineSubsystemSteamworks"]}
SHIM_PACKAGES = {"Core", "Engine", "GameFramework"}
MAP_PROPERTY_SIZE = 60
# (ElementSize, MinAlignment) of the generator's built-in struct spellings (Core intrinsic structs)
SDK_BUILTIN_STRUCTS = {"FPointer": (4, 4), "FQWord": (8, 4), "FDouble": (8, 4), "FMap_Mirror": (MAP_PROPERTY_SIZE, 4), "FMultiMap_Mirror": (MAP_PROPERTY_SIZE, 4),
                       "FScriptInterface": (8, 4), "FScriptDelegate": (12, 4), "FGuid": (16, 4), "FThreadSafeCounter": (4, 4)}
MIRROR_FALLBACK: dict = {}
KNOWN_CORE_TYPES = {"FName", "FString", "FStringNoInit", "FVector", "FRotator", "FMatrix", "FColor", "FLinearColor", "FGuid", "FBox", "FQuat", "FPlane", "FVector2D", "FVector4", "FBoneAtom", "FScriptDelegate", "FScriptInterface", "INT", "FLOAT", "BYTE", "UBOOL", "DWORD", "QWORD", "UObject", "UClass", "AActor"}
TREE_MODULES = ["Core", "Engine", "GameFramework", "IpDrv", "WinDrv"]
SDK_TARRAY_RE = re.compile(r"^class TArray<(.+)>$")
SDK_CLASSPTR_RE = re.compile(r"^class ([UAI]\w+)\*$")
SDK_STRUCT_RE = re.compile(r"^struct (F\w+)$")
EVENTPARM_CTOR_RE = re.compile(r"\b(F\w+)\s*\(\s*EEventParm\s*\)")


def sdk_align(n: int, a: int) -> int:
    return (n + a - 1) // a * a


@dataclass
class SdkData:
    module: str
    classes: dict            # cpp -> SDK class entry (all packages)
    structs: dict            # cpp -> SDK struct entry (all packages, derived structs re-parsed)
    functions: dict          # "Pkg.Class.Func" -> entry
    script: dict             # script name -> script_classes_2013 class
    sizes: dict              # cpp -> native_class_sizes row
    pdb: dict                # 2012 types.json types
    pdb_enums: dict
    natives13: dict          # (cpp, func) -> native index
    declared: set            # types declared by the tree outside this module
    eventparm_structs: set   # structs with an (EEventParm) constructor in the tree
    stem_to_cpp: dict
    declared_enums: set = field(default_factory=set)
    pure_virtuals: dict = field(default_factory=dict)   # tree class -> (bases, [pure virtual signature parts])


def sdk_load(module: str) -> SdkData:
    data = json.loads((TYPES / "retail_sdk_layout.json").read_text(encoding="utf-8"))
    structs = dict(data["structs"])
    # parse_codered_sdk.DECL_RE only accepts `struct X : public Y`; the dump writes derived script structs
    # as `struct X : Y`, so re-parse the struct files with a relaxed declaration regex for the missing ones
    src = _pcs.DEFAULT_SDK / "src"
    if src.is_dir():
        saved = _pcs.DECL_RE
        _pcs.DECL_RE = re.compile(r"^(class|struct) ([A-Za-z_]\w*)(?:\s*:\s*(?:public\s+)?([A-Za-z_]\w*))?\s*$")
        extra: dict = {}
        for p in sorted(src.glob("*_structs.hpp")):
            _pcs.parse_types(p, "struct", p.name.split("_")[0], extra)
        _pcs.DECL_RE = saved
        for n, s in extra.items():
            structs.setdefault(n, s)
    script = json.loads((TYPES / "script_classes_2013.json").read_text(encoding="utf-8"))["classes"]
    with (TYPES / "native_class_sizes.csv").open(newline="", encoding="utf-8") as f:
        sizes = {r["class"]: r for r in csv.DictReader(f) if r.get("size_2013")}
    with (TYPES / "types.json").open(encoding="utf-8") as f:
        pdb_raw = json.load(f)
    pdb = {t["name"]: t for t in pdb_raw["types"]}
    pdb_enums = {e["name"]: e for e in pdb_raw.get("enums", [])}
    natives13 = {}
    p = SYM / "natives_2013.csv"
    if p.exists():
        with p.open(newline="", encoding="utf-8") as f:
            for r in csv.DictReader(f):
                natives13[(r["class"], r["func"])] = r.get("native_index", "") or "-1"
    declared: set = set()
    eventparm: set = set()
    declared_enums: set = set()
    pure_virtuals: dict = {}
    for m in TREE_MODULES + MODULE_PACKAGE_DEPS.get(module, []):
        if m == module or not (SDK_SOURCE / m).is_dir():
            continue
        declared |= declared_types(SDK_SOURCE / m)
        for h in list((SDK_SOURCE / m / "Inc").rglob("*.h")) + list((SDK_SOURCE / m / "Src").rglob("*.h")):
            text = h.read_text(encoding="utf-8", errors="replace")
            eventparm |= set(EVENTPARM_CTOR_RE.findall(text))
            code = "\n".join(l for l in text.splitlines() if not l.lstrip().startswith("#"))
            declared |= {mm.group(1) for mm in re.finditer(r"\bclass\s+(I[A-Z]\w*)\s*(?::[^{;]*)?\{", code)}
            declared_enums |= {mm.group(1) for mm in re.finditer(r"^\s*enum\s+(\w+)\s*(?:\{|$)", code, re.M)}
            pure_virtuals.update(tree_pure_virtuals(code))
            declared |= {f"U{mm.group(1)}Commandlet" for mm in re.finditer(r"BEGIN(?:_CHILD)?_COMMANDLET\(\s*(\w+)\s*,", text)}
    stem_to_cpp = {uc_stem(n): n for n in list(sizes) + list(data["classes"])}
    return SdkData(module, data["classes"], structs, data["functions"], script, sizes, pdb, pdb_enums, natives13, declared, eventparm, stem_to_cpp, declared_enums, pure_virtuals)


PURE_VIRTUAL_RE = re.compile(r"virtual\s+([^;{}()]*?)\b(\w+)\s*(\([^;{}]*?\))\s*(const)?\s*=\s*0\s*;", re.S)
TREE_CLASS_RE = re.compile(r"\bclass\s+([A-Z]\w*)\s*(?::\s*([^{;]*))?\{")


def tree_pure_virtuals(code: str) -> dict:
    """class -> (base names, [(return type, name, params, const)]) for every class of a header that declares pure virtuals"""
    out = {}
    for m in TREE_CLASS_RE.finditer(code):
        depth, i = 1, m.end()
        while i < len(code) and depth:
            depth += {"{": 1, "}": -1}.get(code[i], 0)
            i += 1
        body = code[m.end():i - 1]
        pures = [(r.group(1).strip(), r.group(2), " ".join(r.group(3).split()), bool(r.group(4))) for r in PURE_VIRTUAL_RE.finditer(body)]
        bases = [b.split()[-1] for b in (m.group(2) or "").split(",") if b.strip()]
        if pures or bases:
            out[m.group(1)] = (bases, pures)
    return out


def sdk_write(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="\r\n") as f:
        f.write(text)


# ---------------------------------------------------------------------------------------------
# Link emulation (UStruct::Link / UProperty::Link rules of UnClass.cpp) for the retail package member
# lists: validates the SDK offsets and synthesizes the layout of native classes the dump never saw.


def struct_cpp_of(path: str) -> str:
    return "F" + path.split(".")[-1]


class LinkEmulator:
    def __init__(self, data: SdkData):
        self.d = data

    def struct_size_align(self, cpp: str) -> tuple[int, int] | None:
        """(ElementSize, MinAlignment) of a struct property: UStructProperty::Link aligns PropertiesSize to the struct's MinAlignment"""
        if cpp in SDK_BUILTIN_STRUCTS:
            return SDK_BUILTIN_STRUCTS[cpp]
        s = self.d.structs.get(cpp)
        if s is not None:
            a = sdk_type_align(f"struct {cpp}", self.d)
            return sdk_align(s.get("span_end", 0), a), a
        t = self.d.pdb.get(MIRROR_FALLBACK.get(f"struct {cpp}", f"struct {cpp}")[len("struct "):])
        if t is not None:
            return t["size"], (16 if cpp in ALIGN16 or t.get("align", 4) >= 16 else 4)
        return None

    def prop_type(self, p: dict) -> str | None:
        k = p["kind"]
        if k in ("IntProperty",):
            return "int32_t"
        if k == "FloatProperty":
            return "float"
        if k == "ByteProperty":
            return p["enum"].split(".")[-1] if p.get("enum") else "uint8_t"
        if k == "BoolProperty":
            return "uint32_t"
        if k == "NameProperty":
            return "class FName"
        if k == "StrProperty":
            return "class FString"
        if k in ("ObjectProperty", "ComponentProperty"):
            stem = p.get("class", "Core.Object").split(".")[-1]
            return f"class {self.d.stem_to_cpp.get(stem, 'U' + stem)}*"
        if k == "ClassProperty":
            return "class UClass*"
        if k == "InterfaceProperty":
            return "struct FScriptInterface"
        if k == "DelegateProperty":
            return "struct FScriptDelegate"
        if k == "StructProperty":
            return f"struct {struct_cpp_of(p['struct'])}"
        if k == "ArrayProperty":
            inner = self.prop_type(p["inner"])
            return None if inner is None else f"class TArray<{inner}>"
        if k == "MapProperty":
            return "struct FMap_Mirror"
        return None

    def prop_size_align(self, p: dict) -> tuple[int, int] | None:
        k = p["kind"]
        simple = {"IntProperty": (4, 4), "FloatProperty": (4, 4), "ByteProperty": (1, 1), "NameProperty": (8, 4), "StrProperty": (12, 4),
                  "ObjectProperty": (4, 4), "ComponentProperty": (4, 4), "ClassProperty": (4, 4), "InterfaceProperty": (8, 4),
                  "DelegateProperty": (12, 4), "ArrayProperty": (12, 4), "MapProperty": (MAP_PROPERTY_SIZE, 4)}
        if k in simple:
            return simple[k]
        if k == "StructProperty":
            return self.struct_size_align(struct_cpp_of(p["struct"]))
        return None

    def link(self, props: list[dict], start: int) -> list[dict] | None:
        """members in SDK format (offset/size/type/bitfield/mask/count) or None when a size is unknown"""
        out = []
        cursor = sdk_align(start, 4)
        prev_bool = None  # (offset, next_mask)
        for p in props:
            if p["kind"] == "BoolProperty":
                if prev_bool is not None and prev_bool[1] != 0:
                    off, mask = prev_bool
                else:
                    off = sdk_align(cursor, 4)
                    mask = 1
                    cursor = off + 4
                out.append({"name": p["name"], "type": "uint32_t", "offset": off, "size": 4, "bitfield": True, "mask": mask, "flags": int(p["flags"], 16)})
                prev_bool = (off, (mask << 1) & 0xFFFFFFFF)
                continue
            prev_bool = None
            sa = self.prop_size_align(p)
            ty = self.prop_type(p)
            if sa is None or ty is None:
                return None
            size, align = sa
            off = sdk_align(cursor, align)
            dim = p.get("array_dim", 1) or 1
            m = {"name": p["name"], "type": ty, "offset": off, "size": size * dim, "flags": int(p["flags"], 16)}
            if dim > 1:
                m["count"] = dim
            out.append(m)
            cursor = off + size * dim
        return out

    def synthesize(self, cpp: str, stem: str, package: str) -> dict | None:
        sc = self.d.script.get(stem)
        row = self.d.sizes.get(cpp)
        if sc is None or row is None:
            return None
        sup_cpp = row["super_2013"]
        sup = self.d.classes.get(sup_cpp)
        if sup is None:
            return None
        # the interface vtable pointers are CPF_NoExport Pointer properties of the package class (VfTable_I*), linked like any other
        props = [ch for ch in sc["children"] if ch["kind"].endswith("Property")]
        start = sdk_align(sup["span_end"], 4)
        members = self.link(props, start)
        if members is None:
            return None
        end = max((m["offset"] + m["size"] for m in members), default=start)
        return {"cpp": cpp, "path": f"{package}.{stem}", "package": package, "super": sup_cpp, "members": members, "gaps": [],
                "span_start": sup["span_end"], "span_end": end, "synthesized": True}

    def check(self, entry: dict) -> tuple[int, int, list[str]]:
        """(checked, mismatching, notes) of the emulated layout against the SDK entry"""
        sc = self.d.script.get(uc_stem(entry["cpp"]))
        if sc is None or "span_start" not in entry:
            return 0, 0, []
        props = [ch for ch in sc["children"] if ch["kind"].endswith("Property")]
        linked = self.link(props, sdk_align(entry["span_start"], 4))
        if linked is None:
            return 0, 0, [f"{entry['cpp']}: a property size is unknown to the emulator"]
        want = {m["name"]: m for m in entry["members"]}
        bad = []
        checked = 0
        for m in linked:
            w = want.get(m["name"]) or want.get(m["name"] + "_Object")
            if w is None:
                continue
            checked += 1
            if w["offset"] != m["offset"] or (m.get("bitfield") and w.get("mask") != m.get("mask")):
                bad.append(f"{m['name']} sdk {w['offset']} emu {m['offset']}")
        return checked, len(bad), ([f"{entry['cpp']}: " + ", ".join(bad[:4])] if bad else [])


# ---------------------------------------------------------------------------------------------
# Type mapping


def sdk_type_align(t: str, data: SdkData, module_structs: dict | None = None) -> int:
    if t in ("uint8_t", "bool") or re.match(r"^E\w+$", t):
        return 1
    m = SDK_STRUCT_RE.match(t)
    if m:
        name = m.group(1)
        if name in ALIGN16:
            return 16
        if name in SDK_BUILTIN_STRUCTS:
            return SDK_BUILTIN_STRUCTS[name][1]
        # UStruct::Link (UnClass.cpp): MinAlignment = 16 for Matrix/Plane/Vector4/Quat/SHVector, 4 for Color, else Max(members, 4)
        pt = data.pdb.get(name)
        if pt is not None and name not in data.structs:
            return 16 if pt.get("align", 4) >= 16 else 4
        return max(4, sdk_struct_natural_align(name, data))
    return 4


def sdk_struct_natural_align(name: str, data: SdkData) -> int:
    """C++ alignment the members give a script struct on their own (1 for byte-only structs)"""
    s = data.structs.get(name)
    if s is None:
        return 4
    a = 1
    if s.get("super"):
        a = max(a, sdk_type_align(f"struct {s['super']}", data))
    for mm in s["members"]:
        a = max(a, 4 if mm.get("bitfield") else sdk_type_align(mm["type"], data))
    return a


def sdk_class_align(cpp: str, data: SdkData, in_module: set) -> int:
    a = 4
    entry = data.classes.get(cpp)
    if entry is None:
        pt = data.pdb.get(cpp)
        return 16 if pt is not None and pt.get("align", 4) >= 16 else 4
    if entry.get("super"):
        a = max(a, sdk_class_align(entry["super"], data, in_module))
    for m in entry["members"]:
        if not m.get("bitfield"):
            a = max(a, sdk_type_align(m["type"], data))
    pt = data.pdb.get(cpp)
    if pt is not None and pt.get("align", 4) >= 16:
        a = 16
    return a


def sdk_cpp_type(t: str, inner: bool = False) -> str | None:
    """member spelling: reuse sdk_props.map_type, plus the generator built-ins it does not know"""
    if t in SDK_EXTRA_TYPES:
        return SDK_EXTRA_TYPES[t]
    m = SDK_TARRAY_RE.match(t)
    if m and m.group(1).strip() in SDK_EXTRA_TYPES:
        return f"{'TArray' if inner else 'TArrayNoInit'}<{SDK_EXTRA_TYPES[m.group(1).strip()]}>"
    return map_type(t, inner)


def sdk_value_type(t: str) -> str | None:
    """by-value spelling for function parameters / parameter structs"""
    if t in SDK_EXTRA_TYPES:
        return SDK_EXTRA_TYPES[t]
    m = SDK_TARRAY_RE.match(t)
    if m:
        e = sdk_value_type(m.group(1).strip())
        return None if e is None else f"TArray<{e}>"
    if t == "class FString":
        return "FString"
    return map_type(t, True)


def safe_ident(name: str) -> str:
    if name in CPP_KEYWORDS:
        return name + "_"
    return MEMBER_RENAMES.get(name, name)


def class_flag_text(flags: int) -> str:
    names = [n for bit, n in CLASS_FLAG_NAMES if flags & bit]
    rest = flags & ~sum(bit for bit, _ in CLASS_FLAG_NAMES)
    if rest:
        names.append(f"0x{rest:x}")
    return "0" + "".join("|" + n for n in names)


# ---------------------------------------------------------------------------------------------
# Module selection


@dataclass
class SdkItem:
    kind: str                 # class | struct | iface
    name: str                 # emitted C++ name
    entry: dict | None        # SDK entry (classes/structs); None for placeholder ifaces
    package: str = ""
    row: dict | None = None   # native_class_sizes row (classes)
    script: dict | None = None
    shim: bool = False
    group: str = ""
    deps: list = field(default_factory=list)
    stem: str = ""
    functions: list = field(default_factory=list)


@dataclass
class SdkSelection:
    module: str
    items: dict               # name -> SdkItem
    order: list               # topological order of names
    groups: list              # [(group, [names])] in include order
    enums: dict               # name -> values
    consts: dict              # name -> value
    forward: list             # class names to forward-declare
    rename: dict              # SDK cpp -> emitted cpp (DEPRECATED_ prefix)
    skipped: dict             # cpp -> reason
    notes: list
    stats: dict
    group_deps: dict = field(default_factory=dict)


def sdk_referenced_types(entry: dict) -> tuple[set, set]:
    """(struct names embedded by value, class names referenced by pointer) of an SDK class/struct entry"""
    structs, ptrs = set(), set()
    for m in entry["members"]:
        t = m["type"]
        tm = SDK_TARRAY_RE.match(t)
        inner = tm.group(1).strip() if tm else t
        sm = SDK_STRUCT_RE.match(inner)
        if sm:
            structs.add(sm.group(1))
        pm = SDK_CLASSPTR_RE.match(inner)
        if pm:
            ptrs.add(pm.group(1))
    return structs, ptrs


def sdk_select(data: SdkData, shims: bool) -> SdkSelection:
    module = data.module
    d = data
    items: dict[str, SdkItem] = {}
    skipped: dict[str, str] = {}
    notes: list[str] = []
    rename: dict[str, str] = {}
    emu = LinkEmulator(d)
    tree_stems = {uc_stem(n): n for n in d.declared if n[:1] in "UA" and len(n) > 1 and n[1].isupper()}

    def emitted_name(cpp: str, flags: int) -> str:
        if flags & CLASS_DEPRECATED and "DEPRECATED_" not in cpp:
            return cpp[0] + "DEPRECATED_" + cpp[1:]
        return cpp

    # 1. the module's native classes (retail descriptor = truth for the set, size and flags)
    rows = [(cpp, r) for cpp, r in d.sizes.items() if r["package_2013"] == module]
    if shims:
        for cpp, r in d.sizes.items():
            if r["package_2013"] in SHIM_PACKAGES and cpp not in d.declared:
                other = tree_stems.get(uc_stem(cpp))
                if other and other != cpp:
                    skipped[cpp] = f"shim skipped: the tree declares {other} for script class {uc_stem(cpp)} (retail prefix/base differs)"
                    continue
                rows.append((cpp, r))
    for cpp, r in rows:
        flags = int(r["flags_2013"], 16)
        stem = uc_stem(cpp)
        entry = d.classes.get(cpp)
        if entry is None:
            entry = emu.synthesize(cpp, stem, r["package_2013"])
            if entry is None:
                skipped[cpp] = "not in the SDK dump and the retail package member list cannot be linked"
                continue
            notes.append(f"{cpp}: layout synthesized from the retail package member list (not in the SDK dump): {len(entry['members'])} members, span {entry['span_start']}..{entry['span_end']}")
            d.classes[cpp] = entry
        name = emitted_name(cpp, flags)
        if name != cpp:
            rename[cpp] = name
        it = SdkItem("class", name, entry, r["package_2013"], r, d.script.get(stem), shim=r["package_2013"] != module, stem=stem)
        items[name] = it
        if flags & CLASS_INTERFACE:
            items["I" + stem] = SdkItem("iface", "I" + stem, entry, r["package_2013"], r, it.script, shim=it.shim, stem=stem)
    class_names = {it.entry["cpp"]: it.name for it in items.values() if it.kind == "class"}

    # 2. drop classes whose super chain leaves the declared world
    def super_ok(cpp: str) -> str | None:
        seen = set()
        while cpp and cpp not in seen:
            seen.add(cpp)
            if cpp in d.declared or cpp in class_names:
                return None
            entry = d.classes.get(cpp)
            row = d.sizes.get(cpp)
            if entry is None and row is None:
                return cpp
            if cpp in skipped:
                return cpp
            cpp = (entry or {}).get("super") or (row or {}).get("super_2013")
        return cpp
    changed = True
    while changed:
        changed = False
        for name, it in list(items.items()):
            if it.kind != "class":
                continue
            missing = super_ok(it.entry["super"]) if it.entry.get("super") else None
            if missing:
                skipped[it.entry["cpp"]] = f"super chain needs {missing}, which is neither declared in the tree nor generated"
                del items[name]
                items.pop("I" + it.stem, None)
                del class_names[it.entry["cpp"]]
                changed = True

    # 3. structs: the module's own, plus every struct embedded by value that the tree does not declare
    for cpp, s in d.structs.items():
        if s["package"] == module:
            items[cpp] = SdkItem("struct", cpp, s, s["package"])
    pending = [it.entry for it in items.values() if it.entry is not None]
    seen_entries = set()
    forward: set[str] = set()
    while pending:
        entry = pending.pop()
        key = (entry.get("cpp") or entry["path"], entry["path"])
        if key in seen_entries:
            continue
        seen_entries.add(key)
        structs, ptrs = sdk_referenced_types(entry)
        if entry.get("super") and entry["path"].count(".") == 2 and entry["super"] not in items and entry["super"] not in d.declared and entry["super"] in d.structs:
            structs.add(entry["super"])
        for s in structs:
            if s in items or s in d.declared:
                continue
            se = d.structs.get(s)
            if se is None:
                if s not in ("FPointer", "FQWord", "FDouble", "FMap_Mirror", "FMultiMap_Mirror", "FScriptInterface", "FScriptDelegate"):
                    notes.append(f"struct {s} is referenced but neither declared in the tree nor in the SDK dump")
                continue
            items[s] = SdkItem("struct", s, se, se["package"], shim=se["package"] != module)
            pending.append(se)
        for pcls in ptrs:
            if pcls not in class_names and pcls not in d.declared:
                forward.add(pcls)
    # parameter structs reference types as well
    for path, f in d.functions.items():
        pkg, stem, _ = path.split(".", 2)
        cpp = d.stem_to_cpp.get(stem)
        if cpp not in class_names and cpp not in rename:
            continue
        for p in f["params"]:
            if not p.get("flags", 0) & CPF_PARM:
                continue
            t = p["type"]
            tm = SDK_TARRAY_RE.match(t)
            inner = tm.group(1).strip() if tm else t
            sm = SDK_STRUCT_RE.match(inner)
            if sm and sm.group(1) not in items and sm.group(1) not in d.declared and sm.group(1) in d.structs:
                se = d.structs[sm.group(1)]
                items[sm.group(1)] = SdkItem("struct", sm.group(1), se, se["package"], shim=se["package"] != module)
                for s2 in sdk_referenced_types(se)[0]:
                    if s2 not in items and s2 not in d.declared and s2 in d.structs:
                        items[s2] = SdkItem("struct", s2, d.structs[s2], d.structs[s2]["package"], shim=True)
            pm = SDK_CLASSPTR_RE.match(inner)
            if pm and pm.group(1) not in class_names and pm.group(1) not in d.declared:
                forward.add(pm.group(1))

    # 4. interface vtables: the class inherits the interface; unknown native ones get a placeholder
    for it in list(items.values()):
        if it.kind != "class":
            continue
        for m in it.entry["members"]:
            if m["name"].startswith("VfTable_"):
                iname = m["name"][len("VfTable_"):]
                if iname in items or iname in d.declared:
                    continue
                items[iname] = SdkItem("iface", iname, None, "", shim=True)
                notes.append(f"{it.name}: secondary vtable {iname} is not declared anywhere; a polymorphic placeholder class is generated")

    # 5. functions per class
    for path, f in d.functions.items():
        pkg, stem, func = path.split(".", 2)
        cpp = d.stem_to_cpp.get(stem)
        name = rename.get(cpp, cpp)
        it = items.get(name)
        if it is None or it.kind != "class":
            continue
        it.functions.append((func, f))

    # 6. enums + consts of the module's script classes (native and script-only: they are one namespace);
    #    shim classes contribute only enums the tree does not declare yet
    enums: dict[str, list[str]] = {}
    consts: dict[str, str] = {}
    for sname, sc in d.script.items():
        is_shim_owner = shims and sc["package"] in SHIM_PACKAGES and d.stem_to_cpp.get(sname) in class_names
        if sc["package"] != module and not is_shim_owner:
            continue
        for ch in sc["children"]:
            if ch["kind"] == "Enum":
                if ch["name"] in d.declared_enums:
                    if not is_shim_owner:
                        notes.append(f"enum {ch['name']} of {sname} is already declared by the tree; not emitted")
                    continue
                enums[ch["name"]] = ch["values"]
            elif ch["kind"] == "Const" and not is_shim_owner:
                consts[ch["name"]] = ch["value"]
    # script entries of structs (their ScriptStruct child lists the map properties the dump skipped)
    for it in items.values():
        if it.kind == "struct" and it.entry["path"].count(".") >= 2:
            owner = d.script.get(it.entry["path"].split(".")[1])
            sname = it.entry["path"].split(".")[-1]
            if owner:
                it.script = next((ch for ch in owner["children"] if ch["kind"] == "ScriptStruct" and ch["name"] == sname), None)

    # 7. dependencies, groups, order
    for it in items.values():
        deps: list[str] = []
        if it.kind == "class":
            sup = it.entry.get("super")
            if sup in class_names:
                deps.append(class_names[sup])
            for m in it.entry["members"]:
                if m["name"].startswith("VfTable_") and m["name"][len("VfTable_"):] in items:
                    deps.append(m["name"][len("VfTable_"):])
            within = (it.row or {}).get("within_2013", "UObject")
            if within in class_names:
                deps.append(class_names[within])
            for func, f in it.functions:
                for p in f["params"]:
                    if not p.get("flags", 0) & CPF_PARM:
                        continue
                    tm = SDK_TARRAY_RE.match(p["type"])
                    sm = SDK_STRUCT_RE.match(tm.group(1).strip() if tm else p["type"])
                    if sm and sm.group(1) in items:
                        deps.append(sm.group(1))
        elif it.kind == "iface":
            if it.entry is not None:
                deps.append(rename.get(it.entry["cpp"], it.entry["cpp"]))
                sup = it.entry.get("super")
                if sup in class_names and "I" + uc_stem(sup) in items:
                    deps.append("I" + uc_stem(sup))
        else:
            if it.entry.get("super") in items:
                deps.append(it.entry["super"])
        if it.entry is not None:
            for s in sdk_referenced_types(it.entry)[0]:
                if s in items and s != it.name:
                    deps.append(s)
            # map properties are typed from the 2012 PDB (TMap<K,V> needs complete K/V): depend on the module types they name
            pdb_t = d.pdb.get(it.entry.get("cpp", it.name))
            if pdb_t is not None and it.script is not None:
                maps = {ch["name"] for ch in it.script.get("children", []) if ch["kind"] == "MapProperty"}
                for pm in pdb_data_members(pdb_t):
                    if pm["name"] in maps:
                        for ident in IDENT_RE.findall(fix_type(pm["type"])):
                            if ident in items and ident != it.name and items[ident].kind == "struct":
                                deps.append(ident)
        it.deps = sorted(set(deps) - {it.name})
        if it.kind == "class":
            it.group = "__shims" if it.shim else (it.script or {}).get("header_filename", "") or ""
        elif it.kind == "iface":
            it.group = items[it.deps[0]].group if it.deps and it.deps[0] in items else "__shims"
        else:
            owner = it.entry["path"].split(".")[1] if it.entry["path"].count(".") >= 2 else ""
            owner_cpp = rename.get(d.stem_to_cpp.get(owner), d.stem_to_cpp.get(owner))
            it.group = "__shims" if it.shim else (items[owner_cpp].group if owner_cpp in items else "")
    order = order_by_dependency(sorted(items), items, lambda it: it.deps)

    # group DAG: merge strongly connected groups, then order the groups
    gdeps: dict[str, set] = {}
    for it in items.values():
        gdeps.setdefault(it.group, set())
        for dname in it.deps:
            g2 = items[dname].group
            if g2 != it.group:
                gdeps[it.group].add(g2)
    merged = {g: g for g in gdeps}
    changed = True
    while changed:
        changed = False
        for g in list(gdeps):
            for g2 in list(gdeps[g]):
                if g2 == g:
                    continue
                # reachable back?
                stack, seen = [g2], set()
                while stack:
                    x = stack.pop()
                    if x in seen:
                        continue
                    seen.add(x)
                    stack.extend(gdeps.get(x, ()))
                if g in seen:
                    keep, drop = sorted([g, g2], key=lambda n: (n != "", n))
                    for it in items.values():
                        if it.group == drop:
                            it.group = keep
                    gdeps[keep] = (gdeps[keep] | gdeps[drop]) - {keep, drop}
                    del gdeps[drop]
                    for k in gdeps:
                        if drop in gdeps[k]:
                            gdeps[k] = (gdeps[k] - {drop}) | ({keep} if k != keep else set())
                    changed = True
                    break
            if changed:
                break
    gorder: list[str] = []
    seen_g: set = set()

    def visit_g(g: str) -> None:
        if g in seen_g:
            return
        seen_g.add(g)
        for g2 in sorted(gdeps.get(g, ())):
            visit_g(g2)
        gorder.append(g)
    for g in sorted(gdeps, key=lambda n: (n != "__shims", n != "", n)):
        visit_g(g)
    groups = [(g, [n for n in order if items[n].group == g]) for g in gorder]
    first = gorder[0] if gorder else ""
    group_deps = {g: [x for x in gorder if x != g and (x in gdeps.get(g, ()) or x == first)] for g in gorder}
    stats = {"classes": sum(1 for it in items.values() if it.kind == "class" and not it.shim),
             "shim_classes": sum(1 for it in items.values() if it.kind == "class" and it.shim),
             "structs": sum(1 for it in items.values() if it.kind == "struct" and not it.shim),
             "shim_structs": sum(1 for it in items.values() if it.kind == "struct" and it.shim),
             "interfaces": sum(1 for it in items.values() if it.kind == "iface"),
             "functions": sum(len(it.functions) for it in items.values() if it.kind == "class"),
             "enums": len(enums), "consts": len(consts), "skipped": len(skipped)}
    return SdkSelection(module, items, order, groups, enums, consts, sorted(forward), rename, skipped, notes, stats, group_deps)


# ---------------------------------------------------------------------------------------------
# Layout emission


class LayoutEmitter:
    def __init__(self, data: SdkData, sel: SdkSelection):
        self.d = data
        self.sel = sel
        self.unknown_total = 0
        self.unknown_named_script = 0
        self.unknown_named_pdb = 0
        self.tails = 0
        self.unresolved_types: list[str] = []
        self.forward_natives: set = set()

    def spell(self, t: str, inner: bool = False) -> str | None:
        r = sdk_cpp_type(t, inner)
        if r is None:
            return None
        for old, new in self.sel.rename.items():
            r = re.sub(rf"\b{old}\b", new, r)
        return r

    def pdb_text(self, pdb_m: dict) -> str:
        """2012 PDB declaration; native struct/class names it mentions that nothing declares get a forward declaration"""
        text = pdb_decl(pdb_m)
        for ident in IDENT_RE.findall(text):
            if re.match(r"^[FUAI][A-Z]\w*$", ident) and ident not in self.sel.items and ident not in self.d.declared and ident not in self.d.structs \
                    and ident not in self.d.classes and ident not in SDK_BUILTIN_STRUCTS and ident not in KNOWN_CORE_TYPES:
                self.forward_natives.add(ident)
        return text

    def member_decl(self, m: dict, pdb_m: dict | None) -> tuple[str, str]:
        """(declaration text, note)"""
        if m.get("bitfield"):
            return f"BITFIELD {safe_ident(m['name'])}:1;", ""
        ty = self.spell(m["type"])
        if ty is None or "Mirror" in m["type"]:
            if pdb_m is not None and (pdb_m.get("size") or 0) == m["size"]:
                return self.pdb_text(pdb_m), " // DISHONORED(layout): type from the 2012 PDB (SDK: %s)" % m["type"]
            self.unresolved_types.append(f"{m['name']}:{m['type']}")
            return f"BYTE {safe_ident(m['name'])}[{m['size']}];", f" // DISHONORED(layout): unresolved SDK type {m['type']}, {m['size']} bytes"
        suffix = f"[{m['count']}]" if m.get("count") else ""
        return f"{ty} {safe_ident(m['name'])}{suffix};", ""

    def merge_interfaces(self, members: list[dict], script: dict | None) -> list[dict]:
        """the dump splits an interface property into X_Object/X_Interface pointers: fold them back"""
        iface_names = {ch["name"] for ch in (script or {}).get("children", []) if ch["kind"] == "InterfaceProperty"}
        by_name = {m["name"]: m for m in members}
        out = []
        for m in members:
            n = m["name"]
            if n.endswith("_Object") and n[:-7] in iface_names and (n[:-7] + "_Interface") in by_name:
                cls = m["type"]
                out.append({**m, "name": n[:-7], "type": f"struct FScriptInterface", "size": 8, "iface_class": cls})
                continue
            if n.endswith("_Interface") and n[:-10] in iface_names and (n[:-10] + "_Object") in by_name:
                continue
            out.append(m)
        return out

    def script_props(self, script: dict | None) -> list[dict]:
        return [ch for ch in (script or {}).get("children", []) if ch["kind"].endswith("Property")]

    def emit_members(self, name: str, entry: dict, size: int | None, kind: str, script: dict | None, pdb_t: dict | None, indent: str = "    ") -> tuple[list[str], list[str]]:
        """returns (member lines, secondary base class names)"""
        lines: list[str] = []
        bases: list[str] = []
        members = self.merge_interfaces(entry["members"], script)
        pdb_members = pdb_data_members(pdb_t)
        pdb_index = {m["name"]: i for i, m in enumerate(pdb_members)}
        sprops = self.script_props(script)
        sdk_names = {m["name"] for m in members}
        sindex = {p["name"]: i for i, p in enumerate(sprops)}
        span_start = entry.get("span_start", 0)
        cursor = sdk_align(span_start, 4) if kind == "class" else span_start
        unknown_n = 0
        prev_name: str | None = None
        emitted: set = set()

        def fill(start: int, end: int, next_name: str | None, next_align: int) -> None:
            nonlocal unknown_n, cursor
            gap = end - start
            if gap <= 0:
                cursor = end
                return
            if gap < next_align and end % next_align == 0:
                cursor = end  # alignment padding the compiler inserts itself
                return
            # (a) retail package properties the dump skipped (map properties) between the same neighbours
            lo = sindex.get(prev_name, -1) + 1 if prev_name else 0
            hi = sindex.get(next_name, len(sprops)) if next_name else len(sprops)
            cand = [p for p in sprops[lo:hi] if p["name"] not in sdk_names] if lo <= hi else []
            astart = sdk_align(start, 4)
            if cand and all(p["kind"] == "MapProperty" for p in cand) and astart + MAP_PROPERTY_SIZE * len(cand) == end:
                for p in cand:
                    pm = pdb_members[pdb_index[p["name"]]] if p["name"] in pdb_index else None
                    if pm is not None and (pm.get("size") or 0) == MAP_PROPERTY_SIZE:
                        lines.append(f"{indent}{self.pdb_text(pm)}  // DISHONORED(layout): retail SDK gap @{astart}: MapProperty {p['name']} (script_classes_2013), type from the 2012 PDB")
                        self.unknown_named_pdb += 1
                    else:
                        lines.append(f"{indent}BYTE {safe_ident(p['name'])}[{MAP_PROPERTY_SIZE}];  // DISHONORED(layout): retail SDK gap @{astart}: MapProperty {p['name']} (script_classes_2013), key/value types unknown")
                        self.unknown_named_script += 1
                    astart += MAP_PROPERTY_SIZE
                    emitted.add(p["name"])
                cursor = end
                return
            if len(cand) == 1 and cand[0]["kind"] != "MapProperty":
                p = cand[0]
                lines.append(f"{indent}BYTE {safe_ident(p['name'])}[{gap}];  // DISHONORED(layout): retail SDK gap @{start}: {p['kind']} {p['name']} (script_classes_2013) the dump did not type")
                self.unknown_named_script += 1
                cursor = end
                return
            # (b) 2012 PDB members between the same neighbours whose sizes add up to the gap
            lo = pdb_index.get(prev_name, -1) + 1 if prev_name else 0
            hi = pdb_index.get(next_name, len(pdb_members)) if next_name else len(pdb_members)
            pc = [m for m in pdb_members[lo:hi] if m["name"] not in emitted] if lo <= hi else []
            if pc and pdb_size_of(pc) == gap:
                off = start
                seen_bits = set()
                for m in pc:
                    lines.append(f"{indent}{self.pdb_text(m)}  // DISHONORED(layout): native, 2012 PDB @{m['offset']}, retail @{off} (SDK gap)")
                    emitted.add(m["name"])
                    self.unknown_named_pdb += 1
                    if m.get("bits") is not None:
                        if m["offset"] not in seen_bits:
                            seen_bits.add(m["offset"])
                            off += 4
                    else:
                        off += m.get("size") or 0
                cursor = end
                return
            # (c) placeholder
            lines.append(f"{indent}BYTE UnknownData{unknown_n:02d}[{gap}];  // DISHONORED(layout): retail SDK gap @{start} ({gap} bytes of native members; 2012 PDB candidates: "
                         f"{', '.join(m['name'] for m in pc) or 'none'})")
            unknown_n += 1
            self.unknown_total += 1
            cursor = end

        for group in sdk_groups(members):
            first = group[0]
            if first["name"].startswith("VfTable_"):
                bases.append(first["name"][len("VfTable_"):])
                fill(cursor, first["offset"], None, 4)
                cursor = first["offset"] + 4
                continue
            fill(cursor, first["offset"], first["name"], 4 if first.get("bitfield") else sdk_type_align(first["type"], self.d))
            if first.get("bitfield"):
                bit = 1
                k = 0
                for m in sorted(group, key=lambda x: x.get("mask", 0)):
                    while m.get("mask", bit) > bit:
                        lines.append(f"{indent}BITFIELD UnknownBit{unknown_n:02d}_{k}:1;  // DISHONORED(layout): retail SDK bit 0x{bit:x} @{first['offset']} has no dumped property")
                        k += 1
                        bit <<= 1
                    lines.append(f"{indent}BITFIELD {safe_ident(m['name'])}:1;")
                    emitted.add(m["name"])
                    bit <<= 1
                if k:
                    unknown_n += 1
                cursor = first["offset"] + 4
            else:
                m = first
                pdb_m = pdb_members[pdb_index[m["name"]]] if m["name"] in pdb_index else None
                if m.get("iface_class"):
                    icls = m["iface_class"]
                    icpp = re.sub(r"^class (\w+)\*$", r"\1", icls)
                    iname = "I" + uc_stem(icpp)
                    ty = f"TScriptInterface<class {iname}>" if iname in self.sel.items or iname in self.d.declared else "FScriptInterface"
                    lines.append(f"{indent}{ty} {safe_ident(m['name'])};")
                else:
                    text, note = self.member_decl(m, pdb_m)
                    lines.append(f"{indent}{text}{note}")
                emitted.add(m["name"])
                cursor = m["offset"] + m["size"]
            prev_name = group[-1]["name"]
        span_end = entry.get("span_end", cursor)
        fill(cursor, span_end, None, 1)
        if size is not None:
            align = sdk_class_align(entry["cpp"], self.d, set(self.sel.items)) if kind == "class" else sdk_type_align(f"struct {name}", self.d)
            tail = size - sdk_align(span_end, align)
            if tail > 0:
                lo = pdb_index.get(prev_name, -1) + 1 if prev_name else 0
                pc = [m for m in pdb_members[lo:] if m["name"] not in emitted]
                if pc and pdb_size_of(pc) == tail:
                    off = span_end
                    for m in pc:
                        lines.append(f"{indent}{self.pdb_text(m)}  // DISHONORED(layout): native tail, 2012 PDB @{m['offset']}, retail @{off}")
                        off += m.get("size") or (4 if m.get("bits") is not None else 0)
                        self.unknown_named_pdb += 1
                else:
                    lines.append(f"{indent}BYTE UnknownData{unknown_n:02d}[{tail}];  // DISHONORED(layout): native tail: retail sizeof {size} - reflected span end {span_end} (aligned {align}); 2012 PDB candidates: "
                                 f"{', '.join(m['name'] for m in pc) or 'none'}")
                    self.unknown_total += 1
                self.tails += 1
            elif tail < 0:
                self.sel.notes.append(f"{name}: reflected span end {span_end} exceeds the retail sizeof {size}")
        return lines, bases


# ---------------------------------------------------------------------------------------------
# Function wrappers


def param_default(t: str, cpp: str, data: SdkData, sel: SdkSelection) -> str | None:
    if t in ("int32_t", "uint8_t", "float") or re.match(r"^E\w+$", t):
        return "0"
    if t in ("uint32_t", "bool"):
        return "FALSE"
    if t == "class FName":
        return "NAME_None"
    if t == "class FString":
        return 'TEXT("")'
    if SDK_CLASSPTR_RE.match(t):
        return "NULL"
    sm = SDK_STRUCT_RE.match(t)
    if sm:
        s = sm.group(1)
        if s in sel.items or s in data.eventparm_structs:
            return f"{s}(EC_EventParm)"
        return f"{s}()"
    if SDK_TARRAY_RE.match(t):
        return f"{cpp}()"
    return None


def emit_functions(it: SdkItem, data: SdkData, sel: SdkSelection, g: str) -> tuple[list[str], list[str], list[str], list[str]]:
    """(parms structs before the class, wrapper lines inside the class, native exec names, FName list)"""
    before: list[str] = []
    inside: list[str] = []
    natives: list[str] = []
    names: list[str] = []
    is_iface = bool(int(it.row["flags_2013"], 16) & CLASS_INTERFACE) if it.row else False
    stem = it.stem
    for func, f in it.functions:
        flags = f.get("flags") or 0
        params = [p for p in f["params"] if p.get("flags", 0) & CPF_PARM]
        if flags & FUNC_NATIVE and not is_iface:
            natives.append(f"exec{func}")
            names.append(func)
        is_delegate = bool(flags & FUNC_DELEGATE)
        wants_wrapper = (flags & FUNC_EVENT or is_delegate) and not is_iface
        if is_delegate and not any(m["name"] == f"__{func}__Delegate" for m in it.entry["members"]):
            wants_wrapper = False
        if not wants_wrapper:
            continue
        ret = next((p for p in params if p["flags"] & CPF_RETURN), None)
        args = [p for p in params if not p["flags"] & CPF_RETURN]
        ok = True
        decl_parts: list[str] = []
        struct_lines: list[str] = []
        inits: list[str] = []
        for p in params:
            vt = sdk_value_type(p["type"]) if not p.get("bitfield") else "UBOOL"
            if vt is None:
                ok = False
                break
            for old, new in sel.rename.items():
                vt = re.sub(rf"\b{old}\b", new, vt)
            pname = safe_ident(p["name"])
            suffix = f"[{p['count']}]" if p.get("count") else ""
            struct_lines.append(f"    {vt} {pname}{suffix};")
            sm = SDK_STRUCT_RE.match(p["type"])
            if sm and (sm.group(1) in sel.items or sm.group(1) in data.eventparm_structs) and not p.get("count"):
                inits.append(f"{pname}(EC_EventParm)")
        if not ok:
            sel.notes.append(f"{it.name}::{func}: wrapper skipped, unmapped parameter type")
            continue
        parms_name = f"{stem}_event{func}_Parms"
        before += [f"struct {parms_name}", "{"] + struct_lines
        before.append(f"    {parms_name}(EEventParm)")
        if inits:
            before.append("    : " + "\n    , ".join(inits))
        before += ["    {", "    }", "};"]
        # signature
        defaults_ok = True
        for p in args:
            out = bool(p["flags"] & CPF_OUT)
            vt = sdk_value_type(p["type"]) if not p.get("bitfield") else "UBOOL"
            for old, new in sel.rename.items():
                vt = re.sub(rf"\b{old}\b", new, vt)
            pname = safe_ident(p["name"])
            if p.get("count"):
                part = f"{vt}* {pname}"
                defaults_ok = False
            elif out:
                part = f"{vt}& {pname}"
                defaults_ok = defaults_ok and not (p["flags"] & CPF_OPTIONAL)
            elif vt.startswith("TArray<") or vt == "FString":
                part = f"const {vt}& {pname}"
            elif SDK_STRUCT_RE.match(p["type"]) and vt in sel.items:
                part = f"struct {vt} {pname}"  # engine structs are `class` (FVector, UnMath.h): no elaborated specifier for them
            else:
                part = f"{vt} {pname}"
            if p["flags"] & CPF_OPTIONAL and defaults_ok and not out and not p.get("count"):
                dv = param_default("bool" if p.get("bitfield") else p["type"], vt, data, sel)
                if dv is None:
                    defaults_ok = False
                else:
                    part += f"={dv}"
            elif p["flags"] & CPF_OPTIONAL:
                defaults_ok = False
            decl_parts.append(part)
        rt = "void"
        if ret is not None:
            rt = "UBOOL" if ret.get("bitfield") else (sdk_value_type(ret["type"]) or "void")
            for old, new in sel.rename.items():
                rt = re.sub(rf"\b{old}\b", new, rt)
            if SDK_STRUCT_RE.match(ret["type"]) and rt in sel.items:
                rt = "struct " + rt
        prefix = "delegate" if is_delegate else "event"
        inside.append(f"    {rt} {prefix}{func}({','.join(decl_parts)})")
        inside.append("    {")
        inside.append(f"        {parms_name} Parms(EC_EventParm);")
        if ret is not None:
            dv = param_default("bool" if ret.get("bitfield") else ret["type"], rt, data, sel)
            if dv is not None and not SDK_STRUCT_RE.match(ret["type"]) and not SDK_TARRAY_RE.match(ret["type"]):
                inside.append(f"        Parms.ReturnValue={dv};")
        for p in args:
            pname = safe_ident(p["name"])
            if p.get("count"):
                inside.append(f"        appMemcpy(Parms.{pname},{pname},sizeof(Parms.{pname}));")
            elif p.get("bitfield"):
                inside.append(f"        Parms.{pname}={pname} ? FIRST_BITFIELD : FALSE;")
            else:
                inside.append(f"        Parms.{pname}={pname};")
        if is_delegate:
            inside.append(f"        ProcessDelegate({g}_{func},&__{func}__Delegate,&Parms);")
        else:
            inside.append(f"        ProcessEvent(FindFunctionChecked({g}_{func}),&Parms);")
        for p in args:
            if p["flags"] & CPF_OUT and not p.get("count"):
                pname = safe_ident(p["name"])
                inside.append(f"        {pname}=Parms.{pname};")
        if ret is not None:
            inside.append("        return Parms.ReturnValue;")
        inside.append("    }")
        names.append(func)
    return before, inside, natives, names


# ---------------------------------------------------------------------------------------------
# Header / source emission


def sdk_header_name(module: str, group: str) -> str:
    if group == "__shims":
        return f"{module}EngineShims.h"
    return f"{module}{group}Classes.h"


def sdk_guard(module: str, group: str) -> str:
    g = guard_name(module)
    if group == "":
        return g
    if group == "__shims":
        return f"{g}_SHIMS"
    return f"{g}_{guard_name(group)}"


class SdkWriter:
    def __init__(self, data: SdkData, sel: SdkSelection):
        self.d = data
        self.sel = sel
        self.layout = LayoutEmitter(data, sel)
        self.g = guard_name(data.module)
        self.names: set = set()
        self.natives_by_class: dict[str, list[str]] = {}
        self.asserts: list[tuple[str, str, int]] = []   # (type, member or "", expected)
        self.verify: list[str] = []
        self.forwarded: set = set()

    def flush_forwards(self) -> list[str]:
        """forward declarations of native types the 2012 PDB declarations mention (emitted right before their first use)"""
        new = sorted(self.layout.forward_natives - self.forwarded)
        self.forwarded |= set(new)
        return [f"{'struct' if n[:1] == 'F' else 'class'} {n};  // DISHONORED(port): native type named by a 2012 PDB declaration, not declared yet" for n in new]

    def struct_lines(self, it: SdkItem) -> list[str]:
        e = it.entry
        size = sdk_align(e.get("span_end", 0), sdk_type_align(f"struct {it.name}", self.d))  # C++ sizeof = Align(PropertiesSize, MinAlignment)
        members, _ = self.layout.emit_members(it.name, e, size, "struct", it.script, self.d.pdb.get(it.name))
        head = f"struct {it.name}" + (f" : public {e['super']}" if e.get("super") else "")
        if sdk_struct_natural_align(it.name, self.d) < 4:
            head = "MS_ALIGN(4) " + head  # UStruct::Link gives every script struct MinAlignment >= 4; a byte-only struct must match in C++
        note = f"// {e['path']}: retail SDK size {size}" + (" (2012 PDB %d)" % self.d.pdb[it.name]["size"] if it.name in self.d.pdb else "") + (" [shim: %s package]" % e["package"] if it.shim else "")
        lines = self.flush_forwards() + [note, head, "{"] + members
        lines += ["", "    /** Constructors */", f"    {it.name}() {{}}", f"    {it.name}(EEventParm)", "    {", f"        appMemzero(this, sizeof({it.name}));", "    }", "};", ""]
        self.asserts.append((it.name, "", size))
        for m in self.layout.merge_interfaces(e["members"], it.script):
            if not m.get("bitfield") and not m["name"].startswith("VfTable_"):
                self.asserts.append((it.name, safe_ident(m["name"]), m["offset"]))
        return lines

    def iface_lines(self, it: SdkItem) -> list[str]:
        if it.entry is None:
            return [f"// DISHONORED(port): {it.name} is a native interface the retail classes inherit (secondary vtable in the SDK dump); placeholder until ported",
                    f"class {it.name}", "{", "protected:", f"    virtual ~{it.name}() {{}}", "};", ""]
        ucls = self.sel.rename.get(it.entry["cpp"], it.entry["cpp"])
        sup = it.entry.get("super")
        parent = ""
        if sup and sup != "UInterface":
            cand = "I" + uc_stem(sup)
            if cand in self.sel.items or cand in self.d.declared:
                parent = cand
        return [f"class {it.name}" + (f" : public {parent}" if parent else ""), "{", "protected:", f"    virtual ~{it.name}() {{}}", "public:",
                f"    typedef {ucls} UClassType;", "};", ""]

    def pure_virtual_stubs(self, it: SdkItem, bases: list[str], own_methods: str = "") -> list[str]:
        """a class inheriting a tree interface with pure virtuals must implement them to be constructible (InternalConstructor)"""
        out: list[str] = []
        seen: set = set()
        todo = [b for b in bases if b in self.d.pure_virtuals and b not in self.sel.items]
        while todo:
            b = todo.pop(0)
            if b in seen:
                continue
            seen.add(b)
            bs, pures = self.d.pure_virtuals[b]
            todo += [x for x in bs if x in self.d.pure_virtuals]
            for ret, name, params, const in pures:
                if name in seen or re.search(rf"\b{name}\(", own_methods):
                    continue
                seen.add(name)
                sig = f"    virtual {ret} {name}{params}{' const' if const else ''}"
                err = f'appErrorf(TEXT("{self.d.module} native not ported: %s"), TEXT("{it.name}::{name}"));'
                if name.startswith("GetUObjectInterface"):
                    out.append(f"{sig} {{ return this; }}  // DISHONORED(port): {b} interface glue")
                elif ret == "void":
                    out.append(f"{sig} {{ {err} }}  // DISHONORED(port): {b} pure virtual")
                else:
                    out.append(f"{sig} {{ {err} return {{}}; }}  // DISHONORED(port): {b} pure virtual")
        return out

    def class_lines(self, it: SdkItem) -> list[str]:
        e = it.entry
        row = it.row
        flags = int(row["flags_2013"], 16)
        cast = int(row["cast_flags_2013"], 16)
        size = int(row["size_2013"])
        sup = self.sel.rename.get(e["super"], e["super"])
        members, bases = self.layout.emit_members(it.name, e, size, "class", it.script, self.d.pdb.get(e["cpp"]))
        before, inside, natives, names = emit_functions(it, self.d, self.sel, self.g)
        self.names.update(names)
        if natives:
            self.natives_by_class[it.name] = natives
        head = f"class {it.name} : public {sup}" + "".join(f", public {b}" for b in bases)
        note = f"// {e['path']}: retail sizeof {size}, reflected span {e.get('span_start')}..{e.get('span_end')}" \
               + (f" (2012 PDB sizeof {row['size_2012']})" if row.get("size_2012") else " (new in 2013)") \
               + (f" [shim: {row['package_2013']} package]" if it.shim else "") + (" [layout synthesized from the package member list]" if e.get("synthesized") else "")
        lines = self.flush_forwards() + list(before) + [note, head, "{", "public:", f"    //## BEGIN PROPS {it.stem}"] + members + [f"    //## END PROPS {it.stem}", ""]
        lines += inside
        lines += self.pure_virtual_stubs(it, bases, "\n".join(inside))
        for n in natives:
            lines.append(f"    DECLARE_FUNCTION({n});")
        rest = flags & ~(CLASS_ABSTRACT | CLASS_INTRINSIC)
        ftext = class_flag_text(rest)
        if flags & CLASS_INTRINSIC:
            macro = "DECLARE_ABSTRACT_CASTED_CLASS_INTRINSIC" if (flags & CLASS_ABSTRACT and cast) else "DECLARE_ABSTRACT_CLASS_INTRINSIC" if flags & CLASS_ABSTRACT else "DECLARE_CASTED_CLASS_INTRINSIC" if cast else "DECLARE_CLASS_INTRINSIC"
        else:
            macro = "DECLARE_ABSTRACT_CASTED_CLASS" if (flags & CLASS_ABSTRACT and cast) else "DECLARE_ABSTRACT_CLASS" if flags & CLASS_ABSTRACT else "DECLARE_CASTED_CLASS" if cast else "DECLARE_CLASS"
        lines.append(f"    {macro}({it.name},{sup},{ftext},{row['package_2013']}" + (f",0x{cast:x}" if cast else "") + ")")
        within = row.get("within_2013", "UObject")
        if within and within != "UObject":
            lines.append(f"    DECLARE_WITHIN({self.sel.rename.get(within, within)})")
        sup_row = self.d.sizes.get(e["super"])
        if row.get("config_2013") and (sup_row is None or sup_row.get("config_2013") != row["config_2013"]):
            lines.append(f"    static const TCHAR* StaticConfigName() {{return TEXT(\"{row['config_2013']}\");}}")
        if (SDK_SOURCE / self.d.module / "Inc" / "CppText" / f"{it.name}.h").exists():
            lines.append(f"#include \"CppText/{it.name}.h\"")
        lines += ["};", ""]
        self.asserts.append((it.name, "", size))
        merged = self.layout.merge_interfaces(e["members"], it.script)
        plain = [m for m in merged if not m.get("bitfield") and not m["name"].startswith("VfTable_")]
        for m in plain:
            self.asserts.append((it.name, safe_ident(m["name"]), m["offset"]))
        for m in (plain[:1] + plain[-1:] if len(plain) > 1 else plain):
            self.verify.append(f"VERIFY_CLASS_OFFSET_NODIE({it.name},{it.stem},{safe_ident(m['name'])})")
        self.verify.append(f"VERIFY_CLASS_SIZE_NODIE({it.name})")
        return lines

    def header(self, group: str, names: list[str], first: bool) -> str:
        module = self.d.module
        g = self.g
        gg = sdk_guard(module, group)
        L: list[str] = []
        L += ["/*===========================================================================",
              f"    {sdk_header_name(module, group)} - C++ class definitions of the retail (2013) {module} module.",
              "    Generated by resources/tools/symbols/gen_classes_header.py --sdk from the CodeRed SDK dump",
              "    (retail_sdk_layout.json: member offsets/types/flags, function parameter blocks), native_class_sizes.csv",
              "    (retail sizeof, ClassFlags, Within, config), script_classes_2013.json (enum values, consts, property",
              "    kinds, header groups) and the 2012 types.json (names of native-only members). DO NOT edit by hand:",
              "    regenerate, or carry the change into the generator.",
              "===========================================================================*/",
              "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (push,4)", "#endif", "", f"#include \"{module}Names.h\"", ""]
        deps = self.sel.group_deps.get(group, [])
        if deps:
            # the headers this one depends on (base classes, embedded structs, the enums/forward declarations of the first
            # header); not under the registrant's NAMES_ONLY/NATIVES_ONLY passes, which include every header themselves
            L += ["#ifndef NAMES_ONLY"] + [f'#include "{sdk_header_name(module, g2)}"' for g2 in deps] + ["#endif", ""]
        L += ["#if !NO_ENUMS && !defined(NAMES_ONLY)", "", f"#ifndef INCLUDED_{gg}_ENUMS", f"#define INCLUDED_{gg}_ENUMS 1", ""]
        if first:
            for en, values in sorted(self.sel.enums.items()):
                L += [f"enum {en}", "{"]
                for i, v in enumerate(values):
                    L.append(f"    {v:<24}={i},")
                L.append("};")
                body = [v for v in values if not v.endswith("_MAX")]
                L.append(f"#define FOREACH_ENUM_{en.upper()}(op) \\")
                L += [f"    op({v}) \\" for v in body[:-1]] + ([f"    op({body[-1]}) "] if body else [])
        L += ["", f"#endif // !INCLUDED_{gg}_ENUMS", "#endif // !NO_ENUMS", "", "#if !ENUMS_ONLY", "", "#ifndef NAMES_ONLY", "#define AUTOGENERATE_FUNCTION(cls,idx,name)", "#endif", "", ""]
        L += ["#ifndef NAMES_ONLY", "", f"#ifndef INCLUDED_{gg}_CLASSES", f"#define INCLUDED_{gg}_CLASSES 1", "#define ENABLE_DECLARECLASS_MACRO 1", '#include "UnObjBas.h"', "#undef ENABLE_DECLARECLASS_MACRO", ""]
        if first:
            for cn, cv in sorted(self.sel.consts.items()):
                L.append(f"#define UCONST_{cn} {cv}")
            L.append("")
            fwd = sorted(set(self.sel.forward) | {it.name for it in self.sel.items.values() if it.kind == "class"} | {it.name for it in self.sel.items.values() if it.kind == "iface"})
            L += ["// forward declarations: every class of the module, plus classes referenced by pointer that no included header declares"]
            L += [f"class {n};" for n in fwd]
            L.append("")
        natives_here: list[str] = []
        for n in names:
            it = self.sel.items[n]
            if it.kind == "struct":
                L += self.struct_lines(it)
            elif it.kind == "iface":
                L += self.iface_lines(it)
            else:
                L += self.class_lines(it)
                natives_here.append(n)
        L += ["#undef DECLARE_CLASS", "#undef DECLARE_CASTED_CLASS", "#undef DECLARE_ABSTRACT_CLASS", "#undef DECLARE_ABSTRACT_CASTED_CLASS", f"#endif // !INCLUDED_{gg}_CLASSES", "#endif // !NAMES_ONLY", ""]
        for n in natives_here:
            it = self.sel.items[n]
            for ex in self.natives_by_class.get(n, []):
                idx = self.d.natives13.get((it.entry["cpp"], ex[4:]), "-1")
                L.append(f"AUTOGENERATE_FUNCTION({n},{idx if idx not in ('', '-1') else '-1'},{ex});")
        L += ["", "#ifndef NAMES_ONLY", "#undef AUTOGENERATE_FUNCTION", "#endif", "", "#ifdef STATIC_LINKING_MOJO", f"#ifndef {gg}_NATIVE_DEFS", f"#define {gg}_NATIVE_DEFS", "", f"#define AUTO_INITIALIZE_REGISTRANTS_{gg} \\"]
        for n in natives_here:
            L.append(f"\t{n}::StaticClass(); \\")
            if self.natives_by_class.get(n):
                L.append(f'\tGNativeLookupFuncs.Set(FName("{self.sel.items[n].stem}"), G{module}{n}Natives); \\')
        L += ["", f"#endif // {gg}_NATIVE_DEFS", "", "#ifdef NATIVES_ONLY"]
        for n in natives_here:
            nat = self.natives_by_class.get(n)
            if not nat:
                continue
            L += [f"FNativeFunctionLookup G{module}{n}Natives[] = ", "{ "] + [f"\tMAP_NATIVE({n}, {ex})" for ex in nat] + ["\t{NULL, NULL}", "};", ""]
        L += ["#endif // NATIVES_ONLY", "#endif // STATIC_LINKING_MOJO", "", "#ifdef VERIFY_CLASS_SIZES"]
        L += self.verify
        self.verify = []
        L += ["#endif // VERIFY_CLASS_SIZES", "", "#endif // !ENUMS_ONLY", "", "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (pop)", "#endif", ""]
        return "\n".join(L)

    def names_header(self) -> str:
        g = self.g
        L = ["/*===========================================================================", f"    {self.d.module}Names.h - FName C++ declarations of the retail {self.d.module} module (events, delegates, natives).",
             "    Generated by resources/tools/symbols/gen_classes_header.py --sdk; see the *Classes.h header comment for the inputs.",
             "===========================================================================*/", "#if !ENUMS_ONLY", "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (push,4)", "#endif", "",
             f"#if !defined(__{g}_NAMES_H__) || defined(NAMES_ONLY)", "", f"#ifndef __{g}_NAMES_H__", f"#define __{g}_NAMES_H__", "#endif", "",
             "#ifndef AUTOGENERATE_NAME", "#define DEFINED_NAME_MACRO", f"#define AUTOGENERATE_NAME(name) extern FName {g}_##name;", "#endif", "", ""]
        L += [f"AUTOGENERATE_NAME({n})" for n in sorted(self.names, key=str.lower)]
        L += ["", "#ifdef DEFINED_NAME_MACRO", "#undef DEFINED_NAME_MACRO", "#undef AUTOGENERATE_NAME", "#endif", "", "#endif // HEADER_GUARD", "", "#if SUPPORTS_PRAGMA_PACK", "#pragma pack (pop)", "#endif", "#endif // !ENUMS_ONLY", ""]
        return "\n".join(L)

    def registrants_cpp(self, headers: list[str], main_header: str) -> str:
        module = self.d.module
        g = self.g
        L = [f"// {module}Registrants.cpp - class registration of the retail (2013) {module} module.",
             "// Generated by resources/tools/symbols/gen_classes_header.py --sdk (see the *Classes.h header comment for the inputs).",
             f"// IMPLEMENT_CLASS for every native class of the module, the FName globals, the native lookup tables, and the",
             f"// AutoInitializeRegistrants{module} / AutoGenerateNames{module} / AutoCheckNativeClassSizes{module} hooks Launch calls.",
             f'#include "{main_header}"', "", "#define STATIC_LINKING_MOJO 1", "", "// Register things.", "#define NAMES_ONLY",
             f"#define AUTOGENERATE_NAME(name) FName {g}_##name;", "#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)"]
        # the FName globals are defined by the first header's include of <Module>Names.h only (GameFramework.cpp pattern)
        L += [f'#include "{headers[0]}"', "#undef AUTOGENERATE_NAME"]
        L += [f'#include "{h}"' for h in headers[1:]]
        L += ["#undef AUTOGENERATE_FUNCTION", "#undef NAMES_ONLY", "", "// Register natives.", "#define NATIVES_ONLY", "#define NAMES_ONLY", "#define AUTOGENERATE_NAME(name)", "#define AUTOGENERATE_FUNCTION(cls,idx,name)"]
        L += [f'#include "{h}"' for h in headers]
        L += ["#undef AUTOGENERATE_FUNCTION", "#undef AUTOGENERATE_NAME", "#undef NATIVES_ONLY", "#undef NAMES_ONLY", "", "#if CHECK_NATIVE_CLASS_SIZES", "#if _MSC_VER", '#pragma optimize( "", off )', "#endif", "",
              f"void AutoCheckNativeClassSizes{module}( UBOOL& Mismatch )", "{", "#define NAMES_ONLY", "#define AUTOGENERATE_NAME( name )", "#define AUTOGENERATE_FUNCTION( cls, idx, name )", "#define VERIFY_CLASS_SIZES"]
        L += [f'#include "{h}"' for h in headers]
        L += ["#undef AUTOGENERATE_FUNCTION", "#undef AUTOGENERATE_NAME", "#undef VERIFY_CLASS_SIZES", "#undef NAMES_ONLY", "}", "", "#if _MSC_VER", '#pragma optimize( "", on )', "#endif", "#endif", ""]
        L += [f"void AutoInitializeRegistrants{module}( INT& Lookup )", "{"]
        L += [f"\tAUTO_INITIALIZE_REGISTRANTS_{sdk_guard(module, grp)}" for grp, _ in self.sel.groups]
        L += ["}", "", f"void AutoGenerateNames{module}()", "{", "\t#define NAMES_ONLY", f"\t#define AUTOGENERATE_NAME(name) {g}_##name = FName(TEXT(#name));", f'\t\t#include "{module}Names.h"', "\t#undef AUTOGENERATE_NAME", "", "\t#define AUTOGENERATE_FUNCTION(cls,idx,name)"]
        L += [f'\t#include "{h}"' for h in headers]
        L += ["\t#undef AUTOGENERATE_FUNCTION", "\t#undef NAMES_ONLY", "}", ""]
        for grp, names in self.sel.groups:
            for n in names:
                if self.sel.items[n].kind == "class":
                    L.append(f"IMPLEMENT_CLASS({n});")
        L.append("")
        return "\n".join(L)

    def stubs_cpp(self, main_header: str) -> str:
        module = self.d.module
        L = [f"// {module}NativeStubs.cpp - exec bodies of the retail {module} natives, not ported yet: every one consumes its",
             "// parameters, zeroes its result and warns once (DISHONORED_NATIVE_STUB, Core/Inc/DishonoredNativeStub.h; -strictnatives aborts).",
             "// Generated by resources/tools/symbols/gen_classes_header.py --sdk. Port a native by moving its body into the module's own",
             "// source (and removing it here); the generator skips natives listed in <Module>NativeStubs.ported.txt. C++ declarations a",
             "// port needs go in Inc/CppText/<Class>.h, which the generated class body includes (the cpptext of the retail headers).",
             f'#include "{main_header}"', '#include "DishonoredNativeStub.h"', ""]
        for grp, names in self.sel.groups:
            for n in names:
                for ex in self.natives_by_class.get(n, []):
                    if (n, ex) in self.ported:
                        continue
                    L += [f"void {n}::{ex}( FFrame& Stack, RESULT_DECL )", "{", f"\tDISHONORED_NATIVE_STUB({module}, {n}, {ex});", "}"]
        L.append("")
        return "\n".join(L)

    def auto_pending(self, probe_sizes: dict, probe_offsets: dict | None = None) -> dict:
        """type -> reason for every generated type the probe shows shifted as a whole (an external base or embedded
        engine struct not converged yet); external bases with a probed sizeof different from retail count as well"""
        out = {}
        probe_offsets = probe_offsets or {}
        for it in self.sel.items.values():
            if it.kind not in ("class", "struct") or it.name not in probe_sizes:
                continue
            deltas = set()
            for m in self.layout.merge_interfaces(it.entry["members"], it.script):
                if m.get("bitfield") or m["name"].startswith("VfTable_"):
                    continue
                got = probe_offsets.get((it.name, safe_ident(m["name"])))
                if got is not None:
                    deltas.add(got - m["offset"])
            retail = int(it.row["size_2013"]) if it.kind == "class" else sdk_align(it.entry.get("span_end", 0), sdk_type_align(f"struct {it.name}", self.d))
            size_delta = probe_sizes[it.name] - retail
            if deltas == {0} or (not deltas and size_delta == 0):
                continue
            if len(deltas) == 1 or (not deltas and size_delta):
                shift = next(iter(deltas)) if deltas else size_delta
                root = it.entry.get("super")
                while root in self.sel.items or (root in self.sel.rename and self.sel.rename[root] in self.sel.items):
                    root = (self.d.classes.get(root) or {}).get("super")
                what = f"base {root}" if it.kind == "class" else "an embedded engine struct"
                out[it.name] = f"every reflected member is shifted by {shift:+d} in our tree: {what} is not converged yet (probe)"
        # an engine struct embedded by value whose compiled sizeof differs from the retail element size shifts everything after it
        for it in self.sel.items.values():
            if it.kind not in ("class", "struct") or it.name in out:
                continue
            for m in it.entry["members"]:
                tm = SDK_TARRAY_RE.match(m["type"])
                sm = SDK_STRUCT_RE.match(m["type"]) if not tm else None
                if not sm or sm.group(1) in self.sel.items:
                    continue
                ours = probe_sizes.get(sm.group(1))
                per = m["size"] // (m.get("count") or 1)
                if ours is not None and ours != per:
                    out[it.name] = f"embedded engine struct {sm.group(1)} is {ours} bytes in our tree, {per} in retail (not converged yet)"
                    break
        for it in self.sel.items.values():
            if it.kind != "class" or it.name in out:
                continue
            cpp = it.entry.get("super")
            while cpp:
                if cpp in self.sel.items or (cpp in self.sel.rename and self.sel.rename[cpp] in self.sel.items):
                    cpp = (self.d.classes.get(cpp) or {}).get("super")
                    continue
                ours = probe_sizes.get(cpp)
                retail = self.d.sizes.get(cpp, {}).get("size_2013")
                if ours is not None and retail and ours != int(retail):
                    out[it.name] = f"base {cpp} is {ours} bytes in our tree, {retail} in retail (not converged yet)"
                break
        # propagate: every descendant of a pending class and every type embedding a pending struct is shifted as well
        changed = True
        while changed:
            changed = False
            for it in self.sel.items.values():
                if it.kind not in ("class", "struct") or it.name in out:
                    continue
                sup = it.entry.get("super")
                sup = self.sel.rename.get(sup, sup)
                if it.kind == "class" and sup in out:
                    out[it.name] = f"derives from {sup}: {out[sup]}"
                    changed = True
                    continue
                if it.kind == "struct" and sup in out:
                    out[it.name] = f"derives from {sup}: {out[sup]}"
                    changed = True
                    continue
                for st in sdk_referenced_types(it.entry)[0]:
                    if st in out and st != it.name:
                        out[it.name] = f"embeds {st}: {out[st]}"
                        changed = True
                        break
        return out

    def layouts_header(self, pending: set, probe_sizes: dict | None = None, probe_offsets: dict | None = None) -> str:
        auto = self.auto_pending(probe_sizes or {}, probe_offsets)
        module = self.d.module
        L = [f"// {module}Layouts.h - compile-time layout checks of the retail (2013) {module} classes and script structs:",
             "// sizeof = retail descriptor (native_class_sizes.csv) / retail struct size, member offsets = the offsets the retail",
             "// UStruct::Link computed (CodeRed SDK dump). Generated by resources/tools/symbols/gen_classes_header.py --sdk.",
             "// `pending` lines are known mismatches listed in <Module>Layouts.pending.txt (base classes not converged yet, ...).",
             "// DISHONORED_SDK_LAYOUT_CHECKS (root CMakeLists.txt, per module target) turns them on independently of the Core/Engine PDB asserts.",
             "#pragma once", "#if DISHONORED_LAYOUT_CHECKS || DISHONORED_SDK_LAYOUT_CHECKS", "#include <cstddef>", ""]
        n_ok = n_pending = 0
        for t, m, v in self.asserts:
            key = f"{t}.{m}" if m else t
            if key in pending or t in pending or t in auto:
                why = f" ({auto[t]})" if t in auto and not m else ""
                L.append(f"// pending: {'offsetof(%s, %s) == %d' % (t, m, v) if m else 'sizeof(%s) == %d' % (t, v)}{why}")
                n_pending += 1
            elif m:
                L.append(f"static_assert(offsetof({t}, {m}) == {v}, \"{t}::{m}: retail offset {v}\");")
                n_ok += 1
            else:
                L.append(f"static_assert(sizeof({t}) == {v}, \"{t}: retail sizeof {v}\");")
                n_ok += 1
        L += ["#endif // DISHONORED_LAYOUT_CHECKS || DISHONORED_SDK_LAYOUT_CHECKS", ""]
        self.assert_counts = (n_ok, n_pending)
        self.auto_pending_classes = auto
        return "\n".join(L)


def sdk_module_header(module: str, headers: list[str], deps_headers: list[str]) -> str:
    g = guard_name(module)
    L = [f"// {module}.h - main header of the retail (2013) {module} module: the engine headers its classes derive from,",
         "// then the generated class headers in dependency order. Generated by resources/tools/symbols/gen_classes_header.py --sdk.",
         f"#ifndef _INC_{g}", f"#define _INC_{g}", "", '#include "Engine.h"']
    L += [f'#include "{h}"' for h in deps_headers]
    L += [f'#include "{h}"' for h in headers]
    L += [f'#include "{module}Layouts.h"', "", f"#endif // _INC_{g}", ""]
    return "\n".join(L)


ENGINE_DEP_HEADERS = {
    # headers a module's class declarations need before its own (Engine.h stops at EngineGameEngineClasses.h)
    "GFxUI": ["EngineUserInterfaceClasses.h", "EngineUIPrivateClasses.h", "EngineSequenceClasses.h"],
    "AkAudio": ["EngineSequenceClasses.h", "EngineInterpolationClasses.h", "EngineAudioDeviceClasses.h", "EngineSoundClasses.h"],
    "OnlineSubsystemSteamworks": ["UnIpDrv.h"],
    "DishonoredGame": MODULE_INCLUDES["Engine"][1:] + ["UnIpDrv.h", "GameFrameworkClasses.h", "GameFrameworkAnimClasses.h", "GameFrameworkCameraClasses.h",
                                                      "GameFrameworkSpecialMovesClasses.h", "GameFrameworkGameStatsClasses.h", "gameframeworkdecalclasses.h",
                                                      "GFxUI.h", "AkAudio.h", "OnlineSubsystemSteamworks.h"],
}


def is_comment_only(path: Path) -> bool:
    return all(not l.strip() or l.lstrip().startswith("//") for l in path.read_text(encoding="utf-8", errors="replace").splitlines())


def sdk_write_sources_cmake(module_dir: Path, module: str, generated: list[str]) -> Path:
    src = module_dir / "Src"
    skeletons = sorted(p.name for p in src.glob("*.cpp") if p.name not in generated and is_comment_only(p))
    L = [f"# Generated by resources/tools/symbols/gen_classes_header.py --sdk for {module}.",
         f"# {module}_EXCLUDE: the {len(skeletons)} comment-only skeleton units of resources/tools/import_reference.py (PDB function",
         "# attribution per file, no code) stay out of the build until they are ported; the generated registrant and native-stub units compile.",
         f"set({module}_EXCLUDE"]
    L += [f"  Src/{n}" for n in skeletons]
    L += [")", f"set({module}_NOT_IN_PDB", ")", ""]
    out = module_dir / "Sources.cmake"
    out.write_text("\n".join(L), encoding="utf-8")
    return out


def sdk_main(args: argparse.Namespace) -> int:
    _sdk_imports()
    module = args.module
    data = sdk_load(module)
    # the missing Engine/GameFramework natives are registered once, from the game module
    sel = sdk_select(data, shims=(args.shims or module == "DishonoredGame") and not args.no_shims)
    emu = LinkEmulator(data)
    checked = bad = 0
    emu_notes: list[str] = []
    for it in sel.items.values():
        if it.kind == "class" and not it.entry.get("synthesized"):
            c, b, n = emu.check(it.entry)
            checked += c
            bad += b
            emu_notes += n
    module_dir = SDK_SOURCE / module
    inc = module_dir / "Inc"
    src = module_dir / "Src"
    inc.mkdir(parents=True, exist_ok=True)
    src.mkdir(parents=True, exist_ok=True)
    pending_file = module_dir / f"{module}Layouts.pending.txt"
    pending = {l.strip() for l in pending_file.read_text(encoding="utf-8").splitlines() if l.strip() and not l.startswith("#")} if pending_file.exists() else set()
    probe_sizes: dict = {}
    probe_offsets: dict = {}
    if args.probe:
        for line in Path(args.probe).read_text(encoding="utf-8", errors="replace").splitlines():
            if "," not in line:
                continue
            key, v = line.rsplit(",", 1)
            if v.strip() == "MISSING":
                continue
            if "." in key:
                t, m = key.split(".", 1)
                probe_offsets[(t, m)] = int(v)
            else:
                probe_sizes[key] = int(v)
    ported_file = module_dir / f"{module}NativeStubs.ported.txt"
    writer = SdkWriter(data, sel)
    writer.ported = {tuple(l.strip().split("::")) for l in ported_file.read_text(encoding="utf-8").splitlines() if "::" in l} if ported_file.exists() else set()
    headers = [sdk_header_name(module, grp) for grp, _ in sel.groups]
    texts = {}
    for i, (grp, names) in enumerate(sel.groups):
        texts[sdk_header_name(module, grp)] = writer.header(grp, names, first=(i == 0))
    main_header = f"{module}.h"
    written = []
    if not args.dry_run:
        for h, text in texts.items():
            sdk_write(inc / h, text)
            written.append(h)
        sdk_write(inc / f"{module}Names.h", writer.names_header())
        sdk_write(inc / f"{module}Layouts.h", writer.layouts_header(pending, probe_sizes, probe_offsets))
        if args.module_header or not (inc / main_header).exists():
            sdk_write(inc / main_header, sdk_module_header(module, headers, ENGINE_DEP_HEADERS.get(module, [])))
        sdk_write(src / f"{module}Registrants.cpp", writer.registrants_cpp(headers, main_header))
        sdk_write(src / f"{module}NativeStubs.cpp", writer.stubs_cpp(main_header))
        if args.sources_cmake:
            sdk_write_sources_cmake(module_dir, module, [f"{module}Registrants.cpp", f"{module}NativeStubs.cpp"])
    else:
        writer.layouts_header(pending, probe_sizes, probe_offsets)
    st = sel.stats
    lay = writer.layout
    print(f"{module}: {st['classes']} classes (+{st['shim_classes']} shim classes of other packages), {st['structs']} structs (+{st['shim_structs']} shim structs), "
          f"{st['interfaces']} interfaces, {st['functions']} functions ({sum(len(v) for v in writer.natives_by_class.values())} natives, {len(writer.names)} names), "
          f"{st['enums']} enums, {st['consts']} consts, {len(sel.groups)} headers")
    print(f"  gaps: {lay.unknown_named_script} named from script_classes_2013, {lay.unknown_named_pdb} typed from the 2012 PDB, {lay.unknown_total} UnknownData placeholders; "
          f"{lay.tails} native tails; unresolved member types: {len(lay.unresolved_types)}")
    print(f"  link emulation vs SDK: {checked} members checked, {bad} differ" + (f" ({'; '.join(emu_notes[:5])})" if emu_notes else ""))
    print(f"  layout checks: {writer.assert_counts[0]} static_asserts, {writer.assert_counts[1]} pending"
          + (f"; {len(writer.auto_pending_classes)} types pending on unconverged engine bases (probe)" if writer.auto_pending_classes else ""))
    for n, why in sorted(writer.auto_pending_classes.items()):
        print(f"  pending {n}: {why}")
    for cpp, why in sorted(sel.skipped.items()):
        print(f"  skipped {cpp}: {why}")
    for n in sel.notes[:40]:
        print(f"  note: {n}")
    if len(sel.notes) > 40:
        print(f"  ... {len(sel.notes) - 40} more notes")
    if args.report:
        Path(args.report).write_text(json.dumps({"stats": st, "gaps": {"script_named": lay.unknown_named_script, "pdb_named": lay.unknown_named_pdb, "unknown": lay.unknown_total, "tails": lay.tails,
                                                 "unresolved_types": lay.unresolved_types}, "emulation": {"checked": checked, "bad": bad, "notes": emu_notes},
                                                 "asserts": writer.assert_counts, "auto_pending": writer.auto_pending_classes, "skipped": sel.skipped, "notes": sel.notes, "groups": [(g, len(n)) for g, n in sel.groups],
                                                 "headers": headers}, indent=1), encoding="utf-8")
    return 0


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("module")
    ap.add_argument("--sdk", action="store_true", help="retail (2013) headers + registrants from the CodeRed SDK dump (see the --sdk section)")
    ap.add_argument("--no-shims", action="store_true", help="--sdk: do not generate the missing Engine/GameFramework classes the module derives from")
    ap.add_argument("--shims", action="store_true", help="--sdk: generate the missing Engine/GameFramework native classes in this module (default for DishonoredGame only)")
    ap.add_argument("--dry-run", action="store_true", help="--sdk: statistics only, write nothing")
    ap.add_argument("--module-header", action="store_true", help="--sdk: (re)write <Module>.h even when it exists")
    ap.add_argument("--sources-cmake", action="store_true", help="--sdk: rewrite <Module>/Sources.cmake excluding the skeleton units")
    ap.add_argument("--report", default=None, help="--sdk: write the generation statistics as JSON")
    ap.add_argument("--probe", default=None, help="--sdk: layout_probe.txt of a build; classes on an external base whose sizeof differs from retail become `pending` in <Module>Layouts.h")
    ap.add_argument("--out", default=None)
    ap.add_argument("--diff-reference", action="store_true")
    ap.add_argument("--inventory", default=None)
    ap.add_argument("--reference-src", default=str(REFERENCE_SRC))
    ap.add_argument("--dfsdk", default=str(DFSDK_CLASSES))
    args = ap.parse_args(argv[1:])
    if args.sdk:
        return sdk_main(args)
    sym = load_symbols()
    ref = load_reference(Path(args.reference_src))
    uc = load_dfsdk(Path(args.dfsdk))
    sel = select_module(args.module, sym, ref, uc)
    out = Path(args.out) if args.out else REPO / "resources" / "reference" / f"{args.module}Classes.pdb.h"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(emit_header(sel, sym), encoding="utf-8")
    print(f"{args.module}: {len(sel.script_classes)} script classes, {len(sel.native_classes)} native-only, {len(sel.structs)} structs, {len(sel.enums)} enums -> {out}")
    if args.diff_reference:
        print(diff_reference(sel, sym, ref))
    if args.inventory:
        Path(args.inventory).write_text(inventory(sel, sym, uc), encoding="utf-8")
        print(f"inventory -> {args.inventory}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
