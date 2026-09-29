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
	*(UBOOL*)Result = m_bIsLoadingMoviePackage ? FALSE : TRUE;
}

// DISHONORED(port): 2012 rva 0x80bb90. FormatText reads a text field's own string back out of the movie,
// runs the interaction-key substitution over it and writes it back, which is how "[Use]" in a localised
// string becomes the key the player actually has bound.
void UDisGFxMoviePlayerBase::execFormatText( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rTextVarPath);
	P_FINISH;

	FGFxMovie* Movie = GetMovie();
	if( Movie == NULL || Movie->pView.GetPtr() == NULL )
	{
		return;
	}
	GFxValue Text( GFxValue::VT_ConvertStringW );
	if( Movie->pView->GetVariable( &Text, FTCHARToUTF8(*_rTextVarPath) ) && !Text.IsUndefined() )
	{
		FString Formatted = Text.GetType() == GFxValue::VT_StringW
			? FString( Text.GetStringW() )
			: FString( FUTF8ToTCHAR( Text.GetString() ) );
		// DISHONORED(bringup): retail then substitutes the bound keys through FormatInteractionText
		// (2012 0x8085b0), FormatGamepadKeyName (0x7f5f30) and FormatMouseKeyName (0x7f5fd0), all three of
		// which read UArkProfileSettings' key bindings; that class is a shim with no accessors in this tree.
		GFxValue Out;
		Out.SetStringW( *Formatted );
		Movie->pView->SetVariable( FTCHARToUTF8(*_rTextVarPath), Out, GFxMovie::SV_Sticky );
	}
	Text.ReleaseManaged();
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
