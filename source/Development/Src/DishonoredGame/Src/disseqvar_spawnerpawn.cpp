// DishonoredGame/src/disseqvar_spawnerpawn.cpp
// PDB functions attributed to this file (6):
//   0x7f3cb0  public: static void __cdecl UDisSeqVar_SpawnerPawn::InitializePrivateStaticClassUDisSeqVar_SpawnerPawn(void)
//   0x7f7c10  public: virtual void __thiscall UDisSeqVar_SpawnerPawn::DrawExtraInfo(class FCanvas *, class FVector const &)
//   0x807420  public: virtual class UObject * * __thiscall UDisSeqVar_SpawnerPawn::GetObjectRef(int)
//   0x807560  public: virtual class FString __thiscall UDisSeqVar_SpawnerPawn::GetValueStr(void)
//   0x8147b0  public: static class UClass * __cdecl UDisSeqVar_SpawnerPawn::GetPrivateStaticClassUDisSeqVar_SpawnerPawn(wchar_t const *)
//   0x816c20  public: static class UClass * __cdecl UDisSeqVar_SpawnerPawn::StaticClassNoInline(void)
// (the 2012 rvas above are the import tool's; GetObjectValue 2012 0x7f3c50 is attributed to disseqvar_playerpawn.cpp)
//
// DISHONORED(port): agent FN, wave 20. 2013 rvas, resolved by hand in retail2013_named.i64 (image base 0x400000):
//   GetObjectRef    0x7a2f30, 316 bytes   GetObjectValue 0x786c00, 24 bytes   GetValueStr 0x79cc90, 257 bytes
// DrawExtraInfo (2013 0x792000, 518 bytes) is editor drawing and is not ported.

#include "DishonoredGame.h"

// IMPLEMENT_CLASS(UDisSeqVar_SpawnerPawn) stays in DishonoredGameRegistrants.cpp, as for every registered class.

// Retail: ObjValue is the ADishonoredSpawner; index Idx reads the spawner's m_SpawnedPawns(Idx), mirrored into
// this variable's own m_pSpawnedPawns (resized to the spawner's count when it differs) so the returned pointer is
// stable; NULL for no spawner or an index past the spawned pawns, which is what ends GetObjectVars' index loop.
UObject** UDisSeqVar_SpawnerPawn::GetObjectRef( INT Idx )
{
	if( ObjValue == NULL )
	{
		return NULL;
	}
	ADishonoredSpawner* Spawner = Cast<ADishonoredSpawner>( ObjValue );
	if( Spawner == NULL || Idx < 0 || Idx >= Spawner->m_SpawnedPawns.Num() )
	{
		return NULL;
	}
	if( m_pSpawnedPawns.Num() != Spawner->m_SpawnedPawns.Num() )
	{
		m_pSpawnedPawns.Empty( Spawner->m_SpawnedPawns.Num() );
		m_pSpawnedPawns.AddZeroed( Spawner->m_SpawnedPawns.Num() );
	}
	m_pSpawnedPawns(Idx) = Spawner->m_SpawnedPawns(Idx);
	return &m_pSpawnedPawns(Idx);
}

UObject* UDisSeqVar_SpawnerPawn::GetObjectValue()
{
	UObject** Ref = GetObjectRef( 0 );
	return Ref != NULL ? *Ref : NULL;
}

FString UDisSeqVar_SpawnerPawn::GetValueStr()
{
	if( ObjValue != NULL && ObjValue->IsA( ADishonoredSpawner::StaticClass() ) )
	{
		return FString( TEXT("SpawnerPawns: ") ) + Super::GetValueStr();
	}
	return Super::GetValueStr();
}

void UDisSeqVar_SpawnerPawn::execGetObjectValue( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UObject**)Result = GetObjectValue();
}

// DISHONORED(bringup, agent FN): the link anchor, same reason as agent FI's in disseqact_startspawn.cpp - a
// static-library member nothing references by symbol is dropped with its vtable. Referenced from
// DishonoredGameRegistrants.cpp.
void DishonoredLinkDisSeqVarSpawnerPawn()
{
}
