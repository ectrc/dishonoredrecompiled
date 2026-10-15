// DishonoredGame/src/disseqact_updatecustomtask.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cbb80  public: virtual void __thiscall UDisSeqAct_UpdateCustomTask::HandleTaskInstance(class UDishonoredTask_Base *)
//   0x7ec6f0  public: static class UClass * __cdecl UDisSeqAct_UpdateCustomTask::GetPrivateStaticClassUDisSeqAct_UpdateCustomTask(wchar_t const *)
//   0x7ef5b0  public: static void __cdecl UDisSeqAct_UpdateCustomTask::InitializePrivateStaticClassUDisSeqAct_UpdateCustomTask(void)
//   0x7f0850  public: static class UClass * __cdecl UDisSeqAct_UpdateCustomTask::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x786880 (2012 0x7cbb80)
void UDisSeqAct_UpdateCustomTask::HandleTaskInstance( UDishonoredTask_Base* _pTask )
{
	_pTask->OnDisSeqAct_UpdateCustomTask( this );
}

void DishonoredLinkDisSeqActUpdateCustomTask()
{
}
