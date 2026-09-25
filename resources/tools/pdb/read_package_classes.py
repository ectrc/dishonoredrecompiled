"""Phase 2b (package I): dump the script class/member tables of Dishonored's cooked packages.

Pipeline: FPackageFileSummary (read_package_summary.read_summary) -> FCompressedChunk table ->
LZO1X (lzo1x.py) -> name / import / export tables -> deserialization of the UField exports
(UClass, UState, UScriptStruct, UFunction, UEnum, UConst, U*Property) following UnClass.cpp /
UnProp.cpp / UnCoreNative.cpp of UE3 10897 with the Arkane deltas of
resources/docs/serialization_delta_core.md. Version gates use the package's own
Ver() (801) / LicenseeVer() (30).

Per-export layout relied on (PC cooked package, loading, Ver 801 / LicenseeVer 30):

  UObject::Serialize   [RF_HasStack: never set on fields] INT NetIndex;
                       tagged script properties (FPropertyTag list ending in "None") unless the
                       export is a UClass
  UField::Serialize    UField* Next
  UStruct::Serialize   UStruct* SuperStruct; UTextBuffer* ScriptText; UField* Children;
                       UTextBuffer* CppText; INT Line; INT TextPos; INT ScriptBytecodeSize;
                       INT ScriptStorageSize (>=639); BYTE Script[ScriptStorageSize]
                       (ScriptText/CppText/Line/TextPos present: PC cook is not "cooked for console")
  UScriptStruct        DWORD StructFlags; tagged defaults ending in "None"
  UState               DWORD ProbeMask; WORD LabelTableOffset; DWORD StateFlags;
                       TMap<FName,UFunction*> FuncMap
  UClass               DWORD ClassFlags; UClass* ClassWithin; FName ClassConfigName;
                       TMap<FName,UComponent*> ComponentNameToDefaultObjectMap;
                       TArray<FImplementedInterface{UClass*,UProperty*}> Interfaces;
                       TArray<FName> DontSortCategories (>=603), HideCategories,
                       AutoExpandCategories, AutoCollapseCategories; UBOOL bForceScriptOrder (>=749);
                       FName m_DropdownCategory (LicenseeVer>=10, Arkane); FString ClassHeaderFilename;
                       FName DLLBindName dummy (>=655); DWORD m_OtherClassFlags (>=796, Arkane);
                       UObject* ClassDefaultObject.   No ClassGroupNames (reference 789).
  UFunction            WORD iNative; BYTE OperPrecedence; DWORD FunctionFlags;
                       [FUNC_Net] WORD RepOffset; FName FriendlyName
  UConst               FString Value
  UEnum                TArray<FName> Names
  UProperty            INT ArrayDim; QWORD PropertyFlags; FName Category; UEnum* ArraySizeEnum;
                       [CPF_Net] WORD RepOffset
    Byte: UEnum* Enum   Object/Component: UClass* PropertyClass   Class: PropertyClass, MetaClass
    Interface: UClass* InterfaceClass   Struct: UScriptStruct* Struct   Array: UProperty* Inner
    Map: Key, Value   Delegate: UFunction* Function, UFunction* SourceDelegate
    Bool/Int/Float/Name/Str: nothing

Usage:
  python read_package_classes.py dump <pkg.upk|dir> [...] --out script_classes_2013.json
      (directories expand to *.upk without _LOC_; the first file that carries a class defines it,
       later copies - forced exports in seek-free level packages - are verified and listed in
       "also_in"; a class's "package" is the outer package of its export, e.g. DishonoredGameContent)
  python read_package_classes.py scan <pkg.upk|dir> [...]      (count UClass exports only)
  python read_package_classes.py delta <a.json> <b.json> --out script_delta.md
  python read_package_classes.py check-uc <json> <uc_dir> [--sample 20]
  python read_package_classes.py check-pdb <json> <types.json> [--sample 20]
"""
import argparse
import json
import random
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from lzo1x import decompress_chunk_block  # noqa: E402
from read_package_summary import Reader, read_summary  # noqa: E402

VER_MOVED_SUPERFIELD_TO_USTRUCT = 756
VER_USTRUCT_SERIALIZE_ONDISK_SCRIPTSIZE = 639
VER_DONTSORTCATEGORIES_ADDED = 603
VER_FORCE_SCRIPT_DEFINED_ORDER_PER_CLASS = 749
VER_SCRIPT_BIND_DLL_FUNCTIONS = 655
VER_DIS_OTHER_CLASS_FLAGS = 796
VER_PROPERTYTAG_BOOL_OPTIMIZATION = 673
VER_BYTEPROP_SERIALIZE_ENUM = 633
LICENSEE_VER_DIS_DROPDOWN_CATEGORY = 10

RF_HASSTACK = 1 << 57
CPF_NET = 0x20
FUNC_NET = 0x40

CLASS_FLAGS = {
    0x1: "Abstract", 0x2: "Compiled", 0x4: "Config", 0x8: "Transient", 0x10: "Parsed", 0x20: "Localized",
    0x40: "SafeReplace", 0x80: "Native", 0x100: "NoExport", 0x200: "Placeable", 0x400: "PerObjectConfig",
    0x800: "NativeReplication", 0x1000: "EditInlineNew", 0x2000: "CollapseCategories", 0x4000: "Interface",
    0x8000: "IsAUProperty", 0x10000: "IsAUObjectProperty", 0x20000: "IsAUBoolProperty", 0x40000: "IsAUState",
    0x80000: "IsAUFunction", 0x100000: "IsAUStructProperty", 0x200000: "HasInstancedProps",
    0x400000: "NeedsDefProps", 0x800000: "HasComponents", 0x1000000: "Hidden", 0x2000000: "Deprecated",
    0x4000000: "HideDropDown", 0x8000000: "Exported", 0x10000000: "Intrinsic", 0x20000000: "NativeOnly",
    0x40000000: "PerObjectLocalized", 0x80000000: "HasCrossLevelRefs",
}
PROPERTY_FLAGS = {
    0x1: "Edit", 0x2: "Const", 0x4: "Input", 0x8: "ExportObject", 0x10: "OptionalParm", 0x20: "Net",
    0x40: "EditFixedSize", 0x80: "Parm", 0x100: "OutParm", 0x200: "SkipParm", 0x400: "ReturnParm",
    0x800: "CoerceParm", 0x1000: "Native", 0x2000: "Transient", 0x4000: "Config", 0x8000: "Localized",
    0x20000: "EditConst", 0x40000: "GlobalConfig", 0x80000: "Component", 0x100000: "AlwaysInit",
    0x200000: "DuplicateTransient", 0x400000: "NeedCtorLink", 0x800000: "NoExport", 0x1000000: "NoImport", 0x2000000: "NoClear",
    0x4000000: "EditInline", 0x10000000: "EditInlineUse", 0x20000000: "Deprecated", 0x40000000: "DataBinding",
    0x80000000: "SerializeText", 0x100000000: "RepNotify", 0x200000000: "Interp", 0x400000000: "NonTransactional",
    0x800000000: "EditorOnly", 0x1000000000: "NotForConsole", 0x2000000000: "RepRetry",
    0x4000000000: "PrivateWrite", 0x8000000000: "ProtectedWrite", 0x10000000000: "ArchetypeProperty",
    0x20000000000: "EditHide", 0x40000000000: "EditTextBox", 0x100000000000: "CrossLevelPassive",
    0x200000000000: "CrossLevelActive",
}
FUNCTION_FLAGS = {
    0x1: "Final", 0x2: "Defined", 0x4: "Iterator", 0x8: "Latent", 0x10: "PreOperator", 0x20: "Singular",
    0x40: "Net", 0x80: "NetReliable", 0x100: "Simulated", 0x200: "Exec", 0x400: "Native", 0x800: "Event",
    0x1000: "Operator", 0x2000: "Static", 0x4000: "HasOptionalParms", 0x8000: "Const", 0x20000: "Public",
    0x40000: "Private", 0x80000: "Protected", 0x100000: "Delegate", 0x200000: "NetServer",
    0x400000: "HasOutParms", 0x800000: "HasDefaults", 0x1000000: "NetClient", 0x2000000: "DLLImport",
    0x4000000: "K2Call", 0x8000000: "K2Override", 0x10000000: "K2Pure",
}
STRUCT_FLAGS = {
    0x1: "Native", 0x2: "Export", 0x4: "HasComponents", 0x8: "Transient", 0x10: "Atomic", 0x20: "Immutable",
    0x40: "StrictConfig", 0x80: "ImmutableWhenCooked", 0x100: "AtomicWhenCooked", 0x200: "DevLoad",
}
STATE_FLAGS = {0x1: "Editable", 0x2: "Auto", 0x4: "Simulated", 0x8: "HasLocals"}

PROPERTY_CLASSES = {
    "ByteProperty", "IntProperty", "BoolProperty", "FloatProperty", "ObjectProperty", "ComponentProperty",
    "ClassProperty", "InterfaceProperty", "NameProperty", "StrProperty", "ArrayProperty", "MapProperty",
    "StructProperty", "DelegateProperty",
}
FIELD_CLASSES = PROPERTY_CLASSES | {"Class", "State", "Function", "ScriptStruct", "Enum", "Const"}


def flag_names(value: int, table: dict) -> list:
    names = [n for bit, n in table.items() if value & bit]
    rest = value & ~sum(table.keys())
    if rest:
        names.append(f"0x{rest:x}")
    return names


def read_chunk_table(data: bytes) -> list:
    r = Reader(data)
    r.u32()
    v = r.u16()
    r.u16()
    r.i32()
    r.fstring()
    r.u32()
    for _ in range(7):
        r.i32()
    if v >= 623:
        r.i32(), r.i32(), r.i32()
    if v >= 584:
        r.i32()
    r.guid()
    for _ in range(r.i32()):
        r.i32(), r.i32(), r.i32()
    r.i32()
    r.i32()
    r.u32()
    return [(r.i32(), r.i32(), r.i32(), r.i32()) for _ in range(r.i32())]


def read_summary_and_chunks(path: Path):
    return read_summary(path), read_chunk_table(path.read_bytes())


class FieldReader:
    def __init__(self, pkg, data: bytes):
        self.pkg = pkg
        self.d = data
        self.o = 0

    def u8(self):
        v = self.d[self.o]
        self.o += 1
        return v

    def u16(self):
        v = struct.unpack_from("<H", self.d, self.o)[0]
        self.o += 2
        return v

    def i32(self):
        v = struct.unpack_from("<i", self.d, self.o)[0]
        self.o += 4
        return v

    def u32(self):
        v = struct.unpack_from("<I", self.d, self.o)[0]
        self.o += 4
        return v

    def u64(self):
        v = struct.unpack_from("<Q", self.d, self.o)[0]
        self.o += 8
        return v

    def fname(self):
        idx, num = struct.unpack_from("<ii", self.d, self.o)
        self.o += 8
        return self.pkg.name_string(idx, num)

    def fstring(self):
        n = self.i32()
        if n == 0:
            return ""
        if n < 0:
            s = self.d[self.o : self.o - 2 * n].decode("utf-16le", "replace")
            self.o -= 2 * n
        else:
            s = self.d[self.o : self.o + n].decode("latin-1")
            self.o += n
        return s.rstrip("\0")

    def objref(self):
        return self.pkg.object_path(self.i32())

    def name_array(self):
        return [self.fname() for _ in range(self.i32())]

    def skip_tagged_properties(self):
        tags = []
        while True:
            name = self.fname()
            if name == "None":
                return tags
            ptype = self.fname()
            size = self.i32()
            index = self.i32()
            extra = None
            if ptype == "StructProperty":
                extra = self.fname()
            elif ptype == "BoolProperty":
                extra = self.u8() if self.pkg.ver >= VER_PROPERTYTAG_BOOL_OPTIMIZATION else self.i32()
            elif ptype == "ByteProperty" and self.pkg.ver >= VER_BYTEPROP_SERIALIZE_ENUM:
                extra = self.fname()
            self.o += size
            tags.append({"name": name, "type": ptype, "size": size, "index": index, "extra": extra})


class Package:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        self.summary = read_summary(path)
        self.chunks = read_chunk_table(self.data)
        self.ver = self.summary["file_version"]
        self.licensee_ver = self.summary["licensee_version"]
        self.name = path.stem
        self.chunk_cache = {}
        self.names = self._read_names()
        self.imports = self._read_imports()
        self.exports = self._read_exports()
        self._path_cache = {}

    def read(self, offset: int, size: int) -> bytes:
        if not self.chunks:
            return self.data[offset : offset + size]
        out = bytearray()
        pos = offset
        end = offset + size
        while pos < end:
            for ci, (u_off, u_size, c_off, c_size) in enumerate(self.chunks):
                if u_off <= pos < u_off + u_size:
                    blob = self.chunk_cache.get(ci)
                    if blob is None:
                        blob = decompress_chunk_block(self.data, c_off)
                        self.chunk_cache[ci] = blob
                    take = min(end, u_off + u_size) - pos
                    out += blob[pos - u_off : pos - u_off + take]
                    pos += take
                    break
            else:
                raise ValueError(f"{self.name}: offset {pos} is not inside any compressed chunk")
        return bytes(out)

    def _read_names(self):
        s = self.summary
        names = []
        end_guess = s["import_offset"] if s["import_offset"] > s["name_offset"] else s["export_offset"]
        blob = self.read(s["name_offset"], max(end_guess - s["name_offset"], 16 * s["name_count"]))
        o = 0
        for _ in range(s["name_count"]):
            n = struct.unpack_from("<i", blob, o)[0]
            o += 4
            if n < 0:
                text = blob[o : o - 2 * n].decode("utf-16le", "replace")
                o += -2 * n
            else:
                text = blob[o : o + n].decode("latin-1")
                o += n
            flags = struct.unpack_from("<Q", blob, o)[0]
            o += 8
            names.append((text.rstrip("\0"), flags))
        return names

    def name_string(self, idx: int, num: int) -> str:
        base = self.names[idx][0]
        return base if num == 0 else f"{base}_{num - 1}"

    def _read_imports(self):
        s = self.summary
        blob = self.read(s["import_offset"], 28 * s["import_count"])
        imports = []
        for i in range(s["import_count"]):
            cp, cpn, cn, cnn, outer, on, onn = struct.unpack_from("<iiiiiii", blob, 28 * i)
            imports.append({
                "class_package": self.name_string(cp, cpn),
                "class_name": self.name_string(cn, cnn),
                "outer_index": outer,
                "name": self.name_string(on, onn),
            })
        return imports

    def _read_exports(self):
        s = self.summary
        blob = self.read(s["export_offset"], s["depends_offset"] - s["export_offset"])
        r = FieldReader(self, blob)
        exports = []
        for i in range(s["export_count"]):
            e = {
                "class_index": r.i32(),
                "super_index": r.i32(),
                "outer_index": r.i32(),
                "name": r.fname(),
                "archetype_index": r.i32(),
                "object_flags": r.u64(),
                "serial_size": r.i32(),
                "serial_offset": r.i32(),
                "export_flags": r.u32(),
            }
            e["generation_net_object_count"] = [r.i32() for _ in range(r.i32())]
            e["package_guid"] = blob[r.o : r.o + 16].hex()
            r.o += 16
            e["package_flags"] = r.u32()
            e["index"] = i
            exports.append(e)
        return exports

    def object_path(self, index: int):
        if index == 0:
            return None
        if index in self._path_cache:
            return self._path_cache[index]
        if index > 0:
            e = self.exports[index - 1]
            outer = self.object_path(e["outer_index"])
            if outer:
                path = f"{outer}.{e['name']}"
            elif self.class_name_of(e) == "Package":
                path = e["name"]
            else:
                path = f"{self.name}.{e['name']}"
        else:
            imp = self.imports[-index - 1]
            outer = self.object_path(imp["outer_index"])
            path = f"{outer}.{imp['name']}" if outer else imp["name"]
        self._path_cache[index] = path
        return path

    def class_name_of(self, e: dict) -> str:
        ci = e["class_index"]
        if ci == 0:
            return "Class"
        if ci > 0:
            return self.exports[ci - 1]["name"]
        return self.imports[-ci - 1]["name"]

    def is_null_export(self, e: dict) -> bool:
        return e["name"] == "None" and e["serial_size"] == 0 and e["class_index"] == 0 and e["outer_index"] == 0

    def field_exports(self):
        return [e for e in self.exports if not self.is_null_export(e) and self.class_name_of(e) in FIELD_CLASSES]

    def deserialize_field(self, e: dict) -> dict:
        kind = self.class_name_of(e)
        r = FieldReader(self, self.read(e["serial_offset"], e["serial_size"]))
        f = {"kind": kind, "name": e["name"], "path": self.object_path(e["index"] + 1), "export": e["index"]}
        if e["object_flags"] & RF_HASSTACK:
            raise ValueError(f"{f['path']}: field export with RF_HasStack")
        f["net_index"] = r.i32()
        if kind != "Class":
            r.skip_tagged_properties()
        f["next"] = r.objref()
        if kind in ("Class", "State", "Function", "ScriptStruct"):
            self._read_struct(r, f)
            if kind == "ScriptStruct":
                f["struct_flags"] = r.u32()
                f["defaults"] = r.skip_tagged_properties()
            elif kind == "Function":
                f["native"] = r.u16()
                f["oper_precedence"] = r.u8()
                f["function_flags"] = r.u32()
                if f["function_flags"] & FUNC_NET:
                    f["rep_offset"] = r.u16()
                f["friendly_name"] = r.fname()
            else:
                f["probe_mask"] = r.u32()
                f["label_table_offset"] = r.u16()
                f["state_flags"] = r.u32()
                f["func_map"] = [[r.fname(), r.objref()] for _ in range(r.i32())]
                if kind == "Class":
                    self._read_class(r, f)
        elif kind == "Enum":
            f["values"] = r.name_array()
        elif kind == "Const":
            f["value"] = r.fstring()
        else:
            self._read_property(r, f, kind)
        f["consumed"] = r.o
        f["serial_size"] = e["serial_size"]
        return f

    def _read_struct(self, r: FieldReader, f: dict):
        if self.ver >= VER_MOVED_SUPERFIELD_TO_USTRUCT:
            f["super"] = r.objref()
        f["script_text"] = r.objref()
        f["children"] = r.objref()
        f["cpp_text"] = r.objref()
        f["line"] = r.i32()
        f["text_pos"] = r.i32()
        f["script_size"] = r.i32()
        f["script_storage_size"] = r.i32() if self.ver >= VER_USTRUCT_SERIALIZE_ONDISK_SCRIPTSIZE else f["script_size"]
        r.o += f["script_storage_size"]

    def _read_class(self, r: FieldReader, f: dict):
        f["class_flags"] = r.u32()
        f["within"] = r.objref()
        f["config_name"] = r.fname()
        f["component_defaults"] = [[r.fname(), r.objref()] for _ in range(r.i32())]
        f["interfaces"] = [[r.objref(), r.objref()] for _ in range(r.i32())]
        if self.ver >= VER_DONTSORTCATEGORIES_ADDED:
            f["dont_sort_categories"] = r.name_array()
        f["hide_categories"] = r.name_array()
        f["auto_expand_categories"] = r.name_array()
        f["auto_collapse_categories"] = r.name_array()
        f["force_script_order"] = r.i32() if self.ver >= VER_FORCE_SCRIPT_DEFINED_ORDER_PER_CLASS else 0
        if self.licensee_ver >= LICENSEE_VER_DIS_DROPDOWN_CATEGORY:
            f["dropdown_category"] = r.fname()
        f["header_filename"] = r.fstring()
        if self.ver >= VER_SCRIPT_BIND_DLL_FUNCTIONS:
            f["dll_bind_name"] = r.fname()
        f["other_class_flags"] = r.u32() if self.ver >= VER_DIS_OTHER_CLASS_FLAGS else 0
        f["default_object"] = r.objref()

    def _read_property(self, r: FieldReader, f: dict, kind: str):
        f["array_dim"] = r.i32()
        f["property_flags"] = r.u64()
        f["category"] = r.fname()
        f["array_size_enum"] = r.objref()
        if f["property_flags"] & CPF_NET:
            f["rep_offset"] = r.u16()
        if kind == "ByteProperty":
            f["enum"] = r.objref()
        elif kind in ("ObjectProperty", "ComponentProperty"):
            f["class"] = r.objref()
        elif kind == "ClassProperty":
            f["class"] = r.objref()
            f["meta_class"] = r.objref()
        elif kind == "InterfaceProperty":
            f["class"] = r.objref()
        elif kind == "StructProperty":
            f["struct"] = r.objref()
        elif kind == "ArrayProperty":
            f["inner"] = r.objref()
        elif kind == "MapProperty":
            f["key"] = r.objref()
            f["value"] = r.objref()
        elif kind == "DelegateProperty":
            f["function"] = r.objref()
            f["source_delegate"] = r.objref()


def build_tables(pkg: Package, verbose: bool = True) -> dict:
    fields = {}
    mismatches = []
    for e in pkg.field_exports():
        f = pkg.deserialize_field(e)
        if f["consumed"] != f["serial_size"]:
            mismatches.append((f["path"], f["kind"], f["consumed"], f["serial_size"]))
        fields[f["path"]] = f
    if verbose:
        print(f"{pkg.name}: {len(pkg.exports)} exports, {len(fields)} field exports, {len(mismatches)} size mismatches", file=sys.stderr)
        for m in mismatches[:10]:
            print(f"  mismatch {m}", file=sys.stderr)
    by_outer = {}
    for f in fields.values():
        by_outer.setdefault(f["path"].rsplit(".", 1)[0], []).append(f)

    def children_of(f):
        out = []
        cur = f.get("children")
        seen = set()
        while cur and cur not in seen:
            seen.add(cur)
            c = fields.get(cur)
            if c is None:
                out.append({"kind": "?", "name": cur.rsplit(".", 1)[-1], "unresolved": True})
                break
            out.append(describe(c))
            cur = c.get("next")
        return out

    def describe(f):
        k = f["kind"]
        d = {"name": f["name"], "kind": k}
        if k in PROPERTY_CLASSES:
            d["array_dim"] = f["array_dim"]
            d["flags"] = f"0x{f['property_flags']:x}"
            d["flag_names"] = flag_names(f["property_flags"], PROPERTY_FLAGS)
            if f["category"] != "None":
                d["category"] = f["category"]
            for key in ("array_size_enum", "rep_offset", "enum", "class", "meta_class", "struct", "key", "value", "function", "source_delegate"):
                if f.get(key) is not None:
                    d[key] = f[key]
            if k == "ArrayProperty":
                inner = fields.get(f["inner"])
                d["inner"] = describe(inner) if inner else f["inner"]
        elif k == "Function":
            d["flags"] = f"0x{f['function_flags']:x}"
            d["flag_names"] = flag_names(f["function_flags"], FUNCTION_FLAGS)
            d["native"] = f["native"]
            d["oper_precedence"] = f["oper_precedence"]
            if "rep_offset" in f:
                d["rep_offset"] = f["rep_offset"]
            if f["friendly_name"] != f["name"]:
                d["friendly_name"] = f["friendly_name"]
            d["super"] = f.get("super")
            d["script_size"] = f["script_size"]
            d["children"] = children_of(f)
        elif k == "ScriptStruct":
            d["super"] = f.get("super")
            d["flags"] = f"0x{f['struct_flags']:x}"
            d["flag_names"] = flag_names(f["struct_flags"], STRUCT_FLAGS)
            d["children"] = children_of(f)
            d["defaults"] = [t["name"] for t in f["defaults"]]
        elif k == "State":
            d["flags"] = f"0x{f['state_flags']:x}"
            d["flag_names"] = flag_names(f["state_flags"], STATE_FLAGS)
            d["probe_mask"] = f"0x{f['probe_mask']:x}"
            d["label_table_offset"] = f["label_table_offset"]
            d["super"] = f.get("super")
            d["script_size"] = f["script_size"]
            d["func_map"] = [n for n, _ in f["func_map"]]
            d["children"] = children_of(f)
        elif k == "Enum":
            d["values"] = f["values"]
        elif k == "Const":
            d["value"] = f["value"]
        return d

    classes = {}
    for f in fields.values():
        if f["kind"] != "Class":
            continue
        c = {
            "name": f["name"],
            "package": f["path"].rsplit(".", 1)[0],
            "source_file": pkg.name,
            "path": f["path"],
            "super": f.get("super"),
            "flags": f"0x{f['class_flags']:x}",
            "flag_names": flag_names(f["class_flags"], CLASS_FLAGS),
            "other_class_flags": f"0x{f['other_class_flags']:x}",
            "within": f["within"],
            "config_name": f["config_name"],
            "interfaces": [i[0] for i in f["interfaces"]],
            "component_defaults": [n for n, _ in f["component_defaults"]],
            "hide_categories": f["hide_categories"],
            "auto_expand_categories": f["auto_expand_categories"],
            "auto_collapse_categories": f["auto_collapse_categories"],
            "dont_sort_categories": f.get("dont_sort_categories", []),
            "force_script_order": f["force_script_order"],
            "dropdown_category": f.get("dropdown_category"),
            "header_filename": f["header_filename"],
            "probe_mask": f"0x{f['probe_mask']:x}",
            "state_flags": f"0x{f['state_flags']:x}",
            "label_table_offset": f["label_table_offset"],
            "script_size": f["script_size"],
            "script_storage_size": f["script_storage_size"],
            "func_map": [n for n, _ in f["func_map"]],
            "default_object": f["default_object"],
            "export_object_flags": f"0x{pkg.exports[f['export']]['object_flags']:x}",
            "children": children_of(f),
        }
        classes[f["name"]] = c
    return {
        "package": pkg.name,
        "file": str(pkg.path),
        "file_version": pkg.ver,
        "licensee_version": pkg.licensee_ver,
        "engine_version": pkg.summary["engine_version"],
        "cooked_content_version": pkg.summary["cooked_content_version"],
        "export_count": len(pkg.exports),
        "null_export_count": sum(pkg.is_null_export(e) for e in pkg.exports),
        "field_export_count": len(fields),
        "size_mismatches": [list(m) for m in mismatches],
        "classes": classes,
    }


def class_signature(c: dict):
    def member(m):
        inner = m.get("inner")
        return (m["name"], m["kind"], m.get("flags"), m.get("array_dim"), m.get("native"), tuple(m.get("values", [])),
                m.get("enum") or m.get("struct") or m.get("class") or m.get("function"),
                member(inner) if isinstance(inner, dict) else inner,
                tuple(member(x) for x in m.get("children", [])))
    return (c["super"], c["flags"], c["other_class_flags"], c["within"], tuple(member(m) for m in c["children"]))


def cmd_dump(args) -> int:
    result = {"generator": "resources/tools/pdb/read_package_classes.py", "packages": {}, "files": {}, "classes": {}, "copy_mismatches": []}
    seen = set()
    for path in expand_packages(args.packages):
        key = str(path.resolve()).lower()
        if key in seen:
            continue
        seen.add(key)
        try:
            pkg = Package(path)
        except Exception as ex:
            print(f"{path.name}: {ex}", file=sys.stderr)
            continue
        if not any(e["class_index"] == 0 and not pkg.is_null_export(e) for e in pkg.exports):
            continue
        t = build_tables(pkg)
        info = {k: v for k, v in t.items() if k != "classes"}
        info["class_count"] = len(t["classes"])
        new_classes = []
        for name, c in t["classes"].items():
            have = result["classes"].get(name)
            if have is None:
                result["classes"][name] = c
                new_classes.append(name)
                continue
            have.setdefault("also_in", []).append(pkg.name)
            if class_signature(have) != class_signature(c):
                result["copy_mismatches"].append([name, have["source_file"], pkg.name])
        info["new_classes"] = len(new_classes)
        result["files"][pkg.name] = info
        if new_classes:
            result["packages"].setdefault(pkg.name, {"class_count": 0, "classes": []})
            result["packages"][pkg.name]["class_count"] += len(new_classes)
            result["packages"][pkg.name]["classes"] = new_classes
    by_package = {}
    for name, c in result["classes"].items():
        by_package.setdefault(c["package"], []).append(name)
    result["by_package"] = {k: len(v) for k, v in sorted(by_package.items())}
    out = Path(args.out)
    out.write_text(json.dumps(result, indent=1), encoding="utf-8")
    print(f"-> {out}: {len(result['classes'])} classes from {len(result['files'])} files; by package {result['by_package']}; copy mismatches {len(result['copy_mismatches'])}")
    return 0


def expand_packages(paths: list) -> list:
    out = []
    for p in paths:
        path = Path(p)
        if path.is_dir():
            out += sorted(f for f in path.glob("*.upk") if "_LOC_" not in f.name)
        else:
            out.append(path)
    return out


def cmd_scan(args) -> int:
    for p in expand_packages(args.packages):
        try:
            pkg = Package(Path(p))
        except Exception as ex:
            print(f"{Path(p).name}: {ex}")
            continue
        classes = [e["name"] for e in pkg.exports if e["class_index"] == 0]
        print(f"{Path(p).name}: exports={len(pkg.exports)} classes={len(classes)} {classes[:8]}")
    return 0


def member_sig(m: dict) -> str:
    k = m["kind"]
    if k in PROPERTY_CLASSES:
        ref = m.get("enum") or m.get("struct") or m.get("class") or m.get("function")
        inner = m.get("inner")
        if isinstance(inner, dict):
            ref = f"array<{member_sig(inner)}>"
        return f"{k}[{m['array_dim']}] {ref or ''} {m['flags']}".strip()
    if k == "Function":
        params = ",".join(member_sig(c) for c in m.get("children", []) if "Parm" in c.get("flag_names", []))
        return f"Function({params}) {m['flags']} native={m['native']}"
    if k == "Enum":
        return "Enum " + ",".join(m["values"])
    if k == "ScriptStruct":
        return "ScriptStruct " + ";".join(c["name"] + ":" + member_sig(c) for c in m.get("children", []))
    if k == "Const":
        return f"Const {m['value']}"
    if k == "State":
        return "State " + ",".join(c["name"] for c in m.get("children", []))
    return k


def cmd_delta(args) -> int:
    a = json.loads(Path(args.a).read_text(encoding="utf-8"))
    b = json.loads(Path(args.b).read_text(encoding="utf-8"))
    ca, cb = a["classes"], b["classes"]
    packages = sorted({c["package"] for c in ca.values()} | {c["package"] for c in cb.values()})
    lines = [f"# Script class delta {Path(args.a).stem} -> {Path(args.b).stem}", "",
             f"Generated by `resources/tools/pdb/read_package_classes.py delta`. A = `{Path(args.a).name}` (2012 Debug tree, {len(ca)} classes from {len(a['files'])} cooked files), "
             f"B = `{Path(args.b).name}` (2013 retail tree, the target, {len(cb)} classes from {len(b['files'])} cooked files). "
             "Classes are grouped by their owning script package (the outer of the class export); `DishonoredGameContent` / `DishonoredGameDLC0xContent` classes only exist as forced exports inside `DishonoredGame.upk` or inside level packages. "
             "Member lists are the on-disk `Children` order: properties in declaration order, functions/structs/enums/consts prepended (reverse declaration order). "
             "`members_added/removed` compare by name; `reordered` means the common members appear in a different order; enum values, iNative and function flags are compared for common members.", "",
             "| package | classes A | classes B | added | removed | changed |", "|---|---:|---:|---:|---:|---:|"]
    per_pkg = {}
    totals = {"added": 0, "removed": 0, "changed": 0, "members_added": 0, "members_removed": 0, "reordered": 0, "enum_changes": 0, "native_changes": 0, "flag_changes": 0, "super_changes": 0}
    details = []
    for pkgname in packages:
        na = {n for n, c in ca.items() if c["package"] == pkgname}
        nb = {n for n, c in cb.items() if c["package"] == pkgname}
        added = sorted(nb - na)
        removed = sorted(na - nb)
        changed = []
        for n in sorted(na & nb):
            diff = class_diff(ca[n], cb[n])
            if diff:
                changed.append((n, diff))
                for d in diff:
                    for key in totals:
                        if d.startswith(key):
                            totals[key] += 1
        per_pkg[pkgname] = (len(na), len(nb), added, removed, changed)
        totals["added"] += len(added)
        totals["removed"] += len(removed)
        totals["changed"] += len(changed)
        lines.append(f"| {pkgname} | {len(na)} | {len(nb)} | {len(added)} | {len(removed)} | {len(changed)} |")
    moved = sorted(n for n in set(ca) & set(cb) if ca[n]["package"] != cb[n]["package"])
    lines += ["", f"Totals: classes added {totals['added']}, removed {totals['removed']}, changed {totals['changed']}; "
              f"members added {totals['members_added']}, members removed {totals['members_removed']}, reorders {totals['reordered']}, "
              f"enum value changes {totals['enum_changes']}, native index changes {totals['native_changes']}, flag changes {totals['flag_changes']}, super changes {totals['super_changes']}.",
              f"Classes that moved package: {len(moved)}" + (": " + ", ".join(moved) if moved else "."), ""]
    for pkgname in packages:
        na_n, nb_n, added, removed, changed = per_pkg[pkgname]
        lines.append(f"## {pkgname}")
        lines.append("")
        if added:
            lines.append(f"### Added classes ({len(added)})")
            lines.append("")
            for n in added:
                c = cb[n]
                lines.append(f"- `{n}` extends `{c['super']}` ({len(c['children'])} members, flags {c['flags']})")
            lines.append("")
        if removed:
            lines.append(f"### Removed classes ({len(removed)})")
            lines.append("")
            for n in removed:
                c = ca[n]
                lines.append(f"- `{n}` extends `{c['super']}` ({len(c['children'])} members)")
            lines.append("")
        if changed:
            lines.append(f"### Changed classes ({len(changed)})")
            lines.append("")
            for n, diff in changed:
                lines.append(f"- `{n}`")
                for d in diff:
                    lines.append(f"  - {d}")
            lines.append("")
    Path(args.out).write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"-> {args.out}")
    return 0


def class_diff(a: dict, b: dict) -> list:
    out = []
    if a["super"] != b["super"]:
        out.append(f"super_changes: `{a['super']}` -> `{b['super']}`")
    if a["flags"] != b["flags"]:
        out.append(f"flag_changes: class flags {a['flags']} -> {b['flags']} ({sorted(set(b['flag_names']) - set(a['flag_names']))} added, {sorted(set(a['flag_names']) - set(b['flag_names']))} removed)")
    if a["other_class_flags"] != b["other_class_flags"]:
        out.append(f"flag_changes: other class flags {a['other_class_flags']} -> {b['other_class_flags']}")
    ma = {m["name"]: m for m in a["children"]}
    mb = {m["name"]: m for m in b["children"]}
    for n in [m["name"] for m in b["children"] if m["name"] not in ma]:
        out.append(f"members_added: `{n}` {member_sig(mb[n])}")
    for n in [m["name"] for m in a["children"] if m["name"] not in mb]:
        out.append(f"members_removed: `{n}` {member_sig(ma[n])}")
    common_a = [m["name"] for m in a["children"] if m["name"] in mb]
    common_b = [m["name"] for m in b["children"] if m["name"] in ma]
    if common_a != common_b:
        out.append(f"reordered: common member order differs (first difference at `{next(x for x, y in zip(common_a, common_b) if x != y)}`)")
    for n in common_a:
        x, y = ma[n], mb[n]
        if x["kind"] != y["kind"]:
            out.append(f"members_changed: `{n}` kind {x['kind']} -> {y['kind']}")
            continue
        if x["kind"] == "Enum":
            if x["values"] != y["values"]:
                out.append(f"enum_changes: `{n}` {enum_diff(x['values'], y['values'])}")
        elif x["kind"] == "Function":
            if x["native"] != y["native"]:
                out.append(f"native_changes: `{n}` iNative {x['native']} -> {y['native']}")
            if x["flags"] != y["flags"]:
                out.append(f"flag_changes: `{n}` function flags {x['flags']} -> {y['flags']}")
            sx = [(c["name"], member_sig(c)) for c in x["children"] if "Parm" in c.get("flag_names", [])]
            sy = [(c["name"], member_sig(c)) for c in y["children"] if "Parm" in c.get("flag_names", [])]
            if sx != sy:
                out.append(f"members_changed: `{n}` params {[p for p, _ in sx]} -> {[p for p, _ in sy]}")
        elif x["kind"] == "ScriptStruct":
            sub = class_diff({"super": x["super"], "flags": x["flags"], "flag_names": x["flag_names"], "other_class_flags": 0, "children": x["children"]},
                             {"super": y["super"], "flags": y["flags"], "flag_names": y["flag_names"], "other_class_flags": 0, "children": y["children"]})
            for s in sub:
                out.append(f"{s.split(':', 1)[0]}: struct `{n}`: {s.split(':', 1)[1].strip()}")
        elif x["kind"] in PROPERTY_CLASSES:
            if member_sig(x) != member_sig(y):
                out.append(f"members_changed: `{n}` {member_sig(x)} -> {member_sig(y)}")
        elif x["kind"] == "Const" and x["value"] != y["value"]:
            out.append(f"members_changed: const `{n}` {x['value']!r} -> {y['value']!r}")
    return out


def enum_diff(a: list, b: list) -> str:
    sa, sb = set(a), set(b)
    parts = []
    if sb - sa:
        parts.append(f"added {[v for v in b if v not in sa]}")
    if sa - sb:
        parts.append(f"removed {[v for v in a if v not in sb]}")
    common = [v for v in a if v in sb]
    if common != [v for v in b if v in sa]:
        parts.append("reordered")
    moved = [(v, a.index(v), b.index(v)) for v in common if a.index(v) != b.index(v)]
    if moved:
        parts.append(f"{len(moved)} values renumbered (first `{moved[0][0]}` {moved[0][1]}->{moved[0][2]})")
    return "; ".join(parts) or "identical"


def uc_property_names(text: str) -> list:
    names = []
    depth = 0
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("//") or not s:
            continue
        if s.startswith("struct ") or s.startswith("struct("):
            depth += 1
        if depth and s.startswith("};"):
            depth -= 1
            continue
        if depth:
            continue
        if s.startswith("defaultproperties") or s.startswith("function") or s.startswith("event") or s.startswith("native function") or s.startswith("simulated") or s.startswith("state"):
            break
        m = re.match(r"^var(?:\([^)]*\))?\s+(.*?);", s)
        if not m:
            continue
        decl = m.group(1)
        decl = re.sub(r"<[^>]*>", "", decl)
        decl = re.sub(r"\{[^}]*\}", "", decl)
        decl = re.sub(r"\barray<.*?>", "array", decl)
        toks = [t.strip() for t in re.split(r",", decl)]
        first = toks[0].split()
        if len(first) < 2:
            continue
        names.append(re.sub(r"\[.*\]", "", first[-1]))
        for t in toks[1:]:
            t = t.split()[0] if t.split() else ""
            if t:
                names.append(re.sub(r"\[.*\]", "", t))
    return names


def cmd_check_uc(args) -> int:
    data = json.loads(Path(args.json).read_text(encoding="utf-8"))["classes"]
    files = sorted(Path(args.uc_dir).glob("*.uc"))
    rnd = random.Random(args.seed)
    candidates = [f for f in files if f.stem in data and len(data[f.stem]["children"]) >= 3]
    sample = rnd.sample(candidates, min(args.sample, len(candidates)))
    ok = 0
    ordered = 0
    for f in sample:
        uc = uc_property_names(f.read_text(encoding="utf-8", errors="replace"))
        pkgprops = [m["name"] for m in data[f.stem]["children"] if m["kind"] in PROPERTY_CLASSES]
        match = uc == pkgprops
        subsequence = [n for n in pkgprops if n in set(uc)] == uc
        ok += match
        ordered += subsequence
        status = "OK  " if match else ("SUB " if subsequence else "DIFF")
        print(f"{status} {f.stem}: uc={len(uc)} pkg={len(pkgprops)}" + ("" if match else f" not in uc: {[n for n in pkgprops if n not in uc]}"))
        if not subsequence:
            print(f"    uc : {uc}")
            print(f"    pkg: {pkgprops}")
    print(f"{ok}/{len(sample)} exact, {ordered}/{len(sample)} same order (uc is a subsequence of the package list)")
    return 0


def cmd_check_pdb(args) -> int:
    data = json.loads(Path(args.json).read_text(encoding="utf-8"))["classes"]
    types = {t["name"]: t for t in json.loads(Path(args.types).read_text(encoding="utf-8"))["types"]}
    rnd = random.Random(args.seed)
    candidates = []
    for name, c in data.items():
        props = [m for m in c["children"] if m["kind"] in PROPERTY_CLASSES]
        if len(props) < 3:
            continue
        for prefix in ("U", "A", "F"):
            if prefix + name in types and types[prefix + name]["members"]:
                candidates.append((name, prefix + name))
                break
    sample = rnd.sample(candidates, min(args.sample, len(candidates)))
    ok = 0
    for name, tname in sample:
        pdb = [m["name"] for m in types[tname]["members"] if not m["is_base"] and not m["is_vftable"]]
        bases = [m["type"] for m in types[tname]["members"] if m["is_base"]]
        props = [m for m in data[name]["children"] if m["kind"] in PROPERTY_CLASSES]
        pkgprops = [m["name"] for m in props]
        expected_absent = [m["name"] for m in props if "EditorOnly" in m["flag_names"] or (m["name"].startswith("VfTable_I") and m["name"][8:] in bases)]
        order = [n for n in pdb if n in pkgprops]
        missing = [n for n in pkgprops if n not in pdb and n not in expected_absent]
        match = order == [n for n in pkgprops if n in pdb] and not missing
        ok += match
        print(f"{'OK  ' if match else 'DIFF'} {name} ({tname}): pkg={len(pkgprops)} pdb={len(pdb)} missing_in_pdb={missing[:6]} editoronly_or_interface={expected_absent[:6]}")
        if not match and not missing:
            print(f"    pdb order: {order}")
            print(f"    pkg order: {pkgprops}")
    print(f"{ok}/{len(sample)} match")
    return 0


def main(argv: list) -> int:
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    d = sub.add_parser("dump")
    d.add_argument("packages", nargs="+")
    d.add_argument("--out", required=True)
    s = sub.add_parser("scan")
    s.add_argument("packages", nargs="+")
    dl = sub.add_parser("delta")
    dl.add_argument("a")
    dl.add_argument("b")
    dl.add_argument("--out", required=True)
    cu = sub.add_parser("check-uc")
    cu.add_argument("json")
    cu.add_argument("uc_dir")
    cu.add_argument("--sample", type=int, default=20)
    cu.add_argument("--seed", type=int, default=1)
    cp = sub.add_parser("check-pdb")
    cp.add_argument("json")
    cp.add_argument("types")
    cp.add_argument("--sample", type=int, default=20)
    cp.add_argument("--seed", type=int, default=1)
    args = ap.parse_args(argv[1:])
    return {"dump": cmd_dump, "scan": cmd_scan, "delta": cmd_delta, "check-uc": cmd_check_uc, "check-pdb": cmd_check_pdb}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
