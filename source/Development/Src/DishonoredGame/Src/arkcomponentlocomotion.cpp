// DishonoredGame/src/arkcomponentlocomotion.cpp
// DISHONORED(port): agent DN. The locomotion component's lifetime, its configuration and its per-frame entry point.
// Retail's unit is Engine/Src/arkcomponentlocomotion.cpp (24 functions, 8,111 bytes); it is here rather than in Engine
// for the reason the header's banner gives.

#include "DishonoredGame.h"
#include "arkcomponentpolicy.h"
#include "arkcomponentlocomotion.h"
#include "arkcomponentfaceto.h"
#include "icdiag.h"

#define DISLOCO_TRACE(n, fmt, ...) { static UBOOL bT##n = FALSE; if( !bT##n ) { bT##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n ": " fmt), __VA_ARGS__ ); } }
#define DISLOCO_MARK(n) { static UBOOL bM##n = FALSE; if( !bM##n ) { bM##n = TRUE; debugf( TEXT("DISHONORED(bringup): loco trace " #n) ); } }

#include "disdesirestructs.h"
#include "disaicensus.h"
#include "ibdiag.h"
#include "arkcomponentmeshoffset.h"

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
INT		GDisLocoFalls = 0;
INT		GDisLocoBased = 0;

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
// Agent HC: the component-manager insertion is retail's now (the PreAsyncWork policy, the policy GetPolicy's ICF-folded
// `mov eax,ecx` returns); ADishonoredNPCPawn::physWalking only moves the pawn. Retail also registers for the LOD game event here
// (FArkGameEventDispatcher::RegisterToObjectEvent with event 2 and OnLODChanged, 2013 rva 0x53ecf0): left out because
// FDisComponentLODManager is unported, so no LOD event is ever raised and the handler would never run.
void FArkComponentLocomotion::Starting()
{
	DISLOCO_MARK( starting_enter )
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Register( this );
	}
	m_ReqMgr.Initialize( this, &FArkComponentLocomotion::RequestManagerEventThunk );

	if( m_bDisabled && !m_bPendingStop )
	{
		m_bDisabled = FALSE;
		OnEnable();
	}

	// agent IC: 2012 0x58b750 starts the face-to component SetConfig created
	if( m_pFaceToCpnt )
	{
		m_pFaceToCpnt->m_bStarted = TRUE;
		m_pFaceToCpnt->Starting();
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
	if( GWorld && GWorld->m_pComponentManager )
	{
		GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().Unregister( this );
	}

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
// DISHONORED(bringup): FArkComponentFaceTo is unported, so m_pFaceToCpnt stays NULL and is read defensively everywhere
// below. The anim node is agent GC's port: m_bCanManageAnim is TRUE when the pawn's anim tree has a LocoBlender, which
// retail reads as "this NPC's walk is animated by the node" (an NPC whose tree has none runs with it FALSE, as in retail).
UBOOL FArkComponentLocomotion::SetConfig( const UArkComponentLocomotionConfig* const _pConfig )
{
	m_pPawnOwner = Cast<ADishonoredNPCPawn>( m_pOwner );
	if( !IsConfigValid( _pConfig, m_pPawnOwner ) )
	{
		return FALSE;
	}
	m_pConfig = _pConfig;
	m_pAnimNodeLoco = UArkAnimNodeLocomotion::GetAnimNodeLocomotion( m_pPawnOwner );
	m_bCanManageAnim = ( m_pAnimNodeLoco != NULL ) ? TRUE : FALSE;
	if( m_bCanManageAnim )
	{
		m_pAnimNodeLoco->Initialize( m_pConfig );
	}
	// agent IC: retail (2012 0x584010) adds the face-to component here and initialises it with this config.
	if( m_pPawnOwner->m_ComponentContainer )
	{
		m_pFaceToCpnt = m_pPawnOwner->m_ComponentContainer->AddNewComponent<FArkComponentFaceTo>();
		if( m_pFaceToCpnt )
		{
			m_pFaceToCpnt->Initialize( m_pConfig );
		}
	}
	SetModifier( m_CurModifierIdx );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x543110 (2012 0x5840b0)
FArkComponentLocomotion* FArkComponentLocomotion::GetLocomotionComponent( const APawn* _pPawn )
{
	const ADishonoredNPCPawn* NPCPawn = ConstCast<ADishonoredNPCPawn>( _pPawn );
	return NPCPawn ? (FArkComponentLocomotion*)NPCPawn->m_pCpntLocomotion : NULL;
}

// DISHONORED(port): agent IO, retail whole: 2013 VA 0x941b70 (2012 0x582b40, 702 bytes). The modifier's m_fAcceleration /
// m_fDeceleration are the distances over which the fastest speed mode is reached or left, so the rates are v / (d / (v / 2));
// m_MaxMultiDirSpeedIdx is the last speed of the modifier whose mode has more than one direction animation (-1: none);
// m_bHaveStopAnim says the modifier's idle mode has stop animations. Agents DN/GK copied the two distances as rates and never
// wrote the other two (GK: "m_bHaveStopAnim has no writer in either database" - this function writes it, both databases).
void FArkComponentLocomotion::SetModifier( INT _ModifierIdx )
{
	if( _ModifierIdx < m_pConfig->m_Modifiers.Num() )
	{
		m_CurModifierIdx = _ModifierIdx;
		const FLOAT fMaxSpeed = m_pConfig->GetForwardSpeedOfSpeedMode( m_pConfig->m_SpeedNames.Num() - 1, _ModifierIdx );
		if( fMaxSpeed > 0.f )
		{
			const FArkCpntLocoModifierProp& rModifier = m_pConfig->m_Modifiers( m_CurModifierIdx );
			m_fCurAcceleration = ( rModifier.m_fAcceleration != 0.f ) ? Abs( fMaxSpeed / ( rModifier.m_fAcceleration / ( fMaxSpeed * 0.5f ) ) ) : 0.f;
			const FLOAT fDeceleration = ( rModifier.m_fDeceleration != 0.f ) ? Abs( -fMaxSpeed / ( rModifier.m_fDeceleration / ( fMaxSpeed + 0.5f * -fMaxSpeed ) ) ) : 0.f;
			m_fCurDeceleration = fDeceleration;
			m_fCurAdaptedAccel = fDeceleration;
		}
		const INT SpeedNames = m_pConfig->m_SpeedNames.Num();
		m_MaxMultiDirSpeedIdx = -1;
		for( INT SpeedIdx = 1; SpeedIdx < SpeedNames; SpeedIdx++ )
		{
			if( m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( SpeedNames * m_CurModifierIdx + SpeedIdx ) ).m_MoveAnimProps.Num() <= 1 )
			{
				break;
			}
			m_MaxMultiDirSpeedIdx = SpeedIdx;
		}
		m_bHaveStopAnim = m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( m_CurModifierIdx * SpeedNames ) ).m_StopAnimProps.Num() > 0;
	}
	if( m_bCanManageAnim )
	{
		m_pAnimNodeLoco->SetModifier( m_CurModifierIdx );
	}
}

// DISHONORED(port): 2013 rva 0x53f9b0 (2012 0x580990): the dictionary is indexed by (modifier * speed count + speed), and
// -10 means "the modifier I am in".
const FArkCpntLocoSpeedModeProp& FArkComponentLocomotion::GetSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const
{
	const INT ModifierIdx = ( _ModifierIdx == -10 ) ? m_CurModifierIdx : _ModifierIdx;
	const INT FlatIdx = _SpeedIdx + ModifierIdx * m_pConfig->m_SpeedNames.Num();
	return m_pConfig->m_SpeedModePropDictionary( m_pConfig->m_SpeedModes( FlatIdx ) );
}

// DISHONORED(port): 2013 rva 0x53e9c0 (2012 0x57fc80): forwards to UArkComponentLocomotionConfig::GetForwardSpeedOfSpeedMode
// (agent GC's port, arkanimnodelocomotion.cpp: the forward direction animation's foot speed) with the flattened index.
// Agent DN's stand-in took the fastest of the mode's animations; the two agree on every single-direction mode.
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
	return m_pConfig->GetForwardSpeedOfSpeedMode( _SpeedIdx, ModifierIdx );
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

// DISHONORED(port): 2013 rva 0x53f090 (2012 0x5802a0, 206 bytes): the speed mode actually on screen - the turn's while only
// a turn animation plays, else the speed slot over half the weight in the node's most relevant modifier slot, else 0.
INT FArkComponentLocomotion::ComputeMostRelevantSpeedIdx() const
{
	if( !m_pConfig || m_bPendingStop || !m_pPawnOwner || m_pPawnOwner->bDeleteMe || !m_bCanManageAnim )
	{
		return 0;
	}
	const INT SpeedNames = m_pConfig->m_SpeedNames.Num();
	if( m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() )
	{
		if( m_pAnimNodeLoco->GetTypeOfCurSpecificAnim() == ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN )
		{
			return m_TurnProps.m_SpeedModeIdx % SpeedNames;
		}
		return 0;
	}
	const FAnimNodeLocoModifierSlot* pSlot = m_pAnimNodeLoco->GetMostRelevantModifierSlot();
	if( !m_pAnimNodeLoco->bRelevant || !pSlot || pSlot->m_Modifier == -1 )
	{
		return 0;
	}
	for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2; ++SpeedSlotIdx )
	{
		const FAnimNodeLocoSpeedSlot& rSpeedSlot = pSlot->m_SpeedSlots[SpeedSlotIdx];
		if( rSpeedSlot.m_SpeedModeIdx != -1 && rSpeedSlot.m_fWeight >= 0.5f )
		{
			return rSpeedSlot.m_SpeedModeIdx % SpeedNames;
		}
	}
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
	if( m_bCanManageAnim )
	{
		m_pAnimNodeLoco->SetSpeed( 0.f, m_bInertiaDisabled );
		m_pAnimNodeLoco->SetSpeedMultiplier( m_fLookatSpeedMultiplier );
	}
}

// DISHONORED(port): 2013 rva 0x53dea0 (2012 0x57eff0, 69 bytes): the anim node is told the pawn's real scaled speed (zero
// smoothing when inertia is off) and the look-at speed multiplier. Retail's PreAsyncWorkTick passes it |Velocity.XY
// normalised x m_fRealScaledSpeed|, which is m_fRealScaledSpeed whenever the pawn moves at all and 0 when it does not.
// The node's strafe angle is FArkComponentFaceTo's to set (2012 0x57af4e), not this component's: unported, so a walking
// NPC always plays his forward animation, which is right for a pawn whose body follows his path (ComputeDynamic's yaw).
void FArkComponentLocomotion::UpdateAnimNode( INT _ModifierIdx, FLOAT _fSpeed )
{
	if( m_bCanManageAnim )
	{
		m_pAnimNodeLoco->SetSpeed( _fSpeed, m_bInertiaDisabled );
		m_pAnimNodeLoco->SetSpeedMultiplier( m_fLookatSpeedMultiplier );
	}
}

// DISHONORED(port): 2013 rva 0x5461f0 (2012 0x586ef0): the three root-motion state machines are reset
// (arkcomponentlocomotionrootmove.cpp, agent GK).
void FArkComponentLocomotion::OnDisable()
{
	ResetTurn();
	ResetStartMove();
	ResetStopMove();
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
	if( m_bCanManageAnim )
	{
		m_pAnimNodeLoco->AbortSpecificAnimRequest( TRUE );
	}
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
	IBDiagCount( IBDIAG_TICK );

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
	// DISHONORED(port, agent IK2): retail tests +252 bit 0x40 here (2012 0x58d5f0), the bit ForceActiveRequestRepath / OnEnable /
	// HandleTeleport raise (m_bForcePathComputation); HEAD tested 0x8, the scheduler's own repath bit.
	if( m_bForcePathComputation )
	{
		ResetPathProperties();
	}

	m_CurPawnLocation = m_pPawnOwner->Location;

	if( m_ReqMgr.HasRequest() )
	{
		UpdatePathOfHigherPriorityRequest( _fTimeStep, FALSE, NULL );
	}

	// DISHONORED(port): retail's teleport branch, 2013 rva 0x54caa0 / 2012 0x58d5f0: the scheduler already added the pawn's
	// half-height to m_TeleportLocation (agent IK2); the bit clears only when the move happened.
	// DISHONORED(bringup): retail runs this after ComputeDynamic, outside the STATE 2 / 10 physics, and only once
	// CanBeTeleportedToLocationWithoutBeingSeen (2012 0x57f460) agrees; that test is unported.
	if( m_bTeleportRequired && !m_bTeleportDisabled )
	{
		const FVector Destination = m_TeleportLocation;
		const FVector Before = m_pPawnOwner->Location;
		const UBOOL bMoved = GWorld->FarMoveActor( m_pPawnOwner, Destination, FALSE, TRUE, FALSE );
		if( DisGCLocoDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): gadiag teleport %s from %s to %s -> %d, now at %s base %s"), *m_pPawnOwner->GetName(),
				*Before.ToString(), *Destination.ToString(), (INT)bMoved, *m_pPawnOwner->Location.ToString(), m_pPawnOwner->Base ? *m_pPawnOwner->Base->GetName() : TEXT("none") );
		}
		if( bMoved )
		{
			GDisLocoTeleports++;
			m_bTeleportRequired = FALSE;
			m_CurPawnLocation = m_pPawnOwner->Location;
		}
	}

	IKDiagFalling();
	// DISHONORED(port, agent IO): 2013 VA 0x94caa0: the pawn's velocity before the dynamics write this frame's is what
	// UpdateFaceTo hands the face-to component as the real velocity.
	const FVector VelocityBeforeDynamic = m_pPawnOwner->Velocity;
	ComputeDynamic( _fTimeStep );

	// DISHONORED(port, agent IO): 2013 VA 0x94caa0: unless the pawn is a rigid body, the body leans into its turns - the
	// target pitch / roll from the real speed, the move rotation speed and the face-to angle, smoothed, then written to the
	// mesh-offset component's external rotation with yaw 0 (UpdateMesh).
	if( m_pPawnOwner->Physics != PHYS_RigidBody )
	{
		ComputeTargetPitchAndRoll( m_fRealSpeed, m_fCurMoveRotationSpeed, m_pFaceToCpnt ? m_pFaceToCpnt->GetAngle() : 0, m_fTargetPitch, m_fTargetRoll );
		UpdatePitchAndRoll( _fTimeStep );
		UpdateMesh( (INT)m_fCurPitch, (INT)m_fCurRoll, m_fCurPitchRollZOffset );
	}

	// DISHONORED(port): retail's step 10 (2012 0x58d9af..0x58dae2): the pawn's planar velocity direction scaled by
	// m_fRealScaledSpeed, its length handed to the node; then UpdateFaceTo (agent IO). UpdateLookat stays out.
	const FVector PlanarVelocity( m_pPawnOwner->Velocity.X, m_pPawnOwner->Velocity.Y, 0.f );
	const FVector RealVelocity = PlanarVelocity.SafeNormal() * m_fRealScaledSpeed;
	UpdateAnimNode( m_CurModifierIdx, RealVelocity.Size2D() );
	if( m_pFaceToCpnt )
	{
		UpdateFaceTo( _fTimeStep, VelocityBeforeDynamic );
	}
	IODiagReport();

	m_MostRelevantSpeedIdx = ComputeMostRelevantSpeedIdx();

	if( m_bCanManageAnim && DisGCLocoDiagEnabled() )
	{
		static TMap<const UObject*, FLOAT> LastReportAt;
		const FLOAT Now = GWorld->GetTimeSeconds();
		FLOAT* Last = LastReportAt.Find( m_pPawnOwner );
		if( !Last || Now - *Last >= 1.f )
		{
			LastReportAt.Set( m_pPawnOwner, Now );
			debugf( TEXT("DISHONORED(bringup): gadiag loconode %s vel %.1f realscaled %.1f curmove %.1f mostrelevant %d: %s"),
				*m_pPawnOwner->GetName(), m_pPawnOwner->Velocity.Size2D(), m_fRealScaledSpeed, m_fCurMoveSpeed, m_MostRelevantSpeedIdx, *m_pAnimNodeLoco->DescribeForDiag() );
		}
	}
}

// DISHONORED(port): agent IO, 2013 VA 0x943190 (2012 0x584130, 753 B). The locomotion turns the body through its own face-to
// request: while the pawn walks a computed path under its own control (not disabled, not being deleted, the face-to component
// not in root motion, a velocity and a path move this frame) it keeps one yaw request at the config's m_FaceToPriority towards
// the path move, at the move's maximum rotation speed (x 1.5 x the wall-test multiplier unless that multiplier is the
// emergency one; never below m_fFaceToDefaultSpeed), keep-snapped, accelerating at m_fRotationAccelMultiplier x that speed;
// otherwise the request is stopped. A specific animation hands the body to root motion; the body snaps onto the movement when
// the face-to angle is 0 and the pawn is faster than the multi-directional speed for that angle.
void FArkComponentLocomotion::UpdateFaceTo( FLOAT _fTimeStep, const FVector& _rRealVelocity )
{
	const UBOOL bDeleteMe = m_pPawnOwner->bDeleteMe;
	if( m_bDisabled || bDeleteMe )
	{
		m_pFaceToCpnt->AllowRootMotion( TRUE );
	}
	else if( m_bCanManageAnim && m_pAnimNodeLoco->IsSpecificAnimPlaying() )
	{
		m_pFaceToCpnt->AllowRootMotion( FALSE );
	}
	else
	{
		m_pFaceToCpnt->DisableRootMotion();
	}

	if( m_bDisabled || bDeleteMe || m_pFaceToCpnt->IsRootMotion() || !m_bHasRequestWithPathComputed || m_pPawnOwner->Velocity.IsZero() || m_CurPathMove.IsZero() )
	{
		if( m_FaceToRequestID != INDEX_NONE )
		{
			m_pFaceToCpnt->StopFaceTo( m_FaceToRequestID );
			m_FaceToRequestID = INDEX_NONE;
		}
	}
	else
	{
		FLOAT fRotationSpeed = m_fMaxMoveRotationSpeed;
		if( Abs( m_fMaxMoveRotSpeedMultiplier - m_pConfig->m_fEmergencyRotationSpeedMultiplier ) >= 0.01f )
		{
			fRotationSpeed = m_fMaxMoveRotSpeedMultiplier * 1.5f * fRotationSpeed;
		}
		if( fRotationSpeed < m_pConfig->m_fFaceToDefaultSpeed )
		{
			fRotationSpeed = m_pConfig->m_fFaceToDefaultSpeed;
		}
		FArkCpntFaceToYawProp FaceToProps;
		FaceToProps.InitAllByDefaultExcept( appTrunc( appAtan2( m_CurPathMove.Y, m_CurPathMove.X ) * 10430.219f * 2.f + 0.5f ) >> 1, fRotationSpeed, NULL, NULL );
		FaceToProps.m_CommonProps.m_fAcceleration = m_pConfig->m_fRotationAccelMultiplier * fRotationSpeed;
		FaceToProps.m_CommonProps.m_bKeepSnapped = TRUE;
		if( m_FaceToRequestID == INDEX_NONE )
		{
			m_FaceToRequestID = m_pFaceToCpnt->StartFaceToYaw( this, GetInternalName(), m_pConfig->m_FaceToPriority, FaceToProps );
		}
		else
		{
			m_pFaceToCpnt->UpdateFaceToYaw( m_FaceToRequestID, FaceToProps );
		}
	}

	UBOOL bForceBodySnap = FALSE;
	if( !m_bDisabled && !bDeleteMe && m_RootState != CPNT_LOCO_ROOT_STATE_STOP_END
		&& ( !m_bCanManageAnim || !m_pAnimNodeLoco->IsPlayingOnlySpecificAnim() ) && m_pFaceToCpnt->GetAngle() == 0 )
	{
		bForceBodySnap = ( m_pPawnOwner->Velocity.Size2D() / GetFinalSpeedMultiplier() ) > m_fMaxMultiDirSpeedForAngle;
	}
	m_pFaceToCpnt->ForceBodySnapOnMovement( bForceBodySnap );
	m_pFaceToCpnt->SetRealVelocity( _rRealVelocity );
}

// DISHONORED(port): agent IO, 2013 VA 0x93e9e0 (2012 0x57fca0): the lean into a turn - up to m_fMaxRollAngle, scaled by the speed
// against max(fastest forward speed, m_fSpeedToApplyMaxRoll) and the rotation speed against m_fRotationSpeedToApplyMaxRoll,
// split between pitch and roll by the face-to angle; none while disabled, deleted, playing a specific animation or not turning.
void FArkComponentLocomotion::ComputeTargetPitchAndRoll( FLOAT _fSpeed, FLOAT _fRotSpeed, INT _Angle, FLOAT& _rTargetPitch, FLOAT& _rTargetRoll ) const
{
	if( m_bDisabled || m_pPawnOwner->bDeleteMe || ( m_bCanManageAnim && m_pAnimNodeLoco->IsSpecificAnimPlaying() )
		|| Abs( _fSpeed ) < 0.1f || Abs( _fRotSpeed ) < 0.1f )
	{
		_rTargetPitch = 0.f;
		_rTargetRoll = 0.f;
		return;
	}
	FLOAT fSpeedToApplyMaxRoll = m_pConfig->GetForwardSpeedOfSpeedMode( m_pConfig->m_SpeedNames.Num() - 1, m_CurModifierIdx );
	if( fSpeedToApplyMaxRoll < m_pConfig->m_fSpeedToApplyMaxRoll )
	{
		fSpeedToApplyMaxRoll = m_pConfig->m_fSpeedToApplyMaxRoll;
	}
	const FLOAT fSpeedRatio = Min( _fSpeed / fSpeedToApplyMaxRoll, 1.f );
	const FLOAT fRotRatio = Clamp( ( _fRotSpeed * 0.0054931641f ) / m_pConfig->m_fRotationSpeedToApplyMaxRoll, -1.f, 1.f );
	const FLOAT fRoll = (FLOAT)(INT)( m_pConfig->m_fMaxRollAngle * fRotRatio * fSpeedRatio * 182.04445f );
	const FLOAT fAngle = (FLOAT)_Angle * 0.000095873802f;
	_rTargetPitch = appSin( fAngle ) * fRoll;
	_rTargetRoll = fRoll * appCos( fAngle );
}

// DISHONORED(port): agent IO, 2013 VA 0x93eb30 (2012 0x57fde0): pitch and roll move towards their targets at m_fRollSpeed in
// 0.04 s sub-steps; the larger of the two lifts the mesh by (1 - cos) x the cylinder radius.
void FArkComponentLocomotion::UpdatePitchAndRoll( FLOAT _fTimeStep )
{
	FLOAT fTimeLeft = _fTimeStep;
	do
	{
		const FLOAT fStep = Min( 0.04f, fTimeLeft );
		const FLOAT fRollSpeed = m_pConfig->m_fRollSpeed * 182.04445f;
		m_fCurPitch = ArkSmoothFixAngleRotation( m_fCurPitch, m_fTargetPitch, fRollSpeed, fRollSpeed, fStep, m_fCurPitchRotateSpeed );
		m_fCurRoll = ArkSmoothFixAngleRotation( m_fCurRoll, m_fTargetRoll, fRollSpeed, fRollSpeed, fStep, m_fCurRollRotateSpeed );
		fTimeLeft -= 0.04f;
	}
	while( fTimeLeft > 0.f );
	const INT Pitch = (INT)(WORD)( (INT)m_fCurPitch + 0x7FFF ) - 0x7FFF;
	const INT Roll = (INT)(WORD)( (INT)m_fCurRoll + 0x7FFF ) - 0x7FFF;
	m_fCurPitchRollZOffset = ( 1.f - appCos( (FLOAT)Max( Pitch, Roll ) * 0.000095873802f ) ) * m_pPawnOwner->CylinderComponent->CollisionRadius;
}

// DISHONORED(port): agent IO, 2013 VA 0x93de20 (2012 0x57ef70): the lean goes to the mesh-offset component's external rotation
// with yaw 0, or straight to the mesh without one. Retail never reads _fZOffset here.
void FArkComponentLocomotion::UpdateMesh( INT _Pitch, INT _Roll, FLOAT _fZOffset )
{
	if( !m_pMeshOffsetCpnt )
	{
		m_pMeshOffsetCpnt = FArkComponentMeshOffset::GetMeshOffsetComponent( m_pPawnOwner, TRUE );
	}
	if( m_pMeshOffsetCpnt )
	{
		m_pMeshOffsetCpnt->SetExternalRotationOffset( FRotator( _Pitch, 0, _Roll ) );
	}
	else
	{
		m_pPawnOwner->Mesh->Rotation = FRotator( _Pitch, 0, _Roll );
	}
}

/*-----------------------------------------------------------------------------
	-iodiag (agent IO)
-----------------------------------------------------------------------------*/

struct FIODiagState
{
	FLOAT	NextReport;
	INT		PathPoints;
	FVector	LastPathPoint;
	FIODiagState() : NextReport( 0.f ), PathPoints( -1 ), LastPathPoint( 0.f, 0.f, 0.f ) {}
};
static TMap<const FArkComponentLocomotion*, FIODiagState> GIODiagStates;

static INT IODiagYaw( FLOAT _fX, FLOAT _fY )
{
	return appRound( appAtan2( _fY, _fX ) * ( 32768.f / PI ) ) & 0xFFFF;
}

void FArkComponentLocomotion::IODiagReport() const
{
	if( !DisIODiagEnabled() || !GWorld )
	{
		return;
	}
	FIODiagState& State = GIODiagStates.FindOrAdd( this );
	if( m_bHasRequestWithPathComputed && m_PathPoints.Num() > 0 && ( State.PathPoints != m_PathPoints.Num() || State.LastPathPoint != m_LastPathPoint ) )
	{
		State.PathPoints = m_PathPoints.Num();
		State.LastPathPoint = m_LastPathPoint;
		FString Points;
		for( INT PointIdx = 0; PointIdx < m_PathPoints.Num(); PointIdx++ )
		{
			Points += FString::Printf( TEXT(" %.0f/%.0f/%.0f"), m_PathPoints( PointIdx ).m_Point.X, m_PathPoints( PointIdx ).m_Point.Y, m_PathPoints( PointIdx ).m_Point.Z );
		}
		debugf( TEXT("DISHONORED(bringup): iodiag path %s world %.2f from %.0f/%.0f/%.0f points %d:%s"), *m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(),
			m_pPawnOwner->Location.X, m_pPawnOwner->Location.Y, m_pPawnOwner->Location.Z, m_PathPoints.Num(), *Points );
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FLOAT fSpeed = m_pPawnOwner->Velocity.Size2D();
	if( Now < State.NextReport || ( fSpeed < 1.f && !m_bHasRequestWithPathComputed ) )
	{
		return;
	}
	State.NextReport = Now + 1.f;
	const USkeletalMeshComponent* pMesh = m_pPawnOwner->Mesh;
	const INT VelYaw = fSpeed > 1.f ? IODiagYaw( m_pPawnOwner->Velocity.X, m_pPawnOwner->Velocity.Y ) : -1;
	const INT PathMoveYaw = m_CurPathMove.IsZero() ? -1 : IODiagYaw( m_CurPathMove.X, m_CurPathMove.Y );
	const INT BodyYaw = m_pPawnOwner->Rotation.Yaw & 0xFFFF;
	const INT Delta = VelYaw >= 0 ? Abs( (INT)(SWORD)(WORD)( BodyYaw - VelYaw ) ) : -1;
	const FString Motion = FString::Printf( TEXT("rot %d vel %d delta %.1f pathmove %d curmove %d target %d rotspeed %.0f max %.1f mult %.2f multidir %.1f"),
		BodyYaw, VelYaw, Delta >= 0 ? Delta * 0.0054931641f : -1.f, PathMoveYaw, m_CurMoveYaw, m_TargetMoveYaw,
		m_fCurMoveRotationSpeed, m_fMaxMoveRotationSpeed, m_fMaxMoveRotSpeedMultiplier, m_fMaxMultiDirSpeedForAngle );
	const FString FaceTo = FString::Printf( TEXT("faceto req %d id %d prio %d delta %d angle %d target %d rm %d"),
		m_pFaceToCpnt ? m_pFaceToCpnt->GetRequestsCount() : -1, m_FaceToRequestID, m_pFaceToCpnt ? m_pFaceToCpnt->GetActiveRequestPriority() : -1,
		m_pFaceToCpnt ? m_pFaceToCpnt->GetCurBodyYawDelta() : 0, m_pFaceToCpnt ? m_pFaceToCpnt->GetAngle() : 0, m_pFaceToCpnt ? m_pFaceToCpnt->GetTargetYaw() & 0xFFFF : -1,
		m_pFaceToCpnt ? (INT)m_pFaceToCpnt->IsRootMotion() : -1 );
	debugf( TEXT("DISHONORED(bringup): iodiag yaw %s world %.2f at %.0f/%.0f/%.0f phys %d speed %.1f start %.1f | %s | %s | mesh %d/%d/%d rmm %d rmrm %d | root %d path %d/%d"),
		*m_pPawnOwner->GetName(), Now, m_pPawnOwner->Location.X, m_pPawnOwner->Location.Y, m_pPawnOwner->Location.Z, (INT)m_pPawnOwner->Physics, fSpeed,
		m_pConfig ? m_pConfig->m_fVelocityToStartToMove : 0.f, *Motion, *FaceTo,
		pMesh ? pMesh->Rotation.Pitch : 0, pMesh ? pMesh->Rotation.Yaw : 0, pMesh ? pMesh->Rotation.Roll : 0, pMesh ? (INT)pMesh->RootMotionMode : -1,
		pMesh ? (INT)pMesh->RootMotionRotationMode : -1, (INT)m_RootState, m_CurPathPointIdx, m_PathPoints.Num() );
}

/*-----------------------------------------------------------------------------
	-ibdiag (agent IB)
-----------------------------------------------------------------------------*/

struct FIBDiagCounts
{
	INT Counts[FArkComponentLocomotion::IBDIAG_MAX];
	FIBDiagCounts() { appMemzero( Counts, sizeof( Counts ) ); }
};
static TMap<const FArkComponentLocomotion*, FIBDiagCounts> GIBDiagCounts;

void FArkComponentLocomotion::IBDiagCount( EIBDiagCounter _Counter ) const
{
	if( DisIBDiagEnabled() && DisIBDiagWatched( m_pPawnOwner ) )
	{
		FIBDiagCounts* pCounts = GIBDiagCounts.Find( this );
		if( !pCounts )
		{
			pCounts = &GIBDiagCounts.Set( this, FIBDiagCounts() );
		}
		pCounts->Counts[_Counter]++;
	}
}

void FArkComponentLocomotion::IBDiagCheckStuck() const
{
	if( m_bStuckedInAWall && m_pPawnOwner && m_pConfig )
	{
		IBDiagCount( IBDIAG_STUCK );
		if( m_pPawnOwner->Velocity.Size2D() > m_pConfig->m_fVelocityToStartToMove )
		{
			IBDiagCount( IBDIAG_STUCK_FAST );
		}
	}
}

FString FArkComponentLocomotion::DescribeForIBDiag() const
{
	FIBDiagCounts Counts;
	if( FIBDiagCounts* pCounts = GIBDiagCounts.Find( this ) )
	{
		Counts = *pCounts;
		GIBDiagCounts.Remove( this );
	}
	const UBOOL bInPolicy = GWorld && GWorld->m_pComponentManager
		&& GWorld->m_pComponentManager->GetPolicy<FArkComponentPreAsyncWorkPolicy>().IsRegistered( const_cast<FArkComponentLocomotion*>( this ) );
	return FString::Printf( TEXT("started %d inpolicy %d disabled %d hidden %d root %d stuck %d (frames %d fast %d) curyaw %d real %.1f vel2d %.1f req %d arrived %d pathend %.0f path %d/%d pf %d curmove %.1f target %.1f mod %d ticks %d moves %d"),
		(INT)m_bStarted, (INT)bInPolicy, (INT)IsDisabled(), (INT)m_bPrevHidden, (INT)m_RootState, (INT)m_bStuckedInAWall,
		Counts.Counts[IBDIAG_STUCK], Counts.Counts[IBDIAG_STUCK_FAST], m_CurMoveYaw, m_fRealSpeed, m_pPawnOwner ? m_pPawnOwner->Velocity.Size2D() : 0.f,
		GetRequestsCount(), (INT)m_bIsArrived, appSqrt( m_fSq2DDistToPathEnd ), m_CurPathPointIdx, m_PathPoints.Num(), (INT)m_LastPFError,
		m_fCurMoveSpeed, m_fTargetMoveSpeed, m_CurModifierIdx, Counts.Counts[IBDIAG_TICK], Counts.Counts[IBDIAG_MOVE] );
}

/*-----------------------------------------------------------------------------
	-ikdiag (agent IK)
-----------------------------------------------------------------------------*/

void FArkComponentLocomotion::IKDiagDynamic( UBOOL _bAnimationOwnsThePawn, const FVector& _RootMotionRead ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	USkeletalMeshComponent* pMesh = m_pPawnOwner->Mesh;
	FString Specific( TEXT("none") );
	if( m_pAnimNodeLoco && m_pAnimNodeLoco->Children.Num() > 8 )
	{
		const UAnimNodeSequence* pSequence = Cast<UAnimNodeSequence>( m_pAnimNodeLoco->Children( 8 ).Anim );
		if( pSequence && pSequence->AnimSeqName != NAME_None )
		{
			Specific = FString::Printf( TEXT("%s w %.2f t %.3f prev %.3f rate %.2f play %d rbo %d%d%d rro %d%d%d edge %d"), *pSequence->AnimSeqName.ToString(), m_pAnimNodeLoco->Children( 8 ).Weight,
				pSequence->CurrentTime, pSequence->PreviousTime, pSequence->Rate, (INT)pSequence->bPlaying, pSequence->RootBoneOption[0], pSequence->RootBoneOption[1], pSequence->RootBoneOption[2],
				pSequence->RootRotationOption[0], pSequence->RootRotationOption[1], pSequence->RootRotationOption[2], pSequence->AnimSeq ? (INT)( pSequence->AnimSeq->RotationCompressionFormat == ACF_EdgeAnim ) : -1 );
		}
	}
	const FString Motion = FString::Printf( TEXT("rmread %.2f/%.2f/%.2f rmm %d rmv %.1f/%.1f/%.1f curmove %.2f/%.2f/%.2f vel %.1f/%.1f/%.1f"),
		_RootMotionRead.X, _RootMotionRead.Y, _RootMotionRead.Z, pMesh ? (INT)pMesh->RootMotionMode : -1,
		pMesh ? pMesh->RootMotionVelocity.X : 0.f, pMesh ? pMesh->RootMotionVelocity.Y : 0.f, pMesh ? pMesh->RootMotionVelocity.Z : 0.f,
		m_CurMove.X, m_CurMove.Y, m_CurMove.Z, m_pPawnOwner->Velocity.X, m_pPawnOwner->Velocity.Y, m_pPawnOwner->Velocity.Z );
	debugf( TEXT("DISHONORED(bringup): ikdiag dyn %s world %.3f frame %u root %d road %s %s speed %.1f/%.1f yaw %d/%d pathyaw %d return %d/%.2f been %d computed %d nearest %.0f/%.0f/%.0f at %.1f/%.1f/%.1f base %s | spec %s"),
		*m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(), (DWORD)GFrameCounter, (INT)m_RootState, _bAnimationOwnsThePawn ? TEXT("anim") : TEXT("full"), *Motion,
		m_fCurMoveSpeed, m_fTargetMoveSpeed, m_CurMoveYaw, m_TargetMoveYaw, m_CurPathYaw, (INT)m_bTryToReturnOnNavMesh, m_fTryToReturnOnNavMeshDuration,
		(INT)m_bHasBeenOnNavMesh, (INT)m_bHasRequestWithPathComputed, m_NearestLocationOnNavMesh.X, m_NearestLocationOnNavMesh.Y, m_NearestLocationOnNavMesh.Z,
		m_pPawnOwner->Location.X, m_pPawnOwner->Location.Y, m_pPawnOwner->Location.Z, m_pPawnOwner->Base ? *m_pPawnOwner->Base->GetName() : TEXT("none"), *Specific );
}

void FArkComponentLocomotion::IKDiagMove( const FVector& _Move, const FCheckResult& _Hit, const FVector& _Actual ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	const UPrimitiveComponent* pComponent = _Hit.Component;
	debugf( TEXT("DISHONORED(bringup): ikdiag move %s world %.3f frame %u root %d move %.2f/%.2f/%.2f actual %.2f/%.2f/%.2f time %.3f hit %s comp %s %s normal %.2f/%.2f/%.2f at %.1f/%.1f/%.1f penetrating %d base %s"),
		*m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(), (DWORD)GFrameCounter, (INT)m_RootState, _Move.X, _Move.Y, _Move.Z, _Actual.X, _Actual.Y, _Actual.Z, _Hit.Time,
		_Hit.Actor ? *_Hit.Actor->GetName() : TEXT("none"), pComponent ? *pComponent->GetClass()->GetName() : TEXT("-"), pComponent ? *pComponent->GetName() : TEXT("-"),
		_Hit.Normal.X, _Hit.Normal.Y, _Hit.Normal.Z, _Hit.Location.X, _Hit.Location.Y, _Hit.Location.Z, (INT)_Hit.bStartPenetrating,
		m_pPawnOwner->Base ? *m_pPawnOwner->Base->GetName() : TEXT("none") );
	static TArray<const UPrimitiveComponent*> Described;
	if( pComponent && _Hit.Actor && !Described.ContainsItem( pComponent ) )
	{
		Described.AddItem( pComponent );
		const USkeletalMeshComponent* pSkeletal = ConstCast<USkeletalMeshComponent>( pComponent );
		const UStaticMeshComponent* pStatic = ConstCast<UStaticMeshComponent>( pComponent );
		debugf( TEXT("DISHONORED(bringup): ikdiag blocker %s comp %s collide %d block %d zero %d nonzero %d rigid %d | actor collide %d block %d physics %d | %s %s bounds %s +- %s"),
			*_Hit.Actor->GetName(), *pComponent->GetName(), (INT)pComponent->CollideActors, (INT)pComponent->BlockActors, (INT)pComponent->BlockZeroExtent, (INT)pComponent->BlockNonZeroExtent,
			(INT)pComponent->BlockRigidBody, (INT)_Hit.Actor->bCollideActors, (INT)_Hit.Actor->bBlockActors, (INT)_Hit.Actor->Physics,
			pSkeletal ? TEXT("physicsasset") : ( pStatic ? TEXT("staticmesh") : TEXT("other") ),
			pSkeletal ? ( pSkeletal->PhysicsAsset ? *pSkeletal->PhysicsAsset->GetPathName() : TEXT("none") ) : ( pStatic && pStatic->StaticMesh ? *pStatic->StaticMesh->GetPathName() : TEXT("-") ),
			*pComponent->Bounds.Origin.ToString(), *pComponent->Bounds.BoxExtent.ToString() );
	}
}

void FArkComponentLocomotion::IKDiagStartPoly( const FVector& _StartOnGround, UBOOL _bFound, UBOOL _bTeleport ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	FCheckResult Hit( 1.f );
	const FVector Start = _StartOnGround + FVector( 0.f, 0.f, 10.f );
	const FVector End = _StartOnGround - FVector( 0.f, 0.f, 200.f );
	GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, Start, TRACE_AllBlocking | TRACE_Visible, FVector( 0.f, 0.f, 0.f ) );
	debugf( TEXT("DISHONORED(bringup): ikdiag startpoly %s world %.3f off the mesh: ground %s (trace hit %s at z %.1f) nearest %s found %d teleport %d been %d disabled %d return %.2f"),
		*m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(), *_StartOnGround.ToString(), Hit.Actor ? *Hit.Actor->GetName() : TEXT("none"), Hit.Location.Z,
		*m_NearestLocationOnNavMesh.ToString(), (INT)_bFound, (INT)_bTeleport, (INT)m_bHasBeenOnNavMesh, (INT)m_bTeleportDisabled, m_fTryToReturnOnNavMeshDuration );
}

/** One line per change of the scheduler's exit (reason or error state), so a pawn waiting in one state logs once. */
void FArkComponentLocomotion::IKDiagScheduler( const TCHAR* _Reason, const FVector& _Destination ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	static TMap<const FArkComponentLocomotion*, FString> LastExit;
	const FString Exit = FString::Printf( TEXT("%s | error %d"), _Reason, m_LastPFError );
	const FString* pLast = LastExit.Find( this );
	if( pLast && *pLast == Exit )
	{
		return;
	}
	LastExit.Set( this, Exit );
	FString Points;
	for( INT PointIdx = 0; PointIdx < m_PathPoints.Num() && PointIdx < 24; ++PointIdx )
	{
		const FVector& Point = m_PathPoints( PointIdx ).m_Point;
		Points += FString::Printf( TEXT(" %.0f/%.0f/%.0f"), Point.X, Point.Y, Point.Z );
	}
	debugf( TEXT("DISHONORED(bringup): ikdiag sched %s world %.3f %s | uptodate %d built %d dirty %d return %d/%.2f been %d path %d/%d budget %d | destination %s start %s at %s base %s | points%s"),
		*m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(), *Exit, (INT)m_bPathfindingUpToDate, (INT)m_bBuiltPathUpToDate, (INT)m_bPathIsDirty,
		(INT)m_bTryToReturnOnNavMesh, m_fTryToReturnOnNavMeshDuration, (INT)m_bHasBeenOnNavMesh, m_CurPathPointIdx, m_PathPoints.Num(), ms_TickCountBeforePath,
		*_Destination.ToString(), *m_StartLocationUsedToAskPath.ToString(), *m_pPawnOwner->Location.ToString(), m_pPawnOwner->Base ? *m_pPawnOwner->Base->GetName() : TEXT("none"), *Points );
}

/** The floor sweep of every FullMovePawn of a watched NPC: its outcome, what it hit and the delta it left. */
void FArkComponentLocomotion::IKDiagFloor( const TCHAR* _Outcome, const FCheckResult& _FloorHit, const FVector& _Delta ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): ikdiag floor %s world %.3f %s | hit %s comp %s %s time %.3f normal %.2f/%.2f/%.2f at %.1f/%.1f/%.1f stepupon %d | delta %.2f/%.2f/%.2f at %.1f/%.1f/%.1f"),
		*m_pPawnOwner->GetName(), GWorld->GetTimeSeconds(), _Outcome, _FloorHit.Actor ? *_FloorHit.Actor->GetName() : TEXT("none"),
		_FloorHit.Component ? *_FloorHit.Component->GetClass()->GetName() : TEXT("-"), _FloorHit.Component ? *_FloorHit.Component->GetName() : TEXT("-"),
		_FloorHit.Time, _FloorHit.Normal.X, _FloorHit.Normal.Y, _FloorHit.Normal.Z, _FloorHit.Location.X, _FloorHit.Location.Y, _FloorHit.Location.Z,
		_FloorHit.Actor ? (INT)_FloorHit.Actor->bCanStepUpOn : -1, _Delta.X, _Delta.Y, _Delta.Z, m_pPawnOwner->Location.X, m_pPawnOwner->Location.Y, m_pPawnOwner->Location.Z );
}

static FString IKDiagDescribeHit( const FCheckResult& _Hit )
{
	if( !_Hit.Actor && !_Hit.Component )
	{
		return FString( TEXT("nothing") );
	}
	const UPrimitiveComponent* pComponent = _Hit.Component;
	FString Mesh( TEXT("-") );
	if( const UStaticMeshComponent* pStatic = ConstCast<UStaticMeshComponent>( pComponent ) )
	{
		Mesh = pStatic->StaticMesh ? pStatic->StaticMesh->GetPathName() : FString( TEXT("none") );
	}
	const FDisPrimTraceMask* pMask = pComponent ? &pComponent->m_CollisionTraceTypes : NULL;
	return FString::Printf( TEXT("%s (%s) comp %s %s mesh %s at z %.1f normal %.2f/%.2f/%.2f time %.3f | mask nonpawn %d npc %d player %d | collide %d block %d zero %d nonzero %d | actor collide %d block %d worldgeo %d"),
		_Hit.Actor ? *_Hit.Actor->GetName() : TEXT("none"), _Hit.Actor ? *_Hit.Actor->GetClass()->GetName() : TEXT("-"),
		pComponent ? *pComponent->GetClass()->GetName() : TEXT("-"), pComponent ? *pComponent->GetName() : TEXT("-"), *Mesh,
		_Hit.Location.Z, _Hit.Normal.X, _Hit.Normal.Y, _Hit.Normal.Z, _Hit.Time,
		pMask ? (INT)pMask->m_bTraceForMove_NonPawn : -1, pMask ? (INT)pMask->m_bTraceForMove_NonPlayerPawn : -1, pMask ? (INT)pMask->m_bTraceForMove_Player : -1,
		pComponent ? (INT)pComponent->CollideActors : -1, pComponent ? (INT)pComponent->BlockActors : -1, pComponent ? (INT)pComponent->BlockZeroExtent : -1,
		pComponent ? (INT)pComponent->BlockNonZeroExtent : -1, _Hit.Actor ? (INT)_Hit.Actor->bCollideActors : -1, _Hit.Actor ? (INT)_Hit.Actor->bBlockActors : -1,
		_Hit.Actor ? (INT)_Hit.Actor->bWorldGeometry : -1 );
}

/** At a failed floor sweep: what is under the pawn for other trace kinds (twice a second per pawn). */
void FArkComponentLocomotion::IKDiagProbeFloor( const FVector& _Start, const FVector& _Extent ) const
{
	if( !DisIKDiagEnabled() || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	static TMap<const FArkComponentLocomotion*, FLOAT> LastProbe;
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FLOAT* pLast = LastProbe.Find( this );
	if( pLast && Now - *pLast < 0.5f )
	{
		return;
	}
	LastProbe.Set( this, Now );
	const FVector End = _Start - FVector( 0.f, 0.f, 600.f );
	struct FProbe { const TCHAR* Name; DWORD Flags; UBOOL bExtent; };
	const FProbe Probes[] =
	{
		{ TEXT("npc 0x100030DF extent"), 0x100030DF, TRUE },
		{ TEXT("npc 0x30DF extent"), 0x30DF, TRUE },
		{ TEXT("player 0x200030DF extent"), 0x200030DF, TRUE },
		{ TEXT("allblocking zero-extent"), TRACE_AllBlocking, FALSE },
		{ TEXT("allblocking+player zero-extent"), TRACE_AllBlocking | TRACE_DisMove_Player, FALSE },
	};
	for( INT ProbeIdx = 0; ProbeIdx < ARRAY_COUNT( Probes ); ++ProbeIdx )
	{
		FCheckResult Hit( 1.f );
		GWorld->SingleLineCheck( Hit, m_pPawnOwner, End, _Start, Probes[ProbeIdx].Flags, Probes[ProbeIdx].bExtent ? _Extent : FVector( 0.f, 0.f, 0.f ) );
		debugf( TEXT("DISHONORED(bringup): ikdiag probe %s world %.3f from %s down 600, %s: %s"), *m_pPawnOwner->GetName(), Now, *_Start.ToString(), Probes[ProbeIdx].Name, *IKDiagDescribeHit( Hit ) );
	}
	static UBOOL bCensusDone = FALSE;
	if( !bCensusDone )
	{
		bCensusDone = TRUE;
		const FBox Probe( _Start - FVector( 250.f, 250.f, 700.f ), _Start + FVector( 250.f, 250.f, 100.f ) );
		for( FActorIterator It; It; ++It )
		{
			AActor* pActor = *It;
			for( INT CompIdx = 0; CompIdx < pActor->AllComponents.Num(); ++CompIdx )
			{
				UPrimitiveComponent* pPrimitive = Cast<UPrimitiveComponent>( pActor->AllComponents( CompIdx ) );
				if( !pPrimitive || !pPrimitive->IsAttached() || !Probe.Intersect( pPrimitive->Bounds.GetBox() ) )
				{
					continue;
				}
				FCheckResult Own( 1.f );
				const UBOOL bOwnMiss = pPrimitive->LineCheck( Own, End, _Start, FVector( 0.f, 0.f, 0.f ), TRACE_AllBlocking );
				FCheckResult OwnExtent( 1.f );
				const UBOOL bOwnExtentMiss = pPrimitive->LineCheck( OwnExtent, End, _Start, _Extent, TRACE_AllBlocking );
				FCheckResult Fake( 1.f );
				Fake.Actor = pActor;
				Fake.Component = pPrimitive;
				debugf( TEXT("DISHONORED(bringup): ikdiag census %s: bounds %s +- %s hidden %d | own zero-extent %s z %.1f, own extent %s z %.1f | %s"),
					*m_pPawnOwner->GetName(), *pPrimitive->Bounds.Origin.ToString(), *pPrimitive->Bounds.BoxExtent.ToString(), (INT)pPrimitive->HiddenGame,
					bOwnMiss ? TEXT("miss") : TEXT("HIT"), Own.Location.Z, bOwnExtentMiss ? TEXT("miss") : TEXT("HIT"), OwnExtent.Location.Z, *IKDiagDescribeHit( Fake ) );
			}
		}
	}
	FMemMark Mark( GMainThreadMemStack );
	INT HitIdx = 0;
	for( FCheckResult* pHit = GWorld->MultiLineCheck( GMainThreadMemStack, End, _Start, FVector( 0.f, 0.f, 0.f ), TRACE_AllBlocking | TRACE_Visible, m_pPawnOwner ); pHit && HitIdx < 8; pHit = pHit->GetNext(), ++HitIdx )
	{
		debugf( TEXT("DISHONORED(bringup): ikdiag probe %s world %.3f multi %d: %s"), *m_pPawnOwner->GetName(), Now, HitIdx, *IKDiagDescribeHit( *pHit ) );
	}
}

/** Once a second while a watched NPC is in PHYS_Falling: where, how fast, which volume, what is under it. */
void FArkComponentLocomotion::IKDiagFalling() const
{
	if( !DisIKDiagEnabled() || !m_pPawnOwner || m_pPawnOwner->Physics != PHYS_Falling || !DisIKDiagWatched( m_pPawnOwner ) )
	{
		return;
	}
	static TMap<const FArkComponentLocomotion*, FLOAT> LastReport;
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FLOAT* pLast = LastReport.Find( this );
	if( pLast && Now - *pLast < 1.f )
	{
		return;
	}
	LastReport.Set( this, Now );
	FCheckResult Hit( 1.f );
	const FVector Extent = m_pPawnOwner->GetCylinderExtent();
	GWorld->SingleLineCheck( Hit, m_pPawnOwner, m_pPawnOwner->Location - FVector( 0.f, 0.f, 200.f ), m_pPawnOwner->Location, TRACE_AllBlocking | TRACE_DisMove_NonPlayerPawn, Extent );
	APhysicsVolume* pVolume = m_pPawnOwner->PhysicsVolume;
	debugf( TEXT("DISHONORED(bringup): ikdiag falling %s world %.3f at %s vel %s accel %s volume %s water %d gravz %.1f base %s floor %s | below: %s"),
		*m_pPawnOwner->GetName(), Now, *m_pPawnOwner->Location.ToString(), *m_pPawnOwner->Velocity.ToString(), *m_pPawnOwner->Acceleration.ToString(),
		pVolume ? *pVolume->GetName() : TEXT("none"), pVolume ? (INT)pVolume->bWaterVolume : -1, pVolume ? pVolume->GetGravityZ() : 0.f,
		m_pPawnOwner->Base ? *m_pPawnOwner->Base->GetName() : TEXT("none"), *m_pPawnOwner->Floor.ToString(), *IKDiagDescribeHit( Hit ) );
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
