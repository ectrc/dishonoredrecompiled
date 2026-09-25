"""Regenerate `//## BEGIN PROPS <Class>` blocks from the RETAIL layout (the CodeRed SDK dump,
resources/docs/types/retail_sdk_layout.json) instead of the 2012 PDB (`gen_layout_probe.py props`).

For every reflected member the dump gives the offset retail's `UStruct::Link` computed; the block is
rebuilt in that order. Native-only regions between reflected members (the dump's `UnknownData` gaps
and implicit holes) are filled by the relative-position rule: the 2012 PDB members that sit between
the same two neighbours are emitted when their sizes add up to the hole, otherwise a
`BYTE UnknownDataNN[size]` placeholder is emitted and listed as unresolved. Reference declaration
text is kept by member name (access specifiers included); reference-only members become
`DISHONORED_SHIM_STATIC` shims after the block (same convention as `props`). Members after the
`//## END PROPS` line (native, hand-written) are not touched: the tool reports the native tail
(retail sizeof - span end) and the 2012 PDB members past the last reflected one so the agent can
check them by hand.

Usage:
  python resources/tools/sdk/sdk_props.py [--dry-run] [--print] Engine UInterpTrackInst [UInterpGroup ...]
      --dry-run  print the block instead of writing the header
      --print    print the generated member lines for classes without a PROPS block (native headers)
  python resources/tools/sdk/sdk_props.py --survey Engine
      classify the retail_sdk_delta.md rows of the module: mechanical (PROPS block) vs hand work
Also reachable as `python resources/tools/symbols/gen_layout_probe.py props --sdk Engine Class...`.
"""
import csv
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
TYPES = REPO / "resources" / "docs" / "types"
sys.path.insert(0, str(REPO / "resources" / "tools" / "symbols"))
from gen_layout_probe import ACCESS_RE, PROPS_BLOCK_RE, SOURCE, load_types, parse_props_block, shim_lines, write_retry  # noqa: E402
from gen_classes_header import declarator, fix_type, uc_stem  # noqa: E402

# SDK spelling -> UE3 header spelling for reflected members (the reference *Classes.h conventions)
SIMPLE_TYPES = {"int32_t": "INT", "uint32_t": "INT", "float": "FLOAT", "uint8_t": "BYTE", "bool": "UBOOL",
                "class FName": "FName", "class FString": "FStringNoInit", "struct FPointer": "FPointer",
                "struct FQWord": "QWORD", "struct FScriptDelegate": "FScriptDelegate", "struct FGuid": "FGuid"}
INNER_TYPES = {"class FString": "FString"}
# mirror structs the generator invents for native containers; used only when neither the reference
# header nor the 2012 PDB has the member (the PDB type is preferred: it names the element type)
MIRROR_FALLBACK = {"struct FRenderCommandFence_Mirror": "FRenderCommandFence", "struct FThreadSafeCounter": "FThreadSafeCounter",
                   "struct FKCachedConvexData_Mirror": "FKCachedConvexData", "struct FUntypedBulkData_Mirror": "FUntypedBulkData"}
ALIGN16 = {"FMatrix", "FBoneAtom", "FVector4", "FPlane", "FQuat", "VectorRegister"}
TARRAY_RE = re.compile(r"^class TArray<(.+)>$")
ENUM_RE = re.compile(r"^E\w+$")


def load_sdk() -> dict[str, dict]:
    d = json.loads((TYPES / "retail_sdk_layout.json").read_text(encoding="utf-8"))
    out = dict(d["structs"])
    out.update(d["classes"])
    return out


def retail_sizes() -> dict[str, int]:
    with (TYPES / "native_class_sizes.csv").open(newline="", encoding="utf-8") as f:
        return {r["class"]: int(r["size_2013"]) for r in csv.DictReader(f) if r.get("size_2013")}


def map_type(t: str, inner: bool = False) -> str | None:
    if inner and t in INNER_TYPES:
        return INNER_TYPES[t]
    if t in SIMPLE_TYPES:
        return SIMPLE_TYPES[t]
    m = TARRAY_RE.match(t)
    if m:
        elem = map_type(m.group(1).strip(), True)
        return None if elem is None else f"{'TArray' if inner else 'TArrayNoInit'}<{elem}>"
    if t.startswith("class ") and t.endswith("*"):
        return t
    if t.startswith("TScriptInterface<"):
        return t
    if t in MIRROR_FALLBACK:
        return MIRROR_FALLBACK[t]
    if "Mirror" in t:
        return None
    if t.startswith("struct F"):
        return t[len("struct "):]
    if ENUM_RE.match(t):
        return "BYTE"
    return None


def pdb_data_members(t: dict | None) -> list[dict]:
    if not t:
        return []
    return [m for m in t["members"] if not m["is_base"] and not m["is_vftable"] and m["name"] and not m["name"].startswith("___u")]


def pdb_size_of(members: list[dict]) -> int:
    """bytes a run of 2012 PDB members occupies (bitfields sharing an offset count once, as their DWORD)"""
    total = 0
    seen_bit_offsets = set()
    for m in members:
        if m.get("bits") is not None:
            if m["offset"] not in seen_bit_offsets:
                seen_bit_offsets.add(m["offset"])
                total += 4
        else:
            total += m.get("size") or 0
    return total


def pdb_decl(m: dict) -> str:
    if m.get("bits") is not None:
        return f"BITFIELD {m['name']}:{m['bits']};"
    return f"{declarator(fix_type(m['type']), m['name'])};".replace("struct class ", "class ")


def fold_script_interfaces(members: list[dict]) -> list[dict]:
    """The dump splits a TScriptInterface<I> property into `X_Object` + `X_Interface` (two 4-byte pointers);
    fold them back into one 8-byte member `X` of type TScriptInterface<class I...>."""
    by_name = {m["name"]: m for m in members}
    out = []
    skip = set()
    for m in members:
        if m["name"] in skip:
            continue
        if m["name"].endswith("_Object"):
            stem = m["name"][:-len("_Object")]
            other = by_name.get(stem + "_Interface")
            if other and other["offset"] == m["offset"] + 4 and m["type"].startswith("class U") and m["type"].endswith("*"):
                iface = "I" + m["type"][len("class U"):-1]
                out.append({**m, "name": stem, "size": 8, "type": f"TScriptInterface<class {iface}>", "script_interface": True})
                skip.add(other["name"])
                continue
        out.append(m)
    return out


def sdk_groups(members: list[dict]) -> list[list[dict]]:
    """members sorted by offset; bitfields sharing a DWORD form one group ordered by mask"""
    groups: list[list[dict]] = []
    for m in sorted(fold_script_interfaces(members), key=lambda x: (x["offset"], x.get("mask", 0))):
        if m.get("bitfield") and groups and groups[-1][0].get("bitfield") and groups[-1][0]["offset"] == m["offset"]:
            groups[-1].append(m)
        else:
            groups.append([m])
    return groups


def base_type(t: str) -> str:
    t = t.strip()
    return t[len("struct "):] if t.startswith("struct ") else t


def build_block(name: str, sdk: dict, pdb_t: dict | None, ref: dict[str, tuple[str, str]], indent: str, size_2013: int | None):
    """Returns (lines, added, removed, unresolved, notes)."""
    lines: list[str] = []
    added: list[str] = []
    unresolved: list[str] = []
    notes: list[str] = []
    emitted: set[str] = set()
    access = "public"
    byte_run = False
    pdb_members = pdb_data_members(pdb_t)
    pdb_index = {m["name"]: i for i, m in enumerate(pdb_members)}
    unknown_n = 0
    cursor = sdk.get("span_start", 0)
    prev_name: str | None = None

    def emit(text: str, want: str = "public") -> None:
        nonlocal access
        if want != access:
            lines.append(f"{want}:")
            access = want
        lines.append(indent + text)

    def close_byte_run(is_byte: bool) -> None:
        nonlocal byte_run
        if byte_run and not is_byte:
            emit("SCRIPT_ALIGN;", access)
        byte_run = is_byte

    def fill_gap(start: int, end: int, next_name: str | None, next_type: str) -> None:
        nonlocal unknown_n, cursor
        gap = end - start
        if gap <= 0:
            return
        align = 16 if base_type(next_type) in ALIGN16 else 4
        if gap < align and end % align == 0:
            return  # alignment padding before the next member, the compiler inserts it
        lo = pdb_index.get(prev_name, -1) + 1 if prev_name else 0
        hi = pdb_index.get(next_name, len(pdb_members)) if next_name else len(pdb_members)
        candidates = pdb_members[lo:hi] if lo <= hi else []
        if candidates and pdb_size_of(candidates) == gap:
            off = start
            seen_bits = set()
            for m in candidates:
                text, want = ref.get(m["name"], (None, "public"))
                if text is None:
                    text = pdb_decl(m)
                is_byte = m.get("bits") is None and m.get("size") == 1
                close_byte_run(is_byte)
                emit(f"{text}  // DISHONORED(layout): native, 2012 PDB @{m['offset']}, retail @{off} (SDK gap)", want)
                emitted.add(m["name"])
                added.append(m["name"])
                if m.get("bits") is not None:
                    if m["offset"] not in seen_bits:
                        seen_bits.add(m["offset"])
                        off += 4
                else:
                    off += m.get("size") or 0
        else:
            close_byte_run(False)
            emit(f"BYTE UnknownData{unknown_n:02d}[{gap}];  // DISHONORED(layout): retail SDK gap @{start} ({gap} bytes of native members; 2012 PDB candidates: "
                 f"{', '.join(m['name'] for m in candidates) or 'none'})")
            unresolved.append(f"UnknownData{unknown_n:02d}@{start}:{gap}")
            unknown_n += 1
        cursor = end

    for group in sdk_groups(sdk["members"]):
        first = group[0]
        if first["name"].startswith("VfTable_"):
            # CPF_NoExport pointer the dump reports for a secondary vtable: the class inherits that interface
            # (multiple inheritance), it is not a data member of the PROPS block
            notes.append(f"interface vtable {first['name'][len('VfTable_'):]} @{first['offset']}: the class must inherit it (not a member)")
            fill_gap(cursor, first["offset"], first["name"], first["type"])
            cursor = first["offset"] + first["size"]
            prev_name = first["name"] if first["name"] in pdb_index else prev_name
            continue
        fill_gap(cursor, first["offset"], first["name"], first["type"])
        for m in group:
            text, want = ref.get(m["name"], (None, "public"))
            note = ""
            pdb_m = pdb_members[pdb_index[m["name"]]] if m["name"] in pdb_index else None
            if text is None:
                if m.get("bitfield"):
                    text = f"BITFIELD {m['name']}:1;"
                elif pdb_m is not None and ("Mirror" in m["type"] or map_type(m["type"]) is None):
                    text = pdb_decl(pdb_m)
                else:
                    ty = map_type(m["type"])
                    if ty is None:
                        text = f"BYTE {m['name']}[{m['size']}];"
                        note = f" (unresolved SDK type {m['type']})"
                        unresolved.append(f"{m['name']}:{m['type']}")
                    else:
                        suffix = f"[{m['count']}]" if m.get("count") else ""
                        text = f"{ty} {m['name']}{suffix};"
                text += f"  // DISHONORED(layout): retail SDK @{m['offset']}" + (f", 2012 PDB @{pdb_m['offset']}" if pdb_m else "") + note
                added.append(m["name"])
            is_byte = not m.get("bitfield") and m["size"] == 1 and not m.get("count")
            close_byte_run(is_byte)
            emit(text, want)
            emitted.add(m["name"])
        cursor = first["offset"] + (4 if first.get("bitfield") else first["size"])
        prev_name = group[-1]["name"]
    if byte_run:
        emit("SCRIPT_ALIGN;", access)
    if access != "public":
        lines.append("public:")
    removed = [n for n in ref if n not in emitted]
    span_end = sdk.get("span_end", cursor)
    tail_pdb = [m for m in pdb_members if m["name"] not in emitted and (prev_name is None or pdb_index.get(m["name"], -1) > pdb_index.get(prev_name, -1))]
    if size_2013 is not None and size_2013 > span_end:
        notes.append(f"native tail: retail sizeof {size_2013} - span end {span_end} = {size_2013 - span_end} bytes after the block; "
                     f"2012 PDB members past the last reflected one: {', '.join(f'{m['name']}@{m['offset']}' for m in tail_pdb) or 'none'}")
    elif tail_pdb:
        notes.append(f"no native tail in retail (sizeof {size_2013} == span end {span_end}) but the 2012 PDB has members past the last reflected one: "
                     f"{', '.join(f'{m['name']}@{m['offset']}' for m in tail_pdb)} — they do not exist in retail: delete them from the header (not shim)")
    for m in [m for m in pdb_members if m["name"] not in emitted and m["name"] in ref and m["name"] not in {t["name"] for t in tail_pdb}]:
        notes.append(f"{m['name']}: in the 2012 PDB (@{m['offset']}) and the reference block but not in retail at this position: verify it was removed (shimmed below)")
    return lines, added, removed, unresolved, notes


def find_block(module: str, name: str, headers: dict[Path, str]):
    stem = uc_stem(name)
    for p, text in headers.items():
        for m in PROPS_BLOCK_RE.finditer(text):
            if m.group(2) == stem:
                return p, m
    return None


def depth(name: str, sdk: dict) -> int:
    d = 0
    while name in sdk and sdk[name].get("super"):
        name = sdk[name]["super"]
        d += 1
    return d


def run(module: str, names: list[str], dry_run: bool, print_only: bool) -> int:
    sdk = load_sdk()
    types = load_types()
    sizes = retail_sizes()
    headers = {p: p.read_text(encoding="utf-8") for p in (SOURCE / module / "Inc").glob("*Classes.h")}
    rc = 0
    for name in sorted(names, key=lambda n: depth(n, sdk)):
        entry = sdk.get(name)
        if entry is None:
            print(f"{name}: not in the retail SDK dump")
            rc = 1
            continue
        sup = entry.get("super")
        if sup and sizes.get(sup) is not None and sizes[sup] != entry.get("span_start"):
            print(f"{name}: note: span starts at {entry['span_start']} but retail sizeof({sup}) is {sizes[sup]}: converge the base first")
        hit = find_block(module, name, headers)
        if hit is None and not print_only:
            print(f"{name}: no //## BEGIN PROPS {uc_stem(name)} block in {module}/Inc/*Classes.h (use --print and paste by hand)")
            rc = 1
            continue
        indent = hit[1].group(1) if hit else "    "
        ref = parse_props_block(hit[1].group(3)) if hit else {}
        lines, added, removed, unresolved, notes = build_block(name, entry, types.get(name), ref, indent, sizes.get(name))
        stem = uc_stem(name)
        header = [f"{indent}//## BEGIN PROPS {stem}",
                  f"{indent}// DISHONORED(layout): retail SDK span {entry.get('span_start')}..{entry.get('span_end')}, retail sizeof {sizes.get(name, '?')};"
                  f" regenerated by sdk_props.py{'; reference-only members moved to the shim block below: ' + ', '.join(removed) if removed else ''}"]
        block = "\n".join(header + lines + [f"{indent}//## END PROPS {stem}"] + shim_lines(removed, ref, indent))
        print(f"{name}: {len(lines)} lines, +{len(added)} ({', '.join(added)}), -{len(removed)} reference-only ({', '.join(removed)})"
              + (f", UNRESOLVED {', '.join(unresolved)}" if unresolved else ""))
        for n in notes:
            print(f"  {n}")
        if dry_run or print_only or hit is None:
            print(block)
            continue
        p, m = hit
        headers[p] = headers[p][:m.start()] + block + headers[p][m.end():]
    if not dry_run and not print_only:
        for p, text in headers.items():
            if text != p.read_text(encoding="utf-8"):
                write_retry(p, text)
                print(f"wrote {p.relative_to(REPO)}")
    return rc


def survey(module: str) -> int:
    delta = TYPES / "retail_sdk_delta.md"
    rows = []
    for line in delta.read_text(encoding="utf-8").splitlines():
        if line.startswith("| ") and not line.startswith("| Type") and not line.startswith("|---"):
            cells = [c.strip() for c in line.strip("|").split("|")]
            if len(cells) > 5 and (cells[5] or "too small" in cells[1]):  # exact contract rows are listed too; skip them
                rows.append((cells[0], "too small" in cells[1]))
    headers = {p: p.read_text(encoding="utf-8") for p in (SOURCE / module / "Inc").glob("*Classes.h")}
    mech, hand = [], []
    for name, small in rows:
        hit = find_block(module, name, headers)
        (mech if hit else hand).append((name, small, hit[0].name if hit else ""))
    print(f"{module}: {len(rows)} delta rows; {len(mech)} have a PROPS block (mechanical), {len(hand)} need hand work")
    by_header: dict[str, list[str]] = {}
    for name, small, h in mech:
        by_header.setdefault(h, []).append(name + ("*" if small else ""))
    for h, names in sorted(by_header.items()):
        print(f"  {h}: {len(names)}: {', '.join(names)}")
    print("  hand (no PROPS block in *Classes.h): " + ", ".join(n + ("*" if s else "") for n, s, _ in hand))
    print("  (* = too small)")
    return 0


def main(argv: list[str]) -> int:
    args = argv[1:]
    dry = print_only = False
    if "--survey" in args:
        args.remove("--survey")
        return survey(args[0] if args else "Engine")
    if "--dry-run" in args:
        args.remove("--dry-run")
        dry = True
    if "--print" in args:
        args.remove("--print")
        print_only = True
    if len(args) < 2:
        print(__doc__)
        return 2
    return run(args[0], args[1:], dry, print_only)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
