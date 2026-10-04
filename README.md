# Dishonored Recompilation

Not fully complete yet, will pick this up next month

Functional (non-matching) rebuild of Dishonored's Win32 native executable, targeting the **retail
2013 build** (`Dishonored.exe`, engine 9411, DLC05–07): its layouts, serialization and behavior are
the contract. The symbolized 2012 QA build only supplies names and decompiles; its structs differ
from 2013 in many places and are never the final word. The base is the UE3 build 10897 source tree at `../UnrealEngine3`
(CodeRedModding/UnrealEngine3), a very close engine build to Dishonored's 9014/9411; see
`resources/docs/engine_reference.md`. Inputs: the retail 2013 build (`../Dishonored_Latest2026`,
the target) and the symbolized 2012 QA build (`../Dishonored_Debug2012`, the helping hand for
names and decompiles). Game binaries, PDBs, content, IDA databases and the reference tree stay
outside the repo.

Start with [PLAN.md](PLAN.md) for the overall strategy and phases, then
[resources/docs/PHASE1.md](resources/docs/PHASE1.md) for the current task tracker. Symbol exports live in
`resources/docs/symbols/` and `resources/docs/types/`; scripts that produce them are under `resources/tools/`.

All generated with claude code.

<img width="3202" height="941" alt="image" src="https://github.com/user-attachments/assets/3928471b-3744-4573-9abc-5a31e8596e6d" />
<img width="3204" height="936" alt="image" src="https://github.com/user-attachments/assets/6ba9858a-c7b2-4e73-bbf0-e84c296631cb" />
<img width="3218" height="944" alt="image" src="https://github.com/user-attachments/assets/c13eacaa-b430-43e3-a9fb-3507cc14346a" />
<img width="3205" height="934" alt="image" src="https://github.com/user-attachments/assets/428f5a65-a2fc-406a-9906-26d577982b61" />
<img width="3214" height="937" alt="image" src="https://github.com/user-attachments/assets/6524153a-08a6-42ab-b2ae-89df491f320d" />
<img width="1621" height="965" alt="image" src="https://github.com/user-attachments/assets/27976b10-fc5c-44df-bb07-78607ff69bbb" />
