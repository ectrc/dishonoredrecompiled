// DishonoredGame/src/arkcomponentlocomotiongoto.cpp
// DISHONORED(port): agent DN. The public request surface: the five calls a behaviour or a sub-state actually makes, and
// the accessors that answer "where is this NPC being sent". Retail's unit is Engine/Src/arkcomponentlocomotiongoto.cpp
// (31 functions, 3,234 bytes, most of them the FArkRequestManager<FLocoRequestData> instantiation which now lives in
// Engine/Inc/arkrequestmanager.h).
//
// The shape to notice: a request is a *wish with a priority*, not a command. Two askers can have one open each; the queue
// sorts them and only the first is acted on, and the one that is not acted on is neither cancelled nor told anything. That
// is why FDisLocoRequest (Src/disdesirestructs.cpp) can re-state the same wish every thought without the NPC stuttering.

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"
#include "disdesirestructs.h"

// DISHONORED(port): 2013 rva 0x5450c0 (2012 0x585fd0, arkcomponentlocomotiongoto.cpp:96). A location request is the
// request type plus the common props plus the location props, handed to the queue with no duration (-1 = until withdrawn).
INT FArkComponentLocomotion::StartLocoToLocation( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntLocoGoToLocationProp& _GoToProperties )
{
	FLocoRequestData Request;
	Request.m_Type = CPNT_LOCO_REQUEST_TYPE_LOCATION;
	Request.m_CommonProps = _GoToProperties.m_CommonProps;
	Request.m_LocationProps = _GoToProperties.m_LocationProps;
	GDisLocoRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, -1.f );
}

// DISHONORED(port): 2013 rva 0x545010 (2012 0x585f20, arkcomponentlocomotiongoto.cpp:80)
INT FArkComponentLocomotion::StartLocoToActor( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntLocoGoToActorProp& _GoToProperties )
{
	FLocoRequestData Request;
	Request.m_Type = CPNT_LOCO_REQUEST_TYPE_ACTOR;
	Request.m_CommonProps = _GoToProperties.m_CommonProps;
	Request.m_ActorProps = _GoToProperties.m_ActorProps;
	GDisLocoRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, -1.f );
}

// DISHONORED(port): 2013 rva 0x546420 (2012 0x587110, arkcomponentlocomotiongoto.cpp:112): an unknown id answers FALSE
// rather than starting a new request, which is what lets the desire layer tell "my order is still live" from "it is gone".
UBOOL FArkComponentLocomotion::UpdateLocoToLocation( INT _RequestID, const FArkCpntLocoGoToLocationProp& _GoToProperties )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	FLocoRequestData Request;
	Request.m_Type = CPNT_LOCO_REQUEST_TYPE_LOCATION;
	Request.m_CommonProps = _GoToProperties.m_CommonProps;
	Request.m_LocationProps = _GoToProperties.m_LocationProps;
	GDisLocoRequestsUpdated++;
	UpdateLocoRequest( RequestIdx, Request );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x546360 (2012 0x587050, arkcomponentlocomotiongoto.cpp:128)
UBOOL FArkComponentLocomotion::UpdateLocoToActor( INT _RequestID, const FArkCpntLocoGoToActorProp& _GoToProperties )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	FLocoRequestData Request;
	Request.m_Type = CPNT_LOCO_REQUEST_TYPE_ACTOR;
	Request.m_CommonProps = _GoToProperties.m_CommonProps;
	Request.m_ActorProps = _GoToProperties.m_ActorProps;
	GDisLocoRequestsUpdated++;
	UpdateLocoRequest( RequestIdx, Request );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x545160 (2012 0x586070)
void FArkComponentLocomotion::UpdateLocoRequest( INT _RequestIdx, const FLocoRequestData& _rRequest )
{
	m_ReqMgr.UpdateRequestByIdx( _RequestIdx, _rRequest );
}

// DISHONORED(port): 2013 rva 0x546480 (2012 0x587170, arkcomponentlocomotiongoto.cpp:160)
UBOOL FArkComponentLocomotion::StopLoco( INT _RequestID )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	GDisLocoRequestsStopped++;
	m_ReqMgr.RemoveRequestByIdx( RequestIdx );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x5464c0 (2012 0x5871b0)
UBOOL FArkComponentLocomotion::StopAllLocoFromAsker( const UObject* const _pAsker )
{
	const UBOOL bRemoved = m_ReqMgr.RemoveAllRequestsFromAsker( _pAsker );
	if( bRemoved )
	{
		GDisLocoRequestsStopped++;
	}
	return bRemoved;
}

// DISHONORED(port): 2013 rva 0x5464d0 (2012 0x5871c0): with no request the NPC's "move target" is its own location, which
// is what ADishonoredNPCController::GetMoveTargetLocation and UDishonoredAIBrain::GetCurrentDesiredPosition report.
FVector FArkComponentLocomotion::GetActiveRequestTargetedLocation() const
{
	if( m_ReqMgr.HasRequest() )
	{
		return GetAskedTargetLocOfActiveRequest();
	}
	return m_pPawnOwner ? m_pPawnOwner->Location : FVector( 0.f, 0.f, 0.f );
}

// DISHONORED(port): 2013 rva 0x5451e0 (2012 0x5860f0): an actor request resolves to that actor's location every time it
// is asked, which is how a chase follows a moving target without the asker re-issuing anything.
FVector FArkComponentLocomotion::GetAskedTargetLocOfActiveRequest() const
{
	if( !m_ReqMgr.HasRequest() )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	const FLocoRequestData& rData = m_ReqMgr.GetFirstRequest().m_Data;
	if( rData.m_Type == CPNT_LOCO_REQUEST_TYPE_ACTOR )
	{
		return rData.m_ActorProps.m_pActor ? rData.m_ActorProps.m_pActor->Location : FVector( 0.f, 0.f, 0.f );
	}
	return rData.m_LocationProps.m_Location.Get();
}

// DISHONORED(port): 2013 rva 0x545120 (2012 0x586030)
INT FArkComponentLocomotion::GetActiveRequestMaxSpeedIdx() const
{
	if( !m_ReqMgr.HasRequest() )
	{
		return 0;
	}
	return m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_MaxSpeedIdx;
}

// DISHONORED(port): 2013 rva 0x543130 (2012 0x5840d0): "is there a live request whose path is built", which is the single
// flag the dynamics test before they move anything.
UBOOL FArkComponentLocomotion::ComputeHasRequestWithPathComputedFlag() const
{
	if( !m_bBuiltPathUpToDate || !m_ReqMgr.HasRequest() )
	{
		return FALSE;
	}
	return !m_CurReqWorkData.m_bDisabled;
}

// DISHONORED(port): 2013 rva 0x53e620 (2012 0x57f770): eight bytes in retail - set the dirty bit and let the scheduler do
// the work on the next tick.
void FArkComponentLocomotion::ForceActiveRequestRepath()
{
	m_bForcePathComputation = TRUE;
}
