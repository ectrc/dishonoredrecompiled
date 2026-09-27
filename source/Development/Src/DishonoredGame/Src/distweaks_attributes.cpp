// DishonoredGame/src/distweaks_attributes.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x8edfc0  public: static class UClass * __cdecl UDisTweaks_Attributes::GetPrivateStaticClassUDisTweaks_Attributes(wchar_t const *)
//   0x8f0540  public: static void __cdecl UDisTweaks_Attributes::InitializePrivateStaticClassUDisTweaks_Attributes(void)
//   0x8f0d00  public: static class UClass * __cdecl UDisTweaks_Attributes::StaticClassNoInline(void)
//   0x8f0d30  public: virtual void __thiscall UDisTweaks_Attributes::Serialize(class FArchive &)
//   0x8f8130  public: void __thiscall UDisTweaks_Attributes::RefreshAttributesFromSource(class UDisAttributes *, class UObject * const)const
//   0x8f85f0  public: class UDisAttributes * __thiscall UDisTweaks_Attributes::ConstructAttributes(class UObject *)const

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): UDisTweaks_Attributes ----

// DISHONORED(written): 2012 rva 0x8f8130 (unmatched in the 2013 symbol set; reached from
// ADishonoredPawn::PreBeginPlay_Attributes 2013 rva 0x762190 and OnDifficultyChange). The source of a pawn's attributes is
// this tweak object's own reflected properties: a TFieldIterator<UStructProperty> over its Class collects every property
// whose struct is DisAttribute into one list and every DisAttribute_RangeLimits into another, then RefreshAttributes folds
// them into the map. Nothing native is read - see the layout note in Inc/CppText/UDisTweaks_Attributes.h.
// The two arrays are retail's TMemStackArray<const T*>, which is unported; TArray holds the same pointers in the same
// order. A static array property contributes only its first element, exactly as retail's single Offset read does.
void UDisTweaks_Attributes::RefreshAttributesFromSource( UDisAttributes* Attributes, UObject* const Outer ) const
{
	if( !Attributes )
	{
		return;
	}
	static const FName NAME_DisAttribute( TEXT("DisAttribute") );
	static const FName NAME_DisAttribute_RangeLimits( TEXT("DisAttribute_RangeLimits") );

	TArray<const FDisAttribute*> SourceAttributes;
	TArray<const FDisAttribute_RangeLimits*> SourceRangeLimits;
	for( TFieldIterator<UStructProperty> It( GetClass() ); It; ++It )
	{
		UStructProperty* Property = *It;
		const FName StructName = Property->Struct ? Property->Struct->GetFName() : NAME_None;
		const BYTE* Value = (const BYTE*)this + Property->Offset;
		if( StructName == NAME_DisAttribute )
		{
			SourceAttributes.AddItem( (const FDisAttribute*)Value );
		}
		if( StructName == NAME_DisAttribute_RangeLimits )
		{
			SourceRangeLimits.AddItem( (const FDisAttribute_RangeLimits*)Value );
		}
	}
	Attributes->RefreshAttributes( SourceAttributes, SourceRangeLimits );
}

// DISHONORED(written): 2013 rva 0x88aca0 (2012 0x8f85f0): a fresh UDisAttributes outered to Outer (the transient package
// when Outer is INDEX_NONE, which no caller passes), filled from this tweak object.
UDisAttributes* UDisTweaks_Attributes::ConstructAttributes( UObject* Outer ) const
{
	UDisAttributes* Attributes = (UDisAttributes*)UObject::StaticConstructObject(
		UDisAttributes::StaticClass(), Outer ? Outer : UObject::GetTransientPackage() );
	RefreshAttributesFromSource( Attributes, Outer );
	return Attributes;
}

// DISHONORED(port): UDisTweaks_Attributes::Serialize (2012 rva 0x8f0d30) only does work when
// ArIsLoading && ArLicenseeVer < 25, where it copies each FDisAttribute's pre-difficulty m_fBaseValue into all four
// per-difficulty values and marks the package dirty. No cooked 2013 package is that old, so the override is left out.
