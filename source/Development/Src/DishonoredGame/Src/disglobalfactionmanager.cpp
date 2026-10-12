// DishonoredGame/src/disglobalfactionmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (37), 2012 rvas:
//   0x8aecf0  public: static void __cdecl UDisGlobalFactionManager::InitializePrivateStaticClassUDisGlobalFactionManager(void)
//   0x8b6f50  public: virtual void __thiscall UDisGlobalFactionManager::BeginDestroy(void)
//   0x8b6f80  private: enum ERelationship __thiscall UDisGlobalFactionManager::GetFactionRelationshipToFaction(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8c7d00  public: struct FDisRelationshipOverrideInfo const * __thiscall UDisGlobalFactionManager::GetRelationships(class UDisTweaks_Faction const &)const
//   0x8c7d40  private: unsigned int __thiscall UDisGlobalFactionManager::GetGlobalIndividualOverride(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &, enum ERelationship &)const
//   0x8cb0b0  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &, class IDisRelationshipInterface const &)const
//   0x8ce0e0  public: void __thiscall FDisDetermineRelationshipFactionMap::AddReferencedObjects(class UObject &, class TArray<class UObject *, class FDefaultAllocator> &)const
//   0x8ce160  public: void __thiscall FDisDetermineRelationshipFactionMap::Serialize(class FArchive &)
//   0x8ce250  public: virtual void __thiscall UDisGlobalFactionManager::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x8ce3d0  public: virtual void __thiscall UDisGlobalFactionManager::Serialize(class FArchive &)
//   0x8ce6b0  public: virtual void __thiscall UDisGlobalFactionManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8ce7d0  private: unsigned int __thiscall UDisGlobalFactionManager::CachedIsFallbackDerivedFrom(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8ce860  private: void __thiscall UDisGlobalFactionManager::ClearDerivedFactions(struct FDisDetermineRelationshipIndividualMap &, class UDisTweaks_Faction const &)const
//   0x8ce970  private: void __thiscall UDisGlobalFactionManager::ClearDerivedFactions(struct FDisDetermineRelationshipFactionMap &, class UDisTweaks_Faction const &)const
//   0x8cea70  private: unsigned int __thiscall UDisGlobalFactionManager::IsTrespassing(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8ceaf0  private: enum ERelationship __thiscall UDisGlobalFactionManager::GetFactionRelationshipToIndividual(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8d0570  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &, class UDisTweaks_Faction const &)const
//   0x8d05c0  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class IDisRelationshipInterface const &, class UDisTweaks_Faction const &)const
//   0x8d0710  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8d1c60  public: void __thiscall UDisGlobalFactionManager::AddTrespasserForFaction(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d1c90  public: void __thiscall UDisGlobalFactionManager::RemoveTrespasserForFaction(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d7730  public: void __thiscall UDisGlobalFactionManager::RebuildCaches(void)
//   0x8d7810  private: enum ERelationship __thiscall UDisGlobalFactionManager::DetermineRelationship(class IDisRelationshipInterface const &, class IDisRelationshipInterface const &)const
//   0x8d7960  public: void __thiscall UDisGlobalFactionManager::OverrideRelationToFaction(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &, enum ERelationship)
//   0x8d7ba0  public: void __thiscall UDisGlobalFactionManager::OverrideRelationToIndividual(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &, enum ERelationship)
//   0x8d7de0  public: void __thiscall UDisGlobalFactionManager::ClearRelationToFactionOverride(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)
//   0x8d7e60  public: void __thiscall UDisGlobalFactionManager::ClearRelationToIndividualOverride(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d7ee0  public: void __thiscall UDisGlobalFactionManager::ClearAllRelationOverrides(void)
//   0x8d7f10  public: virtual void __thiscall UDisGlobalFactionManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8d8090  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &)const
//   0x8d80a0  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class UDisTweaks_Faction const &)const
//   0x8d82e0  public: enum ERelationship __thiscall UDisGlobalFactionManager::DetermineRelationship(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8d8470  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class IDisRelationshipInterface const &)const
//   0x8d88d0  public: void __thiscall UDisGlobalFactionManager::OnIndividualDestroyed(class IDisRelationshipInterface const &)
//   0x8d8a30  public: __thiscall UDisGlobalFactionManager::UDisGlobalFactionManager(void)
//   0x8d8c00  public: static class UClass * __cdecl UDisGlobalFactionManager::GetPrivateStaticClassUDisGlobalFactionManager(wchar_t const *)
//   0x8d8c90  public: static class UClass * __cdecl UDisGlobalFactionManager::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent FV (PHASE18 FV): the global faction manager's override table and the cache clears the relationship API reaches.
	GameSave / GameLoad are agent ED's, in dissavegame.cpp.

	The manager keeps one FDisRelationshipOverrideInfo per faction (m_GlobalRelationships: what faction A thinks of
	faction B or of individual X, over the factions' authored ally / enemy lists), and two caches of answered questions
	keyed by the asking individual: m_DetermineIndividualRelationshipCache (individual -> individual) and
	m_DetermineFactionRelationshipCache (faction -> individual). Every override write clears the cached answers it can
	have changed, by walking the keys and dropping those whose faction is fallback-derived from the faction written.

	What is NOT here, each with what it costs:
	  * DetermineRelationship's faction tables (GetFactionRelationshipToIndividual 0x8ceaf0, GetFactionRelationshipToFaction
	    0x8b6f80, IsTrespassing 0x8cea70, GetGlobalIndividualOverride 0x8c7d40 - the authored ally / enemy answer). The
	    two DetermineRelationship entry points answer Neutral and fill no cache; the override writers below are exact,
	    so an override written now is read the moment the tables land.
	  * CachedIsFallbackDerivedFrom's cache (m_FactionRelationshipCache keyed by FDisFactionPair): the uncached
	    UDisTweaksBase::IsFallbackDerivedFrom answers the same question.
	  * RebuildCaches, OnIndividualDestroyed, the trespasser list, UDishonoredGlobalAIManager::HandleRelationshipChanged
	    (0x8b10f0: every brain of the written faction drops its own override of the individual).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

UDisGlobalFactionManager* UDisGlobalFactionManager::s_pInstance = NULL;

// DISHONORED(port): 2013 rva 0x85c8a0 (2012 0x8cb0b0): one cached individual -> individual answer.
void UDisGlobalFactionManager::ClearIndividualCache( const IDisRelationshipInterface& _rFromIndividual, const IDisRelationshipInterface& _rToIndividual )
{
	FDisDetermineRelationshipIndividualMap* Cache = m_DetermineIndividualRelationshipCache.Find( const_cast<IDisRelationshipInterface*>( &_rFromIndividual ) );
	if( Cache )
	{
		Cache->m_IndividualRelationships.Remove( const_cast<IDisRelationshipInterface*>( &_rToIndividual ) );
	}
}

// DISHONORED(port): 2013 rva 0x860ee0 (2012 0x8d0570): every cached answer of the individual about a member of the faction.
void UDisGlobalFactionManager::ClearIndividualCache( const IDisRelationshipInterface& _rFromIndividual, const UDisTweaks_Faction& _rToFaction )
{
	FDisDetermineRelationshipIndividualMap* Cache = m_DetermineIndividualRelationshipCache.Find( const_cast<IDisRelationshipInterface*>( &_rFromIndividual ) );
	if( Cache )
	{
		ClearDerivedFactions( *Cache, _rToFaction );
	}
}

// DISHONORED(port): 2013 rva 0x8631f0 (2012 0x8d8090): everything the individual ever asked.
void UDisGlobalFactionManager::ClearIndividualCache( const IDisRelationshipInterface& _rFromIndividual )
{
	m_DetermineIndividualRelationshipCache.Remove( const_cast<IDisRelationshipInterface*>( &_rFromIndividual ) );
}

// DISHONORED(port): 2012 rva 0x8d05c0 (324 B; unnamed in 2013): every member of the faction forgets what it thought of the
// individual, and the faction's own cached answers about the individual go too.
void UDisGlobalFactionManager::ClearFactionCache( const IDisRelationshipInterface& _rToIndividual, const UDisTweaks_Faction& _rFromFaction )
{
	for( TMap<IDisRelationshipInterface*,FDisDetermineRelationshipIndividualMap>::TIterator It(m_DetermineIndividualRelationshipCache); It; ++It )
	{
		const UDisTweaks_Faction* Faction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( Faction && CachedIsFallbackDerivedFrom( *Faction, _rFromFaction ) )
		{
			It.Value().m_IndividualRelationships.Remove( const_cast<IDisRelationshipInterface*>( &_rToIndividual ) );
		}
	}
	FDisDetermineRelationshipFactionMap* FactionCache = m_DetermineFactionRelationshipCache.Find( const_cast<IDisRelationshipInterface*>( &_rToIndividual ) );
	if( FactionCache )
	{
		ClearDerivedFactions( *FactionCache, _rFromFaction );
	}
}

// DISHONORED(port): 2012 rvas 0x8d0710 (579 B) / 0x8d80a0 (530 B), unnamed in 2013: a faction -> faction write, or a
// faction's row going away. Retail walks both caches and drops every entry whose asking or answered faction derives
// from the one written; the two-faction form keeps the entries of askers outside the first faction.
void UDisGlobalFactionManager::ClearFactionCache( const UDisTweaks_Faction& _rFromFaction, const UDisTweaks_Faction& _rToFaction )
{
	for( TMap<IDisRelationshipInterface*,FDisDetermineRelationshipIndividualMap>::TIterator It(m_DetermineIndividualRelationshipCache); It; ++It )
	{
		const UDisTweaks_Faction* Faction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( Faction && CachedIsFallbackDerivedFrom( *Faction, _rFromFaction ) )
		{
			ClearDerivedFactions( It.Value(), _rToFaction );
		}
	}
	for( TMap<IDisRelationshipInterface*,FDisDetermineRelationshipFactionMap>::TIterator It(m_DetermineFactionRelationshipCache); It; ++It )
	{
		const UDisTweaks_Faction* Faction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( Faction && CachedIsFallbackDerivedFrom( *Faction, _rToFaction ) )
		{
			ClearDerivedFactions( It.Value(), _rFromFaction );
		}
	}
}

void UDisGlobalFactionManager::ClearFactionCache( const UDisTweaks_Faction& _rFromFaction )
{
	for( TMap<IDisRelationshipInterface*,FDisDetermineRelationshipIndividualMap>::TIterator It(m_DetermineIndividualRelationshipCache); It; ++It )
	{
		const UDisTweaks_Faction* Faction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( Faction && CachedIsFallbackDerivedFrom( *Faction, _rFromFaction ) )
		{
			It.RemoveCurrent();
		}
	}
	for( TMap<IDisRelationshipInterface*,FDisDetermineRelationshipFactionMap>::TIterator It(m_DetermineFactionRelationshipCache); It; ++It )
	{
		ClearDerivedFactions( It.Value(), _rFromFaction );
	}
}

// DISHONORED(port): 2012 rvas 0x8ce860 / 0x8ce970 (unnamed in 2013): the entries of one cache row whose faction derives
// from the base faction.
void UDisGlobalFactionManager::ClearDerivedFactions( FDisDetermineRelationshipIndividualMap& _rIndividualMap, const UDisTweaks_Faction& _rBaseFaction )
{
	for( TMap<IDisRelationshipInterface*,ERelationship>::TIterator It(_rIndividualMap.m_IndividualRelationships); It; ++It )
	{
		const UDisTweaks_Faction* Faction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( Faction && CachedIsFallbackDerivedFrom( *Faction, _rBaseFaction ) )
		{
			It.RemoveCurrent();
		}
	}
}

void UDisGlobalFactionManager::ClearDerivedFactions( FDisDetermineRelationshipFactionMap& _rFactionMap, const UDisTweaks_Faction& _rBaseFaction )
{
	for( TMap<UDisTweaks_Faction*,ERelationship>::TIterator It(_rFactionMap.m_FactionRelationships); It; ++It )
	{
		if( It.Key() && CachedIsFallbackDerivedFrom( *It.Key(), _rBaseFaction ) )
		{
			It.RemoveCurrent();
		}
	}
}

// DISHONORED(port): 2013 rva 0x85f9c0 (2012 0x8ce7d0): retail memoises UDisTweaksBase::IsFallbackDerivedFrom per faction pair
// in m_FactionRelationshipCache; the uncached answer is the same answer.
UBOOL UDisGlobalFactionManager::CachedIsFallbackDerivedFrom( const UDisTweaks_Faction& _rDerivedFaction, const UDisTweaks_Faction& _rBaseFaction ) const
{
	return _rDerivedFaction.IsFallbackDerivedFrom( &_rBaseFaction );
}

// DISHONORED(port): 2013 rva 0x863bb0 (2012 0x8d7e60): the faction's override of one individual goes; an emptied row goes
// with it.
void UDisGlobalFactionManager::ClearRelationToIndividualOverride( const UDisTweaks_Faction& _rFromFaction, const IDisRelationshipInterface& _rToIndividual )
{
	UDisTweaks_Faction* FromFaction = const_cast<UDisTweaks_Faction*>( &_rFromFaction );
	FDisRelationshipOverrideInfo* Row = m_GlobalRelationships.Find( FromFaction );
	if( Row && Row->ClearIndividualOverride( _rToIndividual ) )
	{
		if( !Row->HasAnyOverride() )
		{
			m_GlobalRelationships.Remove( FromFaction );
		}
		ClearFactionCache( _rToIndividual, _rFromFaction );
	}
}

// DISHONORED(port): 2013 rva 0x8638e0 (2012 0x8d7ba0, 576 B): every row of a faction derived from the writer drops its own
// override of the individual, the writer's row is created if it has none, and the override is written; a change clears
// the faction's cached answers about the individual and tells the global AI manager.
void UDisGlobalFactionManager::OverrideRelationToIndividual( const UDisTweaks_Faction& _rFromFaction, const IDisRelationshipInterface& _rToIndividual, BYTE _Relationship )
{
	UDisTweaks_Faction* FromFaction = const_cast<UDisTweaks_Faction*>( &_rFromFaction );
	UBOOL bChanged = FALSE;
	for( TMap<UDisTweaks_Faction*,FDisRelationshipOverrideInfo>::TIterator It(m_GlobalRelationships); It; ++It )
	{
		if( It.Key() && It.Key()->IsFallbackDerivedFrom( FromFaction ) && It.Value().ClearIndividualOverride( _rToIndividual ) )
		{
			bChanged = TRUE;
		}
	}
	FDisRelationshipOverrideInfo* Row = m_GlobalRelationships.Find( FromFaction );
	if( !Row )
	{
		Row = &m_GlobalRelationships.Set( FromFaction, FDisRelationshipOverrideInfo(EC_EventParm) );
	}
	if( Row->OverrideIndividualRelationship( _rToIndividual, _Relationship ) || bChanged )
	{
		ClearFactionCache( _rToIndividual, _rFromFaction );
		// DISHONORED(bringup): UDishonoredGlobalAIManager::HandleRelationshipChanged( faction, individual ) (2012 0x8b10f0) is
		// not ported: every brain whose pawn's faction derives from the writer would drop its own override of the individual.
	}
}

// DISHONORED(port): 2013 rva 0x863de0 (2012 0x8d7de0, 115 B): the faction -> faction form of the clear above.
void UDisGlobalFactionManager::ClearRelationToFactionOverride( const UDisTweaks_Faction& _rFromFaction, const UDisTweaks_Faction& _rToFaction )
{
	UDisTweaks_Faction* FromFaction = const_cast<UDisTweaks_Faction*>( &_rFromFaction );
	FDisRelationshipOverrideInfo* Row = m_GlobalRelationships.Find( FromFaction );
	if( Row && Row->ClearFactionOverride( _rToFaction ) )
	{
		if( !Row->HasAnyOverride() )
		{
			m_GlobalRelationships.Remove( FromFaction );
		}
		ClearFactionCache( _rFromFaction, _rToFaction );
	}
}

// DISHONORED(port): 2013 rva 0x863960 (2012 0x8d7960, 561 B): the faction -> faction form of the override above.
void UDisGlobalFactionManager::OverrideRelationToFaction( const UDisTweaks_Faction& _rFromFaction, const UDisTweaks_Faction& _rToFaction, BYTE _Relationship )
{
	UDisTweaks_Faction* FromFaction = const_cast<UDisTweaks_Faction*>( &_rFromFaction );
	UBOOL bChanged = FALSE;
	for( TMap<UDisTweaks_Faction*,FDisRelationshipOverrideInfo>::TIterator It(m_GlobalRelationships); It; ++It )
	{
		if( It.Key() && It.Key()->IsFallbackDerivedFrom( FromFaction ) && It.Value().ClearFactionOverride( _rToFaction ) )
		{
			bChanged = TRUE;
		}
	}
	FDisRelationshipOverrideInfo* Row = m_GlobalRelationships.Find( FromFaction );
	if( !Row )
	{
		Row = &m_GlobalRelationships.Set( FromFaction, FDisRelationshipOverrideInfo(EC_EventParm) );
	}
	if( Row->OverrideFactionRelationship( _rToFaction, _Relationship ) || bChanged )
	{
		ClearFactionCache( _rFromFaction, _rToFaction );
		// DISHONORED(bringup): UDishonoredGlobalAIManager::HandleRelationshipChanged( faction, faction ) (2012 0x8b1080) is not
		// ported; see OverrideRelationToIndividual.
	}
}

// DISHONORED(port): 2012 rvas 0x8d7810 (327 B) / 0x8d82e0 (385 B), 2013 0x862f20 / unnamed. The two questions every caller
// asks, each memoised per asker. See the head of the unit: the faction tables behind the cache miss are not ported, so
// the individual override is consulted and the rest answers Neutral, and nothing is cached - a cache of Neutrals would
// outlive the tables' arrival.
BYTE UDisGlobalFactionManager::DetermineRelationship( const IDisRelationshipInterface& _rFromIndividual, const IDisRelationshipInterface& _rToIndividual )
{
	const FDisDetermineRelationshipIndividualMap* Cache = m_DetermineIndividualRelationshipCache.Find( const_cast<IDisRelationshipInterface*>( &_rFromIndividual ) );
	if( Cache )
	{
		const ERelationship* Cached = Cache->m_IndividualRelationships.Find( const_cast<IDisRelationshipInterface*>( &_rToIndividual ) );
		if( Cached )
		{
			return *Cached;
		}
	}
	UDisTweaks_Faction* Faction = _rFromIndividual.GetFactionTweak();
	if( !Faction )
	{
		return Relationship_Neutral;
	}
	BYTE Result = Relationship_Neutral;
	FDisRelationshipOverrideInfo* Overrides = const_cast<IDisRelationshipInterface&>( _rFromIndividual ).GetRelationshipOverrideInfo();
	if( Overrides && Overrides->GetIndividualOverride( _rToIndividual, Result ) )
	{
		return Result;
	}
	return DetermineRelationship( *Faction, _rToIndividual );
}

BYTE UDisGlobalFactionManager::DetermineRelationship( const UDisTweaks_Faction& _rFromFaction, const IDisRelationshipInterface& _rToIndividual )
{
	const FDisDetermineRelationshipFactionMap* Cache = m_DetermineFactionRelationshipCache.Find( const_cast<IDisRelationshipInterface*>( &_rToIndividual ) );
	if( Cache )
	{
		const ERelationship* Cached = Cache->m_FactionRelationships.Find( const_cast<UDisTweaks_Faction*>( &_rFromFaction ) );
		if( Cached )
		{
			return *Cached;
		}
	}
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalFactionManager::DetermineRelationship: the faction tables (IsTrespassing 2012 0x8cea70, GetGlobalIndividualOverride 0x8c7d40, GetFactionRelationshipToIndividual 0x8ceaf0) are not ported; every faction answers Neutral") );
	}
	return Relationship_Neutral;
}
