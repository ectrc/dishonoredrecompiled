"""Independent reader of FaceFX 1.7.x archives (the bytes of UFaceFXAsset::RawFaceFXActorBytes /
UFaceFXAnimSet::RawFaceFXAnimSetBytes), stdlib only. Agent FX1 (wave 26); the C++ side it checks is
source/Development/Src/External/FaceFX (CoreSmoke's FaceFXSmoke.cpp).

  python resources/tools/facefx_archive_dump.py <file.fxa> [...]            human-readable dump
  python resources/tools/facefx_archive_dump.py --smoke <file.fxa> [...]    writes <file>.txt next to each archive
                                                                            in the line format FaceFXSmoke reads
  python resources/tools/facefx_archive_dump.py --extract <out_dir> <package glob> [name filter]
                                                                            pulls the archives out of cooked packages
                                                                            (needs resources/tools/pdb/read_package_classes.py)

Format (FaceFX SDK 1.7.3.1 as compiled into Dishonored, fxarchive.cpp):
  'F' 'A' 'C' 'E'|'B'  u32 sdkVersion  [u32 fileFormatVersion if sdk >= 0x6B8]
  u32 n + n bytes licensee name (n counts the terminator), u32 n + n bytes project name, u32 licensee version
  directory (sdk >= 0x640): u16 version, [if version] class hash: u32 binsInUse, per bin {u32 bin, u32 len,
  len x {string, u16 classVersion}}; names: u32 count, count x string; u32 numObjectPointers
  string = u32 length + chars (no terminator); u16 versions in front of strings/arrays only before sdk 0x6A4.
The hash bin of a key is djb2 over its signed chars (5381, h = c + 33*h) masked to the table's bits:
7 for the archive class table, 12 for the FxName table. Names are 'proofed' before they enter the name table:
'|' and ' ' become '_', '"' becomes "'".
"""
import struct
import sys
from pathlib import Path


def djb2(key: bytes, bits: int) -> int:
    h = 5381
    for c in key:
        h = (h * 33 + (c - 256 if c >= 128 else c)) & 0xFFFFFFFF
    return h & ((1 << bits) - 1)


def proof(name: str) -> str:
    return name[:512].replace("|", "_").replace(" ", "_").replace('"', "'")


class Reader:
    def __init__(self, data: bytes):
        self.d = data
        self.o = 0
        self.e = "<"

    def u16(self):
        v = struct.unpack_from(self.e + "H", self.d, self.o)[0]
        self.o += 2
        return v

    def u32(self):
        v = struct.unpack_from(self.e + "I", self.d, self.o)[0]
        self.o += 4
        return v

    def raw(self, n):
        v = self.d[self.o:self.o + n]
        self.o += n
        return v

    def string(self, sdk):
        if sdk < 0x6A4:
            self.u16()
        return self.raw(self.u32())


def parse(data: bytes) -> dict:
    r = Reader(data)
    magic = r.raw(4)
    if magic[:3] != b"FAC" or magic[3:] not in (b"E", b"B"):
        raise ValueError(f"not a FaceFX archive: {magic!r}")
    if magic[3:] == b"B":
        r.e = ">"
    out = {"size": len(data), "byteorder": magic[3:].decode()}
    sdk = out["sdk"] = r.u32()
    out["format"] = r.u32() if sdk >= 0x6B8 else 0
    if sdk < 0x6A4:
        r.u16()
    out["licensee"] = r.raw(r.u32()).rstrip(b"\0").decode("latin-1")
    if sdk < 0x6A4:
        r.u16()
    out["project"] = r.raw(r.u32()).rstrip(b"\0").decode("latin-1")
    out["licversion"] = r.u32()
    out["classes"] = []
    out["names"] = []
    out["objects"] = 0
    if sdk >= 0x640:
        out["dirversion"] = r.u16()
        if out["dirversion"]:
            for _ in range(r.u32()):
                b = r.u32()
                for _ in range(r.u32()):
                    key = r.string(sdk)
                    ver = r.u16()
                    if djb2(key, 7) != b:
                        raise ValueError(f"class {key!r} stored in bin {b}, djb2 gives {djb2(key, 7)}")
                    out["classes"].append((b, ver, key.decode("latin-1")))
        if sdk < 0x6A4:
            r.u16()
        out["names"] = [r.string(sdk).decode("latin-1") for _ in range(r.u32())]
        out["objects"] = r.u32()
    out["dirend"] = r.o
    out["actorname"] = struct.unpack_from(r.e + "I", data, r.o)[0] if r.o + 4 <= len(data) else -1
    return out


def name_hash_sim(names):
    """FxStringHash<...,12,FxList>::Insert over the names in order: bins in use, insertions, collisions."""
    bins = {}
    insertions = collisions = 0
    for n in names:
        key = proof(n).encode("latin-1")
        b = djb2(key, 12)
        entries = bins.setdefault(b, [])
        if key in entries:
            continue
        if entries:
            collisions += 1
        insertions += 1
        entries.append(key)
    return len(bins), insertions, collisions


def smoke_text(path: Path, a: dict) -> str:
    lines = [f"file {path.name}", f"size {a['size']}", f"sdk {a['sdk']}", f"format {a['format']}",
             f"licensee {a['licensee']}", f"project {a['project']}", f"licversion {a['licversion']}",
             f"dirversion {a.get('dirversion', 0)}", f"classes {len(a['classes'])}"]
    lines += [f"class {b} {v} {n}" for b, v, n in a["classes"]]
    lines.append(f"names {len(a['names'])}")
    lines += [f"name {i} {djb2(proof(n).encode('latin-1'), 12)} {n}" for i, n in enumerate(a["names"])]
    used, ins, col = name_hash_sim(a["names"])
    lines += [f"namehash {used} {ins} {col}", f"objects {a['objects']}", f"dirend {a['dirend']}",
              f"actorname {a['actorname']}"]
    return "\n".join(lines) + "\n"


def extract(out_dir: Path, pattern: str, flt: str):
    sys.path.insert(0, str(Path(__file__).resolve().parent / "pdb"))
    from read_package_classes import Package, FieldReader
    cooked = Path(r"D:/RecompileDishonored/Dishonored_Latest2026/DishonoredGame/CookedPCConsole")
    out_dir.mkdir(parents=True, exist_ok=True)
    for p in sorted(cooked.glob(pattern)):
        pkg = Package(p)
        for e in pkg.exports:
            if pkg.class_name_of(e) != "FaceFXAsset":
                continue
            path = pkg.object_path(e["index"] + 1)
            if flt.lower() not in path.lower():
                continue
            data = pkg.read(e["serial_offset"], e["serial_size"])
            r = FieldReader(pkg, data)
            r.o = 26 if e["object_flags"] & 0x0200000000000000 else 4
            while True:
                if r.fname() == "None":
                    break
                ptype = r.fname()
                size = r.i32()
                r.i32()
                if ptype in ("StructProperty", "ByteProperty"):
                    r.fname()
                elif ptype == "BoolProperty":
                    r.u8()
                r.o += size
            n = struct.unpack_from("<i", data, r.o)[0]
            target = out_dir / (path.split(".")[-1] + ".fxa")
            if not target.exists():
                target.write_bytes(data[r.o + 4:r.o + 4 + n])
            print(f"{p.name}\t{path}\t{n} bytes")


def main(argv):
    if not argv:
        print(__doc__)
        return 2
    if argv[0] == "--extract":
        extract(Path(argv[1]), argv[2], argv[3] if len(argv) > 3 else "")
        return 0
    smoke = argv[0] == "--smoke"
    for f in argv[1:] if smoke else argv:
        path = Path(f)
        a = parse(path.read_bytes())
        if smoke:
            path.with_suffix(".txt").write_text(smoke_text(path, a), encoding="latin-1")
            print(f"{path.name}: sdk {a['sdk']}, {len(a['classes'])} classes, {len(a['names'])} names, "
                  f"{a['objects']} objects, directory ends at {a['dirend']}")
        else:
            print(smoke_text(path, a), end="")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
