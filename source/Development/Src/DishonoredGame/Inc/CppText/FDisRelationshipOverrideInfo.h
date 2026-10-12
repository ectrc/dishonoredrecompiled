// FDisRelationshipOverrideInfo cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x8598b0 / 0x8599f0, disglobalenums.cpp.
	// Not virtual: UDisGlobalFactionManager calls these directly on each map value.
	void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent FV (PHASE18 FV). 2013 rvas 0x85c880 / 0x85e590 / 0x85e7e0 (2012 0x89c420 / 0x89fae0 / 0x89fc60);
	// ClearFactionOverride 0x85e610 (0x89fb60) and OverrideFactionRelationship 0x860c40 (0x8a0670) are two of the four functions the
	// 2013 database leaves as sub_C5E610 / sub_C60C40; GetIndividualOverride 2012 0x89f9f0, HasAnyOverride 0x894330. Bodies in
	// disglobalenums.cpp. A faction override also drops every individual override whose faction derives from it.
	UBOOL ClearIndividualOverride( const class IDisRelationshipInterface& _rIndividual );
	UBOOL ClearFactionOverride( const class UDisTweaks_Faction& _rFaction );
	void ClearAllOverrides();
	UBOOL OverrideIndividualRelationship( const class IDisRelationshipInterface& _rIndividual, BYTE _Relationship );
	UBOOL OverrideFactionRelationship( const class UDisTweaks_Faction& _rFaction, BYTE _Relationship );
	UBOOL GetIndividualOverride( const class IDisRelationshipInterface& _rIndividual, BYTE& _rOutRelationship ) const;
	UBOOL HasAnyOverride() const;
