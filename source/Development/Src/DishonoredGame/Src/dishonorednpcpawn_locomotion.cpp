// DishonoredGame/src/dishonorednpcpawn_locomotion.cpp
// DISHONORED(port): agent DN. The pawn's half of locomotion: it owns the component, it hands its walking physics to it,
// and it is the object the navigation mesh asks about the pawn.
// Retail attributes Spawned / physWalking / SetupPathfindingParams / SetupPathGoalsAndConstraints / GetComponentLocomotion
// to dishonorednpcpawn.cpp and IsMoving / UpdateLocomotion / GetCurrentTransitSpeed to this unit; they are together here so
// that the locomotion package touches one NPC-pawn unit rather than two (agent DO owns the appearance and animation paths
// of dishonorednpcpawn.cpp in this wave).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "dishonoredutilities.h"
#include "disaicensus.h"

// DISHONORED(port): 2013 rva 0x74b3d0 (2012 0x7ab8c0): seven bytes, the member.
FArkComponentLocomotion* ADishonoredNPCPawn::GetComponentLocomotion() const
{
	return (FArkComponentLocomotion*)m_pCpntLocomotion;
}

// DISHONORED(port): 2013 rva 0x752270 (2012 0x7b0a60, 427 bytes). Retail does, in order: AActor::Spawned; in the editor
// only, the mesh's begin-play and the inventory / actions / body pre-begin-play passes; then, when the NPC's tweaks name a
// UArkComponentLocomotionConfig, InitNavigationHandle followed by AddNewComponent<FArkComponentLocomotion>, SetConfig,
// Starting and SetOwnerPawnAvoidable - and if SetConfig refuses the config, the component is removed again, so a
// mis-tweaked NPC has no locomotion rather than a broken one. Then the avoidance steering angle, the look-at component,
// the pose-blender anim node and the teleport clock.
// DISHONORED(bringup): FArkComponentLookat is not ported, so its two lines are left out; the pose-blender lookup and the
// teleport clock belong to the animation and possession paths and are left to their owners.
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
	if( !NPCTweaks || !NPCTweaks->m_pComponentLocomotionConfig || !m_ComponentContainer )
	{
		return;
	}

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
// DISHONORED(bringup): retail's PreAsyncWork tick comes from FArkComponentManager, which is not ported (see
// Inc/arkcomponentlocomotion.h). It is driven from here instead, immediately before the move, which keeps retail's
// within-frame order - the dynamics decide the frame's move, then the move is applied.
void ADishonoredNPCPawn::physWalking( FLOAT DeltaTime, INT Iterations )
{
	FArkComponentLocomotion* pLoco = GetComponentLocomotion();
	ADishonoredGameInfo* pGameInfo = DisGetGameInfo();
	if( pLoco && pLoco->m_bStarted && m_pPossessingController == NULL && pGameInfo )
	{
		DISLOCO_MARK( phys_enter )
		pLoco->PreAsyncWorkTick( DeltaTime );
		DISLOCO_MARK( phys_tick_done )
		UpdateLocomotion();
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
