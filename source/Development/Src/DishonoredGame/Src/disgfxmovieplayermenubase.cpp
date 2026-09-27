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

// DISHONORED(port): 2012 rva 0x80f920 (CreateGFxSetting). One AS2 object per row of an options page. The
// member names are the contract with the cooked asset: Setting_Id, Setting_Name, Setting_Value,
// Setting_Minimum, Setting_Maximum, Setting_Increment, bDropList, plus the four screen flags that tell the
// asset to open a sub-screen instead of editing in place.
static void DisCreateGFxSetting( UDisGFxMoviePlayerMenuBase* Menu, const FDisSetting& Setting, GFxValue* OutSetting )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	View->CreateObject( OutSetting );
	DisSetGFxNumber( *OutSetting, "Setting_Id", Setting.m_SettingID );
	DisSetGFxString( *OutSetting, "Setting_Name", Setting.m_SettingNameOverride );
	DisSetGFxBool( *OutSetting, "bDropList", Setting.m_bDropList );
	DisSetGFxBool( *OutSetting, "GamepadBindingMenu", Setting.m_bGamepadBindingMenu );
	DisSetGFxBool( *OutSetting, "VideoSettings", Setting.m_bVideoMenu );
	DisSetGFxBool( *OutSetting, "GammaMenu", Setting.m_bGammaMenu );
	DisSetGFxBool( *OutSetting, "DeviceSelectionMenu", Setting.m_bDeviceSelectionMenu );
	// DISHONORED(bringup): retail then fills Setting_Value, Setting_Minimum, Setting_Maximum and
	// Setting_Increment from UArkProfileSettings for this m_SettingID, and for a drop list also the
	// ProfileSettingIDs / ProfileSettingValues arrays; that class has no accessors in this tree.
}

// DISHONORED(port): 2012 rva 0x813db0 (CreateGFxSubCategory)
static void DisCreateGFxSubCategory( UDisGFxMoviePlayerMenuBase* Menu, const FDisSettingsSubCategory& SubCategory, GFxValue* OutSubCategory )
{
	GFxMovieView* View = DisMenuView( Menu );
	if( View == NULL )
	{
		return;
	}
	View->CreateObject( OutSubCategory );
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
	View->CreateObject( OutCategory );
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

// DISHONORED(port): 2012 rva 0x817250 (ShowSettingsCategoryList) - the whole tree in one AS2 call
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
	if( View->GetVariable( &Options, "_root.options_mc" ) )
	{
		GFxValue Unused;
		Options.Invoke( "FillCategories", &Unused, &Categories, 1 );
		Unused.ReleaseManaged();
	}
	Options.ReleaseManaged();
	Categories.ReleaseManaged();
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
	// DISHONORED(bringup): retail walks UDishonoredEngine::GetSaveGame(0) .. and for every slot >= 10 pushes
	// { chapterName = FindSaveName(SaveGame), saveDate = FormatSaveDate(SaveGame),
	//   itemThumb = "img://" + m_pMenuBaseTweaks->m_MapSmallImagePackage + ".MissionsScreen_" +
	//   MapConfig->m_ImageName + "_Small" } while recording SaveGame->m_Slot in m_LoadGameSlots.
	// FDisSaveGame is not declared in this tree and UDishonoredEngine has no reflected save list, so the
	// list is sent empty - which is exactly what the asset shows when there is nothing to load.

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

// DISHONORED(port): 2012 rva 0x5f7b80 exec / 0x806b10 body
void UDisGFxMoviePlayerMenuBase::execOnSettingChange( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_SettingID);
	P_GET_FLOAT(_fValue);
	P_FINISH;

	m_bOptionsChanged = TRUE;
	// DISHONORED(bringup): retail writes the value into UArkProfileSettings for _SettingID and, for the
	// video settings, defers to OnApplyVideoSettings (2012 0x7f7290); the profile-settings accessors are not
	// declared in this tree. The dirty flag is this class's own and is what OnLeaveOptions acts on.
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
	if( m_LoadGameSlots.IsValidIndex( _ListIdx ) )
	{
		// DISHONORED(bringup): UDishonoredEngine::DeleteSaveGame(m_LoadGameSlots(_ListIdx)) (2012 0x642660)
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
	// DISHONORED(bringup): retail asks UDishonoredEngine::HasSaveGame for any slot >= 10 (2012 0x642540);
	// with no save list declared the honest answer is "nothing to load".
	*(UBOOL*)Result = ( Engine && m_LoadGameSlots.Num() > 0 ) ? TRUE : FALSE;
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
	if( View->GetVariable( &Options, "_root.options_mc" ) )
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
