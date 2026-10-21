// DishonoredGame/src/disgfxmovieplayerbase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x7f41a0  public: static void __cdecl UDisGFxMoviePlayerBase::InitializePrivateStaticClassUDisGFxMoviePlayerBase(void)
//   0x7f41c0  public: static void __cdecl UDisUISoundTheme::InitializePrivateStaticClassUDisUISoundTheme(void)
//   0x7f41e0  public: static void __cdecl UDisTweaks_GFxMoviePlayerBase::InitializePrivateStaticClassUDisTweaks_GFxMoviePlayerBase(void)
//   0x7f4200  public: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::Start(unsigned int)
//   0x7f4290  public: virtual void __thiscall UDisGFxMoviePlayerBase::CaptureAnalogInput(unsigned int)
//   0x7f42b0  public: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::HasFinishedAsyncLoading(void)
//   0x7f42c0  public: void __thiscall UDisGFxMoviePlayerBase::AddMessageBoxTimer(float)
//   0x7f4310  public: void __thiscall UDisGFxMoviePlayerBase::HideMessageBox(void)
//   0x7f4340  public: void __thiscall UDisGFxMoviePlayerBase::AllowFocus(unsigned int)
//   0x7f4390  public: void __thiscall UDisGFxMoviePlayerBase::ComputeMovieSpaceInfo(int, int, struct FDisMovieSpaceInfo &)
//   0x7f4460  protected: static void __cdecl UDisGFxMoviePlayerBase::StaticOnCompleteMoviePackageLoading(class UObject *, void *)
//   0x7f4480  protected: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::FilterInputAxis(int, class FName, float, float, unsigned int, unsigned int &)
//   0x7f45a0  protected: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::FilterButtonInput(int, class FName, unsigned char, unsigned int &)
//   0x7f5ce0  public: virtual void __thiscall UDisGFxMoviePlayerBase::OnFocusGained(int)
//   0x7f5dc0  public: virtual void __thiscall UDisGFxMoviePlayerBase::OnFocusLost(int)
//   0x7f5eb0  protected: virtual void __thiscall UDisGFxMoviePlayerBase::OnCompleteMoviePackageLoading(void)
//   0x7f5f30  protected: static void __cdecl UDisGFxMoviePlayerBase::FormatGamepadKeyName(wchar_t const *, class FString &)
//   0x7f5fd0  protected: static void __cdecl UDisGFxMoviePlayerBase::FormatMouseKeyName(wchar_t const *, class FString &)
//   0x7f6030  protected: virtual void __thiscall UDisGFxMoviePlayerBase::SetTweaks_Derived(class UDisTweaksBase *)
//   0x7f6040  protected: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::IsUsingGamepad(void)const
//   0x7fa080  public: virtual void __thiscall UDisGFxMoviePlayerBase::Close(unsigned int)
//   0x7fa1b0  public: class FString __thiscall UDisGFxMoviePlayerBase::Req_EquipmentIconImage(int)
//   0x7fa200  public: virtual void __thiscall UDisGFxMoviePlayerBase::OnMessageBoxResult(class FArkGameEvent const &)
//   0x7fa350  protected: static class FString __cdecl UDisGFxMoviePlayerBase::LocalizeKeyboardKeyName(class FName, unsigned int)
//   0x7fa4c0  private: void __thiscall UDisGFxMoviePlayerBase::UpdateGamepadUseForAS(unsigned int)
//   0x802bd0  public: virtual void __thiscall UDisGFxMoviePlayerBase::Advance(float)
//   0x808490  public: void __thiscall UDisGFxMoviePlayerBase::ShowMessageBox(class FString const &, class FString const &, class FString const &, class FString const &)
//   0x808550  protected: virtual void __thiscall UDisGFxMoviePlayerBase::BeginDestroy(void)
//   0x8085b0  protected: static void __cdecl UDisGFxMoviePlayerBase::FormatInteractionText(class FString &)
//   0x808a20  protected: virtual void __thiscall UDisGFxMoviePlayerBase::ApplyTweakChanges_Derived(void)
//   0x80bb90  public: void __thiscall UDisGFxMoviePlayerBase::FormatText(class FString const &)
//   0x817fa0  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerBase::GetPrivateStaticClassUDisTweaks_GFxMoviePlayerBase(wchar_t const *)
//   0x81ac80  public: static class UClass * __cdecl UDisUISoundTheme::GetPrivateStaticClassUDisUISoundTheme(wchar_t const *)
//   0x81ad10  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerBase::StaticClassNoInline(void)
//   0x81b540  public: static class UClass * __cdecl UDisUISoundTheme::StaticClassNoInline(void)
//   0x81d0c0  public: unsigned int __thiscall UDisGFxMoviePlayerBase::LoadMoviePackage(wchar_t const *)
//   0x81d320  public: unsigned int __thiscall UDisGFxMoviePlayerBase::LoadMoviePackageAsync(wchar_t const *)
//   0x81d5b0  protected: virtual void __thiscall UDisGFxMoviePlayerBase::UpdateAnalogInputForAS(float)
//   0x81eb60  public: static class UClass * __cdecl UDisGFxMoviePlayerBase::GetPrivateStaticClassUDisGFxMoviePlayerBase(wchar_t const *)
//   0x820d50  public: static class UClass * __cdecl UDisGFxMoviePlayerBase::StaticClassNoInline(void)
//   0x8218a0  private: void __thiscall UDisGFxMoviePlayerBase::InitTexts(void)
//   0x822820  public: virtual unsigned int __thiscall UDisGFxMoviePlayerBase::PreLoad(void)

#include "DishonoredGame.h"

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5f6a50; the retail UDisGFxMoviePlayerBase vtable slot +440 it dispatches to is `return FALSE`
// (2013 rva 0x6c0020) and no retail subclass overrides it
void UDisGFxMoviePlayerBase::execWidgetInitialized_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(_WidgetName);
	P_GET_NAME(_WidgetPath);
	P_GET_OBJECT(UGFxObject, _pWidget);
	P_FINISH;
	*(UBOOL*)Result = FALSE;
}

// ---- end of trivial natives ----

/*-----------------------------------------------------------------------------
	Agent BE (PHASE8.md package BE): the 8 remaining UDisGFxMoviePlayerBase natives.
	These are the AS2 -> C++ boundary of every Dishonored menu screen, and they
	arrive through FGFxExternalInterface::Callback (GFxUI/Src/gfxuiexternalinterface.cpp).
	The three message-box ones are honest halves: the m_MsgBoxID bookkeeping is
	this class's and is ported, and each names the UDisGlobalUIManager method
	that is not declared in this tree yet.
-----------------------------------------------------------------------------*/

#include "gfxui_gfx3.h"
#include "dishonoredutilities.h"
#include "arkgameeventdispatcher.h"

// DISHONORED(port): 2012 rva 0x5f5750 exec / 0x7f5ce0 body. Focus is handed to the movie's own AS2 root
// object, not to a widget: _root.UIBase.OnFocusGained(). Only while the movie is open.
void UDisGFxMoviePlayerBase::execOnFocusGained( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_LocalPlayerIndex);
	P_FINISH;

	FGFxMovie* Movie = GetMovie();
	if( !bMovieIsOpen || Movie == NULL || Movie->pView.GetPtr() == NULL )
	{
		return;
	}
	GFxValue UIBase;
	if( Movie->pView->GetVariable( &UIBase, "_root.UIBase" ) )
	{
		GFxValue Unused;
		UIBase.Invoke( "OnFocusGained", &Unused );
		Unused.ReleaseManaged();
	}
	UIBase.ReleaseManaged();
}

// DISHONORED(port): 2012 rva 0x7f4290 - the m_bCaptureAnalogInput bit, read back by UpdateAnalogInputForAS
void UDisGFxMoviePlayerBase::execCaptureAnalogInput( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bCapture);
	P_FINISH;
	m_bCaptureAnalogInput = _bCapture ? TRUE : FALSE;
}

// DISHONORED(port): 2012 rva 0x7f42b0 - the negation of the m_bIsLoadingMoviePackage bit
void UDisGFxMoviePlayerBase::execHasFinishedAsyncLoading( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = HasFinishedAsyncLoading();
}

// DISHONORED(port): agent FP. The four script natives DisGFxMoviePlayerBase redeclares over GFxMoviePlayer's
// (`native function bool Start(optional bool)` and its three siblings) were DISHONORED_NATIVE_STUBs, which is why
// the Kismet open of the main menu answered FALSE: UGFxAction_OpenMovie::Activated reaches the player through
// eventStart, and the script's own Start bound to a stub that consumed the parameter and zeroed the result.
// Neither symbol table names a body for them - they are the same four lines as UGFxMoviePlayer's execStart /
// execClose / execAdvance / execPreLoad (gfxuimovie.cpp) and fold onto them - so these are those bodies, through
// the virtuals, which is all the retail ones can be.
void UDisGFxMoviePlayerBase::execStart( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bStartPaused,FALSE);
	P_FINISH;
	*(UBOOL*)Result = Start( _bStartPaused );
}

void UDisGFxMoviePlayerBase::execClose( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bUnload,TRUE);
	P_FINISH;
	Close( _bUnload );
}

void UDisGFxMoviePlayerBase::execAdvance( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(_fTime);
	P_FINISH;
	Advance( _fTime );
}

void UDisGFxMoviePlayerBase::execPreLoad( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = PreLoad();
}

UDisTweaksBase* UDisGFxMoviePlayerBase::GetTweaks_Derived()
{
	return m_pTweaks;
}

void UDisGFxMoviePlayerBase::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pTweaks = (UDisTweaks_GFxMoviePlayerBase*)Tweaks;
}

// DISHONORED(port): agent JB, 2012 rva 0x808a20. The movie's sound themes are the tweaks' bindings: every theme name
// binds its Theme and, when there is one, its ExternalTheme (both entries under the same name, in that order).
void UDisGFxMoviePlayerBase::ApplyTweakChanges_Derived()
{
	SoundThemes.Empty();
	for( INT Index = 0; Index < m_pTweaks->m_SoundThemes.Num(); Index++ )
	{
		const FDisSoundThemeBinding& Binding = m_pTweaks->m_SoundThemes(Index);
		FSoundThemeBinding SoundThemeBinding;
		SoundThemeBinding.ThemeName = Binding.ThemeName;
		if( Binding.Theme != NULL )
		{
			SoundThemeBinding.Theme = Binding.Theme;
			SoundThemes.AddItem( SoundThemeBinding );
		}
		if( Binding.ExternalTheme != NULL )
		{
			SoundThemeBinding.Theme = Binding.ExternalTheme;
			SoundThemes.AddItem( SoundThemeBinding );
		}
	}
	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("jbdiag") );
	if( bDiag )
	{
		FString Themes;
		for( INT Index = 0; Index < SoundThemes.Num(); Index++ )
		{
			Themes += FString::Printf( TEXT(" %s=%s"), *SoundThemes(Index).ThemeName.ToString(), *SoundThemes(Index).Theme->GetPathName() );
		}
		debugf( TEXT("jbdiag: ApplyTweakChanges_Derived %s tweaks %s: %d sound themes%s"), *GetName(), *m_pTweaks->GetPathName(),
			SoundThemes.Num(), *Themes );
	}
}

// DISHONORED(port): agent FP, 2013 rva 0x7a3fd0 (2012 0x81d0c0). Nothing to do while a movie is already held or a
// package is streaming in. Without a path the tweaks name the package and the movie. Retail brackets LoadPackage with
// the streaming manager's BlockTillAllRequestsFinished and CancelPendingTextureStreaming, which this tree's streaming
// manager does not expose; the movie is then found by its path, rooted, and the path remembered so Close(TRUE) can
// release it (m_bReleaseMoviePackageOnClose).
UBOOL UDisGFxMoviePlayerBase::LoadMoviePackage( const TCHAR* _pMoviePath )
{
	if( MovieInfo != NULL || m_bIsLoadingMoviePackage )
	{
		return FALSE;
	}
	UDisTweaks_GFxMoviePlayerBase* Tweaks = m_pTweaks != NULL ? m_pTweaks
		: (UDisTweaks_GFxMoviePlayerBase*)UDisTweaks_GFxMoviePlayerBase::StaticClass()->GetDefaultObject();
	FString MoviePath;
	FString PackageName;
	if( _pMoviePath != NULL && *_pMoviePath != 0 )
	{
		MoviePath = _pMoviePath;
		const INT Dot = MoviePath.InStr( TEXT(".") );
		PackageName = Dot >= 0 ? MoviePath.Left( Dot ) : MoviePath;
	}
	else
	{
		MoviePath = Tweaks->m_MoviePackageName + TEXT(".") + Tweaks->m_MovieName;
		PackageName = Tweaks->m_MoviePackageName;
	}
	FString FileName = PackageName;
	if( GUseSeekFreeLoading )
	{
		FileName += TEXT("_SF");
	}
	FString Unused;
	if( !GPackageFileCache->FindPackageFile( *FileName, NULL, Unused ) )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s::LoadMoviePackage: no package file %s for %s"),
			*GetClass()->GetName(), *FileName, *MoviePath );
		return FALSE;
	}
	if( UObject::LoadPackage( NULL, *FileName, LOAD_None ) == NULL )
	{
		return FALSE;
	}
	USwfMovie* Movie = FindObject<USwfMovie>( NULL, *MoviePath );
	if( Movie == NULL )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s::LoadMoviePackage: %s loaded and holds no SwfMovie %s"),
			*GetClass()->GetName(), *FileName, *MoviePath );
		return FALSE;
	}
	MovieInfo = Movie;
	Movie->AddToRoot();
	m_DynamicallyLoadedMoviePath = MoviePath;
	return TRUE;
}

// DISHONORED(port): execFormatText 2013 rva 0x600aa0 -> FormatText 2013 0x7a9e20 (2012 0x647be0 -> 0x80bb90). FormatText reads a
// text field's own string back out of the movie, runs FormatInteractionText( text, FALSE, FALSE ) over it (agent HJ: the
// substitution HF ported) and writes it back sticky, which is how a `GBA_<action>` token in a localised string becomes the key
// the player has bound. A variable that does not read back as a string is written back as the empty string, as retail does.
void UDisGFxMoviePlayerBase::execFormatText( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rTextVarPath);
	P_FINISH;

	FGFxMovie* Movie = GetMovie();
	if( Movie == NULL || Movie->pView.GetPtr() == NULL )
	{
		return;
	}
	FString Formatted;
	GFxValue Text( GFxValue::VT_ConvertStringW );
	if( Movie->pView->GetVariable( &Text, FTCHARToUTF8(*_rTextVarPath) ) && !Text.IsUndefined() )
	{
		Formatted = Text.GetType() == GFxValue::VT_StringW
			? FString( Text.GetStringW() )
			: FString( FUTF8ToTCHAR( Text.GetString() ) );
		FormatInteractionText( Formatted, FALSE, FALSE );
	}
	Text.ReleaseManaged();
	GFxValue Out;
	Out.SetStringW( *Formatted );
	Movie->pView->SetVariable( FTCHARToUTF8(*_rTextVarPath), Out, GFxMovie::SV_Sticky );
}

// DISHONORED(port): 2013 rva 0x78c590 (2012 0x7f5f30, same bytes): the movie's own glyph token, "$<key>$" with the
// direction words and the device prefixes shortened
void UDisGFxMoviePlayerBase::FormatGamepadKeyName( const TCHAR* _pKeyName, FString& _rOut )
{
	_rOut = TEXT("$");
	_rOut += _pKeyName;
	_rOut = _rOut.Replace( TEXT("Up"), TEXT("U") );
	_rOut = _rOut.Replace( TEXT("Down"), TEXT("D") );
	_rOut = _rOut.Replace( TEXT("Left"), TEXT("L") );
	_rOut = _rOut.Replace( TEXT("Right"), TEXT("R") );
	_rOut = _rOut.Replace( TEXT("XboxTypeS_"), TEXT("") );
	_rOut = _rOut.Replace( TEXT("Gamepad_"), TEXT("") );
	_rOut += TEXT("$");
}

// DISHONORED(port): 2013 rva 0x78c630. The three mouse buttons' glyph tokens; left and right follow the system's swap.
void UDisGFxMoviePlayerBase::FormatMouseKeyName( FName _Key, FString& _rOut )
{
	if( _Key == KEY_LeftMouseButton || _Key == KEY_RightMouseButton )
	{
		const UBOOL bSwapped = GetSystemMetrics( SM_SWAPBUTTON ) != 0;
		const UBOOL bLeft = _Key == KEY_LeftMouseButton;
		_rOut = bSwapped != bLeft ? TEXT("$LMouseButton$") : TEXT("$RMouseButton$");
	}
	else if( _Key == KEY_MiddleMouseButton )
	{
		_rOut = TEXT("$MMouseButton$");
	}
}

// DISHONORED(port): 2013 rva 0x793dc0 (2012 0x7fa350). Settings.int's [Keys] name of a keyboard key; the seven OEM keys
// whose character depends on the layout are read back from the active keyboard layout instead, the mouse buttons
// follow the system's swap, and no key at all is [Keys] Unbound_Game or Unbound_Menu.
FString UDisGFxMoviePlayerBase::LocalizeKeyboardKeyName( FName _Key, UBOOL _bInGame )
{
	if( _Key == NAME_None )
	{
		return Localize( TEXT("Keys"), _bInGame ? TEXT("Unbound_Game") : TEXT("Unbound_Menu"), TEXT("Settings") );
	}
	if( ( _Key == KEY_LeftMouseButton || _Key == KEY_RightMouseButton ) && GetSystemMetrics( SM_SWAPBUTTON ) )
	{
		_Key = _Key == KEY_LeftMouseButton ? KEY_RightMouseButton : KEY_LeftMouseButton;
	}
	struct FDisLayoutKey { FName* Key; UINT VirtualKey; };
	const FDisLayoutKey LayoutKeys[] =
	{
		{ &KEY_Semicolon, 0xBA }, { &KEY_Slash, 0xBF }, { &KEY_Tilde, 0xC0 }, { &KEY_LeftBracket, 0xDB },
		{ &KEY_Backslash, 0xDC }, { &KEY_RightBracket, 0xDD }, { &KEY_Quote, 0xDE },
	};
	HKL Layout = GetKeyboardLayout( 0 );
	for( INT Index = 0; Index < ARRAY_COUNT(LayoutKeys); Index++ )
	{
		if( _Key == *LayoutKeys[Index].Key )
		{
			const TCHAR Character[2] = { (TCHAR)MapVirtualKeyExW( LayoutKeys[Index].VirtualKey, MAPVK_VK_TO_CHAR, Layout ), 0 };
			return FString( Character );
		}
	}
	return Localize( TEXT("Keys"), *_Key.ToString(), TEXT("Settings") );
}

// DISHONORED(port): 2013 rva 0x7a4620 (2012 0x8085b0). Every `GBA_<action>`, `KEY_<key>` and `k (the use action) in the text
// becomes the glyph or bracketed key name bound to it: a pad key or KEY_ through FormatGamepadKeyName, a mouse button
// through FormatMouseKeyName, any other key as "[<LocalizeKeyboardKeyName>]", and an action with no entry in the
// player input's m_GameActionBindings as "UNKNOWN GBA". ReplaceInline replaces every occurrence of the token, and the
// scan resumes past the replacement.
void UDisGFxMoviePlayerBase::FormatInteractionText( FString& _rText, UBOOL _bForcePC, UBOOL _bForceGamepad )
{
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	UDishonoredPlayerInput* Input = PC ? Cast<UDishonoredPlayerInput>( PC->PlayerInput ) : NULL;
	INT Start = _rText.InStr( TEXT("`") );
	while( Start != -1 )
	{
		const TCHAR* At = *_rText + Start;
		const UBOOL bAction = appStrncmp( At, TEXT("`GBA_"), 5 ) == 0;
		const UBOOL bKey = appStrncmp( At, TEXT("`KEY_"), 5 ) == 0;
		FString Param;
		FString ParamValue;
		FString Replacement;
		UBOOL bResolve = TRUE;
		if( bAction || bKey )
		{
			const INT End = _rText.InStr( TEXT("`"), FALSE, FALSE, Start + 5 );
			if( End == -1 )
			{
				break;
			}
			Param = _rText.Mid( Start, End - Start + 1 );
			ParamValue = _rText.Mid( Start + 1, End - Start - 1 );
			if( bKey )
			{
				FormatGamepadKeyName( *ParamValue + 4, Replacement );
				bResolve = FALSE;
			}
		}
		else if( appStrncmp( At, TEXT("`k"), 2 ) == 0 )
		{
			Param = TEXT("`k");
			ParamValue = TEXT("GBA_Use");
		}
		else
		{
			bResolve = FALSE;
		}
		if( bResolve )
		{
			const FKeyBind* Bind = Input ? Input->GetKeyBindFromAction( FName( *ParamValue ), _bForcePC, _bForceGamepad ) : NULL;
			if( Bind == NULL )
			{
				Replacement = TEXT("UNKNOWN GBA");
			}
			else if( Input->IsGamepadKey( Bind->Name ) )
			{
				FormatGamepadKeyName( *Bind->Name.ToString(), Replacement );
			}
			else if( Bind->Name == KEY_LeftMouseButton || Bind->Name == KEY_RightMouseButton || Bind->Name == KEY_MiddleMouseButton )
			{
				FormatMouseKeyName( Bind->Name, Replacement );
			}
			else
			{
				Replacement = FString( TEXT("[") ) + LocalizeKeyboardKeyName( Bind->Name, TRUE ) + TEXT("]");
			}
		}
		if( Replacement.Len() > 0 )
		{
			_rText = _rText.Replace( *Param, *Replacement );
		}
		Start = _rText.InStr( TEXT("`"), FALSE, FALSE, Start + Max( Replacement.Len(), 1 ) );
	}
}

// DISHONORED(port): 2013 rva 0x600b40 (2012 0x647c80) - the exec of the AS2 'ShowMessageBox' call
void UDisGFxMoviePlayerBase::execShowMessageBox( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rMessage);
	P_GET_STR(_rButton0);
	P_GET_STR(_rButton1);
	P_GET_STR(_rButton2);
	P_FINISH;
	ShowMessageBox( _rMessage, _rButton0, _rButton1, _rButton2 );
}

// DISHONORED(port): 2013 rva 0x5f6b70
void UDisGFxMoviePlayerBase::execHideMessageBox( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	HideMessageBox();
}

// DISHONORED(port): 2013 rva 0x5f6b10
void UDisGFxMoviePlayerBase::execAddMessageBoxTimer( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(_fDuration);
	P_FINISH;
	AddMessageBoxTimer( _fDuration );
}

// DISHONORED(port): 2012 rva 0x7fa1b0 - one string out of UDisGlobalUIManager::m_EquipmentIcons[_ItemIdx]
void UDisGFxMoviePlayerBase::execReq_EquipmentIconImage( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_ItemIdx);
	P_FINISH;
	// DISHONORED(bringup): DisGetGlobalUIManager()->m_EquipmentIcons[_ItemIdx]; UDisGlobalUIManager has no
	// reflected m_EquipmentIcons array in this tree's generated class.
	*(FString*)Result = FString();
}



/*-----------------------------------------------------------------------------
	Agent EI (PHASE12 package EI): the asking half of the message box - the id, the timer and the game event that
	carries the player's answer back into the movie that asked.

	Retail's UDisGFxMoviePlayerBase::ShowMessageBox (2013 0x7a4550) does three things and this does the same three:
	it asks the global UI for a box, it keeps the id it is given, and it subscribes to
	DisGameEventType_MessageBoxResult - unregister first, then register, so a movie that raises a second box is
	still on the list exactly once. BeginDestroy (0x7a45c0) drops the subscription with the object.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x7a4550
void UDisGFxMoviePlayerBase::ShowMessageBox( const FString& _rMessage, const FString& _rButton0,
	const FString& _rButton1, const FString& _rButton2 )
{
	FDisMsgBoxInfo Info(EC_EventParm);
	Info.m_Message = _rMessage;
	Info.m_Buttons[0] = _rButton0;
	Info.m_Buttons[1] = _rButton1;
	Info.m_Buttons[2] = _rButton2;

	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( UIManager != NULL )
	{
		m_MsgBoxID = UIManager->ShowMessageBox( Info, 0 );
	}
	else
	{
		// DISHONORED(bringup): no UDisGlobalUIManager is constructed on the -gfxuimenu path, so the manager's own
		// one-line forwarder is taken here instead. DisGetGlobalMoviePlayer is the seam and the only one.
		m_MsgBoxID = 0;
		UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
		if( Global != NULL )
		{
			Global->AddMessageBox( Info, m_MsgBoxID, 0 );
		}
	}

	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher != NULL )
	{
		const INT EventType = DisGameEventType_MessageBoxResult;
		Dispatcher->UnregisterToEvent( EventType, this, &UDisGFxMoviePlayerBase::OnMessageBoxResult );
		Dispatcher->RegisterToEvent( EventType, this, &UDisGFxMoviePlayerBase::OnMessageBoxResult );
	}
}

// DISHONORED(port): 2013 rva 0x787780
void UDisGFxMoviePlayerBase::HideMessageBox()
{
	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( UIManager != NULL )
	{
		UIManager->HideMessageBox( m_MsgBoxID );
	}
	else
	{
		UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
		if( Global != NULL )
		{
			Global->RemoveMessageBox( m_MsgBoxID );
		}
	}
	m_MsgBoxID = 0;
}

// DISHONORED(port): 2013 rva 0x787730 - a duration of zero means ten seconds, which is retail's own default
void UDisGFxMoviePlayerBase::AddMessageBoxTimer( FLOAT _fDuration )
{
	const FLOAT Duration = _fDuration == 0.f ? 10.f : _fDuration;
	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( UIManager != NULL )
	{
		UIManager->AddMessageBoxTimer( m_MsgBoxID, Duration );
	}
	else
	{
		UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
		if( Global != NULL )
		{
			Global->AddMessageBoxTimer( m_MsgBoxID, Duration );
		}
	}
}

// DISHONORED(port): 2013 rva 0x793c70. The event carries the box's id and the button the player chose. Only the
// movie whose id it is answers, the id is cleared before the content runs, and the content's own
// _common.MessageBoxInvoke.OnMessageBoxClosed(<button>) calls back whatever callback InvokeMessageBox stored - for
// the New Game confirmation that is NewGameMenu.OnNewGameConfirm, which closes the screen and then asks the game
// for a mission through ExternalInterface 'OnNewGameConfirm'.
void UDisGFxMoviePlayerBase::OnMessageBoxResult( const FArkGameEvent& _rEvent )
{
	const FDisMsgBoxResult* Result = (const FDisMsgBoxResult*)_rEvent.m_pEventParams;
	if( Result == NULL || Result->m_ID != m_MsgBoxID )
	{
		return;
	}
	const UBOOL bWasOpen = bMovieIsOpen;
	m_MsgBoxID = 0;
	FGFxMovie* Movie = GetMovie();
	if( !bWasOpen || Movie == NULL || Movie->pView.GetPtr() == NULL )
	{
		return;
	}
	GFxValue Invoke;
	const UBOOL bFound = Movie->pView->GetVariable( &Invoke, "_root.MessageBoxInvoke" );
	debugf( TEXT("DISHONORED(bringup): message box %d result (button %d) delivered to %s: _root.MessageBoxInvoke %s"),
		Result->m_ID, Result->m_SelectedIndex, *GetName(), bFound ? TEXT("found") : TEXT("MISSING") );
	if( bFound )
	{
		GFxValue Selected;
		Selected.SetNumber( (DOUBLE)Result->m_SelectedIndex );
		GFxValue Unused;
		Invoke.Invoke( "OnMessageBoxClosed", &Unused, &Selected, 1 );
		Unused.ReleaseManaged();
		Selected.ReleaseManaged();
	}
	Invoke.ReleaseManaged();
}

// DISHONORED(port): 2013 rva 0x7877b0. Gaining focus flushes what the player pressed while it did not have it, so
// the key that raised the box is not delivered to the box as well.
void UDisGFxMoviePlayerBase::AllowFocus( UBOOL _bAllow )
{
	if( bMovieIsOpen && !bAllowFocus && _bAllow )
	{
		FlushPlayerInput( FALSE );
	}
	bAllowFocus = _bAllow ? TRUE : FALSE;
	SetMovieCanReceiveFocus( _bAllow );
	if( bMovieIsOpen && FGFxEngine::GetEngine() != NULL )
	{
		FGFxEngine::GetEngine()->ReevaluateFocus();
	}
	// DISHONORED(bringup): retail also tells UIManager->OnMovieAttributesChanged (2013 0x84cf80), which is not
	// declared in this tree.
}

// DISHONORED(port): 2013 rva 0x79e820
void UDisGFxMoviePlayerBase::AllowInput( UBOOL _bAllowInput, UBOOL _bCaptureInput )
{
	if( bMovieIsOpen && !bCaptureInput && _bCaptureInput )
	{
		FlushPlayerInput( FALSE );
	}
	bAllowInput = _bAllowInput ? TRUE : FALSE;
	bCaptureInput = _bCaptureInput ? TRUE : FALSE;
	SetMovieCanReceiveInput( _bAllowInput );
}

// DISHONORED(port): 2013 rva 0x7a45c0 - the subscription must not outlive the object the dispatcher would call
void UDisGFxMoviePlayerBase::BeginDestroy()
{
	Super::BeginDestroy();
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher != NULL )
	{
		const INT EventType = DisGameEventType_MessageBoxResult;
		Dispatcher->UnregisterToEvent( EventType, this, &UDisGFxMoviePlayerBase::OnMessageBoxResult );
	}
}

// DISHONORED(bringup, agent DC): the -gfxuimenu bring-up switch that opens a cooked menu movie through
// the real path moved to GFxUI/Src/gfxuiengine.cpp (FGFxEngine's per-frame tick). It started here, as an
// FTickableObject, and that does not run on a menu map: UWorld::Tick walks
// FTickableObject::TickableObjects only when its TickType is not LEVELTICK_TimeOnly and the world is not
// paused (Engine/Src/UnLevTic.cpp:3397), and a streamed-in menu map with no player controller ticks
// time only. The engine's own per-frame call has no such condition. Why the switch exists at all is in
// agentDC.md: UDisGlobalUIManager's config movie set does not name the main menu, the game's own
// UnrealScript constructs it, and none of that chain runs in this build.
// ------ DG tail ------


/*-----------------------------------------------------------------------------
	InitTexts: where every string in the interface comes from. Agent DG.

	Agent DC's report said the menu reads "Text" everywhere because retail fills the fields through
	GFxTranslator / GFxFontMap. Measured, it does not. UDisGFxMoviePlayerBase::PreLoad (2012
	0x822820) calls InitTexts (0x8218a0), which:

	  1. finds the localisation file that holds a [DisGFxMoviePlayerBase_Texts] section for the
	     current language (retail's FindLocalizedConfigSection, which caches the path and the
	     language it was found for in two file-scope statics),
	  2. creates one AS2 object and sticks it on _root.texts,
	  3. walks the class chain from the concrete movie player up to UDisGFxMoviePlayerBase and, for
	     each class, copies every key of its own [<ClassName>_Texts] section to
	     _root.texts.<Key> as a sticky variable.

	The asset then reads _root.texts.t_NewGame and its kin. DishonoredGame.int carries
	DisGFxMoviePlayerMainMenu_Texts with t_PressStart, t_NewGame, t_Missions and the difficulty
	descriptions, which is exactly what the main menu shows.
-----------------------------------------------------------------------------*/

/** 2012 0x821730 (FindLocalizedConfigSection): the first localisation file that has this section,
    searched the way UObject::LoadLocalizedDynamicArray searches - the current language first, then
    INT, over GSys->LocalizationPaths from the back. */
static UBOOL DisFindLocalizedConfigFile( const TCHAR* Section, FString& OutPath )
{
	const TCHAR* LangExt = UObject::GetLanguage();
	for( INT PathIndex = GSys->LocalizationPaths.Num() - 1; PathIndex >= 0; PathIndex-- )
	{
		const TCHAR* Languages[2] = { LangExt, TEXT("INT") };
		for( INT LangIndex = 0; LangIndex < 2; LangIndex++ )
		{
			if( LangIndex == 1 && appStricmp( LangExt, TEXT("INT") ) == 0 )
			{
				continue;
			}
			// Retail names the file after the game module, which is what makes one file hold every
			// movie player's section: DishonoredGame.<lang>.
			const FString Path = FString::Printf( TEXT("%s") PATH_SEPARATOR TEXT("%s") PATH_SEPARATOR TEXT("DishonoredGame.%s"),
				*GSys->LocalizationPaths( PathIndex ), Languages[LangIndex], Languages[LangIndex] );
			if( GConfig->GetSectionPrivate( Section, FALSE, TRUE, *Path ) != NULL )
			{
				OutPath = Path;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x8218a0
void DisGFxMoviePlayerInitTexts( UDisGFxMoviePlayerBase* Player )
{
	FGFxMovie* Movie = Player->GetMovie();
	GFxMovieView* View = Movie ? Movie->pView.GetPtr() : NULL;
	if( View == NULL )
	{
		return;
	}
	// The language and the file are looked up once per language change, as retail's two statics do;
	// read on first use, never as a file-scope initialiser.
	static FString TextsFile;
	static FString TextsLanguage;
	static UBOOL bTextsFileFound = FALSE;
	const FString Language = UObject::GetLanguage();
	if( TextsLanguage != Language )
	{
		bTextsFileFound = DisFindLocalizedConfigFile( TEXT("DisGFxMoviePlayerBase_Texts"), TextsFile );
		TextsLanguage = Language;
	}
	if( !bTextsFileFound )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): InitTexts: no localisation file with [DisGFxMoviePlayerBase_Texts]") );
		return;
	}

	GFxValue Texts;
	View->CreateObject( &Texts, NULL, NULL, 0 );
	View->SetVariable( "_root.texts", Texts, GFxMovie::SV_Sticky );

	INT Sections = 0;
	INT Keys = 0;
	for( const UClass* Class = Player->GetClass(); Class != NULL; Class = Class->GetSuperClass() )
	{
		const FString SectionName = Class->GetName() + TEXT("_Texts");
		FConfigSection* Section = GConfig->GetSectionPrivate( *SectionName, FALSE, TRUE, *TextsFile );
		if( Section != NULL )
		{
			Sections++;
			for( FConfigSection::TIterator It( *Section ); It; ++It )
			{
				const FString Path = FString( TEXT("_root.texts.") ) + It.Key().ToString();
				GFxValue Value;
				Value.SetStringW( *It.Value() );
				View->SetVariable( TCHAR_TO_ANSI( *Path ), Value, GFxMovie::SV_Sticky );
				Keys++;
			}
		}
		if( Class == UDisGFxMoviePlayerBase::StaticClass() )
		{
			break;
		}
	}
	// DISHONORED(bringup): read one key back through the same interface the content reads it with,
	// so the line proves the strings are reachable and not merely written.
	GFxValue Probe;
	const UBOOL bProbe = View->GetVariable( &Probe, "_root.texts.t_PressAnyKey" );
	debugf( TEXT("DISHONORED(bringup): InitTexts: %s from %s, %d sections, %d strings, ")
		TEXT("_root.texts.t_PressAnyKey %s type %d"),
		*Player->GetClass()->GetName(), *TextsFile, Sections, Keys,
		bProbe ? TEXT("read back") : TEXT("NOT READABLE"), bProbe ? (INT)Probe.GetType() : -1 );
}

// DISHONORED(port): 2013 rva 0x787860 (2012 0x7f4390). Retail compares the movie's aspect against the screen's and
// scales on the axis that fits; the empty space is the half-difference on the other.
void UDisGFxMoviePlayerBase::ComputeMovieSpaceInfo( INT _ScreenSizeX, INT _ScreenSizeY, FDisMovieSpaceInfo& _rInfo )
{
	FGFxMovie* Movie = GetMovie();
	_rInfo.m_ScreenSize.X = (FLOAT)_ScreenSizeX;
	_rInfo.m_ScreenSize.Y = (FLOAT)_ScreenSizeY;
	_rInfo.m_MovieSize.X = (FLOAT)Movie->Info.Width;
	_rInfo.m_MovieSize.Y = (FLOAT)Movie->Info.Height;
	if( _rInfo.m_MovieSize.X / _rInfo.m_MovieSize.Y <= _rInfo.m_ScreenSize.X * ( 1.f / _rInfo.m_ScreenSize.Y ) )
	{
		_rInfo.m_fScreenToMovieScaling = _rInfo.m_MovieSize.Y * ( 1.f / _rInfo.m_ScreenSize.Y );
		_rInfo.m_MovieSpaceSize.X = _rInfo.m_fScreenToMovieScaling * _rInfo.m_ScreenSize.X;
		_rInfo.m_MovieSpaceSize.Y = _rInfo.m_MovieSize.Y;
		_rInfo.m_EmptySpace.X = ( _rInfo.m_MovieSpaceSize.X - _rInfo.m_MovieSize.X ) * 0.5f;
		_rInfo.m_EmptySpace.Y = 0.f;
	}
	else
	{
		_rInfo.m_fScreenToMovieScaling = _rInfo.m_MovieSize.X / _rInfo.m_ScreenSize.X;
		_rInfo.m_MovieSpaceSize.Y = _rInfo.m_fScreenToMovieScaling * _rInfo.m_ScreenSize.Y;
		_rInfo.m_MovieSpaceSize.X = _rInfo.m_MovieSize.X;
		_rInfo.m_EmptySpace.X = 0.f;
		_rInfo.m_EmptySpace.Y = ( _rInfo.m_MovieSpaceSize.Y - _rInfo.m_MovieSize.Y ) * 0.5f;
	}
}

// DISHONORED(port): 2012 rva 0x7f45a0. The base filter: a movie that is closing eats everything, and
// the analogue-stick emulation is the rest of the retail body (UpdateAnalogInputForAS, which needs
// the stick state this build does not deliver).
UBOOL DisGFxMoviePlayerFilterButtonInput( UDisGFxMoviePlayerBase* Player, INT ControllerId,
	FName Key, BYTE Event, UBOOL& bHandled )
{
	if( Player->m_bIsClosing )
	{
		bHandled = TRUE;
		return TRUE;
	}
	return FALSE;
}
