// DishonoredGame/src/disgfxmovieplayermenubase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (41):
//   0x7f4b50  public: static void __cdecl UDisTweaks_GFxMoviePlayerMenuBase::InitializePrivateStaticClassUDisTweaks_GFxMoviePlayerMenuBase(void)
//   0x7f4b70  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnKeyListening(int)
//   0x7f7120  protected: void __thiscall UDisGFxMoviePlayerMenuBase::FindSaveImagePath(struct FDisSaveGame const *, class FString &, unsigned int)const
//   0x7f71c0  private: void __thiscall UDisGFxMoviePlayerMenuBase::CloseOptions(void)
//   0x7f7290  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnApplyVideoSettings(void)
//   0x7f72c0  public: void __thiscall UDisGFxMoviePlayerGamma::CloseGammaMenu(void)
//   0x7f73d0  public: virtual void __thiscall UDisGFxMoviePlayerGamma::OnGammaImageClosed(void)
//   0x800ec0  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnLoadGameConfirm(int)
//   0x800f40  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnDeleteSaveConfirm(unsigned int, int)
//   0x800fc0  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::Req_GamepadMappingScreen(int)
//   0x801340  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::CloseGammaImage(void)
//   0x806880  protected: void __thiscall UDisGFxMoviePlayerMenuBase::FormatSaveDate(struct FDisSaveGame const *, class FString &)const
//   0x806b10  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnSettingChange(int, float)
//   0x806de0  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnLeaveOptions(void)
//   0x80f920  private: void __thiscall UDisGFxMoviePlayerMenuBase::CreateGFxSetting(struct FDisSetting const &, class GFxValue *)
//   0x8103b0  private: void __thiscall UDisGFxMoviePlayerMenuBase::TryBindKey(class FName)
//   0x810590  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::Req_VideoSettingsScreen(void)
//   0x813790  protected: virtual unsigned int __thiscall UDisGFxMoviePlayerMenuBase::FilterButtonInput(int, class FName, unsigned char, unsigned int &)
//   0x8137f0  protected: void __thiscall UDisGFxMoviePlayerMenuBase::FindSaveName(struct FDisSaveGame const *, class FString &)const
//   0x813930  private: void __thiscall UDisGFxMoviePlayerMenuBase::CreateGFxLoadGameList(class GFxValue *)
//   0x813db0  private: void __thiscall UDisGFxMoviePlayerMenuBase::CreateGFxSubCategory(struct FDisSettingsSubCategory const &, class GFxValue *)
//   0x815830  protected: void __thiscall UDisGFxMoviePlayerMenuBase::FillLoadGameMenu(void)
//   0x815940  private: void __thiscall UDisGFxMoviePlayerMenuBase::RefreshLoadGameMenu(void)
//   0x815a50  private: void __thiscall UDisGFxMoviePlayerMenuBase::CreateGFxCategory(struct FDisSettingsCategory const &, class GFxValue *)
//   0x817160  protected: virtual void __thiscall UDisGFxMoviePlayerMenuBase::PreAdvance(float)
//   0x817250  private: void __thiscall UDisGFxMoviePlayerMenuBase::ShowSettingsCategoryList(class TArray<struct FDisSettingsCategory, class FDefaultAllocator> const &)
//   0x817400  private: void __thiscall UDisGFxMoviePlayerMenuBase::RefreshSettingsCategory(struct FDisSettingsCategory const &)
//   0x817510  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OnResetOptions(int, int)
//   0x818270  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerMenuBase::GetPrivateStaticClassUDisTweaks_GFxMoviePlayerMenuBase(wchar_t const *)
//   0x81adc0  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerMenuBase::StaticClassNoInline(void)
//   0x81ba20  private: void __thiscall UDisGFxMoviePlayerMenuBase::FillSettingsCategoryList(unsigned int, class TArray<struct FDisSettingsCategory, class FDefaultAllocator> &)
//   0x81da70  protected: void __thiscall UDisGFxMoviePlayerMenuBase::FillOptionsMenu(unsigned int)
//   0x81daf0  public: void __thiscall UDisGFxMoviePlayerGamma::OpenGammaMenu(void)
//   0x81dcf0  private: virtual void __thiscall UDisGFxMoviePlayerGamma::OnCompleteMoviePackageLoading(void)
//   0x820420  public: static class UClass * __cdecl UDisGFxMoviePlayerMenuBase::GetPrivateStaticClassUDisGFxMoviePlayerMenuBase(wchar_t const *)
//   0x8204b0  public: static class UClass * __cdecl UDisGFxMoviePlayerGamma::GetPrivateStaticClassUDisGFxMoviePlayerGamma(wchar_t const *)
//   0x821050  public: static void __cdecl UDisGFxMoviePlayerMenuBase::InitializePrivateStaticClassUDisGFxMoviePlayerMenuBase(void)
//   0x821070  public: static void __cdecl UDisGFxMoviePlayerGamma::InitializePrivateStaticClassUDisGFxMoviePlayerGamma(void)
//   0x822790  public: static class UClass * __cdecl UDisGFxMoviePlayerMenuBase::StaticClassNoInline(void)
//   0x8227c0  public: static class UClass * __cdecl UDisGFxMoviePlayerGamma::StaticClassNoInline(void)
//   0x822980  public: virtual void __thiscall UDisGFxMoviePlayerMenuBase::OpenGammaImage(void)

/*-----------------------------------------------------------------------------
	Agent BE (PHASE8.md package BE): UDisGFxMoviePlayerMenuBase's 7 natives and the
	C++ data models behind the load-game list and the settings tree.

	Agent AW's positive finding (gfx_decision.md 2.4 and "why option B is cheap") was
	that these models are ours and not ActionScript, and that "a replacement layer
	replaces only the CreateGFx* leaves". The leaves are what is ported here, and they
	are complete, because they only need the GFx object interface this wave's GFxUI
	layer now provides: the exact AS2 member names the cooked menu asset reads are in
	the code below and they are the contract with the asset.

	What the models above the leaves still need, each a named declaration in a class
	this package does not own:
	  * the load-game list: FDisSaveGame and UDishonoredEngine::GetSaveGame (2012
	    0x6425f0) / HasSaveGame (0x642540) / DeleteSaveGame (0x642660) /
	    FindMapConfigFromFriendlyName (0x642900). None is declared here, and
	    UDishonoredEngine has no reflected save-slot array either, so the slot
	    enumeration has no input. This is the save system, PLAN.md milestone 6.
	  * the settings tree and key binding: UArkProfileSettings' setting and binding
	    accessors (FindBindableKey, the per-setting value/min/max/increment reads).
	    UArkProfileSettings is a shim class in DishonoredGameEngineShims.h with one
	    native, so FillSettingsCategoryList's 1.7 KB of hard-coded tree cannot read a
	    single value yet.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "gfxui_gfx3.h"
#include "dishonoredutilities_saveload.h"
#include "dishonoredutilities.h"

/** the movie view, or NULL when no movie is open - every leaf below starts here */
static GFxMovieView* DisMenuView( UDisGFxMoviePlayerMenuBase* Menu )
{
	FGFxMovie* Movie = Menu->GetMovie();
	return Movie ? Movie->pView.GetPtr() : NULL;
}

/** set one string member on an AS2 object */
static void DisSetGFxString( GFxValue& Object, const char* Member, const FString& Value )
{
	GFxValue String;
	String.SetStringW( *Value );
	Object.SetMember( Member, String );
}

/** set one number member on an AS2 object */
static void DisSetGFxNumber( GFxValue& Object, const char* Member, DOUBLE Value )
{
	GFxValue Number;
	Number.SetNumber( Value );
	Object.SetMember( Member, Number );
}

/** set one boolean member on an AS2 object */
static void DisSetGFxBool( GFxValue& Object, const char* Member, UBOOL Value )
{
	GFxValue Boolean;
	Boolean.SetBoolean( Value ? true : false );
	Object.SetMember( Member, Boolean );
}

// DISHONORED(port): agent EO. 2013 rva 0x5359b0: PSI_DLC06_MasterAssassinUnlocked (id 151) - the profile bit that
// decides whether the fourth DLC06 difficulty is offered at all.
static UBOOL DisIsMasterAssassinUnlocked( UOnlinePlayerStorage* Settings )
{
	const INT SettingIndex = Settings ? Settings->FindProfileSettingIndex( 151 ) : INDEX_NONE;
	if( !Settings || !Settings->ProfileSettings.IsValidIndex( SettingIndex ) )
	{
		return FALSE;
	}
	INT Unlocked = 0;
	Settings->ProfileSettings(SettingIndex).ProfileSetting.Data.GetData( Unlocked );
	return Unlocked != 0;
}

// DISHONORED(port): agent EO. The profile the options screen reads and writes. Retail reaches it through
// ADishonoredPlayerController::s_pInstance and the virtual at vtable +1424, which is GetProfileSettings();
// every one of CreateGFxSetting (2013 rva 0x7db5f0), OnSettingChange (0x7cb870), FillOptionsMenu (0x7e65f0)
// and OnLeaveOptions (0x7bcad0) opens with exactly that pair of calls.
static UArkProfileSettings* DisMenuProfileSettings()
{
	return ADishonoredPlayerController::s_pInstance ? ADishonoredPlayerController::s_pInstance->GetProfileSettings() : NULL;
}

// DISHONORED(port): agent EO. The listener array retail keeps at DisGFxMoviePlayerMenuBase +468 and hands to
// ArkSettings::OnSettingsChanged. The SDK dump could not type the property, so the generated header carries it
// as twelve opaque bytes - which is a TArray, and the static assert below is what keeps that true.
static TArray<TScriptInterface<IArkSettingsListenerInterface> >& DisMenuSettingsListeners( UDisGFxMoviePlayerMenuBase* Menu )
{
	checkAtCompileTime( sizeof(Menu->m_SettingsListeners) == sizeof(TArray<TScriptInterface<IArkSettingsListenerInterface> >), DisMenuSettingsListeners_is_not_a_TArray );
	return *(TArray<TScriptInterface<IArkSettingsListenerInterface> >*)Menu->m_SettingsListeners;
}

/** the localised label of one profile setting id, out of Settings.int's [ProfileSettingIDs] */
static FString DisSettingLabel( INT SettingID )
{
	const FString EnumName = DisEnumTypeToString( SettingID, TEXT("Engine.OnlineProfileSettings.EProfileSettingID") );
	return Localize( TEXT("ProfileSettingIDs"), *EnumName, TEXT("Settings") );
}

/** the localised label of one id-mapped value, out of Settings.int's [ProfileSettingValues] */
static FString DisSettingValueLabel( FName ValueName )
{
	return Localize( TEXT("ProfileSettingValues"), *ValueName.ToString(), TEXT("Settings") );
}

// DISHONORED(port): agent EO. UArkProfileSettings::GetKeyName (2013 rva 0x533560) is m_BindableKeyMapping[Key]
// and nothing else; retail then formats the FName through the localised [Keys] section (0x793dc0), whose
// keyboard-layout branches - the OEM keys read back through MapVirtualKeyEx and the left/right mouse swap - are
// not ported here, so a non-US layout will name its punctuation keys by their US FName.
static FString DisBindableKeyLabel( UArkProfileSettings* Settings, INT BindableKey )
{
	if( Settings == NULL || BindableKey < 0 || BindableKey >= ARRAY_COUNT(Settings->m_BindableKeyMapping) )
	{
		return FString();
	}
	const FName KeyName = Settings->m_BindableKeyMapping[BindableKey];
	if( KeyName == NAME_None )
	{
		return Localize( TEXT("Keys"), TEXT("Unbound_Menu"), TEXT("Settings") );
	}
	return Localize( TEXT("Keys"), *KeyName.ToString(), TEXT("Settings") );
}

/** the bindable-action span of retail's id space: ids 29..64 name a key rather than carry a number */
static UBOOL DisSettingIsKeyBinding( INT SettingID )
{
	return SettingID >= 29 && SettingID <= 64;
}

// DISHONORED(port): 2013 rva 0x7db5f0 (2012 0x80f920, CreateGFxSetting). One AS2 object per row of an options
// page. The member names are the contract with the cooked asset: Setting_Id, Setting_Name, Mapping_Type,
// Setting_Value, Setting_Minimum, Setting_Maximum, Setting_Increment, Mapping_Names, and exactly ONE of the five
// screen flags - retail's else-if chain leaves the other four members absent rather than false.
//
// The value arm is the whole point of the row and is driven by the mapping type of the setting's own
// FSettingsPropertyPropertyMetaData: PVMT_RawValue reads the plain int/float accessors, PVMT_Ranged reads the
// ranged ones and adds the slider's min/max/increment, and the two id-mapped types (3 and 4 - Dishonored gives
// every boolean option 4) turn the value mappings into the drop list's labels and the current value into an
// index into that list.
static void DisCreateGFxSetting( UDisGFxMoviePlayerMenuBase* Menu, const FDisSetting& Setting, GFxValue* OutSetting )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	UArkProfileSettings* Settings = DisMenuProfileSettings();

	// GFxMovieView::CreateObject takes the class name and the constructor arguments (vt[13] in the
	// PDB); an empty Object is what agent BE's one-argument stand-in meant.
	View->CreateObject( OutSetting, "Object", NULL, 0 );
	DisSetGFxNumber( *OutSetting, "Setting_Id", Setting.m_SettingID );
	const FString RowName = Setting.m_SettingNameOverride.Len() > 0 ? FString( Setting.m_SettingNameOverride ) : DisSettingLabel( Setting.m_SettingID );
	DisSetGFxString( *OutSetting, "Setting_Name", RowName );

	const INT MappingIndex = Settings ? Settings->FindProfileMappingIndex( Setting.m_SettingID ) : INDEX_NONE;
	if( Setting.m_SettingID != -1 && Settings != NULL && Settings->ProfileMappings.IsValidIndex( MappingIndex ) )
	{
		const FSettingsPropertyPropertyMetaData& MetaData = Settings->ProfileMappings(MappingIndex);
		DisSetGFxNumber( *OutSetting, "Mapping_Type", MetaData.MappingType );

		GFxValue MappingNames;
		View->CreateArray( &MappingNames );

		switch( MetaData.MappingType )
		{
		case PVMT_RawValue:
			{
				INT IntValue = 0;
				FLOAT FloatValue = 0.f;
				if( Settings->GetProfileSettingValueInt( Setting.m_SettingID, IntValue ) )
				{
					if( DisSettingIsKeyBinding( Setting.m_SettingID ) )
					{
						DisSetGFxString( *OutSetting, "Setting_Value", DisBindableKeyLabel( Settings, IntValue ) );
					}
					else
					{
						DisSetGFxNumber( *OutSetting, "Setting_Value", IntValue );
					}
				}
				else if( Settings->GetProfileSettingValueFloat( Setting.m_SettingID, FloatValue ) )
				{
					DisSetGFxNumber( *OutSetting, "Setting_Value", FloatValue );
				}
			}
			break;

		case PVMT_Ranged:
			{
				INT IntValue = 0;
				FLOAT FloatValue = 0.f;
				if( Settings->GetRangedProfileSettingValueInt( Setting.m_SettingID, IntValue ) )
				{
					DisSetGFxNumber( *OutSetting, "Setting_Value", IntValue );
				}
				else if( Settings->GetRangedProfileSettingValueFloat( Setting.m_SettingID, FloatValue ) )
				{
					DisSetGFxNumber( *OutSetting, "Setting_Value", FloatValue );
				}
				FLOAT MinValue = 0.f;
				FLOAT MaxValue = 0.f;
				FLOAT Increment = 0.f;
				BYTE bFormatAsInt = 0;
				if( Settings->GetProfileSettingRange( Setting.m_SettingID, MinValue, MaxValue, Increment, bFormatAsInt ) )
				{
					DisSetGFxNumber( *OutSetting, "Setting_Minimum", MinValue );
					DisSetGFxNumber( *OutSetting, "Setting_Maximum", MaxValue );
					DisSetGFxNumber( *OutSetting, "Setting_Increment", Increment );
				}
			}
			break;

		case PVMT_IdMapped:
		case PVMT_MAX:
			{
				if( MetaData.ValueMappings.Num() > 0 )
				{
					INT CurrentId = 0;
					Settings->GetProfileSettingValueId( Setting.m_SettingID, CurrentId );
					for( INT ValueIdx = 0; ValueIdx < MetaData.ValueMappings.Num(); ValueIdx++ )
					{
						const FIdToStringMapping& Mapping = MetaData.ValueMappings(ValueIdx);
						// DISHONORED(port): the fourth DLC06 difficulty (EDifficulty_VeryHard, "Master
						// Assassin") is offered only once PSI_DLC06_MasterAssassinUnlocked (id 151) is set.
						if( Setting.m_SettingID == 152 && Mapping.Id == 3 && !DisIsMasterAssassinUnlocked( Settings ) )
						{
							continue;
						}
						GFxValue Label;
						Label.SetStringW( *DisSettingValueLabel( Mapping.Name ) );
						MappingNames.PushBack( Label );
						if( CurrentId == Mapping.Id )
						{
							DisSetGFxNumber( *OutSetting, "Setting_Value", MappingNames.GetArraySize() - 1 );
						}
						Label.ReleaseManaged();
					}
				}
				else
				{
					// DISHONORED(port): agent EQ. A setting with no value mappings of its own takes its list and
					// its current row from ArkSettings::GetSettingProvider (2013 rva 0x5396a0). Id 115,
					// PSI_GraphicsPC_Resolution, is the only one: the provider is PCResolutionSettingProvider and
					// the list is the display modes at or above 800x600, formatted "%4d x %4d".
					ArkSettings::SettingProvider& Provider = ArkSettings::GetSettingProvider( Setting.m_SettingID );
					TArray<FString> DynamicNames;
					Provider.GetDynamicValueNames( DynamicNames );
					for( INT ValueIdx = 0; ValueIdx < DynamicNames.Num(); ValueIdx++ )
					{
						GFxValue Label;
						Label.SetStringW( *DynamicNames(ValueIdx) );
						MappingNames.PushBack( Label );
						Label.ReleaseManaged();
					}
					DisSetGFxNumber( *OutSetting, "Setting_Value", Provider.GetCurrentValueIndex() );
					warnf( TEXT("DisCreateGFxSetting: dynamic row id %d (%s): %d values, current %d (%s)"),
						Setting.m_SettingID, *DisEnumTypeToString( Setting.m_SettingID, TEXT("Engine.OnlineProfileSettings.EProfileSettingID") ),
						DynamicNames.Num(), Provider.GetCurrentValueIndex(),
						DynamicNames.IsValidIndex( Provider.GetCurrentValueIndex() ) ? *DynamicNames(Provider.GetCurrentValueIndex()) : TEXT("none") );
				}
				OutSetting->SetMember( "Mapping_Names", MappingNames );
			}
			break;

		default:
			break;
		}
		MappingNames.ReleaseManaged();
	}

	// retail sets exactly one of these, in this order, and always to true
	if( Setting.m_bGamepadBindingMenu )
	{
		DisSetGFxBool( *OutSetting, "GamepadBindingMenu", TRUE );
	}
	else if( Setting.m_bVideoMenu )
	{
		DisSetGFxBool( *OutSetting, "VideoSettings", TRUE );
	}
	else if( Setting.m_bGammaMenu )
	{
		DisSetGFxBool( *OutSetting, "GammaMenu", TRUE );
	}
	else if( Setting.m_bDeviceSelectionMenu )
	{
		DisSetGFxBool( *OutSetting, "DeviceSelectionMenu", TRUE );
	}
	else if( Setting.m_bDropList )
	{
		DisSetGFxBool( *OutSetting, "bDropList", TRUE );
	}
}

// DISHONORED(port): 2012 rva 0x813db0 (CreateGFxSubCategory)
static void DisCreateGFxSubCategory( UDisGFxMoviePlayerMenuBase* Menu, const FDisSettingsSubCategory& SubCategory, GFxValue* OutSubCategory )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	View->CreateObject( OutSubCategory, "Object", NULL, 0 );
	DisSetGFxString( *OutSubCategory, "SubCategory_Name", SubCategory.m_SubCategoryName );
	DisSetGFxBool( *OutSubCategory, "KeyboardBindingMenu", SubCategory.m_bKeyboardBindingMenu );

	GFxValue SettingList;
	View->CreateArray( &SettingList );
	for( INT Index = 0; Index < SubCategory.m_Settings.Num(); Index++ )
	{
		GFxValue Setting;
		DisCreateGFxSetting( Menu, SubCategory.m_Settings(Index), &Setting );
		SettingList.PushBack( Setting );
		Setting.ReleaseManaged();
	}
	OutSubCategory->SetMember( "Setting_List", SettingList );
	SettingList.ReleaseManaged();
}

// DISHONORED(port): 2012 rva 0x815a50 (CreateGFxCategory) - a category is a name plus its own settings plus
// its sub-categories, each of which is another list of settings
static void DisCreateGFxCategory( UDisGFxMoviePlayerMenuBase* Menu, const FDisSettingsCategory& Category, GFxValue* OutCategory )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	View->CreateObject( OutCategory, "Object", NULL, 0 );
	DisSetGFxString( *OutCategory, "Category_Name", Category.m_CategoryName );

	GFxValue SubCategories;
	View->CreateArray( &SubCategories );
	for( INT Index = 0; Index < Category.m_SubCategories.Num(); Index++ )
	{
		GFxValue SubCategory;
		DisCreateGFxSubCategory( Menu, Category.m_SubCategories(Index), &SubCategory );
		SubCategories.PushBack( SubCategory );
		SubCategory.ReleaseManaged();
	}
	OutCategory->SetMember( "SubCategories", SubCategories );
	SubCategories.ReleaseManaged();

	GFxValue SettingList;
	View->CreateArray( &SettingList );
	for( INT Index = 0; Index < Category.m_Settings.Num(); Index++ )
	{
		GFxValue Setting;
		DisCreateGFxSetting( Menu, Category.m_Settings(Index), &Setting );
		SettingList.PushBack( Setting );
		Setting.ReleaseManaged();
	}
	OutCategory->SetMember( "Setting_List", SettingList );
	SettingList.ReleaseManaged();
}

// DISHONORED(port): 2013 rva 0x7e3920 (2012 0x817250, ShowSettingsCategoryList) - the whole tree in one AS2
// call. The clip is _root.optionsMenu_mc, which is the string retail's own body holds; this unit had
// _root.options_mc, a name that is in no retail function and in no cooked movie, so FillCategories was never
// reached and every options row the asset drew was undefined. The same name was wrong in TryBindKey.
static void DisShowSettingsCategoryList( UDisGFxMoviePlayerMenuBase* Menu )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	GFxValue Categories;
	View->CreateArray( &Categories );
	for( INT Index = 0; Index < Menu->m_SettingsCategoryList.Num(); Index++ )
	{
		GFxValue Category;
		DisCreateGFxCategory( Menu, Menu->m_SettingsCategoryList(Index), &Category );
		Categories.PushBack( Category );
		Category.ReleaseManaged();
	}
	GFxValue Options;
	const UBOOL bFound = View->GetVariable( &Options, "_root.optionsMenu_mc" );
	if( bFound )
	{
		GFxValue Unused;
		Options.Invoke( "FillCategories", &Unused, &Categories, 1 );
		Unused.ReleaseManaged();
	}
	warnf( TEXT("DisShowSettingsCategoryList: %d categories -> _root.optionsMenu_mc.FillCategories %s"),
		Menu->m_SettingsCategoryList.Num(), bFound ? TEXT("ok") : TEXT("NOT FOUND") );
	Options.ReleaseManaged();
	Categories.ReleaseManaged();
}

// DISHONORED(port): 2013 rva 0x7e50c0 (2012 0x81ba20, FillSettingsCategoryList). The whole options tree, which
// retail hard-codes: four categories, their sub-categories, and each sub-category's run of profile setting ids.
// The category and sub-category names come off m_pMenuBaseTweaks, the row labels off Settings.int, and the values
// off the cooked profile through DisCreateGFxSetting.
//
// bAllowRestartSettings is retail's one argument, TRUE from the main menu and FALSE from the pause menu: the two
// settings that need a restart - PSI_GraphicsPC_TextureDetails (119) and PSI_AudioPC_SpeakerConfiguration (133) -
// are offered only when it is set.
static void DisFillSettingsCategoryList( UDisGFxMoviePlayerMenuBase* Menu, UBOOL bAllowRestartSettings, TArray<FDisSettingsCategory>& OutCategories )
{
	UDisTweaks_GFxMoviePlayerMenuBase* Tweaks = Menu->m_pMenuBaseTweaks;
	if( Tweaks == NULL )
	{
		// every name below would be empty and the asset would draw a nameless tree
		warnf( TEXT("DisFillSettingsCategoryList: m_pMenuBaseTweaks is NULL, the options tree has no names") );
		return;
	}

	// which campaign is running decides whether the difficulty row is the main one or DLC06's: the Dunwall City
	// Trials (DisDLC05GameInfo) have neither a difficulty nor an autosave setting, and the two assassin campaigns
	// carry their own difficulty at PSI_DLC06_Difficulty (152).
	UClass* GameInfoClass = ( GWorld && GWorld->GetGameInfo() ) ? GWorld->GetGameInfo()->GetClass() : NULL;
	const UBOOL bTrialsCampaign = GameInfoClass != NULL && GameInfoClass->IsChildOf( ADisDLC05GameInfo::StaticClass() );
	const UBOOL bAssassinCampaign = GameInfoClass != NULL
		&& GameInfoClass != ADishonoredGameInfo::StaticClass()
		&& GameInfoClass != ADisDLC05GameInfo::StaticClass();

	{
		FDisSettingsCategory& General = OutCategories( OutCategories.AddItem( FDisSettingsCategory(EC_EventParm) ) );
		General.m_CategoryName = Tweaks->m_SettingsCategory_General;
		{
			FDisSettingsSubCategory& Gameplay = General.m_SubCategories( General.m_SubCategories.AddItem( FDisSettingsSubCategory(EC_EventParm) ) );
			Gameplay.m_SubCategoryName = Tweaks->m_SettingsSubCategory_GameplaySettings;
			for( INT SettingID = 104; SettingID < 110; SettingID++ )
			{
				if( bTrialsCampaign && ( SettingID == 106 || SettingID == 107 ) )
				{
					continue;
				}
				FDisSetting Row(EC_EventParm);
				Row.m_SettingID = ( bAssassinCampaign && SettingID == 106 ) ? 152 : SettingID;
				Gameplay.m_Settings.AddItem( Row );
			}
		}
		{
			FDisSettingsSubCategory& HUD = General.m_SubCategories( General.m_SubCategories.AddItem( FDisSettingsSubCategory(EC_EventParm) ) );
			HUD.m_SubCategoryName = Tweaks->m_SettingsSubCategory_HUDSettings;
			for( INT SettingID = 87; SettingID < 102; SettingID++ )
			{
				FDisSetting Row(EC_EventParm);
				Row.m_SettingID = SettingID;
				HUD.m_Settings.AddItem( Row );
			}
		}
	}

	{
		FDisSettingsCategory& Controls = OutCategories( OutCategories.AddItem( FDisSettingsCategory(EC_EventParm) ) );
		Controls.m_CategoryName = Tweaks->m_SettingsCategory_Controls;
		{
			FDisSettingsSubCategory& Keyboard = Controls.m_SubCategories( Controls.m_SubCategories.AddItem( FDisSettingsSubCategory(EC_EventParm) ) );
			Keyboard.m_SubCategoryName = Tweaks->m_SettingsSubCategory_KeyboardMapping;
			Keyboard.m_bKeyboardBindingMenu = TRUE;
			for( INT SettingID = 29; SettingID < 65; SettingID++ )
			{
				FDisSetting Row(EC_EventParm);
				Row.m_SettingID = SettingID;
				Keyboard.m_Settings.AddItem( Row );
			}
		}
		{
			FDisSettingsSubCategory& Mouse = Controls.m_SubCategories( Controls.m_SubCategories.AddItem( FDisSettingsSubCategory(EC_EventParm) ) );
			Mouse.m_SubCategoryName = Tweaks->m_SettingsSubCategory_MouseSettings;
			for( INT SettingID = 67; SettingID < 74; SettingID++ )
			{
				FDisSetting Row(EC_EventParm);
				Row.m_SettingID = SettingID;
				Mouse.m_Settings.AddItem( Row );
			}
		}
		{
			FDisSettingsSubCategory& Gamepad = Controls.m_SubCategories( Controls.m_SubCategories.AddItem( FDisSettingsSubCategory(EC_EventParm) ) );
			Gamepad.m_SubCategoryName = Tweaks->m_SettingsSubCategory_GamepadSettings;
			for( INT SettingID = 76; SettingID < 85; SettingID++ )
			{
				FDisSetting Row(EC_EventParm);
				Row.m_SettingID = SettingID;
				// the control-scheme row opens the pad diagram instead of editing in place
				Row.m_bGamepadBindingMenu = ( SettingID == 76 );
				Gamepad.m_Settings.AddItem( Row );
			}
		}
	}

	{
		FDisSettingsCategory& Graphics = OutCategories( OutCategories.AddItem( FDisSettingsCategory(EC_EventParm) ) );
		Graphics.m_CategoryName = Tweaks->m_SettingsCategory_Graphics;
		{
			// the video sub-screen has no profile setting of its own, so it carries an id of -1 and its own name
			FDisSetting Video(EC_EventParm);
			Video.m_SettingID = -1;
			Video.m_SettingNameOverride = Tweaks->m_SettingsSubMenu_VideoSettings;
			Video.m_bVideoMenu = TRUE;
			Graphics.m_Settings.AddItem( Video );
		}
		{
			// PSI_Graphics_Gamma: the brightness row, and the one row that opens the gamma calibration screen
			FDisSetting Gamma(EC_EventParm);
			Gamma.m_SettingID = 112;
			Gamma.m_bGammaMenu = TRUE;
			Graphics.m_Settings.AddItem( Gamma );
		}
		for( INT SettingID = 118; SettingID < 124; SettingID++ )
		{
			if( SettingID == 119 && !bAllowRestartSettings )
			{
				continue;
			}
			FDisSetting Row(EC_EventParm);
			Row.m_SettingID = SettingID;
			Graphics.m_Settings.AddItem( Row );
		}
	}

	{
		FDisSettingsCategory& Audio = OutCategories( OutCategories.AddItem( FDisSettingsCategory(EC_EventParm) ) );
		Audio.m_CategoryName = Tweaks->m_SettingsCategory_Audio;
		for( INT SettingID = 126; SettingID < 131; SettingID++ )
		{
			FDisSetting Row(EC_EventParm);
			Row.m_SettingID = SettingID;
			Audio.m_Settings.AddItem( Row );
		}
		if( bAllowRestartSettings )
		{
			FDisSetting Speakers(EC_EventParm);
			Speakers.m_SettingID = 133;
			Audio.m_Settings.AddItem( Speakers );
		}
	}
}

// DISHONORED(port): 2013 rva 0x7e65f0 (2012 0x81da70, FillOptionsMenu). Rereads the shared parameters out of the
// profile, rebuilds the listener list and the whole tree, hands the tree to AS2, and caches the objective-marker
// flag so OnLeaveOptions can tell whether the player changed it.
void DisFillOptionsMenu( UDisGFxMoviePlayerMenuBase* Menu, UBOOL bAllowRestartSettings )
{
	UArkProfileSettings* Settings = DisMenuProfileSettings();
	if( Settings == NULL )
	{
		warnf( TEXT("DisFillOptionsMenu: no player profile, the options tree has no values") );
	}
	else
	{
		// retail's ArkSettings::UpdateSettingsFromSystemSettings (2013 rva 0x53b7b0) is exactly this line
		ArkSettings::GetParameters().Read( Settings, TRUE );
	}

	DisMenuSettingsListeners( Menu ).Empty();
	Menu->m_SettingsCategoryList.Reset();
	ArkSettings::FindListeners( DisMenuSettingsListeners( Menu ) );
	DisFillSettingsCategoryList( Menu, bAllowRestartSettings, Menu->m_SettingsCategoryList );
	DisShowSettingsCategoryList( Menu );

	INT ShowObjectiveMarkers = 0;
	if( Settings )
	{
		Settings->GetProfileSettingValueId( 95, ShowObjectiveMarkers );
	}
	Menu->m_bInitialShowObjectiveMarkers = ( ShowObjectiveMarkers == 1 );

	INT RowCount = 0;
	for( INT CategoryIdx = 0; CategoryIdx < Menu->m_SettingsCategoryList.Num(); CategoryIdx++ )
	{
		const FDisSettingsCategory& Category = Menu->m_SettingsCategoryList(CategoryIdx);
		RowCount += Category.m_Settings.Num();
		for( INT SubIdx = 0; SubIdx < Category.m_SubCategories.Num(); SubIdx++ )
		{
			RowCount += Category.m_SubCategories(SubIdx).m_Settings.Num();
		}
	}
	warnf( TEXT("DisFillOptionsMenu: %d categories, %d rows, %d listeners, profile %s, initial objective markers %d"),
		Menu->m_SettingsCategoryList.Num(), RowCount, DisMenuSettingsListeners( Menu ).Num(),
		Settings ? *Settings->GetPathName() : TEXT("NULL"), Menu->m_bInitialShowObjectiveMarkers ? 1 : 0 );
}

// DISHONORED(port): agent EQ. Agent EO gated retail's republish loop behind -arksettings because nine
// DishonoredGame listeners still carried the generated appErrorf ApplyGameSettings and the first slider move
// aborted the game (build/agentEO/afterE_log.txt: `appError called: DishonoredGame native not ported:
// UDisPostProcessManager::ApplyGameSettings`). All nine are ported now, so the gate is gone and every setting
// change goes straight down retail's own path (2013 rva 0x53b7e0).
static void DisRepublishSettings( UOnlinePlayerStorage* Settings, TArray<TScriptInterface<IArkSettingsListenerInterface> >& Listeners, ArkSettings::EChangeReason Reason )
{
	ArkSettings::OnSettingsChanged( Settings, Listeners, Reason );
}

// DISHONORED(port): agent EO. Retail fills the options tree once, out of
// UDisGFxMoviePlayerMenuBase::PostFirstAdvance (2013 rva 0x7e6970, at +1204 of the main menu vtable at
// 0xd64960; it is new in 2013 - its other half is the DLC06 return transition - so the 2012 build has no name
// for it), and NOT out of OnOptionsClicked alone. That is why retail's new-game brightness screen already has a
// value: the gamma row is in the asset before the screen is reached. This tree's UDisGFxMoviePlayerBase has no
// PostFirstAdvance, and _root.optionsMenu_mc is placed about two frames after PostStart - measured, 0.07 s later
// in build/agentEO/afterB_log.txt - so the fill happens on the first advance on which the clip exists, which is
// once, because the category list is only empty until then.
void DisFillOptionsMenuOnce( UDisGFxMoviePlayerMenuBase* Menu, UBOOL bAllowRestartSettings )
{
	if( Menu->m_SettingsCategoryList.Num() > 0 )
	{
		return;
	}
	GFxMovieView* View = DisMenuView( Menu );
	GFxValue OptionsMenu;
	const UBOOL bReady = View != NULL && View->GetVariable( &OptionsMenu, "_root.optionsMenu_mc" );
	OptionsMenu.ReleaseManaged();
	if( bReady )
	{
		DisFillOptionsMenu( Menu, bAllowRestartSettings );
	}
}

// DISHONORED(port): 2013 rva 0x7bc9d0 (2012 0x7f71c0, CloseOptions) - the one AS2 call that takes the options
// screen back to the menu it came from. Without it a B on the options screen reaches the content's own BPressed,
// which calls OnLeaveOptions and then waits for this.
static void DisCloseOptions( UDisGFxMoviePlayerMenuBase* Menu )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	GFxValue OptionsMenu;
	if( View->GetVariable( &OptionsMenu, "_root.optionsMenu_mc" ) )
	{
		GFxValue Unused;
		OptionsMenu.Invoke( "Close", &Unused, NULL, 0 );
		Unused.ReleaseManaged();
	}
	else
	{
		warnf( TEXT("DisCloseOptions: _root.optionsMenu_mc NOT FOUND") );
	}
	OptionsMenu.ReleaseManaged();
}

// DISHONORED(port): 2012 rva 0x8137f0 (FindSaveName). "<autosave or quicksave prefix><localised map name>".
// Retail reads the prefixes off m_pMenuBaseTweaks and the map name out of the localised MapNames config section
// keyed by FMapConfig::m_Name. Retail has this as a protected method of the class; it is a file-static here for
// the same reason agent BE made the other leaves static - UDisGFxMoviePlayerMenuBase has no CppText header and
// adding one would need an edit to a generated header.
static void DisFindSaveName( UDisGFxMoviePlayerMenuBase* Menu, const FDisSaveGame* SaveGame, FString& OutName )
{
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	FMapConfig* MapConfig = Engine ? Engine->FindMapConfigFromFriendlyName( SaveGame->m_MapName ) : NULL;

	// DISHONORED(bringup): m_pMenuBaseTweaks is a per-subclass tweaks pointer, so it is NULL until the tweak
	// objects load; the census line in DisFillLoadGameMenu prints what was actually read.
	if( SaveGame->m_Slot == DIS_SAVE_SLOT_FIRST_AUTO || SaveGame->m_Slot == DIS_SAVE_SLOT_FIRST_AUTO + 1 )
	{
		OutName = Menu->m_pMenuBaseTweaks ? Menu->m_pMenuBaseTweaks->m_AutosavePrefix : FString();
	}
	else if( SaveGame->m_Slot == DIS_SAVE_SLOT_QUICK )
	{
		OutName = Menu->m_pMenuBaseTweaks ? Menu->m_pMenuBaseTweaks->m_QuicksavePrefix : FString();
	}
	else
	{
		OutName = FString();
	}

	if( MapConfig )
	{
		// retail looks the map name up in the localised MapNames section keyed by FMapConfig::m_Name
		const FString Localised = Localize( TEXT("MapNames"), *MapConfig->m_Name, TEXT("DishonoredGame"), NULL, TRUE );
		if( Localised.Len() > 0 )
		{
			OutName += Localised;
			return;
		}
		// the friendly name is what the save file itself carries, and it is what the map list is keyed on
		OutName += MapConfig->m_FriendlyName;
		return;
	}
	OutName += SaveGame->m_MapName.Len() > 0 ? SaveGame->m_MapName : FString( TEXT("UNKNOWN") );
}

// DISHONORED(port): 2012 rva 0x806880 (FormatSaveDate) - m_DateFormat with day / month / year / hour / minute
// replaced in place from localtime of the file's mtime
static void DisFormatSaveDate( UDisGFxMoviePlayerMenuBase* Menu, const FDisSaveGame* SaveGame, FString& OutDate )
{
	OutDate = Menu->m_pMenuBaseTweaks ? Menu->m_pMenuBaseTweaks->m_DateFormat : FString( TEXT("day/month/year hour:minute") );
	const __time64_t SaveTime = (__time64_t)SaveGame->m_Time;
	struct tm* Local = _localtime64( &SaveTime );
	if( Local == NULL )
	{
		return;
	}
	OutDate.ReplaceInline( TEXT("day"), *FString::Printf( TEXT("%02i"), Local->tm_mday ) );
	OutDate.ReplaceInline( TEXT("month"), *FString::Printf( TEXT("%02i"), Local->tm_mon + 1 ) );
	OutDate.ReplaceInline( TEXT("year"), *FString::Printf( TEXT("%02i"), Local->tm_year + 1900 ) );
	OutDate.ReplaceInline( TEXT("hour"), *FString::Printf( TEXT("%02i"), Local->tm_hour ) );
	OutDate.ReplaceInline( TEXT("minute"), *FString::Printf( TEXT("%02i"), Local->tm_min ) );
}

// DISHONORED(port): 2012 rva 0x7f7120 (FindSaveImagePath) - "img://<package>.MissionsScreen_<image>_Small"
static void DisFindSaveImagePath( UDisGFxMoviePlayerMenuBase* Menu, const FDisSaveGame* SaveGame, FString& OutPath, UBOOL bLarge )
{
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	FMapConfig* MapConfig = Engine ? Engine->FindMapConfigFromFriendlyName( SaveGame->m_MapName ) : NULL;
	OutPath = TEXT("img://");
	if( MapConfig == NULL || MapConfig->m_ImageName.Len() == 0 || Menu->m_pMenuBaseTweaks == NULL )
	{
		return;
	}
	OutPath += bLarge ? Menu->m_pMenuBaseTweaks->m_MapLargeImagePackage : Menu->m_pMenuBaseTweaks->m_MapSmallImagePackage;
	OutPath += TEXT(".MissionsScreen_");
	OutPath += MapConfig->m_ImageName;
	OutPath += bLarge ? TEXT("_Large") : TEXT("_Small");
}

// DISHONORED(port): 2012 rva 0x815830 (FillLoadGameMenu) + 0x813930 (CreateGFxLoadGameList). The list is an
// AS2 array of objects with chapterName / saveDate / itemThumb, pushed into _root.loadGame_mc.SetLoadGame,
// and m_LoadGameSlots is the parallel array of real save slots that OnLoadGameConfirm indexes.
static void DisFillLoadGameMenu( UDisGFxMoviePlayerMenuBase* Menu )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	Menu->m_LoadGameSlots.Empty();

	GFxValue LoadGameList;
	View->CreateArray( &LoadGameList );

	// DISHONORED(port): agent CF - the save list is real now (2012 0x813930). Retail walks GetSaveGame(0)..
	// until it returns NULL, keeps every row whose slot is a user slot, and records the slot in
	// m_LoadGameSlots so OnLoadGameConfirm can turn a row index back into a slot.
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	INT SaveListIdx = 0;
	for( FDisSaveGame* SaveGame = Engine ? Engine->GetSaveGame( SaveListIdx ) : NULL;
		 SaveGame != NULL;
		 SaveGame = Engine->GetSaveGame( ++SaveListIdx ) )
	{
		if( SaveGame->m_Slot < DIS_SAVE_SLOT_FIRST_AUTO )
		{
			continue;
		}
		FString SaveName;
		FString FormattedDate;
		FString SaveImagePath;
		DisFindSaveName( Menu, SaveGame, SaveName );
		DisFormatSaveDate( Menu, SaveGame, FormattedDate );
		DisFindSaveImagePath( Menu, SaveGame, SaveImagePath, FALSE );

		GFxValue Entry;
		View->CreateObject( &Entry, "Object", NULL, 0 );
		DisSetGFxString( Entry, "chapterName", SaveName );
		DisSetGFxString( Entry, "saveDate", FormattedDate );
		DisSetGFxString( Entry, "itemThumb", SaveImagePath );
		LoadGameList.PushBack( Entry );
		Entry.ReleaseManaged();

		Menu->m_LoadGameSlots.AddItem( SaveGame->m_Slot );
	}
	// instrumentation, because m_pMenuBaseTweaks is a per-subclass pointer and reading it through the base is
	// exactly the trap that has caught two agents on this tree
	warnf( TEXT("DisFillLoadGameMenu: %d of %d saves listed, tweaks %s (autosave '%s' quicksave '%s' date '%s' small '%s')"),
		Menu->m_LoadGameSlots.Num(), SaveListIdx,
		Menu->m_pMenuBaseTweaks ? *Menu->m_pMenuBaseTweaks->GetPathName() : TEXT("NULL"),
		Menu->m_pMenuBaseTweaks ? *Menu->m_pMenuBaseTweaks->m_AutosavePrefix : TEXT(""),
		Menu->m_pMenuBaseTweaks ? *Menu->m_pMenuBaseTweaks->m_QuicksavePrefix : TEXT(""),
		Menu->m_pMenuBaseTweaks ? *Menu->m_pMenuBaseTweaks->m_DateFormat : TEXT(""),
		Menu->m_pMenuBaseTweaks ? *Menu->m_pMenuBaseTweaks->m_MapSmallImagePackage : TEXT("") );

	GFxValue LoadGame;
	if( View->GetVariable( &LoadGame, "_root.loadGame_mc" ) )
	{
		GFxValue Unused;
		LoadGame.Invoke( "SetLoadGame", &Unused, &LoadGameList, 1 );
		Unused.ReleaseManaged();
	}
	LoadGame.ReleaseManaged();
	LoadGameList.ReleaseManaged();
}

// DISHONORED(port): 2012 rva 0x5f7b80 exec / 2013 rva 0x7cb870 body (2012 0x806b10). One row of an options page
// was edited: the AS2 number is rounded to the nearest half, written back through whichever accessor family the
// setting's mapping type belongs to, and every listener is told. The dirty bit only ever accumulates - it is
// OnLeaveOptions that clears it.
void UDisGFxMoviePlayerMenuBase::execOnSettingChange( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_SettingID);
	P_GET_FLOAT(_fValue);
	P_FINISH;

	UArkProfileSettings* Settings = DisMenuProfileSettings();
	if( Settings == NULL )
	{
		return;
	}
	const INT MappingIndex = Settings->FindProfileMappingIndex( _SettingID );
	const INT SettingIndex = Settings->FindProfileSettingIndex( _SettingID );
	if( !Settings->ProfileMappings.IsValidIndex( MappingIndex ) || !Settings->ProfileSettings.IsValidIndex( SettingIndex ) )
	{
		return;
	}
	const FSettingsPropertyPropertyMetaData& MetaData = Settings->ProfileMappings(MappingIndex);
	const BYTE DataType = Settings->ProfileSettings(SettingIndex).ProfileSetting.Data.Type;
	const INT Rounded = appTrunc( _fValue * 2.f + 0.5f ) >> 1;
	UBOOL bChanged = FALSE;

	switch( MetaData.MappingType )
	{
	case PVMT_RawValue:
		if( DataType == SDT_Int32 )
		{
			// a key binding is exclusive: whichever other action already holds this key loses it
			if( DisSettingIsKeyBinding( _SettingID ) && Rounded != 0 )
			{
				for( INT OtherID = 29; OtherID <= 64; OtherID++ )
				{
					INT OtherKey = 0;
					Settings->GetProfileSettingValueInt( OtherID, OtherKey );
					if( OtherKey == Rounded && OtherID != _SettingID )
					{
						Settings->SetProfileSettingValueInt( OtherID, 0 );
					}
				}
			}
			INT OldValue = -1;
			Settings->GetProfileSettingValueInt( _SettingID, OldValue );
			Settings->SetProfileSettingValueInt( _SettingID, Rounded );
			bChanged = Rounded != OldValue;
		}
		else if( DataType == SDT_Float )
		{
			FLOAT OldValue = -1.f;
			Settings->GetProfileSettingValueFloat( _SettingID, OldValue );
			Settings->SetProfileSettingValueFloat( _SettingID, _fValue );
			bChanged = Abs( _fValue - OldValue ) >= 0.0001f;
		}
		break;

	case PVMT_Ranged:
		if( DataType == SDT_Int32 )
		{
			INT OldValue = -1;
			Settings->GetRangedProfileSettingValueInt( _SettingID, OldValue );
			Settings->SetRangedProfileSettingValueInt( _SettingID, Rounded );
			bChanged = Rounded != OldValue;
		}
		else if( DataType == SDT_Float )
		{
			FLOAT OldValue = -1.f;
			Settings->GetRangedProfileSettingValueFloat( _SettingID, OldValue );
			Settings->SetRangedProfileSettingValueFloat( _SettingID, _fValue );
			bChanged = Abs( _fValue - OldValue ) >= 0.0001f;
		}
		break;

	case PVMT_IdMapped:
	case PVMT_MAX:
		if( MetaData.ValueMappings.Num() > 0 )
		{
			if( MetaData.ValueMappings.IsValidIndex( Rounded ) )
			{
				const INT NewId = MetaData.ValueMappings(Rounded).Id;
				INT OldId = -1;
				Settings->GetProfileSettingValueId( _SettingID, OldId );
				Settings->SetProfileSettingValueId( _SettingID, NewId );
				bChanged = NewId != OldId;
			}
		}
		else
		{
			// DISHONORED(port): agent EQ. The dynamic arm: the row the player picked becomes the provider's own
			// current index (ArkSettings::GetSettingProvider, 2013 rva 0x5396a0, slots 2 and 4). Nothing is
			// written into the profile - the resolution lives in the provider and in GSystemSettings, which is
			// why ArkSettingsParameters::Read takes it from the provider when it is not overriding.
			ArkSettings::SettingProvider& Provider = ArkSettings::GetSettingProvider( _SettingID );
			const INT OldIndex = Provider.GetCurrentValueIndex();
			Provider.SetCurrentValueIndex( Rounded );
			bChanged = Rounded != OldIndex;
		}
		break;

	default:
		break;
	}

	if( bChanged )
	{
		m_bOptionsChanged = TRUE;
	}
	warnf( TEXT("DisOnSettingChange: id %d (%s) mapping %d data %d value %.4f -> rounded %d, changed %d, gamma now %.4f"),
		_SettingID, *DisEnumTypeToString( _SettingID, TEXT("Engine.OnlineProfileSettings.EProfileSettingID") ),
		MetaData.MappingType, DataType, _fValue, Rounded, bChanged ? 1 : 0, ArkSettings::GetParameters().m_fGamma );
	DisRepublishSettings( Settings, DisMenuSettingsListeners( this ), ArkSettings::ECR_ModifiedByUser );
}

// DISHONORED(port): 2013 rva 0x7bcad0 (2012 0x806de0, OnLeaveOptions), reached from the content's own BPressed on
// the options screen. Retail has two arms and only one of them closes the screen: with nothing changed it is
// CloseOptions, and with something changed it validates the new values, saves them, and leaves the screen up for
// the asset to take down through its own transition.
void UDisGFxMoviePlayerMenuBase::execOnLeaveOptions( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;

	if( !m_bOptionsChanged )
	{
		DisCloseOptions( this );
		return;
	}

	UArkProfileSettings* Settings = DisMenuProfileSettings();
	if( Settings == NULL )
	{
		DisCloseOptions( this );
		return;
	}
	DisRepublishSettings( Settings, DisMenuSettingsListeners( this ), ArkSettings::ECR_ValidatedByUser );
	// DISHONORED(port): agent EQ. ArkSettings::SaveSettings (2013 rva 0x5334e0) is retail's next call and it is
	// ported; what it can reach without a Steam client is recorded in resources/docs/agents/agentEQ.md.
	ArkSettings::SaveSettings( ADishonoredPlayerController::s_pInstance );
	m_bOptionsChanged = FALSE;
	m_bLeavingOptions = TRUE;

	INT ShowObjectiveMarkers = 0;
	Settings->GetProfileSettingValueId( 95, ShowObjectiveMarkers );
	const UBOOL bShowObjectiveMarkers = ( ShowObjectiveMarkers == 1 );
	if( bShowObjectiveMarkers != (UBOOL)m_bInitialShowObjectiveMarkers )
	{
		// DISHONORED(bringup): retail republishes the flag to the objective manager the player controller holds
		// at +1784 (2013 rva 0x6cbf50 walks its marker list); that manager is not reached from here yet, so the
		// markers already on screen keep their old state until the level reloads.
		m_bInitialShowObjectiveMarkers = bShowObjectiveMarkers;
	}
}

// DISHONORED(port): agent EQ, 2013 rva 0x7dc260 (2012 0x810590, Req_VideoSettingsScreen). The video sub-screen
// the GRAPHICS category's own row opens, and the only place the resolution row exists: three settings built by
// hand - 116 PSI_GraphicsPC_FullScreen, 117 PSI_GraphicsPC_VSync, 115 PSI_GraphicsPC_Resolution, in that order -
// and handed to _root.optionsMenu_mc.FillVideoSettings. The resolution alone gets m_bDropList (retail sets bit 4
// of the flag byte when the id is 115), because it is the one row with a list of its own rather than two states.
// This is where ArkSettings::GetSettingProvider is reached: FillSettingsCategoryList never carries id 115, so
// without this native the provider is never asked and the row does not exist at all.
void UDisGFxMoviePlayerMenuBase::execReq_VideoSettingsScreen( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;

	GFxMovieView* View = DisMenuView( this );
	if( View == NULL )
	{
		return;
	}
	static const INT VideoSettingIDs[3] = { 116, 117, 115 };
	GFxValue Settings;
	View->CreateArray( &Settings );
	for( INT Index = 0; Index < ARRAY_COUNT(VideoSettingIDs); Index++ )
	{
		FDisSetting Setting(EC_EventParm);
		Setting.m_SettingID = VideoSettingIDs[Index];
		Setting.m_bDropList = ( VideoSettingIDs[Index] == 115 );
		GFxValue Row;
		DisCreateGFxSetting( this, Setting, &Row );
		Settings.PushBack( Row );
		Row.ReleaseManaged();
	}
	GFxValue Options;
	const UBOOL bFound = View->GetVariable( &Options, "_root.optionsMenu_mc" );
	if( bFound )
	{
		GFxValue Unused;
		Options.Invoke( "FillVideoSettings", &Unused, &Settings, 1 );
		Unused.ReleaseManaged();
	}
	warnf( TEXT("DisReq_VideoSettingsScreen: 3 rows -> _root.optionsMenu_mc.FillVideoSettings %s"),
		bFound ? TEXT("ok") : TEXT("NOT FOUND") );
	Options.ReleaseManaged();
	Settings.ReleaseManaged();
}

// DISHONORED(port): 2013 rva 0x7e6b80 (2012 0x820070) - eight bytes: FillOptionsMenu(TRUE). TRUE is what offers
// the two settings that need a restart, which is why the main menu has them and the pause menu does not.
void UDisGFxMoviePlayerMainMenu::execOnOptionsClicked( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	DisFillOptionsMenu( this, TRUE );
}

// DISHONORED(port): 2012 rva 0x822b40 - the same eight bytes with FALSE
void UDisGFxMoviePlayerPauseMenu::execOnOptionsClicked( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	DisFillOptionsMenu( this, FALSE );
}

// DISHONORED(port): the pause menu declares the native separately but retail has no body of its own for it, so
// the base class's is what runs.
void UDisGFxMoviePlayerPauseMenu::execOnLeaveOptions( FFrame& Stack, RESULT_DECL )
{
	UDisGFxMoviePlayerMenuBase::execOnLeaveOptions( Stack, Result );
}

// DISHONORED(port): 2012 rva 0x5f7c10 exec / 0x817510 body - reset one sub-category to its defaults and
// push the tree back into AS2 so the rows show the new values
void UDisGFxMoviePlayerMenuBase::execOnResetOptions( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_CategoryIdx);
	P_GET_INT(_SubCategoryIdx);
	P_FINISH;

	if( !m_SettingsCategoryList.IsValidIndex( _CategoryIdx ) )
	{
		return;
	}
	m_bOptionsChanged = TRUE;
	// DISHONORED(bringup): retail collects the m_SettingID of every FDisSetting in the selected category or
	// sub-category and hands them to UArkProfileSettings to reset; unavailable here for the same reason.
	DisShowSettingsCategoryList( this );
}

// DISHONORED(port): 2012 rva 0x5f79a0 exec / 0x800ec0 body. _ListIdx is a row of the AS2 list, and
// m_LoadGameSlots is what turns it back into a save slot - which is why the two arrays are built together.
void UDisGFxMoviePlayerMenuBase::execOnLoadGameConfirm( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_ListIdx);
	P_FINISH;

	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	if( Engine && m_LoadGameSlots.IsValidIndex( _ListIdx ) )
	{
		m_bLoadingGame = TRUE;
		Engine->Dis_Load( m_LoadGameSlots(_ListIdx) );
	}
}

// DISHONORED(port): 2012 rva 0x5f7a00 exec / 0x800f40 body - the dirty flag is set whether or not the
// delete happens, because the list has to be rebuilt either way
void UDisGFxMoviePlayerMenuBase::execOnDeleteSaveConfirm( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bFromSaveGameMenu);
	P_GET_INT(_ListIdx);
	P_FINISH;

	m_bLoadGameListDirty = TRUE;
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	if( Engine && m_LoadGameSlots.IsValidIndex( _ListIdx ) )
	{
		// DISHONORED(port): agent CF - 2013 rva 0x5fbc80 queues the async deleter and sets SLC_WaitDeleting
		Engine->DeleteSaveGame( m_LoadGameSlots(_ListIdx) );
	}
}

// DISHONORED(port): 2012 rva 0x5f7aa0 exec - closing the list drops the save/load mode flags
void UDisGFxMoviePlayerMenuBase::execOnSaveGameListClosed( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	m_bIsInSaveMenu = FALSE;
	m_bIsInLoadMenu = FALSE;
}

// DISHONORED(port): 2012 rva 0x5f7960 exec - the menu asset greys out Load when this is false
void UDisGFxMoviePlayerMenuBase::execReq_CanLoadGame( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	// DISHONORED(port): agent CF - the whole body of retail's Req_CanLoadGame (2013 rva 0x7c25e0, reached
	// through the exec at 0x5f7960) is "is there an engine and does it have any save at all"
	*(UBOOL*)Result = ( Engine && Engine->HasSaveGame( 0 ) ) ? TRUE : FALSE;
}

// DISHONORED(port): the Continue entry of the main menu bar. Retail's own reading of "can this player
// continue" is in UDisGFxMoviePlayerMainMenu::PostStart (2012 0x821e00), which computes the four
// booleans it hands to mainMenu_mc.Open as `!engine || UDishonoredEngine::HasSaveGame(engine, 0)` for
// both Continue and Load - the same question Req_CanLoadGame answers. Without it the AS2 side got a
// zeroed result from the unported-native handler and MainMenu.SetMenu built a menu with no entries at
// all, so the bar came up empty.
void UDisGFxMoviePlayerMenuBase::execReq_CanContinueGame( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	*(UBOOL*)Result = ( Engine == NULL || Engine->HasSaveGame( 0 ) ) ? TRUE : FALSE;
}

// DISHONORED(port): the Save entry of the pause menu's bar, the same pair of questions: saving is
// offered when the engine allows save/load at all (0x62bb50).
void UDisGFxMoviePlayerMenuBase::execReq_CanSaveGame( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	*(UBOOL*)Result = ( Engine && Engine->m_bSaveLoadEnabled ) ? TRUE : FALSE;
}

// DISHONORED(port): 2012 rva 0x5f7920 exec / 0x62bb50 body - the engine's own m_bSaveLoadEnabled bit, which
// is what -newgame and the streamed menu teardown both key off
void UDisGFxMoviePlayerMenuBase::execReq_IsSaveLoadEnabled( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	*(UBOOL*)Result = ( Engine && Engine->m_bSaveLoadEnabled ) ? TRUE : FALSE;
}

// DISHONORED(port): 2012 rva 0x8103b0 (TryBindKey). Called while the key-binding screen is listening: the
// pressed key is resolved to a bindable action, written through OnSettingChange, and then the whole keyboard
// mapping is rebuilt and handed back to AS2 through OnKeyAssigned. Escape cancels without binding.
// The 29..62 range is the bindable-action span of retail's setting-id space.
void DisTryBindKey( UDisGFxMoviePlayerMenuBase* Menu, FName KeyName )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	if( KeyName != KEY_Escape )
	{
		// DISHONORED(bringup): UArkProfileSettings::FindBindableKey(KeyName) decides whether the key is
		// bindable at all and which BK_* value it is; unavailable, so nothing is written and the screen
		// simply redraws the current mapping.
	}

	GFxValue KeyboardMapping;
	View->CreateArray( &KeyboardMapping );
	for( INT SettingID = 29; SettingID < 63; SettingID++ )
	{
		FDisSetting Setting(EC_EventParm);
		Setting.m_SettingID = SettingID;
		GFxValue ActionBinding;
		DisCreateGFxSetting( Menu, Setting, &ActionBinding );
		KeyboardMapping.PushBack( ActionBinding );
		ActionBinding.ReleaseManaged();
	}

	GFxValue Options;
	if( View->GetVariable( &Options, "_root.optionsMenu_mc" ) )
	{
		GFxValue Unused;
		Options.Invoke( "OnKeyAssigned", &Unused, &KeyboardMapping, 1 );
		Unused.ReleaseManaged();
	}
	Options.ReleaseManaged();
	KeyboardMapping.ReleaseManaged();
	Menu->m_ActionBindingID = 0;
}

/** the two menu refreshes the main menu and the pause menu both drive; retail reaches them through
    RefreshLoadGameMenu (2012 0x815940) and RefreshSettingsCategory (0x817400) */
void DisRefreshLoadGameMenu( UDisGFxMoviePlayerMenuBase* Menu )
{
	DisFillLoadGameMenu( Menu );
	Menu->m_bLoadGameListDirty = FALSE;
}

void DisRefreshSettingsCategoryList( UDisGFxMoviePlayerMenuBase* Menu )
{
	DisShowSettingsCategoryList( Menu );
}
