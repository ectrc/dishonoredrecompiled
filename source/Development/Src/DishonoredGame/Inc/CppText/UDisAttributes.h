// UDisAttributes cpptext: included inside the generated class body (DishonoredGameAttributesClasses.h).
// DISHONORED(written): agent BF. The whole read/write core of the attributes system, over the two reflected members
// m_ModifiedAttributes (TMap<FName,FDisModifiedAttribute>) and m_AttributesWithModifiers. Bodies in disattributes.cpp:
// GetAttributeValue 2013 rva 0x8864d0 (2012 0x8ef200), GetModifiedAttribute 0x889040 (0x8f76d0), RecacheAttributeValue
// 0x8863f0 (0x8ef0e0), RefreshAttributes 0x889bc0 (0x8f8020), TickAttributes 0x886530 (0x8ef260), HasModifier 0x87e0e0,
// GetModifierValue 0x87e140, RemoveModifier 0x885280.
public:
	/** 2013 rva 0x8864d0. 0 for an attribute the tweaks never declared; recaches through the const pointer first. */
	FLOAT GetAttributeValue( const FName& AttributeName ) const;

	UBOOL HasModifier( const FName& AttributeName, const FName& ModifierName ) const;
	FLOAT GetModifierValue( const FName& AttributeName, const FName& ModifierName ) const;
	void RemoveModifier( const FName& AttributeName, const FName& ModifierName );

	/** 2013 rva 0x889bc0. One entry per FDisAttribute of the source tweaks, its base value picked by the difficulty,
	    then the range limits folded in by name. Retail takes two TMemStackArray<const T*>; TMemStackArray is unported,
	    so these are TArray of the same element type - same contents, same order, heap instead of the mem stack. */
	void RefreshAttributes( const TArray<const struct FDisAttribute*>& Attributes,
	                        const TArray<const struct FDisAttribute_RangeLimits*>& RangeLimits );

	/** 2013 rva 0x886530. Ages every modifier of every attribute that has one and drops the expired ones. */
	void TickAttributes( FLOAT DeltaTime );

	/** 2013 rva 0x889040. Retail declares this private and lets IDisAttributesInterface::AddAttributeModifier reach it. */
	struct FDisModifiedAttribute& GetModifiedAttribute( const FName& AttributeName );

	/** DISHONORED(bringup): -disattrib. Off by default and free when off. */
	static UBOOL IsCensusEnabled();
	void DumpAttributes( const TCHAR* Tag ) const;

private:
	/** 2013 rva 0x8863f0. Retail's own signature: the cache is written through the const pointer. */
	void RecacheAttributeValue( const struct FDisModifiedAttribute* Attribute ) const;

	// DISHONORED(port): agent ED (PHASE11 ED), 2012 rva 0x8f77b0 - retail folds GameSave and GameLoad
	// onto one body here, because both are just the modified-attribute map. Body in dissavegame.cpp.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
