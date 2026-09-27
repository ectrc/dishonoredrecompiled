// DishonoredGame/src/disattributesinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x8d9440  public: static void __cdecl UDisAttributesInterface::InitializePrivateStaticClassUDisAttributesInterface(void)
//   0x8dc7d0  public: static class UClass * __cdecl UDisAttributesInterface::GetPrivateStaticClassUDisAttributesInterface(wchar_t const *)
//   0x8e0310  public: static class UClass * __cdecl UDisAttributesInterface::StaticClassNoInline(void)
//   0x8e6f80  public: unsigned int __thiscall IDisAttributesInterface::HasAttributeModifier(class FName const &, class FName const &)const
//   0x8e6fa0  public: float __thiscall IDisAttributesInterface::GetAttributeModifierValue(class FName const &, class FName const &)const
//   0x8ec4b0  public: void __thiscall IDisAttributesInterface::RemoveAttributeModifier(class FName const &, class FName const &)
//   0x8ef580  public: float __thiscall IDisAttributesInterface::GetAttributeValue(class FName const &)const
//   0x8f77d0  public: void __thiscall IDisAttributesInterface::AddAttributeModifier(class FName const &, class FName const &, enum eDisAttributeModifierType, float, float)

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): IDisAttributesInterface ----
//
// Every method is one indirect call to the implementing class's GetAttributes() followed by the matching UDisAttributes
// method, so the interface itself holds no state. Five classes carry the vtable in retail (retail_sdk_layout.json:
// ADishonoredPawn @1192, ADisExplosion @596, UDishonoredInventoryItem @60, UDisDLC06ItemContext_NPCWitchScream @176,
// UDisDLC07AISubStateTentacleAttack @204) and only ADishonoredPawn's override has its own symbol; the other four are
// identical one-instruction bodies and were folded by ICF.

// DISHONORED(bringup): the fallback every implementer that is not ported yet answers with. It is a real, empty
// UDisAttributes, so a read gives 0 and nothing dereferences NULL; the class that reached it is named once so the shim can
// never be mistaken for a working attribute set (the defect pattern agent AV and agent AU both hit).
static UDisAttributes* GDisAttributesFallback = NULL;

UDisAttributes& IDisAttributesInterface::GetAttributes()
{
	if( !GDisAttributesFallback )
	{
		GDisAttributesFallback = (UDisAttributes*)UObject::StaticConstructObject(
			UDisAttributes::StaticClass(), UObject::GetTransientPackage(), FName( TEXT("DisAttributesFallback") ) );
		GDisAttributesFallback->AddToRoot();
	}
	static TArray<UClass*> Named;
	UObject* Object = GetUObjectInterfaceDisAttributesInterface();
	UClass* Class = Object ? Object->GetClass() : NULL;
	if( Named.FindItemIndex( Class ) == INDEX_NONE )
	{
		Named.AddItem( Class );
		debugf( TEXT("DISHONORED(bringup): %s implements DisAttributesInterface but has no GetAttributes override: every attribute reads 0"),
			Class ? *Class->GetName() : TEXT("an unnamed class") );
	}
	return *GDisAttributesFallback;
}

// DISHONORED(written): 2013 rva 0x886a50 (2012 0x8ef580).
FLOAT IDisAttributesInterface::GetAttributeValue( const FName& AttributeName ) const
{
	return GetAttributes().GetAttributeValue( AttributeName );
}

// DISHONORED(written): 2013 rva 0x87e1c0 (2012 0x8e6f80).
UBOOL IDisAttributesInterface::HasAttributeModifier( const FName& AttributeName, const FName& ModifierName ) const
{
	return GetAttributes().HasModifier( AttributeName, ModifierName );
}

// DISHONORED(written): 2013 rva 0x87e1e0 (2012 0x8e6fa0).
FLOAT IDisAttributesInterface::GetAttributeModifierValue( const FName& AttributeName, const FName& ModifierName ) const
{
	return GetAttributes().GetModifierValue( AttributeName, ModifierName );
}

// DISHONORED(written): 2013 rva 0x8852f0 (2012 0x8ec4b0).
void IDisAttributesInterface::RemoveAttributeModifier( const FName& AttributeName, const FName& ModifierName )
{
	GetAttributes().RemoveModifier( AttributeName, ModifierName );
}

// DISHONORED(written): 2013 rva 0x889170 (2012 0x8f77d0): the modifier is added to the attribute's own map, the cached
// value is dirtied and the attribute's name joins m_AttributesWithModifiers so TickAttributes ages it.
void IDisAttributesInterface::AddAttributeModifier( const FName& AttributeName, const FName& ModifierName,
                                                    BYTE ModifierType, FLOAT ModifierValue, FLOAT ModifierLifetime )
{
	UDisAttributes& Attributes = GetAttributes();
	FDisAttributeModifier Modifier;
	Modifier.m_Type = ModifierType;
	Modifier.m_fValue = ModifierValue;
	Modifier.m_fLifetime = ModifierLifetime;
	FDisModifiedAttribute& Attribute = Attributes.GetModifiedAttribute( AttributeName );
	Attribute.m_Modifiers.Set( ModifierName, Modifier );
	Attribute.m_bCachedValueIsDirty = TRUE;
	Attributes.m_AttributesWithModifiers.AddUniqueItem( AttributeName );
}
