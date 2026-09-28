// DishonoredGame/src/disaisubstatestand.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateStand and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateStand_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781f90. "Stand at this spot facing this yaw."
FDisAISubStateStand_Param::FDisAISubStateStand_Param( const FVector& _rLocation, const FRotator& _rRotation, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance )
	: FDisAISubState_Param( UDisAISubStateStand::StaticClass() )
	, m_pFocusActor( NULL )
	, m_eDesiredBodyStance( _eDesiredBodyStance )
	, m_bAccurateStop( _bAccurateStop )
	, m_bExactRotation( _bExactRotation )
	, m_bAlwaysStrafe( _bAlwaysStrafe )
{
	m_FocusProxy.ClearAttnProxy();
	m_ActorPosition.m_Loc = _rLocation;
	m_ActorPosition.m_Rot = _rRotation;
}

// DISHONORED(port): 2012 rva 0x782030. "Stand at this spot facing that actor." The rotation half of m_ActorPosition is
// deliberately left alone - the focus actor supplies the facing, so there is nothing to write there.
FDisAISubStateStand_Param::FDisAISubStateStand_Param( const FVector& _rLocation, AActor* _pFocusActor, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance )
	: FDisAISubState_Param( UDisAISubStateStand::StaticClass() )
	, m_pFocusActor( _pFocusActor )
	, m_eDesiredBodyStance( _eDesiredBodyStance )
	, m_bAccurateStop( _bAccurateStop )
	, m_bExactRotation( _bExactRotation )
	, m_bAlwaysStrafe( _bAlwaysStrafe )
{
	m_FocusProxy.ClearAttnProxy();
	m_ActorPosition.m_Loc = _rLocation;
}

// DISHONORED(port): 2012 rva 0x7820b0. "Stand at this spot facing whatever this proxy resolves to." The proxy form is the
// one combat uses, because it follows the NPC's own belief about where the target is.
FDisAISubStateStand_Param::FDisAISubStateStand_Param( const FVector& _rLocation, const FDisAttentionProxy& _rFocusProxy, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance )
	: FDisAISubState_Param( UDisAISubStateStand::StaticClass() )
	, m_FocusProxy( _rFocusProxy )
	, m_pFocusActor( NULL )
	, m_eDesiredBodyStance( _eDesiredBodyStance )
	, m_bAccurateStop( _bAccurateStop )
	, m_bExactRotation( _bExactRotation )
	, m_bAlwaysStrafe( _bAlwaysStrafe )
{
	m_ActorPosition.m_Loc = _rLocation;
}

// DISHONORED(port): 2012 rva 0x7694e0, 209 bytes, the largest OnPending of the 25. Writes the sub-state's members at
// 2012 offsets 236 (m_pFocusActor), 240 (m_FocusProxy), 208 (m_Position), 232 bits 0/1/5 (m_bAccurateStop,
// m_bExactRotation, m_bAlwaysStrafe) and 256 (m_eDesiredBodyStance) - all resolved with build/agentDF_work/off.py and
// checked against the retail declaration.
void FDisAISubStateStand_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateStand* Stand = (UDisAISubStateStand*)PendingState;
	Stand->m_pFocusActor = m_pFocusActor;
	Stand->m_FocusProxy = m_FocusProxy;
	Stand->m_Position = m_ActorPosition;
	Stand->m_bAccurateStop = m_bAccurateStop ? 1 : 0;
	Stand->m_bExactRotation = m_bExactRotation ? 1 : 0;
	Stand->m_eDesiredBodyStance = m_eDesiredBodyStance;
	Stand->m_bAlwaysStrafe = m_bAlwaysStrafe ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateStand
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765840. Four stims, read off GetFilterStimDelegate_SubState below; the 2012 mask body sets
// exactly four bits, which is the cross-check.
const BYTE* UDisAISubStateStand::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_EndPossession, EAIStimID_IncomingDamage, EAIStimID_Teleported };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x78d0f0. Arriving is one thing; the other three are "you may have been moved" - possession
// ending, taking damage and being teleported all invalidate the loco request, so all three re-state it.
FDisStimPredicateDelegate UDisAISubStateStand::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateStand, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_EndPossession:
	case EAIStimID_IncomingDamage:
	case EAIStimID_Teleported:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateStand, FAIStimStruct, FilterPossibleMovement );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x78c950
void UDisAISubStateStand::BeginSubState_Derived()
{
	EnsureProperLocation();
}

// DISHONORED(port): 2012 rva 0x765880: a state that is merely pausing drops the facing and re-arms it, so the desire is
// re-stated against whatever the focus has become by the time it resumes. One that is terminating leaves it to
// EndSubState.
void UDisAISubStateStand::PauseSubState_Derived( UBOOL bIsBeingTerminated )
{
	if( !bIsBeingTerminated )
	{
		ClearFaceToDesire();
		m_bNeedsRotationFocus = TRUE;
	}
}

// DISHONORED(port): 2012 rva 0x78b9b0. An NPC that always strafes is ready to face its focus at once; one that does not
// has to arrive first, which is what FilterDestinationReached then reports.
void UDisAISubStateStand::EnsureProperLocation()
{
	m_bReadyToFocus = m_bAlwaysStrafe;

	// DISHONORED(retail): the tweaks object is fetched and not read. It is fetched through the virtual GetTweaks_Derived,
	// so the compiler kept the call; the three loco parameters below are literals in both builds.
	GetTweaks_Derived();

	SetLocoLocationDesire( m_Position.m_Loc, ETransitSpeed_Walk, -1.f, 1.f, m_bAccurateStop != 0, FALSE );
	m_bNeedsRotationFocus = TRUE;
	EnsureProperRotation();
}

// DISHONORED(port): 2012 rva 0x7884e0. Reads as a small truth table: disabled, or not yet arrived, means clear the
// facing; arrived and needed means state it, proxy first, then actor, then the fixed yaw from m_Position.
void UDisAISubStateStand::EnsureProperRotation()
{
	if( m_bRotationFocusDisabled )
	{
		ClearFaceToDesire();
		return;
	}

	if( !m_bNeedsRotationFocus )
	{
		if( !m_bReadyToFocus )
		{
			m_bNeedsRotationFocus = TRUE;
			ClearFaceToDesire();
		}
		return;
	}

	if( !m_bReadyToFocus )
	{
		ClearFaceToDesire();
		return;
	}

	GetTweaks_Derived();
	m_bNeedsRotationFocus = FALSE;

	if( m_FocusProxy.IsValid() )
	{
		SetFaceToProxyDesire( m_FocusProxy, -100.f, m_bExactRotation != 0 );
	}
	else if( m_pFocusActor )
	{
		SetFaceToActorDesire( m_pFocusActor, -100.f, m_bExactRotation != 0 );
	}
	else
	{
		SetFaceToYawDesire( m_Position.m_Rot.Yaw, -100.f, m_bExactRotation != 0 );
	}
}

// DISHONORED(port): 2012 rva 0x78b990: arrived, so the facing may start. The predicate answers FALSE - a sub-state filter
// that returns TRUE would stop the stim reaching anything else.
UBOOL UDisAISubStateStand::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	m_bReadyToFocus = TRUE;
	EnsureProperRotation();
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x78c940
UBOOL UDisAISubStateStand::FilterPossibleMovement( const FAIStimStruct& _rStim )
{
	EnsureProperLocation();
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7885e0: the two bits are complementary - disabling the focus also clears "I still need
// one", so re-enabling it re-states the desire.
void UDisAISubStateStand::DisableRotationFocus( UBOOL _bDisable )
{
	m_bRotationFocusDisabled = _bDisable ? 1 : 0;
	m_bNeedsRotationFocus = _bDisable ? 0 : 1;
	EnsureProperRotation();
}

// DISHONORED(port): 2012 rva 0x7658b0
UBOOL UDisAISubStateStand::IsRotationFocusDisabled() const
{
	return m_bRotationFocusDisabled != 0;
}

// DISHONORED(port): 2012 rva 0x7695c0: the stance the param asked for, with both hands empty. This is the first override
// of GetResumingBodyIntentionDesire that answers TRUE, i.e. the first sub-state that has an opinion about the NPC's
// stance when its behaviour comes back to slot 0.
UBOOL UDisAISubStateStand::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	_rResumingBodyIntention.m_IntendedBodyStance = m_eDesiredBodyStance;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = NULL;
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	return TRUE;
}
