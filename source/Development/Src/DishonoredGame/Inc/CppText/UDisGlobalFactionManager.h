// UDisGlobalFactionManager cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x85f8c0 / 0x863d80. Body in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

	// ---- agent FV (PHASE18 FV): the override table and the cache clears; bodies in disglobalfactionmanager.cpp ----
	// s_pInstance is retail's 2012 0x10025bc, set by the constructor (0x8d8a30) for the one non-archetype instance; this tree
	// sets it in ADishonoredGameInfo::InitGlobalManagers, where retail's RebuildCaches is called on the same sub-object.
	static UDisGlobalFactionManager* s_pInstance;

	BYTE DetermineRelationship( const class IDisRelationshipInterface& _rFromIndividual, const class IDisRelationshipInterface& _rToIndividual );
	BYTE DetermineRelationship( const class UDisTweaks_Faction& _rFromFaction, const class IDisRelationshipInterface& _rToIndividual );
	void OverrideRelationToFaction( const class UDisTweaks_Faction& _rFromFaction, const class UDisTweaks_Faction& _rToFaction, BYTE _Relationship );
	void OverrideRelationToIndividual( const class UDisTweaks_Faction& _rFromFaction, const class IDisRelationshipInterface& _rToIndividual, BYTE _Relationship );
	void ClearRelationToFactionOverride( const class UDisTweaks_Faction& _rFromFaction, const class UDisTweaks_Faction& _rToFaction );
	void ClearRelationToIndividualOverride( const class UDisTweaks_Faction& _rFromFaction, const class IDisRelationshipInterface& _rToIndividual );
	void ClearIndividualCache( const class IDisRelationshipInterface& _rFromIndividual, const class IDisRelationshipInterface& _rToIndividual );
	void ClearIndividualCache( const class IDisRelationshipInterface& _rFromIndividual, const class UDisTweaks_Faction& _rToFaction );
	void ClearIndividualCache( const class IDisRelationshipInterface& _rFromIndividual );
	void ClearFactionCache( const class IDisRelationshipInterface& _rToIndividual, const class UDisTweaks_Faction& _rFromFaction );
	void ClearFactionCache( const class UDisTweaks_Faction& _rFromFaction, const class UDisTweaks_Faction& _rToFaction );
	void ClearFactionCache( const class UDisTweaks_Faction& _rFromFaction );
private:
	void ClearDerivedFactions( struct FDisDetermineRelationshipIndividualMap& _rIndividualMap, const class UDisTweaks_Faction& _rBaseFaction );
	void ClearDerivedFactions( struct FDisDetermineRelationshipFactionMap& _rFactionMap, const class UDisTweaks_Faction& _rBaseFaction );
	UBOOL CachedIsFallbackDerivedFrom( const class UDisTweaks_Faction& _rDerivedFaction, const class UDisTweaks_Faction& _rBaseFaction ) const;
public:
