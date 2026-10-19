// DishonoredGame/src/statesharedactionbase_playeronly.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (18):
//   0x692a50  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::DoAction(void)
//   0x692a60  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::OnResetState(void)
//   0x692a70  public: virtual void __thiscall UStateSharedActionBase_PlayerOnly::OnExitState(class UDishonoredNativeState *)
//   0x692a80  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::CanTransitionAction(struct FPawnAction const *)const
//   0x692ac0  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::AllowUsageToInterrupt(class UDisItemContext const *)const
//   0x692ae0  public: unsigned int __thiscall UStateSharedActionBase_PlayerOnly::AllowParryToInterrupt(void)const
//   0x692b00  public: void __thiscall UStateSharedActionBase_PlayerOnly::SetAllowInterrupt_ByUse(void)
//   0x692b10  public: void __thiscall UStateSharedActionBase_PlayerOnly::SetAllowInterrupt_ByParry(void)
//   0x692b20  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::NeedsFootPlacement(void)const
//   0x692b40  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::ShouldLimitControllerSensitivity(unsigned int, float &)const
//   0x695a20  protected: __thiscall FStateSharedActionBase_PlayerOnly_Param::FStateSharedActionBase_PlayerOnly_Param(struct FPawnAction const &)
//   0x695a40  public: virtual void __thiscall FStateSharedActionBase_PlayerOnly_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x69a430  protected: virtual class FName __thiscall UStateSharedActionBase_PlayerOnly::ChooseAnimState(void)
//   0x69ded0  public: static class UClass * __cdecl UStateSharedActionBase_PlayerOnly::GetPrivateStaticClassUStateSharedActionBase_PlayerOnly(wchar_t const *)
//   0x6a04c0  public: static void __cdecl UStateSharedActionBase_PlayerOnly::InitializePrivateStaticClassUStateSharedActionBase_PlayerOnly(void)
//   0x6a1e40  public: static class UClass * __cdecl UStateSharedActionBase_PlayerOnly::StaticClassNoInline(void)
//   0x6a3d10  public: virtual unsigned int __thiscall UStateSharedActionBase_PlayerOnly::CanTransitionExternal(class UClass const *)const
//   0x6a5d20  protected: virtual void __thiscall UStateSharedActionBase_PlayerOnly::RequestStateExit_Derived(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the player's action base (Inc/CppText/UStateSharedActionBase_PlayerOnly.h); 2012 and 2013 bodies are
	byte-identical except ChooseAnimState's assert strings. The interrupt helpers (AllowUsageToInterrupt, AllowParryToInterrupt,
	NeedsFootPlacement, ShouldLimitControllerSensitivity) have no caller in this tree and are not written.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

void DishonoredLinkIM_StateSharedActionBasePlayerOnly()
{
}

// 2012 0x695a40 (39 B)
void FStateSharedActionBase_PlayerOnly_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	// DISHONORED(written): a machine without the requested state has no pending state to tell (retail writes through it)
	if( PendingState == NULL )
	{
		return;
	}
	UStateSharedActionBase_PlayerOnly* State = (UStateSharedActionBase_PlayerOnly*)PendingState;
	State->m_pPawn = (ADishonoredPawn*)ManagedObject;
	if( State->m_pCurAction != (FPointer)m_pCurAction )
	{
		State->m_NonRepeatINTRandomHelper.m_iLastReturnedValue = -1;
	}
	State->m_pCurAction = (FPointer)m_pCurAction;
}

// 2012 0x692a50 / 2013 0x65fa80: a new action clears both interrupt overrides
UBOOL UStateSharedActionBase_PlayerOnly::DoAction()
{
	m_bOverrideInterrupt_ByUse = FALSE;
	m_bOverrideInterrupt_ByParry = FALSE;
	return TRUE;
}

// 2012 0x692a60 / 2013 0x65fa90
UBOOL UStateSharedActionBase_PlayerOnly::OnResetState()
{
	m_bActionIsExiting = FALSE;
	return DoAction();
}

// 2012 0x692a70 / 2013 0x65faa0
void UStateSharedActionBase_PlayerOnly::OnExitState( UDishonoredNativeState* NextState )
{
	m_bActionIsExiting = FALSE;
}

// 2012 0x692a80 / 2013 0x65fab0 (56 B): an aesthetic action never interrupts a real one; the same action only when it allows itself
UBOOL UStateSharedActionBase_PlayerOnly::CanTransitionAction( const FPawnAction* Action ) const
{
	const FPawnAction* CurAction = GetCurAction();
	if( CurAction == NULL )
	{
		return TRUE;
	}
	if( !CurAction->m_bAesthetic && Action->m_bAesthetic )
	{
		return FALSE;
	}
	if( !CurAction->m_bAllowSelfInterrupt && !m_bActionIsExiting && CurAction == Action )
	{
		return FALSE;
	}
	return TRUE;
}

// 2012 0x6a3d10 / 2013 0x66dfc0 (91 B): another player action may not interrupt a running no-interrupt action unless an override allows it
UBOOL UStateSharedActionBase_PlayerOnly::CanTransitionExternal( const UClass* StateID ) const
{
	if( StateID == NULL || !StateID->IsChildOf( UStateSharedActionBase_PlayerOnly::StaticClass() ) )
	{
		return TRUE;
	}
	const FPawnAction* CurAction = GetCurAction();
	return !( CurAction && !m_bActionIsExiting && CurAction->m_bNoInterrupt && !m_bOverrideInterrupt_ByUse && !m_bOverrideInterrupt_ByParry );
}

// 2012 0x6a5d20 / 2013 0x675290 (56 B): the base exit (the machine's default state unless another one is pending) marked as exiting
void UStateSharedActionBase_PlayerOnly::RequestStateExit_Derived()
{
	m_bActionIsExiting = TRUE;
	UDishonoredNativeState::RequestStateExit_Derived();
}

// 2012 0x69a430 / 2013 0x666f70 (127 B): one of the action's anim states, at random without repeating the last
FName UStateSharedActionBase_PlayerOnly::ChooseAnimState()
{
	const FPawnAction* CurAction = GetCurAction();
	if( CurAction == NULL )
	{
		return NAME_None;
	}
	const INT NumStates = CurAction->m_AnimStates.Num();
	m_iCurrentAnimStateIndex = 0;
	if( NumStates > 1 )
	{
		m_iCurrentAnimStateIndex = m_NonRepeatINTRandomHelper.GetNonRepeatingRandomValue( NumStates );
	}
	// DISHONORED(written): retail's assert lets index 0 of an empty array through and reads past it; an action without anim states
	// answers NAME_None here (the action then clears / leaves as for an unnamed state)
	return NumStates > 0 ? CurAction->m_AnimStates( m_iCurrentAnimStateIndex ) : FName( NAME_None );
}
