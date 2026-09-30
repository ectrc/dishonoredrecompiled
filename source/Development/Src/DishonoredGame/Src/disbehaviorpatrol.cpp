// DishonoredGame/src/disbehaviorpatrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31), 2012 rvas:
//   0x723ca0  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorPause(unsigned int)
//   0x72be30  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72be60  public: virtual void __thiscall UDisBehaviorPatrol::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x730e00  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x734040  public: __thiscall UDisBehaviorPatrol::UDisBehaviorPatrol(void)
//   0x73e560  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorResume(void)
//   0x73e5f0  private: unsigned int __thiscall UDisBehaviorPatrol::ChooseNewRoute(void)
//   0x73e710  protected: void __thiscall UDisBehaviorPatrol::SetGuardPoint(class ADishonoredNavPoint *)
//   0x73e800  protected: void __thiscall UDisBehaviorPatrol::ResetPatrol(void)
//   0x73e8f0  public: virtual void __thiscall UDisBehaviorPatrol::TickCallback_TakeActorPosition(class UDishonoredNativeState *, float)
//   0x73e990  private: virtual void __thiscall UDisBehaviorPatrol::OnPostGameLoad(unsigned int, unsigned int)
//   0x73ec30  private: unsigned int __thiscall UDisBehaviorPatrol::FilterPatrolRequest(struct FAIStimStruct_PatrolRequest const &)
//   0x741850  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorStart(void)
//   0x741860  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorStop(unsigned int)
//   0x7418a0  private: void __thiscall UDisBehaviorPatrol::FindNextPoint(void)
//   0x741a50  private: void __thiscall UDisBehaviorPatrol::TickGuarding(float)
//   0x742c70  private: void __thiscall UDisBehaviorPatrol::OnReachedDestination(class AActor * const, unsigned int)
//   0x742d50  public: virtual void __thiscall UDisBehaviorPatrol::RequestStateExitCallback_TakeActorPosition(class UDishonoredNativeState *)
//   0x742e20  public: virtual void __thiscall UDisBehaviorPatrol::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x743a70  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetFilterStimDelegate(enum EAIStimID)

/*-----------------------------------------------------------------------------
	agent EP (PHASE14 EP): the behaviour that makes a guard walk.

	Every part of the road to here was already standing and none of it was moving. Measured on L_Tower_P at HEAD
	2d08c15 with -disai: 26 NPC pawns, 26 initialised brains, every one of them in DisBehaviorIdle/DisAISubStateStand,
	0.0 uu moved, AND "EAIStimID_PatrolRequest(71)=8" - eight of the level's spawners ask their NPC to patrol, the stim
	reaches the brain, and UDishonoredAIBrain::ProcessOneStim drops it because no behaviour's evaluate mask covers id 71.
	This file is that mask and what happens after it.

	The loop, in retail's own order:
	  * the stim activates the behaviour and OnBehaviorStart -> ResetPatrol
	  * ResetPatrol either walks to the actor the stim named (slot 0) or asks the patrol manager for a route
	  * ChooseNewRoute adopts a route and walks to the point it joined at (slot 1)
	  * arriving raises EAIStimID_DestinationReached, the sub-state machine leaves TakeActorPosition, and
	    RequestStateExitCallback_TakeActorPosition -> OnReachedDestination fires the Kismet
	    DisSeqEvent_PatrolPointReached on the point and then either stands at it (a guard post) or FindNextPoint
	  * FindNextPoint asks the route for the next index, flips direction at the end of a there-and-back route, and
	    after m_PatrolAttentionSpan whole repetitions of the route asks for a different one

	What is NOT here, each with what it costs:
	  * OnPostGameLoad (2013 rva 0x6f3af0). It is not an override of anything this tree declares - retail's
	    UDishonoredAIBehavior has an OnPostGameLoad(UBOOL,UBOOL) virtual that this tree does not - and adding a virtual
	    to that class for one behaviour would change the vtable the whole AI save stack was measured against. A patrol
	    restored from a save therefore does not re-adopt its route; it is re-requested by the spawner instead.
	  * the ambient-animation and watch-point sub-processes. UDisAISubProcessAmbientAnims::InduceAmbientAnim (called by
	    OnEnterCallback_Stand when the post says the direction matters) and the whole of
	    UDisAISubProcessWatchPoints beyond its three accessors are unported skeleton units. A guard standing at a post
	    stands still instead of playing an idle animation and looking at each watch point in turn.
	  * the OnBehaviorPause anim stop. It reads FDisComponentAnimPlayer, which this tree does not have.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "dishonoredutilities.h"

/** The map's patrol manager, or NULL. Retail spells these four lines out at every use (ChooseNewRoute,
    EvaluatePatrolRequest, ADishonoredRoute's four). */
static UDisPatrolManager* DisGetPatrolManager()
{
	UDishonoredMapInfo* MapInfo = GWorld && GWorld->GetWorldInfo() ? Cast<UDishonoredMapInfo>( GWorld->GetWorldInfo()->GetMapInfo() ) : NULL;
	return MapInfo ? MapInfo->m_pPatrolManager : NULL;
}

/*-----------------------------------------------------------------------------
	The one stim
-----------------------------------------------------------------------------*/

// DISHONORED(port): the mask. Retail's UDisBehaviorPatrol::BuildEvaluateStimMask is folded onto another class's - any
// class whose mask is exactly {PatrolRequest} produces the same 0x100 bytes - so its own address is not recoverable.
// Its CONTENT is: GetEvaluateStimDelegate (2013 rva 0x6ef140) binds a delegate for id 71 and for nothing else, and
// UDishonoredAIBrain::ProcessOneStim needs both the mask bit and a bound delegate, so a mask bit for any other id would
// be inert. Same argument for the filter mask against GetFilterStimDelegate (0x6fea40).
const BYTE* UDisBehaviorPatrol::BuildEvaluateStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_PatrolRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

const BYTE* UDisBehaviorPatrol::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_PatrolRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2013 rva 0x6ef140 (2012 0x730e00)
FDisStimPredicateDelegate UDisBehaviorPatrol::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolRequest )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorPatrol, FAIStimStruct_PatrolRequest, EvaluatePatrolRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6fea40 (2012 0x743a70)
FDisStimPredicateDelegate UDisBehaviorPatrol::GetFilterStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolRequest )
	{
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorPatrol, FAIStimStruct_PatrolRequest, FilterPatrolRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6e99f0 (2012 0x72be30)
FDisStimSetupDelegate UDisBehaviorPatrol::GetSetupFromStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_PatrolRequest )
	{
		return DIS_BIND_STIM_SETUP( UDisBehaviorPatrol, FAIStimStruct_PatrolRequest, SetupFromPatrolRequest );
	}
	return FDisStimSetupDelegate();
}

// DISHONORED(port): retail's body is ICF-folded with UDisBehaviorPatrolSearch::EvaluatePatrolSearchRequest (2013 rva
// 0x6e99b0), which is what the folded call at 0x6ebbf0 resolves to, so the two are byte-identical: a patrol request is
// worth taking only if the map has at least one route registered. Note this is the ONLY thing evaluated - the stim's
// own m_pStartingActor is not looked at here, so a request that names an actor is still refused on a map with no
// routes at all.
UBOOL UDisBehaviorPatrol::EvaluatePatrolRequest( const FAIStimStruct_PatrolRequest& _rStim ) const
{
	UDisPatrolManager* PatrolManager = DisGetPatrolManager();
	return PatrolManager ? PatrolManager->HasPatrolRoutes() : FALSE;
}

// DISHONORED(port): 2013 rva 0x6f3db0 (2012 0x73ec30): a patrol request arriving at a behaviour that is ALREADY
// patrolling re-points it - the new starting actor replaces the old one and the patrol starts over - and then filters
// the stim out, which is what stops ProcessOneStim from activating it a second time.
UBOOL UDisBehaviorPatrol::FilterPatrolRequest( const FAIStimStruct_PatrolRequest& _rStim )
{
	m_pStartingActor = _rStim.m_pStartingActor;
	ResetPatrol();
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x6e3fa0 (2012 0x724180), reached through the folded delegate thunk 0x6e55f0: the setup
// delegate runs before OnBehaviorStart, so this is where m_pStartingActor is read for a FIRST activation.
void UDisBehaviorPatrol::SetupFromPatrolRequest( const FAIStimStruct_PatrolRequest& _rStim )
{
	m_pStartingActor = _rStim.m_pStartingActor;
}

/*-----------------------------------------------------------------------------
	The two sub-state requests, spelled once each
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x741c50, the FDisAISubStateTakeActorPosition_Param constructor as retail's three callers
// build it: the stop type is 0 for a guard post (stop exactly on it, because the post's rotation matters) and 2
// otherwise, never at full speed, and with no minimum distance.
void UDisBehaviorPatrol::RequestTakeActorPosition( BYTE _Slot, AActor* _pPoint )
{
	ADishonoredNavPoint* NavPoint = Cast<ADishonoredNavPoint>( _pPoint );
	const BYTE StopType = ( NavPoint && NavPoint->IsGuardPoint() ) ? 0 : 2;
	FDisAISubStateTakeActorPosition_Param Param( _pPoint, StopType, FALSE, 0.f );
	RequestSubStateChange< UDisTweaks_AIBehavior_Patrol, UDisTweaks_AISubState_TakeActorPosition >( _Slot, Param );
}

// DISHONORED(port): slot 3, the "there is nowhere to patrol" answer: stand exactly where the pawn already is, in
// whatever body stance it already has. Retail builds it with sub_B38890, the FDisAISubStateStand_Param constructor, at
// 2013 rva 0x738890.
void UDisBehaviorPatrol::RequestStandWhereIAm()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}
	FDisAISubStateStand_Param Param( Pawn->Location, Pawn->Rotation, FALSE, FALSE, FALSE, Pawn->GetBodyStance() );
	RequestSubStateChange< UDisTweaks_AIBehavior_Patrol, UDisTweaks_AISubState_Stand >( 3, Param );
}

/*-----------------------------------------------------------------------------
	The patrol itself
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6fa6e0 (2012 0x741850), a jump to ResetPatrol in both builds.
void UDisBehaviorPatrol::OnBehaviorStart()
{
	ResetPatrol();
}

// DISHONORED(port): 2013 rva 0x6fa6f0 (2012 0x741860): the route is given back so another NPC can have it.
void UDisBehaviorPatrol::OnBehaviorStop( UBOOL _bIsBeingTerminated )
{
	SetGuardPoint( NULL );
	if( m_pCurrentRoute )
	{
		m_pCurrentRoute->RemoveAdoption();
		m_pCurrentRoute = NULL;
	}
	m_pStartingActor = NULL;
}

// DISHONORED(port): 2013 rva 0x6f3960 (2012 0x73e800): start, or start again. An NPC handed a starting actor walks to
// that; otherwise it adopts a route, and if it cannot it stands where it is rather than doing nothing, which is what
// keeps the behaviour holding its slot.
void UDisBehaviorPatrol::ResetPatrol()
{
	m_RouteDirection = ERD_Forward;
	if( m_pCurrentRoute )
	{
		m_pCurrentRoute->RemoveAdoption();
		m_pCurrentRoute = NULL;
	}
	SetGuardPoint( NULL );

	if( m_pStartingActor )
	{
		RequestTakeActorPosition( 0, m_pStartingActor );
		return;
	}
	if( !ChooseNewRoute() )
	{
		RequestStandWhereIAm();
	}
}

// DISHONORED(port): 2013 rva 0x6f3750 (2012 0x73e5f0): ask the manager for a route and walk to the point it says to
// join at. m_Repetitions is reset whether or not a route was found, which is retail's order.
UBOOL UDisBehaviorPatrol::ChooseNewRoute()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	UDisPatrolManager* PatrolManager = DisGetPatrolManager();
	UBOOL bAdopted = FALSE;
	if( PatrolManager && Pawn )
	{
		bAdopted = PatrolManager->AdoptNewRoute( Pawn, Pawn->Location, &m_pCurrentRoute, &m_CurrentIndex );
	}
	m_Repetitions = 0;
	if( !bAdopted || !m_pCurrentRoute )
	{
		return FALSE;
	}

	m_StartingIndex = m_CurrentIndex;
	m_RouteDirection = ERD_Forward;
	if( m_CurrentIndex < 0 || m_CurrentIndex >= m_pCurrentRoute->RouteList.Num() )
	{
		return bAdopted;
	}
	RequestTakeActorPosition( 1, m_pCurrentRoute->RouteList( m_CurrentIndex ).Actor );
	return bAdopted;
}

// DISHONORED(port): 2013 rva 0x6fa730 (2012 0x7418a0): one step along the route.
//   * the next index is the current one plus or minus one, or a fresh random one on a random route
//   * the route resolves it, and may say "the route is over" (bComplete) or "turn round" (bReverse)
//   * arriving back at the index the NPC joined at counts one repetition, and m_PatrolAttentionSpan repetitions ask
//     the manager for a different route
//   * an index that did not change means there is nowhere new to go, so nothing is requested
void UDisBehaviorPatrol::FindNextPoint()
{
	if( !m_pCurrentRoute )
	{
		ResetPatrol();
		return;
	}

	const INT PreviousIndex = m_CurrentIndex;
	INT NextIndex = m_CurrentIndex;
	if( m_pCurrentRoute->RouteType == 3 )
	{
		NextIndex = m_NonRepeatINTRandomHelper.GetNonRepeatingRandomValue( m_pCurrentRoute->RouteList.Num() );
	}
	else if( m_RouteDirection == ERD_Reverse )
	{
		NextIndex--;
	}
	else if( m_RouteDirection == ERD_Forward )
	{
		NextIndex++;
	}

	BYTE bComplete = 0;
	BYTE bReverse = 0;
	m_CurrentIndex = m_pCurrentRoute->ResolveRouteIndex( NextIndex, m_RouteDirection, bComplete, bReverse );
	if( bComplete || m_CurrentIndex < 0 )
	{
		m_CurrentIndex = PreviousIndex;
		return;
	}
	if( bReverse )
	{
		m_RouteDirection = ( m_RouteDirection == ERD_Forward ) ? ERD_Reverse : ERD_Forward;
	}
	if( m_CurrentIndex == m_StartingIndex )
	{
		m_Repetitions++;
		const INT AttentionSpan = m_pCurrentRoute->m_PatrolAttentionSpan;
		if( m_Repetitions >= AttentionSpan && AttentionSpan >= 0 && ChooseNewRoute() )
		{
			OnNewRouteChosen();
			return;
		}
	}
	if( m_CurrentIndex == PreviousIndex )
	{
		return;
	}
	if( m_CurrentIndex < 0 || m_CurrentIndex >= m_pCurrentRoute->RouteList.Num() )
	{
		return;
	}
	RequestTakeActorPosition( 1, m_pCurrentRoute->RouteList( m_CurrentIndex ).Actor );
}

// DISHONORED(port): 2013 rva 0x6ec9e0 (2012 0x72bed0), UDisBehaviorPatrol's UObject vtable slot 110 - the call
// FindNextPoint makes after swapping routes. The whole of it is GetSubProcess( UDisAISubProcessAmbientBarks ) and a tail
// jump to UDisAISubProcessAmbientBarks::ScheduleRouteSwitchBark, whose 59 bytes are at 2013 0x7398a0 (IDA attributes
// them as a tail chunk of 0x6ec9e0 rather than as their own function, which is why match_2012_2013.csv maps 0x6ec9e0 to
// the 2012 ScheduleRouteSwitchBark instead of to OnNewRouteChosen). It ends
//   or dword ptr [esi+68h], 1      ; m_bDoRouteSwitchBark = TRUE
//   fstp dword ptr [esi+6Ch]       ; m_fBarkInhibitionTimer = tweaks->m_fSilenceBeforeRouteSwitchBark (tweaks+144)
// so a guard that starts a different beat says something about it, after a silence.
// DISHONORED(bringup): disaisubprocessambientbarks.cpp is a comment-only skeleton, so there is no timer to reset. The
// virtual exists because UDisBehaviorPatrolSearch overrides it (its own slot 110 is a separate, ICF-folded body), and
// because calling ResetPatrol here instead - which is what this file did before the vtable was read - drops the route
// ChooseNewRoute has just adopted.
void UDisBehaviorPatrol::OnNewRouteChosen()
{
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( TEXT("DISHONORED(bringup): UDisBehaviorPatrol::OnNewRouteChosen: UDisAISubProcessAmbientBarks is unported, so a guard starting a new route says nothing") );
	}
}

// DISHONORED(port): 2013 rva 0x6f3870 (2012 0x73e710): take up, or give up, a guard post. The post's own rotation is
// used only when it says the direction matters; otherwise the guard keeps the rotation it arrived with.
void UDisBehaviorPatrol::SetGuardPoint( ADishonoredNavPoint* _pGuardPoint )
{
	m_pCurrentGuardPoint = _pGuardPoint;
	m_fGuardTimer = 0.f;

	UDisAISubProcessWatchPoints* WatchPoints = (UDisAISubProcessWatchPoints*)GetSubProcess( UDisAISubProcessWatchPoints::StaticClass() );
	if( !m_pCurrentGuardPoint )
	{
		if( WatchPoints )
		{
			WatchPoints->StopWatchingPoints();
		}
		return;
	}

	// DISHONORED(bringup): retail calls UDisAISubProcessWatchPoints::StartWatchingPoints (2013 rva 0x73a9d0, unnamed
	// there) and falls back to the post's own m_fGuardDuration only when the post names no watch point. That function
	// is in the unported half of disaisubprocesswatchpoints.cpp, so the fallback is always taken - see the cpptext.
	m_fGuardTimer = m_pCurrentGuardPoint->m_fGuardDuration;

	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}
	const AActor* RotationSource = m_pCurrentGuardPoint->m_bDirectionMatters ? (const AActor*)m_pCurrentGuardPoint : (const AActor*)Pawn;
	FDisAISubStateStand_Param Param( m_pCurrentGuardPoint->Location, RotationSource->Rotation,
		FALSE, FALSE, FALSE, Pawn->GetBodyStance() );
	RequestSubStateChange< UDisTweaks_AIBehavior_Patrol, UDisTweaks_AISubState_Stand >( 2, Param );
}

// DISHONORED(port): 2013 rva 0x6fa8e0 (2012 0x741a50): the clock at a guard post. While the watch-point sub-process is
// running the clock is ITS remaining duration rather than this one's, which is how a post with watch points is held
// until every point has been looked at once. m_bGuardForever holds the post whatever the clock says.
void UDisBehaviorPatrol::TickGuarding( FLOAT _fDeltaSeconds )
{
	if( !m_pCurrentGuardPoint )
	{
		SetGuardPoint( NULL );
		FindNextPoint();
		return;
	}

	m_fGuardTimer -= _fDeltaSeconds;
	UDisAISubProcessWatchPoints* WatchPoints = (UDisAISubProcessWatchPoints*)GetSubProcess( UDisAISubProcessWatchPoints::StaticClass() );
	if( WatchPoints && WatchPoints->IsWatchingPoints() )
	{
		if( !WatchPoints->HasWatchCycled() || m_pCurrentGuardPoint->m_bGuardForever )
		{
			m_fGuardTimer = WatchPoints->GetRemainingWatchDuration();
		}
		else
		{
			m_fGuardTimer = 0.f;
		}
	}
	if( m_fGuardTimer < 1.e-8f && !m_pCurrentGuardPoint->m_bGuardForever )
	{
		SetGuardPoint( NULL );
		FindNextPoint();
	}
}

// DISHONORED(port): 2013 rva 0x6fd350 (2012 0x742c70, unnamed in 2013): the point is
// announced to Kismet through DisSeqEvent_PatrolPointReached - which is what a level uses to hang a conversation or a
// door on a guard reaching a particular spot - and then the guard either stands there or moves on.
void UDisBehaviorPatrol::OnReachedDestination( AActor* _pPoint, UBOOL _bIsGuardPoint )
{
	ADishonoredNavPoint* NavPoint = Cast<ADishonoredNavPoint>( _pPoint );
	if( NavPoint )
	{
		TArray<INT> ActivateIndices;
		ActivateIndices.AddItem( 0 );
		ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
		DisFireKismetEvent( NavPoint, UDisSeqEvent_PatrolPointReached::StaticClass(), _pPoint, Pawn, FALSE, &ActivateIndices );
	}
	if( _bIsGuardPoint )
	{
		SetGuardPoint( NavPoint );
	}
	else
	{
		FindNextPoint();
	}
}

/*-----------------------------------------------------------------------------
	The sub-state callbacks. Each one switches on which of the four slots is live.
-----------------------------------------------------------------------------*/

void UDisBehaviorPatrol::execRequestStateExitCallback_TakeActorPosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_TakeActorPosition( _pThisState );
}

// DISHONORED(port): 2013 rva 0x6fd430 (2012 0x742d50): the NPC has arrived. Slot 0 means it arrived at the actor the
// stim named, which is consumed - m_pStartingActor is cleared, so the next FindNextPoint goes to a route. Slot 1 means
// it arrived at a route point. Either way OnReachedDestination decides what happens next.
void UDisBehaviorPatrol::RequestStateExitCallback_TakeActorPosition( UDishonoredNativeState* _pThisState )
{
	AActor* Point = NULL;
	const INT Slot = GetActiveSubStateIndex();
	if( Slot == 0 )
	{
		Point = m_pStartingActor;
		if( !Point )
		{
			return;
		}
		m_pStartingActor = NULL;
	}
	else if( Slot == 1 )
	{
		if( !m_pCurrentRoute || m_CurrentIndex < 0 || m_CurrentIndex >= m_pCurrentRoute->RouteList.Num() )
		{
			return;
		}
		Point = m_pCurrentRoute->RouteList( m_CurrentIndex ).Actor;
	}
	else
	{
		return;
	}
	if( !Point )
	{
		return;
	}
	ADishonoredNavPoint* NavPoint = Cast<ADishonoredNavPoint>( Point );
	OnReachedDestination( Point, ( NavPoint && NavPoint->IsGuardPoint() ) ? TRUE : FALSE );
}

void UDisBehaviorPatrol::execTickCallback_TakeActorPosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_FLOAT(_fDeltaSeconds);
	P_FINISH;
	TickCallback_TakeActorPosition( _pThisState, _fDeltaSeconds );
}

// DISHONORED(port): 2013 rva 0x6f3a50 (2012 0x73e8f0): while walking, check that what is being walked to still exists.
// Slot 0: the starting actor went away. Slot 1: the route was switched off by Kismet. Either starts the patrol over.
void UDisBehaviorPatrol::TickCallback_TakeActorPosition( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	const INT Slot = GetActiveSubStateIndex();
	if( Slot == 0 )
	{
		if( !m_pStartingActor )
		{
			ResetPatrol();
		}
		return;
	}
	if( Slot != 1 )
	{
		return;
	}
	if( m_pCurrentRoute )
	{
		if( !m_pCurrentRoute->IsRouteActive() )
		{
			ResetPatrol();
		}
		return;
	}
	if( !ChooseNewRoute() )
	{
		RequestStandWhereIAm();
	}
}

void UDisBehaviorPatrol::execTickCallback_Stand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_FLOAT(_fDeltaSeconds);
	P_FINISH;
	TickCallback_Stand( _pThisState, _fDeltaSeconds );
}

// DISHONORED(port): 2013 rva 0x6fd500 (2012 0x742e20): slot 2 is a guard post, so run its clock; slot 3 is the
// standing-still fallback, so keep asking the manager for a route - which is how an NPC that started away from every
// route joins one as soon as a route is neglected or switched on.
void UDisBehaviorPatrol::TickCallback_Stand( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	const INT Slot = GetActiveSubStateIndex();
	if( Slot == 2 )
	{
		TickGuarding( _fDeltaSeconds );
	}
	else if( Slot == 3 )
	{
		ChooseNewRoute();
	}
}

void UDisBehaviorPatrol::execOnEnterCallback_Stand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pLastState);
	P_FINISH;
	OnEnterCallback_Stand( _pThisState, _pLastState );
}

// DISHONORED(port): 2013 rva 0x6ec970 (2012 0x72be60): arriving at a guard post whose direction matters plays one
// ambient animation, and only when the state before it was TakeActorPosition - i.e. on arrival and not on a re-entry.
// DISHONORED(bringup): UDisAISubProcessAmbientAnims is a skeleton unit, so the animation is not induced.
void UDisBehaviorPatrol::OnEnterCallback_Stand( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pLastState )
{
	if( !_pLastState || !_pLastState->IsA( UDisAISubStateTakeActorPosition::StaticClass() ) )
	{
		return;
	}
	if( !m_pCurrentGuardPoint || !m_pCurrentGuardPoint->m_bDirectionMatters )
	{
		return;
	}
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( TEXT("DISHONORED(bringup): UDisBehaviorPatrol::OnEnterCallback_Stand: UDisAISubProcessAmbientAnims::InduceAmbientAnim is unported, so a guard post plays no idle animation") );
	}
}

/*-----------------------------------------------------------------------------
	Pause and resume
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6e3f60 (2012 0x723ca0): a patrol interrupted by something else stops the idle animation
// it was playing, if it was playing the one the patrol induced (channel 0, anim id 66).
// DISHONORED(bringup): FDisComponentAnimPlayer is not ported, so there is nothing to stop.
void UDisBehaviorPatrol::OnBehaviorPause( UBOOL _bIsBeingTerminated )
{
}

// DISHONORED(port): 2013 rva 0x6f36c0 (2012 0x73e560): coming back to a patrol re-states the patrol look-at pattern -
// the head sweep a walking guard does - and re-enables the ambient-animation sub-process unless the tweaks turn it off.
void UDisBehaviorPatrol::OnBehaviorResume()
{
	SetLookAtProceduralDesire( DisLookAtProceduralPattern_Patrol, FDisLookAtInfluence::Torso, -1.f );

	const UDisTweaks_AIBehavior_Patrol* Tweaks = Cast<UDisTweaks_AIBehavior_Patrol>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AIBehavior_Patrol*)UDisTweaks_AIBehavior_Patrol::StaticClass()->GetDefaultObject();
	}
	if( Tweaks->m_bDisableAmbientAnim )
	{
		UDisAISubProcess* AmbientAnims = GetSubProcess( UDisAISubProcessAmbientAnims::StaticClass() );
		if( AmbientAnims )
		{
			AmbientAnims->DisableSubProcess_Internal( m_bIsPaused );
		}
	}
}
