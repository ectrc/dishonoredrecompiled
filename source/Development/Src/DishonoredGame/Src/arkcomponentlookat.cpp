// DishonoredGame/src/arkcomponentlookat.cpp
// DISHONORED(port): agent FT (PHASE17 FT). The look-at component: its lifetime, its request queue, and the per-frame
// pass that turns "look at that" into the aim UArkAnimNodeLookAt blends, the eye bone controllers and the blink.
// Retail's unit is Engine/Src/arkcomponentlookat.cpp (51 functions); it is here rather than in Engine for the reason
// the header's banner gives. Every body below was read out of the 2012 decompile (shipping2012_v1.i64, imagebase 0)
// and its 2013 start checked by name in retail2013_named.i64; the two builds differ only in the array-assert strings.

#include "DishonoredGame.h"
#include "arkcomponentlookat.h"
#include "arkgameeventdispatcher.h"

ARKCOMPONENT_IMPLEMENT_TYPE( FArkComponentLookat )

// DISHONORED(retail): 2012 dynamic initialisers 0xb9fea0..0xb9ff80 (2013 0xb83720..0xb83800), the strings at
// 2012 .rdata 0xcb2094..0xcb20f8. camera_jnt is preferred over head_jnt because the NPC head's camera joint sits at
// the eyes, which is where a look-at is measured from.
const FName FArkComponentLookat::s_CameraBoneName( TEXT("camera_jnt") );
const FName FArkComponentLookat::s_HeadBoneName( TEXT("head_jnt") );
const FName FArkComponentLookat::s_LeftEyeBoneName( TEXT("eye_L_jnt") );
const FName FArkComponentLookat::s_RightEyeBoneName( TEXT("eye_R_jnt") );
const FName FArkComponentLookat::s_LeftEyeControlName( TEXT("EyeControl_Left") );
const FName FArkComponentLookat::s_RightEyeControlName( TEXT("EyeControl_Right") );
const FName FArkComponentLookat::s_BlinkControlName( TEXT("BlinkBlender") );
const FName FArkComponentLookat::s_BlinkAnimName( TEXT("Blink") );

// DISHONORED(retail): the tuning constants, read off the 2012 .rdata image at 0xcb204c..0xcb2084 (the constructor
// and UpdateEyes reference them by name). TURN_* drive the cursor spring, HEAD_AND_TORSO_RANGE_BLENDTIME_STRENGTH the
// three range springs, EYES_* the eye spring; the eye limits are degrees.
static const FLOAT TURN_MAX_STRENGTH = 100.f;
static const FLOAT TURN_FRICTION = 7.f;
static const FLOAT HEAD_AND_TORSO_RANGE_BLENDTIME_STRENGTH = 50.f;
static const FLOAT EYES_MAX_STRENGTH = 50000.f;
static const FLOAT MAX_EYE_PITCH = 5.f;
static const FLOAT MIN_EYE_PITCH = -15.f;
static const FLOAT MAX_EYE_YAW = 25.f;
static const FLOAT MIN_EYE_YAW = -25.f;

// DISHONORED(retail): the engine-side Ark game event the LOD manager raises; FArkGameEventDispatcher's ids 1..3 are the
// Ark events and 2 is "LOD changed" (Engine/Inc/arkgameeventdispatcher.h). Its parameter block is the LOD manager's;
// OnLODChanged reads one bit of its first word.
enum { ArkGameEventType_LODChanged = 2 };

INT GDisLookatComponents = 0;
INT GDisLookatStarted = 0;
INT GDisLookatRequestsStarted = 0;
INT GDisLookatRequestsStopped = 0;
INT GDisLookatProceduralTicks = 0;

// DISHONORED(layout): the 2012 PDB's sizes for everything the request manager memcpys and the component carries.
checkAtCompileTime( sizeof( FArkComponentLookat::FLookatLocationRequestData ) == 16, FLookatLocationRequestData_is_16 );
checkAtCompileTime( sizeof( FArkComponentLookat::FLookatActorRequestData ) == 12, FLookatActorRequestData_is_12 );
checkAtCompileTime( sizeof( FArkComponentLookat::FLookatRequestData ) == 52, FLookatRequestData_is_52 );
checkAtCompileTime( sizeof( FArkRequestManager<FArkComponentLookat::FLookatRequestData>::FRequest ) == 80, FLookatRequest_is_80 );
checkAtCompileTime( sizeof( FArkComponentLookat::DynamicInterpolator<FLOAT> ) == 40, DynamicInterpolator_float_is_40 );
checkAtCompileTime( sizeof( FArkComponentLookat::DynamicInterpolator<FVector2D> ) == 64, DynamicInterpolator_FVector2D_is_64 );
checkAtCompileTime( sizeof( BlendInfos ) == 88, BlendInfos_is_88 );
checkAtCompileTime( sizeof( FArkCpntLookatProcSeqWork ) == 20, FArkCpntLookatProcSeqWork_is_20 );
checkAtCompileTime( sizeof( FArkCpntLookatProcRuleWork ) == 32, FArkCpntLookatProcRuleWork_is_32 );
checkAtCompileTime( sizeof( FArkCpntLookatProcRule ) == 60, FArkCpntLookatProcRule_is_60 );
checkAtCompileTime( sizeof( FArkCpntLookatProcSeq ) == 36, FArkCpntLookatProcSeq_is_36 );
checkAtCompileTime( sizeof( FArkCpntLookatProcPattern ) == 28, FArkCpntLookatProcPattern_is_28 );
checkAtCompileTime( sizeof( FArkCpntLookatMode ) == 228, FArkCpntLookatMode_is_228 );
// DISHONORED(layout): the whole component. 2012 PDB sizeof 636; every offset in the header's member list came from
// that PDB record and a mistake anywhere shows up here rather than as a wrong number at runtime.
checkAtCompileTime( sizeof( FArkComponentLookat ) == 636, FArkComponentLookat_is_636 );

static FLOAT DisLookatRand01()
{
	// retail: rand() * 0.000030518509 = rand() / 32768
	return (FLOAT)rand() * ( 1.f / 32768.f );
}

/*-----------------------------------------------------------------------------
	DynamicInterpolator
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54dff0 (2012 0x58ec40), 589 bytes.
template<>
void FArkComponentLookat::DynamicInterpolator<FLOAT>::Update( FLOAT _fDeltaTime, const FLOAT* _pfOverrideSpeedFactor, const FLOAT* _pOverrideTarget )
{
	const FLOAT fSpeedFactor = _pfOverrideSpeedFactor ? *_pfOverrideSpeedFactor : m_fSpeedFactor;
	const FLOAT TargetValue = _pOverrideTarget ? *_pOverrideTarget : m_TargetValue;
	const FLOAT ToTarget = TargetValue - m_CurrentValue;
	const FLOAT Distance = Abs( ToTarget );
	const FLOAT CurrentSpeedLength = Abs( m_CurrentSpeed );
	if( Distance < 0.0001f && CurrentSpeedLength < 0.0001f )
	{
		m_CurrentValue = TargetValue;
		m_CurrentSpeed = 0.f;
		return;
	}

	const FLOAT ToTargetDirection = Distance > 0.00000001f ? ToTarget / Distance : 1.f;
	FLOAT FrictionDelta = -( m_Friction * m_CurrentSpeed ) * _fDeltaTime;
	if( m_DampingDistance > Distance )
	{
		FrictionDelta *= ( m_DampingDistance - Distance ) * 0.5f + 1.f;
	}
	m_CurrentSpeed += FrictionDelta;

	FLOAT Acceleration = m_MaxStrength;
	if( CurrentSpeedLength <= 0.0001f )
	{
		if( m_DampingDistance > Distance )
		{
			Acceleration = ( ( Distance / m_DampingDistance ) * 0.9f + 0.1f ) * m_MaxStrength;
		}
	}
	else
	{
		const FLOAT Braked = ( m_MaxStrength / CurrentSpeedLength ) * Distance;
		if( Braked < m_MaxStrength )
		{
			Acceleration = Braked;
		}
	}
	m_CurrentSpeed += appPow( fSpeedFactor + 0.5f, 4.f ) * Acceleration * ToTargetDirection * _fDeltaTime;
	m_CurrentSpeed = Clamp( m_CurrentSpeed, -m_MaxSpeed, m_MaxSpeed );
	if( m_CurrentSpeed * ToTarget < 0.f )
	{
		m_CurrentSpeed = 0.f;
	}

	const FLOAT OldValue = m_CurrentValue;
	m_CurrentValue += m_CurrentSpeed * _fDeltaTime;
	if( ( OldValue - TargetValue ) * ( m_CurrentValue - TargetValue ) <= 0.0001f
		|| ( Abs( m_CurrentValue - TargetValue ) < 0.0001f && Abs( m_CurrentValue ) < 0.0001f ) )
	{
		m_CurrentSpeed = 0.f;
		m_CurrentValue = TargetValue;
	}
	m_CurrentValue = Clamp( m_CurrentValue, m_MinValue, m_MaxValue );
}

// DISHONORED(port): 2013 rva 0x54e240 (2012 0x58ee90), 926 bytes. The 2D spring: the distance is the vector length, the
// speed cap and the clamp are per component, and each component snaps to its target independently.
template<>
void FArkComponentLookat::DynamicInterpolator<FVector2D>::Update( FLOAT _fDeltaTime, const FLOAT* _pfOverrideSpeedFactor, const FVector2D* _pOverrideTarget )
{
	const FLOAT fSpeedFactor = _pfOverrideSpeedFactor ? *_pfOverrideSpeedFactor : m_fSpeedFactor;
	const FVector2D TargetValue = _pOverrideTarget ? *_pOverrideTarget : m_TargetValue;
	const FVector2D ToTarget = TargetValue - m_CurrentValue;
	const FLOAT Distance = ToTarget.Size();
	const FLOAT CurrentSpeedLength = m_CurrentSpeed.Size();
	if( Distance < 0.0001f && CurrentSpeedLength < 0.0001f )
	{
		m_CurrentValue = TargetValue;
		m_CurrentSpeed = FVector2D( 0.f, 0.f );
		return;
	}

	FVector2D ToTargetDirection( 1.f, 0.f );
	if( Distance > 0.00000001f )
	{
		ToTargetDirection = ToTarget * ( 1.f / Distance );
	}
	FVector2D FrictionDelta = m_CurrentSpeed * ( -_fDeltaTime * m_Friction );
	if( m_DampingDistance > Distance )
	{
		FrictionDelta *= ( m_DampingDistance - Distance ) * 0.5f + 1.f;
	}
	m_CurrentSpeed += FrictionDelta;

	FLOAT Acceleration = m_MaxStrength;
	if( CurrentSpeedLength <= 0.0001f )
	{
		if( m_DampingDistance > Distance )
		{
			Acceleration = ( ( Distance / m_DampingDistance ) * 0.9f + 0.1f ) * m_MaxStrength;
		}
	}
	else if( ( m_MaxStrength / CurrentSpeedLength ) * Distance < m_MaxStrength )
	{
		Acceleration = ( m_MaxStrength / CurrentSpeedLength ) * Distance;
	}
	const FLOAT Impulse = appPow( fSpeedFactor + 0.5f, 4.f ) * _fDeltaTime;
	m_CurrentSpeed += ToTargetDirection * ( Impulse * Acceleration );
	m_CurrentSpeed.X = Clamp( m_CurrentSpeed.X, -m_MaxSpeed.X, m_MaxSpeed.X );
	m_CurrentSpeed.Y = Clamp( m_CurrentSpeed.Y, -m_MaxSpeed.Y, m_MaxSpeed.Y );

	const FVector2D OldValue = m_CurrentValue;
	m_CurrentValue += m_CurrentSpeed * _fDeltaTime;
	if( ( OldValue.X - TargetValue.X ) * ( m_CurrentValue.X - TargetValue.X ) <= 0.0001f
		|| ( Abs( m_CurrentValue.X - TargetValue.X ) < 0.0001f && Abs( m_CurrentValue.X ) < 0.0001f ) )
	{
		m_CurrentSpeed.X = 0.f;
		m_CurrentValue.X = TargetValue.X;
	}
	if( ( OldValue.Y - TargetValue.Y ) * ( m_CurrentValue.Y - TargetValue.Y ) <= 0.0001f
		|| ( Abs( m_CurrentValue.Y - TargetValue.Y ) < 0.0001f && Abs( m_CurrentValue.Y ) < 0.0001f ) )
	{
		m_CurrentSpeed.Y = 0.f;
		m_CurrentValue.Y = TargetValue.Y;
	}
	m_CurrentValue.X = Clamp( m_CurrentValue.X, m_MinValue.X, m_MaxValue.X );
	m_CurrentValue.Y = Clamp( m_CurrentValue.Y, m_MinValue.Y, m_MaxValue.Y );
}

/*-----------------------------------------------------------------------------
	Lifetime
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x555d60 (2012 0x5968f0), 1,147 bytes: every member, and the six springs' tuning.
FArkComponentLookat::FArkComponentLookat()
	: m_pConfig( NULL )
	, m_pPawnOwner( NULL )
	, m_pLookAtNode( NULL )
	, m_pSpineBenderNode( NULL )
	, m_pSkeletalComponent( NULL )
	, m_HeadBoneIndex( INDEX_NONE )
	, m_LeftEyeParentBoneIndex( INDEX_NONE )
	, m_RightEyeParentBoneIndex( INDEX_NONE )
	, m_LeftEyeBoneIndex( INDEX_NONE )
	, m_RightEyeBoneIndex( INDEX_NONE )
	, m_pLeftEyeControl( NULL )
	, m_pRightEyeControl( NULL )
	, m_pBlinkControl( NULL )
	, m_pBlinkAnim( NULL )
	, m_pFaceToCpnt( NULL )
	, m_LookAtMode( 0 )
	, m_DebugLookAtMode( INDEX_NONE )
	, m_bTorsoDisabled( FALSE )
	, m_bHeadDisabled( FALSE )
	, m_bEyesDisabled( FALSE )
	, m_bInLOD( FALSE )
	, m_bNoTick( FALSE )
	, m_TargetType( CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION )
	, m_pCurrentTarget( NULL )
	, m_VectorTarget( 0.f, 0.f, 0.f )
	, m_bIsTorsoInfMoveSpeedDep( FALSE )
	, m_bLocal( FALSE )
	, m_ProcPatternIdx( INDEX_NONE )
	, m_ProcSequenceIdx( INDEX_NONE )
	, m_ProcRuleIdx( INDEX_NONE )
	, m_bFaceFxIsPlaying( FALSE )
	, m_bIsNotLookingAnywhere( FALSE )
	, m_ToTarget( 0.f, 0.f, 0.f )
	, m_HeadUp( 0.f, 0.f, 0.f )
	, m_HeadRight( 0.f, 0.f, 0.f )
	, m_HeadAt( 0.f, 0.f, 0.f )
	, m_NpcUp( 0.f, 0.f, 0.f )
	, m_NpcRight( 0.f, 0.f, 0.f )
	, m_NpcAt( 0.f, 0.f, 0.f )
	, m_EyeOffsets( 0.f, 0.f, 0.f )
	, m_CachedHeadPosition( 0.f, 0.f, 0.f )
	, m_DebugCurrentHeadPos( 0.f, 0.f, 0.f )
	, m_DebugCurrentHeadTarget( 0.f, 0.f, 0.f )
	, m_fBeforeNextBlinkDuration( -1.f )
	, m_UnclampedTargetXPosition( 0.f )
	, m_PreviousDelta( 0.f, 0.f )
	, m_PreviousTarget( 0.f, 0.f )
	, m_fDelay( 0.f )
{
	GDisLookatComponents++;

	m_CurrentCursorPosition.m_CurrentValue = FVector2D( 0.f, 0.f );
	m_CurrentCursorPosition.m_MinValue = FVector2D( -PI, -PI * 0.5f );
	m_CurrentCursorPosition.m_MaxValue = FVector2D( PI, PI * 0.5f );
	m_CurrentCursorPosition.m_CurrentSpeed = FVector2D( 0.f, 0.f );
	m_CurrentCursorPosition.m_MaxSpeed = FVector2D( TURN_MAX_STRENGTH, TURN_MAX_STRENGTH );
	m_CurrentCursorPosition.m_TargetValue = FVector2D( 0.f, 0.f );
	m_CurrentCursorPosition.m_MaxStrength = TURN_MAX_STRENGTH;
	m_CurrentCursorPosition.m_Friction = TURN_FRICTION;
	m_CurrentCursorPosition.m_DampingDistance = PI * 0.5f;
	m_CurrentCursorPosition.m_fSpeedFactor = 0.5f;

	m_CurrentHeadRange.m_CurrentValue = 0.f;
	m_CurrentHeadRange.m_MinValue = 0.f;
	m_CurrentHeadRange.m_MaxValue = 1.f;
	m_CurrentHeadRange.m_CurrentSpeed = 0.f;
	m_CurrentHeadRange.m_MaxSpeed = HEAD_AND_TORSO_RANGE_BLENDTIME_STRENGTH;
	m_CurrentHeadRange.m_TargetValue = 0.f;
	m_CurrentHeadRange.m_MaxStrength = HEAD_AND_TORSO_RANGE_BLENDTIME_STRENGTH;
	m_CurrentHeadRange.m_Friction = 3.f;
	m_CurrentHeadRange.m_DampingDistance = 0.5f;
	m_CurrentHeadRange.m_fSpeedFactor = 0.5f;

	m_CurrentTorsoRange = m_CurrentHeadRange;

	m_CurrentEyesPosition.m_CurrentValue = FVector2D( 0.f, 0.f );
	m_CurrentEyesPosition.m_MinValue = FVector2D( MIN_EYE_YAW, MIN_EYE_PITCH );
	m_CurrentEyesPosition.m_MaxValue = FVector2D( MAX_EYE_YAW, MAX_EYE_PITCH );
	m_CurrentEyesPosition.m_CurrentSpeed = FVector2D( 0.f, 0.f );
	m_CurrentEyesPosition.m_MaxSpeed = FVector2D( EYES_MAX_STRENGTH, EYES_MAX_STRENGTH );
	m_CurrentEyesPosition.m_TargetValue = FVector2D( 0.f, 0.f );
	m_CurrentEyesPosition.m_MaxStrength = EYES_MAX_STRENGTH;
	m_CurrentEyesPosition.m_Friction = 1.f;
	m_CurrentEyesPosition.m_DampingDistance = 1.f;
	m_CurrentEyesPosition.m_fSpeedFactor = 0.5f;

	m_EyesControlStrength = m_CurrentHeadRange;
	m_EyesControlStrength.m_Friction = 10.f;

	m_CurrentUseTorsoDeadZone = m_CurrentHeadRange;
	m_CurrentUseTorsoDeadZone.m_CurrentValue = 1.f;
	m_CurrentUseTorsoDeadZone.m_TargetValue = 1.f;
}

// DISHONORED(port): 2013 rva 0x554a60 (2012 0x595520), 27 bytes.
FArkComponentLookat* FArkComponentLookat::GetLookatComponent( const APawn* _pPawnOwner )
{
	if( !_pPawnOwner || !_pPawnOwner->m_ComponentContainer )
	{
		return NULL;
	}
	return _pPawnOwner->m_ComponentContainer->GetFirstComponent<FArkComponentLookat>( TRUE );
}

/**
 * DISHONORED(port): 2013 rva 0x556660 (2012 0x596ee0), 915 bytes. Retail's order: register with the component
 * manager's PreAsyncWork policy; the owner pawn and its first skeletal mesh component without a parent anim component;
 * the camera (or head) bone and the two eye bones with their parents; the two eye skel controls, the blink blender and
 * the blink sequence (child 1 of the blender at full weight, the sequence rewound and stopped); the look-at node, the
 * spine bender control named by the NPC tweaks, the node's config; the face-to component; the request queue; and the
 * LOD-changed event.
 * DISHONORED(bringup): FArkComponentManager is not ported - the tick arrives through the Engine seam in
 * USkeletalMeshComponent::Tick instead (arkcomponentbase.h). FArkComponentFaceTo is not ported, so m_pFaceToCpnt stays
 * NULL and UpdateHeadAndTorso's face-to compensation is zero, which is also what retail computes for a pawn whose
 * face-to component has no angle left to turn.
 */
void FArkComponentLookat::Starting()
{
	GDisLookatStarted++;
	m_pPawnOwner = Cast<APawn>( m_pOwner );
	m_pSkeletalComponent = NULL;
	if( !m_pPawnOwner )
	{
		return;
	}
	for( INT ComponentIndex = 0; ComponentIndex < m_pPawnOwner->AllComponents.Num(); ComponentIndex++ )
	{
		USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( m_pPawnOwner->AllComponents(ComponentIndex) );
		if( SkelComp && !SkelComp->ParentAnimComponent )
		{
			m_pSkeletalComponent = SkelComp;
			break;
		}
	}
	// DISHONORED(divergence): retail dereferences m_pSkeletalComponent unchecked from here on. A pawn with no skeletal
	// mesh yet (an archetype, a pawn whose mesh is set by a later pass) would crash; it is left half-started instead
	// and PreAsyncWorkTick's own m_pSkeletalComponent test keeps it inert.
	if( !m_pSkeletalComponent || !m_pSkeletalComponent->SkeletalMesh )
	{
		return;
	}
	const TArray<FMeshBone>& RefSkeleton = m_pSkeletalComponent->SkeletalMesh->RefSkeleton;
	m_HeadBoneIndex = m_pSkeletalComponent->MatchRefBone( s_CameraBoneName );
	if( m_HeadBoneIndex == INDEX_NONE )
	{
		m_HeadBoneIndex = m_pSkeletalComponent->MatchRefBone( s_HeadBoneName );
	}
	m_LeftEyeBoneIndex = m_pSkeletalComponent->MatchRefBone( s_LeftEyeBoneName );
	if( m_LeftEyeBoneIndex >= 0 )
	{
		m_LeftEyeParentBoneIndex = RefSkeleton(m_LeftEyeBoneIndex).ParentIndex;
	}
	m_RightEyeBoneIndex = m_pSkeletalComponent->MatchRefBone( s_RightEyeBoneName );
	if( m_RightEyeBoneIndex >= 0 )
	{
		m_RightEyeParentBoneIndex = RefSkeleton(m_RightEyeBoneIndex).ParentIndex;
	}

	USkeletalMeshComponent* Mesh = m_pPawnOwner->Mesh;
	UAnimTree* AnimTree = Mesh ? Cast<UAnimTree>( Mesh->Animations ) : NULL;
	if( AnimTree )
	{
		m_pLeftEyeControl = Cast<USkelControlSingleBone>( AnimTree->FindSkelControl( s_LeftEyeControlName ) );
		m_pRightEyeControl = Cast<USkelControlSingleBone>( AnimTree->FindSkelControl( s_RightEyeControlName ) );
		m_pBlinkControl = Cast<UAnimNodeBlendBase>( AnimTree->FindAnimNode( s_BlinkControlName ) );
		m_pBlinkAnim = Cast<UAnimNodeSequence>( AnimTree->FindAnimNode( s_BlinkAnimName ) );
	}
	if( m_pBlinkControl && m_pBlinkAnim )
	{
		if( m_pBlinkControl->Children.IsValidIndex(1) )
		{
			m_pBlinkControl->Children(1).Weight = 1.f;
		}
		m_pBlinkAnim->CurrentTime = 0.f;
		m_pBlinkAnim->bPlaying = FALSE;
		m_pBlinkAnim->bLooping = FALSE;
	}

	m_pLookAtNode = UArkAnimNodeLookAt::GetAnimNodeLookAt( m_pPawnOwner );
	if( AnimTree )
	{
		// DISHONORED(port): APawn::GetSpineBenderNodeName (2013 rva 0x753600 on ADishonoredNPCPawn) answers the NPC body
		// tweaks' m_SpineBenderName; the NPC pawn resolves the same control into m_pSpineBender in PostBeginPlay_Body
		// (dishonorednpcpawn_body.cpp), which is where the name is read from here.
		FName SpineBenderName = NAME_None;
		if( ADishonoredNPCPawn* NPCPawn = Cast<ADishonoredNPCPawn>( m_pPawnOwner ) )
		{
			UDisTweaks_NPCPawn* NPCTweaks = Cast<UDisTweaks_NPCPawn>( NPCPawn->GetTweaks_Derived() );
			UDisTweaks_NPCPawn_Body* BodyTweaks = NPCTweaks ? Cast<UDisTweaks_NPCPawn_Body>( NPCTweaks->m_pBodyTweaks ) : NULL;
			if( BodyTweaks )
			{
				SpineBenderName = BodyTweaks->m_SpineBenderName;
			}
		}
		m_pSpineBenderNode = SpineBenderName != NAME_None ? Cast<USkelControlSingleBone>( AnimTree->FindSkelControl( SpineBenderName ) ) : NULL;
	}
	if( m_pLookAtNode )
	{
		m_pLookAtNode->SetConfig( m_pConfig );
	}
	m_pFaceToCpnt = NULL;

	m_ReqMgr.Initialize( this, NULL );

	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		const INT EventType = ArkGameEventType_LODChanged;
		Dispatcher->RegisterToObjectEvent<FArkComponentLookat>( EventType, m_pPawnOwner, this, &FArkComponentLookat::OnLODChanged );
	}
}

// DISHONORED(port): 2013 rva 0x556a00 (2012 0x597260), 141 bytes.
void FArkComponentLookat::Stopping()
{
	if( FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance() )
	{
		const INT EventType = ArkGameEventType_LODChanged;
		Dispatcher->UnregisterToObjectEvent<FArkComponentLookat>( EventType, m_pPawnOwner, this, &FArkComponentLookat::OnLODChanged );
	}
	m_pPawnOwner = NULL;
	m_ReqMgr.Reset();
}

// DISHONORED(port): 2013 rva 0x552620 (2012 0x593200), 28 bytes: the request queue's allocation.
DWORD FArkComponentLookat::GetAllocatedSize() const
{
	return m_ReqMgr.GetRequestsCount() * sizeof( FArkRequestManager<FLookatRequestData>::FRequest ) + m_ProcWorkData.GetAllocatedSize();
}

// DISHONORED(port): 2013 rva 0x555760 (2012 0x596320), 182 bytes: the actor each actor-targeted request holds is handed
// to the collector, and a request whose actor went away is dropped; then the live target.
void FArkComponentLookat::ManageReferences( FGCHelper* _pHelper )
{
	if( !_pHelper )
	{
		return;
	}
	const INT Count = m_ReqMgr.IsInitialized() ? m_ReqMgr.GetRequestsCount() : 0;
	for( INT RequestIdx = Count - 1; RequestIdx >= 0; RequestIdx-- )
	{
		FLookatRequestData& Data = m_ReqMgr.GetRequest( RequestIdx ).m_Data;
		if( Data.m_Type == CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR || Data.m_Type == CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR )
		{
			AActor* Actor = (AActor*)Data.m_ActorData.m_pActor;
			if( Actor != _pHelper->manageReference( Actor ) )
			{
				m_ReqMgr.RemoveRequestByIdx( RequestIdx );
			}
		}
	}
	AActor* Target = (AActor*)m_pCurrentTarget;
	AActor* Managed = _pHelper->manageReference( Target );
	if( Target != Managed )
	{
		m_pCurrentTarget = Managed;
	}
}

// DISHONORED(port): 2013 rva 0x54ee10 (2012 0x58fab0), 58 bytes. Bit 12 of the LOD event's first parameter word is
// "this LOD still runs the look-at"; both m_bNoTick and m_bInLOD are its complement, and a component that stops ticking
// forgets that FaceFX was playing.
void FArkComponentLookat::OnLODChanged( const FArkGameEvent& _rGameEvent )
{
	const INT* Params = (const INT*)_rGameEvent.m_pEventParams;
	const UBOOL bLookatEnabledAtThisLOD = Params ? ( ( *Params >> 12 ) & 1 ) : TRUE;
	m_bNoTick = !bLookatEnabledAtThisLOD;
	if( m_bNoTick )
	{
		m_bFaceFxIsPlaying = FALSE;
	}
	m_bInLOD = !bLookatEnabledAtThisLOD;
}

/*-----------------------------------------------------------------------------
	The request entry points
-----------------------------------------------------------------------------*/

static void DisLookatZeroRequest( FArkComponentLookat::FLookatRequestData& _rRequest )
{
	appMemzero( &_rRequest, sizeof( _rRequest ) );
	_rRequest.m_PatternIdx = INDEX_NONE;
}

// DISHONORED(port): 2013 rva 0x554a80 (2012 0x595540), 168 bytes: an actor request with no actor, head influence only.
INT FArkComponentLookat::StartLookForward( const void* const _pAsker, const FName& _AskerName, INT _Priority, FLOAT _fLookAtSpeed, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR;
	Request.m_fSpeed = Clamp( _fLookAtSpeed, 0.f, 1.f );
	Request.m_LookAtInfluence = FDisLookAtInfluence::Head;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554b30 (2012 0x5955f0), 174 bytes.
INT FArkComponentLookat::StartLookAtActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, const FName& _AttachName, FLOAT _fLookAtSpeed, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR;
	Request.m_fSpeed = Clamp( _fLookAtSpeed, 0.f, 1.f );
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_ActorData.m_pActor = _pActor;
	Request.m_ActorData.m_AttachName = _AttachName;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554be0 (2012 0x5956a0), 156 bytes.
INT FArkComponentLookat::StartLookAtLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, FVector _Location, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fLookAtSpeed, UBOOL _bLocal, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION;
	Request.m_fSpeed = Clamp( _fLookAtSpeed, 0.f, 1.f );
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_LocationData.m_Location = _Location;
	Request.m_LocationData.m_bLocal = _bLocal;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554c80 (2012 0x595740), 141 bytes: an aim always moves at speed 0.5.
INT FArkComponentLookat::StartAimAtActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR;
	Request.m_fSpeed = 0.5f;
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_ActorData.m_pActor = _pActor;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554d10 (2012 0x5957d0), 130 bytes.
INT FArkComponentLookat::StartAimAtLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, FVector _Location, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_AIM_POSITION;
	Request.m_fSpeed = 0.5f;
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_LocationData.m_Location = _Location;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554da0 (2012 0x595860), 156 bytes. A pattern index the config does not have becomes
// INDEX_NONE, which UpdateVariablesFromRequest reads as "look forward".
INT FArkComponentLookat::StartProceduralLookAt( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL;
	Request.m_fSpeed = 0.5f;
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_PatternIdx = ( m_pConfig && _PatternIdx < m_pConfig->m_ProcPatterns.Num() ) ? _PatternIdx : INDEX_NONE;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554e40 (2012 0x595900), 167 bytes.
INT FArkComponentLookat::StartProceduralLookAtOnActor( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const AActor* const _pActor, const FDisLookAtInfluence& _LookAtInfluence, const FName& _AttachName, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_ACTOR;
	Request.m_fSpeed = 0.5f;
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_ActorData.m_pActor = _pActor;
	Request.m_ActorData.m_AttachName = _AttachName;
	Request.m_PatternIdx = ( m_pConfig && _PatternIdx < m_pConfig->m_ProcPatterns.Num() ) ? _PatternIdx : INDEX_NONE;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x554ef0 (2012 0x5959b0), 150 bytes.
INT FArkComponentLookat::StartProceduralLookAtOnLocation( const void* const _pAsker, const FName& _AskerName, INT _Priority, INT _PatternIdx, const FVector& _rLocation, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fDuration )
{
	FLookatRequestData Request;
	DisLookatZeroRequest( Request );
	Request.m_Type = CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_POSITION;
	Request.m_fSpeed = 0.5f;
	Request.m_LookAtInfluence = _LookAtInfluence;
	Request.m_LocationData.m_Location = _rLocation;
	Request.m_PatternIdx = ( m_pConfig && _PatternIdx < m_pConfig->m_ProcPatterns.Num() ) ? _PatternIdx : INDEX_NONE;
	GDisLookatRequestsStarted++;
	return m_ReqMgr.AddRequest( _pAsker, _AskerName, _Priority, Request, _fDuration );
}

// DISHONORED(port): 2013 rva 0x555710 (2012 0x5962d0), 50 bytes.
UBOOL FArkComponentLookat::StopLookAt( INT _RequestID )
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return FALSE;
	}
	GDisLookatRequestsStopped++;
	m_ReqMgr.RemoveRequestByIdx( RequestIdx );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x555750 (2012 0x596310), 12 bytes.
UBOOL FArkComponentLookat::StopAllLookAtFromAsker( const void* const _pAsker )
{
	return m_ReqMgr.RemoveAllRequestsFromAsker( _pAsker );
}

// DISHONORED(port): 2013 rva 0x554f90 (2012 0x595a50), 60 bytes.
const FArkComponentLookat::FLookatRequestData* FArkComponentLookat::GetActiveRequest() const
{
	if( !m_ReqMgr.HasRequest() )
	{
		return NULL;
	}
	return &m_ReqMgr.GetFirstRequest().m_Data;
}

// DISHONORED(port): 2013 rva 0x554fd0 (2012 0x595a90), 64 bytes.
INT FArkComponentLookat::GetActiveRequestPriority() const
{
	if( !m_ReqMgr.HasRequest() )
	{
		return INDEX_NONE;
	}
	return m_ReqMgr.GetFirstRequest().m_Priority;
}

// DISHONORED(port): 2013 rva 0x555010 (2012 0x595ad0), 179 bytes: 0 for a request that is gone or untimed.
FLOAT FArkComponentLookat::GetTimeLeft( INT _RequestID ) const
{
	const INT RequestIdx = m_ReqMgr.GetRequestIdxByID( _RequestID );
	if( RequestIdx == INDEX_NONE )
	{
		return 0.f;
	}
	const FLOAT fTimeLeft = m_ReqMgr.GetRequest( RequestIdx ).m_fTimeLeft;
	return fTimeLeft >= 0.f ? fTimeLeft : 0.f;
}

/*-----------------------------------------------------------------------------
	The mode
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54e920 (2012 0x58f570), 57 bytes. ADishonoredNPCPawn::Tick calls this every frame with
// ChooseLookAt(); a debug mode (SetDebugLookAtMode) pins it.
void FArkComponentLookat::SetLookAtMode( INT _LookAtMode )
{
	if( !m_pConfig || m_DebugLookAtMode != INDEX_NONE )
	{
		return;
	}
	m_LookAtMode = Clamp( _LookAtMode, 0, m_pConfig->m_LookAtModes.Num() - 1 );
	if( m_pLookAtNode )
	{
		m_pLookAtNode->SetLookAtMode( m_LookAtMode );
	}
}

// DISHONORED(port): 2012 rva 0x58f5b0, 72 bytes; the 2013 database does not match it (UDishonoredCheatManager's
// LocoForceLookAtMode is its caller).
void FArkComponentLookat::SetDebugLookAtMode( INT _DebugLookAtMode )
{
	if( _DebugLookAtMode != INDEX_NONE )
	{
		m_DebugLookAtMode = INDEX_NONE;
		if( m_pConfig )
		{
			m_LookAtMode = Clamp( _DebugLookAtMode, 0, m_pConfig->m_LookAtModes.Num() - 1 );
			if( m_pLookAtNode )
			{
				m_pLookAtNode->SetLookAtMode( m_LookAtMode );
			}
		}
	}
	m_DebugLookAtMode = _DebugLookAtMode;
}

/*-----------------------------------------------------------------------------
	Geometry
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54d380 (2012 0x54d380 is the 2013 start; 2012 0x58df50), 961 bytes. The head (camera)
// bone's world atom: its position, and its +X as up, +Y as right and -Z as the direction it looks along.
FVector FArkComponentLookat::GetLookEyesPosition( FVector* _pHeadUp, FVector* _pHeadRight, FVector* _pHeadAt )
{
	if( !m_pSkeletalComponent || m_HeadBoneIndex < 0 || !m_pSkeletalComponent->SpaceBases.IsValidIndex( m_HeadBoneIndex ) )
	{
		// DISHONORED(divergence): retail indexes the head bone unchecked; a pawn whose camera bone is missing keeps the
		// pawn's own frame instead of a garbage atom.
		if( _pHeadUp ) { *_pHeadUp = m_NpcUp; }
		if( _pHeadRight ) { *_pHeadRight = m_NpcRight; }
		if( _pHeadAt ) { *_pHeadAt = m_NpcAt; }
		return m_pPawnOwner ? m_pPawnOwner->Location : FVector( 0.f, 0.f, 0.f );
	}
	const FBoneAtom HeadAtom = m_pSkeletalComponent->GetBoneAtom( m_HeadBoneIndex );
	const FQuat HeadRotation = HeadAtom.GetRotation();
	const FLOAT Scale = HeadAtom.GetScale();
	if( _pHeadUp )
	{
		*_pHeadUp = HeadRotation.RotateVector( FVector( Scale, 0.f, 0.f ) );
	}
	if( _pHeadRight )
	{
		*_pHeadRight = HeadRotation.RotateVector( FVector( 0.f, Scale, 0.f ) );
	}
	if( _pHeadAt )
	{
		*_pHeadAt = -HeadRotation.RotateVector( FVector( 0.f, 0.f, Scale ) );
	}
	return HeadAtom.GetTranslation();
}

// DISHONORED(port): 2013 rva 0x552bd0 (2012 0x5937b0), 751 bytes. With an attach name: the socket (or bone) of the
// actor's first own skeletal mesh component, in world space. Without: a pawn's view location (the local player's own
// camera for the player pawn, read through its controller), a controller's pawn's view location, or the actor's origin.
FVector FArkComponentLookat::GetLookAtPosition( const AActor* _pActor, const FName& _AttachName )
{
	if( !_pActor )
	{
		return FVector( 0.f, 0.f, 0.f );
	}
	if( _AttachName == NAME_None )
	{
		const APawn* TargetPawn = _pActor->GetAPawn();
		if( !TargetPawn )
		{
			if( const AController* TargetController = Cast<AController>( const_cast<AActor*>( _pActor ) ) )
			{
				TargetPawn = TargetController->Pawn;
			}
		}
		APlayerController* LocalPC = ((AActor*)_pActor)->GetALocalPlayerController();
		if( LocalPC && _pActor == LocalPC->Pawn && TargetPawn )
		{
			FVector ViewLocation( 0.f, 0.f, 0.f );
			FRotator ViewRotation( 0, 0, 0 );
			LocalPC->GetPlayerViewPoint( ViewLocation, ViewRotation );
			return ViewLocation;
		}
		if( TargetPawn )
		{
			return ((APawn*)TargetPawn)->GetPawnViewLocation();
		}
		return _pActor->LocalToWorld().GetOrigin();
	}

	USkeletalMeshComponent* TargetMesh = NULL;
	for( INT ComponentIndex = 0; ComponentIndex < _pActor->AllComponents.Num(); ComponentIndex++ )
	{
		USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( _pActor->AllComponents(ComponentIndex) );
		if( SkelComp && !SkelComp->ParentAnimComponent )
		{
			TargetMesh = SkelComp;
			break;
		}
	}
	if( TargetMesh )
	{
		FName BoneName = _AttachName;
		FVector RelativeLocation( 0.f, 0.f, 0.f );
		USkeletalMeshSocket* Socket = TargetMesh->SkeletalMesh ? TargetMesh->SkeletalMesh->FindSocket( _AttachName ) : NULL;
		if( Socket )
		{
			BoneName = Socket->BoneName;
			RelativeLocation = Socket->RelativeLocation;
		}
		const INT BoneIndex = TargetMesh->MatchRefBone( BoneName );
		if( BoneIndex != INDEX_NONE )
		{
			return TargetMesh->GetBoneMatrix( BoneIndex ).TransformFVector( RelativeLocation );
		}
	}
	return _pActor->LocalToWorld().GetOrigin();
}

// DISHONORED(port): 2013 rva 0x54d760 (2012 0x58e330), 329 bytes: the node's blended aim as a world direction off the
// pawn's rotation. m_CurrentBlendedAim is in radians; 10430.378 is 65536 / 2pi.
const FVector FArkComponentLookat::GetCurrentLookAtDirection() const
{
	FVector2D BlendedAim( 0.f, 0.f );
	if( m_pLookAtNode )
	{
		BlendedAim = m_pLookAtNode->GetCurrentBlendedAim();
	}
	if( !m_pPawnOwner )
	{
		return FVector( 1.f, 0.f, 0.f );
	}
	const FRotator LocalAimDirection( appTrunc( BlendedAim.Y * 10430.378f ), appTrunc( BlendedAim.X * 10430.378f ), 0 );
	const FRotationTranslationMatrix PawnRot( m_pPawnOwner->Rotation, FVector( 0.f, 0.f, 0.f ) );
	return PawnRot.TransformNormal( LocalAimDirection.Vector() );
}

// DISHONORED(port): 2013 rva 0x54d8b0 (2012 0x58e480), 310 bytes: the pawn's forward vector, through its quaternion.
FVector FArkComponentLookat::GetLookAtNeutralDirection()
{
	if( !m_pPawnOwner )
	{
		return FVector( 1.f, 0.f, 0.f );
	}
	return m_pPawnOwner->Rotation.Quaternion().RotateVector( FVector( 1.f, 0.f, 0.f ) );
}

/*-----------------------------------------------------------------------------
	The per-frame pass
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x555b40 (2012 0x596750), 162 bytes. The whole pass is behind "ticking, a node and a mesh";
// the queue ages first, FaceFX is sampled, and an LOD that disabled the look-at skips the four updates.
// DISHONORED(bringup): WITH_FACEFX is compiled out of this tree, so FaceFXActorInstance is a NULL void* and
// m_bFaceFxIsPlaying is always FALSE; retail reads the instance's state word at +48 (2 is "idle").
void FArkComponentLookat::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	if( m_bNoTick || !m_pLookAtNode || !m_pSkeletalComponent )
	{
		return;
	}
	m_ReqMgr.Update( _fTimeStep );
	m_bFaceFxIsPlaying = FALSE;
	if( m_bInLOD )
	{
		return;
	}
	UpdateBlink( _fTimeStep );
	UpdateVariablesFromRequest( _fTimeStep );
	UpdateEyes( _fTimeStep );
	UpdateHeadAndTorso( _fTimeStep );
}

// DISHONORED(port): 2013 rva 0x552ec0 (2012 0x593ac0), 388 bytes. The blink blender's second child is faded in at 10/s
// while blinking is allowed and out otherwise; the blink sequence is restarted at a random rate after a random pause.
void FArkComponentLookat::UpdateBlink( FLOAT _fTimeStep )
{
	UBOOL bBlinkAllowed = FALSE;
	if( m_pBlinkAnim && m_pConfig && m_pConfig->BlinkEnabled && ( !m_bFaceFxIsPlaying || m_pConfig->AllowBlinkingDuringFaceFx ) )
	{
		bBlinkAllowed = TRUE;
	}
	if( m_pBlinkControl && m_pBlinkControl->Children.IsValidIndex(1) )
	{
		FLOAT& Weight = m_pBlinkControl->Children(1).Weight;
		if( !bBlinkAllowed )
		{
			if( Weight > 0.f )
			{
				Weight = Max( Weight - _fTimeStep * 10.f, 0.f );
			}
			return;
		}
		if( Weight < 1.f )
		{
			Weight = Min( Weight + _fTimeStep * 10.f, 1.f );
		}
	}
	if( bBlinkAllowed )
	{
		m_fBeforeNextBlinkDuration -= _fTimeStep;
		if( m_fBeforeNextBlinkDuration < 0.f )
		{
			m_fBeforeNextBlinkDuration = m_pConfig->MinDurationBetweenBlink + ( m_pConfig->MaxDurationBetweenBlink - m_pConfig->MinDurationBetweenBlink ) * DisLookatRand01();
			m_pBlinkAnim->CurrentTime = 0.f;
			m_pBlinkAnim->bPlaying = TRUE;
			m_pBlinkAnim->Rate = m_pConfig->MinBlinkSpeed + ( m_pConfig->MaxBlinkSpeed - m_pConfig->MinBlinkSpeed ) * DisLookatRand01();
		}
	}
}

// DISHONORED(port): 2013 rva 0x54e8a0 (2012 0x58f4f0), 127 bytes: no target, no pattern, speed factor back to 0.5; the
// ranges keep their targets, which is how a look-at fades out instead of snapping.
void FArkComponentLookat::LookForward()
{
	m_TargetType = CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR;
	m_pCurrentTarget = NULL;
	m_AttachName = NAME_None;
	m_ProcPatternIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_ProcRuleIdx = INDEX_NONE;
	m_VectorTarget = FVector( 0.f, 0.f, 0.f );
	m_bIsTorsoInfMoveSpeedDep = FALSE;
	m_bLocal = FALSE;
	m_CurrentCursorPosition.m_fSpeedFactor = 0.5f;
	m_CurrentHeadRange.m_fSpeedFactor = 0.5f;
	m_CurrentTorsoRange.m_fSpeedFactor = 0.5f;
}

// DISHONORED(port): 2013 rva 0x54e5e0 (2012 0x58f230), 174 bytes.
void FArkComponentLookat::LookAt( const AActor* _pActor, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fSpeedFactor, const FName& _AttachName )
{
	m_pCurrentTarget = _pActor;
	m_TargetType = CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR;
	m_AttachName = _AttachName;
	m_ProcPatternIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_ProcRuleIdx = INDEX_NONE;
	m_VectorTarget = FVector( 0.f, 0.f, 0.f );
	m_CurrentCursorPosition.m_fSpeedFactor = _fSpeedFactor;
	m_CurrentHeadRange.m_TargetValue = _LookAtInfluence.m_fHeadInfluence;
	m_CurrentHeadRange.m_fSpeedFactor = _fSpeedFactor;
	m_CurrentTorsoRange.m_TargetValue = _LookAtInfluence.m_fTorsoInfluence;
	m_CurrentTorsoRange.m_fSpeedFactor = _fSpeedFactor;
	m_bIsTorsoInfMoveSpeedDep = _LookAtInfluence.m_bTorsoInfIsMoveSpeedDependant ? TRUE : FALSE;
	m_bLocal = FALSE;
}

// DISHONORED(port): 2013 rva 0x54e690 (2012 0x58f2e0), 167 bytes.
void FArkComponentLookat::LookAt( const FVector& _TargetPosition, const FDisLookAtInfluence& _LookAtInfluence, FLOAT _fSpeedFactor, UBOOL _bLocal )
{
	m_TargetType = CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION;
	m_pCurrentTarget = NULL;
	m_AttachName = NAME_None;
	m_ProcPatternIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_ProcRuleIdx = INDEX_NONE;
	m_VectorTarget = _TargetPosition;
	m_CurrentCursorPosition.m_fSpeedFactor = _fSpeedFactor;
	m_CurrentHeadRange.m_TargetValue = _LookAtInfluence.m_fHeadInfluence;
	m_CurrentHeadRange.m_fSpeedFactor = _fSpeedFactor;
	m_CurrentTorsoRange.m_TargetValue = _LookAtInfluence.m_fTorsoInfluence;
	m_CurrentTorsoRange.m_fSpeedFactor = _fSpeedFactor;
	m_bIsTorsoInfMoveSpeedDep = _LookAtInfluence.m_bTorsoInfIsMoveSpeedDependant ? TRUE : FALSE;
	m_bLocal = _bLocal ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x54e740 (2012 0x58f390), 175 bytes: an aim is all torso, no head, speed-dependent.
void FArkComponentLookat::AimAt( const AActor* _pActor )
{
	m_pCurrentTarget = _pActor;
	m_AttachName = NAME_None;
	m_ProcPatternIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_ProcRuleIdx = INDEX_NONE;
	m_TargetType = CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR;
	m_VectorTarget = FVector( 0.f, 0.f, 0.f );
	m_CurrentHeadRange.m_TargetValue = 0.f;
	m_CurrentCursorPosition.m_fSpeedFactor = 0.5f;
	m_CurrentHeadRange.m_fSpeedFactor = 0.5f;
	m_CurrentTorsoRange.m_TargetValue = 1.f;
	m_CurrentTorsoRange.m_fSpeedFactor = 0.5f;
	m_bLocal = FALSE;
	m_bIsTorsoInfMoveSpeedDep = TRUE;
}

// DISHONORED(port): 2013 rva 0x54e7f0 (2012 0x58f440), 166 bytes.
void FArkComponentLookat::AimAt( const FVector& _TargetPosition )
{
	m_AttachName = NAME_None;
	m_pCurrentTarget = NULL;
	m_ProcPatternIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_ProcRuleIdx = INDEX_NONE;
	m_TargetType = CPNT_LOOKAT_REQUEST_TYPE_AIM_POSITION;
	m_VectorTarget = _TargetPosition;
	m_CurrentHeadRange.m_TargetValue = 0.f;
	m_CurrentCursorPosition.m_fSpeedFactor = 0.5f;
	m_CurrentHeadRange.m_fSpeedFactor = 0.5f;
	m_CurrentTorsoRange.m_TargetValue = 1.f;
	m_CurrentTorsoRange.m_fSpeedFactor = 0.5f;
	m_bLocal = FALSE;
	m_bIsTorsoInfMoveSpeedDep = TRUE;
}

// DISHONORED(port): 2013 rva 0x552970 (2012 0x593550), 604 bytes. The config owns the pattern state machine
// (UArkComponentLookatConfig::InitProceduralLookAt / UpdateProceduralLookAt, arkanimnodelookat.cpp); this hands it the
// cursor in degrees and takes back the active sequence and rule, whose speed becomes the three springs' speed factor.
void FArkComponentLookat::ProceduralLookAt( FLOAT _fTimeStep, const FLookatRequestData& _rRequest )
{
	GDisLookatProceduralTicks++;
	if( m_ProcPatternIdx != _rRequest.m_PatternIdx )
	{
		m_pConfig->InitProceduralLookAt( _rRequest.m_PatternIdx, m_ProcWorkData );
	}
	m_TargetType = _rRequest.m_Type;
	m_pCurrentTarget = _rRequest.m_ActorData.m_pActor;
	m_AttachName = _rRequest.m_ActorData.m_AttachName;
	m_ProcPatternIdx = _rRequest.m_PatternIdx;
	m_ProcRuleIdx = INDEX_NONE;
	m_ProcSequenceIdx = INDEX_NONE;
	m_VectorTarget = _rRequest.m_LocationData.m_Location;

	const FVector2D CurAim( m_CurrentCursorPosition.m_CurrentValue.X * ( 180.f / PI ), m_CurrentCursorPosition.m_CurrentValue.Y * ( 180.f / PI ) );
	m_pConfig->UpdateProceduralLookAt( _fTimeStep, m_ProcPatternIdx, CurAim, m_ProcWorkData, m_ProcSequenceIdx, m_ProcRuleIdx );

	FLOAT fSpeed = 0.5f;
	if( m_ProcSequenceIdx != INDEX_NONE && m_ProcRuleIdx != INDEX_NONE
		&& m_ProcWorkData.IsValidIndex( m_ProcSequenceIdx ) && m_ProcWorkData(m_ProcSequenceIdx).m_Rules.IsValidIndex( m_ProcRuleIdx ) )
	{
		fSpeed = m_ProcWorkData(m_ProcSequenceIdx).m_Rules(m_ProcRuleIdx).m_fSpeed;
	}
	m_CurrentCursorPosition.m_fSpeedFactor = fSpeed;

	const FArkCpntLookatProcPattern& rPattern = m_pConfig->m_ProcPatterns( _rRequest.m_PatternIdx );
	m_CurrentHeadRange.m_TargetValue = Min( _rRequest.m_LookAtInfluence.m_fHeadInfluence, rPattern.m_fMaxHeadInfluence );
	m_CurrentHeadRange.m_fSpeedFactor = fSpeed;
	m_CurrentTorsoRange.m_TargetValue = Min( _rRequest.m_LookAtInfluence.m_fTorsoInfluence, rPattern.m_fMaxTorsoInfluence );
	m_CurrentTorsoRange.m_fSpeedFactor = fSpeed;
	m_bIsTorsoInfMoveSpeedDep = _rRequest.m_LookAtInfluence.m_bTorsoInfIsMoveSpeedDependant ? TRUE : FALSE;
	m_bLocal = TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x553fb0 (2012 0x594a70), 2,668 bytes. Three things, in order: the first request in the
 * queue (or none) becomes the component's target through LookAt / AimAt / ProceduralLookAt / LookForward; the head's
 * world frame is read off the camera bone (or, when the skeleton is not being updated this frame, synthesised from the
 * node's blended aim and the pawn's yaw); the pawn's own frame is the mesh's local-to-world, bent back by the spine
 * bender's current pitch so a bent spine does not read as a turned head; and the target becomes m_ToTarget, a unit
 * direction from the cached head position.
 */
void FArkComponentLookat::UpdateVariablesFromRequest( FLOAT _fTimeStep )
{
	UBOOL bHaveTarget = FALSE;
	if( m_ReqMgr.HasRequest() )
	{
		const FLookatRequestData& Data = m_ReqMgr.GetFirstRequest().m_Data;
		switch( Data.m_Type )
		{
		case CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION:
			LookAt( Data.m_LocationData.m_Location, Data.m_LookAtInfluence, Data.m_fSpeed, Data.m_LocationData.m_bLocal );
			bHaveTarget = TRUE;
			break;
		case CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR:
			LookAt( Data.m_ActorData.m_pActor, Data.m_LookAtInfluence, Data.m_fSpeed, Data.m_ActorData.m_AttachName );
			bHaveTarget = TRUE;
			break;
		case CPNT_LOOKAT_REQUEST_TYPE_AIM_POSITION:
			AimAt( Data.m_LocationData.m_Location );
			bHaveTarget = TRUE;
			break;
		case CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR:
			AimAt( Data.m_ActorData.m_pActor );
			bHaveTarget = TRUE;
			break;
		case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL:
		case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_POSITION:
		case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_ACTOR:
			if( Data.m_PatternIdx != INDEX_NONE && m_pConfig && m_pConfig->m_ProcPatterns.IsValidIndex( Data.m_PatternIdx ) )
			{
				ProceduralLookAt( _fTimeStep, Data );
				bHaveTarget = m_ProcSequenceIdx != INDEX_NONE && m_ProcRuleIdx != INDEX_NONE;
			}
			break;
		default:
			break;
		}
	}
	if( !bHaveTarget )
	{
		LookForward();
	}

	FVector HeadPos = GetLookEyesPosition( &m_HeadUp, &m_HeadRight, &m_HeadAt );
	USkeletalMeshComponent* Mesh = m_pPawnOwner->Mesh;
	if( Mesh && ( Mesh->m_bSkipUpdate || Mesh->bNoSkeletonUpdate ) )
	{
		FVector2D Aim[2];
		m_pLookAtNode->ComputeHeadAndTorsoAim( Aim );
		const FRotator HeadRot( appTrunc( ( Aim[1].Y + Aim[0].Y ) * 10430.378f ),
			m_pPawnOwner->Rotation.Yaw - appTrunc( ( Aim[1].X + Aim[0].X ) * -10430.378f ), 0 );
		m_HeadAt = HeadRot.Vector();
	}
	m_DebugCurrentHeadPos = HeadPos;
	if( ( m_CachedHeadPosition - HeadPos ).SizeSquared() > 0.0000099999997f )
	{
		m_CachedHeadPosition = HeadPos;
	}
	HeadPos = m_CachedHeadPosition;

	const FMatrix MeshLocalToWorld = Mesh
		? FRotationTranslationMatrix( Mesh->Rotation, Mesh->Translation ) * m_pPawnOwner->LocalToWorld()
		: m_pPawnOwner->LocalToWorld();
	m_NpcAt = MeshLocalToWorld.GetAxis( 0 );
	m_NpcRight = MeshLocalToWorld.GetAxis( 1 );
	m_NpcUp = MeshLocalToWorld.GetAxis( 2 );
	if( m_pSpineBenderNode )
	{
		const FLOAT BendAngle = -( (FLOAT)m_pSpineBenderNode->BoneRotation.Pitch * ( 2.f * PI / 65536.f ) * m_pSpineBenderNode->ControlStrength );
		const FQuat Unbend( m_NpcRight, BendAngle );
		m_NpcAt = Unbend.RotateVector( m_NpcAt );
		m_NpcUp = Unbend.RotateVector( m_NpcUp );
	}

	m_bIsNotLookingAnywhere = FALSE;
	switch( m_TargetType )
	{
	case CPNT_LOOKAT_REQUEST_TYPE_TARGET_POSITION:
	case CPNT_LOOKAT_REQUEST_TYPE_AIM_POSITION:
		m_ToTarget = m_bLocal ? m_VectorTarget : ( m_VectorTarget - HeadPos );
		break;
	case CPNT_LOOKAT_REQUEST_TYPE_TARGET_ACTOR:
	case CPNT_LOOKAT_REQUEST_TYPE_AIM_ACTOR:
		if( m_pCurrentTarget )
		{
			m_ToTarget = GetLookAtPosition( m_pCurrentTarget, m_AttachName ) - HeadPos;
		}
		else
		{
			m_ToTarget = m_NpcAt;
			m_bIsNotLookingAnywhere = TRUE;
		}
		break;
	case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL:
	case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_POSITION:
	case CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_ACTOR:
		{
			INT BaseYaw = m_pPawnOwner->Rotation.Yaw;
			INT BasePitch = 0;
			if( m_TargetType == CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_POSITION )
			{
				const FRotator ToPosition = ( m_VectorTarget - HeadPos ).Rotation();
				BaseYaw = ToPosition.Yaw;
				BasePitch = ToPosition.Pitch;
			}
			else if( m_TargetType == CPNT_LOOKAT_REQUEST_TYPE_PROCEDURAL_ON_ACTOR && m_pCurrentTarget )
			{
				const FRotator ToActor = ( GetLookAtPosition( m_pCurrentTarget, m_AttachName ) - HeadPos ).Rotation();
				BaseYaw = ToActor.Yaw;
				BasePitch = ToActor.Pitch;
			}
			if( m_ProcWorkData.IsValidIndex( m_ProcSequenceIdx ) && m_ProcWorkData(m_ProcSequenceIdx).m_Rules.IsValidIndex( m_ProcRuleIdx ) )
			{
				const FArkCpntLookatProcRuleWork& rRule = m_ProcWorkData(m_ProcSequenceIdx).m_Rules(m_ProcRuleIdx);
				// 182.04445 is 65536 / 360: the rule's target is degrees, the rotator is unreal units
				const FRotator Target( BasePitch - appTrunc( rRule.m_Target.m_fVertical * -182.04445f ),
					BaseYaw - appTrunc( rRule.m_Target.m_fHorizontal * -182.04445f ), 0 );
				m_ToTarget = Target.Vector();
			}
			else
			{
				m_ToTarget = m_NpcAt;
			}
		}
		break;
	default:
		break;
	}
	m_DebugCurrentHeadTarget = m_ToTarget + m_DebugCurrentHeadPos;
	m_ToTarget.Normalize( 0.0000000099999999f );
}

/**
 * DISHONORED(port): 2013 rva 0x54e960 (2012 0x58f600), 1,200 bytes. The eyes aim at m_ToTarget in the head's own frame
 * (yaw from m_HeadRight, pitch from m_HeadUp, both in degrees), with the config's small involuntary movements layered on
 * top: a random offset inside the amplitude every FixationMin..Max seconds, biased back toward the centre when the last
 * one was far out. The result is clamped to the eye limits, run through the eye spring and written to the two eye bone
 * controls with the eye strength spring; a disabled or unaimed look-at drives both back to zero.
 */
void FArkComponentLookat::UpdateEyes( FLOAT _fTimeStep )
{
	FLOAT StrengthTarget = 0.f;
	if( m_bIsNotLookingAnywhere || m_bEyesDisabled || !m_pConfig )
	{
		m_CurrentEyesPosition.m_TargetValue = FVector2D( 0.f, 0.f );
	}
	else
	{
		FVector Horizontal = m_ToTarget - m_HeadUp * ( m_HeadUp | m_ToTarget );
		Horizontal.Normalize( 0.0000000099999999f );
		const FLOAT AngleX = ( appAcos( Clamp( m_HeadRight | Horizontal, -1.f, 1.f ) ) - PI * 0.5f ) * -( 180.f / PI );
		const FLOAT AngleY = ( appAcos( Clamp( m_ToTarget | m_HeadUp, -1.f, 1.f ) ) - PI * 0.5f ) * -( 180.f / PI );

		FLOAT Yaw = AngleX;
		FLOAT Pitch = AngleY;
		if( m_pConfig->SmallMovementsEnabled && ( !m_bFaceFxIsPlaying || m_pConfig->AllowSmallEyesMovementsDuringFaceFx ) )
		{
			m_EyeOffsets.Z -= _fTimeStep;
			if( m_EyeOffsets.Z < 0.f )
			{
				const FLOAT FixationMin = m_pConfig->FixationMinDuration;
				const FLOAT FixationSpan = m_pConfig->FixationMaxDuration - m_pConfig->FixationMinDuration;
				const FLOAT AmplitudeYaw = m_pConfig->SmallMovementsHorizontalAmplitude;
				const FLOAT AmplitudePitch = m_pConfig->SmallMovementsVerticalAmplitude;
				const FLOAT OneOverHalfAmplitudeSq = 1.f / ( ( AmplitudePitch * 0.5f ) * ( AmplitudePitch * 0.5f ) + ( AmplitudeYaw * 0.5f ) * ( AmplitudeYaw * 0.5f ) );
				const FLOAT PreviousDistRatio = ( m_EyeOffsets.X * m_EyeOffsets.X + m_EyeOffsets.Y * m_EyeOffsets.Y ) * OneOverHalfAmplitudeSq;
				m_EyeOffsets.X = ( appFrand() - 0.5f ) * AmplitudeYaw;
				m_EyeOffsets.Y = ( appFrand() - 0.5f ) * AmplitudePitch;
				FLOAT DistRatio = ( m_EyeOffsets.X * m_EyeOffsets.X + m_EyeOffsets.Y * m_EyeOffsets.Y ) * OneOverHalfAmplitudeSq;
				if( PreviousDistRatio > 0.5f )
				{
					const FLOAT PullBack = ( 0.5f / PreviousDistRatio ) - 0.5f;
					m_EyeOffsets.X *= PullBack;
					m_EyeOffsets.Y *= PullBack;
					m_EyeOffsets.Z *= PullBack;
					DistRatio = 0.f;
				}
				// retail: FixationMin + ((FixationMax - FixationMin) - FixationMin) / (DistRatio + 1) * appFrand()
				m_EyeOffsets.Z = FixationMin + ( FixationSpan - FixationMin ) / ( DistRatio + 1.f ) * appFrand();
			}
			Yaw += m_EyeOffsets.X;
			Pitch += m_EyeOffsets.Y;
		}
		Yaw = Clamp( Yaw, MIN_EYE_YAW, MAX_EYE_YAW );
		Pitch = Clamp( Pitch, MIN_EYE_PITCH, MAX_EYE_PITCH );
		m_CurrentEyesPosition.m_TargetValue = FVector2D( Yaw, Pitch );
		StrengthTarget = 1.f;
	}
	m_EyesControlStrength.m_TargetValue = StrengthTarget;
	m_CurrentEyesPosition.Update( _fTimeStep, NULL, NULL );
	m_EyesControlStrength.Update( _fTimeStep, NULL, NULL );

	const FRotator EyeRotation = FRotator::MakeFromEuler( FVector( 0.f, m_CurrentEyesPosition.m_CurrentValue.Y, m_CurrentEyesPosition.m_CurrentValue.X ) );
	if( m_pLeftEyeControl )
	{
		m_pLeftEyeControl->ControlStrength = m_EyesControlStrength.m_CurrentValue;
		m_pLeftEyeControl->BoneRotation = EyeRotation;
	}
	if( m_pRightEyeControl )
	{
		m_pRightEyeControl->ControlStrength = m_EyesControlStrength.m_CurrentValue;
		m_pRightEyeControl->BoneRotation = EyeRotation;
	}
}

/**
 * DISHONORED(port): 2013 rva 0x553050 (2012 0x593c50), 2,341 bytes. The one function that decides how far a head
 * turns, in four steps:
 *   1. the head and torso range springs chase the request's influences (a disabled part is driven to 0 at speed 0.6;
 *      a move-speed-dependent torso scales its target by 1 - speed / m_fTorsoInfluenceMaxMoveSpeed) and are handed to
 *      the node as UseHeadRange / UseTorsoRange. Both at zero is the rest state: SetAim(0,0) and out.
 *   2. m_ToTarget becomes a yaw (round the pawn's up axis, measured from its right, so forward is 0 and behind is
 *      +/-pi) and a pitch, in radians; the yaw is relative to whatever the face-to component still has to turn.
 *   3. a target that jumped more than 3pi/2 since last frame holds the cursor for (1 - speed factor) seconds; one
 *      more than pi away keeps counting that delay down; the torso dead zone opens when the target is within pi/2.
 *   4. when the delay is spent: the mode's horizontal/vertical percentages, the summed range and the mode's optional
 *      clamp scale the angle; the node's GetMaxAim clips it to what the two grids can reach for that direction; a
 *      target that crossed behind the pawn (both X beyond 162 degrees, opposite signs) keeps the old side instead of
 *      swinging through the front. The cursor spring then chases it and the node is given the result in degrees.
 */
void FArkComponentLookat::UpdateHeadAndTorso( FLOAT _fTimeStep )
{
	const FLOAT DisabledSpeedFactor = 0.6f;
	const FLOAT DisabledTarget = 0.f;
	if( m_bHeadDisabled || m_bEyesDisabled )
	{
		m_CurrentHeadRange.Update( _fTimeStep, &DisabledSpeedFactor, &DisabledTarget );
	}
	else
	{
		m_CurrentHeadRange.Update( _fTimeStep, NULL, NULL );
	}
	if( m_bTorsoDisabled || m_bHeadDisabled || m_bEyesDisabled )
	{
		m_CurrentTorsoRange.Update( _fTimeStep, &DisabledSpeedFactor, &DisabledTarget );
	}
	else
	{
		FLOAT SpeedMultiplier = 1.f;
		if( m_bIsTorsoInfMoveSpeedDep && !m_pPawnOwner->Velocity.IsZero() )
		{
			const FLOAT Speed = m_pPawnOwner->Velocity.Size();
			SpeedMultiplier = Speed < m_pConfig->m_fTorsoInfluenceMaxMoveSpeed ? 1.f - Speed / m_pConfig->m_fTorsoInfluenceMaxMoveSpeed : 0.f;
		}
		const FLOAT TorsoTarget = m_CurrentTorsoRange.m_TargetValue * SpeedMultiplier;
		m_CurrentTorsoRange.Update( _fTimeStep, NULL, &TorsoTarget );
	}
	m_pLookAtNode->SetUseHeadRange( m_CurrentHeadRange.m_CurrentValue );
	m_pLookAtNode->SetUseTorsoRange( m_CurrentTorsoRange.m_CurrentValue );

	if( m_pLookAtNode->GetUseHeadRange() <= 0.f && m_pLookAtNode->GetUseTorsoRange() <= 0.f )
	{
		m_pLookAtNode->SetAim( FVector2D( 0.f, 0.f ) );
		m_CurrentUseTorsoDeadZone.m_TargetValue = 1.f;
	}
	else
	{
		// DISHONORED(bringup): FArkComponentFaceTo::GetAngle (the yaw the face-to still has to turn, in unreal units)
		// is not ported; the component is NULL and retail's own NULL branch answers 0.
		const FLOAT FaceToAngle = 0.f;

		FVector Horizontal = m_ToTarget - m_NpcUp * ( m_NpcUp | m_ToTarget );
		Horizontal.Normalize( 0.0000000099999999f );
		const FLOAT DotRight = Clamp( m_NpcRight | Horizontal, -1.f, 1.f );
		FLOAT X;
		if( ( m_NpcAt | m_ToTarget ) <= 0.f )
		{
			const FLOAT A = appAcos( DotRight );
			X = A > PI * 0.5f ? A - 3.f * PI * 0.5f : A + PI * 0.5f;
		}
		else
		{
			X = PI * 0.5f - appAcos( DotRight );
		}
		const FLOAT Y = PI * 0.5f - appAcos( Clamp( m_ToTarget | m_NpcUp, -1.f, 1.f ) );
		X -= FaceToAngle;

		const FVector2D DeltaCursor( X - m_CurrentCursorPosition.m_CurrentValue.X, Y - m_CurrentCursorPosition.m_CurrentValue.Y );
		const FLOAT DeltaJump = ( m_PreviousDelta - DeltaCursor ).Size();
		if( DeltaJump > 3.f * PI * 0.5f )
		{
			m_fDelay = Max( 1.f - m_CurrentCursorPosition.m_fSpeedFactor, 0.f );
		}
		else if( DeltaCursor.Size() >= PI )
		{
			m_fDelay = Max( m_fDelay - _fTimeStep, 0.f );
		}
		else
		{
			m_fDelay = 0.f;
		}
		if( DeltaJump <= PI )
		{
			if( DeltaCursor.Size() < PI * 0.5f )
			{
				m_CurrentUseTorsoDeadZone.m_TargetValue = 1.f;
			}
		}
		else
		{
			m_CurrentUseTorsoDeadZone.m_TargetValue = 0.f;
		}
		m_PreviousDelta = DeltaCursor;

		if( m_fDelay <= 0.0001f && m_pConfig->m_LookAtModes.IsValidIndex( m_LookAtMode ) )
		{
			const FArkCpntLookatMode& rMode = m_pConfig->m_LookAtModes( m_LookAtMode );
			const FLOAT RangeSum = Clamp( m_CurrentTorsoRange.m_CurrentValue + m_CurrentHeadRange.m_CurrentValue, 0.f, 1.f );
			FLOAT TargetX = RangeSum * ( rMode.m_HeadAndTorso_HorizontalPercentage * X * 0.01f );
			FLOAT TargetY = RangeSum * ( rMode.m_HeadAndTorso_VerticalPercentage * Y * 0.01f );
			m_CurrentCursorPosition.m_CurrentValue.X -= FaceToAngle;
			FVector2D TargetDelta( TargetX - m_CurrentCursorPosition.m_CurrentValue.X, TargetY - m_CurrentCursorPosition.m_CurrentValue.Y );
			if( rMode.m_bDoClampRange )
			{
				const FLOAT DegToRad = PI / 180.f;
				TargetX = Max( Min( TargetX, rMode.m_ClampRightRange * DegToRad ), rMode.m_ClampLeftRange * DegToRad );
				TargetY = Max( Min( TargetY, rMode.m_ClampUpRange * DegToRad ), rMode.m_ClampDownRange * DegToRad );
			}
			const FVector2D MaxAimDegrees = m_pLookAtNode->GetMaxAim( FVector2D( TargetX, TargetY ) );
			const FVector2D MaxAim( MaxAimDegrees.X * ( PI / 180.f ), MaxAimDegrees.Y * ( PI / 180.f ) );
			FVector2D Target( TargetX, TargetY );
			if( !( Abs( TargetX ) <= Abs( MaxAim.X ) && Abs( TargetY ) <= Abs( MaxAim.Y ) ) )
			{
				Target = MaxAim;
				TargetDelta = FVector2D( MaxAim.X - m_CurrentCursorPosition.m_CurrentValue.X, MaxAim.Y - m_CurrentCursorPosition.m_CurrentValue.Y );
			}
			const FLOAT BehindThreshold = 2.8274333f;
			if( m_UnclampedTargetXPosition * X >= 0.f || Abs( m_UnclampedTargetXPosition ) <= BehindThreshold || Abs( X ) <= BehindThreshold )
			{
				m_UnclampedTargetXPosition = X;
			}
			else
			{
				Target.X = m_CurrentCursorPosition.m_TargetValue.X;
			}
			m_CurrentCursorPosition.m_TargetValue = Target;
			m_PreviousTarget = TargetDelta;
		}
		m_CurrentCursorPosition.Update( _fTimeStep, NULL, NULL );

		const FVector2D Aim( Clamp( m_CurrentCursorPosition.m_CurrentValue.X, -(FLOAT)PI, (FLOAT)PI ) * ( 180.f / PI ),
			Clamp( m_CurrentCursorPosition.m_CurrentValue.Y, -(FLOAT)PI * 0.5f, (FLOAT)PI * 0.5f ) * ( 180.f / PI ) );
		m_pLookAtNode->SetAim( Aim );
	}
	m_CurrentUseTorsoDeadZone.Update( _fTimeStep, NULL, NULL );
	m_pLookAtNode->SetUseTorsoDeadZone( m_CurrentUseTorsoDeadZone.m_CurrentValue );
}

/**
 * DISHONORED(port): 2013 rva 0x552640 (2012 0x593220), 814 bytes. Only while FaceFX is playing: FaceFX drives the eye
 * bones itself, and this clamps what it wrote to the eye limits and recomposes the two eye bones' space bases from
 * their parents. It is the eyes, not the head - agent FL's correction of agent DO's note stands.
 */
void FArkComponentLookat::OnComposeSkeleton()
{
	if( !m_bFaceFxIsPlaying || !m_pSkeletalComponent )
	{
		return;
	}
	const INT EyeBones[2] = { m_LeftEyeBoneIndex, m_RightEyeBoneIndex };
	const INT EyeParents[2] = { m_LeftEyeParentBoneIndex, m_RightEyeParentBoneIndex };
	for( INT Eye = 0; Eye < 2; Eye++ )
	{
		const INT BoneIndex = EyeBones[Eye];
		const INT ParentIndex = EyeParents[Eye];
		if( BoneIndex < 0 || !m_pSkeletalComponent->LocalAtoms.IsValidIndex( BoneIndex )
			|| !m_pSkeletalComponent->SpaceBases.IsValidIndex( BoneIndex ) || !m_pSkeletalComponent->SpaceBases.IsValidIndex( ParentIndex ) )
		{
			continue;
		}
		FBoneAtom& LocalAtom = m_pSkeletalComponent->LocalAtoms( BoneIndex );
		FVector Euler = LocalAtom.GetRotation().Euler();
		Euler.X = Clamp( Euler.X, MIN_EYE_YAW, MAX_EYE_YAW );
		Euler.Y = Clamp( Euler.Y, MIN_EYE_PITCH, MAX_EYE_PITCH );
		LocalAtom.SetRotation( FQuat::MakeFromEuler( Euler ) );
		m_pSkeletalComponent->SpaceBases( BoneIndex ) = LocalAtom * m_pSkeletalComponent->SpaceBases( ParentIndex );
	}
}

/*-----------------------------------------------------------------------------
	The Engine seams: FArkComponentManager's PreAsyncWork policy and UpdateSkelPoseEnd's OnComposeSkeleton call,
	both of which Engine runs on the pawn's body mesh (Engine/Inc/arkcomponentbase.h).
-----------------------------------------------------------------------------*/

static void DisArkLookatPreAsyncWork( USkeletalMeshComponent* _pMesh, FLOAT _fDeltaTime )
{
	APawn* Pawn = _pMesh && _pMesh->GetOwner() ? _pMesh->GetOwner()->GetAPawn() : NULL;
	if( !Pawn || Pawn->Mesh != _pMesh )
	{
		return;
	}
	if( FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( Pawn ) )
	{
		Lookat->PreAsyncWorkTick( _fDeltaTime );
	}
}

static void DisArkLookatComposeSkeleton( USkeletalMeshComponent* _pMesh )
{
	APawn* Pawn = _pMesh && _pMesh->GetOwner() ? _pMesh->GetOwner()->GetAPawn() : NULL;
	if( !Pawn )
	{
		return;
	}
	if( FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( Pawn ) )
	{
		Lookat->OnComposeSkeleton();
	}
}

struct FDisArkLookatSeamInstaller
{
	FDisArkLookatSeamInstaller()
	{
		GArkLookatPreAsyncWorkHook = &DisArkLookatPreAsyncWork;
		GArkLookatComposeSkeletonHook = &DisArkLookatComposeSkeleton;
	}
};
static FDisArkLookatSeamInstaller GDisArkLookatSeamInstaller;
