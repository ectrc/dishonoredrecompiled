// DishonoredGame/src/disseqact_updatetasktarget.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7dd8d0  public: virtual void __thiscall UDisSeqAct_UpdateTaskTarget::HandleTaskInstance(class UDishonoredTask_Base *)
//   0x7ec780  public: static class UClass * __cdecl UDisSeqAct_UpdateTaskTarget::GetPrivateStaticClassUDisSeqAct_UpdateTaskTarget(wchar_t const *)
//   0x7ef5d0  public: static void __cdecl UDisSeqAct_UpdateTaskTarget::InitializePrivateStaticClassUDisSeqAct_UpdateTaskTarget(void)
//   0x7f0880  public: static class UClass * __cdecl UDisSeqAct_UpdateTaskTarget::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7a84d0 (2012 0x7dd8d0; 2013's target carries the localisation file and section of the action's
// m_TargetName instead of the 2012 display string). Input 0 sets target m_TargetIndex (growing the list), input 1 clears it
// (trimming empty targets off the end), input 2 toggles it.
void UDisSeqAct_UpdateTaskTarget::HandleTaskInstance( UDishonoredTask_Base* _pTask )
{
	const UBOOL bSet = InputLinks(0).bHasImpulse;
	const UBOOL bClear = InputLinks(1).bHasImpulse;
	const UBOOL bToggle = InputLinks(2).bHasImpulse;
	TArray<FDisTaskTarget>& Targets = _pTask->GetTargets();
	const UBOOL bHasTarget = Targets.IsValidIndex( m_TargetIndex ) && Targets(m_TargetIndex).m_pActor != NULL;
	if( bSet || ( bToggle && !bHasTarget ) )
	{
		while( Targets.Num() < m_TargetIndex + 1 )
		{
			Targets.AddItem( FDisTaskTarget( EC_EventParm ) );
		}
		FDisTaskTarget& Target = Targets(m_TargetIndex);
		Target.m_pActor = m_pTargetActor;
		FString LocPrefix;
		GetLocalizationDataLocation( this, Target.m_TargetNameLocFile, Target.m_TargetNameLocSection, LocPrefix );
		Target.m_VanishPreset = m_MarkerVanishPreset;
		Target.m_bOptional = _pTask->m_bOptional;
		if( UDishonoredObjectivesComponent::IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag task %s target %d = %s (loc %s / %s, vanish %d, optional %d)"), *_pTask->GetName(), m_TargetIndex,
				m_pTargetActor ? *m_pTargetActor->GetName() : TEXT("None"), *Target.m_TargetNameLocFile, *Target.m_TargetNameLocSection,
				(INT)Target.m_VanishPreset, (INT)Target.m_bOptional );
		}
	}
	else if( bClear || ( bToggle && bHasTarget ) )
	{
		if( !bHasTarget )
		{
			return;
		}
		Targets(m_TargetIndex) = FDisTaskTarget( EC_EventParm );
		if( m_TargetIndex == Targets.Num() - 1 )
		{
			for( INT Index = Targets.Num() - 1; Index >= 0 && Targets(Index).m_pActor == NULL; Index-- )
			{
				Targets.Remove( Index );
			}
		}
		if( UDishonoredObjectivesComponent::IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag task %s target %d cleared, %d left"), *_pTask->GetName(), m_TargetIndex, Targets.Num() );
		}
	}
}

void DishonoredLinkDisSeqActUpdateTaskTarget()
{
}
