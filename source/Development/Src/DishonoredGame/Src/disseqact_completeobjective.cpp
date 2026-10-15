// DishonoredGame/src/disseqact_completeobjective.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbac0  public: virtual void __thiscall UDisSeqAct_CompleteObjective::HandlePackageObjective(class UDishonoredObjectivesComponent *, class UDishonoredObjective const *)
//   0x7ec270  public: static class UClass * __cdecl UDisSeqAct_CompleteObjective::GetPrivateStaticClassUDisSeqAct_CompleteObjective(wchar_t const *)
//   0x7f0770  public: static void __cdecl UDisSeqAct_CompleteObjective::InitializePrivateStaticClassUDisSeqAct_CompleteObjective(void)
//   0x7f0d50  public: static class UClass * __cdecl UDisSeqAct_CompleteObjective::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7867a0 (2012 0x7cbac0)
void UDisSeqAct_CompleteObjective::HandlePackageObjective( UDishonoredObjectivesComponent* _pObjComp, const UDishonoredObjective* _pPackageObjective )
{
	_pObjComp->CompleteObjective( _pPackageObjective );
}

void DishonoredLinkDisSeqActCompleteObjective()
{
}
