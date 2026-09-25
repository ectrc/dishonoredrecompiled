"""Middleware version evidence from PE binaries: printable strings (ASCII + UTF-16LE) matched
against per-library patterns, plus the VS_VERSIONINFO resource parsed by hand (no pefile).

    python resources/tools/binaries/scan_versions.py <file-or-dir> [more...] [--grep REGEX] [--all]

Without --grep the built-in library patterns are used. --all prints every match (default caps
each pattern at 40 hits per file). Output goes to stdout as markdown-ish text.
"""
import os
import re
import struct
import sys

PATTERNS = {
    "wwise": [rb"Wwise\s*v?\d+\.\d+[.\d]*\s*(?:Build\s*\d+)?", rb"Wwise\(R\)[^\x00]{0,60}", rb"AK::[^\x00]{0,60}", rb"Build\s+\d{4}", rb"AkSoundEngine[^\x00]{0,60}", rb"AK_WWISESDK_VERSION[^\x00]{0,40}", rb"Audiokinetic[^\x00]{0,60}"],
    "scaleform": [rb"GFx\s*\d+\.\d+[.\d]*[^\x00]{0,40}", rb"Scaleform[^\x00]{0,80}", rb"GFX_VERSION[^\x00]{0,40}", rb"GFC_[A-Z_]{0,40}VERSION[^\x00]{0,30}", rb"gfx\s*(?:version|release)[^\x00]{0,40}", rb"GFxPlayer[^\x00]{0,40}", rb"Scaleform GFx [^\x00]{0,60}"],
    "facefx": [rb"FaceFX[^\x00]{0,80}", rb"OC3 Entertainment[^\x00]{0,60}", rb"FxSDK[^\x00]{0,60}", rb"fxsdk[^\x00]{0,60}"],
    "physx": [rb"PhysX[^\x00]{0,80}", rb"NX_SDK_VERSION[^\x00]{0,40}", rb"NxPhysicsSDK[^\x00]{0,40}", rb"NVIDIA PhysX[^\x00]{0,60}", rb"PhysX_?[0-9][^\x00]{0,40}", rb"Physics SDK[^\x00]{0,60}", rb"physxloader[^\x00]{0,40}", rb"2\.8\.[0-9][^\x00]{0,20}"],
    "apex": [rb"APEX[^\x00]{0,80}", rb"NxApex[^\x00]{0,60}", rb"ApexSDK[^\x00]{0,60}"],
    "bink": [rb"Bink[^\x00]{0,80}", rb"RAD Game Tools[^\x00]{0,80}", rb"binkw32[^\x00]{0,40}", rb"Miles[^\x00]{0,40}"],
    "steam": [rb"Steam[A-Za-z]+\d{3}", rb"STEAM[A-Z_]*_INTERFACE_VERSION[^\x00]{0,20}", rb"steam_api[^\x00]{0,40}", rb"SteamClient\d{3}", rb"Steamworks[^\x00]{0,60}"],
    "curl": [rb"libcurl/[0-9.]+[^\x00]{0,60}", rb"curl[- ][0-9.]+[^\x00]{0,20}", rb"OpenSSL[/ ][0-9.]+[a-z]?[^\x00]{0,20}", rb"zlib/[0-9.]+"],
    "build": [rb"UnrealEngine3[^\x00]{0,60}", rb"Dishonored_[A-Za-z]+", rb"Changelist[^\x00]{0,40}", rb"Build:? ?\d{4,6}", rb"engineVersion[^\x00]{0,20}"],
    "misc": [rb"lzopro[^\x00]{0,40}", rb"LZO [^\x00]{0,40}", rb"libpng[^\x00]{0,40}", rb"zlib[^\x00]{0,40}", rb"1\.2\.[0-9]+[^\x00]{0,10}", rb"nvapi[^\x00]{0,30}", rb"Xiph[^\x00]{0,40}", rb"libvorbis[^\x00]{0,40}", rb"cudart[^\x00]{0,40}", rb"CUDA[^\x00]{0,40}"],
}


def ascii_strings(data, min_len=6):
    for m in re.finditer(rb"[\x20-\x7e]{%d,}" % min_len, data):
        yield m.start(), m.group().decode("ascii"), "a"


def utf16_strings(data, min_len=6):
    for m in re.finditer(rb"(?:[\x20-\x7e]\x00){%d,}" % min_len, data):
        yield m.start(), m.group().decode("utf-16le"), "u"


def grep_file(path, patterns, cap):
    data = open(path, "rb").read()
    out = {}
    for name, pats in patterns.items():
        hits = []
        for p in pats:
            rx = re.compile(p, re.I)
            seen = set()
            for m in rx.finditer(data):
                s = m.group()
                if s in seen:
                    continue
                seen.add(s)
                hits.append((m.start(), s.decode("latin-1"), "a"))
                if len(hits) >= cap and cap:
                    break
        text_rx = [re.compile(p.decode("latin-1"), re.I) for p in pats]
        for off, s, kind in utf16_strings(data):
            for rx in text_rx:
                m = rx.search(s)
                if m:
                    hits.append((off, m.group(), "u"))
                    break
            if cap and len(hits) >= 2 * cap:
                break
        if hits:
            out[name] = hits
    return out


def version_info(path):
    """Parse VS_VERSIONINFO from the .rsrc section without pefile."""
    data = open(path, "rb").read()
    try:
        pe_off = struct.unpack_from("<I", data, 0x3C)[0]
        if data[pe_off:pe_off + 4] != b"PE\0\0":
            return None
        nsec = struct.unpack_from("<H", data, pe_off + 6)[0]
        opt_size = struct.unpack_from("<H", data, pe_off + 20)[0]
        opt = pe_off + 24
        magic = struct.unpack_from("<H", data, opt)[0]
        dd_off = opt + (96 if magic == 0x10B else 112)
        rsrc_rva, rsrc_size = struct.unpack_from("<II", data, dd_off + 2 * 8)
        sec = pe_off + 24 + opt_size
        sections = []
        for i in range(nsec):
            name = data[sec + i * 40: sec + i * 40 + 8].rstrip(b"\0")
            vsz, vaddr, rsz, rptr = struct.unpack_from("<IIII", data, sec + i * 40 + 8)
            sections.append((name, vaddr, vsz, rptr, rsz))

        def rva2off(rva):
            for name, vaddr, vsz, rptr, rsz in sections:
                if vaddr <= rva < vaddr + max(vsz, rsz):
                    return rva - vaddr + rptr
            return None

        if not rsrc_rva:
            return None
        base = rva2off(rsrc_rva)
        if base is None:
            return None

        def walk(dir_off, depth, want_type):
            n_named, n_id = struct.unpack_from("<HH", data, dir_off + 12)
            res = []
            for i in range(n_named + n_id):
                eid, eoff = struct.unpack_from("<II", data, dir_off + 16 + i * 8)
                if depth == 0 and eid != want_type:
                    continue
                if eoff & 0x80000000:
                    res += walk(base + (eoff & 0x7FFFFFFF), depth + 1, want_type)
                else:
                    drva, dsize = struct.unpack_from("<II", data, base + eoff)
                    off = rva2off(drva)
                    if off is not None:
                        res.append(data[off:off + dsize])
            return res

        blobs = walk(base, 0, 16)
    except Exception as e:
        return {"error": str(e)}
    if not blobs:
        return None
    blob = blobs[0]
    result = {}
    fi = blob.find("VS_VERSION_INFO".encode("utf-16le"))
    if fi >= 0:
        p = fi + len("VS_VERSION_INFO".encode("utf-16le")) + 2
        p = (p + 3) & ~3
        sig, strucver, fvms, fvls, pvms, pvls = struct.unpack_from("<IIIIII", blob, p)
        if sig == 0xFEEF04BD:
            result["FileVersion"] = "%d.%d.%d.%d" % (fvms >> 16, fvms & 0xFFFF, fvls >> 16, fvls & 0xFFFF)
            result["ProductVersion"] = "%d.%d.%d.%d" % (pvms >> 16, pvms & 0xFFFF, pvls >> 16, pvls & 0xFFFF)
    for key in ("CompanyName", "FileDescription", "FileVersion", "ProductVersion", "ProductName", "InternalName", "LegalCopyright", "OriginalFilename", "Comments", "PrivateBuild", "SpecialBuild"):
        k = key.encode("utf-16le")
        i = blob.find(k)
        if i < 0:
            continue
        p = i + len(k) + 2
        p = (p + 3) & ~3
        end = blob.find(b"\0\0", p)
        while end % 2:
            end = blob.find(b"\0\0", end + 1)
        val = blob[p:end].decode("utf-16le", "replace")
        if val:
            result.setdefault("s:" + key, val)
    return result


def main(argv):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    grep = None
    cap = 40
    paths = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--grep":
            grep = argv[i + 1]
            i += 2
            continue
        if a == "--all":
            cap = 0
            i += 1
            continue
        paths.append(a)
        i += 1
    files = []
    for p in paths:
        if os.path.isdir(p):
            for f in sorted(os.listdir(p)):
                if f.lower().endswith((".dll", ".exe")):
                    files.append(os.path.join(p, f))
        else:
            files.append(p)
    patterns = {"grep": [grep.encode("latin-1")]} if grep else PATTERNS
    for f in files:
        print("=" * 100)
        print(f, os.path.getsize(f))
        vi = version_info(f)
        if vi:
            print("  [versioninfo]")
            for k, v in vi.items():
                print("    %s = %s" % (k, v))
        hits = grep_file(f, patterns, cap)
        for name, lst in hits.items():
            print("  [%s] %d hits" % (name, len(lst)))
            for off, s, kind in lst:
                print("    0x%08x %s %s" % (off, kind, s[:160]))


if __name__ == "__main__":
    main(sys.argv[1:])
