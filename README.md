# Dishonored Recompilation

Functional (non-matching) rebuild of Dishonored's Win32 native executable from reconstructed
C++ source, targeting the retail 2013 cooked content. The symbolized 2012 QA build
(`../Dishonored_Debug2012`) is the source of truth for names, types and behavior; the retail
2013 build (`../Dishonored_Latest2026`) is the content target. Game binaries, PDBs, content and
IDA databases stay outside the repo.

Start with [PLAN.md](PLAN.md) for the overall strategy and phases, then
[docs/PHASE1.md](docs/PHASE1.md) for the current task tracker. Symbol exports live in
`docs/symbols/` and `docs/types/`; scripts that produce them are under `tools/`.
