// DishonoredGame/src/disglobalenums.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x887d60  public: static void __cdecl UDisGlobalEnums::InitializePrivateStaticClassUDisGlobalEnums(void)
//   0x887d80  public: struct OC3Ent::Face::FxCompiledFaceGraph & __thiscall OC3Ent::Face::FxActor::GetCompiledFaceGraph(void)
//   0x88ff20  public: static class UClass * __cdecl UDisGlobalEnums::GetPrivateStaticClassUDisGlobalEnums(wchar_t const *)
//   0x892e40  public: static class UClass * __cdecl UDisGlobalEnums::StaticClassNoInline(void)
//   0x894330  public: unsigned int __thiscall FDisRelationshipOverrideInfo::HasAnyOverride(void)const
//   0x89c420  public: unsigned int __thiscall FDisRelationshipOverrideInfo::ClearIndividualOverride(class IDisRelationshipInterface const &)
//   0x89f880  public: void __thiscall FDisRelationshipOverrideInfo::AddReferencedObjects(class UObject &, class TArray<class UObject *, class FDefaultAllocator> &)const
//   0x89f900  public: void __thiscall FDisRelationshipOverrideInfo::Serialize(class FArchive &)
//   0x89f9f0  public: unsigned int __thiscall FDisRelationshipOverrideInfo::GetIndividualOverride(class IDisRelationshipInterface const &, enum ERelationship &)const
//   0x89fae0  public: unsigned int __thiscall FDisRelationshipOverrideInfo::OverrideIndividualRelationship(class IDisRelationshipInterface const &, enum ERelationship)
//   0x89fb60  public: unsigned int __thiscall FDisRelationshipOverrideInfo::ClearFactionOverride(class UDisTweaks_Faction const &)
//   0x89fc60  public: void __thiscall FDisRelationshipOverrideInfo::ClearAllOverrides(void)
//   0x89fc80  public: void __thiscall FDisRelationshipOverrideInfo::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x89fef0  public: void __thiscall FDisRelationshipOverrideInfo::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8a0670  public: unsigned int __thiscall FDisRelationshipOverrideInfo::OverrideFactionRelationship(class UDisTweaks_Faction const &, enum ERelationship)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): FDisRelationshipOverrideInfo::Serialize 2013 rva 0x85e3d0 (2012 0x89f900, disglobalenums.cpp:29): only
// an object reference collector sees the faction keys of m_FactionOverrideMap (the individual map holds interface pointers)
void DisSerializeRelationshipOverrideInfo( FDisRelationshipOverrideInfo& Info, FArchive& Ar )
{
	if( !Ar.IsObjectReferenceCollector() )
	{
		return;
	}
	for( TMap<UDisTweaks_Faction*,ERelationship>::TIterator It( Info.m_FactionOverrideMap ); It; ++It )
	{
		Ar << (UObject*&)It.Key();
	}
}

// ---- agent FV (PHASE18 FV): FDisRelationshipOverrideInfo, the override writers the relationship API reaches ----

// DISHONORED(port): 2013 rva 0x85c880 (2012 0x89c420)
UBOOL FDisRelationshipOverrideInfo::ClearIndividualOverride( const IDisRelationshipInterface& _rIndividual )
{
	return m_IndividualOverrideMap.Remove( const_cast<IDisRelationshipInterface*>( &_rIndividual ) ) > 0;
}

// DISHONORED(port): 2013 rva 0x85e610 (2012 0x89fb60, 227 B): every faction override derived from the faction goes.
UBOOL FDisRelationshipOverrideInfo::ClearFactionOverride( const UDisTweaks_Faction& _rFaction )
{
	UBOOL bRemoved = FALSE;
	for( TMap<UDisTweaks_Faction*,ERelationship>::TIterator It( m_FactionOverrideMap ); It; ++It )
	{
		if( It.Key() && It.Key()->IsFallbackDerivedFrom( &_rFaction ) )
		{
			It.RemoveCurrent();
			bRemoved = TRUE;
		}
	}
	return bRemoved;
}

// DISHONORED(port): 2013 rva 0x85e7e0 (2012 0x89fc60)
void FDisRelationshipOverrideInfo::ClearAllOverrides()
{
	m_IndividualOverrideMap.Empty();
	m_FactionOverrideMap.Empty();
}

// DISHONORED(port): 2013 rva 0x85e590 (2012 0x89fae0, 120 B): TRUE when something changed.
UBOOL FDisRelationshipOverrideInfo::OverrideIndividualRelationship( const IDisRelationshipInterface& _rIndividual, BYTE _Relationship )
{
	IDisRelationshipInterface* Key = const_cast<IDisRelationshipInterface*>( &_rIndividual );
	ERelationship* Existing = m_IndividualOverrideMap.Find( Key );
	if( Existing )
	{
		if( *Existing == (ERelationship)_Relationship )
		{
			return FALSE;
		}
		*Existing = (ERelationship)_Relationship;
		return TRUE;
	}
	m_IndividualOverrideMap.Set( Key, (ERelationship)_Relationship );
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x860c40 (2012 0x8a0670, 448 B): the derived faction overrides go first (ClearFactionOverride),
// then every individual override whose faction derives from the one written, then the write itself; TRUE when any of
// the three changed something.
UBOOL FDisRelationshipOverrideInfo::OverrideFactionRelationship( const UDisTweaks_Faction& _rFaction, BYTE _Relationship )
{
	UDisTweaks_Faction* Key = const_cast<UDisTweaks_Faction*>( &_rFaction );
	UBOOL bChanged = ClearFactionOverride( _rFaction );
	for( TMap<IDisRelationshipInterface*,ERelationship>::TIterator It( m_IndividualOverrideMap ); It; ++It )
	{
		const UDisTweaks_Faction* IndividualFaction = It.Key() ? It.Key()->GetFactionTweak() : NULL;
		if( IndividualFaction && IndividualFaction->IsFallbackDerivedFrom( &_rFaction ) )
		{
			It.RemoveCurrent();
			bChanged = TRUE;
		}
	}
	if( bChanged )
	{
		m_FactionOverrideMap.Set( Key, (ERelationship)_Relationship );
		return TRUE;
	}
	ERelationship* Existing = m_FactionOverrideMap.Find( Key );
	if( Existing )
	{
		if( *Existing == (ERelationship)_Relationship )
		{
			return FALSE;
		}
		*Existing = (ERelationship)_Relationship;
		return TRUE;
	}
	m_FactionOverrideMap.Set( Key, (ERelationship)_Relationship );
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x89f9f0 (235 B): the individual's own override first, then the first faction override the
// individual's faction derives from.
UBOOL FDisRelationshipOverrideInfo::GetIndividualOverride( const IDisRelationshipInterface& _rIndividual, BYTE& _rOutRelationship ) const
{
	const ERelationship* Individual = m_IndividualOverrideMap.Find( const_cast<IDisRelationshipInterface*>( &_rIndividual ) );
	if( Individual )
	{
		_rOutRelationship = *Individual;
		return TRUE;
	}
	const UDisTweaks_Faction* Faction = _rIndividual.GetFactionTweak();
	if( !Faction )
	{
		return FALSE;
	}
	for( TMap<UDisTweaks_Faction*,ERelationship>::TConstIterator It( m_FactionOverrideMap ); It; ++It )
	{
		if( Faction->IsFallbackDerivedFrom( It.Key() ) )
		{
			_rOutRelationship = It.Value();
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x894330
UBOOL FDisRelationshipOverrideInfo::HasAnyOverride() const
{
	return m_IndividualOverrideMap.Num() > 0 || m_FactionOverrideMap.Num() > 0;
}
