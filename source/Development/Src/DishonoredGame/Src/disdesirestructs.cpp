// DishonoredGame/src/disdesirestructs.cpp
// ---- agent DF ports (PHASE10 DF): the desire layer ----
//
// A desire is a standing wish ("walk there", "face that", "look at that", "hold this stance") that an AI re-states every
// thought. Three of the four are carried by an FArkComponent request, and the whole point of this file is that the same
// wish re-stated a hundred times issues at most one order to that component: FDisDesireRequest::GetRequestStatus resolves
// the target as it is NOW, compares it with what the live request was issued with, and answers Unchanged /
// UpdateRequestNeeded / NewRequestNeeded / StopRequestNeeded. Everything else here is one of the four request kinds
// filling in its own parameters and then calling DoRequest with that answer.
//
// DISHONORED(bringup): FArkComponentFaceTo, FArkComponentLocomotion and FArkComponentLookat are NOT ported - locomotion
// alone is 117 functions and a package of its own (agentCG.md "What moving still needs"). ADishonoredNPCPawn's three
// component accessors therefore answer NULL, no request id is ever issued, and the Start / Update / Stop calls at the
// bottom of each DoRequest are replaced by DisDesireStructs::NoteComponentGap, which counts the order and names the
// retail entry point once. The layer above - target resolution, the redundancy filter and the status machine - is the
// faithful port, and it is what makes "what did this NPC ask for" a number in the -disai census.

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "arkcomponentlocomotion.h"
#include "disaisubstate.h"
#include "dishonoredutilities_ai.h"

INT GDisDesireRequests[DisDesireStructs::DDK_MAX] = { 0, 0, 0 };
INT GDisDesireUpdates[DisDesireStructs::DDK_MAX] = { 0, 0, 0 };
INT GDisDesireStops[DisDesireStructs::DDK_MAX] = { 0, 0, 0 };
INT GDisDesireBodyIntentions = 0;
INT GDisDesireSetCalls = 0;

/**
 * DISHONORED(bringup): retail asks UDishonoredInventory::HasItemByClass (2013 rva 0x80a250, 2012 0x855bc0), which is one
 * line over FindItemByClass (0x852180) - a walk of m_Slots comparing each item's class, its equip usage and its
 * removability. FindItemByClass belongs to UDisItemContext, agent AJ's third dependency root, and its slot layout is not
 * ported, so the question "does this NPC carry that item class" cannot be answered here. Answering TRUE keeps the body
 * intention the designer's tweaks asked for; answering FALSE would blank the weapon out of every armed stance on every
 * NPC, which is the worse of the two wrong answers. Named here so the site is findable when the inventory lands.
 */
static UBOOL DisNPCCarriesItemClass( ADishonoredNPCPawn* _pNPCPawn, UClass* _pItemClass )
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredInventory::HasItemByClass is not ported; a body intention's item classes are taken as carried") );
	}
	return TRUE;
}

/*-----------------------------------------------------------------------------
	FDisLookAtInfluence
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8a7d20 (2012 0x8f8da0)
FDisLookAtInfluence::FDisLookAtInfluence( FLOAT _fHeadInfluence, FLOAT _fTorsoInfluence, UBOOL _bTorsoInfIsMoveSpeedDependant )
	: m_fHeadInfluence( _fHeadInfluence )
	, m_fTorsoInfluence( _fTorsoInfluence )
{
	m_bTorsoInfIsMoveSpeedDependant = _bTorsoInfIsMoveSpeedDependant ? 1 : 0;
}

// DISHONORED(port): 2013 rva 0x8a7d50 (2012 0x8f8dd0): the float pair compares exactly (no epsilon), which is why
// SetParams can use it as a redundancy test at all.
UBOOL FDisLookAtInfluence::operator!=( const FDisLookAtInfluence& _rLookAtInfluence ) const
{
	return m_fHeadInfluence != _rLookAtInfluence.m_fHeadInfluence
		|| m_fTorsoInfluence != _rLookAtInfluence.m_fTorsoInfluence
		|| ( m_bTorsoInfIsMoveSpeedDependant != 0 ) != ( _rLookAtInfluence.m_bTorsoInfIsMoveSpeedDependant != 0 );
}

// DISHONORED(retail): the four named influence combinations, read off the 2012 .data image (see Inc/CppText/FDisLookAtInfluence.h).
const FDisLookAtInfluence FDisLookAtInfluence::Eyes( 0.f, 0.f, TRUE );
const FDisLookAtInfluence FDisLookAtInfluence::Head( 1.f, 0.f, TRUE );
const FDisLookAtInfluence FDisLookAtInfluence::Torso( 1.f, 1.f, TRUE );
const FDisLookAtInfluence FDisLookAtInfluence::TorsoSpeedIndependent( 1.f, 1.f, FALSE );

/*-----------------------------------------------------------------------------
	The component boundary
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(bringup): the synthetic component. Retail's answer to "has the component started this request" is
 * FArkComponent*::m_bStarted; with no component the contract is kept instead: an order that was accepted stays live until
 * it is stopped. Returning FALSE here (which is what a missing component literally means) made every request look
 * finished and re-issued it on the next tick.
 */
UBOOL DisDesireStructs::IsComponentRequestStarted( FPointer _Component )
{
	return TRUE;
}

static INT GDisDesireNextRequestID = 1;

INT DisDesireStructs::AcceptRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint )
{
	GDisDesireRequests[_Kind]++;
	NoteComponentGap( _Kind, _pRetailEntryPoint );
	return GDisDesireNextRequestID++;
}

void DisDesireStructs::ReleaseRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint )
{
	GDisDesireStops[_Kind]++;
	NoteComponentGap( _Kind, _pRetailEntryPoint );
}

void DisDesireStructs::UpdateRequest( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint )
{
	GDisDesireUpdates[_Kind]++;
	NoteComponentGap( _Kind, _pRetailEntryPoint );
}

/*-----------------------------------------------------------------------------
	agent DN: the locomotion half of the boundary, now real.
-----------------------------------------------------------------------------*/

/**
 * The event the component raises comes back here and is handed to whoever asked. Retail passes a pointer-to-member of the
 * asker (FArkCpntLocoRequestCommonProps::m_LocoEventCallback, `void (__thiscall UObject::*)(int, EArkCpntLocoEvent)`,
 * supplied by IDisDesiresInterface::GetDesiresLocoEventCallback); in the tree that accessor answers NULL on every class,
 * so the asker is resolved as the interface it is and told directly - which is exactly what
 * UDisAISubStateWithDesires::DesiresLocoEventCallback does with the pointer retail hands it.
 */
static void DisLocoEventTrampoline( UObject* _pCallbackOwner, const INT _RequestID, const BYTE _Event )
{
	if( !_pCallbackOwner )
	{
		return;
	}
	if( IDisDesiresInterface* Desires = InterfaceCast<IDisDesiresInterface>( _pCallbackOwner ) )
	{
		Desires->HandleLocoEvent( _Event );
	}
}

UBOOL DisDesireStructs::IsLocoRequestStarted( FPointer _Component, INT _RequestID )
{
	FArkComponentLocomotion* pLoco = (FArkComponentLocomotion*)_Component;
	if( !pLoco )
	{
		// DISHONORED(bringup): no component (a pawn whose tweaks name no locomotion config, which is retail's own case)
		// keeps agent DF's contract: an order that was accepted stays live until it is withdrawn. Answering FALSE here is
		// what produced 1,523,652 requests from 26 standing NPCs in 119 seconds.
		return TRUE;
	}
	return pLoco->IsRequestStarted( _RequestID );
}

/** The parameter block both Start and Update build. Retail's FDisLocoRequest::DoRequest fills the same fields. */
static void DisFillLocoCommonProps( FArkCpntLocoRequestCommonProps& _rOut, UObject* _pAsker, INT _MaxSpeedIndex,
	FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	_rOut.m_pCallbackOwner				= _pAsker;
	_rOut.m_LocoEventCallback			= &DisLocoEventTrampoline;
	_rOut.m_MaxSpeedIdx					= _MaxSpeedIndex;
	_rOut.m_EndSpeedIdx					= 0;
	_rOut.m_fEndLocationThreshold		= _fEndLocationThreshold;
	_rOut.m_fMaxFunnelRadiusMultiplier	= _fMaxFunnelRadiusMultiplier;
	_rOut.m_fSpeedMultiplier			= 1.f;
	_rOut.m_fStopBeforeEndDist			= 0.f;
	_rOut.m_bAccurateStop				= _bAccurateStop;
	_rOut.m_bSpeedIsLookAtDependent		= _bSpeedIsLookAtDependent;
}

INT DisDesireStructs::StartLoco( FPointer _Component, AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _LocationTarget,
	UObject* _pAsker, BYTE _Priority, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold,
	FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent,
	UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist )
{
	GDisDesireRequests[DDK_Loco]++;
	FArkComponentLocomotion* pLoco = (FArkComponentLocomotion*)_Component;
	if( !pLoco )
	{
		NoteComponentGap( DDK_Loco, TEXT("FArkComponentLocomotion::StartLoco (no component on this pawn)") );
		return GDisDesireNextRequestID++;
	}
	const FName AskerName = _pAsker ? _pAsker->GetFName() : NAME_None;
	if( _pActorTarget )
	{
		FArkCpntLocoGoToActorProp Prop;
		DisFillLocoCommonProps( Prop.m_CommonProps, _pAsker, _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
		Prop.m_ActorProps.m_pActor		= _pActorTarget;
		Prop.m_ActorProps.m_bFollow		= _bFollow;
		Prop.m_ActorProps.m_fFollowAngle= _fFollowAngle;
		Prop.m_ActorProps.m_fFollowDist	= _fFollowDist;
		return pLoco->StartLocoToActor( _pAsker, AskerName, (INT)_Priority, Prop );
	}
	if( _bUsingLocation )
	{
		FArkCpntLocoGoToLocationProp Prop;
		DisFillLocoCommonProps( Prop.m_CommonProps, _pAsker, _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
		Prop.m_LocationProps.m_Location.Set( _LocationTarget );
		return pLoco->StartLocoToLocation( _pAsker, AskerName, (INT)_Priority, Prop );
	}
	return INDEX_NONE;
}

UBOOL DisDesireStructs::UpdateLoco( FPointer _Component, INT _RequestID, AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _LocationTarget,
	UObject* _pAsker, BYTE _Priority, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold,
	FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent,
	UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist )
{
	GDisDesireUpdates[DDK_Loco]++;
	FArkComponentLocomotion* pLoco = (FArkComponentLocomotion*)_Component;
	if( !pLoco )
	{
		NoteComponentGap( DDK_Loco, TEXT("FArkComponentLocomotion::UpdateLoco (no component on this pawn)") );
		return TRUE;
	}
	if( _pActorTarget )
	{
		FArkCpntLocoGoToActorProp Prop;
		DisFillLocoCommonProps( Prop.m_CommonProps, _pAsker, _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
		Prop.m_ActorProps.m_pActor		= _pActorTarget;
		Prop.m_ActorProps.m_bFollow		= _bFollow;
		Prop.m_ActorProps.m_fFollowAngle= _fFollowAngle;
		Prop.m_ActorProps.m_fFollowDist	= _fFollowDist;
		return pLoco->UpdateLocoToActor( _RequestID, Prop );
	}
	if( _bUsingLocation )
	{
		FArkCpntLocoGoToLocationProp Prop;
		DisFillLocoCommonProps( Prop.m_CommonProps, _pAsker, _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
		Prop.m_LocationProps.m_Location.Set( _LocationTarget );
		return pLoco->UpdateLocoToLocation( _RequestID, Prop );
	}
	return FALSE;
}

void DisDesireStructs::StopLoco( FPointer _Component, INT _RequestID )
{
	GDisDesireStops[DDK_Loco]++;
	FArkComponentLocomotion* pLoco = (FArkComponentLocomotion*)_Component;
	if( !pLoco )
	{
		NoteComponentGap( DDK_Loco, TEXT("FArkComponentLocomotion::StopLoco (no component on this pawn)") );
		return;
	}
	pLoco->StopLoco( _RequestID );
}

void DisDesireStructs::NoteComponentGap( EDisDesireKind _Kind, const TCHAR* _pRetailEntryPoint )
{
	static UBOOL bNoted[DDK_MAX] = { FALSE, FALSE, FALSE };
	if( !bNoted[_Kind] )
	{
		bNoted[_Kind] = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): the AI issued a desire but %s is not ported; the request is resolved and counted, not executed"),
			_pRetailEntryPoint );
	}
}

/*-----------------------------------------------------------------------------
	FDisDesireRequest - the target half
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8a8100 (2012 0x8f90f0)
UBOOL DisDesireStructs::ResolveProxyTarget( const FDisAttentionProxy& _rProxyTarget, FVector& _rLocationTarget )
{
	_rLocationTarget = FVector( 0.f, 0.f, 0.f );
	if( !_rProxyTarget.IsValid() || _rProxyTarget.IsIndeterminate() )
	{
		return FALSE;
	}
	_rLocationTarget = _rProxyTarget.GetBestTargetLocation();
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x8b3e30 (2012 0x900450)
EDisDesireRequestStatus FDisDesireRequest::ClearTarget()
{
	EDisDesireRequestStatus Status = DTDRS_Unchanged;
	if( !m_bDesired )
	{
		return Status;
	}
	m_bDesired = FALSE;
	m_bUsingProxy = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	m_bUsingLocation = FALSE;
	if( m_RequestID != INDEX_NONE )
	{
		Status = DTDRS_StopRequestNeeded;
	}
	m_pActorTarget = NULL;
	m_LocationTarget = FVector( 0.f, 0.f, 0.f );
	RegisterToActorTerminationIfNeeded();
	return Status;
}

// DISHONORED(port): 2013 rva 0x8b3e90 (2012 0x9004b0), 881 bytes. Reads left to right: resolve whichever target kind is
// set into (NewActorTarget, NewLocationTarget); then decide, against what the live request holds, whether the component
// needs a new order, an update, a stop, or nothing. The four target kinds are mutually exclusive and tested in retail's
// order - proxy, look-at interface, the caller's actor, the caller's location.
EDisDesireRequestStatus FDisDesireRequest::GetRequestStatus( AActor* _pActorTarget, UBOOL _bUsingLocation, const FVector& _rLocationTarget )
{
	EDisDesireRequestStatus Status = DTDRS_Unchanged;

	if( !m_bDesired )
	{
		// Nothing is wanted any more: a live request has to be stopped, and the resolved target is dropped.
		if( m_RequestID != INDEX_NONE )
		{
			m_pActorTarget = NULL;
			m_LocationTarget = FVector( 0.f, 0.f, 0.f );
			m_bUsingLocation = FALSE;
			Status = DTDRS_StopRequestNeeded;
			RegisterToActorTerminationIfNeeded();
		}
		return Status;
	}

	AActor* NewActorTarget = NULL;
	FVector NewLocationTarget( 0.f, 0.f, 0.f );
	UBOOL bNewUsingLocation = FALSE;

	if( m_bUsingProxy )
	{
		bNewUsingLocation = DisDesireStructs::ResolveProxyTarget( m_ProxyTarget, NewLocationTarget );
	}
	else if( m_pLookAtTarget.GetObject() && m_pLookAtTarget.GetInterface() )
	{
		bNewUsingLocation = TRUE;
			NewLocationTarget = DisGetScriptInterface( m_pLookAtTarget )->GetLookAtLocation();
	}
	else if( _pActorTarget )
	{
		NewActorTarget = _pActorTarget;
	}
	else if( _bUsingLocation )
	{
		bNewUsingLocation = TRUE;
		NewLocationTarget = _rLocationTarget;
	}

	if( m_RequestID == INDEX_NONE )
	{
		// No live request: record what was resolved and ask for a new one unless the target resolved to nothing at all.
		m_pActorTarget = NewActorTarget;
		m_bUsingLocation = bNewUsingLocation;
		m_LocationTarget = NewLocationTarget;
		RegisterToActorTerminationIfNeeded();
		if( !m_bPaused && ( m_pActorTarget || m_bUsingLocation || !m_bUsingProxy ) )
		{
			Status = DTDRS_NewRequestNeeded;
		}
		return Status;
	}

	if( m_pActorTarget )
	{
		if( NewActorTarget )
		{
			// Same kind of target as the live request: only a different actor needs an update.
			if( NewActorTarget != m_pActorTarget )
			{
				m_pActorTarget = NewActorTarget;
				Status = DTDRS_UpdateRequestNeeded;
				RegisterToActorTerminationIfNeeded();
			}
			return Status;
		}
		m_pActorTarget = NULL;
		if( bNewUsingLocation )
		{
			m_bUsingLocation = TRUE;
			m_LocationTarget = NewLocationTarget;
			Status = DTDRS_NewRequestNeeded;
		}
		else
		{
			// An actor target that has resolved to nothing: a proxy that has gone indeterminate stops the request,
			// anything else re-issues it against the request's own (now empty) target.
			Status = m_bUsingProxy ? DTDRS_StopRequestNeeded : DTDRS_NewRequestNeeded;
		}
		RegisterToActorTerminationIfNeeded();
		return Status;
	}

	if( !m_bUsingLocation )
	{
		if( NewActorTarget )
		{
			m_pActorTarget = NewActorTarget;
			Status = DTDRS_NewRequestNeeded;
			RegisterToActorTerminationIfNeeded();
			return Status;
		}
		if( bNewUsingLocation )
		{
			m_bUsingLocation = TRUE;
			m_LocationTarget = NewLocationTarget;
			return DTDRS_NewRequestNeeded;
		}
		return m_bUsingProxy ? DTDRS_StopRequestNeeded : Status;
	}

	if( NewActorTarget )
	{
		m_pActorTarget = NewActorTarget;
		m_bUsingLocation = FALSE;
		m_LocationTarget = FVector( 0.f, 0.f, 0.f );
		Status = DTDRS_NewRequestNeeded;
		RegisterToActorTerminationIfNeeded();
		return Status;
	}
	if( bNewUsingLocation )
	{
		// A moving location target (a proxy tracking a pawn) updates the live request rather than restarting it.
		if( m_LocationTarget != NewLocationTarget )
		{
			m_LocationTarget = NewLocationTarget;
			return DTDRS_UpdateRequestNeeded;
		}
		return Status;
	}
	m_bUsingLocation = FALSE;
	m_LocationTarget = FVector( 0.f, 0.f, 0.f );
	return m_bUsingProxy ? DTDRS_StopRequestNeeded : DTDRS_NewRequestNeeded;
}

// DISHONORED(port): 2013 rva 0x8b74e0 (2012 0x901cc0): the same (brain, target) pair is not a change, which is what stops
// an attention-driven desire from restarting every thought.
EDisDesireRequestStatus FDisDesireRequest::SetProxyTarget( const FDisAttentionProxy& _rProxyTarget )
{
	if( m_bUsingProxy && m_ProxyTarget.m_pBrain == _rProxyTarget.m_pBrain
		&& m_ProxyTarget.GetProxyAttnTarget() == _rProxyTarget.GetProxyAttnTarget()
		&& m_ProxyTarget.m_pTarget.GetObject() == _rProxyTarget.m_pTarget.GetObject() )
	{
		return DTDRS_Unchanged;
	}
	m_bDesired = TRUE;
	m_bUsingProxy = TRUE;
	m_ProxyTarget = _rProxyTarget;
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	const EDisDesireRequestStatus Status = GetRequestStatus( NULL, FALSE, FVector( 0.f, 0.f, 0.f ) );
	RegisterToActorTerminationIfNeeded();
	return Status;
}

// DISHONORED(port): 2013 rva 0x8b7570 (2012 0x901d50)
EDisDesireRequestStatus FDisDesireRequest::SetLookAtTarget( IDisLookAtInterface* _pLookAtTarget )
{
	IDisLookAtInterface* Current = DisGetScriptInterface( m_pLookAtTarget );
	if( Current == _pLookAtTarget )
	{
		return DTDRS_Unchanged;
	}
	m_bDesired = TRUE;
	m_bUsingProxy = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	// DISHONORED(port): retail goes through ToScriptInterface<IDisLookAtInterface> (2012 rva 0x8fd0a0).
	DisSetScriptInterface( m_pLookAtTarget, _pLookAtTarget ? _pLookAtTarget->GetUObjectInterfaceDisLookAtInterface() : NULL );
	const EDisDesireRequestStatus Status = GetRequestStatus( NULL, FALSE, FVector( 0.f, 0.f, 0.f ) );
	RegisterToActorTerminationIfNeeded();
	return Status;
}

// DISHONORED(port): 2013 rva 0x8b75f0 (2012 0x901dd0)
EDisDesireRequestStatus FDisDesireRequest::SetActorTarget( AActor* _pActorTarget )
{
	if( m_pActorTarget == _pActorTarget )
	{
		return DTDRS_Unchanged;
	}
	m_bDesired = TRUE;
	m_bUsingProxy = FALSE;
	m_bUsingLocation = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	const EDisDesireRequestStatus Status = GetRequestStatus( _pActorTarget, FALSE, FVector( 0.f, 0.f, 0.f ) );
	RegisterToActorTerminationIfNeeded();
	return Status;
}

// DISHONORED(port): 2013 rva 0x8b7650 (2012 0x901e30): the exact-equality test is what makes re-stating the same
// destination free.
EDisDesireRequestStatus FDisDesireRequest::SetLocationTarget( const FVector& _rLocationTarget )
{
	if( !m_bUsingProxy
		&& !( m_pLookAtTarget.GetObject() && m_pLookAtTarget.GetInterface() )
		&& m_bUsingLocation
		&& m_LocationTarget.X == _rLocationTarget.X
		&& m_LocationTarget.Y == _rLocationTarget.Y
		&& m_LocationTarget.Z == _rLocationTarget.Z )
	{
		return DTDRS_Unchanged;
	}
	m_bDesired = TRUE;
	m_bUsingProxy = FALSE;
	m_bUsingLocation = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	const EDisDesireRequestStatus Status = GetRequestStatus( NULL, TRUE, _rLocationTarget );
	RegisterToActorTerminationIfNeeded();
	return Status;
}

// DISHONORED(port): 2013 rva 0x8a9ad0 (2012 0x8f9ec0)
UBOOL FDisDesireRequest::ReferencesActor( const AActor* _pActor ) const
{
	if( !m_bDesired )
	{
		return FALSE;
	}
	if( m_bUsingProxy && _pActor && m_ProxyTarget.IsEqualToActor( *_pActor ) )
	{
		return TRUE;
	}
	if( m_pLookAtTarget.GetObject() && m_pLookAtTarget.GetInterface()
		&& DisGetScriptInterface( m_pLookAtTarget )->GetLookAtOwnerActor() == _pActor )
	{
		return TRUE;
	}
	return m_pActorTarget == _pActor;
}

// DISHONORED(port): 2013 rva 0x8b3d70 (2012 0x900390): unregister unconditionally, then register again only while the
// request holds some actor reference, so the dispatcher's list never grows for location-only desires.
void FDisDesireRequest::RegisterToActorTerminationIfNeeded()
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( !Dispatcher )
	{
		// DISHONORED(bringup): the dispatcher singleton is created in FEngineLoop::PreInit (2013 rva 0x5e249d calls
		// FArkGameEventDispatcher::CreateInstance 0x5572d0); this guard is what made the desire layer safe to bring up
		// before that line existed.
		return;
	}
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	Dispatcher->UnregisterToEvent( EventType, this, &FDisDesireRequest::OnOtherActorTerminatedEvent );
	if( m_ProxyTarget.HasActorReference()
		|| ( m_pLookAtTarget.GetObject() && m_pLookAtTarget.GetInterface() )
		|| m_pActorTarget )
	{
		Dispatcher->RegisterToEvent( EventType, this, &FDisDesireRequest::OnOtherActorTerminatedEvent );
	}
}

// DISHONORED(port): 2013 rva 0x8b2390 (2012 0x8ffb20): one handler, three kinds. The discriminator bits are set by each
// kind's Initialize, which is why a plain FDisDesireRequest (there is none in retail) would do nothing here.
void FDisDesireRequest::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	const AActor* Terminated = Cast<AActor>( _rEvent.m_pInstigator );
	if( m_bIsFaceToRequest )
	{
		if( ReferencesActor( Terminated ) )
		{
			( (FDisFaceToRequest*)this )->ClearRequest();
		}
	}
	else if( m_bIsLocoRequest )
	{
		if( ReferencesActor( Terminated ) )
		{
			( (FDisLocoRequest*)this )->ClearRequest();
		}
	}
	else if( m_bIsLookAtRequest )
	{
		if( ReferencesActor( Terminated ) )
		{
			( (FDisLookAtRequest*)this )->ClearRequest();
		}
	}
}

// DISHONORED(port): 2013 rva 0x8b3de0 (2012 0x900400)
FDisDesireRequest::~FDisDesireRequest()
{
	if( !( m_ProxyTarget.HasActorReference()
		|| ( m_pLookAtTarget.GetObject() && m_pLookAtTarget.GetInterface() )
		|| m_pActorTarget ) )
	{
		return;
	}
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		Dispatcher->UnregisterToEvent( EventType, this, &FDisDesireRequest::OnOtherActorTerminatedEvent );
	}
}

/*-----------------------------------------------------------------------------
	FDisFaceToRequest
-----------------------------------------------------------------------------*/

void FDisFaceToRequest::ClearParams()
{
	m_bUsingYaw = FALSE;
	m_bExactRotation = FALSE;
	m_YawTarget = 0;
	m_fRotationSpeed = -100.f;
}

// DISHONORED(port): retail inlines this at eight call sites: a request the component has already finished is forgotten
// before anything else is decided.
void FDisFaceToRequest::SyncRequest()
{
	if( m_RequestID != INDEX_NONE && !DisDesireStructs::IsComponentRequestStarted( m_pFaceToComponent ) )
	{
		m_RequestID = INDEX_NONE;
		ClearTarget();
		ClearParams();
	}
}

// DISHONORED(port): 2013 rva 0x8b4230 (2012 0x900830)
void FDisFaceToRequest::Initialize( FArkComponentFaceTo* _pFaceToComponent, UObject* _pAsker, BYTE _Priority, void* _pCallback )
{
	if( m_pFaceToComponent )
	{
		ClearRequest();
		m_bPaused = FALSE;
		m_pFaceToComponent = NULL;
		m_pAsker = NULL;
		m_Priority = 0;
		m_pCallback = NULL;
	}
	m_bIsFaceToRequest = TRUE;
	m_pFaceToComponent = (FPointer)_pFaceToComponent;
	m_pAsker = _pAsker;
	m_Priority = _Priority;
	m_pCallback = (FPointer)_pCallback;
}

// DISHONORED(port): 2013 rva 0x8a7e00 (2012 0x8f8e20)
void FDisFaceToRequest::SetParams( UBOOL _bUsingYaw, INT _YawTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation, EDisDesireRequestStatus& _rStatus )
{
	if( ( m_bUsingYaw != 0 ) == ( _bUsingYaw != 0 )
		&& m_YawTarget == _YawTarget
		&& m_fRotationSpeed == _fRotationSpeed
		&& ( m_bExactRotation != 0 ) == ( _bExactRotation != 0 ) )
	{
		return;
	}
	m_YawTarget = _YawTarget;
	m_fRotationSpeed = _fRotationSpeed;
	m_bUsingYaw = _bUsingYaw ? 1 : 0;
	m_bExactRotation = _bExactRotation ? 1 : 0;
	if( _rStatus == DTDRS_Unchanged && m_RequestID != INDEX_NONE && !m_bPaused )
	{
		_rStatus = DTDRS_UpdateRequestNeeded;
	}
}

// DISHONORED(port): 2013 rva 0x8b76f0 (2012 0x901ed0)
void FDisFaceToRequest::RequestProxyTarget( const FDisAttentionProxy& _rProxyTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetProxyTarget( _rProxyTarget );
	SetParams( FALSE, 0, _fRotationSpeed, _bExactRotation, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7770 (2012 0x901f50)
void FDisFaceToRequest::RequestActorTarget( AActor* _pActorTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetActorTarget( _pActorTarget );
	SetParams( FALSE, 0, _fRotationSpeed, _bExactRotation, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b77f0 (2012 0x901fd0)
void FDisFaceToRequest::RequestLocationTarget( const FVector& _rLocationTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetLocationTarget( _rLocationTarget );
	SetParams( FALSE, 0, _fRotationSpeed, _bExactRotation, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7870 (2012 0x902050): a yaw target has no actor and no location, so retail spells out the
// "desired, no proxy, no look-at target" part of SetActorTarget rather than calling it.
void FDisFaceToRequest::RequestYawTarget( INT _YawTarget, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	SyncRequest();
	GDisDesireSetCalls++;
	m_bDesired = TRUE;
	m_bUsingProxy = FALSE;
	m_bUsingLocation = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	EDisDesireRequestStatus Status = GetRequestStatus( NULL, FALSE, FVector( 0.f, 0.f, 0.f ) );
	RegisterToActorTerminationIfNeeded();
	SetParams( TRUE, _YawTarget, _fRotationSpeed, _bExactRotation, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7920 (2012 0x902100): the per-tick pass - re-resolve the target and act only on a change.
void FDisFaceToRequest::UpdateRequest()
{
	SyncRequest();
	const EDisDesireRequestStatus Status = GetRequestStatus( m_pActorTarget, m_bUsingLocation, m_LocationTarget );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ae0f0 (2012 0x8fccb0)
void FDisFaceToRequest::ClearRequest()
{
	SyncRequest();
	const EDisDesireRequestStatus Status = ClearTarget();
	ClearParams();
	if( Status == DTDRS_StopRequestNeeded )
	{
		StopRequest();
	}
}

// DISHONORED(port): 2013 rva 0x8ae080 (2012 0x8fcc40): pausing stops the component but keeps the target, so ResumeDesires
// can re-issue exactly the same order.
void FDisFaceToRequest::PauseRequest()
{
	if( m_bPaused )
	{
		return;
	}
	SyncRequest();
	if( m_RequestID != INDEX_NONE )
	{
		StopRequest();
	}
	m_bPaused = TRUE;
}

// DISHONORED(port): 2013 rva 0x8a7e80 (2012 0x8f8ea0)
void FDisFaceToRequest::StopRequest()
{
	DisDesireStructs::ReleaseRequest( DisDesireStructs::DDK_FaceTo, TEXT("FArkComponentFaceTo::StopFaceTo") );
	m_RequestID = INDEX_NONE;
	m_DebugLastFaceToEvent = 0;
}

// DISHONORED(port): 2013 rva 0x8a9b30 (2012 0x8f9f20): three component entry points, chosen by which target kind is set -
// actor, then location, then yaw.
// DISHONORED(bringup): FArkComponentFaceTo is not ported, so each branch builds nothing and counts the order instead.
void FDisFaceToRequest::DoRequest( EDisDesireRequestStatus _Status )
{
	if( _Status == DTDRS_Unchanged )
	{
		return;
	}
	if( _Status == DTDRS_StopRequestNeeded )
	{
		StopRequest();
		return;
	}

	const TCHAR* EntryPoint = NULL;
	if( m_pActorTarget )
	{
		EntryPoint = ( _Status == DTDRS_UpdateRequestNeeded ) ? TEXT("FArkComponentFaceTo::UpdateFaceToActor") : TEXT("FArkComponentFaceTo::StartFaceToActor");
	}
	else if( m_bUsingLocation )
	{
		EntryPoint = ( _Status == DTDRS_UpdateRequestNeeded ) ? TEXT("FArkComponentFaceTo::UpdateFaceToLocation") : TEXT("FArkComponentFaceTo::StartFaceToLocation");
	}
	else if( m_bUsingYaw )
	{
		EntryPoint = ( _Status == DTDRS_UpdateRequestNeeded ) ? TEXT("FArkComponentFaceTo::UpdateFaceToYaw") : TEXT("FArkComponentFaceTo::StartFaceToYaw");
	}
	else
	{
		return;
	}

	if( _Status == DTDRS_UpdateRequestNeeded )
	{
		DisDesireStructs::UpdateRequest( DisDesireStructs::DDK_FaceTo, EntryPoint );
	}
	else
	{
		if( m_RequestID != INDEX_NONE )
		{
			StopRequest();
		}
		m_RequestID = DisDesireStructs::AcceptRequest( DisDesireStructs::DDK_FaceTo, EntryPoint );
	}
}

/*-----------------------------------------------------------------------------
	FDisLocoRequest
-----------------------------------------------------------------------------*/

void FDisLocoRequest::ClearParams()
{
	m_fEndLocationThreshold = -1.f;
	m_fMaxFunnelRadiusMultiplier = 1.f;
	m_MaxSpeedIndex = 0;
	m_bAccurateStop = TRUE;
	m_bSpeedIsLookAtDependent = FALSE;
	m_bFollow = FALSE;
	m_fFollowAngle = 0.f;
	m_fFollowDist = 0.f;
}

void FDisLocoRequest::SyncRequest()
{
	if( m_RequestID != INDEX_NONE && !DisDesireStructs::IsLocoRequestStarted( m_pLocoComponent, m_RequestID ) )
	{
		m_RequestID = INDEX_NONE;
		ClearTarget();
		ClearParams();
	}
}

// DISHONORED(port): 2013 rva 0x8b4280 (2012 0x900880)
void FDisLocoRequest::Initialize( FArkComponentLocomotion* _pLocoComponent, UObject* _pAsker, BYTE _Priority, void* _pCallback )
{
	if( m_pLocoComponent )
	{
		ClearRequest();
		m_bPaused = FALSE;
		m_pLocoComponent = NULL;
		m_pAsker = NULL;
		m_Priority = 0;
		m_pCallback = NULL;
	}
	m_bIsLocoRequest = TRUE;
	m_pLocoComponent = (FPointer)_pLocoComponent;
	m_pAsker = _pAsker;
	m_Priority = _Priority;
	m_pCallback = (FPointer)_pCallback;
}

// DISHONORED(port): 2013 rva 0x8a7ea0 (2012 0x8f8ec0)
void FDisLocoRequest::SetParams( INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent, UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist, EDisDesireRequestStatus& _rStatus )
{
	if( m_MaxSpeedIndex == _MaxSpeedIndex
		&& m_fEndLocationThreshold == _fEndLocationThreshold
		&& m_fMaxFunnelRadiusMultiplier == _fMaxFunnelRadiusMultiplier
		&& ( m_bAccurateStop != 0 ) == ( _bAccurateStop != 0 )
		&& ( m_bSpeedIsLookAtDependent != 0 ) == ( _bSpeedIsLookAtDependent != 0 )
		&& ( m_bFollow != 0 ) == ( _bFollow != 0 )
		&& m_fFollowAngle == _fFollowAngle
		&& m_fFollowDist == _fFollowDist )
	{
		return;
	}
	m_MaxSpeedIndex = _MaxSpeedIndex;
	m_fEndLocationThreshold = _fEndLocationThreshold;
	m_fMaxFunnelRadiusMultiplier = _fMaxFunnelRadiusMultiplier;
	m_bAccurateStop = _bAccurateStop ? 1 : 0;
	m_bSpeedIsLookAtDependent = _bSpeedIsLookAtDependent ? 1 : 0;
	m_bFollow = _bFollow ? 1 : 0;
	m_fFollowAngle = _fFollowAngle;
	m_fFollowDist = _fFollowDist;
	if( _rStatus == DTDRS_Unchanged && m_RequestID != INDEX_NONE && !m_bPaused )
	{
		_rStatus = DTDRS_UpdateRequestNeeded;
	}
}

// DISHONORED(port): 2013 rva 0x8b7980 (2012 0x902160)
void FDisLocoRequest::RequestProxyTarget( const FDisAttentionProxy& _rProxyTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetProxyTarget( _rProxyTarget );
	SetParams( _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent, FALSE, 0.f, 0.f, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7a30 (2012 0x902210)
void FDisLocoRequest::RequestActorTarget( AActor* _pActorTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetActorTarget( _pActorTarget );
	SetParams( _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent, FALSE, 0.f, 0.f, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7ba0 (2012 0x902380)
void FDisLocoRequest::RequestLocationTarget( const FVector& _rLocationTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetLocationTarget( _rLocationTarget );
	SetParams( _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent, FALSE, 0.f, 0.f, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7ae0 (2012 0x9022c0): following keeps an angle and a distance behind the target instead
// of walking onto it.
void FDisLocoRequest::RequestFollowTarget( AActor* _pActorTarget, INT _MaxSpeedIndex, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent, UBOOL _bFollow, FLOAT _fFollowAngle, FLOAT _fFollowDist )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetActorTarget( _pActorTarget );
	SetParams( _MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent, _bFollow, _fFollowAngle, _fFollowDist, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7c50 (2012 0x902430)
void FDisLocoRequest::UpdateRequest()
{
	SyncRequest();
	const EDisDesireRequestStatus Status = GetRequestStatus( m_pActorTarget, m_bUsingLocation, m_LocationTarget );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ae200 (2012 0x8fcdc0)
void FDisLocoRequest::ClearRequest()
{
	SyncRequest();
	const EDisDesireRequestStatus Status = ClearTarget();
	ClearParams();
	if( Status == DTDRS_StopRequestNeeded )
	{
		StopRequest();
	}
}

// DISHONORED(port): 2013 rva 0x8ae170 (2012 0x8fcd30)
void FDisLocoRequest::PauseRequest()
{
	if( m_bPaused )
	{
		return;
	}
	SyncRequest();
	if( m_RequestID != INDEX_NONE )
	{
		StopRequest();
	}
	m_bPaused = TRUE;
}

// DISHONORED(port): 2013 rva 0x8a7f80 (2012 0x8f8fa0)
void FDisLocoRequest::StopRequest()
{
	// DISHONORED(port): agent DN. The order reaches the real component now.
	if( m_RequestID != INDEX_NONE )
	{
		DisDesireStructs::StopLoco( m_pLocoComponent, m_RequestID );
	}
	m_RequestID = INDEX_NONE;
	m_DebugLastLocoEvent = 0;
}

// DISHONORED(port): 2013 rva 0x8a9d80 (2012 0x8fa170): two component entry points, actor or location.
// DISHONORED(bringup): FArkComponentLocomotion is the next package (agentCG.md hand-over 3). Until it exists this is where
// every move an NPC wants ends up, counted per kind, which is the honest answer to "the NPCs cannot walk yet".
void FDisLocoRequest::DoRequest( EDisDesireRequestStatus _Status )
{
	if( _Status == DTDRS_Unchanged )
	{
		return;
	}
	if( _Status == DTDRS_StopRequestNeeded )
	{
		StopRequest();
		return;
	}

	if( !m_pActorTarget && !m_bUsingLocation )
	{
		return;
	}

	// DISHONORED(port): agent DN. Retail's two component entry points, chosen by target kind, reached for real.
	if( _Status == DTDRS_UpdateRequestNeeded )
	{
		if( !DisDesireStructs::UpdateLoco( m_pLocoComponent, m_RequestID, m_pActorTarget, m_bUsingLocation, m_LocationTarget,
				m_pAsker, m_Priority, m_MaxSpeedIndex, m_fEndLocationThreshold, m_fMaxFunnelRadiusMultiplier,
				m_bAccurateStop, m_bSpeedIsLookAtDependent, m_bFollow, m_fFollowAngle, m_fFollowDist ) )
		{
			// The component has forgotten the request (it was completed and removed): a new one is needed.
			m_RequestID = INDEX_NONE;
		}
	}
	else
	{
		if( m_RequestID != INDEX_NONE )
		{
			StopRequest();
		}
		m_RequestID = DisDesireStructs::StartLoco( m_pLocoComponent, m_pActorTarget, m_bUsingLocation, m_LocationTarget,
			m_pAsker, m_Priority, m_MaxSpeedIndex, m_fEndLocationThreshold, m_fMaxFunnelRadiusMultiplier,
			m_bAccurateStop, m_bSpeedIsLookAtDependent, m_bFollow, m_fFollowAngle, m_fFollowDist );
	}
}

/*-----------------------------------------------------------------------------
	FDisLookAtRequest
-----------------------------------------------------------------------------*/

void FDisLookAtRequest::ClearParams()
{
	m_bLocal = FALSE;
	m_bAimAtTarget = FALSE;
	m_ProceduralPatternIndex = DisLookAtProceduralPattern_MAX;
	m_LookAtInfluence = FDisLookAtInfluence::Torso;
	m_fDuration = -1.f;
}

// DISHONORED(port): 2013 rva 0x8b42d0 (2012 0x9008d0): unlike the other two kinds a look-at also expires - a request with
// a positive duration keeps the component's remaining time, and when that reaches zero the request is dropped.
void FDisLookAtRequest::SyncRequest()
{
	if( m_RequestID == INDEX_NONE )
	{
		return;
	}
	UBOOL bFinished = !DisDesireStructs::IsComponentRequestStarted( m_pLookAtComponent );
	// DISHONORED(bringup): retail reads the remaining time back from FArkComponentLookat::GetTimeLeft and drops the request
	// when it hits zero. The synthetic component has no clock, so a look-at with a positive duration is expired by
	// TickRequest below instead - which is what retail's paused path does with the same field.
	if( bFinished )
	{
		m_RequestID = INDEX_NONE;
		ClearTarget();
		ClearParams();
	}
}

// DISHONORED(port): 2013 rva 0x8b2400 (2012 0x8ffb90)
void FDisLookAtRequest::Initialize( FArkComponentLookat* _pLookAtComponent, UObject* _pAsker, BYTE _Priority )
{
	Finalize();
	m_bIsLookAtRequest = TRUE;
	m_pLookAtComponent = (FPointer)_pLookAtComponent;
	m_pAsker = _pAsker;
	m_Priority = _Priority;
}

// DISHONORED(port): 2013 rva 0x8ae2c0 (2012 0x8fce80)
void FDisLookAtRequest::Finalize()
{
	if( m_pLookAtComponent )
	{
		ClearRequest();
		m_bPaused = FALSE;
		m_pLookAtComponent = NULL;
		m_pAsker = NULL;
		m_Priority = 0;
	}
}

// DISHONORED(port): 2013 rva 0x8a7fa0 (2012 0x8f8fc0)
void FDisLookAtRequest::SetParams( UBOOL _bLocal, UBOOL _bAimAtTarget, BYTE _ProceduralPatternIndex, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration, EDisDesireRequestStatus& _rStatus )
{
	if( ( m_bLocal != 0 ) == ( _bLocal != 0 )
		&& ( m_bAimAtTarget != 0 ) == ( _bAimAtTarget != 0 )
		&& m_ProceduralPatternIndex == _ProceduralPatternIndex
		&& !( m_LookAtInfluence != _rLookAtInfluence )
		&& m_fDuration == _fDuration )
	{
		return;
	}
	m_ProceduralPatternIndex = _ProceduralPatternIndex;
	m_bLocal = _bLocal ? 1 : 0;
	m_bAimAtTarget = _bAimAtTarget ? 1 : 0;
	m_LookAtInfluence = _rLookAtInfluence;
	m_fDuration = _fDuration;
	if( _rStatus == DTDRS_Unchanged && m_RequestID != INDEX_NONE && !m_bPaused )
	{
		_rStatus = DTDRS_UpdateRequestNeeded;
	}
}

// DISHONORED(port): the target half shared by RequestActorTarget and RequestActorAimTarget (2013 rvas 0x8b7d20 /
// 0x8b7da0): an actor that implements IDisLookAtInterface is held through the interface, everything else as a plain actor.
EDisDesireRequestStatus FDisLookAtRequest::SetActorOrLookAtTarget( AActor* _pActorTarget )
{
	IDisLookAtInterface* LookAtTarget = _pActorTarget ? InterfaceCast< IDisLookAtInterface >( _pActorTarget ) : NULL;
	if( LookAtTarget )
	{
		return SetLookAtTarget( LookAtTarget );
	}
	return SetActorTarget( _pActorTarget );
}

// DISHONORED(port): 2013 rva 0x8b7cd0 (2012 0x9024b0)
void FDisLookAtRequest::RequestProxyTarget( const FDisAttentionProxy& _rProxyTarget, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetProxyTarget( _rProxyTarget );
	SetParams( FALSE, FALSE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7d20 (2012 0x902500): an actor that implements IDisLookAtInterface is held through the
// interface, so the look-at follows the socket the actor nominates rather than its origin.
void FDisLookAtRequest::RequestActorTarget( AActor* _pActorTarget, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetActorOrLookAtTarget( _pActorTarget );
	SetParams( FALSE, FALSE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7da0 (2012 0x902580): the same, with the aim flag set - a ranged NPC aims where it looks.
void FDisLookAtRequest::RequestActorAimTarget( AActor* _pActorTarget, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	SyncRequest();
	GDisDesireSetCalls++;
	EDisDesireRequestStatus Status = SetActorOrLookAtTarget( _pActorTarget );
	SetParams( FALSE, TRUE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8b7e20 (2012 0x902600): a procedural look-at has no target at all - the pattern index
// names a scripted sweep (patrol, searching with an empty hand, searching with a drawn sword, looking at a target).
void FDisLookAtRequest::RequestProceduralLookAt( BYTE _ProceduralPatternIndex, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	SyncRequest();
	GDisDesireSetCalls++;
	m_bDesired = TRUE;
	m_bUsingProxy = FALSE;
	m_bUsingLocation = FALSE;
	m_ProxyTarget.ClearAttnProxy();
	DisSetScriptInterface( m_pLookAtTarget, (UObject*)NULL );
	EDisDesireRequestStatus Status = GetRequestStatus( NULL, FALSE, FVector( 0.f, 0.f, 0.f ) );
	RegisterToActorTerminationIfNeeded();
	SetParams( FALSE, FALSE, _ProceduralPatternIndex, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba9d0 (2012 0x905b10): a paused look-at still runs its duration down, so a glance issued
// just before the sub-state was paused does not come back when it resumes.
void FDisLookAtRequest::TickRequest( FLOAT _fDeltaTime )
{
	if( m_fDuration > 0.f )
	{
		m_fDuration -= _fDeltaTime;
		if( m_fDuration <= 0.f )
		{
			ClearRequest();
			return;
		}
	}
	if( m_bPaused )
	{
		return;
	}
	SyncRequest();
	const EDisDesireRequestStatus Status = GetRequestStatus( m_pActorTarget, m_bUsingLocation, m_LocationTarget );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8aa040 (2012 0x8fa430)
void FDisLookAtRequest::OnActorTerminated( const AActor& _rActor )
{
	if( ReferencesActor( &_rActor ) )
	{
		ClearRequest();
	}
}

// DISHONORED(port): 2013 rva 0x8baa40 (2012 0x905b80)
void FDisLookAtRequest::PostGameLoad( FArkComponentLookat* _pLookAtComponent, UObject* _pAsker )
{
	const UBOOL bWasUnpaused = !m_bPaused;
	m_pLookAtComponent = (FPointer)_pLookAtComponent;
	m_pAsker = _pAsker;
	if( !bWasUnpaused )
	{
		return;
	}
	SyncRequest();
	const EDisDesireRequestStatus Status = GetRequestStatus( m_pActorTarget, m_bUsingLocation, m_LocationTarget );
	if( Status != DTDRS_Unchanged )
	{
		DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8a9fe0 (2012 0x8fa3d0)
void FDisLookAtRequest::ClearRequest()
{
	SyncRequest();
	const EDisDesireRequestStatus Status = ClearTarget();
	ClearParams();
	if( Status == DTDRS_StopRequestNeeded )
	{
		DisDesireStructs::ReleaseRequest( DisDesireStructs::DDK_LookAt, TEXT("FArkComponentLookat::StopLookAt") );
		m_RequestID = INDEX_NONE;
	}
}

// DISHONORED(port): 2013 rva 0x8aa060 (2012 0x8fa450), 1,068 bytes: six component entry points. Aim or look, then
// actor / location / nothing, then procedural or not. A look-at always stops the live request first (it has no update
// form), which is why this body is so much larger than the other two.
// DISHONORED(bringup): FArkComponentLookat is not ported; each branch counts the order and names its entry point.
void FDisLookAtRequest::DoRequest( EDisDesireRequestStatus _Status )
{
	if( _Status == DTDRS_Unchanged )
	{
		return;
	}
	if( _Status == DTDRS_StopRequestNeeded )
	{
		DisDesireStructs::ReleaseRequest( DisDesireStructs::DDK_LookAt, TEXT("FArkComponentLookat::StopLookAt") );
		m_RequestID = INDEX_NONE;
		return;
	}

	// DISHONORED(retail): a look-at has no update form - a changed one is always stopped and restarted, which is why this
	// body is so much larger than the face-to's and the loco's.
	if( m_RequestID != INDEX_NONE )
	{
		DisDesireStructs::ReleaseRequest( DisDesireStructs::DDK_LookAt, TEXT("FArkComponentLookat::StopLookAt") );
		m_RequestID = INDEX_NONE;
	}

	const UBOOL bProcedural = m_ProceduralPatternIndex != DisLookAtProceduralPattern_MAX;
	const TCHAR* EntryPoint = NULL;
	if( m_pActorTarget )
	{
		EntryPoint = m_bAimAtTarget ? TEXT("FArkComponentLookat::StartAimAtActor")
			: ( bProcedural ? TEXT("FArkComponentLookat::StartProceduralLookAtOnActor") : TEXT("FArkComponentLookat::StartLookAtActor") );
	}
	else if( m_bUsingLocation )
	{
		EntryPoint = m_bAimAtTarget ? TEXT("FArkComponentLookat::StartAimAtLocation")
			: ( bProcedural ? TEXT("FArkComponentLookat::StartProceduralLookAtOnLocation") : TEXT("FArkComponentLookat::StartLookAtLocation") );
	}
	else if( bProcedural )
	{
		EntryPoint = TEXT("FArkComponentLookat::StartProceduralLookAt");
	}
	else
	{
		return;
	}
	m_RequestID = DisDesireStructs::AcceptRequest( DisDesireStructs::DDK_LookAt, EntryPoint );
}

/*-----------------------------------------------------------------------------
	FDisBodyIntentionRequest - the one request with no Ark component behind it
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8b2440 (2012 0x8ffbd0)
void FDisBodyIntentionRequest::Initialize( ADishonoredNPCPawn* _pNPCPawn, BYTE _Priority )
{
	if( m_pNPCPawn )
	{
		ClearIntention();
		m_bPaused = FALSE;
	}
	m_pNPCPawn = _pNPCPawn;
	m_Priority = _Priority;
}

// DISHONORED(port): the withdraw half, inlined at all six retail call sites.
void FDisBodyIntentionRequest::ClearIntention()
{
	if( m_bDesired && !m_bPaused && m_pNPCPawn )
	{
		m_pNPCPawn->SetBodyIntention( m_Priority, eDisNPCBodyStance_None, NULL, NULL );
	}
	m_bDesired = FALSE;
	m_BodyStance = 0;
	m_pPrimaryItemClass = NULL;
	m_pSecondaryItemClass = NULL;
}

// DISHONORED(port): 2013 rva 0x8a8050 (2012 0x8f9070): an item the NPC does not carry is dropped from the intention and
// the caller is told FALSE - the stance is still taken, with that hand empty.
UBOOL FDisBodyIntentionRequest::RequestBodyIntention( BYTE _BodyStance, UClass* _pPrimaryItemClass, UClass* _pSecondaryItemClass )
{
	UBOOL bAllItemsAvailable = TRUE;
	m_bDesired = TRUE;
	m_BodyStance = _BodyStance;

	if( _pPrimaryItemClass && !DisNPCCarriesItemClass( m_pNPCPawn, _pPrimaryItemClass ) )
	{
		_pPrimaryItemClass = NULL;
		bAllItemsAvailable = FALSE;
	}
	m_pPrimaryItemClass = _pPrimaryItemClass;

	if( _pSecondaryItemClass && !DisNPCCarriesItemClass( m_pNPCPawn, _pSecondaryItemClass ) )
	{
		_pSecondaryItemClass = NULL;
		bAllItemsAvailable = FALSE;
	}
	m_pSecondaryItemClass = _pSecondaryItemClass;

	if( !m_bPaused && m_pNPCPawn )
	{
		GDisDesireBodyIntentions++;
		m_pNPCPawn->SetBodyIntention( m_Priority, m_BodyStance, m_pPrimaryItemClass, m_pSecondaryItemClass );
	}
	return bAllItemsAvailable;
}
