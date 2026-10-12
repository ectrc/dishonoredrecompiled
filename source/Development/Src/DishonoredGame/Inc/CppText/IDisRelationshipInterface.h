// IDisRelationshipInterface cpptext: included inside the interface class body (dishonoredgameclasses.h).
// agent FV (PHASE18 FV). Retail's interface has six vtable slots, read off ??_7ADishonoredPawn@@6BIDisRelationshipInterface@@@
// (2012 0xd1a050) and ADishonoredNPCPawn's (0xd39bf8), in this order:
//   0 ~IDisRelationshipInterface                       1 GetUObjectInterfaceDisRelationshipInterface (0x71ce00)
//   2 GetRelationshipOverrideInfo (0x765290, `lea eax,[this+5Ch]`: the interface sits at 1208 in ADishonoredPawn and
//     1208 + 92 = 1300 = m_PersonalRelationships)       3 GetFactionTweak (0x964ef0, `mov eax,[this+54h]`: 1208 + 84 = 1292 =
//     m_pCurFactionTweak; the retail name of this slot is not in either database, GetFactionTweak is this tree's)
//   4 DetermineRelationship_Derived (0x764c30: returns the relationship handed in)
//   5 OnRelationshipChange (ADishonoredPawn: 0x57ef10, an empty body; ADishonoredNPCPawn overrides it, 0x7c04e0)
// Slots 1-3 are pure in retail; they answer NULL here so the implementors this package does not touch (ADisWatchTower,
// ADisRiverKrust, ADisMovableLimb, ADisDLC06NPCAttachment - each has a 7-byte retail body to port) keep building.
// Bodies in Src/disrelationshipinterface.cpp.
public:
	virtual class UObject* GetUObjectInterfaceDisRelationshipInterface() { return NULL; }
	virtual struct FDisRelationshipOverrideInfo* GetRelationshipOverrideInfo() { return NULL; }
	virtual class UDisTweaks_Faction* GetFactionTweak() const { return NULL; }
	virtual BYTE DetermineRelationship_Derived( const IDisRelationshipInterface& _rToIndividual, BYTE _Relationship ) const { return _Relationship; }
	virtual void OnRelationshipChange() {}

	// 2013 rvas 0x7043a0 / 0x704430 / 0x704480 / 0x7044d0 / 0x704500 / 0x70c4b0 / 0x70f6a0 (2012 0x7670c0 / 0x767150 /
	// 0x7671a0 / 0x7671f0 / 0x767220 / 0x76f0f0 / 0x7768b0)
	BYTE DetermineRelationship( const IDisRelationshipInterface& _rToIndividual ) const;
	void OverrideRelationToFaction( const class UDisTweaks_Faction& _rToFaction, BYTE _Relationship, UBOOL _bRefreshRelations );
	void OverrideRelationToIndividual( const IDisRelationshipInterface& _rToIndividual, BYTE _Relationship, UBOOL _bRefreshRelations );
	void ClearRelationOverrides();
	void ClearRelationToFactionOverride( const class UDisTweaks_Faction& _rToFaction );
	void ClearRelationToIndividualOverride( const IDisRelationshipInterface& _rToIndividual );
	static IDisRelationshipInterface* FromUObjectOrController( class UObject* _pObject );

protected:
	// 2013 rvas 0x70f700 / 0x704540 / 0x70fa60 (2012 0x776910 / 0x767260 / 0x776c70)
	void OnSetDispositionHelper( class UDisSeqAct_SetDisposition* _pAction );
	void OnActorTerminated_RelationshipInterface();
	void OnOtherActorTerminated_RelationshipInterface( const class AActor& _rOtherActor );

private:
	// 2013 rva 0x7043d0 (2012 0x7670f0)
	BYTE DetermineRelationship_Internal( const IDisRelationshipInterface& _rToIndividual ) const;
