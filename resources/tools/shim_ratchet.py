"""agent ES: count every DISHONORED_SHIM_STATIC placeholder, decide per shim whether retail 2013 has the member,
and fail when the count rises. The ratchet that keeps "retail does not have this" an honest claim.

`DISHONORED_SHIM_STATIC` expands to `inline static` (Engine/Inc/Engine.h): a member declared with it is one
object shared by the whole process, never serialised, never a UProperty, never written by the loader. Nine of the
last seventeen defects of this project were something invented that retail does not do, and a shim is how an
invention gets into the tree without announcing itself. `shim_audit.py` ranks shims by how their *use* goes
wrong; this tool answers the prior question -- does retail have the member at all -- and holds the answer to a
committed ceiling so a later wave cannot quietly add more.

Why the question is answerable at all: a shim is a member of some class, and for a **script property** retail's
member set is completely enumerated, twice over and independently:

  * `resources/docs/types/retail_sdk_layout.json` -- a CodeRed SDK dump of the running retail 2013 exe;
  * `resources/docs/types/script_classes_2013.json` -- the classes of the cooked 2013 packages.

They agree exactly where both have the class (checked: USkeletalMeshComponent, 107 properties, zero difference),
so a name absent from both is absent from retail. What makes a shim a *script property* rather than a C++-only
member is read off the **reference UE3 tree**, not off this tree, because this tree has already moved every shim
out of the generated property block into a trailing shim block. In the reference tree the same member is either

  * inside a `//## BEGIN PROPS <Class>` .. `//## END PROPS <Class>` region of a generated header, or
  * named by a generated `VERIFY_CLASS_OFFSET*(<CppClass>, <ScriptClass>, <Member>)` line, which the script
    compiler emits for a property declared `var native` in script and by hand in C++ (this is the case for the
    whole cloth block: `ClothMeshPosData` is hand-written in the reference `UnSkeletalMesh.h` and *also* carries
    `VERIFY_CLASS_OFFSET_NODIE(USkeletalMeshComponent,SkeletalMeshComponent,ClothMeshPosData)`).

Either way the member is a script property of a named script class, and retail's property set for that class
settles it. A shim that answers neither test is a genuine C++-only member and this tool does **not** claim
retail lacks it.

Verdicts:

  retail-lacks        a reference-tree script property of its class, and absent from retail's property set for
                      that class in BOTH sources. Deleting it is a deletion of a feature retail does not have.
  retail-lacks-1src   the same, but only one of the two sources has the class, so only one could vote. Weaker,
                      because agent EN's inventory found real holes in both dumps: `retail_sdk_layout.json` is
                      missing `ADishonoredNPCPawn` outright, and `script_classes_2013.json` omits the Core
                      intrinsics (`UClass`, `UState`, `UStruct`, `UFunction`) because they are not cooked into
                      any package. A single source's silence is not absence.
  retail-has          PRESENT in retail's property set. This is not a shim to delete, it is a **defect**: real
                      per-instance storage at retail's own offset was replaced by one process-wide static.
  retail-lacks-span   a C++-only member of a **script struct** whose CodeRed record is closed: `span_start` is 0,
                      `gaps` is empty, and the named members tile every byte up to `span_end`. There is no
                      unnamed byte in the struct for the member to occupy, so retail cannot have it whether it
                      would have been a property or not. This is the one rule that does not need the member to
                      be a script property, and it only applies to a top-level struct, where `span_start == 0`
                      removes the super-class alignment ambiguity that makes the same arithmetic unusable on a
                      class (`AActor` is 592 bytes with its last property ending at 584).
  class-absent        the owner class itself is in neither source, so neither can speak for its members.
  unproven            not a reference-tree script property: a C++-only member. Absence is NOT shown and has to
                      be argued by hand, per member, against the binary.

`missing_type` is a separate column, not a verdict: the U/A/F-prefixed type names in the declaration that retail
2013 has no class or script struct for. It corroborates, it does not decide. An earlier draft of this tool made
it a verdict of its own and that was **wrong in 52 of 204 cases**: the pattern that spots `UReachSpec` also spots
`UBOOL`, `FLOAT`, `FName` and `FString`, which are Core typedefs and intrinsics that appear in no cooked package
and in no CodeRed class list, so a shim declared `FLOAT Foo` scored as proof that retail lacks a class. PRIMITIVES
below is that hole, named; and because a list of names is exactly the kind of thing that silently goes stale, a
type name only ever adds a note to a row whose verdict some other rule already settled.

Usage:
  python resources/tools/shim_ratchet.py                          count, classify, check the ceiling
  python resources/tools/shim_ratchet.py --csv out/shims.csv      also write the per-shim table
  python resources/tools/shim_ratchet.py --update-ceiling         lower the ceiling to what is measured now
  python resources/tools/shim_ratchet.py --list retail-has        print the shims of one verdict

Exit code 0 when every counted verdict is at or below its ceiling, 1 otherwise. `--update-ceiling` refuses to
raise a ceiling: a wave that adds shims has to say so in its report, not in this file. The ceiling records the
`schema` of the verdict set it was measured under, so changing the verdicts cannot launder a rise through a
rename either -- a ceiling from another schema is reported as stale and only `--reset-ceiling` replaces it.
"""
import argparse
import csv
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / "source" / "Development" / "Src"
REFERENCE = Path("D:/RecompileDishonored/UnrealEngine3")
CEILING = REPO / "resources" / "docs" / "shim_ceiling.json"
FALLBACK_TRUTH = Path("D:/RecompileDishonored/Recompile/resources/docs")

SHIM_RE = re.compile(r"^\s*DISHONORED_SHIM_STATIC\s+(?:static\s+)?(.+?)\s*([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*;")
CLASS_RE = re.compile(r"^\s*(?:class|struct)\s+(?:\w+\s+)?([A-Za-z_]\w*)\b")
PROPS_BEGIN = re.compile(r"//##\s*BEGIN PROPS\s+(\w+)")
PROPS_END = re.compile(r"//##\s*END PROPS\s+(\w+)")
VERIFY_RE = re.compile(r"VERIFY_CLASS_(?:OFFSET|PROPERTY)\w*\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)")
MEMBER_RE = re.compile(r"^\s*(?:const\s+|mutable\s+|class\s+|struct\s+|enum\s+)*[\w:]+(?:\s*<[^;]*>)?"
                       r"\s*[\*&\s]+([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*(?::\s*\d+\s*)?;")
TYPENAME_RE = re.compile(r"\b([UAF][A-Z]\w+)\b")
COUNTED = ("retail-lacks", "retail-lacks-span", "retail-lacks-1src", "retail-has", "class-absent", "unproven")
# Core typedefs and intrinsics that match TYPENAME_RE but are not script classes and are cooked into no package,
# so their absence from either retail dump says nothing. See the `missing_type` paragraph above.
PRIMITIVES = {"UBOOL", "FLOAT", "FName", "FString", "FStringNoInit", "FPointer", "FScriptDelegate", "FGuid",
              "FVector", "FVector2D", "FVector4", "FRotator", "FMatrix", "FQuat", "FColor", "FLinearColor",
              "FBox", "FSphere", "FPlane", "FBoneAtom", "FIntPoint", "FRandomStream", "FArchive", "FTexture",
              "FRenderResource", "FSceneViewStateInterface", "FTwoVectors", "FInterpCurvePoint"}


def read_text(path: Path) -> str:
    return path.read_bytes().decode("utf-8", "replace")


def truth(name: str) -> Path:
    local = REPO / "resources" / "docs" / name
    return local if local.exists() else FALLBACK_TRUTH / name


class Retail:
    """retail 2013's classes, script structs and their property sets, from the two independent dumps."""

    def __init__(self):
        sdk = json.loads(read_text(truth("types/retail_sdk_layout.json")))
        script = json.loads(read_text(truth("types/script_classes_2013.json")))["classes"]
        self.sdk_props = {}
        for group in ("classes", "structs"):
            for cpp, rec in sdk[group].items():
                self.sdk_props[cpp] = {m["name"] for m in rec.get("members", [])}
        self.script_props = {}
        for name, rec in script.items():
            self.script_props[name] = {ch["name"] for ch in rec.get("children", [])
                                       if ch.get("kind", "").endswith("Property")}
        self.closed_structs = {}
        for name, rec in sdk["structs"].items():
            members = rec.get("members", [])
            end = rec.get("span_end")
            if rec.get("span_start") == 0 and not rec.get("gaps") and members and end is not None:
                covered = set()
                for m in members:
                    covered.update(range(m["offset"], m["offset"] + m["size"]))
                if covered == set(range(0, end)):
                    self.closed_structs[name] = (end, len(members))
        self.known_types = set(self.sdk_props)
        for name in self.script_props:
            self.known_types.update((name, "U" + name, "A" + name, "F" + name))

    def props(self, script_class: str):
        """(sdk member set or None, cooked-package property set or None) for one script class name."""
        sdk = None
        for candidate in (script_class, "U" + script_class, "A" + script_class, "F" + script_class):
            if candidate in self.sdk_props:
                sdk = self.sdk_props[candidate]
                break
        return sdk, self.script_props.get(script_class)

    def missing_types(self, declared: str):
        """U/A/F-prefixed type names in a declaration that retail 2013 has no class or script struct for.

        Corroboration only. PRIMITIVES are excluded because a Core typedef or intrinsic is in neither dump
        whether retail has it or not, which is the hole that made an earlier verdict of this tool wrong."""
        out = []
        for name in TYPENAME_RE.findall(declared):
            if name in PRIMITIVES or name in self.known_types:
                continue
            bare = name[1:]
            if bare in self.script_props or any(p + bare in self.sdk_props for p in "UAF"):
                continue
            out.append(name)
        return out


def script_properties_of_reference():
    """(script class, member) pairs the reference UE3 tree declares as script properties, by both routes."""
    pairs = set()
    verify = set()
    for header in (REFERENCE / "Development" / "Src").rglob("Inc/*.h"):
        text = read_text(header)
        for m in VERIFY_RE.finditer(text):
            verify.add((m.group(2), m.group(3)))
        owner = None
        for line in text.splitlines():
            begin = PROPS_BEGIN.search(line)
            if begin:
                owner = begin.group(1)
                continue
            if PROPS_END.search(line):
                owner = None
                continue
            if owner:
                m = MEMBER_RE.match(line)
                if m:
                    pairs.add((owner, m.group(1)))
    return pairs, verify


def enclosing_classes(text: str):
    """line number -> the class or struct whose body encloses it, by brace depth."""
    out, stack, depth, pending = {}, [], 0, None
    for n, line in enumerate(text.splitlines(), 1):
        code = line.split("//")[0]
        m = CLASS_RE.match(line)
        if m and ";" not in code:
            pending = m.group(1)
        opens, closes = code.count("{"), code.count("}")
        if opens and pending is not None:
            stack.append((pending, depth))
            pending = None
        depth += opens - closes
        while stack and depth <= stack[-1][1]:
            stack.pop()
        out[n] = stack[-1][0] if stack else ""
        if closes:
            pending = None
    return out


def script_name(owner: str) -> str:
    return owner[1:] if len(owner) > 1 and owner[0] in "UAF" and owner[1].isupper() else owner


def collect(self: Retail, props: set, verify: set):
    shims = []
    for header in sorted(SRC.rglob("*.h")):
        text = read_text(header)
        if "DISHONORED_SHIM_STATIC" not in text:
            continue
        owners = enclosing_classes(text)
        rel = str(header.relative_to(REPO)).replace("\\", "/")
        for n, line in enumerate(text.splitlines(), 1):
            m = SHIM_RE.match(line)
            if not m:
                continue
            declared, name = m.group(1).strip(), m.group(2)
            owner = owners.get(n, "")
            cls = script_name(owner)
            row = {"file": rel, "line": n, "owner": owner, "script_class": cls, "name": name,
                   "type": declared, "prop_source": ("PROPS" if (cls, name) in props else "")
                   + ("VERIFY" if (cls, name) in verify else "")}
            missing = self.missing_types(declared)
            row["missing_type"] = " ".join(missing)
            sdk, cooked = self.props(cls)
            row["retail_sdk_class"] = int(sdk is not None)
            row["retail_script_class"] = int(cooked is not None)
            corroboration = ("; the declaration also names %s, which retail 2013 has no class or script struct "
                             "for" % ", ".join(missing)) if missing else ""
            closed = self.closed_structs.get(owner) or self.closed_structs.get("F" + cls)
            if not row["prop_source"] and closed and not (sdk and name in sdk):
                row["verdict"] = "retail-lacks-span"
                row["evidence"] = ("retail's script struct %s is closed: span 0..%d, no gaps, and its %d named "
                                   "properties tile every byte, so there is no room for %s%s"
                                   % (owner, closed[0], closed[1], name, corroboration))
            elif not row["prop_source"]:
                row["verdict"] = "unproven"
                row["evidence"] = ("not a script property of %s in the reference tree, so it is a C++-only "
                                   "member and neither dump can speak for it%s" % (cls, corroboration))
            elif sdk is None and cooked is None:
                row["verdict"] = "class-absent"
                row["evidence"] = "retail 2013 has no class %s in either dump" % cls
            elif (sdk and name in sdk) or (cooked and name in cooked):
                row["verdict"] = "retail-has"
                row["evidence"] = "PROPERTY OF retail %s (%s) - needs real storage, not a shim" % (
                    cls, "sdk+cooked" if (sdk and name in sdk) and (cooked and name in cooked)
                    else ("sdk" if sdk and name in sdk else "cooked"))
            elif sdk is not None and cooked is not None:
                row["verdict"] = "retail-lacks"
                row["evidence"] = ("reference script property of %s (%s); absent from retail's %d sdk members "
                                   "and %d cooked properties%s"
                                   % (cls, row["prop_source"], len(sdk), len(cooked), corroboration))
            else:
                row["verdict"] = "retail-lacks-1src"
                row["evidence"] = ("reference script property of %s (%s); absent from the only retail source "
                                   "that has the class (%s)" % (cls, row["prop_source"],
                                                                "sdk" if sdk is not None else "cooked")
                                   + corroboration)
            shims.append(row)
    return shims


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", help="write the per-shim table here")
    ap.add_argument("--update-ceiling", action="store_true", help="lower the ceiling to the measured counts")
    ap.add_argument("--reset-ceiling", action="store_true",
                    help="rewrite a ceiling measured under a different verdict schema, rise or not")
    ap.add_argument("--list", metavar="VERDICT", help="print every shim of one verdict and exit")
    ap.add_argument("--ceiling", default=str(CEILING))
    args = ap.parse_args(argv)

    if not REFERENCE.is_dir():
        print("reference tree %s is absent: the script-property test cannot run" % REFERENCE)
        return 2
    retail = Retail()
    props, verify = script_properties_of_reference()
    shims = collect(retail, props, verify)
    counts = Counter(r["verdict"] for r in shims)

    if args.list:
        for r in shims:
            if r["verdict"] == args.list:
                print("%-46s %-30s %-28s %s:%d" % (r["owner"] + "::" + r["name"], r["type"][:30],
                                                   r["verdict"], r["file"], r["line"]))
        print("%d shims with verdict %s" % (counts[args.list], args.list))
        return 0

    if args.csv:
        out = Path(args.csv)
        out.parent.mkdir(parents=True, exist_ok=True)
        cols = ["verdict", "owner", "script_class", "name", "type", "prop_source", "missing_type",
                "retail_sdk_class", "retail_script_class", "file", "line", "evidence"]
        with out.open("w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=cols)
            w.writeheader()
            for r in sorted(shims, key=lambda r: (COUNTED.index(r["verdict"]), r["file"], r["line"])):
                w.writerow({c: r[c] for c in cols})
        print("-> %s" % out)

    ceiling_path = Path(args.ceiling)
    ceiling = json.loads(read_text(ceiling_path)) if ceiling_path.exists() else {}
    schema = "|".join(COUNTED)
    measured = {"schema": schema, "total": len(shims), **{v: counts[v] for v in COUNTED}}
    stale = ceiling and ceiling.get("schema") != schema
    if args.update_ceiling or args.reset_ceiling:
        raised = {k: (ceiling[k], v) for k, v in measured.items()
                  if k != "schema" and k in ceiling and v > ceiling[k]}
        if stale and not args.reset_ceiling:
            print("ceiling %s was measured under verdict schema %r, this tool measures %r: it is stale, and "
                  "--reset-ceiling is the only way to replace it"
                  % (ceiling_path, ceiling.get("schema"), schema))
            return 1
        if raised and not args.reset_ceiling:
            print("refusing to raise the ceiling: " + ", ".join("%s %d -> %d" % (k, a, b)
                                                                for k, (a, b) in sorted(raised.items())))
            return 1
        ceiling_path.parent.mkdir(parents=True, exist_ok=True)
        ceiling_path.write_text(json.dumps(measured, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("ceiling written: %s" % json.dumps(measured, sort_keys=True))
        return 0

    per_file = defaultdict(Counter)
    for r in shims:
        per_file[r["file"]][r["verdict"]] += 1
    print("%d DISHONORED_SHIM_STATIC declarations in %d headers" % (len(shims), len(per_file)))
    failed = []
    if stale:
        print("  ceiling %s is STALE: measured under verdict schema %r, this tool measures %r"
              % (ceiling_path.name, ceiling.get("schema"), schema))
    for key in ("total",) + COUNTED:
        limit = ceiling.get(key)
        state = "no ceiling" if limit is None else ("OK" if measured[key] <= limit else "RISEN")
        if limit is not None and measured[key] > limit:
            failed.append((key, measured[key], limit))
        print("  %-20s %5d   ceiling %-6s %s" % (key, measured[key], "-" if limit is None else limit, state))
    print("\nper header (%s):" % " / ".join(COUNTED))
    for f, c in sorted(per_file.items(), key=lambda kv: -sum(kv[1].values())):
        print("  %-46s %s" % (f.split("/")[-1], " / ".join("%3d" % c[v] for v in COUNTED)))
    if counts["retail-has"]:
        print("\n%d shim(s) that retail 2013 DOES have as a property -- each is a defect, not a placeholder:"
              % counts["retail-has"])
        for r in shims:
            if r["verdict"] == "retail-has":
                print("  %-30s %-34s %s:%d" % (r["owner"], r["name"], r["file"].split("/")[-1], r["line"]))
    if failed:
        print("\nRATCHET FAILED: " + ", ".join("%s %d > %d" % f for f in failed))
        return 1
    print("\nratchet OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
