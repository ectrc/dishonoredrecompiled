// DishonoredGame/src/disseqvar_task.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x7f3cd0  public: static void __cdecl UDisSeqVar_Task::InitializePrivateStaticClassUDisSeqVar_Task(void)
//   0x7f5040  public: virtual void __thiscall UDisSeqVar_Task::PostLoad(void)
//   0x7f7e20  private: class UDishonoredTask_Base * __thiscall UDisSeqVar_Task::GetTaskPtr(void)const
//   0x801cc0  public: virtual class UObject * __thiscall UDisSeqVar_Task::GetObjectValue(void)
//   0x801cd0  public: virtual class UObject * * __thiscall UDisSeqVar_Task::GetObjectRef(int)
//   0x801cf0  public: virtual class FString __thiscall UDisSeqVar_Task::GetValueStr(void)
//   0x814860  public: static class UClass * __cdecl UDisSeqVar_Task::GetPrivateStaticClassUDisSeqVar_Task(wchar_t const *)
//   0x816c50  public: static class UClass * __cdecl UDisSeqVar_Task::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x786cd0 (2012 0x7f5040): an old variable that pointed at the task object keeps its name only.
void UDisSeqVar_Task::PostLoad()
{
	Super::PostLoad();
	if( m_pTask )
	{
		m_TaskName = m_pTask->GetFName();
		m_pTask = NULL;
	}
}

// DISHONORED(port): 2013 rva 0x792210 (2012 0x7f7e20): the package objective's task of that name.
UDishonoredTask_Base* UDisSeqVar_Task::GetTaskPtr() const
{
	if( m_pObjective == NULL )
	{
		return NULL;
	}
	for( INT TaskIdx = 0; TaskIdx < m_pObjective->m_Tasks.Num(); TaskIdx++ )
	{
		UDishonoredTask_Base* Task = m_pObjective->m_Tasks(TaskIdx);
		if( Task && Task->GetFName() == m_TaskName )
		{
			return Task;
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x79cda0 (2012 0x801cc0)
UObject* UDisSeqVar_Task::GetObjectValue()
{
	m_pDummyTask = GetTaskPtr();
	return m_pDummyTask;
}

// DISHONORED(port): 2013 rva 0x79cdb0 (2012 0x801cd0)
UObject** UDisSeqVar_Task::GetObjectRef( INT Idx )
{
	m_pDummyTask = GetTaskPtr();
	return Idx == 0 ? &m_pDummyTask : NULL;
}

// DISHONORED(port): 2013 rva 0x79cdd0 (2012 0x801cf0)
FString UDisSeqVar_Task::GetValueStr()
{
	const UDishonoredTask_Base* Task = GetTaskPtr();
	return Task ? FString::Printf( TEXT("Task:  %s"), *Task->m_Description ) : FString( TEXT("No Task") );
}

void UDisSeqVar_Task::execGetObjectValue( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UObject**)Result = GetObjectValue();
}

void DishonoredLinkDisSeqVarTask()
{
}
