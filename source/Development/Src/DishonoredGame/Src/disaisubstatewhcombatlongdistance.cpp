// DishonoredGame/src/disaisubstatewhcombatlongdistance.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateWHCombatLongDistance and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

/*-----------------------------------------------------------------------------
	FDisAISubState_WH_CombatLongDistance_Param

	DISHONORED(retail): the eight distance and frequency values are ZEROED by the constructor and filled in by the caller
	from its own tweaks, which is why the constructor takes only the enemy. UDisBehaviorCombatWolfhound is the only caller.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7824a0
FDisAISubState_WH_CombatLongDistance_Param::FDisAISubState_WH_CombatLongDistance_Param( const FDisAttentionProxy& _rEnemyProxy )
	: FDisAISubStateCombatBase_Param( _rEnemyProxy, eDisNPCBodyStance_Equipped )
	, m_fAllowedDistanceFromEnemyMin( 0.f )
	, m_fAllowedDistanceFromEnemyMax( 0.f )
	, m_fPreferredDistanceFromEnemyMin( 0.f )
	, m_fPreferredDistanceFromEnemyMax( 0.f )
	, m_fRepositionDistanceMin( 0.f )
	, m_fRepositionFrequencyMin( 0.f )
	, m_fRepositionFrequencyMax( 0.f )
	, m_fLOSOriginOffsetZ( 0.f )
{
	m_pStateClass = UDisAISubStateWHCombatLongDistance::StaticClass();
}

// DISHONORED(port): 2012 rva 0x769770
void FDisAISubState_WH_CombatLongDistance_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubStateCombatBase_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateWHCombatLongDistance* WH = (UDisAISubStateWHCombatLongDistance*)PendingState;
	WH->m_fAllowedDistanceFromEnemyMin = m_fAllowedDistanceFromEnemyMin;
	WH->m_fAllowedDistanceFromEnemyMax = m_fAllowedDistanceFromEnemyMax;
	WH->m_fPreferredDistanceFromEnemyMin = m_fPreferredDistanceFromEnemyMin;
	WH->m_fPreferredDistanceFromEnemyMax = m_fPreferredDistanceFromEnemyMax;
	WH->m_fRepositionDistanceMin = m_fRepositionDistanceMin;
	WH->m_fRepositionFrequencyMin = m_fRepositionFrequencyMin;
	WH->m_fRepositionFrequencyMax = m_fRepositionFrequencyMax;
	WH->m_fLOSOriginOffsetZ = m_fLOSOriginOffsetZ;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateWHCombatLongDistance
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765be0
const BYTE* UDisAISubStateWHCombatLongDistance::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_PathingFail };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

FDisStimPredicateDelegate UDisAISubStateWHCombatLongDistance::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateWHCombatLongDistance, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateWHCombatLongDistance, FAIStimStruct_PathingFail, FilterPathingFail );
	default:
		return FDisStimPredicateDelegate();
	}
}

/*-----------------------------------------------------------------------------
	DISHONORED(retail): UDisAISubStateWHCombatLongDistance::SetHasSeenEnemyRecently (2012 rva 0x765c10) and the
	m_bHasSeenEnemyRecently member it wrote DO NOT EXIST IN RETAIL 2013, and neither is ported here.

	Three independent things say so. The retail reflected layout of the class has m_bIsSettingUpCustomPathfinding and
	m_bHasRepositionRequest in that bitfield and no third bit, where the 2012 PDB has m_bHasSeenEnemyRecently as bit 0 of
	the same DWORD. SetHasSeenEnemyRecently has no match in match_2012_2013.csv. And it is not alone: 8 of this class's 17
	functions have no 2013 counterpart (BeginSubState_Derived, BuildFilterStimMask, EndSubState_Derived, FilterPathingFail,
	GetPathGoals, RefreshSubState, SetHasSeenEnemyRecently and UpdateRepositionRequest), which is the signature of a class
	Arkane reworked between the two builds rather than of a bad match.

	Its caller is UDisBehaviorCombatWolfhound::HasSeenEnemyContinuouslyRecently, which agent CG's classification lists as an
	own-helper of that behaviour - so whoever ports the wolfhound behaviour must decide what retail 2013 does instead, and
	must not reintroduce this setter. This is the same shape as agent CG's finding about m_BrainInhibitors: a 2012-faithful
	port would have added a member retail does not have.
-----------------------------------------------------------------------------*/

void UDisAISubStateWHCombatLongDistance::ResetRepositionRequest()
{
	m_bHasRepositionRequest = FALSE;
	m_bIsSettingUpCustomPathfinding = FALSE;
	m_RepositionStartLocation = FVector( 0.f, 0.f, 0.f );
	m_pLastGoalEvaluator = NULL;
}

// DISHONORED(port): 2012 rva 0x783d60: the reposition clock is rolled from the parameter's own frequency band.
void UDisAISubStateWHCombatLongDistance::BeginSubState_Derived()
{
	CombatEngageAttackPattern();
	ResetRepositionRequest();
	m_fNextRepositionTime = m_fRepositionFrequencyMin + appFrand() * ( m_fRepositionFrequencyMax - m_fRepositionFrequencyMin );
	m_fNextPossibleFindPathTime = 0.f;
	SetLookAtProxyDesire( m_EnemyProxy, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );
	SetFaceToProxyDesire( m_EnemyProxy, -100.f, FALSE );
}

// DISHONORED(port): 2012 rva 0x780520
void UDisAISubStateWHCombatLongDistance::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	ResetRepositionRequest();
}

// DISHONORED(port): 2012 rva 0x783d80 and UpdateRepositionRequest 0x782550 (966 bytes, the largest body of any sub-state).
// DISHONORED(bringup): the reposition is a custom nav-mesh query - UDisNavMeshGoal_WolfhoundBarkPosition scored against
// the allowed and preferred distance bands with a line-of-sight trace from m_fLOSOriginOffsetZ. That goal class is
// generated with its members and has no body, and there is no path finder to hand it to, so the hound holds its facing
// and its distance band is recorded but never acted on. The clock and the facing are faithful.
void UDisAISubStateWHCombatLongDistance::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	m_fNextRepositionTime -= TimeSinceLastThought;
	if( m_fNextRepositionTime > 0.f )
	{
		return;
	}
	m_fNextRepositionTime = m_fRepositionFrequencyMin + appFrand() * ( m_fRepositionFrequencyMax - m_fRepositionFrequencyMin );
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisNavMeshGoal_WolfhoundBarkPosition and the path finder are not ported; a circling wolfhound rolls its reposition clock and stays put") );
	}
}

UBOOL UDisAISubStateWHCombatLongDistance::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		ResetRepositionRequest();
	}
	return bMine;
}

UBOOL UDisAISubStateWHCombatLongDistance::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		ResetRepositionRequest();
	}
	return bMine;
}
