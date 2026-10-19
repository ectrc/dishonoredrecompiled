"""Independent reference decode of a Wwise 2012.1 Vorbis media item (agent IH2, wave 26).

Rewraps one media item of a bank into a standard Ogg Vorbis stream (ww2ogg's method) and decodes it with ffmpeg, so the
ported decoder (source/Development/Src/External/Wwise2012/Src/AkVorbisDecoder) can be checked against a decoder that
shares no code with it.

  python resources/tools/wwise_vorbis_ref.py --container Bk_E43C0E7F49D6DA8943110CA3F7328499.INT --media 0x5c7ca16c
         [--ogg OUT.ogg] [--pcm OUT.s16] [--blocks 1024]

Prints: channels, rate, total frames (fmt chunk), the frame count the Wwise decoder will produce (sum of packet overlaps
minus uLastGranuleExtra), ffmpeg's decoded frame count, sum / crc32 of the s16 samples, and the RMS of every --blocks
frames (the envelope CoreSmoke compares, since two float decoders differ by rounding and a bit-exact checksum cannot
match).

Format (measured on the retail files, cross-checked with the 2012 decompile of CAkSrcBankVorbis / vorbis_unpack_books):
  RIFF 'WAVE': 'fmt ' (66 bytes: WAVEFORMATEXTENSIBLE 24 bytes with wFormatTag 0xFFFF, then dwTotalPCMFrames and the 38-byte
  AkVorbisInfo: loop info, dwSeekTableSize, dwVorbisDataOffset, uMaxPacketSize, uLastGranuleExtra, dwDecodeAllocSize,
  dwDecodeX64AllocSize, uHashCodebook, uBlockSizes[2]), then 'data' = seek table, setup packet, audio packets; every packet
  is a little-endian u16 size + payload. The setup packet is Audiokinetic's packed setup: codebooks are 10-bit indices into
  g_packedCodebooks (retail exe: g_packedCodebookData 2013 rva 0xdd89f8 / 71991 bytes, g_packedCodebooks 2013 rva 0xfb1278,
  599 pointers; 2012 PDB rvas 0xd90d68 / 0xe3df78), floors have no type field, residue types are 2 bits, mappings have no
  type field, modes are blockflag + mapping. Audio packets start with the mode number (no packet-type bit, no window flags).
"""
import argparse
import hashlib
import struct
import subprocess
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import wwise_bank_dump as dump  # noqa: E402

FFMPEG = Path(r"C:\Users\Frederik\AppData\Local\Microsoft\WinGet\Packages\Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe"
              r"\ffmpeg-8.1-full_build\bin\ffmpeg.exe")
EXE = Path(r"D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\Dishonored.exe")
CODEBOOK_DATA_RVA, CODEBOOK_DATA_SIZE, CODEBOOK_DATA_SHA1 = 0xdd89f8, 71991, "9183f2c09ec9dfea32d83ad27eee4c34d88bdf09"
CODEBOOK_PTRS_RVA, CODEBOOK_COUNT = 0xfb1278, 599
IMAGEBASE = 0x400000


def pe_read(data: bytes, rva: int, size: int) -> bytes:
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    sec = pe + 24 + struct.unpack_from("<H", data, pe + 20)[0]
    for i in range(nsec):
        _, vsize, va, rsize, raw = struct.unpack_from("<8sIIII", data, sec + 40 * i)
        if va <= rva < va + max(vsize, rsize):
            return data[raw + rva - va:raw + rva - va + size]
    raise ValueError(f"rva 0x{rva:x}")


def packed_codebooks(exe: Path) -> list:
    data = exe.read_bytes()
    blob = pe_read(data, CODEBOOK_DATA_RVA, CODEBOOK_DATA_SIZE)
    if hashlib.sha1(blob).hexdigest() != CODEBOOK_DATA_SHA1:
        raise SystemExit("g_packedCodebookData does not match the 2012 PDB symbol")
    ptrs = struct.unpack(f"<{CODEBOOK_COUNT}I", pe_read(data, CODEBOOK_PTRS_RVA, 4 * CODEBOOK_COUNT))
    offs = [p - IMAGEBASE - CODEBOOK_DATA_RVA for p in ptrs]
    return [blob[o:] for o in offs]


class BitReader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def read(self, bits: int) -> int:
        v = 0
        for i in range(bits):
            byte = self.data[self.pos >> 3] if (self.pos >> 3) < len(self.data) else 0
            v |= ((byte >> (self.pos & 7)) & 1) << i
            self.pos += 1
        return v


class BitWriter:
    def __init__(self):
        self.out = bytearray()
        self.bits = 0

    def write(self, value: int, bits: int) -> None:
        for i in range(bits):
            if not self.bits & 7:
                self.out.append(0)
            self.out[-1] |= ((value >> i) & 1) << (self.bits & 7)
            self.bits += 1

    def bytes(self) -> bytes:
        return bytes(self.out)


def ilog(v: int) -> int:
    r = 0
    while v:
        r += 1
        v >>= 1
    return r


def quantvals(entries: int, dim: int) -> int:
    vals = int(entries ** (1.0 / dim)) + 1
    while vals ** dim > entries:
        vals -= 1
    return vals


def rebuild_codebook(src: BitReader, w: BitWriter) -> None:
    dim = src.read(4)
    entries = src.read(14)
    w.write(0x564342, 24)
    w.write(dim, 16)
    w.write(entries, 24)
    ordered = src.read(1)
    w.write(ordered, 1)
    if ordered:
        w.write(src.read(5), 5)
        cur = 0
        while cur < entries:
            bits = ilog(entries - cur)
            num = src.read(bits)
            w.write(num, bits)
            cur += num
    else:
        lengthbits = src.read(3)
        sparse = src.read(1)
        w.write(sparse, 1)
        for _ in range(entries):
            if sparse:
                flag = src.read(1)
                w.write(flag, 1)
                if flag:
                    w.write(src.read(lengthbits), 5)
            else:
                w.write(src.read(lengthbits), 5)
    maptype = src.read(1)
    w.write(maptype, 4)
    if maptype:
        w.write(src.read(32), 32)
        w.write(src.read(32), 32)
        qbits = src.read(4)
        w.write(qbits, 4)
        w.write(src.read(1), 1)
        for _ in range(quantvals(entries, dim)):
            w.write(src.read(qbits + 1), qbits + 1)


def rebuild_setup(setup: bytes, channels: int, books: list) -> tuple:
    r = BitReader(setup)
    w = BitWriter()
    for b in b"\x05vorbis":
        w.write(b, 8)
    nbooks = r.read(8) + 1
    w.write(nbooks - 1, 8)
    for _ in range(nbooks):
        rebuild_codebook(BitReader(books[r.read(10)]), w)
    w.write(0, 6)
    w.write(0, 16)
    nfloors = r.read(6) + 1
    w.write(nfloors - 1, 6)
    for _ in range(nfloors):
        w.write(1, 16)
        partitions = r.read(5)
        w.write(partitions, 5)
        classes = []
        for _ in range(partitions):
            c = r.read(4)
            w.write(c, 4)
            classes.append(c)
        dims = {}
        for c in range(max(classes) + 1 if classes else 0):
            d = r.read(3)
            w.write(d, 3)
            dims[c] = d + 1
            subs = r.read(2)
            w.write(subs, 2)
            if subs:
                w.write(r.read(8), 8)
            for _ in range(1 << subs):
                w.write(r.read(8), 8)
        w.write(r.read(2), 2)
        rangebits = r.read(4)
        w.write(rangebits, 4)
        for c in classes:
            for _ in range(dims[c]):
                w.write(r.read(rangebits), rangebits)
    nres = r.read(6) + 1
    w.write(nres - 1, 6)
    for _ in range(nres):
        w.write(r.read(2), 16)
        w.write(r.read(24), 24)
        w.write(r.read(24), 24)
        w.write(r.read(24), 24)
        parts = r.read(6)
        w.write(parts, 6)
        w.write(r.read(8), 8)
        cascades = []
        for _ in range(parts + 1):
            low = r.read(3)
            w.write(low, 3)
            flag = r.read(1)
            w.write(flag, 1)
            high = 0
            if flag:
                high = r.read(5)
                w.write(high, 5)
            cascades.append(low | (high << 3))
        for c in cascades:
            for k in range(8):
                if c & (1 << k):
                    w.write(r.read(8), 8)
    nmaps = r.read(6) + 1
    w.write(nmaps - 1, 6)
    for _ in range(nmaps):
        w.write(0, 16)
        submaps = 1
        flag = r.read(1)
        w.write(flag, 1)
        if flag:
            submaps = r.read(4) + 1
            w.write(submaps - 1, 4)
        flag = r.read(1)
        w.write(flag, 1)
        if flag:
            steps = r.read(8) + 1
            w.write(steps - 1, 8)
            bits = ilog(channels - 1)
            for _ in range(steps):
                w.write(r.read(bits), bits)
                w.write(r.read(bits), bits)
        w.write(r.read(2), 2)
        if submaps > 1:
            for _ in range(channels):
                w.write(r.read(4), 4)
        for _ in range(submaps):
            w.write(r.read(8), 8)
            w.write(r.read(8), 8)
            w.write(r.read(8), 8)
    nmodes = r.read(6) + 1
    w.write(nmodes - 1, 6)
    blockflags = []
    for _ in range(nmodes):
        bf = r.read(1)
        w.write(bf, 1)
        w.write(0, 16)
        w.write(0, 16)
        w.write(r.read(8), 8)
        blockflags.append(bf)
    w.write(1, 1)
    return w.bytes(), blockflags


def rewrite_audio(packets: list, blockflags: list) -> list:
    modebits = ilog(len(blockflags) - 1)
    modes = [BitReader(p).read(modebits) for p in packets]
    out = []
    for i, p in enumerate(packets):
        r = BitReader(p)
        mode = r.read(modebits)
        w = BitWriter()
        w.write(0, 1)
        w.write(mode, modebits)
        if blockflags[mode]:
            prev = blockflags[modes[i - 1]] if i > 0 else 0
            nxt = blockflags[modes[i + 1]] if i + 1 < len(packets) else 0
            w.write(prev, 1)
            w.write(nxt, 1)
        for _ in range(len(p) * 8 - modebits):
            w.write(r.read(1), 1)
        out.append(w.bytes())
    return out, modes


def ogg_crc(data: bytes) -> int:
    crc = 0
    for b in data:
        crc ^= b << 24
        for _ in range(8):
            crc = ((crc << 1) ^ 0x04c11db7) & 0xFFFFFFFF if crc & 0x80000000 else (crc << 1) & 0xFFFFFFFF
    return crc


def ogg_page(packet: bytes, granule: int, seq: int, header_type: int) -> bytes:
    lacing = []
    n = len(packet)
    while n >= 255:
        lacing.append(255)
        n -= 255
    lacing.append(n)
    if len(lacing) > 255:
        raise ValueError("packet too large for one page")
    head = struct.pack("<4sBBqIIIB", b"OggS", 0, header_type, granule, 0x1D1D, seq, 0, len(lacing)) + bytes(lacing)
    page = bytearray(head + packet)
    struct.pack_into("<I", page, 22, ogg_crc(bytes(page)))
    return bytes(page)


def find_media(container: Path, media_id: int) -> bytes:
    for bank_data in (dump.Package(container).bank_bytes(e) for e in dump.Package(container).banks) if container.suffix.lower() != ".bnk" else [container.read_bytes()]:
        pos, chunks = 0, {}
        while pos + 8 <= len(bank_data):
            tag = bank_data[pos:pos + 4]
            size = struct.unpack_from("<I", bank_data, pos + 4)[0]
            chunks[tag] = (pos + 8, size)
            pos += 8 + size
        if b"DIDX" not in chunks:
            continue
        didx, data = chunks[b"DIDX"], chunks[b"DATA"]
        for i in range(didx[0], didx[0] + didx[1], 12):
            mid, off, size = struct.unpack_from("<III", bank_data, i)
            if mid == media_id:
                return bank_data[data[0] + off:data[0] + off + size]
    raise SystemExit(f"media 0x{media_id:08x} not in {container}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--container", required=True)
    ap.add_argument("--media", required=True, type=lambda s: int(s, 0))
    ap.add_argument("--ogg")
    ap.add_argument("--pcm")
    ap.add_argument("--blocks", type=int, default=1024)
    ap.add_argument("--exe", default=str(EXE))
    args = ap.parse_args()
    container = Path(args.container)
    if not container.is_absolute():
        container = dump.COOKED / container
    media = find_media(container, args.media)
    if media[:4] != b"RIFF" or media[8:12] != b"WAVE":
        raise SystemExit("not a RIFF WAVE media item")
    pos, chunks = 12, {}
    while pos + 8 <= len(media):
        tag = media[pos:pos + 4]
        size = struct.unpack_from("<I", media, pos + 4)[0]
        chunks[tag] = media[pos + 8:pos + 8 + size]
        pos += 8 + size + (size & 1)
    fmt, data = chunks[b"fmt "], chunks[b"data"]
    tag, channels, rate = struct.unpack_from("<HHI", fmt, 0)
    mask = struct.unpack_from("<I", fmt, 20)[0]
    (total, loop_start, loop_end, loop_begin_extra, loop_end_extra, seek_size, data_offset, max_packet, last_extra,
     alloc, alloc64, hash_cb, bs0, bs1) = struct.unpack_from("<IIIHHIIHHIIIBB", fmt, 24)
    print(f"media 0x{args.media:08x}: {len(media)} bytes, tag 0x{tag:04x}, {channels} ch (mask 0x{mask:x}), {rate} Hz, "
          f"dwTotalPCMFrames {total}, seek table {seek_size}, data offset {data_offset}, max packet {max_packet}, "
          f"uLastGranuleExtra {last_extra}, alloc {alloc}, hash 0x{hash_cb:08x}, blocks {1 << bs0}/{1 << bs1}")
    setup_size = struct.unpack_from("<H", data, seek_size)[0]
    setup = data[seek_size + 2:seek_size + 2 + setup_size]
    packets = []
    p = data_offset
    while p + 2 <= len(data):
        size = struct.unpack_from("<H", data, p)[0]
        packets.append(data[p + 2:p + 2 + size])
        p += 2 + size
    setup_std, blockflags = rebuild_setup(setup, channels, packed_codebooks(Path(args.exe)))
    audio, modes = rewrite_audio(packets, blockflags)
    sizes = [1 << (bs1 if blockflags[m] else bs0) for m in modes]
    produced = sum(sizes[i - 1] // 4 + sizes[i] // 4 for i in range(1, len(sizes)))
    print(f"{len(packets)} audio packets, {len(blockflags)} modes; standard frame count {produced}, "
          f"Wwise decoder frame count {produced - last_extra} (minus uLastGranuleExtra)")

    ident = struct.pack("<B6sIBIiiiBB", 1, b"vorbis", 0, channels, rate, 0, struct.unpack_from("<I", fmt, 8)[0] * 8, 0,
                        bs0 | (bs1 << 4), 1)
    comment = b"\x03vorbis" + struct.pack("<I", 4) + b"IH2 " + struct.pack("<I", 0) + b"\x01"
    pages = [ogg_page(ident, 0, 0, 2), ogg_page(comment, 0, 1, 0), ogg_page(setup_std, 0, 2, 0)]
    granule = 0
    for i, pkt in enumerate(audio):
        if i:
            granule += sizes[i - 1] // 4 + sizes[i] // 4
        last = i + 1 == len(audio)
        pages.append(ogg_page(pkt, min(granule, total) if last else granule, 3 + i, 4 if last else 0))
    ogg = b"".join(pages)
    ogg_path = Path(args.ogg) if args.ogg else Path.cwd() / f"wwise_ref_{args.media:08x}.ogg"
    ogg_path.write_bytes(ogg)
    res = subprocess.run([str(FFMPEG), "-v", "error", "-i", str(ogg_path), "-f", "s16le", "-acodec", "pcm_s16le", "-"],
                         capture_output=True)
    if res.returncode:
        print(res.stderr.decode(errors="replace"))
        raise SystemExit("ffmpeg failed")
    pcm = res.stdout
    if args.pcm:
        Path(args.pcm).write_bytes(pcm)
    samples = struct.unpack(f"<{len(pcm) // 2}h", pcm)
    frames = len(samples) // channels
    print(f"ffmpeg: {frames} frames, sum {sum(samples)}, crc32 0x{zlib.crc32(pcm):08x}, "
          f"peak {max(abs(s) for s in samples) if samples else 0}")
    rms = []
    step = args.blocks * channels
    for i in range(0, len(samples), step):
        blk = samples[i:i + step]
        rms.append(round((sum(s * s for s in blk) / len(blk)) ** 0.5))
    print(f"rms per {args.blocks} frames ({len(rms)} blocks): {rms}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
