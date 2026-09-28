// DishonoredGame/src/dishonoredaibrain.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44): see agentCG_status.csv for the ported set and their rvas.

// ---- agent CG ports (PHASE9 CG): UDishonoredAIBrain, the AI brain root ----
//
// Read this before changing anything here from a 2012 decompile. Three things differ between the 2012 Shipping build
// (which is the readable one, because it carries the PDB type names) and retail 2013, which is the target:
//   1. m_BrainInhibitors (2012 @224, a TArray<UClass*>) and its three functions AddBrainInhibitor (2012 0x74d710),
//      RemoveBrainInhibitor (0x74fc20) and IsBrainInhibited (0x747730) DO NOT EXIST in retail 2013. None of the three
//      has a 2013 match, retail's InitBrain (0x7252a0) has no inhibitor clear where 2012's has one, and retail's
//      TickBrain (0x724a10) has no `m_BrainInhibitors.Num() <= 0` gate where 2012's does. Reintroducing that gate
//      would stop every brain from ticking.
//   2. m_ActiveBehaviorStack has 19 entries in retail 2013 and 18 in 2012, so every loop over it runs 0..18. Retail's
//      ProcessAllStims (0x717100) ends `while ( v8 < 19 )`.
//   3. m_MandatoryBehaviors (2013 @484) is new in retail and is a second source of behaviours: InitBrain builds one
//      behaviour per slot of the tweaks' m_BehaviorTweak AND one per entry of m_MandatoryBehaviors.
// Every member offset was read from the 2013 retail SDK layout through build/agentCG_work/off.py, never from the 2012
// PDB, which is the trap that has now caught two agents in this project.

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "disdesirestructs.h"
#include "dishonoredutilities.h"
#include "dishonoredutilities_ai.h"
#include "disaisubstate.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "aistimstruct.h"

/** DISHONORED(written): agent CG's DisRaiseStim<T> moved to Inc/aistimstruct.h beside DisHandleAIStim (agent DF), because
    the desire layer raises four stims of its own and the EAIStimID stamp must have exactly one home. */

/** DISHONORED(written): the guard retail inlines into FlushStimQueue (2013 rva 0x716eb0): a stim is serialized through
    an object-reference collector that flags any terminated or pending-kill actor it names, and a stim that names one is
    dropped instead of processed. */
static UBOOL DisStimNamesDeadActor( const FAIStimStruct* Stim )
{
	FDisArchiveCheckForBadActors Archive;
	DisStimSerialize( (FAIStimStruct*)Stim, Archive );
	return Archive.FoundBadActor();
}

/*-----------------------------------------------------------------------------
	Tweaks
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x53dfd0 (2012 0x745ed0, 7 bytes)
UDisTweaksBase* UDishonoredAIBrain::GetTweaks_Derived()
{
	return m_pBrainTweaks;
}

// DISHONORED(port): 2013 rva 0x703c90 (2012 0x747720, 16 bytes)
void UDishonoredAIBrain::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pBrainTweaks = (UDisTweaks_AIBrain*)_pTweaks;
}

/** The brain tweaks or, when the brain has none, their class default - the fallback retail takes in five places. */
static UDisTweaks_AIBrain* DisBrainTweaksOrDefault( UDishonoredAIBrain* Brain )
{
	UDisTweaks_AIBrain* Tweaks = (UDisTweaks_AIBrain*)Brain->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_AIBrain*)UDisTweaks_AIBrain::StaticClass()->GetDefaultObject();
	}
	return Tweaks;
}

/*-----------------------------------------------------------------------------
	Flags and simple queries
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x700d20 (2012 0x745ee0)
UBOOL UDishonoredAIBrain::IsBrainInitialized() const
{
	return m_bBrainIsInitialized != 0;
}

// DISHONORED(port): 2013 rva 0x700d30 (2012 0x745ef0): the next TickBrain does the refresh.
void UDishonoredAIBrain::MarkForRefreshBrainThoughts()
{
	m_bNeedsRefreshThoughts = TRUE;
}

// DISHONORED(port): 2013 rva 0x700e00 (2012 0x745f80): retail compares against 1, not against 0, so a flag byte that
// somehow held 2 reads as clear.
UBOOL UDishonoredAIBrain::IsFlagSet( BYTE _Flag ) const
{
	return m_BrainFlags[_Flag] == 1;
}

// DISHONORED(port): 2013 rva 0x700e20 (2012 0x745fa0)
void UDishonoredAIBrain::SetFlagTo( BYTE _Flag, UBOOL _bValue )
{
	m_BrainFlags[_Flag] = _bValue ? 1 : 0;
}

// DISHONORED(port): 2013 rva 0x7011f0 (2012 0x746390) / 0x7011e0 (0x746380) / 0x704250 (0x747c90): reading gives the
// applied level, writing only sets the pending one, which the next InitBrain or suspicion pass applies.
BYTE UDishonoredAIBrain::GetSuspicionLevel() const
{
	return m_SuspicionLevel;
}

void UDishonoredAIBrain::SetSuspicionLevel( BYTE _SuspicionLevel )
{
	m_PendingSuspicionLevel = _SuspicionLevel;
}

void UDishonoredAIBrain::EscalateSuspicionLevel()
{
	m_PendingSuspicionLevel = DAISL_Suspecting;
}

// DISHONORED(port): 2013 rva 0x700dc0 (2012 0x745f40): the current behaviour decides, not the brain.
UBOOL UDishonoredAIBrain::IsIgnoringTechnologyDanger() const
{
	return m_pCurrentBehavior ? m_pCurrentBehavior->BehaviorIgnoresTechnologyDanger() : FALSE;
}

// DISHONORED(port): 2013 rva 0x700e60 (2012 0x7460a0) / 0x701210 (0x7463a0) / 0x701180 (0x7462b0) / 0x701160 (0x746290)
UBOOL UDishonoredAIBrain::IsCombatEngaged() const
{
	return m_bCombatEngaged != 0;
}

UBOOL UDishonoredAIBrain::HasEngagedEnemy() const
{
	return m_bHasEngagedEnemy != 0;
}

UBOOL UDishonoredAIBrain::IsProtectingNeutrals() const
{
	return m_bProtectNeutrals != 0;
}

void UDishonoredAIBrain::OverrideProtectNeutrals( UBOOL _bProtect )
{
	m_bProtectNeutrals = _bProtect ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x700e40 (2012 0x745fc0)
ADishonoredPawn* UDishonoredAIBrain::GetCurrentEnemy() const
{
	return m_pCurrentBehavior ? m_pCurrentBehavior->GetCurrentEnemy() : NULL;
}

// DISHONORED(port): 2013 rva 0x703d00 (2012 0x745fe0)
// DISHONORED(bringup): retail forwards to UDisSteeringInfluence_EnemyPush::SetEnemyRange (2013 rva 0x76c790); the
// steering influences have no C++ API in the tree (DishonoredGameSteeringClasses.h has the classes and the members but
// no bodies), so the range is not passed on and NPCs keep their tweaked combat spacing.
void UDishonoredAIBrain::SetCombatRange( FLOAT _fRange )
{
}

// DISHONORED(port): 2013 rvas 0x70bad0 / 0x70bb00 / 0x70bb30 / 0x70bb60 (2012 0x74d8c0 / 0x74d8f0 / 0x74d920 /
// 0x74d950): each sense flag is one bit of one of the three per-source sense masks, and the final masks are rebuilt
// from them. Retail takes the mask type as its first argument; ours applies to every source, because
// BuildFinalSenseMask (2012 0x747960) has no 2013 match and the senses pass is not ported.
// DISHONORED(bringup): the mask bits are set but nothing reads them until TickBrain_Senses is real.
void UDishonoredAIBrain::SetDumbFlag( UBOOL _bSet )
{
	for( INT Type = 0; Type < ARRAY_COUNT(m_SenseMasksByType); Type++ )
	{
		m_SenseMasksByType[Type] = _bSet ? ( m_SenseMasksByType[Type] | 1 ) : ( m_SenseMasksByType[Type] & ~1 );
	}
}

void UDishonoredAIBrain::SetDeafFlag( UBOOL _bSet )
{
	for( INT Type = 0; Type < ARRAY_COUNT(m_SenseMasksByType); Type++ )
	{
		m_SenseMasksByType[Type] = _bSet ? ( m_SenseMasksByType[Type] | 2 ) : ( m_SenseMasksByType[Type] & ~2 );
	}
}

void UDishonoredAIBrain::SetBlindFlag( UBOOL _bSet )
{
	for( INT Type = 0; Type < ARRAY_COUNT(m_SenseMasksByType); Type++ )
	{
		m_SenseMasksByType[Type] = _bSet ? ( m_SenseMasksByType[Type] | 4 ) : ( m_SenseMasksByType[Type] & ~4 );
	}
}

void UDishonoredAIBrain::SetNumbFlag( UBOOL _bSet )
{
	for( INT Type = 0; Type < ARRAY_COUNT(m_SenseMasksByType); Type++ )
	{
		m_SenseMasksByType[Type] = _bSet ? ( m_SenseMasksByType[Type] | 8 ) : ( m_SenseMasksByType[Type] & ~8 );
	}
}

/*-----------------------------------------------------------------------------
	The behaviour stack
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x700d80 (2012 0x745f00): "current" is the topmost occupied slot, which is not
// necessarily m_pCurrentBehavior - a behaviour that is pending has the slot but is not yet current.
UBOOL UDishonoredAIBrain::IsCurrentBehavior( UClass* const _pBehaviorClass ) const
{
	for( INT Slot = ARRAY_COUNT(m_ActiveBehaviorStack) - 1; Slot >= 0; Slot-- )
	{
		if( m_ActiveBehaviorStack[Slot] )
		{
			return m_ActiveBehaviorStack[Slot]->GetClass() == _pBehaviorClass;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x703ca0 (2012 0x747740): a NULL class matches the first occupied slot, and a behaviour
// that is finishing does not count as being on the stack. The class test is IsA, not an exact match.
UBOOL UDishonoredAIBrain::IsBehaviorOnStack( UClass* const _pBehaviorClass ) const
{
	for( INT Slot = 0; Slot < ARRAY_COUNT(m_ActiveBehaviorStack); Slot++ )
	{
		UDishonoredAIBehavior* Behavior = m_ActiveBehaviorStack[Slot];
		if( !Behavior )
		{
			continue;
		}
		if( _pBehaviorClass && !Behavior->IsA( _pBehaviorClass ) )
		{
			continue;
		}
		if( Behavior->m_bForceFinishDueToDormancy || Behavior->IsBehaviorFinished() )
		{
			continue;
		}
		return TRUE;
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x70b580 (2012 0x749a70): "supports" asks the constructed behaviour list, not the stack,
// so it answers whether this brain's tweaks gave it such a behaviour at all. An exact-match request compares classes,
// otherwise it is an IsA test and a NULL class means "any behaviour at all".
UBOOL UDishonoredAIBrain::SupportsBehavior( const UClass* _pBehaviorClass, UBOOL _bExactMatch ) const
{
	for( INT Idx = 0; Idx < m_BehaviorArray.Num(); Idx++ )
	{
		UDishonoredAIBehavior* Behavior = m_BehaviorArray(Idx);
		if( !Behavior )
		{
			continue;
		}
		if( _bExactMatch )
		{
			if( Behavior->GetClass() == _pBehaviorClass )
			{
				return TRUE;
			}
			continue;
		}
		if( !_pBehaviorClass || Behavior->IsA( (UClass*)_pBehaviorClass ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x70ed10 (2012 0x7496a0): the tweaks hold one FBehaviorTweak per behaviour slot, and each
// FBehaviorTweak is four UDisTweaks_AIBehavior pointers, one per EDifficulty. The lookup walks DOWN from the current
// difficulty to Easy taking the first non-NULL, and if that finds nothing walks UP from the current difficulty. That
// is how a designer can set only the Hard entry of a slot and have every difficulty use it.
UDisTweaks_AIBehavior* UDishonoredAIBrain::GetAIBehaviorTweakForSlot( INT _iBehaviorTweakIndex ) const
{
	UDisTweaks_AIBrain* Tweaks = DisBrainTweaksOrDefault( (UDishonoredAIBrain*)this );
	if( _iBehaviorTweakIndex < 0 || _iBehaviorTweakIndex >= Tweaks->m_BehaviorTweak.Num() )
	{
		return NULL;
	}
	const FBehaviorTweak& Slot = Tweaks->m_BehaviorTweak(_iBehaviorTweakIndex);
	ADishonoredGameInfo* GameInfo = Cast<ADishonoredGameInfo>( GWorld ? GWorld->GetGameInfo() : NULL );
	const INT Difficulty = GameInfo ? GameInfo->m_Difficulty : 0;

	for( INT Try = Difficulty; Try >= 0; Try-- )
	{
		if( Try < ARRAY_COUNT(Slot.m_Difficulty) && Slot.m_Difficulty[Try] )
		{
			return Slot.m_Difficulty[Try];
		}
	}
	for( INT Try = Difficulty + 1; Try < ARRAY_COUNT(Slot.m_Difficulty); Try++ )
	{
		if( Slot.m_Difficulty[Try] )
		{
			return Slot.m_Difficulty[Try];
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x70b420: retail 2013 factors what 2012 inlines twice in InitBrain - construct the
// behaviour class the tweak names, initialise it and append it to m_BehaviorArray.
void UDishonoredAIBrain::AddBehaviorFromTweak( UDisTweaks_AIBehavior* const _pBehaviorTweak )
{
	if( !_pBehaviorTweak )
	{
		return;
	}
	UClass* BehaviorClass = _pBehaviorTweak->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame );
	if( !BehaviorClass )
	{
		return;
	}
	UDishonoredAIBehavior* Behavior = (UDishonoredAIBehavior*)StaticConstructObject( BehaviorClass, this );
	Behavior->CallInitBehavior( this, _pBehaviorTweak );
	m_BehaviorArray.AddItem( Behavior );
}

// DISHONORED(written): the block ProcessAllStims, ProcessOneStim and TerminateBrain each run to take a behaviour off
// the stack (2013 0x717100, 0x711960 and 0x726720 all contain it verbatim): pause it if it is the current one, drop its
// action target, stop it and its sub-state machine, stop its desires, clear m_bHasStarted, empty the slot.
void UDishonoredAIBrain::StopBehaviorInSlot( INT _Slot, UBOOL _bIsBeingTerminated )
{
	UDishonoredAIBehavior* Behavior = m_ActiveBehaviorStack[_Slot];
	if( !Behavior )
	{
		return;
	}
	if( Behavior == m_pCurrentBehavior )
	{
		Behavior->CallOnBehaviorPause( _bIsBeingTerminated );
		m_pCurrentBehavior = NULL;
	}
	Behavior->ClearActionTarget();
	Behavior->OnBehaviorStop( _bIsBeingTerminated );
	if( Behavior->m_pBehaviorFSM )
	{
		Behavior->m_pBehaviorFSM->OnOwningBehaviorStop( _bIsBeingTerminated );
	}
	// DISHONORED(port): agent DF. ProcessAllStims (2012 0x75b830) and ProcessOneStim (0x759e60) stop the desires;
	// TerminateBrain (0x75ccc0) finalises them, which is the only difference between the three copies of this block.
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( Behavior->m_pDesires ) )
	{
		if( _bIsBeingTerminated )
		{
			Desires->FinalizeDesires();
		}
		else
		{
			Desires->StopDesires();
		}
	}
	Behavior->m_bHasStarted = FALSE;
	m_ActiveBehaviorStack[_Slot] = NULL;
}

/*-----------------------------------------------------------------------------
	Brain processes
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2012 rva 0x749d10 (agent DF). The pawn's locomotion component is asked which of its own speeds the
 * active move request is running at, and UDisTweaks_NPCPawn::m_LocomotionSpeedToTransitSpeed maps that index back to an
 * ETransitSpeed. Read by UDisAISubStateTakePosition::RefreshSubState.
 * DISHONORED(bringup): FArkComponentLocomotion::GetActiveRequestMaxSpeedIdx does not exist yet (agentCG.md hand-over 3),
 * so there is no active request to read and the answer is Idle - which makes TakePosition use row 0 of
 * m_fRotationStartDistance, the standing-still distance, rather than the walking or running one. The lookup below is the
 * faithful half and needs only the component to become exact.
 */
BYTE UDishonoredAIBrain::GetCurrentDesiredTransitSpeed() const
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FArkComponentLocomotion::GetActiveRequestMaxSpeedIdx is not ported; the brain reports ETransitSpeed_Idle") );
	}
	const INT ActiveRequestMaxSpeedIdx = INDEX_NONE;
	if( ActiveRequestMaxSpeedIdx == INDEX_NONE )
	{
		return ETransitSpeed_Idle;
	}
	const UDisTweaks_NPCPawn* Tweaks = m_pOwningPawn ? Cast<UDisTweaks_NPCPawn>( m_pOwningPawn->GetTweaks_Derived() ) : NULL;
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	if( ActiveRequestMaxSpeedIdx >= Tweaks->m_LocomotionSpeedToTransitSpeed.Num() )
	{
		return ETransitSpeed_Idle;
	}
	return (BYTE)Tweaks->m_LocomotionSpeedToTransitSpeed( ActiveRequestMaxSpeedIdx );
}

// DISHONORED(port): 2013 rva 0x70b710 (2012 0x749b60): an exact class match.
UDisAIBrainProcess* UDishonoredAIBrain::GetBrainProcess( UClass* _pBrainProcessClass )
{
	for( INT Idx = 0; Idx < m_BrainProcesses.Num(); Idx++ )
	{
		if( m_BrainProcesses(Idx) && m_BrainProcesses(Idx)->GetClass() == _pBrainProcessClass )
		{
			return m_BrainProcesses(Idx);
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x70edf0 (2012 0x749780): one brain process per enabled entry of the tweaks'
// m_BrainProcessTweaks, and the attention process is additionally cached in m_pAttentionProcess because InitBrain and
// the senses pass reach for it by name.
void UDishonoredAIBrain::InitBrain_Processes()
{
	UDisTweaks_AIBrain* Tweaks = DisBrainTweaksOrDefault( this );
	for( INT Idx = 0; Idx < Tweaks->m_BrainProcessTweaks.Num(); Idx++ )
	{
		UDisTweaks_AIBrainProcess* ProcessTweaks = Tweaks->m_BrainProcessTweaks(Idx);
		if( !ProcessTweaks || !ProcessTweaks->m_bBrainProcessEnabled )
		{
			continue;
		}
		UClass* ProcessClass = ProcessTweaks->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame );
		if( !ProcessClass )
		{
			continue;
		}
		UDisAIBrainProcess* Process = (UDisAIBrainProcess*)StaticConstructObject( ProcessClass, this );
		Process->InitBrainProcess( this, ProcessTweaks );
		m_BrainProcesses.AddItem( Process );
		if( Process->GetClass() == UDisAIBrainProcessAttention::StaticClass() )
		{
			m_pAttentionProcess = (UDisAIBrainProcessAttention*)Process;
		}
	}
}

// DISHONORED(port): 2013 rva 0x720320 (2012 0x75bdf0): the stim queue is flushed first, so a process ticks with the
// stims of this frame already applied.
void UDishonoredAIBrain::TickBrain_Processes( FLOAT _fDeltaSeconds )
{
	ProcessAllStims();
	for( INT Idx = 0; Idx < m_BrainProcesses.Num(); Idx++ )
	{
		if( m_BrainProcesses(Idx) )
		{
			m_BrainProcesses(Idx)->TickBrainProcess( _fDeltaSeconds );
		}
	}
}

// DISHONORED(port): 2013 rva 0x70b4a0 (2012 0x749910)
void UDishonoredAIBrain::RefreshBrain_Processes( FLOAT _fTimeSinceLastThought )
{
	for( INT Idx = 0; Idx < m_BrainProcesses.Num(); Idx++ )
	{
		if( m_BrainProcesses(Idx) )
		{
			m_BrainProcesses(Idx)->RefreshBrainProcess( _fTimeSinceLastThought );
		}
	}
}

/*-----------------------------------------------------------------------------
	Steering
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x711cf0 (2012 0x74a6d0): the first three entries of the reflected m_SteeringInfluences
// array are the combat, enemy-push and danger influences, and each influence is given this brain, this pawn and the
// tweaks' m_fInfluenceFadeSpeed before being started. The final force starts at zero.
// DISHONORED(bringup): retail calls two UDisSteeringInfluence virtuals per influence (2013 vtable +316 and +320, the
// reset and the settings refresh). DishonoredGameSteeringClasses.h declares the classes and their members but no
// methods, so those two calls are not made; the influences therefore hold their defaults and TickBrain_Steering (see
// dishonoredaibrain_steering.cpp) produces no force.
void UDishonoredAIBrain::InitBrain_Steering()
{
	if( m_SteeringInfluences.Num() > 0 )
	{
		m_pSteeringInfluence_Combat = (UDisSteeringInfluence_Combat*)m_SteeringInfluences(0);
	}
	if( m_SteeringInfluences.Num() > 1 )
	{
		m_pSteeringInfluence_EnemyPush = (UDisSteeringInfluence_EnemyPush*)m_SteeringInfluences(1);
	}
	if( m_SteeringInfluences.Num() > 2 )
	{
		m_pSteeringInfluence_Danger = (UDisSteeringInfluence_Danger*)m_SteeringInfluences(2);
	}

	UDisTweaks_AIBrain* Tweaks = DisBrainTweaksOrDefault( this );
	const FLOAT FadeSpeed = Tweaks->m_pSteeringTweaks ? Tweaks->m_pSteeringTweaks->m_fInfluenceFadeSpeed : 0.f;
	for( INT Idx = 0; Idx < m_SteeringInfluences.Num(); Idx++ )
	{
		UDisSteeringInfluence* Influence = m_SteeringInfluences(Idx);
		if( !Influence )
		{
			continue;
		}
		Influence->m_pOwningBrain = this;
		Influence->m_pOwningPawn = m_pOwningPawn;
		Influence->m_fCurFadeSpeed = FadeSpeed;
	}
	m_FinalSteeringForce = FVector( 0.f, 0.f, 0.f );
}

// DISHONORED(port): the counterpart of InitBrain_Steering that TerminateBrain calls (2012 rva 0x74a9b0; the 2013 match
// is unresolved, which is why this carries no 2013 rva).
// DISHONORED(bringup): retail stops every influence through the same two virtuals InitBrain_Steering cannot call.
void UDishonoredAIBrain::TermBrain_Steering()
{
	m_FinalSteeringForce = FVector( 0.f, 0.f, 0.f );
}

/*-----------------------------------------------------------------------------
	Init, tick and terminate
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7252a0 (2012 0x75c8d0, 1017 bytes in retail). The order matters and is retail's:
//   the pawn and its spawner position become the home; a spawner with no patrol sets brain flag 4; the controller is
//   cached and becomes the component container's owner; the tweaks are applied; the suspicion level is set both
//   pending and applied; the global AI manager and its stim manager are cached and this brain joins the manager's
//   intrusive list; the look-at request, the steering and the brain processes are built; the reaction monitor, the
//   knowledge component and (when the attention tweaks ask for it) the attention monitor are added to the container;
//   the behaviour list is emptied and rebuilt from the tweaks' behaviour slots and then from m_MandatoryBehaviors;
//   the container starts; m_bHasEngagedEnemy is cleared, the thought clock starts, the brain is marked initialized and
//   m_bProtectNeutrals is taken from the tweaks; a BrainInit stim is raised and one zero-length tick runs, which is
//   what activates the first behaviour before the first frame.
void UDishonoredAIBrain::InitBrain( ADishonoredNPCPawn* const _pNPCPawn, UDisTweaks_AIBrain* const _pBrainTweaks, BYTE _SuspicionLevel )
{
	if( m_bBrainIsInitialized )
	{
		return;
	}
	if( !_pNPCPawn )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredAIBrain::InitBrain called with no pawn") );
		return;
	}

	m_pOwningPawn = _pNPCPawn;
	m_Home = _pNPCPawn->m_SpawnerInfo.m_Position;
	if( !_pNPCPawn->m_SpawnerInfo.m_bPatrolUponStartup )
	{
		m_BrainFlags[4] = 1;
	}

	m_pOwningController = Cast<ADishonoredNPCController>( _pNPCPawn->Controller );
	// DISHONORED(bringup): retail also sets m_pBrainComponentContainer->Owner to the controller here (2013 InitBrain
	// writes container+72 directly). UActorComponent::Owner is protected in the reference engine with only AActor as a
	// friend, and Engine/Inc/EngineClasses.h is not this package's file, so the container's owner stays NULL. Nothing
	// reads it yet: the three FArkComponentBase components retail would add to the container are themselves unported.

	if( GetTweaks_Derived() != _pBrainTweaks )
	{
		SetTweaks_Derived( _pBrainTweaks );
		ApplyTweakChanges();
	}
	ApplyTweakChanges();

	m_SuspicionLevel = _SuspicionLevel;
	m_PendingSuspicionLevel = _SuspicionLevel;

	UDisTweaks_AIBrain* Tweaks = DisBrainTweaksOrDefault( this );

	m_pGlobalAIManager = DisGetGlobalAIManagerUnchecked();
	if( m_pGlobalAIManager )
	{
		m_pGlobalAIManager->AddBrain( this );
		m_pStimManager = m_pGlobalAIManager->GetStimManager();
	}

	// DISHONORED(bringup): FDisLookAtRequest::Initialize (2013 rva 0x74f180) wires m_LookAtRequest to the pawn's
	// FArkComponentLookat at priority DisLookAtPriority_EnemyAttention (15). FArkComponentLookat is not ported
	// (Engine/Inc/arkcomponentlookat.h is a skeleton), so the head-look request is never issued.

	InitBrain_Steering();
	m_PendingMagicalIncident.m_pMagicUsingPawn = NULL;
	m_PendingMagicalIncident.m_pMagicalIncidentClass = NULL;
	InitBrain_Processes();

	// DISHONORED(bringup): retail adds three FArkComponentBase components to m_pBrainComponentContainer here and they
	// are the brain's own senses and memory: FDisAIMonitorReaction (creator 2012 rva 0x631ca0) with its Init, always;
	// FDisAIKnowledgeComponent (0x631bc0), always; and FDisMonitorNPCAttention (0x632170) when the attention process's
	// UDisTweaks_AIBrainProcess_Attention has its monitor bit set. None of the three is ported (disaimonitorreaction.cpp,
	// disaiknowledgecomponent.cpp and the attention monitor are comment-only skeletons), so the container stays empty
	// and every GetFirstComponent<T> in the AI answers NULL - which is a path retail itself has, so the brain runs.

	m_pCurrentBehavior = NULL;
	m_BehaviorArray.Empty();

	const INT NumBehaviorSlots = Tweaks->m_BehaviorTweak.Num();
	for( INT Slot = 0; Slot < NumBehaviorSlots; Slot++ )
	{
		AddBehaviorFromTweak( GetAIBehaviorTweakForSlot( Slot ) );
	}
	for( INT Idx = 0; Idx < m_MandatoryBehaviors.Num(); Idx++ )
	{
		AddBehaviorFromTweak( m_MandatoryBehaviors(Idx) );
	}

	if( m_pBrainComponentContainer )
	{
		m_pBrainComponentContainer->StartAllComponents();
	}
	m_BehaviorArray.Shrink();

	m_bHasEngagedEnemy = FALSE;
	m_fLastThoughtTime = DisGetAppropriateWorldTime( _pNPCPawn );
	m_bBrainIsInitialized = TRUE;
	m_bProtectNeutrals = Tweaks->m_bProtectNeutrals ? TRUE : FALSE;

	// DISHONORED(bringup): retail raises FAIStimStruct_BrainInit (stim id 12) through
	// HandleAIStim_Internal<FAIStimStruct_BrainInit> (2013 rva 0x720420) right here, and that single stim is what
	// activates the first behaviour - without it the stack stays empty and the NPC has no behaviour. The stim structs
	// are Src/aistimstruct.cpp, ported beside this file in the same package; when that unit is present this becomes
	// HandleAIStim_Internal<FAIStimStruct_BrainInit>( this ).
	DisRaiseStim< FAIStimStruct_BrainInit >( this, EAIStimID_BrainInit, this );

	TickBrain( 0.f );

	// DISHONORED(bringup): retail then registers OnDifficultyChange on global event 9 and OnPushedByAvoidable on the
	// pawn's object event 1 with FArkGameEventDispatcher (2013 rva 0x557160 GetInstance). The dispatcher is ported in
	// this same package (Engine/Src/arkgameeventdispatcher.cpp); both registrations are agent CG hand-over 1.
}

// DISHONORED(port): 2013 rva 0x724a10 (2012 0x75c3f0): the whole per-frame AI pass of one NPC. Note that retail 2013
// has NO inhibitor gate here - see the header comment of this file.
void UDishonoredAIBrain::TickBrain( FLOAT _fDeltaSeconds )
{
	if( !m_bBrainIsInitialized )
	{
		return;
	}
	GDisAIBrainTicks++;

	if( m_pOwningPawn && m_pOwningPawn->ShouldAIBeNotifiedOfRelationshipChange() )
	{
		m_fEmpathyInhibitionTimer = 1.f;
		RefreshRelationshipStatus();
	}
	else
	{
		m_fEmpathyInhibitionTimer -= _fDeltaSeconds;
	}

	TickBrain_Stealth();
	TickBrain_Stims( _fDeltaSeconds );
	TickBrain_Processes( _fDeltaSeconds );
	TickBrain_Senses( _fDeltaSeconds );
	ProcessAllStims();
	if( _fDeltaSeconds > 0.f && m_pCurrentBehavior )
	{
		m_pCurrentBehavior->CallTickBehavior( _fDeltaSeconds );
	}
	TickBrain_Steering( _fDeltaSeconds );

	if( m_bNeedsRefreshThoughts )
	{
		m_bNeedsRefreshThoughts = FALSE;
		RefreshBrainThoughts();
	}
}

// DISHONORED(port): 2013 rva 0x720390 (2012 0x75be60): the thought pass runs on its own clock, and the elapsed time it
// hands out is the time since the last thought rather than the frame delta.
void UDishonoredAIBrain::RefreshBrainThoughts()
{
	if( !m_pOwningPawn || !m_bBrainIsInitialized )
	{
		return;
	}
	ProcessAllStims();
	const FLOAT CurrentThoughtTime = DisGetAppropriateWorldTime( m_pOwningPawn );
	const FLOAT TimeSinceLastThought = CurrentThoughtTime - m_fLastThoughtTime;
	if( TimeSinceLastThought > 0.f )
	{
		if( m_pCurrentBehavior )
		{
			m_pCurrentBehavior->CallRefreshThoughts( TimeSinceLastThought );
		}
		RefreshBrainThoughts_Steering( TimeSinceLastThought );
		RefreshBrain_Processes( TimeSinceLastThought );
	}
	m_fLastThoughtTime = CurrentThoughtTime;
}

// DISHONORED(port): 2013 rva 0x726720 (2012 0x75ccc0): every slot of the stack is stopped from the top down, the stim
// queue and the brain processes go away, combat is disengaged, the brain leaves the global manager's list and its
// components stop. Retail 2013 has no inhibitor list to clear here.
void UDishonoredAIBrain::TerminateBrain()
{
	if( !m_bBrainIsInitialized )
	{
		return;
	}
	m_bBrainIsInitialized = FALSE;
	m_bCombatEngaged = FALSE;
	m_SuspicionLevel = DAISL_Unsuspecting;

	for( INT Slot = ARRAY_COUNT(m_ActiveBehaviorStack) - 1; Slot >= 0; Slot-- )
	{
		StopBehaviorInSlot( Slot, TRUE );
	}

	for( INT Idx = 0; Idx < m_StimQueue.Num(); Idx++ )
	{
		DisStimRefRelease( m_StimQueue(Idx) );
	}
	m_StimQueue.Empty();

	for( INT Idx = 0; Idx < m_BrainProcesses.Num(); Idx++ )
	{
		if( m_BrainProcesses(Idx) )
		{
			m_BrainProcesses(Idx)->TermBrainProcess();
		}
	}
	m_BrainProcesses.Empty();

	// DISHONORED(bringup): retail disengages combat here through UDisSteeringInfluence_Combat::CombatDisengage_Steering,
	// UDisSteeringInfluence_EnemyPush::EnableEnemyPush and UDisGlobalCombatManager::CombatDisengage (2013 rvas 0x76c8b0,
	// 0x76ca10, 0x83fbe0), and clears the two attention targets through SetTopAttnTarget / SetTopEnemyAttnTarget
	// (0x725e50 / 0x724ae0). The combat manager, the steering influence bodies and the attention process are all
	// unported, so only the brain-side state is cleared.

	if( m_pGlobalAIManager )
	{
		m_pGlobalAIManager->RemoveBrain( this );
	}
	TermBrain_Steering();

	if( m_pBrainComponentContainer )
	{
		m_pBrainComponentContainer->StopAllComponents();
	}
}

// DISHONORED(port): 2013 rva 0x726920 (2012 0x75cf80): "forget" is a terminate-and-reinit on the same pawn with the
// same tweaks and the same suspicion level, and a pawn whose spawner gave it a patrol route is put back on patrol.
void UDishonoredAIBrain::Forget()
{
	if( !m_bBrainIsInitialized || !m_pOwningPawn )
	{
		return;
	}
	if( m_pOwningPawn->IsPendingKill() || DisIsPawnDead( m_pOwningPawn ) )
	{
		return;
	}
	ADishonoredNPCPawn* Pawn = m_pOwningPawn;
	UDisTweaks_AIBrain* Tweaks = m_pBrainTweaks;
	const BYTE Suspicion = m_SuspicionLevel;
	TerminateBrain();
	InitBrain( Pawn, Tweaks, Suspicion );
	// DISHONORED(bringup): retail raises FAIStimStruct_PatrolRequest here when the spawner asked for a patrol route
	// (2013 rva 0x726a00's sibling path); the stim structs are ported beside this file, see InitBrain's note.
}

// DISHONORED(port): 2013 rva 0x725240 (2012 0x75c390)
// DISHONORED(bringup): retail also unregisters the two FArkGameEventDispatcher subscriptions InitBrain made.
void UDishonoredAIBrain::BeginDestroy()
{
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x727180 (2012 0x75dd20): an actor this brain remembers was destroyed, so every
// behaviour on the stack and every brain process is told.
void UDishonoredAIBrain::OnOtherActorTerminated_AIBrain( const AActor& _rActor )
{
	for( INT Slot = 0; Slot < ARRAY_COUNT(m_ActiveBehaviorStack); Slot++ )
	{
		UDishonoredAIBehavior* Behavior = m_ActiveBehaviorStack[Slot];
		if( Behavior && Behavior->GetBehaviorActionTargetActor() == &_rActor )
		{
			Behavior->ClearActionTarget();
		}
	}
}

// DISHONORED(port): 2013 rva 0x722ad0 (2012 0x7579b0): a difficulty change re-reads every behaviour tweak, because
// GetAIBehaviorTweakForSlot answers per difficulty.
void UDishonoredAIBrain::OnDifficultyChange( const FArkGameEvent& _rEvent )
{
	if( !m_bBrainIsInitialized )
	{
		return;
	}
	for( INT Idx = 0; Idx < m_BehaviorArray.Num() && Idx < m_pBrainTweaks->m_BehaviorTweak.Num(); Idx++ )
	{
		UDisTweaks_AIBehavior* Tweak = GetAIBehaviorTweakForSlot( Idx );
		if( Tweak && m_BehaviorArray(Idx) && m_BehaviorArray(Idx)->GetTweaks_Derived() != Tweak )
		{
			m_BehaviorArray(Idx)->SetTweaks( Tweak );
		}
	}
	MarkForRefreshBrainThoughts();
}

// DISHONORED(port): 2013 rva 0x722c90 (2012 0x757b50): an avoidable pushed this NPC.
// DISHONORED(bringup): the body reads the push through UArkAvoidable, which is not ported; the thought refresh retail
// ends with is kept, because that is what makes the NPC react at all.
void UDishonoredAIBrain::OnPushedByAvoidable( const FArkGameEvent& _rEvent )
{
	MarkForRefreshBrainThoughts();
}

// DISHONORED(port): 2013 rva 0x723a90 (2012 0x7588e0): the relationship of this NPC to somebody changed, so a
// RelationshipChanged stim is raised and the pawn's request is acknowledged so it is not raised again next frame.
void UDishonoredAIBrain::RefreshRelationshipStatus()
{
	DisRaiseStim< FAIStimStruct_RelationshipChanged >( this, EAIStimID_RelationshipChanged, this );
	if( m_pOwningPawn )
	{
		m_pOwningPawn->AcknowledgeRelationshipChangeHandledByAI();
	}
}

/*-----------------------------------------------------------------------------
	The stim queue: how anything that happens in the world reaches a behaviour
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x70b510 (2012 0x749980): every queued stim's delay counts down by the frame delta. A
// stim whose delay has reached zero is taken by the next FlushStimQueue.
void UDishonoredAIBrain::TickBrain_Stims( FLOAT _fDeltaTime )
{
	for( INT Idx = 0; Idx < m_StimQueue.Num(); Idx++ )
	{
		m_StimQueue(Idx).m_fDelayTimer -= _fDeltaTime;
	}
}

// DISHONORED(port): 2013 rva 0x7148c0 (2012 0x752490): a new stim is offered to every stim already queued through the
// queued stim's own pending-filter delegate, and the four answers decide which of the two survives: keep both,
// discard the queued one, drop the new one, or drop the new one and that queued one too. The last two stop the scan.
// Only then is the new stim queued, with the delay its own GetDelay gives.
void UDishonoredAIBrain::EnqueueStim( FAIStimStruct* _pAddMe )
{
	if( !_pAddMe )
	{
		return;
	}
	TArray<UBOOL> bShouldDiscardExistingStim;
	bShouldDiscardExistingStim.AddZeroed( m_StimQueue.Num() );
	UBOOL bShouldEnqueueStim = TRUE;

	for( INT Idx = 0; Idx < m_StimQueue.Num(); Idx++ )
	{
		const FAIStimStruct* Existing = (const FAIStimStruct*)m_StimQueue(Idx).m_pStim;
		if( !Existing )
		{
			continue;
		}
		const FDisStimPendingFilterDelegate Filter = DisStimGetPendingStimFilterDelegate( Existing, _pAddMe->m_StimID );
		const EDisPendingStimFilterResult Result = Filter.IsBound() ? Filter( *_pAddMe ) : DPSFR_KeepBothStims;
		if( Result == DPSFR_DiscardExistingStim )
		{
			bShouldDiscardExistingStim(Idx) = TRUE;
		}
		else if( Result == DPSFR_DiscardNewStim )
		{
			bShouldEnqueueStim = FALSE;
			bShouldDiscardExistingStim.Empty();
			bShouldDiscardExistingStim.AddZeroed( m_StimQueue.Num() );
			break;
		}
		else if( Result == DPSFR_DiscardBothStims )
		{
			bShouldEnqueueStim = FALSE;
			bShouldDiscardExistingStim.Empty();
			bShouldDiscardExistingStim.AddZeroed( m_StimQueue.Num() );
			bShouldDiscardExistingStim(Idx) = TRUE;
			break;
		}
	}

	for( INT Idx = bShouldDiscardExistingStim.Num() - 1; Idx >= 0; Idx-- )
	{
		if( bShouldDiscardExistingStim(Idx) )
		{
			DisStimRefRelease( m_StimQueue(Idx) );
			m_StimQueue.Remove( Idx, 1 );
		}
	}

	if( bShouldEnqueueStim )
	{
		// DisStimRefInit, not a raw field assignment: the queue entry has to TAKE a reference, or the stack reference
		// DisHandleAIStim_Internal holds across this call is the last one and the stim's block is returned to the pool
		// the moment it returns. See build/agentCG_work/fix10.py for how that showed up.
		const INT Added = m_StimQueue.Add( 1 );
		DisStimRefInit( m_StimQueue(Added), _pAddMe, DisStimGetDelay( _pAddMe, this ) );
		GDisAIStimsEnqueued++;
	}
}

// DISHONORED(port): 2013 rva 0x716eb0 (2012 0x75b630): every stim whose delay has expired leaves the queue and is
// processed, but only after an object-reference pass over it proves it names no terminated or pending-kill actor - a
// stim that does is dropped silently, which is how the AI never touches a dead pointer.
void UDishonoredAIBrain::FlushStimQueue( UBOOL* const _bActiveStackCancels, UBOOL* const _bBecamePaused )
{
	// The references move from the queue into the local array rather than being copied, so the stim stays alive for
	// exactly as long as it is being processed and is released once at the end.
	TArray<FDisStimRef> StimsToProcessThisTick;
	for( INT Idx = 0; Idx < m_StimQueue.Num(); Idx++ )
	{
		if( m_StimQueue(Idx).m_fDelayTimer <= 0.f )
		{
			StimsToProcessThisTick.AddItem( m_StimQueue(Idx) );
			m_StimQueue.Remove( Idx, 1 );
			Idx--;
		}
	}
	for( INT Idx = 0; Idx < StimsToProcessThisTick.Num(); Idx++ )
	{
		const FAIStimStruct* Stim = (const FAIStimStruct*)StimsToProcessThisTick(Idx).m_pStim;
		if( Stim && !DisStimNamesDeadActor( Stim ) )
		{
			ProcessOneStim( *Stim, _bActiveStackCancels, _bBecamePaused );
			GDisAIStimsProcessed++;
		}
		DisStimRefRelease( StimsToProcessThisTick(Idx) );
	}
}

// DISHONORED(port): 2013 rva 0x717100 (2012 0x75b830): the pass that decides which behaviour is current. The queue is
// flushed, then the whole 19-slot stack is walked bottom-up: a slot that was cancelled, forced to finish or reports
// itself finished is stopped, and the highest slot still occupied becomes the current behaviour and is resumed with
// the pawn's body intention as it stands. Every slot that lost its place is told it became dormant.
void UDishonoredAIBrain::ProcessAllStims()
{
	FDisBodyIntention PawnExistingBodyIntention;
	appMemzero( &PawnExistingBodyIntention, sizeof(PawnExistingBodyIntention) );
	if( m_pOwningPawn )
	{
		PawnExistingBodyIntention.m_IntendedBodyStance = m_pOwningPawn->GetBodyStance();
		PawnExistingBodyIntention.m_pDesiredPrimaryItemClass = m_pOwningPawn->GetDesiredPrimaryItem();
		PawnExistingBodyIntention.m_pDesiredSecondaryItemClass = m_pOwningPawn->GetDesiredSecondaryItem();
	}

	UBOOL bActiveStackCancels[ARRAY_COUNT(m_ActiveBehaviorStack)];
	UBOOL bBecamePaused[ARRAY_COUNT(m_ActiveBehaviorStack)];
	appMemzero( bActiveStackCancels, sizeof(bActiveStackCancels) );
	appMemzero( bBecamePaused, sizeof(bBecamePaused) );
	FlushStimQueue( bActiveStackCancels, bBecamePaused );

	UDishonoredAIBehavior* pHighestActiveBehavior = NULL;
	for( INT Slot = 0; Slot < ARRAY_COUNT(m_ActiveBehaviorStack); Slot++ )
	{
		UDishonoredAIBehavior* Behavior = m_ActiveBehaviorStack[Slot];
		if( !Behavior )
		{
			continue;
		}
		if( bActiveStackCancels[Slot] || Behavior->m_bForceFinishDueToDormancy || Behavior->IsBehaviorFinished() )
		{
			StopBehaviorInSlot( Slot, FALSE );
			bBecamePaused[Slot] = FALSE;
		}
		if( m_ActiveBehaviorStack[Slot] )
		{
			pHighestActiveBehavior = m_ActiveBehaviorStack[Slot];
		}
	}

	if( !m_pCurrentBehavior && pHighestActiveBehavior )
	{
		m_pCurrentBehavior = pHighestActiveBehavior;
		m_pCurrentBehavior->CallOnBehaviorResume( PawnExistingBodyIntention );
		bBecamePaused[m_pCurrentBehavior->m_DesignatedSlot] = FALSE;
	}

	for( INT Slot = 0; Slot < ARRAY_COUNT(m_ActiveBehaviorStack); Slot++ )
	{
		if( bBecamePaused[Slot] && m_ActiveBehaviorStack[Slot] )
		{
			m_ActiveBehaviorStack[Slot]->OnBecomeDormant();
		}
	}
}

// DISHONORED(port): 2013 rva 0x711960 (2012 0x759e60, 918 bytes): one stim against one brain, and the only place a
// behaviour is ever activated.
//   1. the stim is recorded as the last received one and the sense interception is asked
//   2. the reaction monitor and then the attention monitor may filter it out
//   3. every brain process may filter it out, and one that does ends the pass
//   4. the stack is walked from the TOP down: the topmost live behaviour filters the stim (once, which is why a
//      behaviour cannot be filtered by one below it), and every live behaviour below is asked whether this stim
//      finishes it while dormant
//   5. if nothing filtered it, every constructed behaviour whose evaluate mask covers this stim id is asked, in
//      m_BehaviorArray order, whether it wants to activate. The first one per designated slot that says yes takes the
//      slot: whatever was there is stopped, a lower-slot current behaviour is paused, the behaviour reads the stim
//      through its setup delegate, and OnBehaviorStart runs.
void UDishonoredAIBrain::ProcessOneStim( const FAIStimStruct& _rAIStim, UBOOL* const _bActiveStackCancels, UBOOL* const _bBecamePaused )
{
	if( _rAIStim.m_StimID >= EAIStimID_MAX )
	{
		static UBOOL bWarnedOnce = FALSE;
		if( !bWarnedOnce )
		{
			bWarnedOnce = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredAIBrain::ProcessOneStim: stim id %i is outside the %i of EAIStimID; every AI stim mask is indexed by it, so the stim is dropped"),
				(INT)_rAIStim.m_StimID, (INT)EAIStimID_MAX );
		}
		return;
	}
	m_LastReceivedStim = _rAIStim.m_StimID;

	// DISHONORED(bringup): IsSenseIntercepted (2012 rva 0x7479f0) has no match in retail 2013, and its body reads the
	// final sense masks that BuildFinalSenseMask (2012 0x747960, also unmatched) would build, so the interception test
	// is skipped. Likewise retail's two component filters at this point: FDisAIMonitorReaction::DoFilterStim and
	// FDisMonitorNPCAttention::DoFilterStim. Both components are unported, so GetFirstComponent answers NULL and
	// retail's own null path runs - the stim is not filtered by them, which is what an NPC with no reaction monitor
	// does in retail too.

	for( INT Idx = 0; Idx < m_BrainProcesses.Num(); Idx++ )
	{
		if( m_BrainProcesses(Idx) && m_BrainProcesses(Idx)->FilterAIStim_BrainProcess( _rAIStim ) )
		{
			return;
		}
	}

	UBOOL bFiltered = FALSE;
	UBOOL bNeedsTopmostFiltering = TRUE;
	for( INT Slot = ARRAY_COUNT(m_ActiveBehaviorStack) - 1; Slot >= 0; Slot-- )
	{
		UDishonoredAIBehavior* Behavior = m_ActiveBehaviorStack[Slot];
		if( !Behavior || Behavior->m_bForceFinishDueToDormancy || Behavior->IsBehaviorFinished() )
		{
			continue;
		}
		if( bNeedsTopmostFiltering )
		{
			bFiltered = Behavior->CallFilterAIStim( _rAIStim );
			bNeedsTopmostFiltering = FALSE;
		}
		else if( Behavior->CallShouldFinishWhileDormant( _rAIStim ) )
		{
			_bActiveStackCancels[Slot] = TRUE;
		}
	}
	if( bFiltered )
	{
		return;
	}

	UBOOL bHasAlreadyStartedBehaviorInSlot[ARRAY_COUNT(m_ActiveBehaviorStack)];
	appMemzero( bHasAlreadyStartedBehaviorInSlot, sizeof(bHasAlreadyStartedBehaviorInSlot) );

	for( INT Idx = 0; Idx < m_BehaviorArray.Num(); Idx++ )
	{
		UDishonoredAIBehavior* Behavior = m_BehaviorArray(Idx);
		if( !Behavior )
		{
			continue;
		}
		const INT Slot = Behavior->m_DesignatedSlot;
		if( Slot < 0 || Slot >= ARRAY_COUNT(m_ActiveBehaviorStack) || bHasAlreadyStartedBehaviorInSlot[Slot] )
		{
			continue;
		}
		const BYTE* EvaluateMask = (const BYTE*)Behavior->m_pEvaluateStimMask;
		if( !EvaluateMask || ( EvaluateMask[_rAIStim.m_StimID] & 1 ) == 0 )
		{
			continue;
		}
		const FDisStimPredicateDelegate Evaluate = Behavior->GetEvaluateStimDelegate( _rAIStim.m_StimID );
		if( !Evaluate.IsBound() || !Evaluate( _rAIStim ) )
		{
			continue;
		}

		Behavior->m_ReasonForActivation = _rAIStim.m_StimID;
		if( m_ActiveBehaviorStack[Slot] )
		{
			StopBehaviorInSlot( Slot, FALSE );
		}
		if( m_pCurrentBehavior && Behavior->m_DesignatedSlot > m_pCurrentBehavior->m_DesignatedSlot )
		{
			_bBecamePaused[m_pCurrentBehavior->m_DesignatedSlot] = TRUE;
			m_pCurrentBehavior->CallOnBehaviorPause( FALSE );
			m_pCurrentBehavior = NULL;
		}

		m_ActiveBehaviorStack[Slot] = Behavior;
		Behavior->m_bIsPending = TRUE;
		const FDisStimSetupDelegate Setup = Behavior->GetSetupFromStimDelegate( _rAIStim.m_StimID );
		if( Setup.IsBound() )
		{
			Setup( _rAIStim );
		}
		Behavior->m_bHasStarted = TRUE;
		Behavior->m_bIsPending = FALSE;
		Behavior->OnBehaviorStart();
		Behavior->m_bForceFinishDueToDormancy = FALSE;

		_bActiveStackCancels[Slot] = FALSE;
		_bBecamePaused[Slot] = TRUE;
		bHasAlreadyStartedBehaviorInSlot[Slot] = TRUE;
	}
}

// DISHONORED(port): the 3-byte predicate UDishonoredAIBrain::CanBeDormant that DisHandleAIStim gates on. It is folded
// with UDisBehaviorTriggerAlarm::CanBeDormant (2013 rva 0x322980) in both exes, so retail's own name for it on the
// brain is not recoverable; the body is `return FALSE` (a brain always takes a stim offered to it, and the dormancy
// decision belongs to the behaviour).
UBOOL UDishonoredAIBrain::CanBeDormant() const
{
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x711ce0 (2012 0x74fc90): the knowledge component of the brain's container.
// DISHONORED(bringup): FDisAIKnowledgeComponent is unported, so the container never holds one and this answers NULL,
// which is the same path retail takes for a brain whose container has not been given one.
FDisAIKnowledgeComponent* UDishonoredAIBrain::GetKnowledge() const
{
	return NULL;
}

/*-----------------------------------------------------------------------------
	agent CG: the attention accessors FDisAttentionProxy reads
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x701120 (2012 0x746250) / 0x701140 (0x746270): both forward to the attention process.
// DISHONORED(bringup): UDisAIBrainProcessAttention is not ported, so a brain answers DAL_Unaware and leaves the info
// struct at its zeroed default. That is retail's path for a brain without the process, so every reader is correct; what
// it costs is that no NPC ever raises its attention on anything. The process is 84 functions and is the single next
// piece of AI work - see agentCG.md.
BYTE UDishonoredAIBrain::GetAttentionLevel( const IDisAttentionTargetInterface* _pForTarget ) const
{
	return DAL_Unaware;
}

void UDishonoredAIBrain::GetAttentionProxyInfo( const IDisAttentionTargetInterface* _pForTarget, FDisAttentionProxyInfo& _rResult ) const
{
}

// DISHONORED(port): 2013 rva 0x701060 (2012 0x746190, 4 bytes) / 0x701070 (0x7461a0, 19 bytes): the brain's own top
// enemy proxy, with the overload that also reports the attention level it is held at.
const FDisAttentionProxy& UDishonoredAIBrain::GetTopEnemyProxy() const
{
	return m_TopEnemyAttnTargetProxy;
}

const FDisAttentionProxy& UDishonoredAIBrain::GetTopEnemyProxy( BYTE& _rOutTopEnemyAttentionLevel ) const
{
	_rOutTopEnemyAttentionLevel = m_TopEnemyAttnTargetAttentionLevel;
	return m_TopEnemyAttnTargetProxy;
}

// DISHONORED(port): 2013 rva 0x724280 (2012 0x758f00, 131 bytes): every minimum-attention request this source placed is
// dropped, so an NPC that was forced to keep noticing something stops.
// DISHONORED(bringup): the minimum-attention list lives on UDisAIBrainProcessAttention, which is not ported, so nothing
// has placed a request to clear. Kept as a named entry point because three ported sub-state bodies and
// CallOnBehaviorPause all call it, and a silently missing call there would be harder to find later than this line.
void UDishonoredAIBrain::ClearAllMinAttention( BYTE _LimitType )
{
}
