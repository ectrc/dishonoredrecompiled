"""P1.3: dump functions, line records, compilands and source files from a PDB via the DIA SDK.

Usage: python tools/pdb/dia_dump.py <file.pdb> [--out docs/symbols] [--suffix _2013]

Outputs (in --out):
  pdb_functions<suffix>.csv   rva,length,name,undecorated
  lines<suffix>.csv           function_rva,rva,length,file,line
  compilands<suffix>.csv      name,library
  sourcefiles<suffix>.txt     distinct source file paths (lower-cased)

msdia140.dll is loaded register-free from the VS 2022 DIA SDK (amd64 build for 64-bit Python).
"""
import argparse
import csv
import ctypes
import sys
from pathlib import Path

import comtypes
import comtypes.client
from comtypes import GUID

MSDIA_CANDIDATES = [
    Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll"),
    Path(r"C:\Program Files\Microsoft Visual Studio\18\Community\DIA SDK\bin\amd64\msdia140.dll"),
]
CLSID_DIA_SOURCE = GUID("{E6756135-1E65-4D17-8576-610761398C3C}")
SYM_TAG_COMPILAND = 2
SYM_TAG_FUNCTION = 5
NS_NONE = 0


def load_dia():
    for path in MSDIA_CANDIDATES:
        if path.exists():
            break
    else:
        raise FileNotFoundError("msdia140.dll (amd64) not found")
    comtypes.client.GetModule(str(path))
    from comtypes.gen import Dia2Lib

    lib = ctypes.WinDLL(str(path))
    factory = ctypes.POINTER(comtypes.IClassFactory)()
    lib.DllGetClassObject(ctypes.byref(CLSID_DIA_SOURCE), ctypes.byref(comtypes.IClassFactory._iid_), ctypes.byref(factory))
    source = factory.CreateInstance(None, Dia2Lib.IDiaDataSource)
    return source, Dia2Lib


def iterate(enum):
    count = enum.Count
    for i in range(count):
        yield enum.Item(i)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("pdb")
    ap.add_argument("--out", default=str(Path(__file__).resolve().parents[2] / "docs" / "symbols"))
    ap.add_argument("--suffix", default="")
    args = ap.parse_args(argv[1:])
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    source, dia = load_dia()
    source.loadDataFromPdb(str(Path(args.pdb).resolve()))
    session = source.openSession()
    scope = session.globalScope
    print(f"pdb loaded: {args.pdb}", file=sys.stderr)

    with (out / f"compilands{args.suffix}.csv").open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["name", "library"])
        n = 0
        for comp in iterate(scope.findChildren(SYM_TAG_COMPILAND, None, NS_NONE)):
            w.writerow([comp.name or "", comp.libraryName or ""])
            n += 1
        print(f"compilands={n}", file=sys.stderr)

    files = set()
    n_func = 0
    n_lines = 0
    with (out / f"pdb_functions{args.suffix}.csv").open("w", newline="", encoding="utf-8") as ff, \
         (out / f"lines{args.suffix}.csv").open("w", newline="", encoding="utf-8") as fl:
        wf = csv.writer(ff)
        wl = csv.writer(fl)
        wf.writerow(["rva", "length", "name", "undecorated"])
        wl.writerow(["function_rva", "rva", "length", "file", "line"])
        for func in iterate(scope.findChildren(SYM_TAG_FUNCTION, None, NS_NONE)):
            frva = func.relativeVirtualAddress
            length = func.length
            wf.writerow([f"0x{frva:x}", length, func.name or "", func.undecoratedName or ""])
            n_func += 1
            if length == 0:
                continue
            try:
                lines = session.findLinesByRVA(frva, length)
            except comtypes.COMError:
                continue
            for ln in iterate(lines):
                fname = (ln.sourceFile.fileName or "").lower()
                files.add(fname)
                wl.writerow([f"0x{frva:x}", f"0x{ln.relativeVirtualAddress:x}", ln.length, fname, ln.lineNumber])
                n_lines += 1
            if n_func % 5000 == 0:
                print(f"  functions={n_func} lines={n_lines}", file=sys.stderr)
    (out / f"sourcefiles{args.suffix}.txt").write_text("\n".join(sorted(files)) + "\n", encoding="utf-8")
    print(f"functions={n_func} lines={n_lines} sourcefiles={len(files)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
