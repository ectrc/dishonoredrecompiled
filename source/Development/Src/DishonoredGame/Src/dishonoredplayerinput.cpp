// DishonoredGame/src/dishonoredplayerinput.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x6d1a40  public: static void __cdecl UDishonoredPlayerInput::InitializePrivateStaticClassUDishonoredPlayerInput(void)
//   0x6d1a60  public: static void __cdecl UDisTweaks_PlayerInput::InitializePrivateStaticClassUDisTweaks_PlayerInput(void)
//   0x6d1a80  public: virtual void __thiscall UDishonoredPlayerInput::Dis_Jump_ButtonDown(void)
//   0x6d1a90  public: virtual void __thiscall UDishonoredPlayerInput::PlayerInput(float)
//   0x6d1ae0  public: struct FVector2D __thiscall UDishonoredPlayerInput::GetMouseDirection(unsigned int)
//   0x6e4100  public: struct FKeyBind const * __thiscall UDishonoredPlayerInput::GetKeyBindFromAction(class FName, unsigned int, unsigned int)const
//   0x6e41a0  public: virtual void __thiscall UDishonoredPlayerInput::Dis_PlayerChoice_RequestSkip(void)
//   0x6e4200  public: virtual void __thiscall UDishonoredPlayerInput::AdjustMouseSensitivity(float)
//   0x6ef8f0  protected: void __thiscall UDishonoredPlayerInput::ReadPCBindingsFromProfile(void)
//   0x6efbe0  protected: void __thiscall UDishonoredPlayerInput::AddBindingSet(class TArrayNoInit<struct FKeyBind> const &)
//   0x6f1750  protected: void __thiscall UDishonoredPlayerInput::TranslateBaseBindings(void)
//   0x6f3fe0  protected: void __thiscall UDishonoredPlayerInput::InitGameActionBindings(void)
//   0x6f53e0  public: static class UClass * __cdecl UDisTweaks_PlayerInput::GetPrivateStaticClassUDisTweaks_PlayerInput(wchar_t const *)
//   0x6f69a0  public: static class UClass * __cdecl UDisTweaks_PlayerInput::StaticClassNoInline(void)
//   0x6f69d0  protected: unsigned int __thiscall UDishonoredPlayerInput::HandleMultiPressCommand(wchar_t const *, wchar_t const *, wchar_t *, int, class FOutputDevice &)
//   0x6f8db0  public: int __thiscall UDishonoredPlayerInput::TranslateBindingSet(int)const
//   0x6f8e10  protected: virtual void __thiscall UDishonoredPlayerInput::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x6f90b0  public: void __thiscall UDishonoredPlayerInput::OnInit(class ADishonoredPlayerController *)
//   0x6f9170  public: virtual void __thiscall UDishonoredPlayerInput::Dis_SneakOrSlide(unsigned int)
//   0x6f92e0  public: virtual void __thiscall UDishonoredPlayerInput::Dis_PlayerChoice_RequestSkip_Released(void)
//   0x6f9380  public: virtual float __thiscall UDishonoredPlayerInput::GetFOVScale(unsigned int)
//   0x6f9460  protected: void __thiscall UDishonoredPlayerInput::HandleCutSceneSkipping(void)
//   0x6f9620  protected: unsigned int __thiscall UDishonoredPlayerInput::HandleAxisCommand(wchar_t const *, wchar_t const *, wchar_t *, int, class FOutputDevice &)
//   0x6f98c0  public: virtual unsigned int __thiscall UDishonoredPlayerInput::Exec(wchar_t const *, class FOutputDevice &)
//   0x6fa1b0  public: static class UClass * __cdecl UDishonoredPlayerInput::GetPrivateStaticClassUDishonoredPlayerInput(wchar_t const *)
//   0x6fa240  public: virtual void __thiscall UDishonoredPlayerInput::PreProcessInput(float)
//   0x6fa8b0  public: static class UClass * __cdecl UDishonoredPlayerInput::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x6afeb0 (2012 0x6f90b0). 2013 reads UDishonoredEngine::m_bUsingGamepad at its 2013 bit (0x400).
void UDishonoredPlayerInput::OnInit( ADishonoredPlayerController* Owner )
{
	m_pPlayerController = Cast<ADishonoredPlayerController>( Owner );
	m_pTweaks = (UDisTweaks_PlayerInput*)UObject::StaticLoadObject( UDisTweaks_PlayerInput::StaticClass(), NULL, *m_TweakName, NULL, LOAD_None, NULL, TRUE );
	UDishonoredEngine* DishonoredEngine = Cast<UDishonoredEngine>( GEngine );
	if( DishonoredEngine )
	{
		bUsingGamepad = DishonoredEngine->m_bUsingGamepad;
	}
	// DISHONORED(bringup): ArkSettings::ApplyCurrentSettings(IArkSettingsListenerInterface) (Engine, 2013 rva 0x53b790) is not in our
	// Engine yet; the sensitivity/invert options stay at their defaults
}

// DISHONORED(written): 2013 rva 0x6a12b0 (2012 0x6d1a80, same bytes)
void UDishonoredPlayerInput::Dis_Jump_ButtonDown()
{
	m_pPlayerController->bPressedJump = TRUE;
}

// DISHONORED(written): 2013 rva 0x5efd00 (vtable +360)
void UDishonoredPlayerInput::execDis_Jump_ButtonDown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	Dis_Jump_ButtonDown();
}
