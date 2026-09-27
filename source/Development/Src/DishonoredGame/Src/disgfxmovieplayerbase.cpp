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

// DISHONORED(port): 2012 rva 0x808490. The box itself belongs to UDisGlobalUIManager, which owns the one
// message-box movie every screen shares; what belongs here is the id, because HideMessageBox and
// AddMessageBoxTimer address the box by it and OnMessageBoxResult matches on it.
void UDisGFxMoviePlayerBase::execShowMessageBox( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rMessage);
	P_GET_STR(_rButton0);
	P_GET_STR(_rButton1);
	P_GET_STR(_rButton2);
	P_FINISH;

	FDisMsgBoxInfo Info(EC_EventParm);
	Info.m_Message = _rMessage;
	Info.m_Buttons[0] = _rButton0;
	Info.m_Buttons[1] = _rButton1;
	Info.m_Buttons[2] = _rButton2;
	// DISHONORED(bringup): m_MsgBoxID = DisGetGlobalUIManager()->ShowMessageBox(Info) (2012 0x8aeee0), and
	// retail also (re)registers this movie player for the FArkGameEvent the box answers with. Neither
	// UDisGlobalUIManager::ShowMessageBox nor FArkGameEventDispatcher is declared in this tree.
	m_MsgBoxID = 0;
}

// DISHONORED(port): 2012 rva 0x7f4310
void UDisGFxMoviePlayerBase::execHideMessageBox( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	// DISHONORED(bringup): DisGetGlobalUIManager()->HideMessageBox(m_MsgBoxID) (2012 0x8aef20)
	m_MsgBoxID = 0;
}

// DISHONORED(port): 2012 rva 0x7f42c0 - a duration of zero means ten seconds, which is retail's own default
void UDisGFxMoviePlayerBase::execAddMessageBoxTimer( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(_fDuration);
	P_FINISH;
	const FLOAT Duration = _fDuration == 0.f ? 10.f : _fDuration;
	// DISHONORED(bringup): DisGetGlobalUIManager()->AddMessageBoxTimer(m_MsgBoxID, Duration) (2012 0x8aef00)
	(void)Duration;
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


// DISHONORED(bringup, agent DC): the -gfxuimenu bring-up switch that opens a cooked menu movie through
// the real path moved to GFxUI/Src/gfxuiengine.cpp (FGFxEngine's per-frame tick). It started here, as an
// FTickableObject, and that does not run on a menu map: UWorld::Tick walks
// FTickableObject::TickableObjects only when its TickType is not LEVELTICK_TimeOnly and the world is not
// paused (Engine/Src/UnLevTic.cpp:3397), and a streamed-in menu map with no player controller ticks
// time only. The engine's own per-frame call has no such condition. Why the switch exists at all is in
// agentDC.md: UDisGlobalUIManager's config movie set does not name the main menu, the game's own
// UnrealScript constructs it, and none of that chain runs in this build.
