// DishonoredGame/src/disaisubstategenericaction.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateGenericAction and its two parameters ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	The two parameters

	DISHONORED(retail): one sub-state class, two parameter types. The full form names an action and overrides the slot's
	tweaks with it; the _Nothing form names only a group and leaves the action to the tweaks, which is how a behaviour
	parks an NPC in an idle animation group without choosing the animation.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x77bbb0
FDisAISubStateGenericAction_Param::FDisAISubStateGenericAction_Param( BYTE _ActionID, UBOOL _bUseDualPlayAnim, INT _StartingAnimStep )
	: FDisAISubState_Param( UDisAISubStateGenericAction::StaticClass() )
	, m_OverriddenActionID( _ActionID )
	, m_StartingAnimStep( _StartingAnimStep )
	, m_bUseDualPlayAnim( _bUseDualPlayAnim )
{
}

// DISHONORED(port): 2012 rva 0x768d50. m_GroupIdx is set to INDEX_NONE, i.e. "pick your own group".
void FDisAISubStateGenericAction_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateGenericAction* GenericAction = (UDisAISubStateGenericAction*)PendingState;
	GenericAction->m_ActionID = m_OverriddenActionID;
	GenericAction->m_GroupIdx = INDEX_NONE;
	GenericAction->m_ChainStep = m_StartingAnimStep;
	GenericAction->m_bUseDualPlayAnim = m_bUseDualPlayAnim ? 1 : 0;
}

// DISHONORED(port): 2012 rva 0x77bc10
FDisAISubStateGenericAction_Nothing_Param::FDisAISubStateGenericAction_Nothing_Param( INT _iCustomGroup )
	: FDisAISubState_Param( UDisAISubStateGenericAction::StaticClass() )
	, m_iCustomGroup( _iCustomGroup )
{
}

// DISHONORED(port): 2012 rva 0x768db0: only the group; m_ActionID keeps whatever the slot's tweaks applied.
void FDisAISubStateGenericAction_Nothing_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
	( (UDisAISubStateGenericAction*)PendingState )->m_GroupIdx = m_iCustomGroup;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateGenericAction

	The sub-state's own four-value state machine (m_GenericActionState) is the whole logic: 0 = ask for the action,
	1 = playing it, 2 = it finished, 3 = it failed or was cut short. TickState drives it and every one of the three
	terminal values leaves the sub-state.
-----------------------------------------------------------------------------*/

enum { DIS_GENERICACTION_REQUEST = 0, DIS_GENERICACTION_PLAYING = 1, DIS_GENERICACTION_ENDED = 2, DIS_GENERICACTION_FAILED = 3 };

/** DISHONORED(written): the FArkGameEvent type retail passes as the literal 8 at all four of this unit's call sites - the
    per-object event an NPC pawn raises when one of its anim actions ends. It is a DisGameEventType, not one of the four
    engine-side Ark ones, and the enumeration has no name in the tree yet (Engine/Inc/arkgameeventdispatcher.h). */
static const INT GDisAIEvent_NpcHActionEnded = 8;

// DISHONORED(port): 2012 rva 0x7654e0. Three stims, all "something interrupted you".
const BYTE* UDisAISubStateGenericAction::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_EndPossession, EAIStimID_EvadedMelee_Incoming, EAIStimID_IncomingDamage };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x770e20
FDisStimPredicateDelegate UDisAISubStateGenericAction::GetFilterStimDelegate_SubState( BYTE StimID )
{
	if( StimID == EAIStimID_EndPossession || StimID == EAIStimID_EvadedMelee_Incoming || StimID == EAIStimID_IncomingDamage )
	{
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateGenericAction, FAIStimStruct, FilterIncomingAttack );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x765510: an action that is interrupted has failed, not ended.
UBOOL UDisAISubStateGenericAction::FilterIncomingAttack( const FAIStimStruct& _rStim )
{
	RequestStateExit();
	m_GenericActionState = DIS_GENERICACTION_FAILED;
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x787de0: any stale subscription from a previous run is dropped, the give-up clock is armed
// from the slot's tweaks, and the state machine starts at "ask".
void UDisAISubStateGenericAction::BeginSubState_Derived()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher )
	{
		const INT EventType = GDisAIEvent_NpcHActionEnded;
		Dispatcher->UnregisterToObjectEvent( EventType, (void*)Pawn, this, &UDisAISubStateGenericAction::OnNpcHActionEnded );
	}

	const UDisTweaks_AISubState_GenericAction* Tweaks = Cast<UDisTweaks_AISubState_GenericAction>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_GenericAction*)UDisTweaks_AISubState_GenericAction::StaticClass()->GetDefaultObject();
	}
	m_GenericActionState = DIS_GENERICACTION_REQUEST;
	m_fGiveUpTimer = Tweaks->m_fTimeBeforeGivingUp;
}

// DISHONORED(port): 2012 rva 0x787e60. When the slot says to take its values, the action id and the dual-play flag come
// from the tweaks rather than from the parameter; an action id of 0 is "no action", and the request is refused.
// DISHONORED(bringup): the request itself is a state change on the PAWN's own FSM with an FStateNPCPlayAnim_Param (2012
// rva 0x6c4b20) or its dual form. The anim-state parameter types are agent AV's package (UDisAnimStatePool and the
// UStateNPC* family) and are not ported, so the animation is never started - which is why m_GenericActionState then runs
// its give-up clock down and the sub-state leaves with FAILED rather than ENDED. Everything else here is faithful.
UBOOL UDisAISubStateGenericAction::RequestActionToPlay()
{
	UBOOL bUseDualPlayAnimFromTweaks = FALSE;
	if( m_bFetchTweaksValues )
	{
		const UDisTweaks_AISubState_GenericAction* Tweaks = Cast<UDisTweaks_AISubState_GenericAction>( GetTweaks_Derived() );
		if( !Tweaks )
		{
			Tweaks = (const UDisTweaks_AISubState_GenericAction*)UDisTweaks_AISubState_GenericAction::StaticClass()->GetDefaultObject();
		}
		m_ActionID = Tweaks->m_ActionID;
		bUseDualPlayAnimFromTweaks = Tweaks->m_bForceVisionForward != 0;
	}
	if( m_ActionID == 0 )
	{
		return FALSE;
	}

	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FStateNPCPlayAnim_Param is not ported (agent AV's anim states); a generic action is requested and never plays") );
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x784e10: the subscription is dropped, and an action that was still playing is cut off by
// asking the pawn's own anim state to exit - but only if that state is still playing OUR action chain, so two NPCs
// sharing an animation do not cancel each other.
void UDisAISubStateGenericAction::KillAction()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher )
	{
		const INT EventType = GDisAIEvent_NpcHActionEnded;
		Dispatcher->UnregisterToObjectEvent( EventType, (void*)Pawn, this, &UDisAISubStateGenericAction::OnNpcHActionEnded );
	}
	if( m_GenericActionState == DIS_GENERICACTION_PLAYING )
	{
		// DISHONORED(bringup): retail casts the pawn's currently active NPC-master state to UStateNPCMasterPlayAnim and
		// compares its m_AnimChainToPlay with m_ActionID (agent AV's anim states, unported).
		m_GenericActionState = DIS_GENERICACTION_FAILED;
	}
}

// DISHONORED(port): 2012 rva 0x768df0: the pawn's anim state reports which chain ended, and only our own counts.
void UDisAISubStateGenericAction::OnNpcHActionEnded( const FArkGameEvent& _rEvent )
{
	const INT* EndedActionID = (const INT*)_rEvent.m_pEventParams;
	if( EndedActionID && *EndedActionID == (INT)m_ActionID )
	{
		RequestStateExit();
		m_GenericActionState = DIS_GENERICACTION_ENDED;
	}
}

// DISHONORED(port): 2012 rva 0x785cb0: pausing kills the action; a pause that is not a termination and whose action had
// not already ended counts as a failure, so the behaviour is told the action did not happen.
void UDisAISubStateGenericAction::PauseSubState_Derived( UBOOL bIsBeingTerminated )
{
	KillAction();
	if( !bIsBeingTerminated && m_GenericActionState != DIS_GENERICACTION_ENDED )
	{
		RequestStateExit();
		m_GenericActionState = DIS_GENERICACTION_FAILED;
	}
}

// DISHONORED(port): 2012 rva 0x785ce0
void UDisAISubStateGenericAction::OnExitState( UDishonoredNativeState* NextState )
{
	KillAction();
	Super::OnExitState( NextState );
}

// DISHONORED(port): 2012 rva 0x78b660: ask, then wait, then leave. The give-up clock only runs while the request is still
// being refused, which is what stops a missing animation from wedging the behaviour.
void UDisAISubStateGenericAction::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( m_GenericActionState != DIS_GENERICACTION_REQUEST )
	{
		if( m_GenericActionState == DIS_GENERICACTION_ENDED || m_GenericActionState == DIS_GENERICACTION_FAILED )
		{
			RequestStateExit();
		}
		return;
	}

	if( RequestActionToPlay() )
	{
		m_GenericActionState = DIS_GENERICACTION_PLAYING;
		FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
		if( Dispatcher )
		{
			const INT EventType = GDisAIEvent_NpcHActionEnded;
			Dispatcher->RegisterToObjectEvent( EventType, (void*)m_pOwningBrain->m_pOwningPawn, this, &UDisAISubStateGenericAction::OnNpcHActionEnded );
		}
		return;
	}

	m_fGiveUpTimer -= DeltaSeconds;
	if( m_fGiveUpTimer < 0.f )
	{
		m_GenericActionState = DIS_GENERICACTION_FAILED;
	}
}

// DISHONORED(port): 2012 rva 0x768e20: the mask is a raw pointer into per-class static data, so it is rebuilt, and the
// action starts over from "ask" because the animation it was playing is not in the save.
void UDisAISubStateGenericAction::PostGameLoad_GenericAction()
{
	PostGameLoad_SubState();
	m_GenericActionState = DIS_GENERICACTION_REQUEST;
}
