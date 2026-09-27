// IDisAttributesInterface cpptext: included inside the generated interface body (DishonoredGameAttributesClasses.h).
// DISHONORED(written): agent BF. Every method is one line over GetAttributes(), which each implementing class supplies
// (ADishonoredPawn 2013 rva 0x7497b0 returns m_pAttributes; the interface subobject sits at ADishonoredPawn @1192 and
// the body reads +724 from it, i.e. ADishonoredPawn @1916 = m_pAttributes). Bodies in disattributesinterface.cpp:
// GetAttributeValue 2013 rva 0x886a50 (2012 0x8ef580), HasAttributeModifier 0x87e1c0, GetAttributeModifierValue
// 0x87e1e0, RemoveAttributeModifier 0x8852f0, AddAttributeModifier 0x889170.
public:
	virtual UObject* GetUObjectInterfaceDisAttributesInterface() { return NULL; }

	/** The one slot an implementing class supplies. Retail returns a reference, so it is never NULL, and each of the five
	    implementers (ADishonoredPawn, ADisExplosion, UDishonoredInventoryItem and the two DLC classes) returns its own
	    m_pAttributes.
	    DISHONORED(bringup): the default answers a shared, empty UDisAttributes and names the caller's class once, so the
	    four implementers that are still comment-only skeleton units keep compiling and read 0 for every attribute -
	    exactly what they did before this package - instead of dereferencing a NULL m_pAttributes. */
	virtual class UDisAttributes& GetAttributes();
	const class UDisAttributes& GetAttributes() const { return const_cast<IDisAttributesInterface*>( this )->GetAttributes(); }

	FLOAT GetAttributeValue( const FName& AttributeName ) const;
	UBOOL HasAttributeModifier( const FName& AttributeName, const FName& ModifierName ) const;
	FLOAT GetAttributeModifierValue( const FName& AttributeName, const FName& ModifierName ) const;
	void RemoveAttributeModifier( const FName& AttributeName, const FName& ModifierName );
	void AddAttributeModifier( const FName& AttributeName, const FName& ModifierName,
	                           BYTE ModifierType, FLOAT ModifierValue, FLOAT ModifierLifetime );
