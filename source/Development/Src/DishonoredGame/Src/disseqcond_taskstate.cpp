// DishonoredGame/src/disseqcond_taskstate.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb3f0  public: static void __cdecl UDisSeqCond_TaskState::InitializePrivateStaticClassUDisSeqCond_TaskState(void)
//   0x7dd030  public: virtual void __thiscall UDisSeqCond_TaskState::Activated(void)
//   0x7ea410  public: static class UClass * __cdecl UDisSeqCond_TaskState::GetPrivateStaticClassUDisSeqCond_TaskState(wchar_t const *)
//   0x7edd80  public: static class UClass * __cdecl UDisSeqCond_TaskState::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x7a78c0 (2012 0x7dd030): the output named by the state of the first "Task" variable's task.
void UDisSeqCond_TaskState::Activated()
{
	USequenceOp::Activated();
	const UDishonoredTask_Base* PackageTask = NULL;
	{
		FMemMark Mark( GMainThreadMemStack );
		TArray<const UDishonoredTask_Base*, TMemStackAllocator<GMainThreadMemStack> > Tasks;
		DisGetTaskVars( this, TEXT("Task"), Tasks );
		if( Tasks.Num() > 0 )
		{
			PackageTask = Tasks(0);
		}
	}
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	if( PackageTask == NULL || PlayerController == NULL || PlayerController->m_pObjectivesComponent == NULL )
	{
		return;
	}
	const UDishonoredTask_Base* Task = PlayerController->m_pObjectivesComponent->GetTaskInstance( PackageTask );
	if( Task )
	{
		OutputLinks( Task->GetTaskState() ).bHasImpulse = TRUE;
	}
}

void DishonoredLinkDisSeqCondTaskState()
{
}
