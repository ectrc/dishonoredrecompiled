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


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("module")
    ap.add_argument("--out", default=None)
    ap.add_argument("--diff-reference", action="store_true")
    ap.add_argument("--inventory", default=None)
    ap.add_argument("--reference-src", default=str(REFERENCE_SRC))
    ap.add_argument("--dfsdk", default=str(DFSDK_CLASSES))
    args = ap.parse_args(argv[1:])
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
