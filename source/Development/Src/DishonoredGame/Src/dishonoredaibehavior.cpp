// DishonoredGame/src/dishonoredaibehavior.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (43): see agentCG_status.csv for the ported set and their rvas.

// ---- agent CG ports (PHASE9 CG): UDishonoredAIBehavior, the brain's behaviour slot ----

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "dishonoredutilities.h"
#include "dishonoredutilities_ai.h"
#include "disaisubstate.h"
#include "aistimstruct.h"

/*-----------------------------------------------------------------------------
	Construction and tweaks
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x53dfd0-shaped accessor pair; the behaviour's own tweaks pointer is m_pBehaviorTweaks
// (2012 0x748600 SetTweaks_Derived / the 7-byte getter). The per-subclass-storage trap that cost agent AU 147 pickups
// does NOT apply here: UDishonoredAIBehavior itself owns the pointer and every subclass inherits it, which the
// retail bodies confirm (SetTweaks_Derived writes this one member and then re-applies the sub-state/sub-process
// tweaks from it).
UDisTweaksBase* UDishonoredAIBehavior::GetTweaks_Derived()
{
	return m_pBehaviorTweaks;
}

// DISHONORED(port): 2013 rva 0x6ea740 (2012 0x748600): store the tweaks, then push the matching sub-state tweaks into
// the currently active and the pending sub-state, and the matching sub-process tweaks into every sub-process. The
// arrays are positional: sub-state index i takes m_SubStateTweaks(i), sub-process index i takes m_SubProcessTweaks(i).
void UDishonoredAIBehavior::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pBehaviorTweaks = (UDisTweaks_AIBehavior*)_pTweaks;
	if( !m_pBehaviorTweaks )
	{
		return;
	}

	if( m_pBehaviorFSM && m_pBehaviorTweaks->m_SubStateTweaks.Num() > 0 )
	{
		const INT ActiveIndex = m_pBehaviorFSM->GetActiveSubStateIndex();
		if( ActiveIndex != INDEX_NONE && ActiveIndex < m_pBehaviorTweaks->m_SubStateTweaks.Num() )
		{
			UDisAISubState* Current = m_pBehaviorFSM->GetCurrentAISubState();
			if( Current )
			{
				Current->SetTweaks( m_pBehaviorTweaks->m_SubStateTweaks(ActiveIndex) );
			}
		}
		const INT PendingIndex = m_pBehaviorFSM->GetPendingSubStateIndex();
		if( PendingIndex != INDEX_NONE && PendingIndex < m_pBehaviorTweaks->m_SubStateTweaks.Num() )
		{
			m_pBehaviorFSM->SetPendingSubStateTweaks( m_pBehaviorTweaks->m_SubStateTweaks(PendingIndex) );
		}
	}

	const INT NumSubProcessTweaks = m_pBehaviorTweaks->m_SubProcessTweaks.Num();
	for( INT Idx = 0; Idx < NumSubProcessTweaks && Idx < m_SubProcesses.Num(); Idx++ )
	{
		UDisAISubProcess* SubProcess = m_SubProcesses(Idx);
		if( SubProcess )
		{
			SubProcess->SetTweaks( m_pBehaviorTweaks->m_SubProcessTweaks(Idx) );
		}
	}
}

// DISHONORED(port): 2013 rva 0x6f7030 (2012 0x750e90, 1396 bytes): the whole construction of a behaviour.
//   1. the owning brain and the global AI manager, the desires interface, the designated slot from the tweaks
//   2. m_bIsPending set and the other three behaviour flags cleared, m_LastUnsupportedBark = eDisDialogHook_MAX (122)
//   3. the tweaks, through SetTweaks_Derived + ApplyTweakChanges
//   4. the sub-state machine: a UDisAISubStateMachine is constructed, one UDisAISubStateInit plus one sub-state per
//      entry of the tweaks' m_SubStateTweaks (skipping a class that is already in the list, which is why two tweaks
//      naming the same sub-state class share one instance), and InitFSM is given the whole list and the init state's
//      FDisAISubStateInit_Param carrying the pawn's current body intention
//   5. one sub-process per entry of m_SubProcessTweaks
//   6. the four stim masks, then the subclass's InitBehavior
void UDishonoredAIBehavior::CallInitBehavior( UDishonoredAIBrain* const _pAIBrain, UDisTweaks_AIBehavior* const _pBehaviorTweaks )
{
	m_pOwningBrain = _pAIBrain;
	m_pGlobalAIMan = DisGetGlobalAIManagerUnchecked();

	UObject* DesiresObject = GetDesires();
	m_pDesires = TScriptInterface<IDisDesiresInterface>();
	if( DesiresObject )
	{
		void* Interface = DesiresObject->GetInterfaceAddress( UDisDesiresInterface::StaticClass() );
		if( Interface )
		{
			m_pDesires.SetObject( DesiresObject );
			m_pDesires.SetInterface( Interface );
		}
	}

	m_DesignatedSlot = _pBehaviorTweaks->m_DesignatedSlot;
	m_bHasStarted = FALSE;
	m_bIsPaused = FALSE;
	m_bIsPending = TRUE;
	m_bForceFinishDueToDormancy = FALSE;
	m_LastUnsupportedBark = DDH_INVALID;

	if( GetTweaks_Derived() != _pBehaviorTweaks )
	{
		SetTweaks_Derived( _pBehaviorTweaks );
		ApplyTweakChanges();
	}

	m_pBehaviorFSM = (UDisAISubStateMachine*)StaticConstructObject( UDisAISubStateMachine::StaticClass(), this );

	TArray<UDishonoredNativeState*> BehaviorStates;
	BehaviorStates.Empty( _pBehaviorTweaks->m_SubStateTweaks.Num() + 1 );

	UDisAISubState* InitState = (UDisAISubState*)StaticConstructObject( UDisAISubStateInit::StaticClass(), this );
	InitState->InitSubState( this, NULL );
	BehaviorStates.AddItem( InitState );

	for( INT Idx = 0; Idx < _pBehaviorTweaks->m_SubStateTweaks.Num(); Idx++ )
	{
		UDisTweaks_AISubState* SubStateTweaks = _pBehaviorTweaks->m_SubStateTweaks(Idx);
		UClass* SubStateClass = SubStateTweaks ? SubStateTweaks->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame ) : NULL;
		if( !SubStateClass )
		{
			continue;
		}
		UBOOL bAlready = FALSE;
		for( INT Have = 0; Have < BehaviorStates.Num(); Have++ )
		{
			if( BehaviorStates(Have)->GetClass() == SubStateClass )
			{
				bAlready = TRUE;
				break;
			}
		}
		if( bAlready )
		{
			continue;
		}
		UDisAISubState* SubState = (UDisAISubState*)StaticConstructObject( SubStateClass, this );
		SubState->InitSubState( this, SubStateTweaks );
		BehaviorStates.AddItem( SubState );
	}

	if( m_pDesires.GetObject() && m_pDesires.GetInterface() )
	{
		DisAINoteDesiresGap( TEXT("UDishonoredAIBehavior::CallInitBehavior") );
	}

	ADishonoredNPCPawn* OwningPawn = m_pOwningBrain ? m_pOwningBrain->GetOwningPawn() : NULL;
	// The parameter carries the state class the machine is to enter: retail's FDisAISubStateInit_Param constructor sets
	// it to UDisAISubStateInit, and without it InitFSM asks the machine to change to a NULL state class.
	FDisAISubStateInit_Param AISubStateParam( UDisAISubStateInit::StaticClass() );
	if( OwningPawn )
	{
		AISubStateParam.m_BodyIntentionBeforeEnterState.m_IntendedBodyStance = OwningPawn->GetBodyStance();
		AISubStateParam.m_BodyIntentionBeforeEnterState.m_pDesiredPrimaryItemClass = OwningPawn->GetDesiredPrimaryItem();
		AISubStateParam.m_BodyIntentionBeforeEnterState.m_pDesiredSecondaryItemClass = OwningPawn->GetDesiredSecondaryItem();
	}
	m_pBehaviorFSM->InitFSM( this, AISubStateParam, &BehaviorStates );

	m_SubProcesses.Empty( _pBehaviorTweaks->m_SubProcessTweaks.Num() );
	for( INT Idx = 0; Idx < _pBehaviorTweaks->m_SubProcessTweaks.Num(); Idx++ )
	{
		UDisTweaks_AISubProcess* SubProcessTweaks = _pBehaviorTweaks->m_SubProcessTweaks(Idx);
		UClass* SubProcessClass = SubProcessTweaks ? SubProcessTweaks->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame ) : NULL;
		if( !SubProcessClass )
		{
			continue;
		}
		UDisAISubProcess* SubProcess = (UDisAISubProcess*)StaticConstructObject( SubProcessClass, this );
		SubProcess->InitSubProcess( this, SubProcessTweaks );
		m_SubProcesses.AddItem( SubProcess );
	}

	m_pEvaluateStimMask = (FPointer)BuildEvaluateStimMask();
	const BYTE* SubProcessesMask = NULL;
	const BYTE* CompleteMask = NULL;
	m_pFilterStimMask = (FPointer)BuildBehaviorFilterStimMasks( SubProcessesMask, CompleteMask );
	m_pSubProcessesFilterStimMask = (FPointer)SubProcessesMask;
	m_pCompleteFilterStimMask = (FPointer)CompleteMask;
	m_pShouldFinishWhileDormantStimMask = (FPointer)BuildShouldFinishWhileDormantStimMask();

	InitBehavior( _pAIBrain );
	GDisAIBehaviorInits++;
}

// DISHONORED(port): 2013 rva 0x6e4b60 (2012 0x12e290, 27 bytes): the base behaviour has no filter mask of its own and
// hands the same NULL out for all three, which is what makes CallFilterAIStim's three guards the whole filter.
const BYTE* UDishonoredAIBehavior::BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask )
{
	_rOutSubProcessesMask = NULL;
	_rOutCompleteMask = NULL;
	return NULL;
}

/*-----------------------------------------------------------------------------
	The per-frame and per-thought passes
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6eaaa0 (2012 0x7488a0): the subclass's TickBehavior, then the desires, then every
// enabled sub-process, then the sub-state machine. The dormancy flag and IsBehaviorFinished are re-tested between
// every step, because any of them can finish the behaviour.
void UDishonoredAIBehavior::CallTickBehavior( FLOAT _fDeltaSeconds )
{
	if( m_bForceFinishDueToDormancy )
	{
		return;
	}
	if( !IsBehaviorFinished() )
	{
		TickBehavior( _fDeltaSeconds );
		if( m_pDesires.GetObject() && m_pDesires.GetInterface() )
		{
			DisAINoteDesiresGap( TEXT("UDishonoredAIBehavior::CallTickBehavior") );
		}
	}
	if( m_bForceFinishDueToDormancy )
	{
		return;
	}
	if( !IsBehaviorFinished() )
	{
		for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
		{
			UDisAISubProcess* SubProcess = m_SubProcesses(Idx);
			if( SubProcess && SubProcess->IsSubProcessEnabled() )
			{
				SubProcess->TickSubProcess( _fDeltaSeconds );
			}
		}
	}
	if( !m_bForceFinishDueToDormancy && !IsBehaviorFinished() )
	{
		m_pBehaviorFSM->TickAIFSM( _fDeltaSeconds );
	}
}

// DISHONORED(port): 2013 rva 0x6ea9b0 (2012 0x7487b0): the same three-step shape as CallTickBehavior on the thought
// clock rather than the frame clock (RefreshThoughts, the sub-processes, RefreshAIFSM).
void UDishonoredAIBehavior::CallRefreshThoughts( FLOAT _fTimeSinceLastThought )
{
	if( m_bForceFinishDueToDormancy )
	{
		return;
	}
	if( !IsBehaviorFinished() )
	{
		RefreshThoughts( _fTimeSinceLastThought );
	}
	if( m_bForceFinishDueToDormancy )
	{
		return;
	}
	if( !IsBehaviorFinished() )
	{
		for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
		{
			UDisAISubProcess* SubProcess = m_SubProcesses(Idx);
			if( SubProcess && SubProcess->IsSubProcessEnabled() )
			{
				SubProcess->RefreshSubProcess( _fTimeSinceLastThought );
			}
		}
	}
	if( !m_bForceFinishDueToDormancy && !IsBehaviorFinished() )
	{
		m_pBehaviorFSM->RefreshAIFSM( _fTimeSinceLastThought );
	}
}

// DISHONORED(port): 2013 rva 0x6ea8f0 (2012 0x759c40): the sub-state machine first, then the subclass, then the
// desires (a behaviour being terminated does not pause them - EndSubProcess finalizes instead), then every
// sub-process, and finally every minimum-attention request this behaviour placed is dropped.
void UDishonoredAIBehavior::CallOnBehaviorPause( UBOOL _bIsBeingTerminated )
{
	m_pBehaviorFSM->OnOwningBehaviorPause( _bIsBeingTerminated );
	OnBehaviorPause( _bIsBeingTerminated );
	if( m_pDesires.GetObject() && m_pDesires.GetInterface() && !_bIsBeingTerminated )
	{
		DisAINoteDesiresGap( TEXT("UDishonoredAIBehavior::CallOnBehaviorPause") );
	}
	m_bIsPaused = TRUE;
	for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
	{
		if( m_SubProcesses(Idx) )
		{
			m_SubProcesses(Idx)->OnOwningBehaviorPause( _bIsBeingTerminated );
		}
	}
	// DISHONORED(bringup): UDishonoredAIBrain::ClearAllMinAttention( DMALT_AIBehavior ) (2013 rva 0x724280) is the last
	// line of retail's body. The minimum-attention list lives on UDisAIBrainProcessAttention, which is not ported, so
	// nothing has placed a request to clear.
}

// DISHONORED(port): 2013 rva 0x6f7670 (2012 0x751410): resume every sub-process, fire the dialog hook that was queued
// while paused, re-register the actor-terminated listener if this behaviour still holds an action target, fire the
// Kismet BehaviorStarted event, resume the desires, then the subclass and the sub-state machine.
void UDishonoredAIBehavior::CallOnBehaviorResume( const FDisBodyIntention& _rPreviousBodyIntention )
{
	m_bIsPaused = FALSE;
	for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
	{
		if( m_SubProcesses(Idx) )
		{
			m_SubProcesses(Idx)->OnOwningBehaviorResume( _rPreviousBodyIntention );
		}
	}

	if( m_OnResumeDialog.m_Hook != DDH_INVALID )
	{
		// DISHONORED(bringup): FireDialogHook (2013 rva 0x6f00f0) needs the conversation system (UDisConvGlobalMan and
		// IDisConvSpeakerInterface), which is not ported; the queued hook is dropped exactly as retail drops it after
		// firing, so the state machine does not queue for ever.
		m_OnResumeDialog.m_Hook = DDH_INVALID;
		m_OnResumeDialog.m_pInitiator = TScriptInterface<IDisConvSpeakerInterface>();
	}

	if( m_pOwningBrain && m_pOwningBrain->GetOwningPawn() )
	{
		ADishonoredNPCPawn* Pawn = m_pOwningBrain->GetOwningPawn();
		DisFireKismetEvent( Pawn, UDisSeqEvent_BehaviorStarted::StaticClass(), Pawn, Pawn, 0, FALSE );
	}

	if( m_pDesires.GetObject() && m_pDesires.GetInterface() )
	{
		DisAINoteDesiresGap( TEXT("UDishonoredAIBehavior::CallOnBehaviorResume") );
	}
	OnBehaviorResume();
	m_pBehaviorFSM->OnOwningBehaviorResume( _rPreviousBodyIntention );
	// DISHONORED(bringup): ADishonoredNPCPawn::UpdateAvoidableCollisionGroupFlags (2013 rva 0x759c50) closes retail's
	// body; it needs UArkAvoidable, which is not ported.
	GDisAIBehaviorActivations++;
}

// DISHONORED(port): 2013 rva 0x6f0020 (2012 0x74f480): a behaviour that leaves the active stack drops its queued
// dialog hook and its action-target listener, and a behaviour that says it cannot be dormant sets the flag that makes
// the brain finish it on the next ProcessAllStims instead of leaving it dormant on the stack.
void UDishonoredAIBehavior::OnBecomeDormant()
{
	m_OnResumeDialog.m_Hook = DDH_INVALID;
	m_OnResumeDialog.m_pInitiator = TScriptInterface<IDisConvSpeakerInterface>();
	if( !CanBeDormant() )
	{
		m_bForceFinishDueToDormancy = TRUE;
	}
}

/*-----------------------------------------------------------------------------
	Stim filtering
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6efdf0 (2012 0x74f2c0): three masks gate three filters. The complete mask is the union
// and short-circuits the whole call; the behaviour's own mask selects its FilterStim delegate; the sub-processes' mask
// asks every enabled sub-process whose own mask matches. A sub-process that blocks wins over one that does not, and
// the sub-state machine's own filter is asked last.
UBOOL UDishonoredAIBehavior::CallFilterAIStim( const FAIStimStruct& _rAIStim )
{
	const BYTE* CompleteMask = (const BYTE*)m_pCompleteFilterStimMask;
	if( !CompleteMask || ( CompleteMask[_rAIStim.m_StimID] & 1 ) == 0 )
	{
		return FALSE;
	}

	UBOOL bBlockAIStim = FALSE;
	const BYTE* FilterMask = (const BYTE*)m_pFilterStimMask;
	if( FilterMask && ( FilterMask[_rAIStim.m_StimID] & 1 ) != 0 )
	{
		bBlockAIStim = GetFilterStimDelegate( _rAIStim.m_StimID )( _rAIStim );
	}

	const BYTE* SubProcessesMask = (const BYTE*)m_pSubProcessesFilterStimMask;
	if( SubProcessesMask && ( SubProcessesMask[_rAIStim.m_StimID] & 1 ) != 0 )
	{
		for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
		{
			UDisAISubProcess* SubProcess = m_SubProcesses(Idx);
			if( !SubProcess || !SubProcess->IsSubProcessEnabled() )
			{
				continue;
			}
			const BYTE* SubProcessMask = (const BYTE*)SubProcess->m_pFilterStimMask;
			if( !SubProcessMask || ( SubProcessMask[_rAIStim.m_StimID] & 1 ) == 0 )
			{
				continue;
			}
			if( SubProcess->GetFilterStimDelegate_SubProcess( _rAIStim.m_StimID )( _rAIStim ) )
			{
				bBlockAIStim = TRUE;
			}
			else
			{
				bBlockAIStim = FALSE;
			}
		}
	}

	if( m_pBehaviorFSM->FilterAIFSM( _rAIStim ) || bBlockAIStim )
	{
		m_LastFilteredStim = _rAIStim.m_StimID;
		return TRUE;
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x6eff50 (2012 0x74f420): a dormant behaviour is asked whether this stim finishes it.
UBOOL UDishonoredAIBehavior::CallShouldFinishWhileDormant( const FAIStimStruct& _rAIStim )
{
	const BYTE* Mask = (const BYTE*)m_pShouldFinishWhileDormantStimMask;
	if( !Mask || ( Mask[_rAIStim.m_StimID] & 1 ) == 0 )
	{
		return FALSE;
	}
	const FDisStimPredicateDelegate Delegate = GetShouldFinishWhileDormantDelegate( _rAIStim.m_StimID );
	if( !Delegate.IsBound() )
	{
		return FALSE;
	}
	return Delegate( _rAIStim );
}

// DISHONORED(written): the base behaviour binds nothing, so every id answers the null delegate (2013 rvas 0x6e4b80,
// 0x6e4ba0 and the 27-byte siblings). Each UDisBehavior* subclass overrides these with its own stim table.
FDisStimPredicateDelegate UDishonoredAIBehavior::GetShouldFinishWhileDormantDelegate( BYTE _StimID )
{
	return FDisStimPredicateDelegate();
}

FDisStimSetupDelegate UDishonoredAIBehavior::GetSetupFromStimDelegate( BYTE _StimID )
{
	return FDisStimSetupDelegate();
}

FDisStimPredicateDelegate UDishonoredAIBehavior::GetFilterStimDelegate( BYTE _StimID )
{
	return FDisStimPredicateDelegate();
}

FDisStimPredicateDelegate UDishonoredAIBehavior::GetEvaluateStimDelegate( BYTE _StimID )
{
	return FDisStimPredicateDelegate();
}

/*-----------------------------------------------------------------------------
	Sub-states, sub-processes and the action target
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6e4bf0 (2012 0x745420)
UDisAISubState* UDishonoredAIBehavior::GetCurrentSubState() const
{
	return m_pBehaviorFSM ? m_pBehaviorFSM->GetCurrentAISubState() : NULL;
}

// DISHONORED(port): 2013 rva 0x6e4ad0 (2012 0x745390)
INT UDishonoredAIBehavior::GetActiveSubStateIndex() const
{
	return m_pBehaviorFSM ? m_pBehaviorFSM->GetActiveSubStateIndex() : INDEX_NONE;
}

// DISHONORED(port): 2013 rva 0x6e4ac0 (2012 0x745380)
INT UDishonoredAIBehavior::GetLogicalSubStateIndex() const
{
	return m_pBehaviorFSM ? m_pBehaviorFSM->GetLogicalSubStateIndex() : INDEX_NONE;
}

// DISHONORED(port): 2013 rva 0x6eae90 (2012 0x748c90): an exact class match, not a derived-from test, which is why a
// behaviour can hold two sub-processes of related classes and address each one.
UDisAISubProcess* UDishonoredAIBehavior::GetSubProcess( UClass* const _pSubProcessClass ) const
{
	for( INT Idx = 0; Idx < m_SubProcesses.Num(); Idx++ )
	{
		if( m_SubProcesses(Idx) && m_SubProcesses(Idx)->GetClass() == _pSubProcessClass )
		{
			return m_SubProcesses(Idx);
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x6e4c00 (2012 0x745430)
ADishonoredNPCPawn* UDishonoredAIBehavior::GetOwningPawn() const
{
	return m_pOwningBrain ? m_pOwningBrain->GetOwningPawn() : NULL;
}

// DISHONORED(port): 2013 rva 0x6eae50 (2012 0x748c50): the base behaviour has no enemy of its own; the combat
// behaviours override this with their attention target.
ADishonoredPawn* UDishonoredAIBehavior::GetCurrentEnemy() const
{
	return NULL;
}

// DISHONORED(port): 2013 rva 0x6eade0 (2012 0x748bd0): the base awareness is the brain tweaks'
// m_MinAttentionForAwareness mapped onto EAIAwareness; the alert behaviours override it.
BYTE UDishonoredAIBehavior::GetAwarenessLevel() const
{
	return EAIAwareness_Unaware;
}

// DISHONORED(port): 2013 rva 0x6e82e0 (2012 0x746b80): the player may push an NPC whose behaviour is not busy.
UBOOL UDishonoredAIBehavior::IsPlayerAllowedToPushMe() const
{
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x6eda60 (2012 0x74c750): the proxy's cached actor wins over the plain actor pointer.
AActor* UDishonoredAIBehavior::GetBehaviorActionTargetActor() const
{
	AActor* ProxyActor = m_ActionTargetProxy.m_pCachedTargetActor;
	return ProxyActor ? ProxyActor : m_pActionTargetActor;
}

// DISHONORED(port): 2013 rva 0x6ed9a0 (2012 0x74c690): the action target's location, from the proxy if it has one.
UBOOL UDishonoredAIBehavior::GetBehaviorActionTargetLocation( FVector& _rOut ) const
{
	AActor* Actor = GetBehaviorActionTargetActor();
	if( !Actor )
	{
		return FALSE;
	}
	_rOut = Actor->Location;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x6f0300 (2012 0x74f760) / 0x6f0230 (0x74f690) / 0x6f03c0 (0x74f820): setting or clearing
// the action target re-registers the actor-terminated listener, so a behaviour never holds a pointer to a destroyed
// actor.
// DISHONORED(bringup): the FArkGameEventDispatcher register/unregister pair around each of the three (event type 3,
// OnOtherActorTerminatedEvent) is left out here and is agent CG's hand-over 1 - see agentCG.md.
void UDishonoredAIBehavior::SetActionTargetActor( AActor* _pActor )
{
	m_pActionTargetActor = _pActor;
}

void UDishonoredAIBehavior::SetActionTargetProxy( FDisAttentionProxy _ActionTarget )
{
	m_ActionTargetProxy = _ActionTarget;
}

void UDishonoredAIBehavior::ClearActionTarget()
{
	m_pActionTargetActor = NULL;
	m_ActionTargetProxy.ClearAttnProxy();
}

// DISHONORED(port): 2013 rva 0x6eaf30 (2012 0x74c560): the flag bits are 1 OnEnter, 2 OnReset, 4 OnExit, 8 Tick,
// 0x10 Refresh, 0x20 RequestStateExit; the first four compose their delegate's function name from the sub-state's
// m_StateSuffix, the last two let the sub-state compose it. This is the call that makes every
// <Kind>Callback_<SubState> native of a UDisBehavior* subclass reachable.
void UDishonoredAIBehavior::RegisterCallbacks( UDisAISubState* const _pAISubState, DWORD _Flags )
{
	if( !_pAISubState )
	{
		return;
	}
	const FString Suffix = _pAISubState->m_StateSuffix.GetNameString();
	if( _Flags & 0x01 )
	{
		_pAISubState->RegisterDelegate_OnEnterCallback( this, Suffix );
	}
	if( _Flags & 0x02 )
	{
		_pAISubState->RegisterDelegate_OnResetCallback( this, Suffix );
	}
	if( _Flags & 0x04 )
	{
		_pAISubState->RegisterDelegate_OnExitCallback( this, Suffix );
	}
	if( _Flags & 0x08 )
	{
		_pAISubState->RegisterDelegate_TickCallback( this, Suffix );
	}
	if( _Flags & 0x10 )
	{
		_pAISubState->RegisterDelegate_RefreshCallback( this );
	}
	if( _Flags & 0x20 )
	{
		_pAISubState->RegisterDelegate_RequestStateExitCallback( this );
	}
}

// DISHONORED(port): 2013 rva 0x6efc80 (2012 0x74f1d0): the action target was destroyed, so it is cleared.
void UDishonoredAIBehavior::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( m_pActionTargetActor && m_pActionTargetActor == (AActor*)_rEvent.m_pInstigator )
	{
		ClearActionTarget();
	}
}

// DISHONORED(port): 2013 rva 0x6f0480 (2012 0x74f8e0)
void UDishonoredAIBehavior::BeginDestroy()
{
	Super::BeginDestroy();
}
