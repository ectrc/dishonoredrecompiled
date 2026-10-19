// DishonoredGame/src/stateplayeraction.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x6afbc0  public: static void __cdecl UDisNativeStateMachine_PlayerAction::InitializePrivateStaticClassUDisNativeStateMachine_PlayerAction(void)
//   0x6afbe0  public: static void __cdecl UStatePlayerUpperBase::InitializePrivateStaticClassUStatePlayerUpperBase(void)
//   0x6afc00  public: static void __cdecl UStatePlayerAction::InitializePrivateStaticClassUStatePlayerAction(void)
//   0x6afc20  public: virtual void __thiscall UStatePlayerAction::OnEnterState(class UDishonoredNativeState *)
//   0x6b4560  public: virtual void __thiscall FStatePlayerUpperBase_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6b4580  public: virtual void __thiscall FStatePlayerAction_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6b9250  public: static class UClass * __cdecl UStatePlayerUpperBase::GetPrivateStaticClassUStatePlayerUpperBase(wchar_t const *)
//   0x6b92e0  public: virtual unsigned int __thiscall UStatePlayerAction::DoAction(void)
//   0x6bee90  public: static class UClass * __cdecl UStatePlayerUpperBase::StaticClassNoInline(void)
//   0x6beec0  public: static class UClass * __cdecl UStatePlayerAction::GetPrivateStaticClassUStatePlayerAction(wchar_t const *)
//   0x6c2210  public: static class UClass * __cdecl UStatePlayerAction::StaticClassNoInline(void)
//   0x6c4330  public: __thiscall FStatePlayerAction_Param::FStatePlayerAction_Param(struct FPawnAction const &)
//   0x6c4370  protected: virtual void __thiscall UStatePlayerAction::RequestStateExit_Derived(void)
//   0x6cec20  public: static class UClass * __cdecl UDisNativeStateMachine_PlayerAction::GetPrivateStaticClassUDisNativeStateMachine_PlayerAction(wchar_t const *)
//   0x6cfbf0  public: static class UClass * __cdecl UDisNativeStateMachine_PlayerAction::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the upper-body / left-arm action state (Inc/CppText/UStatePlayerAction.h), 2013 bodies.
	DISHONORED(written): retail reads the bound hand's equipped item without a NULL test (the player always holds at least the empty
	hand); the item is tested here.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "imdiag.h"

void DishonoredLinkIM_StatePlayerAction()
{
}

// 2012 0x6b4580 (33 B): the action base's, then the player pawn and the action machine
void FStatePlayerAction_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	// DISHONORED(written): a machine without the requested state has no pending state to tell (retail writes through it)
	if( PendingState == NULL )
	{
		return;
	}
	FStateSharedActionBase_PlayerOnly_Param::OnPending( PendingState, ManagedObject );
	UStatePlayerAction* State = (UStatePlayerAction*)PendingState;
	State->m_pPlayerPawn = (ADishonoredPlayerPawn*)ManagedObject;
	State->m_pActionFSM = (UDisNativeStateMachine_PlayerAction*)State->GetStateMachine();
}

// 2012 0x6afc20 (9 B)
void UStatePlayerAction::OnEnterState( UDishonoredNativeState* LastState )
{
	UStateSharedActionBase::OnEnterState( LastState );
}

/**
 * 2013 0x66fc60 (260 B). The chosen anim state goes to the machine's picker (attached to this state, fired from the firing object);
 * its blend time becomes the state's when none was handed over and the hand's item follows; an action without a state, or one the
 * pawn cannot set, leaves at once. Unless the master's logical state is a walk-type state that keeps the arm (m_bAddYOffsetBasedOnPitch,
 * +68 bit 3), the hand's arm follow is re-enabled.
 */
UBOOL UStatePlayerAction::DoAction()
{
	UStateSharedActionBase_PlayerOnly::DoAction();
	const FName AnimState = ChooseAnimState();
	UDisNativeStateMachine_PlayerAction* ActionFSM = m_pActionFSM;
	FDisPawnAnimState* State = AnimState != NAME_None ? m_pPlayerPawn->SetAnimState( AnimState, ActionFSM->m_BoundAnimStatePicker, this, m_fTimeToGo, (UObject*)m_pFiringObj ) : NULL;
	if( State )
	{
		if( m_fTimeToGo < 0.f )
		{
			m_fTimeToGo = State->m_fBlendTime;
		}
		UDishonoredInventoryItem* Item = m_pPawn->GetEquippedItem( ActionFSM->m_BoundToItemUsage );
		if( Item )
		{
			Item->SyncPawnAnimState( AnimState, this, m_fTimeToGo, (UObject*)m_pFiringObj );
		}
	}
	else
	{
		if( AnimState != NAME_None )
		{
			m_fTimeToGo = 0.f;
		}
		RequestStateExit_WithTime( 0.f );
	}
	UStatePlayerMasterBase* Master = Cast<UStatePlayerMasterBase>( m_pPlayerPawn->m_pPlayerMasterFSM->GetLogicalState() );
	if( Master == NULL || Master->m_bAddYOffsetBasedOnPitch )
	{
		m_pPlayerPawn->DisableArmFollow( ActionFSM->m_BoundToItemUsage, FALSE, -1.f );
	}
	return TRUE;
}

// 2013 0x677290 (132 B): back to the idle state, or to the navigation state when the machine uses one and is not locked
void UStatePlayerAction::RequestStateExit_Derived()
{
	if( !m_pActionFSM->m_bUseNavigationState || m_pActionFSM->m_bIsLocked )
	{
		FStatePlayerUpperIdle_Param IdleParam;
		DemandStateChange( IdleParam );
	}
	else
	{
		FStatePlayerActionNav_Param NavParam;
		DemandStateChange( NavParam );
	}
}
