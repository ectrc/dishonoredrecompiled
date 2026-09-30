// DishonoredGame/src/distweaks_npcpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file, 2012 rva of the one ported here:
//   0x7cc540  public: class AActor * __thiscall UDisTweaks_NPCPawn::SpawnActor_WithSpawner(enum eDisTweaksSpawnType, class ADishonoredSpawner *, class FName, class FVector const &, class FRotator const &, class AActor *, unsigned int, unsigned int, class AActor *, class APawn *, unsigned int)

// ---- agent EP ports (PHASE14 EP): the spawn that remembers which spawner made it ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x771bc0 (2012 0x7cc540): three preconditions, then UWorld::SpawnActor with an
// FSpawnNPCPawn_TweakObj rather than an FSpawnActor_TweakObj. The preconditions are retail's and they are not the base
// class's: an NPC-pawn tweak set with no skeletal mesh or no anim tree cannot produce a usable pawn, and a
// fallback-only tweak set never spawns.
AActor* UDisTweaks_NPCPawn::SpawnActor_WithSpawner( BYTE _SpawnType, ADishonoredSpawner* _pSpawner, FName _InName,
	const FVector& _rLocation, const FRotator& _rRotation, AActor* _pTemplate, UBOOL _bNoCollisionFail,
	UBOOL _bRemoteOwned, AActor* _pOwner, APawn* _pInstigator, UBOOL _bNoFail ) const
{
	if( !m_pSkeletalMesh || !m_pAnimTreeTemplate || m_bOnlyUseAsFallback )
	{
		return NULL;
	}
	UClass* SpawnedClass = GetSpawnedObjectClass( _SpawnType );
	if( !SpawnedClass )
	{
		return NULL;
	}
	FSpawnNPCPawn_TweakObj Init( const_cast<UDisTweaks_NPCPawn*>( this ), _pSpawner );
	return GWorld->SpawnActor( SpawnedClass, _InName, _rLocation, _rRotation, _pTemplate, _bNoCollisionFail,
		_bRemoteOwned, _pOwner, _pInstigator, _bNoFail, NULL, &Init );
}
