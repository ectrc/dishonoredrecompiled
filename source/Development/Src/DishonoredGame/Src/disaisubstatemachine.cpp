// DishonoredGame/src/disaisubstatemachine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21):
//   0x657700  public: static void __cdecl UDisAISubStateMachine::InitializePrivateStaticClassUDisAISubStateMachine(void)
//   0x765720  public: void __thiscall UDisAISubStateMachine::RefreshAIFSM(float)
//   0x765760  private: virtual void __thiscall UDisAISubStateMachine::PostDestroyFSM_Derived(void)
//   0x765780  private: virtual unsigned int __thiscall UDisAISubStateMachine::PendingStateIsCurrentState_Derived(void)const
//   0x7657a0  private: virtual void __thiscall UDisAISubStateMachine::ClearPendingState_Derived(void)
//   0x7657c0  private: int __thiscall UDisAISubStateMachine::GetLogicalSubStateIndex(void)const
//   0x7657e0  private: void __thiscall UDisAISubStateMachine::SetPendingSubStateTweaks(class UDisTweaks_AISubState const &)
//   0x7657f0  private: unsigned int __thiscall UDisAISubStateMachine::RequestSafeAIStateChange(struct FDisNativeStateParam &, int, class UDisTweaks_AISubState const &)
//   0x679ac0  private: int __thiscall UDisAISubStateMachine::GetPendingSubStateIndex(void)const
//   0x769300  public: void __thiscall UDisAISubStateMachine::TickAIFSM(float)
//   0x769320  private: virtual void __thiscall UDisAISubStateMachine::PostStateChange_Derived(void)
//   0x7693a0  private: class UDisAISubState * __thiscall UDisAISubStateMachine::GetCurrentAISubState(void)const
//   0x771090  private: class UDisAISubState * __thiscall UDisAISubStateMachine::GetSubState(class UClass * const)const
//   0x775a90  public: unsigned int __thiscall UDisAISubStateMachine::FilterAIFSM(struct FAIStimStruct const &)
//   0x77bce0  public: void __thiscall UDisAISubStateMachine::OnOwningBehaviorResume(struct FDisBodyIntention const &)
//   0x77bd30  public: void __thiscall UDisAISubStateMachine::OnOwningBehaviorPause(unsigned int)
//   0x77bd80  public: void __thiscall UDisAISubStateMachine::OnOwningBehaviorStop(unsigned int)
//   0x7881b0  public: static class UClass * __cdecl UDisAISubStateMachine::GetPrivateStaticClassUDisAISubStateMachine(wchar_t const *)
//   0x78b960  public: static class UClass * __cdecl UDisAISubStateMachine::StaticClassNoInline(void)
//   0x9f3760  private: int __thiscall UDisAISubStateMachine::GetActiveSubStateIndex(void)const

// ---- agent CG ports (PHASE9 CG): the AI sub-state machine ----

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "disaisubstate.h"

// DISHONORED(port): 2013 rva 0x72a1e0 (2012 0x769300): a machine with no states was never InitFSM'd, and ticking it would
// walk a NULL current state.
void UDisAISubStateMachine::TickAIFSM( FLOAT DeltaSeconds )
{
	if( m_NativeStates.Num() > 0 )
	{
		TickStateMachine( DeltaSeconds );
	}
}

// DISHONORED(port): 2013 rva 0x727b20 (2012 0x765720): the thinking half. A state change that is already pending
// suppresses the refresh, so a sub-state that has just asked to leave is not asked to think again on the way out.
void UDisAISubStateMachine::RefreshAIFSM( FLOAT TimeSinceLastThought )
{
	if( m_pCurrentAISubState && !GetPendingStateID() )
	{
		m_pCurrentAISubState->RefreshSubState( TimeSinceLastThought );
	}
}

// DISHONORED(port): 2013 rva 0x738720 (2012 0x775a90): a stim only reaches the sub-state when it is the state the machine
// is logically in (not one it is transitioning out of), the state's own filter mask has the bit for that stim id, and the
// state hands out a filter delegate for it. Returning TRUE consumes the stim: the behaviour stack never sees it.
UBOOL UDisAISubStateMachine::FilterAIFSM( const FAIStimStruct& AIStim )
{
	if( !m_pCurrentAISubState )
	{
		return FALSE;
	}
	if( GetLogicalState() != m_pCurrentAISubState )
	{
		return FALSE;
	}
	const BYTE* FilterStimMask = (const BYTE*)m_pCurrentAISubState->m_pFilterStimMask;
	if( !FilterStimMask || ( FilterStimMask[ AIStim.m_StimID ] & 1 ) == 0 )
	{
		return FALSE;
	}
	FDisStimPredicateDelegate FilterDelegate = m_pCurrentAISubState->GetFilterStimDelegate_SubState( AIStim.m_StimID );
	return FilterDelegate( AIStim );
}

// DISHONORED(port): 2013 rva 0x72e310 (2012 0x77bce0): the idle state is exempt - UDisAISubStateInit has nothing to
// resume, and resuming it would make the NPC look like it had an action in progress.
void UDisAISubStateMachine::OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention )
{
	if( m_pCurrentAISubState && !m_pCurrentAISubState->IsA( UDisAISubStateInit::StaticClass() ) )
	{
		m_pCurrentAISubState->OnOwningBehaviorResume( PreviousBodyIntention );
	}
}

// DISHONORED(port): 2013 rva 0x72e360 (2012 0x77bd30)
void UDisAISubStateMachine::OnOwningBehaviorPause( UBOOL bIsBeingTerminated )
{
	if( m_pCurrentAISubState && !m_pCurrentAISubState->IsA( UDisAISubStateInit::StaticClass() ) )
	{
		m_pCurrentAISubState->OnOwningBehaviorPause( bIsBeingTerminated );
	}
}

// DISHONORED(port): 2013 rva 0x72e3b0 (2012 0x77bd80): a behaviour that is going away for good destroys its machine; one
// that is merely stopping is driven back to the idle state immediately. The immediate drive is why the reset flag is
// raised around the TickStateMachine call: that tick performs the pending change in place, and
// PendingStateIsCurrentState_Derived must not be allowed to short-circuit it.
void UDisAISubStateMachine::OnOwningBehaviorStop( UBOOL bIsBeingTerminated )
{
	if( bIsBeingTerminated )
	{
		DestroyFSM();
		return;
	}

	FDisAISubState_Param AISubStateParam( UDisAISubStateInit::StaticClass() );
	RequestStateChange( AISubStateParam, NULL, FALSE );

	m_bIsInTheProcessOfResetting = TRUE;
	TickStateMachine( 0.f );
	m_bIsInTheProcessOfResetting = FALSE;
}

// DISHONORED(port): 2013 rva 0x72e430 (2012 0x771090): an exact class match, not IsA - a behaviour addresses its states
// by their own class.
UDisAISubState* UDisAISubStateMachine::GetSubState( UClass* const SubStateClass ) const
{
	for( INT i = 0; i < m_NativeStates.Num(); ++i )
	{
		UDisAISubState* SubState = (UDisAISubState*)m_NativeStates(i);
		if( SubState->GetClass() == SubStateClass )
		{
			return SubState;
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x727bc0 (2012 0x7657c0): the slot the machine is logically in, i.e. the pending one when
// there is one.
INT UDisAISubStateMachine::GetLogicalSubStateIndex() const
{
	return m_PendingAISubStateIndex == INDEX_NONE ? m_CurrentAISubStateIndex : m_PendingAISubStateIndex;
}

// DISHONORED(port): 2013 rva 0x727c00 (2012 0x7657f0): "safe" means the request is refused outright while the machine is
// resetting, and the slot and tweaks are only recorded once the base machine has accepted the change - so a refused
// request leaves no half-set pending state behind.
UBOOL UDisAISubStateMachine::RequestSafeAIStateChange( FDisNativeStateParam& Params, INT SubStateIndex, UDisTweaks_AISubState* SubStateTweaks )
{
	if( m_bIsInTheProcessOfResetting )
	{
		return FALSE;
	}
	if( !RequestStateChange( Params, NULL, FALSE ) )
	{
		return FALSE;
	}
	m_PendingAISubStateIndex = SubStateIndex;
	m_pPendingAISubStateTweaks = SubStateTweaks;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x727bf0 (2012 0x7657e0)
void UDisAISubStateMachine::SetPendingSubStateTweaks( UDisTweaks_AISubState* SubStateTweaks )
{
	m_pPendingAISubStateTweaks = SubStateTweaks;
}

// DISHONORED(port): 2013 rva 0x72a200 (2012 0x769320): the change has landed. The pending tweaks are applied to the state
// that just became current - this is the moment a sub-state gets the designer's settings for the slot it was entered in,
// so the same sub-state class entered from two slots behaves differently. A change with no pending tweaks is the
// machine's own idle transition and leaves both indices cleared.
void UDisAISubStateMachine::PostStateChange_Derived()
{
	UDisTweaks_AISubState* PendingTweaks = m_pPendingAISubStateTweaks;
	UDisAISubState* CurrentSubState = (UDisAISubState*)m_pCurrentState;
	m_pCurrentAISubState = CurrentSubState;

	if( PendingTweaks )
	{
		m_CurrentAISubStateIndex = m_PendingAISubStateIndex;
		if( CurrentSubState->GetTweaks_Derived() != PendingTweaks )
		{
			CurrentSubState->SetTweaks_Derived( PendingTweaks );
			CurrentSubState->ApplyTweakChanges();
		}
		m_PendingAISubStateIndex = INDEX_NONE;
		m_pPendingAISubStateTweaks = NULL;
	}
	else
	{
		m_pPendingAISubStateTweaks = NULL;
		m_CurrentAISubStateIndex = INDEX_NONE;
		m_PendingAISubStateIndex = INDEX_NONE;
	}
}

// DISHONORED(port): 2013 rva 0x727b60 (2012 0x765760)
void UDisAISubStateMachine::PostDestroyFSM_Derived()
{
	m_pCurrentAISubState = NULL;
	m_CurrentAISubStateIndex = INDEX_NONE;
	m_PendingAISubStateIndex = INDEX_NONE;
	m_pPendingAISubStateTweaks = NULL;
}

// DISHONORED(port): 2013 rva 0x727b80 (2012 0x765780): the base machine compares state classes; the AI machine compares
// slots, so re-entering the same sub-state class in a different slot is a real change.
UBOOL UDisAISubStateMachine::PendingStateIsCurrentState_Derived()
{
	return m_PendingAISubStateIndex == m_CurrentAISubStateIndex;
}

// DISHONORED(port): 2013 rva 0x727ba0 (2012 0x7657a0)
void UDisAISubStateMachine::ClearPendingState_Derived()
{
	m_PendingAISubStateIndex = INDEX_NONE;
	m_pPendingAISubStateTweaks = NULL;
}
