// DishonoredGame/src/arkcomponentlocomotion.cpp
// DISHONORED(port): agent DN. The locomotion component's lifetime, its configuration and its per-frame entry point.
// Retail's unit is Engine/Src/arkcomponentlocomotion.cpp (24 functions, 8,111 bytes); it is here rather than in Engine
// for the reason the header's banner gives.

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "disdesirestructs.h"
#include "disaicensus.h"

ARKCOMPONENT_IMPLEMENT_TYPE( FArkComponentLocomotion )

// DISHONORED(written): agent EN (PHASE13 EN). This unit is one the link always pulls in - the locomotion component's
// PreAsyncWorkTick is called from dishonorednpcpawn_locomotion.cpp - so it is where the module's other component
// registrants are named. A static-library member holding nothing but a registrant is never linked and its type never
// reaches the creator table; ARKCOMPONENT_LINK_TYPE in Engine/Inc/arkcomponentbase.h records how that was measured.
// The registrants themselves stay in their own components' units, which is where retail has them.
ARKCOMPONENT_LINK_TYPE( FDisAIKnowledgeComponent )			// DisCpntType_AIKnowledge, 210
ARKCOMPONENT_LINK_TYPE( FDisAIMonitorReaction )				// DisCpntType_AIMonitorReaction, 211
ARKCOMPONENT_LINK_TYPE( FDisAIMonitorPawnReachability )		// DisCpntType_AIMonitorPawnReachability, 208

TArray<FArkComponentLocomotion::FLocoCpntSharedProps>	FArkComponentLocomotion::ms_LocoCpntSharedProps;
INT														FArkComponentLocomotion::ms_ComponentCount = 0;
INT														FArkComponentLocomotion::ms_TickCountBeforePath = 0;

INT		GDisLocoComponents = 0;
INT		GDisLocoRequestsStarted = 0;
INT		GDisLocoRequestsUpdated = 0;
INT		GDisLocoRequestsStopped = 0;
INT		GDisLocoPathsBuilt = 0;
INT		GDisLocoPathsFailed = 0;
INT		GDisLocoPathPointsBuilt = 0;
INT		GDisLocoArrivals = 0;
INT		GDisLocoMovePawnCalls = 0;
FLOAT	GDisLocoDistanceMoved = 0.f;
INT		GDisLocoTeleports = 0;

// DISHONORED(layout): the 2012 PDB's sizes for every struct the request interface is built out of. They are what the
// request manager memcpys and what a save file would have to round-trip, so they are asserted rather than trusted.
checkAtCompileTime( sizeof( FArkCpntLocoRequestDataVector ) == 12, FArkCpntLocoRequestDataVector_is_12 );
checkAtCompileTime( sizeof( FArkCpntLocoRequestCommonProps ) == 40, FArkCpntLocoRequestCommonProps_is_40 );
checkAtCompileTime( sizeof( FArkCpntLocoRequestLocationProps ) == 28, FArkCpntLocoRequestLocationProps_is_28 );
checkAtCompileTime( sizeof( FArkCpntLocoRequestActorProps ) == 16, FArkCpntLocoRequestActorProps_is_16 );
checkAtCompileTime( sizeof( FArkCpntLocoGoToProp ) == 48, FArkCpntLocoGoToProp_is_48 );
checkAtCompileTime( sizeof( FArkCpntLocoGoToLocationProp ) == 76, FArkCpntLocoGoToLocationProp_is_76 );
checkAtCompileTime( sizeof( FArkCpntLocoGoToActorProp ) == 64, FArkCpntLocoGoToActorProp_is_64 );
checkAtCompileTime( sizeof( FArkCpntLocoPathPoint ) == 16, FArkCpntLocoPathPoint_is_16 );
checkAtCompileTime( sizeof( FArkCpntLocoPathFindingEdge ) == 32, FArkCpntLocoPathFindingEdge_is_32 );
checkAtCompileTime( sizeof( FArkComponentLocomotion::FLocoRequestData ) == 72, FLocoRequestData_is_72 );
checkAtCompileTime( sizeof( FArkComponentLocomotion::FLocoRequestWorkData ) == 20, FLocoRequestWorkData_is_20 );
checkAtCompileTime( sizeof( FArkComponentLocomotion::FArkCpntLocoTurnProps ) == 44, FArkCpntLocoTurnProps_is_44 );
checkAtCompileTime( sizeof( FArkComponentLocomotion::FLocoCpntSharedProps ) == 24, FLocoCpntSharedProps_is_24 );
checkAtCompileTime( sizeof( FArkRequestManager<FArkComponentLocomotion::FLocoRequestData> ) == 36, FArkRequestManager_loco_is_36 );
// DISHONORED(layout): the whole component. 2012 PDB sizeof 616. This assert is the single most load-bearing statement in
// the package: every offset in the header's member list came from that PDB record, and a mistake anywhere in the 104
// members shows up here rather than as a wrong number at runtime.
checkAtCompileTime( sizeof( FArkComponentLocomotion ) == 616, FArkComponentLocomotion_is_616 );

// DISHONORED(port): 2013 rva 0x5460f0 region (2012 0x586bd0)
FArkComponentLocomotion::FArkComponentLocomotion()
	: m_pConfig( NULL )
	, m_pMeshOffsetCpnt( NULL )
	, m_pPawnOwner( NULL )
	, m_pAvoidable( NULL )
	, m_bDisabled( FALSE )
	, m_bPrevHidden( FALSE )
	, m_bNoTick( FALSE )
	, m_CurPathPointIdx( INDEX_NONE )
	, m_CurPathYaw( -200000 )
	, m_NextPathYaw( -200000 )
	, m_CurPathDir( 0.f, 0.f, 0.f )
	, m_NextPathDir( 0.f, 0.f, 0.f )
	, m_PathPoint( 0.f, 0.f, 0.f )
	, m_LastPathPoint( 0.f, 0.f, 0.f )
	, m_NearestLocationOnNavMesh( 0.f, 0.f, 0.f )
	, m_StartLocationUsedToAskPath( 0.f, 0.f, 0.f )
	, m_EndLocationUsedToAskPath( 0.f, 0.f, 0.f )
	, m_LastPFError( PATHERROR_MAX )
	, m_fTryToReturnOnNavMeshDuration( 0.f )
	, m_fDistBetweenPathPoints( 0.f )
	, m_bTryToReturnOnNavMesh( FALSE )
	, m_bPathfindingUpToDate( FALSE )
	, m_bBuiltPathUpToDate( FALSE )
	, m_bPathIsDirty( FALSE )
	, m_bStartingNewPath( FALSE )
	, m_bPathAtEnd( TRUE )
	, m_bForcePathComputation( FALSE )
	, m_bMustTestPathAfterTurn( FALSE )
	, m_bHasBeenOnNavMesh( FALSE )
	, m_bHasRequestWithPathComputed( FALSE )
	, m_CurMoveYaw( -200000 )
	, m_fRealSpeed( 0.f )
	, m_fRealScaledSpeed( 0.f )
	, m_fCurMoveSpeed( 0.f )
	, m_fCurMoveRotationSpeed( 0.f )
	, m_fMaxMoveRotationSpeed( 0.f )
	, m_fMaxMoveRotSpeedMultiplier( 1.f )
	, m_fMaxMultiDirSpeedForAngle( 0.f )
	, m_TargetMoveYaw( -200000 )
	, m_fTargetMoveSpeed( 0.f )
	, m_fSteeringMultiplier( 1.f )
	, m_CurPawnLocation( 0.f, 0.f, 0.f )
	, m_CurSteeringForce( 0.f, 0.f, 0.f )
	, m_PreviousSteeringForce( 0.f, 0.f, 0.f )
	, m_PreviousPathForce( 0.f, 0.f, 0.f )
	, m_CurPathMove( 0.f, 0.f, 0.f )
	, m_CurMove( 0.f, 0.f, 0.f )
	, m_PreviousVelocity( 0.f, 0.f, 0.f )
	, m_MoveHitNormal( 0.f, 0.f, 0.f )
	, m_TeleportLocation( 0.f, 0.f, 0.f )
	, m_CurModifierIdx( 0 )
	, m_MostRelevantSpeedIdx( 0 )
	, m_MaxMultiDirSpeedIdx( 0 )
	, m_bInertiaDisabled( FALSE )
	, m_bSteeringDisabled( FALSE )
	, m_bFallAllowedByExternal( FALSE )
	, m_bFallAllowedByCode( FALSE )
	, m_bStuckedInAWall( FALSE )
	, m_bTeleportRequired( FALSE )
	, m_bTeleportDisabled( FALSE )
	, m_bIsArrived( TRUE )
	, m_fSq2DDistToPathEnd( 0.f )
	, m_fCurAcceleration( 0.f )
	, m_fCurDeceleration( 0.f )
	, m_fCurAdaptedAccel( 0.f )
	, m_fMultiDirSpeedRatio( 1.f )
	, m_LocoCpntIdx( INDEX_NONE )
	, m_PushStartLocOnPath( 0.f, 0.f, 0.f )
	, m_PushReturnOnPathLoc( 0.f, 0.f, 0.f )
	, m_PushReturnOnPathRequestIdx( INDEX_NONE )
	, m_bInPushPeriod( FALSE )
	, m_bSomeoneWantsToPushMe( FALSE )
	, m_fCurPitch( 0.f )
	, m_fCurRoll( 0.f )
	, m_fCurPitchRotateSpeed( 0.f )
	, m_fCurRollRotateSpeed( 0.f )
	, m_fCurPitchRollZOffset( 0.f )
	, m_fTargetPitch( 0.f )
	, m_fTargetRoll( 0.f )
	, m_fFollowDeadZoneEndDuration( 0.f )
	, m_fCurFollowAngle( 0.f )
	, m_fCurFollowAccelMultiplier( 1.f )
	, m_fFollowPredictRatio( 0.f )
	, m_CurFollowedActorMoveYaw( 0 )
	, m_RootState( CPNT_LOCO_ROOT_STATE_NONE )
	, m_fBeforeTurnSquaredDist( 0.f )
	, m_fBeforeTurnDuration( 0.f )
	, m_bHandleStop( FALSE )
	, m_bHaveStopAnim( FALSE )
	, m_pFaceToCpnt( NULL )
	, m_pLookatCpnt( NULL )
	, m_FaceToRequestID( INDEX_NONE )
	, m_LookatRequestID( INDEX_NONE )
	, m_fLookatSpeedMultiplier( 1.f )
	, m_bCanManageAnim( FALSE )
	, m_pAnimNodeLoco( NULL )
{
	// DISHONORED(written): FPathStore is a reflected script struct, so its EdgeList is a TArrayNoInit and its default
	// constructor leaves the count, the capacity and the allocation pointer as whatever was on the heap. Retail's
	// component is memset before its constructor runs (Arkane's operator new for components); ours is a plain `new`, so
	// the one member that is not self-initialising is zeroed here. Without this, the first
	// `m_RawPathFindingEdges.EdgeList = handle->PathCache.EdgeList` frees a garbage pointer, which is an access violation
	// on the first frame an NPC asks for a path - measured, not guessed: the trace reached pf_findpath_done and died
	// before the next line.
	appMemzero( &m_RawPathFindingEdges, sizeof( m_RawPathFindingEdges ) );
	GDisLocoComponents++;
}

FArkComponentLocomotion::~FArkComponentLocomotion()
{
}

/*-----------------------------------------------------------------------------
	lifetime
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54ac70 (2012 0x58b750). Retail does five things: it inserts itself into the world's
// FArkComponentManager PreAsyncWork policy, initialises the request queue with OnRequestManagerEvent as the owner
// callback, enables itself if it was disabled, starts the face-to component it created in SetConfig, and appends its row
// to the static shared-properties table (remembering its own index, which Stopping uses to swap-remove it).
// DISHONORED(bringup): the component-manager insertion is left out - see the header banner; the tick comes from
// ADishonoredNPCPawn::physWalking. Retail also registers for the LOD game event here
// (FArkGameEventDispatcher::RegisterToObjectEvent with event 2 and OnLODChanged, 2013 rva 0x53ecf0): left out because
// FDisComponentLODManager is unported, so no LOD event is ever raised and the handler would never run.
void FArkComponentLocomotion::Starting()
{
	DISLOCO_MARK( starting_enter )
	m_ReqMgr.Initialize( this, &FArkComponentLocomotion::RequestManagerEventThunk );

	if( m_bDisabled && !m_bPendingStop )
	{
		m_bDisabled = FALSE;
		OnEnable();
	}

	if( m_pFaceToCpnt )
	{
		// DISHONORED(bringup): FArkComponentFaceTo is not ported (2013 rva 0x53a...; agentDF.md lists it as one of the
		// three Ark components the desire layer stubs), so SetConfig leaves m_pFaceToCpnt NULL and retail's
		// `m_pFaceToCpnt->m_bStarted = 1; m_pFaceToCpnt->Starting();` has nothing to start.
	}

	++ms_ComponentCount;
	FLocoCpntSharedProps NewRow;
	NewRow.m_pLocoCpnt = this;
	NewRow.m_FinalLocation = FVector( 0.f, 0.f, 0.f );
	NewRow.m_bCloseToFinalLocation = FALSE;
	NewRow.m_bMustIgnoreAvoidance = FALSE;
	m_LocoCpntIdx = ms_LocoCpntSharedProps.AddItem( NewRow );
}

// DISHONORED(port): 2013 rva 0x54ad50 (2012 0x58b870). The queue is reset without any callback (an asker is never told
// its order was aborted by the component going away), the component leaves the manager's policy, and its row is
// swap-removed from the shared table - which is why the row that gets swapped into its place has to be told its new
// index before the removal.
void FArkComponentLocomotion::Stopping()
{
	m_ReqMgr.Reset();

	if( m_LocoCpntIdx != INDEX_NONE && m_LocoCpntIdx < ms_LocoCpntSharedProps.Num() )
	{
		const INT LastIdx = ms_LocoCpntSharedProps.Num() - 1;
		if( m_LocoCpntIdx < LastIdx )
		{
			ms_LocoCpntSharedProps( LastIdx ).m_pLocoCpnt->m_LocoCpntIdx = m_LocoCpntIdx;
		}
		ms_LocoCpntSharedProps.RemoveSwap( m_LocoCpntIdx, 1 );
		m_LocoCpntIdx = INDEX_NONE;
	}
	--ms_ComponentCount;
}

// DISHONORED(port): 2013 rva 0x53f490 (2012 0x580450)
DWORD FArkComponentLocomotion::GetAllocatedSize() const
{
	return m_PathFindingEdges.GetAllocatedSize()
		+ m_PathPoints.GetAllocatedSize()
		+ m_RawPathFindingEdges.EdgeList.GetAllocatedSize();
}

/*-----------------------------------------------------------------------------
	configuration
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x53f4c0 (2012 0x580480, 1,264 bytes). Retail's validity test is a long list of "does this
// config actually describe this pawn": the modifier and speed-name tables must be non-empty, the speed-mode index table
// must be exactly modifiers x speed names, every index in it must be inside the dictionary, and the skeletal-mesh name
// must match the pawn's. It logs which one failed and refuses the config, and the caller
// (ADishonoredNPCPawn::Spawned) then removes the component again - which is why a mis-tweaked NPC has no locomotion
// component at all rather than a broken one.
// DISHONORED(bringup): the five structural checks below are retail's; the skeletal-mesh-name check and the per-foot
// bone checks are not ported (they need the anim set names resolved against the pawn's mesh, which is the
// UArkAnimNodeLocomotion half of the package).
UBOOL FArkComponentLocomotion::IsConfigValid( const UArkComponentLocomotionConfig* const _pConfig, const APawn* _pPawn )
{
	if( !_pConfig || !_pPawn )
	{
		return FALSE;
	}
	const INT Modifiers = _pConfig->m_Modifiers.Num();
	const INT SpeedNames = _pConfig->m_SpeedNames.Num();
	if( Modifiers <= 0 || SpeedNames <= 0 )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s locomotion config %s has %i modifiers and %i speed names"),
			*_pPawn->GetName(), *_pConfig->GetName(), Modifiers, SpeedNames );
		return FALSE;
	}
	if( _pConfig->m_SpeedModes.Num() != Modifiers * SpeedNames )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s locomotion config %s has %i speed modes, expected %i x %i"),
			*_pPawn->GetName(), *_pConfig->GetName(), _pConfig->m_SpeedModes.Num(), Modifiers, SpeedNames );
		return FALSE;
	}
	for( INT i = 0; i < _pConfig->m_SpeedModes.Num(); ++i )
	{
		const INT DictIdx = _pConfig->m_SpeedModes( i );
		if( DictIdx < 0 || DictIdx >= _pConfig->m_SpeedModePropDictionary.Num() )
		{
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s locomotion config %s speed mode %i names dictionary entry %i of %i"),
				*_pPawn->GetName(), *_pConfig->GetName(), i, DictIdx, _pConfig->m_SpeedModePropDictionary.Num() );
			return FALSE;
		}
	}
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x543060 (2012 0x584010). The config is only taken when it validates against the pawn; then
// the anim node is looked up and initialised, the face-to component is created and initialised from the same config, and
// the modifier is (re)applied so the acceleration pair is in force from the first tick.
// DISHONORED(bringup): UArkAnimNodeLocomotion and FArkComponentFaceTo are unported, so m_bCanManageAnim stays FALSE and
// m_pFaceToCpnt stays NULL. Both are read defensively everywhere below, which is a path retail itself has - an NPC whose
// anim tree has no locomotion node runs with m_bCanManageAnim FALSE.
UBOOL FArkComponentLocomotion::SetConfig( const UArkComponentLocomotionConfig* const _pConfig )
{
	m_pPawnOwner = Cast<ADishonoredNPCPawn>( m_pOwner );
	if( !IsConfigValid( _pConfig, m_pPawnOwner ) )
	{
		return FALSE;
	}
	m_pConfig = _pConfig;
	m_pAnimNodeLoco = NULL;
	m_bCanManageAnim = FALSE;
	SetModifier( m_CurModifierIdx );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x543110 (2012 0x5840b0)
FArkComponentLocomotion* FArkComponentLocomotion::GetLocomotionComponent( const APawn* _pPawn )
{
	const ADishonoredNPCPawn* NPCPawn = ConstCast<ADishonoredNPCPawn>( _pPawn );
	return NPCPawn ? (FArkComponentLocomotion*)NPCPawn->m_pCpntLocomotion : NULL;
}

// DISHONORED(port): 2013 rva 0x541b70 (2012 0x582b40, 702 bytes): the modifier selects the acceleration pair and the
// speed multiplier, and retail then asks the anim node to blend into that modifier's transition animation.
// DISHONORED(bringup): the transition blend is the anim-node half; the acceleration pair is what the dynamics read.
void FArkComponentLocomotion::SetModifier( INT _ModifierIdx )
{
	if( !m_pConfig || _ModifierIdx < 0 || _ModifierIdx >= m_pConfig->m_Modifiers.Num() )
	{
		return;
	}
	m_CurModifierIdx = _ModifierIdx;
	const FArkCpntLocoModifierProp& rModifier = m_pConfig->m_Modifiers( _ModifierIdx );
	m_fCurAcceleration = rModifier.m_fAcceleration;
	m_fCurDeceleration = rModifier.m_fDeceleration;
	m_fCurAdaptedAccel = rModifier.m_fAcceleration;
}

// DISHONORED(port): 2013 rva 0x53f9b0 (2012 0x580990): the dictionary is indexed by (modifier * speed count + speed), and
// -10 means "the modifier I am in".
const FArkCpntLocoSpeedModeProp& FArkComponentLocomotion::GetSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const
{
	const INT ModifierIdx = ( _ModifierIdx == -10 ) ? m_CurModifierIdx : _ModifierIdx;
	const INT FlatIdx = _SpeedIdx + ModifierIdx * m_pConfig->m_SpeedNames.Num();
	return m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( FlatIdx ) );
}

// DISHONORED(port): 2013 rva 0x53e9c0 (2012 0x57fc80): retail forwards to
// UArkComponentLocomotionConfig::GetForwardSpeedOfSpeedMode with the flattened index.
// DISHONORED(bringup): that config method is not ported. The forward speed of a speed mode is the speed its first move
// animation plays at (FArkCpntLocoMoveAnimProp::m_fAnimSpeed), which is the value the dynamics compare against
// m_fCurMoveSpeed; a mode with no move animation is speed zero, which is how the "Idle" mode reads.
FLOAT FArkComponentLocomotion::GetForwardSpeedOfSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const
{
	if( !m_pConfig || _SpeedIdx < 0 || _SpeedIdx >= m_pConfig->m_SpeedNames.Num() )
	{
		return 0.f;
	}
	const INT ModifierIdx = ( _ModifierIdx == -10 ) ? m_CurModifierIdx : _ModifierIdx;
	if( ModifierIdx < 0 || ModifierIdx >= m_pConfig->m_Modifiers.Num() )
	{
		return 0.f;
	}
	const FArkCpntLocoSpeedModeProp& rMode = GetSpeedMode( _SpeedIdx, ModifierIdx );
	FLOAT fBest = 0.f;
	for( INT i = 0; i < rMode.m_MoveAnimProps.Num(); ++i )
	{
		fBest = Max( fBest, rMode.m_MoveAnimProps( i ).m_fAnimSpeed );
	}
	return fBest;
}

// DISHONORED(port): 2013 rva 0x541b00 (2012 0x582ad0): the modifier's speed multiplier times the look-at one.
FLOAT FArkComponentLocomotion::GetFinalSpeedMultiplier() const
{
	if( !m_pConfig || m_CurModifierIdx < 0 || m_CurModifierIdx >= m_pConfig->m_Modifiers.Num() )
	{
		return 1.f;
	}
	const FLOAT fMultiplier = m_pConfig->m_Modifiers( m_CurModifierIdx ).m_fSpeedMultiplier * m_fLookatSpeedMultiplier;
	return ( fMultiplier != 0.f ) ? fMultiplier : 1.f;
}

// DISHONORED(port): 2013 rva 0x53f090 (2012 0x5802a0): retail asks the anim node which speed mode is actually on screen.
// DISHONORED(bringup): with no anim node the honest answer is retail's own fall-through, 0 (the idle mode).
INT FArkComponentLocomotion::ComputeMostRelevantSpeedIdx() const
{
	return 0;
}

/*-----------------------------------------------------------------------------
	enable / disable
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x53e630 (2012 0x57f780)
void FArkComponentLocomotion::Enable()
{
	if( m_bDisabled && !m_bPendingStop )
	{
		m_bDisabled = FALSE;
		OnEnable();
	}
}

// DISHONORED(port): 2013 rva 0x546520 (2012 0x587210)
void FArkComponentLocomotion::Disable()
{
	if( !m_bDisabled )
	{
		m_bDisabled = TRUE;
		OnDisable();
	}
}

// DISHONORED(port): 2013 rva 0x53e650 (2012 0x57f7a0): disabled by the flag, or by the actor's own "no locomotion" bit.
UBOOL FArkComponentLocomotion::IsDisabled() const
{
	if( m_bDisabled )
	{
		return TRUE;
	}
	return ( m_pPawnOwner == NULL ) || m_pPawnOwner->bDeleteMe || m_pPawnOwner->IsPendingKill();
}

// DISHONORED(port): 2013 rva 0x53def0 (2012 0x57f040): the path is forced to recompute on the next tick, and the anim
// node is put back to zero speed.
void FArkComponentLocomotion::OnEnable()
{
	m_bForcePathComputation = TRUE;
}

// DISHONORED(port): 2013 rva 0x5461f0 (2012 0x586ef0): the three root-motion state machines are reset.
// DISHONORED(bringup): ResetTurn / ResetStartMove / ResetStopMove belong to arkcomponentlocomotionrootmove.cpp
// (2013 rvas 0x545a10 / 0x545ae0 / 0x545b70), which is the turn-in-place and start/stop animation system and is not
// ported - it drives UArkAnimNodeLocomotion, which is not ported either. The root state is put back to NONE, which is
// what all three resets amount to while there is no animation to wait for.
void FArkComponentLocomotion::OnDisable()
{
	m_RootState = CPNT_LOCO_ROOT_STATE_NONE;
	m_fCurMoveSpeed = 0.f;
	m_fTargetMoveSpeed = 0.f;
	m_CurMove = FVector( 0.f, 0.f, 0.f );
	m_CurPathMove = FVector( 0.f, 0.f, 0.f );
}

// DISHONORED(port): 2013 rva 0x53e0b0 (2012 0x57f200)
void FArkComponentLocomotion::AllowToFall( UBOOL _bAllow )
{
	m_bFallAllowedByExternal = _bAllow ? TRUE : FALSE;
}

// DISHONORED(port): 2012 rva 0x57f7c0
void FArkComponentLocomotion::EnableTeleportWhenOutsideOfNavmesh( UBOOL _bEnable )
{
	m_bTeleportDisabled = _bEnable ? FALSE : TRUE;
}

// DISHONORED(port): 2013 rva 0x546550 (2012 0x587240): after a teleport nothing the component remembers is true any
// more, so the path is dropped whole and the next tick starts a new one from wherever the pawn now is.
void FArkComponentLocomotion::HandleTeleport()
{
	ResetPathProperties();
	m_bForcePathComputation = TRUE;
	m_CurMove = FVector( 0.f, 0.f, 0.f );
	m_CurPathMove = FVector( 0.f, 0.f, 0.f );
	m_CurSteeringForce = FVector( 0.f, 0.f, 0.f );
	m_PreviousSteeringForce = FVector( 0.f, 0.f, 0.f );
	m_PreviousPathForce = FVector( 0.f, 0.f, 0.f );
	m_PreviousVelocity = FVector( 0.f, 0.f, 0.f );
	m_fCurMoveSpeed = 0.f;
	m_fTargetMoveSpeed = 0.f;
	m_CurMoveYaw = -200000;
	m_TargetMoveYaw = -200000;
	m_RootState = CPNT_LOCO_ROOT_STATE_NONE;
}

// DISHONORED(port): 2013 rva 0x53dde0 (2012 0x57ef30)
FVector FArkComponentLocomotion::GetPawnGroundLocation() const
{
	FVector Ground = m_pPawnOwner->Location;
	if( m_pPawnOwner->CylinderComponent )
	{
		Ground.Z -= m_pPawnOwner->CylinderComponent->CollisionHeight;
	}
	return Ground;
}

/*-----------------------------------------------------------------------------
	the per-frame entry point
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x54caa0 (2012 0x58d5f0, 1,363 bytes). Retail's order, which is what this reproduces:
//   1. skip the whole tick in the editor, or for a zero time step, or when the component is not configured;
//   2. notice that the owner's "hidden" state changed and enable/disable accordingly;
//   3. raise the avoidance push game event when something wants to push this NPC;
//   4. when the pawn's mesh is visible, hand the mesh-offset component the current move;
//   5. count down the shared path budget (ms_TickCountBeforePath);
//   6. drop the path if it was marked dirty (ResetPathProperties);
//   7. schedule the path of the highest-priority request (UpdatePathOfHigherPriorityRequest);
//   8. recompute the crowd priorities from the shared table once per frame;
//   9. ComputeDynamic: the whole speed / yaw / steering computation;
//  10. pitch and roll, the mesh update, the anim node's speed, then the face-to and look-at requests.
// DISHONORED(bringup): steps 3, 4 and 10 need FArkComponentAvoidance / FArkComponentMeshOffset /
// UArkAnimNodeLocomotion / FArkComponentFaceTo / FArkComponentLookat, none of which is ported; step 8
// (ComputePriorityOfNPCsFromSharedProps, 2013 rva 0x5491b0, 1,737 bytes) is the crowd give-way rule and is left out with
// its rva. What remains - 1, 2, 5, 6, 7, 9 - is the path and the dynamics, which is what makes the pawn move.
void FArkComponentLocomotion::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	if( GIsEditor && !GIsGame )
	{
		return;
	}
	if( m_bNoTick || _fTimeStep <= 0.0001f || !m_pConfig || !m_pPawnOwner )
	{
		return;
	}

	const UBOOL bHidden = m_pPawnOwner->bHidden ? TRUE : FALSE;
	if( ( m_bPrevHidden != 0 ) != ( bHidden != 0 ) )
	{
		if( bHidden )
		{
			OnDisable();
		}
		else
		{
			OnEnable();
		}
	}
	m_bPrevHidden = bHidden;

	if( ms_TickCountBeforePath > 0 )
	{
		--ms_TickCountBeforePath;
	}

	DISLOCO_MARK( tick_enter )
	if( m_bPathIsDirty )
	{
		ResetPathProperties();
	}

	m_CurPawnLocation = m_pPawnOwner->Location;

	if( m_ReqMgr.HasRequest() )
	{
		UpdatePathOfHigherPriorityRequest( _fTimeStep, FALSE, NULL );
	}

	// DISHONORED(port): retail's teleport branch, 2013 rva 0x54caa0. An NPC that the path scheduler found off the
	// navigation mesh is put back onto it; the pawn's own half-height is added because the mesh point is a floor point.
	if( m_bTeleportRequired && !m_bTeleportDisabled )
	{
		m_bTeleportRequired = FALSE;
		FVector Destination = m_TeleportLocation;
		if( m_pPawnOwner->CylinderComponent )
		{
			Destination.Z += m_pPawnOwner->CylinderComponent->CollisionHeight;
		}
		if( GWorld->FarMoveActor( m_pPawnOwner, Destination, FALSE, TRUE, FALSE ) )
		{
			GDisLocoTeleports++;
			// NOT HandleTeleport(): retail's teleport branch clears the bit and forces a recomputation, it does not reset the
			// path properties - and ResetPathProperties zeroes m_fTryToReturnOnNavMeshDuration, which is the clock the
			// give-up decision is measured against. Calling it here made the two chase each other: 191,404 teleports in 46
			// seconds, because every successful teleport put the clock back to zero.
			m_bForcePathComputation = TRUE;
			m_CurPawnLocation = m_pPawnOwner->Location;
		}
	}

	ComputeDynamic( _fTimeStep );

	m_MostRelevantSpeedIdx = ComputeMostRelevantSpeedIdx();
}

/*-----------------------------------------------------------------------------
	the request-manager callback
-----------------------------------------------------------------------------*/

void FArkComponentLocomotion::RequestManagerEventThunk( FArkComponentBase* _pOwner, const FArkRequestManager<FLocoRequestData>::EArkReqMgrEvent _Event, const INT _RequestIdx )
{
	( (FArkComponentLocomotion*)_pOwner )->OnRequestManagerEvent( _Event, _RequestIdx );
}

// DISHONORED(port): 2013 rva 0x546660 (2012 0x587350). Three of the four events matter: an update of the active request
// resets the "already told" flags so its events can fire again; a request about to be removed gets one last
// CPNT_LOCO_EVENT_ABORTED through its own callback; and a change of the first request re-initialises the follow
// properties, clears the work data and recomputes whether there is a computed path for it.
void FArkComponentLocomotion::OnRequestManagerEvent( const FArkRequestManager<FLocoRequestData>::EArkReqMgrEvent _Event, INT _RequestIdx )
{
	switch( _Event )
	{
	case FArkRequestManager<FLocoRequestData>::ARK_REQMGR_REQUEST_UPDATED:
		if( _RequestIdx == 0 )
		{
			ResetCurRequestWorkData();
		}
		break;

	case FArkRequestManager<FLocoRequestData>::ARK_REQMGR_REQUEST_WILL_BE_REMOVED:
		{
			const FArkRequestManager<FLocoRequestData>::FRequest& rRequest = m_ReqMgr.GetRequest( _RequestIdx );
			if( rRequest.m_Data.m_CommonProps.m_LocoEventCallback )
			{
				rRequest.m_Data.m_CommonProps.m_LocoEventCallback(
					rRequest.m_Data.m_CommonProps.m_pCallbackOwner, rRequest.m_ID, CPNT_LOCO_EVENT_ABORTED );
			}
		}
		break;

	case FArkRequestManager<FLocoRequestData>::ARK_REQMGR_FIRST_REQUEST_HAS_CHANGED:
		if( m_ReqMgr.HasRequest() )
		{
			// DISHONORED(bringup): InitFollowProperties (2013 rva 0x544780) belongs to arkcomponentlocomotionfollow.cpp,
			// which is the "stay behind this actor at an angle" mode; no behaviour in the tower uses it.
			m_fCurFollowAccelMultiplier = 1.f;
		}
		else
		{
			m_bTryToReturnOnNavMesh = FALSE;
			m_fTryToReturnOnNavMeshDuration = 0.f;
		}
		m_CurReqWorkData = FLocoRequestWorkData();
		m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
		m_bPathIsDirty = TRUE;
		break;

	default:
		break;
	}
}

// DISHONORED(port): 2013 rva 0x5451b0 (2012 0x5860c0)
void FArkComponentLocomotion::ResetCurRequestWorkData()
{
	m_CurReqWorkData = FLocoRequestWorkData();
	m_bHasRequestWithPathComputed = ComputeHasRequestWithPathComputedFlag();
}

/** The active request's asker gets one event. Retail raises them from HandleArrival and the path scheduler. */
void FArkComponentLocomotion::NotifyActiveRequest( BYTE _Event )
{
	if( !m_ReqMgr.HasRequest() )
	{
		return;
	}
	const FArkRequestManager<FLocoRequestData>::FRequest& rRequest = m_ReqMgr.GetFirstRequest();
	if( rRequest.m_Data.m_CommonProps.m_LocoEventCallback )
	{
		rRequest.m_Data.m_CommonProps.m_LocoEventCallback(
			rRequest.m_Data.m_CommonProps.m_pCallbackOwner, rRequest.m_ID, _Event );
	}
}
