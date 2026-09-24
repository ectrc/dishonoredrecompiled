# Dishonored Recompilation

Functional (non-matching) rebuild of Dishonored's Win32 native executable, targeting the retail
2013 cooked content. The base is the UE3 build 10897 source tree at `../UnrealEngine3`
(CodeRedModding/UnrealEngine3), a very close engine build to Dishonored's 9014/9411; see
`resources/docs/engine_reference.md`. The symbolized 2012 QA build (`../Dishonored_Debug2012`) is the
source of truth for names, types and Arkane's changes; the retail 2013 build
(`../Dishonored_Latest2026`) is the content target. Game binaries, PDBs, content, IDA databases
and the reference tree stay outside the repo.

Start with [PLAN.md](PLAN.md) for the overall strategy and phases, then
[resources/docs/PHASE1.md](resources/docs/PHASE1.md) for the current task tracker. Symbol exports live in
`resources/docs/symbols/` and `resources/docs/types/`; scripts that produce them are under `resources/tools/`.
