"""agentAU: name the member of a retail class at a byte offset (retail_sdk_layout.json).
  python build/agentAU_work/off.py <Class> [<offset> ...]      offsets: decimal or 0x..; none = dump all
Also prints sizeof, so the `Base[2].Member` shape the Hex-Rays output uses can be resolved by hand:
offset = 2*sizeof(Base) + offsetof(Base, Member)."""
import json, sys
D = json.load(open("resources/docs/types/retail_sdk_layout.json", encoding="utf-8"))
CL = D["classes"] if "classes" in D else D
name = sys.argv[1]
c = CL.get(name)
if c is None:
    cands = [k for k in CL if name.lower() in k.lower()]
    print(f"{name}: not found; did you mean {cands[:10]}")
    raise SystemExit(1)
print(f"{name}: sizeof {c.get('size')} super {c.get('super')}")
ms = c.get("members") or []
if len(sys.argv) > 2:
    for a in sys.argv[2:]:
        o = int(a, 0)
        hit = [m for m in ms if m.get("offset") == o]
        near = sorted((m for m in ms if m.get("offset") is not None and m["offset"] <= o), key=lambda m: -m["offset"])[:1]
        print(f"  @{o}: " + (", ".join(f"{m['name']}:{m.get('type')}{'/'+hex(m['mask']) if m.get('mask') else ''}" for m in hit) or f"(none; previous {near[0]['name']}@{near[0]['offset']})" if near else "(none)"))
else:
    for m in ms:
        print(f"  @{m.get('offset')} {m.get('name')} {m.get('type')}{'/'+hex(m['mask']) if m.get('mask') else ''}")
