"""Independent reader of Dishonored's Wwise 2012.1 content (bank version 65), stdlib only.

Truth source for agent IH1's bank manager port: the C++ CAkBankMgr must report the same per-bank chunk list and
HIRC object counts by type that this script reads straight out of the files.

  python resources/tools/wwise_bank_dump.py                        Init.bnk + every *.pck in CookedPCConsole
  python resources/tools/wwise_bank_dump.py FILE [FILE ...]        .bnk files and/or AKPK containers (.pck, Bk_*.<lang>)
  python resources/tools/wwise_bank_dump.py --all                  also the 670 Bk_*.<lang> dialogue containers
  python resources/tools/wwise_bank_dump.py --hirc FILE            one line per HIRC object (type, id, size)
  python resources/tools/wwise_bank_dump.py --csv OUT.csv          one row per bank (file, bank id, chunks, counts)
  python resources/tools/wwise_bank_dump.py --fnv NAME [NAME ...]  AK::SoundEngine::GetIDFromString of each name
  python resources/tools/wwise_bank_dump.py --find-media ID        which container holds media id ID (DIDX or streamed)

Formats (measured on the retail files; the type numbers are retail CAkBankMgr::ProcessHircChunk's switch,
2012 rva 0x92c320):
  bank      = chunks { char tag[4]; u32 size; u8 body[size] }
  BKHD      = u32 dwBankGeneratorVersion (65), u32 dwSoundBankID, u32 dwLanguageID, u32 bFeedbackInBank, ... (size bytes)
  DIDX      = { u32 id; u32 offset (into DATA); u32 size } * (size / 12)
  HIRC      = u32 count, then count * { u8 type; u32 size; u8 body[size] }, body starts with u32 ulID
  STID      = u32 type, u32 count, count * { u32 bankID; u8 len; char name[len] }
  AKPK      = char 'AKPK'; u32 uHeaderSize; u32 uVersion (1); u32 uLanguageMapSize; u32 uSoundBanksLUTSize;
              u32 uStmFilesLUTSize; u32 uExternalsLUTSize; the four tables follow at 28 and end at 8 + uHeaderSize
              language map = u32 n; n * { u32 offset (from map start); u32 id }; then UTF-16 names
              LUT = u32 n; n * { u32 fileID; u32 uBlockSize; u32 uFileSize; u32 uStartBlock; u32 uLanguageID }
              (fileID is u64 in the externals LUT); a file sits at uStartBlock * uBlockSize from the file start
Name hash = FNV-1 32-bit over the lowercased name (basis 2166136261, prime 16777619; multiply then xor).
"""
import argparse
import csv
import struct
import sys
from collections import Counter
from pathlib import Path

COOKED = Path(r"D:\RecompileDishonored\Dishonored_Latest2026\DishonoredGame\CookedPCConsole")

HIRC_TYPES = {
    1: "State", 2: "Sound", 3: "Action", 4: "Event", 5: "RanSeqCntr", 6: "SwitchCntr", 7: "ActorMixer", 8: "Bus",
    9: "LayerCntr", 10: "MusicSegment", 11: "MusicTrack", 12: "MusicSwitchCntr", 13: "MusicRanSeqCntr",
    14: "Attenuation", 15: "DialogueEvent", 16: "FeedbackBus", 17: "FeedbackNode", 18: "FxShareSet", 19: "FxCustom",
}


def fnv1(name: str) -> int:
    h = 2166136261
    for b in name.lower().encode("latin-1", "replace"):
        h = (h * 16777619) & 0xFFFFFFFF
        h ^= b
    return h


class Bank:
    def __init__(self, data: bytes, label: str):
        self.label = label
        self.size = len(data)
        self.chunks = []
        self.version = self.bank_id = self.language_id = None
        self.hirc = []
        self.didx = []
        self.stid = []
        self.errors = []
        pos = 0
        while pos + 8 <= len(data):
            tag = data[pos:pos + 4].decode("latin-1")
            (size,) = struct.unpack_from("<I", data, pos + 4)
            body = data[pos + 8:pos + 8 + size]
            if len(body) != size:
                self.errors.append(f"chunk {tag} at {pos} truncated ({len(body)} of {size})")
            self.chunks.append((tag, size))
            getattr(self, "_chunk_" + tag, lambda b: None)(body)
            pos += 8 + size
        if pos != len(data):
            self.errors.append(f"{len(data) - pos} trailing bytes")

    def _chunk_BKHD(self, b: bytes) -> None:
        self.version, self.bank_id, self.language_id = struct.unpack_from("<III", b, 0)

    def _chunk_DIDX(self, b: bytes) -> None:
        self.didx = [struct.unpack_from("<III", b, i) for i in range(0, len(b) - len(b) % 12, 12)]

    def _chunk_HIRC(self, b: bytes) -> None:
        (count,) = struct.unpack_from("<I", b, 0)
        pos = 4
        for _ in range(count):
            if pos + 5 > len(b):
                self.errors.append("HIRC truncated")
                return
            typ = b[pos]
            (size,) = struct.unpack_from("<I", b, pos + 1)
            (oid,) = struct.unpack_from("<I", b, pos + 5) if size >= 4 else (0,)
            self.hirc.append((typ, oid, size))
            pos += 5 + size
        if pos != len(b):
            self.errors.append(f"HIRC: {len(b) - pos} bytes after the last object")

    def _chunk_STID(self, b: bytes) -> None:
        _, count = struct.unpack_from("<II", b, 0)
        pos = 8
        for _ in range(count):
            bid, ln = struct.unpack_from("<IB", b, pos)
            self.stid.append((bid, b[pos + 5:pos + 5 + ln].decode("latin-1")))
            pos += 5 + ln

    def counts(self) -> Counter:
        return Counter(HIRC_TYPES.get(t, f"type{t}") for t, _, _ in self.hirc)

    def summary(self) -> str:
        c = self.counts()
        counts = " ".join(f"{k}={c[k]}" for k in sorted(c))
        chunks = ",".join(t for t, _ in self.chunks)
        err = f" ERRORS: {'; '.join(self.errors)}" if self.errors else ""
        return (f"{self.label}: v{self.version} id 0x{self.bank_id or 0:08x} {self.size} bytes chunks [{chunks}] "
                f"HIRC {len(self.hirc)}: {counts} DIDX {len(self.didx)}{err}")


class Package:
    def __init__(self, path: Path):
        self.path = path
        data = path.read_bytes()
        self.data = data
        if data[:4] != b"AKPK":
            raise ValueError(f"{path}: not an AKPK container")
        header_size, self.version, lang_size, banks_size, stm_size, externals_size = struct.unpack_from("<6I", data, 4)
        if 28 + lang_size + banks_size + stm_size + externals_size != 8 + header_size:
            raise ValueError(f"{path}: AKPK table sizes do not add up to the header size {header_size}")
        table = 28
        self.languages = self._languages(data, table, lang_size)
        self.banks = self._lut(data, table + lang_size, banks_size, False)
        self.streamed = self._lut(data, table + lang_size + banks_size, stm_size, False)
        self.externals = self._lut(data, table + lang_size + banks_size + stm_size, externals_size, True) if externals_size else []

    @staticmethod
    def _languages(data: bytes, at: int, size: int) -> dict:
        if size < 4:
            return {}
        (n,) = struct.unpack_from("<I", data, at)
        out = {}
        for i in range(n):
            off, lid = struct.unpack_from("<II", data, at + 4 + 8 * i)
            s = at + off
            e = s
            while e + 1 < at + size and data[e:e + 2] != b"\0\0":
                e += 2
            out[lid] = data[s:e].decode("utf-16-le", "replace")
        return out

    @staticmethod
    def _lut(data: bytes, at: int, size: int, wide_id: bool) -> list:
        if size < 4:
            return []
        (n,) = struct.unpack_from("<I", data, at)
        fmt, step = ("<QIIII", 24) if wide_id else ("<IIIII", 20)
        return [struct.unpack_from(fmt, data, at + 4 + step * i) for i in range(n)]

    def bank_bytes(self, entry) -> bytes:
        fid, block, size, start, _ = entry
        off = start * block
        return self.data[off:off + size]


def load(path: Path) -> list:
    if path.suffix.lower() == ".bnk":
        return [Bank(path.read_bytes(), path.name)]
    pkg = Package(path)
    out = []
    for entry in pkg.banks:
        out.append(Bank(pkg.bank_bytes(entry), f"{path.name}#0x{entry[0]:08x}"))
    return out


def default_files(include_bk: bool) -> list:
    files = [COOKED / "Init.bnk"] + sorted(COOKED.glob("*.pck"))
    if include_bk:
        files += sorted(p for p in COOKED.glob("Bk_*.*") if p.is_file())
    return files


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--hirc", action="store_true")
    ap.add_argument("--csv")
    ap.add_argument("--fnv", nargs="+")
    ap.add_argument("--find-media", type=lambda s: int(s, 0))
    args = ap.parse_args()

    if args.fnv:
        for n in args.fnv:
            print(f"0x{fnv1(n):08x} {fnv1(n):10d} {n}")
        return 0

    files = [Path(f) for f in args.files] or default_files(args.all)
    if args.find_media is not None:
        for f in files:
            if f.suffix.lower() == ".bnk":
                for b in load(f):
                    for mid, off, size in b.didx:
                        if mid == args.find_media:
                            print(f"{b.label}: DIDX offset {off} size {size}")
                continue
            pkg = Package(f)
            for fid, block, size, start, lang in pkg.streamed:
                if fid == args.find_media:
                    print(f"{f.name}: streamed file offset {start * block} size {size} language {lang}")
            for b in load(f):
                for mid, off, size in b.didx:
                    if mid == args.find_media:
                        print(f"{b.label}: DIDX offset {off} size {size}")
        return 0

    total = Counter()
    rows = []
    banks = 0
    errors = 0
    for f in files:
        for b in load(f):
            banks += 1
            errors += bool(b.errors)
            total.update(b.counts())
            print(b.summary())
            if args.hirc:
                for typ, oid, size in b.hirc:
                    print(f"  {HIRC_TYPES.get(typ, typ)} 0x{oid:08x} {size}")
            c = b.counts()
            rows.append({"bank": b.label, "bank_id": f"0x{b.bank_id or 0:08x}", "version": b.version, "size": b.size,
                         "chunks": ",".join(t for t, _ in b.chunks), "hirc": len(b.hirc), "didx": len(b.didx),
                         **{v: c.get(v, 0) for v in HIRC_TYPES.values()}})
    print(f"\n{banks} banks in {len(files)} files, {errors} with parse errors; HIRC total {sum(total.values())}: "
          + " ".join(f"{k}={total[k]}" for k in sorted(total)))
    if args.csv:
        with open(args.csv, "w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
            w.writeheader()
            w.writerows(rows)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
