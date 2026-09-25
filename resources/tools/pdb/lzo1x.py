"""Pure-Python LZO1X decompressor (the format written by lzo1x_1_compress / lzopro, read by
lzo1x_decompress_safe). Dishonored's cooked packages are COMPRESS_LZO (CompressionFlags=2), so
this is what `appUncompressMemoryLZO` sees for every FCompressedChunk block.

Format (lzo1x_d.ch):
  first byte > 17           -> (byte-17) leading literals (if < 4 the "match_next" path)
  t < 16 at run start       -> literal run of t+3 bytes (t == 0: 255-run length extension +15)
  after a literal run, t<16 -> M1 match, 3 bytes, distance 0x801 + (t>>2) + (next<<2)
  t >= 64  (M2)             -> len (t>>5)+1 bytes, distance 1 + ((t>>2)&7) + (next<<3)
  32<=t<64 (M3)             -> len (t&31)+2 (0: extension +31), distance 1 + (le16>>2)
  16<=t<32 (M4)             -> len (t&7)+2 (0: extension +7), distance 0x4000 + ((t&8)<<11) + (le16>>2);
                               distance == 0x4000 + 0 (le16 == 0, t&8 == 0) is the end marker (0x11 00 00)
  t < 16 in match state     -> M1 match, 2 bytes, distance 1 + (t>>2) + (next<<2)
  after every match, the low 2 bits of the last offset byte give 0..3 trailing literals; 0 means a
  new literal run follows, otherwise the next byte is a match code again.

Usage: python lzo1x.py <package.upk>   decompresses the first FCompressedChunk block and prints
the leading name-table entries as a self-check.
"""
import struct
import sys


class LZOError(Exception):
    pass


def decompress(src: bytes, dst_len: int | None = None) -> bytes:
    out = bytearray()
    ip = 0
    n = len(src)
    try:
        t = src[ip]
        if t > 17:
            ip += 1
            t -= 17
            if t < 4:
                state = _match_next(src, ip, t, out)
                ip, t = state
                ip, done = _match_loop(src, ip, t, out)
                if done:
                    return _finish(out, dst_len)
            else:
                out += src[ip : ip + t]
                ip += t
                t = src[ip]
                ip += 1
                if t >= 16:
                    ip, done = _match_loop(src, ip, t, out)
                    if done:
                        return _finish(out, dst_len)
                else:
                    ip = _first_literal_m1(src, ip, t, out)
                    t = src[ip - 2] & 3
                    if t != 0:
                        ip, t = _match_next(src, ip, t, out)
                        ip, done = _match_loop(src, ip, t, out)
                        if done:
                            return _finish(out, dst_len)
        while True:
            t = src[ip]
            ip += 1
            if t >= 16:
                ip, done = _match_loop(src, ip, t, out)
                if done:
                    return _finish(out, dst_len)
                continue
            if t == 0:
                while src[ip] == 0:
                    t += 255
                    ip += 1
                t += 15 + src[ip]
                ip += 1
            t += 3
            out += src[ip : ip + t]
            ip += t
            t = src[ip]
            ip += 1
            if t >= 16:
                ip, done = _match_loop(src, ip, t, out)
                if done:
                    return _finish(out, dst_len)
                continue
            ip = _first_literal_m1(src, ip, t, out)
            t = src[ip - 2] & 3
            if t != 0:
                ip, t = _match_next(src, ip, t, out)
                ip, done = _match_loop(src, ip, t, out)
                if done:
                    return _finish(out, dst_len)
    except IndexError:
        raise LZOError(f"input overrun at {ip}/{n}, {len(out)} bytes produced")


def _finish(out: bytearray, dst_len: int | None) -> bytes:
    if dst_len is not None and len(out) != dst_len:
        raise LZOError(f"decompressed {len(out)} bytes, expected {dst_len}")
    return bytes(out)


def _copy_match(out: bytearray, dist: int, length: int):
    start = len(out) - dist
    if start < 0:
        raise LZOError(f"lookbehind overrun: distance {dist} at output {len(out)}")
    if dist >= length:
        out += out[start : start + length]
    else:
        seg = out[start:]
        reps, rem = divmod(length, dist)
        out += seg * reps
        if rem:
            out += seg[:rem]


def _first_literal_m1(src: bytes, ip: int, t: int, out: bytearray) -> int:
    dist = 0x801 + (t >> 2) + (src[ip] << 2)
    ip += 1
    _copy_match(out, dist, 3)
    return ip


def _match_next(src: bytes, ip: int, t: int, out: bytearray):
    out += src[ip : ip + t]
    ip += t
    t = src[ip]
    ip += 1
    return ip, t


def _match_loop(src: bytes, ip: int, t: int, out: bytearray):
    while True:
        if t >= 64:
            dist = 1 + ((t >> 2) & 7) + (src[ip] << 3)
            ip += 1
            _copy_match(out, dist, (t >> 5) + 1)
        elif t >= 32:
            t &= 31
            if t == 0:
                while src[ip] == 0:
                    t += 255
                    ip += 1
                t += 31 + src[ip]
                ip += 1
            dist = 1 + ((src[ip] | (src[ip + 1] << 8)) >> 2)
            ip += 2
            _copy_match(out, dist, t + 2)
        elif t >= 16:
            dist = (t & 8) << 11
            t &= 7
            if t == 0:
                while src[ip] == 0:
                    t += 255
                    ip += 1
                t += 7 + src[ip]
                ip += 1
            dist += (src[ip] | (src[ip + 1] << 8)) >> 2
            ip += 2
            if dist == 0:
                return ip, True
            _copy_match(out, dist + 0x4000, t + 2)
        else:
            dist = 1 + (t >> 2) + (src[ip] << 2)
            ip += 1
            _copy_match(out, dist, 2)
        t = src[ip - 2] & 3
        if t == 0:
            return ip, False
        out += src[ip : ip + t]
        ip += t
        t = src[ip]
        ip += 1


PACKAGE_FILE_TAG = 0x9E2A83C1


def decompress_chunk_block(data: bytes, offset: int) -> bytes:
    """Decompress one FCompressedChunk as written by FArchive::SerializeCompressed: tag info
    {PACKAGE_FILE_TAG, chunk size}, summary {compressed, uncompressed}, per-block table, blocks."""
    tag, block_size = struct.unpack_from("<II", data, offset)
    if tag != PACKAGE_FILE_TAG:
        raise LZOError(f"bad chunk tag 0x{tag:08x} at {offset}")
    if block_size == PACKAGE_FILE_TAG:
        block_size = 0x20000
    total_c, total_u = struct.unpack_from("<ii", data, offset + 8)
    count = (total_u + block_size - 1) // block_size
    table = [struct.unpack_from("<ii", data, offset + 16 + 8 * i) for i in range(count)]
    pos = offset + 16 + 8 * count
    out = bytearray()
    for c, u in table:
        out += decompress(data[pos : pos + c], u)
        pos += c
    if len(out) != total_u:
        raise LZOError(f"chunk at {offset}: got {len(out)} bytes, expected {total_u}")
    return bytes(out)


def _self_check(path: str) -> int:
    from pathlib import Path

    sys.path.insert(0, str(Path(__file__).parent))
    from read_package_classes import read_summary_and_chunks

    data = Path(path).read_bytes()
    summary, chunks = read_summary_and_chunks(Path(path))
    u_off, u_size, c_off, c_size = chunks[0]
    blob = decompress_chunk_block(data, c_off)
    print(f"{path}: chunk 0 uncompressed {u_off}+{u_size}, got {len(blob)} bytes")
    o = summary["name_offset"] - u_off
    for i in range(min(8, summary["name_count"])):
        ln = struct.unpack_from("<i", blob, o)[0]
        o += 4
        if ln < 0:
            s = blob[o : o - 2 * ln].decode("utf-16le")
            o += -2 * ln
        else:
            s = blob[o : o + ln].decode("latin-1")
            o += ln
        flags = struct.unpack_from("<Q", blob, o)[0]
        o += 8
        print(f"  name[{i}] = {s.rstrip(chr(0))!r} flags=0x{flags:016x}")
    return 0


if __name__ == "__main__":
    sys.exit(_self_check(sys.argv[1]))
