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
