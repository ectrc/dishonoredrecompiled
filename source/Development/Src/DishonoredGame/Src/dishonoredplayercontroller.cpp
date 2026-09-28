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
#include "disaicensus.h"
#include "disheadcensus.h"

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

// DISHONORED(written): 2013 rva 0x6a0ae0 (2012 0x6d1320, identical): the override only chains to APlayerController::Possess. The
// DISHONORED(bringup) line is agent AF's milestone-5 marker; retail logs nothing (Shipping).
void ADishonoredPlayerController::Possess( APawn* inPawn )
{
	APlayerController::Possess( inPawn );
	// DISHONORED(bringup): retail's APlayerController::Possess (2013 rva 0x1330b0) ends by raising a no-parameter script event on the
	// controller - the one that puts it into the pawn's land movement state - after setting Pawn, SetTickIsDisabled(inPawn, FALSE),
	// TimeMargin = -0.1 and MaxTimeMargin from the AGameInfo default. Our APlayerController has no Possess override at all
	// (Engine/Src/playercontroller.cpp is still an import_reference.py stub), so AController::Possess runs and the controller stays
	// in no state: GetStateFrame()->StateNode is the class itself, `PlayerMove` dispatches nothing, and with it neither
	// PlayerMove_Walking nor UpdateRotation ever run, so input moves and turns nothing. Raising Restart here is the bring-up stand-in.
	// Hand-over to agent AE, who owns UnController.cpp / EngineControllerClasses.h: port APlayerController::Possess and UnPossess
	// (0x1330b0 / 0x130090) so this block can go.
	UFunction* Restart = FindFunction( FName(TEXT("Restart"), FNAME_Find) );
	if( Restart )
	{
		ProcessEvent( Restart, NULL );
	}
	else
	{
		warnf( TEXT("DISHONORED(bringup): ADishonoredPlayerController::Possess: no Restart event, the controller stays stateless") );
	}
	const FString MapName = ( GWorld && GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->CommittedPersistentLevelName != NAME_None )
		? GWorld->GetWorldInfo()->CommittedPersistentLevelName.ToString()
		: ( GWorld && GWorld->PersistentLevel ? GWorld->PersistentLevel->GetOutermost()->GetName() : FString(TEXT("None")) );
	debugf( TEXT("DISHONORED(bringup): possessed %s (%s) in %s by %s at %s"),
		inPawn ? *inPawn->GetName() : TEXT("None"), inPawn ? *inPawn->GetClass()->GetName() : TEXT("None"),
		*MapName, *GetClass()->GetName(), inPawn ? *inPawn->Location.ToString() : TEXT("None") );
}

// DISHONORED(written): 2013 rva 0x6a0af0 (2012 0x6d1330): a thunk to APlayerController::UnPossess
void ADishonoredPlayerController::UnPossess()
{
	APlayerController::UnPossess();
}

// DISHONORED(written): 2013 rva 0x6a0770 (2012 0x6d0fa0): with a pawn the held-button pass runs instead of the reference's bRun
// handling (APlayerController::HandleWalking 0x243000 is never reached from here). HandleHeldButtons (vtable +1264, 2013 0x6ba9a0)
// needs the player FSM and the tweak-driven movement helpers, so it is still a warn-once native stub: walking accelerates through
// APlayerController::PlayerMove_Walking from the aForward/aStrafe axes, which is what milestone 5 exercises.
void ADishonoredPlayerController::HandleWalking( FLOAT DeltaTime )
{
	if( Pawn )
	{
		UFunction* HandleHeldButtons = FindFunction( FName(TEXT("HandleHeldButtons"), FNAME_Find) );
		if( HandleHeldButtons )
		{
			struct { FLOAT DeltaSeconds; } Parms;
			Parms.DeltaSeconds = DeltaTime;
			ProcessEvent( HandleHeldButtons, &Parms );
		}
	}
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

// ---- agent AU ports (PHASE7 AU): HandleHeldButtons, Dis_Zoom, and the -dispickup census / -dispickupprobe ----

#include "dishonoredutilities.h"
#include "disaicensus.h"

// DISHONORED(written): 2013 rva 0x6ba9a0 (2012 0x6f9b80): a paused world, and a single-stepped world that is currently
// paused between steps, skip the whole held-button pass; both thumbsticks pressed together toggles the debug menu.
// DISHONORED(bringup): of the three sub-handlers only HandleHeldButtons_Context is ported. The other two are named here
// rather than silently skipped, because their own dependencies are unported subsystems:
//   HandleHeldButtons_Movement 2013 rva 0x6aeb10 (+ _Movement_Gamepad 0x6ae610, _Movement_Movable 0x6a2ec0) - the
//     movable-object carry and the gamepad dead zones, which need UDisMovableComponent and UDisTweaks_PlayerInput;
//   HandleHeldButtons_Lean 2013 rva 0x6b3ff0 - the lean state, which needs the player master FSM's lean state.
void ADishonoredPlayerController::HandleHeldButtons( FLOAT DeltaSeconds )
{
	if( DisPickupCensusEnabled() )
	{
		DisPickupReport( GWorld, DeltaSeconds );
	}
	// DISHONORED(written): agent CG's -disai census hangs off the same per-frame script call as agent AU's -dispickup,
	// so the AI census costs nothing when the switch is absent and no engine file is touched for it.
	if( DisAICensusEnabled() )
	{
		DisAIReport( GWorld, DeltaSeconds );
	}
	// DISHONORED(written): agent DI's -dishead census rides the same call for the same reason: it costs nothing when the
	// switch is absent and no engine file is touched for it.
	if( DisHeadCensusEnabled() )
	{
		DisHeadReport( GWorld, DeltaSeconds );
	}
	if( DisHeadCamEnabled() )
	{
		DisHeadFrameOneNPC( this, GWorld );
	}

	AWorldInfo* Info = WorldInfo;
	if( Info && !Info->Paused && ( !Info->m_bSingleStep || !Info->m_bSingleStepPaused ) )
	{
		static UBOOL bWarnedSubHandlers = FALSE;
		if( !bWarnedSubHandlers )
		{
			bWarnedSubHandlers = TRUE;
			debugf( TEXT("DISHONORED(bringup): HandleHeldButtons: _Movement (2013 rva 0x6aeb10) and _Lean (0x6b3ff0) are not ported; _Context is") );
		}
		HandleHeldButtons_Context( DeltaSeconds );
	}

	if( m_bLeftThumbstickPressed && m_bRightThumbstickPressed )
	{
		ConsoleCommand( FString( TEXT("ToggleDebugMenu") ), TRUE );
		m_bLeftThumbstickPressed = FALSE;
		m_bRightThumbstickPressed = FALSE;
	}
}

// DISHONORED(written): 2013 rva 0x6ba900 (2012 0x6f9ae0, same bytes): retail reads the player input's tweaks and the
// world's real time (both results discarded in the shipped build, which is why the two calls look dead) and then runs
// the three context handlers, finally stopping the secondary item's zoom when zoom input is disabled.
// DISHONORED(bringup): _Context_Block (2012 rva 0x6e2c90) and _Context_PowerWheel (2013 0x6b7d30) need the block state
// and the GFx power wheel; UDishonoredInventoryItem::StopZoom needs the item contexts (agentAJ.md's UDisItemContext).
void ADishonoredPlayerController::HandleHeldButtons_Context( FLOAT DeltaSeconds )
{
	if( !PlayerInput )
	{
		return;
	}
	HandleHeldButtons_Context_Interactables( DeltaSeconds );
}

// DISHONORED(written): 2013 rva 0x6a2e70 (2012 0x6d6e10, same bytes): while use input is disabled the use-interaction
// FSM is pushed back to its waiting state, and the machine is ticked either way.
// DISHONORED(bringup): m_pUseInteractionFSM is never created, because ADishonoredPlayerController::PostBeginPlay's
// UDishonoredNativeStateMachine::InitFSM call and the UDisUseState_* classes are not ported (agentAJ.md); the state
// parameter FDisUseState_WaitForInput_Param does not exist either. Until they are, the pickup path is reached through
// -dispickupprobe below, which makes the same IDisInteractableInterface::AttemptInteract call
// UDisUseState_WaitForInput::TickState (2013 rva 0x6d5ba0) makes.
void ADishonoredPlayerController::HandleHeldButtons_Context_Interactables( FLOAT DeltaSeconds )
{
	if( !m_pUseInteractionFSM )
	{
		return;
	}
	m_pUseInteractionFSM->TickStateMachine( DeltaSeconds );
}

// DISHONORED(written): 2013 rva 0x6a2e40 (2012 0x6d6de0, same bytes): the secondary item's zoom toggle.
// DISHONORED(bringup): UDishonoredInventory::GetEquippedItem is ported (agent AJ) but
// UDishonoredInventoryItem::ToggleZoom (retail vtable +400) is not, so only the input gate runs.
void ADishonoredPlayerController::Dis_Zoom()
{
	if( !IsInputEnabled( 0x8000 ) )
	{
		return;
	}
	ADishonoredPawn* DisPawn = Cast<ADishonoredPawn>( Pawn );
	UDishonoredInventory* Inventory = DisPawn ? DisPawn->m_pInventory : NULL;
	if( Inventory )
	{
		Inventory->GetEquippedItem( EDisEquipUsage_Secondary );
	}
}

void ADishonoredPlayerController::execHandleHeldButtons( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(DeltaSeconds);
	P_FINISH;
	HandleHeldButtons( DeltaSeconds );
}

void ADishonoredPlayerController::execDis_Zoom( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	Dis_Zoom();
}

/*-----------------------------------------------------------------------------
	-dispickup / -dispickupprobe (DISHONORED(bringup) only, free when off)

	-dispickup      once a second, per world: the pickup inventory of the map, how many of them a trace or a touch can
	                reach, the player's inventory, and how many pickups this run has consumed.
	-dispickupprobe after the census has settled, walk the pawn to one pickup per second (UWorld::FarMoveActor, the same
	                path agent AS's -distouchprobe uses) and then make the call UDisUseState_WaitForInput::TickState
	                makes on the crosshair actor: IDisInteractableInterface::AttemptInteract( PlayerPawn ).
-----------------------------------------------------------------------------*/

static INT GDisPickupCensus = -1;
static INT GDisPickupProbe = -1;

UBOOL DisPickupCensusEnabled()
{
	if( GDisPickupCensus < 0 )
	{
		GDisPickupCensus = ( appStrfind( appCmdLine(), TEXT("-dispickup") ) != NULL ) ? 1 : 0;
	}
	return GDisPickupCensus != 0;
}

static UBOOL DisPickupProbeEnabled()
{
	if( GDisPickupProbe < 0 )
	{
		GDisPickupProbe = ( appStrfind( appCmdLine(), TEXT("-dispickupprobe") ) != NULL ) ? 1 : 0;
	}
	return GDisPickupProbe != 0;
}

/** Everything the census and the probe remember between frames, re-armed for every new world. */
struct FDisPickupCensusState
{
	UWorld* World;
	FLOAT NextReportTime;
	FLOAT NextProbeTime;
	INT Interacts;
	INT Collected;
	TArray<ADisPickup_Base*> Targets;
	TArray<ADisPickup_Base*> Probed;

	FDisPickupCensusState() : World(NULL), NextReportTime(0.f), NextProbeTime(0.f), Interacts(0), Collected(0) {}
};

static FDisPickupCensusState GDisPickupState;

static void DisPickupLogInventory( const TCHAR* Tag, ADishonoredPlayerPawn* PlayerPawn )
{
	if( !PlayerPawn )
	{
		debugf( TEXT("DISHONORED(bringup): dispickup %s inventory: no player pawn"), Tag );
		return;
	}
	UDishonoredInventory* Inventory = PlayerPawn->m_pInventory;
	FString Ammo;
	INT AmmoTotal = 0;
	if( Inventory )
	{
		for( INT Type = 0; Type < Inventory->m_AmmoInfo.Num(); Type++ )
		{
			AmmoTotal += Inventory->m_AmmoInfo(Type).m_AmmoCount;
			Ammo += FString::Printf( TEXT("%i/%i "), Inventory->m_AmmoInfo(Type).m_AmmoCount, Inventory->m_AmmoInfo(Type).m_AmmoCapacity );
		}
	}
	FString Items;
	if( Inventory )
	{
		for( INT Idx = 0; Idx < Inventory->m_AbstractItem.Num(); Idx++ )
		{
			UDisAbstractItem* Item = Inventory->m_AbstractItem(Idx).m_pItem;
			Items += FString::Printf( TEXT("%s=%i "), Item ? *Item->GetName() : TEXT("None"), Inventory->m_AbstractItem(Idx).m_Quantity );
		}
	}
	debugf( TEXT("DISHONORED(bringup): dispickup %s inventory: health %i/%i mana %i/%i elixirs %i/%i ammo total %i [%s] items %i [%s]"),
		Tag, PlayerPawn->Health, PlayerPawn->HealthMax, PlayerPawn->m_Mana, PlayerPawn->m_ManaMax,
		Inventory ? Inventory->m_ElixirCounts[0] : 0, Inventory ? Inventory->m_ElixirCounts[1] : 0,
		AmmoTotal, *Ammo, Inventory ? Inventory->m_AbstractItem.Num() : 0, *Items );
}

void DisPickupReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World || !World->GetWorldInfo() )
	{
		return;
	}
	const FLOAT Now = World->GetTimeSeconds();
	if( GDisPickupState.World != World )
	{
		GDisPickupState = FDisPickupCensusState();
		GDisPickupState.World = World;
		GDisPickupState.NextProbeTime = Now + 14.f;
	}
	// The pickups of a map live in its streamed sub-levels (L_Pub_Day_P holds 147 of them), which are added seconds
	// after the persistent level, so the target list is re-collected until it stops growing.
	if( Now >= GDisPickupState.NextReportTime )
	{
		const INT Was = GDisPickupState.Targets.Num();
		GDisPickupState.Targets.Empty();
		for( FActorIterator It; It; ++It )
		{
			ADisPickup_Base* Pickup = Cast<ADisPickup_Base>( *It );
			if( Pickup )
			{
				GDisPickupState.Targets.AddItem( Pickup );
			}
		}
		if( GDisPickupState.Targets.Num() != Was )
		{
			debugf( TEXT("DISHONORED(bringup): dispickup %s: target list %i -> %i pickups"), *World->GetOutermost()->GetName(), Was, GDisPickupState.Targets.Num() );
		}
	}

	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;

	if( Now >= GDisPickupState.NextReportTime )
	{
		GDisPickupState.NextReportTime = Now + 1.f;
		INT Reachable = 0;
		INT Consumed = 0;
		for( INT Idx = 0; Idx < GDisPickupState.Targets.Num(); Idx++ )
		{
			ADisPickup_Base* Pickup = GDisPickupState.Targets(Idx);
			if( !Pickup || Pickup->IsPendingKill() )
			{
				continue;
			}
			if( Pickup->m_bPendingDestructionAfterOneFullTickCycle )
			{
				Consumed++;
			}
			else if( Pickup->bCollideActors )
			{
				Reachable++;
			}
		}
		GDisPickupState.Collected = Consumed;
		debugf( TEXT("DISHONORED(bringup): dispickup %6.1fs %s: pickups %i (collidable %i, consumed %i), interacts attempted %i"),
			Now, *World->GetOutermost()->GetName(), GDisPickupState.Targets.Num(), Reachable, Consumed, GDisPickupState.Interacts );
		DisPickupLogInventory( TEXT("now"), PlayerPawn );
	}

	if( !DisPickupProbeEnabled() || !PlayerPawn || Now < GDisPickupState.NextProbeTime )
	{
		return;
	}
	GDisPickupState.NextProbeTime = Now + 1.f;
	for( INT Idx = 0; Idx < GDisPickupState.Targets.Num(); Idx++ )
	{
		ADisPickup_Base* Pickup = GDisPickupState.Targets(Idx);
		INT Unused = 0;
		if( !Pickup || Pickup->IsPendingKill() || Pickup->m_bPendingDestructionAfterOneFullTickCycle
			|| GDisPickupState.Probed.FindItem( Pickup, Unused ) )
		{
			continue;
		}
		GDisPickupState.Probed.AddItem( Pickup );
		// The pawn is walked to the pickup the way agent AS's -distouchprobe does: at the collision component's bounds
		// origin, which is inside the brush or mesh, not at the pivot.
		FVector Target = Pickup->CollisionComponent ? Pickup->CollisionComponent->Bounds.Origin : Pickup->Location;
		Target.Z += 40.f;
		GWorld->FarMoveActor( PlayerPawn, Target, FALSE, TRUE, TRUE );

		DisPickupLogInventory( TEXT("before"), PlayerPawn );
		const UBOOL bCanBePickedUp = Pickup->CanBePickedUp( PlayerPawn );
		const eCrossHairStatus Status = Pickup->GetCrosshairStatus( PlayerPawn );
		GDisPickupState.Interacts++;
		const UBOOL bInteracted = Pickup->AttemptInteract( PlayerPawn );
		debugf( TEXT("DISHONORED(bringup): dispickup probe %i/%i %s (%s): CanBePickedUp %i crosshair %i AttemptInteract %i travelling %i"),
			GDisPickupState.Probed.Num(), GDisPickupState.Targets.Num(), *Pickup->GetName(), *Pickup->GetClass()->GetName(),
			bCanBePickedUp, (INT)Status, bInteracted, Pickup->m_pTravellingTowardPC != NULL );
		DisPickupLogInventory( TEXT("after "), PlayerPawn );
		return;
	}
}
