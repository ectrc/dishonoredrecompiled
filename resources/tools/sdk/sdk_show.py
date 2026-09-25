"""Show one class/struct three ways, joined by member name: the RETAIL runtime layout (SDK dump,
retail_sdk_layout.json), the 2012 PDB layout (types.json) and our compiled layout (a LayoutProbe
output). `!` marks a member whose probed offset differs from retail; native-only gaps of the dump
and 2012-only members are listed so the agent sees where the C++-only members must go.

Usage: python resources/tools/sdk/sdk_show.py [--probe build/<dir>/layout_probe.txt] UInterpTrackInst [AActor ...]
"""
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / "resources" / "tools" / "symbols"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_layout_probe import load_types  # noqa: E402
from sdk_props import load_sdk, pdb_data_members, retail_sizes, sdk_groups  # noqa: E402
from xcheck_sdk_layout import load_probe  # noqa: E402


def show(name: str, sdk: dict, types: dict, sizes: dict, probe) -> None:
    entry = sdk.get(name)
    pdb_t = types.get(name)
    psizes, poffs = probe if probe else ({}, {})
    print(f"== {name}: retail span {entry.get('span_start') if entry else '-'}..{entry.get('span_end') if entry else '-'}"
          f"  retail sizeof {sizes.get(name, '-')}  2012 PDB {pdb_t['size'] if pdb_t else '-'}  ours {psizes.get(name, '-')}"
          + (f"  super {entry.get('super')}" if entry and entry.get("super") else ""))
    if entry is None:
        print("  not in the retail SDK dump")
        return
    pdb_members = {m["name"]: m for m in pdb_data_members(pdb_t)}
    print(f"  {'member':40} {'retail':>7} {'size':>5} {'2012':>7} {'ours':>7}  retail type / mask")
    seen = set()
    cursor = entry.get("span_start", 0)
    for group in sdk_groups(entry["members"]):
        first = group[0]
        if first["offset"] > cursor:
            print(f"  {'<gap>':40} {cursor:7} {first['offset'] - cursor:5}                  native-only region")
        for m in group:
            seen.add(m["name"])
            pm = pdb_members.get(m["name"])
            ours = poffs.get((name, m["name"]))
            flag = "!" if ours is not None and ours != m["offset"] else " "
            mask = f" mask 0x{m['mask']:x}" if m.get("bitfield") else ""
            cnt = f"[{m['count']}]" if m.get("count") else ""
            print(f" {flag}{m['name'] + cnt:40} {m['offset']:7} {m['size']:5} {pm['offset'] if pm else '-':>7} {ours if ours is not None else '-':>7}  {m['type']}{mask}")
        cursor = first["offset"] + (4 if first.get("bitfield") else first["size"])
    if sizes.get(name) and sizes[name] > cursor:
        print(f"  {'<tail>':40} {cursor:7} {sizes[name] - cursor:5}                  native members after the last reflected one")
    only_pdb = [m for m in pdb_data_members(pdb_t) if m["name"] not in seen]
    if only_pdb:
        print("  2012 PDB members not reflected in retail (native-only; place them in the gaps/tail):")
        for m in only_pdb:
            bits = f":{m['bits']}" if m.get("bits") is not None else ""
            print(f"    {m['offset']:5} {m.get('size') or 0:4}  {m['type']}  {m['name']}{bits}")
    print()


def main(argv: list[str]) -> int:
    args = argv[1:]
    probe = None
    if args and args[0] == "--probe":
        probe = load_probe(REPO / args[1] if not Path(args[1]).is_absolute() else Path(args[1]))
        args = args[2:]
    if not args:
        print(__doc__)
        return 2
    sdk, types, sizes = load_sdk(), load_types(), retail_sizes()
    for n in args:
        show(n, sdk, types, sizes, probe)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
