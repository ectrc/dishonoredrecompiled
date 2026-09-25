"""Extract the Edge animation data of cooked Dishonored packages (Ver 801 / LicenseeVer 30) for the
EdgeAnimSmoke test (source/Tests/EdgeAnimSmoke) and resources/docs/edgeanim.md.

Per export (after UObject::Serialize = INT NetIndex + tagged properties ending in "None"):
  AnimSequence  TArray<FRawAnimSequenceTrack> RawAnimationData (cooked: empty); INT NumBytes; BYTE[NumBytes]
                (UAnimSequence::Serialize, 2013 rva 0x34c0d0: raw copy when the codec is NULL, i.e. ACF_EdgeAnim)
  AnimSet       tagged only: TrackBoneNames, Sequences, ...
  SkeletalMesh  m_UserBounds (FName, FVector, FLOAT) ; FBoxSphereBounds ; TArray<UMaterialInterface*> ;
                FVector Origin ; FRotator RotOrigin ; TArray<BYTE> m_EdgeSkeleton ; TArray<FMeshBone> RefSkeleton
                (USkeletalMesh::Serialize, 2013 rva 0x355260; FMeshBone = FName, DWORD Flags, FQuat, FVector,
                INT NumChildren, INT ParentIndex, FColor = 52 bytes)

Usage:
  python extract_edgeanim.py <pkg.upk> [...] --out <dir> [--filter <substring>]
Writes <dir>/index.json (sequences, sets, meshes) and one .bin per Edge blob / skeleton.
"""
import argparse
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from read_package_classes import FieldReader, Package  # noqa: E402

ACF = ["ACF_None", "ACF_Float96NoW", "ACF_Fixed48NoW", "ACF_IntervalFixed32NoW", "ACF_Fixed32NoW", "ACF_Float32NoW", "ACF_Identity", "ACF_EdgeAnim"]


def read_tagged(r: FieldReader, wanted: set) -> dict:
    props = {}
    while True:
        name = r.fname()
        if name == "None":
            return props
        ptype = r.fname()
        size = r.i32()
        index = r.i32()
        if ptype == "StructProperty":
            r.fname()
        elif ptype == "BoolProperty":
            props[name] = r.u8()
            continue
        elif ptype == "ByteProperty":
            enum = r.fname()
            if enum != "None":
                props[name] = r.fname()
                continue
        start = r.o
        if name in wanted:
            if ptype == "ArrayProperty" and name == "CompressedTrackOffsets":
                props[name] = r.i32()
            elif ptype == "IntProperty":
                props[name] = r.i32()
            elif ptype == "FloatProperty":
                props[name] = struct.unpack_from("<f", r.d, r.o)[0]
            elif ptype == "NameProperty":
                props[name] = r.fname()
            elif ptype == "ObjectProperty":
                props[name] = r.objref()
            elif ptype == "ByteProperty":
                props[name] = r.u8()
            elif ptype == "ArrayProperty":
                count = r.i32()
                inner = size - 4
                if count and inner == 8 * count:
                    props[name] = [r.fname() for _ in range(count)]
                elif count and inner == 4 * count:
                    props[name] = [r.objref() for _ in range(count)]
                else:
                    props[name] = {"count": count, "bytes": inner}
        r.o = start + size


def export_reader(pkg: Package, e: dict) -> FieldReader:
    r = FieldReader(pkg, pkg.read(e["serial_offset"], e["serial_size"]))
    r.i32()
    return r


def safe(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", name)


def main(argv) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("packages", nargs="+", type=Path)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--filter", default="")
    args = ap.parse_args(argv)
    args.out.mkdir(parents=True, exist_ok=True)
    index = {"sequences": [], "sets": [], "meshes": [], "errors": []}
    for path in args.packages:
        pkg = Package(path)
        for e in pkg.exports:
            cls = pkg.class_name_of(e)
            if cls not in ("AnimSequence", "AnimSet", "SkeletalMesh"):
                continue
            full = pkg.object_path(e["index"] + 1)
            if ".Default__" in full or (args.filter and args.filter not in full):
                continue
            r = export_reader(pkg, e)
            try:
                read_export(pkg, cls, full, r, args.out, index)
            except (struct.error, IndexError) as ex:
                index["errors"].append({"path": full, "class": cls, "error": str(ex)})
    (args.out / "index.json").write_text(json.dumps(index, indent=1), encoding="utf-8")
    print(f"{len(index['sequences'])} sequences, {len(index['sets'])} sets, {len(index['meshes'])} meshes, {len(index['errors'])} errors -> {args.out}")
    return 0


def read_export(pkg, cls, full, r, out, index):
    if cls == "AnimSequence":
        p = read_tagged(r, {"SequenceName", "NumFrames", "SequenceLength", "RateScale", "bIsAdditive",
                            "TranslationCompressionFormat", "RotationCompressionFormat", "KeyEncodingFormat",
                            "CompressedTrackOffsets", "EncodingPkgVersion", "bNoLoopingInterpolation"})
        raw_tracks = r.i32()
        if raw_tracks:
            p["raw_tracks"] = raw_tracks
            index["sequences"].append({"path": full, "package": pkg.name, "props": p, "error": "raw tracks present"})
            return
        num = r.i32()
        blob = r.d[r.o : r.o + num]
        entry = {"path": full, "package": pkg.name, "props": p, "num_bytes": num, "trailing": len(r.d) - r.o - num}
        if num:
            fn = safe(full) + ".anim"
            (out / fn).write_bytes(blob)
            entry["file"] = fn
            entry["tag"] = blob[:4].hex()
        index["sequences"].append(entry)
    elif cls == "AnimSet":
        p = read_tagged(r, {"TrackBoneNames", "Sequences", "bAnimRotationOnly", "UseTranslationBoneNames",
                            "ForceMeshTranslationBoneNames", "PreviewSkelMeshName"})
        index["sets"].append({"path": full, "package": pkg.name, "props": p})
    else:
        read_tagged(r, set())
        r.fname()
        r.o += 12
        r.o += 4
        r.o += 28
        materials = r.i32()
        r.o += 4 * materials
        r.o += 12 + 12
        n = r.i32()
        skel = r.d[r.o : r.o + n]
        r.o += n
        bones = []
        for _ in range(r.i32()):
            name = r.fname()
            flags = r.u32()
            q = struct.unpack_from("<4f", r.d, r.o)
            t = struct.unpack_from("<3f", r.d, r.o + 16)
            r.o += 28
            nchild = r.i32()
            parent = r.i32()
            r.o += 4
            bones.append({"name": name, "q": q, "t": t, "children": nchild, "parent": parent})
        entry = {"path": full, "package": pkg.name, "edge_skeleton_bytes": n, "bones": bones}
        if n:
            fn = safe(full) + ".skel"
            (out / fn).write_bytes(skel)
            entry["file"] = fn
        index["meshes"].append(entry)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
