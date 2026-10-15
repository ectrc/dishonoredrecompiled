// DishonoredGame/src/disseqact_failobjective.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbae0  public: virtual void __thiscall UDisSeqAct_FailObjective::HandlePackageObjective(class UDishonoredObjectivesComponent *, class UDishonoredObjective const *)
//   0x7ec300  public: static class UClass * __cdecl UDisSeqAct_FailObjective::GetPrivateStaticClassUDisSeqAct_FailObjective(wchar_t const *)
//   0x7f0790  public: static void __cdecl UDisSeqAct_FailObjective::InitializePrivateStaticClassUDisSeqAct_FailObjective(void)
//   0x7f0d80  public: static class UClass * __cdecl UDisSeqAct_FailObjective::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7867c0 (2012 0x7cbae0)
void UDisSeqAct_FailObjective::HandlePackageObjective( UDishonoredObjectivesComponent* _pObjComp, const UDishonoredObjective* _pPackageObjective )
{
	_pObjComp->FailObjective( _pPackageObjective );
}

void DishonoredLinkDisSeqActFailObjective()
{
}
