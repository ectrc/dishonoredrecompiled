// DishonoredGame/src/disaisubstatecower.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateCower and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateCower_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7819c0
FDisAISubStateCower_Param::FDisAISubStateCower_Param( const FDisAttentionProxy& _rEnemyProxy, UDisFleeComponent* const _pCurrentFleeComponent )
	: FDisAISubState_Param( UDisAISubStateCower::StaticClass() )
	, m_EnemyProxy( _rEnemyProxy )
	, m_pCurrentFleeComponent( _pCurrentFleeComponent )
{
}

// DISHONORED(port): 2012 rva 0x768690
void FDisAISubStateCower_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateCower* Cower = (UDisAISubStateCower*)PendingState;
	Cower->m_pCurrentFleeComponent = m_pCurrentFleeComponent;
	Cower->m_EnemyProxy = m_EnemyProxy;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateCower
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7652a0
const BYTE* UDisAISubStateCower::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_RotationReached, EAIStimID_TopAttnProxyReplaced };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x78c6c0
FDisStimPredicateDelegate UDisAISubStateCower::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_RotationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateCower, FAIStimStruct_RotationReached, FilterRotationReached );
	case EAIStimID_TopAttnProxyReplaced:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateCower, FAIStimStruct, FilterTopAttnProxyReplaced );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x7864e0. A fighter cowering holds its weapon; a civilian recovers from the panic. The slot's
// m_bFocusOnThreat decides whether it keeps facing what frightened it; if not - or if the threat has gone - there is
// nothing to cower at and the state leaves at once.
void UDisAISubStateCower::BeginSubState_Derived()
{
	const UDisTweaks_AISubState_Cower* Tweaks = Cast<UDisTweaks_AISubState_Cower>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Cower*)UDisTweaks_AISubState_Cower::StaticClass()->GetDefaultObject();
	}
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	const UBOOL bIsFighter = Pawn ? Pawn->IsAFighter() : FALSE;
	SetBodyIntentionDesire( bIsFighter ? eDisNPCBodyStance_Equipped : eDisNPCBodyStance_PanicRecover, NULL, NULL );

	if( Tweaks->m_bFocusOnThreat && m_EnemyProxy.IsValid() )
	{
		SetFaceToProxyDesire( m_EnemyProxy, -100.f, FALSE );
	}
	else
	{
		RequestStateExit();
	}
}

// DISHONORED(port): 2012 rva 0x786690: once it has turned to face the threat, a cowering NPC that was not told to keep
// focusing on it is done.
UBOOL UDisAISubStateCower::FilterRotationReached( const FAIStimStruct_RotationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		const UDisTweaks_AISubState_Cower* Tweaks = Cast<UDisTweaks_AISubState_Cower>( GetTweaks_Derived() );
		if( !Tweaks )
		{
			Tweaks = (const UDisTweaks_AISubState_Cower*)UDisTweaks_AISubState_Cower::StaticClass()->GetDefaultObject();
		}
		if( !Tweaks->m_bFocusOnThreat )
		{
			RequestStateExit();
		}
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x786590. The attention system has replaced what this NPC is most aware of; if the new thing
// is a living enemy the cowering turns to face that instead, otherwise the facing is dropped altogether.
// DISHONORED(bringup): the two questions asked of the new proxy's target - IsAttnTargetDead and
// GetAttnTargetIncomingRelationship - are IDisAttentionTargetInterface methods that agent CG declared but left as the
// base's answers (the interface's own package), so a replaced proxy currently never reads as an enemy and the facing is
// always dropped. The control flow is retail's.
UBOOL UDisAISubStateCower::FilterTopAttnProxyReplaced( const FAIStimStruct& _rStim )
{
	const UDisTweaks_AISubState_Cower* Tweaks = Cast<UDisTweaks_AISubState_Cower>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Cower*)UDisTweaks_AISubState_Cower::StaticClass()->GetDefaultObject();
	}
	if( !Tweaks->m_bFocusOnThreat )
	{
		return FALSE;
	}

	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): IDisAttentionTargetInterface::GetAttnTargetIncomingRelationship is not ported; a cowering NPC drops its focus when its top attention changes") );
	}
	m_EnemyProxy.ClearAttnProxy();
	ClearFaceToDesire();
	return FALSE;
}
