// DishonoredGame/src/dishonorednativestatemachine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x6927b0  public: static void __cdecl UDishonoredNativeStateMachine::InitializePrivateStaticClassUDishonoredNativeStateMachine(void)
//   0x6927d0  public: __thiscall FDisNativeFSMRejectedInfo::FDisNativeFSMRejectedInfo(class UClass *, class UClass *, enum eDisNativeFSMRejectReason)
//   0x692810  public: unsigned int __thiscall FDisNativeFSMRejectedInfo::operator!=(struct FDisNativeFSMRejectedInfo const &)const
//   0x692850  public: void __thiscall UDishonoredNativeStateMachine::DoStateChange(void)
//   0x6928c0  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetCurrentlyActiveState(void)const
//   0x6928e0  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetPendingState(void)const
//   0x692900  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetLogicalState(void)const
//   0x692920  public: class UClass const * __thiscall UDishonoredNativeStateMachine::GetPendingStateID(void)const
//   0x692940  public: void __thiscall UDishonoredNativeStateMachine::LockFSM(unsigned int)
//   0x692960  public: void __thiscall UDishonoredNativeStateMachine::SavePartialState(class FArchive &, enum ESaveLoadLocation)
//   0x6929b0  public: void __thiscall UDishonoredNativeStateMachine::PostLoadPartialState(void)
//   0x6929e0  private: void __thiscall UDishonoredNativeStateMachine::ClearPendingState(void)
//   0x695960  public: unsigned int __thiscall UDishonoredNativeStateMachine::IsCurState(class UClass const *, unsigned int)const
//   0x69a240  private: void __thiscall UDishonoredNativeStateMachine::DebugStoreRejectedStateInfo(struct FDisNativeFSMRejectedInfo const &)
//   0x69a2c0  public: void __thiscall UDishonoredNativeStateMachine::OnPawnShutDown(class ADishonoredPawn const &)
//   0x6a0330  public: void __thiscall UDishonoredNativeStateMachine::TickStateMachine(float)
//   0x6a1d80  public: void __thiscall UDishonoredNativeStateMachine::GetAllStateIDs(struct TMemStackArray<class UClass *> &)const
//   0x6a3970  private: void __thiscall UDishonoredNativeStateMachine::DemandStateChange(class UDishonoredNativeState *, struct FDisNativeStateParam &)
//   0x6a3be0  public: void __thiscall UDishonoredNativeStateMachine::LoadPartialState(class FArchive &, enum ESaveLoadLocation)
//   0x6a3c50  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::FindState(class UClass *)const
//   0x6a59b0  public: unsigned int __thiscall UDishonoredNativeStateMachine::CanTransitionTo(class UClass *, class UDishonoredNativeState * *)const
//   0x6a5a30  public: unsigned int __thiscall UDishonoredNativeStateMachine::RequestStateChange(struct FDisNativeStateParam &, class UObject * const, unsigned int)
//   0x6a89e0  public: void __thiscall UDishonoredNativeStateMachine::DestroyFSM(void)
//   0x6a9270  public: void __thiscall UDishonoredNativeStateMachine::BuildNativeStateMap(void)
//   0x6a9360  public: virtual void __thiscall UDishonoredNativeStateMachine::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6ac450  public: static class UClass * __cdecl UDishonoredNativeStateMachine::GetPrivateStaticClassUDishonoredNativeStateMachine(wchar_t const *)
//   0x6ac4e0  public: void __thiscall UDishonoredNativeStateMachine::InitFSM(class UObject * const, struct FDisNativeStateParam &, struct TMemStackArray<class UDishonoredNativeState *> *)
//   0x6ad300  public: static class UClass * __cdecl UDishonoredNativeStateMachine::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "dishonoredutilities_saveload.h"

// DISHONORED(written): FDisNativeFSMRejectedInfo(UClass*, UClass*, eDisNativeFSMRejectReason) 2012 rva 0x6927d0 (inlined into the
// 2013 callers): stamped with the world's real time, one rejection
static FDisNativeFSMRejectedInfo DisMakeRejectedInfo( UClass* RejectedStateID, UClass* StateThatRejectedID, BYTE Reason )
{
	FDisNativeFSMRejectedInfo Info(EC_EventParm);
	Info.m_pRejectedStateID = RejectedStateID;
	Info.m_pStateThatRejectedID = StateThatRejectedID;
	Info.m_RejectedReason = Reason;
	Info.m_fTimeOfRejection = GWorld ? GWorld->GetRealTimeSeconds() : 0.f;
	Info.m_NumRejections = 1;
	return Info;
}

// DISHONORED(written): the (state, previous firing object) pairs RequestStateChange / DemandStateChange notify after the change;
// retail keeps them in a TMemStackArray on GMainThreadMemStack
struct FDisFiringObjectChange
{
	UDishonoredNativeState* State;
	UObject* OldFiringObject;
};

static void DisSwapFiringObject( UDishonoredNativeState* State, UObject* NewFiringObject, TArray<FDisFiringObjectChange>& Changes )
{
	UObject* Old = State->GetFiringObject();
	State->SetFiringObject( NewFiringObject );
	if( Old != State->GetFiringObject() )
	{
		FDisFiringObjectChange* Change = new( Changes ) FDisFiringObjectChange;
		Change->State = State;
		Change->OldFiringObject = Old;
	}
}

static void DisNotifyFiringObjectChanges( const TArray<FDisFiringObjectChange>& Changes )
{
	for( INT Index = 0; Index < Changes.Num(); Index++ )
	{
		Changes(Index).State->OnFiringObjectChanged( Changes(Index).OldFiringObject );
	}
}

// DISHONORED(written): 2013 rva 0x65f930 (2012 0x6928c0, same bytes): nothing is active while DestroyFSM runs
UDishonoredNativeState* UDishonoredNativeStateMachine::GetCurrentlyActiveState() const
{
	return m_bDestroyFSMCalled ? NULL : m_pCurrentState;
}

// DISHONORED(written): 2013 rva 0x65f950 (2012 0x6928e0)
UDishonoredNativeState* UDishonoredNativeStateMachine::GetPendingState() const
{
	return m_bDestroyFSMCalled ? NULL : m_pPendingState;
}

// DISHONORED(written): 2013 rva 0x65f970 (2012 0x692900, same bytes): the state the machine is about to be in
UDishonoredNativeState* UDishonoredNativeStateMachine::GetLogicalState() const
{
	if( m_bDestroyFSMCalled )
	{
		return NULL;
	}
	return m_pPendingState ? m_pPendingState : m_pCurrentState;
}

// DISHONORED(written): 2013 rva 0x65f990 (2012 0x692920, same bytes)
const UClass* UDishonoredNativeStateMachine::GetPendingStateID() const
{
	return m_bDestroyFSMCalled ? NULL : m_pPendingStateID;
}

// DISHONORED(written): 2013 rva 0x6726e0 (2012 0x6a3c50, same bytes): m_NativeStateMap is keyed by the state class
UDishonoredNativeState* UDishonoredNativeStateMachine::FindState( UClass* StateID ) const
{
	UDishonoredNativeState* const* State = m_NativeStateMap.Find( StateID );
	return State ? *State : NULL;
}

// DISHONORED(written): 2013 rva 0x65f9b0 (2012 0x692940, same bytes)
void UDishonoredNativeStateMachine::LockFSM( UBOOL bLock )
{
	m_bIsLocked = bLock ? TRUE : FALSE;
}

// DISHONORED(written): 2013 rva 0x666df0 (2012 0x695960, same bytes): the pending state wins over the current one
UBOOL UDishonoredNativeStateMachine::IsCurState( const UClass* StateID, UBOOL bExactClass ) const
{
	if( m_bDestroyFSMCalled )
	{
		return FALSE;
	}
	const UClass* CurID = m_pPendingStateID ? m_pPendingStateID : m_pCurrentStateID;
	if( bExactClass )
	{
		return CurID == StateID;
	}
	return CurID && CurID->IsChildOf( StateID );
}

// DISHONORED(written): 2013 rva 0x670e50 (2012 0x6a1d80): the classes of every registered state
void UDishonoredNativeStateMachine::GetAllStateIDs( TArray<UClass*>& OutStateIDs ) const
{
	for( INT Index = 0; Index < m_NativeStates.Num(); Index++ )
	{
		OutStateIDs.AddItem( m_NativeStates(Index)->GetClass() );
	}
}

// DISHONORED(written): 2013 rva 0x666e40 (2012 0x69a2c0)
void UDishonoredNativeStateMachine::OnPawnShutDown( const ADishonoredPawn& Pawn )
{
	for( INT Index = 0; Index < m_NativeStates.Num(); Index++ )
	{
		m_NativeStates(Index)->OnPawnShutDown( Pawn );
	}
}

// DISHONORED(written): 2013 rva 0x67a8f0 (2012 0x6a9270): the class map of m_NativeStates, every state pointed back at this
// machine, then the transition logic initialised from the state list
void UDishonoredNativeStateMachine::BuildNativeStateMap()
{
	m_NativeStateMap.Empty( m_NativeStates.Num() );
	for( INT Index = 0; Index < m_NativeStates.Num(); Index++ )
	{
		UDishonoredNativeState* State = m_NativeStates(Index);
		m_NativeStateMap.Set( State->GetClass(), State );
		State->m_pStateMachine = this;
	}
	if( m_pTransitionLogic )
	{
		m_pTransitionLogic->InitTransitionLogic( m_NativeStates );
	}
}

// DISHONORED(written): 2013 rva 0x67ba50 (2012 0x6ac4e0, dishonorednativestatemachine.cpp:82): the default state parameters are
// kept as raw bytes (SizeOf) for the later resets, and the first state is entered synchronously
void UDishonoredNativeStateMachine::InitFSM( UObject* const ManagedObject, FDisNativeStateParam& DefaultStateParams, const TArray<UDishonoredNativeState*>* FSMStates )
{
	m_bDestroyFSMCalled = FALSE;
	m_pManagedObject = ManagedObject;
	if( FSMStates )
	{
		m_NativeStates = *FSMStates;
	}
	BuildNativeStateMap();
	const INT ParamSize = DefaultStateParams.SizeOf();
	m_DefaultStateParam.Empty( ParamSize );
	m_DefaultStateParam.Add( ParamSize );
	appMemcpy( m_DefaultStateParam.GetData(), &DefaultStateParams, ParamSize );
	RequestStateChange( DefaultStateParams, NULL, FALSE );
	DoStateChange();
}

// DISHONORED(written): 2012 rva 0x6a89e0 (dishonorednativestatemachine.cpp:107; unmatched in the 2013 db, same shape in the 2013
// callers): the pending state is cancelled, the current one exited towards no state, everything unlinked
void UDishonoredNativeStateMachine::DestroyFSM()
{
	m_bDestroyFSMCalled = TRUE;
	if( m_pPendingState )
	{
		m_pPendingState->OnCancelState();
	}
	if( m_pCurrentState )
	{
		m_pCurrentState->OnExitState( NULL );
	}
	m_pCurrentState = NULL;
	m_pCurrentStateID = NULL;
	m_pPendingState = NULL;
	m_pPendingStateID = NULL;
	m_pManagedObject = NULL;
	m_NativeStateMap.Empty();
	PostDestroyFSM_Derived();
}

// DISHONORED(written): 2013 rva 0x674f20 (2012 0x6a59b0, same bytes): without transition logic every change is allowed; the
// pending state is asked first, then the current one
UBOOL UDishonoredNativeStateMachine::CanTransitionTo( UClass* StateID, UDishonoredNativeState** OutRejectingState ) const
{
	if( !m_pTransitionLogic )
	{
		return TRUE;
	}
	if( m_pPendingState && !m_pTransitionLogic->CanTransition( m_pPendingState, StateID ) )
	{
		if( OutRejectingState )
		{
			*OutRejectingState = m_pPendingState;
		}
		return FALSE;
	}
	if( m_pCurrentState && !m_pTransitionLogic->CanTransition( m_pCurrentState, StateID ) )
	{
		if( OutRejectingState )
		{
			*OutRejectingState = m_pCurrentState;
		}
		return FALSE;
	}
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x65fa00 (2012 0x6929e0)
void UDishonoredNativeStateMachine::ClearPendingState()
{
	m_pPendingStateID = NULL;
	m_pPendingState = NULL;
	ClearPendingState_Derived();
}

// DISHONORED(written): 2013 rva 0x674fa0 (2012 0x6a5a30): a locked or destroyed machine refuses; a refused request tells the
// rejecting state (reason 0 pending / 1 current); an accepted one cancels the pending state (reason 2), records the new pending
// state, moves the firing objects (the pending state fires for itself unless a firing object is given) and lets the parameters
// hear about it before the OnFiringObjectChanged notifications
UBOOL UDishonoredNativeStateMachine::RequestStateChange( FDisNativeStateParam& Param, UObject* const FiringObject, UBOOL bTestOnly )
{
	if( m_bIsLocked || m_bDestroyFSMCalled )
	{
		return FALSE;
	}
	UClass* StateID = Param.m_pStateClass;
	UDishonoredNativeState* RejectingState = NULL;
	const UBOOL bCanTransition = CanTransitionTo( StateID, &RejectingState );
	if( bTestOnly )
	{
		return bCanTransition;
	}
	if( !bCanTransition )
	{
		if( RejectingState )
		{
			RejectingState->OnRejectStateRequest( StateID );
			DebugStoreRejectedStateInfo( DisMakeRejectedInfo( StateID, RejectingState->GetClass(), RejectingState == m_pCurrentState ? 1 : 0 ) );
		}
		return bCanTransition;
	}
	m_bIsLocked = TRUE;
	TArray<FDisFiringObjectChange> Changes;
	UBOOL bCancelledPending = FALSE;
	if( m_pPendingState )
	{
		DisSwapFiringObject( m_pPendingState, NULL, Changes );
		m_pPendingState->OnCancelState();
		DebugStoreRejectedStateInfo( DisMakeRejectedInfo( m_pPendingStateID, StateID, 2 ) );
		ClearPendingState();
		bCancelledPending = TRUE;
	}
	m_pPendingStateID = StateID;
	m_pPendingState = m_NativeStateMap.FindRef( StateID );
	if( !bCancelledPending && m_pCurrentState )
	{
		DisSwapFiringObject( m_pCurrentState, NULL, Changes );
	}
	if( m_pPendingState )
	{
		DisSwapFiringObject( m_pPendingState, FiringObject ? FiringObject : m_pPendingState, Changes );
	}
	UObject* ManagedObject = m_pManagedObject;
	m_bIsLocked = FALSE;
	Param.OnPending( m_pPendingState, ManagedObject );
	DisNotifyFiringObjectChanges( Changes );
	return bCanTransition;
}

// DISHONORED(written): 2013 rva 0x672190 (2012 0x6a3970): the unconditional variant a state uses to leave (RequestStateExit):
// no transition logic, no lock, the parameters hear first, the pending state is cancelled (reason 2) and replaced
void UDishonoredNativeStateMachine::DemandStateChange( UDishonoredNativeState* DemandingState, FDisNativeStateParam& Param )
{
	UClass* StateID = Param.m_pStateClass;
	UDishonoredNativeState* NewState = m_NativeStateMap.FindRef( StateID );
	Param.OnPending( NewState, m_pManagedObject );
	TArray<FDisFiringObjectChange> Changes;
	UBOOL bCancelledPending = FALSE;
	if( m_pPendingState )
	{
		DisSwapFiringObject( m_pPendingState, NULL, Changes );
		m_pPendingState->OnCancelState();
		DebugStoreRejectedStateInfo( DisMakeRejectedInfo( m_pPendingStateID, StateID, 2 ) );
		bCancelledPending = TRUE;
	}
	m_pPendingStateID = StateID;
	m_pPendingState = NewState;
	if( !bCancelledPending && m_pCurrentState )
	{
		DisSwapFiringObject( m_pCurrentState, NULL, Changes );
	}
	DisNotifyFiringObjectChanges( Changes );
}

// DISHONORED(written): 2013 rva 0x65f8c0 (2012 0x692850): exit the current state towards the pending one, make it current,
// PostStateChange_Derived, then enter it from the last one (retail enters a NULL pending state too; guarded here)
void UDishonoredNativeStateMachine::DoStateChange()
{
	if( m_pCurrentState )
	{
		m_pCurrentState->OnExitState( m_pPendingState );
	}
	UDishonoredNativeState* Pending = m_pPendingState;
	UDishonoredNativeState* Last = m_pCurrentState;
	if( Pending )
	{
		m_pCurrentStateID = m_pPendingStateID;
		m_pCurrentState = Pending;
		PostStateChange_Derived();
	}
	m_pPendingStateID = NULL;
	m_pPendingState = NULL;
	if( Pending )
	{
		Pending->OnEnterState( Last );
	}
}

// DISHONORED(written): 2013 rva 0x66de60 (2012 0x6a0330): a pending request for the current state's class is a reset
// (OnResetState, then the pending state is cleared when nothing new was requested); a pending different state ends the loop and
// DoStateChange runs it; otherwise the current state ticks. Old debug rejection records are dropped afterwards.
void UDishonoredNativeStateMachine::TickStateMachine( FLOAT DeltaSeconds )
{
	for( ; ; )
	{
		if( m_pPendingStateID )
		{
			if( m_pPendingStateID == m_pCurrentStateID && PendingStateIsCurrentState_Derived() )
			{
				m_pCurrentState->OnResetState();
				if( m_pPendingStateID && m_pPendingStateID == m_pCurrentStateID && PendingStateIsCurrentState_Derived() )
				{
					m_pPendingStateID = NULL;
					m_pPendingState = NULL;
					ClearPendingState_Derived();
				}
			}
			if( m_pPendingStateID )
			{
				if( m_pPendingStateID == m_pCurrentStateID && PendingStateIsCurrentState_Derived() )
				{
					continue;
				}
				break;
			}
		}
		if( m_pCurrentState )
		{
			m_pCurrentState->TickState( DeltaSeconds );
		}
		if( !m_pPendingStateID )
		{
			break;
		}
		if( !( m_pPendingStateID == m_pCurrentStateID && PendingStateIsCurrentState_Derived() ) )
		{
			break;
		}
	}
	if( m_pPendingStateID )
	{
		DoStateChange();
	}
	const FLOAT RealTimeSeconds = GWorld ? GWorld->GetRealTimeSeconds() : 0.f;
	for( INT Index = m_Debug_FailedStates.Num() - 1; Index >= 0; Index-- )
	{
		if( RealTimeSeconds - m_Debug_FailedStates(Index).m_fTimeOfRejection > m_fDebug_KeepFailedStateTime )
		{
			m_Debug_FailedStates.Remove( 0, Index + 1 );
			break;
		}
	}
}

// DISHONORED(written): 2013 rva 0x666d70 (2012 0x69a240, same bytes): only while the player's HUD shows debug info; a repeat of
// the last record bumps its count and time, anything else is appended (FDisNativeFSMRejectedInfo::operator!= 2012 0x692810
// compares the two state ids and the reason)
void UDishonoredNativeStateMachine::DebugStoreRejectedStateInfo( const FDisNativeFSMRejectedInfo& Info )
{
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	if( !PC || !PC->myHUD || !PC->myHUD->bShowDebugInfo )
	{
		return;
	}
	if( m_Debug_FailedStates.Num() )
	{
		FDisNativeFSMRejectedInfo& Last = m_Debug_FailedStates.Last();
		if( Last.m_pRejectedStateID == Info.m_pRejectedStateID && Last.m_pStateThatRejectedID == Info.m_pStateThatRejectedID && Last.m_RejectedReason == Info.m_RejectedReason )
		{
			Last.m_NumRejections++;
			Last.m_fTimeOfRejection = Info.m_fTimeOfRejection;
			return;
		}
	}
	m_Debug_FailedStates.AddItem( Info );
}
/*-----------------------------------------------------------------------------
	DisSaveLoad. DISHONORED(port): agent EJ (PHASE12 EJ). LoadPartialState (0x672670) is in dissavegame.cpp,
	where agent ED put it; this is the machine's own GameLoad, which is a different slot and a different body.

	GameSave is the UDisAttentionInfo_Base::GameSave fold (0x88af60) at slot 69 and is not ported.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x67a9e0 (2012 0x6a9360, byte-identical), retail vtable slot 70. One read - the
// machine's own script properties, which is where m_NativeStates, m_pCurrentStateID and m_pManagedObject come
// from - and then the class-to-state map is rebuilt from the restored m_NativeStates, because the map is a
// TMap of raw pointers that the save does not carry.
void UDishonoredNativeStateMachine::GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location )
{
	DisSaveLoadObject( _rArchive, this );
	BuildNativeStateMap();
}
