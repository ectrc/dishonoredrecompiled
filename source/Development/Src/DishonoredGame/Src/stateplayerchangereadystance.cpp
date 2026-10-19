// DishonoredGame/src/stateplayerchangereadystance.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x6afc30  public: virtual void __thiscall UStatePlayerChangeReadyStance::OnEnterState(class UDishonoredNativeState *)
//   0x6afc50  public: virtual void __thiscall UStatePlayerChangeReadyStance::OnExitState(class UDishonoredNativeState *)
//   0x6afc90  public: virtual void __thiscall UStatePlayerChangeReadyStance::TickState(float)
//   0x6b45b0  public: virtual void __thiscall FStatePlayerChangeReadyStance_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6b93d0  public: virtual unsigned int __thiscall UStatePlayerChangeReadyStance::DoAction(void)
//   0x6befe0  public: static class UClass * __cdecl UStatePlayerChangeReadyStance::GetPrivateStaticClassUStatePlayerChangeReadyStance(wchar_t const *)
//   0x6c2240  public: static void __cdecl UStatePlayerChangeReadyStance::InitializePrivateStaticClassUStatePlayerChangeReadyStance(void)
//   0x6c4400  public: static class UClass * __cdecl UStatePlayerChangeReadyStance::StaticClassNoInline(void)
//   0x6c4430  public: virtual void __thiscall UStatePlayerChangeReadyStance::RequestStateExit_Derived(void)
//   0x6c6190  public: __thiscall FStatePlayerChangeReadyStance_Param::FStatePlayerChangeReadyStance_Param(struct FPawnAction const &, enum eDisPlayerStance)
//   0x6c61f0  public: virtual unsigned int __thiscall UStatePlayerChangeReadyStance::CanTransitionExternal(class UClass const *)const

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the ready-stance change (Inc/CppText/UStatePlayerChangeReadyStance.h). m_bCheckBlend is raised by a
	successful DoAction and lowered when the stance is stored. 2013 bodies (2012's OnExitState / TickState also called the empty base).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

void DishonoredLinkIM_StatePlayerChangeReadyStance()
{
}

// 2012 0x6c6190 / 2013 0x673d40
FStatePlayerChangeReadyStance_Param::FStatePlayerChangeReadyStance_Param( const FPawnAction& Action, BYTE GoToStance )
	: FStatePlayerAction_Param( Action )
	, m_GoToStance( GoToStance )
{
	m_pStateClass = UStatePlayerChangeReadyStance::StaticClass();
}

// 2012 0x6b45b0 (43 B)
void FStatePlayerChangeReadyStance_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	// DISHONORED(written): a machine without the requested state has no pending state to tell (retail writes through it)
	if( PendingState == NULL )
	{
		return;
	}
	FStatePlayerAction_Param::OnPending( PendingState, ManagedObject );
	((UStatePlayerChangeReadyStance*)PendingState)->m_GoToStance = m_GoToStance;
}

static void DisStoreGoToStance( UStatePlayerChangeReadyStance* State )
{
	if( State->m_bCheckBlend )
	{
		State->m_pPlayerPawn->SetPlayerStance( State->m_pActionFSM->m_BoundToItemUsage, State->m_GoToStance );
		State->m_bCheckBlend = FALSE;
	}
}

// 2012 0x6afc30 (24 B)
void UStatePlayerChangeReadyStance::OnEnterState( UDishonoredNativeState* LastState )
{
	UStateSharedActionBase::OnEnterState( LastState );
	m_bCheckBlend = TRUE;
}

// 2013 0x660c70 (45 B)
void UStatePlayerChangeReadyStance::OnExitState( UDishonoredNativeState* NextState )
{
	m_bActionIsExiting = FALSE;
	DisStoreGoToStance( this );
}

// 2013 0x660ca0 (55 B)
void UStatePlayerChangeReadyStance::TickState( FLOAT DeltaSeconds )
{
	if( m_bCheckBlend && m_pPlayerPawn->IsCurAnimStateFullWeight( ANIMSTATE_UPPER_BODY ) )
	{
		DisStoreGoToStance( this );
	}
}

// 2012 0x6b93d0 / 2013 0x66fdd0 (18 B)
UBOOL UStatePlayerChangeReadyStance::DoAction()
{
	const UBOOL bResult = UStatePlayerAction::DoAction();
	if( bResult )
	{
		m_bCheckBlend = TRUE;
	}
	return bResult;
}

// 2012 0x6c4430 / 2013 0x6773f0 (45 B)
void UStatePlayerChangeReadyStance::RequestStateExit_Derived()
{
	DisStoreGoToStance( this );
	UStatePlayerAction::RequestStateExit_Derived();
}

// 2012 0x6c61f0 / 2013 0x673da0 (47 B): an equip change does not interrupt it
UBOOL UStatePlayerChangeReadyStance::CanTransitionExternal( const UClass* StateID ) const
{
	return UStateSharedActionBase_PlayerOnly::CanTransitionExternal( StateID ) && StateID != UStatePlayerEquipChange::StaticClass();
}
