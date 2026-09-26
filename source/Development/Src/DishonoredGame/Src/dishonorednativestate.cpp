// DishonoredGame/src/dishonorednativestate.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x692790  public: static void __cdecl UDishonoredNativeState::InitializePrivateStaticClassUDishonoredNativeState(void)
//   0x695920  public: void __thiscall UDishonoredNativeState::RequestStateExit(void)
//   0x69a1b0  public: static class UClass * __cdecl UDishonoredNativeState::GetPrivateStaticClassUDishonoredNativeState(wchar_t const *)
//   0x69de80  public: static class UClass * __cdecl UDishonoredNativeState::StaticClassNoInline(void)
//   0x6a5940  protected: void __thiscall UDishonoredNativeState::DemandStateChange(struct FDisNativeStateParam &)
//   0x6a5970  protected: virtual void __thiscall UDishonoredNativeState::RequestStateExit_Derived(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x662750 (2012 0x695920): a state may leave when it is the active one, or while pending when
// m_bAllowRequestExitWhenPending is set; the machine must not be destroyed
void UDishonoredNativeState::RequestStateExit()
{
	UDishonoredNativeStateMachine* StateMachine = m_pStateMachine;
	if( !StateMachine || StateMachine->m_bDestroyFSMCalled )
	{
		return;
	}
	if( m_bAllowRequestExitWhenPending || StateMachine->GetCurrentlyActiveState() == this )
	{
		RequestStateExit_Derived();
	}
}

// DISHONORED(written): 2013 rva 0x674ee0 (2012 0x6a5970, same bytes): the default exit demands the machine's default state
// (m_DefaultStateParam, the bytes InitFSM stored) unless another state is already pending
void UDishonoredNativeState::RequestStateExit_Derived()
{
	UDishonoredNativeStateMachine* StateMachine = m_pStateMachine;
	if( !StateMachine || StateMachine->m_bDestroyFSMCalled )
	{
		return;
	}
	if( StateMachine->m_pPendingState && StateMachine->m_pPendingState != this )
	{
		return;
	}
	if( StateMachine->m_DefaultStateParam.Num() )
	{
		StateMachine->DemandStateChange( this, *(FDisNativeStateParam*)StateMachine->m_DefaultStateParam.GetData() );
	}
}

// DISHONORED(written): 2013 rva 0x674eb0 (2012 0x6a5940, same bytes)
void UDishonoredNativeState::DemandStateChange( FDisNativeStateParam& Param )
{
	if( m_pStateMachine && !m_pStateMachine->m_bDestroyFSMCalled )
	{
		m_pStateMachine->DemandStateChange( this, Param );
	}
}
