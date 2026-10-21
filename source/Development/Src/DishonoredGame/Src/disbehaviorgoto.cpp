// DishonoredGame/src/disbehaviorgoto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28), 2012 rvas:
//   0x745480  private: virtual enum eDisDialogAvailability __thiscall UDisBehaviorGoTo::GetDialogAvailability(enum eDisConvPriority, int)const
//   0x7454a0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterInterruptStim(struct FAIStimStruct const &)
//   0x7454b0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterTouchedAlly(struct FAIStimStruct_TouchedAlly const &)
//   0x7454e0  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildShouldFinishWhileDormantStimMask(void)const
//   0x745510  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildEvaluateStimMask(void)const
//   0x746bc0  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildFilterStimMask(void)const
//   0x746cb0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterKismetAIAction(struct FAIStimStruct const &)
//   0x748e60  private: unsigned int __thiscall UDisBehaviorGoTo::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x748e90  private: virtual unsigned int __thiscall UDisBehaviorGoTo::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x74c7a0  private: virtual void __thiscall UDisBehaviorGoTo::InitBehavior(class UDishonoredAIBrain * const)
//   0x74c7d0  private: virtual void __thiscall UDisBehaviorGoTo::OnBehaviorStart(void)
//   0x74c850  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x756e80  public: static class UClass * __cdecl UDisBehaviorGoTo::GetPrivateStaticClassUDisBehaviorGoTo(wchar_t const *)
//   0x75aca0  public: static void __cdecl UDisBehaviorGoTo::InitializePrivateStaticClassUDisBehaviorGoTo(void)
//   0x75acc0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TakePosition::GetPrivateStaticClassUDisTweaks_AIBehavior_TakePosition(wchar_t const *)
//   0x75bc50  public: static class UClass * __cdecl UDisBehaviorGoTo::StaticClassNoInline(void)
//   0x75bc80  public: static void __cdecl UDisTweaks_AIBehavior_TakePosition::InitializePrivateStaticClassUDisTweaks_AIBehavior_TakePosition(void)
//   0x75c120  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TakePosition::StaticClassNoInline(void)
//   0x75c150  private: unsigned int __thiscall UDisBehaviorGoTo::FilterBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)
//   0x75c190  private: unsigned int __thiscall UDisBehaviorGoTo::ShouldFinishWhileDormantBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)const
//   0x75d0f0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x75d250  private: unsigned int __thiscall UDisBehaviorGoTo::ShouldFinishWhileDormantGoToRequest(struct FAIStimStruct_GoToRequest const &)const
//   0x75d2a0  private: unsigned int __thiscall UDisBehaviorGoTo::EvaluateGoToRequest(struct FAIStimStruct_GoToRequest const &)const
//   0x75d2f0  private: void __thiscall UDisBehaviorGoTo::SetupFromGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x760eb0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetFilterStimDelegate(enum EAIStimID)
//   0x761040  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x761080  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x7610b0  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetSetupFromStimDelegate(enum EAIStimID)

/*-----------------------------------------------------------------------------
	agent FV (PHASE18 FV): the Kismet go-to behaviour.

	The brain's ProcessOneStim needs both a mask bit and a bound delegate for a stim to reach a behaviour, so the three
	masks below are the names in the three delegate accessors and nothing else. The behaviour tweaks are
	UDisTweaks_AIBehavior_TakePosition (m_bRespondToUninterruptableStims, m_bAccurateStop, m_bExactRotation), read through
	GetTweaks_Derived with the class default as the fallback, as every Dis behaviour does.

	A go-to request is WANTED when the action's m_bInterruptable differs from the tweaks' m_bRespondToUninterruptableStims:
	an interruptable request for an ordinary go-to, an uninterruptable one for the behaviour instance tweaked to take
	those. That is the one test Evaluate, Filter and ShouldFinishWhileDormant share.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "dishonoredutilities.h"

static UDisTweaks_AIBehavior_TakePosition* DisGoToTweaks( UDisBehaviorGoTo* _pBehavior )
{
	UDisTweaks_AIBehavior_TakePosition* Tweaks = Cast<UDisTweaks_AIBehavior_TakePosition>( _pBehavior->GetTweaks_Derived() );
	return Tweaks ? Tweaks : UDisTweaks_AIBehavior_TakePosition::StaticClass()->GetDefaultObject<UDisTweaks_AIBehavior_TakePosition>();
}

static UBOOL DisGoToRequestWanted( UDisBehaviorGoTo* _pBehavior, const FAIStimStruct_GoToRequest& _rStim )
{
	const UDisTweaks_AIBehavior_TakePosition* Tweaks = DisGoToTweaks( _pBehavior );
	const UBOOL bRespondToUninterruptable = Tweaks->m_bRespondToUninterruptableStims ? TRUE : FALSE;
	const UBOOL bInterruptable = _rStim.m_pAssociatedKismetGoToAction && _rStim.m_pAssociatedKismetGoToAction->m_bInterruptable ? TRUE : FALSE;
	return bRespondToUninterruptable != bInterruptable;
}

/** DISHONORED(port): the shared "yes" predicate retail calls s_DelegateReturnTrue, as disbehavioridle.cpp spells it. */
static UBOOL DisStimAlwaysTrue( void* /*_pObject*/, const FAIStimStruct& /*_rStim*/ )
{
	return TRUE;
}

static const FDisStimPredicateDelegate GDisDelegateReturnTrue( NULL, &DisStimAlwaysTrue );

/*-----------------------------------------------------------------------------
	The masks and the delegates
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6e4d10 (2012 0x745510): the one stim that activates the behaviour.
const BYTE* UDisBehaviorGoTo::BuildEvaluateStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_GoToRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x746bc0: the base's three plus the nineteen the switch in GetFilterStimDelegate names - the
// eight stims that interrupt a go-to, the Kismet AI actions that replace it, the two touch stims, the abort, and the two
// distractions the behaviour swallows.
const BYTE* UDisBehaviorGoTo::BuildFilterStimMask()
{
	static const BYTE StimIDs[] =
	{
		EAIStimID_DestinationReached, EAIStimID_RotationReached, EAIStimID_PathingFail,
		EAIStimID_Alarm, EAIStimID_AttackedByEnemy, EAIStimID_DiscoveredCorpse, EAIStimID_EvadedMelee_Incoming,
		EAIStimID_HeardSomething, EAIStimID_Help, EAIStimID_SearchRequest, EAIStimID_WitnessDeath,
		EAIStimID_BehaviorAbort, EAIStimID_Distracted_Anim, EAIStimID_DocileRatIsNear,
		EAIStimID_FollowRequest, EAIStimID_GuardRequest, EAIStimID_PatrolRequest, EAIStimID_ShootRequest,
		EAIStimID_GoToRequest, EAIStimID_TouchedAlly, EAIStimID_TouchedEnemy, EAIStimID_TouchedNeutral,
	};
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x7454e0
const BYTE* UDisBehaviorGoTo::BuildShouldFinishWhileDormantStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_GoToRequest, EAIStimID_BehaviorAbort };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2013 rva 0x6fc200 (2012 0x761080)
FDisStimPredicateDelegate UDisBehaviorGoTo::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_GoToRequest )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorGoTo, FAIStimStruct_GoToRequest, EvaluateGoToRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6fc230 (2012 0x7610b0)
FDisStimSetupDelegate UDisBehaviorGoTo::GetSetupFromStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_GoToRequest )
	{
		return DIS_BIND_STIM_SETUP( UDisBehaviorGoTo, FAIStimStruct_GoToRequest, SetupFromGoToRequest );
	}
	return FDisStimSetupDelegate();
}

// DISHONORED(port): 2013 rva 0x6fc1c0 (2012 0x761040)
FDisStimPredicateDelegate UDisBehaviorGoTo::GetShouldFinishWhileDormantDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_BehaviorAbort )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorGoTo, FAIStimStruct_BehaviorAbort, ShouldFinishWhileDormantBehaviorAbort );
	}
	if( _StimID == EAIStimID_GoToRequest )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorGoTo, FAIStimStruct_GoToRequest, ShouldFinishWhileDormantGoToRequest );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2013 rva 0x6fc030 (2012 0x760eb0, 244 B): the base's three first, then the behaviour's own.
FDisStimPredicateDelegate UDisBehaviorGoTo::GetFilterStimDelegate( BYTE _StimID )
{
	FDisStimPredicateDelegate Delegate = UDisBehaviorGoTo_Base::GetFilterStimDelegate( _StimID );
	if( Delegate.m_pObject != NULL || Delegate.m_pDelegator != NULL )
	{
		return Delegate;
	}
	switch( _StimID )
	{
	case EAIStimID_Alarm:
	case EAIStimID_AttackedByEnemy:
	case EAIStimID_DiscoveredCorpse:
	case EAIStimID_EvadedMelee_Incoming:
	case EAIStimID_HeardSomething:
	case EAIStimID_Help:
	case EAIStimID_SearchRequest:
	case EAIStimID_WitnessDeath:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct, FilterInterruptStim );
	case EAIStimID_BehaviorAbort:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct_BehaviorAbort, FilterBehaviorAbort );
	case EAIStimID_Distracted_Anim:
	case EAIStimID_DocileRatIsNear:
		return GDisDelegateReturnTrue;
	case EAIStimID_FollowRequest:
	case EAIStimID_GuardRequest:
	case EAIStimID_PatrolRequest:
	case EAIStimID_ShootRequest:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct, FilterKismetAIAction );
	case EAIStimID_GoToRequest:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct_GoToRequest, FilterGoToRequest );
	case EAIStimID_TouchedAlly:
	case EAIStimID_TouchedNeutral:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct_TouchedNeutral, FilterTouchedNeutral );
	case EAIStimID_TouchedEnemy:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorGoTo, FAIStimStruct_TouchedEnemy, FilterTouchedEnemy );
	}
	return FDisStimPredicateDelegate();
}

/*-----------------------------------------------------------------------------
	The go-to request
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6f0700 (2012 0x75d2a0)
UBOOL UDisBehaviorGoTo::EvaluateGoToRequest( const FAIStimStruct_GoToRequest& _rStim ) const
{
	return DisGoToRequestWanted( const_cast<UDisBehaviorGoTo*>( this ), _rStim );
}

// DISHONORED(port): 2013 rva 0x6f0680 (2012 0x75d250): a dormant go-to finishes on a request it would NOT have wanted.
UBOOL UDisBehaviorGoTo::ShouldFinishWhileDormantGoToRequest( const FAIStimStruct_GoToRequest& _rStim ) const
{
	return !DisGoToRequestWanted( const_cast<UDisBehaviorGoTo*>( this ), _rStim );
}

// DISHONORED(port): the destination, speed, rotation alignment and interruptability a request carries, as FilterGoToRequest
// (2012 0x75d0f0) and SetupFromGoToRequest (0x75d2f0) both spell them out. A tracked (moving) destination keeps the actor;
// a fixed one is snapshotted now. The accurate-stop and exact-rotation bits come from the tweaks either way.
void UDisBehaviorGoTo::TakeDestinationFromAction( const FAIStimStruct_GoToRequest& _rStim )
{
	UDisSeqAct_AIGoToActor* Action = _rStim.m_pAssociatedKismetGoToAction;
	const UDisTweaks_AIBehavior_TakePosition* Tweaks = DisGoToTweaks( this );
	AActor* Destination = Action->GetDestinationActor();
	if( Action->m_bTrackMovingTarget )
	{
		m_pDestinationActor = Destination;
		m_bAccurateStop = Tweaks->m_bAccurateStop;
		m_bExactRotation = Tweaks->m_bExactRotation;
	}
	else
	{
		SetFixedDestination( Destination->Location, Destination->Rotation, Tweaks->m_bAccurateStop, Tweaks->m_bExactRotation );
	}
	m_bDoRotationAlign = Action->ShouldAlignRotation();
	m_TransitSpeed = Action->GetDesiredTransitSpeed();
	m_bInterruptable = Action->m_bInterruptable;
	m_pAssociatedKismetGoToAction = Action;
}

// DISHONORED(port): 2013 rva 0x6f0750 (2012 0x75d2f0, 255 B): the stim that activated the behaviour fills it in.
void UDisBehaviorGoTo::SetupFromGoToRequest( const FAIStimStruct_GoToRequest& _rStim )
{
	if( !_rStim.m_pAssociatedKismetGoToAction || !_rStim.m_pAssociatedKismetGoToAction->GetDestinationActor() )
	{
		return;
	}
	TakeDestinationFromAction( _rStim );
}

// DISHONORED(port): 2013 rva 0x6f04e0 (2012 0x75d0f0, 339 B): a second request while this one runs. One the behaviour
// would not want finishes it (unsuccessfully) so the brain can hand the stim to another; one it wants is taken as the
// new destination if the current go-to is interruptable, and refused otherwise. Consumed in both wanted cases.
UBOOL UDisBehaviorGoTo::FilterGoToRequest( const FAIStimStruct_GoToRequest& _rStim )
{
	if( !DisGoToRequestWanted( this, _rStim ) )
	{
		m_bIsFinished = TRUE;
		m_bSuccess = FALSE;
		return FALSE;
	}
	if( !m_bInterruptable )
	{
		return TRUE;
	}
	if( _rStim.m_pAssociatedKismetGoToAction && _rStim.m_pAssociatedKismetGoToAction->GetDestinationActor() )
	{
		TakeDestinationFromAction( _rStim );
		PrepareGoTo();
	}
	return TRUE;
}

/*-----------------------------------------------------------------------------
	The stims that end or survive a go-to
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6f0640 (2012 0x75c150): an abort aimed at this class finishes it, unsuccessfully. Not consumed.
UBOOL UDisBehaviorGoTo::FilterBehaviorAbort( const FAIStimStruct_BehaviorAbort& _rStim )
{
	if( _rStim.m_AbortedBehavior == UDisBehaviorGoTo::StaticClass() )
	{
		m_bIsFinished = TRUE;
		m_bSuccess = FALSE;
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x6f06d0 (2012 0x75c190)
UBOOL UDisBehaviorGoTo::ShouldFinishWhileDormantBehaviorAbort( const FAIStimStruct_BehaviorAbort& _rStim ) const
{
	return _rStim.m_AbortedBehavior == UDisBehaviorGoTo::StaticClass();
}

// DISHONORED(port): 2013 rva 0x6e4c70 (2012 0x7454a0): an uninterruptable go-to swallows the stims that would interrupt it.
UBOOL UDisBehaviorGoTo::FilterInterruptStim( const FAIStimStruct& _rStim )
{
	return !m_bInterruptable;
}

// DISHONORED(port): 2013 rva 0x6e8470 (2012 0x746cb0): another Kismet AI action ends an interruptable go-to (unsuccessfully,
// and the stim goes on to whoever takes it) and is swallowed by an uninterruptable one.
UBOOL UDisBehaviorGoTo::FilterKismetAIAction( const FAIStimStruct& _rStim )
{
	if( m_bInterruptable )
	{
		m_bIsFinished = TRUE;
		m_bSuccess = FALSE;
		return FALSE;
	}
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x6e4c80 (2012 0x7454b0): touching the very actor being walked to counts as arriving.
UBOOL UDisBehaviorGoTo::FilterTouchedNeutral( const FAIStimStruct_TouchedNeutral& _rStim )
{
	if( m_pDestinationActor && _rStim.m_pPawn == m_pDestinationActor )
	{
		m_bHasReachedDestination = TRUE;
		return TRUE;
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x6e4cb0 (2012 0x748e60): an uninterruptable go-to swallows the touch but still lets the brain
// fire the touched-actor Kismet event (UDishonoredAIBrain::OnTouchedEnemyStimAccepted).
UBOOL UDisBehaviorGoTo::FilterTouchedEnemy( const FAIStimStruct_TouchedEnemy& _rStim )
{
	if( !m_bInterruptable && m_pOwningBrain )
	{
		m_pOwningBrain->OnTouchedEnemyStimAccepted( _rStim );
	}
	return !m_bInterruptable;
}

/*-----------------------------------------------------------------------------
	Start and resume
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x6edae0 (2012 0x74c7a0): the personal-space sub-process manages the ambient actions while a
// go-to runs (UDisAISubProcessPersonalSpace::SetAmbientActionManagement( TRUE ), 2013 0x728920 / 2012 0x766240 - the one
// bit it writes is written here).
void UDisBehaviorGoTo::InitBehavior( UDishonoredAIBrain* const _pAIBrain )
{
	UDisAISubProcessPersonalSpace* PersonalSpace = (UDisAISubProcessPersonalSpace*)GetSubProcess( UDisAISubProcessPersonalSpace::StaticClass() );
	if( PersonalSpace )
	{
		PersonalSpace->m_bManageAmbientActions = TRUE;
	}
}

// DISHONORED(port): 2013 rva 0x6edb10 (2012 0x74c7d0, 116 B): the base's reset, and an uninterruptable go-to disables the two
// head-track sub-processes - a guard sent somewhere by Kismet with his attention locked does not glance at the player.
void UDisBehaviorGoTo::OnBehaviorStart()
{
	UDisBehaviorGoTo_Base::OnBehaviorStart();
	if( !m_bInterruptable )
	{
		UDisAISubProcess* HeadTrackBark = GetSubProcess( UDisAISubProcessHeadTrackBark::StaticClass() );
		if( HeadTrackBark )
		{
			HeadTrackBark->DisableSubProcess_Internal( m_bIsPaused );
		}
		UDisAISubProcess* HeadTrackPause = GetSubProcess( UDisAISubProcessHeadTrackPause::StaticClass() );
		if( HeadTrackPause )
		{
			HeadTrackPause->DisableSubProcess_Internal( m_bIsPaused );
		}
	}
}

// DISHONORED(port): 2012 rva 0x748e90 (259 B): the stance to come back to. Aiming and crouched-aim collapse to
// EquippedPatrol; a weapon in hand keeps the pawn Equipped (EquippedPatrol); a suspecting brain whose pawn owns a melee
// weapon draws it; and a running go-to turns EquippedPatrol into Equipped and Search into Equipped-with-weapon or Relaxed.
UBOOL UDisBehaviorGoTo::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	BYTE Stance = _rPreviousBodyIntention.m_IntendedBodyStance;
	if( Stance == eDisNPCBodyStance_Aiming || Stance == eDisNPCBodyStance_CrouchedAim )
	{
		Stance = eDisNPCBodyStance_EquippedPatrol;
	}
	UClass* PrimaryItemClass = _rPreviousBodyIntention.m_pDesiredPrimaryItemClass;
	UBOOL bHoldsWeapon = FALSE;
	if( PrimaryItemClass && PrimaryItemClass->IsChildOf( UDishonoredWeapon::StaticClass() ) )
	{
		bHoldsWeapon = TRUE;
		Stance = eDisNPCBodyStance_EquippedPatrol;
	}
	else if( m_pOwningBrain && m_pOwningBrain->m_pOwningPawn && m_pOwningBrain->m_pOwningPawn->m_pInventory )
	{
		const UBOOL bSuspecting = m_pOwningBrain->m_SuspicionLevel == DAISL_Suspecting;
		if( bSuspecting && m_pOwningBrain->m_pOwningPawn->m_pInventory->HasItemByClass( UDisWepMelee::StaticClass(), FALSE ) )
		{
			Stance = eDisNPCBodyStance_EquippedPatrol;
			PrimaryItemClass = UDisWepMelee::StaticClass();
		}
	}
	if( m_TransitSpeed == ETransitSpeed_Run )
	{
		if( Stance == eDisNPCBodyStance_EquippedPatrol )
		{
			Stance = eDisNPCBodyStance_Equipped;
		}
		else if( Stance == eDisNPCBodyStance_Search )
		{
			Stance = bHoldsWeapon ? eDisNPCBodyStance_Equipped : eDisNPCBodyStance_None;
		}
	}
	_rResumingBodyIntention.m_IntendedBodyStance = Stance;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = PrimaryItemClass;
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = _rPreviousBodyIntention.m_pDesiredSecondaryItemClass;
	return TRUE;
}

// DISHONORED(bringup, agent FV): every function above is a virtual or is reached through one, so nothing references this
// object and MSVC would drop it out of the static library. Named from DishonoredGameRegistrants.cpp.
void DishonoredLinkDisBehaviorGoTo()
{
}

// agent JE (PHASE24 JE), 2013 0x6e4c50 (2012 0x745480): a Kismet go-to gives its pawn to a blocking conversation only when it is
// interruptable.
eDisDialogAvailability UDisBehaviorGoTo::GetDialogAvailability( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	if( ( _StartDialogFlags & 2 ) && !m_bInterruptable )
	{
		return DDA_REJECTED_BY_AI_BEHAVIOR;
	}
	return DDA_AVAILABLE;
}
