// DishonoredGame/src/disseqact_startspawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x7cdbd0  public: virtual void __thiscall UDisSeqAct_StartSpawn::OnPawnSpawned(class ADishonoredNPCPawn *)
//   0x7cdbe0  public: virtual void __thiscall UDisSeqAct_StartSpawn::OnSpawnFailed(int)
//   0x7d2dd0  public: virtual unsigned int __thiscall UDisSeqAct_StartSpawn::UpdateOp(float)
//   0x7d7480  public: virtual void __thiscall UDisSeqAct_StartSpawn::Activated(void)
//   0x7eb170  public: static class UClass * __cdecl UDisSeqAct_StartSpawn::GetPrivateStaticClassUDisSeqAct_StartSpawn(wchar_t const *)
//   0x7ee2f0  public: static void __cdecl UDisSeqAct_StartSpawn::InitializePrivateStaticClassUDisSeqAct_StartSpawn(void)
//   0x7ef3f0  public: static class UClass * __cdecl UDisSeqAct_StartSpawn::StaticClassNoInline(void)
//   0x7f0c80  public: virtual void __thiscall UDisSeqAct_StartSpawn::PostLoad(void)

// ---- agent FI (PHASE14 FI) ----
#include "DishonoredGame.h"

/**
 * DISHONORED(port): agent FI, 2013 rva 0x79bcc0 (2012 0x7d7480), 305 bytes. Count the spawners this action
 * is pointed at, then let the latent machinery attach itself to them and pulse output 0 ("Out").
 *
 * The count is what makes the op latent: UpdateOp holds until every spawner has answered. Each linked
 * variable is walked with USequenceVariable::GetObjectRef until it answers NULL, which is how a
 * SeqVar_Object list and a single SeqVar_Object both work; only objects whose class descends from
 * ADishonoredSpawner are counted, exactly as retail's class walk does.
 */
void UDisSeqAct_StartSpawn::Activated()
{
	for( INT LinkIndex = 0; LinkIndex < VariableLinks.Num(); LinkIndex++ )
	{
		const TArray<USequenceVariable*>& Linked = VariableLinks(LinkIndex).LinkedVariables;
		for( INT VarIndex = 0; VarIndex < Linked.Num(); VarIndex++ )
		{
			USequenceVariable* Variable = Linked(VarIndex);
			if( Variable == NULL )
			{
				continue;
			}
			for( INT ObjIndex = 0; ; ObjIndex++ )
			{
				UObject** ObjRef = Variable->GetObjectRef( ObjIndex );
				if( ObjRef == NULL )
				{
					break;
				}
				if( *ObjRef != NULL && (*ObjRef)->IsA( ADishonoredSpawner::StaticClass() ) )
				{
					m_PendingSpawnCount++;
				}
			}
		}
	}
	Super::Activated();
	if( OutputLinks.Num() > 0 )
	{
		OutputLinks(0).ActivateOutputLink();
	}
	debugf( TEXT("DISHONORED(bringup): StartSpawn %s: %d spawner(s) asked, spawnEvenIfVisible %d, latent actors %d"),
		*GetName(), m_PendingSpawnCount, (INT)m_bSpawnEvenIfVisible, LatentActors.Num() );
}

/**
 * DISHONORED(port): agent FI, 2013 rva 0x790050 (2012 0x7d2dd0), 293 bytes. One pulse of output 2 per pawn
 * that arrived and one of output 3 per refusal, then the op finishes - and pulses output 1 - once no spawn
 * is outstanding.
 */
UBOOL UDisSeqAct_StartSpawn::UpdateOp( FLOAT /*DeltaTime*/ )
{
	for( ; m_SpawnedCount > 0; m_SpawnedCount-- )
	{
		if( OutputLinks.Num() > 2 )
		{
			OutputLinks(2).ActivateOutputLink();
		}
	}
	for( ; m_FailedCount > 0; m_FailedCount-- )
	{
		if( OutputLinks.Num() > 3 )
		{
			OutputLinks(3).ActivateOutputLink();
		}
	}
	if( m_PendingSpawnCount > 0 )
	{
		return FALSE;
	}
	if( OutputLinks.Num() > 1 )
	{
		OutputLinks(1).ActivateOutputLink();
	}
	return TRUE;
}

/** DISHONORED(port): agent FI, 2013 rva 0x78aca0 (2012 0x7cdbd0), 15 bytes. */
void UDisSeqAct_StartSpawn::OnPawnSpawned( ADishonoredNPCPawn* /*Pawn*/ )
{
	m_PendingSpawnCount--;
	if( OutputLinks.Num() > 2 )
	{
		m_SpawnedCount++;
	}
}

/** DISHONORED(port): agent FI, 2013 rva 0x78acb0 (2012 0x7cdbe0), 15 bytes. */
void UDisSeqAct_StartSpawn::OnSpawnFailed( INT /*Reason*/ )
{
	m_PendingSpawnCount--;
	if( OutputLinks.Num() > 3 )
	{
		m_FailedCount++;
	}
}

/**
 * DISHONORED(bringup, agent FI): the link anchor. DishonoredGameModule is a static library, MSVC takes a
 * member of one only to resolve an undefined symbol, and the vtable IMPLEMENT_CLASS emits is a COMDAT the
 * linker folds rather than an external reference into this unit - so without a symbol something always
 * linked names, this whole object file is dropped and the four bodies above are silently not in the
 * executable. Measured: the first build of this package compiled disseqact_startspawn.cpp and the exe did
 * not contain one of its strings. Referenced from DishonoredGameRegistrants.cpp.
 */
void DishonoredLinkDisSeqActStartSpawn()
{
}
