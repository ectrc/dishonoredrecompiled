// DishonoredGame/src/disconv_blurb.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (22):
//   0x8dd3f0  public: void __thiscall UDisConv_Blurb_InGameData::StopBlurbSpeakerData(class IDisConvSpeakerInterface *)
//   0x8dd430  public: virtual void __thiscall UDisConv_Blurb::PostLoad(void)
//   0x8dd4f0  protected: virtual wchar_t const * __thiscall UDisConv_Blurb::ExtractText_Derived(void)const
//   0x8e0cc0  public: struct FDisBlurbPerSpeakerInstance & __thiscall UDisConv_Blurb_InGameData::GetBlurbSpeakerData(class IDisConvSpeakerInterface *)
//   0x8e0d60  public: virtual void __thiscall UDisConv_Blurb::InitNode(class UDisConversation *, unsigned int, unsigned int)
//   0x8e0e30  protected: class IDisConvSpeakerInterface * __thiscall UDisConv_Blurb::GetBlurbLookTarget(class UDisConversation_InGameData *, struct FDisDialogRunningInstance *, enum eDisConvoLookType &, unsigned int &)const
//   0x8e11b0  public: virtual void __thiscall UDisConv_Blurb::InterruptTickingNode(struct FDisDialogRunningInstance *, enum DisConvoEndReasonEnum)const
//   0x8e3250  public: static void __cdecl UDisConv_Blurb_InGameData::InitializePrivateStaticClassUDisConv_Blurb_InGameData(void)
//   0x8e3270  protected: int __thiscall UDisConv_Blurb::PlayBlurb_Shared(unsigned int &, struct FDisDialogRunningInstance *, unsigned int, float)const
//   0x8e4bc0  protected: virtual int __thiscall UDisConv_Blurb::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x8e4be0  protected: unsigned int __thiscall UDisConv_Blurb::StartBlurb(struct FDisDialogRunningInstance *, struct FDisBlurbPerSpeakerInstance &, class IDisConvSpeakerInterface *, unsigned int)const
//   0x8e5430  public: virtual unsigned int __thiscall UDisConv_Blurb::SearchNode(class FString const &)const
//   0x8e7b50  protected: int __thiscall UDisConv_Blurb::TickNode_Shared(float, struct FDisDialogRunningInstance *, unsigned int)const
//   0x8e7d10  public: virtual void __thiscall UDisConv_Blurb::Serialize(class FArchive &)
//   0x8ea660  public: static class UClass * __cdecl UDisConv_Blurb_InGameData::GetPrivateStaticClassUDisConv_Blurb_InGameData(wchar_t const *)
//   0x8ea6f0  public: virtual int __thiscall UDisConv_Blurb::TickNode(float, struct FDisDialogRunningInstance *)const
//   0x8ec8d0  public: static class UClass * __cdecl UDisConv_Blurb_InGameData::StaticClassNoInline(void)
//   0x8ee3f0  public: static class UClass * __cdecl UDisConv_Blurb::GetPrivateStaticClassUDisConv_Blurb(wchar_t const *)
//   0x8ee480  protected: virtual void __thiscall UDisConv_Blurb::GetClassVars_Derived(struct FDisConvClassVars &)const
//   0x8f0750  public: static void __cdecl UDisConv_Blurb::InitializePrivateStaticClassUDisConv_Blurb(void)
//   0x8f1490  public: static class UClass * __cdecl UDisConv_Blurb::StaticClassNoInline(void)
//   0x8f4790  protected: virtual unsigned int __thiscall UDisConv_Blurb::ImportText_Derived(class FString const &, class FName const &, class FString const &, unsigned int)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x8988a0 (2012 0x8e7d10); non-template blurbs store one FDisBlurbLangInfo per language
// after the tagged properties, and outside the editor only the current language is kept.
void UDisConv_Blurb::Serialize( FArchive& Ar )
{
	if( Ar.IsSaving() && !IsTemplate() )
	{
		m_BlurbVersionNum = 2;
	}
	Super::Serialize( Ar );
	if( IsTemplate() )
	{
		return;
	}
	const FName CurrentLanguage( UObject::GetLanguage() );
	UScriptStruct* LangInfoStruct = FindField<UScriptStruct>( GetClass(), TEXT("DisBlurbLangInfo") );
	if( Ar.IsLoading() )
	{
		if( m_BlurbVersionNum < 2 )
		{
			m_PerLanguageData.Empty();
			return;
		}
		INT NumLanguages = 0;
		Ar << NumLanguages;
		m_PerLanguageData.Empty( GIsEditor ? NumLanguages : 1 );
		FDisBlurbLangInfo LangInfo(EC_EventParm);
		for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
		{
			LangInfoStruct->SerializeBin( Ar, (BYTE*)&LangInfo, sizeof(FDisBlurbLangInfo) );
			if( GIsEditor || LangInfo.m_Language == CurrentLanguage )
			{
				m_PerLanguageData.AddItem( LangInfo );
			}
		}
		return;
	}
	INT NumLanguages = m_PerLanguageData.Num();
	Ar << NumLanguages;
	for( INT LanguageIndex = 0; LanguageIndex < NumLanguages; LanguageIndex++ )
	{
		LangInfoStruct->SerializeBin( Ar, (BYTE*)&m_PerLanguageData(LanguageIndex), sizeof(FDisBlurbLangInfo) );
	}
}
