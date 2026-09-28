"""agent DP: cross-reference every DISHONORED_SHIM_STATIC declaration against this tree's own reads and writes.

A DISHONORED_SHIM_STATIC member is `inline static` (Engine/Inc/Engine.h), so it is one object shared by every
instance in the process instead of per-instance storage, and it is never serialised, never a UProperty and never
written by the loader. Nine of the last fourteen defects of this project were exactly that. This tool finds the
declarations, finds the uses in the code the build actually compiles, and ranks them by how they go wrong:

  A  fatal          a container shim that is indexed, or whose Num() is compared against a per-instance count, or
                    that appears inside a check() -- these abort the process once content reaches them, which is
                    what USkeletalMeshComponent::BoneVisibilityStates did to two of the three mission maps
  B  shared write   written at runtime from compiled code: one store for the whole process, so the last writer
                    wins for every other instance
  C  dead read      read from compiled code and never written anywhere: permanently zero/empty/NULL, so whatever
                    reads it silently does nothing (GSystemSettings.MaxFilterBlurSampleCount, wave 8)
  D  unused         no use in compiled code today: harmless until a port or new content reaches it

Three filters make the counts mean something:

  * only files the ninja graph compiles are counted (899 of them; the reference tree carries editor-only units);
  * only preprocessor regions this build keeps are counted -- the -D table is read out of build.ninja, so
    WITH_APEX / WITH_FACEFX / WITH_EDITOR / WITH_GFx blocks do not inflate a row;
  * a name that is ALSO declared as real storage somewhere in the tree is ambiguous by construction (`Materials`,
    `PathList`, `Score`), and those rows are ranked separately with a `?` suffix instead of polluting the top.

Usage:
  python build/agentDP_work/shim_audit.py [--build-dir build/agentDP] [--out build/agentDP_work]

Writes shim_audit.csv (every shim, one row) and shim_audit.txt (the ranked summary).
"""
import argparse
import csv
import re
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / "source" / "Development" / "Src"

SHIM_RE = re.compile(r"^\s*DISHONORED_SHIM_STATIC\s+(?:static\s+)?(.+?)\s*([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*;")
CLASS_RE = re.compile(r"^\s*(?:class|struct)\s+(?:\w+\s+)?([A-Za-z_]\w*)\b")
IF_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")

CONTAINER_HINTS = ("TArray", "TArrayNoInit", "TMap", "TMapBase", "TSet", "TStaticArray", "TLookupMap",
                   "TDoubleLinkedList", "TIndirectArray", "TTransArray", "TStaticBitArray")

WRITE_OPS = r"(?:=(?!=)|\+=|-=|\*=|/=|\|=|&=|\^=|<<=|>>=|\+\+|--)"
MUTATORS = ("Add", "AddItem", "AddUniqueItem", "AddZeroed", "AddAllocated", "Empty", "Remove", "RemoveItem",
            "RemoveSwap", "Insert", "InsertItem", "Reserve", "Reset", "SetNum", "Append", "Push", "Pop",
            "Shrink", "Sort", "Swap")
CHECK_RE = re.compile(r"\b(?:check|checkf|checkSlow|checkMsg|verify|verifyf|appErrorf)\s*\(")


def build_macros(build_dir: Path):
    macros = {}
    ninja = build_dir / "build.ninja"
    if ninja.exists():
        text = ninja.read_text(encoding="utf-8", errors="replace")
        for name, val in re.findall(r"-D([A-Za-z_]\w*)=(\d+)\b", text):
            macros[name] = int(val)
    return macros


def region_live(path: Path, macros):
    """line number -> False when the line sits in a preprocessor region this build does not compile.

    Only conditions of the form `WITH_X`, `!WITH_X`, `WITH_X == n`, `defined(X)` and && chains of those are
    evaluated; anything else is treated as live, so the filter only ever removes a region we are sure about."""
    live = {}
    stack = []  # (state, taken_any) where state in (True, False, None=unknown)

    def eval_cond(kind, expr):
        expr = expr.split("//")[0].strip()
        if kind == "ifdef":
            return True if expr.strip() in macros else None
        if kind == "ifndef":
            return False if macros.get(expr.strip(), 0) else None
        parts = [p.strip() for p in expr.split("&&")]
        if len(parts) > 1:
            vals = [eval_cond("if", p) for p in parts]
            if any(v is False for v in vals):
                return False
            return True if all(v is True for v in vals) else None
        e = parts[0].strip().strip("()").strip()
        m = re.fullmatch(r"(!?)\s*([A-Za-z_]\w*)", e)
        if m and m.group(2) in macros:
            v = bool(macros[m.group(2)])
            return (not v) if m.group(1) else v
        m = re.fullmatch(r"([A-Za-z_]\w*)\s*(==|!=)\s*(\d+)", e)
        if m and m.group(1) in macros:
            v = macros[m.group(1)] == int(m.group(3))
            return v if m.group(2) == "==" else (not v)
        m = re.fullmatch(r"defined\s*\(\s*([A-Za-z_]\w*)\s*\)", e)
        if m:
            return True if m.group(1) in macros else None
        return None

    for n, line in enumerate(path.read_bytes().decode("utf-8", "replace").splitlines(), 1):
        m = IF_RE.match(line)
        if m:
            kind, expr = m.group(1), m.group(2)
            if kind in ("if", "ifdef", "ifndef"):
                stack.append([eval_cond(kind, expr), None])
            elif kind == "elif" and stack:
                prev = stack[-1][0]
                stack[-1] = [None if prev is None else (eval_cond("if", expr) if prev is False else False), None]
            elif kind == "else" and stack:
                prev = stack[-1][0]
                stack[-1] = [None if prev is None else (not prev), None]
            elif kind == "endif" and stack:
                stack.pop()
            live[n] = False
            continue
        live[n] = not any(s[0] is False for s in stack)
    return live


def enclosing_classes(path: Path):
    out, stack, depth, pending = {}, [], 0, None
    for n, line in enumerate(path.read_bytes().decode("utf-8", "replace").splitlines(), 1):
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


def collect_shims():
    shims = []
    for path in sorted(SRC.rglob("*.h")):
        text = path.read_bytes().decode("utf-8", "replace")
        if "DISHONORED_SHIM_STATIC" not in text:
            continue
        owners = enclosing_classes(path)
        for n, line in enumerate(text.splitlines(), 1):
            m = SHIM_RE.match(line)
            if not m:
                continue
            typ, name = m.group(1).strip(), m.group(2)
            shims.append({"file": str(path.relative_to(REPO)).replace("\\", "/"), "line": n,
                          "owner": owners.get(n, ""), "type": typ, "name": name,
                          "container": int(any(h in typ for h in CONTAINER_HINTS))})
    return shims


def compiled_units(build_dir: Path):
    ninja = build_dir / "build.ninja"
    if not ninja.exists():
        return None
    units = set()
    for m in re.finditer(r"^build [^:]+:\s+CXX_COMPILER__\S+\s+(\S.*)$",
                         ninja.read_text(encoding="utf-8", errors="replace"), re.M):
        for tok in m.group(1).split():
            if tok.endswith((".cpp", ".c", ".cc")):
                units.add(Path(tok.replace("$", "")).name.lower())
    return units


def retail_members():
    """owner class -> {member name} from the CodeRed dump of the running retail 2013 exe, plus its 2012 PDB twin.

    A shim whose name is NOT in its own class's retail member set is a member of a UE3 feature that Dishonored's
    engine branch does not have (the legacy path network, morph targets, cloth, secondary viewports): the fix is
    to delete the reference code path, as BoneVisibilityStates was. A shim whose name IS there is real retail
    storage that this tree left storage-less: the fix is a layout change at retail's own offset."""
    out = {}
    try:
        import json
        d = json.loads((REPO / "resources/docs/types/retail_sdk_layout.json").read_text(encoding="utf-8"))
    except OSError:
        return out
    for cls, rec in d.get("classes", {}).items():
        out[cls] = {m["name"] for m in rec.get("members", [])}
    for cls, rec in d.get("structs", {}).items():
        out[cls] = {m["name"] for m in rec.get("members", [])}
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", default="build/agentDP")
    ap.add_argument("--out", default="build/agentDP_work")
    args = ap.parse_args()

    shims = collect_shims()
    retail = retail_members()
    names = {s["name"] for s in shims}
    units = compiled_units(REPO / args.build_dir)
    macros = build_macros(REPO / args.build_dir)
    print("%d shim declarations, %d distinct names, %s, %d -D macros"
          % (len(shims), len(names), ("%d compiled units" % len(units)) if units else "no ninja graph", len(macros)))

    word_re = re.compile(r"\b(" + "|".join(sorted(map(re.escape, names), key=len, reverse=True)) + r")\b")
    decl_lines = defaultdict(set)
    for s in shims:
        decl_lines[s["file"]].add(s["line"])

    # a shim name that is also real storage somewhere: the identifier count cannot be trusted
    ambiguous = set()
    real_decl = re.compile(r"^\s*(?!DISHONORED_SHIM_STATIC)(?:const\s+|mutable\s+|static\s+|class\s+|struct\s+|enum\s+)*"
                           r"[A-Za-z_][\w:]*\s*(?:<[^;]*>)?\s*[\*&\s]+(%s)\s*(?:\[[^\]]*\])?\s*(?::\s*\d+\s*)?;\s*$")
    for path in sorted(SRC.rglob("*.h")):
        text = path.read_bytes().decode("utf-8", "replace")
        rel = str(path.relative_to(REPO)).replace("\\", "/")
        for n, line in enumerate(text.splitlines(), 1):
            if n in decl_lines[rel] or "DISHONORED_SHIM_STATIC" in line:
                continue
            for name in set(word_re.findall(line)):
                if re.match(real_decl.pattern % re.escape(name), line):
                    ambiguous.add(name)

    verify = defaultdict(int)
    use = defaultdict(lambda: {"reads": 0, "writes": 0, "checks": 0, "indexed": 0, "num_cmp": 0,
                               "files": set(), "examples": []})
    for path in sorted(list(SRC.rglob("*.cpp")) + list(SRC.rglob("*.h"))):
        text = path.read_bytes().decode("utf-8", "replace")
        if not word_re.search(text):
            continue
        rel = str(path.relative_to(REPO)).replace("\\", "/")
        if units is not None and path.suffix == ".cpp" and path.name.lower() not in units:
            continue
        live = region_live(path, macros)
        mine = decl_lines[rel]
        for n, line in enumerate(text.splitlines(), 1):
            if n in mine or not live.get(n, True) or "DISHONORED_SHIM_STATIC" in line:
                continue
            if "VERIFY_CLASS_OFFSET" in line or "VERIFY_CLASS_SIZE" in line:
                # the generated layout probe, not a runtime read of the value (and on a shim it takes the
                # address of a process-wide static, so it can never match a script offset anyway)
                verify[word_re.findall(line)[-1] if word_re.findall(line) else ""] += 1
                continue
            code = line.split("//")[0]
            for name in word_re.findall(code):
                u = use[name]
                u["checks"] += int(bool(CHECK_RE.search(code)))
                u["indexed"] += int(bool(re.search(re.escape(name) + r"\s*\(\s*[A-Za-z_0-9]", code)))
                u["num_cmp"] += int(bool(re.search(re.escape(name) + r"\s*\.\s*Num\s*\(\s*\)\s*(?:==|!=|<|>)", code))
                                    or bool(re.search(r"(?:==|!=|<|>)\s*" + re.escape(name) + r"\s*\.\s*Num\s*\(", code)))
                write = bool(re.search(re.escape(name) + r"\s*(?:\([^()]*\))?\s*" + WRITE_OPS, code)) or \
                    any(re.search(re.escape(name) + r"\s*\.\s*" + mu + r"\s*\(", code) for mu in MUTATORS)
                u["writes" if write else "reads"] += 1
                u["files"].add(rel)
                if len(u["examples"]) < 3:
                    u["examples"].append("%s:%d" % (rel, n))

    rows = []
    for s in shims:
        u = use.get(s["name"])
        r = dict(s)
        r["ambiguous"] = int(s["name"] in ambiguous)
        for k in ("reads", "writes", "checks", "indexed", "num_cmp"):
            r[k] = u[k] if u else 0
        r["files"] = len(u["files"]) if u else 0
        r["verify"] = verify.get(s["name"], 0)
        owner = s["owner"]
        rmem = retail.get(owner) or retail.get("U" + owner) or retail.get("A" + owner) or retail.get("F" + owner)
        if rmem is None and owner[:1] in "UAF":
            rmem = retail.get(owner[1:])
        r["in_retail"] = "?" if rmem is None else int(s["name"] in rmem)
        r["examples"] = " ".join(u["examples"]) if u else ""
        used = r["reads"] + r["writes"]
        if s["container"] and (r["checks"] or r["num_cmp"] or r["indexed"]) and used:
            base = "A-fatal"
        elif r["writes"]:
            base = "B-shared-write"
        elif r["reads"]:
            base = "C-dead-read"
        else:
            base = "D-unused"
        r["rank"] = base + ("?" if (r["ambiguous"] and base != "D-unused") else "")
        r["score"] = ({"A-fatal": 3000, "B-shared-write": 2000, "C-dead-read": 1000}.get(base, 0)
                      + 10 * r["checks"] + 5 * r["num_cmp"] + 3 * r["indexed"] + r["writes"] + r["reads"] // 2)
        if r["in_retail"] == 1:
            r["score"] += 500
        if r["ambiguous"] and base != "D-unused":
            r["score"] -= 2500
        rows.append(r)

    rows.sort(key=lambda r: (-r["score"], r["file"], r["line"]))
    out = REPO / args.out
    out.mkdir(parents=True, exist_ok=True)
    cols = ["rank", "score", "owner", "name", "type", "container", "ambiguous", "file", "line",
            "in_retail", "reads", "writes", "checks", "indexed", "num_cmp", "files", "verify", "examples"]
    with (out / "shim_audit.csv").open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols)
        w.writeheader()
        for r in rows:
            w.writerow({c: r.get(c, "") for c in cols})

    counts, per_header = defaultdict(int), defaultdict(lambda: defaultdict(int))
    for r in rows:
        counts[r["rank"]] += 1
        per_header[r["file"]][r["rank"]] += 1
    lines = ["shim audit: %d DISHONORED_SHIM_STATIC declarations in %d headers, %d compiled units, %d build macros"
             % (len(rows), len(per_header), len(units or []), len(macros)), ""]
    for rank in sorted(counts):
        lines.append("%-16s %d" % (rank, counts[rank]))
    lines += ["", "per header, unambiguous only (A / B / C / D):"]
    for h, c in sorted(per_header.items(), key=lambda kv: -(kv[1]["A-fatal"] * 1000 + kv[1]["B-shared-write"] * 10 + kv[1]["C-dead-read"])):
        if c["A-fatal"] or c["B-shared-write"] or c["C-dead-read"]:
            lines.append("  %-44s %3d / %3d / %3d / %3d"
                         % (h.split("/")[-1], c["A-fatal"], c["B-shared-write"], c["C-dead-read"], c["D-unused"]))
    hdr = "%-15s %5s %-46s %-26s %3s %5s %5s %3s %3s %3s %s" % ("rank", "score", "owner::name", "type", "ret", "rd", "wr", "chk", "idx", "num", "first use")

    def table(pred, title, limit):
        got = [r for r in rows if pred(r)][:limit]
        block = ["", title, "", hdr]
        for r in got:
            block.append("%-15s %5d %-46s %-26s %3s %5d %5d %3d %3d %3d %s"
                         % (r["rank"], r["score"], (r["owner"] + "::" + r["name"])[:46], r["type"][:26],
                            r["in_retail"], r["reads"], r["writes"], r["checks"], r["indexed"], r["num_cmp"],
                            r["examples"].split(" ")[0]))
        return block

    lines += table(lambda r: r["rank"] == "A-fatal", "RANK A, unambiguous: a container shim that is indexed / size-compared / checked", 60)
    lines += table(lambda r: r["rank"] == "B-shared-write", "RANK B, unambiguous: written at runtime, so the store is shared process-wide", 60)
    lines += table(lambda r: r["rank"] == "C-dead-read", "RANK C, unambiguous: read and never written -- permanently zero/empty", 40)
    lines += table(lambda r: r["rank"].endswith("?"), "AMBIGUOUS (the identifier is also real storage elsewhere; read by hand)", 30)
    (out / "shim_audit.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines[:8]))
    print("-> %s, %s" % (out / "shim_audit.csv", out / "shim_audit.txt"))


if __name__ == "__main__":
    main()
