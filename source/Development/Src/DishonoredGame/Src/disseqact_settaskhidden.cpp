// DishonoredGame/src/disseqact_settaskhidden.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbb40  public: virtual void __thiscall UDisSeqAct_SetTaskHidden::HandleTaskInstance(class UDishonoredTask_Base *)
//   0x7ec540  public: static class UClass * __cdecl UDisSeqAct_SetTaskHidden::GetPrivateStaticClassUDisSeqAct_SetTaskHidden(wchar_t const *)
//   0x7f0810  public: static void __cdecl UDisSeqAct_SetTaskHidden::InitializePrivateStaticClassUDisSeqAct_SetTaskHidden(void)
//   0x7f0db0  public: static class UClass * __cdecl UDisSeqAct_SetTaskHidden::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x786820 (2012 0x7cbb40)
void UDisSeqAct_SetTaskHidden::HandleTaskInstance( UDishonoredTask_Base* _pTask )
{
	_pTask->SetTaskHidden( m_bHidden );
}

void DishonoredLinkDisSeqActSetTaskHidden()
{
}
