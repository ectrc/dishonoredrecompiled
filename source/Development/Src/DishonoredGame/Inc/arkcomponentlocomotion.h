#pragma once
// DishonoredGame/inc/arkcomponentlocomotion.h
// DISHONORED(port): agent DN. Arkane's move executor: the thing that turns an AI's "go there" into a nav-mesh path and
// then into the pawn's translation every frame. Retail's 130 functions live in six Engine units
// (arkcomponentlocomotion.cpp, ...goto.cpp, ...path.cpp, ...dynamic.cpp, ...rootmove.cpp, ...follow.cpp) plus
// arkpathbuildutils.cpp, and the class is declared in Engine/Inc/arkcomponentlocomotion.h.
//
// DISHONORED(bringup): it is declared HERE, in DishonoredGame, and not in Engine, for exactly one reason: the component
// cannot be compiled without UArkComponentLocomotionConfig, and that reflected Engine-package class is declared in
// DishonoredGame/Inc/DishonoredGameEngineShims.h (one of 113 Engine-package classes the generator puts in this module
// because the Engine headers lack them) and IMPLEMENT_CLASS'd in DishonoredGame/Src/DishonoredGameRegistrants.cpp.
// Moving the component to its retail home therefore means declaring UArkComponentLocomotionConfig (and
// UArkAvoidable, UArkAnimNodeLocomotion) in Engine/Inc/EngineArkaneClasses.h instead, at which point the generator's own
// rule (`package_2013 in SHIM_PACKAGES and cpp not in d.declared`, gen_classes_header.py sdk_select) drops them from the
// shim header by itself. That is a generator/coordinator change and is named in resources/docs/agents/agentDN.md rather
// than made here, because two agents regenerating DishonoredGame is how HEAD has been broken before.
//
// Everything the AI asks for arrives through FDisLocoRequest (Src/disdesirestructs.cpp) and leaves through the pawn's
// translation in MovePawn. The per-frame order is retail's:
//   FArkComponentManager's PreAsyncWork policy -> PreAsyncWorkTick(dt): path scheduling, then the dynamics
//   ADishonoredNPCPawn::physWalking(dt)        -> MovePawn(dt):        the translation
// DISHONORED(bringup): FArkComponentManager (2012 PDB, 180 bytes of six FArkComponentPolicy instantiations;
// UWorld::m_pComponentManager is a forward declaration in the tree and no instance is ever created) is not ported, so
// PreAsyncWorkTick is driven from the head of ADishonoredNPCPawn::physWalking instead. That keeps retail's ordering
// within the frame - dynamics before the move - and is the only tick-source difference.

#ifndef _INC_ARKCOMPONENTLOCOMOTION
#define _INC_ARKCOMPONENTLOCOMOTION

#include "arkcomponentbase.h"
#include "arkrequestmanager.h"

class ADishonoredNPCPawn;
class FArkComponentFaceTo;
class FArkComponentLookat;
class FArkComponentMeshOffset;
class UArkAnimNodeLocomotion;
class UArkAvoidable;
class UArkComponentLocomotionConfig;
struct FNavMeshEdgeBase;

/*-----------------------------------------------------------------------------
	The plain (unreflected) structs of the locomotion request interface.
	Layouts and member names are the 2012 PDB's; every size below is asserted in Src/arkcomponentlocomotion.cpp.
-----------------------------------------------------------------------------*/

/** DISHONORED(layout): 2012 PDB FArkCpntLocoRequestDataVector, 12 bytes. Retail keeps a request's location as three
    floats rather than an FVector so the request block stays a POD the request manager can memcpy. */
struct FArkCpntLocoRequestDataVector
{
	FLOAT m_fX;
	FLOAT m_fY;
	FLOAT m_fZ;

	FArkCpntLocoRequestDataVector() : m_fX( 0.f ), m_fY( 0.f ), m_fZ( 0.f ) {}
	void Set( const FVector& _V ) { m_fX = _V.X; m_fY = _V.Y; m_fZ = _V.Z; }
	FVector Get() const { return FVector( m_fX, m_fY, m_fZ ); }
};

/** The callback an asker gets when its order finishes, fails or is dropped. Retail's typedef; the event is
    EArkCpntLocoEvent (Inc/disdesirestructs.h). */
typedef void (*ArkCpntLocoEventCallbackType)( class UObject* _pCallbackOwner, const INT _RequestID, const BYTE _Event );

/** DISHONORED(layout): 2012 PDB FArkCpntLocoRequestCommonProps, 40 bytes. */
struct FArkCpntLocoRequestCommonProps
{
	class UObject*					m_pCallbackOwner;			// @0
	ArkCpntLocoEventCallbackType	m_LocoEventCallback;		// @4
	INT								m_MaxSpeedIdx;				// @8
	INT								m_EndSpeedIdx;				// @12
	FLOAT							m_fEndLocationThreshold;	// @16
	FLOAT							m_fMaxFunnelRadiusMultiplier;// @20
	FLOAT							m_fSpeedMultiplier;			// @24
	FLOAT							m_fStopBeforeEndDist;		// @28
	UBOOL							m_bAccurateStop;			// @32
	UBOOL							m_bSpeedIsLookAtDependent;	// @36

	FArkCpntLocoRequestCommonProps()
		: m_pCallbackOwner( NULL )
		, m_LocoEventCallback( NULL )
		, m_MaxSpeedIdx( 0 )
		, m_EndSpeedIdx( 0 )
		, m_fEndLocationThreshold( -1.f )
		, m_fMaxFunnelRadiusMultiplier( 1.f )
		, m_fSpeedMultiplier( 1.f )
		, m_fStopBeforeEndDist( 0.f )
		, m_bAccurateStop( TRUE )
		, m_bSpeedIsLookAtDependent( FALSE )
	{}
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoRequestLocationProps, 28 bytes. */
struct FArkCpntLocoRequestLocationProps
{
	FArkCpntLocoRequestDataVector	m_Location;					// @0
	FArkCpntLocoRequestDataVector	m_Orientation;				// @12
	FLOAT							m_fEndOrientationThreshold;	// @24

	FArkCpntLocoRequestLocationProps() : m_fEndOrientationThreshold( -1.f ) {}
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoRequestActorProps, 16 bytes. */
struct FArkCpntLocoRequestActorProps
{
	const AActor*	m_pActor;		// @0
	FLOAT			m_fFollowAngle;	// @4
	FLOAT			m_fFollowDist;	// @8
	UBOOL			m_bFollow;		// @12

	FArkCpntLocoRequestActorProps() : m_pActor( NULL ), m_fFollowAngle( 0.f ), m_fFollowDist( 0.f ), m_bFollow( FALSE ) {}
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoGoToProp, 48 bytes: the base of both public request kinds. */
struct FArkCpntLocoGoToProp
{
	FArkCpntLocoRequestCommonProps		m_CommonProps;	// @0
	const class FArkComponentLocomotion*	m_pLocoCpnt;	// @40
	UBOOL								m_bInitialized;	// @44

	FArkCpntLocoGoToProp() : m_pLocoCpnt( NULL ), m_bInitialized( FALSE ) {}
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoGoToLocationProp, 76 bytes. */
struct FArkCpntLocoGoToLocationProp : public FArkCpntLocoGoToProp
{
	FArkCpntLocoRequestLocationProps m_LocationProps;	// @48
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoGoToActorProp, 64 bytes. */
struct FArkCpntLocoGoToActorProp : public FArkCpntLocoGoToProp
{
	FArkCpntLocoRequestActorProps m_ActorProps;			// @48
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoPathPoint, 16 bytes: one corner of the string-pulled path, with the index of
    the pathfinding edge it came off (-1 for the two endpoints). */
struct FArkCpntLocoPathPoint
{
	FVector	m_Point;		// @0
	INT		m_EdgeIndex;	// @12

	FArkCpntLocoPathPoint() : m_Point( 0.f, 0.f, 0.f ), m_EdgeIndex( INDEX_NONE ) {}
};

/** DISHONORED(layout): 2012 PDB FArkCpntLocoPathFindingEdge, 32 bytes: a nav-mesh edge the path crosses, already pulled
    in from both walls by the pawn's radius. The "adjusted distance" is how far the corner was moved along the edge. */
struct FArkCpntLocoPathFindingEdge
{
	FVector	m_EdgeLeftPoint;				// @0
	FLOAT	m_fEdgeLeftAdjustedDistance;	// @12
	FVector	m_EdgeRightPoint;				// @16
	FLOAT	m_fEdgeRightAdjustedDistance;	// @28

	FArkCpntLocoPathFindingEdge()
		: m_EdgeLeftPoint( 0.f, 0.f, 0.f ), m_fEdgeLeftAdjustedDistance( 0.f )
		, m_EdgeRightPoint( 0.f, 0.f, 0.f ), m_fEdgeRightAdjustedDistance( 0.f )
	{}
};

/**
 * DISHONORED(port): the second base. 2012 PDB IArkComponentPreAsyncWork, 4 bytes, vtable
 * { PreAsyncWorkTick(FLOAT), GetComponentBase() const } in that order.
 */
class IArkComponentPreAsyncWork
{
public:
	// DISHONORED(layout): no virtual destructor - retail's IArkComponentPreAsyncWork_vtbl (2012 PDB) is exactly eight
	// bytes, { PreAsyncWorkTick, GetComponentBase }, and a component is only ever destroyed through FArkComponentBase.
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep ) = 0;
	virtual const FArkComponentBase* GetComponentBase() const = 0;
};

/**
 * DISHONORED(port): agent EN (PHASE13 EN). The second PreAsyncWork policy, the one FArkComponentManager ticks just
 * before the procedural-animation pass. It adds no method of its own: retail's
 * IArkComponentPreAsyncWorkJustBeforeProceduralAnim vftable (2013 rva 0xc1bda4) is exactly two __purecall entries, and
 * every component that implements it carries PreAsyncWorkTick at slot 0 and GetComponentBase at slot 1 in the vftable
 * of its subobject - FDisAIMonitorReaction 0xd3a354, FDisAIMonitorPawnReachability 0xd3a2d8,
 * FDisAIKnowledgeComponent 0xd328f4. The distinct type is what lets the manager keep a second list,
 * FArkComponentPolicy<IArkComponentPreAsyncWorkJustBeforeProceduralAnim,1> - whose DoneTicking is 2012 rva 0x578c50
 * and 2013 rva 0x537ed0, the second of those from match_2012_2013.csv (ratio 0.786, by neighbours) because the 2013
 * build has no name for it.
 */
class IArkComponentPreAsyncWorkJustBeforeProceduralAnim : public IArkComponentPreAsyncWork
{
};

/*-----------------------------------------------------------------------------
	FArkComponentLocomotion
-----------------------------------------------------------------------------*/

class FArkComponentLocomotion : public FArkComponentBase, public IArkComponentPreAsyncWork
{
	ARKCOMPONENT_DECLARE_TYPE( ArkCpntType_Locomotion, "ArkCpntType_Locomotion" )

public:
	/** DISHONORED(layout): 2012 PDB FArkComponentLocomotion::ECpntLocoRequestType. */
	enum ECpntLocoRequestType
	{
		CPNT_LOCO_REQUEST_TYPE_ACTOR	= 0,
		CPNT_LOCO_REQUEST_TYPE_LOCATION	= 1,
	};

	/**
	 * DISHONORED(layout): 2012 PDB FArkComponentLocomotion::EArkCpntLocoRootState. The root state is what the
	 * turn-in-place / start / stop animation system owns; NONE means "the path drives the pawn".
	 */
	enum EArkCpntLocoRootState
	{
		CPNT_LOCO_ROOT_STATE_NONE					= 0,
		CPNT_LOCO_ROOT_STATE_TURN_WAIT_GOOD_FOOT	= 1,
		CPNT_LOCO_ROOT_STATE_TURN_WAIT_ANIM			= 2,
		CPNT_LOCO_ROOT_STATE_TURN_BEGIN				= 3,
		CPNT_LOCO_ROOT_STATE_TURN					= 4,
		CPNT_LOCO_ROOT_STATE_TURN_END				= 5,
		CPNT_LOCO_ROOT_STATE_START_WAIT_ANIM		= 6,
		CPNT_LOCO_ROOT_STATE_START_BEGIN			= 7,
		CPNT_LOCO_ROOT_STATE_START					= 8,
		CPNT_LOCO_ROOT_STATE_START_END				= 9,
		CPNT_LOCO_ROOT_STATE_STOP_APPROACH			= 10,
		CPNT_LOCO_ROOT_STATE_STOP_WAIT_ANIM			= 11,
		CPNT_LOCO_ROOT_STATE_STOP_BEGIN				= 12,
		CPNT_LOCO_ROOT_STATE_STOP					= 13,
		CPNT_LOCO_ROOT_STATE_STOP_END				= 14,
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLocomotion::FLocoRequestData, 72 bytes: type, the common props, then
	    the union of the two kinds' own props (28 for a location, 16 for an actor), which is why 4 + 40 + 28 = 72. */
	struct FLocoRequestData
	{
		ECpntLocoRequestType			m_Type;			// @0
		FArkCpntLocoRequestCommonProps	m_CommonProps;	// @4
		union											// @44
		{
			FArkCpntLocoRequestLocationProps	m_LocationProps;
			FArkCpntLocoRequestActorProps		m_ActorProps;
		};

		FLocoRequestData() : m_Type( CPNT_LOCO_REQUEST_TYPE_LOCATION ) { appMemzero( &m_LocationProps, sizeof( m_LocationProps ) ); }
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLocomotion::FLocoRequestWorkData, 20 bytes: the four events the active
	    request has already been told about, so none of them is delivered twice, plus whether it was disabled. */
	struct FLocoRequestWorkData
	{
		UBOOL m_bThresholdAlreadyReached;
		UBOOL m_bDestinationAlreadyReached;
		UBOOL m_bPathfindAlreadySucceed;
		UBOOL m_bPathfindAlreadyFailed;
		UBOOL m_bDisabled;

		FLocoRequestWorkData() { appMemzero( this, sizeof( FLocoRequestWorkData ) ); }
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLocomotion::FArkCpntLocoTurnProps, 44 bytes. */
	struct FArkCpntLocoTurnProps
	{
		FVector	m_Location;
		INT		m_StartYaw;
		INT		m_EndYaw;
		FLOAT	m_fTurnStartSpeed;
		INT		m_ModifierIdx;
		INT		m_SpeedModeIdx;
		INT		m_TurnAnimIdx;
		UBOOL	m_bPlayNow;
		UBOOL	m_bInMovement;

		FArkCpntLocoTurnProps() { appMemzero( this, sizeof( FArkCpntLocoTurnProps ) ); }
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLocomotion::FLocoCpntSharedProps, 24 bytes: one row per live component
	    in the static table every locomotion component can read, which is how NPCs give way to each other without a
	    crowd manager. */
	struct FLocoCpntSharedProps
	{
		FArkComponentLocomotion*	m_pLocoCpnt;
		FVector						m_FinalLocation;
		UBOOL						m_bCloseToFinalLocation;
		UBOOL						m_bMustIgnoreAvoidance;
	};

	FArkComponentLocomotion();
	virtual ~FArkComponentLocomotion();

	// ---- FArkComponentBase ----
	virtual void Starting();										// 2013 0x54ac70
	virtual void Stopping();										// 2013 0x54ad50
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentLocomotion ); }	// 2013 0x53ddc0
	virtual DWORD GetAllocatedSize() const;							// 2013 0x53f490

	// ---- IArkComponentPreAsyncWork ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );				// 2013 0x54caa0
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	// ---- configuration ----
	static UBOOL IsConfigValid( const UArkComponentLocomotionConfig* const _pConfig, const APawn* _pPawn );	// 2013 0x53f4c0
	UBOOL SetConfig( const UArkComponentLocomotionConfig* const _pConfig );	// 2013 0x543060
	static FArkComponentLocomotion* GetLocomotionComponent( const APawn* _pPawn );	// 2013 0x543110
	const UArkComponentLocomotionConfig* GetConfig() const { return m_pConfig; }

	void SetOwnerPawnAvoidable( const UArkAvoidable* const _pAvoidable ) { m_pAvoidable = _pAvoidable; }	// 2013 0x53ddd0
	void SetModifier( INT _ModifierIdx );							// 2013 0x541b70
	INT GetModifier() const { return m_CurModifierIdx; }			// 2013 0x5cb100
	const struct FArkCpntLocoSpeedModeProp& GetSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const;	// 2013 0x53f9b0
	FLOAT GetForwardSpeedOfSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const;	// 2013 0x53e9c0
	FLOAT GetFinalSpeedMultiplier() const;							// 2013 0x541b00
	INT ComputeMostRelevantSpeedIdx() const;						// 2013 0x53f090
	INT GetMostRelevantSpeedIdx() const { return m_MostRelevantSpeedIdx; }	// 2013 0x57f120

	// ---- the public request interface (arkcomponentlocomotiongoto.cpp) ----
	INT StartLocoToLocation( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntLocoGoToLocationProp& _GoToProperties );	// 2013 0x545010+
	INT StartLocoToActor( const void* const _pAsker, FName _AskerName, INT _Priority, const FArkCpntLocoGoToActorProp& _GoToProperties );		// 2013 0x545010
	UBOOL UpdateLocoToLocation( INT _RequestID, const FArkCpntLocoGoToLocationProp& _GoToProperties );	// 2013 0x546420
	UBOOL UpdateLocoToActor( INT _RequestID, const FArkCpntLocoGoToActorProp& _GoToProperties );			// 2013 0x546360
	UBOOL StopLoco( INT _RequestID );								// 2013 0x546480
	UBOOL StopAllLocoFromAsker( const UObject* const _pAsker );		// 2013 0x5464c0
	INT GetRequestsCount() const { return m_ReqMgr.GetRequestsCount(); }	// 2013 0x541b60
	UBOOL HasRequest() const { return m_ReqMgr.HasRequest(); }
	FVector GetActiveRequestTargetedLocation() const;				// 2013 0x5464d0
	FVector GetAskedTargetLocOfActiveRequest() const;				// 2013 0x5451e0
	INT GetActiveRequestMaxSpeedIdx() const;						// 2013 0x545120
	UBOOL ComputeHasRequestWithPathComputedFlag() const;			// 2013 0x543130
	void ForceActiveRequestRepath();								// 2013 0x53e620
	UBOOL IsRequestStarted( INT _RequestID ) const { return m_ReqMgr.GetRequestIdxByID( _RequestID ) != INDEX_NONE; }

	void Enable();													// 2013 0x53e630
	void Disable();													// 2013 0x546520
	UBOOL IsDisabled() const;										// 2013 0x53e650
	void AllowToFall( UBOOL _bAllow );								// 2013 0x53e0b0
	void EnableTeleportWhenOutsideOfNavmesh( UBOOL _bEnable );		// 2013 0x57f7c0 (2012)
	void HandleTeleport();											// 2013 0x546550

	// ---- steering (arkcomponentlocomotiondynamic.cpp) ----
	void AddSteeringForce( const FVector& _Force );					// 2013 0x53dfe0
	const FVector& GetSteeringForce() const { return m_CurSteeringForce; }	// 2013 0x53e020
	void EnableSteering()	{ m_bSteeringDisabled = FALSE; }		// 2013 0x53e0a0
	void DisableSteering()	{ m_bSteeringDisabled = TRUE; m_CurSteeringForce = FVector( 0.f, 0.f, 0.f ); }	// 2013 0x53e080
	void EnableInertia()	{ m_bInertiaDisabled = FALSE; }			// 2013 0x57f1c0 (2012)
	void DisableInertia()	{ m_bInertiaDisabled = TRUE; }			// 2013 0x57f1b0 (2012)
	const FVector& GetPathMove() const { return m_CurPathMove; }		// 2013 0x53df50
	FLOAT GetRealSpeed() const { return m_fRealSpeed; }				// 2013 0x53df40
	UBOOL IsRootMotion() const;										// 2013 0x53ed30
	UBOOL IsArrived() const { return m_bIsArrived != 0; }			// 2013 0x546210
	FLOAT GetSq2DDistToPathEnd() const { return m_fSq2DDistToPathEnd; }
	FLOAT GetCurMoveSpeed() const { return m_fCurMoveSpeed; }
	FLOAT GetTargetMoveSpeed() const { return m_fTargetMoveSpeed; }
	UBOOL HasComputedPath() const { return m_bHasRequestWithPathComputed != 0; }
	INT GetStopCount() const { return 0; }	// 2013 0x546240
	FVector GetDirectionToNextPathPoint() const;					// 2013 0x53df60
	void MovePawn( FLOAT _fTimeStep, UBOOL _bPreview );				// 2013 0x543d00

	// ---- the nav-mesh queries, which are the only four entries the AI has into the mesh ----
	UBOOL CanNPCPathfindFromLocation( const FVector* const _pStartLocation ) const;	// 2013 0x546770
	UBOOL FindNearestLocationOnNavMesh( const FVector* const _pPawnLocOnGround, FLOAT _fSearchDist, FVector& _rNearestLocationOnNavMesh ) const;	// 2013 0x545660
	UBOOL UpdateStartLocAndVerifyIfOnValidPoly( FVector& _rStartLocationOnGround, const FVector* const _pSpecificStartLoc ) const;	// 2013 0x53ed10

	ADishonoredNPCPawn* GetPawnOwner() const { return m_pPawnOwner; }
	const TArray<FArkCpntLocoPathPoint>& GetPathPoints() const { return m_PathPoints; }
	INT GetCurPathPointIdx() const { return m_CurPathPointIdx; }
	BYTE GetLastPathFindingError() const { return (BYTE)m_LastPFError; }

private:
	// the request-manager event thunk (see arkrequestmanager.h on why this is a static rather than a PMF)
	static void RequestManagerEventThunk( FArkComponentBase* _pOwner, const FArkRequestManager<FLocoRequestData>::EArkReqMgrEvent _Event, const INT _RequestIdx );
	void OnRequestManagerEvent( const FArkRequestManager<FLocoRequestData>::EArkReqMgrEvent _Event, INT _RequestIdx );	// 2013 0x546660

	void OnEnable();												// 2013 0x53def0
	void OnDisable();												// 2013 0x5461f0
	void UpdateLocoRequest( INT _RequestIdx, const FLocoRequestData& _rRequest );	// 2013 0x545160
	void ResetCurRequestWorkData();									// 2013 0x5451b0
	void NotifyActiveRequest( BYTE _Event );

	// path (arkcomponentlocomotionpath.cpp)
	void UpdatePathOfHigherPriorityRequest( FLOAT _fTimeStep, UBOOL _bForcePFComputation, const FVector* const _pForcedDestination );	// 2013 0x549890
	void UpdatePathProperties();									// 2013 0x541ee0
	void ResetPathProperties();										// 2013 0x545410
	UBOOL UpdatePathFindingEdges( const FVector& _StartLocation, const FVector& _EndLocation );	// 2013 0x5468f0
	UBOOL UpdateBuiltPathPoints();									// 2013 0x546a90
	UBOOL DefaultPathBuilder( const FVector& _StartLocation, const FVector& _EndLocation, FLOAT _fStartSpeed, FLOAT _fDestinationSpeed, FLOAT _fMaxSpeed, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _PathPoints );	// 2013 0x545910
	FVector ComputeTargetedLocationForPath( FLOAT _fTimeStep, UBOOL _bForNextTick );	// 2013 0x545270
	INT ComputePathYawFromDist( FLOAT _fDist ) const;				// 2013 0x5428b0

	// dynamics (arkcomponentlocomotiondynamic.cpp)
	void ComputeDynamic( FLOAT _fTimeStep );						// 2013 0x54bec0
	void FullMovePawn( FLOAT _fTimeStep, UBOOL _bOnValidBase, UBOOL _bPreview );	// 2013 0x53fb00
	UBOOL ModerateMovePawn( FLOAT _fTimeStep );						// 2013 0x540d80
	INT ComputeMoveYaw() const;										// 2013 0x53e1f0
	INT ComputeTargetMoveYaw() const;								// 2013 0x547950
	INT UpdateMoveYaw( FLOAT _fTimeStep, FLOAT& _rCurRotSpeed, FLOAT& _rMaxRotSpeed, FLOAT& _rMaxMultiDirSpeedForAngle ) const;	// 2013 0x541330
	FLOAT ComputeTargetMoveSpeed( FLOAT _fTimeStep, FLOAT& _rAdaptedAccel ) const;	// 2013 0x548050
	FLOAT UpdateMoveSpeed( FLOAT _fTimeStep ) const;				// 2013 0x53e220
	FVector ComputePathMove( FLOAT _fTimeStep );					// 2013 0x548410
	FVector ApplySteering( FLOAT _fTimeStep, const FVector& _PathMove );	// 2013 0x54a630
	FVector ClampMove( FLOAT _fTimeStep, const FVector& _Move, FLOAT& _rMaxMoveUpDist ) const;	// 2013 0x544490
	void HandleArrival( FLOAT _fTimeStep );							// 2013 0x5487b0
	UBOOL ComputeIsArrivedFlagAndSq2DDistToPathEnd( FLOAT& _rSq2DDist ) const;	// 2013 0x544390
	UBOOL IsThereAnotherForceThanPath() const;						// 2013 0x541a00
	void UpdateSharedProperties();									// 2013 0x546260
	FVector GetPawnGroundLocation() const;							// 2013 0x53dde0

public:
	/** DISHONORED(layout): retail keeps the live components in a file-scope static (2012 rva 0x1058b60) so that
	    UpdateSharedProperties and ComputePriorityOfNPCsFromSharedProps can see the whole crowd without a manager. */
	static TArray<FLocoCpntSharedProps>	ms_LocoCpntSharedProps;
	static INT							ms_ComponentCount;
	/** Retail's path budget: at most one new A* per component per (component count + 1) ticks. */
	static INT							ms_TickCountBeforePath;

private:
	// --- the 616-byte layout, in the 2012 PDB's order. Offsets are that PDB's and are asserted in the .cpp. ---
	const UArkComponentLocomotionConfig*	m_pConfig;						// @20
	FArkComponentMeshOffset*				m_pMeshOffsetCpnt;				// @24
	ADishonoredNPCPawn*						m_pPawnOwner;					// @28
	const UArkAvoidable*					m_pAvoidable;					// @32
	BITFIELD								m_bDisabled:1;					// @36 0x1
	BITFIELD								m_bPrevHidden:1;				// @36 0x2
	BITFIELD								m_bNoTick:1;					// @36 0x4
	FArkRequestManager<FLocoRequestData>	m_ReqMgr;						// @40
	FLocoRequestWorkData					m_CurReqWorkData;				// @76
	TArray<FArkCpntLocoPathFindingEdge>		m_PathFindingEdges;				// @96
	TArray<FArkCpntLocoPathPoint>			m_PathPoints;					// @108
	FPathStore								m_RawPathFindingEdges;			// @120
	INT										m_CurPathPointIdx;				// @144
	INT										m_CurPathYaw;					// @148
	INT										m_NextPathYaw;					// @152
	FVector									m_CurPathDir;					// @156
	FVector									m_NextPathDir;					// @168
	FVector									m_PathPoint;					// @180
	FVector									m_LastPathPoint;				// @192
	FVector									m_NearestLocationOnNavMesh;		// @204
	FVector									m_StartLocationUsedToAskPath;	// @216
	FVector									m_EndLocationUsedToAskPath;		// @228
	INT										m_LastPFError;					// @240 (EPathFindingError, a dword in retail)
	FLOAT									m_fTryToReturnOnNavMeshDuration;// @244
	FLOAT									m_fDistBetweenPathPoints;		// @248
	BITFIELD								m_bTryToReturnOnNavMesh:1;		// @252 0x1
	BITFIELD								m_bPathfindingUpToDate:1;		// @252 0x2
	BITFIELD								m_bBuiltPathUpToDate:1;			// @252 0x4
	BITFIELD								m_bPathIsDirty:1;				// @252 0x8
	BITFIELD								m_bStartingNewPath:1;			// @252 0x10
	BITFIELD								m_bPathAtEnd:1;					// @252 0x20
	BITFIELD								m_bForcePathComputation:1;		// @252 0x40
	BITFIELD								m_bMustTestPathAfterTurn:1;		// @252 0x80
	BITFIELD								m_bHasBeenOnNavMesh:1;			// @252 0x100
	BITFIELD								m_bHasRequestWithPathComputed:1;// @252 0x200
	INT										m_CurMoveYaw;					// @256
	FLOAT									m_fRealSpeed;					// @260
	FLOAT									m_fRealScaledSpeed;				// @264
	FLOAT									m_fCurMoveSpeed;				// @268
	FLOAT									m_fCurMoveRotationSpeed;		// @272
	FLOAT									m_fMaxMoveRotationSpeed;		// @276
	FLOAT									m_fMaxMoveRotSpeedMultiplier;	// @280
	FLOAT									m_fMaxMultiDirSpeedForAngle;	// @284
	INT										m_TargetMoveYaw;				// @288
	FLOAT									m_fTargetMoveSpeed;				// @292
	FLOAT									m_fSteeringMultiplier;			// @296
	FVector									m_CurPawnLocation;				// @300
	FVector									m_CurSteeringForce;				// @312
	FVector									m_PreviousSteeringForce;		// @324
	FVector									m_PreviousPathForce;			// @336
	FVector									m_CurPathMove;					// @348
	FVector									m_CurMove;						// @360
	FVector									m_PreviousVelocity;				// @372
	FVector									m_MoveHitNormal;				// @384
	FVector									m_TeleportLocation;				// @396
	INT										m_CurModifierIdx;				// @408
	INT										m_MostRelevantSpeedIdx;			// @412
	INT										m_MaxMultiDirSpeedIdx;			// @416
	BITFIELD								m_bInertiaDisabled:1;			// @420 0x1
	BITFIELD								m_bSteeringDisabled:1;			// @420 0x2
	BITFIELD								m_bFallAllowedByExternal:1;		// @420 0x4
	BITFIELD								m_bFallAllowedByCode:1;			// @420 0x8
	BITFIELD								m_bStuckedInAWall:1;			// @420 0x10
	BITFIELD								m_bTeleportRequired:1;			// @420 0x20
	BITFIELD								m_bTeleportDisabled:1;			// @420 0x40
	BITFIELD								m_bIsArrived:1;					// @420 0x80
	FLOAT									m_fSq2DDistToPathEnd;			// @424
	FLOAT									m_fCurAcceleration;				// @428
	FLOAT									m_fCurDeceleration;				// @432
	FLOAT									m_fCurAdaptedAccel;				// @436
	FLOAT									m_fMultiDirSpeedRatio;			// @440
	INT										m_LocoCpntIdx;					// @444
	FVector									m_PushStartLocOnPath;			// @448
	FVector									m_PushReturnOnPathLoc;			// @460
	INT										m_PushReturnOnPathRequestIdx;	// @472
	BITFIELD								m_bInPushPeriod:1;				// @476 0x1
	BITFIELD								m_bSomeoneWantsToPushMe:1;		// @476 0x2
	FLOAT									m_fCurPitch;					// @480
	FLOAT									m_fCurRoll;						// @484
	FLOAT									m_fCurPitchRotateSpeed;			// @488
	FLOAT									m_fCurRollRotateSpeed;			// @492
	FLOAT									m_fCurPitchRollZOffset;			// @496
	FLOAT									m_fTargetPitch;					// @500
	FLOAT									m_fTargetRoll;					// @504
	FLOAT									m_fFollowDeadZoneEndDuration;	// @508
	FLOAT									m_fCurFollowAngle;				// @512
	FLOAT									m_fCurFollowAccelMultiplier;	// @516
	FLOAT									m_fFollowPredictRatio;			// @520
	INT										m_CurFollowedActorMoveYaw;		// @524
	EArkCpntLocoRootState					m_RootState;					// @528
	FArkCpntLocoTurnProps					m_TurnProps;					// @532
	FLOAT									m_fBeforeTurnSquaredDist;		// @576
	FLOAT									m_fBeforeTurnDuration;			// @580
	BITFIELD								m_bHandleStop:1;				// @584 0x1
	BITFIELD								m_bHaveStopAnim:1;				// @584 0x2
	FArkComponentFaceTo*					m_pFaceToCpnt;					// @588
	FArkComponentLookat*					m_pLookatCpnt;					// @592
	INT										m_FaceToRequestID;				// @596
	INT										m_LookatRequestID;				// @600
	FLOAT									m_fLookatSpeedMultiplier;		// @604
	BITFIELD								m_bCanManageAnim:1;				// @608 0x1
	UArkAnimNodeLocomotion*					m_pAnimNodeLoco;				// @612
};

/*-----------------------------------------------------------------------------
	arkpathbuildutils.cpp: the free functions that turn a nav-mesh edge list into a walkable polyline.
	Retail's unit is Engine/Src/arkpathbuildutils.cpp, 8 functions, 6,530 bytes.
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0xacd50 region (2012 0xacd50): is the pulled-in corner still on the mesh? */
UBOOL IsEdgeBorderExtentReachable( const FVector& _Vert, const FVector& _EdgeCenter, const FVector& _BorderExtent, const FVector& _PerpExtent, class UNavigationHandle* _pNavHandle );

/** DISHONORED(port): 2013 rva 0xb5110 region (2012 0xb5110): one adjusted edge, with both walls pulled in and both ends
    flagged reachable or not. */
void AddAdjustedPathFindingEdge( const FVector& _LeftVert, const FVector& _RightVert, const FVector& _EdgeDir, FLOAT _fEdgeSize, UBOOL _bLeftUnreachable, UBOOL _bRightUnreachable, const FVector& _PerpExtent, TArray<FArkCpntLocoPathFindingEdge>& _AdjustedPathFindingEdges );

/** DISHONORED(port): 2012 0xb4d10: the first edge of the store the pawn has not crossed yet. */
INT FindFirstEdgeIndex( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, const FPathStore& _PathFindingEdges, UBOOL& _rbFlipFirstEdge );

/** DISHONORED(port): 2012 0xb54d0: the whole edge list, pulled in from the walls by the pawn's radius. */
void AdjustPathFindingEdges( const FVector& _FirstPathPoint, const FVector& _EndPathPoint, class UNavigationHandle* _pNavHandle, FPathStore& _PathFindingEdges, FLOAT _fMinDistanceFromWall, TArray<FArkCpntLocoPathFindingEdge>& _AdjustedPathFindingEdges );

/** DISHONORED(port): 2012 0xb5cf0 / 0xb5990: Arkane's funnel (simple stupid funnel) over the adjusted edges. */
void MinimizeStraightPath_FunnelAlgorithm( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out );

/** DISHONORED(port): 2012 0xb5fe0 */
void AdjustStraightPath( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out );

/** DISHONORED(port): 2013 rva 0xb9310 (2012 0xb64e0): no edges means a straight line from A to B. */
void BuildStraightPath( const FVector& _FirstPathPoint, const FVector& _LastPathPoint, const TArray<FArkCpntLocoPathFindingEdge>& _PathFindingEdges, TArray<FArkCpntLocoPathPoint>& _StraightPath_Out );

/*-----------------------------------------------------------------------------
	The census counters this package adds. Read by Src/disaicensus.cpp; bumped from the locomotion units.
-----------------------------------------------------------------------------*/
extern INT GDisLocoComponents;			// components created
extern INT GDisLocoRequestsStarted;		// StartLocoTo{Location,Actor}
extern INT GDisLocoRequestsUpdated;		// UpdateLocoTo{Location,Actor}
extern INT GDisLocoRequestsStopped;		// StopLoco / StopAllLocoFromAsker
extern INT GDisLocoPathsBuilt;			// A* searches that produced a path
extern INT GDisLocoPathsFailed;			// A* searches that did not
extern INT GDisLocoPathPointsBuilt;		// path points the funnel produced
extern INT GDisLocoArrivals;			// CPNT_LOCO_EVENT_DESTINATION_REACHED raised
extern INT GDisLocoMovePawnCalls;		// MovePawn ticks
extern FLOAT GDisLocoDistanceMoved;		// total 2D distance the component translated its pawns
extern INT GDisLocoTeleports;		// NPCs put back onto the navigation mesh

#endif
