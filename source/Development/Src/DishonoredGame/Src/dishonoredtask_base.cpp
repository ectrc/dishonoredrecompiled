// DishonoredGame/src/dishonoredtask_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (16):
//   0x721f20  public: static void __cdecl UDishonoredTask_Base::InitializePrivateStaticClassUDishonoredTask_Base(void)
//   0x721f40  public: void __thiscall UDishonoredTask_Base::OnDisSeqAct_UpdateCustomTask(class UDisSeqAct_UpdateCustomTask *)
//   0x721f50  public: unsigned int __thiscall UDishonoredTask_Base::IsComplete(void)const
//   0x721f60  public: unsigned int __thiscall UDishonoredTask_Base::IsFailed(void)const
//   0x721f70  public: unsigned int __thiscall UDishonoredTask_Base::IsHidden(void)const
//   0x721f80  public: enum EDisTaskState __thiscall UDishonoredTask_Base::GetTaskState(void)const
//   0x721f90  public: void __thiscall UDishonoredTask_Base::SetTaskState(enum EDisTaskState)
//   0x721fd0  public: class TArray<struct FDisTaskTarget, class FDefaultAllocator> & __thiscall UDishonoredTask_Base::GetTargets(void)
//   0x721fe0  public: unsigned int __thiscall UDishonoredTask_Base::ShouldShowHUDMarkers(void)const
//   0x721ff0  public: void __thiscall UDishonoredTask_Base::ShowHUDMarkers(unsigned int)
//   0x727020  public: void __thiscall UDishonoredTask_Base::SetTaskHidden(unsigned int)
//   0x727060  public: virtual void __thiscall UDishonoredTask_Base::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x72d7d0  public: class FString __thiscall UDishonoredTask_Base::GetStatusString(void)const
//   0x735eb0  public: virtual unsigned int __thiscall UDishonoredTask_Base::IsSaveable(enum ESaveLoadLocation)const
//   0x738670  public: static class UClass * __cdecl UDishonoredTask_Base::GetPrivateStaticClassUDishonoredTask_Base(wchar_t const *)
//   0x739120  public: static class UClass * __cdecl UDishonoredTask_Base::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI): a task instance ----
#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x6c1770 (2012 0x721f50)
UBOOL UDishonoredTask_Base::IsComplete() const
{
	return m_TaskState == DTS_Completed;
}

// DISHONORED(port): 2013 rva 0x6c1780 (2012 0x721f60)
UBOOL UDishonoredTask_Base::IsFailed() const
{
	return m_TaskState == DTS_Failed;
}

// DISHONORED(port): 2013 rva 0x6c1790 (2012 0x721f70)
UBOOL UDishonoredTask_Base::IsHidden() const
{
	return m_bHidden;
}

// DISHONORED(port): 2013 rva 0x6c17a0 (2012 0x721f80)
BYTE UDishonoredTask_Base::GetTaskState() const
{
	return m_TaskState;
}

// DISHONORED(port): 2013 rva 0x6c17b0 (2012 0x721f90): becoming active announces the task as new.
void UDishonoredTask_Base::SetTaskState( BYTE _NewState )
{
	if( m_TaskState == _NewState )
	{
		return;
	}
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag task %s.%s state %d -> %d"), GetOuter() ? *GetOuter()->GetName() : TEXT("None"), *GetName(),
			(INT)m_TaskState, (INT)_NewState );
	}
	m_TaskState = _NewState;
	if( _NewState == DTS_Active )
	{
		DisSendObjectiveChangedNotify( DOCNET_NewTask, this );
	}
	DisSendObjectiveChangedNotify( DOCNET_TaskStateChanged, this );
}

// DISHONORED(port): 2012 rva 0x721fd0
TArray<FDisTaskTarget>& UDishonoredTask_Base::GetTargets()
{
	return m_Targets;
}

// DISHONORED(port): 2013 rva 0x6c17f0 (2012 0x721fe0)
UBOOL UDishonoredTask_Base::ShouldShowHUDMarkers() const
{
	return m_bShowHUDMarkers;
}

// DISHONORED(port): 2013 rva 0x6c1800 (2012 0x721ff0)
void UDishonoredTask_Base::ShowHUDMarkers( UBOOL _bShow )
{
	m_bShowHUDMarkers = _bShow ? 1 : 0;
}

// DISHONORED(port): 2013 rva 0x6c63c0 (2012 0x727020): revealing an active task announces it as new.
void UDishonoredTask_Base::SetTaskHidden( UBOOL _bHidden )
{
	if( m_bHidden == ( _bHidden ? 1u : 0u ) )
	{
		return;
	}
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag task %s hidden %d"), *GetName(), _bHidden ? 1 : 0 );
	}
	m_bHidden = _bHidden ? 1 : 0;
	if( !m_bHidden && m_TaskState == DTS_Active )
	{
		DisSendObjectiveChangedNotify( DOCNET_NewTask, this );
	}
}

// DISHONORED(port): 2013 rva 0x6cc220 (2012 0x72d7d0): the class's status text (vtable +300).
FString UDishonoredTask_Base::GetStatusString() const
{
	return GetStatusString_Impl();
}

// DISHONORED(port): 2013 rva 0x6c6300 (no 2012 counterpart: 2012's UDishonoredObjective::OnAdded 0x72d1c0 did this inline
// and had no task ids). The task starts in its initial state, takes the next id of the player's objectives component and
// shows markers when its objective does and profile setting 95 reads 1.
void UDishonoredTask_Base::OnAdded( FName _TemplateTaskName )
{
	const UDishonoredObjective* Objective = (const UDishonoredObjective*)GetOuter();
	m_TemplateTaskName = _TemplateTaskName;
	INT ShowMarkers = 0;
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	UArkProfileSettings* Profile = PlayerController ? PlayerController->GetProfileSettings() : NULL;
	if( Profile )
	{
		Profile->GetProfileSettingValueId( 95, ShowMarkers );
	}
	m_TaskState = m_InitialState;
	m_bHidden = m_bInitiallyHidden;
	if( PlayerController && PlayerController->m_pObjectivesComponent )
	{
		m_TaskID = ++PlayerController->m_pObjectivesComponent->m_LastTaskID;
	}
	m_bShowHUDMarkers = !m_bHUDMarkersInitiallyHidden && Objective->m_bShowHUDMarkers && ShowMarkers == 1;
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag task add %s template %s id %d state %d hidden %d markers %d targets %d '%s'"), *GetName(),
			*m_TemplateTaskName.ToString(), m_TaskID, (INT)m_TaskState, (INT)m_bHidden, (INT)m_bShowHUDMarkers, m_Targets.Num(), *m_Description );
	}
	DisSendObjectiveChangedNotify( DOCNET_NewTask, this );
	DisSendObjectiveChangedNotify( DOCNET_TaskStateChanged, this );
	OnAdded_Impl();
}

// DISHONORED(port): 2012 rva 0x721f40: a thunk to the class's vtable +292.
void UDishonoredTask_Base::OnDisSeqAct_UpdateCustomTask( UDisSeqAct_UpdateCustomTask* _pAction )
{
	OnDisSeqAct_UpdateCustomTask_Impl( _pAction );
}

void DishonoredLinkDishonoredTaskBase()
{
}
