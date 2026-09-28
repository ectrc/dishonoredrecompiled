// DishonoredGame/src/disaisubstatelieinwait.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateLieInWait and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateLieInWait_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781e20
FDisAISubStateLieInWait_Param::FDisAISubStateLieInWait_Param( ADisAmbushPoint* _pAmbushPoint, UBOOL _bTeleported, UBOOL _bLieInWaitIndefinitely )
	: FDisAISubState_Param( UDisAISubStateLieInWait::StaticClass() )
	, m_pAmbushPoint( _pAmbushPoint )
	, m_bTeleported( _bTeleported )
	, m_bLieInWaitIndefinitely( _bLieInWaitIndefinitely )
{
}

// DISHONORED(port): 2012 rva 0x769200
void FDisAISubStateLieInWait_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateLieInWait* LieInWait = (UDisAISubStateLieInWait*)PendingState;
	LieInWait->m_pAmbushPoint = m_pAmbushPoint;
	LieInWait->m_bTeleported = m_bTeleported ? 1 : 0;
	LieInWait->m_bLieInWaitIndefinitely = m_bLieInWaitIndefinitely ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateLieInWait

	Four states (m_LieInWaitState): 0 = turning to the ambush point's own facing, 1 = done turning, 2 = waiting,
	3 = ambushing. The wait clock only runs while the NPC was NOT told to wait indefinitely, which is what lets a scripted
	ambush hold forever and an opportunistic one give up.
-----------------------------------------------------------------------------*/

enum { DIS_LIEINWAIT_TURNING = 0, DIS_LIEINWAIT_DONETURNING = 1, DIS_LIEINWAIT_WAITING = 2, DIS_LIEINWAIT_AMBUSHING = 3 };

// DISHONORED(port): 2012 rva 0x765640. Ten stims - the widest filter mask of any sub-state, because almost anything is a
// reason to stop lying in wait. Cross-checked against the delegate table below: ten bits, ten ids.
const BYTE* UDisAISubStateLieInWait::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = {
		EAIStimID_AttackedByEnemy, EAIStimID_EndPossession, EAIStimID_EscapedBeingChoked, EAIStimID_HeardSomething,
		EAIStimID_IncomingDamage, EAIStimID_LocoPushedByPlayer, EAIStimID_PlagueZone, EAIStimID_RotationReached,
		EAIStimID_Teleported, EAIStimID_TouchedEnemy };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x78c860
FDisStimPredicateDelegate UDisAISubStateLieInWait::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_AttackedByEnemy:
	case EAIStimID_EndPossession:
	case EAIStimID_EscapedBeingChoked:
	case EAIStimID_IncomingDamage:
	case EAIStimID_LocoPushedByPlayer:
	case EAIStimID_PlagueZone:
	case EAIStimID_TouchedEnemy:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateLieInWait, FAIStimStruct, FilterAbortingStim );
	case EAIStimID_HeardSomething:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateLieInWait, FAIStimStruct, FilterHeardSomething );
	case EAIStimID_RotationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateLieInWait, FAIStimStruct_RotationReached, FilterRotationReached );
	case EAIStimID_Teleported:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateLieInWait, FAIStimStruct, FilterTeleported );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x787f80: face the way the ambush point faces, and arm the wait clock from the slot's tweaks.
void UDisAISubStateLieInWait::BeginSubState_Derived()
{
	if( m_pAmbushPoint )
	{
		SetFaceToYawDesire( m_pAmbushPoint->Rotation.Yaw, -100.f, FALSE );
	}
	const UDisTweaks_AISubState_LieInWait* Tweaks = Cast<UDisTweaks_AISubState_LieInWait>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_LieInWait*)UDisTweaks_AISubState_LieInWait::StaticClass()->GetDefaultObject();
	}
	m_LieInWaitState = DIS_LIEINWAIT_TURNING;
	m_fWaitTimer = Tweaks->m_fWaitTime;
	m_fAmbushAttackTimer = 0.f;
}

// DISHONORED(port): 2012 rva 0x7740a0: an ambush that ends normally tells the pawn's own lying-in-wait anim state to
// leave, so the NPC stands up.
// DISHONORED(bringup): UStateNPCMasterLyingInWait is one of agent AV's anim states, unported, so the NPC was never lying
// down to begin with.
void UDisAISubStateLieInWait::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	if( !bIsBeingTerminated )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): UStateNPCMasterLyingInWait is not ported; an ambushing NPC never lies down or stands up") );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7656c0 / 0x7656f0: turning finished, or the NPC was moved - either way it is in position.
UBOOL UDisAISubStateLieInWait::FilterRotationReached( const FAIStimStruct_RotationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine && m_LieInWaitState == DIS_LIEINWAIT_TURNING )
	{
		m_LieInWaitState = DIS_LIEINWAIT_DONETURNING;
	}
	return bMine;
}

UBOOL UDisAISubStateLieInWait::FilterTeleported( const FAIStimStruct& _rStim )
{
	m_LieInWaitState = DIS_LIEINWAIT_DONETURNING;
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7656a0: anything violent while merely waiting ends the ambush.
UBOOL UDisAISubStateLieInWait::FilterAbortingStim( const FAIStimStruct& _rStim )
{
	if( m_LieInWaitState == DIS_LIEINWAIT_WAITING )
	{
		RequestStateExit();
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x788040. A noise only counts while waiting, only if the player made it, only if it is one of
// the five loud contexts, only within the slot's danger radius - and, for the loudest of them, only from behind.
// DISHONORED(bringup): the noise contexts are in the stim, so this is faithful, but nothing in the tree raises
// HeardSomething yet (UDishonoredAIBrain::TickBrain_Senses is unported, agentCG.md), so the branch is unreachable.
UBOOL UDisAISubStateLieInWait::FilterHeardSomething( const FAIStimStruct& _rStim )
{
	if( m_LieInWaitState != DIS_LIEINWAIT_WAITING )
	{
		return FALSE;
	}
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): nothing raises EAIStimID_HeardSomething yet (TickBrain_Senses); an ambushing NPC cannot be startled") );
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x78b8b0: the wait clock runs only while the NPC was not told to wait indefinitely, and
// running out is what leaves the state.
void UDisAISubStateLieInWait::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( !m_bLieInWaitIndefinitely )
	{
		m_fWaitTimer -= DeltaSeconds;
		if( m_fWaitTimer <= 0.f )
		{
			RequestStateExit();
			return;
		}
	}

	switch( m_LieInWaitState )
	{
	case DIS_LIEINWAIT_DONETURNING:
		TickState_DoneTurning( DeltaSeconds );
		break;
	case DIS_LIEINWAIT_WAITING:
		TickState_Waiting( DeltaSeconds );
		break;
	case DIS_LIEINWAIT_AMBUSHING:
		TickState_Ambushing( DeltaSeconds );
		break;
	default:
		break;
	}
}

// DISHONORED(port): 2012 rvas 0x788120 / 0x7881d0 / 0x788200.
// DISHONORED(bringup): all three stages drive the pawn's own lying-in-wait anim state and, in the ambushing stage, the
// assassinate item context - agent AV's anim states and agent AJ's item contexts. The stage machine itself is faithful;
// with neither package the NPC waits in place and never springs.
void UDisAISubStateLieInWait::TickState_DoneTurning( FLOAT _fDeltaSeconds )
{
	m_LieInWaitState = DIS_LIEINWAIT_WAITING;
}

void UDisAISubStateLieInWait::TickState_Waiting( FLOAT _fDeltaSeconds )
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UStateNPCMasterLyingInWait and the assassinate item context are not ported; a waiting ambusher never springs") );
	}
}

void UDisAISubStateLieInWait::TickState_Ambushing( FLOAT _fDeltaSeconds )
{
	m_fAmbushAttackTimer += _fDeltaSeconds;
}
