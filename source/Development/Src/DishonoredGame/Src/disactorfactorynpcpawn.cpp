// DishonoredGame/src/disactorfactorynpcpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9): CanCreateActor, CreateActor, CreateNPCPawn, GetDefaultActor,
// IsEnoughRoomToSpawnDisPawn and the four class-registration helpers.

// ---- agent CG ports (PHASE9 CG): the NPC actor factory ----
//
// The factory is a sub-object of ADishonoredSpawner and is the one place an ADishonoredNPCPawn is actually constructed.
// It spawns through the NPC's own UDisTweaks_NPCPawn when it has one, which matters: UDisTweaksBase::SpawnActor (agent
// AJ's port) passes an FSpawnActor_TweakObj init functor, so the pawn has its tweaks applied before PostBeginPlay rather
// than after - the ordering agent AJ established in wave 4.

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "disaicensus.h"

// DISHONORED(port): 2013 rva 0x74a8c0 (2012 0x7ace80): the class default of the pawn class the spawner chose, falling
// back to the factory's own configured class.
AActor* UDisActorFactoryNPCPawn::GetDefaultActor()
{
	if( m_pNPCPawnClass )
	{
		return (AActor*)m_pNPCPawnClass->GetDefaultObject();
	}
	return Super::GetDefaultActor();
}

// DISHONORED(port): 2013 rva 0x74e470 (2012 0x7b0340): the editor's "can I drop one here" query.
UBOOL UDisActorFactoryNPCPawn::CanCreateActor( FString& OutErrorMsg, UBOOL bFromAssetOnly )
{
	if( !m_pNPCPawnClass )
	{
		OutErrorMsg = TEXT("DisActorFactoryNPCPawn has no NPC pawn class");
		return FALSE;
	}
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x74e4b0 (2012 0x7acec0): a pawn whose class default is static or already deleted is never
// spawned once play has begun; otherwise the tweaks spawn it (so the tweak chain runs before begin-play) or, with no
// tweaks, UWorld::SpawnActor does. Retail then calls the script PostCreateActor event on the factory.
ADishonoredNPCPawn* UDisActorFactoryNPCPawn::CreateNPCPawn( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* DefaultActor = GetDefaultActor();
	if( !DefaultActor )
	{
		return NULL;
	}
	if( GWorld && GWorld->HasBegunPlay() && ( DefaultActor->IsStatic() || DefaultActor->bNoDelete ) )
	{
		return NULL;
	}

	const FRotator NewRotation = Rotation ? *Rotation : DefaultActor->Rotation;
	AActor* Spawned = NULL;
	if( m_pNPCPawnTweaks )
	{
		// DISHONORED(port): agent EP - retail 2013 (0x74e4b0) calls SpawnActor_WithSpawner and NOT the base
		// UDisTweaksBase::SpawnActor, which is how the pawn's FDisSpawnerInfo gets filled (FSpawnNPCPawn_TweakObj::DoInit).
		// Two other differences of retail's call that this now matches: bNoCollisionFail is the spawner's own
		// m_bSpawnDead rather than a constant TRUE, and the factory's m_pSpawner is handed through.
		Spawned = m_pNPCPawnTweaks->SpawnActor_WithSpawner( eDisTweaksSpawnType_InGame, m_pSpawner, NAME_None,
			*Location, NewRotation, NULL, ( m_pSpawner && m_pSpawner->m_bSpawnDead ) ? TRUE : FALSE, FALSE, NULL, NULL, FALSE );
	}
	else if( GWorld )
	{
		Spawned = GWorld->SpawnActor( m_pNPCPawnClass, NAME_None, *Location, NewRotation, NULL, TRUE );
	}

	// DISHONORED(port): retail's tail. PostCreateActor is a script event on UActorFactory that the NPC factory
	// overrides, and it is where a spawned NPC is possessed: Engine's Pawn.PostBeginPlay only self-possesses a pawn
	// PLACED in the level ("pawns spawned during gameplay are not automatically possessed by a controller",
	// Pawn.uc:2229). FindFunction rather than FindFunctionChecked, because the latter appErrors on a class that does not
	// override it - agent AJ's hand-over 4.
	if( Spawned )
	{
		UFunction* PostCreate = FindFunction( FName( TEXT("PostCreateActor"), FNAME_Find ) );
		if( PostCreate )
		{
			eventPostCreateActor( Spawned, ActorFactoryData );
		}
		// A factory class that does not override the event simply has nothing to run here; retail's
		// FindFunctionChecked would appError on it, which is agent AJ's hand-over 4.
	}
	return Cast<ADishonoredNPCPawn>( Spawned );
}

// DISHONORED(port): 2013 rva 0x75dbb0 (2012 0x7bf130): the UActorFactory entry point, and the place an NPC gets its
// mind. It refuses when there is not enough room, creates the pawn, and then - in game, and only for a pawn that was
// not spawned dead or straight to ragdoll - spawns an ADishonoredNPCController at the same place and possesses the pawn
// with it. That possession is what ADishonoredSpawner::OnSpawned then finds and hands to
// ADishonoredNPCController::InitNPC, which builds the brain. Engine's own Pawn.PostBeginPlay cannot do it: it
// self-possesses only a pawn PLACED in the level ("pawns spawned during gameplay are not automatically possessed by a
// controller", Pawn.uc:2229), and retail's APawn has no ControllerClass at all - it is a reference-only member in this
// tree, which is the evidence that Arkane replaced that path with this one.
AActor* UDisActorFactoryNPCPawn::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	if( !m_pNPCPawnTweaks && !m_pNPCPawnClass )
	{
		return NULL;
	}
	// DISHONORED(bringup): IsEnoughRoomToSpawnDisPawn (2013 rva 0x758db0) traces a 36-unit box at the spawn point
	// against the pawn trace flags and answers TRUE when the spot is BLOCKED (retail's call site reads
	// `if( IsEnoughRoomToSpawnDisPawn(...) ) return NULL`, so the name is inverted with respect to its meaning). It is
	// not ported: the trace flag constants it uses (8326 / 8351 depending on the Kismet action) are two of the
	// FDisPrimTraceMask combinations agent AU's follow-up 2 says Engine/Inc/UnLevel.h still has no names for. A spawner
	// whose spawn point is blocked therefore spawns anyway, where retail would have refused.

	ADishonoredNPCPawn* Pawn = CreateNPCPawn( Location, Rotation, ActorFactoryData );
	if( !Pawn || !GIsGame )
	{
		return Pawn;
	}
	// DISHONORED(port): agent EP - retail 2013 tests m_bSpawnDead and **m_bTreatAsKnockedOut** here (0x75dbb0:
	// `(pawn+3280 & 1) == 0 && (pawn+3296 & 1) == 0`, and 0x6590e0 the same pair), not m_bStraightToRagdoll. The two
	// live in the same bitfield word - m_bSpawnDead is bit 0 of FDisSpawnerInfo+76 and m_bStraightToRagdoll bit 1 - which
	// is how they were confused. It was invisible while nothing filled FDisSpawnerInfo; the moment
	// FSpawnNPCPawn_TweakObj::DoInit did, m_bStraightToRagdoll was TRUE for every NPC with no dead-pose animation (which
	// is all of them) and the census read "26 NPC pawns, 0 controllers".
	if( Pawn->m_SpawnerInfo.m_bSpawnDead || Pawn->m_SpawnerInfo.m_bTreatAsKnockedOut )
	{
		return Pawn;
	}

	const FRotator NewRotation = Rotation ? *Rotation : Pawn->Rotation;
	ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>(
		GWorld->SpawnActor( ADishonoredNPCController::StaticClass(), NAME_None, *Location, NewRotation, NULL, TRUE ) );
	if( NPCController )
	{
		NPCController->Possess( Pawn );
	}
	else
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisActorFactoryNPCPawn::CreateActor could not spawn an ADishonoredNPCController for %s"), *Pawn->GetName() );
	}
	return Pawn;
}
