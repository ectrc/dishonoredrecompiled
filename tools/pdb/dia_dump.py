"""P1.3: dump functions, line records, compilands and source files from a PDB via the DIA SDK.

Usage: python tools/pdb/dia_dump.py <file.pdb> [--functions docs/symbols/functions.csv] [--out docs/symbols] [--suffix _2013]

Outputs (in --out):
  pdb_functions<suffix>.csv   rva,length,name,undecorated,compiland      (DIA SymTagFunction symbols)
  lines<suffix>.csv           function_rva,rva,length,file,line          (per IDA function when --functions is given,
                                                                           else per DIA function)
  compiland_of<suffix>.csv    function_rva,compiland,library              (nearest symbol's compiland per IDA function)
  compilands<suffix>.csv      name,library
  sourcefiles<suffix>.txt     distinct source file paths (lower-cased)

Querying per IDA function (with --functions) catches inline/COMDAT functions that have line info but no
SymTagFunction symbol of their own. msdia140.dll is loaded register-free from the VS 2022 DIA SDK.
"""
import argparse
import csv
import ctypes
import sys
from pathlib import Path

import comtypes
import comtypes.client
from comtypes import GUID
from comtypes.server import IClassFactory

MSDIA_CANDIDATES = [
    Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\DIA SDK\bin\amd64\msdia140.dll"),
    Path(r"C:\Program Files\Microsoft Visual Studio\18\Community\DIA SDK\bin\amd64\msdia140.dll"),
]
CLSID_DIA_SOURCE = GUID("{E6756135-1E65-4D17-8576-610761398C3C}")
SYM_TAG_NULL = 0
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
    factory = ctypes.POINTER(IClassFactory)()
    lib.DllGetClassObject(ctypes.byref(CLSID_DIA_SOURCE), ctypes.byref(IClassFactory._iid_), ctypes.byref(factory))
    source = factory.CreateInstance(None, Dia2Lib.IDiaDataSource)
    return source


def iterate(enum):
    for i in range(enum.Count):
        yield enum.Item(i)


def compiland_of_symbol(sym):
    seen = 0
    try:
        while sym and seen < 8:
            if sym.symTag == SYM_TAG_COMPILAND:
                return sym
            sym = sym.lexicalParent
            seen += 1
    except (comtypes.COMError, ValueError):
        return None
    return None


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("pdb")
    ap.add_argument("--functions", default=None, help="IDA functions.csv; when given, lines/compilands are resolved per IDA function")
    ap.add_argument("--out", default=str(Path(__file__).resolve().parents[2] / "docs" / "symbols"))
    ap.add_argument("--suffix", default="")
    args = ap.parse_args(argv[1:])
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    source = load_dia()
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

    dia_functions = []
    with (out / f"pdb_functions{args.suffix}.csv").open("w", newline="", encoding="utf-8") as ff:
        wf = csv.writer(ff)
        wf.writerow(["rva", "length", "name", "undecorated", "compiland"])
        for func in iterate(scope.findChildren(SYM_TAG_FUNCTION, None, NS_NONE)):
            comp = compiland_of_symbol(func)
            wf.writerow([f"0x{func.relativeVirtualAddress:x}", func.length, func.name or "", func.undecoratedName or "", comp.name if comp else ""])
            dia_functions.append((func.relativeVirtualAddress, func.length))
    print(f"dia functions={len(dia_functions)}", file=sys.stderr)

    if args.functions:
        targets = []
        with open(args.functions, newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                targets.append((int(row["rva"], 16), int(row["size"])))
    else:
        targets = dia_functions

    files = set()
    n_lines = 0
    n_comp = 0
    with (out / f"lines{args.suffix}.csv").open("w", newline="", encoding="utf-8") as fl, \
         (out / f"compiland_of{args.suffix}.csv").open("w", newline="", encoding="utf-8") as fc:
        wl = csv.writer(fl)
        wc = csv.writer(fc)
        wl.writerow(["function_rva", "rva", "length", "file", "line"])
        wc.writerow(["function_rva", "compiland", "library"])
        for i, (frva, length) in enumerate(targets, 1):
            if length > 0:
                try:
                    for ln in iterate(session.findLinesByRVA(frva, length)):
                        fname = (ln.sourceFile.fileName or "").lower()
                        files.add(fname)
                        wl.writerow([f"0x{frva:x}", f"0x{ln.relativeVirtualAddress:x}", ln.length, fname, ln.lineNumber])
                        n_lines += 1
                except comtypes.COMError:
                    pass
            try:
                sym = session.findSymbolByRVA(frva, SYM_TAG_NULL)
            except (comtypes.COMError, ValueError):
                sym = None
            comp = compiland_of_symbol(sym) if sym else None
            if comp:
                try:
                    wc.writerow([f"0x{frva:x}", comp.name or "", comp.libraryName or ""])
                    n_comp += 1
                except (comtypes.COMError, ValueError):
                    pass
            if i % 10000 == 0:
                print(f"  functions={i} lines={n_lines} compilands={n_comp}", file=sys.stderr)
    (out / f"sourcefiles{args.suffix}.txt").write_text("\n".join(sorted(files)) + "\n", encoding="utf-8")
    print(f"targets={len(targets)} lines={n_lines} compiland_hits={n_comp} sourcefiles={len(files)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
