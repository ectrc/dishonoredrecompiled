// DishonoredGame/src/dishonoredglobalaimanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x8af0b0  public: static void __cdecl UDishonoredGlobalAIManager::InitializePrivateStaticClassUDishonoredGlobalAIManager(void)
//   0x8af0d0  GetNextBrainInList
//   0x8af0e0  GetNextBrainInList_0
//   0x8af0f0  public: void __thiscall UDishonoredGlobalAIManager::PreCommitMapChange(void)
//   0x8af100  public: float __thiscall UDishonoredGlobalAIManager::ExtractReactionDelay(void)const
//   0x8af110  public: float __thiscall UDishonoredGlobalAIManager::ExtractMagicReactionDelay(void)const
//   0x8af120  public: int __thiscall UDishonoredGlobalAIManager::MakeNewAttentionTag(void)
//   0x8af130  private: virtual class UDisTweaksBase * __thiscall ADisMovableLimb::GetTweaks_Derived(void)
//   0x8af140  public: void __thiscall UDishonoredGlobalAIManager::NoteBullying(class ADishonoredNPCPawn const *)
//   0x8af160  public: unsigned int __thiscall UDishonoredGlobalAIManager::IsBullyingAllowed(void)const
//   0x8b1080  public: void __thiscall UDishonoredGlobalAIManager::HandleRelationshipChanged(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)
//   0x8b10f0  public: void __thiscall UDishonoredGlobalAIManager::HandleRelationshipChanged(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8b1160  private: virtual void __thiscall UDishonoredGlobalAIManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8b1190  public: unsigned int __thiscall UDishonoredGlobalAIManager::HasAttentiveBrains(class ADishonoredPawn const *, enum ERelationship, enum EDisAttentionLevel)const
//   0x8b11f0  public: class DisIntrusiveList::TIterator<class UDishonoredAIBrain> __thiscall UDishonoredGlobalAIManager::GetBrainIterator(void)
//   0x8b7a50  public: void __thiscall UDishonoredGlobalAIManager::Engage(class UDishonoredAIBrain * const, class ADishonoredPawn * const)
//   0x8b7b40  public: void __thiscall UDishonoredGlobalAIManager::Disengage(class UDishonoredAIBrain * const)
//   0x8b7c00  public: unsigned int __thiscall UDishonoredGlobalAIManager::IsGroupEngaged(class UDishonoredAIBrain * const, class UDishonoredAIBrain * *)const
//   0x8b7d70  public: void __thiscall UDishonoredGlobalAIManager::AddBrain(class UDishonoredAIBrain *)
//   0x8c48a0  public: void __thiscall UDishonoredGlobalAIManager::CleanUp_GlobalAI(void)
//   0x8c4a50  public: void __thiscall UDishonoredGlobalAIManager::RemoveBrain(class UDishonoredAIBrain *)
//   0x8c4ae0  public: void __thiscall UDishonoredGlobalAIManager::GetAttentiveBrains(class ADishonoredPawn const *, struct TMemStackArray<class UDishonoredAIBrain *> &, enum ERelationship, enum EDisAttentionLevel)
//   0x8c4b80  public: void __thiscall UDishonoredGlobalAIManager::GetAttentiveBrains(class ADishonoredPawn const *, struct TMemStackArray<class UDishonoredAIBrain const *> &, enum ERelationship, enum EDisAttentionLevel)const
//   0x8c4c00  public: void __thiscall UDishonoredGlobalAIManager::OnOtherActorTerminated_GlobalAIMan(class AActor const &)
//   0x8cb480  public: void __thiscall UDishonoredGlobalAIManager::OnPawnDestroyed(class ADishonoredPawn * const)
//   0x8cebd0  public: virtual void __thiscall UDishonoredGlobalAIManager::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x8cecd0  public: virtual void __thiscall UDishonoredGlobalAIManager::Serialize(class FArchive &)
//   0x8d1cc0  public: static class UClass * __cdecl UDishonoredGlobalAIManager::GetPrivateStaticClassUDishonoredGlobalAIManager(wchar_t const *)
//   0x8d1d50  public: void __thiscall UDishonoredGlobalAIManager::Init_GlobalAI(void)
//   0x8d1dd0  public: void __thiscall UDishonoredGlobalAIManager::Tick_GlobalAI(float)
//   0x8d41c0  public: static class UClass * __cdecl UDishonoredGlobalAIManager::StaticClassNoInline(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x860430 (2012 0x8cecd0, same bytes): object reference collectors see the corpse map (retail
// serializes the whole TMap; the values are plain data, so the pawn keys are what a collector gets)
void UDishonoredGlobalAIManager::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( Ar.IsObjectReferenceCollector() )
	{
		for( TMap<ADishonoredNPCPawn*,FGlobalCorpseInfo>::TIterator It( m_Corpses ); It; ++It )
		{
			Ar << (UObject*&)It.Key();
		}
	}
}

// ---- agent CG ports (PHASE9 CG): the brain registry ----

// DISHONORED(port): 2013 rva 0x861a90 (2012 0x8d1d50): the engaged-brain slots are sized from the tweaked maximum, the
// corpse list is gathered, and the stim pool and the global blackboard are initialised. This is what
// ADishonoredGameInfo::InitGlobalManagers calls, and until it runs no brain can allocate a stim.
void UDishonoredGlobalAIManager::Init_GlobalAI()
{
	m_EngagedBrains.Empty( m_iMaxEngagedBrains );
	m_EngagedBrains.AddZeroed( m_iMaxEngagedBrains );
	m_iNumEngagedBrains = 0;
	m_pThinkCandidate = NULL;

	// DISHONORED(bringup): GatherAllDeadNPCs (2013 rva 0x861360) walks the world for corpses so a loaded game keeps its
	// corpse budget; the corpse tracking half of this class is not ported (dishonoredglobalaimanager_corpse.cpp is a
	// comment-only skeleton), so the corpse list starts empty, which is correct for a fresh level.

	if( m_pStimManager )
	{
		m_pStimManager->InitStimManager( m_MaxAIStimSize_bytes, m_MaxNumAIStims );
	}
	else
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredGlobalAIManager::Init_GlobalAI has no UDisStimManager sub-object; no brain will be able to raise a stim") );
	}

	// DISHONORED(bringup): UDisAIBlackboard::InitBlackboard (2013 rva 0x736f90) needs the blackboard record classes,
	// which agent AJ left unported (UDisAIBlackboard::Serialize carries the same note).
	m_bInitialized = TRUE;
}

// DISHONORED(port): 2013 rva 0x847890 (2012 0x8b7d70): the new brain becomes the head of the intrusive list. Retail
// only writes the new brain's next pointer when the list was non-empty, relying on it already being NULL; that is kept,
// because a brain whose next pointer was stale would otherwise be silently re-linked.
void UDishonoredGlobalAIManager::AddBrain( UDishonoredAIBrain* _pAddMe )
{
	if( !_pAddMe )
	{
		return;
	}
	if( m_pBrainList )
	{
		_pAddMe->m_pGlobalAI_NextBrain = m_pBrainList;
	}
	m_pBrainList = _pAddMe;
}

// DISHONORED(port): 2013 rva 0x857190 (2012 0x8c4a50): unlink and forget, then the corpse half is told. The removed
// brain's own next pointer is always cleared, even when it was not on the list at all.
void UDishonoredGlobalAIManager::RemoveBrain( UDishonoredAIBrain* _pRemoveMe )
{
	if( !_pRemoveMe )
	{
		return;
	}
	if( m_pBrainList == _pRemoveMe )
	{
		m_pBrainList = _pRemoveMe->m_pGlobalAI_NextBrain;
	}
	else
	{
		for( UDishonoredAIBrain* Brain = m_pBrainList; Brain; Brain = Brain->m_pGlobalAI_NextBrain )
		{
			if( Brain->m_pGlobalAI_NextBrain == _pRemoveMe )
			{
				Brain->m_pGlobalAI_NextBrain = _pRemoveMe->m_pGlobalAI_NextBrain;
				break;
			}
		}
	}
	_pRemoveMe->m_pGlobalAI_NextBrain = NULL;
	// DISHONORED(bringup): OnBrainRemoved_Corpse (2013 rva 0x851e20) closes retail's body; the corpse half of this
	// class is not ported.
}

// DISHONORED(written): the census of agentCG.md counts the list rather than the world, so that a brain that exists but
// whose pawn has gone is still seen.
INT UDishonoredGlobalAIManager::GetNumBrains() const
{
	INT Count = 0;
	for( UDishonoredAIBrain* Brain = m_pBrainList; Brain; Brain = Brain->m_pGlobalAI_NextBrain )
	{
		Count++;
	}
	return Count;
}
