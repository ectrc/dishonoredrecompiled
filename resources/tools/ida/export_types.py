"""P1.6: export local types as a C header, a JSON layout dump (UDTs and enums with values) and a size table.

Outputs docs/types/all_types.h, docs/types/types.json, docs/types/sizes.csv.
Usage: python resources/tools/ida/run.py resources/tools/ida/export_types.py <db.i64> [suffix]
"""
import json
import sys

import ida_typeinf

from _common import TYPES_DIR, log, open_csv


class FileSink(ida_typeinf.text_sink_t):
    def __init__(self, path):
        super().__init__()
        self.f = path.open("w", encoding="utf-8")

    def _print(self, s):
        self.f.write(s)
        return 0


def udt_record(name: str, t: ida_typeinf.tinfo_t) -> dict:
    udt = ida_typeinf.udt_type_data_t()
    t.get_udt_details(udt)
    members = []
    for m in udt:
        members.append({
            "name": m.name,
            "offset": m.offset // 8,
            "bit_offset": m.offset % 8 if m.is_bitfield() else None,
            "size": m.size // 8 if not m.is_bitfield() else None,
            "bits": m.size if m.is_bitfield() else None,
            "type": str(m.type),
            "is_base": m.is_baseclass(),
            "is_vftable": m.is_vftable(),
        })
    return {
        "name": name,
        "kind": "union" if udt.is_union else "struct",
        "size": t.get_size(),
        "align": udt.effalign,
        "bases": [m["type"] for m in members if m["is_base"]],
        "has_vftable": any(m["is_vftable"] for m in members),
        "members": members,
    }


def enum_record(name: str, t: ida_typeinf.tinfo_t) -> dict:
    etd = ida_typeinf.enum_type_data_t()
    t.get_enum_details(etd)
    size = t.get_size()
    wrap = 1 << (8 * size) if 0 < size <= 8 else 0
    members = []
    for m in etd:
        value = m.value
        if wrap and value >= wrap // 2:
            value -= wrap
        members.append({"name": m.name, "value": value})
    return {
        "name": name,
        "size": size,
        "is_bitmask": etd.is_bf(),
        "members": members,
    }


def main() -> None:
    suffix = sys.argv[1] if len(sys.argv) > 1 else ""
    TYPES_DIR.mkdir(parents=True, exist_ok=True)
    til = ida_typeinf.get_idati()
    limit = ida_typeinf.get_ordinal_limit(til)

    records = []
    enums = []
    for ordinal in range(1, limit):
        t = ida_typeinf.tinfo_t()
        if not t.get_numbered_type(til, ordinal):
            continue
        name = ida_typeinf.get_numbered_type_name(til, ordinal) or f"#{ordinal}"
        if t.is_udt():
            records.append(udt_record(name, t))
        elif t.is_enum():
            enums.append(enum_record(name, t))

    with (TYPES_DIR / f"types{suffix}.json").open("w", encoding="utf-8") as f:
        json.dump({"udt_count": len(records), "enum_count": len(enums), "types": records, "enums": enums}, f, indent=1)

    f, w = open_csv(TYPES_DIR / f"sizes{suffix}.csv", ["name", "size", "align", "kind"])
    for r in records:
        w.writerow([r["name"], r["size"], r["align"], r["kind"]])
    f.close()

    sink = FileSink(TYPES_DIR / f"all_types{suffix}.h")
    sink.f.write(f"// Exported from IDA local types: {len(records)} UDTs, {len(enums)} enums, {limit - 1} ordinals total\n")
    flags = ida_typeinf.PDF_INCL_DEPS | ida_typeinf.PDF_DEF_FWD | ida_typeinf.PDF_DEF_BASE | ida_typeinf.PDF_HEADER_CMT
    printed = ida_typeinf.print_decls(sink, til, [], flags)
    sink.f.close()
    log(f"udts={len(records)} enums={len(enums)} printed_decls={printed}")


if __name__ == "__main__":
    main()
