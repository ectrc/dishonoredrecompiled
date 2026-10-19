// DishonoredGame/src/stateplayerupperidle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x6b98f0  public: static class UClass * __cdecl UStatePlayerUpperIdle::GetPrivateStaticClassUStatePlayerUpperIdle(wchar_t const *)
//   0x6bf3a0  public: static void __cdecl UStatePlayerUpperIdle::InitializePrivateStaticClassUStatePlayerUpperIdle(void)
//   0x6c23b0  public: static class UClass * __cdecl UStatePlayerUpperIdle::StaticClassNoInline(void)
//   0x6c45a0  protected: virtual void __thiscall UStatePlayerUpperIdle::RequestStateExit_Derived(void)
//   0x6c6550  public: virtual void __thiscall UStatePlayerUpperIdle::OnEnterState(class UDishonoredNativeState *)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the upper-body / left-arm machines' default state (Inc/CppText/UStatePlayerUpperIdle.h), 2013 bodies,
	and the upper params' OnPending.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

void DishonoredLinkIM_StatePlayerUpperIdle()
{
}

// 2012 0x6b4560 / 2013 0x664230 (22 B)
void FStatePlayerUpperBase_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	// DISHONORED(written): a machine without the requested state has no pending state to tell (retail writes through it)
	if( PendingState == NULL )
	{
		return;
	}
	UStatePlayerUpperBase* State = (UStatePlayerUpperBase*)PendingState;
	State->m_pActionFSM = (UDisNativeStateMachine_PlayerAction*)State->GetStateMachine();
	State->m_pPawn = (ADishonoredPlayerPawn*)ManagedObject;
}

// 2013 0x6741a0 (170 B): the picker is cleared over the last action's blend time (0.2 s otherwise), the item's state too; a machine
// with a navigation state shows its hand's item
void UStatePlayerUpperIdle::OnEnterState( UDishonoredNativeState* LastState )
{
	FLOAT BlendTime = 0.2f;
	UStatePlayerAction* LastAction = Cast<UStatePlayerAction>( LastState );
	if( LastAction )
	{
		BlendTime = LastAction->m_fTimeToGo;
	}
	m_pPawn->ClearAnimState( m_pActionFSM->m_BoundAnimStatePicker, BlendTime );
	UDishonoredInventoryItem* Item = m_pPawn->GetEquippedItem( m_pActionFSM->m_BoundToItemUsage );
	if( Item )
	{
		Item->SyncPawnAnimState( NAME_None, NULL, BlendTime, NULL );
	}
	if( m_pActionFSM->m_bUseNavigationState )
	{
		m_pPawn->ShowEquippedInventoryItem( FALSE, m_pActionFSM->m_BoundToItemUsage );
	}
}

// 2013 0x677ad0 (144 B): a machine with a navigation state brings its item in; otherwise a state that is not the idle one goes to it
void UStatePlayerUpperIdle::RequestStateExit_Derived()
{
	if( m_pActionFSM->m_bUseNavigationState )
	{
		m_pPawn->TransitionItemIn( m_pActionFSM->m_BoundToItemUsage, m_pActionFSM->m_ActionUsage, TRUE, FALSE );
	}
	else if( GetClass() != UStatePlayerUpperIdle::StaticClass() )
	{
		FStatePlayerUpperIdle_Param IdleParam;
		DemandStateChange( IdleParam );
	}
}
