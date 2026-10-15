// DishonoredGame/src/disseqact_objectiveaction.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7dd5d0  public: void __thiscall UDisSeqAct_ObjectiveAction::GetObjectives(struct TMemStackArray<class UDishonoredObjective *> &)const
//   0x7ec390  public: static class UClass * __cdecl UDisSeqAct_ObjectiveAction::GetPrivateStaticClassUDisSeqAct_ObjectiveAction(wchar_t const *)
//   0x7ee800  public: static void __cdecl UDisSeqAct_ObjectiveAction::InitializePrivateStaticClassUDisSeqAct_ObjectiveAction(void)
//   0x7ef510  public: static class UClass * __cdecl UDisSeqAct_ObjectiveAction::StaticClassNoInline(void)

// ---- agent HI (PHASE22 HI) ----
#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7a81d0 (2012 0x7dd5d0): every object variable linked to the action that holds an objective.
void UDisSeqAct_ObjectiveAction::GetObjectives( TArray<UDishonoredObjective*, TMemStackAllocator<GMainThreadMemStack> >& _rObjectives ) const
{
	TArray<UObject**> LinkedObjects;
	GetObjectVars( LinkedObjects );
	for( INT Index = 0; Index < LinkedObjects.Num(); Index++ )
	{
		UObject** Ref = LinkedObjects(Index);
		if( Ref && *Ref && (*Ref)->IsA( UDishonoredObjective::StaticClass() ) )
		{
			_rObjectives.AddItem( (UDishonoredObjective*)*Ref );
		}
	}
}

void DishonoredLinkDisSeqActObjectiveAction()
{
}
