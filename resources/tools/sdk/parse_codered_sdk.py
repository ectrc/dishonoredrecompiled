"""Parse the CodeRed-Generator SDK dump of the RETAIL exe (D:\\RecompileDishonored\\Dishonored_DumpedSDK_Retail)
into resources/docs/types/retail_sdk_layout.json (+ a short .md summary).

The dump was generated inside the running retail game, so every offset is the value retail
`UStruct::Link` computed for that `UProperty` — the runtime layout truth of the 2013 exe for every
reflected member (script properties, exec-parameter structs, script structs). Native-only C++ members
are invisible to it and show up as `UnknownDataNN` gaps, which are recorded too (they say where the
2012 PDB's native members have to fit in retail).

Known limits of the dump (see resources/docs/sdk_dump.md):
  * class spans end at the last reflected property; the C++ sizeof (native_class_sizes.csv) can be larger
  * `iNative[n]` values are garbage (generator read the wrong UFunction field); use natives_2013.csv
  * static-array members carry their element count in `name[0xN]`; bitfields carry a `[0xMASK]`

Usage: python resources/tools/sdk/parse_codered_sdk.py [--sdk <dir>]
"""
import json
import re
import sys
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
TYPES = REPO / "resources" / "docs" / "types"
DEFAULT_SDK = Path(r"D:\RecompileDishonored\Dishonored_DumpedSDK_Retail")

CLASS_HDR_RE = re.compile(r"^// Class (\S+)$")
STRUCT_HDR_RE = re.compile(r"^// ScriptStruct (\S+)$")
FUNC_HDR_RE = re.compile(r"^// Function (\S+)$")
SPAN_RE = re.compile(r"^// 0x([0-9A-Fa-f]+) \(0x([0-9A-Fa-f]+) - 0x([0-9A-Fa-f]+)\)$")
SIZE_RE = re.compile(r"^// 0x([0-9A-Fa-f]+)$")
FLAGS_RE = re.compile(r"^// \[0x([0-9A-Fa-f]+)\]\s*(?:\((.*?)\))?\s*(?:\(iNative\[(\d+)\]\))?")
DECL_RE = re.compile(r"^(class|struct) ([A-Za-z_]\w*)(?:\s*:\s*public\s+([A-Za-z_]\w*))?\s*$")
MEMBER_RE = re.compile(
    r"^\s+(?P<type>.+?)\s{2,}(?P<name>[A-Za-z_]\w*)(?:\[0x(?P<count>[0-9A-Fa-f]+)\])?(?P<bit>\s*:\s*1)?;\s*"
    r"//\s*0x(?P<off>[0-9A-Fa-f]+)\s*\(0x(?P<size>[0-9A-Fa-f]+)\)\s*(?P<rest>.*)$")
FLAGWORD_RE = re.compile(r"\[0x([0-9A-Fa-f]+)\]")
NAMES_RE = re.compile(r"\(([^()]*)\)\s*$")


def parse_member(m: re.Match) -> dict:
    rest = m.group("rest").strip()
    d = {"name": m.group("name"), "type": m.group("type").strip(), "offset": int(m.group("off"), 16), "size": int(m.group("size"), 16)}
    if m.group("count"):
        d["count"] = int(m.group("count"), 16)
    if "MISSED OFFSET" in rest:
        d["gap"] = True
        return d
    if "ADDED PADDING" in rest:
        d["padding"] = True
        return d
    words = FLAGWORD_RE.findall(rest)
    if words:
        d["flags"] = int(words[0], 16)
    if m.group("bit"):
        d["bitfield"] = True
        if len(words) > 1:
            d["mask"] = int(words[1], 16)
    names = NAMES_RE.search(rest)
    if names:
        d["flag_names"] = [n.strip() for n in names.group(1).split("|") if n.strip()]
    return d


def parse_types(path: Path, kind: str, package: str, out: dict) -> None:
    """kind: 'class' (X_classes.hpp) or 'struct' (X_structs.hpp)."""
    pending_path = None
    pending_span = None
    cur = None
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.rstrip()
        if cur is None:
            hm = (CLASS_HDR_RE if kind == "class" else STRUCT_HDR_RE).match(line)
            if hm:
                pending_path = hm.group(1)
                pending_span = None
                continue
            sm = SPAN_RE.match(line)
            if sm:
                pending_span = (int(sm.group(2), 16), int(sm.group(3), 16))
                continue
            zm = SIZE_RE.match(line)
            if zm and pending_path:
                pending_span = (0, int(zm.group(1), 16))
                continue
            dm = DECL_RE.match(line)
            if dm and pending_path:
                cur = {"cpp": dm.group(2), "path": pending_path, "package": package, "super": dm.group(3), "members": [], "gaps": []}
                if pending_span:
                    cur["span_start"], cur["span_end"] = pending_span
                pending_path = None
            continue
        if line.startswith("};"):
            out[cur["cpp"]] = cur
            cur = None
            continue
        mm = MEMBER_RE.match(line)
        if mm:
            d = parse_member(mm)
            if d.get("gap"):
                cur["gaps"].append({"offset": d["offset"], "size": d["size"]})
            elif not d.get("padding"):
                cur["members"].append(d)
            continue
        if line.startswith("public:") and cur["members"] and "static UClass* StaticClass()" not in line:
            # second `public:` opens the methods block; nothing layout-relevant follows
            continue


def parse_params(path: Path, out: dict) -> None:
    func = None
    flags = None
    cur = None
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.rstrip()
        if cur is None:
            fm = FUNC_HDR_RE.match(line)
            if fm:
                func = fm.group(1)
                flags = None
                continue
            gm = FLAGS_RE.match(line)
            if gm and func:
                flags = int(gm.group(1), 16)
                continue
            if line.startswith("struct ") and func:
                cur = {"path": func, "flags": flags, "params_struct": line.split()[1], "params": []}
                func = None
            continue
        if line.startswith("};"):
            cur["params_size"] = max((p["offset"] + p["size"] for p in cur["params"]), default=0)
            out[cur["path"]] = cur
            cur = None
            continue
        mm = MEMBER_RE.match(line)
        if mm:
            d = parse_member(mm)
            if not d.get("gap") and not d.get("padding"):
                cur["params"].append(d)


def main(argv: list[str]) -> int:
    sdk = DEFAULT_SDK
    if len(argv) > 2 and argv[1] == "--sdk":
        sdk = Path(argv[2])
    src = sdk / "src"
    if not src.is_dir():
        print(f"SDK dump not found at {sdk} (pass --sdk <dir>)", file=sys.stderr)
        return 2
    classes, structs, functions = {}, {}, {}
    packages = sorted({p.name.split("_")[0] for p in src.glob("*_classes.hpp")})
    for pkg in packages:
        parse_types(src / f"{pkg}_classes.hpp", "class", pkg, classes)
        parse_types(src / f"{pkg}_structs.hpp", "struct", pkg, structs)
        parse_params(src / f"{pkg}_parameters.hpp", functions)
    header = (sdk / "include" / "sdk.hpp").read_text(encoding="utf-8", errors="replace").splitlines()[:5]
    data = {"source": str(sdk), "generator": " ".join(l.strip("# ").strip() for l in header[2:4]),
            "classes": classes, "structs": structs, "functions": functions}
    out = TYPES / "retail_sdk_layout.json"
    out.write_text(json.dumps(data, indent=1), encoding="utf-8")
    per_pkg = Counter(c["package"] for c in classes.values())
    per_pkg_s = Counter(s["package"] for s in structs.values())
    per_pkg_f = Counter(f["path"].split(".")[0] for f in functions.values())
    gaps = sum(len(c["gaps"]) for c in classes.values()) + sum(len(s["gaps"]) for s in structs.values())
    members = sum(len(c["members"]) for c in classes.values())
    md = TYPES / "retail_sdk_layout.md"
    with md.open("w", encoding="utf-8") as f:
        f.write("# Retail SDK dump (CodeRed generator) — parsed summary\n\n")
        f.write(f"Source: `{sdk}` ({data['generator']}). Parsed by `resources/tools/sdk/parse_codered_sdk.py` into "
                f"`retail_sdk_layout.json` (git-ignored, seconds to regenerate).\n\n")
        f.write(f"{len(classes)} classes with {members} reflected members, {len(structs)} script structs, "
                f"{len(functions)} functions with exec-parameter layouts, {gaps} native-only gaps (`UnknownDataNN`).\n\n")
        f.write("| Package | Classes | Structs | Functions |\n|---|---:|---:|---:|\n")
        for pkg in packages:
            f.write(f"| {pkg} | {per_pkg.get(pkg, 0)} | {per_pkg_s.get(pkg, 0)} | {per_pkg_f.get(pkg, 0)} |\n")
        f.write("\nWhat it is good for and what it cannot tell: `resources/docs/sdk_dump.md`. "
                "Cross-check of our compiled layouts against it: `retail_sdk_delta.md` (`xcheck_sdk_layout.py`).\n")
    print(f"classes={len(classes)} members={members} structs={len(structs)} functions={len(functions)} gaps={gaps} -> {out.relative_to(REPO)}, {md.relative_to(REPO)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
