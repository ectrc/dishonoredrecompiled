"""Shared helpers for the IDA export scripts.

Every script runs three ways:
  * headless:   python resources/tools/ida/run.py <script.py> <database.i64> [script args]
  * IDA GUI:    File -> Script file (the database is already open)
  * IDA MCP:    idb_open the database, then execute the script body

Under headless mode run.py imports idapro before anything else and opens the database;
the scripts themselves only use the regular ida_* modules.
"""
import csv
import sys
from pathlib import Path

import ida_bytes
import ida_funcs
import ida_ida
import ida_name
import ida_nalt
import ida_segment
import idautils

REPO = Path(__file__).resolve().parents[3]
SYMBOLS_DIR = REPO / "resources" / "docs" / "symbols"
TYPES_DIR = REPO / "resources" / "docs" / "types"


def image_base() -> int:
    return ida_nalt.get_imagebase()


def rva(ea: int) -> int:
    return ea - image_base()


def demangle(name: str) -> str:
    out = ida_name.demangle_name(name, ida_name.MNG_NODEFINIT | ida_name.MNG_NOPTRTYP | ida_name.MNG_NOECSU | ida_name.MNG_NOTYPE)
    return out or name


def demangle_full(name: str) -> str:
    out = ida_name.demangle_name(name, ida_name.MNG_LONG_FORM)
    return out or name


def iter_functions():
    for ea in idautils.Functions():
        yield ida_funcs.get_func(ea)


def function_name(func) -> str:
    return ida_funcs.get_func_name(func.start_ea) or ""


def segment_name(ea: int) -> str:
    seg = ida_segment.getseg(ea)
    return ida_segment.get_segm_name(seg) if seg else ""


def read_ptr(ea: int) -> int:
    return ida_bytes.get_dword(ea) if not ida_ida.inf_is_64bit() else ida_bytes.get_qword(ea)


def ptr_size() -> int:
    return 8 if ida_ida.inf_is_64bit() else 4


def is_code(ea: int) -> bool:
    return ida_funcs.get_func(ea) is not None


def open_csv(path: Path, header: list[str]):
    path.parent.mkdir(parents=True, exist_ok=True)
    f = path.open("w", newline="", encoding="utf-8")
    writer = csv.writer(f)
    writer.writerow(header)
    return f, writer


def log(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)
