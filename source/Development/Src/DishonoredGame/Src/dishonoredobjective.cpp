// DishonoredGame/src/dishonoredobjective.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x721e80  public: static void __cdecl UDishonoredObjective::InitializePrivateStaticClassUDishonoredObjective(void)
//   0x721ea0  public: enum EDisObjectiveState __thiscall UDishonoredObjective::GetObjectiveState(void)const
//   0x721eb0  public: unsigned int __thiscall UDishonoredObjective::IsActive(void)const
//   0x721ec0  public: unsigned int __thiscall UDishonoredObjective::IsHidden(void)const
//   0x721ed0  public: unsigned int __thiscall UDishonoredObjective::ShouldShowHUDMarkers(void)const
//   0x721ee0  public: void __thiscall UDishonoredObjective::ShowHUDMarkers(unsigned int)
//   0x72a320  public: void __thiscall UDishonoredObjective::OnTaskCompletedNotify(class UDishonoredTask_Base * const)
//   0x72a3b0  public: void __thiscall UDishonoredObjective::FailObjective(void)
//   0x72a430  public: void __thiscall UDishonoredObjective::SetHidden(unsigned int)
//   0x72a4f0  public: class UDishonoredTask_Base * __thiscall UDishonoredObjective::GetTaskInstance(class UDishonoredTask_Base const *)const
//   0x72d0e0  public: class FName __thiscall UDishonoredObjective::MakeObjectiveNameFromPackageObject(void)const
//   0x72d1c0  public: void __thiscall UDishonoredObjective::OnAdded(void)
//   0x72f580  public: virtual void __thiscall UDishonoredObjective::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x72f600  public: virtual void __thiscall UDishonoredObjective::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x731230  public: static class UClass * __cdecl UDishonoredObjective::GetPrivateStaticClassUDishonoredObjective(wchar_t const *)
//   0x734da0  public: static class UClass * __cdecl UDishonoredObjective::StaticClassNoInline(void)
//   0x735e80  public: virtual unsigned int __thiscall UDishonoredObjective::IsSaveable(enum ESaveLoadLocation)const

// ---- agent HI (PHASE22 HI): an objective instance and its tasks ----
#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x6c16c0 (2012 0x721eb0)
UBOOL UDishonoredObjective::IsActive() const
{
	return m_ObjectiveState == DOS_Active;
}

// DISHONORED(port): 2013 rva 0x6c16d0 (2012 0x721ec0)
UBOOL UDishonoredObjective::IsHidden() const
{
	return m_bIsHidden;
}

// DISHONORED(port): 2013 rva 0x6c16e0 (2012 0x721ed0)
UBOOL UDishonoredObjective::ShouldShowHUDMarkers() const
{
	return m_bShowHUDMarkers;
}

// DISHONORED(port): 2013 rva 0x6c16f0 (2012 0x721ee0)
void UDishonoredObjective::ShowHUDMarkers( UBOOL _bShow )
{
	m_bShowHUDMarkers = _bShow ? 1 : 0;
}

// DISHONORED(port): 2013 rva 0x6d0a60 (2012 0x72d0e0 MakeObjectiveNameFromPackageObject, which turned the path's dots into
// underscores). An instance inside the objectives component answers the template name OnAdded stored; a package objective
// answers its own path name.
FName UDishonoredObjective::GetTemplateName() const
{
	if( IsInA( UDishonoredObjectivesComponent::StaticClass() ) )
	{
		return m_TemplateObjectiveName;
	}
	return FName( *GetPathName() );
}

// DISHONORED(port): 2013 rva 0x6dc1c0 (2012 0x72d1c0 OnAdded, which had no package argument and no profile read). Profile
// setting 95 (the objective-marker option) must read value 1 for any marker of this objective to show.
void UDishonoredObjective::OnAdded( const UDishonoredObjective* _pPackageObjective )
{
	m_TemplateObjectiveName = FName( *_pPackageObjective->GetPathName() );
	INT ShowMarkers = 0;
	UArkProfileSettings* Profile = ADishonoredPlayerController::s_pInstance ? ADishonoredPlayerController::s_pInstance->GetProfileSettings() : NULL;
	if( Profile )
	{
		Profile->GetProfileSettingValueId( 95, ShowMarkers );
	}
	m_bIsHidden = m_bInitiallyHidden;
	m_bShowHUDMarkers = !m_bHUDMarkersInitiallyHidden && ShowMarkers == 1;

	FMemMark Mark( GMainThreadMemStack );
	TArray<UDishonoredTask_Base*, TMemStackAllocator<GMainThreadMemStack> > PackageTasks;
	PackageTasks = _pPackageObjective->m_Tasks;
	for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
	{
		if( m_Tasks(TaskIdx) == NULL )
		{
			m_Tasks.Remove( TaskIdx );
			PackageTasks.Remove( TaskIdx );
			TaskIdx--;
		}
	}
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag objective OnAdded %s template %s hidden %d markers %d (profile 95 = %d, %s), %d tasks"),
			*GetName(), *m_TemplateObjectiveName.ToString(), (INT)m_bIsHidden, (INT)m_bShowHUDMarkers, ShowMarkers,
			Profile ? TEXT("profile") : TEXT("no profile"), m_Tasks.Num() );
	}
	DisSendObjectiveChangedNotify( DOCNET_GotObjective, this );
	for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
	{
		m_Tasks(TaskIdx)->OnAdded( FName( *PackageTasks(TaskIdx)->GetPathName() ) );
		m_Tasks(TaskIdx)->m_bOptional |= m_bOptional;
	}
}

// DISHONORED(port): 2013 rva 0x6cbc70 (2012 0x72a320): an objective that completes with its tasks completes when every
// task has.
void UDishonoredObjective::OnTaskCompletedNotify( UDishonoredTask_Base* _pTask )
{
	if( !m_bCompleteWhenAllTasksComplete || m_ObjectiveState == DOS_Completed )
	{
		return;
	}
	for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
	{
		if( m_Tasks(TaskIdx)->m_TaskState != DTS_Completed )
		{
			return;
		}
	}
	m_ObjectiveState = DOS_Completed;
	DisSendObjectiveChangedNotify( DOCNET_CompletedObjective, this );
}

// DISHONORED(port): 2013 rva 0x6cbd00 (2012 0x72a3b0): the active tasks fail with it.
void UDishonoredObjective::FailObjective()
{
	if( m_ObjectiveState == DOS_Failed )
	{
		return;
	}
	m_ObjectiveState = DOS_Failed;
	DisSendObjectiveChangedNotify( DOCNET_FailedObjective, this );
	for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
	{
		UDishonoredTask_Base* Task = m_Tasks(TaskIdx);
		if( Task->m_TaskState == DTS_Active )
		{
			Task->m_TaskState = DTS_Failed;
			DisSendObjectiveChangedNotify( DOCNET_TaskStateChanged, Task );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6cbd80 (2012 0x72a430): revealing an objective announces it again in its current state.
void UDishonoredObjective::SetHidden( UBOOL _bHidden )
{
	if( m_bIsHidden == ( _bHidden ? 1u : 0u ) )
	{
		return;
	}
	m_bIsHidden = _bHidden ? 1 : 0;
	if( m_bIsHidden )
	{
		return;
	}
	DisSendObjectiveChangedNotify( DOCNET_GotObjective, this );
	if( m_ObjectiveState == DOS_Completed )
	{
		DisSendObjectiveChangedNotify( DOCNET_CompletedObjective, this );
	}
	else if( m_ObjectiveState == DOS_Failed )
	{
		DisSendObjectiveChangedNotify( DOCNET_FailedObjective, this );
	}
	else
	{
		for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
		{
			if( m_Tasks(TaskIdx)->m_TaskState == DTS_Active )
			{
				DisSendObjectiveChangedNotify( DOCNET_NewTask, m_Tasks(TaskIdx) );
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x6cbe40 (2012 0x72a4f0): the instance task is the one with the package task's name.
UDishonoredTask_Base* UDishonoredObjective::GetTaskInstance( const UDishonoredTask_Base* _pPackageTask ) const
{
	for( INT TaskIdx = 0; TaskIdx < m_Tasks.Num(); TaskIdx++ )
	{
		if( m_Tasks(TaskIdx)->GetFName() == _pPackageTask->GetFName() )
		{
			return m_Tasks(TaskIdx);
		}
	}
	return NULL;
}

void DishonoredLinkDishonoredObjective()
{
}
