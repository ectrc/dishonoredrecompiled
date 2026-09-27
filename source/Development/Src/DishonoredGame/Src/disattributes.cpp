// DishonoredGame/src/disattributes.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x8d9380  public: static void __cdecl UDisAttributes::InitializePrivateStaticClassUDisAttributes(void)
//   0x8d93a0  public: float __thiscall FDisAttribute::GetBaseValue(enum EDifficulty)const
//   0x8d9420  public: void __thiscall FDisAttribute::PopulateWithLegacyValue(float)
//   0x8e6ea0  public: unsigned int __thiscall UDisAttributes::HasModifier(class FName const &, class FName const &)const
//   0x8e6f00  public: float __thiscall UDisAttributes::GetModifierValue(class FName const &, class FName const &)const
//   0x8ec440  public: void __thiscall UDisAttributes::RemoveModifier(class FName const &, class FName const &)
//   0x8ef0e0  private: void __thiscall UDisAttributes::RecacheAttributeValue(struct FDisModifiedAttribute const *)const
//   0x8ef200  public: float __thiscall UDisAttributes::GetAttributeValue(class FName const &)const
//   0x8ef260  public: void __thiscall UDisAttributes::TickAttributes(float)
//   0x8ef4d0  class FArchive & __cdecl operator<<(class FArchive &, struct FDisModifiedAttribute &)
//   0x8f76d0  private: struct FDisModifiedAttribute & __thiscall UDisAttributes::GetModifiedAttribute(class FName const &)
//   0x8f77b0  public: virtual void __thiscall UDisAttributes::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8f8020  public: void __thiscall UDisAttributes::RefreshAttributes(struct TMemStackArray<struct FDisAttribute const *> const &, struct TMemStackArray<struct FDisAttribute_RangeLimits const *> const &)
//   0x8f8530  public: static class UClass * __cdecl UDisAttributes::GetPrivateStaticClassUDisAttributes(wchar_t const *)
//   0x8f85c0  public: static class UClass * __cdecl UDisAttributes::StaticClassNoInline(void)
#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// ---- agent BF ports (PHASE8 BF): the attributes system ----
//
// The whole system is two reflected members of UDisAttributes: m_ModifiedAttributes, a TMap<FName,FDisModifiedAttribute>
// keyed by the attribute name, and m_AttributesWithModifiers, the list of the keys that currently carry a timed modifier
// so TickAttributes does not have to walk the whole map. The values a pawn reads come from the cooked tweak object
// (UDisTweaks_Pawn_Attributes and its 70-odd FDisAttribute / FDisAttribute_RangeLimits properties), copied in by
// UDisTweaks_Attributes::RefreshAttributesFromSource -> RefreshAttributes.

// DISHONORED(bringup): -disattrib. Off by default, free when off: the command line is read once.
static INT GDisAttribCensus = -1;

UBOOL UDisAttributes::IsCensusEnabled()
{
	if( GDisAttribCensus < 0 )
	{
		GDisAttribCensus = ( appStrfind( appCmdLine(), TEXT("-disattrib") ) != NULL ) ? 1 : 0;
	}
	return GDisAttribCensus != 0;
}

void UDisAttributes::DumpAttributes( const TCHAR* Tag ) const
{
	debugf( TEXT("DISHONORED(bringup): disattrib %s %s: %i attributes, %i with modifiers"),
		Tag, GetOuter() ? *GetOuter()->GetName() : TEXT("none"), m_ModifiedAttributes.Num(), m_AttributesWithModifiers.Num() );
	for( TMap<FName,FDisModifiedAttribute>::TConstIterator It( m_ModifiedAttributes ); It; ++It )
	{
		const FDisModifiedAttribute& Attribute = It.Value();
		debugf( TEXT("DISHONORED(bringup): disattrib   %s base %.3f range %i [%.3f .. %.3f] modifiers %i"),
			*It.Key().ToString(), Attribute.m_fAttributeBaseValue, Attribute.m_bEnforceRange ? 1 : 0,
			Attribute.m_fMinRange, Attribute.m_fMaxRange, Attribute.m_Modifiers.Num() );
	}
}

// DISHONORED(written): 2013 rva 0x889040 (2012 0x8f76d0): the map entry for a name, created zeroed when it is missing.
FDisModifiedAttribute& UDisAttributes::GetModifiedAttribute( const FName& AttributeName )
{
	FDisModifiedAttribute* Existing = m_ModifiedAttributes.Find( AttributeName );
	if( Existing )
	{
		return *Existing;
	}
	FDisModifiedAttribute NewModifiedAttribute;
	NewModifiedAttribute.m_fAttributeBaseValue = 0.f;
	NewModifiedAttribute.m_fMinRange = 0.f;
	NewModifiedAttribute.m_fMaxRange = 0.f;
	NewModifiedAttribute.m_bEnforceRange = FALSE;
	NewModifiedAttribute.m_bCachedValueIsDirty = FALSE;
	NewModifiedAttribute.m_fCachedModifiedValue = 0.f;
	return m_ModifiedAttributes.Set( AttributeName, NewModifiedAttribute );
}

// DISHONORED(written): 2013 rva 0x8863f0 (2012 0x8ef0e0): fold every modifier into the base value, then clamp. The three
// modifier types are eDisAttributeModifierType_AddVal (add m_fValue), _AddBasePercent (add m_fValue percent OF THE BASE
// VALUE, not of the running total) and _SetVal (override). Retail writes the cache through the const pointer.
void UDisAttributes::RecacheAttributeValue( const FDisModifiedAttribute* Attribute ) const
{
	FDisModifiedAttribute* const Cache = const_cast<FDisModifiedAttribute*>( Attribute );
	FLOAT Value = Cache->m_fAttributeBaseValue;
	for( TMap<FName,FDisAttributeModifier>::TConstIterator It( Cache->m_Modifiers ); It; ++It )
	{
		const FDisAttributeModifier& Modifier = It.Value();
		switch( Modifier.m_Type )
		{
		case eDisAttributeModifierType_AddVal:
			Value += Modifier.m_fValue;
			break;
		case eDisAttributeModifierType_AddBasePercent:
			Value += Modifier.m_fValue * 0.01f * Cache->m_fAttributeBaseValue;
			break;
		case eDisAttributeModifierType_SetVal:
			Value = Modifier.m_fValue;
			break;
		}
	}
	if( Cache->m_bEnforceRange )
	{
		Value = Clamp<FLOAT>( Value, Cache->m_fMinRange, Cache->m_fMaxRange );
	}
	Cache->m_fCachedModifiedValue = Value;
	Cache->m_bCachedValueIsDirty = FALSE;
}

// DISHONORED(written): 2013 rva 0x8864d0 (2012 0x8ef200).
FLOAT UDisAttributes::GetAttributeValue( const FName& AttributeName ) const
{
	const FDisModifiedAttribute* Attribute = m_ModifiedAttributes.Find( AttributeName );
	if( !Attribute )
	{
		return 0.f;
	}
	if( Attribute->m_bCachedValueIsDirty )
	{
		RecacheAttributeValue( Attribute );
	}
	return Attribute->m_fCachedModifiedValue;
}

// DISHONORED(written): 2013 rva 0x87e0e0 (2012 0x8e6ea0).
UBOOL UDisAttributes::HasModifier( const FName& AttributeName, const FName& ModifierName ) const
{
	const FDisModifiedAttribute* Attribute = m_ModifiedAttributes.Find( AttributeName );
	return Attribute && Attribute->m_Modifiers.Find( ModifierName ) != NULL;
}

// DISHONORED(written): 2013 rva 0x87e140 (2012 0x8e6f00).
FLOAT UDisAttributes::GetModifierValue( const FName& AttributeName, const FName& ModifierName ) const
{
	const FDisModifiedAttribute* Attribute = m_ModifiedAttributes.Find( AttributeName );
	if( Attribute )
	{
		const FDisAttributeModifier* Modifier = Attribute->m_Modifiers.Find( ModifierName );
		if( Modifier )
		{
			return Modifier->m_fValue;
		}
	}
	return 0.f;
}

// DISHONORED(written): 2013 rva 0x885280 (2012 0x8ec440).
// DISHONORED(retail): the emptiness test that prunes m_AttributesWithModifiers is on m_ModifiedAttributes, not on the
// attribute's own m_Modifiers (0x8852c5 reads [esi+4] - [esi+2ch] with esi = this+56 = m_ModifiedAttributes), and it sits
// outside the found-branch. That is a retail defect - the whole map is never empty once a pawn has attributes, so the
// name stays in the list and TickAttributes prunes it instead - and it is ported as retail wrote it.
void UDisAttributes::RemoveModifier( const FName& AttributeName, const FName& ModifierName )
{
	FDisModifiedAttribute* Attribute = m_ModifiedAttributes.Find( AttributeName );
	if( Attribute )
	{
		Attribute->m_Modifiers.Remove( ModifierName );
		Attribute->m_bCachedValueIsDirty = TRUE;
	}
	if( m_ModifiedAttributes.Num() == 0 )
	{
		m_AttributesWithModifiers.RemoveSingleItem( AttributeName );
	}
}

// DISHONORED(written): 2013 rva 0x889bc0 (2012 0x8f8020): the base value of every attribute the source tweaks declare,
// picked by the current difficulty (FDisAttribute::GetBaseValue 2012 rva 0x8d93a0, inlined here as retail inlines it:
// @12 Easy, @16 Normal, @20 Hard, @24 VeryHard, 0 for anything else; m_fBaseValue @8 is the pre-difficulty legacy value
// and is never read), then the range limits by name. ADishonoredGameInfo::GetDifficulty (2012 rva 0x62fbf0) is one byte
// read of m_Difficulty @1092 and is inlined here too.
void UDisAttributes::RefreshAttributes( const TArray<const FDisAttribute*>& Attributes,
                                        const TArray<const FDisAttribute_RangeLimits*>& RangeLimits )
{
	BYTE CurDifficulty = EDifficulty_Normal;
	if( GIsGame )
	{
		// DISHONORED(bringup): retail does not test the game info; DisGetGameInfo answers NULL before it exists.
		ADishonoredGameInfo* GameInfo = DisGetGameInfo();
		if( GameInfo )
		{
			CurDifficulty = GameInfo->m_Difficulty;
		}
	}
	for( INT Index = 0; Index < Attributes.Num(); Index++ )
	{
		const FDisAttribute* Attribute = Attributes(Index);
		FLOAT BaseValue = 0.f;
		switch( CurDifficulty )
		{
		case EDifficulty_Easy:     BaseValue = Attribute->m_fBaseValue1_Easy;     break;
		case EDifficulty_Normal:   BaseValue = Attribute->m_fBaseValue2_Normal;   break;
		case EDifficulty_Hard:     BaseValue = Attribute->m_fBaseValue3_Hard;     break;
		case EDifficulty_VeryHard: BaseValue = Attribute->m_fBaseValue4_VeryHard; break;
		}
		FDisModifiedAttribute& Modified = GetModifiedAttribute( Attribute->m_Name );
		Modified.m_bCachedValueIsDirty = TRUE;
		Modified.m_fAttributeBaseValue = BaseValue;
	}
	for( INT Index = 0; Index < RangeLimits.Num(); Index++ )
	{
		const FDisAttribute_RangeLimits* Limits = RangeLimits(Index);
		FDisModifiedAttribute& Modified = GetModifiedAttribute( Limits->m_Name );
		Modified.m_bEnforceRange = Limits->m_bEnforceRange;
		Modified.m_fMinRange = Limits->m_fMin;
		Modified.m_fMaxRange = Limits->m_fMax;
		Modified.m_bCachedValueIsDirty = TRUE;
	}
	m_ModifiedAttributes.Compact();
}

// DISHONORED(written): 2013 rva 0x886530 (2012 0x8ef260): walk m_AttributesWithModifiers backwards, age every modifier
// whose m_fLifetime is >= 0 (a negative lifetime is permanent), drop the ones that reach 0, and drop the attribute's name
// from the list once it holds no modifier at all.
// DISHONORED(bringup): retail dereferences the map lookup without testing it (0x886530 writes the dirty bit into a
// possibly-NULL pointer); the test is added here.
void UDisAttributes::TickAttributes( FLOAT DeltaTime )
{
	for( INT Index = m_AttributesWithModifiers.Num() - 1; Index >= 0; Index-- )
	{
		FDisModifiedAttribute* Attribute = m_ModifiedAttributes.Find( m_AttributesWithModifiers(Index) );
		if( !Attribute )
		{
			m_AttributesWithModifiers.Remove( Index );
			continue;
		}
		Attribute->m_bCachedValueIsDirty = TRUE;
		for( TMap<FName,FDisAttributeModifier>::TIterator It( Attribute->m_Modifiers ); It; ++It )
		{
			FDisAttributeModifier& Modifier = It.Value();
			if( Modifier.m_fLifetime >= 0.f )
			{
				Modifier.m_fLifetime -= DeltaTime;
				if( Modifier.m_fLifetime <= 0.f )
				{
					It.RemoveCurrent();
				}
			}
		}
		if( Attribute->m_Modifiers.Num() == 0 )
		{
			m_AttributesWithModifiers.Remove( Index );
		}
	}
}

// DISHONORED(port): UDisAttributes::GameLoad (2012 rva 0x8f77b0) and operator<<( FArchive&, FDisModifiedAttribute& )
// (2012 rva 0x8ef4d0) are the save-game half and wait on milestone 6; FDisAttribute::PopulateWithLegacyValue (2012 rva
// 0x8d9420) is only reached from UDisTweaks_Attributes::Serialize's ArLicenseeVer < 25 upgrade path, which no cooked 2013
// package takes.
