// DishonoredGame/src/disseqact_taskaction.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7dd710  public: virtual void __thiscall UDisSeqAct_TaskAction::Activated(void)
//   0x7ec660  public: static class UClass * __cdecl UDisSeqAct_TaskAction::GetPrivateStaticClassUDisSeqAct_TaskAction(wchar_t const *)
//   0x7ee820  public: static void __cdecl UDisSeqAct_TaskAction::InitializePrivateStaticClassUDisSeqAct_TaskAction(void)
//   0x7ef580  public: static class UClass * __cdecl UDisSeqAct_TaskAction::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x7a8300 (2012 0x7dd710; 2013 writes the HUD's m_bHandleObjectiveEvents inline where 2012 called
// HandleObjectiveEvents). The package tasks on the "Task" variables resolve to the player's task instances; with
// m_bOnlyOnTasksInActiveObjectives only those of an active objective are handled.
// DISHONORED(bringup): this tree's DisGetGFxHUD answers NULL while the HUD movie is closed; the flag is then left alone.
void UDisSeqAct_TaskAction::Activated()
{
	FMemMark Mark( GMainThreadMemStack );
	TArray<const UDishonoredTask_Base*, TMemStackAllocator<GMainThreadMemStack> > Tasks;
	DisGetTaskVars( this, TEXT("Task"), Tasks );
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	if( PlayerController && PlayerController->m_pObjectivesComponent )
	{
		UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
		if( HUD )
		{
			HUD->HandleObjectiveEvents( m_bShowHUDNotification );
		}
		for( INT TaskIdx = 0; TaskIdx < Tasks.Num(); TaskIdx++ )
		{
			UDishonoredTask_Base* Task = PlayerController->m_pObjectivesComponent->GetTaskInstance( Tasks(TaskIdx) );
			if( UDishonoredObjectivesComponent::IsDiagEnabled() )
			{
				debugf( TEXT("DISHONORED(bringup): hidiag %s task %s -> %s"), *GetName(), *Tasks(TaskIdx)->GetPathName(), Task ? *Task->GetName() : TEXT("no instance") );
			}
			if( Task == NULL )
			{
				continue;
			}
			const UDishonoredObjective* Objective = Cast<UDishonoredObjective>( Task->GetOuter() );
			if( !m_bOnlyOnTasksInActiveObjectives || Objective == NULL || Objective->IsActive() )
			{
				HandleTaskInstance( Task );
			}
		}
		if( HUD )
		{
			HUD->HandleObjectiveEvents( TRUE );
		}
	}
	USequenceAction::Activated();
}

void DishonoredLinkDisSeqActTaskAction()
{
}
