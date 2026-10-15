// DishonoredGame/src/dishonoredobjectivescomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (23):
//   0x721f00  public: static void __cdecl UDishonoredObjectivesComponent::InitializePrivateStaticClassUDishonoredObjectivesComponent(void)
//   0x72a600  private: void __thiscall UDishonoredObjectivesComponent::OnGotObjectiveNotify(class UDishonoredObjective *)
//   0x72a690  private: void __thiscall UDishonoredObjectivesComponent::OnFailedObjectiveNotify(class UDishonoredObjective *)
//   0x72d340  public: void __thiscall UDishonoredObjectivesComponent::ClearObjectives(void)
//   0x72d370  private: class UDishonoredObjective * __thiscall UDishonoredObjectivesComponent::FindObjectiveInstance(class UDishonoredObjective const * const, int *)const
//   0x72d450  private: void __thiscall UDishonoredObjectivesComponent::PopInactiveObjectiveInstance(int)
//   0x72d5d0  public: class UDishonoredTask_Base * __thiscall UDishonoredObjectivesComponent::GetTaskInstance(class UDishonoredTask_Base const *)const
//   0x72d610  private: void __thiscall UDishonoredObjectivesComponent::OnCompletedObjectiveNotify(class UDishonoredObjective *)
//   0x72d7b0  public: unsigned int __thiscall UDishonoredObjectivesComponent::HasObjective(class UDishonoredObjective const *)const
//   0x72f670  public: void __thiscall UDishonoredObjectivesComponent::RemoveObjective(class UDishonoredObjective const *)
//   0x72f710  public: void __thiscall UDishonoredObjectivesComponent::CompleteObjective(class UDishonoredObjective const *)
//   0x72f760  public: void __thiscall UDishonoredObjectivesComponent::FailObjective(class UDishonoredObjective const *)
//   0x72f7a0  public: void __thiscall UDishonoredObjectivesComponent::SetObjectiveHidden(class UDishonoredObjective const *, unsigned int)const
//   0x72f7d0  public: unsigned int __thiscall UDishonoredObjectivesComponent::IsObjectiveComplete(class UDishonoredObjective *)
//   0x72f800  public: virtual void __thiscall UDishonoredObjectivesComponent::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7312c0  public: static class UClass * __cdecl UDishonoredObjectivesComponent::GetPrivateStaticClassUDishonoredObjectivesComponent(wchar_t const *)
//   0x731350  public: void __thiscall UDishonoredObjectivesComponent::FindActiveTaskTargets(struct TMemStackArray<struct FDisTaskTarget const *> &, unsigned int)const
//   0x734dd0  public: static class UClass * __cdecl UDishonoredObjectivesComponent::StaticClassNoInline(void)
//   0x738020  private: void __thiscall UDishonoredObjectivesComponent::OnNewTaskNotify(class UDishonoredTask_Base *)
//   0x7382a0  private: void __thiscall UDishonoredObjectivesComponent::OnTaskUpdatedNotify(class UDishonoredTask_Base *)
//   0x7383a0  private: void __thiscall UDishonoredObjectivesComponent::OnTaskStateChangedNotify(class UDishonoredTask_Base *)
//   0x7390a0  public: void __thiscall UDishonoredObjectivesComponent::OnObjectiveChangedNotify(enum EDisObjectiveChangedNotifyType, class UObject *)
//   0x7404e0  public: void __thiscall UDishonoredObjectivesComponent::AddObjective(class UDishonoredObjective const *)

// ---- agent HI (PHASE22 HI): the objective list of the player controller ----
// DISHONORED(port): ADishonoredPlayerController::m_pObjectivesComponent is a default subobject of the script class (no
// native code in either exe writes the member). A Kismet DisSeqAct_*Objective action names package objectives (content
// objects such as Tower_Objectives.TowerEmpress_Report); AddObjective duplicates one into this component, and every later
// action finds the instance again by the package objective's path name, which UDishonoredObjective::OnAdded keeps in
// m_TemplateObjectiveName. Every change is announced through DisSendObjectiveChangedNotify, which lands in
// OnObjectiveChangedNotify and from there in the HUD's popup queue and the player pawn's task Kismet events.
// DISHONORED(written): retail also calls every entry of m_Observers (interface slots +12..+36). No class in either PDB
// implements IDisObjectivesObserverInterface (vtables.csv lists no implementer) and nothing adds to the array, so the
// observer loops are left out.

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

/** -hidiag: every objective and task creation, state change, notify and task marker instance */
UBOOL UDishonoredObjectivesComponent::IsDiagEnabled()
{
	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("hidiag") );
	return bDiag;
}

// DISHONORED(port): 2013 rva 0x6d0bf0 (2012 0x72d370). 2013 compares template names where 2012 built a name out of the
// package path; the last match wins, as in both.
UDishonoredObjective* UDishonoredObjectivesComponent::FindObjectiveInstance( const UDishonoredObjective* _pPackageObjective, INT* _pOutFoundIndex ) const
{
	UDishonoredObjective* Found = NULL;
	const FName TemplateName = _pPackageObjective->GetTemplateName();
	for( INT Index = 0; Index < m_Objectives.Num(); Index++ )
	{
		if( m_Objectives(Index)->GetTemplateName() == TemplateName )
		{
			Found = m_Objectives(Index);
			if( _pOutFoundIndex )
			{
				*_pOutFoundIndex = Index;
			}
		}
	}
	return Found;
}

// DISHONORED(port): 2013 rva 0x6e0b60 (2012 0x7404e0). The duplicate keeps an automatic name (2012 named it after the
// package path) and goes in front of the first active objective it outranks: mandatory before optional, then the higher
// m_Priority first. The package objective's tasks are its subobjects, so they are duplicated with it under their own names.
void UDishonoredObjectivesComponent::AddObjective( const UDishonoredObjective* _pPackageObjective )
{
	if( _pPackageObjective == NULL || FindObjectiveInstance( _pPackageObjective, NULL ) )
	{
		return;
	}
	FObjectDuplicationParameters Params( const_cast<UDishonoredObjective*>( _pPackageObjective ), this );
	UDishonoredObjective* Objective = (UDishonoredObjective*)UObject::StaticDuplicateObjectEx( Params );
	INT Index = 0;
	for( ; Index < m_Objectives.Num(); Index++ )
	{
		const UDishonoredObjective* Other = m_Objectives(Index);
		if( Other->m_ObjectiveState == DOS_Active
			&& ( Objective->m_bOptional < Other->m_bOptional
				|| ( Objective->m_bOptional == Other->m_bOptional && Objective->m_Priority > Other->m_Priority ) ) )
		{
			break;
		}
	}
	m_Objectives.InsertItem( Objective, Index );
	if( IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag objective add %s -> %s at %d of %d, %d tasks, '%s'"), *_pPackageObjective->GetPathName(),
			*Objective->GetName(), Index, m_Objectives.Num(), Objective->m_Tasks.Num(), *Objective->m_Description );
	}
	Objective->OnAdded( _pPackageObjective );
}

// DISHONORED(port): 2013 rva 0x6d4dd0 (2012 0x72f670; the observer loop is left out, see the file header)
void UDishonoredObjectivesComponent::RemoveObjective( const UDishonoredObjective* _pPackageObjective )
{
	if( _pPackageObjective == NULL )
	{
		return;
	}
	INT Index = 0;
	if( FindObjectiveInstance( _pPackageObjective, &Index ) )
	{
		m_Objectives.Remove( Index );
		if( IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag objective remove %s"), *_pPackageObjective->GetPathName() );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6d0cc0 (2012 0x72d450, no 2013 name): a finished objective moves in front of the first active one.
void UDishonoredObjectivesComponent::PopInactiveObjectiveInstance( INT _iMoveFromIndex )
{
	UDishonoredObjective* Objective = m_Objectives(_iMoveFromIndex);
	m_Objectives.Remove( _iMoveFromIndex );
	INT Index = 0;
	while( Index < m_Objectives.Num() && m_Objectives(Index)->m_ObjectiveState != DOS_Active )
	{
		Index++;
	}
	m_Objectives.InsertItem( Objective, Index < m_Objectives.Num() ? Index : _iMoveFromIndex );
}

// DISHONORED(port): 2013 rva 0x6d4e70 (2012 0x72f710)
void UDishonoredObjectivesComponent::CompleteObjective( const UDishonoredObjective* _pPackageObjective )
{
	if( _pPackageObjective == NULL )
	{
		return;
	}
	INT Index = -1;
	UDishonoredObjective* Objective = FindObjectiveInstance( _pPackageObjective, &Index );
	if( Objective == NULL )
	{
		return;
	}
	PopInactiveObjectiveInstance( Index );
	if( Objective->m_ObjectiveState != DOS_Completed )
	{
		Objective->m_ObjectiveState = DOS_Completed;
		DisSendObjectiveChangedNotify( DOCNET_CompletedObjective, Objective );
	}
}

// DISHONORED(port): 2013 rva 0x6d4ec0 (2012 0x72f760)
void UDishonoredObjectivesComponent::FailObjective( const UDishonoredObjective* _pPackageObjective )
{
	if( _pPackageObjective == NULL )
	{
		return;
	}
	INT Index = -1;
	UDishonoredObjective* Objective = FindObjectiveInstance( _pPackageObjective, &Index );
	if( Objective == NULL )
	{
		return;
	}
	PopInactiveObjectiveInstance( Index );
	Objective->FailObjective();
}

// DISHONORED(port): 2013 rva 0x6d4f00 (2012 0x72f7a0)
void UDishonoredObjectivesComponent::SetObjectiveHidden( const UDishonoredObjective* _pPackageObjective, UBOOL _bHidden ) const
{
	if( _pPackageObjective == NULL )
	{
		return;
	}
	UDishonoredObjective* Objective = FindObjectiveInstance( _pPackageObjective, NULL );
	if( Objective )
	{
		Objective->SetHidden( _bHidden );
	}
}

// DISHONORED(port): 2013 rva 0x6d4f30 (2012 0x72f7d0)
UBOOL UDishonoredObjectivesComponent::IsObjectiveComplete( const UDishonoredObjective* _pPackageObjective ) const
{
	if( _pPackageObjective == NULL )
	{
		return FALSE;
	}
	const UDishonoredObjective* Objective = FindObjectiveInstance( _pPackageObjective, NULL );
	return Objective != NULL && Objective->m_ObjectiveState == DOS_Completed;
}

// DISHONORED(port): 2013 rva 0x6d1530 (2012 0x72d7b0)
UBOOL UDishonoredObjectivesComponent::HasObjective( const UDishonoredObjective* _pPackageObjective ) const
{
	return FindObjectiveInstance( _pPackageObjective, NULL ) != NULL;
}

// DISHONORED(port): 2012 rva 0x72d340 (the cheat manager's ClearObjectives)
void UDishonoredObjectivesComponent::ClearObjectives()
{
	m_Objectives.Empty();
}

// DISHONORED(port): 2013 rva 0x6d0dc0 (2012 0x72d5d0): a package task's objective is its Outer.
UDishonoredTask_Base* UDishonoredObjectivesComponent::GetTaskInstance( const UDishonoredTask_Base* _pPackageTask ) const
{
	if( _pPackageTask == NULL || _pPackageTask->GetOuter() == NULL )
	{
		return NULL;
	}
	const UDishonoredObjective* Objective = FindObjectiveInstance( (const UDishonoredObjective*)_pPackageTask->GetOuter(), NULL );
	return Objective ? Objective->GetTaskInstance( _pPackageTask ) : NULL;
}

// DISHONORED(port): 2013 rva 0x6d9910 (2012 0x731350; 2013's FDisTaskTarget is 36 bytes to 2012's 24). The targets of every
// visible, not finished task of every visible active objective whose actor is alive; for the HUD markers only those the
// objective and the task show markers for.
void UDishonoredObjectivesComponent::FindActiveTaskTargets( TArray<const FDisTaskTarget*, TMemStackAllocator<GMainThreadMemStack> >& _rTaskTargets, UBOOL _bForHUDMarkers ) const
{
	for( INT ObjectiveIdx = 0; ObjectiveIdx < m_Objectives.Num(); ObjectiveIdx++ )
	{
		const UDishonoredObjective* Objective = m_Objectives(ObjectiveIdx);
		if( Objective->m_ObjectiveState != DOS_Active || Objective->m_bIsHidden || ( _bForHUDMarkers && !Objective->m_bShowHUDMarkers ) )
		{
			continue;
		}
		for( INT TaskIdx = 0; TaskIdx < Objective->m_Tasks.Num(); TaskIdx++ )
		{
			const UDishonoredTask_Base* Task = Objective->m_Tasks(TaskIdx);
			if( Task->m_bHidden || Task->m_TaskState > DTS_Active || ( _bForHUDMarkers && !Task->m_bShowHUDMarkers ) )
			{
				continue;
			}
			for( INT TargetIdx = 0; TargetIdx < Task->m_Targets.Num(); TargetIdx++ )
			{
				const FDisTaskTarget& Target = Task->m_Targets(TargetIdx);
				if( Target.m_pActor && !Target.m_pActor->bDeleteMe && !Target.m_pActor->IsPendingKill() )
				{
					_rTaskTargets.AddItem( &Target );
				}
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x6d4f60 (2012 0x7390a0)
void UDishonoredObjectivesComponent::OnObjectiveChangedNotify( EDisObjectiveChangedNotifyType _Type, UObject* _pReferenceObject )
{
	if( IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag notify %d %s"), (INT)_Type, _pReferenceObject ? *_pReferenceObject->GetName() : TEXT("None") );
	}
	switch( _Type )
	{
	case DOCNET_GotObjective:       OnGotObjectiveNotify( (UDishonoredObjective*)_pReferenceObject );       break;
	case DOCNET_CompletedObjective: OnCompletedObjectiveNotify( (UDishonoredObjective*)_pReferenceObject ); break;
	case DOCNET_FailedObjective:    OnFailedObjectiveNotify( (UDishonoredObjective*)_pReferenceObject );    break;
	case DOCNET_NewTask:            OnNewTaskNotify( (UDishonoredTask_Base*)_pReferenceObject );             break;
	case DOCNET_TaskUpdated:        OnTaskUpdatedNotify( (UDishonoredTask_Base*)_pReferenceObject );         break;
	case DOCNET_TaskStateChanged:   OnTaskStateChangedNotify( (UDishonoredTask_Base*)_pReferenceObject );    break;
	default: break;
	}
}

// DISHONORED(port): 2013 rva 0x6d0f80 (unnamed; inlined in 2012's 0x738020 / 0x7383a0): a task event fires for the task of
// the same name in the objective made from the event's package objective.
static UBOOL DisTaskEventMatches( const UDisSeqEvent_TaskEvent* _pEvent, const UDishonoredTask_Base* _pTask )
{
	const UDishonoredObjective* Objective = Cast<UDishonoredObjective>( _pTask->GetOuter() );
	if( _pEvent->m_pObjective == NULL || _pEvent->m_pTask == NULL || Objective == NULL )
	{
		return FALSE;
	}
	return _pEvent->m_pTask->GetFName() == _pTask->GetFName() && Objective->GetTemplateName() == _pEvent->m_pObjective->GetTemplateName();
}

// DISHONORED(port): 2013 rva 0x6cc050 (2012 0x72a600)
// DISHONORED(bringup): this tree's DisGetGFxHUD answers NULL while the HUD movie is closed, so every HUD call is guarded.
void UDishonoredObjectivesComponent::OnGotObjectiveNotify( UDishonoredObjective* _pObjective )
{
	if( !_pObjective->m_bIsHidden && Cast<ADishonoredPlayerController>( Owner ) )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD )
		{
			HUD->OnObjectiveAdded( _pObjective );
		}
		else if( IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag OnObjectiveAdded %s dropped: no open HUD"), *_pObjective->GetName() );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6d0e00 (2012 0x72d610): the player pawn's DisSeqEvent_ObjectiveCompleted events for it fire.
void UDishonoredObjectivesComponent::OnCompletedObjectiveNotify( UDishonoredObjective* _pObjective )
{
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	if( PlayerPawn )
	{
		for( INT EventIdx = 0; EventIdx < PlayerPawn->GeneratedEvents.Num(); EventIdx++ )
		{
			UDisSeqEvent_ObjectiveCompleted* Event = Cast<UDisSeqEvent_ObjectiveCompleted>( PlayerPawn->GeneratedEvents(EventIdx) );
			if( Event && Event->m_pObjective && Event->m_pObjective->GetTemplateName() == _pObjective->GetTemplateName() )
			{
				Event->CheckActivate( PlayerPawn, PlayerPawn->Controller );
			}
		}
	}
	if( !_pObjective->m_bIsHidden )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD )
		{
			HUD->OnObjectiveCompleted( _pObjective );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6cc0e0 (2012 0x72a690)
void UDishonoredObjectivesComponent::OnFailedObjectiveNotify( UDishonoredObjective* _pObjective )
{
	if( !_pObjective->m_bIsHidden )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD )
		{
			HUD->OnObjectiveFailed( _pObjective );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6d1080 (2012 0x738020): the player pawn's DisSeqEvent_TaskActivated events for the task
// fire, and a visible task of a visible objective goes to the HUD's popup.
void UDishonoredObjectivesComponent::OnNewTaskNotify( UDishonoredTask_Base* _pTask )
{
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	if( PlayerPawn )
	{
		for( INT EventIdx = 0; EventIdx < PlayerPawn->GeneratedEvents.Num(); EventIdx++ )
		{
			UDisSeqEvent_TaskActivated* Event = Cast<UDisSeqEvent_TaskActivated>( PlayerPawn->GeneratedEvents(EventIdx) );
			if( Event && DisTaskEventMatches( Event, _pTask ) )
			{
				Event->CheckActivate( PlayerPawn, PlayerPawn->Controller );
			}
		}
	}
	if( _pTask->m_bHidden )
	{
		return;
	}
	const UDishonoredObjective* Objective = Cast<UDishonoredObjective>( _pTask->GetOuter() );
	if( Objective && Objective->m_bIsHidden )
	{
		return;
	}
	UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
	if( HUD )
	{
		HUD->OnTaskAdded( _pTask );
	}
	else if( IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag OnTaskAdded %s dropped: no open HUD"), *_pTask->GetName() );
	}
}

// DISHONORED(port): 2013 rva 0x6d1220 (2012 0x7382a0): a task whose status string is not empty shows as updated.
void UDishonoredObjectivesComponent::OnTaskUpdatedNotify( UDishonoredTask_Base* _pTask )
{
	const UDishonoredObjective* Objective = Cast<UDishonoredObjective>( _pTask->GetOuter() );
	if( _pTask->m_bHidden || ( Objective && Objective->m_bIsHidden ) )
	{
		return;
	}
	if( _pTask->GetStatusString().Len() > 0 && Cast<ADishonoredPlayerController>( Owner ) )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD )
		{
			HUD->OnTaskUpdated( _pTask );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6d1320 (2012 0x7383a0): the pawn's DisSeqEvent_TaskStateChanged events fire on the output
// named by the new state; a finished visible task goes to the popup; a completed one may complete its objective.
void UDishonoredObjectivesComponent::OnTaskStateChangedNotify( UDishonoredTask_Base* _pTask )
{
	UDishonoredObjective* Objective = Cast<UDishonoredObjective>( _pTask->GetOuter() );
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	if( PlayerPawn )
	{
		for( INT EventIdx = 0; EventIdx < PlayerPawn->GeneratedEvents.Num(); EventIdx++ )
		{
			UDisSeqEvent_TaskStateChanged* Event = Cast<UDisSeqEvent_TaskStateChanged>( PlayerPawn->GeneratedEvents(EventIdx) );
			if( Event && DisTaskEventMatches( Event, _pTask ) )
			{
				TArray<INT> ActivateIndices;
				ActivateIndices.AddItem( _pTask->m_TaskState );
				Event->CheckActivate( PlayerPawn, PlayerPawn->Controller, FALSE, &ActivateIndices );
			}
		}
	}
	if( !_pTask->m_bHidden && ( Objective == NULL || !Objective->m_bIsHidden ) )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD && _pTask->m_TaskState == DTS_Completed )
		{
			HUD->OnTaskCompleted( _pTask );
		}
		else if( HUD && _pTask->m_TaskState == DTS_Failed )
		{
			HUD->OnTaskFailed( _pTask );
		}
	}
	if( _pTask->m_TaskState == DTS_Completed && Objective )
	{
		Objective->OnTaskCompletedNotify( _pTask );
	}
}

void DishonoredLinkDishonoredObjectivesComponent()
{
}
