"""FaceFX 1.7.3.1 actor archive reader (agent FX2, wave 26): the face graph, the master bone list and the animations.

    python resources/tools/facefx_graph_dump.py <archive.fxa> [...]          one summary line per archive
    python resources/tools/facefx_graph_dump.py --nodes <archive.fxa>         every compiled node, its links and user properties
    python resources/tools/facefx_graph_dump.py --csv <out.csv> <archive.fxa> [...]   the counts CoreSmoke compares against

An FxAnimSet archive (UFaceFXAnimSet::RawFaceFXAnimSetBytes, the dialogue data) is told apart by its directory
(class FxAnimSet) and summarised by group, anims, curves and keys (FxAnimSet::Serialize 0xb15950).

An archive is UFaceFXAsset::RawFaceFXActorBytes (the cooked heads carry them; FX1's
build/agentFX1/py/extract_facefx.py writes them out as .fxa). The layout follows the 2012 decompile:
FxArchive::Open 0xb180b0 (header, directory), operator<<(FxArchive&, FxArchiveDirectory&) 0xb17f10,
FxStringHash::Serialize 0xb1ab10, operator<<(FxArchive&, FxName&) 0xb05890, FxStringBase::Serialize 0xb062a0,
FxArray::Serialize 0xb24c60, FxActor::Serialize 0xb0cdf0, operator<<(FxArchive&, FxMasterBoneList&) 0xb0ee30,
operator<<(FxArchive&, FxCompiledFaceGraph&) 0xb12220 and its node/link 0xb111d0/0xb10d50, FxLinkFnParameters
0xb26610, FxFaceGraphNodeUserProperty::Serialize 0xb29980, FxAnimGroup::Serialize 0xb08f10, FxAnim::Serialize
0xb08520, FxAnimCurve::Serialize 0xb16a70, FxPhonemeMap::Serialize 0xb1de20. Class versions come from the
directory (sdk >= 0x6A4: SerializeClassVersion reads nothing from the stream). Shared with FX1: when FX1 lands
resources/tools/facefx_archive_dump.py, this file is the face-graph half of it.
"""
import csv
import struct
import sys
from pathlib import Path

NODE_TYPES = ["Combiner", "Delta", "CurrentTime", "GenericTarget", "BonePose", "MorphTarget",
              "MaterialParameterUE3", "MorphTargetUE3"]
LINK_FNS = ["null", "linear", "quadratic", "cubic", "sqrt", "negate", "inverse", "oneclamp", "constant",
            "corrective", "clamped linear"]


class Archive:
    def __init__(self, data: bytes):
        self.d = data
        self.o = 0
        self.objects = {}
        if data[:3] != b"FAC" or data[3:4] not in (b"E", b"B"):
            raise ValueError("not a FaceFX archive")
        self.endian = "<" if data[3:4] == b"E" else ">"
        self.o = 4
        self.sdk = self.u32()
        self.file_format = self.u32() if self.sdk >= 0x6B8 else 0
        if self.sdk < 0x6A4:
            self.u16()
        self.licensee = self.raw(self.u32())
        if self.sdk < 0x6A4:
            self.u16()
        self.project = self.raw(self.u32())
        self.licensee_version = self.u32()
        self.class_versions = {}
        self.names = []
        self.num_object_pointers = 0
        if self.sdk >= 0x640:
            dir_version = self.u16()
            if dir_version:
                self.class_versions = dict(self.string_hash(self.u16))
            self.names = self.array(self.archive_name)
            self.num_object_pointers = self.u32()
            if "FxDigitalAudio" in self.class_versions:
                self.class_versions["FxAudio"] = self.class_versions.pop("FxDigitalAudio")

    def unpack(self, fmt, size):
        v = struct.unpack_from(self.endian + fmt, self.d, self.o)
        self.o += size
        return v[0]

    def u16(self):
        return self.unpack("H", 2)

    def u32(self):
        return self.unpack("I", 4)

    def i32(self):
        return self.unpack("i", 4)

    def f32(self):
        return self.unpack("f", 4)

    def raw(self, n):
        v = self.d[self.o:self.o + n]
        self.o += n
        return v

    def version(self, cls):
        if self.sdk < 0x6A4:
            return self.u16()
        return self.class_versions.get(cls, 0)

    def string(self):
        if self.sdk < 0x6A4:
            self.u16()
        return self.raw(self.u32()).decode("latin-1")

    def archive_name(self):
        if self.sdk < 0x6A4:
            self.u16()
        return self.string()

    def name(self):
        if self.version("FxName"):
            idx = self.u32()
            return self.names[idx] if idx < len(self.names) else None
        return self.string()

    def array(self, elem):
        if self.sdk < 0x6A4:
            self.u16()
        return [elem() for _ in range(self.u32())]

    def string_hash(self, value):
        out = []
        for _ in range(self.u32()):
            self.u32()
            for _ in range(self.u32()):
                key = self.string()
                out.append((key, value()))
        return out

    def floats(self, n):
        v = struct.unpack_from(self.endian + "%df" % n, self.d, self.o)
        self.o += 4 * n
        return list(v)

    def object_pointer(self, readers):
        idx = self.u32()
        if idx == 0xFFFFFFFF:
            return None
        if idx in self.objects:
            return self.objects[idx]
        cls = self.name() if self.sdk >= 0x640 else self.string()
        self.u32()
        obj = readers[cls]()
        self.objects[idx] = obj
        return obj


def named_object(a):
    a.version("FxObject")
    a.version("FxNamedObject")
    return a.name()


def bone(a):
    name = named_object(a)
    a.version("FxBone")
    return {"name": name, "pos": a.floats(3), "rot": a.floats(4), "scale": a.floats(3)}


def master_bone_list(a):
    if a.version("FxMasterBoneList") < 2:
        raise ValueError("pre-2 master bone list")

    def link():
        return {"node": a.u32(), "bone": bone(a)}

    def entry():
        if a.sdk < 0x6A4:
            a.u16()
        return {"bone": bone(a), "inv_rot": a.floats(4), "index": a.i32(), "ref_weight": a.f32(), "links": a.array(link)}

    return a.array(entry)


def link_fn_parameters(a):
    ver = a.version("FxLinkFnParameters")
    params = a.array(a.f32)
    if not ver:
        raise ValueError("custom curve link parameters")
    return params


def user_property(a):
    name = named_object(a)
    a.version("FxFaceGraphNodeUserProperty")
    return {"name": name, "type": a.i32(), "int": a.i32(), "real": a.f32(), "choices": a.array(a.string)}


def compiled_face_graph(a):
    a.version("FxCompiledFaceGraph")

    def link():
        idx = a.u32()
        fn = a.i32()
        params = link_fn_parameters(a)
        return {"node": idx, "fn": fn, "params": [] if fn == 0 else params}

    def node():
        return {"type": a.i32(), "name": a.name(), "min": a.f32(), "inv_min": a.f32(), "max": a.f32(),
                "inv_max": a.f32(), "op": a.i32(), "links": a.array(link), "props": a.array(lambda: user_property(a))}

    nodes = a.array(node)
    node_hash = a.string_hash(a.u32)
    return nodes, node_hash


def anim_curve(a):
    name = named_object(a)
    ver = a.version("FxAnimCurve")
    interp = a.i32()
    keys = a.array(lambda: (a.u16(), a.floats(4))) if not ver else []
    return {"name": name, "interp": interp, "keys": keys}


def anim(a):
    name = named_object(a)
    ver = a.version("FxAnim")
    if ver < 4:
        a.u32()
    curves = a.array(lambda: anim_curve(a))
    if ver >= 6:
        num_keys = a.u32()
        if num_keys:
            flat = a.floats(4 * num_keys)
            counts = a.array(a.u32)
            off = 0
            for c, n in zip(curves, counts):
                c["keys"] = [flat[off + 4 * k: off + 4 * k + 4] for k in range(n)]
                off += 4 * n
    if ver < 5:
        a.u16()
        a.u32()
    out = {"name": name, "curves": curves, "blend_in": a.f32(), "blend_out": a.f32()}
    out["bone_weights"] = a.array(lambda: (a.version("FxAnimBoneWeight"), a.name(), a.f32())[1:]) if ver >= 2 else []
    if 1 <= ver < 8:
        a.string()
        a.string()
    if 3 <= ver < 8:
        a.u32()
    if ver >= 7:
        out["event_path"] = a.string()
        out["event_wav"] = a.string()
        out["event_index"] = a.u32()
    return out


def anim_group(a):
    name = named_object(a)
    a.version("FxAnimGroup")
    return {"name": name, "anims": a.array(lambda: anim(a))}


def phoneme_map(a):
    a.version("FxObject")
    a.version("FxPhonemeMap")
    mapping = a.array(lambda: (a.i32(), a.name(), a.f32()))
    targets = a.array(a.name)
    return mapping, targets


def actor(a):
    name = named_object(a)
    ver = a.version("FxActor")
    out = {"name": name, "version": ver, "bones": master_bone_list(a)}
    if ver < 3:
        raise ValueError("decompiled face graph archive (FxActor < 3)")
    out["nodes"], out["node_hash"] = compiled_face_graph(a)
    if ver < 2:
        raise ValueError("FxActor < 2")
    out["default_group"] = anim_group(a)
    out["groups"] = a.array(lambda: anim_group(a))
    if ver:
        out["phoneme_map"] = phoneme_map(a)
    return out


def anim_set(a):
    name = named_object(a)
    a.version("FxAnimSet")
    owner = a.name()
    return {"name": name, "owner": owner, "group": anim_group(a)}


def load(path):
    data = Path(path).read_bytes()
    a = Archive(data)
    act = anim_set(a) if "FxAnimSet" in a.class_versions else actor(a)
    act["bytes_read"] = a.o
    act["bytes_total"] = len(data)
    act["sdk"] = a.sdk
    act["class_versions"] = a.class_versions
    return act


def counts(act):
    if "group" in act:
        anims = act["group"]["anims"]
        return {"animset": act["name"], "owner": act["owner"], "group": act["group"]["name"], "anims": len(anims),
                "curves": sum(len(an["curves"]) for an in anims),
                "keys": sum(len(c["keys"]) for an in anims for c in an["curves"]),
                "bytes_read": act["bytes_read"], "bytes_total": act["bytes_total"]}
    by_type = [0] * len(NODE_TYPES)
    links = 0
    for n in act["nodes"]:
        by_type[n["type"]] += 1
        links += len(n["links"])
    anims = len(act["default_group"]["anims"]) + sum(len(g["anims"]) for g in act["groups"])
    curves = sum(len(an["curves"]) for an in act["default_group"]["anims"])
    curves += sum(len(an["curves"]) for g in act["groups"] for an in g["anims"])
    return {"actor": act["name"], "nodes": len(act["nodes"]), "links": links,
            **{"nt_" + NODE_TYPES[i]: by_type[i] for i in range(len(NODE_TYPES))},
            "bones": len(act["bones"]), "bone_links": sum(len(b["links"]) for b in act["bones"]),
            "groups": 1 + len(act["groups"]), "anims": anims, "curves": curves,
            "phoneme_mappings": len(act["phoneme_map"][0]) if "phoneme_map" in act else 0,
            "bytes_read": act["bytes_read"], "bytes_total": act["bytes_total"]}


def main(argv):
    if not argv:
        print(__doc__)
        return 2
    if argv[0] == "--nodes":
        act = load(argv[1])
        for i, n in enumerate(act["nodes"]):
            links = ", ".join("%s<-%s(%s)" % (LINK_FNS[l["fn"]] if 0 <= l["fn"] < len(LINK_FNS) else l["fn"],
                                               act["nodes"][l["node"]]["name"], ",".join("%g" % p for p in l["params"]))
                              for l in n["links"])
            props = ", ".join("%s=%s" % (p["name"], p["choices"][0] if p["type"] == 3 and p["choices"] else p["int"]) for p in n["props"])
            print("%4d %-14s %-36s [%g,%g] op%d %s %s" % (i, NODE_TYPES[n["type"]], n["name"], n["min"], n["max"], n["op"], links, props))
        for b in act["bones"]:
            print("bone %-24s index %d links %s" % (b["bone"]["name"], b["index"], [l["node"] for l in b["links"]]))
        return 0
    out_csv = None
    if argv[0] == "--csv":
        out_csv = argv[1]
        argv = argv[2:]
    rows = []
    for p in argv:
        c = counts(load(p))
        c["file"] = Path(p).name
        rows.append(c)
        print(" ".join("%s=%s" % kv for kv in c.items()))
    if out_csv:
        with open(out_csv, "w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            w.writeheader()
            w.writerows(rows)
    return 0 if all(r["bytes_read"] == r["bytes_total"] for r in rows) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
