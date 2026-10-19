// DishonoredGame/src/dishonorednpcpawn_locomotion.cpp
// DISHONORED(port): agent DN. The pawn's half of locomotion: it owns the component, it hands its walking physics to it,
// and it is the object the navigation mesh asks about the pawn.
// Retail attributes Spawned / physWalking / SetupPathfindingParams / SetupPathGoalsAndConstraints / GetComponentLocomotion
// to dishonorednpcpawn.cpp and IsMoving / UpdateLocomotion / GetCurrentTransitSpeed to this unit; they are together here so
// that the locomotion package touches one NPC-pawn unit rather than two (agent DO owns the appearance and animation paths
// of dishonorednpcpawn.cpp in this wave).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"
#include "arkcomponentlookat.h"
#include "fkmatineediag.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "dishonoredutilities.h"
#include "disaicensus.h"
#include "ibdiag.h"

// DISHONORED(port): 2013 rva 0x74b3d0 (2012 0x7ab8c0): seven bytes, the member.
FArkComponentLocomotion* ADishonoredNPCPawn::GetComponentLocomotion() const
{
	return (FArkComponentLocomotion*)m_pCpntLocomotion;
}

// DISHONORED(port): 2013 rva 0x74b3f0 (2012 0x7ab8e0): seven bytes, the member (agent FT).
FArkComponentLookat* ADishonoredNPCPawn::GetComponentLookat() const
{
	return (FArkComponentLookat*)m_pCpntLookat;
}

// DISHONORED(port): 2013 rva 0x752270 (2012 0x7b0a60, 427 bytes). Retail does, in order: AActor::Spawned; in the editor
// only, the mesh's begin-play and the inventory / actions / body pre-begin-play passes; then, when the NPC's tweaks name a
// UArkComponentLocomotionConfig, InitNavigationHandle followed by AddNewComponent<FArkComponentLocomotion>, SetConfig,
// Starting and SetOwnerPawnAvoidable - and if SetConfig refuses the config, the component is removed again, so a
// mis-tweaked NPC has no locomotion rather than a broken one. Then the avoidance steering angle, the look-at component,
// the pose-blender anim node and the teleport clock.
// agent FT: the look-at component's three lines are in (AddNewComponent<FArkComponentLookat>, SetConfig with the NPC
// tweaks' m_pComponentLookatConfig, m_bStarted + Starting); retail creates it unconditionally, config or none, and the
// component's own PreAsyncWorkTick stays inert without a node.
// agent HB: the tail - the pose blender (the only native writer of m_pBlendPoseNode, 2012 0x7b0bf2 / 2013 offset 3456; no
// script names it) and the teleport clock.
void ADishonoredNPCPawn::Spawned()
{
	Super::Spawned();

	DISLOCO_MARK( spawned_enter )
	UDisTweaks_NPCPawn* NPCTweaks = Cast<UDisTweaks_NPCPawn>( GetTweaks_Derived() );
	if( !NPCTweaks )
	{
		NPCTweaks = UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_NPCPawn>();
	}
	DISLOCO_TRACE( spawned_tweaks, "tweaks %s config %s container %s",
		NPCTweaks ? *NPCTweaks->GetName() : TEXT("none"),
		( NPCTweaks && NPCTweaks->m_pComponentLocomotionConfig ) ? *NPCTweaks->m_pComponentLocomotionConfig->GetName() : TEXT("none"),
		m_ComponentContainer ? TEXT("yes") : TEXT("no") )
	if( !NPCTweaks || !m_ComponentContainer )
	{
		return;
	}
	if( NPCTweaks->m_pComponentLocomotionConfig )
	{
		SpawnedLocomotion( NPCTweaks );
	}

	FArkComponentLookat* pLookat = m_ComponentContainer->AddNewComponent<FArkComponentLookat>();
	m_pCpntLookat = (FPointer)pLookat;
	if( pLookat )
	{
		pLookat->SetConfig( NPCTweaks->m_pComponentLookatConfig );
		pLookat->m_bStarted = TRUE;
		pLookat->Starting();
	}

	UAnimTree* Tree = ( Mesh != NULL ) ? Cast<UAnimTree>( Mesh->Animations ) : NULL;
	m_pBlendPoseNode = Tree ? Cast<UArkAnimNodeBlendPose>( Tree->FindAnimNode( FName(TEXT("PoseBlender")) ) ) : NULL;
	m_fLastTeleportTime = GWorld->GetTimeSeconds();
	// DISHONORED(bringup): agent HB. Retail arms the pose blender only from ADishonoredNPCPawn::OnLODChanged (2012 0x7bfce0:
	// SetEnable( LOD flags bit 8 )), raised by FDisComponentLODManager, which this tree does not have (cooked m_bEnabled is 0 on
	// every NPC PoseBlender, HB1.log). Every NPC is treated as at the LOD that captures, so BlendNow has a pose to blend from.
	if( m_pBlendPoseNode != NULL )
	{
		m_pBlendPoseNode->SetEnable( TRUE );
	}
	if( DisFKDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag blendpose %s Spawned: tree %s PoseBlender %s enabled %d"), *GetName(),
			Tree ? *Tree->GetName() : TEXT("none"), m_pBlendPoseNode ? *m_pBlendPoseNode->GetName() : TEXT("none"),
			m_pBlendPoseNode ? (INT)m_pBlendPoseNode->m_bEnabled : -1 );
	}
}

// agent FT: the locomotion half of Spawned, unchanged from agent DN's port, split out so the look-at component is created
// whether or not the tweaks name a locomotion config - which is retail's control flow.
void ADishonoredNPCPawn::SpawnedLocomotion( UDisTweaks_NPCPawn* NPCTweaks )
{
	// The navigation handle first: SetConfig and everything after it reads it, and UNavigationHandle::FindPath is
	// dispatched through the pawn as IInterface_NavigationHandle, so without the handle there is nothing to search with.
	if( !m_NavigationHandle )
	{
		if( !m_NavigationHandleClass )
		{
			// DISHONORED(bringup): retail's NPC archetypes set m_NavigationHandleClass in script defaults. The cooked
			// archetypes in this build leave it None on the classes the tower spawns, so the base class is used - which is
			// what APawn::InitNavigationHandle would construct if the property were set to its own default.
			m_NavigationHandleClass = UNavigationHandle::StaticClass();
		}
		InitNavigationHandle();
		DISLOCO_TRACE( spawned_navhandle, "handle %s", m_NavigationHandle ? *m_NavigationHandle->GetName() : TEXT("none") )
	}

	FArkComponentLocomotion* pLoco = m_ComponentContainer->AddNewComponent<FArkComponentLocomotion>();
	m_pCpntLocomotion = (FPointer)pLoco;
	if( !pLoco )
	{
		return;
	}
	DISLOCO_MARK( spawned_component )
	if( !pLoco->SetConfig( NPCTweaks->m_pComponentLocomotionConfig ) )
	{
		m_ComponentContainer->RemoveComponent( pLoco );
		m_pCpntLocomotion = NULL;
		return;
	}
	DISLOCO_MARK( spawned_setconfig )
	pLoco->m_bStarted = TRUE;
	pLoco->Starting();
	DISLOCO_MARK( spawned_starting )
	pLoco->SetOwnerPawnAvoidable( m_pAvoidable );

	m_fCosAvoidanceSteeringMaxAngle = appCos( NPCTweaks->m_fAvoidanceSteeringMaxAngle * ( PI / 180.f ) );
}

// DISHONORED(port): 2013 rva 0x74e8a0 (2012 0x7ad240, 107 bytes). The whole point of the NPC's walking physics: when the
// locomotion component is live, the NPC is not possessed and the game is running, the component moves the pawn; otherwise
// UE3's own walking physics does. That "otherwise" is not a fallback - it is what a possessed NPC uses.
// Agent HC: retail's body is the MovePawn alone (2012 0x7ad240); the dynamics tick comes from FArkComponentManager's
// PreAsyncWork policy (Inc/arkcomponentlocomotion.h) and UpdateLocomotion from ADishonoredNPCPawn::Tick.
void ADishonoredNPCPawn::physWalking( FLOAT DeltaTime, INT Iterations )
{
	FArkComponentLocomotion* pLoco = GetComponentLocomotion();
	ADishonoredGameInfo* pGameInfo = DisGetGameInfo();
	if( pLoco && pLoco->m_bStarted && m_pPossessingController == NULL && pGameInfo )
	{
		DISLOCO_MARK( phys_enter )
		pLoco->MovePawn( DeltaTime, FALSE );
		DISLOCO_MARK( phys_move_done )
	}
	else
	{
		Super::physWalking( DeltaTime, Iterations );
	}
}

// DISHONORED(port): 2013 rva 0x77cd10 (2012 0x7b84a0, 841 bytes).
// DISHONORED(bringup): the whole body is avoidance bookkeeping - the radius the NPC claims in the crowd (bigger when
// running or fighting), its force multiplier, and whether its steering force is pointing far enough away from its path to
// count as an avoidance push. UArkAvoidable has no ported methods, so there is nothing to tell. Left as retail's site,
// named, so the avoidance package has somewhere to land.
void ADishonoredNPCPawn::UpdateLocomotion()
{
}

/*-----------------------------------------------------------------------------
	-ibdiag (agent IB, PHASE23 IB)
-----------------------------------------------------------------------------*/

UBOOL DisIBDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ParseParam( appCmdLine(), TEXT("ibdiag") ) ? 1 : 0;
	}
	return Enabled != 0;
}

UBOOL DisIBDiagWatched( const AActor* _pActor )
{
	static const FName Watched[] = { FName( TEXT("DishonoredNPCPawn_26") ), FName( TEXT("DishonoredNPCPawn_27") ), FName( TEXT("DishonoredNPCPawn_20") ), FName( TEXT("DishonoredNPCPawn_3") ) };
	if( !_pActor )
	{
		return FALSE;
	}
	const FName Name = _pActor->GetFName();
	for( INT Index = 0; Index < ARRAY_COUNT( Watched ); Index++ )
	{
		if( Name == Watched[Index] )
		{
			return TRUE;
		}
	}
	return FALSE;
}

struct FIBDiagPawnState
{
	FLOAT	NextReport;
	FVector	LastLocation;
	INT		GoToStage;
	INT		PathingFails;
	FIBDiagPawnState() : NextReport( 0.f ), LastLocation( 0.f, 0.f, 0.f ), GoToStage( -1 ), PathingFails( 0 ) {}
};
static TMap<const AActor*, FIBDiagPawnState> GIBDiagPawns;

void DisIBDiagNoteGoTo( const AActor* _pPawn, INT _Stage, UBOOL _bPathingFail )
{
	if( !DisIBDiagEnabled() || !DisIBDiagWatched( _pPawn ) )
	{
		return;
	}
	FIBDiagPawnState* pState = GIBDiagPawns.Find( _pPawn );
	if( !pState )
	{
		pState = &GIBDiagPawns.Set( _pPawn, FIBDiagPawnState() );
	}
	pState->GoToStage = _Stage;
	if( _bPathingFail )
	{
		pState->PathingFails++;
		debugf( TEXT("DISHONORED(bringup): ibdiag %s FilterPathingFail hit %d world %.2f"), *_pPawn->GetName(), pState->PathingFails, GWorld->GetTimeSeconds() );
	}
}

void DisIBDiagReport( ADishonoredNPCPawn* _pPawn )
{
	if( !DisIBDiagEnabled() || !DisIBDiagWatched( _pPawn ) )
	{
		return;
	}
	FIBDiagPawnState* pState = GIBDiagPawns.Find( _pPawn );
	if( !pState )
	{
		pState = &GIBDiagPawns.Set( _pPawn, FIBDiagPawnState() );
		pState->LastLocation = _pPawn->Location;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < pState->NextReport )
	{
		return;
	}
	pState->NextReport = Now + 1.f;
	const FLOAT Moved = ( _pPawn->Location - pState->LastLocation ).Size2D();
	pState->LastLocation = _pPawn->Location;
	FArkComponentLocomotion* pLoco = _pPawn->GetComponentLocomotion();
	debugf( TEXT("DISHONORED(bringup): ibdiag %s world %.2f at %.0f/%.0f/%.0f moved %.1f phys %d base %s retarget %d possessed %d ctrls %d goto %d pfail %d | %s"),
		*_pPawn->GetName(), Now, _pPawn->Location.X, _pPawn->Location.Y, _pPawn->Location.Z, Moved, (INT)_pPawn->Physics,
		_pPawn->Base ? *_pPawn->Base->GetName() : TEXT("none"), (INT)_pPawn->IsRetargeting(), _pPawn->m_pPossessingController ? 1 : 0,
		_pPawn->m_MatineeControllersInfos.Num(), pState->GoToStage, pState->PathingFails, pLoco ? *pLoco->DescribeForIBDiag() : TEXT("no locomotion") );
}

/*-----------------------------------------------------------------------------
	-ikdiag (agent IK, PHASE23 IK)
-----------------------------------------------------------------------------*/

UBOOL DisIKDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ParseParam( appCmdLine(), TEXT("ikdiag") ) ? 1 : 0;
	}
	return Enabled != 0;
}

UBOOL DisIKDiagWatched( const AActor* _pActor )
{
	static const FName Watched[] = { FName( TEXT("DishonoredNPCPawn_26") ), FName( TEXT("DishonoredNPCPawn_3") ) };
	if( !_pActor || !GWorld )
	{
		return FALSE;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < 100.f || Now > 200.f )
	{
		return FALSE;
	}
	const FName Name = _pActor->GetFName();
	return Name == Watched[0] || Name == Watched[1];
}

// DISHONORED(port): 2013 rva 0x779730 (2012 0x7b4f70, 96 bytes). Retail says an NPC is moving when its transit speed is
// not idle, OR its face-to component has an angle left to turn, OR its locomotion anim node is playing a specific
// animation (a turn, a start or a stop).
// DISHONORED(bringup): the second and third tests need FArkComponentFaceTo and UArkAnimNodeLocomotion; the first is the
// real one and it is what the census reads.
UBOOL ADishonoredNPCPawn::IsMoving() const
{
	// DISHONORED(bringup): GetCurrentTransitSpeed (2013 rva 0x779850) reads the locomotion component's active request
	// speed index back through UDisTweaks_NPCPawn::m_LocomotionSpeedToTransitSpeed and is not ported; the component's own
	// state answers the same question - an NPC with an open request that has not arrived is moving.
	const FArkComponentLocomotion* pLoco = GetComponentLocomotion();
	return ( pLoco && pLoco->GetRequestsCount() > 0 && !pLoco->IsArrived() ) ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x74ad20 (2012 0x7ab3a0, 203 bytes). The NPC's own pathfinding parameters, and they differ
// from APawn's in exactly two ways that matter: the search extent comes from the NPC tweaks' own pathfinding radius and
// height rather than from the pawn's collision cylinder (so a guard can path through a gap narrower than its cylinder's
// bounding box), and the search start is DisGetPawnFeet rather than the cylinder's centre minus its height.
void ADishonoredNPCPawn::SetupPathfindingParams( FNavMeshPathParams& _rOut_ParamCache )
{
	_rOut_ParamCache.bAbleToSearch = TRUE;
	// DISHONORED(bringup): FNavMeshPathParams::SearchLaneMultiplier is reference-only - retail's struct has no such
	// member - and in this tree it is a DISHONORED_SHIM_STATIC, i.e. ONE FLOAT shared by every pawn in the process. It is
	// read inside the A* (UnNavigationMesh.cpp:10752, the successor-edge lane offset), so leaving it unwritten means this
	// pawn's search uses whatever the last controller or crowd agent left there. Every writer in the tree writes 0
	// (AController::SetupPathfindingParams, ACrowdAgentBase's), and 0 is what "retail has no lanes" means, so it is
	// written here too rather than inherited. Agent DP's shim audit is what prompted looking.
	_rOut_ParamCache.SearchLaneMultiplier = 0.f;

	const FVector Cylinder = GetCylinderExtent();
	// DISHONORED(bringup): retail reads the two floats at +452 / +456 of the NPC tweaks (the decompile's
	// `*(float *)(ArrayMax + 452)` pair, reached through this->Touching.ArrayMax, which is how IDA renders the tweaks
	// pointer). The tweaks member names for those two offsets are not resolved, so the pawn's own cylinder is used - which
	// is what APawn does and is never larger than the tweaks' values.
	_rOut_ParamCache.SearchExtent = Cylinder;
	_rOut_ParamCache.SearchStart = DisGetPawnFeet( this );
	_rOut_ParamCache.bCanMantle = bCanMantle;
	_rOut_ParamCache.MaxDropHeight = LedgeCheckThreshold;
	_rOut_ParamCache.MinWalkableZ = WalkableFloorZ;
	_rOut_ParamCache.MaxHoverDistance = bCanFly ? -1.f : 10.f;
}

// DISHONORED(port): 2013 rva 0x75e490 (2012 0x7c2040, 425 bytes). The pawn's navigation handle is cleared of last
// search's goals and constraints, the brain is asked for this search's pair, and each one is added to the handle. A dead
// NPC, an NPC with no controller and an NPC whose brain is not initialised all answer FALSE, which is what makes
// UpdatePathFindingEdges refuse to search rather than search with no goal (a search with no goal evaluator visits the
// whole mesh and answers nothing).
UBOOL ADishonoredNPCPawn::SetupPathGoalsAndConstraints( const FVector& _rFinalDestination, UBOOL _bForReachability )
{
	DISLOCO_MARK( goals_enter )
	UNavigationHandle* pNavigationHandle = GetNavigationHandle();
	if( !pNavigationHandle )
	{
		return FALSE;
	}
	pNavigationHandle->ClearConstraints();

	TArray<UNavMeshPathGoalEvaluator*> Goals;
	TArray<UNavMeshPathConstraint*> Constraints;
	UBOOL bResult = FALSE;

	ADishonoredNPCController* pController = Cast<ADishonoredNPCController>( Controller );
	if( Health > 0 && pController )
	{
		UDishonoredAIBrain* pBrain = pController->GetAIBrain();
		if( pBrain && pBrain->IsBrainInitialized() )
		{
			bResult = pBrain->GetPathGoalsAndConstraintsFromBehavior( _rFinalDestination, _bForReachability, Goals, Constraints ) ? TRUE : FALSE;
		}
	}

	DISLOCO_TRACE( goals_got, "result %i goals %i constraints %i", (INT)bResult, Goals.Num(), Constraints.Num() )
	for( INT i = 0; i < Goals.Num(); ++i )
	{
		pNavigationHandle->AddGoalEvaluator( Goals( i ) );
	}
	for( INT i = 0; i < Constraints.Num(); ++i )
	{
		pNavigationHandle->AddPathConstraint( Constraints( i ) );
	}
	return bResult;
}

// ---- agent FV (PHASE18 FV): the transit speed and the look-at choice ----

// DISHONORED(port): 2013 rva 0x779430 (2012 0x7b4cf0, 151 B): the locomotion component's most relevant speed index, mapped back
// through the NPC tweaks' m_LocomotionSpeedToTransitSpeed; no index, or one past the table, is Idle. (The 0x779850 the note
// in IsMoving cites is not this function.)
BYTE ADishonoredNPCPawn::GetCurrentTransitSpeed() const
{
	const FArkComponentLocomotion* Loco = GetComponentLocomotion();
	const INT SpeedIdx = Loco ? Loco->GetMostRelevantSpeedIdx() : INDEX_NONE;
	if( SpeedIdx == INDEX_NONE )
	{
		return ETransitSpeed_Idle;
	}
	const UDisTweaks_NPCPawn* Tweaks = Cast<UDisTweaks_NPCPawn>( const_cast<ADishonoredNPCPawn*>( this )->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_NPCPawn>();
	}
	if( SpeedIdx < 0 || SpeedIdx >= Tweaks->m_LocomotionSpeedToTransitSpeed.Num() )
	{
		return ETransitSpeed_Idle;
	}
	return (BYTE)Tweaks->m_LocomotionSpeedToTransitSpeed( SpeedIdx );
}

// DISHONORED(port): 2013 rva 0x7794d0 (2012 0x7b4d90, 475 B): the look-at mode for the body stance the pawn intends and the
// speed it moves at. Most stances take the equipped primary item's default mode (an aiming pawn with a secondary item
// in hand aims); Search takes SearchIdle / SearchWalk when the intended primary item is equippable and not empty hands,
// Empty* otherwise; EquippedPatrol takes Empty* by speed; CrouchedAim leaves the mode unchanged (eDisNPCLookAtMode_MAX).
BYTE ADishonoredNPCPawn::ChooseLookAt()
{
	const BYTE TransitSpeed = GetCurrentTransitSpeed();
	const BYTE Stance = m_BodyIntention[m_CurrentBodyStancePriority].m_IntendedBodyStance;
	switch( Stance )
	{
	case eDisNPCBodyStance_None:
	case eDisNPCBodyStance_Relaxed:
	case eDisNPCBodyStance_Equipped:
	case eDisNPCBodyStance_Aiming:
	case eDisNPCBodyStance_Panicked:
	case eDisNPCBodyStance_PanicRecover:
	case eDisNPCBodyStance_MeleeRush:
	case eDisNPCBodyStance_EscapeExplosion:
		{
			UDishonoredInventoryItem* Primary = m_pInventory ? m_pInventory->GetEquippedItem( EDisEquipUsage_Primary ) : NULL;
			const BYTE Mode = Primary ? Primary->GetDefaultNPCLookAtMode( TransitSpeed ) : eDisNPCLookAtMode_MAX;
			if( Stance == eDisNPCBodyStance_Aiming )
			{
				UDishonoredInventoryItem* Secondary = m_pInventory ? m_pInventory->GetEquippedItem( EDisEquipUsage_Secondary ) : NULL;
				if( Secondary && !Secondary->IsA( UDishonoredItemEmpty::StaticClass() ) )
				{
					return eDisNPCLookAtMode_AimIdle;
				}
			}
			return Mode;
		}
	case eDisNPCBodyStance_Search:
		{
			UClass* DesiredPrimary = m_BodyIntention[m_CurrentPrimaryItemPriority].m_pDesiredPrimaryItemClass;
			UDishonoredInventoryItem* Item = ( DesiredPrimary && m_pInventory ) ? m_pInventory->FindEquippableItem( DesiredPrimary, EDisEquipUsage_Primary ) : NULL;
			const UBOOL bHasSearchItem = Item && !Item->IsA( UDishonoredItemEmpty::StaticClass() );
			if( TransitSpeed == ETransitSpeed_Idle )
			{
				return bHasSearchItem ? eDisNPCLookAtMode_SearchIdle : eDisNPCLookAtMode_EmptyIdle;
			}
			if( TransitSpeed == ETransitSpeed_Walk )
			{
				return bHasSearchItem ? eDisNPCLookAtMode_SearchWalk : eDisNPCLookAtMode_EmptyWalk;
			}
			if( TransitSpeed == ETransitSpeed_Run )
			{
				return bHasSearchItem ? eDisNPCLookAtMode_SearchWalk : eDisNPCLookAtMode_EmptyRun;
			}
			return eDisNPCLookAtMode_MAX;
		}
	case eDisNPCBodyStance_EquippedPatrol:
		if( TransitSpeed == ETransitSpeed_Idle )
		{
			return eDisNPCLookAtMode_EmptyIdle;
		}
		if( TransitSpeed == ETransitSpeed_Walk )
		{
			return eDisNPCLookAtMode_EmptyWalk;
		}
		return TransitSpeed == ETransitSpeed_Run ? eDisNPCLookAtMode_EmptyRun : eDisNPCLookAtMode_MAX;
	}
	return eDisNPCLookAtMode_MAX;
}

// DISHONORED(port): agent HA. 2012 rva 0x7ac100 (136 B), 2013 rva 0x76e680 (VA 0xb6e680, 136 B, match_2012_2013 "bytes" 1.000):
// the base's mesh modes, the locomotion component off for the root motion, and a Translate / Velocity mode starts from rest.
void ADishonoredNPCPawn::StartRootMotionMode( BYTE _RootMotionMode, BYTE _RootRotationMode )
{
	ADishonoredPawn::StartRootMotionMode( _RootMotionMode, _RootRotationMode );
	FArkComponentLocomotion* Locomotion = FArkComponentLocomotion::GetLocomotionComponent( this );
	if( Locomotion != NULL )
	{
		Locomotion->Disable();
	}
	if( _RootMotionMode <= RMM_Velocity )
	{
		DesiredSpeed = 0.f;
		Velocity = FVector( 0.f, 0.f, 0.f );
		Acceleration = FVector( 0.f, 0.f, 0.f );
	}
}

// DISHONORED(port): agent HA. 2012 rva 0x7af1a0 (47 B), 2013 rva 0x771280 (VA 0xb71280, 47 B, same bytes): the base
// (ChangeAnimStateMotionModes( RMM_Velocity, RMRM_RotateActor )), the locomotion component back on, and the look-at
// component's +88 bit 1 (m_bHeadDisabled) cleared. UInterpGroupAI::UpdateGroup reaches this every frame of a key without
// root motion (SetActorRootMotion -> MAT_SetRootMotionMode -> MAT_StopRootMotion), as retail's does.
void ADishonoredNPCPawn::StopRootMotionMode()
{
	ADishonoredPawn::StopRootMotionMode();
	FArkComponentLocomotion* Locomotion = FArkComponentLocomotion::GetLocomotionComponent( this );
	if( Locomotion != NULL )
	{
		Locomotion->Enable();
	}
	FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( this );
	if( Lookat != NULL )
	{
		Lookat->m_bHeadDisabled = FALSE;
	}
}
