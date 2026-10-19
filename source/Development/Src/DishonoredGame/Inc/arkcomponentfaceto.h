#pragma once
// DishonoredGame/inc/arkcomponentfaceto.h
// DISHONORED(port): agent IC (PHASE23 IC). Arkane's face-to component: the body-yaw half of an NPC's orientation. Askers queue
// "face this yaw / actor / location" requests by priority; each frame PreAsyncWorkTick moves m_CurBodyYaw towards the first
// request's target at its rotation speed and acceleration (ArkSmoothFixAngleRotation, 0.04 s sub-steps), publishes the
// per-frame delta in m_CurBodyYawDelta and the angle between the body and the movement in m_fCurAngle (handed to
// UArkAnimNodeLocomotion as its strafe angle), and tells the asker ORIENTATION_REACHED / ABORTED through its callback.
// Retail's unit is Engine/Src/arkcomponentfaceto.cpp (2012 rvas 0x574820..0x57dcc0, 2013 0x932e90..0x93d4f0, the same sizes);
// the class is declared in Engine/Inc/arkcomponentfaceto.h.
//
// DISHONORED(bringup): declared HERE, in DishonoredGame, for agent DN's and FT's reason (arkcomponentlocomotion.h,
// arkcomponentlookat.h): it reads UArkComponentLocomotionConfig and UArkAnimNodeLocomotion, reflected Engine-package classes the
// generator declares in DishonoredGame. The Engine skeletons Engine/Inc/arkcomponentfaceto.h and Engine/Src/arkcomponentfaceto.cpp
// stay comment-only.

#ifndef _INC_ARKCOMPONENTFACETO
#define _INC_ARKCOMPONENTFACETO

#include "arkcomponentbase.h"
#include "arkrequestmanager.h"
#include "disdesirestructs.h"	// EArkCpntFaceToEvent (agent DF declared it there for the desire layer)

class APawn;
class AActor;
class UObject;
class UArkAnimNodeLocomotion;
class UArkComponentLocomotionConfig;
class FArkGameEvent;

/** The asker's callback: a member of the asking UObject (4 bytes, UObject has single inheritance). */
typedef void ( UObject::*FArkCpntFaceToEventCallback )( EArkCpntFaceToEvent );

/** DISHONORED(layout): 2012 PDB FArkCpntFaceToRequestCommonProps, 24 bytes. The speeds are degrees per second; -100 is the
    config's default, -200 "over the request's remaining time" (acceleration: none), -300 instant, -400 (FindTargetYaw) "keep". */
struct FArkCpntFaceToRequestCommonProps
{
	UObject*					m_pCallbackOwner;
	FArkCpntFaceToEventCallback	m_FaceToEventCallback;
	FLOAT						m_fRotationSpeed;
	FLOAT						m_fAcceleration;
	UBOOL						m_bKeepSnapped;
	UBOOL						m_bExactRotation;
};

/** DISHONORED(layout): 2012 PDB FArkCpntFaceToRequestDataVector, 8 bytes. */
struct FArkCpntFaceToRequestDataVector
{
	FLOAT m_fX;
	FLOAT m_fY;
};

/** DISHONORED(layout): 2012 PDB FArkCpntFaceToProp (28 bytes) and the three request kinds (32 / 36 / 32). */
struct FArkCpntFaceToProp
{
	FArkCpntFaceToRequestCommonProps	m_CommonProps;
	UBOOL								m_bInitialized;

	FArkCpntFaceToProp() : m_bInitialized( FALSE ) {}		// 2013 0x933090
	void OverwriteExactRotation( UBOOL _bExactRotation ) { m_CommonProps.m_bExactRotation = _bExactRotation; }	// 2013 0x94d370
};

struct FArkCpntFaceToActorProp : public FArkCpntFaceToProp
{
	const AActor* m_pActor;

	void InitAllByDefaultExcept( const class FArkComponentFaceTo* const _pFaceToCpnt, const AActor* _pActor, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback );	// 2013 0x9330c0
};

struct FArkCpntFaceToLocationProp : public FArkCpntFaceToProp
{
	FArkCpntFaceToRequestDataVector m_Location;

	void InitAllByDefaultExcept( const FVector& _Location, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback );	// 2013 0x933110
};

struct FArkCpntFaceToYawProp : public FArkCpntFaceToProp
{
	INT m_BodyYaw;

	void InitAllByDefaultExcept( INT _BodyYaw, FLOAT _fRotationSpeed, UObject* const _pCallbackOwner, FArkCpntFaceToEventCallback _EventCallback );	// 2013 0x933160
};

/** DISHONORED(port): 2012 rva 0xafbd0 (435 B, Engine/Src/arkutils.cpp): one step of an accelerated turn from the current to the
    target angle (rotator units), never past it; the speed is carried in _fCurSpeed. */
FLOAT ArkSmoothFixAngleRotation( FLOAT _fCurrentAngle, FLOAT _fTargetAngle, FLOAT _fAcceleration, FLOAT _fMaxSpeed, FLOAT _fTimeStep, FLOAT& _fCurSpeed );

/*-----------------------------------------------------------------------------
	FArkComponentFaceTo - 2012 PDB sizeof 120
-----------------------------------------------------------------------------*/

class FArkComponentFaceTo : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( ArkCpntType_FaceTo, "ArkCpntType_FaceTo" )

public:
	/** DISHONORED(layout): 2012 PDB FArkComponentFaceTo::ECpntFaceToRequestType. */
	enum ECpntFaceToRequestType
	{
		CPNT_FACETO_REQUEST_TYPE_YAW		= 0,
		CPNT_FACETO_REQUEST_TYPE_ACTOR		= 1,
		CPNT_FACETO_REQUEST_TYPE_LOCATION	= 2,
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentFaceTo::FFaceToRequestData, 36 bytes. */
	struct FFaceToRequestData
	{
		ECpntFaceToRequestType				m_Type;			// @0
		FArkCpntFaceToRequestCommonProps	m_CommonProps;	// @4
		union											// @28
		{
			FArkCpntFaceToRequestDataVector	m_Location;
			const AActor*					m_pActor;
			INT								m_BodyYaw;
		};
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentFaceTo::FFaceToRequestWorkData, 8 bytes. */
	struct FFaceToRequestWorkData
	{
		UBOOL m_bSnapped;
		UBOOL m_bOrientationAlreadyReached;
	};

	FArkComponentFaceTo();												// 2013 0x938a90
	virtual ~FArkComponentFaceTo() {}									// 2013 0x938bf0

	// ---- FArkComponentBase ----
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentFaceTo ); }	// 2013 0x932e90
	virtual void Starting();											// 2013 0x93d4f0
	virtual void Stopping();											// 2012 0x57dcc0
	virtual DWORD GetAllocatedSize() const { return m_ReqMgr.GetRequestsCount() * sizeof( FArkRequestManager<FFaceToRequestData>::FRequest ); }	// 2013 0x934280

	// ---- IArkComponentPreAsyncWork ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );					// 2013 0x93a160
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	void Initialize( const UArkComponentLocomotionConfig* const _pConfig );	// 2012 0x575400
	UBOOL IsInitialized() const { return m_bInitialized; }				// 2013 0x932ee0
	static FArkComponentFaceTo* GetFaceToComponent( const APawn* const _pPawnOwner );	// 2013 0x9365b0
	void OnLODChanged( const FArkGameEvent& _rGameEvent );				// 2013 0x9339a0

	// ---- the requests ----
	INT StartFaceToActor( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToActorProp& _rFaceToProperties );			// 2013 0x9365d0
	INT StartFaceToLocation( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToLocationProp& _rFaceToProperties );	// 2013 0x936640
	INT StartFaceToYaw( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntFaceToYawProp& _rFaceToProperties );				// 2013 0x9366b0
	UBOOL UpdateFaceToActor( INT _RequestID, const FArkCpntFaceToActorProp& _rFaceToProperties );		// 2013 0x938c70
	UBOOL UpdateFaceToLocation( INT _RequestID, const FArkCpntFaceToLocationProp& _rFaceToProperties );	// 2013 0x938ce0
	UBOOL UpdateFaceToYaw( INT _RequestID, const FArkCpntFaceToYawProp& _rFaceToProperties );			// 2013 0x938d60
	UBOOL StopFaceTo( INT _RequestID );									// 2013 0x938de0
	UBOOL StopAllFaceToFromAsker( const void* const _pAsker ) { return m_ReqMgr.RemoveAllRequestsFromAsker( _pAsker ); }	// 2013 0x938e20
	INT GetActiveRequestID() const;										// 2012 0x5779e0
	INT GetActiveRequestPriority() const;								// 2013 0x936770
	INT GetRequestsCount() const { return m_ReqMgr.IsInitialized() ? m_ReqMgr.GetRequestsCount() : 0; }	// 2013 0x934290

	// ---- the state the locomotion component and its anim node read ----
	INT GetTargetYaw() const { return m_TargetYaw; }					// 2013 0x4049f0 (folded)
	INT GetCurBodyYaw() const { return m_CurBodyYaw; }
	INT GetCurBodyYawDelta() const { return m_CurBodyYawDelta; }		// 2012 0x5748f0 (folded as FFluidVertexBuffer::GetNumQuadsX)
	INT GetAngle() const { return (INT)m_fCurAngle; }					// 2013 0x932ef0
	UBOOL IsActiveRequestSnapped() const { return m_CurReqWorkData.m_bSnapped; }	// 2013 0x9c5af0 (folded)
	void AllowRootMotion( UBOOL _bDisableAngleUpdate );					// 2013 0x938e30
	void DisableRootMotion();											// 2013 0x932f00
	UBOOL IsRootMotion() const { return m_bRootMotion && !m_bTurnControl; }	// 2013 0x932f10
	void AllowTurnControl() { m_bTurnControl = TRUE; }					// 2013 0x932f30
	void DisableTurnControl();											// 2013 0x938e60
	void ForceBodySnapOnMovement( UBOOL _bForceBodySnapOnMovement ) { m_bForceBodySnapOnMovement = _bForceBodySnapOnMovement ? 1 : 0; }	// 2013 0x932f40
	void SetRealVelocity( const FVector& _rRealVelocity ) { m_RealVelocity = _rRealVelocity; }	// 2013 0x932f60
	void HandleTeleport();												// 2012 0x574900
	static void SetDebugDontRotate( UBOOL _bDontRotate ) { ms_bDebugDontRotate = _bDontRotate; }	// 2013 0x932f80
	static UBOOL GetDebugDontRotate() { return ms_bDebugDontRotate; }

private:
	UBOOL IsRequestHasInfiniteRotationSpeed( const FFaceToRequestData& _rRequest ) const;	// 2013 0x932fd0
	UBOOL IsRequestHasDefaultAcceleration( const FFaceToRequestData& _rRequest ) const;	// 2013 0x933000
	UBOOL CanKeepSnapped( const FFaceToRequestData& _rRequest, FLOAT _fAbsAngle, FLOAT _fTimeStep ) const;	// 2013 0x933030
	UBOOL UpdateCurYawAndRotationSpeed( FLOAT _fTimeStep, const FFaceToRequestData& _rRequest, FLOAT& _fCurYaw, FLOAT& _fRotateSpeed ) const;	// 2013 0x9367b0
	void ResetCurRequestWorkData( UBOOL _bResetSnappedFlag );			// 2013 0x936a40
	INT FindTargetYaw() const;											// 2013 0x936ac0
	void UpdateFaceToRequest( INT _RequestIdx, const FFaceToRequestData& _rRequest );	// 2013 0x936c40
	UBOOL SendFaceToEvent( INT _RequestIdx, EArkCpntFaceToEvent _FaceToEvent );		// 2013 0x936c90
	void HandleOrientationReachedEvent();								// 2013 0x938e80
	void OnRequestManagerEvent( FArkRequestManager<FFaceToRequestData>::EArkReqMgrEvent _Event, INT _RequestIdx );	// 2013 0x939020
	static void RequestManagerEventThunk( FArkComponentBase* _pOwner, const FArkRequestManager<FFaceToRequestData>::EArkReqMgrEvent _Event, const INT _RequestIdx );
	void ReportIfWatched();

public:
	const UArkComponentLocomotionConfig*	m_pConfig;					// @20
	APawn*									m_pPawnOwner;				// @24
	BITFIELD								m_bInitialized:1;			// @28 bit 0
	BITFIELD								m_bNoTick:1;				// bit 1
	BITFIELD								m_bInLOD:1;					// bit 2
	BITFIELD								m_bRootMotion:1;			// bit 3
	BITFIELD								m_bTurnControl:1;			// bit 4
	BITFIELD								m_bDisableAngleUpdate:1;	// bit 5
	BITFIELD								m_bForceBodySnapOnMovement:1;	// bit 6
	FLOAT									m_fRotateSpeed;				// @32
	FLOAT									m_fCurAngle;				// @36
	INT										m_CurBodyYaw;				// @40
	INT										m_CurBodyYawDelta;			// @44
	INT										m_TargetYaw;				// @48
	FVector									m_RealVelocity;				// @52
	FArkRequestManager<FFaceToRequestData>	m_ReqMgr;					// @64
	FFaceToRequestWorkData					m_CurReqWorkData;			// @100
	EArkCpntFaceToEvent						m_DebugLastFaceToEventSent;	// @108
	BITFIELD								m_bDebugLastFaceToEventDisplayed:1;	// @112 bit 0
	BITFIELD								m_bDebugHadInstantRequest:1;		// bit 1
	BITFIELD								m_bDebugHadInstantRequestDisplayed:1;	// bit 2
	UArkAnimNodeLocomotion*					m_pAnimNodeLoco;			// @116

	static UBOOL ms_bDebugDontRotate;
};

#endif
