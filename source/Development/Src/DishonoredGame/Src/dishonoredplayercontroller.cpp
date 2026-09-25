// DishonoredGame/src/dishonoredplayercontroller.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (106):
//   0x6d0f10  public: static void __cdecl ADishonoredPlayerController::InitializePrivateStaticClassADishonoredPlayerController(void)
//   0x6d0f30  public: static void __cdecl UDisLocalPlayer::InitializePrivateStaticClassUDisLocalPlayer(void)
//   0x6d0f50  protected: virtual class UDisTweaksBase * __thiscall ADishonoredPlayerController::GetTweaks_Derived(void)
//   0x6d0f60  public: virtual void __thiscall ADishonoredPlayerController::BeginDestroy(void)
//   0x6d0f70  public: virtual unsigned int __thiscall ADishonoredPlayerController::InvalidatePostProcessCaches(void)
//   0x6d0f90  public: void __thiscall ADishonoredPlayerController::OnPowerWheelClosed(void)
//   0x6d0fa0  public: virtual void __thiscall ADishonoredPlayerController::HandleWalking(float)
//   0x6d0fd0  public: virtual void __thiscall ADishonoredPlayerController::Dis_Wheel_Open(unsigned int)
//   0x6d1020  public: virtual void __thiscall ADishonoredPlayerController::Dis_Wheel_Close(unsigned int)
//   0x6d10a0  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_GamepadUse(int)
//   0x6d1140  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_KeyboardSelect(int)
//   0x6d1190  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_KeyboardUse(int)
//   0x6d11d0  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_MousePrevious(void)
//   0x6d1250  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_MouseNext(void)
//   0x6d12d0  public: void __thiscall ADishonoredPlayerController::OnEquipmentChange(enum eDisUISelectionType)
//   0x6d1300  public: virtual void __thiscall ADishonoredPlayerController::Dis_OpenPauseMenu(void)
//   0x6d1320  public: virtual void __thiscall ADishonoredPlayerController::Possess(class APawn *)
//   0x6d1330  public: virtual void __thiscall ADishonoredPlayerController::UnPossess(void)
//   0x6d1340  public: void __thiscall ADishonoredPlayerController::EnableControllerInput(int, unsigned int, enum DisInputMaskLevel)
//   0x6d1370  public: unsigned int __thiscall ADishonoredPlayerController::IsInputEnabled(int)const
//   0x6d13f0  public: virtual unsigned int __thiscall ADishonoredPlayerController::IsMoveInputIgnored(void)const
//   0x6d1400  public: virtual unsigned int __thiscall ADishonoredPlayerController::IsLookInputIgnored(void)const
//   0x6d1470  public: virtual void __thiscall ADishonoredPlayerController::Dis_LeanWithAnalogStick_End(void)
//   0x6d1490  public: virtual void __thiscall ADishonoredPlayerController::PreRender(class UCanvas *)
//   0x6d6940  protected: virtual void __thiscall ADishonoredPlayerController::SetTweaks_Derived(class UDisTweaksBase *)
//   0x6d6950  public: virtual void __thiscall ADishonoredPlayerController::PostBeginPlay(void)
//   0x6d6a10  public: void __thiscall ADishonoredPlayerController::CalcDispersion(struct FVector2D const &, struct FVector2D const &, class FVector &)const
//   0x6d6b20  public: unsigned int __thiscall ADishonoredPlayerController::FindCombatTarget_Dir(class UDisItemContext const *, struct FDisLineProbeResult &, class FVector const &, class FVector const &, int, int, int, class FVector, unsigned int, class TArray<class AActor const *, class FDefaultAllocator> const *)const
//   0x6d6cc0  private: unsigned int __thiscall ADishonoredPlayerController::CanInteractWithInteractable(class IDisInteractableInterface const &, struct FCanInteractParams const &, class FVector const &, class FVector const &)const
//   0x6d6d40  protected: void __thiscall ADishonoredPlayerController::ApplyDarkVisionPostProcessSettings(class ADishonoredPlayerPawn *, float)
//   0x6d6de0  public: virtual void __thiscall ADishonoredPlayerController::Dis_Zoom(void)
//   0x6d6e10  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Context_Interactables(float)
//   0x6d6e60  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Movement_Movable(class ADishonoredPawn *)
//   0x6d6f20  public: virtual void __thiscall ADishonoredPlayerController::Dis_NextItemOption(unsigned char)
//   0x6d6f80  public: virtual void __thiscall ADishonoredPlayerController::Dis_PrevItemOption(unsigned char)
//   0x6d6fe0  public: virtual void __thiscall ADishonoredPlayerController::Dis_CancelPossession(void)
//   0x6d7030  public: virtual void __thiscall ADishonoredPlayerController::Dis_WheelShortcuts_GamepadSelect(int)
//   0x6d70e0  public: virtual void __thiscall ADishonoredPlayerController::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x6d7170  public: virtual void __thiscall ADishonoredPlayerController::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6d7210  public: virtual void __thiscall ADishonoredPlayerController::Dis_ToggleJournal(void)
//   0x6d7280  public: virtual void __thiscall ADishonoredPlayerController::Dis_OpenJournalTab(int)
//   0x6d72f0  public: virtual void __thiscall ADishonoredPlayerController::PreSetCinematicMode_Native(unsigned int, unsigned int)
//   0x6d7330  public: virtual void __thiscall ADishonoredPlayerController::ToggleScreenConfig(void)
//   0x6d7370  public: void __thiscall ADishonoredPlayerController::OnStopPossessing(void)
//   0x6d73d0  protected: void __thiscall ADishonoredPlayerController::PlayPossessionActionFailSound(void)
//   0x6d73f0  public: virtual void __thiscall ADishonoredPlayerController::OnShowPowerMenu(class UDisSeqAct_ShowPowerMenu *)
//   0x6d7410  private: virtual class UObject * __thiscall ADishonoredPlayerController::GetCustomSeqActForwardingObject(void)
//   0x6e1d50  public: virtual void __thiscall ADishonoredPlayerController::OnTeleport_Native(class USeqAct_Teleport *)
//   0x6e1da0  public: void __thiscall ADishonoredPlayerController::ShakeCam(class FVector const &)
//   0x6e1dd0  public: void __thiscall ADishonoredPlayerController::ApplyCamRecoil(class FVector const &)
//   0x6e1e00  public: void __thiscall ADishonoredPlayerController::GetPlayerCrosshairTarget(struct FImpactInfo &, int, class FVector const *)const
//   0x6e1ee0  public: void __thiscall ADishonoredPlayerController::GetPlayerCombatTarget(struct FImpactInfo &, int, class UDisItemContext const *, class FVector const *)const
//   0x6e2090  private: void __thiscall ADishonoredPlayerController::SetCrosshairActor(class AActor *, enum eCrossHairStatus, unsigned int)
//   0x6e2260  private: void __thiscall ADishonoredPlayerController::UpdateSunBlindingEffect(void)
//   0x6e23f0  protected: void __thiscall ADishonoredPlayerController::ApplyAdrenalineProcessSettings(class ADishonoredPlayerPawn *, float)
//   0x6e2550  protected: void __thiscall ADishonoredPlayerController::ApplyWaterPostProcessSettings(float)
//   0x6e26d0  protected: void __thiscall ADishonoredPlayerController::ApplyMusicalOverseerPostProcessSettings(class ADishonoredPlayerPawn *, struct FArkPpConfig &, float)
//   0x6e2880  protected: void __thiscall ADishonoredPlayerController::ApplyPossessionPostProcessSettings(float)
//   0x6e2b10  public: virtual void __thiscall ADishonoredPlayerController::CheckJumpOrDuck(void)
//   0x6e2bb0  public: virtual void __thiscall ADishonoredPlayerController::Dis_VersusAlt(void)
//   0x6e2c20  public: unsigned int __thiscall ADishonoredPlayerController::CanHolsterWeapons(void)const
//   0x6e2c90  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Context_Block(void)
//   0x6e2d50  public: virtual void __thiscall ADishonoredPlayerController::Dis_UsePrimaryItemOrAdrenaline_Multi(int, unsigned int)
//   0x6e2e10  public: virtual void __thiscall ADishonoredPlayerController::Dis_ExitKeyhole(void)
//   0x6e2e80  public: virtual unsigned int __thiscall ADishonoredPlayerController::Dis_ConsumeElixir(unsigned char)
//   0x6e2fa0  public: virtual unsigned int __thiscall ADishonoredPlayerController::Dis_UseAdrenaline(unsigned int, unsigned int)
//   0x6e3010  public: virtual unsigned int __thiscall ADishonoredPlayerController::Dis_UseAdrenaline_Multi(unsigned int, int, unsigned int)
//   0x6e3090  public: virtual void __thiscall ADishonoredPlayerController::Dis_LeanOrAdrenaline_Multi(int, unsigned int)
//   0x6e3160  public: virtual void __thiscall ADishonoredPlayerController::Dis_NextEquippedItem(unsigned int)
//   0x6e31a0  public: virtual void __thiscall ADishonoredPlayerController::Dis_PrevEquippedItem(unsigned int)
//   0x6e31e0  public: virtual class UArkProfileSettings * __thiscall ADishonoredPlayerController::GetProfileSettings(void)
//   0x6e3200  public: void __thiscall ADishonoredPlayerController::ReturnToMainMenu(unsigned int)
//   0x6e3280  public: unsigned int __thiscall ADishonoredPlayerController::CanEquip(enum EDisEquipUsage)const
//   0x6e32f0  public: virtual void __thiscall ADishonoredPlayerController::SetCinematicMode_Native(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
//   0x6e33e0  public: void __thiscall ADishonoredPlayerController::UpdateRemoteListener(float, float, class TArray<class AActor const *, class FDefaultAllocator> &)
//   0x6e39a0  public: void __thiscall ADishonoredPlayerController::OnStartPossessing(class UDisTweaks_Possessable const *)
//   0x6e3ac0  public: void __thiscall ADishonoredPlayerController::EquipBestSuitableItem(enum EDisEquipUsage, unsigned int, enum ADishonoredPawn::EEquipItemFlags)
//   0x6e3b50  public: virtual float __thiscall ADishonoredPlayerController::CalcPlayerSwimAccelRate(void)
//   0x6eb830  public: void __thiscall ADishonoredPlayerController::UpdateBlindedEffect(float, struct FDishonoredViewTarget const &)
//   0x6eb900  public: virtual void __thiscall ADishonoredPlayerController::ModifyPostProcessSettings(struct FArkPpConfig &)
//   0x6ebed0  public: void __thiscall ADishonoredPlayerController::HolsterWeapons(class ADishonoredPawn *)
//   0x6ec0a0  public: virtual void __thiscall ADishonoredPlayerController::Dis_UsePrimaryItem(void)
//   0x6ec370  public: virtual void __thiscall ADishonoredPlayerController::Dis_UseSecondaryItem(void)
//   0x6ec4f0  public: virtual void __thiscall ADishonoredPlayerController::Dis_SelectPower(int)
//   0x6ec640  public: void __thiscall ADishonoredPlayerController::SwitchEquipment(enum eDisUISelectionType)
//   0x6ec850  public: virtual void __thiscall ADishonoredPlayerController::Dis_EquipItemByType(class UClass *, unsigned char)
//   0x6ee7d0  protected: void __thiscall ADishonoredPlayerController::TickZoom(float)
//   0x6ef510  public: virtual unsigned int __thiscall ADishonoredPlayerController::Tick(float, enum ELevelTick)
//   0x6ef5e0  public: unsigned int __thiscall ADishonoredPlayerController::FindCombatTargetMulti_Dir(class UDisItemContext const *, struct TMemStackArray<struct FDisLineProbeResult> &, class FVector const &, class FVector const &, int, int, int, class FVector, unsigned int, class TArray<class AActor const *, class FDefaultAllocator> const *)const
//   0x6ef780  public: virtual void __thiscall ADishonoredPlayerController::OnObjectiveAction(class UDisSeqAct_ObjectiveAction *)
//   0x6f3f50  public: static class UClass * __cdecl UDisLocalPlayer::GetPrivateStaticClassUDisLocalPlayer(wchar_t const *)
//   0x6f4f40  public: static class UClass * __cdecl UDisLocalPlayer::StaticClassNoInline(void)
//   0x6f5350  public: static class UClass * __cdecl ADishonoredPlayerController::GetPrivateStaticClassADishonoredPlayerController(wchar_t const *)
//   0x6f6970  public: static class UClass * __cdecl ADishonoredPlayerController::StaticClassNoInline(void)
//   0x6f73f0  public: class AActor * __thiscall ADishonoredPlayerController::FindInteractableActor(struct FDishonoredViewTarget const &)
//   0x6f7910  public: void __thiscall ADishonoredPlayerController::UpdateCrosshair(struct FDishonoredViewTarget const &)
//   0x6f7ac0  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Context_PowerWheel(float)
//   0x6f7e60  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Lean(void)
//   0x6f80b0  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Movement_Gamepad(class UDishonoredPlayerInput *)
//   0x6f8590  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Movement(void)
//   0x6f8690  public: virtual void __thiscall ADishonoredPlayerController::DisToggleSprint(unsigned int)
//   0x6f8770  public: virtual void __thiscall ADishonoredPlayerController::Dis_Lean_Toggle(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
//   0x6f9ac0  public: virtual void __thiscall ADishonoredPlayerController::ReceivedPlayer_Native(void)
//   0x6f9ae0  protected: void __thiscall ADishonoredPlayerController::HandleHeldButtons_Context(float)
//   0x6f9b80  public: virtual void __thiscall ADishonoredPlayerController::HandleHeldButtons(float)
//   0x6fa840  public: void __thiscall ADishonoredPlayerController::OnCameraUpdate(struct FDishonoredViewTarget const &, int)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

ADishonoredPlayerController* ADishonoredPlayerController::s_pInstance = NULL;

// DISHONORED(written): 2013 rva 0x6a2a50 (2012 0x6d6950). Retail calls AController::PostBeginPlay directly (APlayerController has
// no C++ override) and clears CheatClass, so no cheat manager is ever created from it.
void ADishonoredPlayerController::PostBeginPlay()
{
	s_pInstance = this;
	DisGetGameInfo();
	m_bNeedToEnablePlaytestNextTick = FALSE;
	m_WheelPreviousSelection = 0;
	CheatClass = NULL;
	AController::PostBeginPlay();
	for( INT MaskIndex = 0; MaskIndex < ARRAY_COUNT(m_InputEnableMask); MaskIndex++ )
	{
		m_InputEnableMask[MaskIndex] = 0x7FFFF;
	}
	m_PossessionEffectSettings.m_Stage = 0;
	m_pBlindnessPpNode = DisGetArkPpNodeMaterial( FName(TEXT("WatchtowerBlindedPostProcess")), TRUE );
	// DISHONORED(bringup): IDisTweaksInterface::ApplyTweakChanges on the tweaks subobject and
	// UDishonoredNativeStateMachine::InitFSM(m_pUseInteractionFSM, this, FDisUseState_WaitForInput_Param(), 0): the tweak interface
	// and the state machine transitions are not ported yet
}

// DISHONORED(written): 2013 rva 0x6a0730 (2012 0x6d0f60, same bytes). Retail skips the controller bases and calls AActor's.
void ADishonoredPlayerController::BeginDestroy()
{
	s_pInstance = NULL;
	AActor::BeginDestroy();
}

// DISHONORED(written): 2013 rva 0x6a0b30 (2012 0x6d1370). The retail first test is GEngine->IsDebugMenuVisible() (UEngine vtable
// +392), which UDishonoredEngine leaves at the base FALSE (2013 rva 0x322980): the debug menu is compiled out. Every one of the 11
// enable masks has to share a bit with the request.
UBOOL ADishonoredPlayerController::IsInputEnabled( INT InputMask ) const
{
	DWORD Mask = InputMask;
	if( bIgnoreLookInput )
	{
		Mask &= 0xFFFFFFFD;
	}
	if( bIgnoreMoveInput )
	{
		Mask &= 0xFFFFECFE;
	}
	if( m_bInputIgnoreInput_Cinematic )
	{
		Mask &= 0xFFF81303;
	}
	for( INT MaskIndex = 0; MaskIndex < ARRAY_COUNT(m_InputEnableMask); MaskIndex++ )
	{
		if( !( Mask & m_InputEnableMask[MaskIndex] ) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x6a0bb0 (2012 0x6d13f0, same bytes)
UBOOL ADishonoredPlayerController::IsMoveInputIgnored() const
{
	return !IsInputEnabled( 1 );
}

// DISHONORED(written): 2013 rva 0x6a0bc0 (2012 0x6d1400, same bytes): the free and root-following cameras keep the look input
UBOOL ADishonoredPlayerController::IsLookInputIgnored() const
{
	const UBOOL bCameraOwnsLook = PlayerCamera && ( PlayerCamera->CameraStyle == FName(TEXT("FreeCam")) || PlayerCamera->CameraStyle == FName(TEXT("FollowRoot")) );
	return !IsInputEnabled( 2 ) && !bCameraOwnsLook;
}

// DISHONORED(written): 2013 rva 0x6b3d60 (2012 0x6f9ac0, same bytes)
void ADishonoredPlayerController::ReceivedPlayer_Native()
{
	UDishonoredPlayerInput* DishonoredInput = (UDishonoredPlayerInput*)PlayerInput;
	DishonoredInput->OnInit( this );
	// DISHONORED(bringup): ArkSettings::ApplyCurrentSettings(IArkSettingsListenerInterface) (Engine, 2013 rva 0x53b790) is not in our
	// Engine yet; the current options are not pushed to the controller
}

// DISHONORED(written): 2013 rva 0x6a6f30 (2012 0x6e31e0, same bytes)
UArkProfileSettings* ADishonoredPlayerController::GetProfileSettings()
{
	return Cast<UArkProfileSettings>( OnlinePlayerData->ProfileProvider->Profile );
}

// DISHONORED(written): the 2013 vtable slot +1432 is an empty body (2013 rva 0x1cb0c0)
void ADishonoredPlayerController::OnControllerChanged_Native( UBOOL bIsConnected )
{
}

// DISHONORED(written): 2013 rva 0x1d3a70 = APlayerController::execIsMoveInputIgnored (vtable +1172)
void ADishonoredPlayerController::execIsMoveInputIgnored( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = IsMoveInputIgnored();
}

// DISHONORED(written): 2013 rva 0x1d3ab0 = APlayerController::execIsLookInputIgnored (vtable +1176)
void ADishonoredPlayerController::execIsLookInputIgnored( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = IsLookInputIgnored();
}

// DISHONORED(written): 2013 rva 0x5edbf0 (vtable +1268)
void ADishonoredPlayerController::execReceivedPlayer_Native( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	ReceivedPlayer_Native();
}

// DISHONORED(written): 2013 rva 0x5eeca0 (vtable +1424)
void ADishonoredPlayerController::execGetProfileSettings( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UArkProfileSettings**)Result = GetProfileSettings();
}

// DISHONORED(written): 2013 rva 0x5eed20 (vtable +1432)
void ADishonoredPlayerController::execOnControllerChanged_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(bIsConnected);
	P_FINISH;
	OnControllerChanged_Native( bIsConnected );
}
