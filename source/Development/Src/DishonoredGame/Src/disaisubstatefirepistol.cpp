// DishonoredGame/src/disaisubstatefirepistol.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateFirePistol and its two parameters ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

/*-----------------------------------------------------------------------------
	The two parameters

	DISHONORED(retail): one sub-state, two ways in. The combat form takes an attention proxy and resolves it to an actor in
	OnPending, so the shot follows the NPC's own belief about where you are; the controlled form takes a plain actor and a
	shot count, which is what Kismet and the tall-boy scripting use.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781c10
FDisAISubStateFirePistolCombat_Param::FDisAISubStateFirePistolCombat_Param( const FDisAttentionProxy& _rTargetProxy, UBOOL _bChainingShots )
	: FDisAISubState_Param( UDisAISubStateFirePistol::StaticClass() )
	, m_TargetProxy( _rTargetProxy )
	, m_bChainingShots( _bChainingShots )
{
}

// DISHONORED(port): 2012 rva 0x777580. The proxy is resolved to an actor here and the termination subscription moved with
// it; m_iMaxNumShots is set to INDEX_NONE ("until I say stop") and the controlled-firing flag cleared.
void FDisAISubStateFirePistolCombat_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateFirePistol* FirePistol = (UDisAISubStateFirePistol*)PendingState;
	FirePistol->m_TargetProxy = m_TargetProxy;

	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( FirePistol->m_pTarget && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, FirePistol, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
	}
	IDisAttentionTargetInterface* Target = m_TargetProxy.GetProxyAttnTarget();
	FirePistol->m_pTarget = Target ? Target->GetAttnTargetActor() : NULL;
	if( FirePistol->m_pTarget && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, FirePistol, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
	}

	FirePistol->m_bShootForever = FALSE;
	FirePistol->m_bControlledFiring = FALSE;
	FirePistol->m_iMaxNumShots = INDEX_NONE;
	FirePistol->m_bChainingShots = m_bChainingShots ? 1 : 0;
}

// DISHONORED(port): 2012 rva 0x781bb0
FDisAISubStateFirePistolControlled_Param::FDisAISubStateFirePistolControlled_Param( AActor* _pTarget, UBOOL _bShootForever, INT _iMaxNumShots )
	: FDisAISubState_Param( UDisAISubStateFirePistol::StaticClass() )
	, m_pTarget( _pTarget )
	, m_bShootForever( _bShootForever )
	, m_iMaxNumShots( _iMaxNumShots )
{
}

// DISHONORED(port): 2012 rva 0x7774b0: the proxy is cleared - a controlled shot aims at the actor, not at a belief - and
// the controlled-firing flag is SET, which is what stops the sub-state deciding for itself when to stop.
void FDisAISubStateFirePistolControlled_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateFirePistol* FirePistol = (UDisAISubStateFirePistol*)PendingState;
	FirePistol->m_TargetProxy.ClearAttnProxy();

	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( FirePistol->m_pTarget && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, FirePistol, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
	}
	FirePistol->m_pTarget = m_pTarget;
	if( FirePistol->m_pTarget && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, FirePistol, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
	}

	FirePistol->m_bShootForever = m_bShootForever ? 1 : 0;
	FirePistol->m_bChainingShots = FALSE;
	FirePistol->m_bControlledFiring = TRUE;
	FirePistol->m_iMaxNumShots = m_iMaxNumShots;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateFirePistol

	m_AIPistolState is the sub-state's own five-value machine, and it is what makes an NPC raise, hold, fire, lower and
	reload a pistol instead of snapping between poses.
-----------------------------------------------------------------------------*/

enum { DIS_PISTOL_RELAXED = 0, DIS_PISTOL_AIMING = 1, DIS_PISTOL_RECOVERING = 2, DIS_PISTOL_RELOADING = 3 };

// DISHONORED(port): 2012 rva 0x771640-region: the target is aimed at with the whole upper body, and the NPC holds its
// pistol up while the sub-state runs.
// DISHONORED(bringup): the shot itself is a UDisItemContext on the equipped ranged weapon (agent AJ's third root), so the
// state machine below runs but PullTheTrigger has nothing to pull: the NPC aims and never fires.
void UDisAISubStateFirePistol::BeginSubState_Derived()
{
	m_AIPistolState = DIS_PISTOL_RELAXED;
	m_bHasFired = FALSE;
	m_bDoneTrying = FALSE;
	m_bAllowExit = FALSE;
	m_fFireTimer = 0.f;

	if( m_pTarget )
	{
		SetAimAtActorDesire( m_pTarget, FDisLookAtInfluence::Torso, -1.f );
		SetFaceToActorDesire( m_pTarget, -100.f, FALSE );
	}
	else if( m_TargetProxy.IsValid() )
	{
		SetAimAtProxyDesire( m_TargetProxy, FDisLookAtInfluence::Torso, -1.f );
		SetFaceToProxyDesire( m_TargetProxy, -100.f, FALSE );
	}

	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): the ranged-weapon item context is not ported; a shooting NPC aims and never fires") );
	}
}

void UDisAISubStateFirePistol::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	ClearLookAtDesire();
	ClearFaceToDesire();
}

void UDisAISubStateFirePistol::ChangePistolState( BYTE _NewState )
{
	m_AIPistolState = _NewState;
	m_fFireTimer = 0.f;
}

// DISHONORED(port): 2012 rva 0x76ba60-region: the aim follows the target while it is alive and reachable; a target that has
// gone means the NPC is done trying.
void UDisAISubStateFirePistol::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	if( !m_pTarget && !m_TargetProxy.IsValid() )
	{
		m_bDoneTrying = TRUE;
		m_bAllowExit = TRUE;
		return;
	}
	if( m_pTarget )
	{
		SetAimAtActorDesire( m_pTarget, FDisLookAtInfluence::Torso, -1.f );
	}
}

// DISHONORED(port): the exit condition: a controlled shot ends when its count runs out, a combat shot when the sub-state
// says it is done trying.
void UDisAISubStateFirePistol::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	m_fFireTimer += DeltaSeconds;
	if( m_bControlledFiring && !m_bShootForever && m_iMaxNumShots != INDEX_NONE && m_iMaxNumShots <= 0 )
	{
		m_bDoneTrying = TRUE;
	}
	if( m_bDoneTrying && m_bAllowExit )
	{
		RequestStateExit();
	}
}

// DISHONORED(port): 2012 rva 0x768a50: retail refuses to leave mid-shot, which is why the exit is deferred rather than
// taken; m_bAllowExit is what the firing state sets when the animation has finished.
void UDisAISubStateFirePistol::RequestStateExit_Derived()
{
	m_bDoneTrying = TRUE;
	if( m_bAllowExit )
	{
		Super::RequestStateExit_Derived();
	}
}

void UDisAISubStateFirePistol::ForceStateExit()
{
	m_bDoneTrying = TRUE;
	m_bAllowExit = TRUE;
	RequestStateExit();
}

// DISHONORED(port): 2012 rva 0x76b9c0-region: an NPC shoots at what it believes, not at what is there - which is how you
// can break line of sight and be shot at where you were.
FVector UDisAISubStateFirePistol::GetTargetLocation() const
{
	if( m_TargetProxy.IsValid() )
	{
		return m_TargetProxy.GetBestTargetLocation();
	}
	return m_pTarget ? m_pTarget->Location : FVector( 0.f, 0.f, 0.f );
}

UBOOL UDisAISubStateFirePistol::UsingLastSeenLocation() const
{
	return m_TargetProxy.IsValid() && !m_TargetProxy.IsVerifiedAsTarget();
}

// DISHONORED(port): 2012 rva 0x76b8a0-region
void UDisAISubStateFirePistol::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_pTarget )
	{
		FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
		if( Dispatcher )
		{
			const INT EventType = GDisAIEvent_OtherActorTerminated;
			Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
		}
		m_pTarget = NULL;
		m_bDoneTrying = TRUE;
		m_bAllowExit = TRUE;
	}
}

void UDisAISubStateFirePistol::BeginDestroy()
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( m_pTarget && Dispatcher )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateFirePistol::OnOtherActorTerminatedEvent );
	}
	Super::BeginDestroy();
}

// DISHONORED(port): a shooting NPC holds its ranged weapon in the aiming stance.
UBOOL UDisAISubStateFirePistol::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	_rResumingBodyIntention.m_IntendedBodyStance = eDisNPCBodyStance_Aiming;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = NULL;
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	return TRUE;
}
