// DishonoredGame/src/disseqact_removeobjective.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbb00  public: virtual void __thiscall UDisSeqAct_RemoveObjective::HandlePackageObjective(class UDishonoredObjectivesComponent *, class UDishonoredObjective const *)
//   0x7ec420  public: static class UClass * __cdecl UDisSeqAct_RemoveObjective::GetPrivateStaticClassUDisSeqAct_RemoveObjective(wchar_t const *)
//   0x7ef540  public: static void __cdecl UDisSeqAct_RemoveObjective::InitializePrivateStaticClassUDisSeqAct_RemoveObjective(void)
//   0x7f07b0  public: static class UClass * __cdecl UDisSeqAct_RemoveObjective::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7867e0 (2012 0x7cbb00)
void UDisSeqAct_RemoveObjective::HandlePackageObjective( UDishonoredObjectivesComponent* _pObjComp, const UDishonoredObjective* _pPackageObjective )
{
	_pObjComp->RemoveObjective( _pPackageObjective );
}

void DishonoredLinkDisSeqActRemoveObjective()
{
}
