"""Dump UDT layouts, vtables and enums from a PDB through the DIA SDK.

Written for the PhysX 2.8.4 header reconstruction (agent AL): `llvm-pdbutil` cannot read the
NVIDIA/Epic PDBs and `resources/tools/ida/export_types.py` needs an IDA database, which the shipped
middleware DLLs do not have. DIA reads them directly.

Usage:
  python resources/tools/pdb/dia_types.py <file.pdb> --udt-re '^Nx' [--json out.json] [--brief]
  python resources/tools/pdb/dia_types.py <file.pdb> --udt NxActor NxScene [--vtable-only]
  python resources/tools/pdb/dia_types.py <file.pdb> --enum-re '^NxShapeFlag'

Output is text by default (one block per type: size, bases, members with offsets, virtual methods
with their vtable slot) and JSON with --json.
"""
import argparse
import json
import re
import sys
from pathlib import Path

import comtypes
import comtypes.client
import ctypes
from comtypes import GUID
from comtypes.server import IClassFactory

MSDIA_CANDIDATES = [
    Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll"),
    Path(r"C:\Program Files\Microsoft Visual Studio\18\Community\DIA SDK\bin\amd64\msdia140.dll"),
    Path(r"C:\Program Files\Microsoft Visual Studio\2022\Professional\DIA SDK\bin\amd64\msdia140.dll"),
    Path(r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\DIA SDK\bin\amd64\msdia140.dll"),
]
CLSID_DIA_SOURCE = GUID("{E6756135-1E65-4D17-8576-610761398C3C}")

SYM_TAG_UDT = 11
SYM_TAG_ENUM = 12
SYM_TAG_FUNCTION_TYPE = 13
SYM_TAG_POINTER_TYPE = 14
SYM_TAG_ARRAY_TYPE = 15
SYM_TAG_BASE_TYPE = 16
SYM_TAG_TYPEDEF = 17
SYM_TAG_BASE_CLASS = 18
SYM_TAG_FUNCTION = 5
SYM_TAG_DATA = 7
NS_NONE = 0
NS_REGEX = 0x8

LOC_IS_THIS_REL = 4
LOC_IS_BIT_FIELD = 6
LOC_IS_STATIC = 1

BASE_TYPES = {
    0: "<notype>", 1: "void", 2: "char", 3: "wchar_t", 4: "signed char", 5: "unsigned char",
    6: "int", 7: "unsigned int", 8: "float", 9: "<bcd>", 10: "bool", 13: "long",
    14: "unsigned long", 25: "<currency>", 26: "<date>", 27: "VARIANT", 28: "<complex>",
    29: "<bit>", 30: "BSTR", 31: "HRESULT", 32: "char16_t", 33: "char32_t",
}


def load_dia():
    for path in MSDIA_CANDIDATES:
        if path.exists():
            break
    else:
        raise FileNotFoundError("msdia140.dll (amd64) not found")
    comtypes.client.GetModule(str(path))
    from comtypes.gen import Dia2Lib

    lib = ctypes.WinDLL(str(path))
    factory = ctypes.POINTER(IClassFactory)()
    lib.DllGetClassObject(ctypes.byref(CLSID_DIA_SOURCE), ctypes.byref(IClassFactory._iid_), ctypes.byref(factory))
    return factory.CreateInstance(None, Dia2Lib.IDiaDataSource)


def iterate(enum):
    for i in range(enum.Count):
        yield enum.Item(i)


def sized_base(sym):
    try:
        bt = sym.baseType
        ln = sym.length
    except (comtypes.COMError, ValueError):
        return "<basetype>"
    name = BASE_TYPES.get(bt, f"<bt{bt}>")
    if bt == 6:
        return {1: "signed char", 2: "short", 4: "int", 8: "__int64"}.get(ln, name)
    if bt == 7:
        return {1: "unsigned char", 2: "unsigned short", 4: "unsigned int", 8: "unsigned __int64"}.get(ln, name)
    if bt == 8:
        return {4: "float", 8: "double"}.get(ln, name)
    if bt == 13:
        return {4: "long", 8: "__int64"}.get(ln, name)
    if bt == 14:
        return {4: "unsigned long", 8: "unsigned __int64"}.get(ln, name)
    return name


def qualifiers(sym):
    try:
        return "const " if sym.constType else ""
    except (comtypes.COMError, ValueError):
        return ""


def type_name(sym, depth=0):
    if sym is None or depth > 8:
        return "?"
    try:
        tag = sym.symTag
    except (comtypes.COMError, ValueError):
        return "?"
    if tag == SYM_TAG_BASE_TYPE:
        return qualifiers(sym) + sized_base(sym)
    if tag in (SYM_TAG_UDT, SYM_TAG_ENUM, SYM_TAG_TYPEDEF):
        try:
            return qualifiers(sym) + (sym.name or "<anon>")
        except (comtypes.COMError, ValueError):
            return "<anon>"
    if tag == SYM_TAG_POINTER_TYPE:
        try:
            inner = type_name(sym.type, depth + 1)
        except (comtypes.COMError, ValueError):
            inner = "void"
        try:
            ref = sym.reference
        except (comtypes.COMError, ValueError):
            ref = False
        return f"{inner}&" if ref else f"{inner}*"
    if tag == SYM_TAG_ARRAY_TYPE:
        try:
            inner = type_name(sym.type, depth + 1)
            count = sym.count
        except (comtypes.COMError, ValueError):
            return "?[]"
        return f"{inner}[{count}]"
    if tag == SYM_TAG_FUNCTION_TYPE:
        try:
            ret = type_name(sym.type, depth + 1)
        except (comtypes.COMError, ValueError):
            ret = "void"
        args = []
        try:
            for a in iterate(sym.findChildren(0, None, NS_NONE)):
                try:
                    args.append(type_name(a.type, depth + 1))
                except (comtypes.COMError, ValueError):
                    args.append("?")
        except (comtypes.COMError, ValueError):
            pass
        return f"{ret}({', '.join(args)})"
    return f"<tag{tag}>"


def dump_udt(sym):
    out = {"name": sym.name or "", "size": sym.length, "bases": [], "members": [], "virtuals": [], "statics": []}
    for child in iterate(sym.findChildren(SYM_TAG_BASE_CLASS, None, NS_NONE)):
        try:
            out["bases"].append({"name": child.name or "", "offset": child.offset,
                                 "virtual": bool(child.virtualBaseClass)})
        except (comtypes.COMError, ValueError):
            pass
    for child in iterate(sym.findChildren(SYM_TAG_DATA, None, NS_NONE)):
        try:
            loc = child.locationType
        except (comtypes.COMError, ValueError):
            continue
        try:
            tn = type_name(child.type)
            tlen = child.type.length
        except (comtypes.COMError, ValueError):
            tn, tlen = "?", 0
        if loc == LOC_IS_THIS_REL:
            out["members"].append({"name": child.name or "", "offset": child.offset, "type": tn, "size": tlen})
        elif loc == LOC_IS_BIT_FIELD:
            out["members"].append({"name": child.name or "", "offset": child.offset, "type": tn,
                                   "bit": child.bitPosition, "bits": child.length})
        elif loc == LOC_IS_STATIC:
            out["statics"].append({"name": child.name or "", "type": tn})
    for child in iterate(sym.findChildren(SYM_TAG_FUNCTION, None, NS_NONE)):
        try:
            if not child.virtual:
                continue
            slot = child.virtualBaseOffset
        except (comtypes.COMError, ValueError):
            continue
        try:
            pure = bool(child.pure)
        except (comtypes.COMError, ValueError):
            pure = False
        this_const = False
        try:
            objptr = child.type.objectPointerType
            this_const = bool(objptr.type.constType)
        except (comtypes.COMError, ValueError, AttributeError):
            pass
        out["virtuals"].append({"slot": slot // 4, "byte": slot, "name": child.name or "",
                                "sig": type_name(child.type), "pure": pure, "const": this_const,
                                "intro": bool(getattr(child, "intro", False))})
    out["members"].sort(key=lambda m: (m["offset"], m.get("bit", 0)))
    out["virtuals"].sort(key=lambda v: v["slot"])
    return out


def dump_enum(sym):
    out = {"name": sym.name or "", "size": sym.length, "values": []}
    for child in iterate(sym.findChildren(SYM_TAG_DATA, None, NS_NONE)):
        try:
            out["values"].append({"name": child.name or "", "value": child.value})
        except (comtypes.COMError, ValueError):
            pass
    return out


def print_udt(u, brief=False, vtable_only=False):
    print(f"struct {u['name']} // sizeof {u['size']}")
    for b in u["bases"]:
        print(f"  : {b['name']} @{b['offset']}{' virtual' if b['virtual'] else ''}")
    if not vtable_only:
        for m in u["members"]:
            if "bits" in m:
                print(f"  @{m['offset']:<5} {m['type']} {m['name']} : {m['bits']} (bit {m['bit']})")
            else:
                print(f"  @{m['offset']:<5} [{m['size']:>5}] {m['type']} {m['name']}")
        for s in u["statics"]:
            print(f"  static {s['type']} {s['name']}")
    if not brief:
        for v in u["virtuals"]:
            print(f"  vt[{v['slot']:>3}] {'=0 ' if v['pure'] else '   '}{v['name']} {v['sig']}")
    print()


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("pdb")
    ap.add_argument("--udt", nargs="*", default=[])
    ap.add_argument("--udt-re", default=None)
    ap.add_argument("--enum", nargs="*", default=[])
    ap.add_argument("--enum-re", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--brief", action="store_true", help="skip the virtual list")
    ap.add_argument("--vtable-only", action="store_true", help="skip the member list")
    ap.add_argument("--list", action="store_true", help="only print matching type names + sizes")
    args = ap.parse_args(argv[1:])

    source = load_dia()
    source.loadDataFromPdb(str(Path(args.pdb).resolve()))
    session = source.openSession()
    scope = session.globalScope

    wanted_udt = set(args.udt)
    udt_re = re.compile(args.udt_re) if args.udt_re else None
    wanted_enum = set(args.enum)
    enum_re = re.compile(args.enum_re) if args.enum_re else None

    seen = {}
    if wanted_udt or udt_re:
        for sym in iterate(scope.findChildren(SYM_TAG_UDT, None, NS_NONE)):
            try:
                name = sym.name or ""
            except (comtypes.COMError, ValueError):
                continue
            if name in seen:
                continue
            if name in wanted_udt or (udt_re and udt_re.search(name)):
                if args.list:
                    seen[name] = {"name": name, "size": sym.length}
                else:
                    seen[name] = dump_udt(sym)
    enums = {}
    if wanted_enum or enum_re:
        for sym in iterate(scope.findChildren(SYM_TAG_ENUM, None, NS_NONE)):
            try:
                name = sym.name or ""
            except (comtypes.COMError, ValueError):
                continue
            if name in enums:
                continue
            if name in wanted_enum or (enum_re and enum_re.search(name)):
                enums[name] = dump_enum(sym)

    if args.json:
        Path(args.json).write_text(json.dumps({"udts": seen, "enums": enums}, indent=1), encoding="utf-8")
        print(f"udts={len(seen)} enums={len(enums)} -> {args.json}", file=sys.stderr)
    else:
        for name in sorted(seen):
            if args.list:
                print(f"{name} {seen[name]['size']}")
            else:
                print_udt(seen[name], args.brief, args.vtable_only)
        for name in sorted(enums):
            e = enums[name]
            print(f"enum {name} // {e['size']} bytes")
            for v in e["values"]:
                print(f"  {v['name']} = {v['value']}")
            print()
        print(f"udts={len(seen)} enums={len(enums)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
