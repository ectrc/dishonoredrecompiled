#pragma once
// DishonoredGame/inc/arkcomponentlookat.h
// DISHONORED(port): agent FT (PHASE17 FT). Arkane's look-at component: the thing that turns an AI's "look there" into
// the aim the NPC anim tree's UArkAnimNodeLookAt blends, the two eye bone controllers and the blink. Retail's 51
// functions live in Engine/Src/arkcomponentlookat.cpp (2012 rvas 0x58df30..0x597260, 2013 0x54d360..0x556a00) and the
// class is declared in Engine/Inc/arkcomponentlookat.h.
//
// DISHONORED(bringup): it is declared HERE, in DishonoredGame, for agent DN's reason (Inc/arkcomponentlocomotion.h):
// the component cannot be compiled without UArkComponentLookatConfig and UArkAnimNodeLookAt, both reflected Engine-
// package classes the generator declares in DishonoredGame/Inc/DishonoredGameEngineShims.h. The Engine skeletons
// Engine/Src/arkcomponentlookat.cpp, arkcomponentlookatconfig.cpp and arkanimnodelookat.cpp stay comment-only.
//
// The per-frame order is retail's, driven from two Engine seams (Engine/Inc/arkcomponentbase.h):
//   FArkComponentManager's PreAsyncWorkJustBeforeProceduralAnim policy -> PreAsyncWorkTick(dt): the request queue,
//       the blink, the target, the eyes, then the head/torso aim handed to the node. In this tree the policy is a
//       hook USkeletalMeshComponent::Tick runs on the pawn's body mesh just before TickAnimNodes.
//   USkeletalMeshComponent::UpdateSkelPoseEnd (2013 rva 0x3471d0) -> OnComposeSkeleton(): the eye clamp, after
//       ComposeSkeleton. Hooked at the same place in USkeletalMeshComponent::UpdateSkelPose.

#ifndef _INC_ARKCOMPONENTLOOKAT
#define _INC_ARKCOMPONENTLOOKAT

#include "arkcomponentbase.h"
#include "arkrequestmanager.h"
#include "arkcomponentlocomotion.h"

class APawn;
class AActor;
class USkeletalMeshComponent;
class USkelControlSingleBone;
class UAnimNodeBlendBase;
class UAnimNodeSequence;
class UArkAnimNodeLookAt;
class UArkComponentLookatConfig;
class FArkComponentFaceTo;
class FArkGameEvent;

/*-----------------------------------------------------------------------------
	BlendInfos - the node's triangle mapping from a look-at angle to the 3x3 aim grid
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(layout): 2012 PDB BlendInfos, 88 bytes. One per grid (head, torso), owned by UArkAnimNodeLookAt through
 * m_HeadBlendInfos / m_TorsoBlendInfos. m_CustomRanges holds the nine grid corners as angles (radians) relative to the
 * centre, re-ordered round the circle: [0] centre, [1] right, [2] right-up, [3] up, [4] left-up, [5] left, [6] left-down,
 * [7] down, [8] right-down - the same order as ms_SquarePositions, which holds the unit-square position of each corner
 * in the node's normalised [-1,1] aim space. UpdateDirection picks the two corners the requested direction falls
 * between (m_P1Index, m_P2Index) and the range along that direction (m_MaxRange); Map interpolates the requested angle
 * inside the triangle (centre, P1, P2) into normalised space.
 */
class BlendInfos
{
public:
	BlendInfos();

	// DISHONORED(port): 2013 rva 0x50a140 (2012 0x54a9d0), 362 bytes. _CustomRanges is the 3x3 grid in row-major
	// order (left-up, up, right-up, left, centre, right, left-down, down, right-down).
	void UpdateRanges( const FVector2D* const _CustomRanges );
	// DISHONORED(port): 2013 rva 0x50c4c0 (2012 0x54cc70), 234 bytes.
	void UpdateDirection( const FVector2D& _rRequestedAngleDirection );
	// DISHONORED(port): 2013 rva 0x5175c0 (2012 0x557a50), 296 bytes. Clamps _rRequestedAngle to the grid in place.
	FVector2D Map( FVector2D& _rRequestedAngle ) const;

	FVector2D	m_CustomRanges[9];
	FVector2D	m_MaxRange;
	INT			m_P1Index;
	INT			m_P2Index;

private:
	// DISHONORED(port): 2013 rva 0x50a2b0 (2012 0x54ab40), 185 bytes.
	static void staticInit();
	// DISHONORED(port): 2013 rva 0x50a370 (2012 0x54ac00), 188 bytes.
	static UBOOL GetP1P2( const FVector2D* const _CustomRanges, INT& _rP1Index, INT& _rP2Index, const FVector2D& _rRequestedAngleDirection );
	// DISHONORED(port): 2013 rva 0x50a430 (2012 0x54acc0), 228 bytes.
	static UBOOL IsOutside( const FVector2D& _rRequestedAngle, const FVector2D* const _CustomRanges, INT _P1Index, INT _P2Index );
	// DISHONORED(port): 2013 rva 0x511bc0 (2012 0x5522b0), 2,479 bytes.
	static FVector2D MapIntoNormalizedSpace( const FVector2D& _rRequestedAngle, const FVector2D* const _Vertices, const FVector2D* const _ValuesToInterpolate );

	static UBOOL		ms_Init;
	static FVector2D	ms_SquarePositions[9];
};

/*-----------------------------------------------------------------------------
	FArkComponentLookat
-----------------------------------------------------------------------------*/

class FArkComponentLookat : public FArkComponentBase, public IArkComponentPreAsyncWorkJustBeforeProceduralAnim
{
	ARKCOMPONENT_DECLARE_TYPE( ArkCpntType_Lookat, "ArkCpntType_Lookat" )

public:
	/** DISHONORED(layout): 2012 PDB FArkComponentLookat::ECpntLookatRequestType; the switch in UpdateVariablesFromRequest. */
	enum ECpntLookatRequestType
	{
		CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION		= 0,
		CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR			= 1,
		CPNT_LOOKAT_REQUEST_TYPE_AIM_POSITION			= 2,
		CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR				= 3,
		CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL				= 4,
		CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_POSITION	= 5,
		CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_ACTOR	= 6,
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLookat::FLookatLocationRequestData, 16 bytes. */
	struct FLookatLocationRequestData
	{
		FVector	m_Location;
		UBOOL	m_bLocal;
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLookat::FLookatActorRequestData, 12 bytes. */
	struct FLookatActorRequestData
	{
		const AActor*	m_pActor;
		FName			m_AttachName;
	};

	/** DISHONORED(layout): 2012 PDB FArkComponentLookat::FLookatRequestData, 52 bytes: what the request manager queues. */
	struct FLookatRequestData
	{
		ECpntLookatRequestType		m_Type;				// @0
		FLOAT						m_fSpeed;			// @4
		FDisLookAtInfluence			m_LookAtInfluence;	// @8
		FLookatLocationRequestData	m_LocationData;		// @20
		FLookatActorRequestData		m_ActorData;		// @36
		INT							m_PatternIdx;		// @48
	};

	/**
	 * DISHONORED(layout): 2012 PDB FArkComponentLookat::DynamicInterpolator<T>, 40 bytes for float and 64 for FVector2D.
	 * A critically damped spring with a speed cap and a clamp: Update (2013 rvas 0x54dff0 / 0x54e240) accelerates the
	 * value toward m_TargetValue with m_MaxStrength scaled by (m_fSpeedFactor + 0.5)^4, bleeds speed through m_Friction
	 * (more inside m_DampingDistance), caps at m_MaxSpeed and never overshoots.
	 */
	template< typename T >
	struct DynamicInterpolator
	{
		T		m_CurrentValue;
		T		m_MinValue;
		T		m_MaxValue;
		T		m_CurrentSpeed;
		T		m_MaxSpeed;
		T		m_TargetValue;
		FLOAT	m_MaxStrength;
		FLOAT	m_Friction;
		FLOAT	m_DampingDistance;
		FLOAT	m_fSpeedFactor;

		void Update( FLOAT _fDeltaTime, const FLOAT* _pfOverrideSpeedFactor, const T* _pOverrideTarget );
	};

	FArkComponentLookat();												// 2013 0x555d60
	virtual ~FArkComponentLookat() {}									// 2013 0x556290

	// ---- FArkComponentBase ----
	virtual void ManageReferences( FGCHelper* _pHelper );				// 2013 0x555760
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentLookat ); }	// 2013 0x54d360
	virtual void Starting();											// 2013 0x556660
	virtual void Stopping();											// 2013 0x556a00
	virtual DWORD GetAllocatedSize() const;								// 2013 0x552620

	// ---- IArkComponentPreAsyncWork ----
	virtual void PreAsyncWorkTick( FLOAT _fTimeStep );					// 2013 0x555b40
	virtual const FArkComponentBase* GetComponentBase() const { return this; }

	// ---- configuration ----
	// DISHONORED(port): 2013 rva 0x54d370 (2012 0x58df40), 13 bytes: the store. The databases name it
	// FArkCpntFaceToProp::OverwriteExactRotation, a one-store function it was folded with; ADishonoredNPCPawn::Spawned
	// calls it on the look-at component with the NPC tweaks' m_pComponentLookatConfig.
	void SetConfig( const UArkComponentLookatConfig* _pConfig ) { m_pConfig = _pConfig; }
	const UArkComponentLookatConfig* GetConfig() const { return m_pConfig; }
	static FArkComponentLookat* GetLookatComponent( const APawn* _pPawnOwner );	// 2013 0x554a60

	// ---- the request entry points: what FDisLookAtRequest::DoRequest calls ----
	INT StartLookForward( const void* const _pAsker, const FName& _AskerName, INT _Priority, FLOAT _fLookAtSpeed, FLOAT _fDuration );	// 2013 0x554a80
	INT StartLookAtActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, const FName& _AttachName, FLOAT _fLookAtSpeed, FLOAT _fDuration );	// 2013 0x554b30
	INT StartLookAtLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, FVector _Location, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fLookAtSpeed, UBOOL _bLocal, FLOAT _fDuration );	// 2013 0x554be0
	INT StartAimAtActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration );	// 2013 0x554c80
	INT StartAimAtLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, FVector _Location, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration );	// 2013 0x554d10
	INT StartProceduralLookAt( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration );	// 2013 0x554da0
	INT StartProceduralLookAtOnActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, const FName& _AttachName, FLOAT _fDuration );	// 2013 0x554e40
	INT StartProceduralLookAtOnLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const FVector& _rLocation, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration );	// 2013 0x554ef0
	UBOOL StopLookAt( INT _RequestID );									// 2013 0x555710
	UBOOL StopAllLookAtFromAsker( const void* const _pAsker );			// 2013 0x555750
	const FLookatRequestData* GetActiveRequest() const;					// 2013 0x554f90
	INT GetActiveRequestPriority() const;								// 2013 0x554fd0
	FLOAT GetTimeLeft( INT _RequestID ) const;							// 2013 0x555010
	UBOOL IsRequestStarted( INT _RequestID ) const { return m_ReqMgr.GetRequestIdxByID( _RequestID ) != INDEX_NONE; }

	// ---- the mode and the geometry ----
	void SetLookAtMode( INT _LookAtMode );								// 2013 0x54e920
	void SetDebugLookAtMode( INT _DebugLookAtMode );						// 2012 0x58f5b0, unmatched in the 2013 database
	const FVector& GetHeadAt() const { return m_HeadAt; }				// 2013 0x54d750
	const FVector GetCurrentLookAtDirection() const;					// 2013 0x54d760
	FVector GetLookAtNeutralDirection();								// 2013 0x54d8b0
	FVector GetLookEyesPosition( FVector* _pHeadUp, FVector* _pHeadRight, FVector* _pHeadAt );	// 2013 0x54d380
	static FVector GetLookAtPosition( const AActor* _pActor, const FName& _AttachName );			// 2013 0x552bd0
	void OnComposeSkeleton();											// 2013 0x552640
	void OnLODChanged( const FArkGameEvent& _rGameEvent );				// 2013 0x54ee10

	// ---- the flags the anim notifies and the LOD manager flip ----
	UBOOL IsTorsoDisabled() const { return m_bTorsoDisabled; }
	UBOOL IsHeadDisabled() const { return m_bHeadDisabled; }
	UBOOL IsEyesDisabled() const { return m_bEyesDisabled; }
	UBOOL IsInLOD() const { return m_bInLOD; }
	UBOOL IsNoTick() const { return m_bNoTick; }
	UBOOL IsNotLookingAnywhere() const { return m_bIsNotLookingAnywhere; }
	INT GetLookAtMode() const { return m_LookAtMode; }
	INT GetTargetType() const { return m_TargetType; }
	const AActor* GetCurrentTarget() const { return m_pCurrentTarget; }
	const FVector& GetToTarget() const { return m_ToTarget; }
	FLOAT GetCurrentHeadRange() const { return m_CurrentHeadRange.m_CurrentValue; }
	FLOAT GetCurrentTorsoRange() const { return m_CurrentTorsoRange.m_CurrentValue; }
	const FVector2D& GetCurrentCursorPosition() const { return m_CurrentCursorPosition.m_CurrentValue; }
	INT GetRequestsCount() const { return m_ReqMgr.GetRequestsCount(); }
	UArkAnimNodeLookAt* GetLookAtNode() const { return m_pLookAtNode; }
	USkelControlSingleBone* GetSpineBenderNode() const { return m_pSpineBenderNode; }

private:
	void LookAt( const AActor* _pActor, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fSpeedFactor, const FName& _AttachName );	// 2013 0x54e5e0
	void LookAt( const FVector& _TargetPosition, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fSpeedFactor, UBOOL _bLocal );	// 2013 0x54e690
	void AimAt( const AActor* _pActor );								// 2013 0x54e740
	void AimAt( const FVector& _TargetPosition );						// 2013 0x54e7f0
	void LookForward();													// 2013 0x54e8a0
	void ProceduralLookAt( FLOAT _fTimeStep, const FLookatRequestData& _rRequest );	// 2013 0x552970
	void UpdateBlink( FLOAT _fTimeStep );								// 2013 0x552ec0
	void UpdateVariablesFromRequest( FLOAT _fTimeStep );				// 2013 0x553fb0
	void UpdateEyes( FLOAT _fTimeStep );								// 2013 0x54e960
	void UpdateHeadAndTorso( FLOAT _fTimeStep );						// 2013 0x553050

public:
	// DISHONORED(layout): the 2012 PDB's member list, in order, offsets after the two bases (@0..@19).
	const UArkComponentLookatConfig*	m_pConfig;							// @20
	APawn*								m_pPawnOwner;						// @24
	UArkAnimNodeLookAt*					m_pLookAtNode;						// @28
	USkelControlSingleBone*				m_pSpineBenderNode;					// @32
	USkeletalMeshComponent*				m_pSkeletalComponent;				// @36
	INT									m_HeadBoneIndex;					// @40
	INT									m_LeftEyeParentBoneIndex;			// @44
	INT									m_RightEyeParentBoneIndex;			// @48
	INT									m_LeftEyeBoneIndex;					// @52
	INT									m_RightEyeBoneIndex;				// @56
	USkelControlSingleBone*				m_pLeftEyeControl;					// @60
	USkelControlSingleBone*				m_pRightEyeControl;					// @64
	UAnimNodeBlendBase*					m_pBlinkControl;					// @68
	UAnimNodeSequence*					m_pBlinkAnim;						// @72
	FArkComponentFaceTo*				m_pFaceToCpnt;						// @76
	INT									m_LookAtMode;						// @80
	INT									m_DebugLookAtMode;					// @84
	BITFIELD							m_bTorsoDisabled:1;					// @88
	BITFIELD							m_bHeadDisabled:1;
	BITFIELD							m_bEyesDisabled:1;
	BITFIELD							m_bInLOD:1;
	BITFIELD							m_bNoTick:1;
	FArkRequestManager<FLookatRequestData>	m_ReqMgr;						// @92
	ECpntLookatRequestType				m_TargetType;						// @128
	const AActor*						m_pCurrentTarget;					// @132
	FName								m_AttachName;						// @136
	FVector								m_VectorTarget;						// @144
	DynamicInterpolator<FVector2D>		m_CurrentCursorPosition;			// @156
	DynamicInterpolator<FLOAT>			m_CurrentHeadRange;					// @220
	DynamicInterpolator<FLOAT>			m_CurrentTorsoRange;				// @260
	BITFIELD							m_bIsTorsoInfMoveSpeedDep:1;		// @300
	BITFIELD							m_bLocal:1;
	TArray<FArkCpntLookatProcSeqWork>	m_ProcWorkData;						// @304
	INT									m_ProcPatternIdx;					// @316
	INT									m_ProcSequenceIdx;					// @320
	INT									m_ProcRuleIdx;						// @324
	BITFIELD							m_bFaceFxIsPlaying:1;				// @328
	BITFIELD							m_bIsNotLookingAnywhere:1;
	FVector								m_ToTarget;							// @332
	FVector								m_HeadUp;							// @344
	FVector								m_HeadRight;						// @356
	FVector								m_HeadAt;							// @368
	FVector								m_NpcUp;							// @380
	FVector								m_NpcRight;							// @392
	FVector								m_NpcAt;							// @404
	FVector								m_EyeOffsets;						// @416
	FVector								m_CachedHeadPosition;				// @428
	FVector								m_DebugCurrentHeadPos;				// @440
	FVector								m_DebugCurrentHeadTarget;			// @452
	FLOAT								m_fBeforeNextBlinkDuration;			// @464
	DynamicInterpolator<FVector2D>		m_CurrentEyesPosition;				// @468
	DynamicInterpolator<FLOAT>			m_EyesControlStrength;				// @532
	FLOAT								m_UnclampedTargetXPosition;			// @572
	FVector2D							m_PreviousDelta;					// @576
	FVector2D							m_PreviousTarget;					// @584
	FLOAT								m_fDelay;							// @592
	DynamicInterpolator<FLOAT>			m_CurrentUseTorsoDeadZone;			// @596

	// DISHONORED(retail): the eight bone and node names the component looks up at Starting; 2012 dynamic
	// initialisers 0xb9fea0..0xb9ff80, 2013 0xb83720..0xb83800.
	static const FName s_CameraBoneName;
	static const FName s_HeadBoneName;
	static const FName s_LeftEyeBoneName;
	static const FName s_RightEyeBoneName;
	static const FName s_LeftEyeControlName;
	static const FName s_RightEyeControlName;
	static const FName s_BlinkControlName;
	static const FName s_BlinkAnimName;
};

#endif
