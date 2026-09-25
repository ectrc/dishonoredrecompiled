// DishonoredGame/src/disconv_playerchoice.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x8d9860  protected: virtual void __thiscall UDisConv_PlayerChoice::GetClassVars_Derived(struct FDisConvClassVars &)const
//   0x8daa00  public: struct FDisPlayerChoice_Optional const * __thiscall UDisConv_PlayerChoice::GetOptionalChoiceForIndex(int, int &)const
//   0x8daac0  public: virtual unsigned int __thiscall UDisConv_PlayerChoice::IsValidChoice(int)const
//   0x8dd8e0  protected: virtual int __thiscall UDisConv_PlayerChoice::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x8dd950  public: virtual int __thiscall UDisConv_PlayerChoice::TickNode(float, struct FDisDialogRunningInstance *)const
//   0x8dd9d0  protected: virtual unsigned int __thiscall UDisConv_PlayerChoice::ImportText_Derived(class FString const &, class FName const &, class FString const &, unsigned int)
//   0x8ddcd0  public: wchar_t const * __thiscall FDisPlayerChoice_Optional::GetChoiceText(void)const
//   0x8dde30  public: wchar_t const * __thiscall FDisPlayerChoice_Static::GetChoiceText(void)const
//   0x8ddf90  public: virtual struct FDisPlayerChoice_Optional & __thiscall UDisConv_PlayerChoice::GetOptionalChoice(int)
//   0x8ddfe0  public: virtual struct FDisPlayerChoice_Optional const & __thiscall UDisConv_PlayerChoice::GetOptionalChoice(int)const
//   0x8de030  public: virtual struct FDisPlayerChoice_Static const & __thiscall UDisConv_PlayerChoice::GetChoice(int)const
//   0x8e8070  protected: void __thiscall UDisConv_PlayerChoice::MatchChoicesToOutputLinks(unsigned int)
//   0x8ea740  public: virtual void __thiscall UDisConv_PlayerChoice::InitNode(class UDisConversation *, unsigned int, unsigned int)
//   0x8ef660  public: static void __cdecl UDisConv_PlayerChoice::InitializePrivateStaticClassUDisConv_PlayerChoice(void)
//   0x8f1590  public: virtual void __thiscall UDisConv_PlayerChoice::Serialize(class FArchive &)
//   0x8f4c00  public: static class UClass * __cdecl UDisConv_PlayerChoice::GetPrivateStaticClassUDisConv_PlayerChoice(wchar_t const *)
//   0x8f6740  public: static class UClass * __cdecl UDisConv_PlayerChoice::StaticClassNoInline(void)

#include "DishonoredGame.h"

static void SerializePerLanguageChoices( FArchive& Ar, UScriptStruct* LocalizedStruct, TArray<FDisPlayerChoiceLocalized>& PerLanguageData, const FName& CurrentLanguage )
{
	INT NumLanguages = PerLanguageData.Num();
	Ar << NumLanguages;
	if( !Ar.IsLoading() )
	{
		for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
		{
			LocalizedStruct->SerializeBin( Ar, (BYTE*)&PerLanguageData(LanguageIndex), sizeof(FDisPlayerChoiceLocalized) );
		}
		return;
	}
	PerLanguageData.Reset( GIsEditor ? NumLanguages : 1 );
	FDisPlayerChoiceLocalized Localized(EC_EventParm);
	for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
	{
		LocalizedStruct->SerializeBin( Ar, (BYTE*)&Localized, sizeof(FDisPlayerChoiceLocalized) );
		if( GIsEditor || Localized.m_Language == CurrentLanguage )
		{
			PerLanguageData.AddItem( Localized );
		}
	}
}

// DISHONORED(written): 2013 rva 0x8a1850 (2012 0x8f1590); non-template choices store the per-language texts of every static
// and optional choice after the tagged properties (version 1); the pre-version m_Choices strings are moved to m_Choices_Static.
void UDisConv_PlayerChoice::Serialize( FArchive& Ar )
{
	if( Ar.IsSaving() && !IsTemplate() )
	{
		m_PlayerChoiceVersionNum = 1;
	}
	Super::Serialize( Ar );
	if( IsTemplate() )
	{
		return;
	}
	UScriptStruct* LocalizedStruct = FindField<UScriptStruct>( GetClass(), TEXT("DisPlayerChoiceLocalized") );
	if( Ar.IsLoading() )
	{
		if( m_Choices.Num() )
		{
			m_Choices_Static.Reset( m_Choices.Num() );
			for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices.Num(); ChoiceIndex++ )
			{
				FDisPlayerChoice_Static* Static = new(m_Choices_Static) FDisPlayerChoice_Static(EC_EventParm);
				Static->m_ChoiceText = m_Choices(ChoiceIndex);
			}
			m_Choices.Reset();
			MarkPackageDirty();
		}
		if( m_PlayerChoiceVersionNum < 1 )
		{
			return;
		}
	}
	const FName CurrentLanguage( UObject::GetLanguage() );
	for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices_Static.Num(); ChoiceIndex++ )
	{
		SerializePerLanguageChoices( Ar, LocalizedStruct, m_Choices_Static(ChoiceIndex).m_PerLanguageData, CurrentLanguage );
	}
	for( INT ChoiceIndex = 0; ChoiceIndex < m_Choices_Optional.Num(); ChoiceIndex++ )
	{
		SerializePerLanguageChoices( Ar, LocalizedStruct, m_Choices_Optional(ChoiceIndex).m_PerLanguageData, CurrentLanguage );
	}
}
