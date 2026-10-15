// DishonoredGame/src/disseqact_setobjectivehidden.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbb20  private: virtual void __thiscall UDisSeqAct_SetObjectiveHidden::HandlePackageObjective(class UDishonoredObjectivesComponent *, class UDishonoredObjective const *)
//   0x7ec4b0  public: static class UClass * __cdecl UDisSeqAct_SetObjectiveHidden::GetPrivateStaticClassUDisSeqAct_SetObjectiveHidden(wchar_t const *)
//   0x7ef560  public: static void __cdecl UDisSeqAct_SetObjectiveHidden::InitializePrivateStaticClassUDisSeqAct_SetObjectiveHidden(void)
//   0x7f07e0  public: static class UClass * __cdecl UDisSeqAct_SetObjectiveHidden::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x786800 (2012 0x7cbb20)
void UDisSeqAct_SetObjectiveHidden::HandlePackageObjective( UDishonoredObjectivesComponent* _pObjComp, const UDishonoredObjective* _pPackageObjective )
{
	_pObjComp->SetObjectiveHidden( _pPackageObjective, m_bHidden );
}

void DishonoredLinkDisSeqActSetObjectiveHidden()
{
}
