// DishonoredGame/src/arkcomponentfaceto.cpp
// DISHONORED(port): agent IC (PHASE23 IC). FArkComponentFaceTo, retail's Engine/Src/arkcomponentfaceto.cpp (47 functions); here for
// the reason Inc/arkcomponentfaceto.h gives. Every body was read from the 2012 decompile (build/agentIC/decomp2012/) and diffed
// against the 2013 one (build/agentIC/decomp2013/; every function has the same size in both, the request manager Update and the
// tick ratio .997). The FaceTo instantiation of FArkRequestManager is the shared template of Engine/Inc/arkrequestmanager.h.
//
// DISHONORED(bringup): named deviations -
//   - retail's FArkRequestManager<FFaceToRequestData>::Update (2012 0x57a9b0) keeps a one-frame request alive one extra frame
//     (it clears m_bOneFrameRequest and removes it on the next Update) and decrements m_TimedRequestsCount a second time after a
//     timed removal; the shared template is agent DN's Lookat reading. Every face-to request this tree issues is untimed (-1).
//   - the OnLODChanged registration on FArkGameEventDispatcher (event 2, Starting / Stopping) is made, but FDisComponentLODManager
//     is unported, so m_bNoTick / m_bInLOD keep their constructed values (0 / 1 -> m_bInLOD, as retail before the first LOD event).
//   - -icfaceto gates the whole component: without it the locomotion component does not create one (see SetConfig) and every
//     caller keeps its NULL path. The pawn's yaw is applied by retail's FArkComponentLocomotion::FullMovePawn (2012 0x580ae0 head:
//     NewRotation.Yaw = Normalize( Rotation.Yaw + m_CurBodyYawDelta ), and the move runs when only the yaw changed), which is
//     package IK's function in this wave; the component computes the facing but nothing turns the pawn yet.

#include "DishonoredGame.h"
#include "arkcomponentpolicy.h"
#include "arkcomponentfaceto.h"
#include "arkcomponentlocomotion.h"
#include "icdiag.h"

ARKCOMPONENT_IMPLEMENT_TYPE( FArkComponentFaceTo )

UBOOL FArkComponentFaceTo::ms_bDebugDontRotate = FALSE;

UBOOL DisICFaceToEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ParseParam( appCmdLine(), TEXT("icfaceto") ) ? 1 : 0;
	}
	return Enabled != 0;
}

// The game event FArkComponentFaceTo listens to: the LOD manager's per-pawn LOD change (retail type 2).
enum { ArkGameEventType_FaceToLODChanged = 2 };

// The shortest signed rotator-unit delta from one yaw to another, as retail's inlined FindDeltaAngle on 16-bit yaws writes it.
static INT ArkFaceToYawDelta( INT _From, INT _To )
{
	const INT From = _From & 0xFFFF;
	const INT To = _To & 0xFFFF;
	INT Delta = To - From;
	if( From <= To )
	{
		if( Delta >= 0x8000 )
		{
			Delta -= 0x10000;
		}
	}
	else if( From - To >= 0x8000 )
	{
		Delta += 0x10000;
	}
	return Delta;
}

static INT ArkFaceToNormalizeYaw( INT _Yaw )
{
	return (INT)( (WORD)( _Yaw + 0x7FFF ) ) - 0x7FFF;
}

static INT ArkFaceToVectorYaw( FLOAT _fX, FLOAT _fY )
{
	return appRound( appAtan2( _fY, _fX ) * ( 32768.f / PI ) );
}

// 2012 rva 0xafbd0 (435 B)
FLOAT ArkSmoothFixAngleRotation( FLOAT _fCurrentAngle, FLOAT _fTargetAngle, FLOAT _fAcceleration, FLOAT _fMaxSpeed, FLOAT _fTimeStep, FLOAT& _fCurSpeed )
{
	const INT AngleDif = ArkFaceToYawDelta( (INT)_fCurrentAngle, (INT)_fTargetAngle );
	if( AngleDif == 0 )
	{
		_fCurSpeed = 0.f;
		return (FLOAT)ArkFaceToNormalizeYaw( (INT)_fTargetAngle );
	}
	const FLOAT fAbsAngleDif = (FLOAT)Abs( AngleDif );
	const FLOAT fCurSpeed = _fCurSpeed;
	const FLOAT fAbsSpeed = Abs( fCurSpeed );
	FLOAT fBrakeDistance = 0.f;
	if( -fAbsSpeed < 0.f && fAbsSpeed != 0.f && _fAcceleration != 0.f )
	{
		fBrakeDistance = Abs( ( fAbsSpeed + 0.5f * -fAbsSpeed ) * ( -fAbsSpeed / _fAcceleration ) );
	}
	const UBOOL bMustBrake = ( -fAbsSpeed < 0.f ) && !( fBrakeDistance < fAbsAngleDif );
	const FLOAT fSpeedStep = _fAcceleration * _fTimeStep;
	if( ( fCurSpeed > 0.f && AngleDif < 0 ) || ( fCurSpeed < 0.f && AngleDif > 0 ) )
	{
		_fCurSpeed = -fCurSpeed;
	}
	FLOAT fNewSpeed = _fCurSpeed;
	if( !bMustBrake )
	{
		if( AngleDif >= 0 )
		{
			fNewSpeed = Min( fNewSpeed + fSpeedStep, _fMaxSpeed );
		}
		else
		{
			fNewSpeed = Max( fNewSpeed - fSpeedStep, -_fMaxSpeed );
		}
	}
	else if( fNewSpeed <= 0.f )
	{
		fNewSpeed += fSpeedStep;
		if( fNewSpeed > 0.f )
		{
			fNewSpeed = 0.f;
		}
	}
	else
	{
		fNewSpeed -= fSpeedStep;
		if( fNewSpeed < 0.f )
		{
			fNewSpeed = 0.f;
		}
	}
	_fCurSpeed = fNewSpeed;
	const FLOAT fRotateStep = _fCurSpeed * _fTimeStep;
	if( Abs( fRotateStep ) >= fAbsAngleDif )
	{
		_fCurSpeed = 0.f;
		return (FLOAT)ArkFaceToNormalizeYaw( (INT)_fTargetAngle );
	}
	return (FLOAT)ArkFaceToNormalizeYaw( (INT)( fRotateStep + _fCurrentAngle ) );
}

/*-----------------------------------------------------------------------------
	The request properties
-----------------------------------------------------------------------------*/

// 2012 rvas 0x574a00 / 0x574a50 / 0x574aa0: exact rotation, default acceleration (-100), not kept snapped
void FArkCpntFaceToActorProp::InitAllByDefaultExcept( const FArkComponentFaceTo* const _pFaceToCpnt, const AActor* _pActor, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback )
{
	m_CommonProps.m_pCallbackOwner = _pCallbackOwner;
	m_CommonProps.m_bExactRotation = TRUE;
	m_bInitialized = TRUE;
	m_CommonProps.m_fRotationSpeed = _fRotationSpeed;
	m_CommonProps.m_FaceToEventCallback = _EventCallback;
	m_CommonProps.m_fAcceleration = -100.f;
	m_CommonProps.m_bKeepSnapped = FALSE;
	m_pActor = _pActor;
}

void FArkCpntFaceToLocationProp::InitAllByDefaultExcept( const FVector& _Location, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback )
{
	m_CommonProps.m_pCallbackOwner = _pCallbackOwner;
	m_CommonProps.m_fRotationSpeed = _fRotationSpeed;
	m_CommonProps.m_bExactRotation = TRUE;
	m_bInitialized = TRUE;
	m_CommonProps.m_FaceToEventCallback = _EventCallback;
	m_CommonProps.m_fAcceleration = -100.f;
	m_CommonProps.m_bKeepSnapped = FALSE;
	m_Location.m_fX = _Location.X;
	m_Location.m_fY = _Location.Y;
}

void FArkCpntFaceToYawProp::InitAllByDefaultExcept( INT _BodyYaw, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback )
{
	m_CommonProps.m_pCallbackOwner = _pCallbackOwner;
	m_CommonProps.m_bExactRotation = TRUE;
	m_bInitialized = TRUE;
	m_CommonProps.m_fRotationSpeed = _fRotationSpeed;
	m_CommonProps.m_FaceToEventCallback = _EventCallback;
	m_CommonProps.m_fAcceleration = -100.f;
	m_CommonProps.m_bKeepSnapped = FALSE;
	m_BodyYaw = _BodyYaw;
}

/*-----------------------------------------------------------------------------
	Lifetime
-----------------------------------------------------------------------------*/

// 2012 rva 0x5795d0 / 2013 0x938a90 (170 B): the seven flags cleared, the two debug 'displayed' bits set (the & ~7 | 5 store)
FArkComponentFaceTo::FArkComponentFaceTo()
	: m_pConfig( NULL )
	, m_pPawnOwner( NULL )
	, m_fRotateSpeed( 0.f )
	, m_fCurAngle( 0.f )
	, m_CurBodyYaw( 0 )
	, m_CurBodyYawDelta( 0 )
	, m_TargetYaw( 0 )
	, m_RealVelocity( 0.f, 0.f, 0.f )
	, m_DebugLastFaceToEventSent( CPNT_FACETO_EVENT_NONE )
	, m_pAnimNodeLoco( NULL )
{
	m_bInitialized = 0;
	m_bNoTick = 0;
	m_bInLOD = 0;
	m_bRootMotion = 0;
	m_bTurnControl = 0;
	m_bDisableAngleUpdate = 0;
	m_bForceBodySnapOnMovement = 0;
	m_bDebugLastFaceToEventDisplayed = 1;
	m_bDebugHadInstantRequest = 0;
	m_bDebugHadInstantRequestDisplayed = 1;
	m_CurReqWorkData.m_bSnapped = FALSE;
	m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
}

// 2012 rva 0x575400 (45 B)
void FArkComponentFaceTo::Initialize( const UArkComponentLocomotionConfig* const _pConfig )
{
	m_pPawnOwner = Cast<APawn>( m_pOwner );
	m_pConfig = _pConfig;
	m_bInitialized = TRUE;
	m_pAnimNodeLoco = UArkAnimNodeLocomotion::GetAnimNodeLocomotion( m_pPawnOwner );
}

// 2012 rva 0x577860 / 2013 0x9365b0
FArkComponentFaceTo* FArkComponentFaceTo::GetFaceToComponent( const APawn* const _pPawnOwner )
{
	return _pPawnOwner->m_ComponentContainer ? _pPawnOwner->m_ComponentContainer->GetFirstComponent<FArkComponentFaceTo>( TRUE ) : NULL;
}

void FArkComponentFaceTo::RequestManagerEventThunk( FArkComponentBase* _pOwner, const FArkRequestManager<FFaceToRequestData>::EArkReqMgrEvent _Event, const INT _RequestIdx )
{
	( (FArkComponentFaceTo*)_pOwner )->OnRequestManagerEvent( _Event, _RequestIdx );
}

// 2012 rva 0x57dc00 / 2013 0x93d4f0 (186 / 198 B)
void FArkComponentFaceTo::Starting()
{
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Register( this );
	}
	m_ReqMgr.Initialize( this, &FArkComponentFaceTo::RequestManagerEventThunk );
	m_fRotateSpeed = 0.f;
	m_fCurAngle = 0.f;
	m_CurBodyYaw = ArkFaceToNormalizeYaw( m_pPawnOwner->Rotation.Yaw );
	m_CurBodyYawDelta = 0;
	m_TargetYaw = m_CurBodyYaw;
	m_CurReqWorkData.m_bSnapped = FALSE;
	m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
	if( m_pAnimNodeLoco && !m_bDisableAngleUpdate )
	{
		m_pAnimNodeLoco->SetAngle( 0 );
	}
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		const INT EventType = ArkGameEventType_FaceToLODChanged;
		Dispatcher->RegisterToObjectEvent<FArkComponentFaceTo>( EventType, m_pPawnOwner, this, &FArkComponentFaceTo::OnLODChanged );
	}
}

// 2012 rva 0x57dcc0 (211 B)
void FArkComponentFaceTo::Stopping()
{
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Unregister( this );
	}
	m_ReqMgr.Reset();
	m_fRotateSpeed = 0.f;
	m_fCurAngle = 0.f;
	m_CurBodyYaw = ArkFaceToNormalizeYaw( m_pPawnOwner->Rotation.Yaw );
	m_CurBodyYawDelta = 0;
	m_TargetYaw = m_CurBodyYaw;
	m_CurReqWorkData.m_bSnapped = FALSE;
	m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		const INT EventType = ArkGameEventType_FaceToLODChanged;
		Dispatcher->UnregisterToObjectEvent<FArkComponentFaceTo>( EventType, m_pPawnOwner, this, &FArkComponentFaceTo::OnLODChanged );
	}
}

// 2012 rva 0x575430 / 2013 0x9339a0 (49 B): the LOD flags word - bit 16 clear = no tick, bit 6 clear = in LOD (the bodies mask
// ~(Flags >> 15) with 2 and ~(Flags >> 4) with 4)
void FArkComponentFaceTo::OnLODChanged( const FArkGameEvent& _rGameEvent )
{
	const DWORD LODFlags = *(const DWORD*)_rGameEvent.m_pEventParams;
	m_bNoTick = ( ( LODFlags >> 16 ) & 1 ) ? 0 : 1;
	m_bInLOD = ( ( LODFlags >> 6 ) & 1 ) ? 0 : 1;
}

/*-----------------------------------------------------------------------------
	The requests
-----------------------------------------------------------------------------*/

static void ArkFaceToCopyCommonProps( FArkComponentFaceTo::FFaceToRequestData& _rRequest, const FArkCpntFaceToProp& _rProps )
{
	_rRequest.m_CommonProps = _rProps.m_CommonProps;
}

// 2013 0x9365d0
INT FArkComponentFaceTo::StartFaceToActor( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToActorProp& _rFaceToProperties )
{
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_pActor = _rFaceToProperties.m_pActor;
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_ACTOR;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, -1.f );
}

// 2013 0x936640
INT FArkComponentFaceTo::StartFaceToLocation( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToLocationProp& _rFaceToProperties )
{
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_Location = _rFaceToProperties.m_Location;
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_LOCATION;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, -1.f );
}

// 2013 0x9366b0
INT FArkComponentFaceTo::StartFaceToYaw( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToYawProp& _rFaceToProperties )
{
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_BodyYaw = ArkFaceToNormalizeYaw( _rFaceToProperties.m_BodyYaw );
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_YAW;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, -1.f );
}

// 2013 0x938c70
UBOOL FArkComponentFaceTo::UpdateFaceToActor( INT _RequestID, const FArkCpntFaceToActorProp& _rFaceToProperties )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_pActor = _rFaceToProperties.m_pActor;
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_ACTOR;
	UpdateFaceToRequest( RequestIdx, Request );
	return TRUE;
}

// 2013 0x938ce0
UBOOL FArkComponentFaceTo::UpdateFaceToLocation( INT _RequestID, const FArkCpntFaceToLocationProp& _rFaceToProperties )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_Location = _rFaceToProperties.m_Location;
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_LOCATION;
	UpdateFaceToRequest( RequestIdx, Request );
	return TRUE;
}

// 2013 0x938d60
UBOOL FArkComponentFaceTo::UpdateFaceToYaw( INT _RequestID, const FArkCpntFaceToYawProp& _rFaceToProperties )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	FFaceToRequestData Request;
	ArkFaceToCopyCommonProps( Request, _rFaceToProperties );
	Request.m_BodyYaw = ArkFaceToNormalizeYaw( _rFaceToProperties.m_BodyYaw );
	Request.m_Type = CPNT_FACETO_REQUEST_TYPE_YAW;
	UpdateFaceToRequest( RequestIdx, Request );
	return TRUE;
}

// 2013 0x938de0
UBOOL FArkComponentFaceTo::StopFaceTo( INT _RequestID )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	m_ReqMgr.RemoveRequestByIdx( RequestIdx );
	return TRUE;
}

// 2012 0x5779e0
INT FArkComponentFaceTo::GetActiveRequestID() const
{
	return m_ReqMgr.HasRequest() ? m_ReqMgr.GetFirstRequest().m_ID : INDEX_NONE;
}

// 2013 0x936770
INT FArkComponentFaceTo::GetActiveRequestPriority() const
{
	return m_ReqMgr.HasRequest() ? m_ReqMgr.GetFirstRequest().m_Priority : INDEX_NONE;
}

// 2013 0x936c40
void FArkComponentFaceTo::UpdateFaceToRequest( INT _RequestIdx, const FFaceToRequestData& _rRequest )
{
	m_ReqMgr.UpdateRequestByIdx( _RequestIdx, _rRequest );
}

/*-----------------------------------------------------------------------------
	The flags the locomotion component sets
-----------------------------------------------------------------------------*/

// 2013 0x938e30
void FArkComponentFaceTo::AllowRootMotion( UBOOL _bDisableAngleUpdate )
{
	m_bDisableAngleUpdate = _bDisableAngleUpdate ? 1 : 0;
	m_bRootMotion = 1;
	if( IsRootMotion() )
	{
		m_CurReqWorkData.m_bSnapped = FALSE;
		m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
	}
}

// 2013 0x932f00: root motion, turn control and the angle-update bar all cleared together
void FArkComponentFaceTo::DisableRootMotion()
{
	if( m_bRootMotion )
	{
		m_bRootMotion = 0;
		m_bTurnControl = 0;
		m_bDisableAngleUpdate = 0;
	}
}

// 2013 0x938e60
void FArkComponentFaceTo::DisableTurnControl()
{
	m_bTurnControl = 0;
	if( IsRootMotion() )
	{
		m_CurReqWorkData.m_bSnapped = FALSE;
		m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
	}
}

// 2012 0x574900 (47 B): a pawn teleported (actor flag word +296 bit 0x1000, read here as bJustTeleported - not verified against the
// 2013 actor layout; nothing in this tree calls it yet) takes its new yaw as the body yaw
void FArkComponentFaceTo::HandleTeleport()
{
	if( !m_pPawnOwner->bJustTeleported )
	{
		return;
	}
	const INT Yaw = m_pPawnOwner->Rotation.Yaw;
	if( m_CurBodyYaw != Yaw )
	{
		if( m_TargetYaw == m_CurBodyYaw )
		{
			m_TargetYaw = Yaw;
		}
		m_CurBodyYawDelta = 0;
		m_CurBodyYaw = Yaw;
	}
}

/*-----------------------------------------------------------------------------
	The private update chain
-----------------------------------------------------------------------------*/

// 2013 0x932fd0 / 0x933000
UBOOL FArkComponentFaceTo::IsRequestHasInfiniteRotationSpeed( const FFaceToRequestData& _rRequest ) const
{
	return Abs( _rRequest.m_CommonProps.m_fRotationSpeed - -300.f ) < 1.f;
}

UBOOL FArkComponentFaceTo::IsRequestHasDefaultAcceleration( const FFaceToRequestData& _rRequest ) const
{
	return Abs( _rRequest.m_CommonProps.m_fAcceleration - -100.f ) < 1.f;
}

// 2013 0x933030: a snapped request stays snapped while one frame at its speed covers the angle
UBOOL FArkComponentFaceTo::CanKeepSnapped( const FFaceToRequestData& _rRequest, FLOAT _fAbsAngle, FLOAT _fTimeStep ) const
{
	if( !_rRequest.m_CommonProps.m_bKeepSnapped || !m_CurReqWorkData.m_bSnapped )
	{
		return FALSE;
	}
	FLOAT fSpeed = m_pConfig->m_fFaceToDefaultSpeed;
	if( Abs( _rRequest.m_CommonProps.m_fRotationSpeed - -100.f ) >= 1.f )
	{
		fSpeed = _rRequest.m_CommonProps.m_fRotationSpeed;
	}
	return fSpeed * _fTimeStep >= _fAbsAngle;
}

// 2013 0x9367b0 (651 B): snapped or instant -> the target; -200 -> the remaining yaw over the request's remaining time; otherwise
// ArkSmoothFixAngleRotation in 0.04 s sub-steps at the request's (or the config's) speed and acceleration. Answers whether the
// body is (now) snapped on the target.
UBOOL FArkComponentFaceTo::UpdateCurYawAndRotationSpeed( FLOAT _fTimeStep, const FFaceToRequestData& _rRequest, FLOAT& _fCurYaw, FLOAT& _fRotateSpeed ) const
{
	const UBOOL bSnapped = m_CurReqWorkData.m_bSnapped;
	const FLOAT fRotationSpeed = _rRequest.m_CommonProps.m_fRotationSpeed;
	if( bSnapped || Abs( fRotationSpeed - -300.f ) < 1.f )
	{
		_fRotateSpeed = 0.f;
		_fCurYaw = (FLOAT)m_TargetYaw;
		return bSnapped;
	}
	if( Abs( fRotationSpeed - -200.f ) < 1.f )
	{
		const FLOAT fTimeLeft = m_ReqMgr.GetFirstRequest().m_fTimeLeft;
		if( Abs( fTimeLeft ) >= 0.01f )
		{
			const INT Delta = ArkFaceToYawDelta( m_CurBodyYaw, m_TargetYaw );
			_fRotateSpeed = (FLOAT)Delta / ( fTimeLeft + _fTimeStep );
			_fCurYaw = _fRotateSpeed * _fTimeStep + _fCurYaw;
		}
		else
		{
			_fRotateSpeed = 0.f;
			_fCurYaw = (FLOAT)m_TargetYaw;
		}
	}
	else
	{
		FLOAT fSpeed = m_pConfig->m_fFaceToDefaultSpeed;
		if( Abs( fRotationSpeed - -100.f ) >= 1.f )
		{
			fSpeed = fRotationSpeed;
		}
		const FLOAT fAcceleration = _rRequest.m_CommonProps.m_fAcceleration;
		const FLOAT fFixMaxRotSpeed = fSpeed * 182.04445f;
		FLOAT fFixRotAcceleration = fFixMaxRotSpeed;
		if( Abs( fAcceleration - -200.f ) < 1.f )
		{
			_fRotateSpeed = fFixMaxRotSpeed;
			fFixRotAcceleration = 0.f;
		}
		else if( !IsRequestHasDefaultAcceleration( _rRequest ) )
		{
			fFixRotAcceleration = fAcceleration * 182.04445f;
		}
		FLOAT fTimeLeft = _fTimeStep;
		do
		{
			const FLOAT fStep = Min( 0.04f, fTimeLeft );
			_fCurYaw = ArkSmoothFixAngleRotation( _fCurYaw, (FLOAT)m_TargetYaw, fFixRotAcceleration, fFixMaxRotSpeed, fStep, _fRotateSpeed );
			fTimeLeft -= 0.04f;
		}
		while( fTimeLeft > 0.f );
	}
	if( Abs( (FLOAT)m_TargetYaw - _fCurYaw ) >= 0.1f || Abs( fRotationSpeed - -200.f ) < 1.f || IsRequestHasInfiniteRotationSpeed( _rRequest ) )
	{
		return FALSE;
	}
	return TRUE;
}

// 2013 0x936a40 (116 B): a kept-snapped first request stays snapped across an update; everything else starts over
void FArkComponentFaceTo::ResetCurRequestWorkData( UBOOL _bResetSnappedFlag )
{
	const UBOOL bKeepSnapped = !_bResetSnappedFlag && m_CurReqWorkData.m_bSnapped && m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_bKeepSnapped;
	m_CurReqWorkData.m_bSnapped = bKeepSnapped;
	m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
}

// 2012 0x577d70 / 2013 0x936ac0 (382 B): the first request's yaw, or the yaw towards its actor (a controller's pawn) or location;
// the body yaw while that point is the pawn's own; the pawn's yaw with no request or a "keep" (-400) request
INT FArkComponentFaceTo::FindTargetYaw() const
{
	const INT PawnYaw = ArkFaceToNormalizeYaw( m_pPawnOwner->Rotation.Yaw );
	if( !m_ReqMgr.HasRequest() )
	{
		return PawnYaw;
	}
	const FFaceToRequestData& Request = m_ReqMgr.GetFirstRequest().m_Data;
	if( Abs( Request.m_CommonProps.m_fRotationSpeed - -400.f ) < 1.f )
	{
		return PawnYaw;
	}
	FVector Target;
	switch( Request.m_Type )
	{
	case CPNT_FACETO_REQUEST_TYPE_YAW:
		return Request.m_BodyYaw;
	case CPNT_FACETO_REQUEST_TYPE_ACTOR:
		{
			const AActor* Actor = Request.m_pActor;
			const AController* Controller = ConstCast<AController>( Actor );
			if( Controller )
			{
				Actor = Controller->Pawn;
			}
			Target = FVector( Actor->Location.X, Actor->Location.Y, 0.f );
		}
		break;
	case CPNT_FACETO_REQUEST_TYPE_LOCATION:
		Target = FVector( Request.m_Location.m_fX, Request.m_Location.m_fY, 0.f );
		break;
	default:
		return PawnYaw;
	}
	Target.Z = m_pPawnOwner->Location.Z;
	if( Target.Equals( m_pPawnOwner->Location, 0.1f ) )
	{
		return m_CurBodyYaw;
	}
	return ArkFaceToVectorYaw( Target.X - m_pPawnOwner->Location.X, Target.Y - m_pPawnOwner->Location.Y );
}

// 2013 0x936c90 (128 B): ORIENTATION_REACHED once per request, ABORTED always
UBOOL FArkComponentFaceTo::SendFaceToEvent( INT _RequestIdx, EArkCpntFaceToEvent _FaceToEvent )
{
	const FFaceToRequestData& Request = m_ReqMgr.GetRequest( _RequestIdx ).m_Data;
	if( !Request.m_CommonProps.m_FaceToEventCallback )
	{
		return TRUE;
	}
	if( _FaceToEvent == CPNT_FACETO_EVENT_ORIENTATION_REACHED )
	{
		const UBOOL bFirst = !m_CurReqWorkData.m_bOrientationAlreadyReached;
		m_CurReqWorkData.m_bOrientationAlreadyReached = TRUE;
		if( !bFirst )
		{
			return FALSE;
		}
	}
	( Request.m_CommonProps.m_pCallbackOwner->*Request.m_CommonProps.m_FaceToEventCallback )( _FaceToEvent );
	m_bDebugLastFaceToEventDisplayed = 0;
	m_DebugLastFaceToEventSent = _FaceToEvent;
	return TRUE;
}

// 2013 0x938e80 (401 B): reached exactly; or, for a not-exact request on a pawn that is not moving, within the config's
// m_fFaceToNotExactRotThreshold degrees
void FArkComponentFaceTo::HandleOrientationReachedEvent()
{
	UBOOL bReached = FALSE;
	if( m_CurBodyYaw == m_TargetYaw )
	{
		bReached = TRUE;
	}
	else if( m_RealVelocity.X == 0.f && m_RealVelocity.Y == 0.f && m_RealVelocity.Z == 0.f && !m_ReqMgr.GetFirstRequest().m_Data.m_CommonProps.m_bExactRotation )
	{
		const INT AbsDelta = Abs( ArkFaceToYawDelta( m_CurBodyYaw, m_TargetYaw ) );
		bReached = m_pConfig->m_fFaceToNotExactRotThreshold >= (FLOAT)AbsDelta * ( 360.f / 65536.f );
	}
	if( !bReached )
	{
		m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
		return;
	}
	SendFaceToEvent( 0, CPNT_FACETO_EVENT_ORIENTATION_REACHED );
}

// 2013 0x939020 (287 B): an update of the first request restarts its work data; a removal aborts the asker; a new first request
// with infinite speed turns the pawn at once (MoveActor with the new yaw), any other loses the snap
void FArkComponentFaceTo::OnRequestManagerEvent( FArkRequestManager<FFaceToRequestData>::EArkReqMgrEvent _Event, INT _RequestIdx )
{
	if( DisICDiagEnabled() && ( _Event == FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_REQUEST_ADDED || _Event == FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_REQUEST_WILL_BE_REMOVED ) )
	{
		const FArkRequestManager<FFaceToRequestData>::FRequest& Request = m_ReqMgr.GetRequest( _RequestIdx );
		debugf( TEXT("DISHONORED(bringup): icdiag faceto %s %s req %d prio %d type %d asker %s speed %.1f accel %.1f pawn yaw %d world %.2f"),
			*m_pPawnOwner->GetName(), _Event == FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_REQUEST_ADDED ? TEXT("start") : TEXT("stop"),
			Request.m_ID, Request.m_Priority, (INT)Request.m_Data.m_Type, *Request.m_AskerName.ToString(), Request.m_Data.m_CommonProps.m_fRotationSpeed,
			Request.m_Data.m_CommonProps.m_fAcceleration, m_pPawnOwner->Rotation.Yaw & 0xFFFF, GWorld ? GWorld->GetTimeSeconds() : 0.f );
	}
	switch( _Event )
	{
	case FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_REQUEST_UPDATED:
		if( _RequestIdx == 0 )
		{
			ResetCurRequestWorkData( FALSE );
		}
		break;
	case FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_REQUEST_WILL_BE_REMOVED:
		SendFaceToEvent( _RequestIdx, CPNT_FACETO_EVENT_ABORTED );
		break;
	case FArkRequestManager<FFaceToRequestData>::ARK_REQMGR_FIRST_REQUEST_HAS_CHANGED:
		if( m_ReqMgr.HasRequest() )
		{
			if( IsRequestHasInfiniteRotationSpeed( m_ReqMgr.GetFirstRequest().m_Data ) )
			{
				FRotator BodyRotation = m_pPawnOwner->Rotation;
				m_TargetYaw = FindTargetYaw();
				BodyRotation.Yaw = m_TargetYaw;
				m_fRotateSpeed = 0.f;
				FCheckResult Hit( 1.f );
				GWorld->MoveActor( m_pPawnOwner, FVector( 0.f, 0.f, 0.f ), BodyRotation, 0, Hit );
				m_bDebugHadInstantRequest = 1;
				m_bDebugHadInstantRequestDisplayed = 0;
			}
		}
		else
		{
			m_CurReqWorkData.m_bSnapped = FALSE;
			m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
		}
		break;
	default:
		break;
	}
}

// 2012 0x57ab20 / 2013 0x93a160 (1,151 B both)
void FArkComponentFaceTo::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	if( m_bNoTick || _fTimeStep <= 0.0001f )
	{
		return;
	}
	if( m_ReqMgr.Update( _fTimeStep ) )
	{
		m_CurReqWorkData.m_bSnapped = FALSE;
		m_CurReqWorkData.m_bOrientationAlreadyReached = FALSE;
	}
	const INT PawnYaw = ArkFaceToNormalizeYaw( m_pPawnOwner->Rotation.Yaw );
	m_CurBodyYaw = PawnYaw;
	m_CurBodyYawDelta = 0;
	m_TargetYaw = FindTargetYaw();

	UBOOL bRotated = FALSE;
	if( m_bInLOD )
	{
		m_CurBodyYawDelta = ArkFaceToYawDelta( m_CurBodyYaw, m_TargetYaw );
		m_CurBodyYaw = m_TargetYaw;
	}
	else
	{
		if( ( !m_bRootMotion || m_bTurnControl ) && m_ReqMgr.HasRequest() && !m_bForceBodySnapOnMovement && !ms_bDebugDontRotate )
		{
			const FFaceToRequestData& Request = m_ReqMgr.GetFirstRequest().m_Data;
			FLOAT fCurYaw = (FLOAT)m_CurBodyYaw;
			const FLOAT fAbsAngle = Abs( (FLOAT)ArkFaceToYawDelta( m_CurBodyYaw, m_TargetYaw ) * ( 360.f / 65536.f ) );
			if( m_bRootMotion || Request.m_CommonProps.m_bExactRotation || fAbsAngle >= m_pConfig->m_fFaceToNotExactRotThreshold || !m_RealVelocity.IsNearlyZero( 0.0001f ) )
			{
				m_CurReqWorkData.m_bSnapped = CanKeepSnapped( Request, fAbsAngle, _fTimeStep );
				m_CurReqWorkData.m_bSnapped = UpdateCurYawAndRotationSpeed( _fTimeStep, Request, fCurYaw, m_fRotateSpeed );
			}
			m_CurBodyYawDelta = ArkFaceToYawDelta( m_CurBodyYaw, (INT)fCurYaw );
			m_CurBodyYaw = ArkFaceToNormalizeYaw( m_CurBodyYaw + m_CurBodyYawDelta );
			bRotated = TRUE;
		}
		else if( m_bRootMotion )
		{
			USkeletalMeshComponent* Mesh = m_pPawnOwner->Mesh;
			// retail reads the mesh's byte +900 (RootMotionMode) and the quaternion +816 (RootMotionDelta's rotation)
			if( Mesh && Mesh->RootMotionMode != RMM_Translate )
			{
				const FRotator RootRotation = FQuatRotationTranslationMatrix( Mesh->RootMotionDelta.GetRotation(), FVector( 0.f, 0.f, 0.f ) ).Rotator();
				m_CurBodyYawDelta = RootRotation.Yaw;
				m_CurBodyYaw = ArkFaceToNormalizeYaw( m_CurBodyYaw + RootRotation.Yaw );
			}
		}
	}
	if( !bRotated )
	{
		m_fRotateSpeed = 0.f;
	}
	if( m_ReqMgr.HasRequest() )
	{
		HandleOrientationReachedEvent();
	}

	INT MoveYaw = m_CurBodyYaw;
	if( m_RealVelocity.IsNearlyZero( 0.0001f ) )
	{
		const FVector& Velocity = m_pPawnOwner->Velocity;
		if( !Velocity.IsNearlyZero( 0.0001f ) )
		{
			MoveYaw = ArkFaceToVectorYaw( Velocity.X, Velocity.Y );
		}
	}
	else
	{
		MoveYaw = ArkFaceToVectorYaw( m_RealVelocity.X, m_RealVelocity.Y );
	}
	if( m_bForceBodySnapOnMovement )
	{
		m_CurBodyYawDelta = ArkFaceToYawDelta( PawnYaw, MoveYaw );
		m_CurBodyYaw = MoveYaw;
	}
	m_fCurAngle = (FLOAT)ArkFaceToYawDelta( m_CurBodyYaw, MoveYaw );
	if( m_pAnimNodeLoco && !m_bDisableAngleUpdate )
	{
		m_pAnimNodeLoco->SetAngle( (INT)m_fCurAngle );
	}
	if( m_bDebugLastFaceToEventDisplayed )
	{
		m_DebugLastFaceToEventSent = CPNT_FACETO_EVENT_NONE;
	}
	else
	{
		m_bDebugLastFaceToEventDisplayed = 1;
	}
	if( m_bDebugHadInstantRequestDisplayed )
	{
		m_bDebugHadInstantRequest = 0;
	}
	else
	{
		m_bDebugHadInstantRequestDisplayed = 1;
	}
	ReportIfWatched();
}

// -icdiag: once a second per pawn with a request, the first request and the yaw it has reached
void FArkComponentFaceTo::ReportIfWatched()
{
	if( !DisICDiagEnabled() || !m_ReqMgr.HasRequest() || !GWorld )
	{
		return;
	}
	static TMap<FArkComponentFaceTo*, FLOAT> s_NextReport;
	const FLOAT Now = GWorld->GetTimeSeconds();
	FLOAT& Next = s_NextReport.FindOrAdd( this );
	if( Now < Next )
	{
		return;
	}
	Next = Now + 1.f;
	const FArkRequestManager<FFaceToRequestData>::FRequest& First = m_ReqMgr.GetFirstRequest();
	debugf( TEXT("DISHONORED(bringup): icdiag faceto %s req %d prio %d type %d asker %s speed %.1f | pawn yaw %d body %d target %d delta %d rotspeed %.0f snapped %d reached %d requests %d world %.2f"),
		*m_pPawnOwner->GetName(), First.m_ID, First.m_Priority, (INT)First.m_Data.m_Type, *First.m_AskerName.ToString(),
		First.m_Data.m_CommonProps.m_fRotationSpeed, m_pPawnOwner->Rotation.Yaw & 0xFFFF, m_CurBodyYaw & 0xFFFF, m_TargetYaw & 0xFFFF,
		m_CurBodyYawDelta, m_fRotateSpeed, m_CurReqWorkData.m_bSnapped ? 1 : 0, m_CurReqWorkData.m_bOrientationAlreadyReached ? 1 : 0,
		m_ReqMgr.GetRequestsCount(), Now );
}
