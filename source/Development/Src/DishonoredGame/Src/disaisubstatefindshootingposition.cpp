// DishonoredGame/src/disaisubstatefindshootingposition.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateFindShootingPosition and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateFindShootingPosition_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781af0, SizeOf 0x781ba0 = 64. The line-check extent and the shooting height are the trace
// shape the candidate search uses, so they belong to the request rather than to the sub-state's tweaks.
FDisAISubStateFindShootingPosition_Param::FDisAISubStateFindShootingPosition_Param( const FDisAttentionProxy& _rTargetProxy, FVector _LineCheckExtent, FLOAT _fShootingHeight, UBOOL _bForceMovement, UBOOL _bManageRotation, UBOOL _bIgnoreMinRangeWhenUnreachable )
	: FDisAISubState_Param( UDisAISubStateFindShootingPosition::StaticClass() )
	, m_TargetProxy( _rTargetProxy )
	, m_LineCheckExtent( _LineCheckExtent )
	, m_fShootingHeight( _fShootingHeight )
	, m_bForceMovement( _bForceMovement )
	, m_bManageRotation( _bManageRotation )
	, m_bIgnoreMinRangeWhenUnreachable( _bIgnoreMinRangeWhenUnreachable )
{
}

// DISHONORED(port): 2012 rva 0x7687e0
void FDisAISubStateFindShootingPosition_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateFindShootingPosition* Find = (UDisAISubStateFindShootingPosition*)PendingState;
	Find->m_TargetProxy = m_TargetProxy;
	Find->m_LineCheckExtent = m_LineCheckExtent;
	Find->m_fShootingHeight = m_fShootingHeight;
	Find->m_bForceMovement = m_bForceMovement ? 1 : 0;
	Find->m_bManageRotation = m_bManageRotation ? 1 : 0;
	Find->m_bIgnoreMinRangeWhenUnreachable = m_bIgnoreMinRangeWhenUnreachable ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateFindShootingPosition
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765380
const BYTE* UDisAISubStateFindShootingPosition::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_PathingFail };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

FDisStimPredicateDelegate UDisAISubStateFindShootingPosition::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateFindShootingPosition, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateFindShootingPosition, FAIStimStruct_PathingFail, FilterPathingFail );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x788b60.
// DISHONORED(bringup): the shoot-range band is NOT in this sub-state's own tweaks - measured: UDisTweaks_AISubState_FindShootingPosition
// carries the weapon type, the equip usage, the context slot, the speed, the projectile context, the line-check budget, the
// avoidance radius, the allowed pitch and the give-up time, and no distances at all. Retail reads the band off the WEAPON
// the m_pWeaponType names, through the equipped ranged item - agent AJ's item-context root, unported. So the three
// distances stay zero and IsWithinLegalShootRange answers TRUE for everything, which is the widest rather than the
// narrowest wrong answer: a shooter will try from where it is instead of refusing to shoot at all.
void UDisAISubStateFindShootingPosition::CalculateShootDistances()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): the ranged weapon's range band is not readable (UDisItemContext); a shooter has no shoot-distance limits") );
	}
	m_fMinShootDistance = 0.f;
	m_fIdealShootDistance = 0.f;
	m_fMaxShootDistance = 0.f;
}

// DISHONORED(port): 2012 rva 0x788cb0: the test the behaviour's callbacks ask before they decide whether to keep shooting
// from where the NPC already is.
UBOOL UDisAISubStateFindShootingPosition::IsWithinLegalShootRange( const FVector& _rPosition ) const
{
	if( m_fMaxShootDistance <= 0.f )
	{
		// See CalculateShootDistances: with no weapon there is no band, and every position is legal.
		return TRUE;
	}
	const FLOAT DistanceSq = ( _rPosition - m_TargetProxy.GetBestTargetLocation() ).SizeSquared();
	return DistanceSq >= ( m_fMinShootDistance * m_fMinShootDistance )
		&& DistanceSq <= ( m_fMaxShootDistance * m_fMaxShootDistance );
}

// DISHONORED(port): 2012 rva 0x788d40-region.
// DISHONORED(bringup): the candidate search is the nav-mesh half of this sub-state and the largest single thing in it:
// FindShootPositionCandidates walks the mesh around the target, EvaluatePosition traces from each candidate to see whether
// the target can be hit from it, and GetPathGoals hands the path finder a UDisNavMeshGoal_ShootingPosition. All three need
// the nav-mesh runtime agent AD left as the reference design and agent CG's hand-over 3 itemises, so there are no
// candidates and the sub-state falls through to "just start walking somewhere", which is retail's own fallback when the
// search comes up empty.
void UDisAISubStateFindShootingPosition::JustStartWalkingSomewhere()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): the nav-mesh candidate search is not ported; a shooter walks at its target instead of finding a firing position") );
	}
	SetLocoProxyDesire( m_TargetProxy, ETransitSpeed_Run, -1.f, 1.f, FALSE, FALSE );
}

void UDisAISubStateFindShootingPosition::BeginSubState_Derived()
{
	CalculateShootDistances();
	m_ShootPositionCandidates.Empty();
	m_iCandidateEvaluationIndex = 0;
	m_pLastGoalEvaluator = NULL;
	JustStartWalkingSomewhere();
	if( m_bManageRotation )
	{
		SetFaceToProxyDesire( m_TargetProxy, -100.f, FALSE );
	}
}

void UDisAISubStateFindShootingPosition::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
	CalculateShootDistances();
}

UBOOL UDisAISubStateFindShootingPosition::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		RequestStateExit();
	}
	return bMine;
}

UBOOL UDisAISubStateFindShootingPosition::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		RequestStateExit();
	}
	return bMine;
}

// DISHONORED(bringup): 2012 rvas 0x775280 / 0x775100: the goal evaluator and the constraints the path finder is handed.
// UDisNavMeshGoal_ShootingPosition is generated with its members and has no body, and there is no path finder to hand it
// to, so the base's "no extra goals" answer stands - which is what agent CG's UDishonoredAIBehavior::GetDefaultPathGoals
// already answers for every behaviour.
UBOOL UDisAISubStateFindShootingPosition::GetPathGoals( const FVector& Destination, TArray<UNavMeshPathGoalEvaluator*>& OutGoals )
{
	return Super::GetPathGoals( Destination, OutGoals );
}

UBOOL UDisAISubStateFindShootingPosition::GetPathConstraints( const FVector& Destination, UBOOL bIsFleeing, TArray<UNavMeshPathConstraint*>& OutConstraints )
{
	return Super::GetPathConstraints( Destination, bIsFleeing, OutConstraints );
}
