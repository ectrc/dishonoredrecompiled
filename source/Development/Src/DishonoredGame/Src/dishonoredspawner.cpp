// DishonoredGame/src/dishonoredspawner.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (38): the spawn path below is ported; the dialog, matinee preview, save/load and
// editor halves are named where they are reached.

// ---- agent CG ports (PHASE9 CG): the NPC spawn path ----
//
// Why this is in the AI package at all: the -disai census measured L_Tower_P and found 41 DishonoredSpawner actors and
// ZERO placed ADishonoredNPCPawn. Every NPC in Dishonored is spawned, so the AI brain root has nobody to think for
// until the spawner runs. That is the whole reason this unit is here; the spawner's other halves (dialog voice data,
// the matinee preview, the squad bookkeeping, save and load) are not the AI's business and are named rather than
// written.
//
// The per-subclass tweaks trap, in its sharpest form in this project so far: DoSpawnNow reads the NPC tweaks through the
// spawner's own IDisTweaksInterface. Without ADishonoredSpawner::GetTweaks_Derived below, IDisTweaksInterface::GetTweaks
// returns the class default of UDisTweaks_NPCPawn, whose m_pBrainTweak is NULL, and DoSpawnNow's
// `if( !Tweaks->m_pBrainTweak ) return` would refuse every spawn on every map with no warning at all - the exact shape
// of the defect that consumed 147 pickups in wave 7 and would have killed the pawn on every landing in wave 6.

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "dishonoredutilities.h"
#include "dishonoredutilities_ai.h"
#include "aistimstruct.h"

/** The pending-spawn ring is three deep in retail (m_PendingSpawns is one FDisSpawnInfo plus two more in the tail). */
#define DIS_SPAWNER_PENDING_SPAWNS 3

// DISHONORED(port): 2013 rva 0x6433b0 (2012 0x691670, 7 bytes) / 0x643190 (0x6940d0, 16 bytes): the spawner's own NPC
// tweaks. See the trap note at the top of this file - these two lines are what make every spawn work.
UDisTweaksBase* ADishonoredSpawner::GetTweaks_Derived()
{
	return m_pPawnTweaks;
}

void ADishonoredSpawner::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pPawnTweaks = (UDisTweaks_NPCPawn*)_pTweaks;
}

/** The NPC tweaks of this spawner, or their class default - the fallback retail takes in DoSpawnNow and OnSpawned. */
static UDisTweaks_NPCPawn* DisSpawnerTweaksOrDefault( ADishonoredSpawner* Spawner )
{
	UDisTweaks_NPCPawn* Tweaks = (UDisTweaks_NPCPawn*)Spawner->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	return Tweaks;
}

// DISHONORED(port): 2013 rva 0x65c030 (2012 0x6a35d0): the spawner registers itself with the game info, spawns its pawn
// immediately when m_bSpawnOnBeginPlay is set, hides its stealable pickup when it does not, and starts its spawn clock
// at FLT_MAX so the first spawn is never delayed.
// DISHONORED(bringup): the help-request noise listener (m_bSpawnOnHearHelpRequest -> UDisAINoiseManager::RegisterListener,
// 2013 rva 0x851570; agent EP corrected 0x7431e0, which is not a 2013 function start) needs the AI noise manager, which
// is not ported; a spawner that waits for a cry for help therefore
// never hears one. Also not done: registering in ADishonoredGameInfo's spawner list, because that member is reached
// through an offset the SDK dump does not name.
void ADishonoredSpawner::PostBeginPlay()
{
	Super::PostBeginPlay();

	if( m_bSpawnOnBeginPlay )
	{
		FDisSpawnInfo SpawnInfo( EC_EventParm );
		SpawnOnePawn( &SpawnInfo );
	}
	else if( m_pStealablePickup )
	{
		m_pStealablePickup->SetHidden( TRUE );
	}

	m_fTimeSinceLastSpawn = 3.4028235e38f;
}

// DISHONORED(port): 2013 rva 0x658b30 (2012 0x6a0a60): a spawn request joins a three-deep ring; the first one also
// enables the actor's tick, because a spawner with nothing pending does not tick at all. A fourth simultaneous request
// is dropped, which is retail's own back-pressure.
UBOOL ADishonoredSpawner::SpawnOnePawn( const FDisSpawnInfo* _pSpawnInfo )
{
	if( m_NumPendingSpawns >= DIS_SPAWNER_PENDING_SPAWNS )
	{
		return FALSE;
	}
	if( m_NumPendingSpawns == 0 )
	{
		SetTickIsDisabled( FALSE );
	}

	FDisSpawnInfo SpawnInfo( EC_EventParm );
	if( _pSpawnInfo )
	{
		SpawnInfo = *_pSpawnInfo;
	}
	const INT Slot = ( m_NumPendingSpawns + m_FirstPendingSpawn ) % DIS_SPAWNER_PENDING_SPAWNS;
	m_NumPendingSpawns++;
	m_PendingSpawns[Slot] = SpawnInfo;
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x658c20 (2012 0x6a0b50)
void ADishonoredSpawner::ClearPendingSpawns()
{
	m_NumPendingSpawns = 0;
	m_FirstPendingSpawn = 0;
	SetTickIsDisabled( TRUE );
}

// DISHONORED(port): 2013 rva 0x64b300 (2012 0x6973d0): the delay between two spawns from one spawner, which is the
// tweaks' m_fDelayBetweenSpawns unless the spawner overrides it.
UBOOL ADishonoredSpawner::IsMinDelaySinceLastSpawnElapsed( FLOAT* const _pOutDelay ) const
{
	const UDisTweaks_NPCPawn* Tweaks = DisSpawnerTweaksOrDefault( (ADishonoredSpawner*)this );
	const FLOAT Delay = Tweaks->m_fDelayBetweenSpawns;
	if( _pOutDelay )
	{
		*_pOutDelay = Delay;
	}
	return m_fTimeSinceLastSpawn >= Delay;
}

// DISHONORED(port): 2013 rva 0x65e790 (2012 0x6aa460, 1169 bytes): the five gates and then the spawn. Retail evaluates
// EVERY gate before testing them, so the returned reason code is the LAST failing one, not the first; that is kept,
// because the -disai census reports the code and it has to mean what retail's debug display means.
//   2 squad total limit   3 squad alive limit   4 the spawner is visible to the player   5 the requester refused
//   6 the player is too near   7 the minimum delay has not elapsed   8 no actor factory   1 the factory returned nothing
//   0 spawned
BYTE ADishonoredSpawner::DoSpawnNow( const FDisSpawnInfo& _rSpawnInfo )
{
	if( !m_pFactory )
	{
		return 8;
	}
	BYTE Reason = 8;

	// DISHONORED(bringup): the squad limits (UDishonoredMapInfo::FindSquadInfo, 2013 rva 0x6f5760, and the
	// m_NumNPCsAlive / m_MaxNPCsAlive / m_NumNPCsTotal / m_MaxNPCsTotal counters on FDisSquadInfo) are not ported, so a
	// squad never refuses a spawn. On a map whose designers relied on the limit this spawns more NPCs than retail; the
	// census reports the count so the difference is visible rather than silent.
	UBOOL bSquadAllowsSpawn = TRUE;

	// DISHONORED(bringup): the visibility gate asks DisIsActorVisibleToPlayer (2013 rva 0x7be3b0), one of the
	// dishonoredutilities_accessors.cpp helpers that is still a comment-only entry. It tests the player camera frustum
	// and a trace, so a spawner in view of the player defers its spawn by one tick in retail. Ours treats the spawner as
	// not visible, which spawns a frame earlier than retail in the one case where the player is looking straight at the
	// spawn point; the census reports the spawn time so the difference is measurable rather than hidden.
	UBOOL bVisibilityAllowsSpawn = TRUE;

	UBOOL bRequesterAllowsSpawn = TRUE;
	// DISHONORED(bringup): a spawn asked for by an IDisSpawnRequesterInterface (the wolfhound kennel, the assassin
	// summon) asks the requester how many it still wants through its vtable slot 8. That interface is not ported, so a
	// requested spawn is always allowed.

	UBOOL bPlayerIsNear = FALSE;
	if( GIsGame && !m_bCanSpawnWhenPlayerNear && !_rSpawnInfo.m_bSpawnEvenIfVisible )
	{
		const UDisTweaks_NPCPawn* Tweaks = DisSpawnerTweaksOrDefault( this );
		ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
		if( PlayerPawn )
		{
			const FLOAT MinRange = Tweaks->m_fPlayerMinRangeForSpawning;
			if( ( Location - PlayerPawn->Location ).SizeSquared() < MinRange * MinRange )
			{
				bPlayerIsNear = TRUE;
				Reason = 6;
			}
		}
	}

	UBOOL bTooSoon = FALSE;
	FLOAT Delay = 0.f;
	if( GIsGame && !IsMinDelaySinceLastSpawnElapsed( &Delay ) )
	{
		bTooSoon = TRUE;
		Reason = 7;
	}

	if( !bSquadAllowsSpawn || !bVisibilityAllowsSpawn || !bRequesterAllowsSpawn || bPlayerIsNear || bTooSoon )
	{
		return Reason;
	}

	UDisTweaks_NPCPawn* Tweaks = DisSpawnerTweaksOrDefault( this );
	if( !Tweaks->m_pBrainTweak )
	{
		// Retail's own refusal: an NPC with no brain tweaks is not spawned at all. See the trap note at the top of this
		// file - this is the line a missing GetTweaks_Derived would have made unconditional.
		return Reason;
	}

	// The spawn point is the spawner's origin raised by the NPC's collision half-height, so the pawn's feet land on the
	// spawner rather than its middle.
	FLOAT HalfHeight = 0.f;
	if( Tweaks->m_pBodyTweaks )
	{
		HalfHeight = Tweaks->m_pBodyTweaks->m_fCylinderHalfHeight;
	}
	if( HalfHeight == 0.f )
	{
		HalfHeight = 78.f;	// DISHONORED(retail): the ADishonoredNPCPawn class default's CylinderComponent height
	}
	const FVector SpawnLoc = Location + FVector( 0.f, 0.f, HalfHeight );

	m_pFactory->m_pNPCPawnClass = Tweaks->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame );
	m_pFactory->m_pNPCPawnTweaks = Tweaks;
	m_pFactory->m_pSpawner = this;

	// Retail calls CreateActor, not CreateNPCPawn: CreateActor is the half that spawns the NPC's controller and
	// possesses the pawn with it, and without that the pawn exists with no mind at all.
	ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( m_pFactory->CreateActor( &SpawnLoc, &Rotation, NULL ) );
	if( !Pawn )
	{
		return 1;
	}

	if( _rSpawnInfo.m_NPCID )
	{
		Pawn->m_NPCID = _rSpawnInfo.m_NPCID;
	}
	else
	{
		ADishonoredGameInfo* GameInfo = Cast<ADishonoredGameInfo>( GWorld ? GWorld->GetGameInfo() : NULL );
		if( GameInfo )
		{
			Pawn->m_NPCID = GameInfo->m_NextNPCID++;
		}
	}
	m_SpawnedPawns.AddItem( Pawn );
	m_fTimeSinceLastSpawn = 0.f;

	OnSpawned( Pawn );

	if( m_pStealablePickup )
	{
		m_pStealablePickup->Attach( Pawn, Pawn->Mesh );
	}

	// DISHONORED(bringup): the tail of retail's body also feeds the spawn's hear-sound and send-alarm requests
	// (UDisAINoiseManager::HandleNoiseHeard 0x743520, FDisAlarmSpawnInfo::SendAlarmToSpawnedPawn 0x64a8e0) and registers
	// OnPawnDestroyed on the pawn's terminated event. The noise manager and the alarm system are not ported.
	return 0;
}

// DISHONORED(port): 2013 rva 0x6590e0 (2012 0x6a1010, 637 bytes): the line that gives the NPC its mind. A pawn spawned
// dead or knocked out skips it entirely and only gets its Kismet events; a live one has its controller initialised with
// the tweaks' brain tweaks and the spawner's suspicion level, is put on patrol when the spawner asks, and is made aware
// of the player when the spawner asks.
// DISHONORED(bringup): the dialog half (PostSpawned_Dialog with the spawner's voice data and one-shots, 2013 rva
// 0x7548f0), the squad counters and DoDeadBodyPoseHACK are not ported; CopyEventsToSpawned and the Spawned Kismet event
// are named here because the Kismet sequence ops they need belong to agent AT's package.
void ADishonoredSpawner::OnSpawned( ADishonoredNPCPawn* _pPawn )
{
	if( !_pPawn )
	{
		return;
	}
	// DISHONORED(port): agent EP - retail 2013 tests m_bSpawnDead and **m_bTreatAsKnockedOut** here (0x75dbb0:
	// `(pawn+3280 & 1) == 0 && (pawn+3296 & 1) == 0`, and 0x6590e0 the same pair), not m_bStraightToRagdoll. The two
	// live in the same bitfield word - m_bSpawnDead is bit 0 of FDisSpawnerInfo+76 and m_bStraightToRagdoll bit 1 - which
	// is how they were confused. It was invisible while nothing filled FDisSpawnerInfo; the moment
	// FSpawnNPCPawn_TweakObj::DoInit did, m_bStraightToRagdoll was TRUE for every NPC with no dead-pose animation (which
	// is all of them) and the census read "26 NPC pawns, 0 controllers".
	if( _pPawn->m_SpawnerInfo.m_bSpawnDead || _pPawn->m_SpawnerInfo.m_bTreatAsKnockedOut )
	{
		return;
	}

	ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( _pPawn->Controller );
	if( !NPCController )
	{
		// The pawn is possessed by UDisActorFactoryNPCPawn::CreateActor before this runs; reaching here means that
		// possession failed, which is worth one line rather than a silent NPC with no mind.
		static UBOOL bWarnedOnce = FALSE;
		if( !bWarnedOnce )
		{
			bWarnedOnce = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredSpawner::OnSpawned: %s has no ADishonoredNPCController, so it gets no brain"), *_pPawn->GetName() );
		}
		return;
	}

	UDisTweaks_NPCPawn* Tweaks = (UDisTweaks_NPCPawn*)_pPawn->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	NPCController->InitNPC( Tweaks->m_pBrainTweak, m_SpawnAtSuspicionLevel );

	UDishonoredAIBrain* Brain = NPCController->m_pAIBrain;
	if( Brain && m_bPatrolUponStartup )
	{
		FAIStimStruct_PatrolRequest Stim( EC_EventParm );
		Stim.m_StimID = EAIStimID_PatrolRequest;
		DisHandleAIStim_Internal< FAIStimStruct_PatrolRequest >( Brain, Stim, this );
	}
	// DISHONORED(bringup): m_bAwareOfPlayerUponStartup would call UDishonoredAIBrain::MaxOutAttention (2013 rva
	// 0x723be0); the attention process it writes to is not ported, so a spawner marked "aware of the player" starts
	// unaware.
}

// DISHONORED(port): 2013 rva 0x658d30 (2012 0x6a0c60, 693 bytes): the pending ring is drained one entry per tick. A
// spawn that fails for a reason that can change (visible, player near, too soon) stays pending and is retried; one that
// fails because the spawner has no factory is dropped. With nothing pending the spawner disables its own tick again.
UBOOL ADishonoredSpawner::Tick( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	m_fTimeSinceLastSpawn += _fDeltaTime;

	if( m_NumPendingSpawns > 0 )
	{
		const FDisSpawnInfo& SpawnInfo = m_PendingSpawns[m_FirstPendingSpawn];
		const BYTE Result = DoSpawnNow( SpawnInfo );
		if( Result == 0 || Result == 8 || Result == 1 )
		{
			m_NumPendingSpawns--;
			m_FirstPendingSpawn = ( m_FirstPendingSpawn + 1 ) % DIS_SPAWNER_PENDING_SPAWNS;
			if( m_NumPendingSpawns == 0 )
			{
				SetTickIsDisabled( TRUE );
			}
		}
	}

	return Super::Tick( _fDeltaTime, _TickType );
}

// DISHONORED(port): 2013 rva 0x65c150 (2012 0x6a36e0): the Kismet StartSpawn action queues one spawn.
void ADishonoredSpawner::OnStartSpawn( UDisSeqAct_StartSpawn* _pAction )
{
	FDisSpawnInfo SpawnInfo( EC_EventParm );
	SpawnOnePawn( &SpawnInfo );
}

// ---- agent CG ports (PHASE9 CG): the Kismet exec wrappers ----

// DISHONORED(port): 2013 rva 0x64b010 (2012 0x6a3600): the exec wrapper of OnStartSpawn, whose body is above.
void ADishonoredSpawner::execOnStartSpawn( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDisSeqAct_StartSpawn, _pAction);
	P_FINISH;
	OnStartSpawn( _pAction );
}
